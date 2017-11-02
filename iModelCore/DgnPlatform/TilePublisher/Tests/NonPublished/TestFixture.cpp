/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/Tests/NonPublished/TestFixture.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "TestFixture.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TestFixture::SetupDb(WCharCP name)
    {
    ASSERT_TRUE(m_db.IsNull());

    BeFileName filename = GetBaseDir();
    filename.AppendToPath(name);
    filename.append(L".bim");
    BeFileName::BeDeleteFile(filename);

    CreateDgnDbParams params;
    params.SetRootSubjectName("TilePublisherTests");
    params.SetOverwriteExisting(false);

    BeSQLite::DbResult status;
    m_db = DgnDb::CreateDgnDb(&status, filename, params);
    ASSERT_EQ(BeSQLite::BE_SQLITE_OK, status) << status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
ModelSelectorCPtr TestFixture::InsertModelSelector(DgnModelIdSet const& modelIds, Utf8CP name)
    {
    DgnDbR db = GetDb();
    ModelSelector sel(db.GetDictionaryModel(), name);
    sel.GetModelsR() = modelIds;
    
    auto ret = db.Elements().Insert(sel);
    EXPECT_TRUE(ret.IsValid());

    return ret;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
CategorySelectorCPtr TestFixture::InsertCategorySelector(DgnCategoryIdSet const& catIds, Utf8CP name)
    {
    DgnDbR db = GetDb();
    CategorySelector sel(db.GetDictionaryModel(), name);
    sel.GetCategoriesR() = catIds;

    auto ret = db.Elements().Insert(sel);
    EXPECT_TRUE(ret.IsValid());

    return ret;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayStyle3dCPtr TestFixture::InsertDisplayStyle3d(Utf8CP name, ColorDef bgColor, bool groundPlane, ViewFlags vf)
    {
    DisplayStyle3d style(GetDb().GetDictionaryModel(), name);
    style.SetBackgroundColor(bgColor);
    style.SetGroundPlaneEnabled(groundPlane);
    style.SetViewFlags(vf);

    auto ret = GetDb().Elements().Insert(style);
    EXPECT_TRUE(ret.IsValid());
    return ret;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialViewDefinitionCPtr TestFixture::InsertSpatialView(Utf8CP name, ModelSelectorCR models, CategorySelectorCR categories, DisplayStyle3dCR style, DRange3dCP viewedVolume, SpatialViewDefinition::Camera const* camera)
    {
    SpatialViewDefinition view(GetDb().GetDictionaryModel(), name, *DgnElement::MakeCopy(categories), *DgnElement::MakeCopy(style), *DgnElement::MakeCopy(models), camera);
    auto ret = GetDb().Elements().Insert(view);
    EXPECT_TRUE(ret.IsValid());
    if (nullptr != viewedVolume)
        {
        ViewControllerPtr vc = view.LoadViewController();
        vc->GetViewDefinitionR().LookAtVolume(*viewedVolume);
        ret = static_cast<SpatialViewDefinitionCP>(vc->GetViewDefinitionR().Update().get());
        EXPECT_TRUE(ret.IsValid());
        }

    return ret;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr TestFixture::CreateRectangle(DPoint3dCR ll, double w, double h)
    {
    double top = ll.y + h,
           right = ll.x + w;

    DPoint3d pts[5] =
        {
        ll,
        DPoint3d::From(ll.x, top, ll.z),
        DPoint3d::From(right, top, ll.z),
        DPoint3d::From(right, ll.y, ll.z),
        ll
        };

    return CreateShape(pts, 5);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr TestFixture::CreateTriangle(DPoint3dCR ll, double w, double h)
    {
    auto curve = CreateRectangle(ll, w, h);
    auto& shape = *(curve->at(0)->GetLineStringP());
    shape.erase(shape.begin() + 2);
    return curve;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalElementCPtr TestFixture::InsertPhysicalElement(PhysicalModelR model, GeometryBuilderR builder, DPoint3dCR origin)
    {
    auto elem = GenericPhysicalObject::Create(model, builder.GetGeometryParams().GetCategoryId());
    EXPECT_TRUE(elem.IsValid());

    builder.Finish(*elem);

    Placement3d placement = elem->GetPlacement();
    placement.GetOriginR() = origin;
    elem->SetPlacement(placement);

    auto ret = GetDb().Elements().Insert(*elem);
    EXPECT_TRUE(ret.IsValid());
    return ret.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryBuilderPtr TestFixture::CreateGeometryBuilder(DgnModelR model, DgnCategoryId categoryId, ColorDef color)
    {
    auto geom = CreateGeometryBuilder(model, categoryId);
    GeometryParams params(categoryId);
    params.SetFillColor(color);
    params.SetLineColor(color);
    geom->Append(params);
    return geom;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
Cesium::TilesetPublisher::Status TestFixture::PublishTiles()
    {
    UpdateProjectExtents();
    SaveDb();

    auto filename = GetDb().GetFileName();
    Cesium::PublisherParams params(filename, GetBaseDir(), GetTilesetNameW(), /*copyScripts=*/ false);

    DgnViewIdSet views;
    DgnViewId defaultView = params.GetViewIds(views, GetDb());
    EXPECT_TRUE(defaultView.IsValid());
    EXPECT_FALSE(views.empty());

    Cesium::TilesetPublisher publisher(GetDb(), params, views, defaultView, 5);
    return publisher.Publish(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName TestFixture::GetTilesetDir() const
    {
    auto filename = GetBaseDir();
    filename.AppendToPath(L"Tilesets");
    filename.AppendToPath(GetTilesetNameW().c_str());
    return filename;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName TestFixture::GetAppDataFileName() const
    {
    BeFileName filename = GetTilesetDir();
    filename.AppendToPath(GetTilesetNameW().c_str());
    filename.append(L"_AppData.json");
    return filename;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void AppData::Read(BeFileNameCR filename)
    {
    BeFile file;
    ASSERT_EQ(BeFileStatus::Success, file.Open(filename.c_str(), BeFileAccess::Read));
    bvector<Byte> bytes;
    ASSERT_EQ(BeFileStatus::Success, file.ReadEntireFile(bytes));
    ASSERT_FALSE(bytes.empty());

    auto begin = reinterpret_cast<Utf8CP>(bytes.data());
    auto end = begin + bytes.size();

    Json::Value json;
    Json::Reader reader(Json::Features::strictMode());
    ASSERT_TRUE(reader.parse(begin, end, json));

    Read(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void AppData::Read(Json::Value const& json)
    {
    m_defaultView = DgnViewId(json["defaultView"].asUInt64());
    m_groundPoint = JsonUtil::ToPoint(json["groundPoint"]);
    m_name = json["name"].asString();
    m_projectExtents = JsonUtil::ToRange(json["projectExtents"]);
    m_projectOrigin = JsonUtil::ToPoint(json["projectOrigin"]);
    m_projectTransform = JsonUtil::ToTransform(json["projectTransform"]);

    auto const& catJson = json["categories"];
    for (auto const& catId : catJson.getMemberNames())
        m_categories.Insert(JsonUtil::ToId<DgnCategoryId>(catId), catJson[catId].asString());

    auto const& catSelJson = json["categorySelectors"];
    for (auto const& selId : catSelJson.getMemberNames())
        m_categorySelectors.Insert(JsonUtil::ToElementId(selId), JsonUtil::ToIdSet<DgnCategoryIdSet>(catSelJson[selId]));

    auto const& styleJson = json["displayStyles"];
    for (auto const& styleId : styleJson.getMemberNames())
        m_displayStyles.Insert(JsonUtil::ToElementId(styleId), DisplayStyle(styleJson[styleId]));

    auto const& modSelJson = json["modelSelectors"];
    for (auto const& selId : modSelJson.getMemberNames())
        m_modelSelectors.Insert(JsonUtil::ToElementId(selId), JsonUtil::ToIdSet<DgnModelIdSet>(modSelJson[selId]));

    auto const& modJson = json["models"];
    for (auto const& modId : modJson.getMemberNames())
        m_models.Insert(JsonUtil::ToId<DgnModelId>(modId), Model(modJson[modId]));

    auto const& viewJson = json["views"];
    for (auto const& viewId : viewJson.getMemberNames())
        m_views.Insert(JsonUtil::ToId<DgnViewId>(viewId), View(viewJson[viewId]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void AppData::ExpectEqual(AppData const& rhs) const
    {
    EXPECT_EQ(m_defaultView, rhs.m_defaultView);
    EXPECT_EQ(m_name, rhs.m_name);

    EXPECT_TRUE(m_groundPoint.IsEqual(rhs.m_groundPoint));
    EXPECT_TRUE(m_projectExtents.IsEqual(rhs.m_projectExtents));
    EXPECT_TRUE(m_projectOrigin.IsEqual(rhs.m_projectOrigin));
    EXPECT_TRUE(m_projectTransform.IsEqual(rhs.m_projectTransform));

    ExpectEqual(m_categories, rhs.m_categories);
    ExpectEqual(m_categorySelectors, rhs.m_categorySelectors);
    ExpectEqual(m_displayStyles, rhs.m_displayStyles);
    ExpectEqual(m_modelSelectors, rhs.m_modelSelectors);
    ExpectEqual(m_models, rhs.m_models);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
AppData::DisplayStyle::DisplayStyle(Json::Value const& json)
    {
    m_backgroundColor = JsonUtil::ToColor(json["backgroundColor"]);
    m_isGlobeVisible = json["isGlobeVisible"].asBool();
    // ###TODO ViewFlags
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
AppData::Model::Model(Json::Value const& json)
    {
    m_extents = JsonUtil::ToRange(json["extents"]);
    m_name = json["name"].asString();
    m_tilesetUrl = json["tilesetUrl"].asString();
    m_transform = JsonUtil::ToTransform(json["transform"]);

    Utf8String type = json["type"].asString();
    constexpr Utf8CP types[4] = { "reality", "spatial", "sheet", "drawing" };
    for (size_t i = 0; i < 4; i++)
        {
        if (type.Equals(types[i]))
            {
            m_type = static_cast<Type>(i);
            break;
            }
        }

    EXPECT_FALSE(Type::Unknown == m_type);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
AppData::View::View(Json::Value const& json)
    {
    m_categorySelector = JsonUtil::ToElementId(json["categorySelector"].asString());
    m_displayStyle = JsonUtil::ToElementId(json["displayStyle"].asString());
    m_extents = JsonUtil::ToPoint(json["extents"]);
    m_eyePoint = JsonUtil::ToPoint(json["eyePoint"]);
    m_focusDistance = json["focusDistance"].asDouble();
    m_isCameraOn = json["isCameraOn"].asBool();
    m_lensAngle = json["lensAngle"].asDouble();
    m_modelSelector = JsonUtil::ToElementId(json["modelSelector"].asString());
    m_name = json["name"].asString();
    m_origin = JsonUtil::ToPoint(json["origin"]);
    m_rotation = JsonUtil::ToRotMatrix(json["rotation"]);

    Utf8String type = json["type"].asString();
    constexpr Utf8CP types[4] = { "camera", "ortho", "sheet", "drawing" };
    for (size_t i = 0; i < 4; i++)
        {
        if (type.Equals(types[i]))
            {
            m_type = static_cast<Type>(i);
            break;
            }
        }

    EXPECT_FALSE(Type::Unknown == m_type);
    }

