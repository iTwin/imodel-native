/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/IModelClientForBridges.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "IModelClientForBridges.h"
#include <DgnPlatform/DgnProgressMeter.h>

#include <WebServices/iModelHub/Client/Client.h>
#include <WebServices/Connect/ConnectSignInManager.h>
#include <WebServices/Configuration/UrlProvider.h>
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct ServiceLocalState : public IJsonLocalState
    {
    private:
        Json::Value m_map;

    public:
        JsonValueR GetStubMap()
            {
            return m_map;
            }
        //! Saves the Utf8String value in the local state.
        //! @note The nameSpace and key pair must be unique.
        void _SaveValue(Utf8CP nameSpace, Utf8CP key, Utf8StringCR value) override
            {
            Utf8PrintfString identifier("%s/%s", nameSpace, key);

            if (value == "null")
                {
                m_map.removeMember(identifier);
                }
            else
                {
                m_map[identifier] = value;
                }
            };
        //! Returns a stored Utf8String from the local state.
        //! @note The nameSpace and key pair uniquely identifies the value.
        Utf8String _GetValue(Utf8CP nameSpace, Utf8CP key) const override
            {
            Utf8PrintfString identifier("%s/%s", nameSpace, key);
            return m_map.isMember(identifier) ? m_map[identifier].asCString() : "null";
            };
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
static ServiceLocalState* getLocalState()
    {
    BeSystemMutexHolder threadSafety;
    static ServiceLocalState* s_localState;
    if (s_localState == nullptr)
        s_localState = new ServiceLocalState;
    return s_localState;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
IModelClientBase::IModelClientBase(WebServices::ClientInfoPtr info, uint8_t maxRetryCount, WebServices::UrlProvider::Environment environment, int64_t cacheTimeOutMs) : 
    m_clientInfo(info), m_maxRetryCount(maxRetryCount)
    {
    UrlProvider::Initialize(environment, cacheTimeOutMs, getLocalState());
    ClientHelper::Initialize(m_clientInfo, getLocalState());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
IModelBankClient::IModelBankClient(iModelBridgeFwk::IModelBankArgs const& args, WebServices::ClientInfoPtr info) : 
    IModelClientBase(info, args.m_maxRetryCount, WebServices::UrlProvider::Environment::Release, INT64_MAX),
    m_iModelId(args.m_iModelId)
    {
    ClientHelper::GetInstance()->SetUrl(args.m_url);
	m_client = ClientHelper::GetInstance()->SignInWithStaticHeader(args.m_accessToken);
    ClientHelper::GetInstance()->SetUrl("");
	}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
IModelHubClient::IModelHubClient(iModelBridgeFwk::IModelHubArgs const& args, WebServices::ClientInfoPtr info) : 
    IModelClientBase(info, args.m_maxRetryCount, args.m_environment, UrlProvider::DefaultTimeout),
    m_args(args)
    {
    Tasks::AsyncError serror;
    m_client = ClientHelper::GetInstance()->SignInWithCredentials(&serror, args.m_credentials);
    if (m_client == nullptr)
        {
        GetLogger().fatalv("Connect sign-in failed: %s - %s", serror.GetMessage().c_str(), serror.GetDescription().c_str());
        BeAssert(!IsConnected());
        return;
        }

    if (args.m_haveProjectGuid)
        {
        m_projectId = args.m_bcsProjectId;
        }
    else
        {
        WebServices::WSError wserror;
        m_projectId = ClientHelper::GetInstance()->QueryProjectId(&wserror, args.m_bcsProjectId);
        if (m_projectId.empty())
            {
            GetLogger().fatalv("Cannot find iModelHub project: [%s]", args.m_bcsProjectId);
            if (wserror.GetStatus() != WebServices::WSError::Status::None)
                GetLogger().fatalv("%s - %s", wserror.GetDisplayMessage().c_str(), wserror.GetDisplayDescription().c_str());
            m_client = nullptr;
            BeAssert(!IsConnected());
            return;
            }
        }

    BeAssert(IsConnected());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IRepositoryManagerP IModelClientBase::GetRepositoryManager(DgnDbR db)
    {
    return m_client->GetiModelAdmin()->_GetRepositoryManager(db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
iModel::Hub::iModelInfoPtr IModelBankClient::GetIModelInfo()
    {
    auto result = m_client->GetiModelById(m_iModelId.c_str(), m_iModelId.c_str())->GetResult();
    if (result.IsSuccess())
        return result.GetValue();
    
    m_lastServerError = result.GetError();
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
iModel::Hub::iModelInfoPtr IModelHubClient::GetIModelInfo()
    {
    auto result = m_client->GetiModelById(m_projectId, m_args.m_repositoryName)->GetResult();
    if (result.IsSuccess())
        return result.GetValue();
    
    result = m_client->GetiModelByName(m_projectId, m_args.m_repositoryName)->GetResult();
    if (result.IsSuccess())
        return result.GetValue();
    
    m_lastServerError = result.GetError();
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
static Http::Request::ProgressCallback getHttpProgressMeter()
    {
    Http::Request::ProgressCallback progress = [](double bytesTransfered, double bytesTotal)
        {
        if (nullptr != T_HOST.GetProgressMeter())
            T_HOST.GetProgressMeter()->ShowProgress();
        };
    return progress;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IModelClientBase::AcquireBriefcase(BeFileNameCR bcFileName, Utf8CP repoId)
    {
    iModel::Hub::iModelInfoPtr ri = GetIModelInfo();
    if (ri == nullptr)
        return BSIERROR;

    auto progress = getHttpProgressMeter();
    auto result = m_client->AcquireBriefcase(*ri, bcFileName, true, progress)->GetResult();
    if (result.IsSuccess())
        {
        auto createdPath = result.GetValue()->GetLocalPath();
        if (!createdPath.EqualsI(bcFileName))
            {
            BeAssert(false);
            m_lastServerError = Error(Error::Id::InvalidBriefcase);
            return BSIERROR;
            }
        return SUCCESS;
        }

    m_lastServerError = result.GetError();
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IModelClientBase::OpenBriefcase(Dgn::DgnDbR db)
    {
    auto progress = getHttpProgressMeter();
    auto result = m_client->OpenBriefcase(&db, false, progress)->GetResult();
    if (result.IsSuccess())
        {
        m_briefcase = result.GetValue();
        return SUCCESS;
        }
    m_lastServerError = result.GetError();
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IModelHubClient::CreateRepository(Utf8CP repoName, BeFileNameCR localDgnDb)
    {
    DgnDbPtr db = DgnDb::OpenDgnDb(nullptr, localDgnDb, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    if (!db.IsValid())
        {
        BeAssert(false);
        m_lastServerError = Error(Error::Id::InvalidBriefcase);
        return ERROR;
        }
    if (db->IsBriefcase())
        {
        BeAssert(false && "You say you need to create a repository, and yet you have a briefcase. You might be confused.");
        m_lastServerError = Error(Error::Id::InvalidBriefcase);
        return ERROR;
        }

    auto progress = getHttpProgressMeter();
    Utf8String description;
    db->QueryProperty(description, PropertySpec("dgn_proj", "description"));
    auto result = m_client->CreateNewiModel(m_projectId, *db, repoName, description, true, progress)->GetResult();

    if (result.IsSuccess())
        {
        return SUCCESS;
        }

    m_lastServerError = result.GetError();
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IModelClientBase::PullMergeAndPush(Utf8CP descr)
    {
    auto progress = getHttpProgressMeter();
    auto result = m_briefcase->PullMergeAndPush(descr, false, progress, progress, nullptr, m_maxRetryCount)->GetResult();
    if (result.IsSuccess())
        return SUCCESS;

    m_lastServerError = result.GetError();
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool IModelClientBase::SleepBeforeRetry()
    {
    int sleepTime = rand() % 5000;
    BeThreadUtilities::BeSleep(sleepTime);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isTemporaryError(Error error)
    {
    switch (error.GetId())
        {
        case Error::Id::AnotherUserPushing:
        case Error::Id::PullIsRequired:
        case Error::Id::DatabaseTemporarilyLocked:
        case Error::Id::iModelHubOperationFailed:
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IModelClientBase::PullAndMerge()
    {
    auto progress = getHttpProgressMeter();
    uint8_t attempt = 0;
    do {
        auto result = m_briefcase->PullAndMerge(progress)->GetResult();
        if (result.IsSuccess())
            return SUCCESS;
        m_lastServerError = result.GetError();
        }
    while (isTemporaryError(m_lastServerError) && (attempt++ < m_maxRetryCount) && SleepBeforeRetry());

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static ChangeSetsResult tryPullAndMergeSchemaRevisions(Dgn::DgnDbPtr& db, iModel::Hub::BriefcasePtr briefcase)
    {
    // Save briefcase path
    auto briefcasePath = db->GetFileName();

    // Get changeSets that should be applied during DgnDb reopen
    auto downloadChangeSetsResult = briefcase->GetiModelConnection().DownloadChangeSetsAfterId(db->Revisions().GetParentRevisionId())->GetResult();
    if (!downloadChangeSetsResult.IsSuccess())
        return downloadChangeSetsResult;

    auto downloadedChangeSets = downloadChangeSetsResult.GetValue();

    // Close briefcase, dgndb,…
    briefcase = nullptr;
    db->CloseDb();

    // Reopen dgndb with changesets that should be applied
    BeSQLite::DbResult dbres;
    
    bvector<DgnRevisionCP> changeSetVector;
    for (Dgn::DgnRevisionPtr& rev : downloadedChangeSets)
        changeSetVector.push_back(rev.get());

    Dgn::SchemaUpgradeOptions options(changeSetVector);
    options.SetUpgradeFromDomains(SchemaUpgradeOptions::DomainUpgradeOptions::SkipCheck); // We only want to merge schema revisions. We don't also want to import or upgrade required revisions.
    db = DgnDb::OpenDgnDb(&dbres, BeFileName(briefcasePath), Dgn::DgnDb::OpenParams(Dgn::DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, options));

    if (!db.IsValid())
        {
        NativeLogging::LoggingManager::GetLogger("iModelBridge")->errorv("Failed to open after applying schema revision. Error = %x", dbres);
        }

    return downloadChangeSetsResult;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IModelClientBase::PullAndMergeSchemaRevisions(Dgn::DgnDbPtr& db)
    {
    uint8_t attempt = 0;
    do {
        auto result = tryPullAndMergeSchemaRevisions(db, m_briefcase);
        if (result.IsSuccess())
            {
            OpenBriefcase(*db);
            return SUCCESS;
            }
        m_lastServerError = result.GetError();
        }
    while (isTemporaryError(m_lastServerError) && (attempt++ < m_maxRetryCount) && SleepBeforeRetry());
    CloseBriefcase();
    db = nullptr;
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IModelClientBase::AcquireLocks(LockRequest& req, DgnDbR db)
    {
    IBriefcaseManager::Request breq;
    std::swap(breq.Locks(), req);

    uint8_t attempt = 0;
    do  {
        auto response = GetRepositoryManager(db)->Acquire(breq, db);
        if (RepositoryStatus::Success == response.Result())
            return SUCCESS;
        m_lastServerError = Error(Error::Id::Unknown);
        }
    while (isTemporaryError(m_lastServerError) && (attempt++ < m_maxRetryCount) && SleepBeforeRetry());

    return ERROR;
    }
