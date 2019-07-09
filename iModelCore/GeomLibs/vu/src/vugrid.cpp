/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#include <math.h>
END_BENTLEY_GEOMETRY_NAMESPACE

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
typedef uint32_t CellMask;

#define NUM_NODE_PER_CELL 4

#define CELLNODE_RIGHT  0
#define CELLNODE_UP     1
#define CELLNODE_LEFT   2
#define CELLNODE_DOWN   3

#define GCMASK_OCCUPIED (0x01)

struct IntRange
    {
    int low, high;
    IntRange (int i0, int i1)
        {
        if (i0 <= i1)
            {
            low = i0;
            high = i1;
            }
        else
            {
            low = i1;
            high = i0;
            }
        }
    void Extend (int i)
        {
        if (i < low)
            low = i;
        if (i > high)
            high = i;
        }
    static IntRange FromUnion (IntRange rangeA, IntRange rangeB)
        {
        IntRange result = rangeA;
        result.Extend (rangeB.low);
        result.Extend (rangeB.high);
        return result;
        }
    };



/*=================================================================================**//**
* @bsiclass                                                     EarlinLutz      02/2006
+===============+===============+===============+===============+===============+======*/
struct GridCell
    {
    CellMask m_mask;
    VuP m_pNode[NUM_NODE_PER_CELL];

    CellMask getMask (CellMask mask)
        {
        return (m_mask & mask);
        }

    void setMask (CellMask mask)
        {
        m_mask |= mask;
        }

    void clearMask (CellMask mask)
        {
        m_mask &= !mask;
        }

    void setNode (int id, VuP pNode)
        {
        if (id >= 0 && id < NUM_NODE_PER_CELL)
            m_pNode[id] = pNode;
        }

    VuP getNode (int id)
        {
        if (id >= 0 && id < NUM_NODE_PER_CELL)
            return m_pNode[id];
        return NULL;
        }

    void initForIncidenceMarkup ()
        {
        int i;
        clearMask (GCMASK_OCCUPIED);
        for (i = 0; i < 4; i++)
            m_pNode[i] = NULL;
        }
    };



/*=================================================================================**//**
* @bsiclass                                                     EarlinLutz      02/2006
+===============+===============+===============+===============+===============+======*/
struct StrideData
    {
    double m_a0, m_a1;
    double m_da;
    double m_daInv;
    int    m_numCell;
    int    m_iStride;
    int    m_bIsPeriodic;

    void init
        (
        double a0,
        double a1,
        int    iNumCell,
        bool    bIsPeriodic,
        int    iStride
        )
        {
        m_a0            = a0;
        m_a1            = a1;
        m_da            = (a1 - a0) / iNumCell;
        m_daInv         = 1.0 / m_da;
        m_numCell       = iNumCell;
        m_bIsPeriodic   = bIsPeriodic;
        m_iStride       = iStride;
        }

    int getNumCell ()
        {
        return m_numCell;
        }
    IntRange getIntRange (double aa, double fractionTol = 0.000001)
        {
        double g = (aa - m_a0) * m_daInv;
        int    ig = (int)floor (g);
        double dg = g - (double)ig;
        if (dg < fractionTol)
            return IntRange (ig - 1, ig);
        else if (dg > 1.0 - fractionTol)
            return IntRange (ig, ig + 1);
        else
            return IntRange (ig, ig);
        }

    bool    gc_resolveIndex
        (
        int *pIndex,
        int index
        )
        {
        if (0 <= index && index < m_numCell)
            {
            *pIndex = index;
            return true;
            }

        if (m_bIsPeriodic)
            {
            /* code for the normal case -- just one period of adjustment,
                just increment by period (rather than divide) */
            while (index < 0)
                index += m_numCell;

            while (index >= m_numCell)
                index -= m_numCell;

            *pIndex = index;

            return true;
            }
        *pIndex = 0;
        return false;
        }

    /*
    * Map a (gridlineIndex,fraction) pair to world double
    */
    double map (int ix, double fraction)
        {
        return m_a0 + m_da * ((double)ix + fraction);
        }

    /*
    * Map a gridline to world double
    */
    double map (int ix)
        {
        return m_a0 + m_da * (double)ix;
        }

    };


