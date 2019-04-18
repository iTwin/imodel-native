/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
#include <Bentley/Bentley.h>
#define BEGIN_BENTLEY_GEOMETRY_INTERNAL_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace Geom {
#define END_BENTLEY_GEOMETRY_INTERNAL_NAMESPACE          } END_BENTLEY_NAMESPACE
#define BENTLEY_GEOMETRY_INTERNAL_NAMESPACE_NAME         BentleyApi::Geom
#define USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL        using namespace BENTLEY_GEOMETRY_INTERNAL_NAMESPACE_NAME;
#define GEOMAPI_PRINTF printf
#define BENTLEYGEOMETRY_ClipPlaneCoordinatesPublic
// make initial appearance of virtual signature unambiguous
#define GEOMAPI_VIRTUAL virtual
/**
* @addtogroup GROUP_Geometry Geometry Module
* Types related to working with geometry
*/

#ifdef jmdlgeom_internal
#include <Bentley/BeConsole.h>
#endif

#include <float.h>
#include <math.h>
#include <string.h>
#include <Bentley/bvector.h>
#include <algorithm>
#include "msgeomstructs_typedefs.h"
#include "GeomApi.r.h"
#include <Bentley/RefCounted.h>
#include <Bentley/NonCopyableClass.h>
#include <Bentley/ScopedArray.h>
#include <Geom/IntegerTypes/Point.h>
#include <Geom/IntegerTypes/BSIRect.h>
#include <Geom/counters.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*__PUBLISH_SECTION_END__*/
#define NAME(cName,packageName,methodName) cName
#define IARG
#define CHANGE_TO_BOOL(_FunctionCall_) _FunctionCall_

/*__PUBLISH_SECTION_START__*/






/*----------------------------------------------------------------------+
|                                                                       |
|   Constants                                                           |
|                                                                       |
+----------------------------------------------------------------------*/
#ifndef msGeomConst_pi
#define msGeomConst_piOver4     7.85398163397448280000e-001
#define msGeomConst_piOver2     1.57079632679489660000e+000
#define msGeomConst_pi          3.14159265358979310000e+000
#define msGeomConst_2pi         6.28318530717958620000e+000
#define msGeomConst_degreesPerRadian    (45.0 / msGeomConst_piOver4)
#define msGeomConst_radiansPerDegree    (msGeomConst_piOver4 / 45.0)
#endif
#define msGeomConst_piOver12    0.26179938779914943653855361527329

#ifndef PI
#define PI      3.1415926535897932384626433
#endif

/* Constants from DGN / MGDS code */
#define mgds_fc_iang_to_rad   (msGeomConst_degreesPerRadian/360000.0)
#define mgds_fc_rad_to_iang   (msGeomConst_radiansPerDegree*360000.0)
#define mgds_fc_miang_to_rad  (-1.0*msGeomConst_degreesPerRadian/360000.0)
#define mgds_fc_nearZero       1.0e-14     /* verging on zero - from mgds days*/
#define mgds_fc_condition      1.0e-12     /* matrix condition number - from mgds days */
#define mgds_fc_epsilon        0.00001     /* A reasonable epsilon for DGN / MGDS */

//! @description  DISCONNECT is a coordinate value that signifies a disconnected
//!     vertex in a string of points.  These vertices can occur in
//!     linestrings or reference clip boundaries.
//!     The value is the largest value that can be stored in an IEEE double
#define DISCONNECT  (1.7976931348623157e308)

//! static class for rounding methods.
struct Rounding
{
private:
    //! This is a static class - no instances.
    Rounding ();
public:
    //! Enumeration of rounding options
    enum RoundingMode
        {
        RoundingMode_Round = 0,
        RoundingMode_Up    = 1,
        RoundingMode_Down  = 2
        };
    //! Apply a rounding mode to a value.
    //! @param [in] value value to adjust
    //! @param [in] mode selects mode.
    //! @param [in] lowerValue lower limit.
    //! @param [in] upperValue upper limit.
    //! Remark: If lowerValue and upperValue are in reverse order, they are swapped. 
  static GEOMDLLIMPEXP double Round (double value, RoundingMode mode, double lowerValue, double upperValue);
};

END_BENTLEY_GEOMETRY_NAMESPACE
//__PUBLISH_SECTION_END__

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#if !defined(MAX)
#   define MAX(a,b)                 ((a)>(b)?(a):(b))
#endif

#if !defined(MIN)
#   define MIN(a,b)                 ((a)<(b)?(a):(b))
#endif

#if !defined(BOUND)
#define BOUND(x,min,max)            ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))
#endif

#if !defined(LIMIT_RANGE)
#define LIMIT_RANGE(min,max,val)    {if ((val) < (min)) val = min; else if ((val) > (max)) val = max;}
#endif

#if !defined(IN_RANGE)
#define IN_RANGE(x,min,max)         (((x) >= (min)) && ((x) <= (max)))
#endif

