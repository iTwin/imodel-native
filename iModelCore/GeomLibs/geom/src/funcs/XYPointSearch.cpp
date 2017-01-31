/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/XYPointSearch.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <float.h>
#include <math.h>
BEGIN_BENTLEY_GEOMETRY_INTERNAL_NAMESPACE


struct GEOMDLLIMPEXP TaggedPoint
    {
    double x, y;
    XYPointSearcherTagType data;
    TaggedPoint (double xx, double yy, XYPointSearcherTagType dd)
        {
        x = xx;
        y = yy;
        data = dd;
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
class XYPointSearcherUtils;
class XYPointSearcherAlgorithms;


class XYPointSearcherImplementation : XYPointSearcher
{
public:
/// <summary>Add an <x,y,XYPointSearcherTagType> dataum.  Range data is updated and the mbSorted flag is set to
///    force a sort on the next query.</summary>
bool imp_AddPoint (double x, double y, XYPointSearcherTagType data);

/// <summary>Query the range of the points.</summary>
/// <return>True if point count is nonzero</return>
bool imp_GetRange (double &xMin, double &yMin, double &xMax, double &yMax);

/// <summary>Recompute the range of the ponts.  This should never be needed -- AddPoint updates with each call</summary>
bool imp_ComputeRange ();

/// <summary>Return a point from the search structure.  Point indices can change due to sorting.</summary>
bool imp_GetPoint (unsigned int i, double &x, double &y, XYPointSearcherTagType &data);

/// <summary>Fast search for the closest point</summary>
/// <param name="x">x coordinate of search</param>
/// <param name="y">y coordinate of search</param>
/// <param name="xOut">returned closest point x</param>
/// <param name="yOut">returned closest point y</param>
/// <param name="dataOut">returned tag</param>
bool imp_ClosestPoint (double x, double y, double &xOut, double &yOut, XYPointSearcherTagType &dataOut);

/// <summary>Invoke a callback for all points that fall in a specified range.</summary>
void imp_SearchByRange (double xMin, double yMin, double xMax, double yMax, PointHandler &handler);

/// <summary>Slow search for the closest point</summary>
/// <param name="x">x coordinate of search</param>
/// <param name="y">y coordinate of search</param>
/// <param name="xOut">returned closest point x</param>
/// <param name="yOut">returned closest point y</param>
/// <param name="dataOut">returned tag</param>
bool imp_ClosestPointLinear (double x, double y, double &xOut, double &yOut, XYPointSearcherTagType &dataOut);


friend class XYPointSearcherUtils;
friend class XYPointSearcherAlgorithms;
friend class XYPointSearcher;

private:
    bvector<TaggedPoint> points;
    bvector<RowData>     rows;
    bool mbSorted;
    double mXMin, mYMin, mXMax, mYMax;
    XYPointSearcherImplementation ();

private:
/// <summary>Return squared distance from x,y to indexed point.</summary>
double DistanceSquared (double x, double y, unsigned int i);
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
double ClosestPointInRow (int rowIndex, double x, double y, double &xOut, double &yOut, XYPointSearcherTagType &dataOut, int &iMin);

};

XYPointSearcherP XYPointSearcher::Allocate ()
    {
    return new XYPointSearcherImplementation ();
    }

XYPointSearcher::XYPointSearcher ()
    {
    }

bool XYPointSearcher::AddPoint (double x, double y, XYPointSearcherTagType data)
    {return ((XYPointSearcherImplementation*)this)->imp_AddPoint (x, y, data);}

bool XYPointSearcher::GetRange (double &xMin, double &yMin, double &xMax, double &yMax)
    {return ((XYPointSearcherImplementation*)this)->imp_GetRange (xMin, yMin, xMax, yMax);}

bool XYPointSearcher::GetPoint (unsigned int i, double &x, double &y, XYPointSearcherTagType &data)
    {return ((XYPointSearcherImplementation*)this)->imp_GetPoint (i, x, y, data);}

bool XYPointSearcher::ClosestPoint (double x, double y, double &xOut, double &yOut, XYPointSearcherTagType &dataOut)
    {return ((XYPointSearcherImplementation*)this)->imp_ClosestPoint (x, y, xOut, yOut, dataOut);}

void XYPointSearcher::SearchByRange (double xMin, double yMin, double xMax, double yMax, PointHandler &handler)
    {return ((XYPointSearcherImplementation*)this)->imp_SearchByRange (xMin, yMin, xMax, yMax, handler);}

bool XYPointSearcher::ClosestPointLinear (double x, double y, double &xOut, double &yOut, XYPointSearcherTagType &dataOut)
    {return ((XYPointSearcherImplementation*)this)->imp_ClosestPointLinear (x, y, xOut, yOut, dataOut);}


XYPointSearcherImplementation::XYPointSearcherImplementation ()
    {
    mbSorted = false;
    mXMin = mYMin =  DBL_MAX;
    mXMax = mYMax = -DBL_MAX;
    }

bool XYPointSearcherImplementation::imp_GetPoint (unsigned int i, double &x, double &y, XYPointSearcherTagType &data)
    {
    if (i > points.size ())
        return false;
    TaggedPoint tp = points[i];
    x = tp.x;
    y = tp.y;
    data = tp.data;
    return true;
    }

bool XYPointSearcherImplementation::imp_AddPoint (double x, double y, XYPointSearcherTagType data)
    {
    TaggedPoint tp (x, y, data);
    points.push_back (tp);

    if (tp.x < mXMin)
        mXMin = tp.x;
    if (tp.y < mYMin)
        mYMin = tp.y;
    if (tp.x > mXMax)
        mXMax = tp.x;
    if (tp.y > mYMax)
        mYMax = tp.y;

    mbSorted = false;
    return true;
    }

double XYPointSearcherImplementation::DistanceSquared (double x, double y, unsigned int i)
    {
    if (i > points.size ())
        return DBL_MAX;
    double dx = x - points[i].x;
    double dy = y - points[i].y;
    return dx * dx + dy * dy;
    }

bool XYPointSearcherImplementation::imp_GetRange (double &xMin, double &yMin, double &xMax, double &yMax)
    {
    xMin = mXMin;
    yMin = mYMin;
    xMax = mXMax;
    yMax = mYMax;
    return points.size () > 0;
    }

bool XYPointSearcherImplementation::imp_ComputeRange ()
    {
    mXMin = mYMin =  DBL_MAX;
    mXMax = mYMax = -DBL_MAX;
    size_t n = points.size ();
    for (size_t i = 0; i < n; i++)
        {
        TaggedPoint tp = points[i];
        if (tp.x < mXMin)
            mXMin = tp.x;
        if (tp.y < mYMin)
            mYMin = tp.y;
        if (tp.x > mXMax)
            mXMax = tp.x;
        if (tp.y > mYMax)
            mYMax = tp.y;
        }
    return n > 0;
    }

bool XYPointSearcherImplementation::cb_compareForYSort (const TaggedPoint &pointA, const TaggedPoint &pointB)
    {
    if (pointA.y < pointB.y)
        return true;
    if (pointA.y > pointB.y)
        return false;
    return pointA.x < pointB.x;
    }

bool XYPointSearcherImplementation::cb_compareForXSort (const TaggedPoint &pointA, const TaggedPoint &pointB)
    {
    if (pointA.x < pointB.x)
        return true;
    if (pointA.x > pointB.x)
        return false;
    return pointA.y < pointB.y;
    }

#define MAX_XYPOINTSEARCH_ROW 2000
#define MIN_PER_ROW 20
void XYPointSearcherImplementation::Sort ()
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
        rows.push_back (RowData(i0, i1, points[i0].y, points[i1-1].y));
        bvector<TaggedPoint>::iterator limit0 = points.begin ();
        limit0 += i0;
        bvector<TaggedPoint>::iterator limit1 = points.begin ();
        limit1 += i1;
        std::sort (limit0, limit1, cb_compareForXSort);
        }
    mbSorted = true;
    }

