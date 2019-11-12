/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <WebServices/Connect/IConnectTokenProvider.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE
using namespace ::testing;

#if defined (USE_GTEST)
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockConnectTokenProvider : IConnectTokenProvider
    {
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
