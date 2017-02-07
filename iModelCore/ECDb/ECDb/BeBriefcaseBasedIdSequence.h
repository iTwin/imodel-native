/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/BeBriefcaseBasedIdSequence.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__

#pragma once
#include <Bentley/NonCopyableClass.h>
#include <BeSQLite/BeSQLite.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Sequence that generates sequential BeBriefcaseBasedIds and stores
//! its state in the @ref ECDbFile "ECDb file" across sessions.
// @bsiclass                                                 Krischan.Eberle     02/2013
//=======================================================================================
struct BeBriefcaseBasedIdSequence final : NonCopyableClass
    {
private:
    Db* m_db;
    Utf8CP m_briefcaseLocalValueName;
    mutable size_t m_briefcaseLocalValueIndex;

    DbResult GetNextInt64Value(uint64_t& nextValue) const;
    Db& GetDb() const { return *m_db; }

public:
    BeBriefcaseBasedIdSequence(Db& db, Utf8CP briefcaseLocalValueName) : m_db(&db), m_briefcaseLocalValueName(briefcaseLocalValueName)
        {
        BeAssert(!Utf8String::IsNullOrEmpty(briefcaseLocalValueName));
        }

    ~BeBriefcaseBasedIdSequence() {}

    BeBriefcaseBasedIdSequence(BeBriefcaseBasedIdSequence&& rhs)
        : m_db(std::move(rhs.m_db)), m_briefcaseLocalValueName(std::move(rhs.m_briefcaseLocalValueName)), m_briefcaseLocalValueIndex(std::move(rhs.m_briefcaseLocalValueIndex))
        {}

    BeBriefcaseBasedIdSequence& operator=(BeBriefcaseBasedIdSequence&& rhs);

    DbResult Initialize() const;
    DbResult Reset(BeBriefcaseId briefcaseId) const;

    template <typename TBeBriefcaseBasedId>
    DbResult GetNextValue(TBeBriefcaseBasedId& nextValue) const
        {
        uint64_t nextValueInt = INT64_C(0);
        DbResult stat = GetNextInt64Value(nextValueInt);
        if (stat != BE_SQLITE_OK)
            return stat;

        nextValue = TBeBriefcaseBasedId(nextValueInt);
        return BE_SQLITE_OK;
        }
    };

//=======================================================================================
//! A collection of BeBriefcaseBasedIdSequence.
// @bsiclass                                                 Krischan.Eberle     02/2017
//=======================================================================================
struct BeBriefcaseBasedIdSequenceManager final : NonCopyableClass
    {
private:
    DbCR m_db;
    std::vector<BeBriefcaseBasedIdSequence> m_sequences;

public:
    BeBriefcaseBasedIdSequenceManager(DbR, std::vector<Utf8CP> const& sequenceNames);
    BeBriefcaseBasedIdSequence const& GetSequence(uint32_t sequenceKey) const { BeAssert(sequenceKey < (uint32_t) m_sequences.size()); return m_sequences[(size_t) sequenceKey]; }

    DbResult InitializeSequences() const;
    DbResult ResetSequences(BeBriefcaseId* repoId = nullptr) const;

    };
END_BENTLEY_SQLITE_EC_NAMESPACE

