/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMapSubclasses.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ClassMapSubclasses.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//************************** SecondaryTableClassMap ***************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
StructClassMap::StructClassMap (ECN::ECClassCR ecClass, ECDbMapCR ecDbMap, ECDbMapStrategy mapStrategy, bool setIsDirty)
: ClassMap (ecClass, ecDbMap, mapStrategy, setIsDirty), m_embeddedTypeClassView (nullptr)
    {
    m_embeddedTypeClassView = std::unique_ptr<EmbeddedTypeClassMap> (new EmbeddedTypeClassMap (*this));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
ClassMap::Type StructClassMap::_GetClassMapType () const
    {
    return Type::SecondaryTable;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
MappingStatus StructClassMap::_OnInitialized ()
    {
    return m_embeddedTypeClassView->Initialize ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
IClassMap const& StructClassMap::_GetView (View classView) const
    {
    if (classView == IClassMap::View::EmbeddedType)
        return *m_embeddedTypeClassView;

    return ClassMap::_GetView (classView);
    }


//************************** SecondaryTableClassMap::EmbeddedClassMap *******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
MappingStatus StructClassMap::EmbeddedTypeClassMap::Initialize ()
    {
    auto const& schemaManager = GetECDbMap ().GetECDbR ().Schemas ();
    auto systemPropMap = PropertyMapStructArrayTableKey::Create (schemaManager, ECSqlSystemProperty::ECInstanceId, *this);
    m_embeddedClassViewPropMaps.AddPropertyMap (systemPropMap);

    systemPropMap = PropertyMapStructArrayTableKey::Create (schemaManager, ECSqlSystemProperty::ParentECInstanceId, *this);
    m_embeddedClassViewPropMaps.AddPropertyMap (systemPropMap);

    systemPropMap = PropertyMapStructArrayTableKey::Create (schemaManager, ECSqlSystemProperty::ECPropertyPathId, *this);
    m_embeddedClassViewPropMaps.AddPropertyMap (systemPropMap);

    systemPropMap = PropertyMapStructArrayTableKey::Create (schemaManager, ECSqlSystemProperty::ECArrayIndex, *this);
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

    return MappingStatus::Success;
    }

//************************** UnmappedClassMap ***************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
UnmappedClassMap::UnmappedClassMap (ECClassCR ecClass, ECDbMapCR ecdbMap, ECDbMapStrategy mapStrategy, bool setIsDirty)
: ClassMap (ecClass, ecdbMap, mapStrategy, setIsDirty)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
MappingStatus UnmappedClassMap::_MapPart1 (SchemaImportContext&, ClassMapInfo const& classMapInfo, IClassMap const* parentClassMap)
    {
    m_dbView = std::unique_ptr<ClassDbView> (new ClassDbView (*this));

    if (parentClassMap != nullptr)
        m_parentMapClassId = parentClassMap->GetParentMapClassId ();

    auto nullTable = GetECDbMap ().GetSQLManager ().GetNullTable ();
    SetTable (*const_cast<ECDbSqlTable*> (nullTable));

    return MappingStatus::Success;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
