/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/gpa/gp_properties.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <stdlib.h>


BEGIN_BENTLEY_GEOMETRY_NAMESPACE
typedef struct {
    EmbeddedDPoint3dArray *pPointArray;
    GraphicsPointArrayCP pGPA;
    const GraphicsPoint *pGPBuffer;
    int numGraphicsPoint;
    DPoint3d refPoint;
    DPoint3d normalSum;
    } PlaneExtractionContext;

/*---------------------------------------------------------------------------------**//**
* Initialize plane computation.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    pec_initContext
(
PlaneExtractionContext *pContext,
GraphicsPointArrayCP pGPA
)
    {
    bool    needPoint;
    int i;
    memset (pContext, 0, sizeof (*pContext));
    pContext->pGPA = pGPA;
    pContext->pGPBuffer = jmdlGraphicsPointArray_getConstPtr (pGPA, 0);
    pContext->numGraphicsPoint = jmdlGraphicsPointArray_getCount (pGPA);
    pContext->pPointArray = jmdlEmbeddedDPoint3dArray_grab ();

    needPoint = true;
    for (i = 0; needPoint && i < pContext->numGraphicsPoint; i++)
        {
        if (bsiDPoint4d_normalize (&pContext->pGPBuffer[i].point, &pContext->refPoint))
            needPoint = false;
        }

    if (needPoint)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Initialize plane computation.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    pec_decommission
(
PlaneExtractionContext *pContext
)
    {
    jmdlEmbeddedDPoint3dArray_drop (pContext->pPointArray);
    }

/*---------------------------------------------------------------------------------**//**
* Direct inspection of polyline as contiguous buffer.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void pec_polylineBuffer
(
PlaneExtractionContext *pContext,
const GraphicsPoint *pBuffer,
int i0,
int i1
)
    {
    DPoint3d point0, point1, cross;
    int i;
    int point0OK = false;
    for (i = i0; i <= i1; i++)
        {
        if (bsiDPoint4d_normalize (&pBuffer[i].point, &point1))
            {
            if (point0OK)
                {
                bsiDPoint3d_crossProduct3DPoint3d (&cross, &pContext->refPoint, &point0, &point1);
                bsiDPoint3d_addDPoint3dDPoint3d (&pContext->normalSum, &pContext->normalSum, &cross);
                }
            point0 = point1;
            point0OK = true;
            jmdlEmbeddedDPoint3dArray_addDPoint3d (pContext->pPointArray, &point1);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Direct inspection of polyline in situ.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void pec_polyline
(
PlaneExtractionContext *pContext,
int i0,
int i1
)
    {
    /* Polyline is easy -- just look at it in place. */
    pec_polylineBuffer (pContext, pContext->pGPBuffer, i0, i1);
    }

/*---------------------------------------------------------------------------------**//**
* Approximate the ellipse by at least 2 chords of at most 45 degrees.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void pec_ellipse
(
PlaneExtractionContext *pContext,
int i0,
int i1
)
    {
    /* Evaluate DPoint4d's to GP buffer, the treat as polyline. */
#define MIN_POINT 3
#define MAX_POINT 9
    DConic4d conic;
    GraphicsPoint gpBuffer[MAX_POINT];
    int readIndex = i0;
    int numPoint;
    int i;
    double theta, dtheta, theta0, sweep;
    if (jmdlGraphicsPointArray_getDConic4d (pContext->pGPA, &readIndex, &conic, &theta0, &sweep, NULL, NULL))
        {
        numPoint = (int)((double)MAX_POINT * (fabs (sweep) / msGeomConst_2pi) + 0.5);

        if (numPoint < MIN_POINT)
            numPoint = MIN_POINT;
        if (numPoint > MAX_POINT)
            numPoint = MAX_POINT;

        dtheta = sweep / (numPoint - 1);
        for (i = 0; i < numPoint; i++)
            {
            theta = theta0 + i * dtheta;
            bsiDConic4d_angleParameterToDPoint4d (&conic, &gpBuffer[i].point, theta);
            }
        pec_polylineBuffer (pContext, gpBuffer, 0, numPoint - 1);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Treat the bezier polygon as a decent approximation to the bezier, at least for
* purpose of extracting the plane.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void pec_bezier
(
PlaneExtractionContext *pContext,
int i0,
int i1
)
    {
    /* Look at the bezier in place as if it is a polyline */
    pec_polylineBuffer (pContext, pContext->pGPBuffer, i0, i1);
    }

/*---------------------------------------------------------------------------------**//**
* Treat the control polygon as a decent approximation to the bezier, at least for
* purpose of extracting the plane.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void pec_bspline
(
PlaneExtractionContext *pContext,
int i0,
int i1
)
    {
    size_t pole0, pole1;
    if (pContext->pGPA->ParseBsplineCurvePoleIndices ((size_t)i0, pole0, pole1))
        {
        pec_polylineBuffer (pContext, pContext->pGPBuffer, (int)pole0, (int)pole1);
        }
    }

static bool    pec_mainLoop
(
PlaneExtractionContext *pContext
)
    {
    int i0, i1, curveType;
    for (i0 = 0;
         jmdlGraphicsPointArray_parseFragment (
                        pContext->pGPA, &i1,
                        NULL, NULL,
                        &curveType, i0);
         i0 = i1 + 1)
        {
        if (curveType == 0)
            {
            pec_polyline (pContext, i0, i1);
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER)    // BSPLINE CODED
            {
            pec_bezier (pContext, i0, i1);
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
            {
            pec_ellipse (pContext, i0, i1);
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BSPLINE)
            {
            pec_bspline (pContext, i0, i1);
            }
        }
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* Get a plane which contains the geometry.  If geometry is non-planar, plane
* is arbitrary but may be close.
* @param pTransform <= transformation whose xy plane is the plane of the geometry.
* @param pMaxDist <= maximum distance to plane.
* @param pDefaultNormal => reference direction to use if data is colinear.
*               The computed normal in this case is
*                   (lineDirection cross defaultNormal) cross defaultNormal
*               i.e the default normal is strictly a default vector in the plane of the
*               line and the normal.
* @param pRange <= optional range of points considered.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlGraphicsPointArray_getPlaneAsTransformExt2
(
GraphicsPointArrayCP pSource,
      Transform              *pTransform,
      double                    *pMaxDist,
      DPoint3dCP                 pDefaultNormal,
      DRange3d                  *pRange
)
    {
    DVec3d normalByPoints;
    DPoint3d originByPoints;
    DVec3d acceptedNormal;
    Transform transform;
    bool    result = false;
    PlaneExtractionContext context;
    DPoint3d minPoint, maxPoint;
    DPoint3d defaultNormal;
    int i;

    if (pDefaultNormal)
        defaultNormal = *pDefaultNormal;
    else
        bsiDPoint3d_setXYZ (&defaultNormal, 0.0, 0.0, 1.0);

    if (pTransform)
        bsiTransform_initIdentity (pTransform);

    if (pMaxDist)
        *pMaxDist = 0.0;

    if (!pec_initContext (&context, pSource))
        goto cleanup;

    /* Initialization suggests first point in the GPA as origin point ... */
    pTransform->SetTranslation (context.refPoint);

    if (!pec_mainLoop (&context))
        goto cleanup;


    if (bsiGeom_planeThroughPoints
                        (
                        &normalByPoints,
                        &originByPoints,
                        jmdlEmbeddedDPoint3dArray_getConstPtr (context.pPointArray, 0),
                        jmdlEmbeddedDPoint3dArray_getCount (context.pPointArray)
                        ))
        {
        /* This is the first choice */
        }
    else if (bsiGeom_findWidelySeparatedPoints
                    (
                    &minPoint, NULL, &maxPoint, NULL,
                    jmdlEmbeddedDPoint3dArray_getConstPtr (context.pPointArray, 0),
                    jmdlEmbeddedDPoint3dArray_getCount (context.pPointArray)
                    ))
        {
        /* This is second choice, but enough to go on. */
        DPoint3d lineDirection, linePerp1;
        bsiDPoint3d_subtractDPoint3dDPoint3d (&lineDirection, &maxPoint, &minPoint);
        if (bsiDPoint3d_areParallel (&lineDirection, &defaultNormal))
            {
            // If the Default normal is parallel to the line, select an arbitrary nornal.
            DPoint3d        yUnused, zUnused;

            bsiDPoint3d_normalizeInPlace (&lineDirection);
            bsiDPoint3d_getTriad (&lineDirection, &normalByPoints, &yUnused, &zUnused);
            }
        else
            {
            bsiDPoint3d_crossProduct (&linePerp1, &lineDirection, &defaultNormal);
            bsiDPoint3d_crossProduct (&normalByPoints, &lineDirection, &linePerp1);
            }
        }
    else
        {
        goto cleanup;
        }

    acceptedNormal = normalByPoints;

    if (bsiDPoint3d_dotProduct (&normalByPoints, &context.normalSum) < 0.0)
         {
         bsiDPoint3d_negateInPlace (&acceptedNormal);
         }

    DVec3d columnX, columnY, columnZ;
    DPoint3d origin;
    if (!acceptedNormal.GetNormalizedTriad (columnX, columnY, columnZ))
        goto cleanup;
    bsiDPoint3d_averageDPoint3dArray
                    (&origin,
                        jmdlEmbeddedDPoint3dArray_getConstPtr (context.pPointArray, 0),
                        jmdlEmbeddedDPoint3dArray_getCount (context.pPointArray)
                        );
    transform.InitFromOriginAndVectors (origin, columnX, columnY, columnZ);
    if (pTransform)
        *pTransform = transform;

    if (pRange)
        jmdlEmbeddedDPoint3dArray_getDRange3d (context.pPointArray, pRange);

    result = true;

    if (pMaxDist)
        {
        const DPoint3d *pBuffer = jmdlEmbeddedDPoint3dArray_getConstPtr (context.pPointArray, 0);
        int count = jmdlEmbeddedDPoint3dArray_getCount (context.pPointArray);
        double z;

        *pMaxDist = 0.0;
        DPoint3d origin;
        DVec3d columnX, columnY, columnZ;
        pTransform->GetOriginAndVectors (origin, columnX, columnY, columnZ);
        for (i = 0; i < count; i++)
            {
            z = fabs (bsiDPoint3d_dotDifference
                            (
                            &pBuffer[i],
                            &origin,
                            &columnZ
                            ));
            if (z > *pMaxDist)
                *pMaxDist = z;
            }
        }

cleanup:
    pec_decommission (&context);
    return result;
    }


/*---------------------------------------------------------------------------------**//**
@description Compute tangent, outward normal, and upward normal vectors at GraphicsPoint.
@param point OUT point on curve
@param xDir OUT forward unit vector on curve
@param yDir OUT perpendicular unit vector
@param zDir OUT normal to plane of curve (see note below ....)
@param i0 IN primitive index.
@param fraction IN fractional parameter within primitive
@param defaultZ IN z vector to apply when primitive is a line and GPA is non planar
@returns true if the curve has a good tangent.
@bsimethod                                    Sam.Wilson                      03/2006
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlGraphicsPointArray_primitiveFractionToFrenetFrame
(
GraphicsPointArrayCP    pGPA,
DPoint3d*                pXYZ, /* <= point on curve */
DVec3d*                  pUnitX,  /* <= tangent */
DVec3d*                  pUnitY,  /* <= in-plane */
DVec3d*                  pUnitZ,  /* <= outward normal */
double*                  pTangentMagnitude,
int                     i0,     // => primitive index
double                  fraction,   // => fraction within primitive
DVec3d const*          pDefaultZ    // Z to use for nonplanar GPA
)
    {
    //  --------------------------------------------------------
    //  point := curve evaluated at parameter
    //  xDir   = tangent @ parameter
    DPoint3d X;
    DVec3d dX, ddX;
    DVec3d xDir, yDir, zDir;
    int j0, j1;
    DPoint3d xyzA;
    DVec3d dXA, xDirA;
    Transform transform;
    double maxVariance;
    DRange3d dRange;
    double da = 0.0;
    double daA = 0.0;
    bool    boolstat = true;    // Multiple successful branches will pick this up.  Changed to false in failure block.
    bsiDVec3d_setXYZ (&xDir, 1,0,0);
    bsiDVec3d_setXYZ (&yDir, 0,1,0);
    bsiDVec3d_setXYZ (&zDir, 0,0,1);
    if (!jmdlGraphicsPointArray_primitiveFractionArrayToDPoint3dDerivatives (pGPA, &X, &dX, &ddX, i0, &fraction, 1))
        goto returnDefaults;
    if (pXYZ)
        *pXYZ = X;
    da = bsiDVec3d_normalize(&xDir, &dX);
    if (da == 0.0)
        goto returnDefaults;

    // We have a good tangent....

    if (  bsiDPoint3d_magnitudeSquared (&ddX) > 0.0)
        {
        // We have a full frenet frame at this point.
        bsiDVec3d_normalizedCrossProduct (&zDir, &xDir, &ddX);
        bsiDVec3d_normalizedCrossProduct (&yDir, &zDir, &xDir);
        if (bsiDVec3d_magnitudeSquared (&yDir) > 0.0)
            goto copyResults;
        }

    // We do not have a good perpendicular.

    // Look enpoint tangent at an adjacent primitive:
    if (    jmdlGraphicsPointArray_parsePrimitiveBefore (pGPA, &j0, &j1, NULL, NULL, NULL, i0)
        && jmdlGraphicsPointArray_primitiveFractionToDPoint3dTangent (pGPA, &xyzA, &dXA, j0, 1.0)
        && (daA = bsiDVec3d_normalize (&xDirA, &dXA)) > 0.0
        )
        {
        bsiDVec3d_normalizedCrossProduct (&zDir, &xDirA, &xDir);
        bsiDVec3d_normalizedCrossProduct (&yDir, &zDir, &xDir);
        if (bsiDVec3d_magnitudeSquared (&zDir) > 0.0)
            goto copyResults;
        }

    if (   jmdlGraphicsPointArray_parsePrimitiveAfter (pGPA, &j0, &j1, NULL, NULL, NULL, i0)
        && jmdlGraphicsPointArray_parsePrimitiveAfter (pGPA, &j0, &j1, NULL, NULL, NULL, j1)
        && jmdlGraphicsPointArray_primitiveFractionToDPoint3dTangent (pGPA, &xyzA, &dXA, j0, 0.0)
        && (daA = bsiDVec3d_normalize (&xDirA, &dXA)) > 0.0
        )
        {
        bsiDVec3d_normalizedCrossProduct (&zDir, &xDir, &xDirA);
        bsiDVec3d_normalizedCrossProduct (&yDir, &zDir, &xDir);
        if (bsiDVec3d_magnitudeSquared (&zDir) > 0.0)
            goto copyResults;
        }

    if (jmdlGraphicsPointArray_getPlaneAsTransformExt2 (pGPA, &transform, &maxVariance, (DVec3d*)pDefaultZ, &dRange)
        && maxVariance < 1.0e-8 * bsiDRange3d_extentSquared (&dRange))
        {
        DPoint3d origin;
        DVec3d columnX, columnY, columnZ;
        transform.GetOriginAndVectors (origin, columnX, columnY, columnZ);
        zDir = columnZ;
        bsiDVec3d_normalizedCrossProduct (&yDir, &zDir, &xDir);
        bsiDVec3d_normalizedCrossProduct (&zDir, &xDir, &yDir);
        if (bsiDVec3d_magnitudeSquared (&zDir) > 0.0)
            goto copyResults;
        }

    // Fallback to default z with tangent ..
    if (pDefaultZ)
        {
        bsiDVec3d_normalizedCrossProduct (&yDir, pDefaultZ, &xDir);
        bsiDVec3d_normalizedCrossProduct (&zDir, &xDir, &yDir);
        if (bsiDVec3d_magnitudeSquared (&zDir) > 0.0)
            goto copyResults;
        }

returnDefaults:
    if (pDefaultZ)
        bsiDVec3d_getNormalizedTriad (pDefaultZ, &xDir, &yDir, &zDir);
    else
        {
        bsiDVec3d_setXYZ (&xDir, 1,0,0);
        bsiDVec3d_setXYZ (&yDir, 0,1,0);
        bsiDVec3d_setXYZ (&zDir, 0,0,1);
        }
    boolstat = false;

copyResults:

    // Copy to optional args and return ...
    if (pUnitX)
        *pUnitX = xDir;
    if (pUnitY)
        *pUnitY = yDir;
    if (pUnitZ)
        *pUnitZ = zDir;

    if (pTangentMagnitude)
        *pTangentMagnitude = da;

    return boolstat;
    }



