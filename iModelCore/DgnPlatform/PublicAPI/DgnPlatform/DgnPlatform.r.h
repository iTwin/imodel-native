/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnPlatform.r.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnPlatformBaseType.r.h"
#include <BeJsonCpp/BeJsonUtilities.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/** @cond BENTLEY_SDK_Internal */
//! Identifies the type or purpose of a model
enum class DgnModelType : uint16_t
    {
    Physical        = 0,   //!< a physical model.
    Sheet           = 1,   //!< a sheet model.
    Drawing         = 3,   //!< a 2d drawing model.
    Component       = 4,   //!< a 3d component model.
    Query           = 5,   //!< a query model
    Illegal         = 999, //!< @private
    };

/** @endcond */

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/14
//=======================================================================================
enum class StandardView
    {
    NotStandard = -1,
    Top         = 1,
    Bottom      = 2,
    Left        = 3,
    Right       = 4,
    Front       = 5,
    Back        = 6,
    Iso         = 7,
    RightIso    = 8,
    };

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/14
//=======================================================================================
enum class DgnRenderMode : uint8_t
    {
    Wireframe           = 0,
    CrossSection        = 1,
    Wiremesh            = 2,
    HiddenLine          = 3,
    SolidFill           = 4,
    ConstantShade       = 5,
    SmoothShade         = 6,
    Phong               = 7,
    RayTrace            = 8,
    RenderWireframe     = 9,
    Radiosity           = 10,
    ParticleTrace       = 11,
    RenderLuxology      = 12,
    Invalid             = 15,
    };

/*=================================================================================**//**
*  The flags that control view information.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ViewFlags
    {
private:
    DgnRenderMode m_renderMode;

public:
    uint32_t    constructions:1;    //!< Shows or hides construction class geometry.
    uint32_t    text:1;             //!< Shows or hides text.
    uint32_t    dimensions:1;       //!< Shows or hides dimensions.
    uint32_t    patterns:1;         //!< Shows or hides pattern geometry.
    uint32_t    weights:1;          //!< Controls whether non-zero line weights are used or display using weight 0.
    uint32_t    styles:1;           //!< Controls whether custom line styles are used (e.g. control whether elements with custom line styles draw normally, or as solid lines).
    uint32_t    transparency:1;     //!< Controls whether element transparency is used (e.g. control whether elements with transparency draw normally, or as opaque).
    uint32_t    fill:1;             //!< Controls whether the fills on filled elements are displayed.
    uint32_t    grid:1;             //!< Shows or hides the grid. The grid settings are a design file setting.
    uint32_t    acs:1;              //!< Shows or hides the ACS triad.
    uint32_t    bgImage:1;          //!< Shows or hides the background image. The image is a design file setting, and may be undefined.
    uint32_t    bgColor:1;          //!< Controls whether the view's custom background color is used. This is typically controlled through a display style.

    uint32_t    textures:1;         //!< Controls whether to display texture maps for material assignments. When off only material color is used for display.
    uint32_t    materials:1;        //!< Controls whether materials are used (e.g. control whether geometry with materials draw normally, or as if it has no material).
    uint32_t    sceneLights:1;      //!< Controls whether the custom scene lights or the default lighting scheme are used. Note the inversion.
    uint32_t    visibleEdges:1;     //!< Shows or hides visible edges in the shaded render mode. This is typically controlled through a display style.
    uint32_t    hiddenEdges:1;      //!< Shows or hides hidden edges in the shaded render mode. This is typically controlled through a display style.
    uint32_t    shadows:1;          //!< Shows or hides shadows. This is typically controlled through a display style.
    uint32_t    noFrontClip:1;      //!< Controls whether the front clipping plane is used. Note the inversion. Elements beyond will not be displayed.
    uint32_t    noBackClip:1;       //!< Controls whether the back clipping plane is used. Note the inversion. Elements beyond will not be displayed.
    uint32_t    noClipVolume:1;     //!< Controls whether the clip volume is applied. Note the inversion. Elements beyond will not be displayed.

    void SetRenderMode (DgnRenderMode value) {m_renderMode = value;}
    DgnRenderMode GetRenderMode() const {return m_renderMode;}

    DGNPLATFORM_EXPORT void InitDefaults();
    DGNPLATFORM_EXPORT void ToBaseJson(JsonValueR) const;
    DGNPLATFORM_EXPORT void FromBaseJson(JsonValueCR);
    DGNPLATFORM_EXPORT void To3dJson(JsonValueR) const;
    DGNPLATFORM_EXPORT void From3dJson(JsonValueCR);
    };

enum class GradientMode
    {
    None                = 0,
    Linear              = 1,
    Curved              = 2,
    Cylindrical         = 3,
    Spherical           = 4,
    Hemispherical       = 5,
    };

enum class AngleFormatVals
    {
    None        = -2,
    Active      = -1,
    Degrees     = 0,
    DegMinSec   = 1,
    Centesimal  = 2,
    Radians     = 3,
    DegMin      = 4,
    Surveyor    = 5,      //  DWG mode only
    };

struct DirFormat
    {
    uint16_t        mode;
    uint16_t        unused[2];
    struct
        {
        uint16_t clockwise:1;
        uint16_t unused:15;
        } flags;
    double          baseDir;
    };

struct Autodim1
    {
    struct
        {
        uint16_t  adres2:8;
        uint16_t  ref_mastersub:2;
        uint16_t  ref_decfract:1;
        uint16_t  accuracyFlags:2;  /* 0- use ref_decfract; 1- scientific accuracy; 2- fractional zero */
        uint16_t  reserved:3;
        } format;
    };

