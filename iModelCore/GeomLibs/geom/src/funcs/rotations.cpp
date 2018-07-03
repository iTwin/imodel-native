/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/rotations.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*----------------------------------------------------------------------+
|                                                                       |
| This file contains functions for generalized "rotations" used in      |
| linear algebra.  These include                                        |
| -- Givens rotation -- one sided rotation used in QR factorization.    |
| -- Jacobi rotation -- two sided rotation used in symmetric            |
|       eigensystems.                                                   |
| -- Hyperbolic rotation -- one sided rotation used to preserve         |
|       signs of characteristic matrices.                               |
|                                                                       |
+----------------------------------------------------------------------*/



/*-----------------------------------------------------------------*//**
* @param pA <= modified x part of vector
* @param pB <= modified y part of vector
* @param a => x part of vector
* @param b => y part of vector
* @param cosine <= cosine of givens angle
* @param sine <= sine of givens angle
* @see
* @bsihdr                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiTrig_constructHyperbolicWeights

(
double      *pSecant,
double      *pTangent,
double      a,
double      b
)
    {
    double cc = a * a - b * b;
    double c;

    if (cc <= 0.0)
        {
        *pSecant = 1.0;
        *pTangent = 0.0;
        return false;
        }
    else
        {
        c = sqrt (cc);
        if (c < bsiTrig_smallAngle() * fabs (a))
            {
            *pSecant = 1.0;
            *pTangent = 0.0;
            return false;
            }
        else
            {
            *pSecant  = a / c;
            *pTangent = - b / c;
            return true;
            }
        }
    }


/*-----------------------------------------------------------------*//**
* @param pA <= modified x part of vector
* @param pB <= modified y part of vector
* @param a => x part of vector
* @param b => y part of vector
* @param cosine <= cosine of givens angle
* @param sine <= sine of givens angle
* @see
* @bsihdr                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiTrig_applyHyperbolicWeights

(
double      *pA,
double      *pB,
double      a,
double      b,
double      secant,
double      tangent
)
    {
    *pA = a * secant + b * tangent;
    *pB = a * tangent + b * secant;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiTrig_solve3x2InOrder                         |
|                                                                       |
| author        EarlinLutz                              08/96           |
|                                                                       |
| Solve a 3x2 (least squares) system, assuming A0 is nonzero and of     |
| larger magnitude than A1.                                             |
+----------------------------------------------------------------------*/
static int  bsiTrig_solve3x2InOrder   /* matrix rank deficiency .  0 means A is full rank.*/
                                /* 1 means columns of A are either parallel or one is zero.*/
                                /* 2 means A has nothing but zeros.*/

(
double  *pS0,           /* <= 1st solution parameter*/
double  *pS1,           /* <= 2nd solution parameter*/
DPoint3dCP pA0,    /* => First column of matrix*/
DPoint3dCP pA1,    /* => Second column of matrix*/
DPoint3dCP pB      /* => right hand side*/
)
    {
    DPoint3d A0, A1, B;
    int rankDeficiency = 0;
    static double diagonalRatioTol = 1.0e-14;
    double cc, ss;      /* Givens rotations trig values.*/

    A0 = *pA0;
    A1 = *pA1;
    B  = *pB;

    if (fabs (A0.y) > diagonalRatioTol * fabs (A0.x))
        {
        bsiTrig_constructGivensWeights (&cc, &ss, A0.x, A0.y);
        //bsiTrig_applyGivensWeights (&A0.x, &A0.y, A0.x, A0.y, cc, ss);
        A0.y = 0.0;
        bsiTrig_applyGivensWeights (&A1.x, &A1.y, A1.x, A1.y, cc, ss);
        bsiTrig_applyGivensWeights ( &B.x,  &B.y,  B.x,  B.y, cc, ss);
        }

    if (fabs (A0.z) > diagonalRatioTol * fabs (A0.x))
        {
        bsiTrig_constructGivensWeights (&cc, &ss, A0.x, A0.z);
        //bsiTrig_applyGivensWeights (&A0.x, &A0.z, A0.x, A0.z, cc, ss);
        A0.z = 0.0;
        bsiTrig_applyGivensWeights (&A1.x, &A1.z, A1.x, A1.z, cc, ss);
        bsiTrig_applyGivensWeights ( &B.x,  &B.z,  B.x,  B.z, cc, ss);
        }

    if (fabs (A1.z) > diagonalRatioTol * fabs (A0.x))
        {
        bsiTrig_constructGivensWeights (&cc, &ss, A1.y, A1.z);
        //bsiTrig_applyGivensWeights (&A1.y, &A1.z, A1.y, A1.z, cc, ss);
        A1.z = 0.0;
        bsiTrig_applyGivensWeights ( &B.y,  &B.z,  B.y,  B.z, cc, ss);
        }

    if (A0.x == 0.0)
        {
        /* This is not supposed to happen.  Call it a disaster.*/
        rankDeficiency = 2;
        *pS0 = *pS1 = 0.0;
        }
    else if (fabs (A1.y) < diagonalRatioTol * fabs (A0.x))
        {
        rankDeficiency = 1;
        /* Just project onto the original A vector.*/
        *pS1 = 0.0;
        *pS0 = pB->DotProduct (*pA0) / pA0->DotProduct (*pA0);
        }
    else
        {
        *pS1 = B.y / A1.y;
        *pS0 = (B.x - A1.x * (*pS1)) / A0.x;
        }

    return rankDeficiency;
    }