/*---------------------------------------------------------------------------------**//**
* Get a plane which contains the geometry.  If geometry is non-planar, plane
* is arbitrary but may be close.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_getDPlane3d
(
GraphicsPointArrayCP pSource,
      DPlane3d                  *pPlane
)
    {
    Transform transform;
    bool    result = jmdlGraphicsPointArray_getPlaneAsTransformExt2 (pSource, &transform, NULL, NULL, NULL);
    DPoint3d origin;
    DVec3d columnX, columnY, columnZ;
    transform.GetOriginAndVectors (origin, columnX, columnY, columnZ);
    bsiDPlane3d_initFromOriginAndNormal (pPlane, &origin, &columnZ);
    return result;
    }


/*---------------------------------------------------------------------------------**//**
* Return the highest order required for curves in the array.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      jmdlGraphicsPointArray_getHighestBezierOrder
(
GraphicsPointArrayCP pSource
)
    {
    if (!pSource)
        return 0;
    return pSource->HighestBezierOrder ();
    }

/*---------------------------------------------------------------------------------**//**
* Update min/max values for xyz and w components.  Args assumed non null.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    updateExtrema
(
      double *pxyzMin,
      double *pxyzMax,
      double *pwMin,
      double *pwMax,
const DPoint4d *pPoint
)
    {
    if (pPoint->x < *pxyzMin)
        *pxyzMin = pPoint->x;
    if (pPoint->y < *pxyzMin)
        *pxyzMin = pPoint->y;
    if (pPoint->z < *pxyzMin)
        *pxyzMin = pPoint->z;

    if (pPoint->w < *pwMin)
        *pwMin   = pPoint->w;

    if (pPoint->x > *pxyzMax)
        *pxyzMax = pPoint->x;
    if (pPoint->y > *pxyzMax)
        *pxyzMax = pPoint->y;
    if (pPoint->z > *pxyzMax)
        *pxyzMax = pPoint->z;

    if (pPoint->w > *pwMax)
        *pwMax   = pPoint->w;
    }


/*---------------------------------------------------------------------------------**//**
* Return the minimum and maximum component value (any xyz) and the minimum and maximum weight.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_getComponentRanges
(
GraphicsPointArrayCP pSource,
      double *pxyzMin,
      double *pxyzMax,
      double *pwMin,
      double *pwMax
)
    {
    const GraphicsPoint *pBuffer = jmdlGraphicsPointArray_getConstPtr (pSource, 0);

    int n = jmdlGraphicsPointArray_getCount (pSource);
    int i;
    double xMin, xMax;
    double wMin, wMax;

    if (n == 0)
        {
        xMin = xMax = wMin = wMax = 0.0;
        return false;
        }
    else
        {
        xMin = xMax = pBuffer[0].point.x;
        wMin = wMax = pBuffer[0].point.w;
        for (i = 0; i < n; i++)
            {
            updateExtrema (&xMin, &xMax, &wMin, &wMax, &pBuffer[i].point);
            }
        }
    if (pxyzMin)
        *pxyzMin = xMin;
    if (pxyzMax)
        *pxyzMax = xMax;

    if (pwMin)
        *pwMin = wMin;
    if (pwMax)
        *pwMax = wMax;
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* Compute tolerances for xyz and w parts of the graphics point array.
* The xyz tolerance is absTol + relTol * (fabs(xyzmax) + fabs (xyzmin))
* The w tolerance is absTol + relTol * (fabs(wmax) + fabs (wmin))
*  where the min and max values are obtained by scanning the array.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_getTolerances
(
GraphicsPointArrayCP pSource,
      double *pxyzTol,
      double *pwTol,
      double xyzAbsTol,
      double xyzRelTol,
      double wAbsTol,
      double wRelTol
)
    {
    double xyzMin, xyzMax, wMin, wMax;
    if (jmdlGraphicsPointArray_getComponentRanges (pSource, &xyzMin, &xyzMax, &wMin, &wMax))
        {
        if (pxyzTol)
            *pxyzTol = xyzAbsTol + xyzRelTol * (fabs (xyzMax) + fabs (xyzMin));
        if (pwTol)
            *pwTol = wAbsTol     + wRelTol *   (fabs (wMax)   + fabs (wMin));
        return true;
        }
    return false;
    }



/*---------------------------------------------------------------------------------**//**
* Compute the total length of the curves in the array.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   jmdlGraphicsPointArray_getLength
(
GraphicsPointArrayCP pSource
)
    {
    double length = 0.0;
    if (pSource)
        length = pSource->CurveLength ();
    return length;
    }

/*---------------------------------------------------------------------------------**//**
* Compute the total length of the curves in the array.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   jmdlGraphicsPointArray_getQuickLength
(
GraphicsPointArrayCP pSource
)
    {
    double length = 0.0;
    if (pSource)
        length = pSource->CurveLength ();
    return length;
    }



#define NUM_PATH_CENTROID_INTEGRALS 10

#define INDEX_X 0
#define INDEX_Y 1
#define INDEX_Z 2

#define INDEX_LENGTH 3

#define INDEX_XX 4
#define INDEX_XY 5
#define INDEX_XZ 6
#define INDEX_YY 7
#define INDEX_YZ 8
#define INDEX_ZZ 9

typedef struct
    {
    bool    bFixedZ;     // If true, force geometry to origin z.
    bool    bEnableProducts;
    double sum[NUM_PATH_CENTROID_INTEGRALS];
    DPoint3d origin;
    } PathIntegralContext;


#define NUM_MOMENT_INTEGRALS 3
#define INDEX_xdA 0
#define INDEX_ydA 1
#define INDEX_dA 2

typedef struct
    {
    Transform worldToLocal;
    double sum[NUM_MOMENT_INTEGRALS];
    } AreaIntegralContext;

typedef struct
    {
    int numPole;
    DPoint4d poleArray[MAX_BEZIER_CURVE_ORDER];
    DPoint3d origin;
    } BezierContext;

typedef struct
    {
    DConic4d conic;
    DPoint3d origin;
    } ConicContext;

static void setPathIntegrands
(
double *pF,
int     numFunc,
double a,
const DPoint3d *pPoint,
const DPoint3d *pOrigin
)
    {
    double x = pPoint->x - pOrigin->x;
    double y = pPoint->y - pOrigin->y;
    double z = pPoint->z - pOrigin->z;

    pF[INDEX_LENGTH] = a;
    pF[INDEX_X]      = a * x;
    pF[INDEX_Y]      = a * y;
    pF[INDEX_Z]      = a * z;

    if (numFunc == 10)
        {
        pF[INDEX_XX]     = a * x * x;
        pF[INDEX_YY]     = a * y * y;
        pF[INDEX_ZZ]     = a * z * z;

        pF[INDEX_XY]     = a * x * y;
        pF[INDEX_XZ]     = a * x * z;
        pF[INDEX_YZ]     = a * y * z;
        }
    }
/*---------------------------------------------------------------------------------**//**
* Compute path integrands at specified angle on a conic.
* @param
* @return
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static  void   cb_conicPathIntegrals
(
double  *pF,
double  theta,
ConicContext    *pConicContext,
int     numFunc
)
    {
    DPoint3d point, tangent;
    double a;
    bsiDConic4d_angleParameterToDPoint3dDerivatives (&pConicContext->conic, &point, &tangent, NULL, theta);
    a = bsiDPoint3d_magnitude (&tangent);
    setPathIntegrands (pF, numFunc, a, &point, &pConicContext->origin);
    }

/*---------------------------------------------------------------------------------**//**
* Compute path integrands at specified angle on a conic.
* @param
* @return
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static  void   cb_bezierPathIntegrals
(
double  *pF,
double  theta,
BezierContext *pBezier,
int     numFunc
)
    {
    DPoint3d point, tangent;
    double a;
    bsiBezierDPoint4d_evaluateDPoint3dArray
                (
                &point, &tangent,
                pBezier->poleArray, pBezier->numPole,
                &theta, 1);

    a = bsiDPoint3d_magnitude (&tangent);
    setPathIntegrands (pF, numFunc, a, &point, &pBezier->origin);
    }

static void computeAndAccumulatePathIntegrals
(
PathIntegralContext     *pContext,
double                  u0,
double                  u1,
DPoint3d                *pOrigin,
PFVectorIntegrand       F,
void                    *pData
)
    {
    double integral[NUM_PATH_CENTROID_INTEGRALS], error[NUM_PATH_CENTROID_INTEGRALS];
    int count, i;
    static double s_absTol = 0.0;
    static double s_relTol = 1.0e-12;
    int numFunc = pContext->bEnableProducts ? 10 : 4;

    bsiMath_recursiveNewtonCotes5Vector (integral, error, &count,
                        u0, u1, s_absTol, s_relTol, F, pData, numFunc);
    if (pOrigin)
        {
        /* Caller subtracted off an origin.  Add it back to the moment terms. */
        double length = integral[INDEX_LENGTH];
        double x0 = pOrigin->x;
        double y0 = pOrigin->y;
        double z0 = pOrigin->z;
        double Ix = integral[INDEX_X];
        double Iy = integral[INDEX_Y];
        double Iz = integral[INDEX_Z];

        // These integrals are true xx,yy, zz as named ....
        integral[INDEX_X] += x0 * length;
        integral[INDEX_Y] += y0 * length;
        integral[INDEX_Z] += z0 * length;
        if (pContext->bEnableProducts)
            {
            integral[INDEX_XX] += 2.0 * x0 * Ix + x0 * x0 * length;
            integral[INDEX_YY] += 2.0 * y0 * Iy + y0 * y0 * length;
            integral[INDEX_ZZ] += 2.0 * z0 * Iz + z0 * z0 * length;
            integral[INDEX_XY] += x0 * Iy + y0 * Ix + x0 * y0 * length;
            integral[INDEX_XZ] += x0 * Iz + z0 * Ix + x0 * z0 * length;
            integral[INDEX_YZ] += y0 * Iz + z0 * Iy + y0 * z0 * length;
            }
        }

    for (i = 0; i < 4; i++)
        {
        pContext->sum[i] += integral[i];
        }

    /* Flop out to external sums, where xx,yy,zz entries are really r^2 - xx, r^2 - yy, etc. */
    if (pContext->bEnableProducts)
        {
        pContext->sum[INDEX_XX] += integral[INDEX_YY] + integral[INDEX_ZZ];
        pContext->sum[INDEX_YY] += integral[INDEX_XX] + integral[INDEX_ZZ];
        pContext->sum[INDEX_ZZ] += integral[INDEX_XX] + integral[INDEX_YY];

        pContext->sum[INDEX_XY] += integral[INDEX_XY];
        pContext->sum[INDEX_XZ] += integral[INDEX_XZ];
        pContext->sum[INDEX_YZ] += integral[INDEX_YZ];
        }
    }

