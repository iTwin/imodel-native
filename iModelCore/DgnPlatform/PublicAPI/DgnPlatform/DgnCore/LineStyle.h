/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/LineStyle.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "IViewDraw.h"  // For LineStyleParams
#include "../DgnHost.h"
//__PUBLISH_SECTION_END__
#include "ViewContext.h"  // For ILineStyleComponent
#include "../DgnPlatform.r.h"
#include <RmgrTools/Tools/msstrlst.h>
#include <DgnPlatform/Tools/KeyTree.h>

#define STYLELINK_ID        0x79f9      /* Attribute ID (rad50 'STY')  */
#define LSTYPE_Internal     0
#define LSID_DEFAULT        0
#define LSID_HARDWARE       0x80000000
#define LSID_HWMASK         0x0000000f
#define LSUNIT_FACTOR       100000.0


/*-----------------------------------------------------------------------
mdlLineStyle_query options
-----------------------------------------------------------------------*/
#define LSQUERY_RSCFILEHANDLE   1
#define LSQUERY_RSCFILENAME     2
#define LSQUERY_RANGE           3
#define LSQUERY_NAMEMAP         4
#define LSQUERY_IDMAP           5
#define LSQUERY_NAMELIST        6
#define LSQUERY_CURRENTSTYLE_MODE 7
#define LSQUERY_CURRENTSTYLE_STYLEID 8

/*-----------------------------------------------------------------------
mdlLineStyle_nameQuery options
-----------------------------------------------------------------------*/
#define LSQUERY_FILEMASK        0x000000ff
#define LSQUERY_STYLEID         0x80000000

/*-----------------------------------------------------------------------
mdlLineStyle_nameDelete options
-----------------------------------------------------------------------*/
#define LSNAME_FREECOMPONENTS   0x00000001
#define LSNAME_KEEPRESOURCE     0x00000002

/*-----------------------------------------------------------------------
mdlLineStyle_getElementDescr options
-----------------------------------------------------------------------*/
#define LSOPT_SETGG             0x01

/*-----------------------------------------------------------------------
mdlLineStyle_nameGetStringList options
-----------------------------------------------------------------------*/
#define LSSL_OPT_RSCONLY        0x01
#define LSSL_OPT_MASTONLY       0x02

/*-----------------------------------------------------------------------
These apply to the LineStyleParams->mlineFlags
-----------------------------------------------------------------------*/
#define MLSFLAG_LINE        0x01
#define MLSFLAG_CAP         0x02

#define LCCAP_MAXVECS       90

struct dwgLineStyleInfo;   // this is outside the linestyle namespace.

DGNPLATFORM_TYPEDEFS    (LsOffsetComponent)
DGNPLATFORM_TYPEDEFS    (LsResourceFileMap)

//__PUBLISH_SECTION_START__
typedef uint32_t RscFileHandle;

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @ingroup LineStyleManagerModule
//! Smart pointer wrapper for LsComponent
typedef RefCountedPtr <LsComponent> LsComponentPtr;
//! @ingroup LineStyleManagerModule
//! Smart pointer wrapper for LsPointComponent
typedef RefCountedPtr <LsPointComponent> LsPointComponentPtr;
//! @ingroup LineStyleManagerModule
//! Smart pointer wrapper for LsStrokePatternComponent
typedef RefCountedPtr <LsStrokePatternComponent> LsStrokePatternComponentPtr;
//! @ingroup LineStyleManagerModule
//! Smart pointer wrapper for LsSymbolComponent
typedef RefCountedPtr <LsSymbolComponent> LsSymbolComponentPtr;
//! @ingroup LineStyleManagerModule
//! Smart pointer wrapper for LsCompoundComponent
typedef RefCountedPtr <LsCompoundComponent> LsCompoundComponentPtr;
//! @ingroup LineStyleManagerModule
//! Smart pointer wrapper for LsInternalComponent
typedef RefCountedPtr <LsInternalComponent> LsInternalComponentPtr;
//! @ingroup LineStyleManagerModule
//! Smart pointer wrapper for LsMap
typedef RefCountedPtr <LsMap> LsMapPtr;
//! @ingroup LineStyleManagerModule
//! Smart pointer wrapper for LsSystemMap
typedef RefCountedPtr <LsSystemMap>LsSystemMapPtr;
//! @ingroup LineStyleManagerModule
//! Smart pointer wrapper for LsDgnProjectMap
typedef RefCountedPtr <LsDgnProjectMap> LsDgnProjectMapPtr;

//! @ingroup LineStyleManagerModule
typedef bvector<LsComponentPtr> T_LsComponents;

/*__PUBLISH_SECTION_END__*/

/*----------------------------------------------------------------------+
|                                                                       |
|   LineStyleNameInfo                                                   |
|                                                                       |
|   Information returned from the mdlLineStyle_nameQuery function       |
|                                                                       |
+----------------------------------------------------------------------*/
struct LineStyleNameInfo
{
    uint32_t        rscFile;    // Resource file handle
    uint32_t        rscType;    // Resource type
    DgnElementId    rscID;      // Resource ID
    uint32_t        attributes; // Name Attributes
    long            styleID;    // Style ID for MASTERFILE
};

#if !defined(__midl)  // For a MIDL compile, all we care about are the values of the #define constants.

#if defined (NEEDS_WORK_DGNITEM)
/*----------------------------------------------------------------------+
|                                                                       |
|   Custom LineStyle Linkage                                            |
|                                                                       |
+----------------------------------------------------------------------*/
struct StyleLink
{
    LinkageHeader   linkHeader;

    /*-------------------------------------------------------------------
    Anything below here added to the element incrementally as needed.
    -------------------------------------------------------------------*/
    uint32_t        modifiers;

    /*-------------------------------------------------------------------
    Reserve enough space for maximum number of parameters.
    -------------------------------------------------------------------*/
    Byte modData[sizeof (LineStyleParams)];
};
#endif

#endif // !defined(__midl)

//__PUBLISH_SECTION_START__

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

//! Values held in line style definition elements; normally not used by clients of this API
//! @ingroup LineStyleManagerModule
enum class LsElementType
{
    Unknown         = 0,             //!<   Unknown, should never occur
    PointSymbol     = 1,
    Compound        = 2,
    LineCode        = 3,
    LinePoint       = 4,
    Internal        = 6,
    ElmDscr         = 5,
    NameEntry       = 51, // Place holder for name table entries
};

//! Code indicating the source of the component definition.
//! @ingroup LineStyleManagerModule
enum class LsLocationType
{
    Unknown       = 0,                 //!<   Unknown, should never occur
    System        = 3,
    LinFile       = 4,
    DgnDb    = 5
};

//! Values held in line style definition components in resource files
//! @ingroup LineStyleManagerModule
enum class LsResourceType
{
    Unknown               = 1,       //!<   Unknown, should never occur
    Internal              = 0,
    Compound              = 'lsLS',  //!<   LsCacheCompoundComponent
    LineCode              = 'lsLC',  //!<   LsStrokePatternComponent
    LinePoint             = 'lsLP',  //!<   LsPointComponent
    PointSymbol           = 'l8PS',  //!<   LsSymbolComponent containing V8 graphics
    PointSymbolV7         = 'lsPS',  //!<   LsSymbolComponent containing V7 graphics
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
// @bsiclass                                                    John.Gooding    11/2012
//=======================================================================================
struct V10ComponentBase
{
    Utf8Char      m_descr[LS_MAX_DESCR];
    void GetDescription (Utf8P target) const;
    DGNPLATFORM_EXPORT void SetDescription (Utf8CP target);
};

//=======================================================================================
//! Included in a V10LinePoint to describe how symbols are used.
// @bsiclass                                                    John.Gooding    11/2012
//=======================================================================================
struct V10PointSymbolInfo
{
    uint32_t      m_symType;              //  must specify a symbol component in the same file
    uint32_t      m_symID;                //                  ""
    uint16_t      m_strokeNo;             //  If (!(mod1 & LCPOINT_ANYVERTEX)) && (mod1 & LCPOINT_ONSTROKE)) selects stroke number from stroke pattern
    uint16_t      m_mod1;

    double        m_xOffset;
    double        m_yOffset;
    double        m_zAngle;               //  angle in degrees
};

//=======================================================================================
//! Describes the binary representation of LineCode component in a DgnDb
// @bsiclass                                                    John.Gooding    11/2012
//=======================================================================================
struct          V10LinePoint : V10ComponentBase
{
    uint32_t      m_lcType;
    uint32_t      m_lcID;
    uint32_t      m_nSymbols;
    V10PointSymbolInfo m_symbol[1];

    // Note: we used to use offsetof(V10LinePoint,m_symbol). It's not legal to use offsetof macro on a non-POD struct, however. 
    // The cast on nullptr accomplishes the same thing as offsetof. It is safe because we know that V10LinePoint uses standard layout.
    static uint32_t GetBufferSize(uint32_t numberSymbols) { return ((uint32_t)(intptr_t)&(((V10LinePoint*)nullptr)->m_symbol)) + sizeof (V10PointSymbolInfo) * numberSymbols; }
};

//=======================================================================================
// @bsiclass                                                    John.Gooding    11/2012
//=======================================================================================
struct V10StrokeData
    {
    double      m_length;         /* Stroke length                       */
    double      m_width;          /* Stroke width (or start width)       */
    double      m_endWidth;       /* End width of tapered stroke         */

    uint8_t     m_strokeMode;     /* bit 0: dash          | gap    dash  */
                                /* bit 1: trace mode    | linear ray   */
                                /* bit 2: scale mode    | off    on    */
                                /* bit 3: start invert                 */
                                /* bit 4: end invert                   */

