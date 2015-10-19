/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/IViewDraw.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "ColorUtil.h"
#include "LineStyleResource.r.h"
#include "AreaPattern.h"
#include <Bentley/RefCounted.h>
#include "../DgnPlatform.h"
#include "DgnDb.h"
#include "DgnModel.h"
#include "RenderMaterial.h"
#include "DgnCategory.h"

//__PUBLISH_SECTION_END__
DGNPLATFORM_TYPEDEFS (QvBaseMatSym)
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! The GeomStreamEntryId class identifies a geometric primitive in a GeomStream.
//=======================================================================================
struct GeomStreamEntryId
{
enum class Type
    {
    Invalid = 0,
    Indexed = 1,
    };

private:
    Type            m_type;
    DgnGeomPartId   m_partId;
    uint32_t        m_index;
    uint32_t        m_partIndex;

public:
    GeomStreamEntryId() {Init();}
    GeomStreamEntryId(GeomStreamEntryIdCR rhs) {m_type = rhs.m_type; m_partId = rhs.m_partId; m_index = rhs.m_index; m_partIndex = rhs.m_partIndex;}

    DGNPLATFORM_EXPORT bool operator==(GeomStreamEntryIdCR rhs) const;
    DGNPLATFORM_EXPORT bool operator!=(GeomStreamEntryIdCR rhs) const;
    DGNPLATFORM_EXPORT GeomStreamEntryIdR operator=(GeomStreamEntryIdCR rhs);

    void Init() {m_type = Type::Invalid; m_index = 0; m_partIndex = 0; m_partId = DgnGeomPartId();}
    void SetType(Type type) {m_type = type;}
    void SetGeomPartId(DgnGeomPartId partId) {m_partId = partId; m_partIndex = 0;}
    void SetIndex(uint32_t index) {m_index = index;}
    void SetPartIndex(uint32_t partIndex) {m_partIndex = partIndex;}

    Type GetType() const {return m_type;}
    DgnGeomPartId GetGeomPartId() const {return m_partId;}
    uint32_t GetIndex() const {return m_index;}
    uint32_t GetPartIndex() const {return m_partIndex;}
};

//=======================================================================================
//! Line style parameters
//! @private
//=======================================================================================
struct LineStyleParams : LineStyleParamsResource
{
//__PUBLISH_SECTION_END__
    void Init()
        {
        memset(this, 0, sizeof(LineStyleParams));
        this->rMatrix.form3d[0][0] = this->rMatrix.form3d[1][1] = this->rMatrix.form3d[2][2] =
        this->scale = this->gapScale = this->dashScale = this->normal.z = 1.0;
        }

    //! Compare two LineStyleParams.
    DGNPLATFORM_EXPORT bool operator==(LineStyleParamsCR rhs) const;

    DGNPLATFORM_EXPORT void SetScale(double scale);
//__PUBLISH_SECTION_START__
};

typedef RefCountedPtr<LineStyleInfo> LineStyleInfoPtr;

//=======================================================================================
//! Line style id and parameters
//=======================================================================================
struct LineStyleInfo : RefCountedBase
{
//__PUBLISH_SECTION_END__
protected:

DgnStyleId          m_styleId;
LineStyleParams     m_styleParams; //!< modifiers for user defined linestyle (if applicable)

DGNPLATFORM_EXPORT LineStyleInfo(DgnStyleId styleId, LineStyleParamsCP params);

public:

DGNPLATFORM_EXPORT void CopyFrom(LineStyleInfoCR);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

//! Create an instance of a LineStyleInfo.
DGNPLATFORM_EXPORT static LineStyleInfoPtr Create(DgnStyleId styleId, LineStyleParamsCP params);

//! Compare two LineStyleInfo.
DGNPLATFORM_EXPORT bool operator==(LineStyleInfoCR rhs) const;

DGNPLATFORM_EXPORT DgnStyleId GetStyleId() const;
DGNPLATFORM_EXPORT LineStyleParamsCP GetStyleParams() const;

}; // LineStyleInfo

struct  ISprite;
struct  IDisplaySymbol;
struct  IDgnOleDraw;

enum class DrawExpense
{
    Medium   = 1, //!<  Average for a element type that may be cached
    High     = 2, //!<  Cache it unless at risk of exhausting virtual address
};


enum class FillDisplay //!< Whether a closed region should be drawn for wireframe display with its internal area filled or not.
{
    Never    = 0, //!< don't fill, even if fill attribute is on for the viewport
    ByView   = 1, //!< fill if the fill attribute is on for the viewport
    Always   = 2, //!< always fill, even if the fill attribute is off for the viewport
    Blanking = 3, //!< always fill, fill will always be behind subsequent geometry
};

enum class DgnGeometryClass
{
    Primary      = 0,
    Construction = 1,
    Dimension    = 2,
    Pattern      = 3,
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
protected:
    GradientMode    m_mode;
    uint16_t        m_flags;
    uint16_t        m_nKeys;

    double          m_angle;
    double          m_tint;
    double          m_shift;
    ColorDef        m_colors[MAX_GRADIENT_KEYS];
    double          m_values[MAX_GRADIENT_KEYS];

    DGNPLATFORM_EXPORT GradientSymb();

public:
    DEFINE_BENTLEY_NEW_DELETE_OPERATORS

    DGNPLATFORM_EXPORT void CopyFrom(GradientSymbCR);

    //! Create an instance of a GradientSymb.
    DGNPLATFORM_EXPORT static GradientSymbPtr Create();

    //! Compare two GradientSymb.
    DGNPLATFORM_EXPORT bool operator==(GradientSymbCR rhs) const;

    DGNPLATFORM_EXPORT int GetNKeys() const;
    DGNPLATFORM_EXPORT GradientMode GetMode() const;
    DGNPLATFORM_EXPORT uint16_t GetFlags() const;
    DGNPLATFORM_EXPORT double GetShift() const;
    DGNPLATFORM_EXPORT double GetTint() const;
    DGNPLATFORM_EXPORT double GetAngle() const;
    DGNPLATFORM_EXPORT void GetKey(ColorDef& color, double& value, int index) const;

    DGNPLATFORM_EXPORT void SetMode(GradientMode mode);
    DGNPLATFORM_EXPORT void SetFlags(uint16_t flags);
    DGNPLATFORM_EXPORT void SetAngle(double angle);
    DGNPLATFORM_EXPORT void SetTint(double tint);
    DGNPLATFORM_EXPORT void SetShift(double shift);
    DGNPLATFORM_EXPORT void SetKeys(uint16_t nKeys, ColorDef const* colors, double const* values);
};

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

QVAliasMaterialId();
explicit QVAliasMaterialId(uintptr_t qvAliasMaterialId);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Destructor. This will delete the qv material
DGNPLATFORM_EXPORT virtual ~QVAliasMaterialId();

//! Get the id for the generated Qv Material
DGNPLATFORM_EXPORT uintptr_t GetId() const;

//! Create an instance of this object
//! @param[in] qvAliasMaterialId            Id for generated Qv material
DGNPLATFORM_EXPORT static QVAliasMaterialIdPtr Create(uintptr_t qvAliasMaterialId);
};

#if defined (NEEDS_WORK_MATERIALS)
typedef RefCountedPtr<struct MaterialUVDetail> MaterialUVDetailPtr;

//=======================================================================================
//! Materials which use uv mapping modes Directional Drape, Cubic, Spherical and Cylindrical
//! require additional element based information for their definition.
//=======================================================================================
struct MaterialUVDetail : RefCountedBase
{
//__PUBLISH_SECTION_END__
private:

DPoint3d                m_origin;
RotMatrix               m_rMatrix;
DPoint3d                m_size;
QVAliasMaterialIdPtr    m_appQvId;

MaterialUVDetail();

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Copy the contents of the argument object into this object
//! @param[in] rhs          Object to copy
DGNPLATFORM_EXPORT void Copy(MaterialUVDetailCR rhs);

//! Compare the contents of this object with the method argument for equality
//! @param[in] rhs          Object to compare
DGNPLATFORM_EXPORT bool Equals(MaterialUVDetailCR rhs);

//! Get the origin of the UV mapping.
DGNPLATFORM_EXPORT DPoint3dCR GetOrigin() const;
//! Set the origin on the UV mapping.
DGNPLATFORM_EXPORT void SetOrigin(DPoint3dCR origin);

//! Get the orientation of the UV mapping.
DGNPLATFORM_EXPORT RotMatrixCR GetRMatrix() const;
//! Set the orientation of the UV mapping.
DGNPLATFORM_EXPORT void SetRMatrix(RotMatrixCR rMatrix);

//! Get the size of the UV mapping.
DGNPLATFORM_EXPORT DPoint3dCR GetSize() const;
//! Set the size of the UV mapping.
DGNPLATFORM_EXPORT void SetSize(DPoint3dCR size);

//! Get the QV Material Id for the UV mapping to use.
DGNPLATFORM_EXPORT QVAliasMaterialIdCP GetQVAliasMaterialId() const;
//! Set the QV Material Id for the UV mapping to use.
DGNPLATFORM_EXPORT void SetQVAliasMaterialId(QVAliasMaterialIdP qvId); 

//! Create an instance of this class
DGNPLATFORM_EXPORT static MaterialUVDetailPtr Create();
};
#endif