/** @cond BENTLEY_SDK_Internal */
struct DegreeOfFreedom
    {
    int32_t             locked;
    T_Adouble           value;
    };

/** @endcond */

/*=================================================================================**//**
*  The unit information
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct UnitFlags
    {
    /** Flags for the unit base. */
    uint32_t        base:3;                     /* UNIT_BASE_xxx */
    /** Flags for the unit system. */
    uint32_t        system:3;                   /* UNIT_SYSTEM_xxx */
    /** Flags for future use. */
    uint32_t        reserved:26;
    };

/** @cond BENTLEY_SDK_Internal */

enum class SelectionMode
    {
    New         = 0,
    Add         = 1,
    Subtract    = 2,
    Inverse     = 3,
    Clear       = 4,
    All         = 5,
    };

enum class LocateSurfacesPref
    {
    Never           = 0, //!< Don't locate interiors of regions, surfaces, and solids even if filled or rendered.
    ByView          = 1, //!< Locate interiors according to view attributes for fill display and render mode. (Default)
    Always          = 2, //!< Locate interiors of regions, surfaces, and solids even in wireframe and even with fill display off.
    };

/** @endcond */

enum class UnitBase
    {
    None            = 0,
    Meter           = 1,
    Degree          = 2,
    };

/*=================================================================================**//**
A unit is represented by a scale factor that describes its size relative to its
base. Only units with identical bases can be compared.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class StandardUnit
    {
    /* English */
    None                        = 0,

    EnglishFirst                = 1000,
    EnglishMiles                = 1050,
    EnglishYards                = 1075,
    EnglishFeet                 = 1100,
    EnglishInches               = 1125,
    EnglishPicas                = 1130,
    EnglishPoints               = 1135,
    EnglishMils                 = 1150,
    EnglishMicroInches          = 1175,
    EnglishLast                 = 1225,

    /* US Survey-foot based */
    EnglishSurveyMiles          = 1049,
    EnglishFurlongs             = 1055,
    EnglishChains               = 1060,
    EnglishRods                 = 1065,
    EnglishFathoms              = 1070,
    EnglishSurveyFeet           = 1099,
    EnglishSurveyInches         = 1124,

    /* Metric */
    MetricFirst                 = 2000,
    MetricPetameters            = 2010,
    MetricTerameters            = 2020,
    MetricGigameters            = 2030,
    MetricMegameters            = 2040,
    MetricKilometers            = 2050,
    MetricHectometers           = 2060,
    MetricDekameters            = 2070,
    MetricMeters                = 2075,
    MetricDecimeters            = 2090,
    MetricCentimeters           = 2100,
    MetricMillimeters           = 2125,
    MetricMicrometers           = 2150,
    MetricNanometers            = 2160,
    MetricPicometers            = 2180,
    MetricFemtometers           = 2190,
    MetricLast                  = 2225,

    /* No System */
    NoSystemFirst               = 3000,
    NoSystemParsecs             = 3100,
    NoSystemLightYears          = 3200,
    NoSystemAstronomicalUnits   = 3300,
    NoSystemNauticalMiles       = 3500,
    NoSystemAngstroms           = 3800,
    NoSystemLast                = 3900,

    /* No Base */
    UnitlessWhole               = 4000,

    /* Angle */
    AngleRadians                = 4500,
    AngleDegrees                = 4550,
    AngleGrads                  = 4575,
    AngleMinutes                = 4600,
    AngleSeconds                = 4650,

    Custom                      = 5000,
    };

