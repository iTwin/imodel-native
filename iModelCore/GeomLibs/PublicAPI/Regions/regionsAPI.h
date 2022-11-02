/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Mtg/MtgApi.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


typedef struct _RG_Header           RG_Header;

typedef struct _RG_IntersectionList RG_IntersectionList;
typedef struct _RG_Intersection     RG_Intersection;

typedef struct _RG_GapList          RG_GapList;
typedef struct _RG_GapVertex        RG_GapVertex;

typedef struct _RG_EdgeData         RG_EdgeData;
typedef int     RG_CurveId;

struct RIMSBS_Context;

#define RG_NULL_CURVEID (-1)
#define RG_NULL_VERTEXID (-1)

#define RG_COMPRESSED_DEPTH_EXTERIOR    (0)
#define RG_COMPRESSED_DEPTH_PRIMARY     (1)
#define RG_COMPRESSED_DEPTH_CANAL       (2)
#define RG_COMPRESSED_DEPTH_ISLAND      (3)

#define RG_PFC_POLYLINE_IN_FACE (-101)
#define RG_PFC_FACE_IN_POLYGON  (-102)
#define RG_PFC_EDGE_CONFLICT    (-100)

typedef bool    (*RGC_GetCurveRangeFunction)
        (
        RIMSBS_Context        *pContext, /* => general context */
        DRange3d    *pRange,   /* <= returned range */
        RG_CurveId  iCurve    /* => curve identifier */
        );

typedef bool    (*RGC_IntersectCurveCurveFunction)
        (
        RIMSBS_Context     *pContext,                         /* => application context */
        RG_Header               *pRG,               /* => region context */
        RG_IntersectionList     *pIntersections,    /* <=> list of intersection parameters */
        RG_EdgeData *pEdge0Data,                    /* => first edge, with curve id */
        RG_EdgeData *pEdge1Data                     /* => second edge, with curve id */
        );

typedef bool    (*RGC_SweptCurvePropertiesFunction)
        (
        RIMSBS_Context        *pContext,                      /* => application context */
        RG_EdgeData *pEdgeData,                     /* => edge for computation. */
        double      *pArea,                         /* <= swept area */
        double      *pAngle,                        /* <= swept angle */
        const DPoint3d    *pPoint                   /* => fixed point of sweep */
        );


typedef bool    (*RGC_IntersectSegmentCurveFunction)
        (
        RIMSBS_Context     *pContext,                         /* => application context */
        RG_Header               *pRG,               /* => region context */
        RG_IntersectionList     *pIntersections,    /* <=> list of intersection parameters */
        RG_EdgeData *pEdge0Data,                    /* => first edge, with xyz data for segment */
        RG_EdgeData *pEdge1Data                     /* => second edge, with curve id */
        );

typedef bool    (*RGC_EvaluateCurveFunction)
        (
        RIMSBS_Context     *pContext,     /* => application context */
        DPoint3d *pXYZ,         /* <= array of evaluated points */
        DPoint3d *pTangent,     /* <= array of evaluated tangents */
        double   *pParam,       /* => parameters to evaluate */
        int      nParam,        /* => number of points to evaluate */
        RG_CurveId  iCurve      /* => curve identifier */
        );

typedef bool    (*RGC_EvaluateDerivativesFunction)
        (
        RIMSBS_Context     *pContext,     /* => application context */
        DPoint3d *pXYZ,         /* <= array of point, first derivative, etc.  Total numDerivative+1 points.  Point is always evaluated */
        int      nDerivative,   /* => number of derivatives to evaluate */
        double   param,
        RG_CurveId  iCurve      /* => curve identifier */
        );

typedef bool    (*RGC_CreateSubcurveFunction)
        (
        RIMSBS_Context        *pContext,      /* => general context */
        RG_CurveId  *piNewCurve,    /* => new curve identifier */
        RG_CurveId  iParentCurve,   /* => parent curve identifier */
        double      s0,             /* => start parameter of subcurve */
        double      s1              /* => end parameter of subcurve */
        );

typedef bool    (*RGC_GetClosestXYPointOnCurveFunction)
        (
        RIMSBS_Context     *pContext,         /* => application context */
        double   *pMinParam,        /* => parameter at closest approach point */
        double   *pMinDistSquared,  /* => squard distance to closest approach point */
        DPoint3d *pMinPoint,        /* => closest approach point */
        DPoint3d *pMinTangent,      /* => tangent vector at closest approach point */
        DPoint3d *pPoint,           /* => space point */
        RG_CurveId  iCurve      /* => curve identifier */
        );

