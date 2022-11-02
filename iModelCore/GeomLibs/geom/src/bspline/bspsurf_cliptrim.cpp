/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "msbsplinemaster.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define MAX_BOUNDARY 5000

struct RGElmDescrContext
    {
    RG_Header       *pRG;
    RIMSBS_Context  *pCurves;
    MTG_MarkSet     activeFaces;
    int             mergeFaces;
    /* Bindings for callbacks */
    double  absOffset;
    double  relOffset;
    int     drawMode;
    uint32_t color;
    uint32_t weight;
    //ElemDescrPoolP pElmPool;
    // World-to-plane transforms that exist outside the currtrans.
    // Needed so back-projection to akima element will correctly.
    Transform worldToPlane;
    Transform planeToWorld;
    
    RGElmDescrContext (bool _mergeFaces, double distanceTolerance)    
        {
        pRG = jmdlRG_new ();
        pCurves = jmdlRIMSBS_newContext ();
        mergeFaces = _mergeFaces;
        jmdlRIMSBS_setupRGCallbacks (pCurves, pRG);
        jmdlRG_setFunctionContext (pRG, pCurves);
        jmdlRG_setDistanceTolerance (pRG, distanceTolerance);
        //pElmPool = NULL;

        activeFaces = MTG_MarkSet (jmdlRG_getGraph (pRG), MTG_ScopeFace);
        planeToWorld.InitIdentity ();
        worldToPlane.InitIdentity ();
        }
    ~RGElmDescrContext ()
        {
        // activefaces destructor is called by CPP.
        //jmdlMTGMarkSet_decommission (&pContext->activeFaces);
        jmdlRG_free (pRG);

        jmdlRIMSBS_freeContext (pCurves);

        // Dump pool of RIMSBS object element descriptors...
        //        if (pContext->pElmPool)
        //            mdlElmdscr_freePool (&pContext->pElmPool);

        }
    } ;








static int sDebugSegment = 0;
static double sDuplicateParamTol = 1.0e-8;
// Clerical steps for building up array of BsurfBoundary structures during traversal of planar subdivision
// Boundaries are added in fixed array of MAX_BOUNDARY.  Really don't care much about losing them if you have more.
class BoundaryBuilder
{
    int                 mNumBoundary;
    BsurfBoundary       mBoundary[MAX_BOUNDARY];
    int                 mCurrBoundaryIndex;

    DPoint3d mXYZBuffer[MAX_VERTICES];
    int      mNumXYZ;
public:

// COPY completed boundaries directly from local buffer to caller's buffer.
// Return number of boundaries copied.
// numExpected must be EXACT match for number in local buffer.
int GrabBoundaries
(
BsurfBoundary *pDest,
int numExpected
)
    {
    if (numExpected != mNumBoundary || numExpected == 0)
        return 0;

    // Boundaries were built as cyclic lists.
    for (int i = 0; i < mNumBoundary; i++)
        bspTrimCurve_breakCyclicList (&mBoundary[i].pFirst);

    memcpy (pDest, mBoundary, mNumBoundary * sizeof (BsurfBoundary));
    return mNumBoundary;
    }

int GetBoundaryCount ()
    {
    return mNumBoundary;
    }
BoundaryBuilder (int dummy)
    {
    mNumBoundary = 0;
    mNumXYZ = 0;
    mCurrBoundaryIndex = -1;
    memset (mBoundary, 0, MAX_BOUNDARY * sizeof (BsurfBoundary));
    }

bool IsActive ()
    {
    return mCurrBoundaryIndex >= 0 && mCurrBoundaryIndex < mNumBoundary;
    }

void BeginBoundary ()
    {
    mCurrBoundaryIndex = mNumBoundary++;
    }

void FlushLinearBuffer ()
    {
    if (!IsActive ())
        return;
    if (mNumXYZ > 1)
        {
        MSBsplineCurve curve;
        if (SUCCESS == bspconv_lstringToCurveStruct (&curve, mXYZBuffer, mNumXYZ))
            {
            mNumXYZ = 0;
            AddCurve (curve);
            }
        }
    mNumXYZ = 0;
    }

void AddLinear (DPoint3d &xyzCurr)
    {
    if (!IsActive ())
        return;

    if (   mNumXYZ > 0
        && xyzCurr.DistanceXY (mXYZBuffer[mNumXYZ - 1]) < sDuplicateParamTol)
        return;

    if (mNumXYZ >= MAX_VERTICES)
        {
        DPoint3d xyzSave = mXYZBuffer[mNumXYZ - 1];
        FlushLinearBuffer ();
        mXYZBuffer[0] = xyzSave;
        mNumXYZ = 1;
        }
    mXYZBuffer [mNumXYZ++] = xyzCurr;
    }

void AddCurve (MSBsplineCurve &curve)
    {
    if (!IsActive ())
        return;
    FlushLinearBuffer ();
    bspTrimCurve_allocateAndInsertCyclic (&mBoundary[mCurrBoundaryIndex].pFirst, &curve);
    }

};

