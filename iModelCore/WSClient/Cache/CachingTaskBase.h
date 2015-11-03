/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/CachingTaskBase.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <MobileDgn/Utils/Threading/AsyncTask.h>
#include <WebServices/Cache/CachingDataSource.h>
#include <atomic>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct CachingTaskBase : public PackagedAsyncTask<void>
    {
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
        // Get main cancellation token for task
        ICancellationTokenPtr GetCancellationToken() const;
        // Add results or error to task
        void AddResult(const CachingDataSource::BatchResult& result);
        void AddFailedObject(CacheTransactionCR txn, ObjectIdCR objectId, ICachingDataSource::ErrorCR error);

    public:
        CachingTaskBase
            (
            CachingDataSourcePtr cachingDataSource,
            ICancellationTokenPtr cancellationToken
            );

        bool                              IsSuccess();
        CachingDataSource::Error&         GetError();
        CachingDataSource::FailedObjects& GetFailedObjects();
        CachingDataSource::BatchResult    GetResult();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