/*--------------------------------------------------------------------------+
|   Output functions and context data for HPoints data are packaged in      |
|   an HPointsDrawFunctions structure.                                      |
|                                                                           |
| The structure has 2 user data pointers.  When using mdlWindow_xxx for     |
| output, these have the following interpretation:                          |
| pUserData1 is the mdl window pointer.                                     |
| pUserData2 is the clip rectangle pointer.                                 |
+--------------------------------------------------------------------------*/
typedef struct
    {
    void (*pLineStringDrawFunction)
        (
        void            *pWindow,
        void*           pPointArray,
        int             numPoints,
        void            *pClipRect
        );
    void (*pPointDrawFunction)
        (
        void            *pWindow,
        int             xCoord,
        int             yCoord,
        void            *pClipRect
        );
    void (*pLineStyleSetFunction)
        (
        void            *windowP,
        int             pattern,
        int             color,
        int             mode,
        int             lweight
        );
    int (*pFixedColorIndexGetFunction)
        (
        void            *windowP,
        int             menuIndex
        );

    void                                *pUserData1;
    void                                *pUserData2;
    } HPointsDrawFunctions;

typedef double (*OmdlScalarFunction)
(
void   *pUserData,      /* => user data pointer */
double arg              /* => parameter value where function is to be evaluated */
);

#define     HCONIC_Null         0
#define     HCONIC_Point        1
#define     HCONIC_Line         2
#define     HCONIC_LinePair     3
#define     HCONIC_Ellipse      4
typedef struct HConic HConic;
typedef enum
    {
    HConic_Null         = HCONIC_Null,
    HConic_Point        = HCONIC_Point,     /* coordiantes.center is the point */
    HConic_Line         = HCONIC_Line,      /* alpha0*vector0 + alpha90*vector90 */
    HConic_LinePair     = HCONIC_LinePair,  /* center + alpha0 * vector0 + alpha90 * vector90 */
    HConic_Ellipse      = HCONIC_Ellipse    /* center + cos(theta) * vector0 + sin(theta) * vector90 */
    } HConicType;

typedef DMap4d HMap;
typedef HMap *HMapP;
typedef struct rotatedConic RotatedConic;
#define RangePlaneIndexedMaskCount (6)
#define RangePlaneIndexedMask(_index) ((RangePlaneMask)(0x01 << _index))

#define RangePlane_CubePlanes   \
        (                \
        RangePlane_XMin |\
        RangePlane_XMax |\
        RangePlane_YMin |\
        RangePlane_YMax |\
        RangePlane_ZMin |\
        RangePlane_ZMax  \
        )

#define RangePlane_XYPlanes   \
        (                \
        RangePlane_XMin |\
        RangePlane_XMax |\
        RangePlane_YMin |\
        RangePlane_YMax  \
        )

#define RangePlane_xxyyz0Planes   \
        (                \
        RangePlane_XMin |\
        RangePlane_XMax |\
        RangePlane_YMin |\
        RangePlane_YMax |\
        RangePlane_ZMin  \
        )

/*------------------------------------------------------------------+
| 1D interval merging types                                         |
+------------------------------------------------------------------*/
#define ___IntervalLeftLimit    -3
#define ___IntervalOpenRight    -2
#define ___IntervalClosedLeft   -1
#define ___IntervalAtPoint       0
#define ___IntervalClosedRight   1
#define ___IntervalOpenLeft      2
#define ___IntervalRightLimit    3

typedef enum
    {
    IntervalLeftLimit       = ___IntervalLeftLimit,
    IntervalOpenRight       = ___IntervalOpenRight,
    IntervalClosedLeft      = ___IntervalClosedLeft,
    IntervalAtPoint         = ___IntervalAtPoint,
    IntervalClosedRight     = ___IntervalClosedRight,
    IntervalOpenLeft        = ___IntervalOpenLeft,
    IntervalRightLimit      = ___IntervalRightLimit
    } IntervalBoundaryCode;

typedef bool    (*IntervalTestFunction) (int depth);

typedef struct
    {
    double x;
    int     depth;
    IntervalBoundaryCode     typeCode;
    } IntervalBreakPoint;

typedef struct _Varray_BreakPoints *PVArray_BreakPoints;

typedef struct
    {
    int iData;
    void *pData;
    } IntPtrPair;

typedef bool    (*PFScalarIntegrand) (double*pF, double x, void *pContext);
typedef bool    (*PFVectorIntegrand) (double*pF, double x, void *pContext, int numFunc);
typedef void (*PFExtendScalarIntegration)(double, double*, double *, double*, int, void *);



END_BENTLEY_GEOMETRY_NAMESPACE
/*__PUBLISH_SECTION_START__*/

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
struct TaggedPolygon;
typedef bvector <TaggedPolygon> TaggedPolygonVector;
typedef TaggedPolygonVector &TaggedPolygonVectorR;
typedef TaggedPolygonVector const &TaggedPolygonVectorCR;
typedef TaggedPolygonVector *TaggedPolygonVectorP;
typedef TaggedPolygonVector const *TaggedPolygonVectorCP;

