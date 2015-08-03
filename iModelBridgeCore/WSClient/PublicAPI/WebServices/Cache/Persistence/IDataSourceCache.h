/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Cache/Persistence/IDataSourceCache.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

// THIS IS NOT FINALIZED CODE, MAJOR API & CACHE STRUCTURE CHANGES ARE POSSIBLE WITHOUT MUCH CONVERSION SUPPORT //

#include <WebServices/Cache/WebServicesCache.h>
#include <WebServices/Cache/Persistence/DataSourceCacheCommon.h>

#include <WebServices/Cache/Persistence/CachedObjectInfo.h>
#include <WebServices/Cache/Persistence/CachedResponseKey.h>
#include <WebServices/Cache/Persistence/CacheEnvironment.h>
#include <WebServices/Cache/Persistence/CacheQueryHelper.h>
#include <WebServices/Cache/Persistence/IChangeManager.h>
#include <WebServices/Cache/Util/ECDbHelper.h>
#include <WebServices/Cache/Util/IECDbAdapter.h>
#include <WebServices/Cache/Util/IExtendedDataAdapter.h>
#include <WebServices/Cache/Util/ISelectProvider.h>
#include <WebServices/Cache/Util/ObservableECDb.h>
#include <WebServices/Client/ObjectId.h>
#include <WebServices/Client/Response/WSFileResponse.h>
#include <WebServices/Client/Response/WSObjectsResponse.h>
#include <WebServices/Client/WSQuery.h>

#include <ECDb/ECDbApi.h>
#include <ECObjects/ECObjects.h>

