/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Cache/Persistence/ChangeManager.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>
#include <WebServices/Cache/Persistence/IChangeManager.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

struct CachedResponseManager;
struct ChangeInfoManager;
struct ECDbAdapter;
struct FileCacheManager;
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
        ECDbAdapter*                m_dbAdapter;
        InstanceCacheHelper*        m_instanceHelper;
        HierarchyManager*           m_hierarchyManager;
        CachedResponseManager*      m_responseManager;
        FileInfoManager*            m_fileInfoManager;
        ObjectInfoManager*          m_objectInfoManager;
        RelationshipInfoManager*    m_relationshipInfoManager;
        ChangeInfoManager*          m_changeInfoManager;
        FileCacheManager*           m_fileManager;
        RootManager*                m_rootManager;
        bool                        m_isSyncActive;

        static Utf8CP NewObjectIdPrefix;

    private:
        ECInstanceKey CreateRelationship
            (
            ECRelationshipClassCR relationshipClass,
            ECInstanceKeyCR source,
            ECInstanceKeyCR target,
            SyncStatus syncStatus,
            uint64_t optionalChangeNumber
            );

        BentleyStatus CommitObjectCreation(ECInstanceKeyCR instanceKey, Utf8StringCR newRemoteId);
        BentleyStatus CommitRelationshipCreation(ECInstanceKeyCR instanceKey, Utf8StringCR newRemoteId);
        BentleyStatus CommitObjectChange(ECInstanceKeyCR instanceKey);
        BentleyStatus CommitRelationshipChange(ECInstanceKeyCR instanceKey);

        Utf8String CreateRemoteId();

    public:
        WSCACHE_EXPORT ChangeManager
            (
            ECDbAdapter& dbAdapter,
            InstanceCacheHelper& instanceHelper,
            HierarchyManager& hierarchyManager,
            CachedResponseManager& responseManager,
            ObjectInfoManager& objectInfoManager,
            RelationshipInfoManager&  relationshipInfoManager,
            FileInfoManager& fileInfoManager,
            ChangeInfoManager& changeInfoManager,
            FileCacheManager& fileManager,
            RootManager& rootManager
            );

        // -- Making local changes to existing data --

        WSCACHE_EXPORT bool IsSyncActive() const override;
        WSCACHE_EXPORT void SetSyncActive(bool active) override;

        WSCACHE_EXPORT ECInstanceKey LegacyCreateObject(ECClassCR ecClass, JsonValueCR properties, ECInstanceKeyCR parentKey, SyncStatus syncStatus = SyncStatus::Ready) override;
        WSCACHE_EXPORT ECRelationshipClassCP GetLegacyParentRelationshipClass() override;

        WSCACHE_EXPORT ECInstanceKey CreateObject(ECClassCR ecClass, JsonValueCR properties, SyncStatus syncStatus = SyncStatus::Ready) override;

        WSCACHE_EXPORT BentleyStatus ModifyObject(ECInstanceKeyCR instanceKey, JsonValueCR properties, SyncStatus syncStatus = SyncStatus::Ready) override;

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
        WSCACHE_EXPORT BentleyStatus SetSyncStatus(ECInstanceKeyCR instanceKey, SyncStatus syncStatus) override;

        // -- Getting changes --

        WSCACHE_EXPORT bool HasChanges() override;

        WSCACHE_EXPORT BentleyStatus GetChanges(Changes& changesOut, bool onlyReadyToSync = false) override;
        WSCACHE_EXPORT BentleyStatus GetChanges(ECInstanceKeyCR instanceKey, Changes& changesOut) override;
        WSCACHE_EXPORT BentleyStatus GetCreatedRelationships(ECInstanceKeyCR endInstancekey, bvector<RelationshipChange>& changesOut) override;

        WSCACHE_EXPORT ObjectChange GetObjectChange(ECInstanceKeyCR instanceKey) override;
        WSCACHE_EXPORT RelationshipChange GetRelationshipChange(ECInstanceKeyCR relationshipKey) override;
        WSCACHE_EXPORT FileChange GetFileChange(ECInstanceKeyCR instanceKey) override;

        WSCACHE_EXPORT IChangeManager::ChangeStatus GetObjectChangeStatus(ECInstanceKeyCR instance) override;
        WSCACHE_EXPORT ChangeManager::SyncStatus GetObjectSyncStatus(ECInstanceKeyCR instance) override;

        // -- Commiting changes --

        WSCACHE_EXPORT BentleyStatus CommitCreationChanges(const std::map<ECInstanceKey, Utf8String>& newRemoteIds) override;
        WSCACHE_EXPORT BentleyStatus CommitObjectChanges(ECInstanceKeyCR instanceKey) override;
        WSCACHE_EXPORT BentleyStatus CommitFileChanges(ECInstanceKeyCR instanceKey) override;

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
