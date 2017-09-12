/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/Events/EventManager.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <WebServices/iModelHub/Common.h>
#include <WebServices/iModelHub/Client/iModelConnection.h>
#include <WebServices/iModelHub/Client/Briefcase.h>
#include <Bentley/BeThread.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

//=======================================================================================
//@bsiclass                                      Algirdas.Mikoliunas             12/2016
//=======================================================================================
struct EventManagerContext : RefCountedBase
{
private:
    iModelConnectionP m_imodelConnectionP;
    EventManagerP m_managerP;
    SimpleCancellationTokenPtr m_cancellationTokenPtr;
    mutable BeConditionVariable m_cv;

public:
    EventManagerContext(iModelConnectionP imodelConnectionPtr, EventManagerP manager, SimpleCancellationTokenPtr cancellationToken)
        :m_imodelConnectionP(imodelConnectionPtr), m_managerP(manager), m_cancellationTokenPtr(cancellationToken) {}
    iModelConnectionP GetiModelConnectionP() const {return m_imodelConnectionP;}
    EventManagerP GetEventManagerP() const {return m_managerP;}
    SimpleCancellationTokenPtr GetCancellationTokenPtr() const {return m_cancellationTokenPtr;}
    void StopManager() {m_cancellationTokenPtr->SetCanceled();}
    BeConditionVariable& GetConditionVariable() const {return m_cv;}
};

//=======================================================================================
//@bsiclass                                      Algirdas.Mikoliunas             12/2016
//=======================================================================================
struct EventManager : RefCountedBase
{
private:
    iModelConnectionP       m_imodelConnectionP;
    EventMap                m_eventCallbacks;
    BeMutex                 m_eventCallbacksMutex;
    EventManagerContextPtr  m_eventManagerContext;
    EventCallbackPtr        m_pullMergeAndPushCallback;

    bool Start();
    void GetAllSubscribedEvents(EventTypeSet& allEventTypes);

public:
    EventManager(iModelConnectionP imodelConnectionP) : m_imodelConnectionP(imodelConnectionP) {}
    StatusTaskPtr Stop();
    StatusTaskPtr Subscribe(EventTypeSet* eventTypes, EventCallbackPtr callback);
    StatusTaskPtr Unsubscribe(EventCallbackPtr callback, bool* dispose);
    EventMap GetCallbacks() const {return m_eventCallbacks;}
    virtual ~EventManager() {Stop();}
};

END_BENTLEY_IMODELHUB_NAMESPACE
