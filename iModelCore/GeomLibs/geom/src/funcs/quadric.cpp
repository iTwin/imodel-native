/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
| Simple (canonical) quadric coordinate system calculations.            |
| This is the really simple stuff -- points in, points out.             |
| Keep curve/surface data structures elsewhere!!!                       |
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <math.h>
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


/*======================================================================+
|                                                                       |
|   Major Public Code Section                                           |
|                                                                       |
+======================================================================*/
/*-----------------------------------------------------------------*//**
* Evaluate XT*sigma*Y for sigma=diag(1,1,1,-1)
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double     bsiQuadric_sphereProduct

(
DPoint4dCP pX,
DPoint4dCP pY
)
    {
    return    pX->x * pY->x
            + pX->y * pY->y
            + pX->z * pY->z
            - pX->w * pY->w;
    }

/*-----------------------------------------------------------------*//**
* Evaluate XT*sigma*Y for sigma=diag(1,1,0,-r^2)
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double     bsiQuadric_circleProduct

(
DPoint4dCP pX,
DPoint4dCP pY,
double    r
)
    {
    return    pX->x * pY->x
            + pX->y * pY->y
            - r*r*pX->w * pY->w;
    }
/*-----------------------------------------------------------------*//**
* Evaluate XT*sigma*Y for sigma=diag(1,1,0,-1)
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double     bsiQuadric_cylinderProduct

(
DPoint4dCP pX,
DPoint4dCP pY
)
    {
    return    pX->x * pY->x
            + pX->y * pY->y
            - pX->w * pY->w;
    }

/*-----------------------------------------------------------------*//**
* Evaluate XT*sigma*Y for sigma=diag(1,1,-1,0)
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double     bsiQuadric_coneProduct

(
DPoint4dCP pX,
DPoint4dCP pY
)
    {
    return    pX->x * pY->x
            + pX->y * pY->y
            - pX->z * pY->z;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiQuadric_cylinderToCartesian                          |
|                                                                       |
| author        EarlinLutz                              07/97           |
|                                                                       |
| Convert (theta, z, r, w) to (r*w*cos, r*w**sin, w*z, w)               |
+----------------------------------------------------------------------*/
/*-----------------------------------------------------------------*//**
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_cylinderToCartesian

(
DPoint4dP pXYZ,
DPoint4dCP pR
)
    {
    double theta = pR->x;
    double z = pR->y;
    double r = pR->z;
    double w = pR->w;
    double rw = r * w;
    double c = cos (theta);
    double s = sin (theta);
    StatusInt status = SUCCESS;
    if (w == 0.0)
        {
        pXYZ->x = r * c;
        pXYZ->y = r * s;
        pXYZ->z = z;
        pXYZ->w = 0.0;
        status = ERROR;
        }
    else
        {
        pXYZ->x = rw * c;
        pXYZ->y = rw * s;
        pXYZ->z = w * z;
        pXYZ->w = w;
        }
    return status;
    }

/*-----------------------------------------------------------------*//**
* Evaluate (homogeneous) partial derivatives for a unit cone along the z axis.
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_conePartials

(
DPoint4dP pPartial0,
DPoint4dP pPartial1,
DPoint4dP pPartial2,
DPoint4dP pPartial3,
DPoint4dCP pR
)
    {
    double theta = pR->x;
    double alpha = pR->y;
    double r = pR->z;
    double w = pR->w;
    double rw = r * w;
    double aw = alpha * w;
    double ar = alpha * r;
    double arw = alpha * r * w;
    double c = cos (theta);
    double s = sin (theta);

    StatusInt status = SUCCESS;

    if (pPartial0)
        {
        pPartial0->x = -arw * s;
        pPartial0->y =  arw * c;
        pPartial0->z = 0.0;
        pPartial0->w = 0.0;
        }

    if (pPartial1)
        {
        pPartial1->x = rw * c;
        pPartial1->y = rw * s;
        pPartial1->z = w;
        pPartial1->w = 0.0;
        }

    if (pPartial2)
        {
        pPartial2->x = aw * c;
        pPartial2->y = aw * s;
        pPartial2->z = 0.0;
        pPartial2->w = 0.0;
        }

    if (pPartial3)
        {
        pPartial3->x = ar * c;
        pPartial3->y = ar * s;
        pPartial3->z = alpha;
        pPartial3->w = 1.0;
        }

    return status;
    }

/*-----------------------------------------------------------------*//**
* Evaluate (homogeneous) partial derivatives for a unit cylinder along
* the z axis.
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_cylinderPartials

(
DPoint4dP pPartial0,
DPoint4dP pPartial1,
DPoint4dP pPartial2,
DPoint4dP pPartial3,
DPoint4dCP pR
)
    {
    double theta = pR->x;
    double alpha = pR->y;
    double r = pR->z;
    double w = pR->w;
    double rw = r * w;
    double c = cos (theta);
    double s = sin (theta);

    StatusInt status = SUCCESS;

    if (pPartial0)
        {
        pPartial0->x = -rw * s;
        pPartial0->y =  rw * c;
        pPartial0->z = 0.0;
        pPartial0->w = 0.0;
        }

    if (pPartial1)
        {
        pPartial1->x = 0.0;
        pPartial1->y = 0.0;
        pPartial1->z = w;
        pPartial1->w = 0.0;
        }

    if (pPartial2)
        {
        pPartial2->x = w * c;
        pPartial2->y = w * s;
        pPartial2->z = 0.0;
        pPartial2->w = 0.0;
        }

    if (pPartial3)
        {
        pPartial3->x = r * c;
        pPartial3->y = r * s;
        pPartial3->z = alpha;
        pPartial3->w = 1.0;
        }

    return status;
    }

/*-----------------------------------------------------------------*//**
* Evaluate (homogeneous) partial derivatives for a unit sphere.
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_spherePartials

(
DPoint4dP pPartial0,
DPoint4dP pPartial1,
DPoint4dP pPartial2,
DPoint4dP pPartial3,
DPoint4dCP pR
)
    {
    double theta = pR->x;
    double phi   = pR->y;
    double r = pR->z;
    double w = pR->w;
    double rw = r * w;
    double C = cos (theta);
    double S = sin (theta);
    double c = cos (phi);
    double s = sin (phi);

    StatusInt status = SUCCESS;

    if (pPartial0)
        {
        pPartial0->x = -rw * S * c;
        pPartial0->y =  rw * C * c;
        pPartial0->z = 0.0;
        pPartial0->w = 0.0;
        }

    if (pPartial1)
        {
        pPartial1->x = -rw * C * s;
        pPartial1->y = -rw * S * s;
        pPartial1->z = rw * c;
        pPartial1->w = 0.0;
        }

    if (pPartial2)
        {
        pPartial2->x = w * C * c;
        pPartial2->y = w * S * c;
        pPartial2->z = w * s;
        pPartial2->w = 0.0;
        }

    if (pPartial3)
        {
        pPartial3->x = r * C * c;
        pPartial3->y = r * S * c;
        pPartial3->z = r * s;
        pPartial3->w = 1.0;
        }

    return status;
    }

/*-----------------------------------------------------------------*//**
* Evaluate (homogeneous) partial derivatives for torus with primary
* radius 1 in the xy plane, secondary radius b.
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_torusPartials

(
DPoint4dP pPartial0,
DPoint4dP pPartial1,
DPoint4dP pPartial2,
DPoint4dP pPartial3,
DPoint4dCP pR,
double    b
)
    {
    double theta = pR->x;
    double phi   = pR->y;
    double rho   = pR->z;
    double w     = pR->w;
    StatusInt       status = SUCCESS;

    double rhob = rho * b;
    double cosPhi    = cos (phi);
    double sinPhi    = sin (phi);
    double cosTheta = cos (theta);
    double sinTheta = sin (theta);
    double rxy = 1.0 + rhob * cosPhi;

    double xx = cosTheta * rxy;
    double yy = sinTheta * rxy;
    double zz = rhob * sinPhi;

    if (pPartial3)
        {
        pPartial3->x = xx;
        pPartial3->y = yy;
        pPartial3->z = zz;
        pPartial3->w = 1.0;
        }

    if (w != 0.0)
        {
        xx *= w;
        yy *= w;
        zz *= w;
        rhob *= w;
        }

    if (pPartial0)
        {
        pPartial0->x = -yy;
        pPartial0->y = xx;
        pPartial0->z = 0.0;
        pPartial0->w = 0.0;
        }

    if (pPartial1)
        {
        pPartial1->x = -cosTheta * zz;;
        pPartial1->y = -sinTheta * zz;
        pPartial1->z = rhob * cosPhi;
        pPartial1->w = 0.0;
        }

    if (pPartial2)
        {
        pPartial2->x = cosTheta * b * cosPhi;
        pPartial2->y = sinTheta * b * cosPhi;
        pPartial2->z = b * sinPhi;
        pPartial2->w = 0.0;
        if (w != 0.0)
            {
            pPartial2->x *= w;
            pPartial2->y *= w;
            pPartial2->z *= w;
            }
        }

    return status;
    }

/*-----------------------------------------------------------------*//**
* Evaluate the (homogeneous) point on a torus, given latitude, longitude,
*    radius (from major hoop, multiple of minor hoop radius).
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_torusToCartesian

(
DPoint4dP pXYZ,
DPoint4dCP pR,
double    hoopRadius
)
    {
    double theta = pR->x;
    double phi   = pR->y;
    double rho   = pR->z;
    double w     = pR->w;
    StatusInt       status = SUCCESS;

    double rhor = rho * hoopRadius;
    double c    = cos (phi);
    double s    = sin (phi);

    pXYZ->x = cos(theta) * ( 1.0 + rhor * c);
    pXYZ->y = sin(theta) * ( 1.0 + rhor * c);
    pXYZ->z = rhor * s;

    if (w != 0.0)
        {
        pXYZ->x *= w;
        pXYZ->y *= w;
        pXYZ->z *= w;
        pXYZ->w  = w;
        }
    else
        {
        pXYZ->w = 0.0;
        }
    return status;
    }

/*-----------------------------------------------------------------*//**
* Convert (theta, phi, r, w) to (w*r*cos*COS, w*r*sin*COS, w*r*SIN, w)
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_sphereToCartesian

(
DPoint4dP pXYZ,
DPoint4dCP pR
)
    {
    double theta = pR->x;
    double phi = pR->y;
    double r = pR->z;
    double w = pR->w;
    double rw = r * w;
    double c = cos (theta);
    double s = sin (theta);
    double C = cos (phi);
    double S = sin (phi);
    StatusInt status = SUCCESS;
    if (w == 0.0)
        {
        pXYZ->x = r * c * C;
        pXYZ->y = r * s * C;
        pXYZ->z = r * S;
        pXYZ->w = 0.0;
        status = ERROR;
        }
    else
        {
        pXYZ->x = rw * c * C;
        pXYZ->y = rw * s * C;
        pXYZ->z = rw * S;
        pXYZ->w = w;
        }
    return status;
    }

/*-----------------------------------------------------------------*//**
* Convert (r*w*cos, r*w*sin, w*z, w) to  (theta, z, r, 1)
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_cartesianToCylinder

(
DPoint4dP pR,
DPoint4dCP pXYZ
)
    {
    double rwc = pXYZ->x;
    double rws = pXYZ->y;
    double wz  = pXYZ->z;
    double w   = pXYZ->w;
    double rc, rs, r, theta, z;
    StatusInt status = SUCCESS;

    if (w == 0.0)
        {
        r = sqrt (rwc * rwc + rws * rws);
        theta = bsiTrig_atan2 (rws, rwc);
        pR->x = theta;
        pR->y = wz;
        pR->z = r;
        pR->w = 0.0;
        status = ERROR;
        }
    else
        {
        rc = rwc / w;
        rs = rws / w;
        r  = sqrt (rc * rc + rs * rs);
        theta = bsiTrig_atan2 (rs, rc);
        z = wz / w;
        pR->x = theta;
        pR->y = z;
        pR->z = r;
        pR->w = 1.0;
        }
    return status;
    }


/*-----------------------------------------------------------------*//**
* Convert (xyzw)=(r*w*c*C, r*w*s*C, r*w*S, w) to  (theta, phi, r, 1)
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_cartesianToSphere

(
DPoint4dP pR,
DPoint4dCP pXYZ
)
    {
    double rwcC = pXYZ->x;
    double rwsC = pXYZ->y;
    double rwS  = pXYZ->z;
    double w    = pXYZ->w;
    double theta, phi, rwC, rw;
    StatusInt status = SUCCESS;

    rwC  = sqrt (rwcC * rwcC + rwsC * rwsC);
    rw  = sqrt (rwC * rwC + rwS * rwS);
    theta       = bsiTrig_atan2 (rwsC, rwcC);
    phi = bsiTrig_atan2 (rwS, rwC);

    pR->x = theta;
    pR->y = phi;

    if (w == 0.0)
        {
        pR->z = rw;
        pR->w = 0.0;
        status = ERROR;
        }
    else
        {
        pR->z = rw / w;
        pR->w = 1.0;
        status = SUCCESS;
        }

    return status;
    }

/*-----------------------------------------------------------------*//**
* Convert
*       (wC(1+rho*r*s), wS(1+rho*r*c),w*rho*r*s,w)
*   to
*       (theta, phi, rho, w)
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_cartesianToTorus

(
DPoint4dP pR,
DPoint4dCP pXYZ,
double     hoopRadius      /* Assumed nonzero.  Assumed positive */
)
    {
    double wX   = pXYZ->x;
    double wY   = pXYZ->y;
    double wrhors  = pXYZ->z;
    double w    = pXYZ->w != 0.0 ? pXYZ->w : 1.0;
    double theta, phi;
    double Q, Qc, onePlusQc, Qs;   /* Q is rho * r */
    StatusInt status = SUCCESS;

    theta = bsiTrig_atan2 (wY, wX);

    onePlusQc = sqrt (wX * wX + wY * wY) / w; /* What if w is negative ? */

    Qc = onePlusQc - 1.0;
    Qs = wrhors / w;

    Q = sqrt (Qc * Qc + Qs * Qs);
    phi = bsiTrig_atan2 ( Qs, Qc);

    pR->x = theta;
    pR->y = phi;
    pR->z = Q / hoopRadius;
    pR->w = pXYZ->w;

    return status;
    }

