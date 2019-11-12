 /*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

// EDL Oct 2014
// all arrays are required.
// move excludeEndPoint logic to caller.
static void extendAngleArray
(
double          *pCosArray,
double          *pSinArray,
double          *pAngleArray,
int             &numOut,
const double    *pParamArray,
int             numParam,
int             maxOut,
bool            negateSine
)
    {
    int i;
    double u, v, b0, b1, b2, sinPhi, cosPhi;
    double aa = negateSine ? -2.0 : 2.0;
    double w;

    for (i = 0; i < numParam && numOut < maxOut; i++)
        {
        u = pParamArray[i];
        v = 1.0 - u;
        b0 = v * v;
        b1 = aa * u * v;
        b2 = u * u;
        cosPhi = b0 - b2;
        sinPhi = b1;
        w      = b0 + b2;

        pCosArray[numOut] = cosPhi / w;
        pSinArray[numOut] = sinPhi / w;
        pAngleArray[numOut] = atan2 (sinPhi, cosPhi);
        numOut += 1;
        }
    }


static RotMatrix s_matrixC
   = {{
         {1.0, 0.0, -1.0},
         {0.0, 1.0,  0.0},
         {1.0, 0.0,  1.0}
    }};

static RotMatrix s_matrixCT
   = {{
        { 1.0,  0.0,  1.0},
        { 0.0,  1.0,  0.0},
        {-1.0,  0.0,  1.0}
    }};


/**
* This routine finds the solutions of a quartic bezier expressed
* as a quadric form over quadratic bezier basis functions, i.e.
* f(u) = sum A[i][j]*B[i](u)*B[j](u)
* where sums are over 0..2, A[i][j] is a coefficient from the matrix,
* and B[i](u) is the i'th quadratic bezier basis function at u.
* Returns  : number of intersections found.
*
* @param pCosArray <= x coordinates of intersections
* @param pSinArray <= y coordinates of intersections
* @param pAngleArray <= angular positions of intersections
* @param pNumInt <= number of intersections
* @param pCoff0 <= array of bezier coefficients for upper half circle.
* @param pCoff1 <= array of bezier coefficients for lower half circle.
* @param pA => matrix defining implicit conic
* @return -1 if matrix pA is (exactly) a unit circle,
*               else number of intersections.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiBezier_solveTrigForm
(
double      *pCosArray,
double      *pSinArray,
double      *pAngleArray,
int         *pNumAngle,
double      *pCoff0,
double      *pCoff1,
RotMatrixCP  pA
)
    {
    double coff0[5];
    double coff1[5];
    double root0[5];
    double root1[5];
    int num0, num1;
    int numAngles;


    /* Compute coefficients of the quartic equation */
    coff0[0] =  pA->form3d[0][0];
    coff0[1] = (pA->form3d[1][0] + pA->form3d[0][1]) * 0.5;
    coff0[2] = (pA->form3d[2][0] + 4.0 * pA->form3d[1][1] + pA->form3d[0][2]) / 6.0;
    coff0[3] = (pA->form3d[1][2] + pA->form3d[2][1]) * 0.5;
    coff0[4] =  pA->form3d[2][2];

    coff1[0] =  coff0[0];
    coff1[1] = -coff0[1];
    coff1[2] =  coff0[2];
    coff1[3] = -coff0[3];
    coff1[4] =  coff0[4];

    if (pCoff0)
        memcpy (pCoff0, coff0, 5 * sizeof (double));
    if (pCoff1)
        memcpy (pCoff1, coff1, 5 * sizeof (double));
    num0 = 0;
    num1 = 0;
    numAngles = 0;

    double cosArray[10], sinArray[10], angleArray[10];
    int maxOut = 10;

    if (    bsiBezier_univariateRoots (root0, &num0, coff0, 5)
        &&  bsiBezier_univariateRoots (root1, &num1, coff1, 5))
        {
        if (num0 > 4 || num1 > 4)
            {
            /* The coefficient matrix looks like a unit circle itself !!! */
            *pNumAngle = 0;
            return -1;
            }
        else
            {
            extendAngleArray (cosArray, sinArray, angleArray, numAngles,
                                root0, num0, maxOut, false);
            extendAngleArray (cosArray, sinArray, angleArray, numAngles,
                                root1, num1, maxOut, true);
            }
        }

    // The two solutions might have overlapped at the joints.
    // Copy to output, using toleranced test to throw out those possible duplicates.
    // but only do this at 0 and 180.  In between don't second guess the multiplicities in the solver.
    static double s_trigTolerance = 1.0e-15;
    int numAccept = 0;
    for (int candidate = 0; candidate < numAngles; candidate++)
        {
        bool accept = true;
        if (fabs (sinArray[candidate]) < s_trigTolerance)
            {
            for (int i = 0; i < numAccept; i++)
                if (DoubleOps::AlmostEqual (cosArray[candidate], cosArray[i], s_trigTolerance)
                    && DoubleOps::AlmostEqual (sinArray[candidate], sinArray[i], s_trigTolerance))
                    {
                    accept = false;
                    break;
                    }
            }
        if (accept)
            {
            cosArray[numAccept] = cosArray[candidate];
            sinArray[numAccept] = sinArray[candidate];
            angleArray[numAccept] = angleArray[candidate];
            numAccept++;
            }
        }
    
    if (numAccept > 4)
        numAccept = 4;          // really should not happen but guard against funny numerics
    if (pNumAngle)
        *pNumAngle = numAccept;
    
    for (int i = 0; i < numAccept; i++)
        {
        if (pCosArray)
            pCosArray[i] = cosArray[i];
        if (pSinArray)
            pSinArray[i] = sinArray[i];
        if (pAngleArray)
            pAngleArray[i] = angleArray[i];
        }

    return  numAccept;
    }



