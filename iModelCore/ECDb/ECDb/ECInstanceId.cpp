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

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                02/2014
//---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECInstanceId::FromString(ECInstanceId& ecInstanceId, Utf8CP ecInstanceIdString)
    {
    //null strings or negative ids are not valid
    if (Utf8String::IsNullOrEmpty(ecInstanceIdString) || ecInstanceIdString[0] == '-')
        return ERROR;

    uint64_t value;
    if (SUCCESS != BeStringUtilities::ParseUInt64(value, ecInstanceIdString))
        return ERROR;

    ecInstanceId = ECInstanceId(value);
    return ecInstanceId.IsValid() ? SUCCESS : ERROR;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
