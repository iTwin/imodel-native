/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/ServerInfoProvider.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <Bentley/BeTimeUtilities.h>

#define SERVER_INFO_REFRESH_MS (30*60*1000)

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ServerInfoProvider::ServerInfoProvider (std::shared_ptr<const ClientConfiguration> configuration) :
m_configuration (configuration),
m_thread (WorkerThread::Create ("ServerInfoProvider")),
m_serverInfo (HttpResponse ()),
m_serverInfoUpdated (0)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ServerInfoProvider::~ServerInfoProvider ()
    {
    m_thread->Stop ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ServerInfoProvider::RegisterServerInfoListener (std::weak_ptr<IWSClient::IServerInfoListener> listener)
    {
    m_thread->ExecuteAsync ([=]
        {
        m_listeners.push_back (listener);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ServerInfoProvider::UnregisterServerInfoListener (std::weak_ptr<IWSClient::IServerInfoListener> listener)
    {
    m_thread->ExecuteAsync ([=]
        {
        auto listenerPtr = listener.lock ();
        m_listeners.erase (std::remove_if (m_listeners.begin (), m_listeners.end (),
            [=] (std::weak_ptr<IWSClient::IServerInfoListener> candidateWeakPtr)
            {
            auto candidatePtr = candidateWeakPtr.lock ();
            return nullptr == candidatePtr || candidatePtr == listenerPtr;
            }), m_listeners.end ());
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ServerInfoProvider::NotifyServerInfoUpdated (WSInfoCR info) const
    {
    ASSERT_CURRENT_THREAD (m_thread);
    for (auto& listenerWeakPtr : m_listeners)
        {
        auto listenerPtr = listenerWeakPtr.lock ();
        if (listenerPtr)
            {
            listenerPtr->OnServerInfoReceived (info);
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ServerInfoProvider::CanUseCachedInfo () const
    {
    return BeTimeUtilities::GetCurrentTimeAsUnixMillis () - m_serverInfoUpdated < SERVER_INFO_REFRESH_MS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ServerInfoProvider::UpdateInfo (WSInfoCR info) const
    {
    m_serverInfo = info;
    m_serverInfoUpdated = BeTimeUtilities::GetCurrentTimeAsUnixMillis ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpRequest ServerInfoProvider::CreateGetInfoRequest () const
    {
    Utf8PrintfString url ("%s/v1.2/Info", m_configuration->GetServerUrl ().c_str ());
    return m_configuration->GetHttpClient ().CreateGetRequest (url);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HttpRequest ServerInfoProvider::CreateGetAboutPageRequest () const
    {
    Utf8PrintfString url ("%s/Pages/About.aspx", m_configuration->GetServerUrl ().c_str ());
    return m_configuration->GetHttpClient ().CreateGetRequest (url);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Dalius.Dobravolskas             09/14
+---------------+---------------+---------------+---------------+---------------+------*/
HttpRequest ServerInfoProvider::CreateGetPluginsRequest () const
    {
    Utf8PrintfString url ("%s/v2.0/Plugins", m_configuration->GetServerUrl ().c_str ());
    return m_configuration->GetHttpClient ().CreateGetRequest (url);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSInfoResult> ServerInfoProvider::SendGetInfoRequest
(
ICancellationTokenPtr cancellationToken
) const
    {
    auto result = std::make_shared<WSInfoResult> ();

    HttpRequest request = CreateGetInfoRequest ();
    request.SetCancellationToken (cancellationToken);

    return request.PerformAsync ()
    ->Then ([=] (HttpResponse& response)
        {
        if (response.GetHttpStatus () == HttpStatus::NotFound)
            {
            // Compatibility to WSG R1
            SendGetInfoRequestFromAboutPage (cancellationToken)
            ->Then ([=] (WSInfoResult& infoResult)
                {
                if (!infoResult.IsSuccess ())
                    {
                    SendGetInfoRequestFromPlugins (cancellationToken)
                    ->Then ([=] (WSInfoResult& infoResult)
                        {
                        *result = infoResult;
                        });
                    }

                *result = infoResult;
                });
            return;
            }

        WSInfo info (response);
        if (info.IsValid ())
            {
            result->SetSuccess (info);
            }
        else
            {
            result->SetError (response);
            }
        })
    ->Then<WSInfoResult> ([=]
        {
        return *result;
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSInfoResult> ServerInfoProvider::SendGetInfoRequestFromAboutPage
(
ICancellationTokenPtr cancellationToken
) const
    {
    HttpRequest request = CreateGetAboutPageRequest ();
    request.SetCancellationToken (cancellationToken);

    return request.PerformAsync ()
    ->Then<WSInfoResult> ([=] (HttpResponse& response)
        {
        WSInfo info (response);
        if (info.IsValid ())
            {
            return WSInfoResult::Success (info);
            }
        return WSInfoResult::Error (response);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Dalius.Dobravolskas             09/14
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSInfoResult> ServerInfoProvider::SendGetInfoRequestFromPlugins
(
ICancellationTokenPtr cancellationToken
) const
    {
    HttpRequest request = CreateGetPluginsRequest ();
    request.SetCancellationToken (cancellationToken);

    return request.PerformAsync ()
    ->Then<WSInfoResult> ([=] (HttpResponse& response)
        {
        WSInfo info (response);
        if (info.IsValid ())
            {
            return WSInfoResult::Success (info);
            }
        return WSInfoResult::Error (response);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSInfoResult> ServerInfoProvider::GetServerInfo
(
bool forceQuery,
ICancellationTokenPtr cancellationToken
) const
    {
    return m_thread->ExecuteAsync <WSInfoResult> ([=]
        {
        if (!forceQuery && CanUseCachedInfo ())
            {
            return WSInfoResult::Success (m_serverInfo);
            }

        // Block so additional GetServerInfo tasks would queue to m_thread
        WSInfoResult infoResult = SendGetInfoRequest (cancellationToken)->GetResult ();

        if (!infoResult.IsSuccess ())
            {
            return WSInfoResult::Error (infoResult.GetError ());
            }

        UpdateInfo (infoResult.GetValue ());
        NotifyServerInfoUpdated (infoResult.GetValue ());

        return WSInfoResult::Success (m_serverInfo);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> ServerInfoProvider::InvalidateInfo () const
    {
    return m_thread->ExecuteAsync ([this]
        {
        m_serverInfoUpdated = 0;
        });
    }
