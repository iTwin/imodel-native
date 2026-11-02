/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// Implements DgnElements::DeleteElementAspects.
// @bsiclass
//=======================================================================================
class BulkElementAspectDeletion
    {
    struct Row
        {
        ECInstanceId m_id;
        DgnClassId m_classId;
        DgnElementId m_ownerId;
        ECClassCP m_class = nullptr;
        bool m_failed = false;
        };

    struct OwnerGroup
        {
        DgnElementCPtr m_owner;
        bvector<size_t> m_rows;
        };

    DgnDbR m_dgndb;
    bvector<ECInstanceId> const& m_aspectIds;
    bvector<Row> m_rows;
    std::map<DgnClassId, bool> m_hasCustomHandler;
    BulkDeleteElementAspectsResult m_result;

    void Fail(Row& row, BulkElementAspectFailureReason reason, Utf8StringCR detail)
        {
        if (row.m_failed)
            return;
        row.m_failed = true;
        m_result.status = BulkElementAspectsStatus::ValidationFailed;
        m_result.failures.push_back({row.m_id, reason, detail});
        }

    BulkDeleteElementAspectsResult& WriteFailed(DgnDbStatus status, BeSQLite::DbResult sqlStatus)
        {
        m_result.status = BulkElementAspectsStatus::WriteFailed;
        m_result.writeStatus = status;
        m_result.sqlStatus = sqlStatus;
        m_result.failures.clear();
        return m_result;
        }

    template<typename T_Ids> static bool BindIdSet(ECSqlStatement& stmt, int index, T_Ids const& ids)
        {
        auto& binder = stmt.GetBinder(index);
        for (auto const& id : ids)
            {
            if (ECSqlStatus::Success != binder.AddArrayElement().BindId(id))
                return false;
            }
        return true;
        }

    bool HasCustomHandler(DgnClassId classId);
    BeSQLite::DbResult Resolve();
    BeSQLite::DbResult DeleteInstances(ECClassCR aspectClass, bvector<size_t> const& rowIndices);

public:
    BulkElementAspectDeletion(DgnDbR dgndb, bvector<ECInstanceId> const& aspectIds) : m_dgndb(dgndb), m_aspectIds(aspectIds) {}
    BulkDeleteElementAspectsResult Execute();
    };

