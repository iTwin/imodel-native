/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnColors.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#define MAXFACTOR                       100.0
#define MAXDEGREES                      360.0
#define DEGREEFACTOR                    60.0
#define MINIMUM_SATURATION_FIXEDHUE     40.0

const double VISIBILITY_GOAL       = 40.0;
const int    HSV_SATURATION_WEIGHT = 4;
const int    HSV_VALUE_WEIGHT      = 2;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             07/91
+---------------+---------------+---------------+---------------+---------------+------*/
RgbFactor ColorUtil::ToRgbFactor(ColorDef color)
    {
    double      scale = 1.0 / (double) UCHAR_MAX;
    RgbFactor   rgbFactor;

    rgbFactor.red   = (double) (color.GetRed() * scale);
    rgbFactor.green = (double) (color.GetGreen() * scale);
    rgbFactor.blue  = (double) (color.GetBlue() * scale);

    return rgbFactor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             07/91
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef ColorUtil::FromRgbFactor(RgbFactor rgbFactor)
    {
    double      scale = (double) UCHAR_MAX;
    ColorDef    color;

    color.SetRed((unsigned char) (rgbFactor.red * scale + 0.5));
    color.SetGreen((unsigned char) (rgbFactor.green * scale + 0.5));
    color.SetBlue((unsigned char) (rgbFactor.blue * scale + 0.5));

    return color;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             07/91
+---------------+---------------+---------------+---------------+---------------+------*/
FPoint3d ColorUtil::ToFloatRgb(ColorDef color)
    {
    double      scale = 1.0 / (double) UCHAR_MAX;
    FPoint3d    floatRgb;

    floatRgb.x = (float) (color.GetRed() * scale);
    floatRgb.y = (float) (color.GetGreen() * scale);
    floatRgb.z = (float) (color.GetBlue() * scale);

    return floatRgb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             07/91
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef ColorUtil::FromFloatRgb(FPoint3d floatRgb)
    {
    float       scale = (float) UCHAR_MAX;
    ColorDef    color;

    color.SetRed((unsigned char) (floatRgb.x * scale + 0.5));
    color.SetGreen((unsigned char) (floatRgb.y * scale + 0.5));
    color.SetBlue((unsigned char) (floatRgb.z * scale + 0.5));

    return color;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             07/91
+---------------+---------------+---------------+---------------+---------------+------*/
HsvColorDef ColorUtil::ToHSV(ColorDef color)
    {
    double      min, max, r = color.GetRed(), g = color.GetGreen(), b = color.GetBlue();
    double      delta_rgb, inthue;
    double      red_distance, green_distance, blue_distance;
    HsvColorDef hsv;

    min = (r < g) ? r : g;

    if (b < min)
        min = b;

    max = (r > g) ? r : g;

    if (b > max)
        max = b;

    /* amount of "blackness" present */
    hsv.value = (int)((max/255.0 * MAXFACTOR) + 0.5);
    delta_rgb  = max - min;

    if (max != 0.0)
        hsv.saturation = (int)((delta_rgb / max * MAXFACTOR) + 0.5);
    else
        hsv.saturation = 0;

    if (hsv.saturation)
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

        hsv.hue = (int)(inthue + 0.5);

        if (hsv.hue >= 360)
            hsv.hue = 0;
        }
    else
        {
        hsv.hue = 0;
        }

    return hsv;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             07/91
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef ColorUtil::FromHSV(HsvColorDef hsv)
    {
    ColorDef    color;

    // Check for simple case first.
    if ((!hsv.saturation) || (hsv.hue == -1))
        {
        // hue must be undefined, have no color only white
        int white_level = (int)((255.0 * hsv.value / MAXFACTOR) + 0.5);
        color.SetAllColors(white_level & 0xff);

        return color;
        }

    double      dhue = hsv.hue, dsaturation = hsv.saturation, dvalue = hsv.value;
    double      hue_fractpart;
    int         hue_intpart;
    int         p, q, t, v;

    if (dhue == MAXDEGREES)
        dhue = 0.0;

    dhue /= DEGREEFACTOR; // hue is now [0..6]
    hue_intpart = (int)dhue; // convert double -> int
    hue_fractpart = dhue - (double)(hue_intpart);
    dvalue /= MAXFACTOR;
    dsaturation /= MAXFACTOR;

    p = (int)((dvalue * (1.0 - dsaturation) * 255.0) + 0.5);
    q = (int)((dvalue * (1.0 - (dsaturation * hue_fractpart)) * 255.0) + 0.5);
    t = (int)((dvalue * (1.0 - (dsaturation * (1.0 - hue_fractpart))) * 255.0) + 0.5);
    v = (int)(dvalue * 255 + 0.5);

    switch (hue_intpart)
        {
        case 0: // reddish
            {
            color.SetRed(v & 0xff);
            color.SetGreen(t & 0xff);
            color.SetBlue(p & 0xff);
            break;
            }
        case 1: // yellowish
            {
            color.SetRed(q & 0xff);
            color.SetGreen(v & 0xff);
            color.SetBlue(p & 0xff);
            break;
            }
        case 2: // greenish
            {
            color.SetRed(p & 0xff);
            color.SetGreen(v & 0xff);
            color.SetBlue(t & 0xff);
            break;
            }
        case 3: // cyanish
            {
            color.SetRed(p & 0xff);
            color.SetGreen(q & 0xff);
            color.SetBlue(v & 0xff);
            break;
            }
        case 4: // bluish
            {
            color.SetRed(t & 0xff);
            color.SetGreen(p & 0xff);
            color.SetBlue(v & 0xff);
            break;
            }
        case 5: // magenta-ish
            {
            color.SetRed(v & 0xff);
            color.SetGreen(p & 0xff);
            color.SetBlue(q & 0xff);
            break;
            }
        }

    return color;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static double colorVisibilityCheck(ColorDef fg, ColorDef bg)
    {
    // Compute luminosity...
    double red   = abs(fg.GetRed()   - bg.GetRed());
    double green = abs(fg.GetGreen() - bg.GetGreen());
    double blue  = abs(fg.GetBlue()  - bg.GetBlue());

    return (0.30 * red) + (0.59 * green) + (0.11 * blue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void adjustHSVColor(HsvColorDef& fgHsv, bool darkenColor, int delta)
    {
    if (darkenColor)
        {
        int     weightedDelta = delta*HSV_VALUE_WEIGHT;

        if (fgHsv.value >= weightedDelta)
            {
            fgHsv.value -= weightedDelta;
            }
        else
            {
            weightedDelta -= fgHsv.value;

            fgHsv.value = 0;
            fgHsv.saturation = fgHsv.saturation + weightedDelta < 100 ? fgHsv.saturation + weightedDelta : 100;
            }
        }
    else
        {
        int weightedDelta = delta*HSV_SATURATION_WEIGHT;

        if (fgHsv.saturation >= weightedDelta)
            {
            fgHsv.saturation -= weightedDelta;
            }
        else
            {
            weightedDelta -= fgHsv.saturation;

            fgHsv.saturation = 0;
            fgHsv.value = fgHsv.value + weightedDelta < 100 ? fgHsv.value + weightedDelta : 100;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/09
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef ColorUtil::AdjustForContrast(ColorDef fg, ColorDef bg, ColorDef vw)
    {
    double visibility = colorVisibilityCheck(fg, bg);

    if (VISIBILITY_GOAL <= visibility)
        return fg;

    int         adjPercent = (int) (((VISIBILITY_GOAL - visibility) / 255.0) * 100.0);
    Byte        alpha = fg.GetAlpha();
    HsvColorDef fgHSV = ToHSV(fg);
    HsvColorDef darkerHSV = fgHSV;
    HsvColorDef brightHSV = fgHSV;

    adjustHSVColor(darkerHSV, true,  adjPercent);
    adjustHSVColor(brightHSV, false, adjPercent);

    ColorDef darker = FromHSV(darkerHSV); darker.SetAlpha(alpha); // Preserve original color's transparency...
    ColorDef bright = FromHSV(brightHSV); bright.SetAlpha(alpha);

    if (bright.GetValueNoAlpha() == bg.GetValueNoAlpha()) // Couldn't adjust brighter...
        return darker;

    if (darker.GetValueNoAlpha() == bg.GetValueNoAlpha()) // Couldn't adjust darker...
        return bright;

    // NOTE: Best choice is the one most visible against the background color...
    return (colorVisibilityCheck(bright, vw) >= colorVisibilityCheck(darker, vw)) ? bright : darker;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/09
+---------------+---------------+---------------+---------------+---------------+------*/
void ColorUtil::AdjustValueAndSaturation(ColorDefP colorArray, size_t numColors, double valueAdjustment, double saturationAdjustment, bool valueAndSaturationFixed, double hueValue, bool hueFixed)
    {
    if (hueFixed && valueAndSaturationFixed)
        {
        HsvColorDef hsv;

        hsv.hue         = (int)(hueValue);
        hsv.value       = (int)(valueAdjustment * 100.0 + 0.5);
        hsv.saturation  = (int)(saturationAdjustment * 100.0 + 0.5);

        ColorDef oneColor = FromHSV(hsv);

        for (size_t iColor=0; iColor < numColors; iColor++)
            colorArray[iColor] = oneColor;

        return;
        }

    for (size_t iColor=0; iColor < numColors; iColor++)
        {
        double      newValue;
        double      newSaturation;
        HsvColorDef hsv = ToHSV(colorArray[iColor]);

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

        colorArray[iColor] = FromHSV(hsv);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ColorUtil::Interpolate (ColorDefP interpolatedColors, size_t nInterpolatedColors, ColorDef startColor, ColorDef endColor)
    {
    if (nInterpolatedColors < 2)
        return;

    interpolatedColors[0] = startColor;
    interpolatedColors[nInterpolatedColors-1] = endColor;
                                                                                                               
    RgbFactor startFactor, endFactor, interpolatedFactor;

    startFactor = ToRgbFactor(startColor);
    endFactor   = ToRgbFactor(endColor);

    double step = 1.0 / (double) (nInterpolatedColors - 1.0), endRatio = step, startRatio;

    for (size_t i=1; i<nInterpolatedColors-1; i++, endRatio += step)
        {
        startRatio = 1.0 - endRatio;

        interpolatedFactor.red   = startRatio * startFactor.red   + endRatio * endFactor.red;
        interpolatedFactor.green = startRatio * startFactor.green + endRatio * endFactor.green;
        interpolatedFactor.blue  = startRatio * startFactor.blue  + endRatio * endFactor.blue;

        interpolatedColors[i] = FromRgbFactor(interpolatedFactor);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTrueColorId DgnColors::FindMatchingColor(ColorDef color) const
    {
    Statement stmt(m_dgndb, "SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_Color) " WHERE Value=?");
    stmt.BindInt(1, color.GetValue());

    return (BE_SQLITE_ROW == stmt.Step()) ? stmt.GetValueId<DgnTrueColorId>(0) : DgnTrueColorId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnColors::QueryColor(ColorDef& color, Utf8StringP name, Utf8StringP book, DgnTrueColorId id) const
    {
    Statement stmt(m_dgndb, "SELECT Value,Name,Book FROM " DGN_TABLE(DGN_CLASSNAME_Color) " WHERE Id=?");
    stmt.BindId(1, id);
    if (BE_SQLITE_ROW != stmt.Step())
        return  ERROR;

    color = ColorDef(stmt.GetValueInt(0));

    if (name)
        name->AssignOrClear(stmt.GetValueText(1));

    if (book)
        book->AssignOrClear(stmt.GetValueText(2));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnColors::QueryColorByName(ColorDef& color, Utf8StringCR name, Utf8StringCR book) const
    {
    Statement stmt(m_dgndb, "SELECT Value FROM " DGN_TABLE(DGN_CLASSNAME_Color) " WHERE Name=? AND Book=?");
    stmt.BindText(1, name, Statement::MakeCopy::No);
    stmt.BindText(2, book, Statement::MakeCopy::No);
    if (BE_SQLITE_ROW != stmt.Step())
        return  ERROR;

    color = ColorDef(stmt.GetValueInt(0));
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTrueColorId DgnColors::Insert(ColorDef color, Utf8CP name, Utf8CP book)
    {
    DgnTrueColorId newId;

    auto status = m_dgndb.GetServerIssuedId(newId, DGN_TABLE(DGN_CLASSNAME_Color), "Id");
    if (status != BE_SQLITE_OK)
        {
        BeAssert(false);
        return DgnTrueColorId();
        }

    Statement stmt(m_dgndb, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_Color) " (Id,Value,Name,Book) VALUES(?,?,?,?)");

    stmt.BindId(1, newId);
    stmt.BindInt(2, color.GetValue());
    if (name && *name)
        stmt.BindText(3, name, Statement::MakeCopy::No);

    if (book && *book)
        stmt.BindText(4, book, Statement::MakeCopy::No);

    status = stmt.Step();
    BeAssert(BE_SQLITE_DONE==status);
    return (BE_SQLITE_DONE==status) ? newId : DgnTrueColorId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/11
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnColors::Iterator::QueryCount() const
    {
    Utf8String sqlString = MakeSqlString("SELECT count(*) FROM " DGN_TABLE(DGN_CLASSNAME_Color));

    Statement sql(*m_db, sqlString.c_str());

    return (BE_SQLITE_ROW != sql.Step()) ? 0 : sql.GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnColors::Iterator::const_iterator DgnColors::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sqlString = MakeSqlString("SELECT Id,Value,Name,Book FROM " DGN_TABLE(DGN_CLASSNAME_Color));

        m_db->GetCachedStatement(m_stmt, sqlString.c_str());
        m_params.Bind(*m_stmt);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

DgnTrueColorId DgnColors::Iterator::Entry::GetId() const {Verify(); return m_sql->GetValueId<DgnTrueColorId>(0);}
ColorDef DgnColors::Iterator::Entry::GetColorValue() const {Verify(); return ColorDef(m_sql->GetValueInt(1));}
Utf8CP DgnColors::Iterator::Entry::GetName() const {Verify(); return m_sql->GetValueText(2);}
Utf8CP DgnColors::Iterator::Entry::GetBookName() const {Verify(); return m_sql->GetValueText(3);}