    uint8_t     m_widthMode;      /* bit 0: left half     | off    on    */
                                /* bit 1: right half    | off    on    */
                                /* bit 2: in taper      | no     yes   */
                                /* bit 3: end taper     | no     yes   */
    uint8_t     m_capMode;
    uint8_t     m_bReserved;
    };

//=======================================================================================
//! Describes the binary representation of LsStrokePatternComponent component in a DgnDb
// @bsiclass                                                    John.Gooding    11/2012
//=======================================================================================
struct V10LineCode : V10ComponentBase
{
    double        m_phase;
    uint32_t      m_options;
    uint32_t      m_maxIterate;
    uint32_t      m_nStrokes;
    V10StrokeData m_stroke[1];
    static uint32_t GetBufferSize(uint32_t numberStrokes) { return ((uint32_t)(intptr_t)&(((V10LineCode*)nullptr)->m_stroke)) + sizeof (V10StrokeData) * numberStrokes; }
};

//=======================================================================================
//! Describe how a V10Compound component refers to the components
// @bsiclass                                                    John.Gooding    11/2012
//=======================================================================================
struct V10ComponentInfo
    {
    uint32_t    m_type;
    uint32_t    m_id;
    double      m_offset;
    };

//=======================================================================================
//! Describes the binary representation of LsCompoundComponent component in a DgnDb
// @bsiclass                                                    John.Gooding    11/2012
//=======================================================================================
struct V10Compound : V10ComponentBase
{
    uint32_t        m_auxType;
    uint32_t        m_nComponents;
    V10ComponentInfo   m_component[1];
    static uint32_t GetBufferSize(uint32_t numberComponents) { return ((uint32_t)(intptr_t)&(((V10Compound*)nullptr)->m_component)) + sizeof (V10ComponentInfo) * numberComponents; }
};

//=======================================================================================
//! Describes the binary representation of an LsSymbolComponent component in a DgnDb
// @bsiclass                                                    John.Gooding    11/2012
//=======================================================================================
struct V10Symbol : V10ComponentBase
{
    SymbolRange m_range;
    double      m_scale;

    uint32_t    m_symFlags;
    uint32_t    m_symbolId;
    static uint32_t GetBufferSize() { return sizeof (V10Symbol); }
};
#pragma pack(pop)

//=======================================================================================
// @bsiclass                                                    John.Gooding    10/2013
//=======================================================================================
template <typename T> struct RscPtr
    {
    T*          m_data;
    RscPtr() { m_data = nullptr; }
    ~RscPtr() {}
    };

struct LineCodeRscPtr : RscPtr<LineCodeRsc>
    {
    DGNPLATFORM_EXPORT ~LineCodeRscPtr();
    };

struct LinePointRscPtr : RscPtr<LinePointRsc>
    {
    DGNPLATFORM_EXPORT ~LinePointRscPtr();
    };

struct LineStyleRscPtr : RscPtr<LineStyleRsc>
    {
    DGNPLATFORM_EXPORT ~LineStyleRscPtr();
    };

struct PointSymRscPtr : RscPtr<PointSymRsc>
    {
    DGNPLATFORM_EXPORT ~PointSymRscPtr();
    };


//__PUBLISH_SECTION_END__
enum DwgShapeFlag
{
    SHAPEFLAG_TextSymbol    = 0x0002,
    SHAPEFLAG_ShapeSymbol   = 0x0004,
};

struct Centerline;
struct LsRscReader;

typedef struct DecomposedDwgLine*               DecomposedDwgLineP;
typedef struct Centerline*                      CenterlineP;
typedef struct LsLocation*                      LsLocationP;
typedef struct LsLocation const *               LsLocationCP;
typedef struct LsElmComponent*                  LsElmComponentP;
typedef struct LsPointSymbolComponent*          LsPointSymbolComponentP;
typedef struct LsPointSymbolComponent const*    LsPointSymbolComponentCP;
typedef struct LsLineCodeComponent*             LsLineCodeComponentP;
typedef struct LsLineCodeComponent const*       LsLineCodeComponentCP;


//=======================================================================================
// @bsiclass                                                      Chuck.Kirschman   01/03
//=======================================================================================
struct  LsLocation
{
private:
    LsLocationType      m_fileType;
    DgnDbP         m_dgndb;
    LsResourceType      m_rscType;          // Component resource type
    uint32_t            m_rscID;            // Component resource ID

    bool                m_idDefined;

    void Init ()
    {
        m_fileType      = LsLocationType::Unknown;
        m_rscType       = LsResourceType::Unknown;
        m_rscID         = 0;
        m_dgndb       = NULL;

        m_idDefined     = true;
    }

public:
    ~LsLocation         ();
    LsLocation          () { Init (); }
    LsLocation          (LsLocationCP base) { SetFrom (base); }

    void SetLocation    (DgnDbR project, uint32_t componentType, uint32_t componentId);  //  after conversion to DgnDb this should be the only SetLocation
    void SetFrom        (LsLocationCP base, uint32_t rscType);
    void SetFrom        (LsLocationCP base);
    void UpdateRscID    (uint32_t value) { m_idDefined = true; m_rscID = value; }
    void SetIdDefined   (bool value) { m_idDefined = false; }
    bool GetIdDefined   () const { return m_idDefined; }
    void SetRscType     (LsResourceType rscType)         {m_rscType = rscType;}

    LsLocationType                      GetSourceType   () const   {return m_fileType;}
    DGNPLATFORM_EXPORT intptr_t             GetFileKey      () const;
    LsResourceType                      GetRscType      () const   {return m_rscType;}
    DGNPLATFORM_EXPORT LsElementType        GetElementType  () const;
    uint32_t                            GetRscID        () const   {return m_rscID;}
    DGNPLATFORM_EXPORT uint64_t GetIdentKey () const;
    DgnDbP GetDgnDb () const {return m_dgndb;}

    DGNPLATFORM_EXPORT struct LsMap* GetMapPtr () const;

    bool IsValid () const;
    bool IsInternalDefault () const {return (LsResourceType::Internal == m_rscType && LSID_DEFAULT == GetIdentKey()) ? true : false;}
    int32_t GetSeedID () const;

    BentleyStatus GetLineCodeLocation (struct LsRscReader*);
    BentleyStatus GetPointSymbolLocation (struct LsRscReader*, int symbolNumber);
    BentleyStatus GetCompoundComponentLocation (struct LsRscReader*, int componentNumber);

    DGNPLATFORM_EXPORT void* GetResource () const;
};

//=======================================================================================
// @bsiclass                                                      Keith.Bentley   03/03
//=======================================================================================
struct          LsRscReader
{
protected:
    LsLocationCP        m_source;
    DgnDbR         m_dgndb;
    uint32_t            m_rscType;
    LineStyleRsc*       m_rsc;

public:
    LsRscReader (LsLocationCP source, DgnDbR project) : m_dgndb(project)
    {
        m_source    = source;
        m_rsc       = NULL;
        m_rscType   = (uint32_t)m_source->GetRscType();
    }

    virtual ~LsRscReader() {}
    virtual BentleyStatus _LoadDefinition() = 0;

    LsLocationCP   GetSource()   {return m_source;}
    DgnDbR    GetDgnDb () { return m_dgndb; }
    LineStyleRsc*  GetRsc()      {return m_rsc;}
    uint32_t       GetRscType()  {return m_rscType;}

    // This is used by the cache insert function for editing
    // It explicitly does not free any previous resource because the resource is owned by the caller in this case.
    void           SetRsc(LineStyleRsc *pRsc) {m_rsc = pRsc;}

    static LsRscReader* GetRscReader (LsLocationCP source, DgnDbR dgnProject);
};

/*=================================================================================**//**
    @bsiclass                                                   John.Gooding    10/2012
+===============+===============+===============+===============+===============+======*/
struct          LsDgnDbReader : public LsRscReader
{

public:
    LsDgnDbReader (LsLocationCP source, DgnDbR project) : LsRscReader (source, project) {}
    virtual ~LsDgnDbReader();

