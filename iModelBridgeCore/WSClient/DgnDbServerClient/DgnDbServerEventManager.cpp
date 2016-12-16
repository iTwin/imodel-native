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
DgnDbServerEventManagerContext::DgnDbServerEventManagerContext(DgnDbRepositoryConnectionPtr repositoryConnectionPtr, DgnDbServerEventManagerPtr manager)
    {
    m_repositoryConnectionPtr = repositoryConnectionPtr;
    m_managerPtr = manager;
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
bool DgnDbServerEventManagerContext::Run() const
    {
    return m_run;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
void DgnDbServerEventManagerContext::StopManager()
    {
    m_run = false;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
bool EventListContainsEvent(bvector<DgnDbServerEvent::DgnDbServerEventType> eventList, DgnDbServerEvent::DgnDbServerEventType eventType)
    {
    for(auto eventItem : eventList)
        {
        if (eventItem == eventType)
            return true;
        }

    return false;
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

    while (true)
        {
        if (!managerContext || !managerContext->Run())
            {
            return 0;
            }

        if (!repositoryConnectionPtr->IsSubscribedToEvents())
            {
            BeThreadUtilities::BeSleep(50);
            continue;
            }

        auto eventResult = repositoryConnectionPtr->GetEvent(true)->GetResult();
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
DgnDbServerEventManager::DgnDbServerEventManager(RepositoryInfoCR repository, WebServices::CredentialsCR credentials, WebServices::ClientInfoPtr clientInfo)
    {
    m_repositoryConnectionPtr = DgnDbRepositoryConnection::Create(repository, credentials, clientInfo).GetValue();;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
bool DgnDbServerEventManager::Start()
    {
    if (m_eventManagerContext)
        return true;

    DgnDbServerLogHelper::Log(NativeLogging::SEVERITY::LOG_INFO, "DgnDbServerEventManager::Start", "Start");
    m_eventManagerContext = std::make_shared<DgnDbServerEventManagerContext>(m_repositoryConnectionPtr, shared_from_this());
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
bvector<DgnDbServerEvent::DgnDbServerEventType>* DgnDbServerEventManager::GetAllSubscribedEvents()
    {
    bvector<DgnDbServerEvent::DgnDbServerEventType>* allEventTypes = new bvector<DgnDbServerEvent::DgnDbServerEventType>();

    for (auto eventCallback : m_eventCallbacks)
        {
        if (eventCallback.second.empty())
            {
            return nullptr;
            }

        for(auto eventType : eventCallback.second)
            {
            if (!EventListContainsEvent(*allEventTypes, eventType))
                allEventTypes->push_back(eventType);
            }
        }

    return allEventTypes;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbServerEventManager::Subscribe(bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes, DgnDbServerEventCallbackPtr callback)
    {
    // Check callback is already subscribed
    for (auto it = m_eventCallbacks.begin(); it != m_eventCallbacks.end(); it++)
        if ((*it).first == callback)
            return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(DgnDbServerError::Id::EventCallbackAlreadySubscribed));

    if (!Start())
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(DgnDbServerError::Id::EventServiceSubscribingError));

    auto types = eventTypes ? *eventTypes : bvector<DgnDbServerEvent::DgnDbServerEventType>();
    m_eventCallbacks.push_back(make_bpair(callback, types));

    // Gather all event types
    bvector<DgnDbServerEvent::DgnDbServerEventType>* allEventTypes = GetAllSubscribedEvents();
    return m_repositoryConnectionPtr->SubscribeToEvents(allEventTypes);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbServerEventManager::Unsubscribe(DgnDbServerEventCallbackPtr callback)
    {
    if (callback)
        {
        bool callbackFound = false;
        for (auto it = m_eventCallbacks.begin(); it != m_eventCallbacks.end(); it++)
            {
            if ((*it).first == callback)
                {
                m_eventCallbacks.erase(it);
                callbackFound = true;
                break;
                }
            }

        if (!callbackFound)
            return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(DgnDbServerError::Id::EventCallbackNotFound));
        }

    if (!callback || 0 == m_eventCallbacks.size())
        {
        return Stop();
        }
    
    // Subscribe only to remaining events
    bvector<DgnDbServerEvent::DgnDbServerEventType>* allEventTypes = GetAllSubscribedEvents();
    m_repositoryConnectionPtr->SubscribeToEvents(allEventTypes);
    
    return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Success());
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
