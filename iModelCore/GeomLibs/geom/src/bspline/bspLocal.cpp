/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "msbsplinemaster.h"
#include <stdarg.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE



/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     rotMatrix_orthogonalFromZRow (RotMatrixP rotMatrixP, DVec3dCP normalP)
    {
    DVec3d    world, xNormal, yNormal, zNormal;

    world.x = world.y = world.z = 0.0;
    zNormal = *normalP;
    ((DVec3dP) &zNormal)->Normalize ();

    if ((fabs (zNormal.x) < 0.01) && (fabs (zNormal.y) < 0.01))
        world.y = 1.0;
    else
        world.z = 1.0;

    ((DVec3dP)&xNormal)->CrossProduct (*((DVec3dP)&world), *((DVec3dCP)&zNormal));
    ((DVec3dP)&xNormal)->Normalize ();
    ((DVec3dP)&yNormal)->CrossProduct (*((DVec3dP)&zNormal), *((DVec3dCP)&xNormal));
    ((DVec3dP)&yNormal)->Normalize ();
    rotMatrixP->InitFromRowVectors (xNormal, yNormal, zNormal);
    }




Public StatusInt     SegmentSegmentClosestApproachPoints
(
DPoint3dR approach1,    // point on first segment
DPoint3dR approach2,    // point on second segment
DPoint3dCR    pt1,         /* => vector 1 */
DPoint3dCR    dir1,
DPoint3dCR    pt2,         /* => vector 2 */
DPoint3dCR    dir2,
double testDistance  // call it an error if points are farther apart than this distance
)
    {
    DPoint3d closePoint1, closePoint2;
    double param1, param2;
    double distance;

    if (bsiGeom_closestApproachOfRays (&param1, &param2, &closePoint1, &closePoint2,
                &pt1, &dir1, &pt2, &dir2))
        {
        distance = closePoint1.Distance (closePoint2);
        approach1 = closePoint1;
        approach2 = closePoint2;
        if (distance <= testDistance)
            return SUCCESS;
        }
    return ERROR;
    }




END_BENTLEY_GEOMETRY_NAMESPACE
