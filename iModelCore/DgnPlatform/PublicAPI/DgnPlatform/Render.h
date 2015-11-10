/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/Render.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnModel.h"
#include "DgnCategory.h"
#include "ImageUtilities.h"
#include "AreaPattern.h"
#include <list>

BEGIN_BENTLEY_RENDER_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(ElemDisplayParams)
DEFINE_POINTER_SUFFIX_TYPEDEFS(ElemMatSymb)
DEFINE_POINTER_SUFFIX_TYPEDEFS(GradientSymb)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Graphic)
DEFINE_POINTER_SUFFIX_TYPEDEFS(ISprite)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Image)
DEFINE_POINTER_SUFFIX_TYPEDEFS(LineStyleInfo)
DEFINE_POINTER_SUFFIX_TYPEDEFS(LineStyleParams)
DEFINE_POINTER_SUFFIX_TYPEDEFS(LineStyleSymb)
DEFINE_POINTER_SUFFIX_TYPEDEFS(LineTexture)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Material)
DEFINE_POINTER_SUFFIX_TYPEDEFS(MultiResImage)
DEFINE_POINTER_SUFFIX_TYPEDEFS(OvrMatSymb)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Renderer)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Scene)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Target)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Task)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Texture)
DEFINE_POINTER_SUFFIX_TYPEDEFS(ITiledRaster)

DEFINE_REF_COUNTED_PTR(GradientSymb)
DEFINE_REF_COUNTED_PTR(Graphic)
DEFINE_REF_COUNTED_PTR(Image)
DEFINE_REF_COUNTED_PTR(LineStyleInfo)
DEFINE_REF_COUNTED_PTR(LineTexture)
DEFINE_REF_COUNTED_PTR(Material)
DEFINE_REF_COUNTED_PTR(MultiResImage)
DEFINE_REF_COUNTED_PTR(Renderer)
DEFINE_REF_COUNTED_PTR(Scene)
DEFINE_REF_COUNTED_PTR(Target)
DEFINE_REF_COUNTED_PTR(Task)
DEFINE_REF_COUNTED_PTR(Texture)

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   09/15
//=======================================================================================
struct RenderManager
{
    BeConditionVariable m_cv;
    std::deque<TaskPtr> m_tasks;

    DGNPLATFORM_EXPORT void AddTask(Task&);
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/15
//=======================================================================================
struct Task : IRefCounted, NonCopyableClass
{
    virtual Target& _GetTarget() = 0;
    virtual void _Render() = 0;
    virtual int _GetPriority() = 0;
};

//=======================================================================================
// @bsiclass                                                    BentleySystems
//=======================================================================================
struct Image : RefCounted<NonCopyableClass>
{
    enum class Format
        {
        Rgba = 0,
        Bgra = 1,
        Rgb  = 2,
        Bgr  = 3,
        Gray = 4,
        };
    static size_t BytesPerPixel(Format format)
        {
        switch (format)
            {
            case Format::Rgba:
            case Format::Bgra:
                return 4;
            case Format::Rgb:
            case Format::Bgr:
                return 3;
            }
        return 1;
        }

protected:
    uint32_t m_width;
    uint32_t m_height;
    Format   m_format;
    ByteStream m_image;

public:
    Image(uint32_t width, uint32_t height, uint8_t const* data=0, uint32_t size=0) : m_width(width), m_height(height), m_image(data, size) {}
    uint32_t GetWidth() const {return m_width;}
    uint32_t GetHeight() const {return m_height;}
    Format GetFormat() const {return m_format;}
    ByteStream const& GetByteStream() const {return m_image;}
    ByteStream& GetByteStreamR() {return m_image;}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   09/15
//=======================================================================================
struct Texture : RefCounted<NonCopyableClass>
{
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   09/15
//=======================================================================================
struct Material : RefCounted<NonCopyableClass>
{
    bvector<TexturePtr> m_mappedTextures;
    void AddMappedTexture(TextureR texture) {m_mappedTextures.push_back(&texture);}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   11/15
//=======================================================================================
struct LineTexture : RefCounted<NonCopyableClass>
{
    TexturePtr m_texture;
    LineTexture(Texture* texture) : m_texture(texture) {}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   08/15
//=======================================================================================
struct MultiResImage : RefCounted<NonCopyableClass>
{
};

//=======================================================================================
//! Line style parameters
//! @private
//=======================================================================================
struct LineStyleParams
{
    uint32_t    modifiers;      /* see STYLEMOD_... above              */
    uint32_t    reserved;
    double      scale;          /* Applied to all length values        */
    double      dashScale;      /* Applied to adjustable dash strokes  */
    double      gapScale;       /* Applied to adjustable gap strokes   */
    double      startWidth;     /* Taper start width                   */
    double      endWidth;       /* Taper end width                     */
    double      distPhase;      /* Phase shift by distance             */
    double      fractPhase;     /* Phase shift by fraction             */
    uint32_t    lineMask;       /* Multiline line mask                 */
    uint32_t    mlineFlags;     /* Multiline flags                     */
    DPoint3d    normal;
    RotMatrix   rMatrix;

    void Init()
        {
        memset(this, 0, sizeof(LineStyleParams));
        this->rMatrix.form3d[0][0] = this->rMatrix.form3d[1][1] = this->rMatrix.form3d[2][2] =
        this->scale = this->gapScale = this->dashScale = this->normal.z = 1.0;
        }

    //! Compare two LineStyleParams.
    DGNPLATFORM_EXPORT bool operator==(LineStyleParamsCR rhs) const;
    DGNPLATFORM_EXPORT void SetScale(double scale);
};

//=======================================================================================
//! Line style id and parameters
//=======================================================================================
struct LineStyleInfo : RefCountedBase
{
protected:
    DgnStyleId          m_styleId;
    LineStyleParams     m_styleParams; //!< modifiers for user defined linestyle (if applicable)
    DGNPLATFORM_EXPORT LineStyleInfo(DgnStyleId styleId, LineStyleParamsCP params);

public:
    DGNPLATFORM_EXPORT void CopyFrom(LineStyleInfoCR);

    //! Create an instance of a LineStyleInfo.
    DGNPLATFORM_EXPORT static LineStyleInfoPtr Create(DgnStyleId styleId, LineStyleParamsCP params);

