/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnPlatform.r.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnPlatformBaseType.r.h"
#include <BeJsonCpp/BeJsonUtilities.h>

//--------------------------------------------------------------------
// This file is included by both .cpp/h and .r files
//--------------------------------------------------------------------

// The following constants are in the Bentley namespace because they are referenced by MicroStation.r.h.
//  That file cannot use a namespace qualifier such as "DgnPlatform::", because the resource compiler doesn't
//  support that. We could use ifdef's if we don't want to put these symbols into Bentley. In any case,
//  These symbols all begin with "DGNPLATFORM_" so they shouldn't conflict with other symbols in the Bentley ns.
BEGIN_BENTLEY_API_NAMESPACE

/** @cond BENTLEY_SDK_Internal */

struct SPoint2d
    {
    Int16       x;
    Int16       y;
    };

/** @endcond */

//! Color definition
struct RgbaColorDef
    {
    Byte    red;
    Byte    green;
    Byte    blue;
    Byte    alpha;
    };

//! Color definition
struct RgbColorDef
    {
    Byte    red;
    Byte    green;
    Byte    blue;
    };

END_BENTLEY_API_NAMESPACE

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/** @cond BENTLEY_SDK_Internal */

//! Identifies the type or purpose of a model
enum class DgnModelType : UInt16
    {
    Physical                 = 0,   //!< a physical model.
    Sheet                    = 1,   //!< a sheet model.
    Redline                  = 2,   //!< a redline model.
    Drawing                  = 3,   //!< a 2d drawing model.
    Component                = 4,   //!< a 3d compnent model.
    TEMP_V8IMPORT_ONLY_3dSheet = 9, //!< @private
    Dictionary               = 10,  //!< @private
    Query                    = 11,  //!< a query model
    PhysicalRedline          = 12,  //!< a physical redline model.
    Illegal                  = 999, //!< @private
    };

/** @endcond */

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

enum class MSRenderMode
    {
    Invalid             = -1,
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
    };

inline bool operator== (MSRenderMode lhs, UInt32 rhs) {return static_cast<UInt32>(lhs) == rhs;}
inline bool operator!= (MSRenderMode lhs, UInt32 rhs) {return static_cast<UInt32>(lhs) != rhs;}

