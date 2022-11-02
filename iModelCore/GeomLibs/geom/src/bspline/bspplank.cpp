/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "msbsplinemaster.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*
Surface Evaluation function to be called as needed by plank calculations.
@param pSurface IN Surface to evaluate.
@param pUV IN parametric coordinates
@param pXYZ OUT xyz coordinates
@param pdXYZdU OUT partial derivative (tangent) wrt u
@param pdXYZdV OUT partial derivative (tangent) wrt v
@param pd2XYZdUdU OUT second partial wrt u,u
@param pd2XYZdVdV OUT second partial wrt v,v
@param pd2XYZdUdV OUT second partial wrt u,v
*/
static bool    evaluateSurface
(
MSBsplineSurface *pSurface,
DPoint2d *pUV,
DPoint3d *pXYZ,
DPoint3d *pdXYZdU,
DPoint3d *pdXYZdV,
DPoint3d *pd2XYZdUdU,
DPoint3d *pd2XYZdVdV,
DPoint3d *pd2XYZdUdV
)
    {
    bspsurf_computePartials
                    (
                    pXYZ,
                    NULL,
                    pdXYZdU, pdXYZdV, pd2XYZdUdU, pd2XYZdVdV, pd2XYZdUdV,
                    NULL, pUV->x, pUV->y, pSurface);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description A plank curve is (a) is contained within a surface and (b) has no bending around
    the axis perpendicular to the larger plank surface.   This function
    refines the positions of a sequence of points on a surface so that they approximate these
    conditions.
* @param pSurface IN Surface to contain plank path.
* @param pUVOut OUT improved path parameters.  (May be NULL if not needed.)  Caller must
    allocate for numXYZ points.
* @param pXYZOut OUT improved path coordinates. (May be NULL if not neeeded.)  Caller must
    allocate for numXYZ points.
* @param pUVIn IN original path parameters.  If NULL, parameters of pXYZIn will be obtained via
    minimum distance to surface calculation (expensive) from pXYZIn.
* @param pXYZIn IN original path coordinates.  If NULL, will be evaluated (cheap) from pUVIn.
        If both pUVIn and pXYZIn are given, pUVIn is used.
* @param numXYZ IN number of points on paths
* @return SUCCESS if and only if the plank path has been improved
* @group        "B-spline Query"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt mdlBspline_improvePlankPath
(
MSBsplineSurface *pSurface,
DPoint2d *pUVOut,
DPoint3d *pXYZOut,
const DPoint2d *pUVIn,
const DPoint3d *pXYZIn,
int      numXYZ
)
    {
    double distance;

    static double s_uvTol = 1.0e-10;
    //StatusInt status = ERROR;
    DPoint3d xyz;
    if (numXYZ < 3)
        return ERROR;
    ScopedArray<DPoint2d> uvArray(numXYZ);  DPoint2d *pUV  = uvArray.GetData();
    ScopedArray<DPoint3d> xyzArray(numXYZ);  DPoint3d *pXYZ  = xyzArray.GetData();

    int i;

    /* Copy and/or evaluate surface points as needed */
    if (pUVIn)
        {
        for (i = 0; i < numXYZ; i++)
            {
            pUV[i] = pUVIn[i];
            bspsurf_computePartials
                    (
                    &pXYZ[i],
                    NULL,
                    NULL, NULL, NULL, NULL, NULL,
                    NULL, pUV[i].x, pUV[i].y, pSurface);
            }
        }
    else if (pXYZIn)
        {
        for (i = 0; i < numXYZ; i++)
            {
            xyz = pXYZIn[i];
            if (SUCCESS != bsprsurf_minDistToSurface
                    (&distance, &pXYZ[i], &pUV[i], &xyz, pSurface, NULL))
                return ERROR;
            }
        }

    /* Improve coordinates */
    if (!planking_computePathExt (pUV, pXYZ, numXYZ,
                    (PlankingSurfaceEvaluator)evaluateSurface, pSurface, s_uvTol,
                    pSurface->uParams.closed ? 1.0 : 0.0,
                    pSurface->vParams.closed ? 1.0 : 0.0
                    ))
        return ERROR;


    /* Copy to outputs */
    if (pUVOut)
        memcpy (pUVOut, pUV, numXYZ * sizeof (DPoint2d));
    if (pXYZOut)
        memcpy (pXYZOut, pXYZ, numXYZ * sizeof (DPoint3d));

    return SUCCESS;
    }
END_BENTLEY_GEOMETRY_NAMESPACE