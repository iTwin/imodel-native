/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <algorithm>
#include <set>
#include <unordered_set>

BEGIN_BENTLEY_DGN_NAMESPACE

#define TEMP_ELEMENT_DELETION "ElementsToDelete"

//=======================================================================================
//! ON DELETE action of a SQLite foreign key that references bis_Element.Id.
// @bsiclass
//=======================================================================================
enum class FkAction
    {
    Cascade,  //!< ON DELETE CASCADE: the referencing row is deleted along with the element.
    SetNull,  //!< ON DELETE SET NULL: the referencing column is nulled.
    Blocking, //!< ON DELETE NO ACTION / RESTRICT / SET DEFAULT: a surviving referencing row must block the delete.
    };

//=======================================================================================
//! A SQLite foreign key that references bis_Element.Id.
// @bsiclass
//=======================================================================================
struct FkConstraint
    {
    Utf8String m_tableName;        // referencing table
    Utf8String m_fromColumn;       // referencing column
    Utf8String m_elementIdColumn;  // column of the referencing table that identifies the element ("Id" for bis_Element, "ElementId" for joined-table-per-class tables); empty if the table is not an element table
    };

using FkConstraintMap = std::map<FkAction, std::vector<FkConstraint>>;

//=======================================================================================
// @bsiclass
//=======================================================================================
struct LogicalNavColumn
    {
    Utf8String m_idColumn;             // physical column holding the referenced ElementId
    Utf8String m_relClassIdColumn;     // companion RelECClassId column; empty if virtual
    std::set<uint64_t> m_classIds;     // ECClassIds whose rows store this nav property in these columns
    };

struct LogicalNavTable
    {
    Utf8String m_elementIdColumn;      // see FkConstraint::m_elementIdColumn
    std::vector<LogicalNavColumn> m_columns;
    };

using LogicalNavTableMap = std::map<Utf8String, LogicalNavTable>; // table name -> columns

//=======================================================================================
//! Restores ChangeTracker mode.
// @bsiclass
//=======================================================================================
struct ScopedTxnMode
    {
    TxnManagerR m_txns;
    ChangeTracker::Mode m_saved;
    ScopedTxnMode(TxnManagerR txns, ChangeTracker::Mode mode) : m_txns(txns), m_saved(txns.GetMode()) { m_txns.SetMode(mode); }
    ~ScopedTxnMode() { m_txns.SetMode(m_saved); }
    };

class BulkElementDeletion
    {
    DgnDbR m_dgndb;
    FkConstraintMap m_fkConstraints;
    LogicalNavTableMap m_logicalNavTables;
    DgnElementIdSet const& m_originalElementIds;
    std::vector<BulkDeleteFailure> m_failures;
    std::unordered_set<uint64_t> m_failureIds; // elements already present in m_failures

    // Records a blocking element once. An element referenced through several FKs (or both referenced and in use as a
    // definition) keeps the first reason found; FK checks run before the definition-usage check.
    void AddFailure(DgnElementId id, BulkDeleteFailureReason reason, Utf8String detail = "")
        {
        if (m_failureIds.insert(id.GetValue()).second)
            m_failures.push_back({ id, reason, std::move(detail) });
        }

    bool m_geometricElementsExist = false;
    bool m_subModelRootExists = false;

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

    // Invoke fn(fk) for every FK whose ON DELETE action blocks deletion, skipping the structurally handled ones.
    template <typename Fn> void ForEachBlockingFk(Fn&& fn) const
        {
        for (const auto& fk : GetFks(FkAction::Blocking))
            if (!IsStructurallyHandled(fk))
                fn(fk);
        }

    BeSQLite::DbResult LoadFkConstraints();
    BeSQLite::DbResult LoadLogicalNavProperties();
    bool HasPhysicalFk(Utf8StringCR tableName, Utf8StringCR column) const;

    // Create temporary tables for bulk deletion
    BeSQLite::DbResult CreateTempTables() const;
    BeSQLite::DbResult ExpandElementIdList();

    BeSQLite::DbResult FindProtectedElements();
    BeSQLite::DbResult FindReadOnlyDomainElements();
    BeSQLite::DbResult FindConstraintViolators();
    BeSQLite::DbResult FindInUseDefinitionElements();
    void CollectFkViolations();

    bool FireAllCallbacks();
    BeSQLite::DbResult NullifyLogicalNavProperties() const;
    BeSQLite::DbResult DeleteLinkTableRelationships() const;
    BeSQLite::DbResult ExecuteDeletion();

public:
    BulkElementDeletion(DgnDbR dgndb, const DgnElementIdSet& originalElementIds)
        :   m_dgndb(dgndb),
            m_originalElementIds(originalElementIds) {}

    BulkDeleteElementsResult Execute();
    };

