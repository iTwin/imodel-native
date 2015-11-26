/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/AreaPattern.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#define  MAX_DWG_HATCH_LINE_DASHES      20

#if !defined(__midl) // For a MIDL compile, all we care about are the values of the #define constants.

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
 @addtogroup AreaPattern
 Support for MicroStation's Area Patterning capabilities.
 @bsiclass
+===============+===============+===============+===============+===============+======*/
//! DWG Style Hatch Definition.
struct DwgHatchDefLine
{
double          angle;
DPoint2d        through;
DPoint2d        offset;
short           nDashes;
double          dashes[MAX_DWG_HATCH_LINE_DASHES];
};

struct DwgHatchDef
{
double                      pixelSize;
short                       islandStyle;
bvector<DwgHatchDefLine>    hatchLines;
};

//! PatternParams compare flags.
enum PatternParamsCompareFlags
    {
    PATTERNPARAMSCOMPAREFLAGS_RMatrix           = 0x0001,
    PATTERNPARAMSCOMPAREFLAGS_Offset            = 0x0002,
    PATTERNPARAMSCOMPAREFLAGS_Default           = 0x0004,
    PATTERNPARAMSCOMPAREFLAGS_Origin            = 0x0010,
    PATTERNPARAMSCOMPAREFLAGS_Symbology         = 0x0020,
    PATTERNPARAMSCOMPAREFLAGS_Mline             = 0x0040,
    PATTERNPARAMSCOMPAREFLAGS_Tolerance         = 0x0080,
    PATTERNPARAMSCOMPAREFLAGS_HoleStyle         = 0x0100,
    PATTERNPARAMSCOMPAREFLAGS_DwgHatch          = 0x0200,
    PATTERNPARAMSCOMPAREFLAGS_AnnotationScale   = 0x0400,
    PATTERNPARAMSCOMPAREFLAGS_All               = 0xffff,
    };

//! Selects Pattern Hole Style.
enum class PatternParamsHoleStyleType
    {
    Normal = 0, //!< Normal rules....Hatch if in outer, but not hole.
    Parity = 1, //!< Parity
    };

//! @ingroup AreaPattern
//! Flags indicating modification of corresponding field in PatternParams.                  
enum class PatternParamsModifierFlags
    {
    None                = 0,        //!< No flags set.
    Space1              = 0x0001,   //!< patternParams.space1 present
    Angle1              = 0x0002,   //!< patternParams.angle1 present
    Space2              = 0x0004,   //!< patternParams.space2 present
    Angle2              = 0x0008,   //!< patternParams.angle2 present
    Scale               = 0x0010,   //!< patternParams.scale present
    Cell                = 0x0020,   //!< patternParams.cellId present
    Tolerance           = 0x0040,   //!< patternParams.tolerance present
    Style               = 0x0080,   //!< patternParams.style present
    Weight              = 0x0100,   //!< patternParams.weight present
    Color               = 0x0200,   //!< patternParams.color present
    Snap                = 0x0400,   //!< set if pattern is snappable
    RotMatrix           = 0x0800,   //!< patternParams.rMatrix present
    Offset              = 0x1000,   //!< patternParams.offset present
    HoleStyle           = 0x2000,   //!< patternParams.holeStyle present
    DwgHatchDef         = 0x4000,   //!< patternParams.dwgHatchDef present
    Multiline           = 0x8000,   //!< patternParams.minLine, patternParams.maxLine present
    Origin              = 0x10000,  //!< patternParams.origin present
    PixelSize           = 0x20000,  //!< patternParams.dwgHatchDef.pixelSize set
    IslandStyle         = 0x40000,  //!< patternParams.dwgHatchDef.islandStyle set
    TrueScale           = 0x80000,  //!< set if pattern cell is true scaled
    RawDwgLoops         = 0x100000, //!< pattern contains raw DWG loop data
    DwgHatchOrigin      = 0x200000, //!< dwg hatch origin has been specified as pattern origin
    AnnotationScale     = 0x400000, //!< PatternParams.annotationscale present
    };

ENUM_IS_FLAGS(PatternParamsModifierFlags)