struct IFacetOptions;
struct ISolidPrimitive;
struct ICurvePrimitive;
struct CurveVector;
struct CurveVectorWithDistanceIndex;
struct CurveVectorWithXIndex;


DEFINE_REF_COUNTED_PTR(IFacetOptions);
DEFINE_REF_COUNTED_PTR(ISolidPrimitive);
DEFINE_REF_COUNTED_PTR(ICurvePrimitive);
DEFINE_REF_COUNTED_PTR(CurveVector);
DEFINE_REF_COUNTED_PTR(CurveVectorWithDistanceIndex);
DEFINE_REF_COUNTED_PTR(CurveVectorWithXIndex);

struct RefCountedMSBsplineCurve;
typedef RefCountedPtr <RefCountedMSBsplineCurve> MSBsplineCurvePtr;

struct RefCountedMSBsplineSurface;
typedef RefCountedPtr <RefCountedMSBsplineSurface> MSBsplineSurfacePtr;

typedef RefCountedPtr<struct PolyfaceHeader> PolyfaceHeaderPtr;

typedef RefCountedPtr<IGeometry> IGeometryPtr;


//=======================================================================================
//! Inlinable template for initial zeroing an object of size T.
//! @bsiclass 
//=======================================================================================
template<typename T> struct ZeroInit
{
    ZeroInit()                           { memset(this, 0, sizeof(T)); }
    ZeroInit(ZeroInit const&)            { memset(this, 0, sizeof(T)); }
    ZeroInit(ZeroInit&&)                 { memset(this, 0, sizeof(T)); }
    ZeroInit& operator=(ZeroInit const&) = default;
    ZeroInit& operator=(ZeroInit&&)      = default;
};
//! POD struct for pair of size_t values
struct SizeSize
{
size_t dataA;
size_t dataB;
SizeSize (size_t a, size_t b) : dataA(a), dataB (b) {}
};
//=======================================================================================
//! Inlinable template for a value of type T with two size_t tags named TagA and TagB.
//! @bsiclass 
//=======================================================================================
template  <typename T>
struct ValueSizeSize
{
protected:
T m_value;
size_t m_tagA;
size_t m_tagB;

public:

ValueSizeSize (){}
ValueSizeSize (T const & value, size_t tagA, size_t tagB) :
    m_value (value),
    m_tagA  (tagA),
    m_tagB  (tagB)
    {
    }
//! Install TagA value
size_t GetTagA () const {return m_tagA;}
//! Install TagB value
size_t GetTagB () const {return m_tagB;}

//! Get selected tag ..
size_t GetTag (bool tagA) const {return tagA ? m_tagA : m_tagB;}
//! Replace TagA
void SetTagA (ptrdiff_t tag) {m_tagA = tag;}
//! Replace TagB
void SetTagB (ptrdiff_t tag) {m_tagB = tag;}



//! Return primary value.
T Get () const { return m_value;}
T const & GetCR () const { return m_value;}
void Set (T const &value) { m_value = value;}

// Sort callback for std::sort
static bool cbCompareTagATagB_LT (const ValueSizeSize<T> &dataA, const ValueSizeSize<T> &dataB)
    {
    if (dataA.m_tagA < dataB.m_tagA)
        return true;
    if (dataA.m_tagA > dataB.m_tagA)
        return false;
    if (dataA.m_tagB < dataB.m_tagB)
        return true;
    if (dataA.m_tagB > dataB.m_tagB)
        return false;
    // All equal ...
    return false;
    }

// Sort callback for std::sort
static bool cbCompareTagBTagA_LT (const ValueSizeSize<T> &dataA, const ValueSizeSize<T> &dataB)
    {
    if (dataA.m_tagB < dataB.m_tagB)
        return true;
    if (dataA.m_tagB > dataB.m_tagB)
        return false;
    if (dataA.m_tagA < dataB.m_tagA)
        return true;
    if (dataA.m_tagA > dataB.m_tagA)
        return false;
    // All equal ...
    return false;
    }

//! Lexical sort on Value, then TagA, then TagB
static void SortTagATagB (bvector <ValueSizeSize <T> > &data)
    {
    std::sort (data.begin (), data.end (), cbCompareTagATagB_LT);
    }

//! Lexical sort on Value, then TagA, then TagB
static void SortTagBTagA (bvector <ValueSizeSize <T> > &data)
    {
    std::sort (data.begin (), data.end (), cbCompareTagBTagA_LT);
    }

//! Lexical sort on Value, then TagA, then TagB
static void SortByTags (bvector <ValueSizeSize <T> > &data, bool sortAFirst)
    {
    if (sortAFirst)
        SortTagATagB (data);
    else
        SortTagBTagA (data);
    }

void SwapTags ()
    {
    std::swap (m_tagA, m_tagB);
    }
};

