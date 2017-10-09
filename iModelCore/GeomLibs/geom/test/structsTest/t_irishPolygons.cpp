#include "testHarness.h"

static bool s_printAll = false;


// Tagged point: DPoint3d + ptrdiff_t index + double + ptrdiff_t priority
struct DPoint3dIDI
{
DPoint3d m_xyz;
ptrdiff_t m_index;
double m_a;
ptrdiff_t m_priority;

DPoint3dIDI (DPoint3dCR xyz, ptrdiff_t index = 0, double a = 0.0, ptrdiff_t priority = 0)
    {
    m_xyz = xyz;
    m_index = index;
    m_a = a;
    m_priority = priority;
    }

// Lexical sort by (index1 then a)
bool operator < (DPoint3dIDI const &other) const
    {
    // unequal index is definitive ...
    if (m_index < other.m_index)
        return true;
    if (other.m_index < m_index)
        return false;
    // equal index, compare by fraction
    if (m_a <= other.m_a)
        return true;
    return false;        
    }
};

// Vector of DPoint3dIDI
struct DPoint3dIDVector : bvector<DPoint3dIDI>
{
void Extract (bvector<DPoint3d> &dest)
    {
    dest.clear ();
    for (size_t i = 0; i < size (); i++)
        dest.push_back (at(i).m_xyz);
    }

void AddIndexed (bvector<DPoint3d> const &source, ptrdiff_t priority)
    {
    for (size_t i = 0; i < source.size (); i++)
        {
        push_back (DPoint3dIDI (source[i], i, 0.0, priority));
        }
    }

void Sort ()
    {
    std::sort (begin (), end ());
    }

// Among contiguous points starting at startIndex in the array and within toleranceXY of the first,
// return selecteeIndex = index of point with lowest (0,1,2) mask index.
// (If multiple with same index, one is chosen arbitrarily)
bool TryDelimitCluster (double toleranceXY, size_t startIndex, size_t &selecteeIndex, size_t &nextIndex)
    {    
    size_t n = size ();
    nextIndex = startIndex;
    selecteeIndex = startIndex;
    if (startIndex >= size ())
        return false;
    DPoint3d point0 = at(startIndex).m_xyz;
    nextIndex = startIndex + 1;
    while (nextIndex < n && point0.Distance (at(nextIndex).m_xyz) <= toleranceXY)
        {
        if (at(nextIndex).m_priority < at(selecteeIndex).m_priority)
            selecteeIndex = nextIndex;
        nextIndex++;
        }
    return true;
    }

void ExtractPrioritizedClusters (bvector<DPoint3d> &dest, double toleranceXY)
    {
    size_t i0 = 0;
    size_t i1, i2;
    dest.clear ();
    for (i0 = 0;TryDelimitCluster (toleranceXY, i0, i1, i2); i0 = i2)
        dest.push_back (at(i1).m_xyz);
    }
};

// If xyzIn is within toleranceXY of a any vertex of the polyline, move there.
// return true if moved.
void ClosestVertexXY (bvector<DPoint3d> const &xyzA, DPoint3dCR pointB, ptrdiff_t &indexA, double &distanceA)
    {
    indexA = -1;
    distanceA = DBL_MAX;
    // Find the closest vertex ...
    for (size_t i = 0; i < xyzA.size (); i++)
        {
        double d = pointB.DistanceXY (xyzA[i]);
        if (d < distanceA)
            {
            indexA = i;
            distanceA = d;
            }
        }
    }
// given spacePoint, try to get a true interior projection in xyzA.
bool BoundedSegmentProjectionXY (bvector<DPoint3d> const &xyzA, DPoint3dCR spacePoint, DPoint3dR pointA, ptrdiff_t &indexA, double &fractionA, double &distanceA)
    {
    indexA = -1;
    fractionA = 0.0;
    distanceA = DBL_MAX;
    double d;
    pointA = spacePoint;
    DSegment3d segmentA;
    // Find the closest vertex ...
    for (size_t i = 0; i + 1 < xyzA.size (); i++)
        {
        segmentA.Init (xyzA[i], xyzA[i+1]);
        DPoint3d pointAi;
        double fractionAi;
        segmentA.ProjectPointXY (pointAi, fractionAi, spacePoint);
        d = pointAi.DistanceXY(spacePoint);
        if (DoubleOps::IsIn01 (fractionAi) && d < distanceA)
            {
            indexA = i;
            distanceA = d;
            fractionA = fractionAi;
            pointA = pointAi;
            }
        }
    return indexA > -1;
    }


