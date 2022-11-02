/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
// @bsiclass
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
        BE_JSON_NAME(textures);
        BE_JSON_NAME(front);
        BE_JSON_NAME(back);
        BE_JSON_NAME(top);
        BE_JSON_NAME(bottom);
        BE_JSON_NAME(right);
        BE_JSON_NAME(left);

        static Json::StaticString textureNamesByCubeFace[6] =
            {
            json_front(),
            json_back(),
            json_top(),
            json_bottom(),
            json_right(),
            json_left(),
            };
    };

    static ColorDef GetColor(BeJsConst in, Utf8CP name, ColorDef defaultVal) {return ColorDef(in[name].asInt(defaultVal.GetValue()));}
};

using namespace EnvironmentJson;
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle3d::_CopyFrom(DgnElementCR el, CopyFromOptions const& opts)
    {
    T_Super::_CopyFrom(el, opts);
    auto other = static_cast<DisplayStyle3d const*>(&el);
    m_environment = other->m_environment;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle3d::_OnLoadedJsonProperties()
    {
    T_Super::_OnLoadedJsonProperties();

    auto env = GetStyle(json_environment());

    auto ground = env[json_ground()];
    auto elevationJson = ground[GroundPlaneJson::json_elevation()];

    m_environment.m_groundPlane.m_enabled = ground[json_display()].asBool();
    m_environment.m_groundPlane.m_elevation = elevationJson.asDouble(-DgnUnits::OneCentimeter());
    m_environment.m_groundPlane.m_aboveColor = GetColor(ground, GroundPlaneJson::json_aboveColor(), ColorDef::DarkGreen());
    m_environment.m_groundPlane.m_belowColor = GetColor(ground, GroundPlaneJson::json_belowColor(), ColorDef::DarkBrown());

    auto sky = env[json_sky()];

    m_environment.m_skybox.m_enabled = sky[json_display()].asBool();
    m_environment.m_skybox.m_twoColor= sky[SkyBoxJson::json_twoColor()].asBool();
    m_environment.m_skybox.m_groundExponent = sky[SkyBoxJson::json_groundExponent()].asDouble(4.0);
    m_environment.m_skybox.m_skyExponent = sky[SkyBoxJson::json_skyExponent()].asDouble(4.0);
    m_environment.m_skybox.m_groundColor = GetColor(sky, SkyBoxJson::json_groundColor(), ColorDef(120,143,125));
    m_environment.m_skybox.m_zenithColor = GetColor(sky, SkyBoxJson::json_zenithColor(), ColorDef(54,117,255));
    m_environment.m_skybox.m_nadirColor = GetColor(sky, SkyBoxJson::json_nadirColor(), ColorDef(40,15,0));
    m_environment.m_skybox.m_skyColor = GetColor(sky, SkyBoxJson::json_skyColor(), ColorDef(143,205,255));

    m_environment.m_skybox.m_image.FromJson(sky[SkyBoxJson::json_image()]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle3d::EnvironmentDisplay::SkyBox::Image::FromJson(BeJsConst json)
    {
    Reset();
    if (json.isNull())
        return;

    m_type = static_cast<Type>(json[SkyBoxImageJson::json_type()].asUInt(static_cast<uint32_t>(Type::None)));
    switch (m_type)
        {
        case Type::Cylindrical:
        case Type::Spherical:
            m_specs[0] = json[SkyBoxImageJson::json_texture()].asCString();
            break;
        case Type::Cube:
            {
            auto const& specs = json[SkyBoxImageJson::json_textures()];
            for (auto i = 0; i < 6; i++)
                m_specs[i] = specs[SkyBoxImageJson::textureNamesByCubeFace[i]].asCString();

            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle3d::_OnSaveJsonProperties()
    {
    T_Super::_OnSaveJsonProperties();

    BeJsDocument env;
    auto ground = env[json_ground()];
    ground[json_display()] = m_environment.m_groundPlane.m_enabled;
    ground[GroundPlaneJson::json_elevation()] = m_environment.m_groundPlane.m_elevation;
    ground[GroundPlaneJson::json_aboveColor()] = m_environment.m_groundPlane.m_aboveColor.GetValue();
    ground[GroundPlaneJson::json_belowColor()] = m_environment.m_groundPlane.m_belowColor.GetValue();

    auto sky = env[json_sky()];
    sky[json_display()] = m_environment.m_skybox.m_enabled;

    sky[SkyBoxJson::json_twoColor()] = m_environment.m_skybox.m_twoColor;
    sky[SkyBoxJson::json_groundExponent()] = m_environment.m_skybox.m_groundExponent;
    sky[SkyBoxJson::json_skyExponent()] = m_environment.m_skybox.m_skyExponent;

    sky[SkyBoxJson::json_groundColor()] = m_environment.m_skybox.m_groundColor.GetValue();
    sky[SkyBoxJson::json_zenithColor()] = m_environment.m_skybox.m_zenithColor.GetValue();
    sky[SkyBoxJson::json_nadirColor()] = m_environment.m_skybox.m_nadirColor.GetValue();
    sky[SkyBoxJson::json_skyColor()] = m_environment.m_skybox.m_skyColor.GetValue();

    m_environment.m_skybox.m_image.ToJson(sky[SkyBoxJson::json_image()]);

    SetStyle(json_environment(), env);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle3d::EnvironmentDisplay::SkyBox::Image::ToJson(BeJsValue json) const
    {
    json[SkyBoxImageJson::json_type()] = static_cast<uint32_t>(GetType());
    switch (GetType())
        {
        case Type::None:
            break;
        case Type::Cylindrical:
        case Type::Spherical:
            json[SkyBoxImageJson::json_texture()] = GetSingleSpec();
            break;
        case Type::Cube:
            {
            auto specs = json[SkyBoxImageJson::json_textures()];
            for (auto i = 0; i < 6; i++)
                specs[SkyBoxImageJson::textureNamesByCubeFace[i]] = GetCubeSpecs()[i];

            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
