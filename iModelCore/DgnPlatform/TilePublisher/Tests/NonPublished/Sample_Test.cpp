/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/Tests/NonPublished/Sample_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "TestFixture.h"

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/17
//=======================================================================================
struct SampleTestFixture : TestFixture
{

    void TestRectangle();
    void TestTriangle();
    void TestRectangles();
    void TestTriangles();

    template<typename T> void ExecuteTest(WCharCP testName, T testImpl)
        {
        SetupDb(testName);
        testImpl();
        }
};

#define DEFINE_SAMPLE_TEST(MEMBER_FUNC) TEST_F(SampleTestFixture, MEMBER_FUNC) \
    { \
    ExecuteTest(L ## #MEMBER_FUNC , [&]() { MEMBER_FUNC(); }); \
    }

/*---------------------------------------------------------------------------------**//**
* Test a single element represented as a single red rectangle.
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void SampleTestFixture::TestRectangle()
    {
    PhysicalModelPtr model = InsertSpatialModel("MyModel");
    ModelSelectorCPtr modelSel = InsertModelSelector(model->GetModelId());
    ASSERT_TRUE(modelSel.IsValid());

    DgnCategoryId catId = InsertSpatialCategory("MyCategory");
    EXPECT_TRUE(catId.IsValid());

    CategorySelectorCPtr catSel = InsertCategorySelector(catId);
    ASSERT_TRUE(catSel.IsValid());

    DisplayStyle3dCPtr style = InsertDisplayStyle3d("MyStyle", ColorDef::Blue());
    ASSERT_TRUE(style.IsValid());

    auto geom = CreateGeometryBuilder(*model, catId, ColorDef::Red());
    geom->Append(*CreateRectangle(DPoint3d::FromZero(), 10.0, 10.0));

    PhysicalElementCPtr elem = InsertPhysicalElement(*model, *geom, DPoint3d::FromZero());
    ASSERT_TRUE(elem.IsValid());

    AxisAlignedBox3d extents = UpdateProjectExtents();
    SpatialViewDefinitionCPtr view = InsertSpatialView("MyView", *modelSel, *catSel, *style, &extents);
    ASSERT_TRUE(view.IsValid());

    auto status = PublishTiles();
    EXPECT_EQ(status, Cesium::TilesetPublisher::Status::Success);

    AppData appData(GetAppDataFileName());
    AppData expected(*view);
    appData.ExpectEqual(expected);
    }

DEFINE_SAMPLE_TEST(TestRectangle);

