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

