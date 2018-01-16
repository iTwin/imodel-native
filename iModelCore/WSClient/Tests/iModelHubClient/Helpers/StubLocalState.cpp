/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Helpers/StubLocalState.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StubLocalState.h"

StubLocalState* StubLocalState::Instance()
    {
    static StubLocalState* s_instance = nullptr;
    if (nullptr == s_instance)
    s_instance = new StubLocalState();
    return s_instance;
    }

bmap<Utf8String, Utf8String> StubLocalState::GetStubMap()
    {
    return m_map;
    }

void StubLocalState::_SaveValue(Utf8CP nameSpace, Utf8CP key, Utf8StringCR value)
    {
    Utf8PrintfString identifier("%s/%s", nameSpace, key);
    if (value == "null")
        {
        m_map.erase(identifier);
        }
    else
        {
        m_map[identifier] = value;
        }
    };

Utf8String StubLocalState::_GetValue(Utf8CP nameSpace, Utf8CP key) const
    {
    Utf8PrintfString identifier("%s/%s", nameSpace, key);
    auto iterator = m_map.find(identifier);
    if (iterator == m_map.end())
        {
        return "";
        }
    else
        {
        return iterator->second;
        }
    };