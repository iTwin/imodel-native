/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/gpa/gp_perptan.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#define BEZIER_CONVERSION_POLES 5

static double s_sameFractionTol = 1.0e-12;

typedef struct
    {
    GraphicsPointArrayCP pSource[2];
    GraphicsPointArrayP pCurves;
    GraphicsPointArrayP pPointCollector0;
    GraphicsPointArrayP pPointCollector1;

    bool                        bPerpEnable[2];
    bool                        bTanEnable[2];
    bool                        bExtend;
    int                         workdim;
    bool                        bRemoveIntersections;
    double                      intersectionTolerance;
    double                      maxSegmentLength;

    bool RejectBySegmentLength (double segmentLength)
        {
        if (bRemoveIntersections
           && segmentLength < intersectionTolerance)
            return true;
        if (maxSegmentLength > 0.0 && segmentLength > maxSegmentLength)
            return true;
        return false;
        }

    } GPAPerpTanContext;

typedef bool    (*CurveCurveFunc)
(
double *pf,
double *pd0,
double *pd1,
const DPoint3d  *pX0,
const DPoint3d  *pdX0,
const DPoint3d  *pddX0,
const DPoint3d  *pX1,
const DPoint3d  *pdX1,
const DPoint3d  *pddX1,
void            *pVoid,
int             workdim
);

#define GPAPERPTAN_TANGENT (1)
#define GPAPERPTAN_PERP    (2)

#define GPAPERPTAN_SOURCE0  (0)
#define GPAPERPTAN_SOURCE1  (1)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/00
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    newtonStep
(
double *px0,
double *pxLimit0,
double *pxLimit1,
bool    *pConverged,
double f,
double df,
double absTol
)
    {
    double dx;
    double dx01 = *pxLimit1 - *pxLimit0;
    double dx0, dx1;
    double xNew;
    *pConverged = false;
    if (!bsiTrig_safeDivide (&dx, f, df, 0.0))
        return false;

    xNew = *px0 - dx;
    dx0 = xNew - *pxLimit0;
    dx1 = xNew - *pxLimit1;

    if (dx0 * dx1 < 0.0)
        {
        *px0 = xNew;
        *pConverged = fabs (dx) <= absTol;
        return true;
        }

    if (dx01 * dx0 > 0.0)
        {
        *px0 = *pxLimit1;
        }
    else
        {
        *px0 = *pxLimit0;
        }
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* Compute a derterminant of the form
*       | a0 b0 1 |
*       | a1 b1 1 |
*       | a2 b2 1 |
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static double unitDeterminant
(
double a0,
double b0,
double a1,
double b1,
double a2,
double b2
)
    {
    return (a1 * b2 - a2 * b1) + ( a2 * b0 - a0 * b2) + (a0 * b1 - a1 * b0);
    }

/*---------------------------------------------------------------------------------**//**
* Compute the derivative of
*       | a0 b0 1 |
*       | a1 b1 1 |
*       | a2 b2 1 |
* with respect to a variable which controls a1 and b1 and leaves others unchanged.
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static double unitDeterminantDerivative
(
double da0,
double db0,
double a1,
double b1,
double a2,
double b2
)
    {
    return (a2 * db0 - da0 * b2) + (da0 * b1 - a1 * db0);
    }

/*---------------------------------------------------------------------------------**//**
* Determine the circumcenter of three points, and optionally differentiate it
* with respect to parameters controlling each point.
* Points are input as xy parts of DPoint3d.  Center is returned as (x,y,w) in a DPoint3d.
* Actual center is (x/w, y/w).  Derivatives are (x',y',w').
* REMARK: For numerical stability, it is strongly recommended that one of the original
* point coordinates by used as the origin.
* @param pC => circumcenter
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void circumcenterTerms
(
DPoint3d    *pC,
DPoint3d    *pdCds0,
DPoint3d    *pdCds1,
DPoint3d    *pdCds2,
const DPoint3d    *pX0,
const DPoint3d    *pT0,
const DPoint3d    *pX1,
const DPoint3d    *pT1,
const DPoint3d    *pX2,
const DPoint3d    *pT2
)
    {
    double rr0, rr1, rr2;
    double x0, x1, x2;
    double y0, y1, y2;
    double dr;
    static double ax = 1.0;
    static double ay = -1.0;
    static double az = 1.0;

    rr0 = pX0->x * pX0->x + pX0->y * pX0->y;
    rr1 = pX1->x * pX1->x + pX1->y * pX1->y;
    rr2 = pX2->x * pX2->x + pX2->y * pX2->y;

    x0 = pX0->x;
    x1 = pX1->x;
    x2 = pX2->x;
    y0 = pX0->y;
    y1 = pX1->y;
    y2 = pX2->y;

    pC->x = 0.5 * ax * unitDeterminant (rr0, y0, rr1, y1, rr2, y2);
    pC->y = 0.5 * ay * unitDeterminant (rr0, x0, rr1, x1, rr2, x2);
    pC->z = az * unitDeterminant (x0, y0, x1, y1, x2, y2);

    if (pdCds0)
        {
        dr = 2.0 * (pX0->x * pT0->x + pX0->y * pT0->y);
        pdCds0->x =  0.5 * ax * unitDeterminantDerivative (dr, pT0->y, rr1, y1, rr2, y2);
        pdCds0->y =  0.5 * ay * unitDeterminantDerivative (dr, pT0->x, rr1, x1, rr2, x2);
        pdCds0->z =        az * unitDeterminantDerivative (pT0->x, pT0->y, x1, y1, x2, y2);
        }

    if (pdCds1)
        {
        dr = 2.0 * (pX1->x * pT1->x + pX1->y * pT1->y);
        pdCds1->x =  0.5 * ax * unitDeterminantDerivative (dr, pT1->y, rr2, y2, rr0, y0);
        pdCds1->y =  0.5 * ay * unitDeterminantDerivative (dr, pT1->x, rr2, x2, rr0, x0);
        pdCds1->z =        az * unitDeterminantDerivative (pT1->x, pT1->y, x2, y2, x0, y0);
        }

    if (pdCds2)
        {
        dr = 2.0 * (pX2->x * pT2->x + pX2->y * pT2->y);
        pdCds2->x =  0.5 * ax * unitDeterminantDerivative (dr, pT2->y, rr0, y0, rr1, y1);
        pdCds2->y =  0.5 * ay * unitDeterminantDerivative (dr, pT2->x, rr0, x0, rr1, x1);
        pdCds2->z =        az * unitDeterminantDerivative (pT2->x, pT2->y, x0, y0, x1, y1);
        }
    }


/*---------------------------------------------------------------------------------**//**
* f = (X * C.w - C) dot T
*  (X, C, T have x, y components for dot product)
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void circumcenterPerpRadiusFunction
(
double      *pf,
double      *pd0,
double      *pd1,
double      *pd2,
const DPoint3d    *pX0,
const DPoint3d    *pT0,
const DPoint3d    *pdT0,
const DPoint3d    *pC,
const DPoint3d    *pdCds0,
const DPoint3d    *pdCds1,
const DPoint3d    *pdCds2
)
    {
    double ax = pX0->x * pC->z - pC->x;
    double ay = pX0->y * pC->z - pC->y;
    *pf = ax * pT0->x + ay * pT0->y;

    if (pd0)
        *pd0 = (pT0->x * pC->z + pX0->x * pdCds0->z - pdCds0->x) * pT0->x + ax * pdT0->x
             + (pT0->y * pC->z + pX0->y * pdCds0->z - pdCds0->y) * pT0->y + ay * pdT0->y;

    if (pd1)
        *pd1 = (pX0->x * pdCds1->z - pdCds1->x) * pT0->x
             + (pX0->y * pdCds1->z - pdCds1->y) * pT0->y;

    if (pd2)
        *pd2 = (pX0->x * pdCds2->z - pdCds2->x) * pT0->x
             + (pX0->y * pdCds2->z - pdCds2->y) * pT0->y;
    }

static void tangentFunctions_pointPointCurve
(
double      *pf,
double      *pdf,
const DPoint3d    *pX0,
const DPoint3d    *pX1,
const DPoint3d    *pX2,
const DPoint3d    *pT2,
const DPoint3d    *pdT2
)
    {
    DPoint3d center, dCds;
    circumcenterTerms (&center, &dCds, NULL, NULL, pX2, pT2, pX0, NULL, pX1, NULL);
    circumcenterPerpRadiusFunction (pf, pdf, NULL, NULL, pX2, pT2, pdT2, &center, &dCds, NULL, NULL);
    }


static bool    evaluateCurveCurvePointFunction
(
double *pf,
double *pd0,
double *pd1,
const DPoint3d  *pX0,
const DPoint3d  *pdX0,
const DPoint3d  *pddX0,
const DPoint3d  *pX1,
const DPoint3d  *pdX1,
const DPoint3d  *pddX1,
const DPoint3d  *pX2,
int             dummyWorkdim
)
    {
    DPoint3d U0C, dCds0, dCds1;
    /* Should subtract X0 from everything here !!! */
    circumcenterTerms
                (
                &U0C, &dCds0, &dCds1, NULL,
                pX0, pdX0, pX1, pdX1, pX2, NULL
                );
    circumcenterPerpRadiusFunction (pf, pd0, pd1, NULL, pX0, pdX0, pddX0, &U0C, &dCds0, &dCds1, NULL);

    return true;
    }

static int jmdlGraphicsPointArray_primitiveOrder
(
GraphicsPointArrayCP pSource,
      int                       index
)
    {
    int curveType, index0, index1;
    if (jmdlGraphicsPointArray_parsePrimitiveAt (pSource, &index0, &index1, NULL, NULL, &curveType, index))
        {
        if (curveType == 0)
            return 2;
        if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
            return 3;
        if (curveType == HPOINT_MASK_CURVETYPE_BEZIER)  // BSPLINE_CODED
            return index1 - index0 + 1;
        if (curveType == HPOINT_MASK_CURVETYPE_BSPLINE)
            {
            GraphicsPoint gp;
            if (pSource->GetGraphicsPoint ((size_t)index0, gp))
                return gp.GetOrder ();
            }
        return 0;
        }
    else
        return 0;
    }

static bool    gpa_improveOneGPAGPARoot
(
double                          *pParam0,
double                          *pParam1,
GraphicsPointArrayCP pSource0,
int                             index0,
CurveCurveFunc                  pFunc0,
GraphicsPointArrayCP pSource1,
int                             index1,
CurveCurveFunc                  pFunc1,
void                            *pVoid,
int                             workdim
)
    {
    double u = *pParam0;
    double v = *pParam1;
    double f0, f1;
    double f0u, f0v, f1u, f1v;
    DPoint3d point0, point1;
    DPoint3d tangent0, tangent1;
    DPoint3d curvature0, curvature1;
    double du, dv;
    int counter = 0;
    static double maxdelta = 0.1;
    static double s_uvTol = 1.0e-14;

    for (counter = 0; counter < 8; counter++)
        {
        jmdlGraphicsPointArray_primitiveFractionArrayToDPoint3dDerivatives
                                (
                                pSource0,
                                &point0, &tangent0, &curvature0,
                                index0, &u, 1
                                );

        jmdlGraphicsPointArray_primitiveFractionArrayToDPoint3dDerivatives
                                (
                                pSource1,
                                &point1, &tangent1, &curvature1,
                                index1, &v, 1
                                );

        pFunc0 (&f0, &f0u, &f0v,
                &point0, &tangent0, &curvature0,
                &point1, &tangent1, &curvature1,
                pVoid, workdim);

        pFunc1 (&f1, &f1v, &f1u,
                &point1, &tangent1, &curvature1,
                &point0, &tangent0, &curvature0,
                pVoid, workdim);

        if (bsiSVD_solve2x2 (&du, &dv,f0u, f0v, f1u, f1v, f0, f1))
            {
#define NOISYnot
#ifdef NOISY
            printf(" Newton Step u:(%le-%le)   v:(%le-%le)\n",
                            u, du, v, dv);
#endif
            if (fabs (du) > maxdelta)
                du = (du > 0.0 ? 1.0 : -1.0) * maxdelta;
            if (fabs (dv) > maxdelta)
                du = (dv > 0.0 ? 1.0 : -1.0) * maxdelta;
            u -= du;
            v -= dv;
            if (fabs (du) < s_uvTol && fabs (dv) < s_uvTol)
                {
                *pParam0 = u;
                *pParam1 = v;
                return true;
                }
            }
        }
    return false;
    }

static bool    gpa_improveGPAGPARoots
(
      GraphicsPointArrayP pDest,
GraphicsPointArrayCP pSource,
GraphicsPointArrayCP pSource0,
int                             index0,
CurveCurveFunc                  pFunc0,
GraphicsPointArrayCP pSource1,
int                             index1,
CurveCurveFunc                  pFunc1,
void                            *pVoid,
int                             workdim
)
    {
    int i;
    GraphicsPoint gp;
    static bool    s_forceOutput = false;
    for (i = 0; jmdlGraphicsPointArray_getGraphicsPoint (pSource, &gp, i); i++)
        {
        if (s_forceOutput || gpa_improveOneGPAGPARoot
                    (
                    &gp.point.x, &gp.point.y,
                    pSource0, index0, pFunc0,
                    pSource1, index1, pFunc1, pVoid,
                    workdim
                    ))
            jmdlGraphicsPointArray_addGraphicsPoint (pDest, &gp);
        }
    return true;
    }

static void gpa_approximateGPAGPARoots
(
GraphicsPointArrayP pGPA,
GraphicsPointArrayCP pSource0,
      int                       index0,
GraphicsPointArrayCP pSource1,
      int                       index1,
    CurveCurveFunc              pFunc0,
    CurveCurveFunc              pFunc1,
    void                        *pVoid,
    int                         workdim
)
    {
#define SUBDIVISION_FACTOR 4
#define MAX_POINT_PER_CURVE (SUBDIVISION_FACTOR * MAX_BEZIER_CURVE_ORDER)
#define MAX_SURFACE_POINT (100 * 100)
    static int s_minPerCurve = 20;
    /*double param0[MAX_POINT_PER_CURVE];
    double param1[MAX_POINT_PER_CURVE];
    DPoint3d tangent0[MAX_POINT_PER_CURVE];
    DPoint3d tangent1[MAX_POINT_PER_CURVE];
    DPoint3d curvature0[MAX_POINT_PER_CURVE];
    DPoint3d curvature1[MAX_POINT_PER_CURVE];
    DPoint3d point0[MAX_POINT_PER_CURVE];
    DPoint3d point1[MAX_POINT_PER_CURVE];
    int num0 = SUBDIVISION_FACTOR * jmdlGraphicsPointArray_primitiveOrder (pSource0, index0);
    int num1 = SUBDIVISION_FACTOR * jmdlGraphicsPointArray_primitiveOrder (pSource1, index1);
    double   a01[MAX_POINT_PER_CURVE * MAX_POINT_PER_CURVE];
    double   a10[MAX_POINT_PER_CURVE * MAX_POINT_PER_CURVE];*/
    
    double *param0 = (double*)BSIBaseGeom::Malloc (MAX_POINT_PER_CURVE * sizeof (double));
    double *param1 = (double*)BSIBaseGeom::Malloc (MAX_POINT_PER_CURVE * sizeof (double));
    DPoint3d *tangent0 = (DPoint3d*)BSIBaseGeom::Malloc (MAX_POINT_PER_CURVE * sizeof (DPoint3d));
    DPoint3d *tangent1 = (DPoint3d*)BSIBaseGeom::Malloc (MAX_POINT_PER_CURVE * sizeof (DPoint3d));
    DPoint3d *curvature0 = (DPoint3d*)BSIBaseGeom::Malloc (MAX_POINT_PER_CURVE * sizeof (DPoint3d));
    DPoint3d *curvature1 = (DPoint3d*)BSIBaseGeom::Malloc (MAX_POINT_PER_CURVE * sizeof (DPoint3d));
    DPoint3d *point0 = (DPoint3d*)BSIBaseGeom::Malloc (MAX_POINT_PER_CURVE * sizeof (DPoint3d));
    DPoint3d *point1 = (DPoint3d*)BSIBaseGeom::Malloc (MAX_POINT_PER_CURVE * sizeof (DPoint3d));
    int num0 = SUBDIVISION_FACTOR * jmdlGraphicsPointArray_primitiveOrder (pSource0, index0);
    int num1 = SUBDIVISION_FACTOR * jmdlGraphicsPointArray_primitiveOrder (pSource1, index1);
    double *a01 = (double*)BSIBaseGeom::Malloc (MAX_POINT_PER_CURVE*MAX_POINT_PER_CURVE * sizeof (double));
    double *a10 = (double*)BSIBaseGeom::Malloc (MAX_POINT_PER_CURVE*MAX_POINT_PER_CURVE * sizeof (double));

    int i, j;

    if (num0 < s_minPerCurve)
        num0 = s_minPerCurve;
    if (num1 < s_minPerCurve)
        num1 = s_minPerCurve;

    if (num0 > MAX_POINT_PER_CURVE
        || num1 > MAX_POINT_PER_CURVE
        || num0 * num1 > MAX_SURFACE_POINT)
        {
        return;
        }
    bsiDoubleArray_uniformGrid (param0, num0, 0.0, 1.0);
    bsiDoubleArray_uniformGrid (param1, num1, 0.0, 1.0);
    jmdlGraphicsPointArray_primitiveFractionArrayToDPoint3dDerivatives
                (pSource0, point0, tangent0, curvature0, index0, param0, num0);
    jmdlGraphicsPointArray_primitiveFractionArrayToDPoint3dDerivatives
                (pSource1, point1, tangent1, curvature1, index1, param1, num1);

    for (i = 0; i < num0; i++)
        for (j = 0; j < num1; j++)
            {
            pFunc0 (
                                    &a01[i * num1 + j],
                                    NULL,
                                    NULL,
                                    point0 + i, tangent0 + i, curvature0 + i,
                                    point1 + j, tangent1 + j, curvature1 + i, pVoid,
                                    workdim
                                    );
            pFunc1 (
                                    &a10[i * num1 + j],
                                    NULL,
                                    NULL,
                                    point1 + j, tangent1 + j, curvature1 + i,
                                    point0 + i, tangent0 + i, curvature0 + i, pVoid,
                                    workdim
                                    );
            }

    jmdlGraphicsPointArray_addBilinearCommonRoots (pGPA,
                        a01,        0.0,        a10,    0.0,
                        num0, num1, num1, 1);

    BSIBaseGeom::Free (a10);
    BSIBaseGeom::Free (a01);
    BSIBaseGeom::Free (point1);
    BSIBaseGeom::Free (point0);
    BSIBaseGeom::Free (curvature1);
    BSIBaseGeom::Free (curvature0);
    BSIBaseGeom::Free (tangent1);
    BSIBaseGeom::Free (tangent0);
    BSIBaseGeom::Free (param1);
    BSIBaseGeom::Free (param0);
    }

typedef struct
     {
    GraphicsPointArrayP pCircleDest;
    GraphicsPointArrayP pTangentDest[2];
    DPoint3d point2;
    } PointCurveCurveContext;

