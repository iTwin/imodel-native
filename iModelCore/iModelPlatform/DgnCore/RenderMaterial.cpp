/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <cmath>

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RgbFactor RenderingAsset::GetColor(Utf8CP name) const
    {
    RgbFactor rgb = {0.0, 0.0, 0.0};

    auto value = GetValue(name);
    if (value.size() < 3)
        {
        BeAssert(false);
        return rgb;
        }

    rgb.red   = value[0].asDouble();
    rgb.green = value[1].asDouble();
    rgb.blue  = value[2].asDouble();
    return rgb;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static DPoint2d getDPoint2dValue(BeJsConst rootValue, Utf8CP key)
    {
    auto value = rootValue[key];
    DPoint2d point = { 0.0, 0.0 };

    if (value.size() < 2)
        {
        BeAssert(false);
        return point;
        }

    point.x = value[0].asDouble();
    point.y = value[1].asDouble();

    return point;
    }

DPoint2d RenderingAsset::TextureMap::GetScale() const {return getDPoint2dValue(m_value, RENDER_MATERIAL_PatternScale);}
DPoint2d RenderingAsset::TextureMap::GetOffset() const {return getDPoint2dValue(m_value, RENDER_MATERIAL_PatternOffset);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double RenderingAsset::TextureMap::GetUnitScale(Units units)
    {
    switch (units)
        {
        case Units::Meters:
            return 1.0;

        case Units::Millimeters:
            return UnitDefinition::GetStandardUnit(StandardUnit::MetricMillimeters).ToMeters();

        case Units::Feet:
            return UnitDefinition::GetStandardUnit(StandardUnit::EnglishFeet).ToMeters();

        case Units::Inches:
            return UnitDefinition::GetStandardUnit(StandardUnit::EnglishInches).ToMeters();

        default:
            BeAssert(false);
            return 1.0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Render::TextureMapping::Mode RenderingAsset::TextureMap::GetMode() const
    {
    return (Render::TextureMapping::Mode) m_value[RENDER_MATERIAL_PatternMapping].asInt((int)Render::TextureMapping::Mode::Parametric);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double RenderingAsset::TextureMap::GetPatternWeight() const
    {
    return m_value[RENDER_MATERIAL_PatternWeight].asDouble(1.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Render::TextureMapping::Params RenderingAsset::TextureMap::GetTextureMapParams() const
    {
    BeAssert(IsValid());

    Render::TextureMapping::Params mapParams;
    if (!IsValid())
        return mapParams;

    Render::TextureMapping::Trans2x3 trans = GetTransform();
    return Render::TextureMapping::Params(GetMode(), trans, GetPatternWeight(), Units::Relative != GetUnits());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RenderingAsset::TextureMap::Units RenderingAsset::TextureMap::GetUnits() const
    {
    return (Units) m_value[RENDER_MATERIAL_PatternScaleMode].asInt((int)Units::Relative);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Render::TextureMapping::Trans2x3 RenderingAsset::TextureMap::GetTransform() const
    {
    Trans2x3Builder builder(*this);
    return builder.Build();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RenderingAsset::Trans2x3Builder::Trans2x3Builder(RenderingAsset::TextureMap const& map)
    {
    m_angle = Angle::FromDegrees(map.GetDouble(RENDER_MATERIAL_PatternAngle, 0.0));
    m_scale = map.GetScale();
    m_offset = map.GetOffset();
    m_mode = map.GetMode();
    m_units = map.GetUnits();

    if (map.GetBool(RENDER_MATERIAL_PatternFlipU, false))
        FlipU();

    if (map.GetBool(RENDER_MATERIAL_PatternFlipV, false))
        FlipV();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RenderingAsset::Trans2x3Builder::Trans2x3Builder(Mode mode, Units units)
    {
    m_scale = DPoint2d::From(1.0, 1.0);
    m_offset = DPoint2d::From(0.0, 0.0);
    m_angle = Angle::FromRadians(0.0);
    m_mode = mode;
    m_units = units;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Render::TextureMapping::Trans2x3 RenderingAsset::Trans2x3Builder::Build() const
    {
    auto trans = Render::TextureMapping::Trans2x3::FromIdentity();

    DPoint2d scale = m_scale;
    if (Units::Relative != m_units)
        scale.Scale(RenderingAsset::TextureMap::GetUnitScale(m_units));

    static constexpr double s_minScale = 1.0E-10;
    if (fabs(scale.x) < s_minScale)
        scale.x = scale.x < 0.0 ? -s_minScale : s_minScale;

    if (fabs(scale.y) < s_minScale)
        scale.y = scale.y < 0.0 ? -s_minScale : s_minScale;

    double angleRadians = m_angle.Radians();
    double cosAngle = cos(angleRadians);
    double sinAngle = sin(angleRadians);

    trans.m_val[0][0] = cosAngle / scale.x;
    trans.m_val[0][1] = sinAngle / scale.x;
    trans.m_val[1][0] =  sinAngle / scale.y;
    // Invert V value so that we don't need to flip textures on load
    trans.m_val[1][1] = -cosAngle / scale.y;

    if (Mode::ElevationDrape == m_mode)
        {
        trans.m_val[0][2] = -(m_offset.x * trans.m_val[0][0] + m_offset.y * trans.m_val[0][1]);
        trans.m_val[1][2] = 1.0 - (m_offset.x * trans.m_val[1][0] + m_offset.y * trans.m_val[1][1]);
        }
    else
        {
        trans.m_val[0][2] = m_offset.x;
        trans.m_val[1][2] = m_offset.y;
        }

    return trans;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextureId RenderingAsset::TextureMap::GetTextureId() const
    {
    return DgnTextureId(m_value[RENDER_MATERIAL_TextureId].asUInt64());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RenderingAsset::TextureMap::IsPatternEnabled() const
    {
    bool patternOff = m_value[RENDER_MATERIAL_PatternOff].asBool();
    return !patternOff;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RenderingAsset::TextureMap RenderingAsset::GetPatternMap() const
    {
    auto maps = GetValue(RENDER_MATERIAL_Map);
    if (maps.isNull())
        return TextureMap(maps, TextureMap::Type::Pattern); // return invalid value

    return TextureMap(maps[RENDER_MATERIAL_MAP_Pattern], TextureMap::Type::Pattern);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void  RenderingAsset::SetColor(Utf8CP keyword, RgbFactor color)
    {
    auto colorValue = GetValueR(keyword);
    colorValue[0] = color.red;
    colorValue[1] = color.green;
    colorValue[2] = color.blue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RenderingAsset::Relocate(DgnImportContext& context)
    {
    auto patternMap = GetPatternMap();
    if (!patternMap.IsValid())
        return ERROR;

    DgnTextureId newId = patternMap.Relocate(context);
    if (!newId.IsValid())
        return ERROR;

    GetValueR(RENDER_MATERIAL_Map)[RENDER_MATERIAL_TextureId] = newId.ToHexStr();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextureId RenderingAsset::TextureMap::Relocate(DgnImportContext& context)
    {
    DgnTextureId thisId = GetTextureId();
    return thisId.IsValid() ? DgnTexture::ImportTexture(context, thisId) : thisId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static void computeParametricUVParams (DPoint2dP params, PolyfaceVisitorCR visitor, TransformCR uvTransform, bool isRelativeUnits)
    {
    for (size_t i=0; i < visitor.NumEdgesThisFace(); i++)
        {
        DPoint2d        param = DPoint2d::From (0.0, 0.0);

        if (isRelativeUnits || !visitor.TryGetDistanceParameter (i, param))
            {
            if (!visitor.TryGetNormalizedParameter (i, param))
                {
				// If the mesh does not have faceData we still want to use the texture coordinates if they are present.
                DPoint2dCP inParams = visitor.GetParamCP();
                if (NULL != inParams)
                    param = inParams[i];
                }
            }

        uvTransform.Multiply (params[i], param);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static void computePlanarUVParams (DPoint2dP params, PolyfaceVisitorCR visitor, TransformCR uvTransform)
    {
    DPoint3dCP  points = visitor.GetPointCP ();
    DPoint3d    upVector, sideVector, normal;

    auto normals = visitor.GetNormalCP();
    auto normalIndices = visitor.GetClientNormalIndexCP();
    if (nullptr != normals && nullptr != normalIndices && (normalIndices[0] != normalIndices[1] || normalIndices[0] != normalIndices[2]))
        {
        // Potentially non-planar facet - compute normal from points below.
        normals = nullptr;
        }

    if (nullptr == normals)
        normal.CrossProductToPoints (points[0], points[1], points[2]);
    else
        normal = *normals;

    normal.Normalize ();

    /* adjust U texture coordinate to be a continuous length starting at the
       origin.  V coordinate stays the same. This mode assumes Z is up vector. */

    // Flipping the normal puts us in a planar coordinate system consistent with display system.
    normal.Scale (-1.0);

    /* pick the first vertex normal */
    sideVector.x =  normal.y;
    sideVector.y = -normal.x;
    sideVector.z =  0.0;

    /* if the magnitude of the normal is near zero, the real normal points
       almost straight up).  In this case, use Y as the up vector in order to
       match display */

    if (sideVector.Normalize () < 1e-3)
        {
        normal.Init (0.0, 0.0, -1.0);
        sideVector.Init (1.0, 0.0, 0.0);
        }

    upVector.NormalizedCrossProduct (sideVector, normal);
    for (size_t i = 0, count = visitor.NumEdgesThisFace(); i<count; i++)
        {
        DPoint2dP   outParam    = params + i;
        DPoint3d    point       = *(points + i);

        outParam->x = point.DotProduct (sideVector);
        outParam->y = point.DotProduct (upVector);

        uvTransform.Multiply (*outParam, *outParam);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static void computeElevationDrapeUVParams (DPoint2dP params, PolyfaceVisitorCR visitor, TransformCR uvTransform, ElevationDrapeParamsCP drapeParams)
    {
    size_t count = visitor.NumEdgesThisFace();
    for (size_t i = 0; i < count; i++)
        {
        DPoint3d    point        = *(visitor.GetPointCP() + i);
        DPoint2dP   outParam    = params + i;

        if (nullptr != drapeParams)
            drapeParams->m_transform.Multiply(point);

        outParam->Init (point);
        uvTransform.Multiply (*outParam, *outParam);
        }

    if (nullptr != drapeParams)
        drapeParams->Normalize(params, count);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ElevationDrapeParams::Normalize(DPoint2dP params, size_t numParams) const
    {
    if (0 == numParams || 0 == m_width || 0 == m_height)
        return;

    // Translate the first coordinate so it is within +/- dimensions from zero. Then translate all other coordinates relative to that.
    DPoint2d first = *params;
    params->x = fmod(first.x, static_cast<double>(m_width));
    params->y = fmod(first.y, static_cast<double>(m_height));

    for (size_t i = 1; i < numParams; i++)
        {
        params[i].x -= first.x - params->x;
        params[i].y -= first.y - params->y;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus RenderingAsset::TextureMap::ComputeUVParams (bvector<DPoint2d>& params,  PolyfaceVisitorCR visitor, ElevationDrapeParamsCP drapeParams) const
    {
    Render::TextureMapping::Params mapParams = GetTextureMapParams();
    return mapParams.ComputeUVParams(params, visitor, drapeParams);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus Render::TextureMapping::Params::ComputeUVParams (bvector<DPoint2d>& params,  PolyfaceVisitorCR visitor, ElevationDrapeParamsCP drapeParams) const
    {
    params.resize (visitor.NumEdgesThisFace());
    switch (m_mapMode)
        {
        default:
            //BeAssert (false && "Material mapping mode not implemented - defaulting to parametric");
            // Fall through...

        case Render::TextureMapping::Mode::Parametric:
            computeParametricUVParams (&params[0], visitor, m_textureMat2x3.GetTransform(), !m_worldMapping);
            return SUCCESS;

        case Render::TextureMapping::Mode::Planar:
            {
            // We ignore planar mode unless master or sub units for scaleMode (TR# 162118).
            if (!m_worldMapping)
                computeParametricUVParams (&params[0], visitor, m_textureMat2x3.GetTransform(), !m_worldMapping);
            else
                computePlanarUVParams (&params[0], visitor, m_textureMat2x3.GetTransform());

            return SUCCESS;
            }

        case Render::TextureMapping::Mode::ElevationDrape:
            computeElevationDrapeUVParams (&params[0], visitor, m_textureMat2x3.GetTransform(), drapeParams);
            return SUCCESS;

#ifdef WIP
        default:
            if (!projectionFromElement)
                { BeAssert (false); return ERROR; }

            computeProjectionUVParams (&params[0], visitor, pTransformToRoot, *this, layer, projInfo, ignoreLayerTransform);
            return SUCCESS;
#endif
        }
    }

