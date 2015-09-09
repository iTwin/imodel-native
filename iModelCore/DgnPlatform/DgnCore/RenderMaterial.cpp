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
void     RenderMaterial::SetColor (Json::Value& renderMaterial, char const* keyword, RgbFactorCR color)
    {
    Json::Value     colorValue;

    colorValue[0] = color.red;
    colorValue[1] = color.green;
    colorValue[2] = color.blue;

    renderMaterial[keyword] = colorValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void     RenderMaterial::SetPoint (Json::Value& renderMaterial, char const* keyword, DPoint3dCR point)
    {
    Json::Value     pointValue;

    pointValue[0] = point.x;
    pointValue[1] = point.y;
    pointValue[2] = point.z;

    renderMaterial[keyword] = pointValue;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double     RenderMaterial::GetMapUnitScale (MapUnits mapUnits)
    {
    switch (mapUnits)
        {
        default:
            BeAssert (false);
            return 1.0;

        case MapUnits::Meters:
            return 1.0;

        case MapUnits::Millimeters:
            return UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters).GetConversionFactorFrom (UnitDefinition::GetStandardUnit (StandardUnit::MetricMillimeters));

        case MapUnits::Feet:
            return UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters).GetConversionFactorFrom (UnitDefinition::GetStandardUnit (StandardUnit::EnglishFeet));
            
        case MapUnits::Inches:
            return UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters).GetConversionFactorFrom (UnitDefinition::GetStandardUnit (StandardUnit::EnglishInches));
        }
    }

