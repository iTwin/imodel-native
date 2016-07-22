/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/AreaPattern.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#define  MAX_DWG_HATCH_LINE_DASHES      20

BEGIN_BENTLEY_DGN_NAMESPACE

/*=================================================================================**//**
 @addtogroup AreaPattern
 @bsiclass
+===============+===============+===============+===============+===============+======*/
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

//=======================================================================================
//! @ingroup AreaPattern
//! Each member of the PatternParams structure has a corresponding bit in
//! PatternParamsModifierFlags that indicates whether or not the member is used.
//! For example, if "rMatrix" is to be used, set the appropriate bit in
//! the modifiers member using the bitwise OR operator
//! (.modifiers |= PatternParamsModifiers::RotMatrix). If the "rMatrix" member is to be ignored,
//! clear the appropriate bit in "modifiers" (.modifiers &= ~PatternParamsModifierFlags::RotMatrix).
// For in-memory use only! 
// @bsiclass 
//=======================================================================================
struct PatternParams : RefCountedBase
{
    RotMatrix                   rMatrix;                        //!< The pattern coordinate system.
    DPoint3d                    offset;                         //!< The offset from the element origin.
    double                      space1;                         //!< The primary (row) spacing.
    double                      angle1;                         //!< The angle of first hatch or pattern.
    double                      space2;                         //!< The secondary (column) spacing.
    double                      angle2;                         //!< The angle of second hatch.
    double                      scale;                          //!< The pattern scale.
    double                      tolerance;                      //!< The pattern tolerance.
    double                      annotationscale;                //!< The annotation scale.
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

    PatternParams() {Init();}
    DGNPLATFORM_EXPORT void Init();

public:
    //! Create an instance of a PatternParams.
    static PatternParamsPtr Create() {return new PatternParams();}

    //! Create an instance of a PatternParams from an existing PatternParams.
    DGNPLATFORM_EXPORT static PatternParamsPtr CreateFromExisting(PatternParamsCR existing);

    //! Copy values from an existing PatternParams.
    DGNPLATFORM_EXPORT void Copy(PatternParamsCR existing);

    //! Compare two PatternParams.
    DGNPLATFORM_EXPORT bool IsEqual(PatternParamsCR params, PatternParamsCompareFlags compareFlags=PATTERNPARAMSCOMPAREFLAGS_Default);

    //! Get the pattern modifiers bit field.
    PatternParamsModifierFlags GetModifiers() const {return modifiers;}

    //! Get pattern orientation. Used if PatternParamsModifierFlags::RotMatrix is set.
    RotMatrixCR GetOrientation() const {return rMatrix;}

    //! Get pattern offset from element origin. Used if PatternParamsModifierFlags::Offset is set.
    DPoint3dCR GetOffset() const {return offset;}

    //! Get pattern hatch spacing or area pattern row spacing. Used if PatternParamsModifierFlags::Space1 is set.
    double GetPrimarySpacing() const {return space1;}

    //! Get pattern hatch angle or area pattern angle. Used if PatternParamsModifierFlags::Angle1 is set.
    double GetPrimaryAngle() const {return angle1;}

    //! Get pattern cross hatch spacing or area pattern column spacing. Used if PatternParamsModifierFlags::Space2 is set.
    double GetSecondarySpacing() const {return space2;}

    //! Get pattern cross hatch angle. Used if PatternParamsModifierFlags::Angle2 is set.
    double GetSecondaryAngle() const {return angle2;}

    //! Get pattern scale. Used if PatternParamsModifierFlags::Scale is set.
    double GetScale() const {return scale;}

    //! Get pattern choord tolerance. Used if PatternParamsModifierFlags::Tolerance is set.
    double GetTolerance() const {return tolerance;}

    //! Get pattern cell element id. Used if PatternParamsModifierFlags::Cell is set.
    DgnElementId GetCellId() const {return DgnElementId((uint64_t)cellId);}

    //! Get pattern min multiline boundary index. Used if PatternParamsModifierFlags::Multiline is set.
    int32_t GetMinLine() const {return minLine;}

    //! Get pattern max multiline boundary index. Used if PatternParamsModifierFlags::Multiline is set.
    int32_t GetMaxLine() const {return maxLine;}

    //! Get pattern color. Used if PatternParamsModifierFlags::Color is set.
    //! @note Uses element color if not set. Ignored for area pattern using a graphic cell not a point cell.
    ColorDef GetColor() const {return color;}

    //! Get pattern weight. Used if PatternParamsModifierFlags::Weight is set.
    //! @note Uses element weight if not set. Ignored for area pattern using a graphic cell not a point cell.
    uint32_t GetWeight() const {return weight;}

    //! Get pattern line style. Used if PatternParamsModifierFlags::Style is set.
    //! @note Uses element line style if not set. Ignored for area pattern using a graphic cell not a point cell.
    //! @note Only line styles 0-7 are supported.
    int32_t GetStyle() const {return style;}

    //! Get pattern dwg hole style. Used if PatternParamsModifierFlags::HoleStyle is set.
    PatternParamsHoleStyleType GetHoleStyle() const {return static_cast<PatternParamsHoleStyleType>(holeStyle);}

    //! Get pattern origin. Used if PatternParamsModifierFlags::Origin is set.
    //! @note Not supported, pattern origin stored as offset from element origin.
    DPoint3dCR GetOrigin() const {return origin;}

    //! Get annotation scale. Used if PatternParamsModifierFlags::AnnotationScale is set.
    //! @note If PatternParamsModifierFlags::AnnotationScale is set, this pattern is an 'annotative' element, which typically synchronizes its scale with model's annotation scale.
    double GetAnnotationScale() const {return annotationscale;}