/*=================================================================================**//**
*  The flags that control view information.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ViewFlags
    {
    UInt32      fast_text:1;                //!< Shows or hides text elements. Note the inversion (e.g. "fast" text means don't show text elements).
    UInt32      line_wghts:1;               //!< Controls whether line weights are used (e.g. control whether elements with non-zero line weights draw normally, or as weight 0).
    UInt32      patterns:1;                 //!< Shows or hides pattern elements.
    UInt32      text_nodes:1;               //!< Shows or hides text node numbers and origins. These are decorations that can be shown to identify all text node elements.
    UInt32      ed_fields:1;                //!< Shows or hides the underlines that denote a text enter data field.
    UInt32      grid:1;                     //!< Shows or hides the grid. The grid settings are a design file setting.
    UInt32      lev_symb:1;                 //!< Controls whether level overrides are used (e.g. use the element level's symbology vs. the element's symbology).
    UInt32      constructs:1;               //!< Shows or hides elements that are in the construction class (controlled on a per-element basis).
    UInt32      dimens:1;                   //!< Shows or hides dimension elements.
    UInt32      fast_cell:1;                //!< Controls whether cells display as a bounding box instead of showing their actual content.
    UInt32      fill:1;                     //!< Controls whether the fills on filled elements are displayed.
    UInt32      auxDisplay:1;               //!< Shows or hides the ACS triad.
    UInt32      camera:1;                   //!< Controls whether camera settings are applied to the view's frustum.
    UInt32      renderMode:6;               //!< Controls the render mode of the view; see the MSRenderMode enumeration. This is typically controlled through a display style.
    UInt32      background:1;               //!< Shows or hides the background image. The image is a design file setting, and may be undefined.
    UInt32      textureMaps:1;              //!< Controls whether to display texture maps for material assignments.
    UInt32      transparency:1;             //!< Controls whether element transparency is used (e.g. control whether elements with transparency draw normally, or as opaque).
    UInt32      inhibitLineStyles:1;        //!< Controls whether custom line styles are used (e.g. control whether elements with custom line styles draw normally, or as solid lines). Note the inversion.
    UInt32      patternDynamics:1;          //!< Controls whether associative patthern display in dynamics (performance optimization)
    UInt32      renderDisplayEdges:1;       //!< Shows or hides visible edges in the shaded render mode. This is typically controlled through a display style.
    UInt32      renderDisplayHidden:1;      //!< Shows or hides hidden edges in the shaded render mode. This is typically controlled through a display style.
    UInt32      overrideBackground:1;       //!< Controls whether the view's custom background color is used. This is typically controlled through a display style.
    UInt32      noFrontClip:1;              //!< Controls whether the front clipping plane is used. Note the inversion. Elements beyond will not be displayed.
    UInt32      noBackClip:1;               //!< Controls whether the back clipping plane is used. Note the inversion. Elements beyond will not be displayed.
    UInt32      noClipVolume:1;             //!< Controls whether the clip volume is applied. Note the inversion. Elements beyond will not be displayed.
    UInt32      associativeClip:1;          //!< Controls whether the clip volume, if associated to an element should automatically update if/when the clip element is modified.
    UInt32      minimized:1;                //!< Current minimized state of view.
    UInt32      maximized:1;                //!< Current maximized state of view.
    UInt32      renderDisplayShadows:1;     //!< Shows or hides shadows. This is typically controlled through a display style.
    UInt32      hiddenLineStyle:3;          //!< Controls the line style (only line codes 0-7 are allowed) of hidden lines in the shaded render mode. This is typically controlled through a display style.
    UInt32      inhibitRenderMaterials:1;   //!< Controls whether element materials are used (e.g. control whether elements with materials draw normally, or as if they have no material).
    UInt32      ignoreSceneLights:1;        //!< Controls whether the custom scene lights or the default lighting scheme are used. Note the inversion.

    inline void SetRenderMode (MSRenderMode value) {renderMode = static_cast<UInt32>(value);}
    inline MSRenderMode GetRenderMode() {return static_cast<MSRenderMode>(renderMode);}

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
    UInt16          mode;
    UInt16          unused[2];
    struct
        {
        UInt16  clockwise:1;
        UInt16  unused:15;
        } flags;
    double          baseDir;
    };

struct Autodim1
    {
    struct
        {
        UInt16    adres2:8;
        UInt16    ref_mastersub:2;
        UInt16    ref_decfract:1;
        UInt16    accuracyFlags:2;  /* 0- use ref_decfract; 1- scientific accuracy; 2- fractional zero */
        UInt16    reserved:3;
        } format;
    };

/*=================================================================================**//**
* Color, weight, and style.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct Symbology
    {
    Int32           style;
    UInt32          weight;
    UInt32          color;
    };

/** @cond BENTLEY_SDK_Internal */
struct DegreeOfFreedom
    {
    Int32               locked;
    T_Adouble           value;
    };

