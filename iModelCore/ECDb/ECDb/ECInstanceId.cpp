/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECInstanceId.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECDb/ECInstanceId.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*****************************************************************************************
// ECInstanceIdHelper
//*****************************************************************************************

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                02/2014
//---------------+---------------+---------------+---------------+---------------+------
//static
bool ECInstanceIdHelper::ToString (Utf8P stringBuffer, size_t stringBufferLength, ECInstanceId const& ecInstanceId)
    {
    if (!ecInstanceId.IsValid ())
        return false;

    //buffer must be big enough to store the decimal digits of an UInt64 plus the trailing \0
    if (stringBufferLength < ECINSTANCEID_STRINGBUFFER_LENGTH)
        return false;

    BeStringUtilities::FormatUInt64 (stringBuffer, (uint64_t) ecInstanceId.GetValue ());
    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                02/2014
//---------------+---------------+---------------+---------------+---------------+------
//static
bool ECInstanceIdHelper::FromString (ECInstanceId& ecInstanceId, Utf8CP ecInstanceIdString)
    {
    if (Utf8String::IsNullOrEmpty (ecInstanceIdString))
        return false;

    uint64_t value;
    const auto stat = BeStringUtilities::ParseUInt64 (value, ecInstanceIdString);
    if (SUCCESS == stat)
        {
        ecInstanceId = ECInstanceId (value);
        return ecInstanceId.IsValid ();
        }

    return false;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
