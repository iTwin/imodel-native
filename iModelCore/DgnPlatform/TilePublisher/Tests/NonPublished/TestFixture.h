/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/Tests/NonPublished/TestFixture.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnPlatformAPI.h>
#include <TilePublisher/CesiumPublisher.h>
#include <DgnPlatform/TileReader.h>
#include <DgnPlatform/TileWriter.h>
#include <DgnPlatform/UnitTests/DgnDbTestUtils.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>

// Contains many helpful functions for inserting models and elements
// #include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_TILETREE
USING_NAMESPACE_BENTLEY_TILEPUBLISHER

namespace TileIO = BentleyApi::Dgn::TileTree::IO;

namespace JsonUtil
{
    inline DPoint3d ToPoint(Json::Value const& json)
        {
        return DPoint3d::From(json["x"].asDouble(), json["y"].asDouble(), json["z"].asDouble());
        }
    inline ColorDef ToColor(Json::Value const& json)
        {
        auto r = json["red"].asDouble(), g = json["green"].asDouble(), b = json["blue"].asDouble();
        return ColorDef(Byte(r*255), Byte(g*255), Byte(b*255));
        }
    inline DRange3d ToRange(Json::Value const& json)
        {
        return DRange3d::From(ToPoint(json["low"]), ToPoint(json["high"]));
        }
    inline RotMatrix ToRotMatrix(Json::Value const& json)
        {
        return RotMatrix::FromRowValues(
            json[0].asDouble(), json[3].asDouble(), json[6].asDouble(),
            json[1].asDouble(), json[4].asDouble(), json[7].asDouble(),
            json[2].asDouble(), json[5].asDouble(), json[8].asDouble());
        }
    inline Transform ToTransform(Json::Value const& json)
        {
        EXPECT_EQ(0.0, json[3].asDouble());
        EXPECT_EQ(0.0, json[7].asDouble());
        EXPECT_EQ(0.0, json[11].asDouble());
        EXPECT_EQ(1.0, json[15].asDouble());

        return Transform::FromRowValues(
            json[0].asDouble(), json[4].asDouble(), json[8].asDouble(), json[12].asDouble(),
            json[1].asDouble(), json[5].asDouble(), json[9].asDouble(), json[13].asDouble(),
            json[2].asDouble(), json[6].asDouble(), json[10].asDouble(), json[14].asDouble());
        }

    template<typename T> inline void ToIdSet(T& ids, Json::Value const& json)
        {
        using V = std::remove_reference<decltype(*ids.begin())>::type; // no value_type on IdSet...
        for (Json::ArrayIndex i = 0; i < json.size(); i++)
            ids.insert(V(json[i].asUInt64()));
        }
    template<typename T> inline T ToIdSet(Json::Value const& json)
        {
        T idSet;
        ToIdSet(idSet, json);
        return idSet;
        }

    template<typename T> inline T ToId(Utf8StringCR str)
        {
        return T(BeStringUtilities::ParseUInt64(str.c_str()));
        }
    inline DgnElementId ToElementId(Utf8StringCR str) { return ToId<DgnElementId>(str); }

    Json::Value Read(BeFileNameCR);
}

//=======================================================================================
// In-memory representation of the 'app data' associated with a tileset.
// Can be created from the appdata .json file produced by the tile publisher; and by
// a test to describe the expected output from tile publisher.
// @bsistruct                                                   Paul.Connelly   11/17
//=======================================================================================
struct AppData
{
private:
    void Read(BeFileNameCR filename);
    void Read(Json::Value const& json);
public:
    struct DisplayStyle
    {
        ColorDef    m_backgroundColor;
        bool        m_isGlobeVisible;
        ViewFlags   m_viewFlags;

        bool operator==(DisplayStyle const& rhs) const;
        void ExpectEqual(DisplayStyle const& rhs) const
            {
            // ###TODO ViewFlags
            EXPECT_EQ(m_backgroundColor, rhs.m_backgroundColor);
            EXPECT_EQ(m_isGlobeVisible, rhs.m_isGlobeVisible);
            }

        explicit DisplayStyle(DisplayStyle3dCR);
        explicit DisplayStyle(Json::Value const&);
        DisplayStyle() = default;
    };

    struct Model
    {
        enum class Type { Reality, Spatial, Sheet, Drawing, Unknown };

