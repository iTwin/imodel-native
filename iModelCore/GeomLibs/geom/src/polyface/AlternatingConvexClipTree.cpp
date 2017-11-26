/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/AlternatingConvexClipTree.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void AlternatingConvexClipTreeNode::InitialzeWithIndices (size_t index0, size_t numPoint, bool isPositiveArea)
    {
    m_startIndex = index0;
    m_numPoints = numPoint;
    m_isPositiveArea = isPositiveArea;
    m_children.clear ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void AlternatingConvexClipTreeNode::AddEmptyChild (size_t index0, size_t numPoint)
    {
    m_children.push_back (AlternatingConvexClipTreeNode ());
    m_children.back ().InitialzeWithIndices (index0, numPoint, !m_isPositiveArea);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void AlternatingConvexClipTreeNode::AddPlane (ClipPlaneCR plane)
    {
    m_planes.Add (plane);
    }
/*---------------------------------------------------------------------------------**//**
* Recursive search with alternating in and out semantics.
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlternatingConvexClipTreeNode::IsPointOnOrInside (DPoint3d xyz) const
    {
    bool inRoot = m_planes.IsPointOnOrInside (xyz, 0.0);
    if (!inRoot)
        return false;

    for (auto &child : m_children)
        if (child.IsPointOnOrInside (xyz))
            return false;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void AlternatingConvexClipTreeNode::SetAlternatingAreaFlag (bool value)
    {
    m_isPositiveArea = value;
    for (auto &child : m_children)
        child.SetAlternatingAreaFlag (!value);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void AlternatingConvexClipTreeNode::CaptureConvexClipPlaneSetAsVoid (AlternatingConvexClipTreeNode &child)
    {
    m_children.push_back (AlternatingConvexClipTreeNode ());
    std::swap (child, m_children.back ());
    m_children.back ().SetAlternatingAreaFlag (!m_isPositiveArea);
    }



//! Context structure for building an AlternatingConvexClipTreeNode from a polygon.
//! <ul>
//! <li> The polygon is copied to the local m_points structure.
//! <li> During construction, m_stack contains indices of a sequence of points with uniform concavity.
//! </ul>
struct AlternatingConvexClipTreeBuilder
{

bvector<DPoint3d> m_points;
bvector<size_t> m_stack;
size_t m_numPoints;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PushIndex (size_t primaryPointIndex)
    {
    m_stack.push_back (primaryPointIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsInsideTurn (DPoint3dCR xyzA, DPoint3dCR xyzB, DPoint3dCR xyzC, double sign)
    {
    return sign * Cross(xyzA, xyzB, xyzC) > 0;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
double Cross (DPoint3dCR xyzA, DPoint3dCR xyzB, DPoint3dCR xyzC)
    {
    return xyzA.CrossProductToPointsXY (xyzB, xyzC);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Period () const { return m_points.size ();}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d CyclicStackPoint (int32_t cyclicIndex)  // SIGNED index -- but negatives must be in first 10 periods?
    {
    size_t stackIndex;
    if (cyclicIndex > 0)
        {
        stackIndex = cyclicIndex;
        }
    else
        stackIndex = (size_t)(cyclicIndex + 10 * m_stack.size ());
    stackIndex = stackIndex % m_stack.size ();
    return m_points[m_stack[stackIndex]];
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
double SignFromStackTip (size_t pointIndex, double sign) {
    DPoint3d xyzA = CyclicStackPoint (-2);
    DPoint3d xyzB = CyclicStackPoint (-1);
    DPoint3d xyzC = m_points[pointIndex];
    return sign * Cross (xyzA, xyzB, xyzC) >= 0.0 ? 1.0 : -1.0;
    }

// Test of xyz is in the convex region bounded by stack points:
//   polygon[i0]..polygon[i1]
//   polygon[j0]..polygon[j1]
//   polygon[i0]..polygon[i1]
// with "inside" controlled by sign multiplier.
bool IsConvexContinuation (DPoint3dCR xyz, size_t i0, size_t i1, size_t j0, size_t j1, double sign)
    {
    return IsInsideTurn (m_points[m_stack[i0]], m_points[m_stack[i1]], xyz, sign)
        && IsInsideTurn (m_points[m_stack[i0]], m_points[m_stack[j0]], xyz, sign)
        && IsInsideTurn (m_points[m_stack[j1]], m_points[m_stack[i1]], xyz, sign);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
size_t IndexAfter (size_t i) {return (i + 1) % m_numPoints;}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
size_t IndexBefore (size_t i) {return (i + m_numPoints - 1) % m_numPoints;}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
AlternatingConvexClipTreeBuilder (bvector<DPoint3d> const &points)
    {
    m_points = points;
    if (PolygonOps::AreaXY (m_points) < 0.0)
        std::reverse (m_points.begin (), m_points.end ());
    m_numPoints = points.size ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
size_t IndexOfMaxX ()
    {
    size_t k = 0;
    for (size_t i = 1; i < m_numPoints; i++)
        if (m_points[i].x > m_points[k].x)
            k = i;
    return k;
    }

// pop from the stack until sign condition is satisfied.
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ExtendHullChain (size_t k, double sign, bool pushAfterPops)
    {
    while (m_stack.size () > 1 && SignFromStackTip (k, sign) < 0.0)
        m_stack.pop_back ();
    if (pushAfterPops)
        PushIndex (k);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void CollectHullChain (size_t kStart, size_t numK, double sign)
    {
    m_stack.clear ();
    if (numK > 2)
        {
        for (size_t i = 0, k = kStart; i < numK; i++, k = IndexAfter (k))
            ExtendHullChain (k, sign, true);
        }
    }

// Input a ClipTreeRoot that has start and count data.
// Build the hull for that data range.
// Store the hull points in the root.
// Add children with start and count data.
// Recurse to children.
bool BuildHullTree_go (AlternatingConvexClipTreeNode &root)
    {
    CollectHullChain (root.m_startIndex, root.m_numPoints, root.m_isPositiveArea ? 1.0 : -1.0);
    root.m_points.clear ();
    for (size_t i = 0; i < m_stack.size (); i++)
        {
        size_t k0 = m_stack[i];
        root.m_points.push_back (m_points[k0]);
        if (i + 1 < m_stack.size ())
            {
            size_t k1 = m_stack[i + 1];
            if (k1 == IndexAfter (k0))
                {
                // two original points in sequence -- need a clip plane right here!!!
                auto plane = ClipPlane::FromEdgeAndUpVector (m_points[k0], m_points[k1], DVec3d::From (0,0,1), Angle::FromRadians (0));
                if (plane.IsValid ())
                    {
                    auto p = plane.Value ();
                    if (root.m_isPositiveArea)  // Why is the clip plane backwards?
                        p.Negate ();
                    root.AddPlane (p);
                    }
                }
            else
                {
                if (k1 < k0)
                    k1 += Period ();
                root.AddEmptyChild (k0, k1 - k0 + 1);
                }
            }
        }
    for (auto &child : root.m_children)
        {
        BuildHullTree_go (child);
        }
    return true;    // ?? Are there failure modes?  What happens with crossing data?
    }

public:
bool BuildHullTree (AlternatingConvexClipTreeNode &root)
    {
    root.InitialzeWithIndices (IndexOfMaxX (), Period () + 1, true);
    return BuildHullTree_go (root);
    }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlternatingConvexClipTreeNode::CreateTreeForPolygon
(
bvector<DPoint3d> const &points,
AlternatingConvexClipTreeNode &root
)
    {
    AlternatingConvexClipTreeBuilder builder (points);
    return builder.BuildHullTree (root);
    }

END_BENTLEY_GEOMETRY_NAMESPACE