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
AsyncTaskPtr<DgnDbResult> DgnDbBriefcase::PullAndMerge(HttpRequest::ProgressCallbackCR callback, ICancellationTokenPtr cancellationToken)
    {
    BeAssert(DgnDbServerHost::IsInitialized());
    std::shared_ptr<DgnDbServerHost> host = std::make_shared<DgnDbServerHost>();
    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        return CreateCompletedAsyncTask<DgnDbResult>(DgnDbResult::Error(DgnDbServerError::Id::FileNotFound));
        }
    if (!m_repositoryConnection)
        {
        return CreateCompletedAsyncTask<DgnDbResult>(DgnDbResult::Error(DgnDbServerError::Id::InvalidRepositoryConnection));
        }
    if (m_db->IsReadonly())
        {
        return CreateCompletedAsyncTask<DgnDbResult>(DgnDbResult::Error(DgnDbServerError::Id::BriefcaseIsReadOnly));
        }
    if (!m_db->Txns().IsTracking())
        {
        return CreateCompletedAsyncTask<DgnDbResult>(DgnDbResult::Error(DgnDbServerError::Id::TrackingNotEnabled));
        }
    Utf8String lastRevisionId = GetLastRevisionPulled ();
    return m_repositoryConnection->Pull(lastRevisionId, callback, cancellationToken)->Then<DgnDbResult>([=] (const DgnDbServerRevisionsResult& result)
        {
        if (result.IsSuccess())
            {
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
                return DgnDbResult::Success();
            else
                return DgnDbResult::Error(mergeStatus);
            }
        else
            return DgnDbResult::Error(result.GetError());
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbResult> DgnDbBriefcase::PullMergeAndPush(HttpRequest::ProgressCallbackCR downloadCallback,
    HttpRequest::ProgressCallbackCR uploadCallback, ICancellationTokenPtr cancellationToken, int attemptsCount)
    {
    return PullMergeAndPushRepeated(downloadCallback, uploadCallback, cancellationToken, attemptsCount);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                  01/2016
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbResult> DgnDbBriefcase::PullMergeAndPushRepeated(HttpRequest::ProgressCallbackCR downloadCallback,
    HttpRequest::ProgressCallbackCR uploadCallback, ICancellationTokenPtr cancellationToken, int attemptsCount, int attempt, int delay)
    {
    std::shared_ptr<DgnDbResult> finalResult = std::make_shared<DgnDbResult>();
    return PullMergeAndPushInternal(downloadCallback, uploadCallback, cancellationToken)
        ->Then([=] (DgnDbResult& result)
        {
        if (result.IsSuccess())
            {
            finalResult->SetSuccess();
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

        PullMergeAndPushRepeated(downloadCallback, uploadCallback, cancellationToken, attemptsCount, attempt + 1, delay)
            ->Then([=] (const DgnDbResult& result)
            {
            if (result.IsSuccess())
                finalResult->SetSuccess();
            else
                finalResult->SetError(result.GetError());
            });
        })->Then<DgnDbResult>([=]
            {
            return *finalResult;
            });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
AsyncTaskPtr<DgnDbResult> DgnDbBriefcase::PullMergeAndPushInternal(HttpRequest::ProgressCallbackCR downloadCallback,
    HttpRequest::ProgressCallbackCR uploadCallback, ICancellationTokenPtr cancellationToken)
    {
    BeAssert(DgnDbServerHost::IsInitialized());
    std::shared_ptr<DgnDbServerHost> host = std::make_shared<DgnDbServerHost>();
    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        return CreateCompletedAsyncTask<DgnDbResult>(DgnDbResult::Error(DgnDbServerError::Id::FileNotFound));
        }
    if (!m_repositoryConnection)
        {
        return CreateCompletedAsyncTask<DgnDbResult>(DgnDbResult::Error(DgnDbServerError::Id::InvalidRepositoryConnection));
        }
    if (m_db->IsReadonly())
        {
        return CreateCompletedAsyncTask<DgnDbResult>(DgnDbResult::Error(DgnDbServerError::Id::BriefcaseIsReadOnly));
        }
    std::shared_ptr<DgnDbResult> finalResult = std::make_shared<DgnDbResult>();
    return PullAndMerge(downloadCallback, cancellationToken)->Then([=] (const DgnDbResult& result)
        {
        if (result.IsSuccess())
            {
            DgnDbServerHost::Adopt(host);
            DgnRevisionPtr revision = m_db->Revisions ().StartCreateRevision ();
            DgnDbServerHost::Forget (host, false);
            if (!revision.IsValid ())
                {
                finalResult->SetSuccess();
                }
            else
                {
                Utf8String revisionId = revision->GetId();
                BeFileName revisionFile(m_db->GetDbFileName());
                m_repositoryConnection->Push(revision, m_db->GetBriefcaseId (), uploadCallback, cancellationToken)->Then
                    ([=] (const DgnDbResult& pushResult)
                    {
                    if (pushResult.IsSuccess())
                        {
                        DgnDbServerHost::Adopt(host);
                        Dgn::RevisionStatus status = m_db->Revisions().FinishCreateRevision();
                        m_db->SaveChanges();
                        DgnDbServerHost::Forget(host);
                        if (RevisionStatus::Success == status)
                            {
                            finalResult->SetSuccess();
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
                }
            }
        else
            finalResult->SetError(result.GetError());
        })->Then<DgnDbResult>([=] ()
        {
        return *finalResult;
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
Dgn::DgnDbR DgnDbBriefcase::GetDgnDb()
    {
    return *m_db;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             10/2015
//---------------------------------------------------------------------------------------
DgnDbRepositoryConnectionPtr DgnDbBriefcase::GetRepositoryConnection()
    {
    return m_repositoryConnection;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              01/2016
//---------------------------------------------------------------------------------------
BeBriefcaseId DgnDbBriefcase::GetBriefcaseId ()
    {
    return GetDgnDb ().GetBriefcaseId ();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas              01/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbBriefcase::GetLastRevisionPulled ()
    {
    return GetDgnDb ().Revisions ().GetParentRevisionId ();
    }
