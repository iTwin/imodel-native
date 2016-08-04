/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/LightweightCache.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <vector>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//************************************************************************************
// LightweightCache
//************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
void LightweightCache::LoadClassIdsPerTable() const
    {
    if (m_loadedFlags.m_classIdsPerTableIsLoaded)
        return;

    Utf8String sql;
    sql.Sprintf("SELECT t.Id, t.Name, c.Id FROM ec_Table t, ec_Class c, ec_PropertyMap pm, ec_ClassMap cm, ec_PropertyPath pp, ec_Column col "
                "WHERE cm.Id=pm.ClassMapId AND pp.Id=pm.PropertyPathId AND col.Id=pm.ColumnId AND (col.ColumnKind & %d = 0) AND "
                "c.Id=cm.ClassId AND t.Id=col.TableId AND "
                "cm.MapStrategy<>%d AND cm.MapStrategy<>%d "
                "GROUP BY t.Id, c.Id", Enum::ToInt(DbColumn::Kind::ECClassId),
                Enum::ToInt(MapStrategy::ForeignKeyRelationshipInSourceTable),
                Enum::ToInt(MapStrategy::ForeignKeyRelationshipInTargetTable));

    CachedStatementPtr stmt = m_map.GetECDb().GetCachedStatement(sql.c_str());
    DbTableId currentTableId;
    DbTable const* currentTable = nullptr;
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        DbTableId tableId = stmt->GetValueId<DbTableId>(0);
        if (currentTableId != tableId)
            {
            Utf8CP tableName = stmt->GetValueText(1);
            currentTable = m_map.GetDbSchema().FindTable(tableName);
            currentTableId = tableId;
            BeAssert(currentTable != nullptr);
            }

        ECClassId id = stmt->GetValueId<ECClassId>(2);
        m_classIdsPerTable[currentTable].push_back(id);
        m_tablesPerClassId[id].insert(currentTable);
        }

    m_loadedFlags.m_classIdsPerTableIsLoaded = true;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
