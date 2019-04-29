/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "testHarness.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>
void Dump (bvector<DPoint2d> const &data)
    {
    bvector<DPoint3d> xyz;
    for (DPoint2d xy : data)
        {
        xyz.push_back (DPoint3d::From (xy.x, xy.y));
        }
    auto cp = ICurvePrimitive::CreateLineString (xyz);
    Check::SaveTransformed (*cp);
    }

void Dump (DPoint2d const &xy0, DPoint2d const &xy1)
    {
    auto cp = ICurvePrimitive::CreateLine (
        DSegment3d::From (xy0.x, xy0.y, 0.0, xy1.x, xy1.y, 0.0));
    Check::SaveTransformed (*cp);
    }


// EDL June 24 Copy parts of Geometry.h file here for conversion to DPoint2d / DVec2d.
// gema edits :
// \IMPoint2d\I = DPoint2d
// \IMVector2d\I = DVec2d
// \IMRange1d\I = DRange1d
// \IMRay2d\I = DRay2d
// std\:\ : vector = bvector
// \ICrossProductToTargets\I = CrossProductToPoints
// \INormalizeInPlace\I = Normalize
// 
// \ICWPerpendicularVector\I = Rotate90CW
// \ICCWPerpendicularVector\I = Rotate90CCW
// \IMDistance\:\ : SafeDivideDistance = DoubleOps::ValidatedDivideDistance
// 
// issues
// 1) MVec2d (startPoint, endPoint) == > DVec2d::FromStartEnd (startPoint, endPoint)
// ** not safe in gema -- can't distinguish from MVec2d (x,y)
// 2) DVec2d.Add is "in place" --unlike functional style of MVec2d

//! Ray defined by origin and direction vector.
struct DRay2d
    {
    friend struct DPoint2d;
    friend struct DConvexPolygon2d;
    private:
        DPoint2d m_origin;
        DVec2d m_direction;
    public:
        DRay2d ()
            {
            }
        DRay2d (DPoint2d const &origin, DPoint2d const &target)
            {
            m_origin = origin;
            m_direction = DVec2d::FromStartEnd (origin, target);
            }

        DRay2d (DPoint2d const &origin, DVec2d const &direction)
            {
            m_origin = origin;
            m_direction = direction;
            }
        //! Return a ray that is parallel at distance to the left, specified as fraction of the ray's direction vector.
        DRay2d ParallelRay (double leftFraction) const
            {
            return DRay2d
                (
                    m_origin.AddForwardLeft (m_direction, 0.0, leftFraction),
                    m_direction
                    );
            }

        DRay2d CCWPerpendicularRay () const
            {
            return DRay2d
                (
                    m_origin,
                    m_direction.Rotate90CCW ()
                    );
            }

        DRay2d CWPerpendicularRay () const
            {
            return DRay2d
                (
                    m_origin,
                    m_direction.Rotate90CW ()
                    );
            }

        bool NormalizeDirectionInPlace ()
            {
            double magnitude;
            return m_direction.TryNormalize (m_direction, magnitude);
            }

        DPoint2d FractionToPoint (double f) const
            {
            return DPoint2d::FromSumOf (m_origin, m_direction, f);
            }

        // Intersect this ray (ASSUMED NORMALIZED) with unbounded line defined by points.
        // (The normalization assumption affects test for parallel vectors.)
        bool IntersectUnboundedLine (DPoint2d const linePointA, DPoint2d const &linePointB, double &fraction, double &dhds) const
            {
            DVec2d lineDirection = DVec2d::FromStartEnd (linePointA, linePointB);
            DVec2d vector0 = DVec2d::FromStartEnd (linePointA, m_origin);
            double h0 = vector0.CrossProduct (lineDirection);
            dhds = m_direction.CrossProduct (lineDirection);
            // h = h0 + s * dh
            auto ff = DoubleOps::ValidatedDivideDistance (-h0, dhds);
            if (ff.IsValid ())
                {
                fraction = ff;
                return true;
                }
            else
                {
                fraction = 0.0;
                return false;
                }
            }
        //! return the ray fraction where point projects to the ray.
        double ProjectionFraction (DPoint2d const &point) const
            {
            return m_direction.ProjectionFraction (DVec2d::FromStartEnd (m_origin, point));
            }

        //! return the fraction of projection to the perpendicular ray
        double PerpendicularProjectionFraction (DPoint2d const &point) const
            {
            return m_direction.PerpendicularProjectionFraction (DVec2d::FromStartEnd (m_origin, point));
            }
    };


