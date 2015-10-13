/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SystemPropertyMap.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//******************************** PropertyMapSystem ****************************************
//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
PropertyMapSystem::PropertyMapSystem (ECPropertyCR ecProperty, std::weak_ptr<ECDbSqlColumn> column, ECSqlSystemProperty kind, ECDbSqlTable const* primaryTable)
: PropertyMap (ecProperty, ecProperty.GetName ().c_str (), primaryTable, nullptr), m_kind (kind), m_column (column)
    {
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
ECDbSqlColumn const& PropertyMapSystem::GetColumn () const
    {
    BeAssert (m_column.expired() == false);
    return *m_column.lock().get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//---------------------------------------------------------------------------------------
bool PropertyMapSystem::_IsVirtual () const
    {
    return !m_column.expired () && m_column.lock()->GetPersistenceType () == PersistenceType::Virtual;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
bool PropertyMapSystem::_IsECInstanceIdPropertyMap () const
    {
    return false;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
bool PropertyMapSystem::_IsSystemPropertyMap () const
    {
    return true;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
void PropertyMapSystem::_GetColumns (std::vector<ECDbSqlColumn const*>& columns) const
    {
    BeAssert (!m_column.expired ());
    if (!m_column.expired())
        columns.push_back (m_column.lock().get());
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
Utf8CP PropertyMapSystem::_GetColumnBaseName () const
    {
    BeAssert (!m_column.expired ());
    Utf8String propertyName (m_ecProperty.GetName ());
    if (propertyName.EqualsI (m_column.lock()->GetName ()))
        return nullptr;
    else
        return m_column.lock()->GetName ().c_str();
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
PropertyMapECInstanceId::PropertyMapECInstanceId (ECPropertyCR ecInstanceIdProperty, ECDbSqlColumn* column)
: PropertyMapSystem (ecInstanceIdProperty, (column != nullptr ? column->GetWeakPtr () : std::weak_ptr<ECDbSqlColumn> ()), ECSqlSystemProperty::ECInstanceId, &(column->GetTable()))
    {}

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2013
//+---------------+---------------+---------------+---------------+---------------+-
PropertyMapPtr PropertyMapECInstanceId::Create (ECDbSchemaManagerCR schemaManager, IClassMap const& classMap)
    {
    auto property = ECDbSystemSchemaHelper::GetSystemProperty (schemaManager, ECSqlSystemProperty::ECInstanceId);
    if (property == nullptr)
        //log and assert done in child method
        return nullptr;

    std::vector<ECDbSqlColumn const*> systemColumns;
    if (classMap.GetTable ().GetFilteredColumnList (systemColumns, ColumnKind::ECInstanceId) == BentleyStatus::ERROR)
        {
        BeAssert (false && "PropertyMapECInstanceId::Create> Table is expected to have primary key columns.");
        return nullptr;
        }

    auto systemColumn = const_cast<ECDbSqlColumn*>(systemColumns.front ());
    //if (systemColumn->GetDependentPropertiesR ().Add (classMap.GetClass ().GetId (), property->GetName().c_str()) != BentleyStatus::SUCCESS)
    //    return nullptr;

    return new PropertyMapECInstanceId (*property, systemColumn);
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                11/2013
//+---------------+---------------+---------------+---------------+---------------+-
bool PropertyMapECInstanceId::_IsECInstanceIdPropertyMap () const
    {
    return true;
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
PropertyMapSecondaryTableKey::PropertyMapSecondaryTableKey (ECPropertyCR ecProperty, ECDbSqlColumn* column, ECSqlSystemProperty kind)
: PropertyMapSystem (ecProperty, column->GetWeakPtr (), kind, &(column->GetTable ()))
 {}

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
PropertyMapPtr PropertyMapSecondaryTableKey::Create (ECDbSchemaManagerCR schemaManager, ECSqlSystemProperty kind, IClassMap const& classMap)
    {
    if (!classMap.IsMappedToSecondaryTable ())
        {
        BeAssert (false && "PropertyMapSecondaryTableKey must only be used with class maps using ArrayStorage.");
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
    if (classMap.GetTable ().GetFilteredColumnList (systemColumns, ColumnKind::NonRelSystemColumn) == BentleyStatus::ERROR)
        {
        BeAssert (false && "PropertyMapECInstanceId::Create> Table is expected to have primary key columns.");
        return nullptr;
        }

    ECDbSqlColumn* systemColumn = nullptr;
    for (auto& column : systemColumns)
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
            systemColumn = const_cast<ECDbSqlColumn*>(column);
            break;
            }
        }

    if (systemColumn == nullptr)
        {
        BeAssert (false && "No column found for PropertyMapSecondaryTableKey");
        return nullptr;
        }

    return new PropertyMapSecondaryTableKey (*property, systemColumn, kind);
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
Utf8String PropertyMapSecondaryTableKey::_ToString () const
 {
 return Utf8PrintfString ("PropertyMapSecondaryTableKey: Column name=%s", GetColumn ().GetName ().c_str());
 }


//******************************** PropertyMapRelationshipConstraint ****************************************

//----------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                08/2013
//+---------------+---------------+---------------+---------------+---------------+-
PropertyMapRelationshipConstraint::PropertyMapRelationshipConstraint
(
ECN::ECPropertyCR constraintProperty,
ECDbSqlColumn* column,
ECSqlSystemProperty kind,
ECDbSqlTable const* primaryTable,
Utf8CP viewColumnAlias
)
: PropertyMapSystem (constraintProperty, column->GetWeakPtr (), kind, primaryTable), m_viewColumnAlias (viewColumnAlias)
    { 
    }

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
PropertyMapRelationshipConstraintECInstanceId::PropertyMapRelationshipConstraintECInstanceId
(
ECPropertyCR constraintProperty,
ECDbSqlColumn* column,
ECSqlSystemProperty kind,
Utf8CP viewColumnAlias
)
: PropertyMapRelationshipConstraint (constraintProperty, column, kind, &column->GetTable(), viewColumnAlias)
    {  
    }


//----------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                08/2013
//+---------------+---------------+---------------+---------------+---------------+-
PropertyMapPtr PropertyMapRelationshipConstraintECInstanceId::Create (ECRelationshipEnd constraintEnd, ECDbSchemaManagerCR schemaManager, ECDbSqlColumn* column, IClassMap const& classMap, Utf8CP viewColumnAlias)
    {
    auto kind = constraintEnd == ECN::ECRelationshipEnd_Source ? ECSqlSystemProperty::SourceECInstanceId : ECSqlSystemProperty::TargetECInstanceId;
    auto prop = ECDbSystemSchemaHelper::GetSystemProperty (schemaManager, kind);
    PRECONDITION (prop != nullptr, nullptr);

    return new PropertyMapRelationshipConstraintECInstanceId (*prop, column, kind, viewColumnAlias);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
NativeSqlBuilder::List PropertyMapRelationshipConstraintECInstanceId::_ToNativeSql (Utf8CP classIdentifier, ECSqlType ecsqlType, bool wrapInParentheses) const
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
ECDbSqlColumn* column,
ECSqlSystemProperty kind,
ECClassId defaultConstraintECClassId,
ECDbSqlTable const* primaryTable,
Utf8CP viewColumnAlias,
ECDbSqlTable* table
)
: PropertyMapRelationshipConstraint (constraintProperty, column, kind, primaryTable, viewColumnAlias),
m_defaultConstraintClassId (defaultConstraintECClassId)
    {
    if (table != nullptr)
        {
        table->AddColumnEventHandler ([this, table] (ECDbSqlTable::ColumnEvent evt, ECDbSqlColumn& column)
            {
            if (evt == ECDbSqlTable::ColumnEvent::Created && column.GetKind() == ColumnKind::ECClassId)
                {
                if (!GetColumnWeakPtr ().expired ())
                    table->DeleteColumn (GetColumnWeakPtr ().lock()->GetName ().c_str ());

                ReplaceColumn (column.GetWeakPtr());
                }
            });
        }
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                11/2013
//+---------------+---------------+---------------+---------------+---------------+-
PropertyMapPtr PropertyMapRelationshipConstraintClassId::Create
(
ECRelationshipEnd constraintEnd,
ECDbSchemaManagerCR schemaManager,
ECDbSqlColumn* column,
ECClassId defaultSourceECClassId,
IClassMap const& classMap,
Utf8CP viewColumnAlias,
ECDbSqlTable* table
)
    {
    auto kind = constraintEnd == ECN::ECRelationshipEnd_Source ? ECSqlSystemProperty::SourceECClassId : ECSqlSystemProperty::TargetECClassId;
    auto prop = ECDbSystemSchemaHelper::GetSystemProperty (schemaManager, kind);
    PRECONDITION (prop != nullptr, nullptr);
    return new PropertyMapRelationshipConstraintClassId (*prop, column, kind, defaultSourceECClassId, &classMap.GetTable(), viewColumnAlias, table);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
NativeSqlBuilder::List PropertyMapRelationshipConstraintClassId::_ToNativeSql (Utf8CP classIdentifier, ECSqlType ecsqlType, bool wrapInParentheses) const
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

