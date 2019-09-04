/*--------------------------------------------------------------------------------------+
 |
 |  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <Bentley/Tasks/AsyncTask.h>
#include <WebServices/Cache/CachingDataSource.h>
#include <atomic>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct CachingTaskBase : public PackagedAsyncTask<void>
    {
    public:
        typedef std::function<void(size_t synced)> ProgressCallback;

    protected:
        CachingDataSourcePtr        m_ds;

    private:
        CachingDataSource::Error    m_canceledError;

        SimpleCancellationTokenPtr  m_errorCancellationToken;
        ICancellationTokenPtr       m_userProvidedCancellationToken;
        ICancellationTokenPtr       m_cancellationToken;

        CachingDataSource::Error         m_error;
        CachingDataSource::FailedObjects m_failedObjects;

    protected:
        virtual void _OnExecute() = 0;
        virtual void _OnError(CachingDataSource::ErrorCR error)
            {};

        // Set error and cancel task
        void SetError(CachingDataSource::ErrorCR error = ICachingDataSource::Status::InternalCacheError);
        // Returns true if user canceled or error occurred - SetError() was called
        bool IsTaskCanceled() const;
        // Get main cancellation token to use for operations
        ICancellationTokenPtr GetCancellationToken() const;
        // Get user provided cancellation token, use to to pass to other CachingTaskBase objects
        ICancellationTokenPtr GetUserCancellationToken() const { return m_userProvidedCancellationToken; };
        // Get abort cancellation token, use to to pass to other CachingTaskBase objects. Token is set to canceled when error occurs, but not when user cancels.
        SimpleCancellationTokenPtr GetAbortCancellationToken() const { return m_errorCancellationToken; };
        
        // Add results or error to task
        void AddResult(const CachingDataSource::BatchResult& result);
        void AddFailedObject(IDataSourceCache& cache, ObjectIdCR objectId, ICachingDataSource::ErrorCR error, Utf8String objectLabel = nullptr);

    public:
        //! Create new object.
        //! Care must be taken to properly manage cancellation tokens so cancellation and errors would not mix.
        //! @param cachingDataSource
        //! @param userCt - user provided cancellation token. Can be null.
        //! @param abortCt - shared abort cancellation token or new token. Cannot be null.
        CachingTaskBase
            (
            CachingDataSourcePtr cachingDataSource,
            ICancellationTokenPtr userCt,
            SimpleCancellationTokenPtr abortCt
            );

        bool                              IsSuccess();
        CachingDataSource::Error&         GetError();
        CachingDataSource::FailedObjects& GetFailedObjects();
        CachingDataSource::BatchResult    GetResult();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
