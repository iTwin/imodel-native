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

    // Compare TestRectangle_AppData.json (metadata about the dgndb)
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
    
    uint32_t featureId = 0;
    Render::Feature expectedFeature(elem->GetElementId(), DgnCategory::GetDefaultSubCategoryId(catId), DgnGeometryClass::Primary);
    EXPECT_TRUE(featureTable.FindIndex(featureId, expectedFeature));
    EXPECT_EQ(1, featureId); // The published tile's batch table has an unused 'invalid' feature at index 0, which the tile reader omits from the FeatureTable.

    // Verify geometry
    ASSERT_EQ(1, meshes.size());
    Render::Primitives::MeshCR mesh = *meshes[0];
    EXPECT_FALSE(mesh.IsEmpty());
    EXPECT_EQ(Render::Primitives::Mesh::PrimitiveType::Mesh, mesh.GetType());
    EXPECT_FALSE(nullptr == mesh.GetFeatureTable());
    EXPECT_EQ(mesh.Triangles().Count(), 2);
    EXPECT_TRUE(mesh.Polylines().empty());
    EXPECT_EQ(mesh.Points().size(), 4);
    EXPECT_EQ(mesh.Normals().size(), 4);
    EXPECT_TRUE(mesh.Params().empty());

    // Verify colors
    Render::Primitives::ColorTableCR colors = mesh.GetColorTable();
    ASSERT_TRUE(colors.IsUniform());
    EXPECT_EQ(ColorDef::Red().GetValue(), colors.begin()->first);
    EXPECT_EQ(0, colors.begin()->second);

    // Verify feature IDs
    Render::FeatureIndex feats;
    mesh.ToFeatureIndex(feats);

    // NB: We might expect a uniform feature table, but the indices are set directly from the batch table json.
    // The tile reader doesn't both to check if all the indices are identical (they are).
    EXPECT_FALSE(feats.IsUniform());
    }

DEFINE_SAMPLE_TEST(TestRectangle);

