/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/LsLocal.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include    "../Tools/KeyTree.h"
#include    "RmgrTools/Tools/msavltre.h"
#include    "ViewContext.h"
#include    "LineStyle.h"
#include    "RmgrTools/Tools/msstrlst.h"
#include    "lstyle.h"
#include    "../DgnPlatformBaseType.r.h"

#define LCCAP_MAXVECS       90

typedef uint32_t RscFileHandle;
struct dwgLineStyleInfo;   // this is outside the linestyle namespace.

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                      Keith.Bentley   01/03
//=======================================================================================
struct  LineJoint
{
    DPoint3d    m_dir;          // Unit bvector in direction of joint
    double      m_scale;        // Ratio of joint length to line offset

public:
    // Calculates an ARRAY of LineJoints from the input array of points.
    static void  FromVertices (LineJoint*, DPoint3dCP points, int nPoints, DPoint3dCP normal, DPoint3dCP pStartTangent, DPoint3dCP pEndTangent);
    };

struct AddComponentsToDefElm        // For traversing the avltree and adding to elmdscr.
    {
    DgnElementP pDefElm;
    DgnModelP    modelRef;
    AvlTree*        rscElmMap;
    LsMap*          fileNameMap;    // Ones already in the file, to avoid duplication.
    bool            convertToUORS;
    double          scale;
};


//=======================================================================================
// @bsiclass                                                      Keith.Bentley   02/03
//=======================================================================================
struct          Centerline
{
private:
    bool                m_taper;
    bool                m_hasWidth;
    int                 m_count;
    double              m_lastLen;
    DPoint3dP            m_pts;
    double*             m_widths;
    double*             m_lengths;
    DPoint3dCP          m_segmentDirection;

public:
    Centerline (DPoint3dP  pts, double* widths, double* lengths, bool tapered)
            {
            m_taper     = tapered;
            m_hasWidth  = false;
            m_count     = 0;
            m_lastLen   = 0.0;
            m_pts       = pts;
            m_widths    = widths;
            m_lengths   = lengths;
            }

        void                Empty       () {m_count = 0;}
        int                 GetCount    ()          const  {return m_count;}
        DPoint3dCP          GetPointAt  (int index) const  {return ((index <0) || (index >= m_count)) ? NULL : &m_pts[index];}
        double const*       GetWidthAt  (int index) const  {return ((index <0) || (index >= m_count)) ? NULL : &m_widths[index];}
        double const*       GetLengthAt (int index) const  {return ((index <0) || (index >= m_count)) ? NULL : &m_lengths[index];}
        void                SetDirection (DPoint3dCP dir)  {m_segmentDirection = dir;}
        DPoint3dCP          GetDirection () const  {return m_segmentDirection;}
        void                GetDirectionVector (DPoint3dR segDir, DPoint3dCR org, DPoint3dCR end) const;

    void    AddPoint    (DPoint3dCP pt, double width, double length);
    void    Output      (ViewContextP, LsStrokeP, DPoint3dCP normal, DPoint3dCP startTangent, DPoint3dCP endTangent);
};


//=======================================================================================
// @bsiclass                                                       John.Gooding    06/09
//=======================================================================================
class           LineStyleCacheManager
{
public:
static                LsComponentP  GetSubComponent         (LsLocationCP location, DgnDbR dgnProject);
static                void          CacheAdd                (LsComponent* comp);
static DGNPLATFORM_EXPORT void          CacheFree               ();
static DGNPLATFORM_EXPORT void          CacheDelete             (LsLocation const* searchLocation,int option);
static DGNPLATFORM_EXPORT void          CacheDeleteComponent    (LsComponent& compareComponent, int option);
static DGNPLATFORM_EXPORT void          CacheDelete             (uint32_t fileKey, uint32_t rscType, uint32_t elementID, int option);
static DGNPLATFORM_EXPORT void          CacheDelete             (DgnDbP dgnFile, long lsType, DgnElementId elementID, int option);
static DGNPLATFORM_EXPORT BentleyStatus CacheInsert             (RscFileHandle rscFile, long rscType, uint32_t rscId, DgnDbP dgnFile, void* pRsc, long option);
static DGNPLATFORM_EXPORT BentleyStatus CacheInsert             (DgnDbP dgnFile, long compType, DgnElementId compID, void* pRsc, long option);
static DGNPLATFORM_EXPORT void          FreeDgnFileMaps         ();
}; // LineStyleCacheManager

//=======================================================================================
// @bsiclass                                                      John.Gooding    10/09
//=======================================================================================
struct          ILineStyleDgnLibIterator
{
virtual size_t     _Begin       ()    = 0;
virtual DgnDbP   _MoveNext    (size_t handle) = 0;
virtual void       _End         (size_t handle) = 0;
}; // ILineStyleDgnLibIterator

#define LINKS_START(el)         (DgnElementId *)((el).lineStyleDefEntry.data + (el).lineStyleDefEntry.dataSize)
#define IS_LINECODE(styleNo)    ((styleNo) >= MIN_LINECODE && (styleNo) <= MAX_LINECODE)

END_BENTLEY_DGNPLATFORM_NAMESPACE

BEGIN_BENTLEY_NAMESPACE