typedef RefCountedPtr<PlotInfo> PlotInfoPtr;

//=======================================================================================
//! Plot specific resymbolization
//=======================================================================================
struct PlotInfo : RefCountedBase
{
protected:

bool                m_hasLineCap:1;                 //!< if true, use #m_lineCap to determine line cap type.
bool                m_hasLineJoin:1;                //!< if true, use #m_lineJoin to determine line join type.
bool                m_hasScreening:1;               //!< if true, output should be screened by value in #m_screening
bool                m_hasLineWeightMM:1;            //!< if true, line weight is specified in millimeters by #m_widthMM

LineCap             m_lineCap;                      //!< line cap type when m_hasLineCap is set.
LineJoin            m_lineJoin;                     //!< line join type when m_hasLineJoin is set.
double              m_screening;                    //!< screening value when m_hasScreening is set.
double              m_widthMM;                      //!< line width in mm when m_hasLineWeightMM is set.

PlotInfo() {m_hasLineCap = m_hasLineJoin = m_hasScreening = m_hasLineWeightMM = false; m_lineCap = LineCap::None; m_lineJoin = LineJoin::None; m_screening = 0.0; m_widthMM = 0.0;}

public:

static PlotInfoPtr Create() {return new PlotInfo();}

void CopyFrom(PlotInfoCR other)
    {
    m_hasLineCap        = other.m_hasLineCap;
    m_hasLineJoin       = other.m_hasLineJoin;
    m_hasScreening      = other.m_hasScreening;
    m_hasLineWeightMM   = other.m_hasLineWeightMM;

    m_lineCap           = other.m_lineCap;
    m_lineJoin          = other.m_lineJoin;
    m_screening         = other.m_screening;
    m_widthMM           = other.m_widthMM;
    }

bool operator==(PlotInfoCR rhs) const
    {
    if (this == &rhs)
        return true;

    if (rhs.m_hasLineCap      != m_hasLineCap   ||
        rhs.m_hasLineJoin     != m_hasLineJoin  ||
        rhs.m_hasScreening    != m_hasScreening ||
        rhs.m_hasLineWeightMM != m_hasLineWeightMM)
        return false;

    if (rhs.m_lineCap   != m_lineCap   ||
        rhs.m_lineJoin  != m_lineJoin  ||
        rhs.m_screening != m_screening ||
        rhs.m_widthMM   != m_widthMM )
        return false;

    return true;
    }

DGNPLATFORM_EXPORT bool     IsScreeningSet() const;
DGNPLATFORM_EXPORT double   GetScreening() const;
DGNPLATFORM_EXPORT void     SetScreening(double screening, bool set = true);
DGNPLATFORM_EXPORT bool     IsLineJoinSet() const;
DGNPLATFORM_EXPORT LineJoin GetLineJoin() const;
DGNPLATFORM_EXPORT void     SetLineJoin(LineJoin join, bool set = true);
DGNPLATFORM_EXPORT bool     IsLineCapSet() const;
DGNPLATFORM_EXPORT LineCap  GetLineCap() const;
DGNPLATFORM_EXPORT void     SetLineCap(LineCap cap, bool set = true);
DGNPLATFORM_EXPORT bool     IsLineWeightMMSet() const;
DGNPLATFORM_EXPORT double   GetLineWeightMM () const;
DGNPLATFORM_EXPORT void     SetLineWeightMM (double weight, bool set = true);

}; // PlotInfoPtr

//=======================================================================================
//! This structure holds all of the information about an element specifying the "displayable parameters" of the element.
//! It is typically extracted from the "dhdr" section of the element header and from user data.
// @bsiclass 
//=======================================================================================
struct ElemDisplayParams
{
private:
    struct AppearanceOverrides
        {
        bool m_color:1;
        bool m_weight:1;
        bool m_style:1;
        bool m_material:1;
        bool m_fill:1; // If not set, fill is an opaque fill that matches sub-category appearance color...
        bool m_bgFill:1; // When set, fill is an opaque fill that matches current view background color...
        AppearanceOverrides() {memset(this, 0, sizeof(*this));}
        };

    // NOTE: Constructor uses memset (this, 0, offsetof (ElemDisplayParams, m_material));
    AppearanceOverrides m_appearanceOverrides;          //!< flags for parameters that override SubCategory::Appearance.
    bool                m_resolved:1;                   //!< whether Resolve has established SubCategory::Appearance/effective values.
    DgnCategoryId       m_categoryId;                   //!< the Category Id on which the geometry is drawn.
    DgnSubCategoryId    m_subCategoryId;                //!< the SubCategory Id that controls the appearence of subsequent geometry.
    int32_t             m_elmPriority;                  //!< display priority (applies to 2d only)
    int32_t             m_netPriority;                  //!< net display priority for element/category (applies to 2d only)
    uint32_t            m_weight;
    ColorDef            m_lineColor;
    ColorDef            m_fillColor;                    //!< fill color (applicable only if filled)
    FillDisplay         m_fillDisplay;                  //!< whether or not the element should be displayed filled
    double              m_elmTransparency;              //!< transparency, 1.0 == completely transparent.
    double              m_netElmTransparency;           //!< net transparency for element/category.
    double              m_fillTransparency;             //!< fill transparency, 1.0 == completely transparent.
    double              m_netFillTransparency;          //!< net transparency for fill/category.
    DgnGeometryClass    m_geometryClass;                //!< geometry class
    DgnMaterialId       m_material;                     //!< render material ID.
    LineStyleInfoPtr    m_styleInfo;                    //!< line style id plus modifiers.
    GradientSymbPtr     m_gradient;                     //!< gradient fill settings.
    PatternParamsPtr    m_pattern;                      //!< area pattern settings.
    PlotInfoPtr         m_plotInfo;

public:
    DGNPLATFORM_EXPORT ElemDisplayParams();
    DGNPLATFORM_EXPORT explicit ElemDisplayParams(ElemDisplayParamsCR rhs);
    DGNPLATFORM_EXPORT void ResetAppearance(); //!< Like Init, but saves and restores category and sub-category around the call to Init. This is particularly useful when a single element draws objects of different symbology, but its draw code does not have easy access to reset the category.
    DGNPLATFORM_EXPORT void Resolve(ViewContextR); // Resolve effective values

    void SetCategoryId(DgnCategoryId categoryId) {m_categoryId = categoryId; m_subCategoryId = DgnCategory::GetDefaultSubCategoryId(categoryId); memset(&m_appearanceOverrides, 0, sizeof(m_appearanceOverrides)); m_resolved = false;} // Setting the Category Id also sets the SubCategory to the default.
    void SetSubCategoryId(DgnSubCategoryId subCategoryId) {m_subCategoryId = subCategoryId; memset(&m_appearanceOverrides, 0, sizeof(m_appearanceOverrides)); m_resolved = false;}
    void SetWeight(uint32_t weight) {m_appearanceOverrides.m_weight = true; m_weight = weight;}
    void SetLineStyle(LineStyleInfoP styleInfo) {m_appearanceOverrides.m_style = true; m_styleInfo = styleInfo;}
    void SetLineColor(ColorDef color) {m_appearanceOverrides.m_color = true; m_lineColor = color;}
    void SetFillDisplay(FillDisplay display) {m_fillDisplay = display;}
    void SetFillColor(ColorDef color) {m_appearanceOverrides.m_fill = true; m_appearanceOverrides.m_bgFill = false; m_fillColor = color;}
    void SetFillColorToViewBackground() {m_appearanceOverrides.m_fill = false; m_appearanceOverrides.m_bgFill = true;} // FillDisplay::Blanking creates an opaque view background fill...
    void SetGradient(GradientSymbP gradient) {m_gradient = gradient;}
    void SetGeometryClass(DgnGeometryClass geomClass) {m_geometryClass = geomClass;}
    void SetTransparency(double transparency) {m_elmTransparency = m_netElmTransparency = m_fillTransparency = m_netFillTransparency = transparency;} // NOTE: Sets BOTH element and fill transparency...
    void SetFillTransparency(double transparency) {m_fillTransparency = m_netFillTransparency = transparency;} 
    void SetDisplayPriority(int32_t priority) {m_elmPriority = m_netPriority = priority;} // Set display priority (2d only).
    void SetMaterial(DgnMaterialId material) {m_appearanceOverrides.m_material = true; m_material = material;}
    void SetPatternParams(PatternParamsP patternParams) {m_pattern = patternParams;}

