/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnDb.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <Bentley/BeTest.h> // *** WIP_TEST_PERFORMANCE_PROJECT - this is temporary. Remove when we have cleaned up unit tests
#include <DgnPlatform/DgnGeoCoord.h>

static WCharCP s_dgndbExt   = L".bim";

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
DgnDb::DgnDb() : m_schemaVersion(0,0,0,0), m_fonts(*this, DGN_TABLE_Font), m_domains(*this), m_lineStyles(new DgnLineStyles(*this)),
                 m_geoLocation(*this), m_models(*this), m_elements(*this), m_sessionManager(*this),
                 m_codeSpecs(*this), m_ecsqlCache(50, "DgnDb"), m_searchableText(*this), m_sceneQueue(*this)
    {
    m_memoryManager.AddConsumer(m_elements, MemoryConsumer::Priority::Highest);
    m_eccrudWriteToken = nullptr;
    //uncomment this (and remove line above) once API for modifying Aspects has been implemented
    //m_eccrudWriteToken = &T_Super::EnableECCrudWriteTokenValidation();

    m_dbSchemaModificationToken = &T_Super::EnableDbSchemaModificationTokenValidation();
    }

//--------------------------------------------------------------------------------------
//not inlined as it must not be called externally
// @bsimethod                                Krischan.Eberle                11/2016
//---------------+---------------+---------------+---------------+---------------+------
ECCrudWriteToken const* DgnDb::GetECCrudWriteToken() const {return m_eccrudWriteToken;}

//--------------------------------------------------------------------------------------
//Back door for converter
// @bsimethod                                Krischan.Eberle                11/2016
//---------------+---------------+---------------+---------------+---------------+------
extern "C" DGNPLATFORM_EXPORT void* dgnV8Converter_getToken(DgnDbR db)
    {
    return (void*)db.GetECCrudWriteToken();
    }

