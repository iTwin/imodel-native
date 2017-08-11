/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/Events/EventManager.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/TxnManager.h>
#include <WebServices/iModelHub/Client/Briefcase.h>
#include <thread>
#include "EventManager.h"
#include "../Logging.h"

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
bool EventListContainsEvent(EventTypeSet eventList, Event::EventType eventType)
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
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Starting event manager thread.");
        EventManagerContextPtr* managerContextPtr = (EventManagerContextPtr*)arg;
        if (nullptr == managerContextPtr)
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid argument.");
            return 0;
            }
        EventManagerContextPtr managerContext = *managerContextPtr;
        if (managerContext.IsNull())
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid context.");
            return 0;
            }
        managerContext->GetConditionVariable().notify_one();

        LogHelper::Log(SEVERITY::LOG_TRACE, methodName, "Getting context members.");
        iModelConnectionP imodelConnectionP = managerContext->GetiModelConnectionP();
        EventManagerP eventManager = managerContext->GetEventManagerP();
        ICancellationTokenPtr cancellationTokenPtr = managerContext->GetCancellationTokenPtr();

        if (nullptr == imodelConnectionP || nullptr == eventManager)
            {
            LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Invalid context members.");
            return 0;
            }

        LogHelper::Log(SEVERITY::LOG_TRACE, methodName, "Starting thread loop.");
        while (true)
            {
            if (managerContext.IsNull())
                {
                LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Invalid context.");
                return 0;
                }

            if (cancellationTokenPtr->IsCanceled())
                {
                LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Thread cancelled.");
                return 0;
                }

            LogHelper::Log(SEVERITY::LOG_TRACE, methodName, "Getting event.");
            auto eventResult = imodelConnectionP->GetEvent(true, cancellationTokenPtr)->GetResult();
            if (!cancellationTokenPtr->IsCanceled() && eventResult.IsSuccess())
                {
                LogHelper::Log(SEVERITY::LOG_TRACE, methodName, "Event received.");
                auto eventType = eventResult.GetValue()->GetEventType();

                LogHelper::Log(SEVERITY::LOG_TRACE, methodName, "Starting event callbacks.");
                for (auto callback : eventManager->GetCallbacks())
                    {
                    LogHelper::Log(SEVERITY::LOG_TRACE, methodName, "Calling event callback.");
                    if (callback.second.empty() || EventListContainsEvent(callback.second, eventType))
                        {
                        auto callbackForEvent = *callback.first;
                        callbackForEvent(eventResult.GetValue());
                        }
                    LogHelper::Log(SEVERITY::LOG_TRACE, methodName, "Event callback called.");
                    }

                LogHelper::Log(SEVERITY::LOG_TRACE, methodName, "Finished event callbacks.");
                }
            }
        
        return 0;
        }
    catch (std::exception const& e)
        {
        LogHelper::Log(NativeLogging::SEVERITY::LOG_ERROR, "EventManagerThread", e.what());
        }
    catch (Utf8StringCR message)
        {
        LogHelper::Log(NativeLogging::SEVERITY::LOG_ERROR, "EventManagerThread", message.c_str());
        }
    catch (...)
        {
        LogHelper::Log(NativeLogging::SEVERITY::LOG_ERROR, "EventManagerThread", "Unknown exception");
        }
    
    return 0;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
bool EventManager::Start()
    {
    if (m_eventManagerContext.IsValid())
        return true;

    LogHelper::Log(NativeLogging::SEVERITY::LOG_INFO, "EventManager::Start", "Start");
    m_eventManagerContext = new EventManagerContext(m_imodelConnectionP, this, SimpleCancellationToken::Create());
    BentleyStatus status = BeThreadUtilities::StartNewThread(EventManagerThread, &m_eventManagerContext, 1024*1024);
    BeConditionVariable& cv = m_eventManagerContext->GetConditionVariable();
    BeMutexHolder holder(cv.GetMutex());
    cv.RelativeWait(holder, 200);
    return BSISUCCESS == status;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr EventManager::Stop()
    {
    if (m_eventManagerContext.IsValid())
        m_eventManagerContext->StopManager();

    m_eventCallbacks.clear();
    if (m_imodelConnectionP)
        m_imodelConnectionP->UnsubscribeToEvents();

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
    LogHelper::Log(NativeLogging::SEVERITY::LOG_INFO, "EventManager::Stop", "Stop");
    return CreateCompletedAsyncTask<StatusResult>(StatusResult::Success());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
EventTypeSet* EventManager::GetAllSubscribedEvents()
    {
    EventTypeSet* allEventTypes = new EventTypeSet();

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
StatusTaskPtr EventManager::Subscribe(EventTypeSet* eventTypes, EventCallbackPtr callback)
    {
    // Check callback is already subscribed
    if (m_eventCallbacks.find(callback) != m_eventCallbacks.end())
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(Error::Id::EventCallbackAlreadySubscribed));


    auto types = eventTypes ? *eventTypes : EventTypeSet();
    m_eventCallbacks.Insert(callback, types);

    // Gather all event types
    EventTypeSet* allEventTypes = GetAllSubscribedEvents();
    auto subscribeResult = m_imodelConnectionP->SubscribeToEvents(allEventTypes);

    if (!subscribeResult->GetResult().IsSuccess())
        return subscribeResult;

    if (!Start())
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(Error::Id::EventServiceSubscribingError));

    return CreateCompletedAsyncTask<StatusResult>(StatusResult::Success());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr EventManager::Unsubscribe(EventCallbackPtr callback, bool* dispose)
    {
    *dispose = false;
    if (callback)
        {
        auto it = m_eventCallbacks.find(callback);
        bool callbackFound = it != m_eventCallbacks.end();

        if (callbackFound)
            m_eventCallbacks.erase(it);
        else
            return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(Error::Id::EventCallbackNotFound));
        }

    if (!callback || 0 == m_eventCallbacks.size())
        {
        *dispose = true;
        return Stop();
        }
    
    // Subscribe only to remaining events
    EventTypeSet* allEventTypes = GetAllSubscribedEvents();
    return m_imodelConnectionP->SubscribeToEvents(allEventTypes);
    }
