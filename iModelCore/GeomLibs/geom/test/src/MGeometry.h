/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#ifdef WHYARETHESEHERE
typedef OrderedValueSizeSize <double> DoubleSizeSize;
typedef ValueSizeSize <DRange3d> DRange3dSizeSize;
typedef ValueSizeSize <DPoint3d> DPoint3dSizeSize;
#endif

//! @description Templatized type carrying a value type with a boolean indicating if the value
//!     is considered valid.
template<typename T>
struct MValidatedValue
    {
    private:
        T m_value;
        bool m_isValid;
    public:
        //! Default value, marked not valid.
        MValidatedValue() : m_value(), m_isValid(false) {}
        //! Specified value, marked valid.
        MValidatedValue(T const &value) : m_value(value), m_isValid(true) {}
        //! Specified value, with validity indicated by caller
        MValidatedValue(T const &value, bool isValid) : m_value(value), m_isValid(isValid) {}
        //! explicty query for the "the value", without regard for the validity flag.
        T const &Value() const { return m_value; }
        T &Value() { return m_value; }
        //! return the validity flag.
        bool IsValid() const { return m_isValid; }
        //! return the validity flag, copy value to parameter.
        bool IsValid(T &value) const { value = m_value; return m_isValid; }
        //! update the validit member
        void SetIsValid(bool value) { m_isValid = value; }
        //! Implicity type conversion operator Returns "the value", without regard for the validity flag.
        operator T () { return m_value; }
        //! Assign to the T value.  Mark as valid.
        void operator = (T const &value) { m_value = value; m_isValid = true; }
    };

struct MAngle;
typedef MValidatedValue <double> MValidatedDouble;
typedef MValidatedValue <MAngle> MValidatedMAngle;



// "M" Geometry classes are geometry classes for initial porting to new environments.
// (M maybe understood to mean "minimal", or to suggest the "m" in apple objective C)
// MPoint2d -- 2d point, with x and y components.
// MVector2d -- 2d vector, with x and y components.
// MRay2d -- 2d ray defined by start and direction.
// MDistance -- static methods for tolerance tests and scalar computation.
// RULES FOR PORTABILITY (??!!??)
// 1) No known array type other than simple pointer (ouch)
// 2) No operator overload
// 3) Call by reference (except contiguous buffer array pointers)
// 4) Return values by array
// 5) Strong (but alas not attainable) preference for single return values



