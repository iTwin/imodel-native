/*--------------------------------------------------------------------------------------+
|
|     $Source: BeBriefcaseBasedIdSequence.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BeSQLite/BeBriefcaseBasedIdSequence.h>
#include <Logging/bentleylogging.h>

#define LOG (*NativeLogging::LoggingManager::GetLogger(L"BeSQLite"))

BEGIN_BENTLEY_SQLITE_NAMESPACE

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2017
//+---------------+---------------+---------------+---------------+---------------+-
BeBriefcaseBasedIdSequence& BeBriefcaseBasedIdSequence::operator=(BeBriefcaseBasedIdSequence&& rhs)
    {
    if (this != &rhs)
        {
        m_db = std::move(rhs.m_db);
        m_briefcaseLocalValueName = std::move(rhs.m_briefcaseLocalValueName);
        m_briefcaseLocalValueIndex = std::move(rhs.m_briefcaseLocalValueIndex);
        }

    return *this;
    }
//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                07/2014
//+---------------+---------------+---------------+---------------+---------------+-
DbResult BeBriefcaseBasedIdSequence::Initialize() const
    {
    const DbResult stat = GetDb().GetBLVCache().Register(m_briefcaseLocalValueIndex, m_briefcaseLocalValueName);
    if (stat != BE_SQLITE_OK)
        {
        LOG.errorv("Could not register BriefcaseLocalValue for BeBriefcaseBasedIdSequence '%s'. The sequence was already registered with file '%s'.", m_briefcaseLocalValueName, GetDb().GetDbFileName());
        BeAssert(false && "Could not register BriefcaseLocalValue for BeBriefcaseBasedIdSequence. Sequence was already registered before.");
        }

    return stat;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2013
//+---------------+---------------+---------------+---------------+---------------+-
DbResult BeBriefcaseBasedIdSequence::Reset(uint64_t newValue) const
    {
    if (GetDb().IsReadonly())
        return BE_SQLITE_READONLY;

    const DbResult stat = GetDb().GetBLVCache().SaveValue(m_briefcaseLocalValueIndex, newValue);
    if (stat != BE_SQLITE_OK)
        {
        LOG.errorv("Could not save sequence value or BeBriefcaseBasedIdSequence '%s' in file '%s'.", m_briefcaseLocalValueName, GetDb().GetDbFileName());
        BeAssert(false && "BeBriefcaseBasedIdSequence::Reset could not save sequence value in be_Local via SaveBriefcaseLocalValue.");
        }

    return stat;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2013
//+---------------+---------------+---------------+---------------+---------------+-
DbResult BeBriefcaseBasedIdSequence::GetNextInt64Value(uint64_t& nextValue) const
    {
    if (GetDb().IsReadonly())
        return BE_SQLITE_READONLY;

    uint64_t deserializedLastValue = INT64_C(0);
    const DbResult stat = GetDb().GetBLVCache().IncrementValue(deserializedLastValue, m_briefcaseLocalValueIndex);
    if (stat != BE_SQLITE_OK)
        {
        LOG.fatalv("Could not increment sequence value for BeBriefcaseBasedIdSequence '%s'.", m_briefcaseLocalValueName);
        BeAssert(false && "BeBriefcaseBasedIdSequence::GetNextValue could not increment sequence value from be_Local via IncrementBriefcaseLocalValueInt64.");
        return stat;
        }

    BeBriefcaseBasedId lastId(GetDb().GetBriefcaseId().GetNextBriefcaseId(), 0);
    if (deserializedLastValue >= lastId.GetValueUnchecked())
        {
        BeAssert(false && "Ran out of Ids for briefcase");
        return BE_SQLITE_TOOBIG;
        }

    nextValue = deserializedLastValue;
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2017
//+---------------+---------------+---------------+---------------+---------------+------
BeBriefcaseBasedIdSequenceManager::BeBriefcaseBasedIdSequenceManager(DbR db, bvector<Utf8CP> const& sequenceNames) : m_db(db)
    {
    for (Utf8CP sequenceName : sequenceNames)
        {
        m_sequences.push_back(BeBriefcaseBasedIdSequence(db, sequenceName));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2017
//+---------------+---------------+---------------+---------------+---------------+------
DbResult BeBriefcaseBasedIdSequenceManager::InitializeSequences() const
    {
    for (BeBriefcaseBasedIdSequence const& sequence : m_sequences)
        {
        const DbResult stat = sequence.Initialize();
        if (stat != BE_SQLITE_OK)
            return stat;
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2017
//+---------------+---------------+---------------+---------------+---------------+------
DbResult BeBriefcaseBasedIdSequenceManager::ResetSequences(BeBriefcaseId* repoId) const
    {
    BeBriefcaseId actualRepoId = repoId != nullptr ? *repoId : m_db.GetBriefcaseId();

    BeBriefcaseBasedId firstId(actualRepoId, 0);
    
    for (BeBriefcaseBasedIdSequence const& sequence : m_sequences)
        {
        const DbResult stat = sequence.Reset(firstId.GetValueUnchecked());
        if (stat != BE_SQLITE_OK)
            return stat;
        }

    return BE_SQLITE_OK;
    }

END_BENTLEY_SQLITE_NAMESPACE