    //! Compare two LineStyleInfo.
    DGNPLATFORM_EXPORT bool operator==(LineStyleInfoCR rhs) const;

    DGNPLATFORM_EXPORT DgnStyleId GetStyleId() const;
    DGNPLATFORM_EXPORT LineStyleParamsCP GetStyleParams() const;
};

struct ISprite;
struct DgnOleDraw;

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
        bool m_fill:1;   // If not set, fill is an opaque fill that matches sub-category appearance color...
        bool m_bgFill:1; // When set, fill is an opaque fill that matches current view background color...
        AppearanceOverrides() {memset(this, 0, sizeof(*this));}
        };

    AppearanceOverrides m_appearanceOverrides;          //!< flags for parameters that override SubCategory::Appearance.
    bool                m_resolved;                     //!< whether Resolve has established SubCategory::Appearance/effective values.
    DgnCategoryId       m_categoryId;                   //!< the Category Id on which the geometry is drawn.
    DgnSubCategoryId    m_subCategoryId;                //!< the SubCategory Id that controls the appearence of subsequent geometry.
    DgnMaterialId       m_materialId;                   //!< render material ID.
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
    LineStyleInfoPtr    m_styleInfo;                    //!< line style id plus modifiers.
    GradientSymbPtr     m_gradient;                     //!< gradient fill settings.
    PatternParamsPtr    m_pattern;                      //!< area pattern settings.

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
    void SetMaterialId(DgnMaterialId materialId) {m_appearanceOverrides.m_material = true; m_materialId = materialId;}
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

    //! Get gradient fill information. Valid when FillDisplay::Never != GetFillDisplay() and not nullptrptr.
    GradientSymbCP GetGradient() const {return m_gradient.get();}

    //! Get the area pattern params.
    PatternParamsCP GetPatternParams() const {return m_pattern.get();}

    //! Get the geometry class.
    DgnGeometryClass GetGeometryClass() const {return m_geometryClass;}

    //! Get line style information.
    LineStyleInfoCP GetLineStyle() const {BeAssert(m_appearanceOverrides.m_style || m_resolved); return m_styleInfo.get();}

    //! Get element weight.
    uint32_t GetWeight() const {BeAssert(m_appearanceOverrides.m_weight || m_resolved); return m_weight;}

    //! Get element transparency.
    double GetTransparency() const {return m_elmTransparency;}

    //! Get fill/gradient transparency.
    double GetFillTransparency() const {return m_fillTransparency;}

    //! Get render material.
    DgnMaterialId GetMaterialId() const {BeAssert(m_appearanceOverrides.m_material || m_resolved); return m_materialId; }

    //! Get element display priority (2d only).
    int32_t GetDisplayPriority() const {return m_elmPriority;}
}; // ElemDisplayParams

//=======================================================================================
//! This structure contains options (modifications) that can be applied
//! to existing line styles to change their appearance without changing the line style
//! definition. Most of the options pertain to the operation of the StrokePatternComponent
//! component but the plane definition and scale factors can be used by all components.
//=======================================================================================
struct LineStyleSymb
{
private:
    // NOTE: For performance, the constructor initializes members using:
    //         memset (&m_lStyle, 0, offsetof (LineStyleSymb, m_planeByRows)- offsetof (LineStyleSymb, m_lStyle));
    //         So it will be necessary to update it if first/last member are changed. */
    ILineStyleCP    m_lStyle;       // if nullptr, no linestyle active
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
    TexturePtr      m_texture;

public:
    DGNPLATFORM_EXPORT LineStyleSymb();

    DGNPLATFORM_EXPORT int FromResolvedElemDisplayParams(ElemDisplayParamsCR, ViewContextR context, DPoint3dCP, DPoint3dCP);
    DGNPLATFORM_EXPORT int FromNaturalElemDisplayParams(ElemDisplayParamsR, ViewContextR context, DPoint3dCP, DPoint3dCP);
    DGNPLATFORM_EXPORT int FromResolvedStyle(LineStyleInfoCP styleInfo, ViewContextR context, DPoint3dCP startTangent, DPoint3dCP endTangent);

    void Clear() {m_lStyle = nullptr; m_options.orgWidth = m_options.endWidth = false; m_texture = nullptr; }
    void Init(ILineStyleCP);

public:
    ILineStyleCP GetILineStyle() const {return m_lStyle;}
    void GetPlaneAsMatrixRows(RotMatrixR matrix) const {matrix = m_planeByRows;}
    DGNPLATFORM_EXPORT double GetScale() const;
    DGNPLATFORM_EXPORT double GetDashScale() const;
    DGNPLATFORM_EXPORT double GetGapScale() const;
    DGNPLATFORM_EXPORT double GetOriginWidth() const;
    DGNPLATFORM_EXPORT double GetEndWidth() const;
    double GetPhaseShift() const {return m_phaseShift;}
    double GetFractionalPhase() const {return m_autoPhase;}
    double GetMaxCompress() const {return m_maxCompress;}
    int GetNumIterations() const {return m_nIterate;}
    DGNPLATFORM_EXPORT double GetMaxWidth() const;
    double GetTotalLength() const {return m_totalLength;}
    DPoint3dCP GetStartTangent() const {return &m_startTangent;}
    DPoint3dCP GetEndTangent() const{return &m_endTangent;}
    Texture* GetTexture() const {return m_texture.get(); }
    bool IsScaled() const {return m_options.scale;}
    bool IsAutoPhase() const {return m_options.autoPhase;}
    bool IsCenterPhase() const{return m_options.centerPhase;}
    bool IsCosmetic() const {return m_options.cosmetic;}
    bool IsTreatAsSingleSegment() const {return m_options.treatAsSingleSegment;}
    bool HasDashScale() const {return m_options.dashScale;}
    bool HasGapScale() const {return m_options.gapScale;}
    bool HasOrgWidth() const {return m_options.orgWidth;}
    bool HasEndWidth() const{return m_options.endWidth;}
    bool IsElementClosed() const{return m_options.elementIsClosed; }
    bool IsCurve() const {return m_options.isCurve; }
    bool HasPhaseShift() const {return m_options.phaseShift;}
    bool HasIterationLimit() const {return m_options.iterationLimit;}
    bool HasPlane() const {return m_options.plane;}
    bool HasStartTangent() const {return m_options.startTangentSet;}
    bool HasEndTangent() const {return m_options.endTangentSet;}
    DGNPLATFORM_EXPORT void SetPlaneAsMatrixRows(RotMatrixCP);
    DGNPLATFORM_EXPORT void SetNormalVec(DPoint3dCP);
    DGNPLATFORM_EXPORT void SetOriginWidth(double width);
    DGNPLATFORM_EXPORT void SetEndWidth(double width);
    DGNPLATFORM_EXPORT void SetWidth(double width);
    DGNPLATFORM_EXPORT void SetScale(double scaleFactor);
    DGNPLATFORM_EXPORT void SetFractionalPhase(bool isOn, double fraction);
    DGNPLATFORM_EXPORT void SetCenterPhase(bool isOn);
    DGNPLATFORM_EXPORT void SetPhaseShift(bool isOn, double distance);
    DGNPLATFORM_EXPORT void SetTreatAsSingleSegment(bool yesNo);
    DGNPLATFORM_EXPORT void SetTangents(DPoint3dCP, DPoint3dCP);
    void SetLineStyle(ILineStyleCP lstyle) {m_lStyle = lstyle;}
    DGNPLATFORM_EXPORT void ConvertLineStyleToTexture(ViewContextR context, bool force);