struct  CurveCurveCurveContext
    {
    // Primary data ...
    GraphicsPointArrayP pCircleDest;
    GraphicsPointArrayCP pSourceGPA[3];
    GraphicsPointArrayP pTangentDest[3];
    int                     numSamples;
    
    // Per solution support data.
    // Lines are stored in ellipse with center = start, vector0 = direction vector, vector90 = 000.
    int numEllipse;
    DEllipse3d ellipse[3];
    DEllipse3d localEllipse[3];
    double     localRadius[3];
    int        ellipseSource[3];
    int        ellipseIndex[3];
    int        ellipseDim[3];  // 1 for line (center+s*vector0), 2 for ellipse.
    // Maintain indices (0..2) to the geometries that are ellipses and those that are lines.
    int        selectNumEllipse;
    int        selectEllipse[3];
    int        selectNumLine;
    int        selectLine[3];

CurveCurveCurveContext
(
GraphicsPointArrayCP _pSourceGPA0,
GraphicsPointArrayCP _pSourceGPA1,
GraphicsPointArrayCP _pSourceGPA2,
int _numSamples,
GraphicsPointArrayP _pTangentPointCollector0,
GraphicsPointArrayP _pTangentPointCollector1,
GraphicsPointArrayP _pTangentPointCollector2,
GraphicsPointArrayP _pFullCircleCollector
)
    {
    pSourceGPA[0]           = _pSourceGPA0;
    pSourceGPA[1]           = _pSourceGPA1;
    pSourceGPA[2]           = _pSourceGPA2;
    pTangentDest[0] = _pTangentPointCollector0;
    pTangentDest[1] = _pTangentPointCollector1;
    pTangentDest[2] = _pTangentPointCollector2;
    pCircleDest     = _pFullCircleCollector;
    numSamples      = _numSamples;
    }


    Transform worldToLocal, localToWorld;

    struct Solutions
        {
            int numOut;
            DPoint3d rCenter[8];
            double   rRadius[8];
            DPoint3d rTangentA[8];
            DPoint3d rTangentB[8];
            DPoint3d rTangentC[8];
            int iA, iB, iC;
        Solutions (int _iA, int _iB, int _iC) :
            iA(_iA), iB(_iB), iC(_iC)
            {
            }
        };

    void CollectLocalSolutions (Solutions ss)
        {
        DPoint3d worldTangent[3], localTangent[3], worldCenter;
        int gpaSelect[3];
        gpaSelect[0] = ss.iA;
        gpaSelect[1] = ss.iB;
        gpaSelect[2] = ss.iC;
        for (int i = 0; i < ss.numOut; i++)
            {
            localTangent[0] = ss.rTangentA[i];
            localTangent[1] = ss.rTangentB[i];
            localTangent[2] = ss.rTangentC[i];
            double tangentFraction[3];
            int numInterior = 0;
            for (int j = 0; j < 3; j++)
                {
                int k = gpaSelect[j];
                worldTangent[j] = localTangent[j];
                // umm... The localTangents myseriously are in world already. Dunno.
                //localToWorld.Multiply(worldTangent[j], worldTangent[j]);
                tangentFraction[j] = 0.0;
                if (ellipseDim[k] == 1)
                    {
                    DRay3d ray;
                    DPoint3d closePoint;
                    bsiDRay3d_initFromDPoint3dTangent (&ray, &localEllipse[k].center, &localEllipse[k].vector0);
                    bsiDRay3d_projectPoint (&ray, &closePoint, &tangentFraction[j], &localTangent[j]);
                    if (ValidFraction (tangentFraction[j]))
                        numInterior++;
                    }
                else if (ellipseDim[k] == 2)
                    {
                    DVec3d localVector;
                    localVector.DifferenceOf(localTangent[j], *(&localEllipse[k].center));
                    double theta = atan2 (localVector.DotProduct (*(&localEllipse[k].vector90)),
                                          localVector.DotProduct (*(&localEllipse[k].vector0)));
                    tangentFraction[j] = localEllipse[k].AngleToFraction (theta);
                    if (ValidFraction (tangentFraction[j]))
                        numInterior++;
                    }
                }

            localToWorld.Multiply (worldCenter, ss.rCenter[i]);
            DEllipse3d ellipse;
            if (numInterior < 3)
                {
                }
            else
                {
                bsiDEllipse3d_initFromCenterNormalRadius (&ellipse, &ss.rCenter[i], NULL, ss.rRadius[i]);
                bsiDEllipse3d_makeFullSweep (&ellipse);
                if (pCircleDest)
                    jmdlGraphicsPointArray_addDEllipse3d (pCircleDest, &ellipse);
                for (int j = 0; j < 3; j++)
                    {
                    int k = gpaSelect[j];
                    if (pTangentDest[k])
                        {
                        //double fraction = 0.0;  // NEEDSWORK
                        jmdlGraphicsPointArray_addComplete
                                (
                                pTangentDest[k],
                                worldTangent[j].x, worldTangent[j].y, worldTangent[j].z, 1.0,
                                tangentFraction[j], 0, ellipseIndex[k]);
                        }
                    }
                }
            }
        }

    bool ValidFraction (double f)
        {
        double tol = bsiTrig_smallAngle ();
        return f >= -tol && f < 1.0 + tol;
        }
    void ClearSpecials ()
        {
        numEllipse = 0;
        selectNumEllipse = 0;
        selectNumLine = 0;
        }

    bool AddEllipse (DEllipse3dCR newEllipse, int index, int sourceIndex, int dim)
        {
        if (numEllipse < 3)
            {
            ellipse[numEllipse] = newEllipse;
            ellipseIndex[numEllipse] = index;
            ellipseSource[numEllipse] = sourceIndex;
            ellipseDim[numEllipse] = dim;
            if (dim == 1)
                selectLine[selectNumLine++] = numEllipse;
            if (dim == 2)
                selectEllipse[selectNumEllipse++] = numEllipse;
            numEllipse++;
            return true;
            }
        return false;
        }

    bool LoadSpecials (int sourceIndex, GraphicsPointArrayCP gpa, int index)
        {
        DEllipse3d myEllipse;
        DSegment4d mySegment4d;
        DSegment3d mySegment;
        int readIndex = index;
        bool isEllipse;
        if (jmdlGraphicsPointArray_getDEllipse3d (gpa, &readIndex, NULL, &myEllipse, &isEllipse, NULL, NULL))
            {
            if (isEllipse && numEllipse < 3)
                return AddEllipse (myEllipse, index, sourceIndex, 2);
            }
        else if (jmdlGraphicsPointArray_getDSegment4d (gpa, &readIndex, &mySegment4d)
                && bsiDSegment4d_getDSegment3d (&mySegment4d, &mySegment))
            {
            myEllipse.InitFromPoints (mySegment.point[0], mySegment.point[1], mySegment.point[0], 0.0, msGeomConst_2pi);
            return AddEllipse (myEllipse, index, sourceIndex, 1);
            }
        return false;
        }

    bool SetupLocalCoordinates ()
        {
        // Any ellipse (by itself) generates a candidate normal.
        // Any pair of lines generates a candidate;
        double a, aRef = 0.0;
        DVec3d normal, refNormal;
        DPoint3d refPoint;
        for (int i = 0; i < numEllipse; i++)
            {
            if (ellipseDim[i] == 2)
                {
                a = normal.NormalizedCrossProduct (*(&ellipse[i].vector0), *(&ellipse[i].vector90));
                if (a > aRef)
                    {
                    refNormal = normal;
                    refPoint  = ellipse[i].center;
                    aRef = a;
                    }
                }
            else if (ellipseDim[i] == 1)
                {
                for (int j = i + 1; j < 3; j++)
                    {
                    if (ellipseDim[j] == 1)
                        {
                        a = normal.NormalizedCrossProduct (*(&ellipse[i].vector0), *(&ellipse[j].vector0));
                        if (a > aRef)
                            {
                            refNormal = normal;
                            refPoint  = ellipse[i].center;
                            aRef = a;
                            }
                        }
                    }
                }
            }
        if (aRef > 0.0)
            {
            RotMatrix matrix;
            bsiRotMatrix_initFrom1Vector(&matrix, &refNormal, 2, true);
            bsiTransform_initFromMatrixAndTranslation (&localToWorld, &matrix, &refPoint);
            worldToLocal.InverseOf (worldToLocal);

            for (int i = 0; i < numEllipse; i++)
                {
                worldToLocal.Multiply  (localEllipse[i], ellipse[i]);
                }
            return true;
            }
        return false;
        }

    bool SetupLocalGeometry ()
        {
        double maxCoordinate = 0.0;
        for (int i = 0; i < numEllipse; i++)
            {
            double a = localEllipse[i].center.MaxAbs ();
            double b = localEllipse[i].vector0.MaxAbs ();
            if (a > maxCoordinate)
                maxCoordinate = a;
            if (b > maxCoordinate)
                maxCoordinate = b;
            }
        double absTol = bsiTrig_smallAngle () * maxCoordinate;
        for (int i = 0; i < numEllipse; i++)
            {
            if (fabs (localEllipse[i].center.z) > absTol)
                return false;
            if (fabs (localEllipse[i].vector0.z) > absTol)
                return false;
            if (fabs (localEllipse[i].vector90.z) > absTol)
                return false;
            }

        for (int i = 0; i < numEllipse; i++)
            {
            localRadius[i] = 0.0;
            if (ellipseDim[i] == 2)
                {
                if (!localEllipse[i].IsCircular ())
                     return false;
                localRadius[i] = localEllipse[i].vector0.Magnitude ();               
                }
            }
        return true;
        }


    bool SolveCCC ()
        {
        if (selectNumEllipse == 3)
            {
            Solutions ss(selectEllipse[0], selectEllipse[1], selectEllipse[2]);
            bsiGeom_circleTTTCircleCircleCircleConstruction
                (ss.rCenter, ss.rRadius, ss.rTangentA, ss.rTangentB, ss.rTangentC, ss.numOut, 8,
                        localEllipse[ss.iA].center, localRadius[ss.iA],
                        localEllipse[ss.iB].center, localRadius[ss.iB],
                        localEllipse[ss.iC].center, localRadius[ss.iC]);
            CollectLocalSolutions (ss);
            return true;
            }
        return false;
        }


    bool SolveLLC ()
        {
        if (selectNumEllipse == 1 && selectNumLine == 2)
            {
            Solutions ss(selectLine[0], selectLine[1], selectEllipse[0]);
            bsiGeom_circleTTTLineLineCircleConstruction
                (ss.rCenter, ss.rRadius, ss.rTangentA, ss.rTangentB, ss.rTangentC, ss.numOut, 8,
                        localEllipse[ss.iA].center, localEllipse[ss.iA].vector0,
                        localEllipse[ss.iB].center, localEllipse[ss.iB].vector0,
                        localEllipse[ss.iC].center, localRadius[ss.iC]);
            CollectLocalSolutions (ss);
            return true;
            }
        return false;
        }

    bool SolveLCC ()
        {
        if (selectNumEllipse == 2 && selectNumLine == 1)
            {
            Solutions ss(selectEllipse[0], selectEllipse[1], selectLine[0]);
            bsiGeom_circleTTTCircleCircleLineConstructionExt
                (ss.rCenter, ss.rRadius, ss.rTangentA, ss.rTangentB, ss.rTangentC, ss.numOut, 8,
                        localEllipse[ss.iA].center, localRadius[ss.iA],
                        localEllipse[ss.iB].center, localRadius[ss.iB],
                        localEllipse[ss.iC].center, localEllipse[ss.iC].vector0);
            CollectLocalSolutions (ss);
            return true;
            }
        return false;
        }

    bool SolveLLL ()
        {
        if (selectNumLine == 3)
            {
            Solutions ss(selectLine[0], selectLine[1], selectLine[2]);
            bsiGeom_circleTTTLineLineLineConstruction
                (ss.rCenter, ss.rRadius, ss.rTangentA, ss.rTangentB, ss.rTangentC, ss.numOut, 8,
                        localEllipse[ss.iA].center, localEllipse[ss.iA].vector0,
                        localEllipse[ss.iB].center, localEllipse[ss.iB].vector0,
                        localEllipse[ss.iC].center, localEllipse[ss.iC].vector0);
            CollectLocalSolutions (ss);
            return true;
            }
        return false;
        }

    };


static bool    gpa_GPAGPACurveCurvePointSpecialCase
(
      PointCurveCurveContext    *pContext,
      GraphicsPointArrayP pConvergedRoots,
GraphicsPointArrayCP pSource0,
      int                       index0,
GraphicsPointArrayCP pSource1,
      int                       index1
)
    {
    int readIndex;
    DSegment4d segment0;
    DSegment4d segment1;
    DPoint3d tangent0, tangent1, perp0;
    DPoint3d intersectionPoint, vectorToCenter;
    DPoint3d vectorToPoint;
    DPoint4d projection0, projection1, center4d, intersection4d;
    double   param0, param1;
    DPoint3d center;
    int numSolution;
    double tt[2];
    int i;

    double s0, s1;
    double aa, bb, cc, drdt;
    if (    (readIndex = index0, jmdlGraphicsPointArray_getDSegment4d (pSource0, &readIndex, &segment0))
        &&  (readIndex = index1, jmdlGraphicsPointArray_getDSegment4d (pSource1, &readIndex, &segment1))
        &&  bsiDSegment4d_intersectXYDSegment4dDSegment4d (&intersection4d, NULL, NULL, NULL,
                                    &segment0, &segment1)
        )
        {
        bsiDPoint4d_normalize (&intersection4d, &intersectionPoint);
        bsiDSegment4d_pseudoTangent (&segment0, &tangent0);
        bsiDSegment4d_pseudoTangent (&segment1, &tangent1);
        if (bsiSVD_solve2x2 (&s0, &s1,
                                tangent0.x, tangent1.x,
                                tangent0.y, tangent1.y,
                                pContext->point2.x - intersectionPoint.x,
                                pContext->point2.y - intersectionPoint.y))
            {
            if (s0 < 0.0)
                {
                bsiDPoint3d_negateInPlace (&tangent0);
                s0 = -s0;
                }

            if (s1 < 0.0)
                {
                bsiDPoint3d_negateInPlace (&tangent1);
                s1 = -s1;
                }

            bsiDPoint3d_normalizeInPlace (&tangent0);
            bsiDPoint3d_normalizeInPlace (&tangent1);
            bsiDPoint3d_interpolate (&vectorToCenter, &tangent0, 0.5, &tangent1);
            bsiDPoint3d_subtractDPoint3dDPoint3d (&vectorToPoint, &intersectionPoint, &pContext->point2);
            vectorToCenter.z = vectorToPoint.z = 0.0;
            bsiDPoint3d_normalizeInPlace (&vectorToCenter);

            bsiDPoint3d_unitPerpendicularXY (&perp0, &tangent0);
            /* X = intersectionPoint + t * centerVector */
            /*  distance from (both) lines is proportional to t, linearly, so squared distance
                from each line is t^2*drdt^2) */
            drdt = bsiDPoint3d_dotProductXY (&perp0, &vectorToCenter);
            /* And distance from point is also quadratic .. */
            aa = bsiDPoint3d_dotProduct (&vectorToCenter, &vectorToCenter);
            bb = 2.0 * bsiDPoint3d_dotProduct (&vectorToCenter, &vectorToPoint);
            cc = bsiDPoint3d_dotProductXY (&vectorToPoint, &vectorToPoint);
            numSolution = bsiMath_solveQuadratic (tt, aa - drdt * drdt, bb, cc);
            for (i = 0; i < numSolution; i++)
                {
                bsiDPoint3d_addScaledDPoint3d (&center, &intersectionPoint, &vectorToCenter, tt[i]);
                bsiDPoint4d_initFromDPoint3dAndWeight (&center4d, &center, 1.0);
                bsiDSegment4d_projectDPoint4dCartesianXYW (&segment0, &projection0, &param0, &center4d);
                bsiDSegment4d_projectDPoint4dCartesianXYW (&segment1, &projection1, &param1, &center4d);
                jmdlGraphicsPointArray_addComplete
                        (
                        pConvergedRoots,
                        param0, param1, 0.0, 0.0, 0.0, 0, 0);
                }
            return true;
            }
        }
    return false;
    }

static bool    gpa_cbapproximateGPAGPACurveCurvePointRoots
(
      PointCurveCurveContext    *pContext,
GraphicsPointArrayCP pSource0,
      int                       index0,
GraphicsPointArrayCP pSource1,
      int                       index1
)
    {
    GraphicsPointArrayP pApproximateRoots = jmdlGraphicsPointArray_grab ();
    GraphicsPointArrayP pConvergedRoots = jmdlGraphicsPointArray_grab ();
    GraphicsPoint gp;
    DPoint3d point0, point1;
    DPoint4d tangentPoint;
    DEllipse3d ellipse;
    double f0, f1;
    int i;

    if (gpa_GPAGPACurveCurvePointSpecialCase
                (pContext, pConvergedRoots, pSource0, index0, pSource1, index1))
        {
        }
    else
        {
        gpa_approximateGPAGPARoots (pApproximateRoots,
                pSource0, index0, pSource1, index1,
                (CurveCurveFunc)evaluateCurveCurvePointFunction,
                (CurveCurveFunc)evaluateCurveCurvePointFunction,
                &pContext->point2, 2
                );

        gpa_improveGPAGPARoots (pConvergedRoots, pApproximateRoots,
                pSource0, index0, (CurveCurveFunc)evaluateCurveCurvePointFunction,
                pSource1, index1, (CurveCurveFunc)evaluateCurveCurvePointFunction,
                &pContext->point2, 2
                );
    }

    for (i = 0; jmdlGraphicsPointArray_getGraphicsPoint (pConvergedRoots, &gp, i); i++)
        {
        f0 = gp.point.x;
        f1 = gp.point.y;
        jmdlGraphicsPointArray_primitiveFractionToDPoint3d (pSource0, &point0, index0, f0);
        jmdlGraphicsPointArray_primitiveFractionToDPoint3d (pSource1, &point1, index1, f1);
        if (bsiDEllipse3d_initFrom3DPoint3dOnArc
                (
                &ellipse,
                &point0,
                &point1,
                &pContext->point2
                ))
            {
            bsiDEllipse3d_makeFullSweep (&ellipse);
            if (pContext->pCircleDest)
                jmdlGraphicsPointArray_addDEllipse3d (pContext->pCircleDest, &ellipse);

            if (pContext->pTangentDest[0])
                {
                jmdlGraphicsPointArray_primitiveFractionToDPoint4d (pSource0, &tangentPoint, index0, f0);
                jmdlGraphicsPointArray_addComplete
                        (
                        pContext->pTangentDest[0],
                        tangentPoint.x, tangentPoint.y, tangentPoint.z, tangentPoint.w,
                        f0, 0, index0);
                }

            if (pContext->pTangentDest[1])
                {
                jmdlGraphicsPointArray_primitiveFractionToDPoint4d (pSource1, &tangentPoint, index1, f1);
                jmdlGraphicsPointArray_addComplete
                        (
                        pContext->pTangentDest[1],
                        tangentPoint.x, tangentPoint.y, tangentPoint.z, tangentPoint.w,
                        f1, 0, index1
                        );
                }
            }
        }

    jmdlGraphicsPointArray_drop (pConvergedRoots);
    jmdlGraphicsPointArray_drop (pApproximateRoots);
    return true;
    }

typedef void (*CurveCurveCurveFunc)
(
      double    *pf,
      double    *pd0,
      double    *pd1,
      double    *pd2,
const DPoint3d  *pX0,
const DPoint3d  *pT0,
const DPoint3d  *pK0,
const DPoint3d  *pX1,
const DPoint3d  *pT1,
const DPoint3d  *pK1,
const DPoint3d  *pX2,
const DPoint3d  *pT2,
const DPoint3d  *pK2
);