//=======================================================================================
//! Inlinable template for an ordered value of type T with two size_t tags named TagA and TagB.
//! @bsiclass 
//=======================================================================================
template <typename T>
struct OrderedValueSizeSize : public ValueSizeSize <T>
{
OrderedValueSizeSize (T const & value, ptrdiff_t tagA, ptrdiff_t tagB)
    : ValueSizeSize <T> (value, tagA, tagB)
    {}

// Sort callback for std::sort
static bool cbCompareValueTagATagB_LT (const OrderedValueSizeSize<T> &dataA, const OrderedValueSizeSize<T> &dataB)
    {
    if (dataA.m_value < dataB.m_value)
        return true;
    if (dataA.m_value > dataB.m_value)
        return false;
    if (dataA.m_tagA < dataB.m_tagA)
        return true;
    if (dataA.m_tagA > dataB.m_tagA)
        return false;
    if (dataA.m_tagB < dataB.m_tagB)
        return true;
    if (dataA.m_tagB > dataB.m_tagB)
        return false;
    // All equal ...
    return false;
    }

// Sort callback for std::sort
static bool cbCompareTagAValueTagB_LT (const OrderedValueSizeSize<T> &dataA, const OrderedValueSizeSize<T> &dataB)
    {
    if (dataA.m_tagA < dataB.m_tagA)
        return true;
    if (dataA.m_tagA > dataB.m_tagA)
        return false;
    if (dataA.m_value < dataB.m_value)
        return true;
    if (dataA.m_value > dataB.m_value)
        return false;
    if (dataA.m_tagB < dataB.m_tagB)
        return true;
    if (dataA.m_tagB > dataB.m_tagB)
        return false;
    // All equal ...
    return false;
    }

//! Lexical sort on Value, then TagA, then TagB
static void SortValueTagATagB (bvector <OrderedValueSizeSize <T> > &data)
    {
    std::sort (data.begin (), data.end (), cbCompareValueTagATagB_LT);
    }

//! Lexical sort on TagA, then Value, then TagB
static void SortTagAValueTagB (bvector <OrderedValueSizeSize <T> > &data)
    {
    std::sort (data.begin (), data.end (), cbCompareTagAValueTagB_LT);
    }
};
typedef OrderedValueSizeSize <double> DoubleSizeSize;
typedef ValueSizeSize <DRange3d> DRange3dSizeSize;
typedef ValueSizeSize <DPoint3d> DPoint3dSizeSize;

//! @description Templatized type carrying a value type with a boolean indicating if the value
//!     is considered valid.
template<typename T>
struct ValidatedValue
    {
    private:
        T m_value;
        bool m_isValid;
    public:
        //! Default value, marked not valid.
        ValidatedValue() : m_value(), m_isValid(false){}
        //! Specified value, marked valid.
        ValidatedValue(T const &value) : m_value(value), m_isValid(true){}
        //! Specified value, with validity indicated by caller
        ValidatedValue(T const &value, bool isValid) : m_value(value), m_isValid(isValid){}
        //! explicty query for the "the value", without regard for the validity flag.
        T const &Value() const { return m_value; }
        T &Value() { return m_value; }
        //! return the validity flag.
        bool IsValid() const { return m_isValid; }
        //! return the validity flag, copy value to parameter.
        bool IsValid(T &value) const { value = m_value; return m_isValid; }
        //! update the validit member
        void SetIsValid (bool value){m_isValid = value;}
        //! Implicity type conversion operator Returns "the value", without regard for the validity flag.
        operator T (){ return m_value; }
        //! Assign to the T value.  Mark as valid.
        void operator = (T const &value) {m_value = value; m_isValid = true;}
    };

typedef ValidatedValue <struct DVec3d> ValidatedDVec3d;
typedef ValidatedValue <struct DVec2d> ValidatedDVec2d;
typedef ValidatedValue <struct RotMatrix> ValidatedRotMatrix;
typedef ValidatedValue <struct Transform> ValidatedTransform;
typedef ValidatedValue <struct DPoint2d> ValidatedDPoint2d;
typedef ValidatedValue <struct DPoint3d> ValidatedDPoint3d;
typedef ValidatedValue <struct DPoint4d> ValidatedDPoint4d;
typedef ValidatedValue <struct DPlane3d> ValidatedDPlane3d;
typedef ValidatedValue <double> ValidatedDouble;
typedef ValidatedValue <DSegment1d> ValidatedDSegment1d;
typedef ValidatedValue <DSegment3d> ValidatedDSegment3d;
typedef ValidatedValue <CurveLocationDetail> ValidatedCurveLocationDetail;
typedef ValidatedValue <CurveLocationDetailPair> ValidatedCurveLocationDetailPair;
typedef ValidatedValue <PathLocationDetail> ValidatedPathLocationDetail;
typedef ValidatedValue <DEllipse3d> ValidatedDEllipse3d;
typedef ValidatedValue <DRay3d> ValidatedDRay3d;
typedef ValidatedValue <DrapeSegment> ValidatedDrapeSegment;
typedef ValidatedValue <size_t> ValidatedSize;
typedef ValidatedValue <DPoint3dDVec3dDVec3d> ValidatedDPoint3dDVec3dDVec3d;
typedef ValidatedValue <LocalRange> ValidatedLocalRange;
typedef ValidatedValue <struct DRange3d> ValidatedDRange3d;
typedef ValidatedValue <LocalRange> ValidatedLocalRange;
typedef ValidatedValue <ClipPlane> ValidatedClipPlane;