/*=================================================================================**//**
* This structure is not intended to be used directly, and supports file storage.
* These flags identify every property that an element can override from its text style.
* \group    TextStyles
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct LegacyTextStyleOverrideFlags
{
    UInt16  fontNo                  :1;
    UInt16  shxBigFont              :1;
    UInt16  width                   :1;
    UInt16  height                  :1;
    UInt16  slant                   :1;
    UInt16  linespacing             :1;
    UInt16  interCharSpacing        :1;
    UInt16  underlineOffset         :1;
    UInt16  overlineOffset          :1;
    UInt16  just                    :1;
    UInt16  nodeJust                :1;
    UInt16  lineLength              :1;
    UInt16  direction               :1;
    UInt16  underline               :1;
    UInt16  overline                :1;
    UInt16  italics                 :1;
    UInt16  bold                    :1;
    UInt16  superscript             :1;
    UInt16  subscript               :1;
    UInt16  fixedSpacing            :1;
    UInt16  background              :1;
    UInt16  backgroundstyle         :1;
    UInt16  backgroundweight        :1;
    UInt16  backgroundcolor         :1;
    UInt16  backgroundfillcolor     :1;
    UInt16  backgroundborder        :1;
    UInt16  underlinestyle          :1;
    UInt16  underlineweight         :1;
    UInt16  underlinecolor          :1;
    UInt16  overlinestyle           :1;
    UInt16  overlineweight          :1;
    UInt16  overlinecolor           :1;
    UInt16  lineOffset              :1;
    UInt16  fractions               :1;
    UInt16  overlinestyleflag       :1;
    UInt16  underlinestyleflag      :1;
    UInt16  color                   :1;
    UInt16  widthFactor             :1;
    UInt16  colorFlag               :1;
    UInt16  fullJustification       :1;
    UInt16  acadLineSpacingType     :1;
    UInt16  backwards               :1;
    UInt16  upsidedown              :1;
    UInt16  acadInterCharSpacing    :1;
    UInt16  reserved                :4;
    UInt16  reserved2               :16;

    //! Computes the logical or of two sets of flags.
    //! The result (also the left-hand-side argument) contains true for every flag on in either instance. Reserved bits are included in the operation.
    public: DGNPLATFORM_EXPORT void ComputeLogicalOr (LegacyTextStyleOverrideFlags& result, LegacyTextStyleOverrideFlags const& rhs) const;

    //! True if any flags are set.
    //! Reserved bits are included in the operation.
    public: DGNPLATFORM_EXPORT bool AreAnyFlagsSet () const;
};

/*=================================================================================**//**
* This structure is not intended to be used directly.
* These flags identify whether specific attributes from a text style can be utilized.
* \group    TextStyles
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct LegacyTextStyleFlags
{
    UInt32  underline              :1;
    UInt32  overline               :1;
    UInt32  italics                :1;
    UInt32  bold                   :1;
    UInt32  superscript            :1;
    UInt32  subscript              :1;
    UInt32  background             :1;
    UInt32  overlineStyle          :1;
    UInt32  underlineStyle         :1;
    UInt32  fixedSpacing           :1;
    UInt32  fractions              :1;
    UInt32  color                  :1;
    UInt32  acadInterCharSpacing   :1;
    UInt32  fullJustification      :1;
    UInt32  acadLineSpacingType    :2;
    UInt32  acadShapeFile          :1;
    UInt32  reserved               :15;
}; // LegacyTextStyleFlags

/*=================================================================================**//**
* This structure is not intended to be used directly, and supports file storage.
* Contains the property data of a text style. Use DgnTextStyle instead.
* \group    TextStyles
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct LegacyTextStyle
{
    UInt32 fontNo; //!< Font number
    UInt32 shxBigFont; //!< SHX big font number
    double width; //!< Character width (uors)
    double height; //!< Character height (uors)
    double slant; //!< Slant in degrees
    double lineSpacing; //!< Vert uors between chars in text node (nodespace in old tcb)
    double interCharSpacing; //!< Space between characters (textAboveSpacing in old tcb)
    double underlineOffset; //!< Offset from baseline to underline
    double overlineOffset; //!< Offset from ascender height to overline
    double widthFactor; //!< Width factor multiplier
    DPoint2d lineOffset; //!< Offset of text from baseline (ie superscript)
    UInt16 just; //!< Text justification value (0-14)
    UInt16 nodeJust; //!< Text node justification
    UInt16 lineLength; //!< Maximum line length in node ((nodelen in old tcb)
    UInt16 textDirection; //!< Text direction
    Symbology backgroundStyle; //!< Style for text background border lines
    UInt32 backgroundFillColor; //!< Fill color for text background
    DPoint2d backgroundBorder; //!< Offset added background rectangle
    Symbology underlineStyle; //!< Underline style
    Symbology overlineStyle; //!< Overline style
    UInt32 parentId; //!< Parent style id
    LegacyTextStyleFlags flags; //!< Flags for what is on and what is off
    LegacyTextStyleOverrideFlags overrideFlags; //!< What is overriden from the parent in this style
    UInt32 color; //!< Color for the text element
    UInt32 reserved1;
    UInt32 reserved2;
    UInt32 reserved3;
    double reserved4;

    //! Scales all distance properties of this instance.
    DGNPLATFORM_EXPORT void Scale(double scale);
    
    //! Scales the distance properties of this instance according to the mask.
    DGNPLATFORM_EXPORT void Scale(double scale, LegacyTextStyleOverrideFlags const&);
}; // LegacyTextStyle

/** @endcond */

