/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMapSubclasses.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ClassMapSubclasses.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//************************** SecondaryTableClassMap ***************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
SecondaryTableClassMap::SecondaryTableClassMap (ECN::ECClassCR ecClass, ECDbMapCR ecDbMap, MapStrategy mapStrategy, bool setIsDirty)
: ClassMap (ecClass, ecDbMap, mapStrategy, setIsDirty), m_embeddedTypeClassView (nullptr)
    {
    m_embeddedTypeClassView = std::unique_ptr<EmbeddedTypeClassMap> (new EmbeddedTypeClassMap (*this));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
ClassMap::Type SecondaryTableClassMap::_GetClassMapType () const
    {
    return Type::SecondaryTable;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
MapStatus SecondaryTableClassMap::_OnInitialized ()
    {
    return m_embeddedTypeClassView->Initialize ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2014
//---------------------------------------------------------------------------------------
ClassMap::NativeSqlConverter const& SecondaryTableClassMap::_GetNativeSqlConverter () const
    {
    if (m_nativeSqlConverter == nullptr)
        m_nativeSqlConverter = std::unique_ptr<NativeSqlConverter> (new SecondaryTableClassMap::NativeSqlConverterImpl (*this));

    return *m_nativeSqlConverter;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
IClassMap const& SecondaryTableClassMap::_GetView (View classView) const
    {
    if (classView == IClassMap::View::EmbeddedType)
        return *m_embeddedTypeClassView;

    return ClassMap::_GetView (classView);
    }

//************************** SecondaryTableClassMap::NativeSqlConverterImpl *******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
SecondaryTableClassMap::NativeSqlConverterImpl::NativeSqlConverterImpl (ClassMapCR classMap)
: ClassMap::NativeSqlConverterImpl (classMap)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
// 
// ECSQL_TODO  Remove this funtion. Each child ECSql statement must create ECSQL that add ECPropertyId and 
// ECArrayIndex as they are now properties and have property may.
// Preparing via type flag just add complication to code when the actully child statement 
// what to use them in different way then this where statement - Affan(Added to do)
//---------------------------------------------------------------------------------------
ECSqlStatus SecondaryTableClassMap::NativeSqlConverterImpl::_GetWhereClause
(
NativeSqlBuilder& whereClauseBuilder, ECSqlType ecsqlType, bool isPolymorphicClassExp, Utf8CP tableAlias
) const
    {
    BeAssert (whereClauseBuilder.IsEmpty ());
    whereClauseBuilder.AppendParenLeft ().Append (tableAlias, ECDB_COL_ECPropertyId).Append (" IS NULL AND ");
    whereClauseBuilder.Append (tableAlias, ECDB_COL_ECArrayIndex).Append (" IS NULL)");

    return ClassMap::NativeSqlConverterImpl::_GetWhereClause (whereClauseBuilder, ecsqlType, isPolymorphicClassExp, tableAlias);
    }


//************************** SecondaryTableClassMap::EmbeddedClassMap *******************

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
MapStatus SecondaryTableClassMap::EmbeddedTypeClassMap::Initialize ()
    {
    auto const& schemaManager = GetECDbMap ().GetECDbR ().GetSchemaManager ();
    auto systemPropMap = PropertyMapSecondaryTableKey::Create (schemaManager, ECSqlSystemProperty::ECInstanceId, *this);
    m_embeddedClassViewPropMaps.AddPropertyMap (systemPropMap);

    systemPropMap = PropertyMapSecondaryTableKey::Create (schemaManager, ECSqlSystemProperty::OwnerECInstanceId, *this);
    m_embeddedClassViewPropMaps.AddPropertyMap (systemPropMap);

    systemPropMap = PropertyMapSecondaryTableKey::Create (schemaManager, ECSqlSystemProperty::ECPropertyId, *this);
    m_embeddedClassViewPropMaps.AddPropertyMap (systemPropMap);

    systemPropMap = PropertyMapSecondaryTableKey::Create (schemaManager, ECSqlSystemProperty::ECArrayIndex, *this);
    m_embeddedClassViewPropMaps.AddPropertyMap (systemPropMap);

    for (auto propMap : m_secondaryTableClassMap.GetPropertyMaps ())
        {
        PropertyMapPtr propMapPtr = nullptr;
        m_secondaryTableClassMap.GetPropertyMaps ().TryGetPropertyMap (propMapPtr, propMap->GetPropertyAccessString ());
        //skip ECInstanceIdPropertyMap as it is not used in the embedded class map view
        if (propMapPtr->IsECInstanceIdPropertyMap ())
            continue;

        m_embeddedClassViewPropMaps.AddPropertyMap (propMapPtr);
        }

    return MapStatus::Success;
    }

//************************** UnmappedClassMap ***************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
UnmappedClassMap::UnmappedClassMap (ECClassCR ecClass, ECDbMapCR ecdbMap, MapStrategy mapStrategy, bool setIsDirty)
: ClassMap (ecClass, ecdbMap, mapStrategy, setIsDirty)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
MapStatus UnmappedClassMap::_InitializePart1 (ClassMapInfoCR classMapInfo, IClassMap const* parentClassMap)
    {
    m_dbView = std::unique_ptr<ClassDbView> (new ClassDbView (*this));

    if (parentClassMap != nullptr)
        m_parentMapClassId = parentClassMap->GetParentMapClassId ();
    auto nullTable = GetECDbMap ().GetSQLManager ().GetNullTable ();
    SetTable (const_cast<ECDbSqlTable*> (nullTable));

    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
MapStatus UnmappedClassMap::_InitializePart2 (ClassMapInfoCR classMapInfo, IClassMap const* parentClassMap)
    {
    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2014
//---------------------------------------------------------------------------------------
ClassMap::NativeSqlConverter const& UnmappedClassMap::_GetNativeSqlConverter () const
    {
    if (m_nativeSqlConverter == nullptr)
        m_nativeSqlConverter = std::unique_ptr<NativeSqlConverter> (new UnmappedClassMap::NativeSqlConverterImpl ());

    return *m_nativeSqlConverter;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
