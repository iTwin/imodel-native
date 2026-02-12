/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECDb/RelatedInstanceFinder.h>
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsimethod
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
            results.emplace_back(otherEnd, relationshipId, direction);
        }
    }
    return results;
}

//=======================================================================================
// @bsimethod
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
// @bsimethod
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

//=======================================================================================
// @bsimethod
//=======================================================================================
RelatedInstanceFinder::StandaloneIterator RelatedInstanceFinder::StandaloneIterator::Make(RelatedInstanceFinder const& finder, ECN::ECClassId classId){
    BeMutexHolder lock(finder.m_mutex);
    std::vector<std::tuple<ECSqlStatement*, ECN::ECRelatedInstanceDirection>> relationships;
    if (SUCCESS != finder.FindRelevantRelationshipInfo(classId, relationships)){
        return StandaloneIterator();
    }

    StandaloneIterator it;
    it.m_state = std::make_unique<InternalState>();
    for (auto& relationship : relationships) {
        const auto stmt = std::get<0>(relationship);
        const auto relDir = std::get<1>(relationship);
        auto stmtCopy = std::make_unique<ECSqlStatement>();
        if (ECSqlStatus::Success != stmtCopy->Prepare(finder.m_ecdb, stmt->GetECSql())){
            return StandaloneIterator();
        }
        it.m_state->m_relationships.push_back(std::make_tuple(std::move(stmtCopy), relDir));
    }
    it.m_state->m_classId = classId;
    it.m_state->m_init = false;
    return it;
}

//=======================================================================================
// @bsimethod
//=======================================================================================
bool RelatedInstanceFinder::StandaloneIterator::Find(ECInstanceId id, DirectionFilter direction) {
    if (m_state == nullptr){
        return false;
    }
    Reset();
    m_state->m_argDirection = direction;
    m_state->m_argId = id;
    return true;
}

//=======================================================================================
// @bsimethod
//=======================================================================================
RelatedInstanceFinder::StandaloneIterator::StandaloneIterator(StandaloneIterator&& rhs): m_state(std::move(rhs.m_state)) {}

//=======================================================================================
// @bsimethod
//=======================================================================================
RelatedInstanceFinder::StandaloneIterator& RelatedInstanceFinder::StandaloneIterator::operator = (StandaloneIterator&& rhs){
    if (this != &rhs)
        m_state = std::move(rhs.m_state);
    return *this;
}

//=======================================================================================
// @bsimethod
//=======================================================================================
bool RelatedInstanceFinder::StandaloneIterator::Eof() const {
    if (m_state == nullptr){
        return false;
    }
    if(!m_state->m_init)
        return false;

    return m_state->m_cur == m_state->m_relationships.end();
}

//=======================================================================================
// @bsimethod
//=======================================================================================
bool RelatedInstanceFinder::StandaloneIterator::PrepareNextRelationship(){
    if (m_state == nullptr){
        return false;
    }

    if (!m_state->m_init) {
        m_state->m_init = true;
        m_state->m_cur = m_state->m_relationships.begin();
    } else {
        ++m_state->m_cur;
    }

    if (m_state->m_cur == m_state->m_relationships.end()) {
        return false;
    }

    auto stmt = std::get<0>(*m_state->m_cur).get();
    auto relDir = std::get<1>(*m_state->m_cur);

    const auto resolveDir = (int)relDir & (int)m_state->m_argDirection;
    if (!resolveDir)
        return PrepareNextRelationship();

    stmt->Reset();
    stmt->ClearBindings();

    const auto isForward = (resolveDir & (int)ECRelatedInstanceDirection::Forward) > 0;
    const auto isBackward = (resolveDir & (int)ECRelatedInstanceDirection::Backward) > 0;

    stmt->BindInt(1, isForward);
    stmt->BindId(2, m_state->m_argId);
    stmt->BindInt(3, isBackward);
    stmt->BindId(4, m_state->m_argId);
    return true;
}

//=======================================================================================
// @bsimethod
//=======================================================================================
RelatedInstanceFinder::Result RelatedInstanceFinder::Result::Empty() {
    static auto s_empty = Result(ECInstanceKey(ECClassId(0ULL), ECInstanceId(0ULL)) , ECClassId(0ULL), ECRelatedInstanceDirection::Forward);
    return s_empty;
}

//=======================================================================================
// @bsimethod
//=======================================================================================
bool RelatedInstanceFinder::StandaloneIterator::Step(){
    if (m_state == nullptr){
        return false;
    }
    m_state->m_result = Result::Empty();
    if (!m_state->m_init) {
        PrepareNextRelationship();
    }
    if (m_state->m_cur == m_state->m_relationships.end())
        return false;

    auto stmt = std::get<0>(*m_state->m_cur).get();
    if(stmt->Step() == BE_SQLITE_ROW) {
        const auto direction = (ECRelatedInstanceDirection)stmt->GetValueInt(0);
        const auto relationshipId = stmt->GetValueId<ECClassId>(1);
        const auto otherEnd = ECInstanceKey(stmt->GetValueId<ECClassId>(3), stmt->GetValueId<ECInstanceId>(2));
        m_state->m_result = Result(otherEnd, relationshipId, direction);
        return true;
    } else {
        if (PrepareNextRelationship())
            return Step();
    }
    return false;
}

//=======================================================================================
// @bsimethod
//=======================================================================================
void RelatedInstanceFinder::StandaloneIterator::Reset(){
    if (m_state == nullptr){
        return;
    }
    m_state->m_init = false;
}
END_BENTLEY_SQLITE_EC_NAMESPACE