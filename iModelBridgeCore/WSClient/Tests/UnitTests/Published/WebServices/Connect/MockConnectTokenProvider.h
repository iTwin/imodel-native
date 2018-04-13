/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Connect/MockConnectTokenProvider.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <WebServices/Connect/IConnectTokenProvider.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE
using namespace ::testing;

#if defined (USE_GTEST)
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockConnectTokenProvider : public IConnectTokenProvider
    {
    public:
        MockConnectTokenProvider()
            {
            ON_CALL(*this, GetToken()).WillByDefault(Return(nullptr));
            ON_CALL(*this, UpdateToken()).WillByDefault(Return(CreateCompletedAsyncTask(ISecurityTokenPtr())));
            }
        MOCK_METHOD0(UpdateToken, AsyncTaskPtr<ISecurityTokenPtr>());
        MOCK_METHOD0(GetToken, ISecurityTokenPtr());
    };
#endif

END_BENTLEY_WEBSERVICES_NAMESPACE
