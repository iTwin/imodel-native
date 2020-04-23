/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
    ApplyECDbSettings(true /* requireECCrudWriteToken */, true /* requireECSchemaImportToken */);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                    Keith.Bentley                    11/18
+---------------+---------------+---------------+---------------+---------------+------*/
Napi::Object DgnDb::GetJsTxns() {
    return  m_jsIModelDb == nullptr ? Napi::Object() : m_jsIModelDb.Get("txns").As<Napi::Object>();  // should have a member object named "txns"
}

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                    Keith.Bentley                    11/18
+---------------+---------------+---------------+---------------+---------------+------*/
Napi::String DgnDb::GetJsClassName(DgnElementId id) {
    auto el = Elements().Get<DgnElement>(id);
    return ToJsString(el.IsValid() ? el->GetElementClass()->GetFullName() : "");
}

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                    Keith.Bentley                    11/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDb::CallJsFunction(Napi::Object obj, Utf8CP methodName, std::vector<napi_value> const& args) {
    if (obj == nullptr)
        return;

    auto func = obj.Get(methodName);
    if (!func.IsFunction()) {
        Utf8String err("method not found: ");
        err += methodName;
        Napi::TypeError::New(obj.Env(), err.c_str()).ThrowAsJavaScriptException();
        return;
    }
    func.As<Napi::Function>().Call(obj, args);
}

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                    Keith.Bentley                    11/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDb::RaiseJsEvent(Napi::Object obj, Utf8CP eventName, std::vector<napi_value> const& args) {
    if (obj == nullptr)
        return;

    auto event = obj.Get(eventName);
    if (!event.IsObject()) {
        Utf8String err("BeEvent object not found: ");
        err += eventName;
        Napi::TypeError::New(obj.Env(), err.c_str()).ThrowAsJavaScriptException();
        return;
    }

    CallJsFunction(event.As<Napi::Object>(), "raiseEvent", args);
}

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                    Keith.Bentley                    08/19
+---------------+---------------+---------------+---------------+---------------+------*/
CachedECSqlStatementPtr DgnDb::GetGeometricModelUpdateStatement() {
    return GetNonSelectPreparedECSqlStatement("UPDATE " BIS_SCHEMA(BIS_CLASS_GeometricModel) " SET GeometryGuid=?,LastMod=julianday('now') WHERE ECInstanceId=?", GetECCrudWriteToken());
}
CachedStatementPtr DgnDb::GetModelLastModUpdateStatement() {
    return GetCachedStatement("UPDATE " BIS_TABLE(BIS_CLASS_Model) " SET LastMod=julianday('now') WHERE Id=?", false);
}

//--------------------------------------------------------------------------------------
//not inlined as it must not be called externally
// @bsimethod                                Krischan.Eberle                11/2016
//---------------+---------------+---------------+---------------+---------------+------
ECCrudWriteToken const* DgnDb::GetECCrudWriteToken() const {return GetECDbSettingsManager().GetCrudWriteToken();}

