/*--------------------------------------------------------------------------------------+
|
|     $Source: vu/src/vuperiodicconnect.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <math.h>
#include <vector>
#include <algorithm>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

struct PCSortData
{
VuP scanNode;
double scanX;
double scanY;
int scanType;
VuP barrierNode;
double distance;
double fraction;
};
/*-----------------------------------------------------------------*//**
* Unpack PCSortData to scalar data.
* Undo scan node vsucc shift.
+---------------+---------------+---------------+---------------+------*/
static void bd_copyFromGP
(
const PCSortData *pGP,
VuP             *ppScanNode,
double          *pScanX,
double          *pScanY,
int             *pScanType,
VuP             *ppBarrierNode,
double          *pDistance,
double          *pFraction
)
    {
    if (ppScanNode)
        *ppScanNode      = vu_vpred(pGP->scanNode);
    if (pScanX)
        *pScanX         = pGP->scanX;
    if (pScanY)
        *pScanY         = pGP->scanY;
    if (pScanType)
        *pScanType      = pGP->scanType;
    if (ppBarrierNode)
        *ppBarrierNode   = pGP->barrierNode;
    if (pDistance)
        *pDistance      = pGP->distance;
    if (pFraction)
        *pFraction      = pGP->fraction;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     02/2006
+---------------+---------------+---------------+---------------+---------------+------*/
bool cb_compareForBarrierSort
(
const PCSortData &gp0,
const PCSortData &gp1
)
    {
    VuP pBarrier0, pBarrier1;
    double fraction0, fraction1;
    double distance0, distance1;

    bd_copyFromGP (&gp0, NULL, NULL, NULL, NULL, &pBarrier0, &distance0, &fraction0);
    bd_copyFromGP (&gp1, NULL, NULL, NULL, NULL, &pBarrier1, &distance1, &fraction1);

    if (pBarrier0 == pBarrier1)
        return (fraction0 < fraction1) || (fraction0 == fraction1 && distance0 < distance1);

    if (vu_below (pBarrier0, pBarrier1))
        return true;
    if (vu_below (pBarrier1, pBarrier0))
        return false;

    /* Ummm ... multiple sectors at barrier base vertex */
    return (size_t) pBarrier0 < (size_t) pBarrier1;
    }

/*=================================================================================**//**
* @bsiclass                                                     Earlin.Lutz     02/2006
+===============+===============+===============+===============+===============+======*/
class VuSortArray
{
public:
bvector <PCSortData> mArray;

VuSortArray () :
    mArray ()
    {
    }

void Add (PCSortData &gp)
    {
    mArray.push_back (gp);
    }

size_t Size ()
    {
    return mArray.size ();
    }

bool Get (PCSortData &value, int index)
    {
    if (index >= 0 && index < (int) mArray.size ())
        {
        value = mArray[index];
        return true;
        }
    return false;
    }

void Sort ()
    {
    if (mArray.size() > 1)
        std::sort (mArray.begin (), mArray.end (), cb_compareForBarrierSort);
    }
};

typedef enum
    {
    ScanType_None = 0,
    ScanType_Peak = 1,
    ScanType_Icicle = 2,
    ScanType_LeftWall = 3
    } ScanType;

typedef struct
    {
    VuMask lexicalMask;
    VuMask strictMask;
    double xPeriod;
    double yPeriod;
    double maxXEdgeLength;
    double maxYEdgeLength;
    double minXEdgeLength;
    int maxJoin;
    int numJoin;
    int trapCounter;
    VuMask inactiveFaceMask;
    } ExtremaParams;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     02/2006
+---------------+---------------+---------------+---------------+---------------+------*/
static double periodicDifference
(
double a1,
double a0,
double period
)
    {
    double delta = a1 - a0;
    if (period == 0.0)
        return delta;
    else
        return bsiTrig_normalizeToPeriod (delta, -0.5 * period, period);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     02/2006
+---------------+---------------+---------------+---------------+---------------+------*/
static void vu_getPeriodicVector
(
DPoint3d *pVector,
VuP pNode0,
VuP pNode1,
double xPeriod,
double yPeriod
)
    {
    DPoint3d xyz0, xyz1;
    vu_getDPoint3d (&xyz0, pNode0);
    vu_getDPoint3d (&xyz1, pNode1);
    pVector->x = periodicDifference (xyz1.x, xyz0.x, xPeriod);
    pVector->y = periodicDifference (xyz1.y, xyz0.y, yPeriod);
    pVector->z = xyz1.z - xyz0.z;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     02/2006
+---------------+---------------+---------------+---------------+---------------+------*/
static void vu_getPeriodicEdgeVector
(
DPoint3d *pVector,
VuP pNode0,
double xPeriod,
double yPeriod
)
    {
    vu_getPeriodicVector (pVector, pNode0, vu_fsucc(pNode0), xPeriod, yPeriod);
    }

/*-----------------------------------------------------------------*//**
* Test if vector is (Y upward) OR (Y level and X upward)
+---------------+---------------+---------------+---------------+------*/
static bool    isLexicalUpVector
(
DPoint3d *pDelta
)
    {
    return pDelta->y > 0.0 || (pDelta->y == 0.0 && pDelta->x > 0.0);
    }

/*-----------------------------------------------------------------*//**
* Test if vector is (Y upward)
+---------------+---------------+---------------+---------------+------*/
static bool    isStrictUpVector
(
DPoint3d *pDelta
)
    {
    return pDelta->y > 0.0;
    }

/*-----------------------------------------------------------------*//**
* Pack scalar data into PCSortData.
* scan node is shifted to vsucc to make it impervious to join anomalies.
+---------------+---------------+---------------+---------------+------*/
static void bd_copyToGP
(
PCSortData *pGP,
VuP             pScanNode,
double          scanX,
double          scanY,
int             scanType,
VuP             pBarrierNode,
double          distance,
double          fraction
)
    {
    pGP->scanX = scanX;
    pGP->scanY = scanY;
    pGP->distance = distance;
    pGP->scanType = scanType;
    pGP->fraction = fraction;
    pGP->barrierNode = pBarrierNode;
    pGP->scanNode = vu_vsucc (pScanNode);
    }

/*-----------------------------------------------------------------*//**
* @description Read barrier entry from GPA.
* @return true if data accessed.
* @bsihdr                       EarlinLutz      09/02
+---------------+---------------+---------------+---------------+------*/
static bool    bd_readBarrierEntryFromGPA
(
VuSortArray                 &sorter,
VuP                         *ppScanNode,
double                      *pScanX,
double                      *pScanY,
int                         *pScanType,
VuP                         *ppBarrierNode,
double                      *pDistance,
double                      *pFraction,
int                         i0
)
    {
    PCSortData gp;
    if (!sorter.Get (gp, i0))
        return false;
    bd_copyFromGP (&gp, ppScanNode, pScanX, pScanY, pScanType, ppBarrierNode, pDistance, pFraction);
    return true;
    }

/*-----------------------------------------------------------------*//**
* Walk the vertex at pNode0.  Return the sector in which pNode1 is visible.
* @bsihdr                       EarlinLutz      09/02
+---------------+---------------+---------------+---------------+------*/
static VuP selectVisibleSector
(
VuSetP pGraph,
ExtremaParams *pParams,
VuP pNode0,
VuP pNode1
)
    {
    DPoint3d delta01, delta0A, delta0B;
    VuP pNodeB;
    vu_getPeriodicVector (&delta01, pNode0, pNode1, pParams->xPeriod, pParams->yPeriod);
    if (vu_findNodeAroundVertex (pNode0, pNode1))
        return NULL;

    if (vu_vsucc(pNode0) == pNode0)
        {
        // Consider pNode0 visible from anywhere except its (single) neighbor vertex
        VuP pNodeC = vu_fsucc (pNode0);
        VU_VERTEX_LOOP (pNodeA, pNodeC)
            {
            if (pNodeA == pNode1)
                return NULL;
            }
        END_VU_VERTEX_LOOP (pNodeA, pNodeC)
        return pNode0;
        }
    else
        {
        vu_getPeriodicEdgeVector (&delta0A, pNode0, pParams->xPeriod, pParams->yPeriod);
        VU_VERTEX_LOOP (pNodeA, pNode0)
            {
            pNodeB = vu_vsucc (pNodeA);
            vu_getPeriodicEdgeVector (&delta0B, pNodeB, pParams->xPeriod, pParams->yPeriod);
            if (bsiDPoint3d_isVectorInCCWXYSector (&delta01, &delta0A, &delta0B))
                return pNodeA;
            delta0A = delta0B;
            }
        END_VU_VERTEX_LOOP (pNodeA, pNode0)
        }
    return NULL;
    }


/*-----------------------------------------------------------------*//**
*
* @return number of barrier edges encountered.
* @bsihdr                                                                       EarlinLutz      09/02
+---------------+---------------+---------------+---------------+------*/
static int scanToPeriodicRightBarrier
(
VuSetP pGraph,
VuP pScanNode,
int scanType,
PCSortData *pGP,
ExtremaParams *pParams
)
    {
    DPoint3d xyz0, delta;
    DPoint3d xyzScan;
    int numHit;
    double fraction, xHit, dxHit, y0, dxMin;
    VuP pVisibleSectorAtBarrier;
    static double s_specialFractionTol = 2.0e-3;

    vu_getDPoint3d (&xyzScan, pScanNode);

    dxMin = 0.0;  /* Just to initialize it. numHit=0 says this is undefined. */
    numHit = 0;
    VU_SET_LOOP (pNode0, pGraph)
        {
        if (vu_getMask (pNode0, pParams->lexicalMask))
            {
            vu_getDPoint3d (&xyz0, pNode0);
            vu_getPeriodicEdgeVector (&delta, pNode0, pParams->xPeriod, pParams->yPeriod);
            y0 = bsiTrig_normalizeToPeriodAroundZero (xyzScan.y - xyz0.y, pParams->yPeriod);
            /* A scan line "just below" xyz0.y may register negative here but not
                within bounds of the edge from below.
                Look for this condition and force it to be considered a hit:
                */
            if (   y0 < 0.0
                && fabs (y0) < s_specialFractionTol * delta.y
                && vu_getMask (pNode0, pParams->strictMask)
                && vu_getMask (vu_fpred(pNode0), pParams->strictMask)
                )
                {
                y0 = 0;
                }
            if (y0 >= 0 && y0 < delta.y)
                {
                /* From normalization, y0 >= 0.  From test, y0 <= delta.y. */
                fraction = y0 / delta.y;
                xHit = xyz0.x + fraction * delta.x;
                dxHit = bsiTrig_normalizeToPeriod (xHit - xyzScan.x, 0.0, pParams->xPeriod);
                pVisibleSectorAtBarrier = selectVisibleSector
                            (
                            pGraph,
                            pParams,
                            pNode0,
                            pScanNode
                            );
                if (   NULL != pVisibleSectorAtBarrier
                   &&  dxHit > pParams->minXEdgeLength
                   &&  (numHit == 0 || dxHit < dxMin)
                   )
                    {
                    bd_copyToGP (pGP, pScanNode, xyzScan.x, xyzScan.y, scanType,
                        pVisibleSectorAtBarrier,
                        dxHit, fraction);
                    dxMin = dxHit;
                    numHit++;
                    }
                }
            }
        }
    END_VU_SET_LOOP (pNode0, pGraph)


    return numHit;
    }

/*-----------------------------------------------------------------*//**
* @description Set a mask to mark all edges that are "interior" (according to mask)
*       and "upward" by periodic coordinates.
*
* @bsihdr                                                                       EarlinLutz      09/02
+---------------+---------------+---------------+---------------+------*/
static void markPeriodicUpEdges
(
VuSetP pGraph,
VuMask mask,
double xPeriod,
double yPeriod,
bool    (*isUpVector)(DPoint3d *),
VuMask inactiveFaceMask
)
    {
    DPoint3d vector;
    VuMask skipMask = inactiveFaceMask;
    vu_clearMaskInSet (pGraph, mask);
    VU_SET_LOOP (pBase, pGraph)
        {
        if (!vu_getMask (pBase, skipMask))
            {
            vu_getPeriodicEdgeVector (&vector, pBase, xPeriod, yPeriod);
            if (isUpVector (&vector))
                vu_setMask (pBase, mask);
            }
        }
    END_VU_SET_LOOP (pBase, pGraph)
    }

/*-----------------------------------------------------------------*//**
+---------------+---------------+---------------+---------------+------*/
static void collectRightBarrierScans
(
VuSetP          pGraph,
VuSortArray&    sorter,
int             (*testFunc)(VuSetP, VuP, ExtremaParams *),
ExtremaParams  *pParams,
VuMask          inactiveFaceMask
)
    {
    int scanType;
    int numHit;
    PCSortData gp;
    VU_SET_LOOP (pScanNode, pGraph)
        {
        if (!vu_getMask (pScanNode, inactiveFaceMask))
            {
            scanType = testFunc (pGraph, pScanNode, pParams);
            if (scanType != ScanType_None)
                {
                numHit = scanToPeriodicRightBarrier (pGraph, pScanNode, scanType, &gp, pParams);
                if (numHit > 0)
                    sorter.Add (gp);
                }
            }
        }
    END_VU_SET_LOOP (pScanNode, pGraph)
    }


/*-----------------------------------------------------------------*//**
* @return 0 if the node is not an exposed extrema.
*         1 if the node is a peak (points up)
*        -1 if the node is an icicle (points down)
* @bsihdr                       EarlinLutz      09/02
+---------------+---------------+---------------+---------------+------*/
static  int computeScanType
(
VuSetP pGraph,
VuP    pNode,
ExtremaParams *pParams
)
    {
    VuP pPred;
    DPoint3d vector0, vector1;
    double cross;
    static bool    s_bNoScanFromExteriorEdge = 0;
    VuMask mask0, mask1;
    if (s_bNoScanFromExteriorEdge && vu_getMask (pNode, VU_EXTERIOR_EDGE))
        return ScanType_None;
    pPred = vu_fpred (pNode);
    mask0 = vu_getMask (pPred, pParams->lexicalMask);
    mask1 = vu_getMask (pNode, pParams->lexicalMask);
    vu_getPeriodicEdgeVector (&vector0, pPred, pParams->xPeriod, pParams->yPeriod);
    vu_getPeriodicEdgeVector (&vector1, pNode, pParams->xPeriod, pParams->yPeriod);
    if (mask0 != mask1)
        {
        if (vu_vsucc (pNode) == pNode)
            {
            if (vector1.y > 0.0)
                return ScanType_Icicle;
            else if (vector1.y < 0.0)
                return ScanType_Peak;
            }
        else
            {
            cross = bsiDPoint3d_crossProductXY (&vector0, &vector1);
            if (cross < 0.0)
                {
                return mask0 ? ScanType_Peak : ScanType_Icicle;
                }
            }
        }
    else
        {
        /* Same mask.  If same at ZERO, we have down edges.
            If furthermore STRICTLY down, call it a wall.
        */
        if (   !mask0
            && (  (vector0.y < 0.0 && vector1.y < 0.0)  /* Simple wall */
               || (vector0.y < 0.0 && vector1.y == 0.0) /* Approach from left, then up */
               )
            )
            return ScanType_LeftWall;
        }
    return ScanType_None;
    }
#ifdef CompileAll
static bool    checkSameFace
(
VuSetP pGraph,
VuP pNode0,
VuP pNode1
)
    {
    VU_FACE_LOOP (pCurr, pNode0)
        {
        if (pCurr == pNode1)
            return true;
        }
    END_VU_FACE_LOOP (pCurr, pNode0)
    return false;
    }
#endif

/*-----------------------------------------------------------------*//**
* @param bIsVerticalJoin IN true if join is vertical join between islands.
*           (false is horizontal join to barrier edge.)
*       For horizontal join, it is assumed that the node0 is the right barrier,
*       node1 is the scan origin.  This affects edge length testing.
* @return false if edge length violates restrictions.
* @bsihdr                       EarlinLutz      09/02
+---------------+---------------+---------------+---------------+------*/
static bool    bd_join
(
VuSetP pGraph,
ExtremaParams *pParams,
bool    bIsVerticalJoin,
VuP     pNode0,
VuP     pNode1,
VuP     *ppNewNode0,
VuP     *ppNewNode1
)
    {
    DPoint3d xyz0, xyz1, delta;
    VuP pNode0A, pNode1A;

    if (pParams->maxJoin > 0 && pParams->numJoin >= pParams->maxJoin)
        return false;

    *ppNewNode0 = *ppNewNode1 = NULL;
    if (  pParams->maxXEdgeLength > 0.0
       || pParams->maxYEdgeLength > 0.0
       )
        {
        vu_getDPoint3d (&xyz0, pNode0);
        vu_getDPoint3d (&xyz1, pNode1);
        bsiDPoint3d_subtractDPoint3dDPoint3d (&delta, &xyz0, &xyz1);
        if (bIsVerticalJoin)
            {
            /* Normalize for centered stroke.
               I'm have trouble thinking about this one.
            */
            delta.x = bsiTrig_normalizeToPeriod (delta.x, -0.5 * pParams->xPeriod, pParams->xPeriod);
            delta.y = bsiTrig_normalizeToPeriod (delta.y, -0.5 * pParams->yPeriod, pParams->yPeriod);
            }
        else
            {
            /* Normalize for strict left-to-right stroke. */
            delta.x = bsiTrig_normalizeToPeriod (delta.x, 0.0, pParams->xPeriod);
            delta.y = bsiTrig_normalizeToPeriodAroundZero (delta.y, pParams->yPeriod);
            }

        if (pParams->maxXEdgeLength > 0.0 && fabs (delta.x) > pParams->maxXEdgeLength)
            return false;
        if (pParams->maxYEdgeLength > 0.0 && fabs (delta.y) > pParams->maxYEdgeLength)
            return false;

        if (delta.x <= pParams->minXEdgeLength)
            return false;
        }

    //checkSameFace (pGraph, pNode0, pNode1);
    /* Verify that there is a line of sight between these two nodes.
        Umm, this really should have been part of the scan testing, bad stuff happens
        in late passes.
    */
    pNode0A = selectVisibleSector (pGraph, pParams, pNode0, pNode1);
    pNode1A = selectVisibleSector (pGraph, pParams, pNode1, pNode0);
    if (!pNode0A || !pNode1A)
        return false;
    vu_join (pGraph, pNode0A, pNode1A, ppNewNode0, ppNewNode1);
    pParams->numJoin++;
    //vu_postGraphToTrapFunc (pGraph, "periodic Triangulate", 0, pParams->trapCounter++);
    return true;
    }

/*-----------------------------------------------------------------*//**
* @return index where records for the next barrier begin.
* @bsihdr                       EarlinLutz      09/02
+---------------+---------------+---------------+---------------+------*/
static int bd_joinToBarrier
(
VuSetP pGraph,
VuSortArray &sorter,
int baseIndex,
ExtremaParams *pParams
)
    {
    VuP pEdgeNode0, pEdgeNode1;
    VuP pEdgeTop0;
    VuP pScanNode0, pScanNode1;
    double scanX0, scanX1;
    double scanY0, scanY1;
    double fraction0, fraction1;
    VuP pNewNodeA, pNewNodeB;
    VuP pPreviousConnectNode = NULL;
    int currIndex = baseIndex;
    int scanType0, scanType1;

    if (!bd_readBarrierEntryFromGPA
                (
                sorter,
                &pScanNode0, &scanX0, &scanY0, &scanType0,
                &pEdgeNode0, NULL, &fraction0, baseIndex))
        return 0;

    pEdgeTop0 = vu_fsucc(pEdgeNode0);

    /* EDL Sept 18, 2002 ... Added tests in bd_join to prevent long edges.
        At this caller level, watch the return of bd_join carefully and avoid
        reference to new nodes unless confirmed.   Hope this doesn't cause problems
        in labeling conditions.
    */
    if (!bd_readBarrierEntryFromGPA
                (
                sorter,
                &pScanNode1, &scanX1, &scanY1, &scanType1,
                &pEdgeNode1, NULL, &fraction1, baseIndex + 1)
        || pEdgeNode1 != pEdgeNode0
        )
        {
        /* Just one scan line strikes this barrier. Explicit Connection logic top and bottom. */
        if (vu_fsucc (pEdgeTop0) != pScanNode0)
            {
            bd_join (pGraph, pParams, false, pEdgeTop0, pScanNode0, &pNewNodeA, &pNewNodeB);
            }

        if (vu_fsucc (pScanNode0) != pEdgeNode0)
            {
            bd_join (pGraph, pParams,  false, pEdgeNode0, pScanNode0, &pNewNodeA, &pNewNodeB);
            }
        currIndex = baseIndex + 1;
        }
    else
        {

        /* ALMOST ALWAYS connect to lowest scan node .. */
        if (pEdgeNode0 != vu_fsucc (pScanNode0))
            {
            if (bd_join (pGraph, pParams, false, pEdgeNode0, pScanNode0, &pNewNodeA, &pNewNodeB))
                pPreviousConnectNode = scanType0 == ScanType_Peak ? pNewNodeB : NULL;
            }
        else
            {
            pPreviousConnectNode = scanType0 == ScanType_Peak ? pScanNode0 : NULL;
            }

        for (;currIndex++,
                   bd_readBarrierEntryFromGPA
                            (
                            sorter,
                            &pScanNode1,
                            &scanX1, &scanY1, &scanType1,
                            &pEdgeNode1, NULL,
                            &fraction1, currIndex)
                && pEdgeNode1 == pEdgeNode0;
            )
            {
            if  (   scanType1 == ScanType_Icicle)
                {
                if (pPreviousConnectNode)
                    {
                    /* Previously connected a peak to same barrier */
                    if (bd_join (pGraph, pParams, true, pScanNode1, pPreviousConnectNode, &pNewNodeA, &pNewNodeB))
                        {
                        pScanNode1 = pNewNodeA;
                        scanType1 = ScanType_LeftWall;
                        }
                    }
                else
                    {
                    /* Previously connected a wall to same barrier ...
                        I don't think we really know where the wall's peak is, but connect anyway*/
                    if (bd_join (pGraph, pParams, false, pEdgeNode0, pScanNode1, &pNewNodeA, &pNewNodeB))
                        {
                        pScanNode1 = pNewNodeA;
                        scanType1 = ScanType_LeftWall;
                        }

                    }
                }
            if (scanType1 == ScanType_Peak)
                pPreviousConnectNode = pScanNode1;
            else
                pPreviousConnectNode = NULL;

            /* Pro forma advance ... (We don't seem to be using much of this)*/
            pScanNode0 = pScanNode1;
            scanX0 = scanX1;
            scanY0 = scanY1;
            scanType0 = scanType1;
            fraction0 = fraction1;
            }

        /* ALMOST ALWAYS connect to highest scan node.  Just look out for the case where
            the right wall tops out and comes back down to the scan node. */
        if (pScanNode0 != vu_fsucc (pEdgeTop0))
            bd_join (pGraph, pParams, false, pEdgeTop0, pScanNode0, &pNewNodeA, &pNewNodeB);
        }

    return currIndex;
    }

/*-----------------------------------------------------------------*//**
* @description On input, pGPA holds hit records for right scans.
*   Hit records are sorted by fraction (monotone in Y) within same edge.
* @bsihdr                                                                       EarlinLutz      09/02
+---------------+---------------+---------------+---------------+------*/
static void addEdgesToSortedRightBarriers
(
VuSetP pGraph,
VuSortArray &sorter,
ExtremaParams *pParams
)
    {
    int i0, i1;
    int n = (int) sorter.Size();

    for (i0 = 0; i0 < n;)
        {
        i1 = bd_joinToBarrier (pGraph, sorter, i0, pParams);
        if (i1 <= i0)
            i0++;
        else
            i0 = i1;
        }
    }

/*-----------------------------------------------------------------*//**
* @description Add edges to oriented loops using explicit period data.
* Strict bottom to top sweep -- caller responsible for directional validity.
* @param xPeriod IN x coordinate period. (0 if nonperiodic)
* @param yPeriod IN y coordinate period. (0 if nonperiodic)
* @bsihdr                                                                       EarlinLutz      09/02
+---------------+---------------+---------------+---------------+------*/
static void vu_periodicRightConnect_go
(
VuSetP pGraph,
double maxXFraction,
double maxYFraction,
int     baseTrapMask,
VuMask  inactiveFaceMask
)
    {
    VuSortArray sorter = VuSortArray();
    VuP pMinX, pMaxX, pMinY, pMaxY;
    VuMask strictUpEdgeMask = vu_grabMask (pGraph);
    VuMask lexicalUpEdgeMask = vu_grabMask (pGraph);
    ExtremaParams extremaParams;
    DPoint3d periods;
    double xPeriod, yPeriod;
    static int s_maxJoin = 0;
    static double s_minXFraction = 1.0e-9;

    vu_getPeriods (pGraph, &periods);
    xPeriod = periods.x;
    yPeriod = periods.y;

    vu_findExtremaInGraph (pGraph, &pMinX, &pMaxX, &pMinY, &pMaxY);
    extremaParams.lexicalMask = lexicalUpEdgeMask;
    extremaParams.strictMask = strictUpEdgeMask;
    extremaParams.xPeriod   = xPeriod;
    extremaParams.yPeriod   = yPeriod;
    extremaParams.maxXEdgeLength = maxXFraction * xPeriod;
    extremaParams.maxYEdgeLength = maxYFraction * yPeriod;
    //extremaParams.minXEdgeLength;
    extremaParams.trapCounter = baseTrapMask;
    extremaParams.maxJoin = s_maxJoin;
    extremaParams.inactiveFaceMask = inactiveFaceMask;
    extremaParams.numJoin = 0;
    extremaParams.minXEdgeLength = s_minXFraction * (vu_getX (pMaxX) - vu_getX (pMinX));

    vu_postGraphToTrapFunc (pGraph, "vu_periodicRightConnect", 0, extremaParams.trapCounter++);


    markPeriodicUpEdges (pGraph, strictUpEdgeMask,  xPeriod, yPeriod, isStrictUpVector,  inactiveFaceMask);
    markPeriodicUpEdges (pGraph, lexicalUpEdgeMask, xPeriod, yPeriod, isLexicalUpVector, inactiveFaceMask);

    collectRightBarrierScans (pGraph, sorter, computeScanType, &extremaParams, inactiveFaceMask);
    sorter.Sort();
    addEdgesToSortedRightBarriers (pGraph, sorter, &extremaParams);
    vu_returnMask (pGraph, strictUpEdgeMask);
    vu_returnMask (pGraph, lexicalUpEdgeMask);

    vu_postGraphToTrapFunc (pGraph, "vu_periodicRightConnect", 0, extremaParams.trapCounter++);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     09/02
+---------------+---------------+---------------+---------------+---------------+------*/
static void vu_periodicRightConnect
(
VuSetP pGraph,
double maxXFraction,
double maxYFraction,
int     baseTrapMask,
VuMask  inactiveFaceMask
)
    {
    Transform transform0;
    Transform transform1;
    static double s_skewFraction = 1.823423499243234892378e-3;
    bool    bApplySkew = false;

    /* Horizontal scan lines interact with horizontal geometry lines in bad ways.
       Apply a skew transform to the geometry.
    */

    bsiTransform_initIdentity (&transform0);
    bsiTransform_setMatrixComponentByRowAndColumn (&transform0, 1, 0, -s_skewFraction);

    /* All repeat together: to invert a gauss spike matrix, negate the spike. */
    bsiTransform_initIdentity (&transform1);
    bsiTransform_setMatrixComponentByRowAndColumn (&transform1, 1, 0, +s_skewFraction);

    if (bApplySkew)
        vu_transform2d (pGraph, &transform0);

    vu_periodicRightConnect_go (pGraph,
                    maxXFraction,
                    maxYFraction,
                    baseTrapMask,
                    inactiveFaceMask);

    if (bApplySkew)
        vu_transform2d (pGraph, &transform1);
    }

/*---------------------------------------------------------------------------------**//**
@description From each node with the seed mask, mark all nodes in the face loop and their edge mates if the edge
    isn't marked (on either side) by the barrier mask.
@remarks The flood mask must be distinct from the seed and barrier masks.  Seed and barrier masks may be duplicate.
    Any duplication of masks or violation of null conditions triggers return with no action.
@param pGraph       IN OUT  graph to examine
@param seedMask     IN      identifies nodes that are to serve as seeds for the search.  Must be non-null.
@param floodMask    IN      the mask to apply to accessible nodes.  This mask is cleared in the set at start.  Must be non-null.
@param barrierMask  IN      mask identifying edges not to be crossed.  May be null (This means that the
                            flood mask is set throughout any component containing a seed).
@group "VU Node Masks"
@bsimethod                                                      EarlinLutz      09/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_spreadMask
(
VuSetP pGraph,
VuMask seedMask,
VuMask floodMask,
VuMask barrierMask
)
    {
    VuArrayP pStack = vu_grabArray (pGraph);
    VuP pStackSeed, pMate;
    VuMask floodOrBarrierMask = floodMask | barrierMask;

    if (floodMask && seedMask
        && floodMask != seedMask
        && floodMask != barrierMask
        )
        {
        vu_clearMaskInSet (pGraph, floodMask);
        VU_SET_LOOP (pGlobalSeed, pGraph)
            {
            if ( vu_getMask (pGlobalSeed, seedMask)
               && !vu_getMask (pGlobalSeed, floodMask)
               )
                {
                vu_arrayAdd (pStack, pGlobalSeed);
                while (NULL != (pStackSeed = vu_arrayRemoveLast (pStack)))
                    {
                    if (!vu_getMask (pStackSeed, floodMask))
                        {
                        /* Mark this face visited.  Push mates on stack per condition */

                        VU_FACE_LOOP (pFaceNode, pStackSeed)
                            {
                            vu_setMask (pFaceNode, floodMask);
                            if (!vu_getMask (pFaceNode, barrierMask))
                                {
                                pMate = vu_edgeMate (pFaceNode);
                                if (!vu_getMask (pMate, floodOrBarrierMask))
                                    {
                                    vu_arrayAdd (pStack, pMate);
                                    }
                                }
                            }
                        END_VU_FACE_LOOP (pFaceNode, pStackSeed)
                        }
                    }
                }
            }
        END_VU_SET_LOOP (pGlobalSeed, pGraph)
        }
    vu_returnArray (pGraph, pStack);
    }