    //! @cond DONTINCLUDEINDOC
    double GetNetTransparency() const {BeAssert(m_resolved); return m_netElmTransparency;}
    double GetNetFillTransparency() const {BeAssert(m_resolved); return m_netFillTransparency;}

    int32_t GetNetDisplayPriority() const {BeAssert(m_resolved); return m_netPriority;} // Get net display priority (2d only).
    void SetNetDisplayPriority(int32_t priority) {m_netPriority = priority;} // RASTER USE ONLY!!!

    bool IsLineColorFromSubCategoryAppearance() const {return !m_appearanceOverrides.m_color;}
    bool IsWeightFromSubCategoryAppearance() const {return !m_appearanceOverrides.m_weight;}
    bool IsLineStyleFromSubCategoryAppearance() const {return !m_appearanceOverrides.m_style;}
    bool IsMaterialFromSubCategoryAppearance() const {return !m_appearanceOverrides.m_material;}
    bool IsFillColorFromSubCategoryAppearance() const {return !m_appearanceOverrides.m_fill && !m_appearanceOverrides.m_bgFill;}
    bool IsFillColorFromViewBackground() const {return m_appearanceOverrides.m_bgFill;}
    //! @endcond

    //! Compare two ElemDisplayParam.
    DGNPLATFORM_EXPORT bool operator==(ElemDisplayParamsCR rhs) const;

    //! copy operator
    DGNPLATFORM_EXPORT ElemDisplayParamsR operator=(ElemDisplayParamsCR rhs);

    //! Get element category
    DgnCategoryId GetCategoryId() const {return m_categoryId;}

    //! Get element sub-category
    DgnSubCategoryId GetSubCategoryId() const {return m_subCategoryId;}

    //! Get element color
    ColorDef GetLineColor() const {BeAssert(m_appearanceOverrides.m_color || m_resolved); return m_lineColor;}

    //! Get element fill color
    ColorDef GetFillColor() const {BeAssert(m_appearanceOverrides.m_fill || m_resolved); return m_fillColor;}

    //! Get fill display setting
    FillDisplay GetFillDisplay() const {return m_fillDisplay;}

    //! Get gradient fill information. Valid when FillDisplay::Never != GetFillDisplay() and not nullptr.
    GradientSymbCP GetGradient() const {return m_gradient.get();}

    //! Get the area pattern params.
    PatternParamsCP GetPatternParams() const {return m_pattern.get();}

    //! Get the geometry class.
    DgnGeometryClass GetGeometryClass() const {return m_geometryClass;}

    //! Get line style information.
    LineStyleInfoCP GetLineStyle() const {/* BeAssert(m_appearanceOverrides.m_style || m_resolved); */ return m_styleInfo.get();}

    //! Get element weight.
    uint32_t GetWeight() const {BeAssert(m_appearanceOverrides.m_weight || m_resolved); return m_weight;}

    //! Get element transparency.
    double GetTransparency() const {return m_elmTransparency;}

    //! Get fill/gradient transparency.
    double GetFillTransparency() const {return m_fillTransparency;}

    //! Get render material.
    DgnMaterialId GetMaterial() const {BeAssert(m_appearanceOverrides.m_material || m_resolved); return m_material; }

    //! Get element display priority (2d only).
    int32_t GetDisplayPriority() const {return m_elmPriority;}
}; // ElemDisplayParams

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
        uint32_t       scale:1;
        uint32_t       dashScale:1;
        uint32_t       gapScale:1;
        uint32_t       orgWidth:1;
        uint32_t       endWidth:1;
        uint32_t       phaseShift:1;
        uint32_t       autoPhase:1;
        uint32_t       maxCompress:1;
        uint32_t       iterationLimit:1;
        uint32_t       treatAsSingleSegment:1;
        uint32_t       plane:1;
        uint32_t       cosmetic:1;
        uint32_t       centerPhase:1;
        uint32_t       xElemPhaseSet:1;
        uint32_t       startTangentSet:1;
        uint32_t       endTangentSet:1;
        uint32_t       elementIsClosed:1;
        uint32_t       continuationXElems:1;
        uint32_t       isCurve:1;
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
    uintptr_t       m_textureHandle;

public:
    DGNPLATFORM_EXPORT LineStyleSymb();

    DGNPLATFORM_EXPORT int FromResolvedElemDisplayParams(ElemDisplayParamsCR, ViewContextR context, DPoint3dCP, DPoint3dCP);
    DGNPLATFORM_EXPORT int FromNaturalElemDisplayParams(ElemDisplayParamsR, ViewContextR context, DPoint3dCP, DPoint3dCP);
    DGNPLATFORM_EXPORT int FromResolvedStyle(LineStyleInfoCP styleInfo, ViewContextR context, DPoint3dCP startTangent, DPoint3dCP endTangent);

    void               Clear () {m_lStyle = NULL; m_options.orgWidth = m_options.endWidth = false; m_textureHandle = 0; }
    DGNPLATFORM_EXPORT void Init(ILineStyleCP);

public:
    DGNPLATFORM_EXPORT ILineStyleCP  GetILineStyle() const;
    DGNPLATFORM_EXPORT void          GetPlaneAsMatrixRows(RotMatrixR matrix) const;
    DGNPLATFORM_EXPORT double        GetScale() const;
    DGNPLATFORM_EXPORT double        GetDashScale() const;
    DGNPLATFORM_EXPORT double        GetGapScale() const;
    DGNPLATFORM_EXPORT double        GetOriginWidth() const;
    DGNPLATFORM_EXPORT double        GetEndWidth() const;
    DGNPLATFORM_EXPORT double        GetPhaseShift() const;
    DGNPLATFORM_EXPORT double        GetFractionalPhase() const;
    DGNPLATFORM_EXPORT double        GetMaxCompress() const;
    DGNPLATFORM_EXPORT int           GetNumIterations() const;
    DGNPLATFORM_EXPORT double        GetMaxWidth() const;
    DGNPLATFORM_EXPORT double        GetTotalLength() const;
    DGNPLATFORM_EXPORT DPoint3dCP    GetStartTangent() const;
    DGNPLATFORM_EXPORT DPoint3dCP    GetEndTangent() const;
    DGNPLATFORM_EXPORT uintptr_t     GetTextureHandle() const;
    DGNPLATFORM_EXPORT bool          IsScaled() const;
    DGNPLATFORM_EXPORT bool          IsAutoPhase() const;
    DGNPLATFORM_EXPORT bool          IsCenterPhase() const;
    DGNPLATFORM_EXPORT bool          IsCosmetic() const;
    DGNPLATFORM_EXPORT bool          IsTreatAsSingleSegment() const;
    DGNPLATFORM_EXPORT bool          HasDashScale() const;
    DGNPLATFORM_EXPORT bool          HasGapScale() const;
    DGNPLATFORM_EXPORT bool          HasOrgWidth() const;
    DGNPLATFORM_EXPORT bool          HasEndWidth() const;
    DGNPLATFORM_EXPORT bool          IsElementClosed() const;
    DGNPLATFORM_EXPORT bool          IsCurve() const;
    DGNPLATFORM_EXPORT bool          HasPhaseShift() const;
    DGNPLATFORM_EXPORT bool          HasIterationLimit() const;
    DGNPLATFORM_EXPORT bool          HasPlane() const;
    DGNPLATFORM_EXPORT bool          HasStartTangent() const;
    DGNPLATFORM_EXPORT bool          HasEndTangent() const;
    DGNPLATFORM_EXPORT void          SetPlaneAsMatrixRows(RotMatrixCP);
    DGNPLATFORM_EXPORT void          SetNormalVec(DPoint3dCP);
    DGNPLATFORM_EXPORT void          SetOriginWidth(double width);
    DGNPLATFORM_EXPORT void          SetEndWidth(double width);
    DGNPLATFORM_EXPORT void          SetWidth(double width);
    DGNPLATFORM_EXPORT void          SetScale(double scaleFactor);
    DGNPLATFORM_EXPORT void          SetFractionalPhase(bool isOn, double fraction);
    DGNPLATFORM_EXPORT void          SetCenterPhase(bool isOn);
    DGNPLATFORM_EXPORT void          SetPhaseShift(bool isOn, double distance);
    DGNPLATFORM_EXPORT void          SetTreatAsSingleSegment(bool yesNo);
    DGNPLATFORM_EXPORT void          SetTangents(DPoint3dCP, DPoint3dCP);
    DGNPLATFORM_EXPORT void          SetLineStyle(ILineStyleCP lstyle);
    DGNPLATFORM_EXPORT void          ConvertLineStyleToTexture(ViewContextR context, bool force);

    bool HasTrueWidth() const  {return HasOrgWidth() || HasEndWidth();}
    bool HasMaxCompress() const {return m_options.maxCompress;}
    bool ContinuationXElems() const {return m_options.continuationXElems;}
    void SetXElemPhase(double last) {m_xElemPhase = last; m_options.xElemPhaseSet=true;}
    void SetElementClosed(bool closed) {m_options.elementIsClosed = closed;}
    void SetIsCurve(bool isCurve) {m_options.isCurve = isCurve;}

    DGNPLATFORM_EXPORT void SetGapScale(double scaleFactor);
    DGNPLATFORM_EXPORT void SetDashScale(double scaleFactor);
    DGNPLATFORM_EXPORT void SetTotalLength(double);
    DGNPLATFORM_EXPORT void SetCosmetic(bool cosmetic);
    DGNPLATFORM_EXPORT void ClearContinuationData();
    DGNPLATFORM_EXPORT void CheckContinuationData();
}; // LineStyleSymb

