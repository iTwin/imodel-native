/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Mtg/GpaApi.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------..------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

//#include <Mstn\Mstntypes.h>
#include <Geom/GeomApi.h>

/*__PUBLISH_SECTION_END__*/
#include <Mtg/MtgApi.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


#ifndef GraphicsPointArrayDefined
#define GraphicsPointArrayDefined 1
typedef struct GraphicsPointArray *GraphicsPointArrayP;
typedef struct GraphicsPointArray const *GraphicsPointArrayCP;
#endif

/*--------------------------------------------------------------+
| An HPoints is a header structure for multiple points.         |
| The insert operations take care of reallocating the array     |
| memory for both the points and the scratch array.             |

Call the three low order (right side) mask nibbles are "CurveType", "PointType", and "BreakType" parts. (CPB)
<ul>
<li>Linestring (CurveType=0)
  <ul>
    <li>PointType = 0 always (no special point types)</li>
    <li>HPOINT_MASK_BREAK (simple break) marks end of linestring.</li>
  </ul>
</li>
<li>Ellipse (CurveType = HPOINT_MASK_CURVETYPE_ELLIPSE)
  <ul>
  <li>Always five points:
    <ul>
    <li>HPOINT_MASK_ELLIPSE_STARTEND = actual startpoint.   "b" field is angle parameter</li>
    <li>HPOINT_MASK_ELLIPSE_VECTOR   = 0 degree point.</li>
    <li>HPOINT_MASK_ELLIPSE_CENTER   = center point</li>
    <li>HPOINT_MASK_ELLIPSE_VECTOR   = 90 degree point</li>
    <li>HPOINT_MASK_ELLIPSE_STARTEND | HPOINTS_MASK_BREAK = actual endpoint.  "b" field is angle parameter</li>
  </ul>
</li>
<li>Bezier (CurveType = HPOINT_MASK_CURVETYPE_BEZIER)
  <ul>
  <li>Number of points is order</li>
  <li>HPOINT_MASK_BEZIER_STARTEND = first pole, also startpoint</li>
  <li>(order-2 points)HPOINT_MASK_BEZIER_POLE = non-endpoint pole</li>
  <li>HPOINT_MASK_BEZIER_STARTEND | HPOINTS_MASK_BREAK = final pole, also endpoint
  </ul>
</li>
<li>Bspline (CurveType = HPOINT_MASK_CURVETYPE_BSPLINE)
  <ul>
  <li>Notation: K = number of knots</li>
  <li>Notation: P = number of poles</li>
  <li>Notation: ORDER = curve order = one more than curve degree</li>
  <li>Usual convention: (K = P + ORDER)</li>
  <li>First, last points (HPOINTS_MASK_BSPLINE_STARTEND) are true endpoints but not considered part of the working pole sequence.!!!</li>
  <li>This allows unclamped knots while still having exact endpoints for inspection !!!</li>
  <li>Graphics points 1..N are the N poles. (HPOINTS_MASK_BSPLINE_POLE)</li>
  <li>zero-based knot index k appears at graphics point k.</li>
  <li>The (ORDER-2) GraphicsPoints JUST AFTER THE start  point have xyzw matching the start point but are not part of the poles sequence.
            (These points are marked HPOINTS_MASK_BSPLINE_KNOT_CARRIER)</li>
  <li>zero-based pole index (p) appears at graphics point (p+ORDER-1).</li>
  <li>The KNOT Range for the I'th bezier chunk is between the knot values in GraphicsPoint offset (ORDER+I-1) and (ORDER+I)</lu>
  <li>The SUPPORTING KNOTINDICES for the I'th bezier chunk are the (2*ORDER) knots starting at GraphicsPoint offset (I).</li>
  <li>The SUPPORTING POLES for the I'th bezier chunk are the (ORDER) poles starting at GraphicsPoint offset (I+ORDER-1)</li>
  <li>ORDER is not explicitly stored.  It is determined by counting GraphicsPoints from the HPOINT_MASK_BSPLINE_STARTEND to the first HPOINTS_MASK_BSPLINE_POLE</li>
  <li>Hence the work to copy out the support (poles and knots) for bezier chunk (I) requires work strictly proportional to (ORDER).
        (Followed by something proportional to ORDER*ORDER calculations to clamp the knots of that bezier chunk)</li>
  <li>Closure is not explicitly stored.  Closure should be detectable as (1) exact wraparound of (ORDER-1) poles and (2) shift-by-period for first, last ORDER knots.</li>
  </ul>
</li>
</ul>

Nibble layout is 00RRICCB where
<ul>
<li>B  = 4 bits for break marks</li>
<li>CC = 8 bits for curve type</li>
<li>I  = 4 bits for in/out markup</li>
<li>RR = 8 bits for curve order.</li>
</ul>
+---------------------------------------------------------------*/

