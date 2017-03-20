/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/DownloadFilesTask.h $
 |
 |  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
            DownloadFileProperties() : name(std::make_shared<Utf8String>()) {}
            };

    private:
        const CachingDataSource::ProgressCallback m_onProgressCallback;

        FileCache                       m_fileCacheLocation;
        bset<ObjectId>                  m_filesToDownloadIds;
        bvector<DownloadFileProperties> m_filesToDownload;
        size_t                          m_nextFileToDownloadIndex;

        int                             m_downloadTasksRunning;

        CachingDataSource::Progress::State m_downloadBytesProgress;

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
            bset<ObjectId> filesToDownload,
            FileCache fileCacheLocation,
            CachingDataSource::ProgressCallback onProgress,
            ICancellationTokenPtr ct
            );
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
