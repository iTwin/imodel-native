/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/SyncNotifier.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             
+---------------+---------------+---------------+---------------+---------------+------*/
SyncNotifierPtr SyncNotifier::Create()
    {
    return std::shared_ptr<SyncNotifier>(new SyncNotifier());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                         
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncNotifier::AddTask(AsyncTaskPtr<SyncResult> task)
    {
    BeMutexHolder lock(m_mutex);
    m_tasks.insert(task);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<SyncResult> SyncNotifier::OnComplete()
    {
    BeMutexHolder lock(m_mutex);
    if (m_tasks.empty())
        return CreateCompletedAsyncTask(SyncResult::Success(ICachingDataSource::SyncStatus::NotSynced));

    return AsyncTask::WhenAll(m_tasks)->Then<SyncResult>([=]
        {
        SyncResult finalResult = (*m_tasks.begin())->GetResult();

        for (auto it = m_tasks.begin()++; it != m_tasks.end(); ++it)
            {
            auto result = (*it)->GetResult();

            if (!result.IsSuccess())
                continue;

            if (result.GetValue() == ICachingDataSource::SyncStatus::Synced)
                return result;

            finalResult = result;
            }
        
        return finalResult;
        });
    }