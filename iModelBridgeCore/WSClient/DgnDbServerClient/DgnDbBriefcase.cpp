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
DgnDbServerRevisionMergeTaskPtr DgnDbBriefcase::PullMergeAndPush(HttpRequest::ProgressCallbackCR downloadCallback, HttpRequest::ProgressCallbackCR uploadCallback,
                                                                 ICancellationTokenPtr cancellationToken, int attemptsCount) const
    {
    return PullMergeAndPushRepeated(downloadCallback, uploadCallback, cancellationToken, attemptsCount);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                  01/2016
//---------------------------------------------------------------------------------------
DgnDbServerRevisionMergeTaskPtr DgnDbBriefcase::PullMergeAndPushRepeated(HttpRequest::ProgressCallbackCR downloadCallback, HttpRequest::ProgressCallbackCR uploadCallback,
                                                                     ICancellationTokenPtr cancellationToken, int attemptsCount, int attempt, int delay) const
    {
    DgnDbServerRevisionMergeResultPtr finalResult = std::make_shared<DgnDbServerRevisionMergeResult>();
    return PullMergeAndPushInternal(downloadCallback, uploadCallback, cancellationToken)
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
        PullMergeAndPushRepeated(downloadCallback, uploadCallback, cancellationToken, attemptsCount, attempt + 1, delay)->Then([=] (DgnDbServerRevisionMergeResultCR result)
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
DgnDbServerRevisionMergeTaskPtr DgnDbBriefcase::PullMergeAndPushInternal(HttpRequest::ProgressCallbackCR downloadCallback,
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
            finalResult->SetSuccess(result.GetValue());
            return;
            }

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

////---------------------------------------------------------------------------------------
////@bsimethod                                     Caleb.Shafer	              06/2016
////---------------------------------------------------------------------------------------
//EventServiceReceiveTaskPtr DgnDbBriefcase::GetEvents(bool longPolling, ICancellationTokenPtr cancellationToken) const
//	{
//	BeAssert (DgnDbServerHost::IsInitialized ());
//    if (!m_db.IsValid () || !m_db->IsDbOpen ())
//        {
//		return CreateCompletedAsyncTask<EventServiceReceiveResult>(EventServiceReceiveResult::Error(DgnDbServerError::Id::FileNotFound));
//        }
//    if (!m_repositoryConnection)
//        {
//        return CreateCompletedAsyncTask<EventServiceReceiveResult>(EventServiceReceiveResult::Error(DgnDbServerError::Id::InvalidRepositoryConnection));
//        }
//
//	return m_repositoryConnection->GetEvents(longPolling, cancellationToken);
//	}

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              06/2016
//---------------------------------------------------------------------------------------
IDgnDbServerEventTaskPtr DgnDbBriefcase::GetEvent(bool longPolling, ICancellationTokenPtr cancellationToken) const
    {
    BeAssert(DgnDbServerHost::IsInitialized());
    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        return CreateCompletedAsyncTask<IDgnDbServerEventResult>(IDgnDbServerEventResult::Error(DgnDbServerError::Id::FileNotFound));
        }
    if (!m_repositoryConnection)
        {
        return CreateCompletedAsyncTask<IDgnDbServerEventResult>(IDgnDbServerEventResult::Error(DgnDbServerError::Id::InvalidRepositoryConnection));
        }

    return m_repositoryConnection->GetEvent(longPolling, cancellationToken);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              06/2016
//---------------------------------------------------------------------------------------
IDgnDbServerEventsTaskPtr DgnDbBriefcase::GetEvents(bool longPolling, ICancellationTokenPtr cancellationToken) const
    {
    BeAssert(DgnDbServerHost::IsInitialized());
    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        return CreateCompletedAsyncTask<IDgnDbServerEventsResult>(IDgnDbServerEventsResult::Error(DgnDbServerError::Id::FileNotFound));
        }
    if (!m_repositoryConnection)
        {
        return CreateCompletedAsyncTask<IDgnDbServerEventsResult>(IDgnDbServerEventsResult::Error(DgnDbServerError::Id::InvalidRepositoryConnection));
        }

    return m_repositoryConnection->GetEvents(longPolling, cancellationToken);
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
