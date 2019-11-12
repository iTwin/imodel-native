/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

// This h file has all methods directly implemented.
// Hence include it a single C++ file.

// connect tree does not have CurveVector::Clone(transform) ... substitute this ..
static CurveVectorPtr CloneWithTransform(CurveVectorCR curves, TransformCR transform)
    {
    auto clone = curves.Clone();
    clone->TransformInPlace(transform);
    return clone;
    }
/*
<ul>
<li> Let regionBoundary be a (CurveVector) for a region.
<li> We need to place a text box of known size and rotation so it (the text box) is completely inside the region.
<li> The outer region may contain holes (e.g. can be a parity region)
</ul>

<ul>
<li> Call    interiorRegion = CurveVectorXYOffsetContext::ComputeCenterRegionForRectangularBox(..) to obtain an (interior) region
<li> Call    CurveVectorXYOffsetContext::ComputeBoxCenter (regionBoundary, interiorRegion, effort, xyzOut) to obtain a single placement point.
    <ul>
    <li> effort = 10 ==> try vu_anyInteriorPointInPolygon
    <li> effort = 1 ==> try the centroid of the interior region
    <li> effort = 0 ==> choose any point (e.g. start point of the interior region)
</ul>
*/
struct CurveVectorXYOffsetContext
{

//
//                                                3--------2
//                                                |  B     |
//                                                |        |
//                                                0--------1
//
//
//     3--------2
//     |  A     |
//     |        |
//     0--------1
//
//    The (6 sided) convex hll around two such rectangles consists of 
//    * 3 cyclically consectutive indices from the rectangle 0 (here on left) and 3 consecutives from the upper.
//    * as placed here, those are (A3,A0,A1) and (B1,B2,B3)
//    * In general position, as rectangle B moves to other quadrants with respect to rectangle A, the indices advance;
//    * When rectangle B is truly in a quadrant, there are always 3 consecutive vertices on A then three on B, with shared first and last indices.
//    * if one of dx or dy is zero only 2 from each collectively covering all 4 indcies
//        * e.g. (A0,A1,B2,B3) for B "above" A
//        * e.g. (A3,A0,B1,B2) for B "to the right of A"
//    * if dx and dy are both zero, again (A0,A1,B2,B3).   (And using 2 from A and 2 from B rather than 4 from A makes nice table logic in caller)
//    * in the six point case, the tail index of the lower rectangle always is the start for the upper.
//    * caller does comparisons to get the relationship and triggers the calls
static void AppendCyclicPoints(DPoint3d xyz[], uint32_t &n, double xc, double yc, double ax, double ay, uint32_t i0, uint32_t numAdd)
    {
    // coordinates around central origin within each triangle
    static double uv[4][2] = { {-1,-1}, {1,-1}, {1,1}, {-1,1} };
    // indices in CCW order, repeated to allow wrap without if or modulo.
    static uint32_t id[] = {0, 1, 2, 3, 0, 1, 2, 3};
    for (uint32_t k = 0; k < numAdd; k++)
        {
        uint32_t i = id[i0 + k];
        xyz[n].x = xc + uv[i][0] * ax;
        xyz[n].y = yc + uv[i][1] * ay;
        xyz[n].z = 0.0;
        n++;
        }
    }
// Map negative, 0, and positive to 0,1,2
static uint32_t DoubleTo012(double x)
    {
    if (x > 0.0)    return 2;
    if (x < 0.0)    return 0;
    return 1;
    }
// Return (as a CurveVector::BOUNDARY_TYPE_Outer) the convex hull around a pair of rectangles of same half sizes and given centers.
// * Signed combinations of ax,ay are x and y distances from rectangle centers to corners.
static CurveVectorPtr CreateConvexHullAround2Rectangles (DPoint3dCR center0, DPoint3dCR center1, double ax, double ay)
    {
    DPoint3d points[10];
    double x0 = center0.x;
    double y0 = center0.y;
    double x1 = center1.x;
    double y1 = center1.y;
    uint32_t gridCase = 3 * DoubleTo012 (y1 - y0) + DoubleTo012 (x1 - x0);
    static uint32_t pointsPerRectangle [] = {3,2,3,2,2,2,3,2,3};  // number of points taken from each rectangle
    static uint32_t index0 [] = {
        1, 2, 2,        // bottom row (y1 < 0)
        1, 1, 3,        // middle row (y1 == 0)
        0, 0, 3};       // top row (y1 > 0)
    uint32_t n = 0;
    // take 2 or 3 sides from box at center0
    AppendCyclicPoints (points, n, x0, y0, ax, ay, index0[gridCase], pointsPerRectangle[gridCase]);
    // and same number from box at center1, starting 2 later in order around the box.
    AppendCyclicPoints(points, n, x1, y1, ax, ay, index0[gridCase] + 2, pointsPerRectangle[gridCase]);

    return CurveVector::CreateLinear(points, n, CurveVector::BOUNDARY_TYPE_Outer);
    }
// Create a union region containing convex hull around each segment of the (possibly deep) CurveVector.
void static AddConvexHullsAroundSegments(CurveVectorCR curves, double ax, double ay, CurveVectorR unionRegion)
    {
    DSegment3d segment;
    bvector<DPoint3d> const *lines;
    CurveVectorCP child1;
    for (auto &child : curves)
        {
        if (child->TryGetLine(segment))
            {
            unionRegion.Add(CreateConvexHullAround2Rectangles(segment.point[0], segment.point[1], ax, ay));
            }
        else if (nullptr != (lines = child->GetLineStringCP()))
            {
            for (size_t i = 1; i < lines->size (); i++)
                {
                unionRegion.Add (CreateConvexHullAround2Rectangles (lines->at(i-1), lines->at(i), ax, ay));
                }
            }
        else if (nullptr != (child1 = child->GetChildCurveVectorCP()))
            {
            AddConvexHullsAroundSegments(*child1, ax, ay, unionRegion);
            }
        }
    }

/*
* Return a curve vector which is the inner area in which the center of a rectangular box (e.g. text) can be placed
* without the box clashing with the regionBoundary.
*/
CurveVectorPtr static ComputeCenterRegionForRectangularBox(
    CurveVectorCR regionBoundary,
    double xSize,   //!< x dimension of box to be placed
    double ySize,    //!< y dimension of box to be placed
    double angleRadians = 0.0,   //!< angle to rotate the text box.
    bvector<CurveVectorPtr> * debugGeometry = nullptr// optional debug output
)
    {
    // clone as needed to elminate angle and curve effects
    if (angleRadians != 0.0)
        {
        // copy with the rectangle x axis rotated down to global x
        DPoint3d xyzA, xyzB;
        regionBoundary.GetStartEnd(xyzA, xyzB);
        auto transform = Transform::FromMatrixAndFixedPoint(
            RotMatrix::FromAxisAndRotationAngle(2, -angleRadians),
            xyzA);
        auto rotatedBoundary = CloneWithTransform(regionBoundary, transform);
        auto region = ComputeCenterRegionForRectangularBox(*rotatedBoundary, xSize, ySize, 0.0, debugGeometry);
        auto inverseTransform = transform.ValidatedInverse();
        return region.IsValid() ? CloneWithTransform(*region, inverseTransform) : nullptr;
        }
    if (regionBoundary.ContainsNonLinearPrimitive())
        {
        auto options = IFacetOptions::CreateForCurves();
        auto strokes = regionBoundary.Stroke(*options);
        return ComputeCenterRegionForRectangularBox(*strokes, xSize, ySize, 0.0);
        }

    double ax = 0.5 * xSize;
    double ay = 0.5 * ySize;

    bvector<CurveVectorPtr> shifts;
    CurveVectorPtr chopperA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_UnionRegion);
    AddConvexHullsAroundSegments(regionBoundary, ax, ay, *chopperA);
    if (debugGeometry)
        debugGeometry->push_back (chopperA->Clone ());
    auto chopperB = CurveVector::ReduceToCCWAreas(*chopperA);
    if (debugGeometry)
        debugGeometry->push_back(chopperB->Clone());
    auto residual = CurveVector::AreaDifference(regionBoundary, *chopperB);
    return residual;
    }

