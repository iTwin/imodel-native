/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Helpers/StubLocalState.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <BeJsonCpp/BeJsonUtilities.h>

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct StubLocalState : public IJsonLocalState
    {
    private:
        bmap<Utf8String, Utf8String> m_map;

    public:
        static StubLocalState* Instance();
        bmap<Utf8String, Utf8String> GetStubMap();
        void _SaveValue(Utf8CP nameSpace, Utf8CP key, Utf8StringCR value) override;
        Utf8String _GetValue(Utf8CP nameSpace, Utf8CP key) const override;
    };