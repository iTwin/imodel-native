/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/Events/EventCallbackManager.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/TxnManager.h>
#include <WebServices/iModelHub/Client/Briefcase.h>
#include <WebServices/iModelHub/Client/Result.h>
#include <thread>
#include "EventCallbackManager.h"
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
void* EventCallbackManagerThread(void* arg)
#elif defined (_WIN32) // Windows && WinRT
unsigned __stdcall EventCallbackManagerThread(void* arg)
#endif
    {
    const Utf8String methodName = "EventCallbackManagerThread";
    try
        {
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Starting event manager thread.");
        EventCallbackManagerContextPtr* managerContextPtr = (EventCallbackManagerContextPtr*)arg;
        if (nullptr == managerContextPtr)
            {
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Invalid argument.");
            return 0;
            }
        EventCallbackManagerContextPtr managerContext = *managerContextPtr;
        if (managerContext.IsNull())
            {
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Invalid context.");
            return 0;
            }
        managerContext->GetConditionVariable().notify_one();

        LogHelper::Log(SEVERITY::LOG_TRACE, methodName, "Getting context members.");
        iModelConnectionP imodelConnectionP = managerContext->GetiModelConnectionP();
        EventCallbackManagerP eventCallbackManager = managerContext->GetEventCallbackManagerP();
        ICancellationTokenPtr cancellationTokenPtr = managerContext->GetCancellationTokenPtr();

        if (nullptr == imodelConnectionP || nullptr == eventCallbackManager)
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
                for (auto callback : eventCallbackManager->GetCallbacks())
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
        LogHelper::Log(NativeLogging::SEVERITY::LOG_WARNING, "EventCallbackManagerThread", e.what());
        }
    catch (Utf8StringCR message)
        {
        LogHelper::Log(NativeLogging::SEVERITY::LOG_WARNING, "EventCallbackManagerThread", message.c_str());
        }
    catch (...)
        {
        LogHelper::Log(NativeLogging::SEVERITY::LOG_WARNING, "EventCallbackManagerThread", "Unknown exception");
        }

    return 0;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
bool EventCallbackManager::Start()
    {
    if (m_eventCallbackManagerContext.IsValid())
        return true;

    LogHelper::Log(NativeLogging::SEVERITY::LOG_INFO, "EventCallbackManager::Start", "Start");
    m_eventCallbackManagerContext = new EventCallbackManagerContext(m_imodelConnectionP, this, SimpleCancellationToken::Create());
    BentleyStatus status = BeThreadUtilities::StartNewThread(EventCallbackManagerThread, &m_eventCallbackManagerContext, 1024 * 1024);
    BeConditionVariable& cv = m_eventCallbackManagerContext->GetConditionVariable();
    BeMutexHolder holder(cv.GetMutex());
    cv.RelativeWait(holder, 200);
    return BSISUCCESS == status;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr EventCallbackManager::Stop()
    {
    if (m_eventCallbackManagerContext.IsValid())
        m_eventCallbackManagerContext->StopManager();

    m_eventCallbacks.clear();
    if (m_imodelConnectionP)
        m_imodelConnectionP->UnsubscribeToEvents();

    // Wait for events thread to finish
    int maxIterations = 100;
    while (m_eventCallbackManagerContext.IsValid() && maxIterations > 0)
        {
        int useCount = m_eventCallbackManagerContext->GetRefCount();
        if (useCount <= 1)
            break;

        BeThreadUtilities::BeSleep(50);
        maxIterations--;
        }
    BeAssert(m_eventCallbackManagerContext.IsNull() || 1 >= m_eventCallbackManagerContext->GetRefCount() && "Events thread not finished!");

    m_eventCallbackManagerContext = nullptr;
    LogHelper::Log(NativeLogging::SEVERITY::LOG_INFO, "EventCallbackManager::Stop", "Stop");
    return CreateCompletedAsyncTask<StatusResult>(StatusResult::Success());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
void EventCallbackManager::GetAllSubscribedEvents(EventTypeSet& allEventTypes)
    {
    for (auto eventCallback : m_eventCallbacks)
        {
        if (eventCallback.second.empty())
            {
            return;
            }

        for (auto eventType : eventCallback.second)
            {
            if (!EventListContainsEvent(allEventTypes, eventType))
                allEventTypes.insert(eventType);
            }
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr EventCallbackManager::Subscribe(EventTypeSet* eventTypes, EventCallbackPtr callback)
    {
    const Utf8String methodName = "EventCallbackManager::Subscribe";
    BeMutexHolder lock(m_eventCallbacksMutex);

    // Check callback is already subscribed
    if (m_eventCallbacks.find(callback) != m_eventCallbacks.end())
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(Error::Id::EventCallbackAlreadySubscribed));

    auto types = eventTypes ? *eventTypes : EventTypeSet();
    m_eventCallbacks.Insert(callback, types);

    // Gather all event types
    EventTypeSet allEventTypes;
    GetAllSubscribedEvents(allEventTypes);
    return m_imodelConnectionP->SubscribeToEvents(&allEventTypes)->Then<StatusResult>([=](StatusResultCR subscribeResult)
        {
        if (!subscribeResult.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, subscribeResult.GetError().GetMessage().c_str());
            return subscribeResult;
            }

        if (!Start())
            {
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Thread start failed");
            return StatusResult::Error(Error::Id::EventServiceSubscribingError);
            }

        return StatusResult::Success();
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr EventCallbackManager::Unsubscribe(EventCallbackPtr callback, bool* dispose)
    {
    BeMutexHolder lock(m_eventCallbacksMutex);

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
    EventTypeSet allEventTypes;
    GetAllSubscribedEvents(allEventTypes);
    return m_imodelConnectionP->SubscribeToEvents(&allEventTypes);
    }
