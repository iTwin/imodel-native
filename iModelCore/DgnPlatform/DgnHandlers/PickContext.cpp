/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/PickContext.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

enum 
    {
    MAX_BUFFER_VERTS            = 2048,
    MESHPICK_CheckStopPeriod    = 100,
    CLOUDPICK_CheckStopPeriod   = 1000,
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
static double distSquaredXY (DPoint4dCR pVec1, DPoint4dCR pVec2)
    {
    DPoint3d    v1, v2;

    pVec1.GetProjectedXYZ (v1);
    pVec2.GetProjectedXYZ (v2);

    double dx = v1.x - v2.x;
    double dy = v1.y - v2.y;

    return   dx * dx + dy * dy;
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void PickOutput::Init(PickContext* context)
    {
    _SetDrawViewFlags(context->GetViewport()->GetViewFlags());

    m_currGeomDetail.Init();
    // can't set up Pick point in screen coordinates until after we've attached to the view
    SetViewContext(context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void PickOutput::_SetDrawViewFlags(ViewFlags flags)
    {
    T_Super::_SetDrawViewFlags(flags);

    // NOTE: Treat LocateSurfacesPref::Never as LocateSurfacesPref::ByView, still want to hide edge hits that aren't visible, will truncate returned list to remove surfaces...
    if ((LocateSurfacesPref::Always == m_options.GetLocateSurfaces()) && (DgnRenderMode::Wireframe == m_viewFlags.GetRenderMode()))
        m_viewFlags.SetRenderMode(DgnRenderMode::SmoothShade);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
bool PickContext::_IsSnap() const
    {
    return m_options.GetHitSource() == HitSource::AccuSnap || m_options.GetHitSource() == HitSource::TentativeSnap;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static bool edgesVisible(HitDetailCP hit)
    {
    ViewFlags viewFlags = hit->GetViewport().GetViewFlags();

    switch (viewFlags.GetRenderMode())
        {
        case DgnRenderMode::SolidFill:
        case DgnRenderMode::HiddenLine:
            return viewFlags.hiddenEdges;

        case DgnRenderMode::SmoothShade:
            return viewFlags.visibleEdges && viewFlags.hiddenEdges;

        default:
            return true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static double getAdjustedViewZ (ViewContextR context, DPoint4dCR viewPt)
    {
    DPoint3d    viewPt3d;

    viewPt.GetProjectedXYZ (viewPt3d);
    return viewPt3d.z;
    }

/*---------------------------------------------------------------------------------**//**
* After a hit has been found, by first checking the PreLocate filter and then
* doing a proximity check, it is added to the list of Hits by calling this method.
* This method calls the PostLocate filter, and then (presuming the hit passes) adds
* to the actual list. It then trims the list to the maximum hits and determines that
* if the list is full and contains only hits within a certain tolerance, that there
* is no point in continuing, since no improvement can be made. If so, it sets the
* "doneSearching" flag.
* @see          HitList
* @bsimethod                                                    KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
void PickContext::_AddHit(DPoint4dCR hitPtScreen, DPoint3dCP hitPtLocal, HitPriority priority)
    {
    // NOTE: Only reason to have ElemTopology for non-element hit is to allow locate/snap...
    if (nullptr == m_currentGeomSource && nullptr == GetElemTopology())
        return;

    DPoint3d    localPt;

    if (hitPtLocal)
        localPt = *hitPtLocal;
    else
        ViewToLocal(&localPt, &hitPtScreen, 1);

    // if the point is not visible in the current view, skip this hit (skip when drawing base geom after getting lstyle hit!)
    if (!(TEST_LSTYLE_BaseGeom == m_testingLStyle && m_unusableLStyleHit))
        {
        if (!IsLocalPointVisible(localPt, false))
            return;
        }

    // Use override priority if it's been set...
    if (HitPriority::Highest != m_hitPriorityOverride)
        priority = m_hitPriorityOverride;

    switch (m_testingLStyle)
        {
        case TEST_LSTYLE_Component:
            {
            bool    isLocate = (HitSource::DataPoint == m_options.GetHitSource() || HitSource::MotionLocate == m_options.GetHitSource());

            // NOTE: If lstyle not snappable or not snapping...set flag to ignore proximity test for base geom.
            if (!m_currGeomDetail.IsSnappable() || isLocate)
                {
                m_unusableLStyleHit = true;
                return;
                }

            // NOTE: Preserve relative priority of lstyle hits...but make sure base geom priority is "better"...
            priority = (HitPriority) (static_cast<int>(priority) + static_cast<int>(HitPriority::Interior));
            break;
            }

        case TEST_LSTYLE_BaseGeom:
        case TEST_LSTYLE_None:
            {
            // NOTE: Clear lstyle component detail info setup in GetCurrLineStyle.
            if (HitDetailSource::None != (HitDetailSource::LineStyle & m_currGeomDetail.GetDetailSource()))
                {
                m_currGeomDetail.SetDetailSource(HitDetailSource::LineStyle & ~m_currGeomDetail.GetDetailSource());
                m_currGeomDetail.SetNonSnappable(false);
                }
            break;
            }
        }

    DPoint3d    hitPtWorld;
    Transform   localToWorld;

    GetCurrLocalToWorldTrans(localToWorld);
    localToWorld.Multiply(&hitPtWorld, &localPt, 1);

    m_currGeomDetail.SetClosestPoint(hitPtWorld);
    m_currGeomDetail.SetLocatePriority(priority);
    m_currGeomDetail.SetScreenDist(sqrt(distSquaredXY(hitPtScreen, m_pickPointView)));
    m_currGeomDetail.SetZValue(getAdjustedViewZ(*this, hitPtScreen) + GetCurrentDisplayParams().GetNetDisplayPriority());
    m_currGeomDetail.SetGeomStreamEntryId(GetGeomStreamEntryId());

    RefCountedPtr<HitDetail> thisHit = new HitDetail(*GetViewport(), m_currentGeomSource, m_pickPointWorld, m_options.GetHitSource(), m_currGeomDetail);

    if (nullptr != GetElemTopology())
        thisHit->SetElemTopology(GetElemTopology()->_Clone());

    m_hitList->AddHit(thisHit.get(), true, true);

    // if we've got too many, throw away the last one
    if (m_hitList->GetCount() <= (int) m_options.GetMaxHits())
        return;

    // if the distance to the object we're about to throw away was less than a few pixels
    // then there's really no point in continuing (can't do much better than that).
    // The exception to this is in a shaded view when hidden edges aren't displayed, then
    // we want to find the best hits by comparing Z not XY so we have to keep looking...
    HitDetailP lastHit = (HitDetailP) m_hitList->GetHit(-1);
    if ((NULL != lastHit) && edgesVisible(lastHit) && (1.4 >= lastHit->GetGeomDetail().GetScreenDist()))
        m_doneSearching = true;

    m_hitList->RemoveHit(-1);
    }

/*---------------------------------------------------------------------------------**//**
* determine whether a point in world coordinates is inside or outside of the current clipping
* established for this context.
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool PickContext::_IsPointVisible(DPoint3dCP worldPt)
    {
    return IsWorldPointVisible(*worldPt, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool PickContext::PointWithinTolerance(DPoint4dCR testPt)
    {
    return (m_pickApertureSquared >= distSquaredXY (testPt, _GetPickPointView()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/01
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d* PickContext::GetProjectedPickPointView(DPoint3dR point)
    {
    _GetPickPointView().GetProjectedXYZ (point);
    return &point;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
DRay3d PickContext::_GetBoresite() const
    {
    DRay3d      boresite;
    DPoint3d    localPt;
    DMatrix4d   viewToLocal = GetViewToLocal();

    ViewToLocal(&localPt, &_GetPickPointView(), 1);
    InitBoresite(boresite, localPt, viewToLocal);

    return boresite;
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/07
+---------------+---------------+---------------+---------------+---------------+------*/
int PickOutput::LocateQvElemCheckStop(CallbackArgP arg)
    {
    ViewContextP context = (ViewContextP) arg;

    return (context->CheckStop() ? 1 : 0);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/12
+---------------+---------------+---------------+---------------+---------------+------*/
void PickContext::AddSurfaceHit(DPoint3dCR hitPtLocal, DVec3dCR hitNormalLocal, HitPriority priority)
    {
    // User doesn't want surfaces/interiors. Don't add a non-obscuring surface hit...
    if (LocateSurfacesPref::Never == m_options.GetLocateSurfaces() && 0.0 != hitNormalLocal.Magnitude())
        {
        ViewFlags viewFlags = GetViewFlags();

        switch (viewFlags.GetRenderMode())
            {
            case DgnRenderMode::SolidFill:
            case DgnRenderMode::HiddenLine:
                {
                if (!viewFlags.hiddenEdges)
                    break; // Accept, hidden edges are never displayed...

                break;
                }

            case DgnRenderMode::SmoothShade:
                {
                if (!(viewFlags.visibleEdges && viewFlags.hiddenEdges))
                    break; // Accept, hidden edges are never displayed...

                break;
                }
            }
        }

    DVec3d      saveNormal = m_currGeomDetail.GetSurfaceNormal();
    DVec3d      hitNormalWorld = hitNormalLocal;
    DPoint4d    viewPt;
    Transform   localToWorld;

    GetCurrLocalToWorldTrans(localToWorld);
    localToWorld.MultiplyMatrixOnly(hitNormalWorld);
    hitNormalWorld.Normalize();

    m_currGeomDetail.SetGeomType(HitGeomType::Surface);
    m_currGeomDetail.SetSurfaceNormal(hitNormalWorld);

    LocalToView(&viewPt, &hitPtLocal, 1);
    _AddHit(viewPt, &hitPtLocal, priority);

    m_currGeomDetail.SetSurfaceNormal(saveNormal);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool PickContext::TestGraphics(Graphic* qvElem, HitPriority priority)
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    DPoint3d    hitPt, pickPtView;
    DVec3d      hitNormal;

    GetProjectedPickPointView(pickPtView);

    // NOTE: Use smaller than normal pick radius to avoid hitting surface before edge (QV locate is square not a circle!)... 
    bool    haveQvLocate = m_viewOutput->LocateQvElem(qvElem, *((DPoint2dCP) &pickPtView), m_pickAperture * 0.4, hitPt, &hitNormal, LocateQvElemCheckStop, m_context);

    if (!haveQvLocate)
        return false;

    DPoint3d    localPt = hitPt;
    DVec3d      localNormal = hitNormal;
    Transform   worldToLocal;

    if (SUCCESS == m_context->GetCurrWorldToLocalTrans(worldToLocal))
        {
        worldToLocal.Multiply(localPt);

        // NOTE: Wireframe/silhouette hit normal will have 0 magnitude...
        if (0.0 != localNormal.Magnitude())
            {
            worldToLocal.MultiplyMatrixOnly(localNormal);
            localNormal.Normalize();
            }
        }

    AddSurfaceHit(localPt, localNormal, priority);

#endif
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* test a point in Local coordinates against the Pick point. Save hit if close enough.
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool PickContext::TestPoint(DPoint3dCR localPt, HitPriority priority)
    {
    DPoint4d    viewPt;
    LocalToView(&viewPt, &localPt, 1);

    if (!PointWithinTolerance(viewPt))
        return false;

    m_currGeomDetail.SetGeomType(HitGeomType::Point);

    _AddHit(viewPt, &localPt, priority);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool PickContext::TestPointArray(size_t numPts, DPoint3dCP localPts, HitPriority priority)
    {
    bool  hit = false;

    for (size_t i=0; i < numPts; i++)
        {
        DPoint4d    viewPt;

        LocalToView(&viewPt, &localPts[i], 1);

        if (!PointWithinTolerance(viewPt))
            continue;

        m_currGeomDetail.SetGeomType(HitGeomType::Point);

        _AddHit(viewPt, &localPts[i], priority);
        hit = true;
        }

    return hit;
    }

/*---------------------------------------------------------------------------------**//**
@description compute distance from pickPt to a segment identified by indices into
        coordinate array.
@param vertsP IN local coordinate array.
@param hVertsP IN pretransformed homogeneous coordinates.  If NULL, local coordinates are transformed
        as needed.
@param closeVertexId IN index of start vertex
@param segmentVertexId IN index of target vertex
@param pickPt IN pick point, pretransformed to view coordinates.
@bsimethod                                                    EarlinLutz  02/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool PickContext::TestIndexedPolyEdge(DPoint3dCP vertsP, DPoint4dCP hVertsP, int closeVertexId, int segmentVertexId, DPoint3dR pickPt, HitPriority priority)
    {
    DPoint3d    edge[2];

    edge[0] = vertsP[closeVertexId];
    edge[1] = vertsP[segmentVertexId];

    DPoint4d    hEdge[2];
    DPoint4d    hPick;

    bsiDPoint4d_initFromDPoint3dAndWeight(&hPick, &pickPt, 1.0);

    if (hVertsP)
        {
        hEdge[0] = hVertsP[closeVertexId];
        hEdge[1] = hVertsP[segmentVertexId];
        }
    else    // transform source dpoint3d on the spot
        {
        DMatrix4d localToView = GetLocalToView();

        bsiDMatrix4d_multiplyWeightedDPoint3dArray(&localToView, hEdge, edge, NULL, 2);
        }

    if (hEdge[0].w < 0.0 || hEdge[1].w < 0.0)
        return false;

    ProximityData   proximity;

    bsiProximityData_init(&proximity, &pickPt, -1, 0.0);
    bsiProximityData_testXY (&proximity, &hEdge[0], 0.0, 0);
    bsiProximityData_testXY (&proximity, &hEdge[1], 1.0, 0);

    DPoint4d    qPoint[2];
    double      qParam[2];
    int         qCount;

    if (bsiBezierDPoint4d_allPerpendicularsFromDPoint4dExt(qParam, qPoint, &qCount, 2, hEdge, 2, &hPick, 2, false))
        {
        for (int i = 0; i < qCount; i++)
            bsiProximityData_testXY (&proximity, &qPoint[i], qParam[i], 0);
        }

    if (proximity.closeDistanceSquared < m_pickApertureSquared)
        {
        ICurvePrimitivePtr  tmpCurve = ICurvePrimitive::CreateLine(DSegment3d::From(edge[0], edge[1]));
        CurvePrimitiveIdPtr newId = CurvePrimitiveId::Create(CurvePrimitiveId::Type_PolyfaceEdge, CurveTopologyId(CurveTopologyId::Type_PolyfaceEdge, closeVertexId, segmentVertexId), nullptr);

        tmpCurve->SetId(newId.get());
        m_currGeomDetail.SetCurvePrimitive(tmpCurve.get(), GetCurrLocalToWorldTransformCP());
        _AddHit(proximity.closePoint, NULL, priority);

        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool boresiteToCurveVector(CurveVectorCR curves, DRay3dCR boresite, DPoint3dR intersectPt, DVec3dR normal)
    {
    DRange3d        localRange;
    Transform       localToWorld, worldToLocal;
    CurveVectorPtr  curvesLocal = curves.CloneInLocalCoordinates(LOCAL_COORDINATE_SCALE_01RangeBothAxes, localToWorld, worldToLocal, localRange);

    if (!curvesLocal.IsValid())
        return false;

    double      t;
    DPoint3d    uvw;

    if (!boresite.IntersectZPlane(localToWorld, 0.0, uvw, t))
        return false;

    CurveVector::InOutClassification inOut = curvesLocal->PointInOnOutXY (uvw);

    if (CurveVector::INOUT_In != inOut && CurveVector::INOUT_On != inOut)
        return false;

    localToWorld.Multiply(&intersectPt, &uvw, 1);
    localToWorld.GetMatrixColumn(normal, 2);
    normal.Normalize();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool PickContext::TestCurveVectorInterior(CurveVectorCR curves, HitPriority priority)
    {
    DPoint3d    intersectPt;
    DVec3d      normal;

    if (!boresiteToCurveVector(curves, _GetBoresite(), intersectPt, normal))
        return false;

    AddSurfaceHit(intersectPt, normal, priority);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool PickContext::TestCurveVector(CurveVectorCR curves, HitPriority priority)
    {
    DPoint3d    pickPtLocal;
    DPoint4d    pickPtView = _GetPickPointView();
    DMatrix4d   localToView = GetLocalToView();

    ViewToLocal(&pickPtLocal, &pickPtView, 1);

    CurveLocationDetail  location;

    if (!curves.ClosestPointBoundedXY (pickPtLocal, &localToView, location))
        return false;

    DPoint4d    hitPtView;

    LocalToView(&hitPtView, &location.point, 1);

    double      pickDistSquared = distSquaredXY (hitPtView, pickPtView);

    if (!((m_pickApertureSquared > pickDistSquared) || (TEST_LSTYLE_BaseGeom == m_testingLStyle && m_unusableLStyleHit)))
        return false;

    m_currGeomDetail.SetCurvePrimitive(location.curve, GetCurrLocalToWorldTransformCP());
    _AddHit(hitPtView, &location.point, priority);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PickContext::ProcessCurvePrimitive(ICurvePrimitiveCR primitive, bool closed, bool filled)
    {
    switch (primitive.GetCurvePrimitiveType())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            {
            DEllipse3dCP  ellipse = primitive.GetArcCP ();
            DPoint4d      viewPt;

            LocalToView(&viewPt, &ellipse->center, 1);

            if (!PointWithinTolerance(viewPt))
                break;

            // NOTE: Need to set curve to create curve topo associations to arc centers!
            m_currGeomDetail.SetCurvePrimitive(&primitive, GetCurrLocalToWorldTransformCP(), HitGeomType::Point);
            _AddHit(viewPt, &ellipse->center, ellipse->IsFullEllipse() ? HitPriority::Origin : HitPriority::Interior);
            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
            {
            bvector<DPoint3d> const* points = primitive.GetPointStringCP ();

            TestPointArray(points->size(), &points->front(), HitPriority::Vertex);
            break;
            }
        }

    return SUCCESS; // Don't resend curves as strokes...
    }

#define NoisySolidPrimitivePick_not
#ifdef NoisySolidPrimitivePick
static void PrintVector(char *name, DVec3d vector)
    {
    DVec3d unit;
    double d = unit.Normalize(vector);
    if (DoubleOps::AlmostEqual(unit.x, 1.0))
        BeConsole::Printf("     (%hs positive X %g)\n", name, d);
    else if (DoubleOps::AlmostEqual(unit.x, -1.0))
        BeConsole::Printf("     (%hs negative X %g)\n", name, d);
    else if (DoubleOps::AlmostEqual(unit.y,  1.0))
        BeConsole::Printf("     (%hs negative Y %g)\n", name, d);
    else if (DoubleOps::AlmostEqual(unit.y, -1.0))
        BeConsole::Printf("     (%hs negative Y %g)\n", name, d);
    else if (DoubleOps::AlmostEqual(unit.z,  1.0))
        BeConsole::Printf("     (%hs negative Z %g)\n", name, d);
    else if (DoubleOps::AlmostEqual(unit.z, -1.0))
        BeConsole::Printf("     (%hs negative Z %g)\n", name, d);
    else
        BeConsole::Printf("     (%hs (%g,%g,%g) %g)\n", name, unit.x, unit.y, unit.z, d);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isWireframeDisplay(ViewContextR context)
    {
    return (DgnRenderMode::Wireframe == context.GetViewFlags().GetRenderMode());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PickContext::ProcessCurveVector(CurveVectorCR curves, bool isFilled)
    {
    if (!TestCurveVector(curves, HitPriority::Edge) && m_doLocateInteriors)
        {
        // NOTE: Since edge hits are preferred only test interiors when we don't get an edge hit...
        if (curves.IsAnyRegionType() && (isFilled || !isWireframeDisplay(*this)))
            TestCurveVectorInterior(curves, HitPriority::Interior);
        }

    return ERROR; // Process components for arc centers and point strings...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PickContext::ProcessSolidPrimitive(ISolidPrimitiveCR primitive)
    {
    if (isWireframeDisplay(*this))
        {
        m_doLocateSilhouettes = (SolidPrimitiveType_DgnCone != primitive.GetSolidPrimitiveType() && primitive.HasCurvedFaceOrEdge()); // Need QvElem locate for silhouettes in wireframe...

        return ERROR; // Output rules and edges...
        }

    DRay3d      boresite = _GetBoresite();

    bvector<SolidLocationDetail> intersectLocationDetail;

    primitive.AddRayIntersections(intersectLocationDetail, boresite);
    size_t i = 0;
    for (SolidLocationDetail& thisDetail: intersectLocationDetail)
        {
        DVec3d  normal;

        normal.NormalizedCrossProduct(thisDetail.GetUDirection(), thisDetail.GetVDirection());

#ifdef NoisySolidPrimitivePick
        DVec3d uDirection = thisDetail.GetUDirection();
        DVec3d vDirection = thisDetail.GetVDirection();
        BeConsole::Printf("Pick (%d of %d) (rayFraction %g) (face %d %d) (uv %g %g)\n",
              i, intersectLocationDetail.size(),
              thisDetail.GetPickParameter(),
              thisDetail.GetPrimarySelector(), thisDetail.GetSecondarySelector(),
              thisDetail.GetU (), thisDetail.GetV ()
              );
        PrintVector("Uvec", thisDetail.GetUDirection());
        PrintVector("Vvec", thisDetail.GetVDirection());
#endif
        AddSurfaceHit(thisDetail.GetXYZ (), normal, HitPriority::Interior);
        i++;
        }

    return ERROR; // Output rules and edges...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PickContext::ProcessSurface(MSBsplineSurfaceCR surface)
    {
    if (isWireframeDisplay(*this))
        {
        m_doLocateSilhouettes = true; // Need QvElem locate for silhouettes in wireframe...

        return ERROR; // Output uv rules and boundaries...
        }

    DRay3d      boresite = _GetBoresite();

    bvector<DPoint3d> intersectionPoints;
    bvector<double>   rayParameters;
    bvector<DPoint2d> surfaceParameters;

    surface.IntersectRay(intersectionPoints, rayParameters, surfaceParameters, boresite);

#define PrintSurfaceIntersectRay_not
#ifdef PrintSurfaceIntersectRay
    printf(" Hits %d\n", (int)rayParameters.size());
    for (size_t kk = 0; kk < rayParameters.size(); kk++)
        printf("        (lambda %.17g) (uv %.8g %.8g)\n", rayParameters[kk], surfaceParameters[kk].x, surfaceParameters[kk].y);
#endif

    for (size_t iHit = 0; iHit < surfaceParameters.size(); iHit++)
        {
        if (!bsputil_pointOnSurface(&surfaceParameters[iHit], &surface))
            continue;
        
        DVec3d      normal, uDir, vDir, dPdUU, dPdVV, dPdUV;
        DPoint3d    point;

        surface.EvaluateAllPartials(point, uDir, vDir, dPdUU, dPdVV, dPdUV, normal, surfaceParameters[iHit].x, surfaceParameters[iHit].y);
        normal.Normalize();
        AddSurfaceHit(intersectionPoints[iHit], normal, HitPriority::Interior);
        }

    return ERROR; // Output uv rules and boundaries...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PickContext::ProcessFacetSet(PolyfaceQueryCR meshData, bool filled)
    {
    if (!isWireframeDisplay(*this))
        {
        DRay3d              boresite = _GetBoresite();
        PolyfaceVisitorPtr  visitor = PolyfaceVisitor::Attach(meshData);

        for (; visitor->AdvanceToNextFace(); )
            {
            if (CheckStop())
                return ERROR;

            FacetLocationDetail  facetDetail;

            if (!visitor->TryDRay3dIntersectionToFacetLocationDetail(boresite, facetDetail))
                continue;

            DPoint3d  point;

            if (!facetDetail.TryGetPoint(point))
                continue;

            DVec3d  normal = DVec3d::From(0.0, 0.0, 0.0);

            facetDetail.TryGetNormal(normal);
            AddSurfaceHit(point, normal, HitPriority::Interior);
            }
        }
    else
        {
        m_doLocateSilhouettes = true; // Need QvElem locate for silhouettes in wireframe...
        }

    int         polySize = meshData.GetNumPerFace();
    int         numIndices = static_cast<int>(meshData.GetPointIndexCount());
    int const*  vertIndex = meshData.GetPointIndexCP ();

    if (!vertIndex)
        return SUCCESS;

    DPoint3dCP  verts = meshData.GetPointCP ();
    int         thisFaceSize = 0, thisIndex, firstIndex=0, prevIndex=0;
    DPoint3d    pickPt;
    DPoint4d    *hVertBufferP = NULL;       // Will point to _alloca buffer if within allowed size

    GetProjectedPickPointView(pickPt);

    // Try to make pre-transformed vertices ...
    int absVert1;
    int maxVert1 = 0;

    for (int readIndex = 0; readIndex < numIndices; readIndex++)
        {
        if ((absVert1 = abs(vertIndex[readIndex])) > maxVert1)
            maxVert1 = absVert1;
        }

    // The one-based vertex index is also the referenced vertex count ...
    if (maxVert1 <= MAX_BUFFER_VERTS)
        {
        DMatrix4d      localToView = GetLocalToView(); 
        hVertBufferP = (DPoint4d *)_alloca(maxVert1 * sizeof (DPoint4d));

        bsiDMatrix4d_multiplyWeightedDPoint3dArray(&localToView, hVertBufferP, verts, NULL, maxVert1);

        DRange3d range;

        bsiDRange3d_init(&range);
        bsiDRange3d_extendByDPoint4dArray(&range, hVertBufferP, maxVert1);

        double a = sqrt(m_pickApertureSquared);

        if (pickPt.x < range.low.x - a || pickPt.x > range.high.x + a || pickPt.y < range.low.y - a || pickPt.y > range.high.y + a)
            return SUCCESS;
        }

    for (int readIndex = 0; readIndex < numIndices; readIndex++)
        {
        // found face loop entry
        if (thisIndex = vertIndex[readIndex])
            {
            // remember first index in this face loop
            if (!thisFaceSize)
                firstIndex = thisIndex;

            // draw visible edge (prevIndex, thisIndex)
            else if (prevIndex > 0)
                {
                int     closeVertexId = (abs(prevIndex) - 1);
                int     segmentVertexId = (abs(thisIndex) - 1);

                TestIndexedPolyEdge(verts, hVertBufferP, closeVertexId, segmentVertexId, pickPt, HitPriority::Edge);
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
                int     closeVertexId = (abs(prevIndex) - 1);
                int     segmentVertexId = (abs(firstIndex) - 1);

                TestIndexedPolyEdge(verts, hVertBufferP, closeVertexId, segmentVertexId, pickPt, HitPriority::Edge);
                }

            thisFaceSize = 0;
            }

        if ((MESHPICK_CheckStopPeriod <= 1 || (readIndex % MESHPICK_CheckStopPeriod) == 0) && CheckStop())
            return SUCCESS;
        }

    return SUCCESS; // Don't output edges...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PickContext::ProcessBody(ISolidKernelEntityCR)
    {
    // NOTE: We can't be creating/destroying breps every cursor motion. A handler that output breps
    //       must somehow cache their edge geometry for snapping; they can locate surfaces by QvElem.

    BeAssert(false); // Never instantiate an ISolidKernelEntity for locate...too expensive!!!

    return SUCCESS; // Do nothing...
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     08/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool PickOutput::_DrawSprite(ISpriteP sprite, DPoint3dCP location, DPoint3dCP xVec, int transparency)
    {
    Point2d     spriteSize;
    DPoint4d    viewPt;

    sprite->GetSize(&spriteSize); 
    m_context->LocalToView(&viewPt, location, 1);

    double      spriteRadiusSquared = (spriteSize.x * spriteSize.x + spriteSize.y * spriteSize.y) / 4.0;

    if (spriteRadiusSquared < distSquaredXY (viewPt, _GetPickPointView()))
        return false;

    m_currGeomDetail.SetGeomType(HitGeomType::Point);
    m_currGeomDetail.SetDetailSource(HitDetailSource::Sprite | m_currGeomDetail.GetDetailSource());

    _AddHit(viewPt, NULL, HitPriority::Vertex); // Note. Use HIT_PRIOITY_VERTEX so that sprites will always have always supercede segment hits in wireframe.

    m_currGeomDetail.SetDetailSource(HitDetailSource::Sprite & ~m_currGeomDetail.GetDetailSource());

    return true;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
void PickOutput::_AddTextString(TextStringCR text, double* zDepth)
    {
    if (text.GetText().empty())
        return;
    
    DPoint3d    points[5];

    text.ComputeBoundingShape(points);
    text.ComputeTransform().Multiply(points, _countof(points));

    CurveVectorPtr  tmpCurve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer);

    tmpCurve->push_back(ICurvePrimitive::CreateLineString(points, 5));

    if (m_pick.TestCurveVector(*tmpCurve, HitPriority::Edge))
        return;

    DPoint3d    intersectPt;
    DVec3d      normal;

    // Always test for interior hit if we didn't hit origin/edge...
    if (!boresiteToCurveVector(*tmpCurve, m_pick._GetBoresite(), intersectPt, normal))
        return;

    DPoint4d    hitPtView;
                    
    m_context->LocalToView(&hitPtView, &intersectPt, 1);

    // Treat this as a curve hit, need bounding shape for correct snappping behavior (center/bisector/midpoint)...
    m_pick.m_currGeomDetail.SetCurvePrimitive(&(*tmpCurve->front()), m_context->GetCurrLocalToWorldTransformCP());
    m_pick._AddHit(hitPtView, &intersectPt, HitPriority::TextBox);
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/11
+---------------+---------------+---------------+---------------+---------------+------*/
void PickOutput::_AddPointCloud(IPointCloudDrawParams* drawParams)
    {
    m_currGeomDetail.SetGeomType(HitGeomType::Point);
    m_currGeomDetail.SetDetailSource(HitDetailSource::PointCloud | m_currGeomDetail.GetDetailSource());

    uint32_t    nPoints = drawParams->GetNumPoints();
    DPoint3dCP  dPoints = drawParams->GetDPoints();
    FPoint3dCP  fPoints = drawParams->GetFPoints();
    DPoint3d    offset;

    offset.Init(0, 0, 0);
    drawParams->GetOrigin(&offset);
        
    for (uint32_t iPoint = 0; iPoint < nPoints; iPoint++)
        {
        DPoint3d    localPt;
        DPoint4d    viewPt;

        if (NULL != dPoints)
            localPt = dPoints[iPoint];
        else
            localPt.Init(fPoints[iPoint].x + offset.x, fPoints[iPoint].y + offset.y, fPoints[iPoint].z + offset.z);

        m_context->LocalToView(&viewPt, &localPt, 1);

        if (!PointWithinTolerance(viewPt))
            continue;

        _AddHit(viewPt, &localPt, HitPriority::Vertex);

        if (iPoint && 0 == (iPoint % CLOUDPICK_CheckStopPeriod) && m_context->CheckStop())
            break;
        }

    m_currGeomDetail.SetDetailSource(HitDetailSource::PointCloud & ~m_currGeomDetail.GetDetailSource());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/04
+---------------+---------------+---------------+---------------+---------------+------*/
void PickOutput::_DrawGraphic(Graphic* qvElem)
    {
    TestGraphics(qvElem, HitPriority::Interior);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
PickContext::PickContext(LocateOptions const& options, StopLocateTest* stopTester) : m_options(options), m_stopTester(stopTester), m_graphic(*this)
    {
    m_hitList               = NULL;
    m_hitPriorityOverride   = HitPriority::Highest;
    m_unusableLStyleHit     = false;
    m_testingLStyle         = TEST_LSTYLE_None;
    m_doLocateSilhouettes   = false;
    m_doLocateInteriors     = true;
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void PickContext::_SetupOutputs()
    {
    SetIViewDraw(m_output);
    m_output.SetupViewOutput(m_viewport->GetIViewOutput());
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void PickContext::_DrawAreaPattern(ClipStencil& boundary)
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    m_output._GetGeomDetail().SetDetailSource(HitDetailSource::Pattern | m_output._GetGeomDetail().GetDetailSource());
    T_Super::_DrawAreaPattern(boundary);
    m_output._GetGeomDetail().SetDetailSource(HitDetailSource::Pattern & ~m_output._GetGeomDetail().GetDetailSource());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/04
+---------------+---------------+---------------+---------------+---------------+------*/
ILineStyleCP PickContext::_GetCurrLineStyle(LineStyleSymbP* symb)
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    if (m_output.GetInSymbolDraw())
        return NULL;
#endif

    LineStyleSymbP  tSymb;
    ILineStyleCP    style = T_Super::_GetCurrLineStyle(&tSymb);

    if (nullptr == style)
        return nullptr;

    if (symb)
        *symb = tSymb;

    if (style->_GetComponent()->_HasWidth() || tSymb->HasOrgWidth() || tSymb->HasEndWidth())
        {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
        m_output._GetGeomDetail().SetDetailSource(HitDetailSource::LineStyle | m_output._GetGeomDetail().GetDetailSource());
        m_output._GetGeomDetail().SetNonSnappable(!style->_IsSnappable());
#endif

        return style;
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void PickContext::_DrawStyledLineString3d(int nPts, DPoint3dCP pts, DPoint3dCP range, bool closed)
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    if (NULL == _GetCurrLineStyle(NULL))
        {
        if (closed)
            m_IDrawGeom->AddShape(nPts, pts, false, range);
        else
            m_IDrawGeom->AddLineString(nPts, pts, range);
        return;
        }

    // NOTE: When not snapping to lstyle, lstyle hit used to override proximity test of base geom...
    m_output.SetTestLStylePhase(TEST_LSTYLE_Component);

    T_Super::_DrawStyledLineString3d(nPts, pts, range);

    m_output.SetTestLStylePhase(TEST_LSTYLE_BaseGeom);

    if (closed)
        m_IDrawGeom->AddShape(nPts, pts, false, range);
    else
        m_IDrawGeom->AddLineString(nPts, pts, range);

    m_output.SetTestLStylePhase(TEST_LSTYLE_None);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void PickContext::_DrawStyledLineString2d(int nPts, DPoint2dCP pts, double priority, DPoint2dCP range, bool closed)
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    if (NULL == _GetCurrLineStyle(NULL))
        {
        if (closed)
            m_IDrawGeom->AddShape2d(nPts, pts, false, priority, range);
        else
            m_IDrawGeom->AddLineString2d(nPts, pts, priority, range);
        return;
        }

    // NOTE: When not snapping to lstyle, lstyle hit used to override proximity test of base geom...
    m_output.SetTestLStylePhase(TEST_LSTYLE_Component);

    T_Super::_DrawStyledLineString2d(nPts, pts, priority, range);

    m_output.SetTestLStylePhase(TEST_LSTYLE_BaseGeom);

    if (closed)
        m_IDrawGeom->AddShape2d(nPts, pts, false, priority, range);
    else
        m_IDrawGeom->AddLineString2d(nPts, pts, priority, range);

    m_output.SetTestLStylePhase(TEST_LSTYLE_None);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void PickContext::_DrawStyledArc3d(DEllipse3dCR ellipse, bool isEllipse, DPoint3dCP range)
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    if (NULL == _GetCurrLineStyle(NULL))
        {
        m_IDrawGeom->AddArc(ellipse, isEllipse, false, range);
        return;
        }

    // NOTE: When not snapping to lstyle, lstyle hit used to override proximity test of base geom...
    m_output.SetTestLStylePhase(TEST_LSTYLE_Component);

    T_Super::_DrawStyledArc3d(ellipse, isEllipse, range);

    m_output.SetTestLStylePhase(TEST_LSTYLE_BaseGeom);

    m_IDrawGeom->AddArc(ellipse, isEllipse, false, range);

    m_output.SetTestLStylePhase(TEST_LSTYLE_None);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void PickContext::_DrawStyledArc2d(DEllipse3dCR ellipse, bool isEllipse, double zDepth, DPoint2dCP range)
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    if (NULL == _GetCurrLineStyle(NULL))
        {
        m_IDrawGeom->AddArc2d(ellipse, isEllipse, false, zDepth, range);
        return;
        }

    // NOTE: When not snapping to lstyle, lstyle hit used to override proximity test of base geom...
    m_output.SetTestLStylePhase(TEST_LSTYLE_Component);

    T_Super::_DrawStyledArc2d(ellipse, isEllipse, zDepth, range);

    m_output.SetTestLStylePhase(TEST_LSTYLE_BaseGeom);

    m_IDrawGeom->AddArc2d(ellipse, isEllipse, false, zDepth, range);

    m_output.SetTestLStylePhase(TEST_LSTYLE_None);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void PickContext::_DrawStyledBSplineCurve3d(MSBsplineCurveCR curve)
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    if (NULL == _GetCurrLineStyle(NULL))
        {
        m_IDrawGeom->AddBSplineCurve(curve, false);
        return;
        }

    // NOTE: When not snapping to lstyle, lstyle hit used to override proximity test of base geom...
    m_output.SetTestLStylePhase(TEST_LSTYLE_Component);

    T_Super::_DrawStyledBSplineCurve3d(curve);

    m_output.SetTestLStylePhase(TEST_LSTYLE_BaseGeom);

    m_IDrawGeom->AddBSplineCurve(curve, false);

    m_output.SetTestLStylePhase(TEST_LSTYLE_None);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void PickContext::_DrawStyledBSplineCurve2d(MSBsplineCurveCR curve, double zDepth)
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    if (NULL == _GetCurrLineStyle(NULL))
        {
        m_IDrawGeom->AddBSplineCurve2d(curve, false, zDepth);
        return;
        }

    // NOTE: When not snapping to lstyle, lstyle hit used to override proximity test of base geom...
    m_output.SetTestLStylePhase(TEST_LSTYLE_Component);

    T_Super::_DrawStyledBSplineCurve2d(curve, zDepth);

    m_output.SetTestLStylePhase(TEST_LSTYLE_BaseGeom);

    m_IDrawGeom->AddBSplineCurve2d(curve, false, zDepth);

    m_output.SetTestLStylePhase(TEST_LSTYLE_None);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool PickContext::_CheckStop()
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    return (WasAborted() ? true : AddAbortTest(m_output.GetDoneSearching() || (m_stopTester && m_stopTester->_CheckStopLocate())));
#endif
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PickContext::_OnPreDrawTransient()
    {
    T_Super::_OnPreDrawTransient();
    _GetGeomDetail().Init(); // Setup hit detail defaults...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    03/02
+---------------+---------------+---------------+---------------+---------------+------*/
void PickContext::_OutputElement(GeometrySourceCR element)
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    // Setup hit detail defaults...unless this is a symbol, don't want hit detail (pattern/linestyle) cleared...
    if (!m_output.GetInSymbolDraw())
        m_output._GetGeomDetail().Init();
#endif

    // do per-element test
    T_Super::_OutputElement(element);

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    // Reset hit priority override in case it's been set...
    if (!m_output.GetInSymbolDraw())
        m_output._SetHitPriorityOverride(HitPriority::Highest);
#endif
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    03/02
+---------------+---------------+---------------+---------------+---------------+------*/
void PickContext::_DrawCached(aphicStroker& stroker)
    {
    bool    testStroke = stroker._WantLocateByStroker();

    m_output.InitStrokeForCache();

    if (testStroke)
        stroker._Stroke(*this);

    if (CheckStop())
        return ;

    if (testCached || m_output.GetLocateSilhouettes())
        T_Super::_DrawCached(stroker);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void    PickContext::InitNpcSubRect(DPoint3dCR pickPointWorldIn, double pickAperture, DgnViewportR viewport)
    {
    DPoint3d    pickPointView, pickPointWorld = pickPointWorldIn;

    viewport.GetWorldToViewMap()->M0.MultiplyAndRenormalize(&pickPointView, &pickPointWorld, 1);

    m_npcSubRange.low.x  = pickPointView.x - pickAperture;
    m_npcSubRange.high.x = pickPointView.x + pickAperture;

    // Y-direction in View coordinates is reversed
    m_npcSubRange.low.y  = pickPointView.y + pickAperture;
    m_npcSubRange.high.y = pickPointView.y - pickAperture;

    m_npcSubRange.low.z  = pickPointView.z - pickAperture;
    m_npcSubRange.high.z = pickPointView.z + pickAperture;

    ViewToNpc(&m_npcSubRange.low, &m_npcSubRange.low, 2);
    viewport.ViewToNpc(&m_npcSubRange.low, &m_npcSubRange.low, 2);

    m_npcSubRange.low.z  = 0;
    m_npcSubRange.high.z = 1.0;
    
    m_useNpcSubRange = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PickContext::_VisitDgnModel(DgnModelP inDgnModel)
    {
    // Ignore elements that are not from view controller's target project unless tool specifically requests otherwise...
    if (&inDgnModel->GetDgnDb() != &GetDgnDb() && !m_options.GetDisableDgnDbFilter())
        return ERROR;

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    // Make sure the test point is within the clipping region of this file.
    if (m_output._IsPointVisible(&m_output._GetPickPointWorld()))
        return T_Super::_VisitDgnModel(inDgnModel);
#endif

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void PickContext::InitBoresite(DRay3dR boresite, DPoint3dCR spacePoint, DMatrix4dCR worldToLocal)
    {
    boresite.origin = spacePoint;

    DPoint4d    eyePoint;

    worldToLocal.GetColumn(eyePoint, 2);
    boresite.direction.Init(eyePoint.x, eyePoint.y, eyePoint.z);

    double      aa;

    if (DoubleOps::SafeDivide(aa, 1.0, eyePoint.w, 1.0))
        {
        DPoint3d  xyzEye;

        xyzEye.Scale(boresite.direction, aa);
        boresite.direction.DifferenceOf(xyzEye, boresite.origin);
        }

    boresite.direction.Normalize();
    boresite.direction.Negate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
PickOutput::PickOutput(PickContext& pick) : m_pick(pick) 
    {
    SetViewContext(&pick);

    // Avoid doing expensive operation on large facet sets...
    m_defaultFacetOptions->SetNormalsRequired(false);
    m_defaultFacetOptions->SetParamsRequired(false); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void PickContext::InitSearch(DPoint3dCR pickPointWorld, double pickApertureScreen, HitListP hitList)
    {
    m_doneSearching         = false;
    m_hitList               = hitList;
    m_pickPointWorld        = pickPointWorld;
    m_pickAperture          = pickApertureScreen;
    m_pickApertureSquared   = m_pickAperture * m_pickAperture;

    m_hitList->Empty();
    WorldToView(&m_pickPointView, &m_pickPointWorld, 1);

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    m_output.Init(this, pickPointWorld, pickApertureScreen, hitList, m_options);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* search around a point, given a view, for elements that are within the Pick tolerance and
* put the "hits" into a HitList. No filtering of the hits is done by this functions - callers
* can decide which hits are of interest to them. The list is sorted according to "best hit"
* criteria.
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool PickContext::PickElements(DgnViewportR vp, DPoint3dCR pickPointWorld, double pickApertureScreen, HitListP hitList)
    {
    if (!vp.IsActive())
        return SetAborted();

    InitNpcSubRect(pickPointWorld, pickApertureScreen, vp); // Initialize prior to attach so frustum planes are set correctly.

    // attach the context
    if (SUCCESS != Attach(&vp, DrawPurpose::Pick))
        return true;

    InitSearch(pickPointWorld, pickApertureScreen, hitList);
    VisitAllViewElements(true, NULL);
    _Detach();

#if defined (NOT_DUMP)
    printf("HIT LIST COUNT: %d\n", hitList->GetCount());

    for (int iHit = 0; iHit < hitList->GetCount(); ++iHit)
        {
        HitDetailP    thisPath = (HitDetailP) hitList->Get(iHit);
        
        printf("(%d) Elem: %I64d, GeomType: %d Z: %lf\n", iHit, thisPath->GetHeadElem()->GetElementId(), thisPath->GetGeomDetail().GetGeomType(), thisPath->GetGeomDetail().GetZValue());
        }

    printf("\n\n");
#endif

    // User doesn't want surfaces/interiors. Truncate list at first surface encountered (hits have been sorted by z)...
    if (LocateSurfacesPref::Never == m_options.GetLocateSurfaces() && 0 != hitList->GetCount())
        {
        bool    truncateHits = false;

        for (int iHit = 0; iHit < hitList->GetCount(); ++iHit)
            {
            if (truncateHits || ((HitDetailP) hitList->Get(iHit))->GetGeomDetail().IsValidSurfaceHit())
                {
                hitList->Set(iHit, NULL);
                truncateHits = true;
                }
            }

        if (truncateHits)
            hitList->DropNulls();
        }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    return (WasAborted() && !m_output.GetDoneSearching());
#endif
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @return   true if the point is on the path
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
TestHitStatus PickContext::TestHit(HitDetailCR hit, DgnViewportR vp, DPoint3dCR pickPointWorld, double pickApertureScreen, HitListP hitList)
    {
    DgnElementCPtr element = hit.GetElement();

    if (!element.IsValid())
        return TestHitStatus::NotOn;

    GeometrySourceCP source = element->ToGeometrySource();

    if (nullptr == source)
        return TestHitStatus::NotOn;

    InitNpcSubRect(pickPointWorld, pickApertureScreen, vp); // Initialize prior to attach so frustum planes are set correctly.

    if (SUCCESS != Attach(&vp, DrawPurpose::Pick))
        return TestHitStatus::Aborted;

    // Re-test using same hit source as input path (See _AddHit locate behavior for linestyle components)...
    m_options.SetHitSource(HitDetailType::Hit <= hit.GetHitType() ? hit.GetLocateSource() : HitSource::None);

    InitSearch(pickPointWorld, pickApertureScreen, hitList);
    VisitElement(*source);
    _Detach();

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    return WasAborted() ? TestHitStatus::Aborted : ((m_output.GetHitList()->GetCount() > 0) ? TestHitStatus::IsOn : TestHitStatus::NotOn);
#endif
    return TestHitStatus::NotOn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PickOutput::_ProcessCurvePrimitive(ICurvePrimitiveCR prim, bool closed, bool filled) {return m_pick.ProcessCurvePrimitive(prim, closed, filled);}
StatusInt PickOutput::_ProcessCurveVector(CurveVectorCR vector, bool isFilled) {return m_pick.ProcessCurveVector(vector, isFilled);}
StatusInt PickOutput::_ProcessSolidPrimitive(ISolidPrimitiveCR prim) {return m_pick.ProcessSolidPrimitive(prim);}
StatusInt PickOutput::_ProcessSurface(MSBsplineSurfaceCR surface) {return m_pick.ProcessSurface(surface);}
StatusInt PickOutput::_ProcessFacetSet(PolyfaceQueryCR query, bool filled) {return m_pick.ProcessFacetSet(query, filled);}
StatusInt PickOutput::_ProcessBody(ISolidKernelEntityCR entity) {return m_pick.ProcessBody(entity);}