/*=================================================================================**//**
*  The unit information
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct UnitFlags
    {
    /** Flags for the unit base. */
    UInt32 base:3;                     /* UNIT_BASE_xxx */
    /** Flags for the unit system. */
    UInt32 system:3;                   /* UNIT_SYSTEM_xxx */
    /** Flags for future use. */
    UInt32 reserved:26;
    };

#if defined (NEEDS_WORK_DGNITEM)
/*=================================================================================**//**
*  Stores unit information.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct StoredUnitInfo
    {
    UnitFlags       flags;                          /* Unit flags */
    double          numerator;                      /* Units per meter fraction numerator */
    double          denominator;                    /* Units per meter fraction denominator */
    Utf16Char       label[DGNPLATFORM_RESOURCE_MAX_UNIT_LABEL_LENGTH];   /* Units Label */
    };
#endif

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

//__PUBLISH_SECTION_END__
enum XAttributeHandlerMajorIDs : UInt16
    {
    XATTRIBUTEID_CellIndexTool              = 100,
    XATTRIBUTEID_TabBasedViewGroup          = 101,
    XATTRIBUTEID_FilletChamferTool          = 102,
    XATTRIBUTEID_CameraNavigationSpeed      = 103,
    XATTRIBUTEID_ObjectStateID              = 104,
    XATTRIBUTEID_NamedViewDisplayElement    = 105,
    XATTRIBUTEID_RelationDictionary         = 106,
    XATTRIBUTEID_LevelMaskSubtree           = 107,
    XATTRIBUTEID_DynamicView                = 108,
    XATTRIBUTEID_ViewClipper                = 109,
    XATTRIBUTEID_PlacemarkHandler           = 110,
    XATTRIBUTEID_DisplayStyleListElement    = 111,
    XATTRIBUTEID_PolygonDwgExample          = 112,
    XATTRIBUTEID_RELATIONID_Reserved_1003   = 1003,
    XATTRIBUTEID_RELATIONID_GlobalVariable  = 1002,
    XATTRIBUTEID_RightsToken                = 1003,
    XATTRIBUTEID_String                     = 22226,    // LINKAGEID_String
    XATTRIBUTEID_MPTOOLSApplication         = 22239,    // MPTOOLS_APPLICATION_SIGNATURE
    XATTRIBUTEID_XML                        = 22243,    // LINKAGEID_XML
    XATTRIBUTEID_ElementHandler             = 22252,
    XATTRIBUTEID_StdsCheckIgnoredError      = 22258,    // LINKAGEID_StdsCheckIgnoredError
    XATTRIBUTEID_Relationship               = 22260,
    XATTRIBUTEID_DesignLinks                = 22261,
    XATTRIBUTEID_ECXAttributes              = 22271,    // LINKAGEID_ECXAttributes
    XATTRIBUTEID_SymbolSettings             = 22272,
    XATTRIBUTEID_AnnotationScale            = 22274,
    XATTRIBUTEID_Generic                    = 22275,
    XATTRIBUTEID_ColorBook                  = 22276,
    XATTRIBUTEID_ECField                    = 22277,
    XATTRIBUTEID_IcoData                    = 22278,
    XATTRIBUTEID_MstnSettings               = 22279,
    XATTRIBUTEID_MaterialProperties         = 22280,
    XATTRIBUTEID_MaterialTable              = 22282,
    XATTRIBUTEID_RasterFrame                = 22283,
    XATTRIBUTEID_GoogleEarthData            = 22286,
    XATTRIBUTEID_ConflictRevisions          = 22287,    // LINKAGEID_ConflictRevisions */
    XATTRIBUTEID_BSurfTrimCurve             = 22290,
    XATTRIBUTEID_MstnViewHandler            = 22293,
    XATTRIBUTEID_DGNECPlugin                = 22294,    // LINKAGEID_DGNECPlugin
    XATTRIBUTEID_ViewInfo                   = 22295,
    XATTRIBUTEID_XGraphics                  = 22296,
    XATTRIBUTEID_PlacemarkData              = 22297,
    XATTRIBUTEID_PrintStyle                 = 22298,    // LINKAGEID_PrintStyle
    XATTRIBUTEID_TemplateData               = 22587,    // LINKAGEID_TemplateData
    XATTRIBUTEID_ECXDataTreeData            = 22587,    // more generic name then XATTRIBUTEID_TemplateData
    XATTRIBUTEID_REFERENCE_PROVIDERID       = 22624,    // LINKAGEID_REFERENCE_PROVIDERID
    XATTRIBUTEID_NamedExpressionData        = 22625,
    XATTRIBUTEID_DynamicViewSettings        = 22626,
    XATTRIBUTEID_NamedExpressionKeywordData = 22627,
    XATTRIBUTEID_SectionGeometryGenerator   = 22628,
    XATTRIBUTEID_ViewDisplayOverrides       = 22629,
    XATTRIBUTEID_DisplayStyle               = 22630,
    XATTRIBUTEID_StandardSurface            = 22631,
    XATTRIBUTEID_SectionGeometryCached      = 22632,
    XATTRIBUTEID_DisplayStyleMap            = 22633,
    XATTRIBUTEID_DisplayStyleIndex          = 22634,
    XATTRIBUTEID_ActiveTextStyle            = 22635,
    XATTRIBUTEID_RefDynamicViewSettings     = 22636,
    XATTRIBUTEID_DynamicViewHandler         = 22637,
    XATTRIBUTEID_ComponentSetQuery          = 22638,
    XATTRIBUTEID_DesignReviewData           = 22639,
    XATTRIBUTEID_SmartModeling              = 22640,
    XATTRIBUTEID_RebisElementHandler        = 22641,
    XATTRIBUTEID_Markup                     = 22642,
    XATTRIBUTEID_LxoViewSettings            = 22643,
    XATTRIBUTEID_LxoSetupListElement        = 22644,
    XATTRIBUTEID_LxoSetup                   = 22645,
    XATTRIBUTEID_LxoMaterialPreset          = 22646,
    XATTRIBUTEID_MaterialPreview            = 22647,
    XATTRIBUTEID_SpiralHandlerXAtrID        = 22648,
    XATTRIBUTEID_LightProperties            = 22649,
    XATTRIBUTEID_XGraphicsName              = 22650,
    XATTRIBUTEID_InterferenceJob            = 22652,
    XATTRIBUTEID_MarkupViewElementHandler   = 22653,
    XATTRIBUTEID_IModelProxy                = 22654,
    XATTRIBUTEID_CivilPlatform              = 22739,
    XATTRIBUTEID_NamedViewCalloutProfile    = 22741,
    XATTRIBUTEID_InsolationMesh             = 22744,
    XATTRIBUTEID_HUDMarker                  = 22745,
    XATTRIBUTEID_PointCloudHandler          = 22746,
    XATTRIBUTEID_ScheduleLinker             = 22750,     // This is also an Element Handler ID for ScheduleLinker Settings Element Handler
    // ***  STOP!!!  ***
    // This is NOT a comprehensive list of XATTRIBUTE IDs and should not be used to obtain new ids!!!
    // Ideally, all of these values should be moved to private header files.
    // You must get new XAttribute IDs from http://toolsnet.bentley.com/Signature/Default.aspx
    };
