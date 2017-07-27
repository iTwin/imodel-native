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

#ifndef NDEBUG
#define CHECK_NON_NAVIGATION_PROPERTY_API
#endif

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
DgnDb::DgnDb() : m_profileVersion(0,0,0,0), m_fonts(*this, DGN_TABLE_Font), m_domains(*this), m_lineStyles(new DgnLineStyles(*this)),
                 m_geoLocation(*this), m_models(*this), m_elements(*this),
                 m_codeSpecs(*this), m_ecsqlCache(50, "DgnDb"), m_searchableText(*this), m_elementIdSequence(*this, "bis_elementidsequence")
    {
    m_memoryManager.AddConsumer(m_elements, MemoryConsumer::Priority::Highest);

    ApplyECDbSettings(true /* requireECCrudWriteToken */, true /* requireECSchemaImportToken */ , false /* allowChangesetMergingIncompatibleECSchemaImport */ );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RealityData::CachePtr DgnDb::ElementTileCache() const
    {
    if (!m_elementTileCache.IsValid())
        {
        BeFileName  cacheName = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectoryBaseName();

        cacheName.AppendToPath(GetFileName().GetBaseName());
        cacheName.AppendExtension(L"TileCache");

        m_elementTileCache = new TileTree::TileCache(1024*1024*1024);
        if (SUCCESS != m_elementTileCache->OpenAndPrepare(cacheName))
            m_elementTileCache = nullptr;
        }
    return m_elementTileCache;
    }

//--------------------------------------------------------------------------------------
//not inlined as it must not be called externally
// @bsimethod                                Krischan.Eberle                11/2016
//---------------+---------------+---------------+---------------+---------------+------
ECCrudWriteToken const* DgnDb::GetECCrudWriteToken() const {return GetECDbSettings().GetCrudWriteToken();}

//--------------------------------------------------------------------------------------
//not inlined as it must not be called externally
// @bsimethod                                Krischan.Eberle                11/2016
//---------------+---------------+---------------+---------------+---------------+------
SchemaImportToken const* DgnDb::GetSchemaImportToken() const { return GetECDbSettings().GetSchemaImportToken(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDb::Destroy()
    {
    m_models.Empty();
    m_txnManager = nullptr; // RefCountedPtr, deletes TxnManager
    m_lineStyles = nullptr;
    m_revisionManager.reset(nullptr);
    ClearECSqlCache();
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
* @bsimethod                                    Keith.Bentley                   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::_OnDbOpened(Db::OpenParams const& params)
    {
    DbResult rc;

    if (BE_SQLITE_OK != (rc = T_Super::_OnDbOpened(params)))
        return rc;

    if (BE_SQLITE_OK != (rc = InitializeSchemas(params)))
        {
        // *** NEEDS WORK: how can we be sure that DbClose won't automatically save the partial changes?
        // Should we call AbandonChanges(); here?
        m_txnManager = nullptr; // Deletes ref counted ptr so that statement caches are freed
        return rc;
        }

    if (BE_SQLITE_OK != (rc = MergeSchemaRevisions(params))) 
        return rc;

    Fonts().Update(); // ensure the font Id cache is loaded; if you wait for on-demand, it may need to query during an update, which we'd like to avoid
    m_geoLocation.Load();
    Elements().InitLastModifiedTime();

    return BE_SQLITE_OK;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    04/17
//--------------------------------------------------------------------------------------
DbResult DgnDb::InitializeSchemas(Db::OpenParams const& params)
    {
    SchemaUpgradeOptions const& schemaUpgradeOptions = ((DgnDb::OpenParams&) params).GetSchemaUpgradeOptions();
    SchemaStatus status = Domains().InitializeSchemas(schemaUpgradeOptions);
    return SchemaStatusToDbResult(status, true /*=isUpgrade*/);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    04/17
//--------------------------------------------------------------------------------------
// static
DbResult DgnDb::SchemaStatusToDbResult(SchemaStatus status, bool isUpgrade)
    {
    switch (status)
        {
        case SchemaStatus::Success:
            return BE_SQLITE_OK;
        case SchemaStatus::SchemaTooNew:
            return BE_SQLITE_ERROR_SchemaTooNew;
        case SchemaStatus::SchemaTooOld:
            return BE_SQLITE_ERROR_SchemaTooOld;
        case SchemaStatus::SchemaUpgradeRequired:
            return BE_SQLITE_ERROR_SchemaUpgradeRequired;
        case SchemaStatus::CouldNotAcquireLocksOrCodes:
            return BE_SQLITE_ERROR_CouldNotAcquireLocksOrCodes;
        default:
            return isUpgrade ? BE_SQLITE_ERROR_SchemaUpgradeFailed : BE_SQLITE_ERROR_SchemaImportFailed;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    04/17
//--------------------------------------------------------------------------------------
DbResult DgnDb::MergeSchemaRevisions(Db::OpenParams const& params)
    {
    bvector<DgnRevisionCP> revisions = (((DgnDb::OpenParams&) params).GetSchemaUpgradeOptions()).GetUpgradeRevisions();
    if (revisions.empty())
        return BE_SQLITE_OK;

    for (DgnRevisionCP revision : revisions)
        {
        if (!revision)
            {
            BeAssert(false);
            return BE_SQLITE_ERROR_SchemaUpgradeFailed;
            }

        if (RevisionStatus::Success != Revisions().DoMergeRevision(*revision))
            return BE_SQLITE_ERROR_SchemaUpgradeFailed;
        }

    return BE_SQLITE_OK;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    04/17
//--------------------------------------------------------------------------------------
DbResult DgnDb::_OnDbOpening()
    {
    DbResult result = T_Super::_OnDbOpening();
    if (result != BE_SQLITE_OK)
        return result;

    return InitializeElementIdSequence();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    04/17
//--------------------------------------------------------------------------------------
DbResult DgnDb::_OnBriefcaseIdAssigned(BeBriefcaseId newBriefcaseId)
    {
    DbResult result = T_Super::_OnBriefcaseIdAssigned(newBriefcaseId);
    if (result != BE_SQLITE_OK)
        return result;

    result = ResetElementIdSequence(newBriefcaseId);
    if (result != BE_SQLITE_OK)
        return result;

    if (!newBriefcaseId.IsMasterId())
        {
        Txns().EnableTracking(true);
        result = Txns().InitializeTableHandlers();
        }

    return result;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    04/17
//--------------------------------------------------------------------------------------
void DgnDb::_OnAfterSchemaImport() const
    {
    ClearECSqlCache();
    Elements().ClearECCaches();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    04/17
//--------------------------------------------------------------------------------------
DbResult DgnDb::InitializeElementIdSequence()
    {
    return m_elementIdSequence.Initialize();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    04/17
//--------------------------------------------------------------------------------------
DbResult DgnDb::ResetElementIdSequence(BeBriefcaseId briefcaseId)
    {
    BeBriefcaseBasedId firstId(briefcaseId, 0);
    BeBriefcaseBasedId lastId(briefcaseId.GetNextBriefcaseId(), 0);

    Statement stmt;
    stmt.Prepare(*this, "SELECT max(Id) FROM " BIS_TABLE(BIS_CLASS_Element)  " WHERE Id >= ? AND Id < ?");
    stmt.BindInt64(1, firstId.GetValueUnchecked());
    stmt.BindInt64(2, lastId.GetValueUnchecked());
    stmt.Step();

    uint64_t minimumId = stmt.IsColumnNull(0) ? firstId.GetValueUnchecked() : stmt.GetValueInt64(0);

    return m_elementIdSequence.Reset(minimumId);
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

//--------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    10/15
//--------------------------------------------------------------------------------------
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

#ifdef CHECK_NON_NAVIGATION_PROPERTY_API
//--------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      03/17
//--------------+---------------+---------------+---------------+---------------+------
bool isNavigationPropertyOf(ECN::ECRelationshipClassCR relClass, DgnDbR db, BeSQLite::EC::ECInstanceId instid)
    {
    auto el = db.Elements().GetElement(DgnElementId(instid.GetValue()));
    if (!el.IsValid())
        return false;
    auto eclass = el->GetElementClass();
    for (auto ecprop : eclass->GetProperties())
        {
        auto navprop = ecprop->GetAsNavigationProperty();
        if (navprop != nullptr)
            {
            if (navprop->GetRelationshipClass() == &relClass)
                return true;
            }
        }
    return false;
    }
#endif

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   11/16
//+---------------+---------------+---------------+---------------+---------------+------
DbResult DgnDb::InsertLinkTableRelationship(BeSQLite::EC::ECInstanceKey& relKey, ECN::ECRelationshipClassCR relClass, BeSQLite::EC::ECInstanceId sourceId, 
                                     BeSQLite::EC::ECInstanceId targetId, ECN::IECRelationshipInstanceCP relInstanceProperties)
    {
#ifdef CHECK_NON_NAVIGATION_PROPERTY_API
    if (isNavigationPropertyOf(relClass, *this, sourceId) || isNavigationPropertyOf(relClass, *this, targetId))
        {
        BeAssert(false && "this API is for non-navigation properties only");
        return BE_SQLITE_ERROR;
        }
#endif

    //WIP this might need a cache of inserters if called often
    ECInstanceInserter inserter(*this, relClass, GetECCrudWriteToken());
    if (!inserter.IsValid())
        return BE_SQLITE_ERROR;

    return inserter.InsertRelationship(relKey, sourceId, targetId, relInstanceProperties);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/16
//--------------+---------------+---------------+---------------+---------------+------
DbResult DgnDb::UpdateLinkTableRelationshipProperties(EC::ECInstanceKeyCR key, ECN::IECInstanceR props)
    {
    auto eclass = Schemas().GetClass(key.GetClassId());
    if (nullptr == eclass)
        return DbResult::BE_SQLITE_ERROR;
    auto updater = Elements().m_updaterCache.GetUpdater(*this, *eclass);
    if (nullptr == updater)
        return DbResult::BE_SQLITE_ERROR;
    Utf8Char instidstr[32];
    BeStringUtilities::FormatUInt64(instidstr, key.GetInstanceId().GetValue());
    props.SetInstanceId(instidstr);
    return updater->Update(props);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/16
//--------------+---------------+---------------+---------------+---------------+------
DbResult DgnDb::DeleteLinkTableRelationship(EC::ECInstanceKeyCR key)
    {
    ECClassCP eclass = Schemas().GetClass(key.GetClassId());
    if (nullptr == eclass)
        return DbResult::BE_SQLITE_ERROR;

    Utf8String ecsql("DELETE FROM ");
    ecsql.append(eclass->GetECSqlName().c_str()).append(" WHERE ECInstanceId=?");

    CachedECSqlStatementPtr stmt = GetNonSelectPreparedECSqlStatement(ecsql.c_str(), GetECCrudWriteToken());
    if (stmt == nullptr)
        return BE_SQLITE_ERROR;

    stmt->BindId(1, key.GetInstanceId());
    return stmt->Step();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   11/16
//+---------------+---------------+---------------+---------------+---------------+------
DbResult DgnDb::DeleteLinkTableRelationships(Utf8CP relClassECSqlName, ECInstanceId sourceId, ECInstanceId targetId)
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

    // SchemaUpgrade logic may call OpenParams::_ReopenForProfileUpgrade changing the file
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
DgnClassId DgnImportContext::_RemapClassId(DgnClassId source)
    {
    if (!IsBetweenDbs())
        return source;

    DgnClassId dest = m_remap.Find(source);
    if (dest.IsValid())
        return dest;

    ECClassCP sourceecclass = GetSourceDb().Schemas().GetClass(source);
    if (nullptr == sourceecclass)
        return DgnClassId();

    ECClassCP destecclass = GetDestinationDb().Schemas().GetClass(sourceecclass->GetSchema().GetName().c_str(), sourceecclass->GetName().c_str());
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
    // Pre-populate the remap table with "fixed" element IDs
    AddElementId(source.Elements().GetRootSubjectId(), dest.Elements().GetRootSubjectId());
    AddElementId(source.Elements().GetDictionaryPartitionId(), dest.Elements().GetDictionaryPartitionId());
    AddElementId(source.Elements().GetRealityDataSourcesPartitionId(), dest.Elements().GetRealityDataSourcesPartitionId());

    ComputeGcsAdjustment();
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