//=======================================================================================
//! Used by AngleFormatter to specify the mode for angle formatting.  Explanation of terms:
//!    - Degree:     A full circle has 360 degrees.
//!    - Minute:     A degree has 60 minutes.
//!    - Second:     A minute has 60 seconds.
//!    - Centesimal: A full circle has 400 gradians.
//!    - Radian:     A full circle has 2*pi radians.
//! @bsiclass
//=======================================================================================
enum class AngleMode
    {
    Invalid                         = 0,        //!< Uninitialized value. Do not use.
    Degrees                         = 1,        //!< Format as decimal degrees. Ex. 30.5^ where ^ is the unicode degree character.
    DegMin                          = 2,        //!< Format as degrees with minutes. Ex. 30^30' where ^ is the unicode degree character.
    DegMinSec                       = 3,        //!< Format as degrees with minutes and seconds. Ex. 30^30'00" where ^ is the unicode degree character.
    Centesimal                      = 4,        //!< Format as centesimal, also known as gradians. Ex. 50.0g
    Radians                         = 5,        //!< Format as radians. Ex. 1.57r
    };

//=======================================================================================
//! Used by AngleFormatter to specify the maximum number of decimals for angle formatting.
//! @bsiclass
//=======================================================================================
enum class AnglePrecision
    {
    Whole                           = 0,    //!< Ex. 30^
    Use1Place                       = 1,    //!< Ex. 30.1^
    Use2Places                      = 2,    //!< Ex. 30.12^
    Use3Places                      = 3,    //!< Ex. 30.123^
    Use4Places                      = 4,    //!< Ex. 30.1234^
    Use5Places                      = 5,    //!< Ex. 30.12345^
    Use6Places                      = 6,    //!< Ex. 30.123456^
    Use7Places                      = 7,    //!< Ex. 30.1234567^
    Use8Places                      = 8,    //!< Ex. 30.12345678^
    };

enum class DgnUnitFormat
    {
    MUSU                            = 0,    //!< Master Units / SubUnits
    MU                              = 1,    //!< Master Units
    SU                              = 3,    //!< SubUnits
    };

//=======================================================================================
//! Used by DirectionFormatter to specify the mode for direction formatting.
//!     - In Azimuth mode, a direction is formatted as an angle measured from a
//!                        specified base direction at a specified orientation.
//!     - In Bearing mode, a direction is formatted as an angle measure from either
//!                        North or South and oriented to either East or West.
//! @bsiclass
//=======================================================================================
enum class DirectionMode : uint16_t
    {
    Invalid                         = 0,    //!< Uninitialized value. Do not use.
    Azimuth                         = 1,    //!< Ex: 30^
    Bearing                         = 2,    //!< Ex: N60^E
    };