static void    evaluateCurveCurveCurve
(
      double    *pf,
      double    *pd0,
      double    *pd1,
      double    *pd2,
const DPoint3d  *pX0,
const DPoint3d  *pT0,
const DPoint3d  *pK0,
const DPoint3d  *pX1,
const DPoint3d  *pT1,
const DPoint3d  *pK1,
const DPoint3d  *pX2,
const DPoint3d  *pT2,
const DPoint3d  *pK2
)
    {
#define LOCAL_ANGLES_FUNCTIONnot
#ifdef LOCAL_ANGLES_FUNCTION
    /* This function forces (tangents of) angles at left and right
        of each chord to match.  The function itself is wonderfully
        simple.
        Problems:
            1) Something is wrong with a derivative.  NR converges
                using approximate derivative matrix.  Exact, approximate
                have severe differences in zz term (for one observation ---
                maybe others elsewhere.)
            2) equal angle condition is also satisfied for circle PERPENDICULAR
                to all three curves.  (sometimes?)  Neat effect, no use I know of.
    */
    DPoint3d U;
    double UcrossT0, UdotT1, UcrossT1, UdotT0, T0dotT0, T1dotT1;
    double T0dotT1, T0crossT1, T1crossT0;
    double UcrossK0, UcrossK1;
    double UdotK0, UdotK1;

    bsiDPoint3d_subtractDPoint3dDPoint3d (&U, pX1, pX0);

    UcrossT0  = bsiDPoint3d_crossProductXY (&U, pT0);
    UcrossT1  = bsiDPoint3d_crossProductXY (&U, pT1);

    UdotT0    = bsiDPoint3d_dotProductXY (&U, pT0);
    UdotT1    = bsiDPoint3d_dotProductXY (&U, pT1);

    T0dotT1   = bsiDPoint3d_dotProductXY (pT0, pT1);
    T0dotT0   = bsiDPoint3d_dotProductXY (pT0, pT0);
    T1dotT1   = bsiDPoint3d_dotProductXY (pT1, pT1);
    T0crossT1 = bsiDPoint3d_crossProductXY (pT0, pT1);
    T1crossT0 = -T0crossT1;

    UdotK0    = bsiDPoint3d_dotProductXY (&U, pK0);
    UdotK1    = bsiDPoint3d_dotProductXY (&U, pK1);

    UcrossK0  = bsiDPoint3d_crossProductXY (&U, pK0);
    UcrossK1  = bsiDPoint3d_crossProductXY (&U, pK1);

    if (pf)
        *pf  = UcrossT0 * UdotT1 + UcrossT1 * UdotT0;
    if (pd0)
        *pd0 = UcrossK0 * UdotT1
             - UcrossT0 * T0dotT1
             - T0crossT1 * UdotT0
             + UcrossT1 * (-T0dotT0 + UdotK0);

    if (pd1)
        *pd1 = T1crossT0 * UdotT1
             + UcrossT0  * (T1dotT1 + UdotK1)
             + UcrossK1 * UdotT0
             + UcrossT1 * T0dotT1;
    if (pd2)
        {
        *pd2 = 0.0;
        }
#else
        DPoint3d C, dCds0, dCds1, dCds2;
        DPoint3d R;
        DPoint3d Q;
        double w;
        circumcenterTerms (&C, &dCds0, &dCds1, &dCds2, pX0, pT0, pX1, pT1, pX2, pT2);
        w = C.z;
        R.x = C.x - w * pX0->x;
        R.y = C.y - w * pX0->y;
        if (pf)
            *pf  = bsiDPoint3d_dotProductXY (pT0, &R);

        if (pd0)
            {
            bsiDPoint3d_add2ScaledDPoint3d (&Q,
                            &dCds0,
                            pT0, -w,
                            pX0, -dCds0.z);
            *pd0 = bsiDPoint3d_dotProductXY (pK0, &R)
                 + bsiDPoint3d_dotProductXY (pT0, &Q);
            }

        if (pd1)
            {
            bsiDPoint3d_addScaledDPoint3d (&Q,
                            &dCds1,
                            pX0, -dCds1.z);
            *pd1 = bsiDPoint3d_dotProductXY (pT0, &Q);
            }


        if (pd2)
            {
            bsiDPoint3d_addScaledDPoint3d (&Q,
                            &dCds2,
                            pX0, -dCds2.z);
            *pd2 = bsiDPoint3d_dotProductXY (pT0, &Q);
            }
#endif
    }

static void    gpa_ccc_evaluate
(
        DPoint3d    *pF,
        RotMatrix   *pJ,
const   DPoint3d    *pX0,
const   DPoint3d    *pT0,
const   DPoint3d    *pK0,
CurveCurveCurveFunc func012,
const   DPoint3d    *pX1,
const   DPoint3d    *pT1,
const   DPoint3d    *pK1,
CurveCurveCurveFunc func120,
const   DPoint3d    *pX2,
const   DPoint3d    *pT2,
const   DPoint3d    *pK2,
CurveCurveCurveFunc func201
)
    {
    func012 (
        &pF->x, &pJ->form3d[0][0], &pJ->form3d[0][1], &pJ->form3d[0][2],
        pX0, pT0, pK0,
        pX1, pT1, pK1,
        pX2, pT2, pT2
        );

    func120 (
        &pF->y, &pJ->form3d[1][1], &pJ->form3d[1][2], &pJ->form3d[1][0],
        pX1, pT1, pK1,
        pX2, pT2, pT2,
        pX0, pT0, pK0
        );

    func201 (
        &pF->z, &pJ->form3d[2][2], &pJ->form3d[2][0], &pJ->form3d[2][1],
        pX2, pT2, pK2,
        pX0, pT0, pK0,
        pX1, pT1, pK1
        );
    }

/*---------------------------------------------------------------------------------**//**
* Generate an approximate root from corner values of a trivariate function.
* z varies fastest in arrays of 8 functions and jacobians.
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    gpa_approximateTrivariateRoot
(
      GraphicsPointArrayP pApproximateRoots,
const DPoint3d                  *pParamArray,
const DPoint3d                  *pFArray,
const RotMatrix                 *pJArray
)
    {
    int numPos[3];
    int numNeg[3];
    double fMin, fCurr;
    int k, kMin;

    fMin = 1.0e50;
    kMin = -1;
    memset (numPos, 0, 3 * sizeof (int));
    memset (numNeg, 0, 3 * sizeof (int));

    for (k = 0; k < 8; k++)
        {
        fCurr = bsiDPoint3d_magnitudeSquared (&pFArray[k]);
        if (k == 0 || fCurr < fMin)
            {
            fMin = fCurr;
            kMin = k;
            }
        if (pFArray[k].x >= 0)
            numPos[0] += 1;
        else
            numNeg[0]+= 1;

        if (pFArray[k].y >= 0)
            numPos[1] += 1;
        else
            numNeg[1]+= 1;

        if (pFArray[k].z >= 0)
            numPos[2] += 1;
        else
            numNeg[2]+= 1;
        }

    if (fMin > 0.0)
        {
        for (k = 0; k < 3; k++)
            {
            if (numPos[k] == 0 || numNeg[k] == 0)
                return;
            }
        }

    jmdlGraphicsPointArray_addComplete (pApproximateRoots,
                    pParamArray[kMin].x, pParamArray[kMin].y, pParamArray[kMin].z, 1.0,
                    0.0, 0, 0);
    }
static void    gpa_approximateGPAGPAGPARoots
(
      GraphicsPointArrayP pApproximateRoots,
GraphicsPointArrayCP pSource0,
      int                       index0,
GraphicsPointArrayCP pSource1,
      int                       index1,
GraphicsPointArrayCP pSource2,
      int                       index2,
      CurveCurveCurveFunc       func012,
      CurveCurveCurveFunc       func120,
      CurveCurveCurveFunc       func210,
      int                       numSamples
)
    {
#define MAX_CCC_POINT 200
#define MIN_CCC_POINT 10
    DPoint3d X0[MAX_CCC_POINT];
    DPoint3d X1[MAX_CCC_POINT];
    DPoint3d X2[MAX_CCC_POINT];
    DPoint3d T0[MAX_CCC_POINT];
    DPoint3d T1[MAX_CCC_POINT];
    DPoint3d T2[MAX_CCC_POINT];
    DPoint3d C0[MAX_CCC_POINT];
    DPoint3d C1[MAX_CCC_POINT];
    DPoint3d C2[MAX_CCC_POINT];
    double param0[MAX_CCC_POINT];
    double param1[MAX_CCC_POINT];
    double param2[MAX_CCC_POINT];
    DPoint3d UVW[8];
    int num0, num1, num2;
    int i0, i1, i2;
    int k0, k1, k2;
    int m0, m1, m2;
    int k;
    DPoint3d FFF[8];
    RotMatrix JJ[8];

    if (numSamples > MAX_CCC_POINT)
        numSamples = MAX_CCC_POINT;
    if (numSamples < MIN_CCC_POINT)
        numSamples = MIN_CCC_POINT;
    num0 = numSamples;
    num1 = numSamples;
    num2 = numSamples;

    bsiDoubleArray_uniformGrid (param0, num0, 0.0, 1.0);
    bsiDoubleArray_uniformGrid (param1, num1, 0.0, 1.0);
    bsiDoubleArray_uniformGrid (param2, num2, 0.0, 1.0);

    jmdlGraphicsPointArray_primitiveFractionArrayToDPoint3dDerivatives
                (pSource0, X0, T0, C0, index0, param0, num0);
    jmdlGraphicsPointArray_primitiveFractionArrayToDPoint3dDerivatives
                (pSource1, X1, T1, C1, index1, param1, num1);
    jmdlGraphicsPointArray_primitiveFractionArrayToDPoint3dDerivatives
                (pSource2, X2, T2, C2, index2, param2, num2);

    for (i0 = 1; i0 < num0; i0++)
        {
        for (i1 = 1; i1 < num1; i1++)
            {
            for (i2 = 1; i2 < num2; i2++)
                {
                k = 0;
                for (k0 = 0; k0 < 2; k0++)
                    for (k1 = 0; k1 < 2; k1++)
                        for (k2 = 0; k2 < 2; k2++)
                            {
                            m0 = i0 - 1 + k0;
                            m1 = i1 - 1 + k1;
                            m2 = i2 - 1 + k2;
                            UVW[k].x = param0[m0];
                            UVW[k].y = param1[m1];
                            UVW[k].z = param2[m2];
                            gpa_ccc_evaluate (&FFF[k], &JJ[k],
                                    &X0[m0], &T0[m0], &C0[m0], func012,
                                    &X1[m1], &T1[m1], &C1[m1], func120,
                                    &X2[m2], &T2[m2], &C2[m2], func210
                                    );
                            k++;
                            }
                gpa_approximateTrivariateRoot (pApproximateRoots,
                                    UVW, FFF, JJ);

                }
            }
        }
    }

static bool    gpa_improveOneTrivariateRoot
(
      DPoint3d                  *pUVW1,
const DPoint3d                  *pUVW0,
GraphicsPointArrayCP pSource0,
      int                       index0,
GraphicsPointArrayCP pSource1,
      int                       index1,
GraphicsPointArrayCP pSource2,
      int                       index2,
CurveCurveCurveFunc func012,
CurveCurveCurveFunc func120,
CurveCurveCurveFunc func210
)
    {
    DPoint3d F;
    DPoint3d dUVW;
    DPoint3d UVW;
    RotMatrix J;
    DPoint3d X0, T0, C0;
    DPoint3d X1, T1, C1;
    DPoint3d X2, T2, C2;
    int k;
    static int maxK = 14;
    static double s_absTol = 1.0e-12;
#define CHECK_DERIVATIVEnot
#ifdef CHECK_DERIVATIVE
    RotMatrix J1, J2;
    double dJ;
    int m;
    static double s_epsilon = 1.0e-4;
#endif

    UVW = *pUVW0;
    for (k = 0; k < maxK; k++)
        {
        jmdlGraphicsPointArray_primitiveFractionArrayToDPoint3dDerivatives
                    (pSource0, &X0, &T0, &C0, index0, &UVW.x, 1);
        jmdlGraphicsPointArray_primitiveFractionArrayToDPoint3dDerivatives
                    (pSource1, &X1, &T1, &C1, index1, &UVW.y, 1);
        jmdlGraphicsPointArray_primitiveFractionArrayToDPoint3dDerivatives
                    (pSource2, &X2, &T2, &C2, index2, &UVW.z, 1);


        gpa_ccc_evaluate (&F, &J,
                &X0, &T0, &C0, func012,
                &X1, &T1, &C1, func120,
                &X2, &T2, &C2, func210
                );
#ifdef CHECK_DERIVATIVE
        for (m = 0; m < 3; m++)
            {
            DPoint3d X = UVW, F1;
            ((double*)(&X.x))[m] += s_epsilon;
            jmdlGraphicsPointArray_primitiveFractionArrayToDPoint3dDerivatives
                        (pSource0, &X0, &T0, &C0, index0, &X.x, 1);
            jmdlGraphicsPointArray_primitiveFractionArrayToDPoint3dDerivatives
                        (pSource1, &X1, &T1, &C1, index1, &X.y, 1);
            jmdlGraphicsPointArray_primitiveFractionArrayToDPoint3dDerivatives
                        (pSource2, &X2, &T2, &C2, index2, &X.z, 1);
            gpa_ccc_evaluate (&F1, &J1,
                    &X0, &T0, &C0, func012,
                    &X1, &T1, &C1, func120,
                    &X2, &T2, &C2, func210
                    );
            bsiDPoint3d_subtractDPoint3dDPoint3d (&J2.column[m], &F1, &F);
            bsiDPoint3d_scale (&J2.column[m], &J2.column[m], 1.0 / s_epsilon);
            }
        dJ = bsiRotMatrix_maxDiff (&J, &J2);
        J = J2;
#endif
        if (!bsiRotMatrix_solveDPoint3d (&J, &dUVW, &F))
            return false;

        bsiDPoint3d_subtractDPoint3dDPoint3d (&UVW, &UVW, &dUVW);
        if (bsiDPoint3d_maxAbs (&dUVW) < s_absTol)
            {
            *pUVW1 = UVW;
            return true;
            }
        }
    return false;
    }

static void    gpa_improveTrivariateRoots
(
      GraphicsPointArrayP pConvergedRoots,
GraphicsPointArrayCP pApproximateRoots,
GraphicsPointArrayCP pSource0,
      int                       index0,
GraphicsPointArrayCP pSource1,
      int                       index1,
GraphicsPointArrayCP pSource2,
      int                       index2,
CurveCurveCurveFunc func012,
CurveCurveCurveFunc func120,
CurveCurveCurveFunc func210
)
    {
    GraphicsPoint gp;
    DPoint3d uvw0, uvw1;
    int i;
    for (i = 0; jmdlGraphicsPointArray_getGraphicsPoint (pApproximateRoots, &gp, i);i++)
        {
        uvw0.x = gp.point.x;
        uvw0.y = gp.point.y;
        uvw0.z = gp.point.z;
        if (gpa_improveOneTrivariateRoot (&uvw1, &uvw0,
                        pSource0, index0, pSource1, index1, pSource2, index2,
                        func012, func120, func210))
            {
            gp.point.x = uvw1.x;
            gp.point.y = uvw1.y;
            gp.point.z = uvw1.z;
            jmdlGraphicsPointArray_addGraphicsPoint (pConvergedRoots, &gp);
            }
        }
    }

static bool findParameterMatch (double *pDataA, GraphicsPointArrayP *pArrays, int numArray, double tol)
    {
    GraphicsPoint gp;    
    int arrayIndex;
    int pointIndex;
    int numPoint = jmdlGraphicsPointArray_getCount (pArrays[0]);
    if (numArray == 0)
        return false;
    for (pointIndex = 0; pointIndex < numPoint; pointIndex++)
        {
        double diff = 0.0;
        for (arrayIndex = 0; arrayIndex < numArray; arrayIndex++)
            {
            if (!jmdlGraphicsPointArray_getGraphicsPoint (pArrays[arrayIndex], &gp, pointIndex))
                return false;
            diff += fabs (pDataA[arrayIndex] - gp.a);
            }
        if (diff < tol)
            return true;
        }
    return false;
    }
static bool gpa_cbapproximateGPAGPACurveCurveCurveRoots_Analytic
(
      CurveCurveCurveContext    *pContext,
GraphicsPointArrayCP pSource0,
      int                       index0,
GraphicsPointArrayCP pSource1,
      int                       index1,
GraphicsPointArrayCP pSource2,
      int                       index2
)
    {
    pContext->ClearSpecials ();
    if (   pContext->LoadSpecials (0, pSource0, index0)
        && pContext->LoadSpecials (1, pSource1, index1)
        && pContext->LoadSpecials (2, pSource2, index2)
        && pContext->SetupLocalCoordinates ()
        && pContext->SetupLocalGeometry ()
        )
        {
        if (pContext->SolveLLC ())
            return true;
        if (pContext->SolveLCC ())
            return true;
        if (pContext->SolveCCC ())
            return true;
        if (pContext->SolveLLL())
            return true;
        return true;
        }
    return false;
    }
static bool gpa_cbapproximateGPAGPACurveCurveCurveRoots_BySearch
(
      CurveCurveCurveContext    *pContext,
GraphicsPointArrayCP pSource0,
      int                       index0,
GraphicsPointArrayCP pSource1,
      int                       index1,
GraphicsPointArrayCP pSource2,
      int                       index2
)
    {
    GraphicsPointArrayP pApproximateRoots = jmdlGraphicsPointArray_grab ();
    GraphicsPointArrayP pConvergedRoots = jmdlGraphicsPointArray_grab ();
    GraphicsPoint gp;
    DPoint3d point[3];
    DPoint4d tangentPoint;
    DEllipse3d ellipse;
    double f[3];
    static double s_duplicateParameterTol = 1.0e-8;
    int index[3];
    GraphicsPointArrayCP pSource[3];
    GraphicsPointArrayP pTangentDest[3];
    int i, k;

    pTangentDest[0] = pContext->pTangentDest[0];
    pTangentDest[1] = pContext->pTangentDest[1];
    pTangentDest[2] = pContext->pTangentDest[2];

    index[0] = index0;
    index[1] = index1;
    index[2] = index2;

    pSource[0] = pSource0;
    pSource[1] = pSource1;
    pSource[2] = pSource2;

    gpa_approximateGPAGPAGPARoots
            (
            pApproximateRoots,
            pSource0, index0,
            pSource1, index1,
            pSource2, index2,
            evaluateCurveCurveCurve,
            evaluateCurveCurveCurve,
            evaluateCurveCurveCurve,
            pContext->numSamples
            );

    gpa_improveTrivariateRoots (pConvergedRoots, pApproximateRoots,
            pSource0, index0,
            pSource1, index1,
            pSource2, index2,
            evaluateCurveCurveCurve,
            evaluateCurveCurveCurve,
            evaluateCurveCurveCurve
            );
    for (i = 0; jmdlGraphicsPointArray_getGraphicsPoint (pConvergedRoots, &gp, i); i++)
        {
        f[0] = gp.point.x;
        f[1] = gp.point.y;
        f[2] = gp.point.z;
        // Filter out duplicates appearing contiguously.  This does not catch
        if (!findParameterMatch (f, pTangentDest, 3, s_duplicateParameterTol))
            {
        for (k = 0; k < 3; k++)
            jmdlGraphicsPointArray_primitiveFractionToDPoint3d (pSource[k], &point[k], index[k], f[k]);

        if (bsiDEllipse3d_initFrom3DPoint3dOnArc
                (
                &ellipse,
                &point[0],
                &point[1],
                &point[2]
                ))
            {
            bsiDEllipse3d_makeFullSweep (&ellipse);
            if (pContext->pCircleDest)
                jmdlGraphicsPointArray_addDEllipse3d (pContext->pCircleDest, &ellipse);

            for (k = 0; k < 3; k++)
                {
                if (pTangentDest[k])
                    {
                    jmdlGraphicsPointArray_primitiveFractionToDPoint4d (pSource[k], &tangentPoint, index[k], f[k]);
                    jmdlGraphicsPointArray_addComplete
                            (
                            pTangentDest[k],
                            tangentPoint.x, tangentPoint.y, tangentPoint.z, tangentPoint.w,
                            f[k], 0, index[k]);
                    }
                }
            }
        }
        }
    jmdlGraphicsPointArray_drop (pConvergedRoots);
    jmdlGraphicsPointArray_drop (pApproximateRoots);
    return true;
    }
static bool gpa_cbapproximateGPAGPACurveCurveCurveRoots
(
      CurveCurveCurveContext    *pContext,
GraphicsPointArrayCP pSource0,
      int                       index0,
GraphicsPointArrayCP pSource1,
      int                       index1,
GraphicsPointArrayCP pSource2,
      int                       index2
)
    {
    if (gpa_cbapproximateGPAGPACurveCurveCurveRoots_Analytic (pContext, pSource0, index0, pSource1, index1, pSource2, index2))
        return true;
    else 
        return gpa_cbapproximateGPAGPACurveCurveCurveRoots_BySearch (pContext, pSource0, index0, pSource1, index1, pSource2, index2);
    }

static bool      gpaPerpTan_fractionValid
(
GPAPerpTanContext *pContext,
int             index,
double          fraction
)
    {
    static double s_fractionTol = 1.0e-10;
    return fraction >= -s_fractionTol && fraction <= 1.0 + s_fractionTol;
    }

static bool       gpaPerpTan_checkEnable
(
GPAPerpTanContext *pContext,
int sourceId,
int typeCode
)
    {
    if (sourceId < 0 || sourceId > 1)
        return false;
    if (typeCode == GPAPERPTAN_TANGENT)
        return pContext->bTanEnable[sourceId];
    if (typeCode == GPAPERPTAN_PERP)
        return pContext->bPerpEnable[sourceId];
    return false;
    }

static bool       gpaPerpTan_confirmParameterInBounds
(
GPAPerpTanContext *pContext,
int sourceId,
int index,
double fraction,
const DPoint4d *pPoint
)
    {
    return fraction >= 0.0 && fraction <= 1.0;
    }

static void    gpaPerpTan_saveLine
(
GPAPerpTanContext *pContext,
DPoint4d  *pPoint0,
int         source0,
int         index0,
double      fraction0,
int         typeCode0,
DPoint4d  *pPoint1,
int         source1,
int         index1,
double      fraction1,
int         typeCode1
)
    {
    if (source0 > source1)
        {
        gpaPerpTan_saveLine (
                             pContext, pPoint1, source1, index1, fraction1, typeCode1,
                                       pPoint0, source0, index0, fraction0, typeCode0
                            );
        }
    else
        {
        bool    sameCurve =
                pContext->pSource[0] == pContext->pSource[1]        /* Same source array ... */
            &&  index0 == index1;                                   /* Same primitive .... */
        bool    sameCondition = typeCode0 == typeCode1;

        double segmentLength = bsiDPoint4d_realDistance (pPoint0, pPoint1);
        GraphicsPoint gp[2];
        if (pContext->RejectBySegmentLength (segmentLength))
            {
            }
        else if (sameCurve &&
                (   fabs (fraction0 - fraction1) < s_sameFractionTol    /* Same place */
                || (sameCondition && fraction1 < fraction0))
            )
            {
            /* Ignore the degenerate point, or point which should reappear with indices
                swapped. */
            }
        else if (       pContext->pSource[0] == pContext->pSource[1]        /* Same source array ... */
            &&  index0 == index1                                    /* Same primitive .... */
            &&  fabs (fraction0 - fraction1) < s_sameFractionTol    /* Same place */
            )
            {
            /* Ignore the degenerate point */
            }
        else if (       gpaPerpTan_fractionValid (pContext, index0, fraction0)
            &&  gpaPerpTan_fractionValid (pContext, index1, fraction1)
            &&  gpaPerpTan_checkEnable (pContext, source0, typeCode0)
            &&  gpaPerpTan_checkEnable (pContext, source1, typeCode1)
            )
            {

            bsiGraphicsPoint_initFromDPoint4d (&gp[0], pPoint0, fraction0, 0, index0);
            if (typeCode0 == GPAPERPTAN_TANGENT)
                gp[0].mask |= HPOINT_MASK_USER1;
            bsiGraphicsPoint_initFromDPoint4d (&gp[1], pPoint1, fraction1, 0, index1);
            if (typeCode1 == GPAPERPTAN_TANGENT)
                gp[1].mask |= HPOINT_MASK_USER1;

            /* Try to insert consistently if source ids are reversed. */
            if (pContext->pCurves)
                {
                jmdlGraphicsPointArray_addGraphicsPoint (pContext->pCurves, &gp[0]);
                jmdlGraphicsPointArray_addGraphicsPoint (pContext->pCurves, &gp[1]);
                jmdlGraphicsPointArray_markBreak (pContext->pCurves);
                }

            if (pContext->pPointCollector0)
                jmdlGraphicsPointArray_addGraphicsPoint (pContext->pPointCollector0, &gp[0]);

            if (pContext->pPointCollector1)
                jmdlGraphicsPointArray_addGraphicsPoint (pContext->pPointCollector1, &gp[1]);
            }
        }
    }