    bool HasTrueWidth() const  {return HasOrgWidth() || HasEndWidth();}
    bool HasMaxCompress() const {return m_options.maxCompress;}
    bool ContinuationXElems() const {return m_options.continuationXElems;}
    void SetXElemPhase(double last) {m_xElemPhase = last; m_options.xElemPhaseSet=true;}
    void SetElementClosed(bool closed) {m_options.elementIsClosed = closed;}
    void SetIsCurve(bool isCurve) {m_options.isCurve = isCurve;}

    DGNPLATFORM_EXPORT void SetGapScale(double scaleFactor);
    DGNPLATFORM_EXPORT void SetDashScale(double scaleFactor);
    void SetTotalLength(double length) {m_totalLength = length;}
    DGNPLATFORM_EXPORT void SetCosmetic(bool cosmetic);
    DGNPLATFORM_EXPORT void ClearContinuationData();
    DGNPLATFORM_EXPORT void CheckContinuationData();
};

//=======================================================================================
//! DgnCore implements this class for setting/getting the Material and Symbology (ElemMatSymb) used to draw geometry.
//! An ElemMatSymb is the "cooked" material and symbology that determines the real appearance (e.g. color/width/raster pattern/linestyle,
//! etc.) used to draw all geometry.
//=======================================================================================
struct ElemMatSymb
{
private:
    bool                m_isFilled;
    bool                m_isBlankingRegion;
    uint32_t            m_rasterWidth;
    ColorDef            m_lineColor;
    ColorDef            m_fillColor;
    LineTexturePtr      m_lineTexture;
    MaterialPtr         m_material;
    LineStyleSymb       m_lStyleSymb;
    GradientSymbPtr     m_gradient;
    PatternParamsPtr    m_patternParams;

public:
    void Cook(ElemDisplayParamsCR, ViewContextR, DPoint3dCP startTan, DPoint3dCP endTan);

    DGNPLATFORM_EXPORT ElemMatSymb();
    DGNPLATFORM_EXPORT explicit ElemMatSymb(ElemMatSymbCR rhs);

    DGNPLATFORM_EXPORT void Init();

    //! Get the texture applied to lines for this ElemMatSymb
    LineTextureP GetLineTexture() const {return m_lineTexture.get();}

    //! Set a LineTexture for this ElemMatSymb
    void SetLineTexture(LineTextureP texture) {m_lineTexture = texture;}

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
    bool HasTrueWidth() const {return m_lStyleSymb.HasTrueWidth();}

    //! Determine whether the fill flag is on for this ElemMatSymb.
    bool IsFilled() const {return m_isFilled;}

    //! Determine whether the fill represents blanking region.
    bool IsBlankingRegion() const {return m_isBlankingRegion;}

    //! Get the LineStyleSymb from this ElemMatSymb.
    LineStyleSymbCR GetLineStyleSymb() const {return m_lStyleSymb;}

    //! Get the GradientSymb from this ElemMatSymb.
    GradientSymbCP GetGradientSymb() const {return m_gradient.get();}

    //! Get the render material.
    MaterialP GetMaterial() const {return m_material.get();}

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

    //! Get the LineStyleSymb from this ElemMatSymb for setting line style parameters.
    LineStyleSymbR GetLineStyleSymbR () {return m_lStyleSymb;}

    //! Set the render material.
    void SetMaterial(Material* material) {m_material = material;}

    //! Set area patterning parameters.
    void SetPatternParams(PatternParamsP patternParams) {m_patternParams = patternParams;}

    //@}
}; // ElemMatSymb


//=======================================================================================
// @bsiclass
//=======================================================================================
struct OvrMatSymb
{
    enum Flags : uint32_t//! flags to indicate the parts of a MatSymb that are to be overridden
    {
        FLAGS_None                   = (0),      //!< no overrides
        FLAGS_Color                  = (1<<0),   //!< override outline color
        FLAGS_ColorTransparency      = (1<<1),   //!< override outline color transparency
        FLAGS_FillColor              = (1<<2) | (0x80000000), //!< override fill color, override blanking fill with bg color
        FLAGS_FillColorTransparency  = (1<<3),   //!< override fill color transparency
        FLAGS_RastWidth              = (1<<4),   //!< override raster width
        FLAGS_Style                  = (1<<5),   //!< override style
        FLAGS_TrueWidth              = (1<<6),   //!< override true width
        FLAGS_ExtSymb                = (1<<7),   //!< override extended symbology
        FLAGS_RenderMaterial         = (1<<8),   //!< override render material
    };

private:
    uint32_t    m_flags;
    ElemMatSymb m_matSymb;

public:
    OvrMatSymb() : m_flags(FLAGS_None) {}
    ElemMatSymbCR GetMatSymb() const {return m_matSymb;}
    ElemMatSymbR GetMatSymbR () {return m_matSymb;}

public:
    //! Compare two OvrMatSymb.
    bool operator==(OvrMatSymbCR rhs) const {if (this == &rhs) return true; if (rhs.m_flags != m_flags) return false; return rhs.m_matSymb == m_matSymb;}

