/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/** @cond BENTLEY_SDK_Internal */

#include <cstddef>
#include "Render.h"
#include "LineStyleResource.r.h"
#include "DgnHost.h"
#include "ViewContext.h"  // For ILineStyleComponent
#include "DgnPlatform.r.h"
#include <DgnPlatform/DgnElement.h>
#include <DgnPlatform/ElementHandler.h>
#include <DgnPlatform/ECSqlStatementIterator.h>
#include <DgnPlatform/Tools/KeyTree.h>

#define LSID_DEFAULT        0
#define LSID_HARDWARE       0x80000000
#define LSID_HWMASK         0x0000000f

DGNPLATFORM_TYPEDEFS(LineStyleElement);
DGNPLATFORM_REF_COUNTED_PTR(LineStyleElement);


DGNPLATFORM_TYPEDEFS(LsCache)
DGNPLATFORM_TYPEDEFS(LsCacheStyleEntry)
DGNPLATFORM_TYPEDEFS(LsCacheStyleIterator)
DGNPLATFORM_TYPEDEFS(LsComponent)
DGNPLATFORM_TYPEDEFS(LsCompoundComponent)
DGNPLATFORM_TYPEDEFS(LsDefinition)
DGNPLATFORM_TYPEDEFS(LsInternalComponent)
DGNPLATFORM_TYPEDEFS(LsLocation)
DGNPLATFORM_TYPEDEFS(LsLineCodeComponent)
DGNPLATFORM_TYPEDEFS(LsOffsetComponent)
DGNPLATFORM_TYPEDEFS(LsPointComponent)
DGNPLATFORM_TYPEDEFS(LsRasterImageComponent)
DGNPLATFORM_TYPEDEFS(LsStroke)
DGNPLATFORM_TYPEDEFS(LsStrokePatternComponent)
DGNPLATFORM_TYPEDEFS(LsSymbolComponent)
DGNPLATFORM_TYPEDEFS(LsSymbolReference)

DGNPLATFORM_REF_COUNTED_PTR(LsComponent);
DGNPLATFORM_REF_COUNTED_PTR(LsPointComponent);
DGNPLATFORM_REF_COUNTED_PTR(LsStrokePatternComponent);
DGNPLATFORM_REF_COUNTED_PTR(LsSymbolComponent);
DGNPLATFORM_REF_COUNTED_PTR(LsCompoundComponent);
DGNPLATFORM_REF_COUNTED_PTR(LsInternalComponent);
DGNPLATFORM_REF_COUNTED_PTR(LsCache);
DGNPLATFORM_REF_COUNTED_PTR(LsRasterImageComponent);

BEGIN_BENTLEY_DGN_NAMESPACE

static Utf8CP LINESTYLE_PROP_Description = "Description";
static Utf8CP LINESTYLE_PROP_Data = "Data";

//! Special style numbers that form a subset of values that may passed to LineStyleManager::GetNameFromNumber() or returned from LineStyleManager::GetNumberFromName()
//! @ingroup LineStyleManagerModule
enum LsKnownStyleNumber
{
    STYLE_MinLineCode = 0,
    STYLE_MaxLineCode = 7,
    STYLE_ByLevel     = 0x7fffffff,
    STYLE_ByCell      = 0x7ffffffe,
    STYLE_Invalid     = 0x7fffff00
};

enum class LsOkayForTextureGeneration
{
    Unknown                 = -1,  //  Only used as component's cached value
    NoChangeRequired        = 0,
    ChangeRequired          = 1,
    NotAllowed              = 2,
};

//=======================================================================================
//! Used when stroking a line style.
//!
// @bsiclass
//=======================================================================================
struct LineStyleContext
{
private:
    Render::GraphicBuilderPtr   m_graphic;
    Render::GeometryParamsCR    m_params;
    ViewContextR                m_context;
    IFacetOptionsPtr            m_facetOptions;
    bool                        m_isTextureCreate = false;
    bool                        m_hasTextureColors = false;
public:
    LineStyleContext(Render::GraphicBuilderR graphic, Render::GeometryParamsCR params, ViewContextR context, IFacetOptionsP facetOptions = nullptr) : m_graphic(&graphic), m_params(params), m_context(context)
        {
        m_facetOptions = facetOptions;

        if (!m_facetOptions.IsValid())
            {
            m_facetOptions = IFacetOptions::CreateForCurves();
            m_facetOptions->SetAngleTolerance(Angle::FromDegrees(5.0).Radians()); // NOTE: Need a fairly small angle if not always re-stroking to a view tolerance...
            }
        }

    void SetCreatingTexture() {m_isTextureCreate = true;}
    bool GetCreatingTexture() const {return m_isTextureCreate;}
    void SetHasTextureColors() {m_hasTextureColors = true;}
    bool GetHasTextureColors() const {return m_hasTextureColors;}
    Render::GraphicBuilderR GetGraphicR() {return *m_graphic;}
    Render::GeometryParamsCR GetGeometryParams() {return m_params;}
    ViewContextR GetViewContext() {return m_context;}
    IFacetOptionsCR GetFacetOptions() {return *m_facetOptions;}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct LsJsonHelpers
{
    static Utf8CP CompId;

    static double GetDouble(JsonValueCR json, CharCP fieldName, double defaultValue);
    static uint32_t GetUInt32(JsonValueCR json, CharCP fieldName, uint32_t defaultValue);
    static int32_t GetInt32(JsonValueCR json, CharCP fieldName, int32_t defaultValue);
    static uint64_t GetUInt64(JsonValueCR json, CharCP fieldName, uint64_t defaultValue);
    static Utf8String GetString(JsonValueCR json, CharCP fieldName, CharCP defaultValue);
    static LsComponentId GetComponentId(JsonValueCR json, CharCP typeName, CharCP idName, LsComponentType defaultType = LsComponentType::Internal);
};

#pragma pack(push)
#pragma pack(1)
//  LineStyle definitions are kept in dgnPrj_Style entries with Type === DgnStyleType::Line (3)
//  An entry contains the name, description, ID that elements use to reference the
//  line style, and a JSon blob. Here is a sample line style blob:
//
//      {"compId":1,"compType":2,"flags":1,"unitDef":0.0}
//
//  flags is an unsigned int, unitDef is a double, compId and compType are unsigned ints
//
//  Components are all stored in the be_Prop table using namespace dgn_LStyle.  The name
//  field holds the component type: CompoundV1, SymbolEleV1, LineCodeV1, PointSymV1, or LinePointV1.
//
//  The blob in a CompoundV1 entry holds a V10Compound.
//  The blob in a PointSymV1 entry holds a V10Symbol.
//  The blob in a LineCodeV1 entry holds a V10LineCode.
//  The blob in a LinePointV1 entry holds a V10LinePoint.
//  The blob in a SymbolEleV1 contains a list of elements.
//
//  The types defining the blobs follow.  Note that these are all compiled with @pragma pack(1)
//

//=======================================================================================
//! The header for every component definition included in a DgnDb.
// @bsiclass
//=======================================================================================
struct V10ComponentBase
{
    enum V10ComponentVersioon
        {
        InitialDgnDb = 1,
        };

    uint32_t      m_version;
    Utf8Char      m_descr[LS_MAX_DESCR];
    void GetDescription (Utf8P target) const;
    DGNPLATFORM_EXPORT void SetDescription (Utf8CP target);
    DGNPLATFORM_EXPORT void SetVersion ();
};


//=======================================================================================
//! Describes the binary representation of LsRasterImageComponent component in a DgnDb
// @bsiclass
//=======================================================================================
struct V10RasterImage : V10ComponentBase
{
    uint32_t        m_flags;
    Point2d         m_size;
    double          m_trueWidth;
    double          m_reserved2[4];
    uint32_t        m_reserved1[4];

    uint32_t        m_nImageBytes;
    uint8_t         m_imageData[1];
    static uint32_t GetBufferSize(uint32_t m_nImageBytes) { return ((uint32_t)(intptr_t)&(((V10RasterImage*)nullptr)->m_imageData)) + m_nImageBytes; }
};

typedef struct
    {
    DPoint3d        low;
    DPoint3d        high;
    } SymbolRange;

#define LSSYM_3D                0x01    /* 3d symbol                   */
#define LSSYM_NOSCALE           0x02    /* DWG compat - don't scale symbol at all. */

#pragma pack(pop)

enum class LsCapMode
{
    //! 0 - Standard closed polygon (rectangle) strokes.
    Closed            = 0,
    //! 1 - No end cap. The stroke is displayed as two parallel lines.
    Open              = 1,
    //! 2 - The end of the stroke is extended by half the stroke width.
    Extended          = 2,
    //! >= 3 - cap stroked as an arc and the value indicates the number of vectors in the arc.
    Hexagon           = 3,
    //! 4 vectors in the arc
    Octagon           = 4,
    //! 5 vectors in the arc
    LCCAP_Decagon     = 5,
    Arc               = 30,
};


enum DwgShapeFlag
{
    SHAPEFLAG_TextSymbol    = 0x0002,
    SHAPEFLAG_ShapeSymbol   = 0x0004,
};

struct Centerline;
struct LsComponentReader;

typedef struct DecomposedDwgLine*               DecomposedDwgLineP;
typedef struct Centerline*                      CenterlineP;

//=======================================================================================
// @bsiclass
//=======================================================================================
struct  LsLocation
{
private:
    DgnDbP                  m_dgndb;
    LsComponentId           m_componentId;              // Component property ID

    void Init ()
        {
        m_componentId   = LsComponentId();
        m_dgndb         = NULL;
        }

public:
    ~LsLocation         ();
    LsLocation          () { Init (); }
    LsLocation          (LsLocationCP base) { SetFrom (base); }

    bool const operator < (LsLocation const &r ) const;
    void SetLocation    (DgnDbR project, LsComponentId componentId);

    void SetFrom        (LsLocationCP base, LsComponentId componentId);
    void SetFrom        (LsLocationCP base);
    DGNPLATFORM_EXPORT LsComponentId GetComponentId() const;
    DGNPLATFORM_EXPORT intptr_t GetFileKey() const;
    DGNPLATFORM_EXPORT LsComponentType GetComponentType() const;
    DgnDbP GetDgnDb () const {return m_dgndb;}
    LsCacheP GetCacheP () const;

    bool IsValid () const;

