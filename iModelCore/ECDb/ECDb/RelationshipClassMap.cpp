/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <array>
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//************************ RelationshipClassMap **********************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
RelationshipClassMap::RelationshipClassMap(ECDb const& ecdb, TableSpaceSchemaManager const& manager, Type type, ECN::ECClassCR ecRelClass, MapStrategyExtendedInfo const& mapStrategy)
    : ClassMap(ecdb, manager, type, ecRelClass, mapStrategy), m_sourceConstraintMap( ecRelClass.GetRelationshipClassCP()->GetSource()), m_targetConstraintMap( ecRelClass.GetRelationshipClassCP()->GetTarget())
    {
    BeAssert(ecRelClass.IsRelationshipClass());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  07/2014
//---------------------------------------------------------------------------------------
RelationshipConstraintMap const& RelationshipClassMap::GetConstraintMap(ECN::ECRelationshipEnd constraintEnd) const
    {
    return constraintEnd == ECRelationshipEnd_Source ? m_sourceConstraintMap : m_targetConstraintMap;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2016
//---------------------------------------------------------------------------------------
RelationshipConstraintMap& RelationshipClassMap::GetConstraintMapR(ECN::ECRelationshipEnd constraintEnd)
    {
    return constraintEnd == ECRelationshipEnd_Source ? m_sourceConstraintMap : m_targetConstraintMap;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    11/2013
//---------------------------------------------------------------------------------------
ConstraintECInstanceIdPropertyMap const* RelationshipClassMap::GetConstraintECInstanceIdPropMap(ECRelationshipEnd constraintEnd) const
    {
    if (constraintEnd == ECRelationshipEnd_Source)
        return GetSourceECInstanceIdPropMap();

    return GetTargetECInstanceIdPropMap();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    11/2013
//---------------------------------------------------------------------------------------
ConstraintECClassIdPropertyMap const* RelationshipClassMap::GetConstraintECClassIdPropMap(ECRelationshipEnd constraintEnd) const
    {
    if (constraintEnd == ECRelationshipEnd_Source)
        return GetSourceECClassIdPropMap();

    return GetTargetECClassIdPropMap();
    }




//************************ RelationshipClassEndTableMap **********************************
//---------------------------------------------------------------------------------------
// @bsimethod                                               Affan.Khan       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
RelationshipClassEndTableMap const* RelationshipClassEndTableMap::GetBaseClassMap(SchemaImportContext* ctx) const
    {
    if (!GetClass().HasBaseClasses())
        return nullptr;

    BeAssert(GetClass().GetBaseClasses().size() == 1);
    ECRelationshipClassCP relationshipClass = static_cast<ECRelationshipClassCP>(GetClass().GetBaseClasses().front());
    if (ClassMap const* classMap = GetSchemaManager().GetClassMap(*relationshipClass))
        return &classMap->GetAs<RelationshipClassEndTableMap>();

    BeAssert(false);
    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECN::ECRelationshipEnd RelationshipClassEndTableMap::GetReferencedEnd() const
    {
    return GetForeignEnd() == ECRelationshipEnd_Source ? ECRelationshipEnd_Target : ECRelationshipEnd_Source;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECN::ECRelationshipEnd RelationshipClassEndTableMap::GetForeignEnd() const
    {
    return GetMapStrategy().GetStrategy() == MapStrategy::ForeignKeyRelationshipInSourceTable ? ECRelationshipEnd_Source : ECRelationshipEnd_Target;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus RelationshipClassEndTableMap::_Map(ClassMappingContext& ctx)
    {
    RelationshipClassEndTableMap const* baseClassMap = GetBaseClassMap(&ctx.GetImportCtx());
    if (baseClassMap != nullptr)
        return MapSubClass(*baseClassMap);

    //End table relationship is always mapped to virtual table.
    //we use nav properties to generate views dynamically
    if (SUCCESS != DbMappingManager::Tables::CreateVirtualTableForFkRelationship(ctx.GetImportCtx(), *this, ctx.GetClassMappingInfo()))
        return ClassMappingStatus::Error;

    if (SUCCESS != MapSystemColumns())
        return ClassMappingStatus::Error;
    
    DbTable* vtable = GetTables().front();
    {////////SourceECInstanceId
    DbColumn const* sourceIdCol = vtable->AddColumn(ECDBSYS_PROP_SourceECInstanceId, DbColumn::Type::Integer, DbColumn::Kind::Default, PersistenceType::Virtual);
    RefCountedPtr<ConstraintECInstanceIdPropertyMap> propMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Source, {sourceIdCol});
    if (propMap == nullptr || sourceIdCol == nullptr)
        {
        BeAssert(false && "Failed to create SourceECInstanceId PropertyMap");
        return ClassMappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(propMap, 2) != SUCCESS)
        return ClassMappingStatus::Error;

    GetConstraintMapR(ECRelationshipEnd_Source).SetECInstanceIdPropMap(propMap.get());
    }/////////////////////////

    {////////SourceECClassId
    DbColumn const* sourceClassIdCol = vtable->AddColumn(ECDBSYS_PROP_SourceECClassId, DbColumn::Type::Integer, DbColumn::Kind::Default, PersistenceType::Virtual);
    RefCountedPtr<ConstraintECClassIdPropertyMap> propMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Source, {sourceClassIdCol});
    if (propMap == nullptr || sourceClassIdCol == nullptr)
        {
        BeAssert(false && "Failed to create SourceECClassId PropertyMap");
        return ClassMappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(propMap, 3) != SUCCESS)
        return ClassMappingStatus::Error;

    GetConstraintMapR(ECRelationshipEnd_Source).SetECClassIdPropMap(propMap.get());
    }/////////////////////////

    {////////TargetECInstanceId
    DbColumn const* targetIdCol = vtable->AddColumn(ECDBSYS_PROP_TargetECInstanceId, DbColumn::Type::Integer, DbColumn::Kind::Default, PersistenceType::Virtual);
    RefCountedPtr<ConstraintECInstanceIdPropertyMap> propMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Target, {targetIdCol});
    if (propMap == nullptr || targetIdCol == nullptr)
        {
        BeAssert(false && "Failed to create PropertyMap TargetECInstanceId");
        return ClassMappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(propMap, 4) != SUCCESS)
        return ClassMappingStatus::Error;

    GetConstraintMapR(ECRelationshipEnd_Target).SetECInstanceIdPropMap(propMap.get());
    }/////////////////////////

    {////////TargetECClassId
    DbColumn const* targetEClassIdCol = vtable->AddColumn(ECDBSYS_PROP_TargetECClassId, DbColumn::Type::Integer, DbColumn::Kind::Default, PersistenceType::Virtual);
    RefCountedPtr<ConstraintECClassIdPropertyMap> propMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Target, {targetEClassIdCol});
    if (propMap == nullptr || targetEClassIdCol == nullptr)
        {
        BeAssert(false && "Failed to create PropertyMap TargetECClassId");
        return ClassMappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(propMap, 5) != SUCCESS)
        return ClassMappingStatus::Error;

    GetConstraintMapR(ECRelationshipEnd_Target).SetECClassIdPropMap(propMap.get());
    }/////////////////////////

    return ClassMappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus RelationshipClassEndTableMap::MapSubClass(RelationshipClassEndTableMap const& baseRelClassMap)
    {
    //ECInstanceId property map
    SystemPropertyMap const* basePropMap = baseRelClassMap.GetECInstanceIdPropertyMap();
    if (basePropMap == nullptr)
        {
        BeAssert(false);
        return ClassMappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(PropertyMapCopier::CreateCopy(*basePropMap, *this)) != SUCCESS)
        return ClassMappingStatus::Error;

    //ECClassId property map
    SystemPropertyMap const* classIdPropertyMap = baseRelClassMap.GetECClassIdPropertyMap();
    if (classIdPropertyMap == nullptr)
        {
        BeAssert(false);
        return ClassMappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(PropertyMapCopier::CreateCopy(*classIdPropertyMap, *this)) != SUCCESS)
        return ClassMappingStatus::Error;


    //Source
    RelationshipConstraintMap const& baseSourceConstraintMap = baseRelClassMap.GetConstraintMap(ECRelationshipEnd::ECRelationshipEnd_Source);
    if (baseSourceConstraintMap.GetECInstanceIdPropMap() == nullptr ||
        baseSourceConstraintMap.GetECClassIdPropMap() == nullptr)
        {
        BeAssert(false);
        return ClassMappingStatus::Error;
        }

    RelationshipConstraintMap& sourceConstraintMap = GetConstraintMapR(ECRelationshipEnd::ECRelationshipEnd_Source);

    //Source ECInstanceId prop map
    RefCountedPtr<SystemPropertyMap> clonedConstraintInstanceId = PropertyMapCopier::CreateCopy(*baseSourceConstraintMap.GetECInstanceIdPropMap(), *this);
    if (clonedConstraintInstanceId == nullptr)
        return ClassMappingStatus::Error;

    if (GetPropertyMapsR().Insert(clonedConstraintInstanceId) != SUCCESS)
        return ClassMappingStatus::Error;

    sourceConstraintMap.SetECInstanceIdPropMap(&clonedConstraintInstanceId->GetAs<ConstraintECInstanceIdPropertyMap>());

    GetTablesPropertyMapVisitor tableDisp;
    clonedConstraintInstanceId->AcceptVisitor(tableDisp);
    for (DbTable const* table : tableDisp.GetTables())
        {
        AddTable(*const_cast<DbTable *>(table));
        }

    //Source ECClassId prop map
    RefCountedPtr<SystemPropertyMap> clonedConstraintClassId = PropertyMapCopier::CreateCopy(*baseSourceConstraintMap.GetECClassIdPropMap(), *this);
    if (clonedConstraintClassId == nullptr)
        return ClassMappingStatus::Error;

    if (GetPropertyMapsR().Insert(clonedConstraintClassId) != SUCCESS)
        return ClassMappingStatus::Error;

    sourceConstraintMap.SetECClassIdPropMap(&clonedConstraintClassId->GetAs<ConstraintECClassIdPropertyMap>());

    //Target
    RelationshipConstraintMap const& baseTargetEndConstraintMap = baseRelClassMap.GetConstraintMap(ECRelationshipEnd::ECRelationshipEnd_Target);
    if (baseTargetEndConstraintMap.GetECInstanceIdPropMap() == nullptr ||
        baseTargetEndConstraintMap.GetECClassIdPropMap() == nullptr)
        {
        BeAssert(false);
        return ClassMappingStatus::Error;
        }

    RelationshipConstraintMap& targetEndConstraintMap = GetConstraintMapR(ECRelationshipEnd::ECRelationshipEnd_Target);

    //Target ECInstanceId prop map
    clonedConstraintInstanceId = PropertyMapCopier::CreateCopy(*baseTargetEndConstraintMap.GetECInstanceIdPropMap(), *this);
    if (clonedConstraintInstanceId == nullptr)
        return ClassMappingStatus::Error;

    if (GetPropertyMapsR().Insert(clonedConstraintInstanceId) != SUCCESS)
        return ClassMappingStatus::Error;

    targetEndConstraintMap.SetECInstanceIdPropMap(&clonedConstraintInstanceId->GetAs<ConstraintECInstanceIdPropertyMap>());

    //Target ECClassId prop map
    clonedConstraintClassId = PropertyMapCopier::CreateCopy(*baseTargetEndConstraintMap.GetECClassIdPropMap(), *this);
    if (clonedConstraintClassId == nullptr)
        return ClassMappingStatus::Error;
    if (GetPropertyMapsR().Insert(clonedConstraintClassId) != SUCCESS)
        return ClassMappingStatus::Error;

    targetEndConstraintMap.SetECClassIdPropMap(&clonedConstraintClassId->GetAs<ConstraintECClassIdPropertyMap>());

    return ClassMappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Affan.Khan           01/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipClassEndTableMap::_Load(ClassMapLoadContext& ctx, DbClassMapLoadContext const& mapInfo)
    {
    if (SUCCESS != ClassMap::_Load(ctx, mapInfo))
        return ERROR;

    //SourceECInstanceId
    std::vector<DbColumn const*> const* mapColumns = mapInfo.FindColumnByAccessString(ECDBSYS_PROP_SourceECInstanceId);
    if (mapColumns == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    RefCountedPtr<ConstraintECInstanceIdPropertyMap> sourceECInstanceIdPropMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Source, *mapColumns);
    if (sourceECInstanceIdPropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (GetPropertyMapsR().Insert(sourceECInstanceIdPropMap) != SUCCESS)
        return ERROR;

    m_sourceConstraintMap.SetECInstanceIdPropMap(sourceECInstanceIdPropMap.get());

    //SourceECClassId
    mapColumns = mapInfo.FindColumnByAccessString(ECDBSYS_PROP_SourceECClassId);
    if (mapColumns == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    RefCountedPtr<ConstraintECClassIdPropertyMap> sourceClassIdPropMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Source, *mapColumns);
    if (sourceClassIdPropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (GetPropertyMapsR().Insert(sourceClassIdPropMap) != SUCCESS)
        return ERROR;

    m_sourceConstraintMap.SetECClassIdPropMap(sourceClassIdPropMap.get());

    //TargetECInstanceId
    mapColumns = mapInfo.FindColumnByAccessString(ECDBSYS_PROP_TargetECInstanceId);
    if (mapColumns == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    RefCountedPtr<ConstraintECInstanceIdPropertyMap> targetECInstanceIdPropMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Target, *mapColumns);
    if (targetECInstanceIdPropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (GetPropertyMapsR().Insert(targetECInstanceIdPropMap) != SUCCESS)
        return ERROR;

    m_targetConstraintMap.SetECInstanceIdPropMap(targetECInstanceIdPropMap.get());

    //TargetECClassId
    mapColumns = mapInfo.FindColumnByAccessString(ECDBSYS_PROP_TargetECClassId);
    if (mapColumns == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    RefCountedPtr<ConstraintECClassIdPropertyMap> targetClassIdPropMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Target, *mapColumns);
    if (targetClassIdPropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (GetPropertyMapsR().Insert(targetClassIdPropMap) != SUCCESS)
        return ERROR;

    m_targetConstraintMap.SetECClassIdPropMap(targetClassIdPropMap.get());
    return SUCCESS;
    }

//******************************************************************************************************
// ForeignKeyPartitionView
//******************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
ForeignKeyPartitionView::ForeignKeyPartitionView(TableSpaceSchemaManager const& manager, ECN::ECRelationshipClassCR relationship, MapStrategy mapStrategy)
    :m_schemaManager(manager), m_relationshipClass(relationship), m_mapStrategy(mapStrategy)
    {
    BeAssert(m_mapStrategy == MapStrategy::ForeignKeyRelationshipInSourceTable ||
             m_mapStrategy == MapStrategy::ForeignKeyRelationshipInTargetTable);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static
std::unique_ptr<ForeignKeyPartitionView> ForeignKeyPartitionView::CreateReadonly(TableSpaceSchemaManager const& schemaManager, ECN::ECRelationshipClassCR relationship)
    {
    MapStrategy mapStrategy;
    if (GetMapStrategy(mapStrategy, schemaManager, relationship) != SUCCESS)
        return nullptr;

    BeAssert(mapStrategy == MapStrategy::ForeignKeyRelationshipInSourceTable ||
             mapStrategy == MapStrategy::ForeignKeyRelationshipInTargetTable);

    return Create(schemaManager, relationship, mapStrategy, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static
std::unique_ptr<ForeignKeyPartitionView> ForeignKeyPartitionView::Create(TableSpaceSchemaManager const& schemaManager, ECN::ECRelationshipClassCR relationship, MapStrategy mapStrategy)
    {
    MapStrategy persistedMapStrategy;
    if (GetMapStrategy(persistedMapStrategy, schemaManager, relationship) == SUCCESS)
        {
        BeAssert(mapStrategy == persistedMapStrategy);
        if (mapStrategy != persistedMapStrategy)
            return nullptr;
        }

    return Create(schemaManager, relationship, mapStrategy, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static
std::unique_ptr<ForeignKeyPartitionView> ForeignKeyPartitionView::Create(TableSpaceSchemaManager const& schemaManager, ECN::ECRelationshipClassCR relationship, MapStrategy mapStrategy, bool readonly)
    {
    // Following code modify state of schemaManager.GetDbSchema(). DbSchema hold not mutex.
    BeMutexHolder lock(schemaManager.GetECDb().GetImpl().GetMutex());
    enum class PropertyMapKind
        {
        Id = 1,
        RelEClassId = 2,
        Unknown = 3,
        };

    Utf8CP tableSpace = schemaManager.GetTableSpace().GetName().c_str();
    Utf8String sql;
    sql.Sprintf("SELECT DISTINCT CASE (SUBSTR (PP.AccessString, INSTR(PP.AccessString, '.') + 1)) WHEN 'Id' THEN 1 WHEN 'RelECClassId'THEN 2 ELSE 3 END PropertyMapKind, "
        "T.Name, C.Name FROM [%s].ec_Property P "
        "INNER JOIN [%s].ec_PropertyPath PP ON PP.RootPropertyId = P.Id "
        "INNER JOIN [%s].ec_PropertyMap PM ON PM.PropertyPathId = PP.Id "
        "INNER JOIN [%s].ec_Column C ON C.Id = PM.ColumnId "
        "INNER JOIN [%s].ec_Table T ON T.Id = C.TableId "
        "WHERE P.NavigationRelationshipClassId = ? ORDER BY T.Id, PM.ClassId, PropertyMapKind", tableSpace, tableSpace, tableSpace, tableSpace, tableSpace);


    CachedStatementPtr stmt = schemaManager.GetECDb().GetImpl().GetCachedSqliteStatement(sql.c_str());
    if (stmt == nullptr || BE_SQLITE_OK != stmt->BindId(1, GetRootClass(relationship).GetId()))
        {
        BeAssert(false);
        return nullptr;
        }

    std::unique_ptr<ForeignKeyPartitionView> partitionView(new ForeignKeyPartitionView(schemaManager, relationship, mapStrategy));
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        PropertyMapKind idPropertyMapKind = Enum::FromInt<PropertyMapKind>(stmt->GetValueInt(0));
        if (idPropertyMapKind != PropertyMapKind::Id)
            return nullptr;

        Utf8CP tableName = stmt->GetValueText(1);
        DbTable const* table = schemaManager.GetDbSchema().FindTable(tableName);
        if (table == nullptr)
            return nullptr;

        DbColumn const* refIdCol = table->FindColumn(stmt->GetValueText(2));
        if (refIdCol == nullptr)
            return nullptr;

        //now step to next row which is expected to be RelECClassId col
        if (stmt->Step() != BE_SQLITE_ROW)
            return nullptr;

        PropertyMapKind relECClassIdPropertyMapKind = Enum::FromInt<PropertyMapKind>(stmt->GetValueInt(0));
        if (relECClassIdPropertyMapKind != PropertyMapKind::RelEClassId)
            return nullptr;

        Utf8CP relECClassIdTableName = stmt->GetValueText(1);
        if (!table->GetName().EqualsIAscii(relECClassIdTableName))
            return nullptr;

        DbColumn const* refRelECClassIdCol = table->FindColumn(stmt->GetValueText(2));
        if (refRelECClassIdCol == nullptr)
            return nullptr;

        std::unique_ptr<Partition> partition = Partition::Create(*partitionView, *refIdCol, *refRelECClassIdCol);
        if (partition == nullptr)
            return nullptr;

        partition->MarkPersisted();
        if (partitionView->Insert(std::move(partition)) != SUCCESS)
            return nullptr;
        }

    if (partitionView->UpdateFromECClassIdColumn() != SUCCESS)
        return nullptr;

    partitionView->m_readonly = readonly;
    return partitionView;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static
BentleyStatus ForeignKeyPartitionView::GetMapStrategy(MapStrategy& mapStrategy, TableSpaceSchemaManager const& schemaManager, ECN::ECRelationshipClassCR rel)
    {
    CachedStatementPtr stmt = schemaManager.GetECDb().GetImpl().GetCachedSqliteStatement(Utf8PrintfString("SELECT MapStrategy FROM [%s]." TABLE_ClassMap " WHERE ClassId=?",
                                                                                                          schemaManager.GetTableSpace().GetName().c_str()).c_str());
    stmt->BindId(1, rel.GetId());
    if (stmt->Step() == BE_SQLITE_DONE)
        return ERROR;

    mapStrategy = Enum::FromInt<MapStrategy>(stmt->GetValueInt(0));
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
ECN::ECRelationshipClassCR ForeignKeyPartitionView::GetRootClass(ECN::ECRelationshipClassCR ecRelationshipClass)
    {
    if (!ecRelationshipClass.HasBaseClasses())
        return ecRelationshipClass;

    BeAssert(ecRelationshipClass.GetBaseClasses().size() == 1 && ecRelationshipClass.GetBaseClasses().front()->GetRelationshipClassCP() != nullptr);
    return GetRootClass(*ecRelationshipClass.GetBaseClasses().front()->GetRelationshipClassCP());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
std::vector<ForeignKeyPartitionView::Partition const*> ForeignKeyPartitionView::GetPartitions(bool onlyPhysical, bool onlyConcrete) const
    {
    std::vector<Partition const*> result;
    for (std::unique_ptr<Partition> const& partition : m_partitions)
        {
        if (onlyPhysical && !partition->IsPhysical())
            continue;

        if (onlyConcrete && !partition->IsConcrete())
            continue;

        result.push_back(partition.get());
        }

    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
std::vector<ForeignKeyPartitionView::Partition const*> ForeignKeyPartitionView::GetPartitions(DbTable const& table, bool onlyPhysical, bool onlyConcrete) const
    {
    std::vector<Partition const*> result;
    for (Partition const* partition : GetPartitions(onlyPhysical, onlyConcrete))
        {
        if (partition->GetECInstanceIdColumn().GetTable() == table)
            result.push_back(partition);
        }

    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ForeignKeyPartitionView::UpdateFromECClassIdColumn()
    {
    BeAssert(Readonly() == false);
    if (Readonly())
        return ERROR;

    DbColumn const* fromECClassIdColumn;
    if (TryGetFromECClassIdColumn(fromECClassIdColumn) != SUCCESS)
        return ERROR;

    for (std::unique_ptr<Partition> const& partition : m_partitions)
        partition->SetFromECClassIdColumn(fromECClassIdColumn);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
bool ForeignKeyPartitionView::Contains(ForeignKeyPartitionView::Partition const& partition) const
    {
    for (std::unique_ptr<Partition> const& existingPartition : m_partitions)
        if (partition.GetHashCode() == existingPartition->GetHashCode())
            return true;

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
ForeignKeyPartitionView::Partition const* ForeignKeyPartitionView::FindCompatiblePartiton(NavigationPropertyMap const& navigationPropertyMap) const
    {
    for (std::unique_ptr<Partition> const& partition : m_partitions)
        {
        const ClassMap& navClassMap = navigationPropertyMap.GetClassMap();
        const DbTable& partitionTable = partition->GetECInstanceIdColumn().GetTable();
        if (!navClassMap.IsMappedTo(partitionTable))
            {
            if (navClassMap.GetOverflowTable() == nullptr && partitionTable.GetType() == DbTable::Type::Overflow)
                {
                const DbTable::LinkNode* linkNode = navClassMap.GetJoinedOrPrimaryTable().GetLinkNode().FindOverflowTable();
                if (linkNode && &linkNode->GetTable()== &partitionTable)
                    {
                    if (const_cast<ClassMap&>(navClassMap).SetOverflowTable(linkNode->GetTableR()) != SUCCESS)
                        {
                        BeAssert(false && "SetOverflowTable() failed");
                        return nullptr;
                        }

                    return partition.get();
                    }
                }

            continue;
            }

        NavigationInfo navColumns = partition->GetNavigationColumns();
        if (navClassMap.GetColumnFactory().IsColumnInUse(navColumns.GetIdColumn()))
            continue;

        if (navClassMap.GetColumnFactory().IsColumnInUse(navColumns.GetRelECClassIdColumn()))
            continue;

        return partition.get();
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ForeignKeyPartitionView::Insert(std::unique_ptr<Partition> partition)
    {
    BeAssert(Readonly() == false);
    if (Readonly())
        return ERROR;

    BeAssert(partition != nullptr);
    if (partition == nullptr)
        return ERROR;

    const bool contains = Contains(*partition);
    BeAssert(contains == false);
    if (contains)
        return ERROR;

    m_partitions.push_back(std::move(partition));
    if (m_updateFromECClassIdColumnOnInsert)
        return UpdateFromECClassIdColumn();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ForeignKeyPartitionView::TryGetFromECClassIdColumn(DbColumn const*& column) const
    {
    DbTable const* otherEndTable = nullptr;
    if (SUCCESS != TryGetOtherEndTable(otherEndTable, m_schemaManager, m_relationshipClass, m_mapStrategy))
        return ERROR;

    column = otherEndTable != nullptr ? otherEndTable->FindFirst(DbColumn::Kind::ECClassId) : nullptr;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static
BentleyStatus ForeignKeyPartitionView::TryGetOtherEndTable(DbTable const*& otherEndTable, TableSpaceSchemaManager const& schemaManager, ECRelationshipClassCR relationshipClass, MapStrategy strategy)
    {
    if (strategy != MapStrategy::ForeignKeyRelationshipInSourceTable &&
        strategy != MapStrategy::ForeignKeyRelationshipInTargetTable)
        {
        BeAssert(false && " May only call this for end table relationships");
        return ERROR;
        }

    ECRelationshipConstraintCR otherEndConstraint = ECRelationshipEnd::ECRelationshipEnd_Source ? relationshipClass.GetSource() : relationshipClass.GetTarget();

    Utf8CP tableSpace = schemaManager.GetTableSpace().GetName().c_str();
    Utf8String sql = otherEndConstraint.GetIsPolymorphic() ?
        Utf8PrintfString("SELECT DISTINCT CHT.TableId FROM [%s].ec_RelationshipConstraintClass RCC "
        "INNER JOIN [%s].ec_RelationshipConstraint RC ON RC.Id=RCC.ConstraintId "
        "INNER JOIN [%s].ec_Table T ON T.Id=CHT.TableId "
        "INNER JOIN [%s].ec_cache_ClassHierarchy CH ON CH.BaseClassId=RCC.ClassId "
        "INNER JOIN [%s].ec_cache_ClassHasTables CHT ON CHT.ClassId=CH.ClassId "
        "WHERE  RC.RelationshipClassId = ?  AND RC.RelationshipEnd=? AND T.Type!=" SQLVAL_DbTable_Type_Joined " AND T.Type!=" SQLVAL_DbTable_Type_Overflow,
                         tableSpace, tableSpace, tableSpace, tableSpace, tableSpace)
        : Utf8PrintfString("SELECT DISTINCT CHT.TableId FROM [%s].ec_RelationshipConstraintClass RCC "
        "INNER JOIN [%s].ec_RelationshipConstraint RC ON RC.Id=RCC.ConstraintId "
        "INNER JOIN [%s].ec_Table T ON T.Id=CHT.TableId "
        "INNER JOIN [%s].ec_cache_ClassHasTables CHT ON CHT.ClassId=RCC.ClassId "
        "WHERE  RC.RelationshipClassId=? AND RC.RelationshipEnd=? AND T.Type!=" SQLVAL_DbTable_Type_Joined " AND T.Type!=" SQLVAL_DbTable_Type_Overflow,
                    tableSpace, tableSpace, tableSpace, tableSpace);

    CachedStatementPtr stmt = schemaManager.GetECDb().GetImpl().GetCachedSqliteStatement(sql.c_str());
    if (stmt == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    stmt->BindId(1, GetRootClass(relationshipClass).GetId());
    const ECRelationshipEnd otherEnd = strategy == MapStrategy::ForeignKeyRelationshipInSourceTable ? ECRelationshipEnd::ECRelationshipEnd_Target : ECRelationshipEnd::ECRelationshipEnd_Source;
    stmt->BindInt(2, Enum::ToInt(otherEnd));

    std::vector<DbTable const*> list;
    std::vector<DbTable const*> nvlist;
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        if (DbTable const* table = schemaManager.GetDbSchema().FindTable(stmt->GetValueId<DbTableId>(0)))
            {
            list.push_back(table);
            if (table->GetType() != DbTable::Type::Virtual)
                nvlist.push_back(table);
            }
        else
            {
            BeAssert(false);
            return ERROR;
            }
        }

    if (!nvlist.empty())
        {
        if (nvlist.size() > 1)
            return ERROR;

        otherEndTable = nvlist[0];
        return SUCCESS;
        }

    if (list.size() > 1)
        return ERROR;

    if (list.empty())
        otherEndTable = nullptr;
    else
        otherEndTable = list[0];

    return SUCCESS;
    }

//******************************************************************************************************
// ForeignKeyPartitionView::Partition
//******************************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
void ForeignKeyPartitionView::Partition::SetFromECClassIdColumn(DbColumn const* column)
    {
    SetColumn(m_fkInfo.GetPersistedEnd() == PersistedEnd::TargetTable ? ColumnId::SourceECClassId : ColumnId::TargetECClassId, column);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
bool ForeignKeyPartitionView::Partition::IsConcrete() const
    {
    for (DbColumn const* col : m_cols)
        {
        if (!col)
            return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
bool ForeignKeyPartitionView::Partition::IsPhysical() const { return GetECInstanceIdColumn().GetTable().GetType() != DbTable::Type::Virtual; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
uint64_t ForeignKeyPartitionView::Partition::QuickHash64(Utf8CP str, uint64_t mix)
    {
    const uint64_t mulp = 2654435789;
    mix ^= 104395301;
    while (*str)
        mix += (*str++ * mulp) ^ (mix >> 23);

    return mix ^ (mix << 37);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
DbColumn const* ForeignKeyPartitionView::Partition::GetColumn(ColumnId columnId) const { return m_cols[(int) columnId]; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ForeignKeyPartitionView::Partition::SetColumn(ColumnId columnId, DbColumn const* column)
    {
    m_cols[(int) columnId] = column;
    UpdateHash();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
void ForeignKeyPartitionView::Partition::UpdateHash()
    {
    m_hashCode = 0;
    for (DbColumn const* col : m_cols)
        {
        if (col != nullptr)
            {
            m_hashCode = QuickHash64(col->GetTable().GetName().c_str(), m_hashCode);
            m_hashCode = QuickHash64(col->GetName().c_str(), m_hashCode);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
ForeignKeyPartitionView::Partition::Partition(ForeignKeyPartitionView const& fkInfo) : m_fkInfo(fkInfo), m_cols() {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
std::unique_ptr<ForeignKeyPartitionView::Partition> ForeignKeyPartitionView::Partition::Create(ForeignKeyPartitionView const& info, DbColumn const& navId, DbColumn const& navRelECClassId)
    {
    DbTable const& table = navId.GetTable();
    if (&table != &navRelECClassId.GetTable())
        return nullptr;

    std::unique_ptr<Partition> partition = std::unique_ptr<Partition>(new Partition(info));
    DbColumn const* instanceId = table.FindFirst(DbColumn::Kind::ECInstanceId);
    DbColumn const* classId = table.FindFirst(DbColumn::Kind::ECClassId);
    if (instanceId == nullptr || classId == nullptr)
        return nullptr;

    if (partition->SetColumn(ColumnId::ECInstanceId, instanceId) == ERROR)
        return nullptr;

    const bool persistedInSource = info.GetPersistedEnd() == PersistedEnd::SourceTable;
    //ToECInstanceId
    if (partition->SetColumn((persistedInSource ? ColumnId::SourceECInstanceId : ColumnId::TargetECInstanceId), instanceId) == ERROR)
        return nullptr;

    //ToECClassId
    if (partition->SetColumn(persistedInSource ? ColumnId::SourceECClassId : ColumnId::TargetECClassId, classId) == ERROR)
        return nullptr;

    if (partition->SetColumn(ColumnId::ECClassId, &navRelECClassId) == ERROR)
        return nullptr;

    //FromECInstanceId
    if (partition->SetColumn(persistedInSource ? ColumnId::TargetECInstanceId : ColumnId::SourceECInstanceId, &navId) == ERROR)
        return nullptr;

    return partition;
    }



//************************** RelationshipClassLinkTableMap *****************************************
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipClassLinkTableMap::RelationshipClassLinkTableMap(ECDb const& ecdb, TableSpaceSchemaManager const& manager, ECN::ECClassCR ecRelClass, MapStrategyExtendedInfo const& mapStrategy)
    : RelationshipClassMap(ecdb, manager, Type::RelationshipLinkTable, ecRelClass, mapStrategy)
    {}

//---------------------------------------------------------------------------------------
//@bsimethod                                   Ramanujam.Raman                   06 / 12
//---------------------------------------------------------------------------------------
ClassMappingStatus RelationshipClassLinkTableMap::_Map(ClassMappingContext& ctx)
    {
    BeAssert(!MapStrategyExtendedInfo::IsForeignKeyMapping(GetMapStrategy()) &&
             "RelationshipClassLinkTableMap is not meant to be used with other map strategies.");

    ClassMappingStatus stat = DoMapPart1(ctx);
    if (stat != ClassMappingStatus::Success)
        return stat;

    if (GetRelationshipClass().HasBaseClasses())
        return MapSubClass(ctx);

    ECRelationshipConstraintCR sourceConstraint = GetRelationshipClass().GetSource();
    ECRelationshipConstraintCR targetConstraint = GetRelationshipClass().GetTarget();

    LinkTableRelationshipMapCustomAttribute ca;
    ECDbMapCustomAttributeHelper::TryGetLinkTableRelationshipMap(ca, GetRelationshipClass());

    //*** root rel class
    //Table retrieval is only needed for the root rel class. Subclasses will use the tables of its base class
    //TODO: How should we handle this properly?
    std::set<DbTable const*> sourceTables = ctx.GetImportCtx().GetSchemaManager().GetRelationshipConstraintPrimaryTables(ctx.GetImportCtx(), sourceConstraint);
    std::set<DbTable const*> targetTables = ctx.GetImportCtx().GetSchemaManager().GetRelationshipConstraintPrimaryTables(ctx.GetImportCtx(), targetConstraint);

    if (sourceTables.empty() || targetTables.empty())
        {
        Issues().ReportV("Failed to map ECRelationshipClass '%s'. Source or target constraint classes are abstract without subclasses. Consider applying the MapStrategy 'TablePerHierarchy' to the abstract constraint class.",
                        GetClass().GetFullName());
        return ClassMappingStatus::Error;
        }

    const size_t sourceTableCount = sourceTables.size();
    const size_t targetTableCount = targetTables.size();
    if (sourceTableCount > 1 || targetTableCount > 1)
        {
        Utf8CP constraintStr = nullptr;
        if (sourceTableCount > 1 && targetTableCount > 1)
            constraintStr = "source and target constraints are";
        else if (sourceTableCount > 1)
            constraintStr = "source constraint is";
        else
            constraintStr = "target constraint is";

        Issues().ReportV("Failed to map ECRelationshipClass '%s'. It is mapped to a link table, but the %s mapped to more than one table, which is not supported for link tables.",
                        GetClass().GetFullName(), constraintStr);

        return ClassMappingStatus::Error;
        }

    DbTable const* sourceTable = *sourceTables.begin();
    DbTable const* targetTable = *targetTables.begin();

    bool createFkConstraints = true; // default
    if (ca.IsValid())
        {
        Nullable<bool> createFkConstraintsVal;
        if (SUCCESS != ca.TryGetCreateForeignKeyConstraints(createFkConstraintsVal))
            return ClassMappingStatus::Error;

        if (!createFkConstraintsVal.IsNull())
            createFkConstraints = createFkConstraintsVal.Value();
        }

    //**** Constraint columns and prop maps
    bool addSourceECClassIdColumnToTable = false;
    DetermineConstraintClassIdColumnHandling(addSourceECClassIdColumnToTable, sourceConstraint);

    bool addTargetECClassIdColumnToTable = false;
    DetermineConstraintClassIdColumnHandling(addTargetECClassIdColumnToTable, targetConstraint);
    stat = CreateConstraintPropMaps(ctx.GetImportCtx(), ca, addSourceECClassIdColumnToTable, addTargetECClassIdColumnToTable);
    if (stat != ClassMappingStatus::Success)
        return stat;

    stat = DoMapPart2(ctx);
    if (stat != ClassMappingStatus::Success)
        return stat;


    //only create constraints on TPH root or if not TPH and not existing table
    if (createFkConstraints && GetPrimaryTable().GetType() != DbTable::Type::Existing && (!GetMapStrategy().IsTablePerHierarchy() || GetTphHelper()->DetermineTphRootClassId() == GetClass().GetId()))
        {
        //Create FK from Source-Primary to LinkTable
        DbColumn const* fkColumn = &GetSourceECInstanceIdPropMap()->FindDataPropertyMap(GetPrimaryTable())->GetColumn();
        DbColumn const* referencedColumn = sourceTable->FindFirst(DbColumn::Kind::ECInstanceId);
        GetPrimaryTable().AddForeignKeyConstraint(*fkColumn, *referencedColumn, ForeignKeyDbConstraint::ActionType::Cascade, ForeignKeyDbConstraint::ActionType::NotSpecified);

        //Create FK from Target-Primary to LinkTable
        fkColumn = &GetTargetECInstanceIdPropMap()->FindDataPropertyMap(GetPrimaryTable())->GetColumn();
        referencedColumn = targetTable->FindFirst(DbColumn::Kind::ECInstanceId);
        GetPrimaryTable().AddForeignKeyConstraint(*fkColumn, *referencedColumn, ForeignKeyDbConstraint::ActionType::Cascade, ForeignKeyDbConstraint::ActionType::NotSpecified);
        }

    Nullable<bool> allowDuplicateRelationships;
    if (ca.IsValid())
        {
        if (SUCCESS != ca.TryGetAllowDuplicateRelationships(allowDuplicateRelationships))
            return ClassMappingStatus::Error;
       
        if (!allowDuplicateRelationships.IsNull())
            {
            GetECDb().GetImpl().Issues().ReportV("Warning: ECRelationshipClass '%s': has set deprecated property 'AllowDuplicateRelationships' of mapping customattribute LinkTableRelationshipMap. The value of 'AllowDuplicateRelationships' will be ignored.",
                                 GetClass().GetFullName());
            }
        }

    if (!GetMapStrategy().GetTphInfo().IsValid())
        AddIndices(ctx.GetImportCtx(), GetAllowDuplicateRelationshipsFlag(allowDuplicateRelationships));
    else
        AddDefaultIndexes(ctx.GetImportCtx());

    return ClassMappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus RelationshipClassLinkTableMap::MapSubClass(ClassMappingContext& ctx)
    {
    if (GetClass().GetBaseClasses().size() != 1)
        {
        BeAssert(false && "Multi-inheritance of ECRelationshipclasses should have been caught before already");
        return ClassMappingStatus::Error;
        }

    ECClassCP baseClass = GetClass().GetBaseClasses()[0];
    ClassMap const* baseClassMap = ctx.GetImportCtx().GetSchemaManager().GetClassMap(*baseClass);
    if (baseClassMap == nullptr || baseClassMap->GetType() != ClassMap::Type::RelationshipLinkTable)
        {
        BeAssert(false && "Could not find class map of base ECRelationship class or is not of right type");
        return ClassMappingStatus::Error;
        }

    RelationshipClassLinkTableMap const& baseRelClassMap = baseClassMap->GetAs<RelationshipClassLinkTableMap>();

    //SourceECInstanceId prop map
    RefCountedPtr<SystemPropertyMap> clonedSourceECInstanceIdPropMap = PropertyMapCopier::CreateCopy(*baseRelClassMap.GetSourceECInstanceIdPropMap(), *this);
    if (GetPropertyMapsR().Insert(clonedSourceECInstanceIdPropMap) != SUCCESS)
        return ClassMappingStatus::Error;

    m_sourceConstraintMap.SetECInstanceIdPropMap(&clonedSourceECInstanceIdPropMap->GetAs<ConstraintECInstanceIdPropertyMap>());

    //SourceECClassId prop map
    RefCountedPtr<SystemPropertyMap> clonedSourceECClassIdPropMap = PropertyMapCopier::CreateCopy(*baseRelClassMap.GetSourceECClassIdPropMap(), *this);
    if (GetPropertyMapsR().Insert(clonedSourceECClassIdPropMap) != SUCCESS)
        return ClassMappingStatus::Error;

    m_sourceConstraintMap.SetECClassIdPropMap(&clonedSourceECClassIdPropMap->GetAs<ConstraintECClassIdPropertyMap>());

    //TargetECInstanceId prop map
    RefCountedPtr<SystemPropertyMap> clonedTargetECInstanceIdPropMap = PropertyMapCopier::CreateCopy(*baseRelClassMap.GetTargetECInstanceIdPropMap(), *this);
    if (GetPropertyMapsR().Insert(clonedTargetECInstanceIdPropMap) != SUCCESS)
        return ClassMappingStatus::Error;

    m_targetConstraintMap.SetECInstanceIdPropMap(&clonedTargetECInstanceIdPropMap->GetAs<ConstraintECInstanceIdPropertyMap>());

    //TargetECClassId prop map
    RefCountedPtr<SystemPropertyMap> clonedTargetECClassIdPropMap = PropertyMapCopier::CreateCopy(*baseRelClassMap.GetTargetECClassIdPropMap(), *this);
    if (GetPropertyMapsR().Insert(clonedTargetECClassIdPropMap) != SUCCESS)
        return ClassMappingStatus::Error;

    m_targetConstraintMap.SetECClassIdPropMap(&clonedTargetECClassIdPropMap->GetAs<ConstraintECClassIdPropertyMap>());

    ClassMappingStatus stat = DoMapPart2(ctx);
    if (stat != ClassMappingStatus::Success)
        return stat;


    if (!GetMapStrategy().GetTphInfo().IsValid())
        AddIndices(ctx.GetImportCtx(), DetermineAllowDuplicateRelationshipsFlagFromRoot(*baseClass->GetRelationshipClassCP()));

    return ClassMappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Affan.Khan                         04 / 15
//---------------------------------------------------------------------------------------
DbColumn* RelationshipClassLinkTableMap::ConfigureForeignECClassIdKey(SchemaImportContext& ctx, LinkTableRelationshipMapCustomAttribute const& ca, ECRelationshipEnd relationshipEnd)
    {
    DbColumn* endECClassIdColumn = nullptr;
    ECRelationshipConstraintCR foreignEndConstraint = relationshipEnd == ECRelationshipEnd_Source ? GetRelationshipClass().GetSource() : GetRelationshipClass().GetTarget();
    ECClass const* foreignEndClass = foreignEndConstraint.GetConstraintClasses()[0];
    ClassMap const* foreignEndClassMap = ctx.GetSchemaManager().GetClassMap(*foreignEndClass);
    size_t foreignEndTableCount = ctx.GetSchemaManager().GetRelationshipConstraintTableCount(ctx, foreignEndConstraint);

    Utf8String columnName = DetermineConstraintECClassIdColumnName(ca, relationshipEnd);
    if (columnName.empty())
        return nullptr;

    if (GetPrimaryTable().FindColumn(columnName.c_str()) != nullptr &&
        !GetMapStrategy().IsTablePerHierarchy() && GetMapStrategy().GetStrategy() != MapStrategy::ExistingTable)
        {
        //Following error occurs in Upgrading ECSchema but is not fatal.
        LOG.errorv("Failed to map ECRelationshipClass '%s': Table '%s' already contains column named '%s'.",
                   GetClass().GetFullName(), GetPrimaryTable().GetName().c_str(), columnName.c_str());
        return nullptr;
        }

    if (foreignEndTableCount > 1)
        {
        //! We will create ECClassId column in this case
        endECClassIdColumn = CreateConstraintColumn(columnName.c_str(), PersistenceType::Physical);
        BeAssert(endECClassIdColumn != nullptr);
        }
    else
        {
        //! We will use JOIN to otherTable to get the ECClassId (if any)
        endECClassIdColumn = const_cast<DbColumn*>(foreignEndClassMap->GetPrimaryTable().FindFirst(DbColumn::Kind::ECClassId));
        if (endECClassIdColumn == nullptr)
            endECClassIdColumn = CreateConstraintColumn(columnName.c_str(), PersistenceType::Virtual);
        }

    return endECClassIdColumn;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus RelationshipClassLinkTableMap::CreateConstraintPropMaps(SchemaImportContext& ctx, LinkTableRelationshipMapCustomAttribute const& ca, bool addSourceECClassIdColumnToTable,
    bool addTargetECClassIdColumnToTable)
    {
    //**** SourceECInstanceId prop map 
    Utf8String columnName = DetermineConstraintECInstanceIdColumnName(ca, ECRelationshipEnd_Source);
    if (columnName.empty() || GetPrimaryTable().FindColumn(columnName.c_str()) != nullptr && !GetMapStrategy().IsTablePerHierarchy() && GetMapStrategy().GetStrategy() != MapStrategy::ExistingTable)
        {
        ctx.Issues().ReportV("Failed to map ECRelationshipClass '%s': Table '%s' already contains " ECDBSYS_PROP_SourceECInstanceId " column named '%s'.",
                   GetClass().GetFullName(), GetPrimaryTable().GetName().c_str(), columnName.c_str());
        return ClassMappingStatus::Error;
        }

    DbColumn const* sourceECInstanceIdColumn = CreateConstraintColumn(columnName.c_str(), PersistenceType::Physical);
    if (sourceECInstanceIdColumn == nullptr)
        return ClassMappingStatus::Error;

    auto sourceECInstanceIdPropMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Source, {sourceECInstanceIdColumn});
    PRECONDITION(sourceECInstanceIdPropMap.IsValid(), ClassMappingStatus::Error);
    if (GetPropertyMapsR().Insert(sourceECInstanceIdPropMap) != SUCCESS)
        return ClassMappingStatus::Error;

    m_sourceConstraintMap.SetECInstanceIdPropMap(sourceECInstanceIdPropMap.get());

    //**** SourceECClassId prop map
    DbColumn const* sourceECClassIdColumn = ConfigureForeignECClassIdKey(ctx, ca, ECRelationshipEnd_Source);
    auto sourceECClassIdPropMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Source, {sourceECClassIdColumn} );
    PRECONDITION(sourceECClassIdPropMap.IsValid(), ClassMappingStatus::Error);
    if (GetPropertyMapsR().Insert(sourceECClassIdPropMap) != SUCCESS)
        return ClassMappingStatus::Error;

    m_sourceConstraintMap.SetECClassIdPropMap(sourceECClassIdPropMap.get());


    //**** TargetECInstanceId prop map 
    columnName = DetermineConstraintECInstanceIdColumnName(ca, ECRelationshipEnd_Target);
    if (columnName.empty() || GetPrimaryTable().FindColumn(columnName.c_str()) != nullptr && !GetMapStrategy().IsTablePerHierarchy() && GetMapStrategy().GetStrategy() != MapStrategy::ExistingTable)
        {
        LOG.errorv("Failed to map ECRelationshipClass '%s': Table '%s' already contains " ECDBSYS_PROP_TargetECInstanceId " column named '%s'.",
                   GetClass().GetFullName(), GetPrimaryTable().GetName().c_str(), columnName.c_str());
        return ClassMappingStatus::Error;
        }

    DbColumn const* targetECInstanceIdColumn = CreateConstraintColumn(columnName.c_str(), PersistenceType::Physical);
    if (targetECInstanceIdColumn == nullptr)
        return ClassMappingStatus::Error;

    auto targetECInstanceIdPropMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Target, {targetECInstanceIdColumn});
    PRECONDITION(targetECInstanceIdPropMap.IsValid(), ClassMappingStatus::Error);
    if (GetPropertyMapsR().Insert(targetECInstanceIdPropMap) != SUCCESS)
        return ClassMappingStatus::Error;

    m_targetConstraintMap.SetECInstanceIdPropMap(targetECInstanceIdPropMap.get());


    //**** TargetECClassId prop map
    DbColumn const* targetECClassIdColumn = ConfigureForeignECClassIdKey(ctx, ca, ECRelationshipEnd_Target);
    auto targetECClassIdPropMap = ConstraintECClassIdPropertyMap::CreateInstance(*this , ECRelationshipEnd_Target, {targetECClassIdColumn});
    if (targetECClassIdPropMap == nullptr)
        {
        BeAssert(false);
        return ClassMappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(targetECClassIdPropMap) != SUCCESS)
        return ClassMappingStatus::Error;

    m_targetConstraintMap.SetECClassIdPropMap(targetECClassIdPropMap.get());
    return ClassMappingStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                            09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipClassLinkTableMap::AddIndices(SchemaImportContext& ctx, bool allowDuplicateRelationships)
    {
    if (GetPrimaryTable().GetType() == DbTable::Type::Existing)
        return;

    // Add indices on the source and target based on cardinality
    //(the many side can be unique, but the one side must never be unique)
    const bool sourceIsUnique = !allowDuplicateRelationships && (GetRelationshipClass().GetTarget().GetMultiplicity().GetUpperLimit() <= 1);
    const bool targetIsUnique = !allowDuplicateRelationships && (GetRelationshipClass().GetSource().GetMultiplicity().GetUpperLimit() <= 1);

    AddIndex(ctx, RelationshipIndexSpec::Source, sourceIsUnique);
    AddIndex(ctx, RelationshipIndexSpec::Target, targetIsUnique);

    if (!allowDuplicateRelationships)
        AddIndex(ctx, RelationshipClassLinkTableMap::RelationshipIndexSpec::SourceAndTarget, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                            07/19
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipClassLinkTableMap::AddDefaultIndexes(SchemaImportContext& ctx)
    {
    if (GetPrimaryTable().GetType() == DbTable::Type::Existing)
        return;
    
    //1. Unique index on (source,target,classid)
    AddIndex(ctx, RelationshipIndexSpec::SourceAndTargetAndClassId, true);
    //2. None unique index on (target);
    AddIndex(ctx, RelationshipIndexSpec::Target, false);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipClassLinkTableMap::AddIndex(SchemaImportContext& schemaImportContext, RelationshipIndexSpec spec, bool isUniqueIndex)
    {
    // Setup name of the index
    Utf8String name(isUniqueIndex ? "uix_" : "ix_");
    name.append(GetClass().GetSchema().GetAlias()).append("_").append(GetClass().GetName()).append("_");

    switch (spec)
        {
            case RelationshipIndexSpec::Source:
                name.append("source");
                break;
            case RelationshipIndexSpec::Target:
                name.append("target");
                break;
            case RelationshipIndexSpec::SourceAndTarget:
                name.append("sourcetarget");
                break;
            case RelationshipIndexSpec::SourceAndTargetAndClassId:
                name.append("sourcetargetclassid");
                break;
            default:
                BeAssert(false);
                break;
        }

    auto sourceECInstanceIdColumn = &GetSourceECInstanceIdPropMap()->FindDataPropertyMap(GetPrimaryTable())->GetColumn();
    auto sourceECClassIdColumn = GetSourceECClassIdPropMap()->FindDataPropertyMap(GetPrimaryTable()) != nullptr ? &GetSourceECClassIdPropMap()->FindDataPropertyMap(GetPrimaryTable())->GetColumn() : nullptr;
    auto targetECInstanceIdColumn = &GetTargetECInstanceIdPropMap()->FindDataPropertyMap(GetPrimaryTable())->GetColumn();
    auto targetECClassIdColumn = GetTargetECClassIdPropMap()->FindDataPropertyMap(GetPrimaryTable()) != nullptr ? &GetTargetECClassIdPropMap()->FindDataPropertyMap(GetPrimaryTable())->GetColumn() : nullptr;
    auto classId = &GetECClassIdPropertyMap()->FindDataPropertyMap(GetPrimaryTable())->GetColumn();

    std::vector<DbColumn const*> columns;
    switch (spec)
        {
            case RelationshipIndexSpec::Source:
                GenerateIndexColumnList(columns, sourceECInstanceIdColumn, sourceECClassIdColumn, nullptr, nullptr, nullptr);
                break;
            case RelationshipIndexSpec::Target:
                GenerateIndexColumnList(columns, targetECInstanceIdColumn, targetECClassIdColumn, nullptr, nullptr, nullptr);
                break;
            case RelationshipIndexSpec::SourceAndTarget:
                GenerateIndexColumnList(columns, sourceECInstanceIdColumn, sourceECClassIdColumn, targetECInstanceIdColumn, targetECClassIdColumn, nullptr);
                break;
            case RelationshipIndexSpec::SourceAndTargetAndClassId:
                GenerateIndexColumnList(columns, sourceECInstanceIdColumn, sourceECClassIdColumn, targetECInstanceIdColumn, targetECClassIdColumn, classId);
                break;
            default:
                BeAssert(false);
                break;
        }

    if (SUCCESS != DbMappingManager::Tables::CreateIndex(schemaImportContext, GetPrimaryTable(), name, isUniqueIndex, columns, false, true, GetClass().GetId(),
                                                         //if a partial index is created, it must only apply to this class,
                                                         //not to subclasses, as constraints are not inherited by relationships
                                                         false))
        {
        BeAssert(false && "Failed to create index for link table relationship");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipClassLinkTableMap::GenerateIndexColumnList(std::vector<DbColumn const*>& columns, DbColumn const* col1, DbColumn const* col2, DbColumn const* col3, DbColumn const* col4, DbColumn const* col5)
    {
    if (nullptr != col1 && col1->GetPersistenceType() == PersistenceType::Physical)
        columns.push_back(col1);

    if (nullptr != col2 && col2->GetPersistenceType() == PersistenceType::Physical)
        columns.push_back(col2);

    if (nullptr != col3 && col3->GetPersistenceType() == PersistenceType::Physical)
        columns.push_back(col3);

    if (nullptr != col4 && col4->GetPersistenceType() == PersistenceType::Physical)
        columns.push_back(col4);

    if (nullptr != col5 && col5->GetPersistenceType() == PersistenceType::Physical)
        columns.push_back(col5);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
//static
Utf8String RelationshipClassLinkTableMap::DetermineConstraintECInstanceIdColumnName(LinkTableRelationshipMapCustomAttribute const& ca, ECN::ECRelationshipEnd end)
    {
    Utf8String colName;
    switch (end)
        {
            case ECRelationshipEnd_Source:
            {
            Nullable<Utf8String> sourceIdColName;
            if (ca.IsValid())
                {
                if (SUCCESS != ca.TryGetSourceECInstanceIdColumn(sourceIdColName))
                    return Utf8String();
                }

            if (sourceIdColName.IsNull())
                colName.assign(COL_DEFAULTNAME_SourceId);
            else
                colName.assign(sourceIdColName.Value());

            break;
            }
            case ECRelationshipEnd_Target:
            {
            Nullable<Utf8String> targetIdColName;
            if (ca.IsValid())
                {
                if (SUCCESS != ca.TryGetTargetECInstanceIdColumn(targetIdColName))
                    return Utf8String();
                }

            if (targetIdColName.IsNull())
                colName.assign(COL_DEFAULTNAME_TargetId);
            else
                colName.assign(targetIdColName.Value());

            break;
            }

            default:
                BeAssert(false);
                break;
        }

    BeAssert(!colName.empty());
    return colName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
//static
Utf8String RelationshipClassLinkTableMap::DetermineConstraintECClassIdColumnName(LinkTableRelationshipMapCustomAttribute const& ca, ECN::ECRelationshipEnd end)
    {
    Utf8String colName;
    Nullable<Utf8String> idColName;
    switch (end)
        {
            case ECRelationshipEnd_Source:
            {
            if (ca.IsValid())
                {
                if (SUCCESS != ca.TryGetSourceECInstanceIdColumn(idColName))
                    return Utf8String();
                }

            if (idColName.IsNull())
                colName.assign(COL_SourceECClassId);

            break;
            }

            case ECRelationshipEnd_Target:
            {
            if (ca.IsValid())
                {
                if (SUCCESS != ca.TryGetTargetECInstanceIdColumn(idColName))
                    return Utf8String();
                }

            if (idColName.IsNull())
                colName.assign(COL_TargetECClassId);
            
            break;
            }

            default:
                BeAssert(false);
                break;
        }

    if (!idColName.IsNull())
        {
        if (!idColName.Value().EndsWithIAscii("id"))
            colName.assign(idColName.Value());
        else
            colName.assign(idColName.Value().substr(0, idColName.Value().size() - 2));

        colName.append("ClassId");
        }

    BeAssert(!colName.empty());
    return colName;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                10/2015
//+---------------+---------------+---------------+---------------+---------------+-
//static
bool RelationshipClassLinkTableMap::DetermineAllowDuplicateRelationshipsFlagFromRoot(ECRelationshipClassCR baseRelClass)
    {
    LinkTableRelationshipMapCustomAttribute linkRelMap;
    if (ECDbMapCustomAttributeHelper::TryGetLinkTableRelationshipMap(linkRelMap, baseRelClass))
        {
        //default for AllowDuplicateRelationships: false
        Nullable<bool> allowDuplicateRels;
        linkRelMap.TryGetAllowDuplicateRelationships(allowDuplicateRels);
        if (GetAllowDuplicateRelationshipsFlag(allowDuplicateRels))
            return true;
        }

    if (!baseRelClass.HasBaseClasses())
        return false;

    BeAssert(baseRelClass.GetBaseClasses()[0]->GetRelationshipClassCP() != nullptr);
    return DetermineAllowDuplicateRelationshipsFlagFromRoot(*baseRelClass.GetBaseClasses()[0]->GetRelationshipClassCP());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                   01/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RelationshipClassLinkTableMap::_Load(ClassMapLoadContext& ctx, DbClassMapLoadContext const& mapInfo)
    {
    if (ClassMap::_Load(ctx, mapInfo) != SUCCESS)
        return ERROR;

    std::vector<DbColumn const*> const* mapColumns = mapInfo.FindColumnByAccessString(Utf8String(ECDBSYS_PROP_SourceECInstanceId));
    if (mapColumns == nullptr)
        {
        BeAssert(false && "Failed to deserialize property map");
        return ERROR;
        }


    RefCountedPtr<ConstraintECInstanceIdPropertyMap> sourceECInstanceIdPropMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Source, *mapColumns);
    if (sourceECInstanceIdPropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (GetPropertyMapsR().Insert(sourceECInstanceIdPropMap) != SUCCESS)
        return ERROR;

    m_sourceConstraintMap.SetECInstanceIdPropMap(sourceECInstanceIdPropMap.get());

    mapColumns = mapInfo.FindColumnByAccessString(Utf8String(ECDBSYS_PROP_SourceECClassId));
    if (mapColumns == nullptr)
        {
        BeAssert(false && "Failed to deserialize property map");
        return ERROR;
        }


    RefCountedPtr<ConstraintECClassIdPropertyMap> sourceECClassIdPropMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Source, *mapColumns);
    if (sourceECClassIdPropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (GetPropertyMapsR().Insert(sourceECClassIdPropMap) != SUCCESS)
        return ERROR;

    m_sourceConstraintMap.SetECClassIdPropMap(sourceECClassIdPropMap.get());

    mapColumns = mapInfo.FindColumnByAccessString(Utf8String(ECDBSYS_PROP_TargetECInstanceId));
    if (mapColumns == nullptr)
        {
        BeAssert(false && "Failed to deserialize property map");
        return ERROR;
        }

    RefCountedPtr<ConstraintECInstanceIdPropertyMap>  targetECInstanceIdPropMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Target, *mapColumns);
    if (targetECInstanceIdPropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (GetPropertyMapsR().Insert(targetECInstanceIdPropMap) != SUCCESS)
        return ERROR;

    m_targetConstraintMap.SetECInstanceIdPropMap(targetECInstanceIdPropMap.get());

    mapColumns = mapInfo.FindColumnByAccessString(Utf8String(ECDBSYS_PROP_TargetECClassId));
    if (mapColumns == nullptr)
        {
        BeAssert(false && "Failed to deserialize property map");
        return ERROR;
        }

    RefCountedPtr<ConstraintECClassIdPropertyMap> targetECClassIdPropMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Target, *mapColumns);
    if (targetECClassIdPropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }    
    
    if (GetPropertyMapsR().Insert(targetECClassIdPropMap) != SUCCESS)
        return ERROR;

    m_targetConstraintMap.SetECClassIdPropMap(targetECClassIdPropMap.get());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    11/2013
//---------------------------------------------------------------------------------------
DbColumn* RelationshipClassLinkTableMap::CreateConstraintColumn(Utf8CP columnName, PersistenceType persType)
    {
    DbTable& table = GetPrimaryTable();
    const bool wasEditMode = table.GetEditHandle().CanEdit();
    if (!wasEditMode)
        table.GetEditHandleR().BeginEdit();

    DbColumn* column = table.FindColumnP(columnName);
    if (column != nullptr)
        return column;

    persType = table.GetType() != DbTable::Type::Existing ? persType : PersistenceType::Virtual;
    //Following protect creating virtual id/fk columns in persisted tables.
    if (table.GetType() != DbTable::Type::Virtual && persType == PersistenceType::Virtual)
        {
        LOG.errorv("Failed to map ECRelationshipClass '%s': No columns found for " ECDBSYS_PROP_SourceECInstanceId " or " ECDBSYS_PROP_TargetECInstanceId " in table '%s'. Consider applying the LinkTableRelationshipMap custom attribute to the ECRelationshipClass.",
                   GetClass().GetFullName(), table.GetName().c_str());
        return nullptr;
        }
        
    column = table.AddColumn(Utf8String(columnName), DbColumn::Type::Integer, DbColumn::Kind::Default, persType);

    if (!wasEditMode)
        table.GetEditHandleR().EndEdit();

    return column;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    01/2014
//---------------------------------------------------------------------------------------
void RelationshipClassLinkTableMap::DetermineConstraintClassIdColumnHandling(bool& addConstraintClassIdColumnNeeded, ECRelationshipConstraintCR constraint) const
    {
    //A constraint class id column is needed if 
    // * the map strategy implies that multiple classes are stored in the same table or
    // * it has more than one classes including subclasses in case of a polymorphic constraint. 
    //So we first determine whether a constraint class id column is needed
    ECRelationshipConstraintClassList const& constraintClasses = constraint.GetConstraintClasses();
    addConstraintClassIdColumnNeeded = constraintClasses.size() > 1 ;
    //if constraint is polymorphic, and if addConstraintClassIdColumnNeeded is not true yet,
    //we also need to check if the constraint classes have subclasses. If there is at least one, addConstraintClassIdColumnNeeded
    //is set to true;
    if (!addConstraintClassIdColumnNeeded && constraint.GetIsPolymorphic())
        addConstraintClassIdColumnNeeded = true;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
