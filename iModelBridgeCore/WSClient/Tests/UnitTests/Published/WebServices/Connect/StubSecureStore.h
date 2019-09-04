/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <BeSecurity/SecureStore.h>
#include <Bentley/Base64Utilities.h>

USING_NAMESPACE_BENTLEY_SECURITY

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct StubSecureStore : ISecureStore
    {
    Json::Value values;

    void SaveValue (Utf8CP nameSpace, Utf8CP key, Utf8CP value) override
        {
        values[nameSpace][key] = value;
        };

    Utf8String LoadValue (Utf8CP nameSpace, Utf8CP key) override
        {
        return values[nameSpace][key].asString ();
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