static void CollectActiveFaceLoops
(
RGElmDescrContext *pContext,
BoundaryBuilder &bb
)
    {
    MTGNodeId seedNodeId;
    MTGGraph*           pGraph = jmdlRG_getGraph (pContext->pRG);

    EmbeddedIntArray *pLoopArray = jmdlEmbeddedIntArray_grab ();
    jmdlRG_collectOuterAndInnerFaces (pContext->pRG, pLoopArray, &pContext->activeFaces, true, true);
    
    for (int ib = 0; jmdlEmbeddedIntArray_getInt (pLoopArray, &seedNodeId, ib); ib++)
        {
        if (seedNodeId == MTG_NULL_NODEID)
            continue;
        bb.BeginBoundary ();
        MTGARRAY_FACE_LOOP (edgeNodeId, pGraph, seedNodeId)
            {
            if (sDebugSegment)
                printf ("     (edge %d)\n", edgeNodeId);
            int currCurveId, currParentId;
            double currStartFraction, currEndFraction;
            bool currCurveIsReversed;
            DPoint3d point0, point1;
            if (jmdlRG_getCurveData (pContext->pRG, &currCurveId, &currCurveIsReversed, &point0, &point1, edgeNodeId)
	        && currCurveId != RG_NULL_CURVEID
	        && jmdlRIMSBS_getCurveInterval (
				        pContext->pCurves,
				        &currParentId,
				        &currStartFraction,
				        &currEndFraction,
				        currCurveId,
				        currCurveIsReversed
				        ))
                {
                MSBsplineCurve curve;
                if (jmdlRIMSBS_getMappedMSBsplineCurve (pContext->pCurves, &curve, currParentId, currStartFraction, currEndFraction))
	            {
                    if (sDebugSegment)
                        printf (" (curve %d s0 %10.6lg s1 %10.6lg order %d)\n",
                                currParentId, currStartFraction, currEndFraction,
                                curve.params.order);
                    bb.AddCurve (curve);
                    }
                else
                    {
                    if (sDebugSegment)
                        printf (" (curve FAIL %d s0 %10.6lg s1 %10.6lg)\n",
                                currParentId, currStartFraction, currEndFraction);
                    }
                }
            else
                {
                DPoint3d xyz0, xyz1;
                jmdlRG_getVertexData (pContext->pRG, &xyz0, 0, NULL, edgeNodeId, 0.0);
                jmdlRG_getVertexData (pContext->pRG, &xyz1, 0, NULL, jmdlMTGGraph_getFSucc (pGraph, edgeNodeId), 0.0);
                if (sDebugSegment)
                    printf (" (out segment (%lg,%lg) (%lg,%lg)\n",
                                    xyz0.x, xyz0.y,
                                    xyz1.x, xyz1.y);
                bb.AddLinear (xyz0);
                bb.AddLinear (xyz1);
                }
            }
        MTGARRAY_END_FACE_LOOP (edgeNodeId, pGraph, seedNodeId)
        bb.FlushLinearBuffer ();
        }

    jmdlEmbeddedIntArray_drop (pLoopArray);
    }
