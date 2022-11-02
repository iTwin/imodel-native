/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static bool    computeLaplacianParameterDelta
(
VuSetP pGraph,
VuP     pNode,
DPoint3d *pDelta,
double &minEdgeLength,
double &maxEdgeLength,
VuScalarFunction *pFaceWeightFunction
)
    {
    DPoint3d currUV;
    DPoint3d currUVDelta;
    DPoint3d sumUVDelta;
    DPoint3d averageDelta;
    double currArea;
    double totalArea = 0.0;
    int currPos, currNeg;
    int count;
    int negativeAreaCount = 0;
    bool    boolstat = false;
    count = 0;
    sumUVDelta.Zero ();
    currUVDelta.z = 0.0;
    minEdgeLength = maxEdgeLength = vu_edgeLengthXY (pNode);
    VU_VERTEX_LOOP (pCurr, pNode)
        {
        vu_getDPoint3d (&currUV, pCurr);
        double a = vu_edgeLengthXY (pCurr);
        if (a > maxEdgeLength)
            maxEdgeLength = a;
        if (a < minEdgeLength)
            minEdgeLength = a;
        vu_centroidFromOrigin
                (
                (DPoint2d*)&currUVDelta,
                &currArea,
                &currPos,
                &currNeg,
                (DPoint2d*)&currUV,
                pCurr
                );
        if (NULL != pFaceWeightFunction)
            currArea *= pFaceWeightFunction->Evaluate (pGraph, pCurr);
        totalArea += currArea;
        if (currArea < 0.0)
            negativeAreaCount++;
        count++;
        sumUVDelta.SumOf (sumUVDelta,currUVDelta, currArea);
        }
    END_VU_VERTEX_LOOP (pCurr, pNode)

    if (count > 1 && negativeAreaCount == 0 && totalArea > 0.0)
        {
        averageDelta.Scale (sumUVDelta, 1.0 / totalArea);
        if (pDelta)
            {
            *pDelta = averageDelta;
            boolstat = true;
            }
        }
    return boolstat;
    }

Public GEOMDLLIMPEXP void vu_graphRange
(
VuSetP pGraph,
DRange3d *pRange
)
    {
    pRange->Init ();
    VU_SET_LOOP (pNode, pGraph)
        {
        DPoint3d xyz;
        vu_getDPoint3d (&xyz, pNode);
        pRange->Extend (xyz);
        }
    END_VU_SET_LOOP (pNode, pGraph)
    }

Public GEOMDLLIMPEXP double vu_largestRangeEdge
(
VuSetP pGraph
)
    {
    DRange3d range;
    vu_graphRange (pGraph, &range);
    double a = range.high.x - range.low.x;
    double b = range.high.y - range.low.y;
    return a > b ? a : b;
    }

Public void vu_collectUnmaskedVertices (VuSetP pGraph, bvector<VuP> &candidates, VuMask visitMask, VuMask barrierMask)
    {
    vu_clearMaskInSet (pGraph, visitMask);
    candidates.clear ();
    VU_SET_LOOP (node, pGraph)
        {
        if (!vu_getMask (node, visitMask))
            {
            vu_setMaskAroundVertex (node, visitMask);
            if (!vu_findMaskAroundVertex (node, barrierMask))
                candidates.push_back (node);
            }
        }
    END_VU_SET_LOOP (node, pGraph)
    }

Public GEOMDLLIMPEXP bool    vu_smoothInteriorVertices
(
VuSetP pGraph,
VuScalarFunction *pFaceWeightFunction,
VuCoordinateMappingFunction *pMappingFunction,
double relTol,
int    maxSweep,
int    flipInterval,
int    maxFlipsPerEdgePerPass,
double *pShiftFraction,
int   *pNumSweep
)
    {
    bool    converged = false;
    if (relTol > 0.1)
        relTol = 0.1;
    if (relTol < 1.0e-8)
        relTol = 1.0e-8;
    double tol = relTol * vu_largestRangeEdge (pGraph);

    bvector <VuP> candidates;
    VuMask visitMask = vu_grabMask (pGraph);
    VuMask barrierMask = VU_ALL_FIXED_EDGES_MASK;     // Aug 20 2014 change from VU_EXTERIOR_EDGE | VU_BOUNDARY_EDGE;
    vu_collectUnmaskedVertices (pGraph, candidates, visitMask, barrierMask);

    static double s_maxShiftFraction = 0.25;
    size_t numCandidate = candidates.size ();
    int numSweep = 0;
    int numSweepSinceFlip = 0;
    double maxShift = 0.0;
    while (!converged && numSweep < maxSweep)
        {
        numSweep++;
        maxShift = 0.0;
        numSweepSinceFlip++;
        if (numSweepSinceFlip > flipInterval)
            {
            numSweepSinceFlip = 0;
            if (pMappingFunction)
                vu_flipTrianglesToImproveMappedCoordinateAspectRatio (pGraph, pMappingFunction, maxFlipsPerEdgePerPass);
            else
                vu_flipTrianglesToImproveQuadraticAspectRatio (pGraph);
            vu_collectUnmaskedVertices (pGraph, candidates, visitMask, barrierMask);            
            }
        for (size_t i = 0; i < numCandidate; i++)
            {
            DPoint3d delta;
            VuP pNode = candidates[i];
            double maxEdgeLength, minEdgeLength;
            if (computeLaplacianParameterDelta (pGraph, pNode, &delta, minEdgeLength, maxEdgeLength, pFaceWeightFunction))
                {
                double aMax = s_maxShiftFraction * minEdgeLength;
                double a = delta.MaxAbs ();
                if (a > maxShift)
                   maxShift = a;
                double fraction = 1.0;
                if (a > aMax)
                    fraction = aMax / a;
                DPoint3d xyz;
                vu_getDPoint3d (&xyz, pNode);
                xyz.x += fraction * delta.x;
                xyz.y += fraction * delta.y;
                vu_setDPoint3dAroundVertex (pNode, &xyz);
                }
            }
        converged = maxShift < tol;
        }
    vu_returnMask (pGraph, visitMask);
    if (pShiftFraction)
        *pShiftFraction = maxShift / tol;
    if (pNumSweep)
        *pNumSweep = numSweep;
    return converged;
    }

Public GEOMDLLIMPEXP bool    vu_smoothInteriorVertices
(
VuSetP pGraph,
double relTol,
int    maxSweep,
int    flipInterval,
double *pShiftFraction,
int   *pNumSweep
)
    {
    return vu_smoothInteriorVertices (pGraph, NULL, NULL, relTol, maxSweep, flipInterval, 40, pShiftFraction, pNumSweep);
    }

END_BENTLEY_GEOMETRY_NAMESPACE