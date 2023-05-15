/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <GeomSerialization/GeomLibsSerialization.h>
#include <GeomSerialization/GeomLibsJsonSerialization.h>

BEGIN_BENTLEY_DGN_NAMESPACE
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct BRepCache : DgnElement::AppData
{
static DgnElement::AppData::Key const& GetKey() {static DgnElement::AppData::Key s_key; return s_key;}
typedef bmap<uint16_t, IBRepEntityPtr> IndexedGeomMap;
IndexedGeomMap m_map;
BeMutex m_mutex;

virtual DropMe _OnInserted(DgnElementCR el){return DropMe::Yes;}
virtual DropMe _OnUpdated(DgnElementCR modified, DgnElementCR original, bool isOriginal) {return DropMe::Yes;}
virtual DropMe _OnAppliedUpdate(DgnElementCR original, DgnElementCR modified) {return DropMe::Yes;}
virtual DropMe _OnDeleted(DgnElementCR el) {return DropMe::Yes;}

static BRepCache* Get(DgnElementCR elem, bool addIfNotFound)
    {
    if (addIfNotFound)
        return static_cast<BRepCache*>(elem.FindOrAddAppData(GetKey(), []() { return new BRepCache(); }).get());
    else
        return static_cast<BRepCache*>(elem.FindAppData(GetKey()).get());
    }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct BRepDataCache
{
static IBRepEntityPtr FindCachedBRepEntity(DgnElementCR element, GeometryStreamEntryIdCR entryId);
static void AddCachedBRepEntity(DgnElementCR element, GeometryStreamEntryIdCR entryId, IBRepEntityR entity);

}; // BRepDataCache

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IBRepEntityPtr BRepDataCache::FindCachedBRepEntity(DgnElementCR element, GeometryStreamEntryIdCR entryId)
    {
    BRepCache* cache = BRepCache::Get(element, false);

    if (nullptr == cache)
        return nullptr;

    BeMutexHolder lock(cache->m_mutex);

    BRepCache::IndexedGeomMap::const_iterator found = cache->m_map.find(entryId.GetGeometryPartId().IsValid() ? entryId.GetPartIndex() : entryId.GetIndex());

    if (found == cache->m_map.end())
        return nullptr;

    IBRepEntityPtr entity = found->second;

    // Make sure cache entity is still viable...
    if (!entity->IsAlive())
        {
        BeAssert(false);
        return nullptr;
        }

    return entity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BRepDataCache::AddCachedBRepEntity(DgnElementCR element, GeometryStreamEntryIdCR entryId, IBRepEntityR entity)
    {
    BRepCache* cache = BRepCache::Get(element, true);

    BeMutexHolder lock(cache->m_mutex);

    cache->m_map[entryId.GetGeometryPartId().IsValid() ? entryId.GetPartIndex() : entryId.GetIndex()] = &entity;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RangeCache : DgnElement::AppData
{
static DgnElement::AppData::Key const& GetKey() {static DgnElement::AppData::Key s_key; return s_key;}
typedef bmap<uint16_t, DRange3d> IndexedGeomMap;
IndexedGeomMap m_map;
BeMutex m_mutex;

virtual DropMe _OnInserted(DgnElementCR el){return DropMe::Yes;}
virtual DropMe _OnUpdated(DgnElementCR modified, DgnElementCR original, bool isOriginal) {return DropMe::Yes;}
virtual DropMe _OnAppliedUpdate(DgnElementCR original, DgnElementCR modified) {return DropMe::Yes;}
virtual DropMe _OnDeleted(DgnElementCR el) {return DropMe::Yes;}

static RangeCache* Get(DgnElementCR elem, bool addIfNotFound)
    {
    if (addIfNotFound)
        return static_cast<RangeCache*>(elem.FindOrAddAppData(GetKey(), []() { return new RangeCache(); }).get());
    else
        return static_cast<RangeCache*>(elem.FindAppData(GetKey()).get());
    }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct LocalRangeCache
{
static void FindCachedRange(DRange3dR range, DgnElementCR element, GeometryStreamEntryIdCR entryId);
static void AddCachedRange(DgnElementCR element, GeometryStreamEntryIdCR entryId, DRange3dCR range);

}; // LocalRangeCache

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LocalRangeCache::FindCachedRange(DRange3dR range, DgnElementCR element, GeometryStreamEntryIdCR entryId)
    {
    RangeCache* cache = RangeCache::Get(element, false);

    if (nullptr == cache)
        return;

    BeMutexHolder lock(cache->m_mutex);

    RangeCache::IndexedGeomMap::const_iterator found = cache->m_map.find(entryId.GetGeometryPartId().IsValid() ? entryId.GetPartIndex() : entryId.GetIndex());

    if (found == cache->m_map.end())
        return;

    range = found->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LocalRangeCache::AddCachedRange(DgnElementCR element, GeometryStreamEntryIdCR entryId, DRange3dCR range)
    {
    RangeCache* cache = RangeCache::Get(element, true);

    BeMutexHolder lock(cache->m_mutex);

    cache->m_map[entryId.GetGeometryPartId().IsValid() ? entryId.GetPartIndex() : entryId.GetIndex()] = range;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SnapData
{
SnapMode                m_mode;
DPoint3d                m_hitPoint;
DPoint3d                m_snapPoint;
DVec3d                  m_snapNormal = DVec3d::FromZero();
SnapHeat                m_heat = SNAP_HEAT_None;
HitGeomType             m_geomType = HitGeomType::None;
HitParentGeomType       m_parentGeomType = HitParentGeomType::None;
IGeometryPtr            m_geomPtr;
IGeometryPtr            m_intersectGeomPtr;
DgnElementId            m_intersectId;
Transform               m_localToWorld = Transform::FromIdentity();
double                  m_viewDistance = 0.0;
bool                    m_interiorWasPickable = false;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SnapData(SnapMode mode = SnapMode::Invalid, DPoint3dCR snapPoint = DPoint3d::FromZero()) : m_mode(mode), m_snapPoint(snapPoint) { m_hitPoint.InitDisconnect(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsInRange() const { return (SNAP_HEAT_InRange == m_heat && SnapMode::Nearest != m_mode); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsCloser(SnapData const& other)
    {
    if (DoubleOps::WithinTolerance(m_viewDistance, other.m_viewDistance, 0.01))
        return false; // Keep current when distance is the same (snap priority is from set ordering from snapMode value)...

    if (m_viewDistance < other.m_viewDistance)
        return false; // Current is closer of the two hits...

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsPreferredSnap(SnapData const& snap, DPoint3dCR testPoint, DMatrix4dCR worldToView, double snapAperture)
    {
    if (SnapMode::Center != snap.m_mode)
        return false;

    if (HitParentGeomType::Wire != snap.m_parentGeomType && snap.m_interiorWasPickable)
        return false;

    DPoint4d viewPts[3];

    worldToView.Multiply(&viewPts[0], &snap.m_snapPoint, nullptr, 1);
    worldToView.Multiply(&viewPts[1], &snap.m_hitPoint, nullptr, 1);
    worldToView.Multiply(&viewPts[2], &testPoint, nullptr, 1);

    double edgeDist = 0.0, testDist = 0.0;

    viewPts[0].RealDistanceXY(edgeDist, viewPts[1]);
    viewPts[0].RealDistanceXY(testDist, viewPts[2]);

    return (edgeDist > testDist && !DoubleOps::WithinTolerance(edgeDist, testDist, snapAperture * 0.4));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsBetter(SnapData const& other, DPoint3dCR testPoint, DMatrix4dCR worldToView, double snapAperture)
    {
    if (SnapMode::Invalid == other.m_mode)
        return false;

    if (SnapMode::Invalid == m_mode)
        return true;

    bool thisInRange = IsInRange();
    bool otherInRange = other.IsInRange();

    if (thisInRange && otherInRange)
        {
        if (!IsCloser(other))
            return false;
        }
    else if (thisInRange)
        {
        return false; // Current is hot, other is not...
        }
    else if (!otherInRange)
        {
        if (IsPreferredSnap(*this, testPoint, worldToView, snapAperture))
            return false;

        if (IsPreferredSnap(other, testPoint, worldToView, snapAperture))
            return true;

        if (!IsCloser(other))
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool UpdateIfBetter(SnapData const& other, DPoint3dCR testPoint, DMatrix4dCR worldToView, double snapAperture)
    {
    if (!IsBetter(other, testPoint, worldToView, snapAperture))
        return false;

    m_mode = other.m_mode;
    m_hitPoint = other.m_hitPoint;
    m_snapPoint = other.m_snapPoint;
    m_snapNormal = other.m_snapNormal;
    m_heat = other.m_heat;
    m_viewDistance = other.m_viewDistance;
    m_geomType = other.m_geomType;
    m_parentGeomType = other.m_parentGeomType;
    m_localToWorld = other.m_localToWorld;
    m_geomPtr = other.m_geomPtr;
    m_intersectGeomPtr = other.m_intersectGeomPtr;
    m_intersectId = other.m_intersectId;

    return IsInRange(); // Stop looking when we get a hot snap in range...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ToResponse(SnapContext::Response& output) const
    {
    if (SnapMode::Invalid == m_mode)
        {
        output.SetStatus(SnapStatus::NoSnapPossible);
        return;
        }

    output.SetStatus(SnapStatus::Success);
    output.SetSnapMode(m_mode);
    output.SetSnapPoint(m_snapPoint);
    output.SetHeat(m_heat);
    output.SetGeomType(m_geomType);
    output.SetParentGeomType(m_parentGeomType);

    if (!m_hitPoint.IsDisconnect())
        output.SetHitPoint(m_hitPoint);

    if (0.0 != m_snapNormal.Magnitude())
        output.SetNormal(m_snapNormal);

    if (!m_geomPtr.IsValid())
        return;

    if (!m_localToWorld.IsIdentity() && !m_geomPtr->TryTransformInPlace(m_localToWorld))
        return;

    BeJsDocument geomValue;
    if (!IModelJson::TryGeometryToIModelJsonValue(geomValue, *m_geomPtr))
        return;

    output.SetCurve(geomValue);

    if (!m_intersectId.IsValid() || !m_intersectGeomPtr.IsValid())
        return;

    BeJsDocument geomValue2;
    if (!IModelJson::TryGeometryToIModelJsonValue(geomValue2, *m_intersectGeomPtr))
        return;

    output.SetIntersectCurve(geomValue2);
    output.SetIntersectId(m_intersectId);
    }

}; // SnapData

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SnapGeometryHelper : ISnapProcessor
{
private:

uint32_t                m_snapDivisor;
double                  m_snapAperture;
DPoint3d                m_closePtWorld;
DPoint3d                m_closePtLocalCorrected;
DMap4d                  m_worldToView;
ICancellableP           m_cancel;
double                  m_maxOutsideDist = 0.0;

GeometricPrimitivePtr   m_hitGeom;
Transform               m_hitLocalToWorld = Transform::FromIdentity();
GeometryParams          m_hitParams;
GeometryStreamEntryId   m_hitEntryId;
CurveLocationDetail     m_hitCurveDetail;
HitGeomType             m_hitGeomType = HitGeomType::None;
HitParentGeomType       m_hitParentGeomType = HitParentGeomType::None;

DPoint3d                m_hitClosePtLocal;
Transform               m_hitWorldToLocal;
DMap4d                  m_hitLocalToView;
double                  m_hitDistanceView;
double                  m_hitDistanceLocal;
bool                    m_hitCheckInterior;
ICurvePrimitivePtr      m_hitCurveDerived;

// NOTE: ComputeSnapLocation is const to ensure it only changes the below snap information:
mutable CurveLocationDetail     m_snapCurveDetail;
mutable HitGeomType             m_snapGeomType = HitGeomType::None;
mutable DVec3d                  m_snapNormalLocal = DVec3d::FromZero();
mutable bool                    m_findArcCenters = true;

protected:

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsCanceled() const
    {
    return nullptr != m_cancel && m_cancel->IsCanceled();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _CorrectClosePoint(DPoint3dR point)
    {
    // Save point on surface to correct "close point" from readPixels...
    if (m_closePtLocalCorrected.IsDisconnect())
        m_closePtLocalCorrected = point;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _AnnounceICurvePrimitive(ICurvePrimitiveCR curve, DPoint3dCR localPoint, HitGeomType geomType, HitParentGeomType parentGeomType)
    {
    return ProcessICurvePrimitive(curve, localPoint, geomType, parentGeomType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _AnnounceCurveVector(CurveVectorCR curve, DPoint3dCR localPoint, HitGeomType geomType, HitParentGeomType parentGeomType)
    {
    return ProcessCurveVector(curve, localPoint, geomType, parentGeomType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool EvaluateDefaultNormal() const
    {
    if (!m_hitGeom.IsValid())
        return false;

    if (HitParentGeomType::Wire != m_hitParentGeomType)
        return false;

    CurveVectorPtr curves;

    if (GeometricPrimitive::GeometryType::CurveVector == m_hitGeom->GetGeometryType())
        curves = m_hitGeom->GetAsCurveVector();
    else if (m_hitCurveDerived.IsValid())
        curves = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Open, m_hitCurveDerived);

    if (!curves.IsValid())
        return false;

    Transform   localToWorld, worldToLocal;
    DRange3d    localRange;
    DVec3d      defaultNormal = DVec3d::UnitZ(); // Use placement z as default normal for geometry without a well-defined up direction...

    if (!curves->IsPlanarWithDefaultNormal(localToWorld, worldToLocal, localRange, &defaultNormal))
        return false;

    m_snapNormalLocal = localToWorld.ColumnZ();
    m_snapNormalLocal.Normalize();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool EvaluateInterior(SnapMode snapMode) const
    {
    if (!m_hitGeom.IsValid())
        return false;

    if (!m_hitCheckInterior)
        return false;

    // NOTE: Nearest snap tracks surface when edge not within locate aperture...
    bool        interiorPt = (HitGeomType::Point != m_snapGeomType && SnapMode::Nearest == snapMode && m_hitDistanceView > m_snapAperture);
    DPoint3d    localPoint = interiorPt ? m_hitClosePtLocal : m_snapCurveDetail.point; // NOTE: m_hitClosePtLocal has been corrected to surface at this point...

    switch (m_hitGeom->GetGeometryType())
        {
        case GeometricPrimitive::GeometryType::CurveVector:
            {
            CurveVectorCR curves = *m_hitGeom->GetAsCurveVector();

            if (!curves.IsAnyRegionType())
                return false;

            SolidLocationDetail solidDetail;
            DRay3d boresite = GetBoresite(localPoint, m_hitLocalToView.M1);

            CurveVector::InOutClassification result = curves.RayPierceInOnOut(boresite, solidDetail);
            if (CurveVector::InOutClassification::INOUT_Unknown == result)
                return false;

            if (interiorPt)
                m_snapCurveDetail.point = solidDetail.GetXYZ();
            m_snapNormalLocal.NormalizedCrossProduct(solidDetail.GetUDirection(), solidDetail.GetVDirection());
            break;
            }

        case GeometricPrimitive::GeometryType::SolidPrimitive:
            {
            bool                useCurvePoint = (HitGeomType::Point == m_snapGeomType || SnapMode::Center == snapMode);
            DPoint3d            testPointLocal = (useCurvePoint ? m_snapCurveDetail.point : m_hitClosePtLocal); // Prefer face identified by original hit location to disambiguate face for edge...
            SolidLocationDetail solidDetail;

            if (!m_hitGeom->GetAsISolidPrimitive()->ClosestPoint(testPointLocal, solidDetail))
                return false;

            if (useCurvePoint && testPointLocal.Distance(solidDetail.GetXYZ()) > 1.0e-5)
                return false;

            // NOTE: U/V directions don't ensure outward normals, so instead of trying to compare normals, we'll just look for a different face at point closer to eye...
            DRay3d boresite = GetBoresite(localPoint, m_hitLocalToView.M1);
            DPoint3d testPt;
            SolidLocationDetail offsetDetail;

            testPt.SumOf(solidDetail.GetXYZ(), boresite.direction, -1.0e-3);

            if (m_hitGeom->GetAsISolidPrimitive()->ClosestPoint(testPt, offsetDetail) && !offsetDetail.GetFaceIndices().Is(solidDetail.GetFaceIndices().Index0(), solidDetail.GetFaceIndices().Index1(), solidDetail.GetFaceIndices().Index2()))
                solidDetail = offsetDetail;

            IGeometryPtr faceGeom = m_hitGeom->GetAsISolidPrimitive()->GetFace(solidDetail.GetFaceIndices());

            if (!faceGeom.IsValid())
                return false;

            switch (faceGeom->GetGeometryType())
                {
                case IGeometry::GeometryType::CurveVector:
                    {
                    CurveVector::InOutClassification result = faceGeom->GetAsCurveVector()->RayPierceInOnOut(boresite, solidDetail);
                    if (CurveVector::InOutClassification::INOUT_Unknown == result)
                        return false;

                    if (interiorPt)
                        m_snapCurveDetail.point = solidDetail.GetXYZ();
                    m_snapNormalLocal.NormalizedCrossProduct(solidDetail.GetUDirection(), solidDetail.GetVDirection());
                    break;
                    }

                case IGeometry::GeometryType::SolidPrimitive:
                    {
                    if (!faceGeom->GetAsISolidPrimitive()->ClosestPoint(localPoint, solidDetail))
                        return false;

                    DVec3d    uDir, vDir;
                    DPoint3d  point;

                    if (!faceGeom->GetAsISolidPrimitive()->TryUVFractionToXYZ(solidDetail.GetFaceIndices(), solidDetail.GetU(), solidDetail.GetV(), point, uDir, vDir))
                        return false;

                    if (interiorPt)
                        m_snapCurveDetail.point = point;
                    m_snapNormalLocal.NormalizedCrossProduct(uDir, vDir);
                    break;
                    }

                case IGeometry::GeometryType::BsplineSurface:
                    {
                    DPoint3d  surfacePoint;
                    DPoint2d  surfaceUV;
                    DVec3d    uDir, vDir, dPdUU, dPdVV, dPdUV;

                    faceGeom->GetAsMSBsplineSurface()->ClosestPoint(surfacePoint, surfaceUV, localPoint);

                    if (interiorPt)
                        m_snapCurveDetail.point = surfacePoint;
                    faceGeom->GetAsMSBsplineSurface()->EvaluateAllPartials(surfacePoint, uDir, vDir, dPdUU, dPdVV, dPdUV, m_snapNormalLocal, surfaceUV.x, surfaceUV.y);
                    m_snapNormalLocal.Normalize();
                    break;
                    }

                default:
                    return false;
                }
            break;
            }

        case GeometricPrimitive::GeometryType::BsplineSurface:
            {
            DPoint3d  surfacePoint;
            DPoint2d  surfaceUV;
            DVec3d    uDir, vDir, dPdUU, dPdVV, dPdUV;

            m_hitGeom->GetAsMSBsplineSurface()->ClosestPoint(surfacePoint, surfaceUV, localPoint);

            if (interiorPt)
                m_snapCurveDetail.point = surfacePoint;
            m_hitGeom->GetAsMSBsplineSurface()->EvaluateAllPartials(surfacePoint, uDir, vDir, dPdUU, dPdVV, dPdUV, m_snapNormalLocal, surfaceUV.x, surfaceUV.y);
            m_snapNormalLocal.Normalize();
            break;
            }

        case GeometricPrimitive::GeometryType::Polyface:
            {
            PolyfaceVisitorPtr  visitor = PolyfaceVisitor::Attach(*m_hitGeom->GetAsPolyfaceHeader());
            bool                found = false;
            double              tolerance = 1e37 /*fc_hugeVal*/;
            DPoint3d            testPointLocal = localPoint;
            DVec3d              testDir = DVec3d::FromStartEnd(localPoint, m_hitClosePtLocal);

            if (0.0 != testDir.Normalize())
                testPointLocal.SumOf(localPoint, testDir, 1.0e-3); // Bias test point to side of identified facet for better snap normal...locatation won't be used for a hot snap...

            visitor->SetNumWrap(1);

            for (; visitor->AdvanceToNextFace();)
                {
                DPoint3d thisPoint;

                if (!visitor->TryFindCloseFacetPoint(testPointLocal, tolerance, thisPoint))
                    continue;

                FacetLocationDetail thisDetail;

                if (!visitor->LoadVertexData(thisDetail, 0))
                    continue;

                if (interiorPt)
                    m_snapCurveDetail.point = thisPoint;

                DVec3d thisNormal;
                DPoint3d thisCentroid;
                double thisArea;

                if (thisDetail.TryGetNormal(thisNormal))
                    m_snapNormalLocal.Normalize(thisNormal);
                else if (visitor->TryGetFacetCentroidNormalAndArea(thisCentroid, thisNormal, thisArea))
                    m_snapNormalLocal.Normalize(thisNormal);

                tolerance = thisPoint.Distance(testPointLocal); // Refine tolerance...
                found = true;
                }

            if (!found)
                return false;
            break;
            }

        case GeometricPrimitive::GeometryType::BRepEntity:
            {
            bool           useCurvePoint = (HitGeomType::Point == m_snapGeomType || SnapMode::Center == snapMode);
            DPoint3d       testPointLocal = (useCurvePoint ? m_snapCurveDetail.point : m_hitClosePtLocal); // Prefer face identified by original hit location to disambiguate face for edge...
            DRay3d         boresite = GetBoresite(testPointLocal, m_hitLocalToView.M1);
            DVec3d         viewZ = DVec3d::FromScale(boresite.direction, -1.0);
            IBRepEntityCR  entity = *m_hitGeom->GetAsIBRepEntity();
            DPoint3d       point;
            DVec3d         normal;

            if (!T_HOST.GetBRepGeometryAdmin()._SnapEvaluateInterior(entity, localPoint, testPointLocal, viewZ, point, normal))
                return false;

            if (useCurvePoint && testPointLocal.Distance(point) > 1.0e-5)
                return false; // reject point not on surface...

            if (interiorPt)
                m_snapCurveDetail.point = point;
            m_snapNormalLocal = normal;
            break;
            }

        default:
            {
            // Shouldn't be any surface type not covered above...
            return false;
            }
        }

    if (interiorPt)
        {
        m_snapGeomType = HitGeomType::Surface;
        m_snapCurveDetail.curve = nullptr; // Don't flash single curve primitive when snapping to interior...
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool EvaluateArcCenter(SnapMode snapMode) const
    {
    if (!m_findArcCenters)
        return false;

    if (nullptr == m_snapCurveDetail.curve || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc != m_snapCurveDetail.curve->GetCurvePrimitiveType())
        return false;

    DEllipse3dCP arc = m_snapCurveDetail.curve->GetArcCP();

    if (fabs(arc->sweep) < Angle::PiOver2())
        return false;

    DPoint3d localPts[2];
    DPoint4d viewPts[2];

    localPts[0] = m_hitClosePtLocal;
    localPts[1] = arc->center;

    // NOTE: For keypoint snap to interior, choose arc center if interior point is "closer" to it than to the edge...
    if (SnapMode::NearestKeypoint == snapMode && (HitParentGeomType::Sheet == m_hitParentGeomType || HitParentGeomType::Solid == m_hitParentGeomType))
        {
        double radius = DoubleOps::Min(arc->vector0.Magnitude(), arc->vector90.Magnitude());

        if (localPts[0].Distance(localPts[1]) > (radius * 0.25))
            return false;
        }
    else
        {
        m_hitLocalToView.M0.Multiply(viewPts, localPts, nullptr, 2);

        double viewDist = 0.0;
        if (!viewPts[0].RealDistanceXY(viewDist, viewPts[1]) || viewDist > m_snapAperture)
            return false;
        }

    m_snapGeomType = HitGeomType::Point;
    m_snapCurveDetail.point = arc->center;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool EvaluateCurve(SnapMode snapMode) const
    {
    if (nullptr == m_snapCurveDetail.curve)
        return (SnapMode::Nearest == snapMode || HitGeomType::Point == m_snapGeomType);

    SnapMode effectiveSnapMode = snapMode;

    if (HitParentGeomType::Text == m_hitParentGeomType)
        {
        switch (effectiveSnapMode)
            {
            case SnapMode::Nearest:
                effectiveSnapMode = SnapMode::NearestKeypoint;
                break;

            case SnapMode::MidPoint:
            case SnapMode::Bisector:
                effectiveSnapMode = SnapMode::Center;
                break;

            case SnapMode::NearestKeypoint:
                effectiveSnapMode = SnapMode::Origin; // NOTE: Will use lower left of identified TextString. Ideally we'd want user origin for overall text block, but we can't get that from just the GeometryStream...
                break;
            }
        }

    switch (effectiveSnapMode)
        {
        case SnapMode::Nearest:
            {
            EvaluateArcCenter(effectiveSnapMode);
            return true;
            }

        case SnapMode::Origin:
            {
            DPoint3d hitPoint;

            if (!m_snapCurveDetail.curve->GetStartPoint(hitPoint))
                return false;

            m_snapCurveDetail.point = hitPoint;
            return true;
            }

        case SnapMode::MidPoint:
            {
            switch (m_snapCurveDetail.curve->GetCurvePrimitiveType())
                {
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                    {
                    DSegment3dCP segment = m_snapCurveDetail.curve->GetLineCP();

                    m_snapCurveDetail.point.Interpolate(segment->point[0], 0.5, segment->point[1]);
                    return true;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                    {
                    DSegment3d segment;

                    if (!m_snapCurveDetail.curve->TryGetSegmentInLineString(segment, m_snapCurveDetail.componentIndex))
                        return false;

                    m_snapCurveDetail.point.Interpolate(segment.point[0], 0.5, segment.point[1]);
                    return true;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                    {
                    if (EvaluateArcCenter(effectiveSnapMode))
                        return true;

                    DEllipse3dCP ellipse = m_snapCurveDetail.curve->GetArcCP();

                    ellipse->FractionParameterToPoint(m_snapCurveDetail.point, 0.5);
                    return true;
                    }
                }

            // Fall through to bisector if not handled...
            }

        case SnapMode::Bisector:
            {
            double length;

            if (!m_snapCurveDetail.curve->Length(length))
                return false;

            CurveLocationDetail location;

            if (!m_snapCurveDetail.curve->PointAtSignedDistanceFromFraction(0.0, length * 0.5, false, location))
                return false;

            m_snapCurveDetail.point = location.point;
            return true;
            }

        case SnapMode::Center:
            {
            DPoint3d centroid;

            if (m_hitGeom.IsValid() && ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc != m_snapCurveDetail.curve->GetCurvePrimitiveType() && GeometricPrimitive::GeometryType::CurveVector == m_hitGeom->GetGeometryType())
                {
                CurveVectorCR curves = *m_hitGeom->GetAsCurveVector();

                if (!GetCentroid(centroid, curves))
                    return false;

                m_snapCurveDetail.point = centroid;

                if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Invalid == curves.HasSingleCurvePrimitive())
                    {
                    if (curves.IsAnyRegionType())
                        m_snapGeomType = HitGeomType::Surface;

                    m_snapCurveDetail.curve = nullptr; // Don't flash single curve primitive when snapping to center of multi-curve path/region...
                    }

                return true;
                }

            if (!GetCentroid(centroid, *m_snapCurveDetail.curve))
                return false;

            m_snapCurveDetail.point = centroid;
            return true;
            }

        case SnapMode::NearestKeypoint:
            {
            switch (m_snapCurveDetail.curve->GetCurvePrimitiveType())
                {
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                    {
                    DSegment3dCP segment = m_snapCurveDetail.curve->GetLineCP();

                    SnapContext::GetSegmentKeypoint(m_snapCurveDetail.point, m_snapCurveDetail.componentFraction, m_snapDivisor, *segment);
                    return true;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                    {
                    DSegment3d segment;

                    if (!m_snapCurveDetail.curve->TryGetSegmentInLineString(segment, m_snapCurveDetail.componentIndex))
                        return false;

                    SnapContext::GetSegmentKeypoint(m_snapCurveDetail.point, m_snapCurveDetail.componentFraction, m_snapDivisor, segment);
                    return true;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                    {
                    if (EvaluateArcCenter(effectiveSnapMode))
                        return true;

                    // FALL THROUGH...
                    }

                default:
                    {
                    return SnapContext::GetParameterKeypoint(*m_snapCurveDetail.curve, m_snapCurveDetail.point, m_snapCurveDetail.fraction, m_snapDivisor);
                    }
                }
            }

        default:
            {
            // Should never be called with "exotic" snap modes...
            return false;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyHitDetail()
    {
    if (nullptr == m_hitCurveDetail.curve)
        return;

    switch (m_hitCurveDetail.curve->GetCurvePrimitiveType())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            {
            DSegment3dCP segment = m_hitCurveDetail.curve->GetLineCP();

            if (segment->point[0].IsEqual(segment->point[1])) // Check for zero length line and don't include redundant primitive...
                {
                m_hitCurveDetail.curve = nullptr;
                m_hitGeomType = HitGeomType::Point;
                }

            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            {
            bvector<DPoint3d> const* points = m_hitCurveDetail.curve->GetLineStringCP();

            if ((1 == points->size()) || (2 == points->size() && points->at(0).IsEqual(points->at(1))))
                {
                m_hitCurveDetail.curve = nullptr;
                m_hitGeomType = HitGeomType::Point;
                }

            break;
            }

        default:
            {
            MSBsplineCurveCP bcurve = m_hitCurveDetail.curve->GetProxyBsplineCurveCP();

            if (nullptr == bcurve || 2 != bcurve->params.order)
                break;

            // An order 2 bspline curve should be treated the same as a linestring for snapping...
            bvector<DPoint3d> poles;

            bcurve->GetUnWeightedPoles(poles);

            if (bcurve->params.closed)
                poles.push_back(poles.at(0));

            ICurvePrimitivePtr  curve = ICurvePrimitive::CreateLineString(poles);
            CurveLocationDetail detail;

            if (!curve->ClosestPointBounded(m_hitCurveDetail.point, detail))
                break;

            CurvePrimitiveIdCP curveId = m_hitCurveDetail.curve->GetId();

            if (nullptr != curveId)
                curve->SetId(curveId->Clone().get()); // Preserve curve topology id from source curve...

            detail.a = m_hitCurveDetail.a; // Preserve distance to original hit...

            m_hitCurveDerived = curve;
            m_hitCurveDetail = detail;
            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsCloserToTestPoint(CurveLocationDetailCR curveDetail, double distanceLocal, bool checkInterior)
    {
    if (checkInterior || m_hitCheckInterior)
        {
        if (distanceLocal < m_hitDistanceLocal)
            return true; // Prefer surface that is closer to test point...
        }

    return (curveDetail.a < m_hitCurveDetail.a);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool UpdateIfCloser(CurveLocationDetailCR curveDetail, HitGeomType geomType, HitParentGeomType parentGeomType, DPoint3dCR localPoint)
    {
    bool        checkInterior = !m_closePtLocalCorrected.IsDisconnect();
    double      distanceLocal = checkInterior ? localPoint.Distance(m_closePtLocalCorrected) : curveDetail.a;

    if ((nullptr == m_hitCurveDetail.curve && HitGeomType::None == m_hitGeomType) || IsCloserToTestPoint(curveDetail, distanceLocal, checkInterior))
        {
        m_hitCurveDerived   = const_cast<ICurvePrimitiveP>(curveDetail.curve); // Make sure m_hitCurveDetail.curve remains valid...
        m_hitCurveDetail    = curveDetail;
        m_hitGeomType       = geomType;
        m_hitParentGeomType = parentGeomType;

        m_hitClosePtLocal   = checkInterior ? m_closePtLocalCorrected : localPoint;
        m_hitDistanceLocal  = distanceLocal;
        m_hitCheckInterior  = checkInterior;

        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double GetMaxOutsideDistance()
    {
    if (0.0 == m_maxOutsideDist)
        {
        DPoint4d viewPts[2];

        m_worldToView.M0.Multiply(viewPts, &m_closePtWorld, nullptr, 1);
        viewPts[1] = viewPts[0];
        viewPts[1].x += viewPts[1].w;

        DPoint4d worldPts[2];

        m_worldToView.M1.Multiply(worldPts, viewPts, 2);
        m_maxOutsideDist = worldPts[0].RealDistance(worldPts[1]) * m_snapAperture;
        }

    return m_maxOutsideDist;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d GetClosePointLocal(Transform worldToLocalTrans) const
    {
    DPoint3d    closePointLocal = m_closePtWorld;

    worldToLocalTrans.Multiply(closePointLocal);

    return closePointLocal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DMatrix4d GetViewToLocal(Transform worldToLocalTrans) const
    {
    DMatrix4d   worldToLocal = DMatrix4d::From(worldToLocalTrans);
    DMatrix4d   viewToWorld = m_worldToView.M1;
    DMatrix4d   viewToLocal;

    viewToLocal.InitProduct(worldToLocal, viewToWorld);

    return viewToLocal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DRay3d GetBoresite(DPoint3dCR localPoint, DMatrix4dCR viewToLocal) const
    {
    double      aa;
    DRay3d      boresite;
    DPoint4d    eyePoint;

    viewToLocal.GetColumn(eyePoint, 2);
    boresite.direction.Init(eyePoint.x, eyePoint.y, eyePoint.z);
    boresite.origin = localPoint;

    if (DoubleOps::SafeDivide(aa, 1.0, eyePoint.w, 1.0))
        {
        DPoint3d  xyzEye;

        xyzEye.Scale(boresite.direction, aa);
        boresite.direction.DifferenceOf(xyzEye, boresite.origin);
        }

    boresite.direction.Normalize();
    boresite.direction.Negate();

    return boresite;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessPointString(ICurvePrimitiveCR curve, DPoint3dCR localPoint)
    {
    // NOTE: When locating from tiles, localPoint is known to be a vertex, so we can just use ClosestPointBounded.
    CurveLocationDetail curveDetail;

    if (!curve.ClosestPointBounded(localPoint, curveDetail))
        return false;

    curveDetail.curve = nullptr; // Don't store curve for point string hit...

    return UpdateIfCloser(curveDetail, HitGeomType::Point, HitParentGeomType::None, localPoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessICurvePrimitive(ICurvePrimitiveCR curve, DPoint3dCR localPoint, HitGeomType geomType, HitParentGeomType parentGeomType)
    {
    CurveLocationDetail curveDetail;

    if (!curve.ClosestPointBounded(localPoint, curveDetail))
        return false;

    return UpdateIfCloser(curveDetail, geomType, parentGeomType, localPoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessCurveVector(CurveVectorCR curves, DPoint3dCR localPoint, HitGeomType geomType, HitParentGeomType parentGeomType)
    {
    CurveLocationDetail curveDetail;

    if (!curves.ClosestPointBounded(localPoint, curveDetail))
        return false;

    // Save point on surface to correct "close point" from readPixels...
    if (m_closePtLocalCorrected.IsDisconnect() && curves.IsAnyRegionType())
        {
        DPoint3d curveOrRegionPoint;
        CurveVector::InOutClassification result = curves.ClosestCurveOrRegionPoint(localPoint, curveOrRegionPoint);
        if (CurveVector::InOutClassification::INOUT_Unknown != result)
            m_closePtLocalCorrected = curveOrRegionPoint;
        }

    return UpdateIfCloser(curveDetail, geomType, parentGeomType, localPoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessBsplineSurface(MSBsplineSurfaceCR surface, DPoint3dCR localPoint, HitParentGeomType parentGeomType)
    {
    bool    foundCurve = false;

    if (!surface.uParams.closed || !surface.vParams.closed)
        {
        CurveVectorPtr curves = surface.GetUnstructuredBoundaryCurves(0.0, true, true);

        if (curves.IsValid() && ProcessCurveVector(*curves, localPoint, HitGeomType::None, parentGeomType))
            foundCurve = true;

        if (_IsCanceled())
            return false;
        }

    DPoint3d    surfacePoint;
    DPoint2d    surfaceUV;

    surface.ClosestPoint(surfacePoint, surfaceUV, localPoint);

    // Save point on surface to correct "close point" from readPixels...
    if (m_closePtLocalCorrected.IsDisconnect())
        m_closePtLocalCorrected = surfacePoint;

    uint32_t divisorU = m_snapDivisor;
    uint32_t divisorV = divisorU;

    // NOTE: For closed directions we want more key points...
    if (surface.uParams.closed)
        divisorU *= 2;

    if (surface.vParams.closed)
        divisorV *= 2;

    uint32_t    keyFactorU = uint32_t ((surfaceUV.x / 1.0) * (double) divisorU + 0.5);
    uint32_t    keyFactorV = uint32_t ((surfaceUV.y / 1.0) * (double) divisorV + 0.5);
    double      keyParamU = (keyFactorU / (double) divisorU);
    double      keyParamV = (keyFactorV / (double) divisorV);

    if (surface.uParams.closed || (!DoubleOps::AlmostEqual(keyParamU, 0.0) && !DoubleOps::AlmostEqual(keyParamU, 1.0)))
        {
        bvector<MSBsplineCurvePtr> segmentsU;

        surface.GetIsoUCurveSegments(keyParamU, segmentsU);

        for (MSBsplineCurvePtr& isoCurveU : segmentsU)
            {
            ICurvePrimitivePtr  curve = ICurvePrimitive::CreateBsplineCurveSwapFromSource(*isoCurveU);

            if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::Surface, parentGeomType))
                foundCurve = true;

            if (_IsCanceled())
                return false;
            }
        }

    if (surface.vParams.closed || (!DoubleOps::AlmostEqual(keyParamV, 0.0) && !DoubleOps::AlmostEqual(keyParamV, 1.0)))
        {
        bvector<MSBsplineCurvePtr> segmentsV;

        surface.GetIsoVCurveSegments(keyParamV, segmentsV);

        for (MSBsplineCurvePtr& isoCurveV : segmentsV)
            {
            ICurvePrimitivePtr  curve = ICurvePrimitive::CreateBsplineCurveSwapFromSource(*isoCurveV);

            if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::Surface, parentGeomType))
                foundCurve = true;

            if (_IsCanceled())
                return false;
            }
        }

    return foundCurve;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessSingleFaceSolidPrimitive(ISolidPrimitiveCR primitive, DPoint3dCR localPoint, TransformCR worldToLocal, HitParentGeomType parentGeomType)
    {
    uint32_t    divisorU = m_snapDivisor;
    uint32_t    divisorV = divisorU;
    bool        foundCurve = false;

    switch (primitive.GetSolidPrimitiveType())
        {
        case SolidPrimitiveType_DgnTorusPipe:
            {
            DgnTorusPipeDetail detail;

            if (!primitive.TryGetDgnTorusPipeDetail(detail))
                return false;

            divisorU = 4; // Don't think other arcs are very useful snap locations...avoid clutter and just output every 90 degrees...

            for (uint32_t uRule = 0; uRule < divisorU; ++uRule)
                {
                double              uFraction = (1.0 / divisorU) * uRule;
                ICurvePrimitivePtr  curve = ICurvePrimitive::CreateArc(detail.UFractionToVSectionDEllipse3d(uFraction));

                if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::Surface, parentGeomType))
                    foundCurve = true;
                }

            bool  fullTorus = Angle::IsFullCircle(detail.m_sweepAngle);

            if (fullTorus)
                divisorV *= 2;

            for (uint32_t vRule = 0; vRule <= divisorV; ++vRule)
                {
                double vFraction = (1.0 / divisorV) * vRule;

                if (fullTorus && DoubleOps::AlmostEqual(vFraction, 1.0))
                    continue; // Don't duplicate first arc for a full torus...

                HitGeomType hitGeomType = HitGeomType::Surface;

                if (!fullTorus && (DoubleOps::AlmostEqual(vFraction, 0.0) || DoubleOps::AlmostEqual(vFraction, 1.0)))
                    hitGeomType = HitGeomType::None;

                ICurvePrimitivePtr  curve = ICurvePrimitive::CreateArc(detail.VFractionToUSectionDEllipse3d(vFraction));

                if (ProcessICurvePrimitive(*curve, localPoint, hitGeomType, parentGeomType))
                    foundCurve = true;
                }

            return foundCurve;
            }

        case SolidPrimitiveType_DgnCone:
            {
            DgnConeDetail detail;

            if (!primitive.TryGetDgnConeDetail(detail))
                return false;

            DEllipse3d ellipse1;

            if (detail.FractionToSection(0.0, ellipse1))
                {
                ICurvePrimitivePtr  curve = ICurvePrimitive::CreateArc(ellipse1);

                if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::None, parentGeomType))
                    foundCurve = true;
                }

            DEllipse3d ellipse2;

            if (detail.FractionToSection(1.0, ellipse2))
                {
                ICurvePrimitivePtr  curve = ICurvePrimitive::CreateArc(ellipse2);

                if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::None, parentGeomType))
                    foundCurve = true;
                }

            divisorV *= 2;

            for (uint32_t vRule = 0; vRule < divisorV; ++vRule)
                {
                double      vFraction = (1.0 / divisorV) * vRule;
                DSegment3d  segment;

                if (!detail.FractionToRule(vFraction, segment))
                    continue;

                ICurvePrimitivePtr  curve = ICurvePrimitive::CreateLine(segment);

                if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::Surface, parentGeomType))
                    foundCurve = true;
                }

            DSegment3d  silhouette[2];
            DMatrix4d   viewToLocal = GetViewToLocal(worldToLocal);

            if (detail.GetSilhouettes(silhouette[0], silhouette[1], viewToLocal))
                {
                for (int iSilhouette = 0; iSilhouette < 2; ++iSilhouette)
                    {
                    if (0.0 == silhouette[iSilhouette].Length())
                        continue;

                    ICurvePrimitivePtr  curve = ICurvePrimitive::CreateLine(silhouette[iSilhouette]);

                    if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::Surface, parentGeomType))
                        foundCurve = true;
                    }
                }

            return foundCurve;
            }

        case SolidPrimitiveType_DgnSphere:
            {
            DgnSphereDetail detail;

            if (!primitive.TryGetDgnSphereDetail(detail))
                return false;

            divisorU *= 2;

            for (uint32_t uRule = 0; uRule < divisorU; ++uRule)
                {
                double              uFraction = (1.0 / divisorU) * uRule;
                ICurvePrimitivePtr  curve = ICurvePrimitive::CreateArc(detail.UFractionToVSectionDEllipse3d(uFraction));

                if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::Surface, parentGeomType))
                    foundCurve = true;
                }

            double latitude0, latitude1, z0, z1;
            bool   fullSphere = !detail.GetSweepLimits(latitude0, latitude1, z0, z1);

            if (fullSphere)
                divisorV = 2; // Don't think other arcs are very useful snap locations...avoid clutter and just output equator...

            double vFraction0 = detail.LatitudeToVFraction(latitude0);
            double vFraction1 = detail.LatitudeToVFraction(latitude1);

            for (uint32_t vRule = 0; vRule <= divisorV; ++vRule)
                {
                double vFraction = vFraction0 + (((vFraction1 - vFraction0) / divisorV) * vRule);

                if (fullSphere && (DoubleOps::AlmostEqual(vFraction, 0.0) || DoubleOps::AlmostEqual(vFraction, 1.0)))
                    continue; // Don't generate 0 radius arcs at top/bottom of full sphere...

                ICurvePrimitivePtr  curve = ICurvePrimitive::CreateArc(detail.VFractionToUSectionDEllipse3d(vFraction));

                if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::Surface, parentGeomType))
                    foundCurve = true;
                }

            return foundCurve;
            }

        case SolidPrimitiveType_DgnExtrusion:
            {
            DgnExtrusionDetail detail;

            if (!primitive.TryGetDgnExtrusionDetail(detail))
                return false;

            if (ProcessCurveVector(*detail.m_baseCurve, localPoint, HitGeomType::None, parentGeomType))
                foundCurve = true;

            CurveVectorPtr tmpCurve = detail.m_baseCurve->Clone(Transform::From(detail.m_extrusionVector));

            if (tmpCurve.IsValid() && ProcessCurveVector(*tmpCurve, localPoint, HitGeomType::None, parentGeomType))
                foundCurve = true;

            if (_IsCanceled())
                return false;

            bvector<DSegment3d> edges;

            WireframeGeomUtil::CollectLateralEdges(detail, edges);

            for (DSegment3d segment : edges)
                {
                ICurvePrimitivePtr  curve = ICurvePrimitive::CreateLine(segment);

                if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::None, parentGeomType))
                    foundCurve = true;

                if (_IsCanceled())
                    return false;
                }

            bvector<DPoint3d> pts;

            WireframeGeomUtil::CollectLateralRulePoints(*detail.m_baseCurve, pts, divisorU, divisorU * 2);

            for (DPoint3d pt : pts)
                {
                DSegment3d          segment = DSegment3d::FromOriginAndDirection(pt, detail.m_extrusionVector);
                ICurvePrimitivePtr  curve = ICurvePrimitive::CreateLine(segment);

                if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::Surface, parentGeomType))
                    foundCurve = true;

                if (_IsCanceled())
                    return false;
                }

            return foundCurve;
            }

        case SolidPrimitiveType_DgnRotationalSweep:
            {
            DgnRotationalSweepDetail detail;

            if (!primitive.TryGetDgnRotationalSweepDetail(detail))
                return false;

            if (!Angle::IsFullCircle(detail.m_sweepAngle))
                {
                if (ProcessCurveVector(*detail.m_baseCurve, localPoint, HitGeomType::None, parentGeomType))
                    foundCurve = true;

                DPoint3d  axisPoint;
                Transform transform;

                axisPoint.SumOf(detail.m_axisOfRotation.origin, detail.m_axisOfRotation.direction);
                transform.InitFromLineAndRotationAngle(detail.m_axisOfRotation.origin, axisPoint, detail.m_sweepAngle);

                CurveVectorPtr tmpCurve = detail.m_baseCurve->Clone(transform);

                if (tmpCurve.IsValid() && ProcessCurveVector(*tmpCurve, localPoint, HitGeomType::None, parentGeomType))
                    foundCurve = true;

                if (_IsCanceled())
                    return false;
                }

            bvector<DEllipse3d> edges;

            WireframeGeomUtil::CollectLateralEdges(detail, edges);

            for (DEllipse3d arc : edges)
                {
                ICurvePrimitivePtr  curve = ICurvePrimitive::CreateArc(arc);

                if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::None, parentGeomType))
                    foundCurve = true;

                if (_IsCanceled())
                    return false;
                }

            bvector<DPoint3d> pts;

            WireframeGeomUtil::CollectLateralRulePoints(*detail.m_baseCurve, pts, divisorU, 4); // Avoid clutter and just output every 90 degrees for closed face (i.e. rotational sweep is a torus)...

            RotMatrix   axes, invAxes, tmpRMatrix;
            Transform   transform;

            invAxes.InitFrom1Vector(detail.m_axisOfRotation.direction, 2, true);
            axes.TransposeOf(invAxes);

            tmpRMatrix.InitFromPrincipleAxisRotations(axes, 0.0, 0.0, detail.m_sweepAngle);
            tmpRMatrix.InitProduct(invAxes, tmpRMatrix);
            transform.From(tmpRMatrix, detail.m_axisOfRotation.origin);

            for (size_t iPt = 0; iPt < pts.size(); ++iPt)
                {
                DEllipse3d  ellipse;

                if (!WireframeGeomUtil::ComputeRuleArc(ellipse, pts.at(iPt), detail.m_axisOfRotation.origin, detail.m_sweepAngle, transform, axes, invAxes, 1.0e-8))
                    continue;

                ICurvePrimitivePtr  curve = ICurvePrimitive::CreateArc(ellipse);

                if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::Surface, parentGeomType))
                    foundCurve = true;

                if (_IsCanceled())
                    return false;
                }

            bool  fullRevolve = Angle::IsFullCircle(detail.m_sweepAngle);

            if (fullRevolve)
                divisorV *= 2;

            for (uint32_t vRule = 0; vRule < divisorV; ++vRule)
                {
                double vFraction = (1.0 / divisorV) * vRule;

                if (!fullRevolve && DoubleOps::AlmostEqual(vFraction, 0.0))
                    continue;

                CurveVectorPtr  curve = detail.VFractionToProfile(vFraction);

                if (curve.IsValid() && ProcessCurveVector(*curve, localPoint, HitGeomType::Surface, parentGeomType))
                    foundCurve = true;

                if (_IsCanceled())
                    return false;
                }

            return foundCurve;
            }

        case SolidPrimitiveType_DgnRuledSweep:
            {
            DgnRuledSweepDetail detail;

            if (!primitive.TryGetDgnRuledSweepDetail(detail))
                return false;

            for (CurveVectorPtr curves : detail.m_sectionCurves)
                {
                if (ProcessCurveVector(*curves, localPoint, HitGeomType::None, parentGeomType))
                    foundCurve = true;
                }

            if (_IsCanceled())
                return false;

            bvector<DSegment3d> edges;

            WireframeGeomUtil::CollectLateralEdges(detail, edges);

            for (DSegment3d segment : edges)
                {
                ICurvePrimitivePtr  curve = ICurvePrimitive::CreateLine(segment);

                if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::None, parentGeomType))
                    foundCurve = true;

                if (_IsCanceled())
                    return false;
                }

            for (size_t iProfile = 0; iProfile < detail.m_sectionCurves.size()-1; ++iProfile)
                {
                bvector<DPoint3d> rulePts1;
                bvector<DPoint3d> rulePts2;

                WireframeGeomUtil::CollectLateralRulePoints(*detail.m_sectionCurves.at(iProfile), rulePts1, divisorU, divisorU * 2);
                WireframeGeomUtil::CollectLateralRulePoints(*detail.m_sectionCurves.at(iProfile+1), rulePts2, divisorU, divisorU * 2);

                if (rulePts1.size() != rulePts2.size())
                    {
                    if (1 == rulePts2.size()) // Special case to handle zero scale in both XY...
                        rulePts2.insert(rulePts2.end(), rulePts1.size()-1, rulePts2.front());
                    else
                        {
                        rulePts1.clear();
                        rulePts2.clear();
                        }
                    }

                for (size_t iRule = 0; iRule < rulePts1.size(); ++iRule)
                    {
                    DSegment3d          segment = DSegment3d::From(rulePts1.at(iRule), rulePts2.at(iRule));
                    ICurvePrimitivePtr  curve = ICurvePrimitive::CreateLine(segment);

                    if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::Surface, parentGeomType))
                        foundCurve = true;

                    if (_IsCanceled())
                        return false;
                    }
                }

            return foundCurve;
            }

        default:
            {
            BeAssert(false);
            return false;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessSolidPrimitive(ISolidPrimitiveCR primitive, DPoint3dCR localPoint, TransformCR worldToLocal)
    {
    HitParentGeomType parentGeomType = (primitive.GetCapped() ? HitParentGeomType::Solid : HitParentGeomType::Sheet);
    bvector<SolidLocationDetail::FaceIndices> faceIndices;

    primitive.GetFaceIndices(faceIndices);

    if (faceIndices.size() < 2)
        {
        // Save point on surface to correct "close point" from readPixels...
        if (m_closePtLocalCorrected.IsDisconnect())
            {
            SolidLocationDetail location;

            if (primitive.ClosestPoint(localPoint, location))
                m_closePtLocalCorrected = location.GetXYZ();
            }

        return ProcessSingleFaceSolidPrimitive(primitive, localPoint, worldToLocal, parentGeomType);
        }

    SolidLocationDetail location;

    if (!primitive.ClosestPoint(localPoint, location))
        return false;

    // Save point on surface to correct "close point" from readPixels...
    if (m_closePtLocalCorrected.IsDisconnect())
        m_closePtLocalCorrected = location.GetXYZ();

    IGeometryPtr faceGeom = primitive.GetFace(location.GetFaceIndices());

    if (!faceGeom.IsValid())
        return false;

    switch (faceGeom->GetGeometryType())
        {
        case IGeometry::GeometryType::CurveVector:
            {
            CurveVectorPtr faceCurves = faceGeom->GetAsCurveVector();

            return (faceCurves.IsValid() && ProcessCurveVector(*faceCurves, localPoint, HitGeomType::None, parentGeomType));
            }

        case IGeometry::GeometryType::SolidPrimitive:
            {
            ISolidPrimitivePtr facePrimitive = faceGeom->GetAsISolidPrimitive();

            return (facePrimitive.IsValid() && ProcessSingleFaceSolidPrimitive(*facePrimitive, localPoint, worldToLocal, parentGeomType));
            }

        case IGeometry::GeometryType::BsplineSurface:
            {
            MSBsplineSurfacePtr faceSurface = faceGeom->GetAsMSBsplineSurface();

            return (faceSurface.IsValid() && ProcessBsplineSurface(*faceSurface, localPoint, parentGeomType));
            }

        default:
            {
            BeAssert(false);
            return false;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessSingleFacePolyface(PolyfaceQueryCR meshData, DPoint3dCR localPoint)
    {
    int const*  vertIndex = meshData.GetPointIndexCP();

    if (!vertIndex)
        return false;

    size_t      numIndices = meshData.GetPointIndexCount();
    DPoint3dCP  verts = meshData.GetPointCP();
    int         polySize = meshData.GetNumPerFace();
    int         thisIndex, prevIndex=0, firstIndex=0;
    size_t      thisFaceSize = 0;
    bool        foundCurve = false;

    for (size_t readIndex = 0; readIndex < numIndices; readIndex++)
        {
        // found face loop entry
        if (thisIndex = vertIndex[readIndex])
            {
            if (!thisFaceSize)
                {
                // remember first index in this face loop
                firstIndex = thisIndex;
                }
            else if (prevIndex > 0)
                {
                // draw visible edge (prevIndex, thisIndex)
                int closeVertexId = (abs(prevIndex) - 1);
                int segmentVertexId = (abs(thisIndex) - 1);
                ICurvePrimitivePtr curve = ICurvePrimitive::CreateLine(DSegment3d::From(verts[closeVertexId], verts[segmentVertexId]));

                if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::None, HitParentGeomType::Mesh))
                    foundCurve = true;
                }

            prevIndex = thisIndex;
            thisFaceSize++;
            }

        // found end of face loop (found first pad/terminator or last index in fixed block)
        if (thisFaceSize && (!thisIndex || (polySize > 1 && polySize == thisFaceSize)))
            {
            // draw last visible edge (prevIndex, firstIndex)
            if (prevIndex > 0)
                {
                int closeVertexId = (abs(prevIndex) - 1);
                int segmentVertexId = (abs(firstIndex) - 1);
                ICurvePrimitivePtr curve = ICurvePrimitive::CreateLine(DSegment3d::From(verts[closeVertexId], verts[segmentVertexId]));

                if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::None, HitParentGeomType::Mesh))
                    foundCurve = true;
                }

            thisFaceSize = 0;
            }

        if (0 == (readIndex % 100) && _IsCanceled())
            return false;
        }

    return foundCurve;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessPolyface(PolyfaceQueryCR meshData, DPoint3dCR localPoint)
    {
    PolyfaceVisitorPtr  visitor = PolyfaceVisitor::Attach(meshData);
    size_t              readIndex = 0;
    double              tolerance = 1e37 /*fc_hugeVal*/;
    bool                updateClosePoint = m_closePtLocalCorrected.IsDisconnect();
    bool                found = false;

    visitor->SetNumWrap(1);

    for (; visitor->AdvanceToNextFace();)
        {
        DPoint3d thisFacePoint;

        if (!visitor->TryFindCloseFacetPoint(localPoint, tolerance, thisFacePoint))
            continue;

        // Save point on facet to correct "close point" from readPixels...
        if (updateClosePoint)
            m_closePtLocalCorrected = thisFacePoint;

        readIndex = visitor->GetReadIndex();
        tolerance = thisFacePoint.Distance(localPoint); // Refine tolerance...
        found = true;
        }

    if (!found)
        return false;

    // Get a "face" containing this facet, a single facet when there are hidden edges isn't what someone would consider a face...
    bvector<ptrdiff_t> seedReadIndices;
    bvector<ptrdiff_t> allFaceBlocks;
    bvector<ptrdiff_t> activeReadIndexBlocks;

    meshData.PartitionByConnectivity(2, allFaceBlocks);
    seedReadIndices.push_back(readIndex);
    PolyfaceHeader::SelectBlockedIndices(allFaceBlocks, seedReadIndices, true, activeReadIndexBlocks);

    bvector<PolyfaceHeaderPtr> perFacePolyfaces;

    meshData.CopyPartitions(activeReadIndexBlocks, perFacePolyfaces);

    return (0 != perFacePolyfaces.size() && ProcessSingleFacePolyface(*perFacePolyfaces.front(), localPoint));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessBody(IBRepEntityCR entity, DPoint3dCR localPoint)
    {
    return T_HOST.GetBRepGeometryAdmin()._SnapFindClosestCurve(entity, localPoint, m_snapDivisor, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessTextString(TextStringCR text, DPoint3dCR localPoint)
    {
    if (text.GetText().empty())
        return false;

    DPoint3d points[5];

    text.ComputeBoundingShape(points);
    text.ComputeTransform().Multiply(points, _countof(points));

    CurveVectorPtr curve = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateLineString(points, 5));

    return ProcessCurveVector(*curve, localPoint, HitGeomType::None, HitParentGeomType::Text);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessImageGraphic(ImageGraphicCR img, DPoint3dCR localPoint)
    {
    return ProcessCurveVector(*img.ToCurveVector(), localPoint, HitGeomType::Surface, HitParentGeomType::None);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessGeometry(GeometricPrimitiveR geom, TransformCR localToWorld, DRange3dP localRange)
    {
    Transform   worldToLocal;

    worldToLocal.InverseOf(localToWorld);

    DPoint3d    localPoint = GetClosePointLocal(worldToLocal);

    m_closePtLocalCorrected.InitDisconnect(); // Will be corrected to exact interior point by process methods...

    if (nullptr != localRange)
        {
        if (geom.GetRange(*localRange))
            {
            double maxOutsideDist = GetMaxOutsideDistance();
            double outsideDist = localRange->DistanceOutside(localPoint);

            if (outsideDist > maxOutsideDist)
                return false;
            }
        }

    switch (geom.GetGeometryType())
        {
        case GeometricPrimitive::GeometryType::CurvePrimitive:
            {
            ICurvePrimitiveCR curve = *geom.GetAsICurvePrimitive();

            if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString == curve.GetCurvePrimitiveType())
                return ProcessPointString(curve, localPoint);

            return ProcessICurvePrimitive(curve, localPoint, HitGeomType::None, HitParentGeomType::Wire);
            }

        case GeometricPrimitive::GeometryType::CurveVector:
            {
            CurveVectorCR curves = *geom.GetAsCurveVector();

            return ProcessCurveVector(curves, localPoint, HitGeomType::None, curves.IsAnyRegionType() ? HitParentGeomType::Sheet : HitParentGeomType::Wire);
            }

        case GeometricPrimitive::GeometryType::SolidPrimitive:
            {
            ISolidPrimitiveCR primitive = *geom.GetAsISolidPrimitive();

            return ProcessSolidPrimitive(primitive, localPoint, worldToLocal);
            }

        case GeometricPrimitive::GeometryType::BsplineSurface:
            {
            MSBsplineSurfaceCR surface = *geom.GetAsMSBsplineSurface();

            return ProcessBsplineSurface(surface, localPoint, HitParentGeomType::Sheet);
            }

        case GeometricPrimitive::GeometryType::Polyface:
            {
            PolyfaceQueryCR mesh = *geom.GetAsPolyfaceHeader();

            return ProcessPolyface(mesh, localPoint);
            }

        case GeometricPrimitive::GeometryType::BRepEntity:
            {
            IBRepEntityCR entity = *geom.GetAsIBRepEntity();

            return ProcessBody(entity, localPoint);
            }

        case GeometricPrimitive::GeometryType::TextString:
            {
            TextStringCR text = *geom.GetAsTextString();

            return ProcessTextString(text, localPoint);
            }

        case GeometricPrimitive::GeometryType::Image:
            return ProcessImageGraphic(*geom.GetAsImage(), localPoint);

        default:
            {
            BeAssert(false);
            return false;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessEntry(GeometryCollection::Iterator const& iter, bool preFiltered, DgnElementCP element)
    {
    GeometryStreamEntryId elemEntryId = iter.GetGeometryStreamEntryId();
    GeometricPrimitivePtr geom;

    if (nullptr != element && GeometryCollection::Iterator::EntryType::BRepEntity == iter.GetEntryType())
        {
        IBRepEntityPtr entity = BRepDataCache::FindCachedBRepEntity(*element, elemEntryId);

        if (entity.IsValid())
            {
            geom = GeometricPrimitive::Create(entity);
            }
        else
            {
            geom = iter.GetGeometryPtr();

            if (!geom.IsValid())
                return false;

            // Populate brep cache...
            BRepDataCache::AddCachedBRepEntity(*element, elemEntryId, *geom->GetAsIBRepEntity());
            }
        }

    if (!geom.IsValid())
        {
        geom = iter.GetGeometryPtr();

        if (!geom.IsValid())
            return false;
        }

    Transform localToWorld = iter.GetGeometryToWorld();
    DRange3d  localRange = DRange3d::NullRange();
    bool      checkRange = (!preFiltered && nullptr != element);
    bool      accepted = ProcessGeometry(*geom, localToWorld, checkRange ? &localRange : nullptr);

    if (checkRange && elemEntryId.GetGeometryPartId().IsValid() && elemEntryId.GetPartIndex() > 1)
        LocalRangeCache::AddCachedRange(*element, elemEntryId, localRange); // Cache multi-entry parts because they don't support sub-ranges...

    if (!accepted)
        return false;

    m_hitGeom = geom;
    m_hitParams = iter.GetGeometryParams();
    m_hitEntryId = elemEntryId;

    OnHitChanged(localToWorld);

    if (!preFiltered)
        {
        // If we found a curve/surface near the point from readPixels we can stop looking...
        if (m_hitDistanceLocal < 1.0e-3)
            return true;

        return false; // Keep going...
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SkipEntry(GeometryCollection::Iterator const& iter, GeometryStreamEntryIdCR snapElemEntryId, DgnSubCategoryId const* subCategoryId, DgnGeometryClass const* geomClass, ViewFlagsCP viewFlags, DgnElementCP element)
    {
    GeometryStreamEntryId elemEntryId = iter.GetGeometryStreamEntryId();

    if (snapElemEntryId.IsValid())
        return (snapElemEntryId != elemEntryId);

    GeometryParamsCR params = iter.GetGeometryParams();

    if (nullptr != subCategoryId && params.GetSubCategoryId() != *subCategoryId)
        return true;

    // Since patterns aren't stroked for snap ignore input geometry class...
    if (nullptr != geomClass && DgnGeometryClass::Pattern != *geomClass && params.GetGeometryClass() != *geomClass)
        return true;

    if (nullptr != viewFlags)
        {
        switch (params.GetGeometryClass())
            {
            case DgnGeometryClass::Construction:
                if (!viewFlags->ShowConstructions())
                    return true;
                break;

            case DgnGeometryClass::Dimension:
                if (!viewFlags->ShowDimensions())
                    return true;
                break;

            case DgnGeometryClass::Pattern:
                if (!viewFlags->ShowPatterns())
                    return true;
                break;
            }
        }

    bool checkRange = (nullptr != element);

    if (!checkRange)
        return false;

    DRange3d    localRange = iter.GetSubGraphicLocalRange();

    if (localRange.IsNull())
        {
        DgnGeometryPartId   partId = elemEntryId.GetGeometryPartId();

        if (partId.IsValid())
            {
            if (0 == elemEntryId.GetPartIndex())
                {
                // See if entire part can be skipped on range criteria w/o getting it's geometry stream...
                DRange3d partRange;

                if (SUCCESS == DgnGeometryPart::QueryGeometryPartRange(partRange, iter.GetDgnDb(), partId))
                    localRange = partRange;
                }
            else
                {
                // See if we have a cached range for this part's geometry entry since parts don't currently support sub-ranges...
                LocalRangeCache::FindCachedRange(localRange, *element, elemEntryId);
                }
            }
        }

    if (localRange.IsNull())
        return false;

    Transform   localToWorld = iter.GetGeometryToWorld();
    Transform   worldToLocal;

    worldToLocal.InverseOf(localToWorld);

    DPoint3d    localPoint = GetClosePointLocal(worldToLocal);
    double      maxOutsideDist = GetMaxOutsideDistance();
    double      outsideDist = localRange.DistanceOutside(localPoint);

    return (outsideDist > maxOutsideDist);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SnapStatus GetClosestCurve(GeometryCollection& collection, DgnElementCP element, DgnSubCategoryId const* subCategoryId, DgnGeometryClass const* geomClass, ViewFlagsCP viewFlags, ICancellableP cancel)
    {
    if (GeometryStreamIO::Header::Flags::None != (collection.GetHeaderFlags() & GeometryStreamIO::Header::Flags::ViewIndependent))
        return SnapStatus::NotSnappable;

    GeometryStreamEntryId  snapElemEntryId = m_hitEntryId;
    GeometryStreamEntryId  snapPartEntryId = m_hitEntryId;

    snapElemEntryId.SetPartIndex(0);
    m_cancel = cancel;

    for (auto iter : collection)
        {
        // Quick exclude of geometry that didn't generate the hit...
        if (SkipEntry(iter, snapElemEntryId, subCategoryId, geomClass, viewFlags, element))
            continue;

        if (GeometryCollection::Iterator::EntryType::GeometryPart != iter.GetEntryType())
            {
            if (ProcessEntry(iter, snapElemEntryId.IsValid(), element))
                break;

            if (_IsCanceled())
                return SnapStatus::Aborted;

            continue;
            }

        DgnGeometryPartCPtr geomPart = iter.GetGeometryPartCPtr();

        if (!geomPart.IsValid())
            continue; // Shouldn't happen...

        DgnGeometryPartCP  partElement = geomPart.get();
        GeometryCollection partCollection(geomPart->GetGeometryStream(), geomPart->GetDgnDb());

        partCollection.SetNestedIteratorContext(iter); // Iterate part GeomStream in context of parent...

        for (auto partIter : partCollection)
            {
            // Quick exclude of geometry that didn't generate the hit...
            if (SkipEntry(partIter, snapPartEntryId, subCategoryId, geomClass, viewFlags, partElement))
                continue;

            if (ProcessEntry(partIter, snapPartEntryId.IsValid(), partElement))
                break;

            if (_IsCanceled())
                return SnapStatus::Aborted;
            }

        if (snapPartEntryId.IsValid())
            break; // Done with part...
        }

    SimplifyHitDetail();

    return (nullptr != m_hitCurveDetail.curve || HitGeomType::Point == m_hitGeomType ? SnapStatus::Success : SnapStatus::NoSnapPossible);
    }

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SetHitGeometryStreamEntryId(GeometryStreamEntryIdCR entryId) {m_hitEntryId = entryId;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void OnHitChanged(TransformCR localToWorld)
    {
    m_hitLocalToWorld = localToWorld;
    m_hitWorldToLocal.InverseOf(m_hitLocalToWorld);

    DMatrix4d viewToLocal = GetViewToLocal(m_hitWorldToLocal);
    DMatrix4d localToView;

    localToView.QrInverseOf(viewToLocal);
    m_hitLocalToView.InitFrom(localToView, viewToLocal);

    DPoint3d localPts[2];
    DPoint4d viewPts[2];

    localPts[0] = m_hitClosePtLocal;
    localPts[1] = m_hitCurveDetail.point;

    m_hitLocalToView.M0.Multiply(viewPts, localPts, nullptr, 2);
    viewPts[0].RealDistanceXY(m_hitDistanceView, viewPts[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometricPrimitivePtr GetHitGeometry() const {return m_hitGeom;}
TransformCR GetHitLocalToWorld() const {return m_hitLocalToWorld;}
GeometryParamsCR GetHitGeometryParams() const {return m_hitParams;}
GeometryStreamEntryIdCR GetHitGeometryStreamEntryId() const {return m_hitEntryId;}
CurveLocationDetailCR GetHitCurveDetail() const {return m_hitCurveDetail;}
HitGeomType GetHitGeomType() const {return m_hitGeomType;}
HitParentGeomType GetHitParentGeomType() const {return m_hitParentGeomType;}
DPoint3d GetHitPointWorld() const {DPoint3d hitPtWorld = (m_closePtLocalCorrected.IsDisconnect() || HitGeomType::Point == m_hitGeomType || m_hitDistanceView <= m_snapAperture) ? m_hitCurveDetail.point : m_closePtLocalCorrected; m_hitLocalToWorld.Multiply(hitPtWorld); return hitPtWorld;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CurveLocationDetailCR GetSnapCurveDetail() const {return m_snapCurveDetail;}
ICurvePrimitivePtr const& GetSnapCurvePrimitivePtr() const {return m_hitCurveDerived;} // Valid when m_snapCurveDetail.curve is not nullptr...
HitGeomType GetSnapGeomType() const {return m_snapGeomType;}
DPoint3d GetSnapPointWorld() const {DPoint3d snapPtWorld = m_snapCurveDetail.point; m_hitLocalToWorld.Multiply(snapPtWorld); return snapPtWorld;}
bool IsSnapNormalValid() const {return 0.0 != m_snapNormalLocal.Magnitude();}
DVec3d GetSnapNormalWorld() const {DVec3d snapNormalWorld = m_snapNormalLocal; m_hitLocalToWorld.MultiplyMatrixOnly(snapNormalWorld); snapNormalWorld.Normalize(); return snapNormalWorld;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsPhysicallyClosed(ICurvePrimitiveCR primitive)
    {
    switch (primitive.GetCurvePrimitiveType())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            return (primitive.GetLineStringCP()->size() > 3 && primitive.GetLineStringCP()->front().IsEqual(primitive.GetLineStringCP()->back()));

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            return primitive.GetArcCP()->IsFullEllipse();

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
            return (primitive.GetProxyBsplineCurveCP()->IsClosed());

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool GetCentroid(DPoint3dR centroid, CurveVectorCR curve)
    {
    if (curve.IsAnyRegionType())
        {
        Transform   localToWorld, worldToLocal;
        DRange3d    localRange;

        if (curve.IsPlanar(localToWorld, worldToLocal, localRange))
            {
            DVec3d  normal;
            double  area;

            return curve.CentroidNormalArea(centroid, normal, area);
            }
        }

    double  length;

    return curve.WireCentroid(length, centroid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool GetCentroid(DPoint3dR centroid, ICurvePrimitiveCR primitive)
    {
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc == primitive.GetCurvePrimitiveType())
        {
        // Center snap always uses arc center...
        centroid = primitive.GetArcCP ()->center;

        return true;
        }
    else if (IsPhysicallyClosed(primitive))
        {
        CurveVectorPtr curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, primitive.Clone());

        // For physically closed/planar curve use area centroid instead of wire centroid...
        return GetCentroid(centroid, *curve);
        }

    double  length;

    return primitive.WireCentroid(length, centroid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static SnapHeat GetHeat(DPoint3dCR snapPoint, DPoint3dCR closePoint, DMatrix4dCR worldToView, double aperture, bool forceHot, double* distance = nullptr)
    {
    if (distance)
        *distance = 0.0;

    DPoint4d viewPts[2];
    worldToView.Multiply(&viewPts[0], &snapPoint, nullptr, 1);
    worldToView.Multiply(&viewPts[1], &closePoint, nullptr, 1);

    double viewDist = 0.0;
    if (!viewPts[0].RealDistanceXY(viewDist, viewPts[1]))
        return SNAP_HEAT_None;

    if (distance)
        *distance = viewDist;

    bool withinAperture = (viewDist <= aperture);
    return (withinAperture ? SNAP_HEAT_InRange : (forceHot ? SNAP_HEAT_NotInRange : SNAP_HEAT_None));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ComputeSnapLocation(SnapMode snapMode, bool findArcCenters) const
    {
    m_snapCurveDetail = m_hitCurveDetail;
    m_snapGeomType = m_hitGeomType;
    m_snapNormalLocal = DVec3d::FromZero();
    m_findArcCenters = findArcCenters;

    if (!EvaluateCurve(snapMode))
        return false;

    if (!EvaluateInterior(snapMode))
        EvaluateDefaultNormal();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SnapStatus GetClosestCurve(GeometrySourceCR source, DgnSubCategoryId const* subCategoryId, DgnGeometryClass const* geomClass, ViewFlagsCP viewFlags, ICancellableP cancel)
    {
    GeometryCollection collection(source);

    return GetClosestCurve(collection, source.ToElement(), subCategoryId, geomClass, viewFlags, cancel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SnapStatus GetClosestCurve(BeJsConst input, DgnDbR db, DgnSubCategoryId const* subCategoryId, DgnGeometryClass const* geomClass, ViewFlagsCP viewFlags, ICancellableP cancel)
    {
    GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(db, true); // I don't expect pickable decorations to reference GeometryParts...

    if (!builder->FromJson(input))
        return SnapStatus::NoElements;

    GeometryStream stream;

    if (SUCCESS != builder->GetGeometryStream(stream))
        return SnapStatus::NoElements;

    GeometryCollection collection(stream, db);

    return GetClosestCurve(collection, nullptr, subCategoryId, geomClass, viewFlags, cancel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SnapGeometryHelper(uint32_t divisor, double aperture, DPoint3d closePtWorld, DMap4d worldToView) : m_snapDivisor(divisor), m_snapAperture(aperture), m_closePtWorld(closePtWorld), m_worldToView(worldToView) {}

}; // SnapGeometryHelper
END_BENTLEY_DGN_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SnapContext::DoSnap(BeJsValue out, BeJsConst in, DgnDbR db, ICancellableR cancel)
    {
    // NEEDSWORK: Can we ignore snappable patterns/linestyles? These will locate currently, snap will just go to "base" geometry...
    SnapContext::Response output(out);
    SnapContext::Request input(in);
    output.SetStatus(SnapStatus::BadArg);

    // may have been aborted while it was in the queue. If so, don't even start
    if (cancel.IsCanceled() || !input.IsValid())
        return;

    bset<SnapMode> snapModes = input.GetSnapModes();
    if (snapModes.empty())
        return;

    output.SetStatus(SnapStatus::NoElements);
    DgnElementId elementId = input.GetElementId();
    if (!elementId.IsValid())
        return;

    DgnElementCPtr element;
    GeometrySourceCP source = nullptr;
    auto nonElemGeomMap = input.GetNonElementGeometry();
    auto foundNonElemGeom = nonElemGeomMap.find(elementId);

    if (foundNonElemGeom == nonElemGeomMap.end())
        {
        element = db.Elements().GetElement(elementId);
        source = element.IsValid() ? element->ToGeometrySource() : nullptr;
        if (nullptr == source)
            return;
        }

    DPoint3d testPoint = input.GetTestPoint();
    DPoint3d closePoint = input.GetClosePoint();
    DMatrix4d worldToView = input.GetWorldToView(), viewToWorld; viewToWorld.QrInverseOf(worldToView);
    DMap4d worldToViewMap = DMap4d::From(worldToView, viewToWorld);

    // Hot distance in view coordinates (pixels). Locate aperture * hot distance factor...
    double snapAperture = input.GetSnapAperture();
    uint32_t snapDivisor = input.GetSnapDivisor();
    bool doPlacementOriginSnap = (nullptr != source && 0 != snapModes.erase(SnapMode::Origin)); // When using placement origin, remove origin from geometry snap modes...
    bool doCenterSnap = (snapModes.end() != snapModes.find(SnapMode::Center)); // Let center snap handle arc centers when present in multi-snap...
    SnapData bestSnap, intersectSnap;

    DgnElementIdSet intersectCandidates;
    if (snapModes.end() != snapModes.find(SnapMode::Intersection))
        intersectCandidates = input.GetIntersectCandidates(elementId);
    if (intersectCandidates.empty())
        snapModes.erase(SnapMode::Intersection);

    ViewFlags viewFlags = input.GetViewFlags();
    DgnSubCategoryId subCategoryId;
    uint32_t geomClassInt = 0;
    bool haveSubCategory = input.GetSubCategoryId(subCategoryId);
    bool haveGeomClass = input.GetGeometryClass(geomClassInt) && haveSubCategory; // Non-element pick can return primary with an invalid subCategory, ignore...
    DgnGeometryClass geomClass = haveGeomClass ? (DgnGeometryClass) geomClassInt : DgnGeometryClass::Primary;

    if (!snapModes.empty())
        {
        SnapGeometryHelper helper(snapDivisor, snapAperture, closePoint, worldToViewMap);
        SnapStatus status = SnapStatus::NoSnapPossible;

        if (nullptr != source)
            status = helper.GetClosestCurve(*source, haveSubCategory ? &subCategoryId : nullptr, haveGeomClass ? &geomClass : nullptr, haveGeomClass ? nullptr : &viewFlags, &cancel);
        else
            status = helper.GetClosestCurve(foundNonElemGeom->second, db, nullptr, nullptr, &viewFlags, &cancel);

        if (SnapStatus::Success != status)
            {
            if (!doPlacementOriginSnap)
                {
                output.SetStatus(status);
                return;
                }

            snapModes.clear();
            }

        for (SnapMode snapMode : snapModes)
            {
            if (!helper.ComputeSnapLocation(SnapMode::Intersection == snapMode ? SnapMode::Nearest : snapMode, !doCenterSnap))
                continue;

            SnapData currentSnap(snapMode, helper.GetSnapPointWorld());

            currentSnap.m_hitPoint = helper.GetHitPointWorld(); // NOTE: This is corrected close point which will be used when snap isn't hot...
            currentSnap.m_heat = SnapGeometryHelper::GetHeat(currentSnap.m_snapPoint, closePoint, worldToViewMap.M0, snapAperture, SnapMode::Center == snapMode, &currentSnap.m_viewDistance);
            currentSnap.m_geomType = helper.GetSnapGeomType();
            currentSnap.m_parentGeomType = helper.GetHitParentGeomType();
            currentSnap.m_interiorWasPickable = (RenderMode::Wireframe != viewFlags.GetRenderMode() || (FillDisplay::ByView == helper.GetHitGeometryParams().GetFillDisplay() && viewFlags.ShowFill()) || FillDisplay::ByView < helper.GetHitGeometryParams().GetFillDisplay());

            if (helper.IsSnapNormalValid())
                currentSnap.m_snapNormal = helper.GetSnapNormalWorld();

            if (!helper.GetHitLocalToWorld().IsIdentity())
                currentSnap.m_localToWorld = helper.GetHitLocalToWorld();

            if (nullptr != helper.GetSnapCurveDetail().curve)
                currentSnap.m_geomPtr = IGeometry::Create(helper.GetSnapCurvePrimitivePtr());

            if (SnapMode::Intersection == snapMode)
                {
                if (currentSnap.m_geomPtr.IsValid())
                    intersectSnap = currentSnap;
                continue;
                }

            if (bestSnap.UpdateIfBetter(currentSnap, testPoint, worldToViewMap.M0, snapAperture))
               break;
            }
        }

    if (doPlacementOriginSnap)
        {
        // NOTE: Don't have corrected hitPoint, but since it's always hot it shouldn't get used...
        SnapData originSnap(SnapMode::Origin, source->GetPlacementTransform().Origin());

        originSnap.m_heat = SnapGeometryHelper::GetHeat(originSnap.m_snapPoint, closePoint, worldToViewMap.M0, snapAperture, true, &originSnap.m_viewDistance);

        bestSnap.UpdateIfBetter(originSnap, testPoint, worldToViewMap.M0, snapAperture);
        }

    if (SnapMode::Invalid != intersectSnap.m_mode && !bestSnap.IsInRange())
        {
        for (DgnElementId candidateId : intersectCandidates)
            {
            DgnElementCPtr candidateElement;
            GeometrySourceCP candidateSource = nullptr;
            auto foundCandidateNonElemGeom = nonElemGeomMap.find(candidateId);

            if (foundCandidateNonElemGeom == nonElemGeomMap.end())
                {
                candidateElement = db.Elements().GetElement(candidateId);
                candidateSource = candidateElement.IsValid() ? candidateElement->ToGeometrySource() : nullptr;
                if (nullptr == candidateSource)
                    continue;
                }

            SnapGeometryHelper helper(snapDivisor, snapAperture, closePoint, worldToViewMap);
            SnapStatus status = SnapStatus::NoSnapPossible;

            // NOTE: Since the subCategory for intersection candidates isn't currently supplied, we can potentially intersect with geometry on a subCategory that isn't displayed.
            //       The viewFlags suffice to ensure a displayed GeometryClass.
            if (nullptr != candidateSource)
                status = helper.GetClosestCurve(*candidateSource, nullptr, nullptr, &viewFlags, &cancel);
            else
                status = helper.GetClosestCurve(foundCandidateNonElemGeom->second, db, nullptr, nullptr, &viewFlags, &cancel);

            if (SnapStatus::Aborted == status)
                break;

            if (SnapStatus::Success != status || !helper.GetSnapCurvePrimitivePtr().IsValid())
                continue;

            ICurvePrimitivePtr curveA = intersectSnap.m_geomPtr->GetAsICurvePrimitive()->Clone(intersectSnap.m_localToWorld);
            ICurvePrimitivePtr curveB = helper.GetSnapCurvePrimitivePtr()->Clone(helper.GetHitLocalToWorld());

            CurveVectorPtr intersectionsA = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);
            CurveVectorPtr intersectionsB = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);

            CurveCurve::IntersectionsXY(*intersectionsA, *intersectionsB, curveA.get(), curveB.get(), &worldToViewMap.M0);

            CurveLocationDetail intersectDetail;
            if (!intersectionsA->ClosestPointBoundedXY(closePoint, &worldToViewMap.M0, intersectDetail))
                continue;

            PartialCurveDetailCP partialCurve = (intersectDetail.curve ? intersectDetail.curve->GetPartialCurveDetailCP() : nullptr);
            if (!partialCurve || !partialCurve->IsSingleFraction())
                continue; // Reject intersection that isn't a single point...

            intersectSnap.m_snapPoint = intersectDetail.point;
            intersectSnap.m_heat = SnapGeometryHelper::GetHeat(intersectSnap.m_snapPoint, closePoint, worldToViewMap.M0, snapAperture, false, &intersectSnap.m_viewDistance);
            intersectSnap.m_intersectGeomPtr = IGeometry::Create(curveB); // Saved in world coords...
            intersectSnap.m_intersectId = candidateId;

            if (bestSnap.UpdateIfBetter(intersectSnap, testPoint, worldToViewMap.M0, snapAperture))
                break;
            }
        }

    bestSnap.ToResponse(output);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
KeypointType SnapContext::GetSnapKeypointType(SnapMode snapMode)
    {
    KeypointType    keypointType = KEYPOINT_TYPE_Unknown;

    switch (snapMode)
        {
        case SnapMode::Nearest:
            keypointType = KEYPOINT_TYPE_Nearest;
            break;
        case SnapMode::NearestKeypoint:
            keypointType = KEYPOINT_TYPE_Keypoint;
            break;
        case SnapMode::MidPoint:
            keypointType = KEYPOINT_TYPE_Midpoint;
            break;
        case SnapMode::Center:
            keypointType = KEYPOINT_TYPE_Center;
            break;
        case SnapMode::Origin:
            keypointType = KEYPOINT_TYPE_Origin;
            break;
        case SnapMode::Bisector:
            keypointType = KEYPOINT_TYPE_Bisector;
            break;
        case SnapMode::Intersection:
            keypointType = KEYPOINT_TYPE_Intersection;
            break;
        case SnapMode::Tangency:
            keypointType = KEYPOINT_TYPE_Tangent;
            break;
        case SnapMode::TangentPoint:
            keypointType = KEYPOINT_TYPE_Tangentpoint;
            break;
        case SnapMode::Perpendicular:
            keypointType = KEYPOINT_TYPE_Perpendicular;
            break;
        case SnapMode::PerpendicularPoint:
            keypointType = KEYPOINT_TYPE_Perpendicularpt;
            break;
        case SnapMode::Parallel:
            keypointType = KEYPOINT_TYPE_Parallel;
            break;
        case SnapMode::PointOn:
            keypointType = KEYPOINT_TYPE_PointOn;
            break;
        }

    return keypointType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SnapContext::GetParameterKeypoint(ICurvePrimitiveCR curve, DPoint3dR hitPoint, double& keyParam, uint32_t divisor)
    {
    double  length, subLength;

    if (!curve.Length(length) || !curve.SignedDistanceBetweenFractions(0.0, keyParam, subLength))
        return false;

    // NOTE: For a closed curve we want quadrant key points.
    if (SnapGeometryHelper::IsPhysicallyClosed(curve))
        divisor *= 4;

    uint32_t    keySeg = uint32_t ((subLength / length) * (double) divisor + 0.5);
    double      keyDist = (keySeg / (double) divisor) * length;

    CurveLocationDetail location;

    if (!curve.PointAtSignedDistanceFromFraction(0.0, keyDist, false, location))
        return false;

    hitPoint = location.point;
    keyParam = location.fraction;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SnapContext::GetSegmentKeypoint(DPoint3dR hitPoint, double& keyParam, uint32_t divisor, DSegment3dCR segment)
    {
    uint32_t numerator = uint32_t ((keyParam * (double) divisor) + 0.5);

    keyParam = (numerator / (double) divisor);
    hitPoint.Interpolate(segment.point[0], keyParam, segment.point[1]);
    }

