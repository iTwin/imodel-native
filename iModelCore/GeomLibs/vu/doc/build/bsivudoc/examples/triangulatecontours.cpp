/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Vu.h>

#define MAX_SEGMENTS_PER_LONG_FRINGE_EDGE 24
#define MIN_SEGMENTS_PER_LONG_FRINGE_EDGE 4

static void    loadSufficientlyLargeContours
(
VuSetP      pGraph,     // graph to receive contours
DRange3d*   pRange,     // range of resulting graph
DPoint3d*   xyzIn,      // polylines separated by DISCONNECT points
int         numIn       // number of input vertices
)
    {
    static double   s_minEdgeFactor = 1.0e-4;
    static double   s_relTol = 4.0e-2;

    bsiDRange3d_initFromArray (pRange, xyzIn, numIn);
    double smallEdgeTol = s_minEdgeFactor * s_relTol * bsiDPoint3d_distanceXY (&pRange->low, &pRange->high);

    DPoint3d*   pBuffer = xyzIn;
    int         i = 0, numPointsThisContour = 0;
    while (i < numIn)
        {
        if (!bsiDPoint3d_isDisconnect (&xyzIn[i++]))
            {
            numPointsThisContour++;

            // continue accumulating this contour if there are more points; otherwise fall through to load the final contour
            if (i < numIn)
                continue;
            }

        // load the contour
        if (numPointsThisContour > 0)
            {
            // Note: no EXTERIOR masks yet...
            vu_makeEdgesFromArray3d (pGraph, pBuffer, numPointsThisContour, VU_BOUNDARY_EDGE, VU_BOUNDARY_EDGE, smallEdgeTol, -1.0);
            pBuffer += numPointsThisContour;
            numPointsThisContour = 0;
            }

        if (i < numIn)
            pBuffer++;  // move past disconnect
        }
    }

static void    addBoundingRectangle
(
VuSetP      pGraph,         // graph to receive fringe rectangle beyond xy-extents
DRange3d*   pOutRange,      // expanded graph range; fringe at height pInRange->low.z
DRange3d*   pInRange,       // graph range
double      absDelta,       // absolute minimum fringe delta
double      relDelta,       // minimum fringe delta as fraction of range diagonal
int         numOnLongEdge   // # segments into which to divide long sides of rectangle
)
    {
    double  delta = 0.0;
    double  diagonal = bsiDPoint3d_distanceXY (&pInRange->low, &pInRange->high);
    if (relDelta > 0.0)
        delta = relDelta * diagonal;
    if (absDelta > delta)
        delta = absDelta;

    double xMin = pInRange->low.x - delta;
    double xMax = pInRange->high.x + delta;
    double yMin = pInRange->low.y - delta;
    double yMax = pInRange->high.y + delta;
    double zz   = pInRange->low.z;

    double dx = xMax - xMin;
    double dy = yMax - yMin;
    if (dx == 0.0 || dy == 0.0)
        return;

    if (numOnLongEdge > MAX_SEGMENTS_PER_LONG_FRINGE_EDGE)
        numOnLongEdge = MAX_SEGMENTS_PER_LONG_FRINGE_EDGE;
    if (numOnLongEdge < MIN_SEGMENTS_PER_LONG_FRINGE_EDGE)
        numOnLongEdge = MIN_SEGMENTS_PER_LONG_FRINGE_EDGE;

    int numX, numY;
    numX = numY = numOnLongEdge;
    if (dx < dy)
        numX = (int) ((numX * dx) / dy);
    else if (dy < dx)
        numY = (int) ((numY * dy) / dx);
    if (numX <= 0)
        numX = 1;
    if (numY <= 0)
        numY = 1;

    DPoint3d    pBuffer[MAX_SEGMENTS_PER_LONG_FRINGE_EDGE * 4 + 1];
    bsiDPoint3d_setXYZ (pBuffer, xMin, yMin, zz);

    // bottom edge
    DSegment3d  rectangleEdge;
    bsiDSegment3d_initFromStartEndXYZXYZ (&rectangleEdge, xMin, yMin, zz, xMax, yMin, zz);
    bsiDSegment3d_interpolateUniformDPoint3dArray (&rectangleEdge, pBuffer + 1, numX - 1, false);
    pBuffer[numX] = rectangleEdge.point[1];

    // right edge
    bsiDSegment3d_initFromStartEndXYZXYZ (&rectangleEdge, xMax, yMin, zz, xMax, yMax, zz);
    bsiDSegment3d_interpolateUniformDPoint3dArray (&rectangleEdge, pBuffer + numX + 1, numY - 1, false);
    pBuffer[numX + numY] = rectangleEdge.point[1];

    // top edge
    bsiDSegment3d_initFromStartEndXYZXYZ (&rectangleEdge, xMax, yMax, zz, xMin, yMax, zz);
    bsiDSegment3d_interpolateUniformDPoint3dArray (&rectangleEdge, pBuffer + numX + numY + 1, numX - 1, false);
    pBuffer[numX + numY + numX] = rectangleEdge.point[1];

    // left edge
    bsiDSegment3d_initFromStartEndXYZXYZ (&rectangleEdge, xMin, yMax, zz, xMin, yMin, zz);
    bsiDSegment3d_interpolateUniformDPoint3dArray (&rectangleEdge, pBuffer + numX + numY + numX + 1, numY - 1, false);
    pBuffer[numX + numY + numX + numY] = rectangleEdge.point[1];

    // note: no EXTERIOR masks yet...
    vu_makeEdgesFromArray3d (pGraph, pBuffer, numX + numY + numX + numY + 1, VU_BOUNDARY_EDGE, VU_BOUNDARY_EDGE, 0.0, 0.0);
    bsiDRange3d_initFrom2Components (pOutRange, xMin, yMin, zz, xMax, yMax, zz);
    }