/*-----------------------------------------------------------------*//**
* Convert (r*w*z*cos, r*w*z*sin, w*z, w) to  (theta, z, r, 1)
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_cartesianToCone

(
DPoint4dP pR,
DPoint4dCP pXYZ
)
    {
    double rwzc = pXYZ->x;
    double rwzs = pXYZ->y;
    double wz   = pXYZ->z;
    double w    = pXYZ->w;
    double r, theta, z;
    StatusInt status = SUCCESS;
    theta = bsiTrig_atan2 (rwzs, rwzc);

    if (w == 0.0)
        {
        r = sqrt (rwzc * rwzc + rwzs * rwzs);
        pR->x = theta;
        pR->y = wz;
        pR->z = r;
        pR->w = 0.0;
        status = ERROR;
        }
    else
        {
        if (wz == 0.0)
            {
            r = sqrt (rwzc * rwzc + rwzs * rwzs) / w;
            z = 0.0;
            }
        else
            {
            r  = sqrt (rwzc * rwzc + rwzs * rwzs) / wz;
            z = wz / w;
            }
        pR->x = theta;
        pR->y = z;
        pR->z = r;
        pR->w = 1.0;
        }
    return status;
    }

/*-----------------------------------------------------------------*//**
* Convert (theta, z, r, w) to (r*w*z*cos, r*w*z*sin, w*z, w)
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_coneToCartesian

(
DPoint4dP pXYZ,
DPoint4dCP pR
)
    {
    double theta = pR->x;
    double z = pR->y;
    double r = pR->z;
    double w = pR->w;

    double rz = r * z;
    double rzw = rz * w;
    double c = cos (theta);
    double s = sin (theta);
    StatusInt status = SUCCESS;
    if (w == 0.0)
        {
        pXYZ->x = rz * c;
        pXYZ->y = rz * s;
        pXYZ->z = z;
        pXYZ->w = 0.0;
        status = ERROR;
        }
    else
        {
        pXYZ->x = rzw * c;
        pXYZ->y = rzw * s;
        pXYZ->z = w * z;
        pXYZ->w = w;
        }
    return status;
    }

static void jmdlQuadric_swapyz

(
DPoint4dP pOut,
DPoint4dCP pIn
)
    {
    if (pIn == pOut)
        {
        double temp = pIn->y;
        pOut->x = pIn->x;
        pOut->y = pIn->z;
        pOut->z = temp;
        pOut->w = pIn->w;
        }
    else
        {
        pOut->x = pIn->x;
        pOut->y = pIn->z;
        pOut->z = pIn->y;
        pOut->w = pIn->w;
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| A "disk" is a cylindrical coordinate system with r,z reversed.        |
+----------------------------------------------------------------------*/


/*-----------------------------------------------------------------*//**
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_diskToCartesian

(
DPoint4dP pXYZ,
DPoint4dCP pR
)
    {
    DPoint4d param;
    jmdlQuadric_swapyz (&param, pR);
    return bsiQuadric_cylinderToCartesian (pXYZ, &param);
    }

/*-----------------------------------------------------------------*//**
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_diskPartials

(
DPoint4dP pPartial0,
DPoint4dP pPartial1,
DPoint4dP pPartial2,
DPoint4dP pPartial3,
DPoint4dCP pR
)
    {
    DPoint4d param;
    jmdlQuadric_swapyz (&param, pR);
    return bsiQuadric_cylinderPartials (pPartial0, pPartial1, pPartial2, pPartial3, &param);
    }

/*-----------------------------------------------------------------*//**
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt  bsiQuadric_cartesianToDisk

(
DPoint4dP pR,
DPoint4dCP pXYZ
)
    {
    DPoint4d param;
    StatusInt status = bsiQuadric_cartesianToCylinder (&param, pXYZ);
    jmdlQuadric_swapyz (pR, &param);
    return status;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