//! MDistance class contains only static methods for tolerance-aware distance comparisons
struct MDistance
{
public:
//! Test if {a} is a nearly zero distance.
static bool AlmostZero (double a)
    {
    return fabs (a) <= 1.0e-8;
    }
//! Test if {a} is a nearly zero distance squared.
static bool AlmostZeroSquared (double a)
    {
    return a <= 1.0e-16;
    }
//! Test if {a} and {b} have nearly zero distance between.
static bool AlmostEqual (double a, double b)
    {
    return AlmostZero (a-b);
    }
//! Compute {a/b}, considering {b} to be zero if the result is larger than the max allowed system distance.
static MValidatedDouble SafeDivideDistance (double a, double b)
    {
    if (fabs (a) < fabs (b) * 1.0e10)
        return MValidatedDouble (a/b, true);
    return MValidatedDouble (0.0, false);
    }
static double Interpolate (double a, double f, double b)
    {
    return (1.0 - f) * a + f * b;
    }
};
//! range interval with low, high.  In initial state, the inteval is "null".
struct MRange1d
{
double m_low, m_high;
MRange1d ()
    {
    m_low = DBL_MAX;
    m_high = -DBL_MAX;
    }
//! @return true if the interval is in initial state.
bool IsNull ()
    {
    return m_low > m_high;
    }

//! @return true if the interval has received data since initialization
bool IsValid ()
    {
    return m_high >= m_low;
    }


double Low (){return m_low;}
double High (){return m_high;}
double Length (){return IsValid () ? m_high - m_low : 0.0;}
void Extend (double a)
    {
    if (a < m_low)
        m_low = a;
    if (a > m_high)
        m_high = a;
    }
};
struct MPoint2d;
struct MAngle;
//! MVector2d is a 2d vector with x and y components.
struct MVector2d
{
friend struct MPoint2d;
private:
double m_x, m_y;
public:
//! Constructor -- zero vector.
MVector2d ()
    {
    m_x = m_y = 0.0;
    }
//! Constructor -- components given.
MVector2d (double x, double y)
    {
    m_x = x;
    m_y = y;
    }
//! return X component
double X() const { return m_x;}
//! return Y component
double Y() const { return m_y;}

MVector2d Scale (double scale) const { return MVector2d (scale * m_x, scale * m_y);}
MVector2d Add (MVector2d const &other) const { return MVector2d(m_x + other.m_x, m_y + other.m_y); }

//! Return a CCW perpendicular vector with the same length.
MVector2d CCWPerpendicularVector () const
    {
    return MVector2d (-m_y, m_x);
    }
//! Return a CW perpendicular vector with the same length.
MVector2d CWPerpendicularVector () const
    {
    return MVector2d (m_y, -m_x);
    }
//! Return a vector rotated by (signed) angle.
MVector2d Rotate (MAngle const &angle) const;
//! Test if both components are individually near zero.
bool IsNearZero () const
    {
    return MDistance::AlmostZero (m_x) && MDistance::AlmostZero (m_y);
    }
//! return the dot product of instance and other vector.
double DotProduct (MVector2d const &other) const
    {
    return m_x * other.m_x + m_y * other.m_y;
    }
//! return the (scalar, 2d) cross produc to of instance and other vector.
double CrossProduct (MVector2d const &other) const
    {
    return m_x * other.m_y - m_y * other.m_x;
    }
//! Constructor fom specified start to end.
MVector2d (MPoint2d const &start, MPoint2d const &end);

//! Return the magnitude.
double Magnitude () const
    {
    return sqrt (m_x * m_x + m_y * m_y);
    }

//! Return the squared magnitude.
double MagnitudeSquared () const
    {
    return m_x * m_x + m_y * m_y;
    }
//! Normalize the vector in place.
//! @return false if the magnitude is near zero.
bool NormalizeInPlace ()
    {
    double a = Magnitude ();
    if (MDistance::AlmostZero (a))
        return false;
    m_x /= a;
    m_y /= a;
    return true;
    }

//! Multliply x,y in place
void ScaleInPlace (double s)
    {
    m_x *= s;
    m_y *= s;
    }

//! Divide in place, using Distance::SafeDivideDistance.
//! Returns false with no change if unable to divide.
bool SafeDivideInPlace (double s)
    {
    auto x = MDistance::SafeDivideDistance (m_x, s);
    auto y = MDistance::SafeDivideDistance (m_y, s);
    if (x.IsValid () && y.IsValid ())
        {
        m_x = x.Value ();
        m_y = y.Value ();
        return true;
        }
    return false;
    }

//! @return true if the vector's mangitude is near zero
bool IsNearZeroMagnitude () const
    {
    return MDistance::AlmostZeroSquared (MagnitudeSquared ());
    }

//! @return the fractional position along the instance when the other vector is projected perpendicular to the instance.
//! @remark 0.0 is returned if instance length is zero !!!
MValidatedDouble ProjectionFraction (MVector2d const &other) const
    {
    double uv = DotProduct (other);
    double uu = MagnitudeSquared ();
    return MDistance::SafeDivideDistance(uv, uu);
    }

//! @return the fractional position along the instance when the other vector is projected perpendicular to the instance.
//! @remark 0.0 is returned if instance length is zero !!!
MValidatedDouble PerpendicularProjectionFraction (MVector2d const &other) const
    {
    double uv = CrossProduct (other);
    double uu = MagnitudeSquared ();
    return MDistance::SafeDivideDistance (uv, uu);
    }

static MVector2d OffsetBisector (MVector2d const &unitPerpA, MVector2d const &unitPerpB, double offset);
};


