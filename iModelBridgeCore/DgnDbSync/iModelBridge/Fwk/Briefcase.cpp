/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/Briefcase.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <iModelBridge/iModelBridgeFwk.h>
#include "DgnDbServerClientUtils.h"
#include <WebServices/iModelHub/Client/Client.h>
#include <Bentley/Tasks/AsyncTasksManager.h>
#include <DgnPlatform/DgnProgressMeter.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include "Decrypt.h"

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_TASKS
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_SQLITE

static iModelHubFX* s_iModelHubFXForTesting;

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
void iModelBridgeFwk::ServerArgs::PrintUsage()
    {
    fwprintf(stderr, L"\n\
SERVER:\n\
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
BentleyStatus iModelBridgeFwk::ServerArgs::ParseCommandLine(bvector<WCharCP>& bargptrs, int argc, WCharCP argv[])
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
BentleyStatus iModelBridgeFwk::ServerArgs::Validate(int argc, WCharCP argv[])
    {
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
void iModelBridgeFwk::Briefcase_MakeBriefcaseName()
    {
    BeAssert(!m_jobEnvArgs.m_stagingDir.empty() && m_jobEnvArgs.m_stagingDir.IsDirectory());
    // We always create a file in the localDir with the same name as the project. (It's too confusing otherwise.)
    m_briefcaseName = m_jobEnvArgs.m_stagingDir;
    m_briefcaseName.AppendToPath(WString(m_serverArgs.m_repositoryName.c_str(), true).c_str());
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

    if (nullptr == m_clientUtils || !m_clientUtils->IsSignedIn())
        {
        BeAssert(false);
        return BSIERROR;
        }

    if (m_briefcaseName.empty() || !m_briefcaseName.GetDirectoryName().IsDirectory())
        {
        BeAssert(false);
        return BSIERROR;
        }

    if (BSISUCCESS != m_clientUtils->AcquireBriefcase(m_briefcaseName, m_serverArgs.m_repositoryName.c_str()))
        {
        if (Error::Id::iModelDoesNotExist == m_clientUtils->GetLastError().GetId())
            {
            m_lastServerError = EffectiveServerError::iModelDoesNotExist;
            GetLogger().infov("%s - iModel not found in project\n", m_serverArgs.m_repositoryName.c_str());
            }
        else
            {
            GetLogger().fatalv("%s - AcquireBriefcase failed\n", m_serverArgs.m_repositoryName.c_str());
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
BentleyStatus iModelBridgeFwk::Briefcase_CreateRepository()
    {
    m_lastServerError = EffectiveServerError::Unknown;

    if (nullptr == m_clientUtils || !m_clientUtils->IsSignedIn())
        {
        BeAssert(false);
        return BSIERROR;
        }

    if (!m_briefcaseName.GetFileNameWithoutExtension().EqualsI(WString(m_serverArgs.m_repositoryName.c_str(), true)))
        {
        BeAssert(false && "Keep the name of the project and the dgndb the same. It's too confusing otherwise.");
        return BSIERROR;
        }

    GetProgressMeter().SetCurrentStepName("CreateRepository");
    GetLogger().infov("CreateRepository %s", m_serverArgs.m_repositoryName.c_str());

    if (BSISUCCESS == m_clientUtils->CreateRepository(m_serverArgs.m_repositoryName.c_str(), m_briefcaseName))
        return BSISUCCESS;

    if (Error::Id::iModelAlreadyExists == m_clientUtils->GetLastError().GetId())
        {
        // If the local file is not a briefcase and yet the server repository does exist, we'll assume that
        // a previous attempt to use a local file to create a server repo did succeed, but the program crashed
        // before we could clean up the local file.
        GetLogger().info(m_clientUtils->GetLastError().GetMessage().c_str());
        return BSISUCCESS;
        }

    ReportIssue(m_clientUtils->GetLastError().GetMessage());
    GetLogger().error(m_clientUtils->GetLastError().GetMessage().c_str());
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::Briefcase_PullMergePush(Utf8CP desc)
    {
    m_lastServerError = EffectiveServerError::Unknown;

    GetProgressMeter().SetCurrentStepName("PullMergePush");
    GetLogger().infov("PullMergePush %s", m_serverArgs.m_repositoryName.c_str());

    if (!m_briefcaseDgnDb.IsValid() || !m_briefcaseDgnDb->IsDbOpen() || nullptr == m_clientUtils || !m_clientUtils->IsSignedIn())
        {
        BeAssert(false);
        return BSIERROR;
        }

    if (BSISUCCESS != m_clientUtils->OpenBriefcase(*m_briefcaseDgnDb))
        {
        ReportIssue(m_clientUtils->GetLastError().GetMessage());
        GetLogger().error(m_clientUtils->GetLastError().GetMessage().c_str());
        return BSIERROR;
        }

    auto status = m_clientUtils->PullMergeAndPush(desc);
    bool needsSchemaMerge = false;
    if (SUCCESS != status)
        {
        iModel::Hub::Error const& errorVal = m_clientUtils->GetLastError();
        if (iModel::Hub::Error::Id::MergeSchemaChangesOnOpen == errorVal.GetId())
            needsSchemaMerge = true;
        }

    if (needsSchemaMerge)
        {
        GetLogger().infov("PullAndMergeSchemaRevisions %s", m_serverArgs.m_repositoryName.c_str());
        status = m_clientUtils->PullAndMergeSchemaRevisions(m_briefcaseDgnDb); // *** TRICKY: PullAndMergeSchemaRevisions closes and re-opens the briefcase, so m_briefcaseDgnDb is re-assigned!
        if (SUCCESS == status)
            status = m_clientUtils->PullMergeAndPush(desc);
        }

    m_clientUtils->CloseBriefcase();

    if (BSISUCCESS != status)
        {
        ReportIssue(m_clientUtils->GetLastError().GetMessage());
        GetLogger().error(m_clientUtils->GetLastError().GetMessage().c_str());
        return BSIERROR;
        }

    SetSyncState(SyncState::Pushed);

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::Briefcase_PullAndMerge()
    {
    m_lastServerError = EffectiveServerError::Unknown;

    GetProgressMeter().SetCurrentStepName("PullAndMerge");
    GetLogger().infov("PullAndMerge %s", m_serverArgs.m_repositoryName.c_str());

    if (!m_briefcaseDgnDb.IsValid() || !m_briefcaseDgnDb->IsDbOpen() || nullptr == m_clientUtils || !m_clientUtils->IsSignedIn())
        {
        BeAssert(false);
        return BSIERROR;
        }

    if (BSISUCCESS != m_clientUtils->OpenBriefcase(*m_briefcaseDgnDb))
        {
        GetLogger().error(m_clientUtils->GetLastError().GetMessage().c_str());
        return BSIERROR;
        }

    auto status = m_clientUtils->PullAndMerge();

    m_clientUtils->CloseBriefcase();

    if (BSISUCCESS != status)
        {
        GetLogger().error(m_clientUtils->GetLastError().GetMessage().c_str());
        return BSIERROR;
        }

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
            GetLogger().infov("Releasing lock: type=%d level=%d", lock.GetType(), lock.GetLevel());
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
            GetLogger().infov("DemoteLocks failed with error %x", (int)rstatus);
            return BSIERROR;
            }
        }

    SetSyncState(SyncState::Initial);

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::Briefcase_Shutdown()
    {
    if (nullptr != m_clientUtils && m_clientUtils != s_iModelHubFXForTesting)
        delete m_clientUtils;       // This relases the DgnDbBriefcase
        
    m_clientUtils = nullptr;

    Http::HttpClient::Uninitialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::SetiModelHubFXForTesting(iModelHubFX& c)
    {
    s_iModelHubFXForTesting = &c;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::Briefcase_Initialize(int argc, WCharCP argv[])
    {
    BeAssert((nullptr != DgnPlatformLib::QueryHost()) && "framework must initialize the host before calling this.");

    if (m_serverArgs.m_maxRetryCount > 0)
        GetLogger().infov("Server max retry count = %d", (int)m_serverArgs.m_maxRetryCount);

    // Note that we use the framework's asset directory, which is different from the bridge's assets dir.
    BeFileName assetsDir = m_jobEnvArgs.m_fwkAssetsDir;

    m_lastServerError = EffectiveServerError::Unknown;

    Http::HttpClient::Initialize(assetsDir);
    BeAssert(nullptr == m_clientUtils);
    WebServices::ClientInfoPtr clientInfo = nullptr;
    if (NULL != m_bridge)
        {
        m_bridge->_GetParams().m_jobRunCorrelationId = m_jobEnvArgs.m_jobRunCorrelationId;
        clientInfo = m_bridge->GetParamsCR().GetClientInfo();
        }

    if (s_iModelHubFXForTesting)
        m_clientUtils = s_iModelHubFXForTesting;
    else
        {
        m_clientUtils = new DgnDbServerClientUtils(m_serverArgs.m_environment, m_serverArgs.m_maxRetryCount, clientInfo, m_jobEnvArgs.m_imodelBankUrl);
        }

    Tasks::AsyncError serror;
    if (BSISUCCESS != m_clientUtils->SignIn(&serror, m_serverArgs.m_credentials))
        {
        GetLogger().fatalv("briefcase sign in failed: %s - %s", serror.GetMessage().c_str(), serror.GetDescription().c_str());
        return BSIERROR;
        }

    if (m_serverArgs.m_haveProjectGuid)
        {
        m_clientUtils->SetProjectId(m_serverArgs.m_bcsProjectId.c_str());
        }
    else
        {
        WebServices::WSError wserror;
        if (BSISUCCESS != m_clientUtils->QueryProjectId(&wserror, m_serverArgs.m_bcsProjectId))
            {
            GetLogger().fatalv("Cannot find iModelHub project: [%s]", m_serverArgs.m_bcsProjectId.c_str());
            if (wserror.GetStatus() != WebServices::WSError::Status::None)
                GetLogger().fatalv("%s - %s", wserror.GetDisplayMessage().c_str(), wserror.GetDisplayDescription().c_str());
            return BSIERROR;
            }
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
        return BSIERROR;

    LockRequest req;
    for (auto mid : m_modelsInserted)
        {
        auto model = db->Models().GetModel(mid);
        if (!model.IsValid())
            continue;
        req.Insert(*model, LockLevel::Exclusive);
        }

    if (BSISUCCESS != m_clientUtils->AcquireLocks(req, *db))
        {
        GetLogger().info(m_clientUtils->GetLastError().GetMessage().c_str());
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
    return m_clientUtils->GetRepositoryManager(db);
    }