/*---------------------------------------------------------------------------------**//**
* Accumulate the path integrals for ellipse.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_accumulateEllipseCentroid
(
GraphicsPointArrayCP pSource,
      PathIntegralContext       *pContext,
      int                       i0,
      int                       i1
)
    {
    int iRead = i0;
    ConicContext conicContext;
    conicContext.origin = pContext->origin;

    if (jmdlGraphicsPointArray_getDConic4d (pSource, &iRead, &conicContext.conic,
                        NULL, NULL, NULL, NULL))
        {
        if (pContext->bFixedZ)
            {
            conicContext.conic.center.z = pContext->origin.z * conicContext.conic.center.w;
            conicContext.conic.vector0.z = pContext->origin.z * conicContext.conic.vector0.w;
            conicContext.conic.vector90.z = pContext->origin.z * conicContext.conic.vector90.w;
            }
        if (conicContext.conic.sweep < 0.0)
            {
            conicContext.conic.start = - conicContext.conic.start;
            conicContext.conic.sweep = - conicContext.conic.sweep;
            conicContext.conic.vector90.Negate ();
            }
        computeAndAccumulatePathIntegrals
                            (
                            pContext,
                            conicContext.conic.start,
                            conicContext.conic.start + conicContext.conic.sweep,
                            NULL,
                            (PFVectorIntegrand)cb_conicPathIntegrals,
                            &conicContext
                            );
        }
    }


/*---------------------------------------------------------------------------------**//**
* Accumulate the path integrals for bezier.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_accumulateBezierCentroid
(
GraphicsPointArrayCP pSource,
      PathIntegralContext       *pContext,
      int                       i0,
      int                       i1,
       BezierContext &bezier
)
    {
    DPoint3d curveOrigin;
    DPoint3d shift;
    double w;
    int i;
    /* Shift the origin to limit numeric problems */
    if (bsiDPoint4d_normalize (&bezier.poleArray[0], &curveOrigin))
        {
        for (i = 0; i < bezier.numPole; i++)
            {
            w = bezier.poleArray[i].w;
            bezier.poleArray[i].x -= w * curveOrigin.x;
            bezier.poleArray[i].y -= w * curveOrigin.y;
            bezier.poleArray[i].z -= w * curveOrigin.z;
            }
         if (pContext->bFixedZ)
            {
            /* Push all the poles onto the origin z (accounting for weight) */
            curveOrigin.z = pContext->origin.z;
            for (i = 0; i < bezier.numPole; i++)
                {
                w = bezier.poleArray[i].w;
                bezier.poleArray[i].z = w * curveOrigin.z;
                }
            }
        // Immedate integral calculations are relative to the first point of the bezier.
        // These results are subsequently shifted out to the context origin.
        bsiDPoint3d_zero (&bezier.origin);
        bsiDPoint3d_subtractDPoint3dDPoint3d (&shift, &curveOrigin, &pContext->origin);
        computeAndAccumulatePathIntegrals
                            (pContext, 0.0, 1.0,
                            &shift,
                            (PFVectorIntegrand)cb_bezierPathIntegrals, &bezier);
        }
    }


/*---------------------------------------------------------------------------------**//**
* Accumulate the path integrals for bezier.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_accumulateBezierCentroid
(
GraphicsPointArrayCP pSource,
      PathIntegralContext       *pContext,
      int                       i0,
      int                       i1
)
    {
    int iRead = i0;
    BezierContext bezier;

    if (jmdlGraphicsPointArray_getBezier (pSource, &iRead,
                    bezier.poleArray, &bezier.numPole, MAX_BEZIER_CURVE_ORDER))
        {
        jmdlGPA_accumulateBezierCentroid (pSource, pContext, i0, i1, bezier);
        }
    }


/*---------------------------------------------------------------------------------**//**
* Accumulate the length and centroid contributions of a polyline.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_accumulatePolylineCentroid
(
GraphicsPointArrayCP pSource,
      PathIntegralContext       *pContext,
      int                       i0,
      int                       i1
)
    {
    double length = 0.0;
    int i = i0;
    const GraphicsPoint *pGP = jmdlGraphicsPointArray_getConstPtr (pSource, 0);
    DPoint3d basePoint, currPoint, midPoint;

    /* look for the first real point: */
    while (i <= i1 && !bsiDPoint4d_normalize (&pGP[i].point, &basePoint))
        {
        i++;
        }

    if (pContext->bFixedZ)
        basePoint.z = pContext->origin.z;

    /* Accumulate lengths to successive real points */
    while (++i <= i1)
        {
        if (bsiDPoint4d_normalize (&pGP[i].point, &currPoint))
            {
            DPoint3d midVector, delta;

            if (pContext->bFixedZ)
                currPoint.z = pContext->origin.z;

            length = bsiDPoint3d_distance (&basePoint, &currPoint);

            pContext->sum[INDEX_LENGTH] += length;
            bsiDPoint3d_interpolate (&midPoint, &basePoint, 0.5, &currPoint);
            bsiDPoint3d_subtractDPoint3dDPoint3d (&midVector, &midPoint, &pContext->origin);
            pContext->sum[INDEX_X] += midVector.x * length;
            pContext->sum[INDEX_Y] += midVector.y * length;
            pContext->sum[INDEX_Z] += midVector.z * length;
            if (pContext->bEnableProducts)
                {
                DPoint3d vector0, vector1;
                double xMoment, yMoment, zMoment;
                double adiv3 = length / 3.0;
                double adiv6 = 0.5 * adiv3;

                bsiDPoint3d_subtractDPoint3dDPoint3d (&vector0, &basePoint, &pContext->origin);
                bsiDPoint3d_subtractDPoint3dDPoint3d (&vector1, &currPoint, &pContext->origin);
                bsiDPoint3d_subtractDPoint3dDPoint3d (&delta, &currPoint, &basePoint);

                xMoment = (vector0.x * vector0.x + vector0.x * vector1.x + vector1.x * vector1.x) * adiv3;
                yMoment = (vector0.y * vector0.y + vector0.y * vector1.y + vector1.y * vector1.y) * adiv3;
                zMoment = (vector0.z * vector0.z + vector0.z * vector1.z + vector1.z * vector1.z) * adiv3;

                pContext->sum[INDEX_XX] += (yMoment + zMoment);
                pContext->sum[INDEX_YY] += (xMoment + zMoment);
                pContext->sum[INDEX_ZZ] += (xMoment + yMoment);
                pContext->sum[INDEX_XY] += (6.0 * vector0.x * vector0.y + 3.0 * vector0.x * delta.y + 3.0 * vector0.y * delta.x +
                        2.0 * delta.x * delta.y) * adiv6;
                pContext->sum[INDEX_XZ] += (6.0 * vector0.x * vector0.z + 3.0 * vector0.x * delta.z + 3.0 * vector0.z * delta.x +
                        2.0 * delta.x * delta.z) * adiv6;
                pContext->sum[INDEX_YZ] += (6.0 * vector0.y * vector0.z + 3.0 * vector0.y * delta.z + 3.0 * vector0.z * delta.y +
                        2.0 * delta.y * delta.z) * adiv6;
                }
            basePoint = currPoint;
            }
        }
    }

