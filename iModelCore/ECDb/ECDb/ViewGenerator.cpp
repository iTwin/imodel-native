/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ViewGenerator.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <set>
#include "SqlNames.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//************************** ViewGenerator ***************************************************

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2016
//+---------------+---------------+---------------+---------------+---------------+--------
//static 
BentleyStatus ViewGenerator::GenerateSelectViewSql(NativeSqlBuilder& viewSql, ECDb const& ecdb, ClassMap const& classMap, bool isPolymorphicQuery, ECSqlPrepareContext const& prepareContext)
    {
    ViewGenerator viewGenerator(ecdb);
    return viewGenerator.GenerateViewSql(viewSql, classMap, isPolymorphicQuery, &prepareContext);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2016
//+---------------+---------------+---------------+---------------+---------------+--------
//static 
BentleyStatus ViewGenerator::CreateUpdatableViews(ECDbCR ecdb)
    {
    if (ecdb.IsReadonly())
        {
        ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Can only call ECDb::CreateECClassViewsInDb() on an ECDb file with read-write access.");
        return ERROR;
        }

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb,
                                     "SELECT c.Id FROM ec_Class c, ec_ClassMap cm, ec_ClassHasBaseClasses cc "
                                     "WHERE c.Id = cm.ClassId AND c.Id = cc.BaseClassId AND c.Type = " SQLVAL_ECClassType_Entity " AND cm.MapStrategy<> " SQLVAL_MapStrategy_NotMapped
                                     " GROUP BY c.Id"))
        return ERROR;

    std::vector<ClassMapCP> classMaps;
    ECDbMap const& map = ecdb.Schemas().GetDbMap();
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ECClassId classId = stmt.GetValueId<ECClassId>(0);
        ECClassCP ecClass = ecdb.Schemas().GetECClass(classId);
        if (ecClass == nullptr)
            return ERROR;

        ClassMapCP classMap = map.GetClassMap(*ecClass);
        if (classMap == nullptr)
            {
            BeAssert(classMap != nullptr);
            return ERROR;
            }

        BeAssert(classMap->GetClass().IsEntityClass() && classMap->GetType() != ClassMap::Type::NotMapped);
        if (CreateUpdatableViewIfRequired(ecdb, *classMap) != SUCCESS)
            return ERROR;
        }

    return SUCCESS;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2016
//+---------------+---------------+---------------+---------------+---------------+--------
//static 
BentleyStatus ViewGenerator::DropUpdatableViews(ECDbCR ecdb)
    {
    Statement stmt;
    stmt.Prepare(ecdb,
                 "SELECT ('DROP VIEW IF EXISTS _' || ec_Schema.Alias || '_' || ec_Class.Name) "
                 "FROM ec_Class INNER JOIN ec_Schema ON ec_Schema.Id = ec_Class.SchemaId");

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        Utf8CP updatableViewSQL = stmt.GetValueText(0);
        if (ecdb.ExecuteSql(updatableViewSQL) != BE_SQLITE_OK)
            {
            BeAssert(false && "Failed to drop updatable view");
            return ERROR;
            }
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2016
//+---------------+---------------+---------------+---------------+---------------+--------
//static 
BentleyStatus ViewGenerator::CreateECClassViews(ECDbCR ecdb)
    {
    if (ecdb.IsReadonly())
        {
        ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Can only call ECDb::CreateECClassViewsInDb() on an ECDb file with read-write access.");
        return ERROR;
        }

    if (DropECClassViews(ecdb) != SUCCESS)
        return ERROR;

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, 
                                     "SELECT c.Id FROM ec_Class c, ec_ClassMap cm WHERE c.Id = cm.ClassId AND "
                                     "c.Type IN (" SQLVAL_ECClassType_Entity "," SQLVAL_ECClassType_Relationship ") AND "
                                     "cm.MapStrategy<>" SQLVAL_MapStrategy_NotMapped))
        return ERROR;

    std::vector<ClassMapCP> classMaps;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ECClassId classId = stmt.GetValueId<ECClassId>(0);
        ECClassCP ecClass = ecdb.Schemas().GetECClass(classId);
        if (ecClass == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }

        ClassMapCP classMap = ecdb.Schemas().GetDbMap().GetClassMap(*ecClass);
        if (classMap == nullptr)
            {
            BeAssert(classMap != nullptr);
            return ERROR;
            }

        BeAssert((classMap->GetClass().IsEntityClass() || classMap->GetClass().IsRelationshipClass()) && classMap->GetType() != ClassMap::Type::NotMapped);
        if (CreateECClassView(ecdb, *classMap) != SUCCESS)
            return ERROR;
        }

    return SUCCESS;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2016
