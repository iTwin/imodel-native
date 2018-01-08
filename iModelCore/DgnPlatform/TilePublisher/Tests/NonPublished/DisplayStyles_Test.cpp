/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/Tests/NonPublished/DisplayStyles_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "TestFixture.h"

//=======================================================================================
// @bsistruct                                                   Tomas.Sakinis   11/17
//=======================================================================================
struct DisplayStylesTestFixture : TestFixture
    {
    void DisplayStyles();

    template<typename T> void ExecuteTest(WCharCP testName, T testImpl)
        {
        SetupDb(testName);
        testImpl();
        }
    };

#define DEFINE_SAMPLE_TEST(MEMBER_FUNC) TEST_F(DisplayStylesTestFixture, MEMBER_FUNC) \
    { \
    ExecuteTest(L ## #MEMBER_FUNC , [&]() { MEMBER_FUNC(); }); \
    }


/*---------------------------------------------------------------------------------**//**
* Test multiple display styles
* @bsimethod                                                    Tomas.Sakinis   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStylesTestFixture::DisplayStyles()
    {
    // Set up the data to be published...
    PhysicalModelPtr model = InsertSpatialModel("Model1");

    ModelSelectorCPtr modelSel = InsertModelSelector(model->GetModelId());
    ASSERT_TRUE(modelSel.IsValid());

    DgnCategoryId catTriangleId = InsertSpatialCategory("CategoryTriangle");
    EXPECT_TRUE(catTriangleId.IsValid());

    DgnCategoryId catRectangleId = InsertSpatialCategory("CategoryRectangle");
    EXPECT_TRUE(catRectangleId.IsValid());

    CategorySelectorCPtr catTriangleSel = InsertCategorySelector(catTriangleId);
    ASSERT_TRUE(catTriangleSel.IsValid());

    CategorySelectorCPtr catRectangleSel = InsertCategorySelector(catRectangleId);
    ASSERT_TRUE(catRectangleSel.IsValid());

    DgnCategoryIdSet allCategories;
    allCategories.insert(catTriangleId);
    allCategories.insert(catRectangleId);

    CategorySelectorCPtr allCategoriesSel = InsertCategorySelector(allCategories);
    ASSERT_TRUE(allCategoriesSel.IsValid());

    DisplayStyle3dCPtr redStyle = InsertDisplayStyle3d("RedStyle", ColorDef::Red());
    ASSERT_TRUE(redStyle.IsValid());

    DisplayStyle3dCPtr greenStyle = InsertDisplayStyle3d("GreenStyle", ColorDef::Green());
    ASSERT_TRUE(greenStyle.IsValid());

    DisplayStyle3dCPtr blueStyle = InsertDisplayStyle3d("BlueStyle", ColorDef::Blue());
    ASSERT_TRUE(blueStyle.IsValid());

    auto geomTriangle = CreateGeometryBuilder(*model, catTriangleId, ColorDef::Red());
    geomTriangle->Append(*CreateTriangle(DPoint3d::FromZero(), 10.0, 10.0));

    PhysicalElementCPtr elemTriangle = InsertPhysicalElement(*model, *geomTriangle, DPoint3d::FromZero());
    ASSERT_TRUE(elemTriangle.IsValid());
    //---
    auto geomRectangle = CreateGeometryBuilder(*model, catRectangleId, ColorDef::Red());
    geomRectangle->Append(*CreateRectangle(DPoint3d::FromZero(), 10.0, 10.0));

    PhysicalElementCPtr elemRectangle = InsertPhysicalElement(*model, *geomRectangle, DPoint3d::FromZero());
    ASSERT_TRUE(elemRectangle.IsValid());


    AxisAlignedBox3d extents = UpdateProjectExtents();
    SpatialViewDefinitionCPtr redView = InsertSpatialView("RedView", *modelSel, *catTriangleSel, *redStyle, &extents);
    ASSERT_TRUE(redView.IsValid());

    SpatialViewDefinitionCPtr greenView = InsertSpatialView("GreenView", *modelSel, *catRectangleSel, *greenStyle, &extents);
    ASSERT_TRUE(greenView.IsValid());

    SpatialViewDefinitionCPtr blueView = InsertSpatialView("BlueView", *modelSel, *allCategoriesSel, *blueStyle, &extents);
    ASSERT_TRUE(blueView.IsValid());

    // Publish the tileset
    auto status = PublishTiles();
    EXPECT_EQ(status, Cesium::TilesetPublisher::Status::Success);

    // Compare x_AppData.json (metadata about the dgndb)
    AppData appData(GetAppDataFileName());
    AppData expected(*redView);
    expected.AddView(*greenView);
    expected.AddView(*blueView);
    appData.ExpectEqual(expected);

    // Verify files in output directories
    PublishedTilesets tilesets(GetAppDataDir());
    tilesets.ExpectEqual(expected.m_models);
    ASSERT_EQ(tilesets.size(), 1);

    EXPECT_EQ(model->GetModelId(), tilesets.begin()->first);

    auto tileset = tilesets.begin()->second;
    ASSERT_EQ(tileset.size(), 1);
    PublishedTile tile = *tileset.begin();

    EXPECT_EQ(TileIO::Format::B3dm, tile.m_format);
    Json::Value tileJson = tile.ReadJson();
    EXPECT_FALSE(tileJson.isNull());
    EXPECT_TRUE(tileJson.isObject());

    // Extract the geometry from the published tile
    auto tileGeom = tile.ReadGeometry(*model);
    EXPECT_FALSE(tileGeom.IsEmpty());

    // Verify FeatureTable
    Render::Primitives::MeshList const& meshes = tileGeom.Meshes();
    Render::FeatureTableCR featureTable = meshes.FeatureTable();
    EXPECT_EQ(2, featureTable.size());

    // NB: The published tile's batch table has an unused 'invalid' feature at index 0, which the tile reader omits from the FeatureTable.
    ExpectFeatureId(1, featureTable, *elemTriangle, catTriangleId);
    ExpectFeatureId(2, featureTable, *elemRectangle, catRectangleId);

    // Verify geometry
    ASSERT_EQ(1, meshes.size());
    Render::Primitives::MeshCR mesh = *meshes[0];
    AppData::VerifyMesh(mesh, 3, 0, 7, 7, 0, AppData::ColorDefList {ColorDef::Red()}, true);
    }

DEFINE_SAMPLE_TEST(DisplayStyles);