//=======================================================================================
//! DgnCore implements this class for setting/getting the Material and Symbology (ElemMatSymb) used to draw geometry.
//! An ElemMatSymb is the "cooked" material and symbology that determines the real appearance (e.g. color/width/raster pattern/linestyle,
//! etc.) used to draw all geometry.
//!
//! Viewports always have an "active" ElemMatSymb.
//=======================================================================================
struct  ElemMatSymb
{
private:
    ColorDef            m_lineColor;
    ColorDef            m_fillColor;
    int                 m_elementStyle;
    bool                m_isFilled;
    bool                m_isBlankingRegion;
    uintptr_t           m_extSymbID;
    RenderMaterialPtr   m_material;
    uint32_t            m_rasterWidth;
    uint32_t            m_rasterPat;
    LineStyleSymb       m_lStyleSymb;
    GradientSymbPtr     m_gradient;
    PatternParamsPtr    m_patternParams;

public:
    DGNPLATFORM_EXPORT ElemMatSymb();
    DGNPLATFORM_EXPORT explicit ElemMatSymb(ElemMatSymbCR rhs);

    DGNPLATFORM_EXPORT void Init();

    //! INTERNAL USE ONLY: Should only ever be called by sub-classes of ViewContext, use CookDisplayParams instead!
    DGNPLATFORM_EXPORT void FromResolvedElemDisplayParams(ElemDisplayParamsCR, ViewContextR, DPoint3dCP startTan, DPoint3dCP endTan);
    DGNPLATFORM_EXPORT void FromNaturalElemDisplayParams(ElemDisplayParamsR, ViewContextR, DPoint3dCP startTan, DPoint3dCP endTan);

    // Get the element style.
    int GetElementStyle() {return m_elementStyle;}

    //! Get the extended material ID from this ElemMatSymb
    uintptr_t GetExtSymbId() const {return m_extSymbID;}

    //! Set the extended material ID for this ElemMatSymb
    void SetExtSymbId(uintptr_t extSymbID) {m_extSymbID = extSymbID;}

    //! Set the gradient symbology
    void SetGradient(GradientSymbP gradient) {m_gradient = gradient;}

    //! @name Query Methods
    //@{

    //! Compare two ElemMatSymb.
    DGNPLATFORM_EXPORT bool operator==(ElemMatSymbCR rhs) const;

    //! copy operator
    DGNPLATFORM_EXPORT ElemMatSymbR operator=(ElemMatSymbCR rhs);

    //! Get the TBGR line color from this ElemMatSymb
    ColorDef GetLineColor() const {return m_lineColor;}

    //! Get the TBGR fill color from this ElemMatSymb.
    ColorDef GetFillColor() const {return m_fillColor;}

    //! Get the width in pixels from this ElemMatSymb.
    uint32_t GetWidth() const {return m_rasterWidth;}

    //! Determine whether TrueWidth is on for this ElemMatSymb
    DGNPLATFORM_EXPORT bool HasTrueWidth() const;

    //! Get the raster pattern from this ElemMatSymb. The raster pattern is a 32 bit mask that is
    //! repeated along geometry. For each bit that is on in the pattern, a pixel is set to the line color.
    uint32_t GetRasterPattern() const {return m_rasterPat;}

    //! Get the style index from this ElemMatSymb (INVALID_STYLE returned for raster patterns not derived from line codes)
    int32_t GetRasterPatternIndex() const {return m_elementStyle;}

    //! Determine whether the fill flag is on for this ElemMatSymb.
    bool IsFilled() const {return m_isFilled;}

    //! Determine whether the fill represents blanking region.
    bool IsBlankingRegion() const {return m_isBlankingRegion;}

    //! Get the LineStyleSymb from this ElemMatSymb.
    LineStyleSymbCR GetLineStyleSymb() const {return m_lStyleSymb;}

    //! Get the GradientSymb from this ElemMatSymb.
    GradientSymbCP GetGradientSymb() const {return m_gradient.get();}

    //! Get the render material.
    RenderMaterialCP GetMaterial() const {return m_material.get();}

    //! Get the area pattern params.
    PatternParamsCP GetPatternParams() const {return m_patternParams.get();}
    //@}

    //! @name Set Methods
    //@{
    //! Set the current line color for this ElemMatSymb.
    //! @param[in] lineColor   the new TBGR line color for this ElemMatSymb.
    void SetLineColor(ColorDef lineColor) { m_lineColor = lineColor; }
    void SetLineTransparency(Byte transparency) {m_lineColor.SetAlpha(transparency);}

    //! Set the current fill color for this ElemMatSymb.
    //! @param[in] fillColor   the new TBGR fill color for this ElemMatSymb.
    void SetFillColor(ColorDef fillColor) {m_fillColor = fillColor; }
    void SetFillTransparency(Byte transparency) {m_fillColor.SetAlpha(transparency);}

    //! Turn on or off the fill flag for this ElemMatSymb.
    //! @param[in] filled      if true, the interior of elements drawn using this ElemMatSymb will be filled using the fill color.
    void SetIsFilled(bool filled) {m_isFilled = filled;}

    //! Set that fill is always behind other geometry.
    void SetIsBlankingRegion(bool blanking) {m_isBlankingRegion = blanking;}

    //! Set the width in pixels for this ElemMatSymb.
    //! @param[in] rasterWidth the width in pixels of lines drawn using this ElemMatSymb.
    //! @note         If either TrueWidthStart or TrueWidthEnd are non-zero, this value is ignored.
    void SetWidth(uint32_t rasterWidth) {m_rasterWidth = rasterWidth;}

    //! Set the raster pattern for this ElemMatSymb.
    //! @param[in] rasterPat   the 32 bit on-off pattern to be repeated along lines drawn using this ElemMatSymb.
    //! @see          #GetRasterPattern
    void SetRasterPattern(uint32_t rasterPat) {m_rasterPat = rasterPat; m_elementStyle = 0; m_extSymbID = 0;}

    //! Set a raster pattern derived from a line code for this ElemMatSymb. Used to support plotting of cosmetic line styles mapped to line codes.
    //! @param[in] index       the new line style code.
    //! @param[in] rasterPat   the 32 bit on-off pattern to be repeated along lines drawn using this ElemMatSymb.
    //! @see          #GetRasterPattern #GetRasterPatternIndex
    DGNPLATFORM_EXPORT void SetIndexedRasterPattern(int32_t index, uint32_t rasterPat);

