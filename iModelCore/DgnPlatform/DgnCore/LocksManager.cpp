/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/LocksManager.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Ramanujam.Raman   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
LockableId::LockableId(SchemaManagerCR schemas)
    : m_id(s_dbId), m_type(LockableType::Schemas)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    06/17
+---------------+---------------+---------------+---------------+---------------+------*/
LockableId::LockableId(DgnCodeSpecs const& codeSpecs) : m_id(s_dbId), m_type(LockableType::CodeSpecs)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
LockableId::LockableId(DgnModelCR model) : m_id(model.GetModelId()), m_type(LockableType::Model)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
LockableId::LockableId(DgnElementCR el) : m_id(el.GetElementId()), m_type(LockableType::Element)
    {
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
* @bsimethod                                                  Ramanujam.Raman   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void LockRequest::InsertSchemasLock(DgnDbCR db)
    {
    InsertLock(LockableId(db.Schemas()), LockLevel::Exclusive);
    Insert(db, LockLevel::Shared); // Insert shared lock on the DgnDb to prevent it's deletion.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void LockRequest::InsertCodeSpecsLock(DgnDbCR db)
    {
    InsertLock(LockableId(db.CodeSpecs()), LockLevel::Exclusive);
    Insert(db, LockLevel::Shared); // Insert shared lock on the DgnDb to prevent it's deletion.
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
void LockRequest::FromRevision(DgnRevision& rev, DgnDbCR dgndb, bool extractInserted, bool avoidExclusiveModelElements)
    {
    rev.ExtractLocks(m_locks, dgndb, extractInserted, avoidExclusiveModelElements);
    }