typedef ValidatedValue <FPoint3d> ValidatedFPoint3d;
typedef ValidatedValue <FVec3d> ValidatedFVec3d;


template<typename T>
void CompressDuplicates (bvector<T> &data, bool (*SameMember)(T const &memberA, T const &memberB))
    {
    size_t numTotal = data.size ();
    if (numTotal > 1)
        {
        size_t numAccept = 0;
        for (size_t candidate = 1; candidate < numTotal; candidate++)
            {
            if (!SameMember (data[numAccept], data[candidate]))
                {
                numAccept++;
                if (candidate > numAccept)
                    data[numAccept] = data[candidate];
                }
            }
        data.resize (numAccept);
        }
    }

//! Interface with a virtual method to test 1 parameter.
template <typename T>
struct Acceptor
{
//! Virtual method to test a parameter; use of true/false return subject to context.
virtual bool Accept (T const &data) = 0;
};


//=======================================================================================
//! Class for multiple RefCounted geometry types: ICurvePrimitive, CurveVector, 
//! ISolidPrimitive, MSBsplineSurface, PolyfaceHeader.
//! @bsiclass 
//=======================================================================================
struct IGeometry : RefCountedBase
{
public:

enum class GeometryType
    {
    CurvePrimitive      = 1,
    CurveVector         = 2,
    SolidPrimitive      = 3,
    BsplineSurface      = 4,
    Polyface            = 5,
    };

friend struct OrderedIGeometryPtr;
protected:

GeometryType                m_type;
RefCountedPtr <IRefCounted> m_data;

IGeometry (ICurvePrimitivePtr const& source);
IGeometry (CurveVectorPtr const& source);
IGeometry (ISolidPrimitivePtr const& source);
IGeometry (MSBsplineSurfacePtr const& source);
IGeometry (PolyfaceHeaderPtr const& source);

public:

GEOMDLLIMPEXP GeometryType GetGeometryType () const;

GEOMDLLIMPEXP ICurvePrimitivePtr GetAsICurvePrimitive () const;
GEOMDLLIMPEXP CurveVectorPtr GetAsCurveVector () const;
GEOMDLLIMPEXP ISolidPrimitivePtr GetAsISolidPrimitive () const;
GEOMDLLIMPEXP MSBsplineSurfacePtr GetAsMSBsplineSurface () const;
GEOMDLLIMPEXP PolyfaceHeaderPtr GetAsPolyfaceHeader () const;
GEOMDLLIMPEXP IGeometryPtr Clone () const;
GEOMDLLIMPEXP IGeometryPtr Clone (TransformCR transform) const;
GEOMDLLIMPEXP bool TryGetRange (DRange3dR range) const;
GEOMDLLIMPEXP bool TryGetRange (DRange3dR range, TransformCR transform) const;
GEOMDLLIMPEXP bool TryTransformInPlace (TransformCR transform);
GEOMDLLIMPEXP bool IsSameStructureAndGeometry (IGeometryCR other, double tolerance = 0.0) const;
GEOMDLLIMPEXP static IGeometryPtr Create (ICurvePrimitivePtr const& source);
GEOMDLLIMPEXP static IGeometryPtr Create (CurveVectorPtr const& source);
GEOMDLLIMPEXP static IGeometryPtr Create (ISolidPrimitivePtr const& source);
GEOMDLLIMPEXP static IGeometryPtr Create (MSBsplineSurfacePtr const& source);
GEOMDLLIMPEXP static IGeometryPtr Create (PolyfaceHeaderPtr const& source);

}; // IGeometry

