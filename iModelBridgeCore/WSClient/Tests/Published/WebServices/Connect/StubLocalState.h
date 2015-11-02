/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Connect/StubLocalState.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <DgnClientFx/DgnClientApp.h>

USING_NAMESPACE_BENTLEY_DGNCLIENTFX

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct StubLocalState : public ILocalState
    {
    private:
        Json::Value m_map;

    public:
        JsonValueR GetStubMap ()
            {
            return m_map;
            }

        void SaveValue (Utf8CP nameSpace, Utf8CP key, JsonValueCR value) override
            {
            Utf8PrintfString identifier ("%s/%s", nameSpace, key);

            if (value.isNull ())
                {
                m_map.removeMember (identifier);
                }
            else
                {
                m_map[identifier] = value;
                }
            };

        Json::Value GetValue (Utf8CP nameSpace, Utf8CP key) const override
            {
            Utf8PrintfString identifier ("%s/%s", nameSpace, key);
            return m_map[identifier];
            };
    };