//! Selects Pattern Placement Type.
enum PatternPlacementTypes
    {
    PATTERN_HATCH           = 0x0,
    PATTERN_CROSSHATCH      = 0x1,
    PATTERN_AREA            = 0x2,
    };

//! @ingroup AreaPattern
//! Each member of the PatternParams structure has a corresponding bit in
//! PatternParamsModifierFlags that indicates whether or not the member is used.
//! For example, if "rMatrix" is to be used, set the appropriate bit in
//! the modifiers member using the bitwise OR operator
//! (.modifiers |= PatternParamsModifiers::RotMatrix). If the "rMatrix" member is to be ignored,
//! clear the appropriate bit in "modifiers" (.modifiers &= ~PatternParamsModifierFlags::RotMatrix).
// For in-memory use only! 
struct PatternParams : RefCountedBase
{
//__PUBLISH_SECTION_END__
RotMatrix                   rMatrix;                        //!< The pattern coordinate system.
DPoint3d                    offset;                         //!< The offset from the element origin.
double                      space1;                         //!< The primary (row) spacing.
double                      angle1;                         //!< The angle of first hatch or pattern.
double                      space2;                         //!< The secondary (column) spacing.
double                      angle2;                         //!< The angle of second hatch.
double                      scale;                          //!< The pattern scale.
double                      tolerance;                      //!< The pattern tolerance.
double                      annotationscale;                //!< The annotation scale.
WChar                       cellName[512];                  //!< The name of cell - used for create, stored as id.
int64_t                     cellId;                         //!< The ID of shared cell definition.
PatternParamsModifierFlags  modifiers;                      //!< The pattern modifiers bit field.
int32_t                     minLine;                        //!< The min line of multi-line element.
int32_t                     maxLine;                        //!< The max line of multi-line element.
ColorDef                    color;                          //!< The pattern / hatch color.
uint32_t                    weight;                         //!< The pattern / hatch weight.
uint32_t                    style;                          //!< The pattern / hatch line style.
int16_t                     holeStyle;                      //!< The hole parity style.
DwgHatchDef                 dwgHatchDef;                    //!< The DWG style hatch definition.
DPoint3d                    origin;                         //!< The hatch origin.

DGNPLATFORM_EXPORT PatternParams ();
DGNPLATFORM_EXPORT void Init ();

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

//! Create an instance of a PatternParams.
DGNPLATFORM_EXPORT static PatternParamsPtr Create ();

//! Create an instance of a PatternParams from an existing PatternParams.
DGNPLATFORM_EXPORT static PatternParamsPtr CreateFromExisting (PatternParamsCR existing);

//! Copy values from an existing PatternParams.
DGNPLATFORM_EXPORT void Copy (PatternParamsCR existing);

//! Compare two PatternParams.
DGNPLATFORM_EXPORT bool IsEqual (PatternParamsCR params, PatternParamsCompareFlags compareFlags=PATTERNPARAMSCOMPAREFLAGS_Default);

//! Get the pattern modifiers bit field.
DGNPLATFORM_EXPORT PatternParamsModifierFlags GetModifiers () const;

//! Get pattern orientation. Used if PatternParamsModifierFlags::RotMatrix is set.
DGNPLATFORM_EXPORT RotMatrixCR GetOrientation () const;

//! Get pattern offset from element origin. Used if PatternParamsModifierFlags::Offset is set.
DGNPLATFORM_EXPORT DPoint3dCR GetOffset () const;

//! Get pattern hatch spacing or area pattern row spacing. Used if PatternParamsModifierFlags::Space1 is set.
DGNPLATFORM_EXPORT double GetPrimarySpacing () const;

//! Get pattern hatch angle or area pattern angle. Used if PatternParamsModifierFlags::Angle1 is set.
DGNPLATFORM_EXPORT double GetPrimaryAngle () const;

//! Get pattern cross hatch spacing or area pattern column spacing. Used if PatternParamsModifierFlags::Space2 is set.
DGNPLATFORM_EXPORT double GetSecondarySpacing () const;

//! Get pattern cross hatch angle. Used if PatternParamsModifierFlags::Angle2 is set.
DGNPLATFORM_EXPORT double GetSecondaryAngle () const;

//! Get pattern scale. Used if PatternParamsModifierFlags::Scale is set.
DGNPLATFORM_EXPORT double GetScale () const;

//! Get pattern choord tolerance. Used if PatternParamsModifierFlags::Tolerance is set.
DGNPLATFORM_EXPORT double GetTolerance () const;

//! Get pattern cell name. Used if PatternParamsModifierFlags::Cell is set.
//! @note Not stored, set to name of cell referenced by GetCellId.
DGNPLATFORM_EXPORT WCharCP GetCellName () const;

//! Get pattern cell element id. Used if PatternParamsModifierFlags::Cell is set.
DGNPLATFORM_EXPORT DgnElementId GetCellId () const;

//! Get pattern min multiline boundary index. Used if PatternParamsModifierFlags::Multiline is set.
DGNPLATFORM_EXPORT int32_t GetMinLine () const;

//! Get pattern max multiline boundary index. Used if PatternParamsModifierFlags::Multiline is set.
DGNPLATFORM_EXPORT int32_t GetMaxLine () const;

//! Get pattern color. Used if PatternParamsModifierFlags::Color is set.
//! @note Uses element color if not set. Ignored for area pattern using a graphic cell not a point cell.
DGNPLATFORM_EXPORT ColorDef GetColor () const;

//! Get pattern weight. Used if PatternParamsModifierFlags::Weight is set.
//! @note Uses element weight if not set. Ignored for area pattern using a graphic cell not a point cell.
DGNPLATFORM_EXPORT uint32_t GetWeight () const;

//! Get pattern line style. Used if PatternParamsModifierFlags::Style is set.
//! @note Uses element line style if not set. Ignored for area pattern using a graphic cell not a point cell.
//! @note Only line styles 0-7 are supported.
DGNPLATFORM_EXPORT int32_t GetStyle () const;

//! Get pattern dwg hole style. Used if PatternParamsModifierFlags::HoleStyle is set.
DGNPLATFORM_EXPORT PatternParamsHoleStyleType GetHoleStyle () const;

//! Get pattern dwg hatch definition. Used if PatternParamsModifierFlags::DwgHatchDef is set.
DGNPLATFORM_EXPORT DwgHatchDefCR GetDwgHatchDef () const;

//! Get pattern origin. Used if PatternParamsModifierFlags::Origin is set.
//! @note Not supported, pattern origin stored as offset from element origin.
DGNPLATFORM_EXPORT DPoint3dCR GetOrigin () const;

//! Get annotation scale. Used if PatternParamsModifierFlags::AnnotationScale is set.
//! @note If PatternParamsModifierFlags::AnnotationScale is set, this pattern is an 'annotative' element, which typically synchronizes its scale with model's annotation scale.
DGNPLATFORM_EXPORT double GetAnnotationScale () const;

//! Set the pattern modifiers bit field.
DGNPLATFORM_EXPORT void SetModifiers (PatternParamsModifierFlags);

//! Set pattern orientation. Sets modifier bit for PatternParamsModifierFlags::RotMatrix.
DGNPLATFORM_EXPORT void SetOrientation (RotMatrixCR);

//! Set pattern offset from element origin. Sets modifier bit for PatternParamsModifierFlags::Offset.
DGNPLATFORM_EXPORT void SetOffset (DPoint3dCR);

//! Set pattern hatch spacing or area pattern row spacing. Sets modifier bit for PatternParamsModifierFlags::Space1.
DGNPLATFORM_EXPORT void SetPrimarySpacing (double);

//! Set pattern hatch angle or area pattern angle. Sets modifier bit for PatternParamsModifierFlags::Angle1.
DGNPLATFORM_EXPORT void SetPrimaryAngle (double);

//! Set pattern cross hatch spacing or area pattern column spacing. Sets modifier bit for PatternParamsModifierFlags::Space2.
DGNPLATFORM_EXPORT void SetSecondarySpacing (double);

//! Set pattern cross hatch angle. Sets modifier bit for PatternParamsModifierFlags::Angle2.
DGNPLATFORM_EXPORT void SetSecondaryAngle (double);

//! Set pattern pattern scale. Sets modifier bit for PatternParamsModifierFlags::Scale.
DGNPLATFORM_EXPORT void SetScale (double);

//! Set pattern choord tolerance. Sets modifier bit for PatternParamsModifierFlags::Tolerance.
DGNPLATFORM_EXPORT void SetTolerance (double);

//! Set pattern cell by supplying the cell name. Sets modifier bit for PatternParamsModifierFlags::Cell. 
//! @note Used by mdlPattern_addAssociative. Will search attached cell library and attempt to create a new shared cell definition 
//!       if one with this name does not already exist. When using IAreaFillPropertiesEdit::AddPattern you can not specify the 
//!       pattern cell by name and must instead call SetCellId with the element id of an existing shared cell definittion.
DGNPLATFORM_EXPORT void SetCellName (WCharCP);

//! Set pattern cell by supplying the element id of an existing shared cell definition. Sets modifier bit for PatternParamsModifierFlags::Cell.
DGNPLATFORM_EXPORT void SetCellId (DgnElementId);

//! Set pattern min multiline boundary index. Sets modifier bit for PatternParamsModifierFlags::Multiline. Valid only for multiline elements.
DGNPLATFORM_EXPORT void SetMinLine (int32_t);

//! Set pattern max multiline boundary index. Sets modifier bit for PatternParamsModifierFlags::Multiline. Valid only for multiline elements.
DGNPLATFORM_EXPORT void SetMaxLine (int32_t);

//! Set pattern color. Sets modifier bit for PatternParamsModifierFlags::Color.
//! @note Use to set a pattern color that is different than element color. Ignored for area pattern using a graphic cell not a point cell.
DGNPLATFORM_EXPORT void SetColor (ColorDef);

//! Set pattern weight. Sets modifier bit for PatternParamsModifierFlags::Weight.
//! @note Use to set a pattern weight that is different than element weight. Ignored for area pattern using a graphic cell not a point cell.
DGNPLATFORM_EXPORT void SetWeight (uint32_t);

//! Set pattern line style. Sets modifier bit for PatternParamsModifierFlags::Style.
//! @note Use to set a pattern line style that is different than element line style. Ignored for area pattern using a graphic cell not a point cell.
//! @note Only line styles 0-7 are supported.
DGNPLATFORM_EXPORT void SetStyle (int32_t);

//! Set pattern dwg hole style. Sets modifier bit for PatternParamsModifierFlags::HoleStyle.
DGNPLATFORM_EXPORT void SetHoleStyle (PatternParamsHoleStyleType);

//! Set pattern dwg hatch definition. Sets modifier bit for PatternParamsModifierFlags::DwgHatchDef.
DGNPLATFORM_EXPORT void SetDwgHatchDef (DwgHatchDefCR);

//! Set pattern origin. Sets modifier bit for PatternParamsModifierFlags::Origin.
//! @note Not supported, define pattern origin relative to element origin using SetOffset instead.
DGNPLATFORM_EXPORT void SetOrigin (DPoint3dCR);

//! Set annotation scale. Sets modifier bit for PatternParamsModifierFlags::AnnotationScale.
//! @note This is optional. Setting it makes the pattern an 'annotative' element, which typically synchronizes its scale with model's annotation scale.
DGNPLATFORM_EXPORT void SetAnnotationScale (double);

}; // PatternParams

//__PUBLISH_SECTION_END__

enum PatternPlacementFlags
{
PATTERN_FLAGS_OverideTrueScale  = (1<<0),       //!< If false ignore UseTrueScale flag, use active setting
PATTERN_FLAGS_UseTrueScale      = (1<<1),       //!< Apply true scale
PATTERN_FLAGS_Default           = (0),          //!< Use active setting by default
};

//__PUBLISH_SECTION_START__

#endif // !defined(__midl)

END_BENTLEY_DGNPLATFORM_NAMESPACE

