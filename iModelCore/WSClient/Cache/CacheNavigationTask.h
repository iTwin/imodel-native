/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/CacheNavigationTask.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/CachingDataSource.h>

#include "CachingTaskBase.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct CacheNavigationTask : public CachingTaskBase, public IECDbSchemaChangeListener
    {
    private:
        const bvector<ObjectId> m_navigationTreesToCacheFully;
        const bvector<ObjectId> m_navigationTreesToUpdateOnly;

        const CachingDataSource::LabeledProgressCallback m_onProgressCallback;

        int                     m_recursiveTasksRunning;
        bset<ObjectId>          m_cachedObjects;
        std::queue<ObjectId>    m_objectsToCache;
        bset<ObjectId>          m_filesToDownload;
        bset<ObjectId>          m_objectsToRedownload;
        std::vector<CachedResponseKey> m_partialDataNotModifiedParents;

        std::shared_ptr<const ISelectProvider> m_updateSelectProvider;
        bset<Utf8String>        m_updateProperties;

        uint64_t m_lastTimeReported;
        std::shared_ptr<ECSqlStatementCache> m_statementCache;

    protected:
        virtual void _OnExecute();

        void ContinueCachingChildrenRecursively(bool forceFullRecursiveCaching);
        void CacheNavigationTrees(const bvector<ObjectId>& navigationTrees, bool forceFullRecursiveCaching);
        AsyncTaskPtr<void> CacheChildrenRecursively(ObjectIdCR objectid, bool forceFullRecursiveCaching);
        void ResolveGetChildrenResponse
            (
            WSObjectsResult& result,
            ObjectIdCR parentId,
            std::shared_ptr<bset<ObjectId>> oldCachedChildren,
            bool forceFullRecursiveCaching,
            bool forceUpdateChildren,
            bool retrievingFullData
            );
        void CacheRejectedObjects();
        void MarkNotModifiedChildrenAsPartial();
        void CacheFiles();
        void ReportProgress(ObjectIdCR objectId);
        void ReportProgress(double bytesTransfered, double bytesTotal, Utf8StringCR taskLabel);
        bool IsObjectFileBacked(CacheTransactionCR txn, ECInstanceKeyCR instance);

    public:
        CacheNavigationTask
            (
            CachingDataSourcePtr cachingDataSource,
            bvector<ObjectId>&& navigationTreesToCacheFully,
            bvector<ObjectId>&& navigationTreesToUpdateOnly,
            std::shared_ptr<const ISelectProvider> updateSelectProvider,
            CachingDataSource::LabeledProgressCallback&& onProgress,
            ICancellationTokenPtr ct
            );

        virtual void OnSchemaChanged() override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
