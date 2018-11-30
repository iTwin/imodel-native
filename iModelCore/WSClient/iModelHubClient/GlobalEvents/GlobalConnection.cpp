/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/GlobalEvents/GlobalConnection.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/GlobalEvents/GlobalConnection.h>
#include "../Utils.h"
#include "../Logging.h"
#include <WebServices/iModelHub/Client/UserInfoManager.h>
#include <WebServices/iModelHub/GlobalEvents/GlobalEventManager.h>

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGN

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Uzkuraitis             04/2018
//---------------------------------------------------------------------------------------
GlobalConnection::GlobalConnection(const Utf8String serverUrl, CredentialsCR credentials, const ClientInfoPtr clientInfo, const IHttpHandlerPtr customHandler)
    {
    auto wsRepositoryClient = WSRepositoryClient::Create(serverUrl, ServerSchema::Plugin::Global + Utf8String("--") + ServerSchema::Plugin::Global, clientInfo, nullptr, customHandler);
    CompressionOptions options;
    options.EnableRequestCompression(true, 1024);
    wsRepositoryClient->Config().SetCompressionOptions(options);
    wsRepositoryClient->SetCredentials(credentials);

    m_wsRepositoryClient = wsRepositoryClient;
    m_eventManagerPtr = new GlobalEventManager(std::static_pointer_cast<IWSRepositoryClient>(wsRepositoryClient));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            05/2018
//---------------------------------------------------------------------------------------
GlobalConnectionResult GlobalConnection::Create(Utf8String serverUrl, CredentialsCR credentials, const ClientInfoPtr clientInfo, const IHttpHandlerPtr customHandler)
    {
    const Utf8String methodName = "GlobalConnection::Create";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (serverUrl.empty())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid server URL.");
        return GlobalConnectionResult::Error(Error::Id::InvalidServerURL);
        }
    if (!credentials.IsValid() && !customHandler)
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Credentials are not set.");
        return GlobalConnectionResult::Error(Error::Id::CredentialsNotSet);
        }

    const double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    const GlobalConnectionPtr globalConnection(new GlobalConnection(serverUrl, credentials, clientInfo, customHandler));

    const double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, static_cast<float>(end - start), "");
    return GlobalConnectionResult::Success(globalConnection);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            05/2018
//---------------------------------------------------------------------------------------
GlobalConnection::~GlobalConnection()
    {
    }
