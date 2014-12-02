/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/IViewDraw.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include    "DgnElements.h"
#include    "ColorUtil.h"
#include    "LineStyleResource.r.h"
#include    "AreaPattern.h"
#include    "ElementHandle.h"
#include    <Bentley/RefCounted.h>

//__PUBLISH_SECTION_END__
DGNPLATFORM_TYPEDEFS (QvBaseMatSym)
DGNPLATFORM_TYPEDEFS (Material)
DGNPLATFORM_TYPEDEFS (Display_attribute_gradient)
DGNPLATFORM_TYPEDEFS (DgnGraphics)
DGNPLATFORM_TYPEDEFS (DgnGraphicsProcessor)
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @private
struct LineStyleParams : LineStyleParamsResource
{
//__PUBLISH_SECTION_END__
    void Init()
        {
        memset (this, 0, sizeof(LineStyleParams));
        this->rMatrix.form3d[0][0] = this->rMatrix.form3d[1][1] = this->rMatrix.form3d[2][2] =
        this->scale = this->gapScale = this->dashScale = this->normal.z = 1.0;
        }

    //! Compare two LineStyleParams.
    DGNPLATFORM_EXPORT bool operator==(LineStyleParamsCR rhs) const;

    DGNPLATFORM_EXPORT void SetScale (double scale);
    DGNPLATFORM_EXPORT bool ApplyTransform (TransformCR transform, bool allowSizeChange = true);

    // NOTE: GetActiveParams is ONLY for use with IEditProperties!!!
    //MSCORE_EXPORT bool GetActiveParams (ViewportP vp /* 3d only */, UInt32 mlineProfileIndex = 0); removed in graphite
//__PUBLISH_SECTION_START__
};

typedef struct LineStyleParams LineStyleParams;

struct  ISprite;
struct  IDisplaySymbol;
struct  IDgnGlyphLayoutListener;
struct  IDgnOleDraw;

enum class DrawExpense
{
    Medium     = 1,        //!<  Average for a element type that may be cached
    High       = 2,        //!<  Cache it unless at risk of exhausting virtual address
};


enum class FillDisplay        //!< Whether an element should be drawn with its internal area filled or not
{
    Never    = 0,      //!< don't fill, even if fill attribute is on for the viewport
    ByView   = 1,      //!< fill the element iff the fill attribute is on for the viewport
    Always   = 2,      //!< always fill the element, even if the fill attribute is off for the viewport
    Blanking = 3,      //!< always fill/always behind geometry that follows
};

#define SCREENING_Full  0.0
#define SCREENING_None  100.0

enum class LineJoin
{
    None    = 0,
    Bevel   = 1,
    Miter   = 2,
    Round   = 3,
};

enum class LineCap
{
    None     = 0,
    Flat     = 1,
    Square   = 2,
    Round    = 3,
    Triangle = 4,
};

enum class RangeResult
{
    Outside = -1,
    Overlap = 0,
    Inside  = 1,
};

enum class RasterFormat
{
    RGBA  = 0,
    BGRA  = 1,
    RGB   = 2,
    BGR   = 3,
    Gray  = 4,
    Alpha = 5,    // not valid for icons
    RGBS  = 6,    // 4 band with alpha stencil (0 or 255 only)
    BGRS  = 7,    // 4 band with alpha stencil (0 or 255 only)
};

typedef RefCountedPtr<GradientSymb> GradientSymbPtr;

//=======================================================================================
//! Parameters defining a gradient
//=======================================================================================
struct GradientSymb : RefCountedBase
{
//__PUBLISH_SECTION_END__
protected:
    GradientMode    m_mode;
    UInt16          m_flags;
    UInt16          m_nKeys;

    double          m_angle;
    double          m_tint;
    double          m_shift;
    RgbColorDef     m_colors[MAX_GRADIENT_KEYS];
    double          m_values[MAX_GRADIENT_KEYS];

    DGNPLATFORM_EXPORT GradientSymb();

public:
    DEFINE_BENTLEY_NEW_DELETE_OPERATORS

DGNPLATFORM_EXPORT void CopyFrom (GradientSymbCR);

DGNPLATFORM_EXPORT StatusInt FromDisplayAttribute (Display_attribute_gradientCP);
DGNPLATFORM_EXPORT StatusInt ToDisplayAttribute (Display_attribute_gradientR) const;

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

//! Create an instance of a GradientSymb.
DGNPLATFORM_EXPORT static GradientSymbPtr Create ();

//! Compare two GradientSymb.
DGNPLATFORM_EXPORT bool operator==(GradientSymbCR rhs) const;

DGNPLATFORM_EXPORT int GetNKeys () const;
DGNPLATFORM_EXPORT GradientMode GetMode () const;
DGNPLATFORM_EXPORT UInt16 GetFlags () const;
DGNPLATFORM_EXPORT double GetShift () const;
DGNPLATFORM_EXPORT double GetTint () const;
DGNPLATFORM_EXPORT double GetAngle () const;
DGNPLATFORM_EXPORT void GetKey (RgbColorDef& color, double& value, int index) const;

DGNPLATFORM_EXPORT void SetMode (GradientMode mode);
DGNPLATFORM_EXPORT void SetFlags (UInt16 flags);
DGNPLATFORM_EXPORT void SetAngle (double angle);
DGNPLATFORM_EXPORT void SetTint (double tint);
DGNPLATFORM_EXPORT void SetShift (double shift);
DGNPLATFORM_EXPORT void SetKeys (UInt16 nKeys, RgbColorDef const* colors, double const* values);

}; // GradientSymb

typedef RefCountedPtr<struct QVAliasMaterialId> QVAliasMaterialIdPtr;

//=======================================================================================
//! If one of the uv mapping modes, Directional Drape, Cubic, Spherical or Cylindrical is to
//! be used for mapping a material to a non persistent element for draw purposes an application maintained Id
//! is required for the qv material. Typically if there is a one to one mapping between
//! material and element then the MaterialCP pointer can be used here.
//=======================================================================================
struct QVAliasMaterialId : RefCountedBase
{
//__PUBLISH_SECTION_END__
private:
uintptr_t       m_qvAliasMaterialId;

QVAliasMaterialId ();
explicit QVAliasMaterialId (uintptr_t qvAliasMaterialId);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Destructor. This will delete the qv material
DGNPLATFORM_EXPORT virtual ~QVAliasMaterialId ();

//! Get the id for the generated Qv Material
DGNPLATFORM_EXPORT uintptr_t GetId () const;

//! Create an instance of this object
//! @param[in] qvAliasMaterialId            Id for generated Qv material
DGNPLATFORM_EXPORT static QVAliasMaterialIdPtr Create (uintptr_t qvAliasMaterialId);
};

#if defined (NEEDS_WORK_MATERIALS)
typedef RefCountedPtr<struct MaterialUVDetail> MaterialUVDetailPtr;

//=======================================================================================
//! Materials which use uv mapping modes Directional Drape, Cubic, Spherical and Cylindrical
//! require additional element based information for their definition. For persistent elements the
//! information is read from the element. For non persistent elements the origin, matrix, size and
//! qv materialId members need to be set
//=======================================================================================
struct MaterialUVDetail : RefCountedBase
{
//__PUBLISH_SECTION_END__
private:

DPoint3d                m_origin;
RotMatrix               m_rMatrix;
DPoint3d                m_size;
QVAliasMaterialIdPtr    m_appQvId;
ElementHandle           m_eh;

MaterialUVDetail ();

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Copy the contents of the argument object into this object
//! @param[in] rhs          Object to copy
DGNPLATFORM_EXPORT void Copy (MaterialUVDetailCR rhs);

//! Compare the contents of this object with the method argument for equality
//! @param[in] rhs          Object to compare
DGNPLATFORM_EXPORT bool Equals (MaterialUVDetailCR rhs);

//! Get the origin of the UV mapping. Only required for mapping non persistent elements
DGNPLATFORM_EXPORT DPoint3dCR   GetOrigin () const;
//! Set the origin on the UV mapping. Only required for mapping non persistent elements
DGNPLATFORM_EXPORT void         SetOrigin (DPoint3dCR origin);

//! Get the orientation of the UV mapping. Only required for mapping non persistent elements
DGNPLATFORM_EXPORT RotMatrixCR  GetRMatrix () const;
//! Set the orientation of the UV mapping. Only required for mapping non persistent elements
DGNPLATFORM_EXPORT void         SetRMatrix (RotMatrixCR rMatrix);

//! Get the size of the UV mapping. Only required for mapping non persistent elements
DGNPLATFORM_EXPORT DPoint3dCR   GetSize () const;
//! Set the size of the UV mapping. Only required for mapping non persistent elements
DGNPLATFORM_EXPORT void         SetSize (DPoint3dCR size);

//! Get the QV Material Id for the UV mapping to use. Only required for mapping non persistent elements
DGNPLATFORM_EXPORT QVAliasMaterialIdCP  GetQVAliasMaterialId  () const;
//! Set the QV Material Id for the UV mapping to use. This Id is ref counted and its lifecycle will need to exist past
//! the draw of the temporary element. Then when the referece count is 0 the destructor of this object will
//! remove the material definition from QuickVision. Only required for mapping non persistent elements
DGNPLATFORM_EXPORT void                 SetQVAliasMaterialId  (QVAliasMaterialIdP qvId); 

//! Get the element which requires UV mapping
DGNPLATFORM_EXPORT ElementHandleCR  GetElementHandle () const;
//! Set the element which requires UV mapping
DGNPLATFORM_EXPORT void             SetElementHandle (ElementHandleCR eh);

//! Create an instance of this class
DGNPLATFORM_EXPORT static MaterialUVDetailPtr Create ();
};
#endif

