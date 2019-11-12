/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
LightweightCache::LightweightCache(TableSpaceSchemaManager const& manager) : m_schemaManager(manager) { Clear(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle 08/2015
//---------------------------------------------------------------------------------------
bmap<ECN::ECClassId, LightweightCache::RelationshipEnd> const& LightweightCache::GetConstraintClassesForRelationshipClass(ECN::ECClassId relClassId) const
    {
    return LoadConstraintClassesForRelationships(relClassId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
std::vector<ECClassId> const& LightweightCache::GetClassesForTable(DbTable const& table) const
    {
    return LoadClassIdsPerTable(table);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
bset<DbTable const*> const& LightweightCache::GetVerticalPartitionsForClass(ECN::ECClassId classId) const
    {
    return LoadTablesForClassId(classId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      10/2015
//---------------------------------------------------------------------------------------
LightweightCache::ClassIdsPerTableMap const& LightweightCache::GetHorizontalPartitionsForClass(ECN::ECClassId classId) const
    {
    return LoadHorizontalPartitions(classId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
StorageDescription const& LightweightCache::GetStorageDescription(ClassMap const& classMap)  const
    {
    const ECClassId classId = classMap.GetClass().GetId();
    BeMutexHolder ecdbLock(GetECDbMutex());
    auto it = m_storageDescriptions.find(classId);
    if (it != m_storageDescriptions.end())
        return *(it->second.get());

    auto des = StorageDescription::Create(classMap, *this);
    auto desP = des.get();
    m_storageDescriptions[classId] = std::move(des);
    return *desP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
std::vector<ECN::ECClassId> const& LightweightCache::LoadClassIdsPerTable(DbTable const& tbl) const
    {
    BeMutexHolder ecdbLock(GetECDbMutex());
    auto itor = m_classIdsPerTable.find(&tbl);
    if (itor != m_classIdsPerTable.end())
        return itor->second;

    std::vector<ECN::ECClassId>& subset = m_classIdsPerTable[&tbl];
    CachedStatementPtr stmt = nullptr;
    if (m_schemaManager.IsMain())
        stmt = GetCachedStatement("SELECT ClassId FROM main." TABLE_ClassHasTablesCache " WHERE TableId=?");
    else
        stmt = GetCachedStatement(Utf8PrintfString("SELECT ClassId FROM [%s]." TABLE_ClassHasTablesCache " WHERE TableId=?", m_schemaManager.GetTableSpace().GetName().c_str()).c_str());

    if (stmt == nullptr)
        {
        BeAssert(false);
        return subset;
        }

    if (BE_SQLITE_OK != stmt->BindId(1, tbl.GetId()))
        {
        BeAssert(false);
        return subset;
        }

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        ECClassId id = stmt->GetValueId<ECClassId>(0);
        subset.push_back(id);
        }

    return subset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
bset<DbTable const*> const& LightweightCache::LoadTablesForClassId(ECN::ECClassId classId) const
    {
    BeMutexHolder ecdbLock(GetECDbMutex());
    auto itor = m_tablesPerClassId.find(classId);
    if (itor != m_tablesPerClassId.end())
        return itor->second;

    bset<DbTable const*>& subset = m_tablesPerClassId[classId];
    CachedStatementPtr stmt = nullptr;
    if (m_schemaManager.IsMain())
        stmt = GetCachedStatement("SELECT TableId FROM main." TABLE_ClassHasTablesCache " WHERE ClassId = ? ORDER BY TableId");
    else
        stmt = GetCachedStatement(Utf8PrintfString("SELECT TableId FROM [%s]." TABLE_ClassHasTablesCache " WHERE ClassId = ? ORDER BY TableId", m_schemaManager.GetTableSpace().GetName().c_str()).c_str());

    if (stmt == nullptr)
        {
        BeAssert(false);
        return subset;
        }

    if (BE_SQLITE_OK != stmt->BindId(1, classId))
        {
        BeAssert(false);
        return subset;
        }

    DbTableId currentTableId;
    DbTable const* currentTable = nullptr;

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        DbTableId tableId = stmt->GetValueId<DbTableId>(0);
        if (currentTableId != tableId)
            {
            currentTable = m_schemaManager.GetDbSchema().FindTable(tableId);
            currentTableId = tableId;
            BeAssert(currentTable != nullptr);
            }

        subset.insert(currentTable);
        }

    return subset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
bmap<ECN::ECClassId, LightweightCache::RelationshipEnd> const& LightweightCache::LoadConstraintClassesForRelationships(ECN::ECClassId relationshipId) const
    {        
    BeMutexHolder ecdbLock(GetECDbMutex());
    auto itor = m_constraintClassIdsPerRelClassIds.find(relationshipId);
    if (itor != m_constraintClassIdsPerRelClassIds.end())
        return itor->second;
        
    bmap<ECN::ECClassId, RelationshipEnd>& constraintClassIds = m_constraintClassIdsPerRelClassIds[relationshipId];
    Utf8CP tableSpace = m_schemaManager.GetTableSpace().GetName().c_str();
    CachedStatementPtr stmt = GetCachedStatement(Utf8PrintfString("SELECT IFNULL(CH.ClassId, RCC.ClassId) ConstraintClassId, RC.RelationshipEnd FROM [%s].ec_RelationshipConstraintClass RCC"
                                                                 "       INNER JOIN [%s].ec_RelationshipConstraint RC ON RC.Id = RCC.ConstraintId"
                                                                 "       LEFT JOIN [%s]." TABLE_ClassHierarchyCache " CH ON CH.BaseClassId = RCC.ClassId AND RC.IsPolymorphic=" SQLVAL_True
                                                                 " WHERE RC.RelationshipClassId=?", tableSpace, tableSpace, tableSpace).c_str());

    if (stmt == nullptr)
        {
        BeAssert(false);
        return constraintClassIds;
        }

    if (BE_SQLITE_OK != stmt->BindId(1, relationshipId))
        {
        BeAssert(false);
        return constraintClassIds;
        }

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        ECClassId constraintClassId = stmt->GetValueId<ECClassId>(0);
        BeAssert(!stmt->IsColumnNull(1));
        const ECRelationshipEnd ecRelEnd = (ECRelationshipEnd) stmt->GetValueInt(1);
        RelationshipEnd end = ecRelEnd == ECRelationshipEnd_Source ? RelationshipEnd::Source : RelationshipEnd::Target;

        auto constraintIt = constraintClassIds.find(constraintClassId);
        if (constraintIt == constraintClassIds.end())
            constraintClassIds[constraintClassId] = end;
        else
            constraintClassIds[constraintClassId] = (RelationshipEnd) ((int) (constraintIt->second) | (int) (end));
        }

    return constraintClassIds;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
LightweightCache::ClassIdsPerTableMap const& LightweightCache::LoadHorizontalPartitions(ECN::ECClassId classId) const
    {
    BeMutexHolder ecdbLock(GetECDbMutex());
    auto itor = m_horizontalPartitions.find(classId);
    if (itor != m_horizontalPartitions.end())
        return itor->second;

    ClassIdsPerTableMap& subset = m_horizontalPartitions[classId];
    Utf8CP tableSpace = m_schemaManager.GetTableSpace().GetName().c_str();
    bool isMixin = false;
        {
        CachedStatementPtr stmt = GetCachedStatement(Utf8PrintfString(
            "SELECT ca.ContainerId FROM [%s]." TABLE_Class " c "
            " INNER JOIN [%s]." TABLE_CustomAttribute " ca ON ca.ClassId=c.Id "
            " INNER JOIN [%s]." TABLE_Schema " s ON s.Id=c.SchemaId AND c.Name='IsMixin' AND s.Name='CoreCustomAttributes' "
            "WHERE ca.ContainerId=?", tableSpace, tableSpace, tableSpace).c_str());

        BeAssert(stmt != nullptr);
        stmt->BindId(1, classId);
        isMixin = (stmt->Step() == BE_SQLITE_ROW);
        }

    Utf8String sql;
    sql.Sprintf("SELECT ch.ClassId, t.Name FROM [%s]." TABLE_ClassHasTablesCache " ct"
                    " INNER JOIN [%s]." TABLE_ClassHierarchyCache " ch ON ch.ClassId = ct.ClassId"
                    " INNER JOIN [%s].ec_ClassMap cm ON cm.ClassId=ch.BaseClassId"
                    " INNER JOIN [%s].ec_Table t ON t.Id = ct.TableId"
                    " WHERE ch.BaseClassId=?", tableSpace, tableSpace, tableSpace, tableSpace);

    if (!isMixin)
        sql.append(" AND t.Type<>" SQLVAL_DbTable_Type_Joined " AND t.Type<>" SQLVAL_DbTable_Type_Overflow);

    CachedStatementPtr stmt = GetCachedStatement(sql.c_str());
    BeAssert(stmt != nullptr);
    stmt->BindId(1, classId);
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        ECClassId derivedClassId = stmt->GetValueId<ECClassId>(0);
        Utf8CP tableName = stmt->GetValueText(1);
        DbTable const* table = m_schemaManager.GetDbSchema().FindTable(tableName);
        BeAssert(table != nullptr);
        std::vector<ECClassId>& horizontalPartition = subset[table];
        if (derivedClassId == classId)
            horizontalPartition.insert(horizontalPartition.begin(), derivedClassId);
        else
            horizontalPartition.insert(horizontalPartition.end(), derivedClassId);
        }

    return subset;
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
void LightweightCache::Clear()    
    {
    m_horizontalPartitions.clear();
    m_classIdsPerTable.clear();
    m_constraintClassIdsPerRelClassIds.clear();
    m_storageDescriptions.clear();
    m_relationshipPerTable.clear();
    m_tablesPerClassId.clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle  12/2017
//---------------------------------------------------------------------------------------
CachedStatementPtr LightweightCache::GetCachedStatement(Utf8CP sql) const { return m_schemaManager.GetECDb().GetImpl().GetCachedSqliteStatement(sql); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle  12/2017
//---------------------------------------------------------------------------------------
BeMutex& LightweightCache::GetECDbMutex() const { return m_schemaManager.GetECDb().GetImpl().GetMutex(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle  12/2017
//---------------------------------------------------------------------------------------
ECDb& LightweightCache::GetECDbR() const { return const_cast<ECDb&>(m_schemaManager.GetECDb()); }

//****************************************************************************************
// StorageDescription
//****************************************************************************************
Partition const* StorageDescription::GetPartition(DbTable const& table) const
    {
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
            }
        }

    return partition;
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
        RelationshipClassEndTableMap const& relClassMap = classMap.GetAs<RelationshipClassEndTableMap> ();
        std::unique_ptr<ForeignKeyPartitionView> fkView = ForeignKeyPartitionView::CreateReadonly(classMap.GetSchemaManager(), relClassMap.GetRelationshipClass());
        if (fkView == nullptr)
            {
            BeAssert(false);
            return nullptr;
            }

        for (ForeignKeyPartitionView::Partition const* partition : fkView->GetPartitions(true/*onlyPhysical*/))
            {
            DbTable const& endTable = partition->GetTable();
            const LightweightCache::RelationshipEnd foreignEnd = relClassMap.GetForeignEnd() == ECRelationshipEnd::ECRelationshipEnd_Source ? LightweightCache::RelationshipEnd::Source : LightweightCache::RelationshipEnd::Target;
            Partition* hp = storageDescription->AddHorizontalPartition(endTable, true);
            for (bpair<ECClassId, LightweightCache::RelationshipEnd> const& kvpair : lwmc.GetConstraintClassesForRelationshipClass(classId))
                {
                ECClassId constraintClassId = kvpair.first;
                LightweightCache::RelationshipEnd end = kvpair.second;

                if (end == LightweightCache::RelationshipEnd::Both || end == foreignEnd)
                    hp->AddClassId(constraintClassId);
                }

            hp->GenerateClassIdFilter(lwmc.GetClassesForTable(endTable));
            }
        }
    else
        {
        for (auto const& kp : lwmc.GetHorizontalPartitionsForClass(classId))
            {
            DbTable const* table = kp.first;
            std::vector<ECClassId> const& deriveClassList = kp.second;
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
    for (DbTable const* table : lwmc.GetVerticalPartitionsForClass(classId))
        {
        if (table->GetType() == DbTable::Type::Virtual)
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
    m_horizontalPartitions.push_back(Partition(table));

    const size_t indexOfAddedPartition = m_horizontalPartitions.size() - 1;
    if (table.GetType() != DbTable::Type::Virtual)
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
    BeAssert(table.GetType() != DbTable::Type::Virtual);
    if (table.GetType() == DbTable::Type::Virtual)
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
Partition::Partition(Partition const& rhs) : m_table(rhs.m_table), m_partitionClassIds(rhs.m_partitionClassIds),
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
Partition::Partition(Partition&& rhs) : m_table(std::move(rhs.m_table)), m_partitionClassIds(std::move(rhs.m_partitionClassIds)),
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

