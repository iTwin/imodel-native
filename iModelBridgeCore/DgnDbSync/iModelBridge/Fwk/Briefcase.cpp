/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/Briefcase.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <iModelBridge/iModelBridgeFwk.h>
#include <iModelBridge/Fwk/IModelClientForBridges.h>
#include <WebServices/iModelHub/Client/Client.h>
#include <Bentley/Tasks/AsyncTasksManager.h>
#include <DgnPlatform/DgnProgressMeter.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include "Decrypt.h"

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_TASKS
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_SQLITE

static IModelClientForBridges* s_IModelHubClientForBridgesForTesting;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus getEnvironmentFromString(UrlProvider::Environment& environment, Utf8StringCR str)
    {
    if (str.EqualsI("dev"))
        environment = WebServices::UrlProvider::Environment::Dev;
    else if (str.EqualsI("qa"))
        environment = WebServices::UrlProvider::Environment::Qa;
    else if (str.EqualsI("release"))
        environment = WebServices::UrlProvider::Environment::Release;
    else
        return BSIERROR;
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnProgressMeter& iModelBridgeFwk::GetProgressMeter() const
    {
    static NopProgressMeter s_nopMeter;
    auto meter = T_HOST.GetProgressMeter();
    return meter? *meter: s_nopMeter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::IModelHubArgs::PrintUsage()
    {
    fwprintf(stderr, L"\n\
iModelHub:\n\
    --server-project=       (optional)  The name of a project in the iModel Hub Services. Optional if --server-project-guid is specified.\n\
    --server-project-guid= (optional)  The GUID of a project in the iModel Hub Services. Optional if --server-project is specified.\n\
    --server-repository=    (required)  The name of a repository in the project.\n\
    --server-user=          (required)  The username for the project.\n\
    --server-password=      (required)  The password for the project.\n\
    --server-retries=       (optional)  The number of times to retry a pull, merge, and/or push to iModelHub. Must be a value between 0 and 255.\n\
    --server-credentials-isEncrypted (optional) The user name and password passed in is encrypted.\n\
    \n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void            iModelBridgeFwk::DecryptCredentials(Http::Credentials& credentials)
    {
    Utf8String userName;
    CryptoHelper::DecryptString(userName, credentials.GetUsername());

    Utf8String password;
    CryptoHelper::DecryptString(password, credentials.GetPassword());

    credentials.SetUsername(userName);
    credentials.SetPassword(password);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::IModelHubArgs::ParseCommandLine(bvector<WCharCP>& bargptrs, int argc, WCharCP argv[])
    {
    m_isEncrypted = true;
    for (int iArg = 1; iArg < argc; ++iArg)
        {
        if (0 != BeStringUtilities::Wcsnicmp(argv[iArg], L"--server", 8))
            {
            // Not a fwk argument. We will forward it to the bridge.
            m_bargs.push_back(argv[iArg]);  // Keep the string alive
            bargptrs.push_back(m_bargs.back().c_str());
            continue;
            }

        m_parsedAny = true;

        if (argv[iArg] == wcsstr(argv[iArg], L"--server-retries="))
            {
            int n = atoi(getArgValue(argv[iArg]).c_str());
            if (n < 0 || 256 >= n)
                {
                fprintf(stderr, "%s - invalid retries value. Must be a value between 0 and 255\n", getArgValue(argv[iArg]).c_str());
                return BSIERROR;
                }
            m_maxRetryCount = (uint8_t)n;
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--server-repository="))
            {
            m_repositoryName = getArgValue(argv[iArg]);
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--server-user="))
            {
            m_credentials.SetUsername(getArgValue(argv[iArg]));
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--server-password="))
            {
            m_credentials.SetPassword(getArgValue(argv[iArg]));
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--server-project="))
            {
            m_bcsProjectId = getArgValue(argv[iArg]);
            m_haveProjectGuid = false;
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--server-project-guid="))
            {
            m_bcsProjectId = getArgValue(argv[iArg]);
            m_haveProjectGuid = true;
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--server-environment="))
            {
            if (BSISUCCESS != getEnvironmentFromString(m_environment, getArgValue(argv[iArg])))
                {
                fprintf(stderr, "%s - invalid server environment value\n", getArgValue(argv[iArg]).c_str());
                return BSIERROR;
                }
            continue;
            }
        if (argv[iArg] == wcsstr(argv[iArg], L"--server-credentials-isEncrypted"))
            {
            m_isEncrypted = true;
            continue;
            }

        BeAssert(false);
        fwprintf(stderr, L"%ls: unrecognized server argument\n", argv[iArg]);
        return BSIERROR;
        }

    if (m_isEncrypted)
        DecryptCredentials(m_credentials);
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus iModelBridgeFwk::IModelHubArgs::Validate(int argc, WCharCP argv[])
    {
    if (!m_parsedAny)
        return BSISUCCESS;

    if (m_bcsProjectId.empty())
        {
        GetLogger().fatal("missing project name or GUID");
        return BSIERROR;
        }

    if (m_repositoryName.empty())
        {
        GetLogger().fatal("missing repository name");
        return BSIERROR;
        }

    if (m_credentials.GetUsername().empty() || m_credentials.GetPassword().empty())
        {
        GetLogger().fatal("missing server username or password");
        return BSIERROR;
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::IModelBankArgs::PrintUsage()
    {
    fwprintf(stderr, L"\n\
iModelBank:\n\
    --imodel-bank-url=              The URL of the iModelBank server to use.\n\
    --imodel-bank-imodel-id=        The GUID of the iModel that is served by the bank at the specified URL.\n\
    --imodel-bank-access-token=     (optional) The JSON-encoded user access token, if needed by the project mgmt system.\n\
    --imodel-bank-retries=          (optional) The number of times to retry a pull, merge, and/or push to iModelBank. Must be a value between 0 and 255.\n\
    --imodel-bank-dms-credentials-isEncrypted (optional) The DMS user name and password passed in is encrypted.\n\
    \n");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::IModelBankArgs::ParseCommandLine(bvector<WCharCP>& bargptrs, int argc, WCharCP argv[])
    {
    for (int iArg = 1; iArg < argc; ++iArg)
        {
        if (0 != BeStringUtilities::Wcsnicmp(argv[iArg], L"--imodel-bank", 13))
            {
            // Not a fwk argument. We will forward it to the bridge.
            m_bargs.push_back(argv[iArg]);  // Keep the string alive
            bargptrs.push_back(m_bargs.back().c_str());
            continue;
            }

        m_parsedAny = true;

        if (argv[iArg] == wcsstr(argv[iArg], L"--imodel-bank-retries="))
            {
            int n = atoi(getArgValue(argv[iArg]).c_str());
            if (n < 0 || 256 >= n)
                {
                fprintf(stderr, "%s - invalid retries value. Must be a value between 0 and 255\n", getArgValue(argv[iArg]).c_str());
                return BSIERROR;
                }
            m_maxRetryCount = (uint8_t)n;
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--imodel-bank-url="))
            {
            m_url = getArgValue(argv[iArg]);
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--imodel-bank-access-token="))
            {
            m_accessToken = getArgValue(argv[iArg]);
            continue;
            }
        
        if (argv[iArg] == wcsstr(argv[iArg], L"--imodel-bank-imodel-id="))
            {
            m_iModelId = getArgValue(argv[iArg]);
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"--imodel-bank-dms-credentials-isEncrypted"))
            {
            m_dmsCredentialsEncrypted = true;
            continue;
            }

        BeAssert(false);
        fwprintf(stderr, L"%ls: unrecognized imodel-bank argument\n", argv[iArg]);
        return BSIERROR;
        }

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus iModelBridgeFwk::IModelBankArgs::Validate(int argc, WCharCP argv[])
    {
    if (!m_parsedAny)
        return BSISUCCESS;

    if (m_url.empty())
        {
        GetLogger().fatal("missing URL");
        return BSIERROR;
        }

    if (m_iModelId.empty())
        {
        GetLogger().fatal("missing iModel GUID");
        }

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Utf8String iModelBridgeFwk::IModelBankArgs::GetBriefcaseBasename() const
    {
    return m_iModelId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::Briefcase_MakeBriefcaseName()
    {
    BeAssert(!m_jobEnvArgs.m_stagingDir.empty() && m_jobEnvArgs.m_stagingDir.IsDirectory());
    // We always create a file in the localDir with the same name as the project. (It's too confusing otherwise.)
    m_briefcaseName = m_jobEnvArgs.m_stagingDir;
    m_briefcaseName.AppendToPath(WString(m_briefcaseBasename.c_str(), true).c_str());
    m_briefcaseName.append(L".bim");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::Briefcase_AcquireBriefcase()
    {
    m_lastServerError = EffectiveServerError::Unknown;

    GetLogger().info("AcquireBriefcase");
    GetProgressMeter().SetCurrentStepName("AcquireBriefcase");

    if (nullptr == m_client || !m_client->IsConnected())
        {
        GetLogger().error("Briefcase_AcquireBriefcase failed in nullptr == m_client || !m_client->IsConnected()");
        BeAssert(false);
        return BSIERROR;
        }

    if (m_briefcaseName.empty() || !m_briefcaseName.GetDirectoryName().IsDirectory())
        {
        GetLogger().errorv(L"Briefcase_AcquireBriefcase failed for %s", m_briefcaseName.c_str());
        BeAssert(false);
        return BSIERROR;
        }

    if (BSISUCCESS != m_client->AcquireBriefcase(m_briefcaseName, m_briefcaseBasename.c_str()))
        {
        if (Error::Id::iModelDoesNotExist == m_client->GetLastError().GetId())
            {
            m_lastServerError = EffectiveServerError::iModelDoesNotExist;
            GetLogger().errorv("%s - iModel not found in project\n", m_briefcaseBasename.c_str());
            }
        else
            {
            GetLogger().fatalv("%s - AcquireBriefcase failed\n", m_briefcaseBasename.c_str());
            }
        return BSIERROR;
        }

    auto rc = SaveBriefcaseId();

    if (BE_SQLITE_OK != rc)
        {
        if (BE_SQLITE_ERROR_SchemaUpgradeRequired == rc)
            {
            bool madeSchemaChanges = false;
            iModelBridge::OpenBimAndMergeSchemaChanges(rc, madeSchemaChanges, m_briefcaseName);
            rc = SaveBriefcaseId();
            }
        else
            {
            GetLogger().infov("Cannot open briefcase (error %x)\n", rc);
            }
        }
        
    return (BE_SQLITE_OK==rc)? BSISUCCESS: BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool iModelBridgeFwk::Briefcase_IsBriefcase()
    {
    BeAssert(!m_briefcaseName.empty());
    DbResult fileStatus;

    DgnDb::OpenParams openParams(DgnDb::OpenMode::ReadWrite);
    DgnDbPtr dgndb = DgnDb::OpenDgnDb(&fileStatus, m_briefcaseName, openParams);

    if (!dgndb.IsValid())
        return false;

    return dgndb->IsBriefcase();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::Briefcase_IModelHub_CreateRepository()
    {
    m_lastServerError = EffectiveServerError::Unknown;

    auto hubClient = dynamic_cast<IModelHubClientForBridges*>(m_client);

    if (nullptr == hubClient || !hubClient->IsConnected())
        {
        BeAssert(false);
        return BSIERROR;
        }

    if (!m_briefcaseName.GetFileNameWithoutExtension().EqualsI(WString(m_briefcaseBasename.c_str(), true)))
        {
        BeAssert(false && "Keep the name of the project and the dgndb the same. It's too confusing otherwise.");
        return BSIERROR;
        }

    GetProgressMeter().SetCurrentStepName("CreateRepository");
    GetLogger().infov("CreateRepository %s", m_briefcaseBasename.c_str());

    if (BSISUCCESS == hubClient->CreateRepository(m_briefcaseBasename.c_str(), m_briefcaseName))
        return BSISUCCESS;

    if (Error::Id::iModelAlreadyExists == hubClient->GetLastError().GetId())
        {
        // If the local file is not a briefcase and yet the server repository does exist, we'll assume that
        // a previous attempt to use a local file to create a server repo did succeed, but the program crashed
        // before we could clean up the local file.
        GetLogger().info(hubClient->GetLastError().GetMessage().c_str());
        return BSISUCCESS;
        }

    ReportIssue(hubClient->GetLastError().GetMessage());
    GetLogger().error(hubClient->GetLastError().GetMessage().c_str());
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::Briefcase_PullMergePush(Utf8CP desc)
    {
    StopWatch pullpushTimer(true);
    m_lastServerError = EffectiveServerError::Unknown;

    GetProgressMeter().SetCurrentStepName("PullMergePush");
    GetLogger().infov("PullMergePush %s", m_briefcaseBasename.c_str());

    if (!m_briefcaseDgnDb.IsValid() || !m_briefcaseDgnDb->IsDbOpen() || nullptr == m_client || !m_client->IsConnected())
        {
        GetLogger().error("Briefcase_PullMergePush failed in m_briefcaseDgnDb.IsValid() || !m_briefcaseDgnDb->IsDbOpen() || nullptr == m_client || !m_client->IsConnected()");
        BeAssert(false);
        return BSIERROR;
        }

    if (BSISUCCESS != m_client->OpenBriefcase(*m_briefcaseDgnDb))
        {
        ReportIssue(m_client->GetLastError().GetMessage());
        GetLogger().error(m_client->GetLastError().GetMessage().c_str());
        return BSIERROR;
        }

    auto status = m_client->PullMergeAndPush(desc);
    bool needsSchemaMerge = false;
    if (SUCCESS != status)
        {
        iModel::Hub::Error const& errorVal = m_client->GetLastError();
        if (iModel::Hub::Error::Id::MergeSchemaChangesOnOpen == errorVal.GetId())
            needsSchemaMerge = true;
        }

    if (needsSchemaMerge)
        {
        GetLogger().infov("PullAndMergeSchemaRevisions %s", m_briefcaseBasename.c_str());
        status = m_client->PullAndMergeSchemaRevisions(m_briefcaseDgnDb); // *** TRICKY: PullAndMergeSchemaRevisions closes and re-opens the briefcase, so m_briefcaseDgnDb is re-assigned!
        if (SUCCESS == status)
            status = m_client->PullMergeAndPush(desc);
        }

    m_client->CloseBriefcase();

    if (BSISUCCESS != status)
        {
        ReportIssue(m_client->GetLastError().GetMessage());
        GetLogger().error(m_client->GetLastError().GetMessage().c_str());
        return BSIERROR;
        }

    SetSyncState(SyncState::Pushed);
    LogPerformance(pullpushTimer, "Briefcase_PullMergePush to iModelHub");
    GetLogger().infov("PullMergePush %s : Done", m_briefcaseBasename.c_str());

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isLockExclusiveToJob(DgnLockCR lock)
    {
    // Only looking for exclusive locks
    if (lock.GetLevel() != LockLevel::Exclusive)
        return false;

    // A job can only "own" models and elements.
    return lock.GetType() == LockableType::Model || lock.GetType() == LockableType::Element;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::Briefcase_ReleaseAllPublicLocks()
    {
    if (!m_briefcaseDgnDb.IsValid())
        return BSIERROR;

    StopWatch releaseLockTimer(true);
    GetProgressMeter().SetCurrentStepName("ReleaseSharedLocks");

    DgnLockSet toRelease;

    IOwnedLocksIteratorPtr pIter = m_briefcaseDgnDb->BriefcaseManager().GetOwnedLocks(IBriefcaseManager::FastQuery::No);
    if (pIter.IsValid())
        {
        for (auto& iter = *pIter; iter.IsValid(); ++iter)
            {
            DgnLockCR lock = *iter;
            if (isLockExclusiveToJob(lock))    // I only keep locks on the things that I created
                continue;
            if (LockableType::Db == lock.GetType())
                continue;                                   // Don't demote/relinquish the shared lock on the Db. That would have the side effect of relinquishing *all* my locks.
            GetLogger().infov("Releasing lock: type=%d level=%d objid=%llx", lock.GetType(), lock.GetLevel(), lock.GetId());
            DgnLock lockReq(lock);
            lockReq.SetLevel(LockLevel::None);
            toRelease.insert(lockReq);
            }
        }

    if (!toRelease.empty())
        {
        auto rstatus = m_briefcaseDgnDb->BriefcaseManager().DemoteLocks(toRelease);
        if (RepositoryStatus::Success != rstatus)
            {
            GetLogger().errorv("DemoteLocks failed with error %x", (int)rstatus);
            return BSIERROR;
            }
        }

    SetSyncState(SyncState::Initial);
    LogPerformance(releaseLockTimer, "Briefcase_ReleaseAllPublicLocks to iModelHub");
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::Briefcase_Shutdown()
    {
    if (nullptr != m_client && m_client != s_IModelHubClientForBridgesForTesting)
        delete m_client;       // This relases the DgnDbBriefcase
        
    m_client = nullptr;

    Http::HttpClient::Uninitialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::SetIModelClientForBridgesForTesting(IModelClientForBridges& c)
    {
    s_IModelHubClientForBridgesForTesting = &c;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::Briefcase_Initialize(int argc, WCharCP argv[])
    {
    if (nullptr == DgnPlatformLib::QueryHost())
        {
        GetLogger().fatal("framework must initialize the host before calling this.");
        BeAssert(false);
        }

    // Note that we use the framework's asset directory, which is different from the bridge's assets dir.
    BeFileName assetsDir = m_jobEnvArgs.m_fwkAssetsDir;

    m_lastServerError = EffectiveServerError::Unknown;

    Http::HttpClient::Initialize(assetsDir);
    BeAssert(nullptr == m_client);
    WebServices::ClientInfoPtr clientInfo = nullptr;
    if (NULL != m_bridge)
        {
        m_bridge->_GetParams().m_jobRunCorrelationId = m_jobEnvArgs.m_jobRunCorrelationId;
        clientInfo = m_bridge->GetParamsCR().GetClientInfo();
        }

    if (s_IModelHubClientForBridgesForTesting)
        m_client = s_IModelHubClientForBridgesForTesting;
    else
        {
        if (m_useIModelHub)
            m_client = new IModelHubClient(*m_iModelHubArgs, clientInfo);
        else
            m_client = new IModelBankClient(*m_iModelBankArgs, clientInfo);
        }

    if (!m_client->IsConnected())
        {
        GetLogger().error("iModelBridgeFwk client is not connected.");
        return BSIERROR;
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::Briefcase_AcquireExclusiveLocks()
    {
    if (m_modelsInserted.empty())
        return BSISUCCESS;

    DgnDb::OpenParams openParams(DgnDb::OpenMode::Readonly);
    auto db = DgnDb::OpenDgnDb(nullptr, m_briefcaseName, openParams);
    if (!db.IsValid())
        {
        GetLogger().errorv(L"iModelBridgeFwk unable to open db %s for Briefcase_AcquireExclusiveLocks.", m_briefcaseName.c_str());
        return BSIERROR;
        }

    LockRequest req;
    for (auto mid : m_modelsInserted)
        {
        auto model = db->Models().GetModel(mid);
        if (!model.IsValid())
            continue;
        req.Insert(*model, LockLevel::Exclusive);
        }

    if (BSISUCCESS != m_client->AcquireLocks(req, *db))
        {
        GetLogger().info(m_client->GetLastError().GetMessage().c_str());
        return BSIERROR;
        }

    m_modelsInserted.clear();
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
IRepositoryManagerP iModelBridgeFwk::FwkRepoAdmin::_GetRepositoryManager(DgnDbR db) const
    {
    return m_fwk.GetRepositoryManager(db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IRepositoryManagerP iModelBridgeFwk::GetRepositoryManager(DgnDbR db) const
    {
    return m_client->GetRepositoryManager(db);
    }