#ifndef SmallGeomLib
/// IGeometryPtr with < operator for sorting on address of the target object.
struct OrderedIGeometryPtr
{
IGeometryPtr m_geometry;
OrderedIGeometryPtr (IGeometryPtr const &target){m_geometry = target;}
OrderedIGeometryPtr (){}
OrderedIGeometryPtr (ICurvePrimitiveCR target)
    {
    ICurvePrimitivePtr myPtr (const_cast <ICurvePrimitiveP>(&target));
    m_geometry = IGeometry::Create (myPtr);
    }

OrderedIGeometryPtr (CurveVectorCR target)
    {
    CurveVectorPtr myPtr (const_cast <CurveVectorP>(&target));
    m_geometry = IGeometry::Create (myPtr);
    }

OrderedIGeometryPtr (PolyfaceHeaderCR target)
    {
    PolyfaceHeaderPtr myPtr (const_cast <PolyfaceHeaderP>(&target));
    m_geometry = IGeometry::Create (myPtr);
    }

OrderedIGeometryPtr (ISolidPrimitiveCR target)
    {
    ISolidPrimitivePtr myPtr (const_cast <ISolidPrimitiveP>(&target));
    m_geometry = IGeometry::Create (myPtr);
    }

OrderedIGeometryPtr (ICurvePrimitivePtr const &target) {m_geometry = IGeometry::Create (target);}
OrderedIGeometryPtr (CurveVectorPtr const &target) {m_geometry = IGeometry::Create (target);}
OrderedIGeometryPtr (ISolidPrimitivePtr const &target) {m_geometry = IGeometry::Create (target);}
OrderedIGeometryPtr (PolyfaceHeaderPtr const &target) {m_geometry = IGeometry::Create (target);}
OrderedIGeometryPtr (MSBsplineSurfacePtr const &target) {m_geometry = IGeometry::Create (target);}

GEOMDLLIMPEXP bool operator < (OrderedIGeometryPtr const &other) const;
};
#endif

//! Enumeration of simple uv boundary rules
struct UVBoundarySelect
{
public:
//! No boundary implied
static const int Unbounded = 0;
//! Restrict U,V, U+V between 0 and 1 -- i.e. triangle in UV space.
static const int Triangle  = 1;
//! Restrict U,V between 0 and 1 -- i.e. unit square in UV space.
static const int UnitSquare = 2;
//! Restrict U,V between -1 and 1 -- i.e. square centered at origin, side length 2.
static const int CenteredSquare = 3;

//! Restrict U,V to unit circle {U^2 + V^2 < 1}
static const int UnitCircle = 4;

private:
    int m_selector;   // one of the constants UVBoundarySelect::Unbounded, UVBoundarySelect::Triangle, UVBoundarySelect::Square, UVBoundarySelectUnitCircle
public:
//! constructor with enumerated
UVBoundarySelect (int select);

//! test containment
bool IsInOrOn (double u, double v) const;

//! test containment
bool IsInOrOn (DPoint2dCR uv) const;

};

END_BENTLEY_GEOMETRY_NAMESPACE

#include "dpoint2d.h"
#include "dpoint3d.h"
#include "FPoint3d.h"
#include "dpoint4d.h"
#include "GraphicsPoint.h"
#include "GeoPoint.h"
#include "dvec2d.h"
#include "dvec3d.h"
#include "FVec3d.h"
#include "DRange1d.h"
#include "drange2d.h"
#include "drange3d.h"
#include "FRange3d.h"
#include "dmatrix4d.h"
#include "dmap4d.h"
#include "dplane3d.h"
#include "dray3d.h"
#include "DSegment1d.h"
#include "dsegment3d.h"
#include "dellipse3d.h"
#include "rotmatrix.h"
#include "transform.h"
#include "BsplineStructs.h"
#include <Geom/CurveDetails.h>
#include "OperatorOverload.h"
#ifndef SmallGeomLib
#include "MSBsplineCurve.h"
#include "MSInterpolationCurve.h"
#include "MSBsplineSurface.h"
#endif
#include "Angle.h"
#include "DPoint3dOps.h"
#include "DTriangle3d.h"

/*__PUBLISH_SECTION_END__*/
#include "CurveConstraint.h"
#include "DBilinearPatch3d.h"
#include "GeometryNode.h"


BEGIN_BENTLEY_GEOMETRY_NAMESPACE

    typedef bvector<DPoint3d> STDVectorDPoint3d;
    typedef STDVectorDPoint3d & STDVectorDPoint3dR;
    typedef STDVectorDPoint3d const & STDVectorDPoint3dCR;

    typedef bvector<double> STDVectorDouble;
    typedef STDVectorDouble & STDVectorDoubleR;
    typedef STDVectorDouble const & STDVectorDoubleCR;

    typedef bvector<int> STDVectorInt;
    typedef STDVectorInt & STDVectorIntR;
END_BENTLEY_GEOMETRY_NAMESPACE

#include "jrangetest.h"
#include "AnalyticRoots.h"
#include "TriDiagonalSolver.h"
//#include "DMoments3d.h"
#include "BSIQuadrature.h"
#include "XYBucketSearch.h"
#ifndef NoGeomDSpiral2dBase
#include "DSpiral2dBase.h"
#endif
#ifndef NoGeomNewton
#include "newton.h"
#endif
#include "memfuncs.h"
//#include "XYRangeTree.h"
//#include "XYZRangeTree.h"

#include "ccctangent.fdf"

//#include "internal/dcone3d.fdf"
#include "dconic4d.fdf"
//#include "internal/ddisk3d.fdf"
//#include "internal/dellipsoid3d.fdf"
#include "dmap4d.fdf"
#include "doublefuncs.fdf"
#include "dpoint3darray.fdf"
#include "dpoint3ddvec3d.fdf"
#include "drange3d.fdf"

