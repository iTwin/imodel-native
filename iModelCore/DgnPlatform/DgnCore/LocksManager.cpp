/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/LocksManager.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnChangeSummary.h>

static const BeInt64Id s_dbId((uint64_t)1);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
LockableId::LockableId(DgnDbCR db)
    : m_id(s_dbId), m_type(LockableType::Db)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
LockableId::LockableId(DgnModelCR model) : m_id(model.GetModelId()), m_type(LockableType::Model)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
LockableId::LockableId(DgnElementCR el) : m_id(el.GetElementId()), m_type(LockableType::Element)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnLockCP LockRequest::Find(DgnLockCR lock, bool matchExactLevel) const
    {
    auto iter = m_locks.find(DgnLock(lock.GetLockableId(), LockLevel::Exclusive));
    if (m_locks.end() == iter)
        return nullptr;
    else if (matchExactLevel && iter->GetLevel() != lock.GetLevel())
        return nullptr;
    else if (iter->GetLevel() > lock.GetLevel())
        return nullptr;

    return &(*iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LockRequest::Insert(DgnElementCR el, LockLevel level)
    {
    // Currently no reason to obtain shared lock on an element...may have a use case for parent elements at some point.
    if (LockLevel::Shared == level)
        level = LockLevel::Exclusive;

    InsertLock(LockableId(el.GetElementId()), level);
    if (LockLevel::Exclusive == level)
        {
        DgnModelPtr model = el.GetModel();
        BeAssert(model.IsValid());
        if (model.IsValid())
            Insert(*model, LockLevel::Shared);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LockRequest::Insert(DgnModelCR model, LockLevel level)
    {
    InsertLock(LockableId(model.GetModelId()), level);
    if (LockLevel::None != level)
        Insert(model.GetDgnDb(), LockLevel::Shared);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LockRequest::Insert(DgnDbCR db, LockLevel level)
    {
    InsertLock(LockableId(db), level);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LockRequest::InsertLock(LockableId id, LockLevel level)
    {
    if (LockLevel::None == level || !id.IsValid())
        return;

    auto pair = m_locks.insert(DgnLock(id, level));
    pair.first->EnsureLevel(level);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LockRequest::Remove(LockableId id)
    {
    auto iter = m_locks.find(DgnLock(id, LockLevel::Exclusive));
    if (m_locks.end() != iter)
        m_locks.erase(iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void LockRequest::ExtractLockSet(DgnLockSet& locks)
    {
    locks.clear();
    std::swap(m_locks, locks);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void LockRequest::FromRevision(DgnRevision& rev)
    {
    rev.ExtractUsedLocks(m_locks);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LockRequest::FromChangeSummary(DgnChangeSummary const& summary, bool stopOnFirst)
    {
    Clear();

    for (auto const& entry : summary.MakeElementIterator())
        {
        if (entry.IsIndirectChange())
            continue;   // ###TODO: Allow iterator options to specify exclusion of indirect changes

        DgnModelId modelId;
        switch (entry.GetDbOpcode())
            {
            case DbOpcode::Insert:  modelId = entry.GetCurrentModelId(); break;
            case DbOpcode::Delete:  modelId = entry.GetOriginalModelId(); break;
            case DbOpcode::Update:
                {
                modelId = entry.GetCurrentModelId();
                auto oldModelId = entry.GetOriginalModelId();
                if (oldModelId != modelId)
                    InsertLock(LockableId(oldModelId), LockLevel::Shared);

                break;
                }
            }

        BeAssert(modelId.IsValid());
        InsertLock(LockableId(modelId), LockLevel::Shared);
        InsertLock(LockableId(entry.GetElementId()), LockLevel::Exclusive);
        if (stopOnFirst && !IsEmpty())
            return;
        }

    // Any models directly changed?
    ECClassId classId = summary.GetDgnDb().Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_Model);
    DgnChangeSummary::InstanceIterator::Options options(classId);
    for (auto const& entry : summary.MakeInstanceIterator(options))
        {
        if (!entry.GetIndirect())
            {
            InsertLock(LockableId(LockableType::Model, entry.GetInstanceId()), LockLevel::Exclusive);
            if (stopOnFirst && !IsEmpty())
                return;
            }
        }

    // Anything changed at all?
    if (!IsEmpty())
        Insert(summary.GetDgnDb(), LockLevel::Shared);
    }

#define JSON_Status "Status"            // RepositoryStatus
#define JSON_AllAcquired "AllAcquired"  // boolean
#define JSON_Locks "Locks"              // list of DgnLock.
#define JSON_LockableId "LockableId"    // LockableId
#define JSON_Id "Id"                    // BeInt64Id
#define JSON_LockType "Type"            // LockType
#define JSON_LockLevel "Level"          // LockLevel
#define JSON_Owner "Owner"              // BeBriefcaseId
#define JSON_DeniedLocks "DeniedLocks"  // list of DgnLock. Only supplied if AllAcquired=false
#define JSON_ExclusiveOwner "Exclusive" // BeBriefcaseId
#define JSON_SharedOwners "Shared"      // list of BeBriefcaseId

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnLocksJson::BeInt64IdFromJson(BeInt64Id& id, JsonValueCR value)
    {
    if (value.isNull())
        return false;

    id = BeInt64Id(value.asInt64());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnLocksJson::BeInt64IdToJson(JsonValueR value, BeInt64Id id)
    {
    value = id.GetValue();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnLocksJson::BriefcaseIdFromJson(BeBriefcaseId& bcId, JsonValueCR value)
    {
    if (!value.isConvertibleTo(Json::uintValue))
        return false;

    bcId = BeBriefcaseId(value.asUInt());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnLocksJson::BriefcaseIdToJson(JsonValueR value, BeBriefcaseId id)
    {
    value = id.GetValue();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnLocksJson::LockLevelFromJson(LockLevel& level, JsonValueCR value)
    {
    if (value.isConvertibleTo(Json::uintValue))
        {
        level = static_cast<LockLevel>(value.asUInt());
        switch (level)
            {
            case LockLevel::None:
            case LockLevel::Shared:
            case LockLevel::Exclusive:
                return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnLocksJson::LockLevelToJson(JsonValueR value, LockLevel level)
    {
    value = static_cast<uint32_t>(level);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnLocksJson::LockableTypeFromJson(LockableType& type, JsonValueCR value)
    {
    if (value.isConvertibleTo(Json::uintValue))
        {
        type = static_cast<LockableType>(value.asUInt());
        switch (type)
            {
            case LockableType::Db:
            case LockableType::Model:
            case LockableType::Element:
                return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnLocksJson::LockableTypeToJson(JsonValueR value, LockableType type)
    {
    value = static_cast<uint32_t>(type);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnLocksJson::RepositoryStatusFromJson(RepositoryStatus& status, JsonValueCR value)
    {
    if (!value.isConvertibleTo(Json::uintValue))
        return false;

    status = static_cast<RepositoryStatus>(value.asUInt());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnLocksJson::RepositoryStatusToJson(JsonValueR value, RepositoryStatus status)
    {
    value = static_cast<uint32_t>(status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LockableId::ToJson(JsonValueR value) const
    {
    DgnLocksJson::BeInt64IdToJson(value[JSON_Id], m_id);
    DgnLocksJson::LockableTypeToJson(value[JSON_LockType], m_type);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool LockableId::FromJson(JsonValueCR value)
    {
    if (!DgnLocksJson::BeInt64IdFromJson(m_id, value[JSON_Id]) || !DgnLocksJson::LockableTypeFromJson(m_type, value[JSON_LockType]))
        {
        Invalidate();
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnLock::ToJson(JsonValueR value) const
    {
    m_id.ToJson(value[JSON_LockableId]);
    DgnLocksJson::LockLevelToJson(value[JSON_LockLevel], m_level);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnLock::FromJson(JsonValueCR value)
    {
    if (!m_id.FromJson(value[JSON_LockableId]) || !DgnLocksJson::LockLevelFromJson(m_level, value[JSON_LockLevel]))
        {
        Invalidate();
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnLockOwnership::ToJson(JsonValueR value) const
    {
    auto level = GetLockLevel();
    DgnLocksJson::LockLevelToJson(value[JSON_LockLevel], level);
    switch (level)
        {
        case LockLevel::Exclusive:
            value[JSON_ExclusiveOwner] = GetExclusiveOwner().GetValue();
            break;
        case LockLevel::Shared:
            {
            uint32_t nOwners = static_cast<uint32_t>(m_sharedOwners.size());
            Json::Value owners(Json::arrayValue);
            owners.resize(nOwners);

            uint32_t i = 0;
            for (auto const& owner : m_sharedOwners)
                owners[i++] = owner.GetValue();

            value[JSON_SharedOwners] = owners;
            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnLockOwnership::FromJson(JsonValueCR value)
    {
    Reset();
    LockLevel level;
    if (!DgnLocksJson::LockLevelFromJson(level, value[JSON_LockLevel]))
        return false;

    switch (level)
        {
        case LockLevel::None:
            return true;
        case LockLevel::Exclusive:
            return DgnLocksJson::BriefcaseIdFromJson(m_exclusiveOwner, value[JSON_ExclusiveOwner]);
        case LockLevel::Shared:
            {
            JsonValueCR owners = value[JSON_SharedOwners];
            if (!owners.isArray())
                return false;

            BeBriefcaseId owner;
            uint32_t nOwners = owners.size();
            for (uint32_t i = 0; i < nOwners; i++)
                {
                if (!DgnLocksJson::BriefcaseIdFromJson(owner, value[i]))
                    {
                    Reset();
                    return false;
                    }

                AddSharedOwner(owner);
                }

            return true;
            }
        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LockRequest::ToJson(JsonValueR value) const
    {
    uint32_t nLocks = static_cast<uint32_t>(m_locks.size());
    Json::Value locks(Json::arrayValue);
    locks.resize(nLocks);

    uint32_t i = 0;
    for (auto const& lock : m_locks)
        lock.ToJson(locks[i++]);

    value[JSON_Locks] = locks;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool LockRequest::FromJson(JsonValueCR value)
    {
    Clear();
    JsonValueCR locks = value[JSON_Locks];
    if (!locks.isArray())
        return false;

    DgnLock lock;
    uint32_t nLocks = locks.size();
    for (uint32_t i = 0; i < nLocks; i++)
        {
        if (!lock.FromJson(locks[i]))
            {
            Clear();
            return false;
            }

        m_locks.insert(lock);
        }

    return true;
    }