static bool        gpaPerpTan_processDSegment4dDConic4d
(
GPAPerpTanContext   *pContext,
GraphicsPointArrayCP pSource0,
        int index0,
const DSegment4d    *pSegment0,
GraphicsPointArrayCP pSource1,
      int           index1,
const DConic4d      *pConic1
)
    {
    DPoint3d segmentTangent;
    DEllipse3d ellipse;
    double thetaArray[4];
    int numSolution;
    DPoint4d conicPoint, segmentPoint;
    int i;
    int conicRelation;
    DPoint4d directionVector;
    int pass;
    double theta;
    double f0, f1;
    double cosCoff[2] = {1.0, 0.0};
    double sinCoff[2] = {0.0, 1.0};
    int typeCode[2]   = {GPAPERPTAN_PERP, GPAPERPTAN_TANGENT};

    int source0 = pSource0 == pContext->pSource[0] ? 0 : 1;
    int source1 = 1 - source0;

    bsiDSegment4d_pseudoTangent (pSegment0, &segmentTangent);
    bsiDConic4d_pseudoTangent (pConic1, &ellipse);
    bsiDPoint4d_initFromDPoint3dAndWeight (&directionVector, &segmentTangent, 0.0);

    for (pass = 0; pass < 2; pass++)
        {
        /* Note that we use the segment tangent, not its perp, so the perp tan coffs are swapped */
        bsiDConic4d_angularRelationFromDPoint4dEyeXYW
                            (
                            pConic1, thetaArray, &numSolution, 4,
                            &directionVector, cosCoff[pass], sinCoff[pass]);
        conicRelation = typeCode[pass];

        for (i = 0; i < numSolution; i++)
            {
            theta = thetaArray[i];
            if (bsiDConic4d_angleInSweep (pConic1, theta))
                {
                bsiDConic4d_angleParameterToDPoint4d (pConic1, &conicPoint, theta);
                if (bsiDSegment4d_projectDPoint4dCartesianXYW (pSegment0, &segmentPoint, &f0, &conicPoint))
                    {
                    f1 = bsiDConic4d_angleParameterToFraction (pConic1, theta);
                    gpaPerpTan_saveLine (pContext,
                                            &segmentPoint, source0, index0, f0, GPAPERPTAN_PERP,
                                            &conicPoint,   source1, index1, f1, conicRelation);
                    }
                }
            }
        }
    return true;
    }


static bool    evaluatePerpendicularAtCurve
(
double *pf,
double *pd0,
double *pd1,
const DPoint3d  *pX0,
const DPoint3d  *pdX0,
const DPoint3d  *pddX0,
const DPoint3d  *pX1,
const DPoint3d  *pdX1,
const DPoint3d  *pddX1,
void            *pVoid,
int             workdim
)
    {
    DPoint3d vector01;
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vector01, pX1, pX0);
    if (workdim == 2)
        {
        *pf = bsiDPoint3d_dotProductXY (&vector01, pdX0);
        if (pd0 && pd1 && pddX0 && pddX1)
            {
            *pd0 = bsiDPoint3d_dotProductXY (&vector01, pddX0) - bsiDPoint3d_dotProductXY (pdX0, pdX0);
            *pd1 = bsiDPoint3d_dotProductXY (pdX1, pdX0);
            }
        }
    else
        {
        *pf = bsiDPoint3d_dotProduct (&vector01, pdX0);
        if (pd0 && pd1 && pddX0 && pddX1)
            {
            *pd0 = bsiDPoint3d_dotProduct (&vector01, pddX0) - bsiDPoint3d_dotProduct (pdX0, pdX0);
            *pd1 = bsiDPoint3d_dotProduct (pdX1, pdX0);
            }
        }
    return true;
    }

static bool    evaluateTangentAtCurve
(
double *pf,
double *pd0,
double *pd1,
const DPoint3d  *pX0,
const DPoint3d  *pdX0,
const DPoint3d  *pddX0,
const DPoint3d  *pX1,
const DPoint3d  *pdX1,
const DPoint3d  *pddX1,
void            *pVoid,
int             workdim     /* NOT USED -- THIS ONLY WORKS FOR XY.  Caveat Emptor. */
)
    {
    DPoint3d vector01;
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vector01, pX1, pX0);
    *pf = bsiDPoint3d_crossProductXY (&vector01, pdX0);
    if (pd0 && pd1 && pddX0 && pddX1)
        {
        *pd0 = bsiDPoint3d_crossProductXY (&vector01, pddX0) - bsiDPoint3d_crossProductXY (pdX0, pdX0);
        *pd1 = bsiDPoint3d_crossProductXY (pdX1, pdX0);
        }
    return true;
    }


static bool    gpa_improveOneCurveCurveRoot
(
double              *pParam0,
double              *pParam1,
const DPoint4d      *pPole0,
int                 order0,
CurveCurveFunc      pFunc0,
const DPoint4d      *pPole1,
int                 order1,
CurveCurveFunc      pFunc1,
void                *pVoid,
int                 workdim
)
    {
    double u = *pParam0;
    double v = *pParam1;
    double f0, f1;
    double f0u, f0v, f1u, f1v;
    DPoint3d point0, point1;
    DPoint3d tangent0, tangent1;
    DPoint3d curvature0, curvature1;
    double du, dv;
    int counter = 0;
    static double maxdelta = 0.1;
    static int s_maxCount = 55;
    static double s_uvTol = 1.0e-14;

    for (counter = 0; counter < s_maxCount; counter++)
        {
        bsiBezierDPoint4d_evaluateDPoint3dArrayExt (&point0, &tangent0, &curvature0,
                                pPole0, order0, &u, 1);
        bsiBezierDPoint4d_evaluateDPoint3dArrayExt (&point1, &tangent1, &curvature1,
                                pPole1, order1, &v, 1);
        pFunc0 (&f0, &f0u, &f0v,
                &point0, &tangent0, &curvature0,
                &point1, &tangent1, &curvature1,
                pVoid, workdim);

        pFunc1 (&f1, &f1v, &f1u,
                &point1, &tangent1, &curvature1,
                &point0, &tangent0, &curvature0,
                pVoid, workdim);

        if (bsiSVD_solve2x2 (&du, &dv,f0u, f0v, f1u, f1v, f0, f1))
            {
#define NOISYnot
#ifdef NOISY
            printf(" Newton Step u:(%le-%le)   v:(%le-%le)\n",
                            u, du, v, dv);
#endif
            if (fabs (du) > maxdelta)
                du = (du > 0.0 ? 1.0 : -1.0) * maxdelta;
            if (fabs (dv) > maxdelta)
                du = (dv > 0.0 ? 1.0 : -1.0) * maxdelta;
            u -= du;
            v -= dv;
            if (fabs (du) < s_uvTol && fabs (dv) < s_uvTol)
                {
                *pParam0 = u;
                *pParam1 = v;
                return true;
                }
            }
        }
    return false;
    }

static bool    gpa_improveCurveCurveRoots
(
GraphicsPointArrayP pGPA,
const DPoint4d      *pPole0,
int                 order0,
CurveCurveFunc      pFunc0,
const DPoint4d      *pPole1,
int                 order1,
CurveCurveFunc      pFunc1,
void                *pVoid,
int                 workdim
)
    {
    int i;
    GraphicsPoint gp;
    for (i = 0; jmdlGraphicsPointArray_getGraphicsPoint (pGPA, &gp, i); i++)
        {
        gpa_improveOneCurveCurveRoot (&gp.point.x, &gp.point.y,
                    pPole0, order0, pFunc0,
                    pPole1, order1, pFunc1, NULL, workdim);
        jmdlGraphicsPointArray_setGraphicsPoint (pGPA, &gp, i);
        }
    return true;
    }

static void gpa_approximateBezierBezierRoots
(
GPAPerpTanContext   *pContext,
GraphicsPointArrayP pGPA,
const DPoint4d      *pPole0,
int                 order0,
CurveCurveFunc      pFunc0,
const DPoint4d      *pPole1,
int                 order1,
CurveCurveFunc      pFunc1,
void                *pVoid,
int                 workdim
)
    {
#define SUBDIVISION_FACTOR 4
#define MAX_POINT_PER_CURVE (SUBDIVISION_FACTOR * MAX_BEZIER_CURVE_ORDER)
#define MAX_SURFACE_POINT (100 * 100)
    //double param0[MAX_POINT_PER_CURVE];
    double *param0 = (double*)BSIBaseGeom::Malloc (MAX_POINT_PER_CURVE * sizeof (double));
    //double param1[MAX_POINT_PER_CURVE];
    double *param1 = (double*)BSIBaseGeom::Malloc (MAX_POINT_PER_CURVE * sizeof (double));
    //DPoint3d tangent0[MAX_POINT_PER_CURVE];
    DPoint3d *tangent0 = (DPoint3d*)BSIBaseGeom::Malloc (MAX_POINT_PER_CURVE * sizeof (DPoint3d));
    //DPoint3d tangent1[MAX_POINT_PER_CURVE];
    DPoint3d *tangent1 = (DPoint3d*)BSIBaseGeom::Malloc (MAX_POINT_PER_CURVE * sizeof (DPoint3d));
    //DPoint3d point0[MAX_POINT_PER_CURVE];
    DPoint3d *point0 = (DPoint3d*)BSIBaseGeom::Malloc (MAX_POINT_PER_CURVE * sizeof (DPoint3d));
    //DPoint3d point1[MAX_POINT_PER_CURVE];
    DPoint3d *point1 = (DPoint3d*)BSIBaseGeom::Malloc (MAX_POINT_PER_CURVE * sizeof (DPoint3d));
    int num0 = SUBDIVISION_FACTOR * order0;
    int num1 = SUBDIVISION_FACTOR * order1;
    //double   a01[MAX_POINT_PER_CURVE * MAX_POINT_PER_CURVE];
    double *a01 = (double*)BSIBaseGeom::Malloc (MAX_POINT_PER_CURVE*MAX_POINT_PER_CURVE * sizeof (double));
    //double   a10[MAX_POINT_PER_CURVE * MAX_POINT_PER_CURVE];
    double *a10 = (double*)BSIBaseGeom::Malloc (MAX_POINT_PER_CURVE*MAX_POINT_PER_CURVE * sizeof (double));

    int i, j;
    if (num0 > MAX_POINT_PER_CURVE
        || num1 > MAX_POINT_PER_CURVE
        || num0 * num1 > MAX_SURFACE_POINT)
        {
        return;
        }
    bsiDoubleArray_uniformGrid (param0, num0, 0.0, 1.0);
    bsiDoubleArray_uniformGrid (param1, num1, 0.0, 1.0);
    bsiBezierDPoint4d_evaluateDPoint3dArray (point0, tangent0, pPole0, order0, param0, num0);
    bsiBezierDPoint4d_evaluateDPoint3dArray (point1, tangent1, pPole1, order1, param1, num1);

    for (i = 0; i < num0; i++)
        for (j = 0; j < num1; j++)
            {
            pFunc0 (
                                    &a01[i * num1 + j],
                                    NULL,
                                    NULL,
                                    point0 + i, tangent0 + i, NULL,
                                    point1 + j, tangent1 + j, NULL, pVoid,
                                    workdim
                                    );
            pFunc1 (
                                    &a10[i * num1 + j],
                                    NULL,
                                    NULL,
                                    point1 + j, tangent1 + j, NULL,
                                    point0 + i, tangent0 + i, NULL, pVoid,
                                    workdim
                                    );
            }

    jmdlGraphicsPointArray_addBilinearCommonRoots (pGPA,
                        a01,        0.0,        a10,    0.0,
                        num0, num1, num1, 1);

    BSIBaseGeom::Free (a10);
    BSIBaseGeom::Free (a01);
    BSIBaseGeom::Free (point1);
    BSIBaseGeom::Free (point0);
    BSIBaseGeom::Free (tangent1);
    BSIBaseGeom::Free (tangent0);
    BSIBaseGeom::Free (param1);
    BSIBaseGeom::Free (param0);
    }

static bool        gpaPerpTan_processDConic4dDConic4d_numeric
(
GPAPerpTanContext   *pContext,
GraphicsPointArrayCP pSource0,
        int index0,
const DConic4d    *pConic0,
GraphicsPointArrayCP pSource1,
      int           index1,
const DConic4d      *pConic1
)
    {
    //bool isCircular0 = bsiDConic4d_isCircularXY (pConic0);
    //bool isCircular1 = bsiDConic4d_isCircularXY (pConic1);
    int source0 = pSource0 == pContext->pSource[0] ? 0 : 1;
    int source1 = 1 - source0;
    int depth = 0;

    if (   depth == 0)
        {
        DPoint4d pole0[BEZIER_CONVERSION_POLES], pole1[BEZIER_CONVERSION_POLES];
        DPoint3d trig0[BEZIER_CONVERSION_POLES], trig1[BEZIER_CONVERSION_POLES];
        int passCode0[4] = {GPAPERPTAN_PERP, GPAPERPTAN_PERP, GPAPERPTAN_TANGENT, GPAPERPTAN_TANGENT};
        int passCode1[4] = {GPAPERPTAN_PERP, GPAPERPTAN_TANGENT, GPAPERPTAN_PERP, GPAPERPTAN_TANGENT};
 
        CurveCurveFunc func0, func1;
        GraphicsPointArrayP pGPA = jmdlGraphicsPointArray_grab ();
        DPoint4d pointOn0, pointOn1;

        DPoint4d *pCurrPole0;
        DPoint4d *pCurrPole1;
        DPoint4d *pCurrLocalPole0;
        DPoint4d *pCurrLocalPole1;

        int numSpan0, numSpan1;
        int numPole0, numPole1;
        double f0, f1;
        int i0, i1, k;
        int pass;
        DPoint3d *pCurrTrig0, *pCurrTrig1;

        DMatrix4d worldToLocal;
        DPoint4d *pLocalPoleArray0 = (DPoint4d*)_alloca (BEZIER_CONVERSION_POLES * sizeof (DPoint4d));
        DPoint4d *pLocalPoleArray1 = (DPoint4d*)_alloca (BEZIER_CONVERSION_POLES * sizeof (DPoint4d));


        bsiDConic4d_getQuadricBezierPoles (pConic0, pole0, trig0, &numPole0, &numSpan0, BEZIER_CONVERSION_POLES);
        bsiDConic4d_getQuadricBezierPoles (pConic1, pole1, trig1, &numPole1, &numSpan1, BEZIER_CONVERSION_POLES);

        bsiDMatrix4d_initForDPoint4dOrigin (&worldToLocal, NULL, &pole0[0]);
        bsiDMatrix4d_multiply4dPoints (&worldToLocal, pLocalPoleArray0, pole0, numPole0);
        bsiDMatrix4d_multiply4dPoints (&worldToLocal, pLocalPoleArray1, pole1, numPole1);


        for (i0 = 0; i0 < numSpan0; i0++)
            {
            for (i1 = 0; i1 < numSpan1; i1++)
                {
                GraphicsPoint gp;
                pCurrLocalPole0 = pLocalPoleArray0 + 2 * i0;
                pCurrLocalPole1 = pLocalPoleArray1 + 2 * i1;
                pCurrPole0      = pole0 + 2 * i0;
                pCurrPole1      = pole1 + 2 * i1;
                pCurrTrig0 = trig0 + 2 * i0;
                pCurrTrig1 = trig1 + 2 * i1;

                for (pass = 0; pass < 4; pass++)
                    {
                    if  (   gpaPerpTan_checkEnable (pContext, source0, passCode0[pass])
                         && gpaPerpTan_checkEnable (pContext, source1, passCode1[pass]))
                        {
                        func0 = passCode0[pass] == GPAPERPTAN_PERP
                            ? evaluatePerpendicularAtCurve
                            : evaluateTangentAtCurve;
                        func1 = passCode1[pass] == GPAPERPTAN_PERP
                            ? evaluatePerpendicularAtCurve
                            : evaluateTangentAtCurve;

                        jmdlGraphicsPointArray_empty (pGPA);
                        gpa_approximateBezierBezierRoots
                                    (
                                    pContext,
                                    pGPA,
                                    pCurrLocalPole0, 3, func0,
                                    pCurrLocalPole1, 3, func1,
                                    NULL, pContext->workdim
                                    );
                        gpa_improveCurveCurveRoots
                                    (
                                    pGPA,
                                    pCurrLocalPole0,   3, func0,
                                    pCurrLocalPole1,   3, func1,
                                    NULL, pContext->workdim
                                    );


                        for (k = 0; jmdlGraphicsPointArray_getGraphicsPoint (pGPA, &gp, k); k++)
                            {
                            DPoint3d trigPoint0, trigPoint1;
                            double theta0, theta1;
                            double u0 = gp.point.x;
                            double u1 = gp.point.y;
                            bsiBezierDPoint4d_evaluateArray (&pointOn0, NULL, pCurrPole0, 3, &u0, 1);
                            bsiBezierDPoint4d_evaluateArray (&pointOn1, NULL, pCurrPole1, 3, &u1, 1);
                            bsiBezierDPoint3d_evaluateArray (&trigPoint0, NULL, pCurrTrig0, 3, &u0, 1);
                            bsiBezierDPoint3d_evaluateArray (&trigPoint1, NULL, pCurrTrig1, 3, &u1, 1);
                            theta0 = atan2 (trigPoint0.y, trigPoint0.x);
                            theta1 = atan2 (trigPoint1.y, trigPoint1.x);
                            f0 = bsiDConic4d_angleParameterToFraction (pConic0, theta0);
                            f1 = bsiDConic4d_angleParameterToFraction (pConic1, theta1);
                            gpaPerpTan_saveLine (pContext,
                                                    &pointOn0, source0, index0, f0, passCode0[pass],
                                                    &pointOn1, source1, index1, f1, passCode1[pass]
                                                );
                            }
                        }
                    }
                }
            }
        jmdlGraphicsPointArray_drop (pGPA);
        }
    return true;
    }