/*---------------------------------------------------------------------------------**//**
* Reads every SQLite FK that targets bis_Element.Id.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BulkElementDeletion::LoadFkConstraints()
    {
    static const auto toAction = [](Utf8CP sql) -> FkAction
        {
        Utf8String s(sql);
        if (s.EqualsIAscii("CASCADE"))
            return FkAction::Cascade;
        if (s.EqualsIAscii("SET NULL"))
            return FkAction::SetNull;
        return FkAction::Blocking; // NO ACTION, RESTRICT, SET DEFAULT
        };

    Statement stmt;
    const auto rc = stmt.Prepare(m_dgndb,
        "SELECT m.name, f.\"from\", f.on_delete, "
            "CASE WHEN m.name = '" BIS_TABLE(BIS_CLASS_Element) "' THEN 'Id' "
                 "WHEN EXISTS (SELECT 1 FROM pragma_table_info(m.name) c WHERE c.name = 'ElementId' AND c.pk = 1) THEN 'ElementId' "
                 "ELSE NULL END "
        "FROM main.sqlite_master m, pragma_foreign_key_list(m.name) f "
        "WHERE m.type = 'table' AND f.\"table\" = '" BIS_TABLE(BIS_CLASS_Element) "' COLLATE NOCASE "
        "ORDER BY m.name, f.\"from\"");
    if (BE_SQLITE_OK != rc)
        {
        LOG.errorv("BulkElementDeletion: failed to prepare FK query: %s", BeSQLiteLib::GetLogError(rc).c_str());
        return rc;
        }

    FkConstraintMap constraints;
    DbResult stepRc;
    while (BE_SQLITE_ROW == (stepRc = stmt.Step()))
        {
        FkConstraint fk;
        fk.m_tableName  = stmt.GetValueText(0);
        fk.m_fromColumn = stmt.GetValueText(1);
        if (!stmt.IsColumnNull(3))
            fk.m_elementIdColumn = stmt.GetValueText(3);

        constraints[toAction(stmt.GetValueText(2))].push_back(std::move(fk));
        }

    if (BE_SQLITE_DONE != stepRc)
        {
        LOG.errorv("BulkElementDeletion: failed to read FK constraints: %s", BeSQLiteLib::GetLogError(stepRc).c_str());
        return stepRc;
        }

    // An empty result means the FK query itself is broken since we are looking at bis_Element.
    if (constraints.empty())
        {
        LOG.error("BulkElementDeletion: no foreign keys referencing bis_Element were found; refusing to delete");
        return BE_SQLITE_ERROR;
        }

    m_fkConstraints = std::move(constraints);
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BulkElementDeletion::HasPhysicalFk(Utf8StringCR tableName, Utf8StringCR column) const
    {
    for (const auto& [action, fks] : m_fkConstraints)
        for (const auto& fk : fks)
            if (fk.m_tableName.EqualsIAscii(tableName) && fk.m_fromColumn.EqualsIAscii(column))
                return true;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BulkElementDeletion::LoadLogicalNavProperties()
    {
    Statement stmt;
    const auto rc = stmt.Prepare(m_dgndb,
        "SELECT t.Name, cId.Name, CASE WHEN cRel.IsVirtual = 0 THEN cRel.Name END, pm.ClassId, "
            "CASE WHEN t.Name = '" BIS_TABLE(BIS_CLASS_Element) "' THEN 'Id' "
                 "WHEN EXISTS (SELECT 1 FROM pragma_table_info(t.Name) c WHERE c.name = 'ElementId' AND c.pk = 1) THEN 'ElementId' "
                 "ELSE NULL END "
        "FROM ec_Property p "
        "JOIN ec_RelationshipConstraint rc ON rc.RelationshipClassId = p.NavigationRelationshipClassId "
            "AND rc.RelationshipEnd = CASE p.NavigationDirection WHEN 1 THEN 1 ELSE 0 END "
        "JOIN ec_RelationshipConstraintClass rcc ON rcc.ConstraintId = rc.Id "
        "JOIN ec_cache_ClassHierarchy ch ON ch.ClassId = rcc.ClassId AND ch.BaseClassId = ? "
        "JOIN ec_PropertyPath ppId ON ppId.RootPropertyId = p.Id AND ppId.AccessString = p.Name || '.Id' "
        "JOIN ec_PropertyMap pm ON pm.PropertyPathId = ppId.Id "
        "JOIN ec_Column cId ON cId.Id = pm.ColumnId AND cId.IsVirtual = 0 "
        "JOIN ec_Table t ON t.Id = cId.TableId "
        "LEFT JOIN ec_PropertyPath ppRel ON ppRel.RootPropertyId = p.Id AND ppRel.AccessString = p.Name || '.RelECClassId' "
        "LEFT JOIN ec_PropertyMap pmRel ON pmRel.PropertyPathId = ppRel.Id AND pmRel.ClassId = pm.ClassId "
        "LEFT JOIN ec_Column cRel ON cRel.Id = pmRel.ColumnId "
        "WHERE p.NavigationRelationshipClassId IS NOT NULL "
        "ORDER BY t.Name, cId.Name");
    if (BE_SQLITE_OK != rc)
        {
        LOG.errorv("BulkElementDeletion: failed to prepare logical navigation property query: %s", BeSQLiteLib::GetLogError(rc).c_str());
        return rc;
        }

    const auto elementClassId = m_dgndb.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_Element);
    if (!elementClassId.IsValid())
        return BE_SQLITE_ERROR;
    stmt.BindId(1, elementClassId);

    LogicalNavTableMap tables;
    DbResult stepRc;
    while (BE_SQLITE_ROW == (stepRc = stmt.Step()))
        {
        const Utf8String tableName = stmt.GetValueText(0);
        const Utf8String idColumn  = stmt.GetValueText(1);
        if (HasPhysicalFk(tableName, idColumn))
            continue; // SQLite handles it

        auto& table = tables[tableName];
        if (!stmt.IsColumnNull(4))
            table.m_elementIdColumn = stmt.GetValueText(4);

        // One LogicalNavColumn per physical column.
        if (table.m_columns.empty() || !table.m_columns.back().m_idColumn.Equals(idColumn))
            {
            LogicalNavColumn col;
            col.m_idColumn = idColumn;
            if (!stmt.IsColumnNull(2))
                col.m_relClassIdColumn = stmt.GetValueText(2);
            table.m_columns.push_back(std::move(col));
            }
        table.m_columns.back().m_classIds.insert(stmt.GetValueUInt64(3));
        }

    if (BE_SQLITE_DONE != stepRc)
        {
        LOG.errorv("BulkElementDeletion: failed to read logical navigation properties: %s", BeSQLiteLib::GetLogError(stepRc).c_str());
        return stepRc;
        }

    m_logicalNavTables = std::move(tables);
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BulkElementDeletion::CreateTempTables() const
    {
    const auto tempTableCreationStatements = {
        // Create the temp table if needed; subsequent calls reuse it after clearing stale entries.
        "CREATE TABLE IF NOT EXISTS " TEMP_TABLE(TEMP_ELEMENT_DELETION) " ("
            "ElementId              INTEGER PRIMARY KEY, "
            "IsSubModelRoot         INTEGER NOT NULL DEFAULT 0, "
            "ECClassId              INTEGER)",

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
    // Closure of the requested ids over the two structural relationships: children (bis_Element.ParentId) and the contents of 
    // sub-models whose modeled element is in the set.
    constexpr auto expandSql = 
    "WITH RECURSIVE fullSet(id, classId) AS ("
        "SELECT e.Id, e.ECClassId "
        "FROM bis_Element e "
        "WHERE InVirtualSet(?, e.Id) "
        "UNION "
        // Children of elements in the set.
        "SELECT e.Id, e.ECClassId "
        "FROM bis_Element e "
        "INNER JOIN fullSet f ON e.ParentId = f.id "
        "UNION "
        // Elements contained in a sub-model whose root is in the set.
        "SELECT e.Id, e.ECClassId "
        "FROM bis_Element e "
        "INNER JOIN bis_Model cm ON cm.Id = e.ModelId "
        "INNER JOIN fullSet f ON cm.ModeledElementId = f.id "
    ") "
    "INSERT OR IGNORE INTO " TEMP_TABLE(TEMP_ELEMENT_DELETION) " (ElementId, ECClassId) "
    "SELECT id, classId "
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

    // Flag sub-model roots in the expanded set
    if (const auto stat = m_dgndb.TryExecuteSql(
            "UPDATE " TEMP_TABLE(TEMP_ELEMENT_DELETION) " SET IsSubModelRoot = 1 "
            "WHERE ElementId IN (SELECT ModeledElementId FROM " BIS_TABLE(BIS_CLASS_Model) " NOT INDEXED)"); stat != BE_SQLITE_OK)
        {
        LOG.errorv("BulkElementDeletion: Failed to flag sub-model roots: %s", BeSQLiteLib::GetLogError(stat).c_str());
        return stat;
        }
    m_subModelRootExists = m_dgndb.GetModifiedRowCount() > 0;

    // Check if any geometric elements are in the delete set to conditionally optimize the spatial index cleanup.
    const auto geomCheckStmt = m_dgndb.GetCachedStatement(
        "SELECT 1 FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t "
        "INNER JOIN " BIS_TABLE(BIS_CLASS_GeometricElement3d) " g ON g.ElementId = t.ElementId "
        "LIMIT 1");
    if (geomCheckStmt.IsValid() && geomCheckStmt->Step() == BE_SQLITE_ROW)
        m_geometricElementsExist = true;

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* The handful of elements every iModel must keep. Bulk deletion does not so the check is made here on the expanded set.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BulkElementDeletion::FindProtectedElements()
    {
    auto stmt = m_dgndb.GetCachedStatement("SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " WHERE ElementId IN (?, ?, ?)");
    if (stmt.IsNull())
        {
        LOG.error("BulkElementDeletion: Failed to prepare protected element check");
        return BE_SQLITE_ERROR;
        }

    stmt->BindId(1, DgnElements::GetRootSubjectId());
    stmt->BindId(2, m_dgndb.Elements().GetDictionaryPartitionId());
    stmt->BindId(3, m_dgndb.Elements().GetRealityDataSourcesPartitionId());

    while (BE_SQLITE_ROW == stmt->Step())
        {
        const auto id = stmt->GetValueId<DgnElementId>(0);
        AddFailure(id, BulkDeleteFailureReason::ProtectedElement);
        LOG.errorv("BulkElementDeletion: protected element %s is in the delete set", id.ToHexStr().c_str());
        }

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BulkElementDeletion::FindReadOnlyDomainElements()
    {
    const auto& domains = m_dgndb.Domains().GetDomains();
    if (std::none_of(domains.begin(), domains.end(), [](DgnDomainCP d) { return d->IsReadonly(); }))
        return BE_SQLITE_OK;

    auto classStmt = m_dgndb.GetCachedStatement("SELECT DISTINCT ECClassId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION));
    if (classStmt.IsNull())
        {
        LOG.error("BulkElementDeletion: Failed to prepare read-only domain check");
        return BE_SQLITE_ERROR;
        }

    std::vector<std::pair<DgnClassId, Utf8String>> readOnlyClasses;
    while (BE_SQLITE_ROW == classStmt->Step())
        {
        const auto classId = classStmt->GetValueId<DgnClassId>(0);
        const auto handler = dgn_ElementHandler::Element::FindHandler(m_dgndb, classId);
        if (nullptr != handler && handler->GetDomain().IsReadonly())
            readOnlyClasses.emplace_back(classId, handler->GetDomain().GetDomainName());
        }

    if (readOnlyClasses.empty())
        return BE_SQLITE_OK;

    auto elemStmt = m_dgndb.GetCachedStatement("SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " WHERE ECClassId = ?");
    if (elemStmt.IsNull())
        {
        LOG.error("BulkElementDeletion: Failed to prepare read-only domain element query");
        return BE_SQLITE_ERROR;
        }

    for (const auto& [classId, domainName] : readOnlyClasses)
        {
        elemStmt->Reset();
        elemStmt->ClearBindings();
        elemStmt->BindId(1, classId);
        while (BE_SQLITE_ROW == elemStmt->Step())
            AddFailure(elemStmt->GetValueId<DgnElementId>(0), BulkDeleteFailureReason::ReadOnlyDomain, domainName);
        }

    LOG.errorv("BulkElementDeletion: %zu ECClass(es) in the delete set belong to a read-only domain", readOnlyClasses.size());
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* Error path only: for every blocking FK, record each delete-set element that a surviving row still points at.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BulkElementDeletion::CollectFkViolations()
    {
    ForEachBlockingFk([&](FkConstraint const& fk)
        {
        const Utf8String sql = fk.m_elementIdColumn.empty()
            ? SqlPrintfString("SELECT DISTINCT t.ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t JOIN [%s] r ON r.[%s] = t.ElementId",
                                fk.m_tableName.c_str(), fk.m_fromColumn.c_str()).GetUtf8CP()
            : SqlPrintfString("SELECT DISTINCT t.ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t JOIN [%s] r ON r.[%s] = t.ElementId "
                                "LEFT JOIN " TEMP_TABLE(TEMP_ELEMENT_DELETION) " td ON td.ElementId = r.[%s] WHERE td.ElementId IS NULL",
                                fk.m_tableName.c_str(), fk.m_fromColumn.c_str(), fk.m_elementIdColumn.c_str()).GetUtf8CP();

        auto stmt = m_dgndb.GetCachedStatement(sql.c_str());
        if (stmt.IsNull())
            return;

        const Utf8String tableAndColumn = fk.m_tableName + "." + fk.m_fromColumn;
        while (BE_SQLITE_ROW == stmt->Step())
            AddFailure(stmt->GetValueId<DgnElementId>(0), BulkDeleteFailureReason::ReferencedByForeignKey, tableAndColumn);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BulkElementDeletion::FindConstraintViolators()
    {
    // A delete-set element is a direct violator if some row `r` of a referencing table points at it and `r` is not itself being deleted.
    // "r is being deleted" can only be established when the referencing row *is* an element (m_elementIdColumn set).
    constexpr int kSmallTableRows = 1000;
    std::map<Utf8String, int> rowsByTable; // bounded row count per referencing table (several FKs may share a table)

    auto boundedRowCount = [&](Utf8StringCR tableName) -> int
        {
        if (const auto it = rowsByTable.find(tableName); it != rowsByTable.end())
            return it->second;

        int rows = kSmallTableRows + 1; // fail safe: treat as large
        auto stmt = m_dgndb.GetCachedStatement(SqlPrintfString("SELECT COUNT(*) FROM (SELECT 1 FROM [%s] LIMIT %d)", tableName.c_str(), kSmallTableRows + 1));
        if (stmt.IsValid() && BE_SQLITE_ROW == stmt->Step())
            rows = stmt->GetValueInt(0);

        rowsByTable[tableName] = rows;
        return rows;
        };

    bool violatorsExist = false;
    DbResult stat = BE_SQLITE_OK;
    ForEachBlockingFk([&](FkConstraint const& fk)
        {
        if (violatorsExist || stat != BE_SQLITE_OK)
            return;

        const int rows = boundedRowCount(fk.m_tableName);
        if (rows == 0)
            return;

        Utf8CP indexHint = rows <= kSmallTableRows ? " NOT INDEXED" : "";
        const Utf8String sql = fk.m_elementIdColumn.empty()
            ? SqlPrintfString("SELECT 1 FROM [%s] r%s WHERE r.[%s] IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) ") LIMIT 1",
                                fk.m_tableName.c_str(), indexHint, fk.m_fromColumn.c_str()).GetUtf8CP()
            : SqlPrintfString("SELECT 1 FROM [%s] r%s WHERE r.[%s] IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) ") "
                                "AND r.[%s] NOT IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) ") LIMIT 1",
                                fk.m_tableName.c_str(), indexHint, fk.m_fromColumn.c_str(), fk.m_elementIdColumn.c_str()).GetUtf8CP();

        auto stmt = m_dgndb.GetCachedStatement(sql.c_str());
        if (stmt.IsNull())
            {
            LOG.errorv("BulkElementDeletion: Constraint violator guard check could not be prepared for %s.%s", fk.m_tableName.c_str(), fk.m_fromColumn.c_str());
            stat = BE_SQLITE_ERROR;
            return;
            }

        if (BE_SQLITE_ROW == stmt->Step())
            violatorsExist = true;
        });

    if (stat != BE_SQLITE_OK)
        return stat;

    if (!violatorsExist)
        return BE_SQLITE_OK;

    // Violators exist, so we are on the error path: gather the full report for the caller.
    CollectFkViolations();
    LOG.errorv("BulkElementDeletion: %zu element(s) in the delete set are still referenced via foreign keys from elements outside the set", m_failures.size());
    return BE_SQLITE_OK;
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

    // Exclude intra-set usages from the usage check: elements referencing each other inside the delete set are fine since all
    // will be deleted. The exclusion set has to be the *whole* expanded set (not just the definitions): a category used only by
    // elements that are themselves being deleted must not block the operation.
    auto allIdsStmt = m_dgndb.GetCachedStatement("SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " ORDER BY ElementId");
    if (!allIdsStmt.IsValid())
        {
        LOG.error("BulkElementDeletion: Failed to query delete set for definition usage check");
        return BE_SQLITE_ERROR;
        }

    BeIdSet allIds;
    while (BE_SQLITE_ROW == allIdsStmt->Step())
        allIds.insert(allIds.end(), allIdsStmt->GetValueId<BeInt64Id>(0));

    auto excludeIds = std::make_shared<BeSQLite::IdSet<BeInt64Id>>(std::move(allIds));

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

    LOG.errorv("BulkElementDeletion: %zu DefinitionElement(s) in the delete set are still in use by elements outside the set", inUse.size());
    return BE_SQLITE_OK;
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

    // A JS array that outlives the per-row HandleScope. Created lazily on first Push so empty batches cost nothing.
    struct JsArrayRef
        {
        Napi::Reference<Napi::Array> ref;
        uint32_t count = 0;

        bool IsEmpty() const { return ref.IsEmpty(); }
        Napi::Array Value() const { return ref.Value(); }
        void Push(Napi::Env env, Napi::Value value)
            {
            if (ref.IsEmpty())
                ref = Napi::Reference<Napi::Array>::New(Napi::Array::New(env), 1);
            ref.Value().Set(count++, value);
            }
        };

    struct NapiElementDeletionBatch
        {
        Napi::ObjectReference jsClass;
        JsArrayRef            arr;
        };
    struct NapiModelDeletionBatch
        {
        Napi::ObjectReference jsClass;
        JsArrayRef            deletedModelIds;
        std::unordered_map<uint64_t, JsArrayRef> elementsByModel; // modelId -> elementIds[]
        };

    // Resolve a numeric ECClassId to the corresponding JS class object.
    auto resolveJsClass = [&](DgnClassId classId) -> Napi::ObjectReference
        {
        auto ecClass = m_dgndb.Schemas().GetClass(classId);
        if (!ecClass)
            return Napi::ObjectReference();
        auto cv = getJsClassFn.Call(jsDgnDb, {m_dgndb.ToJsString(ecClass->GetFullName())});
        return cv.IsObject() ? Napi::Persistent(cv.As<Napi::Object>()) : Napi::ObjectReference();
        };

    // Per-element metadata is joined in here (read-only, after validation) rather than written into the temp table up front.
    // ParentId / ParentClassId are reported only when the parent is NOT itself in the delete set (tp.ElementId IS NULL).
    const auto stmt = m_dgndb.GetCachedStatement(
        "SELECT t.ElementId, t.ECClassId, e.ModelId, "
            "CASE WHEN tp.ElementId IS NULL THEN e.ParentId END, "
            "CASE WHEN tp.ElementId IS NULL THEN pe.ECClassId END, "
            "t.IsSubModelRoot, e.FederationGuid, subm.ECClassId, cm.ECClassId "
        "FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t "
        "JOIN " BIS_TABLE(BIS_CLASS_Element) " e ON e.Id = t.ElementId "
        "LEFT JOIN " BIS_TABLE(BIS_CLASS_Model) " cm ON cm.Id = e.ModelId "
        "LEFT JOIN " BIS_TABLE(BIS_CLASS_Model) " subm ON subm.ModeledElementId = e.Id "
        "LEFT JOIN " BIS_TABLE(BIS_CLASS_Element) " pe ON pe.Id = e.ParentId "
        "LEFT JOIN " TEMP_TABLE(TEMP_ELEMENT_DELETION) " tp ON tp.ElementId = e.ParentId");
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
            Napi::HandleScope rowScope(env);

            const Utf8String elementIdHex = elementId.ToHexStr();
            const Napi::String jsElementId = Napi::String::New(env, elementIdHex); // reused by every batch below

            auto& elemBatch = batchByClass[classId];
            if (elemBatch.jsClass.IsEmpty())
                elemBatch.jsClass = resolveJsClass(classId);
            if (!elemBatch.jsClass.IsEmpty())
                {
                BeJsNapiObject arg(env);
                static_cast<Napi::Object>(arg).Set("id", jsElementId);
                arg["model"] = modelId.ToHexStr();
                if (federationGuid.IsValid())
                    arg["federationGuid"] = federationGuid.ToString();
                if (isSubModelRoot)
                    static_cast<Napi::Object>(arg).Set("subModelId", jsElementId);
                elemBatch.arr.Push(env, static_cast<Napi::Object>(arg));
                }

            if (parentId.IsValid() && parentClassId.IsValid())
                {
                auto& childBatch = batchByParentClass[parentClassId];
                if (childBatch.jsClass.IsEmpty())
                    childBatch.jsClass = resolveJsClass(parentClassId);
                if (!childBatch.jsClass.IsEmpty())
                    {
                    BeJsNapiObject arg(env);
                    arg["parentId"] = parentId.ToHexStr();
                    static_cast<Napi::Object>(arg).Set("childId", jsElementId);
                    childBatch.arr.Push(env, static_cast<Napi::Object>(arg));
                    }
                }

            if (isSubModelRoot && subModelClassId.IsValid())
                {
                auto& modelBatch = modelClassBatches[subModelClassId];
                if (modelBatch.jsClass.IsEmpty())
                    modelBatch.jsClass = resolveJsClass(subModelClassId);
                if (!modelBatch.jsClass.IsEmpty())
                    modelBatch.deletedModelIds.Push(env, jsElementId);
                }

            if (containingModelClassId.IsValid())
                {
                auto& modelBatch = modelClassBatches[containingModelClassId];
                if (modelBatch.jsClass.IsEmpty())
                    modelBatch.jsClass = resolveJsClass(containingModelClassId);
                if (!modelBatch.jsClass.IsEmpty())
                    modelBatch.elementsByModel[modelId.GetValue()].Push(env, jsElementId);
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
            // The SELECT above already yields NULL for parentId when the parent is in the delete set; loadedParents just avoids repeated pool lookups.
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
        if (batch.jsClass.IsEmpty() || batch.arr.IsEmpty())
            continue;
        // Pass the element list to the JS class to fire higher-level change events for all deleted elements of this class at once.
        BeJsNapiObject batchArg(env);
        static_cast<Napi::Object>(batchArg).Set("iModel", jsDgnDb);
        static_cast<Napi::Object>(batchArg).Set("elements", batch.arr.Value());
        DgnDb::CallJsFunction(batch.jsClass.Value(), "onBulkDeleted", {static_cast<Napi::Object>(batchArg)});
        }

    for (auto& [parentClassId, batch] : batchByParentClass)
        {
        if (batch.jsClass.IsEmpty() || batch.arr.IsEmpty())
            continue;
        // Pass parent and child element lists so JS can update child-list caches on surviving parent elements.
        BeJsNapiObject batchArg(env);
        static_cast<Napi::Object>(batchArg).Set("iModel", jsDgnDb);
        static_cast<Napi::Object>(batchArg).Set("elements", batch.arr.Value());
        DgnDb::CallJsFunction(batch.jsClass.Value(), "onBulkChildDeleted", {static_cast<Napi::Object>(batchArg)});
        }

    for (auto& [classId, batch] : modelClassBatches)
        {
        if (batch.jsClass.IsEmpty())
            continue;

        BeJsNapiObject arg(env);

        if (!batch.deletedModelIds.IsEmpty())
            static_cast<Napi::Object>(arg).Set("deletedModelIds", batch.deletedModelIds.Value());

        if (!batch.elementsByModel.empty())
            {
            auto outerArr = Napi::Array::New(env, batch.elementsByModel.size());
            uint32_t i = 0;
            for (auto& [modelIdVal, elementIds] : batch.elementsByModel)
                {
                BeJsNapiObject entry(env);
                entry["id"] = DgnModelId(modelIdVal).ToHexStr();
                static_cast<Napi::Object>(entry).Set("elementIds", elementIds.Value());
                outerArr.Set(i++, static_cast<Napi::Object>(entry));
                }
            static_cast<Napi::Object>(arg).Set("deletedElementsByModel", outerArr);
            }

        static_cast<Napi::Object>(arg).Set("iModel", jsDgnDb);
        DgnDb::CallJsFunction(batch.jsClass.Value(), "onBulkModelEvents", {static_cast<Napi::Object>(arg)});
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Null out logical (non-FK) navigation properties that point into the delete set.
* One UPDATE per table, touching every affected column of that table in a single pass;
* rows that are themselves being deleted are left alone.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BulkElementDeletion::NullifyLogicalNavProperties() const
    {
    for (const auto& [tableName, table] : m_logicalNavTables)
        {
        if (auto probe = m_dgndb.GetCachedStatement(SqlPrintfString("SELECT 1 FROM [%s] LIMIT 1", tableName.c_str())); probe.IsValid() && BE_SQLITE_ROW != probe->Step())
            continue; // empty table

        std::vector<Utf8String> conditions;
        conditions.reserve(table.m_columns.size());
        for (const auto& col : table.m_columns)
            {
            Utf8String classList;
            for (const auto classId : col.m_classIds)
                classList.append(classList.empty() ? "" : ",").append(std::to_string(classId));

            conditions.push_back(SqlPrintfString("ECClassId IN (%s) AND [%s] IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) ")", classList.c_str(), col.m_idColumn.c_str()).GetUtf8CP());
            }

        Utf8String sql = SqlPrintfString("UPDATE [%s] SET ", tableName.c_str()).GetUtf8CP();
        Utf8String where;
        const bool single = table.m_columns.size() == 1;
        for (size_t i = 0; i < table.m_columns.size(); ++i)
            {
            const auto& col = table.m_columns[i];
            const auto& cond = conditions[i];
            if (i > 0)
                {
                sql.append(", ");
                where.append(" OR ");
                }

            if (single)
                {
                sql.append(SqlPrintfString("[%s] = NULL", col.m_idColumn.c_str()).GetUtf8CP());
                if (!col.m_relClassIdColumn.empty())
                    sql.append(SqlPrintfString(", [%s] = NULL", col.m_relClassIdColumn.c_str()).GetUtf8CP());
                }
            else
                {
                sql.append(SqlPrintfString("[%s] = CASE WHEN %s THEN NULL ELSE [%s] END", col.m_idColumn.c_str(), cond.c_str(), col.m_idColumn.c_str()).GetUtf8CP());
                if (!col.m_relClassIdColumn.empty())
                    sql.append(SqlPrintfString(", [%s] = CASE WHEN %s THEN NULL ELSE [%s] END", col.m_relClassIdColumn.c_str(), cond.c_str(), col.m_relClassIdColumn.c_str()).GetUtf8CP());
                }

            where.append("(").append(cond).append(")");
            }

        sql.append(" WHERE (").append(where).append(")");
        if (!table.m_elementIdColumn.empty())
            sql.append(SqlPrintfString(" AND [%s] NOT IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) ")", table.m_elementIdColumn.c_str()).GetUtf8CP());

        if (const auto stat = m_dgndb.TryExecuteSql(sql.c_str()); stat != BE_SQLITE_OK)
            {
            LOG.errorv("BulkElementDeletion: failed to null logical navigation properties in %s: %s", tableName.c_str(), BeSQLiteLib::GetLogError(stat).c_str());
            return stat;
            }

        if (const auto n = m_dgndb.GetModifiedRowCount(); n > 0)
            LOG.infov("BulkElementDeletion: nulled logical navigation properties in %d row(s) of %s", n, tableName.c_str());
        }

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult BulkElementDeletion::ExecuteDeletion()
    {
    auto isEmptyTable = [&](Utf8StringCR tableName) -> bool
        {
        auto stmt = m_dgndb.GetCachedStatement(SqlPrintfString("SELECT 1 FROM [%s] LIMIT 1", tableName.c_str()));
        return stmt.IsValid() && BE_SQLITE_ROW != stmt->Step();
        };

    // Explicitly execute ON DELETE CASCADE and ON DELETE SET NULL FK actions for all tables that reference bis_Element.
    {
    ScopedTxnMode indirect(m_dgndb.Txns(), ChangeTracker::Mode::Indirect); // cascade changes are tracked as indirect; restored on every exit path

    for (const auto& fk : GetFks(FkAction::Cascade))
        {
        if (IsStructurallyHandled(fk)) // bis_Element.ParentId: children are already in the delete set
            continue;
        if (isEmptyTable(fk.m_tableName))
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
        if (isEmptyTable(fk.m_tableName))
            continue;

        const auto stat = m_dgndb.TryExecuteSql(SqlPrintfString(
            "UPDATE [%s] SET [%s] = NULL WHERE [%s] IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) ")",
            fk.m_tableName.c_str(), fk.m_fromColumn.c_str(), fk.m_fromColumn.c_str()));
        if (stat != BE_SQLITE_OK)
            {
            LOG.errorv("BulkElementDeletion: ON DELETE SET NULL emulation failed for %s.%s: %s", fk.m_tableName.c_str(), fk.m_fromColumn.c_str(), BeSQLiteLib::GetLogError(stat).c_str());
            return stat;
            }
        }

    // Navigation properties without a SQLite FK would otherwise be left dangling.
    if (const auto stat = NullifyLogicalNavProperties(); stat != BE_SQLITE_OK)
        return stat;
    }

    auto restoreFkState = [&]() {
        if (m_subModelRootExists)
            m_dgndb.TryExecuteSql("PRAGMA defer_foreign_keys = false");
    };

    if (m_geometricElementsExist)
        m_dgndb.TryExecuteSql("DELETE FROM " DGN_VTABLE_SpatialIndex " WHERE ElementId IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) ")");

    // Only needed while FKs are enforced: bis_Model.ModeledElementId (NO ACTION) points at sub-model roots that leave bis_Element before bis_Model.
    if (m_subModelRootExists)
        m_dgndb.TryExecuteSql("PRAGMA defer_foreign_keys = true");

    if (const auto stat = m_dgndb.TryExecuteSql("DELETE FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE Id IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) ")"); stat != BE_SQLITE_OK)
        {
        LOG.errorv("BulkElementDeletion: Element deletion failed: %s", BeSQLiteLib::GetLogError(stat).c_str());
        restoreFkState();
        return stat;
        }

    if (m_subModelRootExists)
        {
        if (const auto stat = m_dgndb.TryExecuteSql("DELETE FROM " BIS_TABLE(BIS_CLASS_Model) " WHERE Id IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " WHERE IsSubModelRoot = 1)"); stat != BE_SQLITE_OK)
            {
            LOG.errorv("BulkElementDeletion: Sub-model root deletion failed: %s", BeSQLiteLib::GetLogError(stat).c_str());
            restoreFkState();
            return stat;
            }

        // Do a manual foreign key check for the models to maintain integrity
        struct DeferredFkCheck
            {
            Utf8CP sql;
            Utf8CP what;
            };
        static const DeferredFkCheck s_checks[] = {
            { "SELECT 1 FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE ModelId IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " WHERE IsSubModelRoot = 1) LIMIT 1",
              "surviving element(s) still belong to a deleted model" },
            { "SELECT 1 FROM " BIS_TABLE(BIS_CLASS_Model) " WHERE ParentModelId IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " WHERE IsSubModelRoot = 1) LIMIT 1",
              "surviving sub-model(s) still have a deleted parent model" },
            { "SELECT 1 FROM " BIS_TABLE(BIS_CLASS_Model) " m WHERE NOT EXISTS (SELECT 1 FROM " BIS_TABLE(BIS_CLASS_Element) " e WHERE e.Id = m.ModeledElementId) LIMIT 1",
              "surviving model(s) whose modeled element was deleted" },
        };
        for (const auto& check : s_checks)
            {
            auto stmt = m_dgndb.GetCachedStatement(check.sql);
            if (!stmt.IsValid() || BE_SQLITE_ROW == stmt->Step())
                {
                LOG.errorv("BulkElementDeletion: integrity check failed: %s", check.what);
                restoreFkState();
                return BE_SQLITE_CONSTRAINT_FOREIGNKEY;
                }
            }
        }

    restoreFkState();
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
    //  1. LoadFkConstraints     – read every FK that targets bis_Element.Id from the SQLite schema.
    //  2. LoadLogicalNavProperties – read every nav property targeting bis:Element that has NO SQLite FK (EC metadata).
    //  3. CreateTempTables      – set up temp.ElementsToDelete scratch space.
    //  4. ExpandElementIdList   – recursively pull in children and sub-model members.
    //  5. ANALYZE               – give SQLite fresh statistics for the temp table so the
    //                             query planner makes good choices in later steps.
    //  6. FindProtectedElements         – root Subject and the built-in partitions can never be deleted.
    //  7. FindReadOnlyDomainElements    – elements whose handler belongs to a read-only DgnDomain.
    //  8. FindConstraintViolators       – elements still referenced via NO ACTION / RESTRICT FKs to bis_Element
    //  9. FindInUseDefinitionElements   – DefinitionElements still used by elements outside the set. 
    //      - If 6-9 found anything, nothing has been modified yet: return ConstraintViolation with the report.
    // 10. FireAllCallbacks      – dispatch C++ and JS pre-/post-delete notifications in bulk (purge operation active).
    // 11. ExecuteDeletion       – emulate CASCADE / SET NULL FKs, null logical nav properties, then bulk DELETE from bis_Element and bis_Model for sub-model roots.
    // 12. DeleteLinkTableRelationships – clean up link-table rows for deleted endpoints.

    DgnDb::VerifyClientThread();

    auto queryPragma = [&](Utf8CP pragma, Utf8CP fallback) -> Utf8String
        {
        Statement stmt;
        if (BE_SQLITE_OK == stmt.Prepare(m_dgndb, SqlPrintfString("PRAGMA %s", pragma)) && BE_SQLITE_ROW == stmt.Step())
            return stmt.GetValueText(0);
        return fallback;
        };
    const Utf8String savedCacheSize   = queryPragma("cache_size", "-2000");
    const Utf8String savedSynchronous = queryPragma("synchronous", "1"); // 1 == NORMAL
    const Utf8String savedTempStore   = queryPragma("temp_store", "0");  // 0 == DEFAULT

    m_dgndb.TryExecuteSql("PRAGMA temp_store = MEMORY");
    m_dgndb.TryExecuteSql("PRAGMA synchronous = OFF");
    m_dgndb.TryExecuteSql("PRAGMA cache_size = -131072");

    auto cleanup = [&]() {
        m_dgndb.Elements().SetBulkOperation(false);
        m_dgndb.TryExecuteSql(SqlPrintfString("PRAGMA temp_store = %s", savedTempStore.c_str()));
        m_dgndb.TryExecuteSql(SqlPrintfString("PRAGMA synchronous = %s", savedSynchronous.c_str()));
        m_dgndb.TryExecuteSql(SqlPrintfString("PRAGMA cache_size = %s", savedCacheSize.c_str()));
    };

    // Internal failure
    auto fail = [&](DbResult stat) -> BulkDeleteElementsResult {
        cleanup();
        return { BulkDeleteElementsStatus::DeletionFailed, stat };
    };

    if (const auto stat = LoadFkConstraints(); stat != BE_SQLITE_OK)
        return fail(stat);

    if (const auto stat = LoadLogicalNavProperties(); stat != BE_SQLITE_OK)
        return fail(stat);

    if (const auto stat = CreateTempTables(); stat != BE_SQLITE_OK)
        return fail(stat);

    if (const auto stat = ExpandElementIdList(); stat != BE_SQLITE_OK)
        return fail(stat);

    // Run ANALYZE after populating the temp table so that SQLite has up-to-date statistics
    // for the subsequent constraint-checking and deletion queries.
    m_dgndb.TryExecuteSql("ANALYZE " TEMP_TABLE(TEMP_ELEMENT_DELETION));
    m_dgndb.Elements().SetBulkOperation(true);

    // Validate the whole delete set before touching anything.
    // The FK and definition-usage checks treat references from *inside* the set as fine, so only direct violators are
    // reported; the caller may discover further ones after removing these from the request.
    if (const auto stat = FindProtectedElements(); stat != BE_SQLITE_OK)
        return fail(stat);

    if (const auto stat = FindReadOnlyDomainElements(); stat != BE_SQLITE_OK)
        return fail(stat);

    if (const auto stat = FindConstraintViolators(); stat != BE_SQLITE_OK)
        return fail(stat);

    if (const auto stat = FindInUseDefinitionElements(); stat != BE_SQLITE_OK)
        return fail(stat);

    // Nothing has been modified yet, so bail out and report exactly what blocks the operation.
    if (!m_failures.empty())
        {
        cleanup();
        return { BulkDeleteElementsStatus::ConstraintViolation, BE_SQLITE_OK, std::move(m_failures) };
        }

    // From here on the purge operation is active.
    // Bulk deletion is a purge, and has already verified the definitions are unused.
    m_dgndb.BeginPurgeOperation();
    auto failPurging = [&](DbResult stat) -> BulkDeleteElementsResult {
        m_dgndb.EndPurgeOperation();
        return fail(stat);
    };

    // Fire the pre and post delete callbacks at once
    if (!FireAllCallbacks())
        return failPurging(BE_SQLITE_OK);

    // Execute the actual SQL DELETE. Any failure here is fatal for the whole operation: part of the delete set may already
    // have been removed, so the caller must abandon the enclosing txn.
    if (const auto stat = ExecuteDeletion(); stat != BE_SQLITE_OK)
        return failPurging(stat);

    // Clean up link-table relationships
    if (const auto stat = DeleteLinkTableRelationships(); stat != BE_SQLITE_OK)
        return failPurging(stat);

    m_dgndb.EndPurgeOperation();

    cleanup();
    return { BulkDeleteElementsStatus::Success, BE_SQLITE_OK };
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BulkDeleteElementsResult DgnElements::DeleteElements(const DgnElementIdSet& elementIds)
    {
    DgnDb::VerifyClientThread();
    if (elementIds.empty())
        return { BulkDeleteElementsStatus::Success, BE_SQLITE_OK };

    return BulkElementDeletion(m_dgndb, elementIds).Execute();
    }

END_BENTLEY_DGN_NAMESPACE