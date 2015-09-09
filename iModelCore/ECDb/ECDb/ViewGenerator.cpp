/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ViewGenerator.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECDbPolicyManager.h"
#include <set>
#include "ECSql/ECSqlPrepareContext.h"
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
#define ECDB_HOLDING_VIEW "ec_RelationshipHoldingStatistics"
        
//************************** ViewGenerator ***************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    10/2013
//---------------------------------------------------------------------------------------
//static
Utf8CP const ViewGenerator::ECCLASSID_COLUMNNAME = "ECClassId";

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ViewGenerator::CreateView (NativeSqlBuilder& viewSql, ECDbMapCR map, IClassMap const& classMap, bool isPolymorphicQuery,  ECSqlPrepareContext const& prepareContext, bool optimizeByIncludingOnlyRealTables)
    {
    //isPolymorphic is not implemented. By default all query are polymorphic
    if (classMap.IsRelationshipClassMap ())
        return CreateViewForRelationship (viewSql, map, prepareContext, classMap, isPolymorphicQuery, optimizeByIncludingOnlyRealTables);

    viewSql.AppendParenLeft ();
    auto& db = map.GetECDbR();
    auto objectType = DbMetaDataHelper::GetObjectType(db, classMap.GetTable().GetName().c_str());   
    if (objectType == DbMetaDataHelper::ObjectType::None && !isPolymorphicQuery)
        {
        auto status = CreateNullView (viewSql, prepareContext, classMap);
        if (status != BentleyStatus::SUCCESS)
            return status;

        viewSql.AppendParenRight ();
        return BentleyStatus::SUCCESS;
        }

    ViewMemberByTable viewMembers;
    BentleyStatus status;
    if (ClassMap::IsAnyClass (classMap.GetClass ()))
        {
        PRECONDITION (isPolymorphicQuery && "This operation require require polymorphic query to be enabled", BentleyStatus::ERROR);

        std::vector<IClassMap const*> rootClassMaps;
        status = GetRootClasses(rootClassMaps, map.GetECDbR());
        if (status != BentleyStatus::SUCCESS)
            return status;

        for (auto classMap : rootClassMaps)
            {
            status = ComputeViewMembers (viewMembers, map, classMap->GetClass (), isPolymorphicQuery, optimizeByIncludingOnlyRealTables, /*ensureDerivedClassesAreLoaded=*/ false);
            if (status != BentleyStatus::SUCCESS)
                return status;
            }//a.Add(ecClassId, tables, 
        }
    else
        {
        status = ComputeViewMembers (viewMembers,  map, classMap.GetClass(), isPolymorphicQuery, optimizeByIncludingOnlyRealTables, /*ensureDerivedClassesAreLoaded=*/ true);
        }

    if (status == BentleyStatus::SUCCESS)
        {
        int queriesAddedToUnion = 0;
        for (auto& pvm : viewMembers)
            {
            if (optimizeByIncludingOnlyRealTables)
                if (pvm.second.GetStorageType () == DbMetaDataHelper::ObjectType::None)
                    continue;

            if (queriesAddedToUnion > 0)
                viewSql.Append (" UNION ");

            status = GetViewQueryForChild (viewSql, map, prepareContext, *pvm.first, pvm.second.GetClassMaps (), classMap, isPolymorphicQuery);
            if (status != BentleyStatus::SUCCESS)
                return status;

            queriesAddedToUnion++;
            }

        if (queriesAddedToUnion == 0)
            {
            auto status = CreateNullView (viewSql, prepareContext, classMap);
            if (status != BentleyStatus::SUCCESS)
                return status;

            viewSql.AppendParenRight ();
            return BentleyStatus::SUCCESS;
            }
        else
            viewSql.AppendParenRight ();
        }

    return status;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ViewGenerator::CreateNullView (NativeSqlBuilder& viewSql, ECSqlPrepareContext const& prepareContext, IClassMap const& classMap)
    {
    if (classMap.IsMappedToSecondaryTable ())
        viewSql.Append ("SELECT NULL ECClassId, NULL " ECDB_COL_ECInstanceId ", NULL ParentECInstanceId, NULL ECPropertyPathId, NULL ECArrayIndex");
    else
        viewSql.Append ("SELECT NULL ECClassId, NULL " ECDB_COL_ECInstanceId);

    std::vector<std::pair<PropertyMapCP, PropertyMapCP>> viewPropMaps;
    auto status = GetPropertyMapsOfDerivedClassCastAsBaseClass (viewPropMaps, prepareContext, classMap, classMap, false, false);
    if (status != SUCCESS)
        return status;

 
    AppendViewPropMapsToQuery (viewSql, classMap.GetECDbMap ().GetECDbR (), prepareContext, classMap.GetTable (), viewPropMaps, true /*forNullView*/);
 
    viewSql.Append (" LIMIT 0");
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ViewGenerator::GetRootClasses (std::vector<IClassMap const*>& rootClasses, ECDbR db)
    {
    ECSchemaList schemas;
    if (db.Schemas ().GetECSchemas (schemas, true) != SUCCESS)
        return ERROR;
        
    std::vector<IClassMap const*> rootClassMaps;
    for (auto schema : schemas)
        {
        if (schema->IsStandardSchema())
            continue;

        for(auto ecClass : schema->GetClasses())
            {
            if (ecClass->GetDerivedClasses().empty())
                {
                auto classMap = db.GetECDbImplR().GetECDbMap ().GetClassMap (*ecClass);
                BeAssert (classMap != nullptr);
                if (!ECDbPolicyManager::GetClassPolicy (*classMap, IsValidInECSqlPolicyAssertion::Get ()).IsSupported ())
                    continue;

                rootClassMaps.push_back(classMap);
                }
            }
        }
    return BentleyStatus::SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus  ViewGenerator::ComputeViewMembers 
(
ViewMemberByTable& viewMembers, 
ECDbMapCR map,
ECClassCR ecClass, 
bool isPolymorphic, 
bool optimizeByIncludingOnlyRealTables, 
bool ensureDerivedClassesAreLoaded
)
    {
    auto classMap = map.GetClassMap (ecClass);
    if (classMap == nullptr)
        return SUCCESS;

    if (!classMap->IsRelationshipClassMap ())
        {
        if (!ECDbPolicyManager::GetClassPolicy (*classMap, IsValidInECSqlPolicyAssertion::Get ()).IsSupported ())
            //ECSQL_WIP Not supported yet
            return BentleyStatus::SUCCESS;
        }
        
    if (!classMap->GetMapStrategy ().IsNotMapped ())
        {
        if (classMap->GetTable ().GetColumns ().empty ())
            return BentleyStatus::SUCCESS;

        auto itor = viewMembers.find (&classMap->GetTable ());
        if (itor == viewMembers.end ())
            {
            DbMetaDataHelper::ObjectType storageType = DbMetaDataHelper::ObjectType::Table;
            if (optimizeByIncludingOnlyRealTables)
                {
                //This is a db query so optimization comes at a cost
                storageType = DbMetaDataHelper::GetObjectType (map.GetECDbR (), classMap->GetTable ().GetName ().c_str ());
                }
            viewMembers.insert (
                ViewMemberByTable::value_type (&classMap->GetTable (), ViewMember (storageType, *classMap)));
            }
        else
            {
            if (optimizeByIncludingOnlyRealTables)
                {
                if (itor->second.GetStorageType () == DbMetaDataHelper::ObjectType::Table)
                    itor->second.GetClassMaps ().push_back (classMap);
                }
            else
                itor->second.GetClassMaps ().push_back (classMap);
            }
        }

    if (isPolymorphic)
        {
        auto const& derivedClasses = ensureDerivedClassesAreLoaded ? map.GetECDbR ().Schemas ().GetDerivedECClasses (ecClass) : ecClass.GetDerivedClasses ();
        for (auto derivedClass : derivedClasses)
            {
            auto status = ComputeViewMembers (viewMembers, map, *derivedClass, isPolymorphic, optimizeByIncludingOnlyRealTables, ensureDerivedClassesAreLoaded);
            if (status != BentleyStatus::SUCCESS)
                return status;
            }
        }
    return BentleyStatus::SUCCESS;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::GetPropertyMapsOfDerivedClassCastAsBaseClass (std::vector<std::pair<PropertyMapCP, PropertyMapCP>>& propMaps, ECSqlPrepareContext const& prepareContext, IClassMap const& baseClassMap, IClassMap const& childClassMap, bool skipSystemProperties, bool embededStatement)
    {
    propMaps.clear ();    
   
    auto& parentMap = embededStatement ? baseClassMap.GetView (IClassMap::View::EmbeddedType)
        : baseClassMap;


    auto& childMap = embededStatement ? childClassMap.GetView (IClassMap::View::EmbeddedType)
        : childClassMap;



    for (auto baseClassPropertyMap : parentMap.GetPropertyMaps ())
        {
        if ((skipSystemProperties && baseClassPropertyMap->IsSystemPropertyMap ()) ||
            baseClassPropertyMap->GetAsPropertyMapToTable ())
            continue;

        auto accessString = baseClassPropertyMap->GetPropertyAccessString ();
        auto childClassCounterpartPropMap = childMap.GetPropertyMap (accessString);
        if (childClassCounterpartPropMap == nullptr)
            return ERROR;

        std::vector<ECDbSqlColumn const*> baseClassPropMapColumns;
        std::vector<ECDbSqlColumn const*> childClassPropMapColumns;
        baseClassPropertyMap->GetColumns (baseClassPropMapColumns);
        childClassCounterpartPropMap->GetColumns (childClassPropMapColumns);
        if (baseClassPropMapColumns.size () != childClassPropMapColumns.size ())
            return ERROR;

        propMaps.push_back ({baseClassPropertyMap, childClassCounterpartPropMap});
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::AppendViewPropMapsToQuery (NativeSqlBuilder& viewQuery, ECDbR ecdb, ECSqlPrepareContext const& prepareContext, ECDbSqlTable const& table, std::vector<std::pair<PropertyMapCP, PropertyMapCP>> const& viewPropMaps, bool forNullView)
    {
    for (auto const& propMapPair : viewPropMaps)
        {
        auto basePropMap = propMapPair.first;
        auto actualPropMap = propMapPair.second;
        if (!prepareContext.GetSelectionOptions ().IsSelected (actualPropMap->GetPropertyAccessString()))
            continue;

        auto aliasSqlSnippets = basePropMap->ToNativeSql(nullptr, ECSqlType::Select, false);
        auto colSqlSnippets = actualPropMap->ToNativeSql(nullptr, ECSqlType::Select, false);

        const size_t snippetCount = colSqlSnippets.size ();
        if (aliasSqlSnippets.size () != snippetCount)
            {
            BeAssert (false && "Number of alias SQL snippets is expected to be the same as number of column SQL snippets.");
            return ERROR;
            }

        for (size_t i = 0; i < snippetCount; i++)
            {
            viewQuery.AppendComma (true);
            auto const& aliasSqlSnippet = aliasSqlSnippets[i];
            if (forNullView)
                viewQuery.Append ("NULL ");
            else
                viewQuery.Append (colSqlSnippets[i]).AppendSpace ();
            
            viewQuery.Append (aliasSqlSnippet);
            }        
        }

    return SUCCESS;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      07/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::GetViewQueryForChild (NativeSqlBuilder& viewSql, ECDbMapCR map, ECSqlPrepareContext const& prepareContext, ECDbSqlTable const& table, const std::vector<IClassMap const*>& childClassMap, IClassMap const& baseClassMap, bool isPolymorphic)
    {
    PRECONDITION(!childClassMap.empty(), BentleyStatus::ERROR);  
    PRECONDITION(!table.GetColumns().empty (), BentleyStatus::ERROR);  

    std::vector<ECClassId> classesMappedToTable;

    if (ECDbSchemaPersistence::GetClassesMappedToTable (classesMappedToTable, table, true, map.GetECDbR ()) != SUCCESS)
        return ERROR;
 
    bool oneToManyMapping = classesMappedToTable.size() > 1;

    auto firstChildClassMap = *childClassMap.begin ();

    //Generate Select statement
    viewSql.Append ("SELECT ");

    auto classIdColumn = table.FindColumnCP (ECDB_COL_ECClassId);
    if (classIdColumn != nullptr)
        viewSql.Append (classIdColumn->GetName ().c_str());
    else
        viewSql.Append (firstChildClassMap->GetClass ().GetId ()).AppendSpace ().Append (ECCLASSID_COLUMNNAME);

    std::vector<std::pair<PropertyMapCP, PropertyMapCP>> viewPropMaps;
    
    //auto skipSystemProperties = structArrayProperty == nullptr;
    auto isEmbeded = prepareContext.GetParentArrayProperty () != nullptr;
    auto status = GetPropertyMapsOfDerivedClassCastAsBaseClass (viewPropMaps, prepareContext, baseClassMap, *firstChildClassMap, false, isEmbeded);
    if (status != BentleyStatus::SUCCESS)
        return status;

    //Append prop map columns to query [col1],[col2], ...
    AppendViewPropMapsToQuery (viewSql, map.GetECDbR (), prepareContext, table, viewPropMaps);

    viewSql.Append (" FROM ").AppendEscaped (table.GetName().c_str());

    NativeSqlBuilder where;
    if (classIdColumn != nullptr)
        {
        if (oneToManyMapping)
            {
            if (isPolymorphic)
                {
                std::set<ECClassId> inConstraintCIDs;
                for (auto classMap : childClassMap)
                    inConstraintCIDs.insert (classMap->GetClass ().GetId ());

                std::vector<ECClassId> notInConstraintCIDs;
                for (auto classId : classesMappedToTable)
                    {
                    if (inConstraintCIDs.find (classId) == inConstraintCIDs.end ())
                        notInConstraintCIDs.push_back (classId);
                    }

                //Here we want to create minimum size IN() query. So we will either exclude or include base on which one is minimum
                if (notInConstraintCIDs.size () > inConstraintCIDs.size ()) // include class ids of class we do want.
                    {
                    if (!inConstraintCIDs.empty ())
                        {
                        where.AppendParenLeft ().AppendEscaped (table.GetName ().c_str ()).AppendDot ().AppendEscaped (classIdColumn->GetName ().c_str ()).Append (" IN (");
                        bool isFirstItem = true;
                        for (auto& classMap : childClassMap)
                            {
                            if (!isFirstItem)
                            where.AppendComma (true);

                            where.Append (classMap->GetClass ().GetId ());

                            isFirstItem = false;
                            }
                        where.AppendParenRight ().AppendParenRight ();
                        }
                    }
                else //exclude class ids of class we don't want. 
                    {
                    if (!notInConstraintCIDs.empty ())
                        {
                        where.AppendParenLeft ().AppendEscaped (table.GetName ().c_str ()).AppendDot ().AppendEscaped (classIdColumn->GetName ().c_str ()).Append (" NOT IN (");
                        bool isFirstItem = true;
                        for (auto classId : notInConstraintCIDs)
                            {
                            if (!isFirstItem)
                            where.AppendComma (true);

                            where.Append (classId);

                            isFirstItem = false;
                            }
                        where.AppendParenRight ().AppendParenRight ();
                        }
                    }
                }
            else
            where.AppendParenLeft ().AppendEscaped (table.GetName ().c_str ()).AppendDot ().AppendEscaped (classIdColumn->GetName ().c_str ()).Append (" = ").Append (firstChildClassMap->GetClass ().GetId ()).AppendParenRight ();
            }
        }
    //We allow query of struct classes.
    if (firstChildClassMap->IsMappedToSecondaryTable ())
        {
        if (!where.IsEmpty ())
        where.Append (" AND ");

        if (prepareContext.GetParentArrayProperty() == nullptr)
            where.Append ("(ECPropertyPathId IS NULL AND ECArrayIndex IS NULL)");
        }

    if (!where.IsEmpty())
        viewSql.Append (" WHERE ").Append (where);
    
    return BentleyStatus::SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
    BentleyStatus ViewGenerator::CreateNullViewForRelationshipClassEndTableMap (NativeSqlBuilder& viewSql, ECSqlPrepareContext const& prepareContext, RelationshipClassMapCR relationMap, IClassMap const& baseClassMap)
    {
    AppendSystemPropMapsToNullView (viewSql, prepareContext, relationMap, false /*endWithComma*/);

    viewSql.Append (" LIMIT 0");

    return BentleyStatus::SUCCESS;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateNullViewForRelationshipClassLinkTableMap (NativeSqlBuilder& viewSql, ECSqlPrepareContext const& prepareContext, RelationshipClassMapCR relationMap, IClassMap const& baseClassMap)
    {
    AppendSystemPropMapsToNullView (viewSql, prepareContext, relationMap, false /*endWithComma*/);

    //! Only link table mapped relationship properties are persisted
    std::vector<std::pair<PropertyMapCP, PropertyMapCP>> viewPropMaps;
    auto status = GetPropertyMapsOfDerivedClassCastAsBaseClass (viewPropMaps, prepareContext, baseClassMap, relationMap, true, false);
    if (status != BentleyStatus::SUCCESS)
        return status;

    //Append columns to query [col1],[col2], ...
    AppendViewPropMapsToQuery (viewSql, relationMap.GetECDbMap ().GetECDbR (), prepareContext, relationMap.GetTable (), viewPropMaps, true);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
    BentleyStatus ViewGenerator::GetAllChildRelationships (std::vector<RelationshipClassMapCP>& relationshipMaps, ECDbMapCR map, ECSqlPrepareContext const& prepareContext, IClassMap const& baseRelationMap)
    {
    auto& baseClass = baseRelationMap.GetClass();
    for (auto childClass : map.GetECDbR ().Schemas ().GetDerivedECClasses (const_cast<ECClassR>(baseClass)))
        {
        auto childClassMap = map.GetClassMap (*childClass);
        if (childClassMap == nullptr)
            continue;

        auto policy = ECDbPolicyManager::GetClassPolicy (*childClassMap, IsValidInECSqlPolicyAssertion::Get ());
        if (policy.IsSupported ())
            {
            if (!childClassMap->IsRelationshipClassMap ())
                {
                LOG.errorv ("ECDb does not support relationships that have derived non-relationship classes.");
                BeAssert (childClassMap->IsRelationshipClassMap ());
                return ERROR;
                }

            relationshipMaps.push_back (static_cast<RelationshipClassMapCP>(childClassMap));
            }

        auto status = GetAllChildRelationships (relationshipMaps, map, prepareContext, *childClassMap);
        if (status != BentleyStatus::SUCCESS)
            return status;
        }

    return BentleyStatus::SUCCESS;  
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateViewForRelationshipClassLinkTableMap (NativeSqlBuilder& viewSql, ECDbMapCR ecdbMap, ECSqlPrepareContext const& prepareContext, RelationshipClassMapCR relationMap, IClassMap const& baseClassMap)
    {
    viewSql.Append ("SELECT ");
    AppendSystemPropMaps (viewSql, ecdbMap, prepareContext, relationMap);

    //! Only link table mapped relationship properties are persisted
    std::vector<std::pair<PropertyMapCP, PropertyMapCP>> viewPropMaps;
    auto status = GetPropertyMapsOfDerivedClassCastAsBaseClass (viewPropMaps, prepareContext, baseClassMap, relationMap, true, false);
    if (status != SUCCESS)
        return status;

    //Append prop maps' columns to query [col1],[col2], ...
    AppendViewPropMapsToQuery (viewSql, ecdbMap.GetECDbR (), prepareContext, relationMap.GetTable (), viewPropMaps);

    viewSql.Append (" FROM ").AppendEscaped (relationMap.GetTable ().GetName ().c_str ());
   
    //Append secondary table JOIN
    auto const secondaryTables = relationMap.GetSecondaryTables ();
    auto primaryTable = &relationMap.GetTable ();
    auto primaryECInstanceIdColumn = primaryTable->GetFilteredColumnFirst (ECDbKnownColumns::ECInstanceId);
    BeAssert (primaryECInstanceIdColumn != nullptr);
    if (BuildRelationshipJoinIfAny (viewSql, relationMap, ECN::ECRelationshipEnd::ECRelationshipEnd_Source) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;

    if (BuildRelationshipJoinIfAny (viewSql, relationMap, ECN::ECRelationshipEnd::ECRelationshipEnd_Target) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateViewForRelationshipClassEndTableMap (NativeSqlBuilder& viewSql, ECDbMapCR ecdbMap, ECSqlPrepareContext const& prepareContext, RelationshipClassEndTableMapCR relationMap, IClassMap const& baseClassMap)
    {
    //ECInstanceId, ECClassId of the relationship instance
    viewSql.Append ("SELECT ");
    AppendSystemPropMaps (viewSql, ecdbMap, prepareContext,  relationMap);
    viewSql.Append (" FROM ").AppendEscaped (relationMap.GetTable ().GetName ().c_str ());

    //Append secondary table JOIN
    auto const secondaryTables = relationMap.GetSecondaryTables ();
    auto primaryTable = &relationMap.GetTable ();
    auto primaryECInstanceIdColumn = primaryTable->GetFilteredColumnFirst (ECDbKnownColumns::ECInstanceId);
    BeAssert (primaryECInstanceIdColumn != nullptr);

    if (BuildRelationshipJoinIfAny (viewSql, relationMap, ECN::ECRelationshipEnd::ECRelationshipEnd_Source) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;

    if (BuildRelationshipJoinIfAny (viewSql, relationMap, ECN::ECRelationshipEnd::ECRelationshipEnd_Target) != BentleyStatus::SUCCESS)
    return BentleyStatus::ERROR;


    viewSql.Append (" WHERE ").Append (relationMap.GetOtherEndECInstanceIdPropMap ()->ToNativeSql (nullptr, ECSqlType::Select, false)).Append (" IS NOT NULL");
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      07/2015
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::BuildRelationshipJoinIfAny (NativeSqlBuilder& sqlBuilder, RelationshipClassMapCR classMap, ECN::ECRelationshipEnd endPoint)
    {
    auto ecclassIdPropertyMap = static_cast<PropertyMapRelationshipConstraintClassId const*>(endPoint == ECRelationshipEnd::ECRelationshipEnd_Source ? classMap.GetSourceECClassIdPropMap () : classMap.GetTargetECClassIdPropMap ());
    if (!ecclassIdPropertyMap->IsMappedToPrimaryTable ())
        {
        auto ecInstanceIdPropertyMap = static_cast<PropertyMapRelationshipConstraintECInstanceId const*>(endPoint == ECRelationshipEnd::ECRelationshipEnd_Source ? classMap.GetSourceECInstanceIdPropMap () : classMap.GetTargetECInstanceIdPropMap ());
        auto const& targetTable = ecclassIdPropertyMap->GetFirstColumn ()->GetTable ();

        sqlBuilder.Append (" INNER JOIN ");
        sqlBuilder.AppendEscaped (targetTable.GetName ().c_str ());
        sqlBuilder.AppendSpace ();
        sqlBuilder.Append (GetECClassIdPrimaryTableAlias (endPoint));
        sqlBuilder.Append (" ON ");
        sqlBuilder.Append (GetECClassIdPrimaryTableAlias (endPoint));
        sqlBuilder.AppendDot ();
        auto targetECInstanceIdColumn = targetTable.GetFilteredColumnFirst (ECDbKnownColumns::ECInstanceId);
        if (targetECInstanceIdColumn == nullptr)
            {
            BeAssert (false && "Failed to find ECInstanceId column in target table");
            return BentleyStatus::ERROR;
            }
        sqlBuilder.AppendEscaped (targetECInstanceIdColumn->GetName ().c_str ());
        sqlBuilder.Append (" = ");
        sqlBuilder.AppendEscaped (ecInstanceIdPropertyMap->GetFirstColumn ()->GetTable ().GetName ().c_str ());

        sqlBuilder.AppendDot ();
        sqlBuilder.Append (ecInstanceIdPropertyMap->GetFirstColumn ()->GetName ().c_str ());
        sqlBuilder.AppendSpace ();
        }

    return BentleyStatus::SUCCESS;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
    BentleyStatus ViewGenerator::CreateViewForRelationship (NativeSqlBuilder& viewSql, ECDbMapCR map, ECSqlPrepareContext const& prepareContext, IClassMap const& relationMap, IClassMap const& baseClassMap)
    {
    switch(relationMap.GetClassMapType())
        {
        case ClassMap::Type::RelationshipEndTable:
            return CreateViewForRelationshipClassEndTableMap (viewSql, map, prepareContext, static_cast<RelationshipClassEndTableMapCR>(relationMap), baseClassMap);
        case ClassMap::Type::RelationshipLinkTable:
            return CreateViewForRelationshipClassLinkTableMap (viewSql, map, prepareContext, static_cast<RelationshipClassMapCR>(relationMap), baseClassMap);
        }

    return ERROR;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
    BentleyStatus ViewGenerator::CreateNullViewForRelationship (NativeSqlBuilder& viewSql, ECDbMapCR map, ECSqlPrepareContext const& prepareContext, IClassMap const& relationMap, IClassMap const& baseClassMap)
    {
    switch (relationMap.GetClassMapType ())
        {
        case ClassMap::Type::RelationshipEndTable:
            return CreateNullViewForRelationshipClassEndTableMap (viewSql, prepareContext, static_cast<RelationshipClassMapCR>(relationMap), baseClassMap);
        case ClassMap::Type::RelationshipLinkTable:
            return CreateNullViewForRelationshipClassLinkTableMap (viewSql, prepareContext, static_cast<RelationshipClassMapCR>(relationMap), baseClassMap);
        default:
            return ERROR;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2015
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateViewForRelationship (NativeSqlBuilder& viewSql, ECDbMapCR map, ECSqlPrepareContext const& prepareContext, IClassMap const& relationMap, bool isPolymorphic, bool optimizeByIncludingOnlyRealTables)
    {
    BeAssert (relationMap.IsRelationshipClassMap ());
    BentleyStatus status = BentleyStatus::SUCCESS;

    if (relationMap.GetMapStrategy().IsNotMapped ())
        return BentleyStatus::ERROR;

    ViewMemberByTable vmt;
    status = ComputeViewMembers (vmt, map, relationMap.GetClass (), isPolymorphic, optimizeByIncludingOnlyRealTables, true);
    if (status != BentleyStatus::SUCCESS)
        return status;
    if (vmt.empty ())
        {
        return CreateNullViewForRelationship (viewSql, map, prepareContext, relationMap, relationMap);
        }
     ViewMember viewMember = vmt[&relationMap.GetTable()];
     NativeSqlBuilder unionQuery;

     for (auto cm : viewMember.GetClassMaps())
         {
         switch (cm->GetClassMapType())
             {
             case ClassMap::Type::RelationshipEndTable:
                 if (!unionQuery.IsEmpty())
                     unionQuery.Append(" UNION ");
                 status = CreateViewForRelationshipClassEndTableMap(unionQuery, map, prepareContext, *static_cast<RelationshipClassEndTableMapCP>(cm), relationMap);
                 if (status != BentleyStatus::SUCCESS)
                     return status;
                     break;
             case ClassMap::Type::RelationshipLinkTable:
             {
                 if (!unionQuery.IsEmpty())
                     unionQuery.Append(" UNION ");

                 status = CreateViewForRelationshipClassLinkTableMap(unionQuery, map, prepareContext, *static_cast<RelationshipClassLinkTableMapCP>(cm), relationMap);
                 if (status != BentleyStatus::SUCCESS)
                     return status;

                 auto column = relationMap.GetTable().GetFilteredColumnFirst(ECDbKnownColumns::ECClassId);
                 if (column != nullptr)
                     {

                     std::vector<ECClassId> classesMappedToTable;
                     if (ECDbSchemaPersistence::GetClassesMappedToTable(classesMappedToTable, relationMap.GetTable(), false, map.GetECDbR()) != SUCCESS)
                         return ERROR;

                     if (classesMappedToTable.size() != 1)
                         {
                         unionQuery.Append(" WHERE ");
                         unionQuery.AppendEscaped (relationMap.GetTable ().GetName ().c_str ()).AppendDot ().AppendEscaped (column->GetName ().c_str ()).Append (" IN ");
                         unionQuery.AppendParenLeft();
                         
                             unionQuery.Append(cm->GetClass().GetId());
                             
                         unionQuery.AppendParenRight();
                         }
                     }
                }
             }
         }
     vmt.erase(&relationMap.GetTable());
    for (auto& vm : vmt)
        {
        auto table = vm.first;
        if (vm.second.GetStorageType () != DbMetaDataHelper::ObjectType::Table)
            continue;



        std::vector<RelationshipClassEndTableMapCP> etm;
        std::vector<RelationshipClassLinkTableMapCP> ltm;
        for (auto cm : vm.second.GetClassMaps ())
            {
            switch (cm->GetClassMapType ())
                {
                case ClassMap::Type::RelationshipEndTable:
                    etm.push_back (static_cast<RelationshipClassEndTableMapCP>(cm)); break;
                case ClassMap::Type::RelationshipLinkTable:
                    ltm.push_back (static_cast<RelationshipClassLinkTableMapCP>(cm)); break;
                }
            }

        if (!ltm.empty ())
            {
            if (!unionQuery.IsEmpty ())
                unionQuery.Append (" UNION ");

            status = CreateViewForRelationshipClassLinkTableMap (unionQuery, map, prepareContext, *ltm.front (), relationMap);
            if (status != SUCCESS)
                return status;

            auto column = table->GetFilteredColumnFirst (ECDbKnownColumns::ECClassId);
            if (column != nullptr)
                {

                std::vector<ECClassId> classesMappedToTable;
                if (ECDbSchemaPersistence::GetClassesMappedToTable (classesMappedToTable, *table, false, map.GetECDbR ()) != SUCCESS)
                    return ERROR;

                if (classesMappedToTable.size () != ltm.size ())
                    {
                    unionQuery.Append (" WHERE ");
                    unionQuery.AppendEscaped(table->GetName().c_str()).AppendDot().AppendEscaped (column->GetName ().c_str ()).Append (" IN ");
                    unionQuery.AppendParenLeft ();
                    for (auto lt : ltm)
                        {
                        unionQuery.Append (lt->GetClass ().GetId ());
                        if (ltm.back () != lt)
                            {
                            unionQuery.AppendComma ();
                            }
                        }
                    unionQuery.AppendParenRight ();
                    }
                }
            }

        for (auto et : etm)
            {
            if (!unionQuery.IsEmpty ())
                unionQuery.Append (" UNION ");

            status = CreateViewForRelationshipClassEndTableMap (unionQuery, map, prepareContext, *et, relationMap);
            if (status != BentleyStatus::SUCCESS)
                return status;
            }
        }

    viewSql.AppendParenLeft ().Append (unionQuery.ToString ()).AppendParenRight ();
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
//static
BentleyStatus ViewGenerator::AppendSystemPropMaps (NativeSqlBuilder& viewSql, ECDbMapCR ecdbMap, ECSqlPrepareContext const& prepareContext, RelationshipClassMapCR relationMap)
    {
    auto ecId = relationMap.GetECInstanceIdPropertyMap ();
    if (!ecId->IsVirtual ())
        {
        viewSql.AppendEscaped (ecId->GetFirstColumn ()->GetTable ().GetName ().c_str ()).AppendDot ();
        }

    viewSql.Append (relationMap.GetECInstanceIdPropertyMap ()->ToNativeSql ( nullptr, ECSqlType::Select, false)).AppendComma (true);
    viewSql.Append (relationMap.GetClass ().GetId ()).AppendSpace ().Append (ECCLASSID_COLUMNNAME).AppendComma (true);

    //Source
    BeAssert (dynamic_cast<PropertyMapRelationshipConstraint const*> (relationMap.GetSourceECInstanceIdPropMap ()) != nullptr);
    auto propMap = static_cast<PropertyMapRelationshipConstraint const*> (relationMap.GetSourceECInstanceIdPropMap ());
    if (!propMap->IsVirtual ())
        {
        viewSql.AppendEscaped (propMap->GetFirstColumn ()->GetTable ().GetName ().c_str ()).AppendDot ();
        }

    propMap->AppendSelectClauseSqlSnippetForView (viewSql);
    viewSql.AppendComma (true);

    BeAssert (dynamic_cast<PropertyMapRelationshipConstraint const*> (relationMap.GetSourceECClassIdPropMap ()) != nullptr);
    propMap = static_cast<PropertyMapRelationshipConstraint const*> (relationMap.GetSourceECClassIdPropMap ());
    if (!propMap->IsVirtual ())
        {
        if (!propMap->IsMappedToPrimaryTable ())
            viewSql.AppendEscaped (GetECClassIdPrimaryTableAlias (ECRelationshipEnd::ECRelationshipEnd_Source)).AppendDot ();
        else
            viewSql.AppendEscaped (propMap->GetFirstColumn ()->GetTable ().GetName ().c_str ()).AppendDot ();
        }

    AppendConstraintClassIdPropMap (viewSql, prepareContext, *propMap, ecdbMap, relationMap, relationMap.GetRelationshipClass ().GetSource ());
    viewSql.AppendComma (true);

    //Target
    BeAssert (dynamic_cast<PropertyMapRelationshipConstraint const*> (relationMap.GetTargetECInstanceIdPropMap ()) != nullptr);
    propMap = static_cast<PropertyMapRelationshipConstraint const*> (relationMap.GetTargetECInstanceIdPropMap ());
    if (!propMap->IsVirtual ())
        {
        viewSql.AppendEscaped (propMap->GetFirstColumn ()->GetTable ().GetName ().c_str ()).AppendDot ();
        }

    propMap->AppendSelectClauseSqlSnippetForView (viewSql);
    viewSql.AppendComma (true);

    BeAssert (dynamic_cast<PropertyMapRelationshipConstraint const*> (relationMap.GetTargetECClassIdPropMap ()) != nullptr);
    propMap = static_cast<PropertyMapRelationshipConstraint const*> (relationMap.GetTargetECClassIdPropMap ());
    if (!propMap->IsVirtual ())
        {
        if (!propMap->IsMappedToPrimaryTable ())
            viewSql.AppendEscaped (GetECClassIdPrimaryTableAlias (ECRelationshipEnd::ECRelationshipEnd_Target)).AppendDot ();
        else
            viewSql.AppendEscaped (propMap->GetFirstColumn ()->GetTable ().GetName ().c_str ()).AppendDot ();
        }

    AppendConstraintClassIdPropMap (viewSql, prepareContext, *propMap, ecdbMap, relationMap, relationMap.GetRelationshipClass ().GetTarget ());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
//static
    BentleyStatus ViewGenerator::AppendSystemPropMapsToNullView (NativeSqlBuilder& viewSql, ECSqlPrepareContext const& prepareContext, RelationshipClassMapCR relationMap, bool endWithComma)
    {
    //ECInstanceId and ECClassId
    auto sqlSnippets = relationMap.GetECInstanceIdPropertyMap ()->ToNativeSql (nullptr, ECSqlType::Select, false);
    if (sqlSnippets.size () != 1)
        return ERROR;
    viewSql.Append ("SELECT NULL ").Append (sqlSnippets).AppendComma (true);
    viewSql.Append ("NULL ").Append (ECCLASSID_COLUMNNAME).AppendComma (true);

    //Source constraint
    sqlSnippets = relationMap.GetSourceECInstanceIdPropMap ()->ToNativeSql (nullptr, ECSqlType::Select, false);
    if (sqlSnippets.size () != 1)
        return ERROR;
    viewSql.Append ("NULL ").Append (sqlSnippets).AppendComma (true);

    sqlSnippets = relationMap.GetSourceECClassIdPropMap ()->ToNativeSql (nullptr, ECSqlType::Select, false);
    if (sqlSnippets.size () != 1)
        return ERROR;
    viewSql.Append ("NULL ").Append (sqlSnippets).AppendComma (true);

    //Target constraint
    sqlSnippets = relationMap.GetTargetECInstanceIdPropMap ()->ToNativeSql (nullptr, ECSqlType::Select, false);
    if (sqlSnippets.size () != 1)
        return ERROR;
    viewSql.Append ("NULL ").Append (sqlSnippets).AppendComma (true);

    sqlSnippets = relationMap.GetTargetECClassIdPropMap ()->ToNativeSql (nullptr, ECSqlType::Select, false);
    if (sqlSnippets.size () != 1)
        return ERROR;
    viewSql.Append ("NULL ").Append (sqlSnippets);

    if (endWithComma)
        viewSql.AppendComma (true);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
//static
BentleyStatus ViewGenerator::AppendConstraintClassIdPropMap (NativeSqlBuilder& viewSql, ECSqlPrepareContext const& prepareContext, PropertyMapRelationshipConstraint const& propMap, ECDbMapCR ecdbMap, RelationshipClassMapCR relationMap, ECRelationshipConstraintCR constraint)
    {
    if (propMap.IsVirtual ())
        {
        bset<IClassMap const*> classMaps;
        ecdbMap.GetClassMapsFromRelationshipEnd (classMaps, constraint, true);
        if (classMaps.size () != 1)
            {
            BeAssert (false && "Expecting exactly one ClassMap at end");
            return BentleyStatus::ERROR;
            }
        auto classMap = *classMaps.begin ();
        BeAssert (classMap != nullptr);
        const auto endClassId = classMap->GetClass ().GetId ();

        viewSql.Append (endClassId).AppendSpace ();
        viewSql.Append (propMap.ToNativeSql ( nullptr, ECSqlType::Select, false));
        }
    else
        propMap.AppendSelectClauseSqlSnippetForView (viewSql);

    return SUCCESS;
    }


//static 
void ViewGenerator::LoadDerivedClassMaps (std::map<ECClassId, IClassMap const *>& viewClasses, ECDbMapCR map, IClassMap const* classMap)
    {
    if (viewClasses.find (classMap->GetClass().GetId ()) != viewClasses.end ())
        return;

    ECDbSchemaManagerCR schemaManager = map.GetECDbR ().Schemas ();
    
    for (ECClassCP ecClass : schemaManager.GetDerivedECClasses (classMap->GetClass ()))
        {
        if (viewClasses.find (ecClass->GetId ()) == viewClasses.end ())
            continue;

        IClassMap const* classMap = map.GetClassMap (*ecClass);
        if (classMap == nullptr || classMap->GetMapStrategy ().IsNotMapped ())
            {
            continue;
            }

        viewClasses.insert (std::map<ECClassId, IClassMap const*>::value_type (ecClass->GetId (), classMap));

        if (!ecClass->GetDerivedClasses ().empty ())
            {
            LoadDerivedClassMaps (viewClasses, map, classMap);
            }
        }
    }

//static
void ViewGenerator::CreateSystemClassView (NativeSqlBuilder &viewSql, std::map<ECDbSqlTable const*, std::vector<IClassMap const*>> &tableMap, std::set<ECDbSqlTable const*> &tableToIncludeEntirly, bool forStructArray, ECSqlPrepareContext const& prepareContext)
    {
    bool first = true;
    for (auto& pair : tableMap)
        {

        std::vector<IClassMap const*>& classMaps = pair.second;
        ECDbSqlTable const* tableP = pair.first;

        bool includeEntireTable = tableToIncludeEntirly.find (tableP) != tableToIncludeEntirly.end ();
        IClassMap const* classMap = classMaps[0];
        ECDbSqlColumn const* ecInstanceIdColumn = classMap->GetPropertyMap ("ECInstanceId")->GetFirstColumn ();
        ECDbSqlColumn const* ecClassIdColumn = tableP->FindColumnCP ("ECClassId");

        if (tableP->GetPersistenceType() == PersistenceType::Virtual)
            continue;

        DbMetaDataHelper::ObjectType objectType = DbMetaDataHelper::GetObjectType (classMap->GetECDbMap ().GetECDbR (), tableP->GetName ().c_str ());
        if (objectType == DbMetaDataHelper::ObjectType::None)
            continue;

        if (!first)
            {
            viewSql.Append ("\r\nUNION", true);
            }

        viewSql.Append ("SELECT", true);
        viewSql.Append (ecInstanceIdColumn->GetName ().c_str (), true);
        if (first)
            {
            viewSql.Append (ECDB_COL_ECInstanceId, true);
            }

        viewSql.AppendComma (true);

        if (ecClassIdColumn)
            viewSql.Append (ecClassIdColumn->GetName ().c_str (), true);
        else
            viewSql.Append (classMap->GetClass ().GetId ()).AppendSpace();
        
        if (first)
            {
            viewSql.Append ("ECClassId", true);
            }

        if (forStructArray)
            {
            viewSql.AppendComma (true).Append (ECDbSystemSchemaHelper::ECPROPERTYPATHID_PROPNAME).AppendComma (true).Append (ECDbSystemSchemaHelper::ECARRAYINDEX_PROPNAME);
            }

        viewSql.AppendSpace ().Append ("FROM", true).AppendEscaped (tableP->GetName ().c_str ()).AppendSpace ();

        bool wherePreset = false;
        if (!includeEntireTable && classMaps.size () > 1)
            {
            if (classMaps.size () == 1)
                viewSql.Append ("WHERE ECClassID = ").Append (classMap->GetClass ().GetId ());
            else
                {
                viewSql.Append ("WHERE ECClassID IN (");
                for (auto itor = classMaps.begin (); itor != classMaps.end (); ++itor)
                    {
                    viewSql.Append ((*itor)->GetClass ().GetId ());
                    if (itor != (classMaps.end () - 1))
                        viewSql.AppendComma (true);
                    }

                viewSql.AppendParenRight ();
                }

            wherePreset = true;
            }

        if (classMap->IsMappedToSecondaryTable ())
            {
            viewSql.AppendSpace ();
            if (!wherePreset)
                viewSql.Append ("WHERE", true);
            else
                viewSql.Append ("AND", true);

            if (forStructArray)
                viewSql.Append ("(ECPropertyPathId IS NOT NULL AND ECArrayIndex IS NOT NULL)");
            else
                viewSql.Append ("(ECPropertyPathId IS NULL AND ECArrayIndex IS NULL)");
            }
        first = false;
        }
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
