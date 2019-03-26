/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/test/src/CurveVectorXYOffsetContext.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

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
<li> Call    interiorRegion = CurveVectorXYOffsetContext::ComputeCenterForRectangularBox(..) to obtain an (interior) region
<li> Call    CurveVectorXYOffsetContext::ComputeBoxCenter (regionBoundary, interiorRegion, effort, xyzOut) to obtain a single placement point.
    <ul>
    <li> effort = 10 ==> try vu_anyInteriorPointInPolygon
    <li> effort = 1 ==> try the centroid of the interior region
    <li> effort = 0 ==> choose any point (e.g. start point of the interior region)
</ul>
*/
struct CurveVectorXYOffsetContext
{
void static AddRectangle (CurveVectorR unionRegion, DPoint3dCR xyz, DRange2dCR range, DPoint3dR previousPoint)
    {
    if (xyz.AlmostEqual (previousPoint))
        return;
    DPoint3d points[5];

    points[0] = DPoint3d::From (xyz.x + range.low.x, xyz.y + range.low.y, xyz.z);
    points[1] = DPoint3d::From (xyz.x + range.high.x, xyz.y + range.low.y, xyz.z);
    points[2] = DPoint3d::From (xyz.x + range.high.x, xyz.y + range.high.y, xyz.z);
    points[3] = DPoint3d::From (xyz.x + range.low.x, xyz.y + range.high.y, xyz.z);
    points[4] = points[0];
    unionRegion.Add (CurveVector::CreateLinear (points, 5, CurveVector::BOUNDARY_TYPE_Outer));
    previousPoint = xyz;
    }

void static AddRectanglesAtKeyPoints(CurveVectorCR curves, DRange2dCR range, CurveVectorR unionRegion)
    {
    DSegment3d segment;
    bvector<DPoint3d> const *lines;
    DPoint3d point0 = DPoint3d::From (DISCONNECT, DISCONNECT, DISCONNECT);
    for (auto &child : curves)
        {
        if (child->TryGetLine(segment))
            {
            AddRectangle (unionRegion, segment.point[0], range, point0);
            AddRectangle(unionRegion, segment.point[1], range, point0);
            }
        else if (nullptr != (lines = child->GetLineStringCP()))
            {
            for (auto &xyz : *lines)
                {
                AddRectangle (unionRegion, xyz, range, point0);
                }
            }
        }
    }


/*
* Return a curve vector which is the inner area in which the center of a rectangular box (e.g. text) can be placed
* without the box clashing with the regionBoundary.
*/
CurveVectorPtr static ComputeCenterRegionForRectangularBox (
CurveVectorCR regionBoundary,
double xSize,   //!< x dimension of box to be placed
double ySize,    //!< y dimension of box to be placed
double angleRadians = 0.0   //!< angle to rotate the text box.
)
    {
    // clone as needed to elminate angle and curve effects
    if (angleRadians != 0.0)
        {
        // copy with the rectangle x axis rotated down to global x
        DPoint3d xyzA, xyzB;
        regionBoundary.GetStartEnd (xyzA, xyzB);
        auto transform = Transform::FromMatrixAndFixedPoint(
            RotMatrix::FromAxisAndRotationAngle(2, -angleRadians),
            xyzA);
        auto rotatedBoundary = CloneWithTransform(regionBoundary, transform);
        auto region = ComputeCenterRegionForRectangularBox (*rotatedBoundary, xSize, ySize, 0.0);
        auto inverseTransform = transform.ValidatedInverse ();
        return CloneWithTransform(*region, inverseTransform);
        }
    if (regionBoundary.ContainsNonLinearPrimitive())
        {
        auto options = IFacetOptions::CreateForCurves ();
        auto strokes = regionBoundary.Stroke (*options);
        return ComputeCenterRegionForRectangularBox (*strokes, xSize, ySize, 0.0);
        }
    // REMARK: Hypothetically ax,bx,ay,by allow asymmetric offseting.  But I dont' understand the interaction in the
    // transformBX etc combinations and the vertex box setups.  So this is worked throught with equal ax=bx, ay=by
    double ax = 0.5 * xSize;
    double bx = 0.5 * xSize;
    double ay = 0.5 * ySize;
    double by = 0.5 * ySize;
    // This range is the coordinates (around 0) of box to cut out around vertices.
    DRange2d chop;
    static double s_expansionFactor = 1.001;
    chop.low = DPoint2d::From(-s_expansionFactor * bx, -s_expansionFactor * by);
    chop.high = DPoint2d::From(s_expansionFactor * ax, s_expansionFactor * ay);

    auto transformBX = Transform::From (-bx, -by, 0);
    auto transformBY = Transform::From (-bx,  by, 0);
    auto transformAX = Transform::From (ax, -ay, 0);
    auto transformAY = Transform::From(ax, ay, 0);
    // REMARK: The intersections might be done as one step with a union region.
    // I prefer to deal with smaller region problems.
    auto shapeBX = CloneWithTransform(regionBoundary, transformBX);
    auto shapeBY = CloneWithTransform(regionBoundary, transformBY);
    auto shapeAX = CloneWithTransform(regionBoundary, transformAX);
    auto shapeAY = CloneWithTransform(regionBoundary, transformAY);


    CurveVectorPtr minusBX = CurveVector::AreaIntersection (*shapeBX, regionBoundary);
    CurveVectorPtr minusBXBY = CurveVector::AreaIntersection (*minusBX, *shapeBY);

    CurveVectorPtr minusBXBYAX = CurveVector::AreaIntersection(*minusBXBY, *shapeAX);
    CurveVectorPtr minusBXBYAXAY = CurveVector::AreaIntersection(*minusBXBYAX, *shapeAY);

    auto chopper0 = CurveVector::Create (CurveVector::BOUNDARY_TYPE_UnionRegion);
    AddRectanglesAtKeyPoints(regionBoundary, chop, *chopper0);
    auto chopper = CurveVector::ReduceToCCWAreas (*chopper0);
    CurveVectorPtr minusChop = CurveVector::AreaDifference (*minusBXBYAXAY, *chopper);
    return minusChop;
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
        bvector<bvector<DPoint3d>> loops;
        if (offsetRegion.CollectLinearGeometry(loops))
            {
            DPoint3d disconnect;
            disconnect.InitDisconnect ();
            // ugh ... vu_pointIn wants packed with disconnects . ..
            bvector<DPoint3d> packedXYZ;
            for (auto &loop : loops)
                {
                for (auto &xyz : loop)
                    packedXYZ.push_back (xyz);
                packedXYZ.push_back(disconnect);
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
