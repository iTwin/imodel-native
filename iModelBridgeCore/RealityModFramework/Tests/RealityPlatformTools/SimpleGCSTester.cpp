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
#include <RealityPlatform/SpatioTemporalData.h>
#include <SpatioTemporalSelector/SpatioTemporalSelector.h>
#include <Bentley/BeFileListIterator.h>

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
class SimpleGCSFixture : public testing::Test// : public MockWSGRequestFixture
    {
public:
    WCharCP GetDirectory()
        {
        BeFileName outDir;
        BeTest::GetHost().GetTempDir(outDir);
        outDir.AppendToPath(L"RealityDataServiceConsoleTestDirectory");
        return outDir;
        }

    void InitTestDirectory(WCharCP directoryname)
        {
        if (BeFileName::DoesPathExist(directoryname))
            BeFileName::EmptyAndRemoveDirectory(directoryname);
        BeFileName::CreateNewDirectory(directoryname);
        }
    
    BeFileName GetPemLocation()
        {
        BeFileName outDir;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(outDir);
        outDir.AppendToPath(L"http").AppendToPath(L"cabundle.pem");
        return outDir;
        }
    };

//=====================================================================================
//! @bsimethod                                  Spencer.Mason                  01/2018
//=====================================================================================
TEST_F(SimpleGCSFixture, GCSRequestManagerTest)
    {
    GCSRequestManager::Setup("");

    ASSERT_TRUE(!GeoCoordinationService::GetServerName().empty());
    ASSERT_TRUE(!GeoCoordinationService::GetWSGProtocol().empty());
    ASSERT_EQ(GeoCoordinationService::GetRepoName(), "IndexECPlugin--Server");
    ASSERT_EQ(GeoCoordinationService::GetSchemaName(), "RealityModeling");
    }