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
m_thread(WorkerThread::Create("ServerInfoProvider")),
m_serverInfo(Http::Response()),
m_serverInfoUpdated(0),
m_enableWsgServerHeader(false)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ServerInfoProvider::~ServerInfoProvider()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Adam.Eichelkraut       01/2017
* @remarks Note: Temporary until WSG defect 651740 is fixed for BIMReviewSharing
+---------------+---------------+---------------+---------------+---------------+------*/
void ServerInfoProvider::EnableWsgServerHeader(bool enable)
    {
    m_enableWsgServerHeader = enable;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ServerInfoProvider::RegisterServerInfoListener(std::weak_ptr<IWSClient::IServerInfoListener> listener)
    {
    m_thread->ExecuteAsync([=]
        {
        m_listeners.push_back(listener);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ServerInfoProvider::UnregisterServerInfoListener(std::weak_ptr<IWSClient::IServerInfoListener> listener)
    {
    m_thread->ExecuteAsync([=]
        {
        auto listenerPtr = listener.lock();
        m_listeners.erase(std::remove_if(m_listeners.begin(), m_listeners.end(),
            [=] (std::weak_ptr<IWSClient::IServerInfoListener> candidateWeakPtr)
            {
            auto candidatePtr = candidateWeakPtr.lock();
            return nullptr == candidatePtr || candidatePtr == listenerPtr;
            }), m_listeners.end());
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ServerInfoProvider::NotifyServerInfoUpdated(WSInfoCR info) const
    {
    ASSERT_CURRENT_THREAD(m_thread);
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
void ServerInfoProvider::UpdateInfo(WSInfoCR info) const
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
        if (result.GetError().GetConnectionStatus() != Http::ConnectionStatus::OK)
            {
            finalResult->SetError(result.GetError());
            return;
            }
        if (result.GetError().GetHttpStatus() == HttpStatus::Unauthorized)
            {
            finalResult->SetError(result.GetError());
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
    Http::Request request = m_configuration->GetHttpClient().CreateGetRequest(m_configuration->GetServerUrl() + page);
    request.SetCancellationToken(ct);

    return request.PerformAsync()->Then<WSInfoHttpResult>([=] (Http::Response& response)
        {
        WSInfo info(response, m_enableWsgServerHeader);
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
AsyncTaskPtr<WSInfoResult> ServerInfoProvider::GetServerInfo
(
bool forceQuery,
ICancellationTokenPtr ct
) const
    {
    return m_thread->ExecuteAsync <WSInfoResult>([=]
        {
        if (!forceQuery && CanUseCachedInfo())
            {
            return WSInfoResult::Success(m_serverInfo);
            }

        // Block so additional GetServerInfo tasks would queue to m_thread
        WSInfoResult result = GetInfo(ct)->GetResult();

        if (!result.IsSuccess())
            {
            return WSInfoResult::Error(result.GetError());
            }

        UpdateInfo(result.GetValue());
        NotifyServerInfoUpdated(result.GetValue());

        return WSInfoResult::Success(m_serverInfo);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> ServerInfoProvider::InvalidateInfo() const
    {
    return m_thread->ExecuteAsync([this]
        {
        m_serverInfoUpdated = 0;
        });
    }
