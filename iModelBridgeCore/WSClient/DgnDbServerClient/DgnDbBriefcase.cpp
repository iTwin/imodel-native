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
    BeAssert(DgnDbServerHost::IsInitialized() && Error::NotInitialized);
    std::shared_ptr<DgnDbServerHost> host = std::make_shared<DgnDbServerHost>();
    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        return CreateCompletedAsyncTask<DgnDbResult>(DgnDbResult::Error(Error::DbNotFound));
        }
    if (!m_repositoryConnection)
        {
        return CreateCompletedAsyncTask<DgnDbResult>(DgnDbResult::Error(Error::ConnectionNotFound));
        }
    if (m_db->IsReadonly())
        {
        return CreateCompletedAsyncTask<DgnDbResult>(DgnDbResult::Error(Error::DbReadOnly));
        }
    Utf8String lastRevisionId;
    m_db->QueryBriefcaseLocalValue(Db::Local::LastRevision, lastRevisionId);
    return m_repositoryConnection->Pull(lastRevisionId, callback, cancellationToken)->Then<DgnDbResult>([=] (const DgnDbServerRevisionsResult& result)
        {
        if (result.IsSuccess())
            {
            auto serverRevisions = result.GetValue();
            bvector<DgnRevisionPtr> revisions;
            for (auto revision : serverRevisions)
                revisions.push_back(revision->GetRevision());
            RevisionStatus mergeStatus = RevisionStatus::Success;
            if (!revisions.empty())
                {
                DgnDbServerHost::Adopt(host);
                mergeStatus = m_db->Revisions().MergeRevisions(revisions);
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
    HttpRequest::ProgressCallbackCR uploadCallback, ICancellationTokenPtr cancellationToken)
    {
    BeAssert(DgnDbServerHost::IsInitialized() && Error::NotInitialized);
    std::shared_ptr<DgnDbServerHost> host = std::make_shared<DgnDbServerHost>();
    if (!m_db.IsValid() || !m_db->IsDbOpen())
        {
        return CreateCompletedAsyncTask<DgnDbResult>(DgnDbResult::Error(Error::DbNotFound));
        }
    if (!m_repositoryConnection)
        {
        return CreateCompletedAsyncTask<DgnDbResult>(DgnDbResult::Error(Error::ConnectionNotFound));
        }
    if (m_db->IsReadonly())
        {
        return CreateCompletedAsyncTask<DgnDbResult>(DgnDbResult::Error(Error::DbReadOnly));
        }
    std::shared_ptr<DgnDbResult> finalResult = std::make_shared<DgnDbResult>();
    return PullAndMerge(downloadCallback, cancellationToken)->Then([=] (const DgnDbResult& result)
        {
        if (result.IsSuccess())
            {
            DgnDbServerHost::Adopt(host);
            BeAssert(m_db.IsValid());
            if (!m_db->Txns().IsUndoPossible())
                {
                DgnDbServerHost::Forget(host, false);
                finalResult->SetSuccess();
                }
            else
                {
                DgnRevisionPtr revision = m_db->Revisions().StartCreateRevision();
                DgnDbServerHost::Forget(host, false);
                if (revision.IsValid())
                    {
                    Utf8String revisionId = revision->GetId();
                    BeFileName revisionFile(m_db->GetDbFileName());
                    m_repositoryConnection->Push(revision, m_db->GetBriefcaseId().GetValue(), uploadCallback, cancellationToken)->Then
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
    Utf8String lastRevisionId;
    GetDgnDb ().QueryBriefcaseLocalValue (Db::Local::LastRevision, lastRevisionId);

    return lastRevisionId;
    }