//  Line style definition element functions
DGNPLATFORM_EXPORT int          lstyleElm_rscToElm
(
Dgn::DgnElementP*           ppDefElm,   // <= Definition Element; returned with a unique ID set. May have attached point symbol element.
void*                       pRsc,       // => The resource structure to use.
int                         rscType,    // => Type of resource.
uint32_t const*             rscId,      // => ID of resource; NULL if unknown.
WCharCP                     name,       // => Name of style - should match name in map.
Dgn::DgnModelP              modelRef,   // => modelRef these will be written to - for creating unique ID's.
Dgn::DgnElementId           inputID,    // => Input element ID (or 0 for next highest).
RscFileHandle               rscFile,    // => Only needed if extracting recursively from resource file.
Dgn::AddComponentsToDefElm* info        // => Only needed if extracting recursively from resource file.
);

DGNPLATFORM_EXPORT StatusInt    LineStyle_elementToRsc
(
void**              ppRsc,              // <=
uint32_t*           rscType,            // <=
Dgn::DgnElementCP   pElm,               // =>
Dgn::DgnModelP      modelRef,
bool                asV7Element,        // => true if point symbol should be V7 element.
bool                forResourceFile     // => true if symbols are to be written to a resource file; ignored if V7 is true.
);

DGNPLATFORM_EXPORT void         LineStyle_freeRsc
(
void            **ppRsc                 // <=
);

//  Name map functions
DGNPLATFORM_EXPORT int          lineStyle_nameInsert
(
Utf8CP              styleName,          // => New style name for name map.
uint32_t            rscHandle,          // => Resource file handle.
uint32_t            rscType,            // => UDLS resource type.
Dgn::DgnElementId   rscID,              // => UDLS resource ID.
uint32_t            nameAttributes,     // => Name attributes.
long                id,                 // => Pass 0 for generated seed id.
uint32_t            option,             // => Function options.
Dgn::DgnModelP      modelRef
);

DGNPLATFORM_EXPORT StatusInt    LineStyle_nameDeleteEx
(
WCharCP       pStyleName,
uint32_t        rscFile,
uint32_t        rscType,
long            option
);

DGNPLATFORM_EXPORT StatusInt    LineStyle_nameDelete (WCharCP pStyleName, long option);

DGNPLATFORM_EXPORT  void        LineStyle_setDgnLibIterator
(
Dgn::ILineStyleDgnLibIterator* iter
);

#if defined (NEEDS_WORK_DGNITEM)
//  Element/symbology functions
//  Misc utility
DGNPLATFORM_EXPORT BentleyStatus LineStyle_setElementStyle
(
EditElementHandleR  eeh,
int32_t             pStyleNo,
LineStyleParamsP    pParams
);

DGNPLATFORM_EXPORT bool elemUtil_supportsLineStyle  (ElementHandleR eh);
#endif

StatusInt createDefElement
(
Dgn::DgnElementP* ppElm,          // <=
void*           data,           // =>
size_t          dataSize,       // =>
size_t          dependentCount, // =>
WCharCP         name,
uint16_t        type           // => Type to set element to.
);

DGNPLATFORM_EXPORT  StatusInt   lineStyle_nameDelete
(
WCharCP         styleName,
Dgn::LsMapP     lsMap,
long            option,
Dgn::DgnDbP     dgnFile
);

DGNPLATFORM_EXPORT double lsutil_getMasterToUorConversionFactor (Dgn::DgnModelP destDgnModel);

// Used when deleting definitions
struct UsedElementCount
    {
    Dgn::DgnElementId  uniqueID;
    int                useCt;
    bool               inNameMap;
    bool               toBeDeleted;
    } ;

typedef bmap<Dgn::DgnElementId, UsedElementCount> LineStyleDefUseMap;

DGNPLATFORM_EXPORT BentleyStatus   lstyleElm_getAllDefsUsed (LineStyleDefUseMap& defUseMap, Dgn::DgnDbP dgnFile);
DGNPLATFORM_EXPORT BentleyStatus   lstyleElm_markDeleteDefinition (Dgn::DgnModelP modelRef, Dgn::DgnElementId elId, LineStyleDefUseMap& pDefDependMap);

DGNPLATFORM_EXPORT void lineStyle_changeAllElementIdsForDwg
(
bool*               madeChanges,
Dgn::DgnElementP    pDefinitionTable,
Dgn::DgnElementP    pNameTable,
Dgn::DgnElementId   minID,
Dgn::DgnElementId   maxID,
Dgn::DgnDbP         pFile
);

//  Non-exported functions
#if defined (__DGNPLATFORM_BUILD__)

int32_t  lineStyle_addStyle
(
WCharCP         name,           // => Line style name.
Dgn::DgnModelP  modelRef,       // => Model ref.
long            seedID
);

int32_t  lineStyle_getStyleID
(
WCharCP         name,           // => Line style name.
Dgn::DgnModelP  modelRef,       // => Model ref.
int             create          // => true=create ID entry if necessary.
);

int addDependency
(
Dgn::DgnElementP    pElm,       // =>
Dgn::DgnElementId   uniqueID    // => ID to add.
);

//  Internal utilities
Dgn::LsElementType   remapRscTypeToElmType (uint32_t rscType);
uint32_t                     remapElmTypeToRscType (Dgn::LsElementType  elmType);

bool   mdlLineStyle_typeIsElement  (uint32_t rscType);

Dgn::DgnElementId       lstyleElm_getDependency
(
Dgn::GeometricElementCP pElm,   // =>
int                     index   // => ID desired.
);

#endif // defined (__DGNPLATFORM_BUILD__)

END_BENTLEY_NAMESPACE
