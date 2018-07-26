/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/XYBucketSearch.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <float.h>
#include <math.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


struct TaggedPoint
    {
    DPoint3d xyz;
    XYBucketSearchTagType data;
    TaggedPoint (DPoint3dCR _xyz, XYBucketSearchTagType dd, double zz = 0.0)
        : xyz (_xyz), data (dd)
        {
        }
    };

struct GEOMDLLIMPEXP RowData
    {
    int i0;
    int i1;
    double a0;
    double a1;
    RowData (int _i0, int _i1, double _a0, double _a1)
        {
        i0 = _i0;
        i1 = _i1;
        a0 = _a0;
        a1 = _a1;
        }
    };

/// <summary>Callback object for searches.</summary>
///
class XYBucketSearchUtils;
class XYBucketSearchAlgorithms;


class XYBucketSearchImplementation : XYBucketSearch
{
public:
/// <summary>Add an <x,y,XYBucketSearchTagType> dataum.  Range data is updated and the m_sorted flag is set to
///    force a sort on the next query.</summary>
bool imp_AddPoint (DPoint3dCR xyz, XYBucketSearchTagType data);

/// <summary>Query the range of the points.</summary>
/// <return>True if point count is nonzero</return>
DRange3d imp_GetRange () const {return m_range;}

/// <summary>Return a point from the search structure.  Point indices can change due to sorting.</summary>
bool imp_GetPoint (unsigned int i, DPoint3dR xyz, XYBucketSearchTagType &data) const;

/// <summary>Fast search for the closest point</summary>
/// <param name="x">x coordinate of search</param>
/// <param name="y">y coordinate of search</param>
/// <param name="xOut">returned closest point x</param>
/// <param name="yOut">returned closest point y</param>
/// <param name="dataOut">returned tag</param>
bool imp_ClosestPoint (double x, double y, double &xOut, double &yOut, XYBucketSearchTagType &dataOut);

/// <summary>Invoke a callback for all points that fall in a specified range.</summary>
void imp_CollectPointsInRangeXYZ (DRange3dCR range,
bvector<DPoint3d> &searchPoint,
bvector<XYBucketSearchTagType> &searchId
);

/// <summary>Slow search for the closest point</summary>
/// <param name="x">x coordinate of search</param>
/// <param name="y">y coordinate of search</param>
/// <param name="xOut">returned closest point x</param>
/// <param name="yOut">returned closest point y</param>
/// <param name="dataOut">returned tag</param>
bool imp_ClosestPointLinear (double x, double y, double &xOut, double &yOut, XYBucketSearchTagType &dataOut);


friend class XYBucketSearchUtils;
friend class XYBucketSearchAlgorithms;
friend class XYBucketSearch;

private:
    bvector<TaggedPoint> points;
    bvector<RowData>     rows;
    bool m_sorted;
    DRange3d m_range;
    XYBucketSearchImplementation ();

private:
/// <summary>Return squared distance from x,y to indexed point.</summary>
double DistanceSquaredXY (double x, double y, unsigned int i);
/// <summary>"Less than" function for std::sort.  Lexical ordering by y then x.</summary>
static bool cb_compareForYSort (const TaggedPoint &pointA, const TaggedPoint &pointB);
/// <summary>"Less than" function for std::sort.  Lexical ordering by x then y..</summary>
static bool cb_compareForXSort (const TaggedPoint &pointA, const TaggedPoint &pointB);

/// <summary>If data array is dirty (has points added since previous search),
///         set up new search structures.  This is expensive.
/// </summary>
void Sort ();

/// <summary> search x-sorted row. </summary>
/// <return> returns smallest k such that points[k].x >= x</return>
int SplitRow (int rowIndex, double x);
/// <summary>Find the data row containing specified y value.</summary>
int SelectRow (double y);

/// <summary>Find the closest point within specified row.</summary>
double ClosestPointInRow (int rowIndex, double x, double y, double &xOut, double &yOut, XYBucketSearchTagType &dataOut, int &iMin);

};

XYBucketSearchPtr XYBucketSearch::Create ()
    {
    return new XYBucketSearchImplementation ();
    }

XYBucketSearch::XYBucketSearch ()
    {
    }

bool XYBucketSearch::AddPoint (DPoint3dCR xyz, XYBucketSearchTagType data)
    {return ((XYBucketSearchImplementation*)this)->imp_AddPoint (xyz, data);}

DRange3d XYBucketSearch::GetRange () const
    {return ((XYBucketSearchImplementation*)this)->imp_GetRange ();}

bool XYBucketSearch::GetPoint (unsigned int i, DPoint3dR xyz, XYBucketSearchTagType &data) const
    {return ((XYBucketSearchImplementation*)this)->imp_GetPoint (i, xyz, data);}

bool XYBucketSearch::ClosestPoint (double x, double y, double &xOut, double &yOut, XYBucketSearchTagType &dataOut)
    {return ((XYBucketSearchImplementation*)this)->imp_ClosestPoint (x, y, xOut, yOut, dataOut);}

void XYBucketSearch::CollectPointsInRangeXY (DRange2dCR range2d,
bvector<DPoint3d> &searchPoint,
bvector<XYBucketSearchTagType> &searchId
)
    {
    static double a = 1.0e14;
    DRange3d range3d = DRange3d::From (
        DPoint3d::From (range2d.low, -a),
        DPoint3d::From (range2d.high, a));
        
    return ((XYBucketSearchImplementation*)this)->imp_CollectPointsInRangeXYZ (range3d, searchPoint, searchId);
    }

void XYBucketSearch::CollectPointsInRangeXYZ (DRange3dCR range,
bvector<DPoint3d> &searchPoint,
bvector<XYBucketSearchTagType> &searchId
)
    {
    return ((XYBucketSearchImplementation*)this)->imp_CollectPointsInRangeXYZ (range, searchPoint, searchId);
    }

bool XYBucketSearch::ClosestPointLinear (double x, double y, double &xOut, double &yOut, XYBucketSearchTagType &dataOut)
    {return ((XYBucketSearchImplementation*)this)->imp_ClosestPointLinear (x, y, xOut, yOut, dataOut);}


XYBucketSearchImplementation::XYBucketSearchImplementation ()
    {
    m_sorted = false;
    m_range = DRange3d::NullRange ();
    }

bool XYBucketSearchImplementation::imp_GetPoint (unsigned int i, DPoint3dR xyz, XYBucketSearchTagType &data) const
    {
    if (i > points.size ())
        return false;
    TaggedPoint tp = points[i];
    xyz = tp.xyz;
    data = tp.data;
    return true;
    }

bool XYBucketSearchImplementation::imp_AddPoint (DPoint3dCR xyz, XYBucketSearchTagType data)
    {
    TaggedPoint tp (xyz, data);
    points.push_back (tp);
    m_range.Extend (xyz);
    m_sorted = false;
    return true;
    }

double XYBucketSearchImplementation::DistanceSquaredXY (double x, double y, unsigned int i)
    {
    if (i > points.size ())
        return DBL_MAX;
    double dx = x - points[i].xyz.x;
    double dy = y - points[i].xyz.y;
    return dx * dx + dy * dy;
    }

bool XYBucketSearchImplementation::cb_compareForYSort (const TaggedPoint &pointA, const TaggedPoint &pointB)
    {
    if (pointA.xyz.y < pointB.xyz.y)
        return true;
    if (pointA.xyz.y > pointB.xyz.y)
        return false;
    return pointA.xyz.x < pointB.xyz.x;
    }

bool XYBucketSearchImplementation::cb_compareForXSort (const TaggedPoint &pointA, const TaggedPoint &pointB)
    {
    if (pointA.xyz.x < pointB.xyz.x)
        return true;
    if (pointA.xyz.x > pointB.xyz.x)
        return false;
    return pointA.xyz.y < pointB.xyz.y;
    }

#define MAX_XYPOINTSEARCH_ROW 2000
#define MIN_PER_ROW 20
void XYBucketSearchImplementation::Sort ()
    {
    size_t numPoint = points.size ();
    if (points.size () == 0)
        return;
    rows.clear ();
    unsigned int numRow   = (unsigned int)sqrt ((double)numPoint);
    if (numRow > MAX_XYPOINTSEARCH_ROW)
        numRow = MAX_XYPOINTSEARCH_ROW;
    if (numRow < 1)
        numRow = 1;
    size_t numPerRow = numPoint / numRow;
    if (numPerRow < MIN_PER_ROW)
        {
        numRow = (int)(numPoint / MIN_PER_ROW);
        if (numRow < 1)
            numRow = 1;
        }

    std::sort (points.begin (), points.end (), cb_compareForYSort);
    unsigned int i0 = 0;
    unsigned int i1;
    for (unsigned int rowIndex = 0; rowIndex < numRow; rowIndex++, i0 = i1)
        {
        i1 = (unsigned int)((double) numPoint * (double)(rowIndex + 1)/ (double)numRow);
        rows.push_back (RowData(i0, i1, points[i0].xyz.y, points[i1-1].xyz.y));
        bvector<TaggedPoint>::iterator limit0 = points.begin ();
        limit0 += i0;
        bvector<TaggedPoint>::iterator limit1 = points.begin ();
        limit1 += i1;
        std::sort (limit0, limit1, cb_compareForXSort);
        }
    m_sorted = true;
    }

int XYBucketSearchImplementation::SelectRow (double y)
    {
    size_t numRow = rows.size ();
    size_t i0 = 0;
    size_t i1 = numRow;
    while (i1 > i0 + 1)
        {
        size_t j = (i0 + i1) >> 1;
        double yA = rows[j].a0;
        if (y < yA)
            i1 = j;
        else
            i0 = j;
        }
    return (int)i0;
    }

/// <summary> search x-sorted row. </summary>
/// <return> returns smallest k such that points[k].x >= x</return>
int XYBucketSearchImplementation::SplitRow (int rowIndex, double x)
    {
    RowData r = rows[rowIndex];     // rowIndex assumed ok ...
    int i0 = r.i0;
    int i1 = r.i1;
    int k0;
    if (x <= points[i0].xyz.x)
        k0 = i0;
    else if (x >= points[i1 - 1].xyz.x)
        k0 = i1;
    else
        {
        while (i1 > i0 + 1)
            {
            int iMid = (i0 + i1) / 2;
            if (x < points[iMid].xyz.x)
                i1 = iMid;
            else
                i0 = iMid;
            }
        k0 = i0;
        }
    return k0;
    }


double XYBucketSearchImplementation::ClosestPointInRow (int rowIndex, double x, double y, double &xOut, double &yOut, XYBucketSearchTagType &dataOut, int &iMin)
    {
    RowData r = rows[rowIndex];     // rowIndex assumed ok ...
    int i0 = r.i0;
    int i1 = r.i1;
    int k0 = SplitRow (rowIndex, x);
    iMin = k0;
    double d2 = DistanceSquaredXY (x, y, k0);

    double d2Min = d2;
    double dMin = sqrt (d2);
    double xMax = x + dMin;
    double xMin = x - dMin;
    i0 = r.i0;
    i1 = r.i1;
    // Linear search forward ...
    for (int i = k0 + 1; i < i1 && points[i].xyz.x < xMax; i++)
        {
        //double xx = points[i].x;
        d2 = DistanceSquaredXY (x, y, i);
        if (d2 < d2Min)
            {
            d2Min = d2;
            dMin = sqrt (d2);
            xMax = x + dMin;
            xMin = x - dMin;
            iMin = i;
            }
        }

    // Linear search backward ...
    for (int i = k0 - 1; i >= i0 && points[i].xyz.x > xMin; i--)
        {
        //double xx = points[i].x;
        d2 = DistanceSquaredXY (x, y, i);
        if (d2 < d2Min)
            {
            d2Min = d2;
            dMin = sqrt (d2);
            xMax = x + dMin;
            xMin = x - dMin;
            iMin = i;
            }
        }

    xOut = points[iMin].xyz.x;
    yOut = points[iMin].xyz.y;
    dataOut = points[iMin].data;
    return dMin;
    }

bool XYBucketSearchImplementation::imp_ClosestPoint (double x, double y, double &xOut, double &yOut, XYBucketSearchTagType &dataOut)
    {
    if (points.size () < 1)
        return false;
    if (!m_sorted)
        Sort ();

    int iMin = 0;
    int midRow = SelectRow (y);
    double dMin = ClosestPointInRow (midRow, x, y, xOut, yOut, dataOut, iMin);
    bool bUpActive = true;
    bool bDownActive = true;
    double d;
    double xB, yB;
    XYBucketSearchTagType dataB;
    int iMinB;

    size_t numRow = rows.size ();
    // Look in rows above and below until row boundaries assure no more closest points are possible.
    double yMax = y + dMin;
    double yMin = y - dMin;
    for (int rowStep = 1; bUpActive || bDownActive; rowStep++)
        {
        unsigned int r = (unsigned int)(midRow + rowStep);
        if (bUpActive)
            {
            if (r >= numRow || yMax < rows[r].a0)
                {
                bUpActive = false;
                }
            else
                {
                d = ClosestPointInRow (r, x, y, xB, yB, dataB, iMinB);
                if (d < dMin)
                    {
                    dMin = d;
                    xOut = xB;
                    yOut = yB;
                    dataOut = dataB;
                    yMin = y - dMin;
                    yMax = y + dMin;
                    }
                }
            }
        r = midRow - rowStep;
        if (bDownActive)
            {
            if (midRow < rowStep || yMin > rows[r].a1)
                {
                bDownActive = false;
                }
            else
                {
                d = ClosestPointInRow (r, x, y, xB, yB, dataB, iMinB);
                if (d < dMin)
                    {
                    dMin = d;
                    xOut = xB;
                    yOut = yB;
                    dataOut = dataB;
                    yMin = y - dMin;
                    yMax = y + dMin;
                    }
                }
            }
        }
    return true;
    }

/// <summary>Invoke a callback for all points that fall in a specified range.</summary>
void XYBucketSearchImplementation::imp_CollectPointsInRangeXYZ (
DRange3dCR range,
bvector<DPoint3d> &searchPoint,
bvector<XYBucketSearchTagType> &searchId
)
    {
    searchPoint.clear ();
    searchId.clear ();
    if (!m_sorted)
        Sort ();
    double xMin = range.low.x;
    double xMax = range.high.x;
    double yMin = range.low.y;
    double yMax = range.high.y;
    size_t numRow = rows.size ();
    int lowRow = SelectRow (yMin);
    for (size_t rowIndex = lowRow; rowIndex < numRow; rowIndex++)
        {
        RowData r = rows[rowIndex];     // rowIndex assumed ok ...
        //int i0 = r.i0;
        int i1 = r.i1;
        if (r.a1 < yMin)
            return;
        if (r.a0 > yMax)
            return;
        int k0 = SplitRow ((unsigned int)rowIndex, xMin);
        for (int k = k0; k < i1; k++)
            {
            TaggedPoint p = points[k];
            if (p.xyz.x > xMax)
                break;
            if (range.IsContained (p.xyz))
                {
                searchPoint.push_back (p.xyz);
                searchId.push_back (p.data);
                }
            }
        }
    return;
    }


bool XYBucketSearchImplementation::imp_ClosestPointLinear (double x, double y, double &xOut, double &yOut, XYBucketSearchTagType &dataOut)
    {
    size_t n = points.size ();
    if (n < 1)
        return false;
    double d2Min = DistanceSquaredXY (x, y, 0);
    xOut = points[0].xyz.x;
    yOut = points[0].xyz.y;
    dataOut = points[0].data;
    double d2;
    for (size_t i = 0; i < n; i++)
        {
        d2 = DistanceSquaredXY (x, y, (int)i);
        if (d2 < d2Min)
            {
            d2Min = d2;
            xOut = points[i].xyz.x;
            yOut = points[i].xyz.y;
            dataOut = points[i].data;
            }
        }
    return true;
    }


END_BENTLEY_GEOMETRY_NAMESPACE