    virtual BentleyStatus _LoadDefinition() override;
};

//=======================================================================================
// @bsiclass                                                    John.Gooding    10/2013
//=======================================================================================
struct          LsComponentId
    {
    uint32_t m_type;
    uint32_t m_id;
    bool operator<(LsComponentId const&r) const
        {
        if (this->m_type < r.m_type)
            return true;

        if (this->m_type > r.m_type)
            return false;

        return this->m_id < r.m_id;
        }
    };

//__PUBLISH_SECTION_START__

//=======================================================================================
// @bsiclass
//=======================================================================================
struct          LsComponent : public RefCountedBase
//__PUBLISH_SECTION_END__
    , ILineStyleComponent
//__PUBLISH_SECTION_START__
    {
//__PUBLISH_SECTION_END__
private:
    LsLocation          m_location;    // Where to find components of resource
    WString             m_descr;

    BentleyStatus       StrokeContinuousArc (ViewContextP, LineStyleSymbCP, DPoint3dCP origin, RotMatrix const*, double r0, double r1,
                                             double const* start, double const* sweep, DPoint3dCP range) const;
protected:
    bool                m_isDirty;

    // Should only be used for setting descr in resource definition
    void      CopyDescription (CharP buffer) const;                      // WIP_CHAR_OK
    static void GetNextComponentId (uint32_t& id, DgnDbR project, BeSQLite::PropertySpec spec);

public:
    LsComponent (DgnDbR, uint32_t componentType, uint32_t componentId);
    LsComponent (LsLocationCP location);
    LsComponent (LsComponent const* base) : m_isDirty (false)
        {
        m_location.SetFrom (&base->m_location);
        }

    DGNPLATFORM_EXPORT static LineStyleStatus AddComponentAsProperty (uint32_t& componentId, DgnDbR project, BeSQLite::PropertySpec spec, void const*data, uint32_t dataSize);
    static LineStyleStatus GetComponentList (T_LsComponents& components, intptr_t fileKey, uint32_t componentType);

    bool                IsWidthDiscernible (ViewContextP, LineStyleSymbCP, DPoint3dCR) const;
    bool                IsSingleRepDiscernible (ViewContextP, LineStyleSymbCP, DPoint3dCR) const;
    LsLocationCP        GetLocation() const   {return &m_location;}

    virtual             ~LsComponent() {};
    virtual bool        _IsBySegment() const {return false;}
    virtual bool        _HasLineCodes() const {return false;}
    virtual bool        _IsContinuousOrSingleDash() const {return false;}
    virtual double      _GetMaxWidth (DgnModelP modelRef)       const {return 0.0;}
    virtual bool        _IsAffectedByWidth (bool currentStatusOnly) const {return false;}
    virtual bool        _ContainsComponent (LsComponentP other) const {return other == this;}
    virtual bool        _HasUniformFullWidth (double *pWidth) const  {if (pWidth) *pWidth=0.0; return false;}
    virtual double      _CalcRepetitions (LineStyleSymbCP) const;

    virtual bool        _IsContinuous           () const override  {return false;}
    virtual bool        _HasWidth               () const override  {return true;}
    virtual double      _GetLength              () const override  {return 0.0;}
    virtual void        _PostProcessLoad        (DgnModelP modelRef) { return; }
    virtual void        _ClearPostProcess       () { return; }
    virtual StatusInt   _StrokeLineString       (ViewContextP, LineStyleSymbP, DPoint3dCP, int nPts, bool isClosed) const override;
    virtual StatusInt   _StrokeLineString2d     (ViewContextP, LineStyleSymbP, DPoint2d const*, int nPts, double zDepth, bool isClosed) const override;
    virtual StatusInt   _StrokeArc              (ViewContextP, LineStyleSymbP, DPoint3dCP origin, RotMatrix const*, double r0, double r1,
                                                    double const* start, double const* sweep, DPoint3dCP range) const override;
    virtual StatusInt   _StrokeBSplineCurve     (ViewContextP context, LineStyleSymbP lsSymb, MSBsplineCurve const*, double const* tolerance) const override;
    virtual StatusInt   _DoStroke               (ViewContextP, DPoint3dCP, int, LineStyleSymbCP) const {return SUCCESS;}
    virtual void        _LoadFinished           () { m_isDirty = false; }

    //  Defer until update supported
    DGNPLATFORM_EXPORT void SetDescription (WStringCR descr) { m_descr = descr; }
    DGNPLATFORM_EXPORT void SetDescription (WCharP descr) { m_descr = WString (descr); }

    DGNPLATFORM_EXPORT static void QueryComponentIds(bset<LsComponentId>& ids, DgnDbCR project, LsResourceType lsType);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

    //! Gets the component's resource ID if the definition comes from a resource file, or element ID
    //! if the definition comes from a DgnFile.
    DGNPLATFORM_EXPORT uint64_t GetId () const;
    //! Retrieves the LsResourceType value specifying the type of component. This can be used regardless of
    //! whether the definition comes from a resource file or DgnFile.
    DGNPLATFORM_EXPORT LsElementType GetElementType () const;
    //! Retrieves a value indicating whether the component definition comes from a resource file or DgnFile.
    DGNPLATFORM_EXPORT LsLocationType GetLocationType () const;
    //! Retrieves a DgnDbP if the component is defined in a project file; NULL otherwise.
    DGNPLATFORM_EXPORT DgnDbP GetDgnDbP () const;
    //! Retrieves the name of the resource file or DgnFile that contains the component definition.
    DGNPLATFORM_EXPORT WString GetFileName () const;
    //! Retrieves the description of the component; this is the description that is stored with the definition.
    DGNPLATFORM_EXPORT WString GetDescription () const;
    };

struct          LsSymbolGraphicsCreator;
typedef RefCountedPtr<LsSymbolGraphicsCreator>          LsSymbolGraphicsCreatorPtr;

//=======================================================================================
//!  Used to add graphics to an LsSymbolComponent.
//!  To add graphics to an LsSymbolComponent, first get an LsSymbolGraphicsCreator by
//!  calling LsSymbolComponent::GetGraphicsCreator().  Then make a series of calls to
//!  the Process.. methods.
//!  @ingroup LineStyleManagerModule
// @bsiclass
//=======================================================================================
struct LsSymbolGraphicsCreator : RefCountedBase
{
//__PUBLISH_SECTION_END__
friend struct LsSymbolComponent;

private:

LsSymbolComponentPtr    m_component;
DgnModelR               m_contextModel;
uint32_t                m_color;
uint32_t                m_fillColor;
uint32_t                m_linecode;
uint32_t                m_weight;
LsSymbolGraphicsCreator (LsSymbolComponentP component, DgnModelR contextModel) : m_component (component), m_contextModel (contextModel)
    {
    m_color     = 0;
    m_fillColor = 0;
    m_linecode  = 0;
    m_weight    = 0;
    }

public:
//__PUBLISH_SECTION_START__

//! Establish symbology to be used by subsequent calls to add geometry.
//! @param[in] color Line color to be used.
//! @param[in] linecode Line code to be used (MIN_LINECODE to MAX_LINECODE).
//! @param[in] weight Line weight to be used.
//! @param[in] fillColor Fill color to use for AddCurveVector when isFilled is true.
DGNPLATFORM_EXPORT void SetupSymbology (uint32_t color, uint32_t linecode, uint32_t weight, uint32_t fillColor);

//! Add text or text geometry.
//! @param[in] text The text data.
DGNPLATFORM_EXPORT void AddTextString (TextStringCR text);

//! Add geometry for a single curve primitive.
//! @param[in] curve The curve data.
DGNPLATFORM_EXPORT void AddCurvePrimitive (ICurvePrimitiveCR curve);

//! Add geometry representing an open path, closed path. or region.
//! @param[in] curves The curve data.
//! @param[in] isFilled A closed path or region should have opaque fill.
DGNPLATFORM_EXPORT void AddCurveVector (CurveVectorCR curves, bool isFilled);

}; // LsSymbolGraphicsCreator

//=======================================================================================
//!  Represents a component that contains graphics.
//!  @ingroup LineStyleManagerModule
// @bsiclass
//=======================================================================================
struct          LsSymbolComponent : public LsComponent
//__PUBLISH_SECTION_END__
    , public IDisplaySymbol
//__PUBLISH_SECTION_START__
    {
//__PUBLISH_SECTION_END__
private:
    bool                m_isModified;
    WChar               m_descr[LS_MAX_DESCR];
    WChar               m_styleName[LS_MAX_NAME];   // Stored on element, for round trip.  Kind of dumb though; remove in future file format.

    DgnElementPtrVec    m_elements;

    size_t              m_xGraphicsSize;
    Byte const*         m_xGraphicsData;
    double              m_storedScale;              //
    double              m_muDef;                    // Set to m_storedScale if it is non-zero. Otherwise, it is 1/uorPerMaster for the model ref used in the PostProcessLoad step;
    DPoint3d            m_symSize;
    DPoint3d            m_symBase;                  // Not needed to display; used just to reconstruct range for GetRange method
    uint32_t            m_symFlags;                 // Flags from point symbol resource
    bool                m_postProcessed;

    explicit LsSymbolComponent (LsLocationCP pLocation);
    virtual ~LsSymbolComponent ();

public:
    static LsSymbolComponent* LoadPointSym  (LsRscReader* reader);
    static LsSymbolComponentPtr Create (LsLocation& location) { LsSymbolComponentP retval = new LsSymbolComponent (&location); retval->m_isDirty = true; return retval; }
    DGNPLATFORM_EXPORT static void StreamElements (bvector<Byte>&elementData, DgnElementPtrVec const& );

    double              GetMuDef            () const {return m_muDef;}
    DPoint3dCP          GetSymSize          () const {return &m_symSize;}
    uint32_t            GetFlags            () const {return m_symFlags;}
    bool                IsNotScaled         () const {return 0 != (m_symFlags & LSSYM_NOSCALE);}

    virtual void        _PostProcessLoad    (DgnModelP modelRef) override;
    virtual void        _ClearPostProcess   () override;
    virtual void        _Draw               (ViewContextR);
    virtual StatusInt   _GetRange           (DRange3dR range) const;

    void                SetXGraphics        (Byte const *data, size_t dataSize);
    Byte const*         GetXGraphicsData    () const {return m_xGraphicsData;}
    size_t              GetXGraphicsSize    () const {return m_xGraphicsSize;}
    DgnModelP           GetSymbolDgnModel   (ViewContextCP context) const;
    void                SetMuDef            (double mudef) {m_muDef = mudef;}
    void                SetSymSize          (DPoint3dCP sz){m_symSize = *sz;}
    void                SetSymBase          (DPoint3dCP sz){m_symBase = *sz;}
    void                SetFlags            (uint32_t flags) {m_symFlags = flags;}
    //  Should never be called; symbol components should never be drawn this way.  Therefore, a line style
    //  resource must not directly refer to a symbol component.
    virtual StatusInt   _DoStroke           (ViewContextP, DPoint3dCP, int, LineStyleSymbCP) const override;
    BentleyStatus       CreateFromComponent (LsPointSymbolComponentCP lpsComp);

    DgnElementPtrVec& GetElementsR() {return m_elements;}
    DgnElementPtrVec const& GetElements () const {return m_elements;}

    DGNPLATFORM_EXPORT void AddGraphics (DgnElementPtr&);
    DGNPLATFORM_EXPORT void FreeGraphics (bool freeDescr, bool freeXGraphics);

    DGNPLATFORM_EXPORT static BentleyStatus AddSymbolGraphicsAsProperty (uint32_t& componentId, DgnDbR project, Byte const*data, size_t size, PointSymRsc::SymbolType symbolType);

    static BentleyStatus CreateRscFromDgnDb(PointSymRsc** rscOut, DgnDbR project, uint32_t componentId, bool useRscComponentTypes);
    DGNPLATFORM_EXPORT static BentleyStatus GetRscFromDgnDb(PointSymRscPtr& ptr, DgnDbR project, uint32_t componentId);

//__PUBLISH_SECTION_START__
public:
    //!  Used to retrieve raw V7 elements
    typedef bvector<Byte> T_V7ElementBlock;
    //!  True if the LsSymbolComponent contains V7 elements
    DGNPLATFORM_EXPORT bool                     IsV7                    () const;
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

    //!  Returns a LsSymbolGraphicsCreator that the caller can use to add graphics to an LsSymbolComponent.
    DGNPLATFORM_EXPORT LsSymbolGraphicsCreatorPtr GetGraphicsCreator (LineStyleStatus& status, DgnModelR dgnCache, bool clearCurrentGraphics);
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
//__PUBLISH_SECTION_END__
private:
public:
    LsSymbolComponentPtr  m_symbol;
    LsPointComponentP   m_parent;

    uint32_t            m_mod1;

    DPoint3d            m_offset;
    double              m_angle;

    int                 m_strokeNo;

    DgnElementPtrVec const* GetElements() const {return (NULL != m_symbol.get() ? &m_symbol->GetElements() : NULL);}
    uint32_t            GetMod1             () const {return m_mod1;}
    double              _GetMaxWidth         (DgnModelP modelRef) const;

    StatusInt           Output              (ViewContextP, LineStyleSymbCP, DPoint3dCP org, DPoint3dCP dir, double const* xScale=0,
                                                DPoint3dCP clipOrg=0, DPoint3dCP clipEnd=0) const;
                        LsSymbolReference   (LsSymbolComponentP symbol, LsPointComponentP parent,
                                                uint32_t mod1, double xOffset, double yOffset, double radians, int strokeNumber);
    DGNPLATFORM_EXPORT      LsSymbolReference       ();

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
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
        VERTEX_LineOrigin   = 0x04,    // Symbol at origin of element
        VERTEX_LineEnd      = 0x08,    // Symbol at end of element
        VERTEX_Each         = 0x10,    // Symbol at each vertex
        VERTEX_Any          = (VERTEX_LineOrigin | VERTEX_LineEnd | VERTEX_Each)
        };