//=======================================================================================
//! Used by DateTimeFormatter to specify the sequence in which various elements of
//! the date and time should appear in the formatted output.
//=======================================================================================
enum DateTimeFormatPart
    {
    //==================================
    //! Atomic elements
    //==================================
    //! Day of week
    DATETIME_PART_DayOfWeek,                //! EX: "Tuesday"
    DATETIME_PART_DoW,                      //! EX: "Tue"

    //! Day of month
    DATETIME_PART_D,                        //! EX: "5"
    DATETIME_PART_DD,                       //! EX: "05"

    //! Month
    DATETIME_PART_Month,                    //! EX: "August"
    DATETIME_PART_Mon,                      //! EX: "Aug"
    DATETIME_PART_M,                        //! EX: "8"
    DATETIME_PART_MM,                       //! EX: "08"

    //! Day of year
    DATETIME_PART_d,                        //! EX: "22" (twenty-second day of year, January 22)
    DATETIME_PART_ddd,                      //! EX: "022"

    //! Year
    DATETIME_PART_YYYY,                     //! EX: "2012", "1999"
    DATETIME_PART_YY,                       //! EX: "12", "99", "08"

    //! Hour
    DATETIME_PART_h,                        //! Using 12-hour clock. EX: "8"
    DATETIME_PART_hh,                       //! Using 12-hour clock. EX: "08"
    DATETIME_PART_H,                        //! Using 24-hour clock. EX: "0", "23"
    DATETIME_PART_HH,                       //! Using 24-hour clock. EX: "00", "23"

    //! Minute
    DATETIME_PART_m,                        //! EX: "8"
    DATETIME_PART_mm,                       //! EX: "08"

    //! Second
    DATETIME_PART_s,                        //! EX: "8"
    DATETIME_PART_ss,                       //! EX: "08"

    //! Fractional seconds, precision specified in formatter. Always includes leading zeros
    DATETIME_PART_FractionalSeconds,        //! EX: "00123" (precision 5)

    //! Separators
    DATETIME_PART_Comma,                    //! EX: The comma in "Tuesday, July 4"
    DATETIME_PART_DateSeparator,            //! EX: The slashes in "4/5/2012"
    DATETIME_PART_TimeSeparator,            //! EX: The colons in "4:05 PM"
    DATETIME_PART_DecimalSeparator,         //! EX: The period in "1.234"
    DATETIME_PART_Space,                    //! A space character

    //! Misc
    DATETIME_PART_AMPM,                     //! EX: "AM", "PM"
    DATETIME_PART_AP,                       //! EX: "A", "P"

//__PUBLISH_SECTION_END__
    DATETIME_PART_Y,                        // 0-99, min width 1
    DATETIME_PART_YYY,                      // min width 3
    DATETIME_PART_YYYYY,                    // min width 5
//__PUBLISH_SECTION_START__

    //! UTC offset
    DATETIME_PART_U,                        //! EX: "-7", "+11"
    DATETIME_PART_UU,                       //! EX: "-07", "+11"
    DATETIME_PART_U_UU,                     //! EX: "-7:00", "+11:00"
    DATETIME_PART_UU_UU,                    //! EX: "-07:00", "+11:00"

    DATETIME_PART_UTC,                      //! "UTC"

    //==================================
    //! Composite parts
    //==================================
    DATETIME_PART_h_mm_AMPM = 100,          //! EX: "1:45 PM"
    DATETIME_PART_h_mm_ss_AMPM,             //! EX: "1:45:30 PM"

    DATETIME_PART_M_D_YYYY,                 //! EX: "4/5/2012"
    DATETIME_PART_MM_DD_YYYY,               //! EX: "04/05/2012"
    DATETIME_PART_Day_D_Month_YYYY,         //! EX: "Saturday, 5 April, 2012"
    DATETIME_PART_Day_Month_D_YYYY,         //! EX: "Saturday, April 5, 2012"

    DATETIME_PART_Full,                     //! EX: "Monday, June 15, 2009 1:45 PM"
    DATETIME_PART_General,                  //! EX: "6/15/2009 1:45:30 PM"

//__PUBLISH_SECTION_END__
    DATETIME_PART_END,
    DATETIME_PART_COMPOSITE_BASE    = DATETIME_PART_h_mm_AMPM,
    DATETIME_PART_COMPOSITE_END     = DATETIME_PART_END,
//__PUBLISH_SECTION_START__
    };

