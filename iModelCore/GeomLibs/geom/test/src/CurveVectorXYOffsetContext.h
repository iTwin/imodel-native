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
    CurveVectorCP child1;
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
        else if (nullptr != (child1 = child->GetChildCurveVectorCP ()))
            {
            AddRectanglesAtKeyPoints(*child1, range, unionRegion);
            }
        }
    }

void static AddSegmentChop(CurveVectorR unionRegion, DSegment3dCR segment, double ax, double ay)
    {
    DPoint3d points[10];
    double dx = segment.point[1].x - segment.point[0].x;
    double dy = segment.point[1].y - segment.point[0].y;
    size_t n = 0;
    size_t i0 = 0;
    if (fabs(dx) < fabs(dy))
        {
        if (dy < 0)
            {
            i0 = 1;
            dx = -dx;
            }
        size_t i1 = 1 - i0;
        double x0 = segment.point[i0].x;
        double x1 = segment.point[i1].x;
        double y0 = segment.point[i0].y;
        double y1 = segment.point[i1].y;
        points[n++] = DPoint3d::From (x0 - ax, y0 - ay);
        points[n++] = DPoint3d::From(x0 + ax, y0 - ay);
        if (dx < 0)
            points[n++] = DPoint3d::From (x0 + ax, y0 + ay);
        else if (dx > 0)
            points[n++] = DPoint3d::From(x1 + ax, y1 - ay);
        points[n++] = DPoint3d::From (x1 + ax, y1 + ay);
        points[n++] = DPoint3d::From(x1 - ax, y1 + ay);
        if (dx < 0)
            points[n++] = DPoint3d::From(x1 - ax, y1 - ay);
        else if (dx > 0)
            points[n++] = DPoint3d::From(x0 - ax, y0 + ay);
        points[n++] = points[0];
        }
    else
        {
        if (dx < 0)
            {
            i0 = 1;
            dy = -dy;
            }
        size_t i1 = 1 - i0;
        double x0 = segment.point[i0].x;
        double x1 = segment.point[i1].x;
        double y0 = segment.point[i0].y;
        double y1 = segment.point[i1].y;
        points[n++] = DPoint3d::From(x0 - ax, y0 + ay);
        points[n++] = DPoint3d::From(x0 - ax, y0 - ay);
        if (dy < 0)
            points[n++] = DPoint3d::From(x1 - ax, y1 - ay);
        else if (dy > 0)
            points[n++] = DPoint3d::From(x0 + ax, y0 - ay);
        points[n++] = DPoint3d::From(x1 + ax, y1 - ay);
        points[n++] = DPoint3d::From(x1 + ax, y1 + ay);
        if (dy < 0)
            points[n++] = DPoint3d::From(x0 + ax, y0 + ay);
        else if (dy > 0)
            points[n++] = DPoint3d::From(x1 - ax, y1 + ay);
        points[n++] = points[0];
        }

    unionRegion.Add(CurveVector::CreateLinear(points, n, CurveVector::BOUNDARY_TYPE_Outer));
    }
void static AddSegmentChops (CurveVectorCR curves, double ax, double ay, CurveVectorR unionRegion)
    {
    DSegment3d segment;
    bvector<DPoint3d> const *lines;
    CurveVectorCP child1;
    for (auto &child : curves)
        {
        if (child->TryGetLine(segment))
            {
            AddSegmentChop(unionRegion, segment, ax, ay);
            }
        else if (nullptr != (lines = child->GetLineStringCP()))
            {
            for (size_t i = 1; i < lines->size (); i++)
                {
                AddSegmentChop (unionRegion, DSegment3d::From (lines->at (i-1), lines->at(i)), ax, ay);
                }
            }
        else if (nullptr != (child1 = child->GetChildCurveVectorCP()))
            {
            AddSegmentChops(*child1, ax, ay, unionRegion);
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
double angleRadians = 0.0,   //!< angle to rotate the text box.
bvector<CurveVectorPtr> * debugGeometry = nullptr// optional debug output
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
        auto region = ComputeCenterRegionForRectangularBox (*rotatedBoundary, xSize, ySize, 0.0, debugGeometry);
        auto inverseTransform = transform.ValidatedInverse ();
        return region.IsValid () ? CloneWithTransform(*region, inverseTransform) : nullptr;
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
    bvector<CurveVectorPtr> shifts;
    shifts.push_back (CloneWithTransform(regionBoundary, transformBX));
    shifts.push_back (CloneWithTransform(regionBoundary, transformBY));
    shifts.push_back (CloneWithTransform(regionBoundary, transformAX));
    shifts.push_back (CloneWithTransform(regionBoundary, transformAY));

    auto residual = regionBoundary.Clone ();
    for (auto &shift : shifts) {
        if (!residual.IsValid ())
            break;
        if (shift.IsValid ())
            {
            if (debugGeometry)
                debugGeometry->push_back(shift->Clone ());
            residual = CurveVector::AreaIntersection(*shift, *residual);
            if (debugGeometry)
                debugGeometry->push_back(residual->Clone());

            }
        }
    if (!residual.IsValid ())
        return nullptr;
    static int s_doChop = 0;
    auto chopper0 = CurveVector::Create (CurveVector::BOUNDARY_TYPE_UnionRegion);
    AddRectanglesAtKeyPoints(regionBoundary, chop, *chopper0);
    if (debugGeometry)
        debugGeometry->push_back(chopper0->Clone());
    auto chopper = CurveVector::ReduceToCCWAreas (*chopper0);
    if (debugGeometry)
        debugGeometry->push_back(chopper->Clone());
    if (s_doChop)
        {
        residual = CurveVector::AreaDifference (*residual, *chopper);
        if (debugGeometry)
            debugGeometry->push_back(residual->Clone());
        }
    return residual;
    }

/*
* Return a curve vector which is the inner area in which the center of a rectangular box (e.g. text) can be placed
* without the box clashing with the regionBoundary.
*/
CurveVectorPtr static ComputeChoppedCenterRegionForRectangularBox(
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
        auto region = ComputeChoppedCenterRegionForRectangularBox(*rotatedBoundary, xSize, ySize, 0.0, debugGeometry);
        auto inverseTransform = transform.ValidatedInverse();
        return region.IsValid() ? CloneWithTransform(*region, inverseTransform) : nullptr;
        }
    if (regionBoundary.ContainsNonLinearPrimitive())
        {
        auto options = IFacetOptions::CreateForCurves();
        auto strokes = regionBoundary.Stroke(*options);
        return ComputeChoppedCenterRegionForRectangularBox(*strokes, xSize, ySize, 0.0);
        }

    double ax = 0.5 * xSize;
    double ay = 0.5 * ySize;

    bvector<CurveVectorPtr> shifts;
    CurveVectorPtr chopperA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_UnionRegion);
    AddSegmentChops(regionBoundary, ax, ay, *chopperA);
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
