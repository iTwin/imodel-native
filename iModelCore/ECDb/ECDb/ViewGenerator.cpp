/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ViewGenerator.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECDbPolicyManager.h"
#include <set>
#include "ECSql/ECSqlPrepareContext.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define ECDB_HOLDING_VIEW "ec_RelationshipHoldingStatistics"
        
//************************** ViewGenerator ***************************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ViewGenerator::CreateView(NativeSqlBuilder& viewSql, ECDbMapCR map, IClassMap const& classMap, bool isPolymorphicQuery, ECSqlPrepareContext const& prepareContext, bool optimizeByIncludingOnlyRealTables)
    {
    //isPolymorphic is not implemented. By default all query are polymorphic
    if (classMap.IsRelationshipClassMap())
        return CreateViewForRelationship(viewSql, map, prepareContext, classMap, isPolymorphicQuery, optimizeByIncludingOnlyRealTables);

    viewSql.AppendParenLeft();
    ECDbR db = map.GetECDbR();
    DbMetaDataHelper::ObjectType objectType = DbMetaDataHelper::GetObjectType(db, classMap.GetPrimaryTable().GetName().c_str());
    if (objectType == DbMetaDataHelper::ObjectType::None && !isPolymorphicQuery)
        {
        if (SUCCESS != CreateNullView(viewSql, prepareContext, classMap))
            return ERROR;

        viewSql.AppendParenRight();
        return SUCCESS;
        }

    ViewMemberByTable viewMembers;
    if (ClassMap::IsAnyClass(classMap.GetClass()))
        {
        if (!isPolymorphicQuery)
            {
            BeAssert(false && "This operation require require polymorphic query to be enabled");
            return ERROR;
            }

        std::vector<IClassMap const*> rootClassMaps;
        if (SUCCESS != GetRootClasses(rootClassMaps, db))
            return ERROR;

        for (IClassMap const* classMap : rootClassMaps)
            {
            if (SUCCESS != ComputeViewMembers(viewMembers, map, classMap->GetClass(), isPolymorphicQuery, optimizeByIncludingOnlyRealTables, /*ensureDerivedClassesAreLoaded=*/ false))
                return ERROR;
            }
        }
    else
        {
        if (SUCCESS != ComputeViewMembers(viewMembers, map, classMap.GetClass(), isPolymorphicQuery, optimizeByIncludingOnlyRealTables, /*ensureDerivedClassesAreLoaded=*/ true))
            return ERROR;
        }

    int queriesAddedToUnion = 0;
    for (auto& pvm : viewMembers)
        {
        if (optimizeByIncludingOnlyRealTables && pvm.second.GetStorageType() == DbMetaDataHelper::ObjectType::None)
            continue;

        if (queriesAddedToUnion > 0)
            viewSql.Append(" UNION ");

        if (SUCCESS != GetViewQueryForChild(viewSql, map, prepareContext, *pvm.first, pvm.second.GetClassMaps(), classMap, isPolymorphicQuery))
            return ERROR;

        queriesAddedToUnion++;
        }

    if (queriesAddedToUnion == 0)
        {
        if (SUCCESS != CreateNullView(viewSql, prepareContext, classMap))
            return ERROR;

        viewSql.AppendParenRight();
        return SUCCESS;
        }
    else
        viewSql.AppendParenRight();

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ViewGenerator::CreateNullView (NativeSqlBuilder& viewSql, ECSqlPrepareContext const& prepareContext, IClassMap const& classMap)
    {
    if (classMap.MapsToStructArrayTable ())
        viewSql.Append ("SELECT NULL ECClassId, NULL " ECDB_COL_ECInstanceId ", NULL ParentECInstanceId, NULL ECPropertyPathId, NULL ECArrayIndex");
    else
        viewSql.Append ("SELECT NULL ECClassId, NULL " ECDB_COL_ECInstanceId);

    std::vector<std::pair<PropertyMapCP, PropertyMapCP>> viewPropMaps;
    if (SUCCESS != GetPropertyMapsOfDerivedClassCastAsBaseClass(viewPropMaps, prepareContext, classMap, classMap, false, false))
        return ERROR;
 
    AppendViewPropMapsToQuery (viewSql, classMap.GetECDbMap ().GetECDbR (), prepareContext, classMap.GetJoinedTable (), viewPropMaps, true /*forNullView*/);
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
    for (ECSchemaCP schema : schemas)
        {
        if (schema->IsStandardSchema())
            continue;

        for(ECClassCP ecClass : schema->GetClasses())
            {
            if (ecClass->GetDerivedClasses().empty())
                {
                IClassMap const* classMap = db.GetECDbImplR().GetECDbMap ().GetClassMap (*ecClass);
                if (classMap == nullptr)
                    {
                    BeAssert(classMap != nullptr);
                    return ERROR;
                    }
                
                if (classMap->GetClassMapType() == IClassMap::Type::Unmapped)
                    continue;

                rootClassMaps.push_back(classMap);
                }
            }
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ViewGenerator::ComputeViewMembers(ViewMemberByTable& viewMembers, ECDbMapCR map, ECClassCR ecClass, bool isPolymorphic, bool optimizeByIncludingOnlyRealTables, bool ensureDerivedClassesAreLoaded)
    {
    IClassMap const* classMap = map.GetClassMap(ecClass);
    if (classMap == nullptr || classMap->GetClassMapType() == IClassMap::Type::Unmapped)
        return SUCCESS;

    if (!classMap->GetMapStrategy().IsNotMapped())
        {
        if (classMap->GetJoinedTable().GetColumns().empty())
            return SUCCESS;

        auto itor = viewMembers.find(&classMap->GetJoinedTable());
        if (itor == viewMembers.end())
            {
            DbMetaDataHelper::ObjectType storageType = DbMetaDataHelper::ObjectType::Table;
            if (optimizeByIncludingOnlyRealTables)
                {
                //This is a db query so optimization comes at a cost
                storageType = DbMetaDataHelper::GetObjectType(map.GetECDbR(), classMap->GetJoinedTable().GetName().c_str());
                }

            if (storageType == DbMetaDataHelper::ObjectType::Table)
                viewMembers.insert(ViewMemberByTable::value_type(&classMap->GetJoinedTable(), ViewMember(storageType, *classMap)));
            }
        else
            {
            if (optimizeByIncludingOnlyRealTables)
                {
                if (itor->second.GetStorageType() == DbMetaDataHelper::ObjectType::Table)
                    itor->second.AddClassMap(*classMap);
                }
            else
                itor->second.AddClassMap(*classMap);
            }
        }

    if (isPolymorphic && !classMap->IsParentOfJoinedTable())
        {
        ECDerivedClassesList const& derivedClasses = ensureDerivedClassesAreLoaded ? map.GetECDbR().Schemas().GetDerivedECClasses(ecClass) : ecClass.GetDerivedClasses();
        for (ECClassCP derivedClass : derivedClasses)
                {
            if (SUCCESS != ComputeViewMembers(viewMembers, map, *derivedClass, isPolymorphic, optimizeByIncludingOnlyRealTables, ensureDerivedClassesAreLoaded))
                return ERROR;
                }
        }

    return SUCCESS;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::GetPropertyMapsOfDerivedClassCastAsBaseClass(std::vector<std::pair<PropertyMapCP, PropertyMapCP>>& propMaps, ECSqlPrepareContext const& prepareContext, IClassMap const& baseClassMap, IClassMap const& childClassMap, bool skipSystemProperties, bool embededStatement)
    {
    propMaps.clear();

    IClassMap const& parentMap = embededStatement ? baseClassMap.GetView(IClassMap::View::EmbeddedType) : baseClassMap;
    IClassMap const& childMap = embededStatement ? childClassMap.GetView(IClassMap::View::EmbeddedType) : childClassMap;

    for (PropertyMap const* baseClassPropertyMap : parentMap.GetPropertyMaps())
        {
        if ((skipSystemProperties && baseClassPropertyMap->IsSystemPropertyMap()) ||
            !prepareContext.GetSelectionOptions().IsSelected(baseClassPropertyMap->GetPropertyAccessString()) ||
            baseClassPropertyMap->GetAsStructArrayTablePropertyMap())
            continue;

        NavigationPropertyMap const* navPropMap = baseClassPropertyMap->GetAsNavigationPropertyMap();
        if (navPropMap != nullptr && !navPropMap->IsSupportedInECSql())
            continue;

        PropertyMap const* childClassCounterpartPropMap = childMap.GetPropertyMap(baseClassPropertyMap->GetPropertyAccessString());
        if (childClassCounterpartPropMap == nullptr)
            return ERROR;

        std::vector<ECDbSqlColumn const*> baseClassPropMapColumns;
        std::vector<ECDbSqlColumn const*> childClassPropMapColumns;
        baseClassPropertyMap->GetColumns(baseClassPropMapColumns);
        childClassCounterpartPropMap->GetColumns(childClassPropMapColumns);
        if (baseClassPropMapColumns.size() != childClassPropMapColumns.size())
            return ERROR;

        propMaps.push_back({baseClassPropertyMap, childClassCounterpartPropMap});
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::AppendViewPropMapsToQuery(NativeSqlBuilder& viewQuery, ECDbR ecdb, ECSqlPrepareContext const& prepareContext, ECDbSqlTable const& table, std::vector<std::pair<PropertyMapCP, PropertyMapCP>> const& viewPropMaps, bool forNullView)
    {    
    for (auto const& propMapPair : viewPropMaps)
        {
        PropertyMapCP basePropMap = propMapPair.first;
        PropertyMapCP actualPropMap = propMapPair.second;
        if (!prepareContext.GetSelectionOptions().IsSelected(actualPropMap->GetPropertyAccessString()))
            continue;

        auto aliasSqlSnippets = basePropMap->ToNativeSql(nullptr, ECSqlType::Select, false);
        BeAssert(actualPropMap->GetTable() != nullptr);                                                                                     
        auto colSqlSnippets = actualPropMap->ToNativeSql(actualPropMap->GetTable()->GetName().c_str(), ECSqlType::Select, false);
        auto colSqlSnippetsWithoutTableNames = actualPropMap->ToNativeSql(nullptr, ECSqlType::Select, false);

        const size_t snippetCount = colSqlSnippets.size();
        if (aliasSqlSnippets.size() != snippetCount)
            {
            BeAssert(false && "Number of alias SQL snippets is expected to be the same as number of column SQL snippets.");
            return ERROR;
            }

        for (size_t i = 0; i < snippetCount; i++)
            {
            viewQuery.AppendComma(true);
            auto const& aliasSqlSnippet = aliasSqlSnippets[i];
            if (forNullView)
                viewQuery.Append("NULL ");
            else 
                {
                viewQuery.Append(colSqlSnippets[i]);
                }
            if (strcmp(colSqlSnippetsWithoutTableNames[i].ToString(), aliasSqlSnippet.ToString()) != 0 || forNullView) //do not add alias if column name is same as alias.
                viewQuery.AppendSpace().Append(aliasSqlSnippet);
            }
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      07/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::GetViewQueryForChild(NativeSqlBuilder& viewSql, ECDbMapCR map, ECSqlPrepareContext const& prepareContext, ECDbSqlTable const& table, const std::vector<IClassMap const*>& childClassMap, IClassMap const& baseClassMap, bool isPolymorphic)
    {
    if (childClassMap.empty() || table.GetColumns().empty())
        {
        BeAssert(false);
        return ERROR;
        }

    IClassMap const* firstChildClassMap = *childClassMap.begin();
    //Generate Select statement
    viewSql.Append("SELECT ");

    ECDbSqlColumn const* classIdColumn = nullptr;
    if (table.TryGetECClassIdColumn(classIdColumn))
        {
        viewSql.AppendEscaped(table.GetName().c_str()).AppendDot().Append(classIdColumn->GetName().c_str());
        }
    else
        viewSql.Append(firstChildClassMap->GetClass().GetId()).AppendSpace().Append(ECDB_COL_ECClassId);


    std::vector<std::pair<PropertyMapCP, PropertyMapCP>> viewPropMaps;
    auto isEmbeded = prepareContext.GetParentArrayProperty() != nullptr;
    auto status = GetPropertyMapsOfDerivedClassCastAsBaseClass(viewPropMaps, prepareContext, baseClassMap, *firstChildClassMap, false, isEmbeded);
    if (status != BentleyStatus::SUCCESS)
        return status;

    //Append prop map columns to query [col1],[col2], ...
    AppendViewPropMapsToQuery(viewSql, map.GetECDbR(), prepareContext, table, viewPropMaps);
    
    //Determine which table to join for split table case
    std::set<ECDbSqlTable const*> tableToJoinOn;
    for (auto const& propMapPair : viewPropMaps)
        {
        auto actualPropMap = propMapPair.second;
        if (!prepareContext.GetSelectionOptions().IsSelected(actualPropMap->GetPropertyAccessString()))
            continue;

        tableToJoinOn.insert(actualPropMap->GetTable());
        }

    viewSql.Append(" FROM ").AppendEscaped(table.GetName().c_str());
    //Join necessary table for table
    auto primaryKey = table.GetFilteredColumnFirst(ColumnKind::ECInstanceId);
    for (auto const& vpart : firstChildClassMap->GetStorageDescription().GetVerticalPartitions())
        {
        bool tableReferencedInQuery = tableToJoinOn.find(&vpart.GetTable()) != tableToJoinOn.end();
        bool notYetReferenced = &vpart.GetTable() != &table;
        if (tableReferencedInQuery && notYetReferenced)
            {
            auto fkKey = vpart.GetTable().GetFilteredColumnFirst(ColumnKind::ECInstanceId);
            viewSql.Append(" INNER JOIN ").AppendEscaped(vpart.GetTable().GetName().c_str());
            viewSql.Append(" ON ").AppendEscaped(table.GetName().c_str()).AppendDot().AppendEscaped(primaryKey->GetName().c_str());
            viewSql.Append(" = ").AppendEscaped(vpart.GetTable().GetName().c_str()).AppendDot().AppendEscaped(fkKey->GetName().c_str());
            }
        }


    NativeSqlBuilder where;
    if (classIdColumn != nullptr)
        {
        auto tableP = &firstChildClassMap->GetPrimaryTable();
        OptionsExp const* options = prepareContext.GetCurrentScope().GetOptions();
        if (options == nullptr || !options->HasOption(OptionsExp::NOECCLASSIDFILTER_OPTION))
            {
                if (SUCCESS != baseClassMap.GetStorageDescription().GenerateECClassIdFilter(where, *tableP, *classIdColumn, isPolymorphic, tableP != &table, table.GetName().c_str()))
                return ERROR;
            }
        }

    //We allow query of struct classes.
    if (firstChildClassMap->MapsToStructArrayTable())
        {
        if (!where.IsEmpty())
        where.Append(" AND ");

        if (prepareContext.GetParentArrayProperty() == nullptr)
        where.Append("(ECPropertyPathId IS NULL AND ECArrayIndex IS NULL)");
        }

    if (!where.IsEmpty())
        viewSql.Append(" WHERE ").Append(where);

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateNullViewForRelationshipClassEndTableMap (NativeSqlBuilder& viewSql, ECSqlPrepareContext const& prepareContext, RelationshipClassMapCR relationMap, IClassMap const& baseClassMap)
    {
    viewSql.AppendParenLeft();
    AppendSystemPropMapsToNullView (viewSql, prepareContext, relationMap, false /*endWithComma*/);
    viewSql.Append (" LIMIT 0").AppendParenRight();
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateNullViewForRelationshipClassLinkTableMap (NativeSqlBuilder& viewSql, ECSqlPrepareContext const& prepareContext, RelationshipClassMapCR relationMap, IClassMap const& baseClassMap)
    {
    viewSql.AppendParenLeft();
    AppendSystemPropMapsToNullView (viewSql, prepareContext, relationMap, false /*endWithComma*/);

    //! Only link table mapped relationship properties are persisted
    std::vector<std::pair<PropertyMapCP, PropertyMapCP>> viewPropMaps;
    if (SUCCESS != GetPropertyMapsOfDerivedClassCastAsBaseClass(viewPropMaps, prepareContext, baseClassMap, relationMap, true, false))
        return ERROR;

    //Append columns to query [col1],[col2], ...
    AppendViewPropMapsToQuery (viewSql, relationMap.GetECDbMap ().GetECDbR (), prepareContext, relationMap.GetJoinedTable(), viewPropMaps, true);
    viewSql.AppendParenRight();
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::GetAllChildRelationships (std::vector<RelationshipClassMapCP>& relationshipMaps, ECDbMapCR map, ECSqlPrepareContext const& prepareContext, IClassMap const& baseRelationMap)
    {
    for (ECClassCP childClass : map.GetECDbR ().Schemas ().GetDerivedECClasses (baseRelationMap.GetClass()))
        {
        IClassMap const* childClassMap = map.GetClassMap (*childClass);
        if (childClassMap == nullptr || childClassMap->GetClassMapType() == IClassMap::Type::Unmapped)
            continue;

        if (!childClassMap->IsRelationshipClassMap())
            {
            BeAssert(false);
            return ERROR;
            }

        relationshipMaps.push_back (static_cast<RelationshipClassMapCP>(childClassMap));

        if (SUCCESS != GetAllChildRelationships(relationshipMaps, map, prepareContext, *childClassMap))
            return ERROR;
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateViewForRelationshipClassLinkTableMap (NativeSqlBuilder& viewSql, ECDbMapCR ecdbMap, ECSqlPrepareContext const& prepareContext, RelationshipClassMapCR relationMap, IClassMap const& baseClassMap)
    {
    viewSql.Append ("SELECT ");
    AppendSystemPropMaps (viewSql, ecdbMap, prepareContext, relationMap, relationMap.GetPrimaryTable());

    //! Only link table mapped relationship properties are persisted
    std::vector<std::pair<PropertyMapCP, PropertyMapCP>> viewPropMaps;
    if (SUCCESS != GetPropertyMapsOfDerivedClassCastAsBaseClass(viewPropMaps, prepareContext, baseClassMap, relationMap, true, false))
        return ERROR;

    //Append prop maps' columns to query [col1],[col2], ...
    AppendViewPropMapsToQuery (viewSql, ecdbMap.GetECDbR (), prepareContext, relationMap.GetJoinedTable(), viewPropMaps);

    viewSql.Append (" FROM ").AppendEscaped (relationMap.GetJoinedTable().GetName ().c_str ());
   
    //Append secondary table JOIN
    if (SUCCESS != BuildRelationshipJoinIfAny (viewSql, relationMap, ECN::ECRelationshipEnd::ECRelationshipEnd_Source, relationMap.GetPrimaryTable()))
        return ERROR;

    return BuildRelationshipJoinIfAny(viewSql, relationMap, ECN::ECRelationshipEnd::ECRelationshipEnd_Target, relationMap.GetPrimaryTable());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateViewForRelationshipClassEndTableMap (NativeSqlBuilder& viewSql, ECDbMapCR ecdbMap, ECSqlPrepareContext const& prepareContext, RelationshipClassEndTableMap const& relationMap, IClassMap const& baseClassMap)
    {
    //ECInstanceId, ECClassId of the relationship instance
		std::vector<ECDbSqlColumn const*> columns;
		relationMap.GetECInstanceIdPropertyMap()->GetColumns(columns);
		NativeSqlBuilder::List builders;
        bool first = true;
        for (ECDbSqlColumn const* column : columns)
            {
            ECDbSqlTable const& table = column->GetTable ();
            if (table.GetPersistenceType () == PersistenceType::Virtual)
                continue;

            NativeSqlBuilder selectSQL;
            selectSQL.Append ("SELECT ");
            AppendSystemPropMaps (selectSQL, ecdbMap, prepareContext, relationMap, table);
            selectSQL.Append (" FROM ").AppendEscaped (table.GetName ().c_str ());

            //Append secondary table JOIN
            if (SUCCESS != BuildRelationshipJoinIfAny (selectSQL, relationMap, ECN::ECRelationshipEnd::ECRelationshipEnd_Source, table))
                return ERROR;

            if (SUCCESS != BuildRelationshipJoinIfAny (selectSQL, relationMap, ECN::ECRelationshipEnd::ECRelationshipEnd_Target, table))
                return ERROR;

            selectSQL.Append (" WHERE ").Append (relationMap.GetReferencedEndECInstanceIdPropMap ()->ToNativeSql (/*relationMap.GetPrimaryTable().GetName().c_str()*/ nullptr, ECSqlType::Select, false)).Append (" IS NOT NULL");
            if (first)
                first = false;
            else
                {
                viewSql.Append(" UNION ");
                }

            viewSql.Append(selectSQL);
            }


    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      07/2015
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::BuildRelationshipJoinIfAny (NativeSqlBuilder& sqlBuilder, RelationshipClassMapCR classMap, ECN::ECRelationshipEnd endPoint, ECDbSqlTable const& contextTable)
    {
    if (classMap._RequiresJoin(endPoint))
        {
        ECDbMapCR ecdbMap = classMap.GetECDbMap();
        ECClassIdRelationshipConstraintPropertyMap const* ecclassIdPropertyMap = endPoint == ECRelationshipEnd::ECRelationshipEnd_Source ? classMap.GetSourceECClassIdPropMap() : classMap.GetTargetECClassIdPropMap();
        ECInstanceIdRelationshipConstraintPropertyMap const* ecInstanceIdPropertyMap = static_cast<ECInstanceIdRelationshipConstraintPropertyMap const*>(endPoint == ECRelationshipEnd::ECRelationshipEnd_Source ? classMap.GetSourceECInstanceIdPropMap() : classMap.GetTargetECInstanceIdPropMap());
        size_t tableCount = ecdbMap.GetTableCountOnRelationshipEnd(endPoint == ECRelationshipEnd::ECRelationshipEnd_Source ? classMap.GetRelationshipClass().GetSource() : classMap.GetRelationshipClass().GetTarget());
        ECDbSqlTable const* targetTable = &ecclassIdPropertyMap->GetSingleColumn()->GetTable();
        if (tableCount > 1 
            /*In this case we expecting we have relationship with one end abstract we only support it in case joinedTable*/)
            {
            BeAssert(targetTable->GetTableType() == TableType::Joined);
            if (targetTable->GetTableType() != TableType::Joined)
                return ERROR;

            targetTable = ecdbMap.GetPrimaryTable(ecclassIdPropertyMap->GetSingleColumn()->GetTable());
            if (!targetTable)
                return ERROR;
            }

        sqlBuilder.Append(" INNER JOIN ");
        sqlBuilder.AppendEscaped(targetTable->GetName().c_str());
        sqlBuilder.AppendSpace();
        sqlBuilder.Append(GetECClassIdPrimaryTableAlias(endPoint));
        sqlBuilder.Append(" ON ");
        sqlBuilder.Append(GetECClassIdPrimaryTableAlias(endPoint));
        sqlBuilder.AppendDot();
        ECDbSqlColumn const* targetECInstanceIdColumn = targetTable->GetFilteredColumnFirst(ColumnKind::ECInstanceId);
        if (targetECInstanceIdColumn == nullptr)
            {
            BeAssert(false && "Failed to find ECInstanceId column in target table");
            return ERROR;
            }

        sqlBuilder.AppendEscaped(targetECInstanceIdColumn->GetName().c_str());
        sqlBuilder.Append(BooleanSqlOperator::EqualTo, false);
        sqlBuilder.Append(ecInstanceIdPropertyMap->GetSingleColumn()->GetTable().GetName().c_str(), ecInstanceIdPropertyMap->GetSingleColumn()->GetName().c_str());
        sqlBuilder.AppendSpace();
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateViewForRelationship (NativeSqlBuilder& viewSql, ECDbMapCR map, ECSqlPrepareContext const& prepareContext, IClassMap const& relationMap, IClassMap const& baseClassMap)
    {
    switch(relationMap.GetClassMapType())
        {
        case ClassMap::Type::RelationshipEndTable:
            return CreateViewForRelationshipClassEndTableMap (viewSql, map, prepareContext, static_cast<RelationshipClassEndTableMap const&>(relationMap), baseClassMap);
        case ClassMap::Type::RelationshipLinkTable:
            return CreateViewForRelationshipClassLinkTableMap (viewSql, map, prepareContext, static_cast<RelationshipClassMapCR>(relationMap), baseClassMap);
        default:
            BeAssert(false);
            return ERROR;
        }
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
            BeAssert(false);
            return ERROR;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2015
//+---------------+---------------+---------------+---------------+---------------+-------
 BentleyStatus ViewGenerator::CreateViewForRelationship(NativeSqlBuilder& viewSql, ECDbMapCR map, ECSqlPrepareContext const& prepareContext, IClassMap const& relationMap, bool isPolymorphic, bool optimizeByIncludingOnlyRealTables)
     {
     BeAssert(relationMap.IsRelationshipClassMap());
     if (relationMap.GetMapStrategy().IsNotMapped())
         return ERROR;

     ViewMemberByTable vmt;
     if (SUCCESS != ComputeViewMembers(vmt, map, relationMap.GetClass(), isPolymorphic, optimizeByIncludingOnlyRealTables, true))
         return ERROR;

     if (vmt.empty())
         return CreateNullViewForRelationship(viewSql, map, prepareContext, relationMap, relationMap);

     NativeSqlBuilder unionQuery;

     ViewMember const& viewMemberOfPrimaryTable = vmt[&relationMap.GetJoinedTable()];
     for (IClassMap const* cm : viewMemberOfPrimaryTable.GetClassMaps())
         {
         switch (cm->GetClassMapType())
             {
                 case ClassMap::Type::RelationshipEndTable:
                     if (!unionQuery.IsEmpty())
                         unionQuery.Append(" UNION ");

                     if (SUCCESS != CreateViewForRelationshipClassEndTableMap(unionQuery, map, prepareContext, *static_cast<RelationshipClassEndTableMap const*>(cm), relationMap))
                         return ERROR;

                     break;

                 case ClassMap::Type::RelationshipLinkTable:
                 {
                 if (!unionQuery.IsEmpty())
                     unionQuery.Append(" UNION ");

                 if (SUCCESS != CreateViewForRelationshipClassLinkTableMap(unionQuery, map, prepareContext, *static_cast<RelationshipClassLinkTableMap const*>(cm), relationMap))
                     return ERROR;

                 ECDbSqlTable const& table = relationMap.GetJoinedTable();
                 ECDbSqlColumn const* classIdColumn = nullptr;
                 if (table.TryGetECClassIdColumn(classIdColumn))
                     {
                     OptionsExp const* options = prepareContext.GetCurrentScope().GetOptions();
                     if (options == nullptr || !options->HasOption(OptionsExp::NOECCLASSIDFILTER_OPTION))
                         {
                         NativeSqlBuilder whereClause;
                         if (SUCCESS != cm->GetStorageDescription().GenerateECClassIdFilter(whereClause, table,
                                                                                            *classIdColumn, false, true))
                             return ERROR;

                         if (!whereClause.IsEmpty())
                             unionQuery.Append(" WHERE ").Append(whereClause);
                         }
                     }
                 }
             }
         }

     vmt.erase(&relationMap.GetJoinedTable());

     //now process view members of other tables
     for (bpair<ECDbSqlTable const*, ViewMember> const& vm : vmt)
         {
         ECDbSqlTable const* table = vm.first;
         if (vm.second.GetStorageType() != DbMetaDataHelper::ObjectType::Table)
             continue;

         std::vector<RelationshipClassEndTableMap const*> etm;
         std::vector<RelationshipClassLinkTableMap const*> ltm;
         for (IClassMap const* cm : vm.second.GetClassMaps())
             {
             switch (cm->GetClassMapType())
                 {
                     case ClassMap::Type::RelationshipEndTable:
                         etm.push_back(static_cast<RelationshipClassEndTableMap const*>(cm)); break;
                     case ClassMap::Type::RelationshipLinkTable:
                         ltm.push_back(static_cast<RelationshipClassLinkTableMap const*>(cm)); break;
                     default:
                         BeAssert(false);
                         break;
                 }
             }

         if (!ltm.empty())
             {
             if (!unionQuery.IsEmpty())
                 unionQuery.Append(" UNION ");

             RelationshipClassLinkTableMap const& firstClassMap = *ltm.front();
             if (SUCCESS != CreateViewForRelationshipClassLinkTableMap(unionQuery, map, prepareContext, firstClassMap, relationMap))
                 return ERROR;

             ECDbSqlColumn const* classIdColumn = nullptr;
             if (table->TryGetECClassIdColumn(classIdColumn))
                 {
                 OptionsExp const* options = prepareContext.GetCurrentScope().GetOptions();
                 if (options == nullptr || !options->HasOption(OptionsExp::NOECCLASSIDFILTER_OPTION))
                     {
                     NativeSqlBuilder whereClause;
                     if (SUCCESS != firstClassMap.GetStorageDescription().GenerateECClassIdFilter(whereClause, *table,
                                                                                                  *classIdColumn, isPolymorphic, true))
                         return ERROR;

                     if (!whereClause.IsEmpty())
                         unionQuery.Append(" WHERE ").Append(whereClause);
                     }
                 }
             }

         for (RelationshipClassEndTableMap const* et : etm)
             {
             if (!unionQuery.IsEmpty())
                 unionQuery.Append(" UNION ");

             if (SUCCESS != CreateViewForRelationshipClassEndTableMap(unionQuery, map, prepareContext, *et, relationMap))
                 return ERROR;
             }
         }

     viewSql.AppendParenLeft().Append(unionQuery.ToString()).AppendParenRight();
     return SUCCESS;
     }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
//static
BentleyStatus ViewGenerator::AppendSystemPropMaps (NativeSqlBuilder& viewSql, ECDbMapCR ecdbMap, ECSqlPrepareContext const& prepareContext, RelationshipClassMapCR relationMap, ECDbSqlTable const& contextTable)
    {
    //We only want to render propertyMap w.r.t contextTable. A endRelationship can now have more then one columns in each different table for ECInstanceId, SourceECInstanceId, TargetECInstanceId ...
    //There for we only need to render in term of context table that is choosen before this funtion is called. Resulting select are UNIONed.

    //ECInstanceId-----------------------------------------
    PropertyMapCP ecId = relationMap.GetECInstanceIdPropertyMap ();
    if (!ecId->IsVirtual ())
        viewSql.AppendEscaped (contextTable.GetName ().c_str ()).AppendDot ();

    BeAssert(relationMap.GetECInstanceIdPropertyMap ()->GetSingleColumn(contextTable, true) != nullptr);
    viewSql.Append (relationMap.GetECInstanceIdPropertyMap ()->ToNativeSql ( nullptr, ECSqlType::Select, false, &contextTable)).AppendComma (true);

    //ECClassId-----------------------------------
    viewSql.Append (relationMap.GetClass ().GetId ()).AppendSpace ().Append (ECDB_COL_ECClassId).AppendComma (true);

    //SourceECInstanceId-----------------------------------
    RelationshipConstraintPropertyMap const* idPropMap = static_cast<RelationshipConstraintPropertyMap const*> (relationMap.GetSourceECInstanceIdPropMap ());
    BeAssert(idPropMap->GetSingleColumn(contextTable, true) != nullptr);
    if (!idPropMap->IsVirtual ())
        viewSql.AppendEscaped (contextTable.GetName ().c_str ()).AppendDot ();

    idPropMap->AppendSelectClauseSqlSnippetForView (viewSql, contextTable);
    viewSql.AppendComma (true);

    //SourceECClassId--------------------------------------
    ECClassIdRelationshipConstraintPropertyMap const* classIdPropMap = relationMap.GetSourceECClassIdPropMap();
    if (!classIdPropMap->IsVirtual())
        {
        if (relationMap._RequiresJoin(ECRelationshipEnd::ECRelationshipEnd_Source))
            viewSql.AppendEscaped(GetECClassIdPrimaryTableAlias(ECRelationshipEnd::ECRelationshipEnd_Source)).AppendDot();
        else
            viewSql.AppendEscaped(classIdPropMap->GetSingleColumn()->GetTable().GetName().c_str()).AppendDot();
        }

    AppendConstraintClassIdPropMap (viewSql, prepareContext, *classIdPropMap, ecdbMap, relationMap, relationMap.GetRelationshipClass ().GetSource (), contextTable);
    viewSql.AppendComma (true);

    //TargetECInstanceId-----------------------------------
    BeAssert (dynamic_cast<RelationshipConstraintPropertyMap const*> (relationMap.GetTargetECInstanceIdPropMap ()) != nullptr);
    idPropMap = static_cast<RelationshipConstraintPropertyMap const*> (relationMap.GetTargetECInstanceIdPropMap ());
    BeAssert(idPropMap->GetSingleColumn(contextTable, true) != nullptr);
    if (!idPropMap->IsVirtual())
        viewSql.AppendEscaped(contextTable.GetName().c_str()).AppendDot();

    idPropMap->AppendSelectClauseSqlSnippetForView (viewSql, contextTable);
    viewSql.AppendComma (true);

    //TargetECClassId--------------------------------------
    classIdPropMap = relationMap.GetTargetECClassIdPropMap ();
    if (!classIdPropMap->IsVirtual ())
        {
        if (relationMap._RequiresJoin(ECRelationshipEnd::ECRelationshipEnd_Target))
            viewSql.AppendEscaped (GetECClassIdPrimaryTableAlias (ECRelationshipEnd::ECRelationshipEnd_Target)).AppendDot ();
        else
            viewSql.AppendEscaped (classIdPropMap->GetSingleColumn ()->GetTable ().GetName ().c_str ()).AppendDot ();
        }

    AppendConstraintClassIdPropMap (viewSql, prepareContext, *classIdPropMap, ecdbMap, relationMap, relationMap.GetRelationshipClass ().GetTarget (), contextTable);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
//static
BentleyStatus ViewGenerator::AppendSystemPropMapsToNullView(NativeSqlBuilder& viewSql, ECSqlPrepareContext const& prepareContext, RelationshipClassMapCR relationMap, bool endWithComma)
    {
    //ECInstanceId and ECClassId
    auto sqlSnippets = relationMap.GetECInstanceIdPropertyMap()->ToNativeSql(nullptr, ECSqlType::Select, false);
    if (sqlSnippets.size() != 1)
        return ERROR;

    viewSql.Append("SELECT NULL ").Append(sqlSnippets).AppendComma(true);
    viewSql.Append("NULL ").Append(ECDB_COL_ECClassId).AppendComma(true);

    //Source constraint
    sqlSnippets = relationMap.GetSourceECInstanceIdPropMap()->ToNativeSql(nullptr, ECSqlType::Select, false);
    if (sqlSnippets.size() != 1)
        return ERROR;

    viewSql.Append("NULL ").Append(sqlSnippets).AppendComma(true);

    sqlSnippets = relationMap.GetSourceECClassIdPropMap()->ToNativeSql(nullptr, ECSqlType::Select, false);
    if (sqlSnippets.size() != 1)
        return ERROR;

    viewSql.Append("NULL ").Append(sqlSnippets).AppendComma(true);

    //Target constraint
    sqlSnippets = relationMap.GetTargetECInstanceIdPropMap()->ToNativeSql(nullptr, ECSqlType::Select, false);
    if (sqlSnippets.size() != 1)
        return ERROR;

    viewSql.Append("NULL ").Append(sqlSnippets).AppendComma(true);

    sqlSnippets = relationMap.GetTargetECClassIdPropMap()->ToNativeSql(nullptr, ECSqlType::Select, false);
    if (sqlSnippets.size() != 1)
        return ERROR;

    viewSql.Append("NULL ").Append(sqlSnippets);

    if (endWithComma)
        viewSql.AppendComma(true);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
//static
BentleyStatus ViewGenerator::AppendConstraintClassIdPropMap(NativeSqlBuilder& viewSql, ECSqlPrepareContext const& prepareContext, RelationshipConstraintPropertyMap const& propMap, ECDbMapCR ecdbMap, RelationshipClassMapCR relationMap, ECRelationshipConstraintCR constraint, ECDbSqlTable const& contextTable)
    {
    ECDbSqlColumn const* column = propMap.GetSingleColumn(contextTable, false);
    BeAssert(column != nullptr);
    if (column->GetPersistenceType() == PersistenceType::Virtual)
        {
        bool hasAnyClass = false;
        std::set<ClassMap const*> classMaps = ecdbMap.GetClassMapsFromRelationshipEnd(constraint,&hasAnyClass);
        BeAssert(!hasAnyClass);
        std::vector<ClassMap const*> relaventClassMaps;
        if (classMaps.size() > 1)
            {
            for (ClassMap const* classMap : classMaps)
                {
                if (classMap->IsMappedTo(contextTable))
                    relaventClassMaps.push_back(classMap);
                }

            if (relaventClassMaps.size() != 1)
                {
                BeAssert(false && "Expecting exactly one ClassMap at end");
                return BentleyStatus::ERROR;
                }
            }
        else
           relaventClassMaps.push_back(*classMaps.begin());

        ClassMap const* classMap = relaventClassMaps.front();
        BeAssert(classMap != nullptr);
        const ECClassId endClassId = classMap->GetClass().GetId();

        viewSql.Append(endClassId).AppendSpace();
        viewSql.Append(propMap.ToNativeSql(nullptr, ECSqlType::Select, false, &contextTable));
        }
    else
        propMap.AppendSelectClauseSqlSnippetForView(viewSql, contextTable);

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
            continue;

        viewClasses.insert (std::map<ECClassId, IClassMap const*>::value_type (ecClass->GetId (), classMap));

        if (!ecClass->GetDerivedClasses ().empty ())
            LoadDerivedClassMaps (viewClasses, map, classMap);
        }
    }

//static
void ViewGenerator::CreateSystemClassView(NativeSqlBuilder &viewSql, std::map<ECDbSqlTable const*, std::vector<IClassMap const*>> &tableMap, std::set<ECDbSqlTable const*> &tableToIncludeEntirly, bool forStructArray, ECSqlPrepareContext const& prepareContext)
    {
    bool first = true;
    for (auto& pair : tableMap)
        {
        std::vector<IClassMap const*>& classMaps = pair.second;
        ECDbSqlTable const* tableP = pair.first;

        bool includeEntireTable = tableToIncludeEntirly.find(tableP) != tableToIncludeEntirly.end();
        IClassMap const* classMap = classMaps[0];
        ECDbSqlColumn const* ecInstanceIdColumn = classMap->GetPropertyMap(ECInstanceIdPropertyMap::PROPERTYACCESSSTRING)->GetSingleColumn();
        ECDbSqlColumn const* ecClassIdColumn = nullptr;
        tableP->TryGetECClassIdColumn(ecClassIdColumn);

        if (tableP->GetPersistenceType() == PersistenceType::Virtual)
            continue;

        DbMetaDataHelper::ObjectType objectType = DbMetaDataHelper::GetObjectType(classMap->GetECDbMap().GetECDbR(), tableP->GetName().c_str());
        if (objectType == DbMetaDataHelper::ObjectType::None)
            continue;

        if (!first)
            viewSql.Append("\r\nUNION", true);

        viewSql.Append("SELECT", true);
        viewSql.Append(ecInstanceIdColumn->GetName().c_str(), true);
        if (first)
            viewSql.Append(ECDB_COL_ECInstanceId, true);

        viewSql.AppendComma(true);

        if (ecClassIdColumn)
            viewSql.Append(ecClassIdColumn->GetName().c_str(), true);
        else
            viewSql.Append(classMap->GetClass().GetId()).AppendSpace();

        if (first)
            viewSql.Append(ECDB_COL_ECClassId, true);

        if (forStructArray)
            viewSql.AppendComma(true).Append(ECDbSystemSchemaHelper::ECPROPERTYPATHID_PROPNAME).AppendComma(true).Append(ECDbSystemSchemaHelper::ECARRAYINDEX_PROPNAME);

        viewSql.AppendSpace().Append("FROM", true).AppendEscaped(tableP->GetName().c_str()).AppendSpace();

        bool wherePreset = false;
        if (!includeEntireTable && classMaps.size() > 1)
            {
            if (classMaps.size() == 1)
                viewSql.Append("WHERE " ECDB_COL_ECClassId "=").Append(classMap->GetClass().GetId());
            else
                {
                viewSql.Append("WHERE " ECDB_COL_ECClassId " IN (");
                for (auto itor = classMaps.begin(); itor != classMaps.end(); ++itor)
                    {
                    viewSql.Append((*itor)->GetClass().GetId());
                    if (itor != (classMaps.end() - 1))
                        viewSql.AppendComma(true);
                    }

                viewSql.AppendParenRight();
                }

            wherePreset = true;
            }

        if (classMap->MapsToStructArrayTable())
            {
            viewSql.AppendSpace();
            if (!wherePreset)
                viewSql.Append("WHERE", true);
            else
                viewSql.Append("AND", true);

            if (forStructArray)
                viewSql.Append("(ECPropertyPathId IS NOT NULL AND ECArrayIndex IS NOT NULL)");
            else
                viewSql.Append("(ECPropertyPathId IS NULL AND ECArrayIndex IS NULL)");
            }
        first = false;
        }
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