/*
 * Choose a point for displaying a box.
 * effort indicates how much work to attempt:
 * <ul>
 * <li>0 means as little work as possible -- use any point from the offset region.
 * <li>1 means try the centroid of the offset region.
 * <li>10 means find a deeply buried point
 * </ul>
 * PREFERRED point is the centroid of the offset region.
 * LAST RESORT is start point of first curve of offset region.
*/
static bool ChooseBoxCenter(CurveVectorCR originalRegion, CurveVectorCR offsetRegion, int effort, DPoint3dR xyzOut)
    {
    if (effort >= 10)
        {
        bvector<bvector<bvector<DPoint3d>>> regions; // uh oh -- if there really are multiple regions what happens?
        // If they are non-overlapping, parity works as expected . . .. .
        if (offsetRegion.CollectLinearGeometry(regions))
            {
            DPoint3d disconnect;
            disconnect.InitDisconnect ();
            // ugh ... vu_pointIn wants packed with disconnects . ..
            bvector<DPoint3d> packedXYZ;
            for (auto &region : regions)
                {
                for (auto &loop : region)
                    {
                    for (auto &xyz : loop)
                        packedXYZ.push_back (xyz);
                    packedXYZ.push_back(disconnect);
                    }
                }
            
            if (packedXYZ.size () > 2)
                {
                if (SUCCESS == vu_anyInteriorPointInPolygon(&xyzOut, &packedXYZ[0], (int)packedXYZ.size ()))
                    return true;
                }
            }
        }

    if(effort > 0)
        { 
        DPoint3d centroid;
        double area;
        if (offsetRegion.CentroidAreaXY (centroid, area))
            {
            auto c = offsetRegion.PointInOnOutXY(centroid);
            if (c == CurveVector::INOUT_In || c == CurveVector::INOUT_On)
                {
                xyzOut = centroid;
                return true;
                }
            }
        }

    // Default: any point on the offset region
    DPoint3d xyzA, xyzB;
    if (offsetRegion.GetStartEnd (xyzA, xyzB))
        {
        xyzOut = xyzA;
        return true;
        }

    if (offsetRegion.GetStartEnd(xyzA, xyzB))
        {
        xyzOut = xyzA;
        return false;
        }

    xyzOut.Zero ();
    return false;
    }

};
