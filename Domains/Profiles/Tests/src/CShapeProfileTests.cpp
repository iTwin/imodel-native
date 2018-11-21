#include "ProfilesTestCase.h"
#include <Profiles/ProfilesApi.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_PROFILES


/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct CShapeProfileTestCase : ProfilesTestCase
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, Insert_EmptyCreateParams_FailToInsert)
    {
    CShapeProfile::CreateParams createParams (GetModel());

    CShapeProfilePtr profilePtr = CShapeProfile::Create (createParams);
    ASSERT_TRUE (profilePtr.IsValid());

    DgnDbStatus insertStatus;
    profilePtr->Insert (&insertStatus);
    EXPECT_TRUE (insertStatus != DgnDbStatus::Success);
    }