typedef void (*RG_FaceLoopFunction)
        (
        void    *pArg0,             /* => user context (e.g. mdl descriptor) */
        void    *pArg1,             /* => user context (e.g. mdl function address) */
        void    *pUserData,         /* => user context */
        DPoint3d *pPointArray,      /* => points around face.   Last point NOT doubled */
        int     numPoint,           /* => number of points in loop */
        MTGNodeId  nodeId           /* => a starting node on the face. */
        );


typedef bool    (*RGC_IntersectCurveCircleXYFunction)
        (
        RIMSBS_Context     *pContext,                     /* => application context */
        bvector<double> *pParamArray,       /* <= array of intersection parameters */
        EmbeddedDPoint3dArray *pPointArray,     /* <= array of intersection parameters */
        RG_EdgeData *pEdgeData,                 /* => first edge, with curve id */
        const DPoint3d *pCenter,                /* => circle center */
        double radius                           /* => circle radius */
        );

typedef bool    (*RGC_IntersectCurvePlaneFunction)
        (
        RIMSBS_Context     *pContext,                     /* => application context */
        bvector<double> *pParamArray,       /* <= array of intersection parameters */
        EmbeddedDPoint3dArray *pPointArray,     /* <= array of intersection parameters */
        RG_EdgeData *pEdgeData,                 /* => first edge, with curve id */
        const DPoint3d *pPoint,                 /* => any point on plane */
        const DPoint3d *pNormal                 /* => plane normal */
        );

typedef bool    (*RGC_GetGroupIdFunction)
        (
        RIMSBS_Context        *pContext,                  /* => application context */
        int         *pGroupId,                  /* <= group id */
        RG_CurveId  iCurve                      /* => curve index */
        );

typedef bool    (*RGC_ConsolidateCoincidentGeometryFunction)
        (
        RIMSBS_Context        *pContext,                  /* => application context */
        double      tolerance
        );

typedef void (*RGC_AppendAllCurveSamplePointsFunction)
        (
        RIMSBS_Context        *pContext,                  /* => application context */
        EmbeddedDPoint3dArray   *pXYZArray      /* <= array to receive sample points */
        );

typedef bool    (*RGC_TransformCurveFunction)
        (
        RIMSBS_Context        *pContext,                  /* => application context */
        RG_CurveId  iCurve,                     /* => curve index */
        const Transform *pTransform             /* => transform to apply */
        );

typedef void (*RGC_TransformAllCurvesFunction)
        (
        RIMSBS_Context        *pContext,                  /* => application context */
        const Transform *pTransform             /* => transform to apply */
        );

/*--------------------------------------------------------------------------------------+
|
|   The voluminous user args for RG_NodeFunction are expected to be used as follows:
|   pArg0, pArg1 -- for calling back to mdl function
|   pUserData0 -- the applications curve context
|   pRG -- the region context (vertex coordinates)
|   pGraph -- the MTGGraph (connectivity)
|   nodeId -- specific to computation, provided by RG
|   pUserData1 -- specific to this computation, provided by application
|   userInt -- specific to this computation, provided by application.
|
+--------------------------------------------------------------------------------------*/


typedef void (*RG_NodeFunction)
        (
        void    *pArg0,             /* => user context (e.g. mdl descriptor) */
        void    *pArg1,             /* => user context (e.g. mdl function address) */
        void    *pUserData0,        /* => user context */
        RG_Header   *pRG,           /* => region graph */
        MTGGraph   *pGraph,         /* => connectivity graph for traversal */
        MTGNodeId  nodeId,          /* => an MTG Node */
        MTGNodeId  altNodeId,      /* => another MTG NOde */
        void        *pUserData1,    /* => user data */
        int         userInt,        /* => user data */
        DPoint3d    *pPoint0,
        DPoint3d    *pPoint1
        );


typedef void (*RG_RangeFunction)
        (
        void    *pArg0,             /* => user context (e.g. mdl descriptor) */
        void    *pArg1,             /* => user context (e.g. mdl function address) */
        void    *pUserData,         /* => user context */
        DRange3d    *pRange,        /* => range */
        MTGNodeId  nodeId           /* => relevant node in graph */
        );

