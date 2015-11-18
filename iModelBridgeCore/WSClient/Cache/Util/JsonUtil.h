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

    public:
        static void RemoveECMembers(JsonValueR instanceJson);
        static void RemoveECMembers(RapidJsonValueR instanceJson);

        static void ToRapidJson(JsonValueCR source, RapidJsonDocumentR target);
        static void ToJsonValue(RapidJsonValueCR source, JsonValueR target);

        static void DeepCopy(RapidJsonValueCR source, RapidJsonDocumentR target);

        static Utf8String ToStyledString(RapidJsonValueCR value);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