// Hold 2 dconic4d, parameterized as X(u) and Y(v)
// Calculate 2 functions of u and v
//  f0 = (X - Y). X'
//  f1 = (X - Y). Y'
// (These will both be zero at min and max distance)
struct DConic4dDConic4d_ExtremalDistanceFunction : FunctionRRToRR
{
DConic4d m_conicA, m_conicB;
double m_scaleFactor;
public:
DConic4dDConic4d_ExtremalDistanceFunction (DConic4dCR conicA, DConic4d conicB, double scaleFactor)
    {
    m_conicA = conicA;
    m_conicB = conicB;
    m_scaleFactor = scaleFactor;
    }
// Virtual function
// @param u IN first variable
// @param v IN second variable
// @param f OUT first function value
// @param g OUT second function value
bool EvaluateRRToRR
(
double u,
double v,
double &f,
double &g
) override
    {
    DPoint3d X, Y;
    DVec3d   U, V, E;
    bsiDConic4d_angleParameterToDPoint3dDerivatives (&m_conicA, &X, &U, NULL, u);
    bsiDConic4d_angleParameterToDPoint3dDerivatives (&m_conicB, &Y, &V, NULL, v);
    E.DifferenceOf(X, Y);
    f = E.DotProduct (U) * m_scaleFactor;
    g = E.DotProduct (V) * m_scaleFactor;
    return true;
    }
};

// Carry a conic and various data needed when searching for perpendiculars.
struct ConicPerpData
{
DConic4d m_conic;
DEllipse3d m_tangentSpace;
DVec3d   m_normal;
DPoint3d m_center;
double   m_scale;   // squared distance scale factor
double m_theta;
DPoint3d m_xyz;
DVec3d   m_tangent;
DVec3d   m_perp;
bool     m_valid;
double   m_perpFunc;
double   m_testDistance;
ConicPerpData ()
    {
    memset (this, 0, sizeof (ConicPerpData));
    }
ConicPerpData (DConic4dCR conic)
    {
    m_conic = conic;
    bsiDConic4d_pseudoTangent (&conic, &m_tangentSpace);
    DVec3d tangent0, tangent90;
    tangent0.SumOf (*((DVec3d*)&m_tangentSpace.center),m_tangentSpace.vector0);
    tangent90.SumOf (*((DVec3d*)&m_tangentSpace.center),m_tangentSpace.vector90);
    m_scale = m_normal.NormalizedCrossProduct (tangent0, tangent90);
    conic.center.GetProjectedXYZ (m_center);
    Evaluate (0.0);
    m_valid = false;
    m_perpFunc = DBL_MAX;
    }
bool PlaneContainsPoint (DPoint3dCR xyz)
    {
    double relTol = bsiTrig_smallAngle ();
    double d = xyz.DotDifference (m_center, m_normal);
    return fabs (d) < relTol * (sqrt (m_scale) + xyz.Magnitude ());
    }
// Fill in m_theta, m_xyz, m_tangent, m_perp
void Evaluate (double theta)
    {
    m_theta = theta;
    bsiDConic4d_angleParameterToDPoint3dDerivatives (&m_conic, &m_xyz, &m_tangent, NULL, theta);
    m_perp.NormalizedCrossProduct (m_tangent, m_normal);
    }

// Return distance bewteen m_xyz of this and other ...
double Distance (ConicPerpData &other)
    {
    return m_xyz.Distance (other.m_xyz);
    }

DPoint4d GetPerpendicularPlane ()
    {
    DPoint4d plane;
    bsiDPoint4d_planeFromOriginAndNormal (&plane, &m_xyz, &m_tangent);
    return plane;
    }
// Evaluate the perpendicularity function between this->m_xyz, this->m_tangent, and other->m_xyz.
void EvaluatePerpFunc (ConicPerpData &other)
    {
    m_testDistance = m_xyz.Distance (other.m_xyz);
    m_perpFunc = m_xyz.DotDifference (other.m_xyz, m_tangent) / m_scale;
    }

void SetValid (bool value) {m_valid = value;}

// Complete swap of contents of two ConicPerpData instances ...
static void Swap (ConicPerpData &first, ConicPerpData &other)
    {
    ConicPerpData save = first;
    first = other;
    other = save;
    }

// Run iterative step if the function value interval {B0.m_perpFunc,B1.m_perpFunc} brackets 0.
// @return true if a solution pair {thetaA, thetaB} is determined.
// @param [in] A0 Conic A, with point evaluated at A0.m_theta.
// @param [in] A1 Conic A, with point evaluated at A1.m_theta.
// @param [in] B0 Conic B, with B0.m_theta as approximate perp point from A0. and B0.m_perpFunc evaluated
// @param [in] B1 Conic B, with B1.m_theta as approximate perp point from A1, and B1.m_perpFunc evaluated
// @param [out] thetaA solution angle on conic A
// @param [out] thetaB solution angle on conic B
static bool SolvePerpPerp
(
ConicPerpData &A0,
ConicPerpData &A1,
ConicPerpData &B0,
ConicPerpData &B1,
double &thetaA,
double &thetaB
)
    {
    if (   !B0.m_valid
        || !B1.m_valid
        )
        return false;
    if (B1.m_perpFunc * B0.m_perpFunc > 0.0)
        return false;
    NewtonIterationsRRToRR newton (bsiTrig_smallAngle ());
    DConic4dDConic4d_ExtremalDistanceFunction evaluator (A0.m_conic, B0.m_conic, 1.0 / A0.m_scale);
    double s = -B0.m_perpFunc / (B1.m_perpFunc - B0.m_perpFunc);
    double uA = A0.m_theta + s * (A1.m_theta - A0.m_theta);
    double uB = B0.m_theta + s * (B1.m_theta - B0.m_theta);
    if (newton.RunApproximateNewton (uA, uB, evaluator, 0.1, 0.1))
        {
        thetaA = uA;
        thetaB = uB;
        return true;
        }
    return false;
    }
};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DavidAssaf      06/06
+---------------+---------------+---------------+---------------+---------------+------*/
static void    gpaPerpTan_addLinePair
(
GPAPerpTanContext*  pContext,
const DConic4d*     pConic0,
int                 index0,
const double        phi0[2],
double              theta0,
double              sweep0,
int                 typeCode0,
const DConic4d*     pConic1,
int                 index1,
const double        phi1[2],
double              theta1,
double              sweep1,
int                 typeCode1,
int                 numPhi = 2
)
    {
    DPoint4d    point0, point1;
    double      f0, f1;
    int         i;

    for (i = 0; i < 2; i++)
        {
        if (bsiTrig_angleInSweep (phi0[i], theta0, sweep0) &&
            bsiTrig_angleInSweep (phi1[i], theta1, sweep1))
            {
            f0 = bsiTrig_normalizeAngleToSweep (phi0[i], theta0, sweep0);
            f1 = bsiTrig_normalizeAngleToSweep (phi1[i], theta1, sweep1);
            bsiDConic4d_fractionParameterToDPoint4dDerivatives (pConic0, &point0, NULL, NULL, f0);
            bsiDConic4d_fractionParameterToDPoint4dDerivatives (pConic1, &point1, NULL, NULL, f1);
            gpaPerpTan_saveLine (pContext, &point0, 0, index0, f0, typeCode0,
                                           &point1, 1, index1, f1, typeCode1);
            }
        }
    }

// Attempt to solve for perpendiculars between two conics.
static bool gpaPerpTan_processDConic4dDConic4d_parallelAxisPerpPerp
(
GPAPerpTanContext   *pContext,
GraphicsPointArrayCP pSourceA,
        int indexA,
const DConic4d    *pConicA,
GraphicsPointArrayCP pSourceB,
      int           indexB,
const DConic4d      *pConicB
)
    {
    ConicPerpData conicA (*pConicA);
    ConicPerpData conicB (*pConicB);
    if (  0.0 == conicA.m_normal.Magnitude ()
       || 0.0 == conicB.m_normal.Magnitude ()
#ifdef SAMPLE_BY_PLANE_INTERSECTION
#else
       || !conicA.m_normal.IsParallelTo (conicB.m_normal)
       || !conicA.PlaneContainsPoint (conicB.m_center)
       || !conicB.PlaneContainsPoint (conicA.m_center)
#endif
       )
        return false;

    static int numIntervalPerRadian = 40;
    int numIntervalA = (int)(fabs (pConicA->sweep) * numIntervalPerRadian);
    if (numIntervalA < 1)
        numIntervalA = 2;
    double dThetaA = pConicA->sweep / numIntervalA;
    ConicPerpData dataA[2], dataB[2][2];
    dataA[0] = ConicPerpData (*pConicA);
    dataA[1] = ConicPerpData (*pConicA);

    dataB[0][0] = ConicPerpData (*pConicB);
    dataB[0][1] = ConicPerpData (*pConicB);

    dataB[1][0] = ConicPerpData (*pConicB);
    dataB[1][1] = ConicPerpData (*pConicB);
    double thetaB[2];
    int numB[2];
    // Evaluate conicA at {numIntervalA+1} equally spaced points.
    //   dataA[0], dataA[1] are two successive points.
    // At each, find the two points of conicB that share tangent.
    //   dataB[0][0], dataB[1][0] are two successive "smaller distance" points.
    //   dataB[1][0], dataB[1][1] are two successive "larger distance" points.
    // 
    for (int i = 0;
            i <= numIntervalA;
            i++,
            numB[0] = numB[1],
            dataA[0] = dataA[1],
            dataB[0][0] = dataB[1][0],
            dataB[0][1] = dataB[1][1]
        )
        {
        double thetaA1 = pConicA->start + i * dThetaA;
        dataA[1].Evaluate (thetaA1);
        // concentric ellipses with same axes but different radii have subtle min and max distance behavior.
        // We know the tangent (and perpendicular) of conicA at thetaA
        // Find the (2) angles on conicB that have the same perpendicular.
        // If thetaA happens to be one end of a min or max distance pair, the points on conicB will be
        //    candidates for the other end, and the joining segment will be perpendicular to the (parallel) tangents.
        //    (one conicB pointis a candidate min, the other a candidate max)
        // For other thetA, the joining segment will NOT be perpendicular to either of the (parallel) tangnets.
        // The condition {joiningSegmentVector.DotProduct (*tangent)} is thus 0 at the solution points and
        // nonzero elsewhere.
        // Sooo
        //  for "many" points pointA[i] spaced on conicA:
        //      if {joiningSegmentVector.DotProduct (*tangent)} changes sign between pointA[i] and pointA[i+1]
        //           run Newton iteration to improve the root.
#ifdef SAMPLE_BY_PLANE_INTERSECTION
        DPoint3d trigPointB[2];
        DPoint4d planePerpA = dataA[1].GetPerpendicularPlane ();
        numB[1] = bsiDConic4d_intersectPlane (pConicB, trigPointB, &planePerpA);
        for (int k = 0; k < numB[1]; k++)
            thetaB[k] = trigPointB[k].z;   // That's the angle parameter
#else
        numB[1] = bsiDConic4d_solveTangentsPerpendicularToVector ((DConic4dP)pConicB, thetaB, dataA[1].m_perp);
#endif
        if (numB[1] == 2)
            {
            dataB[1][0].Evaluate (thetaB[0]);
            dataB[1][1].Evaluate (thetaB[1]);
            if (dataA[1].Distance (dataB[1][0]) > dataA[1].Distance (dataB[1][1]) )
                ConicPerpData::Swap (dataB[1][0], dataB[1][1]);
            dataB[1][0].EvaluatePerpFunc (dataA[1]);
            dataB[1][1].EvaluatePerpFunc (dataA[1]);
            dataB[1][0].SetValid (true);
            dataB[1][1].SetValid (true);
            }
        else
            {
            dataB[1][0].SetValid (false);
            dataB[1][1].SetValid (false);
            }
        if (i > 0)
            {
            double thetaA, thetaB;
            for (int iB = 0; iB < 1; iB++)
                {
                // iB==0 is "min distance"
                // iB==1 is "max distance"
                if (ConicPerpData::SolvePerpPerp (dataA[0], dataA[1], dataB[0][iB], dataB[1][iB], thetaA, thetaB))
                    {
                    gpaPerpTan_addLinePair (
                            pContext,
                            pConicA, indexA,  &thetaA, pConicA->start, pConicA->sweep, GPAPERPTAN_PERP,
                            pConicB, indexB,  &thetaB, pConicB->start, pConicB->sweep, GPAPERPTAN_PERP,
                            1);
                    }
                }
            }
        }                        
    return true;
    }


