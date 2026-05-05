/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "SqlNames.h"
#include <ECDb/ChangesetSchema.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

// ==================== Internal Helpers ====================

// Column info for a SQLite table (queried from the current target db schema).
struct TableColInfo
    {
    int m_totalCols = 0;                    // Total column count in current db
    std::map<Utf8String, int> m_nameToOrd;  // column name → 0-based ordinal
    std::set<int> m_classIdOrdinals;        // ordinals that store a class ID (kind=2 OR named SourceECClassId/TargetECClassId)
    int m_classIdKindOrd = -1;              // ordinal of the kind=2 ECClassId column (for per-row class lookup); -1 if none
    int64_t m_exclusiveRootClassId = 0;     // ExclusiveRootClassId of the table, 0 if multi-class
    };

/*---------------------------------------------------------------------------------**//**
* Query column metadata for a SQLite table from the target db.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static TableColInfo QueryTableColInfo(ECDbCR ecdb, Utf8StringCR tableName)
    {
    TableColInfo info;

    Statement stmt;
    if (BE_SQLITE_OK == stmt.Prepare(ecdb,
        "SELECT c.Name, c.Ordinal, c.ColumnKind "
        "FROM " TABLE_Column " c JOIN " TABLE_Table " t ON t.Id = c.TableId "
        "WHERE t.Name = ? AND c.IsVirtual = 0 ORDER BY c.Ordinal"))
        {
        stmt.BindText(1, tableName, Statement::MakeCopy::No);
        int maxOrd = -1;
        while (BE_SQLITE_ROW == stmt.Step())
            {
            Utf8String name = stmt.GetValueText(0);
            int ord = stmt.GetValueInt(1);
            int kind = stmt.GetValueInt(2);
            info.m_nameToOrd[name] = ord;
            if (ord > maxOrd)
                maxOrd = ord;
            if (kind == 2)
                {
                info.m_classIdOrdinals.insert(ord);
                info.m_classIdKindOrd = ord;
                }
            // SourceECClassId / TargetECClassId also store class IDs — remap them by name
            if (name == COL_SourceECClassId || name == COL_TargetECClassId)
                info.m_classIdOrdinals.insert(ord);
            }
        info.m_totalCols = maxOrd + 1;
        }

    Statement exclStmt;
    if (BE_SQLITE_OK == exclStmt.Prepare(ecdb,
        "SELECT ExclusiveRootClassId FROM " TABLE_Table " WHERE Name = ?"))
        {
        exclStmt.BindText(1, tableName, Statement::MakeCopy::No);
        if (BE_SQLITE_ROW == exclStmt.Step())
            info.m_exclusiveRootClassId = exclStmt.GetValueInt64(0);
        }

    return info;
    }

// Per-class per-table column remap: describes how source ordinals map to output ordinals/tables.
// Precomputed derived fields (dstToSrc, crossTableSrcOrds) avoid per-row recalculation.
struct TableRemap
    {
    std::map<int, int> m_sameTable;     // srcOrd → dstOrd (within the same table, ordinal changed)
    struct XMove { Utf8String m_dstTable; int m_dstOrd; };
    std::map<int, XMove> m_crossTable;  // srcOrd → (overflow table, dst ordinal)

    // Derived (call Finalize() after all entries are added):
    std::map<int, int> m_dstToSrc;          // reverse: dstOrd → srcOrd (for same-table swaps)
    std::set<int> m_crossTableSrcOrds;      // source ordinals that move cross-table (skip in primary row)

    void Finalize()
        {
        m_dstToSrc.clear();
        m_crossTableSrcOrds.clear();
        for (auto const& [src, dst] : m_sameTable)
            m_dstToSrc[dst] = src;
        for (auto const& [src, xm] : m_crossTable)
            m_crossTableSrcOrds.insert(src);
        }
    };

/*---------------------------------------------------------------------------------**//**
* Write a DbValue into a Row at the given destination ordinal.
* Optionally remaps class ID values via classIdRemapMap.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static DbResult WriteRemappedValue(ChangeBuilder::Row& row, bool bNew, int dstOrd, DbValue const& val,
                                   std::map<uint64_t, uint64_t> const& classIdRemapMap,
                                   std::set<int> const& classIdOrdinals)
    {
    if (classIdOrdinals.count(dstOrd) && val.GetValueType() == DbValueType::IntegerVal)
        {
        uint64_t srcId = (uint64_t) val.GetValueInt64();
        auto it = classIdRemapMap.find(srcId);
        if (it != classIdRemapMap.end())
            return row.BindInt64(bNew, dstOrd, (int64_t) it->second);
        }

    switch (val.GetValueType())
        {
        case DbValueType::IntegerVal: return row.BindInt64(bNew, dstOrd, val.GetValueInt64());
        case DbValueType::FloatVal:   return row.BindDouble(bNew, dstOrd, val.GetValueDouble());
        case DbValueType::TextVal:    return row.BindText(bNew, dstOrd, val.GetValueText(), val.GetValueBytes());
        case DbValueType::BlobVal:    return row.BindBlob(bNew, dstOrd, val.GetValueBlob(), val.GetValueBytes());
        default:                      return row.BindNull(bNew, dstOrd);
        }
    }

// ==================== ChangesetTransformer::Transform ====================

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ChangesetTransformer::Transform(ChangeSet& output,
                                              ChangeStream const& source,
                                              ChangesetSchemaDiff const& diff,
                                              ECDbCR ecdb)
    {
    if (diff.HasErrors())
        return ERROR;

    if (!diff.NeedsTransform())
        return (BE_SQLITE_OK == output.ReadFrom(*source._GetReader())) ? SUCCESS : ERROR;

    // --- Build classId remap: oldClassId → newClassId ---
    std::map<uint64_t, uint64_t> classIdRemapMap;
    std::map<uint64_t, Utf8String> oldClassIdToKey;
    for (auto const& r : diff.m_classIdRemaps)
        {
        classIdRemapMap[r.m_oldClassId.GetValue()] = r.m_newClassId.GetValue();
        oldClassIdToKey[r.m_oldClassId.GetValue()] = r.m_classKey;
        }

    // --- Collect all classKeys that have column swaps or overflow tables ---
    std::set<Utf8String> allSwapKeys;
    for (auto const& s : diff.m_columnSwaps)
        allSwapKeys.insert(s.m_classKey);
    for (auto const& o : diff.m_overflowTablesAdded)
        allSwapKeys.insert(o.m_classKey);

    // --- Map current classId → classKey from target db (for non-remapped classes) ---
    std::map<uint64_t, Utf8String> curClassIdToKey;
    if (!allSwapKeys.empty())
        {
        Statement q;
        if (BE_SQLITE_OK == q.Prepare(ecdb,
            "SELECT s.Name || ':' || c.Name, c.Id "
            "FROM " TABLE_Class " c JOIN " TABLE_Schema " s ON s.Id = c.SchemaId"))
            {
            while (BE_SQLITE_ROW == q.Step())
                {
                Utf8String key = q.GetValueText(0);
                uint64_t id = (uint64_t) q.GetValueInt64(1);
                if (allSwapKeys.count(key))
                    curClassIdToKey[id] = key;
                }
            }
        }

    // --- Lazy table info cache ---
    std::map<Utf8String, TableColInfo> tableInfoCache;
    auto getTableInfo = [&](Utf8StringCR name) -> TableColInfo const&
        {
        auto it = tableInfoCache.find(name);
        if (it == tableInfoCache.end())
            it = tableInfoCache.emplace(name, QueryTableColInfo(ecdb, name)).first;
        return it->second;
        };

    // --- Build classKey → (srcTable → TableRemap) for column swaps ---
    std::map<Utf8String, std::map<Utf8String, TableRemap>> classTableRemap;
    for (auto const& swap : diff.m_columnSwaps)
        {
        auto const& srcInfo = getTableInfo(swap.m_oldTable);
        auto const& dstInfo = getTableInfo(swap.m_newTable);
        auto srcIt = srcInfo.m_nameToOrd.find(swap.m_oldColumn);
        auto dstIt = dstInfo.m_nameToOrd.find(swap.m_newColumn);
        if (srcIt == srcInfo.m_nameToOrd.end() || dstIt == dstInfo.m_nameToOrd.end())
            continue;
        int srcOrd = srcIt->second;
        int dstOrd = dstIt->second;
        auto& remap = classTableRemap[swap.m_classKey][swap.m_oldTable];
        if (swap.m_oldTable == swap.m_newTable)
            remap.m_sameTable[srcOrd] = dstOrd;
        else
            remap.m_crossTable[srcOrd] = {swap.m_newTable, dstOrd};
        }

    // Finalize all TableRemap objects (precompute derived fields)
    for (auto& [classKey, tableRemaps] : classTableRemap)
        for (auto& [tableName, remap] : tableRemaps)
            remap.Finalize();

    // --- Build output changeset using ChangeBuilder ---
    ChangeBuilder builder(ecdb);
    // Empty containers for when remap is nullptr (avoid per-row allocation; static = init once)
    static const std::map<int, int> s_emptyDstToSrc;
    static const std::set<int> s_emptyCrossSrcOrds;
    Changes changes = const_cast<ChangeStream&>(source).GetChanges();

    for (auto const& change : changes)
        {
        // LIFETIME NOTE: DbValue wraps sqlite3_value* which is valid only while the
        // source iterator is positioned on this change (i.e., within this loop body).
        // All builder.Append() calls below execute their lambdas synchronously — the
        // iterator does NOT advance until the loop body exits, so every DbValue access
        // is safe.  Text/blob pointers are additionally safe because sqlite3changegroup
        // copies them via memcpy before returning from the bind call.
        Utf8StringCR tableName = change.GetTableName();
        DbOpcode opcode = change.GetOpcode();
        int nCols = change.GetColumnCount();
        auto const& tableInfo = getTableInfo(tableName);
        int totalCols = (tableInfo.m_totalCols > 0) ? tableInfo.m_totalCols : nCols;

        // --- Determine source classId for this row ---
        uint64_t srcClassId = 0;
        if (tableInfo.m_classIdKindOrd >= 0 && tableInfo.m_classIdKindOrd < nCols)
            {
            DbValue v = (opcode != DbOpcode::Delete)
                ? change.GetNewValue(tableInfo.m_classIdKindOrd)
                : change.GetOldValue(tableInfo.m_classIdKindOrd);
            if (v.GetValueType() == DbValueType::IntegerVal)
                srcClassId = (uint64_t) v.GetValueInt64();
            }
        if (srcClassId == 0)
            srcClassId = (uint64_t) tableInfo.m_exclusiveRootClassId;

        // --- Look up classKey via source classId ---
        Utf8String classKey;
        {
        auto it = oldClassIdToKey.find(srcClassId);
        if (it != oldClassIdToKey.end())
            classKey = it->second;
        else
            {
            auto it2 = curClassIdToKey.find(srcClassId);
            if (it2 != curClassIdToKey.end())
                classKey = it2->second;
            }
        }

        // --- Get per-table column remap for this class ---
        TableRemap const* remap = nullptr;
        if (!classKey.empty())
            {
            auto ckIt = classTableRemap.find(classKey);
            if (ckIt != classTableRemap.end())
                {
                auto tIt = ckIt->second.find(tableName);
                if (tIt != ckIt->second.end())
                    remap = &tIt->second;
                }
            }

        // --- Build overflow column map: overflowTable → { dstOrd → srcOrd } ---
        // We store source ordinals rather than DbValue (DbValue is a non-owning pointer
        // into the changeset iterator; it remains valid for the lifetime of this loop body).
        std::map<Utf8String, std::map<int, int>> overflowOrdMap; // dstTable → dstOrd → srcOrd
        if (remap != nullptr && !remap->m_crossTable.empty())
            {
            for (auto const& [srcOrd, xm] : remap->m_crossTable)
                {
                if (srcOrd < nCols)
                    overflowOrdMap[xm.m_dstTable][xm.m_dstOrd] = srcOrd;
                }
            }

        // Use precomputed derived fields from TableRemap (avoids per-row allocation)
        auto const& dstToSrc = remap ? remap->m_dstToSrc : s_emptyDstToSrc;
        auto const& crossSrcOrds = remap ? remap->m_crossTableSrcOrds : s_emptyCrossSrcOrds;

        // --- Write primary change row ---
        DbResult rc = builder.Append(tableName.c_str(), opcode,
            [&](ChangeBuilder::Row& row) -> DbResult
            {
            if (opcode == DbOpcode::Insert || opcode == DbOpcode::Delete)
                {
                bool bNew = (opcode == DbOpcode::Insert);
                for (int dstOrd = 0; dstOrd < totalCols; ++dstOrd)
                    {
                    // Determine source ordinal for this destination slot
                    int srcOrd = dstOrd;
                    auto revIt = dstToSrc.find(dstOrd);
                    if (revIt != dstToSrc.end())
                        srcOrd = revIt->second;
                    else if (remap && (remap->m_sameTable.count(dstOrd) || crossSrcOrds.count(dstOrd)))
                        {
                        // dstOrd is a same-table swap source (value moved to different dest)
                        // or a cross-table move source (value moved to overflow) → NULL here
                        row.BindNull(bNew, dstOrd);
                        continue;
                        }

                    if (srcOrd >= nCols)
                        {
                        // New column added by other user → NULL
                        row.BindNull(bNew, dstOrd);
                        continue;
                        }

                    DbValue v = bNew ? change.GetNewValue(srcOrd) : change.GetOldValue(srcOrd);
                    WriteRemappedValue(row, bNew, dstOrd, v, classIdRemapMap, tableInfo.m_classIdOrdinals);
                    }
                }
            else // UPDATE
                {
                for (int srcOrd = 0; srcOrd < nCols; ++srcOrd)
                    {
                    DbValue oldV = change.GetOldValue(srcOrd);
                    DbValue newV = change.GetNewValue(srcOrd);
                    // Both undefined → column was not part of this UPDATE (unchanged)
                    if (oldV.GetValueType() == DbValueType::NullVal &&
                        newV.GetValueType() == DbValueType::NullVal)
                        continue;

                    // Cross-table move → handled by overflow row below
                    if (crossSrcOrds.count(srcOrd))
                        continue;

                    // Determine destination ordinal (apply same-table swap if any)
                    int dstOrd = srcOrd;
                    if (remap)
                        {
                        auto swIt = remap->m_sameTable.find(srcOrd);
                        if (swIt != remap->m_sameTable.end())
                            dstOrd = swIt->second;
                        else if (dstToSrc.count(srcOrd))
                            {
                            // This ordinal is claimed as the destination of another swap.
                            // The property at this srcOrd was displaced but has no explicit
                            // remap entry (should not happen with correct diff) → skip.
                            continue;
                            }
                        }

                    if (oldV.GetValueType() != DbValueType::NullVal)
                        WriteRemappedValue(row, false, dstOrd, oldV, classIdRemapMap, tableInfo.m_classIdOrdinals);
                    if (newV.GetValueType() != DbValueType::NullVal)
                        WriteRemappedValue(row, true, dstOrd, newV, classIdRemapMap, tableInfo.m_classIdOrdinals);
                    }
                }
            return BE_SQLITE_OK;
            });

        if (rc != BE_SQLITE_OK)
            return ERROR;

        // --- Write overflow rows for cross-table column moves ---
        for (auto const& [ovfTable, ovfCols] : overflowOrdMap)
            {
            // For UPDATE: only write an overflow row if some cross-table column was actually modified
            if (opcode == DbOpcode::Update)
                {
                bool hasChanged = false;
                for (auto const& [dstOrd, srcOrd] : ovfCols)
                    {
                    DbValue oldV = change.GetOldValue(srcOrd);
                    DbValue newV = change.GetNewValue(srcOrd);
                    if (oldV.GetValueType() != DbValueType::NullVal ||
                        newV.GetValueType() != DbValueType::NullVal)
                        {
                        hasChanged = true;
                        break;
                        }
                    }
                if (!hasChanged)
                    continue;
                }

            auto const& ovfInfo = getTableInfo(ovfTable);
            int ovfTotal = (ovfInfo.m_totalCols > 0) ? ovfInfo.m_totalCols : 1;

            // ECInstanceId from primary row (always ordinal 0)
            int64_t ecInstanceId = 0;
            {
            DbValue v = (opcode != DbOpcode::Delete)
                ? change.GetNewValue(0) : change.GetOldValue(0);
            if (v.GetValueType() == DbValueType::IntegerVal)
                ecInstanceId = v.GetValueInt64();
            }

            // Remapped classId (for overflow ECClassId column if present)
            uint64_t ovfClassId = srcClassId;
            {
            auto it = classIdRemapMap.find(srcClassId);
            if (it != classIdRemapMap.end())
                ovfClassId = it->second;
            }

            rc = builder.Append(ovfTable.c_str(), opcode,
                [&](ChangeBuilder::Row& row) -> DbResult
                {
                for (int dstOrd = 0; dstOrd < ovfTotal; ++dstOrd)
                    {
                    if (dstOrd == 0)
                        {
                        // ECInstanceId — PK column
                        // INSERT: bind new only. DELETE: bind old only.
                        // UPDATE: bind old only (PK identifies row; PK doesn't change).
                        if (opcode == DbOpcode::Delete || opcode == DbOpcode::Update)
                            row.BindInt64(false, 0, ecInstanceId);
                        if (opcode == DbOpcode::Insert)
                            row.BindInt64(true, 0, ecInstanceId);
                        continue;
                        }

                    if (dstOrd == ovfInfo.m_classIdKindOrd && ovfClassId != 0)
                        {
                        // ECClassId column — never changes in UPDATE (leave unbound = undefined marker)
                        if (opcode == DbOpcode::Insert)
                            row.BindInt64(true, dstOrd, (int64_t) ovfClassId);
                        else if (opcode == DbOpcode::Delete)
                            row.BindInt64(false, dstOrd, (int64_t) ovfClassId);
                        continue;
                        }

                    auto colIt = ovfCols.find(dstOrd);
                    if (colIt != ovfCols.end())
                        {
                        // Moved column — read values directly from the source changeset iterator.
                        // IMPORTANT: sqlite3changeset_old() is invalid (MISUSE) for INSERT rows,
                        // and sqlite3changeset_new() is invalid for DELETE rows.  Call only the
                        // accessor that is valid for the current opcode.
                        int srcOrd = colIt->second;
                        if (opcode == DbOpcode::Delete)
                            {
                            DbValue oldV = change.GetOldValue(srcOrd);
                            if (oldV.GetValueType() != DbValueType::NullVal)
                                WriteRemappedValue(row, false, dstOrd, oldV, classIdRemapMap, ovfInfo.m_classIdOrdinals);
                            }
                        else if (opcode == DbOpcode::Insert)
                            {
                            DbValue newV = change.GetNewValue(srcOrd);
                            if (newV.GetValueType() != DbValueType::NullVal)
                                WriteRemappedValue(row, true, dstOrd, newV, classIdRemapMap, ovfInfo.m_classIdOrdinals);
                            }
                        else // UPDATE — both old and new may be defined
                            {
                            DbValue oldV = change.GetOldValue(srcOrd);
                            DbValue newV = change.GetNewValue(srcOrd);
                            if (oldV.GetValueType() != DbValueType::NullVal)
                                WriteRemappedValue(row, false, dstOrd, oldV, classIdRemapMap, ovfInfo.m_classIdOrdinals);
                            if (newV.GetValueType() != DbValueType::NullVal)
                                WriteRemappedValue(row, true, dstOrd, newV, classIdRemapMap, ovfInfo.m_classIdOrdinals);
                            }
                        continue;
                        }

                    // Other overflow columns: not part of this change.
                    // INSERT/DELETE: bind NULL (all columns must be present).
                    // UPDATE: leave unbound → "unchanged" marker in changeset format.
                    if (opcode == DbOpcode::Insert)
                        row.BindNull(true, dstOrd);
                    else if (opcode == DbOpcode::Delete)
                        row.BindNull(false, dstOrd);
                    }
                return BE_SQLITE_OK;
                });

            if (rc != BE_SQLITE_OK)
                return ERROR;
            }
        }

    return (BE_SQLITE_OK == output.FromChangeBuilder(builder)) ? SUCCESS : ERROR;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
