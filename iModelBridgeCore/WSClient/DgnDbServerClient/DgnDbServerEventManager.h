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
        bool m_run = true;

    public:
        DgnDbServerEventManagerContext(DgnDbRepositoryConnectionPtr repositoryConnectionPtr, DgnDbServerEventManagerPtr manager);
        DgnDbRepositoryConnectionPtr GetRepositoryConnectionPtr() const;
        DgnDbServerEventManagerPtr GetEventManagerPtr() const;
        bool Run() const;
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
        DgnDbServerEventCallback           m_pullMergeAndPushCallback;

        bool Start();
        bvector<DgnDbServerEvent::DgnDbServerEventType>* GetAllSubscribedEvents();

    public:
        DgnDbServerEventManager(RepositoryInfoCR repository, WebServices::CredentialsCR credentials, WebServices::ClientInfoPtr clientInfo);
        DgnDbServerStatusTaskPtr Stop();
        DgnDbServerStatusTaskPtr Subscribe(bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes, DgnDbServerEventCallback callback);
        DgnDbServerStatusTaskPtr Unsubscribe(DgnDbServerEventCallback callback);
        DgnDbServerEventMap GetCallbacks() const;
        virtual ~DgnDbServerEventManager();
    };

END_BENTLEY_DGNDBSERVER_NAMESPACE