/*---------------------------------------------------------------------------------**//**
* Aspects of a class with a registered C++ aspect handler (other than the generic one) are deleted through the handler,
* so that its _DeleteInstance override is honored.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BulkElementAspectDeletion::HasCustomHandler(DgnClassId classId)
    {
    auto found = m_hasCustomHandler.find(classId);
    if (m_hasCustomHandler.end() != found)
        return found->second;

    dgn_AspectHandler::Aspect* handler = dgn_AspectHandler::Aspect::FindHandler(m_dgndb, classId);
    bool isCustom = (nullptr != handler) && (handler != &dgn_AspectHandler::Aspect::GetHandler());
    m_hasCustomHandler[classId] = isCustom;
    return isCustom;
    }

/*---------------------------------------------------------------------------------**//**
* Finds the class and owning element of each aspect with a single query. Duplicate ids are dropped; invalid ids are failures.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult BulkElementAspectDeletion::Resolve()
    {
    std::map<ECInstanceId, size_t> rowById;
    bool invalidIdReported = false;
    for (auto const& id : m_aspectIds)
        {
        if (!id.IsValid())
            {
            if (!invalidIdReported)
                {
                invalidIdReported = true;
                m_result.status = BulkElementAspectsStatus::ValidationFailed;
                m_result.failures.push_back({id, BulkElementAspectFailureReason::InvalidId, Utf8String()});
                }
            continue;
            }

        if (rowById.end() != rowById.find(id))
            continue;
        rowById[id] = m_rows.size();
        m_rows.push_back(Row());
        m_rows.back().m_id = id;
        }

    if (m_rows.empty())
        return BE_SQLITE_OK;

    CachedECSqlStatementPtr stmt = m_dgndb.GetPreparedECSqlStatement(
        "SELECT ECInstanceId,ECClassId,Element.Id FROM " BIS_SCHEMA(BIS_CLASS_ElementUniqueAspect) " WHERE ECInstanceId IN (SELECT id FROM IdSet(?)) UNION ALL "
        "SELECT ECInstanceId,ECClassId,Element.Id FROM " BIS_SCHEMA(BIS_CLASS_ElementMultiAspect)  " WHERE ECInstanceId IN (SELECT id FROM IdSet(?))");
    if (!stmt.IsValid())
        {
        LOG.error("BulkElementAspectDeletion: Failed to prepare the aspect lookup");
        return BE_SQLITE_ERROR;
        }

    bvector<ECInstanceId> ids;
    for (auto const& row : m_rows)
        ids.push_back(row.m_id);

    BeSQLite::DbResult result = BE_SQLITE_OK;
    if (!BindIdSet(*stmt, 1, ids) || !BindIdSet(*stmt, 2, ids))
        {
        LOG.error("BulkElementAspectDeletion: Failed to bind the aspect ids");
        result = BE_SQLITE_ERROR;
        }
    else
        {
        BeSQLite::DbResult stat;
        while (BE_SQLITE_ROW == (stat = stmt->Step()))
            {
            auto found = rowById.find(stmt->GetValueId<ECInstanceId>(0));
            if (rowById.end() == found)
                continue;

            auto& row = m_rows[found->second];
            row.m_classId = stmt->GetValueId<DgnClassId>(1);
            row.m_ownerId = stmt->GetValueId<DgnElementId>(2);
            }

        if (BE_SQLITE_DONE != stat)
            {
            LOG.errorv("BulkElementAspectDeletion: Aspect lookup failed: %s", BeSQLiteLib::GetLogError(stat).c_str());
            result = stat;
            }
        }

    stmt->Reset();
    stmt->ClearBindings();
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* Deletes the given aspects of one class with a single statement.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult BulkElementAspectDeletion::DeleteInstances(ECClassCR aspectClass, bvector<size_t> const& rowIndices)
    {
    Utf8PrintfString ecsql("DELETE FROM %s WHERE ECInstanceId IN (SELECT id FROM IdSet(?))", aspectClass.GetECSqlName().c_str());
    CachedECSqlStatementPtr stmt = m_dgndb.GetNonSelectPreparedECSqlStatement(ecsql.c_str(), m_dgndb.GetECCrudWriteToken());
    if (!stmt.IsValid())
        {
        LOG.errorv("BulkElementAspectDeletion: Failed to prepare the delete for class '%s'", aspectClass.GetFullName());
        return BE_SQLITE_ERROR;
        }

    bvector<ECInstanceId> ids;
    for (auto r : rowIndices)
        ids.push_back(m_rows[r].m_id);

    BeSQLite::DbResult stat = BE_SQLITE_ERROR;
    if (!BindIdSet(*stmt, 1, ids))
        {
        LOG.errorv("BulkElementAspectDeletion: Failed to bind the ids for class '%s'", aspectClass.GetFullName());
        }
    else
        {
        stat = stmt->Step();
        if (BE_SQLITE_DONE == stat)
            stat = BE_SQLITE_OK;
        else
            LOG.errorv("BulkElementAspectDeletion: Failed to delete aspects of class '%s': %s", aspectClass.GetFullName(), BeSQLiteLib::GetLogError(stat).c_str());
        }

    stmt->Reset();
    stmt->ClearBindings();
    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* Validates every aspect, then writes. Nothing is written if any aspect fails validation.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BulkDeleteElementAspectsResult BulkElementAspectDeletion::Execute()
    {
    BeSQLite::DbResult lookupStatus = Resolve();
    if (BE_SQLITE_OK != lookupStatus)
        return std::move(WriteFailed(DgnDbStatus::SQLiteError, lookupStatus));

    // Validation pass: group the aspects by owning element, in the order in which the owners first appear
    bvector<OwnerGroup> ownerGroups;
    std::map<DgnElementId, size_t> groupIndex;
    for (size_t i = 0; i < m_rows.size(); ++i)
        {
        auto& row = m_rows[i];
        if (!row.m_classId.IsValid())
            {
            Fail(row, BulkElementAspectFailureReason::AspectNotFound, Utf8String());
            continue;
            }

        row.m_class = m_dgndb.Schemas().GetClass(row.m_classId);
        if (nullptr == row.m_class)
            {
            Fail(row, BulkElementAspectFailureReason::ClassNotFound, Utf8PrintfString("class id %s", row.m_classId.ToHexStr().c_str()));
            continue;
            }

        auto found = groupIndex.find(row.m_ownerId);
        if (groupIndex.end() == found)
            {
            found = groupIndex.emplace(row.m_ownerId, ownerGroups.size()).first;
            ownerGroups.push_back(OwnerGroup());
            ownerGroups.back().m_owner = m_dgndb.Elements().GetElement(row.m_ownerId);
            }

        auto& group = ownerGroups[found->second];
        if (!group.m_owner.IsValid())
            {
            Fail(row, BulkElementAspectFailureReason::ElementNotFound, row.m_ownerId.ToHexStr());
            continue;
            }
        group.m_rows.push_back(i);
        }

    if (BulkElementAspectsStatus::Success != m_result.status)
        return std::move(m_result);

    // Write pass: update each owning element once. Aspects with a custom handler are deleted by that update, the same as
    // DgnElement::Aspect::Delete. The other aspects are deleted afterwards, one statement per class. The first failure stops the operation.
    std::map<DgnClassId, bvector<size_t>> pendingByClass;
    for (auto const& group : ownerGroups)
        {
        DgnElementPtr elementEdit = group.m_owner->CopyForEdit();
        if (!elementEdit.IsValid())
            {
            LOG.errorv("BulkElementAspectDeletion: Unable to edit owning element %s", group.m_owner->GetElementId().ToHexStr().c_str());
            return std::move(WriteFailed(DgnDbStatus::WriteError, BE_SQLITE_OK));
            }

        bvector<RefCountedPtr<DgnElement::Aspect>> handled;
        for (auto r : group.m_rows)
            {
            auto& row = m_rows[r];
            if (!HasCustomHandler(row.m_classId))
                {
                pendingByClass[row.m_classId].push_back(r);
                continue;
                }

            DgnElement::Aspect* aspect = row.m_class->Is(BIS_ECSCHEMA_NAME, BIS_CLASS_ElementMultiAspect)
                ? static_cast<DgnElement::Aspect*>(DgnElement::MultiAspect::GetAspectP(*elementEdit, *row.m_class, row.m_id))
                : static_cast<DgnElement::Aspect*>(DgnElement::UniqueAspect::GetAspectP(*elementEdit, *row.m_class));
            if (nullptr == aspect)
                {
                LOG.errorv("BulkElementAspectDeletion: Unable to load aspect %s through its handler", row.m_id.ToHexStr().c_str());
                return std::move(WriteFailed(DgnDbStatus::NotFound, BE_SQLITE_OK));
                }
            aspect->Delete();
            handled.push_back(aspect);
            }

        DgnDbStatus stat = elementEdit->Update();
        if (DgnDbStatus::Success != stat)
            {
            LOG.errorv("BulkElementAspectDeletion: Unable to update owning element %s", group.m_owner->GetElementId().ToHexStr().c_str());
            return std::move(WriteFailed(stat, BE_SQLITE_OK));
            }

        // The update of the owner succeeds even when one of its aspects cannot be deleted, so check each aspect.
        for (auto const& aspect : handled)
            {
            DgnDbStatus writeStatus = aspect->GetLastWriteStatus();
            if (DgnDbStatus::Success != writeStatus)
                return std::move(WriteFailed(writeStatus, BE_SQLITE_OK));
            }
        }

    for (auto const& entry : pendingByClass)
        {
        BeSQLite::DbResult stat = DeleteInstances(*m_rows[entry.second.front()].m_class, entry.second);
        if (BE_SQLITE_OK != stat)
            return std::move(WriteFailed(DgnDbStatus::SQLiteError, stat));
        }

    return std::move(m_result);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BulkDeleteElementAspectsResult DgnElements::DeleteElementAspects(bvector<ECInstanceId> const& aspectIds)
    {
    return BulkElementAspectDeletion(m_dgndb, aspectIds).Execute();
    }

END_BENTLEY_DGN_NAMESPACE