//#include "internal/dtoroid3d.fdf"
#include "ellipsefillet.fdf"
#include "eigensys3d.fdf"
#include "ellipticintegrals.fdf"
#include "fpoint2d.fdf"
#include "fpoint3d.fdf"
#include "frange2d.fdf"
#include "frange3d.fdf"
#include "graphicspoint.fdf"
#include "linalg.fdf"
#include "LinearAlgebra.h"
#include "lineargeom.fdf"
#include "polygon3d.fdf"
#include "polygondecomp.fdf"
#include "polyline2d.fdf"
#include "polyline3d.fdf"
#include "polyplane4d.fdf"
#include "polysolv.fdf"
#include "proximitydata.fdf"
#include "quadeqn.fdf"
#include "quadric.fdf"
#include "rotations.fdf"
#include "simpson.fdf"
#include "svd.fdf"


/*__PUBLISH_SECTION_START__*/
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
//=======================================================================================
//! Structure to record name and other data describing error tests.
//! These are instantiated into the MeshAnnotationVector.
//=======================================================================================
struct MeshAnnotation
    {
    Utf8String m_description;   //<! name of error condtion tested
    size_t  m_pass;             //<! number of times passed
    size_t m_fail;              //<! number of times failed
    bvector<std::pair<size_t, ptrdiff_t>> m_indices;    //! array of <index, ptrdiff_t> pairs for error occurances.
    bvector<std::pair<size_t, DPoint3d>> m_xyz;         //! array of <index, xyzx> pairs for error occurrances.

    MeshAnnotation (Utf8CP name) : m_description (name), m_pass (0), m_fail (0) {}

    void IncrementPass () {m_pass++;}
    void IncrementFail () {m_fail++;}
    void Record (size_t index, size_t tag) { m_indices.push_back (std::pair<size_t, ptrdiff_t> (index, (ptrdiff_t)tag)); }
    void Record (size_t index, DPoint3dCR tag) { m_xyz.push_back (std::pair<size_t, DPoint3d> (index, tag)); }
    void Record (size_t index, int tag) { m_indices.push_back (std::pair<size_t, ptrdiff_t> (index, (ptrdiff_t)tag)); }
    };
//=======================================================================================
//! Vector of MeshAnnotation structures.
//=======================================================================================
struct MeshAnnotationVector : bvector<MeshAnnotation>
{
private:
bool m_recordAllTestDescriptions;
size_t m_totalPass;
size_t m_totalFail;
public:
    //! @param [in] recordAllTestDescriptions true to create array entries (with pass counts) even when tests pass.
    MeshAnnotationVector (bool recordAllTestDescriptions) : m_recordAllTestDescriptions (recordAllTestDescriptions), m_totalPass (0), m_totalFail (0)
    { }
//! Query the total number of tests passed.
size_t GetTotalPass () { return m_totalPass;}
//! Query the total number of tests failed.
size_t GetTotalFail () { return m_totalFail;}
    //! return index to indicated key.  Search is linear, but in reverse order so repeated references go fast.
    //! returns SIZE_MAX if key not found
    size_t Find (Utf8CP key);
    //! Return the index where a key was found or added.
    size_t FindOrAdd (Utf8CP key);
    //! Return index where recorded (SIZE_MAX if not recorded)
    size_t Assert (bool condition, Utf8CP key);
    //! check and record a conditionw with index and tag that describe the condition.
    void Assert (bool condition, Utf8CP key, size_t index, size_t tag);
    //! check and record a conditionw with index and tag that describe the condition.
    void Assert (bool condition, Utf8CP key, size_t index, DPoint3dCR tag);
    //! check and record a conditionw with index and tag that describe the condition.
    void Assert (bool condition, Utf8CP key, size_t index, int tag);
};

struct _fPoint2d
    {
    float x;
    float y;
    };

struct _fRange2d
    {
    FPoint2d low;
    FPoint2d high;
    };

END_BENTLEY_GEOMETRY_NAMESPACE

#include <Geom/ClipPlane.h>
#include <Geom/ClipPlaneSet.h>
#include <Geom/AlternatingConvexClipTree.h>
#ifndef SmallGeomLib

#include <Geom/CurvePrimitiveId.h>
#include <Geom/CurvePrimitive.h>
#include <Geom/CurveVector.h>
#include <Geom/SolidPrimitive.h>
#include <Geom/CurveTopologyId.h>
#include <Geom/Polyface.h>
#endif

/*__PUBLISH_SECTION_END__*/

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

struct _dEllipsoid3d
    {
    Transform    frame;          /* Coordinate frame for a unit sphere. */
    DRange2d        parameterRange; /* Range of (theta, phi) parameter space. */
    };

struct _dToroid3d
    {
    Transform    frame;              /* Coordinate frame for a toroid. */
    double          minorAxisRatio;     /* Scale factor to produce the in-plane 0-degree vector of the     */
                                        /* minor radius circle as a multiple of the primary circle vector. */
    DRange2d        parameterRange;     /* Range of (theta, phi) parameter space. */
    };

