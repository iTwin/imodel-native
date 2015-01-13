/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/WSClient.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <MobileDgn/Utils/Threading/AsyncResult.h>
#include <MobileDgn/Utils/Http/HttpClient.h>

#include <WebServices/Common.h>
#include <WebServices/WSRepository.h>
#include <WebServices/WSError.h>
#include <WebServices/WSInfo.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

//--------------------------------------------------------------------------------------+
// WebServices Client API for connecting to BWSG server. 
//--------------------------------------------------------------------------------------+

typedef std::shared_ptr<struct IWSClient>           IWSClientPtr;

typedef MobileDgn::Utils::AsyncResult<struct WSInfo, WSError>         WSInfoResult;
typedef MobileDgn::Utils::AsyncResult<bvector<WSRepository>, WSError> WSRepositoriesResult;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct IWSClient
    {
    public:
        struct IServerInfoListener
            {
            virtual ~IServerInfoListener ()
                {
                };
            virtual void OnServerInfoReceived (WSInfoCR info) = 0;
            };

    public:
        WS_EXPORT virtual ~IWSClient ();

        virtual Utf8String GetServerUrl () const = 0;

        //! Register for ServerInfo received events
        virtual void RegisterServerInfoListener (std::weak_ptr<IServerInfoListener> listener) = 0;

        //! Unregister from ServerInfo received events
        virtual void UnregisterServerInfoListener (std::weak_ptr<IServerInfoListener> listener) = 0;

        //! Returns server information or queries server if needs updating
        virtual MobileDgn::Utils::AsyncTaskPtr<WSInfoResult> GetServerInfo
            (
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        //! Queries server information
        virtual MobileDgn::Utils::AsyncTaskPtr<WSInfoResult> SendGetInfoRequest
            (
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSRepositoriesResult> SendGetRepositoriesRequest
            (
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const = 0;

        virtual MobileDgn::Utils::AsyncTaskPtr<WSRepositoriesResult> SendGetRepositoriesRequest
            (
            const bvector<Utf8String>& types,
            const bvector<Utf8String>& providerIds,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
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
        WSClient (std::shared_ptr<struct ClientConnection> connection);

    public:
        static IWSClientPtr Create (std::shared_ptr<struct ClientConnection> connection);

        //! @param[in] serverUrl - address to supported server/site
        //! @param[in] defaultHeaders - headers used for each request. User-Agent is recomended for being able to identify client in server.
        //!                             Mas-Uuid and Mas-App-Guid are posible for licensing purposes.
        //! @param[in] customHandler - custom http handler for testing purposes
        WS_EXPORT static IWSClientPtr Create
            (
            Utf8StringCR serverUrl,
            MobileDgn::Utils::HttpRequestHeadersCR defaultHeaders,
            MobileDgn::Utils::IHttpHandlerPtr customHandler = nullptr
            );

        WS_EXPORT Utf8String GetServerUrl () const override;

        WS_EXPORT void RegisterServerInfoListener (std::weak_ptr<IServerInfoListener> listener) override;
        WS_EXPORT void UnregisterServerInfoListener (std::weak_ptr<IServerInfoListener> listener) override;

        WS_EXPORT MobileDgn::Utils::AsyncTaskPtr<WSInfoResult> GetServerInfo
            (
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        WS_EXPORT MobileDgn::Utils::AsyncTaskPtr<WSInfoResult> SendGetInfoRequest
            (
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        WS_EXPORT MobileDgn::Utils::AsyncTaskPtr<WSRepositoriesResult> SendGetRepositoriesRequest
            (
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;

        WS_EXPORT MobileDgn::Utils::AsyncTaskPtr<WSRepositoriesResult> SendGetRepositoriesRequest
            (
            const bvector<Utf8String>& types,
            const bvector<Utf8String>& providerIds,
            MobileDgn::Utils::ICancellationTokenPtr cancellationToken = nullptr
            ) const override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
