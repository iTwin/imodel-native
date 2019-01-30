/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ViewContext.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
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
    DgnElementCPtr el = allowLoad ? pool.GetElement(elementId) : pool.FindLoadedElement(elementId);
    if (!el.IsValid())
        return ERROR;

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
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::WorldToNpc(DPoint3dP npcPts, DPoint3dCP worldPts, int nPts) const
    {
    m_worldToNpc.M0.MultiplyAndRenormalize(npcPts, worldPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::_InitContextForView()
    {
    BeAssert(nullptr != m_viewport);
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
        SetCategoryTest(vp->GetViewController().GetViewedCategories());
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
    m_monochromeColor = m_viewport->GetViewControllerR().GetViewDefinitionR().GetDisplayStyle().GetMonochromeColor();

    return _InitContextForView();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::IsCameraOn() const
    {
    BeAssert(nullptr != m_viewport);
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

        t4dPt.GetProjectedXYZ(tPt);

        (viewPts+i)->x = (long) tPt.x;
        (viewPts+i)->y = (long) tPt.y;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::WorldToView(DPoint2dP viewPts, DPoint3dCP worldPts, int nPts) const
    {
    DPoint3d  tPt;
    DPoint4d  t4dPt;

    for (int i=0; i<nPts; i++)
        {
        WorldToView(&t4dPt, worldPts+i, 1);

        t4dPt.GetProjectedXYZ(tPt);

        (viewPts+i)->x = tPt.x;
        (viewPts+i)->y = tPt.y;
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
    RangeIndex::FBox scanRange(polyhedron.ToRange());

    if (!Is3dView())
        {
        scanRange.m_low.z = -1;
        scanRange.m_high.z = 1;
        }

    SetRangeTest(scanRange);

    // if we're doing a skew scan, get the skew parameters
    if (Is3dView())
        {
        DRange3d skewRange;

        // get bounding range of front plane of polyhedron
        skewRange.InitFrom(polyhedron.GetPts(), 4);

        // get unit bvector from front plane to back plane
        DVec3d  skewVec = DVec3d::FromStartEndNormalize(polyhedron.GetCorner(0), polyhedron.GetCorner(4));

        // check to see if it's worthwhile using skew scan (skew bvector not along one of the three major axes */
        int alongAxes = (fabs(skewVec.x) < 1e-8);
        alongAxes += (fabs(skewVec.y) < 1e-8);
        alongAxes += (fabs(skewVec.z) < 1e-8);

        if (alongAxes < 2)
            SetSkewRangeTest(scanRange, RangeIndex::FBox(skewRange), skewVec);
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

    double pixelSize = 0.0;
    if (nullptr != m_viewport)
        {
        DPoint3d origin = source.CalculateRange3d().GetCenter();
        pixelSize = m_viewport->GetPixelSizeAtPoint(&origin);
        }

    Render::GraphicPtr graphic = _StrokeGeometry(source, pixelSize);
    if (!graphic.IsValid())
        return ERROR;

    _OutputGraphic(*graphic, &source);

#if defined(NEEDSWORK_BRIEN)
    static int s_drawRange; // 0 - Host Setting (Bounding Box Debug), 1 - Bounding Box, 2 - Element Range
    if (!s_drawRange)
        return SUCCESS;

    // Output element local range for debug display and locate...
    if (nullptr == GetIPickGeom())
        return SUCCESS;

    auto rangeGraphic = CreateWorldGraphic(2 == s_drawRange ? Transform::FromIdentity() : source.GetPlacementTransform());
    Render::GeometryParams rangeParams;

    rangeParams.SetCategoryId(source.GetCategoryId()); // Need category for pick...
    rangeParams.SetLineColor(DgnViewport::MakeColorTransparency(m_viewport->AdjustColorForContrast(ColorDef::LightGrey(), m_viewport->GetBackgroundColor()), 0x64));
    CookGeometryParams(rangeParams, *rangeGraphic);

    if (nullptr != source.GetAsGeometrySource3d())
        {
        BoundingBox3d range = (2 == s_drawRange ? BoundingBox3d(source.CalculateRange3d()) : BoundingBox3d(source.GetAsGeometrySource3d()->GetPlacement().GetElementBox()));

        rangeGraphic->AddRangeBox(range);
        }
    else
        {
        BoundingBox3d range = (2 == s_drawRange ? BoundingBox3d(source.CalculateRange3d()) : BoundingBox3d(source.GetAsGeometrySource2d()->GetPlacement().GetElementBox()));

        rangeGraphic->AddRangeBox2d(DRange2d::From(DPoint2d::From(range.low), DPoint2d::From(range.high)), 0.0);
        }

    _OutputGraphic(*rangeGraphic->Finish(), &source);
#endif
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_AddSubGraphic(Render::GraphicBuilderR graphic, DgnGeometryPartId partId, TransformCR subToGraphic, Render::GeometryParamsR geomParams)
    {
    DgnGeometryPartCPtr partGeometry = GetDgnDb().Elements().Get<DgnGeometryPart>(partId);

    if (!partGeometry.IsValid())
        return;

    if (m_viewport)
        {
        Transform partToWorld = Transform::FromProduct(graphic.GetLocalToWorldTransform(), subToGraphic);
        ElementAlignedBox3d range = partGeometry->GetBoundingBox();

        partToWorld.Multiply(range, range);

        if (!IsRangeVisible(range))
            return; // Part range doesn't overlap pick...
        }

    GeometryStreamIO::Collection collection(partGeometry->GetGeometryStream().GetData(), partGeometry->GetGeometryStream().GetSize());

    auto partBuilder = graphic.CreateSubGraphic(subToGraphic);
    collection.Draw(*partBuilder, *this, geomParams, false, partGeometry.get());
        
    if (WasAborted()) // if we aborted, the graphic may not be complete, don't save it
        return;

    // NOTE: Need to cook GeometryParams to get GraphicParams, but we don't want to activate and bake into our QvElem...
    GraphicParams graphicParams;
    _CookGeometryParams(geomParams, graphicParams);
    graphic.AddSubGraphic(*partBuilder->Finish(), subToGraphic, graphicParams);
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
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
ScanCriteria::Stop ViewContext::_OnRangeElementFound(DgnElementId id)
    {
    auto el = GetDgnDb().Elements().GetElement(id);
    GeometrySourceCP geomElement = el->ToGeometrySource();
    if (nullptr != geomElement)
        _VisitGeometry(*geomElement);

    return WasAborted() ? ScanCriteria::Stop::Yes : ScanCriteria::Stop::No;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
RangeIndex::Traverser::Accept ViewContext::_CheckRangeTreeNode(RangeIndex::FBoxCR testRange, bool is3d) const
    {
    Frustum box(testRange.ToRange3d());
    return (m_frustumPlanes.Contains(box.m_pts, 8) != FrustumPlanes::Contained::Outside) ? RangeIndex::Traverser::Accept::Yes : RangeIndex::Traverser::Accept::No;
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
            {
            if (!m_volume->PointInside(worldPoints[iPt], tolerance))
                ++nOutside;
            }

        if (nOutside == nPts)
            return false;
        }

    return (FrustumPlanes::Contained::Outside != m_frustumPlanes.Contains(worldPoints, nPts, tolerance));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod        Brien.Bastings  05/16
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
StatusInt ViewContext::_ScanDgnModel(GeometricModelR model)
    {
    if (!ValidateScanRange())
        return ERROR;

    SetDgnModel(model);
    return Scan();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::_VisitDgnModel(GeometricModelR model)
    {
    return CheckStop() ? ERROR : _ScanDgnModel(model);
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
    // The ViewController must orchestrate the display of all of the elements in the view.
    BeAssert(nullptr != m_viewport);
    m_viewport->GetViewControllerR().DrawView(*this);

    return WasAborted();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/03
+---------------+---------------+---------------+---------------+---------------+------*/
double ViewContext::_GetPixelSizeAtPoint(DPoint3dCP inPoint) const
    {
    DPoint3d vec[2];

    if (nullptr != inPoint)
        {
        WorldToView(vec, inPoint, 1); // convert point to pixels
        }
    else
        {
        DPoint3d center = {.5, .5, .5};   // if they didn't give a point, use center of view.
        NpcToView(vec, &center, 1);
        }

    vec[1] = vec[0];
    vec[1].x += 1.0;

    // Convert pixels back to world coordinates and use the length as tolerance
    ViewToWorld(vec, vec, 2);
    return vec[0].Distance(vec[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::MaterialPtr ViewContext::_GetMaterial(RenderMaterialId id) const
    {
    auto system = GetRenderSystem();
    return nullptr != system ? system->_GetMaterial(id, GetDgnDb()) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void GraphicParams::Cook(GeometryParamsCR elParams, ViewContextR context)
    {
    Init();

    m_rasterWidth = DgnViewport::GetDefaultIndexedLineWidth(elParams.GetWeight());
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

#if defined(TODO_ELEMENT_TILE)
            // We need to handle GradientSymb::Flags::Outline at render-time...
            if (0 == (m_gradient->GetFlags() & GradientSymb::Flags::Outline) && (FillDisplay::ByView != elParams.GetFillDisplay() || context.GetViewFlags().ShowFill()))
                {
                m_lineColor.SetAlpha(0xff); // Qvis checks for this to disable auto-outline...
                netElemTransparency = 0.0;  // Don't override the fully transparent outline below...
                }
#endif
            }
        else
            {
            m_fillColor = elParams.GetFillColor();

            bool outline = false;
            if (elParams.IsFillColorFromViewBackground(&outline))
                {
                // NOTE: Problem with white-on-white reversal...don't want this to apply to the background fill... :(
                if (ColorDef::White() == m_fillColor)
                    m_fillColor.SetRed(254);

                // Set line color the same as fill color if an outline fill isn't being drawn...
                if (!outline)
                    m_lineColor = m_fillColor;
                }
            }

        m_isFilled = true;
        m_isBlankingRegion = (FillDisplay::Blanking == elParams.GetFillDisplay());
        }

    if (elParams.GetMaterialId().IsValid())
        {
        m_material = context.GetMaterial(elParams.GetMaterialId());
        m_materialUVDetail = elParams.GetMaterialUVDetail();
        }

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
                {
                SetLinePixels((LinePixels) lsSymb.GetLinePixels());
                }
            else if (lsSymb.IsContinuous() && !lsSymb.GetUseStroker()) // NOTE: QVis can handle this case for 2d and 3d...
                {
                m_trueWidthStart = (lsSymb.HasOrgWidth() ? lsSymb.GetOriginWidth() : lsSymb.GetEndWidth());
                m_trueWidthEnd = (lsSymb.HasEndWidth() ? lsSymb.GetEndWidth() : lsSymb.GetOriginWidth());
                }
            else
                {
                double       textureWidth = 0.0;
                ILineStyleCP currLStyle = lsSymb.GetILineStyle();

                if (nullptr != currLStyle)
                    m_lineTexture = (const_cast<ILineStyleP> (currLStyle))->_GetTexture(textureWidth, context, elParams, false);

                if (m_lineTexture.IsValid())
                    {
                    textureWidth *= lsSymb.GetScale();
                    m_trueWidthStart = textureWidth;
                    m_trueWidthEnd = textureWidth;
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

    if (!rhs.m_materialUVDetail.IsEquivalent(m_materialUVDetail))
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
    m_materialUVDetail  = rhs.m_materialUVDetail;
    m_gradient          = rhs.m_gradient;
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
    m_materialUVDetail  = rhs.m_materialUVDetail;
    m_gradient          = rhs.m_gradient;

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
    m_materialUVDetail      = rhs.m_materialUVDetail;
    m_elmPriority           = rhs.m_elmPriority;
    m_netPriority           = rhs.m_netPriority;
    m_weight                = rhs.m_weight;       
    m_lineColor             = rhs.m_lineColor;
    m_fillColor             = rhs.m_fillColor;
    m_fillDisplay           = rhs.m_fillDisplay;
    m_backgroundFill        = rhs.m_backgroundFill;
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
    m_materialUVDetail      = rhs.m_materialUVDetail;
    m_elmPriority           = rhs.m_elmPriority;
    m_netPriority           = rhs.m_netPriority;
    m_weight                = rhs.m_weight;
    m_lineColor             = rhs.m_lineColor;
    m_fillColor             = rhs.m_fillColor;
    m_fillDisplay           = rhs.m_fillDisplay;
    m_backgroundFill        = rhs.m_backgroundFill;
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
bool GeometryParams::IsEquivalent(GeometryParamsCR other) const
    {
    if (this == &other)
        return true;

    if (m_categoryId != other.m_categoryId)
        return false;

    if (m_subCategoryId != other.m_subCategoryId)
        return false;

    if (m_geometryClass != other.m_geometryClass)
        return false;

    // Don't compare m_netPriority, compare the inputs: m_elmPriority + m_subCateogoryId.
    if (m_elmPriority != other.m_elmPriority)
        return false;

    // Don't compare m_netElmTransparency, compare the inputs: m_elmTransparency + m_subCateogoryId.
    if (m_elmTransparency != other.m_elmTransparency)
        return false;

    // Don't compare m_netFillTransparency, compare the inputs: m_fillTransparency + m_subCateogoryId.
    if (m_fillTransparency != other.m_fillTransparency)
        return false;

    // Don't compare m_lineColor unless sub-category appearance override is set...
    if (m_appearanceOverrides.m_color != other.m_appearanceOverrides.m_color)
        return false;

    if (m_appearanceOverrides.m_color && (m_lineColor != other.m_lineColor))
        return false;

    // Don't compare m_weight unless sub-category appearance override is set...
    if (m_appearanceOverrides.m_weight != other.m_appearanceOverrides.m_weight)
        return false;

    if (m_appearanceOverrides.m_weight && (m_weight != other.m_weight))
        return false;

    // Don't compare m_materialId unless sub-category appearance override is set...
    if (m_appearanceOverrides.m_material != other.m_appearanceOverrides.m_material)
        return false;


    if (m_appearanceOverrides.m_material && (m_materialId != other.m_materialId))
        return false;

    if (!m_materialUVDetail.IsEquivalent(other.m_materialUVDetail))
        return false;

    // Don't compare m_styleInfo unless sub-category appearance override is set...
    if (m_appearanceOverrides.m_style != other.m_appearanceOverrides.m_style)
        return false;

    if (m_appearanceOverrides.m_style)
        {
        if (m_styleInfo.IsValid() != other.m_styleInfo.IsValid())
            return false;

        if (m_styleInfo.IsValid())
            {
            // NOTE: Don't use == operator on LineStyleInfo, don't want to compare LineStyleSymb since it will differ between resolved/un-resolved params...
            if (m_styleInfo->GetStyleId() != other.m_styleInfo->GetStyleId())
                return false;

            LineStyleParamsCP thisParams = m_styleInfo->GetStyleParams();
            LineStyleParamsCP otherParams = other.m_styleInfo->GetStyleParams();

            if ((nullptr == thisParams) != (nullptr == otherParams))
                return false;

            if ((nullptr != thisParams) && !(*thisParams == *otherParams))
                return false;
            }
        }

    if (m_fillDisplay != other.m_fillDisplay)
        return false;

    if (FillDisplay::Never != m_fillDisplay)
        {
        if (m_appearanceOverrides.m_fill != other.m_appearanceOverrides.m_fill)
            return false;

        if (m_gradient.IsValid() != other.m_gradient.IsValid())
            return false;

        if (m_gradient.IsValid() && !(*m_gradient == *other.m_gradient))
            return false;

        // Don't compare m_fillColor unless sub-category appearance override is set...
        if (m_appearanceOverrides.m_fill)
            {
            if (m_backgroundFill != other.m_backgroundFill)
                return false;

            if (BackgroundFill::None == m_backgroundFill && m_fillColor != other.m_fillColor)
                return false;
            }
        }

    if (m_pattern.IsValid() != other.m_pattern.IsValid())
        return false;

    if (m_pattern.IsValid() && !(*m_pattern == *other.m_pattern))
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
        DgnSubCategoryCPtr subCat = DgnSubCategory::Get(dgnDb, m_subCategoryId);
        BeAssert(subCat.IsValid());
        if (!subCat.IsValid())
            return;

        appearance = subCat->GetAppearance();
        }

    if (!m_appearanceOverrides.m_color)
        m_lineColor = appearance.GetColor();

    if (!m_appearanceOverrides.m_fill)
        m_fillColor = appearance.GetColor();
    else if (BackgroundFill::None != m_backgroundFill)
        m_fillColor = (nullptr != vp ? vp->GetBackgroundColor() : ColorDef::Black());

    if (!m_appearanceOverrides.m_weight)
        m_weight = appearance.GetWeight();

    if (!m_appearanceOverrides.m_style)
        m_styleInfo = LineStyleInfo::Create(appearance.GetStyle(), nullptr).get(); // WIP_LINESTYLE - Need LineStyleParams...

    if (!m_appearanceOverrides.m_material)
        m_materialId = appearance.GetRenderMaterial();

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

    if (m_styleInfo.IsValid() && nullptr == m_styleInfo->GetLineStyleSymb().GetILineStyle())
        m_styleInfo->Resolve(dgnDb);

    m_resolved = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryParams::Resolve(ViewContextR context)
    {
    Resolve(context.GetDgnDb(), context.GetViewport());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryParams::ApplyTransform(TransformCR transform, uint32_t options)
    {
    if (m_pattern.IsValid())
        m_pattern->ApplyTransform(transform, options);

    if (m_styleInfo.IsValid())
        m_styleInfo->GetStyleParamsR().ApplyTransform(transform, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DecorateContext::AddWorldDecoration(Render::GraphicR graphic, Render::OvrGraphicParamsCP ovrParams)
    {
    if (!m_decorations.m_world.IsValid())
        m_decorations.m_world = new DecorationList();

    m_decorations.m_world->Add(graphic, ovrParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DecorateContext::SetViewBackground(Render::GraphicR graphic)
    {
    m_decorations.m_viewBackground = &graphic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DecorateContext::AddWorldOverlay(Render::GraphicR graphic, Render::OvrGraphicParamsCP ovrParams)
    {
    if (!m_decorations.m_worldOverlay.IsValid())
        m_decorations.m_worldOverlay = new DecorationList();

    m_decorations.m_worldOverlay->Add(graphic, ovrParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DecorateContext::AddViewOverlay(Render::GraphicR graphic, Render::OvrGraphicParamsCP ovrParams)
    {
    if (!m_decorations.m_viewOverlay.IsValid())
        m_decorations.m_viewOverlay = new DecorationList();

    m_decorations.m_viewOverlay->Add(graphic, ovrParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DecorateContext::AddNormal(Render::GraphicR graphic)
    {
    if (nullptr != m_viewlet)
        {
        m_viewlet->Add(graphic);
        return;
        }

    if (m_decorations.m_normal.IsNull())
        m_decorations.m_normal = new GraphicList();

    m_decorations.m_normal->Add(graphic);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DecorateContext::AddSprite(ISprite& sprite, DPoint3dCR location, DPoint3dCR xVec, int transparency)
    {
    AddViewOverlay(*m_target.CreateSprite(sprite, location, xVec, transparency, GetDgnDb()), nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* show a hit that comes from a view attachment on a sheet.
* @bsimethod                                    Keith.Bentley                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DecorateContext::DrawSheetHit(HitDetailCR hit)
    {
#if defined(TODO_ETT_SHEETS)
    auto& attachVp = *hit.GetSheetAttachViewport();

    // first, create a branch to hold the graphics of the hit using tileVp
    Render::GraphicBranch branch;
    branch.SetViewFlagsOverrides(Render::ViewFlagsOverrides(attachVp.GetViewFlags()));

    m_viewlet = &branch;  // See AddFlashed
    m_viewport = &attachVp; // fake this context to be attached to tileVp
    T_Super::_VisitHit(hit); // and visit the hit normally
    m_viewport = &hit.GetViewport(); // restore m_viewport and m_viewlet so call to _OutputGraphics below will add the branch to the flashed list
    m_viewlet = nullptr;

    if (attachVp.GetViewController().IsSpatialView()) // for 3d views, we need to use viewlets.
        {
        // first get transform from sheet view coordinates to tile view coordinates
        Transform sheetViewToTileView = attachVp.GetTransformFromSheet(*m_viewport);

        // All this is due to the fact that the QV viewlet api doesn't allow rotated viewlets. We therefore have to rotate the frustum of the tileVp to match
        // the orientation of the sheet, in case the sheet view is rotated.
        Render::Plan plan(attachVp);
        plan.m_frustum = GetViewport()->GetFrustum(DgnCoordSystem::View).TransformBy(sheetViewToTileView);      // get corners of sheet view in attach view coordinates
        attachVp.ViewToWorld(plan.m_frustum.m_pts, plan.m_frustum.m_pts, NPC_CORNER_COUNT);    // and create a frustum from those 8 points in world coordinates of attachment

        Frustum sheetFrust = GetFrustum();
        DPoint3d center = sheetFrust.GetCenter();
        center.z = attachVp.m_biasDistance;
        double width  = sheetFrust.Distance(NPC_LeftBottomRear, NPC_RightBottomRear);
        double height = sheetFrust.Distance(NPC_LeftBottomRear, NPC_LeftTopRear);

        Render::ViewletPosition pos(center, width, height, attachVp.GetAttachClips());
        _OutputGraphic(*m_viewport->GetRenderTarget()->GetSystem()._CreateViewlet(branch, plan, pos), nullptr); // put the branch into a "viewlet" and output it
        }
    else
        {
        Transform toNpc;
        toNpc.InitFrom(*attachVp.GetWorldToNpcMap(), false);
        Transform sheetToAttach = Transform::FromProduct(attachVp.m_toParent, toNpc);
        sheetToAttach.form3d[2][3] = attachVp.m_biasDistance;

        auto drawBranch = CreateBranch(branch, GetDgnDb(), sheetToAttach, attachVp.GetAttachClips());
        _OutputGraphic(*drawBranch, nullptr);
        }
#endif

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DecorateContext::DrawNormalHit(HitDetailCR hit)
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
    if (SUCCESS == (nullptr != m_viewport ? m_viewport->GetViewControllerR()._StrokeHit(*this, *source, hit) : source->StrokeHit(*this, hit)))
        return SUCCESS;

    if (element.IsValid())
        {
        m_viewport->SetFlashed(hit.GetElementId(), 0.25);
        return SUCCESS;
        }

    // For locatable transient, can't use SetFlashed to hilite the entire GeometryStream, create a graphic and draw in hilite color instead...
    Render::GraphicPtr graphic = source->Draw(*this, 0.0);

    if (!graphic.IsValid())
        return ERROR;

    hit.FlashGraphic(*graphic, *this);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  06/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DecorateContext::DrawHit(HitDetailCR hit)
    {
    auto sheetVp = hit.GetSheetAttachViewport();
    if (sheetVp && &hit.GetViewport() == GetViewport())
        return DrawSheetHit(hit);

    return DrawNormalHit(hit);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
enum
{
    MAX_GridDotsInRow       = 500,
    GRID_DOT_Transparency   = 100,
    GRID_LINE_Transparency  = 200,
    GRID_PLANE_Transparency = 225,
    MAX_GridPoints          = 90,
    MAX_GridRefs            = 40,
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/04
+---------------+---------------+---------------+---------------+---------------+------*/
static int getGridPlaneViewIntersections(DPoint3dP intersections, DPoint3dCP planePoint, DPoint3dCP planeNormal, DgnViewportCR vp, bool useProjectExtents)
    {
    bool        limitRange = useProjectExtents && vp.GetViewController().IsSpatialView();
    DRange3d    range;

    // Limit non-view aligned grid to project extents in spatial views...
    if (limitRange)
        {
        range = vp.GetViewController().GetDgnDb().GeoLocation().GetProjectExtents(); 

        if (range.IsEmpty())
            return 0;
        }

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

    Frustum frust = vp.GetFrustum();

    if (limitRange)
        range.IntersectionOf(range, frust.ToRange());
    else
        range = frust.ToRange();

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

    min = floor(distLow / gridSize); // NOTE: Should be ok to let grid extend outside project extents since view extends padded for ground plane...
    double max = ceil(distHigh / gridSize);
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
static void drawGridDots(Render::GraphicBuilderR graphic, bool doIsoGrid, DPoint3dCR origin, DVec3d const& rowVec, int rowRepetitions, DVec3d const& colVec, int colRepetitions, int refSpacing, DgnViewportCR vp)
    {
    static double s_maxHorizonGrids = 800.0;
    DVec3d colNormal, rowNormal;
    double colSpacing = colNormal.Normalize(colVec);
    rowNormal.Normalize(rowVec);

    DPoint3d points[MAX_GridDotsInRow];
    bool     cameraOn = vp.IsCameraOn();
    double   zCamera = 0.0, zCameraLimit = 0.0;
    DVec3d   viewZ;

    if (cameraOn)
        {
        auto const& camera = vp.GetCamera();
        double sizeLimit = (s_maxHorizonGrids * colSpacing) / vp.GetViewDelta()->x;

        vp.GetRotMatrix().GetRow(viewZ, 2);
        zCamera = viewZ.DotProduct(*((DVec3dCP) &camera.GetEyePoint()));
        zCameraLimit = zCamera - camera.GetFocusDistance() * sizeLimit;
        }

    Frustum corners = vp.GetFrustum();
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
* @bsimethod                                                    RayBentley      05/04
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawGrid(Render::GraphicBuilderR graphic, bool doIsoGrid, bool drawDots, DPoint3dCR gridOrigin, DVec3dCR xVec, DVec3dCR yVec, uint32_t gridsPerRef, Point2d const& repetitions, DgnViewportCR vp)
    {
    double        aa;
    DVec3d        zVec, viewZ;
    DPoint4d      eyePoint;

    vp.GetWorldToViewMap()->M1.GetColumn(eyePoint, 2);
    viewZ.Init(eyePoint.x, eyePoint.y, eyePoint.z);

    if (DoubleOps::SafeDivide(aa, 1.0, eyePoint.w, 1.0))
        {
        DPoint3d  xyzEye;

        xyzEye.Scale(viewZ, aa);
        viewZ.DifferenceOf(xyzEye, gridOrigin);
        }

    viewZ.Normalize();
    zVec.NormalizedCrossProduct(xVec, yVec);

    ColorDef color = vp.GetContrastToBackgroundColor();
    ColorDef lineColor = vp.MakeColorTransparency(color, GRID_LINE_Transparency);
    ColorDef dotColor = vp.MakeColorTransparency(color, GRID_DOT_Transparency);
    ColorDef planeColor = vp.MakeColorTransparency(color, GRID_PLANE_Transparency);
    LinePixels linePat = LinePixels::Solid;

    if (viewZ.DotProduct(zVec) < 0.0) // Provide visual indication that grid is being viewed from the back (grid z not towards eye)...
        {
        planeColor = vp.MakeColorTransparency(ColorDef::Red(), GRID_PLANE_Transparency);
        linePat = LinePixels::Code2;
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
        drawGridDots(graphic, doIsoGrid, gridOrigin, dotYVec, repetitions.y*gpr, dotXVec, repetitions.x*gpr, gridsPerRef, vp);
        }

    if (0 < gridsPerRef)
        {
        graphic.SetSymbology(lineColor, planeColor, 1, linePat);
        drawGridRefs(graphic, gridOrigin, xVec, yVec, repetitions.x, repetitions.y);
        drawGridRefs(graphic, gridOrigin, yVec, xVec, repetitions.y, repetitions.x);
        }

    // don't draw grid plane if perpendicular to view
    if (viewZ.IsPerpendicularTo(xVec))
        return;

    // grid refs or points will give visual indication or grid plane...
    DPoint3d shapePoints[5];

    shapePoints[0] = shapePoints[4] = gridOrigin;
    shapePoints[1].SumOf(gridOrigin, xVec, repetitions.x);
    shapePoints[2].SumOf(gridOrigin, xVec, repetitions.x, yVec, repetitions.y);
    shapePoints[3].SumOf(gridOrigin, yVec, repetitions.y);

    if (0 == gridsPerRef)
        {
        graphic.SetSymbology(lineColor, planeColor, 1, linePat);
        graphic.AddLineString(5, shapePoints);
        }

    graphic.SetBlankingFill(planeColor);
    graphic.AddShape(5, shapePoints, true);
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
    DPoint2d refSpacing = DPoint2d::FromScale(spacing, refScale);
    DPoint3d gridOrg;
    Point2d  repetitions;

    // Compute grid origin and visible repetitions when fixed sized grid size isn't supplied...
    if (nullptr == fixedRepetitions || 0 == fixedRepetitions->x || 0 == fixedRepetitions->y)
        {
        DPoint3d intersections[12];
        int nIntersections = getGridPlaneViewIntersections(intersections, &gridOrigin, &zVec, vp, nullptr != fixedRepetitions);

        if (nIntersections < 3)
            return;

        DPoint2d min;

        if (!getGridDimension(repetitions.x, min.x, gridOrigin, xVec, refSpacing.x, intersections, nIntersections) ||
            !getGridDimension(repetitions.y, min.y, gridOrigin, yVec, refSpacing.y, intersections, nIntersections))
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
    gridX.Scale(xVec, refSpacing.x);
    gridY.Scale(yVec, refSpacing.y);

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

    if ((refSpacing.x/uorPerPixel) < minRefSeperation || (refSpacing.y/uorPerPixel) < minRefSeperation)
        gridsPerRef = 0;

    // Avoid z fighting with coincident geometry...let the wookie win...
    gridOrg.SumOf(gridOrg, viewZ, uorPerPixel);
    uorPerPixel *= refScale;

    bool drawDots = ((refSpacing.x/uorPerPixel) > minGridSeperationPixels) &&((refSpacing.y/uorPerPixel) > minGridSeperationPixels);
    auto graphic = CreateWorldDecoration();

    drawGrid(*graphic, isoGrid, drawDots, gridOrg, gridX, gridY, gridsPerRef, repetitions, vp);
    AddWorldDecoration(*graphic->Finish());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::TexturePtr ViewContext::_CreateTexture(Render::ImageCR image) const
    {
    Render::TexturePtr tx;
    auto sys = GetRenderSystem();
    if (nullptr != sys)
        tx = sys->_CreateTexture(image, GetDgnDb());

    return tx;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::TexturePtr ViewContext::_CreateTexture(Render::ImageSourceCR source, Render::Image::BottomUp bottomUp) const
    {
    Render::TexturePtr tx;
    auto sys = GetRenderSystem();
    if (nullptr != sys)
        tx = sys->_CreateTexture(source, bottomUp, GetDgnDb());

    return tx;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::TargetP ViewContext::_GetRenderTarget() const
    {
    auto vp = GetViewport();
    return nullptr != vp ? vp->GetRenderTarget() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::SystemP ViewContext::_GetRenderSystem() const
    {
    auto target = GetRenderTarget();
    return nullptr != target ? &target->GetSystem() : nullptr;
    }

