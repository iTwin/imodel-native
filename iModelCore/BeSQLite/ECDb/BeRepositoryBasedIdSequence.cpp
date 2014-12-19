/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/BeRepositoryBasedIdSequence.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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
    const auto stat = m_db.RegisterRepositoryLocalValue (m_repositoryLocalValueIndex, m_repositoryLocalValueName.c_str ());
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
    auto stat = m_db.SaveRepositoryLocalValue (m_repositoryLocalValueIndex, initialId.GetValueUnchecked ());
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
DbResult BeRepositoryBasedIdSequence::GetNextInt64Value (Int64& nextValue) const
    {
    if (m_db.IsReadonly ())
        return BE_SQLITE_READONLY;

    Int64 deserializedLastValue = -1LL;
    DbResult stat = m_db.IncrementRepositoryLocalValue (deserializedLastValue, m_repositoryLocalValueIndex);
    if (stat != BE_SQLITE_OK)
        {
        LOG.fatalv ("Could not increment sequence value for BeRepositoryBasedIdSequence '%s'.", m_repositoryLocalValueName.c_str ());
        BeAssert (false && "BeRepositoryBasedIdSequence::GetNextValue could not increment sequence value from be_Local via IncrementRepositoryLocalValueInt64.");
        return stat;
        }

    nextValue = deserializedLastValue;
    return BE_SQLITE_OK;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2014
//+---------------+---------------+---------------+---------------+---------------+-
bool BeRepositoryBasedIdSequence::TryClearCache () const
    {
    //only need to clear cache if the DB is open. If it is closed, the cache has already been cleared.
    if (m_db.IsDbOpen ())
        return m_db.TryClearRepositoryLocalValueCache (m_repositoryLocalValueIndex);

    return true;
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
