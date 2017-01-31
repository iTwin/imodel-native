/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/XYPointSearch.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

#include <algorithm>
BEGIN_BENTLEY_GEOMETRY_INTERNAL_NAMESPACE

#define XYPointSearcherTagType void *

class   XYPointSearcher;
typedef XYPointSearcher* XYPointSearcherP;

/// <summary>Point cloud search structure.</summary>
///
/// Usage pattern:
///     XYPointSearcherP pSearcher = Allocate ();
///     for each reference point
///         pSearcher->AddPoint (x, y, tag)
///
///     for each search point
///         pSearcher->ClosestPoint (x, y, xOut, yOut, tagOut)
///
///  To search by range, with a callback for each point in the range, create a handler object to receive the
////        point calls:
///
///      class MyRangeSearchHandler : PointHandler
///              {
///              virtual bool ContinueAfterPoint (double x, double y, XYPointSearcherTagType tag) override
///                     {
///                     process point x,y
///                     return true to continue search, false to terminate
///                     }
///              };
///
///        MyRangeSearchHandler handler;
///        pSearcher->SearchByRange (xMin, yMin, xMax, yMax, handler);
///
///
/// Points may be added "after" searches.  However, be aware that the "next" search after 1 or more points are added
///   will incur a significant search/sort cost.    Hence it is best to do "AddPoint" calls in large batches separate from
///   large batches of ClosestPoint or SearchByRange calls.
class GEOMDLLIMPEXP  XYPointSearcher
{
protected:
XYPointSearcher (); // No copies allowed....

public:
/// <summary>Allocate a new searcher.
///     (Use delete operator to free.)</summary>
static XYPointSearcherP Allocate ();

/// <summary>Add an <x,y,XYPointSearcherTagType> dataum.  Range data is updated and the mbSorted flag is set to
///    force a sort on the next query.</summary>
bool AddPoint (double x, double y, XYPointSearcherTagType data);

/// <summary>Query the range of the points.</summary>
/// <return>True if point count is nonzero</return>
bool GetRange (double &xMin, double &yMin, double &xMax, double &yMax);

/// <summary>Return a point from the search structure.  Point indices can change due to sorting.</summary>
bool GetPoint (unsigned int i, double &x, double &y, XYPointSearcherTagType &data);

/// <summary>Fast search for the closest point</summary>
/// <param name="x">x coordinate of search</param>
/// <param name="y">y coordinate of search</param>
/// <param name="xOut">returned closest point x</param>
/// <param name="yOut">returned closest point y</param>
/// <param name="dataOut">returned tag</param>
bool ClosestPoint (double x, double y, double &xOut, double &yOut, XYPointSearcherTagType &dataOut);

    /// <summary>Callback to handle points.  Return true to continue search, false to terminate.</summary>
    class PointHandler
        {
        public: GEOMAPI_VIRTUAL bool ContinueAfterPoint (double x, double y, XYPointSearcherTagType tag) = 0;
        };

/// <summary>Invoke a callback for all points that fall in a specified range.</summary>
void SearchByRange (double xMin, double yMin, double xMax, double yMax, PointHandler &handler);

/// <summary>Slow search for the closest point</summary>
/// <param name="x">x coordinate of search</param>
/// <param name="y">y coordinate of search</param>
/// <param name="xOut">returned closest point x</param>
/// <param name="yOut">returned closest point y</param>
/// <param name="dataOut">returned tag</param>
bool ClosestPointLinear (double x, double y, double &xOut, double &yOut, XYPointSearcherTagType &dataOut);


};


END_BENTLEY_GEOMETRY_INTERNAL_NAMESPACE