    bool IsInternalDefault () const {return (LsComponentType::Internal == m_componentId.GetType() && LSID_DEFAULT == GetComponentId().GetValue()) ? true : false;}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct          LsComponentReader
{
protected:
    LsLocationCP        m_source;
    DgnDbR              m_dgndb;
    LsComponentId       m_componentId;
    Utf8String          m_jsonSource;

public:
    LsComponentReader (LsLocationCP source, DgnDbR project) : m_dgndb(project)
    {
        m_source            = source;
        m_componentId       = m_source->GetComponentId();
    }

    virtual ~LsComponentReader();
    virtual BentleyStatus _LoadDefinition();

    LsLocationCP    GetSource()     {return m_source;}
    DgnDbR          GetDgnDb ()     {return m_dgndb; }
    void            GetJsonValue(JsonValueR componentDef);
    LsComponentType GetComponentType()  {return m_componentId.GetType();}
};


//=======================================================================================
// @bsiclass
//=======================================================================================
struct          LsComponent : public RefCountedBase
    , ILineStyleComponent
    {
private:
    LsLocation              m_location;    // Where to find components of resource
    Utf8String              m_descr;

    BentleyStatus       StrokeContinuousArc (LineStyleContextR, Render::LineStyleSymbCR, DEllipse3dCR arc, bool isClosed) const;

protected:
    bool                m_isDirty;

    // Should only be used for setting descr in resource definition
    void      CopyDescription (Utf8CP buffer);
    static void UpdateLsOkayForTextureGeneration(LsOkayForTextureGeneration&current, LsOkayForTextureGeneration const&newValue);
    virtual LsComponentPtr _Import(DgnImportContext& importer) const = 0;
    void SaveToJson(Json::Value& result) const;

public:
    void ExtractDescription(JsonValueCR result);
    LsComponent (DgnDbR, LsComponentId componentId);
    LsComponent (LsLocationCP location);
    LsComponent (LsComponent const* base) : m_isDirty (false)
        {
        m_location.SetFrom (&base->m_location);
        m_descr = base->m_descr;
        }

    DGNPLATFORM_EXPORT static void GetNextComponentNumber (uint32_t& id, DgnDbR project, BeSQLite::PropertySpec spec);
    DGNPLATFORM_EXPORT static LineStyleStatus AddComponentAsJsonProperty (LsComponentId& componentId, DgnDbR project, LsComponentType componentType, JsonValueCR jsonValue);
    DGNPLATFORM_EXPORT static LineStyleStatus AddRasterComponentAsJson (LsComponentId& componentId, DgnDbR project, JsonValueCR jsonDef, uint8_t const*imageData, uint32_t dataSize);

    LsLocationCP        GetLocation() const {return &m_location;}
    virtual             ~LsComponent() {};
    virtual bool        _IsBySegment() const {return false;}
    virtual bool        _HasLineCodes() const {return false;}
    virtual bool        _IsContinuousOrSingleDash() const {return false;}
    virtual double      _GetMaxWidth () const override {return 0.0;}
    virtual bool        _IsAffectedByWidth (bool currentStatusOnly) const {return false;}
    virtual bool        _ContainsComponent (LsComponentP other) const {return other == this;}
    virtual bool        _HasUniformFullWidth (double *pWidth) const  {if (pWidth) *pWidth=0.0; return false;}
    virtual double      _CalcRepetitions (Render::LineStyleSymbCP) const;
    virtual LsRasterImageComponentP      _GetRasterImageComponent () {return nullptr;}
    virtual bool        _HasRasterImageComponent () const {return false;}

    bool        _IsContinuous           () const override  {return false;}
    bool        _HasWidth               () const override  {return true;}
    double      _GetLength              () const override  {return 0.0;}
    virtual double      _GetLengthForTexture    () const           {return _GetLength();}
    virtual void        _PostProcessLoad        () { return; }
    virtual void        _ClearPostProcess       () { return; }
    StatusInt   _StrokeLineString       (LineStyleContextR, Render::LineStyleSymbR, DPoint3dCP, int nPts, bool isClosed) const override;
    StatusInt   _StrokeLineString2d     (LineStyleContextR, Render::LineStyleSymbR, DPoint2dCP, int nPts, double zDepth, bool isClosed) const override;
    StatusInt   _StrokeArc              (LineStyleContextR, Render::LineStyleSymbR, DEllipse3dCR, bool is3d, double zDepth, bool isClosed) const override;
    StatusInt   _StrokeBSplineCurve     (LineStyleContextR, Render::LineStyleSymbR, MSBsplineCurveCR, bool is3d, double zDepth) const override;
    virtual StatusInt   _DoStroke               (LineStyleContextR, DPoint3dCP, int, Render::LineStyleSymbR) const {return SUCCESS;}
    virtual void        _LoadFinished           () { m_isDirty = false; }
    virtual LsOkayForTextureGeneration _IsOkayForTextureGeneration() const = 0;
    virtual LsComponentPtr _GetForTextureGeneration() const = 0;
    virtual void _StartTextureGeneration() const = 0;
    virtual BentleyStatus _GetRasterTexture (uint8_t const*& image, Point2dR imageSize, uint32_t& flags) const { return BSIERROR; }
    virtual BentleyStatus _GetTextureWidth (double& width) const { return BSIERROR; }

    //  Defer until update supported
    DGNPLATFORM_EXPORT void SetDescription (Utf8StringCR descr) { m_descr = descr; }
    DGNPLATFORM_EXPORT void SetDescription (Utf8CP descr) { m_descr = Utf8String (descr); }

    DGNPLATFORM_EXPORT static bool IsValidComponentType(LsComponentType value);

    static LsComponentId Import(LsComponentId sourceId, DgnImportContext& importer);
    static LsComponentPtr GetImportedComponent(LsComponentId sourceId, DgnImportContext& importer);

public:

    //! Gets the component's resource ID if the definition comes from a resource file, or element ID
    //! if the definition comes from a DgnFile.
    DGNPLATFORM_EXPORT LsComponentId GetId () const;
    //! Retrieves the LsResourceType value specifying the type of component. This can be used regardless of
    //! whether the definition comes from a resource file or DgnFile.
    DGNPLATFORM_EXPORT LsComponentType GetComponentType () const;
    //! Retrieves a DgnDbP if the component is defined in a project file; NULL otherwise.
    DGNPLATFORM_EXPORT DgnDbP GetDgnDbP () const;
    //! Retrieves the name of the resource file or DgnFile that contains the component definition.
    DGNPLATFORM_EXPORT Utf8String GetFileName () const;
    //! Retrieves the description of the component; this is the description that is stored with the definition.
    DGNPLATFORM_EXPORT Utf8String GetDescription () const;
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct           LsRasterImageComponent : LsComponent
    {
    enum FlagMask
        {
        FlagMask_AlphaChannel     = 0x0003,
        FlagMask_AlphaOnly        = 0x0001 << 2,
        FlagMask_AlphaInvert      = 0x0001 << 3,
        FlagMask_TrueWidth        = 0x0001 << 4,
        };

private:
    Point2d             m_size;
    uint32_t            m_flags;
    double              m_trueWidth;
    uint32_t            m_imageDataId;
    bvector<uint8_t>    m_image;

    LsRasterImageComponent(LsRasterImageComponentCR);
    LsRasterImageComponent(LsLocationCP pLocation);
    LsRasterImageComponent(V10RasterImage* rasterImageResource, LsLocationCP location);

    DGNPLATFORM_EXPORT static LsRasterImageComponentPtr Create (BeFileNameCR fileName);
                       static LsRasterImageComponentPtr Create (LsLocation const& location) { LsRasterImageComponentP retVal = new LsRasterImageComponent (&location); retVal->m_isDirty = true; return retVal; }

protected:
    BentleyStatus   _GetRasterTexture (uint8_t const*& image, Point2dR imageSize, uint32_t& flags) const override;
    BentleyStatus   _GetTextureWidth (double& width) const override;
    bool            _HasWidth () const override  { return 0 != (m_flags & FlagMask_TrueWidth); }
    double          _GetMaxWidth () const override  { return _HasWidth() ? m_trueWidth : 0.0; }
    void _StartTextureGeneration() const override {}
    LsComponentPtr _GetForTextureGeneration() const override { return const_cast<LsRasterImageComponentP>(this); }
    LsOkayForTextureGeneration _IsOkayForTextureGeneration() const override { return LsOkayForTextureGeneration::NoChangeRequired; }
    LsComponentPtr _Import(DgnImportContext& importer) const override;
    virtual LsRasterImageComponentP      _GetRasterImageComponent () override {return this; }
    virtual bool        _HasRasterImageComponent () const override {return true;}

public:
    void SaveToJson(Json::Value& result, bvector<uint8_t>& imageData) const;
    static LineStyleStatus CreateFromJson(LsRasterImageComponentP*, Json::Value const & jsonDef, LsLocationCP location);
    static LsRasterImageComponent* LoadRasterImage  (LsComponentReader* reader);
    static BentleyStatus CreateRscFromDgnDb(V10RasterImage** rscOut, DgnDbR project, LsComponentId id);

    uint32_t        GetFlags() const            { return m_flags; }
    uint32_t        GetWidth() const            { return m_size.x; }
    uint32_t        GetHeight() const           { return m_size.y; }
    uint8_t const*  GetImage() const            { return &m_image.front(); }
    size_t          GetImageBufferSize () const { return 4 * m_size.x * m_size.y; }

public:

};  // LsRasterImageComponent

//=======================================================================================
//!  Represents a component that contains graphics.
//!  @ingroup LineStyleManagerModule
// @bsiclass
//=======================================================================================
struct          LsSymbolComponent : LsComponent
    {
private:
    bool                        m_isModified;
    DgnGeometryPartId           m_geomPartId;
    double                      m_storedScale;              //
    double                      m_muDef;                    // Set to m_storedScale if it is non-zero. Otherwise, it is 1/uorPerMaster for the model ref used in the PostProcessLoad step;
    DPoint3d                    m_symSize;
    DPoint3d                    m_symBase;                  // Not needed to display; used just to reconstruct range for GetRange method
    uint32_t                    m_symFlags;                 // Flags from point symbol resource
    bool                        m_postProcessed;

    explicit LsSymbolComponent (LsLocationCP pLocation);
    virtual ~LsSymbolComponent ();
    LsSymbolComponent(LsSymbolComponentCR src);

protected:
    LsComponentPtr _Import(DgnImportContext& importer) const override;

public:
    static LsSymbolComponent* LoadPointSym  (LsComponentReader* reader);
    static LsSymbolComponentPtr Create (LsLocation& location) { LsSymbolComponentP retval = new LsSymbolComponent (&location); retval->m_isDirty = true; return retval; }
    void SaveToJson(Json::Value& result) const;
    static LineStyleStatus CreateFromJson(LsSymbolComponentP*, Json::Value const & jsonDef, LsLocationCP location);

    double              GetMuDef            () const {return m_muDef;}
    DPoint3dCP          GetSymSize          () const {return &m_symSize;}
    uint32_t            GetFlags            () const {return m_symFlags;}
    bool                IsNotScaled         () const {return 0 != (m_symFlags & LSSYM_NOSCALE);}

    void                _PostProcessLoad    () override;
    void                _ClearPostProcess   () override;
    void                Draw                (LineStyleContextR, TransformCR, ClipVectorCP clip, bool ignoreColor, bool ignoreWeight);
    void                SetGeometryPartId   (DgnGeometryPartId id) {m_geomPartId = id;}
    DgnGeometryPartId   GetGeometryPartId   () const {return m_geomPartId;}
    DgnGeometryPartCPtr GetGeometryPart     () const;
    void                SetMuDef            (double mudef) {m_muDef = mudef;}
    void                SetSymSize          (DPoint3dCP sz){m_symSize = *sz;}
    void                SetSymBase          (DPoint3dCP sz){m_symBase = *sz;}
    void                SetFlags            (uint32_t flags) {m_symFlags = flags;}
    //  Should never be called; symbol components should never be drawn this way.  Therefore, a line style
    //  resource must not directly refer to a symbol component.
    StatusInt           _DoStroke           (LineStyleContextR, DPoint3dCP, int, Render::LineStyleSymbR) const override;

    LsComponentPtr _GetForTextureGeneration() const override { return const_cast<LsSymbolComponentP>(this); }
    LsOkayForTextureGeneration _IsOkayForTextureGeneration() const override { return LsOkayForTextureGeneration::NoChangeRequired; }
    void _StartTextureGeneration() const override {}
    DGNPLATFORM_EXPORT static void SaveSymbolDataToJson(Json::Value& result, DPoint3dCR base, DPoint3dCR size, DgnGeometryPartId const& geomPartId, int32_t flags, double storedScale);

public:
    //!  Retrieves the range that is stored with the LsSymbolComponent.  This range is computed
    //!  when the LsSymbolComponent is created.
    DGNPLATFORM_EXPORT void                     GetRange                (DRange3dR range) const;
    //!  Retrieves the scale factor that is stored with the LsSymbolComponent.
    DGNPLATFORM_EXPORT double                   GetStoredUnitScale      () const;
    //!  Sets the scale factor that is stored with the LsSymbolComponent.
    DGNPLATFORM_EXPORT void                     SetStoredUnitScale      (double storedScale);
    //!  Retrieves the scale factor used to stroke the LsSymbolComponent. If the stored scale factor is
    //!  non-zero, this is the stored scale factor.  Otherwise, it is the scale factor for converting master
    //!  units to UOR's for the model that was the root model at the time the LsSymbolComponent was last initialized.
    DGNPLATFORM_EXPORT double                   GetUnitScale            () const;
    //!  True if the symbol should not be scaled.
    DGNPLATFORM_EXPORT bool                     IsNoScale               () const;
    //!  True if the symbol should not be scaled.
    DGNPLATFORM_EXPORT void                     SetIsNoScale            (bool value);
    //!  Returns true if the LsSymbolComponent contains 3d graphics.
    DGNPLATFORM_EXPORT bool                     Is3d                    () const;
    };

//=======================================================================================
//!  Contains a reference to an LsSymbolComponent and some parameters that control how it is
//!  stroked when accessed through this reference.
//!
//!  An LsSymbolReference is never used as a separate object. Every LsSymbolReference is
//!  embedded in an LsPointComponent along with the other LsSymbolReference instances required
//!  by the LsStrokePatternComponent associated with the LsPointComponent.
//!  @ingroup LineStyleManagerModule
// @bsiclass
//=======================================================================================
struct          LsSymbolReference
    {
    friend struct LsPointComponent;
private:
    LsSymbolComponentPtr  m_symbol;
    LsPointComponentP   m_parent;

    uint32_t            m_mod1;

    DPoint3d            m_offset;
    double              m_angle;

    int                 m_strokeNo;

public:
    uint32_t            GetMod1             () const {return m_mod1;}
    double              _GetMaxWidth         () const;

    StatusInt           Output              (LineStyleContextR, Render::LineStyleSymbCP, DPoint3dCP org, DPoint3dCP dir, double const* xScale=0,
                                                DPoint3dCP clipOrg=0, DPoint3dCP clipEnd=0) const;
                        LsSymbolReference   (LsSymbolComponentP symbol, LsPointComponentP parent,
                                                uint32_t mod1, double xOffset, double yOffset, double radians, int strokeNumber);
    DGNPLATFORM_EXPORT      LsSymbolReference       ();

public:
    enum RotationMode
        {
        ROTATE_Relative     = 0,
        ROTATE_Absolute     = 1,
        ROTATE_Adjusted     = 2
        };

    enum StrokeJustification
        {
        JUSTIFICATION_None      = 0,    // Symbol not for stroke
        JUSTIFICATION_Origin    = 1,    // Symbol at origin of stroke
        JUSTIFICATION_End       = 2,    // Symbol at end of stroke
        JUSTIFICATION_Center    = 3,    // Symbol at center of stroke
        };

    //  VERTEX_LineOrigin, VERTEX_LineEnd, and VERTEX_LineEach are mutually
    //  exclusive because the PointComponent stroker never processes more
    //  than one flag at a time.
    enum VertexMask
        {
        VERTEX_None         = 0x00,
        VERTEX_LineOrigin   = LCPOINT_LINEORG,    // Symbol at origin of element
        VERTEX_LineEnd      = LCPOINT_LINEEND,    // Symbol at end of element
        VERTEX_Each         = LCPOINT_LINEVERT,    // Symbol at each vertex
        VERTEX_Any          = (VERTEX_LineOrigin | VERTEX_LineEnd | VERTEX_Each)
        };

    DGNPLATFORM_EXPORT bool                     GetDgnDb              () const;
    DGNPLATFORM_EXPORT void                     SetDgnDb              (bool value);

    DGNPLATFORM_EXPORT bool                     GetNoPartial            () const;                       //!<  Whether to draw a partial symbol at the end of a segment; false means do not draw.
    DGNPLATFORM_EXPORT void                     SetNoPartial            (bool value);                   //!<  Whether to draw a partial symbol at the end of a segment; false means do not draw.

    DGNPLATFORM_EXPORT bool                     GetClipPartial          () const;                       //!<  Clip symbol to the end of a segment.
    DGNPLATFORM_EXPORT void                     SetClipPartial          (bool value);                   //!<  Clip symbol to the end of a segment.

    DGNPLATFORM_EXPORT bool                     GetStretchable          () const;                       //!<  Allow the symbol to be stretched if the stroke is stretched.
    DGNPLATFORM_EXPORT void                     SetStretchable          (bool value);                   //!<  Allow the symbol to be stretched if the stroke is stretched.

    DGNPLATFORM_EXPORT bool                     GetProject              () const;                       //!<  Allow the symbol to display past the end of a segment.
    DGNPLATFORM_EXPORT void                     SetProject              (bool value);                   //!<  Allow the symbol to display past the end of a segment.

    DGNPLATFORM_EXPORT bool                     GetUseElementColor      () const;                       //!<  Use the color from the symbol.
    DGNPLATFORM_EXPORT void                     SetUseElementColor      (bool value);                   //!<  Use the color from the symbol.

    DGNPLATFORM_EXPORT bool                     GetUseElementWeight     () const;                       //!<  Use the weight from the symbol.
    DGNPLATFORM_EXPORT void                     SetUseElementWeight     (bool value);                   //!<  Use the weight from the symbol.

    DGNPLATFORM_EXPORT StrokeJustification      GetJustification        () const;                       //!<  The justification of the symbol relative to its placement point.
    DGNPLATFORM_EXPORT void                     SetJustification        (StrokeJustification value);    //!<  The justification of the symbol relative to its placement point.

    DGNPLATFORM_EXPORT RotationMode             GetRotationMode         () const;                       //!<  What the rotation angle is relative to - element, global, or adjusted to be readable.
    DGNPLATFORM_EXPORT void                     SetRotationMode         (RotationMode value);           //!<  What the rotation angle is relative to - element, global, or adjusted to be readable.

    DGNPLATFORM_EXPORT VertexMask               GetVertexMask           () const;                       //!<  Which vertices (if any) to place the symbol at, as opposed to a stroke number.
    DGNPLATFORM_EXPORT void                     SetVertexMask           (VertexMask value);             //!<  Which vertices (if any) to place the symbol at, as opposed to a stroke number.

    DGNPLATFORM_EXPORT double                   GetXOffset              () const;                       //!<  An offset from the stored origin of the symbol to the placement origin, in symbol coordinates.
    DGNPLATFORM_EXPORT void                     SetXOffset              (double value);                 //!<  An offset from the stored origin of the symbol to the placement origin, in symbol coordinates.

    DGNPLATFORM_EXPORT double                   GetYOffset              () const;                       //!<  An offset from the stored origin of the symbol to the placement origin, in symbol coordinates.
    DGNPLATFORM_EXPORT void                     SetYOffset              (double value);                 //!<  An offset from the stored origin of the symbol to the placement origin, in symbol coordinates.

    DGNPLATFORM_EXPORT double                   GetAngle                () const;                       //!<  A rotation angle for the symbol around the placement origin.
    DGNPLATFORM_EXPORT void                     SetAngle                (double value);                 //!<  A rotation angle for the symbol around the placement origin.

    DGNPLATFORM_EXPORT int                      GetStrokeNumber         () const;                       //!<  The index of the stroke to place this symbol.
    DGNPLATFORM_EXPORT void                     SetStrokeNumber         (int value);                    //!<  The index of the stroke to place this symbol.

    DGNPLATFORM_EXPORT LsSymbolComponentP       GetSymbolComponentP     () const;                       //!<  A pointer into the geometry; GetSymbolComponentCP is preferred.
    DGNPLATFORM_EXPORT LsSymbolComponentCP      GetSymbolComponentCP    () const;                       //!<  A pointer into the geometry.

    //! Update the symbol component (geometry) in the SymbolReference.
    //! @param[in] symbolComponent The new symbol component to use
    //! @see GetSymbolComponentCP()
    DGNPLATFORM_EXPORT void                     SetSymbolComponent      (LsSymbolComponentR symbolComponent);
    };


//=======================================================================================
// @bsiclass
//=======================================================================================
struct  LsOffsetComponent
    {
    double          m_offset;
    LsComponentPtr  m_subComponent;

    LsOffsetComponent (double offset, LsComponentP subComponent) : m_offset (offset), m_subComponent (subComponent) {}
    };


//=======================================================================================
//!  An LsComponent that represents a collection of LsComponents, along with an offset
//!  for each component.
//!  @ingroup LineStyleManagerModule
// @bsiclass
//=======================================================================================
struct          LsCompoundComponent : public LsComponent
{
private:
    typedef  bvector <LsOffsetComponent> T_ComponentsCollection;
    typedef  bvector <LsOffsetComponent>::iterator T_ComponentsCollectionIter;
    typedef  bvector <LsOffsetComponent>::const_iterator T_ComponentsCollectionConstIter;

    T_ComponentsCollection m_components;
    DPoint2d            m_size;
    bool                m_postProcessed;
    mutable LsOkayForTextureGeneration m_okayForTextureGeneration;

                    LsCompoundComponent         (LsLocationCP pLocation);
                    LsCompoundComponent         (LsCompoundComponentCR source);
protected:
    virtual         ~LsCompoundComponent        ();
    LsComponentPtr _Import(DgnImportContext& importer) const override;
    virtual LsRasterImageComponentP      _GetRasterImageComponent () override;
    virtual bool        _HasRasterImageComponent () const override;

public:
    static LsCompoundComponentP  LoadCompoundComponent  (LsComponentReader*reader);
    static LsCompoundComponentPtr Create (LsLocation& location) { LsCompoundComponentP retval = new LsCompoundComponent (&location); retval->m_isDirty = true; return retval; }
    void            CalculateSize                       ();

    void SaveToJson(Json::Value& result) const;
    static LineStyleStatus CreateFromJson(LsCompoundComponentP*, Json::Value const & jsonDef, LsLocationCP location);

    void    _PostProcessLoad            () override;
    void    _ClearPostProcess           () override;
    size_t          GetNumComponents            () const {return m_components.size ();}
    double          GetOffset                   (size_t index)   const   {return m_components[index].m_offset;}
    double  _GetLength                  () const override {return m_size.x;}
    double  _GetMaxWidth                 () const override   {return m_size.y;}
    bool    _HasWidth                   () const override;
    bool    _IsAffectedByWidth           (bool currentStatusOnly) const override;
    bool    _IsBySegment                 () const override;
    bool    _HasLineCodes                () const override;
    bool    _ContainsComponent           (LsComponentP other) const override;
    void            Free                        (bool    sub);
    StatusInt _DoStroke                 (LineStyleContextR, DPoint3dCP, int, Render::LineStyleSymbR) const override;
    bool            _HasUniformFullWidth         (double *pWidth)   const override;
    void _StartTextureGeneration() const override;
    LsComponentPtr _GetForTextureGeneration() const override;
    LsOkayForTextureGeneration _IsOkayForTextureGeneration() const override;

public:
    //! Retrieves the number of components the LsCompoundComponent references. There is no lmit on
    //! the number of components an LsCompoundComponent can reference.
    DGNPLATFORM_EXPORT size_t                   GetNumberComponents() const;
    //! Retrieves a pointer to the indexed component; NULL if the index is invalid.
    DGNPLATFORM_EXPORT LsComponentP             GetComponentP(size_t index) const;
    //! Retrieves a pointer to the indexed component; NULL if the index is invalid.
    DGNPLATFORM_EXPORT LsComponentCP            GetComponentCP (size_t index) const;
    //! Retrieves the offset to the component.  The offset is the distance, in master units measured perpendicularly from the work line,
    //! by which the component is displayed parallel to the work line. If the offset is zero, the selected component is displayed on the work line.
    DGNPLATFORM_EXPORT double                   GetOffsetToComponent (size_t index) const;
    //! Appends a component to the list of components that the LsComponentComponent references.
    DGNPLATFORM_EXPORT LineStyleStatus          AppendComponent (LsComponentR, double offset);
};

//=======================================================================================
//!  An LsStroke controls how a portion of the line style is stroked.  An LsStroke
//!  is never used as a separate object; they are always embedded in LsStrokePatternComponent
//!  components.
//!  @ingroup LineStyleManagerModule
// @bsiclass
//=======================================================================================
struct          LsStroke
{
    // WIP_LINESTYLE *** Partly redundant with LineCodeWidth
    enum WidthMode
    {
        LCWIDTH_None            = 0x00,
        LCWIDTH_Left            = 0x01,
        LCWIDTH_Right           = 0x02,
        LCWIDTH_Full            = 0x03,
    };

    friend struct LsStrokePatternComponent;
private:
    double          m_length;         // Stroke length
    double          m_orgWidth;       // Stroke origin width
    double          m_endWidth;       // Stroke end width

    Byte m_strokeMode;                // bit 0: dash, first rep | gap    dash
                                      // bit 1: dash, int. rep  | gap    dash
                                      // bit 2: dash, last rep  | gap    dash
                                      // bit 4: rigid           | on     off
                                      // bit 5: stretchable     | on     off

    Byte m_widthMode;                 // bit 0: left half       | off    on
                                      // bit 1: right half      | off    on

    Byte m_capMode;                   // 0 = closed       1 = open
                                      // 2 = extended     3 = hexagon
                                      // 4 = octagon      5 = decagon
                                      // and so on, (n vectors in cap - up to 255)

    void        Init (double length, double orgWidth, double endWidth, WidthMode widthMode, LsCapMode capMode)
                    {
                     m_length     = length;
                     m_orgWidth   = orgWidth;
                     m_endWidth   = endWidth;
                     m_strokeMode = 0;
                     m_widthMode  = (Byte)widthMode;
                     m_capMode    = (Byte)capMode;
                    }

public:
    void        SetLength           (double newLength) {m_length = newLength;}
    void        SetWidthMode        (int newMode) {m_widthMode &= ~0x03; m_widthMode |= newMode;}
    void        SetWidth            (double width){m_orgWidth = width; m_endWidth = width;}
    void        SetCapMode          (int mode)    {m_capMode = (Byte)mode;}
    void        SetCapMode          (LsCapMode mode)    {m_capMode = (Byte)mode;}

    bool        HasWidth            () const {return IsDash() && (0 != GetWidthMode());}
    // WIP_LINESTYLE *** (LCWIDTH_FULL == GetWidthMode()) => warning: comparison between 'enum Dgn::LineCodeWidth' and 'enum Dgn::LsStroke::WidthMode'
    bool        _HasUniformFullWidth () const {return !IsDash() || (LCWIDTH_Full == GetWidthMode() && m_orgWidth == m_endWidth);}

    #define TESTSTROKEMODE(flag) const {return ((m_strokeMode & flag) == 0) ? false : true;}
    #define SETSTROKEMODE(flag)  (bool    isOn) {if (isOn!=0) m_strokeMode |= flag; else {m_strokeMode &= ~flag;}}

    enum
    {
         STROKE_Dash         = 0x01,
         STROKE_DashFirst    = 0x02,
         STROKE_DashLast     = 0x04,
         STROKE_Rigid        = 0x10,
         STROKE_Stretchable  = 0x20,
    };

                LsStroke (double length, double startWidth, double endWidth, WidthMode widthMode, LsCapMode capMode);
    explicit    LsStroke (LsStroke const&);

private:
    explicit                   LsStroke ();

public:
    DGNPLATFORM_EXPORT void                     SetIsDash           (bool isDash);      //!<  Stroke Type Dash vs. Stroke Type Gap
    DGNPLATFORM_EXPORT bool                     IsDash              () const;           //!<  Stroke Type Dash vs. Stroke Type Gap
    DGNPLATFORM_EXPORT void                     SetIsDashFirst      (bool isOn);
    DGNPLATFORM_EXPORT bool                     IsDashFirst         () const;           //!<  Computed from IsDash and invert at flags, should have explicit InvertAt
    DGNPLATFORM_EXPORT void                     SetIsDashLast       (bool isOn);
    DGNPLATFORM_EXPORT bool                     IsDashLast          () const;           //      ""
    DGNPLATFORM_EXPORT void                     SetIsStretchable    (bool isOn);        //!<  If the dash or gap is fixed or stretchable to fit the length of the segment.
    DGNPLATFORM_EXPORT bool                     IsStretchable       () const;           //!<  If the dash or gap is fixed or stretchable to fit the length of the segment.
    DGNPLATFORM_EXPORT void                     SetIsRigid          (bool isOn);        //!<  Rigid means to continue past a corner to complete the current stroke.  False means to break at the corner.
    DGNPLATFORM_EXPORT bool                     IsRigid             () const;           //!<  Rigid means to continue past a corner to complete the current stroke.  False means to break at the corner.


    //! Returns the length of the stroke.
    //! @return  Length of the stroke
    //! remarks There is no SetLength method.  Length is set via LsCacheStrokePatternComponent::AppendStroke
    DGNPLATFORM_EXPORT double                   GetLength ()        const;

    //! Returns the width in Master Units at the start of the stroke.  Only applies for dashes.
    //! @return  Start width.
    //! remarks There is no SetStartWidth method.  Start width is set via LsCacheStrokePatternComponent::AppendStroke
    DGNPLATFORM_EXPORT double                   GetStartWidth ()    const;

    //! Returns the width in Master Units at the end of the stroke.  Only applies for dashes.
    //! @return  End width.
    //! remarks There is no SetEndWidth method.  End width is set via LsCacheStrokePatternComponent::AppendStroke
    DGNPLATFORM_EXPORT double                   GetEndWidth ()      const;

    //! Returns the way that the ends of a dash are handled.
    //! @return  Cap mode.
    //! remarks There is no SetCapMode method.  Cap mode is set via LsCacheStrokePatternComponent::AppendStroke
    DGNPLATFORM_EXPORT LsCapMode                GetCapMode ()        const;

    //! Returns the way that the widths of a dash are handled.
    //! @return  Width mode.
    //! remarks There is no SetWidthMode method.  Width mode is set via LsCacheStrokePatternComponent::AppendStroke
    DGNPLATFORM_EXPORT WidthMode                GetWidthMode ()     const;
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct     ISymbolProcess
    {
    virtual bool _ProcessSymbol (LineStyleContextR, Centerline const*, Render::LineStyleSymbCP,LsStrokeCP, int strokeIndex, int endCondition) const = 0;
    };


//=======================================================================================
//! A LsStrokePatternComponent consists of parameters that control how the individual
//! strokes are interpreted, plus up to 32 LsStroke values.
//!
//!  @ingroup LineStyleManagerModule
// @bsiclass
//=======================================================================================
struct          LsStrokePatternComponent : public LsComponent
{
private:
    explicit LsStrokePatternComponent (LsStrokePatternComponent const*);

protected:
    double          m_phaseShift;
    double          m_autoPhase;
    double          m_maxCompress;
    double          m_patternLength;
    DPoint3dCP      m_startTangent;
    DPoint3dCP      m_endTangent;
    int             m_nIterate;
    mutable LsOkayForTextureGeneration m_okayForTextureGeneration;
    struct {
        uint32_t   phaseMode:2;
        uint32_t   iterationLimit:1;
        uint32_t   segMode:1;
        uint32_t   enableEndConditions:1;
        uint32_t   cosmetic:1;
        uint32_t   affectedByWidth:1;       // Cache this because it's called in display.
        }  m_options;

    size_t          m_nStrokes;
    LsStroke        m_strokes[33];

    bool            ApplyModifiers          (double* pOrgWidth, double* pEndWidth, Render::LineStyleSymbCP);
    void            ApplyScale              (double scale);
    void            ApplyStretch            (double dashScale, double gapScale);
    void            ApplyWidth              (double width);
    void            ApplyAutomaticPhase     (double elementLength);
    void            ApplyIteration          (double elementLength);
    void            ApplyCenterPhase        (double length, bool isClosed);
    void            CalcPatternLength       ();

    double          GenerateStrokes         (LineStyleContextR, ISymbolProcess const*, Render::LineStyleSymbCP, DPoint3dCP, int,
                                                double length, double width, double taper, int segFlag);
    void            StrokeLocal             (LineStyleContextR, ISymbolProcess const*, DPoint3dCP, int, double, Render::LineStyleSymbR, DPoint3dCP, DPoint3dCP, int segFlag) const;
    explicit LsStrokePatternComponent       (LsLocationCP pLocation);
    void            FixDashWidths           (double& orgWidth, double& endWidth, bool taper, ViewContextCP context, DPoint3dCP pt);
    LsComponentPtr _Import(DgnImportContext& importer) const override;

public:

    void SaveToJson(Json::Value& result) const;
    static LineStyleStatus CreateFromJson(LsStrokePatternComponentP*, Json::Value const & jsonDef, LsLocationCP location);
    static LsStrokePatternComponentP  LoadStrokePatternComponent    (LsComponentReader*reader);
    static LsStrokePatternComponentPtr Create                       (LsLocation& location) { LsStrokePatternComponentP retval = new LsStrokePatternComponent (&location); retval->m_isDirty = true; return retval; };

    BentleyStatus   PostCreate              ();

    StatusInt       _DoStroke               (LineStyleContextR, DPoint3dCP, int, Render::LineStyleSymbR) const override;
    StatusInt       ProcessStroke           (LineStyleContextR, ISymbolProcess const*, DPoint3dCP inPoints, int nPoints, Render::LineStyleSymbR) const;


    bool            CheckSegmentMode        (Render::LineStyleSymbCP) const;


    size_t          GetStrokeCount          () const {return  m_nStrokes;}
    double          _CalcRepetitions        (Render::LineStyleSymbCP) const override;
    LsStrokeP       AppendStroke            (LsStrokeCR stroke);
    void            AppendStroke            (double length, bool isDash);
    void            DeleteStroke            (size_t index);
    void            GetStroke               (LsStroke* pStroke, size_t index);
    LsStrokeP       GetStrokePtr            (size_t index) {return &m_strokes[index];}
    LsStrokeCP      GetConstStrokePtr       (size_t index) const {return  &m_strokes[index];}

    //  Misc properties
    void            SetCosmetic             (bool   );
    bool            IsCosmetic              () const;
    void            SetMaxCompress          (double);
    bool            IsStretchable           () const;

    //  End Conditions
    void            SetEndConditions        (bool    enabled);
    bool            AreEndConditionsEnabled () const;

    //  Segmentation information
    bool    _IsBySegment             () const override {return IsSingleSegment();}

    //  Continuous mode
    void            SetContinuous           ();
    bool    _IsContinuous           () const override;
    bool    _IsContinuousOrSingleDash() const override;

    //  Computed properties
    //  Returns true if at least one stroke is rigid
    bool            IsRigid                 () const;
    bool    _HasWidth               () const override;
    bool    _HasUniformFullWidth     (double *pWidth) const override;
    bool    _IsAffectedByWidth       (bool currentStatusOnly) const override;
    double  _GetLength              () const override {return m_patternLength;}
    double          GetLength               (double*) const;
    double  _GetMaxWidth             () const override;
    bool            RequiresLength          () const;
    void _StartTextureGeneration() const override { m_okayForTextureGeneration = LsOkayForTextureGeneration::Unknown; }
    LsComponentPtr _GetForTextureGeneration() const override;
    LsOkayForTextureGeneration _IsOkayForTextureGeneration() const override;

public:
    enum PhaseMode
    {
        PHASEMODE_Fixed     = 0,  //!<    Distance, use GetDistancePhase
        PHASEMODE_Fraction  = 1,  //!<    Fraction, use GetFractionalPhase
        PHASEMODE_Center    = 2,  //!<    Centered, no phase value
    };

    //! Get the number of strokes in this Stroke Component.
    //! @return The number of strokes.
    DGNPLATFORM_EXPORT size_t                   GetNumberStrokes    () const;

    //! Returns a pointer to an embedded LsStroke.  GetStrokeCP() is preferred in most cases.
    //! @param[in] index Index of the stroke.
    //! @return A pointer to the stroke or NULL if the index is out of range.
    DGNPLATFORM_EXPORT LsStrokeP                GetStrokeP          (size_t index);

    //! Returns a constant pointer to an embedded LsStroke.
    //! @param[in] index Index of the stroke.
    //! @return A pointer to the stroke or NULL if the index is out of range.
    DGNPLATFORM_EXPORT LsStrokeCP               GetStrokeCP         (size_t index) const;

    //! Determine if the Stroke Component has an iteration limit.
    //! The entire Stroke Pattern will be repeated no more than the limit on an element or segment.
    //! @return True if there is an iteration limit.
    DGNPLATFORM_EXPORT bool                     HasIterationLimit   () const;

    //! Get the number of iterations for this style.
    //! The entire Stroke Pattern will be repeated no more than the limit on an element or segment.
    //! @return The value store in the iteration limit.  This value is NOT used if the HasIterationLimit() flag is not set.
    //! @see SetIterationMode() SetIterationLimit()
    DGNPLATFORM_EXPORT int                      GetIterationLimit   () const;

    //! Get the phase distance for this Stroke Component.  Phase is the distance in the units of this linestyle
    //! to skip before starting the pattern.
    //! @return The phase distance stored in the Stroke Component.  This is only valid if GetPhaseMode() is PHASEMODE_Fixed.
    //! @see SetDistancePhase()
    DGNPLATFORM_EXPORT double                   GetDistancePhase    () const;

    //! Get the fractional phase distance for this Stroke Component.  Phase is the fraction of the first dash to skip before
    //! displaying the line style.
    //! @return The phase distance stored in the Stroke Component.  This is only valid if GetPhaseMode() is PHASEMODE_Fraction.
    //! @see SetFractionalPhase()
    DGNPLATFORM_EXPORT double                   GetFractionalPhase  () const;

    //! Get the phase for the Stroke Component.  Phase describes how to adjust the pattern before starting.  Often this is used
    //! to set a fraction of 0.5 causing half the dash to appear at the start.  Used with stretchable gaps and corner mode this
    //! will avoid linestyles having a gap at a corner.  Centered phasing means that only a whole
    //! number of patterns are displayed, and then solid leaders are added at the start and end.  This typically looks worse.
    //! @return The phase distance stored in the Stroke Component.  This is only valid if GetPhaseMode() is PHASEMODE_Fraction.
    //! @see SetPhaseMode()
    DGNPLATFORM_EXPORT PhaseMode                GetPhaseMode        () const;

    //! Determine if this Stroke Component is set for single-segment.  In this mode it will stop the pattern and restart
    //! as it goes around corners.  If this is not set then the pattern will continue around corners.
    //! @return The whether the pattern is in single-segment mode
    //! @see SetSegmentMode()
    DGNPLATFORM_EXPORT bool                     IsSingleSegment     () const;

    //! Add another stroke to the Stroke Pattern.
    //! @param[in] length Length of the stroke or gap.
    //! @param[in] startWidth Width at the start of the stroke; use 0 for none.
    //! @param[in] endWidth Width at the end of the stroke; use 0 for none.
    //! @param[in] widthMode How to apply the width.
    //! @param[in] capMode The way that endcaps are handled for the stroke.
    //! remarks To add a gap you need to call LsStroke::SetIsDash() on the returned stroke.
    //! @return A pointer to the stroke or NULL if the Stroke Component already contains 32 strokes.
    DGNPLATFORM_EXPORT LsStrokeP                AppendStroke        (double length, double startWidth, double endWidth, LsStroke::WidthMode widthMode, LsCapMode capMode);

    //! Set whether the pattern will have a set number of iterations.
    //! @param[in] limited True to limit the number of iterations, false for unlimited.
    //! @return True if the value was changed, false if limited=true and there is no stretchable gaps or dashes.  In this case the limit will not be set.
    //! remarks If the iteration limit is zero, this method will set it to one.
    //! @see GetIterationLimit() SetIterationLimit()
    DGNPLATFORM_EXPORT bool                     SetIterationMode    (bool limited);

    //! Set the number of iterations for the pattern.  Calling this method with a non-zero numIterations will also set the iteration mode to limited.
    //! @param[in] numIterations The number of iterations for this pattern.
    //! @see GetIterationLimit() SetIterationMode()
    DGNPLATFORM_EXPORT void                     SetIterationLimit   (int numIterations);

    //! Set the Stroke Component to be single-segment (restart the pattern at corners) or to continue around corners.
    //! @param[in] isSingleSegment The new value for the segment mode.
    //! @see IsSingleSegment()
    DGNPLATFORM_EXPORT void                     SetSegmentMode      (bool isSingleSegment);

    //! Set the Stroke Component phase mode.
    //! mode[in] mode The new phase mode for this pattern.
    //! @see GetPhaseMode()
    DGNPLATFORM_EXPORT void                     SetPhaseMode        (PhaseMode mode);

    //! Set the distance to skip into the pattern before starting to draw.  This value only applies if the phase mode is PHASEMODE_Fixed.
    //! mode[in] distance The new phase distance for this pattern.
    //! @see GetDistancePhase()
    DGNPLATFORM_EXPORT void                     SetDistancePhase    (double distance);

    //! Set phasing to be centered.  This value only applies if the phase mode is PHASEMODE_Center.
    //! mode[in] distance The new phase distance for this pattern.
    //! @see GetDistancePhase()
    DGNPLATFORM_EXPORT void                     SetCenterPhaseMode  ();

    //! Set the fraction of the first stroke to skip into the pattern before starting to draw.  This value only applies if the phase mode is PHASEMODE_Fraction.
    //! mode[in] fraction The new phase fraction for this pattern.
    //! @see GetFractionalPhase()
    DGNPLATFORM_EXPORT void                     SetFractionalPhase  (double fraction);
 };

//=======================================================================================
//!  Combines an LsStrokePatternComponent and a collection of LsSymbolReference values.
//!  The LsStrokePatternComponent determines how the LsSymbolRefernce values are used.
//!  @ingroup LineStyleManagerModule
// @bsiclass
//=======================================================================================
struct          LsPointComponent : public LsComponent
                                , public ISymbolProcess
    {
    typedef  bvector <LsSymbolReference> T_SymbolsCollection;
    typedef  bvector <LsSymbolReference>::iterator T_SymbolsCollectionIter;
    typedef  bvector <LsSymbolReference>::const_iterator T_SymbolsCollectionConstIter;

    LsStrokePatternComponentPtr m_strokeComponent;
    T_SymbolsCollection     m_symbols;
    bool                    m_postProcessed;
    mutable LsOkayForTextureGeneration m_okayForTextureGeneration;

private:
    bool                    _ProcessSymbol           (LineStyleContextR, Centerline const*, Render::LineStyleSymbCP, LsStrokeCP, int strokeIndex, int endCondition) const override;

    LsSymbolReferenceP              GetSymbolReferenceP     (T_SymbolsCollectionConstIter iter) const;
    LsPointComponent    (LsLocationCP pLocation);
    LsPointComponent    (LsPointComponentCR source, bool removeSymbrefOnVertex);

protected:
    ~LsPointComponent   ();
    LsComponentPtr _Import(DgnImportContext& importer) const override;

public:
    void                    _PostProcessLoad        () override;
    void                    _ClearPostProcess       () override;
    bool                    _IsContinuous           () const override {return NULL==m_strokeComponent.get () ? false : m_strokeComponent->_IsContinuous();}
    double                  _GetLength              () const override;
    StatusInt               _DoStroke               (LineStyleContextR, DPoint3dCP, int, Render::LineStyleSymbR) const override;
    static LsPointComponent*        LoadLinePoint           (LsComponentReader*reader);
    static LsPointComponentPtr      Create                  (LsLocation&location) { LsPointComponentP retval = new LsPointComponent (&location); retval->m_isDirty = true; return retval; }
    double                  _GetMaxWidth            ()  const override;
    bool                    _ContainsComponent      (LsComponentP other) const override;
    void                            Free                    (bool    sub);
    bool                            HasStrokeSymbol         () const;
    LsComponentPtr _GetForTextureGeneration() const override;
    void _StartTextureGeneration() const override;
    LsOkayForTextureGeneration _IsOkayForTextureGeneration() const override;
    LsOkayForTextureGeneration VerifySymbols() const;
    LsOkayForTextureGeneration VerifySymbol(double& adjustment, double startingOffset, double patternLength, uint32_t strokeIndex) const;

    void SaveToJson(Json::Value& result) const;
    static LineStyleStatus CreateFromJson(LsPointComponentP*, Json::Value const & jsonDef, LsLocationCP location);
    DGNPLATFORM_EXPORT static void SaveLineCodeIdToJson(JsonValueR result, LsComponentId patternId);
    DGNPLATFORM_EXPORT static void SaveSymbolIdToJson(JsonValueR result, LsComponentId symbolId);

public:
    DGNPLATFORM_EXPORT size_t                   GetNumberSymbols        () const;
    //! Gets a pointer to the indexed LsSymbolReference.
    //! param[in] arrayIndex Index into the embedded list of LsSymbolReference instances.
    //! return LsSymbolReferenceP for a matching LsSymbolReference, or NULL if arrayIndex < 0 or >= GetNumberSymbols().
    DGNPLATFORM_EXPORT LsSymbolReferenceP       GetSymbolP              (size_t arrayIndex) const;
    //! Gets a pointer to the indexed LsSymbolReference.
    //! param[in] arrayIndex Index into the embedded list of LsSymbolReference instances.
    //! return LsSymbolReferenceCP for a matching LsSymbolReference, or NULL if arrayIndex < 0 or >= GetNumberSymbols().
    DGNPLATFORM_EXPORT LsSymbolReferenceCP      GetSymbolCP             (size_t arrayIndex) const;
    //! Gets a pointer to the LsSymbolReference with the matching value for LsSymbolReference::GetStrokeNumber().
    //! param[in] strokeNumber value to compare to GetStrokeNumber;
    //! return LsSymbolReferenceP for a matching LsSymbolReference, or NULL if there is no match.
    DGNPLATFORM_EXPORT LsSymbolReferenceP       GetSymbolForStrokeP (int strokeNumber) const;
    //! Gets a pointer to the LsSymbolReference with the matching value for LsSymbolReference::GetStrokeNumber().
    //! param[in] strokeNumber value to compare to GetStrokeNumber.
    //! return LsSymbolReferenceCP for a matching LsSymbolReference, or NULL if there is no match.
    DGNPLATFORM_EXPORT LsSymbolReferenceCP      GetSymbolForStrokeCP (int strokeNumber) const;
    //! Gets a pointer to the LsSymbolReference with the matching vertex mask.
    //! param[in] vertexMask value to compare to LsSymbolReference::GetVertexMask().
    //! return LsSymbolReferenceP for a matching LsSymbolReference, or NULL if there is no match.
    DGNPLATFORM_EXPORT LsSymbolReferenceP       GetSymbolForVertexP     (LsSymbolReference::VertexMask vertexMask) const;
    //! Gets a pointer to the LsSymbolReference with the matching vertex mask.
    //! param[in] vertexMask value to compare to LsSymbolReference::GetVertexMask().
    //! return LsSymbolReferenceCP for a matching LsSymbolReference, or NULL if there is no match.
    DGNPLATFORM_EXPORT LsSymbolReferenceCP      GetSymbolForVertexCP    (LsSymbolReference::VertexMask vertexMask) const;
    DGNPLATFORM_EXPORT LsStrokePatternComponentP GetStrokeComponentP    () const;
    DGNPLATFORM_EXPORT LsStrokePatternComponentCP GetStrokeComponentCP  () const;
    };

//=======================================================================================
//!  Represents a standard line code.
//!  @ingroup LineStyleManagerModule
// @bsiclass
//=======================================================================================
struct          LsInternalComponent : public LsStrokePatternComponent
    {
private:
    uint32_t    m_hardwareLineCode;
    LsInternalComponent (LsLocationCP pLocation);

public:
    static LsStrokePatternComponentP  LoadInternalComponent (LsComponentReader*reader);
    static LsStrokePatternComponentPtr  Create1         (LsLocation&location) { return new LsInternalComponent (&location); }

    bool        _IsAffectedByWidth               (bool currentStatusOnly) const override;
    bool        _IsContinuous                   () const override {return 0==m_hardwareLineCode ? true : false;}
    bool        _IsContinuousOrSingleDash        () const override {return _IsContinuous();}
    bool        _HasLineCodes                    () const override {return IsHardwareStyle();}
    StatusInt   _DoStroke                       (LineStyleContextR, DPoint3dCP, int, Render::LineStyleSymbR) const override;
    double      _GetLengthForTexture    () const override {return 0;}
    static LsInternalComponentPtr CreateInternalComponent   (LsLocation&location);
    LsComponentPtr _GetForTextureGeneration() const override { return const_cast<LsInternalComponentP>(this); }
    LsOkayForTextureGeneration _IsOkayForTextureGeneration() const override { return LsOkayForTextureGeneration::NoChangeRequired; }
    LsComponentPtr _Import(DgnImportContext& importer) const override { return const_cast<LsInternalComponent*>(this); }

public:
    DGNPLATFORM_EXPORT uint32_t                 GetHardwareStyle    () const;
    DGNPLATFORM_EXPORT bool                     IsHardwareStyle     () const;
    DGNPLATFORM_EXPORT uint32_t                 GetLineCode         () const;
    };


typedef struct LsIdNode const*                LsIdNodeP;
typedef struct NameNode const*                NameNodeP;
typedef KeyTree <LsIdNode, int64_t>           T_LsIdTree;
typedef LeafNode <LsIdNode, int64_t>          T_LsIdTreeNode;
typedef KeyTree <NameNode, MSCharIKey>        T_LsNameTree;
typedef LeafNode <NameNode, MSCharIKey>       T_LsNameTreeNode;
typedef T_LsIdTree::Iterator                  T_LsIdIterator;
typedef T_LsIdTree::Iterator                  T_LsIdIterator;

//!  Defines possible values for LsDefinition::GetUnitsType() and LsDefinition::SetUnitsType().
//!  @ingroup LineStyleManagerModule
enum class LsUnit
{
    Device      = 2,        //!< Pixel units
    Meters      = 3,        //!< Meters
};

//=======================================================================================
//! TextureDescr
//!
// @bsiclass
//=======================================================================================
struct          TextureDescr
    {
    Render::TexturePtr  m_texture;
    bool m_hasTextureWidth;
    double m_textureWidth;
    };

struct TextureParams
    {
private:
    uint32_t    m_lineWeight;
    uint32_t    m_flags;
    double      m_scale;
    double      m_styleWidth;  //  don't allow both end and start for texture.
public:
    bool operator< (struct TextureParams const&rhs) const;
    TextureParams();
    TextureParams(uint32_t lineWeight, uint32_t flags, double scale, double styleWidth);
    };

typedef bmap<TextureParams, TextureDescr> ParamsToTexture_t;
//=======================================================================================
//! Represents the definition of a line style.
//!  @ingroup LineStyleManagerModule
// @bsiclass
//=======================================================================================
struct          LsDefinition
                : public ILineStyle
    {
    friend struct DgnLineStyles;
    friend struct LsCache;
private:
    bool                m_isDirty;
    bool                m_componentLookupFailed;
    DgnStyleId          m_styleId;
    MSCharIKey          m_name;
    LsLocation          m_location; // Where to find components of resource
    LsComponentPtr      m_lsComp;
    double              m_unitDef;
    double              m_maxWidth;
    uint32_t            m_attributes;
    int                 m_hardwareLineCode;
    BeAtomic<bool>      m_componentLoadPostProcessed;

    // For texture styles...
    mutable bool        m_firstTextureInitialized;
    mutable bool        m_texturesNotSupported;
    mutable bool        m_usesSymbolWeight; // if m_usesSymbolWeight is true, only use m_textures[0]
    Render::TexturePtr  m_rasterTexture;
    ParamsToTexture_t   m_geometryTextures; // Geometry textures...raster component uses m_rasterTexture

    void Init (CharCP nName, Json::Value& lsDefinition, DgnStyleId styleId);
    void SetHWStyle(LsComponentId componentID);
    int GetUnits() const {return m_attributes & LSATTR_UNITMASK;}
    StatusInt GetGeometryTexture(TextureDescr& textureDescr, ViewContextR viewContext, Render::LineStyleSymbR lineStyleSymb, Render::GeometryParamsCR params);
    StatusInt GenerateTexture(TextureDescr& textureDescr, ViewContextR viewContext, Render::LineStyleSymbR lineStyleSymb, Render::GeometryParamsCR params);

public:
    LsDefinition(Utf8CP name, DgnDbR project, Json::Value& lsDefinition, DgnStyleId styleId);

    DGNPLATFORM_EXPORT static double GetUnitDef (Json::Value& lsDefinition);
    DGNPLATFORM_EXPORT static uint32_t GetAttributes (Json::Value& lsDefinition);
    DGNPLATFORM_EXPORT static LsComponentId GetComponentId (Json::Value& lsDefinition);

    DGNPLATFORM_EXPORT LsDefinition* Clone ();
    DGNPLATFORM_EXPORT static void Destroy (LsDefinitionP);

    virtual ~LsDefinition ();

    Utf8CP _GetName () const override {return m_name.GetValue();}
    ILineStyleComponent const* _GetComponent () const override {return m_lsComp.get ();}
    bool _IsSnappable () const override {return 0 == (m_attributes & LSATTR_NOSNAP);}

    LsComponentP GetLsComponent () const {return m_lsComp.get ();}
    LsLocationCP GetLocation () const {return &m_location;}
    uint32_t GetAttributes () const {return m_attributes;}
    bool IsOfType (LsComponentType type) const {return (m_location.GetComponentType() == type ? true : false);}

    DGNPLATFORM_EXPORT StatusInt UpdateStyleTable() const;

    void CheckForContinuous (LsStrokePatternComponentCP);
    void PostProcessComponentLoad ();
    void ClearPostProcess ();
    DGNPLATFORM_EXPORT void SetName (Utf8CP name);
    void SetAttributes (uint32_t attr) {m_attributes = attr;}
    void SetStyleId (DgnStyleId number) { m_styleId = number; }
    DgnStyleId GetStyleId () { return m_styleId; }

    // Raster Images...
    Render::Texture* GetRasterTexture(double& textureWidth, ViewContextR, Render::GeometryParamsCR);
    Render::Texture* GetGeometryTexture(double& textureWidth, ViewContextR, Render::GeometryParamsCR);
    Render::Texture* _GetTexture(double& textureWidth, ViewContextR, Render::GeometryParamsCR, bool createGeometryTexture) override;

    //  There should no reason to provide set methods or to expose this outside of DgnPlatform.
    DGNPLATFORM_EXPORT double _GetMaxWidth () const;
    DGNPLATFORM_EXPORT int GetHardwareStyle () const;
    DGNPLATFORM_EXPORT bool IsNoWidth () const;
    DGNPLATFORM_EXPORT bool IsInternal () const;
    DGNPLATFORM_EXPORT bool IsHardware () const;

    bool IsDirty () { return m_isDirty; }
    void MarkDirty (bool value = true) { m_isDirty = value; }
    StatusInt Commit ();

    static void InitializeJsonObject (Json::Value& jsonObj, LsComponentId componentId, uint32_t flags, double unitDefinition);
    void InitializeJsonObject (Json::Value& jsonObj);

public:
    //!  Defines a scaling factor to be applied to the components.
    DGNPLATFORM_EXPORT double GetUnitsDefinition () const;
    //!  Defines a scaling factor to be applied to the components.
    DGNPLATFORM_EXPORT void SetUnitsDefinition (double newValue);
    DGNPLATFORM_EXPORT void SetUnitsType (LsUnit unitsType);
    DGNPLATFORM_EXPORT LsUnit GetUnitsType () const;
    //!  This is equivalent to "LsUnit::Meters == GetUnits()"
    DGNPLATFORM_EXPORT bool IsUnitsMeters () const;
    //!  This is equivalent to "LsUnit::Device == GetUnits()"
    DGNPLATFORM_EXPORT bool IsUnitsDevice () const;
    //! Returns true if line styles are physical and should be scaled as such.  This only applies to styles in
    //! dgnlibs, not resources.
    //!
    //! Setting IsPhysical to true prevents the Global Line Style Scale and the Annotation Scale
    //! factors from being applied.
    DGNPLATFORM_EXPORT bool IsPhysical () const;
    //! Line styles are physical and should be scaled as such.  This only applies to styles in
    //! dgnlibs, not resources.
    DGNPLATFORM_EXPORT void SetIsPhysical (bool newValue);

    DGNPLATFORM_EXPORT DgnStyleId               GetStyleId              () const;

    //!  Setting this to true allows a user to snap to the individual components of a line style. If off,
    //!  a snap operation snaps to the underlying line of a line style
    DGNPLATFORM_EXPORT bool                     IsSnappable             () const;
    //!  Setting this to true allows a user to snap to the individual components of a line style. If off,
    //!  a snap operation snaps to the underlying line of a line style
    DGNPLATFORM_EXPORT void                     SetIsSnappable          (bool newValue);

    DGNPLATFORM_EXPORT bool                     IsContinuous            () const;
    DGNPLATFORM_EXPORT void                     SetIsContinuous         (bool newValue);
    //!  Gets the LsComponent for the line style.
    //!
    //!  An LsDefinition can only directly reference one LsComponent.  That LsComponent
    //!  may reference other components.
    DGNPLATFORM_EXPORT LsComponentP             GetComponentP           () const;
    //!  Gets the LsComponent for the line style.
    //!
    //!  An LsDefinition can only directly reference one LsComponent.  That LsComponent
    //!  may reference other components.
    DGNPLATFORM_EXPORT LsComponentCP            GetComponentCP          () const;
    //!  Sets the LsComponent for the line style.
    //!
    //!  An LsDefinition can only directly reference one LsComponent.  That LsComponent
    //!  may reference other components.
    DGNPLATFORM_EXPORT LineStyleStatus          SetComponent            (LsComponentP lsComp);
    //!  Gets the name of the line style
    DGNPLATFORM_EXPORT Utf8String               GetStyleName            () const;
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct  LsIdNode
    {
    friend struct LsCacheStyleEntry;
private:
    int64_t             m_id;
    Utf8P               m_name;
    LsDefinitionP       m_nameRec;

public:
    LsIdNode (int64_t id, Utf8CP name, LsDefinitionP nameRec);

    void Clear ();
    int64_t GetKey () const {return m_id;}
    LsDefinitionP GetValue () const {return m_nameRec;}
    Utf8CP GetName () const {return m_name;}
    void SetValue (LsDefinitionP v) {m_nameRec = v;}
    void SetName (Utf8CP newName);

    static int64_t GetMinKey () {return INT64_MIN;}
    static int64_t GetMaxKey () {return INT64_MAX;}
    };

//=======================================================================================
//! An entry in a LsCache; it contains a pointer to an LsDefinition
//!
//! @ingroup LineStyleManagerModule
// @bsiclass
//=======================================================================================
struct          LsCacheStyleEntry
                : public LsIdNode
{
private:
    LsCacheStyleEntry ();

public:
    //!  Gets the name of the line style
    DGNPLATFORM_EXPORT Utf8CP GetStyleName () const;
    //! Get the line style number. For an LsCacheStyleEntry from an LsDgnFileMap this is the number
    //! that is used to translate from the number in an element's symbology to a line style name.
    //! For an LsCacheStyleEntry from a system map, the style number normally is meaningless.
    DGNPLATFORM_EXPORT int64_t GetStyleId () const;
    //! Get the LsDefinition for the line style associated with the LsCacheStyleEntry.
    DGNPLATFORM_EXPORT LsDefinitionP GetLineStyleP () const;
    //! Get the LsDefinition for the line style associated with the LsCacheStyleEntry.
    //! @See LsCacheStyleEntry::GetLineStyleP
    DGNPLATFORM_EXPORT LsDefinitionCP GetLineStyleCP () const;
};

//=======================================================================================
//! Helper class used for iterating through the LsCacheStyleEntry entries in an LsCache.
//!@code
//!   for each (LsCacheStyleEntryCR  ls in *systemLsCache)
//!       {
//!       Int32           styleNo = ls.GetStyleId ();
//!       Utf8String         name = ls.GetStyleName ();
//!       }
//!@endcode
//!  @ingroup LineStyleManagerModule
//!  @bsiclass
//=======================================================================================
struct          LsCacheStyleIterator
    {
    using iterator_category=std::forward_iterator_tag;
    using value_type=const LsCacheStyleEntry;
    using difference_type=std::ptrdiff_t;
    using pointer=const LsCacheStyleEntry*;
    using reference=const LsCacheStyleEntry&;

private:
    friend struct LsCache;
    LsCacheStyleIterator (LsCacheCP map, bool wantBegin);    // ctors are private. Therefore, callers cannot construct an instance. Since we don't define a private copy constructor, callers can make copies. That should be safe, since we don't define a destructor.
    LsCacheStyleIterator () {}
private:
    int                 m_index;
    void const*         m_node;
    intptr_t            m_auxData;

public:
    DGNPLATFORM_EXPORT LsCacheStyleIteratorR  operator++();
    DGNPLATFORM_EXPORT LsCacheStyleEntryCR    operator*() const;
                   LsCacheStyleEntryCP    operator->() const {return &operator*();}
    DGNPLATFORM_EXPORT bool            operator==(LsCacheStyleIteratorCR rhs) const;
    DGNPLATFORM_EXPORT bool            operator!=(LsCacheStyleIteratorCR rhs) const;
    };

//=======================================================================================
//! Used to access the loaded line styles and components of a DgnDb.
//! @ingroup LineStyleManagerModule
// @bsiclass
//=======================================================================================
struct LsCache : public RefCountedBase
    {
    friend struct DgnLineStyles;
private:
    T_LsIdTree          m_idTree;
    DgnDbR              m_dgnDb;

    LsCache(DgnDbR dgnProject) : m_dgnDb(dgnProject) { Load(); }
    ~LsCache();

    BentleyStatus Load();
    static LsCachePtr Create(DgnDbR project);

    DEFINE_BENTLEY_NEW_DELETE_OPERATORS
public:

    typedef int (*PFNameMapProcessFunc) (NameNode const* nameNode, void* arg);

    DGNPLATFORM_EXPORT LsIdNodeP FindId(DgnStyleId styleId) const;
    T_LsIdIterator FirstId() {return m_idTree.FirstEntry();}
    void AddIdEntry(LsDefinitionP nameRec);
    DGNPLATFORM_EXPORT BentleyStatus RemoveIdEntry(DgnStyleId id);
    T_LsIdTree* GetIdMap() {return &m_idTree;}
    DGNPLATFORM_EXPORT LsIdNodeP SearchIdsForName(Utf8CP name) const;
    DGNPLATFORM_EXPORT static LsDefinitionP FindInMap(DgnDbR dgndb, DgnStyleId styleId);

public:
    DGNPLATFORM_EXPORT Utf8String GetFileName() const;    //!< Name of file used to load the map.

    //! Searches the set of ID's associated with the LsCache.
    //! @param[in] styleId The ID that is used to associate an element with a line style.
    //! @return The associated LsDefinition object if found. Otherwise, it returns NULL. It
    //! also returns NULL for all of the special codes including STYLE_BYLEVEL, STYLE_BYCELL, and the
    //! reserved line codes 0 through 7.
    //! @note If the line style map is a LsDgnFileMap then the LsDefinitionP that is
    //! returned may be a pointer to a stub used to translate from element style
    //! number to name.  Call LsDgnFileMap::Resolve to find the LsDefinition that
    //! is used to stroke the element.
    DGNPLATFORM_EXPORT LsDefinitionP GetLineStyleP(DgnStyleId styleId) const;
    //! Searches the set of ID's associated with the LsCache.
    //! @param[in] styleId The ID that is used to associate an element with a line style.
    //! @return The associated LsDefinition object if found. Otherwise, it returns NULL. It
    //! also returns NULL for all of the special codes including STYLE_BYLEVEL, STYLE_BYCELL, and the
    //! reserved line codes 0 through 7.
    //! @see LsCache::GetLineStyleP
    DGNPLATFORM_EXPORT LsDefinitionCP GetLineStyleCP(DgnStyleId styleId) const;

    //!  Gets a reference to the associated DgnDb.
    //!  @remark Use DgnDb::LineStyles().GetCache() to get a reference to a DgnDb's LsCache.
    DGNPLATFORM_EXPORT DgnDbCR GetDgnDb() const;

    typedef LsCacheStyleIterator const_iterator;
    typedef const_iterator iterator;    //!< only const iteration is possible

    //! Used to step through all of the LsCacheStyleEntry entries for an LsCache. See
    //! LsCacheStyleIterator for an example.
    DGNPLATFORM_EXPORT LsCacheStyleIterator begin() const;
    DGNPLATFORM_EXPORT LsCacheStyleIterator end() const;
    };

//=======================================================================================
//! Groups methods for Line Styles.
// @bsiclass
//=======================================================================================
struct DgnLineStyles : RefCountedBase
{
    friend struct DgnDb;
private:
    typedef bmap<LsLocation, LsComponentPtr> LoadedComponents;

    DgnDbR              m_dgndb;
    LsCachePtr          m_lineStyleMap;
    LoadedComponents    m_loadedComponents;
    BeMutex             m_mutex;

    DgnLineStyles(DgnDbR db) : m_dgndb(db) { }
public:
    DGNPLATFORM_EXPORT static LsComponent* GetLsComponent(LsLocationCR location);
    DGNPLATFORM_EXPORT LsComponentPtr GetLsComponent(LsComponentId componentId);
    LsComponent* GetComponent(LsLocationCR location);

public:
    //! Inserts a new line style into the specified DefinitionModel.
    DGNPLATFORM_EXPORT BentleyStatus Insert(DgnStyleId& newStyleId, DgnModelId modelId, Utf8CP name, Utf8CP description, LsComponentId id, uint32_t flags, double unitDefinition);

    //! Inserts a new line style into the DictionaryModel.
    DGNPLATFORM_EXPORT BentleyStatus Insert(DgnStyleId& newStyleId, Utf8CP name, LsComponentId id, uint32_t flags, double unitDefinition);

    //! Updates an a Line Style in the styles table..
    DGNPLATFORM_EXPORT BentleyStatus Update(DgnStyleId styleId, Utf8CP name, LsComponentId id, uint32_t flags, double unitDefinition);

    DGNPLATFORM_EXPORT LsCacheR GetCache();
};


//=======================================================================================
//! Provides access to the line style data in the element table.
//!  @ingroup LineStyleManagerModule
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LineStyleElement : DefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_LineStyle, DefinitionElement);

private:
    static DgnCode CreateCode(DgnDbR db, Utf8StringCR name) { return CodeSpec::CreateCode(BIS_CODESPEC_LineStyle, db.GetDictionaryModel(), name); }
    static DgnCode CreateCode(DefinitionModelCR model, Utf8StringCR name) { return CodeSpec::CreateCode(BIS_CODESPEC_LineStyle, model, name); }
    static DgnCode CreateCode(DgnDbR db, DgnModelId modelId, Utf8StringCR name)
        {
        DefinitionModelPtr model = db.Models().Get<DefinitionModel>(modelId);
        return model.IsValid() ? CodeSpec::CreateCode(BIS_CODESPEC_LineStyle, *model, name) : DgnCode();
        }

protected:
    DGNPLATFORM_EXPORT DgnDbStatus _OnDelete() const override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsert() override;
    DgnCode _GenerateDefaultCode() const override { return DgnCode(); }
    bool _SupportsCodeSpec(CodeSpecCR codeSpec) const override { return !codeSpec.IsNullCodeSpec(); }

public:
    static ECN::ECClassId QueryECClassId(DgnDbR db) { return db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_LineStyle); }
    static DgnClassId QueryDgnClassId(DgnDbR db) { return DgnClassId(QueryECClassId(db)); }

    explicit LineStyleElement(DgnDbR db) : T_Super(CreateParams(db, DgnModel::DictionaryId(), QueryDgnClassId(db), DgnCode())) {}
    explicit LineStyleElement(DefinitionModelR model) : T_Super(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryDgnClassId(model.GetDgnDb()), DgnCode())) {}
    explicit LineStyleElement(CreateParams const& params) : T_Super(params) {}
    static LineStyleElementPtr Create(DgnDbR db) { return new LineStyleElement(db); }
    static LineStyleElementPtr Create(DefinitionModelR model) { return new LineStyleElement(model); }
    LineStyleElementPtr CreateCopy() const { return MakeCopy<LineStyleElement>(); }

