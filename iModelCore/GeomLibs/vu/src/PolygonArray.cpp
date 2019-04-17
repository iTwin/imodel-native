/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE



#include <algorithm>
#ifdef NoisyAddPolygon
static int s_noisy = 0;
#endif
// push a (copy of a) polygon onto the polygon list.
void PolygonVectorOps::AddPolygon (TaggedPolygonVectorR dest, bvector <DPoint3d> const &points,
         ptrdiff_t indexA, ptrdiff_t indexB, double a)
    {
#define NoisyAddPolygonNOT
#ifdef NoisyAddPolygon
    if (s_noisy > 0)
        GEOMAPI_PRINTF (" AddPolygon (%d,%d,%d)\n", (int)points.size (), (int)indexA, (int)indexB);
    if (s_noisy > 9)
        {
        for (auto xyz : points)
            printf ("  %.17g %.17g\n", xyz.x, xyz.y);
        }
#endif  
    dest.push_back (TaggedPolygon (bvector<DPoint3d> (), indexA, indexB, a));
    dest.back ().GetPointsR () = points;
    }

// push a (copy of a) polygon onto the polygon list.
void PolygonVectorOps::AddPolygonCapture (TaggedPolygonVectorR dest, bvector <DPoint3d> &points,
         ptrdiff_t indexA, ptrdiff_t indexB, double a)
    {
#define NoisyAddPolygonNOT
#ifdef NoisyAddPolygon
    if (s_noisy > 0)
        GEOMAPI_PRINTF (" AddPolygon (%d,%d,%d)\n", (int)points.size (), (int)indexA, (int)indexB);
    if (s_noisy > 9)
        {
        for (auto xyz : points)
            printf ("  %.17g %.17g\n", xyz.x, xyz.y);
        }
#endif  
    dest.push_back (TaggedPolygon (bvector<DPoint3d> (), indexA, indexB, a));
    dest.back ().GetPointsR ().swap (points);
    }

// push a (copy of a) polygon onto the polygon list.
void PolygonVectorOps::AddTransformedPolygon (TaggedPolygonVectorR dest, bvector <DPoint3d> &points,
         DVec3dCR shift,
         ptrdiff_t indexA, ptrdiff_t indexB, double a)
    {
#define NoisyAddPolygonNOT
#ifdef NoisyAddPolygon
    if (s_noisy > 0)
        GEOMAPI_PRINTF (" AddPolygon (%d,%d,%d)\n", (int)points.size (), (int)indexA, (int)indexB);
    if (s_noisy > 9)
        {
        for (auto xyz : points)
            printf ("  %.17g %.17g\n", xyz.x, xyz.y);
        }
#endif  
    dest.push_back (TaggedPolygon (bvector<DPoint3d> (), indexA, indexB, a));
    for (auto xyz : points)
        dest.back().GetPointsR ().push_back (xyz + shift);
    }



void PolygonVectorOps::AddPolygon (TaggedPolygonVectorR dest, DPoint3dCP points, int n, ptrdiff_t indexA, ptrdiff_t indexB, double a)
    {
#ifdef NoisyAddPolygon
    if (s_noisy > 0)
        GEOMAPI_PRINTF (" AddPolygon (%d,%d,%d)\n", n, (int)indexA, (int)indexB);
#endif
    dest.push_back (TaggedPolygon (indexA, indexB, a));
    bvector<DPoint3d> &polygon = dest.back ().GetPointsR ();
    polygon.reserve (n);
    for (int i = 0; i < n; i++)
        polygon.push_back (points[i]);
    }

double PolygonVectorOps::GetSummedAreaXY (TaggedPolygonVectorCR source)
    {
    DVec3d zVector;
    zVector.Init (0,0,1);
    double sum = 0.0;
    for (size_t i = 0,n = source.size (); i < n; i++)
        sum += source[i].Area (zVector);
    return sum;
    }
DRange3d PolygonVectorOps::GetRange (TaggedPolygonVectorCR source)
    {
    DRange3d range;
    range.Init ();
    size_t n = source.size ();
    for (size_t i = 0; i < n; i++)
        {
        bvector<DPoint3d> const &polygon = source[i].GetPointsCR ();
        range.Extend (&polygon[0], (int)polygon.size ());
        }
    return range;
    }

size_t PolygonVectorOps::GetTotalPointCount (TaggedPolygonVectorCR source)
    {
    size_t sum = 0;
    size_t n = source.size ();
    for (size_t i = 0; i < n; i++)
        {
        sum += source[i].GetPointsCR ().size ();
        }
    return sum;
    }

DRange3d PolygonVectorOps::GetRange (TaggedPolygonVectorCR source, size_t i)
    {
    DRange3d range;
    range.Init ();
    size_t numPolygons = source.size ();
    if (i < numPolygons)
        {
        bvector <DPoint3d> const & points = source[i].GetPointsCR ();
        range.Extend (&points[0], (int)points.size ());
        }
    return range;
    }

bool PolygonVectorOps::HasNonNullRange (TaggedPolygonVectorCR source, size_t i, DRange3dR range)
    {
    range = GetRange (source, i);
    return !range.IsNull ();
    }

