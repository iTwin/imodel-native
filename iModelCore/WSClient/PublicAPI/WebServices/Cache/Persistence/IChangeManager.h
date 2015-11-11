/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Cache/Persistence/IChangeManager.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>
#include <WebServices/Client/ObjectId.h>
#include <WebServices/Client/Response/WSObjectsResponse.h>

#include <BeJsonCpp/BeJsonUtilities.h>
#include <ECDb/ECDb.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE IChangeManager
    {
    public:
        enum class ChangeStatus
            {
            NoChange = 0,
            Created = 1,
            Modified = 2,
            Deleted = 4
            };

        enum class SyncStatus
            {
            NotReady = 0,
            Ready = 1
            };

        struct Changes;
        struct FileChange;
        struct ObjectChange;
        struct RelationshipChange;

        typedef Changes& ChangesR;
        typedef const Changes& ChangesCR;
        typedef FileChange& FileChangeR;
        typedef const FileChange& FileChangeCR;
        typedef ObjectChange& ObjectChangeR;
        typedef const ObjectChange& ObjectChangeCR;
        typedef RelationshipChange& RelationshipChangeR;
        typedef const RelationshipChange& RelationshipChangeCR;

    public:
        virtual ~IChangeManager()
            {};

        //! -- Making local changes to existing data --

        // Check if sync is currently active - when modifications to existing changes cannot be done.
        virtual bool IsSyncActive() const = 0;
        // Internal use only! Set sync active so modifications to existing changes could not be done.
        virtual void SetSyncActive(bool active) = 0;

        //! For legacy server (version < 2.0) only. Create new object in local cache under specified parent
        virtual ECInstanceKey LegacyCreateObject(ECClassCR ecClass, JsonValueCR properties, ECInstanceKeyCR parentKey, SyncStatus syncStatus = SyncStatus::Ready) = 0;

        //! For legacy server (version < 2.0) only. Get relationship class is used in LegacyCreateObject()
        virtual ECRelationshipClassCP GetLegacyParentRelationshipClass() = 0;

        //! Create new object in local cache
        //! object id should be unique (e.g. GUID) to later identify created instance.
        //! TODO: remove the need for filling in remoteId. ECInstanceKey should be enough to find instance in cache. Remote id should be empty
        virtual ECInstanceKey CreateObject(ECClassCR ecClass, JsonValueCR properties, SyncStatus syncStatus = SyncStatus::Ready) = 0;

        //! Modify object properties
        virtual BentleyStatus ModifyObject(ECInstanceKeyCR instanceKey, JsonValueCR properties, SyncStatus syncStatus = SyncStatus::Ready) = 0;

        //! Delete object from cache and mark it for sync
        virtual BentleyStatus DeleteObject(ECInstanceKeyCR instanceKey, SyncStatus syncStatus = SyncStatus::Ready) = 0;

        //! Add new relationship between instances.
        virtual ECInstanceKey CreateRelationship(
            ECRelationshipClassCR relationshipClass,
            ECInstanceKeyCR source,
            ECInstanceKeyCR target,
            SyncStatus syncStatus = SyncStatus::Ready
            ) = 0;

        //! Delete existing relationship between instances.
        virtual BentleyStatus DeleteRelationship(
            ECInstanceKeyCR relationshipKey,
            SyncStatus syncStatus = SyncStatus::Ready
            ) = 0;

        //! Modify file content for existing object.
        //! @param[in] instanceKey
        //! @param[in] filePath - path to file that should be cached
        //! @param[in] copyFile - pass false to move file to cache and true to copy and leave original
        //! @param[in] syncStatus
        virtual BentleyStatus ModifyFile(ECInstanceKeyCR instnaceKey, BeFileNameCR filePath, bool copyFile, SyncStatus syncStatus = SyncStatus::Ready) = 0;

        //! Change whether or not an object is ready to be synced to the server
        virtual BentleyStatus SetSyncStatus(ECInstanceKeyCR instnaceKey, SyncStatus syncStatus) = 0;

        // -- Getting changes --

        //! Check if local cache has any pending changes
        virtual bool HasChanges() = 0;

        //! Get all pending uncommited changes. Will always return valid changes or ERROR.
        virtual BentleyStatus GetChanges(Changes& changesOut, bool onlyReadyToSync = false) = 0;
        //! Get pending uncommited changes for specified instance.
        virtual BentleyStatus GetChanges(ECInstanceKeyCR instanceKey, Changes& changesOut) = 0;
        //! Get pending uncommited relationship changes for specified end instance.
        virtual BentleyStatus GetCreatedRelationships(ECInstanceKeyCR endInstancekey, bvector<RelationshipChange>& changesOut) = 0;

        //! Get change. Will return ERROR if no object changes for specified key exist
        virtual ObjectChange GetObjectChange(ECInstanceKeyCR instanceKey) = 0;
        //! Get change. Will return ERROR if no relationship changes for specified key exist
        virtual RelationshipChange GetRelationshipChange(ECInstanceKeyCR relationshipKey) = 0;
        //! Get change. Will return ERROR if no file changes for specified key exist
        virtual FileChange GetFileChange(ECInstanceKeyCR instanceKey) = 0;

        //! More efficient way to get ChangeStatus
        virtual ChangeStatus GetObjectChangeStatus(ECInstanceKeyCR instance) = 0;
        // More efficient way to get SyncStatus
        virtual SyncStatus GetObjectSyncStatus(ECInstanceKeyCR instance) = 0;

        // -- Getting changed data --

        //! Get instance properties that were modified
        virtual BentleyStatus ReadModifiedProperties(ECInstanceKeyCR instance, JsonValueR propertiesOut) = 0;

        // -- Commiting changes --

        //! Commit change for created object or relationship
        virtual BentleyStatus CommitCreationChange(ECInstanceKeyCR instanceKey, Utf8StringCR newRemoteId) = 0;

        //! Commit change for object in local cache.
        virtual BentleyStatus CommitObjectChange(ECInstanceKeyCR instanceKey) = 0;

        //! Commit change for object file content in local cache.
        virtual BentleyStatus CommitFileChange(ECInstanceKeyCR instanceKey) = 0;

        //! Update created instance with new properties and update class if changed. Returns same or new class instance key.
        virtual BentleyStatus UpdateCreatedInstance(
            ObjectIdCR instanceId,
            WSObjectsResponseCR instanceResponse,
            bmap<ECInstanceKey, ECInstanceKey>& changedInstanceKeysOut
            ) = 0;
    };