/**
* This routine finds the points of intersection between an implicit
* conic (specified by matrix A) X^AX = 0  and the unit circle
* x^2 + Y^2 = 1
* Returns  : number of intersections found.
*            -1: conic = circle or input error or polynomial solver failed.
*
* @param pCosArray <= x coordinates of intersections in unit circle system
* @param pSinArray <= y coordinates of intersections in unit circle system
* @param pAngleArray <= angular positions of intersections  in unit circle
* @param pNumAngle <= number of intersections. (0 to 4)
* @param pCoefficientMatrix => matrix defining implicit conic
* @return -1 if matrix pA is (exactly) a unit circle,
*               else number of intersections.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int        bsiBezier_implicitConicIntersectUnitCircle
(
double     *pCosArray,
double     *pSinArray,
double     *pAngleArray,
int        *pNumAngle,
double     *pCoff0,
double     *pCoff1,
RotMatrixCP pCoefficientMatrix
)
    {
    RotMatrix matrixB, matrixA;

    matrixA = *pCoefficientMatrix;
    matrixB.InitProduct (s_matrixCT, matrixA);
    matrixB.InitProduct (matrixB, s_matrixC);

    return bsiBezier_solveTrigForm
                    (
                    pCosArray, pSinArray,
                    pAngleArray, pNumAngle,
                    pCoff0, pCoff1,
                    &matrixB
                    );
    }


/**
* This routine finds the points of intersection between an implicit
* conic (specified by matrix A) X^AX = 0  and the unit circle
* x^2 + Y^2 = 1
* Returns  : number of intersections found.
*            -1: conic = circle or input error or polynomial solver failed.
*
* @param pCosArray <= x coordinates of intersections in unit circle system
* @param pSinArray <= y coordinates of intersections in unit circle system
* @param pAngleArray <= angular positions of intersections  in unit circle
* @param pNumAngle <= number of intersections. (0 to 4)
* @param pCoefficientMatrix => matrix defining implicit conic
* @return -1 if matrix pA is (exactly) a unit circle,
*               else number of intersections.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int    bsiBezier_implicitConicIntersectUnitCircleShort
(
double     *pCosArray,
double     *pSinArray,
double     *pAngleArray,
int        *pNumAngle,
RotMatrixCP pCoefficientMatrix
)
    {
    RotMatrix matrixB, matrixA;


    matrixA = *pCoefficientMatrix;
    matrixB.InitProduct (s_matrixCT, matrixA);
    matrixB.InitProduct (matrixB, s_matrixC);

    return bsiBezier_solveTrigForm
                    (
                    pCosArray, pSinArray,
                    pAngleArray, pNumAngle,
                    NULL, NULL,
                    &matrixB
                    );
    }

/*
* Map from trig parameter to bezier and do intersection calculations.
*/
static int      bsiBezier_conicIntersectUnitCircle_go
(
double          *pCosArray,
      double    *pSinArray,
      double    *pAngleArray,
      int       *pNumAngle,
      double    *pCoff0,
      double    *pCoff1,
const RotMatrix *pB
)
    {
    RotMatrix BC, BCT;
    RotMatrix matrixA;


    BC.InitProduct (*pB, s_matrixC);
    BCT.TransposeOf (BC);
    BCT.ScaleColumns (1.0, 1.0, -1.0);
    matrixA.InitProduct (BCT, BC);

    return bsiBezier_solveTrigForm
                    (
                    pCosArray, pSinArray,
                    pAngleArray, pNumAngle,
                    pCoff0, pCoff1,
                    &matrixA
                    );
    }


