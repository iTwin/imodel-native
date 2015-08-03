/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/Util/MockExtendedDataAdapter.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <WebServices/Cache/Util/IExtendedDataAdapter.h>

#ifdef USE_GTEST
BEGIN_BENTLEY_WEBSERVICES_NAMESPACE
using namespace ::testing;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockExtendedDataAdapter : public IExtendedDataAdapter
    {
    public:
        MOCK_METHOD1 (GetData, ExtendedData (ECInstanceKeyCR));
        MOCK_METHOD1 (UpdateData, BentleyStatus (ExtendedData&));
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
#endif
