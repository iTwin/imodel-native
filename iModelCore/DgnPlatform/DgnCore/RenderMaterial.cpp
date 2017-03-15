/*--------------------------------------------------------------------------------------+                         
|
|     $Source: DgnCore/RenderMaterial.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
RenderingAssetCP RenderingAsset::Load(DgnMaterialId materialId, DgnDbR dgnDb)
    {
    DgnMaterialCPtr material = DgnMaterial::Get(dgnDb, materialId);
    return material.IsValid() ? &material->GetRenderingAsset() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RgbFactor  RenderingAsset::GetColor(Utf8CP name) const
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
Render::Material::MapMode RenderingAsset::TextureMap::GetMode() const 
    {
    Json::Value const& value = m_value[RENDER_MATERIAL_PatternMapping];
    if (!value.isInt())
        {
        BeAssert(false);
        return Render::Material::MapMode::Parametric;
        }

    return (Render::Material::MapMode) value.asInt();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RenderingAsset::TextureMap::Units RenderingAsset::TextureMap::GetUnits() const 
    {
    Json::Value const& value = m_value[RENDER_MATERIAL_PatternScaleMode];
     if (!value.isInt())
        {
        BeAssert(false);
        return Units::Relative;
        }
 
    return (Units) value.asInt();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Ray.Bentley     08/2015
//---------------------------------------------------------------------------------------
Render::Material::Trans2x3 RenderingAsset::TextureMap::GetTransform() const
    {
    Transform   uvTransform = Transform::FromIdentity();
    DPoint2d    scale = GetScale();
    DPoint2d    offset = GetOffset();
    bool        xFlip = GetBool(RENDER_MATERIAL_PatternFlipU, false);
    bool        yFlip = GetBool(RENDER_MATERIAL_PatternFlipV, false);
    double      angleRadians= Angle::DegreesToRadians(GetDouble(RENDER_MATERIAL_PatternAngle, 0.0));
    static double  MIN_MapScale = 1.0E-6;


    if (scale.x > MIN_MapScale)
        uvTransform.ScaleMatrixColumns (((xFlip ? -1.0 : 1.0) / scale.x), 1.0, 1.0);

    if (scale.y > MIN_MapScale)
        uvTransform.ScaleMatrixColumns (1.0, ((yFlip ? -1.0 : 1.0) / scale.y), 1.0);

    if (0.0 != angleRadians)
        {
        RotMatrix rotateMatrix;
        rotateMatrix.InitFromAxisAndRotationAngle (2, angleRadians);
        uvTransform.InitProduct (uvTransform, rotateMatrix);
        }

    if (Render::Material::MapMode::ElevationDrape == GetMode())
        uvTransform.TranslateInLocalCoordinates (uvTransform, -offset.x, -offset.y, 0.0);
    else
        uvTransform.SetTranslation (DPoint3d::FromXYZ (offset.x, -offset.y, 0.0));

    Render::Material::Trans2x3 trans;

    for (size_t i=0; i<2; i++)
        for (size_t j=0; j<2; j++)
            trans.m_val[i][j] = uvTransform.form3d[i][j];


    trans.m_val[0][2] = uvTransform.form3d[0][3];
    trans.m_val[1][2] = uvTransform.form3d[1][3];

    return trans;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextureId RenderingAsset::TextureMap::GetTextureId() const
    {
    JsonValueCR textureIdValue = m_value[RENDER_MATERIAL_TextureId];
    return textureIdValue.isNull() ? DgnTextureId() : DgnTextureId(textureIdValue.asUInt64());
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

    GetValueR(RENDER_MATERIAL_Map)[RENDER_MATERIAL_TextureId] = newId.GetValue();
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
static void computeParametricUVParams (DPoint2dP params, PolyfaceVisitorCR visitor, TransformCR uvTransform, RenderingAsset::TextureMap::Units units)
    {
    for (size_t i=0; i < visitor.NumEdgesThisFace(); i++)
        {
        DPoint2d        param = DPoint2d::From (0.0, 0.0);

        if (RenderingAsset::TextureMap::Units::Relative == units || !visitor.TryGetDistanceParameter (i, param))
            visitor.TryGetNormalizedParameter (i, param);

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
    Transform           uvTransform = GetTransform().GetTransform();

    params.resize (visitor.NumEdgesThisFace());
    switch (GetMode())
        {
        default:
            BeAssert (false && "Material mapping mode not implemented - defaulting to parametric");
            // Fall through...

        case Render::Material::MapMode::Parametric:
            computeParametricUVParams (&params[0], visitor, uvTransform, GetUnits());
            return SUCCESS;

        case Render::Material::MapMode::Planar:
            {
            int32_t const*        normalIndices =visitor.GetClientNormalIndexCP();

            // We ignore planar mode unless master or sub units for scaleMode (TR# 162118) and facet is planar.
            if (Units::Relative == GetUnits () || (NULL != visitor.GetNormalCP () && (normalIndices[0] != normalIndices[1] || normalIndices[0] != normalIndices[2])))
                computeParametricUVParams (&params[0], visitor, uvTransform, GetUnits());
            else
                computePlanarUVParams (&params[0], visitor, uvTransform);

            return SUCCESS;
            }

        case Render::Material::MapMode::ElevationDrape:
            computeElevationDrapeUVParams (&params[0], visitor, uvTransform, transformToDgn);
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

