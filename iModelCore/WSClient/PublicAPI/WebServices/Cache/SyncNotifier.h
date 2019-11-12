/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>
#include <WebServices/Cache/ICachingDataSource.h>
#include <Bentley/BeThread.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

typedef AsyncResult<ICachingDataSource::SyncStatus, ICachingDataSource::Error> SyncResult;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                            
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct SyncNotifier> SyncNotifierPtr;
struct SyncNotifier
    {
    private:
        bset<AsyncTaskPtr<SyncResult>> m_tasks;
        BeMutex m_mutex;

    private:
        SyncNotifier() {};

    public:
        WSCACHE_EXPORT static SyncNotifierPtr Create();

        // Add task the list
        WSCACHE_EXPORT void AddTask(AsyncTaskPtr<SyncResult> task);

        // Notify when all tasks tasks are completed
        WSCACHE_EXPORT AsyncTaskPtr<SyncResult> OnComplete();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