    //! Get the LineStyleSymb from this ElemMatSymb for setting line style parameters.
    LineStyleSymbR GetLineStyleSymbR () {return m_lStyleSymb;}

    //! Set the render material.
    DGNPLATFORM_EXPORT void SetMaterial(RenderMaterialP material) { m_material = material; }

    //! Set area patterning parameters.
    void SetPatternParams(PatternParamsP patternParams) {m_patternParams = patternParams;}

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
private:
    uint32_t        m_flags;
    ElemMatSymb     m_matSymb;

public:
    DGNPLATFORM_EXPORT OvrMatSymb() : m_flags(MATSYMB_OVERRIDE_None) {}

    ElemMatSymbCR GetMatSymb() const {return m_matSymb;}
    ElemMatSymbR GetMatSymbR () {return m_matSymb;}

    uintptr_t GetExtSymbId() const {return m_matSymb.GetExtSymbId();}
    void SetExtSymbId(uintptr_t extSymbID) {m_matSymb.SetExtSymbId(extSymbID); m_flags |= MATSYMB_OVERRIDE_ExtSymb;}

public:
    //! Compare two OvrMatSymb.
    bool operator==(OvrMatSymbCR rhs) const {if (this == &rhs) return true; if (rhs.m_flags != m_flags) return false; return rhs.m_matSymb == m_matSymb;}

    uint32_t GetFlags() const{return m_flags;}
    ColorDef GetLineColor() const {return m_matSymb.GetLineColor();}
    ColorDef GetFillColor() const {return m_matSymb.GetFillColor();}
    uint32_t GetWidth() const {return m_matSymb.GetWidth();}
    uint32_t GetRasterPattern() const {return m_matSymb.GetRasterPattern();}
    int32_t GetRasterPatternIndex() const {return m_matSymb.GetRasterPatternIndex();}
    RenderMaterialCP GetMaterial() const {return m_matSymb.GetMaterial();}
    PatternParamsCP GetPatternParams() const {return m_matSymb.GetPatternParams();}

    DGNPLATFORM_EXPORT void Clear();
    void SetFlags(uint32_t flags) {m_flags = flags;}
    void SetLineColor(ColorDef color) {m_matSymb.SetLineColor(color); m_flags |= MATSYMB_OVERRIDE_Color;}
    void SetFillColor(ColorDef color) {m_matSymb.SetFillColor(color); m_flags |= MATSYMB_OVERRIDE_FillColor;}
    void SetLineTransparency(Byte trans) {m_matSymb.SetLineTransparency(trans); m_flags |= MATSYMB_OVERRIDE_ColorTransparency;}
    void SetFillTransparency(Byte trans) {m_matSymb.SetFillTransparency(trans); m_flags |= MATSYMB_OVERRIDE_FillColorTransparency;}
    void SetWidth(uint32_t width) {m_matSymb.SetWidth(width); m_flags |= MATSYMB_OVERRIDE_RastWidth;}
    void SetRasterPattern(uint32_t rasterPat) {m_matSymb.SetRasterPattern(rasterPat); m_flags |= MATSYMB_OVERRIDE_Style; m_matSymb.GetLineStyleSymbR().SetLineStyle(NULL);}
    void SetIndexedRasterPattern(int32_t index, uint32_t rasterPat) {m_matSymb.SetIndexedRasterPattern(index, rasterPat); m_flags |= MATSYMB_OVERRIDE_Style; m_matSymb.GetLineStyleSymbR().SetLineStyle(NULL);}
    void SetMaterial(RenderMaterialP material) {m_matSymb.SetMaterial(material); m_flags |= MATSYMB_OVERRIDE_RenderMaterial;}
    void SetPatternParams(PatternParamsP patternParams) {m_matSymb.SetPatternParams(patternParams);}
    void SetProxy(bool edge, bool hidden) {m_flags |= (MATSYMB_OVERRIDE_IsProxy | (edge ? MATSYMB_OVERRIDE_IsProxyEdge : 0) | (hidden ? MATSYMB_OVERRIDE_IsProxyHidden: 0)); }
    bool GetProxy(bool& edge, bool& hidden) {edge = 0 != (m_flags & MATSYMB_OVERRIDE_IsProxyEdge); hidden = 0 != (m_flags & MATSYMB_OVERRIDE_IsProxyHidden); return 0 != (m_flags & MATSYMB_OVERRIDE_IsProxy); }
    void SetUnderlay() { m_flags |= MATSYMB_OVERRIDE_IsProxyUnderlay; }
    DGNPLATFORM_EXPORT void SetLineStyle(int32_t styleNo, DgnModelR modelRef, DgnModelR styleDgnModel, LineStyleParamsCP lStyleParams, ViewContextR context, DPoint3dCP startTangent, DPoint3dCP endTangent);

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

    // If IsThreadBound returns false, implement AddRef and Release using InterlockedIncrement
    virtual uint32_t AddRef() = 0;
    virtual uint32_t Release() = 0;
    virtual bool IsThreadBound() = 0; // If true, always executed in calling thread instead of QV thread
    virtual VersionNumber GetVersion() { return IPointCloudDrawParams_InitialVersion; }
    virtual bool GetRange(DPoint3dP range) = 0; // returns false if it does not have range

    //  Added to points returned by GetPoints or GetFPoints
    virtual bool GetOrigin(DPoint3dP origin) = 0; // returns false if no origin

    virtual ColorDef const* GetRgbColors() = 0; // Returns NULL if not using colors

    virtual uint32_t GetNumPoints() = 0;
    virtual DPoint3dCP GetDPoints() = 0; // Returns NULL if using floats
    virtual FPoint3dCP GetFPoints() = 0; // Returns NULL if using doubles
};

//=======================================================================================
//! This interface defines the method used by ViewContext::DrawCached.
// @bsiclass 
//=======================================================================================
struct IStrokeForCache
{
    //! Stroke this object to create a cached representation. This method is the callback for ViewContext::DrawCached and can be used
    //! to create a cached presentation for geometry that is expensive to create. The cached presentation created when this is called
    //! will then be used for subsequent draws.
    //! @param[in] context context to use to create the cached representation.
    //! @param[in] pixelSize size (in local coordinates) of a screen pixel.
    virtual void _StrokeForCache(ViewContextR context, double pixelSize = 0.0) = 0;

    //! Query if it's possible for screen size dependent geometry to be stroked. This is typically
    //! not the case when methods for displaying exact geometry are available. If however, a screen size dependent 
    //! approximation (such as a facetted representation of curved geometry) may be displayed, then return true.
    //! @note This method is called before _StrokeForCache.
    virtual bool _GetSizeDependentGeometryPossible() const {return false;}

    //! Return true only if the stroked geometry was dependent on the screen size. This method is called after stroking and is only
    //! applicable when _GetSizeDepdentGeometryPossible returns true.
    //! @note This method is called after _StrokeForCache to determine if a size dependent cache presentation was actually created.
    virtual bool _GetSizeDependentGeometryStroked() const {return false;}

    //! Return DrawExpense::High only if the representation is very expensive to reproduce, otherwise leave as DrawExpense::Medium.  This will cause
    //! the cached geometry to be preserved in low memory conditions.
    virtual DrawExpense _GetDrawExpense() const {return DrawExpense::Medium;}

    //! Return true if _StrokeForCache should be called for locate. The geometry output by the stroker will be used to generate the curve/edge hits
    //! required for snapping as well as for locating the interiors of filled/rendered geometry.
    //! @note A stroker that has a very expensive to create cached representation (ex. breps) should NEVER return true, see _WantLocateByQvElem.
    virtual bool _WantLocateByStroker() const {return true;}

    //! Return true to locate interiors using the cached result (QvElem). To be used for very expensive cached representations (ex. breps).
    //! As a QvElem hit is only sufficient for locates and not snapping, a stroker that returns true is expected to also maintain it's own
    //! wireframe geometry cache and output it when their _Draw method is called for the purpose of picking.
    virtual bool _WantLocateByQvElem() const {return !_WantLocateByStroker();}

    //! Return geometry range for the purpose of calculating pixelSize for creating a size dependent cache representation.
    //! @note A valid range is required only if _GetSizeDependentGeometryPossible returns true.
    virtual DRange3d _GetRange() const {return DRange3d::NullRange();}

    //! Return QvCache to use. To use temporary QvCache, override to return nullptr.
    virtual QvCacheP _GetQVCache() const {return _GetDgnDb().Models().GetQvCache();}

