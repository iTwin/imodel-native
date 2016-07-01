/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ViewContext.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
    m_purpose = DrawPurpose::NotSpecified;
    m_worldToNpc.InitIdentity();
    m_worldToView.InitIdentity();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::_VisitElement(DgnElementId elementId, bool allowLoad) 
    {
    DgnElements& pool = m_dgndb->Elements();
    DgnElementCPtr el = allowLoad ? pool.GetElement(elementId) : pool.FindElement(elementId);
    if (!el.IsValid())
        {
        BeAssert(!allowLoad);
        return ERROR;
        }

    GeometrySourceCP geomElem = el->ToGeometrySource();
    if (nullptr == geomElem)
        return ERROR;

    return VisitGeometry(*geomElem);
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
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::_InitContextForView()
    {
    m_worldToNpc  = *m_viewport->GetWorldToNpcMap();
    m_worldToView = *m_viewport->GetWorldToViewMap();
    m_scanRangeValid = false;

    m_frustumPlanes.Init(GetFrustum());

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
* @bsimethod                                                    KeithBentley    11/02
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_InitScanRangeAndPolyhedron()
    {
    // set up scanner search criteria
    _ScanRangeFromPolyhedron();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_SetDgnDb(DgnDbR dgnDb)
    {
    m_dgndb = &dgnDb;
    _SetupScanCriteria();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  02/05
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_SetupScanCriteria()
    {
    DgnViewportP vp = GetViewport();
    if (nullptr != vp)
        m_scanCriteria.SetCategoryTest(vp->GetViewController().GetViewedCategories());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::Attach(DgnViewportP viewport, DrawPurpose purpose)
    {
    if (nullptr == viewport)
        return  ERROR;

    m_viewport = viewport;
    m_purpose = purpose;
    ClearAborted();

    m_is3dView = viewport->Is3dView();
    SetViewFlags(viewport->GetViewFlags());

    return _InitContextForView();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::IsCameraOn() const
    {
    return m_viewport->IsCameraOn();
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
* convert the view context polyhedron to scan parameters in the scanCriteria.
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::_ScanRangeFromPolyhedron()
    {
    Frustum polyhedron = GetFrustum();

    // get enclosing bounding box around polyhedron (outside scan range).
    DRange3d scanRange = polyhedron.ToRange();

    if (!Is3dView())
        {
        scanRange.low.z = -1;
        scanRange.high.z = 1;
        }

    m_scanCriteria.SetRangeTest(&scanRange);

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
            m_scanCriteria.SetSkewRangeTest(&scanRange, &skewRange, &skewVec);
        }

    m_scanRangeValid = true;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr ViewContext::_StrokeGeometry(GeometrySourceCR source, double pixelSize)
    {
    Render::GraphicPtr graphic = (nullptr != m_viewport) ?
                m_viewport->GetViewControllerR()._StrokeGeometry(*this, source, pixelSize) :
                source.Stroke(*this, pixelSize);

    // if we aborted, the graphic may not be complete, don't save it
    return WasAborted() ? nullptr : graphic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::_OutputGeometry(GeometrySourceCR source)
    {
    if (!source.HasGeometry())
        return ERROR;

    DPoint3d origin;
    source.GetPlacementTransform().GetTranslation(origin);
    double pixelSize = (nullptr != m_viewport ? m_viewport->GetPixelSizeAtPoint(&origin) : 0.0);

    Render::GraphicPtr graphic = _GetCachedGraphic(source, pixelSize);
    if (!graphic.IsValid())
        graphic = _StrokeGeometry(source, pixelSize);

    if (!graphic.IsValid())
        return ERROR;

    _OutputGraphic(*graphic, &source);

    static int s_drawRange; // 0 - Host Setting (Bounding Box Debug), 1 - Bounding Box, 2 - Element Range
    if (!s_drawRange)
        return SUCCESS;

    // Output element local range for debug display and locate...
    if (!graphic->IsForDisplay() && nullptr == GetIPickGeom())
        return SUCCESS;

    Render::GraphicBuilderPtr rangeGraphic = CreateGraphic(Graphic::CreateParams(nullptr, (2 == s_drawRange ? Transform::FromIdentity() : source.GetPlacementTransform())));
    Render::GeometryParams rangeParams;

    rangeParams.SetCategoryId(source.GetCategoryId()); // Need category for pick...
    rangeParams.SetLineColor(DgnViewport::MakeColorTransparency(m_viewport->AdjustColorForContrast(ColorDef::LightGrey(), m_viewport->GetBackgroundColor()), 0x64));
    CookGeometryParams(rangeParams, *rangeGraphic);

    if (nullptr != source.ToGeometrySource3d())
        {
        BoundingBox3d range = (2 == s_drawRange ? BoundingBox3d(source.CalculateRange3d()) : BoundingBox3d(source.ToGeometrySource3d()->GetPlacement().GetElementBox()));

        rangeGraphic->AddRangeBox(range);
        }
    else
        {
        BoundingBox3d range = (2 == s_drawRange ? BoundingBox3d(source.CalculateRange3d()) : BoundingBox3d(source.ToGeometrySource2d()->GetPlacement().GetElementBox()));

        rangeGraphic->AddRangeBox2d(DRange2d::From(DPoint2d::From(range.low), DPoint2d::From(range.high)), 0.0);
        }

    _OutputGraphic(*rangeGraphic, &source);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr ViewContext::_AddSubGraphic(Render::GraphicBuilderR graphic, DgnGeometryPartId partId, TransformCR subToGraphic, Render::GeometryParamsR geomParams)
    {
    DgnGeometryPartCPtr partGeometry = GetDgnDb().Elements().Get<DgnGeometryPart>(partId);

    if (!partGeometry.IsValid())
        return nullptr;

    bool isForDisplay = graphic.IsForDisplay();

    if (!isForDisplay && m_viewport)
        {
        Transform partToWorld = Transform::FromProduct(graphic.GetLocalToWorldTransform(), subToGraphic);
        ElementAlignedBox3d range = partGeometry->GetBoundingBox();

        partToWorld.Multiply(range, range);

        if (!IsRangeVisible(range))
            return nullptr; // Part range doesn't overlap pick...
        }

    Render::GraphicPtr partGraphic = (isForDisplay ? partGeometry->Graphics().Find(*m_viewport, graphic.GetPixelSize()) : nullptr);

    if (!partGraphic.IsValid())
        {
        GeometryStreamIO::Collection collection(partGeometry->GetGeometryStream().GetData(), partGeometry->GetGeometryStream().GetSize());

        auto partBuilder = graphic.CreateSubGraphic(subToGraphic);
        partGraphic = partBuilder;
        collection.Draw(*partBuilder, *this, geomParams, false, partGeometry.get());
            
        if (WasAborted()) // if we aborted, the graphic may not be complete, don't save it
            return nullptr;

        partBuilder->Close(); 
        if (isForDisplay)
            partGeometry->Graphics().Save(*partGraphic);
        }

    // NOTE: Need to cook GeometryParams to get GraphicParams, but we don't want to activate and bake into our QvElem...
    GraphicParams graphicParams;
    _CookGeometryParams(geomParams, graphicParams);
    graphic.AddSubGraphic(*partGraphic, subToGraphic, graphicParams);

    return partGraphic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_AddViewOverrides(OvrGraphicParamsR ovrMatSymb)
    {
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
void ViewContext::_AddContextOverrides(OvrGraphicParamsR ovrMatSymb, GeometrySourceCP source)
    {
    _AddViewOverrides(ovrMatSymb); // Modify ovrMatSymb for view flags...

    if (nullptr != m_viewport)
        m_viewport->GetViewControllerR()._OverrideGraphicParams(ovrMatSymb, source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_CookGeometryParams(GeometryParamsR geomParams, GraphicParamsR graphicParams)
    {
    geomParams.Resolve(*this);
    graphicParams.Cook(geomParams, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::CookGeometryParams(Render::GeometryParamsR geomParams, Render::GraphicBuilderR graphic)
    {
    GraphicParams graphicParams;

    _CookGeometryParams(geomParams, graphicParams);
    graphic.ActivateGraphicParams(graphicParams, &geomParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      02/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::IsUndisplayed(GeometrySourceCR source)
    {
    return (!_WantUndisplayed() && source.IsUndisplayed());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::_VisitGeometry(GeometrySourceCR source)
    {
    if (_CheckStop())
        return ERROR;

    return IsUndisplayed(source) ? ERROR : _OutputGeometry(source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::_VisitHit(HitDetailCR hit)
    {
    DgnElementCPtr   element = hit.GetElement();
    GeometrySourceCP source = (element.IsValid() ? element->ToGeometrySource() : nullptr);

    if (nullptr == source)
        {
        IElemTopologyCP elemTopo = hit.GetElemTopology();
        if (nullptr == (source = (nullptr != elemTopo ? elemTopo->_ToGeometrySource() : nullptr)))
            return ERROR;
        }

    if (&GetDgnDb() != &source->GetSourceDgnDb())
        return ERROR;

    if (element.IsValid() && nullptr != m_viewport && !m_viewport->GetViewController().IsModelViewed(element->GetModelId()))
        return ERROR;

    // Allow sub-class involvement for flashing sub-entities...
    Render::GraphicPtr graphic = (nullptr != m_viewport ? m_viewport->GetViewControllerR()._StrokeHit(*this, *source, hit) : source->StrokeHit(*this, hit));

    if (WasAborted()) // if we aborted, the graphic may not be complete
        return ERROR;

    if (graphic.IsValid())
        {
        _OutputGraphic(*graphic, source); 
        return SUCCESS;
        }

    return VisitGeometry(*source);
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
    return context.VisitGeometry(*geomElement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_SetScanReturn()
    {
    m_scanCriteria.SetRangeNodeCheck(this);
    m_scanCriteria.SetElementCallback(visitElementFunc, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
ScanCriteria::Result ViewContext::_CheckNodeRange(ScanCriteriaCR scanCriteria, DRange3dCR testRange, bool is3d)
    {
    Frustum box(testRange);
    return (m_frustumPlanes.Contains(box.m_pts, 8) != FrustumPlanes::Contained::Outside) ? ScanCriteria::Result::Pass : ScanCriteria::Result::Fail;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::_AnyPointVisible(DPoint3dCP worldPoints, int nPts, double tolerance)
    {
    if (m_volume.IsValid())
        {
        int nOutside = 0;

        for (int iPt=0; iPt < nPts; iPt++)
            if (!m_volume->PointInside(worldPoints[iPt], tolerance))
                nOutside++;

        if (nOutside == nPts)
            return false;
        }

    return (FrustumPlanes::Contained::Outside != m_frustumPlanes.Contains(worldPoints, nPts, tolerance));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::IsRangeVisible(DRange3dCR range, double tolerance)
    {
    Frustum box(range);

    return _AnyPointVisible(box.m_pts, 8, tolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::IsPointVisible(DPoint3dCR worldPoint, WantBoresite boresite, double tolerance)
    {
    if (WantBoresite::No == boresite)
        return _AnyPointVisible(&worldPoint, 1, tolerance);

    if (m_frustumPlanes.ContainsPoint(worldPoint, tolerance))
        return true;

    DVec3d worldZVec;
    if (IsCameraOn())
        {
        worldZVec.NormalizedDifference(worldPoint, m_viewport->GetCamera().GetEyePoint());              
        }
    else
        {
        DPoint3d    zPoints[2];
        zPoints[0].Zero();
        zPoints[1].Init(0.0, 0.0, 1.0);
        NpcToWorld(zPoints, zPoints, 2);
        worldZVec.NormalizedDifference(zPoints[1], zPoints[0]);
        }

    return m_frustumPlanes.IntersectsRay(worldPoint, worldZVec);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::_ScanDgnModel(DgnModelP model)
    {
    if (!ValidateScanRange())
        return ERROR;

    m_scanCriteria.SetDgnModel(model);
    return m_scanCriteria.Scan();
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
bool ViewContext::VisitAllViewElements(BSIRectCP updateRect)
    {
    ClearAborted();

    if (nullptr != updateRect)
        SetSubRectFromViewRect(updateRect);

    _InitScanRangeAndPolyhedron();

    _VisitAllModelElements();

    return WasAborted();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::_VisitAllModelElements()
    {
    SetScanReturn();

    // The ViewController must orchestrate the display of all of the elements in the view.
    m_viewport->GetViewControllerR().DrawView(*this);

    return WasAborted();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawStyledLineString2d(int nPts, DPoint2dCP pts, double priority, DPoint2dCP range, bool closed)
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
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
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawStyledArc2d(DEllipse3dCR ellipse, bool isEllipse, double zDepth, DPoint2dCP range)
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
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
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawStyledBSplineCurve2d(MSBsplineCurveCR bcurve, double zDepth)
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
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
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_AddTextString(TextStringCR text)
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    text.GetGlyphSymbology(GetCurrentGeometryParams());
    CookGeometryParams();

    double zDepth = GetCurrentGeometryParams().GetNetDisplayPriority();
    GetCurrentGraphicR().AddTextString(text, Is3dView() ? nullptr : &zDepth);                
    text.DrawTextAdornments(*this);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/03
+---------------+---------------+---------------+---------------+---------------+------*/
double ViewContext::GetPixelSizeAtPoint(DPoint3dCP inPoint) const
    {
    DPoint3d    vec[2];

    if (nullptr != inPoint)
        {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
        LocalToView(vec, inPoint, 1); // convert point to pixels
#else
        WorldToView(vec, inPoint, 1); // convert point to pixels
#endif
        }
    else
        {
        DPoint3d    center = {.5, .5, .5};   // if they didn't give a point, use center of view.
        NpcToView(vec, &center, 1);
        }

    vec[1] = vec[0];
    vec[1].x += 1.0;

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    // Convert pixels back to local coordinates and use the length as tolerance
    ViewToLocal(vec, vec, 2);
#else
    // Convert pixels back to world coordinates and use the length as tolerance
    ViewToWorld(vec, vec, 2);
#endif

    return vec[0].Distance(vec[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicParams::GraphicParams()
    {
    Init();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/02
+---------------+---------------+---------------+---------------+---------------+------*/
void GraphicParams::Init()
    {
    m_isFilled          = false;
    m_isBlankingRegion  = false;
    m_linePixels        = (uint32_t) LinePixels::Solid;
    m_rasterWidth       = 1;
    m_lineColor         = ColorDef::Black();
    m_fillColor         = ColorDef::Black();
    m_trueWidthStart    = 0.0;
    m_trueWidthEnd      = 0.0;
    m_lineTexture       = nullptr;
    m_material          = nullptr;
    m_gradient          = nullptr;
    m_patternParams     = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void GraphicParams::Cook(GeometryParamsCR elParams, ViewContextR context)
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

    if (nullptr != elParams.GetLineStyle())
        {
        LineStyleSymbCR lsSymb = elParams.GetLineStyle()->GetLineStyleSymb();

        if (nullptr != lsSymb.GetILineStyle())
            {
            if (lsSymb.UseLinePixels())
                SetLinePixels((LinePixels)lsSymb.GetLinePixels());
            else if (lsSymb.IsContinuous()) // NOTE: QVis can handle this case for 2d and 3d...
                {
                m_trueWidthStart = (lsSymb.HasOrgWidth() ? lsSymb.GetOriginWidth() : lsSymb.GetEndWidth());
                m_trueWidthEnd = (lsSymb.HasEndWidth() ? lsSymb.GetEndWidth() : lsSymb.GetOriginWidth());
                }
            else
                {
                m_lineTexture = lsSymb.GetTexture(); // For 2d do we need to check that this wasn't a forced texture???
                if (m_lineTexture.IsValid())
                    {
                    m_trueWidthStart = (lsSymb.HasOrgWidth() ? lsSymb.GetOriginWidth() : lsSymb.GetEndWidth());
                    m_trueWidthEnd = (lsSymb.HasEndWidth() ? lsSymb.GetEndWidth() : lsSymb.GetOriginWidth());
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* compare two GraphicParams's to see if they're the same.
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool GraphicParams::operator==(GraphicParamsCR rhs) const
    {
    if (this == &rhs)
        return true;

    if (rhs.m_isFilled         != m_isFilled ||
        rhs.m_isBlankingRegion != m_isBlankingRegion ||
        rhs.m_linePixels       != m_linePixels ||
        rhs.m_rasterWidth      != m_rasterWidth ||
        rhs.m_lineColor        != m_lineColor ||
        rhs.m_fillColor        != m_fillColor ||
        rhs.m_trueWidthStart   != m_trueWidthStart ||
        rhs.m_trueWidthEnd     != m_trueWidthEnd ||
        rhs.m_lineTexture      != m_lineTexture ||
        rhs.m_material         != m_material)
        return false;

    if (!(rhs.m_gradient == m_gradient))
        return false;

    if (!(rhs.m_patternParams == m_patternParams))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater  08/13
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicParams::GraphicParams(GraphicParamsCR rhs)
    {
    m_isFilled          = rhs.m_isFilled;
    m_isBlankingRegion  = rhs.m_isBlankingRegion;
    m_linePixels        = rhs.m_linePixels;
    m_rasterWidth       = rhs.m_rasterWidth;
    m_lineColor         = rhs.m_lineColor;
    m_fillColor         = rhs.m_fillColor;
    m_trueWidthStart    = rhs.m_trueWidthStart;
    m_trueWidthEnd      = rhs.m_trueWidthEnd;
    m_lineTexture       = rhs.m_lineTexture;
    m_material          = rhs.m_material;
    m_gradient          = rhs.m_gradient;
    m_patternParams     = rhs.m_patternParams;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater  08/13
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicParamsR GraphicParams::operator=(GraphicParamsCR rhs)
    {
    m_isFilled          = rhs.m_isFilled;
    m_isBlankingRegion  = rhs.m_isBlankingRegion;
    m_linePixels        = rhs.m_linePixels;
    m_rasterWidth       = rhs.m_rasterWidth;
    m_lineColor         = rhs.m_lineColor;
    m_fillColor         = rhs.m_fillColor;
    m_trueWidthStart    = rhs.m_trueWidthStart;
    m_trueWidthEnd      = rhs.m_trueWidthEnd;
    m_lineTexture       = rhs.m_lineTexture;
    m_material          = rhs.m_material;
    m_gradient          = rhs.m_gradient;
    m_patternParams     = rhs.m_patternParams;

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater  08/13
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryParams::GeometryParams(GeometryParamsCR rhs)
    {
    m_appearanceOverrides   = rhs.m_appearanceOverrides;
    m_resolved              = rhs.m_resolved;
    m_categoryId            = rhs.m_categoryId;
    m_subCategoryId         = rhs.m_subCategoryId;
    m_materialId            = rhs.m_materialId;
    m_elmPriority           = rhs.m_elmPriority;
    m_netPriority           = rhs.m_netPriority;
    m_weight                = rhs.m_weight;       
    m_lineColor             = rhs.m_lineColor;
    m_fillColor             = rhs.m_fillColor;
    m_fillDisplay           = rhs.m_fillDisplay;
    m_elmTransparency       = rhs.m_elmTransparency;
    m_netElmTransparency    = rhs.m_netElmTransparency;
    m_fillTransparency      = rhs.m_fillTransparency;
    m_netFillTransparency   = rhs.m_netFillTransparency;
    m_geometryClass         = rhs.m_geometryClass;       
    m_styleInfo             = rhs.m_styleInfo;
    m_gradient              = rhs.m_gradient;
    m_pattern               = rhs.m_pattern;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater  08/13
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryParamsR GeometryParams::operator=(GeometryParamsCR rhs)
    {
    m_appearanceOverrides   = rhs.m_appearanceOverrides;
    m_resolved              = rhs.m_resolved;
    m_categoryId            = rhs.m_categoryId;
    m_subCategoryId         = rhs.m_subCategoryId;
    m_materialId            = rhs.m_materialId;
    m_elmPriority           = rhs.m_elmPriority;
    m_netPriority           = rhs.m_netPriority;
    m_weight                = rhs.m_weight;
    m_lineColor             = rhs.m_lineColor;
    m_fillColor             = rhs.m_fillColor;
    m_fillDisplay           = rhs.m_fillDisplay;
    m_elmTransparency       = rhs.m_elmTransparency;
    m_netElmTransparency    = rhs.m_netElmTransparency;
    m_fillTransparency      = rhs.m_fillTransparency;
    m_netFillTransparency   = rhs.m_netFillTransparency;
    m_geometryClass         = rhs.m_geometryClass;       
    m_styleInfo             = rhs.m_styleInfo;
    m_gradient              = rhs.m_gradient;
    m_pattern               = rhs.m_pattern;

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryParams::GeometryParams()
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
void GeometryParams::ResetAppearance()
    {
    AutoRestore<DgnCategoryId> saveCategory(&m_categoryId);
    AutoRestore<DgnSubCategoryId> saveSubCategory(&m_subCategoryId);

    *this = GeometryParams();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryParams::operator==(GeometryParamsCR rhs) const
    {
    if (this == &rhs)
        return true;

    if (rhs.m_resolved != m_resolved)
        return false;

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
* @bsimethod                                                    Brien.Bastings  12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryParams::Resolve(DgnDbR dgnDb, DgnViewportP vp)
    {
    if (m_resolved)
        return; // Already resolved...

    BeAssert(m_subCategoryId.IsValid());
    if (!m_subCategoryId.IsValid())
        return;

    // Setup from SubCategory appearance...
    DgnSubCategory::Appearance appearance;

    if (nullptr != vp)
        {
        appearance = vp->GetViewController().GetSubCategoryAppearance(m_subCategoryId);
        }
    else
        {
        DgnSubCategoryCPtr subCat = DgnSubCategory::QuerySubCategory(m_subCategoryId, dgnDb);
        BeAssert(subCat.IsValid());
        if (!subCat.IsValid())
            return;

        appearance = subCat->GetAppearance();
        }

    if (!m_appearanceOverrides.m_color)
        m_lineColor = appearance.GetColor();

    if (m_appearanceOverrides.m_bgFill)
        {
        m_fillColor = (nullptr != vp ? vp->GetBackgroundColor() : ColorDef::Black());

        // NOTE: Problem with white-on-white reversal...don't want this to apply to the background fill... :(
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
    if (nullptr != vp && vp->Is3dView())
        m_netPriority = 0;
    else
        m_netPriority = m_elmPriority + appearance.GetDisplayPriority();

    m_resolved = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryParams::Resolve(ViewContextR context)
    {
    Resolve(context.GetDgnDb(), context.GetViewport());

    if (m_styleInfo.IsValid() && nullptr == m_styleInfo->GetLineStyleSymb().GetILineStyle())
        m_styleInfo->Cook(context, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DecorateContext::AddWorldDecoration(Render::GraphicR graphic, Render::OvrGraphicParamsCP ovrParams)
    {
    if (!m_decorations.m_world.IsValid())
        m_decorations.m_world = new GraphicList;

    m_decorations.m_world->Add(graphic, m_target.ResolveOverrides(ovrParams), ovrParams ? ovrParams->GetFlags() : 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DecorateContext::AddWorldOverlay(Render::GraphicR graphic, Render::OvrGraphicParamsCP ovrParams)
    {
    if (!m_decorations.m_worldOverlay.IsValid())
        m_decorations.m_worldOverlay = new GraphicList;

    m_decorations.m_worldOverlay->Add(graphic, m_target.ResolveOverrides(ovrParams), ovrParams ? ovrParams->GetFlags() : 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DecorateContext::AddViewOverlay(Render::GraphicR graphic, Render::OvrGraphicParamsCP ovrParams)
    {
    if (!m_decorations.m_viewOverlay.IsValid())
        m_decorations.m_viewOverlay = new GraphicList;

    m_decorations.m_viewOverlay->Add(graphic, m_target.ResolveOverrides(ovrParams), ovrParams ? ovrParams->GetFlags() : 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DecorateContext::AddFlashed(Render::GraphicR graphic, Render::OvrGraphicParamsCP ovrParams)
    {
    if (!m_decorations.m_flashed.IsValid())
        m_decorations.m_flashed = new GraphicList;

    m_decorations.m_flashed->Add(graphic, m_target.ResolveOverrides(ovrParams), ovrParams ? ovrParams->GetFlags() : 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DecorateContext::AddSprite(ISprite& sprite, DPoint3dCR location, DPoint3dCR xVec, int transparency)
    {
    AddViewOverlay(*m_target.CreateSprite(sprite, location, xVec, transparency), nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
enum
    {
    MAX_GridDotsInRow       = 500,
    GRID_DOT_Transparency   = 110,
    GRID_LINE_Transparency  = 190,
    GRID_PLANE_Transparency = 225,
    MAX_GridPoints          = 90,
    MAX_GridRefs            = 40,
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/04
+---------------+---------------+---------------+---------------+---------------+------*/
static int getGridPlaneViewIntersections(DPoint3dP intersections, DPoint3dCP planePoint, DPoint3dCP planeNormal, DgnViewportCR vp)
    {
    DRange3d range = vp.GetViewController().GetViewedExtents(); // Limit grid to project extents...

    if (range.IsEmpty())
        return 0;

    static int const index[12][2] = {
                        {NPC_000, NPC_001},     // lines connecting front to back
                        {NPC_100, NPC_101},
                        {NPC_010, NPC_011},
                        {NPC_110, NPC_111},

                        {NPC_000, NPC_100},     // around front face
                        {NPC_100, NPC_110},
                        {NPC_110, NPC_010},
                        {NPC_010, NPC_000},

                        {NPC_001, NPC_101},     // around back face.
                        {NPC_101, NPC_111},
                        {NPC_111, NPC_011},
                        {NPC_011, NPC_001}
                        };

    Frustum frust = vp.GetFrustum(DgnCoordSystem::World, true);

    range.IntersectionOf(range, frust.ToRange());
    range.Get8Corners(frust.m_pts);

    int nIntersections = 0;

    for (int i=0; i<12; i++)
        {
        double param;

        if (bsiGeom_linePlaneIntersection(&param, intersections+nIntersections, &frust.GetCorner(index[i][0]), &frust.GetCorner(index[i][1]), planePoint, planeNormal) && param >= 0.0 && param <= 1.0)
            ++nIntersections;
        }

    return nIntersections;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/04
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getGridDimension(int& nRepetitions, double& min, DPoint3dCR org, DVec3dCR dir, double gridSize, DPoint3dCP points, int nPoints)
    {
    // initialized only to avoid warning.
    double distLow=0.0, distHigh=0.0;

    for (int i=0; i<nPoints; i++)
        {
        double  distance = points[i].DotDifference(org, dir);

        if (i)
            {
            if (distance < distLow)
                distLow = distance;

            if (distance > distHigh)
                distHigh = distance;
            }
        else
            {
            distLow = distHigh = distance;
            }
        }

    if (distHigh <= distLow)
        return false;

    min = ceil(distLow / gridSize); // NOTE: Don't let grid extend outside project extents or display can be clipped...
    double max = floor(distHigh / gridSize);
    nRepetitions = (int)(max - min);
    min *= gridSize;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/04
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawGridRefs(Render::GraphicBuilderR graphic, DPoint3dCR org, DVec3dCR rowVec, DVec3dCR colVec, int rowRepetitions, int colRepetitions)
    {
    DPoint3d gridEnd;

    gridEnd.SumOf(org,colVec, colRepetitions);

    for (double d=0.0; d <= rowRepetitions; d += 1.0)
        {
        DPoint3d linePoints[2];

        linePoints[0].SumOf(org,rowVec, d);
        linePoints[1].SumOf(gridEnd,rowVec, d);
        graphic.AddLineString(2, linePoints);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/04
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getClipPlaneIntersection(double& pMin, double& pMax, DPoint3dCR origin, DPoint3dCR direction, ClipPlane const* pPlane)
    {
    pMin = -FLT_MAX;
    pMax =  FLT_MAX;

    for (int i=0; i<6; i++, pPlane++)
        {
        double vD = pPlane->DotProduct(direction);
        double vN = pPlane->EvaluatePoint(origin);
        double testValue;

        if (vD > 0.0)
            {
            if ((testValue = -vN/vD) > pMin)
                pMin = testValue;
            }
        else if (vD < 0.0)
            {
            if ((testValue = -vN/vD) < pMax)
                pMax = testValue;
            }
        }

    return pMin < pMax;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/04
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawGridDots(Render::GraphicBuilderR graphic, bool doIsoGrid, DPoint3dCR origin, DVec3d const& rowVec, int rowRepetitions, DVec3d const& colVec, int colRepetitions, int refSpacing)
    {
    static double s_maxHorizonGrids = 800.0;
    DgnViewportCR vp = *graphic.GetViewport();
    DVec3d colNormal, rowNormal;
    double colSpacing = colNormal.Normalize(colVec);
    rowNormal.Normalize(rowVec);

    DPoint3d points[MAX_GridDotsInRow];
    bool     cameraOn = vp.IsCameraOn();
    double   zCamera = 0.0, zCameraLimit = 0.0;
    DVec3d   viewZ;

    if (cameraOn)
        {
        CameraInfo const& camera = vp.GetCamera();
        double sizeLimit = (s_maxHorizonGrids * colSpacing) / vp.GetViewDelta()->x;

        vp.GetRotMatrix().GetRow(viewZ, 2);
        zCamera = viewZ.DotProduct(*((DVec3d *) &camera.GetEyePoint()));
        zCameraLimit = zCamera - camera.GetFocusDistance() * sizeLimit;
        }

    Frustum corners = vp.GetFrustum(DgnCoordSystem::World, true);
    ClipPlane clipPlanes[6];
    ClipUtil::RangePlanesFromFrustum(clipPlanes, corners, true, true, false);

    double minClipDistance, maxClipDistance;
    for (int i=0; i<rowRepetitions; i++)
        {
        if (0 != refSpacing && 0 == (i % refSpacing))
            continue;

        DPoint3d dotOrigin;
        dotOrigin.SumOf(origin,rowVec, (double) i);

        if (getClipPlaneIntersection(minClipDistance, maxClipDistance, dotOrigin, colNormal, clipPlanes))
            {
            if (cameraOn)
                {
                DPoint3d        startPoint, endPoint;

                startPoint.SumOf(dotOrigin,colNormal, minClipDistance);
                endPoint.SumOf(dotOrigin,colNormal, maxClipDistance);
                if (viewZ.DotProduct(startPoint) < zCameraLimit && viewZ.DotProduct(endPoint) < zCameraLimit)
                    continue;
                }

            int nToDisplay = 0;
            int jMin = (int) floor(minClipDistance/ colSpacing);
            int jMax = (int) ceil(maxClipDistance / colSpacing);

            // Choose values that result in the least amount of dots between jMin-jMax and 0-colRepetitions...
            jMin = (jMin < 0 ? 0 : jMin);
            jMax = (jMax > colRepetitions ? colRepetitions : jMax);

            double isoOffset = (doIsoGrid && (i&1)) ?  0.5 : 0.0;
            for (int j=jMin; j <= jMax && nToDisplay < MAX_GridDotsInRow; j++)
                {
                if (0 != refSpacing && 0 == (j % refSpacing))
                    continue;

                DPoint3d point;
                point.SumOf(dotOrigin,colVec, (double) j + isoOffset);

                if (cameraOn)
                    {
                    double pointZ = viewZ.DotProduct(point);

                    if (pointZ < zCamera && pointZ >zCameraLimit)
                        points[nToDisplay++] = point;
                    }
                else
                    {
                    points[nToDisplay++] = point;
                    }
                }

            if (0 != nToDisplay)
                graphic.AddPointString(nToDisplay, points);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/05
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawGridPlane(Render::GraphicBuilderR graphic, DPoint3dCR gridOrigin, DVec3dCR xVec, DVec3dCR yVec, Point2dCR repetitions)
    {
    DgnViewportCR vp = *graphic.GetViewport();
    DVec3d viewZ;
    vp.GetRotMatrix().GetRow(viewZ, 2);

    // don't draw grid plane if perpendicular to view
    if (viewZ.IsPerpendicularTo(xVec))
        return;

    // grid refs or points will give visual indication or grid plane...
    DPoint3d shapePoints[5];

    shapePoints[0] = shapePoints[4] = gridOrigin;
    shapePoints[1].SumOf(gridOrigin,xVec, repetitions.x);
    shapePoints[2].SumOf(gridOrigin,xVec, repetitions.x, yVec, repetitions.y);
    shapePoints[3].SumOf(gridOrigin,yVec, repetitions.y);

    graphic.AddShape(5, shapePoints, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/04
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawGrid(Render::GraphicBuilderR graphic, bool doIsoGrid, bool drawDots, DPoint3dCR gridOrigin, DVec3dCR xVec, DVec3dCR yVec, uint32_t gridsPerRef, Point2d const& repetitions)
    {
    DgnViewportCR vp = *graphic.GetViewport();
    ColorDef color = vp.GetContrastToBackgroundColor();
    ColorDef lineColor = vp.MakeColorTransparency(color, GRID_LINE_Transparency);
    ColorDef dotColor = vp.MakeColorTransparency(color, GRID_DOT_Transparency);
    ColorDef planeColor = vp.MakeColorTransparency(color, GRID_PLANE_Transparency);
    GraphicParams::LinePixels linePat = GraphicParams::LinePixels::Solid;
    DVec3d zVec, viewZ;

    zVec.NormalizedCrossProduct(xVec, yVec);
    vp.GetRotMatrix().GetRow(viewZ, 2);

    if (viewZ.DotProduct(zVec) < 0.0) // Provide visual indication that grid is being viewed from the back (grid z not towards eye)...
        {
        planeColor = vp.MakeColorTransparency(ColorDef::Red(), GRID_PLANE_Transparency);
        linePat = GraphicParams::LinePixels::Code2;
        }

    int    gpr = (gridsPerRef>0) ? gridsPerRef : 1;
    double rpg = (1.0 / gpr);

    if (doIsoGrid)
        gridsPerRef = 0; // turn off reference grid for iso

    if (drawDots)
        {
        DVec3d  dotXVec = xVec;
        DVec3d  dotYVec = yVec;

        dotXVec.Scale(rpg);
        dotYVec.Scale(rpg);

        graphic.SetSymbology(dotColor, planeColor, 1);
        drawGridDots(graphic, doIsoGrid, gridOrigin, dotYVec, repetitions.y*gpr, dotXVec, repetitions.x*gpr, gridsPerRef);
        }

    if (0 < gridsPerRef)
        {
        graphic.SetSymbology(lineColor, planeColor, 1, linePat);
        drawGridRefs(graphic, gridOrigin, xVec, yVec, repetitions.x, repetitions.y);
        drawGridRefs(graphic, gridOrigin, yVec, xVec, repetitions.y, repetitions.x);
        }

    if (RenderMode::Wireframe == vp.GetViewFlags().GetRenderMode())
        return;

    graphic.SetBlankingFill(planeColor);
    drawGridPlane(graphic, gridOrigin, xVec, yVec, repetitions);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DecorateContext::DrawStandardGrid(DPoint3dR gridOrigin, RotMatrixR rMatrix, DPoint2d spacing, uint32_t gridsPerRef, bool isoGrid, Point2dCP fixedRepetitions)
    {
    DgnViewportCR vp = *GetViewport();
    DVec3d xVec, yVec, zVec, viewZ;

    rMatrix.GetRows(xVec, yVec, zVec);
    vp.GetRotMatrix().GetRow(viewZ, 2);

    if (!vp.IsCameraOn())
        {
        static const double s_minDotProduct = .005;

        if (fabs(viewZ.DotProduct(zVec)) < s_minDotProduct) // Is grid parallel to view?
            return;
        }

    double   refScale = (0 == gridsPerRef) ? 1.0 : (double) gridsPerRef;
    Point2d  repetitions;
    DPoint3d gridOrg;

    if (NULL == fixedRepetitions) // Compute grid origin and visible repetitions when not drawing a fixed sized grid...
        {
        DPoint3d intersections[12];
        int nIntersections = getGridPlaneViewIntersections(intersections, &gridOrigin, &zVec, vp);

        if (nIntersections < 3)
            return;

        DPoint2d min;

        if (!getGridDimension(repetitions.x, min.x, gridOrigin, xVec, spacing.x, intersections, nIntersections) ||
            !getGridDimension(repetitions.y, min.y, gridOrigin, yVec, spacing.y, intersections, nIntersections))
            return;

        gridOrg.SumOf(gridOrigin, xVec, min.x, yVec, min.y);
        }
    else
        {
        gridOrg = gridOrigin;
        repetitions = *fixedRepetitions;
        }

    if (0 == repetitions.x || 0 == repetitions.y)
        return;

    DVec3d gridX, gridY;
    gridX.Scale(xVec, spacing.x);
    gridY.Scale(yVec, spacing.y);

    DPoint3d testPt;
    testPt.SumOf(gridOrg,gridX, repetitions.x/2.0, gridY, repetitions.y/2.0);

    int maxGridPts  = MAX_GridPoints;
    int maxGridRefs = MAX_GridRefs;

    if (maxGridPts < 10)
        maxGridPts = 10;
    if (maxGridRefs < 10)
        maxGridRefs = 10;

    // values are "per 1000 pixels"
    double minGridSeperationPixels = 1000. / maxGridPts;
    double minRefSeperation = 1000. / maxGridRefs;
    double uorPerPixel = vp.GetPixelSizeAtPoint(&testPt); // center of view

    if ((spacing.x/uorPerPixel) < minRefSeperation || (spacing.y/uorPerPixel) < minRefSeperation)
        gridsPerRef = 0;

    // Avoid z fighting with coincident geometry...let the wookie win...
    gridOrg.SumOf(gridOrg, viewZ, uorPerPixel);
    uorPerPixel *= refScale;

    bool drawDots = ((spacing.x/uorPerPixel) > minGridSeperationPixels) &&((spacing.y/uorPerPixel) > minGridSeperationPixels);
    Render::GraphicBuilderPtr graphic = CreateGraphic(Graphic::CreateParams(&vp));

    drawGrid(*graphic, isoGrid, drawDots, gridOrg, gridX, gridY, gridsPerRef, repetitions);
    AddWorldDecoration(*graphic);
    }
