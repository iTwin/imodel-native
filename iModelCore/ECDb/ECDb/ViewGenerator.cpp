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
    PERFLOG_START("ECDb", "Schema import> Create updatable views");

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb,
                                     "SELECT distinct c.Id FROM " TABLE_Class " c, ec_ClassMap cm, " TABLE_ClassHasBaseClasses " cc "
                                     "WHERE c.Id = cm.ClassId AND c.Id = cc.BaseClassId AND c.Type = " SQLVAL_ECClassType_Entity " AND "
                                     "cm.MapStrategy<>" SQLVAL_MapStrategy_NotMapped " AND cm.MapStrategy<>" SQLVAL_MapStrategy_ExistingTable))
        return ERROR;

    DbMap const& map = ecdb.Schemas().GetDbMap();
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ECClassId classId = stmt.GetValueId<ECClassId>(0);
        ECClassCP ecClass = ecdb.Schemas().GetClass(classId);
        if (ecClass == nullptr)
            return ERROR;

        ClassMapCP classMap = map.GetClassMap(*ecClass);
        if (classMap == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }

        if (classMap->GetMapStrategy().GetStrategy() == MapStrategy::NotMapped || classMap->GetMapStrategy().GetStrategy() == MapStrategy::ExistingTable ||
            classMap->GetClass().GetClassType() != ECClassType::Entity)
            {
            BeAssert(false && "Should have been filtered out by the SQL already");
            continue;
            }

        if (classMap->GetClass().GetEntityClassCP()->IsMixin())
            continue; //mixins are not updatable -> no view needed

        if (CreateUpdatableViewIfRequired(ecdb, *classMap) != SUCCESS)
            return ERROR;
        }

    PERFLOG_FINISH("ECDb", "Schema import> Create updatable views");
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2016
//+---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewGenerator::CreateUpdatableViewIfRequired(ECDbCR ecdb, ClassMap const& classMap)
    {
    UpdatableViewContext ctx(ecdb);

    StorageDescription const& descr = classMap.GetStorageDescription();

    if (!descr.HasNonVirtualHorizontalPartitions())
        return SUCCESS; //entire hierarchy is abstract -> no updatable view needed

    std::vector<Partition> const& horizPartitions = descr.GetHorizontalPartitions();

    Partition const& rootPartition = descr.GetRootHorizontalPartition();
    DbTable const& rootTable = rootPartition.GetTable();
    //if only root table is non-virtual and other partitions are virtual, no updatable view needed either
    //Note, this should not happen, as a non-abstract class cannot have abstract subclass
    if (rootTable.GetType() != DbTable::Type::Virtual && !descr.HasMultipleNonVirtualHorizontalPartitions())
        {
        BeAssert(horizPartitions.size() == 1);
        return SUCCESS;
        }

    DbColumn const* rootPartitionIdColumn = rootTable.FindFirst(DbColumn::Kind::ECInstanceId);

    Utf8String updatableViewName;
    updatableViewName.Sprintf("_%s_%s", classMap.GetClass().GetSchema().GetAlias().c_str(), classMap.GetClass().GetName().c_str());

    std::vector<Utf8String> triggerDdlList;

    std::set<DbTable const*> updateTables;
    std::set<DbTable const*> deleteTables;
    std::vector<DbTable const*> joinedTables;
    std::vector<DbTable const*> primaryTables;

    for (Partition const& horizPartition : horizPartitions)
        {
        DbTable const& horizPartitionTable = horizPartition.GetTable();
        if (horizPartitionTable.GetType() == DbTable::Type::Virtual)
            continue;

        //If a class map has a subclass mapping to ExistingTable the class is not updatable polymorphically
        //and therefore doesn't need an updatable view
        if (horizPartitionTable.GetType() == DbTable::Type::Existing)
            return SUCCESS;

        updateTables.insert(&horizPartitionTable);
        deleteTables.insert(&horizPartitionTable);
        if (horizPartitionTable.GetType() == DbTable::Type::Joined)
            joinedTables.push_back(&horizPartitionTable);

        if (horizPartitionTable.GetType() == DbTable::Type::Primary)
            primaryTables.push_back(&horizPartitionTable);
        }
    //Remove any primary table
    for (DbTable const* joinedTable : joinedTables)
        {
        BeAssert(joinedTable->GetLinkNode().GetParent() != nullptr);
        updateTables.erase(&joinedTable->GetLinkNode().GetParent()->GetTable());
        }

    for (DbTable const* joinedTable : joinedTables)
        {
        BeAssert(joinedTable->GetLinkNode().GetParent() != nullptr);
        deleteTables.insert(&joinedTable->GetLinkNode().GetParent()->GetTable());
        deleteTables.erase(joinedTable);
        }

    for (Partition const& horizPartition : horizPartitions)
        {
        if (horizPartition.GetTable().GetType() == DbTable::Type::Virtual)
            continue;

        DbColumn const* partitionIdColumn = horizPartition.GetTable().FindFirst(DbColumn::Kind::ECInstanceId);
        Utf8String triggerNamePrefix;
        triggerNamePrefix.Sprintf("%s_%s", rootTable.GetName().c_str(), horizPartition.GetTable().GetName().c_str());

        Utf8String whenClause;
        if (horizPartition.NeedsECClassIdFilter())
            horizPartition.AppendECClassIdFilterSql(whenClause, "OLD.ECClassId");
        else
            whenClause.append("OLD.ECClassId=").append(horizPartition.GetRootClassId().ToString());

        if (deleteTables.find(&horizPartition.GetTable()) != deleteTables.end())
            {//<----------DELETE trigger----------
            Utf8String ddl("CREATE TRIGGER [");
            ddl.append(triggerNamePrefix).append("_delete]");
            ddl.append(" INSTEAD OF DELETE ON ").append(updatableViewName).append(" WHEN ").append(whenClause);

            Utf8String body;
            body.Sprintf(" BEGIN DELETE FROM [%s] WHERE [%s]=OLD.[%s]; END", horizPartition.GetTable().GetName().c_str(), partitionIdColumn->GetName().c_str(), rootPartitionIdColumn->GetName().c_str());
            ddl.append(body);
            triggerDdlList.push_back(ddl);
            }

        if (updateTables.find(&horizPartition.GetTable()) != updateTables.end())
            {//<----------UPDATE trigger----------
            ECClassCP rootClass = ecdb.Schemas().GetClass(horizPartition.GetRootClassId());
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
            body.Sprintf(" BEGIN UPDATE [%s] SET %s WHERE [%s]=OLD.[%s]; END", horizPartition.GetTable().GetName().c_str(), setClause.ToString(), partitionIdColumn->GetName().c_str(), rootPartitionIdColumn->GetName().c_str());
            ddl.append(body);

            triggerDdlList.push_back(ddl);
            }
        }

    NativeSqlBuilder viewBodySql;
    if (GenerateViewSql(viewBodySql, ctx, classMap) != SUCCESS)
        return ERROR;

    Utf8String updatableViewDdl;
    updatableViewDdl.Sprintf("CREATE VIEW %s AS %s", updatableViewName.c_str(), viewBodySql.ToString());

    if (ctx.GetECDb().ExecuteSql(updatableViewDdl.c_str()) != BE_SQLITE_OK)
        {
        LOG.errorv("Failed to create updatable view for ECClass '%s'. %s (SQL: %s)", classMap.GetClass().GetFullName(),
                   ctx.GetECDb().GetLastError().c_str(), updatableViewDdl.c_str());
        return ERROR;
        }

    for (Utf8StringCR triggerDdl : triggerDdlList)
        {
        if (ctx.GetECDb().ExecuteSql(triggerDdl.c_str()) != BE_SQLITE_OK)
            {
            LOG.errorv("Failed to create trigger for updatable view for ECClass '%s'. %s (SQL: %s)", classMap.GetClass().GetFullName(),
                       ctx.GetECDb().GetLastError().c_str(), triggerDdl.c_str());
            return ERROR;
            }
        }

    CachedStatementPtr stmt = ctx.GetECDb().GetCachedStatement("UPDATE ec_Table SET UpdatableViewName=? WHERE Id=?");
    if (stmt == nullptr ||
        BE_SQLITE_OK != stmt->BindText(1, updatableViewName, Statement::MakeCopy::No) ||
        BE_SQLITE_OK != stmt->BindId(2, rootTable.GetId()) ||
        BE_SQLITE_DONE != stmt->Step())
        {
        BeAssert(false && "Failed to persist UpdatableViewName in ec_Table");
        return ERROR;
        }

    if (ctx.GetECDb().GetModifiedRowCount() != 1)
        {
        BeAssert(false && "ec_Table row does not exist yet for which the updatable view was created");
        return ERROR;
        }

    const_cast<DbTable&> (rootTable).SetUpdatableViewInfo(updatableViewName.c_str());
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2016
//+---------------+---------------+---------------+---------------+---------------+--------
//static 
BentleyStatus ViewGenerator::DropUpdatableViews(ECDbCR ecdb)
    {
    PERFLOG_START("ECDb", "Schema import> Drop updatable views");

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "SELECT UpdatableViewName FROM ec_Table WHERE UpdatableViewName IS NOT NULL"))
        return ERROR;

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        Utf8String dropViewSql;
        dropViewSql.Sprintf("DROP VIEW IF EXISTS %s", stmt.GetValueText(0));
        if (ecdb.ExecuteSql(dropViewSql.c_str()) != BE_SQLITE_OK)
            return ERROR;
        }

    PERFLOG_FINISH("ECDb", "Schema import> Drop updatable views");
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2016
//+---------------+---------------+---------------+---------------+---------------+--------
//static 
BentleyStatus ViewGenerator::CreateECClassViews(ECDbCR ecdb)
    {
    PERFLOG_START("ECDb", "Create ECClass views");
    if (ecdb.IsReadonly())
        {
        ecdb.GetECDbImplR().GetIssueReporter().Report("Can only call ECDb::CreateClassViewsInDb() on an ECDb file with read-write access.");
        return ERROR;
        }

    if (DropECClassViews(ecdb) != SUCCESS)
        return ERROR;

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, 
                                     "SELECT c.Id FROM " TABLE_Class " c, ec_ClassMap cm WHERE c.Id = cm.ClassId AND "
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
    const BentleyStatus stat = CreateECClassViews(ecdb, classIds);
    PERFLOG_FINISH("ECDb", "Create ECClass views");
    return stat;
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

        ECClassCP ecClass = ecdb.Schemas().GetClass(classId);
        if (ecClass == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }

        ClassMapCP classMap = ecdb.Schemas().GetDbMap().GetClassMap(*ecClass);
        if (classMap == nullptr)
            {
            BeAssert(false);
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
    PERFLOG_START("ECDb", "Drop ECClass views");

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb,
                                     "SELECT ('DROP VIEW IF EXISTS [' || s.Alias || '.' || c.Name || '];') FROM ec_Class c "
                                     "INNER JOIN ec_Schema s ON s.Id=c.SchemaId"))
        return ERROR;

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        Utf8CP classViewSQL = stmt.GetValueText(0);
        if (ecdb.ExecuteSql(classViewSQL) != BE_SQLITE_OK)
            return ERROR;
        }

    PERFLOG_FINISH("ECDb", "Drop ECClass views");
    return SUCCESS;
    }



