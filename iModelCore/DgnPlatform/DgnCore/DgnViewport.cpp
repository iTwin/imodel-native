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
DgnViewport::DgnViewport(Render::Target* target) : m_renderTarget(target)
    {
    m_minLOD            = DEFAULT_MINUMUM_LOD;
    m_targetParamsSet   = false;
    m_isCameraOn        = false;
    m_needsRefresh      = false;
    m_zClipAdjusted     = false;
    m_is3dView          = false;
    m_invertY           = true;
    m_frustumValid      = false;
    m_toolGraphicsHandler = nullptr;
    m_maxUndoSteps      = 0;
    m_undoActive        = false;
    m_needSynchWithViewController = true;
    m_targetCenterValid = false;
    m_viewingToolActive = false;
    m_initializedBackingStore = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::Initialize(ViewControllerR viewController)
    {
    m_viewController = &viewController;
    viewController._OnAttachedToViewport(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/02
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::DestroyViewport()
    {
    m_progressiveDisplay.clear();
    m_viewController = nullptr;
    m_targetParamsSet = false;
    m_frustumValid = false;
    m_renderTarget = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::InitViewSettings(bool useBgTexture)
    {
    BeAssert(m_renderTarget.IsValid());

    m_renderTarget->SetViewAttributes(GetViewFlags(), GetBackgroundColor(), useBgTexture, _WantAntiAliasLines(), _WantAntiAliasText());
    m_targetParamsSet = true;
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
    MINIMUM_WINDOW_DEPTH = -32767,
    MAXIMUM_WINDOW_DEPTH = 32767,
    };

/*---------------------------------------------------------------------------------**//**
* Get the DgnCoordSystem::View coordinates of lower-left-back and upper-right-front corners of a viewport.
* NOTE: the y values are "swapped" (llb.y is greater than urf.y) on the screen and and "unswapped" when we plot.
* @bsimethod                                                    KeithBentley    04/02
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d DgnViewport::GetViewCorners() const
    {
    BSIRect viewRect = GetViewRect();

    DRange3d corners;
    corners.low.x  = viewRect.origin.x;
    corners.low.z  = MINIMUM_WINDOW_DEPTH;

    corners.high.x = viewRect.corner.x;
    corners.high.z = MAXIMUM_WINDOW_DEPTH;

    if (m_invertY)
        {
        // y's are swapped on the screen!
        corners.low.y = viewRect.corner.y;
        corners.high.y = viewRect.origin.y;
        }
    else
        {
        corners.low.y = viewRect.origin.y;
        corners.high.y = viewRect.corner.y;
        }
    return corners;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::ViewToNpc(DPoint3dP npcVec, DPoint3dCP screenVec, int nPts) const
    {
    DRange3d corners = GetViewCorners();
    Transform scrToNpcTran;
    bsiTransform_initFromRange(nullptr, &scrToNpcTran, &corners.low, &corners.high);
    scrToNpcTran.Multiply(npcVec, screenVec, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::NpcToView(DPoint3dP screenVec, DPoint3dCP npcVec, int nPts) const
    {
    DRange3d corners = GetViewCorners();
    Transform    npcToScrTran;
    bsiTransform_initFromRange(&npcToScrTran, nullptr, &corners.low, &corners.high);
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
    bsiDMatrix4d_multiplyWeightedDPoint3dArray(&m_rootToView.M0, screenPts, rootPts, nullptr, nPts);
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
    BSIRect viewRect = GetViewRect();
    viewController.AdjustAspectRatio(viewRect.Aspect(), expandView);
    }

/*---------------------------------------------------------------------------------**//**
* Get an origin, 3 direction vectors, and a compression fraction defining a view frustum from the view
* definition specified by camera, origin, delta, and rMatrix.
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnViewport::RootToNpcFromViewDef(DMap4dR rootToNpc, double* compressionFraction, CameraInfo const* camera,
                                            DPoint3dCR inOrigin, DPoint3dCR delta, RotMatrixCR viewRot) const
    {
    DVec3d    xVector, yVector, zVector;
    viewRot.GetRows(xVector, yVector, zVector);

    DPoint3d xExtent, yExtent, zExtent;
    DPoint3d origin;
    double   frustFraction;

    // Compute root vectors along edges of view frustum.
    if (camera)
        {
        // eye coordinates: 000 at eye
        //      z perpendicular to focal plane (positive z is out back of head)
        //      x,y are horizontal, vertical edges of frustum.

        DVec3d eyeToOrigin = DVec3d::FromStartEnd(camera->GetEyePoint(), inOrigin);      // Subtract camera position (still in root)
        viewRot.Multiply(eyeToOrigin);                                                   // Rotate to view coordinates.

        double focusDistance = camera->GetFocusDistance();
        double zDeltaLimit = (-focusDistance / GetCameraPlaneRatio()) - eyeToOrigin.z;   // Limit front clip to be in front of camera plane.
        double zDelta = (delta.z > zDeltaLimit) ? zDeltaLimit : delta.z;                 // Limited zDelta.
        double zBack  = eyeToOrigin.z;                                                   // Distance from eye to back clip plane.
        double zFront = zBack + zDelta;                                                  // Distance from eye to front clip plane.
        double minimumFrontToBackClipRatio = GetRenderTarget()->_GetCameraFrustumNearScaleLimit();

        if (zFront / zBack < minimumFrontToBackClipRatio)
            {
            // The ratio between the front and back clipping plane exceeds the resolution of the QuickVision Z-Buffer.
            // We'll handle this by calculating the front clip from the back clipping plane - but first limit
            // the back clipping distance to 1000 Meters. - This make the minimum front clip
            // around a third of a meter and avoids the case where a very large back clip distance
            // causes objects near the camera to disappear.     - RBB 03/2007.

            double maximumBackClip = DgnUnits::OneKilometer();

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

        origin.x = eyeToOrigin.x * backFraction;                                       // Calculate origin in eye coordinates.
        origin.y = eyeToOrigin.y * backFraction;
        origin.z = eyeToOrigin.z;
        viewRot.MultiplyTranspose(origin);                                             // Rotate back to root coordinates
        origin.Add(camera->GetEyePoint());                                             // Add the eye point.
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

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnViewport::_ConnectRenderTarget()
    {
    if (m_deviceAssigned)
        return SUCCESS;

    if (!m_renderTarget.IsValid())
        return ERROR;

    StatusInt status = m_renderTarget->AssignRenderDevice(*this);
    if (SUCCESS == status)
        m_deviceAssigned = true;

    return status;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* calculate the NPC-to-view transformation matrix.
* @bsimethod                                                    Andrew.Edge     08/04
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::CalcNpcToView(DMap4dR npcToView)
    {
    DRange3d corners = GetViewCorners();
    npcToView.InitFromRanges(s_NpcCorners[NPC_000], s_NpcCorners[NPC_111], corners.low, corners.high);
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

    if (m_renderTarget.IsValid())
        m_renderTarget->DefineFrustum(*frustum, compressionFraction, !use3d);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    RBB                             10/90
+---------------+---------------+---------------+---------------+---------------+------*/
static void validateCamera(CameraViewControllerR controller)
    {
    CameraInfoR camera = controller.GetControllerCameraR();
    camera.ValidateLens();
    if (camera.IsFocusValid())
         return;

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
* The front/back clipping planes may need to be adjusted to fit around the actual range of the elements in the model.
* @bsimethod                                                    KeithBentley    11/02
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::_AdjustZPlanesToModel(DPoint3dR origin, DVec3dR delta, ViewControllerCR viewController) const
    {
    if (!m_is3dView)
        return;

    m_rotMatrix.Multiply(origin);   // put origin into view orientation

    Transform viewTransform;
    viewTransform.InitFrom(m_rotMatrix);

    DRange3d extents = viewController.GetViewedExtents();
    if (!extents.IsEmpty())
        viewTransform.Multiply(extents, extents);
    else
        {
        extents.low = origin;
        extents.high.SumOf(origin,delta);
        }

    // calculate the distance from the current origin to the back plane along the eye vector
    double dist = (origin.z - extents.low.z);

    // if the distance is negative, the unadjusted backplane is further away from the eye than the back of the model.
    // In that case, we want to bring the backplane in as close as possible to the model to give the best resolution for the z buffer.
    // If the distance is positive, some of the model is clipped by the current frustum. Does he want that - check "noBackClip" flag.
    if (viewController.GetViewFlags().noBackClip)
        {
        origin.z -= dist;
        delta.z  += dist;
        }

    // get distance from (potentially moved) origin to front plane.
    double newDeltaZ = std::max(extents.high.z - origin.z, 100. * DgnUnits::OneMillimeter());
    if (viewController.GetViewFlags().noFrontClip)
        {
        delta.z = newDeltaZ;
        }

    DVec3d  zVec;
    m_rotMatrix.GetRow(zVec, 2);

    double maxDepth = PhysicalViewController::CalculateMaxDepth(delta, zVec);
    double minDepth = std::max(std::max(delta.x, delta.y),(DgnUnits::OneMillimeter() * 150.)); // About 6 inches...

    if (minDepth > maxDepth)
        minDepth = maxDepth;

    if (minDepth > delta.z)     // expand depth to extents
        {
        double diff = (minDepth - delta.z) / 2.0;
        if (viewController.GetViewFlags().noBackClip)
            {
            origin.z -= diff;
            delta.z  += diff;
            }

        if (viewController.GetViewFlags().noFrontClip)
            {
            delta.z += diff;
            }
        }

    // we can't allow the z dimension to be too large relative to x/y. Otherwise the transform math fails due to precision errors.
    if (delta.z > maxDepth)
        {
        double diff = (maxDepth - delta.z) / 2.0;

        origin.z -= diff;
        delta.z  = maxDepth;
        }

    m_rotMatrix.MultiplyTranspose(origin);
    }

/*---------------------------------------------------------------------------------**//**
* set up this viewport from the given viewController
* @bsimethod                                                    KeithBentley    04/02
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus DgnViewport::_SetupFromViewController()
    {                                               
    ViewControllerP   viewController = m_viewController.get();
    if (nullptr == viewController)
        return ViewportStatus::InvalidViewport;

    _AdjustAspectRatio(*viewController, false);

    DPoint3d origin = viewController->GetOrigin();
    DVec3d   delta  = viewController->GetDelta();

    m_rotMatrix     = viewController->GetRotation();
    m_rootViewFlags = viewController->GetViewFlags();
    m_is3dView      = false;
    m_isCameraOn    = false;
    m_viewOrg       = m_viewOrgUnexpanded   = origin;
    m_viewDelta     = m_viewDeltaUnexpanded = delta;
    m_zClipAdjusted = false;

    PhysicalViewControllerP physicalView = GetPhysicalViewControllerP();
    if (nullptr != physicalView)
        {
        CameraViewControllerP cameraView = GetCameraViewControllerP();
        if (!Allow3dManipulations())
            {
            // we're in a "2d" view of a physical model. That means that we must have our orientation with z out of the screen with z=0 at the center.
            AlignWithRootZ(); // make sure we're in a z Up view

            DRange3d  extents = m_viewController->GetViewedExtents();
            if (extents.IsEmpty())
                {
                extents.low.z = -DgnUnits::OneMillimeter();
                extents.high.z = DgnUnits::OneMillimeter();
                }

            double zMax = std::max(fabs(extents.low.z), fabs(extents.high.z));
            zMax = std::max(zMax, DgnUnits::OneMillimeter()); // make sure we have at least +-100. Data may be purely planar
            delta.z  = 2.0 * zMax;
            origin.z = -zMax;
            }
        else
            {
            m_is3dView = true;
            m_isCameraOn = false;
            if (cameraView)
                {
                m_isCameraOn = cameraView->IsCameraOn();
                m_camera = cameraView->GetControllerCamera();

                if (m_isCameraOn)
                    {
                    validateCamera(*cameraView);
                    }
                }

            _AdjustZPlanesToModel(origin, delta, *viewController);

            // if we moved the z planes, set the "zClipAdjusted" flag.
            if (!origin.IsEqual(m_viewOrgUnexpanded) || !delta.IsEqual(m_viewDeltaUnexpanded))
                {
                m_zClipAdjusted = true;

                if (m_isCameraOn)
                    {
                    // don't let the front clip move past camera
                    DVec3d cameraDir;
                    cameraDir.DifferenceOf(m_camera.GetEyePoint(), origin);
                    m_rotMatrix.Multiply(cameraDir);

                    if (delta.z > cameraDir.z - DgnUnits::OneMillimeter())
                        delta.z = cameraDir.z - DgnUnits::OneMillimeter();
                    }
                }
            }
        }
    else
        {
        AlignWithRootZ();

        delta.z  =  200. * DgnUnits::OneMillimeter();
        origin.z = -100. * DgnUnits::OneMillimeter();
        }

    m_viewOrg   = origin;
    m_viewDelta = delta;

    if (!m_renderTarget.IsValid())
        return ViewportStatus::InvalidViewport;

    double compressionFraction;
    if (SUCCESS != RootToNpcFromViewDef(m_rootToNpc, &compressionFraction, IsCameraOn() ? &m_camera : nullptr, m_viewOrg, m_viewDelta, m_rotMatrix))
        return  ViewportStatus::InvalidViewport;

    DPoint3d rootBox[NPC_CORNER_COUNT];
    NpcToWorld(rootBox, s_NpcCorners, NPC_CORNER_COUNT);

    DMap4d      npcToView;
    CalcNpcToView(npcToView);
    m_rootToView.InitProduct(npcToView, m_rootToNpc);

    _SetFrustumFromRootCorners(rootBox, compressionFraction);
    m_needSynchWithViewController = false;

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
    if (nullptr == viewController || !m_frustumValid)
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
    if (nullptr == viewController)
        return  ViewportStatus::InvalidViewport;

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    if (!m_deviceAssigned)
        _SetupFromViewController();
#endif

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
        // get the view extents 
        delta.z = viewController->GetDelta().z;

        // make sure its not too big or too small 
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
        RootToNpcFromViewDef(ueRootToNpc, nullptr, IsCameraOn() ? &m_camera : nullptr, m_viewOrgUnexpanded, m_viewDeltaUnexpanded, m_rotMatrix);

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
        return GetCameraTarget(); // if there are no elements in the view and the camera is on, use the camera target point

    return DPoint3d::FromInterpolate(NpcToWorld(DPoint3d::From(0.5,0.5,low)), 0.5, NpcToWorld(DPoint3d::From(0.5,0.5,high)));
    }

/*---------------------------------------------------------------------------------**//**
* scroll the view by a given number of pixels.
* Camera position is unchanged.
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus DgnViewport::Scroll(Point2dCP screenDist) // => distance to scroll in pixels
    {
    ViewControllerP   viewController = m_viewController.get();
    if (nullptr == viewController)
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
    double npcZ = WorldToNpc(GetCameraTarget()).z;

    if (npcZ < 0.0 || npcZ > 1.0)
        npcZ = WorldToNpc(DPoint3d::FromInterpolate(NpcToWorld(DPoint3d::From(0.5,0.5,1.0)), 0.5, NpcToWorld(DPoint3d::From(0.5,0.5,0.0)))).z;

    return npcZ;
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
    if (nullptr == viewController)
        return ViewportStatus::InvalidViewport;

    CameraViewControllerP cameraView = GetCameraViewControllerP();
    if (cameraView && cameraView->IsCameraOn())
        {
        DPoint3d centerNpc;          // center of view in npc coords
        centerNpc.Init(.5, .5, .5);

        DPoint3d    newCenterNpc;       // get new center of view in npc coords
        if (nullptr != newCenterRoot)
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

    DPoint3d center = (nullptr != newCenterRoot) ? *newCenterRoot : viewController->GetCenter();

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

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/02
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::SetSymbologyRgb(ColorDef lineColor, ColorDef fillColor, int lineWidth, int lineCodeIndex)
    {
    m_renderTarget->SetSymbology(lineColor, fillColor, lineWidth, _GetIndexedLinePattern(lineCodeIndex));
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef DgnViewport::AdjustColorForContrast(ColorDef thisColor, ColorDef againstColor) const
    {
    return ColorUtil::AdjustForContrast(thisColor, againstColor, GetBackgroundColor());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/02
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef DgnViewport::MakeTransparentIfOpaque(ColorDef color, int transparency)
    {
    // if it already has a transparency, leave it alone.
    if (0 != color.GetAlpha())
        return color;

    return MakeColorTransparency(color, transparency);
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
    ColorDef backgroundColorDef, inColorDef, outColorDef;

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
double DgnViewport::GetPixelSizeAtPoint(DPoint3dCP rootPtP, DgnCoordSystem coordSys) const 
    {
    DPoint3d    rootTestPt;

    if (nullptr == rootPtP) // can be nullptr - if so, use center of view
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
        value = DgnViewport::GetMinViewDelta();
        error = lowErr;
        }
    else if (value > DgnViewport::GetMaxViewDelta())
        {
        value = DgnViewport::GetMaxViewDelta();
        error = highErr;
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
    ColorDef bgColor = GetBackgroundColor();
    bool    invert  = ((bgColor.GetRed() + bgColor.GetGreen() + bgColor.GetBlue()) > (255*3)/2);
    return  invert ? ColorDef::Black()  : ColorDef::White();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::_SynchWithViewController(bool saveInUndo)
    {
    _SetupFromViewController();

    if (saveInUndo)
        {
        CheckForChanges();
        _SynchViewTitle();
        }
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
    if (nullptr != m_toolGraphicsHandler)
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
        if (nullptr != acs)
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
ColorDef DgnViewport::_GetBackgroundColor() const
    {
    if (!m_viewController.IsValid())
        return ColorDef::Black();

    ColorDef bgColor = m_viewController->GetBackgroundColor();

    // If background color resolved to be black, and user wants inverted, we set background color to white
    if (ColorDef::Black() == bgColor && GetRenderTarget()->_WantInvertBlackBackground())
        bgColor = ColorDef::White();

    return bgColor;
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::CheckForChanges()
    {
    if (!m_undoActive)
        return;

    Json::Value json;
    m_viewController->SaveToSettings(json);
    Utf8String curr = Json::FastWriter::ToString(json);

    if (m_currentBaseline.empty())
        {
        m_currentBaseline = curr;
        return;
        }

    if (curr.Equals(m_currentBaseline))
        return; // nothing changed

    if (m_backStack.size() >= m_maxUndoSteps)
        m_backStack.pop_front();

    m_backStack.push_back(m_currentBaseline);
    m_forwardStack.clear();

    // now update our baseline to match the current settings.
    m_currentBaseline = curr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnViewport::UpdateView(FullUpdateInfo& info)
    {
    if (!IsActive() || !IsVisible())
        return false;

    ClearProgressiveDisplay();

    CreateSceneContext sceneContext(*m_renderTarget->_GetMainScene());

    GetViewControllerR().OnFullUpdate(*this, sceneContext);
    sceneContext.CreateScene(*this);

    InitViewSettings(true);
    m_renderTarget->_GetMainScene()->Create(); // TEMPORARY - should be on other thread
    m_renderTarget->_GetMainScene()->Paint(); // TEMPORARY - should be on other thread

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/08
+---------------+---------------+---------------+---------------+---------------+------*/
UpdateAbort DgnViewport::UpdateViewDynamic(DynamicUpdateInfo& info)
    {
    ClearProgressiveDisplay();

    InitViewSettings(true);
    CreateSceneContext sceneContext(*m_renderTarget->_GetMainScene());
    GetViewControllerR().OnDynamicUpdate(*this, sceneContext, info);
    m_renderTarget->_GetMainScene()->Paint(); // TEMPORARY - should be on other thread

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    // Let BeSQLite detect operations that may block on a range tree query.
    BeSQLite::wt_GraphicsAndQuerySequencerDiagnosticsEnabler highPriorityRequired;

    if (!IsActive())
        return UpdateAbort::BadView;

    CreateSceneContext sceneContext(*m_renderTarget->_GetMainScene());
    GetViewControllerR().OnDynamicUpdate(*this, sceneContext, info);
    sceneContext.CreateScene(*this);

    InitViewSettings(true);
    m_renderTarget->_GetMainScene()->Render(); // TEMPORARY - should be on other thread

    UpdateContext context;
    return context._DoDynamicUpdate(*this, info);
#endif
    return UpdateAbort::None;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::_CallDecorators(bool& stopFlag)
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    BSIRect rect = GetViewRect();
    m_output->BeginDecorating(&rect);

    // first let the viewController decorate with the z-buffer on
    stopFlag = m_viewController->_DrawZBufferedDecorations(*this);

    if (!stopFlag)
        stopFlag = t_viewHost->GetViewManager().CallElementDecorators(this);

    m_output->BeginOverlayMode();

    DrawGrid();

    // first let the viewController decorate itself
    stopFlag = m_viewController->_DrawOverlayDecorations(*this);

    if (!stopFlag)
        stopFlag = t_viewHost->GetViewManager().CallDecorators(this);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::ClearUndo()
    {
    m_currentBaseline.clear();
    m_forwardStack.clear();
    m_backStack.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::ChangeViewController(ViewControllerR viewController)
    {

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    // first discard the current ViewController.
    // save undo state.
    bool undoActive = IsUndoActive();
    _Destroy();
    _Initialize(viewController);
#endif

    ClearUndo();

    m_viewController = &viewController;
    viewController._OnAttachedToViewport(*this);

    SetupFromViewController();
    }