//__PUBLISH_SECTION_END__

//=======================================================================================
//! This structure holds the display parameters that a header element can override 
//! onto all of its children as they are drawn.
//! See BentleyApi::DgnPlatform::ViewContext::PushOverrides.
//=======================================================================================
struct ElemHeaderOverrides
{
private:

SCOverride          m_flags;
Symbology           m_symb;
LevelId             m_level;
int                 m_levelCodeDiff;
UInt32              m_dispPriority;
DgnElementClass     m_elementClass;
LineStyleParams     m_styleParams;

public:

DGNPLATFORM_EXPORT ElemHeaderOverrides ();
DGNPLATFORM_EXPORT void Init (SCOverride const* ovr, LevelId baseLevel, int levelDiff, UInt32 dispPriority, DgnElementClass elementClass, Symbology const* symb, LineStyleParams const* styleParams);

void MergeFrom (ElemHeaderOverrides const* o1, ElemHeaderOverrides const* o2);

DGNPLATFORM_EXPORT SCOverride GetFlags () const;
DGNPLATFORM_EXPORT LevelId AdjustLevel (LevelId inLevel) const; // Apply overrides and return adjusted level.
DGNPLATFORM_EXPORT LevelId GetLevel () const;
DGNPLATFORM_EXPORT UInt32 GetColor () const;
DGNPLATFORM_EXPORT UInt32 GetWeight () const;
DGNPLATFORM_EXPORT Int32 GetLineStyle () const;
DGNPLATFORM_EXPORT LineStyleParamsCP GetLineStyleParams () const;
DGNPLATFORM_EXPORT UInt32 GetDisplayPriority () const;
DGNPLATFORM_EXPORT DgnElementClass GetElementClass () const;

}; // ElemHeaderOverrides

//__PUBLISH_SECTION_START__
//=======================================================================================
//! This structure holds all of the information about an element specifying the "displayable parameters" of the element.
//! It is typically extracted from the "dhdr" section of the element header and from user data.
// @bsiclass 
//=======================================================================================
struct ElemDisplayParams
{
//__PUBLISH_SECTION_END__
    friend struct ElemDisplayParamsStateSaver;
    friend struct ElemDisplayParamsIgnores;
    friend struct DgnGraphics;
    friend struct DgnGraphicsProcessor;

private:

// NOTE: Constructor uses memset (this, 0, offsetof (ElemDisplayParams, m_styleParams));
bool                m_isRenderable:1;               //!< if true, element is renderable (subject to lighting), so don't do black/white reversal
bool                m_hasThickness:1;               //!< if true, draw with an implied thickness specified by #m_thicknessVector
bool                m_isCapped:1;                   //!< if #m_hasThickness is true, whether or the thickening results in solid or surface
bool                m_hasScreening:1;               //!< if true, output should be screened by value in #m_screening
bool                m_hasLineJoin:1;                //!< if true, use #m_lineJoin to determine line join type.
bool                m_hasLineCap:1;                 //!< if true, use #m_lineCap to determine line cap type.
bool                m_hasLineWeightMM:1;            //!< if true, line weight is specified in millimeters by #m_widthMM
bool                m_ignoreLevelSymb:1;            //!< if true, symbology overrides are not applied (DIMENSIONS only)
bool                m_materialIsAttached:1;         //!< if true, material is from attachment not color/level assignment.
bool                m_isValidLineColorTBGR:1;       //!< Used to assert whether Post-Cook TBGR has been set when ElemMatSymb is being created...
bool                m_isValidFillColorTBGR:1;       //!< Used to assert whether Post-Cook TBGR has been set when ElemMatSymb is being created...

SubLevelId     m_subLevel;                     //!< the SubLevel on which the element is drawn
DgnElementClass     m_elementClass;                 //!< element class
Int32               m_elmPriority;                  //!< display priority (applies to 2d only)
Int32               m_netPriority;                  //!< net display priority for element/level/model (applies to 2d only)
Symbology           m_symbology;                    //!< color, weight, style
UInt32              m_lineColorIndex;               //!< pre-resolved line color index, used to setup ElemMatSymb line color index.
UInt32              m_lineColorTBGR;                //!< In form TBGR
UInt32              m_fillColor;                    //!< fill color (applicable only if filled)
UInt32              m_fillColorIndex;               //!< pre-resolved fill color index, used to setup ElemMatSymb fill color index.
UInt32              m_fillColorTBGR;                //!< In form TBGR
FillDisplay         m_fillDisplay;                  //!< whether or not the element should be displayed filled
double              m_transparency;                 //!< transparency, 1.0 == completely transparent.
DVec3d              m_thicknessVector;              //!< scaled vector that determines thickness size and direction
LineCap             m_lineCap;                      //!< line cap type when m_hasLineCap is set.
LineJoin            m_lineJoin;                     //!< line join type when m_hasLineJoin is set.
double              m_screening;                    //!< screening value when m_hasScreening is set.
double              m_widthMM;                      //!< line width in mm when m_hasLineWeightMM is set.
MaterialCP          m_material;                     //!< render material

LineStyleParams     m_styleParams;                  //!< modifiers for user defined linestyle (if applicable)
GradientSymbPtr     m_gradient;                     //!< gradient fill settings.

public:

DGNPLATFORM_EXPORT ElemDisplayParams ();
DGNPLATFORM_EXPORT explicit ElemDisplayParams (ElemDisplayParamsCR rhs);

DGNPLATFORM_EXPORT void Init ();
DGNPLATFORM_EXPORT bool ResolveByLevel (DgnLevels::SubLevel::Appearance const&); // Set effective values for BYLEVEL properties.
DGNPLATFORM_EXPORT bool ResolveByCell (ElemHeaderOverridesCP, DgnLevels::SubLevel::Appearance const&); // Set effective values for BYCELL properties and relative levels.
DGNPLATFORM_EXPORT void ResolveColorTBGR (ViewContextR); // Resolve effective display TBGR for override matSymb.
DGNPLATFORM_EXPORT bool ApplyParentOverrides (ElemHeaderOverridesCP ovr); // Set header override values.

DGNPLATFORM_EXPORT void SetSubLevelId (SubLevelId);
DGNPLATFORM_EXPORT void SetLineStyle (Int32 style, LineStyleParamsCP params = NULL);
DGNPLATFORM_EXPORT void SetWeight (UInt32 weight);
DGNPLATFORM_EXPORT void SetElementClass (DgnElementClass elmClass);
DGNPLATFORM_EXPORT void SetTransparency (double transparency);
DGNPLATFORM_EXPORT void SetThickness (DVec3dCP direction, bool isCapped);

DGNPLATFORM_EXPORT void SetLineColor (UInt32 elementColor);
DGNPLATFORM_EXPORT void SetLineColorTBGR (UInt32 colorTBGR);

DGNPLATFORM_EXPORT void SetFillColor (UInt32 elementColor);
DGNPLATFORM_EXPORT void SetFillColorTBGR (UInt32 colorTBGR);
DGNPLATFORM_EXPORT void SetFillDisplay (FillDisplay display);
DGNPLATFORM_EXPORT void SetGradient (GradientSymbP gradient);

DGNPLATFORM_EXPORT UInt32 GetLineColorIndex () const;       // INTERNAL USE ONLY! Used to setup ElemMatSymb line color index.
DGNPLATFORM_EXPORT void   SetLineColorIndex (UInt32 index); // INTERNAL USE ONLY! Used to setup ElemMatSymb line color index.
DGNPLATFORM_EXPORT UInt32 GetFillColorIndex () const;       // INTERNAL USE ONLY! Used to setup ElemMatSymb fill color index.
DGNPLATFORM_EXPORT void   SetFillColorIndex (UInt32 index); // INTERNAL USE ONLY! Used to setup ElemMatSymb fill color index.

DGNPLATFORM_EXPORT bool IsAttachedMaterial () const;
DGNPLATFORM_EXPORT void SetMaterial (MaterialCP material, bool isAttached = false);
DGNPLATFORM_EXPORT void SetPatternParams (PatternParamsPtr& patternParams);

DGNPLATFORM_EXPORT bool IsRenderable () const;
DGNPLATFORM_EXPORT void SetIsRenderable (bool isRenderable);

DGNPLATFORM_EXPORT bool IsLevelSymbIgnored () const;
DGNPLATFORM_EXPORT void SetLevelSymbIgnored (bool isIgnored);

DGNPLATFORM_EXPORT void   SetElementDisplayPriority (Int32 priority, bool is3d, DgnLevels::SubLevel::Appearance const* appearance); // Set display priority and net priority for direct draws of 2d geometry not extracted from elements using CalcDefaultNetDisplayPriority and level from ElemDisplayParams.
DGNPLATFORM_EXPORT void   SetNetDisplayPriority (Int32 priority); // RASTER USE ONLY!!!
DGNPLATFORM_EXPORT Int32  GetNetDisplayPriority () const; // Get net display priority (2d only).
DGNPLATFORM_EXPORT static Int32 CalcDefaultNetDisplayPriority (Int32 priority, DgnLevels::SubLevel::Appearance const* appearance);

DGNPLATFORM_EXPORT bool     IsScreeningSet () const;
DGNPLATFORM_EXPORT double   GetScreening () const;
DGNPLATFORM_EXPORT void     SetScreening (double screening, bool set = true);

DGNPLATFORM_EXPORT bool     IsLineJoinSet () const;
DGNPLATFORM_EXPORT LineJoin GetLineJoin () const;
DGNPLATFORM_EXPORT void     SetLineJoin (LineJoin join, bool set = true);

DGNPLATFORM_EXPORT bool     IsLineCapSet () const;
DGNPLATFORM_EXPORT LineCap  GetLineCap () const;
DGNPLATFORM_EXPORT void     SetLineCap (LineCap cap, bool set = true);

DGNPLATFORM_EXPORT bool     IsLineWeightMMSet () const;
DGNPLATFORM_EXPORT double   GetLineWeightMM () const;
DGNPLATFORM_EXPORT void     SetLineWeightMM (double weight, bool set = true);

DGNPLATFORM_EXPORT void Resolve (ViewContextR); // Resolve effective values and set TBGR for indexed colors.

SubLevelId GetSubLevelId() const {return m_subLevel;}

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

//! Compare two ElemDisplayParam.
DGNPLATFORM_EXPORT bool operator==(ElemDisplayParamsCR rhs) const;
//! copy operator
DGNPLATFORM_EXPORT ElemDisplayParamsR operator=(ElemDisplayParamsCR rhs);

//! Get element color id. NOTE: Will be INVALID_COLOR when color has been defined by TBGR value.
DGNPLATFORM_EXPORT UInt32 GetLineColor () const;

//! Get element color that has been defined by TBGR value. Valid when INVALID_COLOR == GetLineColor ().
DGNPLATFORM_EXPORT UInt32 GetLineColorTBGR () const;

//! Helper method, checks INVALID_COLOR == GetLineColor ().
DGNPLATFORM_EXPORT bool IsLineColorTBGR () const;

//! Get element fill color id. Valid when INVALID_COLOR != GetFillColor () && FillDisplay::Never != GetFillDisplay () && NULL == GetGradient ().
DGNPLATFORM_EXPORT UInt32 GetFillColor () const;

//! Get element fill color that has been defined by TBGR value. Valid when INVALID_COLOR == GetFillColor () && FillDisplay::Never != GetFillDisplay () && NULL == GetGradient ().
DGNPLATFORM_EXPORT UInt32 GetFillColorTBGR () const;

//! Helper method, checks INVALID_COLOR == GetFillColor () && FillDisplay::Never != GetFillDisplay () && NULL == GetGradient ().
DGNPLATFORM_EXPORT bool IsFillColorTBGR () const;

//! Get fill display setting.
DGNPLATFORM_EXPORT FillDisplay GetFillDisplay () const;

//! Get gradient fill information. Valid when NULL != GetGradient () && FillDisplay::Never != GetFillDisplay ().
DGNPLATFORM_EXPORT GradientSymbCP GetGradient () const;

//! Get element linestyle.
DGNPLATFORM_EXPORT Int32 GetLineStyle () const;

//! Get element linestyle modifiers.
DGNPLATFORM_EXPORT LineStyleParamsCP GetLineStyleParams () const;

//! Get element weight.
DGNPLATFORM_EXPORT UInt32 GetWeight () const;

//! Get element class.
DGNPLATFORM_EXPORT DgnElementClass GetElementClass () const;

//! Get element transparency.
DGNPLATFORM_EXPORT double GetTransparency () const;

//! Get render material.
DGNPLATFORM_EXPORT MaterialCP GetMaterial () const;

//! Get element display priority (2d only).
DGNPLATFORM_EXPORT Int32 GetElementDisplayPriority () const;

//! Get element extrude thickness.
DGNPLATFORM_EXPORT DVec3dCP GetThickness (bool& isCapped) const;

}; // ElemDisplayParams

