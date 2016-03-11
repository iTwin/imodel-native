/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/FenceParams.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/VecMath.h>

#define     CIRCLE_ClipPoints           60
#define     fc_cameraPlaneRatio         300.0
#define     MINIMUM_CLIPSIZE            1.0E-3
#define     NPC_CAMERA_LIMIT            100.0

#if defined (NEEDSWORK_RENDER_GRAPHIC)
/*=================================================================================**//**
* Output to determine if element should be accepted for fence processing..
* @bsiclass                                                     Brien.Bastings  09/04
+===============+===============+===============+===============+===============+======*/
struct FenceAcceptOutput : SimplifyGraphic
{
    DEFINE_T_SUPER(SimplifyGraphic)
protected:

FenceParamsP    m_fp;
bool            m_abort;
bool            m_currentAccept;
bool            m_accept;
bool            m_firstAccept;

public:

FenceAcceptOutput()      { m_fp = NULL; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/04
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool    _DoClipping() const override {return m_inSymbolDraw;} // Only need clip for symbols...
virtual bool    _DoSymbolGeometry() const override {return true;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _SetDrawViewFlags (ViewFlags flags) override
    {
    T_Super::_SetDrawViewFlags(flags);

    m_viewFlags.styles = false; // NOTE: Ignore linestyles for fence accept...

    switch (m_fp->GetLocateInteriors())
        {
        case LocateSurfacesPref::Never:
            {
            m_viewFlags.SetRenderMode(RenderMode::Wireframe);
            m_viewFlags.fill = false;
            break;
            }

        case LocateSurfacesPref::Always:
            {
            if (RenderMode::Wireframe == m_viewFlags.GetRenderMode())
                m_viewFlags.SetRenderMode(RenderMode::SmoothShade);
            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/05
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _PushTransClip(TransformCP trans, ClipPlaneSetCP clip) override
    {
    T_Super::_PushTransClip(trans, clip);

    if (m_viewOutput)
        m_viewOutput->PushTransClip(trans, clip);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/05
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _PopTransClip() override
    {
    T_Super::_PopTransClip();

    if (m_viewOutput)
        m_viewOutput->PopTransClip();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/05
+---------------+---------------+---------------+---------------+---------------+------*/
void GetTestPointWorld(DPoint3dR fencePt, ClipPrimitiveCR clipPrimitive)
    {
    ClipPolygonCP   clipPolygon;

    fencePt.Zero();

    if (NULL == (clipPolygon = clipPrimitive.GetPolygon()) || clipPolygon->empty())
        return;

    fencePt = DPoint3d::From(clipPolygon->front().x, clipPolygon->front().y, 0.0);

    Transform   invTransform;

    invTransform.InverseOf(*(m_fp->GetTransform()));

    if (NULL != clipPrimitive.GetTransformFromClip())
        invTransform.InitProduct(invTransform, *(clipPrimitive.GetTransformFromClip()));

    invTransform.Multiply(fencePt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/05
+---------------+---------------+---------------+---------------+---------------+------*/
void GetTestDirectionWorld(DVec3dR fenceDir)
    {
    Transform       invTransform;
    ClipVectorPtr   clip = m_fp->GetClipVector();

    invTransform.InverseOf(*(m_fp->GetTransform()));

    if (clip.IsValid() && !clip->empty() && clip->front()->GetTransformFromClip())
        invTransform.InitProduct(invTransform, *(clip->front()->GetTransformFromClip()));

    invTransform.GetMatrixColumn(fenceDir, 2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void GetBoresiteLocations(bvector<DRay3d>& out)
    {
    // NOTE: Can't test a single loop point when there are masks...
    ClipVectorPtr   clip = m_fp->GetClipVector();

    if (!clip.IsValid())
        return;

    DMatrix4d       viewToLocal = m_context->GetViewToLocal();

    for (ClipPrimitivePtr const& clipPrimitive: *clip)
        {
        DPoint3d    fencePt, localPt;
        DRay3d      boresite;

        GetTestPointWorld(fencePt, *clipPrimitive);
        m_context->WorldToLocal(&localPt, &fencePt, 1);
        PickContext::InitBoresite(boresite, localPt, viewToLocal);

        out.push_back(boresite);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/04
+---------------+---------------+---------------+---------------+---------------+------*/
TransformCP GetCurrentFenceAcceptTransform(TransformP transformP)
    {
    TransformCP placementTransP = m_context->GetCurrLocalToWorldTransformCP ();

    if (placementTransP)
        *transformP = *placementTransP;
    else
        transformP->InitIdentity();

    return transformP;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
static int      DrawQvElemCheckStop(void* arg)
    {
    FenceAcceptOutput  *output = (FenceAcceptOutput *) arg;
    return (output->m_context->CheckStop() ? 1 : 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/05
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _DrawGraphic(Graphic* qvElem) override
    {
    if (!m_viewOutput || !m_context->GetViewport())
        return;

    if (m_fp->HasOverlaps())
        return; // Already detected overlap, can skip interior check...

    DVec3d      viewNormal, fenceNormal;

    m_context->GetViewport()->GetRotMatrix().GetRow(viewNormal, 2);
    GetTestDirectionWorld(fenceNormal);

    bool        alignedToView = TO_BOOL (fenceNormal.IsParallelTo(viewNormal));

    // Test 1 point in each loop in case clip is disjoint clip mask...
    for (ClipPrimitivePtr const& primitive: *m_fp->GetClipVector())
        {
        DPoint3d    fencePt;

        GetTestPointWorld(fencePt, *primitive);

        // qvi_locateElementCB doesn't allow bore direction to be specified...must rotate view...
        if (!alignedToView)
            {
            RotMatrix   rMatrix;
            Transform   transform;

            LegacyMath::RMatrix::FromVectorToVector(&rMatrix, &fenceNormal, &viewNormal);
            transform.InitFromMatrixAndFixedPoint(rMatrix, fencePt);

            m_context->PushTransform(transform);
            }

        DPoint3d    pickPt;
        DPoint4d    pickPtView;

        m_context->WorldToView(&pickPtView, &fencePt, 1);
        pickPtView.GetProjectedXYZ (pickPt);

        bool        gotHit = false;
        DPoint3d    hitPt;

        gotHit = m_viewOutput->LocateQvElem(qvElem, *((DPoint2dCP) &pickPt), 1.0, hitPt, NULL, DrawQvElemCheckStop, this);

        if (!alignedToView)
            m_context->PopTransformClip();

        if (!gotHit)
            continue;

        m_fp->GetTransform()->Multiply(hitPt);
        if (!m_fp->PointInOtherClips(hitPt, primitive.get()))
            continue;

        SetCurrentAccept(true);
        m_fp->SetHasOverlaps(true);

        CheckCurrentAccept();
        break;
        }
    }

}; // FenceAcceptOutput
#endif

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
bool                m_collectContents;  // true for GetContents, false for AcceptElement...
DgnViewportP        m_nonVisibleViewport;
bool                m_abort;
bool                m_currentAccept;
bool                m_accept;
bool                m_firstAccept;

public:

FenceAcceptContext(FenceParamsR fp) : m_fp(fp)
    {
    m_purpose            = DrawPurpose::FenceAccept;
    m_collectContents    = false;
    m_nonVisibleViewport = nullptr;
    m_ignoreViewRange    = true;

    OnNewGeometrySource();
    }

~FenceAcceptContext()
    {
    DELETE_AND_CLEAR (m_nonVisibleViewport);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual Render::GraphicPtr _CreateGraphic(Render::Graphic::CreateParams const& params) override
    {
    return new SimplifyGraphic(params, *this, *this);
    }

//virtual UnhandledPreference _GetUnhandledPreference(CurveVectorCR) const override {return UnhandledPreference::Curve;}
virtual UnhandledPreference _GetUnhandledPreference(ISolidPrimitiveCR) const override {return UnhandledPreference::Curve;}
virtual UnhandledPreference _GetUnhandledPreference(MSBsplineSurfaceCR) const override {return UnhandledPreference::Curve;}
virtual UnhandledPreference _GetUnhandledPreference(PolyfaceQueryCR) const override {return UnhandledPreference::Curve;} // BAD - NEEDSWORK...
virtual UnhandledPreference _GetUnhandledPreference(ISolidKernelEntityCR) const override {return UnhandledPreference::Curve;}
virtual UnhandledPreference _GetUnhandledPreference(TextStringCR) const override {return UnhandledPreference::Box;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/04
+---------------+---------------+---------------+---------------+---------------+------*/
void OnNewGeometrySource()
    {
    m_fp.ClearSplitParams();

    m_firstAccept   = true;
    m_abort         = false;
    m_currentAccept = false;
    m_accept        = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool GetCurrentAbort() {return m_abort;}
bool GetCurrentAccept() {return m_accept;}
void SetCurrentAccept(bool accept) {m_currentAccept = accept;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/04
+---------------+---------------+---------------+---------------+---------------+------*/
void CheckCurrentAccept()
    {
    if (m_abort) // accept status has already been determined...
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
        m_abort  = true;
        }

    if (!insideMode && m_accept)
        {
        // Fence can overlap a complex element w/o any component having an overlap...
        if (hasOverlap)
            m_fp.SetHasOverlaps(true);

        // Need to look for ALL overlaps in clip mode...
        if (FenceClipMode::None == m_fp.GetClipMode())
            m_abort = m_fp.HasOverlaps();
        }

    m_firstAccept = false;
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _PushFrustumClip() override
    {
    // Not necessary to push frustum clip - Fence clip is pushed below.
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _CheckStop() override
    {
    return GetCurrentAbort(); // Doesn't want abort flag set...not reset between elements...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawAreaPattern(ClipStencil& boundary) override
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    FenceParamsP    fp = m_graphic->GetFenceParamsP ();

    if (fp->HasOverlaps())
        return; // Already have overlap w/element don't need to draw the pattern...

    if (_CheckStop())
        return;

    if (!_WantAreaPatterns())
        return;

    fp->ClearSplitParams();

    if (FenceClipMode::None == fp->GetClipMode()) // Need to draw patterns for interior overlap check when clipping...
        {
        // Attempt to short circuit fence accept using boundary...only check symbol geometry for interior overlap...
        boundary.GetGeomSource().Stroke(*this);

        // Element never rejected by pattern...so if boundary is acceptable we can skip drawing the pattern...
        if (_CheckStop() || m_graphic->GetCurrentAccept())
            return;

        fp->ClearSplitParams();
        }

    // Keep looking for overlaps using pattern geometry...
    T_Super::_DrawAreaPattern(boundary);

    // NOTE: Really only want to report an overlap but non-optimized clip requires split params...
    if (!fp->HasOverlaps())
        return;

    fp->ClearSplitParams(); // These are from the pattern...meaningless for boundary...
    fp->SetHasOverlaps(true); // set overlap again...cleared by ClearSplitParams...
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
        m_currentAccept = m_fp.AcceptCurve(&curve);
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

        m_currentAccept = m_fp.AcceptCurve(&curve);
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

    m_currentAccept = m_fp.AcceptCurve(copy.get());
    CheckCurrentAccept();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _ProcessCurvePrimitive(ICurvePrimitiveCR primitive, bool closed, bool filled, SimplifyGraphic& graphic) override
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
virtual bool _ProcessCurveVector(CurveVectorCR curves, bool filled, SimplifyGraphic& graphic) override
    {
    // If already detected overlap (or not a region/filled in wireframe), can skip interior check... 
    if (m_fp.HasOverlaps() || !curves.IsAnyRegionType() || (RenderMode::Wireframe == GetViewFlags().GetRenderMode() && !filled))
        {
        graphic.ProcessAsCurvePrimitives(curves, filled);

        return true;
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

            SetCurrentAccept(true);
            m_fp.SetHasOverlaps(true);
            CheckCurrentAccept();
            break;
            }
        }
#endif

    graphic.ProcessAsCurvePrimitives(curves, filled);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _ProcessSolidPrimitive(ISolidPrimitiveCR solid, SimplifyGraphic& graphic) override
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

        SetCurrentAccept(true);
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
virtual bool _ProcessSurface(MSBsplineSurfaceCR surface, SimplifyGraphic& graphic) override
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

        SetCurrentAccept(true);
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
virtual bool _ProcessPolyface(PolyfaceQueryCR polyface, bool filled, SimplifyGraphic& graphic) override
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

            SetCurrentAccept(true);
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
virtual StatusInt _VisitGeometry(GeometrySourceCR source) override
    {
    if (!m_collectContents)
        return T_Super::_VisitGeometry(source);

    OnNewGeometrySource(); // Initialize accept status for top-level element...

    if (SUCCESS == T_Super::_VisitGeometry(source) && GetCurrentAccept())
        {
        if (nullptr != source.ToElement())
            m_contents.insert(source.ToElement()->GetElementId());
        }

    OnNewGeometrySource(); // Clear abort status and continue checking next element...

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _VisitDgnModel(DgnModelP inDgnModel) override
    {
    // Always ignore elements that are not from the context's target dgnDb...
    if (&inDgnModel->GetDgnDb() != &GetDgnDb())
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

    return (SUCCESS == _VisitGeometry(source) && GetCurrentAccept());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool AcceptCurveVector(CurveVectorCR curves)
    {
    if (SUCCESS != Attach(m_fp.GetViewport(), m_purpose))
        return false;

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    m_graphic->AddCurveVector(curves, false);
#endif

    return GetCurrentAccept();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GetContents(DgnElementIdSet& contents)
    {
    DRange3d npcRange = m_fp.GetFenceRangeNPC ();
    if (!npcRange.IsNull ())
        {
        SetSubRectNpc(npcRange);
        }

    if (SUCCESS != Attach(m_fp.GetViewport(), m_purpose) || !m_fp.GetClipVector().IsValid())
        return ERROR;

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    ClipVectorPtr transformedClip = ClipVector::CreateCopy(*m_fp.GetClipVector());

    Transform clipToWorld;

    if (transformedClip.IsValid() &&
        NULL != m_fp.GetTransform() &&
        clipToWorld.InverseOf(*m_fp.GetTransform()))
        transformedClip->TransformInPlace(clipToWorld);

    GetTransformClipStack().PushClip(*transformedClip);
#endif
    m_collectContents = true;
    VisitAllViewElements();

    if (m_contents.empty())
        return ERROR;

    contents = m_contents;

    return SUCCESS;
    }

}; // FenceAcceptContext

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/07
+---------------+---------------+---------------+---------------+---------------+------*/
double          FenceParams::GetFocalLength() const {return m_focalLength;}
ClipVectorPtr   FenceParams::GetClipVector() const {return m_clip;}
FenceClipMode   FenceParams::GetClipMode() const {return m_clipMode;}
DgnViewportP    FenceParams::GetViewport() const {BeAssert(m_viewport && "Fence viewport must not be NULL"); return m_viewport;}
DgnModelP       FenceParams::GetDgnModel() const {BeAssert(m_viewport && "Fence viewport must not be NULL"); return m_viewport ? m_viewport->GetViewController().GetTargetModel() : NULL;}
TransformP      FenceParams::GetTransform() {return &m_transform;}
void            FenceParams::SetOverlapMode(bool val) {m_overlapMode = val;}
void            FenceParams::SetClipMode(FenceClipMode val) {m_clipMode = val;}
void            FenceParams::SetClip(ClipVectorCR clip) {m_clip = ClipVector::CreateCopy(clip);}
void            FenceParams::SetTransform(TransformCR trans) {m_transform = trans;}
void            FenceParams::SetLocateInteriors(LocateSurfacesPref interiors) {m_locateInteriors = interiors;}
bool            FenceParams::HasOverlaps() const {return m_hasOverlaps;}
bool            FenceParams::AllowOverlaps() const {return m_overlapMode;}

// Ratio between m_onTolerance and the tolerance we add to Z-Plane distances.
static const double s_zPlaneToleranceRatio = 1.0E-4;

/*---------------------------------------------------------------------------------**//**
| @param view => selects view whose transformation is applied.
|               Invalid view sets up identity transformation.
| @bsimethod                                                    RayBentley      7/92
+---------------+---------------+---------------+---------------+---------------+------*/
void FenceParams::SetViewParams(DgnViewportP viewport)
    {
    // Save viewport for use with fence processing callbacks...
    m_viewport = viewport;
    m_onTolerance = 1.5E-3; // Reduced from .25 for XM to 1.0E-4 (now that we use this uniformly I think smaller value makes sense).   RBB/RBB 03/06.
                            // Increased to 1.5E-3 to match/exceed CLIP_PLANE_BIAS.  - RBB 03/2011.  (TR# 293934).
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

    DMap4dCP worldToNPC = m_viewport->GetWorldToNpcMap ();
    bvector<DPoint2d> backPlaneUV;
    DRange3d uvRange = DRange3d::NullRange();

    for (int i = 0; i < nPoints; i++)
        {
        DPoint3d uvw;
        worldToNPC->M0.MultiplyAndRenormalize (&uvw, &points[i], 1);
        backPlaneUV.push_back (DPoint2d::From (uvw.x, uvw.y));
        uvRange.Extend (uvw.x, uvw.y, 0.0);
        }

    if (2 == nPoints) // Degenerate fence shape is used for crossing line selection...
        backPlaneUV.push_back(backPlaneUV.front());

    // Clipping is defined in npc space --- back plane is always z=0, front is z=1.0
    ClipMask    clipMask = ClipMask::XAndY;
    double      *pZLow = NULL, *pZHigh = NULL, zLow = -1.0e20, zHigh = 1.0e20;

    if (!m_viewport->GetViewFlags().noFrontClip)
        {
        zHigh = 1.0;
        clipMask = clipMask | ClipMask::ZHigh;
        pZHigh = &zHigh;
        }

    if (!m_viewport->GetViewFlags().noBackClip)
        {
        zLow = 0.0;
        clipMask = clipMask | ClipMask::ZLow;
        pZLow = &zLow;
        }

    if (zHigh > NPC_CAMERA_LIMIT)
        {
        zHigh = NPC_CAMERA_LIMIT;
        pZHigh = &zHigh;
        }

    m_clip = ClipVector::CreateFromPrimitive(ClipPrimitive::CreateFromShape(&backPlaneUV.front(), backPlaneUV.size(), outside, pZLow, pZHigh, nullptr).get());
    m_clip->MultiplyPlanesTimesMatrix (worldToNPC->M0);

    m_fenceRangeNPC.Init ();
    if (!outside)
        {
        m_fenceRange = uvRange;
        m_fenceRangeNPC.low.z = zLow;
        m_fenceRangeNPC.high.z = zHigh;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      7/92
+---------------+---------------+---------------+---------------+---------------+------*/
void FenceParams::StoreIntersection(double param)
    {
    m_splitParams.push_back(param);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool FenceParams::StoreIntersectionIfInsideClip(double param, DPoint3dP pointP, ClipPrimitiveCP intersectPrimitive)
    {
   // If overlap testing multiple clips make sure this point is ok for all of them...
   if (NULL != pointP && AllowOverlaps() && !PointInOtherClips(*pointP, intersectPrimitive))
       return false;

    StoreIntersection(param);

    return true;
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
    return  m_clip.IsValid() ? m_clip->PointInside(testPoint, m_onTolerance) : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt FenceParams::GetClipToWorldTransform(TransformR clipToWorld, ClipPrimitiveCR clip) const
    {
    Transform worldToClip = m_transform;

    if (!clipToWorld.InverseOf(worldToClip))
        return ERROR;

    // clip transform (if any) comes first: clipToWorld = fenceToWorld * clipToFence
    if (NULL != clip.GetTransformFromClip())
        clipToWorld.InitProduct(clipToWorld, *clip.GetTransformFromClip());

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
* Compute the (0, 1,or 2) intersections of a plane and an arc.
* @bsimethod                                                    RayBentley      7/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void compute3dArcIntersections(int *n, DPoint3d *isPnt, double *isAngle, DEllipse3dCR ellipse, DPoint3d *pt, DPoint3d *norm)
    {
    double      major, minor;
    DVec3d      xVec, yVec;
    RotMatrix   rotMatrix;

    major = xVec.Normalize(*((DVec3d *) &ellipse.vector0));
    minor = yVec.Normalize(*((DVec3d *) &ellipse.vector90));
    rotMatrix.InitFrom2Vectors(xVec, yVec);

    double      a, b, c, discrim, xtmp, ytmp;
    double      slope, intercept;

    /* at this point we have the plane specified in the view coordinate system
        (by a point and a normal) In order to return them to the view
        coordinate system we unrotate by the transformation */

    *n = 0;

    rotMatrix.MultiplyTranspose(*norm);
    rotMatrix.MultiplyTranspose(*pt);

    /* calculate the line intersection of the fence plane with the ellipse plane */
    intercept = pt->x * norm->x + pt->y * norm->y + pt->z * norm->z;

    if (fabs(norm->x) > fabs(norm->y))
        {
        slope =  - norm->y/norm->x;
        intercept /= norm->x;

        a = major/minor;
        a = slope*slope + a*a;
        b = 2.0 * slope * intercept;
        c = intercept*intercept - major*major;

        discrim = b*b - 4*a*c;

        if (discrim >= 0.0)
            {
            discrim = sqrt(discrim);
            isPnt->y = (-b + discrim) / (2.0 * a);
            isPnt->x = slope * isPnt->y + intercept;
            isPnt->z = 0.0;
            ytmp = isPnt->y*major;
            xtmp = isPnt->x*minor;
            *isAngle = Angle::Atan2(ytmp, xtmp);
            rotMatrix.Multiply(*isPnt);

            if (ellipse.IsAngleInSweep(*isAngle))
                {
                (*n)++;
                isPnt++;
                isAngle++;
                }

            if (discrim > 0.0)
                {
                isPnt->y = (-b - discrim) / (2.0 * a);
                isPnt->x = slope * isPnt->y + intercept;
                isPnt->z = 0.0;
                ytmp = isPnt->y*major;
                xtmp = isPnt->x*minor;
                *isAngle = Angle::Atan2(ytmp, xtmp);
                rotMatrix.Multiply(*isPnt);

                if (ellipse.IsAngleInSweep(*isAngle))
                    {
                    (*n)++;
                    isPnt++;
                    isAngle++;
                    }
                }
            }
        }
    else if (!LegacyMath::DEqual(norm->y, 0.0))
        {
        slope = - norm->x/norm->y;
        intercept /= norm->y;
        a = minor/major;
        a = slope*slope + a*a;
        b = 2.0 * slope * intercept;
        c = intercept*intercept - minor*minor;

        discrim = b*b - 4*a*c;

        if (discrim >= 0.0)
            {
            discrim = sqrt(discrim);
            isPnt->x = (-b + discrim) / (2.0 * a);
            isPnt->y = slope * isPnt->x + intercept;
            isPnt->z = 0.0;
            ytmp = isPnt->y*major;
            xtmp = isPnt->x*minor;
            *isAngle = Angle::Atan2(ytmp, xtmp);
            rotMatrix.Multiply(*isPnt);

            if (ellipse.IsAngleInSweep(*isAngle))
                {
                (*n)++;
                isPnt++;
                isAngle++;
                }

            if (discrim > 0.0)
                {
                isPnt->x = (-b - discrim) / (2.0 * a);
                isPnt->y = slope * isPnt->x + intercept;
                isPnt->z = 0.0;
                ytmp = isPnt->y*major;
                xtmp = isPnt->x*minor;
                *isAngle = Angle::Atan2(ytmp, xtmp);
                rotMatrix.Multiply(*isPnt);

                if (ellipse.IsAngleInSweep(*isAngle))
                    {
                    (*n)++;
                    isPnt++;
                    isAngle++;
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool FenceParams::ClipPlaneArcIntersect(ClipPrimitiveCR primitive, double z, DEllipse3dCR  ellipse)
    {
    int         n;
    bool        intersectFound = false;
    double      isAngle[2];
    DPoint3d    pp, planeNormal, isPnt[2];

    pp.x = pp.y = 0.0;
    pp.z = z - ellipse.center.z;

    planeNormal.x = planeNormal.y = 0.0;
    planeNormal.z = 1.0;

    compute3dArcIntersections(&n, isPnt, isAngle, ellipse, &pp, &planeNormal);

    for (int i=0; i<n; i++)
        {
        isPnt[i].Add(ellipse.center);
        primitive.TransformFromClip(isPnt[i]);

        if (primitive.PointInside(isPnt[i], m_onTolerance))
            intersectFound |= StoreIntersectionIfInsideClip(ellipse.AngleToFraction(isAngle[i]), &isPnt[i], &primitive);
        }

    return intersectFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   07/03
+---------------+---------------+---------------+---------------+---------------+------*/
static inline bool nearEqual(double val1, double val2) {return (fabs(val1 - val2) < mgds_fc_epsilon);}
static void exchange_double(double* double1, double* double2) {double temp = *double1; *double1 = *double2; *double2 = temp;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      7/92
+---------------+---------------+---------------+---------------+---------------+------*/
static bool pointIsInBlock(DRange3dCR range, bool testZLow, bool testZHigh, double onTolerance,  DPoint3dCR testPoint)
    {
    return (testPoint.x >= (range.low.x - onTolerance)) &&
           (testPoint.x <= (range.high.x + onTolerance)) &&
           (testPoint.y >= (range.low.y - onTolerance)) &&
           (testPoint.y <= (range.high.y + onTolerance)) &&
           (!testZLow  || (testPoint.z >= (range.low.z  - onTolerance))) &&
           (!testZHigh || (testPoint.z <= (range.high.z + onTolerance)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool FenceParams::ArcIntersect(DPoint2dCP lineSegP, DEllipse3dCR ellipse, ClipPrimitiveCR primitive)
    {
    int         n;
    bool        intersectFound = false;
    double      isAngle[2];
    DPoint3d    planePoint, planeNormal, isPnt[2];
    DRange3d    range;

    if (nearEqual(lineSegP[0].x, lineSegP[1].x) && nearEqual(lineSegP[0].y, lineSegP[1].y))
        return false;

    range.low.x = lineSegP[0].x - ellipse.center.x;
    range.low.y = lineSegP[0].y - ellipse.center.y;
    range.low.z = primitive.GetZLow() - ellipse.center.z;

    range.high.x = lineSegP[1].x - ellipse.center.x;
    range.high.y = lineSegP[1].y - ellipse.center.y;
    range.high.z =  primitive.GetZHigh() - ellipse.center.z;

    planePoint.x = range.low.x;
    planePoint.y = range.low.y;
    planePoint.z = ellipse.center.z;

    planeNormal.x = (range.high.y - range.low.y);
    planeNormal.y = (range.low.x - range.high.x);
    planeNormal.z = 0.0;
    planeNormal.Normalize(planeNormal);

    if (range.low.x > range.high.x)
        exchange_double(&range.low.x, &range.high.x);

    if (range.low.y > range.high.y)
        exchange_double(&range.low.y, &range.high.y);

    compute3dArcIntersections(&n, isPnt, isAngle, ellipse, &planePoint, &planeNormal);

    for (int i=0; i<n; i++)
        {
        if (pointIsInBlock(range, primitive.ClipZLow(), primitive.ClipZHigh(), m_onTolerance, isPnt[i]))
            {
            double  fraction = ellipse.AngleToFraction(isAngle[i]);

            if (fraction > 1.0)
                fraction = 1.0;          // TR# 291396. The ellipse.IsAngleInSweep() function will return true for angles slightly outside of span so apply limits here.
            else if (fraction < 0.0)     // This should probably be applied upstream (so overlap is consistent) but this change is right before RC for SS2.
                fraction = 0.0;

            DPoint3d        worldPoint = DPoint3d::FromSumOf(isPnt[i], ellipse.center);

            primitive.TransformFromClip(worldPoint);
            intersectFound |= StoreIntersectionIfInsideClip(fraction, &worldPoint, &primitive);
            }
        }

    return intersectFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool FenceParams::ArcFenceIntersect(ClipPrimitiveCR clipPrimitive, DEllipse3dCR ellipse)
    {
    if (LegacyMath::DEqual(ellipse.vector0.Magnitude(), 0.0) ||
        LegacyMath::DEqual(ellipse.vector90.Magnitude(), 0.0))
        return false;

    DEllipse3d  rotatedEllipse;

    if (NULL != clipPrimitive.GetTransformToClip())
        clipPrimitive.GetTransformToClip()->Multiply(rotatedEllipse, ellipse);
    else
        rotatedEllipse = ellipse;

    bool    intersectFound = false;

    if (clipPrimitive.ClipZLow())
        intersectFound |= ClipPlaneArcIntersect(clipPrimitive, clipPrimitive.GetZLow()  - s_zPlaneToleranceRatio * m_onTolerance, rotatedEllipse);

    if (clipPrimitive.ClipZHigh())
        intersectFound |= ClipPlaneArcIntersect(clipPrimitive, clipPrimitive.GetZHigh() + s_zPlaneToleranceRatio * m_onTolerance, rotatedEllipse);

    T_ClipPolygon const*    polygon;

    if (NULL != (polygon = clipPrimitive.GetPolygon()))
        for (DPoint2dCP clipPointP = &polygon->front(), clipEndP = clipPointP + polygon->size() - 1; clipPointP < clipEndP; clipPointP++)
            intersectFound |= ArcIntersect(clipPointP, rotatedEllipse, clipPrimitive);

    return intersectFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      7/92
+---------------+---------------+---------------+---------------+---------------+------*/
static bool linePlaneIntersect
(
DPoint3d        *intPntP,
double          *intParamP,
DPoint2dCP       clipPoints,
DPoint3dCP      linePoints,  // array of two points.
ClipPrimitiveCR clipPrimitive,
double          onTolerance
)
    {
    double      m1, m2;
    DPoint2d    deltaA;
    DPoint3d    deltaB;
    DRange3d    aRange, bRange;

    aRange.InitFrom(clipPoints, 2, 0.0);
    bRange.InitFrom(linePoints, 2);

    // Allow intersections "on the fence". TR# 175566
    aRange.low.x  -= onTolerance;
    aRange.low.y  -= onTolerance;
    aRange.high.x += onTolerance;
    aRange.high.y += onTolerance;

    static double   s_numericalTolerance = 1.0E-8;

    // For numerical error. TR# 183877
    bRange.low.x  -= s_numericalTolerance;
    bRange.low.y  -= s_numericalTolerance;
    bRange.high.x += s_numericalTolerance;
    bRange.high.y += s_numericalTolerance;

    if (aRange.high.x < bRange.low.x ||
        bRange.high.x < aRange.low.x ||
        aRange.high.y < bRange.low.y ||
        bRange.high.y < aRange.low.y ||
        (clipPrimitive.ClipZLow()  && bRange.high.z < clipPrimitive.GetZLow()) ||
        (clipPrimitive.ClipZHigh() && bRange.low.z  > clipPrimitive.GetZHigh()))
        return false;

    deltaA.DifferenceOf(clipPoints[1], clipPoints[0]);
    deltaB.DifferenceOf(linePoints[1], linePoints[0]);

    if (LegacyMath::DEqual(deltaA.x, 0.0))
        {
        if (LegacyMath::DEqual(deltaB.x, 0.0))
            return false;

        intPntP->x = clipPoints[0].x;
        intPntP->y = ((deltaB.y/deltaB.x) * (clipPoints[1].x-linePoints[1].x)) + linePoints[1].y;

        if (intPntP->y < aRange.low.y  || intPntP->y < bRange.low.y ||
            intPntP->y > aRange.high.y || intPntP->y > bRange.high.y)
            return false;
        }
    else if (LegacyMath::DEqual(deltaB.x, 0.0))
        {
        intPntP->x = linePoints[0].x;
        intPntP->y = ((deltaA.y/deltaA.x) * (linePoints[1].x-clipPoints[1].x)) + clipPoints[1].y;

        if (intPntP->y < aRange.low.y  || intPntP->y < bRange.low.y ||
            intPntP->y > aRange.high.y || intPntP->y > bRange.high.y)
            return false;
        }
    else
        {
        m1 =  deltaA.y / deltaA.x;
        m2 =  deltaB.y / deltaB.x;

        if (LegacyMath::DEqual(m1, m2))
            return false;

        intPntP->x = ((m1*clipPoints[0].x-m2*linePoints[0].x)+(linePoints[0].y-clipPoints[0].y)) /  (m1-m2);
        intPntP->y = m1*(intPntP->x-clipPoints[0].x)+clipPoints[0].y;

        if ((intPntP->x < aRange.low.x  || intPntP->x < bRange.low.x) ||
            (intPntP->x > aRange.high.x || intPntP->x > bRange.high.x))
            return false;
        }

    if (! LegacyMath::DEqual(deltaB.x, 0.0))
        *intParamP = (intPntP->x - linePoints[0].x) / deltaB.x;
    else if (! LegacyMath::DEqual(deltaB.y, 0.0))
        *intParamP = (intPntP->y - linePoints[0].y) / deltaB.y;
    else
        return false;

    intPntP->z = *intParamP * deltaB.z + linePoints[0].z;

    return (! clipPrimitive.ClipZLow()  || intPntP->z >= clipPrimitive.GetZLow()) &&
           (! clipPrimitive.ClipZHigh() || intPntP->z <= clipPrimitive.GetZHigh());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      7/92
+---------------+---------------+---------------+---------------+---------------+------*/
static bool clipPlaneLineIntersect
(
DPoint3dR       intClipPnt,
DPoint3dR       intWorldPnt,
double*         intParamP,
ClipPrimitiveCR clipPrimitive,
double          z,
double          onTolerance,
DPoint3dCP      lineP
)
    {
    DPoint3d    delta;

    delta.DifferenceOf(lineP[1], *lineP);

    *intParamP = 0;
    if (LegacyMath::DEqual(delta.z, 0.0))
        return false;                           // Changed to return false (and not test if line is on plane) - TR# 191272

    *intParamP = (z - lineP->z)/(delta.z);
    if (*intParamP < 0.0 || *intParamP > 1.0)
        return false;

    intClipPnt.x = lineP->x + *intParamP * delta.x;
    intClipPnt.y = lineP->y + *intParamP * delta.y;
    intClipPnt.z = z;

    intWorldPnt = intClipPnt;
    clipPrimitive.TransformFromClip(intWorldPnt);
    return clipPrimitive.PointInside(intWorldPnt, onTolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      7/92
+---------------+---------------+---------------+---------------+---------------+------*/
bool FenceParams::LinearFenceIntersect(ClipPrimitiveCR clipPrimitive, DPoint3dCP points, size_t numPoints, bool closed)
    {
    bool            intersectFound = false;
    double          intParam, paramDelta, paramBase = 0.0;
    ClipPolygonCP   polygon = clipPrimitive.GetPolygon();

    DPoint3d*   pRotatedPoints;

    if (NULL != clipPrimitive.GetTransformToClip())
        {
        clipPrimitive.GetTransformToClip()->Multiply(pRotatedPoints = (DPoint3d*) _alloca(numPoints * sizeof(DPoint3d)),  points,  (int) numPoints);
        points = pRotatedPoints;
        }

    paramDelta = 1.0 / (numPoints - 1);

    for (DPoint3dCP pntP = points, endP = points + numPoints - 1; pntP<endP; pntP++, paramBase += paramDelta)
        {
        if (pntP->x != DISCONNECT && (pntP+1)->x != DISCONNECT)
            {
            DPoint3d        intClipPnt, intWorldPnt;

            if (clipPrimitive.ClipZLow() &&
                clipPlaneLineIntersect(intClipPnt, intWorldPnt, &intParam, clipPrimitive, clipPrimitive.GetZLow() - s_zPlaneToleranceRatio * m_onTolerance, m_onTolerance, pntP) &&
                (closed || (! LegacyMath::RpntEqual(&intClipPnt, points) && ! LegacyMath::RpntEqual(&intClipPnt, endP))))
                {
                intersectFound |= StoreIntersectionIfInsideClip(paramBase + intParam * paramDelta, &intWorldPnt, &clipPrimitive);
                }

            if (clipPrimitive.ClipZHigh() &&
                clipPlaneLineIntersect(intClipPnt, intWorldPnt, &intParam, clipPrimitive, clipPrimitive.GetZHigh() + s_zPlaneToleranceRatio * m_onTolerance, m_onTolerance, pntP) &&
                (closed || (! LegacyMath::RpntEqual(&intClipPnt, points) && ! LegacyMath::RpntEqual(&intClipPnt, endP))))
                {
                intersectFound |= StoreIntersectionIfInsideClip(paramBase + intParam * paramDelta, &intWorldPnt, &clipPrimitive);
                }

            if (NULL != polygon)
                {
                for (DPoint2dCP clipPointP = &polygon->front(), clipEndP = clipPointP + polygon->size() - 1; clipPointP < clipEndP; clipPointP++)
                    {
                    if (linePlaneIntersect(&intClipPnt, &intParam, clipPointP, pntP, clipPrimitive, m_onTolerance) &&
                        (closed || (! LegacyMath::RpntEqual(&intClipPnt, points) && ! LegacyMath::RpntEqual(&intClipPnt, endP))))
                        {
                        intWorldPnt = intClipPnt;
                        clipPrimitive.TransformFromClip(intWorldPnt);
                        intersectFound |= StoreIntersectionIfInsideClip(paramBase + intParam * paramDelta, &intWorldPnt, &clipPrimitive);
                        }
                    }
                }
            }
        }

    return intersectFound;
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
bool FenceParams::CurveClipPlaneIntersect
(
ClipPrimitiveCR     clipPrimitive,
MSBsplineCurve*     curveP,
double              clipDistance
)
    {
    DPoint3d    point;
    bool        intersectFound = false;
    bvector<double> params;

    // ??? Plane at z=clipDistance ???
    DPlane3d plane = DPlane3d::FromOriginAndNormal(0.0, 0.0, clipDistance, 0.0, 0.0, 1.0);
    curveP->AddPlaneIntersections(NULL, &params, plane);

    for (double u : params)
        {
        curveP->FractionToPoint(point, u);

        clipPrimitive.TransformFromClip(point);
        if (clipPrimitive.PointInside(point, m_onTolerance))
            intersectFound |= StoreIntersectionIfInsideClip(u, &point, &clipPrimitive);
        }
    return intersectFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      7/92
+---------------+---------------+---------------+---------------+---------------+------*/
bool FenceParams::CurveFenceIntersect(ClipPrimitiveCR clipPrimitive, MSBsplineCurve  *curveP)
    {
    bool            intersectFound = false;
    MSBsplineCurve  rotatedCurve;

    if (NULL != clipPrimitive.GetTransformToClip())
        {
        if (SUCCESS == rotatedCurve.CopyTransformed(*curveP, *clipPrimitive.GetTransformToClip()))
            curveP = &rotatedCurve;
        else
            return false;
        }

    if (clipPrimitive.ClipZLow())
        intersectFound |= CurveClipPlaneIntersect(clipPrimitive, curveP, clipPrimitive.GetZLow()  - s_zPlaneToleranceRatio * m_onTolerance);

    if (clipPrimitive.ClipZHigh())
        intersectFound |= CurveClipPlaneIntersect(clipPrimitive, curveP, clipPrimitive.GetZHigh() + s_zPlaneToleranceRatio * m_onTolerance);

    bvector<DPoint3d> point0, point1;
    bvector<double>   param0, param1;

    if (NULL != clipPrimitive.GetBCurve())
        {
        curveP->AddCurveIntersectionsXY (&point0, &param0, &point1, &param1, *clipPrimitive.GetBCurve(), NULL);
        }
    else if (NULL != clipPrimitive.GetMaskOrClipPlanes())
        {
        for (ConvexClipPlaneSetCR convexPlaneSet: *clipPrimitive.GetMaskOrClipPlanes())
            {
            for (ClipPlaneCR plane: convexPlaneSet)
                {
                curveP->AddPlaneIntersections(&point0, &param0, plane.GetDPlane3d());
                }
            }
        }
    else
        {
        BeAssert(false);
        return false;
        }


    for (size_t i = 0; i < point0.size(); i++)
        {
        double      z = point0[i].z;

        if ((curveP->params.closed || (param0[i] > 0.0 && param0[i] < 1.0)) &&
            (! clipPrimitive.ClipZLow() || z >= clipPrimitive.GetZLow()) && (! clipPrimitive.ClipZHigh() || z <= clipPrimitive.GetZHigh()))
            {
            clipPrimitive.TransformFromClip(point0[i]);
            intersectFound |= StoreIntersectionIfInsideClip(param0[i], &point0[i], &clipPrimitive);
            }
        }

    if (curveP == &rotatedCurve)
        rotatedCurve.ReleaseMem();

    return intersectFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      7/92
+---------------+---------------+---------------+---------------+---------------+------*/
bool FenceParams::AcceptTransformedCurve
(
MSBsplineCurve* pCurve
)
    {
    if (m_overlapMode)
        return m_clip->IsAnyPointInside(*pCurve);
    else if (0 == static_cast<int>(m_clipMode))
        return m_clip->IsCompletelyContained(*pCurve);
    else
        return !m_clip->IsAnyPointInside(*pCurve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      7/92
+---------------+---------------+---------------+---------------+---------------+------*/
bool FenceParams::AcceptCurve(MSBsplineCurve  *curveP)
    {
    curveP->TransformCurve(m_transform);
    return AcceptTransformedCurve(curveP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      7/92
*
*   Accept segment of curve that is already transformed (but camera not applied)
*   and completely in front of the eye plane.
*
+---------------+---------------+---------------+---------------+---------------+------*/
bool FenceParams::AcceptCameraCurveSegment
(
MSBsplineCurve  *curveP
)
    {
    return AcceptTransformedCurve(curveP);
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
* @bsimethod                                                    Keith.Bentley   07/04
+---------------+---------------+---------------+---------------+---------------+------*/
void FenceParams::PushClip(ViewContextP context, DgnViewportP vp, bool displayCut)
    {
    if (NULL == vp)
        {
        vp = m_viewport;

        if (vp != context->GetViewport()) // fence is view-specific
            return;
        }

    if (!m_clip.IsValid())
        return;

    Transform           inverse;

    inverse.InverseOf(m_transform);

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    // If we have a camera on we need to generate the planes that instersect the
    // camera point...
    if (m_camera)
        {
        if (NULL != m_clip->front()->GetClipPlanes())
            {
            ClipPlaneSet    clipPlaneSet = *m_clip->front()->GetClipPlanes();

            for (ConvexClipPlaneSetR convexPlaneSet: clipPlaneSet)
                {
                for (ClipPlaneR plane: convexPlaneSet)
                    {
                    DVec3d      normal = plane.GetNormal();

                    if (fabs(normal.z) > .999)                 // But not front/back planes.
                        continue;

                    double          theta = atan2(plane.GetDistance(), m_focalLength);

                    if (0.0 != theta)
                        {
                        double  cosTheta = cos(theta);

                        plane = ClipPlane(DVec3d::From(normal.x * cosTheta, normal.y  * cosTheta, sin(theta)), 0.0);
                        }
                    }
                convexPlaneSet.TransformInPlace(inverse);
                }

            context->PushClip(*ClipVector::CreateFromPrimitive(ClipPrimitive::CreateFromClipPlanes(clipPlaneSet, !displayCut)));
            }
        }
    else
        {
        ClipVectorPtr       transformedClip = ClipVector::CreateCopy(*m_clip);
        transformedClip->TransformInPlace(inverse);

        transformedClip->SetInvisible(!displayCut);
        context->PushClip(*transformedClip);
        }
#endif
   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   07/03
+---------------+---------------+---------------+---------------+---------------+------*/
FenceParams::FenceParams(TransformCP trans)
    {
    m_focalLength = 0.0;

    if (trans)
        m_transform = *trans;
    else
        m_transform.InitIdentity();

    m_overlapMode       = false;
    m_onTolerance       = .25;    // The traditional UOR tolerance...

    m_viewport          = NULL;
    m_clipMode          = FenceClipMode::None;

    m_fenceRangeNPC.Init();
    m_splitParams.clear();

    m_hasOverlaps       = false;
    m_locateInteriors   = LocateSurfacesPref::Never;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   07/03
+---------------+---------------+---------------+---------------+---------------+------*/
FenceParams::FenceParams(FenceParamsP fpP)
    {
    m_focalLength       = fpP->m_focalLength;
    m_transform         = fpP->m_transform;
    m_overlapMode       = fpP->m_overlapMode;
    m_onTolerance       = fpP->m_onTolerance;
    m_viewport          = fpP->m_viewport;
    m_clipMode          = fpP->m_clipMode;
    m_clip              = fpP->m_clip;
    m_fenceRangeNPC     = fpP->m_fenceRangeNPC;
    m_locateInteriors   = fpP->m_locateInteriors;
    m_hasOverlaps       = false;
    m_splitParams.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      7/92
+---------------+---------------+---------------+---------------+---------------+------*/
void FenceParams::ClearCurrentClip()
    {
    m_clip = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   07/03
+---------------+---------------+---------------+---------------+---------------+------*/
void FenceParams::ClearSplitParams()
    {
    m_hasOverlaps = false;
    m_splitParams.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void FenceParams::SortSplitParams()
    {
    DoubleOps::Sort(m_splitParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool FenceParams::GetSplitParam(size_t i, double &value) const
    {
    value = 0.0;

    if (i >= m_splitParams.size())
        return false;

    value = m_splitParams[i];

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t FenceParams::GetNumSplitParams() const
    {
    return m_splitParams.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   07/03
+---------------+---------------+---------------+---------------+---------------+------*/
FenceParams::~FenceParams()
    {
    ClearSplitParams();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt FenceParams::PushClip(ViewContextR context, TransformCP localToRoot) const
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    DgnViewportP   viewport;

    if (NULL == (viewport = context.GetViewport()) || !m_clip.IsValid())
        return ERROR;

    ClipVectorPtr   clip = ClipVector::CreateCopy(*m_clip);

    Transform       rootToClip  = viewport->Is3dView()   ? m_transform : Transform::FromIdentity();
    Transform       localToClip = (NULL == localToRoot)  ? rootToClip  : Transform::FromProduct(rootToClip, *localToRoot);
    Transform       clipToLocal;

    clipToLocal.InverseOf(localToClip);

    clip->TransformInPlace(clipToLocal);
    context.PushClip(*clip);
#endif

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool FenceParams::IsOutsideClip() const
    {
    return m_clip.IsValid() &&
           1 == m_clip->size() &&
           m_clip->front()->IsMask();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void pointFromParameter(DPoint3dR point, ICurvePrimitive* curvePrimitive, double parameter, FenceParamsR fp)
    {
    curvePrimitive->FractionToPoint(parameter, point);

    fp.GetTransform()->Multiply(point);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void shiftEllipseSplineParameters(double *paramP, size_t nParams, MSBsplineCurveR curve, DEllipse3dCR ellipse)
    {
    double      angle, start, sweep, tolerance = 1.0E-8;
    DPoint3d    point;

    ellipse.GetSweep(start, sweep);
    for (size_t i=0; i<nParams; i++, paramP++)
        {
        if (*paramP != 0.0 && *paramP != 1.0)
            {
            bspcurv_evaluateCurvePoint(&point, NULL, &curve, fmod(*paramP, 1.0));

            angle = ellipse.PointToAngle(point);

            /* the start/end point of a closed arc
                is somewhat ambiguous, so handle it seperately */
            if (Angle::IsFullCircle(fabs(sweep)) && Angle::NearlyEqualAllowPeriodShift(angle, start))
                {
                *paramP = *paramP < .5 ? 0.0 : 1.0;
                }
            else
                {
                double      period = floor(*paramP);       /* Bug in VC5.0 if this is used directly in expression below */

                if (sweep < 0.0)
                    {
                    while (start > 0.0) start -= msGeomConst_2pi;
                    while (angle > start + tolerance) angle -= msGeomConst_2pi;
                    }
                else
                    {
                    while (start < 0.0) start += msGeomConst_2pi;
                    while (angle < start - tolerance) angle += msGeomConst_2pi;
                    }

                *paramP = period + (angle - start) / sweep;
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void calculateSegmentIntersections(FenceParamsR fp, ICurvePrimitive* curvePrimitive)
    {
    if (fp.AcceptByCurve())
        {
        MSBsplineCurve  curve;

        if (!curvePrimitive->GetMSBsplineCurve(curve))
            return;

        size_t  originalNumSplitParams = fp.GetNumSplitParams();

        fp.AcceptCurve(&curve);

        // Defect 88045 - shift from spline to ellipse angle parameters. This should all be deprecated and work on CurveVectors directly.
        if (fp.GetNumSplitParams() > originalNumSplitParams && NULL != curvePrimitive->GetArcCP ())
            shiftEllipseSplineParameters(fp.GetSplitParamsP() + originalNumSplitParams, fp.GetNumSplitParams() - originalNumSplitParams, curve, *curvePrimitive->GetArcCP());

        curve.ReleaseMem();
        return;
        }

    switch (curvePrimitive->GetCurvePrimitiveType())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            {
            DSegment3d segment = *curvePrimitive->GetLineCP ();

            fp.AcceptLineSegments(segment.point, 2, false);
            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            {
            bvector<DPoint3d> points = *curvePrimitive->GetLineStringCP ();

            fp.AcceptLineSegments(&points[0], (int) points.size(), false);
            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            {
            DEllipse3d ellipse = *curvePrimitive->GetArcCP ();

            fp.AcceptDEllipse3d(ellipse);
            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
            {
            MSBsplineCurve  curve;

            if (!curvePrimitive->GetMSBsplineCurve(curve))
                return;

            fp.AcceptCurve(&curve);
            curve.ReleaseMem();
            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void flushPartialCurve(bvector<CurveVectorPtr>* inside, bvector<CurveVectorPtr>* outside, CurveVectorR curveVector, bool isInside)
    {
    if (curveVector.empty())
        return;

    if (isInside)
        {
        if (inside)
            inside->push_back(&curveVector);
        }
    else
        {
        if (outside)
            outside->push_back(&curveVector);
        }

    curveVector.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/10
+---------------+---------------+---------------+---------------+---------------+------*/
void FenceParams::ParseAcceptedGeometry(bvector<CurveVectorPtr>* inside, bvector<CurveVectorPtr>* outside, GeometrySourceCR source)
    {
    // TODO: Look into pushing the fence clip onto the output and having SimplifyGraphic do the clipping...
#if defined (NEEDS_WORK_DGNITEM)
    CurveVectorPtr curveVector = ICurvePathQuery::ElementToCurveVector(eh);
#else
    CurveVectorPtr curveVector;
#endif

    if (curveVector.IsNull())
        return;

    if (FenceClipMode::None == GetClipMode() || !HasOverlaps() || 0 == GetNumSplitParams())
        {
        if (!inside)
            return;

        // Overlap w/o split params denotes an interior overlap (pattern/fill/etc.). Non-optimized clip doesn't handle
        // this case...so we just want to determine whether the boundary is in or out...
        FenceParams tmpFp(this);

        tmpFp.SetLocateInteriors(LocateSurfacesPref::Never);

        FenceAcceptContext context(tmpFp);

        if (!context.AcceptCurveVector(*curveVector))
            return;

        inside->push_back(curveVector);
        return;
        }

    // Only supports simple open/closed paths...
    if (!(curveVector->IsOpenPath() || curveVector->IsClosedPath()))
        return;

    bool           firstInside=false, lastInside=false, lastInsideValid = false;
    CurveVectorPtr partialCurve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    CurveVectorPtr firstPartial = NULL;

    for (ICurvePrimitivePtr curvePrimitive : *curveVector)
        {
        if (curvePrimitive.IsNull())
            continue;

        DPoint3d    segOrg, segEnd;
        double      u0, u1;

        // NOTE: For complex shape/chain we don't know which component a split param belongs to...
        FenceParams tmpFp(this);

        calculateSegmentIntersections(tmpFp, curvePrimitive.get());

        if (!tmpFp.GetSplitParam(0, u0))
            {
            partialCurve->push_back(curvePrimitive);
            continue;
            }

        tmpFp.StoreIntersection(0.0);
        tmpFp.StoreIntersection(1.0);
        tmpFp.SortSplitParams();

        tmpFp.GetSplitParam(0, u0);
        pointFromParameter(segOrg, curvePrimitive.get(), u0, tmpFp);

        for (size_t i = 1; tmpFp.GetSplitParam(i, u1); i++, u0 = u1)
            {
            pointFromParameter(segEnd, curvePrimitive.get(), u1, tmpFp);

            if (segOrg.IsEqual(segEnd, 1.0e-8) || (u1 - u0) <= MINIMUM_CLIPSIZE)
                continue;

            DPoint3d    testPoint;

            pointFromParameter(testPoint, curvePrimitive.get(), (u0 + u1)/2.0, tmpFp);

            bool        isInside = tmpFp.PointInsideClip(testPoint);

            if (lastInsideValid && isInside != lastInside)
                {
                // Keep first partial for a closed element, will append to last partial for wrap around...
                if (firstPartial.IsNull() && curveVector->IsClosedPath())
                    {
                    firstPartial = partialCurve->Clone();
                    firstInside  = lastInside;

                    partialCurve->clear();
                    }
                else
                    {
                    flushPartialCurve(inside, outside, *partialCurve, lastInside);
                    }
                }

            if ((isInside && inside) || outside)
                partialCurve->push_back(curvePrimitive.get()->CloneBetweenFractions(u0, u1, false));

            lastInside = isInside;
            lastInsideValid = true;
            segOrg = segEnd;
            }
        }

    if (firstPartial.IsValid())
        {
        // If first and last partials on same side of fence, join them...otherwise flush separately...
        if (firstInside == lastInside)
            {
            for (ICurvePrimitivePtr curvePrimitive : *firstPartial)
                {
                if (curvePrimitive.IsNull())
                    continue;

                partialCurve->push_back(curvePrimitive);
                }

            // Don't create a complex chain for original that was a single closed element (ex. want linestring from shape).
            if (1 == curveVector->size())
                partialCurve->ConsolidateAdjacentPrimitives();
            }
        else
            {
            flushPartialCurve(inside, outside, *firstPartial, firstInside);
            }
        }

    flushPartialCurve(inside, outside, *partialCurve, lastInside);
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
BentleyStatus FenceParams::GetContents(DgnElementIdSet& contents)
    {
    FenceAcceptContext context(*this);

    return context.GetContents(contents);
    }
