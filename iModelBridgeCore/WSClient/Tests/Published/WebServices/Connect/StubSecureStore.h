/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Connect/StubSecureStore.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <MobileDgn/Utils/SecureStore.h>

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct StubSecureStore : public ISecureStore
    {
    private:
        Json::Value m_map;

    public:
        JsonValueR GetStubMap ()
            {
            return m_map;
            }

        void SaveValue (Utf8CP nameSpace, Utf8CP key, Utf8CP value) override
            {
            m_map[nameSpace][key] = value;
            };

        Utf8String LoadValue (Utf8CP nameSpace, Utf8CP key) override
            {
            return m_map[nameSpace][key].asString ();
            };
    };