//__PUBLISH_SECTION_START__

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
    MUSUPU                          = 2,    //!< Master Units / SubUnits / Positional Units
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
enum class DirectionMode : UInt16
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
/*---------------------------------------------------------------------------------**//**
*  Known file formats supported by DgnPlatform. Some formats are supported only with the help of host-supplied plugins.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
enum class DgnFileFormatType
    {
    Invalid         = -1,       //!< this means "can't recognize file format"
    Current         = 0,        //!< this means "match the current format"

    V7              = 1,        //!< V7 DGN file
    V8              = 2,        //!< V8 DGN file
    DWG             = 3,        //!< DWG file
    DXF             = 4,        //!< DXF file
    DgnDb           = 5,        //!< DgnDb file
    Native          = 5,        //!< The "native" format for Dgn data
//__PUBLISH_SECTION_END__
    PDF             = 6,        //!< PDF file
    ThreeDS         = 7,
    OBJ             = 8,
    Image           = 9,
    SKP             = 10,
    OpenNurbs       = 11,
    PDS             = 12,
    RVM             = 13,
    FBX             = 14,
    SHP             = 100,
    PersonalGeoDB   = 101,      //!< currently not supported.
    MIFMID          = 102,      //!< MapInfo MIF/MID
    MITAB           = 103,
    IFC             = 104,
    JT              = 105,
    //  RESERVED    = 106,
    IMPX            = 107,

    // These are imported and/or exported by MicroStation through keyins, cannote be opened as master file or attached as referenced
    IGESFile        = 200,
    STEP            = 201,
    CGM             = 202,
    XMT             = 203,
    SAT             = 204,
    STL             = 205,
    V7CELL          = 206,

    // special use. Not for general use.
    Copy            = 300,
    Unknown         = 301,

//__PUBLISH_SECTION_START__
    };

enum class DwgUnitFormat
    {
    Scientific                   = 1,
    Decimal                      = 2,
    Engineering                  = 3,
    Architectural                = 4,
    Fractional                   = 5,
    };

// __PUBLISH_SECTION_END__
struct MSStringLinkageData
    {
    UInt16                  linkageKey;                     /* id identifying type of string linkage */
    UInt16                  mustBeZero;                     /* must be zero! Do not reuse! 3rd party programs (incorrectly) read linkageKey as a 32-bit value and so pick up this value as the high word. It must be zero! */
    UInt32                  linkageStringLength;            /* Size of string linkage data */
