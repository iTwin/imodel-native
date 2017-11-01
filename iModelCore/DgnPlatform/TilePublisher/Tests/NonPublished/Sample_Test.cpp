/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/Tests/NonPublished/Sample_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "TestFixture.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TestFixture, Sample)
    {
    SetupDb(L"Sample");

    DgnModelPtr model = InsertSpatialModel("MyModel");
    ModelSelectorCPtr modelSel = InsertModelSelector(model->GetModelId());
    EXPECT_TRUE(modelSel.IsValid());

    DgnCategoryId catId = InsertSpatialCategory("MyCategory");
    EXPECT_TRUE(catId.IsValid());

    CategorySelectorCPtr catSel = InsertCategorySelector(catId);
    EXPECT_TRUE(catSel.IsValid());

    // Avoid assertions on non-null element ptr...alternatively use ExecuteTest().
    catSel = nullptr;
    }