typedef IChangeManager& IChangeManagerR;
typedef const IChangeManager& IChangeManagerCR;

/*--------------------------------------------------------------------------------------+
* @bsiclass
+--------------------------------------------------------------------------------------*/
struct IChangeManager::ObjectChange
    {
    protected:
        ECInstanceKey m_instanceKey;
        ChangeStatus m_changeStatus;
        SyncStatus m_syncStatus;
        uint64_t m_changeNumber;

    public:
        WSCACHE_EXPORT ObjectChange();
        WSCACHE_EXPORT ObjectChange(ECInstanceKeyCR instanceKey, ChangeStatus changeStatus, SyncStatus syncStatus, uint64_t changeNumber);
        WSCACHE_EXPORT ECInstanceKeyCR GetInstanceKey() const;
        WSCACHE_EXPORT void SetInstanceKey(ECInstanceKeyCR instanceKey);
        WSCACHE_EXPORT ChangeStatus GetChangeStatus() const;
        WSCACHE_EXPORT SyncStatus GetSyncStatus() const;
        WSCACHE_EXPORT uint64_t GetChangeNumber() const;
        WSCACHE_EXPORT bool operator == (ObjectChangeCR other) const;
        WSCACHE_EXPORT virtual bool IsValid() const;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass
+--------------------------------------------------------------------------------------*/
struct IChangeManager::RelationshipChange : public IChangeManager::ObjectChange
    {
    protected:
        ECInstanceKey m_sourceKey;
        ECInstanceKey m_targetKey;

    public:
        WSCACHE_EXPORT RelationshipChange();
        WSCACHE_EXPORT RelationshipChange(
            ECInstanceKeyCR relationship,
            ECInstanceKeyCR source,
            ECInstanceKeyCR target,
            ChangeStatus changeStatus,
            SyncStatus syncStatus,
            uint64_t changeNumber
            );
        WSCACHE_EXPORT ECInstanceKeyCR GetSourceKey() const;
        WSCACHE_EXPORT ECInstanceKeyCR GetTargetKey() const;
        WSCACHE_EXPORT void SetSourceKey(ECInstanceKeyCR sourceKey);
        WSCACHE_EXPORT void SetTargetKey(ECInstanceKeyCR targetKey);
        WSCACHE_EXPORT bool operator == (RelationshipChangeCR other) const;
        WSCACHE_EXPORT virtual bool IsValid() const override;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass
+--------------------------------------------------------------------------------------*/
struct IChangeManager::FileChange : public IChangeManager::ObjectChange
    {
    public:
        WSCACHE_EXPORT FileChange();
        WSCACHE_EXPORT FileChange(ECInstanceKeyCR instanceKey, ChangeStatus changeStatus, SyncStatus syncStatus, uint64_t changeNumber);
        WSCACHE_EXPORT virtual bool IsValid() const override;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass
+--------------------------------------------------------------------------------------*/
struct IChangeManager::Changes
    {
    private:
        struct Compare
            {
            bool operator() (const ObjectChange& lhs, const ObjectChange& rhs) const
                {
                return lhs.GetChangeNumber() < rhs.GetChangeNumber();
                }
            };

        bset<ObjectChange, Compare>       m_objectChanges;
        bset<RelationshipChange, Compare> m_relationshipChanges;
        bset<FileChange, Compare>         m_fileChanges;

    public:
        WSCACHE_EXPORT bool IsEmpty() const;

        WSCACHE_EXPORT void AddChange(const ObjectChange& change);
        WSCACHE_EXPORT void AddChange(const RelationshipChange& change);
        WSCACHE_EXPORT void AddChange(const FileChange& change);

        WSCACHE_EXPORT const bset<ObjectChange, Compare>&       GetObjectChanges() const;
        WSCACHE_EXPORT const bset<RelationshipChange, Compare>& GetRelationshipChanges() const;
        WSCACHE_EXPORT const bset<FileChange, Compare>&         GetFileChanges() const;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
