/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnViewport.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/02
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::DestroyViewport()
    {
    m_elementProgressiveTasks.clear();
    m_terrainProgressiveTasks.clear();
    RenderQueue().WaitForIdle();
    if (m_viewController.IsValid())
        {
        m_viewController->GetDgnDb().Models().DropGraphicsForViewport(*this);
        m_viewController->GetDgnDb().Elements().DropGraphicsForViewport(*this);        
        m_viewController = nullptr;
        }

    SetRenderTarget(nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    04/2016
//---------------------------------------------------------------------------------------
void DgnViewport::SuspendViewport()
    {
    m_elementProgressiveTasks.clear();
    m_terrainProgressiveTasks.clear();
    SetRenderTarget(nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::InvalidateScene() const
    {
    m_sync.InvalidateScene();

    if (m_viewController.IsValid())
        m_viewController->_InvalidateScene();
    }

/*---------------------------------------------------------------------------------**//**
* Get the DgnCoordSystem::View coordinates of lower-left-back and upper-right-front corners of a viewport.
* NOTE: the y values are "swapped" (llb.y is greater than urf.y) on the screen and and "unswapped" when we plot.
* @bsimethod                                                    KeithBentley    04/02
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d DgnViewport::GetViewCorners() const
    {
    enum Constant
    {
        MINIMUM_WINDOW_DEPTH = -32767,
        MAXIMUM_WINDOW_DEPTH = 32767,
    };

    BSIRect viewRect = GetViewRect();
    DRange3d corners;
    corners.low.x  = viewRect.origin.x;
    corners.high.x = viewRect.corner.x;
    corners.low.y  = viewRect.corner.y;    // y's are swapped on the screen!
    corners.high.y = viewRect.origin.y;
    corners.low.z  = MINIMUM_WINDOW_DEPTH;
    corners.high.z = MAXIMUM_WINDOW_DEPTH;

    return corners;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2012
//--------------+------------------------------------------------------------------------
double DgnViewport::PixelsFromInches(double inches) const
    {
    return GetRenderTarget()->GetDevice()->PixelsFromInches(inches);
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
StatusInt DgnViewport::RootToNpcFromViewDef(DMap4dR rootToNpc, double& frustFraction, CameraInfo const* camera,
                                            DPoint3dCR inOrigin, DPoint3dCR delta, RotMatrixCR viewRot) const
    {
    DVec3d    xVector, yVector, zVector;
    viewRot.GetRows(xVector, yVector, zVector);

    DPoint3d xExtent, yExtent, zExtent;
    DPoint3d origin;

    // Compute root vectors along edges of view frustum.
    if (camera)
        {
        // eye coordinates: 000 at eye
        //      z perpendicular to focal plane (positive z is out back of head)
        //      x,y are horizontal, vertical edges of frustum.

        DVec3d eyeToOrigin = DVec3d::FromStartEnd(camera->GetEyePoint(), inOrigin);      // Subtract camera position (still in root)
        viewRot.Multiply(eyeToOrigin);                                                   // Rotate to view coordinates.

        double focusDistance = camera->GetFocusDistance();
        double zDelta = delta.z;
        double zBack  = eyeToOrigin.z;                                                   // Distance from eye to back clip plane.
        double zFront = zBack + zDelta;                                                  // Distance from eye to front clip plane.
        double minimumFrontToBackClipRatio = GetRenderTarget()->_GetCameraFrustumNearScaleLimit();

        if (zFront / zBack < minimumFrontToBackClipRatio)
            {
            // The ratio between the front and back clipping plane exceeds the resolution of the Z-Buffer.
            // We'll handle this by calculating the front clip from the back clipping plane - but first limit
            // the back clipping distance to 10000 Kilometers. - This make the minimum front clip
            // around a third of a meter and avoids the case where a very large back clip distance
            // causes objects near the camera to disappear.     - RBB 03/2007.

            double maximumBackClip = 10000.* DgnUnits::OneKilometer();
            if (-zBack > maximumBackClip)
                {
                zBack = -maximumBackClip;
                eyeToOrigin.z = zBack;
                }

            zFront = zBack * minimumFrontToBackClipRatio;
            zDelta = zFront - eyeToOrigin.z;
            }

        // z out back of eye ====> origin z coordinates are negative.  (Back plane more negative than front plane)
        double backFraction  = -zBack  / focusDistance;         // Perspective fraction at back clip plane.
        double frontFraction = -zFront / focusDistance;         // Perspective fraction at front clip plane.
        frustFraction = frontFraction / backFraction;

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
    return  SUCCESS;
    }

BEGIN_UNNAMED_NAMESPACE
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
END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* calculate the NPC-to-view transformation matrix.
* @bsimethod                                                    Andrew.Edge     08/04
+---------------+---------------+---------------+---------------+---------------+------*/
DMap4d DgnViewport::CalcNpcToView()
    {
    DRange3d corners = GetViewCorners();
    DMap4d npcToView;
    npcToView.InitFromRanges(s_NpcCorners[NPC_000], s_NpcCorners[NPC_111], corners.low, corners.high);
    return npcToView;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
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
* Adjust the front and back clip planes to include the project extents.
* @bsimethod                                                    KeithBentley    11/02
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::_AdjustZPlanesToModel(DPoint3dR origin, DVec3dR delta, ViewControllerCR viewController) const
    {
    if (!m_is3dView)
        return;

    DPoint3d saveOrg = origin;
    DVec3d saveDelta = delta;
    m_rotMatrix.Multiply(origin);   // put origin into view orientation

    Transform viewTransform;
    viewTransform.InitFrom(m_rotMatrix);

    DRange3d extents = viewController.GetViewedExtents();
    if (!extents.IsEmpty())
        {
        Frustum extFrust(extents);
        extFrust.Multiply(viewTransform);
        extents = extFrust.ToRange();

        origin.z = extents.low.z;
        delta.z = extents.high.z - origin.z;
        }

    delta.z = std::max(delta.z, DgnUnits::OneMeter());
    double maxDelta = std::max(delta.x, delta.y);
    if (maxDelta > delta.z)
        {
        double offset = maxDelta - delta.z;
        origin.z -= offset;
        delta.z += offset*2.0;
        }

    m_rotMatrix.MultiplyTranspose(origin);

    if (!m_isCameraOn)
        return;

    DVec3d cameraDir;
    cameraDir.DifferenceOf(m_camera.GetEyePoint(), origin);
    m_rotMatrix.Multiply(cameraDir);

    if (cameraDir.z < DgnUnits::OneMeter()) // camera is outside project extents, pointed away. Reset to original values.
        {
        origin = saveOrg;
        delta = saveDelta;
        return;
        }

    // set the front plane distance to about 6 inches
//    delta.z = cameraDir.z - (15.2 * DgnUnits::OneCentimeter());
    }

/*---------------------------------------------------------------------------------**//**
* set up this viewport from its viewController
* @bsimethod                                                    KeithBentley    04/02
+---------------+---------------+---------------+---------------+---------------+------*/
ViewportStatus DgnViewport::SetupFromViewController()
    {                                               
    ViewControllerP viewController = m_viewController.get();
    if (nullptr == viewController)
        return ViewportStatus::InvalidViewport;

    _AdjustAspectRatio(*viewController, false);

    DPoint3d origin = viewController->GetOrigin();
    DVec3d   delta  = viewController->GetDelta();

    m_rotMatrix     = viewController->GetRotation();
    m_is3dView      = false;
    m_isCameraOn    = false;
    m_viewOrg       = m_viewOrgUnexpanded   = origin;
    m_viewDelta     = m_viewDeltaUnexpanded = delta;
    m_zClipAdjusted = false;

    SpatialViewControllerP physicalView = GetSpatialViewControllerP();
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

    if (SUCCESS != RootToNpcFromViewDef(m_rootToNpc, m_frustFraction, IsCameraOn() ? &m_camera : nullptr, m_viewOrg, m_viewDelta, m_rotMatrix))
        return  ViewportStatus::InvalidViewport;

    DMap4d npcToView = CalcNpcToView();
    m_rootToView.InitProduct(npcToView, m_rootToNpc);

    m_sync.SetValidController();

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
    if (nullptr == viewController)
        return ViewportStatus::InvalidWindow;

    ViewportStatus validSize = viewController->SetupFromFrustum(inFrustum);

    ViewportStatus status = SetupFromViewController();
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

    SetupFromViewController();

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
        auto stat = ValidateViewDelta(delta, true);
        if (stat != ViewportStatus::Success)
            return stat;

        viewController->SetDelta(delta);

        range.low.z = viewPts[2].z;     // don't change z origin
        DPoint3d origin;
        GetRotMatrix().MultiplyTranspose(&origin, &range.low, 1);
        viewController->SetOrigin(origin);
        }

    SynchWithViewController(true);
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
        double compression;
        RootToNpcFromViewDef(ueRootToNpc, compression, IsCameraOn() ? &m_camera : nullptr, m_viewOrgUnexpanded, m_viewDeltaUnexpanded, m_rotMatrix);

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
    return SetupFromViewController();
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
    ViewportStatus validSize = ValidateViewDelta(delta, false);
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
    return SetupFromViewController();
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
    return GetDefaultIndexedLineWidth(index);
    }

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
static void limitWindowSize(ViewportStatus& error, double& value)
    {
    if (value < DgnViewport::GetMinViewDelta())
        {
        value = DgnViewport::GetMinViewDelta();
        error = ViewportStatus::MinWindow;
        }
    else if (value > DgnViewport::GetMaxViewDelta())
        {
        value = DgnViewport::GetMaxViewDelta();
        error = ViewportStatus::MaxWindow;
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
ViewportStatus DgnViewport::ValidateViewDelta(DPoint3dR delta, bool messageNeeded)
    {
    ViewportStatus  error=ViewportStatus::Success, ignore;

    limitWindowSize(error,  delta.x);
    limitWindowSize(error,  delta.y);
    limitWindowSize(ignore, delta.z);

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
* @bsimethod                                                    RayBentley  10/06
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
void DgnViewport::SynchWithViewController(bool saveInUndo)
    {
    SetupFromViewController();

    if (saveInUndo)
        {
        SaveViewUndo();
        _SynchViewTitle();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/05
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef DgnViewport::GetBackgroundColor() const
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
* @bsimethod                                    Keith.Bentley                   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::ScheduleElementProgressiveTask(ProgressiveTask& task)
    {
    DgnDb::VerifyClientThread(); // this may only be called from the client thread.
    m_elementProgressiveTasks.push_back(&task);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    04/2016
//---------------------------------------------------------------------------------------
void DgnViewport::ScheduleTerrainProgressiveTask(ProgressiveTask& task)
    {
    DgnDb::VerifyClientThread(); // this may only be called from the client thread.
    m_terrainProgressiveTasks.push_back(&task);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::SaveViewUndo()
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

    if ((int)m_backStack.size() >= m_maxUndoSteps)
        m_backStack.pop_front();

    m_backStack.push_back(m_currentBaseline);
    m_forwardStack.clear();

    // now update our baseline to match the current settings.
    m_currentBaseline = curr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::_CallDecorators(DecorateContextR context)
    {
    m_viewController->_DrawDecorations(context);
    m_viewController->_DrawGrid(context);
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
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::ChangeViewController(ViewControllerR viewController)
    {
    if (m_viewController.IsValid())
        {
        bool dropGraphics = true;

        if (m_viewController->GetClassId() == viewController.GetClassId())
            {
            ViewFlags oldFlags = m_viewController->GetViewFlags();
            ViewFlags newFlags = viewController.GetViewFlags();

            // Check for view flag changes that may require us to re-generate cached graphic...
            if (newFlags.GetRenderMode() == oldFlags.GetRenderMode() &&
                newFlags.constructions == oldFlags.constructions &&
                newFlags.text == oldFlags.text &&
                newFlags.dimensions == oldFlags.dimensions &&
                newFlags.fill == oldFlags.fill)
                {
                // Both sub-category visibility and appearance gets baked into cached graphic...
                if (!m_viewController->HasSubCategoryOverride() && !viewController.HasSubCategoryOverride())
                    dropGraphics = false;
                }
            }

        if (dropGraphics)
            {
            m_viewController->GetDgnDb().Models().DropGraphicsForViewport(*this);
            m_viewController->GetDgnDb().Elements().DropGraphicsForViewport(*this);        
            }
        }

    ClearUndo();

    m_viewController = &viewController;
    viewController._OnAttachedToViewport(*this);

    InvalidateScene();
    m_sync.InvalidateController();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Frustum::ScaleAboutCenter(double scale)
    {
    Frustum orig = *this;
    double f = 0.5 * (1.0 + scale);
    m_pts[NPC_000].Interpolate(orig.GetCorner(NPC_111), f, orig.GetCorner(NPC_000));
    m_pts[NPC_100].Interpolate(orig.GetCorner(NPC_011), f, orig.GetCorner(NPC_100));
    m_pts[NPC_010].Interpolate(orig.GetCorner(NPC_101), f, orig.GetCorner(NPC_010));
    m_pts[NPC_110].Interpolate(orig.GetCorner(NPC_001), f, orig.GetCorner(NPC_110));
    m_pts[NPC_001].Interpolate(orig.GetCorner(NPC_110), f, orig.GetCorner(NPC_001));
    m_pts[NPC_101].Interpolate(orig.GetCorner(NPC_010), f, orig.GetCorner(NPC_101));
    m_pts[NPC_011].Interpolate(orig.GetCorner(NPC_100), f, orig.GetCorner(NPC_011));
    m_pts[NPC_111].Interpolate(orig.GetCorner(NPC_000), f, orig.GetCorner(NPC_111));
    }
