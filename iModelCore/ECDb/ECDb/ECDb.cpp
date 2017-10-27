/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDb.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECDbImpl.h"

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
    if (m_pimpl != nullptr)
        {
        delete m_pimpl;
        m_pimpl = nullptr;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                02/2017
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::ApplyECDbSettings(bool requireECCrudTokenValidation, bool requireECSchemaImportTokenValidation, bool allowChangesetMergingIncompatibleECSchemaImport)
    {
    m_pimpl->m_settingsManager.ApplySettings(requireECCrudTokenValidation, requireECSchemaImportTokenValidation, allowChangesetMergingIncompatibleECSchemaImport);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                02/2017
//---------------+---------------+---------------+---------------+---------------+------
ECDb::SettingsManager const& ECDb::GetECDbSettingsManager() const { return m_pimpl->m_settingsManager; }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                01/2017
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::CheckECDbProfileVersion(bool& fileIsAutoUpgradable, bool openModeIsReadonly) const
    {
    return m_pimpl->CheckProfileVersion(fileIsAutoUpgradable, openModeIsReadonly);
    }

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
// @bsimethod                                Krischan.Eberle                12/2012
//---------------+---------------+---------------+---------------+---------------+------
//override
DbResult ECDb::_OnBriefcaseIdAssigned(BeBriefcaseId newBriefcaseId)
    {
    DbResult stat = Db::_OnBriefcaseIdAssigned(newBriefcaseId);
    if (stat != BE_SQLITE_OK)
        return stat;

    return m_pimpl->OnBriefcaseIdAssigned(newBriefcaseId);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                11/2012
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::_OnDbClose()
    {
    BeAssert(m_pimpl != nullptr && "DbClose was called in destructor after pimpl was deleted.");
    m_pimpl->OnDbClose();
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
DbResult ECDb::_VerifyProfileVersion(Db::OpenParams const& params)
    {
    DbResult stat = Db::_VerifyProfileVersion(params);
    if (stat != BE_SQLITE_OK)
        return stat;

    return m_pimpl->VerifyProfileVersion(params);
    }

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
BentleyStatus ECDb::OpenBlobIO(BlobIO& blobIO, ECN::ECClassCR ecClass, Utf8CP propertyAccessString, BeInt64Id ecinstanceId, bool writable, ECCrudWriteToken const* writeToken) const
    {
    return m_pimpl->OpenBlobIO(blobIO, ecClass, propertyAccessString, ecinstanceId, writable, writeToken);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Raman.Ramanujam                09/2012
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::ClearECDbCache() const { m_pimpl->ClearECDbCache(); }

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
void ECDb::SettingsManager::ApplySettings(bool requireECCrudWriteToken, bool requireECSchemaImportToken, bool allowChangesetMergingIncompatibleECSchemaImport)
    {
    m_settings = ECDb::Settings(requireECCrudWriteToken, requireECSchemaImportToken, allowChangesetMergingIncompatibleECSchemaImport);

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
ECDb::Settings::Settings(bool requiresECCrudWriteToken, bool requiresECSchemaImportToken, bool allowChangesetMergingIncompatibleECSchemaImport)
    : m_requiresECCrudWriteToken(requiresECCrudWriteToken), m_requiresECSchemaImportToken(requiresECSchemaImportToken), m_allowChangesetMergingIncompatibleSchemaImport(allowChangesetMergingIncompatibleECSchemaImport)
    {}


END_BENTLEY_SQLITE_EC_NAMESPACE