//--------------------------------------------------------------------------------------
//not inlined as it must not be called externally
// @bsimethod                                Krischan.Eberle                11/2016
//---------------+---------------+---------------+---------------+---------------+------
SchemaImportToken const* DgnDb::GetSchemaImportToken() const { return GetECDbSettingsManager().GetSchemaImportToken(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDb::Destroy()
    {
    m_models.Empty();
    m_txnManager = nullptr; // RefCountedPtr, deletes TxnManager
    m_lineStyles = nullptr;
    m_revisionManager.reset(nullptr);
    m_cacheECInstanceInserter.clear();
    ClearECSqlCache();
    DestroyBriefcaseManager();
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
        m_txnManager = nullptr; // Deletes ref counted ptr so that statement caches are freed
        return rc;
        }

    Fonts().Update(); // ensure the font Id cache is loaded; if you wait for on-demand, it may need to query during an update, which we'd like to avoid
    m_geoLocation.Load();

    return BE_SQLITE_OK;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    04/17
//--------------------------------------------------------------------------------------
DbResult DgnDb::InitializeSchemas(Db::OpenParams const& params)
    {
    SchemaUpgradeOptions const& schemaUpgradeOptions = ((DgnDb::OpenParams const&) params).GetSchemaUpgradeOptions();

    SchemaStatus status = Domains().InitializeSchemas(schemaUpgradeOptions);
    if (status == SchemaStatus::SchemaTooNew || status == SchemaStatus::SchemaTooOld)
        return SchemaStatusToDbResult(status, true /*=isUpgrade*/);

    SchemaUpgradeOptions::DomainUpgradeOptions domainUpgradeOptions = schemaUpgradeOptions.GetDomainUpgradeOptions();
    bool upgrade = (status == SchemaStatus::SchemaUpgradeRequired || status == SchemaStatus::SchemaUpgradeRecommended) &&
        domainUpgradeOptions == SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade;
    if (!upgrade && status != SchemaStatus::Success)
        return SchemaStatusToDbResult(status, true /*=isUpgrade*/);

    DbResult result;
    if (BE_SQLITE_OK != (result = ProcessRevisions(params)))
        return result;

    if (upgrade)
        status = Domains().UpgradeSchemas();

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
        case SchemaStatus::SchemaUpgradeRecommended:
            return BE_SQLITE_ERROR_SchemaUpgradeRecommended;
        // case SchemaStatus::SchemaLockFailed:     NEEDS WORK - shouldn't we map this to BE_SQLITE_ERROR_CouldNotAcquireLocksOrCodes, too?
        case SchemaStatus::CouldNotAcquireLocksOrCodes:
            return BE_SQLITE_ERROR_CouldNotAcquireLocksOrCodes;
        default:
            return isUpgrade ? BE_SQLITE_ERROR_SchemaUpgradeFailed : BE_SQLITE_ERROR_SchemaImportFailed;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    04/17
//--------------------------------------------------------------------------------------
DbResult DgnDb::ProcessRevisions(Db::OpenParams const& params)
    {
    SchemaUpgradeOptions schemaUpgradeOptions = (((DgnDb::OpenParams const&) params).GetSchemaUpgradeOptions());
    bvector<DgnRevisionCP> revisions = schemaUpgradeOptions.GetRevisions();
    if (revisions.empty())
        return BE_SQLITE_OK;

    IConcurrencyControl* concurrencyControl = schemaUpgradeOptions.GetConcurrencyControl();
    if (concurrencyControl)
        SetConcurrencyControl(concurrencyControl);

    RevisionStatus status = Revisions().DoProcessRevisions(revisions, schemaUpgradeOptions.GetRevisionProcessOption());
    return status == RevisionStatus::Success ? BE_SQLITE_OK : BE_SQLITE_ERROR_SchemaUpgradeFailed;
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

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                    Keith.Bentley                    04/20
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDb::_OnDbGuidChange(BeSQLite::BeGuid guid) {
    // whenever we switch DbGuid's, these values are no longer valid
    Revisions().ClearSavedValues();
}

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                    Keith.Bentley                    04/20
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnDb::HasParentChangeset() const {
     return Revisions().HasParentRevision();
}

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                    Keith.Bentley                    04/20
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnDb::RequireStandaloneTxns() const {
    auto standalone = QueryStandaloneEditFlags();
    return standalone.isNull() ? false : standalone["txns"].asBool();
}

/*---------------------------------------------------------------------------------**/ /**
@bsimethod                                    Keith.Bentley                    04/20
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDb::_OnBeforeSetBriefcaseId(BeBriefcaseId newId) {
    T_Super::_OnBeforeSetBriefcaseId(newId);

    Txns().EnableTracking(false);
    Txns().DeleteAllTxns(); // whenever we switch briefcaseIds, all of the current txn data is invalid
}

/*---------------------------------------------------------------------------------**/ /**
@bsimethod                                    Keith.Bentley                    04/20
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDb::_OnAfterSetBriefcaseId() {
    T_Super::_OnAfterSetBriefcaseId();

    ResetElementIdSequence(GetBriefcaseId());

    if (AreTxnsRequired()) {
        Txns().EnableTracking(true);
        Txns().InitializeTableHandlers();
    }
}

//--------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    09/19
//--------------------------------------------------------------------------------------
DbResult DgnDb::_AfterSchemaChangeSetApplied() const {
    DbResult result = T_Super::_AfterSchemaChangeSetApplied();
    if (result != BE_SQLITE_OK)
        return result;
    Domains().SyncWithSchemas();
    return BE_SQLITE_OK;
}

//--------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    09/19
//--------------------------------------------------------------------------------------
DbResult DgnDb::_AfterDataChangeSetApplied()
    {
    DbResult result = T_Super::_AfterDataChangeSetApplied();
    if (result != BE_SQLITE_OK)
        return result;

    result = ResetElementIdSequence(GetBriefcaseId());
    if (result != BE_SQLITE_OK)
        {
        LOG.errorv("Failed to reset element id sequence after apply with SQLite error %s", GetLastError().c_str());
        return result;
        }

    return BE_SQLITE_OK;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    04/17
//--------------------------------------------------------------------------------------
void DgnDb::_OnBeforeClearECDbCache() const
    {
    m_cacheECInstanceInserter.clear();
    ClearECSqlCache();
    Elements().ClearECCaches();
    Models().ClearECCaches();
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

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     02/18
//-------------------------------------------------------------------------------------------
IBriefcaseManager*  DgnDb::GetExistingBriefcaseManager() const
    {
    return m_briefcaseManager.IsNull() ? nullptr : m_briefcaseManager.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDb::DestroyBriefcaseManager()
    {
    if (m_briefcaseManager.IsValid())
        {
        m_briefcaseManager->OnDgnDbDestroyed();
        m_briefcaseManager = nullptr;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnDb::SetConcurrencyControl(IConcurrencyControl* control)
    {
    // TBD: assert main thread

    if (Txns().HasChanges())
        return BSIERROR;

    m_concurrencyControl = control;

    DestroyBriefcaseManager();
    return BSISUCCESS;
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

    ECInstanceInserter* inserter;
    auto itor = m_cacheECInstanceInserter.find(relClass.GetId().GetValue());
    if (itor == m_cacheECInstanceInserter.end())
        inserter = m_cacheECInstanceInserter.insert(std::make_pair(relClass.GetId().GetValue(), std::unique_ptr<ECInstanceInserter>(new ECInstanceInserter(*this, relClass, GetECCrudWriteToken())))).first->second.get();
    else
        inserter = itor->second.get();

    if (!inserter || !inserter->IsValid())
        return BE_SQLITE_ERROR;

    return inserter->InsertRelationship(relKey, sourceId, targetId, relInstanceProperties);
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
        // When it comes to schema upgrades, the caller probably does know what he is doing -- at least the iModelBridge framework does -- so this is not necessarily an "error".
        auto sev = (BE_SQLITE_ERROR_SchemaUpgradeRequired == stat)? NativeLogging::LOG_INFO: NativeLogging::LOG_ERROR;
        LOG.messagev(sev, "Error %s opening [%s]", Db::InterpretDbResult(stat), m_fileName.c_str());
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
* @bsimethod                                                    Shaun.Sewall    01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnImportContext::Dump(Utf8StringCR outputFileName)
    {
    BeFile outputFile;
    BeFileStatus status = outputFile.Create(outputFileName);
    if (BeFileStatus::Success != status)
        return BentleyStatus::ERROR;

    Utf8PrintfString sourceDbLine("SourceDb: %s\n", m_sourceDb.GetFileName().GetNameUtf8().c_str());
    Utf8PrintfString targetDbLine("TargetDb: %s\n", m_destDb.GetFileName().GetNameUtf8().c_str());
    outputFile.Write(nullptr, sourceDbLine.c_str(), static_cast<uint32_t>(sourceDbLine.size()));
    outputFile.Write(nullptr, targetDbLine.c_str(), static_cast<uint32_t>(targetDbLine.size()));

    Utf8CP codeSpecHeaderLine = "\n=== CodeSpecs ===\n";
    outputFile.Write(nullptr, codeSpecHeaderLine, static_cast<uint32_t>(strlen(codeSpecHeaderLine)));
    if (m_remap.m_codeSpecId.size() > 0)
        {
        for (auto const& it : m_remap.m_codeSpecId)
            {
            CodeSpecCPtr source = m_sourceDb.CodeSpecs().GetCodeSpec(it.first);
            CodeSpecCPtr target = m_destDb.CodeSpecs().GetCodeSpec(it.second);
            Utf8CP sourceName = source.IsValid() ? source->GetName().c_str() : "<Invalid>";
            Utf8CP targetName = target.IsValid() ? target->GetName().c_str() : "<Invalid>";
            Utf8PrintfString line("%llu, %s --> %llu, %s\n", it.first.GetValueUnchecked(), sourceName, it.second.GetValueUnchecked(), targetName);
            outputFile.Write(nullptr, line.c_str(), static_cast<uint32_t>(line.size()));
            }
        }
    else
        {
        Utf8CP noCodeSpecsLine = "No CodeSpec remappings\n";
        outputFile.Write(nullptr, noCodeSpecsLine, static_cast<uint32_t>(strlen(noCodeSpecsLine)));
        }

    Utf8CP fontHeaderLine = "\n=== Fonts ===\n";
    outputFile.Write(nullptr, fontHeaderLine, static_cast<uint32_t>(strlen(fontHeaderLine)));
    if (m_remap.m_fontId.size() > 0)
        {
        for (auto const& it : m_remap.m_fontId)
            {
            DgnFontCP source = m_sourceDb.Fonts().FindFontById(it.first);
            DgnFontCP target = m_destDb.Fonts().FindFontById(it.second);
            Utf8CP sourceName = (nullptr != source) ? source->GetName().c_str() : "<Invalid>";
            Utf8CP targetName = (nullptr != target) ? target->GetName().c_str() : "<Invalid>";
            Utf8PrintfString line("%llu, %s --> %llu, %s\n", it.first.GetValueUnchecked(), sourceName, it.second.GetValueUnchecked(), targetName);
            outputFile.Write(nullptr, line.c_str(), static_cast<uint32_t>(line.size()));
            }
        }
    else
        {
        Utf8CP noFontsLine = "No Font remappings\n";
        outputFile.Write(nullptr, noFontsLine, static_cast<uint32_t>(strlen(noFontsLine)));
        }

    Utf8CP classHeaderLine = "\n=== Classes ===\n";
    outputFile.Write(nullptr, classHeaderLine, static_cast<uint32_t>(strlen(classHeaderLine)));
    if (m_remap.m_classId.size() > 0)
        {
        for (auto const& it : m_remap.m_classId)
            {
            ECClassCP source = m_sourceDb.Schemas().GetClass(it.first);
            ECClassCP target = m_destDb.Schemas().GetClass(it.second);
            Utf8CP sourceName = (nullptr != source) ? source->GetFullName() : "<Invalid>";
            Utf8CP targetName = (nullptr != target) ? target->GetFullName() : "<Invalid>";
            Utf8PrintfString line("%llu, %s --> %llu, %s\n", it.first.GetValueUnchecked(), sourceName, it.second.GetValueUnchecked(), targetName);
            outputFile.Write(nullptr, line.c_str(), static_cast<uint32_t>(line.size()));
            }
        }
    else
        {
        Utf8CP noClassesLine = "No Class remappings\n";
        outputFile.Write(nullptr, noClassesLine, static_cast<uint32_t>(strlen(noClassesLine)));
        }

    Utf8CP elementHeaderLine = "\n=== Elements ===\n";
    outputFile.Write(nullptr, elementHeaderLine, static_cast<uint32_t>(strlen(elementHeaderLine)));
    if (m_remap.m_elementId.size() > 0)
        {
        for (auto const& it : m_remap.m_elementId)
            {
            DgnElementCPtr source = m_sourceDb.Elements().Get<DgnElement>(it.first);
            DgnElementCPtr target = m_destDb.Elements().Get<DgnElement>(it.second);
            if (!source.IsValid() || !target.IsValid())
                {
                Utf8CP sourceName = source.IsValid() ? source->GetDisplayLabel().c_str() : "<Invalid>";
                Utf8CP targetName = target.IsValid() ? target->GetDisplayLabel().c_str() : "<Invalid>";
                Utf8PrintfString line("%llu, %s, %s --> %llu, %s, %s\n",
                    it.first.GetValueUnchecked(), it.first.ToHexStr().c_str(), sourceName,
                    it.second.GetValueUnchecked(), it.second.ToHexStr().c_str(), targetName);
                outputFile.Write(nullptr, line.c_str(), static_cast<uint32_t>(line.size()));
                }
            else
                {
                Utf8PrintfString line("e=%llu, e=%s, c=%llu, m=%llu, p=%llu, %s --> e=%llu, e=%s, c=%llu, m=%llu, p=%llu, %s\n",
                    it.first.GetValueUnchecked(),
                    it.first.ToHexStr().c_str(),
                    source->GetElementClassId().GetValueUnchecked(),
                    source->GetModelId().GetValueUnchecked(),
                    source->GetParentId().GetValueUnchecked(),
                    source->GetDisplayLabel().c_str(),
                    it.second.GetValueUnchecked(),
                    it.second.ToHexStr().c_str(),
                    target->GetElementClassId().GetValueUnchecked(),
                    target->GetModelId().GetValueUnchecked(),
                    target->GetParentId().GetValueUnchecked(),
                    target->GetDisplayLabel().c_str());
                outputFile.Write(nullptr, line.c_str(), static_cast<uint32_t>(line.size()));
                }
            }
        }
    else
        {
        Utf8CP noElementsLine = "No Element remappings\n";
        outputFile.Write(nullptr, noElementsLine, static_cast<uint32_t>(strlen(noElementsLine)));
        }

    outputFile.Close();
    return BentleyStatus::SUCCESS;
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

//=======================================================================================
// @bsiclass                                                    Ray.Bentley     11/2018
//=======================================================================================
struct RangeWithoutOutlierCalculator
{
    struct Stat
        {
        DRange3d                m_range;
        BeInt64Id               m_id;
        double                  m_diagonal;

        Stat() {}
        Stat(DRange3dCR range, BeInt64Id id): m_range(range), m_id(id), m_diagonal(range.DiagonalDistance()) { }
        DPoint3d GetCenter() const { return m_range.LocalToGlobal(.5, .5, .5); }
        };

    bvector<Stat>       m_stats;
    double              m_maxDiagonal = 0.0;
    BeInt64Id           m_maxId;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void Add(DRange3dCR range, BeInt64Id id)
    {
    if (range.DiagonalDistance() > m_maxDiagonal)
        {
        m_maxDiagonal = range.DiagonalDistance();
        m_maxId = id;
        }

    if (!range.IsNull()) m_stats.push_back(Stat(range, id));
    }

    static void DumpRange(const char* label, DRange3dCR range) { DEBUG_PRINTF ("%s Range: %lf, %lf, %lf) \t (%lf, %lf, %lf) Diagonal: %lf (KM)\n", label, range.low.x, range.low.y, range.low.z, range.high.x, range.high.y, range.high.z, range.DiagonalDistance()/1000.0); }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d ComputeRange(bvector<BeInt64Id>& outliers, DRange3dR fullRange, double sigmaMultiplier = 5.0, double minLimit = 100.0)
    {
    double      variance = 0.0, sum = 0.0;
    DPoint3d    centroid = DPoint3d::FromZero();

    for (auto const& stat : m_stats)
        {
        centroid.SumOf(centroid, stat.GetCenter(), stat.m_diagonal);        // Arbitrarily weight by range diagaonal.
        sum += stat.m_diagonal;
        }

    centroid.Scale(1.0 / sum);

    for (auto const& stat : m_stats)
        {
        double  delta = stat.GetCenter().Distance(centroid);
        variance += stat.m_diagonal * delta * delta;
        }
    variance /= sum;
    double      deviation = sqrt(variance);
    double      limit= max(minLimit, sigmaMultiplier * deviation);
    DRange3d    range = DRange3d::NullRange();

    DEBUG_PRINTF("Deviation: %lf, Sum: %lf, Limit: %lf, Multiplier: %lf, Max Diagonal: %lf\n", deviation, sum, limit, sigmaMultiplier, m_maxDiagonal);

    fullRange = DRange3d::NullRange();
    for (auto const& stat : m_stats)
        {
        fullRange.Extend (stat.m_range);
        if (stat.GetCenter().Distance(centroid) < limit)
            range.Extend(stat.m_range);
        else
            outliers.push_back(stat.m_id);
        }
    DumpRange("Full Range", fullRange);
    if (!outliers.empty())
        DumpRange("Reduced Range", range);

    return range;
    }
};  //  RangeWithoutOutlierCalculator.


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2018
* Computes the range of elements that are "statistically" signficant - ignoring elements
* that are more than maxDeviation standard deviations from the centroid.
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d DgnDb::ComputeGeometryExtentsWithoutOutliers(DRange3dP rangeWithOutliers, bvector<BeInt64Id>* elementOutliers, double maxDeviations) const
    {
    auto stmt = GetPreparedECSqlStatement("SELECT ECInstanceId,Origin,Yaw,Pitch,Roll,BBoxLow,BBoxHigh FROM " BIS_SCHEMA(BIS_CLASS_GeometricElement3d));
    RangeWithoutOutlierCalculator   elementRangeCalculator;

    while (BE_SQLITE_ROW == stmt->Step())
        {
        if (stmt->IsValueNull(1)) // has no placement
            continue;

        double yaw   = stmt->GetValueDouble(2);
        double pitch = stmt->GetValueDouble(3);
        double roll  = stmt->GetValueDouble(4);

        DPoint3d low = stmt->GetValuePoint3d(5);
        DPoint3d high = stmt->GetValuePoint3d(6);

        Placement3d placement(stmt->GetValuePoint3d(1),
                              YawPitchRollAngles(Angle::FromDegrees(yaw), Angle::FromDegrees(pitch), Angle::FromDegrees(roll)),
                              ElementAlignedBox3d(low.x, low.y, low.z, high.x, high.y, high.z));

        elementRangeCalculator.Add(placement.CalculateRange(), stmt->GetValueId<DgnElementId>(0));
        }

    bvector<BeInt64Id>  outliers;

    DRange3d            fullRange, elementRange = elementRangeCalculator.ComputeRange(outliers, fullRange);

    if (nullptr != elementOutliers)
        {
        std::for_each (outliers.begin (), outliers.end (), [elementOutliers](BeInt64Id id) {elementOutliers->push_back (id);});
        }

    if (nullptr != rangeWithOutliers)
        *rangeWithOutliers = fullRange;

    if (!outliers.empty())
        {
        double      fullDiagonal = fullRange.DiagonalDistance(), reducedDiagonal = elementRange.DiagonalDistance();
        auto logMessage1 = Utf8PrintfString("%d Outlying elements of %d Total were ignored when calculating project extents\n", (int)outliers.size(), (int) elementRangeCalculator.m_stats.size());
        auto logMessage2 = Utf8PrintfString("Range reduced from %lf to %lf (%lf %%)\n", fullDiagonal, reducedDiagonal, 100.0 * (fullDiagonal - reducedDiagonal) / fullDiagonal);
        DEBUG_PRINTF (">>>>>>>>>>>>>>>>>>%s %s<<<<<<<<<<<<<<<<<<<", logMessage1.c_str(), logMessage2.c_str());
        }
    else
        {
        DEBUG_PRINTF("No Element Outliers of %d Total, Range Diagonal: %lf\n", (int) elementRangeCalculator.m_stats.size(), fullRange.IsNull() ? 0.0 : fullRange.DiagonalDistance());
        }

    if (elementRange.DiagonalDistance() > 5.0E5)
        {
        DEBUG_PRINTF("*********************************************** Range still invalid (%f KM) ******************************************\n\n\n", elementRange.DiagonalDistance() / 1000.0);
        }

    return elementRange;
    }

#ifdef TEST_OUTLYING_MODELS
// This is currently not used - but perhaps may be useful at some point. It performs the same statistical analysis as element outlier, but on the models instead.
// It improves the Mott/EAP files for section 8 - but not enough to make it usable.

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d DgnDb::ComputeExtentsWithoutOutlyingModels(DRange3dCR elementRange, DRange3dCP rangeWithOutliers = nullptr, size_t* outlierCount = nullptr) const

    bvector<BeInt64Id>              modelOutliers;
    RangeWithoutOutlierCalculator   modelRangeCalculator;

    for (auto& entry : Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_SpatialModel)))
        {
        auto model = Models().Get<SpatialModel>(entry.GetModelId());
        if (model.IsValid())
            {
            DRange3d        modelRange;

            modelRange.IntersectionOf(elementRange, model->QueryModelRange());
            modelRangeCalculator.Add(modelRange, entry.GetModelId());
            }
        }

    DRange3d    fullModelRange, modelRange = modelRangeCalculator.ComputeRange(modelOutliers, fullModelRange);

    if (!modelOutliers.empty())
        {
        double      fullDiagonal = fullModelRange.DiagonalDistance(), reducedDiagonal = modelRange.DiagonalDistance();
        auto logMessage1 = Utf8PrintfString("%d Outlying models of %d Total were ignored when calculating project extents\n",  (int) modelOutliers.size(), (int) modelRangeCalculator.m_stats.size());
        auto logMessage2 = Utf8PrintfString("Range reduced from %lf to %lf (%lf %%)\n", fullDiagonal, reducedDiagonal, 100.0 * (fullDiagonal - reducedDiagonal) / fullDiagonal);
        DEBUG_PRINTF ("%s %s", logMessage1.c_str(), logMessage2.c_str());
        }
    else
        {
        DEBUG_PRINTF("No Model Outliers of %d Total, Range Diagonal: %lf\n",  (int) modelRangeCalculator.m_stats.size(), fullModelRange.DiagonalDistance());
        }

    return modelRange;
    }

#endif