USING_NAMESPACE_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
enum class CacheStatus
    {
    OK = 0,
    Error = 1,
    DataNotCached = 2
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE IDataSourceCache
    {
    public:
        enum class JsonFormat
            {
            Raw,
            Display
            };

    public:
        virtual ~IDataSourceCache ()
            {
            };

        //--------------------------------------------------------------------------------------------------------------------------------+
        //  Misc
        //--------------------------------------------------------------------------------------------------------------------------------+
        virtual BentleyStatus Create (BeFileNameCR cacheFilePath, CacheEnvironmentCR environment, const ECDb::CreateParams& params) = 0;
        virtual BentleyStatus Open (BeFileNameCR cacheFilePath, CacheEnvironmentCR environment, const ECDb::OpenParams& params) = 0;
        virtual BentleyStatus Close () = 0;

        //! Register for ECSchema change events. 
        //! Old ECSchema, ECClass, ECSqlStatement and other resource references must be removed when schema changes.
        virtual void RegisterSchemaChangeListener (IECDbSchemaChangeListener* listener) = 0;

        //! Unregister from ECSchema change events.
        virtual void UnRegisterSchemaChangeListener (IECDbSchemaChangeListener* listener) = 0;

        //! Import new or update existing schemas from disk
        virtual BentleyStatus UpdateSchemas (const std::vector<BeFileName>& schemaPaths) = 0;

        //! Import new or update existing schemas
        virtual BentleyStatus UpdateSchemas (const std::vector<ECSchemaPtr>& schemas) = 0;

        //! Clears all cached data and returns to created state. Keeps schemas.
        virtual BentleyStatus Reset () = 0;

        //! Returns path to sqlite database
        virtual BeFileName GetCacheDatabasePath () = 0;

        //! Get IECDbAdapter for accessing common functionality
        virtual IECDbAdapterR GetAdapter () = 0;

        //! Get IExtendedDataAdapter for accessing extended data for instances
        virtual IExtendedDataAdapter& GetExtendedDataAdapter () = 0;

        //! Get ECDb for accessing raw database
        virtual ObservableECDb& GetECDb () = 0;

        //! Get ChangeManager to make local changes to be synced to server
        virtual IChangeManagerR GetChangeManager () = 0;

        //! Extract ObjectId from JSON instance
        virtual ObjectId ObjectIdFromJsonInstance (JsonValueCR instance) const = 0;

        //--------------------------------------------------------------------------------------------------------------------------------+
        //  Saving data to cache
        //--------------------------------------------------------------------------------------------------------------------------------+

        //! Saves query response to cache
        //! @param[in] responseKey - key used to store response data
        //! @param[in] response - contains information about instances
        //! @param[in] cancellationToken - if supplied and canceled, will return ERROR and caller is responsible for rollbacking transaction
        virtual BentleyStatus CacheResponse
            (
            CachedResponseKeyCR responseKey,
            WSObjectsResponseCR response,
            ICancellationTokenPtr cancellationToken = nullptr
            ) = 0;

        //! Saves query response to cache that include instances with subset of properties
        //! @param[in] responseKey - key used to store response data
        //! @param[in] response - contains information about instances
        //! @param[out] rejectedOut - returns ObjectIds that are protected by full persistence and were not updated with partial instances.
        //! @param[in] query - when specified, is used to determine instances that are partial. Null results in all instances treated as partial.
        //! @param[in] cancellationToken - if supplied and canceled, will return ERROR and caller is responsible for rollbacking transaction
        //! ATTENTION: when response.IsModified() == false, MarkTemporaryInstancesAsPartial should be called
        virtual BentleyStatus CachePartialResponse
            (
            CachedResponseKeyCR responseKey,
            WSObjectsResponseCR response,
            bset<ObjectId>& rejectedOut,
            const WSQuery* query = nullptr,
            ICancellationTokenPtr cancellationToken = nullptr
            ) = 0;

        //! Cache instances and link to specified root
        virtual BentleyStatus CacheInstanceAndLinkToRoot
            (
            ObjectIdCR objectId,
            WSObjectsResponseCR response,
            Utf8StringCR rootName
            ) = 0;

        //! Cache instance and link to specified root
        virtual BentleyStatus CacheInstanceAndLinkToRoot
            (
            ObjectIdCR objectId,
            RapidJsonValueCR instancePropertiesJson,
            Utf8StringCR instanceCacheTag,
            Utf8StringCR rootName
            ) = 0;

        //! Notes when weakLinkToRoot == true: 
        //! This is Experimental limited API! 
        //! Relates instances to root with referancing relationship. 
        //!     These instances will stay in cache as long as they are not put in other hierarchy that will be removed
        //!     WeakLinked instances will not work with ReadInstancesLinkedToRoot, ReadInstancesConnectedToRootMap and simmilar methods.
        //!     Only RemoveRoot will work with these instances and will remove them from cache if they are not held by hierarchies.
        virtual BentleyStatus CacheInstancesAndLinkToRoot
            (
            WSObjectsResponseCR response,
            Utf8StringCR rootName,
            ECInstanceKeyMultiMap* cachedInstanceKeysOut = nullptr,
            bool weakLinkToRoot = false,
            ICancellationTokenPtr cancellationToken = nullptr
            ) = 0;

        //! Updates existing instance. instanceResult.IsModified() == false will result in updating cached date
        virtual BentleyStatus UpdateInstance (ObjectIdCR objectId, WSObjectsResponseCR response) = 0;

        //! Updates existing instances. New instances will be placed in notFoundOut. Not modified response will result in error.
        virtual BentleyStatus UpdateInstances
            (
            WSObjectsResponseCR response,
            bset<ObjectId>* notFoundOut = nullptr,
            bset<ECInstanceKey>* cachedInstancesOut = nullptr,
            ICancellationTokenPtr cancellationToken = nullptr
            ) = 0;

        virtual BentleyStatus CacheFile (ObjectIdCR objectId, WSFileResponseCR fileResult, FileCache cacheLocation) = 0;

        //--------------------------------------------------------------------------------------------------------------------------------+
        //  Reading cached data
        //--------------------------------------------------------------------------------------------------------------------------------+

        //! Returns JSON array of instances cached as query response. resultsOut is array of EC Json instances.
        virtual CacheStatus ReadResponse
            (
            CachedResponseKeyCR responseKey,
            JsonValueR instancesOut,
            ISelectProviderCR selectProvider = ISelectProvider ()
            ) = 0;

        //! Read instance keys of cached response instances
        //! NOTE - if response contains multiple cases of same instance in it, this will return only one instance without duplicates.
        virtual CacheStatus ReadResponseInstanceKeys
            (
            CachedResponseKeyCR responseKey,
            ECInstanceKeyMultiMap& instanceKeysOut
            ) = 0;

        //! Returns ObjectIds of instances cached as query response
        virtual CacheStatus ReadResponseObjectIds
            (
            CachedResponseKeyCR responseKey,
            bset<ObjectId>& instanceObjectIdsOut
            ) = 0;

        //! Read once cached instance
        //! format == JsonFormat::Raw - gets flat EC Json instance with additional members:
        //!     DataSourceCache_PROPERTY_RemoteId       - remote id string
        //!     DataSourceCache_PROPERTY_ClassKey       - class key string - "SchemaName.ClassName"
        //! format == JsonFormat::Display - gets display Json with members:
        //!     DataSourceCache_PROPERTY_RemoteId       - remote id string
        //!     DataSourceCache_PROPERTY_ClassKey       - class key string - "SchemaName.ClassName"
        //!     DataSourceCache_PROPERTY_DisplayInfo    - ECDb ColumnHeaders Json
        //!     DataSourceCache_PROPERTY_DisplayData    - flat ECDb Json with values formatted for display
        //!     DataSourceCache_PROPERTY_RawData        - flat ECDb Json with raw values
        virtual CacheStatus   ReadInstance (ObjectIdCR objectId, JsonValueR instanceDataOut, JsonFormat format = JsonFormat::Raw) = 0;

        //! Read instance with remoteId from cache. Will return nullptr if not found or error occurred.
        virtual IECInstancePtr ReadInstance (ObjectIdCR objectId) = 0;

        //! Read instance with remoteId from cache. Will return nullptr if not found or error occurred.
        virtual IECInstancePtr ReadInstance (ECInstanceKeyCR instanceKey) = 0;

        //! Read instances by their ObjectIds
        virtual BentleyStatus ReadInstances
            (
            const bset<ObjectId>& ids,
            JsonValueR instancesArrayOut,
            ISelectProviderCR selectProvider = ISelectProvider ()
            ) = 0;

        //! Returns cached object label if class supports it or empty string otherwise
        virtual Utf8String ReadInstanceLabel (ObjectIdCR objectId) = 0;

        //! Returns cached file path or empty path if not found
        virtual BeFileName ReadFilePath (ObjectIdCR objectId) = 0;
        //! Returns cached file path or empty path if not found
        virtual BeFileName ReadFilePath (ECInstanceKeyCR instanceKey) = 0;
        //! Read main file properties from cached instance.
        //! @param instanceKey
        //! @param[out] fileName will be filled with file name or instance label if found.
        //! @param[out] fileSize will be filled with file size property value or 0 if not found.
        virtual BentleyStatus ReadFileProperties (ECInstanceKeyCR instanceKey, Utf8StringR fileName, uint64_t& fileSize) = 0;

        virtual bool IsResponseCached (CachedResponseKeyCR responseKey) = 0;

        virtual Utf8String ReadResponseCacheTag (CachedResponseKeyCR responseKey) = 0;
        virtual Utf8String ReadInstanceCacheTag (ObjectIdCR objectId) = 0;
        virtual Utf8String ReadFileCacheTag (ObjectIdCR objectId) = 0;

        virtual DateTime ReadResponseCachedDate (CachedResponseKeyCR responseKey) = 0;
        virtual DateTime ReadInstanceCachedDate (ObjectIdCR objectId) = 0;
        virtual DateTime ReadFileCachedDate (ObjectIdCR objectId) = 0;

        //! Set access date for response. Used for application purposes. Can be used in conjuntion with RemoveTemporaryResponses
        virtual BentleyStatus SetResponseAccessDate (CachedResponseKeyCR responseKey, DateTimeCR utcDateTime = DateTime::GetCurrentTimeUtc ()) = 0;
        virtual DateTime ReadResponseAccessDate (CachedResponseKeyCR responseKey) = 0;

        //! Get cached object information
        virtual CachedObjectInfo GetCachedObjectInfo (ECInstanceKeyCR instance) = 0;
        //! Get cached object information
        virtual CachedObjectInfo GetCachedObjectInfo (ObjectIdCR objectId) = 0;

        //! Get cached instance ECInstanceKey for given ObjectId.
        virtual ECInstanceKey FindInstance (ObjectIdCR objectId) = 0;
        //! Get ObjectId for cached instance.
        virtual ObjectId FindInstance (ECInstanceKeyCR instanceKey) = 0;

        //! Get ECInstanceKey for cached relationship.
        virtual ECInstanceKey FindRelationship (ECRelationshipClassCR relClass, ECInstanceKeyCR source, ECInstanceKeyCR target) = 0;
        //! Get ECInstanceKey for cached relationship.
        virtual ECInstanceKey FindRelationship (ECRelationshipClassCR relClass, ObjectIdCR source, ObjectIdCR target) = 0;
        //! Get ObjectId for cached relationship. Note that ObjectId may not be unique for cached relationships.
        virtual ObjectId FindRelationship (ECInstanceKeyCR relationshipKey) = 0;

        //--------------------------------------------------------------------------------------------------------------------------------+
        //  Removing cached data
        //--------------------------------------------------------------------------------------------------------------------------------+

        //! Removes cached query and any response that are not held by anything else
        virtual BentleyStatus RemoveResponse (CachedResponseKeyCR responseKey) = 0;
        //! Removes temporary cached responses that have no date accessed or it is older than date specified
        virtual BentleyStatus RemoveTemporaryResponses (Utf8StringCR name, DateTimeCR accessedBeforeDateUtc) = 0;
        //! Removes all cached responses that match name
        virtual BentleyStatus RemoveResponses (Utf8StringCR name) = 0;
        //! Removes cached instance. Will also remove all held resources
        virtual CacheStatus RemoveInstance (ObjectIdCR objectId) = 0;
        //! Removes cached file from disk
        virtual BentleyStatus RemoveFile (ObjectIdCR objectId) = 0;
        //! Removes files that are not linked to Full persistence roots. See SetupRoot for more info
        virtual BentleyStatus RemoveFilesInTemporaryPersistence () = 0;
        //! Removes root and deletes linked instances that are not held by other roots
        virtual BentleyStatus RemoveRoot (Utf8StringCR rootName) = 0;
        //! Removes roots by prefix and deletes linked instances that are not held by other roots
        virtual BentleyStatus RemoveRootsByPrefix (Utf8StringCR rootPrefix) = 0;

        //--------------------------------------------------------------------------------------------------------------------------------+
        //  Working with roots
        //  Roots are nodes that keep their linked instances persistent. Every saved object in cache must have its ansestor as root.
        //  Root name is unique identifier for root. It should not contain whitespace characters.
        //--------------------------------------------------------------------------------------------------------------------------------+

        //! Return true if root is already exists in cache
        virtual bool DoesRootExist (Utf8StringCR rootName) = 0;

        //! Optionally setup root for caching
        //! @param[in] rootName
        //! @param[in] persistence default is Full. It marks all instance tree underneath as fully persistent. 
        //!   This allows managing files and instance data more easily.
        //!   Full persistence will also notify cache that instances cannot be be overwritten with partial data.
        virtual BentleyStatus SetupRoot (Utf8StringCR rootName, CacheRootPersistence persistence) = 0;

        //! Return new or existing root ECInstanceKey
        virtual ECInstanceKey FindOrCreateRoot (Utf8StringCR rootName) = 0;

        //! Rename existing root. Will fail if root with same name already exists. Will create new root of it does not exist.
        virtual BentleyStatus RenameRoot (Utf8StringCR rootName, Utf8StringCR newRootName) = 0;

        //! Set sync date for root. Used to mark when instances in root were synced for application purposes
        virtual BentleyStatus SetRootSyncDate (Utf8StringCR rootName, DateTimeCR utcDateTime = DateTime::GetCurrentTimeUtc ()) = 0;

        //! Read sync date for root for application purposes
        virtual DateTime ReadRootSyncDate (Utf8StringCR rootName) = 0;

        //! Link cached or not yet cached object to existing or new root. Will create placeholder instance if it was not cached before.
        virtual BentleyStatus LinkInstanceToRoot (Utf8StringCR rootName, ObjectIdCR objectId) = 0;

        //! Require to unlink instance from root. Will succeed if root does not exist or instance is not in root.
        virtual BentleyStatus UnlinkInstanceFromRoot (Utf8StringCR rootName, ObjectIdCR objectId) = 0;
        //! Require to unlink all instances from root. Will succeed if root does not exist.
        virtual BentleyStatus UnlinkAllInstancesFromRoot (Utf8StringCR rootName) = 0;

        //! Check if given instance is linked to root directly
        virtual bool IsInstanceInRoot (Utf8StringCR rootName, ECInstanceKeyCR instance) = 0;
        //! Check if given instance is held by specific root. Use ReadInstancesConnectedToRootMap for multiple instances.
        virtual bool IsInstanceConnectedToRoot (Utf8StringCR rootName, ECInstanceKeyCR instance) = 0;
        //! Read instances (in JSON format) that were linked to specific root
        virtual BentleyStatus ReadInstancesLinkedToRoot
            (
            Utf8StringCR rootName,
            JsonValueR instancesOut,
            ISelectProviderCR selectProvider = ISelectProvider ()
            ) = 0;
        //! Read instances keys that were linked to specific root
        virtual BentleyStatus ReadInstancesLinkedToRoot (Utf8StringCR rootName, ECInstanceKeyMultiMap& instanceMap) = 0;
        //! Get all instances connected to root. More efficient way for checking if multiple instances are held by root
        virtual BentleyStatus ReadInstancesConnectedToRootMap (Utf8StringCR rootName, ECInstanceKeyMultiMap& instancesOut, uint8_t depth = UINT8_MAX) = 0;

        //--------------------------------------------------------------------------------------------------------------------------------+
        //  Instance persistence
        //--------------------------------------------------------------------------------------------------------------------------------+

        //! Returns true if instance is held by FulPersistence root
        virtual bool IsInstanceFullyPersisted (ObjectIdCR objectId) = 0;

        //! Returns true if instance is held by FulPersistence root
        virtual bool IsInstanceFullyPersisted (ECInstanceKeyCR instanceKey) = 0;

        virtual BentleyStatus ReadFullyPersistedInstanceKeys (ECInstanceKeyMultiMap& instancesOut) = 0;

        //! Finds all Temporary persistence instances and changes their status from Full to Partial.
        //! NOTE: Call once for multiple resultKeys for better performance.
        //! Originally added in changeset: 
        //! "[MobileUtils Caching] BF: D-132877. Children eTag checking does not werify that fully cached instances are up to date"
        //! This is best solution to that issue so far, but not ideal.
        virtual BentleyStatus MarkTemporaryInstancesAsPartial (const std::vector<CachedResponseKey>& resultsKeys) = 0;

        //--------------------------------------------------------------------------------------------------------------------------------+
        //  Cached file managment
        //--------------------------------------------------------------------------------------------------------------------------------+

        // Change or prepare instances file cache location. Will move file if location changed.
        virtual BentleyStatus SetFileCacheLocation (const bvector<ObjectId>& ids, FileCache cacheLocation) = 0;
        // Change or prepare instance file cache location. Will move file if location changed.
        virtual BentleyStatus SetFileCacheLocation (ObjectIdCR objectId, FileCache cacheLocation) = 0;
        // Returns FileCache location that is setup for given instasnce - Temporary or Persistent
        virtual FileCache     GetFileCacheLocation (ObjectIdCR objectId) = 0;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
