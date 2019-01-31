/*--------------------------------------------------------------------------------------+                         
|
|     $Source: DgnCore/RenderMaterial.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RgbFactor RenderingAsset::GetColor(Utf8CP name) const
    {
    RgbFactor rgb = {0.0, 0.0, 0.0};

    JsonValueCR value = GetValue(name);
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
// @bsimethod                                                   Ray.Bentley     08/2015
//---------------------------------------------------------------------------------------
static DPoint2d getDPoint2dValue(JsonValueCR rootValue, Utf8CP key)
    {
    JsonValueCR value = rootValue[key];
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
* @bsimethod                                                    RayBentley      08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double RenderingAsset::TextureMap::GetUnitScale(Units units) const
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
* @bsimethod                                                    RayBentley      08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Render::TextureMapping::Mode RenderingAsset::TextureMap::GetMode() const 
    {
    return (Render::TextureMapping::Mode) m_value[RENDER_MATERIAL_PatternMapping].asInt((int)Render::TextureMapping::Mode::Parametric);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
double RenderingAsset::TextureMap::GetPatternWeight() const
    {
    return m_value[RENDER_MATERIAL_PatternWeight].asDouble(1.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Marc Neely      08/2017
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
* @bsimethod                                                    RayBentley      08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RenderingAsset::TextureMap::Units RenderingAsset::TextureMap::GetUnits() const 
    {
    return (Units) m_value[RENDER_MATERIAL_PatternScaleMode].asInt((int)Units::Relative);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Ray.Bentley     08/2015
//---------------------------------------------------------------------------------------
Render::TextureMapping::Trans2x3 RenderingAsset::TextureMap::GetTransform() const
    {
    Render::TextureMapping::Trans2x3 trans;

    for (size_t i=0; i<2; i++)
        {
        for (size_t j=0; j<3; j++)
            trans.m_val[i][j] = (i==j) ? 1.0 : 0.0;
        }
    
    double angleRadians= Angle::DegreesToRadians(GetDouble(RENDER_MATERIAL_PatternAngle, 0.0));
    double cosAngle = cos(angleRadians);
    double sinAngle = sin(angleRadians);
    bool xFlip = GetBool(RENDER_MATERIAL_PatternFlipU, false);
    bool yFlip = GetBool(RENDER_MATERIAL_PatternFlipV, false);
    DPoint2d scale = GetScale();
    DPoint2d offset = GetOffset();
    static double s_minScale = 1.0E-10;
 
    Units units = GetUnits();
    if (Units::Relative != units)
        scale.Scale(GetUnitScale(units));

    if (fabs(scale.x) < s_minScale)
        scale.x = s_minScale;

    if (fabs(scale.y) < s_minScale)
        scale.y = s_minScale;

    if (xFlip)
        scale.x = -scale.x;

    if (yFlip)
        scale.y = -scale.y;

    if (Render::TextureMapping::Mode::ElevationDrape == GetMode())
        {
        trans.m_val[0][0] = cosAngle / scale.x;
        trans.m_val[0][1] = sinAngle / scale.x;
        trans.m_val[0][2] = -(offset.x * trans.m_val[0][0] + offset.y * trans.m_val[0][1]);

        trans.m_val[1][0] = sinAngle / scale.y;
        trans.m_val[1][1] = -cosAngle / scale.y;
        trans.m_val[1][2] = 1.0 - (offset.x * trans.m_val[1][0] + offset.y * trans.m_val[1][1]);
        }
    else
        {
        trans.m_val[0][0] = cosAngle / scale.x;
        trans.m_val[0][1] = sinAngle / scale.x;
        trans.m_val[0][2] = offset.x;

        trans.m_val[1][0] =  sinAngle / scale.y;
        trans.m_val[1][1] = -cosAngle / scale.y;
        trans.m_val[1][2] = offset.y;
        }

    return trans;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/19
+---------------+---------------+---------------+---------------+---------------+------*/
Render::TextureMapping::Trans2x3 Render::TextureMapping::Trans2x3::FromRotationAndScale(Angle angle, DPoint2dCP pScale)
    {
    Trans2x3 tf;
    double cosA = angle.Cos();
    double sinA = angle.Sin();

    static double s_minScale = 1.0E-10;
    DPoint2d scale = nullptr != pScale ? *pScale : DPoint2d::From(1.0, 1.0);
    if (fabs(scale.x) < s_minScale)
        scale.x = scale.x < 0.0 ? -s_minScale : s_minScale;

    if (fabs(scale.y) < s_minScale)
        scale.y = scale.y < 0.0 ? -s_minScale : s_minScale;

    tf.m_val[0][0] = cosA / scale.x;
    tf.m_val[0][1] = sinA / scale.x;
    tf.m_val[1][0] = sinA / scale.y;
    tf.m_val[1][1] = -cosA / scale.y;

    tf.m_val[0][2] = tf.m_val[1][2] = 0.0;

    return tf;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextureId RenderingAsset::TextureMap::GetTextureId() const
    {
    return DgnTextureId(m_value[RENDER_MATERIAL_TextureId].asUInt64());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
RenderingAsset::TextureMap RenderingAsset::GetPatternMap() const
    {
    JsonValueCR maps = GetValue(RENDER_MATERIAL_Map);
    if (maps.isNull())
        return TextureMap(maps, TextureMap::Type::Pattern); // return invalid value

    return TextureMap(maps[RENDER_MATERIAL_MAP_Pattern], TextureMap::Type::Pattern);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void  RenderingAsset::SetColor(Utf8CP keyword, RgbFactor color)
    {
    JsonValueR colorValue = GetValueR(keyword);
    colorValue[0] = color.red;
    colorValue[1] = color.green;
    colorValue[2] = color.blue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RenderingAsset::Relocate(DgnImportContext& context) 
    {
    auto patternMap = GetPatternMap();
    if (!patternMap.IsValid())
        return ERROR;

    DgnTextureId newId = patternMap.Relocate(context);
    if (!newId.IsValid())
        return ERROR;

    GetValueR(RENDER_MATERIAL_Map)[RENDER_MATERIAL_TextureId] = newId.ToHexStr(); // ###INT64TOHEXSTR Was newId.GetValue() which fails on imodel-js, asUInt64 still works with ToHexStr.
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextureId RenderingAsset::TextureMap::Relocate(DgnImportContext& context) 
    {
    DgnTextureId thisId = GetTextureId();
    return thisId.IsValid() ? DgnTexture::ImportTexture(context, thisId) : thisId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley     10/2016
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
// @bsimethod                                                   Ray.Bentley     10/2016
//---------------------------------------------------------------------------------------
static void computePlanarUVParams (DPoint2dP params, PolyfaceVisitorCR visitor, TransformCR uvTransform)
    {
    DPoint3dCP  points = visitor.GetPointCP ();
    DPoint3d    upVector, sideVector, normal;

    if (NULL == visitor.GetNormalCP())
        normal.CrossProductToPoints (points[0], points[1], points[2]);
    else
        normal = *visitor.GetNormalCP();

    normal.Normalize ();

    /* adjust U texture coordinate to be a continuous length starting at the
       origin.  V coordinate stays the same. This mode assumes Z is up vector. */

    // Flipping the normal puts us in a planar coordinate system consistent with MicroStation's display system.
    normal.Scale (-1.0);

    /* pick the first vertex normal */
    sideVector.x =  normal.y;
    sideVector.y = -normal.x;
    sideVector.z =  0.0;

    /* if the magnitude of the normal is near zero, the real normal points
       almost straight up).  In this case, use Y as the up vector in order to
       match QV */

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
// @bsimethod                                                   Ray.Bentley     10/2016
//---------------------------------------------------------------------------------------
static void computeElevationDrapeUVParams (DPoint2dP params, PolyfaceVisitorCR visitor, TransformCR uvTransform, TransformCP transformToDgn)
    {
    for (size_t i = 0, count = visitor.NumEdgesThisFace(); i<count; i++)
        {
        DPoint3d    point        = *(visitor.GetPointCP() + i);
        DPoint2dP   outParam    = params + i;

        if (nullptr != transformToDgn)
            transformToDgn->Multiply(point);

        outParam->Init (point);
        uvTransform.Multiply (*outParam, *outParam);
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley     08/2016
//---------------------------------------------------------------------------------------
BentleyStatus RenderingAsset::TextureMap::ComputeUVParams (bvector<DPoint2d>& params,  PolyfaceVisitorCR visitor, TransformCP transformToDgn) const
    {
    Render::TextureMapping::Params mapParams = GetTextureMapParams();
    return mapParams.ComputeUVParams(params, visitor, transformToDgn);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley     08/2016
//---------------------------------------------------------------------------------------
BentleyStatus Render::TextureMapping::Params::ComputeUVParams (bvector<DPoint2d>& params,  PolyfaceVisitorCR visitor, TransformCP transformToDgn) const
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
            int32_t const*        normalIndices =visitor.GetClientNormalIndexCP();

            // We ignore planar mode unless master or sub units for scaleMode (TR# 162118) and facet is planar.
            if (!m_worldMapping || (NULL != visitor.GetNormalCP () && (normalIndices[0] != normalIndices[1] || normalIndices[0] != normalIndices[2])))
                computeParametricUVParams (&params[0], visitor, m_textureMat2x3.GetTransform(), !m_worldMapping);
            else
                computePlanarUVParams (&params[0], visitor, m_textureMat2x3.GetTransform());

            return SUCCESS;
            }

        case Render::TextureMapping::Mode::ElevationDrape:
            computeElevationDrapeUVParams (&params[0], visitor, m_textureMat2x3.GetTransform(), transformToDgn);
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

