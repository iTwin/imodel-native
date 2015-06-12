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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             07/91
+---------------+---------------+---------------+---------------+---------------+------*/
void ColorUtil::ConvertRgbToHsv(HsvColorDef* hsv, ColorDef const* rgb)
    {
    double      min, max, r = rgb->GetRed(), g = rgb->GetGreen(), b = rgb->GetBlue();
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
void ColorUtil::ConvertIntToRGBFactor(RgbFactor& rgbFactor, ColorDef intColor)
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
void ColorUtil::ConvertRGBFactorToInt(ColorDef& intColor, RgbFactor &rgbFactor)
    {
    double          scale = (double) UCHAR_MAX;
    unsigned char *pByte = (unsigned char *) (&intColor);
    intColor = ColorDef::Black();
    pByte[0] = (unsigned char) (rgbFactor.red   * scale + 0.5);
    pByte[1] = (unsigned char) (rgbFactor.green * scale + 0.5);
    pByte[2] = (unsigned char) (rgbFactor.blue  * scale + 0.5);
    }

/*---------------------------------------------------------------------------------**//**
 Extract bytes from int color, return as (float) FloatRgb.
* @bsimethod                                                    BSI             07/91
+---------------+---------------+---------------+---------------+---------------+------*/
void ColorUtil::ConvertIntToFloatRgb(FPoint3dR rgb, ColorDef intColor)
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
void ColorUtil::ConvertFloatRgbToInt(ColorDef &intColor, FPoint3dR rgb)
    {
    float    scale = (float) UCHAR_MAX;
    unsigned char *pByte = (unsigned char *) (&intColor);
    intColor = ColorDef::Black();
    pByte[0] = (unsigned char) (rgb.x * scale + 0.5);
    pByte[1] = (unsigned char) (rgb.y * scale + 0.5);
    pByte[2] = (unsigned char) (rgb.z * scale + 0.5);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             07/91
+---------------+---------------+---------------+---------------+---------------+------*/
void ColorUtil::ConvertRgbFactorToColorDef(ColorDef &colorDef, RgbFactor const& rgbFactor)
    {
    static double  s_scale  = (double) UCHAR_MAX;

    colorDef.SetRed((unsigned char) (rgbFactor.red   * s_scale + 0.5));
    colorDef.SetGreen((unsigned char) (rgbFactor.green * s_scale + 0.5));
    colorDef.SetBlue((unsigned char) (rgbFactor.blue  * s_scale + 0.5));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             07/91
+---------------+---------------+---------------+---------------+---------------+------*/
void ColorUtil::ConvertColorDefToRgbFactor(RgbFactorR rgbFactor, ColorDef colorDef)
    {
    static double  s_scale  = 1.0 / (double) UCHAR_MAX;

    rgbFactor.red   = (double) colorDef.GetRed()   * s_scale;
    rgbFactor.green = (double) colorDef.GetGreen() * s_scale;
    rgbFactor.blue  = (double) colorDef.GetBlue()  * s_scale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             07/91
+---------------+---------------+---------------+---------------+---------------+------*/
void ColorUtil::ConvertHsvToRgb(ColorDef* rgb, HsvColorDef const* hsv)
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
        rgb->SetAllColors(white_level & 0xff);
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
            rgb->SetRed(v & 0xff);
            rgb->SetGreen(t & 0xff);
            rgb->SetBlue(p & 0xff);
            break;
            }
        case 1:     /* yellowish */
            {
            rgb->SetRed(q & 0xff);
            rgb->SetGreen(v & 0xff);
            rgb->SetBlue(p & 0xff);
            break;
            }
        case 2:     /* greenish */
            {
            rgb->SetRed(p & 0xff);
            rgb->SetGreen(v & 0xff);
            rgb->SetBlue(t & 0xff);
            break;
            }
        case 3:     /* cyanish */
            {
            rgb->SetRed(p & 0xff);
            rgb->SetGreen(q & 0xff);
            rgb->SetBlue(v & 0xff);
            break;
            }
        case 4:     /* bluish */
            {
            rgb->SetRed(t & 0xff);
            rgb->SetGreen(p & 0xff);
            rgb->SetBlue(v & 0xff);
            break;
            }
        case 5:     /* magenta-ish */
            {
            rgb->SetRed(v & 0xff);
            rgb->SetGreen(p & 0xff);
            rgb->SetBlue(q & 0xff);
            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/09
+---------------+---------------+---------------+---------------+---------------+------*/
void ColorUtil::AdjustValueAndSaturation(ColorDef* colorArray, uint32_t numColors, double valueAdjustment, double saturationAdjustment, bool valueAndSaturationFixed, double hueValue, bool hueFixed)
    {
    if (hueFixed && valueAndSaturationFixed)
        {
        ColorDef     oneColor;
        HsvColorDef     hsv;

        hsv.hue         = (int)(hueValue);
        hsv.value       = (int)(valueAdjustment * 100.0 + 0.5);
        hsv.saturation  = (int)(saturationAdjustment * 100.0 + 0.5);

        ColorUtil::ConvertHsvToRgb(&oneColor, &hsv);

        for (uint32_t iColor=0; iColor < numColors; iColor++)
            colorArray[iColor] = oneColor;

        return;
        }

    for (uint32_t iColor=0; iColor < numColors; iColor++)
        {
        HsvColorDef hsv;
        double      newValue;
        double      newSaturation;

        ColorUtil::ConvertRgbToHsv(&hsv, &colorArray[iColor]);

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

        ColorUtil::ConvertHsvToRgb(&colorArray[iColor], &hsv);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ColorUtil::InterpolateColorsRGB (ColorDefP interpolatedColors, size_t nInterpolatedColors, ColorDef startColor, ColorDef endColor)
    {
    if (nInterpolatedColors < 2)
        return;

    interpolatedColors[0] = startColor;
    interpolatedColors[nInterpolatedColors-1] = endColor;
                                                                                                               
    RgbFactor       startFactor, endFactor, interpolatedFactor;

    ConvertColorDefToRgbFactor(startFactor, startColor);
    ConvertColorDefToRgbFactor(endFactor, endColor);

    double      step = 1.0 / (double) (nInterpolatedColors - 1.0), endRatio = step, startRatio;
    for (size_t i=1; i<nInterpolatedColors-1; i++, endRatio += step)
        {
        startRatio = 1.0 - endRatio;

        interpolatedFactor.red   = startRatio * startFactor.red   + endRatio * endFactor.red;
        interpolatedFactor.green = startRatio * startFactor.green + endRatio * endFactor.green;
        interpolatedFactor.blue  = startRatio * startFactor.blue  + endRatio * endFactor.blue;

        ConvertRgbFactorToColorDef(interpolatedColors[i], interpolatedFactor);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTrueColorId DgnColors::FindMatchingColor(ColorDef color) const
    {
    Statement stmt(m_dgndb, "SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_Color) " WHERE Value=?");
    stmt.BindInt(1, color.GetValue());

    return  (BE_SQLITE_ROW == stmt.Step()) ? stmt.GetValueId<DgnTrueColorId>(0) : DgnTrueColorId();
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
    stmt.BindText(1, name, Statement::MakeCopy::No);
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

    auto status = m_dgndb.GetNextRepositoryBasedId(newId, DGN_TABLE(DGN_CLASSNAME_Color), "Id");
    BeAssert(status == BE_SQLITE_OK);

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