    //! Set the pattern modifiers bit field.
    void SetModifiers(PatternParamsModifierFlags value) {modifiers = value;}

    //! Set pattern orientation. Sets modifier bit for PatternParamsModifierFlags::RotMatrix.
    void SetOrientation(RotMatrixCR value) {rMatrix = value; modifiers = modifiers |PatternParamsModifierFlags::RotMatrix;}

    //! Set pattern offset from element origin. Sets modifier bit for PatternParamsModifierFlags::Offset.
    void SetOffset(DPoint3dCR value) {offset = value; modifiers = modifiers |PatternParamsModifierFlags::Offset;}

    //! Set pattern hatch spacing or area pattern row spacing. Sets modifier bit for PatternParamsModifierFlags::Space1.
    void SetPrimarySpacing(double value) {space1 = value; modifiers = modifiers |PatternParamsModifierFlags::Space1;}

    //! Set pattern hatch angle or area pattern angle. Sets modifier bit for PatternParamsModifierFlags::Angle1.
    void SetPrimaryAngle(double value) {angle1 = value; modifiers = modifiers |PatternParamsModifierFlags::Angle1;}

    //! Set pattern cross hatch spacing or area pattern column spacing. Sets modifier bit for PatternParamsModifierFlags::Space2.
    void SetSecondarySpacing(double value) {space2 = value; modifiers = modifiers |PatternParamsModifierFlags::Space2;}

    //! Set pattern cross hatch angle. Sets modifier bit for PatternParamsModifierFlags::Angle2.
    void SetSecondaryAngle(double value) {angle2 = value; modifiers = modifiers |PatternParamsModifierFlags::Angle2;}

    //! Set pattern pattern scale. Sets modifier bit for PatternParamsModifierFlags::Scale.
    void SetScale(double value) {scale = value; modifiers = modifiers |PatternParamsModifierFlags::Scale;}

    //! Set pattern choord tolerance. Sets modifier bit for PatternParamsModifierFlags::Tolerance.
    void SetTolerance(double value) {tolerance = value; modifiers = modifiers |PatternParamsModifierFlags::Tolerance;}

    //! Set pattern cell by supplying the element id of an existing shared cell definition. Sets modifier bit for PatternParamsModifierFlags::Cell.
    void SetCellId(DgnElementId value) {cellId = value.GetValue(); modifiers = modifiers |PatternParamsModifierFlags::Cell;}

    //! Set pattern min multiline boundary index. Sets modifier bit for PatternParamsModifierFlags::Multiline. Valid only for multiline elements.
    void SetMinLine(int32_t value) {minLine = value; modifiers = modifiers |PatternParamsModifierFlags::Multiline;}

    //! Set pattern max multiline boundary index. Sets modifier bit for PatternParamsModifierFlags::Multiline. Valid only for multiline elements.
    void SetMaxLine(int32_t value) {maxLine = value; modifiers = modifiers |PatternParamsModifierFlags::Multiline;}

    //! Set pattern color. Sets modifier bit for PatternParamsModifierFlags::Color.
    //! @note Use to set a pattern color that is different than element color. Ignored for area pattern using a graphic cell not a point cell.
    void SetColor(ColorDef value) {color = value; modifiers = modifiers |PatternParamsModifierFlags::Color;}

    //! Set pattern weight. Sets modifier bit for PatternParamsModifierFlags::Weight.
    //! @note Use to set a pattern weight that is different than element weight. Ignored for area pattern using a graphic cell not a point cell.
    void SetWeight(uint32_t value) {weight = value; modifiers = modifiers |PatternParamsModifierFlags::Weight;}

    //! Set pattern line style. Sets modifier bit for PatternParamsModifierFlags::Style.
    //! @note Use to set a pattern line style that is different than element line style. Ignored for area pattern using a graphic cell not a point cell.
    //! @note Only line styles 0-7 are supported.
    void SetStyle(int32_t value) {style = value; modifiers = modifiers |PatternParamsModifierFlags::Style;}

    //! Set pattern dwg hole style. Sets modifier bit for PatternParamsModifierFlags::HoleStyle.
    void SetHoleStyle(PatternParamsHoleStyleType value) {holeStyle = static_cast<int16_t>(value); modifiers = modifiers |PatternParamsModifierFlags::HoleStyle;}

    //! Set pattern origin. Sets modifier bit for PatternParamsModifierFlags::Origin.
    //! @note Not supported, define pattern origin relative to element origin using SetOffset instead.
    void SetOrigin(DPoint3dCR value) {origin = value; modifiers = modifiers |PatternParamsModifierFlags::Origin;}

    //! Set annotation scale. Sets modifier bit for PatternParamsModifierFlags::AnnotationScale.
    //! @note This is optional. Setting it makes the pattern an 'annotative' element, which typically synchronizes its scale with model's annotation scale.
    void SetAnnotationScale(double scale) {annotationscale = scale; modifiers = modifiers |PatternParamsModifierFlags::AnnotationScale;}
}; // PatternParams

enum PatternPlacementFlags
{
PATTERN_FLAGS_OverideTrueScale  = (1<<0),       //!< If false ignore UseTrueScale flag, use active setting
PATTERN_FLAGS_UseTrueScale      = (1<<1),       //!< Apply true scale
PATTERN_FLAGS_Default           = (0),          //!< Use active setting by default
};

END_BENTLEY_DGN_NAMESPACE

