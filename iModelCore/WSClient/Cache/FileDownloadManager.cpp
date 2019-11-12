/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "FileDownloadManager.h"
#include <WebServices/Cache/CachingDataSource.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                          
+---------------+---------------+---------------+---------------+---------------+------*/
FileDownloadManager::FileDownloadManager(CachingDataSource& ds) :
m_ds(&ds)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                      
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<ICachingDataSource::Result> FileDownloadManager::DownloadAndCacheFile
(
ObjectId objectId,
Utf8StringCR fileName,
FileCache cacheLocation,
Http::Request::ProgressCallbackCR onProgress,
ICancellationTokenPtr ct
)
    {
    std::shared_ptr<FileDownload> fileDownload;    

    bool downloadStarted = false;
    auto it = m_downloadsInProgress.find(objectId);
    if (it != m_downloadsInProgress.end())
        {
        fileDownload = it->second;
        downloadStarted = true;
        }
    else
        {
        fileDownload = std::make_shared<FileDownload>();
        m_downloadsInProgress[objectId] = fileDownload;
        }

    auto listener = FileDownloadListener::Create(ct, onProgress, fileDownload);
    fileDownload->AddListener(listener);

    if (!downloadStarted)
        StartFileDownload(fileDownload, objectId, fileName, cacheLocation);

    if (ct->IsCanceled())
        listener->OnCanceled();

    return listener->GetAsyncTask();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void FileDownloadManager::StartFileDownload
(
std::shared_ptr<FileDownloadManager::FileDownload> fileDownload,
ObjectId objectId,
Utf8StringCR fileName,
FileCache cacheLocation
)
    {
    TempFilePtr tempFile = m_ds->GetTempFile(fileName, objectId); // TODO: avoid private CachingDataSource functions

    auto txn = m_ds->StartCacheTransaction();
    Utf8String fileCacheTag = txn.GetCache().ReadFileCacheTag(objectId);
    txn.Commit();

    auto onProgress = [=] (double bytesTransfered, double bytesTotal)
        {
        fileDownload->OnProgress(bytesTransfered, bytesTotal);
        };

    auto finalResult = std::make_shared<ICachingDataSource::Result>(ICachingDataSource::Result::Success());
    m_ds->GetClient()->SendGetFileRequest
        (
        objectId, 
        tempFile->GetPath(), 
        fileCacheTag, 
        onProgress,
        fileDownload
        )
        ->Then(m_ds->GetCacheAccessThread(), [=] (WSFileResult& fileResult)
        {
        if (fileDownload->IsCanceled()) return;

        auto txn = m_ds->StartCacheTransaction();
        if (!fileResult.IsSuccess())
            {
            if (fileResult.GetError().IsInstanceNotAvailableError())
                {
                txn.GetCache().RemoveFile(objectId);
                }

            finalResult->SetError(fileResult.GetError());
            }
        else if (SUCCESS != txn.GetCache().CacheFile(objectId, fileResult.GetValue(), cacheLocation))
            {
            finalResult->SetError(CachingDataSource::Status::InternalCacheError);
            }

        txn.Commit();
        tempFile->Cleanup();
        })
    ->Then(m_ds->GetCacheAccessThread(), [=]
        {
        EndFileDownload(objectId, *finalResult);
        });
    }
    
/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void FileDownloadManager::EndFileDownload(ObjectId objectId, ICachingDataSource::Result& result)
    {
    m_downloadsInProgress[objectId]->Complete(result);
    m_downloadsInProgress.erase(objectId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<FileDownloadManager::FileDownloadListener> FileDownloadManager::FileDownloadListener::Create
(
ICancellationTokenPtr ct,
Http::Request::ProgressCallbackCR onProgress, 
std::shared_ptr<FileDownload> fileDownload
)
    {
    std::shared_ptr<FileDownloadManager::FileDownloadListener> listener =
        std::shared_ptr<FileDownloadListener>(new FileDownloadListener(onProgress, fileDownload));

    if (ct != nullptr)
        ct->Register(listener);

    return listener;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
FileDownloadManager::FileDownloadListener::FileDownloadListener
(
Http::Request::ProgressCallbackCR onProgress, 
std::shared_ptr<FileDownload> fileDownload
) :
m_onProgress(onProgress),
m_fileDownload(fileDownload)
    {
    m_promise = std::make_shared<PromiseAsyncTask<ICachingDataSource::Result>>();

    auto parentTask = AsyncTasksManager::GetCurrentThreadAsyncTask();
    if (nullptr != parentTask)
        {
        parentTask->AddSubTask(m_promise);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<ICachingDataSource::Result> FileDownloadManager::FileDownloadListener::GetAsyncTask()
    {
    return m_promise;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void FileDownloadManager::FileDownloadListener::OnProgress(double bytesTransfered, double bytesTotal)
    {
    return m_onProgress(bytesTransfered, bytesTotal);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void FileDownloadManager::FileDownloadListener::Complete(const ICachingDataSource::Result& result)
    {
    m_promise->SetValue(result);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void FileDownloadManager::FileDownloadListener::OnCanceled()
    {
    m_fileDownload->RemoveListener(shared_from_this());

    m_promise->SetValue(ICachingDataSource::Result::Error(ICachingDataSource::Status::Canceled));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
FileDownloadManager::FileDownload::FileDownload()
: SimpleCancellationToken(false)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
FileDownloadManager::FileDownload::~FileDownload()
    {
    }
        
/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void FileDownloadManager::FileDownload::AddListener (std::shared_ptr<FileDownloadListener> listener)
    {
    std::unique_lock<std::mutex> lock(m_listeners_mutex);
    m_listeners.insert(listener);
    lock.unlock();
 
    if (m_lastProgressBytesTotal >= 0)
        listener->OnProgress(m_lastProgressBytesTransfered, m_lastProgressBytesTotal);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void FileDownloadManager::FileDownload::RemoveListener(std::shared_ptr<FileDownloadListener> listener)
    {
    std::lock_guard<std::mutex> lock(m_listeners_mutex);

    m_listeners.erase(listener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void FileDownloadManager::FileDownload::OnProgress(double bytesTransfered, double bytesTotal)
    {
    m_lastProgressBytesTransfered = bytesTransfered;
    m_lastProgressBytesTotal = bytesTotal;

    std::unique_lock<std::mutex> lock(m_listeners_mutex);
    bset<std::shared_ptr<FileDownloadListener>> listenersCopy(m_listeners.begin(), m_listeners.end());
    lock.unlock();

    for (auto listener : listenersCopy)
        listener->OnProgress(bytesTransfered, bytesTotal);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void FileDownloadManager::FileDownload::Complete(const ICachingDataSource::Result& result)
    {
    std::unique_lock<std::mutex> lock(m_listeners_mutex);
    bset<std::shared_ptr<FileDownloadListener>> listenersCopy(m_listeners.begin(), m_listeners.end());
    lock.unlock();

    for (auto listener : listenersCopy)
        listener->Complete(result);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool FileDownloadManager::FileDownload::IsCanceled()
    {
    std::lock_guard<std::mutex> lock(m_listeners_mutex);
    // FileDownload is canceled when all FileDownloadListener are cancelled.
    // Cancelling FileDownloadListener removes it from m_listeners list.
    return m_listeners.empty(); 
    }

