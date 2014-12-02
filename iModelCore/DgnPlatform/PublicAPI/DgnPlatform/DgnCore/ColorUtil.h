/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ColorUtil.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "../DgnPlatform.h"

BENTLEY_API_TYPEDEFS (RgbColorDef)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

enum ColorDefConstants
{
    MAX_COLOR_NAME_SIZE     = 30,
    MAX_CMAPENTRIES         = 256,
    MAX_CTBLBYTES           = (MAX_CMAPENTRIES * 3),
    MAX_CTBLNAMESIZE        = 64,
    MAX_ExtendedColorIndex  = 0x0fffff,  // Use 20 bits of high 24.

    // Menu Color Indexes
    BLACK_MENU_COLOR_INDEX      = 0,
    BLUE_MENU_COLOR_INDEX       = 1,
    GREEN_MENU_COLOR_INDEX      = 2,
    CYAN_MENU_COLOR_INDEX       = 3,
    RED_MENU_COLOR_INDEX        = 4,
    MAGENTA_MENU_COLOR_INDEX    = 5,
    YELLOW_MENU_COLOR_INDEX     = 6,
    WHITE_MENU_COLOR_INDEX      = 7,
    LGREY_MENU_COLOR_INDEX      = 8,
    DGREY_MENU_COLOR_INDEX      = 9,
    MGREY_MENU_COLOR_INDEX      = 10,

    NUM_MENU_COLORS         = 12,  // number of menu colors
    NUM_NEW_FIXED_COLORS    = 6,
    NEW_NUM_MENU_COLORS     = NUM_MENU_COLORS+NUM_NEW_FIXED_COLORS,

    // Method Ids for setting and querying color descriptors
    COLORD_RGB                   = 1,   // S/Q
    COLORD_HSV                   = 2,   // S/Q
    COLORD_XCOLORID              = 3,   // S/Q
    COLORD_XNAMEID               = COLORD_XCOLORID,
    COLORD_XCOLORSTRING          = 4,   // S/Q
    COLORD_XNAMESTR              = COLORD_XCOLORSTRING,
    COLORD_ELEM_COLOR_NUMBER     = 5,   // S/Q
    COLORD_MENUCOLORID           = 6,   // S/Q
    COLORD_ATTRDATA              = 7,   // BSI-only
    COLORD_RGBDITHERED           = 8,   // BSI-only
    COLORD_VPAL_INDEX            = 9,   // Q-only
    COLORD_ELEM_COLOR_REF        = 10,  // S-only
    COLORD_DRAW_VALUE            = 20,  // S/Q
    COLORD_DRAW_INDEX            = COLORD_DRAW_VALUE,

    // Bit flags for "asetFlags" member of palette entry.
    PALFLAG_MATCHLOADEDCOLORS    = 0x0,
    PALFLAG_MATCHDGNCTBL         = 0x1,

    // Palette Status flags
    PALSTATUS_SYNCHABLE          = 0x01,

    // Color matching flags
    COLOR_STRESS_DEFAULT         = 0,
    COLOR_STRESS_HUE             = 0x1,
    COLOR_STRESS_SATURATION      = 0x2,
    COLOR_STRESS_VALUE           = 0x4,

    // Palette Synch flags
    SYNCH_DUE_TO_LOAD_COLORS     = 1,
    SYNCH_DUE_TO_CMAP_CHG        = 2,
};

//__PUBLISH_SECTION_END__

// Fixed AutoCAD color palette name
#define     COLOR_PALETTENAME_AUTOCAD   "AcadPalette"

typedef struct  bsipaletteentry
{
    UInt32  setMethod;      // How to set the entry.
    double  tolerance;      // Reserved for future use.
    union
    {
        RgbColorDef         rgb;
        HsvColorDef         hsv;
        UInt32              menuColorId;
        UInt32              elemColorNumber;
        struct setdrawinfo
        {
            UInt32          drawValue;
            int             screen;
        } setDrawInfo;
        struct  seteleminfo
        {
            UInt32          colorNumber;
            DgnModelP    modelRef;
        } setElemInfo;
        UInt32              xColorId;
    } setData;              // Data used to set the palette entry.
    UInt32  setFlags;
} BSIPaletteEntry;

typedef struct bsicolorsettings
{
    int             maxNumColors;
    int             numBalancedColors;
    int             numSystemColors;
    int             numLoadedColors;
    int             numReservedColors;
} BSIColorSettings;

#define ICO_DEFAULT     0
#define ICO_GRAYSCALE   1
#define ICO_CUSTOM      2

