/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "SqlNames.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceFinder::FindOptions::FindOptions(int relatedDirections /* = RelatedDirection_None */, uint8_t relationshipDepth /* = 0 */, ECClassCP ecClass /* = nullptr */) :
m_relatedDirections (relatedDirections),
m_relationshipDepth (relationshipDepth)/*,
unused - m_ecClass (ecClass)*/
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceFinder::QueryableRelationship::QueryableRelationship(ECN::ECRelationshipClassCR relationshipClass, ECN::ECClassCR thisClass, ECN::ECRelationshipEnd thisRelationshipEnd)
    : m_relationshipClass(&relationshipClass), m_thisClass(&thisClass), m_thisRelationshipEnd(thisRelationshipEnd)
    {
    InitializeRelatedDirection();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceFinder::QueryableRelationship::InitializeRelatedDirection()
    {
    StrengthType strengthType = m_relationshipClass->GetStrength();
    if (strengthType == StrengthType::Referencing)
        {
        m_relatedDirection = RelatedDirection_Referencing;
        return;
        }

    ECRelatedInstanceDirection strengthDirection = m_relationshipClass->GetStrengthDirection();
    bool parentsToChildren = (m_thisRelationshipEnd == ECRelationshipEnd_Source && strengthDirection == ECRelatedInstanceDirection::Forward) ||
        (m_thisRelationshipEnd == ECRelationshipEnd_Target && strengthDirection == ECRelatedInstanceDirection::Backward);

    if (strengthType == StrengthType::Embedding)
        {
        m_relatedDirection = parentsToChildren ? RelatedDirection_EmbeddedChildren : RelatedDirection_EmbeddingParent;
        return;
        }

    BeAssert(strengthType == StrengthType::Holding);
    m_relatedDirection = parentsToChildren ? RelatedDirection_HeldChildren : RelatedDirection_HoldingParents;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceFinder::QueryableRelationship::CopyFrom(const QueryableRelationship& copyFrom)
    {
    m_relationshipClass = copyFrom.m_relationshipClass;
    m_thisClass = copyFrom.m_thisClass;
    m_thisRelationshipEnd = copyFrom.m_thisRelationshipEnd;
    m_relatedDirection = copyFrom.m_relatedDirection;
    m_cachedStatement = copyFrom.m_cachedStatement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceFinder::QueryableRelationship::QueryableRelationship(const QueryableRelationship& copyFrom)
    {
    CopyFrom(copyFrom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceFinder::QueryableRelationship& ECInstanceFinder::QueryableRelationship::operator= (const QueryableRelationship& copyFrom)
    {
    if (this == &copyFrom)
        return *this;
    CopyFrom(copyFrom);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSqlStatus ECInstanceFinder::QueryableRelationship::GetPreparedECSqlStatement(std::shared_ptr<ECSqlStatement>& cachedStatement, ECDbCR ecDb)
    {
    if (!m_cachedStatement /* Statement has not been created before - uses shared_ptr<>.operator bool */ ||
        m_cachedStatement.use_count() > 1 /* Statement is currently in use */)
        {
        /*
        * Prepare a new statement
        * Note that we are just caching one statement for querying the relationship. If the user requests another
        * when one is in use, we simply create and prepare a new statement. This situation is possible in the case the
        * same class gets repeated in the hierarchy of relationships. However, since it's an unlikely case we don't
        * prematurely optimize for that and stick to a single cached statement instead of a vector of them.
        */
        ECSqlStatus status = PrepareECSqlStatement(ecDb);
        if (!status.IsSuccess())
            return status;
        cachedStatement = m_cachedStatement;
        return status;
        }
    else
        {
        m_cachedStatement->Reset();
        m_cachedStatement->ClearBindings();
        cachedStatement = m_cachedStatement;
        }

    return ECSqlStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSqlStatus ECInstanceFinder::QueryableRelationship::PrepareECSqlStatement(ECDbCR ecDb)
    {
    Utf8CP relSchemaName = m_relationshipClass->GetSchema().GetName().c_str();
    Utf8CP relClassName = m_relationshipClass->GetName().c_str();
    Utf8String relECSql;
    if (m_thisRelationshipEnd == ECRelationshipEnd_Source)
        {
        relECSql.Sprintf("SELECT " ECDBSYS_PROP_ECInstanceId "," ECDBSYS_PROP_TargetECClassId "," ECDBSYS_PROP_TargetECInstanceId
                         " FROM %s.%s WHERE " ECDBSYS_PROP_SourceECClassId "=%s AND InVirtualSet(?, " ECDBSYS_PROP_SourceECInstanceId ")",
                         relSchemaName, relClassName, m_thisClass->GetId().ToString().c_str());
        }
    else
        {
        relECSql.Sprintf("SELECT " ECDBSYS_PROP_ECInstanceId "," ECDBSYS_PROP_SourceECClassId "," ECDBSYS_PROP_SourceECInstanceId
                         " FROM %s.%s WHERE " ECDBSYS_PROP_TargetECClassId "=%s AND InVirtualSet(?, " ECDBSYS_PROP_TargetECInstanceId ")",
                         relSchemaName, relClassName, m_thisClass->GetId().ToString().c_str());
        }

    m_cachedStatement = std::make_shared<ECSqlStatement>();
    ECSqlStatus status = m_cachedStatement->Prepare(ecDb, relECSql.c_str());
    if (!status.IsSuccess())
        m_cachedStatement = nullptr;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECInstanceFinder::QueryableRelationship::ToString()
    {
    Utf8PrintfString str("%s:%s->%s",
                         Utf8String(m_thisClass->GetName().c_str()).c_str(),
                         m_thisRelationshipEnd == ECRelationshipEnd_Source ? "Source" : "Target",
                         Utf8String(m_relationshipClass->GetName().c_str()).c_str()
    );
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceFinder::ECInstanceFinder(ECDbCR ecDb) : m_ecDb(ecDb), m_queryableRelationshipsByClass()
    {}

/*---------------------------------------------------------------------------------**//**
* Finalizes all the held prepared statements. Note that this needs to be done before the Db can
* be closed, or we will get assertion.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceFinder::Finalize()
    {
    m_queryableRelationshipsByClass.clear();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECInstanceFinder::FindRelationshipsOnEnd(QueryableRelationshipVector& queryableRelationships, ECClassId foreignEndClassId, ECDbCR ecDb)
    {
    queryableRelationships.clear();

    CachedStatementPtr stmt = ecDb.GetImpl().GetCachedSqliteStatement(" WITH RECURSIVE"
        "    BaseClassesOfEndClass(ClassId) AS  ("
        "    VALUES (:endClassId)"
        "    UNION "
        "    SELECT BaseClassId FROM main.ec_ClassHasBaseClasses, BaseClassesOfEndClass WHERE ec_ClassHasBaseClasses.ClassId=BaseClassesOfEndClass.ClassId"
        "    )"
        " SELECT DISTINCT ECRelationshipClass.Id AS RelationshipId, ForeignEndConstraint.RelationshipEnd As ForeignEndIsTarget "
        " FROM main.ec_Class ECRelationshipClass "
        " JOIN main.ec_RelationshipConstraint ForeignEndConstraint ON ForeignEndConstraint.RelationshipClassId = ECRelationshipClass.Id "
        " JOIN main.ec_RelationshipConstraintClass ForeignEndConstraintClass ON ForeignEndConstraintClass.ConstraintId=ForeignEndConstraint.Id "
        " JOIN BaseClassesOfEndClass"
        " WHERE ForeignEndConstraintClass.ClassId = :endClassId "
                                              "   OR (ForeignEndConstraint.IsPolymorphic = " SQLVAL_True " AND ForeignEndConstraintClass.ClassId = BaseClassesOfEndClass.ClassId)");
    if (stmt == nullptr)
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    SchemaManager const& schemaManager = ecDb.Schemas();
    ECClassCP foreignEndClass = schemaManager.GetClass(foreignEndClassId);
    BeAssert(foreignEndClass != nullptr);
    int endClassIdx = stmt->GetParameterIndex(":endClassId");
    stmt->BindId(endClassIdx, foreignEndClass->GetId());

    while (BE_SQLITE_ROW == stmt->Step())
        {
        ECClassId ecRelationshipClassId = stmt->GetValueId<ECClassId>(0);
        ECClassCP ecClass = schemaManager.GetClass(ecRelationshipClassId);
        ECRelationshipClassCP ecRelationshipClass = ecClass->GetRelationshipClassCP();
        BeAssert(ecRelationshipClass != nullptr);

        ECRelationshipEnd thisRelationshipEnd = (ECRelationshipEnd) stmt->GetValueInt(1);
        queryableRelationships.push_back(QueryableRelationship(*ecRelationshipClass, *foreignEndClass, thisRelationshipEnd));
        }

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECInstanceFinder::GetRelationshipsOnEnd(QueryableRelationshipVectorP &queryableRelationships, ECClassId foreignEndClassId)
    {
    queryableRelationships = nullptr;
    QueryableRelationshipsByClass::iterator iter = m_queryableRelationshipsByClass.find(foreignEndClassId);
    if (iter != m_queryableRelationshipsByClass.end())
        {
        queryableRelationships = &(iter->second);
        }
    else
        {
        DbResult result = FindRelationshipsOnEnd(m_queryableRelationshipsByClass[foreignEndClassId], foreignEndClassId, m_ecDb);
        if (result != BE_SQLITE_OK)
            return result;
        queryableRelationships = &(m_queryableRelationshipsByClass[foreignEndClassId]);
        }
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceFinder::DumpInstanceKeyMap(const ECInstanceKeyMultiMap& instanceKeyMultiMap, ECDbCR ecDb)
    {
    if (instanceKeyMultiMap.size() == 0)
        {
        LOG.tracev("Map is empty");
        return;
        }

    ECInstanceKeyMultiMapConstIterator instanceIdIter;
    for (ECInstanceKeyMultiMapConstIterator classIdIter = instanceKeyMultiMap.begin(); classIdIter != instanceKeyMultiMap.end(); classIdIter = instanceIdIter)
        {
        bvector<ECInstanceId> ids;
        ECClassId classId = classIdIter->first;
        bpair<ECInstanceKeyMultiMapConstIterator, ECInstanceKeyMultiMapConstIterator> keyRange = instanceKeyMultiMap.equal_range(classId);
        for (instanceIdIter = keyRange.first; instanceIdIter != keyRange.second; instanceIdIter++)
            {
            ECInstanceId instanceId = instanceIdIter->second;
            ids.push_back(instanceId);
            }

        if (ids.size() == 0)
            continue;

        ECClassCP ecClass = ecDb.Schemas().GetClass(classId);

        LOG.tracev("Class: %s, Instances:%" PRIu64, ecClass->GetName().c_str(), (uint64_t) ids.size());
        for (ECInstanceId& id : ids)
            LOG.tracev("Instance %s", id.ToString().c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceFinder::FindRelatedInstances
(
    ECInstanceKeyMultiMap* relatedInstanceKeyMap,
    ECInstanceKeyMultiMap* relationshipInstanceKeyMap,
    const ECInstanceKey seedInstanceKey,
    int findRelatedDirections
)
    {
    ECInstanceKeyMultiMap seedInstanceKeyMap;
    ECInstanceKeyMultiMapPair seedInstanceEntry(seedInstanceKey.GetClassId(), seedInstanceKey.GetInstanceId());
    seedInstanceKeyMap.insert(seedInstanceEntry);

    return FindRelatedInstances(relatedInstanceKeyMap, relationshipInstanceKeyMap, seedInstanceKeyMap, findRelatedDirections);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceFinder::FindRelatedInstances(ECInstanceKeyMultiMap* relatedInstanceKeyMap, ECInstanceKeyMultiMap* relationshipInstanceKeyMap, ECInstanceKeyMultiMap const& seedInstanceKeyMap, int findRelatedDirections)
    {
    // Initialize incoming maps
    if (relationshipInstanceKeyMap != nullptr)
        relationshipInstanceKeyMap->clear();
    if (relatedInstanceKeyMap != nullptr)
        relatedInstanceKeyMap->clear();

    // Iterate through every seed instance class-by-class
    for (ECInstanceKeyMultiMapConstIterator classIdIter = seedInstanceKeyMap.begin(); classIdIter != seedInstanceKeyMap.end(); )
        {
        // Gather all containing relationships on "this end" class (if possible uses relationships cached previously)
        ECClassId thisClassId = classIdIter->first;
        QueryableRelationshipVectorP queryableRelationshipsOnForeignEnd = nullptr;
        DbResult result = GetRelationshipsOnEnd(queryableRelationshipsOnForeignEnd, thisClassId);
        POSTCONDITION(result == BE_SQLITE_OK, ERROR);
        BeAssert(queryableRelationshipsOnForeignEnd != nullptr);

        // Iterate through all relationships on "this end" class
        bpair<ECInstanceKeyMultiMapConstIterator, ECInstanceKeyMultiMapConstIterator> instanceIdRange = seedInstanceKeyMap.equal_range(thisClassId);
        for (QueryableRelationship& queryableRelationship : *queryableRelationshipsOnForeignEnd)
            {
            RelatedDirection relatedDirection = queryableRelationship.GetRelatedDirection();
            if (!(findRelatedDirections & relatedDirection))
                continue;

            // Get a prepared statement to traverse the relationship (if possible use one prepared before)
            std::shared_ptr<ECSqlStatement> statement;
            ECSqlStatus status;

            auto classP = queryableRelationship.GetRelationshipClass();
            POSTCONDITION(classP != nullptr, ERROR)
            ClassMap const* classMapP = m_ecDb.Schemas().Main().GetClassMap(*classP);
            POSTCONDITION(classMapP != nullptr, ERROR)
            if (classMapP->GetMapStrategy().GetStrategy() == MapStrategy::NotMapped)
                {
                continue;
                }

            status = queryableRelationship.GetPreparedECSqlStatement(statement, m_ecDb);
            POSTCONDITION(status.IsSuccess(), ERROR);
            BeAssert(statement != nullptr);
            statement->Reset();
            statement->ClearBindings();

            // Make all seed instances with "this end" class our search criteria
            std::shared_ptr<ECInstanceIdSet> seedIdSet = std::make_shared<ECInstanceIdSet>();
            for (auto it = instanceIdRange.first; it != instanceIdRange.second; ++it)
                seedIdSet->insert(it->second);

            statement->BindVirtualSet(1, seedIdSet);

            ECClassId relationshipClassId = (ECClassId) queryableRelationship.GetRelationshipClass()->GetId();

            // Iterate through all found relationship instances with "this end" as one of the seed instances
            DbResult stepStatus = BE_SQLITE_DONE;
            while ((stepStatus = statement->Step()) == BE_SQLITE_ROW)
                {
                // Get relationship instance (key)
                if (relationshipInstanceKeyMap != nullptr)
                    {
                    ECInstanceId relationshipInstanceId = statement->GetValueId<ECInstanceId>(0);
                    POSTCONDITION(relationshipClassId.IsValid() && relationshipInstanceId.IsValid(), ERROR);
                    ECInstanceKeyMultiMapPair relationshipEntry(relationshipClassId, relationshipInstanceId);
                    relationshipInstanceKeyMap->insert(relationshipEntry);
                    }

                // Get related instance (key)
                if (relatedInstanceKeyMap != nullptr)
                    {
                    ECClassId referencedEndClassId = (ECClassId) statement->GetValueId<ECClassId>(1);
                    ECInstanceId referencedEndInstanceId = statement->GetValueId<ECInstanceId>(2);
                    POSTCONDITION(referencedEndClassId.IsValid() && referencedEndInstanceId.IsValid(), ERROR);
                    ECInstanceKeyMultiMapPair relatedInstanceEntry(referencedEndClassId, referencedEndInstanceId);
                    if (relatedInstanceKeyMap->end() == std::find(relatedInstanceKeyMap->begin(), relatedInstanceKeyMap->end(), relatedInstanceEntry))
                        relatedInstanceKeyMap->insert(relatedInstanceEntry);
                    }
                }

            POSTCONDITION(stepStatus == BE_SQLITE_DONE, ERROR);
            }

        classIdIter = instanceIdRange.second;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceFinder::FindInstancesRecursive
(
    ECInstanceKeyMultiMap& instanceKeyMap,
    const ECInstanceKeyMultiMap& seedInstanceKeyMap,
    FindOptions findOptions,
    uint8_t& currentDepth
)
    {
    if (seedInstanceKeyMap.size() == 0)
        return SUCCESS;

    // Add seed instances to the result map
    for (ECInstanceKeyMultiMapConstIterator iter = seedInstanceKeyMap.begin(); iter != seedInstanceKeyMap.end(); iter++)
        {
        ECInstanceKeyMultiMapPair instanceEntry(iter->first, iter->second);
        instanceKeyMap.insert(instanceEntry);
        }
#ifndef NDEBUG
    if (LOG.isSeverityEnabled(NativeLogging::LOG_TRACE))
        {
        LOG.tracev("After a collection (depth %" PRIu8 ") map, new instances added:", currentDepth);
        DumpInstanceKeyMap(seedInstanceKeyMap, m_ecDb);
        }
#endif
    // Check for end of recursion
    if (currentDepth >= findOptions.m_relationshipDepth)
        return SUCCESS;

    // Find instances immediately related to the seed instances
    ECInstanceKeyMultiMap newSeedInstanceKeyMap;
    BentleyStatus status = FindRelatedInstances(&newSeedInstanceKeyMap, nullptr, seedInstanceKeyMap, findOptions.m_relatedDirections);
    if (status != SUCCESS)
        return status;

    // Remove entries that are already in the instanceKeyMap (these represent cycles)
    for (ECInstanceKeyMultiMapIterator iter = newSeedInstanceKeyMap.begin(); iter != newSeedInstanceKeyMap.end(); /* no increment due to modifications */)
        {
        ECInstanceKeyMultiMapPair instanceEntry(iter->first, iter->second);
        if (instanceKeyMap.end() != std::find(instanceKeyMap.begin(), instanceKeyMap.end(), instanceEntry))
            {
            ECClassCP ecClass = m_ecDb.Schemas().GetClass(iter->first);
            LOG.warningv("Detected a relationship cycle with instance id %s of class %s", iter->second.ToString().c_str(),
                         ecClass->GetName().c_str());
            iter = newSeedInstanceKeyMap.erase(iter); // C++ 11 returns the next entry after erase
            }
        else
            {
            ++iter;
            }
        }

    // Recurse
    status = FindInstancesRecursive(instanceKeyMap, newSeedInstanceKeyMap, findOptions, ++currentDepth);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceFinder::FindInstances
(
    ECInstanceKeyMultiMap& instanceKeyMap,
    const ECInstanceKeyMultiMap& seedInstanceKeyMap,
    FindOptions findOptions
)
    {
    uint8_t currentDepth = 0;
    instanceKeyMap.clear();
    return FindInstancesRecursive(instanceKeyMap, seedInstanceKeyMap, findOptions, currentDepth);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