#if defined (type_resource_generator)
    char                    linkageString[0];               /* String Buffer */ // WIP_CHAR_OK - Persistence
#else
    char                    linkageString[1];               // WIP_CHAR_OK - Persistence
#endif
    };

struct MSBitMaskLinkageData
    {
    UInt16  linkageKey;             // ID identifying type of bitmask linkage
    UInt16  defaultBitValue :1;     // Default value for bits */
    UInt16  reserved        :15;
    UInt32  numValidBits;           // Number of valid bits in the bitMaskArray
    UInt32  numShorts;              // Number of shorts to follow in the linkage

#if defined (type_resource_generator)
    UInt16  bitMaskArray[0];        // Bit Mask Array
#else
    UInt16  bitMaskArray[2];        // Bit Mask Array (2 to make sizeof(make) sense)
#endif

    };

struct MSMultiStateMaskLinkageData
    {
    UInt16                  linkageKey;                         /* id identifying type of multi state mask linkage */
    UInt16                  numBitsPerState;                    /* number of bits stored per state value */
    UInt16                  defaultValue;                       /* default multi state value*/
    UInt16                  reserved;
    UInt32                  numStates;                          /* Number of states stored in stateArray */
    UInt32                  numShorts;                          /* Number of shorts to follow in the linkage */
#if defined (type_resource_generator)
    UInt16                  stateArray[0];                      /* Multi state mask Array */
#else
    UInt16                  stateArray[2];                      /* Multi state mask Array (2 to make sizeof(make) sense */
#endif
    };

