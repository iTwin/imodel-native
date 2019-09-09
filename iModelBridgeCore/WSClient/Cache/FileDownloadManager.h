/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <WebServices/Cache/ICachingDataSource.h>
#include <BeHttp/HttpRequest.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

struct CachingDataSource;

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct FileDownloadManager
    {
    private:
        struct FileDownload;
        struct FileDownloadListener;

        template <class T>
        struct PromiseAsyncTask : PackagedAsyncTask<T>
            {
            public:
                PromiseAsyncTask() : PackagedAsyncTask<T>(nullptr) {}
                virtual void _OnExecute() {}
                void SetValue(const T& value)
                    {
                    PackagedAsyncTask<T>::m_result = value;
                    PackagedAsyncTask<T>::Execute();
                    }
            };

    private:
        CachingDataSource* m_ds;

        bmap<ObjectId, std::shared_ptr<FileDownload>> m_downloadsInProgress;

    private:
        void StartFileDownload
            (
            std::shared_ptr<FileDownloadManager::FileDownload> fileDownload,
            ObjectId objectId,
            Utf8StringCR fileName,
            FileCache cacheLocation
            );

        void EndFileDownload(ObjectId objectId, ICachingDataSource::Result& result);

    public:
        FileDownloadManager(CachingDataSource& ds);

        AsyncTaskPtr<ICachingDataSource::Result> DownloadAndCacheFile
            (
            ObjectId objectId,
            Utf8StringCR fileName,
            FileCache cacheLocation,
            Http::Request::ProgressCallbackCR onProgress,
            ICancellationTokenPtr ct
            );
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct FileDownloadManager::FileDownloadListener : ICancellationListener, std::enable_shared_from_this<FileDownloadListener>
    {
    private:
        std::shared_ptr<PromiseAsyncTask<ICachingDataSource::Result>> m_promise;
        Http::Request::ProgressCallback m_onProgress;

        std::shared_ptr<FileDownload> m_fileDownload;

    private:
        FileDownloadListener(Http::Request::ProgressCallbackCR onProgress, std::shared_ptr<FileDownload> fileDownload);

    public:
        static std::shared_ptr<FileDownloadListener> Create
            (
            ICancellationTokenPtr ct,
            Http::Request::ProgressCallbackCR onProgress,
            std::shared_ptr<FileDownload> fileDownload
            );

        AsyncTaskPtr<ICachingDataSource::Result> GetAsyncTask();

        void OnProgress(double bytesTransfered, double bytesTotal);
        void Complete(const ICachingDataSource::Result& result);

        virtual void OnCanceled() override;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct FileDownloadManager::FileDownload : SimpleCancellationToken
    {
    private:
        std::mutex m_listeners_mutex;
        bset<std::shared_ptr<FileDownloadListener>> m_listeners;

        double m_lastProgressBytesTransfered = -1;
        double m_lastProgressBytesTotal = -1;

    public:
        FileDownload();
        virtual ~FileDownload();

        void AddListener(std::shared_ptr<FileDownloadListener> listener);
        void RemoveListener(std::shared_ptr<FileDownloadListener> listener);

        void OnProgress(double bytesTransfered, double bytesTotal);
        void Complete(const ICachingDataSource::Result& result);

        virtual bool IsCanceled() override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