//! methods treating an array of DPoint2d as counterclockwise order around a convex polygon
struct DConvexPolygon2d
    {
    private:
        // hull points in CCW order, WITHOUT final duplicate . ..
        bvector<DPoint2d> m_hullPoints;
    public:
        // Create the hull.
        DConvexPolygon2d (bvector<DPoint2d> &points)
            {
            ComputeConvexHull (points, m_hullPoints);
            }

        // Create the hull.
        // First try to use the points as given.  Return isValid = true if that succeeded.
        DConvexPolygon2d (bvector<DPoint2d> &points, bool &isValid)
            {
            m_hullPoints = points;
            isValid = IsValidConvexHull ();
            if (!isValid)
                ComputeConvexHull (points, m_hullPoints);
            }


        // Return a copy of the hull points.
        bvector<DPoint2d> Points () { return m_hullPoints; }

        // test if the hull points are a convex, CCW polygon
        bool IsValidConvexHull () const
            {
            if (m_hullPoints.size () < 3)
                return false;
            size_t n = m_hullPoints.size ();
            for (size_t i = 0; i < m_hullPoints.size (); i++)
                {
                size_t i1 = (i + 1) % n;
                size_t i2 = (i + 2) % n;
                if (m_hullPoints[i].CrossProductToPoints (m_hullPoints[i1], m_hullPoints[i2]) < 0.0)
                    return false;
                }
            return true;
            }

        //! @return true if the convex hull (to the left of edges) contains the test point.
        bool ContainsPoint
            (
                DPoint2d const &point           //!< [in] point to test.
                ) const
            {
            DPoint2d xy0 = m_hullPoints.back ();
            //double tol = -1.0e-20;  // negative tol!!
            for (size_t i = 0; i < m_hullPoints.size (); i++)
                {
                DPoint2d xy1 = m_hullPoints[i];
                double c = xy0.CrossProductToPoints (xy1, point);
                if (c < 0.0)
                    return false;
                xy0 = xy1;
                }
            return true;
            }

        // Return the largest outside.  (return 0 if in or on)
        double DistanceOutside (DPoint2d &xy)
            {
            double maxDistance = 0.0;
            DPoint2d xy0 = m_hullPoints.back ();
            //double tol = -1.0e-20;  // negative tol!!
            for (size_t i = 0; i < m_hullPoints.size (); i++)
                {
                DPoint2d xy1 = m_hullPoints[i];
                double c = xy0.CrossProductToPoints (xy1, xy);
                if (c < 0.0)
                    {
                    DRay2d ray (xy0, xy1);
                    double s = ray.ProjectionFraction (xy);
                    double d = 0.0;
                    if (s < 0.0)
                        d = xy0.Distance (xy);
                    else if (s > 1.0)
                        d = xy1.Distance (xy);
                    else
                        d = xy.Distance (ray.FractionToPoint (s));
                    if (d > maxDistance)
                        maxDistance = d;
                    }
                xy0 = xy1;
                }
            return maxDistance;
            }

        //! Offset the entire hull (in place) by distance.
        void OffsetInPlace (double distance)
            {
            size_t n = m_hullPoints.size ();
            if (n >= 3)
                {
                DVec2d edgeA, perpA, edgeB, perpB;
                edgeA = DVec2d::FromStartEnd (m_hullPoints.back (), m_hullPoints.front ());
                edgeA.Normalize ();
                perpA = edgeA.Rotate90CW ();
                DPoint2d hullPoint0 = m_hullPoints[0];
                for (size_t i = 0; i < m_hullPoints.size (); i++, perpA = perpB)
                    {
                    size_t j = i + 1;
                    edgeB = DVec2d::FromStartEnd (m_hullPoints[i],
                        j < n ? m_hullPoints[j] : hullPoint0);
                    edgeB.Normalize ();
                    perpB = edgeB.Rotate90CW ();
                    m_hullPoints[i] = m_hullPoints[i] + DVec2d::OffsetBisector (perpA, perpB, distance);
                    }
                }
            }

        // Return 2 distances bounding the intersection of the ray with a convex hull.
        // ASSUME (for tolerancing) the ray has normalized direction vector.
        DRange1d ClipRay
            (
                DRay2d const &ray             //!< [in] ray to clip.  Both negative and positive distances along the ray are possible.
                )
            {
            double distanceA = -DBL_MAX;
            double distanceB = DBL_MAX;

            if (m_hullPoints.size () < 3)
                return DRange1d ();
            DPoint2d xy0 = m_hullPoints.back ();
            for (auto xy1 : m_hullPoints)
                {
                double distance, dhds;
                if (ray.IntersectUnboundedLine (xy0, xy1, distance, dhds))
                    {
                    if (dhds > 0.0)
                        {
                        if (distance < distanceB)
                            distanceB = distance;
                        }
                    else
                        {
                        if (distance > distanceA)
                            distanceA = distance;
                        }
                    if (distanceA > distanceB)
                        return DRange1d ();
                    }
                else
                    {
                    // ray is parallel to the edge.
                    // Any single point out classifies it all . ..
                    if (xy0.CrossProductToPoints (xy1, ray.m_origin) < 0.0)
                        return DRange1d ();
                    }
                xy0 = xy1;
                }
            DRange1d range;
            range.Extend (distanceA);
            range.Extend (distanceB);
            return range;
            }

        //! Return the range of (fractional) ray postions for projections of all points from the arrays.
        DRange1d RangeAlongRay
            (
                DRay2d const &ray
                )
            {
            DRange1d range;
            for (auto xy1 : m_hullPoints)
                range.Extend (ray.ProjectionFraction (xy1));
            return range;
            }

        //! Return the range of (fractional) ray postions for projections of all points from the arrays.
        DRange1d RangePerpendicularToRay
            (
                DRay2d const &ray
                )
            {
            DRange1d range;
            for (auto xy1 : m_hullPoints)
                range.Extend (ray.PerpendicularProjectionFraction (xy1));
            return range;
            }


        static bool ComputeConvexHull (bvector<DPoint2d> const &xy, bvector<DPoint2d> &hull)
            {
            hull.clear ();
            size_t n = xy.size ();
            if (n < 3)
                return false;
            bvector<DPoint2d> xy1 = xy;
            std::sort (xy1.begin (), xy1.end (), DPoint2d::LexicalXYLessThan);
            hull.push_back (xy1[0]);    // This is sure to stay
            hull.push_back (xy1[1]);    // This one can be removed in the loop.
                                        // first sweep creates upper hull . .. 

            for (size_t i = 2; i < n; i++)
                {
                DPoint2d candidate = xy1[i];
                size_t top = hull.size () - 1;
                while (top > 0 && hull[top - 1].CrossProductToPoints (hull[top], candidate) <= 0.0)
                    {
                    top--;
                    hull.pop_back ();
                    }
                hull.push_back (candidate);
                }

            // second creates lower hull right to left
            size_t i0 = hull.size () - 1;
            // xy1.back () is already on stack.
            hull.push_back (xy1[n - 2]);
            for (size_t i = n - 2; i-- > 0;)
                {
                DPoint2d candidate = xy1[i];
                size_t top = hull.size () - 1;
                while (top > i0 && hull[top - 1].CrossProductToPoints (hull[top], candidate) <= 0.0)
                    {
                    top--;
                    hull.pop_back ();
                    }
                if (i > 0)  // don't replicate start point !!!
                    hull.push_back (candidate);
                }
            return true;
            }
    };

