/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    RBB                             3/89
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipUtil::GetBoundaryLoops(int *nLoops, int nLoopPoints[], DPoint2d *loopPoints[], DPoint2dCP points, size_t nPoints)
    {
    size_t              i;
    static int          s_minLoopPoints = 3;            // This should perhaps be 4 if we could assume the SCP is always present.
    const DPoint2d      *ptr;

    if (nLoopPoints)
        nLoopPoints[0] = 0;

    if (0 == nPoints)
        {
        *nLoops = 0;
        }
    else
        {
        *nLoops = 1;

        if (loopPoints)
            loopPoints[0] = const_cast<DPoint2d*>(points);

        for (ptr=points, i=0; i<nPoints; i++, ptr++)
            {
            if (ptr->x == DISCONNECT && ptr->y == DISCONNECT)
                {
                bool    havePointInLoop;

                if (! (havePointInLoop = nLoopPoints[*nLoops - 1] >= s_minLoopPoints))      // Fix for TR# 275112. - Loop with only 1 point caused incorrect display..
                    nLoopPoints[*nLoops - 1] = 0;

                if (havePointInLoop || (0 == i))
                    (*nLoops)++;

                if (nLoopPoints)
                    nLoopPoints[*nLoops - 1] = 0;

                if (loopPoints)
                    loopPoints[*nLoops - 1] = const_cast<DPoint2d*>(ptr+1);
                }
            else
                {
                if (nLoopPoints)
                    nLoopPoints[*nLoops - 1]++;
                }
            }

        // if the last loop we encountered had no points, then it's not for real.
        if (nLoopPoints[*nLoops-1] < s_minLoopPoints)                                       // Fix for TR# 275112. - Loop with only 1 point caused incorrect display..
            (*nLoops)--;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    RayBentley                      08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void addPlaneFromPoints(ClipPlaneP pPlanes, int* nPlanes, DPoint3dCR point0, DPoint3dCR point1, DPoint3dCR point2, double expandPlaneDistance)
    {
    DVec3d      normal = DVec3d::FromCrossProductToPoints(point2, point1, point0);

    if (0.0 != normal.Normalize())
        pPlanes[(*nPlanes)++] = ClipPlane(normal, normal.DotProduct(point0) - expandPlaneDistance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    RayBentley                      08/01
+---------------+---------------+---------------+---------------+---------------+------*/
int ClipUtil::RangePlanesFromFrustum(ClipPlaneP pPlanes, FrustumCR frustum, bool clipFront, bool clipBack, double expandPlaneDistance)
    {
    int         nPlanes = 0;

    addPlaneFromPoints(pPlanes, &nPlanes, frustum.m_pts[1], frustum.m_pts[3], frustum.m_pts[5], expandPlaneDistance);                    // Right
    addPlaneFromPoints(pPlanes, &nPlanes, frustum.m_pts[0], frustum.m_pts[4], frustum.m_pts[2], expandPlaneDistance);                    // Left
    addPlaneFromPoints(pPlanes, &nPlanes, frustum.m_pts[2], frustum.m_pts[6], frustum.m_pts[3], expandPlaneDistance);                    // Top
    addPlaneFromPoints(pPlanes, &nPlanes, frustum.m_pts[0], frustum.m_pts[1], frustum.m_pts[4], expandPlaneDistance);                    // Bottom

    if (clipBack)
        addPlaneFromPoints(pPlanes, &nPlanes, frustum.m_pts[0], frustum.m_pts[2], frustum.m_pts[1], expandPlaneDistance);                // Back

    if (clipFront)
        addPlaneFromPoints(pPlanes, &nPlanes, frustum.m_pts[4], frustum.m_pts[5], frustum.m_pts[6], expandPlaneDistance);                // Front

    return nPlanes;
    }

#define HUGE_VALUE                      1.0E14

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    RayBentley                      03/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClipUtil::RayIntersectClipPlanes(DPoint3dCR origin, DVec3dCR direction, ClipPlaneCP pPlane, int nPlanes)
    {
    int         i;
    double      tNear, tFar;

    tNear = -HUGE_VALUE;
    tFar  =  HUGE_VALUE;

    for (i=0; i<nPlanes; i++, pPlane++)
        {
        double     vD = pPlane->DotProduct(direction),
                   vN = pPlane->EvaluatePoint(origin);

        if (0.0 == vD)
            {
            // Ray is parallel... No need to continue testing if outside halfspace.
            if (vN < 0.0)
                return false;
            }
        else
            {
            double  rayDistance = vN  / vD;

            if (vD < 0.0)
                {
                if (rayDistance > tNear)
                    tNear = rayDistance;
                }
            else
                {
                if (rayDistance < tFar)
                    tFar = rayDistance;
                }
            }
        }

    return tNear < tFar;
    }
