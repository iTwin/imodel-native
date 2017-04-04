/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Connect/MockConnectTokenProvider.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <WebServices/Connect/IConnectTokenProvider.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

#if defined (USE_GTEST)
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockConnectTokenProvider : public IConnectTokenProvider
    {
    public:
        MOCK_METHOD0(UpdateToken, AsyncTaskPtr<SamlTokenPtr>());
        MOCK_METHOD0(GetToken, SamlTokenPtr());
    };
#endif

END_BENTLEY_WEBSERVICES_NAMESPACE