static void    markExteriorFaces
(
VuSetP  pGraph,
double  minInteriorFaceArea
)
    {
    VuArrayP    pFaceArray = vu_grabArray (pGraph);
    vu_collectInteriorFaceLoops (pFaceArray, pGraph);

    vu_clearMaskInSet (pGraph, VU_EXTERIOR_EDGE);

    VuP pCurrFaceSeed;
    for (vu_arrayOpen (pFaceArray); vu_arrayRead (pFaceArray, &pCurrFaceSeed);)
        {
        if (vu_area (pCurrFaceSeed) < minInteriorFaceArea)
            vu_setMaskAroundFace (pCurrFaceSeed, VU_EXTERIOR_EDGE);
        }

    vu_returnArray (pGraph, pFaceArray);
    }

static void    spreadExteriorMasksToAdjacentFaces
(
VuSetP  pGraph
)
    {
    VuMask myMask = vu_grabMask (pGraph);

    // Set mask around all faces with any edge touching a vertex with any exterior edges
    vu_clearMaskInSet (pGraph, myMask);
    VU_SET_LOOP (pBase, pGraph)
        {
        if (vu_getMask (pBase, VU_EXTERIOR_EDGE))
            {
            VU_VERTEX_LOOP (pCurr, pBase)
                {
                if (!vu_getMask (pCurr, myMask))
                    vu_setMaskAroundFace (pCurr, myMask);
                }
            END_VU_VERTEX_LOOP (pCurr, pBase)
            }
        }
    END_VU_SET_LOOP (pBase, pGraph)

    // Convert masked edges to exterior
    VU_SET_LOOP (pBase, pGraph)
        {
        if (vu_getMask (pBase, myMask))
            vu_setMask (pBase, VU_EXTERIOR_EDGE);
        }
    END_VU_SET_LOOP (pBase, pGraph)

    vu_returnMask (pGraph, myMask);
    }

