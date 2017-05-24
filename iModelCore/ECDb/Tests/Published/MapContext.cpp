/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/MapContext.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "MapContext.h"
BEGIN_ECDBUNITTESTS_NAMESPACE
/////////////////////////////////////////DataTable::Value////////////////////////////////
//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
void MapContext::LoadTables()
    {
    if (!m_tables.empty())
        return;

    Utf8CP sql = R"(
                SELECT [T].[Name], 
                       CASE [T].[Type] WHEN 0 THEN 'Primary' WHEN 1 THEN 'Joined' WHEN 2 THEN 'Existing' WHEN 3 THEN 'Overflow' ELSE '<err>' END [Type], 
                       CASE [T].[IsVirtual] WHEN 0 THEN 'False' WHEN 1 THEN 'True' ELSE '<err>' END [IsVirtual], 
                       [TP].[Name] [ParentTable], 
                       [S].[Name] [ExclusiveRootSchema], 
                       [C].[Name] [ExclusiveRootClass]
                FROM   [ec_Table] [T]
                       LEFT JOIN [ec_Table] [TP] ON [TP].[Id] = [T].[ParentTableId]
                       LEFT JOIN [ec_Class] [C] ON [C].[Id] = [T].[ExclusiveRootClassId]
                       LEFT JOIN [ec_Schema] [S] ON [S].[Id] = [C].[SchemaId]
                ORDER  BY [T].[Name];)";
    auto stmt = m_ecdb.GetCachedStatement(sql);
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        std::unique_ptr<Table> t = std::unique_ptr<Table>(new Table());
        if (!stmt->IsColumnNull(0)) t->m_name = stmt->GetValueText(0);
        if (!stmt->IsColumnNull(1)) t->m_type = stmt->GetValueText(1);
        if (!stmt->IsColumnNull(2)) t->m_isVirtual = stmt->GetValueText(2);
        if (!stmt->IsColumnNull(3)) t->m_parentTable = stmt->GetValueText(3);
        if (!stmt->IsColumnNull(4)) t->m_exclusiveRootSchema = stmt->GetValueText(4);
        if (!stmt->IsColumnNull(5)) t->m_exclusiveRootClass = stmt->GetValueText(5);
        LoadColumns(*t);
        m_tables[t->GetName()] = std::move(t);
        }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
void MapContext::LoadColumns(Table& table)
    {
    if (!table.GetColumns().empty())
        return;


    Utf8CP sql = R"(
                SELECT  [C].[Name] [Column], 
                        CASE [C].[Type] WHEN 0 THEN 'Any' WHEN 1 THEN 'Boolean' WHEN 2 THEN 'Blob' WHEN 3 THEN 'TimeStamp' WHEN 4 THEN 'Real' WHEN 5 THEN 'Integer' WHEN 6 THEN 'Text' ELSE '<err>' END [Type], 
                        CASE [C].[IsVirtual] WHEN 0 THEN 'False' WHEN 1 THEN 'True' ELSE '<err>' END [IsVirtual], 
                        CAST([Ordinal] AS TEXT), 
                        CASE [C].[NotNullConstraint] WHEN 1 THEN 'True' WHEN 0 THEN 'False' ELSE '<err>' END [NotNullConstraint], 
                        CASE [C].[UniqueConstraint] WHEN 1 THEN 'True' WHEN 0 THEN 'False' ELSE '<err>' END [UniqueConstraint], 
                        [CheckConstraint], 
                        [DefaultConstraint], 
                        CASE [CollationConstraint] WHEN 0 THEN 'Unset' WHEN 1 THEN 'Binary' WHEN 2 THEN 'NoCase' WHEN 3 THEN 'RTrim' ELSE '<err>' END CollationConstraint, 
                        CAST([OrdinalInPrimaryKey] AS TEXT), 
                        (SELECT GROUP_CONCAT ([Name], ' | ')
                            FROM   (SELECT [] [Value], 
                                            [:1] [Name]
                                    FROM   (VALUES (1, 'ECInstanceId')
                                            UNION
                                            VALUES (2, 'ECClassId')
                                            UNION
                                            VALUES (4, 'SourceECInstanceId')
                                            UNION
                                            VALUES (8, 'SourceECClassId')
                                            UNION
                                            VALUES (16, 'TargetECInstanceId')
                                            UNION
                                            VALUES (32, 'TargetECClassId')
                                            UNION
                                            VALUES (64, 'DataColumn')
                                            UNION
                                            VALUES (128, 'SharedDataColumn')
                                            UNION
                                            VALUES (256, 'RelECClassId')))
                                WHERE  [Value] | [ColumnKind] = [ColumnKind]) [ColumnKind]
                FROM   [ec_Column] [C]
                        INNER JOIN [ec_Table] [T] ON [T].[Id] = [C].[TableId] AND [T].[Name]=? 
                ORDER  BY [C].[TableId],
                            [C].[Id];)";

    auto stmt = m_ecdb.GetCachedStatement(sql);
    stmt->BindText(1, table.GetName(), Statement::MakeCopy::No);
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        std::unique_ptr<Column> t = std::unique_ptr<Column>(new Column(table));
        if (!stmt->IsColumnNull(0)) t->m_name = stmt->GetValueText(0);
        if (!stmt->IsColumnNull(1)) t->m_type = stmt->GetValueText(1);
        if (!stmt->IsColumnNull(2)) t->m_isVirtual = stmt->GetValueText(2);
        if (!stmt->IsColumnNull(3)) t->m_ordinal = stmt->GetValueText(3);
        if (!stmt->IsColumnNull(4)) t->m_notNullConstraint = stmt->GetValueText(4);
        if (!stmt->IsColumnNull(5)) t->m_uniqueConstraint = stmt->GetValueText(5);
        if (!stmt->IsColumnNull(6)) t->m_checkConstraint = stmt->GetValueText(6);
        if (!stmt->IsColumnNull(7)) t->m_defaultConstraint = stmt->GetValueText(7);
        if (!stmt->IsColumnNull(8)) t->m_collationConstraint = stmt->GetValueText(8);
        if (!stmt->IsColumnNull(9)) t->m_ordinalInPrimaryKey = stmt->GetValueText(9);
        table.m_columnsOrdered.push_back(t.get());
        table.m_columns[t->GetName()] = std::move(t);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
