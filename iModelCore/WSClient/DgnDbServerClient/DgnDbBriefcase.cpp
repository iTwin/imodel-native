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

USING_NAMESPACE_BENTLEY_DGNDBSERVER
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS
USING_NAMESPACE_BENTLEY_SQLITE

#define DGNDBSERVER_LOCAL_LAST_REVISION "dgndbserver_lastRevisionId"

DgnDbBriefcase::DgnDbBriefcase(Dgn::DgnDbPtr db, DgnDbRepositoryConnectionPtr connection)
    {
    m_db = db;
    m_repositoryConnection = connection;
    m_briefcaseId = db->GetBriefcaseId();
    db->QueryBriefcaseLocalValue(DGNDBSERVER_LOCAL_LAST_REVISION, m_lastRevisionId);
    }

DgnDbBriefcasePtr DgnDbBriefcase::Create(Dgn::DgnDbPtr db, DgnDbRepositoryConnectionPtr connection)
    {
    return DgnDbBriefcasePtr(new DgnDbBriefcase(db, connection));
    }

AsyncTaskPtr<DgnDbResult> DgnDbBriefcase::Sync(MobileDgn::Utils::HttpRequest::ProgressCallbackCR callback, MobileDgn::Utils::ICancellationTokenPtr cancellationToken)
    {
    return m_repositoryConnection->Pull(m_lastRevisionId, callback, cancellationToken)->Then<DgnDbResult>([=] (const DgnDbRevisionsResult& result)
        {
        if (result.IsSuccess())
            {
            BentleyStatus status = m_db->Revisions().MergeRevisions(result.GetValue());
            if (BentleyStatus::SUCCESS == status)
                return DgnDbResult::Success();
            else
                return DgnDbResult::Error(status);
            }
        else
            return DgnDbResult::Error(result.GetError());
        });
    }

AsyncTaskPtr<DgnDbResult> DgnDbBriefcase::SyncAndPush(MobileDgn::Utils::HttpRequest::ProgressCallbackCR callback, MobileDgn::Utils::ICancellationTokenPtr cancellationToken)
    {
    std::shared_ptr<DgnDbResult> finalResult = std::shared_ptr<DgnDbResult>();
    return Sync(callback, cancellationToken)->Then([=] (const DgnDbResult& result)
        {
        if (result.IsSuccess())
            {
            DgnRevisionPtr revision = m_db->Revisions().StartCreateRevision();
            BeFileName revisionFile(m_db->GetDbFileName());
            m_repositoryConnection->Push(revision, callback, cancellationToken)->Then([=] (const DgnDbResult& pushResult)
                {
                if (pushResult.IsSuccess())
                    {
                    BentleyStatus status = m_db->Revisions().FinishCreateRevision();
                    if (BentleyStatus::SUCCESS == status)
                        finalResult->SetSuccess();
                    else
                        finalResult->SetError(status);
                    }
                else
                    {
                    m_db->Revisions().AbandonCreateRevision();
                    finalResult->SetError(pushResult.GetError());
                    }
                });
            }
        else
            finalResult->SetError(result.GetError());
        })->Then<DgnDbResult>([=] ()
        {
        return *finalResult;
        });
    }

Dgn::DgnDbR DgnDbBriefcase::GetDgnDb()
    {
    return *m_db;
    }

const BeBriefcaseId& DgnDbBriefcase::GetBriefcaseId()
    {
    return m_briefcaseId;
    }

Utf8StringCR DgnDbBriefcase::GetLastRevisionId()
    {
    return m_lastRevisionId;
    }

DgnDbRepositoryConnectionPtr DgnDbBriefcase::GetRepositoryConnection()
    {
    return m_repositoryConnection;
    }
