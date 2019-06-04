/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <Bentley/RefCounted.h>
#include <Bentley/Tasks/AsyncError.h>
#include <DgnPlatform/DgnPlatform.h>
#include <WebServices/iModelHub/Client/Client.h>
#include <WebServices/iModelHub/Client/Error.h>
#include <BeHttp/Credentials.h>
#include <BeHttp/HttpRequest.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>
#include <WebServices/iModelHub/Client/ClientHelper.h>
#include <iModelBridge/iModelBridgeFwk.h>

#pragma warning(push)
#pragma warning(disable : 4251) // warning C4251: 'BentleyB0200::Dgn::IModelClientBase::m_client': class 'BentleyB0200::RefCountedPtr<BentleyB0200::iModel::Hub::Client>' needs to have dll-interface to be used by clients of struct 'BentleyB0200::Dgn::IModelClientBase' (compiling source file D:\bim0200dev_2\source\DgnDbSync\iModelBridge\Fwk\IModelClientForBridges.cpp)
#pragma warning(disable : 4275) // warning C4275: non dll-interface struct 'BentleyB0200::Dgn::IModelHubClientForBridges' used as base for dll-interface struct 'BentleyB0200::Dgn::IModelHubClient' (compiling source file D:\bim0200dev_2\source\DgnDbSync\iModelBridge\Fwk\iModelBridgeFwk.cpp)
#pragma warning(disable : 4250) // warning C4250: 'BentleyB0200::Dgn::IModelHubClient': inherits 'BentleyB0200::Dgn::IModelClientBase::BentleyB0200::Dgn::IModelClientBase::GetDgnRevisions' via dominance (compiling source file D:\bim0200dev_2\source\DgnDbSync\iModelBridge\Fwk\IModelClientForBridges.cpp)

BEGIN_BENTLEY_DGN_NAMESPACE
typedef std::shared_ptr<struct WebServices::IConnectSignInManager> OidcSignInManagerPtr;
// ========================================================================================================
//! Interface to be adopted by a class that defines the interface between iModelBridgeFwk and an IModel server.
// ========================================================================================================
struct IMODEL_BRIDGE_FWK_EXPORT IModelClientForBridges
{
    virtual ~IModelClientForBridges() {}

    virtual StatusInt AcquireBriefcase(BeFileNameCR bcFileName, Utf8CP repositoryName) = 0;
    virtual StatusInt RestoreBriefcase(BeFileNameCR bcFileName, Utf8CP repositoryName, BeSQLite::BeBriefcaseId briefcaseId) = 0;
    virtual StatusInt OpenBriefcase(Dgn::DgnDbR db) = 0;
    virtual StatusInt Push(Utf8CP) = 0;
    virtual StatusInt PullMergeAndPush(Utf8CP) = 0;
    virtual StatusInt PullAndMerge() = 0;
    virtual StatusInt PullAndMergeSchemaRevisions(Dgn::DgnDbPtr& db) = 0;
    virtual iModel::Hub::Error const& GetLastError() const = 0;
    virtual IRepositoryManagerP GetRepositoryManager(DgnDbR db) = 0;
    virtual void CloseBriefcase() = 0;
    virtual StatusInt AcquireLocks(LockRequest&, DgnDbR) = 0;
    virtual iModel::Hub::iModelInfoPtr GetIModelInfo() = 0;

    virtual bool IsConnected() const = 0;

};

// ========================================================================================================
//! Interface to be adopted by a class that defines the interface between iModelBridgeFwk and iModelHub.
// ========================================================================================================
struct IModelHubClientForBridges : virtual IModelClientForBridges
{
    virtual ~IModelHubClientForBridges() {}

    virtual StatusInt CreateRepository(Utf8CP repoName, BeFileNameCR localDgnDb) = 0;
    virtual BentleyStatus DeleteRepository() = 0;
};

// ========================================================================================================
// Provides an interface to an IModel server.
// ========================================================================================================
struct IMODEL_BRIDGE_FWK_EXPORT IModelClientBase : virtual IModelClientForBridges
{
protected:
    iModel::Hub::ClientPtr m_client;
    iModel::Hub::BriefcasePtr m_briefcase;
    iModel::Hub::Error m_lastServerError;
    uint8_t m_maxRetryCount {};
    WebServices::ClientInfoPtr m_clientInfo;
    OidcSignInManagerPtr m_oidcMgr;
public:
    IModelClientBase(WebServices::ClientInfoPtr ci, uint8_t maxRetryCount, WebServices::UrlProvider::Environment, int64_t cacheTimeOutMs);

