/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ColorUtil.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatform.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass                                                     Brien.Bastings  05/09
//=======================================================================================
struct ColorUtil
{
DGNPLATFORM_EXPORT static void AdjustValueAndSaturation (ColorDef* colorArray, uint32_t numColors, double valueAdjustment, double saturationAdjustment, bool valueAndSaturationFixed, double hueValue, bool hueFixed);
DGNPLATFORM_EXPORT static void ConvertRgbToHsv (HsvColorDef* hsv, ColorDef const* rgb);
DGNPLATFORM_EXPORT static void ConvertHsvToRgb (ColorDef* rgb, HsvColorDef const* hsv);
DGNPLATFORM_EXPORT static void ConvertIntToRGBFactor (RgbFactor &rgbFactor, ColorDef intColor);
DGNPLATFORM_EXPORT static void ConvertRGBFactorToInt (ColorDef &intColor, RgbFactor &rgbFactor);
DGNPLATFORM_EXPORT static void ConvertIntToFloatRgb (FPoint3dR rgb, ColorDef intColor);
DGNPLATFORM_EXPORT static void ConvertFloatRgbToInt (ColorDef &intColor, FPoint3dR rgb);
DGNPLATFORM_EXPORT static void ConvertRgbFactorToColorDef (ColorDefR colorDef, RgbFactorCR rgbFactor);
DGNPLATFORM_EXPORT static void ConvertColorDefToRgbFactor (RgbFactorR rgbFactpr, ColorDef colorDef);
DGNPLATFORM_EXPORT static void InterpolateColorsRGB (ColorDefP interpolatedColors, size_t nInterpolatedColors, ColorDef startColor, ColorDef endColor);
};

END_BENTLEY_DGN_NAMESPACE