struct GridContext
    {
    VuSetP pGraph;
    GridCell *pCellBuffer;
    StrideData strideData[2];
    VuMask interiorMask;
    VuMask exteriorMask;
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2006
+---------------+---------------+---------------+---------------+---------------+------*/
static GridCell *gc_getCellInStride
(
GridContext *pGC,
int strideSelect,   // MUST be 0 or 1
int ix,
int iy
)
    {
    StrideData *pXStride = &pGC->strideData[strideSelect];
    StrideData *pYStride = &pGC->strideData[1 - strideSelect];

    if (    pXStride->gc_resolveIndex (&ix, ix)
        &&  pYStride->gc_resolveIndex (&iy, iy))
        {
        return &pGC->pCellBuffer[pXStride->m_iStride * ix + pYStride->m_iStride * iy];
        }
    return NULL;
    }

/*-------------------------------------------------------------*//**
@description Map (lineNumber,fraction) pairs to xy.
----------------------------------------------------------------*/
static void gc_mapToDPoint2d
(
GridContext *pGC,
DPoint2d *pXY,
int ix,
double xFraction,
int iy,
double yFraction
)
    {
    StrideData *pXStride = &pGC->strideData[0];
    StrideData *pYStride = &pGC->strideData[1];
    pXY->x = pXStride->map (ix, xFraction);
    pXY->y = pYStride->map (iy, yFraction);
    }


/*-------------------------------------------------------------*//**
----------------------------------------------------------------*/
static void gc_setCellMaskInStride
(
GridContext *pGC,
int strideSelect,
int ix,
int iy,
CellMask mask
)
    {
    GridCell *pCell;
    pCell = gc_getCellInStride (pGC, strideSelect, ix, iy);
    if (pCell)
        pCell->setMask (mask);
    }

static void gc_setCellMaskInStride
(
GridContext *pGC,
int strideSelect,
IntRange rangeX,
IntRange rangeY,
CellMask mask
)
    {
    for (int ix = rangeX.low; ix <= rangeX.high; ix++)
        for (int iy = rangeY.low; iy <= rangeY.high; iy++)
            gc_setCellMaskInStride (pGC, strideSelect, ix, iy, mask);
    }

#ifdef CompileAll
/*-------------------------------------------------------------*//**
----------------------------------------------------------------*/
static int gc_getCellMaskInStride
(
GridContext *pGC,
int strideSelect,
int ix,
int iy,
int mask
)
    {
    GridCell *pCell = gc_getCellInStride (pGC, strideSelect, ix, iy);
    return pCell ? pCell->getMask (mask) : 0;
    }
#endif
/*-------------------------------------------------------------*//**
@description Clear out pNode, GCMASK_OCCUPIED prior to incidence markup.
----------------------------------------------------------------*/
static void gc_initForIncidenceMarkup
(
GridContext *pGC
)
    {
    int ij;
    int numCell = pGC->strideData[0].getNumCell () * pGC->strideData[1].getNumCell ();
    for (ij = 0; ij < numCell; ij++)
        {
        pGC->pCellBuffer[ij].initForIncidenceMarkup ();
        }
    }

/*-------------------------------------------------------------*//**
@description Record scan line cuts by an edge.
@param axisId IN indicates if "a" axis is x or y.
@param a0 IN line start on "a" axis.
@param a1 IN line end on "a" axis.  ASSUMED STRICTLY GREATER THAN a0.
@param b0 IN line start on "a" axis.
@param b1 IN line end on "a" axis.
*/
static void gc_recordAxisIncidence
(
GridContext *pGC,
int         axisId,
double      a0,
double      a1,
double      b0,
double      b1
)
    {
    static double s_smallFraction = 0.01;
    StrideData *pStrideA = &pGC->strideData[axisId];
    StrideData *pStrideB = &pGC->strideData[1 - axisId];
    double db = b1 - b0;

    // Integerized cell neighborhoods at start and end.
    IntRange rangeA0 = pStrideA->getIntRange (a0, s_smallFraction);
    IntRange rangeA1 = pStrideA->getIntRange (a1, s_smallFraction);
    IntRange rangeB0 = pStrideB->getIntRange (b0, s_smallFraction);
    IntRange rangeB1 = pStrideB->getIntRange (b1, s_smallFraction);
    IntRange rangeA  = IntRange::FromUnion (rangeA0, rangeA1);
    gc_setCellMaskInStride (pGC, axisId, rangeA0, rangeB0, GCMASK_OCCUPIED);
    gc_setCellMaskInStride (pGC, axisId, rangeA1, rangeB1, GCMASK_OCCUPIED);

    double divdA = a1 != a0 ? 1.0 / (a1 - a0) : 1.0;
    for (int iA = rangeA.low; iA <= rangeA.high; iA++)
        {
        double aCurr = pStrideA->map (iA);
        double fraction = (aCurr - a0) * divdA;
        double bCurr = b0 + fraction * db;
        IntRange cellRangeA = pStrideA->getIntRange (aCurr, s_smallFraction);
        IntRange cellRangeB = pStrideB->getIntRange (bCurr, s_smallFraction);
        gc_setCellMaskInStride (pGC, axisId, cellRangeA, cellRangeB, GCMASK_OCCUPIED);
        }
    }

/**
@description record crossings by an edge.
*/
static void gc_recordEdgeIncidence
(
GridContext *pGC,
VuP         pNode0
)
    {
    DPoint3d uv0, uv1, delta;
    double dx, dy;
    //VuP pNode1 = vu_fsucc (pNode0);
    vu_getDPoint3d (&uv0, pNode0);
    vu_getPeriodicDPoint3dDXYZ (&delta, pGC->pGraph, pNode0);
    uv1.SumOf (uv0, delta);

    /* slope components normalized to grid size */
    dx = fabs (delta.x) / pGC->strideData[0].m_da;
    dy = fabs (delta.y) / pGC->strideData[1].m_da;

    if (dx > dy)
        {
        if (uv0.x < uv1.x)
            gc_recordAxisIncidence (pGC, 0, uv0.x, uv1.x, uv0.y, uv1.y);
        else
            gc_recordAxisIncidence (pGC, 0, uv1.x, uv0.x, uv1.y, uv0.y);
        }
    else if (dy > dx && dy > 0.0)
        {
        if (uv0.y < uv1.y)
            gc_recordAxisIncidence (pGC, 1, uv0.y, uv1.y, uv0.x, uv1.x);
        else
            gc_recordAxisIncidence (pGC, 1, uv1.y, uv0.y, uv1.x, uv0.x);
        }
    /* And don't worry about dx==dy==0 */
    }

/**
@description
*/
static void gc_runIncidenceMarkup
(
GridContext *pGC
)
    {
    VuP pMate;
    VuMask visitMask = vu_grabMask (pGC->pGraph);
    VuMask skipMask  = visitMask;   // | EXTERIOR?

    vu_clearMaskInSet (pGC->pGraph, visitMask);
    VU_SET_LOOP (pCurrNode, pGC->pGraph)
        {
        if (!vu_getMask (pCurrNode, skipMask))
            {
            pMate = vu_edgeMate (pCurrNode);
            gc_recordEdgeIncidence (pGC, pCurrNode);
            vu_setMask (pCurrNode, visitMask);
            }
        }
    END_VU_SET_LOOP (pCurrNode, pGC->pGraph)
    vu_returnMask (pGC->pGraph, visitMask);
    }
#ifdef CompileAll
/**
@description Build edges for shrunken boxes inside unoccupied grid cells.
*/
static void gc_buildShrinkBoxes
(
GridContext *pGC,
double fraction
)
    {

    int ix, iy, i;
    DPoint2d uv[4];
    VuP pBaseNode, pLeftNode, pRightNode;
    int numX = pGC->strideData[0].getNumCell ();
    int numY = pGC->strideData[1].getNumCell ();
    VuSetP pGraph = pGC->pGraph;
    double f0 = 1.0 - fraction;
    double f1 = fraction;

    for (iy = 0; iy < numY; iy++)
        {
        for (ix = 0; ix < numX; ix++)
            {
            if (!gc_getCellMaskInStride (pGC, 0, ix, iy, GCMASK_OCCUPIED))
                {
                gc_mapToDPoint2d (pGC, &uv[0], ix, f0, iy, f0);
                gc_mapToDPoint2d (pGC, &uv[1], ix, f1, iy, f0);
                gc_mapToDPoint2d (pGC, &uv[2], ix, f1, iy, f1);
                pBaseNode = NULL;
                gc_mapToDPoint2d (pGC, &uv[3], ix, f0, iy, f1);

                for (i = 0; i < 4; i++)
                    {
                    vu_splitEdge (pGraph, pBaseNode, &pLeftNode, &pRightNode);
                    vu_setDPoint2d (pLeftNode,  &uv[i]);
                    vu_setDPoint2d (pRightNode, &uv[i]);
                    pBaseNode = pLeftNode;
                    }
                }
            }
        }
    }
#endif

/**
@description If both specified cells are unoccupied, build an
    edge between their centers and record its nodes in specified
    indices in the cells..
*/
static void gc_connectCellsIfUnoccipied
(
GridContext *pGC,
int ix0,
int iy0,
int ix1,
int iy1,
int index0,
int index1
)
    {
    VuSetP pGraph = pGC->pGraph;
    DPoint2d uv[2];
    GridCell *pCell0;
    GridCell *pCell1;
    VuP pNode0, pNode1;
    double p5 = 0.5;

    pCell0 = gc_getCellInStride (pGC, 0, ix0, iy0);
    pCell1 = gc_getCellInStride (pGC, 0, ix1, iy1);

    if (   pCell0
        && pCell1
        && !pCell0->getMask (GCMASK_OCCUPIED)
        && !pCell1->getMask (GCMASK_OCCUPIED)
        )
        {
        gc_mapToDPoint2d (pGC, &uv[0], ix0, p5, iy0, p5);
        gc_mapToDPoint2d (pGC, &uv[1], ix1, p5, iy1, p5);

        vu_join (pGraph, NULL, NULL, &pNode0, &pNode1);
        vu_setDPoint2d (pNode0, &uv[0]);
        vu_setDPoint2d (pNode1, &uv[1]);
        pCell0->setNode (index0, pNode0);
        pCell1->setNode (index1,  pNode1);
        }
    }

/**
@description Build edges between centers of adjacent unoccupied cells.
*/
static void gc_connectUnoccupiedCellCenters
(
GridContext *pGC
)
    {
    int ix, iy;
    int i;
    int numX = pGC->strideData[0].getNumCell ();
    int numY = pGC->strideData[1].getNumCell ();
    VuSetP pGraph = pGC->pGraph;
    for (iy = 0; iy < numY; iy++)
        {
        for (ix = 0; ix < numX; ix++)
            {
            gc_connectCellsIfUnoccipied (pGC, ix, iy, ix + 1, iy, CELLNODE_RIGHT, CELLNODE_LEFT);
            gc_connectCellsIfUnoccipied (pGC, ix, iy, ix, iy + 1, CELLNODE_UP, CELLNODE_DOWN);
            }
        }

    /*  Each cell has 0 to 4 dangling edges to its center.
        twist them together into a single vertex.
    */
    for (iy = 0; iy < numY; iy++)
        {
        for (ix = 0; ix < numX; ix++)
            {
            VuP pBaseNode = NULL;
            VuP pCurrNode;
            GridCell *pCell= gc_getCellInStride (pGC, 0, ix, iy);
            for (i = 0; i < NUM_NODE_PER_CELL; i++)
                {
                pCurrNode = pCell->getNode (i);
                if (pCurrNode)
                    {
                    if (pBaseNode)
                        vu_vertexTwist (pGraph, pCurrNode, pBaseNode);
                    pBaseNode = pCurrNode;
                    }
                }
            }
        }

    if (pGC->interiorMask || pGC->exteriorMask)
        {
        VuMask interiorMask = pGC->interiorMask;
        VuMask exteriorMask = pGC->exteriorMask;
        if (!interiorMask)
            interiorMask = vu_grabMask (pGraph);
        /*  Look 2x2 grids.  If fully unoccupied, mark the connecting edges. */
        for (iy = 0; iy < numY; iy++)
            {
            for (ix = 0; ix < numX; ix++)
                {
                GridCell *pCell00 = gc_getCellInStride (pGC, 0, ix, iy);
                GridCell *pCell10 = gc_getCellInStride (pGC, 0, ix + 1, iy);
                GridCell *pCell11 = gc_getCellInStride (pGC, 0, ix + 1, iy + 1);
                GridCell *pCell01 = gc_getCellInStride (pGC, 0, ix, iy + 1);

                if (pCell00 && pCell10 && pCell01 && pCell11)
                    {
                    VuP pNode00 = pCell00->getNode (CELLNODE_RIGHT);
                    VuP pNode10 = pCell10->getNode (CELLNODE_UP);
                    VuP pNode11 = pCell11->getNode (CELLNODE_LEFT);
                    VuP pNode01 = pCell01->getNode (CELLNODE_DOWN);
                    if  (  pNode00
                        && pNode10
                        && pNode11
                        && pNode01
                        && pNode10 == vu_fsucc (pNode00)
                        && pNode11 == vu_fsucc (pNode10)
                        && pNode01 == vu_fsucc (pNode11)
                        && pNode00 == vu_fsucc (pNode01)
                        )
                        {
                        vu_setMask (pNode00, interiorMask);
                        vu_setMask (pNode10, interiorMask);
                        vu_setMask (pNode11, interiorMask);
                        vu_setMask (pNode01, interiorMask);
                        }
                    }
                }
            }

        if (exteriorMask)
            {
            /*  Mark all remaining grid edges exterior */
            for (iy = 0; iy < numY; iy++)
                {
                for (ix = 0; ix < numX; ix++)
                    {
                    //VuP pBaseNode = NULL;
                    GridCell *pCell = gc_getCellInStride (pGC, 0, ix, iy);
                    for (i = 0; i < NUM_NODE_PER_CELL; i++)
                        {
                        VuP pNode = pCell->getNode (i);
                        if (pNode && !vu_getMask (pNode, interiorMask))
                            {
                            vu_setMask (pNode, exteriorMask);
                            }
                        }
                    }
                }
            }

        if (interiorMask && !pGC->interiorMask)
            vu_returnMask (pGraph, interiorMask);
        }
    }

/**
@description Build a grid structure which does not interfere with the edges of the graph.
*/
static void vu_buildNonInterferingGrid_go
(
VuSetP pGraph,
double x0,
double x1,
int numXCell,
bool    bIsPeriodicX,
double y0,
double y1,
int numYCell,
bool    bIsPeriodicY,
VuMask interiorMask,
VuMask exteriorMask
)
    {
    GridContext gc;
    int numCell = numXCell * numYCell;
    static int s_trapId = 21000;

    memset (&gc, 0, sizeof (gc));
    gc.pCellBuffer =  (GridCell *) BSIBaseGeom::Malloc (numCell * sizeof(GridCell));
    memset (gc.pCellBuffer, 0, numCell * sizeof (GridCell));
    gc.pGraph = pGraph;
    gc.interiorMask = interiorMask;
    gc.exteriorMask = exteriorMask;

    gc.strideData[0].init (x0, x1, numXCell, bIsPeriodicX, 1);
    gc.strideData[1].init (y0, y1, numYCell, bIsPeriodicY, numXCell);
    gc_initForIncidenceMarkup (&gc);
    gc_runIncidenceMarkup (&gc);
    //gc_buildShrinkBoxes (&gc, 0.80);
    gc_connectUnoccupiedCellCenters (&gc);
    vu_postGraphToTrapFunc (pGraph, "buildNonInterferingGrid::end", 0, s_trapId);
    BSIBaseGeom::Free (gc.pCellBuffer);
    }

/*---------------------------------------------------------------------------------**//**
@nodoc
@description Build a grid structure that does not interfere with the edges of the graph.
@remarks The graph is assumed to have correct boundary and exterior masks.
@param pGraph   IN OUT graph header
@param numXCell IN number of cells in x direction
@param numYCell IN number of cells in y direction
@param interiorMask IN mask to apply to the 'inside' of completely enclosed
        cells of the grid, i.e. to edge sides not reachable from other parts of the
        graph without crossing grid edges.
@param exteriorMask IN mask to apply to 'outside'
@group "VU Meshing"
@bsimethod                                                      EarlinLutz      02/2006
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_buildNonInterferingGrid
(
VuSetP pGraph,
int numXCell,
int numYCell,
VuMask interiorMask,
VuMask exteriorMask
)
    {
    DRange3d range;
    double minX, maxX, minY, maxY;
    DPoint3d periods;
    bool bIsXPeriodic = false, bIsYPeriodic = false;
    vu_graphRange (pGraph, &range);
    
    minX = range.low.x;
    maxX = range.high.x;
    minY = range.low.y;
    maxY = range.high.y;

    if (minX == maxX || minY == maxY)
        return;         // Avoid divide by zero.

    static double s_shiftFactor = 0.0;  // box portion which is to be outside (so the first interior box
          // snuggles closer to the boundary)
    if (s_shiftFactor != 0.0)
        {
        double eX = (maxX - minX) / numXCell;
        double eY = (maxY - minY) / numYCell;
        minX -= s_shiftFactor * eX;
        maxX += s_shiftFactor * eX;
        minY -= s_shiftFactor * eY;
        maxY += s_shiftFactor * eY;
        }
    vu_getPeriods (pGraph, &periods);

    if (periods.x != 0.0)
        {
        bIsXPeriodic = true;
        minX = 0.0;
        maxX = periods.x;
        }

    if (periods.y != 0.0)
        {
        bIsYPeriodic = true;
        minY = 0.0;
        maxY = periods.y;
        }

    vu_buildNonInterferingGrid_go (pGraph,
                minX, maxX, numXCell, bIsXPeriodic,
                minY, maxY, numYCell, bIsYPeriodic,
                interiorMask, exteriorMask
                );
    }

END_BENTLEY_GEOMETRY_NAMESPACE
