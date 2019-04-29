/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/
#include <Regions/regionsAPI.h>

#include <Mtg/MtgApi.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*********************************************************************
* Intersection characteristics are marked by bit masks.
* Notes for interpreting:
* RGI_MASK_END_POINT + RGI_MASK_START_POINT implies whole edge.
*   (and RGI_MASK_TANGENCY should be set)
* RGI_MASK_CLOSE_APPROACH says there is a near approach but not
* an intersection.
*
*
*********************************************************************/
#define RGI_GEOMETRIC_BITS      0x0000FFFF
#define RGI_PARSE_BITS          0xFFFF0000

#define RGI_MASK_SIMPLE         0x00000001
#define RGI_MASK_TANGENCY       0x00000002
#define RGI_MASK_CLOSE_APPROACH 0x00000004
#define RGI_MASK_START_POINT    0x00000020
#define RGI_MASK_END_POINT      0x00000040

#define RGI_MASK_PROCESSED_START    0x00000100
#define RGI_MASK_PROCESSED_MASTER   0x00000200
#define RGI_MASK_PROCESSED_END      0x00000300
#define RGI_MASK_PROCESSED_SLAVE    0x00000400

#define RGI_MASK_ALL_PROCESSED_BITS 0x00000F00
#define RGI_GETPROCESSFIELD(mask) ((mask) & RGI_MASK_ALL_PROCESSED_BITS)

#define RGI_MASK_SINGLE_PARAM   0x00000000
#define RGI_MASK_LOW_PARAM      0x00020000
#define RGI_MASK_HIGH_PARAM     0x00030000

#define RGI_ASSEMBLE_MASK(parsePart,geomPart) (parsePart | geomPart)
#define RGI_MASK_GEOMETRY_PART(mask) ((mask) & RGI_GEOMETRIC_BITS)
#define RGI_MASK_PARSE_PART(mask)    ((mask) & RGI_PARSE_BITS)

#define RGI_CLEARMASK(var,mask) ((var) &= ~(mask))
#define RGI_SETMASK(var,mask)  ((var) |= (mask))
#define RGI_GETMASK(var,mask)  ((var) & (mask))
typedef struct
    {
    RIMSBS_Context                          *pContext;
    RGC_GetCurveRangeFunction               getCurveRange;
    RGC_IntersectCurveCurveFunction         intersectCurveCurve;
    RGC_IntersectSegmentCurveFunction       intersectSegmentCurve;
    RGC_EvaluateCurveFunction               evaluateCurve;
    RGC_CreateSubcurveFunction              createSubcurve;
    RGC_SweptCurvePropertiesFunction        sweptCurveProperties;
    RGC_EvaluateDerivativesFunction         evaluateDerivatives;
    RGC_GetClosestXYPointOnCurveFunction    getClosestXYPointOnCurve;
    RGC_AbortFunction                       abortFunction;
    RGC_IntersectCurveCircleXYFunction      intersectCurveCircleXY;
    RGC_IntersectCurvePlaneFunction         intersectCurvePlane;
    RGC_GetGroupIdFunction                  getGroupId;
    RGC_ConsolidateCoincidentGeometryFunction consolidateCoincidentGeometry;
    RGC_AppendAllCurveSamplePointsFunction  appendAllCurveSamplePoints;
    RGC_TransformCurveFunction              transformCurve;
    RGC_TransformAllCurvesFunction          transformAllCurves;
    } RG_Functions;

#define RG_VertexLabelTag 10
#define RG_CurveLabelTag  12
#define RG_ParentLabelTag  14

/* Use mtg CONSTU and CONSTV masks for particular interpretations
**    of hole relationships -- i.e. we assume these graphs
**    do not have rule line data.
*/
#define RG_MTGMASK_IS_NEGATIVE_AREA_FACE        MTG_CONSTU_MASK
#define RG_MTGMASK_FACE_APPEARS_IN_HOLE_ARRAYS  MTG_CONSTV_MASK
#define RG_MTGMASK_BRIDGE_EDGE                  MTG_SECTION_EDGE_MASK
#define RG_MTGMASK_GAP_MASK                     MTG_USEAM_MASK
#define RG_MTGMASK_NULL_FACE                    MTG_VSEAM_MASK


struct _RG_Header
    {
    MTGGraph       *pGraph;             /* The topology part of the graph */
    bool            aborted;            /* Flag to suppress computations. */
    RG_Functions    funcs;              /* Functions to evaluate curves */
    RG_OutputLinestring pOutputLinestringFunc;  /* Debug output function */
    void            *pDebugContext;     /* Context data for debug output func */
    EmbeddedDPoint3dArray *pVertexArray;      /* Vertex coordinates */
    int             vertexLabelIndex;   /* Access to vertex label data on each mtg node */
    int             edgeLabelIndex;     /* Access to edge label data on each mtg node */
    int             parentLabelIndex;   /* Access to parent label data on each mtg node */
    double          relTol;             /* Relative tolerance */
    double          tolerance;          /* Intersection Tolerance */
    double          minimumTolerance;   /* Minimum tolerance.  Set by caller.
                                                Is expanded by small fraction
                                                of data range. */
    XYRangeTreeNode *pEdgeRangeTree;    /* Structure managed by hideTree_XXX */
    XYRangeTreeNode *pFaceRangeTree;    /* Structure managed by hideTree_XXX */
    MTG_MarkSet     markSet;            /* for application use */

    bool             incrementalEdgeRanges;  /* true if each edge is to be entered in the edge
                                                    range tree */
    DRange3d         graphRange;                /* Overall range of data in graph */
    EmbeddedIntArray *pFaceHoleNodeIdArray;  /* node pairs (f0, f1) describing hole.
                                            f0 is an outer face node id, same node as entered
                                                    in the face range tree.
                                            f1 is an inner face (hole) node id, same node
                                                    as entered in the face range tree.
                                            Unless otherwise managed, there is no
                                                sorting.   Given nodeid f2, a linear
                                                search is required to determine
                                                its contained or containing faces.
                                        */
    };

