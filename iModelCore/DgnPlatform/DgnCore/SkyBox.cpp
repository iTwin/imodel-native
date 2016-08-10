/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/SkyBox.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#if defined (_MSC_VER)
#pragma warning(disable:4505)
#endif

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/16
//=======================================================================================
namespace EnvironmentJson
{
    static Utf8CP Display()     {return "display";}
    static Utf8CP Environment() {return "environment";}
    static Utf8CP GroundPlane() {return "groundPlane";}
    static Utf8CP SkyBox()      {return "skybox";}

    namespace GroundPlaneJson
    {
        static Utf8CP Elevation()   {return "elevation";}
        static Utf8CP AboveColor()  {return "aboveColor";}
        static Utf8CP BelowColor()  {return "belowColor";}
        static Utf8CP Type()        {return "type";}
        static Utf8CP Rectangle()   {return "rectangle";}
    };
    namespace SkyBoxJson
    {
        static Utf8CP Filename()    {return "file";}
        static Utf8CP SkyColor()    {return "skyColor";}
        static Utf8CP ZenithColor() {return "zenithColor";}
        static Utf8CP NadirColor()  {return "nadirColor";}
        static Utf8CP GroundColor() {return "groundColor";}
        static Utf8CP SkyExponent() {return "skyExponent";}
        static Utf8CP GroundExponent() {return "groundExponent";}
    };

    static bool IsDisplayed(JsonValueCR in) {JsonValueCR displayJson = in[Display()]; return displayJson.isNull() || displayJson.asBool();}
    static ColorDef GetColor(JsonValueCR in, Utf8CP name, ColorDef defaultVal) {JsonValueCR json = in[name]; return json.isInt() ? ColorDef(json.asInt()) : defaultVal;}
    static double GetDouble(JsonValueCR in, Utf8CP name, double defaultVal) {JsonValueCR json = in[name]; return json.isDouble() ? json.asDouble() : defaultVal;}
// UNUSED:    static Utf8String GetString(JsonValueCR in, Utf8CP name, Utf8CP defaultVal) {JsonValueCR json = in[name]; return json.isString() ? json.asString() : defaultVal;}
// UNUSED:    static MaterialPtr CreateGradient(JsonValueCR in);
    static Byte lerp(double t, Byte a, Byte b) {return a + t * double(b - a);}
    static DPoint2d GetUVForDirection(DPoint3dCR direction, CameraViewController::Environment::Projection type, double rotation, double zOffset);
    static void DrawBackgroundMesh(Render::GraphicBuilderP builder, DgnViewportCR, CameraViewController::Environment::Projection type, double rotation, double zOffset);
    static DPoint3d ComputeCamera(DgnViewportCR vp);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
CameraViewController::Environment::GroundPlane CameraViewController::GetGroundPlane(DgnViewportCR vp) const
    {
    using namespace EnvironmentJson;

    Environment::GroundPlane ground;
    JsonValueCP setting = GetEnvironmentSetting(GroundPlane());
    Json::Value nullval;
    if (nullptr == setting)
        {
#if defined (NEEDS_WORK_GROUND_PLANE)
        return ground;
#endif
        setting = &nullval;
        }

    JsonValueCR json = *setting;

    // determine the height of the ground plane. By default, draw it one centimeter below 0.0
    JsonValueCR elevationJson = json[GroundPlaneJson::Elevation()];
    double elevation = elevationJson.isNull() ? -DgnUnits::OneCentimeter() : elevationJson.asDouble();

    DgnUnits& units = m_dgndb.Units();
    elevation += units.GetGlobalOrigin().z; // adjust for global origin

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
            return ground; // view does not show ground plane
        }

    ground.m_extents = units.GetProjectExtents();
    ground.m_extents.low.z = ground.m_extents.high.z = elevation;

    DPoint3d center = DPoint3d::FromInterpolate(ground.m_extents.low, 0.5, ground.m_extents.high);

//    static bool s_rectangle = false;
//    if (s_rectangle)
    if (json[GroundPlaneJson::Type()].asString() == GroundPlaneJson::Rectangle())
        {
        ground.m_extents.ScaleAboutCenter(ground.m_extents, 2.0);
        ground.m_edgeAlpha = 0xf0;
        ground.m_mode =  GradientSymb::Mode::Hemispherical;
        }
    else
        {
        double radius = ground.m_extents.low.Distance(ground.m_extents.high); // we need a square, not a rectangle
        ground.m_extents.InitFrom(center);
        ground.m_extents.Extend(radius);
        ground.m_extents.low.z = ground.m_extents.high.z = elevation;
        }

    bool above = IsCameraAbove(elevation);
    if (!above)
        ground.m_centerAlpha = 0x85;

    ground.m_color = GetColor(json, above ? GroundPlaneJson::AboveColor() : GroundPlaneJson::BelowColor(), above ? ColorDef::DarkGreen() : ColorDef::DarkBrown());
    return ground;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraViewController::Environment::GroundPlane::Draw(DecorateContextR context)
    {
    DPoint3d pts[5];
    pts[0] = pts[1] = pts[4] = m_extents.low;
    pts[1].y = m_extents.high.y;
    pts[2] = pts[3] = m_extents.high;
    pts[3].y = m_extents.low.y;

    double keyValues[] = {0.0, 0.25, 0.5}; // gradient goes from edge of rectangle (0.0) to center (1.0)...
    ColorDef colors[] = {m_color, m_color, m_color};

    colors[0].SetAlpha(m_edgeAlpha);
    colors[1].SetAlpha(m_centerAlpha);
    colors[2].SetAlpha(m_centerAlpha);

    GradientSymbPtr gradient = GradientSymb::Create();
    gradient->SetMode(m_mode);
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
Render::TexturePtr CameraViewController::Environment::SkyBox::LoadSkyBox(Utf8CP fileName, Render::SystemCR system)
    {
    BeFile skyFile;
    if (BeFileStatus::Success != skyFile.Open(fileName, BeFileAccess::Read))
        return nullptr;

    ByteStream jpegData;
    if (BeFileStatus::Success != skyFile.ReadEntireFile(jpegData))
        return nullptr;

    ImageSource jpeg(ImageSource::Format::Jpeg, std::move(jpegData));
    return system._CreateTexture(jpeg, Image::Format::Rgba, Image::BottomUp::No);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
CameraViewController::Environment::SkyBox::SkyBox(JsonValueCR json, Render::SystemCR system)
    {
    using namespace EnvironmentJson;

    Render::TexturePtr texture;

    JsonValueCR fileJson = json[SkyBoxJson::Filename()];
    if (!fileJson.isNull())
        texture = LoadSkyBox(fileJson.asString().c_str(), system);

    // we didn't get a jpeg sky, just create a gradient
    if (!texture.IsValid())
        {
        // get the parameters for the environment gradient from the settings Json
        double groundExponent = GetDouble(json, SkyBoxJson::GroundExponent(), 4.0);
        double skyExponent = GetDouble(json, SkyBoxJson::SkyExponent(), 4.0);
        ColorDef groundColor = GetColor(json, SkyBoxJson::GroundColor(), ColorDef(120,143,125));
        ColorDef zenithColor = GetColor(json, SkyBoxJson::ZenithColor(), ColorDef(54,117,255));
        ColorDef nadirColor  = GetColor(json, SkyBoxJson::NadirColor(), ColorDef(40,15,0));
        ColorDef skyColor    = GetColor(json, SkyBoxJson::SkyColor(), ColorDef(143,205,255));

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
                color1 = nadirColor;
                color2 = groundColor;
                frac = 1.0 -(2.0 * (frac - 0.5));
                frac = pow(frac, groundExponent);
                }
            else
                {
                color1 = zenithColor;
                color2 = skyColor;
                frac = 2.0*frac;
                frac = pow(frac, skyExponent);
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
    m_material = system._CreateMaterial(matParams);

    Material::TextureMapParams mapParams;
    Material::Trans2x3 transform(0.0, 1.0, 0.0, 1.0, 0.0, 0.0);
    mapParams.SetTransform(&transform);
    m_material->_MapTexture(*texture, mapParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d EnvironmentJson::GetUVForDirection(DPoint3dCR direction, CameraViewController::Environment::Projection type, double rotation, double zOffset)
    {
    double radius = sqrt(direction.x*direction.x + direction.y*direction.y);
    double zValue = direction.z - radius * zOffset;
    double azimuth  = (atan2(direction.y, direction.x) + rotation) / msGeomConst_2pi;
    double altitude = atan2(zValue, radius);

    DPoint2d uv;
    switch (type)
        {
        default:
            uv.x = 0.5 - altitude / msGeomConst_pi;
            uv.y = 0.25 - azimuth;
            break;

        case CameraViewController::Environment::Projection::Cylindrical:
            double piOver6 = msGeomConst_pi / 6.0;
            double OneOverSinPiOver6 = 1.0 / sin(piOver6);

            uv.x = .25 - azimuth;

            if (fabs(altitude) > piOver6)
                uv.y = .5 - sin(altitude) * OneOverSinPiOver6 / 2.0;   
            else
                uv.y = .5 + sin(altitude) * OneOverSinPiOver6 / 2.0;         

            break;
        }

    return uv;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d EnvironmentJson::ComputeCamera(DgnViewportCR vp)
    {
    Frustum frustum = vp.GetFrustum();
    DVec3d delta = DVec3d::FromStartEnd(frustum.GetCorner(NPC_LeftBottomRear), frustum.GetCorner(NPC_LeftBottomFront));

    if (vp.IsCameraOn())
        return vp.GetCamera().GetEyePoint();

    double pseudoCameraHalfAngle = 22.5;         // Somewhat arbitrily chosen to match Luxology.
    double diagonal = frustum.GetCorner(NPC_LeftBottomRear).Distance(frustum.GetCorner(NPC_RightTopRear));
    double focalLength = diagonal / (2.0 * atan(pseudoCameraHalfAngle * msGeomConst_radiansPerDegree));
    
    return DPoint3d::FromSumOf(frustum.GetCorner(NPC_LeftBottomRear), .5, frustum.GetCorner(NPC_RightTopRear), .5, delta, focalLength / delta.Magnitude());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void EnvironmentJson::DrawBackgroundMesh(Render::GraphicBuilderP builder, DgnViewportCR viewport, CameraViewController::Environment::Projection type, double rotation, double zOffset)
    {
    enum {MESH_DIMENSION=10};

    double delta = 1.0 / (double) (MESH_DIMENSION-1);

    bvector<DPoint3d> meshPoints;
    bvector<DPoint2d> meshParams; 
    bvector<int> indices;
    DPoint3d cameraPos = ComputeCamera(viewport);

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
                params[i] = GetUVForDirection(direction, type, rotation, zOffset);

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
RefCountedPtr<CameraViewController::Environment::SkyBox> CameraViewController::GetSkyBox(DgnViewportCR vp) const
    {
    using namespace EnvironmentJson;

    JsonValueCP setting = GetEnvironmentSetting(SkyBox());
    Json::Value nullval;
    if (nullptr == setting)
        {
#if defined (NEEDS_WORK_SKYBOX)
        return nullptr;
#endif
        setting = &nullval;
        }

    static AppData::Key s_key;
    Environment::SkyBox* sky = (Environment::SkyBox*) FindAppData(s_key);
    if (nullptr == sky)
        {
        sky = new Environment::SkyBox(*setting, vp.GetRenderTarget()->GetSystem());
        AddAppData(s_key, sky);
        }

    return sky;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraViewController::Environment::SkyBox::Draw(TerrainContextR context)
    {
    if (!m_material.IsValid())
        return; // something bad happened when we set up the material. Give up.

    // create a graphic for the skybox, and assign the sky material to it.
    Render::GraphicBuilderPtr skyGraphic = context.CreateGraphic();
    GraphicParams params;
    params.SetMaterial(m_material.get());
    skyGraphic->ActivateGraphicParams(params);

    // now create a 10x10 mesh on the backplane with the sky material mapped to its UV coordinates
    EnvironmentJson::DrawBackgroundMesh(skyGraphic.get(), *context.GetViewport(), Environment::Projection::Spherical, 0.0, context.GetDgnDb().Units().GetGlobalOrigin().z);

    // we want to control the rendermode, lighting, and edges for the mesh. To do that we have to create a GraphicBranch with the appropriate ViewFlags
    ViewFlags flags = context.GetViewFlags();
    flags.SetRenderMode(Render::RenderMode::SmoothShade);
    flags.m_textures = true;
    flags.m_visibleEdges = false;
    flags.m_materials = true;
    flags.m_shadows = false;
    flags.m_ignoreLighting = true;

    GraphicBranch branch;
    branch.Add(*skyGraphic); // put the mesh into the branch
    branch.SetViewFlags(flags); // and set its Viewflags

    // now add the skybox branch to the terrain context.
    context.OutputGraphic(*context.CreateBranch(Graphic::CreateParams(), branch), nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
JsonValueCP CameraViewController::GetEnvironmentSetting(Utf8CP name) const
    {
    JsonValueCR environment = m_settings[EnvironmentJson::Environment()];
    if (environment.isNull() || !EnvironmentJson::IsDisplayed(environment))
        return nullptr;

    JsonValueCR val = environment[name];
    return (!val.isNull() && EnvironmentJson::IsDisplayed(val)) ? &val : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* the ground plane is drawn as a decorator so its transparency blends with the scene properly
* @bsimethod                                    Keith.Bentley                   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraViewController::DrawEnvironment(DecorateContextR context)
    {
    auto ground = GetGroundPlane(*context.GetViewport());
    if (ground.IsValid())
        ground.Draw(context);
    }

/*---------------------------------------------------------------------------------**//**
* the skybox is part of the terrain
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraViewController::_CreateTerrain(TerrainContextR context)
    {
    auto skybox = GetSkyBox(*context.GetViewport());
    if (skybox.IsValid())
        skybox->Draw(context);
    }
