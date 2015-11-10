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
DgnDb::DgnDb() : m_schemaVersion(0,0,0,0), m_fonts(*this, DGN_TABLE_Font), m_domains(*this), m_styles(*this), m_views(*this),
                 m_geomParts(*this), m_units(*this), m_models(*this), m_elements(*this), 
                 m_links(*this), m_authorities(*this), m_ecsqlCache(50, "DgnDb"), m_searchableText(*this), m_revisionManager(nullptr)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDb::Destroy()
    {
    m_models.Empty();
    m_txnManager = nullptr; // RefCountedPtr, deletes TxnManager
    if (nullptr != m_revisionManager)
        {
        delete m_revisionManager;
        m_revisionManager = nullptr;
        }
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

    Fonts().Update(); // ensure the font ID cache is loaded; if you wait for on-demand, it may need to query during an update, which we'd like to avoid

    m_units.Load();
    
    if (BE_SQLITE_OK != (rc = Domains().OnDbOpened()))
        return rc;

    if (BE_SQLITE_OK != (rc = Txns().InitializeTableHandlers())) // make sure txnmanager is allocated and that all txn-related temp tables are created
        return rc;

    return BE_SQLITE_OK;
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ILocksManagerR DgnDb::Locks()
    {
    // This is here rather than in the constructor because _CreateLocksManager() requires briefcase ID, which is obtained from m_dbFile,
    // which is not initialized in constructor.
    if (m_locksManager.IsNull())
        {
        m_locksManager = T_HOST.GetLocksAdmin()._CreateLocksManager(*this);
        BeAssert(m_locksManager.IsValid());
        }

    return *m_locksManager;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Ramanujam.Raman                    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RevisionManagerR DgnDb::Revisions() const
    {
    if (nullptr == m_revisionManager)
        m_revisionManager = new RevisionManager(const_cast<DgnDbR>(*this));

    return *m_revisionManager;
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
DgnStyles::DgnStyles(DgnDbR project) : DgnDbTable(project), m_lineStyles(nullptr)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnStyles::~DgnStyles()
    {
    DELETE_AND_CLEAR(m_lineStyles);
    }

DgnLineStyles& DgnStyles::LineStyles() {if (NULL == m_lineStyles) m_lineStyles = new DgnLineStyles(m_dgndb); return *m_lineStyles;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnScriptLibrary::RegisterScript(Utf8CP tsProgramName, Utf8CP tsProgramText, DgnScriptType stype, bool updateExisting)
    {
    DbEmbeddedFileTable& files = GetDgnDb().EmbeddedFiles();
    DbResult res = files.AddEntry(tsProgramName, (DgnScriptType::JavaScript == stype)? "js": "ts");
    if (BE_SQLITE_OK != res)
        {
        if (!BeSQLiteLib::IsConstraintDbResult(res) || !updateExisting)
            {
            return DgnDbStatus::SQLiteError;
            }
        }

    if (BE_SQLITE_OK != files.Save(tsProgramText, strlen(tsProgramText)+1, tsProgramName, true))
        return DgnDbStatus::SQLiteError;
    
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnScriptLibrary::QueryScript(Utf8StringR sText, DgnScriptType& stypeFound, Utf8CP sName, DgnScriptType stypePreferred)
    {
    DbEmbeddedFileTable& files = GetDgnDb().EmbeddedFiles();
    uint64_t size;
    Utf8String ftype;
    auto id = files.QueryFile(sName, &size, nullptr, nullptr, &ftype, nullptr);
    if (!id.IsValid())
        return DgnDbStatus::NotFound;

    if (ftype.EqualsI("js"))
        stypeFound = DgnScriptType::JavaScript;
    else
        stypeFound = DgnScriptType::TypeScript;

    bvector<Byte> chars;
    if (BE_SQLITE_OK != files.Read(chars, sName))
        return DgnDbStatus::NotFound;

    Utf8CP str = (Utf8CP)&chars[0];
    sText.assign(str, str+chars.size());
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

    BeFile file;
    if (BeFileStatus::Success != file.Open(jsFileName, BeFileAccess::Read))
        return DgnDbStatus::NotFound;

    size_t bufSize = (size_t)fileSize;
    jsprog.resize(bufSize+1);
    uint32_t nread;
    if (BeFileStatus::Success != file.Read(&jsprog[0], &nread, (uint32_t)fileSize))
        return DgnDbStatus::ReadError;

    BeAssert(nread <= bufSize);
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
void DgnImportContext::ComputeGcsAdjustment()
    {
    //  We may need to transform between source and destination GCS.
    m_xyOffset = DPoint2d::FromZero();
    m_yawAdj = AngleInDegrees::FromDegrees(0);
    m_areCompatibleDbs = true;

    if (!IsBetweenDbs())
        return;

    DgnGCS* sourceGcs = m_sourceDb.Units().GetDgnGCS();
    DgnGCS* destGcs = m_destDb.Units().GetDgnGCS();

    if (nullptr == sourceGcs || nullptr == destGcs)
        {
        m_areCompatibleDbs = true;
        return;
        }

    // Check that source and destination are based on equivalent projections.
    if (!destGcs->IsEquivalent(*sourceGcs))
        {
        m_areCompatibleDbs = false;
        return;
        }

    //  Check that source and destination GCSs are at the same elevation
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

    //  We should be able to transform using a simple offset and rotation.
    m_xyOffset = DPoint2d::From(destCoordinates.x, destCoordinates.y);
    m_yawAdj = AngleInDegrees::FromRadians(destGcs->GetAzimuth() - sourceGcs->GetAzimuth());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCloneContext::DgnCloneContext()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnImportContext::DgnImportContext(DgnDbR source, DgnDbR dest) : DgnCloneContext(), m_sourceDb(source), m_destDb(dest)
    {
    ComputeGcsAdjustment();
    }

static uintptr_t  s_nextQvMaterialId;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
uintptr_t DgnDb::AddQvMaterialId(DgnMaterialId materialId) const { return (m_qvMaterialIds[materialId] = ++s_nextQvMaterialId); }
uintptr_t DgnDb::GetQvMaterialId(DgnMaterialId materialId) const
    {
    auto const&   found = m_qvMaterialIds.find(materialId);

    return (found == m_qvMaterialIds.end()) ? 0 : found->second; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
uintptr_t DgnDb::AddQvTextureId(DgnTextureId TextureId) const 
    { 
    static uintptr_t s_nextQvTextureId;
    return (m_qvTextureIds[TextureId] = ++s_nextQvTextureId); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
uintptr_t DgnDb::GetQvTextureId(DgnTextureId TextureId) const
    {
    auto const& found = m_qvTextureIds.find(TextureId);
    return (found == m_qvTextureIds.end()) ? 0 : found->second; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DictionaryModelR DgnDb::GetDictionaryModel()
    {
    // NB: Once loaded, a model is never dropped unless it is deleted (or its creation is undone). This cannot occur for dictionary model so returning a reference is safe
    DictionaryModelPtr dict = Models().Get<DictionaryModel>(DgnModel::DictionaryId());
    BeAssert(dict.IsValid() && "A DgnDb always has a dictionary model");
    return *dict;
    }