/*
* Apply a rotation to the conic parameter space and compute conic circle intersections.
* @param applyShift => if true, an angle is supplied.
* @param theta => shift angle.  Ellipse parameterization is rotated so calculation use
*       angle (alpha-theta) where alpha is the original angle.
* @param cc => cosine (theta).  (Redundant, we trust you.)
* @param ss => sine (theta0 (ditto)
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static int      bsiBezier_conicIntersectUnitCircle_rotateAndGo
(
double          *pCosArray,
      double    *pSinArray,
      double    *pAngleArray,
      int       *pNumAngle,
      double    *pCoff0,
      double    *pCoff1,
const RotMatrix *pB,
      bool      applyShift,
      double    theta,
      double    cc,
      double    ss
)
    {

    if (!applyShift)
        {
        return bsiBezier_conicIntersectUnitCircle_go
                        (
                        pCosArray,
                        pSinArray,
                        pAngleArray,
                        pNumAngle,
                        pCoff0,
                        pCoff1,
                        pB
                        );
        }
    else
        {
        RotMatrix BB;
        int result;
        int i;
        int numAngle;
        double ci, si;
        DVec3d column0 = DVec3d::FromColumn (*pB, 0);
        DVec3d column1 = DVec3d::FromColumn (*pB, 1);
        DVec3d column2 = DVec3d::FromColumn (*pB, 2);
        DVec3d resultColumn0, resultColumn1;
        resultColumn0.SumOf (column0, cc, column1, ss);
        resultColumn1.SumOf (column0, -ss, column1, cc);
        BB.InitFromColumnVectors (resultColumn0, resultColumn1, column2);
        result = bsiBezier_conicIntersectUnitCircle_go
                        (
                        pCosArray,
                        pSinArray,
                        pAngleArray,
                        &numAngle,
                        pCoff0,
                        pCoff1,
                        &BB
                        );
        if (pNumAngle)
            *pNumAngle = numAngle;
        for (i = 0; i < numAngle; i++)
            {
            if (pAngleArray)
                pAngleArray[i] += theta;
            if (pCosArray && pSinArray)
                {
                ci = pCosArray[i];
                si = pSinArray[i];
                pCosArray[i] = ci * cc - si * ss;
                pSinArray[i] = ci * ss + si * cc;
                }
            }
        return result;
        }

    }


/**
* This routine finds the points of intersection between a parametric
* conic specified as the (2d, homogeneous) points
*   B.column[0]*cos(theta) + B.column[1]*sin(theta) + B.column[2]
*  and the unit circle
*       X^2 + Y^2 = W^2
* Returns  : number of intersections found.
*            -1: conic = circle or input error or polynomial solver failed.
*
* @param pCosArray <= cosine coordinates of intersections in conic frame
* @param pSinArray <= sine coordinates of intersections in conic frame
* @param pAngleArray <= angular positions of intersections in conic parameter space
* @param pNumAngle <= number of intersections
* @param pB => matrix defining parametric conic.
* @return -1 if pB is (exactly) a unit circle,
*               else number of intersections.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int        bsiBezier_conicIntersectUnitCircle
(
double     *pCosArray,
double     *pSinArray,
double     *pAngleArray,
int        *pNumAngle,
double     *pCoff0,
double     *pCoff1,
RotMatrixCP pB
)
    {
    int i;
    double dtheta = 0.12528394;
    double theta, cc, ss;
    DPoint3d xyw0, xyw1;
    double xxyy, ww;
    double hh;
//    static double s_bigTol = 1.0e-6;
    static double s_smallTol = 1.0e-12;
    double f0, f1, ff;
    // Find an origin angle which makes the 0 and 180 degree points
    // clearly off of the unit circle.
    for (i = 0; i < 8; i++)
        {
        theta = i * dtheta;
        cc = cos (theta);
        ss = sin (theta);
        pB->MultiplyComponents (xyw0,  cc,  ss, 1.0);
        pB->MultiplyComponents (xyw1, -cc, -ss, 1.0);

        xxyy = xyw0.x * xyw0.x + xyw0.y * xyw0.y;
        ww   = xyw0.z * xyw0.z;
        hh = xxyy + ww;
        f0 = fabs (xxyy - ww);

        xxyy = xyw1.x * xyw1.x + xyw1.y * xyw1.y;
        ww   = xyw1.z * xyw1.z;

        hh += xxyy + ww;
        f1 = fabs (xxyy - ww);

        ff = f0 < f1 ? f0 : f1;
        if (ff > s_smallTol * hh)
            {
            return bsiBezier_conicIntersectUnitCircle_rotateAndGo
                    (pCosArray, pSinArray, pAngleArray, pNumAngle,
                    pCoff0, pCoff1, pB, i > 0, theta, cc, ss);
            }
        }

    if (pNumAngle)
        *pNumAngle = 0;
    return false;
    }
END_BENTLEY_GEOMETRY_NAMESPACE