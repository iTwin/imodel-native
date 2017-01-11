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
struct DgnDbServerPreDownloadManager
    {
    private:
       BeMutex                                    m_revisionMutex;
       bmap<Utf8String, std::shared_ptr<BeMutex>> m_revisionMutexSet;
       static const int                           s_maxCacheSize = 1024 * 1024 * 10; // 10 MB

       //! Gets path for pre-downloaded revision
       BeFileName static BuildRevisionPreDownloadPathname(Utf8String revisionId);

       //! Pre-downloads single revision by revisionId
       DgnDbServerStatusTaskPtr PreDownloadRevision(DgnDbRepositoryConnectionP repositoryConnectionP, Utf8StringCR revisionId, 
           Http::Request::ProgressCallbackCR callback = nullptr, ICancellationTokenPtr cancellationToken = nullptr) const;

       std::shared_ptr<BeMutex> GetRevisionMutex(Utf8String revisionId);

       bvector<BeFileName> GetOrderedCacheFiles(BeFileName directoryName) const;
       uint64_t GetCacheSize(BeFileName directoryName) const;
       void CheckCacheSize(BeFileName revisionFileName) const;
    public:
        bool TryGetRevisionFile(BeFileName revisionFileName, Utf8String revisionId);
        void SubscribeRevisionsDownload(DgnDbRepositoryConnectionP repositoryConnectionP);
    };

END_BENTLEY_DGNDBSERVER_NAMESPACE