//! MPoint2d is a point with x,y coordinates.
struct MPoint2d
{
friend struct MVector2d;
private:
double m_x, m_y;
public:
//! Constructor -- point with 00 coordinates
MPoint2d ()
    {
    m_x = m_y = 0.0;
    }
//! Constructor -- point with coordintes x,y
MPoint2d (double x, double y)
    {
    m_x = x;
    m_y = y;
    }

//! @Return x coordinate
double X() const { return m_x;}
//! @return y coordinate
double Y() const { return m_y;}

//! @Return the subtaction (vector!) {instance - other}
MVector2d Subtract (MPoint2d const &other) const
    {
    return MVector2d (m_x - other.m_x, m_y - other.m_y);
    }
//! @Return the point {instance + vector * scale}
MPoint2d Add (MVector2d const &vector, double scale) const
    {
    return MPoint2d (m_x + vector.m_x * scale, m_y + vector.m_y * scale);
    }

//! @Return the point {instance + vector}
MPoint2d Add(MVector2d const &vector) const
    {
    return MPoint2d(m_x + vector.m_x, m_y + vector.m_y);
    }

//! @Return the point {instance - vector}
MPoint2d Subtract(MVector2d const &vector) const
    {
    return MPoint2d(m_x - vector.m_x, m_y - vector.m_y);
    }

//! @return sum of point plus (forwardFraction times vector) plus (leftFraction time perpendicular CCW rotation of vector)
MPoint2d AddForwardAndLeft (MVector2d const &vector, double forwardFraction, double leftFraction) const
    {
    return MPoint2d (
            m_x + vector.m_x * forwardFraction - vector.m_y * leftFraction,
            m_y + vector.m_y * forwardFraction + vector.m_x * leftFraction);
    }
//! @return the distance from instance to other.
double Distance (MPoint2d const &other) const
    {
    return MVector2d (*this, other).Magnitude ();
    }

//! @return point interpolated at fraction from instance to other
MPoint2d Interpolate (double fraction, MPoint2d const &other) const
    {
    return MPoint2d (
            m_x + fraction * (other.m_x - m_x),
            m_y + fraction * (other.m_y - m_y)
            );
    }

//! @return point interpolated at fraction from instance to other
MPoint2d InterpolateForwardAndLeft (double forwardFraction, double leftFraction, MPoint2d const &other) const
    {
    return AddForwardAndLeft (MVector2d (*this, other), forwardFraction, leftFraction);
    }

//! @return true if coordinates are almost equal to other.
bool IsAlmostEqual (MPoint2d const &other) const
    {
    return MDistance::AlmostEqual (m_x, other.m_x) && MDistance::AlmostEqual (m_y, other.m_y);
    }
//! @return the cross product of vectors from the instance to targetA and targetB
double CrossProductToTargets (MPoint2d const &targetA, MPoint2d const &targetB) const
    {
    MVector2d vectorA (*this, targetA);
    MVector2d vectorB (*this, targetB);
    return vectorA.CrossProduct (vectorB);
    }

//! @return true if a is "to the left of, or below with equal x"
static bool LexicalXYLessThan (MPoint2d const &a, MPoint2d const &b)
    {
    if (a.m_x < b.m_x)
        return true;
    if (a.m_x > b.m_x)
        return false;
    return a.m_y < b.m_y;
    }

};

MVector2d operator* (MVector2d const &vector, double scale) { return vector.Scale(scale); }
MVector2d operator* (double scale, MVector2d const &vector) { return vector.Scale(scale); }
MPoint2d operator+ (MPoint2d const &point, MVector2d const &vector) { return point.Add(vector); }
MPoint2d operator- (MPoint2d const &point, MVector2d const &vector) { return point.Subtract(vector); }

MVector2d operator+ (MVector2d const &vectorA, MVector2d const &vectorB) { return vectorA.Add(vectorB); }

