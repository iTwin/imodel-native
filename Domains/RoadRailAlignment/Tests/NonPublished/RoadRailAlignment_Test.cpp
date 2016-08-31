#include "../BackDoor/PublicApi/BackDoor/RoadRailAlignment/BackDoor.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RoadRailAlignmentTests, BasicAlignmentTest)
    {
    DgnModelId modelId;
    DgnViewId viewId;
    DgnDbPtr projectPtr = CreateProject(L"BasicAlignmentTest.bim", modelId, viewId);
    }