struct _dDisk3d
    {
    Transform frame;
    DRange2d     parameterRange;
    };

//!
//! @group DCone3d
//! @description A DCone3d is a conical surface in 3D.
//! @remarks The surface is parameterized in terms of angle theta and altitude z as
//! <pre>
//!       X = C + (1 +  z * (radiusFraction - 1)) * (U * cos(theta) + V * sin(theta)) + z * W
//! </pre>
//! where
//! <ul>
//! <li>C is the center of the base circle.
//! <li>U is the vector from the base center to the 0-degree point on the base circle.
//! <li>V is the vector from the base center to the 90-degree point on the base circle.
//! <li>W is the axis vector.
//! <li>radiusFraction is the radius of the cross-section circle at z = 1.
//!   <ul>
//!   <li>For perfect cylinder, radiusFraction = 1.
//!   <li>For cone with apex at z = 1, radiusFraction = 0.
//!   <li>For cone with apex at z = Z, radiusFraction = 1 - 1/Z = (Z-1)/Z.
//!   <li>For given radiusFraction, the apex is at z = 1/(1-radiusFraction).
//!   </ul>
//! </ul>
//! @remarks The basis vectors do not have to be normalized or orthogonal.
//! @remarks The parameter range for a complete cone is
//! <pre>
//!       -pi < theta < pi
//!       0 < z < 1
//! </pre>
//!
struct _dCone3d
    {
    //! @description Coordinate frame for the cone (local to world).  Matrix part columns are cone axes in world coordinates; translation is
//!            base center.  Cone base is the xy-plane unit circle in local coordinates (cos(theta), sin(theta), z=0). 

    Transform    frame;
    //! @description Scale factor to produce the z=1 cross section radius. 

    double          radiusFraction;
    //! @description Range of parameters theta and z, in [-pi,pi] and [0,1], respectively. 

    DRange2d        parameterRange;
    };

//!
//! @group DConic4d
//! @description A DConic4d can represent an ellipse, parabola or hyperbola in 4D homogeneous space.
//! @remarks If C is a homogeneous point ("center") and U and V are any homogeneous vectors,
//! <pre>
//!       X = C + U * cos(theta) + V * sin(theta)
//! </pre>
//! sweeps X along an elliptic curve in homogeneous space. This may be a hyperbola, parabola, or ellipse in 3D.
//! @remarks The basis vectors do not have to be normalized or orthogonal.
//!
typedef struct _dConic4d
    {
    //!  @description The homogeneous center. 

    DPoint4d center;
    //!  @description The vector from the homogeneous center to the 0-degree point of the curve. 

    DPoint4d vector0;
    //!  @description The vector from the homogeneous center to the 90-degree point of the curve. 

    DPoint4d vector90;
    //!  @description The radian start angle of the arc in its parameter space. 

    double start;
    //!  @description The radian angle swept by the arc in its parameter space. 

    double sweep;
    } DConic4d;


/* Classic HPoints -- multiple, explicitly managed rubber arrays */
typedef struct HPoints
    {
    DPoint4d   *pointP;
    DPoint4d   *scratchP;           /* 2nd array available for scratch use.  May
                                        be swapped with the primary pointer if appropriate */
    int        *maskP;
    int         mPoint;
    int         nPoint;
    int         headerMask;

    int         arrayMask;

    int         n1;                 /* For HPOINTS_MESH, the first mesh count */
    int         n2;                 /* For HPOINTS_MESH, the second mesh count */
    } HPoints;
typedef void (*HPointsMarkFunction)( HPoints *pHeader, int argVal);


typedef struct _proximityData
    {
    //! @description If false, remaining data fields have not be set. 

    bool        dataValid;
    //! @description Index of search step at which the closest point occurred. 

    int         closeIndex;
    //! @description Parameter of closest approach. 

    double      closeParam;
    //! @description Homogeneous coordinates of closest approach. 

    DPoint4d    closePoint;
    //! @description Squared distance to the closest point. 

    double      closeDistanceSquared;
    //! @description Point being tested. 

    DPoint3d    testPoint;
    } ProximityData;

#define MS_SMALL_PLANE_SET_DIMENSION    16
typedef struct
    {
    DPlane3d    planes[MS_SMALL_PLANE_SET_DIMENSION];
    int         numPlanes;
    int         outside;
    } DPlane3d_SmallSet;


END_BENTLEY_GEOMETRY_NAMESPACE

/*__PUBLISH_SECTION_START__*/
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

typedef struct pointList
    {
    int32_t             numPoints;
    DPoint3d            *points;
    } PointList;

END_BENTLEY_GEOMETRY_NAMESPACE
/*__PUBLISH_SECTION_END__*/

#include "geombezier.h"
#include "Polynomials.h"
#include "TensorProducts.h"
#include "msembeddedarray.h"

#ifndef SmallGeomLib
#include <Geom/bspApi.h>
#endif
