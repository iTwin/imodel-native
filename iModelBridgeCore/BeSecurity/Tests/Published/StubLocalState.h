/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/StubLocalState.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <BeSecurity/SecureStore.h>
#include <Bentley/bmap.h>
USING_NAMESPACE_BENTLEY_SECURITY

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct StubLocalState : public ILocalState
    {
    private:
        bmap<Utf8String, Utf8String> m_map;

    public:
        bmap<Utf8String, Utf8String> GetStubMap ()
            {
            return m_map;
            }

        void _SaveValue (Utf8CP nameSpace, Utf8CP key, Utf8StringCR value) override
            {
            Utf8PrintfString identifier ("%s/%s", nameSpace, key);
            if (value=="null")
                {
                m_map.erase(identifier);
                }
            else
                {
                m_map[identifier] = value;
                }
            };

        Utf8String _GetValue (Utf8CP nameSpace, Utf8CP key) const override
            {
            Utf8PrintfString identifier ("%s/%s", nameSpace, key);
            auto iterator = m_map.find(identifier);
            if (iterator==m_map.end())
                {
                return "";
                }
            else
                {
                return iterator->second;
                }
            };
    };