// Perp-tan search for pair of ellipses.
static bool        gpaPerpTan_processDConic4dDConic4d
(
GPAPerpTanContext   *pContext,
GraphicsPointArrayCP pSource0,
        int index0,
const DConic4d    *pConic0,
GraphicsPointArrayCP pSource1,
      int           index1,
const DConic4d      *pConic1
)
    {
    DEllipse3d  ellipse0, ellipse1;
    DPoint3d    center0, center1;
    RotMatrix   matrix0, matrix1;
    double      r0, r1;
    double      theta0, sweep0, theta1, sweep1;
    double      phi0[2], phi1[2];

    // Special case for bitwise-identical ellipses
    if (0 == memcmp (pConic0, pConic1, sizeof (DConic4d)) &&
        bsiDEllipse3d_initFromDConic4d (&ellipse0, pConic1))
        {
        // all antipodes would be perp-perp, but none are special (as in ellipse case) so don't return any
        if (bsiDEllipse3d_isCircular (&ellipse0))
            return true;

        // An ellipse cannot have a chord that is perpendicular to itself.
        if  (   !gpaPerpTan_checkEnable (pContext, 0, GPAPERPTAN_PERP)
             && !gpaPerpTan_checkEnable (pContext, 1, GPAPERPTAN_PERP))
            return false;

        bsiDEllipse3d_getScaledRotMatrix (&ellipse0, &center0, &matrix0, &r0, &r1, &theta0, &sweep0);

        phi0[0] =  0.0;
        phi0[1] =  msGeomConst_piOver2;
        phi1[0] =  msGeomConst_pi;
        phi1[1] = -msGeomConst_piOver2;

        gpaPerpTan_addLinePair (pContext, pConic0, index0, phi0, theta0, sweep0, GPAPERPTAN_PERP,
                                          pConic1, index1, phi1, theta0, sweep0, GPAPERPTAN_PERP);
        return true;
        }

    // TR #189597: Special case coplanar circles
    if (bsiDConic4d_isCircular (pConic0) &&
        bsiDConic4d_isCircular (pConic1) &&
        bsiDEllipse3d_initFromDConic4d (&ellipse0, pConic0) &&
        bsiDEllipse3d_initFromDConic4d (&ellipse1, pConic1))
        {
        DVec3d normal0, normal1, centerVector01;
        double      centerDistance;
        bsiDPoint3d_normalizedCrossProduct (&normal0, &ellipse0.vector0, &ellipse0.vector90);
        bsiDPoint3d_normalizedCrossProduct (&normal1, &ellipse1.vector0, &ellipse1.vector90);
        centerDistance = bsiDPoint3d_computeNormal (&centerVector01, &ellipse1.center, &ellipse0.center);

        // are circles coplanar?
        if (bsiDPoint3d_areParallel (&normal0, &normal1)
            && bsiDPoint3d_arePerpendicular (&centerVector01, &normal0))
            {
            DPoint3d    axisPoint0, axisPoint1;
            double      alpha, angleAtAxisPoint0, angleAtAxisPoint1;

            bsiDEllipse3d_getScaledRotMatrix (&ellipse0, &center0, &matrix0, &r0, &r0, &theta0, &sweep0);
            bsiDEllipse3d_getScaledRotMatrix (&ellipse1, &center1, &matrix1, &r1, &r1, &theta1, &sweep1);

            // points along center line closer to opposite ellipse
            bsiDPoint3d_addScaledDPoint3d (&axisPoint0, &center0, &centerVector01,  r0);
            bsiDPoint3d_addScaledDPoint3d (&axisPoint1, &center1, &centerVector01, -r1);

            // angles at axisPoints
            angleAtAxisPoint0 = bsiDConic4d_DPoint3dToAngle (pConic0, &axisPoint0);
            angleAtAxisPoint1 = bsiDConic4d_DPoint3dToAngle (pConic1, &axisPoint1);

            // Intersection points satisfy any condition ...
            if (r0 + r1 > centerDistance)
                {
                double a, b, bb;
                if (   bsiTrig_safeDivide (&a, r0 * r0 - r1 * r1 + centerDistance * centerDistance, 2.0 * centerDistance, 0.0)
                   && (bb = r0 * r0 - a * a) > 0.0
                   )
                    {
                    int i;
                    DPoint3d intersection[2];
                    DVec3d perpVector;
                    b = sqrt (bb);
                    bsiDVec3d_normalizedCrossProduct (&perpVector, &normal0, &centerVector01);
                    bsiDPoint3d_add2ScaledDVec3d (&intersection[0], &center0, &centerVector01, a, &perpVector,  b);
                    bsiDPoint3d_add2ScaledDVec3d (&intersection[1], &center0, &centerVector01, a, &perpVector, -b);
                    for (i = 0; i < 2; i++)
                        {
                        DPoint3d xyz = intersection[i];
                        double angle0 = bsiDEllipse3d_pointToAngle (&ellipse0, &xyz);
                        double angle1 = bsiDEllipse3d_pointToAngle (&ellipse1, &xyz);
                        if (   bsiTrig_angleInSweep (angle0, ellipse0.start, ellipse0.sweep)
                            && bsiTrig_angleInSweep (angle1, ellipse1.start, ellipse1.sweep))
                            {
                            double f0 = bsiDEllipse3d_angleToFraction (&ellipse0, angle0);
                            double f1 = bsiDEllipse3d_angleToFraction (&ellipse1, angle1);
                            DPoint4d xyzw;
                            bsiDPoint4d_initFromDPoint3dAndWeight (&xyzw, &xyz, 1.0);
                            gpaPerpTan_saveLine (pContext, &xyzw, 0, index0, f0, GPAPERPTAN_PERP,
                                                           &xyzw, 1, index1, f1, GPAPERPTAN_PERP);
                            }
                        }
                    }
                }

            // perp-perp segments lie on center line
            if (gpaPerpTan_checkEnable (pContext, 0, GPAPERPTAN_PERP) &&
                gpaPerpTan_checkEnable (pContext, 1, GPAPERPTAN_PERP))
                {
                phi0[0] = phi0[1] = angleAtAxisPoint0;
                phi1[0] = angleAtAxisPoint1;
                phi1[1] = angleAtAxisPoint1 + msGeomConst_pi;

                gpaPerpTan_addLinePair (pContext, pConic0, index0, phi0, theta0, sweep0, GPAPERPTAN_PERP,
                                                  pConic1, index1, phi1, theta1, sweep1, GPAPERPTAN_PERP);

                phi0[0] = phi0[1] = angleAtAxisPoint0 + msGeomConst_pi;

                gpaPerpTan_addLinePair (pContext, pConic0, index0, phi0, theta0, sweep0, GPAPERPTAN_PERP,
                                                  pConic1, index1, phi1, theta1, sweep1, GPAPERPTAN_PERP);
                }

            // perp-tan segments extend through ellipse0's center
            if (gpaPerpTan_checkEnable (pContext, 0, GPAPERPTAN_PERP) &&
                gpaPerpTan_checkEnable (pContext, 1, GPAPERPTAN_TANGENT) &&
                centerDistance > 0.0 && centerDistance >= MIN (r0, r1))
                {
                alpha = asin (r1 / centerDistance);

                phi0[0] = angleAtAxisPoint0 + alpha;
                phi0[1] = angleAtAxisPoint0 - alpha;
                phi1[0] = angleAtAxisPoint1 - (msGeomConst_piOver2 - alpha);
                phi1[1] = angleAtAxisPoint1 + (msGeomConst_piOver2 - alpha);

                gpaPerpTan_addLinePair (pContext, pConic0, index0, phi0, theta0, sweep0, GPAPERPTAN_PERP,
                                                  pConic1, index1, phi1, theta1, sweep1, GPAPERPTAN_TANGENT);
                }

            // tan-perp segments extend through ellipse1's center
            if (gpaPerpTan_checkEnable (pContext, 0, GPAPERPTAN_TANGENT) &&
                gpaPerpTan_checkEnable (pContext, 1, GPAPERPTAN_PERP) &&
                centerDistance > 0.0 && centerDistance >= MIN (r0, r1))
                {
                alpha = asin (r0 / centerDistance);

                phi0[0] = angleAtAxisPoint0 + alpha;
                phi0[1] = angleAtAxisPoint0 - alpha;
                phi1[0] = angleAtAxisPoint1 - (msGeomConst_piOver2 - alpha);
                phi1[1] = angleAtAxisPoint1 + (msGeomConst_piOver2 - alpha);

                gpaPerpTan_addLinePair (pContext, pConic0, index0, phi0, theta0, sweep0, GPAPERPTAN_TANGENT,
                                                  pConic1, index1, phi1, theta1, sweep1, GPAPERPTAN_PERP);
                }

            // tan-tan segments
            if (gpaPerpTan_checkEnable (pContext, 0, GPAPERPTAN_TANGENT) &&
                gpaPerpTan_checkEnable (pContext, 1, GPAPERPTAN_TANGENT) &&
                centerDistance > 0.0 && centerDistance >= fabs (r0 - r1))
                {
                DPoint3d    extSimCenter;

                // using internal similitude center = intersection of interior tangent lines = (r0*center1 + r1*center0)/(r1 + r0)
                if (centerDistance >= r0 + r1)
                    {
                    alpha = acos ((r1 + r0) / centerDistance);

                    phi0[0] = angleAtAxisPoint0 + alpha;
                    phi0[1] = angleAtAxisPoint0 - alpha;
                    phi1[0] = angleAtAxisPoint1 + alpha;
                    phi1[1] = angleAtAxisPoint1 - alpha;

                    gpaPerpTan_addLinePair (pContext, pConic0, index0, phi0, theta0, sweep0, GPAPERPTAN_TANGENT,
                                                      pConic1, index1, phi1, theta1, sweep1, GPAPERPTAN_TANGENT);
                    }

                // using external similitude center = intersection of exterior tangent lines = (r0*center1 - r1*center0)/(r1 - r0)
                if (r0 > r1)
                    {
                    bsiDPoint3d_add2ScaledDPoint3d (&extSimCenter, NULL, &center1, r0, &center0, -r1);
                    bsiDPoint3d_scaleInPlace (&extSimCenter, r1 - r0);
                    alpha = acos (r0 / bsiDPoint3d_distance (&extSimCenter, &center0));

                    phi0[0] = angleAtAxisPoint0 + alpha;
                    phi0[1] = angleAtAxisPoint0 - alpha;
                    phi1[0] = angleAtAxisPoint1 + msGeomConst_pi + alpha;
                    phi1[1] = angleAtAxisPoint1 + msGeomConst_pi - alpha;
                    }
                else if (r0 < r1)
                    {
                    bsiDPoint3d_add2ScaledDPoint3d (&extSimCenter, NULL, &center1, r0, &center0, -r1);
                    bsiDPoint3d_scaleInPlace (&extSimCenter, r1 - r0);
                    alpha = acos (r1 / bsiDPoint3d_distance (&extSimCenter, &center1));

                    phi0[0] = angleAtAxisPoint0 + msGeomConst_pi + alpha;
                    phi0[1] = angleAtAxisPoint0 + msGeomConst_pi - alpha;
                    phi1[0] = angleAtAxisPoint1 + alpha;
                    phi1[1] = angleAtAxisPoint1 - alpha;
                    }
                else
                    {
                    alpha = msGeomConst_piOver2;

                    phi0[0] = angleAtAxisPoint0 + alpha;
                    phi0[1] = angleAtAxisPoint0 - alpha;
                    phi1[0] = angleAtAxisPoint1 - alpha;
                    phi1[1] = angleAtAxisPoint1 + alpha;
                    }

                gpaPerpTan_addLinePair (pContext, pConic0, index0, phi0, theta0, sweep0, GPAPERPTAN_TANGENT,
                                                  pConic1, index1, phi1, theta1, sweep1, GPAPERPTAN_TANGENT);
                }

            return true;
            }

        // fall through to numeric case
        }

    // TR#300386 Special case for ellipse-ellipse
   if (   gpaPerpTan_checkEnable (pContext, 0, GPAPERPTAN_PERP)
      && !gpaPerpTan_checkEnable (pContext, 0, GPAPERPTAN_TANGENT)
      &&  gpaPerpTan_checkEnable (pContext, 1, GPAPERPTAN_PERP)
      && !gpaPerpTan_checkEnable (pContext, 1, GPAPERPTAN_TANGENT)
        )
        {
        if (gpaPerpTan_processDConic4dDConic4d_parallelAxisPerpPerp
                    (
                    pContext,
                    pSource0, index0, pConic0,
                    pSource1, index1, pConic1
                    ))
            return true;
        }
    return gpaPerpTan_processDConic4dDConic4d_numeric (pContext, pSource0, index0, pConic0,
                                                                 pSource1, index1, pConic1);
    }


static bool        gpaPerpTan_processDConic4dBezierDPoint4dTagged
(
GPAPerpTanContext   *pContext,
GraphicsPointArrayCP pSource0,
        int index0,
const DConic4d    *pConic0,
TaggedBezierDPoint4d &bezier1
)
    {
    //bool isCircular0 = bsiDConic4d_isCircularXY (pConic0);
    int source0 = pSource0 == pContext->pSource[0] ? 0 : 1;
    int source1 = 1 - source0;
    int depth = 0;

    if (   depth == 0)
        {
        DPoint4d pole0[BEZIER_CONVERSION_POLES];
        DPoint3d trig0[BEZIER_CONVERSION_POLES];
        int passCode0[4] = {GPAPERPTAN_PERP, GPAPERPTAN_PERP, GPAPERPTAN_TANGENT, GPAPERPTAN_TANGENT};
        int passCode1[4] = {GPAPERPTAN_PERP, GPAPERPTAN_TANGENT, GPAPERPTAN_PERP, GPAPERPTAN_TANGENT};

        CurveCurveFunc func0, func1;
        GraphicsPointArrayP pGPA = jmdlGraphicsPointArray_grab ();
        DPoint4d pointOn0, pointOn1;
        DPoint4d *pCurrPole0;
        DPoint4d *pCurrLocalPole0;
        int numSpan0;
        int numPole0;
        int i0, k;
        int pass;
        DMatrix4d worldToLocal;
        DPoint4d *pLocalPoleArray0 = (DPoint4d*)_alloca (BEZIER_CONVERSION_POLES * sizeof (DPoint4d));
        DPoint4d *pLocalPoleArray1 = (DPoint4d*)_alloca (bezier1.m_order * sizeof (DPoint4d));

        DPoint3d *pCurrTrig0;
        bsiDConic4d_getQuadricBezierPoles (pConic0, pole0, trig0, &numPole0, &numSpan0, BEZIER_CONVERSION_POLES);

        /* Compute parameters in local coordinates only ... */
        bsiDMatrix4d_initForDPoint4dOrigin (&worldToLocal, NULL, &pole0[0]);
        bsiDMatrix4d_multiply4dPoints (&worldToLocal, pLocalPoleArray0, pole0, numPole0);
        bsiDMatrix4d_multiply4dPoints (&worldToLocal, pLocalPoleArray1, bezier1.m_poles, bezier1.m_order);

        for (i0 = 0; i0 < numSpan0; i0++)
            {
            GraphicsPoint gp;
            pCurrLocalPole0 = pLocalPoleArray0 + 2 * i0;
            pCurrPole0 = pole0 + 2 * i0;
            pCurrTrig0 = trig0 + 2 * i0;

            for (pass = 0; pass < 4; pass++)
                {
                if  (   gpaPerpTan_checkEnable (pContext, source0, passCode0[pass])
                     && gpaPerpTan_checkEnable (pContext, source1, passCode1[pass]))
                    {
                    func0 = passCode0[pass] == GPAPERPTAN_PERP
                        ? evaluatePerpendicularAtCurve
                        : evaluateTangentAtCurve;
                    func1 = passCode1[pass] == GPAPERPTAN_PERP
                        ? evaluatePerpendicularAtCurve
                        : evaluateTangentAtCurve;

                    jmdlGraphicsPointArray_empty (pGPA);
                    gpa_approximateBezierBezierRoots
                                (
                                pContext,
                                pGPA,
                                pCurrLocalPole0,  3, func0,
                                pLocalPoleArray1, bezier1.m_order, func1,
                                NULL, pContext->workdim
                                );
                    gpa_improveCurveCurveRoots
                                (
                                pGPA,
                                pCurrLocalPole0,    3, func0,
                                pLocalPoleArray1,   bezier1.m_order, func1,
                                NULL, pContext->workdim
                                );


                    for (k = 0; jmdlGraphicsPointArray_getGraphicsPoint (pGPA, &gp, k); k++)
                        {
                        DPoint3d trigPoint0;
                        double theta0;
                        double u0 = gp.point.x;
                        double f1 = gp.point.y;
                        double g1 = bezier1.LocalToGlobal (f1);
                        bsiBezierDPoint4d_evaluateArray (&pointOn0, NULL, pCurrPole0, 3, &u0, 1);
                        bsiBezierDPoint4d_evaluateArray (&pointOn1, NULL, bezier1.m_poles, bezier1.m_order, &f1, 1);
                        bsiBezierDPoint3d_evaluateArray (&trigPoint0, NULL, pCurrTrig0, 3, &u0, 1);
                        theta0 = atan2 (trigPoint0.y, trigPoint0.x);
                        double f0 = bsiDConic4d_angleParameterToFraction (pConic0, theta0);
                        gpaPerpTan_saveLine (pContext,
                                                &pointOn0, source0, index0, f0, passCode0[pass],
                                                &pointOn1, source1, (int)bezier1.m_primitiveIndex, g1, passCode1[pass]
                                            );
                        }
                    }
                }
            }
        jmdlGraphicsPointArray_drop (pGPA);
        }
    return true;
    }

static bool        gpaPerpTan_processBezierDPoint4dBezierDPoint4dTagged
(
        GPAPerpTanContext       *pContext,
TaggedBezierDPoint4d &bezier0,
TaggedBezierDPoint4d &bezier1
)
    {
    int source0 = bezier0.m_pSource== pContext->pSource[0] ? 0 : 1;
    int source1 = 1 - source0;

    int passCode0[4] = {GPAPERPTAN_PERP, GPAPERPTAN_PERP, GPAPERPTAN_TANGENT, GPAPERPTAN_TANGENT};
    int passCode1[4] = {GPAPERPTAN_PERP, GPAPERPTAN_TANGENT, GPAPERPTAN_PERP, GPAPERPTAN_TANGENT};
    CurveCurveFunc func0, func1;
    GraphicsPointArrayP pGPA = jmdlGraphicsPointArray_grab ();
    DPoint4d pointOn0, pointOn1;
    int pass, k;
    //double tol = bsiTrig_smallAngle ();
    DPoint4d *pLocalPoleArray0 = (DPoint4d*)_alloca (bezier0.m_order * sizeof (DPoint4d));
    DPoint4d *pLocalPoleArray1 = (DPoint4d*)_alloca (bezier1.m_order * sizeof (DPoint4d));
    DMatrix4d worldToLocal;

    GraphicsPoint gp;

    /* Compute parameters in local coordinates only ... */
    bsiDMatrix4d_initForDPoint4dOrigin (&worldToLocal, NULL, &bezier0.m_poles[0]);
    bsiDMatrix4d_multiply4dPoints (&worldToLocal, pLocalPoleArray0, bezier0.m_poles, bezier0.m_order);
    bsiDMatrix4d_multiply4dPoints (&worldToLocal, pLocalPoleArray1, bezier1.m_poles, bezier1.m_order);

    for (pass = 0; pass < 4; pass++)
        {
        if      (   gpaPerpTan_checkEnable (pContext, source0, passCode0[pass])
             && gpaPerpTan_checkEnable (pContext, source1, passCode1[pass]))
            {
            func0 = passCode0[pass] == GPAPERPTAN_PERP
                ? evaluatePerpendicularAtCurve
                : evaluateTangentAtCurve;
            func1 = passCode1[pass] == GPAPERPTAN_PERP
                ? evaluatePerpendicularAtCurve
                : evaluateTangentAtCurve;

            jmdlGraphicsPointArray_empty (pGPA);
            gpa_approximateBezierBezierRoots
                        (
                        pContext,
                        pGPA,
                        pLocalPoleArray0, bezier0.m_order, func0,
                        pLocalPoleArray1, bezier1.m_order, func1,
                        NULL, pContext->workdim
                        );
            gpa_improveCurveCurveRoots
                        (
                        pGPA,
                        pLocalPoleArray0,       bezier0.m_order, func0,
                        pLocalPoleArray1,       bezier1.m_order, func1,
                        NULL, pContext->workdim
                        );

            /* And reevaluate output points on world curve */
            for (k = 0; jmdlGraphicsPointArray_getGraphicsPoint (pGPA, &gp, k); k++)
                {
                double u0 = gp.point.x;
                double u1 = gp.point.y;
                bsiBezierDPoint4d_evaluateArray (&pointOn0, NULL, bezier0.m_poles, bezier0.m_order, &u0, 1);
                bsiBezierDPoint4d_evaluateArray (&pointOn1, NULL, bezier1.m_poles, bezier1.m_order, &u1, 1);
                gpaPerpTan_saveLine (pContext,
                                    &pointOn0, source0, (int)bezier0.m_primitiveIndex, bezier0.LocalToGlobal (u0), passCode0[pass],
                                    &pointOn1, source1, (int)bezier1.m_primitiveIndex, bezier1.LocalToGlobal (u1), passCode1[pass]
                                );
                }
            }
        }
    jmdlGraphicsPointArray_drop (pGPA);
    return true;
    }

static bool        gpaPerpTan_processDSegment4dBezierDPoint4dTagged
(
        GPAPerpTanContext       *pContext,
GraphicsPointArrayCP pSource0,
        int                     index0,
const   DSegment4d              *pSegment0,
TaggedBezierDPoint4d            &bezier1
)
    {
    TaggedBezierDPoint4d bezier0 (pSource0, (size_t)index0, pSegment0->point, 2);
    return gpaPerpTan_processBezierDPoint4dBezierDPoint4dTagged
                (pContext, bezier0, bezier1);
    }

static bool        gpaPerpTan_processDSegment4dDConic4d_3d
(
        GPAPerpTanContext       *pContext,
GraphicsPointArrayCP pSource0,
        int                     index0,
const   DSegment4d              *pSegment0,
GraphicsPointArrayCP pSource1,
      int                       index1,
const   DConic4d                *pConic1
)
    {
    DEllipse3d ellipse;
    DSegment3d segment;
    DRay3d ray;
    int source0 = pSource0 == pContext->pSource[0] ? 0 : 1;
    int source1 = 1 - source0;
    if (    gpaPerpTan_checkEnable (pContext, 0, GPAPERPTAN_PERP)
        && !gpaPerpTan_checkEnable (pContext, 0, GPAPERPTAN_TANGENT)
        &&  gpaPerpTan_checkEnable (pContext, 1, GPAPERPTAN_PERP)
        && !gpaPerpTan_checkEnable (pContext, 1, GPAPERPTAN_TANGENT)
        && bsiDConic4d_isUnitWeighted (pConic1)
        && bsiDEllipse3d_initFromDConic4d (&ellipse, pConic1)
        && bsiDSegment4d_isUnitWeighted (pSegment0)
        && bsiDSegment3d_initFromDSegment4d (&segment, pSegment0)
        )
        {
        DPoint3d ellipsePoint[4];
        DPoint3d segmentPoint[4];
        double   ellipseAngle[4];
        double   segmentFraction[4];
        DPoint4d point0, point1;
        int i, numApproach;
        bsiDRay3d_initFromDSegment3d (&ray, &segment);
        numApproach = bsiDEllipse3d_closestApproachDRay3d (&ellipse,
                        ellipseAngle, segmentFraction, ellipsePoint, segmentPoint, &ray);
        for (i = 0; i < numApproach; i++)
            {
            double fraction1 = bsiDConic4d_angleParameterToFraction (pConic1, ellipseAngle[i]);
            double fraction0 = segmentFraction[i];
            bsiDPoint4d_initFromDPoint3dAndWeight (&point0, &segmentPoint[i], 1.0);
            bsiDPoint4d_initFromDPoint3dAndWeight (&point1, &ellipsePoint[i], 1.0);
            gpaPerpTan_saveLine (pContext,
                                        &point0, source0, index0, fraction0, GPAPERPTAN_PERP,
                                        &point1, source1, index1, fraction1, GPAPERPTAN_PERP
                                    );
            }
        return true;
        }
    else
        {
        TaggedBezierDPoint4d bezier0 (pSource0, (size_t)index0, pSegment0->point, 2);
        return gpaPerpTan_processDConic4dBezierDPoint4dTagged
                (pContext, pSource1, index1, pConic1, bezier0);
        }
    }