/*---------------------------------------------------------------------------------**//**
* Apply a matrix to sine and cosine of a single angle, and return the mapped trig functions and
* angle.
*
* @param    pMappedTrigFuncs <= (optional) vector of weighted cosine, weighted sine, weight values.
* @param    pPhi             <= (optional) mapped angle.
* @param    pDerivNumerator  <= (optional) numerator of ratio form of derivative
* @param    pDirivDenominator <= (optional) denominator of ratio form of derivative
* @bsimethod                                                    EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiTrig_mapAngle

(
DPoint3dP pMappedTrigFuncs,
double    *pPhi,
double    *pDerivNumerator,
double    *pDerivDenominator,
RotMatrixCP pMatrix,
double    theta
)
    {
    double c = cos (theta);
    double s = sin (theta);
    DPoint3d    F;
    pMatrix->MultiplyComponents(F, c, s, 1.0);

    if (pMappedTrigFuncs)
        *pMappedTrigFuncs = F;
    if (pPhi)
        *pPhi = Angle::Atan2 (F.y, F.x);

    if (pDerivNumerator || pDerivDenominator)
        {
        DPoint3d dF;
        pMatrix->MultiplyComponents(dF, -s, c, 0.0);

        if (pDerivNumerator)
            *pDerivNumerator = dF.y * F.x - dF.x * F.y;

        if (pDerivDenominator)
            *pDerivDenominator = F.x * F.x + F.y * F.y;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Apply a matrix to sine and cosine of two angles, and return the corresponding
* mapped angles.
*
* @bsimethod                                                    EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiTrig_mapLimits

(
double    *pPhi0,
double    *pPhi1,
RotMatrixCP pMatrix,
double    theta0,
double    theta1
)
    {
    double dTheta = theta1 - theta0;
    double phi0, phi1, orientation;

    bsiTrig_mapAngle (NULL, &phi0, &orientation, NULL, pMatrix, theta0);

    if (Angle::IsFullCircle (dTheta))
        {
        /* Should be easy to map a full circle.  Tis not so -- "identical"
            start and end angles frustrate direction tests.  Work with an artificial
            theta1 to get a direction.
        */
        phi1 = phi0 + (orientation >= 0.0 ? msGeomConst_2pi : - msGeomConst_2pi);
        }
    else
        {
        bsiTrig_mapAngle (NULL, &phi1, NULL, NULL, pMatrix, theta1);

        if (orientation * (phi1 - phi0) < 0.0)
            phi1 = bsiTrig_complementaryAngle (phi1);
        }

    if (pPhi0)
        *pPhi0 = phi0;

    if (pPhi1)
        *pPhi1 = phi1;
    }



