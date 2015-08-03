/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/Persistence/MockDataSourceCache.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <WebServices/Cache/Persistence/IDataSourceCache.h>

#include "MockChangeManager.h"
#include "../Util/MockECDbAdapter.h"

#ifdef USE_GTEST
BEGIN_BENTLEY_WEBSERVICES_NAMESPACE
using namespace ::testing;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockDataSourceCache : public IDataSourceCache
    {
    private:
        NiceMock<MockChangeManager> m_changeManager;
        MockECDbAdapter m_adapter;

    public:
        MockDataSourceCache ()
            {
            ON_CALL (*this, GetChangeManager ()).WillByDefault (ReturnRef (m_changeManager));
            ON_CALL (*this, GetAdapter ()).WillByDefault (ReturnRef (m_adapter));
            }

        MockChangeManager& GetChangeManagerMock ()
            {
            return m_changeManager;
            }

        MockECDbAdapter& GetAdapterMock ()
            {
            return m_adapter;
            }

        MOCK_METHOD3 (Create,
            BentleyStatus (BeFileNameCR cacheFilePath, CacheEnvironmentCR environment, const ECDb::CreateParams& params));
        MOCK_METHOD3 (Open,
            BentleyStatus (BeFileNameCR cacheFilePath, CacheEnvironmentCR environment, const ECDb::OpenParams& params));
        MOCK_METHOD0 (Close,
            BentleyStatus ());
        MOCK_METHOD1 (RegisterSchemaChangeListener,
            void (IECDbSchemaChangeListener* listener));
        MOCK_METHOD1 (UnRegisterSchemaChangeListener,
            void (IECDbSchemaChangeListener* listener));
        MOCK_METHOD1 (UpdateSchemas,
            BentleyStatus (const std::vector<BeFileName>& schemaPaths));
        MOCK_METHOD1 (UpdateSchemas,
            BentleyStatus (const std::vector<ECSchemaPtr>& schemas));
        MOCK_METHOD0 (Reset,
            BentleyStatus ());
        MOCK_METHOD0 (GetCacheDatabasePath,
            BeFileName ());
        MOCK_METHOD0 (GetAdapter,
            IECDbAdapterR ());
        MOCK_METHOD0 (GetExtendedDataAdapter,
            IExtendedDataAdapter& ());
        MOCK_METHOD0 (GetECDb,
            ObservableECDb& ());
        MOCK_METHOD0 (GetChangeManager,
            IChangeManagerR ());
        MOCK_CONST_METHOD1 (ObjectIdFromJsonInstance,
            ObjectId (JsonValueCR instance));
        MOCK_METHOD3 (CacheResponse,
            BentleyStatus (CachedResponseKeyCR responseKey, WSObjectsResponseCR response, ICancellationTokenPtr));
        MOCK_METHOD5 (CachePartialResponse,
            BentleyStatus (CachedResponseKeyCR responseKey, WSObjectsResponseCR response, bset<ObjectId>& rejectedOut, const WSQuery* query, ICancellationTokenPtr));
        MOCK_METHOD3 (CacheInstanceAndLinkToRoot,
            BentleyStatus (ObjectIdCR objectId, WSObjectsResponseCR response, Utf8StringCR rootName));
        MOCK_METHOD4 (CacheInstanceAndLinkToRoot,
            BentleyStatus (ObjectIdCR objectId, RapidJsonValueCR instancePropertiesJson, Utf8StringCR instanceCacheTag, Utf8StringCR rootName));
        MOCK_METHOD5 (CacheInstancesAndLinkToRoot,
            BentleyStatus (WSObjectsResponseCR, Utf8StringCR, ECInstanceKeyMultiMap*, bool, ICancellationTokenPtr));
        MOCK_METHOD2 (UpdateInstance,
            BentleyStatus (ObjectIdCR objectId, WSObjectsResponseCR response));
        MOCK_METHOD4 (UpdateInstances,
            BentleyStatus (WSObjectsResponseCR, bset<ObjectId>*, bset<ECInstanceKey>*, ICancellationTokenPtr));
        MOCK_METHOD3 (CacheFile,
            BentleyStatus (ObjectIdCR objectId, WSFileResponseCR fileResult, FileCache cacheLocation));
        MOCK_METHOD3 (ReadResponse,
            CacheStatus (CachedResponseKeyCR, JsonValueR, ISelectProviderCR));
        MOCK_METHOD2 (ReadResponseInstanceKeys,
            CacheStatus (CachedResponseKeyCR responseKey, ECInstanceKeyMultiMap& instanceKeysOut));
        MOCK_METHOD2 (ReadResponseObjectIds,
            CacheStatus (CachedResponseKeyCR responseKey, bset<ObjectId>& instanceObjectIdsOut));
        MOCK_METHOD3 (ReadInstance,
            CacheStatus (ObjectIdCR, JsonValueR, JsonFormat));
        MOCK_METHOD1 (ReadInstance,
            IECInstancePtr (ObjectIdCR));
        MOCK_METHOD1 (ReadInstance,
            IECInstancePtr (ECInstanceKeyCR));
        MOCK_METHOD3 (ReadInstances,
            BentleyStatus (const bset<ObjectId>&, JsonValueR, ISelectProviderCR));
        MOCK_METHOD3 (ReadInstancesLinkedToRoot,
            BentleyStatus (Utf8StringCR, JsonValueR, ISelectProviderCR));
        MOCK_METHOD2 (ReadInstancesLinkedToRoot,
            BentleyStatus (Utf8StringCR rootName, ECInstanceKeyMultiMap& instanceMap));
        MOCK_METHOD1 (ReadInstanceLabel,
            Utf8String (ObjectIdCR objectId));
        MOCK_METHOD1 (ReadFilePath,
            BeFileName (ObjectIdCR objectId));
        MOCK_METHOD1 (ReadFilePath,
            BeFileName (ECInstanceKeyCR instanceKey));
        MOCK_METHOD3 (ReadFileProperties,
            BentleyStatus (ECInstanceKeyCR instanceKey, Utf8StringR fileName, uint64_t& fileSize));
        MOCK_METHOD1 (IsResponseCached,
            bool (CachedResponseKeyCR responseKey));
        MOCK_METHOD1 (ReadResponseCacheTag,
            Utf8String (CachedResponseKeyCR responseKey));
        MOCK_METHOD1 (ReadInstanceCacheTag,
            Utf8String (ObjectIdCR objectId));
        MOCK_METHOD1 (ReadFileCacheTag,
            Utf8String (ObjectIdCR objectId));
        MOCK_METHOD1 (ReadResponseCachedDate,
            DateTime (CachedResponseKeyCR responseKey));
        MOCK_METHOD1 (ReadInstanceCachedDate,
            DateTime (ObjectIdCR objectId));
        MOCK_METHOD1 (ReadFileCachedDate,
            DateTime (ObjectIdCR objectId));
        MOCK_METHOD2 (SetResponseAccessDate,
            BentleyStatus (CachedResponseKeyCR, DateTimeCR));
        MOCK_METHOD1 (ReadResponseAccessDate,
            DateTime (CachedResponseKeyCR));
        MOCK_METHOD1 (GetCachedObjectInfo,
            CachedObjectInfo (ECInstanceKeyCR instance));
        MOCK_METHOD1 (GetCachedObjectInfo,
            CachedObjectInfo (ObjectIdCR objectId));
        MOCK_METHOD1 (FindInstance,
            ECInstanceKey (ObjectIdCR objectId));
        MOCK_METHOD1 (FindInstance,
            ObjectId (ECInstanceKeyCR instanceKey));
        MOCK_METHOD3 (FindRelationship,
            ECInstanceKey (ECRelationshipClassCR, ECInstanceKeyCR, ECInstanceKeyCR));
        MOCK_METHOD3 (FindRelationship,
            ECInstanceKey (ECRelationshipClassCR, ObjectIdCR, ObjectIdCR));
        MOCK_METHOD1 (FindRelationship,
            ObjectId (ECInstanceKeyCR));
        MOCK_METHOD1 (RemoveResponse,
            BentleyStatus (CachedResponseKeyCR responseKey));
        MOCK_METHOD2 (RemoveTemporaryResponses,
            BentleyStatus (Utf8StringCR name, DateTimeCR accessedBeforeDateUtc));
        MOCK_METHOD1 (RemoveResponses,
            BentleyStatus (Utf8StringCR name));
        MOCK_METHOD1 (RemoveInstance,
            CacheStatus (ObjectIdCR objectId));
        MOCK_METHOD1 (RemoveFile,
            BentleyStatus (ObjectIdCR objectId));
        MOCK_METHOD0 (RemoveFilesInTemporaryPersistence,
            BentleyStatus ());
        MOCK_METHOD1 (RemoveRoot,
            BentleyStatus (Utf8StringCR rootName));
        MOCK_METHOD1 (RemoveRootsByPrefix,
            BentleyStatus (Utf8StringCR rootPrefix));
        MOCK_METHOD1 (DoesRootExist,
            bool (Utf8StringCR rootName));
        MOCK_METHOD2 (SetupRoot,
            BentleyStatus (Utf8StringCR rootName, CacheRootPersistence persistence));
        MOCK_METHOD1 (FindOrCreateRoot,
            ECInstanceKey (Utf8StringCR rootName));
        MOCK_METHOD2 (RenameRoot,
            BentleyStatus (Utf8StringCR rootName, Utf8StringCR newRootName));
        MOCK_METHOD2 (SetRootSyncDate,
            BentleyStatus (Utf8StringCR rootName, DateTimeCR utcDate));
        MOCK_METHOD1 (ReadRootSyncDate,
            DateTime (Utf8StringCR rootName));
        MOCK_METHOD2 (LinkInstanceToRoot,
            BentleyStatus (Utf8StringCR rootName, ObjectIdCR objectId));
        MOCK_METHOD2 (UnlinkInstanceFromRoot,
            BentleyStatus (Utf8StringCR rootName, ObjectIdCR objectId));
        MOCK_METHOD1 (UnlinkAllInstancesFromRoot,
            BentleyStatus (Utf8StringCR rootName));
        MOCK_METHOD2 (IsInstanceInRoot,
            bool (Utf8StringCR rootName, ECInstanceKeyCR instance));
        MOCK_METHOD2 (IsInstanceConnectedToRoot,
            bool (Utf8StringCR rootName, ECInstanceKeyCR instance));
        MOCK_METHOD3 (ReadInstancesConnectedToRootMap,
            BentleyStatus (Utf8StringCR, ECInstanceKeyMultiMap&, uint8_t));
        MOCK_METHOD1 (IsInstanceFullyPersisted,
            bool (ObjectIdCR objectId));
        MOCK_METHOD1 (IsInstanceFullyPersisted,
            bool (ECInstanceKeyCR instanceKey));
        MOCK_METHOD1 (ReadFullyPersistedInstanceKeys,
            BentleyStatus (ECInstanceKeyMultiMap& instancesOut));
        MOCK_METHOD1 (MarkTemporaryInstancesAsPartial,
            BentleyStatus (const std::vector<CachedResponseKey>& resultsKeys));
        MOCK_METHOD2 (SetFileCacheLocation,
            BentleyStatus (const bvector<ObjectId>& ids, FileCache cacheLocation));
        MOCK_METHOD2 (SetFileCacheLocation,
            BentleyStatus (ObjectIdCR objectId, FileCache cacheLocation));
        MOCK_METHOD1 (GetFileCacheLocation,
            FileCache (ObjectIdCR objectId));
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
#endif