static bool        gpaPerpTan_processDSegment4dDSegment4d
(
        GPAPerpTanContext       *pContext,
GraphicsPointArrayCP pSource0,
        int                     index0,
const   DSegment4d              *pSegment0,
GraphicsPointArrayCP pSource1,
      int                       index1,
const   DSegment4d              *pSegment1
)
    {
    double param0, param1;
    DPoint4d point0, point1;

    int source0 = pSource0 == pContext->pSource[0] ? 0 : 1;
    int source1 = 1 - source0;

    /*
    ** In 2d, there are no perpendiculars or tangents between lines.
    ** In 2d, there are no tangent relationships.
    ** Only 3d perp-perp matters.
    */

    if (  pContext->workdim == 3
       && gpaPerpTan_checkEnable (pContext, source0, GPAPERPTAN_PERP)
       && gpaPerpTan_checkEnable (pContext, source1, GPAPERPTAN_PERP)
       )
        {
        if (bsiDSegment4d_closestApproach
                            (
                            &param0, &param1,
                            &point0, &point1,
                            pSegment0, pSegment1
                            )
            && gpaPerpTan_confirmParameterInBounds (pContext, source0, index0, param0, &point0)
            && gpaPerpTan_confirmParameterInBounds (pContext, source1, index1, param1, &point1)
            )
            {
            gpaPerpTan_saveLine (pContext,
                                    &point0, source0, index0, param0, GPAPERPTAN_PERP,
                                    &point1, source1, index1, param1, GPAPERPTAN_PERP
                                    );
            }
        }
    return true;
    }

Public void jmdlGraphicsPointArray_collectPrimitiveRanges
(
GraphicsPointArrayCP pSource,
bvector<DRange3dInt> &ranges
)
    {
    int curr0, curr1;
    int curveType;
    ranges.clear ();

    for (curr0 = curr1 = -1;
	jmdlGraphicsPointArray_parsePrimitiveAfter
		(pSource, &curr0, &curr1, NULL, NULL, &curveType, curr1);)
        {
        DRange3dInt pr;
        jmdlGraphicsPointArray_getPrimitiveRange (pSource, &pr.range, curr0);
        pr.index = curr0;
        ranges.push_back (pr);
        }
    }


/*---------------------------------------------------------------------------------**//**
* Examine primitives in pairs; compute all perpendicular and tangent joining segments.
* @param pCurves <= array of joining segments.
* @param pSource0 => first candidate source.
* @param bPerpEnable0 => true to search for perpendiculars from geometry 0.
* @param bTanEnable0  => true to search for tangents from geometry 0.
* @param pSource1 => second geometry source.
* @param bPerpEnable1 => true to search for perpendiculars from geometry 1.
* @param bTanEnable1  => true to search for tangents from geometry 1.
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_addXYPerpTanSegments
(
        GraphicsPointArrayP pCurves,
GraphicsPointArrayCP pSource0,
        bool                    bPerpEnable0,
        bool                    bTanEnable0,
GraphicsPointArrayCP pSource1,
        bool                    bPerpEnable1,
        bool                    bTanEnable1
)
    {
    GPAPerpTanContext context;
    memset (&context, 0, sizeof (context));
    context.pSource[GPAPERPTAN_SOURCE0] = pSource0;
    context.pSource[GPAPERPTAN_SOURCE1] = pSource1;

    context.bPerpEnable[0] = bPerpEnable0;
    context.bTanEnable[0] = bTanEnable0;

    context.bPerpEnable[1] = bPerpEnable1;
    context.bTanEnable[1] = bTanEnable1;

    context.pCurves   = pCurves;
    context.workdim   = 2;
    jmdlGraphicsPointArray_processAllPairs
                (
                &context,
                pSource0,
                pSource1,
                NULL,
                (GPAPairFunc_DSegment4dDConic4d)gpaPerpTan_processDSegment4dDConic4d,
                (GPAPairFunc_DSegment4dBezierDPoint4dTagged)gpaPerpTan_processDSegment4dBezierDPoint4dTagged,
                (GPAPairFunc_DConic4dDConic4d)gpaPerpTan_processDConic4dDConic4d,
                (GPAPairFunc_DConic4dBezierDPoint4dTagged)gpaPerpTan_processDConic4dBezierDPoint4dTagged,
                (GPAPairFunc_BezierDPoint4dBezierDPoint4dTagged)gpaPerpTan_processBezierDPoint4dBezierDPoint4dTagged
                );
    }



/*---------------------------------------------------------------------------------**//**
* Examine primitives in pairs; compute all perpendicular joining segments. (Full 3d)
* @param pCurves <= array of joining segments.
* @param pSource0 => first candidate source.
* @param pSource1 => second geometry source.
* @param extendLines => true to extend lines
* @param extendConics => true to extend conics
* @param workdim => 2 or 3
* @param bRemoveIntersections => true to remove exact intersection points.
* @param maxDistance => if nonzero, longer segments are ignored.
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_addPerpSegmentsExt4
(
        GraphicsPointArrayP pCurves,
GraphicsPointArrayCP pSource0,
GraphicsPointArrayCP pSource1,
        bool                    extendLines,
        bool                    extendConics,
        int                     workdim,
        bool                    bRemoveIntersections,
        double                  maxDistance
)
    {
    GPAPerpTanContext context;

    memset (&context, 0, sizeof (context));
    context.pSource[GPAPERPTAN_SOURCE0] = pSource0;
    context.pSource[GPAPERPTAN_SOURCE1] = pSource1;

    context.bPerpEnable[0] = true;
    context.bTanEnable[0]  = false;

    context.bPerpEnable[1] = true;
    context.bTanEnable[1]  = false;

    context.pCurves   = pCurves;
    context.workdim   = workdim;
    context.bRemoveIntersections = bRemoveIntersections;
    context.intersectionTolerance = maxDistance;
    if (bRemoveIntersections || maxDistance < 0.0)
        {
        DRange3d range;
        double dataSize;
        bsiDRange3d_init (&range);
        jmdlGraphicsPointArray_extendDRange3d (pSource0, &range);
        jmdlGraphicsPointArray_extendDRange3d (pSource1, &range);
        dataSize = bsiDRange3d_getLargestCoordinate (&range);
        maxDistance = context.intersectionTolerance = bsiTrig_smallAngle () * dataSize;
        }

    jmdlGraphicsPointArray_processPairsWithRangeIntersection
        (
        &context, pSource0, pSource1,
        workdim,
        maxDistance,
        (GPAPairFunc_DSegment4dDSegment4d)gpaPerpTan_processDSegment4dDSegment4d,
        (GPAPairFunc_DSegment4dDConic4d)gpaPerpTan_processDSegment4dDConic4d_3d,
        (GPAPairFunc_DSegment4dBezierDPoint4dTagged)gpaPerpTan_processDSegment4dBezierDPoint4dTagged,
        (GPAPairFunc_DConic4dDConic4d)gpaPerpTan_processDConic4dDConic4d,
        (GPAPairFunc_DConic4dBezierDPoint4dTagged)gpaPerpTan_processDConic4dBezierDPoint4dTagged,
        (GPAPairFunc_BezierDPoint4dBezierDPoint4dTagged)gpaPerpTan_processBezierDPoint4dBezierDPoint4dTagged
        );
    }

/*---------------------------------------------------------------------------------**//**
* Examine primitives in pairs; compute all perpendicular joining segments. (Full 3d)
* @param pCurves <= array of joining segments.
* @param pSource0 => first candidate source.
* @param pSource1 => second geometry source.
* @param extendLines => true to extend lines
* @param extendConics => true to extend conics
* @param workdim => 2 or 3
* @param bRemoveIntersections => true to remove exact intersection points.
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_addPerpSegmentsExt3
(
        GraphicsPointArrayP pCurves,
GraphicsPointArrayCP pSource0,
GraphicsPointArrayCP pSource1,
        bool                    extendLines,
        bool                    extendConics,
        int                     workdim,
        bool                    bRemoveIntersections
)
    {
    jmdlGraphicsPointArray_addPerpSegmentsExt4 (pCurves, pSource0, pSource1, extendLines, extendConics, 3, false, 0.0);
    }

/*---------------------------------------------------------------------------------**//**
* Examine primitives in pairs; compute all perpendicular joining segments. (Full 3d)
* @param pCurves <= array of joining segments.
* @param pSource0 => first candidate source.
* @param pSource1 => second geometry source.
* @param extendLines => true to extend lines
* @param extendConics => true to extend conics
* @param workdim => 2 or 3
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_addPerpSegmentsExt2
(
        GraphicsPointArrayP pCurves,
GraphicsPointArrayCP pSource0,
GraphicsPointArrayCP pSource1,
        bool                    extendLines,
        bool                    extendConics,
        int                     workdim
)
    {
    jmdlGraphicsPointArray_addPerpSegmentsExt3 (pCurves, pSource0, pSource1, extendLines, extendConics, 3, false);
    }



/*---------------------------------------------------------------------------------**//**
* Examine primitives in pairs; compute all perpendicular joining segments. (Full 3d)
* @param pCurves <= array of joining segments.
* @param pSource0 => first candidate source.
* @param pSource1 => second geometry source.
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_addPerpSegmentsExt
(
        GraphicsPointArrayP pCurves,
GraphicsPointArrayCP pSource0,
GraphicsPointArrayCP pSource1,
        bool                    extendLines,
        bool                    extendConics
)
    {
    jmdlGraphicsPointArray_addPerpSegmentsExt2 (pCurves, pSource0, pSource1, extendLines, extendConics, 3);
    }


/*---------------------------------------------------------------------------------**//**
* Examine primitives in pairs; compute all perpendicular joining segments. (Full 3d)
* @param pCurves <= array of joining segments.
* @param pSource0 => first candidate source.
* @param pSource1 => second geometry source.
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_addPerpSegments
(
        GraphicsPointArrayP pCurves,
GraphicsPointArrayCP pSource0,
GraphicsPointArrayCP pSource1
)
    {
    jmdlGraphicsPointArray_addPerpSegmentsExt
                (pCurves, pSource0, pSource1, false, false);
    }


/*---------------------------------------------------------------------------------**//**
* Examine primitives in pairs; compute all perpendicular and tangent joining segments.
* Return points as (point, index, parameter) values in respective output arrays.
* @param pPointCollector0 => array to receive points from source0.
* @param pPointCollector1 => array to receive points from source1.
* @param pSource0 => first candidate source.
* @param bPerpEnable0 => true to search for perpendiculars from geometry 0.
* @param bTanEnable0  => true to search for tangents from geometry 0.
* @param pSource1 => second geometry source.
* @param bPerpEnable1 => true to search for perpendiculars from geometry 1.
* @param bTanEnable1  => true to search for tangents from geometry 1.
*
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_addXYPerpTanPoints
(
        GraphicsPointArrayP pPointCollector0,
        GraphicsPointArrayP pPointCollector1,
GraphicsPointArrayCP pSource0,
        bool                    bPerpEnable0,
        bool                    bTanEnable0,
GraphicsPointArrayCP pSource1,
        bool                    bPerpEnable1,
        bool                    bTanEnable1
)
    {
    GPAPerpTanContext context;
    memset (&context, 0, sizeof (context));
    context.pSource[GPAPERPTAN_SOURCE0] = pSource0;
    context.pSource[GPAPERPTAN_SOURCE1] = pSource1;

    context.bPerpEnable[0] = bPerpEnable0;
    context.bTanEnable[0] = bTanEnable0;

    context.bPerpEnable[1] = bPerpEnable1;
    context.bTanEnable[1] = bTanEnable1;

    context.pPointCollector0   = pPointCollector0;
    context.pPointCollector1   = pPointCollector1;
    context.workdim = 2;
    jmdlGraphicsPointArray_processAllPairs
                (
                &context,
                pSource0,
                pSource1,
                NULL,
                (GPAPairFunc_DSegment4dDConic4d)gpaPerpTan_processDSegment4dDConic4d,
                (GPAPairFunc_DSegment4dBezierDPoint4dTagged)gpaPerpTan_processDSegment4dBezierDPoint4dTagged,
                (GPAPairFunc_DConic4dDConic4d)gpaPerpTan_processDConic4dDConic4d,
                (GPAPairFunc_DConic4dBezierDPoint4dTagged)gpaPerpTan_processDConic4dBezierDPoint4dTagged,
                (GPAPairFunc_BezierDPoint4dBezierDPoint4dTagged)gpaPerpTan_processBezierDPoint4dBezierDPoint4dTagged
                );
    }



typedef struct
    {
    DPoint3d point0;
    DPoint3d point1;
    GraphicsPointArrayP pTangentPointCollector;
    GraphicsPointArrayP pFullCircleCollector;
    DPoint3d midPoint;
    DPoint4d midPoint4d;
    DPoint3d unitPerpFromMidPoint;
    double aa;          /* square of half the distance from point0 to point1 */
    bool        extendLines;
    bool        extendArcs;
    } PointPointTangentContext;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    recordPointPointCurve
