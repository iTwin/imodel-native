/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ChangesetSchema.h"
#include "SqlNames.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String TableTypeToString(int type)
    {
    switch (type)
        {
        case 0: return "Primary";
        case 1: return "Joined";
        case 3: return "Overflow";
        default: return "Unknown";
        }
    }

/*---------------------------------------------------------------------------------**//**
* Capture the schema mapping state for all mapped classes in the ECDb.
* Queries ec_Class, ec_Schema, ec_ClassMap, ec_ClassHasBaseClasses, ec_PropertyMap,
* ec_PropertyPath, ec_Column, ec_Table to build the snapshot.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ChangesetSchema::CaptureFromDb(ChangesetSchema& out, ECDbCR ecdb)
    {
    out.m_classes.clear();

    // Step 1: Build class entries with classId, schemaName:className, and baseClass.
    // Query all mapped classes (those with an entry in ec_ClassMap that have a non-zero MapStrategy).
    Statement classStmt;
    if (BE_SQLITE_OK != classStmt.Prepare(ecdb,
        "SELECT c.Id, s.Name, c.Name, cm.MapStrategy "
        "FROM " TABLE_Class " c "
        "JOIN " TABLE_Schema " s ON c.SchemaId = s.Id "
        "JOIN " TABLE_ClassMap " cm ON cm.ClassId = c.Id "
        "WHERE cm.MapStrategy <> " SQLVAL_MapStrategy_NotMapped))
        return ERROR;

    while (BE_SQLITE_ROW == classStmt.Step())
        {
        ECClassId classId = classStmt.GetValueId<ECClassId>(0);
        Utf8String schemaName = classStmt.GetValueText(1);
        Utf8String className = classStmt.GetValueText(2);
        Utf8String classKey = schemaName + ":" + className;

        ChangesetSchemaClassEntry entry;
        entry.m_classKey = classKey;
        entry.m_classId = classId;
        out.m_classes[classKey] = std::move(entry);
        }

    // Step 2: Resolve base classes. We use the first base class from ec_ClassHasBaseClasses
    // that is also in our mapped set.
    Statement baseStmt;
    if (BE_SQLITE_OK != baseStmt.Prepare(ecdb,
        "SELECT chbc.ClassId, s.Name, bc.Name "
        "FROM " TABLE_ClassHasBaseClasses " chbc "
        "JOIN " TABLE_Class " bc ON bc.Id = chbc.BaseClassId "
        "JOIN " TABLE_Schema " s ON bc.SchemaId = s.Id "
        "ORDER BY chbc.ClassId, chbc.Ordinal"))
        return ERROR;

    while (BE_SQLITE_ROW == baseStmt.Step())
        {
        ECClassId derivedId = baseStmt.GetValueId<ECClassId>(0);
        Utf8String baseSchemaName = baseStmt.GetValueText(1);
        Utf8String baseClassName = baseStmt.GetValueText(2);
        Utf8String baseKey = baseSchemaName + ":" + baseClassName;

        // Find the derived class entry
        for (auto& [key, entry] : out.m_classes)
            {
            if (entry.m_classId == derivedId && entry.m_baseClass.empty())
                {
                // Only set baseClass if the base is also in our mapped set
                if (out.m_classes.find(baseKey) != out.m_classes.end())
                    entry.m_baseClass = baseKey;
                break;
                }
            }
        }

    // Step 3: Get tables for each class.
    // A class maps to tables via its property maps' columns.
    Statement tableStmt;
    if (BE_SQLITE_OK != tableStmt.Prepare(ecdb,
        "SELECT DISTINCT pm.ClassId, t.Name, t.Type "
        "FROM " TABLE_PropertyMap " pm "
        "JOIN " TABLE_Column " c ON c.Id = pm.ColumnId "
        "JOIN " TABLE_Table " t ON t.Id = c.TableId "
        "WHERE c.IsVirtual = 0"))
        return ERROR;

    while (BE_SQLITE_ROW == tableStmt.Step())
        {
        ECClassId classId = tableStmt.GetValueId<ECClassId>(0);
        Utf8String tableName = tableStmt.GetValueText(1);
        int tableType = tableStmt.GetValueInt(2);

        for (auto& [key, entry] : out.m_classes)
            {
            if (entry.m_classId == classId)
                {
                entry.m_tables[tableName] = ChangesetSchemaTableInfo(TableTypeToString(tableType));
                break;
                }
            }
        }

    // Step 4: Get property maps for each class.
    // We query ec_PropertyMap joined with ec_PropertyPath and ec_Column/ec_Table
    // Exclude system columns: ECInstanceId (Kind=1) and ECClassId (Kind=2)
    // Also exclude SourceECClassId, TargetECClassId, RelECClassId by column name
    Statement propStmt;
    if (BE_SQLITE_OK != propStmt.Prepare(ecdb,
        "SELECT pm.ClassId, pp.AccessString, t.Name AS TableName, col.Name AS ColumnName "
        "FROM " TABLE_PropertyMap " pm "
        "JOIN " TABLE_PropertyPath " pp ON pp.Id = pm.PropertyPathId "
        "JOIN " TABLE_Column " col ON col.Id = pm.ColumnId "
        "JOIN " TABLE_Table " t ON t.Id = col.TableId "
        "WHERE col.IsVirtual = 0 "
        "AND col.ColumnKind NOT IN (" SQLVAL_DbColumn_Kind_ECInstanceId ", " SQLVAL_DbColumn_Kind_ECClassId ") "
        "AND col.Name NOT IN ('SourceECClassId', 'TargetECClassId', 'RelECClassId')"))
        return ERROR;

    while (BE_SQLITE_ROW == propStmt.Step())
        {
        ECClassId classId = propStmt.GetValueId<ECClassId>(0);
        Utf8String accessString = propStmt.GetValueText(1);
        Utf8String tableName = propStmt.GetValueText(2);
        Utf8String columnName = propStmt.GetValueText(3);

        for (auto& [key, entry] : out.m_classes)
            {
            if (entry.m_classId == classId)
                {
                entry.m_propertyMaps[accessString] = ChangesetSchemaPropertyMap(tableName, columnName);
                break;
                }
            }
        }

    // Step 5: Optimize — remove inherited property maps.
    // For each class that has a baseClass, remove property maps that are identical to the base.
    for (auto& [key, entry] : out.m_classes)
        {
        if (entry.m_baseClass.empty())
            continue;

        auto baseIt = out.m_classes.find(entry.m_baseClass);
        if (baseIt == out.m_classes.end())
            continue;

        // Get full property map of the base (recursively)
        auto baseFullMap = out.GetFullPropertyMap(entry.m_baseClass);

        // Remove from this class any property map that matches the base
        auto it = entry.m_propertyMaps.begin();
        while (it != entry.m_propertyMaps.end())
            {
            auto baseMapIt = baseFullMap.find(it->first);
            if (baseMapIt != baseFullMap.end() && baseMapIt->second == it->second)
                it = entry.m_propertyMaps.erase(it);
            else
                ++it;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Capture the schema mapping state for only classes referenced by the given changeset.
* Iterates the changeset to discover referenced tables and ECClassId values, then
* captures only those classes and their inheritance chain.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ChangesetSchema::Capture(ChangesetSchema& out, ECDbCR ecdb, ChangeStream const& changeset)
    {
    out.m_classes.clear();

    // Step 1: Iterate the changeset to collect referenced table names and ECClassId values.
    // For each table, determine which classes are referenced.
    bset<Utf8String> referencedTableNames;
    bset<ECClassId> referencedClassIds;

    Changes changes = const_cast<ChangeStream&>(changeset).GetChanges();
    for (auto const& change : changes)
        {
        Utf8String tableName = change.GetTableName();
        referencedTableNames.insert(tableName);

        // Try to read ECClassId from old or new values.
        // We need the ECClassId column index for this table — we'll query it below.
        }

    if (referencedTableNames.empty())
        return SUCCESS;

    // Step 2: For each referenced table, look up ExclusiveRootClassId or find ECClassId column.
    // Build a map: tableName -> { exclusiveRootClassId (or invalid), ecClassIdColumnOrdinal }
    struct TableInfo
        {
        ECClassId m_exclusiveRootClassId;
        int m_ecClassIdColumnOrdinal = -1; // column ordinal in SQLite table (0-based)
        };
    std::map<Utf8String, TableInfo> tableInfoMap;

    // Query ExclusiveRootClassId from ec_Table.
    Statement exclStmt;
    if (BE_SQLITE_OK != exclStmt.Prepare(ecdb,
        "SELECT ExclusiveRootClassId FROM " TABLE_Table " WHERE Name = ?"))
        return ERROR;

    for (auto const& tableName : referencedTableNames)
        {
        TableInfo info;

        // Look up ExclusiveRootClassId.
        exclStmt.Reset();
        exclStmt.BindText(1, tableName, Statement::MakeCopy::No);
        if (BE_SQLITE_ROW == exclStmt.Step() && !exclStmt.IsColumnNull(0))
            info.m_exclusiveRootClassId = exclStmt.GetValueId<ECClassId>(0);

        // Use PRAGMA table_info to find ECClassId column by name.
        // This gives the actual SQLite column position (cid), which matches
        // changeset column indices. Do NOT use ec_Column.Ordinal — it can
        // have gaps when virtual columns are present.
        Utf8String pragmaSql;
        pragmaSql.Sprintf("PRAGMA table_info([%s])", tableName.c_str());
        Statement pragmaStmt;
        if (BE_SQLITE_OK == pragmaStmt.Prepare(ecdb, pragmaSql.c_str()))
            {
            while (BE_SQLITE_ROW == pragmaStmt.Step())
                {
                Utf8CP colName = pragmaStmt.GetValueText(1);
                if (0 == strcmp(colName, "ECClassId"))
                    {
                    info.m_ecClassIdColumnOrdinal = pragmaStmt.GetValueInt(0); // cid
                    break;
                    }
                }
            }

        tableInfoMap[tableName] = info;
        }

    // Step 3: Re-iterate changeset to collect class IDs.
    // For tables with ExclusiveRootClassId, add that class directly.
    // For tables with ECClassId column, read the value from changeset rows.
    for (auto const& [tableName, info] : tableInfoMap)
        {
        if (info.m_exclusiveRootClassId.IsValid())
            referencedClassIds.insert(info.m_exclusiveRootClassId);
        }

    // Now re-iterate to read ECClassId values from changeset rows.
    Changes changes2 = const_cast<ChangeStream&>(changeset).GetChanges();
    for (auto const& change : changes2)
        {
        Utf8String tableName = change.GetTableName();
        auto infoIt = tableInfoMap.find(tableName);
        if (infoIt == tableInfoMap.end())
            continue;

        auto const& info = infoIt->second;
        if (info.m_ecClassIdColumnOrdinal < 0)
            continue; // no ECClassId column in this table

        int colIdx = info.m_ecClassIdColumnOrdinal;
        DbOpcode opcode = change.GetOpcode();

        // Read ECClassId from old or new value depending on operation.
        if (opcode == DbOpcode::Insert || opcode == DbOpcode::Update)
            {
            DbValue newVal = change.GetNewValue(colIdx);
            if (!newVal.IsNull())
                referencedClassIds.insert(ECClassId((uint64_t)newVal.GetValueInt64()));
            }
        if (opcode == DbOpcode::Delete || opcode == DbOpcode::Update)
            {
            DbValue oldVal = change.GetOldValue(colIdx);
            if (!oldVal.IsNull())
                referencedClassIds.insert(ECClassId((uint64_t)oldVal.GetValueInt64()));
            }
        }

    if (referencedClassIds.empty())
        return SUCCESS;

    // Step 4: Walk up the class hierarchy for each referenced class to find all ancestors.
    // Use a queue-based approach to discover the full chain.
    bset<ECClassId> allClassIds = referencedClassIds;
    bvector<ECClassId> queue(referencedClassIds.begin(), referencedClassIds.end());

    Statement baseStmt;
    if (BE_SQLITE_OK != baseStmt.Prepare(ecdb,
        "SELECT chbc.BaseClassId "
        "FROM " TABLE_ClassHasBaseClasses " chbc "
        "JOIN " TABLE_ClassMap " cm ON cm.ClassId = chbc.BaseClassId "
        "WHERE chbc.ClassId = ? AND cm.MapStrategy <> " SQLVAL_MapStrategy_NotMapped " "
        "ORDER BY chbc.Ordinal LIMIT 1"))
        return ERROR;

    while (!queue.empty())
        {
        ECClassId classId = queue.back();
        queue.pop_back();

        baseStmt.Reset();
        baseStmt.BindId(1, classId);
        if (BE_SQLITE_ROW == baseStmt.Step())
            {
            ECClassId baseClassId = baseStmt.GetValueId<ECClassId>(0);
            if (allClassIds.find(baseClassId) == allClassIds.end())
                {
                allClassIds.insert(baseClassId);
                queue.push_back(baseClassId);
                }
            }
        }

    // Step 5: Build class entries for the collected set (same logic as CaptureFromDb but filtered).
    Statement classStmt;
    if (BE_SQLITE_OK != classStmt.Prepare(ecdb,
        "SELECT c.Id, s.Name, c.Name "
        "FROM " TABLE_Class " c "
        "JOIN " TABLE_Schema " s ON c.SchemaId = s.Id "
        "WHERE c.Id = ?"))
        return ERROR;

    for (auto const& classId : allClassIds)
        {
        classStmt.Reset();
        classStmt.BindId(1, classId);
        if (BE_SQLITE_ROW == classStmt.Step())
            {
            Utf8String schemaName = classStmt.GetValueText(1);
            Utf8String className = classStmt.GetValueText(2);
            Utf8String classKey = schemaName + ":" + className;

            ChangesetSchemaClassEntry entry;
            entry.m_classKey = classKey;
            entry.m_classId = classId;
            out.m_classes[classKey] = std::move(entry);
            }
        }

    // Step 6: Resolve base classes.
    Statement baseResolveStmt;
    if (BE_SQLITE_OK != baseResolveStmt.Prepare(ecdb,
        "SELECT chbc.ClassId, s.Name, bc.Name "
        "FROM " TABLE_ClassHasBaseClasses " chbc "
        "JOIN " TABLE_Class " bc ON bc.Id = chbc.BaseClassId "
        "JOIN " TABLE_Schema " s ON bc.SchemaId = s.Id "
        "ORDER BY chbc.ClassId, chbc.Ordinal"))
        return ERROR;

    while (BE_SQLITE_ROW == baseResolveStmt.Step())
        {
        ECClassId derivedId = baseResolveStmt.GetValueId<ECClassId>(0);
        Utf8String baseSchemaName = baseResolveStmt.GetValueText(1);
        Utf8String baseClassName = baseResolveStmt.GetValueText(2);
        Utf8String baseKey = baseSchemaName + ":" + baseClassName;

        for (auto& [key, entry] : out.m_classes)
            {
            if (entry.m_classId == derivedId && entry.m_baseClass.empty())
                {
                if (out.m_classes.find(baseKey) != out.m_classes.end())
                    entry.m_baseClass = baseKey;
                break;
                }
            }
        }

    // Step 7: Get tables for each class.
    Statement classTblStmt;
    if (BE_SQLITE_OK != classTblStmt.Prepare(ecdb,
        "SELECT DISTINCT pm.ClassId, t.Name, t.Type "
        "FROM " TABLE_PropertyMap " pm "
        "JOIN " TABLE_Column " c ON c.Id = pm.ColumnId "
        "JOIN " TABLE_Table " t ON t.Id = c.TableId "
        "WHERE c.IsVirtual = 0 AND pm.ClassId = ?"))
        return ERROR;

    for (auto& [key, entry] : out.m_classes)
        {
        classTblStmt.Reset();
        classTblStmt.BindId(1, entry.m_classId);
        while (BE_SQLITE_ROW == classTblStmt.Step())
            {
            Utf8String tblName = classTblStmt.GetValueText(1);
            int tableType = classTblStmt.GetValueInt(2);
            entry.m_tables[tblName] = ChangesetSchemaTableInfo(TableTypeToString(tableType));
            }
        }

    // Step 8: Get property maps for each class.
    Statement propStmt;
    if (BE_SQLITE_OK != propStmt.Prepare(ecdb,
        "SELECT pp.AccessString, t.Name AS TableName, col.Name AS ColumnName "
        "FROM " TABLE_PropertyMap " pm "
        "JOIN " TABLE_PropertyPath " pp ON pp.Id = pm.PropertyPathId "
        "JOIN " TABLE_Column " col ON col.Id = pm.ColumnId "
        "JOIN " TABLE_Table " t ON t.Id = col.TableId "
        "WHERE col.IsVirtual = 0 "
        "AND col.ColumnKind NOT IN (" SQLVAL_DbColumn_Kind_ECInstanceId ", " SQLVAL_DbColumn_Kind_ECClassId ") "
        "AND col.Name NOT IN ('SourceECClassId', 'TargetECClassId', 'RelECClassId') "
        "AND pm.ClassId = ?"))
        return ERROR;

    for (auto& [key, entry] : out.m_classes)
        {
        propStmt.Reset();
        propStmt.BindId(1, entry.m_classId);
        while (BE_SQLITE_ROW == propStmt.Step())
            {
            Utf8String accessString = propStmt.GetValueText(0);
            Utf8String tblName = propStmt.GetValueText(1);
            Utf8String columnName = propStmt.GetValueText(2);
            entry.m_propertyMaps[accessString] = ChangesetSchemaPropertyMap(tblName, columnName);
            }
        }

    // Step 9: Optimize — remove inherited property maps (same as CaptureFromDb).
    for (auto& [key, entry] : out.m_classes)
        {
        if (entry.m_baseClass.empty())
            continue;

        auto baseIt = out.m_classes.find(entry.m_baseClass);
        if (baseIt == out.m_classes.end())
            continue;

        auto baseFullMap = out.GetFullPropertyMap(entry.m_baseClass);

        auto it = entry.m_propertyMaps.begin();
        while (it != entry.m_propertyMaps.end())
            {
            auto baseMapIt = baseFullMap.find(it->first);
            if (baseMapIt != baseFullMap.end() && baseMapIt->second == it->second)
                it = entry.m_propertyMaps.erase(it);
            else
                ++it;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::map<Utf8String, ChangesetSchemaPropertyMap> ChangesetSchema::GetFullPropertyMap(Utf8StringCR classKey) const
    {
    std::map<Utf8String, ChangesetSchemaPropertyMap> result;

    // Walk the inheritance chain from root to leaf, accumulating property maps.
    // Build the chain first (leaf -> root), then apply root -> leaf.
    bvector<Utf8String> chain;
    Utf8String current = classKey;
    while (!current.empty())
        {
        chain.push_back(current);
        auto it = m_classes.find(current);
        if (it == m_classes.end())
            break;
        current = it->second.m_baseClass;
        }

    // Apply from root to leaf (reverse order)
    for (auto rit = chain.rbegin(); rit != chain.rend(); ++rit)
        {
        auto it = m_classes.find(*rit);
        if (it == m_classes.end())
            continue;
        for (auto const& [accessString, propMap] : it->second.m_propertyMaps)
            result[accessString] = propMap;
        }

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangesetSchema::ToJson(BeJsValue out) const
    {
    out.SetEmptyObject();
    out["version"] = "1.0";

    BeJsValue classesJson = out["classes"];
    classesJson.SetEmptyObject();

    for (auto const& [key, entry] : m_classes)
        {
        BeJsValue classJson = classesJson[key.c_str()];
        classJson.SetEmptyObject();
        classJson["classId"] = entry.m_classId.ToHexStr();

        if (!entry.m_baseClass.empty())
            classJson["baseClass"] = entry.m_baseClass.c_str();
        else
            classJson["baseClass"].SetNull();

        BeJsValue tablesJson = classJson["tables"];
        tablesJson.SetEmptyObject();
        for (auto const& [tableName, tableInfo] : entry.m_tables)
            tablesJson[tableName.c_str()]["type"] = tableInfo.m_type.c_str();

        BeJsValue propertyMapsJson = classJson["propertyMaps"];
        propertyMapsJson.SetEmptyObject();
        for (auto const& [accessString, propMap] : entry.m_propertyMaps)
            {
            BeJsValue propJson = propertyMapsJson[accessString.c_str()];
            propJson["table"] = propMap.m_table.c_str();
            propJson["column"] = propMap.m_column.c_str();
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ChangesetSchema::FromJson(ChangesetSchema& out, BeJsConst json)
    {
    out.m_classes.clear();

    if (!json.isObject() || !json.isMember("classes"))
        return ERROR;

    BeJsConst classesJson = json["classes"];
    if (!classesJson.isObject())
        return ERROR;

    bool failed = false;
    classesJson.ForEachProperty([&](Utf8CP classKey, BeJsConst classJson)
        {
        ChangesetSchemaClassEntry entry;
        entry.m_classKey = classKey;

        if (!classJson.isMember("classId") || !classJson["classId"].isString())
            { failed = true; return true; }
        entry.m_classId = ECClassId(BeStringUtilities::ParseUInt64(classJson["classId"].asCString(), nullptr));

        if (classJson.isMember("baseClass") && !classJson["baseClass"].isNull())
            entry.m_baseClass = classJson["baseClass"].asCString();

        if (classJson.isMember("tables") && classJson["tables"].isObject())
            {
            classJson["tables"].ForEachProperty([&](Utf8CP tableName, BeJsConst tableJson)
                {
                Utf8String tableType = tableJson.isMember("type") ? tableJson["type"].asCString() : "";
                entry.m_tables[tableName] = ChangesetSchemaTableInfo(tableType);
                return false;
                });
            }

        if (classJson.isMember("propertyMaps") && classJson["propertyMaps"].isObject())
            {
            classJson["propertyMaps"].ForEachProperty([&](Utf8CP accessString, BeJsConst propJson)
                {
                Utf8String table  = propJson.isMember("table")  ? propJson["table"].asCString()  : "";
                Utf8String column = propJson.isMember("column") ? propJson["column"].asCString() : "";
                entry.m_propertyMaps[accessString] = ChangesetSchemaPropertyMap(table, column);
                return false;
                });
            }

        out.m_classes[classKey] = std::move(entry);
        return false;
        });

    return failed ? ERROR : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Diff two ChangesetSchema snapshots to find class ID remaps, column swaps,
* and overflow tables added.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ChangesetSchemaDiff::Diff(ChangesetSchemaDiff& out,
                                        ChangesetSchema const& before,
                                        ChangesetSchema const& after)
    {
    out.m_classIdRemaps.clear();
    out.m_columnSwaps.clear();
    out.m_overflowTablesAdded.clear();
    out.m_errors.clear();

    for (auto const& [classKey, beforeEntry] : before.GetClasses())
        {
        // ERROR: Class not found in after
        auto afterIt = after.GetClasses().find(classKey);
        if (afterIt == after.GetClasses().end())
            {
            MissingMapping err;
            err.m_kind = MissingMapping::Kind::Class;
            err.m_classKey = classKey;
            err.m_message = "Class '" + classKey + "' present in changeset schema before pull-merge was not found after pull-merge. Schema changes are additive — this indicates a code error.";
            out.m_errors.push_back(std::move(err));
            continue;
            }

        auto const& afterEntry = afterIt->second;

        // 1. CLASS ID REMAP
        if (beforeEntry.m_classId != afterEntry.m_classId)
            {
            ClassIdRemap remap;
            remap.m_classKey = classKey;
            remap.m_oldClassId = beforeEntry.m_classId;
            remap.m_newClassId = afterEntry.m_classId;
            out.m_classIdRemaps.push_back(std::move(remap));
            }

        // 2. COLUMN SWAP — compare full property maps (with inheritance resolved)
        auto beforeFullMap = before.GetFullPropertyMap(classKey);
        auto afterFullMap = after.GetFullPropertyMap(classKey);

        for (auto const& [accessString, beforePropMap] : beforeFullMap)
            {
            auto afterPropIt = afterFullMap.find(accessString);
            if (afterPropIt == afterFullMap.end())
                {
                // ERROR: Property not found in after
                MissingMapping err;
                err.m_kind = MissingMapping::Kind::Property;
                err.m_classKey = classKey;
                err.m_accessString = accessString;
                err.m_message = "Property '" + accessString + "' of class '" + classKey + "' present in changeset schema before pull-merge has no mapping after pull-merge. Schema changes are additive — this indicates a code error.";
                out.m_errors.push_back(std::move(err));
                continue;
                }

            auto const& afterPropMap = afterPropIt->second;
            if (beforePropMap != afterPropMap)
                {
                ColumnSwap swap;
                swap.m_classKey = classKey;
                swap.m_accessString = accessString;
                swap.m_oldTable = beforePropMap.m_table;
                swap.m_oldColumn = beforePropMap.m_column;
                swap.m_newTable = afterPropMap.m_table;
                swap.m_newColumn = afterPropMap.m_column;
                out.m_columnSwaps.push_back(std::move(swap));
                }
            }

        // 3. OVERFLOW TABLE — check for tables that disappeared or were added
        for (auto const& [tableName, tableInfo] : beforeEntry.m_tables)
            {
            if (afterEntry.m_tables.find(tableName) == afterEntry.m_tables.end())
                {
                // ERROR: Table disappeared
                MissingMapping err;
                err.m_kind = MissingMapping::Kind::Table;
                err.m_classKey = classKey;
                err.m_tableName = tableName;
                err.m_message = "Table '" + tableName + "' for class '" + classKey + "' present in changeset schema before pull-merge was not found after pull-merge. Schema changes are additive — this indicates a code error.";
                out.m_errors.push_back(std::move(err));
                }
            }

        for (auto const& [tableName, tableInfo] : afterEntry.m_tables)
            {
            if (beforeEntry.m_tables.find(tableName) == beforeEntry.m_tables.end())
                {
                if (tableInfo.m_type == "Overflow")
                    {
                    OverflowTableAdded overflow;
                    overflow.m_classKey = classKey;
                    overflow.m_overflowTable = tableName;
                    // Find parent table (the primary table for this class)
                    for (auto const& [tName, tInfo] : afterEntry.m_tables)
                        {
                        if (tInfo.m_type == "Primary")
                            {
                            overflow.m_parentTable = tName;
                            break;
                            }
                        }
                    // Check if overflow table has an ECClassId column
                    // We infer this from whether any property in afterFullMap references this table
                    // For now, default to false — the overflow table having ECClassId is rare
                    overflow.m_hasECClassIdColumn = false;
                    out.m_overflowTablesAdded.push_back(std::move(overflow));
                    }
                }
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangesetSchemaDiff::ToJson(BeJsValue out) const
    {
    out.SetEmptyObject();

    // Errors
    if (!m_errors.empty())
        {
        BeJsValue errorsJson = out["errors"];
        errorsJson.SetEmptyArray();
        for (auto const& err : m_errors)
            {
            BeJsValue errJson = errorsJson.appendValue();
            errJson.SetEmptyObject();
            switch (err.m_kind)
                {
                case MissingMapping::Kind::Class:    errJson["kind"] = "Class";    break;
                case MissingMapping::Kind::Property: errJson["kind"] = "Property"; break;
                case MissingMapping::Kind::Table:    errJson["kind"] = "Table";    break;
                }
            errJson["classKey"] = err.m_classKey.c_str();
            if (!err.m_accessString.empty())
                errJson["accessString"] = err.m_accessString.c_str();
            if (!err.m_tableName.empty())
                errJson["tableName"] = err.m_tableName.c_str();
            errJson["message"] = err.m_message.c_str();
            }
        }

    // Class ID Remaps
    BeJsValue remapsJson = out["classIdRemaps"];
    remapsJson.SetEmptyArray();
    for (auto const& remap : m_classIdRemaps)
        {
        BeJsValue remapJson = remapsJson.appendValue();
        remapJson.SetEmptyObject();
        remapJson["class"]      = remap.m_classKey.c_str();
        remapJson["oldClassId"] = remap.m_oldClassId.ToHexStr();
        remapJson["newClassId"] = remap.m_newClassId.ToHexStr();
        }

    // Column Swaps
    BeJsValue swapsJson = out["columnSwaps"];
    swapsJson.SetEmptyArray();
    for (auto const& swap : m_columnSwaps)
        {
        BeJsValue swapJson = swapsJson.appendValue();
        swapJson.SetEmptyObject();
        swapJson["class"]        = swap.m_classKey.c_str();
        swapJson["accessString"] = swap.m_accessString.c_str();
        swapJson["old"]["table"]  = swap.m_oldTable.c_str();
        swapJson["old"]["column"] = swap.m_oldColumn.c_str();
        swapJson["new"]["table"]  = swap.m_newTable.c_str();
        swapJson["new"]["column"] = swap.m_newColumn.c_str();
        }

    // Overflow Tables Added
    BeJsValue overflowsJson = out["overflowTablesAdded"];
    overflowsJson.SetEmptyArray();
    for (auto const& overflow : m_overflowTablesAdded)
        {
        BeJsValue overflowJson = overflowsJson.appendValue();
        overflowJson.SetEmptyObject();
        overflowJson["class"]             = overflow.m_classKey.c_str();
        overflowJson["overflowTable"]     = overflow.m_overflowTable.c_str();
        overflowJson["parentTable"]       = overflow.m_parentTable.c_str();
        overflowJson["hasECClassIdColumn"] = overflow.m_hasECClassIdColumn;
        }
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
