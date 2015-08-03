/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/IChangeManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Persistence/IChangeManager.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
IChangeManager::ObjectChange::ObjectChange() :
m_changeStatus(ChangeStatus::NoChange),
m_syncStatus(SyncStatus::NotReady)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
IChangeManager::ObjectChange::ObjectChange
(
ECInstanceKeyCR instanceKey,
ChangeStatus changeStatus,
SyncStatus syncStatus,
uint64_t changeNumber
) :
m_instanceKey(instanceKey),
m_changeStatus(changeStatus),
m_syncStatus(syncStatus),
m_changeNumber(changeNumber)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ECInstanceKeyCR IChangeManager::ObjectChange::GetInstanceKey() const
    {
    return m_instanceKey;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
void IChangeManager::ObjectChange::SetInstanceKey(ECInstanceKeyCR instanceKey)
    {
    m_instanceKey = instanceKey;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
IChangeManager::ChangeStatus IChangeManager::ObjectChange::GetChangeStatus() const
    {
    return m_changeStatus;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
IChangeManager::SyncStatus IChangeManager::ObjectChange::GetSyncStatus() const
    {
    return m_syncStatus;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
uint64_t IChangeManager::ObjectChange::GetChangeNumber() const
    {
    return m_changeNumber;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool IChangeManager::ObjectChange::operator == (ObjectChangeCR other) const
    {
    return
        m_instanceKey == other.m_instanceKey &&
        m_changeStatus == other.m_changeStatus &&
        m_syncStatus == other.m_syncStatus &&
        m_changeNumber == other.m_changeNumber;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool IChangeManager::ObjectChange::IsValid() const
    {
    return m_instanceKey.IsValid();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
IChangeManager::RelationshipChange::RelationshipChange() :
ObjectChange()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
IChangeManager::RelationshipChange::RelationshipChange
(
ECInstanceKeyCR relationship,
ECInstanceKeyCR source,
ECInstanceKeyCR target,
ChangeStatus changeStatus,
SyncStatus syncStatus,
uint64_t changeNumber
) :
ObjectChange(relationship, changeStatus, syncStatus, changeNumber),
m_sourceKey(source),
m_targetKey(target)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ECInstanceKeyCR IChangeManager::RelationshipChange::GetSourceKey() const
    {
    return m_sourceKey;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ECInstanceKeyCR IChangeManager::RelationshipChange::GetTargetKey() const
    {
    return m_targetKey;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
void IChangeManager::RelationshipChange::SetSourceKey(ECInstanceKeyCR sourceId)
    {
    m_sourceKey = sourceId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
void IChangeManager::RelationshipChange::SetTargetKey(ECInstanceKeyCR targetId)
    {
    m_targetKey = targetId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool IChangeManager::RelationshipChange::operator == (RelationshipChangeCR other) const
    {
    return
        ObjectChange::operator==(other) &&
        m_sourceKey == other.m_sourceKey &&
        m_targetKey == other.m_targetKey;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool IChangeManager::RelationshipChange::IsValid() const
    {
    return m_instanceKey.IsValid() &&
        (
        (m_changeStatus == ChangeStatus::NoChange) ||
        (m_changeStatus == ChangeStatus::Created && m_sourceKey.IsValid() && m_targetKey.IsValid()) ||
        (m_changeStatus == ChangeStatus::Deleted && !m_sourceKey.IsValid() && !m_targetKey.IsValid())
        // Modified not supported
        );
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
IChangeManager::FileChange::FileChange() :
ObjectChange()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
IChangeManager::FileChange::FileChange
(
ECInstanceKeyCR objectId,
ChangeStatus changeStatus,
SyncStatus syncStatus,
uint64_t changeNumber
) :
ObjectChange(objectId, changeStatus, syncStatus, changeNumber)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool IChangeManager::FileChange::IsValid() const
    {
    return m_instanceKey.IsValid() &&
        (
        (m_changeStatus == ChangeStatus::NoChange) ||
        (m_changeStatus == ChangeStatus::Modified)
        );
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool IChangeManager::Changes::IsEmpty() const
    {
    return m_objectChanges.empty() && m_relationshipChanges.empty() && m_fileChanges.empty();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
void IChangeManager::Changes::AddChange(const ObjectChange& change)
    {
    m_objectChanges.insert(change);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
void IChangeManager::Changes::AddChange(const RelationshipChange& change)
    {
    m_relationshipChanges.insert(change);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
void IChangeManager::Changes::AddChange(const FileChange& change)
    {
    m_fileChanges.insert(change);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
const bset<IChangeManager::ObjectChange, IChangeManager::Changes::Compare>& IChangeManager::Changes::GetObjectChanges() const
    {
    return m_objectChanges;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
const bset<IChangeManager::RelationshipChange, IChangeManager::Changes::Compare>& IChangeManager::Changes::GetRelationshipChanges() const
    {
    return m_relationshipChanges;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
const bset<IChangeManager::FileChange, IChangeManager::Changes::Compare>& IChangeManager::Changes::GetFileChanges() const
    {
    return m_fileChanges;
    }

