/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Licensing/Utils/InMemoryJsonLocalState.h>

USING_NAMESPACE_BENTLEY_LICENSING

Utf8String GetKey(Utf8CP nameSpace, Utf8CP key)
    {
    return Utf8PrintfString("%s:%s", nameSpace, key);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InMemoryJsonLocalState::_SaveValue(Utf8CP nameSpace, Utf8CP key, Utf8StringCR value)
    {
    m_state[GetKey(nameSpace, key)] = value;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String InMemoryJsonLocalState::_GetValue(Utf8CP nameSpace, Utf8CP key) const
    {
    auto it = m_state.find(GetKey(nameSpace, key));
    if (it == m_state.end())
        return nullptr;

    return it->second;
    }
