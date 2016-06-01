/*--------------------------------------------------------------------------------------+
|
|     $Source: ConnectC/WSLocalState.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnClientFx/DgnClientApp.h>

USING_NAMESPACE_BENTLEY_DGNCLIENTFX

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSLocalState : public ILocalState
    {
    private:
        Json::Value m_map;

    public:
        JsonValueR GetStubMap()
            {
            return m_map;
            }

        void _SaveValue(Utf8CP nameSpace, Utf8CP key, JsonValueCR value) override
            {
            Utf8PrintfString identifier("%s/%s", nameSpace, key);

            if (value.isNull())
                {
                m_map.removeMember(identifier);
                }
            else
                {
                m_map[identifier] = value;
                }
            };

        Json::Value _GetValue(Utf8CP nameSpace, Utf8CP key) const override
            {
            Utf8PrintfString identifier("%s/%s", nameSpace, key);
            return m_map[identifier];
            };
    };