/*--------------------------------------------------------------------------------------+                         
|
|     $Source: DgnCore/RenderMaterial.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus JsonRenderMaterial::Load(DgnMaterialId materialId, DgnDbR dgnDb)
    {
    DgnMaterialCPtr material = DgnMaterial::QueryMaterial(materialId, dgnDb);
    return material.IsValid() ? material->GetRenderingAsset(m_value) : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RgbFactor  JsonRenderMaterial::GetColor(Utf8CP name) const
    {
    RgbFactor rgb = {0.0, 0.0, 0.0};

    JsonValueCR value = m_value[name];
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

DPoint2d JsonRenderMaterial::TextureMap::GetScale() const {return getDPoint2dValue(m_value, RENDER_MATERIAL_PatternScale);}
DPoint2d JsonRenderMaterial::TextureMap::GetOffset() const {return getDPoint2dValue(m_value, RENDER_MATERIAL_PatternOffset);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double JsonRenderMaterial::TextureMap::GetUnitScale(Units units) const
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
Render::Material::MapMode JsonRenderMaterial::TextureMap::GetMode() const 
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
JsonRenderMaterial::TextureMap::Units JsonRenderMaterial::TextureMap::GetUnits() const 
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
JsonRenderMaterial::TextureMap::Trans JsonRenderMaterial::TextureMap::GetTransform() const
    {
    Trans trans;

    for (size_t i=0; i<2; i++)
        for (size_t j=0; j<3; j++)
            trans.m_val[i][j] = (i==j) ? 1.0 : 0.0;
    
    double          angleRadians    = Angle::DegreesToRadians(GetDouble(RENDER_MATERIAL_PatternAngle, 0.0));
    double          cosAngle        = cos(angleRadians);
    double          sinAngle        = sin(angleRadians);
    bool            xFlip           = GetBool(RENDER_MATERIAL_PatternFlipU, false);
    bool            yFlip           = GetBool(RENDER_MATERIAL_PatternFlipV, false);
    DPoint2d        scale           = GetScale();
    DPoint2d        offset          = GetOffset();
    static double   s_minScale      = 1.0E-10;
 
    Units units = GetUnits();
    if (Units::Relative != units)
        scale.Scale(GetUnitScale(units));

    scale.x = std::max(scale.x, s_minScale);
    scale.y = std::max(scale.y, s_minScale);

    if (xFlip)
        scale.x = -scale.x;

    if (yFlip)
        scale.y = -scale.y;

    if (Render::Material::MapMode::ElevationDrape == GetMode())
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
* @bsimethod                                    Keith.Bentley                   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextureId JsonRenderMaterial::TextureMap::GetTextureId() const
    {
    JsonValueCR textureIdValue = m_value[RENDER_MATERIAL_TextureId];
    return textureIdValue.isNull() ? DgnTextureId() : DgnTextureId(textureIdValue.asUInt64());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
JsonRenderMaterial::TextureMap JsonRenderMaterial::GetPatternMap()
    {
    JsonValueCR maps = m_value[RENDER_MATERIAL_Map];
    if (maps.isNull())
        return TextureMap(maps, TextureMap::Type::Pattern); // return invalid value

    return TextureMap(maps[RENDER_MATERIAL_MAP_Pattern], TextureMap::Type::Pattern);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void  JsonRenderMaterial::SetColor(Utf8CP keyword, RgbFactor color)
    {
    Json::Value    colorValue;

    colorValue[0] = color.red;
    colorValue[1] = color.green;
    colorValue[2] = color.blue;

    m_value[keyword] = colorValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus JsonRenderMaterial::Relocate(DgnImportContext& context) 
    {
    JsonRenderMaterial::TextureMap patternMap = GetPatternMap();
    if (!patternMap.IsValid())
        return ERROR;

    DgnTextureId newId = patternMap.Relocate(context);
    if (!newId.IsValid())
        return ERROR;

    m_value[RENDER_MATERIAL_Map][RENDER_MATERIAL_TextureId] = newId.GetValue();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextureId JsonRenderMaterial::TextureMap::Relocate(DgnImportContext& context) 
    {
    DgnTextureId thisId = GetTextureId();
    return thisId.IsValid() ? DgnTexture::ImportTexture(context, thisId) : thisId;
    }
