/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/BeBriefcaseBasedIdSequence.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2013
//+---------------+---------------+---------------+---------------+---------------+--
BeBriefcaseBasedIdSequence::BeBriefcaseBasedIdSequence (Db& db, Utf8CP briefcaseLocalValueName)
: m_db (db), m_briefcaseLocalValueName (briefcaseLocalValueName)
    {
    BeAssert (!Utf8String::IsNullOrEmpty (briefcaseLocalValueName));
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2013
//+---------------+---------------+---------------+---------------+---------------+-
BeBriefcaseBasedIdSequence::~BeBriefcaseBasedIdSequence ()
    {
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                07/2014
//+---------------+---------------+---------------+---------------+---------------+-
DbResult BeBriefcaseBasedIdSequence::Initialize () const
    {
    const auto stat = m_db.GetRLVCache().Register(m_briefcaseLocalValueIndex, m_briefcaseLocalValueName.c_str ());
    if (stat != BE_SQLITE_OK)
        {
        LOG.fatalv ("Could not register BriefcaseLocalValue for BeBriefcaseBasedIdSequence '%s'.", m_briefcaseLocalValueName.c_str ());
        BeAssert (false && "Could not register BriefcaseLocalValue for BeBriefcaseBasedIdSequence");
        }

    return stat;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2013
//+---------------+---------------+---------------+---------------+---------------+-
DbResult BeBriefcaseBasedIdSequence::Reset (BeBriefcaseId briefcaseId) const
    {
    if (m_db.IsReadonly ())
        return BE_SQLITE_READONLY;

    //set the sequence start value (first id generated should be 1 for the given repo id.
    //Therefore call GetValueUnchecked as the stored last value is not a valid id yet.
    const BeBriefcaseBasedId initialId (briefcaseId, 0);
    auto stat = m_db.GetRLVCache().SaveValue (m_briefcaseLocalValueIndex, initialId.GetValueUnchecked ());
    if (stat != BE_SQLITE_OK)
        {
        LOG.fatalv ("Could not save initial sequence value SaveBriefcaseLocalValue for BeBriefcaseBasedIdSequence '%s'.", m_briefcaseLocalValueName.c_str ());
        BeAssert (false && "BeBriefcaseBasedIdSequence::Reset could not save initial sequence value in be_Local via SaveBriefcaseLocalValue.");
        }

    return stat;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2013
//+---------------+---------------+---------------+---------------+---------------+-
DbResult BeBriefcaseBasedIdSequence::GetNextInt64Value (uint64_t& nextValue) const
    {
    if (m_db.IsReadonly ())
        return BE_SQLITE_READONLY;

    uint64_t deserializedLastValue = 0LL;
    DbResult stat = m_db.GetRLVCache().IncrementValue (deserializedLastValue, m_briefcaseLocalValueIndex);
    if (stat != BE_SQLITE_OK)
        {
        LOG.fatalv ("Could not increment sequence value for BeBriefcaseBasedIdSequence '%s'.", m_briefcaseLocalValueName.c_str ());
        BeAssert (false && "BeBriefcaseBasedIdSequence::GetNextValue could not increment sequence value from be_Local via IncrementBriefcaseLocalValueInt64.");
        return stat;
        }

    nextValue = deserializedLastValue;
    return BE_SQLITE_OK;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