struct MPolygon
{
static bool Centroid (std::vector<MPoint2d> &points, double &area, MPoint2d &centroid)
    {
    area = 0.0;
    centroid = MPoint2d(0,0);
    if (points.size() == 0)
        return false;
    MVector2d vectorSum (0,0);  // == sum ((U+V)/3) * (U CROSS V)/2 -- but leave out divisions
    double    areaSum = 0.0;    // == sum (U CROSS V) / 2 -- but leave out divisions
    MPoint2d origin = points[0];
    for (size_t i = 1; i + 1< points.size (); i++)
        {
        MVector2d vector0 (origin, points[i]);
        MVector2d vector1 (origin, points[i+1]);
        double area = vector0.CrossProduct (vector1);
        vectorSum = vectorSum + (vector0 + vector1) * area;
        areaSum += area;
        }
    area = areaSum * 0.5;
    vectorSum.ScaleInPlace (1.0 / 6.0);
    if (vectorSum.SafeDivideInPlace (area))
        {
        centroid = origin + vectorSum;
        return true;
        }
    centroid = origin;
    return false;
    }
};


//! Ray defined by origin and direction vector.
struct MRay2d
{
friend struct MPoint2d;
friend struct MConvexPolygon2d;
private:
MPoint2d m_origin;
MVector2d m_direction;
public:
MRay2d ()
    {
    }
MRay2d (MPoint2d const &origin, MPoint2d const &target)
    {
    m_origin = origin;
    m_direction = MVector2d (origin, target);
    }

MRay2d (MPoint2d const &origin, MVector2d const &direction)
    {
    m_origin = origin;
    m_direction = direction;
    }
//! Return a ray that is parallel at distance to the left, specified as fraction of the ray's direction vector.
MRay2d ParallelRay (double leftFraction) const
    {
    return MRay2d
        (
        m_origin.AddForwardAndLeft (m_direction, 0.0, leftFraction),
        m_direction
        );
    }

MRay2d CCWPerpendicularRay () const
    {
    return MRay2d
        (
        m_origin,
        m_direction.CCWPerpendicularVector ()
        );
    }

MRay2d CWPerpendicularRay () const
    {
    return MRay2d
        (
        m_origin,
        m_direction.CWPerpendicularVector ()
        );
    }

bool NormalizeDirectionInPlace ()
    {
    return m_direction.NormalizeInPlace ();
    }

MPoint2d FractionToPoint (double f) const
    {
    return m_origin.Add (m_direction, f);
    }

// Intersect this ray (ASSUMED NORMALIZED) with unbounded line defined by points.
// (The normalization assumption affects test for parallel vectors.)
bool IntersectUnboundedLine (MPoint2d const linePointA, MPoint2d const &linePointB, double &fraction, double &dhds) const
    {
    MVector2d lineDirection = MVector2d (linePointA, linePointB);
    MVector2d vector0 = MVector2d (linePointA, m_origin);
    double h0 = vector0.CrossProduct (lineDirection);
    dhds = m_direction.CrossProduct (lineDirection);
    // h = h0 + s * dh
    auto ff = MDistance::SafeDivideDistance (-h0, dhds);
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
double ProjectionFraction (MPoint2d const &point) const
    {
    return m_direction.ProjectionFraction (MVector2d (m_origin, point));
    }

//! return the fraction of projection to the perpendicular ray
double PerpendicularProjectionFraction (MPoint2d const &point) const
    {
    return m_direction.PerpendicularProjectionFraction (MVector2d (m_origin, point));
    }
};


//! methods treating an array of MPoint2d as counterclockwise order around a convex polygon
struct MConvexPolygon2d
{
private:
// hull points in CCW order, WITHOUT final duplicate . ..
std::vector<MPoint2d> m_hullPoints;
public: 
// Create the hull.
MConvexPolygon2d (std::vector<MPoint2d> &points)
    {
    ComputeConvexHull(points, m_hullPoints);
    }

// Create the hull.
// First try to use the points as given.  Return isValid = true if that succeeded.
MConvexPolygon2d(std::vector<MPoint2d> &points, bool &isValid)
    {
    m_hullPoints = points;
    isValid = IsValidConvexHull ();
    if (!isValid)
        ComputeConvexHull(points, m_hullPoints);
    }


// Return a copy of the hull points.
std::vector<MPoint2d> Points () {return m_hullPoints;}

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
        if (m_hullPoints[i].CrossProductToTargets (m_hullPoints[i1], m_hullPoints[i2]) < 0.0)
            return false;
        }
    return true;
    }

