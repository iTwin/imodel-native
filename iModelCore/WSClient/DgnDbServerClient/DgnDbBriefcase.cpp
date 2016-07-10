/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbBriefcase.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/TxnManager.h>
#include <DgnDbServer/Client/DgnDbBriefcase.h>
#include <DgnPlatform/RevisionManager.h>
#include "DgnDbServerUtils.h"

#include <thread>
#include <random>

USING_NAMESPACE_BENTLEY_DGNDBSERVER
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS
USING_NAMESPACE_BENTLEY_SQLITE

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbBriefcase::DgnDbBriefcase(Dgn::DgnDbPtr db, DgnDbRepositoryConnectionPtr connection)
    {
    m_db = db;
    m_repositoryConnection = connection;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbBriefcasePtr DgnDbBriefcase::Create(Dgn::DgnDbPtr db, DgnDbRepositoryConnectionPtr connection)
    {
    return DgnDbBriefcasePtr(new DgnDbBriefcase(db, connection));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerRevisionMergeTaskPtr DgnDbBriefcase::PullAndMerge(HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    BeAssert(DgnDbServerHost::IsInitialized());
    std::shared_ptr<DgnDbServerHost> host = std::make_shared<DgnDbServerHost>();
    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        return CreateCompletedAsyncTask<DgnDbServerRevisionMergeResult>(DgnDbServerRevisionMergeResult::Error(DgnDbServerError::Id::FileNotFound));
        }
    if (!m_repositoryConnection)
        {
        return CreateCompletedAsyncTask<DgnDbServerRevisionMergeResult>(DgnDbServerRevisionMergeResult::Error(DgnDbServerError::Id::InvalidRepositoryConnection));
        }
    if (m_db->IsReadonly())
        {
        return CreateCompletedAsyncTask<DgnDbServerRevisionMergeResult>(DgnDbServerRevisionMergeResult::Error(DgnDbServerError::Id::BriefcaseIsReadOnly));
        }
    if (!m_db->Txns().IsTracking())
        {
        return CreateCompletedAsyncTask<DgnDbServerRevisionMergeResult> (DgnDbServerRevisionMergeResult::Error (DgnDbServerError::Id::TrackingNotEnabled));
        }
    Utf8String lastRevisionId = GetLastRevisionPulled ();
    return m_repositoryConnection->Pull(lastRevisionId, callback, cancellationToken)->Then<DgnDbServerRevisionsResult>([=] (DgnDbServerRevisionsResultCR result)
        {
        if (!result.IsSuccess())
            return DgnDbServerRevisionMergeResult::Error(result.GetError());

        auto serverRevisions = result.GetValue();
        RevisionStatus mergeStatus = RevisionStatus::Success;

        if (!serverRevisions.empty())
            {
            DgnDbServerHost::Adopt(host);

            for (auto revision : serverRevisions)
                {
                mergeStatus = m_db->Revisions().MergeRevision(*(revision->GetRevision()));
                if (mergeStatus != RevisionStatus::Success)
                    break; // TODO: Use the information on the revision that actually failed. 
                }
                    
            DgnDbServerHost::Forget(host);
            }

        if (RevisionStatus::Success == mergeStatus)
            return DgnDbServerRevisionMergeResult::Success(serverRevisions);

        return DgnDbServerRevisionMergeResult::Error(mergeStatus);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerRevisionMergeTaskPtr DgnDbBriefcase::PullMergeAndPush(Utf8CP description, HttpRequest::ProgressCallbackCR downloadCallback, HttpRequest::ProgressCallbackCR uploadCallback,
                                                                 ICancellationTokenPtr cancellationToken, int attemptsCount) const
    {
    return PullMergeAndPushRepeated(description, downloadCallback, uploadCallback, cancellationToken, attemptsCount);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                  01/2016
//---------------------------------------------------------------------------------------
DgnDbServerRevisionMergeTaskPtr DgnDbBriefcase::PullMergeAndPushRepeated(Utf8CP description, HttpRequest::ProgressCallbackCR downloadCallback, HttpRequest::ProgressCallbackCR uploadCallback,
                                                                     ICancellationTokenPtr cancellationToken, int attemptsCount, int attempt, int delay) const
    {
    DgnDbServerRevisionMergeResultPtr finalResult = std::make_shared<DgnDbServerRevisionMergeResult>();
    return PullMergeAndPushInternal(description, downloadCallback, uploadCallback, cancellationToken)
        ->Then([=] (DgnDbServerRevisionMergeResultCR result)
        {
        if (result.IsSuccess())
            {
            finalResult->SetSuccess(result.GetValue ());
            return;
            }

        if (attempt >= attemptsCount)
            {
            finalResult->SetError(result.GetError());
            return;
            }

        DgnDbServerError::Id errorId = result.GetError().GetId();
        switch (errorId)
            {
            case DgnDbServerError::Id::AnotherUserPushing:
            case DgnDbServerError::Id::PullIsRequired:
            case DgnDbServerError::Id::DatabaseTemporarilyLocked:
            case DgnDbServerError::Id::DgnDbServerOperationFailed:
                break;
            default:
                {
                finalResult->SetError(result.GetError());
                return;
                }
            }

        int currentDelay = delay * attempt;
        if (currentDelay > s_maxDelayTime)
            currentDelay = s_maxDelayTime;

        if (1 == attempt)
            {
            std::default_random_engine         randomEngine;
            std::uniform_int_distribution<int> distribution(50, 500);
            currentDelay = distribution(randomEngine);
            }

        // Sleep.
        std::this_thread::sleep_for(std::chrono::milliseconds(currentDelay));
        PullMergeAndPushRepeated(description, downloadCallback, uploadCallback, cancellationToken, attemptsCount, attempt + 1, delay)->Then([=] (DgnDbServerRevisionMergeResultCR result)
            {
            if (result.IsSuccess())
                finalResult->SetSuccess(result.GetValue());
            else
                finalResult->SetError(result.GetError());
            });
        })->Then<DgnDbServerRevisionMergeResult>([=]
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerRevisionMergeTaskPtr DgnDbBriefcase::PullMergeAndPushInternal(Utf8CP description, HttpRequest::ProgressCallbackCR downloadCallback,
    HttpRequest::ProgressCallbackCR uploadCallback, ICancellationTokenPtr cancellationToken) const
    {
    BeAssert(DgnDbServerHost::IsInitialized());
    std::shared_ptr<DgnDbServerHost> host = std::make_shared<DgnDbServerHost>();
    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        return CreateCompletedAsyncTask<DgnDbServerRevisionMergeResult>(DgnDbServerRevisionMergeResult::Error(DgnDbServerError::Id::FileNotFound));
        }
    if (!m_repositoryConnection)
        {
        return CreateCompletedAsyncTask<DgnDbServerRevisionMergeResult>(DgnDbServerRevisionMergeResult::Error(DgnDbServerError::Id::InvalidRepositoryConnection));
        }
    if (m_db->IsReadonly())
        {
        return CreateCompletedAsyncTask<DgnDbServerRevisionMergeResult>(DgnDbServerRevisionMergeResult::Error(DgnDbServerError::Id::BriefcaseIsReadOnly));
        }
    std::shared_ptr<DgnDbServerRevisionMergeResult> finalResult = std::make_shared<DgnDbServerRevisionMergeResult>();
    return PullAndMerge(downloadCallback, cancellationToken)->Then([=] (DgnDbServerRevisionMergeResultCR result)
        {
        if (!result.IsSuccess())
            {
            finalResult->SetError(result.GetError());
            return;
            }

        DgnDbServerHost::Adopt(host);
        DgnRevisionPtr revision = m_db->Revisions ().StartCreateRevision ();
        DgnDbServerHost::Forget (host, false);
        if (!revision.IsValid ())
            {
            // No changes. Return success.
            finalResult->SetSuccess(result.GetValue());
            return;
            }

        revision->SetSummary(description);

        Utf8String revisionId = revision->GetId();
        m_repositoryConnection->Push(revision, m_db->GetBriefcaseId (), uploadCallback, cancellationToken)->Then
            ([=] (DgnDbServerStatusResultCR pushResult)
            {
            if (pushResult.IsSuccess())
                {
                DgnDbServerHost::Adopt(host);
                Dgn::RevisionStatus status = m_db->Revisions().FinishCreateRevision();
                m_db->SaveChanges();
                DgnDbServerHost::Forget(host);
                if (RevisionStatus::Success == status)
                    {
                    finalResult->SetSuccess(result.GetValue());
                    }
                else
                    finalResult->SetError(status);
                }
            else
                {
                DgnDbServerHost::Adopt(host);
                m_db->Revisions().AbandonCreateRevision();
                DgnDbServerHost::Forget(host);
                finalResult->SetError(pushResult.GetError());
                }
            });

        })->Then<DgnDbServerRevisionMergeResult>([=] ()
        {
        return *finalResult;
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              03/2016
//---------------------------------------------------------------------------------------
DgnDbServerBoolTaskPtr DgnDbBriefcase::IsBriefcaseUpToDate (ICancellationTokenPtr cancellationToken) const
    {
    BeAssert (DgnDbServerHost::IsInitialized ());
    if (!m_db.IsValid () || !m_db->IsDbOpen ())
        {
        return CreateCompletedAsyncTask<DgnDbServerBoolResult> (DgnDbServerBoolResult::Error (DgnDbServerError::Id::FileNotFound));
        }
    if (!m_repositoryConnection)
        {
        return CreateCompletedAsyncTask<DgnDbServerBoolResult> (DgnDbServerBoolResult::Error (DgnDbServerError::Id::InvalidRepositoryConnection));
        }

    //Needswork: think how to optimize this so that we would not need to download all revisions
    return m_repositoryConnection->GetRevisionsAfterId (GetLastRevisionPulled (), cancellationToken)->Then<DgnDbServerBoolResult> ([=](DgnDbServerRevisionsResultCR result)
        {
        if (result.IsSuccess ())
            return DgnDbServerBoolResult::Success (result.GetValue ().size () <= 0); //If there are not pending revisions we are up to date

        return DgnDbServerBoolResult::Error (result.GetError ());
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Caleb.Shafer	                    06/2016
//---------------------------------------------------------------------------------------
bool DgnDbBriefcase::SetEventTypes(bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes)
    {
    BeAssert(DgnDbServerHost::IsInitialized());
    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        return false;
        }
    if (!m_repositoryConnection)
        {
        return false;
        }

    m_repositoryConnection->UpdateEventServiceClient(eventTypes, nullptr);

    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventStringTaskPtr DgnDbBriefcase::GetEventString(bool longPolling, ICancellationTokenPtr cancellationToken)
    {
    auto currentEventTask = GetEvent(longPolling, cancellationToken)->GetResult();
    if (!currentEventTask.IsSuccess())
        return CreateCompletedAsyncTask<DgnDbServerEventStringResult>(DgnDbServerEventStringResult::Error(currentEventTask.GetError()));

    DgnDbServerEventPtr currentEvent = currentEventTask.GetValue();
    /*DgnDbServerEvent::DgnDbServerEventType eventType = DgnDbServerEventParser::GetInstance().GetEventType(currentEvent);*/
    DgnDbServerEvent::DgnDbServerEventType eventType = currentEvent->GetEventType();
    switch (eventType)
        {
            case DgnDbServerEvent::DgnDbServerEventType::LockEvent:
                {
                std::shared_ptr<DgnDbServerLockEvent> lockEvent = DgnDbServerEventParser::GetInstance().GetLockEvent(currentEvent);
                if (lockEvent == nullptr)
                    return CreateCompletedAsyncTask<DgnDbServerEventStringResult>
                                    (DgnDbServerEventStringResult::Error
                                    (
                                    DgnDbServerError::Id::InternalServerError
                                    ));
                return CreateCompletedAsyncTask<DgnDbServerEventStringResult>
                                    (DgnDbServerEventStringResult::Success
                                    (
                                    "Lock Info-> LockId: " + lockEvent->GetObjectId() + 
                                    " LockType: " + lockEvent->GetLockType() + 
                                    " Date: " + lockEvent->GetDate()
                                    ));
                }

            case DgnDbServerEvent::DgnDbServerEventType::RevisionEvent:
                {
                std::shared_ptr<DgnDbServerRevisionEvent> revisionEvent = DgnDbServerEventParser::GetInstance().GetRevisionEvent(currentEvent);
                if (revisionEvent == nullptr)
                    return CreateCompletedAsyncTask<DgnDbServerEventStringResult>
                    (DgnDbServerEventStringResult::Error
                    (
                    DgnDbServerError::Id::InternalServerError
                    ));
                return CreateCompletedAsyncTask<DgnDbServerEventStringResult>
                    (DgnDbServerEventStringResult::Success
                    (
                    "Revision Info-> RevisionId: " + revisionEvent->GetRevisionId() +
                    " Date: " + revisionEvent->GetDate()
                    ));
                }

            case DgnDbServerEvent::DgnDbServerEventType::CodeEvent:
                {
                std::shared_ptr<DgnDbServerCodeEvent> codeEvent = DgnDbServerEventParser::GetInstance().GetCodeEvent(currentEvent);
                if (codeEvent == nullptr)
                    return CreateCompletedAsyncTask<DgnDbServerEventStringResult>
                    (DgnDbServerEventStringResult::Error
                    (
                    DgnDbServerError::Id::InternalServerError
                    ));
                return CreateCompletedAsyncTask<DgnDbServerEventStringResult>
                    (DgnDbServerEventStringResult::Success
                    (
                    "Code Info-> CodeAuthority: " + codeEvent->GetCodeAuthorityId() +
                    " State: " + codeEvent->GetState()
                    ));
                }

            case DgnDbServerEvent::DgnDbServerEventType::AllCodesDeletedEvent:
            case DgnDbServerEvent::DgnDbServerEventType::AllLocksDeletedEvent:
                {
                std::shared_ptr<DgnDbServerDeletedEvent> deletedEvent = DgnDbServerEventParser::GetInstance().GetDeletedEvent(currentEvent);
                if (deletedEvent == nullptr)
                    return CreateCompletedAsyncTask<DgnDbServerEventStringResult>
                    (DgnDbServerEventStringResult::Error
                    (
                    DgnDbServerError::Id::InternalServerError
                    ));
                return CreateCompletedAsyncTask<DgnDbServerEventStringResult>
                    (DgnDbServerEventStringResult::Success
                    (
                    DgnDbServerEvent::Helper::GetEventNameFromEventType(eventType) + " Activated"
                    ));
                }

            case DgnDbServerEvent::DgnDbServerEventType::UnknownEventType:
            default:
                {
                return CreateCompletedAsyncTask<DgnDbServerEventStringResult>
                    (DgnDbServerEventStringResult::Error(DgnDbServerError::Id::NoEventsFound));
                }
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventTaskPtr  DgnDbBriefcase::GetEvent(bool longPolling, ICancellationTokenPtr cancellationToken)
    {
    BeAssert(DgnDbServerHost::IsInitialized());
    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        return CreateCompletedAsyncTask<DgnDbServerEventResult>(DgnDbServerEventResult::Error(DgnDbServerError::Id::FileNotFound));
        }
    if (!m_repositoryConnection)
        {
        return CreateCompletedAsyncTask<DgnDbServerEventResult>(DgnDbServerEventResult::Error(DgnDbServerError::Id::InvalidRepositoryConnection));
        }

    DgnDbServerEventTaskPtr currentEventTask = m_repositoryConnection->GetEvent(longPolling, cancellationToken);
    auto getEventResult = currentEventTask->GetResult();
    if (!getEventResult.IsSuccess())
        return CreateCompletedAsyncTask<DgnDbServerEventResult>(DgnDbServerEventResult::Error(DgnDbServerError::Id::InternalServerError));

    DgnDbServerEventPtr currentEvent = getEventResult.GetValue();
    if (currentEvent == nullptr)
        return CreateCompletedAsyncTask<DgnDbServerEventResult>(DgnDbServerEventResult::Error(DgnDbServerError::Id::NoEventsFound));
    return CreateCompletedAsyncTask<DgnDbServerEventResult>(DgnDbServerEventResult::Success(currentEvent));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventCollectionTaskPtr DgnDbBriefcase::GetEvents(bool longPolling, ICancellationTokenPtr cancellationToken)
    {
    BeAssert(DgnDbServerHost::IsInitialized());
    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        return CreateCompletedAsyncTask<DgnDbServerEventCollectionResult>(DgnDbServerEventCollectionResult::Error(DgnDbServerError::Id::FileNotFound));
        }
    if (!m_repositoryConnection)
        {
        return CreateCompletedAsyncTask<DgnDbServerEventCollectionResult>(DgnDbServerEventCollectionResult::Error(DgnDbServerError::Id::InvalidRepositoryConnection));
        }

    return m_repositoryConnection->GetEvents(longPolling, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              06/2016
//---------------------------------------------------------------------------------------
DgnDbServerCancelEventTaskPtr  DgnDbBriefcase::CancelEventRequest(ICancellationTokenPtr cancellationToken)
    {
    BeAssert(DgnDbServerHost::IsInitialized());
    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        return CreateCompletedAsyncTask<DgnDbServerCancelEventResult>(DgnDbServerCancelEventResult::Error(DgnDbServerError::Id::FileNotFound));
        }
    if (!m_repositoryConnection)
        {
        return CreateCompletedAsyncTask<DgnDbServerCancelEventResult>(DgnDbServerCancelEventResult::Error(DgnDbServerError::Id::InvalidRepositoryConnection));
        }

    return m_repositoryConnection->CancelEventRequest(cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
Dgn::DgnDbR DgnDbBriefcase::GetDgnDb() const
    {
    return *m_db;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbRepositoryConnectionCR DgnDbBriefcase::GetRepositoryConnection() const
    {
    return *m_repositoryConnection;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              01/2016
//---------------------------------------------------------------------------------------
BeBriefcaseId DgnDbBriefcase::GetBriefcaseId () const
    {
    return GetDgnDb ().GetBriefcaseId ();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              01/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbBriefcase::GetLastRevisionPulled () const
    {
    return GetDgnDb ().Revisions ().GetParentRevisionId ();
    }
