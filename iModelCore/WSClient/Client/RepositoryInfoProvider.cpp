/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/RepositoryInfoProvider.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <Bentley/BeTimeUtilities.h>
#include "WebApi/WebApiV2.h"

#define SERVER_INFO_REFRESH_MS (30*60*1000)

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               julius.cepukenas    05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryInfoProvider::RepositoryInfoProvider(std::shared_ptr<const ClientConnection> connection) :
    m_connection(connection),
    m_info(WSRepository()),
    m_infoUpdated(0)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               julius.cepukenas    05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryInfoProvider::~RepositoryInfoProvider()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               julius.cepukenas    05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RepositoryInfoProvider::RegisterInfoListener(std::weak_ptr<IWSRepositoryClient::IRepositoryInfoListener> listener)
    {
    BeMutexHolder lock(m_mutex);
    m_listeners.push_back(listener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               julius.cepukenas    05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RepositoryInfoProvider::UnregisterInfoListener(std::weak_ptr<IWSRepositoryClient::IRepositoryInfoListener> listener)
    {
    BeMutexHolder lock(m_mutex);
    auto listenerPtr = listener.lock();
    m_listeners.erase(std::remove_if(m_listeners.begin(), m_listeners.end(),
        [&] (std::weak_ptr<IWSRepositoryClient::IRepositoryInfoListener> candidateWeakPtr)
        {
        auto candidatePtr = candidateWeakPtr.lock();
        return nullptr == candidatePtr || candidatePtr == listenerPtr;
        }), m_listeners.end());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               julius.cepukenas    05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RepositoryInfoProvider::NotifyInfoUpdated(WSRepositoryCR info) const
    {
    for (auto& listenerWeakPtr : m_listeners)
        {
        auto listenerPtr = listenerWeakPtr.lock();
        if (listenerPtr)
            listenerPtr->OnInfoReceived(info);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               julius.cepukenas    05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool RepositoryInfoProvider::CanUseCachedInfo() const
    {
    return BeTimeUtilities::GetCurrentTimeAsUnixMillis() - m_infoUpdated < SERVER_INFO_REFRESH_MS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               julius.cepukenas    05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RepositoryInfoProvider::UpdateInfo(WSRepositoryCR info)
    {
    m_info = info;
    m_infoUpdated = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               julius.cepukenas    05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSRepositoryResult> RepositoryInfoProvider::GetRepositoryInfo(IWSRepositoryClient::RequestOptionsPtr options, ICancellationTokenPtr ct) const
    {
    return m_connection->GetWebApiAndReturnResponse<WSRepositoryResult>([=] (WebApiPtr webApi)
        {
        return webApi->SendGetRepositoryInfoRequest(options, ct);
        }, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               julius.cepukenas    05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSRepositoryResult> RepositoryInfoProvider::GetInfo(IWSRepositoryClient::RequestOptionsPtr options, ICancellationTokenPtr ct)
    {
    BeMutexHolder lock(m_mutex);
    if (CanUseCachedInfo())
        return CreateCompletedAsyncTask(WSRepositoryResult::Success(m_info));

    return m_getInfoExecutor.GetTask([=]
        {
        return GetRepositoryInfo(options, ct)->Then<WSRepositoryResult>([=] (WSRepositoryResult result)
            {
            BeMutexHolder lock(m_mutex);

            if (!result.IsSuccess())
                return WSRepositoryResult::Error(result.GetError());

            UpdateInfo(result.GetValue());
            NotifyInfoUpdated(result.GetValue());

            return WSRepositoryResult::Success(m_info);
            });
        });
    }