//! @return true if the convex hull (to the left of edges) contains the test point.
bool ContainsPoint
(
MPoint2d const &point           //!< [in] point to test.
) const
    {
    MPoint2d xy0 = m_hullPoints.back ();
    //double tol = -1.0e-20;  // negative tol!!
    for (size_t i = 0; i < m_hullPoints.size (); i++)
        {
        MPoint2d xy1 = m_hullPoints[i];
        double c = xy0.CrossProductToTargets(xy1, point);
        if (c < 0.0)
            return false;
        xy0 = xy1;
        }
    return true;
    }

// Return the largest outside.  (return 0 if in or on)
double DistanceOutside (MPoint2d &xy)
    {
    double maxDistance = 0.0;
    MPoint2d xy0 = m_hullPoints.back();
    //double tol = -1.0e-20;  // negative tol!!
    for (size_t i = 0; i < m_hullPoints.size(); i++)
        {
        MPoint2d xy1 = m_hullPoints[i];
        double c = xy0.CrossProductToTargets(xy1, xy);
        if (c < 0.0)
            {
            MRay2d ray (xy0, xy1);
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
        MVector2d edgeA, perpA, edgeB, perpB;
        edgeA = MVector2d (m_hullPoints.back (), m_hullPoints.front ());
        edgeA.NormalizeInPlace ();
        perpA = edgeA.CWPerpendicularVector();
        MPoint2d hullPoint0 = m_hullPoints[0];
        for (size_t i = 0; i < m_hullPoints.size (); i++, perpA = perpB)
            {
            size_t j = i + 1;
            edgeB = MVector2d(m_hullPoints[i],
                    j < n ? m_hullPoints[j] : hullPoint0);
            edgeB.NormalizeInPlace();
            perpB = edgeB.CWPerpendicularVector();
            m_hullPoints[i] = m_hullPoints[i] + MVector2d::OffsetBisector (perpA, perpB, distance);
            }
        }
    }

// Return 2 distances bounding the intersection of the ray with a convex hull.
// ASSUME (for tolerancing) the ray has normalized direction vector.
MRange1d ClipRay
(
MRay2d const &ray             //!< [in] ray to clip.  Both negative and positive distances along the ray are possible.
)
    {
    double distanceA = -DBL_MAX;
    double distanceB = DBL_MAX;
    
    if (m_hullPoints.size () < 3)
        return MRange1d ();
    MPoint2d xy0 = m_hullPoints.back ();
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
                return MRange1d ();
            }
        else
            {
            // ray is parallel to the edge.
            // Any single point out classifies it all . ..
            if (xy0.CrossProductToTargets (xy1, ray.m_origin) < 0.0)
                return MRange1d ();
            }
        xy0 = xy1;
        }
    MRange1d range;
    range.Extend (distanceA);
    range.Extend (distanceB);
    return range;
    }

//! Return the range of (fractional) ray postions for projections of all points from the arrays.
MRange1d RangeAlongRay
(
MRay2d const &ray
)
    {
    MRange1d range;
    for (auto xy1 : m_hullPoints)
        range.Extend (ray.ProjectionFraction (xy1));
    return range;
    }

//! Return the range of (fractional) ray postions for projections of all points from the arrays.
MRange1d RangePerpendicularToRay
(
MRay2d const &ray
)
    {
    MRange1d range;
    for (auto xy1 : m_hullPoints)
        range.Extend (ray.PerpendicularProjectionFraction (xy1));
    return range;
    }


