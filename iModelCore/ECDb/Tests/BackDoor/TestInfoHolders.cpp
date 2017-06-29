/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/BackDoor/TestInfoHolders.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI/BackDoor/ECDb/TestInfoHolders.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//*****************************************************************
// Table 
//*****************************************************************

//--------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                 06/17
//+---------------+---------------+---------------+---------------+---------------+------
void Table::AddColumn(std::unique_ptr<Column> column)
    {
    Column* columnP = column.get();
    m_columns[columnP->GetName()] = std::move(column);
    m_columnsOrdered.push_back(columnP);
    }


// GTest Format customizations for types not handled by GTest

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  05/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(Table::Type type, std::ostream* os)
    {
    switch (type)
        {
            case Table::Type::Existing:
                *os << ENUM_TOSTRING(Table::Type::Existing);
                break;
            case Table::Type::Joined:
                *os << ENUM_TOSTRING(Table::Type::Joined);
                break;
            case Table::Type::Overflow:
                *os << ENUM_TOSTRING(Table::Type::Overflow);
                break;
            case Table::Type::Primary:
                *os << ENUM_TOSTRING(Table::Type::Primary);
                break;
            case Table::Type::Virtual:
                *os << ENUM_TOSTRING(Table::Type::Virtual);
                break;

            default:
                *os << "Unhandled Table::Type. Adjust the PrintTo method";
                break;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(Virtual virt, std::ostream* os)
    {
    switch (virt)
        {
            case Virtual::Yes:
                *os << ENUM_TOSTRING(Virtual::Yes);
                break;
            case Virtual::No:
                *os << ENUM_TOSTRING(Virtual::No);
                break;
            default:
                *os << "Unhandled Virtual enum value. Adjust the PrintTo method";
                break;
        }
    }

//*****************************************************************
// Column 
//*****************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(Column const* col, std::ostream* os)
    {
    if (col == nullptr)
        *os << "nullptr";
    else
        PrintTo(*col, os);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(Column const& col, std::ostream* os)
    {
    *os << "{" << col.GetTable().GetName() << ":" << col.GetName() << ",";

    PrintTo(col.IsVirtual(), os);
    *os << ",";

    PrintTo(col.GetKind(), os);
    *os << ",";

    PrintTo(col.GetType(), os);

    if (col.GetNotNullConstraint())
        *os << ",NOT NULL";

    if (col.GetUniqueConstraint())
        *os << ",UNIQUE";

    if (!col.GetCheckConstraint().empty())
        *os << ",CHECK '" << col.GetCheckConstraint() << "'";

    if (!col.GetDefaultConstraint().empty())
        *os << ",DEAULT '" << col.GetDefaultConstraint() << "'";

    if (col.GetCollationConstraint() != Column::Collation::Unset)
        {
        *os << ",";
        PrintTo(col.GetCollationConstraint(), os);
        }

    if (!col.GetOrdinalInPrimaryKey().IsNull())
        *os << ",Ordinal in PK: " << col.GetOrdinalInPrimaryKey().Value();

    *os << "}";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  05/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(Column::Type type, std::ostream* os)
    {
    switch (type)
        {
            case Column::Type::Any:
                *os << ENUM_TOSTRING(Column::Type::Any);
                break;
            case Column::Type::Blob:
                *os << ENUM_TOSTRING(Column::Type::Blob);
                break;
            case Column::Type::Boolean:
                *os << ENUM_TOSTRING(Column::Type::Boolean);
                break;
            case Column::Type::Integer:
                *os << ENUM_TOSTRING(Column::Type::Integer);
                break;
            case Column::Type::Text:
                *os << ENUM_TOSTRING(Column::Type::Text);
                break;
            case Column::Type::TimeStamp:
                *os << ENUM_TOSTRING(Column::Type::TimeStamp);
                break;

            default:
                *os << "Unhandled Column::Type. Adjust the PrintTo method";
                break;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  05/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(Column::Kind kind, std::ostream* os)
    {
    const int kindInt = (int) kind;

    bool isFirstMatch = true;
    if ((kindInt & (int) Column::Kind::DataColumn) != 0)
        {
        isFirstMatch = false;
        *os << ENUM_TOSTRING(Column::Kind::DataColumn);
        }

    if ((kindInt & (int) Column::Kind::ECClassId) != 0)
        {
        if (!isFirstMatch)
            *os << "|";

        isFirstMatch = false;
        *os << ENUM_TOSTRING(Column::Kind::ECClassId);
        }

    if ((kindInt & (int) Column::Kind::ECInstanceId) != 0)
        {
        if (!isFirstMatch)
            *os << "|";

        isFirstMatch = false;
        *os << ENUM_TOSTRING(Column::Kind::ECInstanceId);
        }

    if ((kindInt & (int) Column::Kind::RelECClassId) != 0)
        {
        if (!isFirstMatch)
            *os << "|";

        isFirstMatch = false;
        *os << ENUM_TOSTRING(Column::Kind::RelECClassId);
        }

    if ((kindInt & (int) Column::Kind::SharedDataColumn) != 0)
        {
        if (!isFirstMatch)
            *os << "|";

        isFirstMatch = false;
        *os << ENUM_TOSTRING(Column::Kind::SharedDataColumn);
        }

    if ((kindInt & (int) Column::Kind::SourceECClassId) != 0)
        {
        if (!isFirstMatch)
            *os << "|";

        isFirstMatch = false;
        *os << ENUM_TOSTRING(Column::Kind::SourceECClassId);
        }

    if ((kindInt & (int) Column::Kind::SourceECInstanceId) != 0)
        {
        if (!isFirstMatch)
            *os << "|";

        isFirstMatch = false;
        *os << ENUM_TOSTRING(Column::Kind::SourceECInstanceId);
        }

    if ((kindInt & (int) Column::Kind::TargetECClassId) != 0)
        {
        if (!isFirstMatch)
            *os << "|";

        isFirstMatch = false;
        *os << ENUM_TOSTRING(Column::Kind::TargetECClassId);
        }

    if ((kindInt & (int) Column::Kind::TargetECInstanceId) != 0)
        {
        if (!isFirstMatch)
            *os << "|";

        isFirstMatch = false;
        *os << ENUM_TOSTRING(Column::Kind::TargetECInstanceId);
        }

    if ((kindInt & (int) Column::Kind::Unknown) != 0)
        {
        if (!isFirstMatch)
            *os << "|";

        isFirstMatch = false;
        *os << ENUM_TOSTRING(Column::Kind::Unknown);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  05/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(Column::Collation collation, std::ostream* os)
    {
    switch (collation)
        {
            case Column::Collation::Binary:
                *os << ENUM_TOSTRING(Column::Collation::Binary);
                break;
            case Column::Collation::NoCase:
                *os << ENUM_TOSTRING(Column::Collation::NoCase);
                break;
            case Column::Collation::RTrim:
                *os << ENUM_TOSTRING(Column::Collation::RTrim);
                break;

            default:
                *os << "Unhandled Column::Collation. Adjust the PrintTo method";
                break;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(std::vector<Column const*> const& cols, std::ostream* os)
    {
    *os << "{";
    bool isFirstItem = true;
    for (Column const* col : cols)
        {
        if (!isFirstItem)
            *os << ",";

        PrintTo(*col, os);
        isFirstItem = false;
        }

    *os << "}";
    }

//*****************************************************************
// ExpectedColumn 
//*****************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
bool ExpectedColumn::operator==(Column const* actualCP) const
    {
    if (actualCP == nullptr)
        return false;

    Column const& actual = *actualCP;

    return m_name.EqualsIAscii(actual.GetName()) &&
        (m_tableName.IsNull() || m_tableName.Value().EqualsIAscii(actual.GetTable().GetName())) &&
        (m_virtual.IsNull() || m_virtual.Value() == actual.IsVirtual()) &&
        (m_kind.IsNull() || m_kind.Value() == actual.GetKind()) &&
        (m_type.IsNull() || m_type.Value() == actual.GetType()) &&
        (m_notNullConstraint.IsNull() || m_notNullConstraint.Value() == actual.GetNotNullConstraint()) &&
        (m_uniqueConstraint.IsNull() || m_uniqueConstraint.Value() == actual.GetUniqueConstraint()) &&
        (m_checkConstraint.IsNull() || m_checkConstraint.Value().EqualsIAscii(actual.GetCheckConstraint())) &&
        (m_defaultConstraint.IsNull() || m_defaultConstraint.Value().EqualsIAscii(actual.GetDefaultConstraint())) &&
        (m_collationConstraint.IsNull() || m_collationConstraint.Value() == actual.GetCollationConstraint()) &&
        (m_ordinalInPrimaryKey.IsNull() || m_ordinalInPrimaryKey == actual.GetOrdinalInPrimaryKey());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(ExpectedColumn const& expectedCol, std::ostream* os)
    {
    *os << "{";

    if (!expectedCol.m_tableName.IsNull())
        *os << expectedCol.m_tableName.Value() << ":";

    *os << expectedCol.m_name;

    if (!expectedCol.m_virtual.IsNull())
        {
        *os << ",";
        PrintTo(expectedCol.m_virtual.Value(), os);
        }

    if (!expectedCol.m_kind.IsNull())
        {
        *os << ",";
        PrintTo(expectedCol.m_kind.Value(), os);
        }

    if (!expectedCol.m_type.IsNull())
        {
        *os << ",";
        PrintTo(expectedCol.m_type.Value(), os);
        }

    if (!expectedCol.m_notNullConstraint.IsNull())
        *os << "," << "NOT NULL: " << expectedCol.m_notNullConstraint.Value();

    if (!expectedCol.m_uniqueConstraint.IsNull())
        *os << "," << "UNIQUE: " << expectedCol.m_uniqueConstraint.Value();

    if (!expectedCol.m_checkConstraint.IsNull())
        *os << "," << "CHECK: '" << expectedCol.m_checkConstraint.Value() << "'";

    if (!expectedCol.m_defaultConstraint.IsNull())
        *os << "," << "DEFAULT: '" << expectedCol.m_defaultConstraint.Value() << "'";

    if (!expectedCol.m_collationConstraint.IsNull())
        {
        *os << ",";
        PrintTo(expectedCol.m_collationConstraint.Value(), os);
        }

    if (!expectedCol.m_ordinalInPrimaryKey.IsNull())
        *os << "," << expectedCol.m_ordinalInPrimaryKey.Value();

    *os << "}";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(ExpectedColumns const& expectedCols, std::ostream* os)
    {
    *os << "{";
    bool isFirstItem = true;
    for (ExpectedColumn const& expectedCol : expectedCols)
        {
        if (!isFirstItem)
            *os << ",";

        PrintTo(expectedCol, os);
        isFirstItem = false;
        }

    *os << "}";
    }


//*****************************************************************
// IndexInfo 
//*****************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/17
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String IndexInfo::ToDdl() const
    {
    Utf8String ddl("CREATE ");
    if (m_isUnique)
        ddl.append("UNIQUE ");

    ddl.append("INDEX [").append(m_name).append("] ON [").append(m_table).append("](");
    bool isFirstColumn = true;
    for (Utf8StringCR column : m_indexColumns)
        {
        if (!isFirstColumn)
            ddl.append(", ");

        ddl.append("[").append(column).append("]");
        isFirstColumn = false;
        }

    ddl.append(")");
    if (m_whereClause.IsDefined())
        ddl.append(" WHERE ").append(m_whereClause.ToDdl());

    return ddl;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/17
//+---------------+---------------+---------------+---------------+---------------+------
IndexInfo::WhereClause& IndexInfo::WhereClause::AppendClassIdFilter(std::vector<ECN::ECClassId> const& classIdFilter, bool negatedClassIdFilter /*= false*/)
    {
    if (classIdFilter.empty())
        return *this;

    if (!m_whereClause.empty())
        m_whereClause.append(" AND ");

    m_whereClause.append("(");

    bool isFirstClassId = true;
    for (ECN::ECClassId classId : classIdFilter)
        {
        if (!isFirstClassId)
            m_whereClause.append(negatedClassIdFilter ? " AND " : " OR ");

        m_whereClause.append("ECClassId").append(negatedClassIdFilter ? "<>" : "=").append(classId.ToString());
        isFirstClassId = false;
        }

    m_whereClause.append(")");
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/17
//+---------------+---------------+---------------+---------------+---------------+------
IndexInfo::WhereClause&  IndexInfo::WhereClause::AppendNotNullFilter(std::vector<Utf8String> const& indexColumns)
    {
    if (indexColumns.empty())
        return *this;

    if (!m_whereClause.empty())
        m_whereClause.append(" AND ");

    m_whereClause.append("(");

    bool isFirstCol = true;
    for (Utf8StringCR indexColumn : indexColumns)
        {
        if (!isFirstCol)
            m_whereClause.append(" AND ");

        m_whereClause.append(indexColumn).append(" IS NOT NULL");
        isFirstCol = false;
        }

    m_whereClause.append(")");
    return *this;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                   06/17
//+---------------+---------------+---------------+---------------+---------------+------
PropertyMap* ClassMap::AddPropertyMap(std::unique_ptr<PropertyMap> pm)
    {
    PropertyMap* pmP = pm.get();
    m_propertyMapsOrdered.push_back(pmP);
    m_propertyMaps[pm->GetAccessString()] = std::move(pm);
    return pmP;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                 06/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(MapStrategy mapStrategy, std::ostream* os)
    {
    switch (mapStrategy)
        {
            case MapStrategy::ExistingTable:
                *os << "MapStrategy::ExistingTable";
                break;

            case MapStrategy::ForeignKeyRelationshipInSourceTable:
                *os << "MapStrategy::ForeignKeyRelationshipInSourceTable";
                break;

            case MapStrategy::ForeignKeyRelationshipInTargetTable:
                *os << "MapStrategy::ForeignKeyRelationshipInTargetTable";
                break;

            case MapStrategy::NotMapped:
                *os << "MapStrategy::NotMapped";
                break;

            case MapStrategy::OwnTable:
                *os << "MapStrategy::OwnTable";
                break;

            case MapStrategy::TablePerHierarchy:
                *os << "MapStrategy::TablePerHierarchy";
                break;

            default:
                BeAssert(false && "MapStrategy enum has changed. Adjust this method");
                return;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                 06/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(MapStrategyInfo const& mapStrategy, std::ostream* os)
    {
    if (!mapStrategy.IsValid())
        {
        *os << "Invalid MapStrategy";
        return;
        }

    *os << "MapStrategyInfo(";
    PrintTo(mapStrategy.GetStrategy(), os);
    *os << ",";
    PrintTo(mapStrategy.GetTphInfo(), os);
    *os << ")";
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                 06/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(MapStrategyInfo::TablePerHierarchyInfo const& tphInfo, std::ostream* os)
    {
    switch (tphInfo.m_sharedColumnsMode)
        {
            case MapStrategyInfo::TablePerHierarchyInfo::ShareColumnsMode::ApplyToSubclassesOnly:
                *os << "ShareColumnsMode::ApplyToSubclassesOnly";
                break;
            case MapStrategyInfo::TablePerHierarchyInfo::ShareColumnsMode::Yes:
                *os << "ShareColumnsMode::Yes";
                break;
            case MapStrategyInfo::TablePerHierarchyInfo::ShareColumnsMode::No:
                *os << "ShareColumnsMode::No";
                break;

            default:
                BeAssert(false && "ShareColumnsMode enum has changed. Adjust this method");
                return;
        }

    if (tphInfo.m_maxSharedColumnsBeforeOverflow >= 0)
        *os << ",MaxSharedColumnsBeforeOverflow: " << tphInfo.m_maxSharedColumnsBeforeOverflow;

    *os << ",";

    switch (tphInfo.m_joinedTableInfo)
        {
            case MapStrategyInfo::JoinedTableInfo::JoinedTable:
                *os << "JoinedTableInfo::JoinedTable";
                break;

            case MapStrategyInfo::JoinedTableInfo::None:
                *os << "JoinedTableInfo::None";
                break;

            case MapStrategyInfo::JoinedTableInfo::ParentOfJoinedTable:
                *os << "JoinedTableInfo::ParentOfJoinedTable";
                break;

            default:
                BeAssert(false && "JoinedTableInfo enum has changed. Adjust this method");
                return;
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
//static 
std::vector<Column const*> const* PropertyMap::s_emptyColumnList = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
//static 
std::vector<Column const*> const& PropertyMap::EmptyColumnList()
    {
    //avoid triggering destructor for static non-POD members -> hold as pointer and never free it
    if (s_emptyColumnList == nullptr)
        s_emptyColumnList = new std::vector<Column const*>();

    return *s_emptyColumnList;
    }

/*
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
        if (auto pm = cm->FindPropertyMap(accessString))
            const_cast<PropertyMap*>(pm)->AddColumn(*column);
        else
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
MapContext::PropertyMap const* MapContext::FindPropertyMap(AccessString const& accessString)
    {
    ClassMap const* cm = FindClassMap(accessString.m_schemaNameOrAlias.c_str(), accessString.m_className.c_str());
    if (cm == nullptr)
        return nullptr;

    return cm->FindPropertyMap(accessString.m_propAccessString.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
MapContext::PropertyMap const* MapContext::FindPropertyMap(AccessString const& accessString, Utf8CP table)
    {
    PropertyMap const* p = FindPropertyMap(accessString);
    if (p == nullptr)
        return nullptr;

    for (Column const* column : p->GetColumns())
        if (column->GetTable().GetName().EqualsI(table))
            return p;

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
MapContext::PropertyMap const* MapContext::FindPropertyMap(AccessString const& accessString, Utf8CP table, Utf8CP column)
    {
    PropertyMap const* p = FindPropertyMap(accessString, table);
    if (p == nullptr)
        return nullptr;

    for (Column const* col : p->GetColumns())
        if (col->GetTable().GetName().EqualsI(table) && col->GetName().EqualsI(column))
            return p;

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

*/

END_ECDBUNITTESTS_NAMESPACE

