/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ClipUtil.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE


//=======================================================================================
// @bsiclass
//=======================================================================================
struct ClipUtil
{
    DGNPLATFORM_EXPORT static double    ComputeFrustumOverlap (DgnViewportR viewport, DPoint3dCP  testFrustumRootPoints);
    DGNPLATFORM_EXPORT static bool      IntersectClipPlaneSets (DRange3dP intersectRange, ClipPlaneCP planeSet1, size_t nPlanes1, ClipPlaneCP planeSet2, size_t nPlanes2);

    DGNPLATFORM_EXPORT static bool RangeInClipPlanes (bool* overlap, DRange3dCR range, bool is3d, ClipPlaneCP, int nPlanes, TransformCP localTransform = NULL);
    DGNPLATFORM_EXPORT static bool PointInsideClipPlanes (DPoint3dCR point, ClipPlaneCP pPlane, int nPlanes);
    DGNPLATFORM_EXPORT static bool RayIntersectClipPlanes (DPoint3dCR origin, DVec3dCR direction, ClipPlaneCP, int nPlanes);
    DGNPLATFORM_EXPORT static int  RangePlanesFromPolyhedra (ClipPlaneP, DPoint3dCP pPolyhedra, bool clipFront, bool clipBack, double expandPlaneDistance);

    DGNPLATFORM_EXPORT static void      GetBoundaryLoops
                                        (
                                        int             *nLoops,            //!< [out] Number of loops 
                                        int             nLoopPoints[],      //!< [out] Number of points in each loop 
                                        DPoint2d        *loopPoints[],      //!< [out] Array of pointers to loop points 
                                        const DPoint2d  *points,            //!< Input points
                                        size_t           nPoints          //!< Number of input points
                                        );

    DGNPLATFORM_EXPORT void             ExtractBoundaryLoops
                                        (
                                        int             *nLoops,            //!< [out] Number of loops 
                                        int             nLoopPoints[],      //!< [out] Number of points in each loop 
                                        DPoint2d        *loopPoints[],      //!< [out] Array of pointers to loop points 
                                        int*            clipMaskP,          //!< [out] Clip mask (for front-back)
                                        double*         zFrontP,            //!< [out] distance to front clip plane.
                                        double*         zBackP,             //!< [out] distance to back clip plane.
                                        TransformP      transformP,         //!< [out] transform (clip to world)
                                        DPoint2dP       pointBuffer,        //!< [in] points buffer
                                        size_t          nPoints             //!< [in] size of point buffer
                                        ) const;


};  // ClipUtil

END_BENTLEY_DGNPLATFORM_NAMESPACE