//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2016
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ViewGenerator::GenerateUpdateTriggerSetClause(NativeSqlBuilder& sql, ClassMap const& baseClassMap, ClassMap const& derivedClassMap)
    {
    sql.Clear();
    std::vector<Utf8String> values;
    SearchPropertyMapVisitor baseClassDataPropMapVisitor(PropertyMap::Type::Data, false);
    baseClassMap.GetPropertyMaps().AcceptVisitor(baseClassDataPropMapVisitor);
    for (PropertyMap const* baseClassDataPropertyMap : baseClassDataPropMapVisitor.Results())
        {
        PropertyMap const* derivedPropMap = derivedClassMap.GetPropertyMaps().Find(baseClassDataPropertyMap->GetAccessString().c_str());
        if (derivedPropMap == nullptr || !derivedPropMap->IsData())
            {
            BeAssert(false);
            return ERROR;
            }

        GetColumnsPropertyMapVisitor baseColumnVisitor, derivedColumnVisitor;
        baseClassDataPropertyMap->AcceptVisitor(baseColumnVisitor);
        derivedPropMap->AcceptVisitor(derivedColumnVisitor);
        const size_t colCount = baseColumnVisitor.GetColumnCount();
        if (colCount != derivedColumnVisitor.GetColumns().size())
            {
            BeAssert(false);
            return ERROR;
            }

        for (size_t i = 0; i < colCount; i++)
            {
            DbColumn const* derivedPropMapCol = derivedColumnVisitor.GetColumns()[i];
            DbColumn const* basePropMapCol = baseColumnVisitor.GetColumns()[i];
            if (derivedPropMapCol->GetPersistenceType() == PersistenceType::Virtual)
                continue;

            Utf8String str;
            str.Sprintf("[%s] = NEW.[%s]", derivedPropMapCol->GetName().c_str(), basePropMapCol->GetName().c_str());
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
            return RenderRelationshipClassEndTableMap(viewSql, ctx, classMap.GetAs<RelationshipClassEndTableMap>());

        return RenderRelationshipClassLinkTableMap(viewSql, ctx, classMap.GetAs<RelationshipClassLinkTableMap>());
        }

    if (classMap.IsMixin())
        return RenderMixinClassMap(viewSql, ctx, classMap);

    return RenderEntityClassMap(viewSql, ctx, classMap);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                          06/2017
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::RenderMixinClassMap(bmap<Utf8String, bpair<DbTable const*, bvector<ECN::ECClassId>>, CompareIUtf8Ascii>& selectClauses, Context& ctx, ClassMap const& mixInClassMap, ClassMap const& derivedClassMap)
    {
    if (!derivedClassMap.IsMixin())
        {
        NativeSqlBuilder viewSql;
        DbTable const* contextTable;
        GetTablesPropertyMapVisitor visitor(PropertyMap::Type::Data);
        derivedClassMap.GetPropertyMaps().AcceptVisitor(visitor);
        if (visitor.GetTables().empty() || visitor.GetTables().size() == 2)
            contextTable = &derivedClassMap.GetJoinedOrPrimaryTable();
        else
            {
            BeAssert(visitor.GetTables().size() == 1);
            if (visitor.GetTables().size() != 1)
                return ERROR;

            contextTable = *visitor.GetTables().begin();
            }

        if (RenderEntityClassMap(viewSql, ctx, derivedClassMap, *contextTable, &mixInClassMap) != SUCCESS)
            return ERROR;

        Utf8String sql = viewSql.ToString();
        auto itor = selectClauses.find(sql);
        if (itor == selectClauses.end())
            itor = selectClauses.insert(make_bpair(std::move(sql), make_bpair(contextTable, bvector<ECN::ECClassId>()))).first;

        itor->second.second.push_back(derivedClassMap.GetClass().GetId());

        if (!selectClauses.empty())
            if (ctx.GetViewType() == ViewType::ECClassView)
                ctx.GetAs<ECClassViewContext>().StopCaptureViewColumnNames();
        }

    for (ClassMapCP derivedClassMap : derivedClassMap.GetDerivedClassMaps())
        if (RenderMixinClassMap(selectClauses, ctx, mixInClassMap, *derivedClassMap) != SUCCESS)
            return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                          06/2017
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::RenderMixinClassMap(NativeSqlBuilder& viewSql, Context& ctx, ClassMap const& mixInClassMap)
    {
    //! Drill down and find all the classmap.
    if (ctx.GetViewType() == ViewType::UpdatableView)
        {
        BeAssert(false);
        return ERROR;
        }

    if (ctx.GetViewType() == ViewType::SelectFromView)
        {
        if (!ctx.GetAs<SelectFromViewContext>().IsPolymorphicQuery())
            return RenderNullView(viewSql, ctx, mixInClassMap);

        viewSql.AppendParenLeft();
        }

    bmap<Utf8String, bpair<DbTable const*, bvector<ECN::ECClassId>>, CompareIUtf8Ascii> selectClauses;
    if (RenderMixinClassMap(selectClauses, ctx, mixInClassMap, mixInClassMap) != SUCCESS)
        return ERROR;

    NativeSqlBuilder selectClause;
    bool first = true;
    if (selectClauses.empty())
        return  RenderNullView(viewSql, ctx, mixInClassMap);

    for (auto const& kvp : selectClauses)
        {
        bvector<ECClassId> const& classIds = kvp.second.second;
        DbTable const* table = kvp.second.first;
        selectClause.Append(kvp.first.c_str());
        DbColumn const* classId = table->FindFirst(DbColumn::Kind::ECClassId);
        if (classId->GetPersistenceType() == PersistenceType::Physical)
            {
            selectClause.Append(" WHERE ").Append(table->GetName().c_str(), classId->GetName().c_str());
            if (classIds.size() == 1)
                selectClause.Append("=").Append(classIds.front());
            else
                {
                selectClause.Append("IN ").AppendParenLeft();
                for (int i = 0; i < classIds.size(); i++)
                    {
                    if (i > 0)
                        selectClause.AppendComma();

                    selectClause.Append(classIds[i]);
                    }

                selectClause.AppendParenRight();
                }
            }
        if (first)
            first = false;
        else
            viewSql.Append(" UNION ALL ");

        viewSql.AppendLine(selectClause.ToString());
        selectClause.Clear();
        }

    if (ctx.GetViewType() == ViewType::SelectFromView)
        viewSql.AppendParenRight();

    return SUCCESS;
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
        if (partition->GetTable().GetType() == DbTable::Type::Virtual)
            continue;

        //For vertical partition we like to skip the first primary partition table.
        if (isVertical)
            {
            if (&partition->GetTable() == &storageDesc.GetRootHorizontalPartition().GetTable())
                continue;
            }

        NativeSqlBuilder view;
        ECClass const* tableRootClass = ctx.GetECDb().Schemas().GetClass(partition->GetRootClassId());
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

        if (SystemPropertyMap::PerTableIdPropertyMap const* classIdPropertyMap = tableRootClassMap->GetECClassIdPropertyMap()->FindDataPropertyMap(partition->GetTable()))
            {
            const bool isSelectFromView = ctx.GetViewType() == ViewType::SelectFromView;
            if (classIdPropertyMap->GetColumn().GetPersistenceType() == PersistenceType::Physical && 
                (!isSelectFromView || ctx.GetAs<SelectFromViewContext>().IsECClassIdFilterEnabled()))
                {
                const bool considerSubclasses = !isSelectFromView || ctx.GetAs<SelectFromViewContext>().IsPolymorphicQuery();

                Utf8String whereClause;
                if (SUCCESS != GenerateECClassIdFilter(whereClause, classMap, partition->GetTable(), classIdPropertyMap->GetColumn(), considerSubclasses))
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
    bset<DbTable const*> requireJoinTo;
    if (RenderPropertyMaps(viewSql, ctx, requireJoinTo, classMap, contextTable, castAs, PropertyMap::Type::Data | PropertyMap::Type::ECInstanceId | PropertyMap::Type::ECClassId) != SUCCESS)
        return ERROR;

    viewSql.Append(" FROM ").AppendEscaped(contextTable.GetName().c_str());
    //Join necessary table for table 
    for(DbTable const* to : requireJoinTo)
        {
        DbColumn const* primaryKey = contextTable.FindFirst(DbColumn::Kind::ECInstanceId);
        DbColumn const* fkKey = to->FindFirst(DbColumn::Kind::ECInstanceId);
        viewSql.Append(" INNER JOIN ").AppendEscaped(to->GetName().c_str());
        viewSql.Append(" ON ").AppendEscaped(contextTable.GetName().c_str()).AppendDot().AppendEscaped(primaryKey->GetName().c_str());
        viewSql.Append(ExpHelper::ToSql(BooleanSqlOperator::EqualTo)).AppendEscaped(to->GetName().c_str()).AppendDot().AppendEscaped(fkKey->GetName().c_str());
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
            viewSql.Append("NULL ").AppendEscaped(propertyMap->GetAs<SingleColumnDataPropertyMap>().GetColumn().GetName().c_str());

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
        if (partition.GetTable().GetType() == DbTable::Type::Virtual)
            continue;

        NativeSqlBuilder view;
        ECClass const* relationshipECClass = ctx.GetECDb().Schemas().GetClass(partition.GetRootClassId());
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

        RelationshipClassLinkTableMap const& contextRelationship = classMap->GetAs<RelationshipClassLinkTableMap>();
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
        if (SystemPropertyMap::PerTableIdPropertyMap const* classIdDataPropertyMap = classIdPropMap->FindDataPropertyMap(partition.GetTable()))
            {
            const bool isSelectFromView = ctx.GetViewType() == ViewType::SelectFromView;
            if (classIdDataPropertyMap->GetColumn().GetPersistenceType() == PersistenceType::Physical && (!isSelectFromView || ctx.GetAs<SelectFromViewContext>().IsECClassIdFilterEnabled()))
                {
                Utf8String whereClause;
                const bool considerSubclasses = !isSelectFromView || ctx.GetAs<SelectFromViewContext>().IsPolymorphicQuery();
                if (SUCCESS != GenerateECClassIdFilter(whereClause, relationMap, partition.GetTable(), classIdDataPropertyMap->GetColumn(), considerSubclasses))
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
    for (auto const& key : relationMap.GetPartitionView().GetPartitionMap())
        {
        for (auto const & partition : key.second)
            {
            NativeSqlBuilder view;
            view.Append("SELECT ");

            view.Append(partition->GetECInstanceId().GetTable().GetName().c_str(), partition->GetECInstanceId().GetName().c_str()).AppendSpace().Append(ECDBSYS_PROP_ECInstanceId).AppendComma();
            if (partition->GetECClassId().GetPersistenceType() == PersistenceType::Virtual)
                view.Append(relationMap.GetClass().GetId()).AppendSpace().Append(ECDBSYS_PROP_ECClassId).AppendComma();
            else
                view.Append(partition->GetECClassId().GetTable().GetName().c_str(), partition->GetECClassId().GetName().c_str()).AppendSpace().Append(ECDBSYS_PROP_ECClassId).AppendComma();

            view.Append(partition->GetSourceECInstanceId().GetTable().GetName().c_str(), partition->GetSourceECInstanceId().GetName().c_str()).AppendSpace().Append(ECDBSYS_PROP_SourceECInstanceId).AppendComma();
            if (partition->GetSourceECClassId().GetPersistenceType() == PersistenceType::Virtual)
                view.Append(relationMap.GetRelationshipClass().GetSource().GetConstraintClasses().front()->GetId()).AppendSpace().Append(ECDBSYS_PROP_SourceECClassId).AppendComma();
            else
                view.Append(partition->GetSourceECClassId().GetTable().GetName().c_str(), partition->GetSourceECClassId().GetName().c_str()).AppendSpace().Append(ECDBSYS_PROP_SourceECClassId).AppendComma();

            view.Append(partition->GetTargetECInstanceId().GetTable().GetName().c_str(), partition->GetTargetECInstanceId().GetName().c_str()).AppendSpace().Append(ECDBSYS_PROP_TargetECInstanceId).AppendComma();
            if (partition->GetTargetECClassId().GetPersistenceType() == PersistenceType::Virtual)
                view.Append(relationMap.GetRelationshipClass().GetTarget().GetConstraintClasses().front()->GetId()).AppendSpace().Append(ECDBSYS_PROP_TargetECClassId).AppendComma();
            else
                view.Append(partition->GetTargetECClassId().GetTable().GetName().c_str(), partition->GetTargetECClassId().GetName().c_str()).AppendSpace().Append(ECDBSYS_PROP_TargetECClassId);

            view.AppendSpace().Append(partition->GetECInstanceId().GetTable().GetName().c_str());
            view.Append(" FROM ").Append(partition->GetECInstanceId().GetTable().GetName().c_str());

            DbColumn const& refClassId = relationMap.GetReferencedEnd() == ECRelationshipEnd::ECRelationshipEnd_Source ? partition->GetSourceECClassId() : partition->GetTargetECClassId();
            DbColumn const& refId = relationMap.GetReferencedEnd() == ECRelationshipEnd::ECRelationshipEnd_Source ? partition->GetSourceECInstanceId() : partition->GetTargetECInstanceId();
            if (refClassId.GetPersistenceType() == PersistenceType::Physical && refClassId.GetTable().GetId() != partition->GetECInstanceId().GetTable().GetId())
                {
                DbColumn const* id = refClassId.GetTable().FindFirst(DbColumn::Kind::ECInstanceId);
                view.Append(" INNER JOIN ").Append(refClassId.GetTable().GetName().c_str()).Append(" ON ").Append(id->GetTable().GetName().c_str(), id->GetName().c_str()).Append(ExpHelper::ToSql(BooleanSqlOperator::EqualTo)).Append(refId.GetTable().GetName().c_str(), refId.GetName().c_str());
                }

            view.Append(" WHERE ").Append(refId.GetTable().GetName().c_str(), refId.GetName().c_str()).Append(" IS NOT NULL");
            if (partition->GetECClassId().GetPersistenceType() == PersistenceType::Physical)
                {
                const bool isPolymorphic = ctx.GetViewType() == ViewType::SelectFromView ? ctx.GetAs<SelectFromViewContext>().IsPolymorphicQuery() : true;
                view.Append(" AND ").Append(partition->GetECClassId().GetTable().GetName().c_str(), partition->GetECClassId().GetName().c_str());
                if (isPolymorphic)
                    view.Append(" IN (SELECT ClassId FROM " TABLE_ClassHierarchyCache " WHERE BaseClassId=").Append(relationMap.GetClass().GetId()).Append(")");
                else
                    view.Append(ExpHelper::ToSql(BooleanSqlOperator::EqualTo)).Append(relationMap.GetClass().GetId());
                }

            unionList.push_back(view);
            }
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
    ToSqlVisitor sqlVisitor(ctx, contextTable, contextTable.GetName().c_str(), ToSqlVisitor::ColumnAliasMode::SystemPropertyName);
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
            ToSqlVisitor constraintSqlVisitor(ctx, *table, contextTable.GetName().c_str(), ToSqlVisitor::ColumnAliasMode::SystemPropertyName);
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
            ToSqlVisitor constraintSqlVisitor(ctx, *table, contextTable.GetName().c_str(), ToSqlVisitor::ColumnAliasMode::SystemPropertyName);
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

    NativeSqlBuilder dataPropertySql;
    bset<DbTable const*> requireJoinTo;
    if (RenderPropertyMaps(dataPropertySql, ctx, requireJoinTo, relationMap, contextTable, nullptr, PropertyMap::Type::Data, requiresJoin) != SUCCESS)
        return ERROR;

    if (!requireJoinTo.empty())
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
BentleyStatus ViewGenerator::RenderPropertyMaps(NativeSqlBuilder& sqlView, Context& ctx, bset<DbTable const*>& requireJoinTo, ClassMapCR classMap, DbTable const&  contextTable, ClassMapCP baseClass, PropertyMap::Type filter, bool requireJoin)
    {
    requireJoinTo.clear();
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

            if (propertyMap->IsData())
                {
                DataPropertyMap const& dataPropertyMap = propertyMap->GetAs<DataPropertyMap>();
                if (&dataPropertyMap.GetTable() != &contextTable)
                    requireJoinTo.insert(&dataPropertyMap.GetTable());
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
                if (propertyMap->IsData())
                    {
                    DataPropertyMap const& dataPropertyMap = propertyMap->GetAs<DataPropertyMap>();
                    if (&dataPropertyMap.GetTable() != &contextTable)
                        requireJoinTo.insert (&dataPropertyMap.GetTable());
                    }

                propertyMaps.push_back(std::make_pair(propertyMap, nullptr));
                }
            }
        }

    if (propertyMaps.empty())
        return SUCCESS;

    Utf8CP systemContextTableAlias = !requireJoinTo.empty() ? contextTable.GetName().c_str() : nullptr;
    NativeSqlBuilder::List propertySqlList;
    for (auto const& kvp : propertyMaps)
        {
        PropertyMap const* rootPropertyMap = kvp.second;
        PropertyMap const* propertyMap = kvp.first;
        BeAssert(dynamic_cast<CompoundDataPropertyMap const*>(propertyMap) == nullptr);
        BeAssert(dynamic_cast<CompoundDataPropertyMap const*>(rootPropertyMap) == nullptr);

        if (ctx.GetViewType() == ViewType::ECClassView && ctx.GetAs<ECClassViewContext>().MustCaptureViewColumnNames())
            ctx.GetAs<ECClassViewContext>().AddViewColumnName(propertyMap->GetAccessString());

        // We only need table qualifier if there is at least one data property selected that require joining to another table
        // In this case all data properties are table qualified name to ensure no conflict between two tables columns.
        // System property never require a join but therefore requireJoinToTableForDataProperties = nullptr if no data property was chosen
        if (propertyMap->IsSystem())
            {
            SystemPropertyMap const& systemPropertyMap = propertyMap->GetAs<SystemPropertyMap>();

            ToSqlVisitor::ColumnAliasMode colAliasMode = ToSqlVisitor::ColumnAliasMode::SystemPropertyName;
            Utf8CP colAlias = nullptr;
            if (ctx.GetViewType() == ViewType::UpdatableView)
                {
                colAliasMode = ToSqlVisitor::ColumnAliasMode::NoAlias;//we append the alias ourselves

                if (rootPropertyMap == nullptr)
                    {
                    SystemPropertyMap::PerTableIdPropertyMap const* perTableSystemPropMap = systemPropertyMap.FindDataPropertyMap(contextTable);
                    if (perTableSystemPropMap == nullptr)
                        {
                        BeAssert(false);
                        return ERROR;
                        }

                    colAlias = perTableSystemPropMap->GetColumn().GetName().c_str();
                    }
                else
                    {
                    if (!rootPropertyMap->IsSystem())
                        {
                        BeAssert(false);
                        return ERROR;
                        }

                    SystemPropertyMap const* rootSystemPropertyMap = &rootPropertyMap->GetAs<SystemPropertyMap>();
                    SystemPropertyMap::PerTableIdPropertyMap const* perTableSystemPropMap = rootSystemPropertyMap->FindDataPropertyMap(baseClass->GetPrimaryTable());
                    if (perTableSystemPropMap == nullptr)
                        {
                        BeAssert(false);
                        return ERROR;
                        }

                    colAlias = perTableSystemPropMap->GetColumn().GetName().c_str();
                    }
                }

            ToSqlVisitor toSqlVisitor(ctx, contextTable, systemContextTableAlias, colAliasMode);
            if (SUCCESS != propertyMap->AcceptVisitor(toSqlVisitor) || toSqlVisitor.GetResultSet().empty())
                {
                BeAssert(false);
                return ERROR;
                }

            ToSqlVisitor::Result const& visitorResult = toSqlVisitor.GetResultSet().front();
            propertySqlList.push_back(visitorResult.GetSqlBuilder());
            if (colAlias != nullptr)
                {
                //only append alias if it differs from the actual snippet.
                //literal sql snippets are literal ECClassIds which are appended for virtual class id cols 
                if (visitorResult.IsLiteralSqlSnippet() ||
                    !visitorResult.GetColumn().GetName().EqualsIAscii(colAlias))
                    {
                    propertySqlList.back().AppendSpace().AppendEscaped(colAlias);
                    }
                }
                
            continue;
            }

        BeAssert(propertyMap->IsData());
        SingleColumnDataPropertyMap const& dataProperty = propertyMap->GetAs<SingleColumnDataPropertyMap>();
        //! Join table does not require casting as we only split table into exactly two possible tables and only if shared table is enabled.
        if (requireJoinTo.end() != requireJoinTo.find(&dataProperty.GetTable()) || requireJoin)
            {
            ToSqlVisitor toSqlVisitor(ctx, dataProperty.GetTable(), dataProperty.GetTable().GetName().c_str(), ToSqlVisitor::ColumnAliasMode::NoAlias);
            if (baseClass && baseClass->IsMixin())
                   toSqlVisitor.DoNotAddColumnAliasForComputedExpression();

            if (SUCCESS != dataProperty.AcceptVisitor(toSqlVisitor) || toSqlVisitor.GetResultSet().empty())
                {
                BeAssert(false);
                return ERROR;
                }

            ToSqlVisitor::Result const& r = toSqlVisitor.GetResultSet().front();
            //! This is where we generate strong type column for shared column for ECClassView
            NativeSqlBuilder propertySql;
            if (ctx.GetViewType() == ViewType::ECClassView)
                {
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

            propertySql.Append(r.GetSqlBuilder().ToString());
            Utf8StringCP colAlias = nullptr;
            if (rootPropertyMap != nullptr)
                {
                if (!Enum::Contains(PropertyMap::Type::SingleColumnData, rootPropertyMap->GetType()))
                    return ERROR;

                DbColumn const& rootPropertyMapCol = rootPropertyMap->GetAs<SingleColumnDataPropertyMap>().GetColumn();
                if (!r.GetColumn().GetName().EqualsI(rootPropertyMapCol.GetName()))
                    colAlias = &rootPropertyMapCol.GetName();
                }

            if (colAlias != nullptr)
                propertySql.AppendSpace().AppendEscaped(colAlias->c_str());

            propertySqlList.push_back(propertySql);
            continue;
            }

        //no join needed
        ToSqlVisitor toSqlVisitor(ctx, contextTable, systemContextTableAlias, ToSqlVisitor::ColumnAliasMode::NoAlias);
        if (baseClass && baseClass->IsMixin())
            toSqlVisitor.DoNotAddColumnAliasForComputedExpression();

        if (SUCCESS != dataProperty.AcceptVisitor(toSqlVisitor) || toSqlVisitor.GetResultSet().empty())
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
        if (rootPropertyMap != nullptr)
            {
            DbColumn const& basePropertyMapCol = rootPropertyMap->GetAs<SingleColumnDataPropertyMap>().GetColumn();
            if (!r.GetColumn().GetName().EqualsI(basePropertyMapCol.GetName()))
                colAlias = &basePropertyMapCol.GetName();
            }

        if (colAlias != nullptr)
            propertySql.AppendSpace().AppendEscaped(colAlias->c_str());

        propertySqlList.push_back(propertySql);
        }

    sqlView.Append(propertySqlList);
    return SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    10 / 2015
//------------------------------------------------------------------------------------------
//static
    BentleyStatus ViewGenerator::GenerateECClassIdFilter(Utf8StringR filterSqlExpression, ClassMap const& classMap, DbTable const& table, DbColumn const& classIdColumn, bool polymorphic)
    {
    if (table.GetType() == DbTable::Type::Virtual)
        return SUCCESS;

    StorageDescription const& desc = classMap.GetStorageDescription();
    Partition const* partition = desc.GetHorizontalPartition(table);
    if (partition == nullptr)
        {
        if (!desc.GetVerticalPartitions().empty())
            partition = desc.GetVerticalPartition(table);

        if (partition == nullptr)
            {
            BeAssert(false && "Should always find a partition for the given table");
            return ERROR;
            }
        }

    Utf8String classIdColSql("[");
    classIdColSql.append(table.GetName()).append("].").append(classIdColumn.GetName());

    Utf8Char classIdStr[ECClassId::ID_STRINGBUFFER_LENGTH];
    classMap.GetClass().GetId().ToString(classIdStr);

    if (!polymorphic)
        {
        //if partition's table is only used by a single class, no filter needed     
        if (partition->IsSharedTable())
            filterSqlExpression.append(classIdColSql).append("=").append(classIdStr);

        return SUCCESS;
        }

    if (partition->NeedsECClassIdFilter())
        filterSqlExpression.append(classIdColSql).append(" IN (SELECT ClassId FROM " TABLE_ClassHierarchyCache " WHERE BaseClassId=").append(classIdStr).append(")");

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

    DbTable const* primaryTable = RequiresJoinTo(propertyMap, false);
    if (primaryTable == nullptr)
        return joinInfo;

    RelationshipClassMap const& relationshipMap = propertyMap.GetClassMap().GetAs<RelationshipClassMap>();
    if (!relationshipMap.IsMappedTo(contextTable))
        {
        BeAssert(false && "Relationship is not mapped to the context table. It must be one of Relationship.GetTables()");
        return joinInfo;
        }

    const bool isSource = propertyMap.GetEnd() == ECN::ECRelationshipEnd::ECRelationshipEnd_Source;
    ConstraintECInstanceIdPropertyMap const*  constraintId = isSource ? relationshipMap.GetSourceECInstanceIdPropMap() : relationshipMap.GetTargetECInstanceIdPropMap();
    BeAssert(constraintId != nullptr);
    SystemPropertyMap::PerTableIdPropertyMap const* releventECClassIdPropertyMap = constraintId->FindDataPropertyMap(contextTable);
    if (releventECClassIdPropertyMap == nullptr)
        {
        BeAssert(false && "Expecting a property map for given context table");
        return joinInfo;
        }

    DbColumn const* primaryECInstanceIdColumn = primaryTable->FindFirst(DbColumn::Kind::ECInstanceId);
    DbColumn const* primaryECClassIdColumn = primaryTable->FindFirst(DbColumn::Kind::ECClassId);
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

    return m_propertyMap->GetEnd() == ECN::ECRelationshipEnd::ECRelationshipEnd_Source ? ECDBSYS_PROP_SourceECClassId : ECDBSYS_PROP_TargetECClassId;
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
        .Append(ExpHelper::ToSql(BooleanSqlOperator::EqualTo))
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
        BeAssert(propertyMap.FindDataPropertyMap(*table) != nullptr);
        if (table->GetType() == DbTable::Type::Virtual || propertyMap.FindDataPropertyMap(*table)->GetColumn().GetPersistenceType() == PersistenceType::Virtual)
            return nullptr;
        }

    if (propertyMap.GetClassMap().GetType() == ClassMap::Type::RelationshipEndTable)
        {
        RelationshipClassEndTableMap const& map = propertyMap.GetClassMap().GetAs<RelationshipClassEndTableMap>();
        if (map.GetConstraintMap(map.GetReferencedEnd()).GetECClassIdPropMap() == &propertyMap)
            {
            SystemPropertyMap::PerTableIdPropertyMap const* c = propertyMap.FindDataPropertyMap(*table);
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
ViewGenerator::ToSqlVisitor::ToSqlVisitor(Context const& context, DbTable const& tableFilter, Utf8CP classIdentifier, ColumnAliasMode colAliasMode)
    : IPropertyMapVisitor(), m_tableFilter(tableFilter), m_classIdentifier(classIdentifier), m_columnAliasMode(colAliasMode),m_context(context), m_doNotAddColumnAliasForComputedExpression(false)
    {
    if (m_classIdentifier != nullptr && Utf8String::IsNullOrEmpty(m_classIdentifier))
        m_classIdentifier = nullptr;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::ToSqlVisitor::_Visit(SingleColumnDataPropertyMap const& propertyMap) const
    {
    if (m_columnAliasMode != ColumnAliasMode::NoAlias)
        {
        BeAssert(false);
        return ERROR;
        }

    return ToNativeSql(propertyMap);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::ToSqlVisitor::_Visit(SystemPropertyMap const& propertyMap) const
    {
    switch (propertyMap.GetType())
        {
            case PropertyMap::Type::ConstraintECInstanceId:
                return ToNativeSql(propertyMap.GetAs<ConstraintECInstanceIdPropertyMap>());
            case PropertyMap::Type::ConstraintECClassId:
                return ToNativeSql(propertyMap.GetAs<ConstraintECClassIdPropertyMap>());
            case PropertyMap::Type::ECClassId:
                return ToNativeSql(propertyMap.GetAs<ECClassIdPropertyMap>());
            case PropertyMap::Type::ECInstanceId:
                return ToNativeSql(propertyMap.GetAs<ECInstanceIdPropertyMap>());
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
        return ToNativeSql(propertyMap.GetAs<NavigationPropertyMap::RelECClassIdPropertyMap>());

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
    NavigationPropertyMap::IdPropertyMap const& idPropMap = relClassIdPropMap.GetParent()->GetAs<NavigationPropertyMap>().GetIdPropertyMap();

    NativeSqlBuilder idColStrBuilder;
    idColStrBuilder.Append(m_classIdentifier, idPropMap.GetColumn().GetName().c_str());

    NativeSqlBuilder relClassIdColStrBuilder;
    if (relClassIdPropMap.GetColumn().GetPersistenceType() == PersistenceType::Virtual)
        relClassIdColStrBuilder = NativeSqlBuilder(relClassIdPropMap.GetDefaultClassId().ToString().c_str());
    else
        relClassIdColStrBuilder.Append(m_classIdentifier, relClassIdPropMap.GetColumn().GetName().c_str());
    //The RelECClassId should always be logically null if the respective NavId col is null
    //case exp must have the relclassid col name as alias
    if (m_context.GetViewType() == ViewType::ECClassView || m_context.GetViewType() == ViewType::UpdatableView || m_doNotAddColumnAliasForComputedExpression)
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
    SystemPropertyMap::PerTableIdPropertyMap const* vmap = propertyMap.FindDataPropertyMap(m_tableFilter);
    if (vmap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    Utf8StringCR colName = vmap->GetColumn().GetName();
    Result& result = Record(*vmap);
    result.GetSqlBuilderR().Append(m_classIdentifier, colName.c_str());

    switch (m_columnAliasMode)
        {
            case ColumnAliasMode::NoAlias:
                return SUCCESS;

            case ColumnAliasMode::SystemPropertyName:
            {
            if (!colName.EqualsIAscii(propertyMap.GetAccessString()))
                result.GetSqlBuilderR().AppendSpace().Append(propertyMap.GetAccessString().c_str());

            return SUCCESS;
            }

            default:
                BeAssert(false);
                return ERROR;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::ToSqlVisitor::ToNativeSql(ECClassIdPropertyMap const& propertyMap) const
    {
    SystemPropertyMap::PerTableIdPropertyMap const* perTableClassIdPropMap = propertyMap.FindDataPropertyMap(m_tableFilter);
    if (perTableClassIdPropMap == nullptr || perTableClassIdPropMap->GetType() != PropertyMap::Type::SystemPerTableClassId)
        {
        BeAssert(false);
        return ERROR;
        }

    Result& result = Record(*perTableClassIdPropMap);

    DbColumn const& col = perTableClassIdPropMap->GetColumn();
    if (col.GetPersistenceType() == PersistenceType::Virtual)
        {
        BeAssert(perTableClassIdPropMap->GetAs<SystemPropertyMap::PerTableClassIdPropertyMap>().GetDefaultECClassId().IsValid());
        result.GetSqlBuilderR().Append(perTableClassIdPropMap->GetAs<SystemPropertyMap::PerTableClassIdPropertyMap>().GetDefaultECClassId());
        result.SetIsLiteralSqlSnippet();
        }
    else
        result.GetSqlBuilderR().Append(m_classIdentifier, col.GetName().c_str());

    switch (m_columnAliasMode)
        {
            case ColumnAliasMode::NoAlias:
                return SUCCESS;

            case ColumnAliasMode::SystemPropertyName:
            {
            if (!col.GetName().EqualsIAscii(propertyMap.GetAccessString()) || col.GetPersistenceType() == PersistenceType::Virtual)
                result.GetSqlBuilderR().AppendSpace().Append(propertyMap.GetAccessString().c_str());

            return SUCCESS;
            }

            default:
                BeAssert(false);
                return ERROR;
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::ToSqlVisitor::ToNativeSql(ConstraintECClassIdPropertyMap const& propertyMap) const
    {
    SystemPropertyMap::PerTableIdPropertyMap const* perTablePropMap = propertyMap.FindDataPropertyMap(m_tableFilter);
    if (perTablePropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    Result& result = Record(*perTablePropMap);

    DbColumn const& col = perTablePropMap->GetColumn();
    if (col.GetPersistenceType() == PersistenceType::Virtual)
        {
        BeAssert(perTablePropMap->GetAs<SystemPropertyMap::PerTableClassIdPropertyMap>().GetDefaultECClassId().IsValid());
        result.GetSqlBuilderR().Append(perTablePropMap->GetAs<SystemPropertyMap::PerTableClassIdPropertyMap>().GetDefaultECClassId());
        result.SetIsLiteralSqlSnippet();
        }
    else
        result.GetSqlBuilderR().Append(m_classIdentifier, col.GetName().c_str());

    switch (m_columnAliasMode)
        {
            case ColumnAliasMode::NoAlias:
                return SUCCESS;

            case ColumnAliasMode::SystemPropertyName:
            {
            if (!col.GetName().EqualsIAscii(propertyMap.GetAccessString()) || col.GetPersistenceType() == PersistenceType::Virtual)
                result.GetSqlBuilderR().AppendSpace().Append(propertyMap.GetAccessString().c_str());

            return SUCCESS;
            }

            default:
                BeAssert(false);
                return ERROR;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::ToSqlVisitor::ToNativeSql(ECInstanceIdPropertyMap const& propertyMap) const
    {
    SystemPropertyMap::PerTableIdPropertyMap const* vmap = propertyMap.FindDataPropertyMap(m_tableFilter);
    if (vmap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    Result& result = Record(*vmap);
    Utf8StringCR colName = vmap->GetColumn().GetName();
    result.GetSqlBuilderR().Append(m_classIdentifier, colName.c_str());

    switch (m_columnAliasMode)
        {
            case ColumnAliasMode::NoAlias:
                return SUCCESS;

            case ColumnAliasMode::SystemPropertyName:
            {
            if (!colName.EqualsIAscii(propertyMap.GetAccessString()))
                result.GetSqlBuilderR().AppendSpace().Append(propertyMap.GetAccessString().c_str());

            return SUCCESS;
            }

            default:
                BeAssert(false);
                return ERROR;
        }

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
