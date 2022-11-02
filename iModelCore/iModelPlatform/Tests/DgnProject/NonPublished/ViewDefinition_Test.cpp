/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <vector>

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ViewFlagsOverrides, ToFromJson)
    {
    struct BoolProp
    {
        Utf8CP name;
        bool value;
    };

    using Ovrs = Render::ViewFlagsOverrides;
    BeJsDocument intProps;
    intProps[Ovrs::json_renderMode()] = static_cast<int32_t>(Render::RenderMode::SolidFill);
    intProps[Ovrs::json_edgeMask()] = 2;

    auto makeJson = [](std::vector<BoolProp> const& props)
        {
        BeJsDocument val;
        for (auto const& prop : props)
            val[prop.name] = prop.value;

        return val.Stringify();
        };

    Utf8String testCases[] =
        {
        BeJsDocument::Null().Stringify(),
        makeJson({
                { Ovrs::json_dimensions(), true },
                { Ovrs::json_transparency(), false },
                { Ovrs::json_lighting(), true },
            }),
        intProps.Stringify(),
        makeJson({
                { Ovrs::json_dimensions(), true },
                { Ovrs::json_patterns(), true },
                { Ovrs::json_weights(), true },
                { Ovrs::json_styles(), true },
                { Ovrs::json_transparency(), true },
                { Ovrs::json_fill(), true },
                { Ovrs::json_textures(), true },
                { Ovrs::json_materials(), true },
                { Ovrs::json_lighting(), true },
                { Ovrs::json_visibleEdges(), true },
                { Ovrs::json_hiddenEdges(), true },
                { Ovrs::json_shadows(), false },
                { Ovrs::json_clipVolume(), false },
                { Ovrs::json_constructions(), false },
                { Ovrs::json_monochrome(), false },
                { Ovrs::json_noGeometryMap(), false },
                { Ovrs::json_backgroundMap(), false },
                { Ovrs::json_hLineMaterialColors(), false },
                { Ovrs::json_forceSurfaceDiscard(), false },
                { Ovrs::json_whiteOnWhiteReversal(), false },
                { Ovrs::json_thematicDisplay(), false },
            }),
        };

    for (auto const& input: testCases)
        {
        BeJsDocument testCase(input);
        auto ovrs = Render::ViewFlagsOverrides::FromJson(testCase);
        BeJsDocument json;
        ovrs.ToJson(json);
        EXPECT_TRUE(testCase.isExactEqual(json));
        }
    }

