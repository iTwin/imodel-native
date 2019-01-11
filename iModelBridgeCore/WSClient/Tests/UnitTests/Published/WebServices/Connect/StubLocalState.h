/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Connect/StubLocalState.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct StubLocalState : public IJsonLocalState
    {
    private:
        Json::Value m_map;

    public:
        JsonValueR GetStubMap ()
            {
            return m_map;
            }

        void _SaveValue (Utf8CP nameSpace, Utf8CP key, Utf8StringCR value) override
            {
            Utf8PrintfString identifier ("%s/%s", nameSpace, key);

            if (value=="null" || value.empty())
                {
                m_map.removeMember (identifier);
                }
            else
                {
                m_map[identifier] = value;
                }
            };

        Utf8String _GetValue (Utf8CP nameSpace, Utf8CP key) const override
            {
            Utf8PrintfString identifier ("%s/%s", nameSpace, key);
            return m_map.isMember(identifier) ? m_map[identifier].asCString() : "";
            };
    };
