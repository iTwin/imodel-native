/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/iModelAdmin.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbRepositoryAdmin.h>
#include <DgnDbServer/Client/DgnDbClient.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER
USING_NAMESPACE_BENTLEY_DGN

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Karolis.Dziedzelis              09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbRepositoryAdmin::DgnDbRepositoryAdmin(DgnDbClientP client) : m_client(client)
    {
    m_managers = std::make_unique<bmap<Utf8String, DgnDbRepositoryManagerPtr>>();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Karolis.Dziedzelis              09/16
+---------------+---------------+---------------+---------------+---------------+------*/
IRepositoryManagerP DgnDbRepositoryAdmin::_GetRepositoryManager(DgnDbR db) const
    {
    DgnDbServerRepositoryResult readResult = RepositoryInfo::ReadRepositoryInfo(db);
    if (!readResult.IsSuccess())
        return nullptr;
    RepositoryInfoPtr repositoryInfo = readResult.GetValue();
    DgnDbServerBriefcaseInfo briefcase(db.GetBriefcaseId());
    Utf8String repositoryId = repositoryInfo->GetId();
    if ((*m_managers)[repositoryId].IsNull())
        {
        FileInfoPtr fileInfo = FileInfo::Create(db, "");
        auto managerResult = m_client->CreateRepositoryManager(*repositoryInfo, *fileInfo, briefcase)->GetResult();
        if (!managerResult.IsSuccess())
            return nullptr;
        (*m_managers)[repositoryId] = managerResult.GetValue();
        }

    return (*m_managers)[repositoryId].get();
    }
