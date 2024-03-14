/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECDb/RelatedInstanceFinder.h>
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
RelatedInstanceFinder::Results RelatedInstanceFinder::FindAll(ECInstanceKey thisInstanceKey, DirectionFilter direction, BentleyStatus* result) const {
    BeMutexHolder lock(m_mutex);
    Results results;
    std::vector<std::tuple<ECSqlStatement*, ECN::ECRelatedInstanceDirection>> relationships;
    if (result) {
        *result = SUCCESS;
    }
    if (SUCCESS != FindRelevantRelationshipInfo(thisInstanceKey.GetClassId(), relationships)){
        if (result)
            *result = ERROR;
        return results;
    }

    if (relationships.empty()) {
        return results;
    }

    for (auto& relationship : relationships){
        const auto stmt = std::get<0>(relationship);
        const auto relDir = std::get<1>(relationship);
        const auto resolveDir = (int)relDir & (int)direction;
        if (!resolveDir)
            continue;

        stmt->Reset();
        stmt->ClearBindings();

        const auto isForward = (resolveDir & (int)ECRelatedInstanceDirection::Forward) > 0;
        const auto isBackward = (resolveDir & (int)ECRelatedInstanceDirection::Backward) > 0;

        stmt->BindInt(1, isForward);
        stmt->BindId(2, thisInstanceKey.GetInstanceId());
        stmt->BindInt(3, isBackward);
        stmt->BindId(4, thisInstanceKey.GetInstanceId());

        while(stmt->Step() == BE_SQLITE_ROW) {
            const auto direction = (ECRelatedInstanceDirection)stmt->GetValueInt(0);
            const auto relationshipId = stmt->GetValueId<ECClassId>(1);
            const auto otherEnd = ECInstanceKey(stmt->GetValueId<ECClassId>(3), stmt->GetValueId<ECInstanceId>(2));
            if (direction == ECRelatedInstanceDirection::Forward)
                results.emplace_back(thisInstanceKey, otherEnd, relationshipId, ECRelatedInstanceDirection::Forward);
            else
                results.emplace_back(otherEnd, thisInstanceKey, relationshipId, ECRelatedInstanceDirection::Backward);
        }
    }
    return results;
}
//=======================================================================================
// @bsiclass
//=======================================================================================
BentleyStatus RelatedInstanceFinder::InitializeRelationshipCache() const {
    BeMutexHolder lock(m_mutex);
    if ( m_relCache!= nullptr)
        return SUCCESS;

    auto sql = R"(
        SELECT
            COALESCE ([cch].[ClassId], [rcc].[ClassId]) [ClassId],
            [rc].[RelationshipClassId],
            IIF (MIN ([rc].[RelationshipEnd]) == 0, 1, 0) |
            IIF (MAX ([rc].[RelationshipEnd]) == 1, 2, 0)[Direction]
        FROM   [ec_RelationshipConstraint] [rc]
            JOIN [ec_RelationshipConstraintClass] [rcc] ON [rcc].[ConstraintId] = [rc].[Id]
            JOIN [ec_ClassMap] [cm] ON [cm].[ClassId] = [rc].[RelationshipClassId]
            LEFT JOIN [ec_ClassHasBaseClasses] [cbc] ON [cbc].[ClassId] = [rc].[RelationshipClassId]
            LEFT JOIN [ec_cache_ClassHierarchy] [cch] ON [cch].[BaseClassId] = [rcc].[ClassId]
                    AND [rc].[IsPolymorphic]
        WHERE  [cbc].[BaseClassId] IS NULL
        GROUP  BY
                COALESCE ([cch].[ClassId], [rcc].[ClassId]),
                [RelationshipClassId];
    )";

    Statement stmt;
    auto rc = stmt.Prepare(m_ecdb, sql);
    if (rc != BE_SQLITE_OK) {
        return ERROR;
    }

    auto currentClassId = ECClassId();
    std::vector<std::tuple<ECClassId, ECN::ECRelatedInstanceDirection>>* currentRelMap = nullptr;
    m_relCache = std::make_unique<bmap<ECClassId, std::vector<std::tuple<ECClassId, ECN::ECRelatedInstanceDirection>>>>();
    while (stmt.Step() == BE_SQLITE_ROW) {
        const auto classId = stmt.GetValueId<ECN::ECClassId>(0);
        const auto relClassId = stmt.GetValueId<ECN::ECClassId>(1);
        const auto direction = stmt.GetValueId<ECRelatedInstanceDirection>(2);

        if (currentClassId != classId)
            currentRelMap = &m_relCache->operator [](classId);

        if (currentRelMap)
            currentRelMap->push_back(std::make_tuple(relClassId, direction));
    }
    return SUCCESS;
}

//=======================================================================================
// @bsiclass
//=======================================================================================
BentleyStatus RelatedInstanceFinder::FindRelevantRelationshipInfo(ECN::ECClassId classId, std::vector<std::tuple<ECSqlStatement*,ECRelatedInstanceDirection>>& results) const {
    if (m_relCache == nullptr) {
        if (SUCCESS != InitializeRelationshipCache())
            return ERROR;
    }

    if(m_relCache == nullptr)
        return ERROR;

    results.clear();
    auto it = m_relCache->find(classId);
    if (it == m_relCache->end())
        return SUCCESS;

    for (auto& rels : it->second) {
        const auto relationshipClassId = std::get<0>(rels);
        const auto relatedInstanceDir = std::get<1>(rels);
        auto relIt = m_relationshipCachedECSql.find(relationshipClassId);
        if (relIt == m_relationshipCachedECSql.end()) {
            const auto rel = m_ecdb.Schemas().GetClass(relationshipClassId)->GetRelationshipClassCP();
            relIt = m_relationshipCachedECSql.insert(std::make_pair(relationshipClassId, std::make_unique<ECSqlStatement>())).first;
            Utf8String ecsql = SqlPrintfString(R"ecsql(
                SELECT 1 Direction, [ECClassId], [SourceECInstanceId], [SourceECClassId] FROM %s WHERE ? <> 0 AND [TargetECInstanceId] = ?
                UNION ALL
                SELECT 2 Direction, [ECClassId], [TargetECInstanceId], [TargetECClassId] FROM %s WHERE ? <> 0 AND [SourceECInstanceId] = ?
            )ecsql", rel->GetECSqlName().c_str(), rel->GetECSqlName().c_str()).GetUtf8CP();
            if (relIt->second->Prepare(m_ecdb, ecsql.c_str()) != ECSqlStatus::Success) {
                return ERROR;
            }
        }
        results.push_back(std::make_tuple(relIt->second.get(), relatedInstanceDir));
    }
    return SUCCESS;
}

END_BENTLEY_SQLITE_EC_NAMESPACE