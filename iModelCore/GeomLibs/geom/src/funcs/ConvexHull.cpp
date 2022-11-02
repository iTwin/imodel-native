/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*-----------------------------------------------------------------*//**
* std::sort comparator.   Return True for pA < pB with (w, r^2) lexical order.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
static int compareWR
(
    DPoint4dCR pA,
    DPoint4dCR pB
)
    {
    double rrA, rrB;
    if (pA.w < pB.w)
        return true;
    if (pA.w > pB.w)
        return false;
    rrA = pA.x * pA.x + pA.y * pA.y;
    rrB = pB.x * pB.x + pB.y * pB.y;

    if (rrA < rrB)
        return true;
    return false;
    }

/*-----------------------------------------------------------------*//**
* @description Trim points from the tail of an evolving hull if they
*   are covered by segment from new point back to earlier point.
* Note that the new point is NOT added.
* @param hullPoints IN evolving hull array.
* @param xyzNew = new point being added to hull.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
static void trimHull

(
    bvector<DPoint4d> &hullPoints,
    DPoint4dCR xyzNew,
    double ex,
    double ey
)
    {
    size_t i0, i1;
    double dx0, dy0, dx1, dy1;
    double cross;
    while (hullPoints.size() >= 2)
        {
        i0 = hullPoints.size() - 1;
        i1 = i0 - 1;
        dx0 = hullPoints[i0].x - xyzNew.x;
        dy0 = hullPoints[i0].y - xyzNew.y;
        if (fabs(dx0) < ex && fabs(dy0) < ey)
            {
            hullPoints.pop_back();
            }
        else
            {
            dx1 = hullPoints[i1].x - xyzNew.x;
            dy1 = hullPoints[i1].y - xyzNew.y;
            double dx2 = hullPoints[i1].x - hullPoints[i0].x;
            double dy2 = hullPoints[i1].y - hullPoints[i0].y;
            cross = dx0 * dy1 - dy0 * dx1;
            double cross2 = dx0 * dy2 - dy0 * dx2;     // cross and cross2 are mathematically equal.  But they may differ due to roundoff.
            if (cross < 0.0 && cross2 < 0.0)    // If signs are not both positive, it must be near zero (which we want to reject)
                break;
            hullPoints.pop_back();
            }
        }
    }

static bool samePointXYTolerances(DPoint4dCR pointA, DPoint4dCR pointB, double ex, double ey)
    {
    return fabs(pointA.x - pointB.x) <= ex && fabs(pointA.y - pointB.y) <= ey;
    }
/*-----------------------------------------------------------------*//**
* @description Compute a convex hull of given points.  Each output point
*       is one of the inputs, including its z part.
* @param hullPoints OUT Convex hull points.  First/last point NOT duplicated.
* @param allPoints all points, as (x,y,extraData,radians)
* @param pInBuffer IN input points.
* @param numIn IN number of input points.
* @param iMax IN index of point at maximal radius, i.e. guaranteed to be on hull.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
static bool    bsiDPoint3dArray_convexHullXY_go

(
    bvector<DPoint4d> &hullPoints,
    bvector<DPoint4d> &allPoints
)
    {
    size_t numIn = allPoints.size();
    static double toleranceFactor = 8.0;
    std::sort(allPoints.begin(), allPoints.end(), compareWR);
    // find the max-radius point in sort order ...
    double xMax = 0.0;
    double yMax = 0.0;
    size_t iMax = 0;
    double r2Max = 0.0;
    for (size_t i = 0; i < numIn; i++)
        {
        double x = fabs(allPoints[i].x);
        if (x > xMax)
            xMax = x;
        double y = fabs(allPoints[i].y);
        if (y > yMax)
            yMax = y;
        double r2 = x * x + y * y;
        if (r2 > r2Max)
            {
            r2Max = r2;
            iMax = i;
            }
        }
    double ex = toleranceFactor * (nextafter(xMax, 2 * xMax) - xMax);
    double ey = toleranceFactor * (nextafter(yMax, 2 * yMax) - yMax);
    hullPoints.clear();
    // The extreme point is on the hull for sure ...
    hullPoints.push_back(allPoints[iMax]);
    // process the rest cyclically
    for (size_t i = 1; i < numIn; i++)
        {
        size_t k = (iMax + i) % numIn;
        DPoint4d candidate = allPoints[k];
        if (samePointXYTolerances(candidate, hullPoints.back(), ex, ey))
            continue;
        trimHull(hullPoints, candidate, ex, ey);
        hullPoints.push_back(candidate);
        if (hullPoints.size() >= 3)
            trimHull(hullPoints, hullPoints[0], ex, ey);
        }
    return true;
    }

/*-----------------------------------------------------------------*//**
* Compute the xy convex hull of points.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void DPoint3dOps::ConvexHullXY
(
bvector<DPoint3d> const &points,
bvector<DPoint3d> &hullPoints,
bool includeClosurePoint
)
    {
    hullPoints.clear();

    size_t numIn = points.size ();
    if (numIn <= 0)
        return;

    bvector<DPoint4d>sortXYZA;
    bvector<DPoint4d>hullXYZA;

    if (numIn == 1)
        {
        hullPoints.push_back (points.front ());
        }
    else
        {
        /* Compute centroid of all points, relative to first point. */
        double xsum = 0.0;
        double ysum = 0.0;
        double x0 = points.front().x;
        double y0 = points.front().y;
        for (size_t i = 1; i < numIn; i++)
            {
            xsum += points[i].x - x0;
            ysum += points[i].y - y0;
            }
        x0 += xsum / numIn;
        y0 += ysum / numIn;
        /* Set up work array with x,y,i,angle in local coordinates around centroid. */
        for (size_t i = 0; i < numIn; i++)
            {
            DPoint4d xyzA;
            double dx = xyzA.x = points[i].x - x0;
            double dy = xyzA.y = points[i].y - y0;
            xyzA.z = (double)i;
            xyzA.w = bsiTrig_atan2(dy, dx);
            double rr = dx * dx + dy * dy;
            if (rr > 0.0)
                sortXYZA.push_back (xyzA);
            }

        if (sortXYZA.size () <= 1)
            {
            /* All points are at the centroid. Copy the first one out. */
            hullPoints.push_back (points.front ());
            }
        else
            {
            if (bsiDPoint3dArray_convexHullXY_go(hullXYZA, sortXYZA))
                {
                for (auto &xyzA : hullXYZA)
                    {
                    uint32_t k = (uint32_t)xyzA.z;
                    hullPoints.push_back(points[k]);
                    }
                }
            if (includeClosurePoint && hullPoints.size() > 1)
                {
                DPoint3d xyz = hullPoints.back ();
                hullPoints.push_back (xyz);
                }
            }
        }
    }
/*-----------------------------------------------------------------*//**
* @description Compute a convex hull of a point array, ignoring z-coordinates.
* @remarks Each output point is one of the inputs, including its z-coordinate.
* @param pOutBuffer OUT convex hull points, first/last point <em>not</em> duplicated.
*                       This must be allocated by the caller, large enough to contain numIn points.
* @param pNumOut    OUT number of points on hull
* @param pInBuffer  IN  input points
* @param numIn      IN  number of input points
* @return false if numin is nonpositive or memory allocation failure
* @group "DPoint3d Queries"
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3dArray_convexHullXY
(
    DPoint3dP pOutBuffer,
    int         *pNumOut,
    DPoint3dP pInBuffer,
    int         numIn
)
    {
    bvector<DPoint3d> xyzA, xyzB;
    for (int i = 0; i < numIn; i++)
        xyzA.push_back (pInBuffer[i]);
    DPoint3dOps::ConvexHullXY (xyzA, xyzB, false);
    *pNumOut = (int)xyzB.size ();
    for (size_t i = 0; i < xyzB.size (); i++)
        pOutBuffer[i] = xyzB[i];
    return *pNumOut > 0;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