//========================================================================================
// @bsiclass
//========================================================================================
struct ViewDefinitionTests : public DgnDbTestFixture
{
    OrthographicViewDefinitionPtr InsertSpatialView(SpatialModelR model, Utf8CP name, bool isPrivate, DisplayStyle3dP style = nullptr);
    SheetViewDefinitionCPtr InsertSheetView(Sheet::ModelR model, Utf8CP name, bool isPrivate);
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
OrthographicViewDefinitionPtr ViewDefinitionTests::InsertSpatialView(SpatialModelR model, Utf8CP name, bool isPrivate, DisplayStyle3dP style)
    {
    DgnDbR db = model.GetDgnDb();
    DefinitionModelR dictionary = db.GetDictionaryModel();

    if (nullptr == style)
        style = new DisplayStyle3d(dictionary, "");

    ModelSelectorPtr modelSelector = new ModelSelector(dictionary, "");
    modelSelector->AddModel(model.GetModelId());

    OrthographicViewDefinitionPtr viewDef = new OrthographicViewDefinition(dictionary, name, *new CategorySelector(dictionary, ""), *style, *modelSelector);
    BeAssert(viewDef.IsValid());

    for (ElementIteratorEntryCR categoryEntry : SpatialCategory::MakeIterator(db))
        viewDef->GetCategorySelector().AddCategory(categoryEntry.GetId<DgnCategoryId>());

    viewDef->SetIsPrivate(isPrivate);
    viewDef->SetStandardViewRotation(StandardView::Iso);
    viewDef->LookAtVolume(model.QueryElementsRange());
    viewDef->Insert();
    BeAssert(viewDef->GetViewId().IsValid());
    return viewDef;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
SheetViewDefinitionCPtr ViewDefinitionTests::InsertSheetView(Sheet::ModelR model, Utf8CP name, bool isPrivate)
    {
    DgnDbR db = model.GetDgnDb();
    DefinitionModelR dictionary = db.GetDictionaryModel();

    SheetViewDefinition sheetView(dictionary, name, model.GetModelId(), *new CategorySelector(dictionary, ""), *new DisplayStyle2d(dictionary, ""));
    sheetView.SetIsPrivate(isPrivate);
    SheetViewDefinitionCPtr result = db.Elements().Insert<SheetViewDefinition>(sheetView);
    BeAssert(result.IsValid());
    return result;
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(ViewDefinitionTests, MakeIterator)
    {
    SetupSeedProject();

    DocumentListModelPtr drawingListModel = DgnDbTestUtils::InsertDocumentListModel(GetDgnDb(), "DrawingListModel");
    DrawingPtr drawing = DgnDbTestUtils::InsertDrawing(*drawingListModel, "2");
    DrawingModelPtr model2 = DgnDbTestUtils::InsertDrawingModel(*drawing);

    DrawingViewDefinitionPtr view2A = DgnDbTestUtils::InsertDrawingView(*model2, "2A");
    DrawingViewDefinitionPtr view2B = DgnDbTestUtils::InsertDrawingView(*model2, "2B");
    DrawingViewDefinitionPtr view2C = DgnDbTestUtils::InsertDrawingView(*model2, "2C");
    ASSERT_FALSE(view2A->IsPrivate());
    ASSERT_FALSE(view2B->IsPrivate());
    ASSERT_FALSE(view2C->IsPrivate());

    view2A->SetIsPrivate(false);
    EXPECT_EQ(DgnDbStatus::Success, view2A->Update());
    ASSERT_FALSE(view2A->IsPrivate());
    ASSERT_FALSE(view2A->GetPropertyValueBoolean("IsPrivate"));

    view2B->SetIsPrivate(true);
    EXPECT_EQ(DgnDbStatus::Success, view2B->Update());
    ASSERT_TRUE(view2B->IsPrivate());
    ASSERT_TRUE(view2B->GetPropertyValueBoolean("IsPrivate"));

    PhysicalModelPtr model3 = DgnDbTestUtils::InsertPhysicalModel(*m_db, "3");

    OrthographicViewDefinitionPtr view3A = InsertSpatialView(*model3, "3A", false);
    OrthographicViewDefinitionPtr view3B = InsertSpatialView(*model3, "3B", false);
    OrthographicViewDefinitionPtr view3C = InsertSpatialView(*model3, "3C", false);
    ASSERT_FALSE(view3A->IsPrivate());
    ASSERT_FALSE(view3B->IsPrivate());
    ASSERT_FALSE(view3C->IsPrivate());

    ASSERT_EQ(6, ViewDefinition::QueryCount(*m_db));
    ASSERT_EQ(5, ViewDefinition::QueryCount(*m_db, "WHERE IsPrivate=FALSE"));
    ASSERT_EQ(1, ViewDefinition::QueryCount(*m_db, "WHERE IsPrivate=TRUE"));
    ASSERT_EQ(0, ViewDefinition::QueryCount(*m_db, "WHERE IsPrivate IS NULL"));

    int i=0;
    for (ViewDefinition::Entry const& iter : ViewDefinition::MakeIterator(*m_db, nullptr, "ORDER BY [CodeValue] DESC"))
        {
        switch (++i)
            {
            case 1:
                ASSERT_STREQ(iter.GetName(), "3C");
                ASSERT_FALSE(iter.IsPrivate());
                ASSERT_TRUE(iter.IsOrthographicView());
                ASSERT_TRUE(iter.IsSpatialView());
                ASSERT_FALSE(iter.IsDrawingView());
                break;

            case 2:
                ASSERT_STREQ(iter.GetName(), "3B");
                ASSERT_FALSE(iter.IsPrivate());
                ASSERT_TRUE(iter.IsOrthographicView());
                ASSERT_TRUE(iter.IsSpatialView());
                ASSERT_FALSE(iter.IsDrawingView());
                break;

            case 3:
                ASSERT_STREQ(iter.GetName(), "3A");
                ASSERT_FALSE(iter.IsPrivate());
                ASSERT_TRUE(iter.IsOrthographicView());
                ASSERT_TRUE(iter.IsSpatialView());
                ASSERT_FALSE(iter.IsDrawingView());
                break;

            case 4:
                ASSERT_STREQ(iter.GetName(), "2C");
                ASSERT_FALSE(iter.IsPrivate());
                ASSERT_TRUE(iter.IsDrawingView());
                ASSERT_FALSE(iter.IsSpatialView());
                break;

            case 5:
                ASSERT_STREQ(iter.GetName(), "2B");
                ASSERT_TRUE(iter.IsPrivate());
                ASSERT_TRUE(iter.IsDrawingView());
                ASSERT_FALSE(iter.IsSpatialView());
                break;

            case 6:
                ASSERT_STREQ(iter.GetName(), "2A");
                ASSERT_FALSE(iter.IsPrivate());
                ASSERT_TRUE(iter.IsDrawingView());
                ASSERT_FALSE(iter.IsSpatialView());
                break;

            default:
                ASSERT_TRUE(false);
                break;
            }
        }

    i=0;
    for (ViewDefinition::Entry const& iter : ViewDefinition::MakeIterator(*m_db, "WHERE IsPrivate=FALSE", "ORDER BY [CodeValue] ASC"))
        {
        switch (++i)
            {
            case 1: ASSERT_STREQ(iter.GetName(), "2A"); break;
            case 2: ASSERT_STREQ(iter.GetName(), "2C"); break;
            case 3: ASSERT_STREQ(iter.GetName(), "3A"); break;
            case 4: ASSERT_STREQ(iter.GetName(), "3B"); break;
            case 5: ASSERT_STREQ(iter.GetName(), "3C"); break;
            default:
                ASSERT_TRUE(false);
                break;
            }
        }

    for (ViewDefinition::Entry const& iter : ViewDefinition::MakeIterator(*m_db, "WHERE IsPrivate=TRUE"))
        {
        ASSERT_STREQ(iter.GetName(), "2B");
        }

    view3B->SetPropertyValue("IsPrivate", true);
    EXPECT_EQ(DgnDbStatus::Success, view3B->Update());
    ASSERT_TRUE(view3B->IsPrivate());
    ASSERT_TRUE(view3B->GetPropertyValueBoolean("IsPrivate"));
    ASSERT_EQ(2, ViewDefinition::QueryCount(*m_db, "WHERE IsPrivate=TRUE"));
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(ViewDefinitionTests, QueryDefaultViewId)
    {
    SetupSeedProject();
    PhysicalModelPtr physicalModel = DgnDbTestUtils::InsertPhysicalModel(*m_db, "PhysicalModel");
    DocumentListModelPtr documentListModel = DgnDbTestUtils::InsertDocumentListModel(*m_db, "DocumentList");
    DrawingPtr drawing = DgnDbTestUtils::InsertDrawing(*documentListModel, "Drawing");
    DrawingModelPtr drawingModel = DgnDbTestUtils::InsertDrawingModel(*drawing);
    Sheet::ElementPtr sheet = DgnDbTestUtils::InsertSheet(*documentListModel, 1.0, 1.0, 1.0, "Sheet");
    Sheet::ModelPtr sheetModel = DgnDbTestUtils::InsertSheetModel(*sheet);

    ASSERT_EQ(0, ViewDefinition::QueryCount(*m_db));
    ASSERT_FALSE(ViewDefinition::QueryDefaultViewId(*m_db).IsValid());
    ASSERT_EQ(BentleyStatus::SUCCESS, m_db->Schemas().CreateClassViewsInDb());

    DrawingViewDefinitionPtr privateDrawingView = DgnDbTestUtils::InsertDrawingView(*drawingModel, "PrivateDrawingView");
    privateDrawingView->SetIsPrivate(true);
    EXPECT_EQ(DgnDbStatus::Success, privateDrawingView->Update());
    ASSERT_EQ(1, ViewDefinition::QueryCount(*m_db));
    ASSERT_FALSE(ViewDefinition::QueryDefaultViewId(*m_db).IsValid()); // private DrawingViews not considered

    DrawingViewDefinitionPtr publicDrawingView = DgnDbTestUtils::InsertDrawingView(*drawingModel, "PublicDrawingView");
    ASSERT_EQ(2, ViewDefinition::QueryCount(*m_db));
    ASSERT_EQ(ViewDefinition::QueryDefaultViewId(*m_db), publicDrawingView->GetViewId());

    InsertSheetView(*sheetModel, "PrivateSheetView", true);
    ASSERT_EQ(3, ViewDefinition::QueryCount(*m_db));
    ASSERT_EQ(ViewDefinition::QueryDefaultViewId(*m_db), publicDrawingView->GetViewId()); // private SheetViews not considered

    SheetViewDefinitionCPtr publicSheetView = InsertSheetView(*sheetModel, "PublicSheetView", false);
    ASSERT_EQ(4, ViewDefinition::QueryCount(*m_db));
    ASSERT_EQ(ViewDefinition::QueryDefaultViewId(*m_db), publicSheetView->GetViewId()); // public SheetViews take precendence over public DrawingViews

    OrthographicViewDefinitionPtr privateSpatialView = InsertSpatialView(*physicalModel, "PrivateSpatialView", true);
    ASSERT_EQ(5, ViewDefinition::QueryCount(*m_db));
    ASSERT_EQ(ViewDefinition::QueryDefaultViewId(*m_db), publicSheetView->GetViewId()); // private SpatialViews not considered

    OrthographicViewDefinitionPtr publicSpatialView = InsertSpatialView(*physicalModel, "PublicSpatialView", false);
    ASSERT_EQ(6, ViewDefinition::QueryCount(*m_db));
    ASSERT_EQ(ViewDefinition::QueryDefaultViewId(*m_db), publicSpatialView->GetViewId()); // public SpatialViews take precendence over public SheetViews

    DgnViewId defaultViewId(m_db->Elements().GetRootSubjectId().GetValue()); // not a valid view
    m_db->SaveProperty(DgnViewProperty::DefaultView(), &defaultViewId, sizeof(defaultViewId));
    ASSERT_EQ(ViewDefinition::QueryDefaultViewId(*m_db), publicSpatialView->GetViewId()); // invalid default view property should be ignored

    defaultViewId = privateSpatialView->GetViewId();
    m_db->SaveProperty(DgnViewProperty::DefaultView(), &defaultViewId, sizeof(defaultViewId));
    ASSERT_EQ(ViewDefinition::QueryDefaultViewId(*m_db), privateSpatialView->GetViewId()); // valid default view property should take precedence
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ViewDefinitionTests, ViewDefinition2dCRUD)
    {
    SetupSeedProject();
    DgnViewId DviewDefid, SviewDefid,RviewDefid;
    if (true)
        {
        DefinitionModelR dictionary = m_db->GetDictionaryModel();
        DocumentListModelPtr DocListModel = DgnDbTestUtils::InsertDocumentListModel(*m_db, "DrawingListModel");
        DrawingPtr drawing = DgnDbTestUtils::InsertDrawing(*DocListModel, "TestDrawingModel");
        DrawingModelPtr drawingModel = DgnDbTestUtils::InsertDrawingModel(*drawing);

        CategorySelectorPtr categories = new CategorySelector(dictionary, "");
        for (ElementIteratorEntryCR categoryEntry : DrawingCategory::MakeIterator(*m_db))
            categories->AddCategory(categoryEntry.GetId<DgnCategoryId>());

        DisplayStyle2dPtr style = new DisplayStyle2d(dictionary, "");
        ASSERT_TRUE(style.IsValid());
        Render::ViewFlags flags = style->GetViewFlags();
        flags.SetRenderMode(Render::RenderMode::SmoothShade);
        style->SetViewFlags(flags);
        //Create a  DrawingView
        DrawingViewDefinitionPtr viewDef = new DrawingViewDefinition(dictionary, "DrawingView", drawingModel->GetModelId(), *categories, *style);
        ASSERT_TRUE(viewDef.IsValid());
        ASSERT_EQ(viewDef->GetName(), "DrawingView");
        viewDef->SetDescription("DrawingView Descr");
        ASSERT_EQ(viewDef->GetDescription(), "DrawingView Descr");
        ASSERT_TRUE(viewDef->IsDrawingView());
        viewDef->SetIsPrivate(false);
        ASSERT_FALSE(viewDef->IsPrivate());
        //Insert DrawingView
        ViewDefinitionCPtr viewDefele = viewDef->Insert();
        ASSERT_TRUE(viewDefele.IsValid());
        DviewDefid = viewDefele->GetViewId();
        ASSERT_TRUE(DviewDefid == ViewDefinition::QueryViewId(dictionary, "DrawingView"));
        //Create SheetView
        Sheet::ElementPtr sheet = DgnDbTestUtils::InsertSheet(*DocListModel, 1.0, 1.0, 1.0, "MySheet");
        Sheet::ModelPtr sheetModel = DgnDbTestUtils::InsertSheetModel(*sheet);
        SheetViewDefinitionPtr sheetView=new SheetViewDefinition(dictionary, "MySheetView", sheetModel->GetModelId(), *categories, *style);
        ASSERT_TRUE(sheetView.IsValid());
        ASSERT_EQ(sheetView->GetName(), "MySheetView");
        sheetView->SetDescription("SheetView Descr");
        ASSERT_EQ(sheetView->GetDescription(), "SheetView Descr");
        ASSERT_TRUE(sheetView->IsSheetView());
        sheetView->SetIsPrivate(false);
        ASSERT_FALSE(sheetView->IsPrivate());
        //Insert SheetView
        ViewDefinitionCPtr sheetviewele=sheetView->Insert();
        SviewDefid = sheetviewele->GetViewId();
        ASSERT_TRUE(SviewDefid == ViewDefinition::QueryViewId(dictionary, "MySheetView"));
        }
    BeFileName fileName = m_db->GetFileName();
    m_db->SaveChanges();
    m_db->CloseDb();
    m_db = nullptr;
    //Check what stored in Db and then Update DrawingViewDefinition
    OpenDb(m_db, fileName, Db::OpenMode::ReadWrite, true);
    {
    //DrawingViewDefinition
    DrawingViewDefinitionPtr viewDef = m_db->Elements().GetForEdit<DrawingViewDefinition>(DviewDefid);
    ASSERT_TRUE(viewDef.IsValid());
    DisplayStyleR style = viewDef->GetDisplayStyle();
    ASSERT_EQ(style.GetViewFlags().GetRenderMode(), Render::RenderMode::SmoothShade);
    Render::ViewFlags flags = style.GetViewFlags();
    flags.SetRenderMode(Render::RenderMode::SolidFill);
    style.SetViewFlags(flags);
    ASSERT_EQ(style.GetViewFlags().GetRenderMode(), Render::RenderMode::SolidFill);
    style.SetBackgroundColor(ColorDef::Red());
    ASSERT_EQ(style.GetBackgroundColor(), ColorDef::Red());
    ASSERT_EQ(viewDef->GetDescription(), "DrawingView Descr");
    viewDef->SetDescription("Descr");
    ASSERT_EQ(viewDef->GetDescription(), "Descr");
    EXPECT_EQ(DgnDbStatus::Success, viewDef->Update());
    EXPECT_EQ(DgnDbStatus::Success, style.Update());
    //SheetViewDefinition
    SheetViewDefinitionPtr SviewDef = m_db->Elements().GetForEdit<SheetViewDefinition>(SviewDefid);
    ASSERT_TRUE(SviewDef.IsValid());
    DisplayStyleR Sstyle = SviewDef->GetDisplayStyle();
    ASSERT_EQ(Sstyle.GetViewFlags().GetRenderMode(), Render::RenderMode::SolidFill);
    ASSERT_EQ(Sstyle.GetBackgroundColor(), ColorDef::Red());
    ASSERT_EQ(SviewDef->GetDescription(), "SheetView Descr");
    SviewDef->SetDescription("Descr");
    ASSERT_EQ(SviewDef->GetDescription(), "Descr");
    EXPECT_EQ(DgnDbStatus::Success, SviewDef->Update());
    }
    m_db->SaveChanges();
    m_db->CloseDb();
    m_db = nullptr;
    //Check update values are saved in Db
    OpenDb(m_db, fileName, Db::OpenMode::ReadWrite, true);
    // DrawingViewDefinition
    DrawingViewDefinitionPtr viewDef = m_db->Elements().GetForEdit<DrawingViewDefinition>(DviewDefid);
    ASSERT_TRUE(viewDef.IsValid());
    DisplayStyleR style = viewDef->GetDisplayStyle();
    ASSERT_EQ(style.GetViewFlags().GetRenderMode(), Render::RenderMode::SolidFill);
    ASSERT_EQ(style.GetBackgroundColor(), ColorDef::Red());
    ASSERT_EQ(viewDef->GetDescription(), "Descr");
    // SheetViewDefinition
    SheetViewDefinitionPtr SviewDef = m_db->Elements().GetForEdit<SheetViewDefinition>(SviewDefid);
    ASSERT_TRUE(SviewDef.IsValid());
    ASSERT_EQ(SviewDef->GetDescription(), "Descr");
    EXPECT_EQ(DgnDbStatus::Success, SviewDef->Update());

    // Delete the Views
    ASSERT_NE(DgnDbStatus::Success, viewDef->Delete()); // ViewDefinitions can only be deleted via "purge"
    ASSERT_NE(DgnDbStatus::Success, SviewDef->Delete()); // ViewDefinitions can only be deleted via "purge"
    DgnDb::PurgeOperation purgeOperation(*m_db); // Give test permission to delete ViewDefinition (normally reserved for "purge" operations)
    ASSERT_EQ(DgnDbStatus::Success, viewDef->Delete());
    ASSERT_EQ(DgnDbStatus::Success, SviewDef->Delete());
    ASSERT_EQ(0, ViewDefinition::QueryCount(*m_db));
    m_db->SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ViewDefinitionTests, PlanProjectionSettings)
    {
    using OptDbl = OptionalDouble;
    using Settings = DisplayStyle3d::PlanProjectionSettings;
    using Map = DisplayStyle3d::PlanProjectionSettingsMap;

    struct Entry
    {
        DgnModelId m_modelId;
        Settings m_settings;

        Entry(int modelId, OptDbl const& elevation, OptDbl const& transp, bool overlay, bool priority)
            {
            m_modelId = DgnModelId(static_cast<uint64_t>(modelId));
            if (elevation) m_settings.m_elevation = elevation;
            if (transp) m_settings.m_transparency = transp;
            m_settings.m_drawAsOverlay = overlay;
            m_settings.m_enforceDisplayPriority = priority;
            }
    };

    using Entries = std::vector<Entry>;
    auto makeMap = [](Entries const& entries)
        {
        Map map;
        for (auto const& entry : entries)
            map.Insert(entry.m_modelId, entry.m_settings);

        return map;
        };

    SetupSeedProject();
    DisplayStyle3dPtr style = new DisplayStyle3d(m_db->GetDictionaryModel(), "");

    auto test = [&](Entries const& input, Entries const& expected)
        {
        auto inputMap = makeMap(input);
        style->SetPlanProjectionSettings(&inputMap);

        auto outputMap = style->GetPlanProjectionSettings();

        auto size1=outputMap.size();
        auto size2=expected.size();
        EXPECT_EQ(size1, size2);
        for (auto const& kvp : outputMap)
            {
            auto found = std::find_if(expected.begin(), expected.end(), [&](Entry const& entry) { return entry.m_modelId == kvp.first; });
            EXPECT_TRUE(found != expected.end());
            if (found == expected.end())
                continue;

            auto const& exp = found->m_settings;
            auto const& act = kvp.second;
            EXPECT_EQ(act, exp);
            }
        };

    OptDbl empty;
    test({ }, { });
    test({ Entry(0, 1, 0, true, true) }, { });
    test(
        { Entry(0x1c, empty, empty, false, false) },
        { });
    test(
        { Entry(0x1c, 2, 0.5, true, true) },
        { Entry(0x1c, 2, 0.5, true, true) });
    test(
        { Entry(0x1c, empty, 12, false, true) },
        { Entry(0x1c, empty, 1, false, true) });
    test(
        { Entry(0x1c, empty, -0.01, true, false) },
        { Entry(0x1c, empty, 0, true, false) });
    test(
        {
            Entry(0x1, 0.5, empty, false, false),
            Entry(0x2, empty, 0.5, false, false),
            Entry(0x3, empty, empty, true, false),
            Entry(0x4, empty, empty, false, true),
            Entry(0x5, empty, empty, false, false)
        },
        {
            Entry(0x1, 0.5, empty, false, false),
            Entry(0x2, empty, 0.5, false, false),
            Entry(0x3, empty, empty, true, false),
            Entry(0x4, empty, empty, false, true)
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BackgroundMap, Defaults)
    {
    BeJsDocument json;
    auto test = [&]()
        {
        EXPECT_FALSE(json.isNull());
        EXPECT_TRUE(json.isObject());
        EXPECT_EQ(json.size(), 1);
        Utf8String providerName(json["providerName"].asString("WRONG"));
        EXPECT_TRUE(providerName.Equals("BingProvider"));
        };

    BackgroundMapProps props;
    props.ToJson(json);
    test();

    props = BackgroundMapProps::FromJson(BeJsDocument::Null());
    props.ToJson(json);
    test();

    props = BackgroundMapProps::FromJson(BeJsDocument());
    props.ToJson(json);
    test();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BackgroundMap, RoundTrip)
    {
    auto test = [](BackgroundMapPropsCR input)
        {
        BeJsDocument json;
        input.ToJson(json);
        auto output = BackgroundMapProps::FromJson(json);
        EXPECT_TRUE(output == input);
        };

    BackgroundMapProps props(BackgroundMapProviderType::MapBox, BackgroundMapType::Street);
    props.m_groundBias += 2.5;
    props.m_useDepthBuffer = !props.m_useDepthBuffer;
    props.m_applyTerrain = !props.m_applyTerrain;
    props.m_globeMode = GlobeMode::Plane;
    props.SetTransparency(0.5);

    test(props);
    BeJsDocument v;
    props.ToJson(v);
    EXPECT_FALSE(v.isMember("terrainSettings"));

    props.m_terrain.m_exaggeration += 3.2;
    props.m_terrain.m_heightOrigin -= 2.3;
    props.m_terrain.m_heightOriginMode = TerrainHeightOriginMode::Geoid;
    props.m_terrain.m_applyLighting = !props.m_terrain.m_applyLighting;

    test(props);
    props.ToJson(v);
    EXPECT_TRUE(v.isMember("terrainSettings"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BackgroundMap, Transparency)
    {
    auto test = [](BackgroundMapPropsCR props, double expectedTransparency)
        {
        BeJsDocument json;
        props.ToJson(json);
        EXPECT_TRUE(json["transparency"].isNumeric());
        EXPECT_EQ(json["transparency"].asDouble(1234.5678), expectedTransparency);
        };

    BackgroundMapProps props;
    props.SetTransparency(0.5);
    test(props, 0.5);

    props.SetTransparency(0);
    test(props, 0);

    props.SetTransparency(-1);
    test(props, 0);

    props.SetTransparency(1);
    test(props, 1);

    props.SetTransparency(100);
    test(props, 1);

    props.ClearTransparency();
    BeJsDocument json;
    props.ToJson(json);
    EXPECT_FALSE(json.isMember("transparency"));
    }

TEST_F(ViewDefinitionTests, ModelAppearanceOverrides)
    {
    SetupSeedProject();

    DgnModelId                      modelId;
    ModelAppearanceOverrides        overrides, foundOverrides;
    EXPECT_FALSE(overrides.IsAnyOverridden());
    overrides.SetTransparency(.123);
    overrides.SetColor(ColorDef::Orange());
    overrides.SetEmphasized(true);
    overrides.SetIgnoresMaterial(true);
    overrides.SetWeight(15);
    overrides.SetLinePixels(Render::LinePixels::Code3);

    BeJsDocument json;
    overrides.ToJson(json, modelId);
    auto roundTrip = ModelAppearanceOverrides::FromJson(json);

    EXPECT_TRUE(overrides.IsEqual(roundTrip));
    EXPECT_TRUE(overrides.IsAnyOverridden());

    DefinitionModelR dictionary = m_db->GetDictionaryModel();
    DisplayStyle3dPtr displayStyle = new DisplayStyle3d(dictionary, "");

    EXPECT_FALSE(displayStyle->GetModelAppearanceOverrides(foundOverrides, modelId));

    displayStyle->OverrideModelAppearance(modelId, overrides);
    EXPECT_TRUE(displayStyle->GetModelAppearanceOverrides(foundOverrides, modelId));
    EXPECT_TRUE(foundOverrides.IsEqual(overrides));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ViewDefinitionTests, OmitScheduleScriptElementIds)
    {
    SetupSeedProject();

    DisplayStyle3dPtr style(new DisplayStyle3d(m_db->GetDictionaryModel(), ""));
    style->SetStyle("cubism", BeJsDocument("picasso", true));

    // DisplayStyle has its own (non-virtual) ToJson() method which is protected...
    DgnElementCR styleElem = *style;
    BeJsDocument json;
    styleElem.ToJson(json);
    bool eq = json["jsonProperties"]["styles"]  == style->GetJsonProperties("styles");
    EXPECT_TRUE(eq);

    BeJsDocument script;
    auto model = script.appendValue();
    model["modelId"] = "0x123";

    auto elem1 = model["elementTimelines"].appendValue();
    elem1["elementIds"] = "+2+3*5+1";
    elem1["batchId"] = 1;

    auto elem2 = model["elementTimelines"].appendValue();
    elem2["elementIds"].appendValue() = "0x456";
    elem2["elementIds"].appendValue() = "0xfed";
    elem2["batchId"] = 2;

    style->SetStyle("scheduleScript", script);

    styleElem.ToJson(json);
    EXPECT_EQ(json["jsonProperties"]["styles"], style->GetJsonProperties("styles"));

    BeJsDocument opts;
    opts["displayStyle"]["omitScheduleScriptElementIds"] = false;
    styleElem.ToJson(json, opts);
    EXPECT_EQ(json["jsonProperties"]["styles"], style->GetJsonProperties("styles"));

    opts["displayStyle"]["omitScheduleScriptElementIds"] = true;
    styleElem.ToJson(json, opts);
    EXPECT_NE(json["jsonProperties"]["styles"], style->GetJsonProperties("styles"));
    EXPECT_EQ(json["jsonProperties"]["styles"]["cubism"].asString(), "picasso");

    auto stripped = json["jsonProperties"]["styles"]["scheduleScript"];
    EXPECT_EQ(stripped.size(), 1);
    EXPECT_EQ(stripped[0]["modelId"].asString(), "0x123");

    auto elems = stripped[0]["elementTimelines"];
    EXPECT_EQ(elems.size(), 2);
    EXPECT_EQ(elems[0]["batchId"], elem1["batchId"]);
    EXPECT_EQ(elems[0]["elementIds"].asString(), "");
    EXPECT_EQ(elems[1]["batchId"], elem2["batchId"]);
    EXPECT_EQ(elems[1]["elementIds"].asString(), "");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ViewDefinitionTests, ExcludedElements)
    {
    SetupSeedProject();

    enum struct Compress { No, Yes, Unspecified };

    struct Style : DisplayStyle3d
    {
    private:
        BeJsDocument json;
        Style(DgnDbR db) : DisplayStyle3d(db.GetDictionaryModel(), "") { }
    public:
        // DisplayStyle has a private ToJson() returning a string, for some reason.
        using DgnElement::ToJson;

        static RefCountedPtr<Style> Create(DgnDbR db) { return new Style(db); }

        BeJsConst PersistentJson() const { return GetStyle("excludedElements"); }
        BeJsConst WireJson(Compress compress)
            {
            BeJsDocument opts;
            if (Compress::Unspecified != compress)
                opts["displayStyle"]["compressExcludedElementIds"] = Compress::Yes == compress;

            json.SetEmptyObject();
            ToJson(json, opts);
            return json["jsonProperties"]["styles"]["excludedElements"];
            }

        void SetPersistentJson(BeJsConst json)
            {
            GetStylesR()["excludedElements"].From(json);
            }
    };

    auto style = Style::Create(*m_db);
    EXPECT_TRUE(style->GetExcludedElements().empty());
    EXPECT_TRUE(style->PersistentJson().isNull());
    EXPECT_TRUE(style->WireJson(Compress::No).isNull());
    EXPECT_TRUE(style->WireJson(Compress::Yes).isNull());
    EXPECT_TRUE(style->WireJson(Compress::Unspecified).isNull());

    DgnElementIdSet excluded;
    excluded.insert(DgnElementId(uint64_t(1)));
    excluded.insert(DgnElementId(uint64_t(9)));

    BeJsDocument jsonIdStr(excluded.GetBeIdSet().ToCompactString(), true);
    bset<BeInt64Id> ids = excluded.GetBeIdSet();
    BeJsDocument jsonIds;
    for (auto const& id : ids)
        jsonIds.appendValue() = id.ToHexStr();

    // We persist the Ids in compressed format. ToJson() returns them in requested format (uncompressed if unspecified).
    style->SetExcludedElements(excluded);
    EXPECT_EQ(style->GetExcludedElements().GetBeIdSet(), ids);
    EXPECT_EQ(style->PersistentJson(), jsonIdStr);
    EXPECT_EQ(style->WireJson(Compress::No), jsonIds);
    EXPECT_EQ(style->WireJson(Compress::Yes), jsonIdStr);
    EXPECT_EQ(style->WireJson(Compress::Unspecified), jsonIds);

    // Older builds persisted the Ids as an uncompressed array. Test that.
    style->SetPersistentJson(jsonIds);
    EXPECT_EQ(style->GetExcludedElements().GetBeIdSet(), ids);
    EXPECT_EQ(style->PersistentJson(), jsonIds);
    EXPECT_EQ(style->WireJson(Compress::No), jsonIds);
    EXPECT_EQ(style->WireJson(Compress::Yes), jsonIdStr);
    EXPECT_EQ(style->WireJson(Compress::Unspecified), jsonIds);

    // We omit from JSON if no excluded elements.
    excluded.clear();
    style->SetExcludedElements(excluded);
    EXPECT_TRUE(style->GetExcludedElements().empty());
    EXPECT_TRUE(style->PersistentJson().isNull());
    EXPECT_TRUE(style->WireJson(Compress::No).isNull());
    EXPECT_TRUE(style->WireJson(Compress::Yes).isNull());
    EXPECT_TRUE(style->WireJson(Compress::Unspecified).isNull());

    excluded.insert(DgnElementId(uint64_t(123)));
    excluded.insert(DgnElementId(uint64_t(456)));
    style->SetExcludedElements(excluded);
    EXPECT_EQ(style->GetExcludedElements().GetBeIdSet(), excluded.GetBeIdSet());

    excluded.clear();
    excluded.insert(DgnElementId(uint64_t(789)));
    style->SetExcludedElements(excluded);
    EXPECT_EQ(style->GetExcludedElements().GetBeIdSet(), excluded.GetBeIdSet());
    }
