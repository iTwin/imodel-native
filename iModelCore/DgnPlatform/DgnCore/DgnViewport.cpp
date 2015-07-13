/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnViewport.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>


static  uint32_t s_rasterLinePatterns[8] =
    {
    0xffffffff,     // 0
    0x80808080,     // 1
    0xf8f8f8f8,     // 2
    0xffe0ffe0,     // 3
    0xfe10fe10,     // 4
    0xe0e0e0e0,     // 5
    0xf888f888,     // 6
    0xff18ff18      // 7
    };

static DPoint3d const s_NpcCorners[NPC_CORNER_COUNT] =
    {
    { 0.0, 0.0, 0.0 },  // NPC_000
    { 1.0, 0.0, 0.0 },  // NPC_100
    { 0.0, 1.0, 0.0 },  // NPC_010
    { 1.0, 1.0, 0.0 },  // NPC_110
    { 0.0, 0.0, 1.0 },  // NPC_001
    { 1.0, 0.0, 1.0 },  // NPC_101
    { 0.0, 1.0, 1.0 },  // NPC_011
    { 1.0, 1.0, 1.0 },  // NPC_111
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/02
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewport::DgnViewport()
    {
    m_viewNumber        = -1;
    m_minLOD            = DEFAULT_MINUMUM_LOD;
    m_viewNumber        = -1;
    m_isCameraOn        = false;
    m_needsRefresh      = false;
    m_zClipAdjusted     = false;
    m_is3dView          = false;
    m_isSheetView       = false;
    m_qvDCAssigned      = false;
    m_qvParamsSet       = false;
    m_invertY           = true;
    m_frustumValid      = false;
    m_toolGraphicsHandler = NULL;
    m_backgroundColor   = ColorDef::Black();
    m_output            = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/02
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::DestroyViewport()
    {
    RELEASE_AND_CLEAR (m_output);

    m_progressiveDisplay.clear();
    m_viewController     = NULL;
    m_qvDCAssigned = false;
    m_qvParamsSet  = false;
    m_frustumValid = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::InitViewSettings(bool useBgTexture)
    {
    BeAssert(m_output);

    m_output->SetViewAttributes(*GetViewFlags(), m_backgroundColor, useBgTexture, _WantAntiAliasLines(), _WantAntiAliasText());
    m_qvParamsSet = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/02
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::SetDisplayFlagFill(bool newValue)
    {
    m_rootViewFlags.fill = newValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/02
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::SetDisplayFlagPatterns(bool newValue)
    {
    m_rootViewFlags.patterns = newValue;
    }

enum Constant
    {
    MINIMUM_WINDOW_DEPTH            = -32767,
    MAXIMUM_WINDOW_DEPTH            = 32767,
    };

/*---------------------------------------------------------------------------------**//**
* Get the DgnCoordSystem::View coordinates of lower-left-back and upper-right-front corners of a viewport.
* NOTE: the y values are "swapped" (llb.y is greater than urf.y) on the screen and and "unswapped" when we plot.
* @bsimethod                                                    KeithBentley    04/02
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::_GetViewCorners(DPoint3dR llb, DPoint3dR urf) const
    {
    BSIRect viewRect = GetViewRect();

    llb.x  = viewRect.origin.x;
    llb.z  = MINIMUM_WINDOW_DEPTH;

    urf.x = viewRect.corner.x;
    urf.z = MAXIMUM_WINDOW_DEPTH;

    if (m_invertY)
        {
        // y's are swapped on the screen!
        llb.y = viewRect.corner.y;
        urf.y = viewRect.origin.y;
        }
    else
        {
        llb.y = viewRect.origin.y;
        urf.y = viewRect.corner.y;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/02
+---------------+---------------+---------------+---------------+---------------+------*/
BSIRect DgnViewport::GetViewRect() const
    {
    return _GetClientRect();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::ViewToNpc(DPoint3dP npcVec, DPoint3dCP screenVec, int nPts) const
    {
    DPoint3d        llb, urf;
    _GetViewCorners(llb, urf);

    Transform    scrToNpcTran;
    bsiTransform_initFromRange(NULL, &scrToNpcTran, &llb, &urf);
    scrToNpcTran.Multiply(npcVec, screenVec, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::NpcToView(DPoint3dP screenVec, DPoint3dCP npcVec, int nPts) const
    {
    DPoint3d        llb, urf;
    _GetViewCorners(llb, urf);

    Transform    npcToScrTran;
    bsiTransform_initFromRange(&npcToScrTran, NULL, &llb, &urf);
    npcToScrTran.Multiply(screenVec, npcVec, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::NpcToWorld(DPoint3dP rootPts, DPoint3dCP npcPts, int nPts) const
    {
    m_rootToNpc.M1.MultiplyAndRenormalize(rootPts, npcPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::WorldToNpc(DPoint3dP npcPts, DPoint3dCP rootPts, int nPts) const
    {
    m_rootToNpc.M0.MultiplyAndRenormalize(npcPts, rootPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::ViewToWorld(DPoint3dP rootPts, DPoint4dCP screenPts, int nPts) const
    {
    DPoint4d  tPt;
    for (int i=0; i<nPts; i++)
        {
        bsiDMatrix4d_multiplyMatrixPoint(&m_rootToView.M1, &tPt, screenPts+i);
        tPt.GetProjectedXYZ (rootPts[i]);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/02
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::ViewToScreen(DPoint3dP screenPts, DPoint3dCP viewPts, int nPts) const
    {
    memcpy(screenPts, viewPts, nPts * sizeof(DPoint3d));

    Point2d screenOrg = GetScreenOrigin();
    DPoint3d org;
    org.Init(screenOrg.x, screenOrg.y, 0.0);
    DPoint3d::AddToArray(screenPts, nPts, org);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/02
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::ScreenToView(DPoint3dP viewPts, DPoint3dCP screenPts, int nPts) const
    {
    memcpy(viewPts, screenPts, nPts * sizeof(DPoint3d));
    Point2d screenOrg = GetScreenOrigin();

    DPoint3d org;
    org.Init(screenOrg.x, screenOrg.y, 0.0);

    bsiDPoint3d_subtractDPoint3dArray(viewPts, &org, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::WorldToView(DPoint4dP screenPts, DPoint3dCP rootPts, int nPts) const
    {
    bsiDMatrix4d_multiplyWeightedDPoint3dArray(&m_rootToView.M0, screenPts, rootPts, NULL, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* convert an array of points in Root coordinates into View coordinates
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::WorldToView(DPoint3dP viewPts, DPoint3dCP rootPts, int nPts) const
    {
    bsiDMatrix4d_multiplyAndRenormalizeDPoint3dArray(&m_rootToView.M0, viewPts, rootPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* convert an array of points in Root coordinates into View (2d) coordinates
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::WorldToView2d(DPoint2dP viewPts, DPoint3dCP rootPts, int nPts) const
    {
    DPoint3d    dPt;

    for (int i=0; i<nPts; i++)
        {
        WorldToView(&dPt, rootPts+i, 1);

        viewPts[i].x = dPt.x;
        viewPts[i].y = dPt.y;
        }
    }

/*---------------------------------------------------------------------------------**//**
* convert an array of points in View coordinates into Root coordinates
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::ViewToWorld(DPoint3dP rootPts, DPoint3dCP viewPts, int nPts) const
    {
    bsiDMatrix4d_multiplyAndRenormalizeDPoint3dArray(&m_rootToView.M1, rootPts, viewPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* Ensure the rotation matrix for this view is aligns the root z with the view out (i.e. a "2d view").
* @bsimethod                                    Keith.Bentley                   09/05
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::AlignWithRootZ()
    {
    DVec3d zUp;
    zUp.Init(0.0, 0.0, 1.0);

    DVec3d z;
    m_rotMatrix.GetRow(z, 2);
    if (zUp.IsEqual(z))
        return;

    RotMatrix r;
    r.TransposeOf(m_rotMatrix);
    r.SetColumn(zUp, 2);
    r.SquareAndNormalizeColumns(r, 2, 0);
    m_rotMatrix.TransposeOf(r);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::_AdjustAspectRatio(ViewControllerR viewController, bool expandView)
    {
    BSIRect viewRect = GetClientRect();
    viewController.AdjustAspectRatio(viewRect.Aspect(), expandView);
    }

/*---------------------------------------------------------------------------------**//**
* Get an origin, 3 direction vectors, and a compression fraction defining a view frustum from the view
* definition specified by camera, origin, delta, and rMatrix.
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnViewport::RootToNpcFromViewDef(DMap4dR rootToNpc, double* compressionFraction, CameraInfo const* camera,
                                          DPoint3dCR inOrigin, DPoint3dCR delta, RotMatrixCR viewRot)
    {
    DVec3d    xVector, yVector, zVector;
    viewRot.GetRows(xVector, yVector, zVector);

    DPoint3d    xExtent, yExtent, zExtent;
    DPoint3d    origin;
    double      frustFraction;

    // Compute root vectors along edges of view frustum.
    if (camera)
        {
        // eye coordinates: 000 at eye
        //      z perpendicular to focal plane (positive z is out back of head)
        //      x,y are horizontal, vertical edges of frustum.

        DVec3d eyeToOrigin = DVec3d::FromStartEnd(camera->GetEyePoint(), inOrigin);      // Subtract camera position (still in root)
        viewRot.Multiply(eyeToOrigin);                                                   // Rotate to view coordinates.

        double focusDistance = camera->GetFocusDistance();
        double zDeltaLimit = (-focusDistance / GetCameraPlaneRatio()) - eyeToOrigin.z;      // Limit front clip to be in front of camera plane.

        double zDelta = (delta.z > zDeltaLimit) ? zDeltaLimit : delta.z;                 // Limited zDelta.
        double zBack  = eyeToOrigin.z;                                                   // Distance from eye to back clip plane.
        double zFront = zBack + zDelta;                                                  // Distance from eye to front clip plane.
        double minimumFrontToBackClipRatio = T_HOST.GetGraphicsAdmin()._GetCameraFrustumNearScaleLimit();

        if (zFront / zBack < minimumFrontToBackClipRatio)
            {
            // The ratio between the front and back clipping plane exceeds the resolution of the QuickVision Z-Buffer.
            // We'll handle this by calculating the front clip from the back clipping plane - but first limit
            // the back clipping distance to 1000 Meters. - This make the minimum front clip
            // around a third of a meter and avoids the case where a very large back clip distance
            // causes objects near the camera to disappear.     - RBB 03/2007.

            double  maximumBackClip = DgnUnits::OneKilometer();

            if (-zBack > maximumBackClip)
                zBack = -maximumBackClip;

            zFront = zBack * minimumFrontToBackClipRatio;

            eyeToOrigin.z = zBack;
            zDelta = zFront - eyeToOrigin.z;
            }

        // z out back of eye ====> origin z coordinates are negative.  (Back plane more negative than front plane)
        double backFraction  = -zBack  / focusDistance;         // Perspective fraction at back clip plane.
        double frontFraction = -zFront / focusDistance;         // Perspective fraction at front clip plane.

         // delta.x,delta.y are view rectangle sizes at focus distance.  Scale to back plane:
        xExtent.Scale(xVector, delta.x * backFraction);                                // xExtent at back == delta.x * backFraction.
        yExtent.Scale(yVector, delta.y * backFraction);                                // yExtent at back == delta.y * backFraction.

        // Calculate the zExtent in the View coordinate system.
        zExtent.x = eyeToOrigin.x * (frontFraction - backFraction);                    // eyeToOrigin.x * frontFraction - eyeToOrigin.x * backFraction
        zExtent.y = eyeToOrigin.y * (frontFraction - backFraction);                    // eyeToOrigin.y * frontFraction - eyeToOrigin.y * backFraction
        zExtent.z = zDelta;
        viewRot.MultiplyTranspose(zExtent);                                            // rotate back to root coordinates.

        origin.x = eyeToOrigin.x * backFraction;                                         // Calculate origin in eye coordinates.
        origin.y = eyeToOrigin.y * backFraction;
        origin.z = eyeToOrigin.z;
        viewRot.MultiplyTranspose(origin);                                             // Rotate back to root coordinates
        origin.Add(camera->GetEyePoint());                                              // Add the eye point.
        frustFraction = frontFraction / backFraction;
        }
    else
        {
        frustFraction = 1.0;
        origin = inOrigin;
        xExtent.Scale(xVector, delta.x);
        yExtent.Scale(yVector, delta.y);
        zExtent.Scale(zVector, delta.z);
        }

    // calculate the root-to-npc mapping (using expanded frustum)
    DMap4d  newRootToNpc;
    if (!bsiDMap4d_initFromVectorFrustum(&newRootToNpc, &origin, &xExtent, &yExtent, &zExtent, frustFraction))
        {
#if defined (NO_TEST_VIEW_FRUSTUM)
        BeAssert(0);
#endif
        return  ERROR;
        }

    rootToNpc = newRootToNpc;           // Don't screw this up if we are returning ERROR (TR# 251771).
    if (compressionFraction)
        *compressionFraction = frustFraction;

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnViewport::_ConnectToOutput()
    {
    if (m_qvDCAssigned)
        return SUCCESS;

    if (NULL == m_output)
        return ERROR;

    StatusInt status = m_output->AssignDC (_GetDcForView());

    if (SUCCESS == status)
        m_qvDCAssigned = true;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* calculate the NPC-to-view transformation matrix.
* @bsimethod                                                    Andrew.Edge     08/04
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::CalcNpcToView(DMap4dR npcToView)
    {
    DPoint3d    viewLow, viewHigh;
    _GetViewCorners(viewLow, viewHigh);
    npcToView.InitFromRanges(s_NpcCorners[NPC_000], s_NpcCorners[NPC_111], viewLow, viewHigh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Andrew.Edge     08/04
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::_SetFrustumFromRootCorners(DPoint3dCP rootBox, double compressionFraction)
    {
    DPoint3d frustum[4];
    frustum[0] = rootBox[NPC_000];
    frustum[1] = rootBox[NPC_100];
    frustum[2] = rootBox[NPC_010];
    frustum[3] = rootBox[NPC_001];

    // Temporary - I don't know what to do about the front/back planes for a 3d view where we want to enforce top (e.g. mapping)
    // this works for now, but the value for GetMaxDisplayPriority is bogus.
    bool use3d = Allow3dManipulations();
    if (!use3d)
        {
        // for 2d models, make sure the z range includes all possible display priority values
        frustum[3].z = GetMaxDisplayPriority();
        frustum[0].z = frustum[1].z = frustum[2].z = -frustum[3].z;
        }
    else if (_IsSheetView())
        {
        // for 3d sheets expand the range if necessary make sure the z range includes all possible display priority values
        double displayPriority = GetMaxDisplayPriority();
        if (frustum[3].z < displayPriority)
            frustum[3].z = displayPriority;

        if (frustum[0].z > -displayPriority)
            frustum[0].z = frustum[1].z = frustum[2].z = -displayPriority;
        }

    if (m_output)
        m_output->DefineFrustum(*frustum, compressionFraction, !use3d);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    RBB                             10/90
+---------------+---------------+---------------+---------------+---------------+------*/
static void validateCamera(CameraViewControllerR controller)
    {
    CameraInfoR camera = controller.GetCameraR();
    camera.ValidateLens();
    if (camera.IsFocusValid())
         {
         // we used to call controller.CenterEyePoint(NULL) here, but that can cause existing MicroStation
         // 1-point perspective views to jump, so i removed it. - KAB
         return;
         }

    DPoint3dCR vDelta = controller.GetDelta();
    double maxDelta = vDelta.x > vDelta.y ? vDelta.x : vDelta.y;

    double focusDistance = maxDelta / (2.0 * tan(camera.GetLensAngle()/2.0));

    if (focusDistance < vDelta.z / 2.0)
        focusDistance = vDelta.z / 2.0;

    DPoint3d eyePoint;
    eyePoint.x = vDelta.x/2.0;
    eyePoint.y = vDelta.y/2.0;
    eyePoint.z = vDelta.z/2.0 + focusDistance;

    controller.GetRotation().MultiplyTranspose(eyePoint);
    eyePoint.Add(controller.GetOrigin());

    camera.SetEyePoint(eyePoint);
    camera.SetFocusDistance(focusDistance);
    }

/*---------------------------------------------------------------------------------**//**
* set up this viewport from the given viewController
* @bsimethod                                                    KeithBentley    04/02
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus DgnViewport::_SetupFromViewController()
    {
    ViewControllerP   viewController = m_viewController.get();
    if (NULL == viewController)
        return ViewportStatus::InvalidViewport;

    _AdjustAspectRatio(*viewController, false);

    DPoint3d origin = viewController->GetOrigin();
    DVec3d   delta  = viewController->GetDelta();
    m_rotMatrix     = viewController->GetRotation();

    m_rootViewFlags = viewController->GetViewFlags();
    m_is3dView    = false;
    m_isCameraOn  = false;
    m_isSheetView = false;
    m_viewOrg       = m_viewOrgUnexpanded   = origin;
    m_viewDelta     = m_viewDeltaUnexpanded = delta;
    m_zClipAdjusted = false;

    PhysicalViewControllerP physicalView = GetPhysicalViewControllerP();
    if (NULL != physicalView)
        {
        CameraViewControllerP cameraView = GetCameraViewControllerP();
        if (!Allow3dManipulations())
            {
            // we're in a "2d" view of a physical model. That means that we must have our oreintation with z out of the screen with z=0 at the center.
            AlignWithRootZ(); // make sure we're in a z Up view

            DRange3d  range = m_viewController->GetProjectExtents();
            if (range.IsEmpty())
                {
                range.low.z = -DgnUnits::OneMillimeter();
                range.high.z = DgnUnits::OneMillimeter();
                }

            double zMax = std::max(fabs(range.low.z), fabs(range.high.z));
            zMax = std::max(zMax, DgnUnits::OneMillimeter()); // make sure we have at least +-100. Data may be purely planar
            delta.z  = 2.0 * zMax;
            origin.z = -zMax;
            }
        else
            {
            m_is3dView = true;
            m_isCameraOn = (cameraView ? cameraView->IsCameraOn() : false);

            if (m_isCameraOn)
                {
                validateCamera(*cameraView);
                m_camera = cameraView->GetCameraR();
                }

            _AdjustZPlanesToModel(origin, delta, *viewController);

            // if we moved the z planes, set the "zClipAdjusted" flag.
            if (!origin.IsEqual(m_viewOrgUnexpanded) || !delta.IsEqual(m_viewDeltaUnexpanded))
                {
                m_zClipAdjusted = true;

                if (m_isCameraOn)
                    {
                    // don't let the front clip move past camera
                    DPoint3d viewCameraPosition;
                    viewCameraPosition.DifferenceOf(m_camera.GetEyePoint(), origin);
                    m_rotMatrix.Multiply(viewCameraPosition);

                    if (delta.z > viewCameraPosition.z - DgnUnits::OneMillimeter())
                        delta.z = viewCameraPosition.z - DgnUnits::OneMillimeter();
                    }
                }
            }
        }
    else
        {
        AlignWithRootZ();

        SheetViewControllerP sheetView = dynamic_cast<SheetViewControllerP> (viewController);
        if (NULL != sheetView)
            m_isSheetView = true;

        delta.z  = 200. * DgnUnits::OneMillimeter();
        origin.z = -100. * DgnUnits::OneMillimeter();
        }

    m_viewOrg   = origin;
    m_viewDelta = delta;

    DPoint3d    llb, urf;
    _GetViewCorners(llb, urf);

    double zRangeView = fabs(urf.z - llb.z);

    m_scale.x = (fabs(urf.x - llb.x) / delta.x);
    m_scale.y = (fabs(urf.y - llb.y) / delta.y);
    m_scale.z =  zRangeView / delta.z;

    m_viewDelta = delta;
    m_viewOrg   = origin;

    if (SUCCESS != _ConnectToOutput())
        return ViewportStatus::InvalidViewport;

    BeAssert(NULL == m_output || !m_output->IsDrawActive());

    double compressionFraction;
    if (SUCCESS != RootToNpcFromViewDef(m_rootToNpc, &compressionFraction, IsCameraOn() ? &m_camera : NULL, origin, delta, m_rotMatrix))
        return  ViewportStatus::InvalidViewport;

    DPoint3d rootBox[NPC_CORNER_COUNT];
    NpcToWorld(rootBox, s_NpcCorners, NPC_CORNER_COUNT);

    DMap4d      npcToView;
    CalcNpcToView(npcToView);
    m_rootToView.InitProduct(npcToView, m_rootToNpc);

    _SetFrustumFromRootCorners(rootBox, compressionFraction);

    m_frustumValid = true;

    return ViewportStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* determine whether the points in the given polyhedron are in the correct order or are inside out. If not, reverse sense
* @bsimethod                                    Keith.Bentley                   04/05
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::FixFrustumOrder(Frustum& frustum)
    {
    DPoint3dP polyhedron = frustum.GetPtsP();

    DVec3d u, v, w;
    u.DifferenceOf(polyhedron[NPC_001], polyhedron[NPC_000]);
    v.DifferenceOf(polyhedron[NPC_010], polyhedron[NPC_000]);
    w.DifferenceOf(polyhedron[NPC_100], polyhedron[NPC_000]);

    if (u.TripleProduct(v, w) <= 0)
        return;

    // frustum has mirroring, reverse points
    for (int i=0; i<8; i+=2)
        {
        DPoint3d tmpPoint = polyhedron[i];
        polyhedron[i] = polyhedron[i+1];
        polyhedron[i+1] = tmpPoint;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Set up the ViewController structure associated with this view with the supplied Frustum.
* @bsimethod                                                    KeithBentley    11/02
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus DgnViewport::SetupFromFrustum(Frustum const& inFrustum)
    {
    ViewControllerP   viewController = m_viewController.get();
    if (NULL == viewController || !m_frustumValid)
        return ViewportStatus::InvalidWindow;

    ViewportStatus validSize = viewController->SetupFromFrustum(inFrustum);

    ViewportStatus status = _SetupFromViewController();
    if (ViewportStatus::Success != status)
        return  status;

    // may have been corrected by SetupFromViewController for sheet/2d views
    viewController->SetRotation(m_rotMatrix);
    return validSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/90
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus DgnViewport::ChangeArea(DPoint3dCP pts)
    {
    ViewControllerP viewController = m_viewController.get();
    if (NULL == viewController)
        return  ViewportStatus::InvalidViewport;

    if (!m_qvDCAssigned)
        _SetupFromViewController();

    DPoint3d worldPts[3] = {pts[0], pts[1], viewController->GetOrigin()};
    DPoint3d viewPts[3];
    GetRotMatrix().Multiply(viewPts, worldPts, 3);

    DRange3d range = DRange3d::From(viewPts, 2);
    DVec3d delta;
    delta.DifferenceOf(range.high, range.low);

    CameraViewControllerP cameraView = GetCameraViewControllerP();
    if (cameraView && cameraView->IsCameraOn())
        {
        DPoint3d npcPts[2];
        WorldToNpc(npcPts, worldPts, 2);

        /// NEEDS_WORK_RANGE_OF_PIXELS - this should be replace by a QV call that will find the range of the pixels in the rectangle.
        double low, high;
        DRange3d npcRange = DRange3d::From(npcPts, 2);
        if (SUCCESS != DetermineVisibleDepthNpc(low, high, &npcRange))
            high = .5;

        LIMIT_RANGE(0.0, 1.0, high);

        npcPts[0].z = npcPts[1].z = high;
        NpcToWorld(worldPts, npcPts, 2);

        double lensAngle = cameraView->GetLensAngle();
        double focusDist = std::max(delta.x, delta.y) / (2.0 * tan(lensAngle / 2.0));

        DPoint3d newTarget = DPoint3d::FromInterpolate(worldPts[0], .5, worldPts[1]);
        DPoint3d newEye = DPoint3d::FromSumOf(newTarget, cameraView->GetZVector(), focusDist);

        auto stat = cameraView->LookAtUsingLensAngle(newEye, newTarget, cameraView->GetYVector(), lensAngle);
        if (ViewportStatus::Success != stat)
            return stat;
        }
    else
        {
        /* get the view extents */
        delta.z = viewController->GetDelta().z;

        /* make sure its not too big or too small */
        auto stat = ValidateWindowSize(delta, true);
        if (stat != ViewportStatus::Success)
            return stat;

        viewController->SetDelta(delta);

        range.low.z = viewPts[2].z;     // don't change z origin
        DPoint3d origin;
        GetRotMatrix().MultiplyTranspose(&origin, &range.low, 1);
        viewController->SetOrigin(origin);
        }

    _SynchWithViewController(true);
    return ViewportStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* Get an 8-point box corresponding to the 8 corners of the viewport in the specified coordinate system.
* When front or back clipping is turned off, there are two sets of corners that may be of interest.
* The "expanded" box is the one that is computed by examining the extents of the content of the view and moving
* the front and back planes to enclose everything in the view. The "unexpanded" box is the one that is saved in
* the view definition.
* @bsimethod                                                    KeithBentley    08/02
+---------------+---------------+---------------+---------------+---------------+------*/
Frustum DgnViewport::GetFrustum(DgnCoordSystem sys, bool expandedBox) const
    {
    Frustum  box;

    // if they are looking for the "unexpanded" (that is before f/b clipping expansion) box, we need to get the npc
    // coordinates that correspond to the unexpanded box in the npc space of the Expanded view (that's the basis for all
    // of the root-based maps.)
    if (!expandedBox && m_zClipAdjusted)
        {
        // to get unexpanded box, we have to go recompute rootToNpc from original viewController.
        DMap4d  ueRootToNpc;
        RootToNpcFromViewDef(ueRootToNpc, NULL, IsCameraOn() ? &m_camera : NULL, m_viewOrgUnexpanded, m_viewDeltaUnexpanded, m_rotMatrix);

        // get the root corners of the unexpanded box
        DPoint3d  ueRootBox[NPC_CORNER_COUNT];
        ueRootToNpc.M1.MultiplyAndRenormalize(ueRootBox, s_NpcCorners, NPC_CORNER_COUNT);

        // and convert them to npc coordinates of the expanded view
        WorldToNpc(box.GetPtsP(), ueRootBox, NPC_CORNER_COUNT);
        }
    else
        {
        // otherwise, just start from a unit cube.
        memcpy(box.GetPtsP(), s_NpcCorners, sizeof (DPoint3d) * NPC_CORNER_COUNT);
        }

    // now convert from NPC space to the specified coordinate system.
    switch (sys)
        {
        case    DgnCoordSystem::View:
            NpcToView(box.GetPtsP(), box.GetPts(), NPC_CORNER_COUNT);
            break;

        case    DgnCoordSystem::World:
            NpcToWorld(box.GetPtsP(), box.GetPts(), NPC_CORNER_COUNT);
            break;

        case DgnCoordSystem::Screen:
            NpcToView(box.GetPtsP(), box.GetPts(), NPC_CORNER_COUNT);
            ViewToScreen(box.GetPtsP(), box.GetPtsP(), NPC_CORNER_COUNT);
            break;
        }

    return box;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d DgnViewport::DetermineDefaultRotatePoint()
    {
    double low, high;
    if (SUCCESS != DetermineVisibleDepthNpc(low, high) && IsCameraOn())
        {
        // if there are no elements in the view and the camera is on, use the camera target point
        return GetCameraTarget();
        }

    DPoint3d center = DPoint3d::From(.5, .5, (high + low) * .5);
    return NpcToWorld(center);
    }

/*---------------------------------------------------------------------------------**//**
* scroll the view by a given number of pixels.
* Camera position is unchanged.
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus DgnViewport::Scroll(Point2dCP screenDist) // => distance to scroll in pixels
    {
    ViewControllerP   viewController = m_viewController.get();
    if (NULL == viewController)
        return ViewportStatus::InvalidViewport;

    DVec3d offset;
    offset.Init(screenDist->x, screenDist->y, 0.0);

    CameraViewControllerP cameraView = GetCameraViewControllerP();
    if (cameraView && cameraView->IsCameraOn())
        {
        // get current box in view coordinates
        Frustum frust = GetFrustum(DgnCoordSystem::View, false);
        frust.Translate(offset);
        ViewToWorld(frust.GetPtsP(), frust.GetPtsP(), NPC_CORNER_COUNT);

        cameraView->SetupFromFrustum(frust);
        cameraView->CenterEyePoint();

        return SetupFromViewController();
        }

    DPoint3d pts[2];
    pts[0].Zero();
    pts[1] = offset;

    ViewToWorld(pts, pts, 2);
    DVec3d dist;
    dist.DifferenceOf(pts[1], *pts);

    if (!m_is3dView)
        dist.z = 0.0;

    DPoint3d oldOrg = viewController->GetOrigin();
    DPoint3d newOrg;
    newOrg.SumOf(oldOrg, dist);
    viewController->SetOrigin(newOrg);

    _AdjustFencePts(viewController->GetRotation(), oldOrg, newOrg);
    return _SetupFromViewController();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/04
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d DgnViewport::GetCameraTarget() const
    {
    DVec3d viewZ;
    m_rotMatrix.GetRow(viewZ, 2);

    DPoint3d target;
    target.SumOf(m_camera.GetEyePoint(), viewZ, -1.0 * m_camera.GetFocusDistance());
    return target;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/10
+---------------+---------------+---------------+---------------+---------------+------*/
double DgnViewport::GetFocusPlaneNpc()
    {
    return WorldToNpc(GetCameraTarget()).z;
    }

/*---------------------------------------------------------------------------------**//**
* Zoom the view by a scale factor, placing the new center at the projection of the given point (root coordinates)
* on the current plane.
* Updates ViewController and re-synchs viewport.
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus DgnViewport::Zoom(DPoint3dCP newCenterRoot, double factor)
    {
    ViewControllerP   viewController = m_viewController.get();
    if (NULL == viewController)
        return ViewportStatus::InvalidViewport;

    CameraViewControllerP cameraView = GetCameraViewControllerP();
    if (cameraView && cameraView->IsCameraOn())
        {
        DPoint3d centerNpc;          // center of view in npc coords
        centerNpc.Init(.5, .5, .5);

        DPoint3d    newCenterNpc;       // get new center of view in npc coords
        if (NULL != newCenterRoot)
            WorldToNpc(&newCenterNpc, newCenterRoot, 1);
        else
            newCenterNpc = centerNpc;   // leave it alone.

        Transform scaleTransform;
        scaleTransform.InitFromMatrixAndFixedPoint(RotMatrix::FromScaleFactors(factor, factor, 1.0), centerNpc);

        DPoint3d    offset;             // offset by difference of old/new center
        offset.DifferenceOf(newCenterNpc, centerNpc);
        offset.z = 0.0;     // z center stays the same.

        Transform  offsetTransform = Transform::From(offset);
        Transform product;
        product.InitProduct(offsetTransform, scaleTransform);

        Frustum frust = GetFrustum(DgnCoordSystem::Npc, false);
        product.Multiply((frust.GetPtsP()), NPC_CORNER_COUNT);

        NpcToWorld(frust.GetPtsP(), frust.GetPtsP(), NPC_CORNER_COUNT);

        cameraView->SetupFromFrustum(frust);
        cameraView->CenterEyePoint();
        return SetupFromViewController();
        }

    // for non-camera views, do the zooming by adjusting the origin and delta directly so there can be no
    // chance of the rotation changing due to numerical precision errors calculating it from the frustum corners.
    DVec3d delta = viewController->GetDelta();
    delta.x *= factor;
    delta.y *= factor;

    // first check to see whether the zoom operation results in an invalid view. If so, make sure we don't change anything
    ViewportStatus validSize = ValidateWindowSize(delta, false);
    if (ViewportStatus::Success != validSize)
        return  validSize;

    DPoint3d center = (NULL != newCenterRoot) ? *newCenterRoot : viewController->GetCenter();

    if (!Allow3dManipulations())
        center.z = 0.0;

    DPoint3d oldOrg = viewController->GetOrigin();
    DPoint3d newOrg = oldOrg;
    RotMatrix rotation = viewController->GetRotation();

    rotation.Multiply(newOrg);
    rotation.Multiply(center);

    viewController->SetDelta(delta);

    newOrg.x = center.x - delta.x/2.0;
    newOrg.y = center.y - delta.y/2.0;
    rotation.MultiplyTranspose(newOrg);
    viewController->SetOrigin(newOrg);

    _AdjustFencePts(rotation, oldOrg, newOrg);
    return  _SetupFromViewController();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/02
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus DgnViewport::_Activate(QvPaintOptions const& opts)
    {
    if (NULL == m_output || !m_qvParamsSet)
        return  ViewportStatus::ViewNotInitialized;

    m_output->AccumulateDirtyRegion(opts.WantAccumulateDirty());

    if (SUCCESS != m_output->BeginDraw(opts.WantEraseBefore()))
        return  ViewportStatus::DrawFailure;

    return  ViewportStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
int DgnViewport::GetDefaultIndexedLineWidth(int index)
    {
    LIMIT_RANGE (0, 31, index);

    return index+1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/02
+---------------+---------------+---------------+---------------+---------------+------*/
int DgnViewport::_GetIndexedLineWidth(int index) const
    {
    return DgnViewport::GetDefaultIndexedLineWidth(index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t DgnViewport::GetDefaultIndexedLinePattern(int index)
    {
    if (index < 0 || index > 7)
        index = 0;

    return s_rasterLinePatterns[index];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/02
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t DgnViewport::_GetIndexedLinePattern(int index) const
    {
    return DgnViewport::GetDefaultIndexedLinePattern(index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/02
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::SetSymbologyRgb(ColorDef lineColor, ColorDef fillColor, int lineWidth, int lineCodeIndex)
    {
    m_output->SetSymbology(lineColor, fillColor, lineWidth, _GetIndexedLinePattern(lineCodeIndex));
    }

const double    VISIBILITY_GOAL           = 40.0;
const int       HSV_SATURATION_WEIGHT     = 4;
const int       HSV_VALUE_WEIGHT          = 2;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static double colorVisibilityCheck(ColorDef fg, ColorDef bg)
    {
    // Compute luminosity...
    double      red   = abs(fg.GetRed()   - bg.GetRed());
    double      green = abs(fg.GetGreen() - bg.GetGreen());
    double      blue  = abs(fg.GetBlue()  - bg.GetBlue());

    return (0.30 * red) + (0.59 * green) + (0.11 * blue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void adjustHSVColor(HsvColorDef& fgHsv, bool darkenColor, int delta)
    {
    if (darkenColor)
        {
        int     weightedDelta = delta*HSV_VALUE_WEIGHT;

        if (fgHsv.value >= weightedDelta)
            {
            fgHsv.value -= weightedDelta;
            }
        else
            {
            weightedDelta -= fgHsv.value;

            fgHsv.value = 0;
            fgHsv.saturation = fgHsv.saturation + weightedDelta < 100 ? fgHsv.saturation + weightedDelta : 100;
            }
        }
    else
        {
        int weightedDelta = delta*HSV_SATURATION_WEIGHT;

        if (fgHsv.saturation >= weightedDelta)
            {
            fgHsv.saturation -= weightedDelta;
            }
        else
            {
            weightedDelta -= fgHsv.saturation;

            fgHsv.saturation = 0;
            fgHsv.value = fgHsv.value + weightedDelta < 100 ? fgHsv.value + weightedDelta : 100;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void adjustColorForContrast(ColorDef& fg, ColorDef bg, ColorDef vw)
    {
    double visibility = colorVisibilityCheck(fg, bg);

    if (VISIBILITY_GOAL <= visibility)
        return;

    int         adjPercent = (int) (((VISIBILITY_GOAL - visibility) / 255.0) * 100.0);
    HsvColorDef brightHSV, darkerHSV, fgHSV;

    ColorUtil::ConvertRgbToHsv(&fgHSV, &fg);

    darkerHSV = fgHSV;
    brightHSV = fgHSV;

    adjustHSVColor(darkerHSV, true,  adjPercent);
    adjustHSVColor(brightHSV, false, adjPercent);

    ColorDef bright, darker;

    bright = darker = fg; // NOTE: Initialize to original color to preserve transparency
    ColorUtil::ConvertHsvToRgb(&darker, &darkerHSV);
    ColorUtil::ConvertHsvToRgb(&bright, &brightHSV);

    if (bright == bg) // Couldn't adjust brighter...
        {
        fg = darker;
        return;
        }

    if (darker == bg) // Couldn't adjust darker...
        {
        fg = bright;
        return;
        }

    // NOTE: Best choice is the one most visible against the background color...
    fg = (colorVisibilityCheck(bright, vw) >= colorVisibilityCheck(darker, vw)) ? bright : darker;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef DgnViewport::AdjustColorForContrast(ColorDef thisColor, ColorDef againstColor) const
    {
    ColorDef fg, bg, vw;

    fg = thisColor;
    bg = againstColor;
    vw = GetBackgroundColor();

    adjustColorForContrast(fg, bg, vw);

    return fg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/02
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef DgnViewport::MakeTransparentIfOpaque(ColorDef color, int transparency)
    {
    // if it already has a transparency, leave it alone.
    if (0 != color.GetAlpha())
        return color;

    return  MakeColorTransparency(color, transparency);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/02
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef DgnViewport::MakeColorTransparency(ColorDef color, int transparency)
    {
    color.SetAlpha((Byte) transparency);
    return color;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef DgnViewport::GetSolidFillEdgeColor(ColorDef inColor)
    {
    // Logic derived from QVCamera::calcContrast...
    ColorDef     backgroundColorDef, inColorDef, outColorDef;

    backgroundColorDef = GetBackgroundColor();
    inColorDef = inColor;

    double  elemColor[3], bgRGBA[3];

    bgRGBA[0] = backgroundColorDef.GetRed()   / 255.0;
    bgRGBA[1] = backgroundColorDef.GetGreen() / 255.0;
    bgRGBA[2] = backgroundColorDef.GetBlue()  / 255.0;

    elemColor[0] = inColorDef.GetRed()   / 255.0;
    elemColor[1] = inColorDef.GetGreen() / 255.0;
    elemColor[2] = inColorDef.GetBlue()  / 255.0;

    double  s;
    double  bgi = (bgRGBA[0] * 0.3f + bgRGBA[1] * 0.59f + bgRGBA[2] * 0.11f);
    double  rgbi = (elemColor[0] * 0.3f + elemColor[1] * 0.59f + elemColor[2] * 0.11f);

    if (rgbi > 0.81f)
        s = (bgi > 0.57f) ? 0.0f : 0.7f;
    else if (rgbi > 0.57f)
        s = (bgi > 0.57f) ? 0.0f : 1.0f;
    else
        s = (bgi < 0.81f) ? 1.0f : 0.7f;

    outColorDef.SetAllColors((unsigned char) (255.0 * s));
    return outColorDef;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   02/03
+---------------+---------------+---------------+---------------+---------------+------*/
double DgnViewport::GetPixelSizeAtPoint(DPoint3dCP rootPtP, DgnCoordSystem coordSys) const // can be NULL - if so, use center of view
    {
    DPoint3d    rootTestPt;

    if (NULL == rootPtP)
        {
        DPoint3d    npcCenter;

        npcCenter.x = npcCenter.y = npcCenter.z = 0.5;
        NpcToWorld(&rootTestPt, &npcCenter, 1);
        }
    else
        {
        rootTestPt = *rootPtP;
        }

    DPoint4d    viewPts[2];

    WorldToView(&viewPts[0], &rootTestPt, 1);
    viewPts[1] = viewPts[0];
    viewPts[1].x += viewPts[1].w;

    DPoint3d    rootPts[2];

    ViewToWorld(rootPts, viewPts, 2);

    switch (coordSys)
        {
        case DgnCoordSystem::Screen:
        case DgnCoordSystem::View:
            {
            WorldToView(rootPts, rootPts, 2);
            break;
            }

        case DgnCoordSystem::Npc:
            {
            WorldToNpc(rootPts, rootPts, 2);
            break;
            }

        case DgnCoordSystem::World:
        default:
            break;
        }

    return rootPts[0].Distance(rootPts[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/90
+---------------+---------------+---------------+---------------+---------------+------*/
static void limitWindowSize(ViewportStatus& error, double& value, ViewportStatus lowErr, ViewportStatus highErr)
    {
    if (value < DgnViewport::GetMinViewDelta())
        {
        value  = DgnViewport::GetMinViewDelta();
        error  = lowErr;
        }
    else if (value > DgnViewport::GetMaxViewDelta())
        {
        value  = DgnViewport::GetMaxViewDelta();
        error  = highErr;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::OutputFrustumErrorMessage(ViewportStatus errorStatus)
    {
    DgnCoreL10N::StringId id;
    switch (errorStatus)
        {
        case ViewportStatus::InvalidWindow:
            id = DgnCoreL10N::VIEWFRUST_Message_InvalidWindow();
            break;
        case ViewportStatus::MaxWindow:
            id = DgnCoreL10N::VIEWFRUST_Message_MaxWindow();
            break;
        case ViewportStatus::MinWindow:
            id = DgnCoreL10N::VIEWFRUST_Message_MinWindow();
            break;
        case ViewportStatus::MaxZoom:
            id = DgnCoreL10N::VIEWFRUST_Message_MaxZoom();
            break;

        default:
            return;
        }

    Utf8String msg = DgnCoreL10N::GetString(id);
    if (msg.size() > 0)
        NotificationManager::OutputMessage(NotifyMessageDetails(OutputMessagePriority::Error, msg.c_str()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/86
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus DgnViewport::ValidateWindowSize(DPoint3dR delta, bool messageNeeded)
    {
    ViewportStatus  error=ViewportStatus::Success, ignore;

    limitWindowSize(error,  delta.x, ViewportStatus::MinWindow, ViewportStatus::MaxWindow);
    limitWindowSize(error,  delta.y, ViewportStatus::MinWindow, ViewportStatus::MaxWindow);
    limitWindowSize(ignore, delta.z, ViewportStatus::MinWindow, ViewportStatus::MaxWindow);    // always check z depth

    if (messageNeeded)
        OutputFrustumErrorMessage(error);

    return error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t DgnViewport::GetMaxDisplayPriority() {return MAX_HW_DISPLAYPRIORITY;}
int32_t DgnViewport::GetDisplayPriorityFrontPlane() {return GetMaxDisplayPriority() - RESERVED_DISPLAYPRIORITY;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley  10/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnViewport::UseClipVolume(DgnModelCP modelRef) const
    {
    if (!IsActive())
        return false;

    return !GetViewController().GetViewFlags().noClipVolume;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RichardTrefz    08/02
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef DgnViewport::GetContrastToBackgroundColor() const
    {
    // should we use black or white
    bool    invert  = ((m_backgroundColor.GetRed() + m_backgroundColor.GetGreen() + m_backgroundColor.GetBlue()) > (255*3)/2);
    return  invert ? ColorDef::Black()  : ColorDef::White();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::_GetViewName(WStringR name) const
    {
    name = L"";
    if (!Is3dView())
        return;

    ViewController::GetStandardViewName(name, ViewController::IsStandardViewRotation(m_rotMatrix, true));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::_SynchWithViewController(bool saveInUndo)
    {
    _SetupFromViewController();

    if (saveInUndo)
        _SynchViewTitle();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    05/2013
//---------------------------------------------------------------------------------------
void DgnViewport::SetToolGraphicsHandler(ToolGraphicsHandler* handler)
    {
    m_toolGraphicsHandler = handler;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    05/2013
//---------------------------------------------------------------------------------------
void DgnViewport::DrawToolGraphics(ViewContextR context, bool isPreupdate)
    {
    if (NULL != m_toolGraphicsHandler)
        m_toolGraphicsHandler->_DrawToolGraphics(context, isPreupdate);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   07/04
+---------------+---------------+---------------+---------------+---------------+------*/
double DgnViewport::GetGridScaleFactor()
    {
    double  scaleFactor = 1.0;

#ifdef DGNV10FORMAT_CHANGES_WIP
    // Apply ACS scale to grid if ACS Context Lock active...
    if (TO_BOOL(m_rootModel->GetModelFlag(MODELFLAG_ACS_LOCK)))
        {
        IAuxCoordSysP acs = IACSManager::GetManager().GetActive(*this);
        if (NULL != acs)
            scaleFactor *= acs->GetScale();
        }
#endif

    return scaleFactor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::GetGridRoundingDistance(DPoint2dR roundingDistance)
    {
#ifdef DGNV10FORMAT_CHANGES_WIP
    ModelInfoCR modelInfo = m_rootModel->GetModelInfo();

    double uorPerGrid     = modelInfo.GetUorPerGrid();
    double gridRatio      = modelInfo.GetGridRatio();
    double roundUnit      = modelInfo.GetRoundoffUnit();
    double roundUnitRatio = modelInfo.GetRoundoffRatio();

    if (TO_BOOL(m_rootModel->GetModelFlag(MODELFLAG_GRID_LOCK)))
        roundingDistance.x = uorPerGrid;
    else
        roundingDistance.x = roundUnit;

    if (TO_BOOL(m_rootModel->GetModelFlag(MODELFLAG_UNIT_LOCK)))
        {
        if (roundUnit > roundingDistance.x)
            roundingDistance.x = roundUnit;

        if (0.0 != roundUnitRatio)
            gridRatio = roundUnitRatio;
        }

    roundingDistance.y = roundingDistance.x * gridRatio;
    roundingDistance.Scale(roundingDistance, GetGridScaleFactor());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    kab             06/86
+---------------+---------------+---------------+---------------+---------------+------*/
static void roundGrid(double& num, double units)
    {
    double  sign = ((num * units) < 0.0) ? -1.0 : 1.0;

    num = (num * sign) / units + 0.5;
    num = units * sign * floor(num);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::GridFix(DPoint3dR pointRoot, RotMatrixCR rMatrixRoot, DPoint3dCR originRoot, DPoint2dCR roundingDistanceRoot, bool isoGrid)
    {
    DVec3d planeNormal;
    rMatrixRoot.GetRow(planeNormal, 2);

    DVec3d eyeVec;
    if (m_isCameraOn)
        eyeVec.NormalizedDifference(pointRoot, m_camera.GetEyePoint());
    else
        m_rotMatrix.GetRow(eyeVec, 2);

    LegacyMath::Vec::LinePlaneIntersect(&pointRoot, &pointRoot, &eyeVec, &originRoot, &planeNormal, false);

    // get origin and point in view coordinate system
    DPoint3d    pointRootView, originRootView;
    rMatrixRoot.Multiply(pointRootView, pointRoot);
    rMatrixRoot.Multiply(originRootView, originRoot);

    // see whether we need to adjust the origin for iso-grid
    if (isoGrid)
        {
        long ltmp = (long) (pointRootView.y / roundingDistanceRoot.y);

        if (ltmp & 0x0001)
            originRootView.x += (roundingDistanceRoot.x / 2.0);
        }

    // subtract off the origin
    pointRootView.y -= originRootView.y;
    pointRootView.x -= originRootView.x;

    // round off the remainder to the grid distances
    roundGrid(pointRootView.y, roundingDistanceRoot.y);
    roundGrid(pointRootView.x, roundingDistanceRoot.x);

    // add the origin back in
    pointRootView.x += originRootView.x;
    pointRootView.y += originRootView.y;

    // go back to root coordinate system
    rMatrixRoot.MultiplyTranspose(pointRoot,pointRootView);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::PointToStandardGrid(DPoint3dR point, DPoint3dR gridOrigin, RotMatrixR rMatrix)
    {
#ifdef DGNV10FORMAT_CHANGES_WIP
    DPoint2d roundingDistanceRoot;
    GetGridRoundingDistance(roundingDistanceRoot);

    GridFix(point, rMatrix, gridOrigin, roundingDistanceRoot, TO_BOOL(m_rootModel->GetModelFlag(MODELFLAG_ISO_GRID)));
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/05
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef ViewController::ResolveBGColor() const
    {
    ColorDef bgColor = ColorDef::Black();

    // First, see if the view's background color override flag is on. If so, use it.
    if (GetViewFlags().bgColor)
        {
        bgColor = GetBackgroundColor();
        }

    // If background color resolved to be black, and user wants inverted, we set background color to white
    if (ColorDef::Black() == bgColor && T_HOST.GetGraphicsAdmin()._WantInvertBlackBackground())
        bgColor = ColorDef::White();

    return bgColor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/05
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef DgnViewport::_GetWindowBgColor() const
    {
    return (m_viewController.IsValid()) ? m_viewController->ResolveBGColor() : ColorDef::Black();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::ScheduleProgressiveDisplay(IProgressiveDisplay& pd)
    {
    IProgressiveDisplayPtr pdptr(&pd);

    auto iFound = std::find(m_progressiveDisplay.begin(), m_progressiveDisplay.end(), pdptr);
    if (iFound != m_progressiveDisplay.end())
        {
        *iFound = pdptr;
        }
    else
        {
        m_progressiveDisplay.push_back(pdptr);
        }

    // *** TBD: Sort in priority order
    }
