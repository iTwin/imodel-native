/*--------------------------------------------------------------------------------------+
|
|     $Source: vu/src/pbfvu.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <vector>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static int s_noisy = 0;

struct PBFNode
{
VuP     m_node;
double  m_aValue;
int     m_iValue;
PBFNode (VuP node, double aValue, int iValue)
    {
    m_node = node;
    m_aValue = aValue;
    m_iValue = iValue;
    }

PBFNode ()
    {
    m_node = NULL;
    m_aValue = 0.0;
    m_iValue = 0;
    }
bool IsBelow (PBFNode const &other)
    {
    return m_aValue < other.m_aValue;
    }
};

/*=================================================================================**//**
* @bsiclass                                                     Earlin.Lutz     02/2006
+===============+===============+===============+===============+===============+======*/
class PBFHeap
{
private:
bvector <PBFNode> mArray;

public:
PBFHeap ()
    : mArray ()
    {}

void Clear ()
    {
    mArray.clear ();
    }

size_t Size ()
    {
    return mArray.size ();
    }

void Add (PBFNode &gp)
    {
    mArray.push_back (gp);
    }

bool Get (PBFNode &value, int index)
    {
    if (index >= 0 && index < (int) mArray.size ())
        {
        value = mArray[index];
        return true;
        }
    return false;
    }

bool Pop (PBFNode &value)
    {
    if (mArray.size() > 0)
        {
        value = mArray.back();
        mArray.pop_back();
        return true;
        }
    return false;
    }

void SetUnchecked (PBFNode &value, int index)
    {
    mArray[index] = value;
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     07/01
+---------------+---------------+---------------+---------------+---------------+------*/
static int      checkHeap
(
PBFHeap &heap
)
    {
    int i0, i1, i2;
    //size_t count = heap.Size();
    //int internalCount = (count + 1) > 1;
    int errors = 0;
    PBFNode gp0, gp1, gp2;

    for (i0 = 0; heap.Get (gp0, i0); i0++)
        {
        i1 = i0 * 2 + 1;
        i2 = i1 + 1;

        if (heap.Get (gp1, i1) && gp0.IsBelow (gp1))
            {
            printf (" Heap error %d(%lf) %d (%lf)\n", i0, gp0.m_aValue, i1, gp1.m_aValue);
            errors++;
            }
        if (heap.Get (gp2, i2) && gp0.IsBelow (gp2))
            {
            printf (" Heap error %d(%lf) %d (%lf)\n", i0, gp0.m_aValue, i2, gp2.m_aValue);
            errors++;
            }
        }
    return errors;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     07/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void enqueueVuCandidate
(
PBFHeap &heap,
VuP     pNode,
double  value
)
    {
    int idLow, idHigh;
    PBFNode gp (pNode, value, vu_getIndex (pNode));

    if (s_noisy > 2)
        printf(" enqueue %d %lf (%d)\n", (int)gp.m_iValue, gp.m_aValue, (int)heap.Size());

    idLow = (int) heap.Size();
    heap.Add (gp);

    // Bubble the new gp up from final position.
    while (idLow > 0)
        {
        PBFNode gpHi;
        idHigh = (idLow - 1) >> 1;    // Divide-and-truncate goes up in binary heap.
        heap.Get (gpHi, idHigh);

        if (gpHi.IsBelow (gp))
            {
            heap.SetUnchecked (gpHi, idLow);
            idLow = idHigh;
            if (idLow == 0)
                heap.SetUnchecked (gp, 0);
            }
        else
            {
            heap.SetUnchecked (gp, idLow);
            idLow = 0;
            }
        }

    if (s_noisy > 100)
        checkHeap (heap);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     07/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    dequeueVuCandidate
(
PBFHeap &heap,
VuP     *pNode,
double  *pValue
)
    {
    PBFNode gp, gp1, gp2;
    size_t count;
    int id0, id1, id2;
    int iValue;
    double aValue;

    if (!heap.Get (gp, 0))
        return false;

    if (pNode)
        *pNode = gp.m_node;
    if (pValue)
        *pValue = gp.m_aValue;
    aValue    = gp.m_aValue;
    iValue = gp.m_iValue;

    if (heap.Pop (gp) && (count = heap.Size()) > 0)
        {
        /* Move the gp just removed from the end up in place of the top. */
        heap.SetUnchecked (gp, 0);
        id0 = 0;

        // while there are children along a downward path...
        while (heap.Get (gp1, id1 = 2 * id0 + 1))
            {
            id2 = id1 + 1;

            // Find the larger child...
            if (heap.Get (gp2, id2) && gp1.IsBelow (gp2))
                {
                id1 = id2;
                gp1 = gp2;
                }

            // Move down to larger side.
            if (gp.IsBelow (gp1))
                {
                heap.SetUnchecked (gp1, id0);
                heap.SetUnchecked (gp,  id1);
                id0 = id1;
                }
            else
                {
                id0 = (int) count;
                }
            }
        }
    if (s_noisy > 100)
        checkHeap (heap);
    if (s_noisy > 2)
        printf (" dequeue %d %lf (%d)\n", iValue, aValue, (int)heap.Size());
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     07/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void completeEdgeSplit
(
VuSetP pGraph,
VuP pSplitNode,
VuRefinementJoinFunc joinfunc,
void *pFuncData
)
    {
    VuP pCornerNode, pFarNode;

    VuP pAddNodeA, pAddNodeB;

    if (!vu_getMask (pSplitNode, VU_EXTERIOR_EDGE))
        {
        pCornerNode = vu_fsucc(pSplitNode);
        pFarNode = vu_fsucc(pCornerNode);
        vu_join (pGraph, pSplitNode, pFarNode, &pAddNodeA, &pAddNodeB);
        joinfunc (pFuncData, pGraph, pSplitNode, pFarNode, pAddNodeA, pAddNodeB);
        if (s_noisy > 50)
            printf (" Join (base %d cor %d to %d) (New %d %d)\n",
                        vu_getIndex (pSplitNode),
                        vu_getIndex (pCornerNode),
                        vu_getIndex (pFarNode),
                        vu_getIndex (pAddNodeA),
                        vu_getIndex (pAddNodeB)
                        );
        }
    }

/*---------------------------------------------------------------------------------**//**
* On input: triangulated graph with mask on distinguished edges to be split.
* @bsimethod                                    Earlin.Lutz                     07/01
+---------------+---------------+---------------+---------------+---------------+------*/
static int vu_subdivideMarkedTriangulated
(
VuSetP  pGraph,
PBFHeap    &heap,
VuRefinementEdgeSplitFunc splitfunc,
VuRefinementJoinFunc        joinfunc,
void *pFuncData
)
    {
    VuArrayP pSplitArray = vu_grabArray (pGraph);
    int numSplit = 0;
    VuMask visitMask = vu_grabMask (pGraph);
    VuP pNodeA0, pNodeA1, pNodeA2;
    VuP pNodeB0, pNodeB1, pNodeB2;
    VuP pSplitNode;

    vu_clearMaskInSet (pGraph, visitMask);

    /* ASSUME -- candidate array is right.
        ... Each edge is entered only once.
        ... No bad ones to filter.
    */
    for (;dequeueVuCandidate (heap, &pNodeA0, NULL);)
        {
        pNodeA2 = vu_fsucc (pNodeA0);
        pNodeB0 = vu_vsucc (pNodeA2);
        pNodeB2 = vu_fsucc (pNodeB0);

        if (  !vu_getMask (pNodeA0, visitMask)
           && !vu_getMask (pNodeB0, visitMask)
           && splitfunc (pFuncData, pGraph, pNodeA0, &pNodeA1, &pNodeB1)
           )
            {
            // Prevent split of other edges on these faces.
            vu_markFace(pNodeA1, visitMask);
            vu_markFace(pNodeB1, visitMask);
            vu_arrayAdd (pSplitArray, pNodeA1);
            vu_arrayAdd (pSplitArray, pNodeB1);
            numSplit++;
            if (s_noisy > 50)
                printf (" Split edge (L %d %d %d) (R %d %d %d)\n",
                            vu_getIndex (pNodeA0),
                            vu_getIndex (pNodeA1),
                            vu_getIndex (pNodeA2),
                            vu_getIndex (pNodeB0),
                            vu_getIndex (pNodeB1),
                            vu_getIndex (pNodeB2)
                            );
            }
        }

    /* Add back edges in all interior faces. */
    for (vu_arrayOpen (pSplitArray);
         vu_arrayRead (pSplitArray, &pSplitNode);)
        {
        completeEdgeSplit (pGraph, pSplitNode, joinfunc, pFuncData);
        }

    vu_returnArray (pGraph, pSplitArray);
    vu_returnMask (pGraph, visitMask);
    return numSplit;
    }

/*-----------------------------------------------------------------*//**
* @description Refine a triangulation by splitting edges and joining them with new edges to form new triangles.
* @param pGraph     IN OUT  vu graph (triangulated) to refine
* @param testFunc   IN      function to test and prioritize edge split candidate
* @param splitFunc  IN      function to split edge
* @param joinFunc   IN      function to post-process a newly added edge
* @param pFuncData  IN      first arg for test and split funcs
* @param flipFunc   IN      unused
* @return number of edges split
* @group "VU Meshing"
* @bsimethod                            EarlinLutz      07/01
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int  vu_refine
(
VuSetP  pGraph,
VuRefinementPriorityFunc testFunc,
VuRefinementEdgeSplitFunc splitFunc,
VuRefinementJoinFunc joinFunc,
void *pFuncData,
VuRefinementFlipTestFunc  flipFunc
)
    {
    VuP         pMate;
    double      value;
    int         numSplit = 0;
    PBFHeap    heap = PBFHeap();

    VuMask visitMask = vu_grabMask (pGraph);
    vu_clearMaskInSet (pGraph, visitMask);
    /* Find candidate edges */
    VU_SET_LOOP (pCurr, pGraph)
        {
        pMate = vu_edgeMate (pCurr);
        if (!vu_getMask (pCurr, visitMask)
            && !vu_getMask (pMate, visitMask))
            {
            value = testFunc (pFuncData, pGraph, pCurr);
            if (value > 0.0)
                {
                enqueueVuCandidate (heap, pCurr, value);
                vu_setMask (pCurr, visitMask);
                vu_setMask (pMate, visitMask);
                }
            }
        }
    END_VU_SET_LOOP (pCurr, pGraph)

    numSplit = vu_subdivideMarkedTriangulated (pGraph, heap, splitFunc, joinFunc, pFuncData);

    vu_returnMask (pGraph, visitMask);
    return numSplit;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     07/01
+---------------+---------------+---------------+---------------+---------------+------*/
static VuP pbfvu_tcz_findStartPair
(
VuSetP pGraph,
VuP pLoopA,
VuP pLoopB,
double zPeriod
)
    {
    double z0, z1, dz, dzMin;
    VuP pMinB = NULL;
    z0 = vu_getZ (pLoopA);

    dzMin = 1.0e100;

    VU_FACE_LOOP (pCurrB, pLoopB)
        {
        z1 = vu_getZ (pCurrB);
        dz = fabs (bsiTrig_normalizeToPeriodAroundZero (z1 - z0, zPeriod));
        if (pMinB == NULL || dz < dzMin)
            {
            dzMin = dz;
            pMinB = pCurrB;
            }
        }
    END_VU_FACE_LOOP (pCurrB, pLoopB)
    return pMinB;
    }

/*-----------------------------------------------------------------*//**
* Triangulate between two face loops for exact starts.
* @param
* @bsimethod                            EarlinLutz      07/01
+----------------------------------------------------------------------*/
static bool    pbfvu_tcz_run
(
VuSetP  pGraph,
VuP     pStartA,
VuP     pStartB,
double  zPeriod,
VuRefinementJoinFunc joinFunc,
void *pFuncData,
VuMask newEdgeMask
)
    {
    VuMask myNewEdgeMask = vu_grabMask (pGraph);
    VuMask appliedMask = myNewEdgeMask | newEdgeMask;
    VuP pNodeA0, pNodeB0, pNodeA1, pNodeB1, pNewNodeA, pNewNodeB;
    int numA = vu_faceLoopSize (pStartA);
    int numB = vu_faceLoopSize (pStartB);
    int numNeed = numA + numB;
    int numGot = 0;
    bool    bStat = true;
    double dzA, dzB;
    int mask = 16000;

    vu_join (pGraph, pStartA, pStartB, &pNewNodeA, &pNewNodeB);
    joinFunc (pFuncData, pGraph, pStartA, pStartB, pNewNodeA, pNewNodeB);

    vu_setMask (pNewNodeA, appliedMask);
    vu_setMask (pNewNodeB, appliedMask);
    numGot++;

    /* "0" nodes are at ends of the strut */
    pNodeA0 = pNewNodeA;
    pNodeB0 = pStartB;

    pNodeB1 = vu_fsucc (pNodeB0);
    pNodeA1 = vu_fpred (pNodeA0);
    vu_postGraphToTrapFunc (pGraph, "cylindrical", 0, mask++);

    while (pNodeB1 != pNodeA1 && numGot < numNeed)
        {
        /* "1" nodes are 1 ahead of the ends of the strut */
        dzB = bsiTrig_normalizeToPeriodAroundZero
                    (vu_getZ (pNodeB1) - vu_getZ (pNodeA0), zPeriod);
        dzA = bsiTrig_normalizeToPeriodAroundZero
                    (vu_getZ (pNodeA1) - vu_getZ (pNodeB0), zPeriod);

        if (!vu_getMask (pNodeA1, myNewEdgeMask) && fabs (dzA) < fabs (dzB))
            {
            vu_join (pGraph, pNodeA1, pNodeB0, &pNewNodeA, &pNewNodeB);
            joinFunc (pFuncData, pGraph, pNodeA1, pNodeB0, pNewNodeA, pNewNodeB);
            pNodeA0 = pNewNodeA;
            }
        else if (!vu_getMask (pNodeB0, myNewEdgeMask))
            {
            vu_join (pGraph, pNodeA0, pNodeB1, &pNewNodeA, &pNewNodeB);
            joinFunc (pFuncData, pGraph, pNodeA0, pNodeB1, pNewNodeA, pNewNodeB);
            pNodeB0 = pNodeB1;
            pNodeA0 = pNewNodeA;
            }
        else
            {
            bStat = false;
            break;
            }


        vu_setMask (pNewNodeA, appliedMask);
        vu_setMask (pNewNodeB, appliedMask);
        numGot++;
        vu_postGraphToTrapFunc (pGraph, "cylindrical", 0, mask++);
        pNodeB1 = vu_fsucc (pNodeB0);
        pNodeA1 = vu_fpred (pNodeA0);
        }

    vu_returnMask (pGraph, myNewEdgeMask);
    return bStat;
    }

/*-----------------------------------------------------------------*//**
* @nodoc
* @deprecated vu_triangulateBetweenCyclesByZ
* @bsimethod                            EarlinLutz      07/01
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    pbfvu_triangulateyBetweenCyclesByZ
(
VuSetP  pGraph,
VuP     pLoopA,
VuP     pLoopB,
double  zPeriod,
VuRefinementJoinFunc joinFunc,
void *pFuncData,
VuMask newEdgeMask
)
    {
    return vu_triangulateBetweenCyclesByZ (pGraph, pLoopA, pLoopB, zPeriod, joinFunc, pFuncData, newEdgeMask);
    }

/*-----------------------------------------------------------------*//**
* @description Triangulate between two face loops.
* @remarks This function is appropriate for triangulating the annulus on the end face of a pipe, or between top and bottom rings of a cylinder.
*       The triangulation direction is based on periodically adjusted values of the z-coordinate in each node.
* @param pGraph         IN OUT  vu graph
* @param pLoopA         IN      node in first face loop
* @param pLoopB         IN      node in second face loop
* @param zPeriod        IN      length of period of z-coordinate
* @param joinFunc       IN      function to post-process a newly added edge
* @param pFuncData      IN      pointer passed into callback
* @param newEdgeMask    IN      node mask to apply to new edges
* @return true if triangulation is successful
* @group "VU Meshing"
* @bsimethod                            EarlinLutz      07/01
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    vu_triangulateBetweenCyclesByZ
(
VuSetP  pGraph,
VuP     pLoopA,
VuP     pLoopB,
double  zPeriod,
VuRefinementJoinFunc joinFunc,
void *pFuncData,
VuMask newEdgeMask
)
    {
    VuP pStartA = pLoopA;
    VuP pStartB;
    pStartB = pbfvu_tcz_findStartPair (pGraph, pLoopA, pLoopB, zPeriod);
    return pbfvu_tcz_run (pGraph,
            pStartA, pStartB, zPeriod, joinFunc, pFuncData,
            newEdgeMask);
    }
END_BENTLEY_GEOMETRY_NAMESPACE
