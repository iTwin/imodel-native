/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Util/JsonUtil.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <WebServices/Cache/WebServicesCache.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <rapidjson/BeRapidJson.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct JsonUtil
    {
    private:
        JsonUtil() {};

        static bool StringValuesEqual(RapidJsonValueCR a, RapidJsonValueCR b);
        static bool ArrayValuesEqual(RapidJsonValueCR a, RapidJsonValueCR b);
        static bool ObjectValuesEqual(RapidJsonValueCR a, RapidJsonValueCR b);

    public:
        WSCACHE_EXPORT static void RemoveECMembers(JsonValueR instanceJson);
        WSCACHE_EXPORT static void RemoveECMembers(RapidJsonValueR instanceJson);

        WSCACHE_EXPORT static void ToRapidJson(JsonValueCR source, RapidJsonDocumentR target);
        WSCACHE_EXPORT static void ToJsonValue(RapidJsonValueCR source, JsonValueR target);

        WSCACHE_EXPORT static void DeepCopy(RapidJsonValueCR source, RapidJsonDocumentR target);

        //! Copy values
        //! @param source
        //! @param target
        //! @param allocator - allocator for target values
        //! @param reuseStrings - set to true to reuse srings from source without allocating new ones
        WSCACHE_EXPORT static void CopyValues
            (
            RapidJsonValueCR source,
            RapidJsonValueR target,
            rapidjson::Value::AllocatorType& allocator,
            bool reuseStrings = false
            );

        //! Check if values are equal
        WSCACHE_EXPORT static bool AreValuesEqual(RapidJsonValueCR a, RapidJsonValueCR b);

        WSCACHE_EXPORT static Utf8String ToStyledString(RapidJsonValueCR value);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
