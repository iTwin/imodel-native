/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/DgnDbServerClientUtils.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnDbServerClientUtils.h"
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
    // MT Note: C++11 guarantees that the following line of code will be executed only once and in a thread-safe manner:
    ServiceLocalState* s_localState = new ServiceLocalState;
    return s_localState;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
static WebServices::ClientInfoPtr getClientInfo()
    {
    static Utf8CP s_productId = "1654"; // Navigator Desktop
    // MT Note: C++11 guarantees that the following line of code will be executed only once and in a thread-safe manner:
    static WebServices::ClientInfoPtr s_clientInfo = WebServices::ClientInfoPtr(
        new WebServices::ClientInfo("Bentley-Test", BeVersion(1, 0), "{41FE7A91-A984-432D-ABCF-9B860A8D5360}", "TestDeviceId", "TestSystem", s_productId));
    return s_clientInfo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbServerClientUtils::DgnDbServerClientUtils(WebServices::UrlProvider::Environment environment, uint8_t nretries)
    {
    m_maxRetryCount = nretries;
    UrlProvider::Initialize(environment, UrlProvider::DefaultTimeout, getLocalState());
    ClientHelper::Initialize(getClientInfo(), getLocalState());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnDbServerClientUtils::SignIn(Tasks::AsyncError* errorOut, Credentials credentials)
    {
    m_client = ClientHelper::GetInstance()->SignInWithCredentials(errorOut, credentials);
    return (m_client == nullptr)? BSIERROR: BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnDbServerClientUtils::QueryProjectId(WebServices::WSError* errorOut, Utf8StringCR bcsProjectName)
    {
    auto pid = ClientHelper::GetInstance()->QueryProjectId(errorOut, bcsProjectName);
	m_projectId = pid;
    return pid.empty()? BSIERROR: BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IRepositoryManagerP DgnDbServerClientUtils::GetRepositoryManager(DgnDbR db)
    {
    return m_client->GetiModelAdmin()->_GetRepositoryManager(db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static iModelInfoPtr getRepositoryInfo(Error& err, Client& client, Utf8String projectId, Utf8StringCR repoId)
    {
    auto result = client.GetiModelById(projectId, repoId)->GetResult();
    if (result.IsSuccess())
        return result.GetValue();
    
    result = client.GetiModelByName(projectId, repoId)->GetResult();
    if (result.IsSuccess())
        return result.GetValue();
    
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
StatusInt DgnDbServerClientUtils::AcquireBriefcase(BeFileNameCR bcFileName, Utf8CP repoId)
    {
    auto ri = getRepositoryInfo(m_lastServerError, *m_client, m_projectId, repoId);
    if (ri.IsNull())
        {
        m_lastServerError = Error::Id::iModelDoesNotExist;
        return BSIERROR;
        }

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
StatusInt DgnDbServerClientUtils::OpenBriefcase(Dgn::DgnDbR db)
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
StatusInt DgnDbServerClientUtils::CreateRepository(Utf8CP repoName, BeFileNameCR localDgnDb)
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
StatusInt DgnDbServerClientUtils::PullMergeAndPush(Utf8CP descr)
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
static bool delayBeforeRetry()
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
StatusInt DgnDbServerClientUtils::PullAndMerge()
    {
    auto progress = getHttpProgressMeter();
    uint8_t attempt = 0;
    do {
        auto result = m_briefcase->PullAndMerge(progress)->GetResult();
        if (result.IsSuccess())
            return SUCCESS;
        m_lastServerError = result.GetError();
        }
    while (isTemporaryError(m_lastServerError) && (attempt++ < m_maxRetryCount) && delayBeforeRetry());

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

    // Close briefcase, dgndb,�
    briefcase = nullptr;
    db->CloseDb();

    // Reopen dgndb with changesets that should be applied
    BeSQLite::DbResult dbres;
    
    bvector<DgnRevisionCP> changeSetVector;
    for (Dgn::DgnRevisionPtr& rev : downloadedChangeSets)
        changeSetVector.push_back(rev.get());

    Dgn::SchemaUpgradeOptions options(changeSetVector);
    db = DgnDb::OpenDgnDb(&dbres, BeFileName(briefcasePath), Dgn::DgnDb::OpenParams(Dgn::DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, options));
    BeAssert(db.IsValid());

    return downloadChangeSetsResult;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnDbServerClientUtils::PullAndMergeSchemaRevisions(Dgn::DgnDbPtr& db)
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
    while (isTemporaryError(m_lastServerError) && (attempt++ < m_maxRetryCount) && delayBeforeRetry());
    CloseBriefcase();
    db = nullptr;
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnDbServerClientUtils::AcquireLocks(LockRequest& req, DgnDbR db)
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
    while (isTemporaryError(m_lastServerError) && (attempt++ < m_maxRetryCount) && delayBeforeRetry());

    return ERROR;
    }
