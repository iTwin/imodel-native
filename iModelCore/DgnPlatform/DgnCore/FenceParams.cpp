/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/FenceParams.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/VecMath.h>

#define NPC_CAMERA_LIMIT    100.0

BEGIN_BENTLEY_DGN_NAMESPACE
/*=================================================================================**//**
* Context to determine if element should be accepted for fence processing..
* @bsiclass                                                     Brien.Bastings  09/04
+===============+===============+===============+===============+===============+======*/
struct FenceAcceptContext : ViewContext, IGeometryProcessor
{
    DEFINE_T_SUPER(ViewContext)
private:

FenceParamsR        m_fp;
DgnElementIdSet     m_contents;
bool                m_earlyDecision;
bool                m_currentAccept;
bool                m_accept;
bool                m_firstAccept;
FenceCheckStop*     m_checkStop;

public:

FenceAcceptContext(FenceParamsR fp, FenceCheckStop* checkStop = nullptr) : m_fp(fp), m_checkStop(checkStop)
    {
    m_purpose = DrawPurpose::FenceAccept;
    m_ignoreViewRange = true;

    OnNewGeometrySource();
    }

~FenceAcceptContext() {}

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicBuilderPtr _CreateGraphic(Render::Graphic::CreateParams const& params) override
    {
    return new SimplifyGraphic(params, *this, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr _CreateBranch(Render::GraphicBranch& branch, TransformCP trans, ClipVectorCP clips) override
    {
    return new SimplifyGraphic(Render::Graphic::CreateParams(), *this, *this);
    }

UnhandledPreference _GetUnhandledPreference(CurveVectorCR, SimplifyGraphic&) const override {return UnhandledPreference::Curve;} // If view has clipping...
UnhandledPreference _GetUnhandledPreference(ISolidPrimitiveCR, SimplifyGraphic&) const override {return UnhandledPreference::Curve;}
UnhandledPreference _GetUnhandledPreference(MSBsplineSurfaceCR, SimplifyGraphic&) const override {return UnhandledPreference::Curve;}
UnhandledPreference _GetUnhandledPreference(PolyfaceQueryCR, SimplifyGraphic&) const override {return UnhandledPreference::Curve;} // BAD - NEEDSWORK...
UnhandledPreference _GetUnhandledPreference(IBRepEntityCR, SimplifyGraphic&) const override {return UnhandledPreference::Curve;}
UnhandledPreference _GetUnhandledPreference(TextStringCR, SimplifyGraphic&) const override {return UnhandledPreference::Box;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/04
+---------------+---------------+---------------+---------------+---------------+------*/
void OnNewGeometrySource()
    {
    m_fp.SetHasOverlaps(false);

    m_firstAccept   = true;
    m_currentAccept = false;
    m_accept        = false;
    m_earlyDecision = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/04
+---------------+---------------+---------------+---------------+---------------+------*/
void CheckCurrentAccept()
    {
    if (m_earlyDecision) // accept status has already been determined...
        return;

    bool insideMode = !m_fp.AllowOverlaps() && FenceClipMode::None == m_fp.GetClipMode();
    bool hasOverlap = !m_firstAccept && (m_currentAccept != m_accept);

    if (m_currentAccept)
        {
        m_accept = true;
        }
    else if (insideMode)
        {
        m_accept = false;
        m_earlyDecision = true;
        }

    if (!insideMode && m_accept)
        {
        // Fence can overlap an element that has disjoint geometric primitives w/o any single primitive having an overlap...
        if (hasOverlap)
            m_fp.SetHasOverlaps(true);

        // Need to look for ALL overlaps in clip mode...
        if (FenceClipMode::None == m_fp.GetClipMode())
            m_earlyDecision = m_fp.HasOverlaps();
        }

    m_firstAccept = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool _CheckStop() override
    {
    // NOTE: Don't set abort flag set for "early decision", it's not reset between elements; only set for "user events"...    
    if (WasAborted() || (nullptr != m_checkStop && AddAbortTest(m_checkStop->_CheckStopFenceContents())))
        return true;

    return m_earlyDecision;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void _DrawAreaPattern(Render::GraphicBuilderR graphic, CurveVectorCR boundary, Render::GeometryParamsR params, bool doCook) override
    {
#if defined (NEEDSWORK_FENCE)
    FenceParamsP    fp = m_graphic->GetFenceParamsP ();

    if (fp->HasOverlaps())
        return; // Already have overlap w/element don't need to draw the pattern...

    if (_CheckStop())
        return;

    if (!_WantAreaPatterns())
        return;

    fp.SetHasOverlaps(false);

    if (FenceClipMode::None == fp->GetClipMode()) // Need to draw patterns for interior overlap check when clipping...
        {
        // Attempt to short circuit fence accept using boundary...only check symbol geometry for interior overlap...
        boundary.GetGeomSource().Stroke(*this);

        // Element never rejected by pattern...so if boundary is acceptable we can skip drawing the pattern...
        if (_CheckStop() || m_graphic->GetCurrentAccept())
            return;

        fp.SetHasOverlaps(false);
        }

    // Keep looking for overlaps using pattern geometry...
    T_Super::_DrawAreaPattern(boundary);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessPoints(DPoint3dCP points, int numPoints, SimplifyGraphic& graphic)
    {
    Transform transform = graphic.GetLocalToWorldTransform();

    for (int iPoint = 0; iPoint < numPoints; iPoint++)
        {
        DPoint3d tmpPt;

        transform.Multiply(tmpPt, points[iPoint]);
        m_currentAccept = m_fp.PointInside(tmpPt);
        CheckCurrentAccept();

        if (CheckStop())
            break;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessLinearSegments(DPoint3dCP points, int numPoints, bool closed, bool filled, SimplifyGraphic& graphic)
    {
    Transform transform = graphic.GetLocalToWorldTransform();

    if (m_fp.AcceptByCurve() && numPoints > 1)
        {
        MSBsplineCurve curve;

        if (SUCCESS != curve.InitFromPoints(points, numPoints))
            return true;

        curve.TransformCurve(transform);
        m_currentAccept = m_fp.AcceptCurve(curve);
        curve.ReleaseMem();
        }
    else
        {
        DPoint3dP tmpPtsP = (DPoint3dP) alloca(numPoints * sizeof(DPoint3d));

        transform.Multiply(tmpPtsP, points, numPoints);
        m_currentAccept = m_fp.AcceptLineSegments(tmpPtsP, numPoints, closed);
        }

    CheckCurrentAccept();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessDEllipse3d(DEllipse3dCR ellipse, bool closed, bool filled, SimplifyGraphic& graphic)
    {
    Transform transform = graphic.GetLocalToWorldTransform();
    DEllipse3d tmpEllipse;

    transform.Multiply(tmpEllipse, ellipse);

    if (m_fp.AcceptByCurve())
        {
        MSBsplineCurve curve;

        if (SUCCESS != curve.InitFromDEllipse3d(tmpEllipse))
            return true;

        m_currentAccept = m_fp.AcceptCurve(curve);
        curve.ReleaseMem();
        }
    else
        {
        m_currentAccept = m_fp.AcceptDEllipse3d(tmpEllipse);
        }

    CheckCurrentAccept();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessCurve(MSBsplineCurveCR geom, bool filled, SimplifyGraphic& graphic)
    {
    Transform transform = graphic.GetLocalToWorldTransform();
    MSBsplineCurvePtr copy = geom.CreateCopyTransformed(transform);

    m_currentAccept = m_fp.AcceptCurve(*copy);
    CheckCurrentAccept();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool _ProcessCurvePrimitive(ICurvePrimitiveCR primitive, bool closed, bool filled, SimplifyGraphic& graphic) override
    {
    switch (primitive.GetCurvePrimitiveType())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            {
            DSegment3dCP segment = primitive.GetLineCP();

            return ProcessLinearSegments(segment->point, 2, closed, filled, graphic);
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            {
            bvector<DPoint3d> const* points = primitive.GetLineStringCP();

            return ProcessLinearSegments(&points->front(), (int) points->size(), closed, filled, graphic);
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            {
            DEllipse3dCP ellipse = primitive.GetArcCP();

            return ProcessDEllipse3d(*ellipse, closed, filled, graphic);
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
            {
            bvector<DPoint3d> const* points = primitive.GetPointStringCP();

            return ProcessPoints(&points->front(), (int) points->size(), graphic);
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
        default:
            {
            // NOTE: default case handles bcurves to accomdate future proxy bcurve additions...
            MSBsplineCurveCP bcurve = primitive.GetProxyBsplineCurveCP();

            if (nullptr != bcurve)            
                return ProcessCurve(*bcurve, filled, graphic);

            return false; // SimplifyGraphic will recurse for ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector and call us with primitives...
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool _ProcessCurveVector(CurveVectorCR curves, bool filled, SimplifyGraphic& graphic) override
    {
    graphic.ProcessAsCurvePrimitives(curves, filled);

    // If already detected overlap (or not a region/filled in wireframe), can skip interior check... 
    if (m_fp.HasOverlaps() || !curves.IsAnyRegionType() || LocateSurfacesPref::Never == m_fp.GetLocateInteriors())
        return true;

    if (LocateSurfacesPref::ByView == m_fp.GetLocateInteriors() && RenderMode::Wireframe == GetViewFlags().GetRenderMode() && !filled)
        return true;

    Transform worldToLocal, localToWorld = graphic.GetLocalToWorldTransform();

    worldToLocal.InverseOf(localToWorld);

    for (ClipPrimitivePtr const& primitive : *m_fp.GetClipVector())
        {
        if (CheckStop())
            break;

        ClipPlaneSetCP planeSet = primitive->GetMaskOrClipPlanes();

        if (nullptr == planeSet)
            continue;

        for (ConvexClipPlaneSetCR convexPlaneSet : *planeSet)
            {
            if (CheckStop())
                break;

            for (ClipPlaneCR clipPlane : convexPlaneSet)
                {
                bvector<CurveLocationDetailPair> intersections;
                DPlane3d plane = clipPlane.GetDPlane3d();

                worldToLocal.Multiply(plane);

                if (!curves.AppendClosedCurvePlaneIntersections(plane, intersections) || intersections.empty())
                    continue;

#if defined (NOT_NOW) // Just testing, really need CurveVector method that takes the entire ClipPlaneSet...maybe try checking if both "ends" of intersection curve are interior???
                m_currentAccept = true;
                m_fp.SetHasOverlaps(true);
                CheckCurrentAccept();
#endif

                return true;
                }
            }
        }

#if defined (NOT_NOW)
    DRange3d    localRange;
    Transform   localToWorld, worldToLocal;

    CurveVectorPtr curvesLocal = geom.CloneInLocalCoordinates(LOCAL_COORDINATE_SCALE_01RangeBothAxes, localToWorld, worldToLocal, localRange);

    if (curvesLocal.IsValid())
        {
        bvector<DRay3d> boresiteVector;

        GetBoresiteLocations(boresiteVector);

        for (DRay3dR boresite: boresiteVector)
            {
            double      t;
            DPoint3d    uvw;

            if (!boresite.IntersectZPlane(localToWorld, 0.0, uvw, t))
                continue;

            CurveVector::InOutClassification inOut = curvesLocal->PointInOnOutXY (uvw);

            if (CurveVector::INOUT_In != inOut && CurveVector::INOUT_On != inOut)
                continue;

            m_currentAccept = true;
            m_fp.SetHasOverlaps(true);
            CheckCurrentAccept();
            break;
            }
        }
#endif

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool _ProcessSolidPrimitive(ISolidPrimitiveCR solid, SimplifyGraphic& graphic) override
    {
#if defined (NOT_NOW)
    // NOTE: Always return ERROR for default edge processing...
    if (m_fp->HasOverlaps())
        return ERROR; // Already detected overlap, can skip interior check...

    if (RenderMode::Wireframe == GetViewFlags().GetRenderMode() && !isFilled)
        return ERROR;

    bvector<DRay3d> boresiteVector;

    GetBoresiteLocations(boresiteVector);

    for (DRay3dR boresite: boresiteVector)
        {
        bvector<SolidLocationDetail> intersectLocationDetail;

        primitive.AddRayIntersections(intersectLocationDetail, boresite);

        if (0 == intersectLocationDetail.size())
            continue;

        m_currentAccept = true;
        m_fp->SetHasOverlaps(true);
        CheckCurrentAccept();
        break;
        }

    return ERROR;
#else
    return false;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool _ProcessSurface(MSBsplineSurfaceCR surface, SimplifyGraphic& graphic) override
    {
#if defined (NOT_NOW)
    // NOTE: Always return ERROR for default edge processing...
    if (m_fp->HasOverlaps())
        return ERROR; // Already detected overlap, can skip interior check...

    if (RenderMode::Wireframe == GetViewFlags().GetRenderMode() && !isFilled)
        return ERROR;

    double          uorRes = bspsurf_getResolution(&surface);
    bvector<DRay3d> boresiteVector;

    GetBoresiteLocations(boresiteVector);

    for (DRay3dR boresite: boresiteVector)
        {
        int     nHits = 0;

        bsprsurf_allBoresiteToSurface(NULL, NULL, &nHits, &boresite.origin, &boresite.direction, const_cast <MSBsplineSurfaceP> (&surface), &uorRes);

        if (0 == nHits)
            continue;

        m_currentAccept = true;
        m_fp->SetHasOverlaps(true);
        CheckCurrentAccept();
        break;
        }

    return ERROR;
#else
    return false;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool _ProcessPolyface(PolyfaceQueryCR polyface, bool filled, SimplifyGraphic& graphic) override
    {
#if defined (NOT_NOW)
    // NOTE: Always return SUCCESS, don't want default edge processing...
    if (m_fp->HasOverlaps())
        {
        ClipAndProcessFacetSetAsCurves(meshData);

        return SUCCESS; // Already detected overlap, can skip interior check...
        }

    if (RenderMode::Wireframe == GetViewFlags().GetRenderMode() && !isFilled)
        {
        ClipAndProcessFacetSetAsCurves(meshData);

        return SUCCESS;
        }

    PolyfaceVisitorPtr  visitor = PolyfaceVisitor::Attach(meshData);
    bvector<DRay3d>     boresiteVector;

    GetBoresiteLocations(boresiteVector);

    for (; visitor->AdvanceToNextFace(); )
        {
        if (m_context->CheckStop())
            return SUCCESS;

        for (DRay3dR boresite: boresiteVector)
            {
            FacetLocationDetail  facetDetail;

            if (!visitor->TryDRay3dIntersectionToFacetLocationDetail(boresite, facetDetail))
                continue;

            m_currentAccept = true;
            m_fp->SetHasOverlaps(true);
            CheckCurrentAccept();
            break;
            }

        if (m_fp->HasOverlaps())
            break;
        }

    ClipAndProcessFacetSetAsCurves(meshData);

    return SUCCESS;
#else
    return false;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt _VisitGeometry(GeometrySourceCR source) override
    {
    if (SUCCESS == T_Super::_VisitGeometry(source) && m_accept)
        {
        if (nullptr != source.ToElement())
            m_contents.insert(source.ToElement()->GetElementId());
        }

    OnNewGeometrySource(); // Reset accept status and continue checking next element...

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/14
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt _VisitDgnModel(GeometricModelR inDgnModel) override
    {
    // Always ignore elements that are not from the context's target dgnDb...
    if (&inDgnModel.GetDgnDb() != &GetDgnDb())
        return ERROR;

    return T_Super::_VisitDgnModel(inDgnModel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool AcceptGeometrySource(GeometrySourceCR source)
    {
    if (SUCCESS != Attach(m_fp.GetViewport(), m_purpose))
        return false;

    return (SUCCESS == T_Super::_VisitGeometry(source) && m_accept);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool AcceptCurveVector(CurveVectorCR curves)
    {
    if (SUCCESS != Attach(m_fp.GetViewport(), m_purpose))
        return false;

    Render::GraphicBuilderPtr graphic = CreateGraphic(Graphic::CreateParams(GetViewport()));

    graphic->AddCurveVector(curves, false);

    return m_accept;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GetContents(DgnElementIdSet& contents)
    {
    DRange3d npcRange = m_fp.GetFenceRangeNPC ();

    if (!npcRange.IsNull ())
        SetSubRectNpc(npcRange);

    if (SUCCESS != Attach(m_fp.GetViewport(), m_purpose) || !m_fp.GetClipVector().IsValid())
        return ERROR;

    VisitAllViewElements();

    if (m_contents.empty() || WasAborted())
        return ERROR;

    contents = m_contents;

    return SUCCESS;
    }

}; // FenceAcceptContext
END_BENTLEY_DGN_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/07
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVectorPtr   FenceParams::GetClipVector() const {return m_clip;}
FenceClipMode   FenceParams::GetClipMode() const {return m_clipMode;}
DgnViewportP    FenceParams::GetViewport() const {BeAssert(m_viewport && "Fence viewport must not be NULL"); return m_viewport;}
void            FenceParams::SetOverlapMode(bool val) {m_overlapMode = val;}
void            FenceParams::SetClipMode(FenceClipMode val) {m_clipMode = val;}
void            FenceParams::SetClip(ClipVectorCR clip) {m_clip = ClipVector::CreateCopy(clip);}
void            FenceParams::SetLocateInteriors(LocateSurfacesPref interiors) {m_locateInteriors = interiors;}
bool            FenceParams::HasOverlaps() const {return m_hasOverlaps;}
bool            FenceParams::AllowOverlaps() const {return m_overlapMode;}

/*---------------------------------------------------------------------------------**//**
| @param view => selects view whose transformation is applied.
|               Invalid view sets up identity transformation.
| @bsimethod                                                    RayBentley      7/92
+---------------+---------------+---------------+---------------+---------------+------*/
void FenceParams::SetViewParams(DgnViewportP viewport)
    {
    // Save viewport for use with fence processing callbacks...
    m_viewport = viewport;

    // Reduced from .25 for XM to 1.0E-4 (now that we use this uniformly I think smaller value makes sense). RBB/RBB 03/06.
    // Increased to 1.5E-3 to match/exceed CLIP_PLANE_BIAS. RBB 03/2011. (TR# 293934).
    m_onTolerance = 1.5E-3;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void setFenceRangeFromInsideClip(DRange3dR fenceRange, ClipVectorCR clip, DgnViewportP vp)
    {
    ClipPrimitiveCP     clipPrimitive = clip.front().get();
    ClipPolygonCP       clipPolygon;

    if (clip.empty() ||
        NULL == (clipPrimitive = clip.front().get()) ||
        NULL == (clipPolygon = clipPrimitive->GetPolygon()))
        {
        BeAssert(false);
        }

    if (clipPrimitive->ClipZLow() && clipPrimitive->ClipZHigh())
        {
        DRange3d    range;

        if (!clipPrimitive->GetRange(range, NULL))
            return;

        fenceRange = range;

        return;
        }

    if (NULL == vp)
        return;

    DRange3d    range;

    if (!clipPrimitive->GetRange(range, clipPrimitive->GetTransformToClip())) // Returns range in clip coordinates.
        return;

    DPoint3d    fenceOrigin;
    DVec3d      fenceNormal;

    fenceOrigin.Init(range.low.x, range.low.y);
    fenceNormal.Init(0.0, 0.0, 1.0);

    if (NULL != clipPrimitive->GetTransformFromClip())
        {
        clipPrimitive->GetTransformFromClip()->Multiply(fenceOrigin);
        clipPrimitive->GetTransformFromClip()->MultiplyMatrixOnly(fenceNormal);
        }

    DPoint3d    viewBox[8];
    DRange3d    viewRange;
    Frustum     viewFrustum = vp->GetFrustum(DgnCoordSystem::World, true);

    memcpy(viewBox, viewFrustum.GetPts(), sizeof (viewBox));
    viewRange.InitFrom(viewBox, 8);

    double      minDepth, maxDepth;

    LegacyMath::Vec::ComputeRangeProjection(&minDepth, &maxDepth, &viewRange.low, &viewRange.high, &fenceOrigin, &fenceNormal);

    if (minDepth >= maxDepth)
        return;

    if (!clipPrimitive->ClipZLow())
        range.low.z = minDepth;

    if (!clipPrimitive->ClipZHigh())
        range.high.z = maxDepth;

    DPoint3d    corners[8];

    range.Get8Corners(corners);
    if (NULL != clipPrimitive->GetTransformFromClip())
        clipPrimitive->GetTransformFromClip()->Multiply(corners, 8);

    range.InitFrom(corners, 8);
    range.IntersectionOf(range, viewRange);

    if (!range.IsNull())
        fenceRange = range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      7/92
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt FenceParams::StoreClippingVector(ClipVectorCR clip, bool outside)
    {
    m_clip = ClipVector::CreateCopy(clip);

    if (!m_clip.IsValid())
        return ERROR;

    m_fenceRangeNPC.Init ();
    if (!outside)
        setFenceRangeFromInsideClip(m_fenceRangeNPC, *m_clip, m_viewport);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/16
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt FenceParams::StoreClippingPoints(DPoint3dCP points, size_t nPoints, bool outside)
    {
    if (nullptr == m_viewport || nPoints < 2)
        return ERROR;

    DMap4dCP worldToNPC = m_viewport->GetWorldToNpcMap();
    bvector<DPoint2d> backPlaneUV;
    DRange3d uvRange = DRange3d::NullRange();

    for (size_t i = 0; i < nPoints; i++)
        {
        DPoint3d uvw;
        worldToNPC->M0.MultiplyAndRenormalize(&uvw, &points[i], 1);
        backPlaneUV.push_back(DPoint2d::From(uvw.x, uvw.y));
        uvRange.Extend(uvw.x, uvw.y, 0.0);
        }

    if (2 == nPoints) // Degenerate fence shape is used for crossing line selection...
        backPlaneUV.push_back(backPlaneUV.front());

    m_clip = ClipVector::CreateFromPrimitive(ClipPrimitive::CreateFromShape(&backPlaneUV.front(), backPlaneUV.size(), outside, nullptr, nullptr, nullptr).get());
    m_clip->MultiplyPlanesTimesMatrix(worldToNPC->M0);
    m_fenceRangeNPC.Init ();

    if (!outside)
        {
        m_fenceRangeNPC = uvRange;
        m_fenceRangeNPC.low.z = -1.0e20;
        m_fenceRangeNPC.high.z = NPC_CAMERA_LIMIT;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2008
+---------------+---------------+---------------+---------------+---------------+------*/
bool FenceParams::PointInOtherClips(DPoint3dCR testPoint, ClipPrimitiveCP thisPrimitive)
    {
    for (ClipPrimitivePtr const& primitive: *m_clip)
        {
        if (primitive.get() != thisPrimitive && !primitive->PointInside(testPoint, m_onTolerance))
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Test a single point againts z limits and the AND of the clip list
* @bsimethod                                                    RayBentley      7/92
+---------------+---------------+---------------+---------------+---------------+------*/
bool FenceParams::PointInsideClip(DPoint3dCR testPoint)
    {
    return m_clip.IsValid() ? m_clip->PointInside(testPoint, m_onTolerance) : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt FenceParams::GetClipToWorldTransform(TransformR clipToWorld, ClipPrimitiveCR clip) const
    {
    if (nullptr != clip.GetTransformFromClip())
        clipToWorld = *clip.GetTransformFromClip();
    else
        clipToWorld.InitIdentity();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Test whether a single world coordinate point is inside fence
* @bsimethod                                                    Brien.Bastings  07/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool FenceParams::PointInside(DPoint3dCR testPoint)
    {
    return PointInsideClip(testPoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool FenceParams::AcceptDEllipse3d(DEllipse3dCR arc)
    {
    if (m_overlapMode)
        return m_clip->IsAnyPointInside(arc, false);
    else if (0 == static_cast<int>(m_clipMode))
        return m_clip->IsCompletelyContained(arc, false);
    else
        return !m_clip->IsAnyPointInside(arc, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      7/92
+---------------+---------------+---------------+---------------+---------------+------*/
bool FenceParams::AcceptLineSegments(DPoint3dP points, size_t numPoints, bool closed)
    {
    if (m_overlapMode)
        return m_clip->IsAnyLineStringPointInside(points, numPoints, closed);
    else if (0 == static_cast<int>(m_clipMode))
        return m_clip->IsCompletelyContained (points, numPoints, closed);
    else
        return !m_clip->IsAnyLineStringPointInside(points, numPoints, closed);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      7/92
+---------------+---------------+---------------+---------------+---------------+------*/
bool FenceParams::AcceptCurve(MSBsplineCurveCR curve)
    {
    if (m_overlapMode)
        return m_clip->IsAnyPointInside(curve);
    else if (0 == static_cast<int>(m_clipMode))
        return m_clip->IsCompletelyContained(curve);
    else
        return !m_clip->IsAnyPointInside(curve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool FenceParams::AcceptByCurve()
    {
    for (ClipPrimitivePtr const& primitive: *m_clip)
        {
        // If any clip is curved accept by curve...
        if (NULL != primitive->GetGPA (true))
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   07/03
+---------------+---------------+---------------+---------------+---------------+------*/
FenceParams::FenceParams()
    {
    m_overlapMode       = false;
    m_onTolerance       = .25;    // The traditional UOR tolerance...
    m_viewport          = nullptr;
    m_clipMode          = FenceClipMode::None;
    m_locateInteriors   = LocateSurfacesPref::Never;
    m_hasOverlaps       = false;
    m_fenceRangeNPC.Init();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   07/03
+---------------+---------------+---------------+---------------+---------------+------*/
FenceParams::FenceParams(FenceParamsP fpP)
    {
    m_overlapMode       = fpP->m_overlapMode;
    m_onTolerance       = fpP->m_onTolerance;
    m_viewport          = fpP->m_viewport;
    m_clipMode          = fpP->m_clipMode;
    m_clip              = fpP->m_clip;
    m_fenceRangeNPC     = fpP->m_fenceRangeNPC;
    m_locateInteriors   = fpP->m_locateInteriors;
    m_hasOverlaps       = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   07/03
+---------------+---------------+---------------+---------------+---------------+------*/
FenceParams::~FenceParams() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      7/92
+---------------+---------------+---------------+---------------+---------------+------*/
void FenceParams::ClearCurrentClip()
    {
    m_clip = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool FenceParams::IsOutsideClip() const
    {
    return m_clip.IsValid() && 1 == m_clip->size() && m_clip->front()->IsMask();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool FenceParams::AcceptElement(GeometrySourceCR source)
    {
    FenceAcceptContext context(*this);

    return context.AcceptGeometrySource(source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FenceParams::GetContents(DgnElementIdSet& contents, FenceCheckStop* checkStop)
    {
    FenceAcceptContext context(*this, checkStop);

    return context.GetContents(contents);
    }
