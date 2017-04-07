/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbBriefcase.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/TxnManager.h>
#include <DgnDbServer/Client/DgnDbBriefcase.h>
#include <DgnDbServer/Client/DgnDbRepositoryConnection.h>
#include <DgnPlatform/RevisionManager.h>
#include <DgnDbServer/Client/Logging.h>
#include <thread>
#include <random>
#include <DgnDbServer/Client/DgnDbServerConfiguration.h>
#include "DgnDbServerEventManager.h"
#include <DgnDbServer/Client/DgnDbServerBreakHelper.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbBriefcase::DgnDbBriefcase(Dgn::DgnDbPtr db, DgnDbRepositoryConnectionPtr connection)
    {
    m_db = db;
    m_repositoryConnection = connection;

    if (DgnDbServerConfiguration::GetPreDownloadRevisionsEnabled())
        m_repositoryConnection->SubscribeRevisionsDownload();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2016
//---------------------------------------------------------------------------------------
DgnRevisionsTaskPtr DgnDbBriefcase::Pull(Http::Request::ProgressCallbackCR callback, Tasks::ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbBriefcase::Pull";
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<DgnRevisionsResult>(DgnRevisionsResult::Error({DgnDbServerError::Id::FileNotFound, DgnDbServerErrorLocalizedString(MESSAGE_FileNotOpen)}));
        }
    if (m_repositoryConnection.IsNull())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid repository connection.");
        return CreateCompletedAsyncTask<DgnRevisionsResult>(DgnRevisionsResult::Error(DgnDbServerError::Id::InvalidRepositoryConnection));
        }
    if (m_db->IsReadonly())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Briefcase is read only.");
        return CreateCompletedAsyncTask<DgnRevisionsResult>(DgnRevisionsResult::Error(DgnDbServerError::Id::BriefcaseIsReadOnly));
        }
    if (!m_db->Txns().IsTracking())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Tracking is not enabled.");
        return CreateCompletedAsyncTask<DgnRevisionsResult>(DgnRevisionsResult::Error(DgnDbServerError::Id::TrackingNotEnabled));
        }
    CheckCreatingRevision();

    Utf8String lastRevisionId = GetLastRevisionPulled();
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "%s%s", Utf8String::IsNullOrEmpty(lastRevisionId.c_str()) ? "No revisions pulled yet" : "Downloading revisions after revision ", lastRevisionId.c_str());

    return m_repositoryConnection->DownloadRevisionsAfterId(lastRevisionId, GetDgnDb().GetDbGuid(), callback, cancellationToken)
        ->Then<DgnRevisionsResult>([=] (DgnRevisionsResultCR result)
        {
        if (!result.IsSuccess())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            return DgnRevisionsResult::Error(result.GetError());
            }

        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float) (end - start), "Revisions pulled successfully.");
        return result;
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbBriefcase::Merge(DgnRevisions const& revisions) const
    {
    const Utf8String methodName = "DgnDbBriefcase::Merge";
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error({DgnDbServerError::Id::FileNotFound, DgnDbServerErrorLocalizedString(MESSAGE_FileNotOpen)}));
        }
    if (m_repositoryConnection.IsNull())
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
    CheckCreatingRevision();
    RevisionStatus mergeStatus = RevisionStatus::Success;

    if (!revisions.empty())
        {
        for (auto revision : revisions)
            {
            mergeStatus = m_db->Revisions().MergeRevision(*revision);
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
    // unused - double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error({DgnDbServerError::Id::FileNotFound, DgnDbServerErrorLocalizedString(MESSAGE_FileNotOpen)}));
        }
    if (m_repositoryConnection.IsNull())
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
    CheckCreatingRevision();

#if defined (ENABLE_BIM_CRASH_TESTS)
    DgnDbServerBreakHelper::HitBreakpoint(DgnDbServerBreakpoints::BeforeStartCreateRevision);
#endif
    DgnRevisionPtr revision = nullptr;
    if (m_db->Revisions().IsCreatingRevision())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Taking already creating revision.");
        revision = m_db->Revisions().GetCreatingRevision();
        }
    else
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Starting to create a new revision.");
        revision = m_db->Revisions().StartCreateRevision();
        }
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
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, pushResult.GetError().GetDescription().c_str());
            return DgnDbServerStatusResult::Error(pushResult.GetError());
            }
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnRevisionsTaskPtr DgnDbBriefcase::PullAndMerge(Http::Request::ProgressCallbackCR callback, Tasks::ICancellationTokenPtr cancellationToken) const
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
        return CreateCompletedAsyncTask<DgnRevisionsResult>(DgnRevisionsResult::Error(result.GetError()));
        }

    auto serverRevisions = result.GetValue();
    auto mergeResult = Merge(serverRevisions)->GetResult();

    if (!mergeResult.IsSuccess())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Merge failed.");
        return CreateCompletedAsyncTask<DgnRevisionsResult>(DgnRevisionsResult::Error(mergeResult.GetError()));
        }

    double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, (float) (end - start), "Revisions merged successfully.");
    return CreateCompletedAsyncTask<DgnRevisionsResult>(DgnRevisionsResult::Success(serverRevisions));

    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnRevisionsTaskPtr DgnDbBriefcase::PullMergeAndPush(Utf8CP description, bool relinquishCodesLocks, Http::Request::ProgressCallbackCR downloadCallback,
                                                                 Http::Request::ProgressCallbackCR uploadCallback, ICancellationTokenPtr cancellationToken, int attemptsCount)
    {
    const Utf8String methodName = "DgnDbBriefcase::PullMergeAndPush";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    return PullMergeAndPushRepeated(description, relinquishCodesLocks, downloadCallback, uploadCallback, cancellationToken, attemptsCount);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
void DgnDbBriefcase::WaitForRevisionEvent() const
    {
    const Utf8String methodName = "DgnDbBriefcase::WaitForStart";
    int iterationsLeft = 100;
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Starting to wait.");

    while (iterationsLeft > 0)
        {
        if (DgnDbServerEvent::DgnDbServerEventType::RevisionEvent == m_lastPullMergeAndPushEvent)
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Got merge finished event.");
            break;
            }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        iterationsLeft--;
        }

    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Finishing wait.");
    }
    
