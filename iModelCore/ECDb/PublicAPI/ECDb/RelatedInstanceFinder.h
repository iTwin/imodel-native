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
    friend struct StandaloneIterator;
    enum class DirectionFilter {
        Forward = (int)ECN::ECRelatedInstanceDirection::Forward,
        Backward = (int)ECN::ECRelatedInstanceDirection::Backward,
        Both = (int)Forward | (int)Backward,
    };

    struct Result {
        private:
            ECInstanceKey m_key;
            ECN::ECClassId m_relationshipClassId;
            ECN::ECRelatedInstanceDirection m_direction;


        public:
            Result(ECInstanceKey const& key, ECN::ECClassId relationshipClassId, ECN::ECRelatedInstanceDirection direction)
                :m_key(key), m_relationshipClassId(relationshipClassId), m_direction(direction){}
            Result() { *this = Empty(); }
            ECInstanceKey const& GetRelatedKey() const { return m_key; }
            ECN::ECClassId GetRelationshipClassId() const { return m_relationshipClassId; }
            ECN::ECRelatedInstanceDirection GetDirection() const { return m_direction; }
            bool IsValid() const { return m_key.IsValid() && m_relationshipClassId.IsValid(); }
            static Result Empty();
    };
    struct StandaloneIterator {
        private:
            using RelationshipVector = std::vector<std::tuple<std::unique_ptr<ECSqlStatement>, ECN::ECRelatedInstanceDirection>>;
            struct InternalState {
                public:
                InternalState(){}
                RelationshipVector m_relationships;
                RelationshipVector::const_iterator m_cur;
                ECN::ECClassId m_classId;
                ECInstanceId m_argId;
                DirectionFilter m_argDirection;
                Result m_result;
                bool m_init;
            };
            std::unique_ptr<InternalState> m_state;

            bool PrepareNextRelationship();

        public:
            StandaloneIterator(){}
            StandaloneIterator(StandaloneIterator const&) = delete;
            StandaloneIterator& operator=(StandaloneIterator const&) = delete;
            ECDB_EXPORT StandaloneIterator(StandaloneIterator&&);
            ECDB_EXPORT StandaloneIterator& operator=(StandaloneIterator&&);
            ECDB_EXPORT bool Find(ECInstanceId id, DirectionFilter direction);
            ECDB_EXPORT void Reset();
            ECDB_EXPORT bool Step();
            ECDB_EXPORT bool Eof() const;
            ECN::ECClassId GetClassId() const { return m_state != nullptr ? m_state->m_classId : ECN::ECClassId(0ull); };
            Result GetResult() const { return m_state != nullptr ? m_state->m_result : Result::Empty(); };

            ECDB_EXPORT static StandaloneIterator Make(RelatedInstanceFinder const& finder, ECN::ECClassId id);
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