/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveVector::SwapAt (size_t index0, size_t index1)
    {
    size_t n = size ();
    if (index0 >= n || index1 >= n)
        return false;
    ICurvePrimitivePtr value0 = at(index0);
    at (index0) = at (index1);
    at (index1) = value0;
    return true;
    }

//! Confirm that specified child is a CurveVector and set its boundary type.
bool CurveVector::SetChildBoundaryType (size_t index, CurveVector::BoundaryType boundaryType)
    {
    size_t n = size ();
    if (index >= n)
        return false;
    ICurvePrimitivePtr childPrimitive = at(index);
    CurveVectorP childVector = const_cast<CurveVectorP>(childPrimitive->GetChildCurveVectorCP ());
    if (NULL != childVector)
        {
        childVector->SetBoundaryType (boundaryType);
        return true;
        }
    return false;
    }

//! Confirm that specified child is a CurveVector and get its boundary type.
bool CurveVector::GetChildBoundaryType (size_t index, CurveVector::BoundaryType &boundaryType) const
    {
    size_t n = size ();
    if (index >= n)
        return false;
    ICurvePrimitivePtr childPrimitive = at(index);
    CurveVectorCP childVector = childPrimitive->GetChildCurveVectorCP ();
    if (NULL != childVector)
        {
        boundaryType = childVector->GetBoundaryType ();
        return true;
        }
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void CurveVector::AnnounceKeyPoints
(
DPoint3dCR spacePoint,                       //!< [in] point to project
CurveKeyPointCollector &collector,           //!< [in] object to receive keypoint announcements
bool extend0,                               //!< true to extend at start of primitives.
bool extend1                                //!< true to extend at end of primitives.
) const
    {
    if (GetBoundaryType () == CurveVector::BOUNDARY_TYPE_Open)
        for (size_t i = 0, n = size (); i < n; i++) // Only first and last members can be extended.
            at(i)->AnnounceKeyPoints (spacePoint, collector, extend0 && i == 0, extend1 && (i + 1 == n));
    else if (GetBoundaryType () == CurveVector::BOUNDARY_TYPE_None) // each member receives primary extend0,extend1 unchanged
        for (size_t i = 0, n = size (); i < n; i++)
            at(i)->AnnounceKeyPoints (spacePoint, collector, extend0, extend1);
    else
        {
        // everything else contains only loops
        for (size_t i = 0, n = size (); i < n; i++)
            at(i)->AnnounceKeyPoints (spacePoint, collector, false, false);
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveVector::ClosestPointBounded (DPoint3dCR spacePoint, CurveLocationDetailR location, bool extend0, bool extend1) const
    {
    bool stat = false;
    location = CurveLocationDetail ();
    location.SetMaxDistance ();    
    for (size_t i = 0, n = size (); i < n; i++)
        {
        CurveLocationDetail candidate;
        if (at(i)->ClosestPointBounded (spacePoint, candidate, extend0, extend1))
            {
            location.UpdateIfCloser(candidate);
            stat = true;
            }
        }
    return stat;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveVector::ClosestPointBounded (DPoint3dCR spacePoint, CurveLocationDetailR location) const
    {
    return ClosestPointBounded (spacePoint, location, false, false);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveVector::ClosestPointBoundedXY (DPoint3dCR spacePoint, DMatrix4dCP worldToLocal, CurveLocationDetailR location) const
    {
    return ClosestPointBoundedXY (spacePoint, worldToLocal, location, false, false);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveVector::ClosestPointBoundedXY (DPoint3dCR spacePoint, DMatrix4dCP worldToLocal, CurveLocationDetailR location, bool extend0, bool extend1) const
    {
    bool stat = false;
    location = CurveLocationDetail ();
    location.SetMaxDistance ();
    int extensionPolicy = 0;
    if (IsAnyRegionType ())
        {
        extensionPolicy = 0;    // no extension possible.
        }
    else if (IsOpenPath ())
        {
        extensionPolicy = 1;   // extend0, extend1 apply only to left of first, right of last.
        }
    else
        {
        extensionPolicy = 2;    // extend0, extend1 apply to each fragment.
        }

    for (size_t i = 0, n = size (); i < n; i++)
        {
        CurveLocationDetail candidate;
        bool extendA = false;
        bool extendB = false;
        if (extensionPolicy == 1)
            {
            if (i == 0)
                extendA = extend0;
            if (i + 1 == n)
                extendB = extend1;
            }
        else if (extensionPolicy == 2)
            {
            extendA = extend0;
            extendB = extend1;
            }

        if (at(i)->ClosestPointBoundedXY (spacePoint, worldToLocal, candidate, extendA, extendB))
            {
            location.UpdateIfCloser (candidate);
            stat = true;
            }
        }
    return stat;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveVector::GetStartEnd (DPoint3dR pointA, DPoint3dR pointB) const
    {
    bool anyPointFound = false;
    DPoint3d pointA1, pointB1;
    pointA.Zero ();
    pointB.Zero ();
    for (size_t i = 0, n = size (); i < n; i++)
        {
        if (at(i)->GetStartEnd (pointA1, pointB1))
            {
            if (anyPointFound)
                {
                pointB = pointB1;
                }
            else
                {
                pointA = pointA1;
                pointB = pointB1;
                anyPointFound = true;
                }
            }
        }
    return anyPointFound;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveVector::GetStartEnd (DPoint3dR pointA, DPoint3dR pointB, DVec3dR unitTangentA, DVec3dR unitTangentB) const
    {
    bool anyPointFound = false;
    DPoint3d pointA1, pointB1;
    DVec3d   unitTangentA1, unitTangentB1;
    pointA.Zero ();
    pointB.Zero ();
    unitTangentA1.Zero ();
    unitTangentB1.Zero ();
    for (size_t i = 0, n = size (); i < n; i++)
        {
        if (at(i)->GetStartEnd (pointA1, pointB1, unitTangentA1, unitTangentB1))
            {
            if (anyPointFound)
                {
                pointB = pointB1;
                unitTangentB = unitTangentB1;
                }
            else
                {
                pointA = pointA1;
                pointB = pointB1;
                unitTangentA = unitTangentA1;
                unitTangentB = unitTangentB1;
                anyPointFound = true;
                }
            }
        }
    return anyPointFound;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveVector::GetStartEnd (CurveLocationDetailR pointA, CurveLocationDetailR pointB) const
    {
    // hmph .. just look at front and back, don't fuss with recursion . . .
    if (size () < 1)
        return false;
    return front ()->FractionToPoint (0.0, pointA)
        && back()->FractionToPoint (1.0, pointB);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
ICurvePrimitive::CurvePrimitiveType CurveVector::HasSingleCurvePrimitive () const
    {
    if (1 != size ())
        return ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Invalid;

    ICurvePrimitivePtr const& curve = front ();

    if (curve.IsNull ())
        return ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Invalid;

    return curve->GetCurvePrimitiveType ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t CurveVector::CountPrimitivesOfType (ICurvePrimitive::CurvePrimitiveType targetType) const
    {
    size_t count = 0;
    for (size_t i = 0, n = size (); i < n; i++)
        {
        if (at(i)->GetCurvePrimitiveType () == targetType)
            count++;
        CurveVectorCP childVector = at(i)->GetChildCurveVectorCP ();
        if (NULL != childVector)
            count += childVector->CountPrimitivesOfType (targetType);
        }
    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double CurveVector::Length () const
    {
    double sum = 0.0;
    for (size_t i = 0, n = size (); i < n; i++)
        {
        double a;
        if (at(i)->Length (a))
            sum += a;
        }
    return sum;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double CurveVector::Length (RotMatrixCP worldToLocal) const
    {
    double sum = 0.0;
    for (size_t i = 0, n = size (); i < n; i++)
        {
        double a;
        if (at(i)->Length (worldToLocal, a))
            sum += a;
        }
    return sum;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double CurveVector::FastLength () const
    {
    double sum = 0.0;
    for (size_t i = 0, n = size (); i < n; i++)
        {
        double a;
        if (at(i)->FastLength (a))
            sum += a;
        }
    return sum;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveVector::TransformInPlace (TransformCR transform)
    {
    bool ok = true;
    for (size_t i = 0, n = size (); i < n; i++)
        {
        if (at(i).IsValid() && !at(i)->TransformInPlace (transform))
            ok = false;
        }
    return ok;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveVector::ReverseCurvesInPlace ()
    {
    BoundaryType btype = GetBoundaryType ();
    bool reverseOrder = false;
    bool recurse = true;
    bool stat = true;
    if (   btype == BOUNDARY_TYPE_Outer
        || btype == BOUNDARY_TYPE_Inner
        || btype == BOUNDARY_TYPE_Open
        )
        reverseOrder = true;

    size_t n = size ();
    if (reverseOrder && n > 1)
        {
        for (size_t i = 0, j = n - 1; i < j; i++, j--)
            std::swap (at(i), at(j));
        }
    if (recurse)
        {
        for (size_t i = 0; i < n; i++)
            stat &= at(i)->ReverseCurvesInPlace ();
        }

    return stat;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void CurveVector::SetBoundaryType (CurveVector::BoundaryType boundaryType) {m_boundaryType = boundaryType;}
CurveVector::BoundaryType CurveVector::GetBoundaryType () const {return m_boundaryType;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveVector::IsOpenPath () const {return (CurveVector::BOUNDARY_TYPE_Open == m_boundaryType);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveVector::IsClosedPath () const
    {return (CurveVector::BOUNDARY_TYPE_Outer == m_boundaryType || CurveVector::BOUNDARY_TYPE_Inner == m_boundaryType);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveVector::IsPhysicallyClosedPath () const
    {
    DPoint3d pointA, pointB;
    if (CurveVector::BOUNDARY_TYPE_Open == m_boundaryType
        && GetStartEnd (pointA, pointB))
        {
        return pointA.AlmostEqual (pointB);
        }
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveVector::AreStartEndAlmostEqual () const
    {
    DPoint3d pointA, pointB;
    if (GetStartEnd (pointA, pointB))
        return pointA.AlmostEqual (pointB);
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool            CurveVector::IsParityRegion () const
    {
    return (CurveVector::BOUNDARY_TYPE_ParityRegion == m_boundaryType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool            CurveVector::IsUnionRegion () const
    {
    return (CurveVector::BOUNDARY_TYPE_UnionRegion == m_boundaryType);
    }

/*---------------------------------------------------------------------------------**//**
+--------------------------------------------------------------------------------------*/
bool            CurveVector::IsAnyRegionType () const
    {
    return CurveVector::BOUNDARY_TYPE_Outer        == m_boundaryType
        || CurveVector::BOUNDARY_TYPE_Inner        == m_boundaryType
        || CurveVector::BOUNDARY_TYPE_UnionRegion  == m_boundaryType
        || CurveVector::BOUNDARY_TYPE_ParityRegion == m_boundaryType;
    }


/*---------------------------------------------------------------------------------**//**
+--------------------------------------------------------------------------------------*/
bool            CurveVector::IsEllipticDisk (DEllipse3dR ellipse) const
    {
    if (m_boundaryType != CurveVector::BOUNDARY_TYPE_Outer
        || size () != 1)
        return false;
    if (at(0)->TryGetArc (ellipse))
        return true;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
+--------------------------------------------------------------------------------------*/
bool            CurveVector::IsRectangle(TransformR localToWorld, TransformR worldToLocal) const
    {
    if (   m_boundaryType == CurveVector::BOUNDARY_TYPE_Outer
        || m_boundaryType == CurveVector::BOUNDARY_TYPE_Inner
        || m_boundaryType == CurveVector::BOUNDARY_TYPE_Open
        )
        {
        if (size () == 1)
            {
            bvector<DPoint3d> const *points = at(0)->_GetLineStringCP ();
            if (points != nullptr)
                return PolylineOps::IsRectangle (*points,
                    localToWorld, worldToLocal, true);
            }
        }
    localToWorld.InitIdentity ();
    worldToLocal.InitIdentity ();
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurveVectorPtr    CurveVector::Create (CurveVector::BoundaryType boundaryType)
    {
    return new CurveVector (boundaryType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurveVectorPtr CurveVector::Create (BoundaryType boundaryType, bvector<ICurvePrimitivePtr> primitives)
    {
    auto result = Create (boundaryType);
    for (auto &p : primitives)
        result->push_back (p);
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurveVectorPtr    CurveVector::Create (CurveVector::BoundaryType boundaryType, ICurvePrimitivePtr primitive)
    {
    CurveVectorPtr      curveVector = new CurveVector (boundaryType);

    curveVector->push_back (primitive);

    return curveVector;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitivePtr CurveVector::FindPrimitiveById (CurvePrimitiveIdCR id) const
    {
    for (ICurvePrimitivePtr const& curvePrimitive : *this)
        {
        if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector == curvePrimitive->GetCurvePrimitiveType ())
            {
            ICurvePrimitivePtr childPrimitive = curvePrimitive->GetChildCurveVectorCP()->FindPrimitiveById(id);
            if (childPrimitive.IsValid ())
                return childPrimitive;
            }

        if (NULL != curvePrimitive->GetId() && *curvePrimitive->GetId () == id)
            return curvePrimitive;
        }

    return ICurvePrimitivePtr ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t CurveVector::AddPrimitives (CurveVectorCR source)
    {
    size_t n = 0;
    for (size_t i = 0; i < source.size (); i++)
        {
        if (source.at(i).IsValid ())
            {
            CurveVectorCP child = source.at(i)->GetChildCurveVectorCP ();
            if (NULL != child)
                {
                n += AddPrimitives (*child);
                }
            else
                {
                n += 1;
                push_back (source.at(i));
                }
            }
        }
    return n;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void CurveVector::Add (ICurvePrimitivePtr child)
    {
    push_back (child);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void CurveVector::Add (CurveVectorPtr child)
    {
    push_back (ICurvePrimitive::CreateChildCurveVector (child));
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr CurveVector::GetCyclic (ptrdiff_t index) const
    {
    if (size () == 0)
        return NULL;
    return at(CyclicIndex ((int)index));
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurveVectorPtr CurveVector::Stroke (IFacetOptionsR options) const
    {
    CurveVectorPtr strokedCV = Create (m_boundaryType);
    switch (GetBoundaryType ())
        {
        case CurveVector::BOUNDARY_TYPE_Open:
        case CurveVector::BOUNDARY_TYPE_Outer:
        case CurveVector::BOUNDARY_TYPE_Inner:
            {
            bvector <DPoint3d> dest;
            for (size_t i = 0, n = size (); i < n; i++)
                at(i)->AddStrokes (dest, options, dest.size () > 0 ? false : true);
            strokedCV->push_back (ICurvePrimitive::CreateLineString (dest));

            break;
            }

        case CurveVector::BOUNDARY_TYPE_ParityRegion:
        case CurveVector::BOUNDARY_TYPE_UnionRegion:
            {
            for (size_t i = 0, n = size (); i < n; i++)
                strokedCV->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (
                                            *at(i)->GetChildCurveVectorCP ()->Stroke (options)));

            break;
            }

        default:
            return NULL;
        }

    return strokedCV;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void CurveVector::AddStrokePoints (bvector <DPoint3d> &points, IFacetOptionsR options) const
    {
    CurveVectorCP   child;
    bool            restart = true;
    bool            addDisconnects = BOUNDARY_TYPE_None == GetBoundaryType();

    for (size_t i = 0, n = size (); i < n; i++)
        {
        if (addDisconnects && i > 0)
            {
            DPoint3d        disconnect;

            disconnect.InitDisconnect ();
            points.push_back (disconnect);
            }

        if (at(i)->AddStrokes (points, options, restart, 0.0, 1.0))
            {
            restart = false;
            }
        else if (NULL != (child = at(i)->GetChildCurveVectorCP ()))
            {
            child->AddStrokePoints (points, options);
            restart = true;
            }
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void CurveVector::AddStrokePoints (bvector<DPoint3dDoubleUVCurveArrays> &points, IFacetOptionsP optionsIn) const
    {
    IFacetOptionsPtr options = optionsIn;
    if (!options.IsValid ())
        options = IFacetOptions::CreateForCurves ();

    CurveVectorCP   child;
    if (IsOpenPath () || IsClosedPath ())
        {
        points.push_back (DPoint3dDoubleUVCurveArrays ());
        for (size_t i = 0, n = size (); i < n; i++)
            {
            at(i)->AddStrokes (points.back (), *options, 0.0, 1.0);
            }
        }
    else
        {
        for (size_t i = 0, n = size (); i < n; i++)
            {
            if (NULL != (child = at(i)->GetChildCurveVectorCP ()))
                {
                child->AddStrokePoints (points, options.get ());
                }
            else 
                {
                points.push_back (DPoint3dDoubleUVCurveArrays ());
                at(i)->AddStrokes (points.back (), *options, 0.0, 1.0);
                }
            }
        }
    }



struct LinearGeometryCollector
{
size_t errors;
IFacetOptionsPtr m_facetOptions;
private:
void CreateFacetOptions ()
    {
    if (!m_facetOptions.IsValid ())
        m_facetOptions = IFacetOptions::CreateForCurves ();
    }
public:
LinearGeometryCollector (IFacetOptionsP options)
    : m_facetOptions(options)
    {
    errors = 0;
    }

size_t NumErrors () {return errors;}

// Return false if anything other than Line or LineString
bool CollectPointsFromSinglePrimitive
(
ICurvePrimitiveCR prim,
bvector<DPoint3d> &path
)
    {
    if (m_facetOptions.IsValid ())
        {
        size_t initialCount = path.size ();
        prim.AddStrokes (path, *m_facetOptions, true);
        PolylineOps::PackAlmostEqualAfter (path, initialCount);
        return true;
        }
    // No options given .. only take points.
    DSegment3d segment;
    auto linestringPoints = prim.GetLineStringCP ();
    if (nullptr != linestringPoints)
        {
        for (DPoint3d xyz : *linestringPoints)
            {
            PolylineOps::AddPointIfDistinctFromBack (path, xyz);
            }
        }
    else if (prim.TryGetLine (segment))
        {
        PolylineOps::AddPointIfDistinctFromBack (path, segment.point[0]);
        PolylineOps::AddPointIfDistinctFromBack (path, segment.point[1]);
        }
    else
        {
        errors++;
        }
    return true;
    }

// Return false if anything other than Line or LineString
bool CollectPointsFromSinglePrimitive
(
ICurvePrimitiveCR prim,
bvector<bvector<DPoint3d>> &paths
)
    {
    paths.push_back (bvector<DPoint3d> ());
    return CollectPointsFromSinglePrimitive (prim, paths.back ());
    }

// Return false if anything other than Line or LineString
bool CollectPointsFromSinglePrimitive
(
ICurvePrimitiveCR prim,
bvector<bvector<bvector<DPoint3d>>> &paths
)
    {
    paths.push_back (bvector<bvector<DPoint3d>> ());
    return CollectPointsFromSinglePrimitive (prim, paths.back ());
    }

void CollectPointsFromLinearPrimitives
(
CurveVectorCR curves,
bvector<bvector<DPoint3d>> &paths
)
    {
    if (curves.IsOpenPath () || curves.IsClosedPath ())
        {
        paths.push_back (bvector<DPoint3d> ());
        auto &newPath = paths.back ();
        for (auto &prim : curves)
            {
            CollectPointsFromSinglePrimitive (*prim, newPath);
            }
        if (curves.IsClosedPath ())
            PolylineOps::EnforceClosure (newPath);
        }
    else
        {
        // We could be in  . ..
        // Union
        // Parity
        // None
        // keep trying .. all loops from any depth show up as peers in the path array.
        // leaves not wrapped as loops show up as trivial paths.
        for (auto &prim : curves)
            {
            auto child = prim->GetChildCurveVectorCP ();
            if (nullptr != child)
                CollectPointsFromLinearPrimitives (*child, paths);
            else
                CollectPointsFromSinglePrimitive (*prim, paths);
            }
        }
    }



void CollectPointsFromLinearPrimitives
(
CurveVectorCR curves,
bvector<bvector<bvector<DPoint3d>>> &paths,
bool inParityRegion
)
    {
    if (curves.IsUnionRegion ())
        {
        // Each child will become a new top level region
        for (auto &prim : curves)
            {
            CollectPointsFromLinearPrimitives (*prim->GetChildCurveVectorP (), paths, false);
            }
        }
    else if (curves.IsParityRegion ())
        {
        paths.push_back (bvector<bvector<DPoint3d>>());
        for (auto &prim : curves)
            {
            CollectPointsFromLinearPrimitives (*prim->GetChildCurveVectorP (), paths, true);
            }
        }
    else if (curves.IsOpenPath () || curves.IsClosedPath ())
        {
        // If in a parity region, the caller always created an empty bvector of loops at the back.
        // If not, create a new one
        if (!inParityRegion)
            paths.push_back (bvector<bvector<DPoint3d>> ());
        auto &parityLoops = paths.back ();
        parityLoops.push_back (bvector<DPoint3d> ());
        auto &loop = parityLoops.back ();
        for (auto &prim : curves)
            if (!CollectPointsFromSinglePrimitive (*prim, loop))
                errors++;
        if (curves.IsClosedPath ())
            PolylineOps::EnforceClosure (loop);
        }
    else if (curves.GetBoundaryType () == CurveVector::BOUNDARY_TYPE_None)
        {
        // Anything (single prim, open, closed, parity, union) is possible . ..
        for (auto &prim : curves)
            {
            auto child = prim->GetChildCurveVectorCP ();
            if (nullptr != child)
                CollectPointsFromLinearPrimitives (*child, paths, false);
            else
                CollectPointsFromSinglePrimitive (*prim, paths);
            }
        }
    else
        errors++;
    }
};

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveVector::CollectLinearGeometry (bvector <bvector<bvector<DPoint3d>>> &regionPoints, IFacetOptionsP strokeOptions) const
    {
    LinearGeometryCollector collector (strokeOptions);
    regionPoints.clear ();
    collector.CollectPointsFromLinearPrimitives (*this, regionPoints, false);
    return collector.NumErrors () == 0;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveVector::CollectLinearGeometry (bvector<bvector<DPoint3d>> &regionPoints, IFacetOptionsP strokeOptions) const
    {
    LinearGeometryCollector collector (strokeOptions);
    regionPoints.clear ();
    collector.CollectPointsFromLinearPrimitives (*this, regionPoints);
    return collector.NumErrors () == 0;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
MSBsplineCurvePtr CurveVector::GetBsplineCurve () const
    {
    MSBsplineCurve curve;
    if (SUCCESS == ToBsplineCurve (curve))
        return curve.CreateCapture ();
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus   CurveVector::ToBsplineCurve (MSBsplineCurveR curve) const
    {
    if (!IsOpenPath () && !IsClosedPath ())
        return ERROR;

    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Invalid != HasSingleCurvePrimitive ())
        {
        if (!front ()->GetMSBsplineCurve (curve))
            return ERROR;
        }
    else
        {
        double  segmentParam = 0.0;

        memset (&curve, 0, sizeof (curve));

        for (ICurvePrimitivePtr curvePrimitive: *this)
            {
            if (curvePrimitive.IsNull ())
                continue;

            MSBsplineCurve  segment;
            
            if (!curvePrimitive->GetMSBsplineCurve (segment))
                {
                curve.ReleaseMem ();

                return ERROR;
                }

            if (0 == curve.params.numPoles)
                {
                curve = segment;
                }
            else
                {
                bspknot_scaleKnotVector (curve.knots, &curve.params, segmentParam += 1.0);

                if (SUCCESS != curve.AppendCurves (curve, segment, false, false))
                    {
                    segment.ReleaseMem ();
                    curve.ReleaseMem ();

                    return ERROR;
                    }

                // TR #57119: Revalidate knots: we may have just created an oversaturated start/end knot
                mdlBspline_validateCurveKnots (curve.knots, curve.poles, curve.weights, &curve.params);
                }
            }
        }

    if (IsClosedPath ())
        curve.MakeClosed ();
    else if(IsOpenPath ()) //TFS#2620-Converting 360 arc to Bspline curve should not create closed curve.
        curve.MakeOpen(0.0);

    return SUCCESS;
    }
struct AddSpacedPointsContext
{
bvector<double> const &m_distances;
bvector<CurveLocationDetail> &m_locations;
size_t m_distanceIndex;
double m_accumulatedDistance;
AddSpacedPointsContext (bvector<double> const &distances, bvector<CurveLocationDetail> &locations)
    : m_distances (distances), m_locations (locations)
    {
    m_distanceIndex = 0;
    m_accumulatedDistance = 0.0;
    }

void Recurse (CurveVectorCR parent)
    {
    if (1 > parent.size ())
        return;
    for (size_t i = 0, n = parent.size (); i < n && m_distanceIndex < m_distances.size (); i++)
        {
        auto child = parent[i]->GetChildCurveVectorCP ();
        if (nullptr != child)
            {
            Recurse (*child);
            }
        else
            {
            double startFraction = 0.0, currentCurveLength;
            CurveLocationDetail location;
        
            if (!parent[i]->Length (currentCurveLength))
                continue;
            double distanceAtEndOfCurve = m_accumulatedDistance + currentCurveLength;
            while (  m_distanceIndex < m_distances.size ()
                  && m_distances[m_distanceIndex] < distanceAtEndOfCurve + mgds_fc_epsilon)
                {
                if (parent[i]->PointAtSignedDistanceFromFraction (startFraction, m_distances[m_distanceIndex] - m_accumulatedDistance, false, location))
                    {
                    location.a = m_distances[m_distanceIndex];
                    m_locations.push_back (location);
                    startFraction = location.fraction;
                    m_accumulatedDistance = m_distances[m_distanceIndex];
                    m_distanceIndex++;
                    }
                else
                    break;
                }
            m_accumulatedDistance = distanceAtEndOfCurve;
            }
        }

    }
};
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool    CurveVector::AddSpacedPoints (bvector<double> const &distances, bvector<CurveLocationDetail> &locations) const
    {
    AddSpacedPointsContext context (distances, locations);
    context.Recurse (*this);
    return locations.size () > 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
ValidatedCurveLocationDetail CurveVector::PointAtSignedDistanceAlong (CurveLocationDetailCR startPoint, double signedDistance) const
    {
    auto result = ValidatedCurveLocationDetail (startPoint, false);
    size_t index;
    if (!LeafToIndex (startPoint.curve, index))
        return result;
    double startFraction = startPoint.fraction;
    if (signedDistance > 0)
        {
        double distanceLeft = signedDistance;
        for (;index < size (); index++, startFraction = 0.0)
            {
            CurveLocationDetail endPoint;
            at(index)->PointAtSignedDistanceFromFraction (startFraction, distanceLeft, false, endPoint);
            if (DoubleOps::AlmostEqual (distanceLeft, endPoint.a))
                {
                endPoint.a = signedDistance;
                return ValidatedCurveLocationDetail (endPoint, true);
                }
            distanceLeft -= endPoint.a;
            }
        }
    else
        {
        double distanceLeft = signedDistance;   // That's negative !!!!
        startFraction = startPoint.fraction;
        for (index++;index-- > 0; startFraction = 1.0)
            {
            CurveLocationDetail endPoint;
            at(index)->PointAtSignedDistanceFromFraction (startFraction, distanceLeft, false, endPoint);
            if (DoubleOps::AlmostEqual (distanceLeft, endPoint.a))
                {
                endPoint.a = signedDistance;
                return ValidatedCurveLocationDetail (endPoint, true);
                }
            distanceLeft -= endPoint.a; // and that moves towards zero
            }
        }
    return result;
    }





/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool    CurveVector::GetRange (DRange3dR range) const
    {
    range.Init ();
    DRange3d curveRange;
    for (size_t i = 0, n = size (); i < n; i++)
        {
        if (at(i)->GetRange (curveRange))
            range.Extend (curveRange);
        }
    return !range.IsNull ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool    CurveVector::GetRange (DRange3dR range, TransformCR transform) const
    {
    range.Init ();
    DRange3d curveRange;
    for (size_t i = 0, n = size (); i < n; i++)
        {
        if (at(i)->GetRange (curveRange, transform))
            range.Extend (curveRange);
        }
    return !range.IsNull ();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double  CurveVector::FastMaxAbs () const
    {
    double a = 0.0;
    for (size_t i = 0, n = size (); i < n; i++)
        {
        DoubleOps::UpdateMax (a, at(i)->FastMaxAbs ());
        }
    return a;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double CurveVector::ResolveTolerance (double tolerance) const
    {
    double a = FastMaxAbs ();
    double e = Angle::SmallAngle ();
    double b = e;
    if (a > 1.0)
        b = a * e;

    if (tolerance < b)
        return b;
    return tolerance;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t CurveVector::CyclicIndex (int index) const
    {
    size_t n = size ();
    if (n == 0)
        return 0;
    if (index < 0)
        {
        size_t m = n - ((-index) % n);
        if (m == n)
            m = 0;
        return m;
        }
    else if (index < (int)n)
        return (size_t) index;
    else
        return (size_t)index % n;
    }


 

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t          CurveVector::CurveLocationDetailIndex (CurveLocationDetail const& location) const
    {
    return FindIndexOfPrimitive (location.curve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t          CurveVector::FindIndexOfPrimitive (ICurvePrimitiveCP primitive) const
    {
    for (size_t iEntry = 0; iEntry < size (); iEntry++)
        {
        ICurvePrimitivePtr const& pathMember = at (iEntry);

        if (!pathMember.IsValid () || pathMember.get () != primitive)
            continue;
        return iEntry;
        }
    return SIZE_MAX; // Not found...
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurveVectorPtr CurveVector::FindParentOfPrimitive (ICurvePrimitiveCP primitive) const
    {
    for (size_t iEntry = 0; iEntry < size (); iEntry++)
        {
        ICurvePrimitivePtr const& pathMember = at (iEntry);
        if (pathMember.get () == primitive)
            return CurveVectorPtr (const_cast<CurveVectorP>(this));
        CurveVectorPtr childCV = pathMember->GetChildCurveVectorP ();
        if (childCV.IsValid ())
            {
            CurveVectorPtr childSearch = childCV->FindParentOfPrimitive (primitive);
            if (childSearch.IsValid ())
                return childSearch;
            }
        }
    return NULL;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
int             CurveVector::CurveLocationDetailCompare (CurveLocationDetail const& location0, CurveLocationDetail const& location1) const
    {
    size_t      index0 = CurveLocationDetailIndex (location0);
    size_t      index1 = CurveLocationDetailIndex (location1);

    if (index0 < index1)
        {
        return -1;
        }
    else if (index0 > index1)
        {
        return 1;
        }
    else
        {
        if (location0.fraction < location1.fraction)
            return -1;
        else if (location0.fraction > location1.fraction)
            return 1;
        else
            return 0;
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurveVectorPtr CurveVector::Create
(
bvector<DSegment3d> const &segments
)
    {
    auto cv = Create (BOUNDARY_TYPE_None);
    for (auto segment : segments)
        {
        cv->Add (ICurvePrimitive::CreateLine (segment));
        }
    return cv;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateRectangle
(
double x0,
double y0,
double x1,
double y1,
double z,
int areaSignPreference
)
    {
    bvector<DPoint3d> points;
    double area = (x1 - x0) * (y1 - y0);
    bool reverse = false;

    if (areaSignPreference > 0)
        reverse = area < 0.0;
    else if (areaSignPreference > 0)
        reverse = area > 0.0;

    points.push_back (DPoint3d::From (x0, y0, z));

    if (reverse)
        {
        points.push_back (DPoint3d::From (x0, y1, z));
        points.push_back (DPoint3d::From (x1, y1, z));
        points.push_back (DPoint3d::From (x1, y0, z));
        }
    else
        {
        points.push_back (DPoint3d::From (x1, y0, z));
        points.push_back (DPoint3d::From (x1, y1, z));
        points.push_back (DPoint3d::From (x0, y1, z));
        }
    points.push_back (DPoint3d::From (x0, y0, z));

    return ICurvePrimitive::CreateLineString (points);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurveVectorPtr CurveVector::CreateRectangle
(
double x0,
double y0,
double x1,
double y1,
double z,
CurveVector::BoundaryType boundaryType
)
    {
    CurveVectorPtr result = CurveVector::Create (boundaryType);
    if (boundaryType == BOUNDARY_TYPE_UnionRegion
        || boundaryType == BOUNDARY_TYPE_ParityRegion)
        {
        CurveVectorPtr child = CurveVector::CreateRectangle (x0, y0, x1, y1, z, BOUNDARY_TYPE_Outer);
        result->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*child));
        }
    else
        {
        int areaSign = 0;
        if (boundaryType == BOUNDARY_TYPE_Outer)
            areaSign = 1;
        else if (boundaryType == BOUNDARY_TYPE_Inner)
            areaSign = -1;
        result->push_back (ICurvePrimitive::CreateRectangle (x0, y0, x1, y1, z, areaSign));
        }
    return result;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurveVectorPtr CurveVector::CreateRegularPolygonXY
(
DPoint3dCR center,
double xDistance,
int numEdge,
bool isOuterRadius,
CurveVector::BoundaryType boundaryType
)
    {
    CurveVectorPtr result = CurveVector::Create (boundaryType);
    if (boundaryType == BOUNDARY_TYPE_UnionRegion
        || boundaryType == BOUNDARY_TYPE_ParityRegion)
        {
        CurveVectorPtr child = CurveVector::CreateRegularPolygonXY (center, xDistance, numEdge, isOuterRadius, BOUNDARY_TYPE_Outer);
        result->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*child));
        }
    else
        {
        int areaSign = 0;
        if (boundaryType == BOUNDARY_TYPE_Outer)
            areaSign = 1;
        else if (boundaryType == BOUNDARY_TYPE_Inner)
            areaSign = -1;
        result->push_back (ICurvePrimitive::CreateRegularPolygonXY (center, xDistance, numEdge, isOuterRadius, areaSign));
        }
    return result;
    }

ICurvePrimitivePtr ICurvePrimitive::CreateRegularPolygonXY
(
DPoint3dCR center,
double xDistance,
int numEdge,
bool isOuterRadius,
int areaSignPreference
)
    {
    bvector<DPoint3d> points;
    if (numEdge < 3)
        numEdge = 3;
    double ySign = areaSignPreference >= 0.0 ? 1.0 : -1.0;
    if (isOuterRadius)
        {
        points.push_back (center + DVec3d::From (xDistance, 0,0));
        auto point0 = points.front ();
        // work in degrees to get high-accuracy keypoints ..
        double degreeStep = 360.0 / numEdge;
            for (int i = 1; i < numEdge; i++)
            {
            auto theta = AngleInDegrees::FromDegrees (i * degreeStep);
            points.push_back (center + DVec3d::From (xDistance * theta.Cos (), ySign * xDistance * theta.Sin (), 0.0));
            }
        points.push_back (point0);
        }
    else
        {
        // work in degrees to get high-accuracy keypoints ..
        double degreeStep = 360.0 / numEdge;
        double degree0 = ySign * degreeStep * (-0.5);
        double radius = xDistance / cos (Angle::DegreesToRadians (degree0));
        for (int i = 0; i < numEdge; i++)
            {
            auto theta = AngleInDegrees::FromDegrees (degree0 + i * degreeStep);
            points.push_back (center + DVec3d::From (radius * theta.Cos (), ySign * radius * theta.Sin (), 0.0));
            }
        auto point0 = points.front ();
        points.push_back (point0);
        }
    return ICurvePrimitive::CreateLineString (points);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurveVectorPtr CurveVector::CreateDisk
(
DEllipse3dCR arc,
BoundaryType boundaryType,
bool enforceXYOrientation
)
    {
    CurveVectorPtr result = CurveVector::Create (boundaryType);
    if (boundaryType == BOUNDARY_TYPE_UnionRegion
        || boundaryType == BOUNDARY_TYPE_ParityRegion)
        {
        CurveVectorPtr child = CurveVector::CreateDisk (arc, BOUNDARY_TYPE_Outer);
        result->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*child));
        }
    else
        {
        DEllipse3d orientedArc = arc;
        if (enforceXYOrientation)
            {
            int areaSign = 0;
            if (boundaryType == BOUNDARY_TYPE_Outer)
                areaSign = 1;
            else if (boundaryType == BOUNDARY_TYPE_Inner)
                areaSign = -1;
            DVec3d normalVector = DVec3d::FromCrossProduct (arc.vector0, arc.vector90);
            double a = normalVector.z * arc.sweep;

            orientedArc = a * areaSign >= 0.0
                ? arc
                : DEllipse3d::FromReversed (arc);
        }

        result->push_back (ICurvePrimitive::CreateArc (orientedArc));
        if (!orientedArc.IsFullEllipse ())
            {
            DSegment3d segment;
            orientedArc.EvaluateEndPoints (segment.point[0], segment.point[1]);
            result->push_back (ICurvePrimitive::CreateLine (segment));
            }
        }
    return result;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurveVectorPtr CurveVector::Create
(
ICurvePrimitivePtr child,
BoundaryType boundaryType
)
    {
    CurveVectorPtr result = CurveVector::Create (boundaryType);
    result->push_back (child);
    return result;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurveVectorPtr CurveVector::CreateLinear
(
DPoint3dCP points,
size_t numPoints,
BoundaryType boundaryType,
bool enforceXYOrientation
)
    {
    CurveVectorPtr result;
    result = CurveVector::Create (boundaryType);
// July 1 2014 -- allow zero-points.
//    if (numPoints == 0)
//        return result;  // Really don't know what to do with no points.
    if (boundaryType == BOUNDARY_TYPE_UnionRegion
        || boundaryType == BOUNDARY_TYPE_ParityRegion)
        {
        CurveVectorPtr child = CurveVector::CreateLinear (points, numPoints, BOUNDARY_TYPE_Outer, enforceXYOrientation);
        result->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*child));
        }
    else
        {
        int areaSign = 0;
        bool isLoop = false;
        if (boundaryType == BOUNDARY_TYPE_Outer)
            {
            areaSign = 1;
            isLoop = true;
            }
        else if (boundaryType == BOUNDARY_TYPE_Inner)
            {
            areaSign = -1;
            isLoop = true;
            }

        ICurvePrimitivePtr prim = ICurvePrimitive::CreateLineString (points, numPoints);
        bvector<DPoint3d> *primitivePoints = prim->GetLineStringP ();
        if (isLoop && primitivePoints->size () > 1)
            {
            if (DPoint3dOps::AlmostEqual (points[0], points[numPoints-1]))
                {
                // Points are at least almost equal.  Explicitly assign the last for bitwise equality.
                primitivePoints->back () = points[0];
                }
            else
                {
                // add closure point.
                primitivePoints->push_back (points[0]);
                }
            if (primitivePoints->size () > 3 && enforceXYOrientation)
                {
                DPoint3d centroid;
                DVec3d normal;
                double area;
                PolygonOps::CentroidNormalAndArea (*primitivePoints, centroid, normal, area);
                if (normal.z * areaSign < 0.0)
                    DPoint3dOps::Reverse (*primitivePoints);
                }
            }
        result->push_back (prim);
        }
    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurveVectorPtr CurveVector::CreateLinear
(
DPoint2dCP points,
size_t numPoints,
BoundaryType boundaryType,
bool enforceXYOrientation
)
    {
    // ugh ... make a copy, lots of details buried in the creation.
    bvector<DPoint3d> points3d;
    for (size_t i = 0; i < numPoints; i++)
        points3d.push_back (DPoint3d::From (points[i]));
    return CurveVector::CreateLinear (points3d, boundaryType, enforceXYOrientation);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurveVectorPtr CurveVector::CreateLinear
(
bvector<DPoint3d> const &points,
BoundaryType boundaryType,
bool enforceXYOrientation
)
    {
    if (points.size () == 0)
        return CreateLinear ((DPoint3d*)NULL, 0, boundaryType, enforceXYOrientation);
    return CreateLinear (&points[0],points.size (), boundaryType, enforceXYOrientation);
    }

// Recursive search.
// numStepRemaining is decremented at each primitive encountered.
static ICurvePrimitivePtr IndexedLeafSearch (CurveVectorCR parent, size_t &numStepRemaining)
    {
    CurveVector::BoundaryType b = parent.GetBoundaryType ();
    // Outer, Inner, and Open types are not supposed to have subtrees.
    //  Hence they can be indexed directly without looking for children.
    if (   b == CurveVector::BOUNDARY_TYPE_Outer
        || b == CurveVector::BOUNDARY_TYPE_Inner
        || b == CurveVector::BOUNDARY_TYPE_Open)
        {
        size_t n = parent.size ();
        if (numStepRemaining < n)
            {
            size_t iOut = numStepRemaining;
            numStepRemaining = 0;
            return parent.at (iOut);
            }
        numStepRemaining -= n;
        return ICurvePrimitivePtr();
        }
    for (size_t i = 0, n = parent.size (); i < n; i++)
        {
        CurveVectorCP child;
        if (NULL != (child = parent[i]->GetChildCurveVectorCP ()))
            {
            ICurvePrimitivePtr leaf = IndexedLeafSearch (*child, numStepRemaining);
            if (leaf.IsValid ())
                return leaf;
            }
        else if (numStepRemaining == 0)
            {
            return parent.at(i);
            }
        else
            numStepRemaining--;
        }
    return ICurvePrimitivePtr ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr CurveVector::FindIndexedLeaf (size_t index) const
    {
    size_t myIndex = index;
    return IndexedLeafSearch (*this, myIndex);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t CurveVector::CountPrimitivesBelow () const
    {
    size_t numBelow = 0;
    CurveVector::BoundaryType b = GetBoundaryType ();
    if (   b == CurveVector::BOUNDARY_TYPE_Outer
        || b == CurveVector::BOUNDARY_TYPE_Inner
        || b == CurveVector::BOUNDARY_TYPE_Open)
        {
        numBelow += size ();
        }
    else
        {
        for (size_t i = 0, n = size (); i < n; i++)
            {
            CurveVectorCP child;
            if (NULL != (child = at(i)->GetChildCurveVectorCP ()))
                {
                numBelow += child->CountPrimitivesBelow ();
                }
            else
                {
                numBelow++;
                }
            }
        }
    return numBelow;
    }


// Recursive search.
// numStepRemaining is incremented at each primitive that is not the target.
static bool LeafToIndex_go (CurveVectorCR parent, ICurvePrimitiveCP target, size_t &counter)
    {
    for (size_t i = 0, n = parent.size (); i < n; i++)
        {
        CurveVectorCP child;
        if (NULL != (child = parent[i]->GetChildCurveVectorCP ()))
            {
            if (LeafToIndex_go (*child, target, counter))
                return true;
            }
        else if (parent[i].get () == target)
            {
            return true;
            }
        else
            counter++;
        }
    return false;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveVector::LeafToIndex (ICurvePrimitiveCP primitive, size_t &index) const
    {
    index = 0;
    return LeafToIndex_go  (*this, primitive, index);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void CurveVector::SwapContents (CurveVectorR other)
    {
    swap (other);
    std::swap (m_boundaryType, other.m_boundaryType);
    }


END_BENTLEY_GEOMETRY_NAMESPACE
