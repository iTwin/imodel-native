/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/BackDoor/PublicAPI/BackDoor/ECPresentation/StubLocalState.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <UnitTests/BackDoor/ECPresentation/ECPresentationTest.h>
#include <BeJsonCpp/BeJsonUtilities.h>

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct StubLocalState : IJsonLocalState
    {
    private:
        bmap<Utf8String, Utf8String> m_map;

    public:
        bmap<Utf8String, Utf8String>& GetStubMap() {return m_map;}

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

END_ECPRESENTATIONTESTS_NAMESPACE