static void    collectTriangulationArrays
(
bvector<DPoint3d>&  xyzOut,             // packed array of vertices of triangulation
bvector<int>&       xyzIndicesOut,      // triples of 0-based vertex indices of triangles
VuSetP                  pGraph              // user data field is used to store vertex index
)
    {
    int nullIndex = -1;

    // Mark all vertices as unknown using the user data field
    VU_SET_LOOP (currP, pGraph)
        {
        vu_setUserDataPAsInt (currP, nullIndex);
        }
    END_VU_SET_LOOP (currP, pGraph)

    VuArrayP    faceArrayP = vu_grabArray (pGraph);
    vu_collectInteriorFaceLoops (faceArrayP, pGraph);

    xyzOut.clear();
    xyzIndicesOut.clear();

    // Write the face loop indices
    VuP         faceP;
    DPoint3d    xyz;
    int         vertexIndex;
    for (vu_arrayOpen (faceArrayP); vu_arrayRead (faceArrayP, &faceP);)
        {
        VU_FACE_LOOP (currP, faceP)
            {
            vertexIndex = vu_getUserDataPAsInt (currP);
            if (vertexIndex == nullIndex)
                {
                vertexIndex = xyzOut.size();

                // Add newly encountered vertex to packed output array
                vu_getDPoint3d (&xyz, currP);
                xyzOut.push_back (xyz);

                // Remember the index for this VU vertex
                VU_VERTEX_LOOP (vertP, currP)
                    {
                    vu_setUserDataPAsInt (vertP, vertexIndex);
                    }
                END_VU_VERTEX_LOOP (vertP, currP)
                }

            // Add vertex index to this face loop
            xyzIndicesOut.push_back (vertexIndex);
            }
        END_VU_FACE_LOOP (currP, faceP)
        }

    vu_returnArray (pGraph, faceArrayP);
    }

Public void     myTriangulateContours
(
bvector<DPoint3d>&  xyzOut,             // packed array of vertices of triangulation
bvector<int>&       xyzIndicesOut,      // triples of 0-based vertex indices of triangles
DPoint3d*               xyzIn,              // polylines separated by DISCONNECT points
int                     numIn               // number of input vertices
)
    {
    static double   s_graphAbsTol = 0.0;
    static double   s_graphRelTol = 1.0e-9;
    static double   s_areaRelTol = 0.1;

    xyzOut.clear();
    xyzIndicesOut.clear();

    if (numIn <= 0)
        return;

    VuSetP  pGraph;
    pGraph = vu_newVuSet (0);

    // Load sufficiently large contours
    DRange3d    range;
    loadSufficientlyLargeContours (pGraph, &range, xyzIn, numIn);
    if (vu_emptyGraph (pGraph))
        {
        vu_freeVuSet (pGraph);
        return;
        }

    // Compute and load a bounding rectangle
    DRange3d    expandedRange;
    addBoundingRectangle (pGraph, &expandedRange, &range, 0.0, 0.20, (int) (0.5 * sqrt ((double) numIn)));

    // Triangulate expanded graph
    vu_merge2002 (pGraph, VUUNION_KEEP_ONE_AMONG_DUPLICATES, s_graphAbsTol, s_graphRelTol);
    vu_regularizeGraph (pGraph);
    vu_splitMonotoneFacesToEdgeLimit (pGraph, 3);

    // Mark the face with obviously negative area (the outside of the bounding rectangle exterior)
    markExteriorFaces (pGraph, - s_areaRelTol * (range.high.x - range.low.x) * (range.high.y - range.low.y));

    // This (together with bounding rectangle) helps to minimize new edges in the convex hull
    vu_flipTrianglesToImproveQuadraticAspectRatio (pGraph);
    spreadExteriorMasksToAdjacentFaces (pGraph);

    // Fill outputs
    collectTriangulationArrays (xyzOut, xyzIndicesOut, pGraph);

    vu_freeVuSet (pGraph);
    }