    DGNPLATFORM_EXPORT bool                     GetNoPartial            () const;
    DGNPLATFORM_EXPORT void                     SetNoPartial            (bool value);
    DGNPLATFORM_EXPORT bool                     GetClipPartial          () const;
    DGNPLATFORM_EXPORT void                     SetClipPartial          (bool value);
    DGNPLATFORM_EXPORT bool                     GetStretchable          () const;
    DGNPLATFORM_EXPORT void                     SetStretchable          (bool value);
    DGNPLATFORM_EXPORT bool                     GetDgnDb              () const;
    DGNPLATFORM_EXPORT void                     SetDgnDb              (bool value);
    DGNPLATFORM_EXPORT bool                     GetUseColor             () const;
    DGNPLATFORM_EXPORT void                     SetUseColor             (bool value);
    DGNPLATFORM_EXPORT bool                     GetUseWeight            () const;
    DGNPLATFORM_EXPORT void                     SetUseWeight            (bool value);
    DGNPLATFORM_EXPORT StrokeJustification      GetJustification        () const;
    DGNPLATFORM_EXPORT void                     SetJustification        (StrokeJustification value);
    DGNPLATFORM_EXPORT RotationMode             GetRotationMode         () const;
    DGNPLATFORM_EXPORT void                     SetRotationMode         (RotationMode value);
    DGNPLATFORM_EXPORT VertexMask               GetVertexMask           () const;
    DGNPLATFORM_EXPORT void                     SetVertexMask           (VertexMask value);
    DGNPLATFORM_EXPORT double                   GetXOffset              () const;
    DGNPLATFORM_EXPORT void                     SetXOffset              (double value);
    DGNPLATFORM_EXPORT double                   GetYOffset              () const;
    DGNPLATFORM_EXPORT void                     SetYOffset              (double value);
    DGNPLATFORM_EXPORT double                   GetAngle                () const;
    DGNPLATFORM_EXPORT void                     SetAngle                (double value);
    DGNPLATFORM_EXPORT int                      GetStrokeNumber         () const;
    DGNPLATFORM_EXPORT void                     SetStrokeNumber         (int value);
    DGNPLATFORM_EXPORT LsSymbolComponentP       GetSymbolComponentP     () const;
    DGNPLATFORM_EXPORT LsSymbolComponentCP      GetSymbolComponentCP    () const;
    DGNPLATFORM_EXPORT void                     SetSymbolComponent      (LsSymbolComponentR symbolComponent);
    };

//__PUBLISH_SECTION_END__

//=======================================================================================
// @bsiclass
//=======================================================================================
struct  LsOffsetComponent
    {
    double          m_offset;
    LsComponentPtr  m_subComponent;

    LsOffsetComponent (double offset, LsComponentP subComponent) : m_offset (offset), m_subComponent (subComponent) {}
    };

//__PUBLISH_SECTION_START__

//=======================================================================================
//!  An LsComponent that represents a collection of LsComponents, along with an offset
//!  for each component.
//!  @ingroup LineStyleManagerModule
// @bsiclass
//=======================================================================================
struct          LsCompoundComponent : public LsComponent
{
//__PUBLISH_SECTION_END__
private:
    typedef  bvector <LsOffsetComponent> T_ComponentsCollection;
    typedef  bvector <LsOffsetComponent>::iterator T_ComponentsCollectionIter;
    typedef  bvector <LsOffsetComponent>::const_iterator T_ComponentsCollectionConstIter;

    T_ComponentsCollection m_components;
    DPoint2d            m_size;
    bool                m_postProcessed;

                    LsCompoundComponent         (LsLocationCP pLocation);
protected:
    virtual         ~LsCompoundComponent        ();

public:
    static LsCompoundComponentP  LoadCompoundComponent  (LsRscReader*reader);
    static LsCompoundComponentPtr Create (LsLocation& location) { LsCompoundComponentP retval = new LsCompoundComponent (&location); retval->m_isDirty = true; return retval; }
    void            CalculateSize                       (DgnModelP modelRef);

    static BentleyStatus CreateRscFromDgnDb(LineStyleRsc** rscOut, DgnDbR project, uint32_t componentId, bool useRscComponentTypes);
    DGNPLATFORM_EXPORT static BentleyStatus GetRscFromDgnDb(LineStyleRscPtr& ptr, DgnDbR project, uint32_t componentId);
    DGNPLATFORM_EXPORT static LineStyleStatus AddToProject (uint32_t& newId, DgnDbR project, LineStyleRsc const& lsRsc);

    virtual void    _PostProcessLoad            (DgnModelP modelRef) override;
    virtual void    _ClearPostProcess           () override;
    size_t          GetNumComponents            () const {return m_components.size ();}
    double          GetOffset                   (size_t index)   const   {return m_components[index].m_offset;}
    virtual double  _GetLength                  () const {return m_size.x;}
    virtual double  _GetMaxWidth                 (DgnModelP modelRef) const override   {return m_size.y;}
    virtual bool    _HasWidth                   () const override;
    virtual bool    _IsAffectedByWidth           (bool currentStatusOnly) const override;
    virtual bool    _IsBySegment                 () const override;
    virtual bool    _HasLineCodes                () const override;
    virtual bool    _ContainsComponent           (LsComponentP other) const override;
    void            Free                        (bool    sub);
    virtual StatusInt _DoStroke                 (ViewContextP, DPoint3dCP, int, LineStyleSymbCP) const override;
    bool            _HasUniformFullWidth         (double *pWidth)   const;

    //  To be added when supporting write
    DGNPLATFORM_EXPORT void ClearComponents();
    DGNPLATFORM_EXPORT size_t GetResourceSize();
    DGNPLATFORM_EXPORT void InitLineStyleResource(LineStyleRsc& lsRsc);

//__PUBLISH_SECTION_START__
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

    //! Returned from LsStroke::GetCapMode()
    enum CapMode
    {
        //! 0 - Standard closed polygon (rectangle) strokes.
        LCCAP_Closed            = 0,
        //! 1 - No end cap. The stroke is displayed as two parallel lines.
        LCCAP_Open              = 1,
        //! 2 - The end of the stroke is extended by half the stroke width.
        LCCAP_Extended          = 2,
        //! >= 3 - cap stroked as an arc and the value indicates the number of vectors in the arc.
        LCCAP_Hexagon           = 3,
        //! 4 vectors in the arc
        LCCAP_Octagon           = 4,
        //! 5 vectors in the arc
        LCCAP_Decagon           = 5,
        LCCAP_Arc               = 30,
    };

//__PUBLISH_SECTION_END__
    friend struct LsStrokePatternComponent;
private:
    double          m_length;         // Stroke length
    double          m_orgWidth;       // Stroke origin width
    double          m_endWidth;       // Stroke end width

    Byte m_strokeMode;     // bit 0: dash, first rep | gap    dash
                                      // bit 1: dash, int. rep  | gap    dash
                                      // bit 2: dash, last rep  | gap    dash
                                      // bit 4: rigid           | on     off
                                      // bit 5: stretchable     | on     off

    Byte m_widthMode;      // bit 0: left half       | off    on
                                      // bit 1: right half      | off    on

    Byte m_capMode;        // 0 = closed       1 = open
                                      // 2 = extended     3 = hexagon
                                      // 4 = octagon      5 = decagon
                                      // and so on, (n vectors in cap - up to 255)

    void        Init (double length, double orgWidth, double endWidth, WidthMode widthMode, CapMode capMode)
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


    DGNPLATFORM_EXPORT void  SaveToResource (StrokeData& strokeData);

                LsStroke (double length, double startWidth, double endWidth, WidthMode widthMode, CapMode capMode);
    explicit    LsStroke (LsStroke const&);

//__PUBLISH_SECTION_START__
private:
    explicit                   LsStroke ();

public:
    DGNPLATFORM_EXPORT void                     SetIsDash           (bool isOn);
    DGNPLATFORM_EXPORT bool                     IsDash              () const;   //  Stroke Type Dash vs. Stroke Type Gap
    DGNPLATFORM_EXPORT void                     SetIsDashFirst      (bool isOn);
    DGNPLATFORM_EXPORT bool                     IsDashFirst         () const;   //  Computed from IsDash and invert at flags, should have explicit InvertAt
    DGNPLATFORM_EXPORT void                     SetIsDashLast       (bool isOn);
    DGNPLATFORM_EXPORT bool                     IsDashLast          () const;   //      ""
    DGNPLATFORM_EXPORT void                     SetIsStretchable    (bool isOn);
    DGNPLATFORM_EXPORT bool                     IsStretchable       () const;   //  Length Fixed or Length Variable
    DGNPLATFORM_EXPORT void                     SetIsRigid          (bool isOn);
    DGNPLATFORM_EXPORT bool                     IsRigid             () const;   //  LCSTROKE_RAY is true, corresponds to Corners-ByPass, IsRigid is false if Corners-Break

