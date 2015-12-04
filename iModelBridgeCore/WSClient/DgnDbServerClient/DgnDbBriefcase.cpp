/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbBriefcase.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
AsyncTaskPtr<DgnDbResult> DgnDbBriefcase::PullAndMerge(DgnClientFx::Utils::HttpRequest::ProgressCallbackCR callback, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken)
    {
    BeAssert(DgnDbServerHost::IsInitialized() && Error::NotInitialized);
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
    return m_repositoryConnection->Pull(lastRevisionId, callback, cancellationToken)->Then<DgnDbResult>([=] (const DgnDbRevisionsResult& result)
        {
        if (result.IsSuccess())
            {
            bvector<DgnRevisionPtr> revisions = result.GetValue();
            RevisionStatus mergeStatus = RevisionStatus::Success;
            if (!revisions.empty())
                {
                Dgn::DgnPlatformLib::AdoptHost(DgnDbServerHost::Host());
                mergeStatus = m_db->Revisions().MergeRevisions(revisions);
                Dgn::DgnPlatformLib::ForgetHost();
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
AsyncTaskPtr<DgnDbResult> DgnDbBriefcase::PullMergeAndPush(DgnClientFx::Utils::HttpRequest::ProgressCallbackCR downloadCallback, DgnClientFx::Utils::HttpRequest::ProgressCallbackCR uploadCallback, DgnClientFx::Utils::ICancellationTokenPtr cancellationToken)
    {
    BeAssert(DgnDbServerHost::IsInitialized() && Error::NotInitialized);
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
            Dgn::DgnPlatformLib::AdoptHost(DgnDbServerHost::Host());
            BeAssert(m_db.IsValid());
            if (!m_db->Txns().IsUndoPossible())
                {
                Dgn::DgnPlatformLib::ForgetHost();
                finalResult->SetSuccess();
                }
            else
                {
                DgnRevisionPtr revision = m_db->Revisions().StartCreateRevision();
                Dgn::DgnPlatformLib::ForgetHost();
                Utf8String revisionId = revision->GetId();
                BeFileName revisionFile(m_db->GetDbFileName());
                m_repositoryConnection->Push(revision, m_db->GetBriefcaseId().GetValue(), uploadCallback, cancellationToken)->Then([=] (const DgnDbResult& pushResult)
                    {
                    if (pushResult.IsSuccess())
                        {
                        Dgn::DgnPlatformLib::AdoptHost(DgnDbServerHost::Host());
                        Dgn::RevisionStatus status = m_db->Revisions().FinishCreateRevision();
                        m_db->SaveChanges();
                        Dgn::DgnPlatformLib::ForgetHost();
                        if (RevisionStatus::Success == status)
                            {
                            finalResult->SetSuccess();
                            }
                        else
                            finalResult->SetError(status);
                        }
                    else
                        {
                        Dgn::DgnPlatformLib::AdoptHost(DgnDbServerHost::Host());
                        m_db->Revisions().AbandonCreateRevision();
                        Dgn::DgnPlatformLib::ForgetHost();
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
