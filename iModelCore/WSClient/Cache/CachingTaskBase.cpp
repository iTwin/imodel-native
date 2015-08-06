/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/CachingTaskBase.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "CachingTaskBase.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CachingTaskBase::CachingTaskBase
(
CachingDataSourcePtr cachingDataSource,
ICancellationTokenPtr cancellationToken
) :
PackagedAsyncTask<void>(nullptr),
m_ds(cachingDataSource),
m_errorCancellationToken(SimpleCancellationToken::Create()),
m_userProvidedCancellationToken(cancellationToken),
m_cancellationToken(MergeCancellationToken::Create(m_userProvidedCancellationToken, m_errorCancellationToken))
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CachingTaskBase::SetError(CachingDataSource::ErrorCR error)
    {
    m_error = error;
    m_errorCancellationToken->SetCanceled();

    _OnError(m_error);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool CachingTaskBase::IsTaskCanceled() const
    {
    return m_cancellationToken->IsCanceled();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ICancellationTokenPtr CachingTaskBase::GetCancellationToken() const
    {
    return m_cancellationToken;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool CachingTaskBase::IsSuccess()
    {
    return !m_cancellationToken->IsCanceled();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CachingTaskBase::AddFailedObject(CacheTransactionCR txn, ObjectIdCR objectId, ICachingDataSource::ErrorCR error)
    {
    m_failedObjects.push_back({objectId, txn.GetCache().ReadInstanceLabel(objectId), error});
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CachingDataSource::FailedObjects& CachingTaskBase::GetFailedObjects()
    {
    return m_failedObjects;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CachingTaskBase::AddResult(const CachingDataSource::BatchResult& result)
    {
    if (!result.IsSuccess())
        {
        SetError(result.GetError());
        return;
        }
    m_failedObjects.insert(m_failedObjects.end(), result.GetValue().begin(), result.GetValue().end());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CachingDataSource::Error& CachingTaskBase::GetError()
    {
    if (m_userProvidedCancellationToken->IsCanceled())
        {
        m_canceledError = CachingDataSource::Status::Canceled;
        return m_canceledError;
        }

    return m_error;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CachingDataSource::BatchResult CachingTaskBase::GetResult()
    {
    if (!IsSuccess())
        {
        return CachingDataSource::BatchResult::Error(GetError());
        }
    return CachingDataSource::BatchResult::Success(m_failedObjects);
    }