/** @cond BENTLEY_SDK_Internal */
enum class DwgUnitFormat
    {
    Scientific                   = 1,
    Decimal                      = 2,
    Engineering                  = 3,
    Architectural                = 4,
    Fractional                   = 5,
    };

enum ResourceTextStyleProperty
{
    DGNPLATFORM_RESOURCE_TextStyle_LineSpacing           = 30,
    DGNPLATFORM_RESOURCE_TextStyle_InterCharSpacing      = 33,
};

enum class DimensionType
    {
    None                    = 0,
    SizeArrow               = 1,
    SizeStroke              = 2,
    LocateSingle            = 3,
    LocateStacked           = 4,
    AngleSize               = 5,
    ArcSize                 = 6,
    AngleLocation           = 7,
    ArcLocation             = 8,
    AngleLines              = 9,
    AngleAxis               = 10,
    Radius                  = 11,
    Diameter                = 12,
    DiameterParallel        = 13,
    DiameterPerpendicular   = 14,
    CustomLinear            = 15,
    Ordinate                = 16,
    RadiusExtended          = 17,
    DiameterExtended        = 18,
    Center                  = 19,

    AngleAxisX              = 50,
    AngleAxisY              = 51,
    LabelLine               = 52,
    Note                    = 53,

    MaxThatHasTemplate      = 19,
    Max                     = 53,   // Update if more DimensionTypes are added.
    };

enum LineStyleProp
    {
    LINESTYLE_PROP_Invalid                                      = 0,
    LINESTYLE_PROP_Stroke                                       = 101,
    LINESTYLE_PROP_Component                                    = 102,
    LINESTYLE_PROP_LineCode                                     = 103,
    LINESTYLE_PROP_PointSymbol                                  = 104,
    LINESTYLE_PROP_LinePoint                                    = 105,
    LINESTYLE_PROP_Compound                                     = 106,
    LINESTYLE_PROP_Symbol                                       = 107,
    LINESTYLE_PROP_Type_WCHAR                                   = 150,
    LINESTYLE_PROP_LineCode_Description_WCHAR                   = 200,
    LINESTYLE_PROP_LineCode_PhaseMode_INTEGER                   = 201,
    LINESTYLE_PROP_LineCode_PhaseDist_DOUBLE                    = 202,
    LINESTYLE_PROP_LineCode_SegmentMode_BOOLINT                 = 203,
    LINESTYLE_PROP_LineCode_NumIterations_INTEGER               = 204,
    LINESTYLE_PROP_LineCode_NumStrokes_INTEGER                  = 205,
    LINESTYLE_PROP_Stroke_Length_DOUBLE                         = 250,
    LINESTYLE_PROP_Stroke_StartWidth_DOUBLE                     = 251,
    LINESTYLE_PROP_Stroke_EndWidth_DOUBLE                       = 252,
    LINESTYLE_PROP_Stroke_Type_BOOLINT                          = 253,     // strokeMode & 0x1
    LINESTYLE_PROP_Stroke_Corner_BOOLINT                        = 254,     // strokeMode & 0x2
    LINESTYLE_PROP_Stroke_Fixed_BOOLINT                         = 255,     // strokeMode & 0x4
    LINESTYLE_PROP_Stroke_StartInvert_BOOLINT                   = 256,     // strokeMode & 0x8
    LINESTYLE_PROP_Stroke_EndInvert_BOOLINT                     = 257,     // strokeMode & 0x10
    LINESTYLE_PROP_Stroke_WidthMode_INTEGER                     = 258,
    LINESTYLE_PROP_Stroke_DashCaps_INTEGER                      = 259,
    LINESTYLE_PROP_Compound_Description_WCHAR                   = 300,
    LINESTYLE_PROP_Compound_NumComponents_INTEGER               = 301,
    LINESTYLE_PROP_CompoundComp_Offset_DOUBLE                   = 351,
    LINESTYLE_PROP_LinePoint_Description_WCHAR                  = 400,
    LINESTYLE_PROP_LinePoint_NumSymbols_INTEGER                 = 401,
    LINESTYLE_PROP_PointSym_StrokeNum_INTEGER                   = 450,
    LINESTYLE_PROP_PointSym_Location_INTEGER                    = 451,
    LINESTYLE_PROP_PointSym_ColorFromSymbol_BOOLINT             = 452,
    LINESTYLE_PROP_PointSym_WeightFromSymbol_BOOLINT            = 453,
    LINESTYLE_PROP_PointSym_Partial_INTEGER                     = 454,
    LINESTYLE_PROP_PointSym_Clip_BOOLINT                        = 455,
    LINESTYLE_PROP_PointSym_AllowStretch_BOOLINT                = 456,
    LINESTYLE_PROP_PointSym_Justification_INTEGER               = 457,
    LINESTYLE_PROP_PointSym_OffsetX_DOUBLE                      = 458,
    LINESTYLE_PROP_PointSym_OffsetY_DOUBLE                      = 459,
    LINESTYLE_PROP_PointSym_OffsetZ_DOUBLE                      = 460,
    LINESTYLE_PROP_PointSym_Rotation_INTEGER                    = 461,
    LINESTYLE_PROP_PointSym_AngleX_DOUBLE                       = 462,
    LINESTYLE_PROP_PointSym_AngleY_DOUBLE                       = 463,
    LINESTYLE_PROP_PointSym_AngleZ_DOUBLE                       = 464,
    LINESTYLE_PROP_PointSym_ScaleX_DOUBLE                       = 465,
    LINESTYLE_PROP_PointSym_ScaleY_DOUBLE                       = 466,
    LINESTYLE_PROP_PointSym_ScaleZ_DOUBLE                       = 467,
    LINESTYLE_PROP_PointSymCell_Name_WCHAR                      = 500,
    };