//__PUBLISH_SECTION_END__
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ElemDisplayParamsStateSaver
{
private:

ElemDisplayParamsR  m_elParams;
ElemDisplayParams   m_elParamsSaved;

bool                m_restoreLevel;
bool                m_restoreLineColor;
bool                m_restoreFillColor;
bool                m_restoreLineStyle;
bool                m_restoreWeight;

public:

//! Class to help in cases where part of the ElemDisplayParams shouldn't be allowed to change.
DGNPLATFORM_EXPORT ElemDisplayParamsStateSaver (ElemDisplayParamsR elParams, bool restoreLevel, bool restoreLineColor, bool restoreFillColor, bool restoreLineStyle, bool restoreWeight);
DGNPLATFORM_EXPORT ~ElemDisplayParamsStateSaver ();

}; // ElemDisplayParamsStateSaver

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ElemDisplayParamsIgnores
{
private:

bool                m_ignoreSubLevel;
bool                m_ignoreColor;
bool                m_ignoreWeight;
SubLevelId     m_subLevel;
UInt32              m_lineColor;
UInt32              m_lineColorIndex;
UInt32              m_lineColorTBGR;
bool                m_isValidLineColorTBGR;

UInt32              m_weight;

public:

//! Class to help with DrawSymbol where symbol element properties are ignored and are inherited from the base element.
DGNPLATFORM_EXPORT ElemDisplayParamsIgnores ();

DGNPLATFORM_EXPORT void Set (ElemDisplayParamsCR elParams, bool ignoreSubLevel, bool ignoreColor, bool ignoreWeight);
DGNPLATFORM_EXPORT void Apply (ElemDisplayParamsR elParams);
DGNPLATFORM_EXPORT void Clear ();

}; // ElemDisplayParamsIgnores

//__PUBLISH_SECTION_START__
//=======================================================================================
//! This structure contains options (modifications) that can be applied
//! to existing line styles to change their appearance without changing the line style
//! definition. Most of the options pertain to the operation of the StrokePatternComponent
//! component but the plane definition and scale factors can be used by all components.
//=======================================================================================
struct          LineStyleSymb
{
private:
    friend struct QvBaseMatSym;

    // NOTE: For performance, the constructor initializes members using:
    //         memset (&m_lStyle, 0, offsetof (LineStyleSymb, m_planeByRows)- offsetof (LineStyleSymb, m_lStyle));
    //         So it will be necessary to update it if first/last member are changed. */

    ILineStyleCP    m_lStyle;       // if NULL, no linestyle active
    struct
        {
        UInt32         scale:1;
        UInt32         dashScale:1;
        UInt32         gapScale:1;
        UInt32         orgWidth:1;
        UInt32         endWidth:1;
        UInt32         phaseShift:1;
        UInt32         autoPhase:1;
        UInt32         maxCompress:1;
        UInt32         iterationLimit:1;
        UInt32         treatAsSingleSegment:1;
        UInt32         plane:1;
        UInt32         cosmetic:1;
        UInt32         centerPhase:1;
        UInt32         xElemPhaseSet:1;
        UInt32         startTangentSet:1;
        UInt32         endTangentSet:1;
        UInt32         elementIsClosed:1;
        UInt32         continuationXElems:1;
        UInt32         isCurve:1;
        } m_options;

    int             m_nIterate;
    double          m_scale;
    double          m_dashScale;
    double          m_gapScale;
    double          m_orgWidth;
    double          m_endWidth;
    double          m_phaseShift;
    double          m_autoPhase;
    double          m_maxCompress;
    double          m_totalLength;      // length of entire element.
    double          m_xElemPhase;       // where we left off from last element (for compound elements)
    DPoint3d        m_startTangent;
    DPoint3d        m_endTangent;
    RotMatrix       m_planeByRows;

public:
    DGNPLATFORM_EXPORT LineStyleSymb ();

    DGNPLATFORM_EXPORT int FromResolvedElemDisplayParams (ElemDisplayParamsCR, ViewContextR context, DPoint3dCP, DPoint3dCP);
    DGNPLATFORM_EXPORT int FromNaturalElemDisplayParams (ElemDisplayParamsR, ViewContextR context, DPoint3dCP, DPoint3dCP);
    DGNPLATFORM_EXPORT int FromResolvedStyle (Int32 styleNo, LineStyleParamsCP lStyleParams, ViewContextR context, DPoint3dCP startTangent, DPoint3dCP endTangent);

