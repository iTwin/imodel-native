/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <WebServices/Cache/Util/IExtendedDataAdapter.h>

#ifdef USE_GTEST
BEGIN_BENTLEY_WEBSERVICES_NAMESPACE
using namespace ::testing;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockExtendedDataAdapter : IExtendedDataAdapter
    {
    MOCK_METHOD1 (GetData, ExtendedData (ECInstanceKeyCR));
    MOCK_METHOD1 (UpdateData, BentleyStatus (ExtendedData&));
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
#endif
