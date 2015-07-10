/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnDb.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

#define WSTR(astr) WString(astr,BentleyCharEncoding::Utf8).c_str()

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
                 m_materials(*this), m_links(*this), m_ecsqlCache(50, "DgnDb")
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
* Reclaims disk space by vacuuming the database
* @bsimethod                                                    KeithBentley    12/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnDb::CompactFile()
    {
    if (1 < GetCurrentSavepointDepth())
        return  DgnDbStatus::TransactionActive;

    Savepoint* savepoint = GetSavepoint(0);
    if (savepoint)
        savepoint->Commit(nullptr);

    if (BE_SQLITE_OK != TryExecuteSql("VACUUM"))
        return  DgnDbStatus::SQLiteError;
    
    if (savepoint)
        savepoint->Begin();

    return DgnDbStatus::Success;
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
BentleyStatus DgnJavaScriptLibrary::ToJsonFromEC(Json::Value& json, ECN::IECInstanceCR ec, Utf8CP prop)
    {
    ECN::ECValue v;
    if (ec.GetValue(v, WSTR(prop)) != ECN::ECOBJECTS_STATUS_Success || v.IsNull() || !v.IsPrimitive())
        return BSIERROR;

    auto& jv = json[prop];

    switch(v.GetPrimitiveType())
        {
        case ECN::PRIMITIVETYPE_Boolean:    jv = v.GetBoolean(); break;
        case ECN::PRIMITIVETYPE_Double:     jv = v.GetDouble(); break;
        case ECN::PRIMITIVETYPE_Integer:    jv = v.GetInteger(); break;
        case ECN::PRIMITIVETYPE_Long:       jv = v.GetLong(); break;
        case ECN::PRIMITIVETYPE_String:     jv = Utf8String(v.GetString()).c_str(); break;
        
        case ECN::PRIMITIVETYPE_Point2D:    JsonUtils::DPoint2dToJson(jv, v.GetPoint2D()); break;
        case ECN::PRIMITIVETYPE_Point3D:    JsonUtils::DPoint3dToJson(jv, v.GetPoint3D()); break;

        /* WIP_EGA 
        case ECN::PRIMITIVETYPE_IGeometry:  jv = ...
        case ECN::PRIMITIVETYPE_DateTime:   jv = v.GetDateTime(); break;
        */

        default:
            return BSIERROR;
        }
    
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
BentleyStatus DgnJavaScriptLibrary::ToJsonFromEC(Json::Value& json, ECN::IECInstanceCR ec, Utf8StringCR props)
    {
    size_t offset = 0;
    Utf8String parm;
    while ((offset = props.GetNextToken (parm, ",", offset)) != Utf8String::npos)
        {
        parm.Trim();
        if (ToJsonFromEC(json, ec, parm.c_str()) != BSISUCCESS)
            return BSIERROR;
        }
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
static BentleyStatus registerJavaScript(DgnDbR db, Utf8CP tsProgramName, Utf8CP tsProgramText, bool updateExisting)
    {
    Statement stmt;
    stmt.Prepare(db, SqlPrintfString(
        "INSERT %s INTO be_Prop (Namespace,     Name, Id, SubId, TxnMode, StrData) " 
                       "VALUES('dgn_JavaScript',?,    ?,  ?,      0,       ?)", updateExisting? "OR REPLACE": ""));
    stmt.BindText(1, tsProgramName, Statement::MakeCopy::No);
    stmt.BindInt(2, 0);
    stmt.BindInt(3, 0);
    stmt.BindText(4, tsProgramText, Statement::MakeCopy::No);
    DbResult res = stmt.Step();
    
    NativeLogging::LoggingManager::GetLogger("DgnScriptContext")->infov ("Registering %s -> %d", tsProgramName, res);
    
    return (BE_SQLITE_DONE == res)? BSISUCCESS: BSIERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
BentleyStatus DgnJavaScriptLibrary::QueryJavaScript(Utf8StringR tsProgramText, Utf8CP tsProgramName)
    {
    Statement stmt;
    stmt.Prepare(m_dgndb, "SELECT StrData FROM be_Prop WHERE (Namespace='dgn_JavaScript' AND Name=? AND Id=? AND SubId=?)");
    stmt.BindText(1, tsProgramName, Statement::MakeCopy::No);
    stmt.BindInt(2, 0);
    stmt.BindInt(3, 0);
    if (stmt.Step() != BE_SQLITE_ROW)
        return BSIERROR;
    tsProgramText = stmt.GetValueText(0);
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void DgnJavaScriptLibrary::ImportJavaScript(BeFileNameCR jsDir, bool updateExisting)
    {
    BeFileName jsFiles(jsDir);
    jsFiles.AppendToPath(L"*.js");
    BeFileListIterator iter(jsFiles.GetName(), false);
    BeFileName acmeTs;
    bool anyImported = true;
    while (BSISUCCESS == iter.GetNextFileName(acmeTs))
        {
        uint64_t fileSize;
        if (acmeTs.GetFileSize(fileSize) == BeFileNameStatus::Success)
            {
            size_t bufSize = (size_t)fileSize;
            ScopedArray<char> buf (bufSize+1);
            FILE* fp = fopen(Utf8String(acmeTs).c_str(), "r");
            size_t nread = fread(buf.GetData(), 1, bufSize, fp);
            BeAssert(nread <= bufSize);
            fclose(fp);
            buf.GetData()[nread] = 0;
            Utf8String name (acmeTs.GetFileNameWithoutExtension());
            if (BSISUCCESS == registerJavaScript(m_dgndb, name.c_str(), buf.GetDataCP(), updateExisting))
                anyImported = true;
            }
        }
    if (anyImported)
        m_dgndb.SaveSettings();
    }