(
PointPointTangentContext *pContext,
const DPoint3d                  *pTangentPoint,
GraphicsPointArrayCP pSource,
int                             index,
double                          fraction,
bool                            checkFraction
)
    {
    DEllipse3d ellipse;
    static double s_fractionTol = 1.0e-8;
    if (fraction < -s_fractionTol || fraction > 1.0 + s_fractionTol)
        return;

    bsiDEllipse3d_initFrom3DPoint3dOnArc
                (
                &ellipse,
                pTangentPoint,
                &pContext->point0,
                &pContext->point1
                );
    bsiDEllipse3d_makeFullSweep (&ellipse);
    if (pContext->pFullCircleCollector)
        jmdlGraphicsPointArray_addDEllipse3d (pContext->pFullCircleCollector, &ellipse);
    if (pContext->pTangentPointCollector)
        {
        jmdlGraphicsPointArray_addComplete (pContext->pTangentPointCollector,
                    pTangentPoint->x, pTangentPoint->y, pTangentPoint->z, 1.0,
                    fraction,0, index);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool                    cbPointPointBezierDPoint4d
(
        PointPointTangentContext    *pContext,
        TaggedBezierDPoint4d        &bezier
)
    {
    double delta, u0, u1, u;
    int numStepPerPole = 4;
    int i;
    int numStep;
    double   df0, df1;
    double   f0, f1;
    DPoint3d X0, X1;
    DPoint3d T0, T1;
    DPoint3d K0, K1;
    int k;
    int s_maxIterations = 10;

    double absTol = 1.0e-10;

    numStep = numStepPerPole * bezier.m_order;
    delta = 1.0 / numStep;

    for (i = 0; i < numStep; i++)
        {
        u0 = i * delta;
        u1 = (i + 1) * delta;
        bsiBezierDPoint4d_evaluateDPoint3dArrayExt (&X0, &T0, &K0, bezier.m_poles, bezier.m_order, &u0, 1);
        bsiBezierDPoint4d_evaluateDPoint3dArrayExt (&X1, &T1, &K1, bezier.m_poles, bezier.m_order, &u1, 1);

        tangentFunctions_pointPointCurve (&f0, &df0,
                    &pContext->point0,
                    &pContext->point1,
                    &X0, &T0, &K0);
        tangentFunctions_pointPointCurve (&f1, &df1,
                    &pContext->point0,
                    &pContext->point1,
                    &X1, &T1, &K1);
        if (f0 == 0.0)
            {
            recordPointPointCurve (pContext, &X0, bezier.m_pSource, (int)bezier.m_primitiveIndex, bezier.LocalToGlobal (u0), false);
            }
        else if (i == numStep - 1 && f1 == 0.0)
            {
            recordPointPointCurve (pContext, &X0, bezier.m_pSource, (int)bezier.m_primitiveIndex, bezier.LocalToGlobal (u1), false);
            }
        else
            {
            if (f0 * f1 < 0.0)
                {
                double a0 = u0;
                double a1 = u1;
                bool    converged;
                u = u0 - (u1 - u0) * f0 / (f1 - f0);
                for (k = 0; k < s_maxIterations; k++)
                    {
                    bsiBezierDPoint4d_evaluateDPoint3dArrayExt (&X0, &T0, &K0, bezier.m_poles, bezier.m_order, &u, 1);
                    tangentFunctions_pointPointCurve (&f0, &df0,
                                &pContext->point0,
                                &pContext->point1,
                                &X0, &T0, &K0);
                    if (!newtonStep (&u, &a0, &a1, &converged, f0, df0, absTol))
                        break;
                    if (converged)
                        {
                        bsiBezierDPoint4d_evaluateDPoint3dArrayExt (&X1, &T1, &K1, bezier.m_poles, bezier.m_order, &u, 1);
                        recordPointPointCurve (pContext, &X0, bezier.m_pSource, (int)bezier.m_primitiveIndex, bezier.LocalToGlobal (u), false);
                        k = s_maxIterations;  /* Force loop exit */
                        }
                    }
                }
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool                    cbPointPointDConic4d
(
        PointPointTangentContext    *pContext,
GraphicsPointArrayCP pSource,
        int                         index,
        DConic4d                    *pConic
)
    {
    double dtheta, theta0, theta1, theta;
    int numFullStep = 12;
    int i;
    int numStep;
    double   df0, df1;
    double   f0, f1;
    DPoint3d X0, X1;
    DPoint3d T0, T1;
    DPoint3d K0, K1;
    int k;
    int s_maxIterations = 10;
    bool    isFullArcSearch = pContext->extendArcs || bsiDConic4d_isFullSweep (pConic);
    double absTol = bsiTrig_smallAngle ();

    if (isFullArcSearch)
        {
        numStep = numFullStep;
        dtheta = msGeomConst_2pi / numStep;
        }
    else
        {
        numStep = (int) (1 + numFullStep * pConic->sweep / msGeomConst_2pi);
        if (numStep < 2)
            numStep = 2;
        dtheta = pConic->sweep / numStep;
        }

    for (i = 0; i < numStep; i++)
        {
        theta0 = pConic->start + i * dtheta;
        theta1 = pConic->start + (i + 1) * dtheta;
        bsiDConic4d_angleParameterToDPoint3dDerivatives (pConic, &X0, &T0, &K0, theta0);
        bsiDConic4d_angleParameterToDPoint3dDerivatives (pConic, &X1, &T1, &K1, theta1);
        tangentFunctions_pointPointCurve (&f0, &df0,
                    &pContext->point0,
                    &pContext->point1,
                    &X0, &T0, &K0);
        tangentFunctions_pointPointCurve (&f1, &df1,
                    &pContext->point0,
                    &pContext->point1,
                    &X1, &T1, &K1);
        if (f0 == 0.0)
            {
            recordPointPointCurve (pContext, &X0, pSource, index,
                        bsiDConic4d_angleParameterToFraction (pConic, theta0),
                        pContext->extendArcs);
            }
        else if (i == numStep - 1 && f1 == 0.0)
            {
            recordPointPointCurve (pContext, &X1, pSource, index,
                        bsiDConic4d_angleParameterToFraction (pConic, theta1),
                        pContext->extendArcs);
            }
        else
            {
            if (f0 * f1 < 0.0)
                {
                double a0 = theta0;
                double a1 = theta1;
                bool    converged;
                theta = theta0 - (theta1 - theta0) * f0 / (f1 - f0);
                for (k = 0; k < s_maxIterations; k++)
                    {
                    bsiDConic4d_angleParameterToDPoint3dDerivatives (pConic, &X0, &T0, &K0, theta);
                    tangentFunctions_pointPointCurve (&f0, &df0,
                                &pContext->point0,
                                &pContext->point1,
                                &X0, &T0, &K0);
                    if (!newtonStep (&theta, &a0, &a1, &converged, f0, df0, absTol))
                        break;
                    if (converged)
                        {
                        bsiDConic4d_angleParameterToDPoint3dDerivatives (pConic, &X0, NULL, NULL, theta);
                        recordPointPointCurve (pContext, &X0, pSource, index,
                                bsiDConic4d_angleParameterToFraction (pConic, theta),
                                pContext->extendArcs);
                        k = s_maxIterations;  /* Force loop exit */
                        }
                    }
                }
            }
        }

    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool                    cbPointPointDSegment4d
(
        PointPointTangentContext    *pContext,
GraphicsPointArrayCP pSource,
        int                         index,
        DSegment4d                  *pSegment
)
    {
    DPoint3d segmentTangent;
    double uu[2];
    int numu, i;
    DPoint3d segmentPerp;
    DPoint4d closestPoint4d;
    DPoint3d closestPoint3d;
    DPoint3d center;
    DPoint4d center4d;
    DPoint3d tangency3d;
    DPoint3d refVector;
    DEllipse3d ellipse;
    DPoint4d tangency4d;
    double param;
    DVec3d vector0, vector90;
    double a, b, c;
    double   dhdu;
    double  h0;

    bsiDSegment4d_pseudoTangent (pSegment, &segmentTangent);
    bsiDPoint3d_unitPerpendicularXY (&segmentPerp, &segmentTangent);
    bsiDSegment4d_projectDPoint4dCartesianXYW (pSegment, &closestPoint4d, NULL, &pContext->midPoint4d);
    bsiDPoint4d_normalize (&closestPoint4d, &closestPoint3d);
    bsiDPoint3d_subtractDPoint3dDPoint3d
                    (
                    &refVector,
                    &pContext->midPoint,
                    &closestPoint3d
                    );
    h0 = bsiDPoint3d_dotProductXY (&segmentPerp, &refVector);
    dhdu = bsiDPoint3d_dotProductXY
                        (
                        &segmentPerp,
                        &pContext->unitPerpFromMidPoint
                        );
    /* To solve: u^2 = (h0 + dhdu * u)^2 */

    a = 1.0 - dhdu * dhdu;
    b = - 2.0 * h0 * dhdu;
    c = -h0 * h0 + pContext->aa;

    numu = bsiMath_solveQuadratic (uu, a, b, c);
    for (i = 0; i < numu; i++)
        {
        bsiDPoint3d_addScaledDPoint3d (&center, &pContext->midPoint, &pContext->unitPerpFromMidPoint, uu[i]);
        bsiDPoint3d_subtractDPoint3dDPoint3d (&vector0, &pContext->point0, &center);
        vector0.z = 0.0;
        bsiDPoint3d_setXYZ (&vector90, -vector0.y, vector0.x, 0.0);
        bsiDEllipse3d_initFrom3dVectors (&ellipse, &center, &vector0, &vector90, 0.0, msGeomConst_2pi);
        if (pContext->pFullCircleCollector)
            jmdlGraphicsPointArray_addDEllipse3d (pContext->pFullCircleCollector, &ellipse);
        if (pContext->pTangentPointCollector)
            {
            bsiDPoint4d_initFromDPoint3dAndWeight (&center4d, &center, 1.0);
            bsiDSegment4d_projectDPoint4dCartesianXYW
                        (pSegment, &tangency4d, &param, &center4d);
            if (bsiDPoint4d_normalize (&tangency4d, &tangency3d))
                {
                recordPointPointCurve (pContext, &tangency3d, pSource, index, param,
                            pContext->extendLines);
                }
            }
        }
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* Construct xy circles passing through two given points and tangent to
* geometry in the GPA.   Return tangency points and/or full circles as requested.
*
* @param pTangentPointCollector => array to receive tangency points, with parameters
*               on primitives in pSource.
* @param pFullCircleCollector => array to receive full circles constructed through the
*               2 points and tangency.
* @param pSource => array of source geometry.
* @param pPoint0 => first fixed point
* @param pPoint1 => second fixed point
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_addXYCirclesFromPointPointTangent
(
        GraphicsPointArrayP pTangentPointCollector,
        GraphicsPointArrayP pFullCircleCollector,
GraphicsPointArrayCP pSource,
const   DPoint3d                *pPoint0,
const   DPoint3d                *pPoint1,
        bool                    extendLines,
        bool                    extendArcs
)
    {
    PointPointTangentContext context;
    DPoint3d vector01;

    context.point0 = *pPoint0;
    context.point1 = *pPoint1;
    context.pTangentPointCollector = pTangentPointCollector;
    context.pFullCircleCollector   = pFullCircleCollector;
    context.aa = 0.25 * bsiDPoint3d_distanceSquaredXY (pPoint0, pPoint1);
    context.extendLines = extendLines;
    context.extendArcs  = extendArcs;

    bsiDPoint3d_interpolate (&context.midPoint, pPoint0, 0.5, pPoint1);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vector01, pPoint1, pPoint0);
    bsiDPoint3d_unitPerpendicularXY (&context.unitPerpFromMidPoint, &vector01);
    bsiDPoint4d_initFromDPoint3dAndWeight (&context.midPoint4d, &context.midPoint, 1.0);
    jmdlGraphicsPointArray_processPrimitives
                (
                &context,
                pSource,
                (GPAFunc_DSegment4d)cbPointPointDSegment4d,
                (GPAFunc_DConic4d)cbPointPointDConic4d,
                (GPAFunc_BezierDPoint4dTagged)cbPointPointBezierDPoint4d
                );
    }


/*---------------------------------------------------------------------------------**//**
* Construct xy circles passing through given point and tangent to two curves.
*
* @param pTangentPointCollector0 => array to receive tangency points, with parameters
*               on primitives in pSource0
* @param pTangentPointCollector1 => array to receive tangency points, with parameters
*               on primitives in pSource1
* @param pFullCircleCollector => array to receive full circles constructed through the
*               2 points and tangency.
* @param pSource0 => array of source geometry.
* @param pSource1 => array of source geometry.
* @param pPoint   => fixed point
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_addXYCirclesFromPointTangentTangent
(
        GraphicsPointArrayP pTangentPointCollector0,
        GraphicsPointArrayP pTangentPointCollector1,
        GraphicsPointArrayP pFullCircleCollector,
GraphicsPointArrayCP pSource0,
GraphicsPointArrayCP pSource1,
const   DPoint3d                *pPoint
)
    {
    PointCurveCurveContext context;

    context.pTangentDest[0] = pTangentPointCollector0;
    context.pTangentDest[1] = pTangentPointCollector1;
    context.pCircleDest   = pFullCircleCollector;
    context.point2 = *pPoint;

    jmdlGraphicsPointArray_processAllIndexedPairs
                (
                &context,
                pSource0,
                pSource1,
                (GPAPairFunc_IndexIndex)gpa_cbapproximateGPAGPACurveCurvePointRoots
                );
    }

/*---------------------------------------------------------------------------------**//**
* Construct xy circles tangent to 3 curves.
*
* @param pTangentPointCollector0 => array to receive tangency points, with parameters on primitives in pSource0
* @param pTangentPointCollector1 => array to receive tangency points, with parameters on primitives in pSource1
* @param pTangentPointCollector2 => array to receive tangency points, with parameters on primitives in pSource2
* @param pFullCircleCollector => array to receive full circles constructed through the 3 tangency points.
* @param pSource0 => array of source geometry.
* @param pSource1 => array of source geometry.
* @param pSource2 => array of source geometry.
* @param numSamples => how many parameter space samples to use in computing initial root approximations
* @bsimethod                                                    DavidAssaf      07/03
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_addXYCirclesFromTangentTangentTangentExt
(
        GraphicsPointArrayP pTangentPointCollector0,
        GraphicsPointArrayP pTangentPointCollector1,
        GraphicsPointArrayP pTangentPointCollector2,
        GraphicsPointArrayP pFullCircleCollector,
GraphicsPointArrayCP pSource0,
GraphicsPointArrayCP pSource1,
GraphicsPointArrayCP pSource2,
        int                     numSamples
)
    {
    CurveCurveCurveContext context (pSource0, pSource1, pSource2, numSamples,

                pTangentPointCollector0, pTangentPointCollector1, pTangentPointCollector2,
                pFullCircleCollector);


    jmdlGraphicsPointArray_processAllIndexedTriples
                (
                &context,
                pSource0,
                pSource1,
                pSource2,
                (GPATripleFunc_IndexIndexIndex)gpa_cbapproximateGPAGPACurveCurveCurveRoots
                );
    }


/*---------------------------------------------------------------------------------**//**
* Construct xy circles tangent to 3 curves.
*
* @param pTangentPointCollector0 => array to receive tangency points, with parameters
*               on primitives in pSource0
* @param pTangentPointCollector1 => array to receive tangency points, with parameters
*               on primitives in pSource1
* @param pTangentPointCollector2 => array to receive tangency points, with parameters
*               on primitives in pSource2
* @param pFullCircleCollector => array to receive full circles constructed through the
*               3 tangency points.
* @param pSource0 => array of source geometry.
* @param pSource1 => array of source geometry.
* @param pSource2 => array of source geometry.
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_addXYCirclesFromTangentTangentTangent
(
        GraphicsPointArrayP pTangentPointCollector0,
        GraphicsPointArrayP pTangentPointCollector1,
        GraphicsPointArrayP pTangentPointCollector2,
        GraphicsPointArrayP pFullCircleCollector,
GraphicsPointArrayCP pSource0,
GraphicsPointArrayCP pSource1,
GraphicsPointArrayCP pSource2
)
    {
    jmdlGraphicsPointArray_addXYCirclesFromTangentTangentTangentExt (pTangentPointCollector0, pTangentPointCollector1, pTangentPointCollector2,
                                                                     pFullCircleCollector, pSource0, pSource1, pSource2, 10);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/02
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    projectedDistance
(
double  *pDistance,
const DPoint4d *pHPoint0,
const DPoint4d *pHPoint1,
int   workdim
)
    {
    DPoint3d point0, point1;
    DPoint3d vector01;
    double d2;

    if (  bsiDPoint4d_normalize (pHPoint0, &point0)
       && bsiDPoint4d_normalize (pHPoint1, &point1)
       )
        {
        bsiDPoint3d_subtractDPoint3dDPoint3d (&vector01, &point1, &point0);
        d2 = vector01.x * vector01.x;
        if (workdim > 1)
            d2 += vector01.y * vector01.y;
        if (workdim > 2)
            d2 += vector01.z * vector01.z;
        *pDistance = sqrt (d2);
        return true;
        }
    *pDistance = 0.0;
    return false;
    }

#ifdef NOISY_OPTION
static int s_noisy = 0;
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/02
+---------------+---------------+---------------+---------------+---------------+------*/
static void recordPair
(
GraphicsPointArrayP pGPA1,
GraphicsPointArrayP pGPA2,
GraphicsPoint *pGP1,
GraphicsPoint *pGP2,
char const *pDescription
)
    {
    DPoint3d point1, point2;

    jmdlGraphicsPointArray_addGraphicsPoint (pGPA1, pGP1);
    jmdlGraphicsPointArray_addGraphicsPoint (pGPA2, pGP2);

    bsiDPoint4d_normalize (&pGP1->point, &point1);
    bsiDPoint4d_normalize (&pGP2->point, &point2);

#ifdef NOISY_OPTION
    if (s_noisy)
        {
        printf ("%s\n", pDescription);
        printf ("    (%d,%17.13lf) (%le,%le,%le)\n",
                        pGP1->userData, pGP1->a,
                        point1.x, point1.y, point1.z);
        printf ("    (%d,%17.13lf) (%le,%le,%le)\n",
                        pGP2->userData, pGP2->a,
                        point2.x, point2.y, point2.z);
        }
#endif
    }

static void pullPairForward
(
GraphicsPointArrayP pArrayA,
GraphicsPointArrayP pArrayB,
int &numAccepted,
int index
)
    {
    if (index >= numAccepted)
        {
        if (index > numAccepted)
            {
            GraphicsPoint gpA, gpB;
            jmdlGraphicsPointArray_getGraphicsPoint (pArrayA, &gpA, index);
            jmdlGraphicsPointArray_getGraphicsPoint (pArrayB, &gpB, index);
            jmdlGraphicsPointArray_setGraphicsPoint (pArrayA, &gpA, numAccepted);
            jmdlGraphicsPointArray_setGraphicsPoint (pArrayB, &gpB, numAccepted);
            }
        numAccepted++;
        }
    }
/*----------------------------------------------------------------------+
* Find (multiple) points at which two elements approach within
*   a tolerance.  This considers all combinations of interior-interior, interior-endpoint,
*   and endpoint-endpoint.  For workdim==2 it also considers xy intersections.
* @remarks Originally minDistance_collectProjectedApproachPoints; moved here
*       so it can be called from msbspline.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void jmdlGraphicsPointArray_collectProjectedApproachPointsExt
(
GraphicsPointArrayP pSegmentGPA1,
GraphicsPointArrayP pSegmentGPA2,
GraphicsPointArrayP pGPA1,
GraphicsPointArrayP pGPA2,
bool            extend,
double          maxDist,
int             workdim,
double          duplicatePointTolerance
)
    {
    GraphicsPointArrayP pWorkArray1 = jmdlGraphicsPointArray_grab ();
    GraphicsPointArrayP pWorkArray2 = jmdlGraphicsPointArray_grab ();
    GraphicsPointArrayP pInteriorPointArray = jmdlGraphicsPointArray_grab ();
    GraphicsPointArrayP pDiscontinuityPointArray = NULL;
    GraphicsPointArrayP pCurveGeometryArray = NULL;
    GraphicsPointArrayP pCurveOutputArray = NULL;
    GraphicsPointArrayP pPointArray = NULL;
    int pass;

    int i, j;
    GraphicsPoint gp1, gp2;
    double currDist;
    double relTol = bsiTrig_smallAngle ();
    static double angleTol = 1.0e-8;
    static double s_defaultRelMaxDist = 1.0e-10;

    if (maxDist <= 0.0)
        {
        /* generate a machine tolerance for closest approach */
        DRange3d range;
        double LargestCoordinate;
        bsiDRange3d_init (&range);
        jmdlGraphicsPointArray_extendDRange3d (pGPA1, &range);
        jmdlGraphicsPointArray_extendDRange3d (pGPA2, &range);
        LargestCoordinate = bsiDRange3d_getLargestCoordinate (&range);
        maxDist = s_defaultRelMaxDist * LargestCoordinate;
        }

    jmdlGraphicsPointArray_addPerpSegmentsExt4 (pWorkArray1, pGPA1, pGPA2, extend, extend, workdim, false, maxDist);

    for (i = 0;
            jmdlGraphicsPointArray_getGraphicsPoint (pWorkArray1, &gp1, i)
        &&  jmdlGraphicsPointArray_getGraphicsPoint (pWorkArray1, &gp2, i + 1);
        i += 2)
        {
        if (  projectedDistance (&currDist, &gp1.point, &gp2.point, workdim)
           && currDist < maxDist
           )
            {
            recordPair (pSegmentGPA1, pSegmentGPA2, &gp1, &gp2, "II");
            }
        }

    /* Pass 0 -- distances from discontinuities of pGPA1 to any point of pGPA2,
       Pass 1 -- switched
    */
    jmdlGraphicsPointArray_empty (pWorkArray1);
    jmdlGraphicsPointArray_addDiscontinuityPoints
            (pWorkArray1, pGPA1, NULL, 0.0, workdim, -relTol, angleTol);

    jmdlGraphicsPointArray_empty (pWorkArray2);
    jmdlGraphicsPointArray_addDiscontinuityPoints
            (pWorkArray2, pGPA2, NULL, 0.0, workdim, -relTol, angleTol);

    for (pass = 0; pass < 2; pass++)
        {
        if (pass == 0)
            {
            pCurveOutputArray = pSegmentGPA1;
            pCurveGeometryArray = pGPA1;
            pPointArray = pSegmentGPA2;
            pDiscontinuityPointArray = pWorkArray2;
            }
        else
            {
            pCurveOutputArray = pSegmentGPA2;
            pCurveGeometryArray = pGPA2;
            pPointArray = pSegmentGPA1;
            pDiscontinuityPointArray = pWorkArray1;
            }


        for (i = 0; jmdlGraphicsPointArray_getGraphicsPoint  (pDiscontinuityPointArray, &gp1, i); i++)
            {
            jmdlGraphicsPointArray_empty (pInteriorPointArray);
            jmdlGraphicsPointArray_addPerpendicularsFromDPoint4dExt
                        (pInteriorPointArray, pCurveGeometryArray, &gp1.point, workdim, extend);
            for (j = 0; jmdlGraphicsPointArray_getGraphicsPoint (pInteriorPointArray, &gp2, j); j++)
                {
                if (  projectedDistance (&currDist, &gp1.point, &gp2.point, workdim)
                   && currDist < maxDist
                   )
                    {
                    recordPair (pPointArray, pCurveOutputArray, &gp1, &gp2, pass == 0 ? "ID" : "DI");
                    }
                }
            }
        }

    for (i = 0; jmdlGraphicsPointArray_getGraphicsPoint  (pWorkArray1, &gp1, i); i++)
        {
        for (j = 0; jmdlGraphicsPointArray_getGraphicsPoint (pWorkArray2, &gp2, j); j++)
            {
            if (  projectedDistance (&currDist, &gp1.point, &gp2.point, workdim)
               && currDist < maxDist
               )
                {
                recordPair (pSegmentGPA1, pSegmentGPA2, &gp1, &gp2, "DD");
                }
            }
        }

    /* And finally xy intersections */
    if (workdim == 2)
        {
        jmdlGraphicsPointArray_xyIntersectionPointsExt
                        (
                        pWorkArray1, pWorkArray2,
                        pGPA1, pGPA2,
                        extend, extend
                        );
        for (i = 0;
               jmdlGraphicsPointArray_getGraphicsPoint  (pWorkArray1, &gp1, i)
            && jmdlGraphicsPointArray_getGraphicsPoint  (pWorkArray2, &gp2, i);
            i++
            )
            {
            recordPair (pSegmentGPA1, pSegmentGPA2, &gp1, &gp2, "XX");
            }
        }

    // Sort along first array ...
    jmdlGraphicsPointArray_installSortIndex (pSegmentGPA1);
    jmdlGraphicsPointArray_sortByUserDataAndA (pSegmentGPA1);

    if (duplicatePointTolerance > 0
        && jmdlGraphicsPointArray_shuffleBySortIndex (pSegmentGPA2, pSegmentGPA1))
        {
        int baseCount = jmdlGraphicsPointArray_getCount (pSegmentGPA1);
        int newCount, iA, iB;
        for (newCount = 0, iA = 0;
                iA < baseCount;
                iA = iB
                )
            {
            GraphicsPoint gp1A, gp1B;
            jmdlGraphicsPointArray_getGraphicsPoint (pSegmentGPA1, &gp1A, iA);
            // accept point iA ...
            pullPairForward (pSegmentGPA1, pSegmentGPA2, newCount, iA);
            // skip over immediate successors within tolerance ....
            for (iB = iA + 1; iB < baseCount; iB++)
                {
                jmdlGraphicsPointArray_getGraphicsPoint (pSegmentGPA1, &gp1B, iB);
                if (gp1A.point.RealDistance (gp1B.point) > duplicatePointTolerance)
                    break;
                }
            }
        jmdlGraphicsPointArray_trim (pSegmentGPA1, newCount);
        jmdlGraphicsPointArray_trim (pSegmentGPA2, newCount);
        }

    jmdlGraphicsPointArray_drop (pInteriorPointArray);
    jmdlGraphicsPointArray_drop (pWorkArray2);
    jmdlGraphicsPointArray_drop (pWorkArray1);
    }

/*----------------------------------------------------------------------+
* Find (multiple) points at which two elements approach within
*   a tolerance.  This considers all combinations of interior-interior, interior-endpoint,
*   and endpoint-endpoint.  For workdim==2 it also considers xy intersections.
* @remarks Originally minDistance_collectProjectedApproachPoints; moved here
*       so it can be called from msbspline.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void jmdlGraphicsPointArray_collectProjectedApproachPoints
(
GraphicsPointArrayP pSegmentGPA1,
GraphicsPointArrayP pSegmentGPA2,
GraphicsPointArrayP pGPA1,
GraphicsPointArrayP pGPA2,
bool            extend,
double          maxDist,
int             workdim
)
    {
    jmdlGraphicsPointArray_collectProjectedApproachPointsExt (pSegmentGPA1, pSegmentGPA2, pGPA1, pGPA2, extend, maxDist, workdim, 0.0);
    }
END_BENTLEY_GEOMETRY_NAMESPACE