    DgnStyleId GetId() const {return DgnStyleId(GetElementId().GetValue());}
    Utf8String GetName() const { return GetCode().GetValue().GetUtf8(); }
    void SetName(Utf8CP value) { T_Super::SetCode(CreateCode(GetDgnDb(), GetModelId(), value)); /* Only SetName is allowed to SetCode. */ }
    Utf8String GetDescription() const { return GetPropertyValueString(LINESTYLE_PROP_Description); }
    void SetDescription(Utf8CP value) { SetPropertyValue(LINESTYLE_PROP_Description, value); }
    Utf8String GetData() const { return GetPropertyValueString(LINESTYLE_PROP_Data); }
    void SetData(Utf8CP value) { SetPropertyValue(LINESTYLE_PROP_Data, value); }

    static DgnStyleId QueryId(DgnDbR db, Utf8CP name) { return DgnStyleId(db.Elements().QueryElementIdByCode(CreateCode(db, name)).GetValueUnchecked()); }
    static DgnStyleId QueryId(DefinitionModelR model, Utf8CP name) { return DgnStyleId(model.GetDgnDb().Elements().QueryElementIdByCode(CreateCode(model, name)).GetValueUnchecked()); }
    static LineStyleElementCPtr Get(DgnDbR db, Utf8CP name) { return Get(db, QueryId(db, name)); }
    static LineStyleElementCPtr Get(DefinitionModelR model, Utf8CP name) { return Get(model.GetDgnDb(), QueryId(model, name)); }
    static LineStyleElementCPtr Get(DgnDbR db, DgnStyleId id) { return db.Elements().Get<LineStyleElement>(id); }
    static LineStyleElementPtr GetForEdit(DgnDbR db, DgnStyleId id) { return db.Elements().GetForEdit<LineStyleElement>(id); }
    static LineStyleElementPtr GetForEdit(DgnDbR db, Utf8CP name) { return GetForEdit(db, QueryId(db, name)); }
    LineStyleElementCPtr Insert() { return GetDgnDb().Elements().Insert<LineStyleElement>(*this); }

