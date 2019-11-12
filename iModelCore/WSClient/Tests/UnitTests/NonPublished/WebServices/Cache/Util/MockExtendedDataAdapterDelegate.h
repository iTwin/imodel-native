/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <WebServices/Cache/Util/ExtendedDataAdapter.h>

#ifdef USE_GTEST
BEGIN_BENTLEY_WEBSERVICES_NAMESPACE
using namespace ::testing;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockExtendedDataAdapterDelegate : ExtendedDataAdapter::IDelegate
    {
    MockExtendedDataAdapterDelegate()
        {
        ON_CALL(*this, GetHolderKey(_)).WillByDefault(Invoke([] (ECInstanceKeyCR key)
            {
            return key;
            }));
        }
    MOCK_METHOD1(GetExtendedDataClass, ECClassCP(ECInstanceKeyCR ownerKey));
    MOCK_METHOD1(GetExtendedDataRelationshipClass, ECRelationshipClassCP(ECInstanceKeyCR ownerKey));
    MOCK_METHOD1(GetHolderKey, ECInstanceKey(ECInstanceKeyCR ownerKey));
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
#endif
