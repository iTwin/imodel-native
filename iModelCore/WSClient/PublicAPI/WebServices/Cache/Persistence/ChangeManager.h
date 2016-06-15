/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Cache/Persistence/ChangeManager.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>
#include <WebServices/Cache/Persistence/IChangeManager.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

struct CachedResponseManager;
struct ChangeInfoManager;
struct ECDbAdapter;
struct FileStorage;
struct FileInfoManager;
struct HierarchyManager;
struct InstanceCacheHelper;
struct ObjectInfoManager;
struct RelationshipInfoManager;
struct RootManager;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct ChangeManager : public IChangeManager
    {
    private:
        ECDbAdapter&                m_dbAdapter;
        InstanceCacheHelper&        m_instanceCacheHelper;
        HierarchyManager&           m_hierarchyManager;
        CachedResponseManager&      m_responseManager;
        FileInfoManager&            m_fileInfoManager;
        ObjectInfoManager&          m_objectInfoManager;
        RelationshipInfoManager&    m_relationshipInfoManager;
        ChangeInfoManager&          m_changeInfoManager;
        FileStorage&                m_fileStorage;
        RootManager&                m_rootManager;
        bool                        m_isSyncActive;

        static Utf8CP LocalInstanceIdPrefix;

    private:
        ECInstanceKey CreateRelationship
            (
            ECRelationshipClassCR relationshipClass,
            ECInstanceKeyCR source,
            ECInstanceKeyCR target,
            SyncStatus syncStatus,
            uint64_t optionalChangeNumber
            );

        BentleyStatus MarkFileAsModified(struct FileInfo& info, SyncStatus syncStatus);
        BentleyStatus SetupNewRevision(struct ChangeInfo& info);
        InstanceRevisionPtr ReadObjectRevision(ECInstanceKeyCR instanceKey);
        InstanceRevisionPtr ReadRelationshipRevision(ECInstanceKeyCR instanceKey);
        static void SetupRevisionChanges(const struct ChangeInfo& info, Revision& revisionInOut);
        BentleyStatus CommitInstanceChange(InstanceRevisionCR revision);
        BentleyStatus CommitRelationshipChange(InstanceRevisionCR revision);

        Utf8String CreateRemoteId();

        JsonValuePtr ReadChangeProperties(ChangeStatus status, ECInstanceKeyCR instance);
        BentleyStatus ReadObjectProperties(ECInstanceKeyCR instanceKey, JsonValueR propertiesOut);
        BentleyStatus ReadObjectPropertiesForCreation(ECInstanceKeyCR instanceKey, JsonValueR propertiesOut);
        BentleyStatus ReadObjectPropertiesForModification(ECInstanceKeyCR instanceKey, JsonValueR propertiesOut);
        static void RemoveCacheSpecificProperties(JsonValueR propertiesJson);
        static void RemoveReadOnlyProperties(JsonValueR propertiesJson, ECClassCR ecClass);
        static void RemoveCalculatedProperties(JsonValueR propertiesJson, ECClassCR ecClass);
        static void RemoveEmptyMembersRecursively(JsonValueR jsonObject);
        static void RemoveEmptyMembersRecursively(JsonValueR childJson, Utf8StringCR childMemberNameInParent, JsonValueR parentJson);

    public:
        WSCACHE_EXPORT ChangeManager
            (
            ECDbAdapter& dbAdapter,
            InstanceCacheHelper& instanceCacheHelper,
            HierarchyManager& hierarchyManager,
            CachedResponseManager& responseManager,
            ObjectInfoManager& objectInfoManager,
            RelationshipInfoManager&  relationshipInfoManager,
            FileInfoManager& fileInfoManager,
            ChangeInfoManager& changeInfoManager,
            FileStorage& fileStorage,
            RootManager& rootManager
            );

        // -- Making local changes to existing data --

        WSCACHE_EXPORT bool IsSyncActive() const override;
        WSCACHE_EXPORT void SetSyncActive(bool active) override;

        WSCACHE_EXPORT ECRelationshipClassCP GetLegacyParentRelationshipClass(ECClassId parentClassId, ECClassId childClassId, bool createIfNotExists = true) override;

        WSCACHE_EXPORT ECInstanceKey CreateObject(ECClassCR ecClass, JsonValueCR properties, SyncStatus syncStatus = SyncStatus::Ready) override;

        WSCACHE_EXPORT BentleyStatus ModifyObject(ECInstanceKeyCR instanceKey, JsonValueCR properties, SyncStatus syncStatus = SyncStatus::Ready) override;

        WSCACHE_EXPORT BentleyStatus RevertModifiedObject(ECInstanceKeyCR instanceKey) override;

        WSCACHE_EXPORT BentleyStatus DeleteObject(ECInstanceKeyCR instanceKey, SyncStatus syncStatus = SyncStatus::Ready) override;

        WSCACHE_EXPORT ECInstanceKey CreateRelationship
            (
            ECRelationshipClassCR relationshipClass,
            ECInstanceKeyCR source,
            ECInstanceKeyCR target,
            SyncStatus syncStatus = SyncStatus::Ready
            ) override;

        WSCACHE_EXPORT BentleyStatus DeleteRelationship
            (
            ECInstanceKeyCR relationshipKey,
            SyncStatus syncStatus = SyncStatus::Ready
            ) override;

        WSCACHE_EXPORT BentleyStatus ModifyFile(ECInstanceKeyCR instanceKey, BeFileNameCR filePath, bool copyFile, SyncStatus syncStatus = SyncStatus::Ready) override;
        WSCACHE_EXPORT BentleyStatus ModifyFileName(ECInstanceKeyCR instanceKey, Utf8StringCR newFileName) override;
        WSCACHE_EXPORT BentleyStatus DetectFileModification(ECInstanceKeyCR instanceKey, SyncStatus syncStatus = SyncStatus::Ready) override;

        WSCACHE_EXPORT BentleyStatus SetSyncStatus(ECInstanceKeyCR instanceKey, SyncStatus syncStatus) override;

        WSCACHE_EXPORT BentleyStatus AddCreatedInstanceToResponse(CachedResponseKeyCR responseKey, ECInstanceKeyCR instanceKey);
        WSCACHE_EXPORT BentleyStatus RemoveCreatedInstanceFromResponse(CachedResponseKeyCR responseKey, ECInstanceKeyCR instanceKey);

        // -- Getting changes --

        WSCACHE_EXPORT bool HasChanges() override;

        WSCACHE_EXPORT BentleyStatus GetChanges(Changes& changesOut, bool onlyReadyToSync = false) override;
        WSCACHE_EXPORT BentleyStatus GetChanges(ECInstanceKeyCR instanceKey, Changes& changesOut) override;
        WSCACHE_EXPORT BentleyStatus GetCreatedRelationships(ECInstanceKeyCR endInstancekey, bvector<RelationshipChange>& changesOut) override;

        WSCACHE_EXPORT ObjectChange GetObjectChange(ECInstanceKeyCR instanceKey) override;
        WSCACHE_EXPORT RelationshipChange GetRelationshipChange(ECInstanceKeyCR relationshipKey) override;
        WSCACHE_EXPORT FileChange GetFileChange(ECInstanceKeyCR instanceKey) override;

        WSCACHE_EXPORT IChangeManager::ChangeStatus GetObjectChangeStatus(ECInstanceKeyCR instanceKey) override;
        WSCACHE_EXPORT ChangeManager::SyncStatus GetObjectSyncStatus(ECInstanceKeyCR instanceKey) override;

        WSCACHE_EXPORT InstanceRevisionPtr ReadInstanceRevision(ECInstanceKeyCR instanceKey) override;
        WSCACHE_EXPORT FileRevisionPtr ReadFileRevision(ECInstanceKeyCR instanceKey) override;

        WSCACHE_EXPORT BentleyStatus ReadModifiedProperties(ECInstanceKeyCR instanceKey, JsonValueR propertiesOut) override;

        // -- Commiting changes --

        WSCACHE_EXPORT BentleyStatus CommitLocalDeletions() override;
        WSCACHE_EXPORT BentleyStatus CommitInstanceRevision(InstanceRevisionCR revision) override;
        WSCACHE_EXPORT BentleyStatus CommitFileRevision(FileRevisionCR revision) override;

        WSCACHE_EXPORT BentleyStatus UpdateCreatedInstance
            (
            ObjectIdCR instanceId,
            WSObjectsResponseCR instanceResponse,
            bmap<ECInstanceKey, ECInstanceKey>& changedInstanceKeysOut
            ) override;
    };

typedef ChangeManager& ChangeManagerR;
typedef const ChangeManager& ChangeManagerCR;

END_BENTLEY_WEBSERVICES_NAMESPACE
