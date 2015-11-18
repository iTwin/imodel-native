/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ViewContext.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

static DRange3d const s_fullNpcRange =
    {
    {0.0, 0.0, 0.0},
    {1.0, 1.0, 1.0}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
ViewContext::ViewContext()
    {
    m_dgnDb = nullptr;
    m_viewport = nullptr;
    m_scanCriteria = nullptr;
    m_purpose = DrawPurpose::NotSpecified;
    m_arcTolerance = .01;
    m_minLOD = DEFAULT_MINUMUM_LOD;
    m_isAttached = false;
    m_is3dView = true;
    m_useNpcSubRange = false;
    m_filterLOD = FILTER_LOD_ShowRange;
    m_wantMaterials = false;
    m_startTangent = m_endTangent = nullptr;
    m_ignoreViewRange = false;
    m_hiliteState = DgnElement::Hilited::None;
    m_scanRangeValid = false;
    m_levelOfDetail = 1.0;
    m_worldToNpc.InitIdentity();
    m_worldToView.InitIdentity();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/05
+---------------+---------------+---------------+---------------+---------------+------*/
ViewContext::~ViewContext()
    {
    BeAssert(!m_isAttached);
    DELETE_AND_CLEAR(m_scanCriteria);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::ViewToNpc(DPoint3dP npcVec, DPoint3dCP screenVec, int nPts) const
    {
    ViewToWorld(npcVec, screenVec, nPts);
    m_worldToNpc.M0.MultiplyAndRenormalize(npcVec, npcVec, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::NpcToView(DPoint3dP viewVec, DPoint3dCP npcVec, int nPts) const
    {
    NpcToWorld(viewVec, npcVec, nPts);
    WorldToView(viewVec, viewVec, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::NpcToWorld(DPoint3dP worldPts, DPoint3dCP npcPts, int nPts) const
    {
    m_worldToNpc.M1.MultiplyAndRenormalize(worldPts, npcPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/05
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::InitDisplayPriorityRange()
    {
    m_displayPriorityRange[0] = (m_is3dView ? 0 : -MAX_HW_DISPLAYPRIORITY);
    m_displayPriorityRange[1] = (m_is3dView ? 0 : MAX_HW_DISPLAYPRIORITY);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::_InitContextForView()
    {
    BeAssert(0 == GetTransClipDepth());

    m_elemMatSymb.Init();
    m_ovrMatSymb.Clear();

    m_worldToNpc  = *m_viewport->GetWorldToNpcMap();
    m_worldToView = *m_viewport->GetWorldToViewMap();
    m_transformClipStack.Clear();
    m_scanRangeValid = false;

    _PushFrustumClip();

    SetDgnDb(m_viewport->GetViewController().GetDgnDb());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Frustum ViewContext::GetFrustum()
    {
    Frustum frustum;
    DPoint3dP frustPts = frustum.GetPtsP();

    if (m_useNpcSubRange)
        {
        m_npcSubRange.Get8Corners(frustPts);
        }
    else
        {
        if (nullptr == m_viewport)
            s_fullNpcRange.Get8Corners(frustPts);
        else
            frustum = m_viewport->GetFrustum(DgnCoordSystem::Npc, true);
        }

    m_worldToNpc.M1.MultiplyAndRenormalize(frustPts, frustPts, NPC_CORNER_COUNT);
    DgnViewport::FixFrustumOrder(frustum);
    return frustum;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_PushFrustumClip()
    {
    if (m_ignoreViewRange)
        return;

    int         nPlanes;
    ClipPlane   frustumPlanes[6];
    ViewFlags viewFlags = GetViewFlags();

    Frustum polyhedron = GetFrustum();

    if (0 != (nPlanes = ClipUtil::RangePlanesFromPolyhedra(frustumPlanes, polyhedron.GetPts(), !viewFlags.noFrontClip, !viewFlags.noBackClip, 1.0E-6)))
        m_transformClipStack.PushClipPlanes(frustumPlanes, nPlanes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/02
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_InitScanRangeAndPolyhedron()
    {
    // set up scanner search criteria
    _ScanRangeFromPolyhedron();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::PushClipPlanes(ClipPlaneSetCR clipPlanes)
    {
    _PushClip(*ClipVector::CreateFromPrimitive(ClipPrimitive::CreateFromClipPlanes(clipPlanes)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_PushClip(ClipVectorCR clip)
    {
    m_transformClipStack.PushClip(clip);

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    for (ClipPrimitivePtr const& primitive: clip)
        {
        GetCurrentGraphicR()._PushTransClip(nullptr, primitive->GetClipPlanes());
        m_transformClipStack.IncrementPushedToDrawGeom();
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_PushTransform(TransformCR trans)
    {
    m_transformClipStack.PushTransform(trans);
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    GetCurrentGraphicR()._PushTransClip(&trans , nullptr);
#endif
    m_transformClipStack.IncrementPushedToDrawGeom();
    InvalidateScanRange();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_PushViewIndependentOrigin(DPoint3dCP origin)
    {
    Transform   viTrans;
    GetViewIndependentTransform(&viTrans, origin);
    _PushTransform(viTrans);
    m_transformClipStack.SetViewIndependent();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_PopTransformClip()
    {
    if (m_transformClipStack.IsEmpty())
        {
        BeAssert(false);
        return;
        }

    m_transformClipStack.Pop(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_SetDgnDb(DgnDbR dgnDb)
    {
    m_dgnDb = &dgnDb;
    InitDisplayPriorityRange();
    _SetupScanCriteria();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  02/05
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_SetupScanCriteria()
    {
    if (nullptr == m_scanCriteria)
        return;

    DgnViewportP vp = GetViewport();
    if (nullptr != vp)
        m_scanCriteria->SetCategoryTest(vp->GetViewController().GetViewedCategories());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/05
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_AllocateScanCriteria()
    {
    if (nullptr == m_scanCriteria)
        m_scanCriteria = new ScanCriteria;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::_Attach(DgnViewportP viewport, DrawPurpose purpose)
    {
    if (nullptr == viewport)
        return  ERROR;

    if (IsAttached())
        {
        BeAssert(!IsAttached());
        return  ERROR;
        }

    m_isAttached = true;
    _AllocateScanCriteria();

    m_viewport = viewport;
    m_purpose = purpose;
    ClearAborted();

    m_minLOD = viewport->GetMinimumLOD();
    m_filterLOD = FILTER_LOD_ShowRange;

    m_is3dView = viewport->Is3dView();
    SetViewFlags(viewport->GetViewFlags());

    m_arcTolerance = 0.1;

    return _InitContextForView();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_Detach()
    {
    BeAssert(IsAttached());

    m_isAttached = false;

    m_transformClipStack.PopAll(*this);
    m_currentGeomSource = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::LocalToView(DPoint4dP viewPts, DPoint3dCP localPts, int nPts) const
    {
    GetLocalToView().Multiply(viewPts, localPts, nullptr, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::LocalToView(DPoint3dP viewPts, DPoint3dCP localPts, int nPts) const
    {
    DMatrix4dCR  localToView = GetLocalToView();

    if (m_viewport->IsCameraOn())
        localToView.MultiplyAndRenormalize(viewPts, localPts, nPts);
    else
        localToView.MultiplyAffine(viewPts, localPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::ViewToLocal(DPoint3dP localPts, DPoint4dCP viewPts, int nPts) const
    {
    GetViewToLocal().MultiplyAndNormalize(localPts, viewPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::ViewToLocal(DPoint3dP localPts, DPoint3dCP viewPts, int nPts) const
    {
    GetViewToLocal().MultiplyAndRenormalize(localPts, viewPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::WorldToView(DPoint4dP viewPts, DPoint3dCP worldPts, int nPts) const
    {
    m_worldToView.M0.Multiply(viewPts, worldPts, nullptr, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::WorldToView(DPoint3dP viewPts, DPoint3dCP worldPts, int nPts) const
    {
    m_worldToView.M0.MultiplyAndRenormalize(viewPts, worldPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::WorldToView(Point2dP viewPts, DPoint3dCP worldPts, int nPts) const
    {
    DPoint3d  tPt;
    DPoint4d  t4dPt;

    for (int i=0; i<nPts; i++)
        {
        WorldToView(&t4dPt, worldPts+i, 1);

        t4dPt.GetProjectedXYZ (tPt);

        (viewPts+i)->x = (long) tPt.x;
        (viewPts+i)->y = (long) tPt.y;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::ViewToWorld(DPoint3dP worldPts, DPoint4dCP viewPts, int nPts) const
    {
    m_worldToView.M1.MultiplyAndNormalize(worldPts, viewPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::ViewToWorld(DPoint3dP worldPts, DPoint3dCP viewPts, int nPts) const
    {
    m_worldToView.M1.MultiplyAndRenormalize(worldPts, viewPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::LocalToWorld(DPoint3dP worldPts, DPoint3dCP localPts, int nPts) const
    {
    Transform   localToWorld;

    if (SUCCESS == m_transformClipStack.GetTransform(localToWorld))
        localToWorld.Multiply(worldPts, localPts, nPts);
    else
        memcpy(worldPts, localPts, nPts * sizeof(DPoint3d));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::WorldToLocal(DPoint3dP localPts, DPoint3dCP worldPts, int nPts) const
    {
    Transform   worldToLocal;

    if (SUCCESS == m_transformClipStack.GetInverseTransform(worldToLocal))
        worldToLocal.Multiply(localPts, worldPts, nPts);
    else
        memcpy(localPts, worldPts, nPts * sizeof(DPoint3d));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::GetViewIndependentTransform(TransformP trans, DPoint3dCP originLocal)
    {
    RotMatrix   rMatrix;
    DgnViewportP vp = GetViewport();
    if (nullptr != vp)
        {
        // get two vectors from origin in VIEW x and y directions
        DPoint4d    screenPt[2];
        LocalToView(screenPt, originLocal, 1);

        DPoint3d viewSize;
        Frustum viewBox = vp->GetFrustum(DgnCoordSystem::View, true);
        viewSize.DifferenceOf(viewBox.GetCorner(NPC_111), viewBox.GetCorner(NPC_000));

        screenPt[1] = screenPt[0];
        screenPt[0].x += viewSize.x;
        screenPt[1].y += viewSize.y;

        // convert to local coordinates
        DPoint3d    localPt[2];
        ViewToLocal(localPt, screenPt, 2);

        // if we're in a 2d view, we remove any fuzz
        if (!m_is3dView)
            localPt[0].z = localPt[1].z = originLocal->z;

        DVec3d  u, v;
        u.NormalizedDifference(*localPt, *originLocal);
        v.NormalizedDifference(localPt[1], *originLocal);

        // convert to rmatrix
        rMatrix.InitFrom2Vectors(u, v);
        }
    else
        {
        rMatrix.InitIdentity();
        }

    // get transform about origin
    trans->InitFromMatrixAndFixedPoint(rMatrix, *originLocal);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
ILineStyleCP ViewContext::_GetCurrLineStyle(LineStyleSymbP* symb)
    {
    LineStyleSymbR  tSymb = (m_ovrMatSymb.GetFlags() & OvrMatSymb::FLAGS_Style) ? m_ovrMatSymb.GetMatSymbR().GetLineStyleSymbR() : m_elemMatSymb.GetLineStyleSymbR();

    if (symb)
        *symb = &tSymb;

    return nullptr == tSymb.GetTexture() ? tSymb.GetILineStyle() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* convert the view context polyhedron to scan parameters in the scanCriteria.
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::_ScanRangeFromPolyhedron()
    {
    Frustum polyhedron = GetFrustum();

    WorldToLocal(polyhedron.GetPtsP(), polyhedron.GetPts(), 8);

    // get enclosing bounding box around polyhedron (outside scan range).
    DRange3d scanRange = polyhedron.ToRange();

    if (!Is3dView())
        {
        scanRange.low.z = -1;
        scanRange.high.z = 1;
        }

    if (m_scanCriteria)
        {
        m_scanCriteria->SetRangeTest(&scanRange);

        // if we're doing a skew scan, get the skew parameters
        if (Is3dView())
            {
            DRange3d skewRange;

            // get bounding range of front plane of polyhedron
            skewRange.InitFrom(polyhedron.GetPts(), 4);

            // get unit bvector from front plane to back plane
            DVec3d      skewVec = DVec3d::FromStartEndNormalize(polyhedron.GetCorner(0), polyhedron.GetCorner(4));

            // check to see if it's worthwhile using skew scan (skew bvector not along one of the three major axes */
            int alongAxes = (fabs(skewVec.x) < 1e-8);
            alongAxes += (fabs(skewVec.y) < 1e-8);
            alongAxes += (fabs(skewVec.z) < 1e-8);

            if (alongAxes < 2)
                m_scanCriteria->SetSkewRangeTest(&scanRange, &skewRange, &skewVec);
            }
        }
    m_scanRangeValid = true;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Test an element against the current scan range using the range planes.
* @return true if the element is outside the range and should be ignored.
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::_FilterRangeIntersection(GeometrySourceCR source)
    {
    return ClipPlaneContainment_StronglyOutside == m_transformClipStack.ClassifyRange(source.CalculateRange3d(), nullptr != source.ToGeometrySource3d());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    03/02
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_OutputElement(GeometrySourceCR source)
    {
    m_currentGeomSource = &source;
    m_currGraphic = nullptr;

    if (!source.HasGeometry())
        return;

    Transform placementTrans = source.GetPlacementTransform();
    DPoint3d origin;
    placementTrans.GetTranslation(origin);

    DgnViewportCR vp = *GetViewport();
    double pixelSize = vp.GetPixelSizeAtPoint(&origin);

    m_currGraphic = _GetCachedGraphic(pixelSize);
    if (m_currGraphic.IsValid())
        return;

    m_currGraphic = _BeginGraphic(Graphic::CreateParams(&vp, placementTrans, pixelSize));
    vp.GetViewControllerR()._StrokeGeometry(*this, source);
    _SaveGraphic();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_AddViewOverrides(OvrMatSymbR ovrMatSymb)
    {
    // NOTE: ElemDisplayParams/ElemMatSymb ARE NOT setup at this point!
    if (!m_viewflags.weights)
        ovrMatSymb.SetWidth(1);

    if (!m_viewflags.transparency)
        {
        ovrMatSymb.SetLineTransparency(0);
        ovrMatSymb.SetFillTransparency(0);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_AddContextOverrides(OvrMatSymbR ovrMatSymb)
    {
    // Modify m_ovrMatSymb for view flags...
    _AddViewOverrides(ovrMatSymb); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_ModifyPreCook(ElemDisplayParamsR elParams)
    {
    elParams.Resolve(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_CookDisplayParams(ElemDisplayParamsR elParams, ElemMatSymbR elMatSymb)
    {
    _ModifyPreCook(elParams); // Allow context to modify elParams before cooking...

    // "cook" the display params into a MatSymb
    elMatSymb.Cook(elParams, *this, m_startTangent, m_endTangent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::CookDisplayParams()
    {
    _CookDisplayParams(m_currDisplayParams, m_elemMatSymb);

    //  NEEDSWORK_LINESTYLES
    //  If this is a 3d view and we have a line style then we want to convert the line style
    //  to a texture line style.  We don't do it prior to this because generating the geometry map
    //  may use the current symbology.  This seems like a horrible place to do this,
    //  so we need to come up with something better.
    LineStyleSymb & lsSym = m_elemMatSymb.GetLineStyleSymbR();
    if (nullptr==lsSym.GetTexture() && lsSym.GetILineStyle() != nullptr && Is3dView())
        {
        lsSym.ConvertLineStyleToTexture(*this, true);
        }

    // Activate the matsymb in the IDrawGeom
    GetCurrentGraphicR().ActivateMatSymb(&m_elemMatSymb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::ResetContextOverrides()
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    m_rasterDisplayParams.SetFlags(ViewContext::RasterDisplayParams::RASTER_PARAM_None); // NEEDSWORK_RASTER_DISPLAY - Not sure how this fits into new continuous update approach?!?
#endif

    // NOTE: Context overrides CAN NOT look at m_currDisplayParams or m_elemMatSymb as they are not valid.
    m_ovrMatSymb.Clear();
    _AddContextOverrides(m_ovrMatSymb);
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    GetCurrentGraphicR().ActivateOverrideMatSymb(&m_ovrMatSymb);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      02/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::IsUndisplayed(GeometrySourceCR source)
    {
    return (!_WantUndisplayed() && source.IsUndisplayed());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/05
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::DrawBox(DPoint3dP box, bool is3d)
    {
    GraphicR drawGeom = GetCurrentGraphicR();
    DPoint3d    tmpPts[9];

    if (is3d)
        {
        tmpPts[0] = box[0];
        tmpPts[1] = box[1];
        tmpPts[2] = box[2];
        tmpPts[3] = box[3];

        tmpPts[4] = box[5];
        tmpPts[5] = box[6];
        tmpPts[6] = box[7];
        tmpPts[7] = box[4];

        tmpPts[8] = box[0];

        // Draw a "saddle" shape to accumulate correct dirty region, simple lines can be clipped out when zoomed in...
        drawGeom.AddLineString(9, tmpPts, nullptr);

        // Draw missing connecting lines to complete box...
        drawGeom.AddLineString(2, DSegment3d::From(box[0], box[3]).point, nullptr);
        drawGeom.AddLineString(2, DSegment3d::From(box[4], box[5]).point, nullptr);
        drawGeom.AddLineString(2, DSegment3d::From(box[1], box[7]).point, nullptr);
        drawGeom.AddLineString(2, DSegment3d::From(box[2], box[6]).point, nullptr);
        return;
        }

    tmpPts[0] = box[0];
    tmpPts[1] = box[1];
    tmpPts[2] = box[2];
    tmpPts[3] = box[3];
    tmpPts[4] = box[0];

    drawGeom.AddLineString(5, tmpPts, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::_VisitElement(GeometrySourceCR source)
    {
    if (_CheckStop())
        return ERROR;

    if (IsUndisplayed(source))
        return SUCCESS;

    _OutputElement(source);

    // Output element or local range for debugging if requested...
    switch (GetDrawPurpose())
        {
        case DrawPurpose::FitView:
        case DrawPurpose::CaptureGeometry:
            break; // Don't do this when trying to compute range or drop!

        default:
            {
            static int s_drawRange; // 0 - Host Setting (Bounding Box Debug), 1 - Bounding Box, 2 - Element Range
            if (nullptr == m_viewport || !s_drawRange)
                break;

            DPoint3d  p[8];
            BoundingBox3d  range = (2 == s_drawRange ? BoundingBox3d(source.CalculateRange3d()) : (nullptr != source.ToGeometrySource3d() ? 
                                    BoundingBox3d(source.ToGeometrySource3d()->GetPlacement().GetElementBox()) : 
                                    BoundingBox3d(source.ToGeometrySource2d()->GetPlacement().GetElementBox())));
            Transform placementTrans = (2 == s_drawRange ? Transform::FromIdentity() : source.GetPlacementTransform());

            p[0].x = p[3].x = p[4].x = p[5].x = range.low.x;
            p[1].x = p[2].x = p[6].x = p[7].x = range.high.x;
            p[0].y = p[1].y = p[4].y = p[7].y = range.low.y;
            p[2].y = p[3].y = p[5].y = p[6].y = range.high.y;
            p[0].z = p[1].z = p[2].z = p[3].z = range.low.z;
            p[4].z = p[5].z = p[6].z = p[7].z = range.high.z;

            m_ovrMatSymb.SetLineColor(m_viewport->MakeTransparentIfOpaque(m_viewport->AdjustColorForContrast(m_elemMatSymb.GetLineColor(), m_viewport->GetBackgroundColor()), 150));
            m_ovrMatSymb.SetWidth(1);
            _AddContextOverrides(m_ovrMatSymb);
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
            GetCurrentGraphicR().ActivateOverrideMatSymb(&m_ovrMatSymb);
#endif

            PushTransform(placementTrans);
            DrawBox(p, nullptr != source.ToGeometrySource3d());
            PopTransformClip();

            ResetContextOverrides();
            break;
            }
        }

    m_currentGeomSource = nullptr;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_VisitTransientGraphics(bool isPreUpdate)
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    Render::Output*    output = (IsAttached() ? GetViewport()->GetIViewOutput() : nullptr);
    bool            restoreZWrite = (output && isPreUpdate ? output->EnableZWriting(false) : false);

    T_HOST.GetGraphicsAdmin()._CallViewTransients(*this, isPreUpdate);

    if (restoreZWrite)
        output->EnableZWriting(true);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* private callback (called from scanner)
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt visitElementFunc(DgnElementCR element, void* inContext, ScanCriteriaR sc)
    {
    GeometrySourceCP geomElement = element.ToGeometrySource();
    if (nullptr == geomElement)
        return SUCCESS;
    
    ViewContextR context = *(ViewContext*)inContext;
    return context.VisitElement(*geomElement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_SetScanReturn()
    {
    m_scanCriteria->SetRangeNodeCheck(this);
    m_scanCriteria->SetElementCallback(visitElementFunc, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
ScanCriteria::Result  ViewContext::_CheckNodeRange(ScanCriteriaCR scanCriteria, DRange3dCR testRange, bool is3d)
    {
    return ClipPlaneContainment_StronglyOutside != m_transformClipStack.ClassifyElementRange(testRange, is3d, true) ? ScanCriteria::Result::Pass : ScanCriteria::Result::Fail;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::IsWorldPointVisible(DPoint3dCR worldPoint, bool boresite)
    {
    DPoint3d    localPoint;

    WorldToLocal(&localPoint, &worldPoint, 1);

    return IsLocalPointVisible(localPoint, boresite);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::IsLocalPointVisible(DPoint3dCR localPoint, bool boresite)
    {
    if (m_transformClipStack.IsEmpty())
        return true;

    if (!boresite)
        return m_transformClipStack.TestPoint(localPoint);

    DVec3d      localZVec;

    if (IsCameraOn())
        {
        DPoint3d        localCamera;
        
        WorldToLocal(&localCamera, &GetViewport()->GetCamera().GetEyePoint(), 1);
        localZVec.NormalizedDifference(localPoint, localCamera);
        }
    else
        {
        DPoint3d        zPoints[2];
        Transform       worldToLocal;

        zPoints[0].Zero();
        zPoints[1].Init(0.0, 0.0, 1.0);

        NpcToWorld(zPoints, zPoints, 2);

        localZVec.NormalizedDifference(zPoints[1], zPoints[0]);

        if (GetCurrWorldToLocalTrans(worldToLocal))
            {
            worldToLocal.MultiplyMatrixOnly(localZVec);
            localZVec.Normalize();
            }
        }

    return  m_transformClipStack.TestRay(localPoint, localZVec);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::PointInsideClip(DPoint3dCR point)
    {
    return m_transformClipStack.TestPoint(point);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::GetRayClipIntersection(double& distance, DPoint3dCR origin, DVec3dCR direction)
    {
    return  m_transformClipStack.GetRayIntersection(distance, origin, direction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::_ScanDgnModel(DgnModelP model)
    {
    if (!ValidateScanRange())
        return ERROR;

    m_scanCriteria->SetDgnModel(model);
    return m_scanCriteria->Scan(this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::_VisitDgnModel(DgnModelP modelRef)
    {
    if (CheckStop())
        return ERROR;

    return _ScanDgnModel(modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::SetSubRectFromViewRect(BSIRectCP viewRect)
    {
    if (nullptr == viewRect)
        return;

    BSIRect tRect = *viewRect;
    tRect.Expand(1);

    DRange3d viewRange;
    viewRange.low.Init(tRect.origin.x, tRect.corner.y, 0.0);
    viewRange.high.Init(tRect.corner.x, tRect.origin.y, 0.0);

    GetViewport()->ViewToNpc(&viewRange.low, &viewRange.low, 2);

    // this is due to fact that y's can be reversed from view to npc
    DRange3d npcSubRect;
    npcSubRect.InitFrom(&viewRange.low, 2);
    SetSubRectNpc(npcSubRect);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::SetSubRectNpc(DRange3dCR subRect)
    {
    m_npcSubRange = subRect;
    m_npcSubRange.low.z  = 0.0;                // make sure entire z range.
    m_npcSubRange.high.z = 1.0;
    m_useNpcSubRange = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::VisitAllViewElements(bool includeTransients, BSIRectCP updateRect)
    {
    ClearAborted();
    if (nullptr != updateRect)
        SetSubRectFromViewRect(updateRect);

    _InitScanRangeAndPolyhedron();

    SetScanReturn();
    _VisitAllModelElements(includeTransients);

    return WasAborted();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::_VisitAllModelElements(bool includeTransients)
    {
    if (includeTransients)
        _VisitTransientGraphics(true);

    PhysicalViewControllerCP physController = m_viewport->GetPhysicalViewControllerCP();
    ClipVectorPtr clipVector = physController ? physController->GetClipVector() : nullptr;
    if (clipVector.IsValid())
        PushClip(*clipVector);

    // The ViewController must orchestrate the display of all of the elements in the view.
    m_viewport->GetViewControllerR().DrawView(*this);

    if (clipVector.IsValid())
        PopTransformClip();

    if (includeTransients) // Give post-update IViewTransients a chance to display even if aborted the element draw...
        _VisitTransientGraphics(false);

    return WasAborted();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::VisitHit(HitDetailCR hit)
    {
    ClearAborted();
    _InitScanRangeAndPolyhedron();

    return m_viewport->GetViewController().VisitHit(hit, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawStyledLineString3d(int nPts, DPoint3dCP pts, DPoint3dCP range, bool closed)
    {
    if (nPts < 1)
        return;

    LineStyleSymbP  currLsSymb;
    ILineStyleCP    currLStyle = _GetCurrLineStyle(&currLsSymb);

    if (currLStyle && (nPts > 2 || !pts->IsEqual(pts[1])))
        {
        currLStyle->_GetComponent()->_StrokeLineString(this, currLsSymb, pts, nPts, closed);
        return;
        }

    if (closed)
        GetCurrentGraphicR().AddShape(nPts, pts, false, range);
    else
        GetCurrentGraphicR().AddLineString(nPts, pts, range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawStyledLineString2d(int nPts, DPoint2dCP pts, double priority, DPoint2dCP range, bool closed)
    {
    if (nPts < 1)
        return;

    LineStyleSymbP  currLsSymb;
    ILineStyleCP    currLStyle = _GetCurrLineStyle(&currLsSymb);

    if (currLStyle && (nPts > 2 || !pts->IsEqual(pts[1])))
        {
        currLStyle->_GetComponent()->_StrokeLineString2d(this, currLsSymb, pts, nPts, priority, closed);
        return;
        }

    if (closed)
        GetCurrentGraphicR().AddShape2d(nPts, pts, false, priority, range);
    else
        GetCurrentGraphicR().AddLineString2d(nPts, pts, priority, range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawStyledArc3d(DEllipse3dCR ellipse, bool isEllipse, DPoint3dCP range)
    {
    LineStyleSymbP  currLsSymb;
    ILineStyleCP    currLStyle = _GetCurrLineStyle(&currLsSymb);

    if (currLStyle)
        {
        double      r0, r1, start, sweep;
        RotMatrix   rMatrix;
        DPoint3d    center;

        ellipse.GetScaledRotMatrix(center, rMatrix, r0, r1, start, sweep);
        currLStyle->_GetComponent()->_StrokeArc(this, currLsSymb, &center, &rMatrix, r0, r1, isEllipse ? nullptr : &start, isEllipse ? nullptr : &sweep, range);
        return;
        }

    GetCurrentGraphicR().AddArc(ellipse, isEllipse, false, range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawStyledArc2d(DEllipse3dCR ellipse, bool isEllipse, double zDepth, DPoint2dCP range)
    {
    LineStyleSymbP  currLsSymb;
    ILineStyleCP    currLStyle = _GetCurrLineStyle(&currLsSymb);

    if (currLStyle)
        {
        double      r0, r1, start, sweep;
        RotMatrix   rMatrix;
        DPoint3d    center;

        ellipse.GetScaledRotMatrix(center, rMatrix, r0, r1, start, sweep);
        center.z = zDepth; // Set priority on center...
        currLStyle->_GetComponent()->_StrokeArc(this, currLsSymb, &center, &rMatrix, r0, r1, isEllipse ? nullptr : &start, isEllipse ? nullptr : &sweep, nullptr);
        return;
        }

    GetCurrentGraphicR().AddArc2d(ellipse, isEllipse, false, zDepth, range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawStyledBSplineCurve3d(MSBsplineCurveCR bcurve)
    {
    LineStyleSymbP  currLsSymb;
    ILineStyleCP    currLStyle = _GetCurrLineStyle(&currLsSymb);

    if (currLStyle)
        {
        currLStyle->_GetComponent()->_StrokeBSplineCurve(this, currLsSymb, &bcurve, nullptr);
        return;
        }

    GetCurrentGraphicR().AddBSplineCurve(bcurve, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawStyledBSplineCurve2d(MSBsplineCurveCR bcurve, double zDepth)
    {
    LineStyleSymbP  currLsSymb;
    ILineStyleCP    currLStyle = _GetCurrLineStyle(&currLsSymb);

    if (currLStyle)
        {
        if (0.0 == zDepth)
            {
            currLStyle->_GetComponent()->_StrokeBSplineCurve(this, currLsSymb, &bcurve, nullptr);
            return;
            }

        // NOTE: Copy curve and set priority on poles since we won't be drawing cached...
        MSBsplineCurvePtr tmpCurve = bcurve.CreateCopy();
        bool useWeights = tmpCurve->rational && nullptr != tmpCurve->GetWeightCP();
        for (int iPoint = 0; iPoint < tmpCurve->params.numPoles; ++iPoint)
            {
            DPoint3d xyz = tmpCurve->GetPole(iPoint);
            if (useWeights)
                xyz.z = zDepth * tmpCurve->GetWeight(iPoint);
            else
                xyz.z = zDepth;
            tmpCurve->SetPole(iPoint, xyz);
            }

        currLStyle->_GetComponent()->_StrokeBSplineCurve(this, currLsSymb, tmpCurve.get(), nullptr);
        return;
        }

    GetCurrentGraphicR().AddBSplineCurve2d(bcurve, false, zDepth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   10/03
+---------------+---------------+---------------+---------------+---------------+------*/
DMatrix4d ViewContext::GetViewToLocal() const
    {
    DMatrix4d  viewToLocal = GetWorldToView().M1;
    Transform  worldToLocalTransform;

    if (SUCCESS == GetCurrWorldToLocalTrans(worldToLocalTransform) && !worldToLocalTransform.IsIdentity())
        {
        DMatrix4d worldToLocal = DMatrix4d::From(worldToLocalTransform);
        viewToLocal.InitProduct(worldToLocal, viewToLocal);
        }

    return viewToLocal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   10/03
+---------------+---------------+---------------+---------------+---------------+------*/
DMatrix4d ViewContext::GetLocalToView() const
    {
    DMatrix4d localToView = GetWorldToView().M0;
    Transform localToWorldTransform;

    if (SUCCESS == GetCurrLocalToWorldTrans(localToWorldTransform) && !localToWorldTransform.IsIdentity())
        {
        DMatrix4d   localToWorld = DMatrix4d::From(localToWorldTransform);
        localToView.InitProduct(localToView, localToWorld);
        }

    return localToView;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/03
+---------------+---------------+---------------+---------------+---------------+------*/
double ViewContext::GetPixelSizeAtPoint(DPoint3dCP inPoint) const
    {
    DPoint3d    vec[2];

    if (nullptr != inPoint)
        LocalToView(vec, inPoint, 1); // convert point to pixels
    else
        {
        DPoint3d    center = {.5, .5, .5};   // if they didn't give a point, use center of view.
        NpcToView(vec, &center, 1);
        }

    vec[1] = vec[0];
    vec[1].x += 1.0;

    // Convert pixels back to local coordinates and use the length as tolerance
    ViewToLocal(vec, vec, 2);

    return vec[0].Distance(vec[1]);
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ViewContext::RasterDisplayParams::RasterDisplayParams()
    : m_flags(0), m_contrast(50), m_brightness(50), m_greyScale(false), m_applyBinaryWhiteOnWhiteReversal(false), m_quality(1.0)
    {
    m_backgroundColor = ColorDef::Black();      // Background color for binary image.
    m_foregroundColor = ColorDef::White();      // Foreground color for binary image.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::RasterDisplayParams::operator==(RasterDisplayParams const& rhs) const
    {
    if (this == &rhs)
        return true;

    return (memcmp(this, &rhs, sizeof(*this)) == 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::RasterDisplayParams::operator!=(RasterDisplayParams const& rhs) const
    {
    return !(operator==(rhs));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::RasterDisplayParams::SetContrast(int8_t value)     
    {
    m_contrast = value;
    m_flags |= ViewContext::RasterDisplayParams::RASTER_PARAM_Contrast;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::RasterDisplayParams::SetBrightness(int8_t value)   
    {
    m_brightness = value;
    m_flags |= ViewContext::RasterDisplayParams::RASTER_PARAM_Brightness;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::RasterDisplayParams::SetGreyscale(bool value)
    {
    m_greyScale = value;
    m_flags |= ViewContext::RasterDisplayParams::RASTER_PARAM_GreyScale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::RasterDisplayParams::SetApplyBinaryWhiteOnWhiteReversal(bool value)
    {
    m_applyBinaryWhiteOnWhiteReversal = value;
    m_flags |= ViewContext::RasterDisplayParams::RASTER_PARAM_ApplyBinaryWhiteOnWhiteReversal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::RasterDisplayParams::SetEnableGrid(bool value)
    {
    m_enableGrid = value;
    m_flags |= ViewContext::RasterDisplayParams::RASTER_PARAM_EnableGrid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::RasterDisplayParams::SetBackgroundColor(ColorDef value)
    {
    m_backgroundColor = value;
    m_flags |= ViewContext::RasterDisplayParams::RASTER_PARAM_BackgroundColor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::RasterDisplayParams::SetForegroundColor(ColorDef value)
    {
    m_foregroundColor = value;
    m_flags |= ViewContext::RasterDisplayParams::RASTER_PARAM_ForegroundColor;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::RasterDisplayParams::SetQualityFactor(double factor)
    {
    m_quality = factor;
    m_flags |= ViewContext::RasterDisplayParams::RASTER_PARAM_Quality;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::ContextMark::Pop()
    {
    if (nullptr == m_context)
        return;

    while (m_context->GetTransClipDepth() > m_transClipMark)
        m_context->GetTransformClipStack().Pop(*m_context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
inline void ViewContext::ContextMark::SetNow()
    {
    m_transClipMark = m_context->GetTransClipDepth();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
ViewContext::ContextMark::ContextMark(ViewContextP context)
    {
    if (nullptr == (m_context = context))
        Init(context);
    else
        SetNow();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/15
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t ViewContext::ResolveNetDisplayPriority(int32_t geomPriority, DgnSubCategoryId subCategoryId, DgnSubCategory::Appearance* appearanceIn) const
    {
    if (m_is3dView)
        return 0;

    // SubCategory display priority is combined with element priority to compute net display priority. 
    int32_t netPriority = geomPriority;

    if (nullptr == appearanceIn)
        {
        if (!subCategoryId.IsValid())
            return netPriority;

        DgnSubCategory::Appearance appearance;

        if (nullptr != GetViewport())
            {
            appearance = GetViewport()->GetViewController().GetSubCategoryAppearance(subCategoryId);
            }
        else
            {
            DgnSubCategoryCPtr subCat = DgnSubCategory::QuerySubCategory(subCategoryId, GetDgnDb());
            BeAssert(subCat.IsValid());
            if (!subCat.IsValid())
                return netPriority;
            else
                appearance = subCat->GetAppearance();
            }

        netPriority += appearance.GetDisplayPriority();
        }
    else
        {
        netPriority += appearanceIn->GetDisplayPriority();
        }

    int32_t displayRange[2];

    if (GetDisplayPriorityRange(displayRange[0], displayRange[1]))
        LIMIT_RANGE (displayRange[0], displayRange[1], netPriority);

    return netPriority;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
ElemMatSymb::ElemMatSymb()
    {
    Init();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/02
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemMatSymb::Init()
    {
    m_lineColor         = ColorDef::Black();
    m_fillColor         = ColorDef::Black();
    m_isFilled          = false;
    m_isBlankingRegion  = false;
    m_rasterWidth       = 1;
    m_patternParams     = nullptr;
    m_gradient          = nullptr;
    m_material          = nullptr;
    m_lStyleSymb.Clear();
    }

#if defined (WIP_PLOTTING)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   08/03
+---------------+---------------+---------------+---------------+---------------+------*/
static Byte screenColor(Byte color, double factor)
    {
    uint32_t tmp = color + (uint32_t) (factor * (255 - color));
    LIMIT_RANGE (0, 255, tmp);
    return  (Byte) tmp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AndrewEdge      08/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void     applyScreeningFactor(ColorDef* rgb, double screeningFactor)
    {
    rgb->SetRed(screenColor(rgb->GetRed(),   screeningFactor));
    rgb->SetGreen(screenColor(rgb->GetGreen(), screeningFactor));
    rgb->SetBlue(screenColor(rgb->GetBlue(),  screeningFactor));
    }
#endif


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemMatSymb::Cook(ElemDisplayParamsCR elParams, ViewContextR context, DPoint3dCP startTangent, DPoint3dCP endTangent)
    {
    Init();

    DgnViewportP vp = context.GetViewport();

    m_rasterWidth = nullptr != vp ? vp->GetIndexedLineWidth(elParams.GetWeight()) : DgnViewport::GetDefaultIndexedLineWidth(elParams.GetWeight());
    m_lineColor = m_fillColor = elParams.GetLineColor(); // NOTE: In case no fill is defined it should be set the same as line color...

    double netElemTransparency = elParams.GetNetTransparency();
    double netFillTransparency = elParams.GetNetFillTransparency();

    if (FillDisplay::Never != elParams.GetFillDisplay())
        {
        if (nullptr != elParams.GetGradient())
            {
            m_gradient = GradientSymb::Create();
            m_gradient->CopyFrom(*elParams.GetGradient());

            m_fillColor = ColorDef::White(); // Fill should be set to opaque white for gradient texture...

            if (0 == (m_gradient->GetFlags() & static_cast<int>(GradientFlags::Outline)))
                {
                m_lineColor.SetAlpha(0xff); // Qvis checks for this to disable auto-outline...
                netElemTransparency = 0.0;  // Don't override the fully transparent outline below...
                }
            }
        else
            {
            m_fillColor = elParams.GetFillColor();
            }

        m_isFilled = true;
        m_isBlankingRegion = (FillDisplay::Blanking == elParams.GetFillDisplay());
        }

    if (vp && elParams.GetMaterialId().IsValid())
        m_material = vp->GetRenderTarget()->GetMaterial(elParams.GetMaterialId(), context.GetDgnDb());

    if (0.0 != netElemTransparency)
        {
        Byte netTransparency = (Byte) (netElemTransparency * 255.0);

        if (netTransparency > 250)
            netTransparency = 250; // Don't allow complete transparency.

        m_lineColor.SetAlpha(netTransparency);
        }

    if (0.0 != netFillTransparency)
        {
        Byte netTransparency = (Byte) (netFillTransparency * 255.0);

        if (netTransparency > 250)
            netTransparency = 250; // Don't allow complete transparency.

        m_fillColor.SetAlpha(netTransparency);
        }

#if defined (WIP_PLOTTING)    
    if (elParams.IsScreeningSet() && (elParams.GetScreening() >= SCREENING_Full) && (elParams.GetScreening() < SCREENING_None))
        {
        double  screeningFactor = (SCREENING_None - elParams.GetScreening()) / SCREENING_None;

        applyScreeningFactor(&m_lineColor, screeningFactor);
        applyScreeningFactor(&m_fillColor, screeningFactor);
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* compare two ElemMatSymb's to see if they're the same.
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElemMatSymb::operator==(ElemMatSymbCR rhs) const
    {
    if (this == &rhs)
        return true;

    if (rhs.m_lineColor        != m_lineColor        ||
        rhs.m_fillColor        != m_fillColor        ||
        rhs.m_isFilled         != m_isFilled         ||
        rhs.m_isBlankingRegion != m_isBlankingRegion ||
        rhs.m_material         != m_material         ||
        rhs.m_rasterWidth      != m_rasterWidth)
        return false;

    if (!(rhs.m_gradient == m_gradient))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater  08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ElemMatSymb::ElemMatSymb(ElemMatSymbCR rhs)
    {
    m_lineColor         = rhs.m_lineColor;
    m_fillColor         = rhs.m_fillColor;
    m_isFilled          = rhs.m_isFilled;
    m_isBlankingRegion  = rhs.m_isBlankingRegion;
    m_material          = rhs.m_material;
    m_rasterWidth       = rhs.m_rasterWidth;
    m_lStyleSymb        = rhs.m_lStyleSymb;
    m_gradient          = rhs.m_gradient;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater  08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ElemMatSymbR ElemMatSymb::operator=(ElemMatSymbCR rhs)
    {
    m_lineColor         = rhs.m_lineColor;
    m_fillColor         = rhs.m_fillColor;
    m_isFilled          = rhs.m_isFilled;
    m_isBlankingRegion  = rhs.m_isBlankingRegion;
    m_material          = rhs.m_material;
    m_rasterWidth       = rhs.m_rasterWidth;
    m_lStyleSymb        = rhs.m_lStyleSymb;
    m_gradient          = rhs.m_gradient;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void OvrMatSymb::Clear()
    {
    SetFlags(FLAGS_None);
    m_matSymb.Init();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void  OvrMatSymb::SetLineStyle(int32_t styleNo, DgnModelR modelRef, DgnModelR styleDgnModel, LineStyleParamsCP lStyleParams, ViewContextR context, DPoint3dCP startTangent, DPoint3dCP endTangent)
    {
#ifdef WIP_VANCOUVER_MERGE // linestyle
    m_matSymb.GetLineStyleSymbR().FromResolvedStyle(styleNo, modelRef, styleDgnModel, lStyleParams, context, startTangent, endTangent);
    m_flags |= MATSYMB_OVERRIDE_Style;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater  08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ElemDisplayParams::ElemDisplayParams(ElemDisplayParamsCR rhs)
    {
    m_appearanceOverrides   = rhs.m_appearanceOverrides;
    m_categoryId            = rhs.m_categoryId;
    m_subCategoryId         = rhs.m_subCategoryId;
    m_elmPriority           = rhs.m_elmPriority;
    m_netPriority           = rhs.m_netPriority;
    m_weight                = rhs.m_weight;       
    m_geometryClass         = rhs.m_geometryClass;       
    m_lineColor             = rhs.m_lineColor;
    m_fillColor             = rhs.m_fillColor;
    m_fillDisplay           = rhs.m_fillDisplay;
    m_elmTransparency       = rhs.m_elmTransparency;
    m_netElmTransparency    = rhs.m_netElmTransparency;
    m_fillTransparency      = rhs.m_fillTransparency;
    m_netFillTransparency   = rhs.m_netFillTransparency;
    m_materialId            = rhs.m_materialId;
    m_styleInfo             = rhs.m_styleInfo;
    m_gradient              = rhs.m_gradient;
    m_pattern               = rhs.m_pattern;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater  08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ElemDisplayParamsR ElemDisplayParams::operator=(ElemDisplayParamsCR rhs)
    {
    m_appearanceOverrides   = rhs.m_appearanceOverrides;
    m_categoryId            = rhs.m_categoryId;
    m_subCategoryId         = rhs.m_subCategoryId;
    m_elmPriority           = rhs.m_elmPriority;
    m_netPriority           = rhs.m_netPriority;
    m_weight                = rhs.m_weight;
    m_geometryClass         = rhs.m_geometryClass;       
    m_lineColor             = rhs.m_lineColor;
    m_fillColor             = rhs.m_fillColor;
    m_fillDisplay           = rhs.m_fillDisplay;
    m_elmTransparency       = rhs.m_elmTransparency;
    m_netElmTransparency    = rhs.m_netElmTransparency;
    m_fillTransparency      = rhs.m_fillTransparency;
    m_netFillTransparency   = rhs.m_netFillTransparency;
    m_materialId            = rhs.m_materialId;
    m_styleInfo             = rhs.m_styleInfo;
    m_gradient              = rhs.m_gradient;
    m_pattern               = rhs.m_pattern;

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ElemDisplayParams::ElemDisplayParams()
    {
    m_resolved = false;
    m_elmPriority = 0;
    m_netPriority = 0;    
    m_weight = 0;
    m_fillDisplay = FillDisplay::Never;
    m_elmTransparency = 0;             
    m_netElmTransparency = 0;          
    m_fillTransparency = 0;            
    m_netFillTransparency = 0;         
    m_geometryClass = DgnGeometryClass::Primary; 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
void ElemDisplayParams::ResetAppearance()
    {
    AutoRestore<DgnCategoryId> saveCategory(&m_categoryId);
    AutoRestore<DgnSubCategoryId> saveSubCategory(&m_subCategoryId);

    *this = ElemDisplayParams();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElemDisplayParams::operator==(ElemDisplayParamsCR rhs) const
    {
    if (this == &rhs)
        return true;

    if (rhs.m_categoryId    != m_categoryId ||
        rhs.m_subCategoryId != m_subCategoryId ||
        rhs.m_elmPriority   != m_elmPriority ||
        rhs.m_netPriority   != m_netPriority)
        return false;

    if (rhs.m_lineColor     != m_lineColor ||
        rhs.m_weight        != m_weight ||
        rhs.m_geometryClass != m_geometryClass)
        return false;

    if (rhs.m_fillColor             != m_fillColor ||
        rhs.m_fillDisplay           != m_fillDisplay ||
        rhs.m_elmTransparency       != m_elmTransparency ||
        rhs.m_netElmTransparency    != m_netElmTransparency ||
        rhs.m_fillTransparency      != m_fillTransparency ||
        rhs.m_netFillTransparency   != m_netFillTransparency)
        return false;

    if (0 != memcmp(&rhs.m_appearanceOverrides, &m_appearanceOverrides, sizeof(m_appearanceOverrides)))
        return false;

    if (!(m_materialId == rhs.m_materialId))
        return false;

    if (!(m_gradient == rhs.m_gradient))
        return false;

    if (!(m_pattern == rhs.m_pattern))
        return false;

    if (!(m_styleInfo == rhs.m_styleInfo))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemDisplayParams::Resolve(ViewContextR context)
    {
    DgnSubCategoryId subCategoryId = GetSubCategoryId();

    BeAssert(subCategoryId.IsValid());
    if (!subCategoryId.IsValid())
        return;

    // Setup from SubCategory appearance...
    DgnSubCategory::Appearance appearance;

    if (nullptr != context.GetViewport())
        {
        appearance = context.GetViewport()->GetViewController().GetSubCategoryAppearance(subCategoryId);
        }
    else
        {
        DgnSubCategoryCPtr subCat = DgnSubCategory::QuerySubCategory(subCategoryId, context.GetDgnDb());
        BeAssert(subCat.IsValid());
        if (!subCat.IsValid())
            return;

        appearance = subCat->GetAppearance();
        }

    if (!m_appearanceOverrides.m_color)
        m_lineColor = appearance.GetColor();

    if (m_appearanceOverrides.m_bgFill)
        {
        m_fillColor = (nullptr != context.GetViewport() ? context.GetViewport()->GetBackgroundColor() : ColorDef::Black());

        // NEEDSWORK_ASK_QVIS_FOLKS: Problem with white-on-white reversal...don't want this to apply to the background fill... :(
        if (ColorDef::White() == m_fillColor)
            m_fillColor.SetRed(254);
        }
    else if (!m_appearanceOverrides.m_fill)
        {
        m_fillColor = appearance.GetColor();
        }

    if (!m_appearanceOverrides.m_weight)
        m_weight = appearance.GetWeight();

    if (!m_appearanceOverrides.m_style)
        m_styleInfo = LineStyleInfo::Create(appearance.GetStyle(), nullptr).get(); // WIP_LINESTYLE - Need LineStyleParams...

    if (!m_appearanceOverrides.m_material)
        m_materialId = appearance.GetMaterial();

    // SubCategory transparency is combined with element transparency to compute net transparency. 
    if (0.0 != appearance.GetTransparency())
        {
        // combine transparencies by multiplying the opaqueness.
        // A 50% transparent element on a 50% transparent category should give a 75% transparent result.
        // (1 - ((1 - .5) * (1 - .5))
        double elementOpaque = 1.0 - m_elmTransparency;
        double fillOpaque = 1.0 - m_fillTransparency;
        double categoryOpaque = 1.0 - appearance.GetTransparency();

        m_netElmTransparency = (1.0 - (elementOpaque * categoryOpaque));
        m_netFillTransparency = (1.0 - (fillOpaque * categoryOpaque));
        }

    // SubCategory display priority is combined with element priority to compute net display priority. 
    m_netPriority = context.ResolveNetDisplayPriority(m_elmPriority, subCategoryId, &appearance);
    m_resolved = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_AddTextString(TextStringCR text)
    {
    text.GetGlyphSymbology(GetCurrentDisplayParams());
    CookDisplayParams();

    double zDepth = GetCurrentDisplayParams().GetNetDisplayPriority();
    GetCurrentGraphicR().AddTextString(text, Is3dView() ? nullptr : &zDepth);                
    text.DrawTextAdornments(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::SetLinestyleTangents(DPoint3dCP start, DPoint3dCP end)
    {
    m_startTangent = start;
    m_endTangent = end;
    m_elemMatSymb.GetLineStyleSymbR().ClearContinuationData();
    m_ovrMatSymb.GetMatSymbR().GetLineStyleSymbR().ClearContinuationData();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::IsCameraOn() const
    {
    return m_viewport->IsCameraOn();
    }

