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

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct HsvColorDef
{
int32_t  hue;           /* red=0, yellow, green, cyan, blue, magenta */
int32_t  saturation;    /* 0=white, 100=no white, tints */
int32_t  value;         /* 0=black, 100=no black, shades */
};

//=======================================================================================
// @bsiclass                                                     Brien.Bastings  05/09
//=======================================================================================
struct ColorUtil
{
DGNPLATFORM_EXPORT static RgbFactor     ToRgbFactor(ColorDef);
DGNPLATFORM_EXPORT static ColorDef      FromRgbFactor(RgbFactor);

DGNPLATFORM_EXPORT static FPoint3d      ToFloatRgb(ColorDef);
DGNPLATFORM_EXPORT static ColorDef      FromFloatRgb(FPoint3d);

DGNPLATFORM_EXPORT static HsvColorDef   ToHSV(ColorDef);
DGNPLATFORM_EXPORT static ColorDef      FromHSV(HsvColorDef);

DGNPLATFORM_EXPORT static ColorDef      AdjustForContrast(ColorDef color, ColorDef againstColor, ColorDef bgColor);
DGNPLATFORM_EXPORT static void          AdjustValueAndSaturation (ColorDefP colors, size_t nColors, double valueAdjustment, double saturationAdjustment, bool valueAndSaturationFixed, double hueValue, bool hueFixed);
DGNPLATFORM_EXPORT static void          Interpolate(ColorDefP interpolatedColors, size_t nInterpolatedColors, ColorDef startColor, ColorDef endColor);

}; // ColorUtil

END_BENTLEY_DGNPLATFORM_NAMESPACE