typedef struct iconcoloroverrides
{
    byte            colorScheme;
    byte            colorizeActiveOnly;
    byte            values[NEW_NUM_MENU_COLORS];
} IconColorOverrides;

typedef struct colorpalette BSIColorPalette;

//__PUBLISH_SECTION_START__

//=======================================================================================
//! Union that allows a color to be accessed as a UInt32 (in form TBGR), RgbColorDef, or RgbaColorDef.
//! @bsiclass
//=======================================================================================
union IntColorDef
{
    RgbColorDef     m_rgb;
    RgbaColorDef    m_rgba;
    UInt32          m_int;

    void SetColors (Byte r, Byte g, Byte b, Byte a) {m_rgba.red = r; m_rgba.green = g; m_rgba.blue = b; m_rgba.alpha = a;}
    void ClearAlpha() {m_rgba.alpha=0;}
    UInt32 AsUInt32() const {return m_int;}
    operator UInt32() const {return m_int;}

    IntColorDef () {m_int = 0;}
    IntColorDef (RgbColorDef rgb) {m_int = 0; m_rgb = rgb;}
    IntColorDef (UInt32 intval) {m_int=intval;}
    IntColorDef (Byte red, Byte green, Byte blue) {SetColors (red,green,blue,0);}
    IntColorDef (Byte red, Byte green, Byte blue, Byte alpha) {SetColors (red,green,blue,alpha);}
};

//__PUBLISH_SECTION_END__

//=======================================================================================
//! @bsiclass                                                     Brien.Bastings  05/09
//=======================================================================================
struct ColorUtil
{
DGNPLATFORM_EXPORT static UInt32 GetV8ElementColor (UInt32 elementColor);
DGNPLATFORM_EXPORT static bool ElementColorsAreEqual (UInt32 color1, DgnModelP modelRef1, UInt32 color2, DgnModelP modelRef2, bool keepRefColorIndexOnCopy);
DGNPLATFORM_EXPORT static void AdjustValueAndSaturation (RgbColorDef* colorArray, UInt32 numColors, double valueAdjustment, double saturationAdjustment, bool valueAndSaturationFixed, double hueValue, bool hueFixed);
DGNPLATFORM_EXPORT static void ConvertRgbToHsv (HsvColorDef* hsv, RgbColorDef const* rgb);
DGNPLATFORM_EXPORT static void ConvertHsvToRgb (RgbColorDef* rgb, HsvColorDef const* hsv);
DGNPLATFORM_EXPORT static void ConvertIntToRGBFactor (RgbFactor &rgbFactor, UInt32 intColor);
DGNPLATFORM_EXPORT static void ConvertRGBFactorToInt (UInt32 &intColor, RgbFactor &rgbFactor);
DGNPLATFORM_EXPORT static void ConvertIntToFloatRgb (FPoint3dR rgb, UInt32 intColor);
DGNPLATFORM_EXPORT static void ConvertFloatRgbToInt (UInt32 &intColor, FPoint3dR rgb);
DGNPLATFORM_EXPORT static void ConvertRgbFactorToRgbColorDef (RgbColorDefR colorDef, RgbFactorCR rgbFactor);
DGNPLATFORM_EXPORT static void ConvertRgbColorDefToRgbFactor (RgbFactorR rgbFactpr, RgbColorDefCR colorDef);
DGNPLATFORM_EXPORT static UInt32 GetExtendedIndexFromRawColor (UInt32 rawColor);
DGNPLATFORM_EXPORT static void InterpolateColorsRGB (RgbColorDefP interpolatedColors, size_t nInterpolatedColors, RgbColorDefCR startColor, RgbColorDefCR endColor);
};

//=======================================================================================
//! @bsiclass                                                     BSI      02/2009
//=======================================================================================
struct ColorMapping
{
    DgnModelP    srcDgnModel;                    // file srcTable is for
    DgnModelP    destDgnModel;                   // file destTable is for
    UInt32          remapTable[MAX_CMAPENTRIES];    // Initialized to background color 0xff
    HsvColorDef     destHSVTable[MAX_CMAPENTRIES];
    bool            differentTables;

    DGNPLATFORM_EXPORT static BentleyStatus GetChangeColor (UInt32& remappedIndex, UInt32 colorIndex, ColorMapping **colorMapPP, DgnModelP destDgnModel, DgnModelP srcDgnModel, bool keepRefColorIndexOnCopy);
    DGNPLATFORM_EXPORT static UInt32 RemapColor (UInt32 sourceColorIndex, DgnModelP dest, DgnModelP source, bool keepRefColorIndexOnCopy);
};

//__PUBLISH_SECTION_START__

END_BENTLEY_DGNPLATFORM_NAMESPACE