    void               Clear () {m_lStyle = NULL; m_options.orgWidth = m_options.endWidth = false;}
    void               Init (ILineStyleCP);

public:
DGNPLATFORM_EXPORT ILineStyleCP  GetILineStyle () const;
DGNPLATFORM_EXPORT void          GetPlaneAsMatrixRows (RotMatrixR matrix) const;
DGNPLATFORM_EXPORT double        GetScale () const;
DGNPLATFORM_EXPORT double        GetDashScale () const;
DGNPLATFORM_EXPORT double        GetGapScale () const;
DGNPLATFORM_EXPORT double        GetOriginWidth () const;
DGNPLATFORM_EXPORT double        GetEndWidth () const;
DGNPLATFORM_EXPORT double        GetPhaseShift () const;
DGNPLATFORM_EXPORT double        GetFractionalPhase () const;
DGNPLATFORM_EXPORT double        GetMaxCompress () const;
DGNPLATFORM_EXPORT int           GetNumIterations () const;
DGNPLATFORM_EXPORT double        GetMaxWidth () const;
DGNPLATFORM_EXPORT double        GetTotalLength () const;
DGNPLATFORM_EXPORT DPoint3dCP    GetStartTangent () const;
DGNPLATFORM_EXPORT DPoint3dCP    GetEndTangent () const;
DGNPLATFORM_EXPORT bool          IsScaled () const;
DGNPLATFORM_EXPORT bool          IsAutoPhase () const;
DGNPLATFORM_EXPORT bool          IsCenterPhase () const;
DGNPLATFORM_EXPORT bool          IsCosmetic () const;
DGNPLATFORM_EXPORT bool          IsTreatAsSingleSegment () const;
DGNPLATFORM_EXPORT bool          HasDashScale () const;
DGNPLATFORM_EXPORT bool          HasGapScale () const;
DGNPLATFORM_EXPORT bool          HasOrgWidth () const;
DGNPLATFORM_EXPORT bool          HasEndWidth () const;
DGNPLATFORM_EXPORT bool          IsElementClosed () const;
DGNPLATFORM_EXPORT bool          IsCurve () const;
DGNPLATFORM_EXPORT bool          HasPhaseShift () const;
DGNPLATFORM_EXPORT bool          HasIterationLimit () const;
DGNPLATFORM_EXPORT bool          HasPlane () const;
DGNPLATFORM_EXPORT bool          HasStartTangent () const;
DGNPLATFORM_EXPORT bool          HasEndTangent () const;
DGNPLATFORM_EXPORT void          SetPlaneAsMatrixRows (RotMatrixCP);
DGNPLATFORM_EXPORT void          SetNormalVec (DPoint3dCP);
DGNPLATFORM_EXPORT void          SetOriginWidth (double width);
DGNPLATFORM_EXPORT void          SetEndWidth (double width);
DGNPLATFORM_EXPORT void          SetWidth (double width);
DGNPLATFORM_EXPORT void          SetScale (double scaleFactor);
DGNPLATFORM_EXPORT void          SetFractionalPhase (bool isOn, double fraction);
DGNPLATFORM_EXPORT void          SetCenterPhase (bool isOn);
DGNPLATFORM_EXPORT void          SetPhaseShift (bool isOn, double distance);
DGNPLATFORM_EXPORT void          SetTreatAsSingleSegment(bool yesNo);
DGNPLATFORM_EXPORT void          SetTangents (DPoint3dCP, DPoint3dCP);
DGNPLATFORM_EXPORT void          SetLineStyle (ILineStyleCP lstyle);

bool HasTrueWidth() const  {return HasOrgWidth () || HasEndWidth();}
bool HasMaxCompress () const {return m_options.maxCompress;}
bool ContinuationXElems () const {return m_options.continuationXElems;}
void SetXElemPhase (double last) {m_xElemPhase = last; m_options.xElemPhaseSet=true;}
void SetElementClosed (bool closed) {m_options.elementIsClosed = closed;}
void SetIsCurve (bool isCurve) {m_options.isCurve = isCurve;}

DGNPLATFORM_EXPORT void SetGapScale (double scaleFactor);
DGNPLATFORM_EXPORT void SetDashScale (double scaleFactor);
DGNPLATFORM_EXPORT void SetTotalLength (double);
DGNPLATFORM_EXPORT void SetCosmetic (bool cosmetic);
DGNPLATFORM_EXPORT void ClearContinuationData ();
DGNPLATFORM_EXPORT void CheckContinuationData ();

}; // LineStyleSymb

//=======================================================================================
//! DgnCore implements this class for setting/getting the Material and Symbology (ElemMatSymb) used to draw geometry.
//! An ElemMatSymb is the "cooked" material and symbology that determines the real appearance (e.g. color/width/raster pattern/linestyle,
//! etc.) used to draw all geometry.
//!
//! The processing of "cooking" an ElemDisplayParams into an ElemMatSymb involves resolving color table indices (if color is indexed),
//! applying line weight to width mappings, resolving custom line styles, etc.
//! For example, an ElemMatSymb stores the full RGBA values for color and fill color, versus the index into a color table
//! stored in ElemDisplayParams.
//!
//! Viewports always have an "active" ElemMatSymb.
//=======================================================================================
struct  ElemMatSymb
{
//__PUBLISH_SECTION_END__
private:
   
    UInt32              m_lineColorTBGR;
    UInt32              m_fillColorTBGR;
    int                 m_lineColorIndex;
    int                 m_fillColorIndex;
    int                 m_elementStyle;
    bool                m_isFilled;
    bool                m_isBlankingRegion;
    uintptr_t           m_extSymbID;
    MaterialCP          m_material;
    UInt32              m_rasterWidth;
    UInt32              m_rasterPat;
    LineStyleSymb       m_lStyleSymb;
    GradientSymbPtr     m_gradient;
    PatternParamsPtr    m_patternParams;

public:

DGNPLATFORM_EXPORT ElemMatSymb ();
DGNPLATFORM_EXPORT explicit ElemMatSymb (ElemMatSymbCR rhs);

DGNPLATFORM_EXPORT void Init ();
DGNPLATFORM_EXPORT void Init (ViewContextR, UInt32 lineColorIndex, UInt32 fillColorIndex, UInt32 lineWeight, Int32 lineStyle);

//! INTERNAL USE ONLY: Should only ever be called by sub-classes of ViewContext, use CookDisplayParams instead!
DGNPLATFORM_EXPORT void FromResolvedElemDisplayParams (ElemDisplayParamsCR, ViewContextR, DPoint3dCP startTan, DPoint3dCP endTan);
DGNPLATFORM_EXPORT void FromNaturalElemDisplayParams (ElemDisplayParamsR, ViewContextR, DPoint3dCP startTan, DPoint3dCP endTan);

// Get the element style.
int GetElementStyle () {return m_elementStyle;}

//! Get the extended material ID from this ElemMatSymb
DGNPLATFORM_EXPORT uintptr_t GetExtSymbId () const;

//! Set the extended material ID for this ElemMatSymb
DGNPLATFORM_EXPORT void SetExtSymbId (uintptr_t extSymbID);

//! Set the gradient symbology
DGNPLATFORM_EXPORT void SetGradient (GradientSymbP gradient);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
//! @name Query Methods
//@{

//! Compare two ElemMatSymb.
DGNPLATFORM_EXPORT bool operator==(ElemMatSymbCR rhs) const;

//! copy operator
DGNPLATFORM_EXPORT ElemMatSymbR operator=(ElemMatSymbCR rhs);

//! Get the TBGR line color from this ElemMatSymb
DGNPLATFORM_EXPORT UInt32 GetLineColorTBGR () const;

//! Get the line color index from this ElemMatSymb (DgnColorMap::INDEX_Invalid returned for TBGR color)
DGNPLATFORM_EXPORT int GetLineColorIndex () const;

//! Get the TBGR fill color from this ElemMatSymb.
DGNPLATFORM_EXPORT UInt32 GetFillColorTBGR () const;

//! Get the fill color index from this ElemMatSymb (DgnColorMap::INDEX_Invalid returned for TBGR color)
DGNPLATFORM_EXPORT int GetFillColorIndex () const;

//! Get the width in pixels from this ElemMatSymb.
DGNPLATFORM_EXPORT UInt32 GetWidth () const;

//! Determine whether TrueWidth is on for this ElemMatSymb
DGNPLATFORM_EXPORT bool HasTrueWidth () const;

//! Get the raster pattern from this ElemMatSymb. The raster pattern is a 32 bit mask that is
//! repeated along geometry. For each bit that is on in the pattern, a pixel is set to the line color.
DGNPLATFORM_EXPORT UInt32 GetRasterPattern () const;

//! Get the style index from this ElemMatSymb (INVALID_STYLE returned for raster patterns not derived from line codes)
DGNPLATFORM_EXPORT Int32 GetRasterPatternIndex () const;

//! Determine whether the fill flag is on for this ElemMatSymb.
DGNPLATFORM_EXPORT bool IsFilled () const;

//! Determine whether the fill represents blanking region.
DGNPLATFORM_EXPORT bool IsBlankingRegion () const;

//! Get the LineStyleSymb from this ElemMatSymb.
DGNPLATFORM_EXPORT LineStyleSymbCR GetLineStyleSymb () const;

//! Get the GradientSymb from this ElemMatSymb.
DGNPLATFORM_EXPORT GradientSymbCP GetGradientSymb () const;

//! Get the render material.
DGNPLATFORM_EXPORT MaterialCP GetMaterial () const;

//! Get the area pattern params.
DGNPLATFORM_EXPORT PatternParamsCP GetPatternParams () const;


//@}

//! @name Set Methods
//@{

//! Set the current line color for this ElemMatSymb.
//! @param[in] lineColor   the new TBGR line color for this ElemMatSymb.
DGNPLATFORM_EXPORT void SetLineColorTBGR (UInt32 lineColor);

//! Set the current line color by color map index for this ElemMatSymb.
//! @param[in] index       the new color map index.
//! @param[in] lineColor   the TBGR color for this color index.
DGNPLATFORM_EXPORT void SetIndexedLineColorTBGR (int index, UInt32 lineColor);

//! Set the current fill color for this ElemMatSymb.
//! @param[in] fillColor   the new TBGR fill color for this ElemMatSymb.
DGNPLATFORM_EXPORT void SetFillColorTBGR (UInt32 fillColor);

//! Set the current fill color by color map index for this ElemMatSymb.
//! @param[in] index       the new color map index.
//! @param[in] fillColor   the TBGR color for this color index.
DGNPLATFORM_EXPORT void SetIndexedFillColorTBGR (int index, UInt32 fillColor);

//! Turn on or off the fill flag for this ElemMatSymb.
//! @param[in] filled      if true, the interior of elements drawn using this ElemMatSymb will be filled using the fill color.
DGNPLATFORM_EXPORT void SetIsFilled (bool filled);

//! Set that fill is always behind other geometry.
DGNPLATFORM_EXPORT void SetIsBlankingRegion (bool blanking);

//! Set the width in pixels for this ElemMatSymb.
//! @param[in] rasterWidth the width in pixels of lines drawn using this ElemMatSymb.
//! @note         If either TrueWidthStart or TrueWidthEnd are non-zero, this value is ignored.
DGNPLATFORM_EXPORT void SetWidth (UInt32 rasterWidth);

//! Set the raster pattern for this ElemMatSymb.
//! @param[in] rasterPat   the 32 bit on-off pattern to be repeated along lines drawn using this ElemMatSymb.
//! @see          #GetRasterPattern
DGNPLATFORM_EXPORT void SetRasterPattern (UInt32 rasterPat);

//! Set a raster pattern derived from a line code for this ElemMatSymb. Used to support plotting of cosmetic line styles mapped to line codes.
//! @param[in] index       the new line style code.
//! @param[in] rasterPat   the 32 bit on-off pattern to be repeated along lines drawn using this ElemMatSymb.
//! @see          #GetRasterPattern #GetRasterPatternIndex
DGNPLATFORM_EXPORT void SetIndexedRasterPattern (Int32 index, UInt32 rasterPat);

//! Get the LineStyleSymb from this ElemMatSymb for setting line style parameters.
DGNPLATFORM_EXPORT LineStyleSymbR GetLineStyleSymbR ();

//! Set the render material. NOTE: You must supply a seed context to support geometry maps!
DGNPLATFORM_EXPORT void SetMaterial (MaterialCP, ViewContextP seedContext = NULL);

//! Set area patterning parameters.
DGNPLATFORM_EXPORT void         SetPatternParams (PatternParamsPtr& patternParams);

//@}
}; // ElemMatSymb

