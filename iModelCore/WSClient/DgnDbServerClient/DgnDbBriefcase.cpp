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
#include <DgnDbServer/Client/Logging.h>
#include "DgnDbServerUtils.h"
#include <DgnDbServer/Client/DgnDbServerBreakHelper.h>
#include <thread>
#include <random>

USING_NAMESPACE_BENTLEY_DGNDBSERVER
USING_NAMESPACE_BENTLEY_DGNPLATFORM
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
//@bsimethod                                     Karolis.Dziedzelis             11/2016
//---------------------------------------------------------------------------------------
DgnDbServerRevisionsTaskPtr DgnDbBriefcase::Pull(Http::Request::ProgressCallbackCR callback, Tasks::ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbBriefcase::Pull";
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<DgnDbServerRevisionsResult>(DgnDbServerRevisionsResult::Error(DgnDbServerError::Id::FileNotFound));
        }
    if (!m_repositoryConnection)
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid repository connection.");
        return CreateCompletedAsyncTask<DgnDbServerRevisionsResult>(DgnDbServerRevisionsResult::Error(DgnDbServerError::Id::InvalidRepositoryConnection));
        }
    if (m_db->IsReadonly())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Briefcase is read only.");
        return CreateCompletedAsyncTask<DgnDbServerRevisionsResult>(DgnDbServerRevisionsResult::Error(DgnDbServerError::Id::BriefcaseIsReadOnly));
        }
    if (!m_db->Txns().IsTracking())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Tracking is not enabled.");
        return CreateCompletedAsyncTask<DgnDbServerRevisionsResult>(DgnDbServerRevisionsResult::Error(DgnDbServerError::Id::TrackingNotEnabled));
        }

    Utf8String lastRevisionId = GetLastRevisionPulled();
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "%s%s", Utf8String::IsNullOrEmpty(lastRevisionId.c_str()) ? "No revisions pulled yet" : "Downloading revisions after revision ", lastRevisionId.c_str());

    return m_repositoryConnection->DownloadRevisionsAfterId(lastRevisionId, GetDgnDb().GetDbGuid(), callback, cancellationToken)
        ->Then<DgnDbServerRevisionsResult>([=] (DgnDbServerRevisionsResultCR result)
        {
        if (!result.IsSuccess())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            return DgnDbServerRevisionsResult::Error(result.GetError());
            }

        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float) (end - start), "Revisions pulled successfully.");
        return result;
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbBriefcase::Merge(bvector<DgnDbServerRevisionPtr> const& revisions) const
    {
    const Utf8String methodName = "DgnDbBriefcase::Merge";
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(DgnDbServerError::Id::FileNotFound));
        }
    if (!m_repositoryConnection)
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid repository connection.");
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(DgnDbServerError::Id::InvalidRepositoryConnection));
        }
    if (m_db->IsReadonly())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Briefcase is read only.");
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(DgnDbServerError::Id::BriefcaseIsReadOnly));
        }
    if (!m_db->Txns().IsTracking())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Tracking is not enabled.");
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(DgnDbServerError::Id::TrackingNotEnabled));
        }

    RevisionStatus mergeStatus = RevisionStatus::Success;

    if (!revisions.empty())
        {
        for (auto revision : revisions)
            {
            mergeStatus = m_db->Revisions().MergeRevision(*(revision->GetRevision()));
            if (mergeStatus != RevisionStatus::Success)
                break; // TODO: Use the information on the revision that actually failed. 
            }
        }

    if (RevisionStatus::Success == mergeStatus)
        {
        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float) (end - start), "Revisions merged successfully.");
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Success());
        }

    DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Merge failed.");
    return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(mergeStatus));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbBriefcase::Push(Utf8CP description, bool relinquishCodesLocks, Http::Request::ProgressCallbackCR uploadCallback, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbBriefcase::Push";
    // NOT USED: double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(DgnDbServerError::Id::FileNotFound));
        }
    if (!m_repositoryConnection)
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid repository connection.");
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(DgnDbServerError::Id::InvalidRepositoryConnection));
        }
    if (m_db->IsReadonly())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Briefcase is read only.");
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(DgnDbServerError::Id::BriefcaseIsReadOnly));
        }
    if (!m_db->Txns().IsTracking())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Tracking is not enabled.");
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(DgnDbServerError::Id::TrackingNotEnabled));
        }

