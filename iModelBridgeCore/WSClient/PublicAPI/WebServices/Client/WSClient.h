/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Client/WSClient.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnClientFx/Utils/Threading/AsyncResult.h>
#include <DgnClientFx/Utils/Http/HttpClient.h>

#include <WebServices/Client/ClientInfo.h>
#include <WebServices/Client/WebServicesClient.h>
#include <WebServices/Client/WSRepository.h>
#include <WebServices/Client/WSError.h>
#include <WebServices/Client/WSInfo.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS

//--------------------------------------------------------------------------------------+
// WebServices Client API for connecting to BWSG server.
//--------------------------------------------------------------------------------------+

typedef std::shared_ptr<struct IWSClient>           IWSClientPtr;

typedef AsyncResult<struct WSInfo, WSError>         WSInfoResult;
typedef AsyncResult<bvector<WSRepository>, WSError> WSRepositoriesResult;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct IWSClient
    {
    public:
        struct IServerInfoListener
            {
            virtual ~IServerInfoListener()
                {};
            virtual void OnServerInfoReceived(WSInfoCR info) = 0;
            };

    public:
        WSCLIENT_EXPORT virtual ~IWSClient();

        virtual Utf8String GetServerUrl() const = 0;

        //! Register for ServerInfo received events
        virtual void RegisterServerInfoListener(std::weak_ptr<IServerInfoListener> listener) = 0;

        //! Unregister from ServerInfo received events
        virtual void UnregisterServerInfoListener(std::weak_ptr<IServerInfoListener> listener) = 0;

        //! Returns server information or queries server if needs updating
        virtual AsyncTaskPtr<WSInfoResult> GetServerInfo
            (
            ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        //! Queries server information
        virtual AsyncTaskPtr<WSInfoResult> SendGetInfoRequest
            (
            ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual AsyncTaskPtr<WSRepositoriesResult> SendGetRepositoriesRequest
            (
            ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual AsyncTaskPtr<WSRepositoriesResult> SendGetRepositoriesRequest
            (
            const bvector<Utf8String>& types,
            const bvector<Utf8String>& providerIds,
            ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                               Beneditas.Lipnickas   06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSClient : public IWSClient, public std::enable_shared_from_this<WSClient>
    {
    private:
        std::shared_ptr<struct ClientConnection> m_connection;

    private:
        WSClient(std::shared_ptr<struct ClientConnection> connection);

    public:
        static IWSClientPtr Create(std::shared_ptr<struct ClientConnection> connection);

        //! @param[in] serverUrl - address to supported server/site
        //! @param[in] clientInfo - client infomation for licensing and other information
        //! @param[in] customHandler - custom http handler for testing purposes
        WSCLIENT_EXPORT static IWSClientPtr Create
            (
            Utf8StringCR serverUrl,
            ClientInfoPtr clientInfo,
            IHttpHandlerPtr customHandler = nullptr
            );

        WSCLIENT_EXPORT Utf8String GetServerUrl() const override;

        WSCLIENT_EXPORT void RegisterServerInfoListener(std::weak_ptr<IServerInfoListener> listener) override;
        WSCLIENT_EXPORT void UnregisterServerInfoListener(std::weak_ptr<IServerInfoListener> listener) override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSInfoResult> GetServerInfo
            (
            ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSInfoResult> SendGetInfoRequest
            (
            ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSRepositoriesResult> SendGetRepositoriesRequest
            (
            ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        WSCLIENT_EXPORT AsyncTaskPtr<WSRepositoriesResult> SendGetRepositoriesRequest
            (
            const bvector<Utf8String>& types,
            const bvector<Utf8String>& providerIds,
            ICancellationTokenPtr cancellationToken = nullptr
            ) const override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
