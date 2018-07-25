/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/XYBucketSearch.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

#include <algorithm>
BEGIN_BENTLEY_GEOMETRY_INTERNAL_NAMESPACE

#define XYBucketSearchTagType size_t

class   XYBucketSearch;
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
class XYBucketSearch :  public RefCountedBase
{
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
/// <param name="xOut">returned closest point x</param>
/// <param name="yOut">returned closest point y</param>
/// <param name="dataOut">returned tag</param>
bool GEOMDLLIMPEXP ClosestPoint (double x, double y, double &xOut, double &yOut, XYBucketSearchTagType &dataOut);

/// <summary>Invoke a callback for all points that fall in a specified range.</summary>
void GEOMDLLIMPEXP CollectPointsInRange (double xMin, double yMin, double xMax, double yMax,
    bvector<DPoint3d> &searchPoint,
    bvector<XYBucketSearchTagType> &searchId);

/// <summary>Slow search for the closest point</summary>
/// <param name="x">x coordinate of search</param>
/// <param name="y">y coordinate of search</param>
/// <param name="xOut">returned closest point x</param>
/// <param name="yOut">returned closest point y</param>
/// <param name="dataOut">returned tag</param>
bool GEOMDLLIMPEXP ClosestPointLinear (double x, double y, double &xOut, double &yOut, XYBucketSearchTagType &dataOut);

};


END_BENTLEY_GEOMETRY_INTERNAL_NAMESPACE