        DRange3d    m_extents;
        Utf8String  m_name;
        Utf8String  m_tilesetUrl;
        Transform   m_transform;
        Type        m_type = Type::Unknown;

        void ExpectEqual(Model const& rhs) const
            {
            EXPECT_EQ(m_name, rhs.m_name);
            EXPECT_EQ(m_tilesetUrl, rhs.m_tilesetUrl);
            EXPECT_EQ(m_type, rhs.m_type);
            // ###TODO EXPECT_TRUE(m_extents.IsEqual(rhs.m_extents));
            // ###TODO EXPECT_TRUE(m_transform.IsEqual(rhs.m_transform));
            }

        explicit Model(GeometricModel3dR model);
        explicit Model(Json::Value const&);
        Model() = default;
    };

    struct View
    {
        enum class Type { Camera, Ortho, Sheet, Drawing, Unknown };

        DgnElementId    m_categorySelector;
        DgnElementId    m_displayStyle;
        DPoint3d        m_extents;
        DPoint3d        m_eyePoint;
        double          m_focusDistance;
        bool            m_isCameraOn;
        double          m_lensAngle;
        DgnElementId    m_modelSelector;
        Utf8String      m_name;
        DPoint3d        m_origin;
        RotMatrix       m_rotation;
        Type            m_type = Type::Unknown;

        void ExpectEqual(View const& rhs) const
            {
            // ###TODO eyepoint, focusdistance, lensangle
            EXPECT_EQ(m_categorySelector, rhs.m_categorySelector);
            EXPECT_EQ(m_displayStyle, rhs.m_displayStyle);
            EXPECT_EQ(m_modelSelector, rhs.m_modelSelector);
            EXPECT_TRUE(m_extents.IsEqual(rhs.m_extents));
            EXPECT_EQ(m_isCameraOn, rhs.m_isCameraOn);
            EXPECT_EQ(m_name, rhs.m_name);
            EXPECT_TRUE(m_origin.IsEqual(rhs.m_origin));
            EXPECT_TRUE(m_rotation.IsEqual(rhs.m_rotation));
            EXPECT_EQ(m_type, rhs.m_type);
            }

        explicit View(SpatialViewDefinitionCR);
        explicit View(Json::Value const&);
        View() = default;
    };

    typedef bmap<DgnCategoryId, Utf8String> Categories;
    typedef bmap<DgnElementId, DgnCategoryIdSet> CategorySelectors;
    typedef bmap<DgnElementId, DisplayStyle> DisplayStyles;
    typedef bmap<DgnElementId, DgnModelIdSet> ModelSelectors;
    typedef bmap<DgnModelId, Model> Models;
    typedef bmap<DgnViewId, View> Views;

    Categories          m_categories;
    CategorySelectors   m_categorySelectors;
    DgnViewId           m_defaultView;
    DisplayStyles       m_displayStyles;
    DPoint3d            m_groundPoint;
    ModelSelectors      m_modelSelectors;
    Models              m_models;
    Utf8String          m_name;
    DRange3d            m_projectExtents;
    DPoint3d            m_projectOrigin;
    Transform           m_projectTransform;
    Views               m_views;

    // Read from publisher output
    explicit AppData(BeFileNameCR filename) { Read(filename); }

//  [ === Construct from test data === ]

    explicit AppData(SpatialViewDefinitionCR defaultView);

    void AddCategory(DgnCategoryCR cat) { m_categories.Insert(cat.GetCategoryId(), cat.GetCategoryName()); }
    void AddCategory(DgnCategoryId id, DgnDbR db) { auto cat = db.Elements().Get<DgnCategory>(id); ASSERT_TRUE(cat.IsValid()); AddCategory(*cat); }

    // Also adds all of the categories in the selector
    void AddCategorySelector(CategorySelectorCR);

    // Also adds all of the models in the selector
    void AddModelSelector(ModelSelectorCR);

    void AddModel(GeometricModel3dR);

    // Also adds the category+model selectors and the display style
    void AddView(SpatialViewDefinitionCR view);

//  [ === Comparisons === ]

