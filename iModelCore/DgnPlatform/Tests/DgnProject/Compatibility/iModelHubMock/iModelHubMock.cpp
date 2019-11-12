/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "iModelHubMock.h"

//-------------------------------------------------------------------------------------
// @bsimethod                                            Kyle.Abramowitz       06/2018
//-------------------------------------------------------------------------------------
BeGuid IModelHubMock::CreateiModel(Utf8StringCR name)
    {
    DbResult stat;
    auto filepath = m_storage;
    filepath.AppendToPath(BeFileName(name.c_str()));
    filepath.AppendExtension(L"bim");
    auto db = DgnDb::CreateDgnDb(&stat, filepath, CreateDgnDbParams(name.c_str(), "Awesome description"));
    auto id = db->GetDbGuid();
    m_storageMap[id] = filepath;
    db->SaveChanges();
    db->CloseDb();
    return id;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                            Kyle.Abramowitz       06/2018
//-------------------------------------------------------------------------------------
bool IModelHubMock::AcquireBriefcase(BeGuid iModelId, BeFileName briefcaseDownloadPath)
    {
    DbResult stat;
    auto filepath = m_storage;
    filepath.AppendToPath(BeFileName(iModelId.ToString()));
    filepath.AppendExtension(L"bim");
    BeFileName::BeCopyFile(m_storageMap[iModelId], filepath);
    auto db = DgnDb::OpenDgnDb(&stat, filepath, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    m_currentId = m_currentId.GetNextBriefcaseId(); // Update briefcaseID
    if (db->IsBriefcase()) // Can't set briefcaseID if already a briefcase. Hacky workaround. This comes up when pulling multiple briefcases for 1 model
        db->SetAsMaster(iModelId);

    db->SetAsBriefcase(m_currentId);
    db->Txns().EnableTracking(true);
    db->SaveChanges();
    db->CloseDb();
    BeFileName::BeCopyFile(filepath, briefcaseDownloadPath);
    return true;
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                            Kyle.Abramowitz       06/2018
//-------------------------------------------------------------------------------------
Utf8String IModelHubMock::PushChangeset(DgnRevisionPtr revision, BeGuid iModelId)
    {
    m_revisions[iModelId].push_back(revision);
    return revision->GetId();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                            Kyle.Abramowitz       06/2018
//-------------------------------------------------------------------------------------
DgnRevisionCPtr IModelHubMock::PullChangeset(Utf8StringCR changeSetId, BeGuid iModelId)
    {
    BeAssert(false && "Not implemented");
    return nullptr; // TODO
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                            Kyle.Abramowitz       06/2018
//-------------------------------------------------------------------------------------
bool IModelHubMock::ManualMergeAllChangesets(BeGuid iModelId)
    {
    auto path = m_storageMap[iModelId];
    DbResult stat;

    {
    auto db = DgnDb::OpenDgnDb(&stat, path, DgnDb::OpenParams(Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes));
    db->SetAsBriefcase(BeBriefcaseId(BeBriefcaseId::Standalone()));
    db->SaveChanges();
    }

    bvector<DgnRevisionCP> revisions;
    for (const auto& rev : m_revisions[iModelId])
        revisions.push_back(rev.get());

    auto openParam = DgnDb::OpenParams(Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes);
    openParam.GetSchemaUpgradeOptionsR().SetUpgradeFromRevisions(revisions);
    auto db = DgnDb::OpenDgnDb(&stat, path, openParam);
    db->SetAsMaster(iModelId);
    db->SaveChanges();
    m_revisions[iModelId].clear();
    return true;
    }


