/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/gpa/gp_ellipse.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
| The cannonical form of an ellipse in this file is                     |
|     center  = center of the ellipse                                   |
|     vectorU = vector from center to '0 degree' point                  |
|     vectorV = vector from center to '90 degree' point                 |
|     theta0  = start angle (radians)                                   |
|     dtheta  = sweep angle (radians)                                   |
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/GeomPrivateApi.h>
#include    <math.h>
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
|   Public GEOMDLLIMPEXP Global variables                                             |
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
|   Major Public GEOMDLLIMPEXP Code Section                                           |
|                                                                       |
+======================================================================*/

/**
* Strokes a fixed number of points from an ellipse into a GraphicsPointArray
* header.
* @param pHeader <=> header to receive points
* @param pCenter => ellipse center
* @param pVectorU => 0 degree vector
* @param pVectorV => 90 degree vector
* @param theta0 => start angle
* @param delta => sweep angle
* @param n => number of points
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            jmdlEllipse_strokeToGraphicsPoints
(
        GraphicsPointArray  *pHeader,
const   DPoint4d                *pCenter,
const   DPoint4d                *pVectorU,
const   DPoint4d                *pVectorV,
        double                  theta0,
        double                  delta,
        int                     n
)
    {
    int i;
    double dtheta = delta / ( n > 1 ? n - 1 : 1 );
    double theta;
    double c0 = cos(theta0);
    double s0 = sin(theta0);
    double c1 = cos(theta0 + dtheta);
    double s1 = sin(theta0 + dtheta);
    double c2,s2;
    double twok = 2.0*cos(dtheta);
    DPoint4d point;
    for ( i = 0 ; i < n ; i++ )
        {
        theta = theta0 + (double)i * dtheta;
        point.x = pCenter->x + c0 * pVectorU->x + s0 * pVectorV->x;
        point.y = pCenter->y + c0 * pVectorU->y + s0 * pVectorV->y;
        point.z = pCenter->z + c0 * pVectorU->z + s0 * pVectorV->z;
        point.w = pCenter->w + c0 * pVectorU->w + s0 * pVectorV->w;
        jmdlGraphicsPointArray_addDPoint4d ( pHeader, &point );

        /* Recurrence relations for advancing sine and cosine */
        /* These maintain the unit-vector property to 14 digits through 4000
           steps.  Angles may be a bit off.
        */
        c2 = twok*c1 - c0;
        s2 = twok*s1 - s0;
        c0 = c1;
        s0 = s1;
        c1 = c2;
        s1 = s2;
        }
    /* jmdlGraphicsPointArray_markBreak (pHeader);*/
    }




/**
*
* Add a stroked version of each ellipse sector to the GraphicsPoints
* If a positive tolerance is given, it takes precedence over the point
* count.
*
* @param pHeader <=> Header to receive points
* @param pEllipse => ellipse to be stroked
* @param n => max number of points on full ellipse
* @param tol => stroke tolerance
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlDEllipse4d_strokeToGraphicsPoints
(
        GraphicsPointArray  *pHeader,
const   DEllipse4d              *pEllipse,
        int                     n,
        double                  tol
)
    {
    int i;
    int ni;
    double sweep,start;
    n = bsiDEllipse4d_getStrokeCount(pEllipse, n, 4 * n, tol);
    for ( i=0; i< pEllipse->sectors.n; i++ )
        {
        start = pEllipse->sectors.interval[i].minValue;
        sweep = pEllipse->sectors.interval[i].maxValue - start;
        ni = (int) ((double)(n + 1) * ( fabs(sweep) / msGeomConst_2pi));
        if ( ni < 2 )
            ni = 2;
        jmdlEllipse_strokeToGraphicsPoints ( pHeader, &pEllipse->center, &pEllipse->vector0,
                        &pEllipse->vector90, start, sweep, ni );
        }
    }

END_BENTLEY_GEOMETRY_NAMESPACE