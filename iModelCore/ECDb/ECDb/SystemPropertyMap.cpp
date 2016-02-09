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
//----------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2016
//+---------------+---------------+---------------+---------------+---------------+-
std::vector<ECDbSqlColumn const*> PropertyMapSystem::ToVector(ECDbSqlColumn const* column)
    {
    std::vector<ECDbSqlColumn const*> tmp;
    tmp.push_back(column);
    return std::move(tmp);
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                02/2016
//+---------------+---------------+---------------+---------------+---------------+-
std::vector<std::weak_ptr<ECDbSqlColumn>> PropertyMapSystem::ToWeakPtr(std::vector<ECDbSqlColumn const*>const& columns)
    {
    std::vector<std::weak_ptr<ECDbSqlColumn>> tmp;
    for (ECDbSqlColumn const* column : columns)
        {
        if (column != nullptr)
            tmp.push_back(column->GetWeakPtr());
        }

    BeAssert(tmp.size() == columns.size());
    return std::move(tmp);
    }
//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
PropertyMapSystem::PropertyMapSystem(ECPropertyCR ecProperty, std::vector<ECDbSqlColumn const*> columns, ECSqlSystemProperty kind)
    : PropertyMap(ecProperty, ecProperty.GetName().c_str(), nullptr), m_kind(kind)
    {
    std::set<ECDbSqlTable const*> tables;
    for (ECDbSqlColumn const* column : columns)
        {
        if (column != nullptr)
            {
            m_columns.push_back(column->GetWeakPtr());
            if (tables.find(&column->GetTable()) == tables.end())
                {
                m_mappedTables.push_back(&column->GetTable());
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
ECDbSqlColumn const& PropertyMapSystem::GetColumn() const
    {
    BeAssert(!m_columns.front().expired() && m_columns.size() == 1);
    return *m_columns.front().lock().get();
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                01/2016
//+---------------+---------------+---------------+---------------+---------------+-
ECDbSqlColumn const* PropertyMapSystem::GetColumn(ECDbSqlTable const& table) const
    {
    BeAssert(!m_columns.empty());
    if (&GetColumn().GetTable() == &table)
        return &GetColumn();


    for (std::weak_ptr<ECDbSqlColumn> const& column: m_columns)
        {
        if (&table == &column.lock()->GetTable())
            return column.lock().get();
        }

    return nullptr;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
void PropertyMapSystem::_GetColumns (std::vector<ECDbSqlColumn const*>& columns) const
    {
    if (m_columns.empty())
        {
        BeAssert(!m_columns.empty());
        }

    columns.clear();
    for (std::weak_ptr<ECDbSqlColumn> const& column : m_columns)
        {
        BeAssert (!column.expired ());
        columns.push_back(column.lock().get());
        }
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                01/2016
//+---------------+---------------+---------------+---------------+---------------+-
ColumnKind PropertyMapSystem::ToColumnKind() const
    {
    switch (m_kind)
        {
        case ECSqlSystemProperty::ECArrayIndex:
            return ColumnKind::ECArrayIndex;

        case ECSqlSystemProperty::ECInstanceId:
            return ColumnKind::ECInstanceId;

        case ECSqlSystemProperty::ECPropertyPathId:
            return ColumnKind::ECPropertyPathId;

        case ECSqlSystemProperty::ParentECInstanceId:
            return ColumnKind::ParentECInstanceId;

        case ECSqlSystemProperty::SourceECClassId:
            return ColumnKind::SourceECClassId;

        case ECSqlSystemProperty::SourceECInstanceId:
            return ColumnKind::SourceECInstanceId;

        case ECSqlSystemProperty::TargetECClassId:
            return ColumnKind::TargetECClassId;

        case ECSqlSystemProperty::TargetECInstanceId:
            return ColumnKind::TargetECInstanceId;

        default:
            BeAssert(false);
            return ColumnKind::Unknown;
        }
    }

//******************************** PropertyMapECInstanceId ****************************************
//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2013
//+---------------+---------------+---------------+---------------+---------------+-
//static
Utf8CP const PropertyMapECInstanceId::PROPERTYACCESSSTRING = "ECInstanceId";

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2013
//+---------------+---------------+---------------+---------------+---------------+-
PropertyMapECInstanceId::PropertyMapECInstanceId (ECPropertyCR ecInstanceIdProperty, ClassMap const& classMap, std::vector<ECDbSqlColumn const*> columns)
: PropertyMapSystem (ecInstanceIdProperty, std::move(columns), ECSqlSystemProperty::ECInstanceId)
    {
   
    std::vector<ECDbSqlTable*> tables = classMap.GetTables();
    m_mappedTables.clear();
    m_mappedTables.insert(m_mappedTables.begin(), tables.begin(), tables.end());
    }

PropertyMapPtr PropertyMapECInstanceId::Create(ECDbSchemaManagerCR schemaManager, ClassMap const& classMap, std::vector<ECDbSqlColumn const*> columns)
    {
    ECPropertyCP property = ECDbSystemSchemaHelper::GetSystemProperty(schemaManager, ECSqlSystemProperty::ECInstanceId);
    if (property == nullptr)
        //log and assert done in child method
        return nullptr;

    return new PropertyMapECInstanceId(*property, classMap, columns);
    }
//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2013
//+---------------+---------------+---------------+---------------+---------------+-
PropertyMapPtr PropertyMapECInstanceId::Create (ECDbSchemaManagerCR schemaManager, ClassMap const& classMap)
    {
    ECPropertyCP property = ECDbSystemSchemaHelper::GetSystemProperty (schemaManager, ECSqlSystemProperty::ECInstanceId);
    if (property == nullptr)
        //log and assert done in child method
        return nullptr;

    std::vector<ECDbSqlColumn const*> systemColumns;
    if (classMap.GetJoinedTable().GetFilteredColumnList(systemColumns, ColumnKind::ECInstanceId) == BentleyStatus::ERROR)
        {
        BeAssert(false && "PropertyMapECInstanceId::Create> Table is expected to have primary key columns.");
        return nullptr;
        }

    return new PropertyMapECInstanceId (*property, classMap, systemColumns);
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
Utf8String PropertyMapECInstanceId::_ToString () const
    {
    return Utf8PrintfString ("PropertyMapECInstanceId: Column name=%s", GetColumn ().GetName ().c_str());
    }



//******************************** PropertyMapSecondaryTableKey ****************************************
//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
PropertyMapStructArrayTableKey::PropertyMapStructArrayTableKey (ECPropertyCR ecProperty, std::vector<ECDbSqlColumn const*> columns, ECSqlSystemProperty kind)
: PropertyMapSystem (ecProperty, std::move(columns), kind)
 {}

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
PropertyMapPtr PropertyMapStructArrayTableKey::Create (ECDbSchemaManagerCR schemaManager, ECSqlSystemProperty kind, IClassMap const& classMap)
    {
    if (!classMap.MapsToStructArrayTable ())
        {
        BeAssert (false);
        return nullptr;
        }

    if (kind != ECSqlSystemProperty::ECInstanceId && kind != ECSqlSystemProperty::ECPropertyPathId && kind != ECSqlSystemProperty::ECArrayIndex&& kind != ECSqlSystemProperty::ParentECInstanceId)
        {
        BeAssert (false && "PropertyMapSecondaryTableKey::Create must only be called with ECSqlSystemProperty::ECInstanceId, ECSqlSystemProperty::ECPropertyPathId, or ECSqlSystemProperty::ECArrayIndex or ECSqlSystemProperty::ParentECInstanceId.");
        return nullptr;
        }

    auto property = ECDbSystemSchemaHelper::GetSystemProperty (schemaManager, kind);
    if (property == nullptr)
        //log and assert done in child method
        return nullptr;

    std::vector<ECDbSqlColumn const*> systemColumns;  
    if (classMap.GetJoinedTable().GetFilteredColumnList (systemColumns, ColumnKind::NonRelSystemColumn) == BentleyStatus::ERROR)
        {
        BeAssert (false && "PropertyMapECInstanceId::Create> Table is expected to have primary key columns.");
        return nullptr;
        }

    ECDbSqlColumn const* systemColumn = nullptr;
    for (ECDbSqlColumn const* column : systemColumns)
        {
        bool found = false;
        switch (kind)
            {
                case ECSqlSystemProperty::ECInstanceId:
                {
                if (column->GetKind() == ColumnKind::ECInstanceId)
                    found = true;

                break;
                }
                case ECSqlSystemProperty::ParentECInstanceId:
                {
                if (column->GetKind() == ColumnKind::ParentECInstanceId)
                    found = true;

                break;
                }
                case ECSqlSystemProperty::ECPropertyPathId:
                {
                if (column->GetKind() == ColumnKind::ECPropertyPathId)
                    found = true;

                break;
                }
                case ECSqlSystemProperty::ECArrayIndex:
                {
                if (column->GetKind() == ColumnKind::ECArrayIndex)
                    found = true;

                break;
                }
                default:
                    break;
            }

        if (found)
            {
            systemColumn = column;
            break;
            }
        }

    if (systemColumn == nullptr)
        {
        BeAssert (false && "No column found for PropertyMapSecondaryTableKey");
        return nullptr;
        }

    return new PropertyMapStructArrayTableKey(*property, std::move(ToVector(systemColumn)), kind);
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
Utf8String PropertyMapStructArrayTableKey::_ToString () const
 {
 return Utf8PrintfString ("PropertyMapSecondaryTableKey: Column name=%s", GetColumn ().GetName ().c_str());
 }


//******************************** PropertyMapRelationshipConstraint ****************************************
//----------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                08/2013
//+---------------+---------------+---------------+---------------+---------------+-
PropertyMapRelationshipConstraint::PropertyMapRelationshipConstraint(ECN::ECPropertyCR constraintProperty, std::vector<ECDbSqlColumn const*> columns, ECSqlSystemProperty kind, Utf8CP endTableColumnAlias)
    : PropertyMapSystem(constraintProperty, std::move(columns), kind),m_viewColumnAlias(endTableColumnAlias)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
void PropertyMapRelationshipConstraint::AppendSelectClauseSqlSnippetForView (NativeSqlBuilder& viewSql) const
    {
    viewSql.Append (GetColumn ().GetName ().c_str());
    if (HasViewColumnAlias ())
        viewSql.AppendSpace ().Append (GetViewColumnAlias ());
    }


//******************************** PropertyMapRelationshipConstraintECInstanceId ****************************************

//----------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle             01/2014
//+---------------+---------------+---------------+---------------+---------------+-
PropertyMapRelationshipConstraintECInstanceId::PropertyMapRelationshipConstraintECInstanceId (ECPropertyCR constraintProperty, std::vector<ECDbSqlColumn const*> columns, ECSqlSystemProperty kind, Utf8CP endTableColumnAlias)
: PropertyMapRelationshipConstraint (constraintProperty, columns, kind, endTableColumnAlias)
    {}


//----------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                08/2013
//+---------------+---------------+---------------+---------------+---------------+-
PropertyMapPtr PropertyMapRelationshipConstraintECInstanceId::Create (ECRelationshipEnd constraintEnd, ECDbSchemaManagerCR schemaManager, std::vector<ECDbSqlColumn const*> columns, Utf8CP endTableColumnAlias)
    {
    auto kind = constraintEnd == ECN::ECRelationshipEnd_Source ? ECSqlSystemProperty::SourceECInstanceId : ECSqlSystemProperty::TargetECInstanceId;
    auto prop = ECDbSystemSchemaHelper::GetSystemProperty (schemaManager, kind);
    PRECONDITION (prop != nullptr, nullptr);

    return new PropertyMapRelationshipConstraintECInstanceId (*prop, columns, kind, endTableColumnAlias);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
NativeSqlBuilder::List PropertyMapRelationshipConstraintECInstanceId::_ToNativeSql (Utf8CP classIdentifier, ECSqlType ecsqlType, bool wrapInParentheses, ECDbSqlTable const* tableFilter) const
    {
    NativeSqlBuilder nativeSqlSnippet;

    if (wrapInParentheses)
        nativeSqlSnippet.AppendParenLeft();
    //view column alias is only relevant for SELECT as in the native SQL the FROM table is a view
    //whose columns differ from the actual DbColumn name for end table mappings
    auto columnExp = ecsqlType == ECSqlType::Select && HasViewColumnAlias () ? GetViewColumnAlias () : GetColumn ().GetName ().c_str();
    nativeSqlSnippet.Append (classIdentifier, columnExp);

    if (wrapInParentheses)
        nativeSqlSnippet.AppendParenRight();

    NativeSqlBuilder::List nativeSqlSnippets;
    nativeSqlSnippets.push_back (std::move (nativeSqlSnippet));
    return std::move (nativeSqlSnippets);
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                08/2013
//+---------------+---------------+---------------+---------------+---------------+-
Utf8String PropertyMapRelationshipConstraintECInstanceId::_ToString () const
    {
    return Utf8PrintfString ("PropertyMapRelationshipConstraintECInstanceId: Column name=%s View column alias=%s",
        GetColumn ().GetName ().c_str(), GetViewColumnAlias ());
    }

//******************************** PropertyMapRelationshipConstraintClassId ****************************************

//----------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle             01/2014
//+---------------+---------------+---------------+---------------+---------------+-
PropertyMapRelationshipConstraintClassId::PropertyMapRelationshipConstraintClassId
(
ECN::ECPropertyCR constraintProperty,
std::vector<ECDbSqlColumn const*> columns,
ECSqlSystemProperty kind,
ECClassId defaultConstraintECClassId,
ClassMap const& classMap, 
Utf8CP endTableColumnAlias,
bool colIsDelayGenerated
)
: PropertyMapRelationshipConstraint (constraintProperty, columns, kind, endTableColumnAlias),
m_defaultConstraintClassId (defaultConstraintECClassId), m_isMappedToClassMapTables(false)
    {
    for (ECDbSqlColumn const* column : columns)
        {
        m_isMappedToClassMapTables = classMap.IsMappedTo(column->GetTable());

        if (colIsDelayGenerated)
            {
            ECDbSqlTable& table = classMap.GetPrimaryTable();
            table.AddColumnEventHandler([this] (ECDbSqlTable::ColumnEvent evt, ECDbSqlColumn& column)
                {
                auto ptrList = GetColumnWeakPtrs();
                if (evt == ECDbSqlTable::ColumnEvent::Created && column.GetKind() == ColumnKind::ECClassId)
                    {
                    for (auto itor = ptrList.begin(); itor != ptrList.end(); ++itor)
                        {
                        std::weak_ptr<ECDbSqlColumn>& ptr = (*itor);
                        if (&column.GetTable() != &ptr.lock()->GetTable())
                            continue;

                        if (!ptr.expired())
                            {
                            ECDbSqlTable&  table = ptr.lock()->GetTableR();
                            table.DeleteColumn(ptr.lock()->GetName().c_str());
                            
                            }

                        ptrList[std::distance(ptrList.begin(), itor)] = column.GetWeakPtr();

                        }
                    }
                });
            }
        }
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                11/2013
//+---------------+---------------+---------------+---------------+---------------+-
RefCountedPtr<PropertyMapRelationshipConstraintClassId> PropertyMapRelationshipConstraintClassId::Create(ECRelationshipEnd constraintEnd, ECDbSchemaManagerCR schemaManager,
                        std::vector<ECDbSqlColumn const*> columns, ECClassId defaultSourceECClassId, ClassMap const& classMap, Utf8CP endTableColumnAlias, bool colIsDelayGenerated)
    {
    auto kind = constraintEnd == ECN::ECRelationshipEnd_Source ? ECSqlSystemProperty::SourceECClassId : ECSqlSystemProperty::TargetECClassId;
    auto prop = ECDbSystemSchemaHelper::GetSystemProperty(schemaManager, kind);
    PRECONDITION(prop != nullptr, nullptr);

    return new PropertyMapRelationshipConstraintClassId(*prop, columns, kind, defaultSourceECClassId, classMap,endTableColumnAlias, colIsDelayGenerated);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
NativeSqlBuilder::List PropertyMapRelationshipConstraintClassId::_ToNativeSql (Utf8CP classIdentifier, ECSqlType ecsqlType, bool wrapInParentheses, ECDbSqlTable const* tableFilter) const
    {
    NativeSqlBuilder nativeSqlSnippet;

    if (wrapInParentheses)
        nativeSqlSnippet.AppendParenLeft();

    if (ecsqlType == ECSqlType::Select)
        {
        //view column alias is only relevant for SELECT as in the native SQL the FROM table is a view
        //whose columns differ from the actual DbColumn name for end table mappings
        auto columnExp = ecsqlType == ECSqlType::Select && HasViewColumnAlias () ? GetViewColumnAlias () : GetColumn ().GetName ().c_str();
        nativeSqlSnippet.Append (classIdentifier, columnExp);
        }
    else
        {
        if (IsVirtual ())
            nativeSqlSnippet.Append (m_defaultConstraintClassId);
        else
            nativeSqlSnippet.Append (classIdentifier, GetColumn ().GetName ().c_str());
        }

    if (wrapInParentheses)
        nativeSqlSnippet.AppendParenRight();

    NativeSqlBuilder::List nativeSqlSnippets;
    nativeSqlSnippets.push_back (std::move (nativeSqlSnippet));
    return std::move (nativeSqlSnippets);
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                08/2013
//+---------------+---------------+---------------+---------------+---------------+-
Utf8String PropertyMapRelationshipConstraintClassId::_ToString () const
    {
    return Utf8PrintfString ("PropertyMapRelationshipConstraintClassId: Column name=%s View column alias=%s Default constraint ECClassId=%lld",
        GetColumn ().GetName ().c_str(), GetViewColumnAlias (), m_defaultConstraintClassId);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

