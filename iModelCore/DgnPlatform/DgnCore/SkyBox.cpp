/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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

    Render::GraphicBuilderPtr graphic = context.CreateWorldDecoration();
    graphic->ActivateGraphicParams(params);
    graphic->AddShape(5, pts, true);
    context.AddWorldDecoration(*graphic->Finish());
    }

static Byte lerp(double t, Byte a, Byte b) {return a + t * double(b - a);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16                                                                  
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle3d::LoadSkyBoxMaterial(Render::SystemCR system)
    {
    if (m_skyboxMaterial.IsValid())
        return;

    Render::TexturePtr texture;

    auto& env = GetEnvironmentDisplay();
    if (env.m_skybox.m_image.m_type != EnvironmentDisplay::SkyBox::Image::Type::None)
        texture = system._GetTexture(env.m_skybox.m_image.m_textureId, GetDgnDb());

    // we didn't get a jpeg sky, just create a gradient
    if (!texture.IsValid())
        {
        enum {GRADIENT_PIXEL_COUNT=1024};
        ByteStream buffer(GRADIENT_PIXEL_COUNT * sizeof(ColorDef));
        ColorDef* thisColor = (ColorDef*) buffer.GetDataP();
        ColorDef color1, color2;

        // set up the 4 color gradient
        for (int i = 0; i < GRADIENT_PIXEL_COUNT; ++i, ++thisColor)
            {
            double frac = (double) i / (double) GRADIENT_PIXEL_COUNT;

            if (env.m_skybox.m_twoColor)
                {
                color1 = env.m_skybox.m_zenithColor;
                color2 = env.m_skybox.m_nadirColor;
                }
            else if (frac > 0.5)
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
        texture = system._CreateTexture(image, GetDgnDb());
        }

    Material::CreateParams matParams;
    matParams.SetDiffuseColor(ColorDef::White());
    matParams.SetShadows(false);
    matParams.SetAmbient(1.0);
    matParams.SetDiffuse(0.0);
    matParams.SetShadows(false);

    TextureMapping::Params mapParams;
    TextureMapping::Trans2x3 transform(0.0, 1.0, 0.0, 1.0, 0.0, 0.0);
    mapParams.SetTransform(&transform);
    matParams.MapTexture(*texture, mapParams);

    m_skyboxMaterial = system._CreateMaterial(matParams, GetDgnDb());
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
DPoint3d ViewDefinition3d::ComputeEyePoint(Frustum const& frustum) const
    {
    if (IsCameraOn())
        return GetEyePoint();

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
#define DIRECT_MESH_CONSTRUCTION_not
#ifdef DIRECT_MESH_CONSTRUCTION
PolyfaceHeaderPtr polyface = PolyfaceHeader::create ();
polyface.Param().setActive (true);
polyface.ParamIndex().setActive (true);
polyface.PointIndex().setActive(true;
#else
    enum {MESH_DIMENSION=10};

    double delta = 1.0 / (double) (MESH_DIMENSION-1);

    bvector<DPoint3d> meshPoints;
    bvector<DPoint2d> meshParams; 
    bvector<int> indices;
    Frustum frustum = viewport.GetFrustum();
    DPoint3d cameraPos = viewport.GetSpatialViewControllerCP()->GetSpatialViewDefinition().ComputeEyePoint(frustum);

    for (int row = 1; row < MESH_DIMENSION;  ++row)
        {
        for (int col = 1; col < MESH_DIMENSION; ++col)
            {
            DPoint2d low  = {(double) (row-1) * delta, (double) (col-1) * delta};
            DPoint2d high = {(double) row * delta, (double) col * delta};

            DPoint2d params[4];
            DPoint3d points[4];
            DPoint3d worldPoints[4];

            double npcZ = 0.5;
            points[0].Init(low.x,  low.y, npcZ);
            points[1].Init(low.x,  high.y, npcZ);
            points[2].Init(high.x, high.y, npcZ);
            points[3].Init(high.x, low.y, npcZ);

            viewport.NpcToWorld(worldPoints, points, 4);
            for (int i=0; i<4; ++i)
                {
                DVec3d direction = DVec3d::FromStartEnd(cameraPos, worldPoints[i]);
                params[i] = getUVForDirection(direction, rotation, zOffset);
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

            viewport.WorldToView(points, worldPoints, 4);
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
                                            0, nullptr, nullptr);                          // Colors
#endif
    builder->AddPolyface(polyface, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewController::DrawSkyBox(DecorateContextR context)
    {
    auto& style3d = GetSpatialViewDefinition().GetDisplayStyle3d();
    if (!style3d.IsSkyBoxEnabled())
        return;

    auto vp=context.GetViewport();
    style3d.LoadSkyBoxMaterial(vp->GetRenderTarget()->GetSystem());

    if (nullptr == style3d.GetSkyBoxMaterial())
        {
        BeAssert(false);
        return;
        }

    // create a graphic for the skybox, and assign the sky material to it.
    Render::GraphicBuilderPtr skyGraphic = context.CreateViewBackground();
    GraphicParams params;
    params.SetMaterial(style3d.GetSkyBoxMaterial());
    skyGraphic->ActivateGraphicParams(params);

    // now create a 10x10 mesh on the backplane with the sky material mapped to its UV coordinates
    drawBackgroundMesh(skyGraphic.get(), *vp, 0.0, context.GetDgnDb().GeoLocation().GetGlobalOrigin().z);

    context.SetViewBackground(*skyGraphic->Finish());
    }

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/16
//=======================================================================================
namespace EnvironmentJson
{
    BE_JSON_NAME(display);
    BE_JSON_NAME(ground);
    BE_JSON_NAME(sky);

    namespace GroundPlaneJson
    {
        BE_JSON_NAME(elevation);
        BE_JSON_NAME(aboveColor);
        BE_JSON_NAME(belowColor);
    };
    namespace SkyBoxJson
    {
        BE_JSON_NAME(twoColor);
        BE_JSON_NAME(skyColor);
        BE_JSON_NAME(zenithColor);
        BE_JSON_NAME(nadirColor);
        BE_JSON_NAME(groundColor);
        BE_JSON_NAME(skyExponent);
        BE_JSON_NAME(groundExponent);
        BE_JSON_NAME(image);
    };
    namespace SkyBoxImageJson
    {
        BE_JSON_NAME(type);
        BE_JSON_NAME(texture);
    };

    static ColorDef GetColor(JsonValueCR in, Utf8CP name, ColorDef defaultVal) {return ColorDef(in[name].asInt(defaultVal.GetValue()));}
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

    JsonValueCR env = GetStyle(json_environment());
    
    JsonValueCR ground = env[json_ground()];
    JsonValueCR elevationJson = ground[GroundPlaneJson::json_elevation()];

    m_environment.m_groundPlane.m_enabled = ground[json_display()].asBool();
    m_environment.m_groundPlane.m_elevation = elevationJson.asDouble(-DgnUnits::OneCentimeter());
    m_environment.m_groundPlane.m_aboveColor = GetColor(ground, GroundPlaneJson::json_aboveColor(), ColorDef::DarkGreen());
    m_environment.m_groundPlane.m_belowColor = GetColor(ground, GroundPlaneJson::json_belowColor(), ColorDef::DarkBrown());

    JsonValueCR sky = env[json_sky()];

    m_environment.m_skybox.m_enabled = sky[json_display()].asBool();
    m_environment.m_skybox.m_twoColor= sky[SkyBoxJson::json_twoColor()].asBool();
    m_environment.m_skybox.m_groundExponent = sky[SkyBoxJson::json_groundExponent()].asDouble(4.0);
    m_environment.m_skybox.m_skyExponent = sky[SkyBoxJson::json_skyExponent()].asDouble(4.0);
    m_environment.m_skybox.m_groundColor = GetColor(sky, SkyBoxJson::json_groundColor(), ColorDef(120,143,125));
    m_environment.m_skybox.m_zenithColor = GetColor(sky, SkyBoxJson::json_zenithColor(), ColorDef(54,117,255));
    m_environment.m_skybox.m_nadirColor = GetColor(sky, SkyBoxJson::json_nadirColor(), ColorDef(40,15,0));
    m_environment.m_skybox.m_skyColor = GetColor(sky, SkyBoxJson::json_skyColor(), ColorDef(143,205,255));

    m_environment.m_skybox.m_image.m_type = EnvironmentDisplay::SkyBox::Image::Type::None;
    m_environment.m_skybox.m_image.m_textureId.Invalidate();
    JsonValueCR image = sky[SkyBoxJson::json_image()];
    if (!image.isNull())
        {
        auto textureId = BeInt64Id::FromString(image[SkyBoxImageJson::json_texture()].asCString());
        m_environment.m_skybox.m_image.m_textureId = DgnTextureId(textureId.GetValueUnchecked());
        m_environment.m_skybox.m_image.m_type = static_cast<EnvironmentDisplay::SkyBox::Image::Type>(image[SkyBoxImageJson::json_type()].asUInt(static_cast<uint32_t>(EnvironmentDisplay::SkyBox::Image::Type::None)));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle3d::_OnSaveJsonProperties() 
    {
    T_Super::_OnSaveJsonProperties();

    Json::Value env, ground;
    ground[json_display()] = m_environment.m_groundPlane.m_enabled;
    ground[GroundPlaneJson::json_elevation()] = m_environment.m_groundPlane.m_elevation;
    ground[GroundPlaneJson::json_aboveColor()] = m_environment.m_groundPlane.m_aboveColor.GetValue();
    ground[GroundPlaneJson::json_belowColor()] = m_environment.m_groundPlane.m_belowColor.GetValue();
    env[json_ground()] = ground;

    Json::Value sky = env[json_sky()];
    sky[json_display()] = m_environment.m_skybox.m_enabled;
    sky[SkyBoxJson::json_groundExponent()] = m_environment.m_skybox.m_groundExponent;
    sky[SkyBoxJson::json_skyExponent()] = m_environment.m_skybox.m_skyExponent;
    sky[SkyBoxJson::json_image()][SkyBoxImageJson::json_type()] = static_cast<uint32_t>(m_environment.m_skybox.m_image.m_type);
    sky[SkyBoxJson::json_image()][SkyBoxImageJson::json_texture()] = m_environment.m_skybox.m_image.m_textureId.ToHexStr();

    sky[SkyBoxJson::json_groundColor()] = m_environment.m_skybox.m_groundColor.GetValue();
    sky[SkyBoxJson::json_zenithColor()] = m_environment.m_skybox.m_zenithColor.GetValue();
    sky[SkyBoxJson::json_nadirColor()] = m_environment.m_skybox.m_nadirColor.GetValue();
    sky[SkyBoxJson::json_skyColor()] = m_environment.m_skybox.m_skyColor.GetValue();
    env[json_sky()] = sky;
    
    SetStyle(json_environment(), env);
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
