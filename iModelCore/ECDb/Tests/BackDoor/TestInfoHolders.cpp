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

//avoid triggering destructor for static non-POD members -> hold as pointer and never free it
//static
Column const* Table::s_nullColumn = new Column();

//--------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                 06/17
//+---------------+---------------+---------------+---------------+---------------+------
void Table::AddColumn(Column&& column)
    {
    m_columns.push_back(column);
    m_columnLookupMap[m_columns.back().GetName()] = m_columns.size() - 1;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                 06/17
//+---------------+---------------+---------------+---------------+---------------+------
Column const& Table::GetColumn(Utf8StringCR name) const
    {
    auto it = m_columnLookupMap.find(name); 
    if (it == m_columnLookupMap.end())
        return *s_nullColumn;

    return m_columns[it->second];
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
void PrintTo(Column const& col, std::ostream* os)
    {
    if (!col.Exists())
        {
        *os << "<column does not exist>";
        return;
        }

    *os << "{" << col.GetTableName() << ":" << col.GetName() << ",";

    PrintTo(col.GetVirtual(), os);
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
bool ExpectedColumn::operator==(Column const& actual) const
    {
    if (!actual.Exists())
        return false;

    return m_name.EqualsIAscii(actual.GetName()) &&
        (m_tableName.IsNull() || m_tableName.Value().EqualsIAscii(actual.GetTableName())) &&
        (m_virtual.IsNull() || m_virtual.Value() == actual.GetVirtual()) &&
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
bool operator==(ExpectedColumns const& lhs, std::vector<Column> const& rhs)
    {
    const size_t lhsSize = lhs.size();
    if (lhsSize != rhs.size())
        return false;

    for (size_t i = 0; i < lhsSize; i++)
        {
        if (lhs[i] != rhs[i])
            return false;
        }

    return true;
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

    const bool whereClauseWasEmpty = m_whereClause.empty();
    if (!whereClauseWasEmpty)
        m_whereClause.append(" AND (");

    bool isFirstClassId = true;
    for (ECN::ECClassId classId : classIdFilter)
        {
        if (!isFirstClassId)
            m_whereClause.append(negatedClassIdFilter ? " AND " : " OR ");

        m_whereClause.append("ECClassId").append(negatedClassIdFilter ? "<>" : "=").append(classId.ToString());
        isFirstClassId = false;
        }

    if (!whereClauseWasEmpty)
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

        m_whereClause.append("[").append(indexColumn).append("] IS NOT NULL");
        isFirstCol = false;
        }

    m_whereClause.append(")");
    return *this;
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
    if (!mapStrategy.Exists())
        {
        *os << "Invalid MapStrategy (class map does not exist)";
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

END_ECDBUNITTESTS_NAMESPACE

