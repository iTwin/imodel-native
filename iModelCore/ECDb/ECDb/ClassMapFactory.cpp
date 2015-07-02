/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMapFactory.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ClassMapFactory.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//********************* ClassMapFactory ******************************************
//---------------------------------------------------------------------------------------
//* @bsimethod                                 Affan.Khan                           07 / 2012
//---------------------------------------------------------------------------------------
//static
ClassMapPtr ClassMapFactory::Load (MapStatus& mapStatus, ECClassCR ecClass, ECDbMapCR ecdbMap)
    {   
    auto const& schemaManager = ecdbMap.GetECDbR().Schemas();
    auto& mapStorage = ecdbMap.GetSQLManager().GetMapStorage();
    auto classMaps = mapStorage.FindClassMapsByClassId (ecClass.GetId());
    if (classMaps == nullptr)
        {
     //   BeAssert (false && "Failed to find classMap info for give ECClass");
        return nullptr;
        }

    if (classMaps->empty())
        {
        BeAssert (false && "Failed to find classMap info for give ECClass");
        return nullptr;
        }

    if (classMaps->size() > 1)
        {
        BeAssert (false && "Feature of nested class map not implemented");
        return nullptr;
        }

    auto& classMapInfo = *classMaps->front();
    auto baseClassMapInfo = classMapInfo.GetBaseClassMap();
    auto baseClass = baseClassMapInfo == nullptr ? nullptr : schemaManager.GetECClass (baseClassMapInfo->GetClassId());
    auto baseClassMap = baseClass == nullptr ? nullptr : ecdbMap.GetClassMap (*baseClass);

    bool setIsDirty = false;
    auto mapStrategy = classMapInfo.GetMapStrategy();
    ClassMapPtr classMap = nullptr;
    if (mapStrategy.IsNotMapped())
        classMap = UnmappedClassMap::Create (ecClass, ecdbMap, mapStrategy, setIsDirty);
    else
        {
        auto ecRelationshipClass = ecClass.GetRelationshipClassCP();
        if (ecRelationshipClass != nullptr)
            {
            if (mapStrategy.IsForeignKeyMapping())
                classMap = RelationshipClassEndTableMap::Create(*ecRelationshipClass, ecdbMap, mapStrategy, setIsDirty);
            else
                classMap = RelationshipClassLinkTableMap::Create(*ecRelationshipClass, ecdbMap, mapStrategy, setIsDirty);
            }
        else if (IClassMap::IsMapToSecondaryTableStrategy (ecClass))
            classMap = SecondaryTableClassMap::Create (ecClass, ecdbMap, mapStrategy, setIsDirty);
        else
            classMap = ClassMap::Create (ecClass, ecdbMap, mapStrategy, setIsDirty);
        }
    classMap->SetId (classMapInfo.GetId());

    std::set<ClassMap const*> loadGraph;
    auto stat = classMap->Load (loadGraph, classMapInfo, baseClassMap);
    if (stat != BentleyStatus::SUCCESS)
        {
        return nullptr;
        }

    ECRelationshipClassCP ecRelationshipClass = ecClass.GetRelationshipClassCP();
    // Construct and initialize the class map
    if (ecRelationshipClass != nullptr)
        {
        //Load Relation End ECClasses before creating instance of RelationshipECClass
        for (ECClassCP endECClassToLoad : ecdbMap.GetClassesFromRelationshipEnd (ecRelationshipClass->GetSource()))
            {
            ecdbMap.GetClassMap (*endECClassToLoad);
            }

        for (ECClassCP endECClassToLoad : ecdbMap.GetClassesFromRelationshipEnd (ecRelationshipClass->GetTarget()))
            {
            ecdbMap.GetClassMap (*endECClassToLoad);
            }
        }

    return classMap;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    02/2014
//---------------------------------------------------------------------------------------
//static
ClassMapPtr ClassMapFactory::Create (MapStatus& mapStatus, SchemaImportContext const& schemaImportContext, ECN::ECClassCR ecClass, ECDbMapCR ecdbMap)
    {
    if (ecdbMap.AssertIfNotMapping())
        {
        mapStatus = MapStatus::Error;
        return nullptr;
        }

    BeAssert (ecdbMap.GetClassMap (ecClass, false) == nullptr);

    auto classMapInfo = ClassMapInfoFactory::Create (mapStatus, schemaImportContext, ecClass, ecdbMap);
    if (classMapInfo == nullptr)
        return nullptr;

    return ClassMapFactory::CreateInstance (mapStatus, *classMapInfo, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    02/2014
//---------------------------------------------------------------------------------------
ClassMapPtr ClassMapFactory::CreateInstance (MapStatus& mapStatus, ClassMapInfo const& mapInfo, bool setIsDirty)
    {
    auto const& ecClass = mapInfo.GetECClass();
    auto const& ecdbMap = mapInfo.GetECDbMap();
    const auto mapStrategy = mapInfo.GetMapStrategy();
    BeAssert (mapStrategy.GetStrategy() != MapStrategy::None);

    ClassMapPtr classMap = nullptr;
    if (mapStrategy.IsNotMapped())
        classMap = UnmappedClassMap::Create (ecClass, ecdbMap, mapStrategy, setIsDirty);
    else
        {
        auto ecRelationshipClass = ecClass.GetRelationshipClassCP();
        if (ecRelationshipClass != nullptr)
            {
            if (mapStrategy.IsForeignKeyMapping())
                classMap = RelationshipClassEndTableMap::Create(*ecRelationshipClass, ecdbMap, mapStrategy, setIsDirty);
            else
                classMap = RelationshipClassLinkTableMap::Create(*ecRelationshipClass, ecdbMap, mapStrategy, setIsDirty);
            }
        else if (IClassMap::IsMapToSecondaryTableStrategy (ecClass))
            classMap = SecondaryTableClassMap::Create (ecClass, ecdbMap, mapStrategy, setIsDirty);
        else
            classMap = ClassMap::Create (ecClass, ecdbMap, mapStrategy, setIsDirty);
        }

    mapStatus = classMap->Initialize (mapInfo);
    return classMap;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
