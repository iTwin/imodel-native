/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

BEGIN_BENTLEY_DGN_NAMESPACE

#define TEMP_ELEMENT_DELETION "ElementsToDelete"

class BulkElementDeletion
    {
    using FkAction = DgnElements::FkOnDeleteAction;
    using FkConstraint = DgnElements::ElementFkConstraint;

    DgnDbR m_dgndb;
    DgnElements::ElementFkConstraintMap const& m_fkConstraints;
    DgnElementIdSet m_originalElementIds;
    std::vector<BulkDeleteFailure> m_failures;
    std::unordered_map<uint64_t, size_t> m_failureIndex; // elementId -> index into m_failures

    void AddFailure(DgnElementId id, BulkDeleteFailureReason reason, Utf8String detail = "")
        {
        const auto [it, inserted] = m_failureIndex.try_emplace(id.GetValue(), m_failures.size());
        if (inserted)
            {
            m_failures.push_back({ id, reason, std::move(detail) });
            return;
            }

        auto& existing = m_failures[it->second];
        if (static_cast<int>(reason) < static_cast<int>(existing.reason))
            {
            existing.reason = reason;
            existing.detail = std::move(detail);
            }
        }

    bool m_geometricElementsExist = false;
    bool m_subModelRootExists = false;
    bool m_prunedInRound = false;

    BulkDeleteMode m_mode = BulkDeleteMode::RemoveFKViolators;

    bool IsFailFastModeEnabled() const { return m_mode == BulkDeleteMode::FailFast; }

    static bool IsStructurallyHandled(FkConstraint const& fk)
        {
        return (fk.m_tableName.Equals(BIS_TABLE(BIS_CLASS_Element)) && fk.m_fromColumn.Equals("ParentId"))
            || (fk.m_tableName.Equals(BIS_TABLE(BIS_CLASS_Model))   && fk.m_fromColumn.Equals("ModeledElementId"));
        }

    std::vector<FkConstraint> const& GetFks(FkAction action) const
        {
        static const std::vector<FkConstraint> s_empty;
        const auto it = m_fkConstraints.find(action);
        return it == m_fkConstraints.end() ? s_empty : it->second;
        }

    // Create temporary tables for bulk deletion
    BeSQLite::DbResult CreateTempTables() const;
    BeSQLite::DbResult ExpandElementIdList();

    BeSQLite::DbResult FindConstraintViolators();
    BeSQLite::DbResult FindInUseDefinitionElements();
    BeSQLite::DbResult PruneViolators();
    void CollectFkViolations();

    bool FireAllCallbacks();
    BeSQLite::DbResult DeleteLinkTableRelationships() const;
    BeSQLite::DbResult ExecuteDeletion();

public:
    BulkElementDeletion(DgnDbR dgndb, DgnElements::ElementFkConstraintMap const& fkConstraints, const DgnElementIdSet& originalElementIds, BulkDeleteMode mode) 
        :   m_dgndb(dgndb), 
            m_fkConstraints(fkConstraints),
            m_originalElementIds(originalElementIds), 
            m_mode(mode) {}

    BulkDeleteElementsResult Execute();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BulkElementDeletion::CreateTempTables() const
    {
    const auto tempTableCreationStatements = {
        // Create the temp table if needed; subsequent calls reuse it after clearing stale entries.
        "CREATE TABLE IF NOT EXISTS " TEMP_TABLE(TEMP_ELEMENT_DELETION) " ("
            "ElementId              INTEGER PRIMARY KEY, "
            "LogicalParentId        INTEGER, "
            "IsSubModelRoot         INTEGER NOT NULL DEFAULT 0, "
            "Depth                  INTEGER NOT NULL DEFAULT 0, "
            "IsViolator             INTEGER NOT NULL DEFAULT 0, "
            "BlockedById            INTEGER, "
            "ECClassId              INTEGER)",

        // Set up indexes for faster lookups
        "CREATE INDEX IF NOT EXISTS idx_etd_logicalParent ON " TEMP_ELEMENT_DELETION " (LogicalParentId)",
        
        // Partial index to accelerate IsViolator=1 scans (PruneViolators, deleteAllViolators).
        "CREATE INDEX IF NOT EXISTS idx_etd_violator ON " TEMP_ELEMENT_DELETION " (IsViolator) WHERE IsViolator = 1",

        // Partial index to accelerate IsSubModelRoot=1 scans (DeleteLinkTableRelationships, ExecuteDeletion).
        "CREATE INDEX IF NOT EXISTS idx_etd_submodelroot ON " TEMP_ELEMENT_DELETION " (IsSubModelRoot) WHERE IsSubModelRoot = 1",

        // Clear out table to avoid stale entries
        "DELETE FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) "",
    };

    for (const auto& sql : tempTableCreationStatements)
        {
        if (const auto stat = m_dgndb.TryExecuteSql(sql); stat != BE_SQLITE_OK)
            {
            LOG.errorv("Error prepping elements for bulk deletion: %s", BeSQLiteLib::GetLogError(stat).c_str());
            return stat;
            }
        }
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BulkElementDeletion::ExpandElementIdList()
    {
    constexpr auto expandSql = 
    "WITH RECURSIVE fullSet(id, logicalParentId, isSubModelRoot, depth, classId) AS ("
        "SELECT e.Id, "
            "e.ParentId, "
            "CASE WHEN m.Id IS NOT NULL THEN 1 ELSE 0 END, "
            "0, "
            "e.ECClassId "
        "FROM bis_Element e "
        "LEFT JOIN bis_Model m ON m.ModeledElementId = e.Id "
        "WHERE InVirtualSet(?, e.Id) "
        "UNION "
        "SELECT e.Id,"
            "e.ParentId, "
            "CASE WHEN m.Id IS NOT NULL THEN 1 ELSE 0 END, "
            "f.depth + 1, "
            "e.ECClassId "
        "FROM bis_Element e "
        "INNER JOIN fullSet f ON e.ParentId = f.id "
        "LEFT JOIN bis_Model m ON m.ModeledElementId = e.Id "
        "UNION "
        // Elements contained in a sub-model whose root is being deleted.
        // Their logical parent is the modeled-element (sub-model root), not their SQLite ParentId.
        "SELECT e.Id,"
            "sm.ModeledElementId, "
            "CASE WHEN m.Id IS NOT NULL THEN 1 ELSE 0 END, "
            "f.depth + 1, "
            "e.ECClassId "
        "FROM bis_Element e "
        "INNER JOIN bis_Model sm ON sm.Id = e.ModelId "
        "INNER JOIN fullSet f ON sm.ModeledElementId = f.id "
        "LEFT JOIN bis_Model m ON m.ModeledElementId = e.Id "
    ") "
    "INSERT OR IGNORE INTO " TEMP_TABLE(TEMP_ELEMENT_DELETION) " (ElementId, LogicalParentId, IsSubModelRoot, Depth, ECClassId) "
    "SELECT id, logicalParentId, isSubModelRoot, depth, classId "
    "FROM fullSet";

    const auto expandStmt = m_dgndb.GetCachedStatement(expandSql);
    if (!expandStmt.IsValid())
        {
        LOG.error("BulkElementDeletion: Failed to add dependent elements");
        return BE_SQLITE_ERROR;
        }
    expandStmt->BindVirtualSet(1, m_originalElementIds);
    if (const auto stat = expandStmt->Step(); stat != BE_SQLITE_DONE)
        {
        LOG.errorv("BulkElementDeletion: Failed to add dependent elements: %s", BeSQLiteLib::GetLogError(stat).c_str());
        return stat;
        }

    // Check if any geometric elements are in the delete set to conditionally optimize the spatial index cleanup.
    const auto geomCheckStmt = m_dgndb.GetCachedStatement(
        "SELECT 1 FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t "
        "INNER JOIN " BIS_TABLE(BIS_CLASS_GeometricElement3d) " g ON g.ElementId = t.ElementId "
        "LIMIT 1");
    if (geomCheckStmt.IsValid() && geomCheckStmt->Step() == BE_SQLITE_ROW)
        m_geometricElementsExist = true;

    // Check if any sub-model roots are in the delete set; ExecuteDeletion/DeleteLinkTableRelationships need bis_Model cleanup in that case.
    const auto subModelCheckStmt = m_dgndb.GetCachedStatement(
        "SELECT 1 FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " WHERE IsSubModelRoot = 1 LIMIT 1");
    if (subModelCheckStmt.IsValid() && subModelCheckStmt->Step() == BE_SQLITE_ROW)
        m_subModelRootExists = true;

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BulkElementDeletion::PruneViolators()
    {
    // Requested ids being pruned that are not themselves direct violators (BlockedById == ElementId) were taken down by a
    // relative; report them with the id of the direct violator so the caller can pair them with its ReferencedByForeignKey /
    // DefinitionInUse entry. Error path only.
    auto collectStmt = m_dgndb.GetCachedStatement(
        "SELECT ElementId, BlockedById FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " "
        "WHERE IsViolator = 1 AND BlockedById IS NOT ElementId AND InVirtualSet(?, ElementId)");
    if (collectStmt.IsNull())
        {
        LOG.error("BulkElementDeletion: Failed to collect constraint violators");
        return BE_SQLITE_ERROR;
        }
    collectStmt->BindVirtualSet(1, m_originalElementIds);
    while (BE_SQLITE_ROW == collectStmt->Step())
        AddFailure(collectStmt->GetValueId<DgnElementId>(0), BulkDeleteFailureReason::BlockedByRelative, collectStmt->GetValueId<DgnElementId>(1).ToHexStr());

    if (IsFailFastModeEnabled())
        {
        if (const auto stat = m_dgndb.TryExecuteSql("UPDATE " TEMP_TABLE(TEMP_ELEMENT_DELETION) " SET IsViolator = 2 WHERE IsViolator = 1"); stat != BE_SQLITE_OK)
            {
            LOG.errorv("BulkElementDeletion: Failed to stamp reported violators: %s", BeSQLiteLib::GetLogError(stat).c_str());
            return stat;
            }
        return BE_SQLITE_OK;
        }

    constexpr auto deleteAllViolators = "DELETE FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " WHERE IsViolator = 1";
    if (const auto stat = m_dgndb.TryExecuteSql(deleteAllViolators); stat != BE_SQLITE_OK)
        {
        LOG.errorv("BulkElementDeletion: Failed to delete all constraint violators: %s", BeSQLiteLib::GetLogError(stat).c_str());
        return stat;
        }

    if (m_dgndb.GetModifiedRowCount() > 0)
        m_prunedInRound = true;

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BulkElementDeletion::CollectFkViolations()
    {
    for (const auto& fk : GetFks(FkAction::NoAction))
        {
        if (IsStructurallyHandled(fk))
            continue;

        const Utf8String sql = fk.m_elementIdColumn.empty()
            ? SqlPrintfString("SELECT DISTINCT t.ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t JOIN [%s] r ON r.[%s] = t.ElementId",
                                fk.m_tableName.c_str(), fk.m_fromColumn.c_str()).GetUtf8CP()
            : SqlPrintfString("SELECT DISTINCT t.ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t JOIN [%s] r ON r.[%s] = t.ElementId "
                                "LEFT JOIN " TEMP_TABLE(TEMP_ELEMENT_DELETION) " td ON td.ElementId = r.[%s] WHERE td.ElementId IS NULL",
                                fk.m_tableName.c_str(), fk.m_fromColumn.c_str(), fk.m_elementIdColumn.c_str()).GetUtf8CP();

        auto stmt = m_dgndb.GetCachedStatement(sql.c_str());
        if (stmt.IsNull())
            continue;

        const Utf8String tableAndColumn = fk.m_tableName + "." + fk.m_fromColumn;
        while (BE_SQLITE_ROW == stmt->Step())
            AddFailure(stmt->GetValueId<DgnElementId>(0), BulkDeleteFailureReason::ReferencedByForeignKey, tableAndColumn);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BulkElementDeletion::FindConstraintViolators()
    {
    // A delete-set element `t` is a direct violator if some row `r` of the referencing table points at it and `r` is not itself being deleted.
    // "r is being deleted" can only be established when the referencing row *is* an element (m_elementIdColumn set).
    Utf8String predicates;
    for (const auto& fk : GetFks(FkAction::NoAction))
        {
        if (IsStructurallyHandled(fk))
            continue;

        if (!predicates.empty())
            predicates.append(" OR ");

        if (fk.m_elementIdColumn.empty())
            predicates.append(SqlPrintfString(
                "EXISTS (SELECT 1 FROM [%s] r WHERE r.[%s] = t.ElementId)",
                fk.m_tableName.c_str(), fk.m_fromColumn.c_str()));
        else
            predicates.append(SqlPrintfString(
                "EXISTS (SELECT 1 FROM [%s] r "
                        "LEFT JOIN " TEMP_TABLE(TEMP_ELEMENT_DELETION) " td ON td.ElementId = r.[%s] "
                        "WHERE r.[%s] = t.ElementId AND td.ElementId IS NULL)",
                fk.m_tableName.c_str(), fk.m_elementIdColumn.c_str(), fk.m_fromColumn.c_str()));
        }

    if (predicates.empty())
        return BE_SQLITE_OK; // no NO ACTION FKs to bis_Element at all

    const Utf8String guardSql = "SELECT 1 FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t WHERE " + predicates + " LIMIT 1";
    auto guardStmt = m_dgndb.GetCachedStatement(guardSql.c_str());
    if (guardStmt.IsNull())
        {
        LOG.error("BulkElementDeletion: Constraint violator guard check failed");
        return BE_SQLITE_ERROR;
        }

    if (const auto stat = guardStmt->Step(); BE_SQLITE_ROW != stat)
        return BE_SQLITE_OK;

    // Violators exist, so we are on the error path.
    CollectFkViolations();

    if (IsFailFastModeEnabled())
        LOG.errorv("BulkElementDeletion: %zu element(s) in the delete set are still referenced via foreign keys from elements outside the set", m_failures.size());

    const Utf8String findViolators =
        "WITH "
        // directViolators: elements in the delete set that are directly referenced by a surviving row via a NO ACTION
        "directViolators AS ("
            "SELECT DISTINCT t.ElementId "
            "FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t "
            "WHERE " + predicates +
        "), "
        // ancestors: walk UP the LogicalParentId chain for each direct violator, collecting all ancestor rows still in the delete set.
        // The walk terminates when LogicalParentId is NULL or points outside the delete set. violatorId is carried along for diagnostics.
        "ancestors(id, logicalParentId, violatorId) AS ("
            "SELECT t.ElementId, t.LogicalParentId, t.ElementId "
            "FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t "
            "WHERE t.ElementId IN (SELECT ElementId FROM directViolators) "
            "UNION ALL "
            "SELECT t.ElementId, t.LogicalParentId, a.violatorId "
            "FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t "
            "INNER JOIN ancestors a ON t.ElementId = a.logicalParentId"
        "), "
        // subtreeRoots: the topmost element from the ancestor walk - i.e. the element whose logical parent is either NULL or is outside the delete set.
        // This is the root of the subtree that must be fully pruned. If several violators share a root, any one of them is a valid explanation.
        "subtreeRoots AS ("
            "SELECT a.id, MIN(a.violatorId) AS violatorId FROM ancestors a "
            "LEFT JOIN " TEMP_TABLE(TEMP_ELEMENT_DELETION) " p ON p.ElementId = a.logicalParentId "
            "WHERE a.logicalParentId IS NULL OR p.ElementId IS NULL "
            "GROUP BY a.id"
        "), "
        // subtree: walk DOWN from each subtree root, collecting the root and all of its logical descendants that are in the delete set.
        "subtree(id, violatorId) AS ("
            "SELECT id, violatorId FROM subtreeRoots "
            "UNION ALL "
            "SELECT t.ElementId, s.violatorId "
            "FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t "
            "INNER JOIN subtree s ON t.LogicalParentId = s.id"
        ") "
        // Mark the entire subtree as violators so PruneViolators() can remove them in one pass. Direct violators keep BlockedById == ElementId.
        "UPDATE " TEMP_TABLE(TEMP_ELEMENT_DELETION) " SET IsViolator = 1, "
            "BlockedById = CASE WHEN ElementId IN (SELECT ElementId FROM directViolators) THEN ElementId ELSE s.violatorId END "
        "FROM (SELECT id, MIN(violatorId) AS violatorId FROM subtree GROUP BY id) s "
        "WHERE " TEMP_TABLE(TEMP_ELEMENT_DELETION) ".ElementId = s.id";

    if (const auto stat = m_dgndb.TryExecuteSql(findViolators.c_str()); stat != BE_SQLITE_OK)
        {
        LOG.errorv("BulkElementDeletion: Failed to find constraint violators: %s", BeSQLiteLib::GetLogError(stat).c_str());
        return stat;
        }

    return PruneViolators();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BulkElementDeletion::FindInUseDefinitionElements()
    {
    // DefinitionElements (categories, materials, view definitions, etc.) may be referenced
    // by elements that are NOT in the delete set.  Deleting a definition that is still in
    // use would leave dangling references and corrupt the iModel.

    // Collect only the DefinitionElement rows from the temp table.
    // Non-definition elements in the set don't need usage checking.
    constexpr auto defIdsSql = 
        "SELECT t.ElementId "
        "FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t "
        "INNER JOIN bis_DefinitionElement d ON d.ElementId = t.ElementId";

    auto defStmt = m_dgndb.GetCachedStatement(defIdsSql);
    if (!defStmt.IsValid())
        {
        LOG.error("BulkElementDeletion: Failed to query for definition elements");
        return BE_SQLITE_ERROR;
        }

    DgnElementIdSet definitionIds;
    while (BE_SQLITE_ROW == defStmt->Step())
        {
        if (auto id = defStmt->GetValueId<DgnElementId>(0); id.IsValid())
            definitionIds.insert(id);
        }

    if (definitionIds.empty())
        return BE_SQLITE_OK;

    // Exclude intra-set usages from the usage check: elements referencing
    // each other inside the delete set are fine since all will be deleted.
    auto excludeIds = std::make_shared<BeSQLite::IdSet<BeInt64Id>>();
    auto allIdsStmt = m_dgndb.GetCachedStatement("SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION));
    if (!allIdsStmt.IsValid())
        {
        LOG.error("BulkElementDeletion: Failed to query delete set for definition usage check");
        return BE_SQLITE_ERROR;
        }
    while (BE_SQLITE_ROW == allIdsStmt->Step())
        excludeIds->insert(allIdsStmt->GetValueId<BeInt64Id>(0));

    auto usageInfo = DefinitionElementUsageInfo::Create(m_dgndb, definitionIds, excludeIds);
    if (!usageInfo.IsValid())
        return BE_SQLITE_OK; // no usage info means nothing blocks deletion

    DgnElementIdSet inUse;
    for (const auto& id : definitionIds)
        {
        if (usageInfo->GetUsedIds().Contains(id))
            inUse.insert(id);
        }

    if (inUse.empty())
        return BE_SQLITE_OK;

    for (const auto& id : inUse)
        AddFailure(id, BulkDeleteFailureReason::DefinitionInUse);

    if (IsFailFastModeEnabled())
        LOG.errorv("BulkElementDeletion: %zu DefinitionElement(s) in the delete set are still in use by elements outside the set", inUse.size());

    constexpr auto findDependents = 
        "WITH RECURSIVE ancestry(id, logicalParentId, violatorId) AS ("
            // Seed: every in-use definition element that is in the delete set.
            "SELECT ElementId, LogicalParentId, ElementId "
            "FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " "
            "WHERE InVirtualSet(?, ElementId) "

            "UNION ALL "

            // Recurse upward: the logical parent of the current element (if still in the
            // delete set), so we reach the topmost ancestor that owns the in-use definition.
            "SELECT t.ElementId, t.LogicalParentId, a.violatorId "
            "FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t "
            "INNER JOIN ancestry a ON t.ElementId = a.logicalParentId"
        ") "
        "UPDATE " TEMP_TABLE(TEMP_ELEMENT_DELETION) " SET IsViolator = 1, BlockedById = s.violatorId "
        "FROM (SELECT id, MIN(violatorId) AS violatorId FROM ancestry GROUP BY id) s "
        "WHERE " TEMP_TABLE(TEMP_ELEMENT_DELETION) ".ElementId = s.id AND " TEMP_TABLE(TEMP_ELEMENT_DELETION) ".IsViolator = 0";

    const auto findDependentsStmt = m_dgndb.GetCachedStatement(findDependents);
    if (!findDependentsStmt.IsValid())
        {
        LOG.error("BulkElementDeletion: Failed to query for dependent elements");
        return BE_SQLITE_ERROR;
        }
    findDependentsStmt->BindVirtualSet(1, inUse);
    if (const auto stat = findDependentsStmt->Step(); stat != BE_SQLITE_DONE)
        {
        LOG.errorv("BulkElementDeletion: Failed to prune in-use definition elements: %s", BeSQLiteLib::GetLogError(stat).c_str());
        return stat;
        }

    return PruneViolators();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BulkElementDeletion::FireAllCallbacks()
    {
    bool           jsAvailable = false;
    Napi::Object   jsDgnDb;
    Napi::Function getJsClassFn;
    Napi::Env      env(nullptr);

    if (auto jsDb = m_dgndb.GetJsIModelDb())
        {
        jsDgnDb = jsDb->Value();
        env = jsDgnDb.Env();
        auto fn = jsDgnDb.Get("getJsClass");
        if (fn.IsFunction())
            {
            getJsClassFn = fn.As<Napi::Function>();
            jsAvailable  = true;
            }
        }

    struct NapiElementDeletionBatch
        {
        Napi::Object jsClass;
        Napi::Array  arr;
        uint32_t     count = 0;
        };
    struct NapiModelDeletionBatch
        {
        Napi::Object jsClass;
        Napi::Array  deletedModelIdsArr;
        uint32_t     deletedModelCount = 0;
        std::unordered_map<uint64_t, std::pair<Napi::Array, uint32_t>> elementsByModel; // modelId → (elementIds[], count)
        };

    // Resolve a numeric ECClassId to the corresponding JS class object.
    auto resolveJsClass = [&](DgnClassId classId) -> Napi::Object
        {
        auto ecClass = m_dgndb.Schemas().GetClass(classId);
        if (!ecClass)
            return Napi::Object();
        auto cv = getJsClassFn.Call(jsDgnDb, {m_dgndb.ToJsString(ecClass->GetFullName())});
        return cv.IsObject() ? cv.As<Napi::Object>() : Napi::Object();
        };

    // Per-element metadata is joined in here (read-only, after pruning) rather than written into the temp table up front.
    // ParentId / ParentClassId are reported only when the parent is NOT itself in the delete set (tp.ElementId IS NULL).
    const auto stmt = m_dgndb.GetCachedStatement(
        "SELECT t.ElementId, t.ECClassId, e.ModelId, "
            "CASE WHEN tp.ElementId IS NULL THEN e.ParentId END, "
            "CASE WHEN tp.ElementId IS NULL THEN pe.ECClassId END, "
            "t.IsSubModelRoot, e.FederationGuid, subm.ECClassId, cm.ECClassId "
        "FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t "
        "JOIN " BIS_TABLE(BIS_CLASS_Element) " e ON e.Id = t.ElementId "
        "JOIN " BIS_TABLE(BIS_CLASS_Model) " cm ON cm.Id = e.ModelId "
        "LEFT JOIN " BIS_TABLE(BIS_CLASS_Model) " subm ON subm.ModeledElementId = e.Id "
        "LEFT JOIN " BIS_TABLE(BIS_CLASS_Element) " pe ON pe.Id = e.ParentId "
        "LEFT JOIN " TEMP_TABLE(TEMP_ELEMENT_DELETION) " tp ON tp.ElementId = e.ParentId "
        "ORDER BY t.ElementId");
    if (!stmt.IsValid())
        {
        LOG.error("BulkElementDeletion: Failed to prepare FireAllCallbacks statement");
        return false;
        }

    std::unordered_map<uint64_t, DgnElementCPtr> loadedParents;
    std::unordered_map<uint64_t, DgnModelPtr> modelCache;

    std::map<DgnClassId, NapiElementDeletionBatch> batchByClass;
    std::map<DgnClassId, NapiElementDeletionBatch> batchByParentClass;
    std::map<DgnClassId, NapiModelDeletionBatch>   modelClassBatches;

    auto getCachedModel = [&](DgnModelId mId) -> DgnModelPtr
        {
        auto it = modelCache.find(mId.GetValue());
        if (it != modelCache.end())
            return it->second;

        auto model = m_dgndb.Models().GetModel(mId);
        modelCache.emplace(mId.GetValue(), model);
        return model;
        };

    DgnElementIdSet allIds;

    while (BE_SQLITE_ROW == stmt->Step())
        {
        const DgnElementId elementId            = stmt->GetValueId<DgnElementId>(0);
        const DgnClassId   classId              = stmt->GetValueId<DgnClassId>(1);
        const DgnModelId   modelId              = stmt->GetValueId<DgnModelId>(2);
        const DgnElementId parentId             = stmt->GetValueId<DgnElementId>(3);
        const DgnClassId   parentClassId        = stmt->GetValueId<DgnClassId>(4);
        const bool         isSubModelRoot       = (stmt->GetValueInt(5) != 0);
        const DgnClassId   subModelClassId      = stmt->GetValueId<DgnClassId>(7);
        const DgnClassId   containingModelClassId = stmt->GetValueId<DgnClassId>(8);

        allIds.insert(elementId);

        BeGuid federationGuid;
        if (stmt->GetColumnType(6) != DbValueType::NullVal)
            federationGuid = stmt->GetValueGuid(6);

        if (jsAvailable)
            {
            auto& elemBatch = batchByClass[classId];
            if (elemBatch.jsClass.IsEmpty())
                {
                elemBatch.jsClass = resolveJsClass(classId);
                if (!elemBatch.jsClass.IsEmpty())
                    elemBatch.arr = Napi::Array::New(env);
                }
            if (!elemBatch.jsClass.IsEmpty())
                {
                BeJsNapiObject arg(env);
                arg["id"]    = elementId.ToHexStr();
                arg["model"] = modelId.ToHexStr();
                if (federationGuid.IsValid())
                    arg["federationGuid"] = federationGuid.ToString();
                if (isSubModelRoot)
                    arg["subModelId"] = elementId.ToHexStr();
                elemBatch.arr.Set(elemBatch.count++, static_cast<Napi::Object>(arg));
                }

            if (parentId.IsValid() && parentClassId.IsValid())
                {
                auto& childBatch = batchByParentClass[parentClassId];
                if (childBatch.jsClass.IsEmpty())
                    {
                    childBatch.jsClass = resolveJsClass(parentClassId);
                    if (!childBatch.jsClass.IsEmpty())
                        childBatch.arr = Napi::Array::New(env);
                    }
                if (!childBatch.jsClass.IsEmpty())
                    {
                    BeJsNapiObject arg(env);
                    arg["parentId"] = parentId.ToHexStr();
                    arg["childId"]  = elementId.ToHexStr();
                    childBatch.arr.Set(childBatch.count++, static_cast<Napi::Object>(arg));
                    }
                }

            if (isSubModelRoot && subModelClassId.IsValid())
                {
                auto& modelBatch = modelClassBatches[subModelClassId];
                if (modelBatch.jsClass.IsEmpty())
                    modelBatch.jsClass = resolveJsClass(subModelClassId);
                if (!modelBatch.jsClass.IsEmpty())
                    {
                    if (modelBatch.deletedModelIdsArr.IsEmpty())
                        modelBatch.deletedModelIdsArr = Napi::Array::New(env);
                    modelBatch.deletedModelIdsArr.Set(modelBatch.deletedModelCount++, Napi::String::New(env, elementId.ToHexStr()));
                    }
                }

            if (containingModelClassId.IsValid())
                {
                auto& modelBatch = modelClassBatches[containingModelClassId];
                if (modelBatch.jsClass.IsEmpty())
                    modelBatch.jsClass = resolveJsClass(containingModelClassId);
                if (!modelBatch.jsClass.IsEmpty())
                    {
                    auto& [modelArr, modelCount] = modelBatch.elementsByModel[modelId.GetValue()];
                    if (modelArr.IsEmpty())
                        modelArr = Napi::Array::New(env);
                    modelArr.Set(modelCount++, Napi::String::New(env, elementId.ToHexStr()));
                    }
                }
            }

        DgnElementCPtr element = m_dgndb.Elements().FindLoadedElement(elementId);
        if (element.IsValid())
            {
            element->_OnDelete();
            element->_OnDeleted();
            }

        if (auto model = getCachedModel(modelId); model.IsValid())
            {
            if (element.IsValid())
                model->_OnDeleteElement(*element);
            model->_OnDeletedElement(elementId);

            if (isSubModelRoot)
                {
                model->_OnDeleteNotify();
                model->_OnDeleted();
                }
            }

        if (parentId.IsValid())
            {
            // Only fire child callbacks for parents that are NOT also being deleted.
            // ParentId in the temp table is already set to NULL for parents in the delete set
            // but we double-check here via the cache.
            auto it = loadedParents.find(parentId.GetValue());
            if (it == loadedParents.end())
                {
                DgnElementCPtr parentElement = m_dgndb.Elements().FindLoadedElement(parentId);
                it = loadedParents.emplace(parentId.GetValue(), parentElement).first;
                }
            if (it->second.IsValid() && element.IsValid())
                {
                it->second->_OnChildDelete(*element);
                it->second->_OnChildDeleted(*element);
                }
            }
        }

    // Evict all deleted elements from the MRU pool in one pass, before the JS callbacks fire.
    m_dgndb.Elements().DropFromPool(allIds);

    if (!jsAvailable)
        return true;

    for (auto& [classId, batch] : batchByClass)
        {
        if (batch.jsClass.IsEmpty())
            continue;
        // Pass the element list to the JS class to fire higher-level change events for all deleted elements of this class at once.
        BeJsNapiObject batchArg(env);
        static_cast<Napi::Object>(batchArg).Set("iModel", jsDgnDb);
        static_cast<Napi::Object>(batchArg).Set("elements", batch.arr);
        DgnDb::CallJsFunction(batch.jsClass, "onBulkDeleted", {static_cast<Napi::Object>(batchArg)});
        }

    for (auto& [parentClassId, batch] : batchByParentClass)
        {
        if (batch.jsClass.IsEmpty())
            continue;
        // Pass parent and child element lists so JS can update child-list caches on surviving parent elements.
        BeJsNapiObject batchArg(env);
        static_cast<Napi::Object>(batchArg).Set("iModel", jsDgnDb);
        static_cast<Napi::Object>(batchArg).Set("elements", batch.arr);
        DgnDb::CallJsFunction(batch.jsClass, "onBulkChildDeleted", {static_cast<Napi::Object>(batchArg)});
        }

    for (auto& [classId, batch] : modelClassBatches)
        {
        if (batch.jsClass.IsEmpty())
            continue;

        BeJsNapiObject arg(env);

        if (batch.deletedModelCount > 0)
            static_cast<Napi::Object>(arg).Set("deletedModelIds", batch.deletedModelIdsArr);

        if (!batch.elementsByModel.empty())
            {
            auto outerArr = Napi::Array::New(env, batch.elementsByModel.size());
            uint32_t i = 0;
            for (auto& [modelIdVal, pair] : batch.elementsByModel)
                {
                BeJsNapiObject entry(env);
                entry["id"] = DgnModelId(modelIdVal).ToHexStr();
                static_cast<Napi::Object>(entry).Set("elementIds", pair.first);
                outerArr.Set(i++, static_cast<Napi::Object>(entry));
                }
            static_cast<Napi::Object>(arg).Set("deletedElementsByModel", outerArr);
            }

        static_cast<Napi::Object>(arg).Set("iModel", jsDgnDb);
        DgnDb::CallJsFunction(batch.jsClass, "onBulkModelEvents", {static_cast<Napi::Object>(arg)});
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BulkElementDeletion::ExecuteDeletion()
    {
    // Explicitly execute ON DELETE CASCADE and ON DELETE SET NULL FK actions for all tables that reference bis_Element.
    {
    const auto changeMode = m_dgndb.Txns().GetMode();

    m_dgndb.Txns().SetMode(ChangeTracker::Mode::Indirect);  // The cascade changes should be tracked as indirect changes

    for (const auto& fk : GetFks(FkAction::Cascade))
        {
        if (IsStructurallyHandled(fk)) // bis_Element.ParentId: children are already in the delete set
            continue;

        const auto stat = m_dgndb.TryExecuteSql(SqlPrintfString(
            "DELETE FROM [%s] WHERE [%s] IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) ")",
            fk.m_tableName.c_str(), fk.m_fromColumn.c_str()));
        if (stat != BE_SQLITE_OK)
            {
            LOG.errorv("BulkElementDeletion: ON DELETE CASCADE emulation failed for %s.%s: %s", fk.m_tableName.c_str(), fk.m_fromColumn.c_str(), BeSQLiteLib::GetLogError(stat).c_str());
            return stat;
            }
        }

    for (const auto& fk : GetFks(FkAction::SetNull))
        {
        const auto stat = m_dgndb.TryExecuteSql(SqlPrintfString(
            "UPDATE [%s] SET [%s] = NULL WHERE [%s] IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) ")",
            fk.m_tableName.c_str(), fk.m_fromColumn.c_str(), fk.m_fromColumn.c_str()));
        if (stat != BE_SQLITE_OK)
            {
            LOG.errorv("BulkElementDeletion: ON DELETE SET NULL emulation failed for %s.%s: %s", fk.m_tableName.c_str(), fk.m_fromColumn.c_str(), BeSQLiteLib::GetLogError(stat).c_str());
            return stat;
            }
        }
    m_dgndb.Txns().SetMode(changeMode); // restore original transaction mode
    }

    bool fkDisabled = false;
    bool fkEnabled = false;
    if (BE_SQLITE_OK == m_dgndb.QueryForeignKeyEnforcement(fkEnabled) && fkEnabled)
        fkDisabled = (BE_SQLITE_OK == m_dgndb.SetForeignKeyEnforcement(false));

    auto reset = [&]() {
        if (fkDisabled && BE_SQLITE_OK != m_dgndb.SetForeignKeyEnforcement(true))
            LOG.error("BulkElementDeletion: failed to restore foreign key enforcement");
        m_dgndb.EndPurgeOperation();
        m_dgndb.TryExecuteSql("PRAGMA synchronous = NORMAL");
        m_dgndb.TryExecuteSql("PRAGMA cache_size = -2000");
        m_dgndb.TryExecuteSql("PRAGMA defer_foreign_keys = false");
    };

    // Prep the db and handlers for a bulk delete
    m_dgndb.BeginPurgeOperation();
    m_dgndb.TryExecuteSql("PRAGMA synchronous = OFF");
    m_dgndb.TryExecuteSql("PRAGMA cache_size = -131072");

    if (m_geometricElementsExist)
        m_dgndb.TryExecuteSql("DELETE FROM " DGN_VTABLE_SpatialIndex " WHERE ElementId IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) ")");

    // Only needed while FKs are enforced: bis_Model.ModeledElementId (NO ACTION) points at sub-model roots that leave bis_Element before bis_Model.
    if (m_subModelRootExists && !fkDisabled)
        m_dgndb.TryExecuteSql("PRAGMA defer_foreign_keys = true");

    if (const auto stat = m_dgndb.TryExecuteSql("DELETE FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE Id IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) ")"); stat != BE_SQLITE_OK)
        {
        LOG.errorv("BulkElementDeletion: Element deletion failed: %s", BeSQLiteLib::GetLogError(stat).c_str());
        reset();
        return stat;
        }

    if (m_subModelRootExists)
        {
        if (const auto stat = m_dgndb.TryExecuteSql("DELETE FROM " BIS_TABLE(BIS_CLASS_Model) " WHERE Id IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " WHERE IsSubModelRoot = 1)"); stat != BE_SQLITE_OK)
            {
            LOG.errorv("BulkElementDeletion: Sub-model root deletion failed: %s", BeSQLiteLib::GetLogError(stat).c_str());
            reset();
            return stat;
            }
        }

    reset();
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BulkElementDeletion::DeleteLinkTableRelationships() const
    {
    for (const auto& tableName : { BIS_TABLE(BIS_REL_ElementRefersToElements), BIS_TABLE(BIS_REL_ElementDrivesElement) })
        {
        auto stat = m_dgndb.TryExecuteSql(Utf8PrintfString("DELETE FROM %s WHERE SourceId IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) ")", tableName).c_str());
        if (stat != BE_SQLITE_OK)
            {
            LOG.errorv("BulkElementDeletion: Link table SourceId deletion failed: %s", BeSQLiteLib::GetLogError(stat).c_str());
            return stat;
            }

        stat = m_dgndb.TryExecuteSql(Utf8PrintfString("DELETE FROM %s WHERE TargetId IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) ")", tableName).c_str());
        if (stat != BE_SQLITE_OK)
            {
            LOG.errorv("BulkElementDeletion: Link table TargetId deletion failed: %s", BeSQLiteLib::GetLogError(stat).c_str());
            return stat;
            }
        }

    if (m_subModelRootExists)
        {
        const auto stat = m_dgndb.TryExecuteSql("DELETE FROM " BIS_TABLE(BIS_REL_ModelSelectorRefersToModels) " WHERE TargetId IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " WHERE IsSubModelRoot = 1)");
        if (stat != BE_SQLITE_OK)
            {
            LOG.errorv("BulkElementDeletion: Sub-model root deletion failed: %s", BeSQLiteLib::GetLogError(stat).c_str());
            return stat;
            }
        }

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BulkDeleteElementsResult BulkElementDeletion::Execute()
    {
    // Bulk Element Deletion algorithm:
    //
    //  1. CreateTempTables      – set up temp.ElementsToDelete scratch space.
    //  2. ExpandElementIdList   – recursively pull in children and sub-model members.
    //  3. ANALYZE               – give SQLite fresh statistics for the temp table so the
    //                             query planner makes good choices in later steps.
    //  4. FindConstraintViolators       – elements still referenced via NO ACTION FKs to bis_Element
    //  5. FindInUseDefinitionElements   – DefinitionElements still used by elements outside the set.
    //          (4 and 5 are repeated until a round prunes nothing, since pruning can expose new violators.)
    //  6. FireAllCallbacks      – dispatch C++ and JS pre-/post-delete notifications in bulk.
    //  7. ExecuteDeletion       – emulate CASCADE / SET NULL FKs (from the cache), then bulk DELETE from bis_Element
    //                             (and bis_Model for sub-model roots).
    //  8. DeleteLinkTableRelationships – clean up link-table rows for deleted endpoints.

    DgnDb::VerifyClientThread();

    m_dgndb.TryExecuteSql("PRAGMA temp_store = MEMORY");

    auto cleanup = [&]() {
        m_dgndb.TryExecuteSql("PRAGMA temp_store = DEFAULT");
        m_dgndb.Elements().SetBulkOperation(false);
    };

    // Internal failure
    auto fail = [&](DbResult stat) -> BulkDeleteElementsResult {
        cleanup();
        return { BulkDeleteElementsStatus::DeletionFailed, stat };
    };

    if (const auto stat = CreateTempTables(); stat != BE_SQLITE_OK)
        return fail(stat);

    if (const auto stat = ExpandElementIdList(); stat != BE_SQLITE_OK)
        return fail(stat);

    // Run ANALYZE after populating the temp table so that SQLite has up-to-date statistics
    // for the subsequent constraint-checking and deletion queries.
    m_dgndb.TryExecuteSql("ANALYZE " TEMP_TABLE(TEMP_ELEMENT_DELETION));
    m_dgndb.Elements().SetBulkOperation(true);

    // Handle transitive fk violations which will only get exposed after the dependent element is pruned.
    // The intra-set checks will have ignored it till then.
    for (;;)
        {
        m_prunedInRound = false;

        if (const auto stat = FindConstraintViolators(); stat != BE_SQLITE_OK)
            return fail(stat);

        if (const auto stat = FindInUseDefinitionElements(); stat != BE_SQLITE_OK)
            return fail(stat);

        if (!m_prunedInRound)
            break;
        }

    // FailFast: nothing has been modified yet, so bail out and report exactly what blocks the operation.
    if (IsFailFastModeEnabled() && !m_failures.empty())
        {
        cleanup();
        return { BulkDeleteElementsStatus::ConstraintViolation, BE_SQLITE_OK, std::move(m_failures) };
        }

    // Fire the pre and post delete callbacks at once
    if (!FireAllCallbacks())
        return fail(BE_SQLITE_OK);

    // Execute the actual SQL DELETE. Any failure here is fatal for the whole operation: part of the delete set may already
    // have been removed, so the caller must abandon the enclosing txn.
    if (const auto stat = ExecuteDeletion(); stat != BE_SQLITE_OK)
        return fail(stat);

    // Clean up link-table relationships
    if (const auto stat = DeleteLinkTableRelationships(); stat != BE_SQLITE_OK)
        return fail(stat);

    cleanup();

    // Success only when every requested element was deleted; PartialSuccess when some were skipped due to constraints.
    const auto status = m_failures.empty() ? BulkDeleteElementsStatus::Success : BulkDeleteElementsStatus::PartialSuccess;
    return { status, BE_SQLITE_OK, std::move(m_failures) };
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BulkDeleteElementsResult DgnElements::DeleteElements(const DgnElementIdSet& elementIds, BulkDeleteMode mode)
    {
    DgnDb::VerifyClientThread();
    if (elementIds.empty())
        return { BulkDeleteElementsStatus::Success, BE_SQLITE_OK };

    // Expected to populate once per DgnDb (reset by ClearECCaches)
    CacheFkConstraints();

    return BulkElementDeletion(m_dgndb, GetFkConstraints(), elementIds, mode).Execute();
    }

END_BENTLEY_DGN_NAMESPACE