static double s_relTol = 1.0e-12;
struct PolygonMergeContext
{
static const int markup_intersection = 2;
static const int markup_vertex = 0;
static const int markup_alongEdge = 1;


bvector<DPoint3d> const &m_xyzA;
bvector<DPoint3d> const &m_xyzB;

DPoint3dIDVector m_sortA;
DPoint3dIDVector m_sortB;

bvector<DPoint3d> m_xyzB1; // scratch array for intermediate B values.
double m_toleranceXY;


PolygonMergeContext (bvector<DPoint3d> const &xyzA, bvector<DPoint3d> const &xyzB, double toleranceXY)
    : m_xyzA(xyzA), m_xyzB(xyzB)
    {
    DRange3d dualRange;
    dualRange.Init ();
    dualRange.Extend (&xyzA[0], (int)xyzA.size ());
    dualRange.Extend (&xyzB[0], (int)xyzB.size ());
    m_toleranceXY = DoubleOps::Max (toleranceXY, s_relTol * dualRange.low.Distance (dualRange.high));
    }

bool AlmostEqualXY (DPoint3d const &pointA, DPoint3d const &pointB)
    {
    return pointA.DistanceXY (pointB) <= m_toleranceXY;
    }
    
void LoadVertexBSnaps ()
    {
    // Build up B1 as all the vertices of B snapped to nearest vertex or edge interior of A        
    // For edge interiors, also add to m_sortA.
    DPoint3d pointA;
    double distanceA;
    double fractionA;
    ptrdiff_t indexA;
    for (size_t j = 0; j < m_xyzB.size (); j++)
        {
        DPoint3d xyzB = m_xyzB[j];
        // Vertex snap goes first ..
        ClosestVertexXY (m_xyzA, xyzB, indexA, distanceA);
        if (distanceA <= m_toleranceXY)
            {
            // this B point moves to its A anchor ...
            xyzB = m_xyzA[indexA];
            }
        else
            {
            BoundedSegmentProjectionXY (m_xyzA, xyzB, pointA, indexA, fractionA, distanceA);
            if (distanceA <= m_toleranceXY)
                {
                // B polygon vertex j is adjusted to the point on the edge of A ...
                xyzB = pointA;
                // polygonA gets additional mid-edge vertex.  (If close to a vertex, it would have been caught as vertex hit)
                m_sortA.push_back (DPoint3dIDI (pointA, indexA, fractionA, markup_alongEdge));
                }
            }
        m_xyzB1.push_back (xyzB);
        }
    }        

// Find edgeB1 interior points that are close to a vertex of A.  snap them to the A vertex.
// resort and rebuild m_xyzB1.
void LoadVertexAEdgeB1Snaps ()
    {
    // If a vertex of A is close to a B segment, insert it as a break in B.
    // (with xyz from A but sort fraction from B?)
    // (Note that this uses the "new" coordinates from B)
    DSegment3d segmentB;
    m_sortB.clear ();
    m_sortB.AddIndexed (m_xyzB1, markup_vertex);
    DPoint3d pointB;
    double fractionB;
    for (size_t j = 0; j + 1 < m_xyzB.size (); j++)
        {
        segmentB.Init (m_xyzB1[j], m_xyzB1[j+1]);
        for (size_t i = 0; i < m_xyzA.size (); i++)
            {
            segmentB.ProjectPointXY (pointB, fractionB, m_xyzA[i]);
            if (    DoubleOps::IsIn01 (fractionB)
                &&  AlmostEqualXY (m_xyzA[i], pointB)
                && !AlmostEqualXY (pointB, segmentB.point[0])
                && !AlmostEqualXY (pointB, segmentB.point[1])
                )
                {
                m_sortB.push_back (DPoint3dIDI (m_xyzA[i], j, fractionB, markup_alongEdge));
                }
            }
        }
    m_sortB.Sort ();
    m_sortB.Extract (m_xyzB1);
    }

void LoadIntersections_A_B1 ()
    {
    DSegment3d segmentA, segmentB;
    double fractionA, fractionB;
    DPoint3d pointA, pointB;
    for (size_t j = 0; j + 1 < m_xyzB.size (); j++)
        {
        segmentB.Init (m_xyzB1[j], m_xyzB1[j+1]);
        for (size_t i = 0; i + 1 < m_xyzA.size (); i++)
            {
            segmentA.Init (m_xyzA[i], m_xyzA[i+1]);
            if (DSegment3d::IntersectXY (
                        fractionA, fractionB, pointA, pointB,
                        segmentA, segmentB))
                 {
                 // simple intersection.  Ignore if out of segment extent ...
                 if (DoubleOps::IsIn01 (fractionA) && DoubleOps::IsIn01 (fractionB))
                    {
                    // coordinates from polygon A takes priority ...
                    m_sortA.push_back (DPoint3dIDI (pointA, i, fractionA, markup_intersection));
                    m_sortB.push_back (DPoint3dIDI (pointA, j, fractionB, markup_intersection));
                    }
                }
            }
        }
    }

void Go (bvector<DPoint3d> &xyzA1, bvector<DPoint3d> &xyzA2)
    {
    m_sortA.clear ();
    m_sortB.clear ();
    m_xyzB1.clear ();
    m_sortA.AddIndexed (m_xyzA, markup_vertex);
    LoadVertexBSnaps ();
    LoadVertexAEdgeB1Snaps ();
    LoadIntersections_A_B1 ();
    m_sortA.Sort ();
    m_sortA.ExtractPrioritizedClusters (xyzA1, m_toleranceXY);
    m_sortB.Sort ();    
    m_sortB.ExtractPrioritizedClusters (xyzA2, m_toleranceXY);
    }

};


