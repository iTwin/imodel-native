/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

BEGIN_BENTLEY_DGN_NAMESPACE

#define TEMP_ELEMENT_DELETION "ElementsToDelete"

class BulkElementDeletion
    {
    DgnDbR m_dgndb;
    DgnElementIdSet m_originalElementIds;
    DgnElementIdSet m_failedToDelete;

    bool m_geometricElementsExist = false;
    bool m_subModelRootExists = false;
    
    bool m_skipFKConstraintValidations = false;

    // Create temporary tables for bulk deletion
    BeSQLite::DbResult CreateTempTables() const;
    BeSQLite::DbResult ExpandElementIdList();
    int GetTempTableRowCount() const;

    // Find and prune constraint violators
    BeSQLite::DbResult FindAndPruneConstraintViolators();
    BeSQLite::DbResult FindAndPruneInUseDefinitionElements();
    BeSQLite::DbResult PruneViolators();

    bool FireAllCallbacks();
    BeSQLite::DbResult DeleteLinkTableRelationships() const;
    BeSQLite::DbResult ExecuteDeletion();

public:
    BulkElementDeletion(DgnDbR dgndb, const DgnElementIdSet& originalElementIds, bool skipFKConstraintValidations) 
        :   m_dgndb(dgndb), 
            m_originalElementIds(originalElementIds), 
            m_skipFKConstraintValidations(skipFKConstraintValidations) {}

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
            "ECClassId              INTEGER, "
            "ModelId                INTEGER, "
            "ParentId               INTEGER, "
            "ParentClassId          INTEGER, "
            "FederationGuid         BLOB, "
            "ContainingModelClassId INTEGER, "
            "SubModelClassId        INTEGER)",

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
    "WITH RECURSIVE fullSet(id, logicalParentId, isSubModelRoot, depth) AS ("
        "SELECT e.Id, "
            "e.ParentId, "
            "CASE WHEN m.Id IS NOT NULL THEN 1 ELSE 0 END, "
            "0 "
        "FROM bis_Element e "
        "LEFT JOIN bis_Model m ON m.ModeledElementId = e.Id "
        "WHERE InVirtualSet(?, e.Id) "
        "UNION "
        "SELECT e.Id,"
            "e.ParentId, "
            "CASE WHEN m.Id IS NOT NULL THEN 1 ELSE 0 END, "
            "f.depth + 1 "
        "FROM bis_Element e "
        "INNER JOIN fullSet f ON e.ParentId = f.id "
        "LEFT JOIN bis_Model m ON m.ModeledElementId = e.Id "
        "UNION "
        // Elements contained in a sub-model whose root is being deleted.
        // Their logical parent is the modeled-element (sub-model root), not their SQLite ParentId.
        "SELECT e.Id,"
            "sm.ModeledElementId, "
            "CASE WHEN m.Id IS NOT NULL THEN 1 ELSE 0 END, "
            "f.depth + 1 "
        "FROM bis_Element e "
        "INNER JOIN bis_Model sm ON sm.Id = e.ModelId "
        "INNER JOIN fullSet f ON sm.ModeledElementId = f.id "
        "LEFT JOIN bis_Model m ON m.ModeledElementId = e.Id "
    ") "
    "INSERT OR IGNORE INTO " TEMP_TABLE(TEMP_ELEMENT_DELETION) " (ElementId, LogicalParentId, IsSubModelRoot, Depth) "
    "SELECT id, logicalParentId, isSubModelRoot, depth "
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

    // Update the temp table with the additional metadata needed by FireAllCallbacks to dispatch JS events and by ExecuteDeletion for targeted cleanups.
    constexpr auto updateTableSql =
        "UPDATE " TEMP_TABLE(TEMP_ELEMENT_DELETION) " "
        "SET "
        "    ECClassId              = e.ECClassId, "
        "    ModelId                = e.ModelId, "
        "    FederationGuid         = e.FederationGuid, "
        "    ContainingModelClassId = cm.ECClassId, "
        "    SubModelClassId        = subm.ECClassId, "
        "    ParentId = CASE "
        "        WHEN e.ParentId IS NOT NULL "
        "         AND NOT EXISTS (SELECT 1 FROM temp.ElementsToDelete td WHERE td.ElementId = e.ParentId) "
        "        THEN e.ParentId ELSE NULL "
        "    END, "
        "    ParentClassId = CASE "
        "        WHEN e.ParentId IS NOT NULL "
        "         AND NOT EXISTS (SELECT 1 FROM temp.ElementsToDelete td WHERE td.ElementId = e.ParentId) "
        "        THEN (SELECT pe.ECClassId FROM " BIS_TABLE(BIS_CLASS_Element) " pe WHERE pe.Id = e.ParentId) "
        "        ELSE NULL "
        "    END "
        "FROM " BIS_TABLE(BIS_CLASS_Element) " e "
        "JOIN " BIS_TABLE(BIS_CLASS_Model) " cm ON cm.Id = e.ModelId "
        "LEFT JOIN " BIS_TABLE(BIS_CLASS_Model) " subm ON subm.ModeledElementId = e.Id "
        "WHERE temp.ElementsToDelete.ElementId = e.Id";

    if (const auto stat = m_dgndb.TryExecuteSql(updateTableSql); stat != BE_SQLITE_OK)
        {
        LOG.errorv("BulkElementDeletion: Failed to enrich temp table: %s", BeSQLiteLib::GetLogError(stat).c_str());
        return stat;
        }

    // Check if any geometric elements are in the delete set to conditionally optimize the spatial index cleanup.
    const auto geomCheckStmt = m_dgndb.GetCachedStatement(
        "SELECT 1 FROM " BIS_TABLE(BIS_CLASS_GeometricElement3d) " g "
        "WHERE EXISTS (SELECT 1 FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t WHERE t.ElementId = g.ElementId) "
        "LIMIT 1");
    if (geomCheckStmt.IsValid() && geomCheckStmt->Step() == BE_SQLITE_ROW)
        m_geometricElementsExist = true;

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BulkElementDeletion::PruneViolators()
    {
    // This is purely for error reporting.
    // While this is a hinderance for the performance, it is important to inform the caller exactly which elements failed deletion.
    constexpr auto collectViolatorsSql =
        "SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " WHERE IsViolator = 1";

    auto pruneStmt = m_dgndb.GetCachedStatement(collectViolatorsSql);
    if (!pruneStmt.IsValid())
        {
        LOG.error("BulkElementDeletion: Failed to find constraint violators");
        return BE_SQLITE_ERROR;
        }

    while (BE_SQLITE_ROW == pruneStmt->Step())
        {
        auto id = pruneStmt->GetValueId<DgnElementId>(0);
        if (id.IsValid() && m_originalElementIds.Contains(id))
            m_failedToDelete.insert(id);
        }

    constexpr auto deleteAllViolators = "DELETE FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " WHERE IsViolator = 1";
    if (const auto stat = m_dgndb.TryExecuteSql(deleteAllViolators); stat != BE_SQLITE_OK)
        {
        LOG.errorv("BulkElementDeletion: Failed to delete all constraint violators: %s", BeSQLiteLib::GetLogError(stat).c_str());
        return stat;
        }

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BulkElementDeletion::FindAndPruneConstraintViolators()
    {
    // Optimization guard: Don't run the expensive search queries if no violators exist
    constexpr auto guardSql = 
        "SELECT 1 FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t "
        "INNER JOIN bis_Element e ON e.CodeScopeId = t.ElementId "
        "LEFT JOIN " TEMP_TABLE(TEMP_ELEMENT_DELETION) " td ON td.ElementId = e.Id "
        "WHERE td.ElementId IS NULL "

        "UNION ALL "

        "SELECT 1 FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t "
        "INNER JOIN bis_GeometricElement3d g ON g.CategoryId = t.ElementId "
        "LEFT JOIN " TEMP_TABLE(TEMP_ELEMENT_DELETION) " td ON td.ElementId = g.ElementId "
        "WHERE td.ElementId IS NULL "

        "UNION ALL "

        "SELECT 1 FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t "
        "INNER JOIN bis_GeometricElement2d g ON g.CategoryId = t.ElementId "
        "LEFT JOIN " TEMP_TABLE(TEMP_ELEMENT_DELETION) " td ON td.ElementId = g.ElementId "
        "WHERE td.ElementId IS NULL "
        "LIMIT 1";

    auto guardStmt = m_dgndb.GetCachedStatement(guardSql);
    if (guardStmt.IsNull())
        {
        LOG.error("BulkElementDeletion: Constraint violator guard check failed");
        return BE_SQLITE_ERROR;
        }

    if (const auto stat = guardStmt->Step(); BE_SQLITE_ROW != stat)
        return BE_SQLITE_OK;

    constexpr auto findViolators =
        "WITH "
        // directViolators: elements in the delete set that are directly referenced by an element NOT in the delete set via a NO ACTION FK (CodeScopeId or CategoryId).
        "directViolators AS ("
            "SELECT DISTINCT t.ElementId "
            "FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t "
            "WHERE EXISTS ("
                "SELECT 1 FROM bis_Element e "
                "LEFT JOIN " TEMP_TABLE(TEMP_ELEMENT_DELETION) " td ON td.ElementId = e.Id "
                "WHERE e.CodeScopeId = t.ElementId AND td.ElementId IS NULL"
            ") "
            "OR EXISTS ("
                "SELECT 1 FROM bis_GeometricElement3d g3 "
                "LEFT JOIN " TEMP_TABLE(TEMP_ELEMENT_DELETION) " td ON td.ElementId = g3.ElementId "
                "WHERE g3.CategoryId = t.ElementId AND td.ElementId IS NULL"
            ") "
            "OR EXISTS ("
                "SELECT 1 FROM bis_GeometricElement2d g2 "
                "LEFT JOIN " TEMP_TABLE(TEMP_ELEMENT_DELETION) " td ON td.ElementId = g2.ElementId "
                "WHERE g2.CategoryId = t.ElementId AND td.ElementId IS NULL"
            ")"
        "), "
        // ancestors: walk UP the LogicalParentId chain for each direct violator, collecting all ancestor rows still in the delete set.
        // The walk terminates when LogicalParentId
        // is NULL or points outside the delete set.
        "ancestors(id, logicalParentId) AS ("
            "SELECT t.ElementId, t.LogicalParentId "
            "FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t "
            "WHERE t.ElementId IN (SELECT ElementId FROM directViolators) "
            "UNION ALL "
            "SELECT t.ElementId, t.LogicalParentId "
            "FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t "
            "INNER JOIN ancestors a ON t.ElementId = a.logicalParentId"
        "), "
        // subtreeRoots: the topmost element from the ancestor walk — i.e. the element whose logical parent is either NULL or is outside the delete set.  This is the root of
        // the subtree that must be fully pruned.
        "subtreeRoots AS ("
            "SELECT DISTINCT a.id FROM ancestors a "
            "LEFT JOIN " TEMP_TABLE(TEMP_ELEMENT_DELETION) " p ON p.ElementId = a.logicalParentId "
            "WHERE a.logicalParentId IS NULL OR p.ElementId IS NULL"
        "), "
        // subtree: walk DOWN from each subtree root, collecting the root and all of its
        // logical descendants that are in the delete set.
        "subtree(id) AS ("
            "SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " "
            "WHERE ElementId IN (SELECT id FROM subtreeRoots) "
            "UNION ALL "
            "SELECT t.ElementId "
            "FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t "
            "INNER JOIN subtree s ON t.LogicalParentId = s.id"
        ") "
        // Mark the entire subtree as violators so PruneViolators() can remove them in one pass.
        "UPDATE " TEMP_TABLE(TEMP_ELEMENT_DELETION) " SET IsViolator = 1 WHERE ElementId IN (SELECT id FROM subtree)";

    if (const auto stat = m_dgndb.TryExecuteSql(findViolators); stat != BE_SQLITE_OK)
        {
        LOG.errorv("BulkElementDeletion: Failed to find constraint violators: %s", BeSQLiteLib::GetLogError(stat).c_str());
        return stat;
        }

    return PruneViolators();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BulkElementDeletion::FindAndPruneInUseDefinitionElements()
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

    // Exclude intra-set usages from the usage check — elements referencing
    // each other inside the delete set are fine since all will be deleted.
    auto usageInfo = DefinitionElementUsageInfo::Create(m_dgndb, definitionIds, std::make_shared<BeSQLite::IdSet<BeInt64Id>>(BeIdSet(definitionIds.GetBeIdSet())));
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

    constexpr auto findDependents = 
        "WITH RECURSIVE ancestry(id, logicalParentId) AS ("
            // Seed: every in-use definition element that is in the delete set.
            "SELECT ElementId, LogicalParentId "
            "FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " "
            "WHERE InVirtualSet(?, ElementId) "

            "UNION ALL "

            // Recurse upward: the logical parent of the current element (if still in the
            // delete set), so we reach the topmost ancestor that owns the in-use definition.
            "SELECT t.ElementId, t.LogicalParentId "
            "FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t "
            "INNER JOIN ancestry a ON t.ElementId = a.logicalParentId"
        ") "
        "Update " TEMP_TABLE(TEMP_ELEMENT_DELETION) " SET IsViolator = 1 WHERE ElementId IN (SELECT id FROM ancestry)";

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

    const auto stmt = m_dgndb.GetCachedStatement("SELECT ElementId, ECClassId, ModelId, ParentId, ParentClassId, IsSubModelRoot, FederationGuid, SubModelClassId, ContainingModelClassId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION));
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
                    {
                    modelBatch.jsClass = resolveJsClass(subModelClassId);
                    if (!modelBatch.jsClass.IsEmpty())
                        modelBatch.deletedModelIdsArr = Napi::Array::New(env);
                    }
                if (!modelBatch.jsClass.IsEmpty())
                    modelBatch.deletedModelIdsArr.Set(modelBatch.deletedModelCount++, Napi::String::New(env, modelId.ToHexStr()));
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

        if (isSubModelRoot)
            m_subModelRootExists = true;

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

    Statement onDeleteStmt;
    onDeleteStmt.Prepare(m_dgndb, "SELECT name FROM main.sqlite_master WHERE type='table' AND sql LIKE '%REFERENCES%bis_Element%ON DELETE%' AND name != '" BIS_TABLE(BIS_CLASS_Element) "' ORDER BY name");
    while (BE_SQLITE_ROW == onDeleteStmt.Step())
        {
        const auto tableName = onDeleteStmt.GetValueText(0);
        Statement fkStmt;
        fkStmt.Prepare(m_dgndb, SqlPrintfString("PRAGMA foreign_key_list(%s)", tableName));
        while (BE_SQLITE_ROW == fkStmt.Step())
            {
            if (Utf8String(fkStmt.GetValueText(2)).CompareToIAscii(BIS_TABLE(BIS_CLASS_Element)) != 0)
                continue;

            const auto onDeleteAction = fkStmt.GetValueText(6); // on-delete action
            if (Utf8String(onDeleteAction).CompareToIAscii("CASCADE") == 0)
                {
                const auto stat = m_dgndb.TryExecuteSql(SqlPrintfString("DELETE FROM %s WHERE %s IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) ")", tableName, fkStmt.GetValueText(3) /* from-column */));
                if (stat != BE_SQLITE_OK)
                    {
                    LOG.errorv("BulkElementDeletion: ON DELETE CASCADE emulation failed for table '%s': %s", tableName, BeSQLiteLib::GetLogError(stat).c_str());
                    return stat;
                    }
                }
            else if (Utf8String(onDeleteAction).CompareToIAscii("SET NULL") == 0)
                {
                const auto columnName = fkStmt.GetValueText(3); // from-column
                const auto stat = m_dgndb.TryExecuteSql(SqlPrintfString("UPDATE %s SET %s = NULL WHERE %s IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) ")", tableName, columnName, columnName));
                if (stat != BE_SQLITE_OK)
                    {
                    LOG.errorv("BulkElementDeletion: ON DELETE SET NULL emulation failed for table '%s': %s", tableName, BeSQLiteLib::GetLogError(stat).c_str());
                    return stat;
                    }
                }
            }
        }
    m_dgndb.Txns().SetMode(changeMode); // restore original transaction mode
    }

    auto reset = [&]() {
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

    if (m_subModelRootExists)
        m_dgndb.TryExecuteSql("PRAGMA defer_foreign_keys = true");

    const auto stat = m_dgndb.TryExecuteSql("DELETE FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE Id IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) ")");
    if (stat != BE_SQLITE_OK)
        {
        LOG.errorv("BulkElementDeletion: Element deletion failed: %s", BeSQLiteLib::GetLogError(stat).c_str());
        reset();
        return stat;
        }

    if (m_subModelRootExists)
        {
        const auto modelStat = m_dgndb.TryExecuteSql("DELETE FROM " BIS_TABLE(BIS_CLASS_Model) " WHERE Id IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " WHERE IsSubModelRoot = 1)");
        if (modelStat != BE_SQLITE_OK)
            {
            LOG.errorv("BulkElementDeletion: Sub-model root deletion failed: %s", BeSQLiteLib::GetLogError(modelStat).c_str());
            reset();
            return modelStat;
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
int BulkElementDeletion::GetTempTableRowCount() const
    {
    const auto countStmt = m_dgndb.GetCachedStatement("SELECT COUNT(*) FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION));
    if (countStmt.IsNull())
        return 0;

    if (auto stat = countStmt->Step(); stat != BE_SQLITE_ROW)
        {
        LOG.errorv("BulkElementDeletion: Failed to get final row count for deletion: %s", BeSQLiteLib::GetLogError(stat).c_str());
        return 0;
        }

    return countStmt->GetValueInt(0);
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
    //  4. FindAndPruneConstraintViolators    – remove elements that would violate NO ACTION FKs (skipped when m_skipFKConstraintValidations is set).
    //  5. FindAndPruneInUseDefinitionElements – remove in-use DefinitionElements (skipped when m_skipFKConstraintValidations is set).
    //  6. FireAllCallbacks      – dispatch C++ and JS pre-/post-delete notifications in bulk.
    //  7. ExecuteDeletion       – bulk DELETE from bis_Element (and bis_Model for sub-model roots).
    //  8. DeleteLinkTableRelationships – clean up link-table rows for deleted endpoints.

    DgnDb::VerifyClientThread();

    m_dgndb.TryExecuteSql("PRAGMA temp_store = MEMORY");

    auto cleanup = [&]() {
        m_dgndb.TryExecuteSql("PRAGMA temp_store = DEFAULT");
        m_dgndb.Elements().SetBulkOperation(false);
    };

    if (const auto stat = CreateTempTables(); stat != BE_SQLITE_OK)
        {
        cleanup();
        return { BulkDeleteElementsStatus::DeletionFailed, stat, m_originalElementIds };
        }

    if (const auto stat = ExpandElementIdList(); stat != BE_SQLITE_OK)
        {
        cleanup();
        return { BulkDeleteElementsStatus::DeletionFailed, stat, m_originalElementIds };
        }

    // Run ANALYZE after populating the temp table so that SQLite has up-to-date statistics
    // for the subsequent constraint-checking and deletion queries.
    m_dgndb.TryExecuteSql("ANALYZE " TEMP_TABLE(TEMP_ELEMENT_DELETION));
    m_dgndb.Elements().SetBulkOperation(true);

    if (!m_skipFKConstraintValidations)
        {
        if (const auto stat = FindAndPruneConstraintViolators(); stat != BE_SQLITE_OK)
            {
            cleanup();
            return { BulkDeleteElementsStatus::DeletionFailed, stat, m_originalElementIds };
            }

        // If definition elements exist in the delete set, check usage and prune any that are
        // still referenced by elements outside the delete set.
        if (const auto stat = FindAndPruneInUseDefinitionElements(); stat != BE_SQLITE_OK)
            {
            cleanup();
            return { BulkDeleteElementsStatus::DeletionFailed, stat, m_originalElementIds };
            }

        // Early exit if constraint pruning removed every element from the delete set.
        // Returning DeletionFailed here (with BE_SQLITE_OK) signals that nothing was deleted.
        if (GetTempTableRowCount() == 0)
            {
            cleanup();
            return { BulkDeleteElementsStatus::DeletionFailed, BE_SQLITE_OK, m_originalElementIds };
            }
        }

    // Fire the pre and post delete callbacks at once
    if (!FireAllCallbacks())
        {
        cleanup();
        return { BulkDeleteElementsStatus::DeletionFailed, BE_SQLITE_OK, m_originalElementIds };
        }

    // Execute the actual SQL DELETE.  m_failedToDelete may already be non-empty if some
    // elements were pruned by the constraint checks above; we still attempt the deletion for
    // the remaining valid elements.
    if (const auto stat = ExecuteDeletion(); stat != BE_SQLITE_OK && m_failedToDelete.empty())
        {
        cleanup();
        return { BulkDeleteElementsStatus::DeletionFailed, stat, m_originalElementIds };
        }

    // Clean up link-table relationships
    if (const auto stat = DeleteLinkTableRelationships(); stat != BE_SQLITE_OK)
        { 
        cleanup();
        return { BulkDeleteElementsStatus::DeletionFailed, stat, m_originalElementIds };
        }

    cleanup();

    // Report Success only when every originally requested element was deleted;
    // PartialSuccess when at least one was blocked by a constraint.
    return { m_failedToDelete.empty() ? BulkDeleteElementsStatus::Success : BulkDeleteElementsStatus::PartialSuccess, BE_SQLITE_OK, std::move(m_failedToDelete) };
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BulkDeleteElementsResult DgnElements::DeleteElements(const DgnElementIdSet& elementIds, bool skipFKConstraintValidations)
    {
    DgnDb::VerifyClientThread();
    if (elementIds.empty())
        return { BulkDeleteElementsStatus::Success, BE_SQLITE_OK, {} };

    return BulkElementDeletion(m_dgndb, elementIds, skipFKConstraintValidations).Execute();
    }

END_BENTLEY_DGN_NAMESPACE