void LightweightCache::LoadRelationshipCache() const
    {
    if (m_loadedFlags.m_relationshipCacheIsLoaded)
        return;

    Utf8CP sql0 =
        "SELECT  IFNULL(CH.ClassId, RCC.[ClassId]) ConstraintClassId, RC.RelationshipClassId, RC.RelationshipEnd"
        "    FROM ec_RelationshipConstraintClass RCC"
        "       INNER JOIN ec_RelationshipConstraint RC ON RC.Id = RCC.ConstraintId"
        "       LEFT JOIN ec_ClassHierarchy CH ON CH.BaseClassId = RCC.[ClassId]  AND RC.IsPolymorphic = 1";

    auto stmt0 = m_map.GetECDb().GetCachedStatement(sql0);
    while (stmt0->Step() == BE_SQLITE_ROW)
        {
        ECClassId constraintClassId = stmt0->GetValueId<ECClassId>(0);
        ECClassId relationshipId = stmt0->GetValueId<ECClassId>(1);
        BeAssert(!stmt0->IsColumnNull(2));
        RelationshipEnd end = stmt0->GetValueInt(2) == 0 ? RelationshipEnd::Source : RelationshipEnd::Target;
        bmap<ECN::ECClassId, RelationshipEnd>& relClassIds = m_relationshipClassIdsPerConstraintClassIds[constraintClassId];
        auto relIt = relClassIds.find(relationshipId);
        if (relIt == relClassIds.end())
            relClassIds[relationshipId] = end;
        else
            relClassIds[relationshipId] = static_cast<RelationshipEnd>((int) (relIt->second) | (int) (end));

        bmap<ECN::ECClassId, RelationshipEnd>& constraintClassIds = m_constraintClassIdsPerRelClassIds[relationshipId];
        auto constraintIt = constraintClassIds.find(constraintClassId);
        if (constraintIt == constraintClassIds.end())
            constraintClassIds[constraintClassId] = end;
        else
            constraintClassIds[constraintClassId] = static_cast<RelationshipEnd>((int) (constraintIt->second) | (int) (end));
        }

    m_loadedFlags.m_relationshipCacheIsLoaded = true;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
void LightweightCache::LoadHorizontalPartitions()  const
    {
    if (m_loadedFlags.m_horizontalPartitionsIsLoaded)
        return;

    Utf8String sql;
    sql.Sprintf(
        "WITH "
        "TableMapInfo AS ( "
        "    SELECT ec_Class.Id ClassId, ec_Table.Name TableName FROM ec_PropertyMap "
        "        JOIN ec_Column ON ec_Column.Id = ec_PropertyMap.ColumnId AND (ec_Column.ColumnKind & %d= 0) "
        "        JOIN ec_PropertyPath ON ec_PropertyPath.Id = ec_PropertyMap.PropertyPathId "
        "        JOIN ec_ClassMap ON ec_ClassMap.Id = ec_PropertyMap.ClassMapId "
        "        JOIN ec_Class ON ec_Class.Id = ec_ClassMap.ClassId "
        "        JOIN ec_Table ON ec_Table.Id = ec_Column.TableId "
        "    WHERE ec_ClassMap.MapStrategy <> %d AND ec_ClassMap.MapStrategy <> %d AND ec_Table.Type <> %d "
        "    GROUP BY ec_Class.Id, ec_Table.Name) "
        "SELECT  DCL.BaseClassId, DCL.ClassId, TMI.TableName FROM ec_ClassHierarchy DCL "
        "    INNER JOIN TableMapInfo TMI ON TMI.ClassId=DCL.ClassId GROUP BY DCL.BaseClassId, TMI.TableName, DCL.ClassId",
        Enum::ToInt(DbColumn::Kind::ECClassId), Enum::ToInt(MapStrategy::ForeignKeyRelationshipInSourceTable),
        Enum::ToInt(MapStrategy::ForeignKeyRelationshipInTargetTable), Enum::ToInt(DbTable::Type::Joined));

    CachedStatementPtr stmt = m_map.GetECDb().GetCachedStatement(sql.c_str());
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        ECClassId rootClassId = stmt->GetValueId<ECClassId>(0);
        ECClassId derivedClassId = stmt->GetValueId<ECClassId>(1);

        Utf8CP tableName = stmt->GetValueText(2);
        DbTable const* table = m_map.GetDbSchema().FindTable(tableName);
        BeAssert(table != nullptr);
        std::vector<ECClassId>& horizontalPartition = m_horizontalPartitions[rootClassId][table];
        if (derivedClassId == rootClassId)
            horizontalPartition.insert(horizontalPartition.begin(), derivedClassId);
        else
            horizontalPartition.insert(horizontalPartition.end(), derivedClassId);
        }

    m_loadedFlags.m_horizontalPartitionsIsLoaded = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle 08/2015
//---------------------------------------------------------------------------------------
bmap<ECN::ECClassId, LightweightCache::RelationshipEnd> const& LightweightCache::GetConstraintClassesForRelationshipClass(ECN::ECClassId relClassId) const
    {
    LoadRelationshipCache();
    return m_constraintClassIdsPerRelClassIds[relClassId];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
std::vector<ECClassId> const& LightweightCache::GetClassesForTable(DbTable const& table) const
    {
    LoadClassIdsPerTable();
    return m_classIdsPerTable[&table];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
bset<DbTable const*> const& LightweightCache::GetVerticalPartitionsForClass(ECN::ECClassId classId) const
    {
    LoadClassIdsPerTable();
    return m_tablesPerClassId[classId];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      10/2015
//---------------------------------------------------------------------------------------
LightweightCache::ClassIdsPerTableMap const& LightweightCache::GetHorizontalPartitionsForClass(ECN::ECClassId classId) const
    {
    LoadHorizontalPartitions();
    return m_horizontalPartitions[classId];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
void LightweightCache::Reset()
    {
    m_loadedFlags.m_classIdsPerTableIsLoaded =
        m_loadedFlags.m_relationshipPerTableLoaded =
        m_loadedFlags.m_horizontalPartitionsIsLoaded =
        m_loadedFlags.m_relationshipCacheIsLoaded = false;

    m_horizontalPartitions.clear();
    m_classIdsPerTable.clear();
    m_relationshipClassIdsPerConstraintClassIds.clear();
    m_constraintClassIdsPerRelClassIds.clear();
    m_storageDescriptions.clear();
    m_relationshipPerTable.clear();
    m_tablesPerClassId.clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
LightweightCache::LightweightCache(ECDbMap const& map) : m_map(map) { Reset(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
StorageDescription const& LightweightCache::GetStorageDescription(ClassMap const& classMap)  const
    {
    const ECClassId classId = classMap.GetClass().GetId();
    auto it = m_storageDescriptions.find(classId);
    if (it == m_storageDescriptions.end())
        {
        auto des = StorageDescription::Create(classMap, *this);
        auto desP = des.get();
        m_storageDescriptions[classId] = std::move(des);
        return *desP;
        }

    return *(it->second.get());
    }


//****************************************************************************************
// StorageDescription
//****************************************************************************************

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    10 / 2015
//------------------------------------------------------------------------------------------
BentleyStatus StorageDescription::GenerateECClassIdFilter(Utf8StringR filterSqlExpression, DbTable const& table, DbColumn const& classIdColumn, bool polymorphic, bool fullyQualifyColumnName, Utf8CP tableAlias) const
    {
    if (table.GetPersistenceType() != PersistenceType::Persisted)
        return SUCCESS; //table is virtual -> noop

    Partition const* partition = GetHorizontalPartition(table);
    if (partition == nullptr)
        {
        if (!GetVerticalPartitions().empty())
            {
            partition = GetVerticalPartition(table);
            }

        if (partition == nullptr)
            {
            BeAssert(false && "Should always find a partition for the given table");
            return ERROR;
            }
        }

    Utf8String classIdColSql;
    if (fullyQualifyColumnName)
        {
        classIdColSql.append("[");
        if (tableAlias)
            classIdColSql.append(tableAlias);
        else
            classIdColSql.append(table.GetName());

        classIdColSql.append("].");
        }

    classIdColSql.append(classIdColumn.GetName());

    if (!polymorphic)
        {
        //if partition's table is only used by a single class, no filter needed     
        if (partition->IsSharedTable())
            {
            Utf8Char classIdStr[ECClassId::ID_STRINGBUFFER_LENGTH];
            m_classId.ToString(classIdStr);
            filterSqlExpression.append(classIdColSql).append("=").append(classIdStr);
            }

        return SUCCESS;
        }

    partition->AppendECClassIdFilterSql(filterSqlExpression, classIdColSql.c_str());
    return SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan    05 / 2015
//------------------------------------------------------------------------------------------
//static
std::unique_ptr<StorageDescription> StorageDescription::Create(ClassMap const& classMap, LightweightCache const& lwmc)
    {
    const ECClassId classId = classMap.GetClass().GetId();
    std::unique_ptr<StorageDescription> storageDescription = std::unique_ptr<StorageDescription>(new StorageDescription(classId));
    std::set<ECClassId> derviedClassSet;
    if (classMap.GetType() == ClassMap::Type::RelationshipEndTable)
        {
        RelationshipClassEndTableMap const& relClassMap = static_cast<RelationshipClassEndTableMap const&> (classMap);
        for (DbTable const* endTable : relClassMap.GetTables())
            {
            const LightweightCache::RelationshipEnd foreignEnd = relClassMap.GetForeignEnd() == ECRelationshipEnd::ECRelationshipEnd_Source ? LightweightCache::RelationshipEnd::Source : LightweightCache::RelationshipEnd::Target;

            Partition* hp = storageDescription->AddHorizontalPartition(*endTable, true);

            for (bpair<ECClassId, LightweightCache::RelationshipEnd> const& kvpair : lwmc.GetConstraintClassesForRelationshipClass(classId))
                {
                ECClassId constraintClassId = kvpair.first;
                LightweightCache::RelationshipEnd end = kvpair.second;

                if (end == LightweightCache::RelationshipEnd::Both || end == foreignEnd)
                    hp->AddClassId(constraintClassId);
                }

            hp->GenerateClassIdFilter(lwmc.GetClassesForTable(*endTable));
            }

        }
    else
        {
        for (auto& kp : lwmc.GetHorizontalPartitionsForClass(classId))
            {
            auto table = kp.first;

            auto& deriveClassList = kp.second;
            derviedClassSet.insert(deriveClassList.begin(), deriveClassList.end());
            if (deriveClassList.empty())
                continue;

            Partition* hp = storageDescription->AddHorizontalPartition(*table, deriveClassList.front() == classId);
            for (ECClassId ecClassId : deriveClassList)
                {
                hp->AddClassId(ecClassId);
                }

            hp->GenerateClassIdFilter(lwmc.GetClassesForTable(*table));
            }
        }
    //add vertical partitions
    for (auto table : lwmc.GetVerticalPartitionsForClass(classId))
        {
        if (table->GetPersistenceType() == PersistenceType::Virtual)
            continue;

        Partition* vp = storageDescription->AddVerticalPartition(*table, storageDescription->GetHorizontalPartition(*table) != nullptr);
        for (ECClassId ecClassId : derviedClassSet)
            {
            vp->AddClassId(ecClassId);
            }

        vp->GenerateClassIdFilter(lwmc.GetClassesForTable(*table));
        }

    return storageDescription;
    }


//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    10 / 2015
//------------------------------------------------------------------------------------------
Partition const* StorageDescription::GetHorizontalPartition(bool polymorphic) const
    {
    if (!polymorphic || !HasNonVirtualPartitions())
        return &GetRootHorizontalPartition();

    if (HierarchyMapsToMultipleTables())
        return nullptr; //no single partition available

    size_t ix = m_nonVirtualHorizontalPartitionIndices[0];
    BeAssert(ix < m_horizontalPartitions.size());

    return &m_horizontalPartitions[ix];
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    10 / 2015
//------------------------------------------------------------------------------------------
Partition const* StorageDescription::GetHorizontalPartition(DbTable const& table) const
    {
    for (Partition const& part : m_horizontalPartitions)
        {
        if (&part.GetTable() == &table)
            return &part;
        }

    return nullptr;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.KHan    11 / 2015
//------------------------------------------------------------------------------------------
Partition const* StorageDescription::GetVerticalPartition(DbTable const& table) const
    {
    for (Partition const& part : m_verticalPartitions)
        {
        if (&part.GetTable() == &table)
            return &part;
        }

    return nullptr;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    10 / 2015
//------------------------------------------------------------------------------------------
Partition const& StorageDescription::GetRootHorizontalPartition() const
    {
    BeAssert(m_rootHorizontalPartitionIndex < m_horizontalPartitions.size());
    return m_horizontalPartitions[m_rootHorizontalPartitionIndex];
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    05 / 2015
//------------------------------------------------------------------------------------------
Partition* StorageDescription::AddHorizontalPartition(DbTable const& table, bool isRootPartition)
    {
    const bool isVirtual = table.GetPersistenceType() == PersistenceType::Virtual;
    m_horizontalPartitions.push_back(Partition(table));

    const size_t indexOfAddedPartition = m_horizontalPartitions.size() - 1;
    if (!isVirtual)
        m_nonVirtualHorizontalPartitionIndices.push_back(indexOfAddedPartition);

    if (isRootPartition)
        m_rootHorizontalPartitionIndex = indexOfAddedPartition;

    return &m_horizontalPartitions[indexOfAddedPartition];
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan    11 / 2015
//------------------------------------------------------------------------------------------
Partition* StorageDescription::AddVerticalPartition(DbTable const& table, bool isRootPartition)
    {
    BeAssert(table.GetPersistenceType() == PersistenceType::Persisted);
    if (table.GetPersistenceType() == PersistenceType::Virtual)
        return nullptr;

    m_verticalPartitions.push_back(Partition(table));

    const size_t indexOfAddedPartition = m_verticalPartitions.size() - 1;
    if (isRootPartition)
        m_rootVerticalPartitionIndex = indexOfAddedPartition;

    return &m_verticalPartitions[indexOfAddedPartition];
    }

//****************************************************************************************
// Partition
//****************************************************************************************
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    02 / 2016
//------------------------------------------------------------------------------------------
Partition::Partition(Partition const& rhs)
    : m_table(rhs.m_table), m_partitionClassIds(rhs.m_partitionClassIds),
    m_inversedPartitionClassIds(rhs.m_inversedPartitionClassIds), m_hasInversedPartitionClassIds(rhs.m_hasInversedPartitionClassIds)
    {}

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    05 / 2015
//------------------------------------------------------------------------------------------
Partition& Partition::operator=(Partition const& rhs)
    {
    if (this != &rhs)
        {
        m_table = rhs.m_table;
        m_partitionClassIds = rhs.m_partitionClassIds;
        m_inversedPartitionClassIds = rhs.m_inversedPartitionClassIds;
        m_hasInversedPartitionClassIds = rhs.m_hasInversedPartitionClassIds;
        }

    return *this;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    05 / 2015
//------------------------------------------------------------------------------------------
Partition::Partition(Partition&& rhs)
    : m_table(std::move(rhs.m_table)), m_partitionClassIds(std::move(rhs.m_partitionClassIds)),
    m_inversedPartitionClassIds(std::move(rhs.m_inversedPartitionClassIds)), m_hasInversedPartitionClassIds(std::move(rhs.m_hasInversedPartitionClassIds))
    {
    //nulling out the RHS m_table pointer is defensive, even if the destructor doesn't
    //free the table (as it is now owned by Partition). If the ownership ever changes,
    //this method is safe.
    rhs.m_table = nullptr;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    05 / 2015
//------------------------------------------------------------------------------------------
void Partition::GenerateClassIdFilter(std::vector<ECN::ECClassId> const& tableClassIds)
    {
    BeAssert(!m_partitionClassIds.empty());
    m_hasInversedPartitionClassIds = m_partitionClassIds.size() > tableClassIds.size() / 2;
    if (m_partitionClassIds.size() == tableClassIds.size())
        return;

    //tableClassIds list is already sorted
    auto sortedPartitionClassIds = m_partitionClassIds;
    std::sort(sortedPartitionClassIds.begin(), sortedPartitionClassIds.end());

    auto partitionClassIdsIt = sortedPartitionClassIds.begin();
    for (ECClassId candidateClassId : tableClassIds)
        {
        if (partitionClassIdsIt == sortedPartitionClassIds.end() || candidateClassId < *partitionClassIdsIt)
            m_inversedPartitionClassIds.push_back(candidateClassId);
        else
            ++partitionClassIdsIt;
        }
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    05 / 2015
//------------------------------------------------------------------------------------------
bool Partition::NeedsECClassIdFilter() const
    {
    BeAssert(!m_partitionClassIds.empty());
    //If class ids are not inversed, we always have a non-empty partition class id list. So filtering is needed.
    //if class ids are inversed, filtering is needed if the inversed list is not empty. If it is empty, it means
    //don't filter at all -> consider all class ids
    return !m_inversedPartitionClassIds.empty();
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2015
//------------------------------------------------------------------------------------------
void Partition::AppendECClassIdFilterSql(Utf8StringR filterSqlExpression, Utf8CP classIdColName) const
    {
    BeAssert(!m_partitionClassIds.empty());

    std::vector<ECClassId> const* classIds = nullptr;
    Utf8CP equalOp = nullptr;
    Utf8CP setOp = nullptr;
    if (m_hasInversedPartitionClassIds)
        {
        classIds = &m_inversedPartitionClassIds;
        if (classIds->empty())
            return; //no filter needed, as all class ids should be considered

        equalOp = "<>";
        setOp = "AND";
        }
    else
        {
        classIds = &m_partitionClassIds;
        equalOp = "=";
        setOp = "OR";
        }

    bool isFirstItem = true;
    for (ECClassId classId : *classIds)
        {
        if (!isFirstItem)
            filterSqlExpression.append(" ").append(setOp).append(" ");

        Utf8Char classIdStr[BeInt64Id::ID_STRINGBUFFER_LENGTH];
        classId.ToString(classIdStr);
        filterSqlExpression.append(classIdColName).append(equalOp).append(classIdStr);

        isFirstItem = false;
        }
    }
END_BENTLEY_SQLITE_EC_NAMESPACE

