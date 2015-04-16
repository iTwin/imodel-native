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
    BeAssert (ecClass.HasId () && "ClassMapFactory::Load is expected to look up the class id if not present.");
    auto classId = ecClass.GetId ();
    DbECClassMapInfo info;
    info.m_mapStrategy = MapStrategy::NoHint;
    info.m_mapParentECClassId = 0;
    info.ColsWhere = DbECClassMapInfo::COL_ECClassId;
    info.ColsSelect =
        DbECClassMapInfo::COL_MapStrategy |
        DbECClassMapInfo::COL_MapToDbTable |
        DbECClassMapInfo::COL_MapParentECClassId |
        DbECClassMapInfo::COL_ECInstanceIdColumn;
    // Key column(s)

    info.m_ecClassId = classId;

    CachedStatementPtr stmt = nullptr;
    ECDbSchemaPersistence::FindECClassMapInfo (stmt, ecdbMap.GetECDbR (), info);
    if (BE_SQLITE_ROW != ECDbSchemaPersistence::Step (info, *stmt))
        return nullptr;

    //Set map strategy
    auto const& schemaManager = ecdbMap.GetECDbR ().GetEC ().GetSchemaManager ();

    bool isMapInParent = (info.m_mapStrategy == MapStrategy::InParentTable);
    // Save Parent ECClassId if this class is mapped to parent
    IClassMap const* parentClassMap = nullptr;
    if (isMapInParent)
        {
        ECClassP mapParentECClass = nullptr;
        if (SUCCESS != schemaManager.GetECClass (mapParentECClass, info.m_mapParentECClassId))
            LOG.errorv (L"Fail to get parent class to which current class is mapped.");
        else
            parentClassMap = ecdbMap.GetClassMap (*mapParentECClass);
        }

    Utf8CP pk = (info.ColsNull & DbECClassMapInfo::COL_ECInstanceIdColumn) ? nullptr : info.m_primaryKeyColumnName.c_str ();

    ClassMapInfoPtr classMapInfo = ClassMapInfoFactory::CreateInstance (ecClass, ecdbMap, info.m_mapToDbTable.c_str (), pk, info.m_mapStrategy);

    //restore saved map setting.
    if (info.m_mapStrategy != MapStrategy::NoHint)
        classMapInfo->RestoreSaveSettings (info.m_mapStrategy, info.m_mapToDbTable.c_str());

    ECRelationshipClassCP ecRelationshipClass = ecClass.GetRelationshipClassCP ();
    // Construct and initialize the class map
    if (ecRelationshipClass != nullptr)
        {
        //Load Relation End ECClasses before creating instance of RelationshipECClass
        for (ECClassCP endECClassToLoad : ecdbMap.GetClassesFromRelationshipEnd (ecRelationshipClass->GetSource ()))
            {
            ecdbMap.GetClassMap (*endECClassToLoad);
            }

        for (ECClassCP endECClassToLoad : ecdbMap.GetClassesFromRelationshipEnd (ecRelationshipClass->GetTarget ()))
            {
            ecdbMap.GetClassMap (*endECClassToLoad);
            }
        }

    classMapInfo->SetParentClassMap (parentClassMap);

    auto classMap = CreateInstance (mapStatus, *classMapInfo, false);
    BeAssert (info.m_mapStrategy == classMapInfo->GetMapStrategy ());

    //now see if there are struct type properties
    classMap->GetPropertyMaps ().Traverse ([&ecdbMap, classId] (TraversalFeedback& feedback, PropertyMapCP propMap)
        {
        BeAssert (propMap != nullptr);
        auto propertyMapToTable = propMap->GetAsPropertyMapToTable ();
        if (propertyMapToTable == nullptr)
            return;

        ECClassCR elementType = propertyMapToTable->GetElementType ();
        //If the struct type happens to be the same as the one which is currently being loaded
        //don't try to load it again to prevent endless loop
        if (!elementType.HasId () || elementType.GetId () != classId)
            ecdbMap.GetClassMap (elementType);
        }, true);

    //This is hack solution. For SharedTables can have unrelated classes and FinishTableDefinition only add classed when there is more then one classes in a table.
    //TODO: come up with a better solution. 
    if (classMap->GetMapStrategy () == MapStrategy::SharedTableForThisClass &&
        classMap->GetTable ().GetClassIdColumn () == nullptr)
        {
        if (ecdbMap.GetECDbR ().ColumnExists (classMap->GetTable ().GetName (), ECDB_COL_ECClassId))
            classMap->GetTable ().GenerateClassIdColumn ();
        }

    return classMap;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    02/2014
//---------------------------------------------------------------------------------------
//static
ClassMapPtr ClassMapFactory::Create (MapStatus& mapStatus, SchemaImportContext const& schemaImportContext, ECN::ECClassCR ecClass, ECDbMapCR ecdbMap)
    {
    BeAssert (ecdbMap.GetClassMap (ecClass, false) == nullptr);

    auto classMapInfo = ClassMapInfoFactory::CreateFromHint (mapStatus, schemaImportContext, ecClass, ecdbMap);
    if (classMapInfo == nullptr)
        return nullptr;

    return ClassMapFactory::CreateInstance (mapStatus, *classMapInfo, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    02/2014
//---------------------------------------------------------------------------------------
ClassMapPtr ClassMapFactory::CreateInstance (MapStatus& mapStatus, ClassMapInfoCR mapInfo, bool setIsDirty)
    {
    auto const& ecClass = mapInfo.GetECClass ();
    auto const& ecdbMap = mapInfo.GetECDbMap ();
    const auto mapStrategy = mapInfo.GetMapStrategy ();
    BeAssert (mapStrategy != MapStrategy::NoHint);

    ClassMapPtr classMap = nullptr;
    if (IClassMap::IsDoNotMapStrategy (mapStrategy))
        classMap = UnmappedClassMap::Create (ecClass, ecdbMap, mapStrategy, setIsDirty);
    else
        {
        auto ecRelationshipClass = ecClass.GetRelationshipClassCP ();
        if (ecRelationshipClass != nullptr)
            {
            if (RelationshipClassMap::IsMapToRelationshipLinkTableStrategy (mapStrategy))
                classMap = RelationshipClassLinkTableMap::Create (*ecRelationshipClass, ecdbMap, mapStrategy, setIsDirty);
            else
                classMap = RelationshipClassEndTableMap::Create (*ecRelationshipClass, ecdbMap, mapStrategy, setIsDirty);
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