/* Mask values for individual points */
#define HPOINT_NORMAL      0
#define HPOINT_MASK_BREAK  0x00000001   /* LAST point in polygon or polyline is marked as a break */
#define HPOINT_MASK_POINT  0x00000002   /* This point is isolated. Function that sets
                                                this also sets BREAK on both this and its predecessor*/
#define HPOINT_MASK_FRAGMENT_BREAK      0x00000004   /* Start or end of a fragment during clipping */
#define HPOINT_MASK_MAJOR_BREAK         0x00000008  /* End of major block (e.g. loop of character or grouped hole?) */
#define HPOINT_MASK_USER1               0x00010000  /* Temporary mask */
#define HPOINT_MASK_GROUP_BREAK         0x00020000  /* End of group of loops (major breaks) in loop containment */


#define HPOINT_MASK_CURVE_BITS           0x00000FF0    /* All curve definition points have at least one of these bits set */

#define HPOINT_MASK_CURVETYPE_BITS       0x00000F00    /* These bits indicated curve type */
#define HPOINT_MASK_POINTTYPE_BITS       0x000000F0    /* This bits indicate point type.  Interpretation
                                                            depends on curve type */
#define HPOINT_MASK_BREAK_BITS           0x0000000F     /* These bits indicate end of curve or loop */
#define HPOINT_MASK_INOUT_BITS           0x00003000    /* These 2 bits indicate in/out classification.
                                                                Note that there are 4 states:
                                                                00 = unclassified, ambiguous, unknown
                                                                11 = ON the boundary
                                                                01 = IN
                                                                11 = OUT
                                                        If you just look for IN bits, you will also
                                                                get the ON case.  Ditto for OUT.
                                                        */
#define HPOINT_MASK_INOUT_BIT_IN        0x000001000     /* "in" bit */
#define HPOINT_MASK_INOUT_BIT_OUT       0x000002000     /* "out" bit */


#define HPOINT_MASK_CURVETYPE_ELLIPSE    0x00000100    /* All points of an ellipse get this mask */
#define HPOINT_MASK_ELLIPSE_STARTEND     0x00000010    /* Ellipse start or end point */
#define HPOINT_MASK_ELLIPSE_CENTER       0x00000020    /* Ellipse center point */
#define HPOINT_MASK_ELLIPSE_VECTOR       0x00000030    /* Vector to ellipse  0 or 90 degree point */

#define HPOINT_MASK_CURVETYPE_BEZIER     0x00000200    /* All points of a bezier get this mask */
#define HPOINT_MASK_BEZIER_STARTEND      0x00000010    /* Start or end point of bezier */
#define HPOINT_MASK_BEZIER_POLE          0x00000020    /* Any intermediate pole of the bezier */


// Bspline packing.
#define HPOINT_MASK_CURVETYPE_BSPLINE    0x00000300    /* All points of a bspline get this mask */
#define HPOINT_MASK_BSPLINE_STARTEND     0x00000010    /* Actual start or end point of bspline.  (NOT a pole) knot values IS significant.*/
#define HPOINT_MASK_BSPLINE_POLE         0x00000020    /* Bspline pole with knot. */
#define HPOINT_MASK_BSPLINE_EXTRA_POLE   0x00000030    /* Bspline "extra" pole -- coordinates repolicate start pole to facilitate transform and range
                                                                logic, but knot value may be different. */



