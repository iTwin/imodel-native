/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/BeRepositoryBasedIdSequence.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2013
//+---------------+---------------+---------------+---------------+---------------+--
BeRepositoryBasedIdSequence::BeRepositoryBasedIdSequence (Db& db, Utf8CP repositoryLocalValueName)
: m_db (db), m_repositoryLocalValueName (repositoryLocalValueName)
    {
    BeAssert (!Utf8String::IsNullOrEmpty (repositoryLocalValueName));
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2013
//+---------------+---------------+---------------+---------------+---------------+-
BeRepositoryBasedIdSequence::~BeRepositoryBasedIdSequence ()
    {
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                07/2014
//+---------------+---------------+---------------+---------------+---------------+-
DbResult BeRepositoryBasedIdSequence::Initialize () const
    {
    const auto stat = m_db.GetRLVCache().Register(m_repositoryLocalValueIndex, m_repositoryLocalValueName.c_str ());
    if (stat != BE_SQLITE_OK)
        {
        LOG.fatalv ("Could not register RepositoryLocalValue for BeRepositoryBasedIdSequence '%s'.", m_repositoryLocalValueName.c_str ());
        BeAssert (false && "Could not register RepositoryLocalValue for BeRepositoryBasedIdSequence");
        }

    return stat;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2013
//+---------------+---------------+---------------+---------------+---------------+-
DbResult BeRepositoryBasedIdSequence::Reset (BeRepositoryId repositoryId) const
    {
    if (m_db.IsReadonly ())
        return BE_SQLITE_READONLY;

    //set the sequence start value (first id generated should be 1 for the given repo id.
    //Therefore call GetValueUnchecked as the stored last value is not a valid id yet.
    const BeRepositoryBasedId initialId (repositoryId, 0);
    auto stat = m_db.GetRLVCache().SaveValue (m_repositoryLocalValueIndex, initialId.GetValueUnchecked ());
    if (stat != BE_SQLITE_OK)
        {
        LOG.fatalv ("Could not save initial sequence value SaveRepositoryLocalValue for BeRepositoryBasedIdSequence '%s'.", m_repositoryLocalValueName.c_str ());
        BeAssert (false && "BeRepositoryBasedIdSequence::Reset could not save initial sequence value in be_Local via SaveRepositoryLocalValue.");
        }

    return stat;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2013
//+---------------+---------------+---------------+---------------+---------------+-
DbResult BeRepositoryBasedIdSequence::GetNextInt64Value (uint64_t& nextValue) const
    {
    if (m_db.IsReadonly ())
        return BE_SQLITE_READONLY;

    uint64_t deserializedLastValue = 0LL;
    DbResult stat = m_db.GetRLVCache().IncrementValue (deserializedLastValue, m_repositoryLocalValueIndex);
    if (stat != BE_SQLITE_OK)
        {
        LOG.fatalv ("Could not increment sequence value for BeRepositoryBasedIdSequence '%s'.", m_repositoryLocalValueName.c_str ());
        BeAssert (false && "BeRepositoryBasedIdSequence::GetNextValue could not increment sequence value from be_Local via IncrementRepositoryLocalValueInt64.");
        return stat;
        }

    nextValue = deserializedLastValue;
    return BE_SQLITE_OK;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