MapContext::ClassMap const* MapContext::LoadClassMap(Utf8CP schemaName, Utf8CP className)
    {
    Utf8String qualifiedName = schemaName;
    qualifiedName.append(":").append(className);
    auto itor = m_classMaps.find(qualifiedName);
    if (itor != m_classMaps.end())
        return itor->second.get();

    Utf8CP sql = R"(
                SELECT  [PP].[AccessString], 
                        [T].[Name] [Table], 
                        [C].[Name] [Column]
                FROM   [ec_ClassMap] [CM]
                        INNER JOIN [ec_PropertyMap] [PM] ON [PM].[ClassId] = [CM].[ClassId]
                        INNER JOIN [ec_PropertyPath] [PP] ON [PP].[Id] = [PM].[PropertyPathId]
                        INNER JOIN [ec_Class] [CL] ON [CL].[Id] = [PM].[ClassId]
                        INNER JOIN [ec_Schema] [S] ON [S].[Id] = [CL].[SchemaId]
                        INNER JOIN [ec_Column] [C] ON [C].[Id] = [PM].[ColumnId]
                        INNER JOIN [ec_Table] [T] ON [T].[Id] = [C].[TableId]
                WHERE  S.Name = ? AND CL.Name=?
                ORDER  BY [PM].[Id];)";
    auto stmt = m_ecdb.GetCachedStatement(sql);
    stmt->BindText(1, schemaName, Statement::MakeCopy::No);
    stmt->BindText(2, className, Statement::MakeCopy::No);

    std::unique_ptr<ClassMap> cm = std::unique_ptr<ClassMap>(new ClassMap());
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        Column const* column = FindColumn(stmt->GetValueText(1), stmt->GetValueText(2));
        std::unique_ptr<PropertyMap> t = std::unique_ptr<PropertyMap>(new PropertyMap(*cm, *column));
        t->m_accessString = stmt->GetValueText(0);
        cm->m_propertyMapsOrdered.push_back(t.get());
        cm->m_propertyMaps[t->GetAccessString()] = std::move(t);
        }

    if (cm->GetPropertyMaps().empty())
        return nullptr;

    ClassMap* p = cm.get();
    m_classMaps[p->GetFullName()] = std::move(cm);
    return p;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
MapContext::PropertyMap const* MapContext::FindPropertyMap(Utf8CP schemaName, Utf8CP className, Utf8CP accessString)
    {
    ClassMap const* cm = FindClassMap(schemaName, className);
    if (cm == nullptr)
        return nullptr;

    return cm->FindPropertyMap(accessString);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
MapContext::PropertyMap const* MapContext::FindPropertyMap(Utf8CP schemaName, Utf8CP className, Utf8CP accessString, Utf8CP table)
    {
    PropertyMap const* p = FindPropertyMap(schemaName, className, accessString);
    if (p == nullptr)
        return nullptr;

    if (!p->GetColumn().GetTable().GetName().EqualsI(table))
        return nullptr;

    return p;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
MapContext::PropertyMap const* MapContext::FindPropertyMap(Utf8CP schemaName, Utf8CP className, Utf8CP accessString, Utf8CP table, Utf8CP column)
    {
    PropertyMap const* p = FindPropertyMap(schemaName, className, accessString, table);
    if (p == nullptr)
        return nullptr;

    if (!p->GetColumn().GetName().EqualsI(column))
        return nullptr;

    return p;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
MapContext::ClassMap const* MapContext::FindClassMap(Utf8CP schemaName, Utf8CP className)
    {
    return LoadClassMap(schemaName, className);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
MapContext::Column const* MapContext::FindColumn(Utf8CP tableName, Utf8CP columnName)
    {
    Table const* table = FindTable(tableName);
    if (table == nullptr)
        return nullptr;

    return table->FindColumn(columnName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
MapContext::Table const* MapContext::FindTable(Utf8CP tableName)
    {
    if (m_tables.empty())
        LoadTables();

    auto itor = m_tables.find(tableName);
    if (itor != m_tables.end())
        return itor->second.get();

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
void MapContext::Clear()
    {
    m_classMaps.clear();
    m_tables.clear();
    }
END_ECDBUNITTESTS_NAMESPACE