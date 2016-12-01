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
    SearchPropertyMapVisitor typeVisitor(PropertyMap::Type::Data); //Only include none-system properties
    baseClassMap.GetPropertyMaps().AcceptVisitor(typeVisitor);
    for (PropertyMap const* result: typeVisitor.Results())
        {
        DataPropertyMap const* derivedPropertyMap = static_cast<DataPropertyMap const*> (derivedClassMap.GetPropertyMaps().Find(result->GetAccessString().c_str()));
        if (derivedPropertyMap == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }

        std::vector<DbColumn const*> derivedColumnList, baseColumnList;
        GetColumnsPropertyMapVisitor baseColumnVisitor, derivedColumnVisitor;
        result->AcceptVisitor(baseColumnVisitor);
        derivedPropertyMap->AcceptVisitor(derivedColumnVisitor);
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

    if (classMap.IsRelationshipClassMap())
        {
        if (classMap.GetType() == ClassMap::Type::RelationshipEndTable)
            return RenderRelationshipClassEndTableMap(viewSql, static_cast<RelationshipClassEndTableMap const&>(classMap));

        return RenderRelationshipClassLinkTableMap(viewSql, static_cast<RelationshipClassLinkTableMap const&>(classMap));
        }

    return RenderEntityClassMap(viewSql, classMap);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                          11/2016
//---------------------------------------------------------------------------------------
bool ViewGenerator::IsECClassIdFilterEnabled() const
    {
    if (m_prepareContext)
        {
        if (OptionsExp const* options = m_prepareContext->GetCurrentScope().GetOptions())
            return !options->HasOption(OptionsExp::NOECCLASSIDFILTER_OPTION);
        }
    return true;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                          11/2016
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::RenderEntityClassMap(NativeSqlBuilder& viewSql, ClassMap const& classMap)
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
        if (m_isPolymorphic)
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
        ECClass const* entityClassMap = m_ecdb.Schemas().GetECClass(partition->GetRootClassId());
        if (entityClassMap == nullptr || entityClassMap->GetClassType() != ECClassType::Entity)
            {
            BeAssert(false);
            return ERROR;
            }

        ClassMap const* contextClassMap = m_ecdb.Schemas().GetDbMap().GetClassMap(*entityClassMap);
        if (contextClassMap == nullptr || contextClassMap->GetType() != ClassMap::Type::Class)
            {
            BeAssert(false);
            return ERROR;
            }

        ClassMap const* castInto = contextClassMap == &classMap ? nullptr : &classMap;
        if (RenderEntityClassMap(view, *contextClassMap, partition->GetTable(), castInto) != SUCCESS)
            return ERROR;


        if (SystemPropertyMap::PerTablePrimitivePropertyMap const* classIdPropertyMap = contextClassMap->GetECClassIdPropertyMap()->FindDataPropertyMap(partition->GetTable()))
            {
            if (classIdPropertyMap->GetColumn().GetPersistenceType() == PersistenceType::Persisted && IsECClassIdFilterEnabled())
                {
                Utf8String whereClause;
                if (SUCCESS != storageDesc.GenerateECClassIdFilter(whereClause, partition->GetTable(), classIdPropertyMap->GetColumn(), m_isPolymorphic, true))
                    return ERROR;

                if (!whereClause.empty())
                    view.Append(" WHERE ").Append(whereClause.c_str());
                }
            }

        unionList.push_back(view);
        }

    if (unionList.empty())
        {
        if (RenderNullView(viewSql, classMap) != SUCCESS)
            return ERROR;
        }
    else
        {
        if (m_asSubQuery)
            viewSql.AppendParenLeft();

        viewSql.Append(NativeSqlBuilder::Union(unionList));

        if (m_asSubQuery)
            viewSql.AppendParenRight();
        }

    if (m_captureViewAccessStringList)
        m_captureViewAccessStringList = false;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                          11/2016
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::RenderEntityClassMap(NativeSqlBuilder& viewSql, ClassMap const& classMap, DbTable const& contextTable, ClassMapCP castAs)
    {
    viewSql.Append("SELECT ");
    DbTable const* requireJoinTo = nullptr;
    if (RenderPropertyMaps(viewSql, requireJoinTo, classMap, contextTable, castAs, PropertyMap::Type::Entity) != SUCCESS)
        return ERROR;

    viewSql.Append(" FROM ").AppendEscaped(contextTable.GetName().c_str());
    //Join necessary table for table 
    if (requireJoinTo != nullptr)
        {
        auto primaryKey = contextTable.GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
        auto fkKey = requireJoinTo->GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
        viewSql.Append(" INNER JOIN ").AppendEscaped(requireJoinTo->GetName().c_str());
        viewSql.Append(" ON ").AppendEscaped(contextTable.GetName().c_str()).AppendDot().AppendEscaped(primaryKey->GetName().c_str());
        viewSql.Append(" = ").AppendEscaped(requireJoinTo->GetName().c_str()).AppendDot().AppendEscaped(fkKey->GetName().c_str());
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                          11/2016
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::RenderNullView(NativeSqlBuilder& viewSql, ClassMap const& classMap)
    {
    SearchPropertyMapVisitor visitor(PropertyMap::Type::All, true);
    classMap.GetPropertyMaps().AcceptVisitor(visitor);
    if (m_asSubQuery)
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
            {
            viewSql.Append("NULL").AppendSpace().AppendEscaped(propertyMap->GetAccessString().c_str());
            }
        else
            {
            PrimitivePropertyMap const* primitiveMap = static_cast<PrimitivePropertyMap const*>(propertyMap);
            viewSql.Append("NULL").AppendSpace().AppendEscaped(primitiveMap->GetColumn().GetName().c_str());
            }

        RecordPropertyMapIfRequried(*propertyMap);
        }

    viewSql.Append(" LIMIT 0");

    if (m_asSubQuery)
        viewSql.AppendParenRight();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                          11/2016
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::RenderRelationshipClassLinkTableMap(NativeSqlBuilder& viewSql, RelationshipClassLinkTableMap const& relationMap)
    {
    NativeSqlBuilder::List unionList;
    StorageDescription const& storageDesc = relationMap.GetStorageDescription();
    for (Partition const& partition : storageDesc.GetHorizontalPartitions())
        {
        if (partition.GetTable().GetPersistenceType() == PersistenceType::Virtual)
            continue;

        NativeSqlBuilder view;
        ECClass const* relationshipECClass = m_ecdb.Schemas().GetECClass(partition.GetRootClassId());
        if (relationshipECClass == nullptr || !relationshipECClass->IsRelationshipClass())
            {
            BeAssert(false);
            return ERROR;
            }

        ClassMap const* classMap = m_ecdb.Schemas().GetDbMap().GetClassMap(*relationshipECClass);
        if (classMap == nullptr || classMap->GetType() != ClassMap::Type::RelationshipLinkTable)
            {
            BeAssert(false);
            return ERROR;
            }

        RelationshipClassLinkTableMap const& contextRelationship = static_cast<RelationshipClassLinkTableMap const&>(*classMap);
        RelationshipClassLinkTableMap const* castInto = &contextRelationship == &relationMap ? nullptr : &relationMap;
        ConstraintECClassIdJoinInfo sourceECClassIdJoinInfo = ConstraintECClassIdJoinInfo::Create(*relationMap.GetSourceECClassIdPropMap(), partition.GetTable());
        ConstraintECClassIdJoinInfo targetECClassIdJoinInfo = ConstraintECClassIdJoinInfo::Create(*relationMap.GetTargetECClassIdPropMap(), partition.GetTable());
        if (RenderRelationshipClassMap(view, contextRelationship, partition.GetTable(), sourceECClassIdJoinInfo, targetECClassIdJoinInfo, castInto) != SUCCESS)
            return ERROR;

        if (sourceECClassIdJoinInfo.RequiresJoin())
            view.Append(sourceECClassIdJoinInfo.GetNativeJoinSql());

        if (targetECClassIdJoinInfo.RequiresJoin())
            view.Append(targetECClassIdJoinInfo.GetNativeJoinSql());

        ECClassIdPropertyMap const* classIdPropMap = relationMap.GetECClassIdPropertyMap();
        if (SystemPropertyMap::PerTablePrimitivePropertyMap const* classIdDataPropertyMap = classIdPropMap->FindDataPropertyMap(partition.GetTable()))
            {
            if (classIdDataPropertyMap->GetColumn().GetPersistenceType() == PersistenceType::Persisted && IsECClassIdFilterEnabled())
                {
                Utf8String whereClause;
                if (SUCCESS != storageDesc.GenerateECClassIdFilter(whereClause, partition.GetTable(), classIdDataPropertyMap->GetColumn(), m_isPolymorphic, true))
                    return ERROR;

                if (!whereClause.empty())
                    view.Append(" WHERE ").Append(whereClause.c_str());
                }
            }

        unionList.push_back(view);
        }

    if (unionList.empty())
        {
        if (RenderNullView(viewSql, relationMap) != SUCCESS)
            return ERROR;
        }
    else
        {
        if (m_asSubQuery)
            viewSql.AppendParenLeft();

        viewSql.Append(NativeSqlBuilder::Union(unionList));

        if (m_asSubQuery)
            viewSql.AppendParenRight();
        }

    if (m_captureViewAccessStringList)
        m_captureViewAccessStringList = false;

    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                          11/2016
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::RenderRelationshipClassEndTableMap(NativeSqlBuilder& viewSql, RelationshipClassEndTableMap const& relationMap) 
    {
    NativeSqlBuilder::List unionList;
    for (DbTable const* table : relationMap.GetTables())
        {
        if (table->GetPersistenceType() == PersistenceType::Virtual)
            continue;

        NativeSqlBuilder view;
        ConstraintECClassIdJoinInfo sourceECClassIdJoinInfo = ConstraintECClassIdJoinInfo::Create(*relationMap.GetSourceECClassIdPropMap(), *table);
        ConstraintECClassIdJoinInfo targetECClassIdJoinInfo = ConstraintECClassIdJoinInfo::Create(*relationMap.GetTargetECClassIdPropMap(), *table);

        if (RenderRelationshipClassMap(view, relationMap, *table, sourceECClassIdJoinInfo, targetECClassIdJoinInfo) != SUCCESS)
            return ERROR;

        if (sourceECClassIdJoinInfo.RequiresJoin())
            view.Append(sourceECClassIdJoinInfo.GetNativeJoinSql());

        if (targetECClassIdJoinInfo.RequiresJoin())
            view.Append(targetECClassIdJoinInfo.GetNativeJoinSql());

        view.Append(" WHERE ").AppendEscaped(relationMap.GetReferencedEndECInstanceIdPropMap()->GetAccessString().c_str()).Append(" IS NOT NULL");
        //! Add Polymorphic Filter if required
        if (SystemPropertyMap::PerTablePrimitivePropertyMap const* classIdPropertyMap = relationMap.GetECClassIdPropertyMap()->FindDataPropertyMap(*table))
            {
            if (classIdPropertyMap->GetColumn().GetPersistenceType() == PersistenceType::Persisted && IsECClassIdFilterEnabled())
                {    
                NativeSqlBuilder classIdFilter;
                Utf8Char classIdStr[ECClassId::ID_STRINGBUFFER_LENGTH];
                relationMap.GetClass().GetId().ToString(classIdStr);
                classIdFilter.AppendEscaped(table->GetName().c_str()).AppendDot().AppendEscaped(classIdPropertyMap->GetColumn().GetName().c_str());
                if (m_isPolymorphic)
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
        if (RenderNullView(viewSql, relationMap) != SUCCESS)
            return ERROR;
        }
    else
        {
        if (m_asSubQuery)
            viewSql.AppendParenLeft();

        viewSql.Append(NativeSqlBuilder::Union(unionList));

        if (m_asSubQuery)
            viewSql.AppendParenRight();
        }

    if (m_captureViewAccessStringList)
        m_captureViewAccessStringList = false;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                          11/2016
//---------------------------------------------------------------------------------------
void ViewGenerator::RecordPropertyMapIfRequried(PropertyMap const& propertyMap) 
    {
    if (m_viewAccessStringList && m_captureViewAccessStringList)
        m_viewAccessStringList->push_back(propertyMap.GetAccessString());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                          11/2016
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::RenderRelationshipClassMap(NativeSqlBuilder& viewSql, RelationshipClassMap const& relationMap, DbTable const& contextTable, ConstraintECClassIdJoinInfo const& sourceJoinInfo, ConstraintECClassIdJoinInfo const& targetJoinInfo, RelationshipClassLinkTableMap const* castInto)
    {
    const bool requiresJoin = sourceJoinInfo.RequiresJoin() || targetJoinInfo.RequiresJoin();
    SqlVisitor sqlVisitor(contextTable, contextTable.GetName().c_str(), true);
    viewSql.Append("SELECT ");
    //ECInstanceId
    sqlVisitor.Reset();
    RecordPropertyMapIfRequried(*relationMap.GetECInstanceIdPropertyMap());
    relationMap.GetECInstanceIdPropertyMap()->AcceptVisitor(sqlVisitor);
    viewSql.Append(sqlVisitor.GetResultSet().front().GetSqlBuilder());

    //ECClassId
    sqlVisitor.Reset();
    RecordPropertyMapIfRequried(*relationMap.GetECClassIdPropertyMap());
    relationMap.GetECClassIdPropertyMap()->AcceptVisitor(sqlVisitor);
    viewSql.AppendComma().Append(sqlVisitor.GetResultSet().front().GetSqlBuilder());

    //SourceECInstanceId
    sqlVisitor.Reset();
    RecordPropertyMapIfRequried(*relationMap.GetSourceECInstanceIdPropMap());
    relationMap.GetSourceECInstanceIdPropMap()->AcceptVisitor(sqlVisitor);
    viewSql.AppendComma().Append(sqlVisitor.GetResultSet().front().GetSqlBuilder());

    //SourceECClassId
    RecordPropertyMapIfRequried(*relationMap.GetSourceECClassIdPropMap());
    if (sourceJoinInfo.RequiresJoin())
        viewSql.AppendComma().Append(sourceJoinInfo.GetNativeConstraintECClassIdSql(true));
    else
        {
        if (DbTable const* table = ConstraintECClassIdJoinInfo::RequiresJoinTo(*relationMap.GetSourceECClassIdPropMap(), true /*ignoreVirtualColumnCheck*/))
            {
            SqlVisitor constraintSqlVisitor(*table, contextTable.GetName().c_str(), true);
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
    sqlVisitor.Reset();
    RecordPropertyMapIfRequried(*relationMap.GetTargetECInstanceIdPropMap());
    relationMap.GetTargetECInstanceIdPropMap()->AcceptVisitor(sqlVisitor);
    viewSql.AppendComma().Append(sqlVisitor.GetResultSet().front().GetSqlBuilder());

    //TargetECClassId
    RecordPropertyMapIfRequried(*relationMap.GetTargetECClassIdPropMap());
    if (targetJoinInfo.RequiresJoin())
        viewSql.AppendComma().Append(targetJoinInfo.GetNativeConstraintECClassIdSql(true));
    else
        {
        if (DbTable const* table = ConstraintECClassIdJoinInfo::RequiresJoinTo(*relationMap.GetTargetECClassIdPropMap(), true /*ignoreVirtualColumnCheck*/))
            {
            SqlVisitor constraintSqlVisitor(*table, contextTable.GetName().c_str(), true);
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
    if (RenderPropertyMaps(dataPropertySql, requireJoinTo, relationMap, contextTable, nullptr, PropertyMap::Type::Data, requiresJoin) != SUCCESS)
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
BentleyStatus ViewGenerator::RenderPropertyMaps(NativeSqlBuilder& sqlView, DbTable const*& requireJoinTo, ClassMapCR classMap, DbTable const&  contextTable, ClassMapCP baseClass, PropertyMap::Type filter, bool requireJoin)
    {
    requireJoinTo = nullptr;
    const bool generateECClassView = m_viewAccessStringList && m_captureViewAccessStringList;
    if (Enum::Contains(filter, PropertyMap::Type::ConstraintECClassId) || Enum::Contains(filter, PropertyMap::Type::ConstraintECInstanceId))
        {
        BeAssert(false && "This funtion cannot render ConstraintECClassId and ConstraintECInstanceId propertymaps");
        return ERROR;
        }

    std::vector<std::pair<PropertyMap const*, PropertyMap const*>> propertyMaps;
    if (!classMap.IsMappedTo(contextTable))
        {
        BeAssert(false);
        return ERROR;
        }

    DbTable const* requireJoinToTableForDataProperties = nullptr;
    if (baseClass)
        {
        SearchPropertyMapVisitor propertyVisitor(PropertyMap::Type::All, true /*traverse compound properties but compound properties themself is not included*/);
        baseClass->GetPropertyMaps().AcceptVisitor(propertyVisitor);
        for (PropertyMap const* basePropertyMap : propertyVisitor.Results())
            {
            if (!Enum::Contains(filter, basePropertyMap->GetType()))
                continue;

            if (m_prepareContext && !m_prepareContext->GetSelectionOptions().IsSelected(basePropertyMap->GetAccessString().c_str()))
                continue;

            PropertyMap const* propertyMap = classMap.GetPropertyMaps().Find(basePropertyMap->GetAccessString().c_str());
            if (propertyMap == nullptr)
                {
                BeAssert(false && "classMap is not subclass of baseClass");
                return ERROR;
                }
                /* ECObjects allows this for one case: where base prop is prim array and sub prop is prim
                of same prim type. ECObjects might be disallowing this with EC3.1
                if (propertyMap->GetType() != basePropertyMap->GetType())
                {
                BeAssert(propertyMap->GetType() == basePropertyMap->GetType());
                return ERROR;
                }
                */

            //!We assum that in case of joinedTable we can only have exactly one table to join to.
            //!Therefore not using a set/vector to store joinTable list
            if (requireJoinToTableForDataProperties == nullptr)
                {
                if (propertyMap->IsData())
                    {
                    DataPropertyMap const* dataPropertyMap = static_cast<DataPropertyMap const*> (propertyMap);
                    if (&dataPropertyMap->GetTable() != &contextTable)
                        requireJoinToTableForDataProperties = &dataPropertyMap->GetTable();
                    }
                }

            propertyMaps.push_back(std::make_pair(propertyMap, basePropertyMap));
            }
        }
    else
        {
        SearchPropertyMapVisitor propertyVisitor(PropertyMap::Type::All, true /*traverse compound properties but compound properties themself is not included*/);
        classMap.GetPropertyMaps().AcceptVisitor(propertyVisitor);
        for (PropertyMap const* propertyMap : propertyVisitor.Results())
            {
            if (Enum::Contains(filter, propertyMap->GetType()))
                {
                if (m_prepareContext && !m_prepareContext->GetSelectionOptions().IsSelected(propertyMap->GetAccessString().c_str()))
                    continue;

                //!We assum that in case of joinedTable we can only have exactly one table to joint to.
                //!Therefore not using a set/vector to store joinTable list
                if (requireJoinToTableForDataProperties == nullptr)
                    {
                    if (propertyMap->IsData())
                        {
                        DataPropertyMap const* dataPropertyMap = static_cast<DataPropertyMap const*> (propertyMap);
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
            {
            BeAssert(dynamic_cast<CompoundDataPropertyMap const*>(basePropertyMap) == nullptr);
            }

        BeAssert(dynamic_cast<CompoundDataPropertyMap const*>(propertyMap) == nullptr);
        RecordPropertyMapIfRequried(*propertyMap);

        NativeSqlBuilder propertySql;
        // We only need table qualifier if there is at least one data property selected that require joining to another table
        // In this case all data properties are table qualified name to ensure no conflict between two tables columns.
        // System property never require a join but therefor requireJoinToTableForDataProperties = nullptr if no data property was choosen
        if (propertyMap->IsSystem())
            {
            SqlVisitor toSqlVisitor(contextTable, systemContextTableAlias, true);
            if (SUCCESS != propertyMap->AcceptVisitor(toSqlVisitor) || toSqlVisitor.GetResultSet().empty())
                {
                BeAssert(false);
                return ERROR;
                }

            SqlVisitor::Result const& r = toSqlVisitor.GetResultSet().front();
            propertySql = r.GetSqlBuilder();
            }
        else //Data Property
            {
            BeAssert(propertyMap->IsData());
            BeAssert(dynamic_cast<SingleColumnDataPropertyMap const*>(propertyMap) != nullptr);


            SingleColumnDataPropertyMap const* dataProperty = static_cast<SingleColumnDataPropertyMap const*>(propertyMap);
            //! Join table does not require casting as we only split table into exactly two possible tables and only if shared table is enabled.
            if (&dataProperty->GetTable() == requireJoinToTableForDataProperties)
                {
                SqlVisitor toSqlVisitor(*requireJoinToTableForDataProperties, requireJoinToTableForDataProperties->GetName().c_str(), false);
                if (m_viewAccessStringList && m_captureViewAccessStringList)
                    toSqlVisitor.EnableDebugView();
                    
                if (SUCCESS != dataProperty->AcceptVisitor(toSqlVisitor) || toSqlVisitor.GetResultSet().empty())
                    {
                    BeAssert(false);
                    return ERROR;
                    }

                SqlVisitor::Result const& r = toSqlVisitor.GetResultSet().front();
                //! This is where we generate strong type column for shared column for debug view
                if (generateECClassView && r.GetColumn().IsShared())
                    {
                    const DbColumn::Type colType = DbColumn::PrimitiveTypeToColumnType(r.GetPropertyMap().GetProperty().GetAsPrimitiveProperty()->GetType());
                    propertySql.Append("CAST (").Append(r.GetSql()).Append(" AS ").Append(DbColumn::TypeToSql(colType)).Append(")");
                    }
                else
                    {
                    propertySql = r.GetSqlBuilder();
                    if (auto of = dataProperty->GetColumn().GetPhysicalOverflowColumn())
                        {
                        propertySql.AppendSpace().AppendEscaped(dataProperty->GetColumn().GetName().c_str());
                        }
                    }
                }
            else
                {
                SqlVisitor toSqlVisitor(contextTable, systemContextTableAlias, false);
                if (m_viewAccessStringList && m_captureViewAccessStringList)
                    toSqlVisitor.EnableDebugView();
               
                if (SUCCESS != dataProperty->AcceptVisitor(toSqlVisitor) || toSqlVisitor.GetResultSet().empty())
                    {
                    BeAssert(false);
                    return ERROR;
                    }

                SqlVisitor::Result const& r = toSqlVisitor.GetResultSet().front();
                if (generateECClassView && r.GetColumn().IsShared())
                    {
                    if (r.GetPropertyMap().GetProperty().GetIsStructArray())
                        {
                        if (r.GetColumn().GetType() != DbColumn::Type::Text)
                            propertySql.Append("CAST (").Append(r.GetSql()).Append(" AS TEXT)");
                        else
                            propertySql.Append(r.GetSql());
                        }
                    else if (r.GetPropertyMap().GetProperty().GetIsPrimitiveArray())
                        {
                        if (r.GetColumn().GetType() != DbColumn::Type::Blob)
                            propertySql.Append("CAST (").Append(r.GetSql()).Append(" AS BLOB)");
                        else
                            propertySql.Append(r.GetSql());
                        }
                    else
                        {
                        const DbColumn::Type colType = DbColumn::PrimitiveTypeToColumnType(r.GetPropertyMap().GetProperty().GetAsPrimitiveProperty()->GetType());
                        propertySql.Append("CAST (").Append(r.GetSql()).Append(" AS ").Append(DbColumn::TypeToSql(colType)).Append(")");
                        }
                    }
                else
                    {
                    propertySql = r.GetSqlBuilder();
                    //! Here we want rename or add column alias so it appear to be a basePropertyMap
                    //! But we only do that if column name differ
                    bool appendAlias = false;
                    if (basePropertyMap != nullptr)
                        {
                        SingleColumnDataPropertyMap const* baseDataProperty = static_cast<SingleColumnDataPropertyMap const*>(basePropertyMap);
                        if (!r.GetColumn().GetName().EqualsI(baseDataProperty->GetColumn().GetName()))
                            {
                            propertySql.AppendSpace().AppendEscaped(baseDataProperty->GetColumn().GetName().c_str());
                            appendAlias = true;
                            }
                        }
                    if (!appendAlias)
                        {
                        if (auto of = dataProperty->GetColumn().GetPhysicalOverflowColumn())
                            {
                            propertySql.AppendSpace().AppendEscaped(dataProperty->GetColumn().GetName().c_str());
                            }
                        }
                    }
                }
            }

        propertySqlList.push_back(propertySql);
        }

    requireJoinTo = requireJoinToTableForDataProperties;
    sqlView.Append(NativeSqlBuilder::GenerateSelectList(propertySqlList));
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

//*********************************ViewGenerator::SqlVisitor*****************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
ViewGenerator::SqlVisitor::SqlVisitor(DbTable const& tableFilter, Utf8CP classIdentifier, bool usePropertyNameAsAliasForSystemPropertyMaps /*= false*/)
    : IPropertyMapVisitor(), m_tableFilter(tableFilter), m_classIdentifier(classIdentifier), m_usePropertyNameAsAliasForSystemPropertyMaps(usePropertyNameAsAliasForSystemPropertyMaps), m_forDebugView(false)
    {
    if (m_classIdentifier != nullptr && Utf8String::IsNullOrEmpty(m_classIdentifier))
        m_classIdentifier = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::SqlVisitor::_Visit(SystemPropertyMap const& propertyMap) const
    {
    switch (propertyMap.GetType())
        {
            case PropertyMap::Type::ConstraintECInstanceId:
                return ToNativeSql(static_cast<ConstraintECInstanceIdPropertyMap const&>(propertyMap));
            case PropertyMap::Type::ConstraintECClassId:
                return ToNativeSql(static_cast<ConstraintECClassIdPropertyMap const&>(propertyMap));
            case PropertyMap::Type::ECClassId:
                return ToNativeSql(static_cast<ECClassIdPropertyMap const&>(propertyMap));
            case PropertyMap::Type::ECInstanceId:
                return ToNativeSql(static_cast<ECInstanceIdPropertyMap const&>(propertyMap));
            default:
                BeAssert(false);
                return ERROR;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::SqlVisitor::ToNativeSql(SingleColumnDataPropertyMap const& propertyMap) const
    {
    if (propertyMap.GetType() == PropertyMap::Type::NavigationRelECClassId)
        return ToNativeSql(static_cast<NavigationPropertyMap::RelECClassIdPropertyMap const&>(propertyMap));

    Result& result = Record(propertyMap);
    if (propertyMap.GetOverflowState() == DataPropertyMap::OverflowState::Yes)
        {
        DbColumn const* overFlowColumn = propertyMap.GetColumn().GetPhysicalOverflowColumn();
        BeAssert(overFlowColumn != nullptr);
        bool addBlobToBase64Func = false;
        if (propertyMap.GetProperty().GetIsPrimitiveArray() || (propertyMap.GetProperty().GetIsPrimitive() && propertyMap.GetProperty().GetAsPrimitiveProperty()->GetType() == PRIMITIVETYPE_Binary))
            {
            addBlobToBase64Func = true;
            }

        if (addBlobToBase64Func && !m_forDebugView)
            result.GetSqlBuilderR().Append("Base64ToBlob(json_extract(")
            .Append(m_classIdentifier, overFlowColumn->GetName().c_str())
            .AppendComma().Append("'$.")
            .Append(propertyMap.GetColumn().GetName().c_str()).Append("'))");
        else
            result.GetSqlBuilderR().Append("json_extract(")
            .Append(m_classIdentifier, overFlowColumn->GetName().c_str())
            .AppendComma().Append("'$.")
            .Append(propertyMap.GetColumn().GetName().c_str()).Append("')");
        }
    else
        {
        result.GetSqlBuilderR().Append(m_classIdentifier, propertyMap.GetColumn().GetName().c_str());
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::SqlVisitor::ToNativeSql(NavigationPropertyMap::RelECClassIdPropertyMap const& propertyMap) const
    {
    Result& result = Record(propertyMap);
    if (!propertyMap.IsVirtual())
        {
        result.GetSqlBuilderR().Append(m_classIdentifier, propertyMap.GetColumn().GetName().c_str());
        return SUCCESS;
        }

    result.GetSqlBuilderR().Append(propertyMap.GetDefaultClassId()).AppendSpace().Append(propertyMap.GetColumn().GetName().c_str());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ViewGenerator::SqlVisitor::ToNativeSql(ConstraintECInstanceIdPropertyMap const& propertyMap) const
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
BentleyStatus ViewGenerator::SqlVisitor::ToNativeSql(ECClassIdPropertyMap const& propertyMap) const
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
BentleyStatus ViewGenerator::SqlVisitor::ToNativeSql(ConstraintECClassIdPropertyMap const& propertyMap) const
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
BentleyStatus ViewGenerator::SqlVisitor::ToNativeSql(ECInstanceIdPropertyMap const& propertyMap) const
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
ViewGenerator::SqlVisitor::Result& ViewGenerator::SqlVisitor::Record(SingleColumnDataPropertyMap const& propertyMap) const
    {
    m_resultSetByAccessString[propertyMap.GetAccessString().c_str()] = m_resultSet.size();
    m_resultSet.push_back(Result(propertyMap));
    return m_resultSet.back();
    }

//****************************************************************************************
END_BENTLEY_SQLITE_EC_NAMESPACE
