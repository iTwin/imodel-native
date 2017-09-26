/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/Briefcase.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/TxnManager.h>
#include <WebServices/iModelHub/Client/Briefcase.h>
#include <WebServices/iModelHub/Client/iModelConnection.h>
#include <DgnPlatform/RevisionManager.h>
#include "Logging.h"
#include <thread>
#include <random>
#include "Events/EventManager.h"
#include <WebServices/iModelHub/Client/BreakHelper.h>
#include "Utils.h"

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
Briefcase::Briefcase(Dgn::DgnDbPtr db, iModelConnectionPtr connection)
    {
    m_db = db;
    m_imodelConnection = connection;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2016
//---------------------------------------------------------------------------------------
ChangeSetsTaskPtr Briefcase::Pull(Http::Request::ProgressCallbackCR callback, Tasks::ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Briefcase::Pull";
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<ChangeSetsResult>(ChangeSetsResult::Error({Error::Id::FileNotFound, ErrorLocalizedString(MESSAGE_FileNotOpen)}));
        }
    if (m_imodelConnection.IsNull())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Invalid iModel connection.");
        return CreateCompletedAsyncTask<ChangeSetsResult>(ChangeSetsResult::Error(Error::Id::InvalidiModelConnection));
        }
    if (m_db->IsReadonly())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Briefcase is read only.");
        return CreateCompletedAsyncTask<ChangeSetsResult>(ChangeSetsResult::Error(Error::Id::BriefcaseIsReadOnly));
        }
    if (!m_db->Txns().IsTracking())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Tracking is not enabled.");
        return CreateCompletedAsyncTask<ChangeSetsResult>(ChangeSetsResult::Error(Error::Id::TrackingNotEnabled));
        }
    CheckCreatingChangeSet();

    Utf8String lastChangeSetId = GetLastChangeSetPulled();
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "%s%s", Utf8String::IsNullOrEmpty(lastChangeSetId.c_str()) ? "No changeSets pulled yet" : "Downloading changeSets after changeSet ", lastChangeSetId.c_str());

    return m_imodelConnection->DownloadChangeSetsAfterId(lastChangeSetId, GetDgnDb().GetDbGuid(), callback, cancellationToken)
        ->Then<ChangeSetsResult>([=] (ChangeSetsResultCR result)
        {
        if (!result.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, result.GetError().GetMessage().c_str());
            return ChangeSetsResult::Error(result.GetError());
            }

        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float) (end - start), "ChangeSets pulled successfully.");
        return result;
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr Briefcase::Merge(ChangeSets const& changeSets) const
    {
    const Utf8String methodName = "Briefcase::Merge";
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error({Error::Id::FileNotFound, ErrorLocalizedString(MESSAGE_FileNotOpen)}));
        }
    if (m_imodelConnection.IsNull())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Invalid iModel connection.");
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(Error::Id::InvalidiModelConnection));
        }
    if (m_db->IsReadonly())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Briefcase is read only.");
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(Error::Id::BriefcaseIsReadOnly));
        }
    if (!m_db->Txns().IsTracking())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Tracking is not enabled.");
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(Error::Id::TrackingNotEnabled));
        }
    CheckCreatingChangeSet();

    RevisionStatus mergeStatus = AddRemoveChangeSetsFromDgnDb(changeSets);

    if (RevisionStatus::Success == mergeStatus)
        {
        double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float) (end - start), "ChangeSets merged successfully.");
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Success());
        }

    LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Merge failed.");
    return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(mergeStatus));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             11/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr Briefcase::Push(Utf8CP description, bool relinquishCodesLocks, Http::Request::ProgressCallbackCR uploadCallback, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Briefcase::Push";
    // unused - double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error({Error::Id::FileNotFound, ErrorLocalizedString(MESSAGE_FileNotOpen)}));
        }
    if (m_imodelConnection.IsNull())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Invalid iModel connection.");
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(Error::Id::InvalidiModelConnection));
        }
    if (m_db->IsReadonly())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Briefcase is read only.");
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(Error::Id::BriefcaseIsReadOnly));
        }
    if (!m_db->Txns().IsTracking())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Tracking is not enabled.");
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(Error::Id::TrackingNotEnabled));
        }
    CheckCreatingChangeSet();

