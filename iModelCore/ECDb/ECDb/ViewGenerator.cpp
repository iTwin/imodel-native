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
    //isPolymorphic is not implemented. By default all querys are polymorphic
    if (classMap.IsRelationshipClassMap ())
        return CreateViewForRelationship (viewSql, map, classMap, isPolymorphicQuery, optimizeByIncludingOnlyRealTables);

    viewSql.AppendParenLeft ();
    auto& db = map.GetECDbR();
    auto objectType = DbMetaDataHelper::GetObjectType(db, classMap.GetTable().GetName().c_str());   
    if (objectType == DbMetaDataHelper::ObjectType::None && !isPolymorphicQuery)
        {
        auto status = CreateNullView (viewSql, classMap);
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
                if (pvm.second.GetStorageType() == DbMetaDataHelper::ObjectType::None)
                continue;

            if (queriesAddedToUnion > 0)
                viewSql.Append (" UNION ");

            status = GetViewQueryForChild (viewSql, map, *pvm.first, pvm.second.GetClassMaps (), classMap, isPolymorphicQuery, prepareContext);
            if (status != BentleyStatus::SUCCESS)
                return status;

            queriesAddedToUnion++;
            }

        if (queriesAddedToUnion == 0)
            {
            auto status = CreateNullView (viewSql, classMap);
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
BentleyStatus ViewGenerator::CreateNullView (NativeSqlBuilder& viewSql, IClassMap const& classMap)
    {
    if (classMap.IsMappedToSecondaryTable ())
        viewSql.Append ("SELECT NULL ECClassId, NULL " ECDB_COL_ECInstanceId ", NULL ParentECInstanceId, NULL ECPropertyPathId, NULL ECArrayIndex");
    else
        viewSql.Append ("SELECT NULL ECClassId, NULL " ECDB_COL_ECInstanceId);

    std::vector<std::pair<PropertyMapCP, PropertyMapCP>> viewPropMaps;
    auto status = GetPropertyMapsOfDerivedClassCastAsBaseClass (viewPropMaps, classMap, classMap, false, false);
    if (status != SUCCESS)
        return status;

    if (!viewPropMaps.empty ())
        {
        viewSql.AppendComma (true);
        AppendViewPropMapsToQuery (viewSql, classMap.GetECDbMap ().GetECDbR (), classMap.GetTable (), viewPropMaps, true /*forNullView*/);
        }

    viewSql.Append (" LIMIT 0");
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ViewGenerator::GetRootClasses (std::vector<IClassMap const*>& rootClasses, ECDbR db)
    {
    ECSchemaList schemas;
    if (db.GetSchemaManager ().GetECSchemas (schemas, true) != SUCCESS)
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

    if (!ECDbPolicyManager::GetClassPolicy (*classMap, IsValidInECSqlPolicyAssertion::Get ()).IsSupported () || classMap->IsRelationshipClassMap ())
        //ECSQL_WIP Not supported yet
        return BentleyStatus::SUCCESS;
                        
    if (classMap->GetTable().GetColumns ().empty ())
        return BentleyStatus::SUCCESS;
        
    auto itor = viewMembers.find (&classMap->GetTable());
    if (itor == viewMembers.end())
        {
        DbMetaDataHelper::ObjectType storageType = DbMetaDataHelper::ObjectType::Table;
        if (optimizeByIncludingOnlyRealTables)
            {
            //This is a db query so optimization comes at a cost
            storageType = DbMetaDataHelper::GetObjectType(map.GetECDbR(), classMap->GetTable().GetName().c_str());
            }
        viewMembers.insert(
            ViewMemberByTable::value_type(&classMap->GetTable(), ViewMember(storageType, *classMap)));
        }
    else
        {
        if (optimizeByIncludingOnlyRealTables)
            {
            if (itor->second.GetStorageType() == DbMetaDataHelper::ObjectType::Table)
                itor->second.GetClassMaps().push_back (classMap);
            }
        else
            itor->second.GetClassMaps().push_back (classMap);
        }

    if (isPolymorphic)
        {
        auto const& derivedClasses = ensureDerivedClassesAreLoaded ? map.GetECDbR ().GetSchemaManager ().GetDerivedECClasses (ecClass) : ecClass.GetDerivedClasses ();
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
BentleyStatus ViewGenerator::GetPropertyMapsOfDerivedClassCastAsBaseClass (std::vector<std::pair<PropertyMapCP, PropertyMapCP>>& propMaps, IClassMap const& baseClassMap, IClassMap const& childClassMap, bool skipSystemProperties, bool embededStatement)
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
BentleyStatus ViewGenerator::AppendViewPropMapsToQuery (NativeSqlBuilder& viewQuery, ECDbR ecdb, ECDbSqlTable const& table, std::vector<std::pair<PropertyMapCP, PropertyMapCP>> const& viewPropMaps, bool forNullView)
    {
    bool isFirstItem = true;
    for (auto const& propMapPair : viewPropMaps)
        {
        auto basePropMap = propMapPair.first;
        auto actualPropMap = propMapPair.second;

        auto aliasSqlSnippets = basePropMap->ToNativeSql (nullptr, ECSqlType::Select);
        //legacy file support: This overload checks whether the columns exist (only for CG props) and
        //in that case returns "NULL" for the column.
        auto colSqlSnippets = actualPropMap->ToNativeSql (ecdb, table);

        const size_t snippetCount = colSqlSnippets.size ();
        if (aliasSqlSnippets.size () != snippetCount)
            {
            BeAssert (false && "Number of alias SQL snippets is expected to be the same as number of column SQL snippets.");
            return ERROR;
            }

        for (size_t i = 0; i < snippetCount; i++)
            {
            if (!isFirstItem)
                viewQuery.AppendComma (true);

            auto const& aliasSqlSnippet = aliasSqlSnippets[i];
            if (forNullView)
                viewQuery.Append ("NULL ");
            else
                viewQuery.Append (colSqlSnippets[i]).AppendSpace ();
            
            viewQuery.Append (aliasSqlSnippet);
            isFirstItem = false;
            }        
        }

    return SUCCESS;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      07/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::GetViewQueryForChild (NativeSqlBuilder& viewSql, ECDbMapCR map, ECDbSqlTable const& table, const std::vector<IClassMap const*>& childClassMap, IClassMap const& baseClassMap, bool isPolymorphic,  ECSqlPrepareContext const& prepareContext)
    {
    PRECONDITION(!childClassMap.empty(), BentleyStatus::ERROR);  
    PRECONDITION(!table.GetColumns().empty (), BentleyStatus::ERROR);  

    std::vector<ECClassId> classesMappedToTable;

    if (ECDbSchemaPersistence::GetClassesMappedToTable (classesMappedToTable, table, map.GetECDbR ()) != BE_SQLITE_DONE)
        return BentleyStatus::ERROR;
 
    bool oneToManyMapping = classesMappedToTable.size() > 1;

    auto firstChildClassMap = *childClassMap.begin ();

    //Generate Select statement
    viewSql.Append ("SELECT ");
    
    if (auto classIdColumn = table.FindColumnCP(ECDB_COL_ECClassId))
        viewSql.Append (classIdColumn->GetName ().c_str());
    else
        viewSql.Append (firstChildClassMap->GetClass ().GetId ()).AppendSpace ().Append (ECCLASSID_COLUMNNAME);

    std::vector<std::pair<PropertyMapCP, PropertyMapCP>> viewPropMaps;
    
    //auto skipSystemProperties = structArrayProperty == nullptr;
    auto isEmbeded = prepareContext.GetParentArrayProperty () != nullptr;
    auto status = GetPropertyMapsOfDerivedClassCastAsBaseClass (viewPropMaps, baseClassMap, *firstChildClassMap, false, isEmbeded);
    if (status != BentleyStatus::SUCCESS)
        return status;

    if (!viewPropMaps.empty ())
        viewSql.AppendComma (true);

    //Append prop map columns to query [col1],[col2], ...
    AppendViewPropMapsToQuery (viewSql, map.GetECDbR (), table, viewPropMaps);

    viewSql.Append (" FROM ").AppendEscaped (table.GetName().c_str());

    NativeSqlBuilder where;
    if (oneToManyMapping)
        {
        if (isPolymorphic)
            {
            std::set<ECClassId> inConstraintCIDs;
            for(auto classMap  : childClassMap)
                inConstraintCIDs.insert (classMap->GetClass ().GetId ());

            std::vector<ECClassId> notInConstraintCIDs;
            for(auto classId  : classesMappedToTable)
                {
                if (inConstraintCIDs.find(classId) == inConstraintCIDs.end())
                    notInConstraintCIDs.push_back(classId);
                }

            //Here we want to create minimum size IN() query. So we will either exclude or include base on which one is minimum
            if (notInConstraintCIDs.size() > inConstraintCIDs.size()) // include class ids of class we do want.
                {
                if (!inConstraintCIDs.empty())
                    {
                    where.AppendParenLeft().Append (ECCLASSID_COLUMNNAME).Append (" IN (");
                    bool isFirstItem = true;
                    for(auto& classMap  : childClassMap)
                        {
                        if (!isFirstItem)
                            where.AppendComma (true);

                        where.Append(classMap->GetClass ().GetId());       

                        isFirstItem = false;
                        }
                    where.AppendParenRight ().AppendParenRight ();
                    }
                }
            else //exclude class ids of class we don't want. 
                {
                if (!notInConstraintCIDs.empty())
                    {
                    where.AppendParenLeft ().Append (ECCLASSID_COLUMNNAME).Append (" NOT IN (");
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
            where.AppendParenLeft ().Append (ECCLASSID_COLUMNNAME).Append (" = ").Append (firstChildClassMap->GetClass ().GetId()).AppendParenRight ();
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
BentleyStatus ViewGenerator::CreateNullViewForRelationshipClassEndTableMap (NativeSqlBuilder& viewSql, RelationshipClassMapCR relationMap, IClassMap const& baseClassMap)
    {
    AppendSystemPropMapsToNullView (viewSql, relationMap, false /*endWithComma*/);

    viewSql.Append (" LIMIT 0");

    return BentleyStatus::SUCCESS;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateNullViewForRelationshipClassLinkTableMap (NativeSqlBuilder& viewSql, RelationshipClassMapCR relationMap, IClassMap const& baseClassMap)
    {
    AppendSystemPropMapsToNullView (viewSql, relationMap, false /*endWithComma*/);

    //! Only link table mapped relationship properties are persisted
    std::vector<std::pair<PropertyMapCP, PropertyMapCP>> viewPropMaps;
    auto status = GetPropertyMapsOfDerivedClassCastAsBaseClass (viewPropMaps, baseClassMap, relationMap, true, false);
    if (status != BentleyStatus::SUCCESS)
        return status;

    if (!viewPropMaps.empty ())
        viewSql.AppendComma (true);

    //Append columns to query [col1],[col2], ...
    AppendViewPropMapsToQuery (viewSql, relationMap.GetECDbMap ().GetECDbR (), relationMap.GetTable (), viewPropMaps, true);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::GetAllChildRelationships (std::vector<RelationshipClassMapCP>& relationshipMaps, ECDbMapCR map, IClassMap const& baseRelationMap)
    {
    auto& baseClass = baseRelationMap.GetClass();
    for (auto childClass : map.GetECDbR ().GetSchemaManager ().GetDerivedECClasses (const_cast<ECClassR>(baseClass)))
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

        auto status = GetAllChildRelationships (relationshipMaps, map, *childClassMap);
        if (status != BentleyStatus::SUCCESS)
            return status;
        }

    return BentleyStatus::SUCCESS;  
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateViewForRelationshipClassLinkTableMap (NativeSqlBuilder& viewSql, ECDbMapCR ecdbMap, RelationshipClassMapCR relationMap, IClassMap const& baseClassMap)
    {
    viewSql.Append ("SELECT ");
    AppendSystemPropMaps (viewSql, ecdbMap, relationMap);

    //! Only link table mapped relationship properties are persisted
    std::vector<std::pair<PropertyMapCP, PropertyMapCP>> viewPropMaps;
    auto status = GetPropertyMapsOfDerivedClassCastAsBaseClass (viewPropMaps, baseClassMap, relationMap, true, false);
    if (status != SUCCESS)
        return status;

    if (!viewPropMaps.empty ())
        viewSql.AppendComma (true);

    //Append prop maps' columns to query [col1],[col2], ...
    AppendViewPropMapsToQuery (viewSql, ecdbMap.GetECDbR (), relationMap.GetTable (), viewPropMaps);

    viewSql.Append (" FROM ").AppendEscaped (relationMap.GetTable ().GetName ().c_str ());
    return BentleyStatus::SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateViewForRelationshipClassEndTableMap (NativeSqlBuilder& viewSql, ECDbMapCR ecdbMap, RelationshipClassEndTableMapCR relationMap, IClassMap const& baseClassMap)
    {
    //ECInstanceId, ECClassId of the relationship instance
    viewSql.Append ("SELECT ");
    AppendSystemPropMaps (viewSql, ecdbMap, relationMap);

    //FROM & WHERE
    viewSql.Append (" FROM ").AppendEscaped (relationMap.GetTable ().GetName ().c_str ());
    viewSql.Append (" WHERE (").Append (relationMap.GetOtherEndECInstanceIdPropMap ()->ToNativeSql (nullptr, ECSqlType::Select)).Append (" IS NOT NULL");
    if (!relationMap.GetOtherEndECClassIdPropMap ()->IsVirtual ())
        {
        viewSql.Append (" AND ").Append (relationMap.GetOtherEndECClassIdPropMap ()->ToNativeSql (nullptr, ECSqlType::Select)).Append (" IS NOT NULL");
        }
    viewSql.Append (")");
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateViewForRelationship (NativeSqlBuilder& viewSql, ECDbMapCR map, IClassMap const& relationMap, IClassMap const& baseClassMap)
    {
    switch(relationMap.GetClassMapType())
        {
        case ClassMap::Type::RelationshipEndTable:
            return CreateViewForRelationshipClassEndTableMap (viewSql, map, static_cast<RelationshipClassEndTableMapCR>(relationMap), baseClassMap);
        case ClassMap::Type::RelationshipLinkTable:
            return CreateViewForRelationshipClassLinkTableMap (viewSql, map, static_cast<RelationshipClassMapCR>(relationMap), baseClassMap);
        }

    return ERROR;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateNullViewForRelationship (NativeSqlBuilder& viewSql, ECDbMapCR map, IClassMap const& relationMap, IClassMap const& baseClassMap)
    {
    switch (relationMap.GetClassMapType ())
        {
        case ClassMap::Type::RelationshipEndTable:
            return CreateNullViewForRelationshipClassEndTableMap (viewSql, static_cast<RelationshipClassMapCR>(relationMap), baseClassMap);
        case ClassMap::Type::RelationshipLinkTable:
            return CreateNullViewForRelationshipClassLinkTableMap (viewSql, static_cast<RelationshipClassMapCR>(relationMap), baseClassMap);
        default:
            return ERROR;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      07/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateViewForRelationship (NativeSqlBuilder& viewSql, ECDbMapCR map, IClassMap const& relationMap, bool isPolymorphic, bool optimizeByIncludingOnlyRealTables)
    {
    BeAssert (relationMap.IsRelationshipClassMap ());

    auto& db = map.GetECDbR();
    BentleyStatus status = BentleyStatus::SUCCESS;

    if (relationMap.GetMapStrategy().IsUnmapped ())
        return BentleyStatus::ERROR;

    auto& table = relationMap.GetTable ();
    if (map.GetSQLManager().IsNullTable (table))
        {
        BeAssert(false && "Failed to get persistence table for the relationship");
        return BentleyStatus::ERROR;
        }
    viewSql.AppendParenLeft();
    viewSql.Append("Select * From ");
    viewSql.AppendParenLeft ();
    auto objectType = DbMetaDataHelper::GetObjectType (db, table.GetName ().c_str ());
    if (objectType == DbMetaDataHelper::ObjectType::None || objectType == DbMetaDataHelper::ObjectType::Index)
        status = CreateNullViewForRelationship (viewSql, map, relationMap, relationMap);
    else 
        status = CreateViewForRelationship (viewSql, map, relationMap, relationMap);

    if (status != BentleyStatus::SUCCESS)
        return status; 

    if (isPolymorphic)
        {
        std::vector<RelationshipClassMapCP> childRelationshipMaps;
        if (GetAllChildRelationships(childRelationshipMaps, map, relationMap) != BentleyStatus::SUCCESS)
            return BentleyStatus::ERROR;      

        for (auto childRelationshipMap : childRelationshipMaps)
            {           
            NativeSqlBuilder unionQuery;
            if (optimizeByIncludingOnlyRealTables)
                {
                auto& table = childRelationshipMap->GetTable ();
                if (map.GetSQLManager ().IsNullTable (table))
                    {
                    BeAssert(false && "Failed to get persistence table for the relationship");
                    return BentleyStatus::ERROR;
                    }

                auto objectType = DbMetaDataHelper::GetObjectType (db, table.GetName ().c_str ());
                if (objectType != DbMetaDataHelper::ObjectType::Table)
                    continue;
                }

            status = CreateViewForRelationship(unionQuery, map, *childRelationshipMap, relationMap);
            if (status != BentleyStatus::SUCCESS)
                return status;

            if (!unionQuery.IsEmpty ())
                viewSql.Append (" UNION ").Append (unionQuery);
            }
        }
    viewSql.AppendParenRight();
    viewSql.Append("Where ECClassId not NULL");
    viewSql.AppendParenRight ();
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
//static
BentleyStatus ViewGenerator::AppendSystemPropMaps (NativeSqlBuilder& viewSql, ECDbMapCR ecdbMap, RelationshipClassMapCR relationMap)
    {
    viewSql.Append (relationMap.GetECInstanceIdPropertyMap ()->ToNativeSql (nullptr, ECSqlType::Select)).AppendComma (true);
    viewSql.Append (relationMap.GetClass ().GetId ()).AppendSpace ().Append (ECCLASSID_COLUMNNAME).AppendComma (true);

    //Source
    BeAssert (dynamic_cast<PropertyMapRelationshipConstraint const*> (relationMap.GetSourceECInstanceIdPropMap ()) != nullptr);
    auto propMap = static_cast<PropertyMapRelationshipConstraint const*> (relationMap.GetSourceECInstanceIdPropMap ());
    propMap->AppendSelectClauseSqlSnippetForView (viewSql);
    viewSql.AppendComma (true);

    BeAssert (dynamic_cast<PropertyMapRelationshipConstraint const*> (relationMap.GetSourceECClassIdPropMap ()) != nullptr);
    propMap = static_cast<PropertyMapRelationshipConstraint const*> (relationMap.GetSourceECClassIdPropMap ());
    AppendConstraintClassIdPropMap (viewSql, *propMap, ecdbMap, relationMap.GetRelationshipClass ().GetSource ());
    viewSql.AppendComma (true);

    //Target
    BeAssert (dynamic_cast<PropertyMapRelationshipConstraint const*> (relationMap.GetTargetECInstanceIdPropMap ()) != nullptr);
    propMap = static_cast<PropertyMapRelationshipConstraint const*> (relationMap.GetTargetECInstanceIdPropMap ());
    propMap->AppendSelectClauseSqlSnippetForView (viewSql);
    viewSql.AppendComma (true);

    BeAssert (dynamic_cast<PropertyMapRelationshipConstraint const*> (relationMap.GetTargetECClassIdPropMap ()) != nullptr);
    propMap = static_cast<PropertyMapRelationshipConstraint const*> (relationMap.GetTargetECClassIdPropMap ());
    AppendConstraintClassIdPropMap (viewSql, *propMap, ecdbMap, relationMap.GetRelationshipClass ().GetTarget ());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
//static
BentleyStatus ViewGenerator::AppendSystemPropMapsToNullView (NativeSqlBuilder& viewSql, RelationshipClassMapCR relationMap, bool endWithComma)
    {
    //ECInstanceId and ECClassId
    auto sqlSnippets = relationMap.GetECInstanceIdPropertyMap ()->ToNativeSql (nullptr, ECSqlType::Select);
    if (sqlSnippets.size () != 1)
        return ERROR;
    viewSql.Append ("SELECT NULL ").Append (sqlSnippets).AppendComma (true);
    viewSql.Append ("NULL ").Append (ECCLASSID_COLUMNNAME).AppendComma (true);

    //Source constraint
    sqlSnippets = relationMap.GetSourceECInstanceIdPropMap ()->ToNativeSql (nullptr, ECSqlType::Select);
    if (sqlSnippets.size () != 1)
        return ERROR;
    viewSql.Append ("NULL ").Append (sqlSnippets).AppendComma (true);

    sqlSnippets = relationMap.GetSourceECClassIdPropMap ()->ToNativeSql (nullptr, ECSqlType::Select);
    if (sqlSnippets.size () != 1)
        return ERROR;
    viewSql.Append ("NULL ").Append (sqlSnippets).AppendComma (true);

    //Target constraint
    sqlSnippets = relationMap.GetTargetECInstanceIdPropMap ()->ToNativeSql (nullptr, ECSqlType::Select);
    if (sqlSnippets.size () != 1)
        return ERROR;
    viewSql.Append ("NULL ").Append (sqlSnippets).AppendComma (true);

    sqlSnippets = relationMap.GetTargetECClassIdPropMap ()->ToNativeSql (nullptr, ECSqlType::Select);
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
BentleyStatus ViewGenerator::AppendConstraintClassIdPropMap (NativeSqlBuilder& viewSql, PropertyMapRelationshipConstraint const& propMap, ECDbMapCR ecdbMap, ECRelationshipConstraintCR constraint)
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
        viewSql.Append (propMap.ToNativeSql (nullptr, ECSqlType::Select));
        }
    else
        propMap.AppendSelectClauseSqlSnippetForView (viewSql);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khna                        03/2014
//---------------------------------------------------------------------------------------
//static
BentleyStatus ViewGenerator::CreateSystemView (NativeSqlBuilder& viewSql, SystemViewType systemView, ECDbMapCR map, std::vector<ECClassId> const& classesToInclude, bool polymorphic)
    {
    std::map<ECClassId, IClassMap const*> viewClasses;

    ECDbSchemaManagerCR schemaManager = map.GetECDbR ().GetSchemaManager ();

    if (classesToInclude.empty ())
        {
        Utf8CP classSql = nullptr;
        switch (systemView)
            {
                case SystemViewType::Class: //ECClass and ECStructs (all primary instances)
                    classSql = "SELECT C.ECClassId FROM ec_Class C JOIN ec_ClassMap E USING (ECClassId) WHERE (E.MapStrategy  <> 3 AND E.MapStrategy  <> 8) AND C.IsRelationship <> 1"; break;
                case SystemViewType::RelationshipClass: //RelationshipOnly
                    classSql = "SELECT C.ECClassId FROM ec_Class C JOIN ec_ClassMap E USING (ECClassId) WHERE (E.MapStrategy  <> 3 AND E.MapStrategy  <> 8) AND C.IsRelationship = 1"; break;
                case SystemViewType::StructArray: //Structs only (all struct array instances)
                    classSql = "SELECT C.ECClassId FROM ec_Class C JOIN ec_ClassMap E USING (ECClassId) WHERE (E.MapStrategy  <> 3 AND E.MapStrategy  <> 8) AND C.IsStruct = 1"; break;
            }

        BeAssert (classSql != nullptr);
        Statement stmt;
        auto status = stmt.Prepare (map.GetECDbR (), classSql);
        if (status != BE_SQLITE_OK)
            {
            BeAssert (false && "Failed to prepare statement");
            return BentleyStatus::ERROR;
            }

        while (stmt.Step () == BE_SQLITE_ROW)
            {
            ECClassCP ecClass = schemaManager.GetECClass (stmt.GetValueInt64 (0));
            if (ecClass == nullptr)
                {
                BeAssert (false && "Failed to get ECClass from ECClassId using ECSchemaManager");
                return BentleyStatus::ERROR;
                }

            IClassMap const* classMap = map.GetClassMap (*ecClass);
            if (classMap == nullptr || !ECDbPolicyManager::GetClassPolicy (*classMap, IsValidInECSqlPolicyAssertion::Get ()).IsSupported ())
                {              
                continue;
                }

            viewClasses.insert (std::map<ECClassId, IClassMap const*>::value_type (ecClass->GetId(), classMap));
            }
        }
    else
        {     
        for (auto ecClassId : classesToInclude)
            {       
            ECClassCP ecClass = schemaManager.GetECClass (ecClassId);
            if (ecClass == nullptr)
                {
                BeAssert (false && "Failed to get ECClass from ECClassId using ECSchemaManager");
                return BentleyStatus::ERROR;
                }

            IClassMap const* classMap = map.GetClassMap (*ecClass);
            if (classMap == nullptr || classMap->GetMapStrategy ().IsUnmapped ())
                {
                continue;
                }
            
            if (polymorphic && ecClass->GetRelationshipClassCP () == nullptr)
                LoadDerivedClassMaps (viewClasses, map, classMap);
            else
                viewClasses.insert (std::map<ECClassId, IClassMap const*>::value_type (ecClass->GetId (), classMap));
            }
        }

    std::map<ECDbSqlTable const*, std::vector<IClassMap const*>> tableMap;
    std::map<ECDbSqlTable const*, size_t> tableMapDb;
    std::set<ECDbSqlTable const*> tableToIncludeEntirly;   

    for (auto& pair : viewClasses)
        {
        tableMap[&pair.second->GetTable ()].push_back (pair.second);
        }

    if (!classesToInclude.empty ())
        {
        Statement stmt;
        auto status= stmt.Prepare (map.GetECDbR (), "SELECT COUNT(*) FROM ec_ClassMap GROUP BY MapToDbTable WHERE MapToDbTable = ?");
        if (status != BE_SQLITE_OK)
            {
            BeAssert (false && "Failed to prepare statement");
            return BentleyStatus::ERROR;
            }

        for (auto& pair : tableMap)
            {
            stmt.BindText (1, pair.first->GetName (), Statement::MakeCopy::No);
            if (stmt.Step () != BE_SQLITE_ROW)
                {
                BeAssert (false);
                return BentleyStatus::ERROR;
                }

            if (pair.second.size() == (size_t)stmt.GetValueInt (0))
                tableToIncludeEntirly.insert (pair.first);

            tableMapDb[pair.first] = (size_t)stmt.GetValueInt (0);

            stmt.Reset ();
            stmt.ClearBindings ();
            }
        }
    else
        {
        for (auto& pair : tableMap)
            {
            tableToIncludeEntirly.insert (pair.first);
            }

        for (auto& pair : tableMap)
            {
            tableToIncludeEntirly.insert (pair.first);
            tableMapDb[pair.first] = pair.second.size ();
            }
        }

    if (tableMap.empty ())
        return BentleyStatus::SUCCESS;

    if (SystemViewType::Class == systemView)
        {
        CreateSystemClassView (viewSql, tableMap, tableToIncludeEntirly, false);
        }

    if (SystemViewType::StructArray == systemView)
        {
        CreateSystemClassView (viewSql, tableMap, tableToIncludeEntirly, true);
        }

    if (SystemViewType::RelationshipClass == systemView)
        {
        for (auto& pair : tableMap)
            {
            std::vector<IClassMap const*>& classMaps = pair.second;
            BeAssert (classMaps.size () == 1);

            RelationshipClassMapCP relationshipMap = dynamic_cast<RelationshipClassMapCP>(classMaps[0]);// = static_cast
            BeAssert (relationshipMap != nullptr);

            viewSql.Append ("\r\nUNION SELECT ");
            AppendSystemPropMaps (viewSql, map, *relationshipMap);
            viewSql.Append (" FROM ").AppendEscaped (relationshipMap->GetTable ().GetName ().c_str ());

            if (RelationshipClassEndTableMapCP endTableMap = dynamic_cast<RelationshipClassEndTableMapCP>(relationshipMap))
                {                
                viewSql.Append (" WHERE ").Append (endTableMap->GetOtherEndECInstanceIdPropMap ()->ToNativeSql (nullptr, ECSqlType::Select)).Append (" IS NOT NULL");
                }
            }
        }
    return BentleyStatus::SUCCESS;
    }

//static 
void ViewGenerator::LoadDerivedClassMaps (std::map<ECClassId, IClassMap const *>& viewClasses, ECDbMapCR map, IClassMap const* classMap)
    {
    if (viewClasses.find (classMap->GetClass().GetId ()) != viewClasses.end ())
        return;

    ECDbSchemaManagerCR schemaManager = map.GetECDbR ().GetSchemaManager ();
    
    for (ECClassCP ecClass : schemaManager.GetDerivedECClasses (classMap->GetClass ()))
        {
        if (viewClasses.find (ecClass->GetId ()) == viewClasses.end ())
            continue;

        IClassMap const* classMap = map.GetClassMap (*ecClass);
        if (classMap == nullptr || classMap->GetMapStrategy ().IsUnmapped ())
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
void ViewGenerator::CreateSystemClassView (NativeSqlBuilder &viewSql, std::map<ECDbSqlTable const*, std::vector<IClassMap const*>> &tableMap, std::set<ECDbSqlTable const*> &tableToIncludeEntirly, bool forStructArray)
    {
    bool first = true;
    for (auto& pair : tableMap)
        {

        std::vector<IClassMap const*>& classMaps = pair.second;
        ECDbSqlTable const* tableP = pair.first;

        bool includeEntireTable = tableToIncludeEntirly.find (tableP) != tableToIncludeEntirly.end ();
        IClassMap const* classMap = classMaps[0];
        ECDbSqlColumn const* ecInstanceIdColumn = classMap->GetPropertyMap (L"ECInstanceId")->GetFirstColumn ();
        ECDbSqlColumn const* ecClassIdColumn = tableP->FindColumnCP ("ECclassId");

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
