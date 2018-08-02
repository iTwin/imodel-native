/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/XYBucketSearch.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/


BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define XYBucketSearchTagType size_t

struct   XYBucketSearch;
typedef RefCountedPtr<XYBucketSearch> XYBucketSearchPtr;


/// <summary>Point cloud search structure.</summary>
///
/// Usage pattern:
///     XYBucketSearchP pSearcher = Allocate ();
///     for each reference point
///         pSearcher->AddPoint (x, y, tag)
///
///     for each search point
///         pSearcher->ClosestPoint (x, y, xOut, yOut, tagOut)
///
///    To collect all search results
///         bvector<DPoint3d> searchPoint;
///         bvector<size_t> searchId;
///         pSearcher->SearchByRange (xMin, yMin, xMax, yMax, searchPoint, searchId);
///
/// Points may be added "after" searches.  However, be aware that the "next" search after 1 or more points are added
///   will incur a significant search/sort cost.    Hence it is best to do "AddPoint" calls in large batches separate from
///   large batches of ClosestPoint or SearchByRange calls.
struct XYBucketSearch :  public RefCountedBase
{
public:
struct TaggedPoint
    {
    DPoint3d xyz;
    XYBucketSearchTagType data;
    GEOMDLLIMPEXP TaggedPoint (DPoint3dCR _xyz, XYBucketSearchTagType dd);
    };

struct RowData
    {
    int i0;
    int i1;
    double a0;
    double a1;
    GEOMDLLIMPEXP RowData (int _i0, int _i1, double _a0, double _a1);
    };
private:
    // Array of all points with tags.
    // These are shuffled to achieve (1) buckets with y bands and (2) x sort withing bucket
    bvector<TaggedPoint> points;
    // Per row:
    // i0,i1 = limit indices for row 
    bvector<RowData>     rows;
    // if false, data has been added and the points array must be resorted.
    bool m_sorted;
    // overall range of points in the array.
    DRange3d m_range;


/// <summary>Return squared distance from x,y to indexed point.</summary>
double DistanceSquaredXY (double x, double y, unsigned int i);

/// <summary>"Less than" function for std::sort.  Lexical ordering by y then x.</summary>
static bool cb_compareForYSort (TaggedPoint const &pointA, TaggedPoint const &pointB);
/// <summary>"Less than" function for std::sort.  Lexical ordering by x then y..</summary>
static bool cb_compareForXSort (TaggedPoint const &pointA, TaggedPoint const &pointB);

/// <summary>If data array is dirty (has points added since previous search),
///         set up new search structures.  This is expensive.
/// </summary>
void Sort ();

/// <summary> search x-sorted row. </summary>
/// <return> returns smallest k such that points[k].x >= x</return>
int SplitRow (int rowIndex, double x);
/// <summary>Find the data row containing specified y value.</summary>
int SearchRowContainingY (double y);

/// <summary>Find the closest point within specified row.</summary>
double ClosestPointInRow (int rowIndex, double x, double y, DPoint3dR xyz, XYBucketSearchTagType &dataOut, int &iMin);

protected:
XYBucketSearch (); // No copies allowed....

public:

/// <summary>Allocate a new searcher.
///     (Use delete operator to free.)</summary>
static GEOMDLLIMPEXP XYBucketSearchPtr Create ();

/// <summary>Add an <x,y,XYBucketSearchTagType> dataum.  Range data is updated and the mbSorted flag is set to
///    force a sort on the next query.</summary>
bool GEOMDLLIMPEXP AddPoint (DPoint3dCR xyz, XYBucketSearchTagType data);

/// <summary>Query the range of the points.</summary>
/// <return>True if point count is nonzero</return>
DRange3d GEOMDLLIMPEXP GetRange () const;

/// <summary>Return a point from the search structure.  Point indices can change due to sorting.</summary>
bool GEOMDLLIMPEXP  GetPoint (unsigned int i, DPoint3dR xyz, XYBucketSearchTagType &data) const;

/// <summary>Fast search for the closest point</summary>
/// <param name="x">x coordinate of search</param>
/// <param name="y">y coordinate of search</param>
/// <param name="xyz">returned closest point</param>
/// <param name="dataOut">returned tag</param>
bool GEOMDLLIMPEXP ClosestPoint (double x, double y, DPoint3dR xyz, XYBucketSearchTagType &dataOut);

/// <summary>Invoke a callback for all points that fall in a specified range.</summary>
void GEOMDLLIMPEXP CollectPointsInRangeXY (DRange2dCR range,
    bvector<DPoint3d> &searchPoint,
    bvector<XYBucketSearchTagType> &searchId);

/// <summary>Collect points that fall in a specified range.</summary>
void GEOMDLLIMPEXP CollectPointsInRangeXYZ (DRange3dCR range,
    bvector<DPoint3d> &searchPoint,
    bvector<XYBucketSearchTagType> &searchId);

};


END_BENTLEY_GEOMETRY_NAMESPACE