#if defined (ENABLE_BIM_CRASH_TESTS)
    BreakHelper::HitBreakpoint(Breakpoints::BeforeStartCreateChangeSet);
#endif
    DgnRevisionPtr changeSet = nullptr;
    if (m_db->Revisions().IsCreatingRevision())
        {
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Taking already creating changeSet.");
        changeSet = m_db->Revisions().GetCreatingRevision();
        }
    else
        {
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Starting to create a new changeSet.");
        changeSet = m_db->Revisions().StartCreateRevision();
        }
#if defined (ENABLE_BIM_CRASH_TESTS)
    BreakHelper::HitBreakpoint(Breakpoints::AfterStartCreateChangeSet);
#endif

    if (!changeSet.IsValid())
        {
        // No changes. Return success.
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, "No changes found.");
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Success());
        }

    changeSet->SetSummary(description);

    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Created changeSet with ID %s.", changeSet->GetId().c_str());
    Utf8String changeSetId = changeSet->GetId();
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Starting push.");

#if defined (ENABLE_BIM_CRASH_TESTS)
    BreakHelper::HitBreakpoint(Breakpoints::BeforePushChangeSetToServer);
#endif
    return m_imodelConnection->Push(changeSet, *m_db, relinquishCodesLocks, uploadCallback, cancellationToken)
        ->Then<StatusResult>([=] (StatusResultCR pushResult)
        {
#if defined (ENABLE_BIM_CRASH_TESTS)
        BreakHelper::HitBreakpoint(Breakpoints::AfterPushChangeSetToServer);
#endif

        if (pushResult.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Push successful. Finishing creating new ChangeSet.");
            Dgn::RevisionStatus status = m_db->Revisions().FinishCreateRevision();

#if defined (ENABLE_BIM_CRASH_TESTS)
            BreakHelper::HitBreakpoint(Breakpoints::AfterFinishCreateChangeSet);
#endif
            m_db->SaveChanges();
            if (RevisionStatus::Success == status)
                {
                LogHelper::Log(SEVERITY::LOG_INFO, methodName, "New ChangeSet created successfully.");
                return StatusResult::Success();
                }
            else
                {
                LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "An error occured while trying to finish creating a changeSet.");
                return StatusResult::Error(status);
                }
            }
        else
            {
            m_db->Revisions().AbandonCreateRevision();
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, pushResult.GetError().GetMessage().c_str());
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, pushResult.GetError().GetDescription().c_str());
            return StatusResult::Error(pushResult.GetError());
            }
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
ChangeSetsTaskPtr Briefcase::PullAndMerge(Http::Request::ProgressCallbackCR callback, Tasks::ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Briefcase::PullAndMerge";
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    auto result = Pull(callback, cancellationToken)->GetResult();

#if defined (ENABLE_BIM_CRASH_TESTS)
        BreakHelper::HitBreakpoint(Breakpoints::AfterDownloadChangeSets);
#endif
    if (!result.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Pull failed.");
        return CreateCompletedAsyncTask<ChangeSetsResult>(ChangeSetsResult::Error(result.GetError()));
        }

    auto pulledChangeSets = result.GetValue();
    auto mergeResult = Merge(pulledChangeSets)->GetResult();

    if (!mergeResult.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Merge failed.");
        return CreateCompletedAsyncTask<ChangeSetsResult>(ChangeSetsResult::Error(mergeResult.GetError()));
        }

    double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float) (end - start), "ChangeSets merged successfully.");
    return CreateCompletedAsyncTask<ChangeSetsResult>(ChangeSetsResult::Success(pulledChangeSets));

    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
