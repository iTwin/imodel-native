/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/Tests/NonPublished/Selectors_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "TestFixture.h"

//=======================================================================================
// @bsistruct                                                   Tomas.Sakinis   11/17
//=======================================================================================
struct SelectorsTestFixture : TestFixture
    {
    void CategorySelectors();
    void ModelSelectors();

    template<typename T> void ExecuteTest(WCharCP testName, T testImpl)
        {
        SetupDb(testName);
        testImpl();
        }
    };

#define DEFINE_SAMPLE_TEST(MEMBER_FUNC) TEST_F(SelectorsTestFixture, MEMBER_FUNC) \
    { \
    ExecuteTest(L ## #MEMBER_FUNC , [&]() { MEMBER_FUNC(); }); \
    }


/*---------------------------------------------------------------------------------**//**
* Test multiple category selectors for one model
* @bsimethod                                                    Tomas.Sakinis   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectorsTestFixture::CategorySelectors()
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

    DisplayStyle3dCPtr style = InsertDisplayStyle3d("MyStyle", ColorDef::Blue());
    ASSERT_TRUE(style.IsValid());

    auto geomTriangle = CreateGeometryBuilder(*model, catTriangleId, ColorDef::Red());
    geomTriangle->Append(*CreateTriangle(DPoint3d::FromZero(), 10.0, 10.0));

    PhysicalElementCPtr elemTriangle = InsertPhysicalElement(*model, *geomTriangle, DPoint3d::FromZero());
    ASSERT_TRUE(elemTriangle.IsValid());

    auto geomRectangle = CreateGeometryBuilder(*model, catRectangleId, ColorDef::Red());
    geomRectangle->Append(*CreateRectangle(DPoint3d::FromZero(), 10.0, 10.0));

    PhysicalElementCPtr elemRectangle = InsertPhysicalElement(*model, *geomRectangle, DPoint3d::FromZero());
    ASSERT_TRUE(elemRectangle.IsValid());

    AxisAlignedBox3d extents = UpdateProjectExtents();
    SpatialViewDefinitionCPtr view = InsertSpatialView("MyView", *modelSel, *allCategoriesSel, *style, &extents);
    ASSERT_TRUE(view.IsValid());

    // Publish the tileset
    auto status = PublishTiles();
    EXPECT_EQ(status, Cesium::TilesetPublisher::Status::Success);

    // Compare x_AppData.json (metadata about the dgndb)
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

    // NB: The published tile's batch table has an unused 'invalid' feature at index 0, which the tile reader omits from the FeatureTable.
    ExpectFeatureId(1, featureTable, *elemTriangle, catTriangleId);

    // Verify geometry
    ASSERT_EQ(1, meshes.size());
    Render::Primitives::MeshCR mesh = *meshes[0];

    AppData::VerifyMesh(mesh, 3, 0, 7, 7, 0, AppData::ColorDefList {ColorDef::Red()}, true);
    }

/*---------------------------------------------------------------------------------**//**
* Test category selectors for two models
* @bsimethod                                                    Tomas.Sakinis   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectorsTestFixture::ModelSelectors()
    {
    // Set up the data to be published...
    PhysicalModelPtr model1 = InsertSpatialModel("Model1");

    ModelSelectorCPtr model1Sel = InsertModelSelector(model1->GetModelId());
    ASSERT_TRUE(model1Sel.IsValid());

    PhysicalModelPtr model2 = InsertSpatialModel("Model2");

    ModelSelectorCPtr model2Sel = InsertModelSelector(model2->GetModelId());
    ASSERT_TRUE(model2Sel.IsValid());

    DgnCategoryId cat1Id = InsertSpatialCategory("Category1");
    EXPECT_TRUE(cat1Id.IsValid());

    DgnCategoryId cat2Id = InsertSpatialCategory("Category2");
    EXPECT_TRUE(cat2Id.IsValid());

    CategorySelectorCPtr cat1Sel = InsertCategorySelector(cat1Id);
    ASSERT_TRUE(cat1Sel.IsValid());

    CategorySelectorCPtr cat2Sel = InsertCategorySelector(cat2Id);
    ASSERT_TRUE(cat2Sel.IsValid());

    DisplayStyle3dCPtr style = InsertDisplayStyle3d("MyStyle", ColorDef::Blue());
    ASSERT_TRUE(style.IsValid());

    auto geomTriangle = CreateGeometryBuilder(*model1, cat1Id, ColorDef::Red());
    geomTriangle->Append(*CreateTriangle(DPoint3d::FromZero(), 10.0, 10.0));

    PhysicalElementCPtr elemTriangle = InsertPhysicalElement(*model1, *geomTriangle, DPoint3d::FromZero());
    ASSERT_TRUE(elemTriangle.IsValid());
    //---
    auto geomRectangle = CreateGeometryBuilder(*model2, cat2Id, ColorDef::Red());
    geomRectangle->Append(*CreateRectangle(DPoint3d::FromZero(), 10.0, 10.0));

    PhysicalElementCPtr elemRectangle = InsertPhysicalElement(*model2, *geomRectangle, DPoint3d::FromZero());
    ASSERT_TRUE(elemRectangle.IsValid());

    AxisAlignedBox3d extents = UpdateProjectExtents();
    SpatialViewDefinitionCPtr view1 = InsertSpatialView("ModelView1", *model1Sel, *cat1Sel, *style, &extents);
    ASSERT_TRUE(view1.IsValid());

    SpatialViewDefinitionCPtr view2 = InsertSpatialView("ModelView2", *model2Sel, *cat2Sel, *style, &extents);
    ASSERT_TRUE(view2.IsValid());

    // Publish the tileset
    auto status = PublishTiles();
    EXPECT_EQ(status, Cesium::TilesetPublisher::Status::Success);

    // Compare x_AppData.json (metadata about the dgndb)
    AppData appData(GetAppDataFileName());
    AppData expected(*view1);
    expected.AddView(*view2);
    appData.ExpectEqual(expected);

    // Verify files in output directories
    PublishedTilesets tilesets(GetAppDataDir());
    tilesets.ExpectEqual(expected.m_models);

    ASSERT_EQ(tilesets.size(), 2);
    DgnDbR db = GetDb();

    int geomValues[2][3] = {{1, 3, 3}, {2, 4, 4}};
    int i = 0;
    for (auto const &item : tilesets)
        {
        auto model = db.Models().Get<GeometricModel3d>(item.first);
        EXPECT_EQ(model->GetModelId(), item.first);

        auto tileset = item.second;
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
        EXPECT_EQ(featureTable.size(), 1);

        // NB: The published tile's batch table has an unused 'invalid' feature at index 0, which the tile reader omits from the FeatureTable.
        //auto elem = db.Elements().begin();
        //ExpectFeatureId(1, featureTable, *elem, catId);

        // Verify geometry
        ASSERT_EQ(1, meshes.size());
        Render::Primitives::MeshCR mesh = *meshes[0];

        AppData::VerifyMesh(mesh, geomValues[i][0], 0, geomValues[i][1], geomValues[i][2], 0, AppData::ColorDefList {ColorDef::Red()}, true);
        i++;
        }
    }


DEFINE_SAMPLE_TEST(CategorySelectors);
DEFINE_SAMPLE_TEST(ModelSelectors);

