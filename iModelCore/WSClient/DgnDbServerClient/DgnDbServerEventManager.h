/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerEventManager.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/DgnDbRepositoryConnection.h>
#include <DgnDbServer/Client/DgnDbBriefcase.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE

//=======================================================================================
//@bsiclass                                      Algirdas.Mikoliunas             12/2016
//=======================================================================================
struct DgnDbServerEventManagerContext : RefCountedBase
{
private:
    DgnDbRepositoryConnectionP m_repositoryConnectionP;
    DgnDbServerEventManagerP m_managerP;
    SimpleCancellationTokenPtr m_cancellationTokenPtr;

public:
    DgnDbServerEventManagerContext(DgnDbRepositoryConnectionP repositoryConnectionPtr, DgnDbServerEventManagerP manager, SimpleCancellationTokenPtr cancellationToken)
        :m_repositoryConnectionP(repositoryConnectionPtr), m_managerP(manager), m_cancellationTokenPtr(cancellationToken) {}
    DgnDbRepositoryConnectionP GetRepositoryConnectionP() const {return m_repositoryConnectionP;}
    DgnDbServerEventManagerP GetEventManagerP() const {return m_managerP;}
    SimpleCancellationTokenPtr GetCancellationTokenPtr() const {return m_cancellationTokenPtr;}
    void StopManager() {m_cancellationTokenPtr->SetCanceled();}
};

//=======================================================================================
//@bsiclass                                      Algirdas.Mikoliunas             12/2016
//=======================================================================================
struct DgnDbServerEventManager : RefCountedBase
{
private:
    DgnDbRepositoryConnectionP         m_repositoryConnectionP;
    DgnDbServerEventMap                m_eventCallbacks;
    DgnDbServerEventManagerContextPtr  m_eventManagerContext;
    DgnDbServerEventCallbackPtr        m_pullMergeAndPushCallback;

    bool Start();
    DgnDbServerEventTypeSet* GetAllSubscribedEvents();

public:
    DgnDbServerEventManager(DgnDbRepositoryConnectionP repositoryConnectionP) : m_repositoryConnectionP(repositoryConnectionP) {}
    DgnDbServerStatusTaskPtr Stop();
    DgnDbServerStatusTaskPtr Subscribe(DgnDbServerEventTypeSet* eventTypes, DgnDbServerEventCallbackPtr callback);
    DgnDbServerStatusTaskPtr Unsubscribe(DgnDbServerEventCallbackPtr callback, bool* dispose);
    DgnDbServerEventMap GetCallbacks() const {return m_eventCallbacks;}
    virtual ~DgnDbServerEventManager() {Stop();}
};

END_BENTLEY_DGNDBSERVER_NAMESPACE
