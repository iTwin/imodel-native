/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/iModelAdmin.h>
#include <WebServices/iModelHub/Client/Client.h>
#include "Utils.h"

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_DGN

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              09/16
+---------------+---------------+---------------+---------------+---------------+------*/
iModelAdmin::iModelAdmin(ClientP client) : m_client(client)
    {
    m_managers = std::make_unique<bmap<Utf8String, iModelManagerPtr>>();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              09/16
+---------------+---------------+---------------+---------------+---------------+------*/
IRepositoryManagerP iModelAdmin::_GetRepositoryManager(DgnDbR db) const
    {
    iModelResult readResult = iModelInfo::ReadiModelInfo(db);
    if (!readResult.IsSuccess())
        return nullptr;
    iModelInfoPtr iModelInfo = readResult.GetValue();
    BriefcaseInfo briefcase(db.GetBriefcaseId());
    Utf8String iModelId = iModelInfo->GetId();
    if ((*m_managers)[iModelId].IsNull())
        {
        FileInfoPtr fileInfo = FileInfo::Create(db, "");
        auto managerResult = ExecuteAsync(m_client->CreateiModelManager(*iModelInfo, *fileInfo, briefcase));
        if (!managerResult->IsSuccess())
            return nullptr;
        (*m_managers)[iModelId] = managerResult->GetValue();
        }

    return (*m_managers)[iModelId].get();
    }