//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
void DgnDbBriefcase::SubscribeForRevisionEvents()
    {
    const Utf8String methodName = "DgnDbBriefcase::SubscribeForPullMergeAndPushEvents";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    DgnDbServerEventTypeSet eventTypes;
    eventTypes.insert(DgnDbServerEvent::DgnDbServerEventType::RevisionEvent);
    eventTypes.insert(DgnDbServerEvent::DgnDbServerEventType::RevisionCreateEvent);

    m_pullMergeAndPushCallback = std::make_shared<DgnDbServerEventCallback>([=](DgnDbServerEventPtr event)
        {
        auto eventType = event->GetEventType();
        if (DgnDbServerEvent::DgnDbServerEventType::RevisionCreateEvent == eventType ||
            DgnDbServerEvent::DgnDbServerEventType::RevisionEvent == eventType)
            {
            m_lastPullMergeAndPushEvent = eventType;
            }
        });
    auto subscribeResult = SubscribeEventsCallback(&eventTypes, m_pullMergeAndPushCallback);
    m_eventsAvailable = subscribeResult->GetResult().IsSuccess();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
void DgnDbBriefcase::UnsubscribeRevisionEvents()
    {
    const Utf8String methodName = "DgnDbBriefcase::UnsubscribeRevisionEvents";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    if (m_pullMergeAndPushCallback)
        UnsubscribeEventsCallback(m_pullMergeAndPushCallback);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                  01/2016
//---------------------------------------------------------------------------------------
DgnRevisionsTaskPtr DgnDbBriefcase::PullMergeAndPushRepeated(Utf8CP description, bool relinquishCodesLocks, Http::Request::ProgressCallbackCR downloadCallback, Http::Request::ProgressCallbackCR uploadCallback,
                                                                     ICancellationTokenPtr cancellationToken, int attemptsCount, int attempt, int delay)
    {
    const Utf8String methodName = "DgnDbBriefcase::PullMergeAndPushRepeated";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Attempt %d/%d.", attempt, attemptsCount);
    auto result = PullMergeAndPushInternal(description, relinquishCodesLocks, downloadCallback, uploadCallback, cancellationToken)->GetResult();

    if (result.IsSuccess())
        {
        UnsubscribeRevisionEvents();
        return CreateCompletedAsyncTask<DgnRevisionsResult>(DgnRevisionsResult::Success(result.GetValue()));
        }

    if (attempt >= attemptsCount)
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Too many unsuccessful attempts.");
        UnsubscribeRevisionEvents();
        return CreateCompletedAsyncTask<DgnRevisionsResult>(DgnRevisionsResult::Error(result.GetError()));
        }

    DgnDbServerError::Id errorId = result.GetError().GetId();
    switch (errorId)
        {
        case DgnDbServerError::Id::AnotherUserPushing:
        case DgnDbServerError::Id::PullIsRequired:
        case DgnDbServerError::Id::DatabaseTemporarilyLocked:
        case DgnDbServerError::Id::BIMCSOperationFailed:
            break;
        default:
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            UnsubscribeRevisionEvents();
            return CreateCompletedAsyncTask<DgnRevisionsResult>(DgnRevisionsResult::Error(result.GetError()));
            }
        }

    if (attempt == 1)
        {
        SubscribeForRevisionEvents();
        srand(time(0) / GetBriefcaseId().GetValue());
        }

    if (m_eventsAvailable)
        WaitForRevisionEvent();
    else
        {
        int sleepTime = rand() % 5000;
        BeThreadUtilities::BeSleep(sleepTime);
        }

    m_lastPullMergeAndPushEvent = DgnDbServerEvent::DgnDbServerEventType::UnknownEventType;
    return PullMergeAndPushRepeated(description, relinquishCodesLocks, downloadCallback, uploadCallback, cancellationToken, attemptsCount, attempt + 1, 0);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas            12/2015
//---------------------------------------------------------------------------------------
void DgnDbBriefcase::CheckCreatingRevision() const
    {
    if (m_db->Revisions().IsCreatingRevision())
        {
        auto creatingRevisionId = m_db->Revisions().GetCreatingRevision()->GetId();
        if (Utf8String::IsNullOrEmpty(creatingRevisionId.c_str()))
            return;

        auto creatingRevisionResult = m_repositoryConnection->GetRevisionById(creatingRevisionId)->GetResult();
        if (creatingRevisionResult.IsSuccess())
            {
            m_db->Revisions().FinishCreateRevision();
            }
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnRevisionsTaskPtr DgnDbBriefcase::PullMergeAndPushInternal(Utf8CP description, bool relinquishCodesLocks, Http::Request::ProgressCallbackCR downloadCallback,
    Http::Request::ProgressCallbackCR uploadCallback, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "DgnDbBriefcase::PullMergeAndPushInternal";
    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<DgnRevisionsResult>(DgnRevisionsResult::Error({DgnDbServerError::Id::FileNotFound, DgnDbServerErrorLocalizedString(MESSAGE_FileNotOpen)}));
        }
    if (m_repositoryConnection.IsNull())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid repository connection.");
        return CreateCompletedAsyncTask<DgnRevisionsResult>(DgnRevisionsResult::Error(DgnDbServerError::Id::InvalidRepositoryConnection));
        }
    if (m_db->IsReadonly())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Briefcase is read only.");
        return CreateCompletedAsyncTask<DgnRevisionsResult>(DgnRevisionsResult::Error(DgnDbServerError::Id::BriefcaseIsReadOnly));
        }
    std::shared_ptr<DgnRevisionsResult> finalResult = std::make_shared<DgnRevisionsResult>();
    return PullAndMerge(downloadCallback, cancellationToken)->Then([=] (DgnRevisionsResultCR result)
        {
        if (!result.IsSuccess())
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            finalResult->SetError(result.GetError());
            return;
            }
        
        // This sleep waits for events from other clients who just started a push
        int sleepTime = rand() % 200;
        DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, sleepTime, "Sleeping.");
        BeThreadUtilities::BeSleep(sleepTime);

        if (m_eventsAvailable && DgnDbServerEvent::DgnDbServerEventType::UnknownEventType != m_lastPullMergeAndPushEvent)
            {
            DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Other user pushed. Waiting again.");
            finalResult->SetError(DgnDbServerError::Id::PullIsRequired);
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

        })->Then<DgnRevisionsResult>([=] ()
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
        return CreateCompletedAsyncTask<DgnDbServerBoolResult> (DgnDbServerBoolResult::Error({DgnDbServerError::Id::FileNotFound, DgnDbServerErrorLocalizedString(MESSAGE_FileNotOpen)}));
        }
    if (m_repositoryConnection.IsNull())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid repository connection.");
        return CreateCompletedAsyncTask<DgnDbServerBoolResult> (DgnDbServerBoolResult::Error(DgnDbServerError::Id::InvalidRepositoryConnection));
        }

    //Needswork: think how to optimize this so that we would not need to download all revisions
    auto lastPulledRevision = GetLastRevisionPulled();
    DgnDbServerLogHelper::Log(SEVERITY::LOG_INFO, methodName, "Getting revisions after revision %s.", lastPulledRevision.c_str());
    return m_repositoryConnection->GetRevisionsAfterId(lastPulledRevision, GetDgnDb().GetDbGuid(), cancellationToken)->Then<DgnDbServerBoolResult> ([=](DgnDbServerRevisionsInfoResultCR result)
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
//@bsimethod                                     Algirdas.Mikoliunas            12/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbBriefcase::SubscribeEventsCallback(DgnDbServerEventTypeSet* eventTypes, DgnDbServerEventCallbackPtr callback) const
    {
    const Utf8String methodName = "DgnDbBriefcase::SubscribeEventsCallback";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (!m_db.IsValid() || !m_db->IsDbOpen())
        return nullptr;
    if (m_repositoryConnection.IsNull())
        return nullptr;

    return m_repositoryConnection->SubscribeEventsCallback(eventTypes, callback);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas            12/2016
//---------------------------------------------------------------------------------------
DgnDbServerStatusTaskPtr DgnDbBriefcase::UnsubscribeEventsCallback(DgnDbServerEventCallbackPtr callback) const
    {
    const Utf8String methodName = "DgnDbBriefcase::UnsubscribeEventsCallback";
    DgnDbServerLogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error({DgnDbServerError::Id::FileNotFound, DgnDbServerErrorLocalizedString(MESSAGE_FileNotOpen)}));
        }
    if (m_repositoryConnection.IsNull())
        {
        DgnDbServerLogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Invalid repository connection.");
        return CreateCompletedAsyncTask<DgnDbServerStatusResult>(DgnDbServerStatusResult::Error(DgnDbServerError::Id::InvalidRepositoryConnection));
        }

    return m_repositoryConnection->UnsubscribeEventsCallback(callback);
    }

/* EventService Methods End */
