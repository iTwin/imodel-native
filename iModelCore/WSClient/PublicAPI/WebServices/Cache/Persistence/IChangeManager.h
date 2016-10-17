/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Cache/Persistence/IChangeManager.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>
#include <WebServices/Cache/Persistence/CachedResponseKey.h>
#include <WebServices/Client/ObjectId.h>
#include <WebServices/Client/Response/WSObjectsResponse.h>

#include <BeJsonCpp/BeJsonUtilities.h>
#include <ECDb/ECDb.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

typedef std::shared_ptr<Json::Value> JsonValuePtr;

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

        struct Revision;
        struct InstanceRevision;
        struct FileRevision;

        typedef Changes& ChangesR;
        typedef const Changes& ChangesCR;
        typedef FileChange& FileChangeR;
        typedef const FileChange& FileChangeCR;
        typedef ObjectChange& ObjectChangeR;
        typedef const ObjectChange& ObjectChangeCR;
        typedef RelationshipChange& RelationshipChangeR;
        typedef const RelationshipChange& RelationshipChangeCR;

        typedef InstanceRevision& InstanceRevisionR;
        typedef const InstanceRevision& InstanceRevisionCR;
        typedef std::shared_ptr<InstanceRevision> InstanceRevisionPtr;
        typedef FileRevision& FileRevisionR;
        typedef const FileRevision& FileRevisionCR;
        typedef std::shared_ptr<FileRevision> FileRevisionPtr;

    public:
        virtual ~IChangeManager()
            {};

        //! -- Making local changes to existing data --

        // Check if sync is currently active - when modifications to existing changes cannot be done.
        virtual bool IsSyncActive() const = 0;
        // Internal use only! Set sync active so modifications to existing changes could not be done.
        virtual void SetSyncActive(bool active) = 0;

        //! For legacy server (version < 2.0) only.
        //! Get or generate relationship class to be used with specific parent and child class instances.
        //! If creating new class, will reset ECDb schema cache, so all pointers to ECSchema or ECClass objects can be made invalid.
        //! IChangeManager pointers will also be invalidated.
        virtual ECRelationshipClassCP GetLegacyParentRelationshipClass(ECClassId parentClassId, ECClassId childClassId, bool createIfNotExists = true) = 0;

        //! Create new object in local cache
        //! object id should be unique (e.g. GUID) to later identify created instance.
        virtual ECInstanceKey CreateObject(ECClassCR ecClass, JsonValueCR properties, SyncStatus syncStatus = SyncStatus::Ready) = 0;

        //! Modify object properties
        virtual BentleyStatus ModifyObject(ECInstanceKeyCR instanceKey, JsonValueCR properties, SyncStatus syncStatus = SyncStatus::Ready) = 0;

        //! Remove change and revert modified instance properties to latest cached version
        virtual BentleyStatus RevertModifiedObject(ECInstanceKeyCR instanceKey) = 0;

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
        virtual BentleyStatus ModifyFile(ECInstanceKeyCR instanceKey, BeFileNameCR filePath, bool copyFile, SyncStatus syncStatus = SyncStatus::Ready) = 0;

        //! Modify name for existing modified file on disk. Does not modify any properties in ECInstance.
        //! @param[in] instanceKey
        //! @param[in] newFileName - new name for file. Invalid characters that are not supported by file system will be normalized
        virtual BentleyStatus ModifyFileName(ECInstanceKeyCR instanceKey, Utf8StringCR newFileName) = 0;

        //! Checks if file was modified externally. If so, marks file ChangeStatus as Modified.
        //! @param[in] instanceKey
        //! @param[in] syncStatus
        virtual BentleyStatus DetectFileModification(ECInstanceKeyCR instanceKey, SyncStatus syncStatus = SyncStatus::Ready) = 0;

        //! Change whether or not an object is ready to be synced to the server
        virtual BentleyStatus SetSyncStatus(ECInstanceKeyCR instanceKey, SyncStatus syncStatus) = 0;

        //! Add created instance to response to act as placeholder until it is synced to server and pulled back. Instance commit will remove it from response
        virtual BentleyStatus AddCreatedInstanceToResponse(CachedResponseKeyCR responseKey, ECInstanceKeyCR instanceKey) = 0;

        //! Remove created instance from response
        virtual BentleyStatus RemoveCreatedInstanceFromResponse(CachedResponseKeyCR responseKey, ECInstanceKeyCR instanceKey) = 0;

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

        //! Get object or relationship revision for sync and commit. Will not return null.
        virtual InstanceRevisionPtr ReadInstanceRevision(ECInstanceKeyCR instanceKey) = 0;
        //! Get file revision for sync and commit. Will not return null.
        virtual FileRevisionPtr ReadFileRevision(ECInstanceKeyCR instanceKey) = 0;

        //! Get instance properties that were modified
        virtual BentleyStatus ReadModifiedProperties(ECInstanceKeyCR instanceKey, JsonValueR propertiesOut) = 0;

        // -- Commiting changes --

        //! Commit and clear all created and deleted objects.
        virtual BentleyStatus CommitLocalDeletions() = 0;

        //! Commit revision for object or relationship in local cache.
        virtual BentleyStatus CommitInstanceRevision(InstanceRevisionCR revision) = 0;

        //! Commit revision for file in local cache.
        virtual BentleyStatus CommitFileRevision(FileRevisionCR revision) = 0;

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
        virtual  ~ObjectChange() {}
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