/*----------------------------------------------------------------------+
@description Intersect trim curve region with clip polygon
@param pBoundaries IN boundaries to split
@param numBoundary IN num input boundaries.
@param ppNewBoundaries OUT new boundaries (allocated via dlmSystem)
@param pNumNewBoundaries OUT number of new boundaries
@param pPolygonPoints IN array of polygon points.   ASSUMED to have closure point.
@param numPolygonPoints IN number of points in clip polygon.
+----------------------------------------------------------------------*/
Public StatusInt bspsurf_clipTrimLoops
(
BsurfBoundary    *pBoundaries,
int              numBoundary,
BsurfBoundary    **ppNewBoundaries,
int              *pNumNewBoundaries,
DPoint3d         *pPolygonPoints,
int              numPolygonPoints
)
    {
    int curveCounter = 0;
    bool bMergeHoles = true;
    static double sDistanceTolerance = 1.0e-6;
    double gapTolerance = sDistanceTolerance;

    if (NULL != ppNewBoundaries)
        *ppNewBoundaries = NULL;
    
    if (NULL != pNumNewBoundaries)
        *pNumNewBoundaries = 0;

    RGElmDescrContext context (bMergeHoles, sDistanceTolerance);

    jmdlRIMSBS_setCurrGroupId (context.pCurves, 0);
    for (int i = 0; i < numBoundary; i++)
        {
        for (TrimCurve *pCurr = pBoundaries[i].pFirst; NULL != pCurr; pCurr = pCurr->pNext)
            {
            MSBsplineCurve localCurve;  // RIMSBS will free the (copied) contents at end.
            int curveId;
            DPoint3d xyz0, xyz1;
            bspcurv_evaluateCurvePoint  (&xyz0, NULL, &pCurr->curve, 0.0);
            bspcurv_evaluateCurvePoint  (&xyz1, NULL, &pCurr->curve, 1.0);
            bspcurv_copyCurve (&localCurve, &pCurr->curve);
            if (sDebugSegment)
                {
                DPoint3d xyz;
                double s;
                int k;
                if (pCurr->curve.params.order == 2)
                    {
                    for (int i = 0; i < pCurr->curve.params.numPoles; i++)
                        {
                        if (pCurr->curve.rational)
                            printf ("(pole %d  (%lg,%lg)\n", i,
                                        pCurr->curve.poles[i].x / pCurr->curve.weights[i],
                                        pCurr->curve.poles[i].y / pCurr->curve.weights[i]
                                        );
                        else
                            printf ("(pole %d (%lg,%lg)\n", i,
                                        pCurr->curve.poles[i].x, pCurr->curve.poles[i].y);
                        }
                    }
                else
                    {
                    int numSample = (pCurr->curve.params.numPoles - pCurr->curve.params.order + 1) * pCurr->curve.params.order;
                    for (k = 0; k <= numSample ; k++)
                        {
                        s = (double)k / (double)numSample;
                        bspcurv_evaluateCurvePoint  (&xyz, NULL, &pCurr->curve, s);
                        printf ("(s=%10.5g) (uv=%10.6g,%10.6g)\n", s, xyz.x, xyz.y);
                        }
                    }
                }
            curveId = jmdlRIMSBS_addMSBsplineCurve (context.pCurves,
                                curveCounter, NULL, &localCurve);
            jmdlRG_addCurve (context.pRG, curveId, curveId);
            curveCounter++;
            }
        }

    jmdlRIMSBS_setCurrGroupId (context.pCurves, 1);
    int polygonCurveId = jmdlRIMSBS_addDataCarrier (context.pCurves, curveCounter, NULL);
    jmdlRG_addLinear (context.pRG, pPolygonPoints, numPolygonPoints, false, polygonCurveId);

    if (sDebugSegment)
        {
        int i;
        printf ("(ClipPolygon \n");
        for (i = 0; i < numPolygonPoints; i++)
            {
            printf ("(uv=%9.6g,%9.6g)\n", pPolygonPoints[i].x, pPolygonPoints[i].y);
            }
        printf (")\n");
        }

    jmdlRG_mergeWithGapTolerance
		(
		context.pRG,
		gapTolerance,
		gapTolerance
		);


    jmdlRG_buildFaceRangeTree
		    (
		    context.pRG,
		    0.0,
		    jmdlRG_getTolerance (context.pRG)
		    );
    if  (bMergeHoles)
	jmdlRG_buildFaceHoleArray (context.pRG);


    //MTGGraph*           pGraph = jmdlRG_getGraph (context.pRG);
    BoundaryBuilder bb (0);
    if (jmdlRG_collectBooleanFaces (context.pRG, RGBoolSelect_Intersection, 0, &context.activeFaces))
        {
        CollectActiveFaceLoops (&context, bb);
        int n = bb.GetBoundaryCount ();
        if (n > 0)
            {
            *ppNewBoundaries = (BsurfBoundary*)dlmSystem_mdlCalloc (n, sizeof (BsurfBoundary));
            *pNumNewBoundaries = bb.GrabBoundaries (*ppNewBoundaries, n);
            }
        }

    return SUCCESS;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