#if defined (ENABLE_BIM_CRASH_TESTS)
    DgnDbServerBreakHelper::HitBreakpoint(DgnDbServerBreakpoints::BeforeStartCreateRevision);
#endif
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Starting to create a new revision.");
    DgnRevisionPtr revision = m_db->Revisions().StartCreateRevision();
#if defined (ENABLE_BIM_CRASH_TESTS)
    DgnDbServerBreakHelper::HitBreakpoint(DgnDbServerBreakpoints::AfterStartCreateRevision);
#endif

    if (!revision.IsValid())
        {
        // No changes. Return success.
        DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "No changes found.");
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Success());
        }

    revision->SetSummary(description);

    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Created revision with ID %s.", revision->GetId().c_str());
    Utf8String revisionId = revision->GetId();
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Starting push.");

#if defined (ENABLE_BIM_CRASH_TESTS)
    DgnDbServerBreakHelper::HitBreakpoint(DgnDbServerBreakpoints::BeforePushRevisionToServer);
#endif
    return m_repositoryConnection->Push(revision, *m_db, relinquishCodesLocks, uploadCallback, cancellationToken)
        ->Then<DgnDbServerStatusResult>([=] (DgnDbServerStatusResultCR pushResult)
        {
#if defined (ENABLE_BIM_CRASH_TESTS)
        DgnDbServerBreakHelper::HitBreakpoint(DgnDbServerBreakpoints::AfterPushRevisionToServer);
#endif

        if (pushResult.IsSuccess())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Push successful. Finishing creating new revision.");
            Dgn::RevisionStatus status = m_db->Revisions().FinishCreateRevision();

#if defined (ENABLE_BIM_CRASH_TESTS)
            DgnDbServerBreakHelper::HitBreakpoint(DgnDbServerBreakpoints::AfterFinishCreateRevision);
#endif
            m_db->SaveChanges();
            if (RevisionStatus::Success == status)
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "New revision created successfully.");
                return DgnDbServerStatusResult::Success();
                }
            else
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "An error occured while trying to finish creating a revision.");
                return DgnDbServerStatusResult::Error(status);
                }
            }
        else
            {
            m_db->Revisions().AbandonCreateRevision();
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, pushResult.GetError().GetMessage().c_str());
            return DgnDbServerStatusResult::Error(pushResult.GetError());
            }
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerRevisionMergeTaskPtr DgnDbBriefcase::PullAndMerge(Http::Request::ProgressCallbackCR callback, Tasks::ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbBriefcase::PullAndMerge";
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    auto result = Pull(callback, cancellationToken)->GetResult();

#if defined (ENABLE_BIM_CRASH_TESTS)
        DgnDbServerBreakHelper::HitBreakpoint(DgnDbServerBreakpoints::AfterDownloadRevisions);
#endif
    if (!result.IsSuccess())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Pull failed.");
        return CreateCompletedAsyncTask<DgnDbServerRevisionMergeResult>(DgnDbServerRevisionMergeResult::Error(result.GetError()));
        }

    auto serverRevisions = result.GetValue();
    auto mergeResult = Merge(serverRevisions)->GetResult();

    if (!mergeResult.IsSuccess())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Merge failed.");
        return CreateCompletedAsyncTask<DgnDbServerRevisionMergeResult>(DgnDbServerRevisionMergeResult::Error(mergeResult.GetError()));
        }

    double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float) (end - start), "Revisions merged successfully.");
    return CreateCompletedAsyncTask<DgnDbServerRevisionMergeResult>(DgnDbServerRevisionMergeResult::Success(serverRevisions));

    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerRevisionMergeTaskPtr DgnDbBriefcase::PullMergeAndPush(Utf8CP description, bool relinquishCodesLocks, Http::Request::ProgressCallbackCR downloadCallback, 
                                                                 Http::Request::ProgressCallbackCR uploadCallback, ICancellationTokenPtr cancellationToken, int attemptsCount) const
    {
    const Utf8String methodName = "DgnDbBriefcase::PullMergeAndPush";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    return PullMergeAndPushRepeated(description, relinquishCodesLocks, downloadCallback, uploadCallback, cancellationToken, attemptsCount);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                  01/2016
//---------------------------------------------------------------------------------------
DgnDbServerRevisionMergeTaskPtr DgnDbBriefcase::PullMergeAndPushRepeated(Utf8CP description, bool relinquishCodesLocks, Http::Request::ProgressCallbackCR downloadCallback, Http::Request::ProgressCallbackCR uploadCallback,
                                                                     ICancellationTokenPtr cancellationToken, int attemptsCount, int attempt, int delay) const
    {
    const Utf8String methodName = "DgnDbBriefcase::PullMergeAndPushRepeated";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Attempt %d/%d.", attempt, attemptsCount);
    auto result = PullMergeAndPushInternal(description, relinquishCodesLocks, downloadCallback, uploadCallback, cancellationToken)->GetResult();

    if (result.IsSuccess())
        {
        return CreateCompletedAsyncTask<DgnDbServerRevisionMergeResult>(DgnDbServerRevisionMergeResult::Success(result.GetValue()));
        }

    if (attempt >= attemptsCount)
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Too many unsuccessful attempts.");
        return CreateCompletedAsyncTask<DgnDbServerRevisionMergeResult>(DgnDbServerRevisionMergeResult::Error(result.GetError()));
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
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            return CreateCompletedAsyncTask<DgnDbServerRevisionMergeResult>(DgnDbServerRevisionMergeResult::Error(result.GetError()));
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
    std::this_thread::sleep_for (std::chrono::milliseconds(currentDelay));
    return PullMergeAndPushRepeated(description, relinquishCodesLocks, downloadCallback, uploadCallback, cancellationToken, attemptsCount, attempt + 1, delay);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbServerRevisionMergeTaskPtr DgnDbBriefcase::PullMergeAndPushInternal(Utf8CP description, bool relinquishCodesLocks, Http::Request::ProgressCallbackCR downloadCallback,
    Http::Request::ProgressCallbackCR uploadCallback, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbBriefcase::PullMergeAndPushInternal";
    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<DgnDbServerRevisionMergeResult>(DgnDbServerRevisionMergeResult::Error(DgnDbServerError::Id::FileNotFound));
        }
    if (!m_repositoryConnection)
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid repository connection.");
        return CreateCompletedAsyncTask<DgnDbServerRevisionMergeResult>(DgnDbServerRevisionMergeResult::Error(DgnDbServerError::Id::InvalidRepositoryConnection));
        }
    if (m_db->IsReadonly())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Briefcase is read only.");
        return CreateCompletedAsyncTask<DgnDbServerRevisionMergeResult>(DgnDbServerRevisionMergeResult::Error(DgnDbServerError::Id::BriefcaseIsReadOnly));
        }
    std::shared_ptr<DgnDbServerRevisionMergeResult> finalResult = std::make_shared<DgnDbServerRevisionMergeResult>();
    return PullAndMerge(downloadCallback, cancellationToken)->Then([=] (DgnDbServerRevisionMergeResultCR result)
        {
        if (!result.IsSuccess())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            finalResult->SetError(result.GetError());
            return;
            }

        Push(description, relinquishCodesLocks, uploadCallback, cancellationToken)->Then([=] (DgnDbServerStatusResultCR pushResult)
            {
            if (!pushResult.IsSuccess())
                {
                DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, pushResult.GetError().GetMessage().c_str());
                finalResult->SetError(pushResult.GetError());
                }
            else
                {
                finalResult->SetSuccess(result.GetValue());
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
DgnDbServerBoolTaskPtr DgnDbBriefcase::IsBriefcaseUpToDate(ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbBriefcase::IsBriefcaseUpToDate";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<DgnDbServerBoolResult> (DgnDbServerBoolResult::Error(DgnDbServerError::Id::FileNotFound));
        }
    if (!m_repositoryConnection)
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid repository connection.");
        return CreateCompletedAsyncTask<DgnDbServerBoolResult> (DgnDbServerBoolResult::Error(DgnDbServerError::Id::InvalidRepositoryConnection));
        }

    //Needswork: think how to optimize this so that we would not need to download all revisions
    auto lastPulledRevision = GetLastRevisionPulled();
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Getting revisions after revision %s.", lastPulledRevision.c_str());
    return m_repositoryConnection->GetRevisionsAfterId(lastPulledRevision, GetDgnDb().GetDbGuid(), cancellationToken)->Then<DgnDbServerBoolResult> ([=](DgnDbServerRevisionsResultCR result)
        {
        if (result.IsSuccess())
            {
            size_t pendingRevisions = result.GetValue().size();
            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            if (pendingRevisions <= 0)
                DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "No pending revisions. Briefcase is up to date.");
            else
                DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "There are %d pending revision(s). Briefcase is not up to date.", pendingRevisions);

            return DgnDbServerBoolResult::Success(pendingRevisions <= 0); //If there are not pending revisions we are up to date
            }

        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
        return DgnDbServerBoolResult::Error(result.GetError());
        });
    }

