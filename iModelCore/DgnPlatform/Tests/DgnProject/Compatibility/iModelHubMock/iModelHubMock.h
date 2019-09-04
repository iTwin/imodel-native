/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Bentley.h>
#include <DgnPlatform/DgnPlatformApi.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DGN

struct IModelHubMock
{
private:
    DgnDbPtr                                m_iModel;
    BeFileName                              m_storage;
    bmap<BeGuid, BeFileName>                m_storageMap;
    BeSQLite::BeBriefcaseId                 m_currentId;
    bmap<BeGuid, bvector<DgnRevisionPtr>>   m_revisions;
public:
    IModelHubMock(BeFileName storageDirectory) : m_storage(storageDirectory), m_iModel(nullptr), m_currentId(2) {}
    ~IModelHubMock() {BeFileName::EmptyDirectory(m_storage);}
    BeGuid CreateiModel(Utf8StringCR name);
    bool AcquireBriefcase(BeGuid iModelId, BeFileName briefcaseDownloadPath);
    Utf8String PushChangeset(DgnRevisionPtr revision, BeGuid iModelId);
    DgnRevisionCPtr PullChangeset(Utf8StringCR changesetId, BeGuid iModelId);
    bool ManualMergeAllChangesets(BeGuid iModelId);
    void ClearStoredRevisions(BeGuid iModelId) {m_revisions.erase(m_revisions.find(iModelId));}
    void GetRevisionCount(BeGuid iModelId) {m_revisions[iModelId].size();}
};
