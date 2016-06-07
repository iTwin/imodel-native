/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SystemPropertyMap.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
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
// @bsimethod                                 Affan.Khan                02/2016
//+---------------+---------------+---------------+---------------+---------------+-
std::vector<DbColumn const*> SystemPropertyMap::ToVector(DbColumn const* column)
    {
    std::vector<DbColumn const*> tmp;
    tmp.push_back(column);
    return tmp;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
SystemPropertyMap::SystemPropertyMap(ECPropertyCR ecProperty, std::vector<DbColumn const*> columns, ECSqlSystemProperty kind)
    : PropertyMap(ecProperty, ecProperty.GetName().c_str(), nullptr), m_kind(kind)
    {
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

//******************************** PropertyMapECInstanceId ****************************************
//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2013
//+---------------+---------------+---------------+---------------+---------------+-
//static
Utf8CP const ECInstanceIdPropertyMap::PROPERTYACCESSSTRING = "ECInstanceId";

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2013
//+---------------+---------------+---------------+---------------+---------------+-
ECInstanceIdPropertyMap::ECInstanceIdPropertyMap(ECPropertyCR ecInstanceIdProperty, ClassMap const& classMap, std::vector<DbColumn const*> columns)
    : SystemPropertyMap(ecInstanceIdProperty, std::move(columns), ECSqlSystemProperty::ECInstanceId)
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

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
Utf8String ECInstanceIdPropertyMap::_ToString() const
    {
    return Utf8PrintfString("PropertyMapECInstanceId: Column name=%s", GetColumn().GetName().c_str());
    }


//******************************** PropertyMapRelationshipConstraint ****************************************
//----------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                08/2013
//+---------------+---------------+---------------+---------------+---------------+-
RelationshipConstraintPropertyMap::RelationshipConstraintPropertyMap(ECN::ECPropertyCR constraintProperty, std::vector<DbColumn const*> columns, ECSqlSystemProperty kind, Utf8CP endTableColumnAlias)
    : SystemPropertyMap(constraintProperty, std::move(columns), kind), m_viewColumnAlias(endTableColumnAlias)
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
ECInstanceIdRelationshipConstraintPropertyMap::ECInstanceIdRelationshipConstraintPropertyMap(ECPropertyCR constraintProperty, std::vector<DbColumn const*> columns, ECSqlSystemProperty kind, Utf8CP endTableColumnAlias)
    : RelationshipConstraintPropertyMap(constraintProperty, columns, kind, endTableColumnAlias)
    {}


//----------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                08/2013
//+---------------+---------------+---------------+---------------+---------------+-
PropertyMapPtr ECInstanceIdRelationshipConstraintPropertyMap::Create(ECRelationshipEnd constraintEnd, ECDbSchemaManagerCR schemaManager, std::vector<DbColumn const*> columns, Utf8CP endTableColumnAlias)
    {
    auto kind = constraintEnd == ECN::ECRelationshipEnd_Source ? ECSqlSystemProperty::SourceECInstanceId : ECSqlSystemProperty::TargetECInstanceId;
    auto prop = ECDbSystemSchemaHelper::GetSystemProperty(schemaManager, kind);
    PRECONDITION(prop != nullptr, nullptr);

    return new ECInstanceIdRelationshipConstraintPropertyMap(*prop, columns, kind, endTableColumnAlias);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
NativeSqlBuilder::List ECInstanceIdRelationshipConstraintPropertyMap::_ToNativeSql(Utf8CP classIdentifier, ECSqlType ecsqlType, bool wrapInParentheses, DbTable const* tableFilter) const
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

//----------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                08/2013
//+---------------+---------------+---------------+---------------+---------------+-
Utf8String ECInstanceIdRelationshipConstraintPropertyMap::_ToString() const
    {
    return Utf8PrintfString("ECInstanceIdRelationshipConstraintPropertyMap: Column name=%s View column alias=%s",
                            GetColumn().GetName().c_str(), GetViewColumnAlias());
    }

//******************************** PropertyMapRelationshipConstraintClassId ****************************************

//----------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle             01/2014
//+---------------+---------------+---------------+---------------+---------------+-
ECClassIdRelationshipConstraintPropertyMap::ECClassIdRelationshipConstraintPropertyMap(ECN::ECPropertyCR constraintProperty, std::vector<DbColumn const*> columns,
    ECSqlSystemProperty kind, ECClassId defaultConstraintECClassId, ClassMap const& classMap, Utf8CP endTableColumnAlias, bool colIsDelayGenerated)
    : RelationshipConstraintPropertyMap(constraintProperty, columns, kind, endTableColumnAlias), m_defaultConstraintClassId(defaultConstraintECClassId), m_isMappedToClassMapTables(false)
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
RefCountedPtr<ECClassIdRelationshipConstraintPropertyMap> ECClassIdRelationshipConstraintPropertyMap::Create(ECRelationshipEnd constraintEnd, ECDbSchemaManagerCR schemaManager,
                                                                                                             std::vector<DbColumn const*> columns, ECClassId defaultSourceECClassId, ClassMap const& classMap, Utf8CP endTableColumnAlias, bool colIsDelayGenerated)
    {
    auto kind = constraintEnd == ECN::ECRelationshipEnd_Source ? ECSqlSystemProperty::SourceECClassId : ECSqlSystemProperty::TargetECClassId;
    auto prop = ECDbSystemSchemaHelper::GetSystemProperty(schemaManager, kind);
    PRECONDITION(prop != nullptr, nullptr);

    return new ECClassIdRelationshipConstraintPropertyMap(*prop, columns, kind, defaultSourceECClassId, classMap, endTableColumnAlias, colIsDelayGenerated);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
NativeSqlBuilder::List ECClassIdRelationshipConstraintPropertyMap::_ToNativeSql(Utf8CP classIdentifier, ECSqlType ecsqlType, bool wrapInParentheses, DbTable const* tableFilter) const
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

//----------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                08/2013
//+---------------+---------------+---------------+---------------+---------------+-
Utf8String ECClassIdRelationshipConstraintPropertyMap::_ToString() const
    {
    Utf8String str;
    str.Sprintf("ECClassIdRelationshipConstraintPropertyMap: Column name=%s View column alias=%s Default constraint ECClassId=%s",
                GetColumn().GetName().c_str(), GetViewColumnAlias(), m_defaultConstraintClassId.ToString().c_str());
    return str;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

