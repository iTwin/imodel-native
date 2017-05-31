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

    CachedStatementPtr stmt = m_ecdb.GetCachedStatement(R"sql(
                SELECT t.Name, 
                       t.Type, 
                       tp.Name ParentTable, 
                       s.Name ExclusiveRootSchema, 
                       c.Name ExclusiveRootClass
                FROM   ec_Table t
                       LEFT JOIN ec_Table tp ON tp.Id = t.ParentTableId
                       LEFT JOIN ec_Class c ON c.Id = t.ExclusiveRootClassId
                       LEFT JOIN ec_Schema s ON s.Id = c.SchemaId
                ORDER  BY t.Name)sql");
    ASSERT_TRUE(stmt != nullptr);

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        ASSERT_FALSE(stmt->IsColumnNull(0)) << stmt->GetSql();
        Utf8CP tableName = stmt->GetValueText(0);
        Table::Type type = (Table::Type) stmt->GetValueInt(1);
        Utf8CP parentTableName = !stmt->IsColumnNull(2) ? stmt->GetValueText(2) : nullptr;
        Utf8CP exclusiveRootSchemaName = !stmt->IsColumnNull(3) ? stmt->GetValueText(3) : nullptr;
        Utf8CP exclusiveRootClassName = !stmt->IsColumnNull(4) ? stmt->GetValueText(4) : nullptr;

        std::unique_ptr<Table> table = std::make_unique<Table>(tableName, type, parentTableName, exclusiveRootSchemaName, exclusiveRootClassName);
        ASSERT_EQ(SUCCESS, LoadColumns(*table));
        m_tables[tableName] = std::move(table);
        }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus MapContext::LoadColumns(Table& table) const
    {
    if (!table.GetColumns().empty())
        return ERROR;

    CachedStatementPtr stmt = m_ecdb.GetCachedStatement(R"sql(
                        SELECT c.Name, c.Type, c.IsVirtual, c.Ordinal, 
                               c.NotNullConstraint, c.UniqueConstraint, c.CheckConstraint, c.DefaultConstraint, 
                               CASE c.CollationConstraint WHEN 0 THEN 'Unset' WHEN 1 THEN 'Binary' WHEN 2 THEN 'NoCase' WHEN 3 THEN 'RTrim' ELSE '<err>' END, 
                               c.OrdinalInPrimaryKey, c.ColumnKind
                        FROM ec_Column c
                        INNER JOIN ec_Table t ON t.Id = c.TableId AND t.Name=? 
                        ORDER BY c.TableId, c.Id;)sql");

    if (stmt == nullptr)
        return ERROR;

    stmt->BindText(1, table.GetName(), Statement::MakeCopy::No);
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        EXPECT_FALSE(stmt->IsColumnNull(0)) << stmt->GetSql();
        std::unique_ptr<Column> c = std::unique_ptr<Column>(new Column(table, stmt->GetValueText(0)));

        if (!stmt->IsColumnNull(1)) 
            c->m_type = (Column::Type) stmt->GetValueInt(1);

        if (!stmt->IsColumnNull(2)) 
            c->m_isVirtual = stmt->GetValueBoolean(2);

        if (!stmt->IsColumnNull(3)) 
            c->m_ordinal = stmt->GetValueInt(3);

        if (!stmt->IsColumnNull(4)) 
            c->m_notNullConstraint = stmt->GetValueBoolean(4);

        if (!stmt->IsColumnNull(5)) 
            c->m_uniqueConstraint = stmt->GetValueBoolean(5);

        if (!stmt->IsColumnNull(6)) 
            c->m_checkConstraint = stmt->GetValueText(6);

        if (!stmt->IsColumnNull(7)) 
            c->m_defaultConstraint = stmt->GetValueText(7);

        if (!stmt->IsColumnNull(8)) 
            c->m_collationConstraint = stmt->GetValueText(8);

        if (!stmt->IsColumnNull(9)) 
            c->m_ordinalInPrimaryKey = stmt->GetValueInt(9);

        table.AddColumn(std::move(c));
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
MapContext::ClassMap const* MapContext::LoadClassMap(Utf8CP schemaName, Utf8CP className)
    {
    Utf8String qualifiedName(schemaName);
    qualifiedName.append(":").append(className);
    auto itor = m_classMaps.find(qualifiedName);
    if (itor != m_classMaps.end())
        return itor->second.get();

    CachedStatementPtr stmt = m_ecdb.GetCachedStatement(R"sql(
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
                ORDER  BY [PM].[Id];)sql");
    if (stmt == nullptr)
        {
        EXPECT_TRUE(stmt != nullptr);
        return nullptr;
        }

    stmt->BindText(1, schemaName, Statement::MakeCopy::No);
    stmt->BindText(2, className, Statement::MakeCopy::No);

    std::unique_ptr<ClassMap> cm = std::unique_ptr<ClassMap>(new ClassMap());
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        Utf8CP accessString = stmt->GetValueText(0);;
        Column const* column = FindColumn(stmt->GetValueText(1), stmt->GetValueText(2));
        cm->AddPropertyMap(std::make_unique<PropertyMap>(*cm, *column, Utf8String(accessString)));
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
MapContext::PropertyMap const* MapContext::FindPropertyMap(PropertyAccessString const& accessString)
    {
    ClassMap const* cm = FindClassMap(accessString.m_schemaNameOrAlias.c_str(), accessString.m_className.c_str());
    if (cm == nullptr)
        return nullptr;

    return cm->FindPropertyMap(accessString.m_propAccessString.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
MapContext::PropertyMap const* MapContext::FindPropertyMap(PropertyAccessString const& accessString, Utf8CP table)
    {
    PropertyMap const* p = FindPropertyMap(accessString);
    if (p == nullptr)
        return nullptr;

    if (!p->GetColumn().GetTable().GetName().EqualsI(table))
        return nullptr;

    return p;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
MapContext::PropertyMap const* MapContext::FindPropertyMap(PropertyAccessString const& accessString, Utf8CP table, Utf8CP column)
    {
    PropertyMap const* p = FindPropertyMap(accessString, table);
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


// GTest Format customizations for types not handled by GTest

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  05/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(MapContext::Table::Type type, std::ostream* os)
    {
    switch (type)
        {
            case MapContext::Table::Type::Existing:
                *os << ENUM_TOSTRING(MapContext::Table::Type::Existing);
                break;
            case MapContext::Table::Type::Joined:
                *os << ENUM_TOSTRING(MapContext::Table::Type::Joined);
                break;
            case MapContext::Table::Type::Overflow:
                *os << ENUM_TOSTRING(MapContext::Table::Type::Overflow);
                break;
            case MapContext::Table::Type::Primary:
                *os << ENUM_TOSTRING(MapContext::Table::Type::Primary);
                break;
            case MapContext::Table::Type::Virtual:
                *os << ENUM_TOSTRING(MapContext::Table::Type::Virtual);
                break;

            default:
                *os << "Unhandled Table::Type. Adjust the PrintTo method";
                break;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  05/17
//+---------------+---------------+---------------+---------------+---------------+------
void Tests::PrintTo(MapContext::Column::Type type, std::ostream* os)
    {
    switch (type)
        {
            case MapContext::Column::Type::Any:
                *os << ENUM_TOSTRING(MapContext::Column::Type::Any);
                break;
            case MapContext::Column::Type::Blob:
                *os << ENUM_TOSTRING(MapContext::Column::Type::Blob);
                break;
            case MapContext::Column::Type::Boolean:
                *os << ENUM_TOSTRING(MapContext::Column::Type::Boolean);
                break;
            case MapContext::Column::Type::Integer:
                *os << ENUM_TOSTRING(MapContext::Column::Type::Integer);
                break;
            case MapContext::Column::Type::Text:
                *os << ENUM_TOSTRING(MapContext::Column::Type::Text);
                break;
            case MapContext::Column::Type::TimeStamp:
                *os << ENUM_TOSTRING(MapContext::Column::Type::TimeStamp);
                break;

            default:
                *os << "Unhandled Column::Type. Adjust the PrintTo method";
                break;
        }
    }

END_ECDBUNITTESTS_NAMESPACE