    //! Return key value that may be used to identify/lookup QvElem in QvCache.
    virtual int32_t _GetQvIndex() const = 0;

    //! Return QvElem created by a prior call to _StrokeForCache. When nullptr is returned, _StrokeForCache will be called to create a new QvElem.
    virtual QvElemP _GetQvElem(double pixelSize = 0.0) const = 0;

    //! Save the QvElem created by calling _StrokeForCache for use in subsequent draws.
    virtual void _SaveQvElem(QvElemP, double pixelSize = 0.0, double sizeDependentRatio = 0.0) const = 0;

    //! Return dgnDb for default QvCache.
    virtual DgnDbR _GetDgnDb() const = 0;

}; // IStrokeForCache

//=======================================================================================
//! IStrokeForCache sub-class specific to GeometricElement.
// @bsiclass 
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE StrokeElementForCache : IStrokeForCache
{
protected:
    GeometricElementCR  m_element;

public:

    explicit StrokeElementForCache(GeometricElementCR element) : m_element(element) {}

    virtual DgnDbR _GetDgnDb() const override {return m_element.GetDgnDb();}
    virtual DRange3d _GetRange() const override {return m_element.CalculateRange3d();}
    virtual QvCacheP _GetQVCache() const override {return m_element.GetMyQvCache();}

    DGNPLATFORM_EXPORT virtual QvElemP _GetQvElem(double pixelSize) const;
    DGNPLATFORM_EXPORT virtual void _SaveQvElem(QvElemP, double pixelSize, double sizeDependentRatio) const;
};

