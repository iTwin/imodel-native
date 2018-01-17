/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/Tests/NonPublished/Sample_Test.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "TestFixture.h"

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/17
//=======================================================================================
struct SampleTestFixture : TestFixture
    {

    void RedRectangle();
    void TwoElementsNonUniformColor();

    template<typename T> void ExecuteTest(WCharCP testName, T testImpl)
        {
        SetupDb(testName);
        testImpl();
        }
    };

/*---------------------------------------------------------------------------------**//**
* Test a single element represented as a single red rectangle.
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void SampleTestFixture::RedRectangle()
    {
    // Set up the data to be published...
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

    // Publish the tileset
    auto status = PublishTiles();
    EXPECT_EQ(status, Cesium::TilesetPublisher::Status::Success);

    // Compare RedRectangle_AppData.json (metadata about the dgndb)
    AppData appData(GetAppDataFileName());
    AppData expected(*view);
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
    EXPECT_EQ(1, featureTable.size());

    // NB: The published tile's batch table has an unused 'invalid' feature at index 0, which the tile reader omits from the FeatureTable.
    ExpectFeatureId(1, featureTable, *elem, catId);

    // Verify geometry
    ASSERT_EQ(1, meshes.size());
    Render::Primitives::MeshCR mesh = *meshes[0];

    AppData::VerifyMesh(mesh, 2, 0, 4, 4, 0, AppData::ColorDefList {ColorDef::Red()}, true);
    }

/*---------------------------------------------------------------------------------**//**
* Test two elements: a red rectangle and a blue triangle.
* ###TODO: Consolidate shared code...
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void SampleTestFixture::TwoElementsNonUniformColor()
    {
    // Set up the data to be published...
    PhysicalModelPtr model = InsertSpatialModel("MyModel");
    ModelSelectorCPtr modelSel = InsertModelSelector(model->GetModelId());
    ASSERT_TRUE(modelSel.IsValid());

    DgnCategoryId catId = InsertSpatialCategory("MyCategory");
    EXPECT_TRUE(catId.IsValid());

    CategorySelectorCPtr catSel = InsertCategorySelector(catId);
    ASSERT_TRUE(catSel.IsValid());

    DisplayStyle3dCPtr style = InsertDisplayStyle3d("MyStyle", ColorDef::Blue());
    ASSERT_TRUE(style.IsValid());

    // Create a red rectangle
    auto geom = CreateGeometryBuilder(*model, catId, ColorDef::Red());
    geom->Append(*CreateRectangle(DPoint3d::FromZero(), 10.0, 10.0));

    PhysicalElementCPtr rectangle = InsertPhysicalElement(*model, *geom, DPoint3d::FromZero());
    ASSERT_TRUE(rectangle.IsValid());

    // Create a blue triangle
    geom = CreateGeometryBuilder(*model, catId, ColorDef::Blue());
    geom->Append(*CreateTriangle(DPoint3d::FromZero(), 10.0, 10.0));

    PhysicalElementCPtr triangle = InsertPhysicalElement(*model, *geom, DPoint3d::From(5.0, 5.0, 0.0));
    ASSERT_TRUE(triangle.IsValid());

    // Adjust project extents
    AxisAlignedBox3d extents = UpdateProjectExtents();
    SpatialViewDefinitionCPtr view = InsertSpatialView("MyView", *modelSel, *catSel, *style, &extents);
    ASSERT_TRUE(view.IsValid());

    // Publish the tileset
    auto status = PublishTiles();
    EXPECT_EQ(status, Cesium::TilesetPublisher::Status::Success);

    // Compare "appdata" json (metadata about the dgndb)
    AppData appData(GetAppDataFileName());
    AppData expected(*view);
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

    // NB: We're relying on elements being processed and assigned feature IDs in a specific order...
    ExpectFeatureId(1, featureTable, *rectangle, catId);
    ExpectFeatureId(2, featureTable, *triangle, catId);

    // Verify geometry. 1 mesh with two colors.
    ASSERT_EQ(1, meshes.size());

    Render::Primitives::MeshCR mesh = *meshes[0];
    AppData::VerifyMesh(mesh, 3, 0, 7, 7, 0, AppData::ColorDefList {ColorDef::Red(), ColorDef::Blue()}, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SampleTestFixture, TestRedRectangle)
    {
    ExecuteTest(L"RedRectangle", [&]() { RedRectangle(); });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SampleTestFixture, TestTwoElementsNonUniformColor)
    {
    ExecuteTest(L"TwoElementsNonUniformColor", [&]() { TwoElementsNonUniformColor(); });
    }

