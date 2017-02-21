/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/LightweightCache.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <vector>
#include "SqlNames.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//************************************************************************************
// LightweightCache
//************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
LightweightCache::LightweightCache(ECDb const& ecdb) : m_ecdb(ecdb) { Reset(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
std::vector<ECN::ECClassId> const& LightweightCache::LoadClassIdsPerTable(DbTable const& tbl) const
    {
    auto itor = m_classIdsPerTable.find(&tbl);
    if (itor != m_classIdsPerTable.end())
        return itor->second;

    std::vector<ECN::ECClassId>& subset = m_classIdsPerTable[&tbl];
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT ClassId FROM " TABLE_ClassHasTablesCache " WHERE TableId = ?");
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
    auto itor = m_tablesPerClassId.find(classId);
    if (itor != m_tablesPerClassId.end())
        return itor->second;

    bset<DbTable const*>& subset = m_tablesPerClassId[classId];
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT TableId FROM " TABLE_ClassHasTablesCache " WHERE ClassId = ? ORDER BY TableId");
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
            currentTable = m_ecdb.Schemas().GetDbMap().GetDbSchema().FindTable(tableId);
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
bmap<ECN::ECClassId, LightweightCache::RelationshipEnd> const& LightweightCache::GetRelationshipClasssForConstraintClass(ECN::ECClassId constraintClassId) const
    {
    return LoadRelationshipConstraintClasses(constraintClassId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
bmap<ECN::ECClassId, LightweightCache::RelationshipEnd> const& LightweightCache::LoadRelationshipConstraintClasses(ECN::ECClassId constraintClassId) const
    {
    auto itor = m_relationshipClassIdsPerConstraintClassIds.find(constraintClassId);
    if (itor != m_relationshipClassIdsPerConstraintClassIds.end())
        return itor->second;

    bmap<ECN::ECClassId, RelationshipEnd>& relClassIds = m_relationshipClassIdsPerConstraintClassIds[constraintClassId];
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT RC.RelationshipClassId, RC.RelationshipEnd FROM ec_RelationshipConstraintClass RCC"
                                                                 "       INNER JOIN ec_RelationshipConstraint RC ON RC.Id = RCC.ConstraintId"
                                                                 "       LEFT JOIN " TABLE_ClassHierarchyCache " CH ON CH.BaseClassId = RCC.ClassId AND RC.IsPolymorphic=" SQLVAL_True " AND CH.ClassId=? "
                                                                 "WHERE RCC.ClassId=?");

    if (stmt == nullptr)
        {
        BeAssert(false);
        return relClassIds;
        }

    if (BE_SQLITE_OK != stmt->BindId(1, constraintClassId) || //!This speed up query from 98ms to 28 ms by remove OR results that are not required in LEFT JOIN
        BE_SQLITE_OK != stmt->BindId(2, constraintClassId))
        {
        BeAssert(false);
        return relClassIds;
        }

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        ECClassId relationshipId = stmt->GetValueId<ECClassId>(0);
        BeAssert(!stmt->IsColumnNull(1));
        const ECRelationshipEnd ecRelEnd = (ECRelationshipEnd) stmt->GetValueInt(1);
        RelationshipEnd end = ecRelEnd == ECRelationshipEnd_Source ? RelationshipEnd::Source : RelationshipEnd::Target;

        auto relIt = relClassIds.find(relationshipId);
        if (relIt == relClassIds.end())
            relClassIds[relationshipId] = end;
        else
            relClassIds[relationshipId] = static_cast<RelationshipEnd>((int) (relIt->second) | (int) (end));
        }

    return relClassIds;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
bmap<ECN::ECClassId, LightweightCache::RelationshipEnd> const& LightweightCache::LoadConstraintClassesForRelationships(ECN::ECClassId relationshipId) const
    {
    auto itor = m_constraintClassIdsPerRelClassIds.find(relationshipId);
    if (itor != m_constraintClassIdsPerRelClassIds.end())
        return itor->second;

    bmap<ECN::ECClassId, RelationshipEnd>& constraintClassIds = m_constraintClassIdsPerRelClassIds[relationshipId];
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT IFNULL(CH.ClassId, RCC.ClassId) ConstraintClassId, RC.RelationshipEnd FROM ec_RelationshipConstraintClass RCC"
                                                                 "       INNER JOIN ec_RelationshipConstraint RC ON RC.Id = RCC.ConstraintId"
                                                                 "       LEFT JOIN " TABLE_ClassHierarchyCache " CH ON CH.BaseClassId = RCC.ClassId AND RC.IsPolymorphic=" SQLVAL_True
                                                                 " WHERE RC.RelationshipClassId=?");

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
            constraintClassIds[constraintClassId] = static_cast<RelationshipEnd>((int) (constraintIt->second) | (int) (end));
        }

    return constraintClassIds;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
LightweightCache::ClassIdsPerTableMap const& LightweightCache::LoadHorizontalPartitions(ECN::ECClassId classId) const
    {
    auto itor = m_horizontalPartitions.find(classId);
    if (itor != m_horizontalPartitions.end())
        return itor->second;

    ClassIdsPerTableMap& subset = m_horizontalPartitions[classId];

    CachedStatementPtr stmt = m_ecdb.GetCachedStatement(
        "SELECT ch.ClassId, ct.TableId FROM " TABLE_ClassHasTablesCache " ct"
        "       INNER JOIN " TABLE_ClassHierarchyCache " ch ON ch.ClassId = ct.ClassId"
        "       INNER JOIN ec_ClassMap cm ON cm.ClassId=ch.BaseClassId"
        "       INNER JOIN ec_Table t ON t.Id = ct.TableId "
        "WHERE ch.BaseClassId=?1 AND (cm.MapStrategy<>" SQLVAL_MapStrategy_TablePerHierarchy " OR t.Type<>" SQLVAL_DbTable_Type_Joined ")");
    BeAssert(stmt != nullptr);
    stmt->BindId(1, classId);

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        ECClassId derivedClassId = stmt->GetValueId<ECClassId>(0);
        DbTableId tableId = stmt->GetValueId<DbTableId>(1);
        DbTable const* table = m_ecdb.Schemas().GetDbMap().GetDbSchema().FindTable(tableId);
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
void LightweightCache::Reset()
    {
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
//@bsimethod                                                    Krischan.Eberle    10 / 2015
//------------------------------------------------------------------------------------------
BentleyStatus StorageDescription::GenerateECClassIdFilter(Utf8StringR filterSqlExpression, DbTable const& table, DbColumn const& classIdColumn, bool polymorphic, bool fullyQualifyColumnName, Utf8CP tableAlias) const
    {
    if (table.GetPersistenceType() != PersistenceType::Physical)
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
    Utf8Char classIdStr[ECClassId::ID_STRINGBUFFER_LENGTH];
    m_classId.ToString(classIdStr);

    if (!polymorphic)
        {
        //if partition's table is only used by a single class, no filter needed     
        if (partition->IsSharedTable())
            {
            filterSqlExpression.append(classIdColSql).append("=").append(classIdStr);
            }

        return SUCCESS;
        }

    if ( partition->NeedsECClassIdFilter())
        {
        filterSqlExpression.append(classIdColSql).append(" IN (SELECT ClassId FROM " TABLE_ClassHierarchyCache " WHERE BaseClassId=").append(classIdStr).append(")");
        }
   
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
    BeAssert(table.GetPersistenceType() == PersistenceType::Physical);
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