struct MSSymbologyLinkageData
    {
    UInt16                  linkageKey;                     /* id identifying type of symbology linkage */
    UInt16                  overrideStyle:1;
    UInt16                  overrideWeight:1;
    UInt16                  overrideColor:1;
    UInt16                  overrideLevel:1;
    UInt16                  reserved:12;
    Int32                   style;
    UInt32                  weight;
    UInt32                  color;
    UInt32                  level;
    };

struct MSXMLLinkageData
    {
    UInt16                  linkageType;        /* id identifying type of XML linkage */
    UInt16                  appID;              /* Application user id number */
    UInt16                  appType;            /* Application type of data stored */
    UInt16                  reserved;
    UInt32                  numBytes;
#if defined (type_resource_generator)
    UInt8                   byteBuffer[0];
#else
    UInt8                   byteBuffer[1];
#endif
    };

struct MSNoteData
    {
    unsigned long   size;
    char            name [20]; //Note name identifier // WIP_CHAR_OK - Persistence
    };

// __PUBLISH_SECTION_START__
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

/*---------------------------------------------------------------------------------**//**
@ingroup MultilineElements
*  The Offset Mode (or justification) of a multi-line defines the location of the line that the user
*   has drawn relative to the offset of the style.  The Work Line is the 0-offset line,
*   which may not be the center of a multi-line.  This value was not stored for pre-V8 files.
+---------------+---------------+---------------+---------------+---------------+------*/
enum class MlineOffsetMode
    {
    //! The Offset Mode is not stored on all multi-lines.  This will be returned for elements that do not have it set.
    Unknown         = -1,
    //! The user-defined line traces the 0 profile offset of the style
    ByWork          = 0,
    //! The user-defined line traces the center ((max-min)/2) of the style
    ByCenter        = 1,
    //! The user-defined line traces the maximum offset of the style
    ByMax           = 2,
    //! The user-defined line traces the minimum offset of the style
    ByMin           = 3,
    //! The user-defined line traces a custom offset of the style
    Custom          = 4
    };

