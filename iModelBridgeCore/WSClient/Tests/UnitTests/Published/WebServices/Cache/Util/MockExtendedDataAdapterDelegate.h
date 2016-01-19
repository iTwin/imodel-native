/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Cache/Util/MockExtendedDataAdapterDelegate.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <WebServices/Cache/Util/ExtendedDataAdapter.h>

#ifdef USE_GTEST
BEGIN_BENTLEY_WEBSERVICES_NAMESPACE
using namespace ::testing;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockExtendedDataAdapterDelegate : public ExtendedDataAdapter::IDelegate
    {
    public:
        MockExtendedDataAdapterDelegate()
            {
            ON_CALL(*this, GetHolderKey(_)).WillByDefault(Invoke([] (ECInstanceKeyCR key)
                {
                return key;
                }));
            }
        MOCK_METHOD0(GetExtendedDataClass, ECClassCP());
        MOCK_METHOD0(GetExtendedDataRelationshipClass, ECRelationshipClassCP());
        MOCK_METHOD1(GetHolderKey, ECInstanceKey(ECInstanceKeyCR ownerKey));
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
#endif
