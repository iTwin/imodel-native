/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/BeBriefcaseBasedIdSequence.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                07/2014
//+---------------+---------------+---------------+---------------+---------------+-
DbResult BeBriefcaseBasedIdSequence::Initialize() const
    {
    const DbResult stat = m_db.GetBLVCache().Register(m_briefcaseLocalValueIndex, m_briefcaseLocalValueName.c_str());
    if (stat != BE_SQLITE_OK)
        {
        LOG.errorv("Could not register BriefcaseLocalValue for BeBriefcaseBasedIdSequence '%s'. The sequence was already registered with file '%s'.", m_briefcaseLocalValueName.c_str(), m_db.GetDbFileName());
        BeAssert(false && "Could not register BriefcaseLocalValue for BeBriefcaseBasedIdSequence. Sequence was already registered before.");
        }

    return stat;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2013
//+---------------+---------------+---------------+---------------+---------------+-
DbResult BeBriefcaseBasedIdSequence::Reset(BeBriefcaseId briefcaseId) const
    {
    if (m_db.IsReadonly())
        return BE_SQLITE_READONLY;

    //set the sequence start value (first id generated should be 1 for the given repo id.
    //Therefore call GetValueUnchecked as the stored last value is not a valid id yet.
    const BeBriefcaseBasedId initialId(briefcaseId, 0);
    const DbResult stat = m_db.GetBLVCache().SaveValue(m_briefcaseLocalValueIndex, initialId.GetValueUnchecked());
    if (stat != BE_SQLITE_OK)
        {
        LOG.errorv("Could not save initial sequence value or BeBriefcaseBasedIdSequence '%s' in file '%s'.", m_briefcaseLocalValueName.c_str(), m_db.GetDbFileName());
        BeAssert(false && "BeBriefcaseBasedIdSequence::Reset could not save initial sequence value in be_Local via SaveBriefcaseLocalValue.");
        }

    return stat;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2013
//+---------------+---------------+---------------+---------------+---------------+-
DbResult BeBriefcaseBasedIdSequence::GetNextInt64Value(uint64_t& nextValue) const
    {
    if (m_db.IsReadonly())
        return BE_SQLITE_READONLY;

    uint64_t deserializedLastValue = INT64_C(0);
    const DbResult stat = m_db.GetBLVCache().IncrementValue(deserializedLastValue, m_briefcaseLocalValueIndex);
    if (stat != BE_SQLITE_OK)
        {
        LOG.fatalv("Could not increment sequence value for BeBriefcaseBasedIdSequence '%s'.", m_briefcaseLocalValueName.c_str());
        BeAssert(false && "BeBriefcaseBasedIdSequence::GetNextValue could not increment sequence value from be_Local via IncrementBriefcaseLocalValueInt64.");
        return stat;
        }

    nextValue = deserializedLastValue;
    return BE_SQLITE_OK;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE