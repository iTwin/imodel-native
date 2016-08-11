/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/SkyBox.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

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

    static ColorDef GetColor(JsonValueCR in, Utf8CP name, ColorDef defaultVal) {JsonValueCR json = in[name]; return json.isInt() ? ColorDef(json.asInt()) : defaultVal;}
    static double GetDouble(JsonValueCR in, Utf8CP name, double defaultVal) {JsonValueCR json = in[name]; return json.isDouble() ? json.asDouble() : defaultVal;}
    static Byte lerp(double t, Byte a, Byte b) {return a + t * double(b - a);}
    static DPoint2d GetUVForDirection(DPoint3dCR direction, double rotation, double zOffset);
    static void DrawBackgroundMesh(Render::GraphicBuilderP builder, DgnViewportCR, double rotation, double zOffset);
    static DPoint3d ComputeCamera(DgnViewportCR vp);
};

using namespace EnvironmentJson;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d CameraViewController::GetGroundExtents(DgnViewportCR vp) const
    {
    AxisAlignedBox3d extents;
#if defined (NEEDS_WORK_GROUND_PLANE)
    if (!IsGroundPlaneEnabled())
        return extents;
#endif

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

    extents = m_dgndb.Units().GetProjectExtents();
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
double CameraViewController::GetGroundElevation() const
    {
    JsonValueCR ground = m_settings[Environment()][GroundPlane()];

    // determine the height of the ground plane. By default, draw it one centimeter below 0.0
    JsonValueCR elevationJson = ground[GroundPlaneJson::Elevation()];
    double elevation = elevationJson.isNull() ? -DgnUnits::OneCentimeter() : elevationJson.asDouble();

    elevation += m_dgndb.Units().GetGlobalOrigin().z; // adjust for global origin
    return elevation;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraViewController::DrawGroundPlane(DecorateContextR context)
    {
    AxisAlignedBox3d extents = GetGroundExtents(*context.GetViewport());
    if (!extents.IsValid())
        return;

    DPoint3d pts[5];
    pts[0] = pts[1] = pts[4] = extents.low;
    pts[1].y = extents.high.y;
    pts[2] = pts[3] = extents.high;
    pts[3].y = extents.low.y;

    bool above = IsCameraAbove(extents.low.z);
    double keyValues[] = {0.0, 0.25, 0.5}; // gradient goes from edge of rectangle (0.0) to center (1.0)...
    ColorDef color = GetColor(m_settings[Environment()][GroundPlane()], above ? GroundPlaneJson::AboveColor() : GroundPlaneJson::BelowColor(), above ? ColorDef::DarkGreen() : ColorDef::DarkBrown());
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
Render::TexturePtr CameraViewController::LoadTexture(Utf8CP fileName, Render::SystemCR system)
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
void CameraViewController::LoadSkyBox(Render::SystemCR system)
    {
    Render::TexturePtr texture;

    JsonValueCR json =  m_settings[Environment()][SkyBox()];
    JsonValueCR fileJson = json[SkyBoxJson::Filename()];
    if (!fileJson.isNull())
        texture = LoadTexture(fileJson.asString().c_str(), system);

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
    m_skybox = system._CreateMaterial(matParams);

    Material::TextureMapParams mapParams;
    Material::Trans2x3 transform(0.0, 1.0, 0.0, 1.0, 0.0, 0.0);
    mapParams.SetTransform(&transform);
    m_skybox->_MapTexture(*texture, mapParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d EnvironmentJson::GetUVForDirection(DPoint3dCR direction, double rotation, double zOffset)
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
void EnvironmentJson::DrawBackgroundMesh(Render::GraphicBuilderP builder, DgnViewportCR viewport, double rotation, double zOffset)
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
                params[i] = GetUVForDirection(direction, rotation, zOffset);

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
void CameraViewController::DrawSkyBox(TerrainContextR context)
    {
#if defined (NEEDS_WORK_GROUND_PLANE)
    if (!IsSkyBoxEnabled())
        return;
#endif

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
    EnvironmentJson::DrawBackgroundMesh(skyGraphic.get(), *vp, 0.0, context.GetDgnDb().Units().GetGlobalOrigin().z);

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
    context.OutputGraphic(*context.CreateBranch(branch), nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool CameraViewController::IsEnvironmentEnabled() const {return m_settings[Environment()][Display()].asBool();}
bool CameraViewController::IsSkyBoxEnabled() const {return IsEnvironmentEnabled() && m_settings[Environment()][SkyBox()][Display()].asBool();}
bool CameraViewController::IsGroundPlaneEnabled() const {return IsEnvironmentEnabled() && m_settings[Environment()][GroundPlane()][Display()].asBool();}

