/*--------------------------------------------------------------------------------------+
|
|     $Source: vu/doc/build/bsivudoc/examples/vuSpineContext.h $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <STDVectorDPoint3d.h>
#include <mivu.h>
#include <vector>
#include <algorithm>
//#include <vupolygonclassifier.h>
#ifdef MSVUPRINT_AVAILABLE
#include <msvuprint.fdf>
#endif


// Local struct to pair a Vu node with a double as a sort key for std::sort
struct VuSortKey
    {
    double mA;
    VuP    mpNode;
    VuSortKey (VuP pNode, double a)
        : mA(a), mpNode (pNode) {}
    };

// "Less than" function for sorting Vu nodes with double key:
static bool CompareSortKey (VuSortKey keyA, VuSortKey keyB)
    {
    return keyA.mA < keyB.mA;
    }


static double sRelTol = 1.0e-8;
static double s_graphAbsTol  = 0.0;
static double s_graphRelTol = 1.0e-10;


//***********************************************************************************
// Context manager to hold a vu graph and do spine operations
//
// Spine calculations determine both (a) a "skeletal" network of linework that follows the interior
//      path through wihtin the boundaries, and (b) a block decomposition into quads and triangles.
//
// Usage pattern:
//    VuSpineContext sc();
//    // Data setup ....
//    foreach polygon or polyline
//        {
//        sc.InsertEdges (pXYZ, n, bClosed)
//        }
//    // Analysis steps ...
//    // bParity = true to treat the data as a "polygon".  The interior is determined by parity rules
//                   and the triangulation and spine are only constructed "inside"
//    // bParity = false if "all" spaces are to be triangulated and spined.
//    // minSplitRadians -- suggested value 0.3.  If this value is large, it will encourage add internal
//    //     edges from a vertex to an edge 'across' the polygon even if it creates small angles.
//    // minDiagonalAngle -- suggested value 1.0.  If this value is large (up to about 1.5 as max) it favors
//    //     using triangles to navigate turns.  If it is small, it favors using skewed quadrilaterals.
//    sc.TriangulateForSpine (bParity, minSplitRadians)
//    sc.MarkBoxes (true, minDiagonalAngle);
//    STDVectorDPoint3d xyzOut;
//    sc.GetSpineEdges (xyzOut);
//    for (int i = 0; i < xyzOut.size () - 1; i++)
//          {
//          if (!xyzOut[i].isDisconnect () && !xyzOut[i+1}.isDisconnect ())
//               ... edge from xyzOut[i] to xyzOut[i+1]
//          }
//*************************************************************************************
class VuSpineContext
{
private:
VuSetP mpGraph;     // The evolving graph
VuArrayP mpFaceArray;   // Array for face queries
VuMask   mDiagonalMask; // Mask applied to edges declared "diagonals" in quads.
VuMask   mBoxMask;      // Maks applied to (interior) edges of triangles paired to form a quad.
public:

    int      mNoisy;
protected:

// Add an edge to evolving disconnect-separated output.
void  AddEdge (STDVectorDPoint3dR xyzOut, DPoint3d &xyzA, DPoint3d &xyzB)
    {
    DPoint3d disconnect;
    disconnect.initDisconnect ();
    xyzOut.push_back (xyzA);
    xyzOut.push_back (xyzB);
    xyzOut.push_back (disconnect);
    }


// If possible, return coordinates and nodes for the 4 corners of a box
// around a given node on the diagonal of the box.
bool GetBoxCorners (VuP pDiagonalNode,
VuP &pA,
DPoint3d &xyzA,
VuP &pB,
DPoint3d &xyzB,
VuP &pC,
DPoint3d &xyzC,
VuP &pD,
DPoint3d &xyzD
)
    {
    VuP pDiagonalMate = vu_edgeMate (pDiagonalNode);
    if (  !vu_getMask (pDiagonalNode, VU_BOUNDARY_EDGE)
            && vu_countEdgesAroundFace (pDiagonalNode) == 3
            && vu_countEdgesAroundFace (pDiagonalMate) == 3)
        {
        pA = vu_fsucc (pDiagonalMate);
        pB = vu_fsucc (pA);
        pC = vu_fsucc (pDiagonalNode);
        pD = vu_fsucc (pC);
        vu_getDPoint3d (&xyzA, pA);
        vu_getDPoint3d (&xyzB, pB);
        vu_getDPoint3d (&xyzC, pC);
        vu_getDPoint3d (&xyzD, pD);
        return true;
        }
    return false;
    }

// Compute bisectors of the quad.
// function key is the smaller absolute angle between the bisectors.
// (pi/2 is max possible value).
double DiagonalKeyFunc (VuP pDiagonalNode)
    {
    DPoint3d xyzA, xyzB, xyzC, xyzD;
    VuP     pA,   pB,   pC,   pD;
    if (GetBoxCorners (pDiagonalNode,
                pA, xyzA, pB, xyzB, pC, xyzC, pD, xyzD))
        {
        DPoint3d xyzAB, xyzBC, xyzCD, xyzDA;
        xyzAB.interpolate (&xyzA, 0.5, &xyzB);
        xyzBC.interpolate (&xyzB, 0.5, &xyzC);
        xyzCD.interpolate (&xyzC, 0.5, &xyzD);
        xyzDA.interpolate (&xyzD, 0.5, &xyzA);
        DVec3d vectorAB, vectorBC, vectorCD, vectorDA;

        vectorAB.differenceOf (&xyzB, &xyzA);
        vectorBC.differenceOf (&xyzC, &xyzB);
        vectorCD.differenceOf (&xyzD, &xyzC);
        vectorDA.differenceOf (&xyzA, &xyzD);

        DVec3d vectorAB_CD, vectorBC_DA;
        vectorAB_CD.differenceOf (&xyzAB, &xyzCD);
        vectorBC_DA.differenceOf (&xyzBC, &xyzDA);
        vectorAB_CD.z = 0.0;
        vectorBC_DA.z = 0.0;
        double thetaAB = bsiDVec3d_smallerAngleBetweenUnorientedVectors (&vectorAB_CD, &vectorAB);
        double thetaBC = bsiDVec3d_smallerAngleBetweenUnorientedVectors (&vectorBC_DA, &vectorBC);
        double thetaCD = bsiDVec3d_smallerAngleBetweenUnorientedVectors (&vectorAB_CD, &vectorCD);
        double thetaDA = bsiDVec3d_smallerAngleBetweenUnorientedVectors (&vectorBC_DA, &vectorDA);

        double alpha = thetaAB < thetaCD ? thetaAB : thetaCD;
        double beta  = thetaBC < thetaDA ? thetaBC : thetaDA;
        return alpha > beta ? alpha : beta;
        }

    return - DBL_MAX;
    }

// Select a branch point in a triangle.
// This may be the centroid or the midpoint of an edge joining midpoints of a pair of edges.
void SelectTriangleInteriorPoint (DPoint3d *pXYZ, DPoint3d &xyzInterior)
    {
    DPoint3d xyz[6];
    DPoint3d xyzMid[6];     // Midpoints of each edge.
    DPoint3d interiorCandidate[4];   // for i in {012}, [i] is midpoint of midpoint[i+1] and midpoint[i+2].
                            // [3] is centroid.
    DVec3d   edgeVector[6];
    DPoint3d centroid;
    centroid.zero ();
    for (int i = 0; i < 3; i++)
        {
        xyz[i] = xyz[i+3] = pXYZ[i];
        centroid.add (&xyz[i]);
        }
    centroid.scale (1.0/3.0);

    // Edge midpoints ...
    for (int i = 0; i < 3; i++)
        {
        xyzMid[i].interpolate (&xyz[i], 0.5, &xyz[i+1]);
        xyzMid[i+3] = xyzMid[i];
        edgeVector[i].differenceOf (&xyz[i+1], &xyz[i]);    // use wraparound
        edgeVector[i+3] = edgeVector[i];
        }

    // Midpoints of midpoint-to-midpoint connections ..
    for (int i = 0; i < 3; i++)
        {
        int i1 = i + 1;
        int i2 = i + 2;
        interiorCandidate[i].interpolate (&xyzMid[i1], 0.5, &xyzMid[i2]);
        }

    interiorCandidate[3] = centroid;

    double bestAngle = -DBL_MAX;
    int    bestIndex = -1;
    for (int k = 0; k < 4; k++)
        {
        double theta[3];
        // Measure angles from edge midpoints towards interior candidate.
        double thetaMin = DBL_MAX;
        for (int i = 0; i < 3; i++)
            {
            DVec3d edgeToInterior;
            edgeToInterior.differenceOf (&interiorCandidate[k], &xyzMid[i]);
            theta[i] = bsiDVec3d_smallerAngleBetweenUnorientedVectors (&edgeVector[i], &edgeToInterior);
            if (theta[i] < thetaMin)
                thetaMin = theta[i];
            }
        if (thetaMin > bestAngle)
            {
            bestAngle = thetaMin;
            bestIndex = k;
            }
        }

    xyzInterior = interiorCandidate[bestIndex];
    }

void MarkBox (VuP pA)
    {
    VuP pB = vu_edgeMate (pA);
    vu_setMask (pA, mDiagonalMask);
    vu_setMask (pB, mDiagonalMask);
    vu_setMaskAroundFace (pA, mBoxMask);
    vu_setMaskAroundFace (pB, mBoxMask);
    }

int SetSortedDiagonalMasks (double minA)
    {
    bvector <VuSortKey> candidates;
    int numDiagonal = 0;
    VU_SET_LOOP (pA, mpGraph)
        {
        double a = DiagonalKeyFunc (pA);
        if (a > minA)
            candidates.push_back (VuSortKey (pA, a));
        }
    END_VU_SET_LOOP (pA, mpGraph)

    std::sort (candidates.begin (), candidates.end (), CompareSortKey );

    while (!candidates.empty ())
        {
        VuSortKey key = candidates.back ();
        candidates.pop_back ();
        VuP pA = key.mpNode;
        VuP pB = vu_edgeMate (pA);
        if (   !vu_getMask (pA, mBoxMask)
            && !vu_getMask (pB, mBoxMask))
            {
            MarkBox (pA);
            numDiagonal++;
            }
        }
    return numDiagonal;
    }

/// <param name="xyzA">Vertex whose angle is being split</param>
bool splitOK (DPoint3d &xyzA, DPoint3d &xyzB, DPoint3d xyzQ, DPoint3d xyzC, double minAngle)
    {
    DVec3d vectorAB, vectorAQ, vectorAC;
    vectorAB.differenceOf (&xyzB, &xyzA);
    vectorAQ.differenceOf (&xyzQ, &xyzA);
    vectorAC.differenceOf (&xyzC, &xyzA);
    double angleBAQ = bsiDVec3d_angleBetweenVectorsXY (&vectorAB, &vectorAQ);
    double angleQAC = bsiDVec3d_angleBetweenVectorsXY (&vectorAQ, &vectorAC);
    return fabs (angleBAQ) > minAngle && fabs (angleQAC) > minAngle;
    }

// Search a triangulation for vertices which have
//   (a) angle greater than 90 degrees
// (b) the opposite edge is a boundary.
// Drop a perpenedicular to that boundary.
// return the number of edges added.
int AddPerpendicularsToBoundaries (double minSplitRadians)
    {
    int numAdd = 0;
    VU_SET_LOOP (pA, mpGraph)
        {
        VuP pB = vu_fsucc (pA);
        VuP pC = vu_fsucc (pB);
        if (  !vu_getMask (pA, VU_EXTERIOR_EDGE)
           &&  vu_getMask (pB, VU_BOUNDARY_EDGE)
           &&  vu_fsucc (pC) == pA
           )
            {
            DPoint2d vectorAB, vectorBC, vectorCA;
            vu_getDPoint2dDXY (&vectorAB, pA);
            vu_getDPoint2dDXY (&vectorBC, pB);
            vu_getDPoint2dDXY (&vectorCA, pC);
            if (bsiDPoint2d_dotProduct (&vectorCA, &vectorAB) > 0.0)
                {
                double bb = bsiDPoint2d_dotProduct (&vectorBC, &vectorBC);
                double ba = - bsiDPoint2d_dotProduct (&vectorBC, &vectorAB);
                double s;
                if (bsiTrig_safeDivide (&s, ba, bb, 0.0)
                    && s > 0.0 && s < 1.0)
                    {
                    DPoint3d xyzA, xyzB, xyzC, xyzE;
                    vu_getDPoint3d (&xyzA, pA);
                    vu_getDPoint3d (&xyzB, pB);
                    vu_getDPoint3d (&xyzC, pC);
                    xyzE.interpolate (&xyzB, s, &xyzC);
                    if (splitOK (xyzA, xyzB, xyzE, xyzC, minSplitRadians))
                        {
                        VuP pE, pF; // New nodes on split of far edge.
                        VuP pA1, pE1; // ends of inserted edge.
                        vu_splitEdge (mpGraph, pB, &pE, &pF);
                        vu_setDPoint3dAroundVertex (pE, &xyzE);
                        vu_join (mpGraph, pA, pE, &pA1, &pE1);
                        vu_setDPoint3dAroundVertex (pE, &xyzE);
                        vu_setDPoint3d (pA1, &xyzA);
                        numAdd++;
                        }
                    }
                }
            }
        }
    END_VU_SET_LOOP (pA, mpGraph)
    return numAdd;
    }

bool  GetSpineEdgesInQuad (VuP pFace, STDVectorDPoint3dR xyzOut)
    {
    if (vu_countEdgesAroundFace (pFace) != 4)
        return false;
    VuP pNode[8];
    DPoint3d xyz[8];
    DPoint3d midpoint[8];
    pNode[0] = pNode[4] = pFace;
    pNode[1] = pNode[5] = vu_fsucc (pNode[0]);
    pNode[2] = pNode[6] = vu_fsucc (pNode[1]);
    pNode[3] = pNode[7] = vu_fsucc (pNode[2]);
    int numBoundary = 0;
    int numInterior = 0;
    int iBoundary[4];
    int iInterior[4];
    DPoint3d centroid;
    centroid.zero ();
    for (int i = 0; i < 4; i++)
        {
        if (vu_getMask (pNode[i], VU_BOUNDARY_EDGE))
            iBoundary[numBoundary++] = i;
        else
            iInterior[numInterior++] = i;
        vu_getDPoint3d (&xyz[i], pNode[i]);
        xyz[i+4] = xyz[i];
        centroid.add (&xyz[i]);
        }
    for (int i = 0; i < 4; i ++)
        {
        midpoint[i].interpolate (&xyz[i], 0.5, &xyz[i+1]);
        midpoint[i+4] = midpoint[i];
        }

    centroid.scale (0.25);
    if (numBoundary == 0 || numBoundary == 1)
        {
        for (int i = 0; i < numInterior; i++)
            AddEdge (xyzOut, midpoint[iInterior[i]], centroid);
        }
    else if (numBoundary == 4)
        {
        for (int i = 0; i < numBoundary; i++)
            AddEdge (xyzOut, midpoint[i], centroid);
        }
    else if (numBoundary == 2)
        {
        if (iInterior[1] == iInterior[0] + 2)
            {
            // Spine enters one end, exits the other ..
            AddEdge (xyzOut, midpoint[iInterior[0]], midpoint [iInterior[1]]);
            }
        else
            {
            // Block sits as exterior corner.  Let the two spines continue to their opposite faces ..
            for (int i = 0; i < 4; i++)
                AddEdge (xyzOut, midpoint[i], centroid);
            }
        }
    else if (numBoundary == 3)
        {
        AddEdge (xyzOut, midpoint[iInterior[0]], midpoint [iInterior[0] + 2]);
        }
    return true;
    }

// Find pseudo spine edges. Return coordinates in DISCONNECT-deliminated array.
bool  GetSpineEdgesInTriangle (VuP pFace, STDVectorDPoint3dR xyzOut)
    {
    if (vu_countEdgesAroundFace (pFace) != 3)
        return false;
    int n = 0;
    DPoint3d xyz[6], xyzCentroid;
    DPoint3d xyzMidpoint[6];
    xyzCentroid.zero ();
    bool isBoundary[6];
    int numBoundary = 0;
    int lastBoundary = -1;
    int lastInterior = -1;
    VU_FACE_LOOP (pCurr, pFace)
        {
        vu_getDPoint3d (&xyz[n], pCurr);
        xyzCentroid.sumOf (&xyzCentroid, &xyz[n]);
        isBoundary[n] = false;
        if (vu_getMask (pCurr, VU_BOUNDARY_EDGE))
            {
            isBoundary[n] = true;
            numBoundary++;
            lastBoundary = n;
            }
        else
            {
            lastInterior = n;
            }
        xyz[n+3] = xyz[n];
        isBoundary[n+3] = isBoundary[n];
        n++;
        }
    END_VU_FACE_LOOP (pCurr, pFace)

    for (int i = 0; i < 3; i++)
        {
        xyzMidpoint[i].interpolate (&xyz[i], 0.5, &xyz[i+1]);
        xyzMidpoint[i+3] = xyzMidpoint[i];
        }

    xyzCentroid.scale (1.0 / 3.0);

    if (numBoundary == 0)
        {
        // Interior branch
        DPoint3d xyzInterior;
        SelectTriangleInteriorPoint (xyz, xyzInterior);
        AddEdge (xyzOut, xyzMidpoint[0], xyzInterior);
        AddEdge (xyzOut, xyzMidpoint[1], xyzInterior);
        AddEdge (xyzOut, xyzMidpoint[2], xyzInterior);
        }
    else if (numBoundary == 1)
        {
        AddEdge (xyzOut, xyzMidpoint[lastBoundary+1], xyzMidpoint[lastBoundary+2]);
        }
    else if (numBoundary == 2)
        {
        AddEdge (xyzOut, xyzMidpoint[lastInterior], xyz[lastInterior + 2]);
        }
    else if (numBoundary == 3)
        {
        DPoint3d xyzInterior;
        SelectTriangleInteriorPoint (xyz, xyzInterior);
        AddEdge (xyzOut, xyzMidpoint[0], xyzInterior);
        AddEdge (xyzOut, xyzMidpoint[1], xyzInterior);
        AddEdge (xyzOut, xyzMidpoint[2], xyzInterior);
        }
    return true;
    }

public:

// Prepare for a scan of interior faces.
void InitFaceScan ()
    {
    vu_arrayClear (mpFaceArray);
    vu_collectInteriorFaceLoops (mpFaceArray, mpGraph);
    vu_arrayOpen (mpFaceArray);
    }

// Search an array for "disconnect" points separating multiple polylines
// Add each polyline section to the vu graph.
void InsertEdges (STDVectorDPoint3dR xyzIn, bool bClosed)
    {
    int i1;
    int numXYZ = xyzIn.size ();
    for (int i0 = 0; i0 < numXYZ - 1; i0 = ++i1)
        {
        i1 = i0;
        while (i1 < numXYZ && !bsiDPoint3d_isDisconnect (&xyzIn[i1]))
            i1++;
        int numThisLoop = i1 - i0;
        VuP pFirstA = NULL;
        VuP pPreviousB = NULL;
        for (int i = i0 + 1; i < i1; i++)
            {
            VuP pNodeA, pNodeB;
            vu_makePair (mpGraph, &pNodeA, &pNodeB);
            vu_setDPoint3d (pNodeA, &xyzIn[i-1]);
            vu_setDPoint3d (pNodeB, &xyzIn[i]);
            vu_setMask (pNodeA, VU_BOUNDARY_EDGE);
            vu_setMask (pNodeB, VU_BOUNDARY_EDGE);
            if (pPreviousB == NULL)
                {
                pFirstA = pNodeA;
                }
            else
                {
                vu_vertexTwist (mpGraph, pPreviousB, pNodeA);
                }
            pPreviousB = pNodeB;
            }
        if (bClosed && pFirstA != NULL && pPreviousB != NULL)
            vu_vertexTwist (mpGraph, pPreviousB, pFirstA);
        }
    }


// Compute triangulation.
/// <param nam="bMarkParity">true to treat the geometry as closed, false for general subdivision</param>
/// <param name="minSplitRadians"
void TriangulateForSpine (bool bMarkParity, double minSplitRadians)
    {
    vu_mergeOrUnionLoops (mpGraph, VUUNION_KEEP_ONE_AMONG_DUPLICATES);
    vureg_regularizeGraph (mpGraph);
    if (bMarkParity)
        vu_markAlternatingExteriorBoundaries (mpGraph, true);
    vu_triangulateMonotoneInteriorFaces (mpGraph, false);
    vu_flipTrianglesToImproveQuadraticAspectRatio (mpGraph);
    while (AddPerpendicularsToBoundaries (minSplitRadians) > 0)
        vu_flipTrianglesToImproveQuadraticAspectRatio (mpGraph);
    }


// Find pseudo spine edges. Return coordinates in DISCONNECT-deliminated array.
void  GetSpineEdges (STDVectorDPoint3dR xyzOut)
    {
    xyzOut.clear ();
    vu_arrayClear (mpFaceArray);
    vu_collectInteriorFaceLoops (mpFaceArray, mpGraph);
    vu_arrayOpen (mpFaceArray);
    VuP pFace;
    for (vu_arrayOpen (mpFaceArray);
         NULL != vu_arrayRead (mpFaceArray, &pFace);)
        {
        if (GetSpineEdgesInTriangle (pFace, xyzOut))
            {
            }
        else if (GetSpineEdgesInQuad (pFace, xyzOut))
            {
            }
        }
    }


// Constructor.  Create vu graph and face array to be reused ...
VuSpineContext ()
    {
    mpGraph = vu_newVuSet (0);
    mpFaceArray = vu_grabArray (mpGraph);
    vu_setTol (mpGraph, s_graphAbsTol, s_graphRelTol);
    mDiagonalMask = vu_grabMask (mpGraph);
    mBoxMask      = vu_grabMask (mpGraph);
    mNoisy = 0;
    }

// Destructor.  Dispose of array and graph ...
~VuSpineContext ()
    {
    vu_returnArray (mpGraph, mpFaceArray);
    vu_freeVuSet (mpGraph);
    }



// Retrieve the "next" face of the graph.
// This is to be called in a loop
//     for (;GetFace (xyzOut));)
//          {
//          }
//  Once the faces are visited, use InitFaceScan() to reset for additional visits.
bool GetFace (STDVectorDPoint3dR xyzOut)
    {
    xyzOut.clear ();
    VuP pFaceSeed;
    // We're going to ignore faces with just 2 edges or zero area ..
    //   have to be in a loop even though we expect to "usually" read
    //   only once.
    for (;vu_arrayRead (mpFaceArray, &pFaceSeed); xyzOut.clear ())
        {
        VU_FACE_LOOP (pCurr, pFaceSeed)
            {
            DPoint3d xyzCurr;
            vu_getDPoint3d (&xyzCurr, pCurr);
            xyzOut.push_back (xyzCurr);
            }
        END_VU_FACE_LOOP (pCurr, pFaceSeed)

        if (xyzOut.size () > 2 && vu_area (pFaceSeed) != 0.0)
            {
            xyzOut.push_back (xyzOut[0]);
            return true;
            }
        }
    return false;
    }

// Search for interior diagonals that can be removed to leave "boxes"
// Boxes are marked for further use.
// Optinoally delete the diagonals.
// <param name="bDeleteDiagonals">true to delete diagonals as quads are formed.</param>
// <param name="minAngleRadians">Consider a pair of triangles that share an edge.  Consider the
//             quad formed by joining the triangles.  Form segemnts between the midpoints of opposite
//             edges of the quad.  Compute the smallest angle of these segments with their edges.
//             If this angle is larger than minAngleRadians, this quad is good.
//             Suggested values are between 0.3. and 1.2.
//  </param>
int MarkBoxes (bool bDeleteDiagonals, double minAngleRadians)
    {
    int numDiagonal = SetSortedDiagonalMasks (minAngleRadians);
    if (bDeleteDiagonals && numDiagonal > 0)
        vu_freeMarkedEdges (mpGraph, mDiagonalMask);
    return numDiagonal;
    }
};