enum LineStyleProp_Type
    {
    LSTYLE_PROPTYPE_None        =  0,
    LSTYLE_PROPTYPE_BoolInt     =  1,
    LSTYLE_PROPTYPE_Distance    =  2,
    LSTYLE_PROPTYPE_Double      =  3,
    LSTYLE_PROPTYPE_Integer     =  4,
    LSTYLE_PROPTYPE_MSWChar     =  5,
    };

/* Values for "options" member         */
enum LineCodeOptionFlags
    {
    LCOPT_NONE               = 0x00000000,
    LCOPT_AUTOPHASE          = 0x00000001,
    LCOPT_RES1               = 0x00000002,
    LCOPT_RES2               = 0x00000004,
    LCOPT_ITERATION          = 0x00000008,   /* Uses iteration limit   */
    LCOPT_SEGMENT            = 0x00000010,   /* Single segment mode    */
    LCOPT_CENTERSTRETCH      = 0x00000020,   /* Center the line style and stretch the ends - ACAD style */
    };

/* Values for strokeMode               */
enum LineCodeStrokeFlags
    {
    LCSTROKE_DASH      = 0x01,
    LCSTROKE_GAP       = 0x00,
    LCSTROKE_RAY       = 0x02,
    LCSTROKE_SCALE     = 0x04,    /* Stroke can be scaled        */
    LCSTROKE_SDASH     = 0x05,    /* short for dash and scale    */
    LCSTROKE_SGAP      = 0x04,    /* short for gap and scale     */
    LCSTROKE_SINVERT   = 0x08,    /* Invert stroke in first code */
    LCSTROKE_EINVERT   = 0x10,    /* Invert stroke in last code  */
    };

/* Values for widthMode                */
    // WIP_LINESTYLE *** Overlaps LsStroke::WidthMode??
