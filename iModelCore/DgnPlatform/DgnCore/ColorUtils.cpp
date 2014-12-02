/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ColorUtils.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ColorMapping::GetChangeColor (UInt32& remappedColor, UInt32 elementColor, ColorMapping **colorMapPP, DgnModelP destDgnModel, DgnModelP srcDgnModel, bool keepRefColorIndexOnCopy)
    {
    remappedColor = elementColor;
    
    IntColorDef colorDef;
    UInt32      colorIndex;
    bool        isTrueColor;
    Utf8String  bookName, entryName;

    if (SUCCESS != srcDgnModel->GetDgnProject().Colors().Extract (&colorDef, &colorIndex, &isTrueColor, &bookName, &entryName, elementColor))
        return ERROR;

    if (colorMapPP && destDgnModel != srcDgnModel && colorIndex < 0xff)
        {
        DgnColorMapCP   srcColorMap  = DgnColorMap::Get (srcDgnModel->GetDgnProject());
        DgnColorMapCP   destColorMap = DgnColorMap::Get (destDgnModel->GetDgnProject());
        ColorMapping*   colorMapP    = *colorMapPP;

        /* Get color tables for files, initialize remap table */
        if (!colorMapP)
            {
            if (NULL == (colorMapP = static_cast<ColorMapping *>(memutil_calloc (sizeof (ColorMapping), 1, HEAPSIG_GGMP))))
                return ERROR;

            /* Initialize remap table to background color which isn't a color that would be remapped */
            for (int i = 0; i < MAX_CMAPENTRIES; ++i)
                colorMapP->remapTable[i] = 0xff;

            colorMapP->srcDgnModel  = srcDgnModel;
            colorMapP->destDgnModel = destDgnModel;

            /* Don't need to do remapping if they use the same color table - ignore bg color */
            colorMapP->differentTables = !srcColorMap->IsSame (destColorMap);

            /* Create hsv table since that is what is used for rgb test */
            destColorMap->GetHsvColors (colorMapP->destHSVTable);

            *colorMapPP = colorMapP;
            }
        else
            {
            bool    isValid = true;

            /* Source file is different, get new color map */
            if (colorMapP->srcDgnModel != srcDgnModel)
                {
                colorMapP->srcDgnModel = srcDgnModel;
                isValid = false;
                }

            /* Destintation file is different, get new color map */
            if (colorMapP->destDgnModel != destDgnModel)
                {
                colorMapP->destDgnModel = destDgnModel;
                isValid = false;

                /* Save hsv table since that is what is needed for rgb test */
                destColorMap->GetHsvColors (colorMapP->destHSVTable);
                }

            /* Reset remap table due to src/dest file change */
            if (!isValid)
                {
                /* Initialize remap table to background color which isn't a color that would be remapped */
                for (int i = 0; i < MAX_CMAPENTRIES; ++i)
                    colorMapP->remapTable[i] = 0xff;

                /* Don't need to do remapping if they use the same color table - ignore bg color */
                colorMapP->differentTables = !srcColorMap->IsSame (destColorMap);
                }
            }

        /* Does closest match for this index need to be found? NOTE: Never remap bg color index. */
        if (colorMapP->differentTables && 0xff != colorIndex)
            {
            if (colorMapP->remapTable[colorIndex] == 0xff)
                {
                IntColorDef     srcColorDef, destColorDef;

                srcColorDef.m_int  = srcColorMap->GetTbgrColors()[colorIndex];
                destColorDef.m_int = destColorMap->GetTbgrColors()[colorIndex];

                /* Check if color index refers to same rgb even though tables differ. */
                if (srcColorDef.m_int == destColorDef.m_int)
                    colorMapP->remapTable[colorIndex] = colorIndex;
                else
                    {
                    colorMapP->remapTable[colorIndex] = destColorMap->FindClosestMatch (srcColorDef, colorMapP->destHSVTable);

                    // When remapping colors for publishing, get exact match by creating true color if necessary.
                    if (srcColorDef.m_int != destColorMap->GetColor (colorMapP->remapTable[colorIndex]).m_int)
                        {
                        colorMapP->remapTable[colorIndex] = destDgnModel->GetDgnProject().Colors().FindElementColor (srcColorDef);
                        if (INVALID_COLOR == colorMapP->remapTable[colorIndex])
                            colorMapP->remapTable[colorIndex] = destDgnModel->GetDgnProject().Colors().CreateElementColor (srcColorDef, NULL, NULL);
                        }
                    }
                }

            remappedColor = colorMapP->remapTable[colorIndex];
            }
        }

    if (isTrueColor)
        remappedColor = destDgnModel->GetDgnProject().Colors().CreateElementColor (colorDef, bookName.c_str(), entryName.c_str());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 ColorMapping::RemapColor (UInt32 sourceColorIndex, DgnModelP dest, DgnModelP source, bool keepRefColorIndexOnCopy)
    {
    ColorMapping*   colorMap = NULL;
    UInt32          remappedColor;
    
    GetChangeColor (remappedColor, sourceColorIndex, &colorMap, dest, source, keepRefColorIndexOnCopy);

    if (colorMap)
        memutil_free (colorMap);

    return remappedColor;
    }

#define MAXFACTOR                       100.0
#define MAXDEGREES                      360.0
#define DEGREEFACTOR                    60.0
#define MINIMUM_SATURATION_FIXEDHUE     40.0

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             07/91
+---------------+---------------+---------------+---------------+---------------+------*/
void ColorUtil::ConvertRgbToHsv (HsvColorDef* hsv, RgbColorDef const* rgb)
    {
    double      min, max, r = rgb->red, g = rgb->green, b = rgb->blue;
    double      delta_rgb, inthue;
    double      red_distance, green_distance, blue_distance;

    min = (r < g) ? r : g;

    if (b < min)
        min = b;

    max = (r > g) ? r : g;

    if (b > max)
        max = b;

    /* amount of "blackness" present */
    hsv->value = (int)((max/255.0 * MAXFACTOR) + 0.5);
    delta_rgb  = max - min;

    if (max != 0.0)
        hsv->saturation = (int)((delta_rgb / max * MAXFACTOR) + 0.5);
    else
        hsv->saturation = 0;

    if (hsv->saturation)
        {
        red_distance   = (max - r) / delta_rgb;
        green_distance = (max - g) / delta_rgb;
        blue_distance  = (max - b) / delta_rgb;

        if (r == max)           /* color between yellow & magenta */
            inthue = blue_distance - green_distance;
        else if (g == max)      /* color between cyan & yellow */
            inthue = 2.0 + red_distance - blue_distance;
        else                    /* color between magenta & cyan */
            inthue = 4.0 + green_distance - red_distance;

        /* intermediate hue is [0..6] */
        inthue *= DEGREEFACTOR;

        if (inthue < 0.0)
            inthue += MAXDEGREES;

        hsv->hue = (int)(inthue + 0.5);

        if (hsv->hue >= 360)
            hsv->hue = 0;
        }
    else
        {
        hsv->hue = 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
    Extract bytes from int color, return as (doubles) RgbFactor.
* @bsimethod                                                    BSI             07/91
+---------------+---------------+---------------+---------------+---------------+------*/
void ColorUtil::ConvertIntToRGBFactor (RgbFactor& rgbFactor, UInt32 intColor)
    {
    double              scale = 1.0 / (double) UCHAR_MAX;
    const unsigned char *pByte = (const unsigned char *) (&intColor);

    rgbFactor.red   = (double) pByte[0] * scale;
    rgbFactor.green = (double) pByte[1] * scale;
    rgbFactor.blue  = (double) pByte[2] * scale;
    }
 
/*---------------------------------------------------------------------------------**//**
    Scale 0..1 doubles to 0..255, pack as bytes.
* @bsimethod                                                    BSI             07/91
+---------------+---------------+---------------+---------------+---------------+------*/
void ColorUtil::ConvertRGBFactorToInt (UInt32& intColor, RgbFactor &rgbFactor)
    {
    double          scale = (double) UCHAR_MAX;
    unsigned char *pByte = (unsigned char *) (&intColor);
    intColor = 0;
    pByte[0] = (unsigned char) (rgbFactor.red   * scale + 0.5);
    pByte[1] = (unsigned char) (rgbFactor.green * scale + 0.5);
    pByte[2] = (unsigned char) (rgbFactor.blue  * scale + 0.5);
    }

/*---------------------------------------------------------------------------------**//**
 Extract bytes from int color, return as (float) FloatRgb.
* @bsimethod                                                    BSI             07/91
+---------------+---------------+---------------+---------------+---------------+------*/
void ColorUtil::ConvertIntToFloatRgb (FPoint3dR rgb, UInt32 intColor)
    {
    double              scale = 1.0 / (double) UCHAR_MAX;
    const unsigned char *pByte = (const unsigned char *) (&intColor);

    rgb.x   = (float) (pByte[0] * scale);
    rgb.y   = (float) (pByte[1] * scale);
    rgb.z   = (float) (pByte[2] * scale);
    }

/*---------------------------------------------------------------------------------**//**
 Scale 0..1 float to 0..255, pack as bytes.
* @bsimethod                                                    BSI             07/91
+---------------+---------------+---------------+---------------+---------------+------*/
void ColorUtil::ConvertFloatRgbToInt(UInt32 &intColor, FPoint3dR rgb)
    {
    float    scale = (float) UCHAR_MAX;
    unsigned char *pByte = (unsigned char *) (&intColor);
    intColor = 0;
    pByte[0] = (unsigned char) (rgb.x * scale + 0.5);
    pByte[1] = (unsigned char) (rgb.y * scale + 0.5);
    pByte[2] = (unsigned char) (rgb.z * scale + 0.5);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             07/91
+---------------+---------------+---------------+---------------+---------------+------*/
void ColorUtil::ConvertRgbFactorToRgbColorDef (RgbColorDef &colorDef, RgbFactor const& rgbFactor)
    {
    static double  s_scale  = (double) UCHAR_MAX;

    colorDef.red   = (unsigned char) (rgbFactor.red   * s_scale + 0.5);
    colorDef.green = (unsigned char) (rgbFactor.green * s_scale + 0.5);
    colorDef.blue  = (unsigned char) (rgbFactor.blue  * s_scale + 0.5);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             07/91
+---------------+---------------+---------------+---------------+---------------+------*/
void ColorUtil::ConvertRgbColorDefToRgbFactor (RgbFactorR rgbFactor, RgbColorDefCR colorDef)
    {
    static double  s_scale  = 1.0 / (double) UCHAR_MAX;

    rgbFactor.red   = (double) colorDef.red   * s_scale;
    rgbFactor.green = (double) colorDef.green * s_scale;
    rgbFactor.blue  = (double) colorDef.blue  * s_scale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             07/91
+---------------+---------------+---------------+---------------+---------------+------*/
void ColorUtil::ConvertHsvToRgb (RgbColorDef* rgb, HsvColorDef const* hsv)
    {
    double      dhue, dsaturation, dvalue = hsv->value;
    double      hue_fractpart;
    int         white_level, hue_intpart;
    int         p, q, t, v;

    /* Check for simple case first. */
    if ((!hsv->saturation) || (hsv->hue == -1))
        {
        /* hue must be undefined, have no color only white */
        white_level = (int)((255.0 * dvalue / MAXFACTOR) + 0.5);
        rgb->red = rgb->green = rgb->blue = white_level & 0xff;

        return;
        }

    dhue        = hsv->hue;
    dsaturation = hsv->saturation;

    if (dhue == MAXDEGREES)
        dhue = 0.0;

    dhue         /= DEGREEFACTOR;                 /* hue is now [0..6] */
    hue_intpart   = (int)dhue;                    /* convert double -> int */
    hue_fractpart = dhue - (double)(hue_intpart);
    dvalue       /= MAXFACTOR;
    dsaturation  /= MAXFACTOR;

    p = (int)((dvalue * (1.0 - dsaturation) * 255.0) + 0.5);
    q = (int)((dvalue * (1.0 - (dsaturation * hue_fractpart)) * 255.0) + 0.5);
    t = (int)((dvalue * (1.0 - (dsaturation * (1.0 - hue_fractpart))) * 255.0) + 0.5);
    v = (int)(dvalue * 255 + 0.5);

    switch (hue_intpart)
        {
        case 0:     /* reddish */
            {
            rgb->red   = v & 0xff;
            rgb->green = t & 0xff;
            rgb->blue  = p & 0xff;
            break;
            }
        case 1:     /* yellowish */
            {
            rgb->red   = q & 0xff;
            rgb->green = v & 0xff;
            rgb->blue  = p & 0xff;
            break;
            }
        case 2:     /* greenish */
            {
            rgb->red   = p & 0xff;
            rgb->green = v & 0xff;
            rgb->blue  = t & 0xff;
            break;
            }
        case 3:     /* cyanish */
            {
            rgb->red   = p & 0xff;
            rgb->green = q & 0xff;
            rgb->blue  = v & 0xff;
            break;
            }
        case 4:     /* bluish */
            {
            rgb->red   = t & 0xff;
            rgb->green = p & 0xff;
            rgb->blue  = v & 0xff;
            break;
            }
        case 5:     /* magenta-ish */
            {
            rgb->red   = v & 0xff;
            rgb->green = p & 0xff;
            rgb->blue  = q & 0xff;
            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            ColorUtil::AdjustValueAndSaturation
(
RgbColorDef*    colorArray,
UInt32          numColors, 
double          valueAdjustment,
double          saturationAdjustment, 
bool            valueAndSaturationFixed, 
double          hueValue, 
bool            hueFixed
)
    {
    if (hueFixed && valueAndSaturationFixed)
        {
        RgbColorDef     oneColor;
        HsvColorDef     hsv;

        hsv.hue         = (int)(hueValue);
        hsv.value       = (int)(valueAdjustment * 100.0 + 0.5);
        hsv.saturation  = (int)(saturationAdjustment * 100.0 + 0.5);

        ColorUtil::ConvertHsvToRgb (&oneColor, &hsv);

        for (UInt32 iColor=0; iColor < numColors; iColor++)
            colorArray[iColor] = oneColor;

        return;
        }

    for (UInt32 iColor=0; iColor < numColors; iColor++)
        {
        HsvColorDef hsv;
        double      newValue;
        double      newSaturation;

        ColorUtil::ConvertRgbToHsv (&hsv, &colorArray[iColor]);

        if (valueAndSaturationFixed)
            {
            newValue      = valueAdjustment * 100.0;
            newSaturation = saturationAdjustment * 100.0;
            }
        else
            {
            newValue = (double) hsv.value * valueAdjustment;

            if (newValue > 100.0)
                newValue = 100.0;

            if (newValue < 0.0)
                newValue = 0.0;

            newSaturation = (double) hsv.saturation * saturationAdjustment;

            if (newSaturation > 100.0)
                newSaturation = 100.0;

            if (newSaturation < 0.0)
                newSaturation = 0.0;
            }

        if (hueFixed)
            {
            hsv.hue = (int) hueValue;

            // we need a minimum saturation or the hue doesn't work.
            if (newSaturation < MINIMUM_SATURATION_FIXEDHUE)
                newSaturation = MINIMUM_SATURATION_FIXEDHUE;
            }

        hsv.value       = (int)(newValue + 0.5);
        hsv.saturation  = (int)(newSaturation + 0.5);

        ColorUtil::ConvertHsvToRgb (&colorArray[iColor], &hsv);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 ColorUtil::GetV8ElementColor (UInt32 elementColor)
    {
    // NOTE: Can't use DgnColorMap::ExtractElementColorInfo without a file...
    return (COLOR_BYLEVEL == elementColor || COLOR_BYCELL == elementColor) ? elementColor : (elementColor & 0x000000ff);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool ColorUtil::ElementColorsAreEqual (UInt32 color1, DgnModelP modelRef1, UInt32 color2, DgnModelP modelRef2, bool keepRefColorIndexOnCopy)
    {
    if (modelRef1 == modelRef2)
        return color1 == color2;

    if (COLOR_BYCELL == color1 && COLOR_BYCELL == color2)
        return true;

    if (COLOR_BYLEVEL == color1 && COLOR_BYLEVEL == color2)
        return true;

    IntColorDef colorDef1;
    UInt32      colorIndex1;
    bool        isTrueColor1;
    Utf8String  colorName1;
    Utf8String  bookName1;

    if (SUCCESS != modelRef1->GetDgnProject().Colors().Extract (&colorDef1, &colorIndex1, &isTrueColor1, &bookName1, &colorName1, color1))
        return false;

    IntColorDef colorDef2;
    UInt32      colorIndex2;
    bool        isTrueColor2;
    Utf8String  colorName2, bookName2;

    if (SUCCESS != modelRef2->GetDgnProject().Colors().Extract(&colorDef2, &colorIndex2, &isTrueColor2, &bookName2, &colorName2, color2))
        return false;

    if (isTrueColor1 != isTrueColor2)
        return false;

    // Two indexed colors...same color if remap index is the same...
    if (!isTrueColor1 && !isTrueColor2)
        return colorIndex1 == ColorMapping::RemapColor (colorIndex2, modelRef1, modelRef2, keepRefColorIndexOnCopy);

    // Two true colors...
    if (!colorName1.Equals(colorName2) || !bookName1.Equals(bookName2))
        return false;

    // At this point both colors are pure rgb colors.
    return colorDef1.m_int == colorDef2.m_int;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 ColorUtil::GetExtendedIndexFromRawColor (UInt32 rawColor)
    {
    UInt32      highColorBytes = rawColor >> 8;

    if (0 == highColorBytes || COLOR_BYCELL == rawColor || COLOR_BYLEVEL == rawColor || INVALID_COLOR == rawColor)
        return 0; // Not an RGB

    return (rawColor >> 8) & MAX_ExtendedColorIndex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ColorUtil::InterpolateColorsRGB (RgbColorDefP interpolatedColors, size_t nInterpolatedColors, RgbColorDefCR startColor, RgbColorDefCR endColor)
    {
    if (nInterpolatedColors < 2)
        return;

    interpolatedColors[0] = startColor;
    interpolatedColors[nInterpolatedColors-1] = endColor;
                                                                                                               
    RgbFactor       startFactor, endFactor, interpolatedFactor;

    ConvertRgbColorDefToRgbFactor (startFactor, startColor);
    ConvertRgbColorDefToRgbFactor (endFactor, endColor);

    double      step = 1.0 / (double) (nInterpolatedColors - 1.0), endRatio = step, startRatio;
    for (size_t i=1; i<nInterpolatedColors-1; i++, endRatio += step)
        {
        startRatio = 1.0 - endRatio;

        interpolatedFactor.red   = startRatio * startFactor.red   + endRatio * endFactor.red;
        interpolatedFactor.green = startRatio * startFactor.green + endRatio * endFactor.green;
        interpolatedFactor.blue  = startRatio * startFactor.blue  + endRatio * endFactor.blue;

        ConvertRgbFactorToRgbColorDef (interpolatedColors[i], interpolatedFactor);
        }
    }
