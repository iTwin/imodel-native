/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerEventManager.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/TxnManager.h>
#include <DgnDbServer/Client/DgnDbBriefcase.h>
#include <thread>
#include "DgnDbServerEventManager.h"
#include <DgnDbServer/Client/Logging.h>

USING_NAMESPACE_BENTLEY_DGNDBSERVER
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventManagerContext::DgnDbServerEventManagerContext(DgnDbRepositoryConnectionP repositoryConnectionPtr, DgnDbServerEventManagerP manager, SimpleCancellationTokenPtr cancellationToken)
    {
    m_repositoryConnectionP = repositoryConnectionPtr;
    m_managerP = manager;
    m_cancellationTokenPtr = cancellationToken;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
DgnDbRepositoryConnectionP DgnDbServerEventManagerContext::GetRepositoryConnectionP() const
    {
    return m_repositoryConnectionP;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventManagerP DgnDbServerEventManagerContext::GetEventManagerP() const
    {
    return m_managerP;
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
    try {
        DgnDbServerEventManagerContextPtr* managerContextPtr = (DgnDbServerEventManagerContextPtr*)arg;
        DgnDbServerEventManagerContextPtr managerContext = *managerContextPtr;
        if (nullptr == managerContext)
            return 0;

        DgnDbRepositoryConnectionP repositoryConnectionP = managerContext->GetRepositoryConnectionP();
        DgnDbServerEventManagerP eventManager = managerContext->GetEventManagerP();
        ICancellationTokenPtr cancellationTokenPtr = managerContext->GetCancellationTokenPtr();

        while (true)
            {
            if (!managerContext)
                return 0;

            if (cancellationTokenPtr->IsCanceled()) 
                return 0;

            auto eventResult = repositoryConnectionP->GetEvent(true, cancellationTokenPtr)->GetResult();
            if (!cancellationTokenPtr->IsCanceled() && eventResult.IsSuccess())
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
    catch (const std::exception &e)
        {
        DgnDbServerLogHelper::Log(NativeLogging::SEVERITY::LOG_ERROR, "EventManagerThread", e.what());
        }
    
    return 0;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventManager::DgnDbServerEventManager(DgnDbRepositoryConnectionP repositoryConnectionPtr)
    {
    m_repositoryConnectionP = repositoryConnectionPtr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
bool DgnDbServerEventManager::Start()
    {
    if (m_eventManagerContext)
        return true;

    DgnDbServerLogHelper::Log(NativeLogging::SEVERITY::LOG_INFO, "DgnDbServerEventManager::Start", "Start");
    m_eventManagerContext = std::make_shared<DgnDbServerEventManagerContext>(m_repositoryConnectionP, this, SimpleCancellationToken::Create());
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
    if (m_repositoryConnectionP)
        m_repositoryConnectionP->UnsubscribeToEvents();

    // Wait for events thread to finish
    int maxIterations = 100;
    while (maxIterations > 0)
        {
        int useCount = m_eventManagerContext.use_count();
        if (useCount <= 1)
            break;

        BeThreadUtilities::BeSleep(50);
        maxIterations--;
        }
    BeAssert(1 >= m_eventManagerContext.use_count() && "Events thread not finished!");

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


    auto types = eventTypes ? *eventTypes : DgnDbServerEventTypeSet();
    m_eventCallbacks.Insert(callback, types);

    // Gather all event types
    DgnDbServerEventTypeSet* allEventTypes = GetAllSubscribedEvents();
    auto subscribeResult = m_repositoryConnectionP->SubscribeToEvents(allEventTypes);

    if (!subscribeResult->GetResult().IsSuccess())
        return subscribeResult;

    if (!Start())
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(DgnDbServerError::Id::EventServiceSubscribingError));

    return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Success());
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
    return m_repositoryConnectionP->SubscribeToEvents(allEventTypes);
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