ChangeSetsTaskPtr Briefcase::PullMergeAndPush(Utf8CP description, bool relinquishCodesLocks, Http::Request::ProgressCallbackCR downloadCallback,
                                                                 Http::Request::ProgressCallbackCR uploadCallback, ICancellationTokenPtr cancellationToken, int attemptsCount)
    {
    const Utf8String methodName = "Briefcase::PullMergeAndPush";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    return PullMergeAndPushRepeated(description, relinquishCodesLocks, downloadCallback, uploadCallback, cancellationToken, attemptsCount);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
void Briefcase::WaitForChangeSetEvent() const
    {
    const Utf8String methodName = "Briefcase::WaitForChangeSetEvent";
    int iterationsLeft = 100;
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Starting to wait.");

    while (iterationsLeft > 0)
        {
        if (Event::EventType::ChangeSetPostPushEvent == m_lastPullMergeAndPushEvent)
            {
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Got merge finished event.");
            break;
            }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        iterationsLeft--;
        }

    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Finishing wait.");
    }
    
//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             12/2016
//---------------------------------------------------------------------------------------
void Briefcase::SubscribeForChangeSetEvents()
    {
    const Utf8String methodName = "Briefcase::SubscribeForChangeSetEvents";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    EventTypeSet eventTypes;
    eventTypes.insert(Event::EventType::ChangeSetPostPushEvent);
    eventTypes.insert(Event::EventType::ChangeSetPrePushEvent);

    m_pullMergeAndPushCallback = std::make_shared<EventCallback>([=](EventPtr event)
        {
        auto eventType = event->GetEventType();
        if (Event::EventType::ChangeSetPrePushEvent == eventType ||
            Event::EventType::ChangeSetPostPushEvent == eventType)
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
void Briefcase::UnsubscribeChangeSetEvents()
    {
    const Utf8String methodName = "Briefcase::UnsubscribeChangeSetEvents";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    if (m_pullMergeAndPushCallback)
        UnsubscribeEventsCallback(m_pullMergeAndPushCallback);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                  01/2016
//---------------------------------------------------------------------------------------
ChangeSetsTaskPtr Briefcase::PullMergeAndPushRepeated(Utf8CP description, bool relinquishCodesLocks, Http::Request::ProgressCallbackCR downloadCallback, Http::Request::ProgressCallbackCR uploadCallback,
                                                                     ICancellationTokenPtr cancellationToken, int attemptsCount, int attempt, int delay)
    {
    const Utf8String methodName = "Briefcase::PullMergeAndPushRepeated";
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Attempt %d/%d.", attempt, attemptsCount);
    auto result = PullMergeAndPushInternal(description, relinquishCodesLocks, downloadCallback, uploadCallback, cancellationToken)->GetResult();

    if (result.IsSuccess())
        {
        UnsubscribeChangeSetEvents();
        return CreateCompletedAsyncTask<ChangeSetsResult>(ChangeSetsResult::Success(result.GetValue()));
        }

    if (attempt >= attemptsCount)
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Too many unsuccessful attempts.");
        UnsubscribeChangeSetEvents();
        return CreateCompletedAsyncTask<ChangeSetsResult>(ChangeSetsResult::Error(result.GetError()));
        }

    Error::Id errorId = result.GetError().GetId();
    switch (errorId)
        {
        case Error::Id::AnotherUserPushing:
        case Error::Id::PullIsRequired:
        case Error::Id::DatabaseTemporarilyLocked:
        case Error::Id::iModelHubOperationFailed:
            break;
        default:
            {
            LogHelper::Log(SEVERITY::LOG_WARNING, methodName, result.GetError().GetMessage().c_str());
            UnsubscribeChangeSetEvents();
            return CreateCompletedAsyncTask<ChangeSetsResult>(ChangeSetsResult::Error(result.GetError()));
            }
        }

    if (attempt == 1)
        {
        SubscribeForChangeSetEvents();
        srand(time(0) / GetBriefcaseId().GetValue());
        }

    if (m_eventsAvailable)
        WaitForChangeSetEvent();
    else
        {
        int sleepTime = rand() % 5000;
        BeThreadUtilities::BeSleep(sleepTime);
        }

    m_lastPullMergeAndPushEvent = Event::EventType::UnknownEventType;
    return PullMergeAndPushRepeated(description, relinquishCodesLocks, downloadCallback, uploadCallback, cancellationToken, attemptsCount, attempt + 1, 0);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas            12/2015
//---------------------------------------------------------------------------------------
void Briefcase::CheckCreatingChangeSet() const
    {
    if (m_db->Revisions().IsCreatingRevision())
        {
        auto creatingChangeSetId = m_db->Revisions().GetCreatingRevision()->GetId();
        if (Utf8String::IsNullOrEmpty(creatingChangeSetId.c_str()))
            return;

        auto creatingChangeSetResult = m_imodelConnection->GetChangeSetById(creatingChangeSetId)->GetResult();
        if (creatingChangeSetResult.IsSuccess())
            {
            m_db->Revisions().FinishCreateRevision();
            }
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
ChangeSetsTaskPtr Briefcase::PullMergeAndPushInternal(Utf8CP description, bool relinquishCodesLocks, Http::Request::ProgressCallbackCR downloadCallback,
    Http::Request::ProgressCallbackCR uploadCallback, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Briefcase::PullMergeAndPushInternal";
    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<ChangeSetsResult>(ChangeSetsResult::Error({Error::Id::FileNotFound, ErrorLocalizedString(MESSAGE_FileNotOpen)}));
        }
    if (m_imodelConnection.IsNull())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Invalid iModel connection.");
        return CreateCompletedAsyncTask<ChangeSetsResult>(ChangeSetsResult::Error(Error::Id::InvalidiModelConnection));
        }
    if (m_db->IsReadonly())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Briefcase is read only.");
        return CreateCompletedAsyncTask<ChangeSetsResult>(ChangeSetsResult::Error(Error::Id::BriefcaseIsReadOnly));
        }
    std::shared_ptr<ChangeSetsResult> finalResult = std::make_shared<ChangeSetsResult>();
    return PullAndMerge(downloadCallback, cancellationToken)->Then([=] (ChangeSetsResultCR result)
        {
        if (!result.IsSuccess())
            {
            LogHelper::Log(SEVERITY::LOG_ERROR, methodName, result.GetError().GetMessage().c_str());
            finalResult->SetError(result.GetError());
            return;
            }
        
        // This sleep waits for events from other clients who just started a push
        int sleepTime = rand() % 200;
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, sleepTime, "Sleeping.");
        BeThreadUtilities::BeSleep(sleepTime);

        if (m_eventsAvailable && Event::EventType::UnknownEventType != m_lastPullMergeAndPushEvent)
            {
            LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Other user pushed. Waiting again.");
            finalResult->SetError(Error::Id::PullIsRequired);
            return;
            }
        
        Push(description, relinquishCodesLocks, uploadCallback, cancellationToken)->Then([=] (StatusResultCR pushResult)
            {
            if (!pushResult.IsSuccess())
                {
                LogHelper::Log(SEVERITY::LOG_WARNING, methodName, pushResult.GetError().GetMessage().c_str());
                finalResult->SetError(pushResult.GetError());
                }
            else
                {
                finalResult->SetSuccess(result.GetValue());
                }
            });

        })->Then<ChangeSetsResult>([=] ()
        {
        return *finalResult;
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              03/2016
//---------------------------------------------------------------------------------------
BoolTaskPtr Briefcase::IsBriefcaseUpToDate(ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Briefcase::IsBriefcaseUpToDate";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    double start = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<BoolResult> (BoolResult::Error({Error::Id::FileNotFound, ErrorLocalizedString(MESSAGE_FileNotOpen)}));
        }
    if (m_imodelConnection.IsNull())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Invalid iModel connection.");
        return CreateCompletedAsyncTask<BoolResult> (BoolResult::Error(Error::Id::InvalidiModelConnection));
        }

    //Needswork: think how to optimize this so that we would not need to download all changeSets
    auto lastPulledChangeSet = GetLastChangeSetPulled();
    LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Getting changeSets after changeSet %s.", lastPulledChangeSet.c_str());
    return m_imodelConnection->GetChangeSetsAfterId(lastPulledChangeSet, GetDgnDb().GetDbGuid(), cancellationToken)->Then<BoolResult> ([=](ChangeSetsInfoResultCR result)
        {
        if (result.IsSuccess())
            {
            size_t pendingChangeSets = result.GetValue().size();
            double end = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
            if (pendingChangeSets <= 0)
                LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "No pending changeSets. Briefcase is up to date.");
            else
                LogHelper::Log(SEVERITY::LOG_INFO, methodName, (float)(end - start), "There are %d pending changeSet(s). Briefcase is not up to date.", pendingChangeSets);

            return BoolResult::Success(pendingChangeSets <= 0); //If there are not pending changeSets we are up to date
            }

        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, result.GetError().GetMessage().c_str());
        return BoolResult::Error(result.GetError());
        });
    }

/* EventService Methods Start */

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas            12/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr Briefcase::SubscribeEventsCallback(EventTypeSet* eventTypes, EventCallbackPtr callback) const
    {
    const Utf8String methodName = "Briefcase::SubscribeEventsCallback";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (!m_db.IsValid() || !m_db->IsDbOpen())
        return nullptr;
    if (m_imodelConnection.IsNull())
        return nullptr;

    return m_imodelConnection->SubscribeEventsCallback(eventTypes, callback);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas            12/2016
//---------------------------------------------------------------------------------------
StatusTaskPtr Briefcase::UnsubscribeEventsCallback(EventCallbackPtr callback) const
    {
    const Utf8String methodName = "Briefcase::UnsubscribeEventsCallback";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");
    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error({Error::Id::FileNotFound, ErrorLocalizedString(MESSAGE_FileNotOpen)}));
        }
    if (m_imodelConnection.IsNull())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, "Invalid iModel connection.");
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(Error::Id::InvalidiModelConnection));
        }

    return m_imodelConnection->UnsubscribeEventsCallback(callback);
    }

