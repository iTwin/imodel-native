/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerPreDownloadManager.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/DgnDbRepositoryConnection.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE

//=======================================================================================
//@bsiclass                                      Algirdas.Mikoliunas             01/2017
//=======================================================================================
struct DgnDbServerPreDownloadManager : RefCountedBase
{
private:
    //! Gets path for pre-downloaded revision
    BeFileName static BuildRevisionPreDownloadPathname(Utf8String revisionId);

    //! Pre-downloads single revision by revisionId
    DgnDbServerStatusTaskPtr PreDownloadRevision(DgnDbRepositoryConnectionP repositoryConnectionP, Utf8StringCR revisionId, 
        Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

    bvector<BeFileName> GetOrderedCacheFiles(BeFileName directoryName) const;
    uint64_t GetCacheSize(BeFileName directoryName) const;
    void CheckCacheSize(BeFileName revisionFileName) const;
public:
    bool TryGetRevisionFile(BeFileName revisionFileName, Utf8String revisionId);
    void SubscribeRevisionsDownload(DgnDbRepositoryConnectionP repositoryConnectionP);
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

END_BENTLEY_DGNDBSERVER_NAMESPACE
