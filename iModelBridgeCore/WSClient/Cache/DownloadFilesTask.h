/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/DownloadFilesTask.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include "CachingTaskBase.h"
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
            Utf8String name;
            uint64_t size;
            uint64_t bytesDownloaded;
            };

    private:
        const CachingDataSource::LabeledProgressCallback m_onProgressCallback;

        FileCache                       m_fileCacheLocation;
        bset<ObjectId>                  m_filesToDownloadIds;
        bvector<DownloadFileProperties> m_filesToDownload;
        size_t                          m_nextFileToDownloadIndex;

        int                             m_downloadTasksRunning;

        std::atomic<uint64_t>           m_totalBytesToDownload;
        std::atomic<uint64_t>           m_totalBytesDownloaded;

    private:
        virtual void _OnExecute();

        void ContinueDownloadingFiles();
        AsyncTaskPtr<void> DownloadFile(DownloadFileProperties& file);
        void ProgressCalback(double bytesDownloaded, double bytesTotal, DownloadFileProperties& file);

    public:
        DownloadFilesTask
            (
            CachingDataSourcePtr cachingDataSource,
            bset<ObjectId> filesToDownload,
            FileCache fileCacheLocation,
            CachingDataSource::LabeledProgressCallback onProgress,
            ICancellationTokenPtr ct
            );
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
