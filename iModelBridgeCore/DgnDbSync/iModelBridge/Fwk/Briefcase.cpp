/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <iModelBridge/iModelBridgeFwk.h>
#include <iModelBridge/IModelClientForBridges.h>
#include <WebServices/iModelHub/Client/Client.h>
#include <Bentley/Tasks/AsyncTasksManager.h>
#include <DgnPlatform/DgnProgressMeter.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include "Decrypt.h"
#include "../iModelBridgeSettings.h"

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_TASKS
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_SQLITE

static IModelClientForBridges* s_clientForTesting;

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
    else if (str.EqualsI("Perf"))
        environment = WebServices::UrlProvider::Environment::Perf;
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
    --server-oidcCallBackUrl= (optional) The OIDC callback url to receive access token instead of credentials. \n\
    --server-briefcaseId= (optional) The briefcase Id incase we do not have the original BIM file\n\
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
            if (n < 0 || 256 <= n)
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

        if (argv[iArg] == wcsstr(argv[iArg], L"--server-oidcCallBackUrl="))
            {
            m_callBackurl = getArgValue(argv[iArg]);
            continue;
            }
        if (argv[iArg] == wcsstr(argv[iArg], L"--server-briefcaseId="))
            {
            m_briefcaseId = BeSQLite::BeBriefcaseId(atoi(getArgValue(argv[iArg]).c_str()));
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

    if ((m_credentials.GetUsername().empty() || m_credentials.GetPassword().empty()) && m_callBackurl.empty())
        {
        GetLogger().fatal("missing server username or password or OIDC callback url.");
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
            if (n < 0 || 256 <= n)
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

        if (argv[iArg] == wcsstr(argv[iArg], L"--imodel-bank-imodel-name="))
            {
            m_iModelName = getArgValue(argv[iArg]);
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
    if (!m_iModelName.empty())
        return m_iModelName;
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
BentleyStatus iModelBridgeFwk::Briefcase_AcquireBriefcase(iModelBridgeFwk::FwkContext& context)
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
    BeSQLite::BeBriefcaseId briefcaseId = GetBriefcaseId();
    if (!briefcaseId.IsValid())//We were not able to find a passed or cached briefcaseId. Let's check the settings service.
        {
        context.m_settings.GetBriefCaseId(m_dmsServerArgs.GetDocumentGuid(), briefcaseId);
        }
    
    if (briefcaseId.IsValid())
        {
        if (BSISUCCESS != m_client->RestoreBriefcase(m_briefcaseName, m_briefcaseBasename.c_str(), briefcaseId))
            return BSIERROR;
        }
    else if (BSISUCCESS != m_client->AcquireBriefcase(m_briefcaseName, m_briefcaseBasename.c_str()))
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

    auto rc = SaveBriefcaseId(briefcaseId);

    if (BE_SQLITE_OK != rc)
        {
        if (BE_SQLITE_ERROR_SchemaUpgradeRequired == rc)
            {
            bool madeSchemaChanges = false;
            iModelBridge::OpenBimAndMergeSchemaChanges(rc, madeSchemaChanges, m_briefcaseName);
            rc = SaveBriefcaseId(briefcaseId);
            }
        else
            {
            GetLogger().infov("Cannot open briefcase (error %x)\n", rc);
            }
        }
    
    if (BE_SQLITE_OK == rc)
        context.m_settings.SetBriefCaseId(m_dmsServerArgs.GetDocumentGuid(), briefcaseId);

    return (BE_SQLITE_OK==rc)? BSISUCCESS: BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool iModelBridgeFwk::Briefcase_IsBriefcase()
    {
    BeAssert(!m_briefcaseName.empty());
    DbResult fileStatus;

    DgnDb::OpenParams openParams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Exclusive);
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
BentleyStatus iModelBridgeFwk::Briefcase_PullMergePush(Utf8CP descIn, bool doPullAndMerge, bool doPush)
    {
    BeAssert(doPullAndMerge || doPush);
    bool doPullMergeAndPush = doPullAndMerge && doPush;
    Utf8CP opName = doPullMergeAndPush? "PullMergePush": doPullAndMerge? "PullAndMerge": "Push";

    StopWatch pullpushTimer(true);
    m_lastServerError = EffectiveServerError::Unknown;

    if (doPush)
        m_lastBridgePushStatus = iModelBridge::IBriefcaseManager::PushStatus::Success;

    Utf8String comment(descIn);
    if (m_bridge)
        comment = m_bridge->_FormatPushComment(*m_briefcaseDgnDb, descIn);

    if (comment.length() > 350) // iModelHub imposes a hard limit on ChangeSet description length.
        comment.resize(350);

    GetProgressMeter().SetCurrentStepName(opName);
    GetLogger().infov("%s %s %s", opName, m_briefcaseBasename.c_str(), comment.c_str());

    if (!m_briefcaseDgnDb.IsValid() || !m_briefcaseDgnDb->IsDbOpen() || nullptr == m_client || !m_client->IsConnected())
        {
        GetLogger().errorv("%s failed in m_briefcaseDgnDb.IsValid() || !m_briefcaseDgnDb->IsDbOpen() || nullptr == m_client || !m_client->IsConnected()", opName);
        BeAssert(false);
        return BSIERROR;
        }

    if (BSISUCCESS != m_client->OpenBriefcase(*m_briefcaseDgnDb))
        {
        ReportIssue(m_client->GetLastError().GetMessage());
        GetLogger().error(m_client->GetLastError().GetMessage().c_str());
        return BSIERROR;
        }

    auto status = doPullMergeAndPush? m_client->PullMergeAndPush(comment.c_str()): 
                  doPullAndMerge?     m_client->PullAndMerge():
                                      m_client->Push(comment.c_str());
    bool needsSchemaMerge = false;
    if (SUCCESS != status)
        {
        iModel::Hub::Error const& errorVal = m_client->GetLastError();
        if (iModel::Hub::Error::Id::MergeSchemaChangesOnOpen == errorVal.GetId())
            needsSchemaMerge = true;

        if (doPush)
            {
            switch (errorVal.GetId())
                {
                case iModel::Hub::Error::Id::PullIsRequired:
                case iModel::Hub::Error::Id::AnotherUserPushing:
                    m_lastBridgePushStatus = iModelBridge::IBriefcaseManager::PushStatus::PullIsRequired;
                    break;
                default:
                    m_lastBridgePushStatus = iModelBridge::IBriefcaseManager::PushStatus::UnknownError;
                }
            }
        }

    if (needsSchemaMerge)
        {
        GetLogger().infov("PullAndMergeSchemaRevisions %s", m_briefcaseBasename.c_str());
        status = m_client->PullAndMergeSchemaRevisions(m_briefcaseDgnDb); // *** TRICKY: PullAndMergeSchemaRevisions closes and re-opens the briefcase, so m_briefcaseDgnDb is re-assigned!
        if (SUCCESS == status)
            status = m_client->PullMergeAndPush(comment.c_str());
        }

    m_client->CloseBriefcase();

    if (BSISUCCESS != status)
        {
        ReportIssue(m_client->GetLastError().GetMessage());
        GetLogger().error(m_client->GetLastError().GetMessage().c_str());
        return BSIERROR;
        }

    SetSyncState(SyncState::Pushed);
    LogPerformance(pullpushTimer, "Briefcase_PullMergePush(%d,%d) to iModelHub", doPullAndMerge, doPush);
    GetLogger().infov("%s %s : Done", opName, m_briefcaseBasename.c_str());

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/19
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isRepositoryLinkElement(DgnElementId const& id, DgnDbR db) {return db.Elements().Get<RepositoryLink>(id).IsValid();}
static DgnElementId elementId(DgnLockCR lock) {return DgnElementId(lock.GetLockableId().GetId().GetValue());}
static DgnModelId modelId(DgnLockCR lock) {return DgnModelId(lock.GetLockableId().GetId().GetValue());}
static bool isRepositoryModel(DgnLockCR lock) {return modelId(lock) == DgnModel::RepositoryModelId();}
#ifndef NDEBUG
static bool isShareOnlyElement(DgnLockCR lock, DgnDbR db)
    {
    auto id = elementId(lock);
    return id == db.Elements().GetRootSubjectId()
         || id == db.Elements().GetDictionaryPartitionId();
    }
static bool isShareOnlyModel(DgnLockCR lock)
    {
    auto id = modelId(lock);
    return id == DgnModel::DictionaryId()
        || id == DgnModel::RepositoryModelId();
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/19
+---------------+---------------+---------------+---------------+---------------+------*/
static bool shouldBridgeHoldThisLock(DgnLockCR lock, DgnDbR db)
    {
    bool isExclusive = (lock.GetLevel() == LockLevel::Exclusive);

    if (lock.GetType() == LockableType::Element)
        {
        // A bridge holds onto locks on the elements that it created
        // ... except for RepositoryLink elements.(4)
        BeAssert((!isExclusive || !isShareOnlyElement(lock, db)) && "bridge must never lock a root subject element exclusively");
        return isExclusive && !isRepositoryLinkElement(elementId(lock), db);
        }

    if (lock.GetType() == LockableType::Model)
        {
        // A bridge holds onto locks on the models that it created.
        // A bridge must hold onto its shared lock on the repository model.(1)
        // (A bridge does NOT hold onto the shared lock on the DictionaryModel.(2))
        BeAssert((!isExclusive || !isShareOnlyModel(lock)) && "bridge must never lock a shared definitions model exclusively");
        return isExclusive || isRepositoryModel(lock);
        }
    
    if (LockableType::Db == lock.GetType())
        {
        // A bridge must hold onto its shared lock on the Db.(3)
        return true;
        }

    // A bridge never holds the Schema, CodeSpecs, or other locks that guard the schema channel.
    return false;

    // (1) The repository model is where the bridge created its JobSubject and its child Subject. 
    // The bridge must retain its exclusive lock on those Subjects. If we were to release
    // The bridge's shared lock on that model, that would auto-release locks on all elements in that model.

    // (2) The DictionaryModel contains shared definitions, such as Categories, which are created/contributed
    // by many bridges. No bridge should hold onto its exclusive lock on any element that it creates in
    // the DictionaryModel. Releasing the bridge's lock on the DictionaryModel itself will have the effect
    // of releasing its locks on the elements in it.

    // (3) Releasing the shared lock on the Db itself would have the side-effect of relinquishing *all* my locks.

    // (4) RepositoryLink elements are stored in the repository model and are shared by multiple bridges.
    // There is no way to predict which bridge will create a RepositoryLink element. A bridge may create
    // a RepositoryLink that corresponds to a file that is not assigned to that bridge, if that is necessary
    // in order to traverse references from that file and to create subjects for them. 
    // Nevertheless. the bridge to which the file is assigned must be able to update the RepositoryLink element.
    // So, we must keep RepositoryLink elements unlocked.
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
            if (!shouldBridgeHoldThisLock(lock, *m_briefcaseDgnDb))
                {
                GetLogger().infov("Releasing lock: type=%d level=%d objid=%llx", lock.GetType(), lock.GetLevel(), lock.GetId().GetValue());
                DgnLock lockReq(lock);
                lockReq.SetLevel(LockLevel::None);
                toRelease.insert(lockReq);
                }
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

    BeAssert(!iModelBridge::HoldsSchemaLock(*m_briefcaseDgnDb));

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::Briefcase_Shutdown()
    {
    if (nullptr != m_client && m_client != s_clientForTesting)
        delete m_client;       // This relases the DgnDbBriefcase
        
    m_client = nullptr;

    Http::HttpClient::Uninitialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::SetIModelClientForBridgesForTesting(IModelClientForBridges& c)
    {
    s_clientForTesting = &c;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::ClearIModelClientForBridgesForTesting()
    {
    s_clientForTesting = nullptr;
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
    Http::HttpClient::Reinitialize(); // In case Unintialize was called prior to this.
    
    BeAssert(nullptr == m_client);
    WebServices::ClientInfoPtr clientInfo = nullptr;
    if (NULL != m_bridge)
        {
        m_bridge->_GetParams().m_jobRunCorrelationId = m_jobEnvArgs.m_jobRunCorrelationId;
        clientInfo = m_bridge->GetParamsCR().GetClientInfo();
        }

    if (s_clientForTesting)
        m_client = s_clientForTesting;
    else
        {
        if (m_useIModelHub)
            {
            IModelHubClient* client = new IModelHubClient(*m_iModelHubArgs, clientInfo);
            m_client = client;
            }
        else
            m_client = new IModelBankClient(*m_iModelBankArgs, clientInfo);
        }

    if (!m_client->IsConnected())
        {
        GetLogger().error("iModelBridgeFwk client is not connected.");
        return BSIERROR;
        }

    if (nullptr != m_bridge)
        m_bridge->_GetParams().SetConnectSigninManager(m_client->GetConnectSignInManager());

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

