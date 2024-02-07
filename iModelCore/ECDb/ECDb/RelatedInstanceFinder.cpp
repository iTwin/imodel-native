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
ECSqlStatement* RelatedInstanceFinder::RelationshipInfo::TryGetQueryStatement(ECDbCR ecdb, ECRelationshipEnd end) const {
    if (end == ECRelationshipEnd_Source) {
        if (!m_queryTargetStmt.IsPrepared()) {
            if (ECSqlStatus::Success != m_queryTargetStmt.Prepare(ecdb, SqlPrintfString("SELECT [TargetECClassId], [TargetECInstanceId], [ECClassId] FROM %s WHERE [SourceECInstanceId] =  ?", m_relationshipClass.GetECSqlName().c_str())))
                return nullptr;
        }
        m_queryTargetStmt.ClearBindings();
        m_queryTargetStmt.Reset();
        return &m_queryTargetStmt;
    }
    if (!m_querySourceStmt.IsPrepared()) {
        if (ECSqlStatus::Success != m_querySourceStmt.Prepare(ecdb, SqlPrintfString("SELECT [SourceECClassId], [SourceECInstanceId], [ECClassId] FROM %s WHERE [TargetECInstanceId] =  ?", m_relationshipClass.GetECSqlName().c_str())))
            return nullptr;
    }
    m_querySourceStmt.ClearBindings();
    m_querySourceStmt.Reset();
    return &m_querySourceStmt;
}

//=======================================================================================
// @bsiclass
//=======================================================================================
RelatedInstanceFinder::Results RelatedInstanceFinder::FindAll(ECInstanceKey thisInstanceKey, DirectionFilter direction, BentleyStatus* result) const {
    BeMutexHolder lock(m_mutex);
    Results results;
    std::vector<std::tuple<RelationshipInfo*, ECRelationshipEnd>> relationships;
    if (result) {
        *result = SUCCESS;
    }
    if (SUCCESS != FindRelevantRelationshipInfo(thisInstanceKey.GetClassId(), direction, relationships)){
        if (result)
            *result = ERROR;
        return results;
    }

    if (relationships.empty()) {
        return results;
    }

    for (auto& relationship : relationships){
        const auto relInfo = std::get<0>(relationship);
        const auto relEnd = std::get<1>(relationship);
        const auto otherEnd = relEnd == ECRelationshipEnd_Source ? ECRelationshipEnd_Target : ECRelationshipEnd_Source;
        const auto relInstDir = relEnd == ECRelationshipEnd_Source ? ECRelatedInstanceDirection::Forward : ECRelatedInstanceDirection::Backward;
        auto stmt = relInfo->TryGetQueryStatement(m_ecdb, otherEnd);
        if (stmt == nullptr) {
            if (result)
                *result = ERROR;
            return results;
        }
        stmt->BindId(1, thisInstanceKey.GetInstanceId());
        while(stmt->Step() == BE_SQLITE_ROW) {
            auto otherInstanceKey = ECInstanceKey(stmt->GetValueId<ECClassId>(0), stmt->GetValueId<ECInstanceId>(1));
            auto relationshipId = stmt->GetValueId<ECClassId>(2);
            if (relInstDir == ECRelatedInstanceDirection::Forward) {
                results.emplace_back(thisInstanceKey, otherInstanceKey, relationshipId, ECRelatedInstanceDirection::Forward);
            } else {
                results.emplace_back(otherInstanceKey, thisInstanceKey, relationshipId, ECRelatedInstanceDirection::Backward);
            }
        }
    }
    return results;
}

//=======================================================================================
// @bsiclass
//=======================================================================================
BentleyStatus RelatedInstanceFinder::FindRelevantRelationshipInfo(ECN::ECClassId classId, DirectionFilter direction, std::vector<std::tuple<RelationshipInfo*,ECRelationshipEnd>>& results) const {
    if (m_queryRelationshipStmt == nullptr) {
        // This query only return root relationships skip all derived relationships.
        auto sql = R"(
                SELECT
                    [rc].[RelationshipClassId],
                    [rc].[RelationshipEnd],
                    [pp].[ClassId] [NavClassId],
                    [pp].[Name] [NavProp]
                FROM   [ec_RelationshipConstraint] [rc]
                    JOIN [ec_RelationshipConstraintClass] [rcc] ON [rcc].[ConstraintId] = [rc].[Id]
                    JOIN [ec_ClassMap] [cm] ON [cm].[ClassId] = [rc].[RelationshipClassId]
                    LEFT JOIN [ec_ClassHasBaseClasses] [cbc] ON [cbc].[ClassId] = [rc].[RelationshipClassId]
                    LEFT JOIN [ec_cache_ClassHierarchy] [cch] ON [cch].[BaseClassId] = [rcc].[ClassId]
                            AND [rc].[IsPolymorphic]
                    LEFT JOIN [ec_Property] [pp] ON [pp].[NavigationRelationshipClassId] = [rc].[RelationshipClassId]
                WHERE  ? = COALESCE ([cch].[ClassId], [rcc].[ClassId])
                        AND [cbc].[BaseClassId] IS NULL
                ORDER  BY
                        [MapStrategy],
                        [RelationshipClassId],
                        [RelationshipEnd];
            )";

        m_queryRelationshipStmt = std::make_unique<Statement>();
        if (m_queryRelationshipStmt->Prepare(m_ecdb, sql) != BE_SQLITE_OK) {
            m_queryRelationshipStmt = nullptr;
            return ERROR;
        }
    }

    results.clear();
    m_queryRelationshipStmt->Reset();
    m_queryRelationshipStmt->ClearBindings();
    m_queryRelationshipStmt->BindId(1, classId);
    while (m_queryRelationshipStmt->Step() == BE_SQLITE_ROW) {
        const auto relationshipClassId = m_queryRelationshipStmt->GetValueId<ECN::ECClassId>(0);
        const auto relationshipEnd = (ECRelationshipEnd)m_queryRelationshipStmt->GetValueInt(1);
        const auto isNav = !m_queryRelationshipStmt->IsColumnNull(2);
        auto it = m_relationshipMap.find(relationshipClassId);
        if (it == m_relationshipMap.end()) {
            const auto navClassId = isNav ? m_queryRelationshipStmt->GetValueId<ECN::ECClassId>(2) : ECN::ECClassId();
            const auto navPropName = isNav ? m_queryRelationshipStmt->GetValueText(3) : nullptr;
            const auto rel = m_ecdb.Schemas().GetClass(relationshipClassId)->GetRelationshipClassCP();
            const auto navProp = isNav ? m_ecdb.Schemas().GetClass(navClassId)->GetPropertyP(navPropName)->GetAsNavigationProperty() : nullptr;
            it = m_relationshipMap.insert(std::make_pair(relationshipClassId, std::make_unique<RelationshipInfo>(*rel, navProp))).first;
        }
        if (relationshipEnd == ECRelationshipEnd::ECRelationshipEnd_Source && (((int)direction & (int)DirectionFilter::Forward) == (int)DirectionFilter::Forward))
            results.emplace_back(std::tuple<RelationshipInfo*,ECRelationshipEnd>(it->second.get(), relationshipEnd));
        else if (relationshipEnd == ECRelationshipEnd::ECRelationshipEnd_Target && (((int)direction & (int)DirectionFilter::Backward) == (int)DirectionFilter::Backward))
            results.emplace_back(std::tuple<RelationshipInfo*,ECRelationshipEnd>(it->second.get(), relationshipEnd));
    }
    return SUCCESS;
}

END_BENTLEY_SQLITE_EC_NAMESPACE