/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelHubClient/PredownloadManager.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <WebServices/iModelHub/Common.h>
#include <WebServices/iModelHub/Client/iModelConnection.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

//=======================================================================================
//@bsiclass                                      Algirdas.Mikoliunas             01/2017
//=======================================================================================
struct PredownloadManager : RefCountedBase
{
private:
    //! Gets path for pre-downloaded changeSet
    BeFileName static BuildChangeSetPredownloadPathname(Utf8String changeSetId);

    //! Pre-downloads single changeSet by changeSetId
    StatusTaskPtr PredownloadChangeSet(iModelConnectionP imodelConnectionP, Utf8StringCR changeSetId, 
        Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    bvector<BeFileName> GetOrderedCacheFiles(BeFileName directoryName) const;
    uint64_t GetCacheSize(BeFileName directoryName) const;
    void CheckCacheSize(BeFileName changeSetFileName) const;
public:
    bool TryGetChangeSetFile(BeFileName changeSetFileName, Utf8String changeSetId);
    void SubscribeChangeSetsDownload(iModelConnectionP imodelConnectionP);
};

//=======================================================================================
//@bsiclass                                      Algirdas.Mikoliunas             01/2017
//=======================================================================================
struct FileLock : NonCopyableClass
{
private:
    BeFileName m_lockFilePath;
    bool m_locked = false;
public:
    FileLock(BeFileName filePath) {m_lockFilePath = filePath; m_lockFilePath.AppendExtension(L"lock");}
    ~FileLock();
    bool Lock();
};

END_BENTLEY_IMODELHUB_NAMESPACE