/* EventService Methods End */

//---------------------------------------------------------------------------------------
//@bsimethod                                   Viktorija.Adomauskaite             10/2015
//---------------------------------------------------------------------------------------
StatusTaskPtr Briefcase::UpdateToVersion(Utf8String versionId, Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Briefcase::UpdateToVersion";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(Error::Id::FileNotFound));
        }

    auto versionManager = m_imodelConnection->GetVersionsManager();

    ChangeSetsInfoResult changeSetResult = versionManager.GetChangeSetsBetweenVersionAndChangeSet(versionId, GetLastChangeSetPulled(), m_db->GetDbGuid(), cancellationToken)->GetResult();

    if (!changeSetResult.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, changeSetResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(changeSetResult.GetError()));
        }
    auto changeSetInfos = changeSetResult.GetValue();

    auto changeSetsResult = m_imodelConnection->DownloadChangeSetsInternal(changeSetInfos, callback, cancellationToken)->GetResult();
    if (!changeSetsResult.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, changeSetsResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(changeSetsResult.GetError()));
        }
    auto changeSets = changeSetsResult.GetValue();

    RevisionStatus mergeStatus = AddRemoveChangeSetsFromDgnDb(changeSets);

    if (RevisionStatus::Success == mergeStatus)
        {
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Success.");
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Success());
        }

    LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Merge failed.");
    return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(mergeStatus));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Viktorija.Adomauskaite             10/2015