#define HPOINT_GET_CURVETYPE_BITS(mask)                 ((mask) & HPOINT_MASK_CURVETYPE_BITS)
#define HPOINT_GET_POINTTYPE_BITS(mask)                 ((mask) & HPOINT_MASK_POINTTYPE_BITS)
#define HPOINT_IS_ELLIPSE_POINT(mask)                   (HPOINT_GET_CURVETYPE_BITS(mask) == HPOINT_MASK_CURVETYPE_ELLIPSE)
#define HPOINT_IS_BEZIER_POINT(mask)                    (HPOINT_GET_CURVETYPE_BITS(mask) == HPOINT_MASK_CURVETYPE_BEZIER)
#define HPOINT_IS_CURVE_POINT(mask)                     (HPOINT_GET_CURVETYPE_BITS(mask) != 0)

#define HPOINT_MASK_ORDER               0x00FF0000
#define HPOINT_MASK_ORDER_BITSHIFT      (16)
#define HPOINT_MAX_ORDER                (255)
enum HPointArrayMasks
    {
    HPOINT_ARRAYMASK_DEFAULT            = 0,
    HPOINT_ARRAYMASK_FILL               = 0x00000001,
    HPOINT_ARRAYMASK_CURVES             = 0x00000002,
    HPOINT_ARRAYMASK_HAS_MAJOR_BREAKS   = 0x00000004,
    HPOINT_ARRAYMASK_STROKED_DATA       = 0x00000008,
    };

#define IPOINT_NORMAL      1
#define IPOINT_MASK_BREAK  0x00000003
#define IPOINT_MASK_POINT  0x00000004

struct TaggedBezierDPoint4d
{
GraphicsPointArrayCP m_pSource;
size_t m_primitiveIndex;
size_t m_intervalIndex;
int    m_order;
bool   m_isNullInterval;
double m_knot0;
double m_knot1;
DPoint4d m_poles[MAX_BEZIER_CURVE_ORDER];

TaggedBezierDPoint4d (GraphicsPointArrayCP pSource);
// COPY poles into working bezier...
TaggedBezierDPoint4d (GraphicsPointArrayCP pSource,
        size_t index,
        DPoint4dCP poles, int order);

bool LoadSingleBezier (size_t primitiveIndex);
bool LoadBsplineSpan (size_t primitiveIndex, size_t spanIndex);

double LocalToGlobal (double u) const;
bool IsNullInterval () const;
};

typedef bool    (*GPAPairFunc_DSegment4dDSegment4d)
        (
        void                    *pContext,
        GraphicsPointArrayCP pSource0,
        int                     index0,
        DSegment4d              *pSegment0,
        GraphicsPointArrayCP pSource1,
        int                     index1,
        DSegment4d              *pSegment1
        );

typedef bool    (*GPAPairFunc_DSegment4dDConic4d)
        (
        void                    *pContext,
        GraphicsPointArrayCP pSource0,
        int                     index0,
        DSegment4d              *pSegment0,
        GraphicsPointArrayCP pSource1,
        int                     index1,
        DConic4d                *pConic1
        );

typedef bool    (*GPAPairFunc_DSegment4dBezierDPoint4dTagged)
        (
        void                    *pContext,
        GraphicsPointArrayCP pSource0,
        int                     index0,
        DSegment4d              *pSegment0,
        TaggedBezierDPoint4d    &bezier1
        );

typedef bool    (*GPAPairFunc_DConic4dDConic4d)
        (
        void                    *pContext,
        GraphicsPointArrayCP pSource0,
        int                     index0,
        DConic4d                *pConic0,
        GraphicsPointArrayCP pSource1,
        int                     index1,
        DConic4d                *pConic1
        );

