/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Cache/Persistence/DataSourceCache.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

// THIS IS NOT FINALIZED CODE, MAJOR API & CACHE STRUCTURE CHANGES ARE POSSIBLE WITHOUT MUCH CONVERSION SUPPORT //

// Make sure that these are initialized before using DataSourceCache:
//    ECSchemaReadContext::Initialize (schemasPath) override;
//    BeSQLiteLib::Initialize (tempDir) override;

#include <WebServices/Cache/Persistence/IDataSourceCache.h>
#include <WebServices/Cache/Util/ECDbAdapter.h>
#include <WebServices/Cache/Util/ObservableECDb.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    07/2013
* Documentation in IDataSourceCache class.
+---------------+---------------+---------------+---------------+---------------+------*/
struct DataSourceCache : public IDataSourceCache
    {
    private:
        ObservableECDb m_db;
        std::shared_ptr<struct DataSourceCacheOpenState> m_state;

    private:
        BentleyStatus ExecuteWithinTransaction(std::function<BentleyStatus()> execute);

        BentleyStatus InitializeCreatedDb();
        void SetupOpenState(CacheEnvironmentCR environment);
        void ClearRuntimeCaches();

        BentleyStatus UpgradeIfNeeded
            (
            BeFileNameCR cacheFilePath,
            CacheEnvironmentCR environment,
            const ECDb::OpenParams& params
            );

        std::shared_ptr<ECSqlStatement> GetReadInstanceStatement(ECClassCR ecClass, Utf8CP remoteId);
        std::shared_ptr<ECSqlStatement> GetReadInstanceStatement(ECClassCR ecClass, ECInstanceId ecInstanceId);

    public:
        //--------------------------------------------------------------------------------------------------------------------------------+
        //  Misc
        //--------------------------------------------------------------------------------------------------------------------------------+
        WSCACHE_EXPORT DataSourceCache();
        WSCACHE_EXPORT virtual ~DataSourceCache();

        WSCACHE_EXPORT BentleyStatus Create
            (
            BeFileNameCR cacheFilePath,
            CacheEnvironmentCR environment,
            const ECDb::CreateParams& params = ECDb::CreateParams()
            ) override;

        WSCACHE_EXPORT BentleyStatus Open
            (
            BeFileNameCR cacheFilePath,
            CacheEnvironmentCR environment,
            const ECDb::OpenParams& params = ECDb::OpenParams(ECDb::OPEN_ReadWrite)
            ) override;

        WSCACHE_EXPORT BentleyStatus Close() override;

        WSCACHE_EXPORT void RegisterSchemaChangeListener(IECDbSchemaChangeListener* listener) override;
        WSCACHE_EXPORT void UnRegisterSchemaChangeListener(IECDbSchemaChangeListener* listener) override;

        WSCACHE_EXPORT BentleyStatus UpdateSchemas(const std::vector<BeFileName>& schemaPaths) override;
        WSCACHE_EXPORT BentleyStatus UpdateSchemas(const std::vector<ECSchemaPtr>& schemas) override;

        WSCACHE_EXPORT BentleyStatus Reset() override;

        WSCACHE_EXPORT static BentleyStatus DeleteCacheFromDisk(BeFileNameCR cacheFilePath, CacheEnvironmentCR environment);

        WSCACHE_EXPORT BeFileName GetCacheDatabasePath() override;
        WSCACHE_EXPORT IECDbAdapterR GetAdapter() override;
        WSCACHE_EXPORT IExtendedDataAdapter& GetExtendedDataAdapter() override;
        WSCACHE_EXPORT ObservableECDb& GetECDb() override;
        WSCACHE_EXPORT IChangeManagerR GetChangeManager() override;
        WSCACHE_EXPORT ObjectId ObjectIdFromJsonInstance(JsonValueCR instance) const override;

        //--------------------------------------------------------------------------------------------------------------------------------+
        //  Saving data to cache
        //--------------------------------------------------------------------------------------------------------------------------------+

        WSCACHE_EXPORT BentleyStatus CacheResponse
            (
            CachedResponseKeyCR responseKey,
            WSObjectsResponseCR response,
            bset<ObjectId>* rejectedOut = nullptr,
            const WSQuery* query = nullptr,
            uint64_t page = 0,
            ICancellationTokenPtr ct = nullptr
            ) override;

        WSCACHE_EXPORT BentleyStatus CacheInstanceAndLinkToRoot
            (
            ObjectIdCR objectId,
            WSObjectsResponseCR response,
            Utf8StringCR rootName
            ) override;

        WSCACHE_EXPORT BentleyStatus CacheInstanceAndLinkToRoot
            (
            ObjectIdCR objectId,
            RapidJsonValueCR instancePropertiesJson,
            Utf8StringCR instanceCacheTag,
            Utf8StringCR rootName
            ) override;

        WSCACHE_EXPORT BentleyStatus CacheInstancesAndLinkToRoot
            (
            WSObjectsResponseCR response,
            Utf8StringCR rootName,
            ECInstanceKeyMultiMap* cachedInstanceKeysOut = nullptr,
            bool weakLinkToRoot = false,
            ICancellationTokenPtr ct = nullptr
            ) override;

        WSCACHE_EXPORT BentleyStatus UpdateInstance(ObjectIdCR objectId, WSObjectsResponseCR response) override;

        WSCACHE_EXPORT BentleyStatus UpdateInstances
            (
            WSObjectsResponseCR response,
            bset<ObjectId>* notFoundOut = nullptr,
            bset<ECInstanceKey>* cachedInstancesOut = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) override;

        WSCACHE_EXPORT BentleyStatus CacheFile(ObjectIdCR objectId, WSFileResponseCR fileResult, FileCache cacheLocation) override;

        //--------------------------------------------------------------------------------------------------------------------------------+
        //  Reading cached data
        //--------------------------------------------------------------------------------------------------------------------------------+

        WSCACHE_EXPORT CacheStatus ReadResponse
            (
            CachedResponseKeyCR responseKey,
            JsonValueR instancesOut,
            ISelectProviderCR selectProvider = ISelectProvider()
            ) override;

        WSCACHE_EXPORT CacheStatus ReadResponseInstanceKeys
            (
            CachedResponseKeyCR responseKey,
            ECInstanceKeyMultiMap& instanceKeysOut
            ) override;

        WSCACHE_EXPORT CacheStatus ReadResponseObjectIds
            (
            CachedResponseKeyCR responseKey,
            bset<ObjectId>& instanceObjectIdsOut
            ) override;

        WSCACHE_EXPORT CacheStatus ReadInstance(ObjectIdCR objectId, JsonValueR instanceDataOut, JsonFormat format = JsonFormat::Raw) override;
        WSCACHE_EXPORT IECInstancePtr ReadInstance(ObjectIdCR objectId) override;
        WSCACHE_EXPORT IECInstancePtr ReadInstance(ECInstanceKeyCR instanceKey) override;

        WSCACHE_EXPORT BentleyStatus ReadInstances
            (
            const bset<ObjectId>& ids,
            JsonValueR instancesArrayOut,
            ISelectProviderCR selectProvider = ISelectProvider()
            ) override;

        WSCACHE_EXPORT Utf8String ReadInstanceLabel(ObjectIdCR objectId) override;

        WSCACHE_EXPORT BeFileName ReadFilePath(ObjectIdCR objectId) override;
        WSCACHE_EXPORT BeFileName ReadFilePath(ECInstanceKeyCR instanceKey) override;
        WSCACHE_EXPORT BentleyStatus ReadFileProperties(ECInstanceKeyCR instanceKey, Utf8String* fileName, uint64_t* fileSize) override;

        WSCACHE_EXPORT bool IsResponseCached(CachedResponseKeyCR responseKey) override;

        WSCACHE_EXPORT Utf8String ReadResponseCacheTag(CachedResponseKeyCR responseKey, uint64_t page = 0) override;
        WSCACHE_EXPORT Utf8String ReadInstanceCacheTag(ObjectIdCR objectId) override;
        WSCACHE_EXPORT Utf8String ReadFileCacheTag(ObjectIdCR objectId) override;

        WSCACHE_EXPORT DateTime ReadResponseCachedDate(CachedResponseKeyCR responseKey, uint64_t page = 0) override;
        WSCACHE_EXPORT DateTime ReadInstanceCachedDate(ObjectIdCR objectId) override;
        WSCACHE_EXPORT DateTime ReadFileCachedDate(ObjectIdCR objectId) override;

        WSCACHE_EXPORT BentleyStatus SetResponseAccessDate(CachedResponseKeyCR responseKey, DateTimeCR utcDateTime = DateTime::GetCurrentTimeUtc()) override;
        WSCACHE_EXPORT DateTime ReadResponseAccessDate(CachedResponseKeyCR responseKey) override;

        WSCACHE_EXPORT CachedObjectInfo GetCachedObjectInfo(ECInstanceKeyCR instance) override;
        WSCACHE_EXPORT CachedObjectInfo GetCachedObjectInfo(ObjectIdCR objectId) override;

        WSCACHE_EXPORT ECInstanceKey FindInstance(ObjectIdCR objectId) override;
        WSCACHE_EXPORT ObjectId FindInstance(ECInstanceKeyCR instanceKey) override;

        WSCACHE_EXPORT ECInstanceKey FindRelationship(ECRelationshipClassCR relClass, ECInstanceKeyCR source, ECInstanceKeyCR target) override;
        WSCACHE_EXPORT ECInstanceKey FindRelationship(ECRelationshipClassCR relClass, ObjectIdCR source, ObjectIdCR target) override;
        WSCACHE_EXPORT ObjectId FindRelationship(ECInstanceKeyCR relationshipKey) override;

        //--------------------------------------------------------------------------------------------------------------------------------+
        //  Removing cached data
        //--------------------------------------------------------------------------------------------------------------------------------+

        WSCACHE_EXPORT BentleyStatus RemoveResponse(CachedResponseKeyCR responseKey) override;
        WSCACHE_EXPORT BentleyStatus RemoveTemporaryResponses(Utf8StringCR name, DateTimeCR accessedBeforeDateUtc) override;
        WSCACHE_EXPORT BentleyStatus RemoveResponses(Utf8StringCR name) override;
        WSCACHE_EXPORT CacheStatus RemoveInstance(ObjectIdCR objectId) override;
        WSCACHE_EXPORT BentleyStatus RemoveFile(ObjectIdCR objectId) override;
        WSCACHE_EXPORT BentleyStatus RemoveFilesInTemporaryPersistence(DateTimeCP maxLastAccessDate = nullptr) override;
        WSCACHE_EXPORT BentleyStatus RemoveRoot(Utf8StringCR rootName) override;
        WSCACHE_EXPORT BentleyStatus RemoveRootsByPrefix(Utf8StringCR rootPrefix) override;

        //--------------------------------------------------------------------------------------------------------------------------------+
        //  Working with roots
        //--------------------------------------------------------------------------------------------------------------------------------+

        WSCACHE_EXPORT bool DoesRootExist(Utf8StringCR rootName) override;
        WSCACHE_EXPORT BentleyStatus SetupRoot(Utf8StringCR rootName, CacheRootPersistence persistence) override;
        WSCACHE_EXPORT ECInstanceKey FindOrCreateRoot(Utf8StringCR rootName) override;
        WSCACHE_EXPORT BentleyStatus RenameRoot(Utf8StringCR rootName, Utf8StringCR newRootName) override;

        WSCACHE_EXPORT BentleyStatus SetRootSyncDate(Utf8StringCR rootName, DateTimeCR utcDateTime = DateTime::GetCurrentTimeUtc()) override;
        WSCACHE_EXPORT DateTime ReadRootSyncDate(Utf8StringCR rootName) override;

        WSCACHE_EXPORT BentleyStatus LinkInstanceToRoot(Utf8StringCR rootName, ObjectIdCR objectId) override;
        WSCACHE_EXPORT BentleyStatus UnlinkInstanceFromRoot(Utf8StringCR rootName, ObjectIdCR objectId) override;
        WSCACHE_EXPORT BentleyStatus UnlinkInstanceFromRoot(Utf8StringCR rootName, ECInstanceKey instance);
        WSCACHE_EXPORT BentleyStatus UnlinkAllInstancesFromRoot(Utf8StringCR rootName) override;

        WSCACHE_EXPORT bool IsInstanceInRoot(Utf8StringCR rootName, ECInstanceKeyCR instance) override;
        WSCACHE_EXPORT bool IsInstanceConnectedToRoot(Utf8StringCR rootName, ECInstanceKeyCR instance) override;

        WSCACHE_EXPORT BentleyStatus ReadInstancesLinkedToRoot
            (
            Utf8StringCR rootName,
            JsonValueR instancesOut,
            ISelectProviderCR selectProvider = ISelectProvider()
            ) override;

        WSCACHE_EXPORT BentleyStatus ReadInstancesLinkedToRoot
            (
            Utf8StringCR rootName,
            ECInstanceKeyMultiMap& instanceMap
            ) override;

        WSCACHE_EXPORT BentleyStatus ReadInstancesConnectedToRootMap
            (
            Utf8StringCR rootName,
            ECInstanceKeyMultiMap& instancesOut,
            uint8_t depth = UINT8_MAX
            ) override;

        //--------------------------------------------------------------------------------------------------------------------------------+
        //  Instance persistence
        //--------------------------------------------------------------------------------------------------------------------------------+

        WSCACHE_EXPORT bool IsInstanceFullyPersisted(ObjectIdCR objectId) override;
        WSCACHE_EXPORT bool IsInstanceFullyPersisted(ECInstanceKeyCR instanceKey) override;
        WSCACHE_EXPORT BentleyStatus ReadFullyPersistedInstanceKeys(ECInstanceKeyMultiMap& instancesOut) override;
        WSCACHE_EXPORT BentleyStatus MarkTemporaryInstancesAsPartial(const std::vector<CachedResponseKey>& resultsKeys) override;

        //--------------------------------------------------------------------------------------------------------------------------------+
        //  Cached file managment
        //--------------------------------------------------------------------------------------------------------------------------------+

        WSCACHE_EXPORT BentleyStatus SetFileCacheLocation(ObjectIdCR objectId, FileCache cacheLocation, BeFileNameCR externalRelativeDir = BeFileName()) override;
        WSCACHE_EXPORT FileCache     GetFileCacheLocation(ObjectIdCR objectId, FileCache defaultLocation = FileCache::Temporary) override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