/*--------------------------------------------------------------------------------------+
* @bsiclass
+--------------------------------------------------------------------------------------*/
struct IChangeManager::Revision
    {
    protected:
        ECInstanceKey m_instanceKey;
        ObjectId m_objectId;
        ChangeStatus m_changeStatus = ChangeStatus::NoChange;
        SyncStatus m_syncStatus = SyncStatus::NotReady;
        uint64_t m_changeNumber = 0;
        uint64_t m_revisionNumber = 0;

    public:
        ECInstanceKeyCR GetInstanceKey() const
            {
            return m_instanceKey;
            }
        void SetInstanceKey(ECInstanceKey value)
            {
            m_instanceKey = value;
            }
        ObjectIdCR GetObjectId() const
            {
            return m_objectId;
            }
        void SetObjectId(ObjectId value)
            {
            m_objectId = value;
            }
        void SetRemoteId(Utf8String remoteId)
            {
            m_objectId.remoteId = remoteId;
            }
        ChangeStatus GetChangeStatus() const
            {
            return m_changeStatus;
            }
        void SetChangeStatus(ChangeStatus value)
            {
            m_changeStatus = value;
            }
        SyncStatus GetSyncStatus() const
            {
            return m_syncStatus;
            }
        void SetSyncStatus(SyncStatus value)
            {
            m_syncStatus = value;
            }
        uint64_t GetChangeNumber() const
            {
            return m_changeNumber;
            }
        void SetChangeNumber(uint64_t value)
            {
            m_changeNumber = value;
            }
        uint64_t GetRevisionNumber() const
            {
            return m_revisionNumber;
            }
        void SetRevisionNumber(uint64_t value)
            {
            m_revisionNumber = value;
            }
        bool IsValid() const
            {
            return m_instanceKey.IsValid() && m_changeStatus != ChangeStatus::NoChange;
            }
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass
+--------------------------------------------------------------------------------------*/
struct IChangeManager::InstanceRevision : public IChangeManager::Revision
    {
    protected:
        JsonValuePtr m_changedProperties; // TODO: JsonValueCPtr

    public:
        //! Get changed instance properties
        JsonValuePtr GetChangedProperties() const
            {
            return m_changedProperties;
            }
        //! Set changed instance properties
        void SetChangedProperties(JsonValuePtr value)
            {
            m_changedProperties = value;
            }
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass
+--------------------------------------------------------------------------------------*/
struct IChangeManager::FileRevision : public IChangeManager::Revision
    {
    protected:
        BeFileName m_filePath;

    public:
        //! Get modified file path
        BeFileNameCR GetFilePath() const
            {
            return m_filePath;
            }
        //! Set modified file path
        void SetFilePath(BeFileName value)
            {
            m_filePath = value;
            };
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