    //!  There is no SetLength method.  Length is set via LsStrokePatternComponent::InsertStroke
    DGNPLATFORM_EXPORT double                   GetLength ()        const;

    //!  There is no SetStartWidth method.  Start width is set via LsStrokePatternComponent::InsertStroke
    DGNPLATFORM_EXPORT double                   GetStartWidth ()    const;

    //!  There is no SetEndWidth method.  End width is set via LsStrokePatternComponent::InsertStroke
    DGNPLATFORM_EXPORT double                   GetEndWidth ()      const;

    //!  There is no SetCapMode method.  Cap mode is set via LsStrokePatternComponent::InsertStroke
    DGNPLATFORM_EXPORT CapMode                  GetCapMode ()        const;
    //!  There is no SetWidthMode method.  Width mode is set via LsStrokePatternComponent::InsertStroke

    DGNPLATFORM_EXPORT WidthMode                GetWidthMode ()     const;

    DGNPLATFORM_EXPORT void                     SetIsTaper          (bool isOn);
    DGNPLATFORM_EXPORT bool                     IsTaper             () const;

    DGNPLATFORM_EXPORT void                     SetIsTaperEnd       (bool isOn);
    DGNPLATFORM_EXPORT bool                     IsTaperEnd          () const;
};

//__PUBLISH_SECTION_END__
//=======================================================================================
// @bsiclass
//=======================================================================================
struct     ISymbolProcess
    {
    virtual bool _ProcessSymbol (ViewContextP, Centerline const*, LineStyleSymbCP,LsStrokeCP, int strokeIndex, int endCondition) const = 0;
    };

//__PUBLISH_SECTION_START__

//=======================================================================================
//! A LsStrokePatternComponent consists of parameters that control how the individual
//! strokes are interpreted, plus up to 32 LsStroke values.
//!
//!  @ingroup LineStyleManagerModule
// @bsiclass
//=======================================================================================
struct          LsStrokePatternComponent : public LsComponent
{
//__PUBLISH_SECTION_END__
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

    bool            ApplyModifiers          (double* pOrgWidth, double* pEndWidth, LineStyleSymbCP);
    void            ApplyScale              (double scale);
    void            ApplyStretch            (double dashScale, double gapScale);
    void            ApplyWidth              (double width);
    void            ApplyAutomaticPhase     (double elementLength);
    void            ApplyIteration          (double elementLength);
    void            ApplyCenterPhase        (double length, bool isClosed);
    void            CalcPatternLength       ();

    double          GenerateStrokes         (ViewContextP, ISymbolProcess const*, LineStyleSymbCP, DPoint3dCP, int,
                                                double length, double width, double taper, int segFlag);
    void            StrokeLocal             (ViewContextP, ISymbolProcess const*, DPoint3dCP, int, double, LineStyleSymbCP, DPoint3dCP, DPoint3dCP, int segFlag) const;
    explicit LsStrokePatternComponent       (LsLocationCP pLocation);
    void            FixDashWidths           (double& orgWidth, double& endWidth, bool taper, ViewContextCP context, DPoint3dCP pt);
    void            Init                    (LineCodeRsc const* lcRsc);

public:

    static LsStrokePatternComponentP  LoadStrokePatternComponent    (LsRscReader*reader);
    static LsStrokePatternComponentPtr Create                       (LsLocation& location) { LsStrokePatternComponentP retval = new LsStrokePatternComponent (&location); retval->m_isDirty = true; return retval; };
    BentleyStatus   CreateFromRsrc          (LineCodeRsc const* pRsc);
    static BentleyStatus   CreateRscFromDgnDb      (LineCodeRsc** rsc, DgnDbR project, uint32_t componentId, bool useRscComponentTypes);
    BentleyStatus   CreateFromComponent     (LsLineCodeComponentCP lcComp);
    DGNPLATFORM_EXPORT static BentleyStatus GetRscFromDgnDb(LineCodeRscPtr& ptr, DgnDbR project, uint32_t componentId);
    DGNPLATFORM_EXPORT static LineStyleStatus AddToProject (uint32_t& newId, DgnDbR project, LineCodeRsc const& lsRsc);

    BentleyStatus   PostCreate              ();

    virtual StatusInt _DoStroke             (ViewContextP, DPoint3dCP, int, LineStyleSymbCP) const override;
    StatusInt       ProcessStroke           (ViewContextP, ISymbolProcess const*, DPoint3dCP inPoints, int nPoints, LineStyleSymbCP) const;


    bool            CheckSegmentMode        (LineStyleSymbCP) const;


    size_t          GetStrokeCount          () const {return  m_nStrokes;}
    double          _CalcRepetitions         (LineStyleSymbCP) const;
    LsStrokeP       InsertStroke            (LsStrokeCR stroke);
    void            InsertStroke            (double length, bool isDash);
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
    virtual bool    _IsBySegment             () const override {return IsSingleSegment();}

    //  Continuous mode
    void            SetContinuous           ();
    virtual bool    _IsContinuous           () const override;
    virtual bool    _IsContinuousOrSingleDash() const override;

    //  Computed properties
    //  Returns true if at least one stroke is rigid
    bool            IsRigid                 () const;
    virtual bool    _HasWidth               () const override;
    virtual bool    _HasUniformFullWidth     (double *pWidth) const override;
    virtual bool    _IsAffectedByWidth       (bool currentStatusOnly) const override;
    virtual double  _GetLength              () const override {return m_patternLength;}
    double          GetLength               (double*) const;
    virtual double  _GetMaxWidth             (DgnModelP modelRef) const override;
    bool            RequiresLength          () const;


    DGNPLATFORM_EXPORT size_t GetResourceSize         () { return sizeof(LineCodeRsc) + m_nStrokes * sizeof(LineCodeStroke) - sizeof(LineCodeStroke); }
    DGNPLATFORM_EXPORT void SaveToResource          (LineCodeRsc& resource);

//__PUBLISH_SECTION_START__
public:
    enum PhaseMode
    {
        PHASEMODE_Fixed     = 0,  //!<    Distance, use GetDistancePhase
        PHASEMODE_Fraction  = 1,  //!<    Fraction, use GetFractionalPhase
        PHASEMODE_Center    = 2,  //!<    Centered, no phase value
    };

    DGNPLATFORM_EXPORT size_t                   GetNumberStrokes    () const;
    //! Returns a pointer to an embedded LsStroke.
    DGNPLATFORM_EXPORT LsStrokeP                GetStrokeP          (size_t index);
    //! Returns a pointer to an embedded LsStroke.
    DGNPLATFORM_EXPORT LsStrokeCP               GetStrokeCP         (size_t index) const;
    DGNPLATFORM_EXPORT bool                     HasIterationLimit   () const;
    DGNPLATFORM_EXPORT int                      GetIterationLimit   () const;
    DGNPLATFORM_EXPORT double                   GetDistancePhase    () const;
    DGNPLATFORM_EXPORT double                   GetFractionalPhase  () const;
    DGNPLATFORM_EXPORT PhaseMode                GetPhaseMode        () const;
    DGNPLATFORM_EXPORT bool                     IsSingleSegment     () const;
    DGNPLATFORM_EXPORT LsStrokeP                InsertStroke        (double length, double startWidth, double endWidth, LsStroke::WidthMode widthMode, LsStroke::CapMode capMode);
    //  Iteration information
    DGNPLATFORM_EXPORT bool                     SetIterationMode    (bool limited);
    DGNPLATFORM_EXPORT void                     SetIterationLimit   (int);
    DGNPLATFORM_EXPORT void                     SetSegmentMode      (bool);
    //  Phase Mode
    DGNPLATFORM_EXPORT void                     SetPhaseMode        (int mode);
    DGNPLATFORM_EXPORT void                     SetDistancePhase    (double);
    DGNPLATFORM_EXPORT void                     SetCenterPhaseMode  ();
    DGNPLATFORM_EXPORT void                     SetFractionalPhase  (double);
 };

//=======================================================================================
//!  Combines an LsStrokePatternComponent and a collection of LsSymbolReference values.
//!  The LsStrokePatternComponent determines how the LsSymbolRefernce values are used.
//!  @ingroup LineStyleManagerModule
// @bsiclass
//=======================================================================================
struct          LsPointComponent : public LsComponent
//__PUBLISH_SECTION_END__
                                , public ISymbolProcess
