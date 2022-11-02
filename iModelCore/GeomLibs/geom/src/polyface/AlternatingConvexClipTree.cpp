/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AlternatingConvexClipTreeNode::InitialzeWithIndices (size_t index0, size_t numPoint)
    {
    m_startIndex = index0;
    m_numPoints = numPoint;
    m_children.clear ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AlternatingConvexClipTreeNode::AddEmptyChild (size_t index0, size_t numPoint)
    {
    m_children.push_back (AlternatingConvexClipTreeNode ());
    m_children.back ().InitialzeWithIndices (index0, numPoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AlternatingConvexClipTreeNode::AddPlane (ClipPlaneCR plane)
    {
    m_planes.Add (plane);
    }
/*---------------------------------------------------------------------------------**//**
* Recursive search with alternating in and out semantics.
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AlternatingConvexClipTreeNode::CaptureConvexClipPlaneSetAsVoid (AlternatingConvexClipTreeNode &child)
    {
    m_children.push_back (AlternatingConvexClipTreeNode ());
    std::swap (child, m_children.back ());
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PushIndex (size_t primaryPointIndex)
    {
    m_stack.push_back (primaryPointIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsInsideTurn (DPoint3dCR xyzA, DPoint3dCR xyzB, DPoint3dCR xyzC, double sign)
    {
    return sign * Cross(xyzA, xyzB, xyzC) > 0;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double Cross (DPoint3dCR xyzA, DPoint3dCR xyzB, DPoint3dCR xyzC)
    {
    return xyzA.CrossProductToPointsXY (xyzB, xyzC);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Period () const { return m_points.size ();}
/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t IndexAfter (size_t i) {return (i + 1) % m_numPoints;}
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t IndexBefore (size_t i) {return (i + m_numPoints - 1) % m_numPoints;}
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AlternatingConvexClipTreeBuilder (bvector<DPoint3d> const &points)
    {
    m_points = points;
    if (PolygonOps::AreaXY (m_points) < 0.0)
        std::reverse (m_points.begin (), m_points.end ());
    m_numPoints = points.size ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ExtendHullChain (size_t k, double sign, bool pushAfterPops)
    {
    while (m_stack.size () > 1 && SignFromStackTip (k, sign) < 0.0)
        m_stack.pop_back ();
    if (pushAfterPops)
        PushIndex (k);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
bool BuildHullTree_go (AlternatingConvexClipTreeNode &root, bool thisLevelIsPositiveArea)
    {
    CollectHullChain (root.m_startIndex, root.m_numPoints, thisLevelIsPositiveArea ? 1.0 : -1.0);
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
                    if (thisLevelIsPositiveArea)  // Why is the clip plane backwards?
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
        BuildHullTree_go (child, !thisLevelIsPositiveArea);
        }
    return true;    // ?? Are there failure modes?  What happens with crossing data?
    }

public:
bool BuildHullTree (AlternatingConvexClipTreeNode &root)
    {
    root.InitialzeWithIndices (IndexOfMaxX (), Period () + 1);
    return BuildHullTree_go (root, true);
    }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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

struct AlternatingConvexClipTreeNode__CurveClipper
{
public:
AlternatingConvexClipTreeNode__CurveClipper ()
    {
    m_stackDepth = 0;
    }
private:
ICurvePrimitiveCP m_curve;
void SetCurve (ICurvePrimitiveCP curve) {m_curve = curve;}
// the interval stack can have multiple segments at each level.
// when reused over multiple curves, do NOT "clear" the stack -- that would deallocate all the memory.
// instead, set the stack counter to zero.  Each level can then "clear" and reuse on later curves.
bvector<bvector<DRange1d> >m_intervalStack;
bvector<DRange1d> m_work;
size_t m_stackDepth;


void PopSegmentFrame ()
    {
    BeAssert (m_stackDepth > 0);
    if (m_stackDepth > 0)
        {
        TopOfStack().clear ();        // formality.
        m_stackDepth -= 1;
        }
    }

void ClearSegmentStack (){
    while (m_stackDepth > 0)
        PopSegmentFrame ();       // and that will reduce stack depth.
    }

void PushEmptySegmentFrame ()
    {
    m_stackDepth += 1;
    while (m_intervalStack.size () < m_stackDepth)
        m_intervalStack.push_back (bvector<DRange1d> ());
    TopOfStack ().clear ();
    }

bvector<DRange1d> &TopOfStack () {return m_intervalStack[m_stackDepth - 1];}
// access enry [TopOfStack() - numSkip}
bvector<DRange1d> &StackEntry (size_t numSkip)
    {
    BeAssert (numSkip <= m_stackDepth);
    return m_intervalStack[m_stackDepth - 1 - numSkip];
    }
bool IsTopOfStackEmpty () { return TopOfStack ().size () == 0;}

bvector<DSegment1d> m_fractionIntervals;  // for reuse in AppendSingleClipToStack (which is not recursive)
bool AppendSingleClipToStack(ConvexClipPlaneSet const &planes, bvector<DRange1d> &insideSegments)
    {
    DSegment3d segment;
    DEllipse3d arc;
    bvector<DPoint3d> const *ls;
    if (m_curve->TryGetLine (segment))
        {
        double f0, f1;
        if (planes.ClipBoundedSegment (segment.point[0], segment.point[1], f0, f1, -1.0))
            insideSegments.push_back (DRange1d (f0, f1));
        }
    else if (m_curve->TryGetArc (arc))
        {
        m_fractionIntervals.clear ();
        planes.AppendIntervals (arc, &m_fractionIntervals, -1.0);
        for (auto &s : m_fractionIntervals)
            insideSegments.push_back (DRange1d (s.GetStart (), s.GetEnd ()));
        }
    else if ((ls = m_curve->GetLineStringCP ()) != nullptr
        && ls->size () > 1)
        {
        double df = 1.0 / (double)(ls->size () - 1);
        for (int i = 0; m_curve->TryGetSegmentInLineString (segment, i); i++)
            {
            double f0, f1;
            if (planes.ClipBoundedSegment (segment.point[0], segment.point[1], f0, f1, -1.0))
                insideSegments.push_back (DRange1d ((i + f0) * df, (i + f1) * df));
            }
        }
    else
        {
        auto bcurve = m_curve->GetProxyBsplineCurveCP ();
        if (bcurve != nullptr)
            {
            m_fractionIntervals.clear ();
            planes.AppendIntervals (*bcurve, &m_fractionIntervals);
            for (auto &s : m_fractionIntervals)
                insideSegments.push_back (DRange1d (s.GetStart (), s.GetEnd ()));
            }
        }
    return false;
    }
// run one level or recursion.  On return, the stack is one level deeper than at entry and the new top of stack has clip for this node.
void Recurse (AlternatingConvexClipTreeNode const &node)
    {
    PushEmptySegmentFrame ();
    AppendSingleClipToStack (node.m_planes, TopOfStack ());
    DRange1d::SortLowInPlace (TopOfStack());
    if (IsTopOfStackEmpty ())
        return;
    for (auto &child : node.m_children)
        {
        Recurse (child);
        if (!IsTopOfStackEmpty ())
            {
            DRange1d::DifferenceSorted (StackEntry (1), StackEntry (0), m_work);
            PopSegmentFrame ();
            TopOfStack ().swap (m_work);
            }
        else
            PopSegmentFrame ();
        if (IsTopOfStackEmpty ())
            break;
        }
    }
public:
void AppendSinglePrimitiveClip (
AlternatingConvexClipTreeNode const &root,
ICurvePrimitiveCR curve, 
bvector<CurveLocationDetailPair> *insideIntervals,
bvector<CurveLocationDetailPair> *outsideIntervals
)
    {
    SetCurve (&curve);
    ClearSegmentStack ();
    Recurse (root);
    BeAssert (m_stackDepth == 1);

    if (insideIntervals)
        {
        auto & intervals = TopOfStack ();
        for (auto &interval : intervals)
            {
            DPoint3d xyz0, xyz1;
            double f0 = interval.low;
            double f1 = interval.high;
            curve.FractionToPoint (f0, xyz0);
            curve.FractionToPoint (f1, xyz1);
            insideIntervals->push_back (
                CurveLocationDetailPair (
                    CurveLocationDetail (&curve, f0, xyz0),
                    CurveLocationDetail (&curve, f1, xyz1)
                    )); 
            }
        }
    PopSegmentFrame ();
    }

void AppendCurveVectorClip
(
AlternatingConvexClipTreeNode const &root,
CurveVectorCR curves,
bvector<CurveLocationDetailPair> *insideIntervals,
bvector<CurveLocationDetailPair> *outsideIntervals
)
    {
    for (auto &cp : curves)
        {
        auto child = cp->GetChildCurveVectorCP ();
        if (child != nullptr)
            AppendCurveVectorClip (root, *child, insideIntervals, outsideIntervals);
        else
            AppendSinglePrimitiveClip (root, *cp, insideIntervals, outsideIntervals);
        }
    }
};

void AlternatingConvexClipTreeNode::AppendCurvePrimitiveClipIntervals
(
ICurvePrimitiveCR curve,
bvector<CurveLocationDetailPair> *insideIntervals,
bvector<CurveLocationDetailPair> *outsideIntervals
)
    {
    AlternatingConvexClipTreeNode__CurveClipper clipper;
    clipper.AppendSinglePrimitiveClip (*this, curve, insideIntervals, outsideIntervals);
    }
//! Append start-end positions for curve intervals classified as inside or outside.
void AlternatingConvexClipTreeNode::AppendCurveVectorClipIntervals
(
CurveVectorCR curves,
bvector<CurveLocationDetailPair> *insideIntervals,
bvector<CurveLocationDetailPair> *outsideIntervals
)
    {
    AlternatingConvexClipTreeNode__CurveClipper clipper;
    clipper.AppendCurveVectorClip (*this, curves, insideIntervals, outsideIntervals);
    }

END_BENTLEY_GEOMETRY_NAMESPACE