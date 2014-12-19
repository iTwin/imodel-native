/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECInstanceId.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <BeSQLite/ECDb/ECInstanceId.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*****************************************************************************************
// ECInstanceIdHelper
//*****************************************************************************************

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                02/2014
//---------------+---------------+---------------+---------------+---------------+------
//static
bool ECInstanceIdHelper::ToString (WCharP stringBuffer, size_t stringBufferLength, ECInstanceId const& ecInstanceId)
    {
    if (!ecInstanceId.IsValid ())
        return false;

    //buffer must be big enough to store the decimal digits of an UInt64 plus the trailing \0
    if (stringBufferLength < ECINSTANCEID_STRINGBUFFER_LENGTH)
        return false;

    BeStringUtilities::FormatUInt64 (stringBuffer, (UInt64) ecInstanceId.GetValue ());
    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                02/2014
//---------------+---------------+---------------+---------------+---------------+------
//static
bool ECInstanceIdHelper::FromString (ECInstanceId& ecInstanceId, WCharCP ecInstanceIdString)
    {
    if (WString::IsNullOrEmpty (ecInstanceIdString))
        return false;

    UInt64 value;
    const auto stat = BeStringUtilities::ParseUInt64 (value, ecInstanceIdString);
    if (SUCCESS == stat)
        {
        ecInstanceId = ECInstanceId (value);
        return ecInstanceId.IsValid ();
        }

    return false;
    }

//*****************************************************************************************
// ECInstanceIdSet
//*****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   04/14
//+---------------+---------------+---------------+---------------+---------------+------
bool ECInstanceIdSet::_IsInSet (int nVals, DbValue const* vals) const
    {
    BeAssert (nVals == 1);
    return this->end () != this->find (ECInstanceId (vals[0].GetValueInt64 ()));
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
