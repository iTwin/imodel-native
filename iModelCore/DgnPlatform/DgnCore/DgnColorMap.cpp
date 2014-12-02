/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnColorMap.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#define LOG (*LoggingManager::GetLogger (L"DgnCore"))

struct ColorTableParams
    {
    unsigned char   redInit;
    unsigned char   grnInit;
    unsigned char   bluInit;
    int             redDelta;
    int             grnDelta;
    int             bluDelta;
    };

static ColorTableParams s_defColorTblParams[8] =
    {
    {   0, 255, 255,      0, -15, -15 }, /* cyan   scale */  
    { 255, 255, 255,    -15, -15, -15 }, /* white  scale */  
    {   0,   0, 255,      0,   0, -15 }, /* blue   scale */  
    {   0, 255,   0,      0, -15,   0 }, /* green  scale */  
    { 255,   0,   0,    -15,   0,   0 }, /* red    scale */  
    { 255, 255,   0,    -15, -15,   0 }, /* yellow scale */  
    { 255,   0, 255,    -15,   0, -15 }, /* violet scale */  
    { 255, 127,   0,    -15,  -5,   0 }  /* orange scale */  
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             03/90
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnColorMap::SetupDefaultColors()
    {
    byte msDefaultCtbl[MAX_CTBLBYTES];

    /* Initialize each color scale's first element. */
    for (int i=0; i<16; i++)
        {
        msDefaultCtbl[i*3]   = s_defColorTblParams[i%8].redInit;
        msDefaultCtbl[i*3+1] = s_defColorTblParams[i%8].grnInit;
        msDefaultCtbl[i*3+2] = s_defColorTblParams[i%8].bluInit;
        }

    /* Set all successors in terms of their predecessors in scale. */
    for (int i=16; i<256; i++)
        {
        msDefaultCtbl[i*3]   = (msDefaultCtbl[(i-16)*3]   + s_defColorTblParams[i%8].redDelta) & 0xFF;
        msDefaultCtbl[i*3+1] = (msDefaultCtbl[(i-16)*3+1] + s_defColorTblParams[i%8].grnDelta) & 0xFF;
        msDefaultCtbl[i*3+2] = (msDefaultCtbl[(i-16)*3+2] + s_defColorTblParams[i%8].bluDelta) & 0xFF;
        }

    /* Background color */
    msDefaultCtbl [0]  = 0;
    msDefaultCtbl [1]  = 0;
    msDefaultCtbl [2]  = 0;

    /* dim grey */
    msDefaultCtbl [9*3]    = 64;
    msDefaultCtbl [9*3+1]  = 64;
    msDefaultCtbl [9*3+2]  = 64;

    /* grey */
    msDefaultCtbl [10*3]   = 192;
    msDefaultCtbl [10*3+1] = 192;
    msDefaultCtbl [10*3+2] = 192;

    /* pink */
    msDefaultCtbl [11*3]   = 254;
    msDefaultCtbl [11*3+1] = 0;
    msDefaultCtbl [11*3+2] = 96;

    /* greenish yellow */
    msDefaultCtbl [12*3]   = 160;
    msDefaultCtbl [12*3+1] = 224;
    msDefaultCtbl [12*3+2] = 0;

    /* greenish cyan */
    msDefaultCtbl [13*3]   = 0;
    msDefaultCtbl [13*3+1] = 254;
    msDefaultCtbl [13*3+2] = 160;

    /* dark purple */
    msDefaultCtbl [14*3]   = 128;
    msDefaultCtbl [14*3+1] = 0;
    msDefaultCtbl [14*3+2] = 160;

    /* medium grey */
    msDefaultCtbl [15*3]   = 176;
    msDefaultCtbl [15*3+1] = 176;
    msDefaultCtbl [15*3+2] = 176;

    /* Old 3.3 compatible lt.grey hilite color */
    msDefaultCtbl [255*3]   = 192;
    msDefaultCtbl [255*3+1] = 192;
    msDefaultCtbl [255*3+2] = 192;

    SetupFromRgbColors ((RgbColorDef *) msDefaultCtbl);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/07
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 const* DgnColorMap::GetTbgrColors () const {return m_colors;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/07
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnColorMap::GetRgbColors (RgbColorDef* colors) const
    {
    // NOTE: First entry is background...but in colorMap it's 255...
    for (int iColor = 0; iColor < INDEX_ColorCount; iColor++)
        {
        IntColorDef colorEntry = GetColor (iColor);
        memcpy (&colors[(INDEX_Background == iColor ? 0 : iColor+1)], &colorEntry.m_rgb, sizeof (colorEntry.m_rgb));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/09
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnColorMap::GetHsvColors (HsvColorDef* colors) const
    {
    // NOTE: First entry is background...but in colorMap it's 255...
    for (int iColor = 0; iColor < INDEX_ColorCount; iColor++)
        {
        IntColorDef     colorEntry = GetColor (iColor);

        ColorUtil::ConvertRgbToHsv (&colors[(INDEX_Background == iColor ? 0 : iColor+1)], &colorEntry.m_rgb);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/07
+---------------+---------------+---------------+---------------+---------------+------*/
IntColorDef const& DgnColorMap::GetColor (int index) const
    {
    // NOTE: Called with -1 to get background!
    if (-1 == index)
        index = DgnColorMap::INDEX_Background;
    else if (index < 0 || index > DgnColorMap::INDEX_Count-1)
        index = 0;

    return *(IntColorDef*) &m_colors[index];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/07
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnColorMap::SetupFromRgbColors (RgbColorDef const* colors)
    {
    // NOTE: First entry is background...but in colorMap it's 255...
    for (int iColor = 0; iColor < INDEX_ColorCount; iColor++)
        {
        IntColorDef     colorEntry;
        colorEntry.m_int = 0;
        colorEntry.m_rgb = colors[iColor];
        m_colors[(0 == iColor ? INDEX_Background : iColor-1)] = colorEntry.m_int;
        }

    memset (&m_colors[INDEX_ColorCount], 0, (INDEX_Count - INDEX_ColorCount) * sizeof (UInt32));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnColorMapCP DgnColorMap::Get (DgnProjectCR project)
    {
    return project.Colors().GetDgnColorMap();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnColorMap::IsSame (DgnColorMapCP otherMap, bool ignoreBG) const
    {
    int  compareCount = (ignoreBG ? INDEX_ColorCount-1 : INDEX_ColorCount);
    return (0 == memcmp (m_colors, otherMap->GetTbgrColors(), compareCount * sizeof (UInt32)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 DgnColorMap::FindClosestMatch (IntColorDef const& colorDef, HsvColorDef* hsvTable) const
    {
    HsvColorDef tmpHsvTable[INDEX_ColorCount];

    // convert color map to hsv...
    if (!hsvTable)
        {
        GetHsvColors (tmpHsvTable);
        hsvTable = tmpHsvTable;
        }

    // convert input color to hsv for compare...
    HsvColorDef colorHSV;

    ColorUtil::ConvertRgbToHsv (&colorHSV, &colorDef.m_rgb);

    // ignore bg color for search so skip index 0 in hsv table...
    HsvColorDef*    startHsvP = hsvTable + 1;
    HsvColorDef*    endHsvP   = startHsvP + (INDEX_ColorCount-1);

    // search hsv table looking for closest match...
    int             colorMapIndex = 0;
    long            mindist = LMAXI4, dist, dh, ds, dv;
    int             darkestGrayValue = 255;
    int             dkGreyCmapIndex = -1;

    for (HsvColorDef* nextHsvP = startHsvP; nextHsvP < endHsvP; nextHsvP++)
        {
        if (colorHSV.saturation > 2)
            {
            dh = nextHsvP->hue - colorHSV.hue;

            // Find minimum arc length along color wheel between hues.
            if (dh > 180)
                dh = (colorHSV.hue + 360) - nextHsvP->hue;
            else if (dh < -180)
                dh = (nextHsvP->hue + 360) - colorHSV.hue;
            }
        else
            {
            dh = 0;
            }

        ds = nextHsvP->saturation - colorHSV.saturation;
        dv = nextHsvP->value - colorHSV.value;

        dist = ((dh*dh) << 1) + ds*ds + dv*dv;

        if (dist < mindist)
            {
            colorMapIndex = static_cast<int>(nextHsvP - startHsvP);
            mindist = dist;

            if (mindist == 0)
                break;
            }

        // Save darkest gray just in case...
        if ((nextHsvP->saturation == 0) &&  // This indicates shade of gray.
            (nextHsvP->value != 0) &&       // Ignore black.
            (nextHsvP->value < darkestGrayValue))
            {
            darkestGrayValue = nextHsvP->value;
            dkGreyCmapIndex  = static_cast<int>(nextHsvP - startHsvP);
            }
        }

    // If non-black color matched to black, change to darkest grey found.
    HsvColorDef*    indexHsvP = &startHsvP[colorMapIndex];

    if ((indexHsvP->value == 0) && (colorDef.m_rgb.red || colorDef.m_rgb.green || colorDef.m_rgb.blue) && (dkGreyCmapIndex != -1))
        colorMapIndex = dkGreyCmapIndex;

    return colorMapIndex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/09
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnColorMap::AdjustValueAndSaturation (double valueAdjustment, double saturationAdjustment, bool valueAndSaturationFixed, double hueValue, bool hueFixed)
    {
    RgbColorDef localColors[INDEX_ColorCount];

    GetRgbColors (localColors);
    
    // NOTE: First entry is background...but in colorMap it's 255...
    ColorUtil::AdjustValueAndSaturation (localColors+1, INDEX_ColorCount-1, valueAdjustment, saturationAdjustment, valueAndSaturationFixed, hueValue, hueFixed);

    for (int iColor = 1; iColor < INDEX_ColorCount-1; iColor++)
        {
        IntColorDef     colorEntry;

        colorEntry.m_int = 0;
        colorEntry.m_rgb = localColors[iColor];

        m_colors[iColor-1] = colorEntry.m_int;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTrueColorId DgnColorMap::GetNextTrueColorId(DgnProjectR project) const
    {
    if (!m_nextTrueColorID.IsValid())
        {
        HighPriorityOperationBlock highPriorityOperationBlock;
        if (BE_SQLITE_OK != project.GetNextServerIssuedId (m_nextTrueColorID, DGN_TABLE_Color, "Id"))
            BeAssert (false);
        }

    return m_nextTrueColorID;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTrueColorId DgnColorMap::UseNextTrueColorId(DgnProjectR project) const
    {
    DgnTrueColorId curr = GetNextTrueColorId(project);
    m_nextTrueColorID = DgnTrueColorId(curr.GetValue() + 1);
    return  curr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnColorMap::CanBeTrueColor(UInt32 elementColor)
    {
    UInt32  idx = (elementColor >> 8) & MAX_ExtendedColorIndex;
    if (0 == idx)
        return  false;

    // Ignore special color values.
    switch (elementColor)
        {
        case INVALID_COLOR:
        case COLOR_BYCELL:
        case COLOR_BYLEVEL:
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnColorMap::IsTrueColor (DgnProjectR project, IntColorDef& colorDef, UInt32 elementColor) const
    {
    if (!CanBeTrueColor(elementColor))
        return false;

    UInt32  idx = (elementColor >> 8) & MAX_ExtendedColorIndex;
    // Detect invalid IDs
    if (idx >= GetNextTrueColorId(project).GetValue())
        return false;

    // Try the cache first before blocking for database access.
    TrueColorMap::const_iterator iter = m_trueColorCache.find (idx);
    if (m_trueColorCache.end() != iter)
        {
        colorDef = iter->second;
        return true;
        }

    // Query for the value and cache it. 
    // This block has no effect unless there is a range tree query occurring during update dynamics. See comments on HighPriorityOperationBlock for more information.
    HighPriorityOperationBlock highPriorityOperationBlock;
    if (SUCCESS != project.Colors().QueryTrueColorInfo (&colorDef.m_rgb, NULL, NULL, DgnTrueColorId(idx)))
        {
        // We should have caught invalid IDs above; if we got here, something is corrupt within the color table.
        BeAssert(false);
        return false;
        }

    m_trueColorCache[idx] = colorDef;
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnColors::IsTrueColor (IntColorDef& colorDef, UInt32 elementColor) const 
    {
    return GetDgnColorMap()->IsTrueColor(m_project, colorDef, elementColor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnColors::Extract (IntColorDef* outColorDef, UInt32* outColorIndex, bool* outIsTrueColor, Utf8StringP bookName, Utf8StringP colorName, UInt32 elementColor) const
    {
    IntColorDef ALLOW_NULL_OUTPUT (colorDef, outColorDef);
    UInt32      ALLOW_NULL_OUTPUT (colorIndex, outColorIndex);
    bool        ALLOW_NULL_OUTPUT (isTrueColor, outIsTrueColor);

    colorDef.m_int = 0;
    colorIndex = INVALID_COLOR;
    isTrueColor = IsTrueColor (colorDef, elementColor);

    if (isTrueColor)
        {
        if (NULL != bookName || NULL != colorName)
            QueryTrueColorInfo (NULL, colorName, bookName, DgnTrueColorId((elementColor >> 8) & MAX_ExtendedColorIndex));

        return  SUCCESS;
        }

    if (NULL != bookName)
        bookName->clear();

    if (NULL != colorName)
        colorName->clear();

    switch (elementColor)
        {
        case INVALID_COLOR:
        case COLOR_BYCELL:
        case COLOR_BYLEVEL:
            return ERROR; // Ignore special color values...
        }

    if (0 != (elementColor >> 8)) // all upper bytes must be zero or something is wrong
        return  ERROR;

    colorIndex = (elementColor & 0xff);
    colorDef = GetDgnColorMap()->GetColor (colorIndex);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 DgnColors::GetElementColor (IntColorDef const& colorDef, Utf8CP bookName, Utf8CP colorName, bool createIfNotPresent)
    {
    DgnTrueColorId extIndex = QueryTrueColorId(colorDef.m_rgb); 
    if (!extIndex.IsValid())
        {
        if (!createIfNotPresent)
            return INVALID_COLOR;

        extIndex = InsertTrueColor (colorDef.m_rgb, colorName, bookName);
        }

    // NOTE: In V8, whenever we store a truecolor id, we also stored the id of the closest match in the color table in the low 8 bytes.
    // In Graphite we're not going to do that any more. If the color is meant to be a TRUE color, no point in giving people a way of
    // misinterpreting it as some random (hopefully, but not necessarily) "close" rgb value from the color map.
    return extIndex.IsValid() ? (extIndex.GetValue() << 8) : INVALID_COLOR;
    }
