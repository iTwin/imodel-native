/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/Tests/NonPublished/TestFixture.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "TestFixture.h"
#include <Bentley/BeDirectoryIterator.h>

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
    auto extents = UpdateProjectExtents();
    SaveDb();

    auto filename = GetDb().GetFileName();
    Cesium::PublisherParams params(filename, GetBaseDir(), GetTilesetNameW());

    DgnViewIdSet views;
    DgnViewId defaultView = params.GetViewIds(views, GetDb());
    EXPECT_TRUE(defaultView.IsValid());
    EXPECT_FALSE(views.empty());

    Cesium::TilesetPublisher publisher(GetDb(), extents, params, views, defaultView, 5);
    return publisher.Publish(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName TestFixture::GetAppDataDir() const
    {
    auto filename = GetBaseDir();
    filename.AppendToPath(L"TileSets");
    filename.AppendToPath(GetTilesetNameW().c_str());
    return filename;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TestFixture::GetRelativeTilesetUrl(DgnModelId id, DgnDbR db)
    {
    WPrintfString modelName(L"Model_%" PRIx64 , id.GetValue());

    BeFileName url(L"TileSets");
    url.AppendToPath(GetTilesetNameW(db).c_str());
    url.AppendToPath(modelName.c_str());
    url.AppendToPath(modelName.c_str());
    url.AppendExtension(L"json");

    url.ReplaceAll(L"\\", L"//");
    return Utf8String(url.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName TestFixture::GetAppDataFileName() const
    {
    BeFileName filename = GetAppDataDir();
    filename.AppendToPath(GetTilesetNameW().c_str());
    filename.append(L"_AppData.json");
    return filename;
    }

DgnSubCategoryId TestFixture::InsertSubCategory (DgnCategoryId categoryId, Utf8CP name, ColorDefCR color)
    {
    DgnDbR db = GetDb();
    DgnSubCategory::Appearance appearance;
    appearance.SetColor(color);
    DgnSubCategoryPtr newSubCategory = new DgnSubCategory(DgnSubCategory::CreateParams(db, categoryId, name, appearance));
    if (!newSubCategory.IsValid())
        return DgnSubCategoryId();
    DgnSubCategoryCPtr insertedSubCategory = newSubCategory->Insert();
    if (!insertedSubCategory.IsValid())
        return DgnSubCategoryId();
    return insertedSubCategory->GetSubCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
AppData::AppData(SpatialViewDefinitionCR defaultView)
    {
    m_defaultView = defaultView.GetViewId();
    AddView(defaultView);

    auto& db = defaultView.GetDgnDb();
    m_name = Utf8String(db.GetFileName().GetFileNameWithoutExtension().c_str());
    m_projectExtents = db.GeoLocation().GetProjectExtents();
    // ###TODO transform, origin
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void AppData::Read(BeFileNameCR filename)
    {
    Json::Value json = JsonUtil::Read(filename);
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

    // ###TODO EXPECT_TRUE(m_groundPoint.IsEqual(rhs.m_groundPoint));
    // ###TODO EXPECT_TRUE(m_projectExtents.IsEqual(rhs.m_projectExtents));
    // ###TODO EXPECT_TRUE(m_projectOrigin.IsEqual(rhs.m_projectOrigin));
    // ###TODO EXPECT_TRUE(m_projectTransform.IsEqual(rhs.m_projectTransform));

    ExpectEqual(m_categories, rhs.m_categories);
    ExpectEqual(m_categorySelectors, rhs.m_categorySelectors);
    ExpectEqual(m_displayStyles, rhs.m_displayStyles);
    ExpectEqual(m_modelSelectors, rhs.m_modelSelectors);
    ExpectEqual(m_models, rhs.m_models);
    }

void AppData::VerifyMesh(Render::Primitives::Mesh const& mesh, int triangles, int polilines, int points, int normals, int params, ColorDefList const& colorDefs, bool areColorsUniform)
    {
    // verify mech
    EXPECT_FALSE(mesh.IsEmpty());
    EXPECT_EQ(Render::Primitives::Mesh::PrimitiveType::Mesh, mesh.GetType());
    EXPECT_FALSE(nullptr == mesh.GetFeatureTable());
    EXPECT_EQ(mesh.Triangles().Count(), triangles);
    EXPECT_EQ(mesh.Polylines().size(), polilines);
    EXPECT_EQ(mesh.Points().size(), points);
    EXPECT_EQ(mesh.Normals().size(), normals);
    EXPECT_EQ(mesh.Params().size(), params);

    // Verify colors. Again relying on order of element processing...
    Render::Primitives::ColorTableCR colors = mesh.GetColorTable();
    ASSERT_EQ(colors.size(), colorDefs.size());

    if (!areColorsUniform)
        EXPECT_FALSE(colors.IsUniform());
    else
        EXPECT_TRUE(colors.IsUniform());

    std::vector<bpair<uint32_t, uint16_t> > v;
    copy(colors.begin(), colors.end(), back_inserter(v));

    sort(v.begin(), v.end(),
         [] (const bpair<int, int>  &p1, const bpair<int, int> &p2)
            {
            return p1.second < p2.second;
            });

    int i = 0;
    auto iter = colorDefs.begin();
    for (auto const &item : v)
        {
        EXPECT_EQ(item.first, iter->GetValue());
        EXPECT_EQ(item.second, i);
        iter++;
        i++;
        }

    // Verify feature IDs
    Render::FeatureIndex feats;
    mesh.ToFeatureIndex(feats);

    EXPECT_FALSE(feats.IsUniform());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void AppData::AddView(SpatialViewDefinitionCR viewCR)
    {
    // Could look up the selectors + style by ID to avoid cloning the view, but lazy.
    auto view = DgnElement::MakeCopy(viewCR);
    m_views.Insert(view->GetViewId(), View(*view));
    m_displayStyles.Insert(view->GetDisplayStyleId(), DisplayStyle(view->GetDisplayStyle3d()));
    AddCategorySelector(view->GetCategorySelector());
    AddModelSelector(view->GetModelSelector());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void AppData::AddCategorySelector(CategorySelectorCR sel)
    {
    m_categorySelectors.Insert(sel.GetElementId(), sel.GetCategories());
    for (auto const& catId : sel.GetCategories())
        AddCategory(catId, sel.GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void AppData::AddModelSelector(ModelSelectorCR sel)
    {
    m_modelSelectors.Insert(sel.GetElementId(), sel.GetModels());
    for (auto const& modelId : sel.GetModels())
        {
        auto model = sel.GetDgnDb().Models().Get<GeometricModel3d>(modelId);
        ASSERT_TRUE(model.IsValid());
        AddModel(*model);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void AppData::AddModel(GeometricModel3dR model)
    {
    m_models.Insert(model.GetModelId(), Model(model));
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
AppData::DisplayStyle::DisplayStyle(DisplayStyle3dCR style)
    {
    m_backgroundColor = style.GetBackgroundColor();
    m_isGlobeVisible = style.IsGroundPlaneEnabled();
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
AppData::Model::Model(GeometricModel3dR model)
    {
    m_type = Type::Spatial;
    m_name = model.GetName();
    m_tilesetUrl = TestFixture::GetRelativeTilesetUrl(model.GetModelId(), model.GetDgnDb());
    m_extents = model.GetDgnDb().GeoLocation().GetProjectExtents();
    // ###TODO transform, extents
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
AppData::View::View(SpatialViewDefinitionCR view)
    {
    m_type = Type::Camera;
    m_categorySelector = view.GetCategorySelectorId();
    m_displayStyle = view.GetDisplayStyleId();
    m_modelSelector = view.GetModelSelectorId();
    m_extents = view.GetExtents();
    m_eyePoint = view.GetEyePoint();
    m_focusDistance = view.GetFocusDistance();
    m_isCameraOn = view.IsCameraOn();
    m_lensAngle = view.GetLensAngle().Radians();
    m_name = view.GetName();
    m_origin = view.GetOrigin();
    m_rotation = view.GetRotation();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
PublishedTilesets::PublishedTilesets(BeFileNameCR appDataDir)
    {
    BeFileName filename;
    bool isDir;
    for (BeDirectoryIterator iter(appDataDir); SUCCESS == iter.GetCurrentEntry(filename, isDir); iter.ToNext())
        {
        if (isDir)
            ProcessTilesetDir(filename);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId PublishedTilesets::ParseModelId(BeFileNameCR filename)
    {
    DgnModelId modelId;
    auto underPos = filename.rfind(L'_'),
         dotPos   = filename.rfind(L'.');

    if (WString::npos == underPos || WString::npos == dotPos || underPos >= dotPos)
        return modelId;

    underPos += 1; // skip the underscore
    WString hexW = filename.substr(underPos, dotPos-underPos);
    Utf8String hex(hexW);
    modelId = DgnModelId(BeStringUtilities::ParseHex(hex.c_str()));

    return modelId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PublishedTilesets::ProcessTilesetDir(BeFileNameCR dir)
    {
    BeFileName filename;
    bool isDir;
    for (BeDirectoryIterator iter(dir); SUCCESS == iter.GetCurrentEntry(filename, isDir); iter.ToNext())
        {
        EXPECT_FALSE(isDir);
        DgnModelId modelId = ParseModelId(filename);
        EXPECT_TRUE(modelId.IsValid());
        if (isDir || !modelId.IsValid() || filename.GetExtension().EqualsI(L"json"))
            continue;

        auto& entries = (*this)[modelId];
        entries.insert(PublishedTile(filename));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PublishedTilesets::ExpectEqual(AppData::Models const& models) const
    {
    EXPECT_EQ(models.size(), size());
    for (auto const& kvp : *this)
        EXPECT_FALSE(models.end() == models.find(kvp.first));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
PublishedTile::PublishedTile(BeFileNameCR filename) : m_filenameWithoutExtension(filename), m_format(TileIO::TileHeader::FormatFromFileName(filename))
    {
    EXPECT_FALSE(TileIO::Format::Unknown == m_format);
    auto pos = filename.rfind(L'.');
    EXPECT_FALSE(WString::npos == pos);
    m_filenameWithoutExtension.erase(pos);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value PublishedTile::ReadJson() const
    {
    BeFileName filename = m_filenameWithoutExtension;
    filename.AppendExtension(L"json");
    return JsonUtil::Read(filename);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value JsonUtil::Read(BeFileNameCR filename)
    {
    BeFile file;
    EXPECT_EQ(BeFileStatus::Success, file.Open(filename.c_str(), BeFileAccess::Read));
    bvector<Byte> bytes;
    EXPECT_EQ(BeFileStatus::Success, file.ReadEntireFile(bytes));
    EXPECT_FALSE(bytes.empty());

    auto begin = reinterpret_cast<Utf8CP>(bytes.data());
    auto end = begin + bytes.size();

    Json::Value json;
    Json::Reader reader(Json::Features::strictMode());
    EXPECT_TRUE(reader.parse(begin, end, json));

    return json;
    }

BEGIN_BENTLEY_RENDER_NAMESPACE

//=======================================================================================
// ###TODO: Move FakeRenderSystem.h from dgnplatform unit tests to backdoor or some
// other shared location.
// @bsistruct                                                   Paul.Connelly   11/17
//=======================================================================================
struct FakeSystem : System
{
protected:
    int _Initialize(void*, bool) override { return SUCCESS; }

    TargetPtr _CreateTarget(Device& dev, double tileSizeMod) override { BeAssert(false); return nullptr; }
    TargetPtr _CreateOffscreenTarget(Device& dev, double tileSizeMod) { return _CreateTarget(dev, tileSizeMod); }

    MaterialPtr _FindMaterial(MaterialNameCR, DgnDbR) const override { return nullptr; }
    MaterialPtr _GetMaterial(RenderMaterialId id, DgnDbR db) const override { return nullptr; }
    MaterialPtr _CreateMaterial(Material::CreateParams const&, DgnDbR) const override { return nullptr; }
    TexturePtr _FindTexture(TextureNameCR, DgnDbR) const override { return nullptr; }
    TexturePtr _GetTexture(DgnTextureId, DgnDbR) const override { return nullptr; }
    TexturePtr _GetTexture(GradientSymbCR, DgnDbR) const override { return nullptr; }
    TexturePtr _CreateTexture(ImageCR, DgnDbR, Texture::CreateParams const&) const override { return nullptr; }
    TexturePtr _CreateTexture(ImageSourceCR, Image::BottomUp, DgnDbR, Texture::CreateParams const&) const override { return nullptr; }
    TexturePtr _CreateGeometryTexture(GraphicCR, DRange2dCR, bool, bool) const override { return nullptr; }
    LightPtr _CreateLight(Lighting::Parameters const&, DVec3dCP, DPoint3dCP) const override { return nullptr; }

    GraphicBuilderPtr _CreateGraphic(GraphicBuilder::CreateParamsCR params) const override { return nullptr; }

    GraphicPtr _CreateSprite(ISprite& sprite, DPoint3dCR location, DPoint3dCR xVec, int transparency, DgnDbR db) const { return nullptr; }
    GraphicPtr _CreateViewlet(GraphicBranch& branch, PlanCR, ViewletPosition const&) const { BeAssert(false); return nullptr; }
    GraphicPtr _CreateTriMesh(TriMeshArgsCR args, DgnDbR dgndb) const { return nullptr; }
    GraphicPtr _CreateIndexedPolylines(IndexedPolylineArgsCR args, DgnDbR dgndb) const { return nullptr; }
    GraphicPtr _CreateVisibleEdges(MeshEdgeArgsCR args, DgnDbR dgndb) const { return nullptr; }
    GraphicPtr _CreateSilhouetteEdges(SilhouetteEdgeArgsCR args, DgnDbR dgndb) const { return nullptr; }
    GraphicPtr _CreatePointCloud(PointCloudArgsCR args, DgnDbR dgndb) const { return nullptr; }
    GraphicPtr _CreateGraphicList(bvector<GraphicPtr>&& primitives, DgnDbR dgndb) const { return nullptr; }
    GraphicPtr _CreateBranch(GraphicBranch&& branch, DgnDbR dgndb, TransformCR transform, ClipVectorCP clips) const { return nullptr; }
    GraphicPtr _CreateBatch(GraphicR graphic, FeatureTable&& features) const { return nullptr; }

    uint32_t _GetMaxFeaturesPerBatch() const override { return 0x7fffffff; }
};

END_BENTLEY_RENDER_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::Primitives::GeometryCollection PublishedTile::ReadGeometry(GeometricModelR model) const
    {
    Render::Primitives::GeometryCollection geom;

    Utf8CP ext = TileIO::TileHeader::FileExtensionFromFormat(m_format);
    EXPECT_FALSE(nullptr == ext);
    if (nullptr == ext)
        return geom;

    BeFile file;
    StreamBuffer bytes;
    BeFileName filename = m_filenameWithoutExtension;
    filename.AppendExtension(WString(ext, true).c_str());
    EXPECT_EQ(BeFileStatus::Success, file.Open(filename.c_str(), BeFileAccess::Read));
    EXPECT_EQ(BeFileStatus::Success, file.ReadEntireFile(bytes));
    EXPECT_FALSE(bytes.empty());

    FakeSystem system;
    EXPECT_EQ(TileIO::ReadStatus::Success, TileIO::ReadWebTile(geom, bytes, model, system));

    return geom;
    }