// For each hullPoints[i], form chord to hullPoints[i+step].
// Compute points fractionally on the chord.
// Evaluate 
static bool CheckHullChords (DConvexPolygon2d hull, size_t step)
    {
    auto hullPoints = hull.Points ();
    bvector<double> fractions{ -0.2, -0.01, 0.43, 0.96, 1.08 };
    size_t numError = 0;
    for (size_t i = 0; i < hullPoints.size (); i++)
        {
        size_t j = (i + step) % hullPoints.size ();
        for (double f : fractions)
            {
            auto xy = DPoint2d::FromInterpolate (hullPoints[i], f, hullPoints[j]);
            if (!Check::Bool
                (
                    DoubleOps::IsIn01 (f),
                    hull.ContainsPoint (xy)
                    ))
                numError++;
            }
        }
    return numError == 0;
    }

// Form rays from centroid to each point.
// Compute points fractionally on the chord.
// Evaluate in/out
static bool CheckHullRaysFromCentroid (DConvexPolygon2d hull)
    {
    auto hullPoints = hull.Points ();
    DPoint2d centroid;
    double area;
    size_t numError = 0;
    if (Check::True (PolygonOps::CentroidAndArea (hullPoints, centroid, area)))
        {
        bvector<double> fractions{ 0.0, 0.43, 0.96, 1.08, 2.5 };
        for (size_t i = 0; i < hullPoints.size (); i++)
            {
            for (double f : fractions)
                {
                auto xy = DPoint2d::FromInterpolate (centroid, f, hullPoints[i]);
                if (!Check::Bool
                    (
                        f <= 1.0,
                        hull.ContainsPoint (xy)
                        ))
                    numError++;
                }
            }
        }
    return numError == 0;
    }

