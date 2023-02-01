/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
ECDb::ECDb() : Db(), m_pimpl(new Impl(*this)) {}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
ECDb::~ECDb()
    {
    m_appData.Clear();
    if (m_pimpl != nullptr)
        {
        delete m_pimpl;
        m_pimpl = nullptr;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
ECSqlConfig& ECDb::GetECSqlConfig() const
    {
    return m_pimpl->GetECSqlConfig();
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::ApplyECDbSettings(bool requireECCrudTokenValidation, bool requireECSchemaImportTokenValidation)
    {
    m_pimpl->m_settingsManager.ApplySettings(requireECCrudTokenValidation, requireECSchemaImportTokenValidation);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
ECDb::SettingsManager const& ECDb::GetECDbSettingsManager() const { return m_pimpl->m_settingsManager; }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
bool ECDb::SchemaRequiresProfileUpgrade(ECN::ECSchemaCR ecSchema) const
    {
    return FeatureManager::SchemaRequiresProfileUpgrade(*this, ecSchema);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::_OnDbOpening()
    {
    DbResult stat = Db::_OnDbOpening();
    if (stat != BE_SQLITE_OK)
        return stat;

    return m_pimpl->OnDbOpening();
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::_OnDbCreated(CreateParams const& params)
    {
    DbResult stat = Db::_OnDbCreated(params);
    if (stat != BE_SQLITE_OK)
        return stat;

    BeAssert(!IsReadonly());
    return m_pimpl->OnDbCreated();
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::_AfterSchemaChangeSetApplied() const
    {
    ClearECDbCache();
    Schemas().RepopulateCacheTables();
    Schemas().UpgradeECInstances();
    return BE_SQLITE_OK;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::_AfterDataChangeSetApplied()
    {
    BentleyStatus status = ResetInstanceIdSequence(GetBriefcaseId());
    if (status != SUCCESS)
        return BE_SQLITE_ERROR;
    return BE_SQLITE_OK;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
bool ECDb::TryGetSqlFunction(DbFunction*& function, Utf8CP name, int argCount) const
    {
    return m_pimpl->TryGetSqlFunction(function, name, argCount);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
bvector<DbFunction*> ECDb::GetSqlFunctions() const {
    return m_pimpl->GetSqlFunctions();
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::_OnAfterSetBriefcaseId()
    {
    m_pimpl->OnBriefcaseIdAssigned(GetBriefcaseId());
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::_OnDbClose()
    {
    BeAssert(m_pimpl != nullptr && "DbClose was called in destructor after pimpl was deleted.");
    if (m_pimpl != nullptr)
        {
        delete m_pimpl;
        m_pimpl = nullptr;
        }

    m_pimpl = new Impl(*this);
    Db::_OnDbClose();
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::_OnDbChangedByOtherConnection()
    {
    Db::_OnDbChangedByOtherConnection();
    m_pimpl->OnDbChangedByOtherConnection();
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
ProfileState ECDb::_CheckProfileVersion() const
    {
    ProfileState besqliteState = Db::_CheckProfileVersion();
    return besqliteState.Merge(m_pimpl->GetProfileManager().CheckProfileVersion());
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::_UpgradeProfile(Db::OpenParams const& params)
    {
    DbResult stat = Db::_UpgradeProfile(params);
    if (BE_SQLITE_OK != stat)
        return stat;

    return m_pimpl->GetProfileManager().UpgradeProfile(params);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::_OnDbAttached(Utf8CP fileName, Utf8CP dbAlias) const { return m_pimpl->OnDbAttached(fileName, dbAlias); }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::_OnDbDetached(Utf8CP dbAlias) const { return m_pimpl->OnDbDetached(dbAlias); }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
int ECDb::_OnAddFunction(DbFunction& func) const { return m_pimpl->OnAddFunction(func) == SUCCESS ? 0 : 1; }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::_OnRemoveFunction(DbFunction& func) const { m_pimpl->OnRemoveFunction(func); }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb::ResetInstanceIdSequence(BeBriefcaseId briefcaseId, IdSet<ECN::ECClassId> const* ecClassIgnoreList) { return m_pimpl->ResetInstanceIdSequence(briefcaseId, ecClassIgnoreList);}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
SchemaManager const& ECDb::Schemas() const { return m_pimpl->Schemas(); }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
ECN::IECSchemaLocater& ECDb::GetSchemaLocater() const { return m_pimpl->GetSchemaLocater(); }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
ECN::IECClassLocater& ECDb::GetClassLocater() const { return m_pimpl->GetClassLocater(); }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
bool ECDb::IsChangeCacheAttached() const { return m_pimpl->IsChangeCacheAttached(); }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
bool ECDb::TryGetChangeCacheFileName(BeFileNameR changeCachePath) const { return m_pimpl->TryGetChangeCacheFileName(changeCachePath); }
//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::AttachChangeCache(BeFileNameCR changeCachePath) const { return m_pimpl->GetChangeManager().AttachChangeCacheFile(changeCachePath, true); }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::DetachChangeCache() const { return m_pimpl->GetChangeManager().DetachChangeCacheFile(); }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::CreateChangeCache(ECDbR changeCache, BeFileNameCR changeCachePath) const { return m_pimpl->GetChangeManager().CreateChangeCacheFile(changeCache, changeCachePath); }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
//static
BeFileName ECDb::GetDefaultChangeCachePath(Utf8CP ecdbPath) { return ChangeManager::DetermineDefaultCachePath(ecdbPath); }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECDb::ExtractChangeSummary(ECInstanceKey& changeSummaryKey, ECDbR changeCacheFile, ECDbCR primaryFile, ChangeSetArg const& changeSetInfo, ChangeSummaryExtractOptions const& options)
    {
    BeMutexHolder lock(primaryFile.GetImpl().GetMutex());
    return ChangeSummaryExtractor::Extract(changeSummaryKey, changeCacheFile, primaryFile, changeSetInfo, options);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb::ExtractChangeSummary(ECInstanceKey& changeSummaryKey, ChangeSetArg const& changeSetInfo, ChangeSummaryExtractOptions const& options) const
    {
    BeMutexHolder lock(GetImpl().GetMutex());
    return ChangeSummaryExtractor::Extract(changeSummaryKey, *this, changeSetInfo, options);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb::Purge(PurgeMode mode) const { return m_pimpl->Purge(mode); }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb::AddIssueListener(IIssueListener const& issueListener) { return m_pimpl->AddIssueListener(issueListener); }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::RemoveIssueListener() { m_pimpl->RemoveIssueListener(); }


//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
BeGuid ECDb::GetId() const  {return m_pimpl->GetId(); }
//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::AddAppData(AppData::Key const& key, AppData* appData, bool deleteOnClearCache) const
    {
    m_pimpl->AddAppData(key, appData, deleteOnClearCache);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb::OpenBlobIO(BlobIO& blobIO, Utf8CP tableSpace, ECN::ECClassCR ecClass, Utf8CP propertyAccessString, BeInt64Id ecinstanceId, bool writable, ECCrudWriteToken const* writeToken) const
    {
    return m_pimpl->OpenBlobIO(blobIO, tableSpace, ecClass, propertyAccessString, ecinstanceId, writable, writeToken);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::ClearECDbCache() const { m_pimpl->ClearECDbCache(); }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::AddECDbCacheClearListener(IECDbCacheClearListener& listener) { m_pimpl->AddECDbCacheClearListener(listener); }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::RemoveECDbCacheClearListener(IECDbCacheClearListener& listener) { m_pimpl->RemoveECDbCacheClearListener(listener); }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
ProfileVersion const& ECDb::GetECDbProfileVersion() const { return m_pimpl->GetProfileManager().GetProfileVersion(); }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
ECDb::Impl& ECDb::GetImpl() const { BeAssert(m_pimpl != nullptr); return *m_pimpl; }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ECDb::Initialize(BeFileNameCR ecdbTempDir, BeFileNameCP hostAssetsDir, BeSQLiteLib::LogErrors logSqliteErrors)
    {
    return Impl::InitializeLib(ecdbTempDir, hostAssetsDir, logSqliteErrors);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDb::IsInitialized() { return Impl::IsInitialized(); }

//*****************************************************************************************
//SettingsManager
//*****************************************************************************************

struct ECCrudWriteToken final {};
struct SchemaImportToken final {};

//---------------------------------------------------------------------------------------
//not inlined to prevent being called outside ECDb
// @bsimethod
//---------------------------------------------------------------------------------------
ECDb::SettingsManager::SettingsManager() {}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
ECDb::SettingsManager::~SettingsManager()
    {
    if (m_crudWriteToken != nullptr)
        {
        delete m_crudWriteToken;
        m_crudWriteToken = nullptr;
        }

    if (m_schemaImportToken != nullptr)
        {
        delete m_schemaImportToken;
        m_schemaImportToken = nullptr;
        }
    }

//---------------------------------------------------------------------------------------
//not inlined to prevent being called outside ECDb
// @bsimethod
//---------------------------------------------------------------------------------------
void ECDb::SettingsManager::ApplySettings(bool requireECCrudWriteToken, bool requireECSchemaImportToken)
    {
    m_settings = ECDb::Settings(requireECCrudWriteToken, requireECSchemaImportToken);

    if (requireECCrudWriteToken)
        m_crudWriteToken = new ECCrudWriteToken();

    if (requireECSchemaImportToken)
        m_schemaImportToken = new SchemaImportToken();
    }

//---------------------------------------------------------------------------------------
//not inlined to prevent being called outside ECDb
// @bsimethod
//---------------------------------------------------------------------------------------
ECDb::Settings::Settings() {}

//---------------------------------------------------------------------------------------
//not inlined to prevent being called outside ECDb
// @bsimethod
//---------------------------------------------------------------------------------------
ECDb::Settings::Settings(bool requiresECCrudWriteToken, bool requiresECSchemaImportToken) : m_requiresECCrudWriteToken(requiresECCrudWriteToken), m_requiresECSchemaImportToken(requiresECSchemaImportToken) {}


END_BENTLEY_SQLITE_EC_NAMESPACE
