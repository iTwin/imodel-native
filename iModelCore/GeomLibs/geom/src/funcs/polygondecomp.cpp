/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/polygondecomp.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


#ifdef polygonDecompDebug
END_BENTLEY_GEOMETRY_NAMESPACE
#include    <toolsubs.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#endif

typedef double IndexedCrossProductFunction (DPoint3d *, size_t, size_t, size_t);
/*---------------------------------------------------------------------------------**//**
* Twice the signed area of the triangle formed by three points identified by indices
* in an array.  Only xy coordinates are used.
* @param points => point array.
* @param i0 => first index.
* @param i1 => second index.
* @param i2 => third index.
* @return cross product
* @return
* @bsimethod                                                    EarlinLutz      06/99
+---------------+---------------+---------------+---------------+---------------+------*/
static double jmdlConvex_crossProduct3IndicesXY

(
bvector<DPoint3d> const &points,
size_t      i0,
size_t      i1,
size_t      i2
)
    {
    DPoint3d point0 = points[i0];
    DPoint3d point1 = points[i1];
    DPoint3d point2 = points[i2];
    return (point1.x - point0.x) * (point2.y - point0.y)
         - (point1.y - point0.y) * (point2.x - point0.x);
    }


/*---------------------------------------------------------------------------------**//**
* Return the successor of an index in cyclic, zero based numbering.
* @param i => index to increment.  Assumed nonnegative.
* @param n => number of indices in cycle.
* @return incremented and wrapped index.
* @bsimethod                                                    EarlinLutz      06/99
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t jmdlConvex_incrementCyclic

(
size_t     i,
size_t     n
)
    {
    size_t j = i + 1;
    return j < n ? j : 0;
    }

/*---------------------------------------------------------------------------------**//**
* Return the predcessor of an index in cyclic, zero based numbering.
* @param i => index to decremented.  Assumed nonnegative.
* @param n => number of indices in cycle.
* @return decremented and wrapped index.
* @bsimethod                                                    EarlinLutz      06/99
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t jmdlConvex_decrementCyclic

(
size_t     i,
size_t     n
)
    {
    if (i == 0)
        return n - 1;
    return i - 1;
    }


/*---------------------------------------------------------------------------------**//**
* Sweep from i0 to i1 (cyclically).
* @bsimethod                                                    EarlinLutz      06/99
+---------------+---------------+---------------+---------------+---------------+------*/
static ptrdiff_t jmdlConvex_indexedHullSweep

(
bvector<ptrdiff_t> &indexArray,
bvector<DPoint3d> const &points,
size_t      i0,
size_t      i1,
double   sign,
ClipPlaneTreeR     tree
)
    {
    size_t numPoint = points.size ();
    size_t k;
    size_t kStop;
    size_t baseCount = indexArray.size ();
    size_t singletonStackHeight = baseCount + 1;

    k = jmdlConvex_incrementCyclic (i0, numPoint);
    if (i1 == k)
        {
        indexArray.push_back (i0);
        indexArray.push_back (i1);
        }
    else
        {
        kStop = i0 == i1 ? i0 : jmdlConvex_incrementCyclic (i1, numPoint);
        indexArray.push_back (i0);
        k = jmdlConvex_incrementCyclic (i0, numPoint);
        indexArray.push_back (k);
        k = jmdlConvex_incrementCyclic (k, numPoint);
        do  {
            /* Pop concave points from the stack */
            while (   indexArray.size () > singletonStackHeight
                   && sign * jmdlConvex_crossProduct3IndicesXY (points,
                                        indexArray[indexArray.size () - 2],
                                        indexArray[indexArray.size () - 1],
                                        k) < 0.0)
                {
                indexArray.pop_back ();
                }

            indexArray.push_back (k);
            k = jmdlConvex_incrementCyclic (k, numPoint);
            } while (k != kStop);
        indexArray.push_back (kStop);
        /* Pop from stack tail, looking back form kStop */
        while (   baseCount <= indexArray.size () - 3
                && sign * jmdlConvex_crossProduct3IndicesXY (points,
                                    indexArray[indexArray.size () - 3],
                                    indexArray[indexArray.size () - 2],
                                    indexArray.back ()) < 0.0
              )
            {
            indexArray.pop_back ();
            }

        }
    return indexArray.size () - baseCount;
    }

