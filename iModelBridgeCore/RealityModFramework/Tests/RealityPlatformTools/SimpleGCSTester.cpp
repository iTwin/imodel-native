//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/RealityPlatformTools/SimpleGCSTester.cpp $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <Bentley/BeTest.h>
#include "../../RealityPlatformTools/RealityDataServiceInternal.h"
#include "../Common/RealityModFrameworkTestsCommon.h"
#include <RealityPlatformTools/SimpleGCSApi.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

using ::testing::NiceMock;
using ::testing::_;
using ::testing::Eq;
using ::testing::Invoke;
using ::testing::StrEq;
using ::testing::HasSubstr;
using ::testing::Matcher;
using ::testing::Mock;
using ::testing::Return;

//=====================================================================================
//! @bsiclass                                   Spencer.Mason                  01/2018
//! SimpleGCSFixture
//=====================================================================================
class SimpleGCSFixture : public MockWSGRequestFixture
    {
public:
    static void SetUpTestCase()
        {
        //s_mockWSGInstance = new MockWSGRequest();
        }

    static void TearDownTestCase()
        {
        /*delete s_mockWSGInstance;
        s_mockWSGInstance = nullptr;*/
        }
    };

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  10/2017
//=====================================================================================
TEST_F(SimpleGCSFixture, GCSRequestManagerTest)
    {
    GCSRequestManager::Setup("");

    ASSERT_TRUE(!GeoCoordinationService::GetServerName().empty());
    ASSERT_TRUE(!GeoCoordinationService::GetWSGProtocol().empty());
    ASSERT_EQ(GeoCoordinationService::GetRepoName(), "IndexECPlugin--Server");
    ASSERT_EQ(GeoCoordinationService::GetSchemaName(), "RealityModeling");
    }