//+---------------+---------------+---------------+---------------+---------------+--------
//static 
BentleyStatus ViewGenerator::CreateECClassView(ECDbCR ecdb, ClassMapCR classMap)
    {
    Utf8String viewName;
    viewName.Sprintf("[%s.%s]", classMap.GetClass().GetSchema().GetAlias().c_str(), classMap.GetClass().GetName().c_str());

    ViewGenerator viewGenerator(ecdb, true, false);
    NativeSqlBuilder viewSql;
    if (viewGenerator.GenerateViewSql(viewSql, classMap, true, nullptr) != SUCCESS)
        return ERROR;

    Utf8String columns;
    bool bFirst = true;
    std::vector<Utf8String> const& accessStringList = *viewGenerator.m_viewAccessStringList;
    for (Utf8StringCR column : accessStringList)
        {
        if (bFirst)
            bFirst = false;
        else
            columns.append(", ");

        columns.append("[").append(column).append("]");
        }

    Utf8String createViewSql;
    createViewSql.Sprintf("CREATE VIEW %s (%s)\n\t--### ECCLASS VIEW is for debugging purpose only!.\n\tAS %s;", viewName.c_str(), columns.c_str(), viewSql.ToString());
    if (ecdb.ExecuteSql(createViewSql.c_str()) != BE_SQLITE_OK)
        return ERROR;

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2016
//+---------------+---------------+---------------+---------------+---------------+--------
//static 
BentleyStatus ViewGenerator::DropECClassViews(ECDbCR ecdb)
    {
    Statement stmt;
    stmt.Prepare(ecdb,
                 "SELECT ('DROP VIEW IF EXISTS [' || ec_Schema.Alias || '.' || ec_Class.Name || '];') FROM ec_Class "
                 "INNER JOIN ec_Schema ON ec_Schema.Id = ec_Class.SchemaId");

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        Utf8CP classViewSQL = stmt.GetValueText(0);
        if (ecdb.ExecuteSql(classViewSQL) != BE_SQLITE_OK)
            {
            BeAssert(false && "Failed to drop ECClass view");
            return ERROR;
            }
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2016
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ViewGenerator::GenerateUpdateTriggerSetClause(NativeSqlBuilder& sql, ClassMap const& baseClassMap, ClassMap const& derivedClassMap)
    {
    sql.Reset();
    std::vector<Utf8String> values;
    WipPropertyMapTypeDispatcher typeDispatcher(PropertyMapKind::Business); //Only inlcude none-system properties
    baseClassMap.GetPropertyMaps().Accept(typeDispatcher);
    for (WipPropertyMap const* result: typeDispatcher.ResultSet())
        {
 
        WipVerticalPropertyMap const* derivedPropertyMap = static_cast<WipVerticalPropertyMap const*> (derivedClassMap.GetPropertyMaps().Find(result->GetAccessString().c_str()));
        if (derivedPropertyMap == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }

        std::vector<DbColumn const*> derivedColumnList, baseColumnList;
        if (result->GetKind() == PropertyMapKind::NavigationPropertyMap)
            {
            if (!static_cast<WipNavigationPropertyMap const&>(*result).IsSupportedInECSql())
                return ERROR;
            }

        WipPropertyMapColumnDispatcher baseColumnDispatcher, derivedColumnDispatcher;
        result->Accept(baseColumnDispatcher);
        derivedPropertyMap->Accept(derivedColumnDispatcher);
        if (baseColumnDispatcher.GetColumns().size() != derivedColumnDispatcher.GetColumns().size())
            {
            BeAssert(false);
            return ERROR;
            }

        for (auto deriveColumnItor = derivedColumnDispatcher.GetColumns().begin(), baseColumnItor = baseColumnDispatcher.GetColumns().begin(); deriveColumnItor != derivedColumnDispatcher.GetColumns().end() && baseColumnItor != baseColumnDispatcher.GetColumns().end(); ++deriveColumnItor, ++baseColumnItor)
            {
            Utf8String str;
            str.Sprintf("[%s] = NEW.[%s]", (*deriveColumnItor)->GetName().c_str(), (*baseColumnItor)->GetName().c_str());
            values.push_back(str);
            }
        }
    

    if (values.empty())
        return ERROR;

    for (auto itor = values.begin(); itor != values.end(); ++itor)
        {
        if (itor != values.begin())
            sql.AppendComma();

        sql.Append((*itor).c_str());
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2016
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateUpdatableViewIfRequired(ECDbCR ecdb, ClassMap const& classMap)
    {
    if (classMap.GetMapStrategy().GetStrategy() == MapStrategy::NotMapped || classMap.IsRelationshipClassMap())
        return ERROR;

    ECDbMap const& ecdbMap = ecdb.Schemas().GetDbMap();
    StorageDescription const& descr = classMap.GetStorageDescription();
    std::vector<Partition> const& partitions = descr.GetHorizontalPartitions();
    Partition const& rootPartition = classMap.GetStorageDescription().GetRootHorizontalPartition();
    DbColumn const* rootPartitionIdColumn = rootPartition.GetTable().GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);

    Utf8String updatableViewName;
    updatableViewName.Sprintf("[%s]", classMap.GetUpdatableViewName().c_str());

    std::vector<Utf8String> triggerDdlList;

    std::set<DbTable const*> updateTables;
    std::set<DbTable const*> deleteTables;
    std::vector<DbTable const*> joinedTables;
    std::vector<DbTable const*> primaryTables;

    for (Partition const& partition : partitions)
        {
        if (partition.GetTable().GetPersistenceType() == PersistenceType::Virtual)
            continue;

        updateTables.insert(&partition.GetTable());
        deleteTables.insert(&partition.GetTable());
        if (partition.GetTable().GetType() == DbTable::Type::Joined)
            {
            joinedTables.push_back(&partition.GetTable());
            }

        if (partition.GetTable().GetType() == DbTable::Type::Primary)
            {
            primaryTables.push_back(&partition.GetTable());
            }
        }
    //Remove any primary table
    for (DbTable const* joinedTable : joinedTables)
        {
        updateTables.erase(joinedTable->GetParentOfJoinedTable());
        }

    for (DbTable const* joinedTable : joinedTables)
        {
        deleteTables.insert(joinedTable->GetParentOfJoinedTable());
        deleteTables.erase(joinedTable);
        }

    int tableCount = 0;
    for (Partition const& partition : partitions)
        {
        if (partition.GetTable().GetPersistenceType() == PersistenceType::Virtual)
            continue;

        tableCount++;
        DbColumn const* partitionIdColumn = partition.GetTable().GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
        Utf8String triggerNamePrefix;
        triggerNamePrefix.Sprintf("%s_%s", rootPartition.GetTable().GetName().c_str(), partition.GetTable().GetName().c_str());

        Utf8String whenClause;
        if (partition.NeedsECClassIdFilter())
            partition.AppendECClassIdFilterSql(whenClause, "OLD.ECClassId");
        else
            whenClause.append("OLD.ECClassId=").append(partition.GetRootClassId().ToString());

        if (deleteTables.find(&partition.GetTable()) != deleteTables.end())
            {//<----------DELETE trigger----------
            Utf8String ddl("CREATE TRIGGER [");
            ddl.append(triggerNamePrefix).append("_delete]");
            ddl.append(" INSTEAD OF DELETE ON ").append(updatableViewName).append(" WHEN ").append(whenClause);

            Utf8String body;
            body.Sprintf(" BEGIN DELETE FROM [%s] WHERE [%s] = OLD.[%s]; END", partition.GetTable().GetName().c_str(), partitionIdColumn->GetName().c_str(), rootPartitionIdColumn->GetName().c_str());
            ddl.append(body);
            triggerDdlList.push_back(ddl);
            }

        if (updateTables.find(&partition.GetTable()) != updateTables.end())
            {//<----------UPDATE trigger----------
            ECClassCP rootClass = ecdb.Schemas().GetECClass(partition.GetRootClassId());
            if (rootClass == nullptr)
                {
                BeAssert(false);
                return ERROR;
                }

            ClassMapCP derviedClassMap = ecdbMap.GetClassMap(*rootClass);
            if (derviedClassMap == nullptr)
                {
                BeAssert(false && "ClassMap not found");
                return ERROR;
                }

            Utf8String ddl("CREATE TRIGGER [");
            ddl.append(triggerNamePrefix).append("_update]");

            ddl.append(" INSTEAD OF UPDATE ON ").append(updatableViewName).append(" WHEN ").append(whenClause);

            NativeSqlBuilder setClause;
            if (SUCCESS != GenerateUpdateTriggerSetClause(setClause, classMap, *derviedClassMap))
                continue; //nothing to update.

            Utf8String body;
            body.Sprintf(" BEGIN UPDATE [%s] SET %s WHERE [%s] = OLD.[%s]; END", partition.GetTable().GetName().c_str(), setClause.ToString(), partitionIdColumn->GetName().c_str(), rootPartitionIdColumn->GetName().c_str());
            ddl.append(body);

            triggerDdlList.push_back(ddl);
            }
        }

    if (tableCount < 2)
        return SUCCESS;

    ViewGenerator generator(ecdb, false, false);
    NativeSqlBuilder viewBodySql;
    if (generator.GenerateViewSql(viewBodySql, classMap, true, nullptr) != SUCCESS)
        return ERROR;
    
    Utf8String updatableViewDdl;
    updatableViewDdl.Sprintf("CREATE VIEW %s AS %s", updatableViewName.c_str(), viewBodySql.ToString());

    if (ecdb.ExecuteSql(updatableViewDdl.c_str()) != BE_SQLITE_OK)
        return ERROR;

    for (Utf8StringCR triggerDdl : triggerDdlList)
        {
        if (ecdb.ExecuteSql(triggerDdl.c_str()) != BE_SQLITE_OK)
            return ERROR;
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ViewGenerator::GenerateViewSql(NativeSqlBuilder& viewSql, ClassMap const& classMap, bool isPolymorphicQuery, ECSqlPrepareContext const* prepareContext)
    {
    m_isPolymorphic = isPolymorphicQuery;
    m_prepareContext = prepareContext;
    m_captureViewAccessStringList = true;
    if (classMap.GetMapStrategy().GetStrategy() == MapStrategy::NotMapped)
        {
        BeAssert(false && "ViewGenerator::CreateView must not be called on unmapped class");
        return ERROR;
        }

    //isPolymorphic is not implemented. By default all query are polymorphic
    if (classMap.IsRelationshipClassMap())
        return CreateViewForRelationship(viewSql, classMap);

    if (m_asSubQuery)
        viewSql.AppendParenLeft();

    DbSchema::EntityType entityType = DbSchema::GetEntityType(m_ecdb, classMap.GetPrimaryTable().GetName().c_str());
    if (entityType == DbSchema::EntityType::None && !isPolymorphicQuery)
        {
        if (SUCCESS != CreateNullView(viewSql, classMap))
            return ERROR;

        if (m_asSubQuery)
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

        std::vector<ClassMap const*> rootClassMaps;
        if (SUCCESS != GetRootClasses(rootClassMaps))
            return ERROR;

        for (ClassMap const* classMap : rootClassMaps)
            {
            if (SUCCESS != ComputeViewMembers(viewMembers, classMap->GetClass(), /*ensureDerivedClassesAreLoaded=*/ false))
                return ERROR;
            }
        }
    else
        {
        if (SUCCESS != ComputeViewMembers(viewMembers, classMap.GetClass(), /*ensureDerivedClassesAreLoaded=*/ true))
            return ERROR;
        }

    int queriesAddedToUnion = 0;
    for (auto& pvm : viewMembers)
        {
        if (m_optimizeByIncludingOnlyRealTables && pvm.second.GetStorageType() == DbSchema::EntityType::None)
            continue;

        if (queriesAddedToUnion > 0)
            viewSql.Append(" UNION ");

        if (SUCCESS != GetViewQueryForChild(viewSql, *pvm.first, pvm.second.GetClassMaps(), classMap))
            return ERROR;

        queriesAddedToUnion++;
        }

    if (queriesAddedToUnion == 0)
        {
        if (SUCCESS != CreateNullView(viewSql, classMap))
            return ERROR;

        if (m_asSubQuery)
            viewSql.AppendParenRight();

        return SUCCESS;
        }
    else
        {
        if (m_asSubQuery)
            viewSql.AppendParenRight();
        }
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ViewGenerator::CreateNullView(NativeSqlBuilder& viewSql, ClassMap const& classMap)
    {
    viewSql.Append("SELECT ");

    std::vector<std::pair<WipPropertyMap const*, WipPropertyMap const*>> viewPropMaps;
    if (SUCCESS != GetPropertyMapsOfDerivedClassCastAsBaseClass(viewPropMaps, classMap, classMap, false))
        return ERROR;

    AppendViewPropMapsToQuery(viewSql, classMap.GetJoinedTable(), viewPropMaps, true /*forNullView*/);
    viewSql.Append(" LIMIT 0");
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ViewGenerator::GetRootClasses(std::vector<ClassMap const*>& rootClasses) const
    {
    bvector<ECN::ECSchemaCP> schemas = m_ecdb.Schemas().GetECSchemas(true);
    if (schemas.empty())
        return ERROR;

    std::vector<ClassMap const*> rootClassMaps;
    for (ECSchemaCP schema : schemas)
        {
        if (schema->IsStandardSchema())
            continue;

        for (ECClassCP ecClass : schema->GetClasses())
            {
            if (ecClass->GetDerivedClasses().empty())
                {
                ClassMap const* classMap = m_ecdb.Schemas().GetDbMap().GetClassMap(*ecClass);
                if (classMap == nullptr)
                    {
                    BeAssert(classMap != nullptr);
                    return ERROR;
                    }

                if (classMap->GetType() == ClassMap::Type::NotMapped)
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
BentleyStatus ViewGenerator::ComputeViewMembers(ViewMemberByTable& viewMembers, ECClassCR ecClass, bool ensureDerivedClassesAreLoaded)
    {
    ClassMap const* classMap = m_ecdb.Schemas().GetDbMap().GetClassMap(ecClass);
    if (classMap == nullptr || classMap->GetType() == ClassMap::Type::NotMapped)
        return SUCCESS;

    if (classMap->GetJoinedTable().GetColumns().empty())
        return SUCCESS;

    auto itor = viewMembers.find(&classMap->GetJoinedTable());
    if (itor == viewMembers.end())
        {
        DbSchema::EntityType storageType = DbSchema::EntityType::Table;
        if (m_optimizeByIncludingOnlyRealTables)
            {
            //This is a db query so optimization comes at a cost
            storageType = DbSchema::GetEntityType(m_ecdb, classMap->GetJoinedTable().GetName().c_str());
            }

        if (storageType == DbSchema::EntityType::Table)
            viewMembers.insert(ViewMemberByTable::value_type(&classMap->GetJoinedTable(), ViewMember(storageType, *classMap)));
        }
    else
        {
        if (m_optimizeByIncludingOnlyRealTables)
            {
            if (itor->second.GetStorageType() == DbSchema::EntityType::Table)
                itor->second.AddClassMap(*classMap);
            }
        else
            itor->second.AddClassMap(*classMap);
        }

    if (!m_isPolymorphic ||
        (classMap->GetMapStrategy().IsTablePerHierarchy() && classMap->GetTphHelper()->IsParentOfJoinedTable()) ||
        (!classMap->IsRelationshipClassMap() && classMap->GetMapStrategy().IsTablePerHierarchy()))
        return SUCCESS;

    ECDerivedClassesList const& derivedClasses = ensureDerivedClassesAreLoaded ? m_ecdb.Schemas().GetDerivedECClasses(ecClass) : ecClass.GetDerivedClasses();
    for (ECClassCP derivedClass : derivedClasses)
        {
        if (SUCCESS != ComputeViewMembers(viewMembers, *derivedClass, ensureDerivedClassesAreLoaded))
            return ERROR;
        }

    return SUCCESS;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::GetPropertyMapsOfDerivedClassCastAsBaseClass(std::vector<std::pair<WipPropertyMap const*, WipPropertyMap const*>>& propMaps, ClassMap const& baseClassMap, ClassMap const& childClassMap, bool skipSystemProperties)
    {
    propMaps.clear();
    WipPropertyMapTypeDispatcher typeDispatcher(PropertyMapKind::All, true /*traverse compound properties*/);
    baseClassMap.GetPropertyMaps().Accept(typeDispatcher);

    for (WipPropertyMap const* baseClassPropertyMap : typeDispatcher.ResultSet())
        {
        if (skipSystemProperties && baseClassPropertyMap->IsSystem())
            continue;

        if(m_prepareContext && !m_prepareContext->GetSelectionOptions().IsSelected(baseClassPropertyMap->GetAccessString().c_str()))
            continue;

        //This is compound property and we will get its components instead of parent property. So to call IsSupportedInECSql we must call its parent.
        if (baseClassPropertyMap->IsKindOf(PropertyMapKind::NavigationPropertyMap) &&
            !static_cast<WipNavigationPropertyMap const*>(baseClassPropertyMap->GetParent())->IsSupportedInECSql())
            continue;

        WipPropertyMap const* childClassCounterpartPropMap = childClassMap.GetPropertyMaps().Find(baseClassPropertyMap->GetAccessString().c_str());
        if (childClassCounterpartPropMap == nullptr)
            return ERROR;

        propMaps.push_back({baseClassPropertyMap, childClassCounterpartPropMap});
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::AppendViewPropMapsToQuery(NativeSqlBuilder& viewSql, DbTable const& table, std::vector<std::pair<WipPropertyMap const*, WipPropertyMap const*>> const& viewPropMaps, bool forNullView)
    {
    bool second = false;
    for (auto const& propMapPair : viewPropMaps)
        {
        WipPropertyMap const* basePropMap = propMapPair.first;
        WipPropertyMap const* actualPropMap = propMapPair.second;

        //We are not expecting 
        BeAssert(dynamic_cast<WipCompoundPropertyMap const*>(basePropMap) == nullptr);
        BeAssert(dynamic_cast<WipCompoundPropertyMap const*>(actualPropMap) == nullptr);


        if (m_prepareContext && !m_prepareContext->GetSelectionOptions().IsSelected(actualPropMap->GetAccessString().c_str()))
            continue;

        DbTable const& basePropMapTable = basePropMap->GetClassMap().GetJoinedTable();
        DbTable const& actualPropMapTable = actualPropMap->GetClassMap().GetJoinedTable();
        WipPropertyMapSqlDispatcher baseSqlDispatcher(basePropMapTable, WipPropertyMapSqlDispatcher::SqlTarget::Table, nullptr);
        WipPropertyMapSqlDispatcher actualSqlDispatcher(actualPropMapTable, WipPropertyMapSqlDispatcher::SqlTarget::Table, actualPropMapTable.GetName().c_str());
        WipPropertyMapSqlDispatcher actualSqlDispatcherWithTableName(actualPropMapTable, WipPropertyMapSqlDispatcher::SqlTarget::Table, nullptr);

        basePropMap->Accept(baseSqlDispatcher);
        actualPropMap->Accept(actualSqlDispatcher);
        actualPropMap->Accept(actualSqlDispatcherWithTableName);
        

        const bool generateECClassView = m_viewAccessStringList && m_captureViewAccessStringList;
        const size_t snippetCount = actualSqlDispatcher.GetResultSet().size();
        if (baseSqlDispatcher.GetResultSet().size() != snippetCount && snippetCount == 1LL)
            {
            BeAssert(false && "Number of alias SQL snippets is expected to be the same as number of column SQL snippets.");
            return ERROR;
            }

        const WipPropertyMapSqlDispatcher::Result& baseResult = baseSqlDispatcher.GetResultSet().front();
        const WipPropertyMapSqlDispatcher::Result& actualResult = actualSqlDispatcher.GetResultSet().front();
        const WipPropertyMapSqlDispatcher::Result& actualResultWithoutTable = actualSqlDispatcherWithTableName.GetResultSet().front();

        if (WipECClassIdPropertyMap const* baseECClassIdPropertyMap = dynamic_cast<WipECClassIdPropertyMap const*>(basePropMap))
            {
            if (generateECClassView)
                m_viewAccessStringList->push_back(basePropMap->GetAccessString());

            WipColumnVerticalPropertyMap const* baseVMap = baseECClassIdPropertyMap->FindVerticalPropertyMap(basePropMapTable);
            if (baseVMap == nullptr)
                {
                BeAssert(false);
                return ERROR;
                }

            if (second)
                viewSql.AppendComma();
            else
                second = true;

            if (forNullView)
                {
                viewSql.Append("NULL").AppendSpace().Append(baseVMap->GetColumn().GetName().c_str());
                }
            else
                {
                WipECClassIdPropertyMap const* actualECClassIdPropertyMap = static_cast<WipECClassIdPropertyMap  const*>(actualPropMap);
                WipColumnVerticalPropertyMap const* actuallVMap = actualECClassIdPropertyMap->FindVerticalPropertyMap(actualPropMapTable);
                if (actuallVMap == nullptr)
                    {
                    BeAssert(false);
                    return ERROR;
                    }

                if (actuallVMap->GetColumn().GetPersistenceType() == PersistenceType::Persisted)
                    {
                    viewSql.AppendEscaped(actuallVMap->GetColumn().GetTable().GetName().c_str()).AppendDot().AppendEscaped(actuallVMap->GetColumn().GetName().c_str());
                    }
                else
                    {
                    Utf8Char classIdStr[ECClassId::ID_STRINGBUFFER_LENGTH];
                    actualECClassIdPropertyMap->GetDefaultECClassId().ToString(classIdStr);
                    viewSql.Append(classIdStr).AppendSpace().Append(COL_ECClassId);
                    }
                }
            }
        else
            {
            if (second)
                viewSql.AppendComma();
            else
                second = true;

            Utf8CP aliasSqlSnippet = baseResult.GetSql();
            if (forNullView)
                viewSql.Append("NULL ");
            else
                {
                if (generateECClassView)
                    {

                    if (!actualResult.GetPropertyMap().GetColumn().IsShared())
                        viewSql.Append(actualResult.GetSql());
                    else
                        {
                        const DbColumn::Type colType = DbColumn::PrimitiveTypeToColumnType(actualResult.GetPropertyMap().GetProperty().GetAsPrimitiveProperty()->GetType());
                        viewSql.Append("CAST (");
                        viewSql.Append(actualResult.GetSql());
                        viewSql.Append(" AS ").Append(DbColumn::TypeToSql(colType));
                        viewSql.Append(")");
                        
                        }
                    }
                else
                    {
                    viewSql.Append(actualResult.GetSql());
                    }
                }

            if (strcmp(actualResultWithoutTable.GetSql(), aliasSqlSnippet) != 0 || forNullView) //do not add alias if column name is same as alias.
                viewSql.AppendSpace().Append(aliasSqlSnippet);

            if (generateECClassView)
                {
                m_viewAccessStringList->push_back(baseResult.GetPropertyMap().GetAccessString());
                }
            }
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      07/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::GetViewQueryForChild(NativeSqlBuilder& viewSql, DbTable const& table, const std::vector<ClassMap const*>& childClassMap, ClassMap const& baseClassMap)
    {
    if (childClassMap.empty() || table.GetColumns().empty())
        {
        BeAssert(false);
        return ERROR;
        }

    ClassMap const* firstChildClassMap = childClassMap.front();
    //Generate Select statement
    viewSql.Append("SELECT ");
    std::vector<std::pair<WipPropertyMap const*, WipPropertyMap const*>> viewPropMaps;
    auto status = GetPropertyMapsOfDerivedClassCastAsBaseClass(viewPropMaps, baseClassMap, *firstChildClassMap, false);
    if (status != BentleyStatus::SUCCESS)
        return status;

    //Append prop m_map columns to query [col1],[col2], ...
    AppendViewPropMapsToQuery(viewSql, table, viewPropMaps);

    //Determine which table to join for split table case
    std::set<DbTable const*> tableToJoinOn;
    for (auto const& propMapPair : viewPropMaps)
        {
        auto actualPropMap = propMapPair.second;
        if (m_prepareContext && !m_prepareContext->GetSelectionOptions().IsSelected(actualPropMap->GetAccessString().c_str()))
            continue;
        
        if (actualPropMap->IsSystem())
            continue;

        if (WipVerticalPropertyMap const* v = dynamic_cast<WipColumnVerticalPropertyMap const*>(actualPropMap))
            {
            tableToJoinOn.insert(&v->GetTable());
            }
        else
            {
            BeAssert(false && "Programmer Error");
            }
        }

    viewSql.Append(" FROM ").AppendEscaped(table.GetName().c_str());
    //Join necessary table for table
    auto primaryKey = table.GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
    for (auto const& vpart : firstChildClassMap->GetStorageDescription().GetVerticalPartitions())
        {
        bool tableReferencedInQuery = tableToJoinOn.find(&vpart.GetTable()) != tableToJoinOn.end();
        bool notYetReferenced = &vpart.GetTable() != &table;
        if (tableReferencedInQuery && notYetReferenced)
            {
            auto fkKey = vpart.GetTable().GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
            viewSql.Append(" INNER JOIN ").AppendEscaped(vpart.GetTable().GetName().c_str());
            viewSql.Append(" ON ").AppendEscaped(table.GetName().c_str()).AppendDot().AppendEscaped(primaryKey->GetName().c_str());
            viewSql.Append(" = ").AppendEscaped(vpart.GetTable().GetName().c_str()).AppendDot().AppendEscaped(fkKey->GetName().c_str());
            }
        }


    Utf8String where;
    WipColumnVerticalPropertyMap const* ecClassIdPropertyMap = firstChildClassMap->GetECClassIdPropertyMap()->FindVerticalPropertyMap(table);
    if (ecClassIdPropertyMap == nullptr)
        {
        BeAssert(ecClassIdPropertyMap != nullptr);
        return ERROR;
        }

    if (ecClassIdPropertyMap->GetColumn().GetPersistenceType() == PersistenceType::Persisted)
        {
        auto tableP = &firstChildClassMap->GetJoinedTable();
        bool noClassIdFilterOption = false;
        if (m_prepareContext)
            {
            if (OptionsExp const* options = m_prepareContext->GetCurrentScope().GetOptions())
                noClassIdFilterOption = options->HasOption(OptionsExp::NOECCLASSIDFILTER_OPTION);
            }

        if (!noClassIdFilterOption)
            {
            if (SUCCESS != baseClassMap.GetStorageDescription().GenerateECClassIdFilter(where, *tableP, ecClassIdPropertyMap->GetColumn(), m_isPolymorphic, true, tableP->GetName().c_str()))
                return ERROR;
            }
        }

    if (!where.empty())
        viewSql.Append(" WHERE ").Append(where.c_str());

    if (m_viewAccessStringList)
        m_captureViewAccessStringList = false; //stop viewAccessString capture;

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateNullViewForRelationshipClassEndTableMap(NativeSqlBuilder& viewSql, RelationshipClassMapCR relationMap, ClassMap const& baseClassMap)
    {
    viewSql.AppendParenLeft();
    AppendSystemPropMapsToNullView(viewSql, relationMap, false /*endWithComma*/);
    viewSql.Append(" LIMIT 0").AppendParenRight();
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateNullViewForRelationshipClassLinkTableMap(NativeSqlBuilder& viewSql, RelationshipClassMapCR relationMap, ClassMap const& baseClassMap)
    {
    viewSql.AppendParenLeft();
    AppendSystemPropMapsToNullView(viewSql, relationMap, false /*endWithComma*/);

    //! Only link table mapped relationship properties are persisted
    std::vector<std::pair<WipPropertyMap const*, WipPropertyMap const*>> viewPropMaps;
    if (SUCCESS != GetPropertyMapsOfDerivedClassCastAsBaseClass(viewPropMaps, baseClassMap, relationMap, true))
        return ERROR;

    //Append columns to query [col1],[col2], ...
    AppendViewPropMapsToQuery(viewSql, relationMap.GetJoinedTable(), viewPropMaps, true);
    viewSql.AppendParenRight();
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateViewForRelationshipClassLinkTableMap(NativeSqlBuilder& viewSql, RelationshipClassMapCR relationMap, ClassMap const& baseClassMap)
    {
    viewSql.Append("SELECT ");
    AppendSystemPropMaps(viewSql, relationMap, relationMap.GetPrimaryTable());

    //! Only link table mapped relationship properties are persisted
    std::vector<std::pair<WipPropertyMap const*, WipPropertyMap const*>> viewPropMaps;
    if (SUCCESS != GetPropertyMapsOfDerivedClassCastAsBaseClass(viewPropMaps, baseClassMap, relationMap, true))
        return ERROR;

    if (!viewPropMaps.empty())
        {
        viewSql.AppendComma();
        //Append prop maps' columns to query [col1],[col2], ...
        AppendViewPropMapsToQuery(viewSql, relationMap.GetJoinedTable(), viewPropMaps);
        }
    viewSql.Append(" FROM ").AppendEscaped(relationMap.GetJoinedTable().GetName().c_str());

    //Append secondary table JOIN
    if (SUCCESS != BuildRelationshipJoinIfAny(viewSql, relationMap, ECN::ECRelationshipEnd::ECRelationshipEnd_Source, relationMap.GetPrimaryTable()))
        return ERROR;

    if (m_viewAccessStringList)
        m_captureViewAccessStringList = false; //stop viewAccessString capture;

    return BuildRelationshipJoinIfAny(viewSql, relationMap, ECN::ECRelationshipEnd::ECRelationshipEnd_Target, relationMap.GetPrimaryTable());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateViewForRelationshipClassEndTableMap(NativeSqlBuilder& viewSql, RelationshipClassEndTableMap const& relationMap, ClassMap const& baseClassMap)
    {
    //ECInstanceId, ECClassId of the relationship instance
    
    NativeSqlBuilder::List builders;
    bool first = true;
    for (DbTable const* table : relationMap.GetECInstanceIdPropertyMap()->GetTables())
        {
        if (table->GetPersistenceType() == PersistenceType::Virtual)
            continue;

        NativeSqlBuilder selectSQL;
        selectSQL.Append("SELECT ");
        AppendSystemPropMaps(selectSQL, relationMap, *table);
        selectSQL.Append(" FROM ").AppendEscaped(table->GetName().c_str());

        //Append secondary table JOIN
        if (SUCCESS != BuildRelationshipJoinIfAny(selectSQL, relationMap, ECN::ECRelationshipEnd::ECRelationshipEnd_Source, *table))
            return ERROR;

        if (SUCCESS != BuildRelationshipJoinIfAny(selectSQL, relationMap, ECN::ECRelationshipEnd::ECRelationshipEnd_Target, *table))
            return ERROR;

        WipPropertyMapSqlDispatcher sqlDispatcher(*table, WipPropertyMapSqlDispatcher::SqlTarget::Table, nullptr);
        relationMap.GetReferencedEndECInstanceIdPropMap()->Accept(sqlDispatcher);
        
        selectSQL.Append(" WHERE ").Append(sqlDispatcher.GetResultSet().front().GetSql()).Append(" IS NOT NULL");
        if (first)
            first = false;
        else
            {
            viewSql.Append(" UNION ");
            }

        viewSql.Append(selectSQL);
        }


    if (m_viewAccessStringList)
        m_captureViewAccessStringList = false; //stop viewAccessString capture;

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      07/2015
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::BuildRelationshipJoinIfAny(NativeSqlBuilder& sqlBuilder, RelationshipClassMapCR classMap, ECN::ECRelationshipEnd endPoint, DbTable const& contextTable)
    {
    if (classMap._RequiresJoin(endPoint))
        {
        WipConstraintECClassIdPropertyMap const* ecclassIdPropertyMap = endPoint == ECRelationshipEnd::ECRelationshipEnd_Source ? classMap.GetSourceECClassIdPropMap() : classMap.GetTargetECClassIdPropMap();
        WipConstraintECInstanceIdPropertyMap const* ecInstanceIdPropertyMap = endPoint == ECRelationshipEnd::ECRelationshipEnd_Source ? classMap.GetSourceECInstanceIdPropMap() : classMap.GetTargetECInstanceIdPropMap();
        size_t tableCount = m_ecdb.Schemas().GetDbMap().GetTableCountOnRelationshipEnd(endPoint == ECRelationshipEnd::ECRelationshipEnd_Source ? classMap.GetRelationshipClass().GetSource() : classMap.GetRelationshipClass().GetTarget());

        if (ecclassIdPropertyMap->GetTables().size() != 1LL)
            {
            BeAssert(ecclassIdPropertyMap->GetTables().size() == 1LL);
            return ERROR;
            }

        DbTable const* targetTable = ecclassIdPropertyMap->GetTables().front();
        if (tableCount > 1
            /*In this case we expecting we have relationship with one end abstract we only support it in case joinedTable*/)
            {
            BeAssert(targetTable->GetType() == DbTable::Type::Joined && targetTable->GetParentOfJoinedTable() != nullptr);
            if (targetTable->GetType() != DbTable::Type::Joined)
                return ERROR;
            }

        sqlBuilder.Append(" INNER JOIN ");
        sqlBuilder.AppendEscaped(targetTable->GetName().c_str());
        sqlBuilder.AppendSpace();
        sqlBuilder.Append(GetECClassIdPrimaryTableAlias(endPoint));
        sqlBuilder.Append(" ON ");
        sqlBuilder.Append(GetECClassIdPrimaryTableAlias(endPoint));
        sqlBuilder.AppendDot();
        DbColumn const* targetECInstanceIdColumn = targetTable->GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
        if (targetECInstanceIdColumn == nullptr)
            {
            BeAssert(false && "Failed to find ECInstanceId column in target table");
            return ERROR;
            }

        sqlBuilder.AppendEscaped(targetECInstanceIdColumn->GetName().c_str());
        sqlBuilder.Append(BooleanSqlOperator::EqualTo);   
        if (ecInstanceIdPropertyMap->GetTables().size() != 1LL)
            {
            BeAssert(ecInstanceIdPropertyMap->GetTables().size() == 1LL);
            return ERROR;
            }
        DbTable const* primaryTable = ecInstanceIdPropertyMap->GetTables().front();

        WipColumnVerticalPropertyMap const* vMap = ecInstanceIdPropertyMap->FindVerticalPropertyMap(*primaryTable);
        sqlBuilder.Append(primaryTable->GetName().c_str(), vMap->GetColumn().GetName().c_str());
        sqlBuilder.AppendSpace();
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateViewForRelationship(NativeSqlBuilder& viewSql, ClassMap const& relationMap, ClassMap const& baseClassMap)
    {
    switch (relationMap.GetType())
        {
            case ClassMap::Type::RelationshipEndTable:
                return CreateViewForRelationshipClassEndTableMap(viewSql, static_cast<RelationshipClassEndTableMap const&>(relationMap), baseClassMap);
            case ClassMap::Type::RelationshipLinkTable:
                return CreateViewForRelationshipClassLinkTableMap(viewSql, static_cast<RelationshipClassMapCR>(relationMap), baseClassMap);
            default:
                BeAssert(false);
                return ERROR;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2013
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateNullViewForRelationship(NativeSqlBuilder& viewSql, ClassMap const& relationMap, ClassMap const& baseClassMap)
    {
    switch (relationMap.GetType())
        {
            case ClassMap::Type::RelationshipEndTable:
                return CreateNullViewForRelationshipClassEndTableMap(viewSql, static_cast<RelationshipClassMapCR>(relationMap), baseClassMap);
            case ClassMap::Type::RelationshipLinkTable:
                return CreateNullViewForRelationshipClassLinkTableMap(viewSql, static_cast<RelationshipClassMapCR>(relationMap), baseClassMap);
            default:
                BeAssert(false);
                return ERROR;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      09/2015
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateViewForRelationship(NativeSqlBuilder& viewSql, ClassMap const& relationMap)
    {
    BeAssert(relationMap.IsRelationshipClassMap());
    if (relationMap.GetMapStrategy().GetStrategy() == MapStrategy::NotMapped)
        return ERROR;

    ViewMemberByTable vmt;
    if (SUCCESS != ComputeViewMembers(vmt, relationMap.GetClass(), true))
        return ERROR;

    if (vmt.empty())
        return CreateNullViewForRelationship(viewSql, relationMap, relationMap);

    NativeSqlBuilder unionQuery;

    ViewMember const& viewMemberOfPrimaryTable = vmt[&relationMap.GetJoinedTable()];
    for (ClassMap const* cm : viewMemberOfPrimaryTable.GetClassMaps())
        {
        switch (cm->GetType())
            {
                case ClassMap::Type::RelationshipEndTable:
                {
                if (!unionQuery.IsEmpty())
                    unionQuery.Append(" UNION ");

                    if (SUCCESS != CreateViewForRelationshipClassEndTableMap(unionQuery, *static_cast<RelationshipClassEndTableMap const*>(cm), relationMap))
                    return ERROR;

                break;
                }
                case ClassMap::Type::RelationshipLinkTable:
                {
                if (!unionQuery.IsEmpty())
                    unionQuery.Append(" UNION ");

                if (SUCCESS != CreateViewForRelationshipClassLinkTableMap(unionQuery, *static_cast<RelationshipClassLinkTableMap const*>(cm), relationMap))
                    return ERROR;

                DbTable const& table = relationMap.GetJoinedTable();
                DbColumn const* classIdColumn = nullptr;
                if (table.TryGetECClassIdColumn(classIdColumn))
                    {
                    bool noClassIdFilterOption = false;
                    if (m_prepareContext)
                        {
                        if (OptionsExp const* options = m_prepareContext->GetCurrentScope().GetOptions())
                            noClassIdFilterOption = options->HasOption(OptionsExp::NOECCLASSIDFILTER_OPTION);
                        }
                    if (!noClassIdFilterOption)
                        {
                        Utf8String whereClause;
                        if (SUCCESS != cm->GetStorageDescription().GenerateECClassIdFilter(whereClause, table,
                                                                                           *classIdColumn, false, true))
                            return ERROR;

                        if (!whereClause.empty())
                            unionQuery.Append(" WHERE ").Append(whereClause.c_str());
                        }
                    }
                }
            }
        }

    vmt.erase(&relationMap.GetJoinedTable());

    //now process view members of other tables
    for (bpair<DbTable const*, ViewMember> const& vm : vmt)
        {
        DbTable const* table = vm.first;
        if (vm.second.GetStorageType() != DbSchema::EntityType::Table)
            continue;

        std::vector<RelationshipClassEndTableMap const*> etm;
        std::vector<RelationshipClassLinkTableMap const*> ltm;
        for (ClassMap const* cm : vm.second.GetClassMaps())
            {
            switch (cm->GetType())
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
            if (SUCCESS != CreateViewForRelationshipClassLinkTableMap(unionQuery, firstClassMap, relationMap))
                return ERROR;

            DbColumn const* classIdColumn = nullptr;
            if (table->TryGetECClassIdColumn(classIdColumn))
                {
                bool noClassIdFilterOption = false;
                if (m_prepareContext)
                    {
                    if (OptionsExp const* options = m_prepareContext->GetCurrentScope().GetOptions())
                        noClassIdFilterOption = options->HasOption(OptionsExp::NOECCLASSIDFILTER_OPTION);
                    }

                if (!noClassIdFilterOption)
                    {
                    Utf8String whereClause;
                    if (SUCCESS != firstClassMap.GetStorageDescription().GenerateECClassIdFilter(whereClause, *table,
                                                                                                 *classIdColumn, true))
                        return ERROR;

                    if (!whereClause.empty())
                        unionQuery.Append(" WHERE ").Append(whereClause.c_str());
                    }
                }
            }

        for (RelationshipClassEndTableMap const* et : etm)
            {
            if (!unionQuery.IsEmpty())
                unionQuery.Append(" UNION ");

            if (SUCCESS != CreateViewForRelationshipClassEndTableMap(unionQuery, *et, relationMap))
                return ERROR;
            }
        }
    
    if (m_asSubQuery)
        viewSql.AppendParenLeft();

    viewSql.Append(unionQuery.ToString());

    if (m_asSubQuery)
        viewSql.AppendParenRight();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
//static
BentleyStatus ViewGenerator::AppendSystemPropMaps(NativeSqlBuilder& viewSql, RelationshipClassMapCR relationMap, DbTable const& contextTable)
    {
    //We only want to render propertyMap w.r.t contextTable. A endRelationship can now have more then one columns in each different table for ECInstanceId, SourceECInstanceId, TargetECInstanceId ...
    //There for we only need to render in term of context table that is chosen before this function is called. Resulting select are UNIONed.
    WipPropertyMapSqlDispatcher sqlDispatcher(contextTable, WipPropertyMapSqlDispatcher::SqlTarget::View, contextTable.GetName().c_str());
   
    relationMap.GetECInstanceIdPropertyMap()->Accept(sqlDispatcher);
    relationMap.GetECClassIdPropertyMap()->Accept(sqlDispatcher);
    relationMap.GetSourceECInstanceIdPropMap()->Accept(sqlDispatcher);
    relationMap.GetSourceECClassIdPropMap()->Accept(sqlDispatcher);
    relationMap.GetTargetECInstanceIdPropMap()->Accept(sqlDispatcher);
    relationMap.GetTargetECClassIdPropMap()->Accept(sqlDispatcher);

    if (sqlDispatcher.GetResultSet().size() != 6LL)
        {
        BeAssert(false);
        return ERROR;
        }

    WipPropertyMapSqlDispatcher::Result const& rECInstanceId = sqlDispatcher.GetResultSet().at(0);
    WipPropertyMapSqlDispatcher::Result const& rECClassId = sqlDispatcher.GetResultSet().at(1);
    WipPropertyMapSqlDispatcher::Result const& rSourceECInstanceId = sqlDispatcher.GetResultSet().at(2);
    WipPropertyMapSqlDispatcher::Result const& rSourceECClassId = sqlDispatcher.GetResultSet().at(3);
    WipPropertyMapSqlDispatcher::Result const& rTargetECInstanceId = sqlDispatcher.GetResultSet().at(4);
    WipPropertyMapSqlDispatcher::Result const& rTargetECClassId = sqlDispatcher.GetResultSet().at(5);


    //ECInstanceId-----------------------------------------
    viewSql.Append(rECInstanceId.GetSql()).AppendComma();
    if (m_viewAccessStringList && m_captureViewAccessStringList)
        m_viewAccessStringList->push_back(rECInstanceId.GetAccessString());

    //ECClassId-----------------------------------
    viewSql.Append(rECClassId.GetSql()).AppendComma();
    if (m_viewAccessStringList && m_captureViewAccessStringList)
        m_viewAccessStringList->push_back(rECClassId.GetAccessString());
    viewSql.AppendComma();

    //SourceECInstanceId-----------------------------------

    viewSql.Append(rSourceECInstanceId.GetSql()).AppendComma();
    if (m_viewAccessStringList && m_captureViewAccessStringList)
        m_viewAccessStringList->push_back(rSourceECInstanceId.GetAccessString());
    viewSql.AppendComma();

    //SourceECClassId--------------------------------------
    WipConstraintECClassIdPropertyMap const* classIdPropMap = relationMap.GetSourceECClassIdPropMap();
    if (rSourceECClassId.IsColumnPersisted())
        {
        if (relationMap._RequiresJoin(ECRelationshipEnd::ECRelationshipEnd_Source))
            viewSql.AppendEscaped(GetECClassIdPrimaryTableAlias(ECRelationshipEnd::ECRelationshipEnd_Source)).AppendDot();
        else
            viewSql.AppendEscaped(rSourceECClassId.GetTable().GetName().c_str()).AppendDot();
        }

    AppendConstraintClassIdPropMap(viewSql, *classIdPropMap, relationMap, relationMap.GetRelationshipClass().GetSource(), contextTable);
    if (m_viewAccessStringList && m_captureViewAccessStringList)
        m_viewAccessStringList->push_back(classIdPropMap->GetAccessString());

    viewSql.AppendComma();

    //TargetECInstanceId-----------------------------------
    viewSql.Append(rTargetECInstanceId.GetSql()).AppendComma();
    if (m_viewAccessStringList && m_captureViewAccessStringList)
        m_viewAccessStringList->push_back(rTargetECInstanceId.GetAccessString());
    viewSql.AppendComma();

    //TargetECClassId--------------------------------------
    classIdPropMap = relationMap.GetTargetECClassIdPropMap();
    if (rTargetECClassId.IsColumnPersisted())
        {
        if (relationMap._RequiresJoin(ECRelationshipEnd::ECRelationshipEnd_Target))
            viewSql.AppendEscaped(GetECClassIdPrimaryTableAlias(ECRelationshipEnd::ECRelationshipEnd_Target)).AppendDot();
        else
            viewSql.AppendEscaped(rTargetECClassId.GetTable().GetName().c_str()).AppendDot();
        }

    AppendConstraintClassIdPropMap(viewSql, *classIdPropMap, relationMap, relationMap.GetRelationshipClass().GetTarget(), contextTable);
    if (m_viewAccessStringList && m_captureViewAccessStringList)
        m_viewAccessStringList->push_back(classIdPropMap->GetAccessString());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
//static
BentleyStatus ViewGenerator::AppendSystemPropMapsToNullView(NativeSqlBuilder& viewSql, RelationshipClassMapCR relationMap, bool endWithComma)
    {
    WipPropertyMapSqlDispatcher sqlDispatcher(relationMap.GetJoinedTable(), WipPropertyMapSqlDispatcher::SqlTarget::View, relationMap.GetJoinedTable().GetName().c_str());

    relationMap.GetECInstanceIdPropertyMap()->Accept(sqlDispatcher);
    relationMap.GetECClassIdPropertyMap()->Accept(sqlDispatcher);
    relationMap.GetSourceECInstanceIdPropMap()->Accept(sqlDispatcher);
    relationMap.GetSourceECClassIdPropMap()->Accept(sqlDispatcher);
    relationMap.GetTargetECInstanceIdPropMap()->Accept(sqlDispatcher);
    relationMap.GetTargetECClassIdPropMap()->Accept(sqlDispatcher);

    if (sqlDispatcher.GetResultSet().size() != 6LL)
        {
        BeAssert(false);
        return ERROR;
        }

    WipPropertyMapSqlDispatcher::Result const& rECInstanceId = sqlDispatcher.GetResultSet().at(0);
    WipPropertyMapSqlDispatcher::Result const& rECClassId = sqlDispatcher.GetResultSet().at(1);
    WipPropertyMapSqlDispatcher::Result const& rSourceECInstanceId = sqlDispatcher.GetResultSet().at(2);
    WipPropertyMapSqlDispatcher::Result const& rSourceECClassId = sqlDispatcher.GetResultSet().at(3);
    WipPropertyMapSqlDispatcher::Result const& rTargetECInstanceId = sqlDispatcher.GetResultSet().at(4);
    WipPropertyMapSqlDispatcher::Result const& rTargetECClassId = sqlDispatcher.GetResultSet().at(5);


    viewSql.Append("SELECT NULL ").Append(rECInstanceId.GetColumn().GetName().c_str()).AppendComma();
    if (m_viewAccessStringList && m_captureViewAccessStringList)
        m_viewAccessStringList->push_back(rECInstanceId.GetAccessString());

    viewSql.Append("NULL ").Append(rECClassId.GetColumn().GetName().c_str()).AppendComma();
    if (m_viewAccessStringList && m_captureViewAccessStringList)
        m_viewAccessStringList->push_back(rECClassId.GetAccessString());

    viewSql.Append("NULL ").Append(rSourceECInstanceId.GetColumn().GetName().c_str()).AppendComma();
    if (m_viewAccessStringList && m_captureViewAccessStringList)
        m_viewAccessStringList->push_back(rSourceECInstanceId.GetAccessString());

    viewSql.Append("NULL ").Append(rSourceECClassId.GetColumn().GetName().c_str()).AppendComma();
    if (m_viewAccessStringList && m_captureViewAccessStringList)
        m_viewAccessStringList->push_back(rSourceECClassId.GetAccessString());
    
    viewSql.Append("NULL ").Append(rTargetECInstanceId.GetColumn().GetName().c_str()).AppendComma();
    if (m_viewAccessStringList && m_captureViewAccessStringList)
        m_viewAccessStringList->push_back(rTargetECInstanceId.GetAccessString());
    
    viewSql.Append("NULL ").Append(rTargetECClassId.GetColumn().GetName().c_str());
    if (m_viewAccessStringList && m_captureViewAccessStringList)
        m_viewAccessStringList->push_back(rTargetECClassId.GetAccessString());
    
    if (endWithComma)
        viewSql.AppendComma();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
//static
BentleyStatus ViewGenerator::AppendConstraintClassIdPropMap(NativeSqlBuilder& viewSql, WipConstraintECClassIdPropertyMap const& propMap, RelationshipClassMapCR relationMap, ECRelationshipConstraintCR constraint, DbTable const& contextTable)
    {
    WipPropertyMapSqlDispatcher sqlDispatherTable(contextTable, WipPropertyMapSqlDispatcher::SqlTarget::Table, nullptr);
    if (sqlDispatherTable.GetStatus() == ERROR || sqlDispatherTable.GetResultSet().empty())
        {
        BeAssert(false);
        return ERROR;
        }

    WipPropertyMapSqlDispatcher::Result const& r = sqlDispatherTable.GetResultSet().front();
    if (!r.IsColumnPersisted())
        {
        bool hasAnyClass = false;
        std::set<ClassMap const*> classMaps = m_ecdb.Schemas().GetDbMap().GetClassMapsFromRelationshipEnd(constraint, &hasAnyClass);
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
        Utf8Char endClassIdStr[ECClassId::ID_STRINGBUFFER_LENGTH];
        endClassId.ToString(endClassIdStr);
        viewSql.Append(endClassIdStr).AppendSpace();
        viewSql.Append(r.GetSql());
        }
    else
        {
        WipPropertyMapSqlDispatcher sqlDispatherView(contextTable, WipPropertyMapSqlDispatcher::SqlTarget::View, nullptr);
        viewSql.Append(sqlDispatherView.GetResultSet().front().GetSql());
        }

    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
