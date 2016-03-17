/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECInstanceId.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

    //buffer must be big enough to store the decimal digits of an int64 plus the sign character plus the trailing \0
    if (stringBufferLength < ECINSTANCEID_STRINGBUFFER_LENGTH)
        return false;

    const uint64_t val = ecInstanceId.GetValue();
    BeStringUtilities::FormatUInt64(stringBuffer, val);
    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                02/2014
//---------------+---------------+---------------+---------------+---------------+------
//static
bool ECInstanceIdHelper::FromString(ECInstanceId& ecInstanceId, Utf8CP ecInstanceIdString)
    {
    //null strings or negative ids are not valid
    if (Utf8String::IsNullOrEmpty(ecInstanceIdString) || ecInstanceIdString[0] == '-')
        return false;

    uint64_t value;
    if (SUCCESS != BeStringUtilities::ParseUInt64(value, ecInstanceIdString))
        return false;

    ecInstanceId = ECInstanceId(value);
    return ecInstanceId.IsValid();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
