/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Connect/StubSecureStore.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <BeSecurity/SecureStore.h>
#include <Bentley/Base64Utilities.h>

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct StubSecureStore : public SecurityISecureStore
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
        
    Utf8String Encrypt (Utf8CP value) override
        {
        return Base64Utilities::Encode(value);
        };
        
    Utf8String Decrypt (Utf8CP value) override
        {
        return Base64Utilities::Decode(value);
        };
    };