size_t CountPointsInHull (DConvexPolygon2d const &hull, bvector<DPoint2d> const &points)
    {
    size_t n = 0;
    for (auto xy : points)
        if (hull.ContainsPoint (xy))
            n++;
    return n;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Geometry, ConvexHullQueries)
    {
#define testA
#ifdef testA
    bvector<DPoint2d> points
        {
        DPoint2d::From (0, 0),
        DPoint2d::From (10, 0),
        DPoint2d::From (20, 10),
        DPoint2d::From (10, 20),
        DPoint2d::From (0, 10)
        };
#else
    bvector<DPoint2d> points
        {
        DPoint2d::From (0, 0),
        DPoint2d::From (10, 0),
        DPoint2d::From (10, 20),
        DPoint2d::From (0, 20)
        };
#endif
    DConvexPolygon2d hull (points);
    Check::True (CheckHullRaysFromCentroid (hull));

    DRay2d rayA (DPoint2d::From (0, 5), DVec2d::From (2, 0));
    Check::True (rayA.NormalizeDirectionInPlace ());


    uint32_t skip = 3;
    for (size_t i = 0; i < points.size (); i++)
        {
        DPoint2d pointA = points[i];
        DPoint2d pointB = points[(i + skip) % points.size ()];
        DRay2d rayAB (pointA, pointB);
        DPoint2d pointA1, pointB1;
        DRange1d range = hull.ClipRay (rayAB);
        if (Check::False (range.IsNull (), "Clip interior segment"))
            {
            pointA1 = rayAB.FractionToPoint (range.Low ());
            pointB1 = rayAB.FractionToPoint (range.High ());
            double dAB = pointA.Distance (pointB);
            double d1 = pointA1.Distance (pointB1);
            Check::True (DoubleOps::AlmostEqual (dAB, d1), "Clipped chord length");
            Check::True (pointA1.AlmostEqual (pointA), "Clipped chord start");
            Check::True (pointB1.AlmostEqual (pointB), "Clipped chord end");

            DPoint2d pointC0 = DPoint2d::FromInterpolate (pointA, -0.5, pointB);
            DPoint2d pointC1 = DPoint2d::FromInterpolate (pointA, -0.1, pointB);
            DPoint2d pointD = DPoint2d::FromInterpolate (pointA, 1.5, pointB);
            DPoint2d pointM = DPoint2d::FromInterpolate (pointA, 0.323, pointB);
            // point order (C,A,M,B,D)
            Check::True (hull.ContainsPoint (pointM), "Known interior Point");
            Check::False (hull.ContainsPoint (pointC0), "Known exterior Point");
            Check::False (hull.ContainsPoint (pointC1), "Known exterior Point");
            Check::False (hull.ContainsPoint (pointD), "Known exterior Point");
            DRay2d rayC0C1 (pointC0, pointC1);
            Check::True (rayC0C1.NormalizeDirectionInPlace ());
            range = hull.ClipRay (rayC0C1);
            Check::False (range.IsNull (), "Clip exterior segment");
            DRay2d rayC1M (pointC1, pointM);
            Check::True (rayC1M.NormalizeDirectionInPlace ());
            range = hull.ClipRay (rayC1M);
            if (Check::False (range.IsNull (), "Clip mixed segment"))
                {
                Check::True (DoubleOps::AlmostEqual (range.Length (), pointA.Distance (pointB)), "Clipped segment length");
                }
            }
        }

    // construct known exterior rays
    for (size_t i = 0; i < points.size (); i++)
        {
        DPoint2d pointA = points[i];
        DPoint2d pointB = points[(i + 1) % points.size ()];
        DPoint2d pointA1 = DPoint2d::FromForwardLeftInterpolate (pointA, 0.0, -0.1, pointB);
        DPoint2d pointB1 = DPoint2d::FromForwardLeftInterpolate (pointA, 1.0, -0.1, pointB);
        DRay2d ray (pointA1, pointB1);

        DRange1d range = hull.ClipRay (ray);
        Check::True (range.IsNull (), "Clip exterior segment");
        }

    // Construct a grid of parallel segments ...
    DRay2d scanBase (DPoint2d::From (1, 4), DVec2d::From (1, 2));
    scanBase.NormalizeDirectionInPlace ();
    DRange1d hullRange = hull.RangePerpendicularToRay (scanBase);
    double parallelDistance = 2.25;
    double epsilon = 0.01;
    for (double a = hullRange.Low () + epsilon; a + epsilon <= hullRange.High (); a += parallelDistance)
        {
        DRay2d scanRay = scanBase.ParallelRay (a);
        // a is strictly within -- expect an interior segment ..
        DRange1d range = hull.ClipRay (scanRay);
        Check::True (!range.IsNull (), "Clip scan segment");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Geometry, ConvexHullQueriesContstruction)
    {

    bvector<DPoint2d> points
        {
        // These are the hull (CW)
        DPoint2d::From (0, 0),
        DPoint2d::From (10, 0),
        DPoint2d::From (20, 10),
        DPoint2d::From (10, 20),
        DPoint2d::From (0, 10),
        // These are inside
        DPoint2d::From (5, 5),
        DPoint2d::From (2, 4),
        DPoint2d::From (12, 4),
        DPoint2d::From (13, 5)
        };
    size_t insideBase = 5;
    DConvexPolygon2d hull (points);

    for (size_t i = insideBase; i < points.size (); i++)
        {
        Check::True (
            hull.ContainsPoint (points[i]),
            "point inside hull");
        }

    CheckHullChords (hull, 3);
    }

DPoint2d Lisajoue (double theta, double a)
    {
    double r = cos (a * theta);
    return DPoint2d::From (r * cos (theta), r * sin (theta));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Geometry, ConvexHullManyPoints)
    {

    bvector<DPoint2d> points;
    double a = 3.29;
    double dtheta = 0.34;
    size_t numPoints = 1000;
    for (double theta = 0.01; points.size () < numPoints; theta += dtheta)
        {
        points.push_back (Lisajoue (theta * theta, a));
        }

    DConvexPolygon2d hull (points);

    for (size_t i = 0; i < points.size (); i++)
        {
        Check::True (
            hull.ContainsPoint (points[i]),
            "point inside hull");
        }

    // CheckHullChords (hull, hull.Points().size () / 3);   // This has nasty tolerance problems -- short edges, near-zero cross products
    Check::True (CheckHullRaysFromCentroid (hull));
    double offsetDistance = 1.0;
    auto hull1 = hull;
    hull1.OffsetInPlace (offsetDistance);
    auto hull2 = hull1;
    hull2.OffsetInPlace (0.01 * offsetDistance);
    size_t n = hull.Points ().size ();
    Check::Size (n, CountPointsInHull (hull1, hull.Points ()));
    Check::Size (0, CountPointsInHull (hull, hull1.Points ()));
    Check::Size (0, CountPointsInHull (hull1, hull2.Points ()));
    Dump (points);
    Dump (hull.Points ());
    Check::ClearGeometry ("Geometry.ConvexHullManyPoints");
    }


//===================================================================

//! Pair of points (named "A" and "wallX") with a tag indicating how constructed.
struct MPathPoint2d {
    enum class Type
        {
        Unknown,
        Initial,
        Final,
        MidEdge,
        ExteriorSwingStart,
        ExteriorSwingIntermediate,
        ExteriorSwingEnd,
        InteriorSwingStart,
        InteriorSwingIntermediate,
        InteriorSwingEnd
        };

    Type m_type;
    DPoint2d m_xyA;
    DPoint2d m_xyB;

    MPathPoint2d ()
        : m_type (Type::Unknown),
        m_xyA (DPoint2d::From (0, 0)),
        m_xyB (DPoint2d::From (0, 0))
        {
        }
    MPathPoint2d (Type type, DPoint2d const &xyA, DPoint2d const &xyB)
        : m_type (type),
        m_xyA (DPoint2d::From (0, 0)),
        m_xyB (DPoint2d::From (0, 0))
        {
        }
    };
//! Virtual method interface structure for algorithms to announce MPathPoint2d.
struct MAnnouncePathPoint
    {
    virtual void Announce (MPathPoint2d const &data) = 0;
    };
//! Implement MAnnouncePathPoint -- collect the path points in an bvector.
struct MPathPoint2dCollector : MAnnouncePathPoint
    {
    //! Publicly accessible collected points ...
    bvector<MPathPoint2d> path;

    //! Implement the Announce method to capture points ...
    void Announce (MPathPoint2d const &data) override
        {
        path.push_back (data);
        }
    };

//! Algorithm context for building offset paths.
//!
//! Build offset paths.
//! At each offset point, the MPathPoint2d contains:
//!<ul>
//!<li>xyA = point on the offset path.
//!<li>xyB = target point.
//!<li>type = one of:
//!<ul>
//!<li>Initial -- first point of the path.
//!<li>MidEdge -- point in the strict interior of a straight edge of the target geometry.
//!<li>ExteriorSwingStart -- first point of a swing around a corner.   Start, Intermediate and End points of the swing rotate around the same target point
//!<li>ExteriorSwingIntermediate -- zero, one, or many points as the is swinging around the target point.
//!<li>ExteriorSwingEnd -- final point of a swing around a target point.
//!<li>InteriorSwingStart -- First point of a rotation to view in interior corner.
//!<li>InteriorSwingIntermedate -- Zero, one, or many points as the path rotates to view the interior corner.
//!<li>InteriorSwingEnd -- final point of interior swing.
//!<li>Final -- last point of path.
//!</ul>
//!<li>The regular expression for the tag sequence is:
//! Start ( (MideEdge*) | (ExteriorSwingStart ExteriorSwingIntermediate* ExteriorSwingEnd) | (InteriorSwingStart InteriorSwingIntermediate* InteriorSwingEnd) Final
//!<li>Point density is controlled by these methods:
//!<ul>
//!<li>SetLinearStep (h) -- sets the max distance between path points along a straight edge of the target geometry.
//!<li>SetOffsetDistance (d) -- set the distance to offset.   Positive is to the right of the path, negative is to left.
//!<li>SetExteriorSwingStep (theta) -- sets the max angular step when moving around the outside of a corner.
//!<li>SetInteriorSwingStep (theta) -- sets the max angular step when swing to view an inside corner.
//!</ul>
//!</ul>
struct MOffsetPathBuilder
    {
    private:
        double m_linearStep;
        Angle m_exteriorSwingStep;
        Angle m_interiorSwingStep;
        double m_offsetDistance;
        MAnnouncePathPoint &m_announcer;

        void Announce (MPathPoint2d::Type type, DPoint2d const &xy, DPoint2d const &target)
            {
            m_announcer.Announce (MPathPoint2d (type, xy, target));
            }
        // ALWAYS add start.
        // add intermediates as needed by linearStep.
        // ALWAYS add end.
        void AddStepsOnLine
            (
                DPoint2d const &xyA,
                DPoint2d const &xyB,
                DVec2d const &offsetVector,
                MPathPoint2d::Type typeA,
                MPathPoint2d::Type type,
                MPathPoint2d::Type typeB
                )
            {
            Announce (typeA, xyA + offsetVector, xyA);
            double distance = xyA.Distance (xyB);
            if (m_linearStep > 0.0 && distance > m_linearStep)
                {
                double numStep = ceil (fabs (distance) / m_linearStep);    // just leave as double ..
                double df = 1.0 / numStep;
                for (int i = 1; i < numStep; i++)
                    {
                    DPoint2d xy = DPoint2d::FromInterpolate (xyA, i * df, xyB);
                    Announce (type, xy + offsetVector, xy);
                    }
                }
            Announce (typeB, xyB + offsetVector, xyB);
            }

        // NEVER add initial point
        // add intermediates as necessary
        // NEVER add last
        void AddExteriorSwing
            (
                DPoint2d const &xyTarget,        // center of rotation
                DVec2d const &offsetVector,
                Angle const &totalTurnAngle
                )
            {
            double totalRadians = totalTurnAngle.Radians ();
            double stepRadians = m_exteriorSwingStep.Radians ();
            if (stepRadians > 0.0 && fabs (totalRadians) > stepRadians)
                {
                double numStep = ceil (fabs (totalRadians) / stepRadians);
                double df = 1.0 / numStep;
                for (int i = 1; i < numStep; i++)
                    {
                    Angle turn = totalTurnAngle * (i * df);
                    DVec2d radialVector = DVec2d::FromRotateCCW (offsetVector, turn.Radians ());
                    Announce (MPathPoint2d::Type::ExteriorSwingIntermediate, xyTarget + radialVector, xyTarget);
                    }
                }
            }

        // NEVER add initial point
        // add intermediates as necessary
        // NEVER add last
        void AddInteriorSwing
            (
                DPoint2d const &xyTargetA,        // first target
                DVec2d const &offsetVector,  // first target to center
                Angle const &totalTurnAngle    // (Expect negative)
                )
            {
            double totalRadians = totalTurnAngle.Radians ();
            double stepRadians = m_interiorSwingStep.Radians ();
            if (stepRadians > 0.0 && fabs (totalRadians) > stepRadians)
                {
                double numStep = ceil (fabs (totalRadians) / stepRadians);
                double df = 1.0 / numStep;
                auto center = xyTargetA + offsetVector;
                for (int i = 1; i < numStep; i++)
                    {
                    Angle turn = totalTurnAngle * (i * df);
                    DVec2d radialVector = DVec2d::FromRotateCCW (offsetVector, turn.Radians ());
                    Announce (MPathPoint2d::Type::InteriorSwingIntermediate, center, center - radialVector);
                    }
                }
            }

    public:
        //! Construct an MOffsetPathBuilder with default parameters.
        MOffsetPathBuilder (MAnnouncePathPoint &announcer)
            : m_linearStep (0.0),
            m_exteriorSwingStep (Angle ()),
            m_interiorSwingStep (),
            m_offsetDistance (0.0),
            m_announcer (announcer)
            {
            }
        //! Set the offset distance.  Positive is to the right of the path, negative is to left.
        void SetOffsetDistance (double data) { m_offsetDistance = data; }
        //! Set the maximum distance between offset points when moving along a straight edge.
        void SetLinearStep (double data) { m_linearStep = data; }
        //! Set the maximum angular swing when rotating around an outside corner.
        void SetExteriorSwingStep (Angle data) { m_exteriorSwingStep = data; }
        //! Set the maximum angular swing when swinging (in place) to view an inside corner.
        void SetInteriorSwingStep (Angle data) { m_interiorSwingStep = data; }

        //! build a path offset from a sequence of target edges.
        //! At each path points
        //!<ul>
        //!<li>xyA is the path coordinate
        //!<li>xyB is the target (viewed) point
        //!<li>type indicates what motion the path is following.
        //!</ul>
        void BuildAlongPolyline (bvector<DPoint2d> const &targetXY)
            {
            if (targetXY.size () < 2)
                return;
            MPathPoint2d::Type typeA = MPathPoint2d::Type::Initial;
            DPoint2d xyA = targetXY.front ();
            size_t targetIndexB = 1;
            Angle angleA;    // initialized to 0.
            double offsetSign = m_offsetDistance >= 0.0 ? 1.0 : -1.0;
            for (; targetIndexB + 1 < targetXY.size (); targetIndexB++)
                {
                size_t targetIndexC = targetIndexB + 1;
                DPoint2d xyB = targetXY[targetIndexB];
                DPoint2d xyC = targetXY[targetIndexC];
                DVec2d deltaAB = DVec2d::FromStartEnd (xyA, xyB);
                DVec2d deltaBC = DVec2d::FromStartEnd (xyB, xyC);
                DVec2d unitAB = deltaAB;
                DVec2d unitBC = deltaBC;
                if (unitAB.Normalize ()
                    && unitBC.Normalize ()
                    )
                    {
                    Angle angleB = Angle::FromRadians (deltaAB.AngleTo (deltaBC));
                    auto offsetVector = m_offsetDistance * unitAB.Rotate90CW ();
                    if (offsetSign * angleB.Radians () >= 0.0)
                        {
                        AddStepsOnLine (xyA, xyB, offsetVector,
                            typeA,
                            MPathPoint2d::Type::MidEdge,
                            MPathPoint2d::Type::ExteriorSwingStart);
                        AddExteriorSwing (xyB, offsetVector, angleB);
                        typeA = MPathPoint2d::Type::ExteriorSwingEnd;
                        xyA = xyB;
                        }
                    else
                        {
                        Angle halfAngle = 0.5 * angleB;
                        double setbackDistance = -m_offsetDistance * halfAngle.Tan ();
                        auto xySetback = xyB - setbackDistance * unitAB;
                        AddStepsOnLine (xyA, xySetback, offsetVector,
                            typeA,
                            MPathPoint2d::Type::MidEdge,
                            MPathPoint2d::Type::InteriorSwingStart);
                        AddInteriorSwing (xySetback, offsetVector, angleB);
                        xyA = xyB + setbackDistance * unitBC;
                        typeA = MPathPoint2d::Type::InteriorSwingEnd;
                        }
                    if (targetIndexB + 2 == targetXY.size ())
                        {
                        auto offsetVectorC = m_offsetDistance * unitBC.Rotate90CW ();
                        AddStepsOnLine (
                            xyA, xyC, offsetVectorC,
                            typeA,
                            MPathPoint2d::Type::MidEdge,
                            MPathPoint2d::Type::Final
                            );
                        }
                    }
                }
            }
    };





static void KeyinColor (int color)
    {
    printf ("Active color %d\n", color);
    }
static void KeyinTargets (MPathPoint2dCollector &data)
    {
    if (Check::IsSuppressed (MULTI_STRUCTURE_PRINT_VOLUME))
        return;
    for (auto &target : data.path)
        {
        printf ("PLACE SMARTLINE\n");
        KeyinColor (1 + (int)target.m_type);
        printf ("xy=%.17g,%.17g\n", target.m_xyA.x, target.m_xyA.y);
        printf ("xy=%.17g,%.17g\n", target.m_xyB.x, target.m_xyB.y);
        }
    printf ("PLACE SMARTLINE\n");
    KeyinColor (15);
    for (auto &target : data.path)
        {
        printf ("xy=%.17g,%.17g\n", target.m_xyA.x, target.m_xyA.y);
        }
    printf ("PLACE SMARTLINE\n");
    KeyinColor (8);
    for (auto &target : data.path)
        {
        printf ("xy=%.17g,%.17g\n", target.m_xyB.x, target.m_xyB.y);
        }
    }
static void KeyinLinestring (bvector<DPoint2d> const &data, int color = 1)
    {
    if (Check::IsSuppressed (MULTI_STRUCTURE_PRINT_VOLUME))
        return;
    printf ("PLACE SMARTLINE\n");
    KeyinColor (color);
    for (auto xy : data)
        {
        printf ("xy=%.17g,%.17g\n", xy.x, xy.y);
        }
    printf ("PLACE SMARTLINE\n");
    }

void TestPath (bvector<DPoint2d> const &points, DVec2d const &shift, double offsetDistance)
    {
    bvector<DPoint2d> shiftedPoints;
    for (auto xy : points)
        shiftedPoints.push_back (xy + shift);
    MPathPoint2dCollector announcer;
    MOffsetPathBuilder builder (announcer);
    builder.SetOffsetDistance (offsetDistance);
    builder.SetLinearStep (4.0);
    builder.SetExteriorSwingStep (Angle::FromDegrees (40.0));
    builder.SetInteriorSwingStep (Angle::FromDegrees (30.0));
    builder.BuildAlongPolyline (shiftedPoints);
    KeyinLinestring (shiftedPoints);
    KeyinTargets (announcer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Geometry, PathBuilder)
    {
    bvector<DPoint2d> points
        {
        DPoint2d::From (5, 0),
        DPoint2d::From (10, 0),
        DPoint2d::From (10, 10),
        DPoint2d::From (20, 10),
        DPoint2d::From (40, 20),
        DPoint2d::From (50, 30),
        DPoint2d::From (0, 15),
        DPoint2d::From (0, 0),
        DPoint2d::From (4.5, 0)
        };
    double shiftX = 0.0;
    TestPath (points, DVec2d::From (0, 0), 1.0);
    shiftX += 60.0;
    TestPath (points, DVec2d::From (shiftX, 0), -1.0);
    shiftX += 60.0;
#ifdef TestLargeOffset
    TestPath (points, DVec2d (shiftX, 0), 5.0);
    shiftX += 60.0;
    TestPath (points, DVec2d (shiftX, 0), 20.0);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Geometry, ScanRays)
    {
    bvector<DPoint2d> points
        {
        DPoint2d::From (-8416019.016781, 4455112.022850),
        DPoint2d::From (-8415955.314612, 4455078.984322),
        DPoint2d::From (-8415921.022189, 4455078.399881),
        DPoint2d::From (-8415895.801577, 4455104.715051),
        DPoint2d::From (-8415872.330896, 4455157.749142),
        DPoint2d::From (-8415928.376437, 4455194.276293),
        DPoint2d::From (-8416002.202269, 4455176.636193)
        };
    bool validAsGiven = false;
    DConvexPolygon2d hull (points, validAsGiven);
    Check::True (validAsGiven);
    double angleInRadians = -1.882268;
    double distanceBetweenFlightLines = 14.471536;

    bvector<DPoint2d> convexHullArray = hull.Points ();
    Dump (convexHullArray);
    DVec2d gridDirection0 = DVec2d::FromStartEnd (convexHullArray[0], convexHullArray[1]);
    gridDirection0.Normalize ();
    DVec2d gridDirection1 = cos (angleInRadians) * gridDirection0 + sin (angleInRadians) * gridDirection0.Rotate90CCW ();
    DRay2d scanBase (convexHullArray[0], gridDirection1);
    scanBase.NormalizeDirectionInPlace ();
    DRange1d hullRange = hull.RangePerpendicularToRay (scanBase);
    double parallelDistance = distanceBetweenFlightLines;

    bvector<DPoint2d> rawflightPathArray;
    //    bool forwardSegment = true;
    double epsilon = 1.0e-6;
    DRange1d outsideRange;
    for (double a = hullRange.Low () + epsilon + (.5 * parallelDistance); a + epsilon <= hullRange.High (); a += parallelDistance)
        {
        DRay2d scanRay = scanBase.ParallelRay (a);
        DRange1d range = hull.ClipRay (scanRay);
        auto xy0 = scanRay.FractionToPoint (range.Low ());
        auto xy1 = scanRay.FractionToPoint (range.High ());
        double d0 = hull.DistanceOutside (xy0);
        double d1 = hull.DistanceOutside (xy1);
        Dump (scanRay.FractionToPoint (0.0), scanRay.FractionToPoint (1.0));
        Dump (xy0, xy1);
        outsideRange.Extend (d0);
        outsideRange.Extend (d1);
        }

    Check::LessThanOrEqual (outsideRange.High (), 1.0e-8);
    Check::ClearGeometry ("Geometry.ScanRays");
    }

// CameraMath has static methods for various camera calculations.
struct CameraMath
    {
    // Given: distanceToWall, cameraX, cameraSwing, cameraHalfAngle
    // Return: wallX
    static double ComputeWallX (double distanceToWall, double cameraX, Angle cameraSwingFromPerpendicular, Angle cameraAngleFromMidline)
        {
        return cameraX + distanceToWall * tan (cameraSwingFromPerpendicular.Radians () + cameraAngleFromMidline.Radians ());
        }
    // Given: distanceToWall, wallX, cameraSwing, cameraHalfAngle
    // Return: cameraX
    static double ComputeCameraX (double distanceToWall, Angle cameraSwingFromPerpendicular, Angle cameraAngleFromMidline, double wallX)
        {
        return wallX - distanceToWall * tan (cameraSwingFromPerpendicular.Radians () + cameraAngleFromMidline.Radians ());
        }

    // Given: distanceToWall, cameraX, wallX, cameraHalfAngle
    // Return: cameraSwing
    static Angle ComputeCameraSwingFromPerpendicular (double distanceToWall, double cameraX, Angle cameraAngleFromMidline, double wallX)
        {
        double dx = wallX - cameraX;
        double betaRadians = atan2 (dx, distanceToWall);
        return Angle::FromRadians (betaRadians - cameraAngleFromMidline.Radians ());
        }

    // Given: distanceToWall, cameraHalfAngle, overlapFraction
    // Return: cameraSwingAngle, cameraStep
    static void SetupCameraSwingAndStep
        (
            double distanceToWall,  // Distance from camera to wall for "head on" (middle) of 3 shots from each camera position.
            Angle cameraHalfAngle, // angle from camera midline to edge
            double overlapFraction,  // fractional overlap between consecutive images
            Angle &cameraSwingAngle,           // Angle between left, middle, and right shots at fixed camera position
            double &cameraStep      // camera motion along wall between steps.
            )
        {

        // Interval0 = (x0Left .. x0Right) = camera pointing directly at (0,0) from above
        double x0Left = CameraMath::ComputeWallX (distanceToWall, 0.0, Angle::FromDegrees (0.0), -cameraHalfAngle);
        double x0Right = CameraMath::ComputeWallX (distanceToWall, 0.0, Angle::FromDegrees (0.0), cameraHalfAngle);
        // Interval1 = (x1Left .. x1Right) = camera still above (0,0) but rotated "forward".
        //  this intervals x1Left is required to be fractional position "overlapFraction" from x0Right back towards x0Left
        double x1Left = DoubleOps::Interpolate (x0Right, overlapFraction, x0Left);
        // Find camera swing to make the back edge of the forward swing image
        //   have the right overlap with the headon image
        cameraSwingAngle = CameraMath::ComputeCameraSwingFromPerpendicular (distanceToWall, 0.0, -cameraHalfAngle, x1Left);


        // alpha and cameraSwingAngle now stay fixed.

        // Compute the other end of the Interval1 ..
        double x1Right = CameraMath::ComputeWallX (distanceToWall, 0.0, cameraSwingAngle, cameraHalfAngle);
        // Interval2 = (x2Left..x2Right) as seen when camera looks backward after first move along the wall.
        // x2Left is set within Interval1 to get overlap
        double x2Left = DoubleOps::Interpolate (x1Right, overlapFraction, x1Left);
        // make back end of field of view hit x2Left ...
        cameraStep = CameraMath::ComputeCameraX (distanceToWall, -cameraSwingAngle, -cameraHalfAngle, x2Left);
        }

    };

void PrintCameraSetup
(
    double distanceToWall,  // Distance from camera to wall for "head on" (middle) of 3 shots from each camera position.
    Angle cameraHalfAngle, // angle from camera midline to edge
    double overlapFraction,  // fractional overlap between consecutive images
    Angle cameraSwingAngle,           // Angle between left, middle, and right shots at fixed camera position
    double &cameraStep      // camera motion along wall between steps.
    )
    {
    CameraMath::SetupCameraSwingAndStep (distanceToWall, cameraHalfAngle, overlapFraction, cameraSwingAngle, cameraStep);
    bool doPrint = Check::PrintFixedStructs ();
    if (doPrint)
        {
        printf ("\n ** CAMERA SETUP\n");
        printf ("  (DistanceFromWall %.17g)\n", distanceToWall);
        printf ("  (cameraHalfAngle %.17g)\n", cameraHalfAngle.Degrees ());
        printf ("  (overlap fraction %g)\n\n", overlapFraction);
        printf ("  (cameraSwingAngle %.17g)\n", cameraSwingAngle.Degrees ());
        printf ("  (CameraXStep  %.17g)\n\n", cameraStep);
        }
    for (size_t i = 0; i < 3; i++)
        {
        double cameraX = i * cameraStep;
        if (doPrint)
            printf (" (CameraXCoordinate %.17g)\n", cameraX);
        for (auto thetaMultiplier : bvector<double>{ -1.0, 0.0, 1.0 })
            {
            Angle theta = Angle::FromRadians (thetaMultiplier * cameraSwingAngle.Radians ());
            double x0 = CameraMath::ComputeWallX (distanceToWall, cameraX, theta, -cameraHalfAngle);
            double x1 = CameraMath::ComputeWallX (distanceToWall, cameraX, theta, cameraHalfAngle);
            if (doPrint)
                printf ("    (swing %g) (ImageX %.17g   %.17g (size %.17g))\n", theta.Degrees (), x0, x1, x1 - x0);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Geometry, WallStepper)
    {
    double cameraStep;
    Angle cameraSwingAngle;
    PrintCameraSetup (2.0, Angle::FromDegrees (10.0), 0.60, cameraSwingAngle, cameraStep);
    PrintCameraSetup (2.0, Angle::FromDegrees (20.0), 0.60, cameraSwingAngle, cameraStep);
    PrintCameraSetup (2.0, Angle::FromDegrees (30.0), 0.60, cameraSwingAngle, cameraStep);
    }