// When coordinates in polylines A and B have matching xy:
//   1) Add vertex as needed if common point is within either edge.
//   2) Make z of xyzB match z from xyzA.
//
// Remark: If these are from polygons, the start point must be repeated at the end.
void MovePolylineZ (
    bvector<DPoint3d> const &xyzA0,
    bvector<DPoint3d> const &xyzB0,
    bvector<DPoint3d> &xyzA1,
    bvector<DPoint3d> &xyzB1,
    double toleranceXY
    )
    {
    if (xyzA0.size () == 0 || xyzB0.size () == 0)
        {
        xyzA1 = xyzA0;
        xyzB1 = xyzB0;
        }
    else
        {
        PolygonMergeContext context (xyzA0, xyzB0, toleranceXY);
        context.Go (xyzA1, xyzB1);
        }
    }

void Add (bvector<DPoint3d> &dest, double x, double y, double z)
    {
    DPoint3d xyz;
    xyz.x = x;
    xyz.y = y;
    xyz.z = z;
    dest.push_back (xyz);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(IrishPolygons, SimpleIntersection)
    {
    bvector<DPoint3d> pointA, pointB, pointA1, pointB1;
    Add (pointA, 0,1,0);
    Add (pointA, 5,1,0);
    
    Add (pointB, 2,0,1);
    Add (pointB, 2,4,1);
    double tolerance = 1.0e-8;
    MovePolylineZ (pointA, pointB, pointA1, pointB1, tolerance);
    Check::Size (pointA.size () + 1, pointA1.size (), "intersection point added");
    Check::Size (pointB.size () + 1, pointB1.size (), "intersection point added");

    }
    
bool AlmostEqual (bvector<DPoint3d> const &pointA, bvector<DPoint3d> const &pointB, double dxy, double dz)
    {
    if (pointA.size () != pointB.size ())
        return false;
    for (size_t i = 0; i < pointA.size (); i++)
        {
        if (pointA[i].DistanceXY (pointB[i]) > dxy)
            return false;
        if (fabs (pointA[i].z - pointB[i].z) < dz)
            return false;
        }
    return true;
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(IrishPolygons, MoveVertexToVertex)
    {
    bvector<DVec3d> vectors;
    vectors.push_back (DVec3d::From (1,0,-0.5));
    vectors.push_back (DVec3d::From (1,1,-0.5));
    vectors.push_back (DVec3d::From (-1,1,-0.5));
    vectors.push_back (DVec3d::From (-1,-1,-0.5));


    bvector<DPoint3d> pointA, pointB0, pointA1, pointB1;
    Add (pointA, 0,1,1);
    Add (pointA, 2,1,1);
    Add (pointA, 5,1,1);
    
    Add (pointB0, 2,10,2);
    Add (pointB0, 2,8,2);
    double tolerance = 0.1;
    double factor = 1.0e-3; // each vector becomes plainly less than tolerance when scaled by this.
    for (size_t i = 0; i < pointA.size (); i++)
        {
        // Make B polyline end near this A vertex.  Move should just move that point.
        for (size_t j = 0; j < vectors.size (); j++)
            {
            bvector<DPoint3d> pointB = pointB0;
            pointB.push_back (DPoint3d::FromSumOf (pointA[i], vectors[j], factor));
            MovePolylineZ (pointA, pointB, pointA1, pointB1, tolerance);
            Check::Size (pointA.size (), pointA1.size (), "only move");
            Check::Size (pointB.size (), pointB1.size (), "only move");
            Check::Near (pointA[i], pointB1.back (), "this point moves");
            Check::True (AlmostEqual (pointA, pointA1, 0.0, 0.0), "A unchanged");
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(IrishPolygons, MoveVertexToEdge)
    {
    bvector<DVec3d> vectors;
    vectors.push_back (DVec3d::From (1,0,-0.5));
    vectors.push_back (DVec3d::From (1,1,-0.5));
    vectors.push_back (DVec3d::From (-1,1,-0.5));
    vectors.push_back (DVec3d::From (-1,-1,-0.5));

    bvector<DPoint3d> pointA, pointB0, pointA1, pointB1;
    Add (pointA, 0,1,1);
    Add (pointA, 2,1,1);
    Add (pointA, 5,1,1);
    
    Add (pointB0, 1,10,2);
    Add (pointB0, 3,10,2);
    Add (pointB0, 5,10,2);
    
    double tolerance = 0.1;
    double factor = 1.0e-3; // each vector becomes plainly less than tolerance when scaled by this.

    // Put points of B "very close" to edge interior points of A.
    for (size_t j = 0;  j < pointB0.size (); j++)
        {
        for (size_t i = 0; i + 1 < pointA.size (); i++)
            {
            // Make B polyline end near an A edge.  Move should just move that point.
            double interpolationFraction = 0.4;
            for (size_t k = 0; k + 1 < vectors.size (); k++)
                {
                DPoint3d edgePoint = DPoint3d::FromInterpolate (pointA[i], interpolationFraction, pointA[i+1]);
                bvector<DPoint3d> pointB = pointB0;
                pointB[j] = DPoint3d::FromSumOf (edgePoint, vectors[k], factor);
                // Edge of A pulls vertex of B ....
                MovePolylineZ (pointA, pointB, pointA1, pointB1, tolerance);
                Check::Size (pointA.size () + 1, pointA1.size (), "new edge point");
                Check::Size (pointB.size (), pointB1.size (), "B vertex moved to A edge");
                Check::Near (pointA1[i+1], pointB1[j], "point added to A");
                Check::True (pointB[j].Distance (edgePoint) < tolerance, "edge projection near original");
                
                // Vertex of B pulls edge of A
                bvector<DPoint3d> pointA2, pointB2;
                MovePolylineZ (pointB, pointA, pointB2, pointA2, tolerance);

                Check::Size (pointA.size () + 1, pointA1.size (), "new edge point");
                Check::Size (pointB.size (), pointB1.size (), "B vertex moved to A edge");
                Check::Near (pointA2[i+1], pointB[j], "secondary edge split moves to primary vertex");
                Check::True (pointB[j].Distance (edgePoint) < tolerance, "edge projection near original");

                }
            }
        }
    }    


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PrincipalAxes,Zero)
    {
    bvector<DPoint3d>points;
    DVec3d centroid, moments;
    RotMatrix axes;
    Check::False (DPoint3dOps::PrincipalAxes (points, centroid, axes, moments), "Empty points principal axes expected to fail");
    
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PrincipalAxes,XAXis)
    {
    Check::StartScope ("XAxisPointPair");
    bvector<DPoint3d>points;
    double a = 1.0;
    double b = 3.0;
    points.push_back (DPoint3d::From (a,0,0));
    points.push_back (DPoint3d::From (b,0,0));
    DVec3d centroid, moments;
    RotMatrix axes;
    DPoint3dOps::PrincipalAxes (points, centroid, axes, moments);
    Check::Near (DPoint3d::From (0.5 * (a+b), 0,0), centroid, "centroid");
    double u = 0.5 * (b-a);
    double M = u*u*2.0;
    Check::Near (DVec3d::From (M, M, 0), moments, "moments");
    Check::EndScope ();
    }        


// momentB[i] => momentA[indexBinA[i]] for smallest difference.
// return largest error in matching
double MatchMoments (DVec3dCR momentA, RotMatrixCR axesA, DVec3dCR momentB, RotMatrixCR axesB, size_t indexBinA[3], DVec3dR momentB1, RotMatrixR axesB1)
    {
    double a[3] = {momentA.x, momentA.y, momentA.z};
    double b[3] = {momentB.x, momentB.y, momentB.z};
    double b1[3];
    DVec3d BB0[3], BB1[3];
    DVec3d AA[3];
    axesB.GetColumns (BB0[0], BB0[1], BB0[2]);
    axesA.GetColumns (AA[0], AA[1], AA[2]);
    double dMax = 0.0;
    for (size_t i = 0; i < 3; i++)
        {
        double dMin = fabs (b[i] - a[0]);
        size_t jMin = 0;
        for (size_t j = 1; j < 3; j++)
            {
            double d = fabs (b[i] - a[j]);
            if (d < dMin)
                {
                jMin = j;
                dMin = d;
                }
            }
        dMax = DoubleOps::Max (dMin, dMax);
        BB1[jMin] = BB0[i];
        if (BB1[jMin].DotProduct(AA[jMin]) < 0.0)
            BB1[jMin].Negate ();
        b1[jMin] = b[i];
        indexBinA[i] = jMin;
        }
    momentB1.Init (b1[0], b1[1], b1[2]);
    axesB1.InitFromColumnVectors (BB1[0], BB1[1], BB1[2]);
    return dMax;    
    }
void CheckTransformedMoments
(
TransformCR frame,              // must be orthogonal !!!
bvector<DPoint3d> const &pointsA,
double shiftMagnitude         // 
)
    {
    DVec3d centroidA, centroidB, centroidC;
    DVec3d   momentA, momentB, momentB1;
    RotMatrix axesA, axesB, axesC, axesB1;
    size_t indexBinA [3];
    bvector <DPoint3d> pointsB;
    frame.Multiply (pointsB, pointsA);
    DPoint3dOps::PrincipalAxes (pointsA, centroidA, axesA, momentA);
    // centroid and axes should go with the transfrom.  moments should be unchanged.
    axesC.InitProduct (frame, axesA);
    frame.Multiply (centroidC, centroidA);
    DPoint3dOps::PrincipalAxes (pointsB, centroidB, axesB, momentB);
    double momentRef = centroidB.Magnitude () + momentB.Magnitude ();
    double momentDiff = MatchMoments (momentA, axesC, momentB, axesB, indexBinA, momentB1, axesB1);
    if (s_printAll)
        printf (" (centroidMagnitude %g) (momentDiff %g)\n", centroidB.Magnitude (), momentDiff); 
    Check::Near (1.0, axesB.Determinant (), "det A");
    Check::Near (1.0, axesB1.Determinant (), "det B");
    Check::Near (1.0, axesA.Determinant (), "det B1");
    Check::Near (1.0, axesC.Determinant (), "det C");

    Check::Near (fabs (momentRef), fabs (momentRef) + momentDiff, "momentDiff");
    Check::Near (momentA, momentB1, "Transformed moments");
    Check::Near (centroidC, centroidB, "Transformed centroid");
    Check::Near (axesC, axesB1, "Transformed axes");

    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PrincipalAxes,Slab)
    {
    Check::StartScope ("XAxisPointPair");
    bvector<DPoint3d>points;
    // a>b>c matters to the checks ...
    double a = 7.0;
    double b = 5.0;
    double c = 3.0;
    points.push_back (DPoint3d::From ( a, b, c));
    points.push_back (DPoint3d::From (-a, b, c));
    points.push_back (DPoint3d::From ( a,-b, c));
    points.push_back (DPoint3d::From (-a,-b, c));
    points.push_back (DPoint3d::From ( a, b,-c));
    points.push_back (DPoint3d::From (-a, b,-c));
    points.push_back (DPoint3d::From ( a,-b,-c));
    points.push_back (DPoint3d::From (-a,-b,-c));
    
    DVec3d centroid, moments;
    RotMatrix axes;
    DPoint3dOps::PrincipalAxes (points, centroid, axes, moments);
    Check::Near (DPoint3d::From (0,0,0), centroid, "centroid");
    double AB = 8.0 * (a*a + b*b);
    double BC = 8.0 * (c*c + b*b);
    double AC = 8.0 * (a*a + c*c);
    Check::Near (DVec3d::From (AB, AC, BC), moments, "moments");
    
    DVec3d shiftDirection = DVec3d::From (cos (3.0), sin(1.2), tan (1.2));
    DPoint3d basePoint = DPoint3d::From (1,2,3);
    double f = 1.0;
    static double fScale = 10.0;
    static size_t s_numScale = 8;
    // We see amazing precision *on some platforms* even when the data is scaled 5 orders of magnitude away from the origin. We think that's because we build with
    // "fast" floating point optimizations turned on on all platforms. We think that allows the compiler to avoid rounding intermediate results
    // and to maintain 128-bit precision throughout a series of calculations.
    // In any case, we can't count on this, since it's not guaranteed by the standard.
#ifdef BENTLEY_WIN32 // Some versions of GCC and clang non-optimized build does not appear implement 'unsafe floating point optimizations'; also, exclude for WinRT.
    static size_t const s_tightToleranceIterationLimit = 5;
#else
    static size_t const s_tightToleranceIterationLimit = 2;
#endif

    for (size_t i = 0; i < s_numScale; i++, f *= fScale)
        {
        DRay3d axis = DRay3d::FromOriginAndVector (DPoint3d::FromSumOf (basePoint, shiftDirection, f), DVec3d::From (6,2,-2));
        double theta = 1.0;
        Transform transform = Transform::FromAxisAndRotationAngle (axis, theta);
        if (i > s_tightToleranceIterationLimit)
            Check::PushTolerance (ToleranceSelect_Loose);
        CheckTransformedMoments (transform, points, f);
        if (i > s_tightToleranceIterationLimit)
            Check::PopTolerance ();
        }
    Check::EndScope ();
    }            

void ShowXYZ(char const * name, DPoint3dCR xyz)
    {
    if (s_printAll)
        printf (" (%s    %.17g  %.17g   %.17g)\n", name, xyz.x, xyz.y, xyz.z);
    }
void TestNearIntersction (
    double ax, double ay, double az,
    double bx, double by, double bz,
    double cx, double cy, double cz,
    double ux, double uy, double uz
    )
    {
    DPoint3d A = DPoint3d::From(ax, ay, az);
    DPoint3d B = DPoint3d::From (bx, by, bz);
    DPoint3d C = DPoint3d::From (cx, cy, cz);

    DVec3d U = DVec3d::From (ux, uy, uz);
    DVec3d V = DVec3d::FromStartEnd (A, B);
    if (s_printAll)
      printf ("\n\n    TestNearIntersection\n");
    ShowXYZ("A", A);
    ShowXYZ("B", B);
    ShowXYZ("C", C);

    ShowXYZ("U", U);
    ShowXYZ("V", V);

    if (s_printAll)
        printf ("  Angle Between vectors %.17g\n", U.AngleTo (V));
    DRay3d ray0 = DRay3d::FromOriginAndTarget (A, B);
    DRay3d ray1 = DRay3d::FromOriginAndVector (C, U);
    double s0, s1;
    DPoint3d E0, E1;
    DRay3d::ClosestApproachUnboundedRayUnboundedRay (s0, s1, E0, E1, ray0, ray1);
    if (s_printAll)
        printf (" intersection fractions (s0 %.17g) (s1 %.17g)\n", s0, s1);
    DPoint3d F0 = ray0.FractionParameterToPoint (s0);
    DPoint3d F1 = ray1.FractionParameterToPoint (s1);
    ShowXYZ ("F0", F0);
    ShowXYZ ("E0", E0);
    ShowXYZ ("               E0 to F0", DVec3d::FromStartEnd (E0, F0));
    ShowXYZ ("F1", F1);
    ShowXYZ ("E1", E1);
    ShowXYZ ("               E1 to F1", DVec3d::FromStartEnd (E1, F1));
    DVec3d W = DVec3d::FromStartEnd (F0, F1);
    ShowXYZ ("W", W);
    if (s_printAll)
        {
        printf(" (W dot U %.17g)\n", W.DotProduct (U));
        printf(" (W dot V %.17g)\n", W.DotProduct (V));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DRay3d,NearIntersection)
    {
    TestNearIntersction (
        2138149.06,    222376.83,871,
        2138156.30,    222384.72,870.56,
        2138149.675 ,  222383.815,870.654,
        0.8681,0.4964,0.365
        );

    TestNearIntersction (
        2138149.06,222376.83,871,
        2138156.30,222384.72,870.56,
        2138149.6755310423,222383.81533691793,870.65395908411790,
        0.86809901567175984,0.49639107499388047,0.36504544792137494
        );
    }