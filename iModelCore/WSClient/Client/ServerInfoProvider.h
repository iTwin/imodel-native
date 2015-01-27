/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/ServerInfoProvider.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Client/WSClient.h>
#include <MobileDgn/Utils/Threading/WorkerThread.h>
#include <MobileDgn/Utils/Threading/AsyncTask.h>
#include <set>

#include "ClientConnection.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ServerInfoProvider
    {
    private:
        std::shared_ptr<const ClientConfiguration> m_configuration;
        std::shared_ptr<MobileDgn::Utils::WorkerThread> m_thread;
        std::vector<std::weak_ptr<IWSClient::IServerInfoListener>> m_listeners;

        mutable WSInfo m_serverInfo;
        mutable uint64_t m_serverInfoUpdated;

    private:
        bool CanUseCachedInfo () const;
        void UpdateInfo (WSInfoCR info) const;

        MobileDgn::Utils::HttpRequest CreateGetInfoRequest () const;
        MobileDgn::Utils::HttpRequest CreateGetAboutPageRequest () const;
        MobileDgn::Utils::HttpRequest CreateGetPluginsRequest () const;

        MobileDgn::Utils::AsyncTaskPtr<WSInfoResult> SendGetInfoRequest (MobileDgn::Utils::ICancellationTokenPtr cancellationToken) const;
        MobileDgn::Utils::AsyncTaskPtr<WSInfoResult> SendGetInfoRequestFromAboutPage (MobileDgn::Utils::ICancellationTokenPtr cancellationToken) const;
        MobileDgn::Utils::AsyncTaskPtr<WSInfoResult> SendGetInfoRequestFromPlugins (MobileDgn::Utils::ICancellationTokenPtr cancellationToken) const;

        void NotifyServerInfoUpdated (WSInfoCR info) const;

    public:
        ServerInfoProvider (std::shared_ptr<const ClientConfiguration> configuration);
        ~ServerInfoProvider ();

        void RegisterServerInfoListener (std::weak_ptr<IWSClient::IServerInfoListener> listener);
        void UnregisterServerInfoListener (std::weak_ptr<IWSClient::IServerInfoListener> listener);

        MobileDgn::Utils::AsyncTaskPtr<WSInfoResult> GetServerInfo (bool forceQuery, MobileDgn::Utils::ICancellationTokenPtr cancellationToken) const;
        MobileDgn::Utils::AsyncTaskPtr<void> InvalidateInfo () const;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
