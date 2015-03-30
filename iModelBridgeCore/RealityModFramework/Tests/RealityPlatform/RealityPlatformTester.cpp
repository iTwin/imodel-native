//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/RealityPlatform/RealityPlatformTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <Bentley/BeTest.h>
#include <RealityPlatform/RealityPlatformAPI.h>

USING_BENTLEY_NAMESPACE_REALITYPLATFORM


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
class PlatformTestFixture : public testing::Test 
{
public:
    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  3/2015
    //----------------------------------------------------------------------------------------
    BeFileName BuildOutputFilename(WCharCP filename)
        {
        BeFileName outputFilePath;
        BeTest::GetHost().GetOutputRoot (outputFilePath);
        outputFilePath.AppendToPath(filename);

        return outputFilePath;
        }
};

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
TEST_F (PlatformTestFixture, dummy)
{
    ASSERT_EQ(1/*expected*/, 1/*actual*/);    
}