struct _RG_Intersection
    {
    double      param;              /* relevant parameter */
    int         type;               /* start, end, tangency, etc flags.  See RGI_MASK_xxx */
    MTGNodeId  nodeId;              /* node at base of edge */
    int         clusterIndex;       /* Each descriptor is part of a cluster. */
    int         label;              /* scratch label for use during merge */
    MTGNodeId  mergedNodeId;       /* node id at the effective vertex */
    MTGNodeId  seedNodeId;          /* for secondary sorting.  Used by polyline intersect,
                                        not for full merge */
    };

struct _RG_EdgeData
    {
    MTGNodeId  nodeId[2];
    int         vertexIndex[2];
    DPoint3d    xyz[2];
    int         curveIndex;         /* Curve index for calculations.  Set to NULL on linear edge */
    bool        isReversed;
    int         auxIndex;           /* Auxilliary index.   May be same as curve index. */
    };

struct _RG_IntersectionList
    {
    bvector <RG_Intersection> m_intersections;
    bvector<int> m_clusterArray;

    void ClearAll ()
        {
        m_intersections.clear ();
        m_clusterArray.clear ();
        }
    size_t GetIntersectionSize (){return m_intersections.size ();}
    bool GetIntersection (size_t i, RG_Intersection &intersection) const
        {
        if (i >= m_intersections.size ())
            return false;
        intersection = m_intersections[i];
        return true;
        }
    const RG_Intersection *GetIntersectionCPAt (size_t i) const
        {
        if (i < m_intersections.size ())
            return m_intersections.data () + i;
        return NULL;
        }

    RG_Intersection *GetIntersectionPAt (size_t i)
        {
        if (i < m_intersections.size ())
            return m_intersections.data () + i;
        return NULL;
        }



    void AddIntersection (RG_Intersection const &intersection)
        {
        m_intersections.push_back (intersection);
        }
    void SortIntersections (VBArray_SortFunction compare)
        {
        if (m_intersections.size () > 1)
            qsort (m_intersections.data (), (int)m_intersections.size (), sizeof (RG_Intersection), compare);
        }

    void MergeClusters (size_t indexA, size_t indexB)
        {
        if (indexA < m_intersections.size () && indexB < m_intersections.size ())
            {
            int newClusterId = jmdlVArrayInt_mergeClusters (&m_clusterArray,
                                        m_intersections[indexA].clusterIndex,
                                        m_intersections[indexB].clusterIndex);
            m_intersections[indexA].clusterIndex = m_intersections[indexB].clusterIndex = newClusterId;
            }
        }

    void SetSeedNodeIdAt (size_t index, MTGNodeId seedNodeId)
        {
        if (index < m_intersections.size ())
            m_intersections[index].seedNodeId = seedNodeId;
        }

    bool GetSeedNodeIdAt (size_t index, MTGNodeId &seedNodeId) const
        {
        if (index < m_intersections.size ())
            {
            seedNodeId = m_intersections[index].seedNodeId;
            return true;
            }
        return false;
        }
    
    bool GetNodeIdAt (size_t index, MTGNodeId &nodeId) const
        {
        if (index < m_intersections.size ())
            {
            nodeId = m_intersections[index].nodeId;
            return true;
            }
        return false;
        }

    bool GetClusterIndexAt (size_t index, int &clusterIndex) const
        {
        if (index < m_intersections.size ())
            {
            clusterIndex = m_intersections[index].clusterIndex;
            return true;
            }
        return false;
        }

    int NewClusterIndex (){ return jmdlVArrayInt_newClusterIndex(&m_clusterArray);}

    int ResolveClusterIndexAt (size_t intersectionIndex)
        {
        if (intersectionIndex < m_intersections.size ())
            {
            int clusterIndex = m_intersections[intersectionIndex].clusterIndex;
            int newClusterIndex = jmdlVArrayInt_getMergedClusterIndex
                                (
                                &m_clusterArray,
                                clusterIndex
                                );
            m_intersections[intersectionIndex].clusterIndex = newClusterIndex;
            return newClusterIndex;
            }
        return -1;
        }
    };

typedef struct _RG_GapList
    {
    RG_Header               *pRG;
    MTGMask                 mask;
    double                  vertexSize;
    double                  minVertexVertexGap;
    double                  maxVertexVertexGap;
    double                  minVertexEdgeGap;
    double                  maxVertexEdgeGap;
    } RG_GapList;

END_BENTLEY_GEOMETRY_NAMESPACE

#include <Regions/rg_merge.fdf>
#include "privinc/rg_rangetree.h"
#include "privinc/rg_geom.fdf"
#include "privinc/rg_gaps.fdf"
#include "privinc/rg_merge2.fdf"
#include "privinc/rg_fixface.fdf"

