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

    ECDbCR m_ecdb;
    mutable std::unique_ptr<Statement> m_queryRelationshipStmt;
    mutable bmap<ECN::ECClassId, std::unique_ptr<ECSqlStatement>> m_relationshipCachedECSql;
    mutable std::unique_ptr<bmap<ECN::ECClassId, std::vector<std::tuple<ECN::ECClassId, ECN::ECRelatedInstanceDirection>>>> m_relCache;
    mutable BeMutex m_mutex;
    ECDB_EXPORT BentleyStatus FindRelevantRelationshipInfo(ECN::ECClassId classId, std::vector<std::tuple<ECSqlStatement*, ECN::ECRelatedInstanceDirection>>& results) const;
    BentleyStatus InitializeRelationshipCache() const;

public:
    explicit RelatedInstanceFinder(ECDbCR ecdb):m_ecdb(ecdb){}
    RelatedInstanceFinder(RelatedInstanceFinder const&) = delete;
    RelatedInstanceFinder& operator=(RelatedInstanceFinder const&) = delete;
    ECDB_EXPORT std::vector<Result> FindAll(ECInstanceKey instanceKey, DirectionFilter direction, BentleyStatus* result = nullptr) const;
};

END_BENTLEY_SQLITE_EC_NAMESPACE