//---------------------------------------------------------------------------------------
StatusTaskPtr Briefcase::UpdateToChangeSet(Utf8String changeSetId, Http::Request::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken) const
    {
    const Utf8String methodName = "Briefcase::UpdateToChangeSet";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "File not found.");
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(Error::Id::FileNotFound));
        }

    ChangeSetsInfoResult changeSetResult;
    changeSetResult = m_imodelConnection->GetChangeSetsBetween(changeSetId, GetLastChangeSetPulled(), m_db->GetDbGuid(), cancellationToken)->GetResult();

    if (!changeSetResult.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, changeSetResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(changeSetResult.GetError()));
        }
    auto changeSetInfos = changeSetResult.GetValue();

    auto changeSetsResult = m_imodelConnection->DownloadChangeSetsInternal(changeSetInfos, callback, cancellationToken)->GetResult();
    if (!changeSetsResult.IsSuccess())
        {
        LogHelper::Log(SEVERITY::LOG_WARNING, methodName, changeSetsResult.GetError().GetMessage().c_str());
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(changeSetsResult.GetError()));
        }
    auto changeSets = changeSetsResult.GetValue();

    RevisionStatus mergeStatus = AddRemoveChangeSetsFromDgnDb(changeSets);

    if (RevisionStatus::Success == mergeStatus)
        {
        LogHelper::Log(SEVERITY::LOG_INFO, methodName, "Success.");
        return CreateCompletedAsyncTask<StatusResult>(StatusResult::Success());
        }

    LogHelper::Log(SEVERITY::LOG_ERROR, methodName, "Merge failed.");
    return CreateCompletedAsyncTask<StatusResult>(StatusResult::Error(mergeStatus));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Viktorija.Adomauskaite             10/2015
