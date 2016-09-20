#include "../BackDoor/PublicApi/BackDoor/RoadRailPhysical/BackDoor.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RoadRailPhysicalTests, BasicSegmentRangeTest)
    {
    DgnDbPtr projectPtr = CreateProject(L"BasicSegmentRangeTest.bim");
    ASSERT_TRUE(projectPtr.IsValid());
    }
