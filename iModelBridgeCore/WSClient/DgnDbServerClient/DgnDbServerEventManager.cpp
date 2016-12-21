/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerEventManager.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/TxnManager.h>
#include <DgnDbServer/Client/DgnDbBriefcase.h>
#include <thread>
#include "DgnDbServerEventManager.h"
#include <DgnDbServer/Client/Logging.h>

USING_NAMESPACE_BENTLEY_DGNDBSERVER
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventManagerContext::DgnDbServerEventManagerContext(DgnDbRepositoryConnectionPtr repositoryConnectionPtr, DgnDbServerEventManagerPtr manager, SimpleCancellationTokenPtr cancellationToken)
    {
    m_repositoryConnectionPtr = repositoryConnectionPtr;
    m_managerPtr = manager;
    m_cancellationTokenPtr = cancellationToken;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
DgnDbRepositoryConnectionPtr DgnDbServerEventManagerContext::GetRepositoryConnectionPtr() const
    {
    return m_repositoryConnectionPtr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventManagerPtr DgnDbServerEventManagerContext::GetEventManagerPtr() const
    {
    return m_managerPtr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
SimpleCancellationTokenPtr DgnDbServerEventManagerContext::GetCancellationTokenPtr() const
    {
    return m_cancellationTokenPtr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
void DgnDbServerEventManagerContext::StopManager()
    {
    m_cancellationTokenPtr->SetCanceled();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
bool EventListContainsEvent(DgnDbServerEventTypeSet eventList, DgnDbServerEvent::DgnDbServerEventType eventType)
    {
    return eventList.find(eventType) != eventList.end();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
#if defined (__APPLE__) || defined (ANDROID) || defined (__linux) || defined (__EMSCRIPTEN__)
void* EventManagerThread(void* arg)
#elif defined (_WIN32) // Windows && WinRT
unsigned __stdcall EventManagerThread(void* arg)
#endif
    {
    DgnDbServerEventManagerContextPtr* managerContextPtr = (DgnDbServerEventManagerContextPtr*)arg;
    DgnDbServerEventManagerContextPtr managerContext = *managerContextPtr;
    DgnDbRepositoryConnectionPtr repositoryConnectionPtr = managerContext->GetRepositoryConnectionPtr();
    DgnDbServerEventManagerPtr eventManager = managerContext->GetEventManagerPtr();
    ICancellationTokenPtr cancellationTokenPtr = managerContext->GetCancellationTokenPtr();

    while (true)
        {
        if (!managerContext)
            return 0;

        if (cancellationTokenPtr->IsCanceled()) 
            return 0;

        if (!repositoryConnectionPtr->IsSubscribedToEvents())
            {
            BeThreadUtilities::BeSleep(50);
            continue;
            }

        auto eventResult = repositoryConnectionPtr->GetEvent(true, cancellationTokenPtr)->GetResult();
        if (eventResult.IsSuccess())
            {
            auto eventType = eventResult.GetValue()->GetEventType();
            for (auto callback : eventManager->GetCallbacks())
                {
                if (callback.second.empty() || EventListContainsEvent(callback.second, eventType))
                    {
                    auto callbackForEvent = *callback.first;
                    callbackForEvent(eventResult.GetValue());
                    }
                }
            }
        }
        
    return 0;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventManager::DgnDbServerEventManager(DgnDbRepositoryConnectionPtr repositoryConnectionPtr)
    {
    m_repositoryConnectionPtr = repositoryConnectionPtr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
bool DgnDbServerEventManager::Start()
    {
    if (m_eventManagerContext)
        return true;

    DgnDbServerLogHelper::Log(NativeLogging::SEVERITY::LOG_INFO, "DgnDbServerEventManager::Start", "Start");
    m_eventManagerContext = std::make_shared<DgnDbServerEventManagerContext>(m_repositoryConnectionPtr, shared_from_this(), SimpleCancellationToken::Create());
    return BSISUCCESS == BeThreadUtilities::StartNewThread(1024 * 1024, EventManagerThread, &m_eventManagerContext);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbServerEventManager::Stop()
    {
    if (m_eventManagerContext)
        m_eventManagerContext->StopManager();

    m_eventCallbacks.clear();
    if (m_repositoryConnectionPtr)
        m_repositoryConnectionPtr->UnsubscribeToEvents();

    m_eventManagerContext = nullptr;
    DgnDbServerLogHelper::Log(NativeLogging::SEVERITY::LOG_INFO, "DgnDbServerEventManager::Stop", "Stop");
    return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Success());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventTypeSet* DgnDbServerEventManager::GetAllSubscribedEvents()
    {
    DgnDbServerEventTypeSet* allEventTypes = new DgnDbServerEventTypeSet();

    for (auto eventCallback : m_eventCallbacks)
        {
        if (eventCallback.second.empty())
            {
            return nullptr;
            }

        for(auto eventType : eventCallback.second)
            {
            if (!EventListContainsEvent(*allEventTypes, eventType))
                allEventTypes->insert(eventType);
            }
        }

    return allEventTypes;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbServerEventManager::Subscribe(DgnDbServerEventTypeSet* eventTypes, DgnDbServerEventCallbackPtr callback)
    {
    // Check callback is already subscribed
    if (m_eventCallbacks.find(callback) != m_eventCallbacks.end())
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(DgnDbServerError::Id::EventCallbackAlreadySubscribed));

    if (!Start())
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(DgnDbServerError::Id::EventServiceSubscribingError));

    auto types = eventTypes ? *eventTypes : DgnDbServerEventTypeSet();
    m_eventCallbacks.Insert(callback, types);

    // Gather all event types
    DgnDbServerEventTypeSet* allEventTypes = GetAllSubscribedEvents();
    return m_repositoryConnectionPtr->SubscribeToEvents(allEventTypes);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbServerEventManager::Unsubscribe(DgnDbServerEventCallbackPtr callback, bool* dispose)
    {
    *dispose = false;
    if (callback)
        {
        auto it = m_eventCallbacks.find(callback);
        bool callbackFound = it != m_eventCallbacks.end();

        if (callbackFound)
            m_eventCallbacks.erase(it);
        else
            return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(DgnDbServerError::Id::EventCallbackNotFound));
        }

    if (!callback || 0 == m_eventCallbacks.size())
        {
        *dispose = true;
        return Stop();
        }
    
    // Subscribe only to remaining events
    DgnDbServerEventTypeSet* allEventTypes = GetAllSubscribedEvents();
    return m_repositoryConnectionPtr->SubscribeToEvents(allEventTypes);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventMap DgnDbServerEventManager::GetCallbacks() const
    {
    return m_eventCallbacks;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventManager::~DgnDbServerEventManager()
    {
    Stop();
    }
