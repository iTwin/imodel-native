/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ViewGenerator.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <set>
#include "SqlNames.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
#define TABLEALIAS_InstanceChange "ic"
#define TABLE_InstanceChange ECSCHEMA_ALIAS_ECDbChange "_" ECDBCHANGE_CLASS_InstanceChange
#define TABLE_InstanceChange_COL_SummaryId "SummaryId"
#define TABLE_InstanceChange_COL_OpCode "OpCode"
#define TABLE_InstanceChange_COL_ChangedInstanceId "ChangedInstance_Id"
#define TABLE_InstanceChange_COL_ChangedInstanceClassId "ChangedInstance_ClassId"
//************************** ViewGenerator ***************************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2016
//+---------------+---------------+---------------+---------------+---------------+--------
//static 
BentleyStatus ViewGenerator::GenerateSelectFromViewSql(NativeSqlBuilder& viewSql, ECSqlPrepareContext const& prepareContext, ClassMap const& classMap, bool isPolymorphicQuery, MemberFunctionCallExp const* memberFunctionCallExp)
    {
    // Turn a polymorphic query into a non-polymorphic query if the class is sealed or has no subclasses.
    // This will speed up the query
    if (isPolymorphicQuery)
        {
        if (classMap.GetClass().GetClassModifier() == ECClassModifier::Sealed)
            isPolymorphicQuery = false;
        else
            {
            ECDerivedClassesList const* subClasses = prepareContext.GetECDb().Schemas().GetDerivedClassesInternal(classMap.GetClass(), classMap.GetSchemaManager().GetTableSpace().GetName().c_str());
            if (subClasses == nullptr)
                return ERROR; // loading subclasses failed

            if (subClasses->empty())
                isPolymorphicQuery = false;
            }
        }

    SelectFromViewContext ctx(prepareContext, classMap.GetSchemaManager(), isPolymorphicQuery, memberFunctionCallExp);
    if (memberFunctionCallExp == nullptr) 
        return GenerateViewSql(viewSql, ctx, classMap);

    if (!memberFunctionCallExp->GetFunctionName().EqualsIAscii(ECSQLFUNC_Changes))
        {
        ctx.GetECDb().GetImpl().Issues().ReportV("Class exp has member function call %s which is not supported yet.", memberFunctionCallExp->GetFunctionName().c_str());
        return ERROR;
        }

    return GenerateChangeSummaryViewSql(viewSql, ctx, classMap);
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
        ecdb.GetImpl().Issues().ReportV("Can only call ECDb::CreateClassViewsInDb() on an ECDb file with read-write access.");
        return ERROR;
        }

    if (DropECClassViews(ecdb) != SUCCESS)
        return ERROR;

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, 
                                     "SELECT c.Id FROM main." TABLE_Class " c, main." TABLE_ClassMap " cm WHERE c.Id = cm.ClassId AND "
                                     "c.Type IN (" SQLVAL_ECClassType_Entity "," SQLVAL_ECClassType_Relationship ") AND "
                                     "cm.MapStrategy NOT IN (" SQLVAL_MapStrategy_NotMapped "," SQLVAL_MapStrategy_ForeignKeyRelationshipInSourceTable "," SQLVAL_MapStrategy_ForeignKeyRelationshipInTargetTable ")"))
        return ERROR;

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ECClassId classId = stmt.GetValueId<ECClassId>(0);
        BeAssert(classId.IsValid());

        ECClassCP ecClass = ecdb.Schemas().GetClass(classId);
        if (ecClass == nullptr)
            return ERROR;

        ClassMap const* classMap = ecdb.Schemas().Main().GetClassMap(*ecClass);
        if (classMap == nullptr)
            return ERROR;

        if (CreateECClassView(ecdb, *classMap) != SUCCESS)
            return ERROR;
        }

    PERFLOG_FINISH("ECDb", "Create ECClass views");
    return SUCCESS;
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
            return ERROR;

        ClassMap const* classMap = ecdb.Schemas().Main().GetClassMap(*ecClass);
        if (classMap == nullptr)
            return ERROR;

        const MapStrategy mapStrategy = classMap->GetMapStrategy().GetStrategy();

        if (mapStrategy == MapStrategy::NotMapped || (!classMap->GetClass().IsEntityClass() && !classMap->GetClass().IsRelationshipClass())
            || mapStrategy == MapStrategy::ForeignKeyRelationshipInSourceTable || mapStrategy == MapStrategy::ForeignKeyRelationshipInTargetTable)
            {
            ecdb.GetImpl().Issues().ReportV("Cannot create ECClassView for ECClass '%s' (Id: %s) because it is not mapped or not an ECEntityClass or a link table ECRelationshipClass.",
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
    createViewSql.Sprintf("CREATE VIEW main.%s (%s)\n\t--### ECCLASS VIEW is for debugging purpose only!.\n\tAS %s;", viewName.c_str(), viewColumnNameList.c_str(), viewSql.GetSql().c_str());
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
                                     "SELECT ('DROP VIEW IF EXISTS main.[' || s.Alias || '.' || c.Name || '];') FROM main.ec_Class c "
                                     "INNER JOIN main.ec_Schema s ON s.Id=c.SchemaId"))
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

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2017
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ViewGenerator::GenerateChangeSummaryViewSql(NativeSqlBuilder& viewSql, SelectFromViewContext& ctx, ClassMap const& classMap)
    {
    BeAssert(ctx.GetMemberFunctionCallExp() != nullptr);
    MemberFunctionCallExp const& memberFuncCallExp = *ctx.GetMemberFunctionCallExp();

    if (memberFuncCallExp.GetChildrenCount() != 2)
        {
        ctx.GetECDb().GetImpl().Issues().ReportV("Class exp's member function '%s' is called with invalid number of arguments.", memberFuncCallExp.GetFunctionName().c_str());
        return ERROR;
        }

    if (!ctx.GetECDb().IsChangeCacheAttached())
        {
        ctx.GetECDb().GetImpl().Issues().ReportV("Failed to prepare ECSQL. When using the function " ECSQLFUNC_Changes " the Change Cache file must have been attached before.");
        return ERROR;
        }

    NativeSqlBuilder::List argSqlSnippets;
    if (ECSqlExpPreparer::PrepareFunctionArgList(argSqlSnippets, const_cast<ECSqlPrepareContext&> (ctx.GetPrepareCtx()), memberFuncCallExp) != ECSqlStatus::Success)
        return ERROR;


    // ----------------------------------------------
    // Optimization when ChangeValueState is known
    // - AfterDelete - Call DeleteValue() to construct row entirely from changeset
    // - AfterInsert - Call InsertValue() to construct row entirely from changeset
    // - BeforeUpdate/AfterUpdate - Require LEFT JOIN
    // - By finding which accessString changed we can remove the call to ChangeValue() but this require a additional query
    // -------------------------------------------------

    BeAssert(argSqlSnippets.size() == 2);
    NativeSqlBuilder const& summaryIdArgSql = argSqlSnippets[0];
    NativeSqlBuilder const& changedValueStateArgSql = argSqlSnippets[1];

    NativeSqlBuilder internalView;
    if (GenerateViewSql(internalView, ctx, classMap) != SUCCESS)
        return ERROR;

    Utf8StringCR viewName = classMap.GetClass().GetName();
    internalView.AppendSpace().AppendEscaped(viewName);
    NativeSqlBuilder columnSql(TABLEALIAS_InstanceChange "." TABLE_InstanceChange_COL_ChangedInstanceId " " ECDBSYS_PROP_ECInstanceId ", "  TABLEALIAS_InstanceChange "." TABLE_InstanceChange_COL_ChangedInstanceClassId " " ECDBSYS_PROP_ECClassId);
    SearchPropertyMapVisitor propertyVisitor(PropertyMap::Type::ConstraintECClassId | PropertyMap::Type::ConstraintECInstanceId | PropertyMap::Type::SingleColumnData);
    if (SUCCESS != classMap.GetPropertyMaps().AcceptVisitor(propertyVisitor))
        return ERROR;

    // see if changesetId and changeValueState is constant literals or not. If yes then also try to determine the values
    /* Not used yet
    ECInstanceId summaryId;
    bool optimizeForLiteralSummaryId = false;
    ValueExp const* changeSummaryIdExp = memberFuncCallExp.GetArgument(0);
    if (changeSummaryIdExp->GetType() == Exp::Type::LiteralValue && !changeSummaryIdExp->GetTypeInfo().IsNull())
        {
        ECValue summaryIdVal;
        if (SUCCESS != changeSummaryIdExp->GetAs<LiteralValueExp>().TryParse(summaryIdVal) || !summaryIdVal.IsLong())
            {
            ctx.GetECDb().GetImpl().Issues().ReportV("Failed to prepare ECSQL. Invalid ChangeSummary Id expression '%s' in function " ECSQLFUNC_Changes ".", changeSummaryIdExp->ToECSql().c_str());
            return ERROR;
            }

        summaryId = ECInstanceId((uint64_t) summaryIdVal.GetLong());
        optimizeForLiteralSummaryId = true;
        }
    */

    ValueExp const* changeValueStateExp = memberFuncCallExp.GetArgument(1);
    bool needsJoinToDataTable = true;
    Nullable<ChangedValueState> changedValueState;
    if (changeValueStateExp->GetType() == Exp::Type::LiteralValue)
        {
        ECValue stateVal;
        if (SUCCESS != changeValueStateExp->GetAs<LiteralValueExp>().TryParse(stateVal) || !(stateVal.IsLong() || stateVal.IsInteger() || stateVal.IsString()) )
            {
            ctx.GetECDb().GetImpl().Issues().ReportV("Failed to prepare ECSQL. Invalid ChangedValueStatue expression '%s' in function " ECSQLFUNC_Changes ".", changeValueStateExp->ToECSql().c_str());
            return ERROR;
            }

        if (stateVal.IsInteger())
            changedValueState = ChangeManager::ToChangedValueState(stateVal.GetInteger());
        else if (stateVal.IsLong())
            changedValueState = ChangeManager::ToChangedValueState((int) stateVal.GetLong());
        else
            changedValueState = ChangeManager::ToChangedValueState(stateVal.GetUtf8CP());

        if (changedValueState == nullptr)
            {
            ctx.GetECDb().GetImpl().Issues().ReportV("Failed to prepare ECSQL. Invalid ChangedValueStatue expression '%s' in function " ECSQLFUNC_Changes ".", changeValueStateExp->ToECSql().c_str());
            return ERROR;
            }

        //For before and after we still need a join to the data table
        needsJoinToDataTable = changedValueState == ChangedValueState::BeforeUpdate || changedValueState == ChangedValueState::AfterUpdate;
        }

    for (PropertyMap const* propertyMap : propertyVisitor.Results())
        {
        if (ctx.GetViewType() == ViewType::SelectFromView && !ctx.GetAs<SelectFromViewContext>().IsInSelectClause(propertyMap->GetAccessString()))
            continue;

        Utf8StringCP colName = nullptr;
        if (propertyMap->GetType() == PropertyMap::Type::ConstraintECClassId || propertyMap->GetType() == PropertyMap::Type::ConstraintECInstanceId)
            colName = &propertyMap->GetAccessString();
        else
            {
            const SingleColumnDataPropertyMap& dataProperty = propertyMap->GetAs<SingleColumnDataPropertyMap>();
            Utf8StringCR columnName = dataProperty.GetColumn().GetName();
            if (dataProperty.GetType() == PropertyMap::Type::NavigationRelECClassId && dataProperty.GetColumn().IsVirtual())
                {
                columnSql.AppendComma().Append(dataProperty.GetAs<NavigationPropertyMap::RelECClassIdPropertyMap>().GetDefaultClassId()).AppendSpace().Append(columnName);
                continue;
                }

            colName = &columnName;
            }

        BeAssert(colName != nullptr);

        // ChangedValue(<InstanceChange Id Column>, <access string>, <ChangedValueState>, <Fallback Value>)
        columnSql.Append("," SQLFUNC_ChangedValue "(" TABLEALIAS_InstanceChange "." COL_DEFAULTNAME_Id ",'").Append(propertyMap->GetAccessString()).Append("',");

        if (changedValueState != nullptr) // literal value for changed value state
            columnSql.AppendFormatted("%d,", Enum::ToInt(changedValueState.Value()));
        else
            columnSql.Append(changedValueStateArgSql).AppendComma(); // any other type of expression for changed value state (e.g. a parameter)

          // For insert and delete, we don't have to pass a fallback value as we know the value must come from the change summary table
        if (changedValueState == ChangedValueState::AfterInsert || changedValueState == ChangedValueState::BeforeDelete)
            columnSql.Append("NULL)");
        else
            columnSql.AppendEscaped(viewName).AppendDot().AppendEscaped(*colName).AppendParenRight();

        columnSql.AppendSpace().AppendEscaped(*colName);
        }

    viewSql.AppendParenLeft();

    Utf8String classIdStr = classMap.GetClass().GetId().ToString();
    viewSql.Append("SELECT ").Append(columnSql.GetSql()).Append(" FROM " TABLESPACE_ECChange "." TABLE_InstanceChange " " TABLEALIAS_InstanceChange);
    if (ctx.IsPolymorphicQuery())
        viewSql.AppendFormatted(" INNER JOIN [%s]." TABLE_ClassHierarchyCache " ch ON " TABLEALIAS_InstanceChange "." TABLE_InstanceChange_COL_ChangedInstanceClassId "=ch.ClassId AND ch.BaseClassId=%s",
                                ctx.GetSchemaManager().GetTableSpace().GetName().c_str(), classIdStr.c_str());

    if (needsJoinToDataTable)
        {
        viewSql.Append(" LEFT JOIN ").Append(internalView.GetSql()).Append(" ON ").AppendEscaped(viewName);
        viewSql.Append("." ECDBSYS_PROP_ECInstanceId "=" TABLEALIAS_InstanceChange "." TABLE_InstanceChange_COL_ChangedInstanceId);
        }

    viewSql.Append(" WHERE " TABLEALIAS_InstanceChange "." TABLE_InstanceChange_COL_OpCode "=");
    
    if (changedValueState == nullptr)
        viewSql.Append(SQLFUNC_ChangedValueStateToOpCode "(").Append(changedValueStateArgSql).Append(")");
    else
        {
        Nullable<ChangeOpCode> opCode = ChangeManager::DetermineOpCodeFromChangedValueState(changedValueState.Value());
        BeAssert(opCode != nullptr);
        viewSql.AppendFormatted("%d", Enum::ToInt(opCode.Value()));
        }

    viewSql.Append(" AND " TABLEALIAS_InstanceChange "." TABLE_InstanceChange_COL_SummaryId "=").Append(summaryIdArgSql);

    if (!ctx.IsPolymorphicQuery())
        viewSql.AppendFormatted(" AND " TABLEALIAS_InstanceChange "." TABLE_InstanceChange_COL_ChangedInstanceClassId "=%s", classIdStr.c_str());

    viewSql.AppendParenRight();
    return SUCCESS;
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
            Utf8CP accessString = nullptr;
            for (PropertyMap const* propertyMap : mixInClassMap.GetPropertyMaps())
                {
                if (propertyMap->IsSystem())
                    continue;

                accessString = propertyMap->GetAccessString().c_str();
                break;
                }

            if (accessString)
                {
                PropertyMap const* propertyMap = derivedClassMap.GetPropertyMaps().Find(accessString);
                BeAssert(propertyMap != nullptr);
                if (propertyMap == nullptr)
                    return ERROR;

                BeAssert(propertyMap->IsData());
                if (!propertyMap->IsData())
                    return ERROR;

                contextTable = &propertyMap->GetAs<DataPropertyMap>().GetTable();
                }
            else
                contextTable = &derivedClassMap.GetPrimaryTable();
            }

        if (RenderEntityClassMap(viewSql, ctx, derivedClassMap, *contextTable, &mixInClassMap) != SUCCESS)
            return ERROR;

        auto itor = selectClauses.find(viewSql.GetSql());
        if (itor == selectClauses.end())
            itor = selectClauses.insert(make_bpair(viewSql.GetSql(), make_bpair(contextTable, bvector<ECN::ECClassId>()))).first;

        itor->second.second.push_back(derivedClassMap.GetClass().GetId());

        if (!selectClauses.empty())
            if (ctx.GetViewType() == ViewType::ECClassView)
                ctx.GetAs<ECClassViewContext>().StopCaptureViewColumnNames();
        }

    Nullable<std::vector<ClassMap const*>> derivedClassMaps = derivedClassMap.GetDerivedClassMaps();
    if (derivedClassMaps == nullptr)
        return ERROR;

    for (ClassMap const* derivedClassMap : derivedClassMaps.Value())
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
        return RenderNullView(viewSql, ctx, mixInClassMap);

    for (auto const& kvp : selectClauses)
        {
        bvector<ECClassId> const& classIds = kvp.second.second;
        DbTable const* table = kvp.second.first;
        selectClause.Append(kvp.first);
        DbColumn const* classId = table->FindFirst(DbColumn::Kind::ECClassId);
        if (classId->GetPersistenceType() == PersistenceType::Physical)
            {
            selectClause.Append(" WHERE ").AppendFullyQualified(table->GetName(), classId->GetName());
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

        viewSql.Append(selectClause.GetSql()).AppendSpace();
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
        ECClass const* tableRootClass = ctx.GetSchemaManager().GetClass(partition->GetRootClassId());
        if (tableRootClass == nullptr || tableRootClass->GetClassType() != ECClassType::Entity)
            {
            BeAssert(false);
            return ERROR;
            }

        ClassMap const* tableRootClassMap = ctx.GetSchemaManager().GetClassMap(*tableRootClass);
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

                Utf8String filterSQL;
                if (SUCCESS != GenerateECClassIdFilter(filterSQL, classMap, partition->GetTable(), classIdPropertyMap->GetColumn(), considerSubclasses))
                    return ERROR;

                if (!filterSQL.empty())
                    {
                    if (!considerSubclasses)
                        view.Append(" WHERE ").Append(filterSQL.c_str());
                    else
                        view.Append(" ").Append(filterSQL.c_str());
                    }
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
BentleyStatus ViewGenerator::RenderEntityClassMap(NativeSqlBuilder& viewSql, Context& ctx, ClassMap const& classMap, DbTable const& contextTable, ClassMap const* castAs)
    {
    viewSql.Append("SELECT ");
    bset<DbTable const*> requireJoinTo;
    if (RenderPropertyMaps(viewSql, ctx, requireJoinTo, classMap, contextTable, castAs, PropertyMap::Type::Data | PropertyMap::Type::ECInstanceId | PropertyMap::Type::ECClassId) != SUCCESS)
        return ERROR;

    viewSql.Append(" FROM ").AppendEscaped(contextTable.GetTableSpace().GetName()).AppendDot().AppendEscaped(contextTable.GetName());

    //Join necessary table for table 
    for(DbTable const* to : requireJoinTo)
        {
        DbColumn const* primaryKey = contextTable.FindFirst(DbColumn::Kind::ECInstanceId);
        DbColumn const* fkKey = to->FindFirst(DbColumn::Kind::ECInstanceId);
        viewSql.Append(" INNER JOIN ").AppendEscaped(to->GetTableSpace().GetName()).AppendDot().AppendEscaped(to->GetName());
        viewSql.Append(" ON ").AppendEscaped(contextTable.GetName()).AppendDot().AppendEscaped(primaryKey->GetName());
        viewSql.Append(ExpHelper::ToSql(BooleanSqlOperator::EqualTo)).AppendEscaped(to->GetName()).AppendDot().AppendEscaped(fkKey->GetName());
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                          11/2016
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::RenderNullView(NativeSqlBuilder& viewSql, Context& ctx, ClassMap const& classMap)
    {
    viewSql.Clear();
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
            viewSql.Append("NULL ").AppendEscaped(propertyMap->GetAccessString());
        else
            viewSql.Append("NULL ").AppendEscaped(propertyMap->GetAs<SingleColumnDataPropertyMap>().GetColumn().GetName());

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
        ECClass const* relationshipECClass = ctx.GetSchemaManager().GetClass(partition.GetRootClassId());
        if (relationshipECClass == nullptr || !relationshipECClass->IsRelationshipClass())
            {
            BeAssert(false);
            return ERROR;
            }

        ClassMap const* classMap = ctx.GetSchemaManager().GetClassMap(*relationshipECClass);
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
                Utf8String filterSQL;
                const bool considerSubclasses = !isSelectFromView || ctx.GetAs<SelectFromViewContext>().IsPolymorphicQuery();
                if (SUCCESS != GenerateECClassIdFilter(filterSQL, relationMap, partition.GetTable(), classIdDataPropertyMap->GetColumn(), considerSubclasses))
                    return ERROR;

                if (!filterSQL.empty())
                    {
                    if (!considerSubclasses)
                        view.Append(" WHERE ").Append(filterSQL.c_str());
                    else
                        view.Append(" ").Append(filterSQL.c_str());
                    }
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
    constexpr Utf8CP referencedEndAlias = "_ReferencedEnd";
    const bool isECClassView = ctx.GetViewType() == ViewType::ECClassView && ctx.GetAs<ECClassViewContext>().MustCaptureViewColumnNames();
    if (isECClassView)
        {
        ECClassViewContext& viewContext = ctx.GetAs<ECClassViewContext>();
        viewContext.AddViewColumnName(relationMap.GetECInstanceIdPropertyMap()->GetAccessString());
        viewContext.AddViewColumnName(relationMap.GetECClassIdPropertyMap()->GetAccessString());
        viewContext.AddViewColumnName(relationMap.GetSourceECInstanceIdPropMap()->GetAccessString());
        viewContext.AddViewColumnName(relationMap.GetSourceECClassIdPropMap()->GetAccessString());
        viewContext.AddViewColumnName(relationMap.GetTargetECInstanceIdPropMap()->GetAccessString());
        viewContext.AddViewColumnName(relationMap.GetTargetECClassIdPropMap()->GetAccessString());
        }

    auto toSql = [&ctx] (NativeSqlBuilder& sqlBuilder, DbColumn const& col, Utf8CP tableQualifier = nullptr)
        {
  
        const bool requiresCast = ctx.GetViewType() == ViewType::ECClassView && col.IsShared();
        if (requiresCast)
            sqlBuilder.Append("CAST(");

        if (tableQualifier == nullptr)
            sqlBuilder.AppendFullyQualified(col.GetTable().GetName(), col.GetName());
        else
            sqlBuilder.AppendFullyQualified(tableQualifier, col.GetName().c_str());

        if (requiresCast)
            sqlBuilder.Append(" AS INTEGER)");
        };

    const ECClassId classId = relationMap.GetClass().GetId();
    std::unique_ptr<ForeignKeyPartitionView> view = ForeignKeyPartitionView::CreateReadonly(ctx.GetSchemaManager(), relationMap.GetRelationshipClass());
    for (ForeignKeyPartitionView::Partition const* partition : view->GetPartitions(true, true))
        {
        const bool isSelf = partition->GetSourceECClassIdColumn()->GetId() == partition->GetTargetECClassIdColumn()->GetId();
        const bool appendAlias = unionList.empty();
        NativeSqlBuilder viewSql("SELECT ");
        //ECInstanceId
        toSql(viewSql, partition->GetECInstanceIdColumn());
        if (appendAlias)
            viewSql.AppendSpace().Append(ECDBSYS_PROP_ECInstanceId);
        
        viewSql.AppendComma();

        //ECClassId
        if (partition->GetECClassIdColumn().IsVirtual())
            viewSql.Append(classId);
        else
            toSql(viewSql, partition->GetECClassIdColumn());

        if (appendAlias)
            viewSql.AppendSpace().Append(ECDBSYS_PROP_ECClassId);

        viewSql.AppendComma();

        //SourceECInstanceId
        toSql(viewSql, partition->GetSourceECInstanceIdColumn());
        if (appendAlias)
            viewSql.AppendSpace().Append(ECDBSYS_PROP_SourceECInstanceId);

        viewSql.AppendComma();

        //SourceECClassID
        if (partition->GetSourceECClassIdColumn()->IsVirtual())
            {
            // If ClassId is virtual then the class must be mapped to its own table other wise it would have a ECClassId column.
            // Following reverse lookup class from table and expect exactly one class map to table that is only way that the ECClassId is virtual
            std::vector<ECClassId> const& classIds = ctx.GetSchemaManager().GetLightweightCache().GetClassesForTable(partition->GetSourceECClassIdColumn()->GetTable());
            BeAssert(classIds.size() == 1);
            if (classIds.size() != 1)
                return ERROR;

            viewSql.Append(classIds.front());
            }
        else
            {
            if (isSelf && relationMap.GetReferencedEnd() == ECRelationshipEnd_Source)
                toSql(viewSql, *partition->GetSourceECClassIdColumn(), referencedEndAlias);
            else
                toSql(viewSql, *partition->GetSourceECClassIdColumn());
            }

        if (appendAlias)
            viewSql.AppendSpace().Append(ECDBSYS_PROP_SourceECClassId);

        viewSql.AppendComma();

        //TargetECInstanceId
        toSql(viewSql, partition->GetTargetECInstanceIdColumn());
        if (appendAlias)
            viewSql.AppendSpace().Append(ECDBSYS_PROP_TargetECInstanceId);

        viewSql.AppendComma();

        //TargetECClassId
        if (partition->GetTargetECClassIdColumn()->IsVirtual())
            {
            // If ClassId is virtual then the class must be mapped to its own table other wise it would have a ECClassId column.
            // Following reverse lookup class from table and expect exactly one class map to table that is only way that the ECClassId is virtual
            std::vector<ECClassId> const& classIds = ctx.GetSchemaManager().GetLightweightCache().GetClassesForTable(partition->GetTargetECClassIdColumn()->GetTable());
            BeAssert(classIds.size() == 1);
            if (classIds.size() != 1)
                return ERROR;

            viewSql.Append(classIds.front());
            }
        else
            {
            if (isSelf && relationMap.GetReferencedEnd() == ECRelationshipEnd_Target)
                toSql(viewSql, *partition->GetTargetECClassIdColumn(), referencedEndAlias);
            else
                toSql(viewSql, *partition->GetTargetECClassIdColumn());
            }

        if (appendAlias)
            viewSql.AppendSpace().Append(ECDBSYS_PROP_TargetECClassId);

        //FROM
        viewSql.Append(" FROM ").AppendEscaped(partition->GetECInstanceIdColumn().GetTable().GetTableSpace().GetName()).AppendDot().AppendEscaped(partition->GetECInstanceIdColumn().GetTable().GetName());
        DbColumn const& refClassIdCol = relationMap.GetReferencedEnd() == ECRelationshipEnd::ECRelationshipEnd_Source ? *partition->GetSourceECClassIdColumn() : *partition->GetTargetECClassIdColumn();
        DbColumn const& referenceIdColumn = relationMap.GetReferencedEnd() == ECRelationshipEnd::ECRelationshipEnd_Source ? partition->GetSourceECInstanceIdColumn() : partition->GetTargetECInstanceIdColumn();
        if (refClassIdCol.GetPersistenceType() == PersistenceType::Physical)
            {
            DbColumn const* idColumn = refClassIdCol.GetTable().FindFirst(DbColumn::Kind::ECInstanceId);
            BeAssert(idColumn != nullptr);
            viewSql.Append(" INNER JOIN ").AppendEscaped(refClassIdCol.GetTable().GetTableSpace().GetName()).AppendDot().AppendEscaped(refClassIdCol.GetTable().GetName());
            if (isSelf)
                {
                viewSql.AppendSpace().Append(referencedEndAlias).Append(" ON ");
                toSql(viewSql, *idColumn, referencedEndAlias);
                }
            else
                {
                viewSql.Append(" ON ");
                toSql(viewSql, *idColumn);
                }

            viewSql.Append("=");
            toSql(viewSql, referenceIdColumn);
            }

        //no need to cast referencidColumn because IS NOT NULL doesn't need to change the data affinity
        viewSql.Append(" WHERE ").AppendFullyQualified(referenceIdColumn.GetTable().GetName(), referenceIdColumn.GetName()).Append(" IS NOT NULL");
        if (partition->GetECClassIdColumn().GetPersistenceType() == PersistenceType::Physical)
            {
            const bool isPolymorphic = ctx.GetViewType() == ViewType::SelectFromView ? ctx.GetAs<SelectFromViewContext>().IsPolymorphicQuery() : true;
            viewSql.Append(" AND ");
            toSql(viewSql, partition->GetECClassIdColumn());
            if (isPolymorphic)
                viewSql.AppendFormatted(" IN (SELECT ClassId FROM [%s]." TABLE_ClassHierarchyCache " WHERE BaseClassId=%s)", ctx.GetSchemaManager().GetTableSpace().GetName().c_str(), relationMap.GetClass().GetId().ToString().c_str());
            else
                viewSql.Append(ExpHelper::ToSql(BooleanSqlOperator::EqualTo)).Append(relationMap.GetClass().GetId());
            }

        unionList.push_back(viewSql);
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

        viewSql.Append(unionList, " UNION ALL ");

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
    viewSql.Append("SELECT ");

    ToSqlVisitor sqlVisitor(ctx, contextTable, contextTable.GetName());

    {
    //ECInstanceId
    BeAssert(relationMap.GetECInstanceIdPropertyMap() != nullptr);
    ECInstanceIdPropertyMap const& propMap = *relationMap.GetECInstanceIdPropertyMap();

    if (ctx.GetViewType() == ViewType::ECClassView && ctx.GetAs<ECClassViewContext>().MustCaptureViewColumnNames())
        ctx.GetAs<ECClassViewContext>().AddViewColumnName(propMap.GetAccessString());

    if (SUCCESS != propMap.AcceptVisitor(sqlVisitor))
        return ERROR;

    ToSqlVisitor::Result const& sqlResult = sqlVisitor.GetResultSet().front();
    viewSql.Append(sqlResult.GetSqlBuilder());
    if (sqlResult.GetSqlExpressionType() != ToSqlVisitor::Result::SqlExpressionType::PropertyName || !sqlResult.GetColumn().GetName().EqualsI(propMap.GetAccessString()))
        viewSql.AppendSpace().AppendEscaped(propMap.GetAccessString());
    }

    {
    //ECClassId
    BeAssert(relationMap.GetECClassIdPropertyMap() != nullptr);
    ECClassIdPropertyMap const& propMap = *relationMap.GetECClassIdPropertyMap();

    if (ctx.GetViewType() == ViewType::ECClassView && ctx.GetAs<ECClassViewContext>().MustCaptureViewColumnNames())
        ctx.GetAs<ECClassViewContext>().AddViewColumnName(propMap.GetAccessString());

    sqlVisitor.Reset();
    if (SUCCESS != propMap.AcceptVisitor(sqlVisitor))
        return ERROR;

    ToSqlVisitor::Result const& sqlResult = sqlVisitor.GetResultSet().front();
    viewSql.AppendComma().Append(sqlResult.GetSqlBuilder());
    if (sqlResult.GetSqlExpressionType() != ToSqlVisitor::Result::SqlExpressionType::PropertyName || !sqlResult.GetColumn().GetName().EqualsI(propMap.GetAccessString()))
        viewSql.AppendSpace().AppendEscaped(propMap.GetAccessString());
    }

    {
    //SourceECInstanceId
    BeAssert(relationMap.GetSourceECInstanceIdPropMap() != nullptr);
    ConstraintECInstanceIdPropertyMap const& propMap = *relationMap.GetSourceECInstanceIdPropMap();

    if (ctx.GetViewType() == ViewType::ECClassView && ctx.GetAs<ECClassViewContext>().MustCaptureViewColumnNames())
        ctx.GetAs<ECClassViewContext>().AddViewColumnName(propMap.GetAccessString());

    sqlVisitor.Reset();
    if (SUCCESS != propMap.AcceptVisitor(sqlVisitor))
        return ERROR;

    ToSqlVisitor::Result const& sqlResult = sqlVisitor.GetResultSet().front();
    viewSql.AppendComma().Append(sqlResult.GetSqlBuilder());
    if (sqlResult.GetSqlExpressionType() != ToSqlVisitor::Result::SqlExpressionType::PropertyName || !sqlResult.GetColumn().GetName().EqualsI(propMap.GetAccessString()))
        viewSql.AppendSpace().AppendEscaped(propMap.GetAccessString());
    }

    {
    //SourceECClassId
    BeAssert(relationMap.GetSourceECClassIdPropMap() != nullptr);
    ConstraintECClassIdPropertyMap const& propMap = *relationMap.GetSourceECClassIdPropMap();

    if (ctx.GetViewType() == ViewType::ECClassView && ctx.GetAs<ECClassViewContext>().MustCaptureViewColumnNames())
        ctx.GetAs<ECClassViewContext>().AddViewColumnName(propMap.GetAccessString());

    if (sourceJoinInfo.RequiresJoin())
        viewSql.AppendComma().Append(sourceJoinInfo.GetNativeConstraintECClassIdSql(true));
    else
        {
        if (DbTable const* table = ConstraintECClassIdJoinInfo::RequiresJoinTo(propMap, true /*ignoreVirtualColumnCheck*/))
            {
            ToSqlVisitor constraintSqlVisitor(ctx, *table, contextTable.GetName());
            if (SUCCESS != propMap.AcceptVisitor(constraintSqlVisitor))
                return ERROR;

            BeAssert(!constraintSqlVisitor.GetResultSet().empty());

            ToSqlVisitor::Result const& sqlResult = constraintSqlVisitor.GetResultSet().front();
            viewSql.AppendComma().Append(sqlResult.GetSqlBuilder());

            if (sqlResult.GetSqlExpressionType() != ToSqlVisitor::Result::SqlExpressionType::PropertyName || !sqlResult.GetColumn().GetName().EqualsI(propMap.GetAccessString()))
                viewSql.AppendSpace().AppendEscaped(propMap.GetAccessString());
            }
        else
            {
            //SourceECClassId = ECClassId, TargetECClassId = ECClassId
            sqlVisitor.Reset();
            if (SUCCESS != propMap.AcceptVisitor(sqlVisitor))
                return ERROR;

            BeAssert(!sqlVisitor.GetResultSet().empty());

            ToSqlVisitor::Result const& sqlResult = sqlVisitor.GetResultSet().front();
            viewSql.AppendComma().Append(sqlResult.GetSqlBuilder());

            if (sqlResult.GetSqlExpressionType() != ToSqlVisitor::Result::SqlExpressionType::PropertyName || !sqlResult.GetColumn().GetName().EqualsI(propMap.GetAccessString()))
                viewSql.AppendSpace().AppendEscaped(propMap.GetAccessString());
            }
        }
    }

    {
    //TargetECInstanceId
    BeAssert(relationMap.GetTargetECInstanceIdPropMap() != nullptr);
    ConstraintECInstanceIdPropertyMap const& propMap = *relationMap.GetTargetECInstanceIdPropMap();

    if (ctx.GetViewType() == ViewType::ECClassView && ctx.GetAs<ECClassViewContext>().MustCaptureViewColumnNames())
        ctx.GetAs<ECClassViewContext>().AddViewColumnName(propMap.GetAccessString());

    sqlVisitor.Reset();
    if (SUCCESS != propMap.AcceptVisitor(sqlVisitor))
        return ERROR;

    ToSqlVisitor::Result const& sqlResult = sqlVisitor.GetResultSet().front();
    viewSql.AppendComma().Append(sqlResult.GetSqlBuilder());
    if (sqlResult.GetSqlExpressionType() != ToSqlVisitor::Result::SqlExpressionType::PropertyName || !sqlResult.GetColumn().GetName().EqualsI(propMap.GetAccessString()))
        viewSql.AppendSpace().AppendEscaped(propMap.GetAccessString());
    }

    {
    //TargetECClassId
    BeAssert(relationMap.GetTargetECClassIdPropMap() != nullptr);
    ConstraintECClassIdPropertyMap const& propMap = *relationMap.GetTargetECClassIdPropMap();

    if (ctx.GetViewType() == ViewType::ECClassView && ctx.GetAs<ECClassViewContext>().MustCaptureViewColumnNames())
        ctx.GetAs<ECClassViewContext>().AddViewColumnName(propMap.GetAccessString());

    if (targetJoinInfo.RequiresJoin())
        viewSql.AppendComma().Append(targetJoinInfo.GetNativeConstraintECClassIdSql(true));
    else
        {
        if (DbTable const* table = ConstraintECClassIdJoinInfo::RequiresJoinTo(propMap, true /*ignoreVirtualColumnCheck*/))
            {
            ToSqlVisitor constraintSqlVisitor(ctx, *table, contextTable.GetName());
            if (SUCCESS != propMap.AcceptVisitor(constraintSqlVisitor))
                return ERROR;

            BeAssert(!constraintSqlVisitor.GetResultSet().empty());

            ToSqlVisitor::Result const& sqlResult = constraintSqlVisitor.GetResultSet().front();
            viewSql.AppendComma().Append(sqlResult.GetSqlBuilder());

            if (sqlResult.GetSqlExpressionType() != ToSqlVisitor::Result::SqlExpressionType::PropertyName || !sqlResult.GetColumn().GetName().EqualsI(propMap.GetAccessString()))
                viewSql.AppendSpace().AppendEscaped(propMap.GetAccessString());
            }
        else
            {
            sqlVisitor.Reset();
            if (SUCCESS != propMap.AcceptVisitor(sqlVisitor))
                return ERROR;

            BeAssert(!sqlVisitor.GetResultSet().empty());

            ToSqlVisitor::Result const& sqlResult = sqlVisitor.GetResultSet().front();
            viewSql.AppendComma().Append(sqlResult.GetSqlBuilder());

            if (sqlResult.GetSqlExpressionType() != ToSqlVisitor::Result::SqlExpressionType::PropertyName || !sqlResult.GetColumn().GetName().EqualsI(propMap.GetAccessString()))
                viewSql.AppendSpace().AppendEscaped(propMap.GetAccessString());
            }
        }
    }

    NativeSqlBuilder dataPropertySql;
    bset<DbTable const*> requireJoinTo;
    if (RenderPropertyMaps(dataPropertySql, ctx, requireJoinTo, relationMap, contextTable, nullptr, PropertyMap::Type::Data, sourceJoinInfo.RequiresJoin() || targetJoinInfo.RequiresJoin()) != SUCCESS)
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

    viewSql.Append(" FROM ").AppendEscaped(contextTable.GetTableSpace().GetName()).AppendDot().AppendEscaped(contextTable.GetName());
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                          11/2016
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::RenderPropertyMaps(NativeSqlBuilder& sqlView, Context& ctx, bset<DbTable const*>& requireJoinTo, ClassMapCR classMap, DbTable const&  contextTable, ClassMap const* baseClass, PropertyMap::Type filter, bool requireJoin)
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
                        requireJoinTo.insert(&dataPropertyMap.GetTable());
                    }

                propertyMaps.push_back(std::make_pair(propertyMap, nullptr));
                }
            }
        }

    if (propertyMaps.empty())
        return SUCCESS;

    bool addTableAlias = true;
    if (requireJoinTo.empty())
        {
        if (ctx.GetViewType() == ViewType::SelectFromView)
            addTableAlias = ctx.GetAs<SelectFromViewContext>().IsPolymorphicQuery() && ctx.GetAs<SelectFromViewContext>().IsECClassIdFilterEnabled();
        }
        

    Utf8String systemContextTableAlias = addTableAlias ? contextTable.GetName() : Utf8String();
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
            ToSqlVisitor toSqlVisitor(ctx, contextTable, systemContextTableAlias);
            if (SUCCESS != propertyMap->AcceptVisitor(toSqlVisitor) || toSqlVisitor.GetResultSet().empty())
                {
                BeAssert(false);
                return ERROR;
                }

            ToSqlVisitor::Result const& visitorResult = toSqlVisitor.GetResultSet().front();
            propertySqlList.push_back(visitorResult.GetSqlBuilder());
            //if sql is an expression or if col name differs from prop map access string, add prop map access string as col alias
            if (visitorResult.GetSqlExpressionType() != ToSqlVisitor::Result::SqlExpressionType::PropertyName ||
                !visitorResult.GetColumn().GetName().EqualsIAscii(propertyMap->GetAccessString()))
                propertySqlList.back().AppendSpace().Append(propertyMap->GetAccessString()); //system prop map access strings don't need to be escaped

            continue;
            }

        BeAssert(propertyMap->IsData());
        SingleColumnDataPropertyMap const& dataProperty = propertyMap->GetAs<SingleColumnDataPropertyMap>();

        if (requireJoinTo.end() != requireJoinTo.find(&dataProperty.GetTable()) || requireJoin)
            {
            ToSqlVisitor toSqlVisitor(ctx, dataProperty.GetTable(), dataProperty.GetTable().GetName());
            if (SUCCESS != dataProperty.AcceptVisitor(toSqlVisitor) || toSqlVisitor.GetResultSet().empty())
                {
                BeAssert(false);
                return ERROR;
                }

            ToSqlVisitor::Result const& r = toSqlVisitor.GetResultSet().front();
            propertySqlList.push_back(r.GetSqlBuilder());
            NativeSqlBuilder& sqlBuilder = propertySqlList.back();

            //determine col alias
            if (rootPropertyMap != nullptr)
                {
                if (!Enum::Contains(PropertyMap::Type::SingleColumnData, rootPropertyMap->GetType()))
                    return ERROR;

                DbColumn const& rootPropertyMapCol = rootPropertyMap->GetAs<SingleColumnDataPropertyMap>().GetColumn();

                if (r.GetSqlExpressionType() != ToSqlVisitor::Result::SqlExpressionType::PropertyName || !r.GetColumn().GetName().EqualsIAscii(rootPropertyMapCol.GetName()))
                    sqlBuilder.AppendSpace().AppendEscaped(rootPropertyMapCol.GetName());
                }
            else
                {
                if (r.GetSqlExpressionType() != ToSqlVisitor::Result::SqlExpressionType::PropertyName)
                    sqlBuilder.AppendSpace().AppendEscaped(r.GetColumn().GetName());
                }

            continue;
            }

        //no join needed
        ToSqlVisitor toSqlVisitor(ctx, contextTable, systemContextTableAlias);
        if (SUCCESS != dataProperty.AcceptVisitor(toSqlVisitor) || toSqlVisitor.GetResultSet().empty())
            {
            BeAssert(false);
            return ERROR;
            }

        ToSqlVisitor::Result const& r = toSqlVisitor.GetResultSet().front();
        propertySqlList.push_back(r.GetSqlBuilder());
        NativeSqlBuilder& sqlBuilder = propertySqlList.back();

        if (rootPropertyMap != nullptr)
            {
            //Here we want rename or add column alias so it appear to be a basePropertyMap
            //But we only do that if column name differ
            DbColumn const& rootPropertyMapCol = rootPropertyMap->GetAs<SingleColumnDataPropertyMap>().GetColumn();
            if (r.GetSqlExpressionType() != ToSqlVisitor::Result::SqlExpressionType::PropertyName || !r.GetColumn().GetName().EqualsIAscii(rootPropertyMapCol.GetName()))
                sqlBuilder.AppendSpace().AppendEscaped(rootPropertyMapCol.GetName());
            }
        else
            {
            if (r.GetSqlExpressionType() != ToSqlVisitor::Result::SqlExpressionType::PropertyName)
                sqlBuilder.AppendSpace().AppendEscaped(r.GetColumn().GetName());
            }
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
        {
        Utf8String cacheTableAlias = "[CHC_" + table.GetName() + "]";
        filterSqlExpression.append("INNER JOIN [").append(classMap.GetSchemaManager().GetTableSpace().GetName()).append("]." TABLE_ClassHierarchyCache " ").append(cacheTableAlias)
            .append(" ON ").append(cacheTableAlias).append(".[ClassId]").append("=").append(classIdColSql)
            .append(" AND ").append(cacheTableAlias).append(".[BaseClassId]").append("=").append(classIdStr);
        }
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
    sql.AppendEscaped(m_primaryECInstanceIdCol->GetTable().GetTableSpace().GetName()).AppendDot().AppendEscaped(m_primaryECInstanceIdCol->GetTable().GetName()).AppendSpace();
    sql.AppendEscaped(GetSqlTableAlias()).Append(" ON ").AppendEscaped(GetSqlTableAlias()).AppendDot().AppendEscaped(m_primaryECInstanceIdCol->GetName());
    sql.Append(ExpHelper::ToSql(BooleanSqlOperator::EqualTo)).AppendEscaped(m_foreignECInstanceIdCol->GetTable().GetName()).AppendDot().AppendEscaped(m_foreignECInstanceIdCol->GetName());
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
            if (c->GetColumn().GetKind() == DbColumn::Kind::ECClassId)
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
ViewGenerator::SelectFromViewContext::SelectFromViewContext(ECSqlPrepareContext const& prepareCtx, TableSpaceSchemaManager const& manager, bool isPolymorphicQuery, MemberFunctionCallExp const* functionCallExp)
    : Context(ViewType::SelectFromView, prepareCtx.GetECDb(), manager), m_prepareCtx(prepareCtx), m_isPolymorphicQuery(isPolymorphicQuery), m_memberFunctionCallExp(functionCallExp)
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
bool ViewGenerator::SelectFromViewContext::IsInSelectClause(Utf8StringCR exp) const { return m_prepareCtx.GetSelectionOptions().IsSelected(exp); }

//*********************************ViewGenerator::ToSqlVisitor*****************************
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

    const bool requiresCast = RequiresCast(propertyMap);

    Result& result = AddResult(propertyMap, requiresCast ? Result::SqlExpressionType::Computed : Result::SqlExpressionType::PropertyName);
    NativeSqlBuilder& sqlBuilder = result.GetSqlBuilderR();

    if (requiresCast)
        sqlBuilder.Append("CAST(");

    sqlBuilder.AppendFullyQualified(m_classIdentifier, propertyMap.GetColumn().GetName());

    if (requiresCast)
        sqlBuilder.Append(" AS ").Append(DbColumn::TypeToSql(propertyMap.GetColumnDataType())).AppendParenRight();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::ToSqlVisitor::ToNativeSql(NavigationPropertyMap::RelECClassIdPropertyMap const& relClassIdPropMap) const
    {
    Result& result = AddResult(relClassIdPropMap, Result::SqlExpressionType::Computed);

    BeAssert(relClassIdPropMap.GetParent() != nullptr && relClassIdPropMap.GetParent()->GetType() == PropertyMap::Type::Navigation);
    NavigationPropertyMap::IdPropertyMap const& idPropMap = relClassIdPropMap.GetParent()->GetAs<NavigationPropertyMap>().GetIdPropertyMap();

    NativeSqlBuilder idColStrBuilder;
    idColStrBuilder.AppendFullyQualified(m_classIdentifier, idPropMap.GetColumn().GetName());

    NativeSqlBuilder relClassIdColStrBuilder;
    if (relClassIdPropMap.GetColumn().GetPersistenceType() == PersistenceType::Virtual)
        relClassIdColStrBuilder.Append(relClassIdPropMap.GetDefaultClassId());
    else
        relClassIdColStrBuilder.AppendFullyQualified(m_classIdentifier, relClassIdPropMap.GetColumn().GetName());

    const bool requiresCast = RequiresCast(relClassIdPropMap);

    //wrap cast around case expression rather than the rel class id sql. Cast expressions have the affinity of the target type
    //whereas case expressions don't have an affinity and therefore behave like BLOB columns and would therefore negate the whole
    //idea of injecting the casts again which is what we want to avoid
    //No affinity behaves differently in terms of type conversions prior to comparisons
    //(see https://sqlite.org/datatype3.html#type_conversions_prior_to_comparison)
    if (requiresCast)
        result.GetSqlBuilderR().Append("CAST(");

    //The RelECClassId should always be logically null if the respective NavId col is null
    result.GetSqlBuilderR().AppendFormatted("(CASE WHEN %s IS NULL THEN NULL ELSE %s END)", idColStrBuilder.GetSql().c_str(), relClassIdColStrBuilder.GetSql().c_str());

    if (requiresCast)
        result.GetSqlBuilderR().Append(" AS ").Append(DbColumn::TypeToSql(relClassIdPropMap.GetColumnDataType())).AppendParenRight();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::ToSqlVisitor::ToNativeSql(ConstraintECInstanceIdPropertyMap const& propertyMap) const
    {
    SystemPropertyMap::PerTableIdPropertyMap const* perTablePropMap = propertyMap.FindDataPropertyMap(m_tableFilter);
    if (perTablePropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    const bool requiresCast = RequiresCast(*perTablePropMap);

    Result& result = AddResult(*perTablePropMap, requiresCast ? Result::SqlExpressionType::Computed : Result::SqlExpressionType::PropertyName);

    if (requiresCast)
        result.GetSqlBuilderR().Append("CAST(");

    result.GetSqlBuilderR().AppendFullyQualified(m_classIdentifier, perTablePropMap->GetColumn().GetName());

    if (requiresCast)
        result.GetSqlBuilderR().Append(" AS ").Append(DbColumn::TypeToSql(perTablePropMap->GetColumnDataType())).AppendParenRight();

    return SUCCESS;
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

    const bool requiresCast = RequiresCast(*perTableClassIdPropMap);
    DbColumn const& col = perTableClassIdPropMap->GetColumn();

    Result::SqlExpressionType sqlExpType = Result::SqlExpressionType::PropertyName;
    if (col.GetPersistenceType() == PersistenceType::Virtual)
        sqlExpType = Result::SqlExpressionType::Literal;
    else if (requiresCast)
        sqlExpType = Result::SqlExpressionType::Computed;

    Result& result = AddResult(*perTableClassIdPropMap, sqlExpType);


    if (col.GetPersistenceType() == PersistenceType::Virtual)
        {
        BeAssert(perTableClassIdPropMap->GetAs<SystemPropertyMap::PerTableClassIdPropertyMap>().GetDefaultECClassId().IsValid());
        result.GetSqlBuilderR().Append(perTableClassIdPropMap->GetAs<SystemPropertyMap::PerTableClassIdPropertyMap>().GetDefaultECClassId());
        }
    else
        {
        if (requiresCast)
            result.GetSqlBuilderR().Append("CAST(");

        result.GetSqlBuilderR().AppendFullyQualified(m_classIdentifier, col.GetName());

        if (requiresCast)
            result.GetSqlBuilderR().Append(" AS ").Append(DbColumn::TypeToSql(perTableClassIdPropMap->GetColumnDataType())).AppendParenRight();
        }

    return SUCCESS;
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

    const bool requiresCast = RequiresCast(*perTablePropMap);
    DbColumn const& col = perTablePropMap->GetColumn();

    Result::SqlExpressionType sqlExpType = Result::SqlExpressionType::PropertyName;
    if (col.GetPersistenceType() == PersistenceType::Virtual)
        sqlExpType = Result::SqlExpressionType::Literal;
    else if (requiresCast)
        sqlExpType = Result::SqlExpressionType::Computed;

    Result& result = AddResult(*perTablePropMap, sqlExpType);

    if (col.GetPersistenceType() == PersistenceType::Virtual)
        {
        BeAssert(perTablePropMap->GetAs<SystemPropertyMap::PerTableClassIdPropertyMap>().GetDefaultECClassId().IsValid());
        result.GetSqlBuilderR().Append(perTablePropMap->GetAs<SystemPropertyMap::PerTableClassIdPropertyMap>().GetDefaultECClassId());
        }
    else
        {
        if (requiresCast)
            result.GetSqlBuilderR().Append("CAST(");

        result.GetSqlBuilderR().AppendFullyQualified(m_classIdentifier, col.GetName());

        if (requiresCast)
            result.GetSqlBuilderR().Append(" AS ").Append(DbColumn::TypeToSql(perTablePropMap->GetColumnDataType())).AppendParenRight();
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::ToSqlVisitor::ToNativeSql(ECInstanceIdPropertyMap const& propertyMap) const
    {
    SystemPropertyMap::PerTableIdPropertyMap const* idPropMap = propertyMap.FindDataPropertyMap(m_tableFilter);
    if (idPropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    const bool requiresCast = RequiresCast(*idPropMap);

    Result& result = AddResult(*idPropMap, requiresCast ? Result::SqlExpressionType::Computed : Result::SqlExpressionType::PropertyName);

    if (requiresCast)
        result.GetSqlBuilderR().Append("CAST(");

    result.GetSqlBuilderR().AppendFullyQualified(m_classIdentifier, idPropMap->GetColumn().GetName());

    if (requiresCast)
        result.GetSqlBuilderR().Append(" AS ").Append(DbColumn::TypeToSql(idPropMap->GetColumnDataType())).AppendParenRight();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
ViewGenerator::ToSqlVisitor::Result& ViewGenerator::ToSqlVisitor::AddResult(SingleColumnDataPropertyMap const& propertyMap, Result::SqlExpressionType sqlExpType) const
    {
    m_resultSet.push_back(Result(propertyMap, sqlExpType));
    return m_resultSet.back();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
