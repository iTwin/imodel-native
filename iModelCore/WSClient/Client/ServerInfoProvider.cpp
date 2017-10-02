/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/ServerInfoProvider.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <Bentley/BeTimeUtilities.h>

#define SERVER_INFO_REFRESH_MS (30*60*1000)

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ServerInfoProvider::ServerInfoProvider(std::shared_ptr<const ClientConfiguration> configuration) :
m_configuration(configuration),
m_serverInfo(HttpResponse()),
m_serverInfoUpdated(0)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ServerInfoProvider::~ServerInfoProvider()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ServerInfoProvider::RegisterServerInfoListener(std::weak_ptr<IWSClient::IServerInfoListener> listener)
    {
    BeCriticalSectionHolder lock(m_mutex);
    m_listeners.push_back(listener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ServerInfoProvider::UnregisterServerInfoListener(std::weak_ptr<IWSClient::IServerInfoListener> listener)
    {
    BeCriticalSectionHolder lock(m_mutex);
    auto listenerPtr = listener.lock();
    m_listeners.erase(std::remove_if(m_listeners.begin(), m_listeners.end(),
        [=] (std::weak_ptr<IWSClient::IServerInfoListener> candidateWeakPtr)
        {
        auto candidatePtr = candidateWeakPtr.lock();
        return nullptr == candidatePtr || candidatePtr == listenerPtr;
        }), m_listeners.end());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ServerInfoProvider::NotifyServerInfoUpdated(WSInfoCR info) const
    {
    for (auto& listenerWeakPtr : m_listeners)
        {
        auto listenerPtr = listenerWeakPtr.lock();
        if (listenerPtr)
            {
            listenerPtr->OnServerInfoReceived(info);
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ServerInfoProvider::CanUseCachedInfo() const
    {
    return BeTimeUtilities::GetCurrentTimeAsUnixMillis() - m_serverInfoUpdated < SERVER_INFO_REFRESH_MS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ServerInfoProvider::UpdateInfo(WSInfoCR info)
    {
    m_serverInfo = info;
    m_serverInfoUpdated = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSInfoResult> ServerInfoProvider::GetInfo(ICancellationTokenPtr ct) const
    {
    auto finalResult = std::make_shared<WSInfoResult>();

    // WSG 2.0 +
    return GetInfoFromPage("/v2.0/Plugins", ct)->Then([=] (WSInfoHttpResult& result)
        {
        if (result.IsSuccess())
            {
            finalResult->SetSuccess(result.GetValue());
            return;
            }

        WSError error(result.GetError());
        if (error.GetStatus() != WSError::Status::ServerNotSupported)
            {
            finalResult->SetError(error);
            return;
            }

        // WSG R2 - R3.5
        GetInfoFromPage("/v1.2/Info", ct)->Then([=] (WSInfoHttpResult& result)
            {
            if (result.IsSuccess())
                {
                finalResult->SetSuccess(result.GetValue());
                return;
                }
            if (result.GetError().GetHttpStatus() != HttpStatus::NotFound)
                {
                finalResult->SetError(result.GetError());
                return;
                }

            // WSG R1
            GetInfoFromPage("/Pages/About.aspx", ct)->Then([=] (WSInfoHttpResult& result)
                {
                if (result.IsSuccess())
                    {
                    finalResult->SetSuccess(result.GetValue());
                    return;
                    }

                finalResult->SetError(result.GetError());
                });
            });
        })
            ->Then<WSInfoResult>([=]
            {
            return *finalResult;
            });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSInfoHttpResult> ServerInfoProvider::GetInfoFromPage(Utf8StringCR page, ICancellationTokenPtr ct) const
    {
    HttpRequest request = m_configuration->GetHttpClient().CreateGetRequest(m_configuration->GetServerUrl() + page);
    request.SetCancellationToken(ct);

    return request.PerformAsync()->Then<WSInfoHttpResult>([=] (HttpResponse& response)
        {
        WSInfo info(response);
        if (info.IsValid())
            {
            return WSInfoHttpResult::Success(info);
            }
        return WSInfoHttpResult::Error(response);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSInfoResult> ServerInfoProvider::GetServerInfo(bool forceQuery, ICancellationTokenPtr ct)
    {
    BeCriticalSectionHolder lock(m_mutex);
    if (!forceQuery && CanUseCachedInfo())
        return CreateCompletedAsyncTask(WSInfoResult::Success(m_serverInfo));

    return m_getInfoExecutor.GetTask([=]
        {
        return GetInfo(ct)->Then<WSInfoResult>([=] (WSInfoResult result)
            {
            BeCriticalSectionHolder lock(m_mutex);

            if (!result.IsSuccess())
                return WSInfoResult::Error(result.GetError());

            UpdateInfo(result.GetValue());
            NotifyServerInfoUpdated(result.GetValue());

            return WSInfoResult::Success(m_serverInfo);
            });
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ServerInfoProvider::InvalidateInfo()
    {
    BeCriticalSectionHolder lock(m_mutex);
    m_serverInfoUpdated = 0;
    }