//=======================================================================================
// @bsiclass
//=======================================================================================
enum OvrMatSymbFlags //! flags to indicate the parts of a MatSymb that are to be overridden
{
    MATSYMB_OVERRIDE_None                   = (0),      //!< no overrides
    MATSYMB_OVERRIDE_Color                  = (1<<0),   //!< override outline color
    MATSYMB_OVERRIDE_ColorTransparency      = (1<<1),   //!< override outline color transparency
    MATSYMB_OVERRIDE_FillColor              = (1<<2) | (1<<31), //!< override fill color, override blanking fill with bg color
    MATSYMB_OVERRIDE_FillColorTransparency  = (1<<3),   //!< override fill color transparency
    MATSYMB_OVERRIDE_RastWidth              = (1<<4),   //!< override raster width
    MATSYMB_OVERRIDE_Style                  = (1<<5),   //!< override style
    MATSYMB_OVERRIDE_TrueWidth              = (1<<6),   //!< override true width
    MATSYMB_OVERRIDE_ExtSymb                = (1<<7),   //!< override extended symbology
    MATSYMB_OVERRIDE_RenderMaterial         = (1<<8),   //!< override render material
    MATSYMB_OVERRIDE_PatternParams          = (1<<9),   //!< override (area) pattern params.
    // The proxy flags are informational but do not effect the display.
    MATSYMB_OVERRIDE_IsProxy                = (1<<16),   //!< is proxy              
    MATSYMB_OVERRIDE_IsProxyHidden          = (1<<17),   //!< is proxy edge
    MATSYMB_OVERRIDE_IsProxyEdge            = (1<<18),   //!< is proxy hidden
    MATSYMB_OVERRIDE_IsProxyUnderlay        = (1<<19),  //!< proxy underlay.
    MATSYMB_OVERRIDE_ProxyFlags             = (0xf << 16)  //!< all proxy flags
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct OvrMatSymb
{
//__PUBLISH_SECTION_END__
private:
    UInt32          m_flags;
    ElemMatSymb     m_matSymb;

public:
    DGNPLATFORM_EXPORT OvrMatSymb () : m_flags (MATSYMB_OVERRIDE_None) {}

    ElemMatSymbCR   GetMatSymb () const {return m_matSymb;}
    ElemMatSymbR    GetMatSymbR () {return m_matSymb;}

    DGNPLATFORM_EXPORT uintptr_t GetExtSymbId () const;
    DGNPLATFORM_EXPORT void SetExtSymbId (uintptr_t extSymbID);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

    //! Compare two OvrMatSymb.
    DGNPLATFORM_EXPORT bool operator==(OvrMatSymbCR rhs) const;

    DGNPLATFORM_EXPORT UInt32       GetFlags () const;
    DGNPLATFORM_EXPORT UInt32       GetLineColorTBGR () const;
    DGNPLATFORM_EXPORT UInt32       GetFillColorTBGR () const;
    DGNPLATFORM_EXPORT int          GetLineColorIndex () const;
    DGNPLATFORM_EXPORT int          GetFillColorIndex () const;
    DGNPLATFORM_EXPORT UInt32       GetWidth () const;
    DGNPLATFORM_EXPORT UInt32       GetRasterPattern () const;
    DGNPLATFORM_EXPORT Int32        GetRasterPatternIndex () const;
    DGNPLATFORM_EXPORT MaterialCP   GetMaterial () const;
    DGNPLATFORM_EXPORT PatternParamsCP GetPatternParams () const;

    DGNPLATFORM_EXPORT void         Clear ();
    DGNPLATFORM_EXPORT void         SetFlags (UInt32 flags);
    DGNPLATFORM_EXPORT void         SetLineColorTBGR (UInt32 lineColor);
    DGNPLATFORM_EXPORT void         SetFillColorTBGR (UInt32 fillColor);
    DGNPLATFORM_EXPORT void         SetIndexedLineColorTBGR (int index, UInt32 lineColor);
    DGNPLATFORM_EXPORT void         SetIndexedFillColorTBGR (int index, UInt32 fillColor);
    DGNPLATFORM_EXPORT void         SetTransparentLineColor (UInt32 transparency);
    DGNPLATFORM_EXPORT void         SetTransparentFillColor (UInt32 transparency);
    DGNPLATFORM_EXPORT void         SetWidth (UInt32 rasterWidth);
    DGNPLATFORM_EXPORT void         SetRasterPattern (UInt32 rasterPat);
    DGNPLATFORM_EXPORT void         SetIndexedRasterPattern (Int32 index, UInt32 rasterPat);
    DGNPLATFORM_EXPORT void         SetMaterial (MaterialCP, ViewContextP = NULL);    
    DGNPLATFORM_EXPORT void         SetPatternParams (PatternParamsPtr& patternParams);
    DGNPLATFORM_EXPORT void         SetProxy (bool isEdge, bool isHidden);
    DGNPLATFORM_EXPORT bool         GetProxy (bool& edge, bool& hidden);
    DGNPLATFORM_EXPORT void         SetUnderlay ();
    DGNPLATFORM_EXPORT void         SetLineStyle (Int32 styleNo, DgnModelR modelRef, DgnModelR styleDgnModel, LineStyleParamsCP lStyleParams, ViewContextR context, DPoint3dCP startTangent, DPoint3dCP endTangent);

}; // OvrMatSymb

//=======================================================================================
// @bsiclass
//=======================================================================================
struct IPointCloudDrawParams
{
    enum VersionNumber
    {
        IPointCloudDrawParams_InitialVersion = 1
    };

    //  If IsThreadBound returns false, implement AddRef and Release using InterlockedIncrement
    virtual UInt32 AddRef () = 0;
    virtual UInt32 Release () = 0;
    virtual bool IsThreadBound () = 0; // If true, always executed in calling thread instead of QV thread
    virtual VersionNumber GetVersion () { return IPointCloudDrawParams_InitialVersion; }
    virtual bool GetRange (DPoint3dP range) = 0; // returns false if it does not have range

    //  Added to points returned by GetPoints or GetFPoints
    virtual bool GetOrigin (DPoint3dP origin) = 0; // returns false if no origin

    virtual RgbColorDef const* GetRgbColors () = 0; // Returns NULL if not using colors