/*---------------------------------------------------------------------------------**//**
@ingroup MultilineElements
*  The break flags are used to specify that a particular break should extend to or from
*    a joint.  Use MLBREAK_STD to specify the exact length of a break.
+---------------+---------------+---------------+---------------+---------------+------*/
enum MlineBreakLengthType
    {
    //! Use both offset and length.
    MLBREAK_STD             = 0,
    //! The break starts at the joint line at the specified segment's origin, and the value of offset is ignored.
    MLBREAK_FROM_JOINT      = 0x8000,
    //! The break ends at the joint line at the specified segment's end, and the value of length is ignored.
    MLBREAK_TO_JOINT        = 0x4000,
    //! Break from the specified break location to the second point on the segment.  This is MLBREAK_FROM_JOINT & MLBREAK_TO_JOINT.
    MLBREAK_BETWEEN_JOINTS  = 0xC000,
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
    LCSTROKE_SDASH     = 0x05,    /* Short for dash and scale    */
    LCSTROKE_SGAP      = 0x04,    /* Short for gap and scale     */
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

#define     LAST_SNAP_MODE 16

enum class  DgnElementClass
    {
    Primary                 = 0,
    PatternComponent        = 1,
    Construction            = 2,
    Dimension               = 3,
    PrimaryRule             = 4, //!< Never used directly, internal to Type 18/19 implmentation only!
    LinearPatterned         = 5,
    ConstructionRule        = 6, //!< Never used directly, internal to Type 18/19 implmentation only!
    };

#if defined (NEEDS_WORK_DGNITEM)
enum MSElementTypes : UShort
    {
    CELL_HEADER_ELM                 = 2,
    LINE_ELM                        = 3,
    LINE_STRING_ELM                 = 4,
    GROUP_DATA_ELM                  = 5,
    SHAPE_ELM                       = 6,
    TEXT_NODE_ELM                   = 7,
    DIG_SETDATA_ELM                 = 8,
    DGNFIL_HEADER_ELM               = 9,
    LEV_SYM_ELM                     = 10,
    CURVE_ELM                       = 11,
    CMPLX_STRING_ELM                = 12,
    CMPLX_SHAPE_ELM                 = 14,
    ELLIPSE_ELM                     = 15,
    ARC_ELM                         = 16,
    TEXT_ELM                        = 17,
    SURFACE_ELM                     = 18,
    SOLID_ELM                       = 19,
    BSPLINE_POLE_ELM                = 21,
    POINT_STRING_ELM                = 22,
    CONE_ELM                        = 23,
    BSPLINE_SURFACE_ELM             = 24,
    BSURF_BOUNDARY_ELM              = 25,
    BSPLINE_KNOT_ELM                = 26,
    BSPLINE_CURVE_ELM               = 27,
    BSPLINE_WEIGHT_ELM              = 28,
    DIMENSION_ELM                   = 33,
    SHAREDCELL_DEF_ELM              = 34,
    SHARED_CELL_ELM                 = 35,
    MULTILINE_ELM                   = 36,
    DGNSTORE_COMP                   = 38,
    DGNSTORE_HDR                    = 39,
    MICROSTATION_ELM                = 66,
    RASTER_HDR                      = 87,
    RASTER_COMP                     = 88,
    RASTER_REFERENCE_ELM            = 90,
    RASTER_REFERENCE_COMP           = 91,
    RASTER_HIERARCHY_ELM            = 92,
    RASTER_HIERARCHY_COMP           = 93,
    RASTER_FRAME_ELM                = 94,
    TABLE_ENTRY_ELM                 = 95,
    TABLE_ELM                       = 96,
//__PUBLISH_SECTION_END__
    CELL_LIB_ELM                    = 1,
    CONIC_ELM                       = 13,
    MATRIX_HEADER_ELM               = 101,
    MATRIX_INT_DATA_ELM             = 102,
    MATRIX_DOUBLE_DATA_ELM          = 103,
    MESH_HEADER_ELM                 = 105,
    EXTENDED_NONGRAPHIC_ELM         = 107,
    EXTENDED_ELM                    = 106,

//__PUBLISH_SECTION_START__
    MSELEMENTTYPES_MaxElements      = 113,
    };
#endif

//__PUBLISH_SECTION_END__
#if !defined (mdl_resource_compiler) && !defined (mdl_resource_compiler)
    struct DgnFont;
#endif
//__PUBLISH_SECTION_START__

/*----------------------------------------------------------------------+
|   Color Modes
+----------------------------------------------------------------------*/
enum  class ImageColorMode
    {
    Unknown         = 65535,
    Any             = 0,
    RGB             = 1,
    Palette16       = 2,
    Palette256      = 3,
    GreyScale       = 4,
    Monochrome      = 5,
    RGBA            = 6,
    Palette256Alpha = 7,
    GreyScale16     = 8,
    Palette2        = 9
    };

//__PUBLISH_SECTION_END__

enum
    {
    DOUBLEQUOTE = 0x22,
    QUOTE       = 0x27,
    TAB         = 0x09,
    LF          = 0x0A,
    CR          = 0x0D,
    CNTRL_Z     = 0x1a,
    BACKSLASH   = 0x5c,
    BS          = 0x08,
    FF          = 0x0c,
    };

//__PUBLISH_SECTION_START__

/** @endcond */

END_BENTLEY_DGNPLATFORM_NAMESPACE
