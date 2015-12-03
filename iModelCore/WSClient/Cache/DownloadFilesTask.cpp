/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/DownloadFilesTask.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "DownloadFilesTask.h"

#include <algorithm>
#include <Bentley/BeTimeUtilities.h>
#include <WebServices/Cache/Persistence/CacheQueryHelper.h>
#include <WebServices/Cache/Persistence/DataReadOptions.h>

#include "Logging.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DownloadFilesTask::DownloadFilesTask
(
CachingDataSourcePtr cachingDataSource,
bset<ObjectId> filesToDownload,
FileCache fileCacheLocation,
CachingDataSource::LabeledProgressCallback onProgress,
ICancellationTokenPtr ct
) :
CachingTaskBase(cachingDataSource, ct),
m_filesToDownloadIds(std::move(filesToDownload)),
m_fileCacheLocation(fileCacheLocation),
m_onProgressCallback(std::move(onProgress)),
m_downloadTasksRunning(0),
m_totalBytesToDownload(0),
m_totalBytesDownloaded(0),
m_nextFileToDownloadIndex(0)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DownloadFilesTask::ProgressCalback(double bytesDownloaded, double bytesTotal, DownloadFileProperties& file)
    {
    if (!m_onProgressCallback)
        {
        return;
        }

    m_totalBytesDownloaded += (uint64_t) bytesDownloaded - file.bytesDownloaded;
    file.bytesDownloaded = (uint64_t) bytesDownloaded;

    uint64_t currentTimeMillis = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
    if (currentTimeMillis - m_lastTimeReported >= 250)
        {
        m_lastTimeReported = currentTimeMillis;

        m_onProgressCallback((double) m_totalBytesDownloaded, (double) m_totalBytesToDownload, file.name);
        }
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
        if (SUCCESS != txn.GetCache().ReadFileProperties(fileKey, fileProperties.name, fileProperties.size))
            {
            SetError(ICachingDataSource::Status::DataNotCached);
            return;
            }

        fileProperties.objectId = fileId;
        fileProperties.bytesDownloaded = 0;

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
    int maxDownloadsRunning = 10;

    LOG.tracev("DownloadFilesTask: running download tasks: %d", m_downloadTasksRunning);

    while (m_downloadTasksRunning < maxDownloadsRunning && m_nextFileToDownloadIndex < m_filesToDownload.size())
        {
        if (IsTaskCanceled()) break;

        auto launchedTask = DownloadFile(m_filesToDownload[m_nextFileToDownloadIndex]);

        m_nextFileToDownloadIndex++;

        m_downloadTasksRunning++;
        launchedTask->Then(m_ds->GetCacheAccessThread(), [=]
            {
            m_downloadTasksRunning--;
            ContinueDownloadingFiles();
            });
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> DownloadFilesTask::DownloadFile(DownloadFileProperties& file)
    {
    auto txn = m_ds->StartCacheTransaction();
    TempFilePtr tempFile = m_ds->GetTempFile(file.name, file.objectId);
    Utf8String fileCacheTag = txn.GetCache().ReadFileCacheTag(file.objectId);
    auto onProgress = std::bind(&DownloadFilesTask::ProgressCalback, this, std::placeholders::_1, std::placeholders::_2, std::ref(file));

    return m_ds->m_client->SendGetFileRequest(file.objectId, tempFile->GetPath(), fileCacheTag, onProgress, GetCancellationToken())
        ->Then(m_ds->GetCacheAccessThread(), [=, &file] (WSFileResult& fileResult)
        {
        if (IsTaskCanceled()) return;

        auto txn = m_ds->StartCacheTransaction();
        if (!fileResult.IsSuccess())
            {
            WSError::Id errorId = fileResult.GetError().GetId();

            if (WSError::Id::InstanceNotFound == errorId ||
                WSError::Id::NotEnoughRights == errorId)
                {
                AddFailedObject(txn, file.objectId, fileResult.GetError());
                txn.GetCache().RemoveFile(file.objectId);
                return;
                }
            else
                {
                SetError(fileResult.GetError());
                return;
                }
            }

        if (SUCCESS != txn.GetCache().CacheFile(file.objectId, fileResult.GetValue(), m_fileCacheLocation))
            {
            SetError();
            return;
            }

        txn.Commit();
        tempFile->Cleanup();
        });
    }
