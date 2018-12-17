/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/qsic.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/GeomPrivateApi.h>
#include <math.h>
#include <stdlib.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

extern void bsiDoubleArray_sort (double *, int, int);

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
#define MAX_TOROIDAL_POINT 400
#define MINIMUM_ANGLE_STEP 0.05

#define GRID_SAMPLE_POINT       0
#define CRITICAL_POINT          1
#define BOUNDARY_CROSSING_POINT 2
#define MID_RANGE_POINT         3
#define SILHOUETTE_CROSSING_POINT 4
#define SUBDIVISION_POINT  5
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

typedef struct
    {
    const RotatedConic *pSurface0;
    const RotatedConic *pSurface1;
    DPoint4d sigma0;
    DPoint4d sigma1;
    DMap4d hMap0to1;
    DMap4d hMap1to0;
    double tolerance;
    SilhouetteArrayHandler handlerFunc;
    void            *pUserData;
    } QSIC_Params;

/* Context data for the overall intersection of a cylinder and a quadric */
typedef struct
    {
    RotMatrix matrixR;
    DPoint3d  vectorA;
    double    alpha;
    } QSIC_Coefficients;

/* Context data for the intersection of a single cylinder ray with a quadric. */
typedef struct
    {
    double theta;       /* Coordinates on master cylinder */
    double c, s;
    double FTA;                 /* F^T * a */
    double FTRF;                /* F^T * R * F */
    int typecode;
    } QSIC_Ray;

#define QSIC_MAX_RAY 120

typedef struct
    {
    int numRay;
    QSIC_Ray ray[QSIC_MAX_RAY];
    } QSIC_RayArray;


typedef void (*QSIC_ArrayHandler)
    (
    QSIC_Ray *pRay,
    int numRay,
    const RotatedConic *pSurface0,
    const RotatedConic *pSurface1,
    const RotatedConic_Tree *pTree0,
    const RotatedConic_Tree *pTree1,
    const QSIC_Coefficients *pCoffs,
    void *pUserData
    );

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlRotatedConic_discriminantMatrix                     |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
| Compute the discriminant matrix R = a*a^t - alpha * Q                 |
+----------------------------------------------------------------------*/
static void    jmdlQSIC_discriminantMatrix

