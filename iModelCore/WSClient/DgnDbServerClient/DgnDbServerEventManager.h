/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerEventManager.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
struct DgnDbServerEventManagerContext
    {
    private:
        DgnDbRepositoryConnectionPtr m_repositoryConnectionPtr;
        DgnDbServerEventManagerPtr m_managerPtr;
        SimpleCancellationTokenPtr m_cancellationTokenPtr;
        bool m_run = true;

    public:
        DgnDbServerEventManagerContext(DgnDbRepositoryConnectionPtr repositoryConnectionPtr, DgnDbServerEventManagerPtr manager, SimpleCancellationTokenPtr cancellationToken);
        DgnDbRepositoryConnectionPtr GetRepositoryConnectionPtr() const;
        DgnDbServerEventManagerPtr GetEventManagerPtr() const;
        SimpleCancellationTokenPtr GetCancellationTokenPtr() const;
        void StopManager();
    };

//=======================================================================================
//@bsiclass                                      Algirdas.Mikoliunas             12/2016
//=======================================================================================
struct DgnDbServerEventManager : std::enable_shared_from_this<DgnDbServerEventManager>
    {
    private:
        DgnDbRepositoryConnectionPtr       m_repositoryConnectionPtr;
        DgnDbServerEventMap                m_eventCallbacks;
        DgnDbServerEventManagerContextPtr  m_eventManagerContext;
        DgnDbServerEventCallbackPtr        m_pullMergeAndPushCallback;

        bool Start();
        DgnDbServerEventTypeSet* GetAllSubscribedEvents();

    public:
        DgnDbServerEventManager(DgnDbRepositoryConnectionPtr repositoryConnectionPtr);
        DgnDbServerStatusTaskPtr Stop();
        DgnDbServerStatusTaskPtr Subscribe(DgnDbServerEventTypeSet* eventTypes, DgnDbServerEventCallbackPtr callback);
        DgnDbServerStatusTaskPtr Unsubscribe(DgnDbServerEventCallbackPtr callback);
        DgnDbServerEventMap GetCallbacks() const;
        virtual ~DgnDbServerEventManager();
    };

END_BENTLEY_DGNDBSERVER_NAMESPACE