/*---------------------------------------------------------------------------------**//**
* Find a point on the lowest y level, and among multiple points at this level choose
* the one with lowest x.
* @bsimethod                                                    EarlinLutz      06/99
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t jmdlConvex_lowerLeftIndex

(
bvector<DPoint3d> const &points
)
    {
    size_t i, k;
    double xk, yk;
    k = 0;
    xk = points[0].x;
    yk = points[0].y;
    for (i = 1; i < points.size (); i++)
        {
        if (   points[i].y < yk
            || (points[i].y == yk && points[i].x < xk))
            {
            k = i;
            xk = points[i].x;
            yk = points[i].y;
            }
        }
    return k;
    }

/*---------------------------------------------------------------------------------**//**
* Recursively sweep a polygon to find (not necessarily contiguous) sequences of
* points on convex components.  Indices are pushed onto the index array separated by
* -1.
* @bsimethod                                                    EarlinLutz      06/99
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t jmdlConvex_recursiveXORHullSweeps

(
bvector<ptrdiff_t> &indexArray,
bvector<DPoint3d> const &points,
size_t             i0,
size_t             i1,
size_t             depth,
ClipPlaneTreeR     tree
)
    {
    size_t numPoint = points.size ();
    size_t numHullPoint;
    size_t j0, j1, k0, k1, kTest, numInterval;
    size_t j0Index = indexArray.size ();
    double sign = depth & 0x01 ? -1.0 : 1.0;
    size_t numHull = 0;
    numHullPoint = jmdlConvex_indexedHullSweep (indexArray,
                        points,
                        i0, i1,
                        sign,
                        tree
                        );

    if (j0Index + 1 < indexArray.size ())
        {
        size_t jLastIndex = indexArray.size () - 1;
        for (size_t j = j0Index; j + 1 < indexArray.size (); j++)
            {
            tree.AddXYPlane (points[indexArray[j]], points[indexArray[j+1]], sign);
            }
        tree.AddXYPlane (points[indexArray[jLastIndex]], points[indexArray[j0Index]], sign);
        }
    indexArray.push_back (sign > 0 ? -1 : -2);


    numInterval = i0 == i1 ? numHullPoint : numHullPoint - 1;
    numHull++;
    /*
        At each place where there are non-contiguous
        indices in the current fragment,
        recurse for further search of that peninsula
    */
    for (j0 = 0; j0 < numInterval ; j0++)
        {
        j1 = jmdlConvex_incrementCyclic (j0, numHullPoint);
        k0 = indexArray [j0 + j0Index];
        k1 = indexArray [j1 + j0Index];
        kTest = jmdlConvex_incrementCyclic (k0, numPoint);
        if (kTest != k1)
                {
                tree.m_children.push_back (ClipPlaneTree (ClipPlaneTree::NodeType::Difference));
                numHull += jmdlConvex_recursiveXORHullSweeps (
                                        indexArray,
                                        points,
                                        k0, k1, depth + 1, tree.m_children.back ());
                }
        }

    return numHull;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      06/99
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t jmdlConvex_setInitialLevel

(
bvector<DPoint3d> const &points,
size_t     i1
)
    {
    size_t i0, i2;
    i0 = jmdlConvex_decrementCyclic (i1, points.size ());
    i2 = jmdlConvex_incrementCyclic (i1, points.size ());
    return jmdlConvex_crossProduct3IndicesXY (points, i0, i1, i2) < 0.0
                ? 1 : 0;
    }
/*---------------------------------------------------------------------------------**//**
* Decompose an (arbitrary) polygon into an XOR of simpler polygons.
* Each output polygon will have a consistent turning direction, either CW or CCW as
* indicated by the flag value.
* If you have any reason to feel that the input polygon does not criss-cross itself,
* "consistent turning direction" means it is convex.  If the input is not trusted, the
* output may need to be inspected to check for cases where the polygon spirals "inward" then
* outward so that it manages to cross over itself without having any changes in concavity.
*
* The output is an array of indices into the original points.
* Each output polygon is described by a sequence of point indices, followed by
* a negative index.   The negative index is  -1 for a CCW polygon, -2 for CW.
*
* @param pIndexArray <= indices and flags
* @param pNumIndex   <= number of ints in the index array.
* @param maxIndex    => max number of indices (dimension of pIndexArray).  The recommended
*                           value for this is twice the number of points.
* @param points => point coordinates.  Only the xy coordiantes are used -- transform
*                           to planar condition as needed.
* @param numPoint   => number of points in the array.  The initial/final point should NOT
*                       be duplicated.
* @bsimethod                                                    EarlinLutz      06/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP size_t bsiPolygon_decomposeXYToXOR

(
bvector<ptrdiff_t> &indexArray,
bvector<DPoint3d> const &points,
ClipPlaneTreeR tree
)
    {
    size_t i0;
    i0 = jmdlConvex_lowerLeftIndex (points);
    tree.Clear ();
    indexArray.clear ();
    return jmdlConvex_recursiveXORHullSweeps
                (
                indexArray,
                points,
                i0, i0, jmdlConvex_setInitialLevel (points, i0), tree);
    }


bool ClipPlaneTree::AddXYPlane (DPoint3dCR point0, DPoint3dCR point1, double sign)
    {
    DVec3d vector01 = DVec3d::FromStartEnd (point0, point1);
    auto unitNormal = DVec3d::From (-vector01.y, vector01.x).ValidatedNormalize ();
    if (unitNormal.IsValid ())
        {
        if (sign < 0.0)
            unitNormal.Value ().Negate ();
        m_planes.push_back (ClipPlane (unitNormal, point0));
        return true;
        }
    return false;
    }

bool ClipPlaneTree::IsPointInOrOn (DPoint3dCR point) const
    {
    if (m_nodeType == NodeType::Difference)
        {
        for (auto &plane : m_planes)
            if (!plane.IsPointOnOrInside (point))
                return false;

        for (auto & child : m_children)
            {
            if (child.IsPointInOrOn (point))
                return false;
            }
        }
    else if (m_nodeType == NodeType::Union)
        {
        for (auto &plane : m_planes)
            if (!plane.IsPointOnOrInside (point))
                return false;

        for (auto & child : m_children)
            {
            if (child.IsPointInOrOn (point))
                return true;
            }
        }
    return true;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