    // Compare to the other AppData, producing test failure if they differ
    void ExpectEqual(AppData const& rhs) const;
    static void ExpectEqual(Utf8StringCR lhs, Utf8StringCR rhs) { EXPECT_EQ(lhs, rhs); }
    static void ExpectEqual(DgnCategoryIdSet const& lhs, DgnCategoryIdSet const& rhs) { ExpectEqualSets(lhs, rhs); }
    static void ExpectEqual(DisplayStyle const& lhs, DisplayStyle const& rhs) { lhs.ExpectEqual(rhs); }
    static void ExpectEqual(DgnModelIdSet const& lhs, DgnModelIdSet const& rhs) { ExpectEqualSets(lhs, rhs); }
    static void ExpectEqual(Model const& lhs, Model const& rhs) { lhs.ExpectEqual(rhs); }
    static void ExpectEqual(View const& lhs, View const& rhs) { lhs.ExpectEqual(rhs); }

    template<typename T> static void ExpectEqual(T const& lhs, T const& rhs)
        {
        EXPECT_EQ(lhs.size(), rhs.size());
        for (auto const& lkv : lhs)
            {
            auto rkv = rhs.find(lkv.first);
            EXPECT_FALSE(rhs.end() == rkv);
            if (rhs.end() != rkv)
                ExpectEqual(lkv.second, rkv->second);
            }
        }
    template<typename T> static void ExpectEqualSets(T const& lhs, T const& rhs)
        {
        EXPECT_EQ(lhs.size(), rhs.size());
        for (auto const& lkv : lhs)
            EXPECT_FALSE(rhs.end() == rhs.find(lkv));
        }
};

//=======================================================================================
// Represents a single published tile.
// @bsistruct                                                   Paul.Connelly   11/17
//=======================================================================================
struct PublishedTile
{
    BeFileName      m_filenameWithoutExtension;
    TileIO::Format  m_format;

    PublishedTile() = default;
    PublishedTile(BeFileNameCR gltfFileNameWithExtension);

    bool operator<(PublishedTile const& rhs) const { return m_filenameWithoutExtension.CompareToI(rhs.m_filenameWithoutExtension) < 0; }

    Json::Value ReadJson() const;
    Render::Primitives::GeometryCollection ReadGeometry(GeometricModelR model) const;
};

//=======================================================================================
// Represents the full contents of a tileset's directory associated with a single model.
// @bsistruct                                                   Paul.Connelly   11/17
//=======================================================================================
typedef bset<PublishedTile> PublishedTileset;

//=======================================================================================
// Represents the set of tilesets produced by the publisher.
// @bsistruct                                                   Paul.Connelly   11/17
//=======================================================================================
struct PublishedTilesets : bmap<DgnModelId, PublishedTileset>
{
private:
    void ProcessTilesetDir(BeFileNameCR dir);
    static DgnModelId ParseModelId(BeFileNameCR);
public:
    PublishedTilesets() = default;

    // Iterates the output directory to find all tileset directories and tiles within them.
    explicit PublishedTilesets(BeFileNameCR appDataDir);

    // Validate that there is one subdirectory corresponding to each model.
    void ExpectEqual(AppData::Models const& models) const;
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/17
//=======================================================================================
struct TestFixture : public ::testing::Test
{
protected:
    ScopedDgnHost   m_host;
    DgnDbPtr        m_db;

    void TearDown() override { SaveDb(); }
public:

//  [ === Managing the DgnDb === ]

    // Create a blank DgnDb to hold data to be published
    void SetupDb(WCharCP filenameWithoutExtension);

    // Get the DgnDb produced by SetupDb()
    DgnDbR GetDb() const { BeAssert(m_db.IsValid()); return *m_db; }

    // Close the DgnDb.
    void CloseDb() { GetDb().CloseDb(); }

    // Update the project extents for the DgnDb. You generally want to do this before publishing tiles.
    // It can also be convenient to set a view's frustum to match the project extents.
    AxisAlignedBox3d UpdateProjectExtents() { DgnDbTestUtils::UpdateProjectExtents(GetDb()); return GetDb().GeoLocation().GetProjectExtents(); }

    // Save all changes to the DgnDb. You generally want to do this before publishing tiles.
    void SaveDb()
        {
        if (m_db.IsValid() && m_db->IsDbOpen() && !m_db->IsReadonly())
            m_db->SaveChanges();
        }

//  [ === Executing Tests === ]

    // If a test obtains ref-counted pointers to DgnElements, and does not explicitly set all of them to null before the test terminates,
    // DgnDb will assert. Pass your test function/lambda to this function to ensure any DgnElementPtrs within it are released before termination.
    template<typename T> void ExecuteTest(T testFunc) { testFunc(); }

//  [ == Creating definition elements === ]

