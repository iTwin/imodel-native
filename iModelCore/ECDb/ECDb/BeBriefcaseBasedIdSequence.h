/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/BeBriefcaseBasedIdSequence.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
//!
//! @bsiclass                                                 Krischan.Eberle     02/2013
//=======================================================================================
struct BeBriefcaseBasedIdSequence : NonCopyableClass
    {
private:
    Db& m_db;
    Utf8String const m_briefcaseLocalValueName;
    mutable size_t m_briefcaseLocalValueIndex;
    DbResult GetNextInt64Value (uint64_t& nextValue) const;

public:
    BeBriefcaseBasedIdSequence (Db& db, Utf8CP briefcaseLocalValueName);
    ~BeBriefcaseBasedIdSequence ();

    DbResult Initialize () const;

    DbResult Reset (BeBriefcaseId briefcaseId) const;

    template <typename TBeBriefcaseBasedId>
    DbResult GetNextValue (TBeBriefcaseBasedId& nextValue) const
        {
        uint64_t nextValueInt = 0LL;
        DbResult stat = GetNextInt64Value (nextValueInt);
        if (stat != BE_SQLITE_OK)
            return stat;

        nextValue = TBeBriefcaseBasedId(nextValueInt);
        return BE_SQLITE_OK;
        }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

