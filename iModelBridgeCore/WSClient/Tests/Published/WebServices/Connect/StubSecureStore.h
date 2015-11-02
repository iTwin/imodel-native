/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Connect/StubSecureStore.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <DgnClientFx/Utils/SecureStore.h>

USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct StubSecureStore : public ISecureStore
    {
    Json::Value values;
    Json::Value legacyValues;

    void SaveValue (Utf8CP nameSpace, Utf8CP key, Utf8CP value) override
        {
        values[nameSpace][key] = value;
        };

    Utf8String LoadValue (Utf8CP nameSpace, Utf8CP key) override
        {
        return values[nameSpace][key].asString ();
        };

    Utf8String LegacyLoadValue (Utf8CP nameSpace, Utf8CP key) override
        {
        return legacyValues[nameSpace][key].asString ();
        };

    void LegacyClearValue (Utf8CP nameSpace, Utf8CP key) override
        {
        legacyValues[nameSpace][key] = "";
        };
    };
