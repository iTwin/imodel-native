/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "BeSQLite.h"
#include <Bentley/NonCopyableClass.h>

BEGIN_BENTLEY_SQLITE_NAMESPACE

//=======================================================================================
//! Sequence that generates sequential BeBriefcaseBasedIds and stores
//! its state in the Db across sessions.
// @bsiclass                                                 Krischan.Eberle     02/2013
//=======================================================================================
struct BeBriefcaseBasedIdSequence final : NonCopyableClass
    {
private:
    Db* m_db;
    Utf8CP m_briefcaseLocalValueName;
    mutable size_t m_briefcaseLocalValueIndex;

    BE_SQLITE_EXPORT DbResult GetNextInt64Value(uint64_t& nextValue) const;
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

    BE_SQLITE_EXPORT DbResult Initialize() const;

    BE_SQLITE_EXPORT DbResult Reset(uint64_t minimumId) const;

    //! Get the next value in the sequence
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

    Utf8CP GetName() const { return m_briefcaseLocalValueName; }
    };

//=======================================================================================
//! A collection of BeBriefcaseBasedIdSequence.
// @bsiclass                                                 Krischan.Eberle     02/2017
//=======================================================================================
struct BeBriefcaseBasedIdSequenceManager final : NonCopyableClass
    {
private:
    DbCR m_db;
    bvector<BeBriefcaseBasedIdSequence> m_sequences;

public:
    BE_SQLITE_EXPORT BeBriefcaseBasedIdSequenceManager(DbR, bvector<Utf8CP> const& sequenceNames);

    //! Gets a sequence from the manager
    //! @param[in] sequenceKey Key of the sequence. It is the index of the sequence name in the vector passed to the constructor.
    //! @return Retrieved sequence
    BeBriefcaseBasedIdSequence const& GetSequence(uint32_t sequenceKey) const { BeAssert(sequenceKey < (uint32_t) m_sequences.size()); return m_sequences[(size_t) sequenceKey]; }

    BE_SQLITE_EXPORT DbResult InitializeSequences() const;
    BE_SQLITE_EXPORT DbResult ResetSequences(BeBriefcaseId* repoId = nullptr) const;
    };

END_BENTLEY_SQLITE_NAMESPACE
