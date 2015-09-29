/*--------------------------------------------------------------------------------------+                         
|
|     $Source: DgnCore/RenderMaterial.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double Texture::GetUnitScale(Units units)
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
Texture::Mode Texture::GetMode(Json::Value const& in) 
    {
    Json::Value const& value = in[RENDER_MATERIAL_PatternMapping];

    if (!value.isInt())
        {
        BeAssert(false);
        return Mode::Parametric;
        }

    return (Mode) value.asInt();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Texture::Units Texture::GetUnits(Json::Value const& in) 
    {
    Json::Value const& value = in[RENDER_MATERIAL_PatternScaleMode];

    if (!value.isInt())
        {
        BeAssert(false);
        return Units::Relative;
        }
    return (Units) value.asInt();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley     08/2015
//---------------------------------------------------------------------------------------
static DPoint2d getDPoint2dValue(Json::Value const& value)
    {
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

//---------------------------------------------------------------------------------------
// @bsimethod                                               Ray.Bentley     08/2015
//---------------------------------------------------------------------------------------
Render::Texture::Trans Render::Texture::GetPatternTransform(Json::Value const& value)
    {
    Render::Texture::Trans transform;
    for (size_t i=0; i<2; ++i)
        {
        for (size_t j=0; j<3; ++j)
            transform.m_val[i][j] = (i==j) ? 1.0 : 0.0;
        }
    
    double          angleRadians    = Angle::DegreesToRadians(value[RENDER_MATERIAL_PatternAngle].asDouble());
    double          cosAngle        = cos(angleRadians);
    double          sinAngle        = sin(angleRadians);
    bool            xFlip           = value[RENDER_MATERIAL_PatternFlipU].asBool();
    bool            yFlip           = value[RENDER_MATERIAL_PatternFlipV].asBool();
    DPoint2d        scale           = getDPoint2dValue(value[RENDER_MATERIAL_PatternScale]);
    DPoint2d        offset          = getDPoint2dValue(value[RENDER_MATERIAL_PatternOffset]);
    static double   s_minScale      = 1.0E-10;
 
    Units units = GetUnits(value);
    if (Units::Relative != units)
        scale.Scale(GetUnitScale(units));

    scale.x = std::max(scale.x, s_minScale);
    scale.y = std::max(scale.y, s_minScale);

    if (xFlip)
        scale.x = -scale.x;

    if (yFlip)
        scale.y = -scale.y;

    transform.m_val[0][0] = cosAngle / scale.x;
    transform.m_val[0][1] = sinAngle / scale.x;
    transform.m_val[0][2] = offset.x;

    transform.m_val[1][0] =  sinAngle / scale.y;
    transform.m_val[1][1] = -cosAngle / scale.y;
    transform.m_val[1][2] = offset.x;

    return transform;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Texture::SetColor(Json::Value& out, Utf8CP keyword, RgbFactorCR color)
    {
    Json::Value     colorValue;

    colorValue[0] = color.red;
    colorValue[1] = color.green;
    colorValue[2] = color.blue;

    out[keyword] = colorValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Texture::SetPoint(Json::Value& out, Utf8CP keyword, DPoint3dCR point)
    {
    Json::Value     pointValue;

    pointValue[0] = point.x;
    pointValue[1] = point.y;
    pointValue[2] = point.z;

    out[keyword] = pointValue;
    }
