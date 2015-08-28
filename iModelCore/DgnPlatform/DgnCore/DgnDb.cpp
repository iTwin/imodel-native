/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnDb.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/DgnGeoCoord.h>

static WCharCP s_dgndbExt   = L".dgndb";

/*---------------------------------------------------------------------------------**//**
* used to check names saved in categories, models, etc.
* @bsimethod                                    Keith.Bentley                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnDbTable::IsValidName(Utf8StringCR name, Utf8CP invalidChars)
    {
    // empty names, names that start or end with space, or contain an invalid character are illegal.
    // NOTE: don't use isspace for test below - it is locale specific and finds the non-breaking-space (0xA0) when using Latin-8 locale.
    return !name.empty() && ' ' != *name.begin() && ' ' != *name.rbegin() &&(Utf8String::npos == name.find_first_of(invalidChars));
    }

/*---------------------------------------------------------------------------------**//**
* replace invalid characters in a string with a substitute character
* @bsimethod                                    Keith.Bentley                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbTable::ReplaceInvalidCharacters(Utf8StringR str, Utf8CP invalidChars, Utf8Char r)
    {
    size_t i, iprev = 0;
    while ((i = str.find_first_of(invalidChars, iprev)) != Utf8String::npos)
        {
        str[i] = r;
        iprev = i+1;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDb::DgnDb() : m_schemaVersion(0,0,0,0), m_fonts(*this, DGN_TABLE_Font), m_colors(*this), m_categories(*this), m_domains(*this), m_styles(*this), m_views(*this),
                 m_geomParts(*this), m_units(*this), m_models(*this), m_elements(*this), 
                 m_materials(*this), m_links(*this), m_authorities(*this), m_textures(*this),
                 m_ecsqlCache(50, "DgnDb")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDb::Destroy()
    {
    m_models.Empty();
    m_txnManager = nullptr; // RefCountedPtr, deletes TxnManager
    m_ecsqlCache.Empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDb::~DgnDb()
    {
    if (m_txnManager.IsValid() && m_txnManager->HasChanges())
        {
        BeAssert(false && "Make sure you save your outstanding Txn before deleting a DgnDb");
        SaveChanges(); // make sure we save changes before we remove the change tracker (really, the app shouldn't have left them uncommitted!)
        }
    Destroy();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDb::_OnDbClose() 
    {
    Domains().OnDbClose();
    Destroy(); 
    T_Super::_OnDbClose();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::_OnDbOpened()
    {
    DbResult rc = T_Super::_OnDbOpened();
    if (BE_SQLITE_OK != rc)
        return rc;

    Txns(); // make sure txnmanager is allocated

    m_units.Load();
    return Domains().OnDbOpened();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
TxnManagerR DgnDb::Txns()
    {
    if (!m_txnManager.IsValid())
        m_txnManager = new TxnManager(*this);

    return *m_txnManager;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   02/15
//+---------------+---------------+---------------+---------------+---------------+------
CachedECSqlStatementPtr DgnDb::GetPreparedECSqlStatement(Utf8CP ecsql) const
    {
    return m_ecsqlCache.GetPreparedStatement(*this, ecsql);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::DoOpenDgnDb(BeFileNameCR projectNameIn, OpenParams const& params)
    {
    BeFileName fileName(projectNameIn);
    fileName.SupplyDefaultNameParts(s_dgndbExt);
    m_fileName = fileName.GetNameUtf8();

    DbResult stat = OpenBeSQLiteDb(fileName, params);
    if (BE_SQLITE_OK != stat)
        {
        LOG.errorv("Error %s opening [%s]", Db::InterpretDbResult(stat), m_fileName.c_str());
        }

    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr DgnDb::OpenDgnDb(DbResult* outResult, BeFileNameCR fileName, OpenParams const& openParams)
    {
    DbResult ALLOW_NULL_OUTPUT(status, outResult);
    bool wantReadonly = openParams.IsReadonly();

    BeFileName dbFileName(fileName);
    dbFileName.SupplyDefaultNameParts(s_dgndbExt);

    DgnDbPtr dgnDb = new DgnDb();

    status = dgnDb->DoOpenDgnDb(dbFileName, openParams);
    if (status != BE_SQLITE_OK)
        return nullptr;

    // SchemaUpgrade logic may call OpenParams::_ReopenForSchemaUpgrade changing the file
    // from Readonly to ReadWrite.  This changes it back to what the caller requested.
    if (!wantReadonly || openParams.IsReadonly())
        return dgnDb;

    dgnDb = new DgnDb(); // release old and create a new DgnDb
    OpenParams readonlyParams(openParams);
    readonlyParams.SetOpenMode(Db::OpenMode::Readonly);
    status = dgnDb->DoOpenDgnDb(dbFileName, readonlyParams);
    return (status != BE_SQLITE_OK) ? nullptr : dgnDb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateNewDgnDb(BeFileNameCR inFileName, CreateDgnDbParams const& params)
    {
    BeFileName projectFile(inFileName);

    if (inFileName.IsEmpty())
        {
        projectFile.SetNameUtf8(BEDB_MemoryDb);
        }
    else
        {
        projectFile.SupplyDefaultNameParts(s_dgndbExt);
        if (params.m_overwriteExisting && BeFileName::DoesPathExist(projectFile))
            {
            if (BeFileNameStatus::Success != BeFileName::BeDeleteFile(projectFile))
                {
                LOG.errorv("Unable to create DgnDb because '%s' cannot be deleted.", projectFile.GetNameUtf8().c_str());
                return BE_SQLITE_ERROR_FileExists;
                }
            }
        }

    bool useSeedDb = !params.m_seedDb.empty();

    if (useSeedDb)
        {
        BeFileNameStatus status = BeFileName::BeCopyFile(params.m_seedDb.c_str(), projectFile);
        if (BeFileNameStatus::Success != status)
            return BE_SQLITE_ERROR_FileExists;
        }

    DbResult rc = CreateNewDb(projectFile, params.GetGuid(), params);
    if (BE_SQLITE_OK != rc)
        return rc;

    m_fileName = projectFile.GetNameUtf8();

    rc = CreateDgnDbTables();
    if (BE_SQLITE_OK != rc)
        return rc;

    InitializeDgnDb(params);

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr DgnDb::CreateDgnDb(DbResult* result, BeFileNameCR fileName, CreateDgnDbParams const& params)
    {
    DbResult ALLOW_NULL_OUTPUT(stat, result);

    DgnDbPtr dgndb = new DgnDb();
    stat = dgndb->CreateNewDgnDb(fileName, params);

    return (BE_SQLITE_OK==stat) ? dgndb : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnDb::CompactFile()
    {
    if (1 < GetCurrentSavepointDepth())
        return  DgnDbStatus::TransactionActive;

    Savepoint* savepoint = GetSavepoint(0);
    if (savepoint)
        savepoint->Commit(nullptr);

    DbResult rc= TryExecuteSql("VACUUM");

    if (savepoint)
        savepoint->Begin();

    return BE_SQLITE_OK != rc ? DgnDbStatus::SQLiteError : DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnStyles::DgnStyles(DgnDbR project) : DgnDbTable(project), m_lineStyles(nullptr), m_annotationTextStyles(nullptr), m_annotationFrameStyles(nullptr),
            m_annotationLeaderStyles(nullptr), m_textAnnotationSeeds(nullptr)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnStyles::~DgnStyles()
    {
    DELETE_AND_CLEAR(m_lineStyles);
    DELETE_AND_CLEAR(m_annotationTextStyles);
    DELETE_AND_CLEAR(m_annotationFrameStyles);
    DELETE_AND_CLEAR(m_annotationLeaderStyles);
    DELETE_AND_CLEAR(m_textAnnotationSeeds);
    }

DgnLineStyles& DgnStyles::LineStyles() {if (NULL == m_lineStyles) m_lineStyles = new DgnLineStyles(m_dgndb); return *m_lineStyles;}
DgnAnnotationTextStyles& DgnStyles::AnnotationTextStyles() {if (NULL == m_annotationTextStyles) m_annotationTextStyles = new DgnAnnotationTextStyles(m_dgndb); return *m_annotationTextStyles;}
DgnAnnotationFrameStyles& DgnStyles::AnnotationFrameStyles() {if (NULL == m_annotationFrameStyles) m_annotationFrameStyles = new DgnAnnotationFrameStyles(m_dgndb); return *m_annotationFrameStyles;}
DgnAnnotationLeaderStyles& DgnStyles::AnnotationLeaderStyles() {if (NULL == m_annotationLeaderStyles) m_annotationLeaderStyles = new DgnAnnotationLeaderStyles(m_dgndb); return *m_annotationLeaderStyles;}
DgnTextAnnotationSeeds& DgnStyles::TextAnnotationSeeds() {if (NULL == m_textAnnotationSeeds) m_textAnnotationSeeds = new DgnTextAnnotationSeeds(m_dgndb); return *m_textAnnotationSeeds;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnScriptLibrary::RegisterScript(Utf8CP tsProgramName, Utf8CP tsProgramText, DgnScriptType stype, bool updateExisting)
    {
    Statement stmt;
    stmt.Prepare(m_dgndb, SqlPrintfString(
        "INSERT %s INTO be_Prop (Namespace,Name,Id,SubId,TxnMode,StrData) " 
                       "VALUES('dgn_Script',?,?,?,0,?)", updateExisting ? "OR REPLACE" : ""));
    stmt.BindText(1, tsProgramName, Statement::MakeCopy::No);
    stmt.BindInt(2, 0);
    stmt.BindInt(3, (int)stype);
    stmt.BindText(4, tsProgramText, Statement::MakeCopy::No);
    DbResult res = stmt.Step();
    
    NativeLogging::LoggingManager::GetLogger("DgnScript")->infov ("Registering %s -> %d", tsProgramName, res);
    
    return (BE_SQLITE_DONE == res)? DgnDbStatus::Success: DgnDbStatus::SQLiteError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnScriptLibrary::QueryScript(Utf8StringR sText, DgnScriptType& stypeFound, Utf8CP sName, DgnScriptType stypePreferred)
    {
    Statement stmt;
    stmt.Prepare(m_dgndb, "SELECT StrData,SubId FROM be_Prop WHERE (Namespace='dgn_Script' AND Name=? AND Id=?)");
    stmt.BindText(1, sName, Statement::MakeCopy::No);
    stmt.BindInt(2, 0);
    if (stmt.Step() != BE_SQLITE_ROW)
        return DgnDbStatus::NotFound;

    do  {
        sText = stmt.GetValueText(0);
        stypeFound = (DgnScriptType)stmt.GetValueInt(1);
        } while ((stypeFound != stypePreferred) && (stmt.Step() == BE_SQLITE_ROW));

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnScriptLibrary::ReadText(Utf8StringR jsprog, BeFileNameCR jsFileName)
    {
    uint64_t fileSize;
    if (jsFileName.GetFileSize(fileSize) != BeFileNameStatus::Success)
        return DgnDbStatus::NotFound;

    size_t bufSize = (size_t)fileSize;
    jsprog.resize(bufSize+1);
    FILE* fp = fopen(Utf8String(jsFileName).c_str(), "r");
    size_t nread = fread(&jsprog[0], 1, bufSize, fp);
    BeAssert(nread <= bufSize);
    fclose(fp);
    jsprog[nread] = 0;
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnClassId DgnImportContext::RemapClassId(DgnClassId source)
    {
    if (!IsBetweenDbs())
        return source;

    DgnClassId dest = m_remap.Find(source);
    if (dest.IsValid())
        return dest;

    ECClassCP sourceecclass = GetSourceDb().Schemas().GetECClass(source.GetValue());
    if (nullptr == sourceecclass)
        return DgnClassId();

    ECClassCP destecclass = GetDestinationDb().Schemas().GetECClass(sourceecclass->GetSchema().GetName().c_str(), sourceecclass->GetName().c_str());
    if (nullptr == destecclass)
        return DgnClassId();

    return m_remap.Add(source, DgnClassId(destecclass->GetId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnImportContext::DgnImportContext(DgnDbR source, DgnDbR dest) : m_sourceDb(source), m_destDb(dest) 
    {
    DgnGCS* sourceGcs = m_sourceDb.Units().GetDgnGCS();
    DgnGCS* destGcs = m_destDb.Units().GetDgnGCS();

    m_xyOffset = DPoint2d::FromZero();
    m_yawAdj = AngleInDegrees::FromDegrees(0);

    if (!IsBetweenDbs() || nullptr == sourceGcs || nullptr == destGcs)
        {
        m_areCompatibleDbs = true;
        return;
        }

    WString spn, dpn;
    if (0 != wcscmp(sourceGcs->GetProjectionName(spn), destGcs->GetProjectionName(dpn)))
        {
        m_areCompatibleDbs = false;
        return;
        }

    GeoPoint sourceOrgLatLng;
    if (REPROJECT_Success != sourceGcs->LatLongFromUors(sourceOrgLatLng, DPoint3d::FromZero()))
        {
        m_areCompatibleDbs = false;
        return;
        }
    DPoint3d destCoordinates;
    if (REPROJECT_Success != destGcs->UorsFromLatLong(destCoordinates, sourceOrgLatLng))
        {
        m_areCompatibleDbs = false;
        return;
        }
    
    if (0 != BeNumerical::Compare(0.0, destCoordinates.z))
        {
        BeDataAssert(false && "different elevations??");
        m_areCompatibleDbs = false;
        return;
        }

    m_xyOffset = DPoint2d::From(destCoordinates.x, destCoordinates.y);
    m_yawAdj = AngleInDegrees::FromRadians(destGcs->GetAzimuth() - sourceGcs->GetAzimuth());
    }