/*-----------------------------------------------------------------*//**
* Compute the least-squares solution to the overdetermined problem AX = B
* where A is 3 rows and 2 columns and B is one column.
*
* @param pS0 <= 1st solution parameter
* @param pS1 <= 2nd solution parameter
* @param pA0 => First column of matrix
* @param pA1 => Second column of matrix
* @param pB => right hand side
* @see
* @return rank deficiency of A.
*                   0 means A is full rank, i.e. normal solution.
*                   1 means columns of A are either parallel or one is zero.
*                   2 means A has nothing but zeros.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiTrig_solve3x2

(
double  *pS0,
double  *pS1,
DPoint3dCP pA0,
DPoint3dCP pA1,
DPoint3dCP pB
)
    {
    double mag0, mag1;
    int rankDeficiency = 0;

    mag0 = pA0->DotProduct (*pA0);
    mag1 = pA1->DotProduct (*pA1);

    if (mag0 == 0.0 && mag1 == 0.0)
        {
        *pS0 = *pS1 = 0.0;
        rankDeficiency = 2;
        }
    else if (mag0 > mag1)
        {
        rankDeficiency = bsiTrig_solve3x2InOrder (pS0, pS1, pA0, pA1, pB);
        }
    else
        {
        rankDeficiency = bsiTrig_solve3x2InOrder (pS1, pS0, pA1, pA0, pB);
        }
    return rankDeficiency;
    }



/*-----------------------------------------------------------------*//**
*
* @param pB <= matrix containing coefficients that map a unit circle to the
*               standard parabola y=x^2
* @param pBinverse <= inverse matrix
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiTrig_getStandardParabola

(
RotMatrixP pB,
RotMatrixP pBinverse
)
    {
    if (pB)
        pB->InitFromRowValues (
                 0,1,0,
                -1,0,1,
                 1,0,1
                );
    if (pBinverse)
        pBinverse->InitFromRowValues (
                0, -0.5, 0.5,
                1,    0,   0,
                0,  0.5, 0.5
                );
    }


/*-----------------------------------------------------------------*//**
*
* @param pB <= matrix containing coefficients that map a unit circle to the
*               standard parabola y=1/x
* @param pBinverse <= inverse matrix
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiTrig_getStandardHyperbola

(
RotMatrixP pB,
RotMatrixP pBinverse
)
    {
    if (pB)
        pB->InitFromRowValues (
                0, 1,1,
                0,-1,1,
                1, 0,0
                );
    if (pBinverse)
        pBinverse->InitFromRowValues (
                 0.0, -0.0,  1,
                 0.5, -0.5,  0,
                 0.5,  0.5,  0
                );
    }



/*-----------------------------------------------------------------*//**
* @description Compute matrices B, C, Q such that:
*<UL>
*</LI>w*B*C*Q = I
*</LI>(wc ws w1) * B = (0 0 1)
*</LI>If F is a trig point (F.x*F.x + F.y * F.y = F.w * F.w), then Q*F is also.
*</UL>
* @param pw <= scale factor
* @param pB <= transformation to purely cartesian weights.
* @param pC <= basis matrix for a standard ellipse, parabola, or hyperbola.
* @param pQ <= angle transformation
* @param pCurveType <= 1 for ellipse, 0 for parabola, -1 for hyperbola.
* @param wc => weight of conic's vector0
* @param ws => weight of conic's vector90
* @param ws => weight of conic's center
* @return true if |wc| + |ws| + |w1| > 0
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiTrig_factorConicWeights

(
double      *pw,
RotMatrixP pB,
RotMatrixP pC,
RotMatrixP pQ,
int         *pCurveType,
double      wc,
double      ws,
double      w1
)
    {
    double bigW = fabs (wc) + fabs (ws) + fabs (w1);
    static double s_relTol = 1.0e-10;
    double smallW = s_relTol * bigW;
    double wcs;
    double rw, dw;
    bool    boolStat = false;

    if (pw)
        *pw = 1.0;

    if (pB)
        pB->InitIdentity ();

    if (pC)
        pC->InitIdentity ();

    if (pQ)
        pQ->InitIdentity ();

    if (pCurveType)
        *pCurveType = 1;

    if (bigW != 0.0)
        {
        wcs = sqrt (wc * wc + ws * ws);
        dw = wcs - fabs (w1);
        /* Special case to detect things that are already elliptic. */
        if (wcs < smallW)
            {
            *pCurveType = 1;
            boolStat = true;
            if (fabs (w1 - 1.0) > s_relTol)
                {
                double rw = 1.0 / w1;
                *pw = w1;
                if (pB)
                    pB->InitFromScaleFactors (rw, rw, rw);
                }
            }
        else
            {
            /* Shift ws into wc ... */
            double cosine, sine, secant, tangent;
            RotMatrix Cinverse;
            bsiTrig_constructGivensWeights (&cosine, &sine, wc, ws);

            if (pB)
                pB->GivensColumnOp (cosine, sine, 0, 1);
            if (pQ)
                pQ->GivensRowOp (cosine, -sine, 0, 1);

            bsiTrig_applyGivensWeights (&wc, &ws, wc, ws, cosine, sine);

            if (fabs (dw) < smallW)        /* Parabola.  |wc| == |w1| */
                {
                DPoint3d WT;
                RotMatrix B;
                *pCurveType = 0;

                bsiTrig_getStandardParabola (pC, &Cinverse);
                B = *pB;
                B.InitProduct (B, Cinverse);

                if (pB)
                    *pB = B;

                WT.Init ( wc, 0.0, w1);
                B.MultiplyTranspose (WT);
                rw = 1.0 / WT.z;
                if (pw)
                    *pw = WT.z;
                if (pB)
                    pB->ScaleColumns (rw, rw, rw);

                boolStat = true;
                }
            else if (wcs < fabs (w1))   /* Ellipse.  |wc| < |w1| */
                {
                /* Reflect all the weight into w1 */
                bsiTrig_constructHyperbolicWeights (&secant, &tangent, w1, wc);
                bsiTrig_applyHyperbolicWeights (&w1, &wc, w1, wc, secant, tangent);
                if (pQ)
                    pQ->HyperbolicRowOp (secant, -tangent, 2, 0);
                if (pB)
                    pB->HyperbolicColumnOp (secant, tangent, 2, 0);

                rw = 1.0 / w1;
                if (pw)
                    *pw = w1;
                if (pB)
                    pB->ScaleColumns (rw, rw, rw);

                /* Leave C as the identity */
                *pCurveType = 1;

                boolStat = true;
                }
            else                        /* Hyperbola. |wc| > |w1| */
                {
                /* Reflect all the weight into wc */
                bsiTrig_constructHyperbolicWeights (&secant, &tangent, wc, w1);
                bsiTrig_applyHyperbolicWeights (&wc, &w1, wc, w1, secant, tangent);
                if (pQ)
                    pQ->HyperbolicRowOp (secant, -tangent, 2, 0);
                if (pB)
                    pB->HyperbolicColumnOp (secant, tangent, 2, 0);

                bsiTrig_getStandardHyperbola (pC, &Cinverse);
                if (pB)
                    pB->InitProduct (*pB, Cinverse);

                rw = 1.0 / wc;
                if (pw)
                    *pw = wc;
                if (pB)
                    pB->ScaleColumns (rw, rw, rw);
                *pCurveType = -1;

                boolStat = true;
                }
            }
        }
#ifdef VERIFY_bsiTrig_factorConicWeights
    {
    RotMatrix BTSigmaB;
    RotMatrix Sigma;
    RotMatrix BT;
    double d;
    Sigma.InitFromScaleFactors (1.0, 1.0, -1.0);
    BT.TransposeOf (*pB);
    BTSigmaB.InitProduct (BT, Sigma);
    BTSigmaB.InitProduct (BTSigmaB, *pB);
    d = bsiRotMatrix_maxDiff (&Sigma, &BTSigmaB);

    }

    {
    RotMatrix M;
    RotMatrix MTSigmaM;
    RotMatrix Sigma;
    RotMatrix MT;
    double d;
    M.InverseOf (*pB);
    Sigma.InitFromScaleFactors (1.0, 1.0, -1.0);
    MT.TransposeOf (M);
    MTSigmaM.InitProduct (MT, Sigma);
    MTSigmaM.InitProduct (MTSigmaM, M);
    d = bsiRotMatrix_maxDiff (&Sigma, &MTSigmaM);

    }

#endif
    return boolStat;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
