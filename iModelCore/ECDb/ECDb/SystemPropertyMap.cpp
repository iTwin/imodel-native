/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SystemPropertyMap.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//******************************** PropertyMapSystem ****************************************
//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
PropertyMapSystem::PropertyMapSystem (ECPropertyCR ecProperty, DbColumnPtr& column, ECSqlSystemProperty kind)
: PropertyMap (ecProperty, ecProperty.GetName ().c_str (), nullptr), m_kind (kind), m_column (column)
    {
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
DbColumnCR PropertyMapSystem::GetColumn () const
    {
    BeAssert (m_column != nullptr);
    return *m_column;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//---------------------------------------------------------------------------------------
bool PropertyMapSystem::_IsVirtual () const
    {
    return m_column != nullptr && m_column->IsVirtual ();
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
MapStatus PropertyMapSystem::_FindOrCreateColumnsInTable (ClassMap& classMap)
    {
    //System columns were already created
    return MapStatus::Success;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
bool PropertyMapSystem::_MapsToColumn (Utf8CP columnName) const
    {
    return (m_column != nullptr) && (0 == BeStringUtilities::Stricmp (columnName, m_column->GetName ()));
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
void PropertyMapSystem::_GetColumns (DbColumnList& columns) const
    {
    BeAssert (m_column != nullptr);
    if (m_column != nullptr)
        columns.push_back (m_column.get ());
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
Utf8CP PropertyMapSystem::_GetColumnBaseName () const
    {
    BeAssert (m_column != nullptr);
    Utf8String propertyName (m_ecProperty.GetName ());
    Utf8CP columnName = m_column->GetName ();
    if (propertyName.EqualsI (columnName))
        return nullptr;
    else
        return columnName;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
void PropertyMapSystem::_SetColumnBaseName (Utf8CP columnName)
    {
    BeAssert (false && "PropertyMapSystem::_SetColumnBaseName should never be called.");
    }

//******************************** PropertyMapECInstanceId ****************************************
//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2013
//+---------------+---------------+---------------+---------------+---------------+-
//static
WCharCP const PropertyMapECInstanceId::PROPERTYACCESSSTRING = L"ECInstanceId";

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2013
//+---------------+---------------+---------------+---------------+---------------+-
PropertyMapECInstanceId::PropertyMapECInstanceId (ECPropertyCR ecInstanceIdProperty, DbColumnPtr& column)
: PropertyMapSystem (ecInstanceIdProperty, column, ECSqlSystemProperty::ECInstanceId)
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

    auto const& systemKeyColumns = classMap.GetTable ().GetSystemKeys ();
    if (systemKeyColumns.empty ())
        {
        BeAssert (false && "PropertyMapECInstanceId::Create> Table is expected to have primary key columns.");
        return nullptr;
        }

    auto systemColumn = systemKeyColumns[0];
    if (systemColumn == nullptr)
        {
        BeAssert (false && "PropertyMapECInstanceId::Create> Table is expected to have primary key columns.");
        return nullptr;
        }

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
WString PropertyMapECInstanceId::_ToString () const
    {
    return WPrintfString (L"PropertyMapECInstanceId: Column name=%ls", WString (GetColumn ().GetName (), BentleyCharEncoding::Utf8).c_str ());
    }



//******************************** PropertyMapSecondaryTableKey ****************************************
//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
PropertyMapSecondaryTableKey::PropertyMapSecondaryTableKey (ECPropertyCR ecProperty, DbColumnPtr& column, ECSqlSystemProperty kind)
 : PropertyMapSystem (ecProperty, column, kind)
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

    if (kind != ECSqlSystemProperty::ECInstanceId && kind != ECSqlSystemProperty::ECPropertyId && kind != ECSqlSystemProperty::ECArrayIndex&& kind != ECSqlSystemProperty::OwnerECInstanceId)
        {
        BeAssert (false && "PropertyMapSecondaryTableKey::Create must only be called with ECSqlSystemProperty::ECInstanceId, ECSqlSystemProperty::ECPropertyId, or ECSqlSystemProperty::ECArrayIndex or ECSqlSystemProperty::OwnerECInstanceId.");
        return nullptr;
        }

    auto property = ECDbSystemSchemaHelper::GetSystemProperty (schemaManager, kind);
    if (property == nullptr)
        //log and assert done in child method
        return nullptr;

    auto const& columns = classMap.GetTable ().GetSystemKeys();

    DbColumnPtr systemColumn = nullptr;
    for (auto& column : columns)
        {
        Utf8CP columnName = column->GetName ();
        bool found = false;
        switch (kind)
            {
                case ECSqlSystemProperty::ECInstanceId:
                    {
                    if (0 == BeStringUtilities::Stricmp (columnName, ECDB_COL_ECInstanceId))
                        found = true;
                       
                    break;
                    }
                case ECSqlSystemProperty::OwnerECInstanceId:
                    {
                    if (0 == BeStringUtilities::Stricmp (columnName, ECDB_COL_OwnerECInstanceId))
                        found = true;

                    break;
                    }
                case ECSqlSystemProperty::ECPropertyId:
                    {
                    if (0 == BeStringUtilities::Stricmp (columnName, ECDB_COL_ECPropertyId))
                        found = true;

                    break;
                    }
                case ECSqlSystemProperty::ECArrayIndex:
                    {
                    if (0 == BeStringUtilities::Stricmp (columnName, ECDB_COL_ECArrayIndex))
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

    return new PropertyMapSecondaryTableKey (*property, systemColumn, kind);
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
WString PropertyMapSecondaryTableKey::_ToString () const
 {
 return WPrintfString (L"PropertyMapSecondaryTableKey: Column name=%ls", WString (GetColumn ().GetName (), BentleyCharEncoding::Utf8).c_str ());
 }


//******************************** PropertyMapRelationshipConstraint ****************************************

//----------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                08/2013
//+---------------+---------------+---------------+---------------+---------------+-
PropertyMapRelationshipConstraint::PropertyMapRelationshipConstraint
(
ECN::ECPropertyCR constraintProperty,
DbColumnPtr& column,
ECSqlSystemProperty kind,
Utf8CP viewColumnAlias
)
: PropertyMapSystem (constraintProperty, column, kind), m_viewColumnAlias (viewColumnAlias)
    { 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
void PropertyMapRelationshipConstraint::AppendSelectClauseSqlSnippetForView (NativeSqlBuilder& viewSql) const
    {
    viewSql.Append (GetColumn ().GetName ());
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
DbColumnPtr& column,
ECSqlSystemProperty kind,
Utf8CP viewColumnAlias
)
: PropertyMapRelationshipConstraint (constraintProperty, column, kind, viewColumnAlias)
    {  
    }


//----------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                08/2013
//+---------------+---------------+---------------+---------------+---------------+-
PropertyMapPtr PropertyMapRelationshipConstraintECInstanceId::Create (ECRelationshipEnd constraintEnd, ECDbSchemaManagerCR schemaManager, DbColumnPtr& column, Utf8CP viewColumnAlias)
    {
    auto kind = constraintEnd == ECN::ECRelationshipEnd_Source ? ECSqlSystemProperty::SourceECInstanceId : ECSqlSystemProperty::TargetECInstanceId;
    auto prop = ECDbSystemSchemaHelper::GetSystemProperty (schemaManager, kind);
    PRECONDITION (prop != nullptr, nullptr);

    return new PropertyMapRelationshipConstraintECInstanceId (*prop, column, kind, viewColumnAlias);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
NativeSqlBuilder::List PropertyMapRelationshipConstraintECInstanceId::_ToNativeSql (Utf8CP classIdentifier, ECSqlType ecsqlType) const
    {
    NativeSqlBuilder nativeSqlSnippet;
    //view column alias is only relevant for SELECT as in the native SQL the FROM table is a view
    //whose columns differ from the actual DbColumn name for end table mappings
    auto columnExp = ecsqlType == ECSqlType::Select && HasViewColumnAlias () ? GetViewColumnAlias () : GetColumn ().GetName ();
    nativeSqlSnippet.Append (classIdentifier, columnExp);

    NativeSqlBuilder::List nativeSqlSnippets;
    nativeSqlSnippets.push_back (std::move (nativeSqlSnippet));
    return std::move (nativeSqlSnippets);
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                08/2013
//+---------------+---------------+---------------+---------------+---------------+-
WString PropertyMapRelationshipConstraintECInstanceId::_ToString () const
    {
    return WPrintfString (L"PropertyMapRelationshipConstraintECInstanceId: Column name=%ls View column alias=%ls",
        WString (GetColumn ().GetName (), BentleyCharEncoding::Utf8).c_str (),
        WString (GetViewColumnAlias (), BentleyCharEncoding::Utf8).c_str ());
    }

//******************************** PropertyMapRelationshipConstraintClassId ****************************************

//----------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle             01/2014
//+---------------+---------------+---------------+---------------+---------------+-
PropertyMapRelationshipConstraintClassId::PropertyMapRelationshipConstraintClassId
(
ECN::ECPropertyCR constraintProperty,
DbColumnPtr& column,
ECSqlSystemProperty kind,
ECClassId defaultConstraintECClassId,
Utf8CP viewColumnAlias,
DbTableP table
)
: PropertyMapRelationshipConstraint (constraintProperty, column, kind, viewColumnAlias),
m_defaultConstraintClassId (defaultConstraintECClassId)
    {
    if (table != nullptr)
        {
        table->AddAfterSetClassIdHook ([this] (DbColumnPtr& newClassIdCol)
            {
            ReplaceColumn (newClassIdCol);
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
DbColumnPtr& column,
ECClassId defaultSourceECClassId,
Utf8CP viewColumnAlias,
DbTableP table
)
    {
    auto kind = constraintEnd == ECN::ECRelationshipEnd_Source ? ECSqlSystemProperty::SourceECClassId : ECSqlSystemProperty::TargetECClassId;
    auto prop = ECDbSystemSchemaHelper::GetSystemProperty (schemaManager, kind);
    PRECONDITION (prop != nullptr, nullptr);

    return new PropertyMapRelationshipConstraintClassId (*prop, column, kind, defaultSourceECClassId, viewColumnAlias, table);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
NativeSqlBuilder::List PropertyMapRelationshipConstraintClassId::_ToNativeSql (Utf8CP classIdentifier, ECSqlType ecsqlType) const
    {
    NativeSqlBuilder nativeSqlSnippet;
    if (ecsqlType == ECSqlType::Select)
        {
        //view column alias is only relevant for SELECT as in the native SQL the FROM table is a view
        //whose columns differ from the actual DbColumn name for end table mappings
        auto columnExp = ecsqlType == ECSqlType::Select && HasViewColumnAlias () ? GetViewColumnAlias () : GetColumn ().GetName ();
        nativeSqlSnippet.Append (classIdentifier, columnExp);
        }
    else
        {
        if (IsVirtual ())
            nativeSqlSnippet.Append (m_defaultConstraintClassId);
        else
            nativeSqlSnippet.Append (classIdentifier, GetColumn ().GetName ());
        }

    NativeSqlBuilder::List nativeSqlSnippets;
    nativeSqlSnippets.push_back (std::move (nativeSqlSnippet));
    return std::move (nativeSqlSnippets);
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                08/2013
//+---------------+---------------+---------------+---------------+---------------+-
WString PropertyMapRelationshipConstraintClassId::_ToString () const
    {
    return WPrintfString (L"PropertyMapRelationshipConstraintClassId: Column name=%ls View column alias=%ls Default constraint ECClassId=%lld",
        WString (GetColumn ().GetName (), BentleyCharEncoding::Utf8).c_str (),
        WString (GetViewColumnAlias (), BentleyCharEncoding::Utf8).c_str (),
        m_defaultConstraintClassId);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

