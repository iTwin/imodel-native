/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/



static double sSpineRelTol = 1.0e-8;
static double sSpineGraphAbsTol  = 0.0;
static double sSpineGraphRelTol = 1.0e-10;


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
//    bvector<bvector<DPoint3d>> xyzOut;
//    sc.GetSpineEdges (xyzOut);
//*************************************************************************************
class VuSpineContext
{
private:
VuSetP mpGraph;     // The evolving graph
VuArrayP mpFaceArray;   // Array for face queries
VuMask   mDiagonalMask; // Mask applied to edges declared "diagonals" in quads.
VuMask   mBoxMask;      // Maks applied to (interior) edges of triangles paired to form a quad.


// Local struct to pair a Vu node with a double as a sort key for std::sort
struct VuSortKey
    {
    double mA;
    VuP    mpNode;
    VuSortKey (VuP pNode, double a)
        : mA(a), mpNode (pNode) {}
    bool operator < (VuSortKey const &other) const
        {
        return mA < other.mA;
        }
    };




public:

    int      mNoisy;
protected:

// Add an edge (as new bvector<DPoint3d> at back>
void  AddEdge (bvector<bvector<DPoint3d>> & xyzOut, DPoint3d &xyzA, DPoint3d &xyzB)
    {
    xyzOut.push_back (bvector<DPoint3d> ());
    xyzOut.back ().push_back (xyzA);
    xyzOut.back ().push_back (xyzB);
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
        xyzAB.Interpolate (xyzA, 0.5, xyzB);
        xyzBC.Interpolate (xyzB, 0.5, xyzC);
        xyzCD.Interpolate (xyzC, 0.5, xyzD);
        xyzDA.Interpolate (xyzD, 0.5, xyzA);
        DVec3d vectorAB, vectorBC, vectorCD, vectorDA;

        vectorAB.DifferenceOf (xyzB, xyzA);
        vectorBC.DifferenceOf (xyzC, xyzB);
        vectorCD.DifferenceOf (xyzD, xyzC);
        vectorDA.DifferenceOf (xyzA, xyzD);

        DVec3d vectorAB_CD, vectorBC_DA;
        vectorAB_CD.DifferenceOf (xyzAB, xyzCD);
        vectorBC_DA.DifferenceOf (xyzBC, xyzDA);
        vectorAB_CD.z = 0.0;
        vectorBC_DA.z = 0.0;
        double thetaAB = vectorAB_CD.SmallerUnorientedAngleTo (vectorAB);
        double thetaBC = vectorBC_DA.SmallerUnorientedAngleTo (vectorBC);
        double thetaCD = vectorAB_CD.SmallerUnorientedAngleTo (vectorCD);
        double thetaDA = vectorBC_DA.SmallerUnorientedAngleTo (vectorDA);
        
        double alpha = thetaAB < thetaCD ? thetaAB : thetaCD;
        double beta  = thetaBC < thetaDA ? thetaBC : thetaDA;
        return alpha < beta ? alpha : beta;
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
    centroid.Zero ();
    for (int i = 0; i < 3; i++)
        {
        xyz[i] = xyz[i+3] = pXYZ[i];
        centroid.Add (xyz[i]);
        }
    centroid.Scale (1.0/3.0);

    // Edge midpoints ...
    for (int i = 0; i < 3; i++)
        {
        xyzMid[i].Interpolate (xyz[i], 0.5, xyz[i+1]);
        xyzMid[i+3] = xyzMid[i];
        edgeVector[i].DifferenceOf (xyz[i+1], xyz[i]);    // use wraparound
        edgeVector[i+3] = edgeVector[i];
        }

    // Midpoints of midpoint-to-midpoint connections ..
    for (int i = 0; i < 3; i++)
        {
        int i1 = i + 1;
        int i2 = i + 2;
        interiorCandidate[i].Interpolate (xyzMid[i1], 0.5, xyzMid[i2]);
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
            edgeToInterior.DifferenceOf (interiorCandidate[k], xyzMid[i]);
            theta[i] = edgeVector[i].SmallerUnorientedAngleTo (edgeToInterior);
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

    std::sort (candidates.begin (), candidates.end ());
  
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
    vectorAB.DifferenceOf (xyzB, xyzA);
    vectorAQ.DifferenceOf (xyzQ, xyzA);
    vectorAC.DifferenceOf (xyzC, xyzA);
    double angleBAQ = vectorAB.AngleToXY (vectorAQ);
    double angleQAC = vectorAQ.AngleToXY (vectorAC);
    return fabs (angleBAQ) > minAngle && fabs (angleQAC) > minAngle;
    }

// Search a triangulation for vertices which have
//   (a) pre-split angle greater than 90 degrees
// (b) the opposite edge is a boundary.
//  (c) each post split angle is less than minSplitRadians
// Drop a perpenedicular to that boundary.
// return the number of edges added.
int AddPerpendicularsToBoundaries (double minSplitRadians, double minCandidateRadians)
    {
    int numAdd = 0;
    VU_SET_LOOP (pA, mpGraph)
        {
        VuP pB = vu_fsucc (pA);
        VuP pC = vu_fsucc (pB);
        if (  !vu_getMask (pA, VU_EXTERIOR_EDGE)
            && !vu_getMask (pA, VU_BOUNDARY_EDGE)   // ?? prevent deep recursion
            && !vu_getMask (pC, VU_BOUNDARY_EDGE)   // ?? prevent deep recursion
           &&  vu_getMask (pB, VU_BOUNDARY_EDGE)
           &&  vu_fsucc (pC) == pA
           )
            {
            DVec2d vectorAB, vectorBC, vectorCA;
            vu_getDPoint2dDXY (&vectorAB, pA);
            vu_getDPoint2dDXY (&vectorBC, pB);
            vu_getDPoint2dDXY (&vectorCA, pC);
            double candidateRadians = msGeomConst_pi - vectorCA.AngleTo (vectorAB);
            //double candidateDot     = vectorCA.DotProduct (vectorAB);
            if (candidateRadians > minCandidateRadians)//vectorCA.DotProduct (vectorAB) > 0.0)
                {
                double bb = vectorBC.DotProduct (vectorBC);
                double ba = -vectorBC.DotProduct (vectorAB);
                double s;
                if (DoubleOps::SafeDivide (s, ba, bb, 0.0)
                    && s > 0.0 && s < 1.0)
                    {
                    DPoint3d xyzA, xyzB, xyzC, xyzE;
                    vu_getDPoint3d (&xyzA, pA);
                    vu_getDPoint3d (&xyzB, pB);
                    vu_getDPoint3d (&xyzC, pC);
                    xyzE.Interpolate (xyzB, s, xyzC);
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

bool  GetSpineEdgesInQuad
(
VuP pFace,
bvector<bvector<DPoint3d>> & xyzOut,
bool bIncludeInterior,
bool bIncludeFinal,         // true to include the edge to boundary when the qued is a dead end.
bool bIncludeCornerSpokes  // true to include the two adjacent edges to boundary if the quad is at a corner.
)
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
    bool bIsBoundary[4];
    DPoint3d centroid;
    centroid.Zero ();
    for (int i = 0; i < 4; i++)
        {
        bIsBoundary[i] = 0 != vu_getMask (pNode[i], VU_BOUNDARY_EDGE);
        if (vu_getMask (pNode[i], VU_BOUNDARY_EDGE))
            iBoundary[numBoundary++] = i;
        else
            iInterior[numInterior++] = i;
        vu_getDPoint3d (&xyz[i], pNode[i]);
        xyz[i+4] = xyz[i];
        centroid.Add (xyz[i]);
        }
    for (int i = 0; i < 4; i ++)
        {
        midpoint[i].Interpolate (xyz[i], 0.5, xyz[i+1]);
        midpoint[i+4] = midpoint[i];
        }

    centroid.Scale (0.25);
    if (numBoundary == 0 || numBoundary == 1)
        {
        for (int i = 0; i < numInterior; i++)
            if (bIncludeInterior)
                AddEdge (xyzOut, midpoint[iInterior[i]], centroid);
        }
    else if (numBoundary == 4)
        {
        for (int i = 0; i < numBoundary; i++)
            if (bIncludeFinal)
                AddEdge (xyzOut, midpoint[i], centroid);
        }
    else if (numBoundary == 2)
        {
        if (iInterior[1] == iInterior[0] + 2)
            {
            // Spine enters one end, exits the other ..
            if (bIncludeInterior)
                AddEdge (xyzOut, midpoint[iInterior[0]], midpoint [iInterior[1]]);
            }
        else
            {
            // Block sits as exterior corner.  Let the two spines continue to their opposite faces ..
            for (int i = 0; i < 4; i++)
                if ((bIsBoundary[i] && bIncludeCornerSpokes)
                   || (!bIsBoundary[i] && bIncludeInterior))
                    AddEdge (xyzOut, midpoint[i], centroid);
            }
        }
    else if (numBoundary == 3)
        {
        if (bIncludeInterior)
            AddEdge (xyzOut, midpoint[iInterior[0]], centroid);
        if (bIncludeFinal)
            AddEdge (xyzOut, centroid, midpoint [iInterior[0] + 2]);
        }
    return true;
    }


void expandProjectedRange
(
double &f0,
double &f1,
DPoint3dR P,
DPoint2dR C,
DVec2dR U
)
    {
    double f = (P.x - C.x) * U.x + (P.y - C.y) * U.y;
    if (f < f0)
        f0 = f;
    if (f > f1)
        f1 = f;
    }
    
// On unbounded line that bisects A0A1 and B0B1, return segment
// within projects of A0,A1,B0,B1,C
bool GetBisector
(
bvector<bvector<DPoint3d>> & xyzOut,
DPoint3dR A0,
DPoint3dR A1,
DPoint3dR B0,
DPoint3dR B1,
DPoint3dR E     // Additional point.
)
    {
    DVec2d UA, UB, U, VA, W;
    UA.x = A1.x - A0.x;
    UA.y = A1.y - A0.y;
    UB.x = B1.x - B0.x;
    UB.y = B1.y - B0.y;
    W.x = A0.x - B0.x;
    W.y = A0.y - B0.y;
    UA.Normalize ();
    UB.Normalize ();
    U.SumOf (UA,UB);
    U.Normalize ();
    VA.x =  UA.y;
    VA.y = -UA.x;
    double e = W.CrossProduct (UB);
    double f = VA.CrossProduct (UB);
    double lambda;
    static double se = -1.0;
    static double sf = 1.0;
    DoubleOps::SafeDivide (lambda, se * e, f + sf, 0.0);
    if (true)
        {
        DPoint2d C;
        C.Init (A0);
        C.SumOf (C,VA, lambda);
        double f0 = 0.0;
        double f1 = 0.0;    // f0,f1 are projected range of A0 alone .. expand for others...
        expandProjectedRange (f0, f1, A1, C, U);
        expandProjectedRange (f0, f1, B0, C, U);
        expandProjectedRange (f0, f1, B1, C, U);
        expandProjectedRange (f0, f1, E, C, U);
        DPoint2d P0, P1;
        DPoint3d Q0, Q1;
        P0.SumOf (C,U, f0);
        P1.SumOf (C,U, f1);
        Q0.Init (P0.x, P0.y, A0.z);
        Q1.Init (P1.x, P1.y, A0.z);
        AddEdge (xyzOut, Q0, Q1);
        return true;
        }
    return false;
    }

bool  GetLocalBisectorEdgesInQuad
(
VuP pFace,
bvector<bvector<DPoint3d>> & xyzOut
)
    {
    if (vu_countEdgesAroundFace (pFace) != 4)
        return false;
    VuP pNode[8];
    DPoint3d xyz[8];

    pNode[0] = pNode[4] = pFace;
    pNode[1] = pNode[5] = vu_fsucc (pNode[0]);
    pNode[2] = pNode[6] = vu_fsucc (pNode[1]);
    pNode[3] = pNode[7] = vu_fsucc (pNode[2]);
    
    for (int i = 0; i < 4; i++)
        {
        vu_getDPoint3d (&xyz[i], pNode[i]);
        xyz[i+4] = xyz[i];
        }
    // corner angle bisectors ...
    for (int i = 0; i < 4; i++)
        GetBisector (xyzOut, xyz[i], xyz[i+1], xyz[i], xyz[i+3], xyz[i+2]);
    // opposite side bisectors
    for (int i = 0; i < 2; i++)
        GetBisector (xyzOut, xyz[i], xyz[i+1], xyz[i+3], xyz[i+2], xyz[i]);
        
    return true;
    }






bool  GetSpineEdgesInTriangle
(
VuP pFace, bvector<bvector<DPoint3d>> & xyzOut,
bool bIncludeInterior,
bool bIncludeFinal
)
    {
    if (vu_countEdgesAroundFace (pFace) != 3)
        return false;
    int n = 0;
    DPoint3d xyz[6], xyzCentroid;
    DPoint3d xyzMidpoint[6];
    xyzCentroid.Zero ();
    bool isBoundary[6];
    int numBoundary = 0;
    int lastBoundary = -1;
    int lastInterior = -1;
    VU_FACE_LOOP (pCurr, pFace)
        {
        vu_getDPoint3d (&xyz[n], pCurr);
        xyzCentroid.SumOf (xyzCentroid,xyz[n]);
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
        xyzMidpoint[i].Interpolate (xyz[i], 0.5, xyz[i+1]);
        xyzMidpoint[i+3] = xyzMidpoint[i];
        }

    xyzCentroid.Scale (1.0 / 3.0);

    if (numBoundary == 0)
        {
        // Interior branch 
        DPoint3d xyzInterior;
        SelectTriangleInteriorPoint (xyz, xyzInterior);
        if (bIncludeInterior)
            {
            AddEdge (xyzOut, xyzMidpoint[0], xyzInterior);
            AddEdge (xyzOut, xyzMidpoint[1], xyzInterior);
            AddEdge (xyzOut, xyzMidpoint[2], xyzInterior);
            }
        }
    else if (numBoundary == 1)
        {
        if (bIncludeInterior)
            AddEdge (xyzOut, xyzMidpoint[lastBoundary+1], xyzMidpoint[lastBoundary+2]);        
        }
    else if (numBoundary == 2)
        {
        if (bIncludeFinal && lastInterior >= 0)
            AddEdge (xyzOut, xyzMidpoint[lastInterior], xyz[lastInterior + 2]);
        }
    else if (numBoundary == 3)
        {
        DPoint3d xyzInterior;
        SelectTriangleInteriorPoint (xyz, xyzInterior);
        if (bIncludeFinal)
            {
            AddEdge (xyzOut, xyzMidpoint[0], xyzInterior);
            AddEdge (xyzOut, xyzMidpoint[1], xyzInterior);
            AddEdge (xyzOut, xyzMidpoint[2], xyzInterior);
            }
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

// Add a a polyline to the graph.  This may be called multiple times.
void InsertEdges (bvector<DPoint3d> & xyzIn, bool bClosed)
    {
    VuP pPreviousB = nullptr, pFirstA = nullptr;
    for (size_t i = 1; i < xyzIn.size (); i++)
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


// Compute triangulation.
/// <param nam="bMarkParity">true to treat the geometry as closed, false for general subdivision</param>
/// <param name="minSplitRadians"
void TriangulateForSpine (bool bMarkParity, double minSplitRadians)
    {
    static int sMaxSplit = 20;
    static double sMinCandidateRadians = 1.0;
    int numSplit = 0;
    vu_mergeOrUnionLoops (mpGraph, VUUNION_KEEP_ONE_AMONG_DUPLICATES);
    vureg_regularizeGraph (mpGraph);
    if (bMarkParity)
        vu_markAlternatingExteriorBoundaries (mpGraph, TRUE);
    vu_triangulateMonotoneInteriorFaces (mpGraph, FALSE);
    vu_flipTrianglesToImproveQuadraticAspectRatio (mpGraph);
    while (numSplit++ < sMaxSplit && AddPerpendicularsToBoundaries (minSplitRadians, sMinCandidateRadians) > 0)
        vu_flipTrianglesToImproveQuadraticAspectRatio (mpGraph);
    }


// Find pseudo spine edges
// Optionally include pure internal midline segments.
// Optionally include midline segments into "dead end"
// Optionally include adjacent spokes to corner.
void  GetSpineEdges (bvector<bvector<DPoint3d>> & xyzOut, bool bIncludeInterior = true, bool bIncludeFinal = true, bool bIncludeCornerSpokes = true)
    {
    xyzOut.clear ();
    vu_arrayClear (mpFaceArray);
    vu_collectInteriorFaceLoops (mpFaceArray, mpGraph);
    vu_arrayOpen (mpFaceArray);
    VuP pFace;
    for (vu_arrayOpen (mpFaceArray);
         0 != vu_arrayRead (mpFaceArray, &pFace);)
        {
        if (GetSpineEdgesInTriangle (pFace, xyzOut, bIncludeInterior, bIncludeFinal))
            {
            }
        else if (GetSpineEdgesInQuad (pFace, xyzOut, bIncludeInterior, bIncludeFinal, bIncludeCornerSpokes))
            {
            }
        }
    }

// Find local edges bisecting angles and opposite sides within blocks
void  GetLocalBisectionEdges (bvector<bvector<DPoint3d>> & xyzOut)
    {
    xyzOut.clear ();
    vu_arrayClear (mpFaceArray);
    vu_collectInteriorFaceLoops (mpFaceArray, mpGraph);
    vu_arrayOpen (mpFaceArray);
    VuP pFace;
    for (vu_arrayOpen (mpFaceArray);
         0 != vu_arrayRead (mpFaceArray, &pFace);)
        {
        GetLocalBisectorEdgesInQuad (pFace, xyzOut);
        }
    }

// Constructor.  Create vu graph and face array to be reused ...
VuSpineContext ()
    {
    mpGraph = vu_newVuSet (0);
    mpFaceArray = vu_grabArray (mpGraph);
    vu_setTol (mpGraph, sSpineGraphAbsTol, sSpineGraphRelTol);
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
bool GetFace (bvector<DPoint3d> & xyzOut)
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