/*--------------------------------------------------------------------------------------+
|
| Callback for debug output.
|
+--------------------------------------------------------------------------------------*/
#define     RG_NORMALDRAW           0       /* drawing mode = set */
#define     RG_ERASE                1       /* erase from screen */
#define     RG_HILITE               2       /* highlight */
#define     RG_TEMPDRAW             3       /* draw temporarily */
#define     RG_TEMPERASE            4       /* erase temporarily drawn */

typedef void (*RG_OutputLinestring)
        (
        void    *pDebugContext,
        RG_Header *pRG,
        const DPoint3d *pPointArray,
        int numPoint,
        int color,      /* 0..255 */
        int weight,      /* 0..32 */
        int drawmode
        );

/*--------------------------------------------------------------------------------------+
|
| Called to check if abort is requested.  Return true if to abort.
|
+--------------------------------------------------------------------------------------*/
typedef bool    (*RGC_AbortFunction)
        (
        RG_Header *pRG              /* => the regions context */
        );


enum RGBoolSelect
    {
    RGBoolSelect_Union               = 0,
    RGBoolSelect_Intersection        = 1,
    RGBoolSelect_Difference          = 2,
    RGBoolSelect_Parity              = 3,
    RGBoolSelect_CountedIntersection = 4,
    RGBoolSelect_GlobalParity        = 5,
    RGBoolSelect_Left                = 6
    } ;



typedef struct _XYRangeTreeNode XYRangeTreeNode;

struct RegionCurveRef
{
int             m_curveId;
int             m_parentCurveId;
int             m_reversed;
double          m_startParam;
double          m_endParam;
};

struct RegionSectorSequence
{
int             m_parentCurveId;    // Curve id for the common parent curve
int             m_numSector;
MTGNodeId       m_nodeId0;          // Node id for first edge
int             m_curveId0;         // First curve id of sequence
int             m_curveId1;         // Last curve id of sequence
double          m_s00;              // Start parameter of first sector
double          m_s01;              // End parameter of first sector
double          m_s10;              // Start parameter of last sector
double          m_s11;              // End parameter of last sector
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RegionSequencer
{
int             m_i0;               // Lower limit of sequence
int             m_i1;               // Upper limit of sequence
int             m_iStart;           // Start position
int             m_step;             // Step within sequence. (Plus or minus 1)
int             m_maxSteps;         // Number of steps allowed
int             m_numStepsTaken;
int             m_currPos;

void Copy (RegionSequencer const& from)
    {
    m_i0            = from.m_i0;
    m_i1            = from.m_i1;
    m_iStart        = from.m_iStart;
    m_step          = from.m_step;
    m_maxSteps      = from.m_maxSteps;
    m_numStepsTaken = from.m_numStepsTaken;
    m_currPos       = from.m_currPos;
    }

void Backup ()
    {
    m_numStepsTaken--;
    m_currPos -= m_step;

    if (m_currPos > m_i1)
        m_currPos = m_i0;
    else if (m_currPos < m_i0)
        m_currPos = m_i1;
    }

bool Advance (int& newPos)
    {
    if (m_numStepsTaken >= m_maxSteps)
        return false;

    newPos = m_currPos;

    m_numStepsTaken++;
    m_currPos += m_step;

    if (m_currPos > m_i1)
        m_currPos = m_i0;
    else if (m_currPos < m_i0)
        m_currPos = m_i1;

    return true;
    }

int RestrictIndex (int i)
    {
    if (i > m_i1)
        return m_i1;
    if (i < m_i0)
        return m_i0;

    return i;
    }

void Init (int i0, int i1, int iStart)
    {
    if (i1 >= i0)
        {
        m_i0   = i0;
        m_i1   = i1;
        m_step = 1;
        }
    else
        {
        m_i0   = i1;
        m_i1   = i0;
        m_step = -1;
        }

    m_maxSteps      = i1 - i0 + 1;
    m_iStart        = RestrictIndex (iStart);
    m_numStepsTaken = 0;
    m_currPos       = iStart;

    if (m_currPos != m_iStart)
        m_maxSteps = 0;
    }

}; // RegionSequencer


END_BENTLEY_GEOMETRY_NAMESPACE

#include "capi/rg_header_capi.h"
#include "capi/rg_loops_capi.h"
#include "capi/rg_inwardparity_capi.h"
#include "capi/rg_intersections_capi.h"