    // Create a spatial (3d) model
    PhysicalModelPtr InsertSpatialModel(Utf8CP partitionName) { return DgnDbTestUtils::InsertPhysicalModel(GetDb(), partitionName); }

    // Create a spatial category (for 3d elements)
    DgnCategoryId InsertSpatialCategory(Utf8CP name) { return DgnDbTestUtils::InsertSpatialCategory(GetDb(), name); }

    // Spatial (3d) views can display any number of spatial models, defined by a ModelSelector which holds a DgnModelIdSet of the viewed models.
    ModelSelectorCPtr InsertModelSelector(DgnModelId modelId, Utf8CP name="") { DgnModelIdSet modelIds; modelIds.insert(modelId); return InsertModelSelector(modelIds, name); }
    ModelSelectorCPtr InsertModelSelector(DgnModelIdSet const& modelIds, Utf8CP name="");

    // A view can display any number of categories, defined by a CategorySelector which holds a DgnCategoryIdSet of the viewed categories.
    CategorySelectorCPtr InsertCategorySelector(DgnCategoryId catId, Utf8CP name="") { DgnCategoryIdSet catIds; catIds.insert(catId); return InsertCategorySelector(catIds, name); }
    CategorySelectorCPtr InsertCategorySelector(DgnCategoryIdSet const& catIds, Utf8CP name="");

    DisplayStyle3dCPtr InsertDisplayStyle3d(Utf8CP name, ColorDef backgroundColor=ColorDef::Black(), bool groundPlane=false, ViewFlags viewFlags=ViewFlags());

    SpatialViewDefinitionCPtr InsertSpatialView(Utf8CP name, ModelSelectorCR models, CategorySelectorCR categories, DisplayStyle3dCR style, DRange3dCP viewedVolume, SpatialViewDefinition::Camera const* camera=nullptr);

//  [ === Defining element geometry === ]

    // Points are specified in local coordinates relative to element origin.

    CurveVectorPtr CreateShape(DPoint3dCP points, size_t numPoints) { return CurveVector::CreateLinear(points, numPoints, CurveVector::BOUNDARY_TYPE_Outer); }
    CurveVectorPtr CreateRectangle(DPoint3dCR lowerLeft, double width, double height);
    CurveVectorPtr CreateTriangle(DPoint3dCR lowerLeft, double width, double height);
    CurveVectorPtr CreateLineString(DPoint3dCP points, size_t numPoints) { return CurveVector::CreateLinear(points, numPoints, CurveVector::BOUNDARY_TYPE_Open); }

    GeometryBuilderPtr CreateGeometryBuilder(DgnModelR model, DgnCategoryId categoryId) { return GeometryBuilder::Create(model, categoryId, DPoint3d::FromZero()); }
    GeometryBuilderPtr CreateGeometryBuilder(DgnModelR model, DgnCategoryId categoryId, ColorDef color);

//  [ === Creating geometric elements === ]

    PhysicalElementCPtr InsertPhysicalElement(PhysicalModelR model, GeometryBuilderR builder, DPoint3dCR elementOrigin);

//  [ === Publishing tiles === ]

    // Get the full path to the directory in which the .bim and the tile publisher output's subdirectories reside.
    static BeFileName GetBaseDir() { BeFileName filename; BeTest::GetHost().GetOutputRoot(filename); return filename; }

    // Publish tiles to base directory.
    Cesium::TilesetPublisher::Status PublishTiles();

    // Get the root directory containing the published output.
    // This should contain XXX_AppData.json and 1 subdirectory for each published model, named Model_XX where XX is the model ID in hexadecimal format.
    BeFileName GetAppDataDir() const;

    // Get the path to the published appdata json file.
    BeFileName GetAppDataFileName() const;

    // Get the name of the tileset (same as the name of the .bim)
    WString GetTilesetNameW() const { return GetTilesetNameW(GetDb()); }
    static WString GetTilesetNameW(DgnDbR db) { return db.GetFileName().GetFileNameWithoutExtension(); }
    Utf8String GetTilesetName() const { return Utf8String(GetTilesetNameW()); }

    static Utf8String GetRelativeTilesetUrl(DgnModelId modelId, DgnDbR db);
};

