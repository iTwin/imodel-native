/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/DownloadFilesTask.cpp $
 |
 |  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "DownloadFilesTask.h"

#include <algorithm>
#include <Bentley/BeTimeUtilities.h>
#include <WebServices/Cache/Persistence/CacheQueryHelper.h>
#include <WebServices/Cache/Persistence/DataReadOptions.h>
#include <WebServices/Cache/Util/ProgressFilter.h>


#include "Logging.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DownloadFilesTask::DownloadFilesTask
(
CachingDataSourcePtr cachingDataSource,
std::shared_ptr<FileDownloadManager> fileDownloadManager,
bset<ObjectId> filesToDownload,
FileCache fileCacheLocation,
CachingDataSource::ProgressCallback onProgress,
ICancellationTokenPtr ct
) :
CachingTaskBase(cachingDataSource, ct),
m_fileDownloadManager(fileDownloadManager),
m_filesToDownloadIds(std::move(filesToDownload)),
m_fileCacheLocation(fileCacheLocation),
m_downloadTasksRunning(0),
m_nextFileToDownloadIndex(0)
    {
    std::function<bool(CachingDataSource::ProgressCR)> shouldSkipFilter = [] (CachingDataSource::ProgressCR progress)
        {
        return progress.GetBytes().current == progress.GetBytes().total;
        };
    m_onProgressCallback = ProgressFilter::Create(onProgress, shouldSkipFilter);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DownloadFilesTask::ProgressCalback(double bytesDownloaded, double bytesTotal, DownloadFileProperties& file)
    {
    m_downloadBytesProgress.current += (uint64_t) bytesDownloaded - file.bytesDownloaded;
    file.bytesDownloaded = (uint64_t) bytesDownloaded;

    m_onProgressCallback({m_downloadBytesProgress, file.name});
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DownloadFilesTask::_OnExecute()
    {
    auto txn = m_ds->StartCacheTransaction();

    for (ObjectIdCR fileId : m_filesToDownloadIds)
        {
        ECInstanceKey fileKey = txn.GetCache().FindInstance(fileId);
        if (!fileKey.IsValid())
            {
            SetError(ICachingDataSource::Status::DataNotCached);
            return;
            }

        DownloadFileProperties fileProperties;
        if (SUCCESS != txn.GetCache().ReadFileProperties(fileKey, fileProperties.name.get(), &fileProperties.size))
            {
            SetError(ICachingDataSource::Status::DataNotCached);
            return;
            }

        fileProperties.objectId = fileId;
        fileProperties.bytesDownloaded = 0;

        m_filesToDownload.push_back(fileProperties);
        m_downloadBytesProgress.total += fileProperties.size;
        }

    txn.Commit();

    ContinueDownloadingFiles();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DownloadFilesTask::ContinueDownloadingFiles()
    {
    int maxDownloadsRunning = 10;

    LOG.tracev("DownloadFilesTask: running download tasks: %d", m_downloadTasksRunning);

    while (m_downloadTasksRunning < maxDownloadsRunning && m_nextFileToDownloadIndex < m_filesToDownload.size())
        {
        if (IsTaskCanceled()) break;

        DownloadFileProperties& file = m_filesToDownload[m_nextFileToDownloadIndex];

        m_nextFileToDownloadIndex++;

        m_downloadTasksRunning++;

        auto onProgress = std::bind(&DownloadFilesTask::ProgressCalback, this, std::placeholders::_1, std::placeholders::_2, std::ref(file));
        m_fileDownloadManager->DownloadAndCacheFile(file.objectId, *file.name, m_fileCacheLocation, onProgress, GetCancellationToken())
            ->Then(m_ds->GetCacheAccessThread(), [=, &file] (ICachingDataSource::Result& result)
            {
            m_downloadTasksRunning--;

            if (IsTaskCanceled()) return;

            if (result.IsSuccess())
                ProgressCalback(static_cast<double>(file.size), static_cast<double>(file.size), file);

            if (!result.IsSuccess())
                {
                WSError::Id errorId = result.GetError().GetWSError().GetId();

                if (WSError::Id::InstanceNotFound == errorId ||
                    WSError::Id::NotEnoughRights == errorId)
                    {
                    auto txn = m_ds->StartCacheTransaction();
                    AddFailedObject(txn.GetCache(), file.objectId, result.GetError());
                    txn.Commit();
                    return;
                    }
                else
                    {
                    SetError(result.GetError());
                    return;
                    }
                }
            
            ContinueDownloadingFiles();
            });
        }
    }