/*
Internal moment integrator.
*/
static bool    jmdlGraphicsPointArray_integrateMoments
(
GraphicsPointArrayCP pSource,
      double                    *pArcLength,
      DPoint3d                  *pX,        // Integrals of x,y,z
      RotMatrix                 *pXX,       // Integrals of (yy+zz), xy, xz, etc
const DPoint3d                  *pOrigin,
      bool                      bFixedZ
)
    {
    int i0, i1;
    int curveType;
    PathIntegralContext params;
    double length, a;
    bool    ok;

    memset (&params, 0, sizeof (PathIntegralContext));
    params.bFixedZ = bFixedZ;
    params.origin = *pOrigin;
    params.bEnableProducts = pXX != NULL;

    for (i0 = 0;
         jmdlGraphicsPointArray_parseFragment (
                        pSource, &i1,
                        NULL, NULL,
                        &curveType, i0);
         i0 = i1 + 1)
        {
        if (curveType == 0)
            {
            jmdlGPA_accumulatePolylineCentroid (pSource, &params, i0, i1);
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER)  // BSPLINE_CODED
            {
            jmdlGPA_accumulateBezierCentroid (pSource, &params, i0, i1);
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
            {
            jmdlGPA_accumulateEllipseCentroid (pSource, &params, i0, i1);
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BSPLINE)
            {
            BezierContext bezier;
            bool isNullInterval;
            double knot0, knot1;
            for (size_t spanIndex = 0;
                pSource->GetBezierSpanFromBsplineCurve (i0, spanIndex,
                        bezier.poleArray, bezier.numPole,
                        MAX_BEZIER_ORDER, isNullInterval, knot0, knot1); spanIndex++)
                {
                if (!isNullInterval)
                    jmdlGPA_accumulateBezierCentroid (pSource, &params, i0, i1, bezier);
                }
            }
        }

    length  = params.sum[INDEX_LENGTH];

    // test if length is significant
    ok = bsiTrig_safeDivide (&a, 1.0, length, 1.0);

    if (pArcLength)
        *pArcLength = length;

    if (pX)
        {
        pX->x = params.sum[INDEX_X];
        pX->y = params.sum[INDEX_Y];
        pX->z = params.sum[INDEX_Z];
        }

    if (pXX)
        {
        bsiRotMatrix_initFromRowValues
            (
            pXX,
            params.sum[INDEX_XX], params.sum[INDEX_XY], params.sum[INDEX_XZ],
            params.sum[INDEX_XY], params.sum[INDEX_YY], params.sum[INDEX_YZ],
            params.sum[INDEX_XZ], params.sum[INDEX_YZ], params.sum[INDEX_ZZ]
            );
        }

    return ok;
    }



/*---------------------------------------------------------------------------------**//**
* Get the centroid of the path in the array.  This is defined only in terms of the paths,
* so it is a computable property even if the array has dangling paths.
* If the path is a closed path and is symmetric about a point, that point will be
* the centroid, and this result will look like the area centroid.  If the path
* is closed but has no symmetry, the result will be centrally located, but may
* be different from the area centroid.  For instance, imagine a rectangle with one
* edge replaced by a curved path which wiggles very rapidly within a small distance
* of the original edge.  The area centroid will be changed very little, but the
* path centroid is shifted more strongly towards the added arc length in oscillating portion.
*
* @param bFixedZ IN If true, compute with fixed z. This affects the apparent length.
* @param zFix IN z coordinate for optional fixed-z mode.
* @param bFixedZ IN If true, compute with projection of the geoemtry onto this z plane.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlGraphicsPointArray_getPathCentroidExt
(
GraphicsPointArrayCP pSource,
      DPoint3d                  *pCentroid,
      double                    *pArcLength,
      bool                      bFixedZ,
      double                    fixedZ
)
    {
    bool    ok = false;
    GraphicsPoint gp;
    DPoint3d xyz, xyz0;
    DPoint3d Ix;
    double length;
    int i0;

    bsiDPoint3d_zero (&xyz);
    // Take any point in the array as origin.
    for (
        i0 = 0;
        jmdlGraphicsPointArray_getGraphicsPoint (pSource, &gp, i0);
        i0++
        )
         {
         if (bsiDPoint4d_normalize (&gp.point, &xyz))
            {
            ok = true;
            break;
            }
         }
    if (ok && bFixedZ)
        xyz.z = fixedZ;
    // Save the first point (if any) in case there is zero length.
    xyz0 = xyz;

    if (  ok
       && jmdlGraphicsPointArray_integrateMoments
                (pSource, &length, &Ix, NULL, &xyz, bFixedZ)
       )
        {
        if (pCentroid)
            {
            *pCentroid = xyz;
            if (length != 0.0)
                bsiDPoint3d_addScaledDPoint3d (pCentroid, &xyz, &Ix, 1.0 / length);
            }

        if (pArcLength)
            *pArcLength = length;
        }
    else
        {

        if (pCentroid)
            *pCentroid = xyz0;
        if (pArcLength)
            *pArcLength = 0.0;
        }

    return ok;
    }


/*---------------------------------------------------------------------------------**//**
* Get the products of x,y,z, xx, xy, xz, yy, yz of the path in the array,
*       measured relative to specified origin.
* @param pXMoments OUT integrals of x,y,z dr
* @param pXXMoments OUT integrals of r^2-xx,xy,xz etc. (symmetric)
* @param pOrigin IN origin for moment calculations.  This is NOT an optional argument.
* @param bFixedZ IN If true, compute with projection of geometry onto z plane of the origin.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlGraphicsPointArray_getPathProducts
(
GraphicsPointArrayCP pSource,
      double                    *pArcLength,
      DPoint3d                  *pXMoments,
      RotMatrix                 *pXXMoments,
const DPoint3d                  *pOrigin,
      bool                      bFixedZ
)
    {
    return jmdlGraphicsPointArray_integrateMoments
                (pSource, pArcLength, pXMoments, pXXMoments, pOrigin, bFixedZ);
    }

/*---------------------------------------------------------------------------------**//**
* Compute centroid of a path, treated strictly as a wire -- not an area boundary.
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlGraphicsPointArray_getPathCentroid
(
GraphicsPointArrayCP pSource,
      DPoint3d                  *pCentroid,
      double                    *pArcLength
)
    {
    return jmdlGraphicsPointArray_getPathCentroidExt (pSource, pCentroid, pArcLength, false, 0.0);
    }

/*---------------------------------------------------------------------------------**//**
* Centroid of triangle from origin to line segment.
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool                    cbCentroidDSegment4d
(
        AreaIntegralContext         *pContext,
GraphicsPointArrayCP pSource,
        int                         index,
const   DSegment4d                  *pSegment
)
    {
    DPoint3d worldPoint[2], localVector[2], centroid;
    double a = 1.0 / 3.0;
    if (    bsiDPoint4d_normalize (&pSegment->point[0], &worldPoint[0])
        &&  bsiDPoint4d_normalize (&pSegment->point[1], &worldPoint[1]))
        {
        double area;
        bsiTransform_multiplyDPoint3dArray
                    (&pContext->worldToLocal, localVector, worldPoint, 2);

        area = 0.5 * bsiDPoint3d_crossProductXY (&localVector[0], &localVector[1]);

        bsiDPoint3d_add2ScaledDPoint3d (&centroid, NULL, &localVector[0], a, &localVector[1], a);

        pContext->sum[INDEX_xdA] += area * centroid.x;
        pContext->sum[INDEX_ydA] += area * centroid.y;
        pContext->sum[INDEX_dA]  += area;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Compute xy area integrands at specified angle on a conic.
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static  void   cb_DConic4dAreaIntegrands
(
double   *pF,
double   theta,
DConic4d *pConic,
int      numFunc
)
    {
    DPoint3d point, tangent;
    double dA;
    double a = 2.0 / 3.0;
    bsiDConic4d_angleParameterToDPoint3dDerivatives (pConic, &point, &tangent, NULL, theta);
    dA = 0.5 * (point.x * tangent.y - point.y * tangent.x);
    pF[INDEX_dA]        = dA;
    pF[INDEX_xdA]       = a * point.x * dA;
    pF[INDEX_ydA]       = a * point.y * dA;
    }

/*---------------------------------------------------------------------------------**//**
* Compute path integrands at specified parameter on a bezier
* @param
* @return
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static  void   cb_DPoint4dBezierAreaIntegrands
(
double  *pF,
double  theta,
BezierContext *pBezier,
int     numFunc
)
    {
    DPoint3d point, tangent;
    double dA;
    double a = 2.0 / 3.0;
    bsiBezierDPoint4d_evaluateDPoint3dArray
                (
                &point, &tangent,
                pBezier->poleArray, pBezier->numPole,
                &theta, 1);
    dA = 0.5 * (point.x * tangent.y - point.y * tangent.x);
    pF[INDEX_dA]        = dA;
    pF[INDEX_xdA]       = a * point.x * dA;
    pF[INDEX_ydA]       = a * point.y * dA;
    }

static void computeAndAccumulateXYCentroidIntegrals
(
AreaIntegralContext     *pContext,
double                  x0,
double                  x1,
PFVectorIntegrand       F,
void                    *pData
)
    {
    double integral[NUM_MOMENT_INTEGRALS], error[NUM_MOMENT_INTEGRALS];
    int count, i;
    static double s_absTol = 0.0;
    static double s_relTol = 1.0e-8;

    bsiMath_recursiveNewtonCotes5Vector (integral, error, &count,
                        x0, x1, s_absTol, s_relTol, F, pData, NUM_MOMENT_INTEGRALS);
    for (i = 0; i < NUM_MOMENT_INTEGRALS; i++)
        {
        pContext->sum[i] += integral[i];
        }
    }

/*---------------------------------------------------------------------------------**//**
* "Process" function for conic area integrals.  Repackage the
* conic and dispatch to more generic integrators.  (Still a couple levels away
* from evaluating the curve!!)
+---------------+---------------+---------------+---------------+---------------+------*/
static bool                    cbCentroidDConic4d
(
        AreaIntegralContext         *pContext,
GraphicsPointArrayCP pSource,
        int                         index,
const   DConic4d                    *pConic
)
    {
    DConic4d localConic;
    bsiDConic4d_applyTransform (&localConic, &pContext->worldToLocal, pConic);
    computeAndAccumulateXYCentroidIntegrals
                        (pContext, pConic->start, pConic->start + pConic->sweep,
                        (PFVectorIntegrand)cb_DConic4dAreaIntegrands,
                        &localConic);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* "Process" function for BezierDPoint4d area integrals.  Repackage the
* bezier and dispatch to more generic integrators. (Still a couple levels away
* from evaluating the curve!!)
+---------------+---------------+---------------+---------------+---------------+------*/
static bool                    cbCentroidBezierDPoint4d
(
        AreaIntegralContext         *pContext,
        TaggedBezierDPoint4d        &bezier
)
    {
    BezierContext bezierContext;
    bezierContext.numPole = bezier.m_order;
    if (bezier.m_order <= MAX_BEZIER_CURVE_ORDER)
        {
        bsiTransform_multiplyDPoint4dArray (&pContext->worldToLocal,
                        bezierContext.poleArray,
                        bezier.m_poles,
                        bezier.m_order);
        computeAndAccumulateXYCentroidIntegrals
                        (pContext, 0.0, 1.0,
                        (PFVectorIntegrand)cb_DPoint4dBezierAreaIntegrands,
                        &bezierContext);
        }
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* Compute the centroid of the xy projection of the curves into a local coordinate frame.
* @param pCx        => x coordinate of centroid.
* @param pCy        => y coordinate of centroid.
* @param pArea      => area.
* The calculation is specifically defined as
*    (mx,my) = integral ( x (x dy - y dx), y (x dy - y dx) )
*    area    = integral ( x dy - y dx)
*    (cx,cy) = (mx,my) / area
*
* There is no test for whether the gpa is closed, planar, or nonintersecting,
* Interpretation of results is therefore at callers' risk.
*
* @bsihdr                                       EarlinLutz      05/00
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlGraphicsPointArray_getXYCentroid_go
(
GraphicsPointArrayCP pSource,
      double                    *pCx,
      double                    *pCy,
      double                    *pArea,
const Transform              *pWorldToLocal  /* MUST BE PROVIDED */
)
    {
    double area;
    double mx, my, cx, cy;
    AreaIntegralContext params;
    bool    retcode;
    bool    bDivX, bDivY;

    memset (&params, 0, sizeof (AreaIntegralContext));
    params.worldToLocal = *pWorldToLocal;

    jmdlGraphicsPointArray_processPrimitives
                    (
                    &params,
                    pSource,
                    (GPAFunc_DSegment4d)    cbCentroidDSegment4d,
                    (GPAFunc_DConic4d)      cbCentroidDConic4d,
                    (GPAFunc_BezierDPoint4dTagged)cbCentroidBezierDPoint4d
                    );

    area  = params.sum[INDEX_dA];
    mx = params.sum[INDEX_xdA];
    my = params.sum[INDEX_ydA];
    // Cautious division for each component -- failure for either one is overall failure.
    // (TR 135079)
    bDivX = bsiTrig_safeDivide (&cx, mx, area, 0.0);
    bDivY = bsiTrig_safeDivide (&cy, my, area, 0.0);
    retcode = bDivX && bDivY;
    if (pCx)
        *pCx = cx;

    if (pCy)
        *pCy = cy;

    if (pArea)
        *pArea = area;

    return retcode;
    }


/*---------------------------------------------------------------------------------**//**
* Compute the centroid of the xy projection of the curves
* @param pCx        => x coordinate of centroid.
* @param pCy        => y coordinate of centroid.
* @param pArea      => area.
* The calculation is specifically defined as
*    (mx,my) = integral ( x (x dy - y dx), y (x dy - y dx) )
*    area    = integral ( x dy - y dx)
*    (cx,cy) = (mx,my) / area
*
* There is no test for whether the gpa is closed, planar, or nonintersecting,
* Interpretation of results is therefore at callers' risk.
*
* @bsihdr                                       EarlinLutz      05/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlGraphicsPointArray_getXYCentroid
(
GraphicsPointArrayCP pSource,
      double                    *pCx,
      double                    *pCy,
      double                    *pArea
)
    {
    Transform worldToLocal;
    bool    boolstat;

    bsiTransform_initIdentity (&worldToLocal);
    boolstat = jmdlGraphicsPointArray_getXYCentroid_go
                (
                pSource,
                pCx,
                pCy,
                pArea,
                &worldToLocal
                );
    return boolstat;
    }



/*---------------------------------------------------------------------------------**//**
* Compute the best plane containing the geometry.   Compute centroid and area of projection
*       into that plane.
* @param pLocalToWorld  <= transformation from plane to world.
* @param pWorldToLocal  <= transformation from world to plane
* @param pCentroid      <= (world) centroid
* @param pArea          <= area as projected in the plane.
* @bsihdr                                       EarlinLutz      05/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlGraphicsPointArray_getCentroid
(
GraphicsPointArrayCP pSource,
      Transform              *pLocalToWorld,
      Transform              *pWorldToLocal,
      double                    *pMaxDeviation,
      DPoint3d                  *pCentroid,
      double                    *pArea
)
    {
    Transform localToWorld, worldToLocal;
    double maxDeviation, area = 0.0;
    double cx, cy;
    DPoint3d worldCentroid;
    bool    retcode = false;
    worldToLocal.InitIdentity ();
    localToWorld.InitIdentity ();
    worldCentroid.Zero ();
    if (jmdlGraphicsPointArray_getPlaneAsTransformExt2
                    (pSource, &localToWorld, pMaxDeviation, NULL, NULL))
        {
        bsiTransform_invertAsRotation (&worldToLocal, &localToWorld);
        retcode = jmdlGraphicsPointArray_getXYCentroid_go
                (
                pSource, &cx, &cy, &area, &worldToLocal
                );
        if (retcode)
            {
            bsiTransform_multiplyComponents (&localToWorld, &worldCentroid, cx, cy, 0.0);
            }
        }

    if (!retcode)
        {
        bsiTransform_initIdentity (&localToWorld);
        worldToLocal = localToWorld;
        bsiDPoint3d_zero (&worldCentroid);
        maxDeviation = area = 0.0;
        }

    if (pLocalToWorld)
        *pLocalToWorld = localToWorld;
    if (pWorldToLocal)
        *pWorldToLocal = worldToLocal;
    if (pCentroid)
        *pCentroid = worldCentroid;
    if (pArea)
        *pArea = area;

    return retcode;
    }

/*---------------------------------------------------------------------------------**//**
* Compute the centroid of the xy projection of the curves
* @param pCx        => x coordinate of centroid.
* @param pCy        => y coordinate of centroid.
* @param pArea      => area.
* The calculation is specifically defined as
*    (mx,my) = integral ( x (x dy - y dx), y (x dy - y dx) )
*    area    = integral ( x dy - y dx)
*    (cx,cy) = (mx,my) / area
*
* @param readIndex  => start with primitive "after" this index.
*
* There is no test for whether the gpa is closed, planar, or nonintersecting,
* Interpretation of results is therefore at callers' risk.
*
* @bsihdr                                       EarlinLutz      05/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlGraphicsPointArray_getXYCentroidToMajorBreak
(
GraphicsPointArrayCP pSource,
      double                    *pCx,
      double                    *pCy,
      double                    *pArea,
      int                       readIndex
)
    {
    double area, a;
    double mx, my;
    AreaIntegralContext params;
    bool    retcode;
    DPoint3d origin;
    int nGot;

    memset (&params, 0, sizeof (AreaIntegralContext));
    jmdlGraphicsPointArray_getDPoint3dArray (pSource, &origin, &nGot, readIndex, 1);
    bsiTransform_initFromTranslationXYZ (&params.worldToLocal,
                    -origin.x, -origin.y, -origin.z);
    jmdlGraphicsPointArray_processPrimitivesToMajorBreak
                    (
                    &params,
                    pSource,
                    readIndex,
                    (GPAFunc_DSegment4d)cbCentroidDSegment4d,
                    (GPAFunc_DConic4d)cbCentroidDConic4d,
                    (GPAFunc_BezierDPoint4dTagged)cbCentroidBezierDPoint4d
                    );

    area  = params.sum[INDEX_dA];
    mx = params.sum[INDEX_xdA];
    my = params.sum[INDEX_ydA];

    retcode = bsiTrig_safeDivide (&a, 1.0, area, 0.0);
    if (pCx)
        *pCx = mx * a + origin.x;

    if (pCy)
        *pCy = my * a + origin.y;

    if (pArea)
        *pArea = area;

    return retcode;
    }

#define NUM_SOLID_ANGLE_INTEGRAND 2
#define INDEX_dTheta 0
#define INDEX_sinPhi_dTheta 1

/*---------------------------------------------------------------------------------**//**
* Solid angle is integral of
*       (1-sin phi)dTheta
* where theta =atan (y/x)
*       phi = atan (z/r)
*       r  = sqrt (x^2 + y^2 + z^2)
* Return this as two integrands so the closed integral of dTheta can be checked
*       for singularity when the pole passes through the surface patch.
* @bsihdr                                       EarlinLutz      05/00
+---------------+---------------+---------------+---------------+---------------+------*/
static void solidAngleIntegrands
(
Transform *pTransform,
double *pIntegrandArray,
int    numIntegrand,
DPoint3d *pPoint,
DPoint3d *pTangent
)
    {
    DPoint3d localPoint, localTangent;
    double dTheta, cross;
    double sinPhi;
    double rho2;
    double r;
    /* The transform puts us in the spherical coordinates base frame */
    bsiTransform_multiplyDPoint3d (pTransform, &localPoint, pPoint);
    bsiTransform_multiplyDPoint3dByMatrixPart (pTransform, &localTangent, pTangent);
    /* various Spherical coordinates components ... */
    rho2 = localPoint.x * localPoint.x + localPoint.y * localPoint.y;
    cross = localPoint.x * localTangent.y - localPoint.y * localTangent.x;
    r = bsiDPoint3d_magnitude (&localPoint);
    bsiTrig_safeDivide (&dTheta, cross, rho2, 0.0);
    bsiTrig_safeDivide (&sinPhi, localPoint.z, r, 0.0);
    pIntegrandArray[INDEX_dTheta] = dTheta;
    pIntegrandArray[INDEX_sinPhi_dTheta] = sinPhi * dTheta;
    }


/*---------------------------------------------------------------------------------**//**
* @description Computes the solid angle subtended by the (presumably closed
* non-selfintersecting)* shape in the GPA.
* There is no test for whether the gpa is closed and nonintersecting,
* Interpretation of results is therefore at callers' risk.
*
* @bsihdr                                       EarlinLutz      05/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlGraphicsPointArray_solidAngle
(
GraphicsPointArrayCP pSource,
      double                    *pTheta,
      DPoint3d                  *pEyePoint
)
    {
    //bool    retCode = false;
    double localIntegral[NUM_SOLID_ANGLE_INTEGRAND];
    double totalIntegral[NUM_SOLID_ANGLE_INTEGRAND];
    int readIndex;
    double theta;
    int nextReadIndex;
    Transform transform;
    DPoint3d negatedOrigin;
    //int errorCount = 0;
    int okCount    = 0;
    bsiDPoint3d_negate (&negatedOrigin, pEyePoint);
    bsiTransform_initIdentity (&transform);
    bsiTransform_setTranslation (&transform, &negatedOrigin);

    theta = totalIntegral[INDEX_dTheta] = totalIntegral[INDEX_sinPhi_dTheta] = 0.0;

    for (readIndex = 0;
        jmdlGraphicsPointArray_integratePrimitivesToMajorBreak
                    (
                    pSource,
                    &nextReadIndex,
                    localIntegral,
                    NUM_SOLID_ANGLE_INTEGRAND,
                    readIndex,
                    (GPAFunc_TangentialIntegrand)solidAngleIntegrands,
                    &transform
                    );
        readIndex = nextReadIndex
        )
        {
        okCount++;
        theta += localIntegral[INDEX_dTheta] - localIntegral[INDEX_sinPhi_dTheta];
        }

    if (pTheta)
        *pTheta = theta;

    return okCount > 0;
    }

//#include <Mstn\Tools\ToolsAPI.h>
#define MAX_FUNCTION 10

/*---------------------------------------------------------------------------------**//**
* Return a vector of function values cos((i+1) x) for 0 <= i < numFunc.
* @param
* @return
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static  void   trigIntegralTest
(
double  *pF,
double  x,
void    *voidP,
int     numFunc
)
    {
    int i;
    for (i = 0; i < numFunc; i++)
        {
        pF[i] = cos ((double) (i + 1) * x);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Evaluate the quartic polynomial (x-a0)(x-a1)(x-a2)(x-a3)
* @param
* @return
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static  bool    evaluateQuartic
(
double *pF,
double x,
double *pA
)
    {
    *pF = (x - pA[0]) * (x - pA[1]) * (x - pA[2]) * (x - pA[3]);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Integrate under (x-a0)(x-a1)(x-a2)(x-a3) from x=0 to 1,2,3,4
* @param
* @return
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static  void nc5WeightGen
(
double spike,
double a0,
double a1,
double a2,
double a3,
double a
)
    {
    double x0 = 0.0;
    double x1;
    double integral;
    double error;
    int count;
    double aa[4];
    double bb[4];
    double denom;
    int i;
    aa[0] = a0;
    aa[1] = a1;
    aa[2] = a2;
    aa[3] = a3;


    denom = (spike - a0) * (spike - a1) * (spike - a2) * (spike - a3);

    printf(" Integrals under quartic %lf * (x-%lf) * (x-%lf) * (x-%lf) * (x-%lf) denom %lf\n",
                        a, a0, a1, a2, a3, denom);
        printf("       x1   integral   error     count      final denom\n");

    for (x1 = 1.0, i = 0; x1 < 4.5; x1 += 1.0, i++)
        {
        bsiMath_recursiveNewtonCotes5 (&integral, &error, &count,x0, x1,
                    0.0, 1.0e-12,
                    (PFScalarIntegrand)evaluateQuartic, aa);
        printf("       %lf %lf %lf %d   %lf\n", x1, a * integral, error, count, denom * a);
        bb[i] = a * integral;
        }

    printf("        {");
    for (i = 0; i < 4; i++)
        {
        printf (    " %lf%s", bb[i],
                    i < 3 ? "," : ""
                    );
        }
    printf("}\n");

    }

/*---------------------------------------------------------------------------------**//**
* Integrate under known functions to test integration logic.
* @param
* @return
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMath_testIntegrals
(
)
    {
    double integral[MAX_FUNCTION];
    double error[MAX_FUNCTION];
    double exact, a, e;
    int    count;
    double x0 = 1.0;
    double x1 = 4.2;
    double abstol = 1.0e-12;
    double reltol = 0.0;
    bool    ok = true;

    /* Evaluate a vector of integrands
            f(i,x) = cos ((i+1) x)
        Each indefinite integral is sin( (i+1) x) / (i+1)
    */
    int i, j;
    for (i = 1; i <= MAX_FUNCTION; i++)
        {
        bsiMath_recursiveNewtonCotes5Vector (integral, error, &count,
                        x0, x1, abstol, reltol,
                        (PFVectorIntegrand)trigIntegralTest, NULL, i);
        printf(" Integrals of cos(ax) for 1<=a<=%d, %lf <=x<= %lf    %d function calls\n",
                        i, x0, x1, count);
        printf(" a      approx      exact     error\n");
        for (j = 0; j < i; j++)
            {
            a = j + 1;
            exact = (sin(a * x1) - sin (a* x0)) / a;
            e = fabs (exact - integral[j]);
            ok &= e < abstol;
            printf(" %5lf %le %le %le %s\n",
                        a, integral[j], exact, e,
                        e < abstol ? "ok" : "ERROR"
                  );
            }
        }

    a = 60.0;
    nc5WeightGen (0.0, 1.0, 2.0, 3.0, 4.0, a);
    nc5WeightGen (1.0, 0.0, 2.0, 3.0, 4.0, a);
    nc5WeightGen (2.0, 0.0, 1.0, 3.0, 4.0, a);
    nc5WeightGen (3.0, 0.0, 1.0, 2.0, 4.0, a);
    nc5WeightGen (4.0, 0.0, 1.0, 2.0, 3.0, a);

    return ok;
    }


typedef struct
    {
    GraphicsPointArray          *pDest;
    GraphicsPointArrayCP pSource;
    const double                    *pDistanceArray;
    int                             numDist;
    double                          NCRelTol;       // relative tolerance for Newton-Cotes
    double                          absTol;         // absolute distance tolerance for termination
    bool                            storeTangent;
    bool                            allDone;
    int                             numUsed;
    double                          distanceToNextOutput;
    int                             primitiveIndex;
    } ALSParams;


/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void als_advance
(
        ALSParams               *pParams
)
    {
    if (pParams->numUsed < pParams->numDist && pParams->pDistanceArray)
        {
        pParams->distanceToNextOutput= pParams->pDistanceArray[pParams->numUsed];
        pParams->numUsed++;
        }
    else
        {
        pParams->allDone = true;
        }
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void als_init
(
        ALSParams               *pParams,
        GraphicsPointArray  *pDest,
GraphicsPointArrayCP pSource,
const   double                  *pDistanceArray,
        int                     numDist,
        double                  tolerance,
        double                  maxTangentChange,
        bool                    storeTangent
)
    {
    static double s_defaultTol = 1.0e-9;

    double  length;
    double  relTol = (tolerance <= 0.0) ? s_defaultTol : tolerance;

    pParams->pDest                  = pDest;
    pParams->pDistanceArray         = pDistanceArray;
    pParams->numDist                = numDist;
    pParams->NCRelTol               = relTol;
    pParams->absTol                 = relTol;

    // use a fraction of the arclength for the stop criterion in als_recordEndPoint
    if (jmdlGraphicsPointArray_getPathCentroidExt (pSource, NULL, &length, false, 0.0))
        pParams->absTol *= length;

    pParams->storeTangent           = storeTangent;

    pParams->numUsed                = 0;
    pParams->allDone                = false;

    als_advance (pParams);
    }

/*---------------------------------------------------------------------------------**//**
* Test if the requested points have already been output.
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        als_allDone
(
ALSParams           *pParams
)
    {
    return pParams->allDone;
    }

/*---------------------------------------------------------------------------------**//**
* Add a point to the ouput array and advance the status data.
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void als_addDPoint3d
(
        ALSParams               *pParams,
const   DPoint3d                *pStart,
const   DPoint3d                *pTangent,
        double                  fraction
)
    {
    if (!als_allDone (pParams))
        {
        jmdlGraphicsPointArray_addComplete
                            (
                            pParams->pDest,
                            pStart->x, pStart->y, pStart->z, 1.0,
                            fraction,
                            0,
                            pParams->primitiveIndex
                            );
        if (pParams->storeTangent)
            jmdlGraphicsPointArray_addComplete
                            (
                            pParams->pDest,
                            pTangent->x, pTangent->y, pTangent->z, 0.0,
                            fraction,
                            0,
                            pParams->primitiveIndex
                            );

        als_advance (pParams);
        }

    }
/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void als_recordEndPoint
(
        ALSParams               *pParams,
const   DPoint3d                *pPoint,
const   DPoint3d                *pTangent,
        double                  unusedLength
)
    {
    pParams->distanceToNextOutput    -= unusedLength;
    if (!als_allDone (pParams) && pParams->distanceToNextOutput < pParams->absTol)
        {
        als_addDPoint3d (pParams, pPoint, pTangent, 1.0);
        }
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void als_recordStartPoint
(
        ALSParams               *pParams,
const   DPoint3d                *pPoint,
const   DPoint3d                *pTangent
)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void als_segment
(
        ALSParams               *pParams,
const   DPoint3d                *pStart,
const   DPoint3d                *pEnd
)
    {
    DPoint3d tangent;
    double length;
    double remainingLength;
    double s, ds;
    DPoint3d currPoint;

    bsiDPoint3d_subtractDPoint3dDPoint3d (&tangent, pEnd, pStart);

    length = remainingLength = bsiDPoint3d_magnitude (&tangent);
    if (length <= 0.0)
        return;

    als_recordStartPoint (pParams, pStart, &tangent);
    s = 0.0;
    while (s < 1.0 && pParams->distanceToNextOutput < remainingLength && !als_allDone (pParams))
        {
        ds = pParams->distanceToNextOutput / length;
        s += ds;
        remainingLength -= pParams->distanceToNextOutput;
        bsiDPoint3d_addScaledDPoint3d (&currPoint, pStart, &tangent, s);
        als_addDPoint3d (pParams, &currPoint, &tangent, s);
        }

    als_recordEndPoint (pParams, pEnd, &tangent, remainingLength);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void als_circularArc
(
        ALSParams               *pParams,
const   DEllipse3d              *pEllipse
)
    {
    DPoint3d startPoint, startTangent, endPoint, endTangent;
    DPoint3d currPoint, currTangent;
    double length;
    double remainingLength;
    double accumulatedTurn;
    double theta, dTheta;
    double r;
    double fraction;
    double sweep = pEllipse->sweep;
    double aSweep = fabs (sweep);

    theta = pEllipse->start;

    bsiDEllipse3d_evaluateDerivatives (pEllipse, &startPoint, &startTangent,
                            NULL, pEllipse->start);

    bsiDEllipse3d_evaluateDerivatives (pEllipse, &endPoint,   &endTangent,
                            NULL, pEllipse->start + pEllipse->sweep);

    /* For a circle, the radius and tangent magnitude are the same (everywhere) */
    r = bsiDPoint3d_magnitude (&startTangent);

    if (r <= 0.0)
        return;

    length = remainingLength = fabs(pEllipse->sweep) * r;

    if (length <= 0.0)
        return;

    als_recordStartPoint (pParams, &startPoint, &startTangent);

    accumulatedTurn = 0.0;
    while (accumulatedTurn < aSweep && pParams->distanceToNextOutput < remainingLength && !als_allDone (pParams))
        {
        bsiTrig_safeDivide (&dTheta, pParams->distanceToNextOutput, r, pEllipse->sweep);
        theta += dTheta;
        accumulatedTurn += fabs (dTheta);
        remainingLength -= pParams->distanceToNextOutput;
        bsiDEllipse3d_evaluateDerivatives (pEllipse, &currPoint, &currTangent, NULL, theta);
        fraction = bsiDEllipse3d_angleToFraction (pEllipse, theta);
        als_addDPoint3d (pParams, &currPoint, &currTangent, fraction);
        }

    als_recordEndPoint (pParams, &endPoint, &endTangent, remainingLength);

    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void als_parsePolyline
(
        ALSParams               *pParams,
GraphicsPointArrayCP pSource,
        int                     i0,
        int                     i1
)
    {
    //double length = 0.0;
    int i = i0;
    const GraphicsPoint *pGP = jmdlGraphicsPointArray_getConstPtr (pSource, 0);

    DPoint3d basePoint, currPoint;

    /* look for the first real point: */
    while (i <= i1 && !bsiDPoint4d_normalize (&pGP[i].point, &basePoint))
        {
        i++;
        }

    /* Accumulate lengths to successive real points */

    while (++i <= i1)
        {
        if (bsiDPoint4d_normalize (&pGP[i].point, &currPoint))
            {
            pParams->primitiveIndex = i - 1;
            als_segment (pParams, &basePoint, &currPoint);
            basePoint = currPoint;
            }
        }
    }

typedef struct _ALSIntegrationParams
    {
    ALSParams   *pParams;
    double unusedLength;

    bool    (*cb_pointAndTangent)
                (
                const struct _ALSIntegrationParams *,
                DPoint3d * ,
                DPoint3d *,
                double
                );

    bool    (*cb_tangentMagnitude)
                (
                double *,
                double,
                const struct _ALSIntegrationParams *
                );

    /* Only one of the following is used on any particular curve */
    const DEllipse3d  *pEllipse;
    const DConic4d    *pConic;
    const DPoint4d    *pPoleArray;
    int         order;

    /* Natural parameters of interval start and end, for
        normalization during NewtonCotes search. */
    double                          normalizationParam0;
    double                          normalizationParam1;
    // Output parameter interval
    double                          knot0;
    double                          knot1;
    } ALSIntegrationParams;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    als_cb_DEllipse3d_tangentMagnitude
(
        double                  *pMag,
        double                  theta,
const   ALSIntegrationParams    *pIntegrationParams
)
    {
    DPoint3d tangent;
    bsiDPoint3d_add2ScaledDPoint3d (&tangent, NULL,
                                    &pIntegrationParams->pEllipse->vector0, -sin(theta),
                                    &pIntegrationParams->pEllipse->vector90, cos(theta));
    *pMag = bsiDPoint3d_magnitude (&tangent);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    als_cb_DEllipse3d_pointAndTangent
(
const   ALSIntegrationParams    *pIntegrationParams,
        DPoint3d                *pPoint,
        DPoint3d                *pTangent,
        double                  theta
)
    {
    bsiDEllipse3d_evaluateDerivatives
                    (
                    pIntegrationParams->pEllipse,
                    pPoint,
                    pTangent,
                    NULL,
                    theta
                    );
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    als_cb_bezierDPoint4d_tangentMagnitude
(
        double                  *pMag,
        double                  param,
const   ALSIntegrationParams    *pIntegrationParams
)
    {
    DPoint3d tangent;
    bsiBezierDPoint4d_evaluateDPoint3dArray (NULL, &tangent,
                        pIntegrationParams->pPoleArray, pIntegrationParams->order,
                        &param, 1);
    *pMag = bsiDPoint3d_magnitude (&tangent);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    als_cb_bezierDPoint4d_pointAndTangent
(
const   ALSIntegrationParams    *pIntegrationParams,
        DPoint3d                *pPoint,
        DPoint3d                *pTangent,
        double                  param
)
    {
    bsiBezierDPoint4d_evaluateDPoint3dArray (pPoint, pTangent,
                        pIntegrationParams->pPoleArray, pIntegrationParams->order,
                        &param, 1);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    als_cb_DConic4d_tangentMagnitude
(
        double                  *pMag,
        double                  theta,
const   ALSIntegrationParams    *pIntegrationParams
)
    {
    *pMag = bsiDConic4d_tangentMagnitude (pIntegrationParams->pConic, theta);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    als_cb_DConic4d_pointAndTangent
(
const   ALSIntegrationParams    *pIntegrationParams,
        DPoint3d                *pPoint,
        DPoint3d                *pTangent,
        double                  theta
)
    {
    bsiDConic4d_angleParameterToDPoint3dDerivatives
                    (
                    pIntegrationParams->pConic,
                    pPoint,
                    pTangent,
                    NULL,
                    theta
                    );
    return true;
    }

#ifdef abc
static bool    inverseBezierInterpolate0
(
double *pFraction,
const double    *pdYArray,
int             numXY,
double          targetY
)
    {
    double pole[MAX_BEZIER_ORDER];
    double root[MAX_BEZIER_ORDER];
    int numRoot;
    int i;
    *pFraction = 0.0;

    if (   numXY <= MAX_BEZIER_ORDER
        && bsiBezier_univariateInterpolationPoles (pole, pYArray, numXY)
        )
        {
        for (i = 0; i < numXY; i++)
            pole[i] -= targetY;
        if (   bsiBezier_univariateRoots (root, &numRoot, pole, numXY)
            && numRoot == 1
            )
            {
            *pFraction = root[0];
            return true;
            }
        }
    return false;
    }
#else
static bool    inverseBezierInterpolate
(
double *pFraction,
const double    *pdYArray,
int             numXY,
double          y0,
double          yTarget,
double          deltaX
)
    {
    double pole[MAX_BEZIER_ORDER];
    double root[MAX_BEZIER_ORDER];
    double integralPole[MAX_BEZIER_ORDER];
    int numRoot;
    int integralOrder = numXY + 1;
    double deltaY = yTarget - y0;
    *pFraction = 0.0;

    if (   numXY + 1 <= MAX_BEZIER_ORDER
        && bsiBezier_univariateInterpolationPoles (pole, pdYArray, numXY)
        )
        {
        bsiBezier_univariateIntegralPoles (integralPole, -deltaY, deltaX, pole, numXY);

        if (   bsiBezier_univariateRoots (root, &numRoot, integralPole, integralOrder)
            && numRoot == 1
            )
            {
            *pFraction = root[0];
            return true;
            }
        }
    return false;
    }
#endif
/*---------------------------------------------------------------------------------**//**
* Inverse interpolation in accumulated distance arrays.  Inversion is by
* simple linear interpolation in each interval.
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    als_cb_receiveIncrementalArcLengths
(
        double                  length0,
        double                  *pX,
        double                  *pTangentMagnitude,
        double                  *pY,
        int                     numLength,
const   ALSIntegrationParams    *pIntegrationParams
)
    {
    ALSParams *pParams = pIntegrationParams->pParams;
    double    xx, fraction;
    DPoint3d    point, tangent;
    double nextY;
    double baseY;
    double tailY;
    double s, u;
    double h = pX[numLength-1] - pX[0];

    /* On entry and exit:
        pParams->distanceToNextOutput = distance to next point.  If a previous interval
            moved towards the point, the length within that interval was already
            decremented away from pParams->distanceToNextOutput.

       nextY and baseY are measured only within the current interval
    */
    int i = 1;
    nextY = pParams->distanceToNextOutput;
    baseY = 0.0;
    while (!als_allDone (pParams))
        {
        if (nextY > pY[numLength - 1])
            {
            tailY = pY[numLength-1] - baseY;
            pParams->distanceToNextOutput -= tailY;
            break;
            }
        else if (pY[i] >= nextY)
            {
            if (inverseBezierInterpolate (&fraction, pTangentMagnitude, numLength, pY[0], nextY, h))
                {
                xx = pX[0] + fraction * (pX[numLength-1] -pX[0]);
                }
            else
                {
                s = (pY[i] == pY[i-1]) ? 0.0 : ((nextY - pY[i-1]) / (pY[i] - pY[i-1]));
                xx = pX[i-1] + s * (pX[i] - pX[i-1]);
                }
            pIntegrationParams->cb_pointAndTangent (pIntegrationParams, &point, &tangent, xx);
            u = (xx - pIntegrationParams->normalizationParam0)
              / (pIntegrationParams->normalizationParam1 - pIntegrationParams->normalizationParam0);
            double uOut = pIntegrationParams->knot0 + u * (pIntegrationParams->knot1 - pIntegrationParams->knot0);
            als_addDPoint3d (pParams, &point, &tangent, uOut);
            baseY = nextY;
            nextY = baseY + pParams->distanceToNextOutput;
            }
        else
            {
            i++;
            if (i >= numLength)
                {
                tailY = pY[numLength-1] - baseY;
                pParams->distanceToNextOutput -= tailY;
                break;
                }
            }
        }

    return !als_allDone (pParams);
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void als_genericCurveIntegrationDriver
(
        ALSIntegrationParams    *pIntegrationParams,
        double                  x0,
        double                  x1,
        double                  knot0 = 0.0,
        double                  knot1 = 1.0
)
    {
    DPoint3d startPoint, startTangent, endPoint, endTangent;

    int count;
    double totalLength;
    double error;
    ALSParams   *pParams = pIntegrationParams->pParams;
    pIntegrationParams->normalizationParam0 = x0;
    pIntegrationParams->normalizationParam1 = x1;
    pIntegrationParams->knot0 = knot0;
    pIntegrationParams->knot1 = knot1;

    pIntegrationParams->cb_pointAndTangent (pIntegrationParams, &startPoint, &startTangent, x0);
    pIntegrationParams->cb_pointAndTangent (pIntegrationParams, &endPoint, &endTangent, x1);

    als_recordStartPoint (pParams, &startPoint, &startTangent);

    bsiMath_recursiveIncremantalNewtonCotes5 (
                        &totalLength, &error, &count,
                        x0, x1,
                        0.0, pParams->NCRelTol,
                        (PFScalarIntegrand)pIntegrationParams->cb_tangentMagnitude,
                        (PFExtendScalarIntegration)als_cb_receiveIncrementalArcLengths,
                        pIntegrationParams
                        );

    als_recordEndPoint (pParams, &endPoint, &endTangent, 0.0);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void als_DEllipse3d
(
        ALSParams               *pParams,
const   DEllipse3d              *pEllipse
)
    {
    double startAngle   = pEllipse->start;
    double endAngle     = startAngle + pEllipse->sweep;
    ALSIntegrationParams params;

    params.pEllipse = pEllipse;
    params.pParams  = pParams;
    params.unusedLength = 0.0;
    params.cb_pointAndTangent = als_cb_DEllipse3d_pointAndTangent;
    params.cb_tangentMagnitude = als_cb_DEllipse3d_tangentMagnitude;
    als_genericCurveIntegrationDriver (&params, startAngle, endAngle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void als_DConic4d
(
        ALSParams               *pParams,
const   DConic4d                *pConic
)
    {
    double startAngle   = pConic->start;
    double endAngle     = startAngle + pConic->sweep;
    ALSIntegrationParams params;

    params.pConic = pConic;
    params.pParams  = pParams;
    params.unusedLength = 0.0;
    params.cb_pointAndTangent  = als_cb_DConic4d_pointAndTangent;
    params.cb_tangentMagnitude = als_cb_DConic4d_tangentMagnitude;
    als_genericCurveIntegrationDriver (&params, startAngle, endAngle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void als_bezierDPoint4d
(
        ALSParams               *pParams,
const   DPoint4d                *pPoleArray,
        int                     order,
        double                  knot0,
        double                  knot1
)
    {
    ALSIntegrationParams params;

    params.pPoleArray = pPoleArray;
    params.order      = order;
    params.pParams  = pParams;
    params.unusedLength = 0.0;
    params.cb_pointAndTangent  = als_cb_bezierDPoint4d_pointAndTangent;
    params.cb_tangentMagnitude = als_cb_bezierDPoint4d_tangentMagnitude;
    als_genericCurveIntegrationDriver (&params, 0.0, 1.0, knot0, knot1);
    }


/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void als_parseConic
(
        ALSParams               *pParams,
GraphicsPointArrayCP pSource,
        int                     i0,
        int                     i1
)
    {
    //double length = 0.0;
    //int i = i0;
    DConic4d conic;
    DEllipse3d ellipse;
    bool    isEllipse;

    if (jmdlGraphicsPointArray_getDEllipse3d (pSource, &i0, &conic, &ellipse, &isEllipse, NULL, NULL))
        {
        if (!isEllipse)
            {
            als_DConic4d (pParams, &conic);
            }
        else if (bsiDEllipse3d_isCircular(&ellipse))
            {
            als_circularArc (pParams, &ellipse);
            }
        else
            {
            als_DEllipse3d (pParams, &ellipse);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void als_parseBezier
(
        ALSParams               *pParams,
GraphicsPointArrayCP pSource,
        int                     i0,
        int                     i1
)
    {
    int numPole;
    DPoint4d poleArray[MAX_BEZIER_CURVE_ORDER];
    int iRead = i0;
    if (jmdlGraphicsPointArray_getBezier (pSource, &iRead, poleArray, &numPole, MAX_BEZIER_CURVE_ORDER))
        {
        als_bezierDPoint4d (pParams, poleArray, numPole, 0.0, 1.0);
        }
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      05/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void als_parseBspline
(
        ALSParams               *pParams,
GraphicsPointArrayCP pSource,
        int                     i0,
        int                     i1
)
    {
    DPoint4d poleArray[MAX_BEZIER_ORDER];
    int order;
    bool isNullInterval;
    double knot0, knot1;
    for (size_t spanIndex = 0;
        pSource->GetBezierSpanFromBsplineCurve (i0, spanIndex, poleArray, order,
                MAX_BEZIER_ORDER, isNullInterval, knot0, knot1); spanIndex++)
        {
        if (!isNullInterval)
            als_bezierDPoint4d (pParams, poleArray, order, knot0, knot1);
        }
    }




/*---------------------------------------------------------------------------------**//**
* Compute points at indicated spacings along the path and add them to the destination array.
*
*
* @param pDest <= array to receive points
* @param pSource => array of path geoemtry.
* @param pDistanceArray => array of distances between successive points.  All must be
*                       positive.    The start point of the curve is NOT output unless
*                       pDistanceArray[0] is zero.
* @param numDist => number of distances
* @param tolerance => relative tolerance.   If 0 is entered, a fairly crude tolerance
*                       is used.
* @param maxTangentChange => NOT USED.
* @param storeTangent   => if true, each point is followed by the curve tangent.
* @param
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addSpacedPoints
(
        GraphicsPointArray  *pDest,
GraphicsPointArrayCP pSource,
const   double                  *pDistanceArray,
        int                     numDist,
        double                  tolerance,
        double                  maxTangentChange,
        bool                    storeTangent
)
    {
    int i0, i1;
    int curveType;
    ALSParams params;

    /* special case zero distance to avoid integrator altogether -DA4 */
    if (numDist > 0 && pDistanceArray[0] == 0.0)
        {
        DPoint4d    point, tangent;
        if (   !jmdlGraphicsPointArray_primitiveFractionToDPoint4dTangent (pSource, &point, &tangent, 0, 0.0)
            || !jmdlGraphicsPointArray_addDPoint4d (pDest, &point)
            || (storeTangent && !jmdlGraphicsPointArray_addDPoint4d (pDest, &tangent)))
            {
            return;
            }

        pDistanceArray++;
        numDist--;
        }

    if (numDist <= 0)
        return;

    als_init (&params, pDest, pSource, pDistanceArray, numDist,
                        tolerance,
                        maxTangentChange,
                        storeTangent);

    for (i0 = 0;
         jmdlGraphicsPointArray_parseFragment (
                        pSource, &i1,
                        NULL, NULL,
                        &curveType, i0);
         i0 = i1 + 1)
        {
        params.primitiveIndex = i0;
        if (curveType == 0)
            {
            als_parsePolyline (&params, pSource, i0, i1);
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER) // BSPLINE_CODED
            {
            als_parseBezier (&params, pSource, i0, i1);
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
            {
            als_parseConic (&params, pSource, i0, i1);
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BSPLINE)
            {
            als_parseBspline (&params, pSource, i0, i1);
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* Compute a single point at specified distance from start.
* @param
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlGraphicsPointArray_pointAtDistance
(
GraphicsPointArray  *pSource,
DPoint3d                *pPoint,
double                  distance
)
    {
    GraphicsPointArray *pDest = jmdlGraphicsPointArray_grab ();
    DPoint4d hpoint;
    bool    boolstat;
    jmdlGraphicsPointArray_addSpacedPoints
                    (pDest, pSource, &distance, 1, 0.0, 0.0, false);
    boolstat = jmdlGraphicsPointArray_getDPoint4dWithMask (pDest, &hpoint, NULL, 0)
        && bsiDPoint4d_normalize (&hpoint, pPoint);
    jmdlGraphicsPointArray_drop (pDest);
    return boolstat;
    }


/*---------------------------------------------------------------------------------**//**
* Compute a single point at specified distance from start.
* @param
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlGraphicsPointArray_primitiveFractionAtDistance
(
GraphicsPointArray  *pSource,
int                     *pPrimitiveIndex,
double                  *pFraction,
DPoint3d                *pPoint,
double                  distance
)
    {
    GraphicsPointArray *pDest = jmdlGraphicsPointArray_grab ();
    GraphicsPoint gp;
    bool    boolstat;
    jmdlGraphicsPointArray_addSpacedPoints
                    (pDest, pSource, &distance, 1, 0.0, 0.0, false);
    boolstat = jmdlGraphicsPointArray_getGraphicsPoint (pDest, &gp, 0);
    if (pPoint)
        bsiDPoint4d_normalize (&gp.point, pPoint);
    if (pPrimitiveIndex)
        *pPrimitiveIndex = gp.userData;
    if (pFraction)
        *pFraction = gp. a;
    jmdlGraphicsPointArray_drop (pDest);
    return boolstat;
    }


/*---------------------------------------------------------------------------------**//**
* Build parameters for contiguous bezier chunks with identical end/start points and
*       order.
* @param pGPA <=> array to update.
* @param pIndex1 <= on output, the final index of the final bezier
* @param index0 => on input, start index of the first bezier.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
static  bool       jmdlGraphicsPointArray_parameterizeContiguousBeziers
(
GraphicsPointArray  *pGPA,
int *pI1,
int i0
)
    {
    int j0, j1, jstep;
    int i;
    int curveType;
    int numBezier;
    DPoint4d point0, point1;
    double a;
    GraphicsPoint *pBuffer = jmdlGraphicsPointArray_getPtr (pGPA, 0);

    if (jmdlGraphicsPointArray_parseFragment (
                    pGPA, &j1,
                    NULL, &point0,
                    &curveType, i0)
        && curveType == HPOINT_MASK_CURVETYPE_BEZIER)   // BSPLINE_NOT_REQUIRED
        {
        jstep = j1 - i0;
        /* Count the upcoming contiguous beziers to find out the step size ... */
        for (j0 = j1 + 1, numBezier = 0;
             jmdlGraphicsPointArray_parseFragment (
                            pGPA, &j1,
                            NULL, &point1,
                            &curveType, j0)
            && curveType == HPOINT_MASK_CURVETYPE_BEZIER    // BSPLINE_NOT_REQUIRED
            && j1 - j0 == jstep
            && bsiDPoint4d_pointEqual (&point0, &point1);
            j0 = j1 + 1, point0 = point1
            )
            {
            point0 = point1;
            numBezier++;
            }

        /* ... and sweep again to install the parameters. */
        i = 0;
        a = 1.0 / numBezier;
        for (j0 = i0, i = 0;
             jmdlGraphicsPointArray_parseFragment (
                            pGPA, &j1,
                            NULL, NULL,
                            &curveType, j0);
            j0 = j1 + 1
            )
            {
            pBuffer[j0].a = i * a;
            pBuffer[j1].a = (i + 1) * a;
            }
        *pI1 = j1;
        return true;
        }
    return false;
    }



/*---------------------------------------------------------------------------------**//**
* Scan the GPA and assign parameter ("a") values to start and end points of each
*  primitive.
*           Linestrings are 0 at first vertex, 1 at last, uniformly spaced
*                   in between.
*           Arcs are ALWAYS 0 to 1.
*           Contiguous bezier chunks with exactly matching end/start points
*                   are 0 to 1 over the entire range with uniform space per bezier.
*                   Beware that if the beziers came from a general NURBS curve
*                   these might NOT correspond to the original knots.
* @param pGPA <=> array to update.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_setApplicationParameters
(
GraphicsPointArray  *pGPA
)
    {
    int i0, i1;
    int i;
    double a;
    int curveType;
    GraphicsPoint *pBuffer = jmdlGraphicsPointArray_getPtr (pGPA, 0);

    for (i0 = 0;
         jmdlGraphicsPointArray_parseFragment (
                        pGPA, &i1,
                        NULL, NULL,
                        &curveType, i0);
         i0 = i1 + 1)
        {

        if (curveType == 0 && i1 > i0)
            {
            a = 1.0 / (i1 - i0);
            for (i = i0; i < i1; i++)   // BUG ? Final point not set?
                pBuffer[i].a = (i - i0) * a;
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER) // BSPLINE_CODED
            {
            jmdlGraphicsPointArray_parameterizeContiguousBeziers (pGPA, &i1, i0);
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
            {
            pBuffer[i0].a = 0.0;
            pBuffer[i1].a = 1.0;
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BSPLINE)
            {
            // Copy {b} field (knots) to {a} field.
            for (i = i0; i <= i1; i++)
                pBuffer[i].a = pBuffer[i].b;
            }
        }
    }



typedef enum {DCPOINT_BY_PRIMITIVE, DCPOINT_EXACT, DCPOINT_ABSTOL} DCPointCondition;
typedef enum {DCTANGENT_NOTEST, DCTANGENT_ABSTOL} DCTangentCondition;

typedef struct
    {
    DPoint3d refPoint;
    double   hitRadius;
    bool     testHitRadius;

    double pointAbsTol;
    double tangentAbsTol;
    int    pointTestType;
    int    tangentTestType;
    bool    oldPointDefined;
    int    workDimension;
    int      oldi0;
    DPoint4d oldRawPoint;
    DPoint3d oldTangent;
    GraphicsPointArrayCP pSource;
    GraphicsPointArray *pDest;
    } DiscontinuityContext;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/00
+---------------+---------------+---------------+---------------+---------------+------*/
static void dc_init
(
DiscontinuityContext *pContext,
GraphicsPointArray  *pDest,
GraphicsPointArrayCP pSource,
const DPoint3d *pRefPoint,
double hitRadius,
double  pointTol,
double  tangentTol,
int     workDimension
)
    {
    DRange3d range;
    memset (pContext, 0, sizeof (DiscontinuityContext));
    pContext->pSource = pSource;
    pContext->pDest   = pDest;

    if (pRefPoint && hitRadius > 0.0)
        {
        pContext->testHitRadius = true;
        pContext->refPoint = *pRefPoint;
        pContext->hitRadius = hitRadius;
        }

    pContext->workDimension = workDimension;
    if (pointTol == 0.0)
        {
        pContext->pointTestType = DCPOINT_EXACT;
        pContext->pointAbsTol = 0.0;
        }
    else if (pointTol > 0.0)
        {
        pContext->pointTestType = DCPOINT_ABSTOL;
        pContext->pointAbsTol = pointTol;
        }
    else if (pointTol > -1.0)
        {
        pContext->pointTestType = DCPOINT_ABSTOL;
        bsiDRange3d_init (&range);
        jmdlGraphicsPointArray_extendDRange3d (pSource, &range);
        if (workDimension == 2)
            range.low.z = range.high.z = 0.0;
        pContext->pointAbsTol = fabs (pointTol) *
                bsiDRange3d_getLargestCoordinate (&range);
        }
    else
        {
        pContext->pointTestType = DCPOINT_BY_PRIMITIVE;
        }

    if (tangentTol < 0.0)
        {
        pContext->tangentTestType = DCTANGENT_NOTEST;
        }
    else
        {
        pContext->tangentTestType = DCTANGENT_ABSTOL;
        pContext->tangentAbsTol = tangentTol;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/00
+---------------+---------------+---------------+---------------+---------------+------*/
static void dc_flush
(
DiscontinuityContext *pContext
)
    {
    GraphicsPoint gp1;
    DPoint4d point;

    if (pContext->oldPointDefined)
        {
        point = pContext->oldRawPoint;
        bsiGraphicsPoint_init
                    (
                    &gp1,
                    point.x, point.y, point.z, point.w,
                    1.0, 0, pContext->oldi0
                    );
        jmdlGraphicsPointArray_addGraphicsPoint (pContext->pDest, &gp1);
        jmdlGraphicsPointArray_markPoint(pContext->pDest);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/00
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    dc_samePoint
(
DiscontinuityContext *pContext,
const GraphicsPoint *pGP0,
const GraphicsPoint *pGP1
)
    {
    GraphicsPoint gp0 = *pGP0;
    GraphicsPoint gp1 = *pGP1;
    DPoint3d tangent0, tangent1;
    bool    duplicatePoint = false;
    bool    duplicateTangent = false;

    if (pContext->workDimension == 2)
        gp0.point.z = gp1.point.z = 0.0;

    if (pContext->pointTestType == DCPOINT_EXACT)
        {
        duplicatePoint = bsiDPoint4d_pointEqual (&gp0.point, &gp1.point);
        }
    else if (pContext->pointTestType == DCPOINT_ABSTOL)
        {
        duplicatePoint = bsiDPoint4d_realDistance (&gp0.point, &gp1.point) < pContext->pointAbsTol;
        }

    if (!duplicatePoint)
        return false;

    if (pContext->tangentTestType == DCTANGENT_ABSTOL
        && jmdlGraphicsPointArray_primitiveFractionToDPoint3dTangent
                        (pContext->pSource, NULL, &tangent0, gp0.userData, 0.0)
        && jmdlGraphicsPointArray_primitiveFractionToDPoint3dTangent
                        (pContext->pSource, NULL, &tangent1, gp1.userData, 1.0))
        {
        if (pContext->workDimension == 2)
            {
            tangent0.z = tangent1.z = 0.0;
            }
        duplicateTangent = bsiDPoint3d_angleBetweenVectors (&tangent0, &tangent1) < pContext->tangentAbsTol;
        return duplicateTangent;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/00
+---------------+---------------+---------------+---------------+---------------+------*/
static void dc_testAndUpdate
(
DiscontinuityContext *pContext,
int     i0,
int     i1,
int     curveType,
DPoint4d    *pPoint0,
DPoint4d    *pPoint1
)
    {
    GraphicsPoint gp0;
    DPoint3d currTangent, oldTangent;
    DPoint4d oldPoint;
    bool    duplicateStart = false;
    bool    duplicateTangent = true;
    bool    outputThisPoint;
    oldPoint = pContext->oldRawPoint;
    if (pContext->workDimension == 2)
        oldPoint.z = pPoint0->z;


    bsiGraphicsPoint_init (&gp0, pPoint0->x, pPoint0->y, pPoint0->z, pPoint0->w,
                    0.0, 0, i0);

    if (pContext->pointTestType == DCPOINT_EXACT)
        {
        duplicateStart =  pContext->oldPointDefined
                       && bsiDPoint4d_pointEqual (pPoint0, &pContext->oldRawPoint);
        }
    else if (pContext->pointTestType == DCPOINT_ABSTOL)
        {
        duplicateStart = pContext->oldPointDefined
                        && bsiDPoint4d_realDistance (pPoint0, &pContext->oldRawPoint) < pContext->pointAbsTol;
        }

    if (duplicateStart && pContext->tangentTestType == DCTANGENT_ABSTOL
        && jmdlGraphicsPointArray_primitiveFractionToDPoint3dTangent
                        (pContext->pSource, NULL, &currTangent, i0, 0.0))
        {
        oldTangent = pContext->oldTangent;
        if (pContext->workDimension == 2)
            {
            currTangent.z = oldTangent.z = 0.0;
            }
        duplicateTangent = bsiDPoint3d_angleBetweenVectors (&currTangent, &oldTangent) < pContext->tangentAbsTol;
        }

    if (pContext->tangentTestType == DCTANGENT_ABSTOL
        && jmdlGraphicsPointArray_primitiveFractionToDPoint3dTangent
                        (pContext->pSource, NULL, &currTangent, i0, 1.0))
        {
        pContext->oldTangent = currTangent;
        }


    outputThisPoint = !(duplicateStart && duplicateTangent);
    if (pContext->oldPointDefined && !duplicateStart)
            dc_flush (pContext);

    if (outputThisPoint)
        {
        jmdlGraphicsPointArray_addGraphicsPoint (pContext->pDest, &gp0);
        jmdlGraphicsPointArray_markPoint(pContext->pDest);
        }


    pContext->oldRawPoint = *pPoint1;
    pContext->oldi0 = i0;
    pContext->oldPointDefined = true;

    }


/*---------------------------------------------------------------------------------**//**
* Search pSource for points of specified discontinuity.
* Optionally ignore points too distant from a given point.
* @param pDest => write data to here.
* @param pSource => read data from here.
* @param pRefPoint => optional reference point for distance calculations.   If
*           null, all discontinuity points are appended to the destination.
* @param hitRadius => radius to use with pRefPoint.  Ignored if pRefPoint is null or
*               hitRadius is negative.
* @param workDimension => 2 for xy only, 3 for xyz.
* @param pointTolerance => tolerance to consider two consecutive endpoints identical.
*           Tolerance -2 means copy all primitive start and end points (including
*           intermediate points of linestrings) without computed distance test.
*           Tolerance -1 < e < 0 means compute the tolerance as e times the
*           range of the points in the array.
* @param angleTolerance => tolerance (radians) to consider incoming and outgoing tangents
*           at a common point different.  Negative means no tangent test.
* @bsimethod                                                    EarlinLutz      05/00
+---------------+---------------+---------------+---------------+---------------+------*/
static  void    jmdlGraphicsPointArray_addDiscontinuityPointsExt
(
        GraphicsPointArray  *pDest,
        DiscontinuityContext    *pDiscontinuityContext,
GraphicsPointArrayCP pSource,
const   DPoint3d                *pRefPoint,
        double                  hitRadius,
        int                     workDimension,
        double                  pointTolerance,
        double                  angleTolerance
)
    {
    int curr0, curr1;
    int curveType;
    DPoint4d currRawPoint[2];
    dc_init (pDiscontinuityContext, pDest, pSource,
                        pRefPoint, hitRadius,
                        pointTolerance, angleTolerance, workDimension);

    for (curr0 = curr1 = -1;
        jmdlGraphicsPointArray_parsePrimitiveAfter (pSource, &curr0, &curr1,
                        &currRawPoint[0],
                        &currRawPoint[1],
                        &curveType,
                        curr1);
        )
        {
        dc_testAndUpdate    (
                            pDiscontinuityContext,
                            curr0, curr1, curveType,
                            &currRawPoint[0],
                            &currRawPoint[1]
                            );
        }
        dc_flush (pDiscontinuityContext);
    }


/*---------------------------------------------------------------------------------**//**
* Search pSource for points of specified discontinuity.
* Optionally ignore points too distant from a given point.
* @param pDest => write data to here.
* @param pSource => read data from here.
* @param pRefPoint => optional reference point for distance calculations.   If
*           null, all discontinuity points are appended to the destination.
* @param hitRadius => radius to use with pRefPoint.  Ignored if pRefPoint is null or
*               hitRadius is negative.
* @param workDimension => 2 for xy only, 3 for xyz.
* @param pointTolerance => tolerance to consider two consecutive endpoints identical.
*           Tolerance -2 means copy all primitive start and end points (including
*           intermediate points of linestrings) without computed distance test.
*           Tolerance -1 < e < 0 means compute the tolerance as e times the
*           range of the points in the array.
* @param angleTolerance => tolerance (radians) to consider incoming and outgoing tangents
*           at a common point different.  Negative means no tangent test.
* @bsimethod                                                    EarlinLutz      05/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    jmdlGraphicsPointArray_addDiscontinuityPoints
(
        GraphicsPointArray  *pDest,
GraphicsPointArrayCP pSource,
const   DPoint3d                *pRefPoint,
        double                  hitRadius,
        int                     workDimension,
        double                  pointTolerance,
        double                  angleTolerance
)
    {
    DiscontinuityContext dc;
    jmdlGraphicsPointArray_addDiscontinuityPointsExt (pDest, &dc, pSource, pRefPoint, hitRadius,
                        workDimension, pointTolerance, angleTolerance);
    }

/*---------------------------------------------------------------------------------**//**
* Find the vertex which is closest to a given point.  All points in the GPA
* are considered as isolated vertices, not parts of curve definitions.
* This function is intended to be used as a post-process to search
* arrays of points returned by other functions.
* @param pGP <= complete point record.
* @param pPoint => base point for distance computations.
* @param workDimension => 2 or 3.
* @return index of closest point; -1 if empty array.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int             jmdlGraphicsPointArray_closestVertexExt
(
GraphicsPointArrayCP pSource,
      GraphicsPoint                 *pGP,
const DPoint3d                      *pPoint,
      int                           workDimension
)
    {
    GraphicsPoint gp, gpSave, gpWork;
    int iCurr;
    int iSave = -1;
    double dSave = 0.0;
    double dCurr;
    memset (&gpSave, 0, sizeof (GraphicsPoint));
    for (iCurr = 0; jmdlGraphicsPointArray_getGraphicsPoint (pSource, &gp, iCurr); iCurr++)
        {
        gpWork = gp;
        if (workDimension == 2)
            gpWork.point.z = pPoint->z * gp.point.w;
        if (bsiDPoint4d_realDistanceSquaredDPoint3d (&gpWork.point, &dCurr, pPoint)
                && (iCurr == 0 ||dCurr < dSave))
            {
            dSave = dCurr;
            iSave = iCurr;
            gpSave = gp;
            }
        }

    if (pGP)
        {
        *pGP = gpSave;
        }
    return iSave;
    }


/*---------------------------------------------------------------------------------**//**
* Find the vertex which is closest to a given point.  All points in the GPA
* are considered as isolated vertices, not parts of curve definitions.
* This function is intended to be used as a post-process to search
* arrays of points returned by other functions.
* @param pGP <= complete point record.
* @param pPoint => base point for distance computations.
* @return index of closest point; -1 if empty array.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int             jmdlGraphicsPointArray_closestVertex
(
GraphicsPointArrayCP pSource,
      GraphicsPoint                 *pGP,
const DPoint3d                      *pPoint
)
    {
    return jmdlGraphicsPointArray_closestVertexExt (pSource, pGP, pPoint, 3);
    }


/*---------------------------------------------------------------------------------**//**
* Find the discontinuities "before" and "after" a specified primitive.
* @param pIndex0 <= index of start primitive.
* @param pIndex1 <= index of end primitive.
* @param *pClosed <= true if start of pIndex0 matches end of pIndex1.
* @param index => index for start of search.
* @param workDimension => 2 for xy only, 3 for xyz.
* @param pointTolerance => tolerance to consider two consecutive endpoints identical.
*           Tolerance -2 means copy all primitive start and end points (including
*           intermediate points of linestrings) without computed distance test.
*           Tolerance -1 < e < 0 means compute the tolerance as e times the
*           range of the points in the array.
* @param angleTolerance => tolerance (radians) to consider incoming and outgoing tangents
*           at a common point different.  Negative means no tangent test.
* @return true if indx is the index of a primitive on a path which starts at
*               primitive pIndex0 at parameter 0.0 and ends at pIndex1 at parameter 1.0.
*
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            jmdlGraphicsPointArray_findPathLimits
(
GraphicsPointArrayCP pSource,
      int                           *pIndex0,
      int                           *pIndex1,
      bool                          *pClosed,
      int                           index,
      int                           workDimension,
      double                        pointTolerance,
      double                        angleTolerance
)
    {
    int i;
    int index0, index1;
    GraphicsPoint gp, gp0, gp1;
    DiscontinuityContext dc;  /* Just for setting up tolerance */

    GraphicsPointArray  *pDiscontinuities = jmdlGraphicsPointArray_grab ();
    jmdlGraphicsPointArray_addDiscontinuityPointsExt (pDiscontinuities, &dc,
                        pSource, NULL, 0.0, workDimension, pointTolerance, angleTolerance);

    if (pIndex0)
        *pIndex0 = -1;
    if (pIndex1)
        *pIndex1 = -1;
    if (pClosed)
        *pClosed = false;
    if (index < 0)
        return false;

    if (jmdlGraphicsPointArray_getCount (pDiscontinuities) < 2)
        return false;
    index0 = index1 = -1;
    for (i = 0; jmdlGraphicsPointArray_getGraphicsPoint (pDiscontinuities, &gp, i)
                && (gp.userData < index
                    || (gp.userData == index && gp.a == 0.0));
                i++)
        {
        index0 = gp.userData;
        gp0 = gp;
        }

    if (jmdlGraphicsPointArray_getGraphicsPoint (pDiscontinuities, &gp, i)
        && (gp.userData >= index && gp.a == 1.0))
        {
        index1 = gp.userData;
        gp1 = gp;
        }

    jmdlGraphicsPointArray_drop (pDiscontinuities);
    if (index0 <= index && index1 >= index)
        {
        if (pIndex0)
            *pIndex0 = index0;
        if (pIndex1)
            *pIndex1 = index1;
        if (pClosed)
            *pClosed = dc_samePoint (&dc, &gp0, &gp1);
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGPA_printCoordinate

(
char const *pString0,
double x,
char const *pString1
)
    {
    double ax = fabs (x);
    static double s_smallNum = 1.0e-3;
    int ix = (int) ax;

    if (pString0)
        printf ("%s", pString0);

    if (ax == ix)
        {
        if (x < 0.0)
            printf ("-");
        printf ("%d", ix);
        }
    else if (ax < s_smallNum)
        {
        printf ("%le", x);
        }
    else if (ax < 1.0)
        {
        printf("%12.9lf", x);
        }
    else if (ax < 1000.0)
        {
        printf("%12.6lf", x);
        }
    else if (ax < 1.0e6)
        {
        printf("%12.3lf", x);
        }
    else if (ax < 1.0e9)
        {
        printf("%12.1lf", x);
        }
    else
        {
        printf ("%le", x);
        }

    if (pString1)
        printf ("%s", pString1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_print

(
GraphicsPointArrayCP pGPA,
bool        parse
)
    {
    int i;
    GraphicsPoint gp;
    for (i = 0; jmdlGraphicsPointArray_getGraphicsPoint (pGPA, &gp, i); i++)
        {
        printf (" %4d ", i);
        jmdlGPA_printCoordinate ("[", gp.point.x, "");
        jmdlGPA_printCoordinate (",", gp.point.y, "");
        jmdlGPA_printCoordinate (",", gp.point.z, "");
        jmdlGPA_printCoordinate (",", gp.point.w, "]");
        jmdlGPA_printCoordinate (" ", gp.a, " ");
        printf (" %x %x\n", gp.mask, gp.userData);
        }
    }

static void updateMax
(
double *pXYZMax,
double *pWMax,
const DPoint4d *pPoint
)
    {
    double a;
    a = fabs (pPoint->x);
    if (a > *pXYZMax)
        *pXYZMax = a;
    a = fabs (pPoint->y);
    if (a > *pXYZMax)
        *pXYZMax = a;
    a = fabs (pPoint->z);
    if (a > *pXYZMax)
        *pXYZMax = a;

    a = fabs (pPoint->w);
    if (a > *pWMax)
        *pWMax = a;
    }

/*---------------------------------------------------------------------------------**//**
* Test if two graphics point arrays have the same geometry, using point by point
*       testing.
* If both tolerances are zero, a near-machine-precision relative tolerance is used.
* @param pGPA0 => first array.
* @param pGAP1 => second array.
* @param xyzAbsTol => absolute distance tolerance for xyz parts.
* @param relTol => relative tolerance.
* @return true if the two arrays have the same number and type of geometry and coordinates
*               are within tolerance.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_sameGeometryPointByPoint
(
GraphicsPointArrayCP pGPA0,
GraphicsPointArrayCP pGPA1,
double xyzAbsTol,
double relTol
)
    {
    int i;
    const GraphicsPoint *pBuffer0 = jmdlGraphicsPointArray_getConstPtr (pGPA0, 0);
    const GraphicsPoint *pBuffer1 = jmdlGraphicsPointArray_getConstPtr (pGPA1, 0);
    int n0 = jmdlGraphicsPointArray_getCount (pGPA0);
    int n1 = jmdlGraphicsPointArray_getCount (pGPA1);
    double xyzMax, wMax;
    double xyzTol, wTol;

     if (n0 != n1)
        return false;

    if (xyzAbsTol < 0.0)
        xyzAbsTol = 0.0;
    if (relTol < 0.0)
        relTol = 0.0;

    if (xyzAbsTol == 0.0 && relTol == 0.0)
        relTol = bsiTrig_smallAngle ();

    /* Test point types first .. */
    for (i = 0; i < n0; i++)
        {
        if ((pBuffer0[i].mask & HPOINT_MASK_CURVE_BITS) != (pBuffer1[i].mask & HPOINT_MASK_CURVE_BITS))
            return false;
        }

    /* Bad bad bad.  There's no really good point-by-point tolerance in weighted world.
        Compromise:  If only abstol is given, use it directly on xyz, and still use a
                     small (absolute) machine tolerance on weights.
       Also use wTol for {b} field comparison.
    */
    if (relTol > 0.0)
        {
        xyzMax = wMax = 0.0;
        for (i = 0; i < n0; i++)
            {
            updateMax (&xyzMax, &wMax, &pBuffer0[i].point);
            updateMax (&xyzMax, &wMax, &pBuffer1[i].point);
            }
        xyzTol = xyzAbsTol + relTol * xyzMax;
        wTol   = bsiTrig_smallAngle () * wMax;
        }
    else
        {
        xyzTol = xyzAbsTol;
        wTol   = bsiTrig_smallAngle ();
        }

    for (i = 0; i < n0; i++)
        {
        if (!bsiDPoint4d_pointEqualMixedTolerance (&pBuffer0[i].point, &pBuffer1[i].point, xyzTol, wTol))
            return false;
        if (fabs (pBuffer0[i].b - pBuffer1[i].b) > wTol)
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
@description Test if the gpa is closed.
    Tolerance for point comparison is sum of abstol with reltol times largest coordinate
        in the gpa.
@param pSource IN GPA to examine
@param abstol IN absolute tol for distinct points
@param reltol IN relative tol for distinct pounts.
                The reltol used will be the larger of the parameter and bsiTrig_smallAngle ().
@return true if the two arrays have the same number and type of geometry and coordinates
               are within tolerance.
@bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool       jmdlGraphicsPointArray_isClosed
(
GraphicsPointArrayCP pSource,
double abstol,
double reltol
)
    {
    int curr0, curr1;
    int curveType;
    double tol, tol2;
    double d0, d1, dSelf;
    DRange3d range;
    bool    bNewPath;
    DPoint4d pathStart, oldEnd, newStart, newEnd;
    if (abstol < 0.0)
        abstol = 0.0;
    if (reltol < bsiTrig_smallAngle ())
        reltol = bsiTrig_smallAngle ();

    bsiDRange3d_init (&range);
    jmdlGraphicsPointArray_extendDRange3d (pSource, &range);
    tol = abstol + reltol * bsiDRange3d_getLargestCoordinate (&range);
    tol2 = tol * tol;

    for (curr1 = -1, bNewPath = true;
        jmdlGraphicsPointArray_parsePrimitiveAfter (
                pSource,
                &curr0,
                &curr1,
                &newStart,
                &newEnd,
                &curveType,
                curr1);
        oldEnd = newEnd
        )
        {
        if (bsiDPoint4d_realDistanceSquared (&newStart, &dSelf, &newEnd)
			&& curveType != 0
            && dSelf < tol2)
            {
            // This primitive is closed.
            bNewPath = true;
            }
        else if (bNewPath)
            {
            // Just starting out (or perhaps restarting after prior closure)
            pathStart = newStart;
            bNewPath = false;
            }
        else
            {
            // Connected to predecessor?
            if (bsiDPoint4d_realDistanceSquared (&oldEnd, &d0, &newStart)
                && d0 > tol2)
                return false;
            // Return to start?
            if (bsiDPoint4d_realDistanceSquared (&newEnd, &d1, &pathStart)
                && d1 <= tol2)
                {
                bNewPath = true;
                }
            }
        }
    // If closed, we should have finished by coming back to start and setting bNewPath
    return bNewPath;
    }
END_BENTLEY_GEOMETRY_NAMESPACE