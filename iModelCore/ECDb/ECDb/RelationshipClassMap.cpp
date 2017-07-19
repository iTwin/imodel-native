/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/RelationshipClassMap.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <array>
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//************************ RelationshipClassMap **********************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
RelationshipClassMap::RelationshipClassMap(ECDb const& ecdb, Type type, ECN::ECClassCR ecRelClass, MapStrategyExtendedInfo const& mapStrategy)
    : ClassMap(ecdb, type, ecRelClass, mapStrategy), m_sourceConstraintMap( ecRelClass.GetRelationshipClassCP()->GetSource()), m_targetConstraintMap( ecRelClass.GetRelationshipClassCP()->GetTarget())
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
    else
        return GetTargetECInstanceIdPropMap();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    11/2013
//---------------------------------------------------------------------------------------
ConstraintECClassIdPropertyMap const* RelationshipClassMap::GetConstraintECClassIdPropMap(ECRelationshipEnd constraintEnd) const
    {
    if (constraintEnd == ECRelationshipEnd_Source)
        return GetSourceECClassIdPropMap();
    else
        return GetTargetECClassIdPropMap();
    }

//************************ RelationshipClassEndTableMap::Partition**********************************
//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         06/17
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t RelationshipClassEndTableMap::Partition::QuickHash64(Utf8CP str, uint64_t mix)
    {
    const uint64_t mulp = 2654435789;
    mix ^= 104395301;
    while (*str)
        mix += (*str++ * mulp) ^ (mix >> 23);

    return mix ^ (mix << 37);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         06/17
//+---------------+---------------+---------------+---------------+---------------+------
RelationshipClassEndTableMap::Partition::Partition(DbColumn const& ecInstanceId, DbColumn const& ecClassId, DbColumn const& sourceId, DbColumn const* sourceClassId, DbColumn const& targetId, DbColumn const* targetClassId)
    :m_ecInstanceId(ecInstanceId), m_ecClassId(ecClassId), m_sourceId(sourceId), m_sourceClassId(sourceClassId), m_targetId(targetId), m_targetClassId(targetClassId), m_hashCode(0)
    {
    m_hashCode = QuickHash64(ecInstanceId.GetName().c_str(), m_hashCode);
    m_hashCode = QuickHash64(ecClassId.GetName().c_str(), m_hashCode);
    m_hashCode = QuickHash64(sourceId.GetName().c_str(), m_hashCode);
    if (sourceClassId)
        m_hashCode = QuickHash64(sourceClassId->GetName().c_str(), m_hashCode);
    m_hashCode = QuickHash64(targetId.GetName().c_str(), m_hashCode);
    if (m_targetClassId)
        m_hashCode = QuickHash64(targetClassId->GetName().c_str(), m_hashCode);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         06/17
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<RelationshipClassEndTableMap::Partition> RelationshipClassEndTableMap::Partition::Create(DbColumn const& ecInstanceId, DbColumn const& ecClassId, DbColumn const& sourceId, DbColumn const* sourceClassId, DbColumn const& targetId, DbColumn const* targetClassId, ECN::ECRelationshipEnd fromEnd)
    {
    return std::unique_ptr<Partition>(new Partition(ecInstanceId, ecClassId, sourceId, sourceClassId, targetId, targetClassId));
    }

//************************ RelationshipClassEndTableMap::PartitionView**********************************
//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         06/17
//+---------------+---------------+---------------+---------------+---------------+------
std::vector<RelationshipClassEndTableMap::Partition const*>  RelationshipClassEndTableMap::PartitionView::GetPhysicalPartitions() const
    {
    std::vector<Partition const*> physcialPartitions;
    for (DbTable const* table : GetTables(true))
        {
        for (auto const* part : GetPartitions(*table, true))
            if (part->CanQuery())
                physcialPartitions.push_back(part);
        }

    return physcialPartitions;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         06/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
std::vector<DbTable const*> RelationshipClassEndTableMap::PartitionView::GetOtherEndTables(RelationshipClassEndTableMap const& relationMap)
    {
    ECRelationshipEnd otherEnd = relationMap.GetMapStrategy().GetStrategy() == MapStrategy::ForeignKeyRelationshipInSourceTable ? ECRelationshipEnd::ECRelationshipEnd_Target : ECRelationshipEnd::ECRelationshipEnd_Source;
    ECRelationshipConstraintCR otherEndConstraint = ECRelationshipEnd::ECRelationshipEnd_Source ? relationMap.GetRelationshipClass().GetSource() : relationMap.GetRelationshipClass().GetTarget();
    ECDbCR ecdb = relationMap.GetECDb();
    DbSchema const& dbSchema = ecdb.Schemas().GetDbMap().GetDbSchema();
    std::vector<DbTable const*> list;
    std::vector<DbTable const*> nvlist;
    if (otherEndConstraint.GetIsPolymorphic())
        {
        CachedStatementPtr stmt = ecdb.GetCachedStatement(
            "SELECT DISTINCT [CHT].[TableId] "
            "FROM   [ec_RelationshipConstraintClass] [RCC] "
            "       INNER JOIN [ec_RelationshipConstraint] [RC] ON [RC].[Id] = [RCC].[ConstraintId] "
            "       INNER JOIN [ec_Table] [T] ON [T].[Id] = [CHT].[TableId] "
            "       INNER JOIN [ec_cache_ClassHierarchy] [CH] ON [CH].[BaseClassId] = [RCC].[ClassId] "
            "       INNER JOIN [ec_cache_ClassHasTables] [CHT] ON [CHT].[ClassId] = [CH].[ClassId] "
            "WHERE  [RC].[RelationshipClassId] = ? "
            "       AND [RC].[RelationshipEnd] = ? AND [T].[Type] != " SQLVAL_DbTable_Type_Joined " AND [T].[Type] != " SQLVAL_DbTable_Type_Overflow);
            
        PRECONDITION(stmt != nullptr, list);
        stmt->BindId(1, relationMap.GetClass().GetId());
        stmt->BindInt(2, Enum::ToInt(otherEnd));
        while (stmt->Step() == BE_SQLITE_ROW)
            {
            if (DbTable const* table = dbSchema.FindTable(stmt->GetValueId<DbTableId>(0)))
                {
                list.push_back(table);
                if (table->GetType() != DbTable::Type::Virtual)
                    nvlist.push_back(table);
                }
            else
                {
                list.clear();
                PRECONDITION(false, list);
                }
            }
        }
    else
        {
        CachedStatementPtr stmt = ecdb.GetCachedStatement(
            "SELECT DISTINCT [CHT].[TableId] "
            "FROM   [ec_RelationshipConstraintClass] [RCC] "
            "       INNER JOIN [ec_RelationshipConstraint] [RC] ON [RC].[Id] = [RCC].[ConstraintId] "
            "       INNER JOIN [ec_Table] [T] ON [T].[Id] = [CHT].[TableId] "
            "       INNER JOIN [ec_cache_ClassHasTables] [CHT] ON [CHT].[ClassId] = [RCC].[ClassId] "
            "WHERE  [RC].[RelationshipClassId] = ? "
            "       AND [RC].[RelationshipEnd] = ? AND [T].[Type] != " SQLVAL_DbTable_Type_Joined " AND [T].[Type] != " SQLVAL_DbTable_Type_Overflow);

        PRECONDITION(stmt != nullptr, list);
        stmt->BindId(1, relationMap.GetClass().GetId());
        stmt->BindInt(2, Enum::ToInt(otherEnd));
        while (stmt->Step() == BE_SQLITE_ROW)
            {
            if (DbTable const* table = dbSchema.FindTable(stmt->GetValueId<DbTableId>(0)))
                {
                list.push_back(table);
                if (table->GetType() != DbTable::Type::Virtual)
                    nvlist.push_back(table);
                }
            else
                {
                list.clear();
                PRECONDITION(false, list);
                }
            }
        }

    if (!nvlist.empty())
        return nvlist;

    return list;
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         06/17
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipClassEndTableMap::PartitionView::InsertPartition(std::unique_ptr<Partition> partition, bool assertAndFailOnDuplicatePartition)
    {
    PRECONDITION(partition != nullptr, ERROR);
    const auto itor = m_partitionMap[partition->GetTable().GetId()].insert(std::move(partition));
    const bool isDuplicatePartition = !itor.second;
    if (isDuplicatePartition && assertAndFailOnDuplicatePartition)
        {
        POSTCONDITION(isDuplicatePartition == false, ERROR);
        }

    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         06/17
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipClassEndTableMap::PartitionView::ResurrectPartition(std::vector<DbTable const*> const& tables, DbColumn const& navId, DbColumn const& navRelECClassId)
    {
    PRECONDITION(navId.GetTable().GetId() == navRelECClassId.GetTable().GetId(), ERROR);
    PRECONDITION(tables.size() <= 1, ERROR);

    DbTable const& table = navId.GetTable();
    DbColumn const* fromEndClassId = tables.empty() ? nullptr : tables.front()->FindFirst(DbColumn::Kind::ECClassId);
    DbColumn const* toEndClassId = navId.GetTable().FindFirst(DbColumn::Kind::ECClassId);
    DbColumn const* ecInstnaceId = table.FindFirst(DbColumn::Kind::ECInstanceId);
    DbColumn const* ecClassId = &navRelECClassId;
    DbColumn const* sourceECInstanceId = GetFromEnd() == ECRelationshipEnd_Source ? &navId : ecInstnaceId;
    DbColumn const* sourceECClassId = GetFromEnd() == ECRelationshipEnd_Source ? fromEndClassId : toEndClassId;
    DbColumn const* targetECInstanceId = GetFromEnd() == ECRelationshipEnd_Target ? &navId : ecInstnaceId;
    DbColumn const* targetECClassId = GetFromEnd() == ECRelationshipEnd_Target ? fromEndClassId : toEndClassId;;

    PRECONDITION(ecInstnaceId != nullptr, ERROR);
    PRECONDITION(ecClassId != nullptr, ERROR);
    PRECONDITION(sourceECInstanceId != nullptr, ERROR);
    PRECONDITION(targetECInstanceId != nullptr, ERROR);

    return InsertPartition(Partition::Create(*ecInstnaceId, *ecClassId, *sourceECInstanceId, sourceECClassId, *targetECInstanceId, targetECClassId, GetFromEnd()), true);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         06/17
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipClassEndTableMap::PartitionView::AddDefaultPartition()
    {
    DbTable const* table = m_relationshipMap.GetTables().front();
    return InsertPartition(
        Partition::Create(
            m_relationshipMap.GetECInstanceIdPropertyMap()->FindDataPropertyMap(*table)->GetColumn(),
            m_relationshipMap.GetECClassIdPropertyMap()->FindDataPropertyMap(*table)->GetColumn(),
            m_relationshipMap.GetSourceECInstanceIdPropMap()->FindDataPropertyMap(*table)->GetColumn(),
            &m_relationshipMap.GetSourceECClassIdPropMap()->FindDataPropertyMap(*table)->GetColumn(),
            m_relationshipMap.GetTargetECInstanceIdPropMap()->FindDataPropertyMap(*table)->GetColumn(),
            &m_relationshipMap.GetTargetECClassIdPropMap()->FindDataPropertyMap(*table)->GetColumn(),
            GetFromEnd()), true);
    }
//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         06/17
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipClassEndTableMap::PartitionView::Load()
    {
    if (m_loadedPartitions)
        return SUCCESS;

    m_partitionMap.clear();
    if (AddDefaultPartition() != SUCCESS)
        return ERROR;

    enum class PropertyMapKind
        {
        Id = 1,
        RelEClassId = 2,
        Unknown = 3,
        };

    Utf8CP sql =
        "SELECT DISTINCT "
        "       CASE (SUBSTR ([PP].[AccessString], INSTR([PP].[AccessString], '.') + 1)) WHEN 'Id'  THEN 1 WHEN 'RelECClassId'THEN 2 ELSE 3 END PropertyMapKind, "
        "       [T].[Id] [TableId], "
        "       [C].[Name] [Column] "
        "FROM   [ec_Property] [P] "
        "       INNER JOIN [ec_PropertyPath] [PP] ON [PP].[RootPropertyId] = [P].[Id] "
        "       INNER JOIN [ec_PropertyMap] [PM] ON [PM].[PropertyPathId] = [PP].[Id] "
        "       INNER JOIN [ec_Column] [C] ON [C].[Id] = [PM].[ColumnId] "
        "       INNER JOIN [ec_Table] [T] ON [T].[Id] = [C].[TableId] "
        "WHERE  [P].[NavigationRelationshipClassId] = ? ORDER BY [T].[Id], [PM].[ClassId], [PropertyMapKind]; ";

    ECDbCR ecdb = m_relationshipMap.GetECDb();
    DbSchema const& dbSchema = ecdb.Schemas().GetDbMap().GetDbSchema();
    const std::vector<DbTable const*> primaryTables = GetOtherEndTables(m_relationshipMap);
    CachedStatementPtr stmt = ecdb.GetCachedStatement(sql);
    PRECONDITION(stmt != nullptr, ERROR);

    stmt->BindId(1, m_relationshipMap.GetClass().GetId());
    //We expect multiple of two rows
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        //1. Expecting multiple of two rows
        //2. Expecting ProperytMapKind 1 to be first
        //3. Expecting PropertyMapKind 2 to be second
        PropertyMapKind idPropertyMapKind = Enum::FromInt<PropertyMapKind>(stmt->GetValueInt(0));
        DbTableId idTableId = stmt->GetValueId<DbTableId>(1);
        if (idPropertyMapKind != PropertyMapKind::Id)
            return ERROR;

        DbTable const* table = dbSchema.FindTable(idTableId);
        if (table == nullptr)
            return ERROR;

        DbColumn const* refId = table->FindColumn(stmt->GetValueText(2));
        if (refId == nullptr)
            return ERROR;

        if (stmt->Step() != BE_SQLITE_ROW)
            return ERROR;

        PropertyMapKind relECClassIdPropertyMapKind = Enum::FromInt<PropertyMapKind>(stmt->GetValueInt(0));
        DbTableId relECClassIdTableId = stmt->GetValueId<DbTableId>(1);
        if (relECClassIdPropertyMapKind != PropertyMapKind::RelEClassId)
            return ERROR;

        if (idTableId != relECClassIdTableId)
            return ERROR;

        DbColumn const* refRelECClassId = table->FindColumn(stmt->GetValueText(2));
        if (refRelECClassId == nullptr)
            return ERROR;

        if (ResurrectPartition(primaryTables, *refId, *refRelECClassId) != SUCCESS)
            return ERROR;
        }

    m_loadedPartitions = true;
    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         06/17
//+---------------+---------------+---------------+---------------+---------------+------
ECN::ECRelationshipEnd RelationshipClassEndTableMap::PartitionView::GetToEnd() const { return m_relationshipMap.GetMapStrategy().GetStrategy() == MapStrategy::ForeignKeyRelationshipInSourceTable ? ECRelationshipEnd_Source : ECRelationshipEnd_Target; }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         06/17
//+---------------+---------------+---------------+---------------+---------------+------
ECN::ECRelationshipEnd RelationshipClassEndTableMap::PartitionView::GetFromEnd() const { return GetToEnd() == ECRelationshipEnd_Source ? ECRelationshipEnd_Target : ECRelationshipEnd_Source; }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         06/17
//+---------------+---------------+---------------+---------------+---------------+------
RelationshipClassEndTableMap::PartitionView::PartitionView(RelationshipClassEndTableMap const& relationshipMap)
    :m_relationshipMap(relationshipMap), m_loadedPartitions(false)
    {}

//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         06/17
//+---------------+---------------+---------------+---------------+---------------+------
const std::map <DbTable const*, std::vector<RelationshipClassEndTableMap::Partition const*>> RelationshipClassEndTableMap::PartitionView::GetPartitionMap() const
    {
    std::map < DbTable const*, std::vector<Partition const*>> map;
    for (auto const& partition : m_partitionMap)
        for (auto const& entry : partition.second)
            map[&entry->GetTable()].push_back(entry.get());

    return map;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         06/17
//+---------------+---------------+---------------+---------------+---------------+------
const std::vector <DbTable const*> RelationshipClassEndTableMap::PartitionView::GetTables(bool skipVirtualPartition) const
    {
    std::vector <DbTable const*> list;
    for (auto const& partition : GetPartitionMap())
        {
        if (partition.first->GetType() == DbTable::Type::Virtual && skipVirtualPartition)
            continue;

        list.push_back(partition.first);
        }

    return list;
    }
//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         06/17
//+---------------+---------------+---------------+---------------+---------------+------
const std::vector<RelationshipClassEndTableMap::Partition const*> RelationshipClassEndTableMap::PartitionView::GetPartitions(bool skipVirtualPartition) const
    {
    std::vector<Partition const*> list;
    for (auto const& partition : m_partitionMap)
        for (auto const& entry : partition.second)
            {
            if (skipVirtualPartition && entry->GetTable().GetType() == DbTable::Type::Virtual)
                continue;

            list.push_back(entry.get());
            }

    return list;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         06/17
//+---------------+---------------+---------------+---------------+---------------+------
const std::vector<RelationshipClassEndTableMap::Partition const*> RelationshipClassEndTableMap::PartitionView::GetPartitions(DbTable const& toEnd, bool skipVirtualPartition) const
    {
    std::vector<Partition const*> list;
    auto itor = m_partitionMap.find(toEnd.GetId());
    if (itor == m_partitionMap.end())
        {
        BeAssert(false && "Programmer Error");
        }
    else
        {
        for (auto const& entry : itor->second)
            {
            if (skipVirtualPartition && entry->GetTable().GetType() == DbTable::Type::Virtual)
                continue;

            list.push_back(entry.get());
            }
        }

    return list;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         06/17
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr< RelationshipClassEndTableMap::PartitionView> RelationshipClassEndTableMap::PartitionView::Create(RelationshipClassEndTableMap const& relationMap)
    {
    std::unique_ptr< PartitionView> col = std::unique_ptr< PartitionView>(new PartitionView(relationMap));
    POSTCONDITION(col->Load() == SUCCESS, nullptr);
    return col;
    }


//************************ RelationshipClassEndTableMap **********************************
//---------------------------------------------------------------------------------------
// @bsimethod                                               Affan.Khan       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
RelationshipClassEndTableMap const* RelationshipClassEndTableMap::GetBaseClassMap(SchemaImportContext * ctx) const
    {
    if (!GetClass().HasBaseClasses())
        return nullptr;

    PRECONDITION(GetClass().GetBaseClasses().size() == 1, nullptr);
    ECRelationshipClassCP relationshipClass = static_cast<ECRelationshipClassCP>(GetClass().GetBaseClasses().front());
    if (ClassMapCP classMap = GetDbMap().GetClassMap(*relationshipClass))
        return  &classMap->GetAs<RelationshipClassEndTableMap>();
    else
        {
        PRECONDITION(ctx != nullptr, nullptr);
        if (GetDbMap().MapRelationshipClass(*ctx, *relationshipClass) == ClassMappingStatus::Success)
            return &GetDbMap().GetClassMap(*relationshipClass)->GetAs<RelationshipClassEndTableMap>();
        }

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
// @bsimethod                                               Krischan.Eberle       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus RelationshipClassEndTableMap::_Map(ClassMappingContext& ctx)
    {
    if (RelationshipClassEndTableMap const* baseClassMap = GetBaseClassMap(&ctx.GetImportCtx()))
        return MapSubClass(*baseClassMap);

    //End table relationship is always mapped to virtual table.
    //we use nav properties to generate views dynamically
    if (SUCCESS != DbMappingManager::Tables::MapToTable(ctx.GetImportCtx(), *this, ctx.GetClassMappingInfo()))
        return ClassMappingStatus::Error;

    if (SUCCESS != MapSystemColumns())
        return ClassMappingStatus::Error;

    
    DbTable* vtable = GetTables().front();
    PRECONDITION(vtable != nullptr && vtable->GetType() == DbTable::Type::Virtual, ClassMappingStatus::Error);
    {////////SourceECInstanceId
    DbColumn* sourceECInstanceId = vtable->CreateColumn(ECDBSYS_PROP_SourceECInstanceId, DbColumn::Type::Integer, DbColumn::Kind::Default, PersistenceType::Virtual);
    RefCountedPtr<ConstraintECInstanceIdPropertyMap> propMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Source, {sourceECInstanceId});
    if (propMap == nullptr || sourceECInstanceId == nullptr)
        {
        BeAssert(false && "Failed to create PropertyMap SourceECInstanceId");
        return ClassMappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(propMap, 2) != SUCCESS)
        return ClassMappingStatus::Error;

    GetConstraintMapR(ECRelationshipEnd_Source).SetECInstanceIdPropMap(propMap.get());
    }/////////////////////////

    {////////SourceECClassId
    DbColumn* sourceECClassId = vtable->CreateColumn(ECDBSYS_PROP_SourceECClassId, DbColumn::Type::Integer, DbColumn::Kind::Default, PersistenceType::Virtual);
    RefCountedPtr<ConstraintECClassIdPropertyMap> propMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Source, {sourceECClassId});
    if (propMap == nullptr || sourceECClassId == nullptr)
        {
        BeAssert(false && "Failed to create PropertyMap SourceECClassId");
        return ClassMappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(propMap, 3) != SUCCESS)
        return ClassMappingStatus::Error;

    GetConstraintMapR(ECRelationshipEnd_Source).SetECClassIdPropMap(propMap.get());
    }/////////////////////////

    {////////TargetECInstanceId
    DbColumn* targetECInstanceId = vtable->CreateColumn(ECDBSYS_PROP_TargetECInstanceId, DbColumn::Type::Integer, DbColumn::Kind::Default, PersistenceType::Virtual);
    RefCountedPtr<ConstraintECInstanceIdPropertyMap> propMap = ConstraintECInstanceIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Target, {targetECInstanceId});
    if (propMap == nullptr || targetECInstanceId == nullptr)
        {
        BeAssert(false && "Failed to create PropertyMap TargetECInstanceId");
        return ClassMappingStatus::Error;
        }

    if (GetPropertyMapsR().Insert(propMap, 4) != SUCCESS)
        return ClassMappingStatus::Error;

    GetConstraintMapR(ECRelationshipEnd_Target).SetECInstanceIdPropMap(propMap.get());
    }/////////////////////////

    {////////TargetECClassId
    DbColumn* targetECClassId = vtable->CreateColumn(ECDBSYS_PROP_TargetECClassId, DbColumn::Type::Integer, DbColumn::Kind::Default, PersistenceType::Virtual);
    RefCountedPtr<ConstraintECClassIdPropertyMap> propMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Target, {targetECClassId});
    if (propMap == nullptr || targetECClassId == nullptr)
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
// @bsimethod                                               Affan.Khan           01/2015
//+---------------+---------------+---------------+---------------+---------------+------

RelationshipClassEndTableMap::PartitionView const& RelationshipClassEndTableMap::GetPartitionView() const
    {
    if (GetClass().HasBaseClasses())
        return GetBaseClassMap()->GetPartitionView();

    if (m_partitionCollection == nullptr)
        m_partitionCollection = PartitionView::Create(*this);

    return *m_partitionCollection;
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


//************************** RelationshipClassLinkTableMap *****************************************
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipClassLinkTableMap::RelationshipClassLinkTableMap(ECDb const& ecdb, ECN::ECClassCR ecRelClass, MapStrategyExtendedInfo const& mapStrategy)
    : RelationshipClassMap(ecdb, Type::RelationshipLinkTable, ecRelClass, mapStrategy)
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

    BeAssert(dynamic_cast<RelationshipMappingInfo const*> (&ctx.GetClassMappingInfo()) != nullptr);
    RelationshipMappingInfo const& relationClassMapInfo = static_cast<RelationshipMappingInfo const&> (ctx.GetClassMappingInfo());

    if (GetRelationshipClass().HasBaseClasses())
        return MapSubClass(ctx, relationClassMapInfo);

    ECRelationshipClassCR relationshipClass = GetRelationshipClass();
    ECRelationshipConstraintCR sourceConstraint = relationshipClass.GetSource();
    ECRelationshipConstraintCR targetConstraint = relationshipClass.GetTarget();

    //**** Constraint columns and prop maps
    bool addSourceECClassIdColumnToTable = false;
    DetermineConstraintClassIdColumnHandling(addSourceECClassIdColumnToTable, sourceConstraint);

    bool addTargetECClassIdColumnToTable = false;
    DetermineConstraintClassIdColumnHandling(addTargetECClassIdColumnToTable, targetConstraint);
    stat = CreateConstraintPropMaps(ctx, relationClassMapInfo, addSourceECClassIdColumnToTable, addTargetECClassIdColumnToTable);
    if (stat != ClassMappingStatus::Success)
        return stat;

    stat = DoMapPart2(ctx);
    if (stat != ClassMappingStatus::Success)
        return stat;

    LinkTableMappingType const& linkTableMappingType = relationClassMapInfo.GetMappingType().GetAs<LinkTableMappingType>();

    //only create constraints on TPH root or if not TPH and not existing table
    if (GetPrimaryTable().GetType() != DbTable::Type::Existing && linkTableMappingType.GetCreateForeignKeyConstraintsFlag() &&
        (!GetMapStrategy().IsTablePerHierarchy() || GetTphHelper()->DetermineTphRootClassId() == GetClass().GetId())) 
        {
        //Create FK from Source-Primary to LinkTable
        DbTable const* sourceTable = *relationClassMapInfo.GetSourceTables().begin();
        DbColumn const* fkColumn = &GetSourceECInstanceIdPropMap()->FindDataPropertyMap(GetPrimaryTable())->GetColumn();
        DbColumn const* referencedColumn = sourceTable->FindFirst(DbColumn::Kind::ECInstanceId);
        GetPrimaryTable().CreateForeignKeyConstraint(*fkColumn, *referencedColumn, ForeignKeyDbConstraint::ActionType::Cascade, ForeignKeyDbConstraint::ActionType::NotSpecified);

        //Create FK from Target-Primary to LinkTable
        DbTable const* targetTable = *relationClassMapInfo.GetTargetTables().begin();
        fkColumn = &GetTargetECInstanceIdPropMap()->FindDataPropertyMap(GetPrimaryTable())->GetColumn();
        referencedColumn = targetTable->FindFirst(DbColumn::Kind::ECInstanceId);
        GetPrimaryTable().CreateForeignKeyConstraint(*fkColumn, *referencedColumn, ForeignKeyDbConstraint::ActionType::Cascade, ForeignKeyDbConstraint::ActionType::NotSpecified);
        }


    AddIndices(ctx, linkTableMappingType.AllowDuplicateRelationships());
    return ClassMappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus RelationshipClassLinkTableMap::MapSubClass(ClassMappingContext& ctx, RelationshipMappingInfo const& mappingInfo)
    {
    if (GetClass().GetBaseClasses().size() != 1)
        {
        BeAssert(false && "Multi-inheritance of ECRelationshipclasses should have been caught before already");
        return ClassMappingStatus::Error;
        }

    ECClassCP baseClass = GetClass().GetBaseClasses()[0];
    ClassMap const* baseClassMap = GetDbMap().GetClassMap(*baseClass);
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

    AddIndices(ctx, DetermineAllowDuplicateRelationshipsFlagFromRoot(*baseClass->GetRelationshipClassCP()));
    return ClassMappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Affan.Khan                         04 / 15
//---------------------------------------------------------------------------------------
DbColumn* RelationshipClassLinkTableMap::ConfigureForeignECClassIdKey(ClassMappingContext& ctx, RelationshipMappingInfo const& mapInfo, ECRelationshipEnd relationshipEnd)
    {
    DbColumn* endECClassIdColumn = nullptr;
    ECRelationshipClassCP relationship = mapInfo.GetClass().GetRelationshipClassCP();
    BeAssert(relationship != nullptr);
    ECRelationshipConstraintCR foreignEndConstraint = relationshipEnd == ECRelationshipEnd_Source ? relationship->GetSource() : relationship->GetTarget();
    ECClass const* foreignEndClass = foreignEndConstraint.GetConstraintClasses()[0];
    ClassMap const* foreignEndClassMap = GetDbMap().GetClassMap(*foreignEndClass);
    size_t foreignEndTableCount = GetDbMap().GetTableCountOnRelationshipEnd(ctx.GetImportCtx(), foreignEndConstraint);

    Utf8String columnName = DetermineConstraintECClassIdColumnName(mapInfo.GetMappingType().GetAs<LinkTableMappingType>(), relationshipEnd);
    if (GetPrimaryTable().FindColumn(columnName.c_str()) != nullptr &&
        GetMapStrategy().GetStrategy() != MapStrategy::TablePerHierarchy && GetMapStrategy().GetStrategy() != MapStrategy::ExistingTable)
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
ClassMappingStatus RelationshipClassLinkTableMap::CreateConstraintPropMaps(ClassMappingContext& ctx, RelationshipMappingInfo const& mapInfo, bool addSourceECClassIdColumnToTable,
    bool addTargetECClassIdColumnToTable)
    {
    //**** SourceECInstanceId prop map 
    Utf8String columnName = DetermineConstraintECInstanceIdColumnName(mapInfo.GetMappingType().GetAs<LinkTableMappingType>(), ECRelationshipEnd_Source);
    if (columnName.empty() || GetPrimaryTable().FindColumn(columnName.c_str()) != nullptr && GetMapStrategy().GetStrategy() != MapStrategy::TablePerHierarchy && GetMapStrategy().GetStrategy() != MapStrategy::ExistingTable)
        {
        LOG.errorv("Failed to map ECRelationshipClass '%s': Table '%s' already contains " ECDBSYS_PROP_SourceECInstanceId " column named '%s'.",
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
    DbColumn const* sourceECClassIdColumn = ConfigureForeignECClassIdKey(ctx, mapInfo, ECRelationshipEnd_Source);
    auto sourceECClassIdPropMap = ConstraintECClassIdPropertyMap::CreateInstance(*this, ECRelationshipEnd_Source, {sourceECClassIdColumn} );
    PRECONDITION(sourceECClassIdPropMap.IsValid(), ClassMappingStatus::Error);
    if (GetPropertyMapsR().Insert(sourceECClassIdPropMap) != SUCCESS)
        return ClassMappingStatus::Error;

    m_sourceConstraintMap.SetECClassIdPropMap(sourceECClassIdPropMap.get());


    //**** TargetECInstanceId prop map 
    columnName = DetermineConstraintECInstanceIdColumnName(mapInfo.GetMappingType().GetAs<LinkTableMappingType>(), ECRelationshipEnd_Target);
    if (columnName.empty() || GetPrimaryTable().FindColumn(columnName.c_str()) != nullptr && GetMapStrategy().GetStrategy() != MapStrategy::TablePerHierarchy && GetMapStrategy().GetStrategy() != MapStrategy::ExistingTable)
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
    DbColumn const* targetECClassIdColumn = ConfigureForeignECClassIdKey(ctx, mapInfo, ECRelationshipEnd_Target);
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
void RelationshipClassLinkTableMap::AddIndices(ClassMappingContext& ctx, bool allowDuplicateRelationships)
    {
    if (GetPrimaryTable().GetType() == DbTable::Type::Existing)
        return;

    // Add indices on the source and target based on cardinality
    //(the many side can be unique, but the one side must never be unique)
    const bool sourceIsUnique = !allowDuplicateRelationships && (GetRelationshipClass().GetTarget().GetMultiplicity().GetUpperLimit() <= 1);
    const bool targetIsUnique = !allowDuplicateRelationships && (GetRelationshipClass().GetSource().GetMultiplicity().GetUpperLimit() <= 1);

    AddIndex(ctx.GetImportCtx(), RelationshipIndexSpec::Source, sourceIsUnique);
    AddIndex(ctx.GetImportCtx(), RelationshipIndexSpec::Target, targetIsUnique);

    if (!allowDuplicateRelationships)
        AddIndex(ctx.GetImportCtx(), RelationshipClassLinkTableMap::RelationshipIndexSpec::SourceAndTarget, true);
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
            default:
                BeAssert(false);
                break;
        }

    auto sourceECInstanceIdColumn = &GetSourceECInstanceIdPropMap()->FindDataPropertyMap(GetPrimaryTable())->GetColumn();
    auto sourceECClassIdColumn = GetSourceECClassIdPropMap()->FindDataPropertyMap(GetPrimaryTable()) != nullptr ? &GetSourceECClassIdPropMap()->FindDataPropertyMap(GetPrimaryTable())->GetColumn() : nullptr;
    auto targetECInstanceIdColumn = &GetTargetECInstanceIdPropMap()->FindDataPropertyMap(GetPrimaryTable())->GetColumn();
    auto targetECClassIdColumn = GetTargetECClassIdPropMap()->FindDataPropertyMap(GetPrimaryTable()) != nullptr ? &GetTargetECClassIdPropMap()->FindDataPropertyMap(GetPrimaryTable())->GetColumn() : nullptr;

    std::vector<DbColumn const*> columns;
    switch (spec)
        {
            case RelationshipIndexSpec::Source:
                GenerateIndexColumnList(columns, sourceECInstanceIdColumn, sourceECClassIdColumn, nullptr, nullptr);
                break;
            case RelationshipIndexSpec::Target:
                GenerateIndexColumnList(columns, targetECInstanceIdColumn, targetECClassIdColumn, nullptr, nullptr);
                break;

            case RelationshipIndexSpec::SourceAndTarget:
                GenerateIndexColumnList(columns, sourceECInstanceIdColumn, sourceECClassIdColumn, targetECInstanceIdColumn, targetECClassIdColumn);
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
void RelationshipClassLinkTableMap::GenerateIndexColumnList(std::vector<DbColumn const*>& columns, DbColumn const* col1, DbColumn const* col2, DbColumn const* col3, DbColumn const* col4)
    {
    if (nullptr != col1 && col1->GetPersistenceType() == PersistenceType::Physical)
        columns.push_back(col1);

    if (nullptr != col2 && col2->GetPersistenceType() == PersistenceType::Physical)
        columns.push_back(col2);

    if (nullptr != col3 && col3->GetPersistenceType() == PersistenceType::Physical)
        columns.push_back(col3);

    if (nullptr != col4 && col4->GetPersistenceType() == PersistenceType::Physical)
        columns.push_back(col4);
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
//static
Utf8String RelationshipClassLinkTableMap::DetermineConstraintECInstanceIdColumnName(LinkTableMappingType const& mappingType, ECN::ECRelationshipEnd end)
    {
    Utf8String colName;
    switch (end)
        {
            case ECRelationshipEnd_Source:
            {
            if (mappingType.GetSourceIdColumnName().IsNull())
                colName.assign(COL_DEFAULTNAME_SourceId);
            else
                colName.assign(mappingType.GetSourceIdColumnName().Value());

            break;
            }
            case ECRelationshipEnd_Target:
            {
            if (mappingType.GetTargetIdColumnName().IsNull())
                colName.assign(COL_DEFAULTNAME_TargetId);
            else
                colName.assign(mappingType.GetTargetIdColumnName().Value());

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
Utf8String RelationshipClassLinkTableMap::DetermineConstraintECClassIdColumnName(LinkTableMappingType const& mappingType, ECN::ECRelationshipEnd end)
    {
    Utf8String colName;
    Utf8StringCP idColName = nullptr;
    switch (end)
        {
            case ECRelationshipEnd_Source:
            {
            if (mappingType.GetSourceIdColumnName().IsNull())
                colName.assign(COL_SourceECClassId);
            else
                idColName = &mappingType.GetSourceIdColumnName().Value();
            
            break;
            }

            case ECRelationshipEnd_Target:
            {
            if (mappingType.GetTargetIdColumnName().IsNull())
                colName.assign(COL_TargetECClassId);
            else
                idColName = &mappingType.GetTargetIdColumnName().Value();
            
            break;
            }

            default:
                BeAssert(false);
                break;
        }

    if (idColName != nullptr)
        {
        if (!idColName->EndsWithIAscii("id"))
            colName.assign(*idColName);
        else
            colName.assign(idColName->substr(0, idColName->size() - 2));

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
        if (!allowDuplicateRels.IsNull() && allowDuplicateRels.Value())
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
        
    column = table.CreateColumn(Utf8String(columnName), DbColumn::Type::Integer, DbColumn::Kind::Default, persType);

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