//__PUBLISH_SECTION_START__
    {
//__PUBLISH_SECTION_END__
    typedef  bvector <LsSymbolReference> T_SymbolsCollection;
    typedef  bvector <LsSymbolReference>::iterator T_SymbolsCollectionIter;
    typedef  bvector <LsSymbolReference>::const_iterator T_SymbolsCollectionConstIter;

    LsStrokePatternComponentPtr m_strokeComponent;
    T_SymbolsCollection     m_symbols;
    bool                    m_postProcessed;

private:
    virtual bool                    _ProcessSymbol           (ViewContextP, Centerline const*, LineStyleSymbCP, LsStrokeCP, int strokeIndex, int endCondition) const;

    LsSymbolReferenceP              GetSymbolReferenceP     (T_SymbolsCollectionConstIter iter) const;
    LsPointComponent    (LsLocationCP pLocation);

protected:
    ~LsPointComponent   ();

public:
    virtual void                    _PostProcessLoad        (DgnModelP modelRef) override;
    virtual void                    _ClearPostProcess       () override;
    virtual bool                    _IsContinuous           () const override {return NULL==m_strokeComponent.get () ? false : m_strokeComponent->_IsContinuous();}
    virtual double                  _GetLength              () const override;
    virtual StatusInt               _DoStroke               (ViewContextP, DPoint3dCP, int, LineStyleSymbCP) const override;
    static LsPointComponent*        LoadLinePoint           (LsRscReader*reader);
    static LsPointComponentPtr      Create                  (LsLocation&location) { LsPointComponentP retval = new LsPointComponent (&location); retval->m_isDirty = true; return retval; }
    virtual double                  _GetMaxWidth             (DgnModelP modelRef)  const override;
    //  T_SymbolsCollectionConstIter    GetSymbols ()           const   {return m_symbols.begin ();}
    virtual bool                    _ContainsComponent       (LsComponentP other) const override;
    void                            Free                    (bool    sub);
    bool                            HasStrokeSymbol         () const;

    DGNPLATFORM_EXPORT size_t GetResourceSize         () { return sizeof(LinePointRsc) + m_symbols.size () * sizeof(PointSymInfo) - sizeof(PointSymInfo); }
    DGNPLATFORM_EXPORT void SaveToResource          (LinePointRsc& resource);

    static BentleyStatus   CreateRscFromDgnDb(LinePointRsc** rscOut, DgnDbR project, uint32_t componentId, bool useRscComponentTypes);
    DGNPLATFORM_EXPORT static BentleyStatus   GetRscFromDgnDb(LinePointRscPtr& ptr, DgnDbR project, uint32_t componentId);
    DGNPLATFORM_EXPORT static LineStyleStatus AddToProject (uint32_t& newId, DgnDbR project, LinePointRsc const& lpr);

//__PUBLISH_SECTION_START__
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
    DGNPLATFORM_EXPORT void                     SetStrokeComponent      (LsStrokePatternComponentP newStrokeComponent);
    DGNPLATFORM_EXPORT StatusInt                DeleteSymbolForStroke   (LsSymbolReferenceCR symRef);
    DGNPLATFORM_EXPORT LsSymbolReferenceR       AppendSymbolForStroke   (LsSymbolComponentP symbol, double xOffset, double yOffset, double radians, int strokeNo);
    };

//=======================================================================================
//!  Represents a standard line code.
//!  @ingroup LineStyleManagerModule
// @bsiclass
//=======================================================================================
struct          LsInternalComponent : public LsStrokePatternComponent
    {
//__PUBLISH_SECTION_END__
private:
    uint32_t    m_hardwareLineCode;
    LsInternalComponent (LsLocationCP pLocation);

public:
    static LsStrokePatternComponentP  LoadInternalComponent (LsRscReader*reader);
    static LsStrokePatternComponentPtr  Create1         (LsLocation&location) { return new LsInternalComponent (&location); }

    virtual bool        _IsAffectedByWidth               (bool currentStatusOnly) const override;
    virtual bool        _IsContinuous                   () const override {return 0==m_hardwareLineCode ? true : false;}
    virtual bool        _IsContinuousOrSingleDash        () const override {return _IsContinuous();}
    virtual bool        _HasLineCodes                    () const override {return IsHardwareStyle();}
    virtual StatusInt   _DoStroke                       (ViewContextP, DPoint3dCP, int, LineStyleSymbCP) const override;
    static LsInternalComponentPtr CreateInternalComponent   (LsLocation&location);

//__PUBLISH_SECTION_START__
public:
    DGNPLATFORM_EXPORT uint32_t                 GetHardwareStyle    () const;
    DGNPLATFORM_EXPORT bool                     IsHardwareStyle     () const;
    DGNPLATFORM_EXPORT uint32_t                 GetLineCode         () const;
    };

//__PUBLISH_SECTION_END__

typedef struct LsIdNode const*                LsIdNodeP;
typedef struct NameNode const*                NameNodeP;
typedef KeyTree <LsIdNode, int64_t>             T_LsIdTree;
typedef LeafNode <LsIdNode, int64_t>            T_LsIdTreeNode;
typedef KeyTree <NameNode, MSCharIKey>        T_LsNameTree;
typedef LeafNode <NameNode, MSCharIKey>       T_LsNameTreeNode;
typedef T_LsIdTree::Iterator                  T_LsIdIterator;
typedef T_LsIdTree::Iterator                  T_LsIdIterator;

/*=================================================================================**//**
    Class that performs the upgrade logic required for the line style names table element
    and the line style component definitions table element.
    @bsiclass                                                   John.Gooding    10/2012
+===============+===============+===============+===============+===============+======*/
struct LineStyleUpgradeProcessor : RefCountedBase
{
private:
    bmap <uint64_t, uint32_t> m_idMap;
    DgnElementCP   m_v8Components;
    DgnDbR         m_dgnProject;

    LineStyleStatus UpgradeLsComponent (uint32_t& v10Id, uint64_t v8Id, LsElementType type);
    DgnDbR GetDgnDb () { return m_dgnProject; }

    DgnElementCP FindComponent (uint64_t v8Id);
    static uint64_t const* GetDependents (struct LStyleDefEntryElm const& lStyleDef);
    static void SetDescription (V10ComponentBase*v10, struct LStyleDefEntryElm const& lStyleDef);
    LineStyleStatus UpgradeCompoundType (uint32_t&v10Id, struct LStyleDefEntryElm const& lStyleDef);
    LineStyleStatus UpgradeLineCode (uint32_t& v10Id, struct LStyleDefEntryElm const& lStyleDef);
    LineStyleStatus UpgradeLinePoint (uint32_t& v10Id, struct LStyleDefEntryElm const& lStyleDef);
    LineStyleStatus UpgradePointSymbol (uint32_t& v10Id, struct LStyleDefEntryElm const& lStyleDef);
    LineStyleUpgradeProcessor (DgnDbR project, DgnElementCP components);

public:
    DGNPLATFORM_EXPORT static RefCountedPtr<LineStyleUpgradeProcessor> Create (DgnDbR project, DgnElementCP components);
    DGNPLATFORM_EXPORT LineStyleStatus UpgradeLineStyleV8toV10 (struct LStyleNameEntryElm const& lStyle, Utf8CP alternateName);
};

//__PUBLISH_SECTION_START__
//!  Defines possible values for LsDefinition::GetUnitsType() and LsDefinition::SetUnitsType().
//!  @ingroup LineStyleManagerModule
enum class LsUnit
{
    Master      = 0,        //!< Master Units
    Uor         = 1,        //!< Internal Units (UORS)
    Device      = 2         //!< Pixel units
};

//=======================================================================================
//! Represents the defintion of a line style.
//!  @ingroup LineStyleManagerModule
// @bsiclass
//=======================================================================================
struct          LsDefinition
//__PUBLISH_SECTION_END__
                : public ILineStyle
