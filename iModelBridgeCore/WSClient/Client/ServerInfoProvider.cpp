/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <Bentley/BeTimeUtilities.h>

#define SERVER_INFO_REFRESH_MS (30*60*1000)

std::map<Utf8String, std::pair<WSInfo, uint64_t>> ServerInfoProvider::s_serverInfo{};
BeMutex ServerInfoProvider::s_mutex;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ServerInfoProvider::ServerInfoProvider(std::shared_ptr<const ClientConfiguration> configuration) :
m_configuration(configuration)
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
    BeMutexHolder lock(m_mutex);
    m_listeners.push_back(listener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ServerInfoProvider::UnregisterServerInfoListener(std::weak_ptr<IWSClient::IServerInfoListener> listener)
    {
    BeMutexHolder lock(m_mutex);
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
    Utf8String url = m_configuration->GetServerUrl();
    if (s_serverInfo.find(url) == s_serverInfo.end())
        return false;
    return BeTimeUtilities::GetCurrentTimeAsUnixMillis() - s_serverInfo[url].second < SERVER_INFO_REFRESH_MS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ServerInfoProvider::UpdateInfo(WSInfoCR info) const
    {
    Utf8String url = m_configuration->GetServerUrl();
    s_serverInfo[url].first = info;
    s_serverInfo[url].second = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSInfoResult> ServerInfoProvider::GetInfo(ICancellationTokenPtr ct) const
    {
    if (!m_configuration->GetServiceVersion().IsEmpty())
        return GetInfo(m_configuration->GetServiceVersion(), ct);

    // WSG 2.0 +
    return GetInfoFromPage("/v2.0/Plugins", {}, ct)->Then<WSInfoResult>([=] (WSInfoHttpResult& result)
        {
        if (!result.IsSuccess())
            return WSInfoResult::Error(result.GetError());

        return WSInfoResult::Success(result.GetValue());
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSInfoResult> ServerInfoProvider::GetInfo(BeVersionCR serviceVersion, ICancellationTokenPtr ct) const
    {
    Utf8String url = "/sv" + serviceVersion.ToMajorMinorString() + "/Plugins";
    return GetInfoFromPage(url, serviceVersion, ct)->Then<WSInfoResult>([=] (WSInfoHttpResult& result)
        {
        if (!result.IsSuccess())
            return WSInfoResult::Error(result.GetError());

        return WSInfoResult::Success(result.GetValue());
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSInfoHttpResult> ServerInfoProvider::GetInfoFromPage(Utf8StringCR page, BeVersionCR serviceVersion, ICancellationTokenPtr ct) const
    {
    Http::Request request = m_configuration->GetHttpClient().CreateGetRequest(m_configuration->GetServerUrl() + page);
    request.SetCancellationToken(ct);

    return request.PerformAsync()->Then<WSInfoHttpResult>([=] (Http::Response& response)
        {
        WSInfo info(response, serviceVersion);
        if (!info.IsValid())
            return WSInfoHttpResult::Error(response);

        return WSInfoHttpResult::Success(info);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSInfoResult> ServerInfoProvider::GetServerInfo(bool forceQuery, ICancellationTokenPtr ct)
    {
    BeMutexHolder lock(s_mutex);
    Utf8String url = m_configuration->GetServerUrl();
    if (!forceQuery && CanUseCachedInfo())
        return CreateCompletedAsyncTask(WSInfoResult::Success(s_serverInfo[url].first));
        

    return m_getInfoExecutor.GetTask([=]
        {
        return GetInfo(ct)->Then<WSInfoResult>([=] (WSInfoResult result)
            {
            BeMutexHolder lock(s_mutex);

            if (!result.IsSuccess())
                return WSInfoResult::Error(result.GetError());

            UpdateInfo(result.GetValue());
            NotifyServerInfoUpdated(result.GetValue());
            return WSInfoResult::Success(s_serverInfo[url].first);
            });
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ServerInfoProvider::InvalidateInfo() const
{
    BeMutexHolder lock(s_mutex);
    Utf8String url = m_configuration->GetServerUrl();
    s_serverInfo[url].second = 0;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               Vismantas.Stonkus    08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ServerInfoProvider::InvalidateAllInfo()
    {
    BeMutexHolder lock(s_mutex);
    s_serverInfo.clear();
    }
