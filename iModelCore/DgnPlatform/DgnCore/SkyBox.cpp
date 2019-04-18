/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
double SpatialViewController::GetGroundElevation() const
    {
    auto& env = GetSpatialViewDefinition().GetDisplayStyle3d().GetEnvironmentDisplay();
    return env.m_groundPlane.m_elevation + GetDgnDb().GeoLocation().GetGlobalOrigin().z; // adjust for global origin
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
void DisplayStyle3d::_CopyFrom(DgnElementCR el, CopyFromOptions const& opts)
    {
    T_Super::_CopyFrom(el, opts);
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