void PolygonVectorOps::Multiply (TaggedPolygonVectorR polygons, TransformCR transform)
    {
    size_t n = polygons.size ();
    for (size_t i = 0; i < n; i++)
        {
        bvector <DPoint3d> & points = polygons[i].GetPointsR ();
        transform.Multiply (&points[0], (int)points.size ());
        }
    }
void TaggedPolygon::Multiply (TransformCR transform)
    {
    transform.Multiply (&m_points[0], (int)m_points.size ());
    }

void PolygonVectorOps::MultiplyAndRenormalize (TaggedPolygonVectorR polygons, DMatrix4dCR matrix)
    {
    size_t n = polygons.size ();
    for (size_t i = 0; i < n; i++)
        {
        bvector <DPoint3d> & points = polygons[i].GetPointsR ();
        matrix.MultiplyAndRenormalize (&points[0], &points[0], (int)points.size ());
        }
    }


DRange3d TaggedPolygon::GetRange () const
    {
    DRange3d range;
    range.Init ();
    if (m_points.size () > 0)
        {
        range.Extend (&m_points[0], (int)m_points.size ());
        }
    return range;
    }

TaggedPolygon::TaggedPolygon (bvector<DPoint3d> points, ptrdiff_t indexA, ptrdiff_t indexB, double tag)
    : m_tag (indexA, indexB, tag),
      m_points(points)
    {
    }

TaggedPolygon::TaggedPolygon (DPoint3dCP points, size_t n,  ptrdiff_t indexA, ptrdiff_t indexB, double tag )
    : m_tag (indexA, indexB, tag)
    {
    if (n > 0)
        {
        m_points.reserve (n);
        for (size_t i = 0; i < n; i++)
            m_points.push_back (points[i]);
        }
    }
TaggedPolygon::TaggedPolygon ()
    : m_tag ()
    {
    }

TaggedPolygon::TaggedPolygon (ptrdiff_t indexA, ptrdiff_t indexB, double tag)
    : m_tag (indexA, indexB, tag)
    {
    }

void TaggedPolygon::SetIndexA (ptrdiff_t indexA) { m_tag.m_indexA = indexA;}
void TaggedPolygon::SetIndexB (ptrdiff_t indexB) { m_tag.m_indexB = indexB;}
void TaggedPolygon::SetTag (double dTag) { m_tag.m_a = dTag;}

ptrdiff_t TaggedPolygon::GetIndexA () const {return m_tag.m_indexA;}
ptrdiff_t TaggedPolygon::GetIndexB () const {return m_tag.m_indexB;}
double    TaggedPolygon::GetTag () const {return m_tag.m_a;}

bvector<DPoint3d> &TaggedPolygon::GetPointsR () {return m_points;}
bvector<DPoint3d> const &TaggedPolygon::GetPointsCR () const {return m_points;}
size_t TaggedPolygon::GetPointSize () const {return m_points.size ();}

DPoint3dP TaggedPolygon::GetDataP () {return &m_points[0];}
DPoint3dCP TaggedPolygon::GetDataCP () const {return &m_points[0];}

bool TaggedPolygon::TryGetPoint (size_t index, DPoint3dR value) const
    {
    if (index < m_points.size ())
        {
        value = m_points[index];
        return true;
        }
    return false;
    }

bool TaggedPolygon::TryGetCyclicPoint (size_t index, DPoint3dR value) const
    {
    size_t n = m_points.size ();
    if (n == 0)
        return false;
    index = index % n;
    value = m_points[index];
    return true;
    }


size_t TaggedPolygon::GetTrimmedSize (double abstol) const
    {
    size_t n = m_points.size ();
    while (n > 1 && m_points[n-1].Distance(m_points[n-2]) < abstol)
        {
        n--;
        }
    return n;
    }

void TaggedPolygon::CopyTagsFrom (TaggedPolygonCR source)
    {
    m_tag = source.m_tag;
    }

void TaggedPolygon::Clear () {m_points.clear ();}
void TaggedPolygon::Add (DPoint3dCR xyz) {m_points.push_back (xyz);}

void TaggedPolygon::Add (double x, double y, double z)
    {
    DPoint3d xyz;
    xyz.x = x;
    xyz.y = y;
    xyz.z = z;
    m_points.push_back (xyz);
    }

TaggedPolygon::PolygonTag::PolygonTag() : m_indexA (0), m_indexB(0), m_a (0.0) {}

TaggedPolygon::PolygonTag::PolygonTag (ptrdiff_t indexA, ptrdiff_t indexB, double a)
    : m_indexA (indexA), m_indexB(indexB), m_a (a) {}


void TaggedPolygon::Swap (TaggedPolygon &other)
    {
    std::swap (m_tag, other.m_tag);
    m_points.swap (other.m_points);
    }

double TaggedPolygon::Area (DVec3dCR perpendicularVector) const
    {
    DVec3d zVector = perpendicularVector;
    zVector.Normalize ();
    int n = (int)m_points.size ();
    if (n <= 2)
        return 0.0;
    return 3.0 * bsiPolygon_tentVolume (GetDataCP (), n, &zVector);
    }

double TaggedPolygon::AreaXY () const
    {
    DVec3d zVector;
    zVector.Init (0, 0, 1);
    return Area (zVector);
    }

END_BENTLEY_GEOMETRY_NAMESPACE
