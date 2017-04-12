/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/Events/EventManager.cpp $
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
    const Utf8String methodName = "EventManagerThread";
    try {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Starting event manager thread.");
        DgnDbServerEventManagerContextPtr* managerContextPtr = (DgnDbServerEventManagerContextPtr*)arg;
        if (nullptr == managerContextPtr)
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid argument.");
            return 0;
            }
        DgnDbServerEventManagerContextPtr managerContext = *managerContextPtr;
        if (managerContext.IsNull())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid context.");
            return 0;
            }
        managerContext->GetConditionVariable().notify_one();

        DgnDbServerLogHelper::Log(SEVERITY::LOG_TRACE, methodName, "Getting context members.");
        DgnDbRepositoryConnectionP repositoryConnectionP = managerContext->GetRepositoryConnectionP();
        DgnDbServerEventManagerP eventManager = managerContext->GetEventManagerP();
        ICancellationTokenPtr cancellationTokenPtr = managerContext->GetCancellationTokenPtr();

        if (nullptr == repositoryConnectionP || nullptr == eventManager)
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Invalid context members.");
            return 0;
            }

        DgnDbServerLogHelper::Log(SEVERITY::LOG_TRACE, methodName, "Starting thread loop.");
        while (true)
            {
            if (managerContext.IsNull())
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Invalid context.");
                return 0;
                }

            if (cancellationTokenPtr->IsCanceled())
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Thread cancelled.");
                return 0;
                }

            DgnDbServerLogHelper::Log(SEVERITY::LOG_TRACE, methodName, "Getting event.");
            auto eventResult = repositoryConnectionP->GetEvent(true, cancellationTokenPtr)->GetResult();
            if (!cancellationTokenPtr->IsCanceled() && eventResult.IsSuccess())
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_TRACE, methodName, "Event received.");
                auto eventType = eventResult.GetValue()->GetEventType();

                DgnDbServerLogHelper::Log(SEVERITY::LOG_TRACE, methodName, "Starting event callbacks.");
                for (auto callback : eventManager->GetCallbacks())
                    {
                    DgnDbServerLogHelper::Log(SEVERITY::LOG_TRACE, methodName, "Calling event callback.");
                    if (callback.second.empty() || EventListContainsEvent(callback.second, eventType))
                        {
                        auto callbackForEvent = *callback.first;
                        callbackForEvent(eventResult.GetValue());
                        }
                    DgnDbServerLogHelper::Log(SEVERITY::LOG_TRACE, methodName, "Event callback called.");
                    }

                DgnDbServerLogHelper::Log(SEVERITY::LOG_TRACE, methodName, "Finished event callbacks.");
                }
            }
        
        return 0;
        }
    catch (std::exception const& e)
        {
        DgnDbServerLogHelper::Log(NativeLogging::SEVERITY::LOG_ERROR, "EventManagerThread", e.what());
        }
    catch (Utf8StringCR message)
        {
        DgnDbServerLogHelper::Log(NativeLogging::SEVERITY::LOG_ERROR, "EventManagerThread", message.c_str());
        }
    catch (...)
        {
        DgnDbServerLogHelper::Log(NativeLogging::SEVERITY::LOG_ERROR, "EventManagerThread", "Unknown exception");
        }
    
    return 0;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
bool DgnDbServerEventManager::Start()
    {
    if (m_eventManagerContext.IsValid())
        return true;

    DgnDbServerLogHelper::Log(NativeLogging::SEVERITY::LOG_INFO, "DgnDbServerEventManager::Start", "Start");
    m_eventManagerContext = new DgnDbServerEventManagerContext(m_repositoryConnectionP, this, SimpleCancellationToken::Create());
    BentleyStatus status = BeThreadUtilities::StartNewThread(1024 * 1024, EventManagerThread, &m_eventManagerContext);
    BeConditionVariable& cv = m_eventManagerContext->GetConditionVariable();
    BeMutexHolder holder(cv.GetMutex());
    cv.RelativeWait(holder, 200);
    return BSISUCCESS == status;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbServerEventManager::Stop()
    {
    if (m_eventManagerContext.IsValid())
        m_eventManagerContext->StopManager();

    m_eventCallbacks.clear();
    if (m_repositoryConnectionP)
        m_repositoryConnectionP->UnsubscribeToEvents();

    // Wait for events thread to finish
    int maxIterations = 100;
    while (m_eventManagerContext.IsValid() && maxIterations > 0)
        {
        int useCount = m_eventManagerContext->GetRefCount();
        if (useCount <= 1)
            break;

        BeThreadUtilities::BeSleep(50);
        maxIterations--;
        }
    BeAssert(m_eventManagerContext.IsNull() || 1 >= m_eventManagerContext->GetRefCount() && "Events thread not finished!");

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