//=======================================================================================
//! IStrokeForCache sub-class specific to tools/transient cached graphics.
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TransientCachedGraphics : IStrokeForCache
{
protected:
    DgnDbR          m_dgnDb;
    mutable QvElemP m_qvElem;

public:
    explicit TransientCachedGraphics(DgnDbR dgnDb) : m_dgnDb(dgnDb) {m_qvElem = nullptr;}
    DGNPLATFORM_EXPORT virtual ~TransientCachedGraphics();

    virtual DgnDbR _GetDgnDb() const override {return m_dgnDb;}
    virtual int32_t _GetQvIndex() const override {return 0;} // Not used by Get/Save...

    virtual QvElemP _GetQvElem(double pixelSize) const override {return m_qvElem;}
    virtual void _SaveQvElem(QvElemP qvElem, double pixelSize, double sizeDependentRatio) const override {m_qvElem = qvElem;}
};

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
//! there is no such thing as a "2D" or "3D" DgnViewport, all viewports are always 3D - if you use the 2D methods, they are intrinsically
//! planar and oriented on the X,Y plane at the specified Z depth.
// @bsiclass 
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IDrawGeom
{
    friend struct ViewContext;

protected:
    virtual ViewFlagsCP _GetDrawViewFlags() = 0;
    virtual void _SetDrawViewFlags(ViewFlagsCP) = 0;
    virtual void _ActivateMatSymb(ElemMatSymbCP matSymb) = 0;
    virtual void _ActivateOverrideMatSymb(OvrMatSymbCP ovrMatSymb) = 0;
    virtual void _DrawLineString3d(int numPoints, DPoint3dCP points, DPoint3dCP range) = 0;
    virtual void _DrawLineString2d(int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range) = 0;
    virtual void _DrawPointString3d(int numPoints, DPoint3dCP points, DPoint3dCP range) = 0;
    virtual void _DrawPointString2d(int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range) = 0;
    virtual void _DrawShape3d(int numPoints, DPoint3dCP points, bool filled, DPoint3dCP range) = 0;
    virtual void _DrawShape2d(int numPoints, DPoint2dCP points, bool filled, double zDepth, DPoint2dCP range) = 0;
    virtual void _DrawTriStrip3d(int numPoints, DPoint3dCP points, int32_t usageFlags, DPoint3dCP range) = 0;
    virtual void _DrawTriStrip2d(int numPoints, DPoint2dCP points, int32_t usageFlags, double zDepth, DPoint2dCP range) = 0;
    virtual void _DrawArc3d(DEllipse3dCR ellipse, bool isEllipse, bool filled, DPoint3dCP range) = 0;
    virtual void _DrawArc2d(DEllipse3dCR ellipse, bool isEllipse, bool filled, double zDepth, DPoint2dCP range) = 0;
    virtual void _DrawBSplineCurve(MSBsplineCurveCR curve, bool filled) = 0;
    virtual void _DrawBSplineCurve2d(MSBsplineCurveCR curve, bool filled, double zDepth) = 0;
    virtual void _DrawCurveVector(CurveVectorCR curves, bool isFilled) = 0;
    virtual void _DrawCurveVector2d(CurveVectorCR curves, bool isFilled, double zDepth) = 0;
    virtual void _DrawSolidPrimitive(ISolidPrimitiveCR primitive) = 0;
    virtual void _DrawBSplineSurface(MSBsplineSurfaceCR surface) = 0;
    virtual void _DrawPolyface(PolyfaceQueryCR meshData, bool filled = false) = 0;
    virtual StatusInt _DrawBody(ISolidKernelEntityCR, double pixelSize = 0.0) = 0;
    virtual void _DrawTextString(TextStringCR text, double* zDepth = NULL) = 0;
    virtual void _DrawMosaic(int numX, int numY, uintptr_t const* tileIds, DPoint3d const* verts) = 0;
    virtual void _PushTransClip(TransformCP trans, ClipPlaneSetCP clip = NULL) = 0;
    virtual void _PopTransClip() = 0;
    virtual RangeResult _PushBoundingRange3d(DPoint3dCP range) = 0;
    virtual RangeResult _PushBoundingRange2d(DPoint2dCP range, double zDepth) = 0;
    virtual void _PopBoundingRange() = 0;
    virtual size_t _GetMethodIndex() {return 0;}
    virtual void _PushMethodState() {}
    virtual void _PopMethodState() {}
    virtual ~IDrawGeom() {}

public:

    DGNPLATFORM_EXPORT IDrawGeom();

    //! Get the current View Flags for this object. The view flags are initialized from the view flags
    //! of its controlling DgnViewport at the beginning of every display operation. However, during display operations,
    //! the view flags are sometimes temporarily modified for specific purposes, so they are not
    //! always the same.
    //! @return the current view flags for this IViewDraw
    DGNPLATFORM_EXPORT ViewFlagsCP GetDrawViewFlags();

    //! Sets the current state of the ViewFlags for this context's output.
    DGNPLATFORM_EXPORT void SetDrawViewFlags(ViewFlagsCP);

    //! Set an ElemMatSymb to be the "active" ElemMatSymb for this IDrawGeom.
    //! @param[in]          matSymb     The new active ElemMatSymb. All geometry drawn via calls to this IDrawGeom will
    //!                                     be displayed using the values in this ElemMatSymb.
    //! @note     See discussion of the symbology "overrides" in #ActivateOverrideMatSymb
    DGNPLATFORM_EXPORT void ActivateMatSymb(ElemMatSymbCP matSymb);

    //! Set an ElemMatSymb to be the "active override" ElemMatSymb for this IDrawGeom.
    //! @param[in]          ovrMatSymb  The new active override ElemMatSymb.
    //!                                     value in ovrMatSymb will be used instead of the value set by #ActivateMatSymb.
    DGNPLATFORM_EXPORT void ActivateOverrideMatSymb(OvrMatSymbCP ovrMatSymb);

    //! Draw a 3D line string.
    //! @param[in]          numPoints   Number of vertices in points array.
    //! @param[in]          points      Array of vertices in the line string.
    //! @param[in]          range       Array of 2 points with the range (min followed by max) of the vertices in \c points. This argument is
    //!                                     optional and is only used to speed processing. If you do not already have the range of your points, pass NULL.
    DGNPLATFORM_EXPORT void DrawLineString3d(int numPoints, DPoint3dCP points, DPoint3dCP range);

    //! Draw a 2D line string.
    //! @param[in]          numPoints   Number of vertices in points array.
    //! @param[in]          points      Array of vertices in the line string.
    //! @param[in]          zDepth      Z depth value in local coordinates.
    //! @param[in]          range       Array of 2 points with the range (min followed by max) of the vertices in \c points. This argument is
    //!                                     optional and is only used to speed processing. If you do not already have the range of your points, pass NULL.
    DGNPLATFORM_EXPORT void DrawLineString2d(int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range);

    //! Draw a 3D point string. A point string is displayed as a series of points, one at each vertex in the array, with no vectors connecting the vertices.
    //! @param[in]          numPoints   Number of vertices in points array.
    //! @param[in]          points      Array of vertices in the point string.
    //! @param[in]          range       Array of 2 points with the range (min followed by max) of the vertices in \c points. This argument is
    //!                                     optional and is only used to speed processing. If you do not already have the range of your points, pass NULL.
    DGNPLATFORM_EXPORT void DrawPointString3d(int numPoints, DPoint3dCP points, DPoint3dCP range);

    //! Draw a 2D point string. A point string is displayed as a series of points, one at each vertex in the array, with no vectors connecting the vertices.
    //! @param[in]          numPoints   Number of vertices in points array.
    //! @param[in]          points      Array of vertices in the point string.
    //! @param[in]          zDepth      Z depth value.
    //! @param[in]          range       Array of 2 points with the range (min followed by max) of the vertices in \c points. This argument is
    //!                                     optional and is only used to speed processing. If you do not already have the range of your points, pass NULL.
    DGNPLATFORM_EXPORT void DrawPointString2d(int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range);

    //! Draw a closed 3D shape.
    //! @param[in]          numPoints   Number of vertices in \c points array. If the last vertex in the array is not the same as the first vertex, an
    //!                                     additional vertex will be added to close the shape.
    //! @param[in]          points      Array of vertices of the shape.
    //! @param[in]          filled      If true, the shape will be drawn filled.
    //! @param[in]          range       Array of 2 points with the range (min followed by max) of the vertices in \c points. This argument is
    //!                                     optional and is only used to speed processing. If you do not already have the range of your points, pass NULL.
    DGNPLATFORM_EXPORT void DrawShape3d(int numPoints, DPoint3dCP points, bool filled, DPoint3dCP range);

    //! Draw a 2D shape.
    //! @param[in]          numPoints   Number of vertices in \c points array. If the last vertex in the array is not the same as the first vertex, an
    //!                                     additional vertex will be added to close the shape.
    //! @param[in]          points      Array of vertices of the shape.
    //! @param[in]          zDepth      Z depth value.
    //! @param[in]          filled      If true, the shape will be drawn filled.
    //! @param[in]          range       Array of 2 points with the range (min followed by max) of the vertices in \c points. This argument is
    //!                                     optional and is only used to speed processing. If you do not already have the range of your points, pass NULL.
    DGNPLATFORM_EXPORT void DrawShape2d(int numPoints, DPoint2dCP points, bool filled, double zDepth, DPoint2dCP range);

    //! Draw a 3D elliptical arc or ellipse.
    //! @param[in]          ellipse     arc data.
    //! @param[in]          isEllipse   If true, and if full sweep, then draw as an ellipse instead of an arc.
    //! @param[in]          filled      If true, and isEllipse is also true, then draw ellipse filled.
    //! @param[in]          range       Array of 2 points with the range (min followed by max) of the arc.
    //!                                     This argument is optional and is only used to speed processing. If you do not already have the range, pass NULL.
    DGNPLATFORM_EXPORT void DrawArc3d(DEllipse3dCR ellipse, bool isEllipse, bool filled, DPoint3dCP range);

    //! Draw a 2D elliptical arc or ellipse.
    //! @param[in]          ellipse     arc data.
    //! @param[in]          isEllipse   If true, and if full sweep, then draw as an ellipse instead of an arc.
    //! @param[in]          filled      If true, and isEllipse is also true, then draw ellipse filled.
    //! @param[in]          zDepth      Z depth value
    //! @param[in]          range       Array of 2 points with the range (min followed by max) of the arc.
    //!                                     This argument is optional and is only used to speed processing. If you do not already have the range, pass NULL.
    DGNPLATFORM_EXPORT void DrawArc2d(DEllipse3dCR ellipse, bool isEllipse, bool filled, double zDepth, DPoint2dCP range);

    //! Draw a BSpline curve.
    DGNPLATFORM_EXPORT void DrawBSplineCurve(MSBsplineCurveCR curve, bool filled);

    //! Draw a BSpline curve as 2d geometry with display priority.
    //! @note Only necessary for non-ICachedDraw calls to support non-zero display priority.
    DGNPLATFORM_EXPORT void DrawBSplineCurve2d(MSBsplineCurveCR curve, bool filled, double zDepth);

    //! Draw a curve vector.
    DGNPLATFORM_EXPORT void DrawCurveVector(CurveVectorCR curves, bool isFilled);

    //! Draw a curve vector as 2d geometry with display priority.
    //! @note Only necessary for non-ICachedDraw calls to support non-zero display priority.
    DGNPLATFORM_EXPORT void DrawCurveVector2d(CurveVectorCR curves, bool isFilled, double zDepth);

    //! Draw a light-weight surface or solid primitive.
    //! @remarks Solid primitives can be capped or uncapped, they include cones, torus, box, spheres, and sweeps.
    DGNPLATFORM_EXPORT void DrawSolidPrimitive(ISolidPrimitiveCR primitive);

    //! Draw a BSpline surface.
    DGNPLATFORM_EXPORT void DrawBSplineSurface(MSBsplineSurfaceCR);

    //! @note Caller is expected to define texture id for illuminated meshed, SetTextureId.
    //! @remarks Wireframe fill display supported for non-illuminated meshes.
    DGNPLATFORM_EXPORT void DrawPolyface(PolyfaceQueryCR meshData, bool filled = false);

    //! Draw a BRep surface/solid entity from the solids kernel.
    //! @note Only implemented for ICachedDraw due to potentially expensive/time consuming solids kernel calls.
    DGNPLATFORM_EXPORT StatusInt DrawBody(ISolidKernelEntityCR, double pixelSize = 0.0);

    //! Draw a series of Glyphs
    //! @param[in]          text        Text drawing parameters
    //! @param[in]          zDepth      Priority value in 2d or NULL
    DGNPLATFORM_EXPORT void DrawTextString(TextStringCR text, double* zDepth = NULL);

    //! Draw a filled triangle strip from 3D points.
    //! @param[in]          numPoints   Number of vertices in \c points array.
    //! @param[in]          points      Array of vertices.
    //! @param[in]          usageFlags  0 or 1 if tri-strip represents a thickened line.
    //! @param[in]          range       Array of 2 points with the range (min followed by max) of the vertices in \c points. This argument is
    //!                                     optional and is only used to speed processing. If you do not already have the range of your points, pass NULL.
    DGNPLATFORM_EXPORT void DrawTriStrip3d(int numPoints, DPoint3dCP points, int32_t usageFlags, DPoint3dCP range);

    //! Draw a filled triangle strip from 2D points.
    //! @param[in]          numPoints   Number of vertices in \c points array.
    //! @param[in]          points      Array of vertices.
    //! @param[in]          zDepth      Z depth value.
    //! @param[in]          usageFlags  0 or 1 if tri-strip represents a thickened line.
    //! @param[in]          range       Array of 2 points with the range (min followed by max) of the vertices in \c points. This argument is
    //!                                     optional and is only used to speed processing. If you do not already have the range of your points, pass NULL.
    DGNPLATFORM_EXPORT void DrawTriStrip2d(int numPoints, DPoint2dCP points, int32_t usageFlags, double zDepth, DPoint2dCP range);


    DGNPLATFORM_EXPORT RangeResult  PushBoundingRange3d(DPoint3dCP range);
    DGNPLATFORM_EXPORT RangeResult  PushBoundingRange2d(DPoint2dCP range, double zDepth);
    DGNPLATFORM_EXPORT void         PopBoundingRange();
    size_t GetMethodIndex();
    void PushMethodState();
    void PopMethodState();

    //! @private
    // Published to expose access for performance reasons for Bryan Oswalt's augmented reality prototyping.
    DGNPLATFORM_EXPORT void DrawMosaic(int numX, int numY, uintptr_t const* tileIds, DPoint3d const* verts);

    // Helper Methods to draw simple SolidPrimitives.
    DGNPLATFORM_EXPORT void DrawTorus(DPoint3dCR center, DVec3dCR vectorX, DVec3dCR vectorY, double majorRadius, double minorRadius, double sweepAngle, bool capped);
    DGNPLATFORM_EXPORT void DrawCone(DPoint3dCR centerA, DPoint3dCR centerB, double radiusA, double radiusB, bool capped);
    DGNPLATFORM_EXPORT void DrawSphere(DPoint3dCR center, RotMatrixCR axes, double radius);
    DGNPLATFORM_EXPORT void DrawBox(DVec3dCR primary, DVec3dCR secondary, DPoint3dCR basePoint, DPoint3dCR topPoint, double baseWidth, double baseLength, double topWidth, double topLength, bool capped);
}; // IDrawGeom