/* EventService Methods Start */

//---------------------------------------------------------------------------------------
//@bsimethod                                 Caleb.Shafer	                    06/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbBriefcase::SubscribeToEvents(bvector<DgnDbServerEvent::DgnDbServerEventType>* eventTypes)
    {
    const Utf8String methodName = "DgnDbBriefcase::SubscribeToEvents";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (!m_db.IsValid() || !m_db->IsDbOpen())
        return nullptr;
    if (!m_repositoryConnection)
        return nullptr;
    return m_repositoryConnection->SubscribeToEvents(eventTypes, nullptr);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              06/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr  DgnDbBriefcase::UnsubscribeToEvents()
    {
    const Utf8String methodName = "DgnDbBriefcase::UnsubscribeToEvents";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(DgnDbServerError::Id::FileNotFound));
        }
    if (!m_repositoryConnection)
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid repository connection.");
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(DgnDbServerError::Id::InvalidRepositoryConnection));
        }
    return m_repositoryConnection->UnsubscribeToEvents();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Arvind.Venkateswaran	              06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventTaskPtr  DgnDbBriefcase::GetEvent(bool longPolling, ICancellationTokenPtr cancellationToken)
    {
    const Utf8String methodName = "DgnDbBriefcase::GetEvent";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<DgnDbServerEventResult>(DgnDbServerEventResult::Error(DgnDbServerError::Id::FileNotFound));
        }
    if (!m_repositoryConnection)
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid repository connection.");
        return CreateCompletedAsyncTask<DgnDbServerEventResult>(DgnDbServerEventResult::Error(DgnDbServerError::Id::InvalidRepositoryConnection));
        }

    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    return m_repositoryConnection->GetEvent(longPolling, cancellationToken)->Then<DgnDbServerEventResult>([=](DgnDbServerEventResult result)
        {
        if (!result.IsSuccess())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            return DgnDbServerEventResult::Error(result.GetError());
            }
        DgnDbServerEventPtr currentEvent = result.GetValue();
        if (currentEvent == nullptr)
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_WARNING, methodName, "No events found.");
            return DgnDbServerEventResult::Error(DgnDbServerError::Id::NoEventsFound);
            }
        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "");
        return DgnDbServerEventResult::Success(currentEvent);
        });
    }

/* EventService Methods End */

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
//@bsimethod                                    Algirdas.Mikoliunas             09/2016
//---------------------------------------------------------------------------------------
DgnDbRepositoryConnectionPtr DgnDbBriefcase::GetRepositoryConnectionPtr()
    {
    return m_repositoryConnection;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              01/2016
//---------------------------------------------------------------------------------------
BeBriefcaseId DgnDbBriefcase::GetBriefcaseId() const
    {
    return GetDgnDb().GetBriefcaseId();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              01/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbBriefcase::GetLastRevisionPulled() const
    {
    return GetDgnDb().Revisions().GetParentRevisionId();
    }