typedef bool    (*GPAPairFunc_DConic4dBezierDPoint4dTagged)
        (
        void                    *pContext,
        GraphicsPointArrayCP pSource0,
        int                     index0,
        DConic4d                *pConic0,
        TaggedBezierDPoint4d    &bezier1
        );

typedef bool    (*GPAPairFunc_BezierDPoint4dBezierDPoint4dTagged)
        (
        void                    *pContext,
        TaggedBezierDPoint4d    &bezier0,
        TaggedBezierDPoint4d    &bezier1
        );

typedef bool    (*GPAFunc_DSegment4d)
        (
        void                    *pContext,
        GraphicsPointArrayCP    pSource,
        int                     index,
        DSegment4d              *pSegment0
        );

typedef bool    (*GPAFunc_DConic4d)
        (
        void                    *pContext,
        GraphicsPointArrayCP pSource,
        int                     index,
        DConic4d                *pConic
        );

typedef bool    (*GPAFunc_BezierDPoint4dTagged)
        (
        void                    *pContext,
        TaggedBezierDPoint4d  &bezier
        );

typedef void (*GPAFunc_TangentialIntegrand)
        (
        void                    *pContext,      /* Caller Context */
        double                  *pIntegrand,    /* Array of integrands computed by the callback */
        int                     numIntegrand,   /* Number of integrands expected */
        DPoint3d                *pPoint,        /* Point on curve (homogeneous) */
        DPoint3d                *pTangent       /* Tangent vector (homogeneous) */
        );


typedef bool    (*GPAPairFunc_IndexIndex)
    (
    void    *pContext,
    GraphicsPointArrayCP pSource0,
    int     index0,
    GraphicsPointArrayCP pSource1,
    int     index1
    );

typedef bool    (*GPATripleFunc_IndexIndexIndex)
    (
    void    *pContext,
    GraphicsPointArrayCP pSource0,
    int     index0,
    GraphicsPointArrayCP pSource1,
    int     index1,
    GraphicsPointArrayCP pSource2,
    int     index2
    );

/* Bit mask values for fractional parameter relabeling
    in jmdlGraphicsPointArray_relabelFractionParameterToAccumulatedArcLength
*/
enum
    {
    GPA_RELABEL_START = 1,
    GPA_RELABEL_END   = 2
    };

END_BENTLEY_GEOMETRY_NAMESPACE 

#include "Mtg/gp_beziersection.fdf"
#include "Mtg/gp_bool.fdf"
#include "Mtg/gp_offset.fdf"
#include "Mtg/gp_ellipse.fdf"
//#include "Mtg/gp_fragments.fdf"
#include "Mtg/gp_hatch.fdf"
#include "Mtg/gp_hatchblock.fdf"
#include "Mtg/gp_inout.fdf"
#include "Mtg/gp_intersectxy.fdf"
#include "Mtg/gp_pairs.fdf"
#include "Mtg/gp_perptan.fdf"
#include "Mtg/gp_polygon.fdf"
#include "Mtg/gp_properties.fdf"
#include "Mtg/gp_silhouette.fdf"
#include "Mtg/gp_stroke.fdf"
//#include "Mtg/gp_clip.fdf"
#include "Mtg/graphicspointarray.fdf"
#include "Mtg/gp_developable.fdf"
#include "Mtg/jmdl_polyhdr.fdf"
#include "Mtg/jmdl_planeset.fdf"

#ifdef CompileTriangleIndices
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*----------------------------------------------------------------------+
|                                                                       |
| Structure for compact triangle lists for rendering.                   |
|                                                                       |
+----------------------------------------------------------------------*/
typedef struct
    {
    DPoint3d *pPointArray;
    DPoint3d *pNormalArray;
    DPoint2d *pUVArray;
    int     *pTriangleToVertexArray;
    int     *pTriangleToTriangleArray;
    int     maxPoint;
    int     maxTri;
    int     numPoint;
    int     numTri;
    } TriangleIndices;
END_BENTLEY_GEOMETRY_NAMESPACE
#include "TriangleIndices.fdf"
#endif