//=======================================================================================
//! DgnCore implements this interface to provide the display system for Viewports. 
// @bsiclass 
//=======================================================================================
struct IViewDraw : IDrawGeom
{
protected:
    virtual void _SetToViewCoords(bool yesNo) = 0;
    virtual void _SetSymbology(ColorDef lineColor, ColorDef fillColor, int lineWidth, uint32_t linePattern) = 0;
    virtual void _DrawGrid(bool doIsoGrid, bool drawDots, DPoint3dCR gridOrigin, DVec3dCR xVector, DVec3dCR yVector, uint32_t gridsPerRef, Point2dCR repetitions) = 0;
    virtual bool _DrawSprite(ISprite* sprite, DPoint3dCP location, DPoint3dCP xVec, int transparency) = 0;
    virtual void _DrawTiledRaster(ITiledRaster* tiledRaster) = 0;
    virtual void _DrawRaster2d(DPoint2d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, Byte const* texels, double zDepth, DPoint2d const *range) = 0;
    virtual void _DrawRaster(DPoint3d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, Byte const* texels, DPoint3dCP range) = 0;
    virtual void _DrawDgnOle(IDgnOleDraw*) = 0;
    virtual void _DrawPointCloud(IPointCloudDrawParams* drawParams) = 0;
    virtual void _DrawQvElem(QvElem* qvElem, int subElemIndex) = 0;
    virtual void _ClearZ () = 0;

    virtual uintptr_t _DefineQVTexture(WCharCP textureName, DgnDbP) {return 0;}
    virtual void _DefineQVGeometryMap(uintptr_t textureId, IStrokeForCache&, DRange2dCR range, bool useCellColors, ViewContextR seedContext, bool forAreaPattern) {}

    virtual bool _IsOutputQuickVision() const = 0;
    virtual bool _ApplyMonochromeOverrides(ViewFlagsCR) const = 0;
    virtual StatusInt _TestOcclusion(int numVolumes, DPoint3dP verts, int* results) = 0;
    virtual void _PushClipStencil(QvElem* qvElem) = 0;
    virtual void _PopClipStencil() = 0;
    virtual ~IViewDraw() {}

public:
    //! Set the coordinate system temporarily to DgnCoordSystem::View. This removes the root coordinate system,
    //! including all camera definitions. It is ONLY valid or useful for drawing "overlay" graphics while drawing View Decorations.
    //! @param[in]      yesNo       If true, set to DgnCoordSystem::View. If false, restore to COORDSYS_Root.
    //! @note           calls to this method should be paired with true then false values for \c yesNo.
    DGNPLATFORM_EXPORT void SetToViewCoords(bool yesNo);

    //! Set the active symbology for this IViewDraw. All subsequent draw methods will use the new active symbology.
    //! @param[in]      lineColorTBGR   TBGR line color.
    //! @param[in]      fillColorTBGR   TBGR color for filled regions.
    //! @param[in]      lineWidth       The line width in pixels.
    //! @param[in]      linePattern     The 32 bit on/off pattern for lines.
    DGNPLATFORM_EXPORT void SetSymbology(ColorDef lineColorTBGR, ColorDef fillColorTBGR, int lineWidth, uint32_t linePattern);

    //! Draw the grid matrix.
    //! @param[in]      doIsoGrid       Draw the isometric grid points (if applicable).
    //! @param[in]      drawDots        Draw grid dots.
    //! @param[in]      gridOrigin      Point for the origin of the grid matrix.
    //! @param[in]      xVector         Direction of grid X.
    //! @param[in]      yVector         Direction of grid y.
    //! @param[in]      gridsPerRef     Draw reference lines.
    //! @param[in]      repetitions     X,y values for number or repetitions.
    DGNPLATFORM_EXPORT void DrawGrid(bool doIsoGrid, bool drawDots, DPoint3dCR gridOrigin, DVec3dCR xVector, DVec3dCR yVector, uint32_t gridsPerRef, Point2dCR repetitions);

    //! Draw a sprite at a specific location.
    //! @param[in]      sprite          The sprite definition.
    //! @param[in]      location        The location where the origin of the sprite should be drawn (in DgnCoordSystem::View coordinates.)
    //! @param[in]      xVec            A vector that points in the direction (in DgnCoordSystem::View coordinates) that the x vector
    //!                                     of the sprite definition should be oriented. This allows for rotating sprites. If NULL, sprite is draw
    //!                                     unrotated.
    //! @param[in]      transparency    Sprite is drawn with this transparency  (0=opaque, 255=invisible).
    //! @note this method is only valid from View Decorators.
    DGNPLATFORM_EXPORT bool DrawSprite(ISprite* sprite, DPoint3dCP location, DPoint3dCP xVec, int transparency);

    //! Draw a Tiled Raster.
    //! @param[in]      tiledRaster     The Tiled Raster to draw.
    DGNPLATFORM_EXPORT void DrawTiledRaster(ITiledRaster* tiledRaster);

    DGNPLATFORM_EXPORT void DrawRaster2d(DPoint2d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, Byte const* texels, double zDepth, DPoint2dCP range);
    DGNPLATFORM_EXPORT void DrawRaster(DPoint3d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, Byte const* texels, DPoint3dCP range);

    //! Draw a 3D point cloud.
    //! @param[in]      drawParams      Object containing draw parameters.
    DGNPLATFORM_EXPORT void DrawPointCloud(IPointCloudDrawParams* drawParams);

    //! Draw a QvElem
    DGNPLATFORM_EXPORT void DrawQvElem(QvElem* qvElem, int subElemIndex = 0);

    //! Draw OLE object.
    DGNPLATFORM_EXPORT void DrawDgnOle(IDgnOleDraw*);

    DGNPLATFORM_EXPORT void DrawTile(uintptr_t tileId, DPoint3d const* verts);

    //! Push the supplied QvElem as a clip stencil boundary.
    DGNPLATFORM_EXPORT void PushClipStencil(QvElem* qvElem);

    //! Pop the most recently pushed clip stencil boundary.
    DGNPLATFORM_EXPORT void PopClipStencil();

    DGNPLATFORM_EXPORT StatusInt TestOcclusion(int numVolumes, DPoint3dP verts, int* results);

    DGNPLATFORM_EXPORT void ClearZ ();
    DGNPLATFORM_EXPORT uintptr_t DefineQVTexture(WCharCP textureName, DgnDbP dgnFile);
    DGNPLATFORM_EXPORT void DefineQVGeometryMap(uintptr_t textureId, IStrokeForCache&, DRange2dCR range, bool useCellColors, ViewContextR seedContext, bool forAreaPattern = false);
    DGNPLATFORM_EXPORT bool IsOutputQuickVision() const;
    bool ApplyMonochromeOverrides(ViewFlagsCR) const;
}; // IViewDraw

//=======================================================================================
//! Begin/End announcements around cached drawing sequences.
// @bsiclass 
//=======================================================================================
struct ICachedDraw : IRefCounted, IDrawGeom
{
private:
    virtual QvElem* _GetCacheElement() = 0;
    virtual void    _SetCacheElement(QvElem*) = 0;

protected:
    virtual void    _BeginCacheElement(QvCache*) = 0;
    virtual QvElem* _EndCacheElement() = 0;
    virtual void    _AssignElementToView(QvView*, QvElem*, int viewMode) = 0;

public:
    DGNPLATFORM_EXPORT void    BeginCacheElement(QvCache*);
    DGNPLATFORM_EXPORT QvElem* EndCacheElement();
    DGNPLATFORM_EXPORT void    AssignElementToView(QvView*, QvElem*, int viewMode = 0);

    //! Push a transform.
    //! @param[in]  trans Transform to push.
    //! @see #PopTransform
    DGNPLATFORM_EXPORT void PushTransform(TransformCR trans);

    //! Pop the most recently pushed transform.
    //! @see #PushTransform
    DGNPLATFORM_EXPORT void PopTransform();

    //__PUBLISH_SECTION_END__
    DGNPLATFORM_EXPORT QvElem* GetCacheElement();
    DGNPLATFORM_EXPORT void    SetCacheElement(QvElem*);
    //__PUBLISH_SECTION_START__
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
