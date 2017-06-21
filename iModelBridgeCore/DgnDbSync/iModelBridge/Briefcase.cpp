/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Briefcase.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <iModelBridge/iModelBridgeFwk.h>
#include "DgnDbServerClientUtils.h"
#include <WebServices/iModelHub/Client/Client.h>
#include <Bentley/Tasks/AsyncTasksManager.h>
#include <DgnPlatform/DgnProgressMeter.h>
#include <DgnPlatform/DgnPlatformLib.h>

    // *** WIP_WSCLIENT - WSClient/DgnDbServer should not depend on DgnClientFx
#include <DgnClientFx/DgnClientFxL10N.h>

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_TASKS
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_SQLITE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String getArgValue(WCharCP arg)
    {
    WString argValue(arg);
    argValue = argValue.substr(argValue.find_first_of('=', 0) + 1);
    argValue.Trim(L"\"");
    argValue.Trim();
    return Utf8String(argValue);
    }

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
    --server-project=       (required)  The name of a project in the iModel Hub Services.\n\
    --server-repository=    (required)  The name of a repository in the project.\n\
    --server-user=          (required)  The username for the project.\n\
    --server-password=      (required)  The password for the project.\n\
    ");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::ServerArgs::ParseCommandLine(bvector<WCharCP>& bargptrs, int argc, WCharCP argv[])
    {
    for (int iArg = 1; iArg < argc; ++iArg)
        {
        if (0 != BeStringUtilities::Wcsnicmp(argv[iArg], L"--server", 8))
            {
            // Not a fwk argument. We will forward it to the bridge.
            m_bargs.push_back(argv[iArg]);  // Keep the string alive
            bargptrs.push_back(m_bargs.back().c_str());
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
            m_bcsProjectName = getArgValue(argv[iArg]);
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

        BeAssert(false);
        fwprintf(stderr, L"%ls: unrecognized server argument\n", argv[iArg]);
        return BSIERROR;
        }

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus iModelBridgeFwk::ServerArgs::Validate(int argc, WCharCP argv[])
    {
    if (m_bcsProjectName.empty())
        {
        GetLogger().fatal("missing project name");
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
            m_lastServerError = EffectiveServerError::iModelDoesNotExist;
        GetLogger().infov("%s - iModel not found in project\n", m_serverArgs.m_repositoryName.c_str());
        return BSIERROR;
        }

    return BSISUCCESS;
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
        GetLogger().error(m_clientUtils->GetLastError().GetMessage().c_str());
        return BSIERROR;
        }

    auto status = m_clientUtils->PullMergeAndPush(desc);

    m_clientUtils->CloseBriefcase();

    if (BSISUCCESS != status)
        {
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
BentleyStatus iModelBridgeFwk::Briefcase_ReleaseSharedLocks()
    {
    GetProgressMeter().SetCurrentStepName("ReleaseSharedLocks");

    DgnLockSet toRelease;

    IOwnedLocksIteratorPtr pIter = m_briefcaseDgnDb->BriefcaseManager().GetOwnedLocks(IBriefcaseManager::FastQuery::No);
    if (pIter.IsValid())
        {
        for (auto& iter = *pIter; iter.IsValid(); ++iter)
            {
            DgnLockCR lock = *iter;
            if (LockLevel::Exclusive == lock.GetLevel())    // I only keep locks on the things that I created
                continue;
            if (LockableType::Db == lock.GetType())
                continue;                                   // Don't demote/relinquish the shared lock on the Db. That would have the side effect of relinquishing *all* my locks.
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
    if (nullptr != m_clientUtils)
        {
        delete m_clientUtils;       // This relases the DgnDbBriefcase
        m_clientUtils = nullptr;
        }
    //AsyncTasksManager::StopThreadingAndWait();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeFwk::Briefcase_Initialize(int argc, WCharCP argv[])
    {
    BeAssert((nullptr != DgnPlatformLib::QueryHost()) && "framework must initialize the host before calling this.");

    // Note that we use the framework's asset directory, which is different from the bridge's assets dir.
    BeFileName assetsDir = m_jobEnvArgs.m_fwkAssetsDir;

    // *** TRICKY: We are *adding*  WSClient's sqlang files to the set of sqlang files that were *already registered* for the bridge (by the host)
    BeFileName wsclientSqlangDir(assetsDir);
    wsclientSqlangDir.AppendToPath(L"sqlang/DgnClientFx_en.sqlang.db3");
    auto wsclientSqlangFiles = BeSQLite::L10N::SqlangFiles(wsclientSqlangDir);
    auto bridgeSqlangFiles = T_HOST._SupplySqlangFiles();
    DgnClientFx::DgnClientFxL10N::Initialize(bridgeSqlangFiles, wsclientSqlangFiles);

    m_lastServerError = EffectiveServerError::Unknown;

    Http::HttpClient::Initialize(assetsDir);
    BeAssert(nullptr == m_clientUtils);
    m_clientUtils = new DgnDbServerClientUtils(m_serverArgs.m_environment);
    Tasks::AsyncError serror;
    if (BSISUCCESS != m_clientUtils->SignIn(&serror, m_serverArgs.m_credentials))
        {
        GetLogger().fatalv("briefcase sign in failed: %s - %s", serror.GetMessage().c_str(), serror.GetDescription().c_str());
        return BSIERROR;
        }

    WebServices::WSError wserror;
    if (BSISUCCESS != m_clientUtils->QueryProjectId(&wserror, m_serverArgs.m_bcsProjectName))
        {
        GetLogger().fatalv("Cannot find BCS project: [%s]", m_serverArgs.m_bcsProjectName.c_str());
        if (wserror.GetStatus() != WebServices::WSError::Status::None)
            GetLogger().fatalv("%s - %s", wserror.GetDisplayMessage().c_str(), wserror.GetDisplayDescription().c_str());
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

