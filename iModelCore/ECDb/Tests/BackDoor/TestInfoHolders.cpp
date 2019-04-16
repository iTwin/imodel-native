/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI/BackDoor/ECDb/TestInfoHolders.h"
#include "PublicAPI/BackDoor/ECDb/TestHelper.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//**************************************************************************************
// ComparableJsonCppValue
//**************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle     12/17
//---------------------------------------------------------------------------------------
JsonValue::JsonValue(Utf8CP json)
    {
    if (SUCCESS != TestUtilities::ParseJson(m_value, json))
        m_value = Json::Value(Json::nullValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle     10/17
//---------------------------------------------------------------------------------------
bool JsonValue::operator==(JsonValue const& rhs) const
    {
    if (m_value.isNull())
        return rhs.m_value.isNull();

    if (m_value.isArray())
        {
        if (!rhs.m_value.isArray() || m_value.size() != rhs.m_value.size())
            return false;

        for (Json::ArrayIndex i = 0; i < m_value.size(); i++)
            {
            if (JsonValue(m_value[i]) != JsonValue(rhs.m_value[i]))
                return false;
            }

        return true;
        }

    if (m_value.isObject())
        {
        if (!rhs.m_value.isObject())
            return false;

        bvector<Utf8String> lhsMemberNames = m_value.getMemberNames();
        if (lhsMemberNames.size() != rhs.m_value.size())
            return false;

        for (Utf8StringCR memberName : lhsMemberNames)
            {
            if (!rhs.m_value.isMember(memberName))
                return false;

            if (JsonValue(m_value[memberName]) != JsonValue(rhs.m_value[memberName]))
                return false;
            }

        return true;
        }

    if (m_value.isIntegral())
        {
        if (!rhs.m_value.isIntegral())
            return false;

        if (m_value.isBool())
            return rhs.m_value.isBool() && m_value.asBool() == rhs.m_value.asBool();

        if (m_value.isInt())
            return rhs.m_value.isConvertibleTo(Json::intValue) && m_value.asInt64() == rhs.m_value.asInt64();

        if (m_value.isUInt())
            return rhs.m_value.isConvertibleTo(Json::uintValue) && m_value.asUInt64() == rhs.m_value.asUInt64();

        BeAssert(false && "Should not end up here");
        return false;
        }

    if (m_value.isDouble())
        return rhs.m_value.isDouble() && TestUtilities::Equals(m_value.asDouble(), rhs.m_value.asDouble());

    if (m_value.isString())
        return rhs.m_value.isString() && strcmp(m_value.asCString(), rhs.m_value.asCString()) == 0;

    BeAssert(false && "Unhandled JsonCPP value type. This method needs to be adjusted");
    return false;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(JsonValue const& json, std::ostream* os) { *os << json.ToString(); }

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
// @bsimethod                                      Krischan.Eberle                  11/17
//+---------------+---------------+---------------+---------------+---------------+------
bool operator==(Table::Type lhs, Nullable<Table::Type> rhs) { return rhs == lhs; }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  11/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(Nullable<Table::Type> type, std::ostream* os)
    {
    if (type.IsNull())
        {
        *os << "<unset>";
        return;
        }

    PrintTo(type.Value(), os);
    }

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
    switch (kind)
        {
            case Column::Kind::Default:
                *os << ENUM_TOSTRING(Column::Kind::Default);
                break;

            case Column::Kind::ECClassId:
                *os << ENUM_TOSTRING(Column::Kind::ECClassId);
                break;

            case Column::Kind::ECInstanceId:
                *os << ENUM_TOSTRING(Column::Kind::ECInstanceId);
                break;
            case Column::Kind::SharedData:
                *os << ENUM_TOSTRING(Column::Kind::SharedData);
                break;

            default:
                *os << "Unhandled Column::Kind. Adjust the PrintTo method";
                break;
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
Utf8CP MapStrategyInfo::MapStrategyToString(MapStrategy strat)
    {
    switch (strat)
        {
            case MapStrategy::ExistingTable:
                return ENUM_TOSTRING(MapStrategy::ExistingTable);

            case MapStrategy::ForeignKeyRelationshipInSourceTable:
                return ENUM_TOSTRING(MapStrategy::ForeignKeyRelationshipInSourceTable);

            case MapStrategy::ForeignKeyRelationshipInTargetTable:
                return ENUM_TOSTRING(MapStrategy::ForeignKeyRelationshipInTargetTable);

            case MapStrategy::NotMapped:
                return ENUM_TOSTRING(MapStrategy::NotMapped);

            case MapStrategy::OwnTable:
                return ENUM_TOSTRING(MapStrategy::OwnTable);

            case MapStrategy::TablePerHierarchy:
                return ENUM_TOSTRING(MapStrategy::TablePerHierarchy);

            default:
                BeAssert(false);
                return "";
        }
    }
//--------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                 06/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(MapStrategy mapStrategy, std::ostream* os) { *os << MapStrategyInfo::MapStrategyToString(mapStrategy); }

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

