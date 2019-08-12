/*--------------------------------------------------------------------------------------+
 |
 |  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
bmap<ObjectId, ICancellationTokenPtr> filesToDownload,
FileCache fileCacheLocation,
size_t maxParalelDownloads,
uint64_t minTimeBetweenProgressCallsMs,
CachingDataSource::ProgressCallback onProgress,
ICancellationTokenPtr ct
) :
CachingTaskBase(cachingDataSource, ct),
m_maxParalelDownloads(maxParalelDownloads),
m_fileDownloadManager(fileDownloadManager),
m_filesToDownloadIds(std::move(filesToDownload)),
m_fileCacheLocation(fileCacheLocation),
m_nextFileToDownloadIndex(0)
    {
    std::function<bool(CachingDataSource::ProgressCR)> shouldSkipFilter = [] (CachingDataSource::ProgressCR progress)
        {
        return progress.GetBytes().current == progress.GetBytes().total;
        };
    m_onProgressCallback = ProgressFilter::Create(onProgress, shouldSkipFilter, minTimeBetweenProgressCallsMs);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DownloadFilesTask::ProgressCalback(double bytesDownloaded, double bytesTotal, DownloadFileProperties& file)
    {
    BeMutexHolder lock(m_progressInfoCS);

    file.bytesDownloaded = (uint64_t) bytesDownloaded;

    double totalBytesDownloaded = m_processedFileSizes;
    for (auto file : m_filesBeingDownloaded)
        totalBytesDownloaded += file->bytesDownloaded;

    ICachingDataSource::Progress::State progress(totalBytesDownloaded, m_totalBytesToDownload);
    m_onProgressCallback({progress, file.name});
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DownloadFilesTask::_OnExecute()
    {
    auto txn = m_ds->StartCacheTransaction();

    for (auto fileEntry : m_filesToDownloadIds)
        {
        ObjectId fileId = fileEntry.first;
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
        fileProperties.cancellationToken = fileEntry.second;

        m_filesToDownload.push_back(fileProperties);
        m_totalBytesToDownload += fileProperties.size;
        }

    txn.Commit();

    ContinueDownloadingFiles();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DownloadFilesTask::ContinueDownloadingFiles()
    {
    LOG.tracev("DownloadFilesTask: running download tasks: %d", m_filesBeingDownloaded.size());

    BeMutexHolder lock(m_progressInfoCS);
    while ((size_t)m_filesBeingDownloaded.size() < m_maxParalelDownloads && m_nextFileToDownloadIndex < m_filesToDownload.size())
        {
        if (IsTaskCanceled()) break;

        DownloadFileProperties* file = &m_filesToDownload[m_nextFileToDownloadIndex];
        m_nextFileToDownloadIndex++;
        m_filesBeingDownloaded.insert(file);

        // TODO: move max download limitation to m_fileDownloadManager->DownloadAndCacheFile()
        auto onProgress = std::bind(&DownloadFilesTask::ProgressCalback, this, std::placeholders::_1, std::placeholders::_2, std::ref(*file));
        auto ct = MergeCancellationToken::Create(GetCancellationToken(), file->cancellationToken);
        m_fileDownloadManager->DownloadAndCacheFile(file->objectId, *file->name, m_fileCacheLocation, onProgress, ct)
            ->Then(m_ds->GetCacheAccessThread(), [=] (ICachingDataSource::Result& result)
            {
            if (IsTaskCanceled()) return;

            if (true)
                {
                BeMutexHolder lock(m_progressInfoCS);
                m_filesBeingDownloaded.erase(file);
                m_processedFileSizes += file->size;
                }

            if (result.IsSuccess())
                ProgressCalback(static_cast<double>(file->size), static_cast<double>(file->size), *file);

            if (!result.IsSuccess())
                {
                if (result.GetError().GetWSError().IsInstanceNotAvailableError())
                    {
                    auto txn = m_ds->StartCacheTransaction();
                    AddFailedObject(txn.GetCache(), file->objectId, result.GetError());
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