    virtual UInt32 GetNumPoints () = 0;
    virtual DPoint3dCP GetDPoints () = 0; // Returns NULL if using floats
    virtual FPoint3dCP GetFPoints () = 0; // Returns NULL if using doubles
};

//=======================================================================================
//! This interface
// defines the method used by ViewContext::DrawCached.
// @bsiclass 
//=======================================================================================
struct IStrokeForCache
{
//! Stroke this object to create a cached representation. This method is the callback for ViewContext::DrawCached and can be used
//! to create a cached presentation for all or part of an element. It will only be called the \b first time ViewContext::DrawCached
//! is called for a given Element. After that, the cached representation is used and this method is not needed for display.
//! @param[in]      drawHandle  data from which to draw.
//! @param[in]      context     context to use to create the cached representation.
//! @param[in]      pixelSize   size (in local coordinates) of a screen pixel.
virtual void _StrokeForCache (CachedDrawHandleCR drawHandle, ViewContextR context, double pixelSize = 0.0) = 0;

//! Return true only if it's possible for screen size dependent geometry to be stroked. This is typically 
//! not the case as methods for displaying exact geometry are available.  If, however, a screen size dependent approximation 
//! (such as a facetted representation of curved geometry) may be displayed then return true. Note that after stroking the
//! _GetSizeDependentGeometryStroked  method is called to determine if any size dependent geometry was actually displayed.
//! This method is called before stroking.
virtual bool _GetSizeDependentGeometryPossible () {return false;}

//! Return true only if the stroked geometry was dependent on the screen size.  This method is called after stroking and is only
//! applicable when _GetSizeDepdentGeometryPossible returns true.
virtual bool _GetSizeDependentGeometryStroked () {return false;}

//! Return DrawExpense::High only if the representation is very expensive to reproduce, otherwise leave as DrawExpense::Medium.  This will cause 
//! the cached geometry to be preserved in low memory conditions.  
virtual DrawExpense _GetDrawExpense () { return DrawExpense::Medium; }

//! Return true if _StrokeForCache should be called for locate. The geometry output by the stroker will be used to generate the curve/edge hits
//! required for snapping as well as for locating the interiors of filled/rendered geometry.
//! @note A stroker that has a very expensive to create cached representation (ex. breps) should NEVER return true, see _WantLocateByQvElem.
virtual bool _WantLocateByStroker () {return true;}

//! Return true to locate interiors using the cached result (QvElem). To be used for very expensive cached representations (ex. breps).
//! As a QvElem hit is only sufficient for locates and not snapping, a stroker that returns true is expected to also maintain it's own
//! wireframe geometry cache and output it when their _Draw method is called for the purpose of picking.
virtual bool _WantLocateByQvElem () {return !_WantLocateByStroker ();}

}; // IStrokeForCache

//=======================================================================================
//! DgnCore implements this interface to provide methods that draw geometry in either cached or non-cached contexts. However,
//! not all implementations of this interface actually "draw" the geometry. For example, one implementation of this interface
//! is used for locating (aka "picking") visible geometry previously drawn.
//! 
//! Any object that implements IDrawGeom will have an "active" ElemMatSymb that determines the appearance of
//! geometry drawn via calls to methods in this interface.
//! 
//! <h3>Coordinates and Dimensionality</h3>
//! All coordinate information are specified in the current "local" coordinate system (see ILocalCoordSys for a discussion of pushing and
//! popping coordinate transforms.)
//! 
//! There are typically both 2D and 3D versions of the geometry methods. The choice of whether to use the 2D or 3D version
//! depends only on whether you have 2D or 3D coordinate information, not on any inherent property of the IDrawGeom. In other words,
//! there is no such thing as a "2D" or "3D" Viewport, all viewports are always 3D - if you use the 2D methods, they are intrinsically
//! planar and oriented on the X,Y plane at the specified Z depth.
// @bsiclass 
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IDrawGeom
{
//__PUBLISH_SECTION_END__
friend struct ViewContext;

protected:

virtual ViewFlagsCP         _GetDrawViewFlags () = 0;
virtual void                _SetDrawViewFlags (ViewFlagsCP) = 0;
virtual void                _ActivateMatSymb (ElemMatSymbCP matSymb) = 0;
virtual void                _ActivateOverrideMatSymb (OvrMatSymbCP ovrMatSymb) = 0;

virtual void                _DrawLineString3d (int numPoints, DPoint3dCP points, DPoint3dCP range) = 0;
virtual void                _DrawLineString2d (int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range) = 0;
virtual void                _DrawPointString3d (int numPoints, DPoint3dCP points, DPoint3dCP range) = 0;
virtual void                _DrawPointString2d (int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range) = 0;
virtual void                _DrawShape3d (int numPoints, DPoint3dCP points, bool filled, DPoint3dCP range) = 0;
virtual void                _DrawShape2d (int numPoints, DPoint2dCP points, bool filled, double zDepth, DPoint2dCP range) = 0;
virtual void                _DrawTriStrip3d (int numPoints, DPoint3dCP points, Int32 usageFlags, DPoint3dCP range) = 0;
virtual void                _DrawTriStrip2d (int numPoints, DPoint2dCP points, Int32 usageFlags, double zDepth, DPoint2dCP range) = 0;
virtual void                _DrawArc3d (DEllipse3dCR ellipse, bool isEllipse, bool filled, DPoint3dCP range) = 0;
virtual void                _DrawArc2d (DEllipse3dCR ellipse, bool isEllipse, bool filled, double zDepth, DPoint2dCP range) = 0;
virtual void                _DrawBSplineCurve (MSBsplineCurveCR curve, bool filled) = 0;
virtual void                _DrawBSplineCurve2d (MSBsplineCurveCR curve, bool filled, double zDepth) = 0;
virtual void                _DrawCurveVector (CurveVectorCR curves, bool isFilled) = 0;
virtual void                _DrawCurveVector2d (CurveVectorCR curves, bool isFilled, double zDepth) = 0;
virtual void                _DrawSolidPrimitive (ISolidPrimitiveCR primitive) = 0;
virtual void                _DrawBSplineSurface (MSBsplineSurfaceCR surface) = 0;
virtual void                _DrawPolyface (PolyfaceQueryCR meshData, bool filled = false) = 0;
virtual StatusInt           _DrawBody (ISolidKernelEntityCR, IFaceMaterialAttachmentsCP attachments = NULL, double pixelSize = 0.0) = 0;
virtual void                _DrawTextString (TextStringCR text, double* zDepth = NULL) = 0;
virtual void                _DrawMosaic (int numX, int numY, uintptr_t const* tileIds, DPoint3d const* verts) = 0;

virtual void                _PushTransClip (TransformCP trans, ClipPlaneSetCP clip = NULL) = 0;
virtual void                _PopTransClip () = 0;

virtual RangeResult         _PushBoundingRange3d (DPoint3dCP range) = 0;
virtual RangeResult         _PushBoundingRange2d (DPoint2dCP range, double zDepth) = 0;
virtual void                _PopBoundingRange () = 0;

virtual size_t              _GetMethodIndex() { return 0; }
virtual void                _PushMethodState () { }
virtual void                _PopMethodState () { }
virtual                     ~IDrawGeom () { }

// NOTE: VTable place holders for future IDrawGeom methods
virtual bool _Dummy1 (void*) {return false;}
virtual bool _Dummy2 (void*) {return false;}
virtual bool _Dummy3 (void*) {return false;}

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

DGNPLATFORM_EXPORT IDrawGeom ();

//! Get the current View Flags for this object. The view flags are initialized from the view flags
//! of its controlling Viewport at the beginning of every display operation. However, during display operations,
//! the view flags are sometimes temporarily modified for specific purposes, so they are not
//! always the same.
//! @return the current view flags for this IViewDraw
DGNPLATFORM_EXPORT ViewFlagsCP GetDrawViewFlags ();

//! Sets the current state of the ViewFlags for this context's output.
DGNPLATFORM_EXPORT void SetDrawViewFlags (ViewFlagsCP);

//! Set an ElemMatSymb to be the "active" ElemMatSymb for this IDrawGeom.
//! @param[in]          matSymb     The new active ElemMatSymb. All geometry drawn via calls to this IDrawGeom will
//!                                     be displayed using the values in this ElemMatSymb.
//! @note     See discussion of the symbology "overrides" in #ActivateOverrideMatSymb
DGNPLATFORM_EXPORT void ActivateMatSymb (ElemMatSymbCP matSymb);

//! Set an ElemMatSymb to be the "active override" ElemMatSymb for this IDrawGeom.
//! @param[in]          ovrMatSymb  The new active override ElemMatSymb.
//!                                     value in ovrMatSymb will be used instead of the value set by #ActivateMatSymb.
DGNPLATFORM_EXPORT void ActivateOverrideMatSymb (OvrMatSymbCP ovrMatSymb);

//! Draw a 3D line string.
//! @param[in]          numPoints   Number of vertices in points array.
//! @param[in]          points      Array of vertices in the line string.
//! @param[in]          range       Array of 2 points with the range (min followed by max) of the vertices in \c points. This argument is
//!                                     optional and is only used to speed processing. If you do not already have the range of your points, pass NULL.
DGNPLATFORM_EXPORT void DrawLineString3d (int numPoints, DPoint3dCP points, DPoint3dCP range);

//! Draw a 2D line string.
//! @param[in]          numPoints   Number of vertices in points array.
//! @param[in]          points      Array of vertices in the line string.
//! @param[in]          zDepth      Z depth value in local coordinates.
//! @param[in]          range       Array of 2 points with the range (min followed by max) of the vertices in \c points. This argument is
//!                                     optional and is only used to speed processing. If you do not already have the range of your points, pass NULL.
DGNPLATFORM_EXPORT void DrawLineString2d (int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range);

//! Draw a 3D point string. A point string is displayed as a series of points, one at each vertex in the array, with no vectors connecting the vertices.
//! @param[in]          numPoints   Number of vertices in points array.
//! @param[in]          points      Array of vertices in the point string.
//! @param[in]          range       Array of 2 points with the range (min followed by max) of the vertices in \c points. This argument is
//!                                     optional and is only used to speed processing. If you do not already have the range of your points, pass NULL.
DGNPLATFORM_EXPORT void DrawPointString3d (int numPoints, DPoint3dCP points, DPoint3dCP range);

//! Draw a 2D point string. A point string is displayed as a series of points, one at each vertex in the array, with no vectors connecting the vertices.
//! @param[in]          numPoints   Number of vertices in points array.
//! @param[in]          points      Array of vertices in the point string.
//! @param[in]          zDepth      Z depth value.
//! @param[in]          range       Array of 2 points with the range (min followed by max) of the vertices in \c points. This argument is
//!                                     optional and is only used to speed processing. If you do not already have the range of your points, pass NULL.
DGNPLATFORM_EXPORT void DrawPointString2d (int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range);

//! Draw a closed 3D shape.
//! @param[in]          numPoints   Number of vertices in \c points array. If the last vertex in the array is not the same as the first vertex, an
//!                                     additional vertex will be added to close the shape.
//! @param[in]          points      Array of vertices of the shape.
//! @param[in]          filled      If true, the shape will be drawn filled.
//! @param[in]          range       Array of 2 points with the range (min followed by max) of the vertices in \c points. This argument is
//!                                     optional and is only used to speed processing. If you do not already have the range of your points, pass NULL.
DGNPLATFORM_EXPORT void DrawShape3d (int numPoints, DPoint3dCP points, bool filled, DPoint3dCP range);

//! Draw a 2D shape.
//! @param[in]          numPoints   Number of vertices in \c points array. If the last vertex in the array is not the same as the first vertex, an
//!                                     additional vertex will be added to close the shape.
//! @param[in]          points      Array of vertices of the shape.
//! @param[in]          zDepth      Z depth value.
//! @param[in]          filled      If true, the shape will be drawn filled.
//! @param[in]          range       Array of 2 points with the range (min followed by max) of the vertices in \c points. This argument is
//!                                     optional and is only used to speed processing. If you do not already have the range of your points, pass NULL.
DGNPLATFORM_EXPORT void DrawShape2d (int numPoints, DPoint2dCP points, bool filled, double zDepth, DPoint2dCP range);

//! Draw a 3D elliptical arc or ellipse.
//! @param[in]          ellipse     arc data.
//! @param[in]          isEllipse   If true, and if full sweep, then draw as an ellipse instead of an arc.
//! @param[in]          filled      If true, and isEllipse is also true, then draw ellipse filled.
//! @param[in]          range       Array of 2 points with the range (min followed by max) of the arc.
//!                                     This argument is optional and is only used to speed processing. If you do not already have the range, pass NULL.
DGNPLATFORM_EXPORT void DrawArc3d (DEllipse3dCR ellipse, bool isEllipse, bool filled, DPoint3dCP range);

//! Draw a 2D elliptical arc or ellipse.
//! @param[in]          ellipse     arc data.
//! @param[in]          isEllipse   If true, and if full sweep, then draw as an ellipse instead of an arc.
//! @param[in]          filled      If true, and isEllipse is also true, then draw ellipse filled.
//! @param[in]          zDepth      Z depth value
//! @param[in]          range       Array of 2 points with the range (min followed by max) of the arc.
//!                                     This argument is optional and is only used to speed processing. If you do not already have the range, pass NULL.
DGNPLATFORM_EXPORT void DrawArc2d (DEllipse3dCR ellipse, bool isEllipse, bool filled, double zDepth, DPoint2dCP range);

//! Draw a BSpline curve.
DGNPLATFORM_EXPORT void DrawBSplineCurve (MSBsplineCurveCR curve, bool filled);

//! Draw a BSpline curve as 2d geometry with display priority.
//! @note Only necessary for non-ICachedDraw calls to support non-zero display priority.
DGNPLATFORM_EXPORT void DrawBSplineCurve2d (MSBsplineCurveCR curve, bool filled, double zDepth);

//! Draw a curve vector.
DGNPLATFORM_EXPORT void DrawCurveVector (CurveVectorCR curves, bool isFilled);

//! Draw a curve vector as 2d geometry with display priority.
//! @note Only necessary for non-ICachedDraw calls to support non-zero display priority.
DGNPLATFORM_EXPORT void DrawCurveVector2d (CurveVectorCR curves, bool isFilled, double zDepth);

//! Draw a light-weight surface or solid primitive.
//! @remarks Solid primitives can be capped or uncapped, they include cones, torus, box, spheres, and sweeps.
DGNPLATFORM_EXPORT void DrawSolidPrimitive (ISolidPrimitiveCR primitive);

//! Draw a BSpline surface.
DGNPLATFORM_EXPORT void DrawBSplineSurface (MSBsplineSurfaceCR);

//! @note Caller is expected to define texture id for illuminated meshed, SetTextureId.
//! @remarks Wireframe fill display supported for non-illuminated meshes.
DGNPLATFORM_EXPORT void DrawPolyface (PolyfaceQueryCR meshData, bool filled = false);

//! Draw a BRep surface/solid entity from the solids kernel.
//! @note Only implemented for ICachedDraw due to potentially expensive/time consuming solids kernel calls.
DGNPLATFORM_EXPORT StatusInt DrawBody (ISolidKernelEntityCR, IFaceMaterialAttachmentsCP attachments = NULL, double pixelSize = 0.0);

//! Draw a series of Glyphs
//! @param[in]          text        Text drawing parameters
//! @param[in]          zDepth      Priority value in 2d or NULL
DGNPLATFORM_EXPORT void DrawTextString (TextStringCR text, double* zDepth = NULL);

//__PUBLISH_SECTION_END__
//! Draw a filled triangle strip from 3D points.
//! @param[in]          numPoints   Number of vertices in \c points array.
//! @param[in]          points      Array of vertices.
//! @param[in]          usageFlags  0 or 1 if tri-strip represents a thickened line.
//! @param[in]          range       Array of 2 points with the range (min followed by max) of the vertices in \c points. This argument is
//!                                     optional and is only used to speed processing. If you do not already have the range of your points, pass NULL.
DGNPLATFORM_EXPORT void DrawTriStrip3d (int numPoints, DPoint3dCP points, Int32 usageFlags, DPoint3dCP range);

//! Draw a filled triangle strip from 2D points.
//! @param[in]          numPoints   Number of vertices in \c points array.
//! @param[in]          points      Array of vertices.
//! @param[in]          zDepth      Z depth value.
//! @param[in]          usageFlags  0 or 1 if tri-strip represents a thickened line.
//! @param[in]          range       Array of 2 points with the range (min followed by max) of the vertices in \c points. This argument is
//!                                     optional and is only used to speed processing. If you do not already have the range of your points, pass NULL.
DGNPLATFORM_EXPORT void DrawTriStrip2d (int numPoints, DPoint2dCP points, Int32 usageFlags, double zDepth, DPoint2dCP range);

DGNPLATFORM_EXPORT void DrawMosaic (int numX, int numY, uintptr_t const* tileIds, DPoint3d const* verts);

DGNPLATFORM_EXPORT RangeResult  PushBoundingRange3d (DPoint3dCP range);
DGNPLATFORM_EXPORT RangeResult  PushBoundingRange2d (DPoint2dCP range, double zDepth);
DGNPLATFORM_EXPORT void         PopBoundingRange ();

size_t  GetMethodIndex ();
void    PushMethodState ();
void    PopMethodState ();

//__PUBLISH_SECTION_START__
// Helper Methods to draw simple SolidPrimitives.
DGNPLATFORM_EXPORT void DrawTorus (DPoint3dCR center, DVec3dCR vectorX, DVec3dCR vectorY, double majorRadius, double minorRadius, double sweepAngle, bool capped);
DGNPLATFORM_EXPORT void DrawCone (DPoint3dCR centerA, DPoint3dCR centerB, double radiusA, double radiusB, bool capped);
DGNPLATFORM_EXPORT void DrawSphere (DPoint3dCR center, RotMatrixCR axes, double radius);
DGNPLATFORM_EXPORT void DrawBox (DVec3dCR primary, DVec3dCR secondary, DPoint3dCR basePoint, DPoint3dCR topPoint, double baseWidth, double baseLength, double topWidth, double topLength, bool capped);

}; // IDrawGeom

//=======================================================================================
//! DgnCore implements this interface to provide the display system for Viewports. 
// @bsiclass 
//=======================================================================================
struct IViewDraw : IDrawGeom
{
//__PUBLISH_SECTION_END__
protected:
virtual void        _SetToViewCoords (bool yesNo) = 0;
virtual void        _SetSymbology (UInt32 lineColorTBGR, UInt32 fillColorTBGR, int lineWidth, UInt32 linePattern) = 0;
virtual void        _DrawGrid (bool doIsoGrid, bool drawDots, DPoint3dCR gridOrigin, DVec3dCR xVector, DVec3dCR yVector, UInt32 gridsPerRef, Point2dCR repetitions) = 0;
virtual bool        _DrawSprite (ISprite* sprite, DPoint3dCP location, DPoint3dCP xVec, int transparency) = 0;
virtual void        _DrawTiledRaster (ITiledRaster* tiledRaster) = 0;
virtual void        _DrawRaster2d (DPoint2d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, byte const* texels, double zDepth, DPoint2d const *range) = 0;
virtual void        _DrawRaster (DPoint3d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, byte const* texels, DPoint3dCP range) = 0;
virtual void        _DrawDgnOle (IDgnOleDraw*) = 0;
virtual void        _DrawPointCloud (IPointCloudDrawParams* drawParams) = 0;
virtual void        _DrawQvElem3d (QvElem* qvElem, int subElemIndex) = 0;
virtual void        _DrawQvElem2d (QvElem* qvElem, double zDepth, int subElemIndex) = 0;
virtual void        _PushRenderOverrides(ViewFlags, CookedDisplayStyleCP displayOverrides = NULL) = 0;
virtual void        _PopRenderOverrides () = 0;
virtual void        _ClearZ () = 0;

virtual CookedDisplayStyleCP _GetDrawDisplayStyle () const = 0;
virtual uintptr_t   _DefineQVTexture (WCharCP textureName, DgnProjectP) {return 0;}
virtual void        _DefineQVGeometryMap (MaterialCR material, ElementHandleCR eh, DPoint2dCP spacing, bool useCellColors, ViewContextR seedContext, bool forAreaPattern) {}

virtual bool        _IsOutputQuickVision () const = 0;
virtual bool        _DeferShadowsToHeal () const = 0;
virtual bool        _ApplyMonochromeOverrides (ViewFlagsCR) const = 0;
virtual StatusInt   _TestOcclusion (int numVolumes, DPoint3dP  verts, int* results) = 0;

virtual void        _PushClipStencil (QvElem* qvElem) = 0;
virtual void        _PopClipStencil () = 0;

virtual ~IViewDraw() {}

//__PUBLISH_SECTION_START__
public:

//! Set the coordinate system temporarily to DgnCoordSystem::View. This removes the root coordinate system,
//! including all camera definitions. It is ONLY valid or useful for drawing "overlay" graphics while drawing View Decorations.
//! @param[in]      yesNo       If true, set to DgnCoordSystem::View. If false, restore to COORDSYS_Root.
//! @note           calls to this method should be paired with true then false values for \c yesNo.
DGNPLATFORM_EXPORT void SetToViewCoords (bool yesNo);

//! Set the active symbology for this IViewDraw. All subsequent draw methods will use the new active symbology.
//! @param[in]      lineColorTBGR   TBGR line color.
//! @param[in]      fillColorTBGR   TBGR color for filled regions.
//! @param[in]      lineWidth       The line width in pixels.
//! @param[in]      linePattern     The 32 bit on/off pattern for lines.
DGNPLATFORM_EXPORT void SetSymbology (UInt32 lineColorTBGR, UInt32 fillColorTBGR, int lineWidth, UInt32 linePattern);

//! Draw the grid matrix.
//! @param[in]      doIsoGrid       Draw the isometric grid points (if applicable).
//! @param[in]      drawDots        Draw grid dots.
//! @param[in]      gridOrigin      Point for the origin of the grid matrix.
//! @param[in]      xVector         Direction of grid X.
//! @param[in]      yVector         Direction of grid y.
//! @param[in]      gridsPerRef     Draw reference lines.
//! @param[in]      repetitions     X,y values for number or repetitions.
DGNPLATFORM_EXPORT void DrawGrid (bool doIsoGrid, bool drawDots, DPoint3dCR gridOrigin, DVec3dCR xVector, DVec3dCR yVector, UInt32 gridsPerRef, Point2dCR repetitions);

//! Draw a sprite at a specific location.
//! @param[in]      sprite          The sprite definition.
//! @param[in]      location        The location where the origin of the sprite should be drawn (in DgnCoordSystem::View coordinates.)
//! @param[in]      xVec            A vector that points in the direction (in DgnCoordSystem::View coordinates) that the x vector
//!                                     of the sprite definition should be oriented. This allows for rotating sprites. If NULL, sprite is draw
//!                                     unrotated.
//! @param[in]      transparency    Sprite is drawn with this transparency  (0=opaque, 255=invisible).
//! @note this method is only valid from View Decorators.
DGNPLATFORM_EXPORT bool DrawSprite (ISprite* sprite, DPoint3dCP location, DPoint3dCP xVec, int transparency);

//! Draw a Tiled Raster.
//! @param[in]      tiledRaster     The Tiled Raster to draw.
DGNPLATFORM_EXPORT void DrawTiledRaster (ITiledRaster* tiledRaster);

DGNPLATFORM_EXPORT void DrawRaster2d (DPoint2d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, byte const* texels, double zDepth, DPoint2dCP range);
DGNPLATFORM_EXPORT void DrawRaster (DPoint3d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, byte const* texels, DPoint3dCP range);

//! Draw a 3D point cloud.
//! @param[in]      drawParams      Object containing draw parameters.
DGNPLATFORM_EXPORT void DrawPointCloud (IPointCloudDrawParams* drawParams);

DGNPLATFORM_EXPORT void DrawQvElem3d (QvElem* qvElem, int subElemIndex = 0);

//! Draw a QvElem
DGNPLATFORM_EXPORT void DrawQvElem2d (QvElem* qvElem, double zDepth, int subElemIndex = 0);

//! Push render overrides
DGNPLATFORM_EXPORT void PushRenderOverrides (ViewFlags, CookedDisplayStyleCP = NULL);

//! Pop render overrides. See #PushRenderOverrides.
DGNPLATFORM_EXPORT void PopRenderOverrides();

//__PUBLISH_SECTION_END__
//! Draw OLE object.
DGNPLATFORM_EXPORT void DrawDgnOle (IDgnOleDraw*);

DGNPLATFORM_EXPORT void DrawTile (uintptr_t tileId, DPoint3d const* verts);

//! Push the supplied QvElem as a clip stencil boundary.
DGNPLATFORM_EXPORT void PushClipStencil (QvElem* qvElem);

//! Pop the most recently pushed clip stencil boundary.
DGNPLATFORM_EXPORT void PopClipStencil ();

DGNPLATFORM_EXPORT StatusInt TestOcclusion (int numVolumes, DPoint3dP verts, int* results);

DGNPLATFORM_EXPORT CookedDisplayStyleCP GetDrawDisplayStyle ();
DGNPLATFORM_EXPORT void ClearZ ();
DGNPLATFORM_EXPORT uintptr_t DefineQVTexture (WCharCP textureName, DgnProjectP dgnFile);
DGNPLATFORM_EXPORT void DefineQVGeometryMap (MaterialCR material, ElementHandleCR eh, DPoint2dCP spacing, bool useCellColors, ViewContextR seedContext, bool forAreaPattern = false);
DGNPLATFORM_EXPORT bool IsOutputQuickVision () const;
DGNPLATFORM_EXPORT bool DeferShadowsToHeal () const;
bool ApplyMonochromeOverrides (ViewFlagsCR) const;

//__PUBLISH_SECTION_START__
}; // IViewDraw

//=======================================================================================
//! Begin/End announcements around cached drawing sequences.
// @bsiclass 
//=======================================================================================
struct ICachedDraw : IRefCounted, IDrawGeom
{
//__PUBLISH_SECTION_END__

private:

virtual QvElem* _GetCacheElement () = 0;
virtual void    _SetCacheElement (QvElem*) = 0;

protected:
                                                                                                           
virtual void    _BeginCacheElement (bool is3d, QvCache*, ViewFlagsCP, uintptr_t elementId) = 0;
virtual QvElem* _EndCacheElement () = 0;
virtual void    _AssignElementToView (QvView*, QvElem*, int viewMode) = 0;

//__PUBLISH_SECTION_START__
public:

DGNPLATFORM_EXPORT void    BeginCacheElement (bool is3d, QvCache*, ViewFlagsCP, uintptr_t elementId = 0);
DGNPLATFORM_EXPORT QvElem* EndCacheElement ();
DGNPLATFORM_EXPORT void    AssignElementToView (QvView*, QvElem*, int viewMode = 0);

//! Push a transform.
//! @param[in]  trans Transform to push.
//! @see #PopTransform
DGNPLATFORM_EXPORT void PushTransform (TransformCR trans);

//! Pop the most recently pushed transform.
//! @see #PushTransform
DGNPLATFORM_EXPORT void PopTransform ();

//__PUBLISH_SECTION_END__
DGNPLATFORM_EXPORT QvElem* GetCacheElement ();
DGNPLATFORM_EXPORT void    SetCacheElement (QvElem*);
//__PUBLISH_SECTION_START__

}; // ICachedDraw

END_BENTLEY_DGNPLATFORM_NAMESPACE
