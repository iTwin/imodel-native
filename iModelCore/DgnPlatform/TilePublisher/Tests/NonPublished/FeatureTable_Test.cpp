/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/Tests/NonPublished/FeatureTable_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "TestFixture.h"

//=======================================================================================
// @bsistruct                                                   Tomas.Sakinis   11/17
//=======================================================================================
struct FeatureTableTestFixture : TestFixture
    {
    void MultipleElementsWithDifferentCategories();
    void MultipleElementsWithDifferentSubCategories();
    void MultipleElementsWithDifferentDgnGeometryClass();

    template<typename T> void ExecuteTest(WCharCP testName, T testImpl)
        {
        SetupDb(testName);
        testImpl();
        }
    };

#define DEFINE_SAMPLE_TEST(MEMBER_FUNC) TEST_F(FeatureTableTestFixture, MEMBER_FUNC) \
    { \
    ExecuteTest(L ## #MEMBER_FUNC , [&]() { MEMBER_FUNC(); }); \
    }

/*---------------------------------------------------------------------------------**//**
* Test multiple elements in different categories
* @bsimethod                                                    Tomas.Sakinis   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void FeatureTableTestFixture::MultipleElementsWithDifferentCategories()
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
* Test multiple elements in different subcategories
* @bsimethod                                                    Tomas.Sakinis   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void FeatureTableTestFixture::MultipleElementsWithDifferentSubCategories()
    {
    PhysicalModelPtr model = InsertSpatialModel("Model1");

    ModelSelectorCPtr modelSel = InsertModelSelector(model->GetModelId());
    ASSERT_TRUE(modelSel.IsValid());

    // categories
    DgnCategoryId catTriangleId = InsertSpatialCategory("CategoryTriangle");
    EXPECT_TRUE(catTriangleId.IsValid());

    DgnSubCategoryId subCategoryGreenId = InsertSubCategory(catTriangleId, "Green", ColorDef::Green());
    DgnSubCategoryId subCategoryYellowId = InsertSubCategory(catTriangleId, "Yellow", ColorDef::Yellow());

    CategorySelectorCPtr catTriangleSel = InsertCategorySelector(catTriangleId);
    ASSERT_TRUE(catTriangleSel.IsValid());

    DgnCategoryId catRectangleId = InsertSpatialCategory("CategoryRectangle");
    EXPECT_TRUE(catRectangleId.IsValid());

    DgnSubCategoryId subCategoryMagentaId = InsertSubCategory(catRectangleId, "Magenta", ColorDef::Magenta());
    DgnSubCategoryId subCategoryOrangeId = InsertSubCategory(catRectangleId, "Orange", ColorDef::Orange());

    CategorySelectorCPtr catRectangleSel = InsertCategorySelector(catRectangleId);
    ASSERT_TRUE(catRectangleSel.IsValid());

    DgnCategoryIdSet allCategories;
    allCategories.insert(catTriangleId);
    allCategories.insert(catRectangleId);

    CategorySelectorCPtr allCategoriesSel = InsertCategorySelector(allCategories);
    ASSERT_TRUE(allCategoriesSel.IsValid());

    // geometry
    auto geomTriangle = CreateGeometryBuilder(*model, catTriangleId, ColorDef::Red());
    geomTriangle->Append(*CreateTriangle(DPoint3d::FromZero(), 10.0, 10.0));
    geomTriangle->Append(subCategoryGreenId);
    geomTriangle->Append(*CreateTriangle(DPoint3d::FromZero(), 20.0, 20.0));
    geomTriangle->Append(subCategoryYellowId);
    geomTriangle->Append(*CreateTriangle(DPoint3d::FromZero(), 30.0, 30.0));

    PhysicalElementCPtr elemTriangle = InsertPhysicalElement(*model, *geomTriangle, DPoint3d::FromZero());
    ASSERT_TRUE(elemTriangle.IsValid());

    auto geomRectangle = CreateGeometryBuilder(*model, catRectangleId, ColorDef::Red());
    geomRectangle->Append(*CreateRectangle(DPoint3d::FromZero(), 10.0, 10.0));
    geomRectangle->Append(subCategoryMagentaId);
    geomRectangle->Append(*CreateRectangle(DPoint3d::FromZero(), 20.0, 20.0));
    geomRectangle->Append(subCategoryOrangeId);
    geomRectangle->Append(*CreateRectangle(DPoint3d::FromZero(), 30.0, 30.0));

    PhysicalElementCPtr elemRectangle = InsertPhysicalElement(*model, *geomRectangle, DPoint3d::FromZero());
    ASSERT_TRUE(elemRectangle.IsValid());

    // view
    AxisAlignedBox3d extents = UpdateProjectExtents();
    DisplayStyle3dCPtr style = InsertDisplayStyle3d("MyStyle", ColorDef::White());
    ASSERT_TRUE(style.IsValid());

    SpatialViewDefinitionCPtr view = InsertSpatialView("MyView", *modelSel, *allCategoriesSel, *style, &extents);
    ASSERT_TRUE(view.IsValid());

    // Publish the tileset
    auto status = PublishTiles();
    EXPECT_EQ(status, Cesium::TilesetPublisher::Status::Success);

    // verification
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
    EXPECT_EQ(6, featureTable.size());

    // NB: The published tile's batch table has an unused 'invalid' feature at index 0, which the tile reader omits from the FeatureTable.
    ExpectFeatureId(1, featureTable, *elemTriangle, catTriangleId);

    // Verify geometry
    ASSERT_EQ(1, meshes.size());
    Render::Primitives::MeshCR mesh = *meshes[0];

    AppData::VerifyMesh(mesh, 9, 0, 21, 21, 0, AppData::ColorDefList {ColorDef::Red(), ColorDef::Green(), ColorDef::Yellow(), ColorDef::Magenta(), ColorDef::Orange()}, false);
    }

/*---------------------------------------------------------------------------------**//**
* Test multiple elements in different Dgn

* @bsimethod                                                    Tomas.Sakinis   12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void FeatureTableTestFixture::MultipleElementsWithDifferentDgnGeometryClass()
    {
    PhysicalModelPtr model = InsertSpatialModel("Model1");

    ModelSelectorCPtr modelSel = InsertModelSelector(model->GetModelId());
    ASSERT_TRUE(modelSel.IsValid());

    // categories
    DgnCategoryId catTriangleId = InsertSpatialCategory("CategoryTriangle");
    EXPECT_TRUE(catTriangleId.IsValid());

    DgnSubCategoryId subCategoryRedId = InsertSubCategory(catTriangleId, "Red", ColorDef::Red());
    DgnSubCategoryId subCategoryGreenId = InsertSubCategory(catTriangleId, "Green", ColorDef::Green());
    DgnSubCategoryId subCategoryYellowId = InsertSubCategory(catTriangleId, "Yellow", ColorDef::Yellow());

    CategorySelectorCPtr catTriangleSel = InsertCategorySelector(catTriangleId);
    ASSERT_TRUE(catTriangleSel.IsValid());

    DgnCategoryId catRectangleId = InsertSpatialCategory("CategoryRectangle");
    EXPECT_TRUE(catRectangleId.IsValid());

    DgnSubCategoryId subCategoryBlueId = InsertSubCategory(catRectangleId, "Blue", ColorDef::Blue());
    DgnSubCategoryId subCategoryMagentaId = InsertSubCategory(catRectangleId, "Magenta", ColorDef::Magenta());
    DgnSubCategoryId subCategoryOrangeId = InsertSubCategory(catRectangleId, "Orange", ColorDef::Orange());

    CategorySelectorCPtr catRectangleSel = InsertCategorySelector(catRectangleId);
    ASSERT_TRUE(catRectangleSel.IsValid());

    DgnCategoryIdSet allCategories;
    allCategories.insert(catTriangleId);
    allCategories.insert(catRectangleId);

    CategorySelectorCPtr allCategoriesSel = InsertCategorySelector(allCategories);
    ASSERT_TRUE(allCategoriesSel.IsValid());

    // geometry
    auto geomTriangle = CreateGeometryBuilder(*model, catTriangleId);
    auto geomTriangleParams = GeometryParams(catTriangleId);

    geomTriangleParams.SetGeometryClass(DgnGeometryClass::Primary);
    geomTriangleParams.SetSubCategoryId(subCategoryRedId);
    geomTriangle->Append(geomTriangleParams);
    geomTriangle->Append(*CreateTriangle(DPoint3d::FromZero(), 10.0, 10.0));

    geomTriangleParams.SetGeometryClass(DgnGeometryClass::Construction);
    geomTriangleParams.SetSubCategoryId(subCategoryGreenId);
    geomTriangle->Append(geomTriangleParams);
    geomTriangle->Append(*CreateTriangle(DPoint3d::FromZero(), 20.0, 20.0));

    geomTriangleParams.SetGeometryClass(DgnGeometryClass::Pattern);
    geomTriangleParams.SetSubCategoryId(subCategoryYellowId);
    geomTriangle->Append(geomTriangleParams);
    geomTriangle->Append(*CreateTriangle(DPoint3d::FromZero(), 30.0, 30.0));

    PhysicalElementCPtr elemTriangle = InsertPhysicalElement(*model, *geomTriangle, DPoint3d::FromZero());
    ASSERT_TRUE(elemTriangle.IsValid());

    auto geomRectangle = CreateGeometryBuilder(*model, catRectangleId);
    auto geomRectangleParams = GeometryParams(catRectangleId);

    geomRectangleParams.SetGeometryClass(DgnGeometryClass::Primary);
    geomRectangleParams.SetSubCategoryId(subCategoryBlueId);
    geomRectangle->Append(geomRectangleParams);
    geomRectangle->Append(*CreateRectangle(DPoint3d::FromZero(), 10.0, 10.0));

    geomRectangleParams.SetGeometryClass(DgnGeometryClass::Construction);
    geomRectangleParams.SetSubCategoryId(subCategoryMagentaId);
    geomRectangle->Append(geomRectangleParams);
    geomRectangle->Append(*CreateRectangle(DPoint3d::FromZero(), 20.0, 20.0));

    geomRectangleParams.SetGeometryClass(DgnGeometryClass::Pattern);
    geomRectangleParams.SetSubCategoryId(subCategoryOrangeId);
    geomRectangle->Append(geomRectangleParams);
    geomRectangle->Append(*CreateRectangle(DPoint3d::FromZero(), 30.0, 30.0));

    PhysicalElementCPtr elemRectangle = InsertPhysicalElement(*model, *geomRectangle, DPoint3d::FromZero());
    ASSERT_TRUE(elemRectangle.IsValid());

    // view
    AxisAlignedBox3d extents = UpdateProjectExtents();
    DisplayStyle3dCPtr style = InsertDisplayStyle3d("MyStyle", ColorDef::White());
    ASSERT_TRUE(style.IsValid());

    SpatialViewDefinitionCPtr view = InsertSpatialView("MyView", *modelSel, *allCategoriesSel, *style, &extents);
    ASSERT_TRUE(view.IsValid());

    // Publish the tileset
    auto status = PublishTiles();
    EXPECT_EQ(status, Cesium::TilesetPublisher::Status::Success);

    // verification
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
    EXPECT_EQ(6, featureTable.size());

    // NB: The published tile's batch table has an unused 'invalid' feature at index 0, which the tile reader omits from the FeatureTable.
    ExpectFeatureId(1, featureTable, *elemTriangle, subCategoryRedId, DgnGeometryClass::Primary);
    ExpectFeatureId(2, featureTable, *elemTriangle, subCategoryGreenId, DgnGeometryClass::Construction);
    ExpectFeatureId(3, featureTable, *elemTriangle, subCategoryYellowId, DgnGeometryClass::Pattern);
    ExpectFeatureId(4, featureTable, *elemRectangle, subCategoryBlueId, DgnGeometryClass::Primary);
    ExpectFeatureId(5, featureTable, *elemRectangle, subCategoryMagentaId, DgnGeometryClass::Construction);
    ExpectFeatureId(6, featureTable, *elemRectangle, subCategoryOrangeId, DgnGeometryClass::Pattern);

    //ExpectFeatureId(2, featureTable, *elemRectangle, catRectangleId, DgnGeometryClass::Construction);

    // Verify geometry
    ASSERT_EQ(1, meshes.size());
    Render::Primitives::MeshCR mesh = *meshes[0];

    AppData::VerifyMesh(mesh, 9, 0, 21, 21, 0, AppData::ColorDefList {ColorDef::Red(), ColorDef::Green(),ColorDef::Yellow(), ColorDef::Blue(), ColorDef::Magenta(), ColorDef::Orange()}, false);
    }


DEFINE_SAMPLE_TEST(MultipleElementsWithDifferentCategories);
DEFINE_SAMPLE_TEST(MultipleElementsWithDifferentSubCategories);
DEFINE_SAMPLE_TEST(MultipleElementsWithDifferentDgnGeometryClass);
