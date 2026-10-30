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

    DgnDbR m_dgndb;
    bvector<ECInstanceId> const& m_aspectIds;
    bvector<Row> m_rows;
    std::map<DgnClassId, bool> m_hasCustomHandler;
    BulkDeleteElementAspectsResult m_result;

    void Fail(Row& row, DgnDbStatus status, Utf8CP message)
        {
        if (row.m_failed)
            return;
        row.m_failed = true;
        m_result.failures.push_back({row.m_id, status, message});
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
    void DeleteInstances(ECClassCR aspectClass, bvector<size_t> const& rowIndices);

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
* Finds the class and owning element of each aspect with a single query. Invalid and duplicate ids are dropped.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult BulkElementAspectDeletion::Resolve()
    {
    std::map<ECInstanceId, size_t> rowById;
    for (auto const& id : m_aspectIds)
        {
        if (!id.IsValid() || rowById.end() != rowById.find(id))
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
void BulkElementAspectDeletion::DeleteInstances(ECClassCR aspectClass, bvector<size_t> const& rowIndices)
    {
    Utf8PrintfString ecsql("DELETE FROM %s WHERE ECInstanceId IN (SELECT id FROM IdSet(?))", aspectClass.GetECSqlName().c_str());
    CachedECSqlStatementPtr stmt = m_dgndb.GetNonSelectPreparedECSqlStatement(ecsql.c_str(), m_dgndb.GetECCrudWriteToken());

    bool ok = stmt.IsValid();
    if (ok)
        {
        bvector<ECInstanceId> ids;
        for (auto r : rowIndices)
            ids.push_back(m_rows[r].m_id);

        ok = BindIdSet(*stmt, 1, ids);
        if (ok)
            {
            BeSQLite::DbResult stat = stmt->Step();
            ok = (BE_SQLITE_DONE == stat);
            if (!ok)
                LOG.errorv("BulkElementAspectDeletion: Failed to delete aspects of class '%s': %s", aspectClass.GetFullName(), BeSQLiteLib::GetLogError(stat).c_str());
            }

        stmt->Reset();
        stmt->ClearBindings();
        }
    else
        {
        LOG.errorv("BulkElementAspectDeletion: Failed to prepare the delete for class '%s'", aspectClass.GetFullName());
        }

    if (!ok)
        {
        for (auto r : rowIndices)
            Fail(m_rows[r], DgnDbStatus::WriteError, "unable to delete aspect");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BulkDeleteElementAspectsResult BulkElementAspectDeletion::Execute()
    {
    m_result.sqlStatus = Resolve();
    if (BE_SQLITE_OK != m_result.sqlStatus)
        return std::move(m_result);

    // Group the aspects by owning element, in the order in which the owners first appear
    bvector<std::pair<DgnElementId, bvector<size_t>>> ownerGroups;
    std::map<DgnElementId, size_t> groupIndex;
    for (size_t i = 0; i < m_rows.size(); ++i)
        {
        auto& row = m_rows[i];
        if (!row.m_classId.IsValid())
            {
            Fail(row, DgnDbStatus::NotFound, "aspect not found");
            continue;
            }

        row.m_class = m_dgndb.Schemas().GetClass(row.m_classId);
        if (nullptr == row.m_class)
            {
            Fail(row, DgnDbStatus::BadSchema, "aspect class not found");
            continue;
            }

        auto found = groupIndex.find(row.m_ownerId);
        if (groupIndex.end() == found)
            {
            found = groupIndex.emplace(row.m_ownerId, ownerGroups.size()).first;
            ownerGroups.push_back(std::make_pair(row.m_ownerId, bvector<size_t>()));
            }
        ownerGroups[found->second].second.push_back(i);
        }

    // Update each owning element once. Aspects with a custom handler are deleted by that update, the same as DgnElement::Aspect::Delete.
    // The other aspects are deleted afterwards, one statement per class.
    std::map<DgnClassId, bvector<size_t>> pendingByClass;
    for (auto const& group : ownerGroups)
        {
        DgnElementCPtr owner = m_dgndb.Elements().GetElement(group.first);
        if (!owner.IsValid())
            {
            for (auto r : group.second)
                Fail(m_rows[r], DgnDbStatus::MissingId, "owning element not found");
            continue;
            }

        DgnElementPtr elementEdit = owner->CopyForEdit();
        if (!elementEdit.IsValid())
            {
            for (auto r : group.second)
                Fail(m_rows[r], DgnDbStatus::WriteError, "unable to edit owning element");
            continue;
            }

        bvector<size_t> generic;
        std::map<size_t, RefCountedPtr<DgnElement::Aspect>> handled;
        for (auto r : group.second)
            {
            auto& row = m_rows[r];
            if (!HasCustomHandler(row.m_classId))
                {
                generic.push_back(r);
                continue;
                }

            DgnElement::Aspect* aspect = row.m_class->Is(BIS_ECSCHEMA_NAME, BIS_CLASS_ElementMultiAspect)
                ? static_cast<DgnElement::Aspect*>(DgnElement::MultiAspect::GetAspectP(*elementEdit, *row.m_class, row.m_id))
                : static_cast<DgnElement::Aspect*>(DgnElement::UniqueAspect::GetAspectP(*elementEdit, *row.m_class));
            if (nullptr == aspect)
                {
                Fail(row, DgnDbStatus::NotFound, "aspect not found");
                continue;
                }
            aspect->Delete();
            handled[r] = aspect;
            }

        if (generic.empty() && handled.empty())
            continue;

        DgnDbStatus stat = elementEdit->Update();
        if (DgnDbStatus::Success != stat)
            {
            for (auto r : group.second)
                Fail(m_rows[r], stat, "unable to update owning element");
            continue;
            }

        // The update of the owner succeeds even when one of its aspects cannot be deleted, so check each aspect.
        for (auto const& entry : handled)
            {
            DgnDbStatus writeStatus = entry.second->GetLastWriteStatus();
            if (DgnDbStatus::Success != writeStatus)
                Fail(m_rows[entry.first], writeStatus, "unable to delete aspect");
            }

        for (auto r : generic)
            pendingByClass[m_rows[r].m_classId].push_back(r);
        }

    for (auto const& entry : pendingByClass)
        DeleteInstances(*m_rows[entry.second.front()].m_class, entry.second);

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