//__PUBLISH_SECTION_START__
    {
//__PUBLISH_SECTION_END__
private:
    bool                m_isDirty;
    bool                m_componentLookupFailed;
    int64_t             m_styleNumber;
    MSCharIKey          m_name;
    LsLocation          m_location;             // Where to find components of resource
    LsComponentPtr      m_lsComp;
    double              m_unitDef;
    double              m_maxWidth;
    uint32_t            m_attributes;
    int                 m_hardwareLineCode;
    double              m_uorsPerMeter;         // Used to True Scale between models when it is units UOR
    bool                m_componentLoadPostProcessed;

    void Init (CharCP nName, Json::Value& lsDefinition, int64_t styleId);
    void SetHWStyle (uint32_t rscType, uint64_t rscID);
    //  m_uorsPerMeter is only set when the line style is loaded from a DGN file.
    //  It is used only to compute the true scale value when units is UOR.
    double              GetUorsPerMeter         () const;
    int                 GetUnits                () const {return m_attributes & LSATTR_UNITMASK;}

public:
    DGNPLATFORM_EXPORT static double GetUnitDef (Json::Value& lsDefinition);
    DGNPLATFORM_EXPORT static uint32_t GetAttributes (Json::Value& lsDefinition);
    DGNPLATFORM_EXPORT static uint32_t GetComponentType (Json::Value& lsDefinition);
    DGNPLATFORM_EXPORT static uint32_t GetComponentId (Json::Value& lsDefinition);

    LsDefinition (Utf8CP name, DgnDbR project, Json::Value& lsDefinition, int64_t styleId);

    virtual ~LsDefinition ();

    virtual Utf8CP _GetName () const override {return m_name.GetValue();}
    virtual ILineStyleComponent const* _GetComponent () const override {return m_lsComp.get ();}
    virtual bool _IsSnappable () const override {return 0 == (m_attributes & LSATTR_NOSNAP);}

    LsComponentP GetLsComponent () const {return m_lsComp.get ();}
    LsLocationCP GetLocation () const {return &m_location;}
    uint32_t GetAttributes () const {return m_attributes;}
    bool IsOfType (uint32_t type) const {return ((uint32_t)m_location.GetRscType() == type ? true : false);}

    //  This method should only be used during remapping. It is assumed that LsDefinition has
    //  not loaded the component and that the previous component ID was invalid.
    DGNPLATFORM_EXPORT void      RemapComponentId (uint32_t newId);

    DGNPLATFORM_EXPORT StatusInt UpdateStyleTable() const;
    DGNPLATFORM_EXPORT StatusInt TransferStyleTableEntry(DgnStyleId& newId, DgnDbR targetProject) const;

    void CheckForContinuous (LsStrokePatternComponentCP);
    void PostProcessComponentLoad (DgnModelP modelRef);
    void ClearPostProcess ();
    void SetName (Utf8CP name);
    void SetAttributes (uint32_t attr) {m_attributes = attr;}
    void SetStyleNumber (int64_t number) { m_styleNumber = number; }
    int64_t GetStyleNumber () { return m_styleNumber; }

    DGNPLATFORM_EXPORT double GetTrueScale (DgnModelP styleDgnModel) const;

    //  There should no reason to provide set methods or to expose this outside of DgnPlatform.
    DGNPLATFORM_EXPORT double _GetMaxWidth () const;
    DGNPLATFORM_EXPORT int GetHardwareStyle () const;
    DGNPLATFORM_EXPORT bool IsNoWidth () const;
    DGNPLATFORM_EXPORT bool IsInternal () const;
    DGNPLATFORM_EXPORT bool IsHardware () const;

    bool IsDirty () { return m_isDirty; }
    void MarkDirty (bool value = true) { m_isDirty = value; }
    StatusInt Commit ();

    static void InitializeJsonObject (Json::Value& jsonObj, uint32_t componentId, uint16_t componentType, uint32_t flags, double unitDefinition);
    void InitializeJsonObject (Json::Value& jsonObj);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    //!  Defines a scaling factor to be applied to the components.
    DGNPLATFORM_EXPORT double GetUnitsDefinition () const;
    //!  Defines a scaling factor to be applied to the components.
    DGNPLATFORM_EXPORT void SetUnitsDefinition (double newValue);
    DGNPLATFORM_EXPORT void SetUnitsType (LsUnit unitsType);
    DGNPLATFORM_EXPORT LsUnit GetUnitsType () const;
    //!  This is equivalent to "LSATTR_UNITUOR == GetUnits()"
    DGNPLATFORM_EXPORT bool IsUnitsUOR () const;
    //!  This is equivalent to "LSATTR_UNITMASTER == GetUnits()"
    DGNPLATFORM_EXPORT bool IsUnitsMaster () const;
    //!  This is equivalent to "LSATTR_UNITDEV == GetUnits()"
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

    //! Returns the style's number.
    //!
    //! For a style definition from a design file, this is the number that is used to translate from the number in an
    //! element's symbology to a line style name. For a style definition from a resource file,
    //! this normally is zero.
    DGNPLATFORM_EXPORT int64_t                  GetStyleNumber          () const;

    DGNPLATFORM_EXPORT DgnStyleId               GetStyleId              () const;

    //!  Setting this to true allows a user to snap to the individual components of a line style. If off,
    //!  a snap operation snaps to the underlying line of a line style
    DGNPLATFORM_EXPORT bool                     IsSnappable             () const;
    //!  Setting this to true allows a user to snap to the individual components of a line style. If off,
    //!  a snap operation snaps to the underlying line of a line style
    DGNPLATFORM_EXPORT void                     SetIsSnappable          (bool newValue);

    DGNPLATFORM_EXPORT bool                     IsContinuous            () const;
    DGNPLATFORM_EXPORT void                     SetIsContinuous         (bool newValue);
    //!  Controls whether the line style scale is independent of the scale of shared cells
    DGNPLATFORM_EXPORT bool                     IsSCScaleIndependent    () const;
    //!  Controls whether the line style scale is independent of the scale of shared cells
    DGNPLATFORM_EXPORT void                     SetIsSCScaleIndependent (bool newValue);
    //!  Gets the LsComponent for the line style.
    //!
    //!  An LsDefinition can only directly reference one LsComponent.  That LsComponent
    //!  may reference other components.
    DGNPLATFORM_EXPORT LsComponentP             GetComponentP           (DgnModelP modelRef) const;
    //!  Gets the LsComponent for the line style.
    //!
    //!  An LsDefinition can only directly reference one LsComponent.  That LsComponent
    //!  may reference other components.
    DGNPLATFORM_EXPORT LsComponentCP            GetComponentCP          (DgnModelP modelRef) const;
    //!  Sets the LsComponent for the line style.
    //!
    //!  An LsDefinition can only directly reference one LsComponent.  That LsComponent
    //!  may reference other components.
    DGNPLATFORM_EXPORT LineStyleStatus          SetComponent            (LsComponentP lsComp);
    //!  Gets the name of the line style
    DGNPLATFORM_EXPORT WString                  GetStyleName            () const;
    //! Retrieves a value indicating whether the line style definition comes from a resource file or DgnFile.
    DGNPLATFORM_EXPORT LsLocationType           GetLocationType         () const;
    };

