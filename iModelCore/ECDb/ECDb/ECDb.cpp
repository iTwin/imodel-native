/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDb.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                11/2016
//---------------+---------------+---------------+---------------+---------------+------
ECDb::ECDb() : Db(), m_pimpl(new Impl(*this)) {}

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                09/2012
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
// @bsimethod                                Krischan.Eberle                02/2017
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::ApplyECDbSettings(bool requireECCrudTokenValidation, bool requireECSchemaImportTokenValidation)
    {
    m_pimpl->m_settingsManager.ApplySettings(requireECCrudTokenValidation, requireECSchemaImportTokenValidation);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                02/2017
//---------------+---------------+---------------+---------------+---------------+------
ECDb::SettingsManager const& ECDb::GetECDbSettingsManager() const { return m_pimpl->m_settingsManager; }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                11/2012
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::_OnDbOpening()
    {
    DbResult stat = Db::_OnDbOpening();
    if (stat != BE_SQLITE_OK)
        return stat;

    return m_pimpl->OnDbOpening();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                11/2012
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
// @bsimethod                                Affan.Khan                    07/2018
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::_OnAfterChangesetApplied(bool hasSchemaChanges) const
    {
    Schemas().RepopulateCacheTables();
    Schemas().UpgradeECInstances();
    return BE_SQLITE_OK;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                    02/2019
//---------------+---------------+---------------+---------------+---------------+------
bool ECDb::TryGetSqlFunction(DbFunction*& function, Utf8CP name, int argCount) const
    {
    return m_pimpl->TryGetSqlFunction(function, name, argCount);
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                12/2012
//---------------+---------------+---------------+---------------+---------------+------
//override
DbResult ECDb::_OnAfterSetAsBriefcase(BeBriefcaseId newBriefcaseId)
    {
    DbResult stat = Db::_OnAfterSetAsBriefcase(newBriefcaseId);
    if (stat != BE_SQLITE_OK)
        return stat;

    return m_pimpl->OnBriefcaseIdAssigned(newBriefcaseId);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                12/2012
//---------------+---------------+---------------+---------------+---------------+------
//override
DbResult ECDb::_OnAfterSetAsMaster(BeGuid guid)
{
    DbResult stat = Db::_OnAfterSetAsMaster(guid);
    if (stat != BE_SQLITE_OK)
        return stat;

    BeBriefcaseId masterBriefcaseId(BeBriefcaseId::Master());
    return m_pimpl->OnBriefcaseIdAssigned(masterBriefcaseId);
}

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                11/2012
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
// @bsimethod                                Krischan.Eberle                01/2015
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::_OnDbChangedByOtherConnection()
    {
    Db::_OnDbChangedByOtherConnection();
    m_pimpl->OnDbChangedByOtherConnection();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                07/2013
//---------------+---------------+---------------+---------------+---------------+------
ProfileState ECDb::_CheckProfileVersion() const 
    { 
    ProfileState besqliteState = Db::_CheckProfileVersion();
    return besqliteState.Merge(m_pimpl->GetProfileManager().CheckProfileVersion()); 
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                07/2013
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::_UpgradeProfile()
    {
    DbResult stat = Db::_UpgradeProfile();
    if (BE_SQLITE_OK != stat)
        return stat;

    return m_pimpl->GetProfileManager().UpgradeProfile();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                03/2014
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::_OnDbAttached(Utf8CP fileName, Utf8CP dbAlias) const { return m_pimpl->OnDbAttached(fileName, dbAlias); }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                03/2014
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::_OnDbDetached(Utf8CP dbAlias) const { return m_pimpl->OnDbDetached(dbAlias); }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                03/2014
//---------------+---------------+---------------+---------------+---------------+------
int ECDb::_OnAddFunction(DbFunction& func) const { return m_pimpl->OnAddFunction(func) == SUCCESS ? 0 : 1; }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                03/2014
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::_OnRemoveFunction(DbFunction& func) const { m_pimpl->OnRemoveFunction(func); }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                10/2017
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb::ResetInstanceIdSequence(BeBriefcaseId briefcaseId, IdSet<ECN::ECClassId> const* ecClassIgnoreList) { return m_pimpl->ResetInstanceIdSequence(briefcaseId, ecClassIgnoreList);}

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                07/2013
//---------------+---------------+---------------+---------------+---------------+------
SchemaManager const& ECDb::Schemas() const { return m_pimpl->Schemas(); }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                07/2013
//---------------+---------------+---------------+---------------+---------------+------
ECN::IECSchemaLocater& ECDb::GetSchemaLocater() const { return m_pimpl->GetSchemaLocater(); }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                07/2013
//---------------+---------------+---------------+---------------+---------------+------
ECN::IECClassLocater& ECDb::GetClassLocater() const { return m_pimpl->GetClassLocater(); }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                11/2017
//---------------+---------------+---------------+---------------+---------------+------
bool ECDb::IsChangeCacheAttached() const { return m_pimpl->IsChangeCacheAttached(); }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                11/2017
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::AttachChangeCache(BeFileNameCR changeCachePath) const { return m_pimpl->GetChangeManager().AttachChangeCacheFile(changeCachePath, true); }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                11/2017
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::DetachChangeCache() const { return m_pimpl->GetChangeManager().DetachChangeCacheFile(); }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                12/2017
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::CreateChangeCache(ECDbR changeCache, BeFileNameCR changeCachePath) const { return m_pimpl->GetChangeManager().CreateChangeCacheFile(changeCache, changeCachePath); }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                11/2017
//---------------+---------------+---------------+---------------+---------------+------
//static
BeFileName ECDb::GetDefaultChangeCachePath(Utf8CP ecdbPath) { return ChangeManager::DetermineDefaultCachePath(ecdbPath); }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                11/2017
//---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECDb::ExtractChangeSummary(ECInstanceKey& changeSummaryKey, ECDbR changeCacheFile, ECDbCR primaryFile, ChangeSetArg const& changeSetInfo, ChangeSummaryExtractOptions const& options)
    {
    BeMutexHolder lock(primaryFile.GetImpl().GetMutex());
    return ChangeSummaryExtractor::Extract(changeSummaryKey, changeCacheFile, primaryFile, changeSetInfo, options);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                11/2017
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb::ExtractChangeSummary(ECInstanceKey& changeSummaryKey, ChangeSetArg const& changeSetInfo, ChangeSummaryExtractOptions const& options) const
    {
    BeMutexHolder lock(GetImpl().GetMutex());
    return ChangeSummaryExtractor::Extract(changeSummaryKey, *this, changeSetInfo, options);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                11/2015
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb::Purge(PurgeMode mode) const { return m_pimpl->Purge(mode); }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                09/2015
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb::AddIssueListener(IIssueListener const& issueListener) { return m_pimpl->AddIssueListener(issueListener); }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                09/2015
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::RemoveIssueListener() { m_pimpl->RemoveIssueListener(); }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                03/2016
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::AddAppData(AppData::Key const& key, AppData* appData, bool deleteOnClearCache) const
    {
    m_pimpl->AddAppData(key, appData, deleteOnClearCache);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                12/2016
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb::OpenBlobIO(BlobIO& blobIO, Utf8CP tableSpace, ECN::ECClassCR ecClass, Utf8CP propertyAccessString, BeInt64Id ecinstanceId, bool writable, ECCrudWriteToken const* writeToken) const
    {
    return m_pimpl->OpenBlobIO(blobIO, tableSpace, ecClass, propertyAccessString, ecinstanceId, writable, writeToken);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Raman.Ramanujam                09/2012
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::ClearECDbCache() const { m_pimpl->ClearECDbCache(); }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                06/2018
//---------------+---------------+---------------+---------------+---------------+------
ProfileVersion const& ECDb::GetECDbProfileVersion() const { return m_pimpl->GetProfileManager().GetProfileVersion(); }

//--------------------------------------------------------------------------------------
// @bsimethod                                Raman.Ramanujam                09/2012
//---------------+---------------+---------------+---------------+---------------+------
ECDb::Impl& ECDb::GetImpl() const { BeAssert(m_pimpl != nullptr); return *m_pimpl; }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                09/2012
//---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ECDb::Initialize(BeFileNameCR ecdbTempDir, BeFileNameCP hostAssetsDir, BeSQLiteLib::LogErrors logSqliteErrors)
    {
    return Impl::InitializeLib(ecdbTempDir, hostAssetsDir, logSqliteErrors);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                         03/2018
//---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDb::IsInitialized() { return Impl::IsInitialized(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   09/2015
//---------------------------------------------------------------------------------------
void ECDb::IIssueListener::ReportIssue(Utf8CP message) const { _OnIssueReported(message); }


//*****************************************************************************************
//SettingsManager
//*****************************************************************************************

struct ECCrudWriteToken final {};
struct SchemaImportToken final {};

//---------------------------------------------------------------------------------------
//not inlined to prevent being called outside ECDb
// @bsimethod                                                   Krischan.Eberle   10/2017
//---------------------------------------------------------------------------------------
ECDb::SettingsManager::SettingsManager() {}

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                10/2017
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
// @bsimethod                                                   Krischan.Eberle   10/2017
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
// @bsimethod                                                   Krischan.Eberle   02/2017
//---------------------------------------------------------------------------------------
ECDb::Settings::Settings() {}

//---------------------------------------------------------------------------------------
//not inlined to prevent being called outside ECDb
// @bsimethod                                                   Krischan.Eberle   10/2017
//---------------------------------------------------------------------------------------
ECDb::Settings::Settings(bool requiresECCrudWriteToken, bool requiresECSchemaImportToken) : m_requiresECCrudWriteToken(requiresECCrudWriteToken), m_requiresECSchemaImportToken(requiresECSchemaImportToken) {}


END_BENTLEY_SQLITE_EC_NAMESPACE