int XYPointSearcherImplementation::SelectRow (double y)
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
int XYPointSearcherImplementation::SplitRow (int rowIndex, double x)
    {
    RowData r = rows[rowIndex];     // rowIndex assumed ok ...
    int i0 = r.i0;
    int i1 = r.i1;
    int k0;
    if (x <= points[i0].x)
        k0 = i0;
    else if (x >= points[i1 - 1].x)
        k0 = i1;
    else
        {
        while (i1 > i0 + 1)
            {
            int iMid = (i0 + i1) / 2;
            if (x < points[iMid].x)
                i1 = iMid;
            else
                i0 = iMid;
            }
        k0 = i0;
        }
    return k0;
    }


double XYPointSearcherImplementation::ClosestPointInRow (int rowIndex, double x, double y, double &xOut, double &yOut, XYPointSearcherTagType &dataOut, int &iMin)
    {
    RowData r = rows[rowIndex];     // rowIndex assumed ok ...
    int i0 = r.i0;
    int i1 = r.i1;
    int k0 = SplitRow (rowIndex, x);
    iMin = k0;
    double d2 = DistanceSquared (x, y, k0);

    double d2Min = d2;
    double dMin = sqrt (d2);
    double xMax = x + dMin;
    double xMin = x - dMin;
    i0 = r.i0;
    i1 = r.i1;
    // Linear search forward ...
    for (int i = k0 + 1; i < i1 && points[i].x < xMax; i++)
        {
        //double xx = points[i].x;
        d2 = DistanceSquared (x, y, i);
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
    for (int i = k0 - 1; i >= i0 && points[i].x > xMin; i--)
        {
        //double xx = points[i].x;
        d2 = DistanceSquared (x, y, i);
        if (d2 < d2Min)
            {
            d2Min = d2;
            dMin = sqrt (d2);
            xMax = x + dMin;
            xMin = x - dMin;
            iMin = i;
            }
        }

    xOut = points[iMin].x;
    yOut = points[iMin].y;
    dataOut = points[iMin].data;
    return dMin;
    }

bool XYPointSearcherImplementation::imp_ClosestPoint (double x, double y, double &xOut, double &yOut, XYPointSearcherTagType &dataOut)
    {
    if (points.size () < 1)
        return false;
    if (!mbSorted)
        Sort ();

    int iMin = 0;
    int midRow = SelectRow (y);
    double dMin = ClosestPointInRow (midRow, x, y, xOut, yOut, dataOut, iMin);
    bool bUpActive = true;
    bool bDownActive = true;
    double d;
    double xB, yB;
    XYPointSearcherTagType dataB;
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
void XYPointSearcherImplementation::imp_SearchByRange
(
double xMin,
double yMin,
double xMax,
double yMax,
PointHandler &handler
)
    {
    if (!mbSorted)
        Sort ();
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
            if (p.x > xMax)
                return;
            if (p.y >= yMin && p.y <= yMax)
                if (!handler.ContinueAfterPoint (p.x, p.y, p.data))
                    return;
            }
        }
    return;
    }


bool XYPointSearcherImplementation::imp_ClosestPointLinear (double x, double y, double &xOut, double &yOut, XYPointSearcherTagType &dataOut)
    {
    size_t n = points.size ();
    if (n < 1)
        return false;
    double d2Min = DistanceSquared (x, y, 0);
    xOut = points[0].x;
    yOut = points[0].y;
    dataOut = points[0].data;
    double d2;
    for (size_t i = 0; i < n; i++)
        {
        d2 = DistanceSquared (x, y, (int)i);
        if (d2 < d2Min)
            {
            d2Min = d2;
            xOut = points[i].x;
            yOut = points[i].y;
            dataOut = points[i].data;
            }
        }
    return true;
    }


END_BENTLEY_GEOMETRY_INTERNAL_NAMESPACE
