/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMapFactory.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
    ECDbSchemaManager const& schemaManager = ecdbMap.GetECDbR().Schemas();
    std::vector<ECDbClassMapInfo const*> const* classMaps = ecdbMap.GetSQLManager().GetMapStorage().FindClassMapsByClassId (ecClass.GetId());
    if (classMaps == nullptr)
        return nullptr;

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

    ECDbClassMapInfo const& classMapInfo = *classMaps->front();
    ECDbClassMapInfo const* baseClassMapInfo = classMapInfo.GetBaseClassMap();
    ECClassCP baseClass = baseClassMapInfo == nullptr ? nullptr : schemaManager.GetECClass (baseClassMapInfo->GetClassId());
    IClassMap const* baseClassMap = baseClass == nullptr ? nullptr : ecdbMap.GetClassMap (*baseClass);

    bool setIsDirty = false;
    ECDbMapStrategy const& mapStrategy = classMapInfo.GetMapStrategy();
    ClassMapPtr classMap = nullptr;
    if (mapStrategy.IsNotMapped())
        classMap = UnmappedClassMap::Create (ecClass, ecdbMap, mapStrategy, setIsDirty);
    else
        {
        ECRelationshipClassCP ecRelationshipClass = ecClass.GetRelationshipClassCP();
        if (ecRelationshipClass != nullptr)
            {
            if (mapStrategy.IsForeignKeyMapping())
                classMap = RelationshipClassEndTableMap::Create(*ecRelationshipClass, ecdbMap, mapStrategy, setIsDirty);
            else
                classMap = RelationshipClassLinkTableMap::Create(*ecRelationshipClass, ecdbMap, mapStrategy, setIsDirty);
            }
        else if (IClassMap::MapsToStructArrayTable (ecClass))
            classMap = StructClassMap::Create (ecClass, ecdbMap, mapStrategy, setIsDirty);
        else
            classMap = ClassMap::Create (ecClass, ecdbMap, mapStrategy, setIsDirty);
        }
    classMap->SetId (classMapInfo.GetId());

    std::set<ClassMap const*> loadGraph;
    if (SUCCESS != classMap->Load (loadGraph, classMapInfo, baseClassMap))
        return nullptr;

    ECRelationshipClassCP ecRelationshipClass = ecClass.GetRelationshipClassCP();
    // Construct and initialize the class map
    if (ecRelationshipClass != nullptr)
        {
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
ClassMapPtr ClassMapFactory::Create(MapStatus& mapStatus, SchemaImportContext& schemaImportContext, ECN::ECClassCR ecClass, ECDbMapCR ecdbMap)
    {
    if (ecdbMap.AssertIfIsNotImportingSchema())
        {
        mapStatus = MapStatus::Error;
        return nullptr;
        }

    BeAssert(ecdbMap.GetClassMap(ecClass, false) == nullptr);

    std::unique_ptr<ClassMapInfo> classMapInfo = ClassMapInfoFactory::Create(mapStatus, schemaImportContext, ecClass, ecdbMap);
    if (classMapInfo == nullptr)
        return nullptr;

    ECDbMapStrategy const& mapStrategy = classMapInfo->GetMapStrategy();

    ClassMapPtr classMap = nullptr;
    if (mapStrategy.IsNotMapped())
        classMap = UnmappedClassMap::Create(ecClass, ecdbMap, mapStrategy, true);
    else
        {
        auto ecRelationshipClass = ecClass.GetRelationshipClassCP();
        if (ecRelationshipClass != nullptr)
            {
            if (mapStrategy.IsForeignKeyMapping())
                classMap = RelationshipClassEndTableMap::Create(*ecRelationshipClass, ecdbMap, mapStrategy, true);
            else
                classMap = RelationshipClassLinkTableMap::Create(*ecRelationshipClass, ecdbMap, mapStrategy, true);
            }
        else if (IClassMap::MapsToStructArrayTable(ecClass))
            classMap = StructClassMap::Create(ecClass, ecdbMap, mapStrategy, true);
        else
            classMap = ClassMap::Create(ecClass, ecdbMap, mapStrategy, true);
        }

    mapStatus = classMap->Map(schemaImportContext, *classMapInfo);
    schemaImportContext.CacheClassMapInfo(*classMap, classMapInfo);
    return classMap;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
