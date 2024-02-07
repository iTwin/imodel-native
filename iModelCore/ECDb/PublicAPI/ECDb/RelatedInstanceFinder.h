/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include <ECDb/ECSqlStatement.h>
#include <ecobjects/ECObjects.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct RelatedInstanceFinder final {
    enum class DirectionFilter {
        Forward = ECN::ECRelatedInstanceDirection::Forward,
        Backward = ECN::ECRelatedInstanceDirection::Backward,
        Both = (int)Forward | (int)Backward,
    };

    struct Result {
        private:
            ECInstanceKey m_from;
            ECInstanceKey m_to;
            ECN::ECClassId m_relationshipClassId;
            ECN::ECRelatedInstanceDirection m_direction;
        public:
            Result(ECInstanceKey const& from, ECInstanceKey const& to, ECN::ECClassId relationshipClassId, ECN::ECRelatedInstanceDirection direction)
                :m_from(from), m_to(to), m_relationshipClassId(relationshipClassId), m_direction(direction){}
            ECInstanceKey const& GetFrom() const { return m_from; }
            ECInstanceKey const& GetTo() const { return m_to; }
            ECN::ECClassId GetRelationshipClassId() const { return m_relationshipClassId; }
            ECN::ECRelatedInstanceDirection GetDirection() const { return m_direction; }
    };
    using Results = std::vector<Result>;

private:
    struct RelationshipInfo {
    private:
        ECN::ECRelationshipClassCR m_relationshipClass;
        ECN::NavigationECPropertyCP m_navProp;
        mutable ECSqlStatement m_querySourceStmt;
        mutable ECSqlStatement m_queryTargetStmt;

    public:
        RelationshipInfo(ECN::ECRelationshipClassCR rel, ECN::NavigationECPropertyCP navProp) : m_relationshipClass(rel), m_navProp(navProp) {}
        ECN::ECRelationshipClassCR GetRelationshipClass() const { return m_relationshipClass; }
        ECN::NavigationECPropertyCP GetNavigationProperty() const { return m_navProp; }
        ECDB_EXPORT ECSqlStatement* TryGetQueryStatement(ECDbCR ecdb, ECN::ECRelationshipEnd end) const;
    };

    ECDbCR m_ecdb;
    mutable std::unique_ptr<Statement> m_queryRelationshipStmt;
    mutable bmap<ECN::ECClassId, std::unique_ptr<RelationshipInfo>> m_relationshipMap;
    mutable BeMutex m_mutex;
    ECDB_EXPORT BentleyStatus FindRelevantRelationshipInfo(ECN::ECClassId classId, DirectionFilter direction, std::vector<std::tuple<RelationshipInfo*, ECN::ECRelationshipEnd>>& results) const;

public:
    explicit RelatedInstanceFinder(ECDbCR ecdb):m_ecdb(ecdb){}
    RelatedInstanceFinder(RelatedInstanceFinder const&) = delete;
    RelatedInstanceFinder& operator=(RelatedInstanceFinder const&) = delete;
    ECDB_EXPORT std::vector<Result> FindAll(ECInstanceKey instanceKey, DirectionFilter direction, BentleyStatus* result = nullptr) const;
};

END_BENTLEY_SQLITE_EC_NAMESPACE