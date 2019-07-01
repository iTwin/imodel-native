/*--------------------------------------------------------------------------------------+
 |
 |  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include "CachingTaskBase.h"
#include "FileDownloadManager.h"
#include <WebServices/Cache/CachingDataSource.h>
#include <atomic>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct DownloadFilesTask : public CachingTaskBase
    {
    private:
        struct DownloadFileProperties
            {
            ObjectId objectId;
            std::shared_ptr<Utf8String> name;
            uint64_t size;
            uint64_t bytesDownloaded;
            ICancellationTokenPtr cancellationToken;
            DownloadFileProperties() : name(std::make_shared<Utf8String>()) {}
            };

    private:
        CachingDataSource::ProgressCallback m_onProgressCallback;
        size_t m_maxParalelDownloads;

        FileCache                       m_fileCacheLocation;
        bmap<ObjectId, ICancellationTokenPtr>   m_filesToDownloadIds;
        bvector<DownloadFileProperties> m_filesToDownload;
        size_t                          m_nextFileToDownloadIndex;

        BeMutex               m_progressInfoCS;
        bset<DownloadFileProperties*>   m_filesBeingDownloaded;
        double                          m_totalBytesToDownload = 0;
        double                          m_processedFileSizes = 0;

        std::shared_ptr<FileDownloadManager> m_fileDownloadManager;

    private:
        virtual void _OnExecute();

        void ContinueDownloadingFiles();
        void ProgressCalback(double bytesDownloaded, double bytesTotal, DownloadFileProperties& file);

    public:
        DownloadFilesTask
            (
            CachingDataSourcePtr cachingDataSource,
            std::shared_ptr<FileDownloadManager> fileDownloadManager,
            bmap<ObjectId, ICancellationTokenPtr> filesToDownload,
            FileCache fileCacheLocation,
            size_t maxParalelDownloads,
            uint64_t minTimeBetweenProgressCallsMs,
            CachingDataSource::ProgressCallback onProgress,
            ICancellationTokenPtr ct
            );
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
