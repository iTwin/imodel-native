/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/Client/WSRepositoryClient.h>
#include <WebServices/iModelHub/Client/Error.h>
#include <WebServices/iModelHub/Common.h>
#include <WebServices/iModelHub/Client/iModelInfo.h>
#include <WebServices/Azure/AzureBlobStorageClient.h>
#include <WebServices/Azure/AzureServiceBusSASDTO.h>
#include <BeHttp/AuthenticationHandler.h>
#include <WebServices/iModelHub/Client/UserInfoManager.h>
#include <WebServices/iModelHub/GlobalEvents/GlobalEventManager.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(GlobalConnection);
typedef RefCountedPtr<struct GlobalConnection> GlobalConnectionPtr;

DEFINE_TASK_TYPEDEFS(GlobalConnectionPtr, GlobalConnection);
DEFINE_TASK_TYPEDEFS(AzureServiceBusSASDTOPtr, AzureServiceBusSASDTO);

//=======================================================================================
//! Connection to Global events
//! This class performs all of the operations related to a Global events on the server.
//@bsiclass                                      Karolis.Uzkuraitis             04/2018
//=======================================================================================
struct GlobalConnection : RefCountedBase
    {
private:
    friend struct Client;
    friend struct GlobalEventManager;

    IWSRepositoryClientPtr      m_wsRepositoryClient;
    IAzureBlobStorageClientPtr  m_azureClient;
    IHttpHandlerPtr             m_customHandler;

    GlobalEventManagerPtr       m_eventManagerPtr;

    GlobalConnection(Utf8String serverUrl, CredentialsCR credentials, ClientInfoPtr clientInfo, IHttpHandlerPtr customHandler);

    //! Create an instance of the connection to a iModel on the server.
    //! @param[in] serverUrl Base url of iModelHubServices Server.
    //! @param[in] credentials Credentials used to authenticate on the iModel.
    //! @param[in] clientInfo Application information sent to server.
    //! @param[in] customHandler Http handler for connect authentication.
    //! @return Asynchronous task that has the created connection instance as the result.
    //! @note Client is the class that creates this connection.
    static GlobalConnectionResult Create(Utf8String serverUrl, CredentialsCR credentials, ClientInfoPtr clientInfo, IHttpHandlerPtr customHandler = nullptr);

public:
    virtual ~GlobalConnection();

    //! Gets GlobalEventManager for global event subscriptions and event gathering
    //! @return GlobalEventManager
    IMODELHUBCLIENT_EXPORT GlobalEventManagerPtr GetGlobalEventManager() const { return m_eventManagerPtr; }
    };
END_BENTLEY_IMODELHUB_NAMESPACE