    iModel::Hub::ClientPtr GetImodelHubClientPtr() const {return m_client;}
    iModel::Hub::BriefcasePtr GetImodelHubBriefcase() const {return m_briefcase;}

    bool IsConnected() const override {return m_client.IsValid();}

    StatusInt AcquireBriefcase(BeFileNameCR bcFileName, Utf8CP repoId) override;
    StatusInt OpenBriefcase(Dgn::DgnDbR db) override;
    StatusInt Push(Utf8CP) override;
    StatusInt PullMergeAndPush(Utf8CP) override;
    StatusInt PullAndMerge() override;
    StatusInt PullAndMergeSchemaRevisions(Dgn::DgnDbPtr& db) override;
    StatusInt RestoreBriefcase(BeFileNameCR bcFileName, Utf8CP repositoryName, BeSQLite::BeBriefcaseId briefcaseId) override;
    iModel::Hub::Error const& GetLastError() const override {return m_lastServerError;}
    IRepositoryManagerP GetRepositoryManager(DgnDbR db) override;
    void CloseBriefcase() override {m_briefcase = nullptr;}

    StatusInt AcquireLocks(LockRequest&, DgnDbR) override;

    static bool SleepBeforeRetry();

    static NativeLogging::ILogger& GetLogger() { return *NativeLogging::LoggingManager::GetLogger("iModelBridge"); }

    WebServices::IConnectTokenProviderPtr GetTokenProvider();
};

// ========================================================================================================
// Exposes the ability to cleanly shutdown an iModel server
// ========================================================================================================
struct IMODEL_BRIDGE_FWK_EXPORT IShutdownIModelServer
    {
    virtual BentleyStatus Shutdown() = 0;
    };

// ========================================================================================================
// Provides access to an iModelBank
// ========================================================================================================
struct IMODEL_BRIDGE_FWK_EXPORT IModelBankClient : IModelClientBase, IShutdownIModelServer
    {
    Utf8String m_iModelId;

    IModelBankClient(iModelBridgeFwk::IModelBankArgs const&, WebServices::ClientInfoPtr ci);

    iModel::Hub::iModelInfoPtr GetIModelInfo() override;
    void SetUrlAndAccessToken(iModelBridgeFwk::IModelBankArgs const&);

    BentleyStatus Shutdown() override;
    };

// ========================================================================================================
// Provides access to an iModelHub
// ========================================================================================================
struct IMODEL_BRIDGE_FWK_EXPORT IModelHubClient : IModelClientBase, IModelHubClientForBridges
    {
	Utf8String m_projectId;
    iModelBridgeFwk::IModelHubArgs m_args;
    Utf8String m_iModelId;

    IModelHubClient(iModelBridgeFwk::IModelHubArgs const&, WebServices::ClientInfoPtr info);
    ~IModelHubClient() {}

    iModel::Hub::iModelInfoPtr GetIModelInfo() override;
    
    /*
    StatusInt AcquireBriefcase(BeFileNameCR bcFileName, Utf8CP repoId) override {return m_impl.AcquireBriefcase(bcFileName, repoId);}
    StatusInt OpenBriefcase(Dgn::DgnDbR db) override {return m_impl.OpenBriefcase(db);}
    StatusInt PullMergeAndPush(Utf8CP a) override {return m_impl.PullMergeAndPush(a);}
    StatusInt PullAndMerge() override {return m_impl.PullAndMerge();}
    StatusInt PullAndMergeSchemaRevisions(Dgn::DgnDbPtr& db) override {return m_impl.PullAndMergeSchemaRevisions(db);}
    iModel::Hub::Error const& GetLastError() const override {return m_impl.GetLastError();}
    IRepositoryManagerP GetRepositoryManager(DgnDbR db) override {return m_impl.GetRepositoryManager(db);}
    void CloseBriefcase() override {m_impl.CloseBriefcase();}
    */
    StatusInt CreateRepository(Utf8CP repoName, BeFileNameCR localDgnDb) override;
    BentleyStatus DeleteRepository() override;
    };

END_BENTLEY_DGN_NAMESPACE

#pragma warning(pop)