/*-----------------------------------------------------------------*//**
@nodoc
@description Add edges to oriented loops using period data from the graph header.
@remarks This function is useful for generating connections to holes in tricky toroidal parameter spaces.
@remarks Multiple sweeps are performed, one from each cardinal direction.
@param pGraph IN OUT graph to examine
@param minGridXCell IN suggested grid count in x direction
@param minGridYCell IN suggested grid count in y direction
@group "VU Meshing"
@see vu_buildNonInterferingGrid
@bsimethod                                      EarlinLutz       09/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_periodicRotateAndConnect
(
VuSetP pGraph,
int minGridXCell,
int minGridYCell
)
    {
    DPoint3d originalPeriods;
    double xPeriod, yPeriod;
    double periodFraction = 0.49999;
    int trapMask = 11500;
    int gridTrapMask = 12500;
    int numInteriorX, numInteriorY;
    static bool    s_bAlwaysFlood = true;
    bool    bRotateFirst, bBuildGrid;
    bool    bRunStepA = false;
    bool    bRunStepB = false;
    static int s_defaultGridCell = 12;
    int numGridXCell = 0, numGridYCell = 0;

    VuMask mGridInteriorMask = vu_grabMask (pGraph);
    VuMask mGridExteriorMask = vu_grabMask (pGraph);

    vu_getPeriods (pGraph, &originalPeriods);
    vu_clearMaskInSet (pGraph, mGridInteriorMask | mGridExteriorMask);

    xPeriod = originalPeriods.x;
    yPeriod = originalPeriods.y;

    vu_countPolarLoops (pGraph, NULL, &numInteriorX, NULL, NULL, &numInteriorY, NULL);

    bRotateFirst = bBuildGrid = false;
    if (xPeriod != 0.0 && yPeriod == 0.0)
        {
        /* Cylindrical or spherical coordinates -- always rotate */
        bRotateFirst = true;
        }

    if (xPeriod != 0.0 && yPeriod != 0.0)
        {
        bBuildGrid = true;

        numGridXCell = s_defaultGridCell;
        numGridYCell = s_defaultGridCell;

        if (minGridXCell > numGridXCell)
            numGridXCell = minGridXCell;

        if (minGridYCell > numGridYCell)
            numGridYCell = minGridYCell;

        /* Toroid -- rotate if xLoops but not yLoops */
        if (true)//numInteriorX > 0 && numInteriorY == 0)
            {
            bRotateFirst = true;
            bRunStepA = true;
            }
        }

    if (minGridXCell > 0 && minGridYCell > 0)
        {
        bBuildGrid = true;
        numGridXCell = minGridXCell;
        numGridYCell = minGridYCell;
        /* If periodic, we are going to work with the whole periodic space,
            so impose minimum counts. */
        if (xPeriod > 0.0 && numGridXCell < s_defaultGridCell)
            numGridXCell = s_defaultGridCell;

        if (yPeriod > 0.0 && numGridYCell < s_defaultGridCell)
            numGridYCell = s_defaultGridCell;

        }


    if (bRunStepA)
        vu_periodicRightConnect (pGraph, periodFraction, periodFraction, trapMask, mGridInteriorMask);
    trapMask += 100;

    if (bRotateFirst)
        vu_rotate90CCW (pGraph);

    if (bBuildGrid)
        {
        /* Construct a grid which "fills" large spaces */
        vu_buildNonInterferingGrid
            (
            pGraph,
            numGridXCell, numGridYCell,
            mGridInteriorMask, mGridExteriorMask
            );
        vu_postGraphToTrapFunc (pGraph, "vu_periodicConnect::baseGrid", 0, gridTrapMask++);
        }

    vu_periodicRightConnect (pGraph, periodFraction, periodFraction, trapMask, mGridInteriorMask);
    trapMask += 100;

    if (bRotateFirst)
        vu_rotate90CW (pGraph);

    if (bRunStepB)
        vu_periodicRightConnect (pGraph, periodFraction, periodFraction, trapMask, mGridInteriorMask);
    trapMask += 100;

    if (bBuildGrid || s_bAlwaysFlood)
        {
        /* The grid may contain edges "outside" the bounded interior faces.  They are now
            connected to nearby exteriors, so mask flooding can mark them exterior.
        */
        VuMask mFloodMask = vu_grabMask (pGraph);
        vu_postGraphToTrapFunc (pGraph, "vu_periodicConnect::connected Grid", 0, gridTrapMask++);
        vu_spreadMask (pGraph, VU_EXTERIOR_EDGE, mFloodMask, VU_BOUNDARY_EDGE);
        VU_SET_LOOP (pCurr, pGraph)
            {
            if (vu_getMask (pCurr, mFloodMask))
                vu_setMask (pCurr, VU_EXTERIOR_EDGE);
            else
                vu_clrMask (pCurr, VU_EXTERIOR_EDGE);
            }
        END_VU_SET_LOOP (pCurr, pGraph)

        vu_postGraphToTrapFunc (pGraph, "vu_periodicConnect::after first flood", 0, gridTrapMask++);
        /* Graph construction can create components completely isolated from exterior edges.
        Call these exterior?????
        */
        vu_spreadMask (pGraph, VU_EXTERIOR_EDGE, mFloodMask, 0);
        VU_SET_LOOP (pCurr, pGraph)
            {
            if (!vu_getMask (pCurr, mFloodMask))
                vu_setMask (pCurr, VU_EXTERIOR_EDGE);
            }
        END_VU_SET_LOOP (pCurr, pGraph)
        vu_postGraphToTrapFunc (pGraph, "vu_periodicConnect::after second flood", 0, gridTrapMask++);
        vu_returnMask (pGraph, mFloodMask);
        }


    vu_setPeriodicCoordinatesWithinFaces (pGraph);
    vu_postGraphToTrapFunc (pGraph, "vu_periodicConnect", 0, trapMask++);

    vureg_regularizeConnectedInteriorFaces (pGraph);
    vu_postGraphToTrapFunc (pGraph, "vu_periodicConnect", 0, trapMask++);

    vu_returnMask (pGraph, mGridExteriorMask);
    vu_returnMask (pGraph, mGridInteriorMask);
    }


END_BENTLEY_GEOMETRY_NAMESPACE
