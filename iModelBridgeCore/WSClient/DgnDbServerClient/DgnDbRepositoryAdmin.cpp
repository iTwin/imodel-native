/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbRepositoryAdmin.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbRepositoryAdmin.h>
#include <DgnDbServer/Client/DgnDbClient.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER
USING_NAMESPACE_BENTLEY_DGNPLATFORM

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
    RepositoryInfo repositoryInfo;
    RepositoryInfo::ReadRepositoryInfo(repositoryInfo, db);
    DgnDbBriefcaseInfo briefcase(db.GetBriefcaseId());
    Utf8String repositoryId = repositoryInfo.GetId();
    if (nullptr == (*m_managers)[repositoryId])
        {
        FileInfo fileInfo(db, "");
        auto managerResult = m_client->CreateRepositoryManager(repositoryInfo, fileInfo, briefcase)->GetResult();
        if (!managerResult.IsSuccess())
            return nullptr;
        (*m_managers)[repositoryId] = managerResult.GetValue();
        }

    return (*m_managers)[repositoryId].get();
    }