static bool ComputeConvexHull(std::vector<MPoint2d> const &xy, std::vector<MPoint2d> &hull)
    {
    hull.clear();
    size_t n = xy.size();
    if (n < 3)
        return false;
    std::vector<MPoint2d> xy1 = xy;
    std::sort(xy1.begin(), xy1.end(), MPoint2d::LexicalXYLessThan);
    hull.push_back(xy1[0]);    // This is sure to stay
    hull.push_back(xy1[1]);    // This one can be removed in the loop.
                               // first sweep creates upper hull . .. 

    for (size_t i = 2; i < n; i++)
        {
        MPoint2d candidate = xy1[i];
        size_t top = hull.size() - 1;
        while (top > 0 && hull[top - 1].CrossProductToTargets(hull[top], candidate) <= 0.0)
            {
            top--;
            hull.pop_back();
            }
        hull.push_back(candidate);
        }

    // second creates lower hull right to left
    size_t i0 = hull.size() - 1;
    // xy1.back () is already on stack.
    hull.push_back(xy1[n - 2]);
    for (size_t i = n - 2; i-- > 0;)
        {
        MPoint2d candidate = xy1[i];
        size_t top = hull.size() - 1;
        while (top > i0 && hull[top - 1].CrossProductToTargets(hull[top], candidate) <= 0.0)
            {
            top--;
            hull.pop_back();
            }
        if (i > 0)  // don't replicate start point !!!
            hull.push_back(candidate);
        }
    return true;
    }
};

//! A strongly typed angle.
//!
struct MAngle
{
private:
    double m_radians;
    MAngle (double radians) : m_radians (radians) {}        // private -- callers do not "know" whether single constructor input is in degrees or radians
    friend MAngle operator *(MAngle const &, double);
    friend MAngle operator *(double, MAngle const &);
    friend MAngle operator -(MAngle const &);

public:
    static double RadiansToDegrees (double radians){return 180.0 * radians / M_PI;}
    static double DegreesToRadians (double degrees){return M_PI * degrees / 180.0;}
    MAngle () : m_radians (0.0) {}
    static MAngle FromDegrees (double degrees)
        {
        return MAngle (degrees * M_PI / 180.0);
        }
    static MAngle FromRadians (double radians)
        {
        return MAngle (radians);
        }
    static MAngle SignedTurn (MVector2d const &vectorA, MVector2d const &vectorB)
        {
        return FromRadians (atan2 (vectorA.CrossProduct (vectorB), vectorA.DotProduct (vectorB)));
        }

    double Radians () const{return m_radians;}
    double Degrees ()const {return RadiansToDegrees (m_radians);}
    double Tan () const {return tan(m_radians);}
    double Cos () const {return cos(m_radians);}
    double Sin () const {return sin(m_radians);}
};

MVector2d MVector2d::Rotate (MAngle const &theta) const
    {
    double c = theta.Cos ();
    double s = theta.Sin ();
    return MVector2d
        (
        m_x * c - m_y * s,
        m_x * s + m_y * c
        );
    }


MVector2d::MVector2d(MPoint2d const &start, MPoint2d const &end)
    {
    m_x = end.m_x - start.m_x;
    m_y = end.m_y - start.m_y;
    }

MVector2d MVector2d::OffsetBisector
(
    MVector2d const &unitPerpA,
    MVector2d const &unitPerpB,
    double offset
    )
    {
    MVector2d bisector = unitPerpA + unitPerpB;
    bisector.NormalizeInPlace();
    double c = offset * bisector.DotProduct(unitPerpA);
    return c * bisector;
    }

MAngle operator *(MAngle const &theta, double factor){return MAngle (theta.m_radians * factor);}
MAngle operator *(double factor, MAngle const &theta){return MAngle (theta.m_radians * factor);}
MAngle operator -(MAngle const &theta) { return MAngle(-theta.m_radians); }


//***************************************************************************************************
//***************************************************************************************************
//***************************************************************************************************
//***************************************************************************************************



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
MPoint2d m_xyA;
MPoint2d m_xyB;

MPathPoint2d ()
    : m_type (Type::Unknown), m_xyA(0,0), m_xyB (0,0)
    {}
