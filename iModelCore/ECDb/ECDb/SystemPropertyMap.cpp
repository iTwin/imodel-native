/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SystemPropertyMap.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//******************************** PropertyMapSystem ****************************************

void SystemPropertyMap::_QueryColumnMappedToProperty(ColumnMappedToPropertyList& result, ColumnMappedToProperty::LoadFlags loadFlags, bool recusive) const 
    {
    ColumnMappedToProperty info;
    if (Enum::Contains(loadFlags, ColumnMappedToProperty::LoadFlags::AccessString))
        info.SetAccessString(GetPropertyAccessString());

    if (Enum::Contains(loadFlags, ColumnMappedToProperty::LoadFlags::Column))
        info.SetColumn(*GetSingleColumn());

    if (Enum::Contains(loadFlags, ColumnMappedToProperty::LoadFlags::PropertyMap))
        info.SetPropertyMap(*this);

    if (Enum::Contains(loadFlags, ColumnMappedToProperty::LoadFlags::StrongType))
        info.SetStrongType(DbColumn::Type::Integer);

    result.push_back(info);
    }


//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
SystemPropertyMap::SystemPropertyMap(Type type, ECPropertyCR ecProperty, std::vector<DbColumn const*> columns, ECSqlSystemProperty kind)
    : PropertyMap(type, ecProperty, ecProperty.GetName().c_str(), nullptr), m_kind(kind)
    {
    BeAssert(!columns.empty());
    std::set<DbTable const*> tables;
    for (DbColumn const* column : columns)
        {
        if (column != nullptr)
            {
            DbTable const& table = column->GetTable();
            m_columns.push_back(table.FindColumnWeakPtr(column->GetName().c_str()));
            if (tables.find(&table) == tables.end())
                {
                m_mappedTables.push_back(&table);
                }
            }
        else
            {
            BeAssert(false && "Unexpected null column");
            }
        }
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
DbColumn const& SystemPropertyMap::GetColumn() const
    {
    BeAssert(!m_columns.front().expired() && m_columns.size() == 1);
    return *m_columns.front().lock().get();
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
void SystemPropertyMap::_GetColumns(std::vector<DbColumn const*>& columns) const
    {
    if (m_columns.empty())
        {
        BeAssert(!m_columns.empty());
        }

    columns.clear();
    for (std::weak_ptr<DbColumn> const& column : m_columns)
        {
        BeAssert(!column.expired());
        columns.push_back(column.lock().get());
        }
    }

//******************************** ECInstanceIdPropertyMap ****************************************
//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2013
//+---------------+---------------+---------------+---------------+---------------+-
ECInstanceIdPropertyMap::ECInstanceIdPropertyMap(ECPropertyCR ecInstanceIdProperty, ClassMap const& classMap, std::vector<DbColumn const*> columns)
    : SystemPropertyMap(Type::ECInstanceId, ecInstanceIdProperty, std::move(columns), ECSqlSystemProperty::ECInstanceId)
    {
    std::vector<DbTable*> tables = classMap.GetTables();
    m_mappedTables.clear();
    m_mappedTables.insert(m_mappedTables.begin(), tables.begin(), tables.end());
    }

PropertyMapPtr ECInstanceIdPropertyMap::Create(ECDbSchemaManagerCR schemaManager, ClassMap const& classMap, std::vector<DbColumn const*> columns)
    {
    ECPropertyCP property = ECDbSystemSchemaHelper::GetSystemProperty(schemaManager, ECSqlSystemProperty::ECInstanceId);
    if (property == nullptr)
        //log and assert done in child method
        return nullptr;

    return new ECInstanceIdPropertyMap(*property, classMap, columns);
    }
//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2013
//+---------------+---------------+---------------+---------------+---------------+-
PropertyMapPtr ECInstanceIdPropertyMap::Create(ECDbSchemaManagerCR schemaManager, ClassMap const& classMap)
    {
    ECPropertyCP property = ECDbSystemSchemaHelper::GetSystemProperty(schemaManager, ECSqlSystemProperty::ECInstanceId);
    if (property == nullptr)
        //log and assert done in child method
        return nullptr;

    std::vector<DbColumn const*> systemColumns;
    if (classMap.GetJoinedTable().GetFilteredColumnList(systemColumns, DbColumn::Kind::ECInstanceId) == BentleyStatus::ERROR)
        {
        BeAssert(false && "ECInstanceIdPropertyMap::Create> Table is expected to have primary key columns.");
        return nullptr;
        }

    return new ECInstanceIdPropertyMap(*property, classMap, systemColumns);
    }

//******************************** ECClassIdPropertyMap ****************************************

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2016
//+---------------+---------------+---------------+---------------+---------------+-
ECClassIdPropertyMap::ECClassIdPropertyMap(ECPropertyCR ecClassIdProperty, ClassMap const& classMap, std::vector<DbColumn const*> cols)
    : SystemPropertyMap(Type::ECClassId, ecClassIdProperty, cols, ECSqlSystemProperty::ECClassId), m_defaultConstraintClassId(classMap.GetClass().GetId())
    {
    std::vector<DbTable*> tables = classMap.GetTables();
    m_mappedTables.clear();
    m_mappedTables.insert(m_mappedTables.begin(), tables.begin(), tables.end());
    }


//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2016
//+---------------+---------------+---------------+---------------+---------------+-
PropertyMapPtr ECClassIdPropertyMap::Create(ECDbSchemaManagerCR schemaManager, ClassMap const& classMap, std::vector<DbColumn const*> cols)
    {
    ECPropertyCP property = ECDbSystemSchemaHelper::GetSystemProperty(schemaManager, ECSqlSystemProperty::ECClassId);
    if (property == nullptr)
        //log and assert done in child method
        return nullptr;

    return new ECClassIdPropertyMap(*property, classMap, cols);
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2016
//+---------------+---------------+---------------+---------------+---------------+-
NativeSqlBuilder::List ECClassIdPropertyMap::_ToNativeSql(Utf8CP classIdentifier, ECSqlType ecsqlType, bool wrapInParentheses, DbTable const* tableFilter) const
    {
    NativeSqlBuilder nativeSqlSnippet;
 
    if (wrapInParentheses)
        nativeSqlSnippet.AppendParenLeft(); //SelectClause, WhereClause, InsertCaluse, UpdateClause

    if (IsPersisted())
        {
        nativeSqlSnippet.Append(classIdentifier, GetColumn().GetName().c_str());
        }
    else
        {
        Utf8Char classIdStr[ECClassId::ID_STRINGBUFFER_LENGTH];
        GetDefaultConstraintClassId().ToString(classIdStr);
        nativeSqlSnippet.Append(classIdStr).AppendSpace().Append(/*GetColumn().GetName().c_str()*/ ECDB_COL_ECClassId);
        }

    if (wrapInParentheses)
        nativeSqlSnippet.AppendParenRight();

    NativeSqlBuilder::List nativeSqlSnippets;
    nativeSqlSnippets.push_back(std::move(nativeSqlSnippet));

    return {nativeSqlSnippet};
    }

//******************************** RelationshipConstraintPropertyMap ****************************************
//----------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                08/2013
//+---------------+---------------+---------------+---------------+---------------+-
RelationshipConstraintPropertyMap::RelationshipConstraintPropertyMap(Type type, ECN::ECPropertyCR constraintProperty, std::vector<DbColumn const*> columns, ECSqlSystemProperty kind, Utf8CP endTableColumnAlias)
    : SystemPropertyMap(type, constraintProperty, std::move(columns), kind), m_viewColumnAlias(endTableColumnAlias)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
void RelationshipConstraintPropertyMap::AppendSelectClauseSqlSnippetForView(NativeSqlBuilder& viewSql, DbTable const& table) const
    {
    DbColumn const* column = GetSingleColumn(table, false);
    if (column == nullptr)
        {
        BeAssert(column != nullptr && "Expecting a column");
        return;
        }

    viewSql.Append(column->GetName().c_str());
    if (HasViewColumnAlias())
        viewSql.AppendSpace().Append(GetViewColumnAlias());
    }

//******************************** PropertyMapRelationshipConstraintECInstanceId ****************************************
//----------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle             01/2014
//+---------------+---------------+---------------+---------------+---------------+-
RelationshipConstraintECInstanceIdPropertyMap::RelationshipConstraintECInstanceIdPropertyMap(ECPropertyCR constraintProperty, std::vector<DbColumn const*> columns, ECSqlSystemProperty kind, Utf8CP endTableColumnAlias)
    : RelationshipConstraintPropertyMap(Type::RelConstraintECInstanceId, constraintProperty, columns, kind, endTableColumnAlias)
    {}


//----------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                08/2013
//+---------------+---------------+---------------+---------------+---------------+-
PropertyMapPtr RelationshipConstraintECInstanceIdPropertyMap::Create(ECRelationshipEnd constraintEnd, ECDbSchemaManagerCR schemaManager, std::vector<DbColumn const*> columns, Utf8CP endTableColumnAlias)
    {
    auto kind = constraintEnd == ECN::ECRelationshipEnd_Source ? ECSqlSystemProperty::SourceECInstanceId : ECSqlSystemProperty::TargetECInstanceId;
    auto prop = ECDbSystemSchemaHelper::GetSystemProperty(schemaManager, kind);
    PRECONDITION(prop != nullptr, nullptr);

    return new RelationshipConstraintECInstanceIdPropertyMap(*prop, columns, kind, endTableColumnAlias);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
NativeSqlBuilder::List RelationshipConstraintECInstanceIdPropertyMap::_ToNativeSql(Utf8CP classIdentifier, ECSqlType ecsqlType, bool wrapInParentheses, DbTable const* tableFilter) const
    {
    NativeSqlBuilder nativeSqlSnippet;

    if (wrapInParentheses)
        nativeSqlSnippet.AppendParenLeft();
    //view column alias is only relevant for SELECT as in the native SQL the FROM table is a view
    //whose columns differ from the actual DbColumn name for end table mappings
    auto columnExp = ecsqlType == ECSqlType::Select && HasViewColumnAlias() ? GetViewColumnAlias() : GetColumn().GetName().c_str();
    nativeSqlSnippet.Append(classIdentifier, columnExp);

    if (wrapInParentheses)
        nativeSqlSnippet.AppendParenRight();

    NativeSqlBuilder::List nativeSqlSnippets;
    nativeSqlSnippets.push_back(std::move(nativeSqlSnippet));
    return nativeSqlSnippets;
    }

//******************************** PropertyMapRelationshipConstraintClassId ****************************************

//----------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle             01/2014
//+---------------+---------------+---------------+---------------+---------------+-
RelConstraintECClassIdPropertyMap::RelConstraintECClassIdPropertyMap(ECN::ECPropertyCR constraintProperty, std::vector<DbColumn const*> columns,
    ECSqlSystemProperty kind, ECClassId defaultConstraintECClassId, ClassMap const& classMap, Utf8CP endTableColumnAlias, bool colIsDelayGenerated)
    : RelationshipConstraintPropertyMap(Type::RelConstraintECClassId, constraintProperty, columns, kind, endTableColumnAlias), m_defaultConstraintClassId(defaultConstraintECClassId), m_isMappedToClassMapTables(false)
    {
    m_isMappedToClassMapTables = true;
    for (DbColumn const* column : columns)
        {
        m_isMappedToClassMapTables = m_isMappedToClassMapTables && classMap.IsMappedTo(column->GetTable());

        if (colIsDelayGenerated)
            {
            DbTable& table = classMap.GetPrimaryTable();
            table.AddColumnEventHandler([this] (DbTable::ColumnEvent evt, DbColumn& column)
                {
                auto ptrList = GetColumnWeakPtrs();
                if (evt == DbTable::ColumnEvent::Created && column.GetKind() == DbColumn::Kind::ECClassId)
                    {
                    for (auto itor = ptrList.begin(); itor != ptrList.end(); ++itor)
                        {
                        std::weak_ptr<DbColumn>& ptr = (*itor);
                        if (&column.GetTable() != &ptr.lock()->GetTable())
                            continue;

                        if (!ptr.expired())
                            {
                            DbTable& table = ptr.lock()->GetTableR();
                            if (SUCCESS != table.DeleteColumn(*ptr.lock()))
                                {
                                BeAssert(false && "DbTable::DeleteColumn failed in ColumnEventHandler of ECClassIdRelationshipConstraintPropertyMap");
                                break;
                                }
                            }

                        ptrList[std::distance(ptrList.begin(), itor)] = column.GetTable().FindColumnWeakPtr(column.GetName().c_str());
                        }
                    }
                });
            }
        }
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                11/2013
//+---------------+---------------+---------------+---------------+---------------+-
RefCountedPtr<RelConstraintECClassIdPropertyMap> RelConstraintECClassIdPropertyMap::Create(ECRelationshipEnd constraintEnd, ECDbSchemaManagerCR schemaManager,
                                                                                                             std::vector<DbColumn const*> columns, ECClassId defaultSourceECClassId, ClassMap const& classMap, Utf8CP endTableColumnAlias, bool colIsDelayGenerated)
    {
    auto kind = constraintEnd == ECN::ECRelationshipEnd_Source ? ECSqlSystemProperty::SourceECClassId : ECSqlSystemProperty::TargetECClassId;
    auto prop = ECDbSystemSchemaHelper::GetSystemProperty(schemaManager, kind);
    PRECONDITION(prop != nullptr, nullptr);

    return new RelConstraintECClassIdPropertyMap(*prop, columns, kind, defaultSourceECClassId, classMap, endTableColumnAlias, colIsDelayGenerated);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
NativeSqlBuilder::List RelConstraintECClassIdPropertyMap::_ToNativeSql(Utf8CP classIdentifier, ECSqlType ecsqlType, bool wrapInParentheses, DbTable const* tableFilter) const
    {
    NativeSqlBuilder nativeSqlSnippet;

    if (wrapInParentheses)
        nativeSqlSnippet.AppendParenLeft();

    if (ecsqlType == ECSqlType::Select)
        {
        //view column alias is only relevant for SELECT as in the native SQL the FROM table is a view
        //whose columns differ from the actual DbColumn name for end table mappings
        auto columnExp = ecsqlType == ECSqlType::Select && HasViewColumnAlias() ? GetViewColumnAlias() : GetColumn().GetName().c_str();
        nativeSqlSnippet.Append(classIdentifier, columnExp);
        }
    else
        {
        if (IsVirtual())
            {
            Utf8Char classIdStr[ECClassId::ID_STRINGBUFFER_LENGTH];
            m_defaultConstraintClassId.ToString(classIdStr);
            nativeSqlSnippet.Append(classIdStr);
            }
        else
            nativeSqlSnippet.Append(classIdentifier, GetColumn().GetName().c_str());
        }

    if (wrapInParentheses)
        nativeSqlSnippet.AppendParenRight();

    NativeSqlBuilder::List nativeSqlSnippets;
    nativeSqlSnippets.push_back(std::move(nativeSqlSnippet));
    return nativeSqlSnippets;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE

