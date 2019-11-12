/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <float.h>
#include <math.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

XYBucketSearch::TaggedPoint::TaggedPoint (DPoint3dCR _xyz, XYBucketSearchTagType dd)
    : xyz (_xyz), data (dd)
    {
    }

XYBucketSearch::RowData::RowData (int _i0, int _i1, double _a0, double _a1)
    : i0(_i0), i1(_i1), a0(_a0), a1(_a1)
    {
    }

XYBucketSearchPtr XYBucketSearch::Create ()
    {
    return new XYBucketSearch ();
    }


void XYBucketSearch::CollectPointsInRangeXY (DRange2dCR range2d,
bvector<DPoint3d> &searchPoint,
bvector<XYBucketSearchTagType> &searchId
)
    {
    static double a = 1.0e14;
    DRange3d range3d = DRange3d::From (
        DPoint3d::From (range2d.low.x, range2d.low.y, -a),
        DPoint3d::From (range2d.high.x, range2d.high.y, a));
        
    return CollectPointsInRangeXYZ (range3d, searchPoint, searchId);
    }

XYBucketSearch::XYBucketSearch ()
    {
    m_sorted = false;
    m_range = DRange3d::NullRange ();
    }

bool XYBucketSearch::GetPoint (unsigned int i, DPoint3dR xyz, XYBucketSearchTagType &data) const
    {
    if (i > points.size ())
        return false;
    TaggedPoint tp = points[i];
    xyz = tp.xyz;
    data = tp.data;
    return true;
    }

DRange3d XYBucketSearch::GetRange () const { return m_range;}

bool XYBucketSearch::AddPoint (DPoint3dCR xyz, XYBucketSearchTagType data)
    {
    TaggedPoint tp (xyz, data);
    points.push_back (tp);
    m_range.Extend (xyz);
    m_sorted = false;
    return true;
    }

double XYBucketSearch::DistanceSquaredXY (double x, double y, unsigned int i)
    {
    if (i > points.size ())
        return DBL_MAX;
    double dx = x - points[i].xyz.x;
    double dy = y - points[i].xyz.y;
    return dx * dx + dy * dy;
    }

bool XYBucketSearch::cb_compareForYSort (const TaggedPoint &pointA, const TaggedPoint &pointB)
    {
    if (pointA.xyz.y < pointB.xyz.y)
        return true;
    if (pointA.xyz.y > pointB.xyz.y)
        return false;
    return pointA.xyz.x < pointB.xyz.x;
    }

bool XYBucketSearch::cb_compareForXSort (const TaggedPoint &pointA, const TaggedPoint &pointB)
    {
    if (pointA.xyz.x < pointB.xyz.x)
        return true;
    if (pointA.xyz.x > pointB.xyz.x)
        return false;
    return pointA.xyz.y < pointB.xyz.y;
    }

#define MAX_XYPOINTSEARCH_ROW 2000
#define MIN_PER_ROW 20
void XYBucketSearch::Sort ()
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

int XYBucketSearch::SearchRowContainingY (double y)
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
int XYBucketSearch::SplitRow (int rowIndex, double x)
    {
    RowData r = rows[rowIndex];     // rowIndex assumed ok ...
    int i0 = r.i0;
    int i1 = r.i1;
    int k0;
    if (x <= points[i0].xyz.x)
        k0 = i0;
    else if (x >= points[i1 - 1].xyz.x)
        k0 = i1 - 1;
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


double XYBucketSearch::ClosestPointInRow (int rowIndex, double x, double y, DPoint3dR xyzOut, XYBucketSearchTagType &dataOut, int &iMin)
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

    xyzOut = points[iMin].xyz;
    dataOut = points[iMin].data;
    return dMin;
    }

bool XYBucketSearch::ClosestPoint (double x, double y, DPoint3dR xyz, XYBucketSearchTagType &dataOut)
    {
    if (points.size () < 1)
        return false;
    if (!m_sorted)
        Sort ();

    int iMin = 0;
    int midRow = SearchRowContainingY (y);
    double dMin = ClosestPointInRow (midRow, x, y, xyz, dataOut, iMin);
    bool bUpActive = true;
    bool bDownActive = true;
    double d;
    DPoint3d xyzB;
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
                d = ClosestPointInRow (r, x, y, xyzB, dataB, iMinB);
                if (d < dMin)
                    {
                    dMin = d;
                    xyz = xyzB;
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
                d = ClosestPointInRow (r, x, y, xyzB, dataB, iMinB);
                if (d < dMin)
                    {
                    dMin = d;
                    xyz = xyzB;
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
void XYBucketSearch::CollectPointsInRangeXYZ (
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
    int lowRow = SearchRowContainingY (yMin);
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

END_BENTLEY_GEOMETRY_NAMESPACE
