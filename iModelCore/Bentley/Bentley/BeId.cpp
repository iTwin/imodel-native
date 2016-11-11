/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/BeId.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeId.h>
#include <Bentley/BeStringUtilities.h>

BEGIN_BENTLEY_NAMESPACE

 //--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                11/2016
//---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus BeInt64Id::FromString(BeInt64Id& id, Utf8CP idString)
    {
    //null strings or negative ids are not valid
    if (Utf8String::IsNullOrEmpty(idString) || idString[0] == '-')
        return ERROR;

    uint64_t value;
    if (SUCCESS != BeStringUtilities::ParseUInt64(value, idString))
        return ERROR;

    id = BeInt64Id(value);
    return id.IsValid() ? SUCCESS : ERROR;
    }


END_BENTLEY_NAMESPACE