MPathPoint2d (Type type, MPoint2d const &xyA, MPoint2d const &xyB)
    : m_type (type), m_xyA (xyA), m_xyB (xyB)
    {
    }
};
//! Virtual method interface structure for algorithms to announce MPathPoint2d.
struct MAnnouncePathPoint
{
virtual void Announce (MPathPoint2d const &data) = 0;
};
//! Implement MAnnouncePathPoint -- collect the path points in an std::vector.
struct MPathPoint2dCollector : MAnnouncePathPoint
{
//! Publicly accessible collected points ...
std::vector<MPathPoint2d> path;

//! Implement the Announce method to capture points ...
void Announce(MPathPoint2d const &data) override
    {
    path.push_back(data);
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
MAngle m_exteriorSwingStep;
MAngle m_interiorSwingStep;
double m_offsetDistance;
MAnnouncePathPoint &m_announcer;

void Announce (MPathPoint2d::Type type, MPoint2d const &xy, MPoint2d const &target)
    {
    m_announcer.Announce (MPathPoint2d (type, xy, target));
    }
// ALWAYS add start.
// add intermediates as needed by linearStep.
// ALWAYS add end.
void AddStepsOnLine
(
MPoint2d const &xyA,
MPoint2d const &xyB,
MVector2d const &offsetVector,
MPathPoint2d::Type typeA,
MPathPoint2d::Type type,
MPathPoint2d::Type typeB
)
    {
    Announce (typeA, xyA + offsetVector, xyA);
    double distance = xyA.Distance (xyB);
    if (m_linearStep > 0.0 && distance > m_linearStep)
        {
        double numStep = ceil (fabs(distance) / m_linearStep);    // just leave as double ..
        double df = 1.0 / numStep;
        for (int i = 1; i < numStep; i++)
            {
            MPoint2d xy = xyA.Interpolate (i * df, xyB);
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
MPoint2d const &xyTarget,        // center of rotation
MVector2d const &offsetVector,
MAngle const &totalTurnAngle
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
            MAngle turn = totalTurnAngle * (i * df);
            MVector2d radialVector = offsetVector.Rotate (turn);
            Announce (MPathPoint2d::Type::ExteriorSwingIntermediate, xyTarget + radialVector, xyTarget);
            }
        }
    }

// NEVER add initial point
// add intermediates as necessary
// NEVER add last
void AddInteriorSwing
(
MPoint2d const &xyTargetA,        // first target
MVector2d const &offsetVector,  // first target to center
MAngle const &totalTurnAngle    // (Expect negative)
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
            MAngle turn = totalTurnAngle * (i * df);
            MVector2d radialVector = offsetVector.Rotate (turn);
            Announce (MPathPoint2d::Type::InteriorSwingIntermediate, center, center - radialVector);
            }
        }
    }

public:
    //! Construct an MOffsetPathBuilder with default parameters.
    MOffsetPathBuilder(MAnnouncePathPoint &announcer)
        : m_linearStep(0.0),
        m_exteriorSwingStep(MAngle()),
        m_interiorSwingStep(),
        m_offsetDistance(0.0),
        m_announcer(announcer)
        {
        }
    //! Set the offset distance.  Positive is to the right of the path, negative is to left.
    void SetOffsetDistance(double data) { m_offsetDistance = data; }
    //! Set the maximum distance between offset points when moving along a straight edge.
    void SetLinearStep(double data) { m_linearStep = data; }
    //! Set the maximum angular swing when rotating around an outside corner.
    void SetExteriorSwingStep(MAngle data) { m_exteriorSwingStep = data; }
    //! Set the maximum angular swing when swinging (in place) to view an inside corner.
    void SetInteriorSwingStep(MAngle data) { m_interiorSwingStep = data; }