//---------------------------------------------------------------------------------------
RevisionStatus Briefcase::AddRemoveChangeSetsFromDgnDb(ChangeSets changeSets) const
    {
    /*
    So, there are four main use cases:
    1. All ChangeSets should be merged(added)/ChangeSets are new
       In this case reinstation step is skipped and merge step is done
    2. All ChangeSets should be reversed(removed)/ChangeSets are already merged
       In this case reverse step is done
    3. All ChangeSets should be reinstated(added)/ChangeSets has been reversed before
       In this case reinstation step is done and merge step is skipped
    4. Some ChangeSets should be reinstated and some merged(added)/First part of ChangeSets has been reversed and others are new ChangeSets
       In this case reinstations step and then merge step are done.
    */
    const Utf8String methodName = "Briefcase::AddRemoveChangeSetsFromDgnDb";
    LogHelper::Log(SEVERITY::LOG_DEBUG, methodName, "Method called.");

    if (changeSets.size() <= 0)
        return RevisionStatus::Success;

    Utf8String parentChangeSetId = GetLastChangeSetPulled();

    RevisionStatus mergeStatus = RevisionStatus::Success;
    if (Utf8String::IsNullOrEmpty(parentChangeSetId.c_str()) ||
        changeSets.at(0)->GetParentId() == parentChangeSetId)
        {
        auto changeSetIterator = changeSets.begin();
        //reinstation step
        while (changeSetIterator != changeSets.end() && m_db->Revisions().HasReversedRevisions())
            {
            mergeStatus = m_db->Revisions().ReinstateRevision(**changeSetIterator);
            changeSetIterator++;

            if (mergeStatus != RevisionStatus::Success)
                return mergeStatus;
            }

        if (changeSetIterator != changeSets.begin())
            changeSets.erase(changeSets.begin(), changeSetIterator--);

        //merge step
        mergeStatus = ValidateChangeSets(changeSets, *m_db);
        if (mergeStatus != RevisionStatus::Success)
            return mergeStatus;

        for (auto changeSet : changeSets)
            {
            mergeStatus = m_db->Revisions().MergeRevision(*changeSet);

            if (mergeStatus != RevisionStatus::Success)
                break;
            }
        }
    else
        {
        //reverse step
        std::reverse(changeSets.begin(), changeSets.end());

        for (auto changeSet : changeSets)
            {
            mergeStatus = m_db->Revisions().ReverseRevision(*changeSet);
            if (mergeStatus != RevisionStatus::Success)
                break;
            }
        }

    return mergeStatus;
    }
