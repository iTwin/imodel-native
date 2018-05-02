#include <Bentley/BeTest.h>
#include <DgnPlatform/UnitTests/DgnDbTestUtils.h>
#include <DgnPlatform/UnitTests/ScopedDgnHost.h>
#include <ClassificationSystems/ClassificationSystemsApi.h>
#include <DgnPlatform/DgnPlatformAPI.h>
#include "ClassificationSystemsTestsBase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_CLASSIFICATIONSYSTEMS


//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                04/2018
//---------------+---------------+---------------+---------------+---------------+------
struct ClassificationSystemsTest : public ClassificationSystemsTestsBase {
        ClassificationSystemsTest() {};
        ~ClassificationSystemsTest() {};

        
    };

//--------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius                04/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ClassificationSystemsTest, TestForTest)
    {
    ASSERT_TRUE(true); 
    }