    uint32_t GetFlags() const{return m_flags;}
    ColorDef GetLineColor() const {return m_matSymb.GetLineColor();}
    ColorDef GetFillColor() const {return m_matSymb.GetFillColor();}
    uint32_t GetWidth() const {return m_matSymb.GetWidth();}
    MaterialPtr GetMaterial() const {return m_matSymb.GetMaterial();}
    PatternParamsCP GetPatternParams() const {return m_matSymb.GetPatternParams();}

    DGNPLATFORM_EXPORT void Clear();
    void SetFlags(uint32_t flags) {m_flags = flags;}
    void SetLineColor(ColorDef color) {m_matSymb.SetLineColor(color); m_flags |=  FLAGS_Color;}
    void SetFillColor(ColorDef color) {m_matSymb.SetFillColor(color); m_flags |= FLAGS_FillColor;}
    void SetLineTransparency(Byte trans) {m_matSymb.SetLineTransparency(trans); m_flags |= FLAGS_ColorTransparency;}
    void SetFillTransparency(Byte trans) {m_matSymb.SetFillTransparency(trans); m_flags |= FLAGS_FillColorTransparency;}
    void SetWidth(uint32_t width) {m_matSymb.SetWidth(width); m_flags |= FLAGS_RastWidth;}
    void SetMaterial(Material* material) {m_matSymb.SetMaterial(material); m_flags |= FLAGS_RenderMaterial;}
    void SetPatternParams(PatternParamsP patternParams) {m_matSymb.SetPatternParams(patternParams);}
    DGNPLATFORM_EXPORT void SetLineStyle(int32_t styleNo, DgnModelR modelRef, DgnModelR styleDgnModel, LineStyleParamsCP lStyleParams, ViewContextR context, DPoint3dCP startTangent, DPoint3dCP endTangent);
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct PointCloudDraw
{
    enum VersionNumber {IPointCloudDrawParams_InitialVersion = 1};

    // If IsThreadBound returns false, implement AddRef and Release using InterlockedIncrement
    virtual uint32_t AddRef() = 0;
    virtual uint32_t Release() = 0;
    virtual bool IsThreadBound() = 0; // If true, always executed in calling thread instead of QV thread
    virtual VersionNumber GetVersion() { return IPointCloudDrawParams_InitialVersion; }
    virtual bool GetRange(DPoint3dP range) = 0; // returns false if it does not have range

    //  Added to points returned by GetPoints or GetFPoints
    virtual bool GetOrigin(DPoint3dP origin) = 0; // returns false if no origin

    virtual ColorDef const* GetRgbColors() = 0; // Returns nullptr if not using colors

    virtual uint32_t GetNumPoints() = 0;
    virtual DPoint3dCP GetDPoints() = 0; // Returns nullptr if using floats
    virtual FPoint3dCP GetFPoints() = 0; // Returns nullptr if using doubles
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct Graphic : RefCounted<NonCopyableClass>
{
    friend struct ViewContext;
    struct CreateParams
    {
        DgnViewportCP m_vp;
        Transform     m_placement;
        double        m_pixelSize;
        CreateParams(DgnViewportCP vp=nullptr, TransformCR placement=Transform::FromIdentity(), double pixelSize=0.0) : m_vp(vp), m_pixelSize(pixelSize), m_placement(placement) {}
    };

    struct Overrides : RefCountedBase {};
    DEFINE_REF_COUNTED_PTR(Overrides)

protected:
    DgnViewportCP m_vp;
    double        m_pixelSize;

    virtual StatusInt _FinishDrawing() {return SUCCESS;}
    virtual void _ActivateMatSymb(ElemMatSymbCP matSymb) = 0;
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
    virtual void _DrawTextString(TextStringCR text, double* zDepth = nullptr) = 0;
    virtual void _DrawMosaic(int numX, int numY, uintptr_t const* tileIds, DPoint3d const* verts) = 0;
    virtual bool _IsQuickVision() const {return false;}
    virtual bool _IsValidFor(DgnViewportCR vp, double metersPerPixel) const {return true;}
    virtual void _DrawRaster2d(DPoint2d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, Byte const* texels, double zDepth, DPoint2d const *range) = 0;
    virtual void _DrawRaster(DPoint3d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, Byte const* texels, DPoint3dCP range) = 0;
    virtual void _DrawDgnOle(DgnOleDraw*) = 0;
    virtual void _DrawPointCloud(PointCloudDraw* drawParams) = 0;
    virtual ~Graphic() {}

public:
    StatusInt FinishDrawing() {return _FinishDrawing();}
    explicit Graphic(CreateParams const& params=CreateParams()) : m_vp(params.m_vp), m_pixelSize(params.m_pixelSize) {}

    bool IsValidFor(DgnViewportCR vp, double metersPerPixel) const {return _IsValidFor(vp, metersPerPixel);}

    //! Set an ElemMatSymb to be the "active" ElemMatSymb for this IDrawGeom.
    //! @param[in]          matSymb     The new active ElemMatSymb. All geometry drawn via calls to this IDrawGeom will
    //!                                     be displayed using the values in this ElemMatSymb.
    void ActivateMatSymb(ElemMatSymbCP matSymb) {_ActivateMatSymb(matSymb);}

    //! Draw a 3D line string.
    //! @param[in]          numPoints   Number of vertices in points array.
    //! @param[in]          points      Array of vertices in the line string.
    //! @param[in]          range       Array of 2 points with the range (min followed by max) of the vertices in \c points. This argument is
    //!                                     optional and is only used to speed processing. If you do not already have the range of your points, pass nullptr.
    void DrawLineString3d(int numPoints, DPoint3dCP points, DPoint3dCP range) {_DrawLineString3d(numPoints, points, range);}

    //! Draw a 2D line string.
    //! @param[in]          numPoints   Number of vertices in points array.
    //! @param[in]          points      Array of vertices in the line string.
    //! @param[in]          zDepth      Z depth value in local coordinates.
    //! @param[in]          range       Array of 2 points with the range (min followed by max) of the vertices in \c points. This argument is
    //!                                     optional and is only used to speed processing. If you do not already have the range of your points, pass nullptr.
    void DrawLineString2d(int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range) {_DrawLineString2d(numPoints, points, zDepth, range);}

    //! Draw a 3D point string. A point string is displayed as a series of points, one at each vertex in the array, with no vectors connecting the vertices.
    //! @param[in]          numPoints   Number of vertices in points array.
    //! @param[in]          points      Array of vertices in the point string.
    //! @param[in]          range       Array of 2 points with the range (min followed by max) of the vertices in \c points. This argument is
    //!                                     optional and is only used to speed processing. If you do not already have the range of your points, pass nullptr.
    void DrawPointString3d(int numPoints, DPoint3dCP points, DPoint3dCP range) {_DrawPointString3d(numPoints, points, range);}

    //! Draw a 2D point string. A point string is displayed as a series of points, one at each vertex in the array, with no vectors connecting the vertices.
    //! @param[in]          numPoints   Number of vertices in points array.
    //! @param[in]          points      Array of vertices in the point string.
    //! @param[in]          zDepth      Z depth value.
    //! @param[in]          range       Array of 2 points with the range (min followed by max) of the vertices in \c points. This argument is
    //!                                     optional and is only used to speed processing. If you do not already have the range of your points, pass nullptr.
    void DrawPointString2d(int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range) {_DrawPointString2d(numPoints, points, zDepth, range);}

    //! Draw a closed 3D shape.
    //! @param[in]          numPoints   Number of vertices in \c points array. If the last vertex in the array is not the same as the first vertex, an
    //!                                     additional vertex will be added to close the shape.
    //! @param[in]          points      Array of vertices of the shape.
    //! @param[in]          filled      If true, the shape will be drawn filled.
    //! @param[in]          range       Array of 2 points with the range (min followed by max) of the vertices in \c points. This argument is
    //!                                     optional and is only used to speed processing. If you do not already have the range of your points, pass nullptr.
    void DrawShape3d(int numPoints, DPoint3dCP points, bool filled, DPoint3dCP range) {_DrawShape3d(numPoints, points, filled, range);}

    //! Draw a 2D shape.
    //! @param[in]          numPoints   Number of vertices in \c points array. If the last vertex in the array is not the same as the first vertex, an
    //!                                     additional vertex will be added to close the shape.
    //! @param[in]          points      Array of vertices of the shape.
    //! @param[in]          zDepth      Z depth value.
    //! @param[in]          filled      If true, the shape will be drawn filled.
    //! @param[in]          range       Array of 2 points with the range (min followed by max) of the vertices in \c points. This argument is
    //!                                     optional and is only used to speed processing. If you do not already have the range of your points, pass nullptr.
    void DrawShape2d(int numPoints, DPoint2dCP points, bool filled, double zDepth, DPoint2dCP range) {_DrawShape2d(numPoints, points, filled, zDepth, range);}

    //! Draw a 3D elliptical arc or ellipse.
    //! @param[in]          ellipse     arc data.
    //! @param[in]          isEllipse   If true, and if full sweep, then draw as an ellipse instead of an arc.
    //! @param[in]          filled      If true, and isEllipse is also true, then draw ellipse filled.
    //! @param[in]          range       Array of 2 points with the range (min followed by max) of the arc.
    //!                                     This argument is optional and is only used to speed processing. If you do not already have the range, pass nullptr.
    void DrawArc3d(DEllipse3dCR ellipse, bool isEllipse, bool filled, DPoint3dCP range) {_DrawArc3d(ellipse, isEllipse, filled, range);}

    //! Draw a 2D elliptical arc or ellipse.
    //! @param[in]          ellipse     arc data.
    //! @param[in]          isEllipse   If true, and if full sweep, then draw as an ellipse instead of an arc.
    //! @param[in]          filled      If true, and isEllipse is also true, then draw ellipse filled.
    //! @param[in]          zDepth      Z depth value
    //! @param[in]          range       Array of 2 points with the range (min followed by max) of the arc.
    //!                                     This argument is optional and is only used to speed processing. If you do not already have the range, pass nullptr.
    void DrawArc2d(DEllipse3dCR ellipse, bool isEllipse, bool filled, double zDepth, DPoint2dCP range) {_DrawArc2d(ellipse, isEllipse, filled, zDepth, range);}

    //! Draw a BSpline curve.
    void DrawBSplineCurve(MSBsplineCurveCR curve, bool filled) {_DrawBSplineCurve(curve, filled);}

    //! Draw a BSpline curve as 2d geometry with display priority.
    //! @note Only necessary for non-ICachedDraw calls to support non-zero display priority.
    void DrawBSplineCurve2d(MSBsplineCurveCR curve, bool filled, double zDepth) {_DrawBSplineCurve2d(curve, filled, zDepth);}

    //! Draw a curve vector.
    void DrawCurveVector(CurveVectorCR curves, bool isFilled) {_DrawCurveVector(curves, isFilled);}

    //! Draw a curve vector as 2d geometry with display priority.
    //! @note Only necessary for non-ICachedDraw calls to support non-zero display priority.
    void DrawCurveVector2d(CurveVectorCR curves, bool isFilled, double zDepth) {_DrawCurveVector2d(curves, isFilled, zDepth);}

    //! Draw a light-weight surface or solid primitive.
    //! @remarks Solid primitives can be capped or uncapped, they include cones, torus, box, spheres, and sweeps.
    void DrawSolidPrimitive(ISolidPrimitiveCR primitive) {_DrawSolidPrimitive(primitive);}

    //! Draw a BSpline surface.
    void DrawBSplineSurface(MSBsplineSurfaceCR surface) {_DrawBSplineSurface(surface);}

    //! @note Caller is expected to define texture id for illuminated meshed, SetTextureId.
    //! @remarks Wireframe fill display supported for non-illuminated meshes.
    void DrawPolyface(PolyfaceQueryCR meshData, bool filled = false) {_DrawPolyface(meshData, filled);}

    //! Draw a BRep surface/solid entity from the solids kernel.
    //! @note Only implemented for ICachedDraw due to potentially expensive/time consuming solids kernel calls.
    StatusInt DrawBody(ISolidKernelEntityCR entity, double pixelSize = 0.0) {return _DrawBody(entity, pixelSize);}

    //! Draw a series of Glyphs
    //! @param[in]          text        Text drawing parameters
    //! @param[in]          zDepth      Priority value in 2d or nullptr
    void DrawTextString(TextStringCR text, double* zDepth = nullptr) {_DrawTextString(text, zDepth);}

    //! Draw a filled triangle strip from 3D points.
    //! @param[in]          numPoints   Number of vertices in \c points array.
    //! @param[in]          points      Array of vertices.
    //! @param[in]          usageFlags  0 or 1 if tri-strip represents a thickened line.
    //! @param[in]          range       Array of 2 points with the range (min followed by max) of the vertices in \c points. This argument is
    //!                                     optional and is only used to speed processing. If you do not already have the range of your points, pass nullptr.
    void DrawTriStrip3d(int numPoints, DPoint3dCP points, int32_t usageFlags, DPoint3dCP range) {_DrawTriStrip3d(numPoints, points, usageFlags, range);}

    //! Draw a filled triangle strip from 2D points.
    //! @param[in]          numPoints   Number of vertices in \c points array.
    //! @param[in]          points      Array of vertices.
    //! @param[in]          zDepth      Z depth value.
    //! @param[in]          usageFlags  0 or 1 if tri-strip represents a thickened line.
    //! @param[in]          range       Array of 2 points with the range (min followed by max) of the vertices in \c points. This argument is
    //!                                     optional and is only used to speed processing. If you do not already have the range of your points, pass nullptr.
    void DrawTriStrip2d(int numPoints, DPoint2dCP points, int32_t usageFlags, double zDepth, DPoint2dCP range) {_DrawTriStrip2d(numPoints, points, usageFlags, zDepth, range);}

    //! @private
    // Published to expose access for performance reasons for Bryan Oswalt's augmented reality prototyping.
    void DrawMosaic(int numX, int numY, uintptr_t const* tileIds, DPoint3d const* verts) {_DrawMosaic(numX, numY, tileIds, verts);}

    // Helper Methods to draw simple SolidPrimitives.
    void DrawTorus(DPoint3dCR center, DVec3dCR vectorX, DVec3dCR vectorY, double majorRadius, double minorRadius, double sweepAngle, bool capped) { DrawSolidPrimitive(*ISolidPrimitive::CreateDgnTorusPipe(DgnTorusPipeDetail(center, vectorX, vectorY, majorRadius, minorRadius, sweepAngle, capped)));}
    void DrawBox(DVec3dCR primary, DVec3dCR secondary, DPoint3dCR basePoint, DPoint3dCR topPoint, double baseWidth, double baseLength, double topWidth, double topLength, bool capped) {DrawSolidPrimitive(*ISolidPrimitive::CreateDgnBox(DgnBoxDetail::InitFromCenters(basePoint, topPoint, primary, secondary, baseWidth, baseLength, topWidth, topLength, capped))); }

    void DrawRaster2d(DPoint2d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, Byte const* texels, double zDepth, DPoint2dCP range) {_DrawRaster2d(points, pitch, numTexelsX, numTexelsY, enableAlpha, format, texels, zDepth, range);}
    void DrawRaster(DPoint3d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, Byte const* texels, DPoint3dCP range) {_DrawRaster(points, pitch, numTexelsX, numTexelsY, enableAlpha, format, texels, range);}

    //! Draw a 3D point cloud.
    //! @param[in]      drawParams      Object containing draw parameters.
    void DrawPointCloud(PointCloudDraw* drawParams) {_DrawPointCloud(drawParams);}

    //! Draw OLE object.
    void DrawDgnOle(DgnOleDraw* ole) {_DrawDgnOle(ole);}

    bool IsQuickVision() const {return _IsQuickVision();}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct Scene : NonCopyableClass
{
    struct Node
    {
        GraphicPtr m_graphic;
        Graphic::OverridesPtr m_overrides;
        explicit Node(Graphic& graphic, Graphic::Overrides* ovr=nullptr) : m_graphic(&graphic), m_overrides(ovr) {}
    };

protected:
    bvector<Node> m_scene;

    virtual void _SetToViewCoords(bool yesNo) = 0;
    virtual void _ActivateOverrideMatSymb(OvrMatSymbCP ovrMatSymb) = 0;
    virtual void _DrawGrid(bool doIsoGrid, bool drawDots, DPoint3dCR gridOrigin, DVec3dCR xVector, DVec3dCR yVector, uint32_t gridsPerRef, Point2dCR repetitions) = 0;
    virtual bool _DrawSprite(ISprite* sprite, DPoint3dCP location, DPoint3dCP xVec, int transparency) = 0;
    virtual void _DrawTiledRaster(ITiledRaster* tiledRaster) = 0;
    virtual void _PushClipStencil(Graphic* graphic) = 0;
    virtual void _PopClipStencil() = 0;
    virtual void _Render() = 0;
    virtual Target& _GetRenderTarget() = 0;
    DGNPLATFORM_EXPORT virtual void _AddGraphic(Graphic& graphic);
    DGNPLATFORM_EXPORT virtual void _DropGraphic(Graphic& graphic);
    DGNPLATFORM_EXPORT virtual void _Clear();
    virtual ~Scene() {_Clear();}

public:
    //! Set an ElemMatSymb to be the "active override" ElemMatSymb for this IDrawGeom.
    //! @param[in]          ovrMatSymb  The new active override ElemMatSymb.
    //!                                     value in ovrMatSymb will be used instead of the value set by ActivateMatSymb.
    void ActivateOverrideMatSymb(OvrMatSymbCP ovrMatSymb) {_ActivateOverrideMatSymb(ovrMatSymb);}

    //! Set the coordinate system temporarily to DgnCoordSystem::View. This removes the root coordinate system,
    //! including all camera definitions. It is ONLY valid or useful for drawing "overlay" graphics while drawing View Decorations.
    //! @param[in]      yesNo       If true, set to DgnCoordSystem::View. If false, restore to COORDSYS_Root.
    //! @note           calls to this method should be paired with true then false values for \c yesNo.
    void SetToViewCoords(bool yesNo) {_SetToViewCoords(yesNo);}

    //! Draw the grid matrix.
    //! @param[in]      doIsoGrid       Draw the isometric grid points (if applicable).
    //! @param[in]      drawDots        Draw grid dots.
    //! @param[in]      gridOrigin      Point for the origin of the grid matrix.
    //! @param[in]      xVector         Direction of grid X.
    //! @param[in]      yVector         Direction of grid y.
    //! @param[in]      gridsPerRef     Draw reference lines.
    //! @param[in]      repetitions     X,y values for number or repetitions.
    void DrawGrid(bool doIsoGrid, bool drawDots, DPoint3dCR gridOrigin, DVec3dCR xVector, DVec3dCR yVector, uint32_t gridsPerRef, Point2dCR repetitions) {_DrawGrid(doIsoGrid, drawDots, gridOrigin, xVector, yVector, gridsPerRef, repetitions);}

    //! Draw a sprite at a specific location.
    //! @param[in]      sprite          The sprite definition.
    //! @param[in]      location        The location where the origin of the sprite should be drawn (in DgnCoordSystem::View coordinates.)
    //! @param[in]      xVec            A vector that points in the direction (in DgnCoordSystem::View coordinates) that the x vector
    //!                                     of the sprite definition should be oriented. This allows for rotating sprites. If nullptr, sprite is draw
    //!                                     unrotated.
    //! @param[in]      transparency    Sprite is drawn with this transparency  (0=opaque, 255=invisible).
    //! @note this method is only valid from View Decorators.
    bool DrawSprite(ISprite* sprite, DPoint3dCP location, DPoint3dCP xVec, int transparency) {return _DrawSprite(sprite, location, xVec, transparency);}

    //! Draw a Tiled Raster.
    //! @param[in]      tiledRaster     The Tiled Raster to draw.
    void DrawTiledRaster(ITiledRaster* tiledRaster) {_DrawTiledRaster(tiledRaster);}

    DGNPLATFORM_EXPORT void DrawTile(uintptr_t tileId, DPoint3d const* verts);

    //! Push the supplied Graphic as a clip stencil boundary.
    void PushClipStencil(Graphic* graphic) {_PushClipStencil(graphic);}

    //! Pop the most recently pushed clip stencil boundary.
    void PopClipStencil() {_PopClipStencil();}

    void AddGraphic(Graphic& graphic) {_AddGraphic(graphic);}
    void DropGraphic(Graphic& graphic) {_DropGraphic(graphic);}
    void Clear() {_Clear();}
    void Render() {return _Render();}

    Target& GetRenderTarget() {return _GetRenderTarget();}

    bool ApplyMonochromeOverrides(ViewFlagsCR) const;

};

//=======================================================================================
//! Selects the output buffer for IViewDraw methods.
// @bsiclass
//=======================================================================================
enum class DgnDrawBuffer
{
    None         = 0,                    //!< Do not draw to any buffer.
    Screen       = 1,                    //!< The visible buffer.
    Dynamic      = 2,                    //!< Offscreen, usually implemented in hardware as the "back buffer" of a double-buffered context
    BackingStore = 4,                    //!< Non-drawable offscreen buffer. Holds a copy of the most recent scene.
    Drawing      = 8,                    //!< The offscreen drawable buffer.
};

/*=================================================================================**//**
* Draw modes for displaying information in viewports.
* @bsistruct
+===============+===============+===============+===============+===============+======*/
enum class DgnDrawMode
{
    Normal    = 0,
    Erase     = 1,
    Hilite    = 2,
    TempDraw  = 3,
    Flash     = 11,
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/14
//=======================================================================================
struct PaintOptions
{
private:
    DgnDrawBuffer m_buffer;
    DgnDrawMode   m_drawMode;
    BSIRectCP     m_subRect;
    bool          m_eraseBefore;
    bool          m_synchDrawingFromBs;
    bool          m_synchToScreen;
    bool          m_drawDecorations;
    bool          m_accumulateDirty;
    bool          m_showTransparent;
    bool          m_progressiveDisplay;
    mutable bool  m_lockVp;

public:
    PaintOptions(DgnDrawBuffer buffer=DgnDrawBuffer::None, BSIRectCP subRect=nullptr) : m_buffer(buffer), m_subRect(subRect)
        {
        m_drawMode = DgnDrawMode::Normal;
        m_eraseBefore = true;
        m_synchDrawingFromBs = true;
        m_accumulateDirty = true;
        m_drawDecorations = false;
        m_synchToScreen = false;
        m_showTransparent = true;
        m_progressiveDisplay = false;
        m_lockVp = false;
        }
    DgnDrawBuffer GetDrawBuffer() const{return m_buffer;}
    DgnDrawMode GetDrawMode() const {return m_drawMode;}
    BSIRectCP GetSubRect() const {return m_subRect;}
    bool WantEraseBefore() const {return m_eraseBefore;}
    bool WantSynchFromBackingStore() const {return m_synchDrawingFromBs;}
    bool WantAccumulateDirty() const {return m_accumulateDirty;}
    bool WantLockVp() const {return m_lockVp;}
    bool WantSynchToScreen() const {return m_synchToScreen;}
    bool WantDrawDecorations() const {return m_drawDecorations;}
    bool WantShowTransparent() const {return m_showTransparent;}
    bool IsProgressiveDisplay() const {return m_progressiveDisplay;}
    void SetDrawBuffer(DgnDrawBuffer buffer) {m_buffer = buffer;}
    void SetDrawMode(DgnDrawMode mode) {m_drawMode = mode;}
    void SetSubRect(BSIRectCP rect) {m_subRect = rect;}
    void SetEraseBefore(bool val) {m_eraseBefore = val;}
    void SetSynchFromBackingStore(bool val) {m_synchDrawingFromBs = val;}
    void SetAccumulateDirty(bool val) {m_accumulateDirty = val;}
    void SetLockVp(bool val) const {m_lockVp = val;}
    void SetSynchToScreen(bool val) {m_synchToScreen = val;}
    void SetDrawDecorations(bool val) {m_drawDecorations = val;}
    void SetShowTransparent(bool val) {m_showTransparent = val;}
    void SetProgressiveDisplay(bool val) {m_progressiveDisplay = val;}
    bool IsHiliteMode() const {return (m_drawMode == DgnDrawMode::Hilite);}
    bool IsEraseMode() const {return (m_drawMode == DgnDrawMode::Erase);}
};

enum class AntiAliasPref {Detect = 0, On = 1, Off = 2};

enum class DrawExportFlags
{
    UseDefault          = 0,
    ClipToFrustum       = 1,
    LinesAsPolys        = 2,
    DeferTransparent    = 4,
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   09/15
//=======================================================================================
struct SystemContext {};
struct RenderDevice {};
struct RenderWindow {};
struct RenderCursor {};
struct CursorSource {};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct Target : RefCounted<NonCopyableClass>
{
    typedef ImageUtilities::RgbImageInfo CapturedImageInfo;

protected:
    virtual Render::GraphicPtr _CreateGraphic(Render::Graphic::CreateParams const& params) = 0;
    virtual void _SetViewAttributes(ViewFlags viewFlags, ColorDef bgColor, AntiAliasPref aaLines, AntiAliasPref aaText) = 0;
    virtual RenderDevice* _GetRenderDevice() const = 0;
    virtual StatusInt _AssignRenderDevice(DgnViewportR) = 0;
    virtual void _AddLights(bool threeDview, RotMatrixCP rotMatrixP, DgnModelP model) = 0;
    virtual void _AdjustBrightness(bool useFixedAdaptation, double brightness) = 0;
    virtual void _DefineFrustum(DPoint3dCR frustPts, double fraction, bool is2d) = 0;
    virtual StatusInt _SynchDrawingFromBackingStore() = 0;
    virtual void _SynchDrawingFromBackingStoreAsynch() = 0;
    virtual StatusInt _SynchScreenFromDrawing() = 0;
    virtual void _SynchScreenFromDrawingAsynch() = 0;
    virtual bool _IsScreenDirty(BSIRect*) = 0;
    virtual bool _IsBackingStoreValid() const = 0;
    virtual void _SetBackingStoreValid(bool) = 0;
    virtual bool _IsAccelerated() const = 0;
    virtual void _ScreenDirtied(BSIRect const* rect) = 0;
    virtual void _AccumulateDirtyRegion(bool val) = 0;
    virtual void _ClearHealRegion() = 0;
    virtual void _SetNeedsHeal(BSIRect const* dirty) = 0;
    virtual void _HealComplete(bool aborted) = 0;
    virtual bool _CheckNeedsHeal(BSIRect* rect) = 0;
    virtual BentleyStatus _FillImageCaptureBuffer(bvector<unsigned char>& buffer, CapturedImageInfo& info, DRange2dCR screenBufferRange, Point2dCR outputImageSize, bool topDown) = 0;
    virtual MaterialPtr _GetMaterial(DgnMaterialId, DgnDbR) const = 0;
    virtual TexturePtr _GetTexture(DgnTextureId, DgnDbR) const = 0;
    virtual TexturePtr _CreateTileSection(Image*, bool enableAlpha) const = 0;
                                                
public:
    virtual double _GetCameraFrustumNearScaleLimit() = 0;
    virtual bool _WantInvertBlackBackground() {return false;}
    virtual Scene* _GetMainScene() {return nullptr;} // TEMPORARY!

    Render::GraphicPtr CreateGraphic(Render::Graphic::CreateParams const& params) {return _CreateGraphic(params);}
    void SetViewAttributes(ViewFlags viewFlags, ColorDef bgColor, bool usebgTexture, AntiAliasPref aaLines, AntiAliasPref aaText) {_SetViewAttributes(viewFlags, bgColor, aaLines, aaText);}
    RenderDevice* GetRenderDevice() const {return _GetRenderDevice();}
    StatusInt AssignRenderDevice(DgnViewportR vp) {return _AssignRenderDevice(vp);}
    void AddLights(bool threeDview, RotMatrixCP rotMatrixP, DgnModelP model = nullptr) {_AddLights(threeDview, rotMatrixP, model);}
    void AdjustBrightness(bool useFixedAdaptation, double brightness){_AdjustBrightness(useFixedAdaptation, brightness);}
    void DefineFrustum(DPoint3dCR frustPts, double fraction, bool is2d) {_DefineFrustum(frustPts, fraction, is2d);}
    void HealComplete(bool aborted) {_HealComplete(aborted);}
    StatusInt SynchDrawingFromBackingStore() {return _SynchDrawingFromBackingStore();}
    void SynchDrawingFromBackingStoreAsynch() {_SynchDrawingFromBackingStoreAsynch();}
    StatusInt SynchScreenFromDrawing() {return _SynchScreenFromDrawing();}
    void SynchScreenFromDrawingAsynch() {_SynchScreenFromDrawingAsynch();}
    bool IsScreenDirty(BSIRect* rect) {return _IsScreenDirty(rect);}
    bool IsBackingStoreValid() const {return _IsBackingStoreValid();}
    bool IsAccelerated() const {return _IsAccelerated();}
    void ScreenDirtied(BSIRect const* rect) {_ScreenDirtied(rect);}
    void AccumulateDirtyRegion(bool val) {_AccumulateDirtyRegion(val);}
    void ClearHealRegion() {_ClearHealRegion();}
    void SetNeedsHeal(BSIRectCP dirty) {_SetNeedsHeal(dirty);}
    BentleyStatus FillImageCaptureBuffer(bvector<unsigned char>& buffer, CapturedImageInfo& info, DRange2dCR screenBufferRange, Point2dCR outputImageSize, bool topDown) {return _FillImageCaptureBuffer(buffer, info, screenBufferRange, outputImageSize, topDown);}
    bool CheckNeedsHeal(BSIRectP rect){return _CheckNeedsHeal(rect);}
    MaterialPtr GetMaterial(DgnMaterialId id, DgnDbR dgndb) const {return _GetMaterial(id, dgndb);}
    TexturePtr GetTexture(DgnTextureId id, DgnDbR dgndb) const {return _GetTexture(id, dgndb);}
    TexturePtr CreateTileSection(Image* image, bool enableAlpha) const {return _CreateTileSection(image, enableAlpha);}
};

END_BENTLEY_RENDER_NAMESPACE