//__PUBLISH_SECTION_END__
//=======================================================================================
// @bsiclass
//=======================================================================================
struct  LsIdNode
    {
    friend struct LsMapEntry;
private:
    int64_t             m_id;
    Utf8P               m_name;
    LsDefinitionP       m_nameRec;
    bool                m_resolves;

public:
    LsIdNode (int64_t id, Utf8CP name, LsDefinitionP nameRec, bool resolves);

    bool Resolves () const { return m_resolves; }
    void Clear ();
    int64_t GetKey () const {return m_id;}
    LsDefinitionP GetValue () const {return m_nameRec;}
    Utf8CP GetName () const {return m_name;}
    void SetValue (LsDefinitionP v) {m_nameRec = v;}
    void SetName (Utf8CP newName);

    static int64_t GetMinKey () {return INT32_MIN;}
    static int64_t GetMaxKey () {return INT32_MAX;}
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct NameNode
{
    LsDefinitionP       m_entry;

public:
    NameNode (LsDefinition* entry) {m_entry = entry;}

    LsDefinitionP      GetValue     () const {return m_entry;}
    MSCharIKey        GetKey        () const   {return m_entry->_GetName();}

    static MSCharIKey GetMinKey    () {return MINCKEY;}
    static MSCharIKey GetMaxKey    () {return MAXCKEY;}
};


//__PUBLISH_SECTION_START__
//=======================================================================================
//! An entry in a LsMap; it contains a pointer to an LsDefinition
//!
//! If the LsMapEntry is in the system map then the LsDefinition comes from the same file as the map entry.
//! If the LsMapEntry is from an LsDgnFile map then the LsDefinition is from a DgnFile
//! if the line style definition was copied to the design file. If it was not copied to the design
//! file then the LsDefinition is from a resource file.  LsDefinitionCP can be NULL when a map
//! contains an LsMapEntry that refers to a line style that cannot be found.
//! @ingroup LineStyleManagerModule
// @bsiclass
//=======================================================================================
struct          LsMapEntry
//__PUBLISH_SECTION_END__
                : public LsIdNode
//__PUBLISH_SECTION_START__
{
private:
    LsMapEntry ();

public:
    //!  Gets the name of the line style
    DGNPLATFORM_EXPORT Utf8CP GetStyleName () const;
    //! Get the line style number. For an LsMapEntry from an LsDgnFileMap this is the number
    //! that is used to translate from the number in an element's symbology to a line style name.
    //! For an LsMapEntry from a system map, the style number normally is meaningless.
    DGNPLATFORM_EXPORT int64_t GetStyleNumber () const;
    //! Get the LsDefinition for the line style associated with the LsMapEntry.
    DGNPLATFORM_EXPORT LsDefinitionP GetLineStyleP () const;
    //! Get the LsDefinition for the line style associated with the LsMapEntry.
    //! @See LsMapEntry::GetLineStyleP
    DGNPLATFORM_EXPORT LsDefinitionCP GetLineStyleCP () const;
};

//=======================================================================================
//! Helper class used for iterating through the LsMapEntry entries in an LsMap.
//!@code
//!   for each (LsMapEntryCR  ls in *systemLsMap)
//!       {
//!       Int32           styleNo = ls.GetStyleNumber ();
//!       WString         name = ls.GetStyleName ();
//!       }
//!@endcode
//!  @ingroup LineStyleManagerModule
//!  @bsiclass
//=======================================================================================
struct          LsMapIterator : std::iterator<std::forward_iterator_tag, const LsMapEntry>
    {
private:
    friend struct LsMap;
    LsMapIterator (LsMapCP map, bool wantBegin);    // ctors are private. Therefore, callers cannot construct an instance. Since we don't define a private copy constructor, callers can make copies. That should be safe, since we don't define a destructor.
    LsMapIterator () {}
private:
    int                 m_index;
    void const*         m_node;
    intptr_t            m_auxData;

public:
    DGNPLATFORM_EXPORT LsMapIteratorR  operator++();
    DGNPLATFORM_EXPORT LsMapEntryCR    operator*() const;
                   LsMapEntryCP    operator->() const {return &operator*();}
    DGNPLATFORM_EXPORT bool            operator==(LsMapIteratorCR rhs) const;
    DGNPLATFORM_EXPORT bool            operator!=(LsMapIteratorCR rhs) const;
    };

//=======================================================================================
//! A map used to map from line style number to line style name and vice versa.  There is a LsDgnFileMap
//! for each DgnFile and a system map.
//!
//! To resolve a line style for an element, the element's line style number is used
//! to search the LsDgnFileMap of the DgnDb that holds the element. If that yields an
//! LsMapEntry, the style name is found from the entry and uses that name to
//! search the LsSystemMap.
//!  @ingroup LineStyleManagerModule
// @bsiclass
//=======================================================================================
struct LsMap : public RefCountedBase
    {
//__PUBLISH_SECTION_END__
    struct Maps_T : bvector <LsMapP>, DgnHost::HostObjectBase {;};
    typedef Maps_T::iterator        MapsIter_T;

    static Maps_T*                  AllMaps ();

protected:
    T_LsIdTree          m_idTree;
    T_LsNameTree        m_nameTree;
    bool                m_isLoaded;

    void                                    EmptyIdMap      ();   // EmptyIdMap and EmptyNameMap should probably always be called together.
    void                                    EmptyNameMap    ();  // EmptyIdMap and EmptyNameMap should probably always be called together.

    virtual LsLocationType                  _GetType        () = 0;
    virtual WString                         _GetFileName    () = 0;
    //  virtual LineStyleComponentP     _FindComponent (ComponentID& componentID, bool searchSystemMaps) const;

public:
    DEFINE_BENTLEY_NEW_DELETE_OPERATORS 

    typedef int (*PFNameMapProcessFunc) (NameNode const* nameNode, void* arg);

    LsMap();
    virtual ~LsMap();

    DGNPLATFORM_EXPORT LsIdNodeP               FindId                   (int64_t styleId) const;
    DGNPLATFORM_EXPORT void                    EmptyMaps                ();
    T_LsIdIterator                         FirstId                  () {return m_idTree.FirstEntry();}
    void                                   AddIdEntry               (int64_t id, DgnDbR project, Utf8CP name, bool resolves);
    void                                   AddIdEntry               (int64_t id, LsDefinitionP nameRec, bool resolves);
    DGNPLATFORM_EXPORT BentleyStatus           RemoveIdEntry            (int64_t id);
    DGNPLATFORM_EXPORT BentleyStatus           RemoveNameEntry          (Utf8CP);
    DGNPLATFORM_EXPORT LsDefinitionP           AddNameEntry             (Utf8CP, int64_t styleId, DgnDbR, Json::Value& lsDefinition);
    DGNPLATFORM_EXPORT void                    AddNameEntry             (LsDefinition*);
    DGNPLATFORM_EXPORT void                    ProcessNameMap           (PFNameMapProcessFunc processFunc, void* arg);
    T_LsIdTree*                            GetIdMap                 () {return &m_idTree;}
    DGNPLATFORM_EXPORT LsIdNodeP               SearchIdsForName         (Utf8CP name) const;
    DGNPLATFORM_EXPORT void                    RenameStyle              (Utf8CP  oldName, Utf8CP  newName, LsLocationType type);
    bool                                   IsLoaded                 () const {return m_isLoaded;}
    void                                   TreeLoaded               () { m_isLoaded=true; }

    DGNPLATFORM_EXPORT static LsDefinitionP     FindInRefOrRsc          (DgnDbP project, Utf8CP name);
    DGNPLATFORM_EXPORT static LsDefinitionP     FindInRefOrRsc          (DgnDbP project, int64_t styleId);
    DGNPLATFORM_EXPORT static LsDefinitionP     FindInRef               (DgnDbP project, int64_t styleId);
    DGNPLATFORM_EXPORT static LsDgnProjectMapP  GetMapPtr               (DgnDbR, bool loadIfNotLoaded = true);
    DGNPLATFORM_EXPORT static void              DropAllComponentRefs    ();
    DGNPLATFORM_EXPORT static void              EmptyAllMaps            ();

    //  These should be replaced by the Get methods
    DGNPLATFORM_EXPORT LsDefinitionP            Find                    (Utf8CP lsName) const;
    DGNPLATFORM_EXPORT LsDefinitionP            Find                    (int64_t styleId) const;

    //  DGNPLATFORM_EXPORT LineStyleComponentP     Copy (LineStyleComponentP source);
    //  DGNPLATFORM_EXPORT LsDefinitionP           CreateLineStyle (WStringCR name, LineStyleComponentP component, UInt32 options, Int32 lineStyleID);
//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    //! Gets the line style map associated with the specified project.
    //! @return a A pointer to the map if it is loaded or if not loaded but loadIfNotLoaded is true; NULL otherwise
    DGNPLATFORM_EXPORT static LsDgnProjectMapP  GetProjectMap           (DgnDbR, bool loadIfNotLoaded = true);

    DGNPLATFORM_EXPORT LsLocationType           GetType                 () const;    //!< Type of file used to load the map.
    DGNPLATFORM_EXPORT WString                  GetFileName             () const;    //!< Name of file used to load the map.

    //! Searches the set of names associated with the LsMap.
    //! @param[in] lsName The name of the line style.
    //! @return The associated LsDefinition object if found. Otherwise, it returns NULL;
    //! @note If the line style map is a LsDgnFileMap then the LsDefinitionP that is
    //! returned may point to a line style defined in a DGN file or a line style defined
    //! in a resource file.
    DGNPLATFORM_EXPORT LsDefinitionP            GetLineStyleP   (Utf8CP lsName) const;
    //! Searches the set of names associated with the LsMap.
    //! @param[in] lsName The name of the line style.
    //! @return The associated LsDefinition object if found. Otherwise, it returns NULL;
    //! @see LsMap::GetLineStyleP
    DGNPLATFORM_EXPORT LsDefinitionCP           GetLineStyleCP  (Utf8CP lsName) const;

    //! Searches the set of ID's associated with the LsMap.
    //! @param[in] styleId The ID that is used to associate an element with a line style.
    //! @return The associated LsDefinition object if found. Otherwise, it returns NULL. It
    //! also returns NULL for all of the special codes including STYLE_BYLEVEL, STYLE_BYCELL, and the
    //! reserved line codes 0 through 7.
    //! @note If the line style map is a LsDgnFileMap then the LsDefinitionP that is
    //! returned may be a pointer to a stub used to translate from element style
    //! number to name.  Call LsDgnFileMap::Resolve to find the LsDefinition that
    //! is used to stroke the element.
    DGNPLATFORM_EXPORT LsDefinitionP            GetLineStyleP   (int64_t styleId) const;
    //! Searches the set of ID's associated with the LsMap.
    //! @param[in] styleId The ID that is used to associate an element with a line style.
    //! @return The associated LsDefinition object if found. Otherwise, it returns NULL. It
    //! also returns NULL for all of the special codes including STYLE_BYLEVEL, STYLE_BYCELL, and the
    //! reserved line codes 0 through 7.
    //! @see LsMap::GetLineStyleP
    DGNPLATFORM_EXPORT LsDefinitionCP           GetLineStyleCP  (int64_t styleId) const;

    typedef LsMapIterator const_iterator;
    typedef const_iterator iterator;    //!< only const iteration is possible

    //! Used to step through all of the LsMapEntry entries for an LsMap. See
    //! LsMapIterator for an example.
    DGNPLATFORM_EXPORT LsMapIterator            begin () const;
    DGNPLATFORM_EXPORT LsMapIterator            end   () const;
    };

//=======================================================================================
//! Used to access the line styles that are in the system map. Typically these are loaded
//! from resource files.
//! @ingroup LineStyleManagerModule
// @bsiclass
//=======================================================================================
struct          LsSystemMap : public LsMap
//__PUBLISH_SECTION_END__
                                    , DgnHost::HostObjectBase
//__PUBLISH_SECTION_START__
{
//__PUBLISH_SECTION_END__

    using LsMap::operator new;
    using LsMap::operator delete;

protected:
    virtual LsLocationType  _GetType        () override { return LsLocationType::System; }
    virtual WString         _GetFileName    () override { return WString (L""); }

public:

    DGNPLATFORM_EXPORT LsDefinitionP            FindSystemName          (Utf8CP) const;
    DGNPLATFORM_EXPORT LsDefinitionP            FindNameInLibs          (Utf8CP  name) const;
    DGNPLATFORM_EXPORT void                     ProcessSystemNameMaps   (PFNameMapProcessFunc processFunc, void* arg, bool includeDgnLibs);
    void                                    AddSystemIdEntry        (int64_t id, Utf8CP name, bool resolves);

    DGNPLATFORM_EXPORT static LsSystemMapP      GetSystemMapPtr         ();
    DGNPLATFORM_EXPORT static LsSystemMapP      GetSystemMapP           (bool requiresLoadedMap);
    DGNPLATFORM_EXPORT static void              EmptySystemMap          ();
    LsSystemMap () : LsMap () {}

//__PUBLISH_SECTION_START__
public:
    //!  Gets a reference to the system LsMap.
    DGNPLATFORM_EXPORT static LsSystemMapR      GetSystemMapR           ();
    //!  Gets a reference to the system LsMap.
    DGNPLATFORM_EXPORT static LsSystemMapCR     GetSystemMap            ();
};

//=======================================================================================
//! Used to access the line styles known to a given DgnDb.
//! @ingroup LineStyleManagerModule
// @bsiclass
//=======================================================================================
struct  LsDgnProjectMap : public LsMap
{
//__PUBLISH_SECTION_END__
private:
    DgnDbR       m_dgnProject;
    LsDgnProjectMap(DgnDbR dgnProject) : m_dgnProject(dgnProject) {}
    ~LsDgnProjectMap();

protected:
    virtual LsLocationType          _GetType        () override { return LsLocationType::DgnDb; }
    virtual WString                 _GetFileName    () override
        {
        return L""; // m_msDgnFile->GetFileName();
        }

public:
    BentleyStatus                   Load            ();

    static LsDgnProjectMapPtr Create (DgnDbR project);

//__PUBLISH_SECTION_START__
public:
    //!  Gets a pointer to the associated DgnFile.
    //!  @remark Use LineStyleManager::GetLineStyleMapP to get a pointer to a DgnFile's LsDgnFileMap.
    DGNPLATFORM_EXPORT DgnDbR      GetDgnProjectR  () const;
    //!  Gets a pointer to the associated DgnFile.
    //!  @remark Use LineStyleManager::GetLineStyleMapP to get a pointer to a DgnFile's LsDgnFileMap.
    DGNPLATFORM_EXPORT DgnDbCR     GetDgnDb   () const;
};

//=======================================================================================
//! Groups methods for Line Styles.
// @bsiclass
//=======================================================================================
struct DgnLineStyles : public DgnDbTable
{
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(DgnDbTable);
    friend struct DgnStyles;

    LsDgnProjectMap* m_lineStyleMap;

    //! Only the outer class is designed to construct this class.
    DgnLineStyles(DgnDbR db) : T_Super(db), m_lineStyleMap(nullptr) {}
    ~DgnLineStyles();

public:
//__PUBLISH_SECTION_START__
    //! Adds a new line style to the project. If a style already exists by-name, no action is performed.
    DGNPLATFORM_EXPORT BentleyStatus Insert(DgnStyleId& newStyleId, Utf8CP name, uint32_t componentId, uint16_t componentType, uint32_t flags, double unitDefinition);

    //! Updates an a Line Style in the styles table..
    DGNPLATFORM_EXPORT BentleyStatus Update(DgnStyleId styleId, Utf8CP name, uint32_t componentId, uint16_t componentType, uint32_t flags, double unitDefinition);

    DGNPLATFORM_EXPORT LsDgnProjectMapP GetMapP (bool load=true);
    DGNPLATFORM_EXPORT LsDgnProjectMapR ReloadMap();
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