//! build a path offset from a sequence of target edges.
//! At each path points
//!<ul>
//!<li>xyA is the path coordinate
//!<li>xyB is the target (viewed) point
//!<li>type indicates what motion the path is following.
//!</ul>
void BuildAlongPolyline (std::vector<MPoint2d> const &targetXY)
    {
    if (targetXY.size () < 2)
        return;
    MPathPoint2d::Type typeA = MPathPoint2d::Type::Initial;
    MPoint2d xyA = targetXY.front ();
    size_t targetIndexB = 1;
    MAngle angleA;    // initialized to 0.
    double offsetSign = m_offsetDistance >= 0.0 ? 1.0 : -1.0;
    for (; targetIndexB + 1 < targetXY.size (); targetIndexB++)
        {
        size_t targetIndexC = targetIndexB + 1;
        MPoint2d xyB = targetXY[targetIndexB];
        MPoint2d xyC = targetXY[targetIndexC];
        MVector2d deltaAB (xyA, xyB);
        MVector2d deltaBC (xyB, xyC);
        MVector2d unitAB = deltaAB;
        MVector2d unitBC = deltaBC;
        if (unitAB.NormalizeInPlace ()
            && unitBC.NormalizeInPlace ()
            )
            {
            MAngle angleB = MAngle::SignedTurn (deltaAB, deltaBC);
            auto offsetVector = m_offsetDistance * unitAB.CWPerpendicularVector ();
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
                MAngle halfAngle = 0.5 * angleB;
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
                auto offsetVectorC = m_offsetDistance * unitBC.CWPerpendicularVector ();
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

// CameraMath has static methods for various camera calculations.
struct CameraMath
{
// Given: distanceToWall, cameraX, cameraSwing, cameraHalfAngle
// Return: wallX
static double ComputeWallX (double distanceToWall, double cameraX, MAngle cameraSwingFromPerpendicular, MAngle cameraAngleFromMidline)
    {
    return cameraX + distanceToWall * tan (cameraSwingFromPerpendicular.Radians() + cameraAngleFromMidline.Radians ());
    }
// Given: distanceToWall, wallX, cameraSwing, cameraHalfAngle
// Return: cameraX
static double ComputeCameraX (double distanceToWall, MAngle cameraSwingFromPerpendicular, MAngle cameraAngleFromMidline, double wallX)
    {
    return wallX - distanceToWall * tan(cameraSwingFromPerpendicular.Radians() + cameraAngleFromMidline.Radians());
    }

// Given: distanceToWall, cameraX, wallX, cameraHalfAngle
// Return: cameraSwing
static MAngle ComputeCameraSwingFromPerpendicular (double distanceToWall, double cameraX, MAngle cameraAngleFromMidline, double wallX)
    {
    double dx = wallX - cameraX;
    double betaRadians = atan2 (dx, distanceToWall);
    return MAngle::FromRadians (betaRadians - cameraAngleFromMidline.Radians ());
    }

// Given: distanceToWall, cameraHalfAngle, overlapFraction
// Return: cameraSwingAngle, cameraStep
static void SetupCameraSwingAndStep
(
double distanceToWall,  // Distance from camera to wall for "head on" (middle) of 3 shots from each camera position.
MAngle cameraHalfAngle, // angle from camera midline to edge
double overlapFraction,  // fractional overlap between consecutive images
MAngle &cameraSwingAngle,           // Angle between left, middle, and right shots at fixed camera position
double &cameraStep      // camera motion along wall between steps.
)
    {

    // Interval0 = (x0Left .. x0Right) = camera pointing directly at (0,0) from above
    double x0Left = CameraMath::ComputeWallX(distanceToWall, 0.0, MAngle::FromDegrees(0.0), -cameraHalfAngle);
    double x0Right = CameraMath::ComputeWallX(distanceToWall, 0.0, MAngle::FromDegrees(0.0), cameraHalfAngle);
    // Interval1 = (x1Left .. x1Right) = camera still above (0,0) but rotated "forward".
    //  this intervals x1Left is required to be fractional position "overlapFraction" from x0Right back towards x0Left
    double x1Left = MDistance::Interpolate(x0Right, overlapFraction, x0Left);
    // Find camera swing to make the back edge of the forward swing image
    //   have the right overlap with the headon image
    cameraSwingAngle = CameraMath::ComputeCameraSwingFromPerpendicular(distanceToWall, 0.0, -cameraHalfAngle, x1Left);


    // alpha and cameraSwingAngle now stay fixed.

    // Compute the other end of the Interval1 ..
    double x1Right = CameraMath::ComputeWallX(distanceToWall, 0.0, cameraSwingAngle, cameraHalfAngle);
    // Interval2 = (x2Left..x2Right) as seen when camera looks backward after first move along the wall.
    // x2Left is set within Interval1 to get overlap
    double x2Left = MDistance::Interpolate(x1Right, overlapFraction, x1Left);
    // make back end of field of view hit x2Left ...
    cameraStep = CameraMath::ComputeCameraX(distanceToWall, -cameraSwingAngle, -cameraHalfAngle, x2Left);
    }

};