(
RotMatrixP pR,
RotMatrixCP pQ,
DPoint3dCP pA,
double  alpha
)
    {
    int j;
    double aj;

    for (j = 0; j < 3; j++)
        {
        aj = bsiDPoint3d_getComponent (pA, j);
        pR->form3d[0][j] = pA->x * aj - alpha * pQ->form3d[0][j];
        pR->form3d[1][j] = pA->y * aj - alpha * pQ->form3d[1][j];
        pR->form3d[2][j] = pA->z * aj - alpha * pQ->form3d[2][j];
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlRotatedConic_shiftedRank1Matrix_case00              |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
| Shifted rank 1 vector computation for special case where a02=a21=0,   |
| and a01 not zero.                                                     |
+----------------------------------------------------------------------*/
static StatusInt jmdlRotatedConic_shiftedRank1Matrix_case00

(
double *pH0,
double *pH1,
double *pH2,
double *pLambda,
double a00,
double a11,
double a22,
double a01,
double d0,
double d1,
double d2,
double abstol
)
    {
    double b00, b11, diagErr;
    StatusInt status = ERROR;
    *pH2 = 0.0;     /* Because both affected offdiagonals are zero */
    *pLambda = -a22 / d2;   /* Because pH2=0 */
    b00 = a00 + *pLambda * d0;
    b11 = a11 + *pLambda * d1;
    if (b00 > 0.0 && b00 > b11)
        {
        *pH0 = sqrt (b00);
        *pH1 = a01 / *pH0;
        diagErr = b11 - (*pH1) * (*pH1);

        if (fabs(diagErr) < abstol)
            status = SUCCESS;

        }
    else if (b11 > 0.0 && b11 > b00)
        {
        *pH1 = sqrt (b11);
        *pH0 = a01 / *pH1;
        diagErr = b11 - (*pH0) * (*pH0);

        if (fabs(diagErr) < abstol)
            status = SUCCESS;

        }
    else
        {
        status = ERROR;
        }

    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlRotatedConic_shiftedRank1Matrix                     |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
|
+----------------------------------------------------------------------*/
static StatusInt jmdlRotatedConic_shiftedRank1Matrix

(
DPoint3dP pH,
double    *pLambda,
RotMatrixCP pA,
DPoint3dCP pD
)
    {
    RotMatrix absA;
    double absTol;
    static double relTol = 1.0e-7;
    StatusInt status = ERROR;
    double lambda0, lambda1, lambda2;
    double diag0, diag1, diag2;
    int i;

    absA = *pA;
    for (i = 0; i < 3; i++)
        {
        absA.form3d[0][i] = fabs (pA->form3d[0][i]);
        absA.form3d[1][i] = fabs (pA->form3d[1][i]);
        absA.form3d[2][i] = fabs (pA->form3d[2][i]);
        }

    absTol = relTol * bsiRotMatrix_maxAbs (&absA);

    if (absA.form3d[0][1] > absTol && absA.form3d[0][2] > absTol && absA.form3d[1][2] > absTol)
        {
        double arg = pA->form3d[0][1] * pA->form3d[0][2] / pA->form3d[1][2];
        if (arg > 0.0)
            {
            pH->x = sqrt (arg);
            pH->y = pA->form3d[0][1] / pH->x;
            pH->z = pA->form3d[0][2] / pH->x;
            lambda0 = (pA->form3d[0][0] - pH->x * pH->x) / pD->x;
            lambda1 = (pA->form3d[1][1] - pH->y * pH->y) / pD->y;
            lambda2 = (pA->form3d[2][2] - pH->z * pH->z) / pD->z;
            if (fabs(lambda0 - lambda1) < absTol && fabs (lambda1 - lambda2) < absTol)
                {
                *pLambda = lambda0;
                status = SUCCESS;
                }
            }
        }
    else if (absA.form3d[0][1] > absTol && absA.form3d[0][2] <= absTol && absA.form3d[1][2] <= absTol)
        {
        return jmdlRotatedConic_shiftedRank1Matrix_case00 (&pH->x, &pH->y, &pH->z, pLambda,
                                            pA->form3d[0][0], pA->form3d[1][1], pA->form3d[2][2],
                                            pA->form3d[0][1],
                                            pD->x, pD->y, pD->z,
                                            absTol);
        }
    else if (absA.form3d[0][1] <= absTol && absA.form3d[0][2] > absTol && absA.form3d[1][2] <= absTol)
        {
        return jmdlRotatedConic_shiftedRank1Matrix_case00 (&pH->z, &pH->x, &pH->y, pLambda,
                                            pA->form3d[2][2], pA->form3d[0][0], pA->form3d[1][1],
                                            pA->form3d[0][2],
                                            pD->z, pD->x, pD->y,
                                            absTol);
        }
    else if (absA.form3d[0][1] <= absTol && absA.form3d[0][2] <= absTol && absA.form3d[1][2] > absTol)
        {
        return jmdlRotatedConic_shiftedRank1Matrix_case00 (&pH->y, &pH->z, &pH->x, pLambda,
                                            pA->form3d[1][1], pA->form3d[2][2], pA->form3d[0][0],
                                            pA->form3d[1][2],
                                            pD->y, pD->z, pD->x,
                                            absTol);
        }
    else if (absA.form3d[0][1] <= absTol && absA.form3d[0][2] <= absTol && absA.form3d[1][2] <= absTol)
        {

        /* Compute lambdas assuming H entries are zero.  Two of these must agree */
        lambda0 = - pA->form3d[0][0] / pD->x;
        lambda1 = - pA->form3d[1][1] / pD->y;
        lambda2 = - pA->form3d[2][2] / pD->z;
        diag0  = pA->form3d[0][0] + lambda1 * pD->x;
        diag1  = pA->form3d[1][1] + lambda2 * pD->y;
        diag2  = pA->form3d[2][2] + lambda0 * pD->z;

        if (fabs (lambda0 - lambda1) < absTol && diag2 >= 0.0)
            {
            pH->x = pH->y = 0.0;
            *pLambda = lambda0;
            pH->z = sqrt (diag2);
            status = SUCCESS;
            }
        else if (fabs (lambda0 - lambda2) < absTol && diag1 >= 0.0)
            {
            pH->x = pH->z = 0.0;
            *pLambda = lambda2;
            pH->y = sqrt (diag1);
            status = SUCCESS;
            }
        else if (fabs (lambda1 - lambda2) < absTol && diag0 >= 0.0)
            {
            pH->y = pH->z = 0.0;
            *pLambda = lambda1;
            pH->x = sqrt (diag0);
            status = SUCCESS;
            }
        }
    return status;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlQSICRA_init                                         |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static void    jmdlQSICRA_init

(
QSIC_RayArray  *pArray
)
    {
    pArray->numRay = 0;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlQSICRA_getCount                                     |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static int     jmdlQSICRA_getCount

(
QSIC_RayArray  *pArray
)
    {
    return pArray->numRay;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlQSICRA_getAngles                                    |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static StatusInt       jmdlQSICRA_getAngles

(
double  *pTheta,
int     *pNumTheta,
int     maxTheta,
const QSIC_RayArray     *pArray
)
    {
    int numOut = pArray->numRay;
    int i;
    StatusInt status = SUCCESS;

    if (numOut > maxTheta)
        {
        status = ERROR;
        numOut = maxTheta;
        }

    for (i = 0; i < pArray->numRay; i++)
        {
        pTheta[i] = pArray->ray[i].theta;
        }
    *pNumTheta = numOut;

    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlQSICRA_compareRayTheta                              |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static int     jmdlQSICRA_compareRayTheta

(
const QSIC_Ray *pRay0,
const QSIC_Ray *pRay1
)
    {
    if (pRay0->theta < pRay1->theta)
        return -1;

    if (pRay0->theta > pRay1->theta)
        return 1;

    return 0;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlQSICRA_sort                                         |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static void    jmdlQSICRA_sort

(
QSIC_RayArray    *pArray
)
    {
    qsort (pArray->ray, pArray->numRay, sizeof(QSIC_Ray),
            (int (*)(const void *,const void *))jmdlQSICRA_compareRayTheta);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlQSICRA_duplicateStartPoint                          |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
| Make a copy of point 0 as the last point, with theta increased by 2pi |
+----------------------------------------------------------------------*/
static void    jmdlQSICRA_duplicateStartPoint

(
QSIC_RayArray    *pArray
)
    {
    int n0 = pArray->numRay;
    if (n0 > 0 && n0 < QSIC_MAX_RAY - 1)
        {
        pArray->ray[n0] = pArray->ray[0];
        pArray->ray[n0].theta += msGeomConst_2pi;
        pArray->numRay++;
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlRotatedConic_pointsOnCylinderRay                    |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
| Fill pPoint with all the 'relevant' data about intersection of a      |
| cylinder ray with a quadric.  That is, set:                           |
|   x = cos(theta)                                                      |
|   y = sin(theta)                                                      |
|   z = F^t * A                                                         |
|   w = F^t * R * F                                                     |
+----------------------------------------------------------------------*/
static void    jmdlQSIC_initRay

(
QSIC_Ray    *pRay,
const QSIC_Coefficients *pCoffs,
double    theta,
int       typecode
)
    {
    double c = cos (theta);
    double s = sin (theta);
    double FTA, FTRF;
    //StatusInt status = ERROR;

    DPoint3d tempPoint;
    bsiRotMatrix_multiplyComponents (&pCoffs->matrixR, &tempPoint, c, s, 1.0);
    FTRF = c * tempPoint.x + s * tempPoint.y + tempPoint.z;
    FTA  = c * pCoffs->vectorA.x + s * pCoffs->vectorA.y + pCoffs->vectorA.z;

    pRay->c = c;
    pRay->s = s;
    pRay->theta = bsiTrig_getPositiveNormalizedAngle (theta);
    pRay->FTA = FTA;
    pRay->FTRF = FTRF;
    pRay->typecode = typecode;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlQSICRA_addRay                                       |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static StatusInt   jmdlQSICRA_addRay

(
QSIC_RayArray       *pArray,
const QSIC_Coefficients     *pCoffs,
double              theta,
int                 typecode
)
    {
    StatusInt status = ERROR;
    if (pArray->numRay < QSIC_MAX_RAY)
        {
        jmdlQSIC_initRay (&pArray->ray[pArray->numRay++], pCoffs, theta, typecode);
        status = SUCCESS;
        }
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlQSIC_initCoefficients                               |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static void    jmdlQSIC_initCoefficients

(
QSIC_Coefficients  *pParams,
RotMatrixCP pMatrix,
DPoint3dCP pVector,
double          alpha
)
    {
    pParams->matrixR = *pMatrix;
    pParams->vectorA = *pVector;
    pParams->alpha   = alpha;
    }



/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlQSICRA_addCriticalPoints                            |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
| Compute critical points of the discriminant and add the angles to     |
| the rays.                                                             |
+----------------------------------------------------------------------*/
static void    jmdlQSICRA_addCriticalPoints

(
QSIC_RayArray      *pArray,
RotMatrixCP pMatrix,
const QSIC_Coefficients  *pParams,
int              typecode
)
    {
    double sinTheta[4];
    double cosTheta[4];
    double theta[4];
    int i;
    int numIntersection = 0;
    RotMatrix rMatrix = *pMatrix;
    if (SUCCESS == bsiMath_implicitConicIntersectUnitCircle
                    (
                    sinTheta,
                    cosTheta,
                    theta,
                    &numIntersection,
                    &rMatrix
                    )
        && numIntersection > 0)
        {
        bsiDoubleArray_sort (theta, numIntersection, true);
        for (i = 0; i < numIntersection; i++)
            {
            jmdlQSICRA_addRay (pArray, pParams, theta[i], typecode);
            }
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlQSICRA_addPlaneIntersections                        |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
| Compute intersections of a cylinder-quadric intersection curve with   |
| a plane.  Add the intersections to a ray array.                       |
+----------------------------------------------------------------------*/
static void    jmdlQSICRA_addPlaneIntersections

(
QSIC_RayArray      *pArray,
const QSIC_Coefficients  *pParams,
DPoint4dCP pPlane,
int              typecode
)
    {
    DPoint3d bCoff;
    double alpha = pParams->alpha;
    RotMatrix myMatrix;

    bCoff.x = alpha * pPlane->x - pPlane->z * pParams->vectorA.x;
    bCoff.y = alpha * pPlane->y - pPlane->z * pParams->vectorA.y;
    bCoff.z = alpha * pPlane->w - pPlane->z * pParams->vectorA.z;

    jmdlQSIC_discriminantMatrix (&myMatrix, &pParams->matrixR, &bCoff, pPlane->z * pPlane->z);

    jmdlQSICRA_addCriticalPoints (pArray, &myMatrix, pParams, typecode);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlQSIC_expandParameters                               |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static int     jmdlQSIC_expandParameters

(
DPoint4dP pPoint0,
DPoint4dP pPoint1,
const QSIC_Ray *pRay,
const QSIC_Coefficients *pCoffs
)
    {
    double c = pRay->c;
    double s = pRay->s;
    double FTA = pRay->FTA;
    double FTRF = pRay->FTRF;
    double delta;
    double alpha = pCoffs->alpha;
    int     numRoot = 0;

    //StatusInt status = ERROR;

    pPoint0->x = pPoint1->x = alpha * c;
    pPoint0->y = pPoint1->y = alpha * s;
    pPoint0->z = pPoint1->z = -FTA;
    pPoint0->w = pPoint1->w = alpha;

    if (FTRF > 0.0)
        {
        delta = sqrt (FTRF);
        numRoot = 2;
        pPoint0->z -= delta;
        pPoint1->z += delta;
        }
    else if (FTRF == 0.0)
        {
        numRoot = 1;
        }

    return numRoot;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlQSIC_expandAndMap                                   |
|                                                                       |
| author        EarlinLutz                              10/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static void jmdlQSIC_expandAndMap

(
DPoint4dP pHPoint,
DPoint3dP pRPoint,
double      *pTheta,
const QSIC_Ray      *pRay,
int         index,
const RotatedConic  *pSurface,
const QSIC_Coefficients *pCoffs
)
    {
    DPoint4d hPoint[2];
    jmdlQSIC_expandParameters (&hPoint[0], &hPoint[1], pRay, pCoffs);
    bsiDMatrix4d_multiply4dPoints (&pSurface->rotationMap.M0, pHPoint, index ? &hPoint[1] : &hPoint[0], 1); pHPoint->GetProjectedXYZ (*pRPoint) ; /* THISWAS a bool thrown away as a statement */
    *pTheta = pRay->theta;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlQSIC_collectRange                                   |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static int     jmdlQSICRA_collectRayRange

(
QSIC_Ray  *pBuffer,   /* <= Packed buffer.  Assumed size numPoint, i.e. enough to hold a complete range */
const QSIC_RayArray  *pArray,   /* => ray definitions */
int         i0,             /* => first index of range to extract */
int         i1              /* => last index of range to extract */
)
    {
    int i;
    int numOut = 0;
    int numPoint = pArray->numRay;

    if (i0 <= i1)
        {
        for (i = i0; i <= i1; i++)
            {
            pBuffer[numOut++] = pArray->ray[i];
            }
        }
    else
        {

        for (i = i0; i < numPoint; i++)
            {
            pBuffer[numOut++] = pArray->ray[i];
            }

        for (i = 0; i <= i1; i++)
            {
            pBuffer[numOut++] = pArray->ray[i];
            }
        }
    return numOut;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlQSIC_getMidRangeRay                                 |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static StatusInt       jmdlQSIC_getMidRangeRay

(
QSIC_Ray  *pMidRay,   /* <= A ray somewhere in the middle of the range of
rays.   May be a copy or a new ray.
Uninitialized if ERROR */
const QSIC_Ray  *pInputRay, /* => sorted array of rays. */
int numIn,          /* => number of input rays */
const QSIC_Coefficients *pCoffs,
double minDelta     /* => returns ERROR if angular range is smaller */
)
    {
    int i0 = 0;
    int i1 = numIn - 1;
    StatusInt status = ERROR;
    double sweep, theta;

    if (i1 > i0)
        {
        sweep = pInputRay[i1].theta - pInputRay[i0].theta;
        if (fabs (sweep) > minDelta)
            {
            theta = pInputRay[i0].theta + 0.5 * sweep;
            jmdlQSIC_initRay (pMidRay, pCoffs, theta, MID_RANGE_POINT);
            status = SUCCESS;
            }
        }
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlQSIC_clipAndOutputLine                              |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static void    jmdlQSIC_clipAndOutputLine

(
DPoint4dCP pPoint0,
DPoint4dCP pPoint1,
const RotatedConic *pSurface,
const RotatedConic_Tree *pTree,
QSIC_Params     *pParams
)
    {
    int planeId, i0, i1;
    DPoint4d hPlane;
    double lambdaCut[10];
    double lambdaMid;
    DPoint4d outputPoint[2];
    DPoint3d outputPoint3d[2];

    int    numCut;
    DPoint4d hDelta, testPoint;
    int numOut;
    bool    pointIsIn;
    double dot0, dot1;

    bsiDPoint4d_addScaledDPoint4d (&hDelta, pPoint1, pPoint0, -1.0);
    numCut = 0;
    lambdaCut[numCut++] = 0.0;
    lambdaCut[numCut++] = 1.0;
    for (planeId = 0; SUCCESS == bsiRCTree_getPlane (&hPlane, pTree, planeId); planeId++)
        {
        if (hPlane.z != 0.0)
            {
            dot0 = bsiDPoint4d_dotProduct (pPoint0, &hPlane);
            dot1 = bsiDPoint4d_dotProduct (&hDelta, &hPlane);

            if (fabs (dot1) >= fabs (dot0))
                {
                lambdaCut[numCut] = - dot0 / dot1;
                if (lambdaCut[numCut] > 0.0)
                    numCut++;
                }
            }
        }


    bsiDoubleArray_sort (lambdaCut, numCut, true);

    for (i1 = 1; i1 < numCut; i1++)
        {
        i0 = i1 - 1;
        lambdaMid = 0.5 * (lambdaCut[i0] + lambdaCut[i1]);
        bsiDPoint4d_addScaledDPoint4d (&testPoint, pPoint0, &hDelta, lambdaMid);
        if (SUCCESS == bsiRCTree_classifyPoint (&pointIsIn, pTree, &testPoint)
            && pointIsIn)
            {
            bsiDPoint4d_addScaledDPoint4d (&outputPoint[0], pPoint0, &hDelta, lambdaCut[i0]);
            bsiDPoint4d_addScaledDPoint4d (&outputPoint[1], pPoint0, &hDelta, lambdaCut[i1]);
            numOut = 2;

            bsiDMatrix4d_multiply4dPoints (&pSurface->rotationMap.M0, outputPoint, outputPoint, numOut);

            bsiDPoint4d_normalizeArray (outputPoint3d, outputPoint, numOut);
            pParams->handlerFunc (NULL, outputPoint3d, numOut, 0, pSurface, pParams->pUserData);
            }
        }
    }

/* Reasonable cut limits: 2 surfaces * 4 planes/surface * 2 points/plane + 2= 18 */
#define MAX_ELLIPSE_PLANE_CUT 20
/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlQSIC_clipAndOutputEllipse                           |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static void    jmdlQSIC_clipAndOutputEllipse

(
const HConic    *pConic,        /* => which must be an ellipse */
const RotatedConic *pSurface,   /* => in this coordinate system */
const RotatedConic_Tree *pTree0,
const RotatedConic_Tree *pTree1,
QSIC_Params     *pParams
)
    {
    int         planeId, i0, i1, i;
    DPoint3d    trigPoint[2];
    HConic      sectorConic;
    int         numIntersection;
    DPoint4d    hPlane;
    double      thetaCut[MAX_ELLIPSE_PLANE_CUT];
    bool        classification[MAX_ELLIPSE_PLANE_CUT];
    bool        currClassification;
    int         numCut = 2;
    int         numDistinctCut;
    double      thetaMid, theta0, theta1, thetaSweep, theta;
    DPoint4d    testPoint;
    const RotatedConic_Tree *treePointer[2];
    int         numTree = 0;
    int         sector;
    int         treeId;
    double      smallAngle = bsiTrig_smallAngle ();

    if (pTree0)
        treePointer[numTree++] = pTree0;
    if (pTree1)
        treePointer[numTree++] = pTree1;

    /* Collect up all the places where the ellipse might cut a bounding plane */
    for (sector = 0;
         SUCCESS == bsiHConic_getSegment (&theta0, &theta1, pConic, sector);
         sector++)
        {
        thetaCut[0] = theta0;
        thetaCut[1] = theta1;
        thetaSweep = theta1 - theta0;
        numCut = 2;
        for (treeId = 0; treeId < numTree; treeId++)
            {
            for (planeId = 0; SUCCESS == bsiRCTree_getPlane (&hPlane, treePointer[treeId], planeId); planeId++)
                {
                numIntersection = bsiDEllipse4d_intersectPlane
                                                (
                                                trigPoint,
                                                &pConic->coordinates.center,
                                                &pConic->coordinates.vector0,
                                                &pConic->coordinates.vector90,
                                                &hPlane
                                                );
                for (i = 0; i < numIntersection && numCut < MAX_ELLIPSE_PLANE_CUT; i++)
                    {
                    theta = bsiTrig_adjustAngleToSweep (trigPoint[i].z, theta0, thetaSweep);
                    if (bsiTrig_angleInSweep (theta, theta0, thetaSweep))
                        thetaCut[numCut++] = theta;
                    }
                }
            }
        }

    bsiDoubleArray_sort (thetaCut, numCut, true);
    numDistinctCut = 1;
    theta = thetaCut[0] + smallAngle;
    for (i = 1; i < numCut; i++)
        {
        if (thetaCut[i] > theta)
            {
            thetaCut[numDistinctCut++] = thetaCut[i];
            theta = thetaCut[i] + smallAngle;
            }
        }
    numCut = numDistinctCut;
    /* Classify the midpoint of each sector of the ellipse */
    for (i1 = 1; i1 < numCut; i1++)
        {
        i0 = i1 - 1;
        thetaMid = 0.5 * (thetaCut[i1] + thetaCut[i0]);
        bsiDEllipse4d_evaluateDPoint4d (&pConic->coordinates, &testPoint, thetaMid);
        classification[i0] = true;
        for (treeId = 0; treeId < numTree && classification[i0]; treeId++)
            {
            if (SUCCESS == bsiRCTree_classifyPoint (&currClassification, treePointer[treeId], &testPoint))
                classification[i0] &= currClassification;
            }
        }

    for (i0 = 0; i0 < numCut - 1; i0 = i1)
        {
        i1 = i0 + 1;
        if (classification[i0])
            {
            while (i1 < numCut - 1 && classification[i1])
                {
                i1++;
                }
            sectorConic = *pConic;
            bsiRange1d_setUncheckedArcSweep (&sectorConic.coordinates.sectors, thetaCut[i0], thetaCut[i1] - thetaCut[i0]);
            bsiHConic_multiplyHMatrix (&sectorConic, &sectorConic, &pSurface->rotationMap.M0);
            pParams->handlerFunc (&sectorConic, NULL, 0, 0, pSurface, pParams->pUserData);
            }
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlQSIC_cylinderLineIntersection
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static StatusInt jmdlQSIC_outputLinesOnCylinder

(
const RotatedConic      *pSurface0,
const RotatedConic      *pSurface1,
const RotatedConic_Tree *pTree0,
const RotatedConic_Tree *pTree1,
RotMatrixCP pMatrix,       /* <= matrix identifying critical angles */
QSIC_Params             *pParams
)
    {
    double cosTheta[4];
    double sinTheta[4];
    double theta[4];
    int numTheta;
    DPoint4d point0, point1;
    double oldTheta;
    int i;
    StatusInt status = ERROR;
    double smallAngle = bsiTrig_smallAngle ();

    double theta0   = pSurface0->parameterRange.low.x;
    double theta1   = pSurface0->parameterRange.high.x;
    double thetaSweep = theta1 - theta0;

    double alpha0  = pSurface0->parameterRange.low.y;
    double alpha1  = pSurface0->parameterRange.high.y;
    RotMatrix rMatrix = *pMatrix;
    if (SUCCESS == bsiMath_implicitConicIntersectUnitCircle (cosTheta, sinTheta, theta, &numTheta, &rMatrix))
        {
        if (numTheta <= 0 )
            {
            status = SUCCESS;
            }
        else
            {
            bsiDoubleArray_sort (theta, numTheta, true);
            oldTheta = theta[0] - 1.0;
            for (i = 0; i < numTheta && theta[i] > oldTheta; i++)
                {
                if (theta[i] > oldTheta + smallAngle && bsiTrig_angleInSweep (theta[i], theta0, thetaSweep))
                    {
                    double c = cos (theta[i]);
                    double s = sin (theta[i]);

                    bsiDPoint4d_setComponents (&point0, c, s, alpha0, 1.0);
                    bsiDPoint4d_setComponents (&point1, c, s, alpha1, 1.0);

                    jmdlQSIC_clipAndOutputLine (&point0, &point1, pSurface0, pTree1, pParams);
                    oldTheta = theta[i];
                    }
                }
            status = SUCCESS;
            }
        }
    return status;
    }

#define MAX_THETA_SAMPLE 120
#define MAX_RANGE 8
/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlQSIC_UCQI_debugCriticalAngles                       |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static void jmdlQSIC_UCQI_debugCriticalAngles

(
QSIC_RayArray *pArray,
const RotatedConic *pSurface0,
const RotatedConic *pSurface1,
const RotatedConic_Tree *pTree0,
const RotatedConic_Tree *pTree1,
const QSIC_Coefficients *pCoffs,
QSIC_Params *pParams
)
    {
    int i;
    DPoint4d hPoint[20];
    DPoint3d rPoint[20];
    int numOut;
    double c, s, theta;
    int typecode;
    double clipDist;

    for (i = 0; i < pArray->numRay; i++)
        {
        typecode = pArray->ray[i].typecode;
        if (typecode != GRID_SAMPLE_POINT)
            {
            numOut=0;
            theta = pArray->ray[i].theta;
            c     = cos (theta);
            s     = sin (theta);
            if (typecode == CRITICAL_POINT)
                {
                clipDist = -0.1;
                }
            else
                {
                clipDist = 0.0;
                }
            bsiDPoint4d_setComponents (&hPoint[numOut++], c, s,       clipDist, 1.0);
            bsiDPoint4d_setComponents (&hPoint[numOut++], c, s, 1.0 - clipDist, 1.0);

            bsiDMatrix4d_multiply4dPoints (&pSurface0->rotationMap.M0, hPoint, hPoint, numOut);

            bsiDPoint4d_normalizeArray (rPoint, hPoint, numOut);
            pParams->handlerFunc (NULL, rPoint, numOut, RC_CURVEMASK_SMOOTH, pSurface0, pParams->pUserData);
            }
        }
    }

#define MAX_OUTPUT_POINT 200
/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlQSIC_UCQI_output                                    |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static void jmdlQSIC_UCQI_refineAndOutput

(
QSIC_Ray *pRay,     /* => ray definitions */
int numRay,
int index,          /* => 0 for lower index, 1 for upper */
const RotatedConic *pSurface0,
const QSIC_Coefficients *pCoffs,
QSIC_Params *pParams
)
    {
    DPoint3d rPoint[MAX_OUTPUT_POINT];
    DPoint4d currHChord[3];
    DPoint3d currRChord[3];
    QSIC_Ray testRay;
    double currTheta[3];
    double thetaMid;
    int numOut = 0;
    int i, k;
    double chordTolerance, chordError;
    DPoint3d projectionToChord;
    double projectionParameter, subdivisionDenominator;
    int maxSubChord = 8;
    int numSubChord;
    double thetaStep;
    static int s_smooth = true;
    int mask = s_smooth ? RC_CURVEMASK_SMOOTH : 0;

    if (index != 0)
        index = 1;

    chordTolerance = 4.0 * pParams->tolerance;
    subdivisionDenominator = pParams->tolerance;

    jmdlQSIC_expandAndMap (&currHChord[0], &currRChord[0], &currTheta[0], &pRay[0], index, pSurface0, pCoffs);
    currTheta[0] = pRay[0].theta;
    rPoint[numOut++] = currRChord[0];

    for (i = 1; i < numRay; i++)
        {
        jmdlQSIC_expandAndMap (&currHChord[2], &currRChord[2], &currTheta[2], &pRay[i], index, pSurface0, pCoffs);

        thetaMid = 0.5 * (currTheta[0] + currTheta[2]);
        jmdlQSIC_initRay (&testRay, pCoffs, thetaMid, SUBDIVISION_POINT);
        jmdlQSIC_expandAndMap (&currHChord[1], &currRChord[1], &currTheta[1], &testRay, index, pSurface0, pCoffs);
        bsiGeom_projectPointToLine (&projectionToChord, &projectionParameter,
                                        &currRChord[1], &currRChord[0], &currRChord[2]);
        chordError = projectionToChord.Distance (currRChord[1]);

        if (numOut + 1 + maxSubChord >= MAX_OUTPUT_POINT)
            {
            /* Flush the buffer and shift current end to start */
            pParams->handlerFunc (NULL, rPoint, numOut, mask, pSurface0, pParams->pUserData);
            numOut = 0;
            rPoint[numOut++] = currRChord[0];
            }

        if (chordTolerance <= 0.0 || chordError <= chordTolerance)
            {
            /* Just output the midpoint */
            rPoint[numOut++] = currRChord[1];
            }
        else
            {
            numSubChord = (int)(sqrt (chordError / subdivisionDenominator) + 1.0);

            if (numSubChord < 3)
                numSubChord = 3;
            if (numSubChord > maxSubChord)
                numSubChord = maxSubChord;

            thetaStep = (currTheta[2] - currTheta[0]) / (double) numSubChord;
            for (k = 1; k < numSubChord; k++)
                {
                thetaMid = currTheta[0] + k * thetaStep;
                jmdlQSIC_initRay (&testRay, pCoffs, thetaMid, SUBDIVISION_POINT);
                jmdlQSIC_expandAndMap (&currHChord[1], &currRChord[1], &currTheta[1], &testRay, index, pSurface0, pCoffs);
                rPoint[numOut++] = currRChord[1];
                }

            }
        rPoint[numOut++] = currRChord[2];
        currHChord[0] = currHChord[2];
        currRChord[0] = currRChord[2];
        currTheta[0]  = currTheta[2];
        }

    pParams->handlerFunc (NULL, rPoint, numOut, mask, pSurface0, pParams->pUserData);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlQSIC_UCQI_output                                    |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static void jmdlQSIC_UCQI_output

(
QSIC_Ray *pRay,     /* => ray definitions */
int numRay,
const RotatedConic *pSurface0,
const RotatedConic *pSurface1,
const RotatedConic_Tree *pTree0,
const RotatedConic_Tree *pTree1,
const QSIC_Coefficients *pCoffs,
void *vpParams
)
    {
    QSIC_Params *pParams = (QSIC_Params*)vpParams;
    DPoint4d lowPoint, highPoint;
    double minOutputSpan = 0.01;
    QSIC_Ray testRay;
    bool    pointIsIn;
    static bool    alwaysOutput = false;

    if (SUCCESS == jmdlQSIC_getMidRangeRay (&testRay, pRay, numRay, pCoffs, minOutputSpan)
        && testRay.FTRF > 0.0
       )
        {
        jmdlQSIC_expandParameters (&lowPoint, &highPoint, &testRay, pCoffs);

        if (    alwaysOutput
            ||
                (   SUCCESS == bsiRCTree_classifyPoint (&pointIsIn, pTree0, &lowPoint)
                &&  pointIsIn
                && SUCCESS == bsiRCTree_classifyPoint (&pointIsIn, pTree1, &lowPoint)
                &&  pointIsIn
                )
           )
            {
            jmdlQSIC_UCQI_refineAndOutput (pRay, numRay, 0, pSurface0, pCoffs, pParams);
            }

        if (    alwaysOutput
            ||
                (   SUCCESS == bsiRCTree_classifyPoint (&pointIsIn, pTree0, &highPoint)
                &&  pointIsIn
                && SUCCESS == bsiRCTree_classifyPoint (&pointIsIn, pTree1, &highPoint)
                &&  pointIsIn
                )
           )
            {
            jmdlQSIC_UCQI_refineAndOutput (pRay, numRay, 1,  pSurface0, pCoffs, pParams);
            }
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlQSICRA_sampleIntervals                              |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static void jmdlQSICRA_sampleIntervals

(
QSIC_RayArray   *pArray,
double          *pCriticalAngle,        /* Assumed large enough for 2 additional angles to be added !! */
int             numCriticalAngle,
double          theta0,
double          theta1,
double          maxDTheta,
int             minPerInterval,
QSIC_Coefficients       *pCoffs,
int             typeCode
)
    {
    int i;
    int numTheta;
    double dTheta, thetaCurr;
    double thetaA, thetaB;
    //bool    done = false;
    double sweep = theta1 - theta0;
    int j;
    double tol = bsiTrig_smallAngle();

    if (sweep < 0.0)
        {
        double temp = theta0;
        theta0 = theta1;
        theta1 = temp;
        sweep  = -sweep;
        }

    for (i = 0; i < numCriticalAngle; i++)
        {
        pCriticalAngle[i] = bsiTrig_adjustAngleToSweep (pCriticalAngle[i], theta0, sweep);
        }

    while (numCriticalAngle > 0 && pCriticalAngle[numCriticalAngle - 1] >= theta1)
        numCriticalAngle--;

    pCriticalAngle[numCriticalAngle++] = theta1;

    bsiDoubleArray_sort (pCriticalAngle, numCriticalAngle, true);

    thetaA = theta0;
    for (i = 0; i < numCriticalAngle; i++)
        {
        thetaB = pCriticalAngle[i];
        if (thetaB > thetaA + tol)
            {
            numTheta = (int)((thetaB - thetaA) / maxDTheta);
            if (numTheta < minPerInterval)
                numTheta = minPerInterval;
            dTheta = (thetaB - thetaA) / numTheta;
            for (j = 1; j < numTheta; j++)
                {
                thetaCurr = thetaA + j * dTheta;
                jmdlQSICRA_addRay (pArray, pCoffs,thetaCurr, typeCode);
                }
            }
        thetaA = thetaB;
        }
    }

#define MAX_THETA_SAMPLE 120
#define MIN_POINT_PER_INTERVAL 12
#define MAX_RANGE 8
#define MAX_CRITICAL_ANGLE 10

typedef enum
    {
    QSIC_CaseNotHandled,
    QSIC_CoincidentSurfaces,
    QSIC_ConicCase,
    QSIC_CompleteAngularRange,
    QSIC_PartialAngularRange
    } QSIC_CaseDescriptor;
/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlQSIC_unitCylinderQuadricSpecialCaseTest             |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static bool    jmdlQSIC_isUnitZCylinderWithZScale

(
DMatrix4dP pA,
double      relTol
)
    {
    double refVal = relTol * fabs (pA->coff[3][3]);
    double b = - pA->coff[3][3];
    int i, j;
    for (i = 0; i < 3; i++)
        for (j = i + 1; j < 4; j++)
            {
            if (fabs (pA->coff[i][j]) > refVal)
                return false;
            if (fabs (pA->coff[j][i]) > refVal)
                return false;
            }


    if (fabs (pA->coff[0][0] - b) > refVal)
        return false;
    if (fabs (pA->coff[1][1] - b) > refVal)
        return false;
    if (fabs (pA->coff[2][2]) > refVal)
        return false;
    return true;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlQSIC_unitCylinderQuadricSpecialCaseTest             |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static QSIC_CaseDescriptor jmdlQSIC_unitCylinderQuadricSpecialCaseTest

(
const RotatedConic  *pSurface0,  /* => embedding data for cylinder */
const RotatedConic  *pSurface1   /* => embedding data for far surface */
)
    {
    DPoint4d sigma1;
    bool     surface1IsQuadric;
    DMap4d    hMap0to1;
    DMatrix4d matrixA;
    RotMatrix matrixQ;
    RotMatrix matrixR;
    DPoint3d vectorA;
    int numCriticalAngle;
    double   alpha;
    QSIC_Coefficients qCoffs;
    QSIC_RayArray rayArray;
    double scaleA;
    double tolA;
    static double relTol = 1.0e-13;
    static double matrixRelTol = 1.0e-6;
    double shiftFactor;
    DPoint3d vectorH, vectorD;
    QSIC_CaseDescriptor caseDescriptor = QSIC_CaseNotHandled;

    surface1IsQuadric = bsiRotatedConic_isQuadric (&sigma1, pSurface1);

    bsiRotatedConic_mappedBasis (&hMap0to1, pSurface0, pSurface1);
    bsiDMatrix4d_symmetricProduct (&matrixA, &sigma1, &hMap0to1.M0);
    if (jmdlQSIC_isUnitZCylinderWithZScale (&matrixA, matrixRelTol))
        {
        caseDescriptor = QSIC_CoincidentSurfaces;
        }
    else
        { bsiDMatrix4d_extractAroundPivot(&matrixA, &matrixQ, &vectorA, NULL, &alpha, 2) ; /* THISWAS a bool thrown away as a statement */
        scaleA = bsiDMatrix4d_maxAbs (&matrixA);
        tolA   = relTol * scaleA;
        jmdlQSIC_discriminantMatrix (&matrixR, &matrixQ, &vectorA, alpha);
        jmdlQSIC_initCoefficients (&qCoffs, &matrixR, &vectorA, alpha);

            vectorD.Init ( 1.0, 1.0, -1.0);

        /* We are dealing with a quadratic equation
            alpha * lambda^2 + 2 FT*A * lambda + FT*Q*F = 0
            whose coefficients (alpha, 2FT*A, FT*R*F) vary with theta.
            Check for degenerate cases:
        */
        if (fabs (alpha) <= tolA)
            {
            if (bsiDPoint3d_maxAbs (&vectorA) > tolA)
                {
                /* This case needs a whole new branch on the QSIC ray code */
                /* (This only occurs in intersection with hyperboloids) */
                /* Just fall out with QSIC_CaseNotHandled */
                }
            else
                {
                caseDescriptor = QSIC_ConicCase;
                }
            }
        else
            {
            int rankStatus;

            rankStatus = jmdlRotatedConic_shiftedRank1Matrix (&vectorH, &shiftFactor, &matrixR, &vectorD);

            if (SUCCESS == rankStatus)
                {
                caseDescriptor = QSIC_ConicCase;
                }
            else
                {
                jmdlQSICRA_init (&rayArray);
                jmdlQSICRA_addCriticalPoints
                                                (
                                                &rayArray,
                                                &qCoffs.matrixR,
                                                &qCoffs,
                                                CRITICAL_POINT
                                                );
                numCriticalAngle = jmdlQSICRA_getCount (&rayArray);

                if (numCriticalAngle == 0)
                    {
                    caseDescriptor = QSIC_CompleteAngularRange;
                    }
                else
                    {
                    caseDescriptor = QSIC_PartialAngularRange;
                    }

                }
            }
        }
    return caseDescriptor;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          unitCylinderQuadricIntersection                         |
|                                                                       |
| author        EarlinLutz                              09/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static StatusInt jmdlQSIC_unitCylinderQuadricIntersection

(
const RotatedConic  *pSurface0,     /* => embedding data for cylinder */
const RotatedConic  *pSurface1,     /* => embedding data for far surface */
const RotatedConic  *pClipSurface0, /* => first clip surface (commonly the same as pSurface0)  */
const RotatedConic  *pClipSurface1, /* => second clip surface (commonly the same as pSurface1) */
const DPoint4d      *pEyePoint,     /* => optional eyepoint.  Intersection curve
                                            has breaks at silhouette  points */
QSIC_ArrayHandler   handlerFunc,
QSIC_Params         *pParams,
int                 showDebug
)
    {
    DPoint4d sigma1;
    DMap4d    hMap0to1;
    bool    surface1IsQuadric;
    DMatrix4d matrixA;
    RotMatrix matrixQ;
    RotMatrix matrixR;
    DPoint3d vectorA;
    double criticalAngle[MAX_CRITICAL_ANGLE];
    int numCriticalAngle;
    double   alpha;
    QSIC_Coefficients qCoffs;
    RotatedConic_Tree clipTree0, clipTree1;
    StatusInt status = ERROR;
    static double maxDTheta = 0.20;
    double theta0, theta1, thetaSweep;
    QSIC_RayArray rayArray;
    double scaleA;
    double tolA;
    static double relTol = 1.0e-13;
    double shiftFactor;
    DPoint3d vectorH, vectorD;
#define DECLARE_VECTORS_AT_FUNCTION_LEVEL
#if defined (DECLARE_VECTORS_AT_FUNCTION_LEVEL)
    HConic conic0, conic1;
    DPoint4d center, vector0, vector90;
#endif


    theta0 = pSurface0->parameterRange.low.x;
    theta1 = pSurface0->parameterRange.high.x;
    thetaSweep = theta1 - theta0;

    surface1IsQuadric = bsiRotatedConic_isQuadric (&sigma1, pSurface1);
    bsiRotatedConic_mappedBasis (&hMap0to1, pSurface0, pSurface1);
    bsiDMatrix4d_symmetricProduct (&matrixA, &sigma1, &hMap0to1.M0); bsiDMatrix4d_extractAroundPivot(&matrixA, &matrixQ, &vectorA, NULL, &alpha, 2) ; /* THISWAS a bool thrown away as a statement */
    scaleA = bsiDMatrix4d_maxAbs (&matrixA);
    tolA   = relTol * scaleA;
    jmdlQSIC_discriminantMatrix (&matrixR, &matrixQ, &vectorA, alpha);
    jmdlQSIC_initCoefficients (&qCoffs, &matrixR, &vectorA, alpha);

    if (   SUCCESS == bsiRCTree_initPatchClip (&clipTree0, pClipSurface0, true, true)
        && SUCCESS == bsiRCTree_initPatchClip (&clipTree1, pClipSurface1, true, true)
       )
        {
        /* All calculations are done in the data space of surface0.  Have to
            transform all clippers to there.   (This might be an identity)
        */
        DMap4d clipMap0, clipMap1;

        bsiRotatedConic_mappedBasis (&clipMap0, pSurface0, pClipSurface0);
        bsiRCTree_transform (&clipTree0, &clipMap0);

        bsiRotatedConic_mappedBasis (&clipMap1, pSurface0, pClipSurface1);
        bsiRCTree_transform (&clipTree1, &clipMap1);

        vectorD.Init ( 1.0, 1.0, -1.0);

        /* We are dealing with a quadratic equation
            alpha * lambda^2 + 2 FT*A * lambda + FT*R*F = 0
            whose coefficients (alpha, 2FT*A, FT*R*R) vary with theta.
            Check for degenerate cases:
        */
        if (fabs (alpha) <= tolA)
            {
            if (bsiDPoint3d_maxAbs (&vectorA) > tolA)
                {
                /* This case needs a whole new branch on the QSIC ray code */
                /* (This only occurs in intersection with hyperboloids) */
                /* Just fall out with QSIC_CaseNotHandled */
                }
            else
                {
                status = jmdlQSIC_outputLinesOnCylinder (
                                    pSurface0,
                                    pSurface1,
                                    &clipTree0,
                                    &clipTree1,
                                    &matrixQ,
                                    pParams);
                }
            }
        else
            {
            int rankStatus;
            jmdlQSICRA_init (&rayArray);
            status = SUCCESS;


            rankStatus = jmdlRotatedConic_shiftedRank1Matrix (&vectorH, &shiftFactor, &matrixR, &vectorD);

            if (SUCCESS == rankStatus)
                {
#if defined (DECLARE_VECTORS_IN_BLOCK)
                /* Oct 1, 1997 E.Lutz and D. Rahnis observe that if these
                    vectors are declared here (as originally coded), the compiler
                    (MSVC 4.1) placed vector0 so it overlayed status.
                */
                HConic conic0, conic1;
                DPoint4d center, vector0, vector90;
#endif
                /* The intersection is just a pair of ellipses !!! */
                bsiDPoint4d_setComponents (&vector0,   alpha,   0.0, -vectorA.x + vectorH.x, 0.0);
                bsiDPoint4d_setComponents (&vector90,    0.0, alpha, -vectorA.y + vectorH.y, 0.0);
                bsiDPoint4d_setComponents (&center,      0.0,   0.0, -vectorA.z + vectorH.z, alpha);
                bsiHConic_init4dEllipseVectors (&conic0, &center, &vector0, &vector90, theta0, thetaSweep);
                jmdlQSIC_clipAndOutputEllipse (&conic0, pSurface0, &clipTree0, &clipTree1, pParams);

                bsiDPoint4d_setComponents (&vector0,   alpha,   0.0, -vectorA.x - vectorH.x, 0.0);
                bsiDPoint4d_setComponents (&vector90,    0.0, alpha, -vectorA.y - vectorH.y, 0.0);
                bsiDPoint4d_setComponents (&center,      0.0,   0.0, -vectorA.z - vectorH.z, alpha);
                bsiHConic_init4dEllipseVectors (&conic1, &center, &vector0, &vector90, theta0, thetaSweep);
                jmdlQSIC_clipAndOutputEllipse (&conic1, pSurface0, &clipTree0, &clipTree1, pParams);
                status = SUCCESS;
                }
            else
                {

                /* The composite curve must include . . .  */

                /* Critical points of the discriminant . . . */

                jmdlQSICRA_addCriticalPoints
                                                (
                                                &rayArray,
                                                &qCoffs.matrixR,
                                                &qCoffs,
                                                CRITICAL_POINT
                                                );
                jmdlQSICRA_getAngles (criticalAngle, &numCriticalAngle, MAX_CRITICAL_ANGLE, &rayArray);
                jmdlQSICRA_sampleIntervals (&rayArray, criticalAngle, numCriticalAngle, theta0, theta1, maxDTheta, MIN_POINT_PER_INTERVAL, &qCoffs, GRID_SAMPLE_POINT);

            /* . . . Intersections of the QSIC with boundary planes in the original surface */

                {
                int iPlane;
                DPoint4d hPlane;
                for (iPlane = 0; SUCCESS == bsiRCTree_getPlane (&hPlane, &clipTree0, iPlane); iPlane++)
                    {
                    jmdlQSICRA_addPlaneIntersections
                                            (
                                            &rayArray,
                                            &qCoffs,
                                            &hPlane,
                                            BOUNDARY_CROSSING_POINT
                                            );

                    }

                for (iPlane = 0; SUCCESS == bsiRCTree_getPlane (&hPlane, &clipTree1, iPlane); iPlane++)
                    {
                    jmdlQSICRA_addPlaneIntersections
                                            (
                                            &rayArray,
                                            &qCoffs,
                                            &hPlane,
                                            BOUNDARY_CROSSING_POINT
                                            );
                    }
                }

            /* . . . Intersections of the QSIC with silhouettes planes */

                if (pEyePoint)
                    {
                    DPoint4d hPlane0, hPlane1, hPlane1in0;
                    DPoint4d eyeInFrame0, eyeInFrame1;
                    bsiDMatrix4d_multiply4dPoints (&pSurface0->rotationMap.M1, &eyeInFrame0, pEyePoint, 1);

                    hPlane0 = eyeInFrame0;
                    hPlane0.z = 0.0;

                    jmdlQSICRA_addPlaneIntersections
                                            (
                                            &rayArray,
                                            &qCoffs,
                                            &hPlane0,
                                            SILHOUETTE_CROSSING_POINT
                                            );

                    bsiDMatrix4d_multiply4dPoints (&pSurface0->rotationMap.M1, &eyeInFrame1, pEyePoint, 1);

                    hPlane1 = eyeInFrame1;
                    hPlane1.z = 0.0;
                    bsiDMatrix4d_multiplyTransposePoints (&hMap0to1.M0, &hPlane1in0, &hPlane1, 1);


                    jmdlQSICRA_addPlaneIntersections
                                            (
                                            &rayArray,
                                            &qCoffs,
                                            &hPlane1in0,
                                            SILHOUETTE_CROSSING_POINT
                                            );

                    }

            if (showDebug)
                jmdlQSIC_UCQI_debugCriticalAngles
                                                (
                                                &rayArray,
                                                pSurface0,
                                                pSurface1,
                                                &clipTree0,
                                                &clipTree1,
                                                &qCoffs,
                                                pParams
                                                );
            jmdlQSICRA_sort (&rayArray);
            jmdlQSICRA_duplicateStartPoint (&rayArray);

                    {
                    QSIC_Ray outputRay[MAX_THETA_SAMPLE];
                    int i0, i1;
                    int numPointInRange = 0;
                    int numCandidate = rayArray.numRay;
                    int numPos, numNeg;

                    for (i0 = 0; i0 < numCandidate;)
                        {
                        numPos = 0;
                        numNeg = 0;
                        /* Find the first critical point following i0 */
                        for (i1 = i0 + 1; i1 < numCandidate && rayArray.ray[i1].typecode == GRID_SAMPLE_POINT; i1++)
                            {
                            }

                        if (i1 == numCandidate)
                            i1--;

                        numPointInRange = jmdlQSICRA_collectRayRange (outputRay, &rayArray, i0, i1);

                        if (numPointInRange > 0)
                            handlerFunc (outputRay,
                                            numPointInRange, pSurface0, pSurface1,
                                            &clipTree0, &clipTree1,
                                            &qCoffs, pParams);

                        if (i1 == numCandidate - 1)
                            {
                            i0 = numCandidate;
                            }
                        else
                            {
                            i0 = i1;
                            }
                        }
                    }
                }

            }

        }
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          isNearPMOne                                             |
|                                                                       |
| author        EarlinLutz                              07/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static bool    isNearPMOne

(
double value,
double tol
)
    {
    return fabs(fabs (value) - 1.0) <= tol;
    }
/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlRotatedConic_isXYOrthogonal                         |
|                                                                       |
| author        EarlinLutz                              07/97           |
|                                                                       |
| Test if matrix has only mirroring and xy rotation.                    |
+----------------------------------------------------------------------*/
static bool       jmdlRotatedConic_isXYOrthogonal

(
DMatrix4dP pA,
double  tol
)
    {
    double a00, a01, a10, a11;
    /* We expect some zeros */
    if (   fabs (pA->coff[0][2]) > tol
        || fabs (pA->coff[0][3]) > tol
        || fabs (pA->coff[1][2]) > tol
        || fabs (pA->coff[1][3]) > tol
        || fabs (pA->coff[2][3]) > tol
        || fabs (pA->coff[2][0]) > tol
        || fabs (pA->coff[2][1]) > tol
        || fabs (pA->coff[3][0]) > tol
        || fabs (pA->coff[3][1]) > tol
        || fabs (pA->coff[3][2]) > tol
        )
        return false;

    /* We expect some +-ones */
    if (!isNearPMOne (pA->coff[2][2], tol))
        return false;
    if (!isNearPMOne (pA->coff[3][3], tol))
        return false;

    /* And we expect the leading 2x2 to be orthongonal */
    a00 = pA->coff[0][0];
    a01 = pA->coff[0][1];
    a10 = pA->coff[1][0];
    a11 = pA->coff[1][1];

    /* Check for near identity matrix as special case */
    if (   isNearPMOne (a00, tol)
        && isNearPMOne (a11, tol)
        && fabs (a01) <= tol
        && fabs (a10) <= tol
        )
        return true;

    if (   isNearPMOne (a00 * a00 + a01 * a10, tol)
        && isNearPMOne (a10 * a01 + a11 * a11, tol)
        && fabs        (a00 * a01 + a01 * a11) <= tol
        && fabs        (a10 * a00 + a11 * a10) <= tol
        )
        return true;
    return false;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlRotatedConic_areToriiIdentical                      |
|                                                                       |
| author        EarlinLutz                              07/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static bool       jmdlRotatedConic_areToriiIdentical

(
const RotatedConic  *pSurface0,     /* => first torus */
const RotatedConic  *pSurface1      /* => second torus */
)
    {
#define TORUS_ORIGIN 0
#define TORUS_PZ 1
#define TORUS_NZ 2
#define TORUS_X  3
#define TORUS_Y  4
#define GLOBAL_TEST_POINTS 5

    /* Check for same origin and +-Z directions. */
    static DPoint4d hLocalPoint[GLOBAL_TEST_POINTS] =
        {
            {0.0, 0.0,  0.0, 1.0},
            {0.0, 0.0,  1.0, 1.0},
            {0.0, 0.0, -1.0, 1.0},
            {1.0, 0.0,  0.0, 1.0},
            {0.0, 1.0,  0.0, 1.0}
        };

    DPoint3d point0[GLOBAL_TEST_POINTS], point1[GLOBAL_TEST_POINTS];
    DPoint4d hPoint0[GLOBAL_TEST_POINTS], hPoint1[GLOBAL_TEST_POINTS];


    double worldTol = pSurface0->tolerance > pSurface1->tolerance ? pSurface0->tolerance : pSurface1->tolerance;
    double worldTol2 = worldTol * worldTol;
    double localTol = 1.0e-6;
    double d0, d1, d2;
    DMatrix4d M10;
    int zDir = 0;

    bsiRotatedConic_transformDPoint4dArray (hPoint0, hLocalPoint, GLOBAL_TEST_POINTS, pSurface0, RC_COORDSYS_local, RC_COORDSYS_world);
    bsiRotatedConic_transformDPoint4dArray (hPoint1, hLocalPoint, GLOBAL_TEST_POINTS, pSurface1, RC_COORDSYS_local, RC_COORDSYS_world);
    bsiDPoint4d_normalizeArray (point0, hPoint0, GLOBAL_TEST_POINTS);
    bsiDPoint4d_normalizeArray (point1, hPoint1, GLOBAL_TEST_POINTS);

    d0 = point0[TORUS_ORIGIN].DistanceSquared (point1[TORUS_ORIGIN]);
    d1 = point0[TORUS_PZ].DistanceSquared (point1[TORUS_PZ]);
    d2 = point0[TORUS_NZ].DistanceSquared (point1[TORUS_PZ]); /* Yes, those are different indices */
    if (d0 > worldTol2)
        return false;

    if      (d1 < worldTol)
        zDir = 1;
    else if (d2 < worldTol)
        zDir = -1;
    else
        zDir = 0;

    if (zDir == 0)
        return false;

    bsiDMatrix4d_multiply (&M10, &pSurface0->rotationMap.M1, &pSurface1->rotationMap.M0);

    /* If we passed the z-axis test, apply a pretty loose tolerance on the rest of the local coordinates mappings. */
    return jmdlRotatedConic_isXYOrthogonal (&M10, localTol);
    }
/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiRotatedConic_intersectPlanarRotatedConic             |
|                                                                       |
| author        EarlinLutz                              07/97           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt    bsiRotatedConic_intersectPlanarRotatedConic

(
const RotatedConic  *pSurface0,     /* => first rotated conic */
const RotatedConic  *pSurface1,     /* => second rotated conic. Must be planar */
DPoint4dCP pEyePoint,       /* => optional eyepoint.  Intersection curve
has breaks at silhouette  points */
double      tolerance,      /* => chord tolerance */
SilhouetteArrayHandler handlerFunc, /* => callback for points */
void                *pUserData,      /* => arbitrary pointer */
bool                showDebug       /* => true to otutput debug geometry
(e.g. critical angle traces) */
)
    {
    DPoint4d planeCoffs;
    StatusInt status = ERROR;
    HConic conicCandidate[4];
    HConic clippedCandidate;
    int numCandidate;
    int candidate;

    if (bsiRotatedConic_isPlanar (&planeCoffs, pSurface1)
        && SUCCESS == bsiRotatedConic_intersectPlane (
                        conicCandidate, &numCandidate, pSurface0, &planeCoffs, true))
        {
        status = SUCCESS;
        for (candidate = 0; candidate < numCandidate; candidate++)
            {
            /* Clipping should either always succeed or always fail. Hence don't really have to
                worry about mixtures of fail and success */
            if (SUCCESS != bsiRotatedConic_clipHConicToTransverseSpaces (
                                &clippedCandidate, &conicCandidate[candidate], pSurface1))
                return ERROR;
            handlerFunc (&clippedCandidate, NULL, 0, 0, pSurface1, pUserData);
            }
        }
    return status;
    }

/*------------------------------------------------------------------*//**
* Classify a qudric surface by its diagonal characteristic.
* @param pIndex <= returned. For each standard form index i, pIndex[i]
*   is the corresponding column of the original data.
* @param pSigma <= values from pSigma0, sorted into standard form.
* @param pNegated <= true if a negative factor was applied to reach
*           standard form. (I.e. input had more negative than positive
*           entries.)
* @param pSigma0 => original eigenvalues of quadric form.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt    bsiRotatedConic_sortCharacteristic

(
int       *pIndex,
DPoint4dP pSigma,
QuadricSurfaceClass   *pClass,
bool      *pNegated,
DPoint4dCP pSigma0
)
    {
    int zeroIndex[4], numZero;
    int  posIndex[4], numPos;
    int  negIndex[4], numNeg;
    StatusInt status = ERROR;
    bool    classified = false;

    const double *pDiag = (const double*)pSigma0;
    int i;

    /* Find out where the current entries of each type are */
    numZero = numPos = numNeg = 0;
    for (i = 0; i < 4; i++)
        {
        if (pDiag[i] > 0.0)
            posIndex[numPos++] = i;
        else if (pDiag[i] < 0)
            negIndex[numNeg++] = i;
        else
            zeroIndex[numZero++] = i;
        }

    /* Find out if the whole thing has to be negated to make the positve-negative relationship right */
    if (numNeg > numPos)
        {
        int saveIndex[4];
        int numSave = numNeg;
        for (i = 0; i < numNeg; i++)
            saveIndex[i] = negIndex[i];
        for (i = 0; i < numPos; i++)
            negIndex[i] = posIndex[i];
        for (i = 0; i < numNeg; i++)
            posIndex[i] = saveIndex[i];
        numNeg = numPos;
        numPos = numSave;
        *pNegated = true;
        }
    else
        *pNegated = false;

    /* Look at all the special cases.  Setup to default to a null surface,
        but the case list should be exhaustive. */
    *pClass = QSC_NullSurface;
    pIndex[0] = 0;
    pIndex[1] = 1;
    pIndex[2] = 2;
    pIndex[3] = 3;

    if (numZero == 0)
        {
        if (numPos == 3)
            {
            *pClass = QSC_Sphere;
            pIndex[0] = posIndex[0];
            pIndex[1] = posIndex[1];
            pIndex[2] = posIndex[2];
            pIndex[3] = negIndex[0];
            classified = true;
            }
        else if (numPos == 2)
            {
            *pClass = QSC_Hyperbolic;
            pIndex[0] = posIndex[0];
            pIndex[1] = posIndex[1];
            pIndex[2] = negIndex[0];
            pIndex[3] = negIndex[1];
            classified = true;
            }
        else if (numPos == 4)
            {
            classified = true;
            }
        }
    else if (numZero == 1)
        {
        if (numPos == 2)
            {
            *pClass = QSC_Cylinder;
            pIndex[0] = posIndex[0];
            pIndex[1] = posIndex[1];
            pIndex[2] = zeroIndex[0];
            pIndex[3] = negIndex[0];
            classified = true;
            }
        }
    else if (numZero == 2)
        {
        if (numPos == 1)
            {
            *pClass = QSC_TwoPlanes;
            pIndex[0] = posIndex[0];
            pIndex[1] = zeroIndex[0];
            pIndex[2] = zeroIndex[1];
            pIndex[3] = negIndex[0];
            classified = true;
            }
        else if (numPos == 2)
            {
            *pClass = QSC_OneLine;
            pIndex[0] = posIndex[0];
            pIndex[1] = posIndex[1];
            pIndex[2] = zeroIndex[0];
            pIndex[3] = zeroIndex[1];
            classified = true;
            }
        }
    else if (numZero == 3)
        {
        if (numPos == 1)
            {
            *pClass = QSC_OnePlane;
            pIndex[0] = posIndex[0];
            pIndex[1] = zeroIndex[0];
            pIndex[2] = zeroIndex[1];
            pIndex[3] = zeroIndex[2];
            classified = true;
            }
        }
    else /* numZero == 4 */
        {
        classified = true;
        *pClass = QSC_AllPoints;
        }

    if (!classified)
        {
        *pSigma = *pSigma0;
        status = ERROR;
        }
    else
        {
        bsiDPoint4d_setComponents  (
                                pSigma,
                                pDiag[pIndex[0]],
                                pDiag[pIndex[1]],
                                pDiag[pIndex[2]],
                                pDiag[pIndex[3]]
                                );
        status = SUCCESS;
        }
    return status;
    }
#ifdef CompileAll
/*----------------------------------------------------------------------+
|                                                                       |
| name          outputHPlane                                            |
|                                                                       |
| author        EarlinLutz                              07/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static void outputHPlane

(
DPoint4dP pPoint0,
DPoint4dP pPoint1,
DPoint4dP pPoint2,
const RotatedConic  *pSurface,      /* => reference surface */
SilhouetteArrayHandler handlerFunc, /* => callback for points */
void                *pUserData      /* => arbitrary pointer */
)
    {
    DPoint4d hPoint[3];
    DPoint3d outputPoint[10];
    DPoint3d point00, point10, point01, point11;
    DPoint4d workPoint[4];
    HConic conic;
    int i, i0, i1, i2;
    double w, w0;
    hPoint[0] = *pPoint0;
    hPoint[1] = *pPoint1;
    hPoint[2] = *pPoint2;
    w0 = fabs (hPoint[0].w);
    i0 = 0;
    for (i = 1; i < 3; i++)
        {
        w = fabs (hPoint[i].w);
        if (w > w0)
            {
            w0 = w;
            i0 = i;
            }
        }

    if (w0 > 1.0e-5)
        {
#ifdef DEBUG
        extern void debug_displayLineSegment (DPoint3d *, DPoint3d *, int);
#endif
        double wTol = 1.0e-5 * w0;
        i1 = (i0 + 1) % 3;
        i2 = (i1 + 1) % 3;

        bsiDPoint4d_add2ScaledDPoint4d (&workPoint[0], &hPoint[i0], &hPoint[i1], 1.0, &hPoint[i2], 0.0);
        bsiDPoint4d_add2ScaledDPoint4d (&workPoint[1], &hPoint[i0], &hPoint[i1], 0.0, &hPoint[i2], 1.0);
        bsiDPoint4d_add2ScaledDPoint4d (&workPoint[2], &hPoint[i0], &hPoint[i1], 1.0, &hPoint[i2], 1.0);
        if (fabs (hPoint[i0].w) > wTol
            && fabs (workPoint[0].w) > wTol
            && fabs (workPoint[1].w) > wTol
            && fabs (workPoint[2].w) > wTol
            )
            {
            hPoint[i0].GetProjectedXYZ (point00);
            workPoint[0].GetProjectedXYZ (point10);
            workPoint[1].GetProjectedXYZ (point01);
            workPoint[2].GetProjectedXYZ (point11);

            outputPoint[0] = point01;
            outputPoint[1] = point00;
            outputPoint[2] = point10;
            outputPoint[3] = point11;
            outputPoint[4] = point01;

#ifdef DEBUG
            debug_displayLineSegment (&point00, &point10, 0);
            debug_displayLineSegment (&point00, &point01, 0);
            debug_displayLineSegment (&point01, &point10, 0);
#endif
            handlerFunc (NULL, outputPoint, 5, 0, pSurface, pUserData);

            bsiDEllipse4d_initFromDPoint4d (&conic.coordinates,
                            &hPoint[i0], &hPoint[i1], &hPoint[i2],
                            0.0, msGeomConst_2pi);
            conic.type = HConic_Ellipse;
            handlerFunc (&conic, NULL, 0, 0, pSurface, pUserData);
            }
        }
    }
 #endif
static void outputPlaneIntersection

(
const RotatedConic  *pSurface,      /* => reference surface */
DPoint4dCP pPlane,          /* => plane coefficients */
SilhouetteArrayHandler handlerFunc, /* => callback for points */
void                *pUserData      /* => arbitrary pointer */
)
    {
    HConic conic[4];
    int numConic;
    if (SUCCESS == bsiRotatedConic_intersectPlane (conic, &numConic, pSurface, pPlane, false))
        {
        int i;
        for (i = 0; i < numConic;i++)
            {
            handlerFunc (&conic[i], NULL, 0, 0, pSurface, pUserData);
            }
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiRotatedConic_intersectEllipsoids                     |
|                                                                       |
| author        EarlinLutz                              07/97           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt    bsiRotatedConic_intersectEllipsoids

(
const RotatedConic  *pSurface0,     /* => first rotated conic */
const RotatedConic  *pSurface1,     /* => second rotated conic */
DPoint4dCP pEyePoint,       /* => optional eyepoint.  Intersection curve
has breaks at silhouette  points */
double      tolerance,      /* => chord tolerance */
SilhouetteArrayHandler handlerFunc, /* => callback for points */
void                *pUserData,      /* => arbitrary pointer */
bool                showDebug       /* => true to otutput debug geometry
(e.g. critical angle traces) */
)
    {
    DMatrix4d B2[4], B2inv[4], B2invT;
    DPoint4d sigma2[4];
    int numSingularSurface;
    DPoint4d sigma0, sigma1;
    int i;
    StatusInt status = ERROR;

    bsiDPoint4d_setComponents (&sigma0, 1.0, 1.0, 1.0, -1.0);
    bsiDPoint4d_setComponents (&sigma1, 1.0, 1.0, 1.0, -1.0);


    bsiDMatrix4d_searchSingularPencil (sigma2, B2, B2inv, &numSingularSurface,
                &sigma0,
                &pSurface0->rotationMap.M0,
                &pSurface0->rotationMap.M1,
                &sigma1,
                &pSurface1->rotationMap.M0,
                &pSurface1->rotationMap.M1
                );
#define NORMALDRAW 0
    for (i = 0; i < numSingularSurface && status == ERROR; i++)
        {
        DPoint4d sigma;
        int type;
        int index[4];
        bool    negated;
        DPoint4d hPoint[4];
        DPoint4d invHPoint[4];
        DPoint4d planeCoffs[2];
        DPoint4d planePoint[2];
        RotatedConic surface2;
        DMap4d map2;
        int j;
        bsiDMatrix4d_transpose (&B2invT, &B2inv[i]);

        if (  SUCCESS == bsiRotatedConic_sortCharacteristic (index, &sigma, (QuadricSurfaceClass *) &type, &negated, &sigma2[i])
           )
            {

            for (j = 0; j < 4; j++)
                {
                bsiDMatrix4d_getColumnDPoint4d (&B2[i], &hPoint[j], index[j]);
                bsiDMatrix4d_getColumnDPoint4d (&B2invT, &invHPoint[j], index[j]);
                }

            switch (type)
                {
                case QSC_OnePlane:
#ifdef EXTRA_PLANE_OUTPUT
                    outputHPlane (&hPoint[3], &hPoint[1], &hPoint[2], pSurface0, handlerFunc, pUserData);
#endif
                    outputPlaneIntersection (pSurface0, &hPoint[0], handlerFunc, pUserData);
                    status = SUCCESS;
                    break;
                case QSC_TwoPlanes:
                    {
                    bsiDPoint4d_add2ScaledDPoint4d (&planeCoffs[0], NULL, &invHPoint[0], 1.0, &invHPoint[3],  1.0);
                    bsiDPoint4d_add2ScaledDPoint4d (&planeCoffs[1], NULL, &invHPoint[0], 1.0, &invHPoint[3], -1.0);

                    bsiDPoint4d_add2ScaledDPoint4d (&planePoint[0], NULL, &hPoint[0], 1.0, &hPoint[3],  1.0);
                    bsiDPoint4d_add2ScaledDPoint4d (&planePoint[1], NULL, &hPoint[0], 1.0, &hPoint[3], -1.0);

#ifdef EXTRA_PLANE_OUTPUT
                    outputHPlane (&planePoint[0], &hPoint[1], &hPoint[2], pSurface0, handlerFunc, pUserData);
                    outputHPlane (&planePoint[1], &hPoint[1], &hPoint[2], pSurface0, handlerFunc, pUserData);
#endif
                    outputPlaneIntersection (pSurface0, &planeCoffs[0], handlerFunc, pUserData);
                    outputPlaneIntersection (pSurface0, &planeCoffs[1], handlerFunc, pUserData);
                    status = SUCCESS;
                    }
                    break;
                case QSC_OnePoint:
                    break;
                case QSC_Cylinder:
                    {
                    bsiDMap4d_initFromIndexedMatrices (&map2, &B2[i], &B2inv[i], index);
                    bsiRotatedConic_initFrameAndSweep (
                                        &surface2,
                                        RC_Cylinder,
                                        &map2,
                                        0.0,
                                        msGeomConst_2pi,
                                        -100.0,
                                        100.0
                                        );
                    bsiRotatedConic_intersectRotatedConicExt (
                                        &surface2,
                                        pSurface1,
                                        pSurface0,
                                        pSurface1,
                                        pEyePoint,
                                        tolerance,
                                        handlerFunc,
                                        pUserData,
                                        showDebug
                                        );
                    status = SUCCESS;
                    }
                    break;
                case QSC_OneLine:
                    break;
                }
            }
        }

    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiRotatedConic_intersectRotatedConic                   |
|                                                                       |
| author        EarlinLutz                              07/97           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt    bsiRotatedConic_intersectRotatedConic

(
const RotatedConic  *pSurface0,         /* => first rotated conic  */
const RotatedConic  *pSurface1,         /* => second rotated conic */
DPoint4dCP pEyePoint,           /* => optional eyepoint.  Intersection curve
has breaks at silhouette  points */
double      tolerance,          /* => chord tolerance */
SilhouetteArrayHandler handlerFunc,     /* => callback for points */
void                *pUserData,         /* => arbitrary pointer */
bool                showDebug           /* => true to otutput debug geometry (e.g. critical angle traces) */
)
    {
    return bsiRotatedConic_intersectRotatedConicExt (
                        pSurface0,
                        pSurface1,
                        pSurface0,
                        pSurface1,
                        pEyePoint,
                        tolerance,
                        handlerFunc,
                        pUserData,
                        showDebug
                        );
    }
/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiRotatedConic_intersectRotatedConicExt                |
|                                                                       |
| author        EarlinLutz                              07/97           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt    bsiRotatedConic_intersectRotatedConicExt

(
const RotatedConic  *pSurface0,     /* => first rotated conic  */
const RotatedConic  *pSurface1,     /* => second rotated conic */
const RotatedConic  *pClipSurface0, /* => first clip surface (commonly the same as pSurface0)  */
const RotatedConic  *pClipSurface1, /* => second clip surface (commonly the same as pSurface1) */
const DPoint4d      *pEyePoint,     /* => optional eyepoint.  Intersection curve
                                            has breaks at silhouette  points */
      double        tolerance,      /* => chord tolerance */
SilhouetteArrayHandler handlerFunc, /* => callback for points */
void                *pUserData,      /* => arbitrary pointer */
bool                showDebug       /* => true to otutput debug geometry
                                            (e.g. critical angle traces) */
)
    {
    static int enableQSIC = true;
    StatusInt status = ERROR;
    int type0 = pSurface0->type;
    int type1 = pSurface1->type;
    DPoint4d  sigma0, sigma1;
    bool    surface0IsQuadric, surface1IsQuadric;
    bool    surface0IsPlanar, surface1IsPlanar;
    DPoint4d plane0Coffs, plane1Coffs;


    DMap4d hMap0to1, hMap1to0;

    if (!enableQSIC)
        return ERROR;

    surface0IsPlanar  = bsiRotatedConic_isPlanar (&plane0Coffs, pSurface0);
    surface1IsPlanar  = bsiRotatedConic_isPlanar (&plane1Coffs, pSurface1);

    if (surface0IsPlanar || surface1IsPlanar)
        {
        if (surface0IsPlanar && !surface1IsPlanar)
            {
            return bsiRotatedConic_intersectPlanarRotatedConic (
                                pSurface1, pSurface0,
                                pEyePoint, tolerance, handlerFunc,
                                pUserData, showDebug
                                );
            }
        else if (!surface0IsPlanar && surface1IsPlanar)
            {
            return bsiRotatedConic_intersectPlanarRotatedConic (
                                pSurface0, pSurface1,
                                pEyePoint, tolerance, handlerFunc,
                                pUserData, showDebug
                                );
            }
        else
            {
            return bsiRotatedConic_intersectPlanarRotatedConic (
                                pSurface0, pSurface1,
                                pEyePoint, tolerance, handlerFunc,
                                pUserData, showDebug
                                );
            }
        }

    bsiRotatedConic_mappedBasis (&hMap0to1, pSurface0, pSurface1);
    bsiRotatedConic_mappedBasis (&hMap1to0, pSurface1, pSurface0);

    surface0IsQuadric = bsiRotatedConic_isQuadric (&sigma0, pSurface0);
    surface1IsQuadric = bsiRotatedConic_isQuadric (&sigma1, pSurface1);



    if (surface0IsQuadric && surface1IsQuadric)
        {
        /* We have two quadrics */
        if (type0 == RC_Cylinder && type1 == RC_Cylinder)
            {
            QSIC_Params param01,  param10;
            QSIC_CaseDescriptor case01, case10;

            param01.pSurface0 = pSurface0;
            param01.pSurface1 = pSurface1;
            param01.sigma0 = sigma0;
            param01.sigma1 = sigma1;
            param01.hMap0to1 = hMap0to1;
            param01.hMap1to0 = hMap1to0;

            param10.pSurface0 = pSurface1;
            param10.pSurface1 = pSurface0;
            param10.sigma0 = sigma1;
            param10.sigma1 = sigma0;
            param10.hMap0to1 = hMap1to0 ;
            param10.hMap1to0 = hMap0to1;

            param01.handlerFunc = handlerFunc;
            param01.pUserData   = pUserData;

            param10.handlerFunc = handlerFunc;
            param10.pUserData   = pUserData;
            param01.tolerance = param10.tolerance = tolerance;

            case01 = jmdlQSIC_unitCylinderQuadricSpecialCaseTest (pSurface0, pSurface1);
            case10 = jmdlQSIC_unitCylinderQuadricSpecialCaseTest (pSurface1, pSurface0);

            if (case01 == QSIC_CoincidentSurfaces || case10 == QSIC_CoincidentSurfaces)
                {
                /* Conicident surfaces -- return as empty intersection */
                status = SUCCESS;
                }
            else if (case01 == QSIC_CompleteAngularRange)
                {
                status = jmdlQSIC_unitCylinderQuadricIntersection
                                (
                                pSurface0,
                                pSurface1,
                                pClipSurface0,
                                pClipSurface1,
                                pEyePoint,
                                jmdlQSIC_UCQI_output,
                                &param01,
                                showDebug
                                );
                }
            else if (case10 == QSIC_CompleteAngularRange)
                {
                status = jmdlQSIC_unitCylinderQuadricIntersection
                                (
                                pSurface1,
                                pSurface0,
                                pClipSurface0,
                                pClipSurface1,
                                pEyePoint,
                                jmdlQSIC_UCQI_output,
                                &param10,
                                showDebug
                                );
                }
            else
                {
                /* Both ranges incomplete.  Arbitrarily choose case01 */
                status = jmdlQSIC_unitCylinderQuadricIntersection
                                (
                                pSurface0,
                                pSurface1,
                                pClipSurface0,
                                pClipSurface1,
                                pEyePoint,
                                jmdlQSIC_UCQI_output,
                                &param01,
                                showDebug
                                );
                }

            }
        else if (type0 == RC_Cylinder)
            {
            QSIC_Params         intParams;
            intParams.pSurface0 = pSurface0;
            intParams.pSurface1 = pSurface1;
            intParams.sigma0 = sigma0;
            intParams.sigma1 = sigma1;
            intParams.hMap0to1 = hMap0to1;
            intParams.hMap1to0 = hMap1to0;
            intParams.handlerFunc = handlerFunc;
            intParams.pUserData   = pUserData;
            intParams.tolerance = tolerance;

            status = jmdlQSIC_unitCylinderQuadricIntersection
                                (
                                pSurface0,
                                pSurface1,
                                pClipSurface0,
                                pClipSurface1,
                                pEyePoint,
                                jmdlQSIC_UCQI_output,
                                &intParams,
                                showDebug
                                );
            }
        else if (type1 == RC_Cylinder)
            {
            QSIC_Params         intParams;
            intParams.pSurface0 = pSurface1;
            intParams.pSurface1 = pSurface0;
            intParams.sigma0 = sigma1;
            intParams.sigma1 = sigma0;
            intParams.hMap0to1 = hMap1to0 ;
            intParams.hMap1to0 = hMap0to1;
            intParams.handlerFunc = handlerFunc;
            intParams.pUserData   = pUserData;
            intParams.tolerance = tolerance;
            status = jmdlQSIC_unitCylinderQuadricIntersection
                                (
                                pSurface1,
                                pSurface0,
                                pClipSurface0,
                                pClipSurface1,
                                pEyePoint,
                                jmdlQSIC_UCQI_output,
                                &intParams,
                                showDebug
                                );
            }
        else if (type0 == RC_Sphere && type1 == RC_Sphere)
            {
            static bool    searchForCone = true;
            if (searchForCone)
                status = bsiRotatedConic_intersectEllipsoids (pSurface0, pSurface1,
                                    pEyePoint, tolerance, handlerFunc, pUserData, showDebug);
            }

        }
    else if (type0 == RC_Torus && type1 == RC_Torus)
        {
        if (jmdlRotatedConic_areToriiIdentical (pSurface0, pSurface1))
            {
            status = SUCCESS;
            }
        else
            {
            status = ERROR;
            }
        }
    return status;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