    //=======================================================================================
    // @bsiclass
    //=======================================================================================
    struct Entry : ECSqlStatementEntry
    {
        DEFINE_T_SUPER(ECSqlStatementEntry);
        friend struct ECSqlStatementIterator<Entry>;
        friend struct LineStyleElement;

    private:
        Entry() : T_Super(nullptr) {}
        Entry(BeSQLite::EC::ECSqlStatement* stmt) : T_Super(stmt) {}

    public:
        DgnStyleId GetElementId() const { return m_statement->GetValueId<DgnStyleId>(0); }
        Utf8CP GetName() const { return m_statement->GetValueText(1); }
        Utf8CP GetDescription() const { return m_statement->GetValueText(2); }
        Utf8CP GetData() const { return m_statement->GetValueText(3); }
    };

    typedef ECSqlStatementIterator<Entry> Iterator;

    static DgnStyleId ImportLineStyle(DgnStyleId srcStyleId, DgnImportContext& importer);
    DGNPLATFORM_EXPORT static Iterator MakeIterator(DgnDbR);
    DGNPLATFORM_EXPORT static size_t QueryCount(DgnDbR);
};

namespace dgn_ElementHandler
{
    //=======================================================================================
    //! The handler for annotation text styles
    //! @bsistruct
    //=======================================================================================
    struct LineStyleHandler : Definition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_LineStyle, LineStyleElement, LineStyleHandler, Definition, DGNPLATFORM_EXPORT);
    };
}

END_BENTLEY_DGN_NAMESPACE

/** @endcond */