//--------------------------------------------------------------------------------------
//not inlined as it must not be called externally
// @bsimethod                                Krischan.Eberle                12/2016
//---------------+---------------+---------------+---------------+---------------+------
DbSchemaModificationToken const* DgnDb::GetDbSchemaModificationToken() const {return m_dbSchemaModificationToken;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDb::Destroy()
    {
    m_sessionManager.ClearCurrent();
    m_sceneQueue.Terminate();
    m_models.Empty();
    m_txnManager = nullptr; // RefCountedPtr, deletes TxnManager
    m_lineStyles = nullptr;
    m_revisionManager.reset(nullptr);
    m_ecsqlCache.Empty();
    if (m_briefcaseManager.IsValid())
        {
        m_briefcaseManager->OnDgnDbDestroyed();
        m_briefcaseManager = nullptr;
        }
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
* @bsimethod                                    Sam.Wilson                      02/15
+---------------+---------------+---------------+---------------+---------------+------*/
static DbResult checkIsConvertibleVersion(Db& db)
    {
    Utf8String versionString;
    DbResult rc = db.QueryProperty(versionString, DgnProjectProperty::SchemaVersion());
    if (BE_SQLITE_ROW != rc)
        return BE_SQLITE_ERROR_InvalidProfileVersion;

    SchemaVersion sver(0,0,0,0);
    sver.FromJson(versionString.c_str());
    if (sver.GetMajor() < DGNDB_CURRENT_VERSION_Major)
        return BE_SQLITE_ERROR_ProfileTooOld;

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::_OnDbOpened()
    {
    DbResult rc = checkIsConvertibleVersion(*this);
    if (BE_SQLITE_OK != rc)
        {
        // Short-circuit this function if the Db is too old or new such that we cannot continue;
        //  The caller does the version check/upgrade after this _OnDbOpened logic finishes. 
        //  If we have missing tables and columns, however, then we cannot even execute this _OnDbOpened logic.
        //  *** NEEDS WORK: Do we need some kind of "pre" version upgrade?
        //  In any case, we don't intend to upgrade from 05 to 06, so it's a moot point for now.
        return rc;
        }

    if (BE_SQLITE_OK != (rc = T_Super::_OnDbOpened()))
        return rc;

    if (BE_SQLITE_OK != (rc = Domains().OnDbOpened()))
        return rc;

    if (BE_SQLITE_OK != (rc = Txns().InitializeTableHandlers())) // make sure txnmanager is allocated and that all txn-related temp tables are created. 
        return rc;                                               // NB: InitializeTableHandlers calls SaveChanges!

    Fonts().Update(); // ensure the font Id cache is loaded; if you wait for on-demand, it may need to query during an update, which we'd like to avoid
    m_geoLocation.Load();

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
IBriefcaseManagerR DgnDb::BriefcaseManager()
    {
    // This is here rather than in the constructor because _CreateBriefcaseManager() requires briefcase ID, which is obtained from m_dbFile,
    // which is not initialized in constructor.
    if (m_briefcaseManager.IsNull())
        {
        m_briefcaseManager = T_HOST.GetRepositoryAdmin()._CreateBriefcaseManager(*this);
        BeAssert(m_briefcaseManager.IsValid());
        }

    return *m_briefcaseManager;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Ramanujam.Raman                    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RevisionManagerR DgnDb::Revisions() const
    {
    if (nullptr == m_revisionManager)
        m_revisionManager.reset(new RevisionManager(const_cast<DgnDbR>(*this)));

    return *m_revisionManager;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   02/15
//+---------------+---------------+---------------+---------------+---------------+------
CachedECSqlStatementPtr DgnDb::GetPreparedECSqlStatement(Utf8CP ecsql) const
    {
    return m_ecsqlCache.GetPreparedStatement(*this, ecsql);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   11/16
//+---------------+---------------+---------------+---------------+---------------+------
CachedECSqlStatementPtr DgnDb::GetNonSelectPreparedECSqlStatement(Utf8CP ecsql, ECCrudWriteToken const* writeToken) const
    {
    return m_ecsqlCache.GetPreparedStatement(*this, ecsql, writeToken);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   11/16
//+---------------+---------------+---------------+---------------+---------------+------
DbResult DgnDb::InsertECRelationship(BeSQLite::EC::ECInstanceKey& relKey, ECN::ECRelationshipClassCR relClass, BeSQLite::EC::ECInstanceId sourceId, 
                                     BeSQLite::EC::ECInstanceId targetId, ECN::IECRelationshipInstanceCP relInstanceProperties)
    {
    //WIP this might need a cache of inserters if called often
    ECInstanceInserter inserter(*this, relClass, GetECCrudWriteToken());
    if (!inserter.IsValid())
        return BE_SQLITE_ERROR;

    return inserter.InsertRelationship(relKey, sourceId, targetId, relInstanceProperties);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/16
//--------------+---------------+---------------+---------------+---------------+------
DbResult DgnDb::UpdateECRelationshipProperties(EC::ECInstanceKeyCR key, ECN::IECInstanceR props)
    {
    auto eclass = Schemas().GetECClass(key.GetECClassId());
    if (nullptr == eclass)
        return DbResult::BE_SQLITE_ERROR;
    auto updater = Elements().m_updaterCache.GetUpdater(*this, *eclass);
    if (nullptr == updater)
        return DbResult::BE_SQLITE_ERROR;
    Utf8Char instidstr[32];
    BeStringUtilities::FormatUInt64(instidstr, key.GetECInstanceId().GetValue());
    props.SetInstanceId(instidstr);
    return updater->Update(props);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/16
//--------------+---------------+---------------+---------------+---------------+------
DbResult DgnDb::DeleteECRelationship(EC::ECInstanceKeyCR key)
    {
    auto eclass = Schemas().GetECClass(key.GetECClassId());
    if (nullptr == eclass)
        return DbResult::BE_SQLITE_ERROR;

    Utf8String ecsql("DELETE FROM ");
    ecsql.append(eclass->GetECSqlName().c_str()).append(" WHERE ECInstanceId=?");

    CachedECSqlStatementPtr stmt = GetNonSelectPreparedECSqlStatement(ecsql.c_str(), GetECCrudWriteToken());
    if (stmt == nullptr)
        return BE_SQLITE_ERROR;

    stmt->BindId(1, key.GetECInstanceId());
    return stmt->Step();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   11/16
//+---------------+---------------+---------------+---------------+---------------+------
DbResult DgnDb::DeleteECRelationships(Utf8CP relClassECSqlName, ECInstanceId sourceId, ECInstanceId targetId)
    {
    if (!sourceId.IsValid() && !targetId.IsValid())
        {
        BeAssert(false && "SourceId and TargetId cannot both be invalid");
        return BE_SQLITE_ERROR;
        }

    Utf8String ecsql("DELETE FROM ");
    ecsql.append(relClassECSqlName).append(" WHERE ");

    if (sourceId.IsValid())
        {
        ecsql.append("SourceECInstanceId=?");
        if (targetId.IsValid())
            ecsql.append(" AND ");
        }

    if (targetId.IsValid())
        ecsql.append("TargetECInstanceId=?");

    CachedECSqlStatementPtr stmt = GetNonSelectPreparedECSqlStatement(ecsql.c_str(), GetECCrudWriteToken());
    if (stmt == nullptr)
        return BE_SQLITE_ERROR;

    int parameterIndex = 1;
    if (sourceId.IsValid())
        {
        if (ECSqlStatus::Success != stmt->BindId(parameterIndex, sourceId))
            return BE_SQLITE_ERROR;

        ++parameterIndex;
        }

    if (targetId.IsValid())
        {
        if (ECSqlStatus::Success != stmt->BindId(parameterIndex, targetId))
            return BE_SQLITE_ERROR;
        }

    const DbResult stat = stmt->Step();
    return BE_SQLITE_DONE == stat ? BE_SQLITE_OK : stat;
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

    if (BeTest::IsInitialized())                                        // *** WIP_TEST_PERFORMANCE_PROJECT - this is temporary. Remove when we have cleaned up unit tests
        wprintf(L"!!!!!!!!!!!!!!!!!! DgnDb::CreateNewDgnDb %s\n", inFileName.c_str());     // *** WIP_TEST_PERFORMANCE_PROJECT - this is temporary. Remove when we have cleaned up unit tests

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

    rc = CreateDgnDbTables(params);
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

    if (params.m_rootSubjectName.empty())
        {
        BeAssert(false); // required to create the root Subject in the RepositoryModel
        return nullptr;
        }

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
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnScriptLibrary::RegisterScript(Utf8CP tsProgramName, Utf8CP tsProgramText, DgnScriptType stype, DateTime const& lastModifiedTime, bool updateExisting)
    {
    DbEmbeddedFileTable& files = GetDgnDb().EmbeddedFiles();
    DbResult res = files.AddEntry(tsProgramName, (DgnScriptType::JavaScript == stype)? "js": "ts", nullptr, &lastModifiedTime);
    if (BE_SQLITE_OK != res)
        {
        if (!BeSQLiteLib::IsConstraintDbResult(res) || !updateExisting)
            {
            return DgnDbStatus::SQLiteError;
            }
        }

    if (BE_SQLITE_OK != files.Save(tsProgramText, strlen(tsProgramText)+1, tsProgramName, &lastModifiedTime, true))
        return DgnDbStatus::SQLiteError;
    
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnScriptLibrary::QueryScript(Utf8StringR sText, DgnScriptType& stypeFound, DateTime& lastModifiedTime, Utf8CP sName, DgnScriptType stypePreferred)
    {
    DbEmbeddedFileTable& files = GetDgnDb().EmbeddedFiles();
    uint64_t size;
    Utf8String ftype;
    auto id = files.QueryFile(sName, &size, nullptr, nullptr, &ftype, &lastModifiedTime);
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
DgnClassId DgnImportContext::_RemapClassId(DgnClassId source)
    {
    if (!IsBetweenDbs())
        return source;

    DgnClassId dest = m_remap.Find(source);
    if (dest.IsValid())
        return dest;

    ECClassCP sourceecclass = GetSourceDb().Schemas().GetECClass(source);
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
void DgnImportContext::ComputeGcsAndGOadjustment()
    {
    //  We may need to transform between source and destination GCS.
    m_xyzOffset = DPoint3d::FromZero();
    m_yawAdj = AngleInDegrees::FromDegrees(0);
    m_areCompatibleDbs = true;

    if (!IsBetweenDbs())
        return;

    DPoint3dCR sourceGO(m_sourceDb.GeoLocation().GetGlobalOrigin());
    DPoint3dCR destGO(m_destDb.GeoLocation().GetGlobalOrigin());

    m_xyzOffset.DifferenceOf(destGO, sourceGO);

    DgnGCS* sourceGcs = m_sourceDb.GeoLocation().GetDgnGCS();
    DgnGCS* destGcs = m_destDb.GeoLocation().GetDgnGCS();

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
    ComputeGcsAndGOadjustment();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnImportContext::~DgnImportContext()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryModelPtr DgnDb::GetRepositoryModel()
    {
    RepositoryModelPtr model = Models().Get<RepositoryModel>(DgnModel::RepositoryModelId());
    BeAssert(model.IsValid() && "A DgnDb always has a " BIS_CLASS_RepositoryModel);
    return model;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DictionaryModelR DgnDb::GetDictionaryModel()
    {
    // NB: Once loaded, a model is never dropped unless it is deleted (or its creation is undone). This cannot occur for dictionary model so returning a reference is safe
    DictionaryModelPtr dict = Models().Get<DictionaryModel>(DgnModel::DictionaryId());
    BeAssert(dict.IsValid() && "A DgnDb always has a " BIS_CLASS_DictionaryModel);
    return *dict;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
SessionModelPtr DgnDb::GetSessionModel()
    {
    DefinitionPartitionCPtr partition = Elements().Get<DefinitionPartition>(Elements().GetSessionPartitionId());
    BeAssert(partition.IsValid() && "A DgnDb always has a sessions partition");
    SessionModelPtr model = Models().Get<SessionModel>(partition->GetSubModelId());
    BeAssert(model.IsValid() && "A DgnDb always has a " BIS_CLASS_SessionModel);
    return model;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
LinkModelPtr DgnDb::GetRealityDataSourcesModel()
    {
    LinkPartitionCPtr partition = Elements().Get<LinkPartition>(Elements().GetRealityDataSourcesPartitionId());
    BeAssert(partition.IsValid() && "A DgnDb always has a reality data sources partition");
    LinkModelPtr model = Models().Get<LinkModel>(partition->GetSubModelId());
    BeAssert(model.IsValid() && "A DgnDb always has a reality data sources model");
    return model;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECSqlStatement* ECSqlStatementIteratorBase::PrepareStatement(DgnDbCR dgndb, Utf8CP ecSql, uint32_t idSelectColumnIndex)
    {
    m_statement = dgndb.GetPreparedECSqlStatement(ecSql);
    if (m_statement.IsNull())
        {
        BeAssert(false);
        return nullptr;
        }

    m_isAtEnd = false;
    m_idSelectColumnIndex = (int) idSelectColumnIndex;
    return m_statement.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSqlStatementIteratorBase::IsEqual(ECSqlStatementIteratorBase const& rhs) const
    {
    if (m_isAtEnd && rhs.m_isAtEnd)
        return true;
    if (m_isAtEnd != rhs.m_isAtEnd)
        return false;

    BeAssert(m_statement.IsValid() && rhs.m_statement.IsValid());
    ECInstanceId thisId = m_statement->GetValueId<ECInstanceId>(m_idSelectColumnIndex);
    
    // Do NOT delete the next line and simply use rhs.m_statement on the subsequent.
    // Android GCC 4.9 and clang 6.1.0 cannot deduce the templates when you try to combine it all up.
    CachedECSqlStatementPtr rhsStatement = rhs.m_statement;
    ECInstanceId rhsId = rhsStatement->GetValueId<ECInstanceId>(rhs.m_idSelectColumnIndex);
    
    return thisId == rhsId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSqlStatementIteratorBase::MoveNext()
    {
    if (m_isAtEnd)
        {
        BeAssert(false && "Do not attempt to iterate beyond the end of the instances.");
        return;
        }
    DbResult stepStatus = m_statement->Step();
    BeAssert(stepStatus == BE_SQLITE_ROW || stepStatus == BE_SQLITE_DONE);
    if (stepStatus != BE_SQLITE_ROW)
        m_isAtEnd = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSqlStatementIteratorBase::MoveFirst()
    {
    if (!m_statement.IsValid())
        {
        m_isAtEnd = true;
        return;
        }

    m_statement->Reset();
    m_isAtEnd = false;
    MoveNext();
    }

