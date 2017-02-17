/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ViewGenerator.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
BentleyStatus ViewGenerator::GenerateSelectFromViewSql(NativeSqlBuilder& viewSql, ECSqlPrepareContext const& prepareContext, ClassMap const& classMap, bool isPolymorphicQuery)
    {
    SelectFromViewContext ctx(prepareContext, isPolymorphicQuery);
    return GenerateViewSql(viewSql, ctx, classMap);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2016
//+---------------+---------------+---------------+---------------+---------------+--------
//static 
BentleyStatus ViewGenerator::CreateUpdatableViews(ECDbCR ecdb)
    {
    if (ecdb.IsReadonly())
        {
        ecdb.GetECDbImplR().GetIssueReporter().Report("Can only call ECDb::CreateECClassViewsInDb() on an ECDb file with read-write access.");
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
        ecdb.GetECDbImplR().GetIssueReporter().Report("Can only call ECDb::CreateECClassViewsInDb() on an ECDb file with read-write access.");
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

    bvector<ECClassId> classIds;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ECClassId classId = stmt.GetValueId<ECClassId>(0);
        BeAssert(classId.IsValid());
        classIds.push_back(classId);
        }

    stmt.Finalize();
    return CreateECClassViews(ecdb, classIds);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                     12/2016
//+---------------+---------------+---------------+---------------+---------------+--------
//static 
BentleyStatus ViewGenerator::CreateECClassViews(ECDbCR ecdb, bvector<ECClassId> const& ecclassIds)
    {
    for (ECClassId classId : ecclassIds)
        {
        if (!classId.IsValid())
            return ERROR;

        ECClassCP ecClass = ecdb.Schemas().GetECClass(classId);
        if (ecClass == nullptr)
            {
            BeAssert(ecClass != nullptr);
            return ERROR;
            }

        ClassMapCP classMap = ecdb.Schemas().GetDbMap().GetClassMap(*ecClass);
        if (classMap == nullptr)
            {
            BeAssert(classMap != nullptr);
            return ERROR;
            }

        if (classMap->GetType() == ClassMap::Type::NotMapped || (!classMap->GetClass().IsEntityClass() && !classMap->GetClass().IsRelationshipClass()))
            {
            ecdb.GetECDbImplR().GetIssueReporter().Report("Cannot create ECClassView for ECClass '%s' (Id: %s) because it is not mapped or not an ECEntityclass or ECRelationshipClass.",
                                                          classMap->GetClass().GetFullName(), classId.ToString().c_str());
            return ERROR;
            }

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
    ECClassViewContext ctx(ecdb);
    Utf8String viewName;
    viewName.Sprintf("[%s.%s]", classMap.GetClass().GetSchema().GetAlias().c_str(), classMap.GetClass().GetName().c_str());

    NativeSqlBuilder viewSql;
    if (GenerateViewSql(viewSql, ctx, classMap) != SUCCESS)
        return ERROR;

    Utf8String viewColumnNameList;
    bool bFirst = true;
    for (Utf8StringCP columnName : ctx.GetViewColumnNames())
        {
        BeAssert(columnName != nullptr);
        if (bFirst)
            bFirst = false;
        else
            viewColumnNameList.append(", ");

        viewColumnNameList.append("[").append(*columnName).append("]");
        }

    Utf8String createViewSql;
    createViewSql.Sprintf("CREATE VIEW %s (%s)\n\t--### ECCLASS VIEW is for debugging purpose only!.\n\tAS %s;", viewName.c_str(), viewColumnNameList.c_str(), viewSql.ToString());
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
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateUpdatableViewIfRequired(ECDbCR ecdb, ClassMap const& classMap)
    {
    if (classMap.GetMapStrategy().GetStrategy() == MapStrategy::NotMapped || classMap.IsRelationshipClassMap())
        return ERROR;

    UpdatableViewContext ctx(ecdb);

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

            ClassMapCP derviedClassMap = ctx.GetECDb().Schemas().GetDbMap().GetClassMap(*rootClass);
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

    NativeSqlBuilder viewBodySql;
    if (GenerateViewSql(viewBodySql, ctx, classMap) != SUCCESS)
        return ERROR;
    
    Utf8String updatableViewDdl;
    updatableViewDdl.Sprintf("CREATE VIEW %s AS %s", updatableViewName.c_str(), viewBodySql.ToString());

    if (ctx.GetECDb().ExecuteSql(updatableViewDdl.c_str()) != BE_SQLITE_OK)
        return ERROR;

    for (Utf8StringCR triggerDdl : triggerDdlList)
        {
        if (ctx.GetECDb().ExecuteSql(triggerDdl.c_str()) != BE_SQLITE_OK)
            return ERROR;
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
        SearchPropertyMapVisitor typeVisitor(PropertyMap::Type::Data);
        baseClassMap.GetPropertyMaps().AcceptVisitor(typeVisitor);
        for (PropertyMap const* result : typeVisitor.Results())
            {
            PropertyMap const* derivedPropMap = derivedClassMap.GetPropertyMaps().Find(result->GetAccessString().c_str());
            if (derivedPropMap == nullptr || !derivedPropMap->IsData())
                {
                BeAssert(false);
                return ERROR;
                }

            std::vector<DbColumn const*> derivedColumnList, baseColumnList;
            GetColumnsPropertyMapVisitor baseColumnVisitor, derivedColumnVisitor;
            result->AcceptVisitor(baseColumnVisitor);
            derivedPropMap->AcceptVisitor(derivedColumnVisitor);
            if (baseColumnVisitor.GetColumns().size() != derivedColumnVisitor.GetColumns().size())
                {
                BeAssert(false);
                return ERROR;
                }

            for (auto deriveColumnItor = derivedColumnVisitor.GetColumns().begin(), baseColumnItor = baseColumnVisitor.GetColumns().begin(); deriveColumnItor != derivedColumnVisitor.GetColumns().end() && baseColumnItor != baseColumnVisitor.GetColumns().end(); ++deriveColumnItor, ++baseColumnItor)
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
// @bsimethod                                    Affan.Khan                      07/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
BentleyStatus ViewGenerator::GenerateViewSql(NativeSqlBuilder& viewSql, Context& ctx, ClassMap const& classMap)
    {
    if (classMap.GetMapStrategy().GetStrategy() == MapStrategy::NotMapped)
        {
        BeAssert(false && "ViewGenerator::CreateView must not be called on unmapped class");
        return ERROR;
        }

    if (classMap.IsRelationshipClassMap())
        {
        if (classMap.GetType() == ClassMap::Type::RelationshipEndTable)
            return RenderRelationshipClassEndTableMap(viewSql, ctx, static_cast<RelationshipClassEndTableMap const&>(classMap));

        return RenderRelationshipClassLinkTableMap(viewSql, ctx, static_cast<RelationshipClassLinkTableMap const&>(classMap));
        }

    return RenderEntityClassMap(viewSql, ctx, classMap);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                          11/2016
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::RenderEntityClassMap(NativeSqlBuilder& viewSql, Context& ctx, ClassMap const& classMap)
    {
    NativeSqlBuilder::List unionList;
    StorageDescription const& storageDesc = classMap.GetStorageDescription();
    bool isVertical = storageDesc.GetVerticalPartitions().size() > 1;
    std::vector<Partition const*> partitionOfInterest;
    if (isVertical)
        {
        for (Partition const& partition : storageDesc.GetVerticalPartitions())
            partitionOfInterest.push_back(&partition);
        }
    else
        {
        if (ctx.GetViewType() != ViewType::SelectFromView || ctx.GetAs<SelectFromViewContext>().IsPolymorphicQuery())
            {
            for (Partition const& partition : storageDesc.GetHorizontalPartitions())
                partitionOfInterest.push_back(&partition);
            }
        else
            partitionOfInterest.push_back(&storageDesc.GetRootHorizontalPartition());

        }

    for (Partition const* partition : partitionOfInterest)
        {
        if (partition->GetTable().GetPersistenceType() == PersistenceType::Virtual)
            continue;

        //For vertical partition we like to skip the first primary partition table.
        if (isVertical)
            {
            if (&partition->GetTable() == &storageDesc.GetRootHorizontalPartition().GetTable())
                continue;
            }

        NativeSqlBuilder view;
        ECClass const* tableRootClass = ctx.GetECDb().Schemas().GetECClass(partition->GetRootClassId());
        if (tableRootClass == nullptr || tableRootClass->GetClassType() != ECClassType::Entity)
            {
            BeAssert(false);
            return ERROR;
            }

        ClassMap const* tableRootClassMap = ctx.GetECDb().Schemas().GetDbMap().GetClassMap(*tableRootClass);
        if (tableRootClassMap == nullptr || tableRootClassMap->GetType() != ClassMap::Type::Class)
            {
            BeAssert(false);
            return ERROR;
            }

        ClassMap const* castInto = tableRootClassMap == &classMap ? nullptr : &classMap;
        if (RenderEntityClassMap(view, ctx, *tableRootClassMap, partition->GetTable(), castInto) != SUCCESS)
            return ERROR;

        //capture view column names only for the first class, all other classes will be unioned together and therefore
        //have the same select clause
        if (ctx.GetViewType() == ViewType::ECClassView)
            ctx.GetAs<ECClassViewContext>().StopCaptureViewColumnNames();

        if (SystemPropertyMap::PerTablePrimitivePropertyMap const* classIdPropertyMap = tableRootClassMap->GetECClassIdPropertyMap()->FindDataPropertyMap(partition->GetTable()))
            {
            const bool isSelectFromView = ctx.GetViewType() == ViewType::SelectFromView;
            if (classIdPropertyMap->GetColumn().GetPersistenceType() == PersistenceType::Physical && 
                (!isSelectFromView || ctx.GetAs<SelectFromViewContext>().IsECClassIdFilterEnabled()))
                {
                const bool considerSubclasses = !isSelectFromView || ctx.GetAs<SelectFromViewContext>().IsPolymorphicQuery();
                Utf8String whereClause;
                if (SUCCESS != storageDesc.GenerateECClassIdFilter(whereClause, partition->GetTable(), classIdPropertyMap->GetColumn(), considerSubclasses, true))
                    return ERROR;

                if (!whereClause.empty())
                    view.Append(" WHERE ").Append(whereClause.c_str());
                }
            }

        unionList.push_back(view);
        }

    if (unionList.empty())
        {
        if (RenderNullView(viewSql, ctx, classMap) != SUCCESS)
            return ERROR;
        }
    else
        {
        if (ctx.GetViewType() == ViewType::SelectFromView)
            viewSql.AppendParenLeft();

        viewSql.Append(unionList, " UNION ");

        if (ctx.GetViewType() == ViewType::SelectFromView)
            viewSql.AppendParenRight();
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                          11/2016
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::RenderEntityClassMap(NativeSqlBuilder& viewSql, Context& ctx, ClassMap const& classMap, DbTable const& contextTable, ClassMapCP castAs)
    {
    viewSql.Append("SELECT ");
    DbTable const* requireJoinTo = nullptr;
    if (RenderPropertyMaps(viewSql, ctx, requireJoinTo, classMap, contextTable, castAs, PropertyMap::Type::Data | PropertyMap::Type::ECInstanceId | PropertyMap::Type::ECClassId) != SUCCESS)
        return ERROR;

    viewSql.Append(" FROM ").AppendEscaped(contextTable.GetName().c_str());
    //Join necessary table for table 
    if (requireJoinTo != nullptr)
        {
        DbColumn const* primaryKey = contextTable.GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
        DbColumn const* fkKey = requireJoinTo->GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
        viewSql.Append(" INNER JOIN ").AppendEscaped(requireJoinTo->GetName().c_str());
        viewSql.Append(" ON ").AppendEscaped(contextTable.GetName().c_str()).AppendDot().AppendEscaped(primaryKey->GetName().c_str());
        viewSql.Append(" = ").AppendEscaped(requireJoinTo->GetName().c_str()).AppendDot().AppendEscaped(fkKey->GetName().c_str());
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                          11/2016
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::RenderNullView(NativeSqlBuilder& viewSql, Context& ctx, ClassMap const& classMap)
    {
    SearchPropertyMapVisitor visitor(PropertyMap::Type::System | PropertyMap::Type::SingleColumnData);
    classMap.GetPropertyMaps().AcceptVisitor(visitor);
    if (ctx.GetViewType() == ViewType::SelectFromView)
        viewSql.AppendParenLeft();

    viewSql.Append("SELECT ");
    bool first = true;
    for (PropertyMap const* propertyMap : visitor.Results())
        {
        if (first)
            first = false;
        else
            viewSql.AppendComma();

        if (propertyMap->IsSystem())
            viewSql.Append("NULL ").AppendEscaped(propertyMap->GetAccessString().c_str());
        else
            {
            PrimitivePropertyMap const* primitiveMap = propertyMap->GetAs<PrimitivePropertyMap>();
            viewSql.Append("NULL ").AppendEscaped(primitiveMap->GetColumn().GetName().c_str());
            }

        if (ctx.GetViewType() == ViewType::ECClassView && ctx.GetAs<ECClassViewContext>().MustCaptureViewColumnNames())
            ctx.GetAs<ECClassViewContext>().AddViewColumnName(propertyMap->GetAccessString());
        }

    viewSql.Append(" LIMIT 0");

    if (ctx.GetViewType() == ViewType::SelectFromView)
        viewSql.AppendParenRight();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                          11/2016
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::RenderRelationshipClassLinkTableMap(NativeSqlBuilder& viewSql, Context& ctx, RelationshipClassLinkTableMap const& relationMap)
    {
    NativeSqlBuilder::List unionList;
    StorageDescription const& storageDesc = relationMap.GetStorageDescription();
    for (Partition const& partition : storageDesc.GetHorizontalPartitions())
        {
        if (partition.GetTable().GetPersistenceType() == PersistenceType::Virtual)
            continue;

        NativeSqlBuilder view;
        ECClass const* relationshipECClass = ctx.GetECDb().Schemas().GetECClass(partition.GetRootClassId());
        if (relationshipECClass == nullptr || !relationshipECClass->IsRelationshipClass())
            {
            BeAssert(false);
            return ERROR;
            }

        ClassMap const* classMap = ctx.GetECDb().Schemas().GetDbMap().GetClassMap(*relationshipECClass);
        if (classMap == nullptr || classMap->GetType() != ClassMap::Type::RelationshipLinkTable)
            {
            BeAssert(false);
            return ERROR;
            }

        RelationshipClassLinkTableMap const& contextRelationship = static_cast<RelationshipClassLinkTableMap const&>(*classMap);
        RelationshipClassLinkTableMap const* castInto = &contextRelationship == &relationMap ? nullptr : &relationMap;
        ConstraintECClassIdJoinInfo sourceECClassIdJoinInfo = ConstraintECClassIdJoinInfo::Create(*relationMap.GetSourceECClassIdPropMap(), partition.GetTable());
        ConstraintECClassIdJoinInfo targetECClassIdJoinInfo = ConstraintECClassIdJoinInfo::Create(*relationMap.GetTargetECClassIdPropMap(), partition.GetTable());
        if (DoRenderRelationshipClassMap(view, ctx, contextRelationship, partition.GetTable(), sourceECClassIdJoinInfo, targetECClassIdJoinInfo, castInto) != SUCCESS)
            return ERROR;

        //capture view column names only for the first table, all other tables will be unioned together and therefore
        //have the same select clause
        if (ctx.GetViewType() == ViewType::ECClassView)
            ctx.GetAs<ECClassViewContext>().StopCaptureViewColumnNames();

        if (sourceECClassIdJoinInfo.RequiresJoin())
            view.Append(sourceECClassIdJoinInfo.GetNativeJoinSql());

        if (targetECClassIdJoinInfo.RequiresJoin())
            view.Append(targetECClassIdJoinInfo.GetNativeJoinSql());

        ECClassIdPropertyMap const* classIdPropMap = relationMap.GetECClassIdPropertyMap();
        if (SystemPropertyMap::PerTablePrimitivePropertyMap const* classIdDataPropertyMap = classIdPropMap->FindDataPropertyMap(partition.GetTable()))
            {
            const bool isSelectFromView = ctx.GetViewType() == ViewType::SelectFromView;
            if (classIdDataPropertyMap->GetColumn().GetPersistenceType() == PersistenceType::Physical && (!isSelectFromView || ctx.GetAs<SelectFromViewContext>().IsECClassIdFilterEnabled()))
                {
                Utf8String whereClause;
                const bool considerSubclasses = !isSelectFromView || ctx.GetAs<SelectFromViewContext>().IsPolymorphicQuery();
                if (SUCCESS != storageDesc.GenerateECClassIdFilter(whereClause, partition.GetTable(), classIdDataPropertyMap->GetColumn(), considerSubclasses, true))
                    return ERROR;

                if (!whereClause.empty())
                    view.Append(" WHERE ").Append(whereClause.c_str());
                }
            }

        unionList.push_back(view);
        }

    if (unionList.empty())
        {
        if (RenderNullView(viewSql, ctx, relationMap) != SUCCESS)
            return ERROR;
        }
    else
        {
        if (ctx.GetViewType() == ViewType::SelectFromView)
            viewSql.AppendParenLeft();

        viewSql.Append(unionList, " UNION ");

        if (ctx.GetViewType() == ViewType::SelectFromView)
            viewSql.AppendParenRight();
        }

    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                          11/2016
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::RenderRelationshipClassEndTableMap(NativeSqlBuilder& viewSql, Context& ctx, RelationshipClassEndTableMap const& relationMap) 
    {
    NativeSqlBuilder::List unionList;
    for (DbTable const* table : relationMap.GetTables())
        {
        if (table->GetPersistenceType() == PersistenceType::Virtual)
            continue;

        NativeSqlBuilder view;
        ConstraintECClassIdJoinInfo sourceECClassIdJoinInfo = ConstraintECClassIdJoinInfo::Create(*relationMap.GetSourceECClassIdPropMap(), *table);
        ConstraintECClassIdJoinInfo targetECClassIdJoinInfo = ConstraintECClassIdJoinInfo::Create(*relationMap.GetTargetECClassIdPropMap(), *table);

        if (DoRenderRelationshipClassMap(view, ctx, relationMap, *table, sourceECClassIdJoinInfo, targetECClassIdJoinInfo) != SUCCESS)
            return ERROR;

        //capture view column names only for the first table, all other tables will be unioned together and therefore
        //have the same select clause
        if (ctx.GetViewType() == ViewType::ECClassView)
            ctx.GetAs<ECClassViewContext>().StopCaptureViewColumnNames();

        if (sourceECClassIdJoinInfo.RequiresJoin())
            view.Append(sourceECClassIdJoinInfo.GetNativeJoinSql());

        if (targetECClassIdJoinInfo.RequiresJoin())
            view.Append(targetECClassIdJoinInfo.GetNativeJoinSql());

        view.Append(" WHERE ").AppendEscaped(relationMap.GetReferencedEndECInstanceIdPropMap()->GetAccessString().c_str()).Append(" IS NOT NULL");
        //! Add Polymorphic Filter if required
        if (SystemPropertyMap::PerTablePrimitivePropertyMap const* classIdPropertyMap = relationMap.GetECClassIdPropertyMap()->FindDataPropertyMap(*table))
            {
            const bool isSelectFromView = ctx.GetViewType() == ViewType::SelectFromView;
            if (classIdPropertyMap->GetColumn().GetPersistenceType() == PersistenceType::Physical &&
                (!isSelectFromView || ctx.GetAs<SelectFromViewContext>().IsECClassIdFilterEnabled()))
                {    
                NativeSqlBuilder classIdFilter;
                Utf8Char classIdStr[ECClassId::ID_STRINGBUFFER_LENGTH];
                relationMap.GetClass().GetId().ToString(classIdStr);
                classIdFilter.AppendEscaped(table->GetName().c_str()).AppendDot().AppendEscaped(classIdPropertyMap->GetColumn().GetName().c_str());

                if (!isSelectFromView || ctx.GetAs<SelectFromViewContext>().IsPolymorphicQuery())
                    classIdFilter.Append(" IN (SELECT ClassId FROM " TABLE_ClassHierarchyCache " WHERE BaseClassId=").Append(classIdStr).Append(")");
                else
                    classIdFilter.Append(BooleanSqlOperator::EqualTo).Append(classIdStr);

                //We always have a WHERE so always add AND operator
                view.AppendSpace().Append(BooleanSqlOperator::And).AppendSpace().Append(classIdFilter);
                }
            }
        unionList.push_back(view);
        }

    if (unionList.empty())
        {
        if (RenderNullView(viewSql, ctx, relationMap) != SUCCESS)
            return ERROR;
        }
    else
        {
        if (ctx.GetViewType() == ViewType::SelectFromView)
            viewSql.AppendParenLeft();

        viewSql.Append(unionList, " UNION ");

        if (ctx.GetViewType() == ViewType::SelectFromView)
            viewSql.AppendParenRight();
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                          11/2016
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::DoRenderRelationshipClassMap(NativeSqlBuilder& viewSql, Context& ctx, RelationshipClassMap const& relationMap, DbTable const& contextTable, ConstraintECClassIdJoinInfo const& sourceJoinInfo, ConstraintECClassIdJoinInfo const& targetJoinInfo, RelationshipClassLinkTableMap const* castInto)
    {
    const bool requiresJoin = sourceJoinInfo.RequiresJoin() || targetJoinInfo.RequiresJoin();
    ToSqlVisitor sqlVisitor(ctx, contextTable, contextTable.GetName().c_str(), true, false);
    viewSql.Append("SELECT ");
    //ECInstanceId
    if (ctx.GetViewType() == ViewType::ECClassView && ctx.GetAs<ECClassViewContext>().MustCaptureViewColumnNames())
        ctx.GetAs<ECClassViewContext>().AddViewColumnName(relationMap.GetECInstanceIdPropertyMap()->GetAccessString());

    sqlVisitor.Reset();
    relationMap.GetECInstanceIdPropertyMap()->AcceptVisitor(sqlVisitor);
    viewSql.Append(sqlVisitor.GetResultSet().front().GetSqlBuilder());

    //ECClassId
    if (ctx.GetViewType() == ViewType::ECClassView && ctx.GetAs<ECClassViewContext>().MustCaptureViewColumnNames())
        ctx.GetAs<ECClassViewContext>().AddViewColumnName(relationMap.GetECClassIdPropertyMap()->GetAccessString());

    sqlVisitor.Reset();
    relationMap.GetECClassIdPropertyMap()->AcceptVisitor(sqlVisitor);
    viewSql.AppendComma().Append(sqlVisitor.GetResultSet().front().GetSqlBuilder());

    //SourceECInstanceId
    if (ctx.GetViewType() == ViewType::ECClassView && ctx.GetAs<ECClassViewContext>().MustCaptureViewColumnNames())
        ctx.GetAs<ECClassViewContext>().AddViewColumnName(relationMap.GetSourceECInstanceIdPropMap()->GetAccessString());

    sqlVisitor.Reset();
    relationMap.GetSourceECInstanceIdPropMap()->AcceptVisitor(sqlVisitor);
    viewSql.AppendComma().Append(sqlVisitor.GetResultSet().front().GetSqlBuilder());

    //SourceECClassId
    if (ctx.GetViewType() == ViewType::ECClassView && ctx.GetAs<ECClassViewContext>().MustCaptureViewColumnNames())
        ctx.GetAs<ECClassViewContext>().AddViewColumnName(relationMap.GetSourceECClassIdPropMap()->GetAccessString());

    if (sourceJoinInfo.RequiresJoin())
        viewSql.AppendComma().Append(sourceJoinInfo.GetNativeConstraintECClassIdSql(true));
    else
        {
        if (DbTable const* table = ConstraintECClassIdJoinInfo::RequiresJoinTo(*relationMap.GetSourceECClassIdPropMap(), true /*ignoreVirtualColumnCheck*/))
            {
            ToSqlVisitor constraintSqlVisitor(ctx, *table, contextTable.GetName().c_str(), true, false);
            relationMap.GetSourceECClassIdPropMap()->AcceptVisitor(constraintSqlVisitor);
            BeAssert(!constraintSqlVisitor.GetResultSet().empty());
            viewSql.AppendComma().Append(constraintSqlVisitor.GetResultSet().front().GetSqlBuilder());
            }
        else
            {
            //SourceECClassId = ECClassId, TargetECClassId = ECClassId
            sqlVisitor.Reset();
            relationMap.GetSourceECClassIdPropMap()->AcceptVisitor(sqlVisitor);
            BeAssert(!sqlVisitor.GetResultSet().empty());
            viewSql.AppendComma().Append(sqlVisitor.GetResultSet().front().GetSqlBuilder());
            }
        }

    //TargetECInstanceid
    if (ctx.GetViewType() == ViewType::ECClassView && ctx.GetAs<ECClassViewContext>().MustCaptureViewColumnNames())
        ctx.GetAs<ECClassViewContext>().AddViewColumnName(relationMap.GetTargetECInstanceIdPropMap()->GetAccessString());

    sqlVisitor.Reset();
    relationMap.GetTargetECInstanceIdPropMap()->AcceptVisitor(sqlVisitor);
    viewSql.AppendComma().Append(sqlVisitor.GetResultSet().front().GetSqlBuilder());

    //TargetECClassId
    if (ctx.GetViewType() == ViewType::ECClassView && ctx.GetAs<ECClassViewContext>().MustCaptureViewColumnNames())
        ctx.GetAs<ECClassViewContext>().AddViewColumnName(relationMap.GetTargetECClassIdPropMap()->GetAccessString());

    if (targetJoinInfo.RequiresJoin())
        viewSql.AppendComma().Append(targetJoinInfo.GetNativeConstraintECClassIdSql(true));
    else
        {
        if (DbTable const* table = ConstraintECClassIdJoinInfo::RequiresJoinTo(*relationMap.GetTargetECClassIdPropMap(), true /*ignoreVirtualColumnCheck*/))
            {
            ToSqlVisitor constraintSqlVisitor(ctx, *table, contextTable.GetName().c_str(), true, false);
            relationMap.GetTargetECClassIdPropMap()->AcceptVisitor(constraintSqlVisitor);
            BeAssert(!constraintSqlVisitor.GetResultSet().empty());
            viewSql.AppendComma().Append(constraintSqlVisitor.GetResultSet().front().GetSqlBuilder());
            }
        else
            {
            sqlVisitor.Reset();
            relationMap.GetTargetECClassIdPropMap()->AcceptVisitor(sqlVisitor);
            BeAssert(!sqlVisitor.GetResultSet().empty());
            viewSql.AppendComma().Append(sqlVisitor.GetResultSet().front().GetSqlBuilder());
            }
        }

    DbTable const* requireJoinTo;
    NativeSqlBuilder dataPropertySql;
    if (RenderPropertyMaps(dataPropertySql, ctx, requireJoinTo, relationMap, contextTable, nullptr, PropertyMap::Type::Data, requiresJoin) != SUCCESS)
        return ERROR;

    if (requireJoinTo != nullptr)
        {
        BeAssert(false && "Relationship does not support joined table so this is a error");
        return ERROR;
        }

    if (!dataPropertySql.IsEmpty())
        {
        BeAssert(relationMap.GetType() == ClassMap::Type::RelationshipLinkTable && "Only LinkTable can have property");
        viewSql.AppendComma().Append(dataPropertySql);
        }

    viewSql.Append(" FROM ").AppendEscaped(contextTable.GetName().c_str());
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                          11/2016
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::RenderPropertyMaps(NativeSqlBuilder& sqlView, Context& ctx, DbTable const*& requireJoinTo, ClassMapCR classMap, DbTable const&  contextTable, ClassMapCP baseClass, PropertyMap::Type filter, bool requireJoin)
    {
    requireJoinTo = nullptr;
    if (Enum::Contains(filter, PropertyMap::Type::ConstraintECClassId) || Enum::Contains(filter, PropertyMap::Type::ConstraintECInstanceId))
        {
        BeAssert(false && "This function cannot render ConstraintECClassId and ConstraintECInstanceId property maps");
        return ERROR;
        }

    std::vector<std::pair<PropertyMap const*, PropertyMap const*>> propertyMaps;
    if (!classMap.IsMappedTo(contextTable))
        {
        BeAssert(false);
        return ERROR;
        }

    DbTable const* requireJoinToTableForDataProperties = nullptr;
    if (baseClass != nullptr)
        {
        SearchPropertyMapVisitor propertyVisitor(PropertyMap::Type::System | PropertyMap::Type::SingleColumnData);
        baseClass->GetPropertyMaps().AcceptVisitor(propertyVisitor);
        for (PropertyMap const* basePropertyMap : propertyVisitor.Results())
            {
            if (!Enum::Contains(filter, basePropertyMap->GetType()))
                continue;

            if (ctx.GetViewType() == ViewType::SelectFromView && !ctx.GetAs<SelectFromViewContext>().IsInSelectClause(basePropertyMap->GetAccessString()))
                continue;

            PropertyMap const* propertyMap = classMap.GetPropertyMaps().Find(basePropertyMap->GetAccessString().c_str());
            if (propertyMap == nullptr)
                {
                BeAssert(false && "classMap is not subclass of baseClass");
                return ERROR;
                }

            //!We assume that in case of joinedTable we can only have exactly one table to join to.
            //!Therefore not using a set/vector to store joinTable list
            if (requireJoinToTableForDataProperties == nullptr)
                {
                if (propertyMap->IsData())
                    {
                    DataPropertyMap const* dataPropertyMap = propertyMap->GetAs<DataPropertyMap>();
                    if (&dataPropertyMap->GetTable() != &contextTable)
                        requireJoinToTableForDataProperties = &dataPropertyMap->GetTable();
                    }
                }

            propertyMaps.push_back(std::make_pair(propertyMap, basePropertyMap));
            }
        }
    else
        {
        SearchPropertyMapVisitor propertyVisitor(PropertyMap::Type::System | PropertyMap::Type::SingleColumnData);
        classMap.GetPropertyMaps().AcceptVisitor(propertyVisitor);
        for (PropertyMap const* propertyMap : propertyVisitor.Results())
            {
            if (Enum::Contains(filter, propertyMap->GetType()))
                {
                if (ctx.GetViewType() == ViewType::SelectFromView && !ctx.GetAs<SelectFromViewContext>().IsInSelectClause(propertyMap->GetAccessString()))
                    continue;

                //!We assume that in case of joinedTable we can only have exactly one table to joint to.
                //!Therefore not using a set/vector to store joinTable list
                if (requireJoinToTableForDataProperties == nullptr)
                    {
                    if (propertyMap->IsData())
                        {
                        DataPropertyMap const* dataPropertyMap = propertyMap->GetAs<DataPropertyMap>();
                        if (&dataPropertyMap->GetTable() != &contextTable)
                            requireJoinToTableForDataProperties = &dataPropertyMap->GetTable();
                        }
                    }
                propertyMaps.push_back(std::make_pair(propertyMap, nullptr));
                }
            }
        }

    if (propertyMaps.empty())
        return SUCCESS;

    Utf8CP systemContextTableAlias = requireJoinToTableForDataProperties || requireJoin ? contextTable.GetName().c_str() : nullptr;
    NativeSqlBuilder::List propertySqlList;
    for (auto const& kvp : propertyMaps)
        {
        PropertyMap const* basePropertyMap = kvp.second;
        PropertyMap const* propertyMap = kvp.first;
        if (basePropertyMap != nullptr)
            BeAssert(dynamic_cast<CompoundDataPropertyMap const*>(basePropertyMap) == nullptr);

        BeAssert(dynamic_cast<CompoundDataPropertyMap const*>(propertyMap) == nullptr);
        if (ctx.GetViewType() == ViewType::ECClassView && ctx.GetAs<ECClassViewContext>().MustCaptureViewColumnNames())
            ctx.GetAs<ECClassViewContext>().AddViewColumnName(propertyMap->GetAccessString());

        // We only need table qualifier if there is at least one data property selected that require joining to another table
        // In this case all data properties are table qualified name to ensure no conflict between two tables columns.
        // System property never require a join but therefor requireJoinToTableForDataProperties = nullptr if no data property was choosen
        if (propertyMap->IsSystem())
            {
            ToSqlVisitor toSqlVisitor(ctx, contextTable, systemContextTableAlias, true, ctx.GetViewType() == ViewType::ECClassView);
            if (SUCCESS != propertyMap->AcceptVisitor(toSqlVisitor) || toSqlVisitor.GetResultSet().empty())
                {
                BeAssert(false);
                return ERROR;
                }

            ToSqlVisitor::Result const& r = toSqlVisitor.GetResultSet().front();
            propertySqlList.push_back(r.GetSqlBuilder());
            continue;
            }

        BeAssert(propertyMap->IsData());
        BeAssert(dynamic_cast<SingleColumnDataPropertyMap const*>(propertyMap) != nullptr);

        SingleColumnDataPropertyMap const* dataProperty = propertyMap->GetAs<SingleColumnDataPropertyMap>();
        //! Join table does not require casting as we only split table into exactly two possible tables and only if shared table is enabled.
        if (&dataProperty->GetTable() == requireJoinToTableForDataProperties)
            {
            ToSqlVisitor toSqlVisitor(ctx, *requireJoinToTableForDataProperties, requireJoinToTableForDataProperties->GetName().c_str(), false, ctx.GetViewType() == ViewType::ECClassView);

            if (SUCCESS != dataProperty->AcceptVisitor(toSqlVisitor) || toSqlVisitor.GetResultSet().empty())
                {
                BeAssert(false);
                return ERROR;
                }

            ToSqlVisitor::Result const& r = toSqlVisitor.GetResultSet().front();
            //! This is where we generate strong type column for shared column for ECClassView
            if (ctx.GetViewType() == ViewType::ECClassView)
                {
                NativeSqlBuilder propertySql;
                if (r.GetColumn().IsShared())
                    {
                    //shared columns don't have a column data type in its DDL.
                    //ECDb uses SQL-99 data types as SQLite supports them in their CAST expression
                    //and DDL - although they have only informational meaning.
                    //So to render data in shared columns according to the correct type, ECDb casts the shared column
                    //to the respective type
                    BeAssert(r.GetColumn().GetType() == DbColumn::Type::Any);
                    Utf8String castExp;
                    castExp.Sprintf("CAST(%s AS %s)", r.GetSqlBuilder().ToString(), DbColumn::TypeToSql(r.GetPropertyMap().GetColumnDataType()));
                    propertySql.Append(castExp.c_str());
                    }
                else
                    propertySql.Append(r.GetSqlBuilder().ToString());

                propertySqlList.push_back(propertySql);
                continue;
                }

            propertySqlList.push_back(r.GetSqlBuilder());
            continue;
            }

        //no join needed
        ToSqlVisitor toSqlVisitor(ctx, contextTable, systemContextTableAlias, false, ctx.GetViewType() == ViewType::ECClassView);
        if (SUCCESS != dataProperty->AcceptVisitor(toSqlVisitor) || toSqlVisitor.GetResultSet().empty())
            {
            BeAssert(false);
            return ERROR;
            }

        NativeSqlBuilder propertySql;
        ToSqlVisitor::Result const& r = toSqlVisitor.GetResultSet().front();
        if (ctx.GetViewType() == ViewType::ECClassView && r.GetColumn().IsShared())
            {
            //shared columns don't have a column data type in its DDL.
            //ECDb uses SQL-99 data types as SQLite supports them in their CAST expression
            //and DDL - although they have only informational meaning.
            //So to render data in shared columns according to the correct type, ECDb casts the shared column
            //to the respective type
            BeAssert(r.GetColumn().GetType() == DbColumn::Type::Any);
            Utf8String castExp;
            castExp.Sprintf("CAST(%s AS %s)", r.GetSqlBuilder().ToString(), DbColumn::TypeToSql(r.GetPropertyMap().GetColumnDataType()));
            propertySql.Append(castExp.c_str());
            }
        else
            propertySql.Append(r.GetSqlBuilder().ToString());

        //! Here we want rename or add column alias so it appear to be a basePropertyMap
        //! But we only do that if column name differ
        Utf8StringCP colAlias = nullptr;
        if (basePropertyMap != nullptr)
            {
            DbColumn const& basePropertyMapCol = basePropertyMap->GetAs<SingleColumnDataPropertyMap>()->GetColumn();
            if (!r.GetColumn().GetName().EqualsI(basePropertyMapCol.GetName()))
                colAlias = &basePropertyMapCol.GetName();
            }

        if (colAlias != nullptr)
            propertySql.AppendSpace().AppendEscaped(colAlias->c_str());

        propertySqlList.push_back(propertySql);
        }

    requireJoinTo = requireJoinToTableForDataProperties;
    sqlView.Append(propertySqlList);
    return SUCCESS;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                          11/2016
//---------------------------------------------------------------------------------------
//static
ConstraintECClassIdJoinInfo ConstraintECClassIdJoinInfo::Create(ConstraintECClassIdPropertyMap const& propertyMap, DbTable const& contextTable)
    {
    ConstraintECClassIdJoinInfo joinInfo;

    DbTable const* primaryTable = RequiresJoinTo(propertyMap);
    if (primaryTable == nullptr)
        return joinInfo;

    RelationshipClassMap const& relationshipMap = static_cast<RelationshipClassMap const&>(propertyMap.GetClassMap());
    if (!relationshipMap.IsMappedTo(contextTable))
        {
        BeAssert(false && "Relationship is not mapped to the context table. It must be one of Relationship.GetTables()");
        return joinInfo;
        }

    const bool isSource = propertyMap.GetEnd() == ECN::ECRelationshipEnd::ECRelationshipEnd_Source;
    ConstraintECInstanceIdPropertyMap const*  constraintId = isSource ? relationshipMap.GetSourceECInstanceIdPropMap() : relationshipMap.GetTargetECInstanceIdPropMap();
    BeAssert(constraintId != nullptr);
    SystemPropertyMap::PerTablePrimitivePropertyMap const* releventECClassIdPropertyMap = constraintId->FindDataPropertyMap(contextTable);
    if (releventECClassIdPropertyMap == nullptr)
        {
        BeAssert(false && "Expecting a property map for given context table");
        return joinInfo;
        }

    DbColumn const* primaryECInstanceIdColumn = primaryTable->GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
    DbColumn const* primaryECClassIdColumn = primaryTable->GetFilteredColumnFirst(DbColumn::Kind::ECClassId);
    DbColumn const* forignECInstanceIdColumn = &releventECClassIdPropertyMap->GetColumn();
    if (primaryECInstanceIdColumn == nullptr || primaryECClassIdColumn == nullptr || forignECInstanceIdColumn == nullptr)
        {
        BeAssert(false);
        return joinInfo;
        }

    joinInfo.m_joinIsRequired = true;
    joinInfo.m_propertyMap = &propertyMap;
    joinInfo.m_primaryECInstanceIdCol = primaryECInstanceIdColumn;
    joinInfo.m_primaryECClassIdCol = primaryECClassIdColumn;
    joinInfo.m_foreignECInstanceIdCol = forignECInstanceIdColumn;
    return joinInfo;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                          11/2016
//---------------------------------------------------------------------------------------
Utf8CP ConstraintECClassIdJoinInfo::GetSqlTableAlias()const
    {
    if (!RequiresJoin())
        {
        BeAssert(false && "ConstraintECClassIdJoinInfo::GetSqlTableAlias can only be called if RequiresJoin() is true");
        return nullptr;
        }

    return m_propertyMap->GetEnd() == ECN::ECRelationshipEnd::ECRelationshipEnd_Source ? "SourceECClassPrimaryTable" : "TargetECClassPrimaryTable";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                          11/2016
//---------------------------------------------------------------------------------------
Utf8CP ConstraintECClassIdJoinInfo::GetSqlECClassIdColumnAlias()const
    {
    if (!RequiresJoin())
        {
        BeAssert(false && "ConstraintECClassIdJoinInfo::GetSqlECClassIdColumnAlias can only be called if RequiresJoin() is true");
        return nullptr;
        }

    return m_propertyMap->GetEnd() == ECN::ECRelationshipEnd::ECRelationshipEnd_Source ? ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME : ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                          11/2016
//---------------------------------------------------------------------------------------
NativeSqlBuilder ConstraintECClassIdJoinInfo::GetNativeConstraintECClassIdSql(bool appendAlias) const
    {
    if (!RequiresJoin())
        {
        BeAssert(false && "ConstraintECClassIdJoinInfo::GetNativeConstraintECClassIdSql can only be called if RequiresJoin() is true");
        return NativeSqlBuilder();
        }

    NativeSqlBuilder sql;
    sql.AppendEscaped(GetSqlTableAlias()).AppendDot().AppendEscaped(m_primaryECClassIdCol->GetName().c_str());
    if (appendAlias)
        sql.AppendSpace().Append(GetSqlECClassIdColumnAlias());
    return sql;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                          11/2016
//---------------------------------------------------------------------------------------
NativeSqlBuilder ConstraintECClassIdJoinInfo::GetNativeJoinSql() const
    {
    if (!RequiresJoin())
        {
        BeAssert(false && "ConstraintECClassIdJoinInfo::GetNativeJoinSql can only be called if RequiresJoin() is true");
        return NativeSqlBuilder();
        }

    NativeSqlBuilder sql(" INNER JOIN ");
    sql.AppendEscaped(m_primaryECInstanceIdCol->GetTable().GetName().c_str())
        .AppendSpace()
        .AppendEscaped(GetSqlTableAlias())
        .Append(" ON ")
        .AppendEscaped(GetSqlTableAlias())
        .AppendDot()
        .AppendEscaped(m_primaryECInstanceIdCol->GetName().c_str())
        .Append(BooleanSqlOperator::EqualTo)
        .AppendEscaped(m_foreignECInstanceIdCol->GetTable().GetName().c_str()).AppendDot().AppendEscaped(m_foreignECInstanceIdCol->GetName().c_str());

    return sql;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                          11/2016
//---------------------------------------------------------------------------------------
DbTable const* ConstraintECClassIdJoinInfo::RequiresJoinTo(ConstraintECClassIdPropertyMap const& propertyMap, bool ignoreVirtualColumnCheck)
    {
    if (!propertyMap.IsMappedToSingleTable())
        return nullptr;

    DbTable const* table = propertyMap.GetTables().front();
    if (!ignoreVirtualColumnCheck)
        {
        if (propertyMap.IsVirtual(*table))
            return nullptr;
        }

    if (propertyMap.GetClassMap().GetType() == ClassMap::Type::RelationshipEndTable)
        {
        RelationshipClassEndTableMap const& map = static_cast<RelationshipClassEndTableMap const&>(propertyMap.GetClassMap());
        if (map.GetConstraintMap(map.GetReferencedEnd()).GetECClassIdPropMap() == &propertyMap)
            {
            SystemPropertyMap::PerTablePrimitivePropertyMap const* c = propertyMap.FindDataPropertyMap(*table);
            if (Enum::Contains(c->GetColumn().GetKind(), DbColumn::Kind::ECClassId))
                return table;
            }

        return nullptr;
        }

    if (!propertyMap.IsMappedToClassMapTables())
        return table;

    return nullptr;
    }


//*********************************ViewGenerator::SelectFromViewContext*****************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                     12/2016
//---------------------------------------------------------------------------------------
ViewGenerator::SelectFromViewContext::SelectFromViewContext(ECSqlPrepareContext const& prepareCtx, bool isPolymorphicQuery) 
    : Context(ViewType::SelectFromView, prepareCtx.GetECDb()), m_prepareCtx(prepareCtx), m_isPolymorphicQuery(isPolymorphicQuery)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                     12/2016
//---------------------------------------------------------------------------------------
bool ViewGenerator::SelectFromViewContext::IsECClassIdFilterEnabled() const
    {
    if (OptionsExp const* options = m_prepareCtx.GetCurrentScope().GetOptions())
        return !options->HasOption(OptionsExp::NOECCLASSIDFILTER_OPTION);

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                     12/2016
//---------------------------------------------------------------------------------------
bool ViewGenerator::SelectFromViewContext::IsInSelectClause(Utf8StringCR exp) const
    {
    return m_prepareCtx.GetSelectionOptions().IsSelected(exp);
    }

//*********************************ViewGenerator::SqlVisitor*****************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
ViewGenerator::ToSqlVisitor::ToSqlVisitor(Context const& context, DbTable const& tableFilter, Utf8CP classIdentifier, bool usePropertyNameAsAliasForSystemPropertyMaps, bool forECClassView)
    : IPropertyMapVisitor(), m_tableFilter(tableFilter), m_classIdentifier(classIdentifier), m_usePropertyNameAsAliasForSystemPropertyMaps(usePropertyNameAsAliasForSystemPropertyMaps),m_context(context) 
    {
    if (m_classIdentifier != nullptr && Utf8String::IsNullOrEmpty(m_classIdentifier))
        m_classIdentifier = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::ToSqlVisitor::_Visit(SystemPropertyMap const& propertyMap) const
    {
    switch (propertyMap.GetType())
        {
            case PropertyMap::Type::ConstraintECInstanceId:
                return ToNativeSql(*propertyMap.GetAs<ConstraintECInstanceIdPropertyMap>());
            case PropertyMap::Type::ConstraintECClassId:
                return ToNativeSql(*propertyMap.GetAs<ConstraintECClassIdPropertyMap>());
            case PropertyMap::Type::ECClassId:
                return ToNativeSql(*propertyMap.GetAs<ECClassIdPropertyMap>());
            case PropertyMap::Type::ECInstanceId:
                return ToNativeSql(*propertyMap.GetAs<ECInstanceIdPropertyMap>());
            default:
                BeAssert(false);
                return ERROR;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::ToSqlVisitor::ToNativeSql(SingleColumnDataPropertyMap const& propertyMap) const
    {
    if (propertyMap.GetType() == PropertyMap::Type::NavigationRelECClassId)
        return ToNativeSql(*propertyMap.GetAs<NavigationPropertyMap::RelECClassIdPropertyMap>());

    Result& result = Record(propertyMap);
    NativeSqlBuilder& sqlBuilder = result.GetSqlBuilderR();

    sqlBuilder.Append(m_classIdentifier, propertyMap.GetColumn().GetName().c_str());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::ToSqlVisitor::ToNativeSql(NavigationPropertyMap::RelECClassIdPropertyMap const& relClassIdPropMap) const
    {
    Result& result = Record(relClassIdPropMap);

    BeAssert(relClassIdPropMap.GetParent() != nullptr && relClassIdPropMap.GetParent()->GetType() == PropertyMap::Type::Navigation);
    NavigationPropertyMap::IdPropertyMap const& idPropMap = relClassIdPropMap.GetParent()->GetAs<NavigationPropertyMap>()->GetIdPropertyMap();

    NativeSqlBuilder idColStrBuilder;
    idColStrBuilder.Append(m_classIdentifier, idPropMap.GetColumn().GetName().c_str());

    NativeSqlBuilder relClassIdColStrBuilder;
    if (relClassIdPropMap.IsVirtual())
        relClassIdColStrBuilder = NativeSqlBuilder(relClassIdPropMap.GetDefaultClassId().ToString().c_str());
    else
        relClassIdColStrBuilder.Append(m_classIdentifier, relClassIdPropMap.GetColumn().GetName().c_str());
    //The RelECClassId should always be logically null if the respective NavId col is null
    //case exp must have the relclassid col name as alias
    if (m_context.GetViewType() == ViewType::ECClassView)
        result.GetSqlBuilderR().AppendFormatted("(CASE WHEN %s IS NULL THEN NULL ELSE %s END)", idColStrBuilder.ToString(), relClassIdColStrBuilder.ToString());
    else
        result.GetSqlBuilderR().AppendFormatted("(CASE WHEN %s IS NULL THEN NULL ELSE %s END) %s", idColStrBuilder.ToString(), relClassIdColStrBuilder.ToString(), relClassIdPropMap.GetColumn().GetName().c_str());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::ToSqlVisitor::ToNativeSql(ConstraintECInstanceIdPropertyMap const& propertyMap) const
    {
    SystemPropertyMap::PerTablePrimitivePropertyMap const* vmap = propertyMap.FindDataPropertyMap(m_tableFilter);
    if (vmap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    Result& result = Record(*vmap);
    result.GetSqlBuilderR().Append(m_classIdentifier, vmap->GetColumn().GetName().c_str());

    if (m_usePropertyNameAsAliasForSystemPropertyMaps)
        {
        if (!vmap->GetColumn().GetName().EqualsIAscii(propertyMap.GetAccessString()))
            result.GetSqlBuilderR().AppendSpace().Append(propertyMap.GetAccessString().c_str());
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::ToSqlVisitor::ToNativeSql(ECClassIdPropertyMap const& propertyMap) const
    {
    SystemPropertyMap::PerTablePrimitivePropertyMap const* vmap = propertyMap.FindDataPropertyMap(m_tableFilter);
    if (vmap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    const bool isVirtual = propertyMap.IsVirtual(m_tableFilter);

    Result& result = Record(*vmap);
    if (isVirtual)
        result.GetSqlBuilderR().Append(propertyMap.GetDefaultECClassId());
    else
        result.GetSqlBuilderR().Append(m_classIdentifier, vmap->GetColumn().GetName().c_str());

    if (m_usePropertyNameAsAliasForSystemPropertyMaps)
        {
        if (!vmap->GetColumn().GetName().EqualsIAscii(propertyMap.GetAccessString()) || isVirtual)
            result.GetSqlBuilderR().AppendSpace().Append(propertyMap.GetAccessString().c_str());
        }

    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::ToSqlVisitor::ToNativeSql(ConstraintECClassIdPropertyMap const& propertyMap) const
    {
    SystemPropertyMap::PerTablePrimitivePropertyMap const* vmap = propertyMap.FindDataPropertyMap(m_tableFilter);
    if (vmap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    const bool isVirtual = propertyMap.IsVirtual(m_tableFilter);
    Result& result = Record(*vmap);
    if (isVirtual)
        result.GetSqlBuilderR().Append(propertyMap.GetDefaultECClassId());
    else
        result.GetSqlBuilderR().Append(m_classIdentifier, vmap->GetColumn().GetName().c_str());

    if (m_usePropertyNameAsAliasForSystemPropertyMaps)
        {
        if (!vmap->GetColumn().GetName().EqualsIAscii(propertyMap.GetAccessString()) || isVirtual)
            result.GetSqlBuilderR().AppendSpace().Append(propertyMap.GetAccessString().c_str());
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::ToSqlVisitor::ToNativeSql(ECInstanceIdPropertyMap const& propertyMap) const
    {
    SystemPropertyMap::PerTablePrimitivePropertyMap const* vmap = propertyMap.FindDataPropertyMap(m_tableFilter);
    if (vmap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    Result& result = Record(*vmap);
    Utf8CP columnExp = vmap->GetColumn().GetName().c_str();
    result.GetSqlBuilderR().Append(m_classIdentifier, columnExp);
    if (m_usePropertyNameAsAliasForSystemPropertyMaps)
        {
        if (!vmap->GetColumn().GetName().EqualsIAscii(propertyMap.GetAccessString()))
            result.GetSqlBuilderR().AppendSpace().Append(propertyMap.GetAccessString().c_str());
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
ViewGenerator::ToSqlVisitor::Result& ViewGenerator::ToSqlVisitor::Record(SingleColumnDataPropertyMap const& propertyMap) const
    {
    m_resultSetByAccessString[propertyMap.GetAccessString().c_str()] = m_resultSet.size();
    m_resultSet.push_back(Result(propertyMap));
    return m_resultSet.back();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
