/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/rctree.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/GeomPrivateApi.h>
#include <math.h>
#include <stdlib.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*----------------------------------------------------------------------+
|                                                                       |
|   macro definitions                                                   |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   Local defines                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   Local type definitions                                              |
|                                                                       |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                       |
|   Private Global variables                                            |
|                                                                       |
+----------------------------------------------------------------------*/


/*----------------------------------------------------------------------+
|                                                                       |
|   Public Global variables                                             |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   External variables                                                  |
|                                                                       |
+----------------------------------------------------------------------*/

/*======================================================================+
|                                                                       |
|   Private Utility Routines                                            |
|                                                                       |
+======================================================================*/

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlRCTree_init                                         |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static void    jmdlRCTree_init

(
RotatedConic_Tree   *pTree
)
    {
    memset (pTree, 0, sizeof (RotatedConic_Tree));
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlRCTree_addPlane                                     |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static StatusInt jmdlRCTree_addPlane

(
RotatedConic_Tree   *pTree,     /* <= tree to receive plane */
int                 *pIndex,    /* <= plane index for subsequent push/pop operations */
DPoint4dCP pPlane     /* => plane to insert */
)
    {
    StatusInt status = ERROR;
    if (pTree->numPlane < RC_MAX_TREEPLANE)
        {
        *pIndex = pTree->numPlane++;
        pTree->hPlane [*pIndex] = *pPlane;
        status = SUCCESS;
        }
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlRCTree_addPlane                                     |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static StatusInt jmdlRCTree_addPlaneComponents

(
RotatedConic_Tree   *pTree,     /* <= tree to receive plane */
int                 *pIndex,    /* <= plane index for subsequent push/pop operations */
double              hx,         /* => plane x component */
double              hy,         /* => plane y component */
double              hz,         /* => plane z component */
double              hw          /* => plane w component */
)
    {
    DPoint4d hPlane;
    bsiDPoint4d_setComponents (&hPlane, hx, hy, hz, hw);
    return jmdlRCTree_addPlane (pTree, pIndex, &hPlane);
    }

/* MAP bsiRCTree_getPlane=Geom.getPlane ENDMAP */

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiRCTree_getPlane
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bsiRCTree_getPlane

(
DPoint4dP pPlane,           /* <= plane from tree data */
const RotatedConic_Tree  *pTree,     /* => tree with containing plane */
int         index                   /* => plane index to retrieve */
)

    {
    StatusInt status = SUCCESS;
    if (0 <= index && index < pTree->numPlane)
        {
        *pPlane = pTree->hPlane[index];
        }
    else
        status = ERROR;
    return status;
    }

#ifdef CompileAll
/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlRCTree_addSurface                                   |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static StatusInt jmdlRCTree_addSurface

(
RotatedConic_Tree   *pTree,     /* <= tree to receive plane */
int                 *pIndex,    /* <= plane index for subsequent push/pop operations */
const RotatedConic  *pSurface   /* => surface to insert */
)
    {
    StatusInt status = ERROR;
    if (pTree->numSurf < RC_MAX_TREESURF)
        {
        *pIndex = pTree->numPlane++;
        pTree->rcSurface [*pIndex] = *pSurface;
        status = SUCCESS;
        }
    return status;
    }
#endif
/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlRCTree_addOpCode                                    |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static StatusInt jmdlRCTree_addOpcode

(
RotatedConic_Tree   *pTree,     /* <= tree to receive plane */
int                 op,         /* => the operation */
int                 index       /* => index (i.e. plane or surface) */
)
    {
    StatusInt status = ERROR;
    int opIndex;
    if (pTree->numOpcode < RC_MAX_TREEOP)
        {
        opIndex = pTree->numOpcode++;
        pTree->opcode [opIndex].op = op;
        pTree->opcode [opIndex].index = index;
        status = SUCCESS;
        }
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlRCTree_addPlanePair
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static StatusInt jmdlRCTree_addPlanePair

(
RotatedConic_Tree   *pTree,     /* <= tree to receive planes */
DPoint3dCP pPoint0,     /* => point on first plane */
DPoint3dCP pPoint1      /* => point on second plane */
)
    {
    DPoint3d uVector;
    double dot0, dot1;
    int index0, index1;
    StatusInt status = ERROR;
    bsiDPoint3d_subtractDPoint3dDPoint3d (&uVector, pPoint1, pPoint0);

    dot0 = bsiDPoint3d_dotProduct (pPoint0, &uVector);
    dot1 = bsiDPoint3d_dotProduct (pPoint1, &uVector);

    if (   SUCCESS == jmdlRCTree_addPlaneComponents (pTree, &index0, uVector.x,  uVector.y,  uVector.z, -dot1)
        && SUCCESS == jmdlRCTree_addPlaneComponents (pTree, &index1,  -uVector.x, -uVector.y, -uVector.z,  dot0)
        && SUCCESS == jmdlRCTree_addOpcode (pTree, RC_TREEOP_PUSH_PLANE, index0)
        && SUCCESS == jmdlRCTree_addOpcode (pTree, RC_TREEOP_PUSH_PLANE, index1)
        && SUCCESS == jmdlRCTree_addOpcode (pTree, RC_TREEOP_AND, 0)
        )
        {
        status = SUCCESS;
        }

    return SUCCESS;
    }

    /* MAP bsiRCTree_initPatchClip=Geom.classifyPoint ENDMAP */

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiRCTree_classifyPoint                         |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt  bsiRCTree_classifyPoint    /* true if point is IN the tree volume */

(
bool                       *pPointIsIn,  /* <= classification */
const RotatedConic_Tree    *pTree,       /* => tree to evaluate */
DPoint4dCP                  pPoint       /* => point to classify */
)
    {
    int i;
    StatusInt status = SUCCESS;
    bool    stack[RC_MAX_TREEOP];
    //bool    classification = false;
    bool    c0, c1;
    double f;
    int index;
    int stackDepth = 0;

    double fs = 1.0;
    if (pPoint->w < 0.0)
        fs = -1.0;
    for (i = 0;i < pTree->numOpcode && status == SUCCESS; i++)
        {
        index = pTree->opcode[i].index;
        switch (pTree->opcode[i].op)
            {
            case RC_TREEOP_PUSH_PLANE:
                f = bsiDPoint4d_dotProduct (&pTree->hPlane[index], pPoint);
                stack[stackDepth++] = f * fs <= 0.0 ? 1 : 0;
                break;
            case RC_TREEOP_PUSH_SURF:
                status = ERROR;
                break;
            case RC_TREEOP_AND:
                if (stackDepth < 2)
                    {
                    status = ERROR;
                    }
                else
                    {
                    stackDepth--;
                    c0 = stack[stackDepth];
                    c1 = stack[stackDepth-1];
                    stack[stackDepth-1] = c0 & c1;
                    }
                break;
            case RC_TREEOP_OR:
                if (stackDepth < 2)
                    {
                    status = ERROR;
                    }
                else
                    {
                    stackDepth--;
                    c0 = stack[stackDepth];
                    c1 = stack[stackDepth-1];
                    stack[stackDepth-1] = c0 | c1;
                    }
                break;
            case RC_TREEOP_PUSH_1:
                stack[stackDepth++] = 1;
                break;
            }
        }

    if (SUCCESS == status && stackDepth == 1)
        {
        *pPointIsIn = stack[0];
        }
    else
        {
        *pPointIsIn = false;
        }
    return status;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiRCTree_initPatchClip                         |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt  bsiRCTree_initPatchClip

(
RotatedConic_Tree   *pTree,     /* <= Tree which clips IN SURFACE points to the patch */
const RotatedConic        *pSurface,  /* => surface to clip around */
bool              xLimits,    /* => true to include planes for limits on the first parameter */
bool              yLimits     /* => true to include planes for limits on the second parameter */
)

    {
    StatusInt status = ERROR;
    double theta0, theta1, phi0, phi1, dTheta, dPhi;
    DPoint3d point0, point1;
    int index;
    double dz;

    jmdlRCTree_init (pTree);

    theta0 = pSurface->parameterRange.low.x;
    theta1 = pSurface->parameterRange.high.x;
    dTheta = theta1 - theta0;

    phi0 = pSurface->parameterRange.low.y;
    phi1 = pSurface->parameterRange.high.y;
    dPhi = phi1 - phi0;

    switch (pSurface->type)
        {
        case RC_Cylinder:
            dz = dPhi > 0.0 ? 1.0 : -1.0;

            if (yLimits)
                {
                jmdlRCTree_addPlaneComponents (pTree, &index, 0.0, 0.0, dz, - dz * phi1);
                jmdlRCTree_addOpcode (pTree, RC_TREEOP_PUSH_PLANE, index);

                jmdlRCTree_addPlaneComponents (pTree, &index, 0.0, 0.0, -dz,  dz * phi0);
                jmdlRCTree_addOpcode (pTree, RC_TREEOP_PUSH_PLANE, index);

                jmdlRCTree_addOpcode (pTree, RC_TREEOP_AND, 0);
                }
            else
                {
                jmdlRCTree_addOpcode (pTree, RC_TREEOP_PUSH_1, 0);
                }

            if (xLimits)
                {
                if (bsiTrig_isAngleFullCircle (dTheta))
                    {
                    /* All done !! */
                    }
                else
                    {
                    /* Cut the cylinder plane with an arc */
                    double thetaMid = 0.5 * (theta0 + theta1);
                    /* Sine and cosine of the angle pointing out the back end of the cylinder */
                    double c = -cos (thetaMid);
                    double s = -sin (thetaMid);
                    double c0 = cos (theta0);
                    double s0 = sin (theta0);
                    jmdlRCTree_addPlaneComponents (pTree, &index, c, s, 0.0, - (c0 * c + s0 * s));
                    jmdlRCTree_addOpcode (pTree, RC_TREEOP_PUSH_PLANE, index);
                    jmdlRCTree_addOpcode (pTree, RC_TREEOP_AND, index);
                    }
                }
            status = SUCCESS;
            break;

        case RC_Sphere:
            if (yLimits && phi0 <= -msGeomConst_piOver2 && phi1 >= msGeomConst_piOver2)
                yLimits = false;

            if (xLimits && bsiTrig_isAngleFullCircle (dTheta))
                xLimits = false;

            if (!xLimits && !yLimits)
                {
                jmdlRCTree_addOpcode (pTree, RC_TREEOP_PUSH_1, 0);
                }
            else
                {
                if (yLimits)
                    {
                    bsiDPoint3d_setXYZ (&point0, 0.0, 0.0, sin (phi0));
                    bsiDPoint3d_setXYZ (&point1, 0.0, 0.0, sin (phi1));
                    jmdlRCTree_addPlanePair (pTree, &point0, &point1);
                    }

                 if (xLimits)
                    {
                    double c0, s0, c1, s1;
                    if (dTheta < 0.0)
                        {
                        double temp = theta0;
                        theta0 = theta1;
                        theta1 = temp;
                        dTheta = - dTheta;
                        }

                    /* Direction cosines for vectors normal to each plane,in CCW rotation */
                    c0 = -sin (theta0);
                    s0 =  cos (theta0);
                    c1 = -sin (theta1);
                    s1 =  cos (theta1);

                    if (fabs (dTheta - msGeomConst_pi) < bsiTrig_smallAngle ())
                        {
                        jmdlRCTree_addPlaneComponents (pTree, &index, c1, s1, 0.0, 0.0);
                        jmdlRCTree_addOpcode (pTree, RC_TREEOP_PUSH_PLANE, index);
                        }
                    else if (fabs (dTheta) < msGeomConst_pi)
                        {
                        jmdlRCTree_addPlaneComponents (pTree, &index, -c0, -s0, 0.0, 0.0);
                        jmdlRCTree_addOpcode (pTree, RC_TREEOP_PUSH_PLANE, index);

                        jmdlRCTree_addPlaneComponents (pTree, &index, c1, s1, 0.0, 0.0);
                        jmdlRCTree_addOpcode (pTree, RC_TREEOP_PUSH_PLANE, index);
                        jmdlRCTree_addOpcode (pTree, RC_TREEOP_AND, 0);
                        }
                    else
                        {
                        jmdlRCTree_addPlaneComponents (pTree, &index, -c0, -s0, 0.0, 0.0);
                        jmdlRCTree_addOpcode (pTree, RC_TREEOP_PUSH_PLANE, index);

                        jmdlRCTree_addPlaneComponents (pTree, &index, c1, s1, 0.0, 0.0);
                        jmdlRCTree_addOpcode (pTree, RC_TREEOP_PUSH_PLANE, index);
                        jmdlRCTree_addOpcode (pTree, RC_TREEOP_OR, 0);
                        }
                    }

                 if (xLimits && yLimits)
                    jmdlRCTree_addOpcode (pTree, RC_TREEOP_AND, 0);

                }
            status = SUCCESS;
            break;

        }
    return status;
    }

/* MAP bsiRCTree_transform=Geom.transform ENDMAP */

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiRCTree_transform                                     |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt  bsiRCTree_transform

(
RotatedConic_Tree   *pTree,     /* <= Tree which clips IN SURFACE points to the patch */
DMap4dCP pMap       /* => Map that takes POINTS from their native space into the space where the tree was defined */
)

    {
    int i;

    DMap4d inverseMap;
    StatusInt status = SUCCESS;
    bsiDMap4d_invert (&inverseMap, pMap);
    bsiDMatrix4d_multiplyTransposePoints (&pMap->M0, pTree->hPlane, pTree->hPlane, pTree->numPlane);
    for (i = 0; i < pTree->numSurf; i++)
        {
        bsiRotatedConic_setHMap (&pTree->rcSurface[i], &inverseMap);
        }
    return status;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
