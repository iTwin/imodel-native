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
* @bsiclass                                                     Brien.Bastings  11/17
+===============+===============+===============+===============+===============+======*/
struct FencePolyfaceTester : PolyfaceQuery::IClipToPlaneSetOutput
{
bool m_unclipped;
bvector<PolyfaceHeaderPtr> m_output;
        
FencePolyfaceTester() : m_unclipped(false) {}
StatusInt _ProcessUnclippedPolyface(PolyfaceQueryCR) override {m_unclipped = true; return SUCCESS;}
StatusInt _ProcessClippedPolyface(PolyfaceHeaderR mesh) override {PolyfaceHeaderPtr meshPtr = &mesh; m_output.push_back(meshPtr); return SUCCESS;}
bool HasOverlap(PolyfaceQueryCR mesh, ClipVectorCR clip) {clip.ClipPolyface(mesh, *this, false); return !m_output.empty();}
bool IsInside() {return m_unclipped;}

}; // FencePolyfaceTester

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  11/17
+===============+===============+===============+===============+===============+======*/
struct FenceSimplifyGraphic : SimplifyGraphic
{
bool _WantPreBakedBody(IBRepEntityCR) override {return true;}

explicit FenceSimplifyGraphic(Render::GraphicBuilder::CreateParams const& params, IGeometryProcessorR processor, ViewContextR context) : SimplifyGraphic(params, processor, context) {}
};

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
Render::GraphicBuilderPtr _CreateGraphic(Render::GraphicBuilder::CreateParams const& params) override
    {
    return new FenceSimplifyGraphic(params, *this, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr _CreateBranch(Render::GraphicBranch& branch, DgnDbR db, TransformCR tf, ClipVectorCP clips) override
    {
    return new FenceSimplifyGraphic::Base(db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/04
+---------------+---------------+---------------+---------------+---------------+------*/
UnhandledPreference _GetUnhandledPreference(ISolidPrimitiveCR, SimplifyGraphic&) const override {return UnhandledPreference::Facet;}
UnhandledPreference _GetUnhandledPreference(MSBsplineSurfaceCR, SimplifyGraphic&) const override {return UnhandledPreference::Facet;}
UnhandledPreference _GetUnhandledPreference(IBRepEntityCR, SimplifyGraphic&) const override {return UnhandledPreference::Facet;} // NOTE: Won't be used for persistent BReps, we'll use backup mesh from GeometryStream...
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

    bool insideMode = m_fp.IsInsideMode();
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

        // I don't think we need to look for all overlaps in clip mode anymore...might have been because of split params for non-optimized clip???
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
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool _ProcessCurvePrimitive(ICurvePrimitiveCR primitive, bool closed, bool filled, SimplifyGraphic& graphic) override
    {
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString == primitive.GetCurvePrimitiveType())
        {
        bvector<DPoint3d> const* points = primitive.GetPointStringCP();

        ProcessPoints(&points->front(), (int) points->size(), graphic);

        return true;
        }

    return (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != primitive.GetCurvePrimitiveType());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool _ProcessCurveVector(CurveVectorCR curves, bool filled, SimplifyGraphic& graphic) override
    {
    Transform localToWorld = graphic.GetLocalToWorldTransform();
    Transform worldToLocal;

    worldToLocal.InverseOf(localToWorld);

    // NEEDSWORK: Earlin will add a method to handle all clips/masks...
    for (ClipPrimitivePtr const& primitive : *m_fp.GetClipVector())
        {
        if (CheckStop())
            break;

        ClipPlaneSetCP clipPlanes = primitive->GetClipPlanes();
//        ClipPlaneSetCP clipMask = primitive->GetMaskPlanes();

        if (nullptr == clipPlanes)
            continue;

        ClipPlaneSet localClipPlanes(*clipPlanes);

        localClipPlanes.TransformInPlace(worldToLocal);

        switch (ClipPlaneSet::ClassifyCurveVectorInSetDifference(curves, localClipPlanes, nullptr, !m_fp.IsInsideMode()))
            {
            case ClipPlaneContainment_StronglyInside:
                m_currentAccept = true;
                break;

            case ClipPlaneContainment_Ambiguous:
                if (m_fp.IsInsideMode())
                    break;
                m_currentAccept = true;
                m_fp.SetHasOverlaps(true);
                break;

            case ClipPlaneContainment_StronglyOutside:
                continue;
           }

        CheckCurrentAccept();
        }

    if (!CheckStop())
        graphic.ProcessAsCurvePrimitives(curves, filled); // Check for point strings...

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool _ProcessSolidPrimitive(ISolidPrimitiveCR solid, SimplifyGraphic& graphic) override
    {
    if (CheckStop())
        return true;

    // NEEDSWORK: Ask Earlin if we can get containment more efficiently without creating Polyface...
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool _ProcessSurface(MSBsplineSurfaceCR surface, SimplifyGraphic& graphic) override
    {
    if (CheckStop())
        return true;

    // NEEDSWORK: Ask Earlin if we can get containment more efficiently without creating Polyface...
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool _ProcessPolyface(PolyfaceQueryCR polyface, bool filled, SimplifyGraphic& graphic) override
    {
    if (CheckStop())
        return true;

    // NEEDSWORK: Ask Earlin if we can get containment more efficiently without creating clipped result...
    Transform localToWorld = graphic.GetLocalToWorldTransform();
    Transform worldToLocal;

    worldToLocal.InverseOf(localToWorld);

    ClipVectorPtr       localClip = m_fp.GetClipVector()->Clone(&worldToLocal);
    FencePolyfaceTester polyfaceTester;

    if (polyfaceTester.HasOverlap(polyface, *localClip))
        {
        if (!m_fp.IsInsideMode())
            {
            m_currentAccept = true;
            m_fp.SetHasOverlaps(true);
            }
        }
    else if (polyfaceTester.IsInside())
        {
        m_currentAccept = true;
        }

    CheckCurrentAccept();

    return true;
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
bool            FenceParams::HasOverlaps() const {return m_hasOverlaps;}
bool            FenceParams::AllowOverlaps() const {return m_overlapMode;}
bool            FenceParams::IsInsideMode() const {return !m_overlapMode && FenceClipMode::None == m_clipMode;}

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
