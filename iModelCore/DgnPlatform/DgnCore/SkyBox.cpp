/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/SkyBox.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d SpatialViewController::GetGroundExtents(DgnViewportCR vp) const
    {
    auto& displayStyle = GetSpatialViewDefinition().GetDisplayStyle3d();
    AxisAlignedBox3d extents;
    if (!displayStyle.IsGroundPlaneEnabled())
        return extents;

    double elevation = GetGroundElevation();

    DRay3d viewRay = {DPoint3d(), vp.GetZVector()};
    DPlane3d xyPlane = DPlane3d::FromOriginAndNormal(DPoint3d::From(0,0,elevation), DVec3d::From(0, 0, 1.0));

    // first determine whether the ground plane is displayed in the view
    Frustum worldFrust = vp.GetFrustum();
    for (DPoint3dCR pt : worldFrust.m_pts)
        {
        DPoint3d xyzPt;
        viewRay.origin = pt;
        double param;
        if (!viewRay.Intersect(xyzPt, param, xyPlane))
            return extents; // view does not show ground plane
        }

    extents = GetDgnDb().GeoLocation().GetProjectExtents();
    extents.low.z = extents.high.z = elevation;

    DPoint3d center = DPoint3d::FromInterpolate(extents.low, 0.5, extents.high);

    double radius = extents.low.Distance(extents.high); // we need a square, not a rectangle
    extents.InitFrom(center);
    extents.Extend(radius);
    extents.low.z = extents.high.z = elevation;
    return extents;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
double SpatialViewController::GetGroundElevation() const
    {
    auto& env = GetSpatialViewDefinition().GetDisplayStyle3d().GetEnvironmentDisplay();
    return env.m_groundPlane.m_elevation + GetDgnDb().GeoLocation().GetGlobalOrigin().z; // adjust for global origin
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewController::DrawGroundPlane(DecorateContextR context)
    {
    AxisAlignedBox3d extents = GetGroundExtents(*context.GetViewport());
    if (!extents.IsValid())
        return;

    DPoint3d pts[5];
    pts[0] = pts[1] = pts[4] = extents.low;
    pts[1].y = extents.high.y;
    pts[2] = pts[3] = extents.high;
    pts[3].y = extents.low.y;

    auto& viewDef= GetSpatialViewDefinition();
    auto& environment = viewDef.GetDisplayStyle3d().GetEnvironmentDisplay();

    bool above = viewDef.IsEyePointAbove(extents.low.z);
    double keyValues[] = {0.0, 0.25, 0.5}; // gradient goes from edge of rectangle (0.0) to center (1.0)...
    ColorDef color = above ? environment.m_groundPlane.m_aboveColor : environment.m_groundPlane.m_belowColor;
    ColorDef colors[] = {color, color, color};

    Byte alpha = above ? 0x80 : 0x85;
    colors[0].SetAlpha(0xff);
    colors[1].SetAlpha(alpha);
    colors[2].SetAlpha(alpha);

    GradientSymbPtr gradient = GradientSymb::Create();
    gradient->SetMode(Render::GradientSymb::Mode::Spherical);
    gradient->SetKeys(_countof(colors), colors, keyValues);

    GraphicParams params;
    params.SetLineColor(colors[0]);
    params.SetFillColor(ColorDef::White()); // Fill should be set to opaque white for gradient texture...
    params.SetGradient(gradient.get());

    Render::GraphicBuilderPtr graphic = context.CreateGraphic();
    graphic->ActivateGraphicParams(params);
    graphic->AddShape(5, pts, true);
    context.AddWorldDecoration(*graphic);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
Render::TexturePtr SpatialViewController::LoadTexture(Utf8CP fileName, Render::SystemCR system)
    {
#if defined (NEEDS_WORK_GROUND_PLANE)
    bool isHttp = (0 == strncmp("http:", fileName, 5) || 0 == strncmp("https:", fileName, 6));

    if (isHttp)
        {
        m_rootDir = m_rootUrl.substr(0, m_rootUrl.find_last_of("/"));
        }
#endif

    BeFile skyFile;
    if (BeFileStatus::Success != skyFile.Open(fileName, BeFileAccess::Read))
        return nullptr;

    ByteStream jpegData;
    if (BeFileStatus::Success != skyFile.ReadEntireFile(jpegData))
        return nullptr;

    ImageSource jpeg(ImageSource::Format::Jpeg, std::move(jpegData));
    return system._CreateTexture(jpeg, Image::Format::Rgba, Image::BottomUp::No);
    }

static Byte lerp(double t, Byte a, Byte b) {return a + t * double(b - a);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewController::LoadSkyBox(Render::SystemCR system)
    {
    Render::TexturePtr texture;

    auto& env = GetSpatialViewDefinition().GetDisplayStyle3d().GetEnvironmentDisplay();
    if (!env.m_skybox.m_jpegFile.empty())
        texture = LoadTexture(env.m_skybox.m_jpegFile.c_str(), system);

    // we didn't get a jpeg sky, just create a gradient
    if (!texture.IsValid())
        {
        enum {GRADIENT_PIXEL_COUNT=1024};

        // set up the gradient
        ByteStream buffer(GRADIENT_PIXEL_COUNT * sizeof(ColorDef));
        ColorDef* thisColor = (ColorDef*) buffer.GetDataP();
        for (int i = 0; i < GRADIENT_PIXEL_COUNT; ++i, ++thisColor)
            {
            double frac = (double) i / (double) GRADIENT_PIXEL_COUNT;
            ColorDef color1, color2;

            if (frac > 0.5)
                {
                color1 = env.m_skybox.m_nadirColor;
                color2 = env.m_skybox.m_groundColor;
                frac = 1.0 -(2.0 * (frac - 0.5));
                frac = pow(frac, env.m_skybox.m_groundExponent);
                }
            else
                {
                color1 = env.m_skybox.m_zenithColor;
                color2 = env.m_skybox.m_skyColor;
                frac = 2.0*frac;
                frac = pow(frac, env.m_skybox.m_skyExponent);
                }
            thisColor->SetRed(lerp(frac, color1.GetRed(), color2.GetRed()));
            thisColor->SetGreen(lerp(frac, color1.GetGreen(), color2.GetGreen()));
            thisColor->SetBlue(lerp(frac, color1.GetBlue(), color2.GetBlue()));
            thisColor->SetAlpha(0xff);
            }

        Render::Image image(1, GRADIENT_PIXEL_COUNT, std::move(buffer), Image::Format::Rgba);
        texture = system._CreateTexture(image);
        }

    Material::CreateParams matParams;
    matParams.SetShadows(false);
    m_skybox = system._CreateMaterial(matParams);

    Material::TextureMapParams mapParams;
    Material::Trans2x3 transform(0.0, 1.0, 0.0, 1.0, 0.0, 0.0);
    mapParams.SetTransform(&transform);
    m_skybox->_MapTexture(*texture, mapParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static DPoint2d getUVForDirection(DPoint3dCR direction, double rotation, double zOffset)
    {
    double radius = sqrt(direction.x*direction.x + direction.y*direction.y);
    double zValue = direction.z - radius * zOffset;
    double azimuth  = (atan2(direction.y, direction.x) + rotation) / msGeomConst_2pi;
    double altitude = atan2(zValue, radius);

    DPoint2d uv;
    uv.x = 0.5 - altitude / msGeomConst_pi;
    uv.y = 0.25 - azimuth;
    return uv;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d SpatialViewDefinition::_ComputeEyePoint(Frustum const& frustum) const
    {
    DVec3d delta = DVec3d::FromStartEnd(frustum.GetCorner(NPC_LeftBottomRear), frustum.GetCorner(NPC_LeftBottomFront));

    double pseudoCameraHalfAngle = 22.5;         // Somewhat arbitrily chosen to match Luxology.
    double diagonal = frustum.GetCorner(NPC_LeftBottomRear).Distance(frustum.GetCorner(NPC_RightTopRear));
    double focalLength = diagonal / (2.0 * atan(pseudoCameraHalfAngle * msGeomConst_radiansPerDegree));
    
    return DPoint3d::FromSumOf(frustum.GetCorner(NPC_LeftBottomRear), .5, frustum.GetCorner(NPC_RightTopRear), .5, delta, focalLength / delta.Magnitude());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawBackgroundMesh(Render::GraphicBuilderP builder, DgnViewportCR viewport, double rotation, double zOffset)
    {
    enum {MESH_DIMENSION=10};

    double delta = 1.0 / (double) (MESH_DIMENSION-1);

    bvector<DPoint3d> meshPoints;
    bvector<DPoint2d> meshParams; 
    bvector<int> indices;
    DPoint3d cameraPos = viewport.GetSpatialViewControllerCP()->GetSpatialViewDefinition().ComputeEyePoint(viewport.GetFrustum());

    for (int row = 1; row < MESH_DIMENSION;  ++row)
        {
        for (int col = 1; col < MESH_DIMENSION; ++col)
            {
            DPoint2d low  = {(double) (row-1) * delta, (double) (col-1) * delta};
            DPoint2d high = {(double) row * delta, (double) col * delta};

            DPoint2d params[4];
            DPoint3d points[4];

            points[0].Init(low.x,  low.y);
            points[1].Init(low.x,  high.y);
            points[2].Init(high.x, high.y);
            points[3].Init(high.x, low.y);

            viewport.NpcToWorld(points, points, 4);
            for (int i=0; i<4; ++i)
                {
                DVec3d direction = DVec3d::FromStartEnd(cameraPos, points[i]);
                params[i] = getUVForDirection(direction, rotation, zOffset);

                // We need to move the point off the back plane slightly so it won't be clipped. Move it 1/10000 of the distance to the eye.
                // That should keep it behind any geomtery of interest, but at least one value in zbuffer resolution inside the frustum.
                double len = direction.Normalize();
                points[i].SumOf(points[i], direction, -len/10000.);
                }

            // Avoid seam discontinuities by eliminating cycles.
            DRange2d paramRange;
            paramRange.InitFrom(params, 4);
            if ((paramRange.high.x - paramRange.low.x) > .5)
                {
                for (int i=0; i<4; i++)
                    while (params[i].x < .5) params[i].x +=  1.0;      
                }
    
            if ((paramRange.high.y - paramRange.low.y) > .5)
                {
                for (int i=0; i<4; i++)
                    while (params[i].y < .5) params[i].y += 1.0;
                }

            for (int i=0; i<4; ++i)
                {
                meshPoints.push_back(points[i]);
                meshParams.push_back(params[i]);
                indices.push_back((int)meshPoints.size());
                }
            }
        }

    PolyfaceQueryCarrier polyface(4, false, indices.size(),                                 // NumPerFace, twosided, nIndices,
                                            meshPoints.size(), &meshPoints[0], &indices[0], // Points.
                                            0, nullptr, nullptr,                            // Normals.
                                            meshParams.size(), &meshParams[0], &indices[0], // Params.
                                            0,  nullptr, nullptr);                          // Colors

    builder->AddPolyface(polyface, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewController::DrawSkyBox(TerrainContextR context)
    {
    auto& style3d = GetSpatialViewDefinition().GetDisplayStyle3d();
    if (!style3d.IsSkyBoxEnabled())
        return;

    auto vp=context.GetViewport();
    if (!m_skybox.IsValid())
        LoadSkyBox(vp->GetRenderTarget()->GetSystem());

    BeAssert(m_skybox.IsValid());

    // create a graphic for the skybox, and assign the sky material to it.
    Render::GraphicBuilderPtr skyGraphic = context.CreateGraphic();
    GraphicParams params;
    params.SetMaterial(m_skybox.get());
    skyGraphic->ActivateGraphicParams(params);

    // now create a 10x10 mesh on the backplane with the sky material mapped to its UV coordinates
    drawBackgroundMesh(skyGraphic.get(), *vp, 0.0, context.GetDgnDb().GeoLocation().GetGlobalOrigin().z);

    // we want to control the rendermode, lighting, and edges for the mesh. To do that we have to create a GraphicBranch with the appropriate ViewFlags
    ViewFlags flags = context.GetViewFlags();
    flags.SetRenderMode(Render::RenderMode::SmoothShade);
    flags.SetShowTextures(true);
    flags.SetShowVisibleEdges(false);
    flags.SetShowMaterials(true);
    flags.SetShowShadows(false);
    flags.SetIgnoreLighting(true);

    GraphicBranch branch;
    branch.Add(*skyGraphic); // put the mesh into the branch
    branch.SetViewFlags(flags); // and set its Viewflags

    // now add the skybox branch to the terrain context.
    context.OutputGraphic(*context.CreateBranch(branch), nullptr);
    }

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/16
//=======================================================================================
namespace EnvironmentJson
{
    static Utf8CP str_Environment() {return "environment";}
    static Utf8CP str_Display()     {return "display";}
    static Utf8CP str_Ground()      {return "ground";}
    static Utf8CP str_Sky()         {return "sky";}

    namespace GroundPlaneJson
    {
        static Utf8CP str_Elevation()   {return "elevation";}
        static Utf8CP str_AboveColor()  {return "aboveColor";}
        static Utf8CP str_BelowColor()  {return "belowColor";}
    };
    namespace SkyBoxJson
    {
        static Utf8CP str_Filename()    {return "file";}
        static Utf8CP str_SkyColor()    {return "skyColor";}
        static Utf8CP str_ZenithColor() {return "zenithColor";}
        static Utf8CP str_NadirColor()  {return "nadirColor";}
        static Utf8CP str_GroundColor() {return "groundColor";}
        static Utf8CP str_SkyExponent() {return "skyExponent";}
        static Utf8CP str_GroundExponent() {return "groundExponent";}
    };

    static ColorDef GetColor(JsonValueCR in, Utf8CP name, ColorDef defaultVal) {JsonValueCR json = in[name]; return json.isInt() ? ColorDef(json.asInt()) : defaultVal;}
    static double GetDouble(JsonValueCR in, Utf8CP name, double defaultVal) {JsonValueCR json = in[name]; return json.isDouble() ? json.asDouble() : defaultVal;}
};

using namespace EnvironmentJson;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle3d::_CopyFrom(DgnElementCR el) 
    {
    T_Super::_CopyFrom(el);
    auto other = static_cast<DisplayStyle3d const*>(&el);
    m_environment = other->m_environment;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle3d::_OnLoadedJsonProperties()
    {
    T_Super::_OnLoadedJsonProperties();

    JsonValueCR env = GetStyle(str_Environment());
    
    JsonValueCR ground = env[str_Ground()];
    JsonValueCR elevationJson = ground[GroundPlaneJson::str_Elevation()];

    m_environment.m_groundPlane.m_enabled = ground[str_Display()].asBool();
    m_environment.m_groundPlane.m_elevation = elevationJson.isNull() ? -DgnUnits::OneCentimeter() : elevationJson.asDouble();
    m_environment.m_groundPlane.m_aboveColor = GetColor(ground, GroundPlaneJson::str_AboveColor(), ColorDef::DarkGreen());
    m_environment.m_groundPlane.m_belowColor = GetColor(ground, GroundPlaneJson::str_BelowColor(), ColorDef::DarkBrown());

    JsonValueCR sky = env[str_Sky()];

    m_environment.m_skybox.m_enabled = sky[str_Display()].asBool();
    m_environment.m_skybox.m_jpegFile = sky[SkyBoxJson::str_Filename()].asString();
    m_environment.m_skybox.m_groundExponent = GetDouble(sky, SkyBoxJson::str_GroundExponent(), 4.0);
    m_environment.m_skybox.m_skyExponent = GetDouble(sky, SkyBoxJson::str_SkyExponent(), 4.0);
    m_environment.m_skybox.m_groundColor = GetColor(sky, SkyBoxJson::str_GroundColor(), ColorDef(120,143,125));
    m_environment.m_skybox.m_zenithColor = GetColor(sky, SkyBoxJson::str_ZenithColor(), ColorDef(54,117,255));
    m_environment.m_skybox.m_nadirColor  = GetColor(sky, SkyBoxJson::str_NadirColor(), ColorDef(40,15,0));
    m_environment.m_skybox.m_skyColor    = GetColor(sky, SkyBoxJson::str_SkyColor(), ColorDef(143,205,255));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle3d::_OnSaveJsonProperties() 
    {
    T_Super::_OnSaveJsonProperties();

    Json::Value env;
    Json::Value ground;
    ground[str_Display()] = m_environment.m_groundPlane.m_enabled;
    ground[GroundPlaneJson::str_Elevation()] = m_environment.m_groundPlane.m_elevation;
    ground[GroundPlaneJson::str_AboveColor()] = m_environment.m_groundPlane.m_aboveColor.GetValue();
    ground[GroundPlaneJson::str_BelowColor()] = m_environment.m_groundPlane.m_belowColor.GetValue();
    env[str_Ground()] = ground;

    Json::Value sky = env[str_Sky()];
    sky[str_Display()] = m_environment.m_skybox.m_enabled;
    sky[SkyBoxJson::str_Filename()] = m_environment.m_skybox.m_jpegFile;
    sky[SkyBoxJson::str_GroundExponent()] = m_environment.m_skybox.m_groundExponent;
    sky[SkyBoxJson::str_SkyExponent()] = m_environment.m_skybox.m_skyExponent;

    sky[SkyBoxJson::str_GroundColor()] = m_environment.m_skybox.m_groundColor.GetValue();
    sky[SkyBoxJson::str_ZenithColor()] = m_environment.m_skybox.m_zenithColor.GetValue();
    sky[SkyBoxJson::str_NadirColor()] = m_environment.m_skybox.m_nadirColor.GetValue();
    sky[SkyBoxJson::str_SkyColor()] = m_environment.m_skybox.m_skyColor.GetValue();
    env[str_Sky()] = sky;
    
    SetStyle(str_Environment(), env);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle3d::EnvironmentDisplay::Initialize()
    {
    m_groundPlane.m_enabled = true;
    m_groundPlane.m_elevation = -DgnUnits::OneCentimeter();
    m_groundPlane.m_aboveColor = ColorDef::DarkGreen();
    m_groundPlane.m_belowColor = ColorDef::DarkBrown();

    m_skybox.m_enabled = true;
    m_skybox.m_groundExponent = 4.0;
    m_skybox.m_skyExponent = 4.0;
    m_skybox.m_groundColor = ColorDef(120,143,125);
    m_skybox.m_zenithColor = ColorDef(54,117,255);
    m_skybox.m_nadirColor  = ColorDef(40,15,0);
    m_skybox.m_skyColor    = ColorDef(143,205,255);
    }