enum LineCodeWidth
    {
    LCWIDTH_NONE      = 0x00,
    LCWIDTH_LEFT      = 0x01,
    LCWIDTH_RIGHT     = 0x02,
    LCWIDTH_FULL      = 0x03,
    LCWIDTH_TAPER     = 0x04,
    LCWIDTH_TAPEREND  = 0x08,
    };

/*-----------------------------------------------------------------------
0 - Standard closed polygon (rectangle) strokes.
1 - No end cap. The stroke is displayed as two parallel lines.
2 - The end of the stroke is extended by half the stroke width.
3...
    If cap mode is >= 3, the cap is stroked as an arc and the value of
    capMode indicates the number of vectors in the arc.
-----------------------------------------------------------------------*/
enum  LineCodeCap
    {
    LCCAP_CLOSED     = 0,
    LCCAP_OPEN       = 1,
    LCCAP_EXTENDED   = 2,
    LCCAP_HEXAGON    = 3,
    LCCAP_OCTAGON    = 4,
    LCCAP_DECAGON    = 5,
    LCCAP_ARC        = 30,
    };

/* Values for "mod1"                   */
enum LineCodeMod
    {
    LCPOINT_NONE      = 0x0000,  // No point symbol
    LCPOINT_ORIGIN    = 0x0001,  // Symbol at origin of stroke
    LCPOINT_END       = 0x0002,  // Symbol at end of stroke
    LCPOINT_CENTER    = 0x0003,  // Symbol at center of stroke
    LCPOINT_ONSTROKE  = 0x0003,  // test mask
    LCPOINT_LINEORG   = 0x0004,  // Symbol at origin of element
    LCPOINT_LINEEND   = 0x0008,  // Symbol at end of element
    LCPOINT_LINEVERT  = 0x0010,  // Symbol at each vertex
    LCPOINT_ADJROT    = 0x0020,  // Adjust rotation left->right
    LCPOINT_ABSROT    = 0x0040,  // Angles not relative to line
    LCPOINT_NOSCALE   = 0x0100,  // No scale on variable strokes
    LCPOINT_NOCLIP    = 0x0200,  // No clip on partial strokes
    LCPOINT_NOPARTIAL = 0x0400,  // No partial strokes
    LCPOINT_PROJECT   = 0x0800,  // Project partial origin
    LCPOINT_COLOR     = 0x4000,  // Use color from symbol
    LCPOINT_WEIGHT    = 0x8000,  // Use weight from symbol
    };

enum class SnapMode
    {
    Invalid                 = -1,
    First                   = 0,
    None                    = 0,
    Nearest                 = 1,
    NearestKeypoint         = 1 << 1,
    MidPoint                = 1 << 2,
    Center                  = 1 << 3,
    Origin                  = 1 << 4,
    Bisector                = 1 << 5,
    Intersection            = 1 << 6,
    Tangency                = 1 << 7,
    TangentPoint            = 1 << 8,
    Perpendicular           = 1 << 9,
    PerpendicularPoint      = 1 << 10,
    Parallel                = 1 << 11,
    Multi3                  = 1 << 12,
    PointOn                 = 1 << 13,
    Multi1                  = 1 << 14,
    Multi2                  = 1 << 15,      // For compilation with VS2010, we can't use the | construct. Put back when we compile everything with VS2012 or later.
    MultiSnaps              = (Multi1 | Multi2 | Multi3),
//__PUBLISH_SECTION_END__
    AllOrdinary             = (Nearest | NearestKeypoint | MidPoint | Center | Origin | Bisector | Intersection | MultiSnaps),
    AllConstraint           = (Tangency | TangentPoint | Perpendicular | PerpendicularPoint | Parallel | PointOn),
    IntersectionCandidate   = (Intersection | Nearest),
//__PUBLISH_SECTION_START__
    };

ENUM_IS_FLAGS (SnapMode)

/** @endcond */

END_BENTLEY_DGNPLATFORM_NAMESPACE
