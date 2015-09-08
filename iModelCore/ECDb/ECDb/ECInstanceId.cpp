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

    //buffer must be big enough to store the decimal digits of an int64 plus the sign character plus the trailing \0
    if (stringBufferLength < ECINSTANCEID_STRINGBUFFER_LENGTH)
        return false;

    int64_t val = ecInstanceId.GetValue();
    const bool isNegative = val < 0LL;
    Utf8P digitStringBuffer = stringBuffer;
    uint64_t uvalue = val;
    if (isNegative)
        {
        uvalue = -1LL * val;
        digitStringBuffer = stringBuffer + 1;
        }

    BeStringUtilities::FormatUInt64(digitStringBuffer, uvalue);

    if (isNegative)
        stringBuffer[0] = '-';

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

    const bool isNegative = ecInstanceIdString[0] == '-';
    if (isNegative)
        ecInstanceIdString++;

    uint64_t uvalue;
    const auto stat = BeStringUtilities::ParseUInt64(uvalue, ecInstanceIdString);
    if (SUCCESS == stat)
        {
        const int64_t value = isNegative ? (int64_t) (-1LL * uvalue) : (int64_t) uvalue;
        ecInstanceId = ECInstanceId (value);
        return ecInstanceId.IsValid ();
        }

    return false;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
