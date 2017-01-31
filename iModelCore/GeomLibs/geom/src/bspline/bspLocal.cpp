/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bspline/bspLocal.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "msbsplinemaster.h"
#include <stdarg.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     rotMatrix_orthogonalFromZRow (RotMatrixP rotMatrixP, DVec3dCP normalP)
    {
    DVec3d    world, xNormal, yNormal, zNormal;

    world.x = world.y = world.z = 0.0;
    zNormal = *normalP;
    bsiDVec3d_normalizeInPlace ((DVec3dP) &zNormal);

    if ((fabs (zNormal.x) < 0.01) && (fabs (zNormal.y) < 0.01))
        world.y = 1.0;
    else
        world.z = 1.0;

    bsiDVec3d_crossProduct ((DVec3dP)&xNormal, (DVec3dP)&world, (DVec3dCP)&zNormal);
    bsiDVec3d_normalizeInPlace ((DVec3dP)&xNormal);
    bsiDVec3d_crossProduct ((DVec3dP)&yNormal, (DVec3dP)&zNormal, (DVec3dCP)&xNormal);
    bsiDVec3d_normalizeInPlace ((DVec3dP)&yNormal);
    bsiRotMatrix_initFromRowVectors (rotMatrixP, &xNormal, &yNormal, &zNormal);
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
