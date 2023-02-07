/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

#include "msbsplinemaster.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    LUDecompositionBanded
(
int         num,  /* num*num matrix */
int         bw,   /* band width */
double      *B    /* num*bw */
)
    {
    int     i, j, jh, k, kl, n = num-1, sbw = bw/2;
    double  sum;

    for( i=0; i<=n; i++ )
        {
        jh = std::min(n, i+sbw);
        for( j=i; j<=jh; j++ )
            {
            kl  = std::max(0,j-sbw);
            sum = 0.0;
            for( k=kl; k<i; k++ )
                sum += B[i*bw + k-i+sbw]*B[k*bw + j-k+sbw];
            B[i*bw + j-i+sbw] -= sum;
            }

        for( j=i+1; j<=jh; j++ )
            {
            kl  = std::max(0,j-sbw);
            sum = 0.0;
            for( k=kl; k<i; k++ )
                sum += B[j*bw + k-j+sbw]*B[k*bw + i-k+sbw];

            if( fabs(B[i*bw + sbw]) < 1e-9 )
                return false;

            B[j*bw + i-j+sbw] = (B[j*bw + i-j+sbw]-sum)/B[i*bw + sbw];
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    linearSystemSolverBanded
(
int         num,  /* num*num matrix */
int         bw,   /* band width */
double      *B,   /* num*bw */
DPoint3d    *P
)
    {
    if (!LUDecompositionBanded (num, bw, B))
        return false;

    int     i, j, jl, jh, n, sbw;

    double  fact;

    DPoint3d *Z, S, *Q, O;

    sbw = bw/2;
    n = num - 1;

    /* Compute solution vector */

    Z = (DPoint3d*)BSIBaseGeom::Malloc ((2*n+2)*sizeof(DPoint3d));
    Q = &Z[n+1];

    for( i=0; i<=n; i++ )
        {
        S.Zero ();

        jl = std::max (0,i-sbw);
        for( j=jl; j<i; j++ )
            {
            S.SumOf (S, Z[j], B[i*bw + j-i+sbw]);
            }
        Z[i].DifferenceOf (P[i], S);
        }

    double ratio;
    O.Zero ();
    for( i=n; i>=0; i-- )
        {
        if (!DoubleOps::SafeDivide (ratio, 1.0, B[i*bw + sbw], fc_hugeVal))
            return false;

        fact = 1.0/B[i*bw + sbw];

        S.Zero ();

        jh = std::min (n, i+sbw);
        for( j=i+1; j<=jh; j++ )
            {
            S.SumOf (S, Q[j], B[i*bw + j-i+sbw]);
            }
        Q[i].SumOf (O, Z[i], fact, S, -fact);
        }

    for (i=0; i<=n; i++)
        P[i] = Q[i];
    BSIBaseGeom::Free (Z);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description Return whether the rational curve's range diagonal is smaller than tolerance, and
*       if so, optionally approximate with a degenerate nonrational curve.
*
* @param pOut       OUT     if return true, replace poles with center of range and drop weights (or NULL)
* @param pIn        IN      curve to query/approximate
* @param tolerance  IN      maximum length of curve's range diagonal to be considered degenerate
* @return true if curve is rational and degenerate
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    isRationalDegenerateCurve
(
MSBsplineCurveP         pOut,
MSBsplineCurveCP        pIn,
double                  tolerance
)
    {
    MSBsplineCurve* pInCast = const_cast <MSBsplineCurveP> (pIn);
    DRange3d        range;

    if (!pIn || !pIn->rational)
        return false;

    pInCast->GetPoleRange (range);
    if (range.ExtentSquared () <= tolerance * tolerance)
        {
        if (pOut)
            {
            MSBsplineCurve  approxCurve;
            MSBsplineStatus       status;
            int             rationalSave = pIn->rational;

            pInCast->rational = false;
            status = bspcurv_copyCurve (&approxCurve, pInCast);
            pInCast->rational = rationalSave;

            if (MSB_SUCCESS == status)
                {
                DPoint3d    center;
                int         i;
                center.Interpolate (range.high, 0.5, range.low);
                for (i = 0; i < approxCurve.params.numPoles; i++) {approxCurve.poles[i] = center;}

                if (pOut == pIn)
                    bspcurv_freeCurve (pOut);
                *pOut = approxCurve;
                }
            }

        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::SampleG1CurveByPoints
(
bvector<DPoint3d>&  P,
bvector<double>&    up,
bvector<double>&    uq,
MSBsplineCurveCP        pCurve,
int                     par,
double                  Eg,
double                  ptol
)
    {
    int      i, j, k, jj, kk, np1, np2, k1 = 0, k2, k3, k4, vjumps, bet, toss, mb;
    int      pb, numPoles = pCurve->params.numPoles;
    double   u, *up1, *up2, *uqq = NULL;
    double   *Ub, cosa, mincos, du, length, d1 = 0, d2, d3, v1 = 0, v2;
    bool     sseg;
    DPoint3d *PP1, *PP2;
    DVec3d   VV[2];

    /* Compute an initial sample of points and analyze curve     */
    /* complexity. Use points from each knot span of base curve. */

    pb = pCurve->params.order - 1;
    mb = numPoles + pb;
    Ub = pCurve->knots;
    for(i=pb; i<mb-pb; i++ )
        {
        if( fabs (Ub[i]-Ub[i+1]) > KNOT_TOLERANCE_BASIS)
            k1++;
        }   /* k1 is number of knot spans */

    if (k1 <= 0)
        return MSB_ERROR;   // avoid division by zero; input curve has invalid knots

    k2 = 1;   /* number of initial sample points interior to a span */
    if ( k1 < 25 )
        {
        if ( k1 >= 10 )
            k2 = 3;
        else
            {
            k2 = 60/k1;
            if ( k2%2 == 0 )
                k2 = k2-1;
            }
        }

    np1 = k1*(k2+1);
    PP1 = (DPoint3d *)BSIBaseGeom::Malloc((np1+1)*sizeof(DPoint3d));
    up1 = (double *)BSIBaseGeom::Malloc((np1+1)*sizeof(double));

    /* Loop and compute points, parameters, and several complexity measures */

    k3 = 1;
    k4 = 0;
    length = 0.0;
    mincos = 1.0;  /* minimum turning cosine */
    sseg = false;  /* possible straight segment in curve */
    vjumps = 0;    /* number of big velocity jumps */
    kk = 0;

    j = pb;
    up1[0] = Ub[j];
    pCurve->FractionToPoint (PP1[0], up1[0]);
    np1 = 1;
    k2 += 1;

    for ( i=0; i<k1; i++ )
        {
        du = (Ub[j+1]-Ub[j])/(k2+1);
        for ( k=1; k<=k2; k++ )
            {
            if ( k < k2 )
                up1[np1] = Ub[j] + k*du;
            else
                up1[np1] = Ub[j+1];
            pCurve->FractionToPoint (PP1[np1], up1[np1]);

            VV[k4].DifferenceOf (PP1[np1], PP1[np1-1]);
            d2 = VV[k4].Magnitude ();
            length += d2;
            v2 = d2/du;
            if ( np1 > 1 && d1*d2 > ptol )
                {
                if ( 7.5*v1 < v2 )
                    vjumps += 1;
                else
                    {
                    if ( 7.5*v2 < v1 )
                        vjumps += 1;
                    }
                d3 = VV[0].DotProduct (VV[1]);
                cosa = d3/(d1*d2);
                if ( cosa < mincos )
                    mincos = cosa;
                if (!sseg && cosa > 0.9999)
                    {
                    if ( kk == 0 )
                        kk = 1;
                    else
                        sseg = true;
                    }
                else
                    kk = 0;
                }
            else
                kk = 0;

            jj = k3;
            k3 = k4;
            k4 = jj;
            d1 = d2;
            v1 = v2;
            np1 += 1;
        }

        if ( i < k1-1 )
            {
            j += 1;
            while ( fabs(Ub[j]-Ub[j+1])<KNOT_TOLERANCE_BASIS && j < mb-1)
                j += 1;
            }
        }

    np1 -= 1;

    /* Handle the degenerate case */

    if ( length < ptol )
        {
        np2 = 2;                 /* pass back 3 points */
        PP2 = (DPoint3d *)BSIBaseGeom::Malloc((np2+1)*sizeof(DPoint3d));
        up2 = (double *)BSIBaseGeom::Malloc((np2+1)*sizeof(double));

        if ( par == 2 || par == 3 )
            {
            uq.push_back (0.0);
            uq.push_back (0.5);
            uq.push_back (1.0);
            }

        up2[0] = up1[0];
        PP2[0] = PP1[0];
        up2[2] = up1[np1];
        PP2[2] = PP1[np1];
        i = np1/2;
        up2[1] = up1[i];
        PP2[1] = PP1[i];

        DPoint3dOps::Append (&P, PP2, np2+1);
        up.assign (up2, up2+np2+1);
        BSIBaseGeom::Free (up2);
        BSIBaseGeom::Free (PP2);
        BSIBaseGeom::Free (up1);
        BSIBaseGeom::Free (PP1);

        return MSB_SUCCESS;
        }

    /* Now the general case. We insert more points between */
    /* the existing ones. First determine how many.        */

    bet = -1;   /* number of additional points between existing */
    if ( length < 10.0*Eg )
        bet = 0;
    else
        {
        if ( length < 100.0*Eg )
            {
            bet = 1;
            if ( mincos < 0.9975 && np1 < 50 )
                {
                if ( sseg || vjumps > 0 )
                    bet = 2;
                }
            }
        }

    if ( bet == -1 )
        {
        d1 = 4.0*Eg;
        d2 = 1.0 - 2.0*PI*d1/length;  /* based on circle */
        if ( d2 < -1.0 )
            d2 = -1.0;
        d3 = acos(d2);
        if ( 2.0*PI > 1400.0*d3 )
            k = 1400;     /* roughly total number of */
        else
            k = (int)(2.0*PI/d3) + 2;    /* points */

        bet = (k-np1)/np1 + 1;
        if ( bet < 2 )
            bet = 2;

        k = vjumps;             /* add points for possible complexities */
        if ( mincos <= 0.999 )
            {
            if ( sseg )
                k += 3;
            if ( mincos < 0.55 )
                k += 3;
            else
                {
                if ( mincos < 0.75 )
                    k += 1;
                }
            }

        if ( k > 0 )
            {
            k = k + (k%2);
            if ( k > 10 )
                k = 10;
            if ( bet < 10 )
                bet += k;
            else
                bet += (3*k/2);
            d3 = 50.0/np1;
            bet = (int)(bet*d3);
            }
        }

    /* Now compute points and parameters. */

    k = np1*(bet+1) + 1;
    PP2 = (DPoint3d *)BSIBaseGeom::Malloc((k+1)*sizeof(DPoint3d));
    up2 = (double *)BSIBaseGeom::Malloc((k+1)*sizeof(double));

    if ( par == 2 || par == 3 )
        {
        uqq = (double *)BSIBaseGeom::Malloc((k+1)*sizeof(double));;
        uqq[0] = 0.0;
        }

    length = 0.0;
    np2 = 0;
    up2[0] = up1[0];
    PP2[0] = PP1[0];
    for ( i=1; i<=np1; i++ )
        {
        u = up1[i-1];
        du = (up1[i]-u)/(bet+1);
        toss = 0;
        for ( j=1; j<=bet; j++ )
            {
            np2 += 1;
            up2[np2] = u + j*du;
            pCurve->FractionToPoint (PP2[np2], up2[np2]);
            d1 = PP2[np2-1].Distance (PP2[np2]);
            if ( d1 <= ptol )
                {
                np2 -= 1;
                toss += 1;
                }
            else
                if ( par == 2 )
                    {
                    length += d1;
                    uqq[np2] = length;
                    }
            }
        np2 += 1;
        up2[np2] = up1[i];
        pCurve->FractionToPoint (PP2[np2], up2[np2]);
        d1 = PP2[np2-1].Distance (PP2[np2]);
        if ( d1 <= ptol && bet-toss > 0 )
            {
            up2[np2-1] = up2[np2];
            PP2[np2-1] = PP2[np2];
            np2 -= 1;
            d1 = PP2[np2-1].Distance (PP2[np2]);
            }
        if ( par == 2 )
            {
            length += d1;
            uqq[np2] = length;
            }
        }

    if ( par == 3 )
        {
        double *dist = (double *)BSIBaseGeom::Malloc((np2+1)*sizeof(double));
        double sum = 0.0;
        for( i=1; i<=np2; i++ )
            {
            dist[i] = PP2[i-1].Distance (PP2[i]);
            dist[i] = sqrt(dist[i]);
            sum    += dist[i];
            }
        for( i=1; i<np2; i++ )
            uqq[i] = uqq[i-1] + dist[i]/sum;
        _Analysis_assume_(uqq != nullptr);
        uqq[np2] = 1.0;
        BSIBaseGeom::Free (dist);
        }
    else
        if ( par == 2 )
            {
            uqq[np2] = 1.0;
            for ( i=1; i<np2; i++ )
                uqq[i] = uqq[i]/length;
            }

    DPoint3dOps::Append (&P, PP2, np2+1);
    uq.assign (uqq, uqq + np2+1);
    up.assign (up2, up2 + np2+1);

    BSIBaseGeom::Free (up2);
    BSIBaseGeom::Free (PP2);
    BSIBaseGeom::Free (up1);
    BSIBaseGeom::Free (PP1);
    if ( par == 2 || par == 3 )
        BSIBaseGeom::Free (uqq);

    return MSB_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void   computeParameterization
(
double              *params,
int                 num,
DPoint3dCP          pts,
int                 type
)
    {
    int i, n = num - 1;
    params[0] = 0.0;
    params[n] = 1.0;

    if (1 == type)
        {
        double fact = 1.0/n;

        for( i=1; i<n; i++ )
            params[i] = i*fact;

        return;
        }

    /* Compute chord length or centripetal parametrization */

    double *dist = (double*)BSIBaseGeom::Malloc (num * sizeof(double));
    double sum = 0.0;

    switch( type )
      {
        case 2:

          for( i=1; i<=n; i++ )
          {
          dist[i] = pts[i-1].Distance (pts[i]);
          sum += dist[i];
          }
          break;

        case 3:

         for( i=1; i<=n; i++ )
          {
          dist[i] = pts[i-1].Distance (pts[i]);
          dist[i] = sqrt(dist[i]);
          sum += dist[i];
          }
          break;

        default:

          BSIBaseGeom::Free (dist);
          return;
      }

    if (sum > fc_epsilon )
        for( i=1; i<n; i++ )  params[i] = params[i-1] + dist[i]/sum;

    BSIBaseGeom::Free (dist);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int   intervalIndex
(
bvector<double> knotVector,
int                 order,
double              param
)
    {
    int low, high, mid;
    int n = (int)knotVector.size()- order - 1;
    int p= order - 1;
    double u = param;

    if( fabs(u - knotVector[n+1]) < KNOT_TOLERANCE_BASIS )
        {
        if( fabs(u - knotVector[n+p+1]) < KNOT_TOLERANCE_BASIS )
            return n;
        else
            return (n+1);
        }
    low = p;
    high = n + 1;
    mid = (low + high)/2;
    while( u < knotVector[mid] || u>=knotVector[mid+1] )
        {
        if( u < knotVector[mid])
            high = mid;
        else
            low = mid;
        mid = (low + high)/2;
        }

    return mid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static MSBsplineStatus   nonZeroBasisFuncs
(
double              *N,
int                 &spn,
bvector<double> knotVector,
int                 order,
double              u
)
    {
    int     i, k, l, m, p = order - 1;

    double  left[MAX_ORDER+1], right[MAX_ORDER+1], saved, temp;

    /* Get local notation */

    m = (int)knotVector.size() - 1;

    /* Check parameter */

    if ( u < knotVector[0]  ||  u > knotVector[m] )
        return MSB_ERROR;

    /* Special cases for end values */

    if( fabs (u-knotVector[p]) < KNOT_TOLERANCE_BASIS )
        {
        N[0] = 1.0;
        spn = p;
        for( k=1; k<=p; k++ )
            N[k] = 0.0;

        return MSB_SUCCESS;
        }

    if( fabs (u-knotVector[m-p]) < KNOT_TOLERANCE_BASIS )
        {
        N[p] = 1.0;
        spn = m-p-1;
        for( k=0; k<p; k++ )
            N[k] = 0.0;

        return MSB_SUCCESS;
        }

    /* Find the knot span u is in */

    spn = i = intervalIndex (knotVector, order, u);

    /* Compute the non-vanishing B-splines */

    N[0] = 1.0;
    for(k=1; k<=p; k++)
        {
        left[k]  = u-knotVector[i+1-k];
        right[k] = knotVector[i+k]-u;
        saved    = 0.0;
        for(l=0; l<k; l++)
            {
            temp  = N[l]/(right[l+1]+left[k-l]);
            N[l]  = saved + right[l+1]*temp;
            saved = left[k-l]*temp;
            }
        N[k] = saved;
        }

    return MSB_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static MSBsplineStatus   nonZeroBasisFuncsDers
(
double              *ND, /*size = (der+1)*order*/
int                 der,
int                 &spn,
bvector<double> U,
int                 order,
double              u
)
    {
    int    i, j, k, r, s1, s2, rk, pk, j1, j2, mder, p = order - 1;
    double saved, temp, d;

    /* Get local notation */

    k = (int)U.size () - 1;

    /* Find the knot span u is in */

    spn = i = intervalIndex (U, order, u);

    /* Get maximum derivative index and set zero derivatives */

    mder = std::min(p,der);

    for( k=0; k<=mder; k++ )
        {
        for( j=0; j<=p; j++ )
            {
            ND[k*order+j] = 0.0;
            }
        }

    /* Allocate memory */

    bvector<double> left (order), right (order), ndu (order*order), a (2*order);

    /* Compute the basis functions */

    ndu[0] = 1.0;
    for( j=1; j<=p; j++ )
        {
        left[j]  = u-U[i+1-j];
        right[j] = U[i+j]-u;
        saved    = 0.0;
        for( r=0; r<j; r++ )
            {
            ndu[j*order+r] = right[r+1]+left[j-r];
            temp      = ndu[r*order+j-1]/ndu[j*order+r];
            ndu[r*order+j] = saved + right[r+1]*temp;
            saved = left[j-r]*temp;
            }
        ndu[j*order+j] = saved;
        }

    /* Load the basis functions */

    for( j=0; j<=p; j++ )
        ND[j] = ndu[j*order+p];

    /* Compute derivatives */

    for( r=0; r<=p; r++ )
        {
        s1 = 0;
        s2 = 1;
        a[0] = 1.0;
        for( k=1; k<=mder; k++ )
            {
            d  = 0.0;
            rk = r-k;
            pk = p-k;
            if( r >= k )
                {
                a[s2*order] = a[s1*order]/ndu[(pk+1)*order+rk];
                d           = a[s2*order]*ndu[rk*order+pk];
                }
            if( rk    >= -1 )
                j1 = 1;
            else
                j1 = -rk;
            if( (r-1) <= pk )
                j2 = k-1;
            else
                j2 = p-r;
            for( j=j1; j<=j2; j++ )
                {
                a[s2*order+j]  = (a[s1*order+j]-a[s1*order+j-1])/ndu[(pk+1)*order+rk+j];
                d += a[s2*order+j]*ndu[(rk+j)*order+pk];
                }
            if( r <= pk )
                {
                a[s2*order+k] = -a[s1*order+k-1]/ndu[(pk+1)*order+r];
                d +=  a[s2*order+k]*ndu[r*order+pk];
                }
            ND[k*order+r] = d;
            std::swap (s1,s2);
            }
        }


    /* Multiply through by the correct factors */

    r = p;
    for( k=1; k<=mder; k++ )
        {
        for( j=0; j<=p; j++ )
            {
            ND[k*order+j] *= r;
            }
        r *= (p-k);
        }

    return MSB_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static MSBsplineStatus   oneBasisFuncs
(
double              *N,
int                 i,
double             const *knotVector,
int                 mKnots,
int                 order,
double              u,
int                 flg /*0-left, 1-right*/
)
    {
    int    j, k, n, m = mKnots - 1, p = order - 1;

    double UL, UR, saved, temp;

    n = m - p - 1;

    /* Check parameter and index */

    if( u < knotVector[0]  ||  u > knotVector[m] )
        return MSB_ERROR;

    if( i < 0  || i > n )
        return MSB_ERROR;

    /* Special cases for end values */

    if( fabs (u-knotVector[p]) < KNOT_TOLERANCE_BASIS )
        {
        if( i == 0 )
            *N = 1.0;
        else
            *N = 0.0;

        return MSB_SUCCESS;
        }

    if( fabs (u-knotVector[m-p]) < KNOT_TOLERANCE_BASIS )
        {
        if( i == n )
            *N = 1.0;
        else
            *N = 0.0;

        return MSB_SUCCESS;
        }

    bvector<double> NA (p+1);

    /* Compute degree zero B-splines */

    switch( flg )
        {
        case 0:

            if( u < knotVector[i]  ||  (u > knotVector[i+p+1] || fabs (u-knotVector[i+p+1]) < KNOT_TOLERANCE_BASIS) )
                {
                *N = 0.0;
                return MSB_SUCCESS;
                }

            for( j=0; j<=p; j++ )
                {
                if( (u > knotVector[i+j] || fabs (u-knotVector[i+j]) < KNOT_TOLERANCE_BASIS) && u < knotVector[i+j+1] )
                    NA[j] = 1.0;
                else
                    NA[j] = 0.0;
                }
            break;

        case 1:

            if( (u < knotVector[i] || fabs (u-knotVector[i]) < KNOT_TOLERANCE_BASIS)  ||  u > knotVector[i+p+1] )
                {
                *N = 0.0;
                return MSB_SUCCESS;
                }

            for( j=0; j<=p; j++ )
                {
                if( u > knotVector[i+j] && (u < knotVector[i+j+1] || fabs (u-knotVector[i+j+1]) < KNOT_TOLERANCE_BASIS) )
                    NA[j] = 1.0;
                else
                    NA[j] = 0.0;
                }
            break;

        default:

            return MSB_ERROR;
        }

    /* Compute the i-th B-spline using a triangular array */

    for( k=1; k<=p; k++ )
        {
        if( fabs (NA[0]) < fc_epsilon )
            saved = 0.0;
        else
            saved = ((u-knotVector[i])*NA[0])/(knotVector[i+k]-knotVector[i]);

        for( j=0; j<p-k+1; j++ )
            {
            UR = knotVector[i+j+k+1];
            UL = knotVector[i+j+1];
            if( fabs (NA[j+1]) < fc_epsilon )
                {
                NA[j] = saved;
                saved = 0.0;
                }
            else
                {
                temp  = NA[j+1]/(UR-UL);
                NA[j]  = saved + (UR-u)*temp;
                saved = (u-UL)*temp;
                }
            }
        }

    *N = NA[0];

    return MSB_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static MSBsplineStatus InterpolationByPointsEndsParamsAndKnots
(
MSBsplineCurveP         pOut,
bvector<DPoint3d>   P,
bvector<double>     u,
bvector<double>     knots,
int                     order,
DVec3d                  As,
DVec3d                  Ae
)
    {
    int    i, j, k, l, n, m, mc, n1, bw, sbw, sp;
    double     fact;

    int kk = (int)(P.size ()) - 1, p = order - 1;
    m = (int)(knots.size ()) - 1;
    n  = kk+2;
    mc = n+p+1;
    n1 = n+1;

    if( p < 2  ||  p > kk+2 )
        return MSB_ERROR;
    if( m !=  mc )
        return MSB_ERROR;

    pOut->Allocate (n1, p+1, 0, 0);
    for( i=0; i<=m; i++ )
        pOut->knots[i] = knots[i];

    /* Compute coefficient matrix */
    if ( 2*p <= n )
        {
        bw  = 2*p+1;
        sbw = p;
        }
    else
        {
        bw = 2*p-1;
        sbw = p-1;
        }
    sp = sbw-p;    /* this must be non-positive */

    bvector<double> N (p+1), B(n1*bw);
    memset(&B[0], 0, n1*bw*sizeof (double));

    B[sbw] =  1.0;
    B[bw + sbw-1] = -1.0;
    B[bw + sbw] =  1.0;
    B[(n-1)*bw + sbw] = -1.0;
    B[(n-1)*bw + sbw+1] =  1.0;
    B[n*bw + sbw] =  1.0;

    for( i=2; i<=kk; i++ )
        {
        nonZeroBasisFuncs (&N[0], j, knots, order, u[i-1]);

        l = sp-i+j;
        if( l < 0  ||  l+p >= bw )
            return MSB_ERROR;

        for( k=0; k<=p; k++ )
            B[i*bw + l+k] = N[k];
        }

    bvector<DPoint3d> Q (n1);
    fact = (knots[p+1]-knots[1])/p;
    Q[0] = P[0];
    Q[1].Scale (*((DPoint3dP)&As), fact);

    for( i=2; i<=kk; i++ )
        Q[i] = P[i-1];

    fact = (knots[n+p]-knots[n])/p;
    Q[n-1].Scale (*((DPoint3dP)&Ae), fact);
    Q[n] = P[kk];

    if (linearSystemSolverBanded (n1, bw, &B[0], &Q[0]))
        {
        for (i=0; i<n1; i++)
            {
            pOut->poles[i] = Q[i];
            }

        return MSB_SUCCESS;
        }

    return MSB_ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus InterpolationByPointsParamsAndKnots
(
MSBsplineCurveP         pOut,
bvector<DPoint3d>   &P,     // solved in place !!!!
bvector<double>     const &u,
bvector<double>     const &knots,
int                     order
)
    {
    int    i, j, k, l, m, mc, n1, bw, sbw, sp;

    int n = (int)(P.size ()) - 1, p = order - 1;
    m = (int)(knots.size ()) - 1;
    mc = n+p+1;
    n1 = n + 1;

    if( m !=  mc ||  p > n )
        return MSB_ERROR;

    pOut->Allocate (n1, p+1, 0, 0);
    for( i=0; i<=m; i++ )
        pOut->knots[i] = knots[i];

    if(p == 1)
        {
        for (i=0; i<=n; i++ )
             pOut->poles[i] = P[i];
        }

    /* Compute coefficient matrix */
    if ( 2*p <= n )
        {
        bw = 2*p+1;
        sbw = p;
        }
    else
        {
        bw  = 2*p-1;
        sbw = p-1;
        }
    sp = sbw-p;   /* this must be non-positive */

    bvector<double> N (p+1), B(n1*bw);
    memset(&B[0], 0, n1*bw*sizeof (double));

    B[sbw] = 1.0;
    B[n*bw + sbw] = 1.0;
    for( i=1; i<n; i++ )
        {
        nonZeroBasisFuncs (&N[0], j, knots, order, u[i]);
        l = sp-i+j;
        if( l < 0  ||  l+p >= bw )
            return MSB_ERROR;

        for( k=0; k<=p; k++ )
            B[i*bw + l+k] = N[k];
        }

    if (linearSystemSolverBanded (n1, bw, &B[0], &P[0]))
        {
        for (i=0; i<n1; i++)
            {
            pOut->poles[i] = P[i];
            }

        return MSB_SUCCESS;
        }

    return MSB_ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static MSBsplineStatus     knotRemoveWithParamsEnds
(
MSBsplineCurveP         pCurve,
double                  *u,
double                  *er,
int                     num,
double                  E,
int                     whr, /* End contrainsts -- NO:0, START:1, END:2, BOTH=3 */
int                     der  /* Order of end contrainsts */
)
    {
    MSBsplineStatus status = MSB_SUCCESS;
    bool     rmf;
    int      i, j, k, l, ii, jj, first, last, off, fout, n, m, r, s, norem = 0;
    int      p = pCurve->params.order - 1;
    double   *U, b, alf, oma, bet, omb, lam = 0.0, oml = 0.0, N, N1;
    DPoint4d Ow;

    Ow.Zero ();
    n = pCurve->params.numPoles - 1;
    m = n + p +1;
    int mu = num - 1;

    bvector<DPoint4d> Rw (2*p+1);
    bvector<DPoint4d> Pw (n+1);
    bvector<double> br (m+1), te (mu+1);
    bvector<int> sr (m+1), left (n+1), right (n+1), nr (m+1);

    U = pCurve->knots;
    for (k=0; k<n+1; k++)
        {
        if (pCurve->rational)
            Pw[k].InitFrom (*(&pCurve->poles[k]), pCurve->weights[k]);
        else
            Pw[k].InitFrom (*(&pCurve->poles[k]), 1.0);
        }

    for( i=0; i<=m; i++ )
        {
        br[i] = fc_hugeVal;
        sr[i] = 0;
        nr[i] = 0;
        }

    /* Compute knot removal errors for each distinct knot */

    r = p+1;
    while( r <= n )
        {
        i = r;
        while( r <= n  &&  U[r+1] - U[r] < KNOT_TOLERANCE_BASIS)
            r++;
        sr[r] = r-i+1;
        br[r] = MSBsplineCurve::GetRemovalKnotBound (pCurve, r, sr[r] );
        r++;
        }

    /* Get ranges of parameter indexes */

    j = 1;
    for( i=0; i<=n; i++ )
        {
        left[i] = j;
        while( j < mu  &&  u[j] > U[i]  &&  u[j] <= U[i+1] )
            j++;

        for( k=j; k<mu; k++ )
            {
            if( u[k] >= U[i+p+1])
                break;
            }
        right[i] = k-1;
        }

    /* Try to remove knots until cumulative error < E */

    while( true )
        {
        /* Find knot with smallest error */

        b = br[p+1];
        norem = nr[p+1];
        s = sr[p+1];
        r = p+1;
        for( i=p+2; i<=m-p-1; i++ )
            {
            if( br[i] < b )
                {
                b = br[i];
                s = sr[i];
                r = i;
                norem = nr[i];
                }
            }

        /* If no more removable knot -> finished */

        if( norem  == 1 )  break;

        /* Check error of removal */

        rmf = true;
        if( whr == 1  ||  whr == 3 )
            {
            if( r <= (p+der) )
                rmf = false;
            }

        if( whr == 2  ||  whr == 3 )
            {
            if( r >= (n-der+1) )
                rmf = false;
            }

        if( !rmf )
            {
            br[r] = fc_hugeVal;
            nr[r] = 1;
            continue;
            }

        rmf = true;
        if( (p+s)%2 )
            {
            k   = (p+s+1)/2;
            i   = r-k;
            alf = (U[r]-U[i  ])/(U[i+p+1]-U[i  ]);
            bet = (U[r]-U[i+1])/(U[i+p+2]-U[i+1]);
            omb = 1.0-bet;
            lam = alf/(alf+bet);
            oml = 1.0-lam;

            for( j=left[i]; j<=right[i+1]; j++ )
                {
                status = oneBasisFuncs (&N, i, U, m+1, p+1, u[j], 0);
                if(MSB_SUCCESS != status)
                    goto wrapup;

                status = oneBasisFuncs (&N1, i+1, U, m+1, p+1, u[j], 0);
                if(MSB_SUCCESS != status)
                    goto wrapup;

                te[j] = er[j] + fabs(lam*alf*N-oml*omb*N1)*b;
                if( te[j] > E )
                    {
                    rmf = false;
                    break;
                    }
                }
            }
        else
            {
            k = (p+s)/2;
            i = r-k;

            for( j=left[i]; j<=right[i]; j++ )
                {
                status = oneBasisFuncs (&N, i, U, m+1, p+1, u[j], 0);
                if(MSB_SUCCESS != status)
                    goto wrapup;

                te[j] = er[j]+N*b;
                if( te[j] > E )
                    {
                    rmf = false;
                    break;
                    }
                }
            }

        /* If knot is removable, update error and remove knot */

        if( rmf )
            {
            if( (p+s)%2 )
                l = right[i+1];
            else
                l = right[i];

            for( j=left[i]; j<=l; j++ )
                er[j] = te[j];

            fout  = (2*r-s-p)/2;
            first = r-p;
            last  = r-s;
            off   = first-1;
            i     = first;
            j     = last;
            ii    = 1;
            jj    = last-off;

            Rw[0] = Pw[off];
            Rw[last+1-off] = Pw[last+1];

            /* Get new control points for one removal step */

            while( (j-i) > 0 )
                {
                alf = (U[i+p+1]-U[i])/(U[r]-U[i]);
                oma = 1.0-alf;
                bet = (U[j+p+1]-U[j])/(U[j+p+1]-U[r]);
                omb = 1.0-bet;
                //comcpt(Rw[ii], alf, Pw[i], oma, Rw[ii-1]);
                Rw[ii].SumOf (Ow, Pw[i], alf, Rw[ii-1], oma);
                //comcpt(Rw[jj], bet, Pw[j], omb, Rw[jj+1]);
                Rw[jj].SumOf (Ow, Pw[j], bet, Rw[jj+1], omb);
                i ++;  j --;
                ii++;  jj--;
                }

            /* Save new control points */

            if( (p+s)%2 )
                {
                //comcpt(Rw[jj+1], lam, Rw[jj+1], oml, Rw[ii-1]);
                Rw[jj+1].SumOf (Ow, Rw[jj+1], lam, Rw[ii-1], oml);
                }

            i = first;
            j = last;
            while( (j-i) > 0 )
                {
                Pw[i] = Rw[i-off];
                Pw[j] = Rw[j-off];
                i++;  j--;
                }

            /* Shift down some parameters */

            if( s > 1 )
                sr[r-1] = sr[r]-1;

            for( i=r+1; i<=m; i++ )
                {
                br[i-1] = br[i];
                nr[i-1] = nr[i];
                sr[i-1] = sr[i];
                }

            /* Shift down knots and control points */

            for( i=r+1;    i<=m; i++ )
                U[i-1] = U[i];
            for( i=fout+1; i<=n; i++ )
                Pw[i-1] = Pw[i];
            n--;  m--;

            pCurve->params.numPoles = n+1;
            for (int cpi = 0; cpi < n+1; cpi++)
                {
                pCurve->poles[cpi].x = Pw[cpi].x;
                pCurve->poles[cpi].y = Pw[cpi].y;
                pCurve->poles[cpi].z = Pw[cpi].z;
                if (pCurve->rational)
                    pCurve->weights[cpi] = Pw[cpi].w;
                }

            /* If no more internal knots -> finished */

            if( n == p )
                break;

            /* Update error bounds */

            k = std::max (r-p, p+1);
            l = std::min (n, r+p-s);
            for( i=k; i<=l; i++ )
                {
                if( U[i] != U[i+1] && nr[i] != 1 )
                    {
                    br[i] = MSBsplineCurve::GetRemovalKnotBound (pCurve, i, sr[i]);
                    }
                }

            /* Update index ranges */

            for( i=r-p-1; i<=r-s; i++ )
                {
                for( k=right[i]+1; k<=mu; k++ )
                    {
                    if( u[k] >= U[i+p+1] )
                        break;
                    }
                right[i] = k-1;
                }

            for( i=r-s+1; i<=n; i++ )
                {
                left [i] = left [i+1];
                right[i] = right[i+1];
                }
            }
        else
            {
            /* Knot is not removable */
            br[r] = fc_hugeVal;
            nr[r] = 1;
            }
        } /* End of while loop */

wrapup:
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static MSBsplineStatus   linearSystemSolver3D
(
DPoint3d    *pPntOut,
int         n1,
double      *matrix,
DPoint3d    *pPntIn
)
    {
    MSBsplineStatus status = MSB_ERROR;
    int i;
    bvector<double> xyz (3*n1);

    for (i=0; i<n1; i++)
        {
        xyz[i] = pPntIn[i].x;
        xyz[i + n1] = pPntIn[i].y;
        xyz[i + 2*n1] = pPntIn[i].z;
        }

    if (bsiLinAlg_solveLinearGaussPartialPivot (matrix, n1, &xyz[0], 3))
        {
        status = MSB_SUCCESS;
        for (i=0; i<n1; i++)
            {
            pPntOut[i].x = xyz[i];
            pPntOut[i].y = xyz[i + n1];
            pPntOut[i].z = xyz[i + 2*n1];
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     matrixReverse
(
double      *matrix,
int         n
)
    {
    int i, j, k, l, u, v;
    double d,p;
    bvector<int> is (n), js (n);

    for (k=0; k<=n-1; k++)
        {
        d=0.0;

        for (i=k; i<=n-1; i++)
            for (j=k; j<=n-1; j++)
                {
                l = i*n + j;
                p = fabs (matrix[l]);
                if (p > d)
                    {
                    d = p;
                    is[k] = i;
                    js[k] = j;
                    }
                }

        if (d + 1.0 == 1.0)
            return false;

        if (is[k] != k)
            for (j=0; j<=n-1; j++)
                {
                u = k*n + j;
                v = is[k]*n + j;
                p = matrix[u];
                matrix[u] = matrix[v];
                matrix[v] = p;
                }

        if (js[k] != k)
            for (i=0; i<=n-1; i++)
                {
                u = i*n + k;
                v = i*n + js[k];
                p = matrix[u];
                matrix[u] = matrix[v];
                matrix[v] = p;
                }

        l = k*n + k;
        matrix[l] = 1.0/matrix[l];

        for (j=0; j<=n-1; j++)
            if (j != k)
                {
                u = k*n + j;
                matrix[u] = matrix[u] * matrix[l];
                }

        for (i=0; i<=n-1; i++)
            if (i != k)
                for (j=0; j<=n-1; j++)
                    if ( j!= k)
                        {
                        u = i*n + j;
                        matrix[u] = matrix[u] - matrix[i*n+k] * matrix[k*n+j];
                        }

        for (i=0; i<=n-1; i++)
            if (i != k)
                {
                u = i*n + k;
                matrix[u] = -matrix[u] * matrix[l];
                }
        }

    for (k=n-1; k>=0; k--)
        {
        if (js[k] != k)
            for (j=0; j<=n-1; j++)
                {
                u = k*n + j;
                v = js[k] * n + j;
                p = matrix[u];
                matrix[u] = matrix[v];
                matrix[v] = p;
                }

        if (is[k]!=k)
          for (i=0; i<=n-1; i++)
              {
              u = i*n + k;
              v = i*n + is[k];
              p = matrix[u];
              matrix[u] = matrix[v];
              matrix[v]=p;
              }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void     matrixMultiple
(
double      *a,
double      *b,
double      *result,
int         m,
int         n,
int         k
)
    {
    int i, j, l, u;
    for (i=0; i<=m-1; i++)
        for(j=0; j<=k-1; j++)
            {
            u = i*k + j;
            result[u]=0.0;
            for (l=0; l<=n-1; l++)
                result[u] = result[u] + a[i*n+l] * b[l*k+j];
            }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void     matrixPointArrayMultiple
(
DPoint3d        *Q,
double          *A,
DPoint3d        *P,
int             n,
int             m
)
    {
    for(int i=0; i<n; i++ )
        {
        Q[i].Zero ();
        for(int j=0; j<m; j++ )
            {
            Q[i].SumOf (Q[i], P[j], A[i*m+j]);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static MSBsplineStatus   weightedLeastSquaresApproximation
(
MSBsplineCurveP         pOut,   /* <= Output curve */
bvector<DPoint3d>   R,       /* => input points */
bvector<double>     u,       /* => Parameters corresponding to data points.*/
bvector<double>     wp,      /* => Array of point weights: wp[i] > 0.0: R[i] is unconstrained, wp[i] < 0.0: R[i] is constrained */
bvector<DVec3d>     D,       /* => input vectors */
bvector<double>     wd,      /* => Array of point weights: wd[i] > 0.0: D[i] is unconstrained, wd[i] < 0.0: D[i] is constrained */
bvector<int>        I,       /*.=> Index array; the derivative at R[I[j]] is D[j].*/
bvector<double>     knots,   /* => Knot vector of approximating curve. */
int                     numPoles,/* => number of poles of output curve */
int                     order    /* => order of output curve */
)
    {
    MSBsplineStatus status = MSB_SUCCESS;
    bool der = false;
    int    i, j, k, rj, sj, ej, lk, hk, ru, su, rc, sc, mu, mc, mu2, mc2, jd, mknt, mcur;
    DPoint3d    B;

    /* Initialize some parameters */
    int p = order - 1, r = (int)R.size () - 1, s = (int)D.size () - 1;

    rj =  p;       sj  =  p;      ej  = -1;
    ru = -1;       su  = -1;      rc  = -1;
    sc = -1;

      /* Get constrained and unconstrained indexes and check error */

    for( i=0; i<=r; i++ )
        {
        if( wp[i] > 0.0 )
            ru++;
        else
            rc++;
        }

    for( j=0; j<=s; j++ )
        {
        if( wd[j] > 0.0 )
            su++;
        else
            sc++;
        }

    mknt = (int)knots.size () - 1;
    int n = numPoles - 1;

    mu   = ru+su+1;
    mc   = rc+sc+1;
    mcur = n+p+1;
    int mu1 = mu + 1;

    if ( mc >= n  ||  (mc+n) >= (mu+1) )
        return MSB_ERROR;
    if ( n < 1  ||  p  > n )
        return MSB_ERROR;
    if ( mknt != mcur )
        return MSB_ERROR;

    /* Allocate memory */

    bvector<int>    index (mu1), start (numPoles), end (numPoles);
    bvector<double> W (mu1), ND (2*order), N (mu1*order), WN (mu1*order), NTWN (numPoles*numPoles), M;
    if (mc >= 0)
        M.resize((mc+1)*numPoles);

    /* Initialize matrices */

    for( i=0; i<=mu; i++ )
        {
        for( j=0; j<=p; j++ )
            {
            N [i*order+j] = 0.0;
            WN[i*order+j] = 0.0;
            }
        }

    if( mc >= 0 )
        {
        for( i=0; i<=mc; i++ )
            {
            for( j=0; j<=n; j++ )
                M[i*numPoles+j] = 0.0;
            }
        }

    for( i=0; i<=n; i++ )
        {
        for( j=0; j<=n; j++ )
            NTWN[i*numPoles+j] = 0.0;
        }

    /* Compute N, M and W */

    for( i=0; i<=n; i++ )
        {
        start[i] = 0;
        end  [i] = mu;
        }

    mu2 = 0;
    mc2 = 0;
    jd  = 0;
    for( i=0; i<=r; i++ )
        {
        nonZeroBasisFuncsDers (&ND[0], 1, j, knots, p+1, u[i]);

        der = false;
        if( jd <= s  &&  i == I[jd] )
            der = true;

        if( wp[i] > 0.0 )  /* Unconstrained point data */
            {
            W[mu2] = wp[i];

            for( k=0; k<=p; k++ )
                N[mu2*order+k] = ND[k];

            index[mu2] = j-p;
            if( j > rj )
                {
                for( k=1; k<=j-rj; k++ )
                    {
                    sj++;  ej++;
                    start[sj] = mu2;
                    end  [ej] = mu2-1;
                    }
                rj = j;
                }
            mu2++;
            }
        else                /* Constrained point data */
            {
            for( k=0; k<=p; k++ )
                M[mc2*numPoles+j-p+k] = ND[k];
            mc2++;
            }

        if( der )
            {
            if( wd[jd] > 0.0 )  /* Unconstrained derivative data */
                {
                W[mu2] = wd[jd];

                for( k=0; k<=p; k++ )
                    N[mu2*order+k] = ND[order+k];

                index[mu2] = j-p;
                if( j > rj )
                    {
                    for( k=1; k<=j-rj; k++ )
                        {
                        sj++;  ej++;
                        start[sj] = mu2;
                        end  [ej] = mu2-1;
                        }
                    rj = j;
                    }
                mu2++;
                }
            else                /* Constrained derivative data */
                {
                for( k=0; k<=p; k++ )
                    M[mc2*numPoles+j-p+k] = ND[order+k];
                mc2++;
                }
            jd++;
            }
        }

    if( sj < n  ||  end[0] == -1 )
        return MSB_ERROR;

    /* Compute WN, NTWN*/

    for( j=0; j<=n; j++ )
        {
        lk = start[j];
        hk = end  [j];
        for( k=lk; k<=hk; k++ )
            {
            WN[k*order+j-index[k]] = W[k]*N[k*order+j-index[k]];
            }
        }

    for( i=0; i<=n; i++ )
        {
        for( j=0; j<=n; j++ )
            {
            lk = std::max (start[i],start[j]);
            hk = std::min (end  [i],end  [j]);
            for( k=lk; k<=hk; k++ )
                {
                NTWN[i*numPoles+j] += N[k*order+i-index[k]]*WN[k*order+j-index[k]];
                }
            }
        }

    bvector<DPoint3d> S(mu+1), Pw (numPoles), NTWS (numPoles), INTWS (numPoles), V (std::max (n,mc)+1), A, T;
    if( mc >= 0 )
        {
        A.resize (mc+1);
        T.resize (mc+1);
        }

    mu2 = 0;
    mc2 = 0;
    jd  = 0;
    for( i=0; i<=r; i++ )
        {
        der = false;
        if( jd <= s  &&  i == I[jd] )
        der = true;

        if( wp[i] > 0.0 )  /* Unconstrained point data */
            {
            S[mu2].Scale (R[i], W[mu2]);
            mu2++;
            }
        else                /* Constrained point data */
            {
            T[mc2] = R[i];
            mc2++;
            }

        if( der )
            {
            if( wd[jd] > 0.0 )  /* Unconstrained derivative data */
                {
                S[mu2].Scale (D[jd], W[mu2]);
                mu2++;
                }
            else                /* Constrained derivative data */
                {
                T[mc2] = D[jd];
                mc2++;
                }
            jd++;
            }
        }

    for( i=0; i<=n; i++ )
        {
        NTWS[i].Zero();

        lk = start[i];
        hk = end  [i];
        for( k=lk; k<=hk; k++ )
            {
            NTWS[i].SumOf (NTWS[i], S[k], N[k*order+i-index[k]]);
            }
        }

    /* In no constraints, do weighted approximation */

    if( mc < 0 )
        {
        status = linearSystemSolver3D (&Pw[0], numPoles, &NTWN[0], &NTWS[0]);
        if(MSB_SUCCESS != status)
            return MSB_ERROR;
        }

    /* Get matrices to compute Lagrange multipliers */

    bvector<double> INTWN (NTWN);
    if (!matrixReverse (&INTWN[0], numPoles))
        return MSB_ERROR;

    bvector<double> MT (numPoles*(mc+1)), MIT (numPoles*(mc+1)), MI ((mc+1)*(mc+1));
    for( i=0; i<=mc; i++ )
        {
        for( j=0; j<numPoles; j++ )
            {
            MT[j*(mc+1)+i] = M[i*numPoles+j];
            }
        }
    matrixMultiple(&INTWN[0], &MT[0], &MIT[0], numPoles, numPoles, mc+1);
    matrixMultiple(&M[0], &MIT[0], &MI[0], mc+1, numPoles, mc+1);

    bvector<double> IMIT (MI);
    if (!matrixReverse (&IMIT[0], mc+1))
        return MSB_ERROR;

    matrixPointArrayMultiple(&INTWS[0], &INTWN[0], &NTWS[0], numPoles, numPoles);
    matrixPointArrayMultiple(&V[0], &M[0], &INTWS[0], mc+1, numPoles);
    for( i=0; i<=mc; i++ )
        V[i].DifferenceOf (V[i], T[i]);
    matrixPointArrayMultiple(&A[0], &IMIT[0], &V[0], mc+1, mc+1);
    matrixPointArrayMultiple(&V[0], &MIT[0], &A[0], numPoles, mc+1);
    for( i=0; i<=n; i++ )
        {
        B.DifferenceOf (INTWS[i], V[i]);
        Pw[i] = B;
        }
    if( wp[0] < 0.0 )
        Pw[0] = R[0];
    if( wp[r] < 0.0 )
        Pw[n] = R[r];

    pOut->Allocate (numPoles, order, false, false);
    memcpy (pOut->poles, &Pw[0], numPoles*sizeof(DPoint3d));
    memcpy (pOut->knots, &knots[0], (mknt+1)*sizeof(double));
    return MSB_SUCCESS;
    }
END_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus       MSBsplineCurve::GeneralLeastSquaresApproximation
(
MSBsplineCurveP         pOut,      /* <= Output curve */
bvector<DPoint3d> const &Q,          /* => input points */
bvector<double>   const &u,          /* => Parameters corresponding to data points.*/
bvector<double>   const &knots,      /* => Knot vector of approximating curve. */
int                     numPoles,   /* => number of poles of output curve */
int                     order       /* => order of output curve */
)
    {
    MSBsplineStatus   status = MSB_SUCCESS;
    int         i, j, k, l, bw, rj, sj, ej, lk, hk, lj, hj, mk, mc, sbw;
    double      n0, np;

    int p = order - 1, n = numPoles - 1, m = (int)Q.size () - 1, n1 = n - 1, m1 = m - 1;
    mk = (int)knots.size () - 1;
    mc = n + p + 1;

    if (mk != mc)
        return MSB_ERROR;
    if (n < 2   ||  p > n  ||  m < n)
        return MSB_ERROR;

    /* Compute coefficient matrix */

    bw  = 2*p + 1;
    sbw = p;
    rj  = p;
    sj  = p - 1;
    ej  = -2;

    bvector<double> B(m1*order), N(order), NTN (n1*bw);
    bvector<int>    start(n1), end (n1), index(m1);

    memset (&NTN[0], 0, n1*bw*sizeof (double));

    for( i=0; i<=m-2; i++ )
        B[i*order+p] = 0.0;

    for( i=0; i<=std::min(p-1,n-2); i++ )
        start[i] = 0;

    end[0] = -2;

    for( i=1; i<=m-1; i++ )
        {
        status = nonZeroBasisFuncs (&N[0], j, knots, order, u[i]);
        if( MSB_SUCCESS != status )
            return MSB_ERROR;

        if( j == p             )
            l  = 1;
        else
            l  = 0;

        if( j == p  ||  j == n )
            hk = p - 1;
        else
            hk = p;

        for( k=0; k<=hk; k++ )
            B[(i-1)*order + k] = N[l+k];

        index[i-1] = std::max(0,j-p-1);

        if( j > rj )
            {
            for( k=1; k<=j-rj; k++ )
                {
                sj++;  ej++;
                if( sj <= n-2 )
                    start[sj] = i-1;
                if( ej >= 0   )
                    end[ej] = i-2;
                }
            rj = j;
            }
        }

    if( sj < n-2  ||  end[0] == -1 )
        return MSB_ERROR;

    for( i=std::max(0,ej+1); i<=n-2; i++ )
        end[i] = m-2;

    for( i=0; i<=n-2; i++ )
        {
        lj = std::max (0  ,i-p);
        hj = std::min (n-2,i+p);
        for( j=lj; j<=hj; j++ )
            {
            lk = std::max (start[i],start[j]);
            hk = std::min (end  [i],end  [j]);
            for( k=lk; k<=hk; k++ )
                {
                NTN[i*bw + j-i+sbw] += B[k*order + i-index[k]] * B[k*order + j-index[k]];
                }
            }
        }

    /* Setup and solve system of equations */

    bvector<DPoint3d>   Pw(numPoles), Rk(m1), R(n1);

    Pw[0] = Q[0];
    Pw[n] = Q[m];

    for( i=1; i<=m-1; i++ )
        {
        status = oneBasisFuncs (&n0, 0, &knots[0], mk+1, order, u[i], 0);
        if( MSB_SUCCESS != status )
            return MSB_ERROR;

        status = oneBasisFuncs (&np, n, &knots[0], mk+1, order, u[i], 0);
        if( MSB_SUCCESS != status )
            return MSB_ERROR;

        Rk[i-1].SumOf (Q[i],Q[0], -n0, Q[m], -np);
        }

    for( i=1; i<=n-1; i++ )
        {
        R[i-1].Zero ();

        lk = start[i-1];
        hk = end  [i-1];
        for( k=lk; k<=hk; k++ )
            {
            R[i-1].SumOf (R[i-1],Rk[k], B[k*order + i-index[k]-1]);
            }
        }

    if (!linearSystemSolverBanded (n1, bw, &NTN[0], &R[0])/*SUCCESS != linearSystemSolver3D (&Pw[1], n1, &NTN[0], &R[0])*/)
        return MSB_ERROR;

    for (i=1; i<n; i++)
        Pw[i] = R[i-1];

    pOut->Allocate (numPoles, order, false, false);
    memcpy (pOut->poles, &Pw[0], numPoles*sizeof(DPoint3d));
    memcpy (pOut->knots, &knots[0], (mk+1)*sizeof(double));

    return MSB_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus       MSBsplineCurve::KnotRefinement
(
bvector<double> const &X,
MSBsplineCurveP         pCurve
)
    {
    int i, j, k, l, ind, m, n, p, r, s = (int)X.size (), ns1, ms1;
    double alfa;
    n = pCurve->params.numPoles - 1;
    p = pCurve->params.order - 1;
    r = s - 1;
    m = n + p + 1;
    ns1 = n + s + 1;
    ms1 = m + s + 1;
    bvector<DPoint4d> Q(ns1), Pw(n+1);
    bvector<double>   Ubar(ms1), knots;

    for (k=0; k<n+1; k++)
        {
        if (pCurve->rational)
            Pw[k].InitFrom (*(&pCurve->poles[k]), pCurve->weights[k]);
        else
            Pw[k].InitFrom (*(&pCurve->poles[k]), 1.0);
        }

    int a, b;
    knots.assign (pCurve->knots, pCurve->knots + m+1);
    a = intervalIndex (knots, p+1, X[0]);
    b = intervalIndex (knots, p+1, X[r]);
    b++;
    for (j=0; j<=a-p; j++)
        Q[j] = Pw[j];
    for (j=b-1; j<=n; j++)
        Q[j+r+1] = Pw[j];
    for (j=0; j<=a; j++)
        Ubar[j] = knots[j];
    for (j=b+p; j<=m; j++)
        Ubar[j+r+1] = knots[j];

    i = b + p - 1;
    k = b + p + r;
    for (j=r; j>=0; j--)
        {
        while (X[j]<=knots[i] && i>a)
            {
            Q[k-p-1] = Pw[i-p-1];
            Ubar[k] = knots[i];
            k--;
            i--;
            }

        Q[k-p-1] = Q[k-p];

        for (l=1; l<=p; l++)
            {
            ind = k-p+l;
            alfa = Ubar[k+l] - X[j];
            if(fabs(alfa) < KNOT_TOLERANCE_BASIS)
                Q[ind-1] = Q[ind];
            else
                {
                alfa = alfa/(Ubar[k+l] - knots[i-p+l]);
                Q[ind-1].Interpolate (Q[ind], alfa, Q[ind-1]);
                }
            }
        Ubar[k] = X[j];
        k--;
        }

    int rational = pCurve->rational;
    int closed = pCurve->params.closed;

    pCurve->ReleaseMem ();
    pCurve->Allocate (ns1, p+1, closed == 0 ? false : true, rational == 0 ? false : true);
    for(i=0; i<ns1;i++)
        {
        pCurve->poles[i].x = Q[i].x;
        pCurve->poles[i].y = Q[i].y;
        pCurve->poles[i].z = Q[i].z;
        if (rational)
            pCurve->weights[i] = Q[i].w;
        }
    for(i=0; i<ms1;i++)
        pCurve->knots[i] = Ubar[i];

    return MSB_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int     removeMultiKnot
(
MSBsplineCurveP     pOut,
MSBsplineCurveCP    pIn,
int                 num,
int                 r,
int                 s,
double              tol
)
    {
    int         n, p;
    double      u, alfi, alfj;
    int         m, ord, fout, last, first, t, off, i, j, ii, jj, k;
    bool        remflag;

    n = pIn->params.numPoles - 1;
    p = pIn->params.order - 1;
    DPoint4d pt;

    DPoint4d* temp = (DPoint4d*)BSIBaseGeom::Malloc ((2*p+1) * sizeof(DPoint4d));
    DPoint4d* ctrlpt = (DPoint4d*)BSIBaseGeom::Malloc ((n+1) * sizeof(DPoint4d));

    for(i=0; i<=n; i++)
        {
        ctrlpt[i].x = pIn->poles[i].x;
        ctrlpt[i].y = pIn->poles[i].y;
        ctrlpt[i].z = pIn->poles[i].z;
        if (pIn->rational)
            ctrlpt[i].w = pIn->weights[i];
        else
            ctrlpt[i].w = 1.0;
        }

    u = pIn->knots[r];
    m = n + p + 1;
    ord = p + 1;

    fout = (2*r - s - p)/2;
    last = r - s;
    first = r - p;

    for(t = 0; t < num; t++)
        {
        off = first-1;
        temp[0] = ctrlpt[off];
        temp[last+1-off] = ctrlpt[last+1];

        i = first;
        j = last;
        ii = 1;
        jj = last - off;
        remflag = false;

        while( j-i > t )
            {
            alfi = (u-pIn->knots[i])/(pIn->knots[i+ord+t]-pIn->knots[i]);
            alfj = (u-pIn->knots[j-t])/(pIn->knots[j+ord]-pIn->knots[j-t]);
            pt.SumOf(ctrlpt[i], temp[ii-1], -(1.0-alfi));
            temp[ii].Scale (pt, 1.0/alfi);
            pt.SumOf(ctrlpt[j], temp[jj+1], -alfj);
            temp[jj].Scale (pt, 1.0/(1.0-alfj));

            i = i+1;
            j = j-1;
            ii = ii+1;
            jj = jj-1;
            }

        if(j-i < t)
            {
            if( temp[ii-1].RealDistance (temp[jj+1]) < tol )
                    remflag = true;
            }
        else
            {
            alfi = (u-pIn->knots[i])/(pIn->knots[i+ord+t]-pIn->knots[i]);
            pt.Interpolate (temp[ii+t+1], 1.0-alfi, temp[ii-1]);
            if( ctrlpt[i].RealDistance (pt) < tol )
                    remflag = true;
            }

        if(!remflag)
            break;

        i = first;
        j = last;

        while(j-i > t)
            {
            ctrlpt[i] = temp[i-off];
            ctrlpt[j] = temp[j-off];

            i = i+1;
            j = j-1;
            }

        first = first-1;
        last = last+1;
        }

    pOut->CopyFrom (*pIn);

    if( t == 0 )
        {
        BSIBaseGeom::Free (ctrlpt);
        BSIBaseGeom::Free (temp);
        return 0;
        }

    int numk = bspknot_numberKnots (pOut->params.numPoles, pOut->params.order, pOut->params.closed) - t;

    double* knots = (double*)BSIBaseGeom::Malloc (numk * sizeof(double));
    DPoint3d* poles = (DPoint3d*)BSIBaseGeom::Malloc ((n+1-t) * sizeof(DPoint3d));
    double *weights = (double*)BSIBaseGeom::Malloc ((n+1-t) * sizeof(double));

    if (pOut->params.numKnots > 0)
        pOut->params.numKnots -=t;
    for(k = 0; k <r+1-t; k++)
        knots[k] = pIn->knots[k];
    for(k = r+1; k <= m; k++)
        knots[k-t] = pIn->knots[k];

    i = j = fout;

    for(k = 1; k < t; k++)
        {
        if( k%2 )
            i = i+1;
        else
            j = j-1;
        }

    pOut->params.numPoles -= t;

    for(k = i+1; k <= n; k++, j++)
        {
        ctrlpt[j] = ctrlpt[k];
        }

    for(i=0; i<=n-t; i++)
        {
        poles[i].x = ctrlpt[i].x;
        poles[i].y = ctrlpt[i].y;
        poles[i].z = ctrlpt[i].z;
        weights[i] = ctrlpt[i].w;
        }

    if (pOut->rational)
        BSIBaseGeom::Free (pOut->weights);
    BSIBaseGeom::Free (pOut->knots);
    BSIBaseGeom::Free (pOut->poles);

    pOut->display.curveDisplay = true;
    pOut->display.polygonDisplay = false;
    pOut->poles = poles;
    pOut->knots = knots;

    if (pOut->rational)
        pOut->weights = weights;
    else
        BSIBaseGeom::Free (weights);

    BSIBaseGeom::Free (ctrlpt);
    BSIBaseGeom::Free (temp);
    return t;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static MSBsplineStatus   pointInversionOfCurve
(
DPoint3d&           Q,      /* <= Point on the curve closest to P */
double&             u,      /* <= Parameter corresponding to Q, i.e. Q = C(u) */
MSBsplineCurveP     cur,    /* => NURBS curve */
DPoint3d&           pnt,    /* => Point to be inverted/projected */
double              u0,     /* => Guess parameter */
double              top,    /* => Point coincidence tolerance */
double              toc     /* => Zero cosine tolerance */
)
    {
    const int   ITLIM = 4;
    bool        closed = false;
    MSBsplineStatus   status = MSB_SUCCESS;
    int         k, m, ucount;
    double      *U, num, den, der, dot, dis, best_dis, zco, uold, unew;
    DVec3d      D[3], V;

    U = cur->knots;
    m = cur->params.numPoles + cur->params.order - 1;

    /* Perform Newton interations till convergence reached */

    if ( cur->IsPhysicallyClosed(top) || cur->IsClosed ())
        closed = true;

    k    = 0;
    unew = u0;
    best_dis = fc_hugeVal;

    ucount = 0;
    while ( k < ITLIM )
        {
        /* Get derivatives */

        if (MSB_SUCCESS != (status = cur->ComputeDerivatives (D, 2, unew)))
            return status;

        /* Prepare for convergence test */

        V.DifferenceOf (D[0], pnt);
        num = V.DotProduct (D[1]);
        der = D[1].Magnitude ();

        /* Check #1: point coincidence */

        dis = V.Magnitude ();
        if ( dis < best_dis )
            {
            u = unew;
            Q = D[0];
            best_dis = dis;
            }

        if ( dis < top )
            break;

        /* Check #2: zero cosine */

        double ratio;
        if (!DoubleOps::SafeDivide (ratio, num, der*dis, fc_hugeVal))
            return MSB_ERROR;

        zco = fabs(num)/(der*dis);
        if ( zco <= toc  &&  dis == best_dis)
            break;

        /* No convergence, compute new parameter */

        dot = V.DotProduct (D[2]);
        den = dot + der*der;

        if (!DoubleOps::SafeDivide (ratio, num, den, fc_hugeVal))
            return MSB_ERROR;

        uold = unew;
        unew = uold - num/den;

        /* Check #3: parameter range */

        if ( closed )
            {
            if ( unew < U[0] )
                {
                if ( (U[0]-unew) < (100.0*(U[m]-U[0])) )
                    {
                    while ( unew < U[0] )
                        unew = U[m] - U[0] + unew;
                    ucount = 0;
                    }
                else
                    {
                    if ( ucount >= 2 )
                        return MSB_ERROR;
                    unew = uold;
                    ucount += 1;
                    }
                }
            else
                {
                if ( unew > U[m] )
                    {
                    if ( (unew-U[m]) < (100.0*(U[m]-U[0])) )
                        {
                        while( unew > U[m] )
                            unew = U[0] - U[m] + unew;
                        ucount = 0;
                        }
                    else
                        {
                        if ( ucount >= 2 )
                            return MSB_ERROR;
                        unew = uold;
                        ucount += 1;
                        }
                    }
                else
                    ucount = 0;
                }
            }
        else
            {
            if ( unew < U[0] )
                unew = U[0];
            if ( unew > U[m] )
                unew = U[m];
            }

        /* Check #4: parameter change */

        if ( fabs((unew-uold)*der) <= top  && dis == best_dis)
            break;

        k++;
        }

    /* If no convergence, error out */

    if ( k >= ITLIM )
        return MSB_ERROR;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static MSBsplineStatus   approximateG1CurvePoints
(
MSBsplineCurveP             pOut,       /* <= non-rational curve created (may be the same as pIn) */
MSBsplineCurveCP            pIn,        /* => input G1 curve */
bvector<DPoint3d> const&    P,          /* => input points */
bvector<double> const&      up,         /* => input params */
bvector<double> const&      uq,         /* => input params */
int                         p,          /* => desired degree of the pOut, 3 is recommended */
DVec3dCP                    Ts,         /* => start tangent */
DVec3dCP                    Te,         /* => end tangent */
int                         par,        /* => CHORDLENGTH = 2, CENTRIPETAL = 3 */
double                      Eg,         /* => geometric tolerance, this should be in general the chord height tol */
double                      Ep,         /* => parametric tolerance, recommand set to 10.0*geomTol */
double                      ptol        /* => point equal tolerance, recommand set to 0.01*geomTol */
)
    {
    bool        keepTangent = false, closed = false;
    int         nn, mm, i, j, k, km, kn, jj, kk, np, k1, k2, k3, m, u2size;
    double      *U1, *U2, mintol, d1, d2, d3, du, top, toc, dv, Ebnd, ktol, maxtol;
    DVec3d      Vs, Ve, V1, V2, VO;
    double      u, v;
    int         pp;
    DPoint3d    *Pw, Q, Qw;
    MSBsplineCurve  cur1, cur2, *cur3 = pOut;
    MSBsplineStatus status;
    bvector<int> I(2);
    bvector<DVec3d> D(2);
    bvector<double> wd(2);
    Vs.Init (0,0,0);
    Ve.Init (0,0,0);

    m = (int)(P.size ()) - 1;
    if (p < 2 || m < 2)
        return MSB_ERROR;

    /* Initialize some variables */

    top = ptol;
    toc = 1.e-6;
    ktol = 0.0001;

    /* Create degree 1 curve from input points */

    cur1.Zero ();
    if (MSB_SUCCESS != (status = cur1.CreateFromPointsAndOrder (&P[0], m+1, 2)))
        return status;;
    U1 = cur1.knots;
    Pw = cur1.poles;

    U1[0] = U1[1] = 0.0;
    U1[m+1] = U1[m+2] = 1.0;
    for ( i=2; i<=m; i++ )
        U1[i] = uq[i-1];

    /* Do knot removal on degree 1 curve */

    mintol = std::min(Eg, Ep);
    cur2.CopyFrom (cur1);
    cur2.RemoveKnotsBounded (60.0*mintol, 0, 0);

    /* Check for smooth closure, and get end derivatives if indicated */

    d1 = ptol + 1.0;
    if ( Ts != NULL && Te != NULL )
        keepTangent = true;
    else
        d1 = P[0].Distance (P[m]);

    VO.Zero ();
    if ( keepTangent || (d1 <= ptol && m > 2) )
        {
        d1 = U1[2]-U1[1];   /* use 3-point Bessel to get end derivatives */
        d2 = U1[3]-U1[2];
        d3 = U1[3]-U1[1];
        V1.DifferenceOf (P[1], P[0]);
        V2.DifferenceOf (P[2], P[1]);
        Vs.SumOf (*((DVec3dCP)&VO),*((DVec3dCP)&V1), (d1+d3)/(d1*d3), *((DVec3dCP)&V2), -d1/(d2*d3));
        d1 = U1[m+1]-U1[m];
        d2 = U1[m]-U1[m-1];
        d3 = U1[m+1]-U1[m-1];
        V1.DifferenceOf (P[m], P[m-1]);
        V2.DifferenceOf (P[m-1], P[m-2]);
        Ve.SumOf (*((DVec3dCP)&VO),*((DVec3dCP)&V1), (d1+d3)/(d1*d3), *((DVec3dCP)&V2), -d1/(d2*d3));

        d1 = Vs.Magnitude ();
        d2 = Ve.Magnitude ();

        if ( keepTangent )
            { /* use magnitudes from Vs and Ve */
            Vs = *Ts;
            Ve = *Te;
            Vs.Normalize ();
            Ve.Normalize ();
            Vs.Scale (d1);
            Ve.Scale (d2);
            }
        else
            {
            if ( d1 > ptol && d2 > ptol )
                {  /* get cosine of angle between end derivatives */
                d3 = Vs.DotProduct (Ve);
                d3 = d3/(d1*d2);       /* cosine */
                if ( d3 > 0.9986 )    /* 3 degrees */
                    {
                    closed = true;
                    V1.SumOf (*((DVec3dCP)&VO),*((DVec3dCP)&Vs), 0.5/d1, *((DVec3dCP)&Ve), 0.5/d2);/* average unit direction */

                    Vs.Scale (V1,d1);/* the end derivatives */
                    Ve.Scale (V1,d2);
                    }
                }
            }
        D[0] = Vs;
        D[1] = Ve;
        }

    /* Compute points for quadratic fit. Ensure that there are */
    /* at least np points in interior of each quadratic span. */

    kn = cur2.params.numPoles - 1;
    Pw = cur2.poles;
    pp = cur2.params.order - 1;
    km = kn + pp + 1;
    U2 = cur2.knots;
    np = 3;
    nn = m + np*km;   /* maximum size required */

    bvector <DPoint3d> R1(nn+1);
    bvector <double>   up1(nn+1);

    R1[0] = P[0];
    up1[0] = up[0];
    nn = 0;
    j = 1;

    for ( i=2; i<km; i++ )
        {
        k = j+1;
        while ( U1[k] < U2[i] )
            k = k+1;
        U2[i] = up[k-1];  /* so I know where to put double knots below */
        if ( k-j > np )
            {
            for ( jj=j; jj<k-1; jj++ )      /* copy points over */
                {
                nn = nn+1;
                R1[nn] = P[jj];
                up1[nn] = up[jj];
                }
            }
        else
            {
            du = (up[k-1]-up[j-1])/(np+1);  /* compute np new points */
            for ( jj=1; jj<=np; jj++ )
                {
                nn = nn+1;
                up1[nn] = up[j-1] + jj*du;
                pIn->FractionToPoint (R1[nn], up1[nn]);
                }
            }
        nn = nn+1;
        R1[nn] = P[k-1];
        up1[nn] = up[k-1];
        j = k;
        }

    int nErase = (int)R1.size () - nn - 1;
    R1.erase (R1.end() - nErase, R1.end());
    up1.erase (up1.end() - nErase, up1.end());

    /* There are nn+1 fit points. Now get new parameters for fit. */
    /* Also get error vector for knot removal. */

    bvector<double> err(nn+1), uq1(nn+1);//S_getr1d(nn,&SL);

    for ( i=0; i<=nn; i++ )
        err[i] = 0.0;
    if ( nn == m )
        {   /* same parameters as for degree 1 fit */
        for ( i=0; i<=nn; i++ )
            uq1[i] = U1[i+1];
        }
    else
        {   /* compute new parameters */
        computeParameterization (&uq1[0], nn + 1, &R1[0], par);
        }

    mm = nn;
    if ( closed || keepTangent )
        {
        err[1] = err[nn-1] = 0.1*mintol;
        nn = nn+2;
        }

    /* Kill current cur1 and prepare it for the quadratic fit */

    cur1.ReleaseMem ();
    cur1.Allocate (nn + 1, 3, 0, 0);
    U1 = cur1.knots;
    Pw = cur1.poles;

    /* Now compute the knot vector. Put a double knot at each knot */
    /* remaining from the degree 1 curve after knot removal. Also  */
    /* insert a knot at each end if derivatives are called for.    */
    /* Also insert small error at double knots so that they are    */
    /* not real easy to remove.                                    */

    j = nn+3;
    for ( i=0; i<3; i++ )
        {
        U1[i]   = uq1[0];
        U1[j--] = uq1[mm];
        }

    km = 3;   /* index in U1 */
    kn = 2;   /* index in U2 */
    kk = 1;   /* index in up1 and uq1 */

    if ( closed || keepTangent )
        {
        U1[3]  = 0.5*(uq1[0]+uq1[1]);
        U1[nn] = 0.5*(uq1[mm]+uq1[mm-1]);
        km = 4;
        }

    while ( 1 )
        {
        k = kk;
        while ( up1[k] < U2[kn] )
            k = k+1;
        k = k-1;
        for ( i=kk; i<k; i++ )
            U1[km++] = 0.5*(uq1[i]+uq1[i+1]);
        k = k+1;
        if ( k >= mm )
            break;
        else
            {
            U1[km++] = uq1[k];
            U1[km++] = uq1[k];
            kk = k+1;
            kn = kn+1;
            err[k] = 0.15*mintol;
            }
        }

    bvector<double> knt1(U1, U1+nn+4);

    cur1.ReleaseMem ();
    cur2.ReleaseMem ();
    if ( closed || keepTangent )
        {
        status = InterpolationByPointsEndsParamsAndKnots (&cur1, R1, uq1, knt1, 3, Vs, Ve);
        }
    else
        {
        status = InterpolationByPointsParamsAndKnots (&cur1, R1, uq1, knt1, 3);
        }
    if (MSB_SUCCESS != status)
        return MSB_ERROR;

    /* Now remove knots. If error = NO, then cur1 is adequate, */
    /* and we save it in cur3. */

    status = knotRemoveWithParamsEnds (&cur1, &uq1[0], &err[0], mm + 1, 0.95*mintol, 0, 0);
    if (MSB_SUCCESS != status)
        return MSB_ERROR;
    cur3->Zero ();
    cur3->CopyFrom(cur1);    /* saved curve */

    /* Least squares approximate */
    bvector<double> wp, uq2(uq1), knt3;
    /* to map old knots to new ones */
    bvector<int> pmk;
    /* compute points and parameters */
    bvector<DPoint3d> R2 (R1);

    if ( p == 2 )
        {
        nn = cur1.params.numPoles - 1;
        kk = nn + cur1.params.order;
        knt1.assign (cur1.knots, cur1.knots + kk+1);

        if ( closed || keepTangent )
            {
            wp.resize (m+1);
            wp[0] = wp[mm] = -1.0;
            for ( i=1; i<mm; i++)
                wp[i] = 1.0;
            wd[0] = wd[1] = -1.0;
            I[0] = 0;
            I[1] = mm;

            status = weightedLeastSquaresApproximation (&cur2, R1, uq1, wp, D, wd, I, knt1, nn+1, 3);
            }
        else
            {
            status = MSBsplineCurve::GeneralLeastSquaresApproximation (&cur2, R1, uq1, knt1, nn+1, 3);
            }

        if ( MSB_SUCCESS == status )
            {
            cur3->ReleaseMem ();
            cur3->CopyFrom(cur2);  /* save this one */
            }
        }
    else
        {
        nn = cur3->params.numPoles - 1;
        Pw = cur3->poles;
        pp = cur3->params.order - 1;
        km = nn + pp + 1;
        U1 = cur3->knots;

        /* Build the p-th degree knot vector */

        U2 = (double*)BSIBaseGeom::Malloc ((km*(p-2+1)+1)*sizeof (double));
        i = j = 0;
        km = km-2;
        while ( 1 )
            {
            u = U1[i];
            do
                {
                U2[j++] = U1[i++];
                }  while ( U1[i] == u );
            for ( k=2; k<p; k++ )
                U2[j++] = u;
            if ( i >= km )
                {
                for ( k=0; k<=p; k++ )
                    U2[j++] = U1[km];
                km = j-1;   /* new high index of knots */
                break;
                }
            }
        u2size = j;

        /* Build the new parameter values. May have to increase    */
        /* number of points. Want at least np points in each span. */
        np = p+1;
        uq2.resize (mm+nn*np+1);
        /* to map old knots to new ones */
        pmk.resize (km/2+1);
        /* compute points and parameters */
        R2.resize (mm+nn*np+1);

        R2[0] = R1[0];
        uq2[0] = uq1[0];
        k1 = k2 = 1;   /* indices into uq1 and uq2 */
        kk = p+1;      /* index into U2 */
        k3 = -1;       /* index into pmk (location in uq2 of p-mult knots) */

        while ( k1 <= mm )
            {
            for ( k=k1; k<=mm; k++ )
                if ( uq1[k] >= U2[kk] )
                    break;
            jj = k-k1;   /* number of parameters in interior of span */
            if ( jj >= np )
                {
                for ( /*k1=k1*/; k1<k; k1++ )
                    {
                    uq2[k2] = uq1[k1];
                    R2[k2++] = R1[k1];
                    }
                }
            else
                {
                dv = (U2[kk]-U2[kk-1])/(np+1);
                j = k1-1;
                k1 = k;
                for ( i=1; i<=np; i++ )
                    {
                    v = U2[kk-1] + i*dv;
                    while ( 1 )
                        {
                        if ( v >= uq1[j] && v <= uq1[j+1] )
                            break;
                        else
                            j = j+1;
                        }
                    u = up1[j] + ((up1[j+1]-up1[j])/(uq1[j+1]-uq1[j]))*(v-uq1[j]);
                    pIn->FractionToPoint (R2[k2], u);
                    k2 = k2+1;
                    }
                }
            if ( fabs (uq1[k1] - U2[kk]) < KNOT_TOLERANCE_BASIS )
                {
                if ( fabs (U2[kk+p-1] - U2[kk]) < KNOT_TOLERANCE_BASIS )
                    pmk[++k3] = k2;
                uq2[k2] = uq1[k1];
                R2[k2++] = R1[k1++];
                }

            if ( kk >= km-p || (U2[kk] > uq1[mm] && fabs (U2[kk] - uq1[mm]) < KNOT_TOLERANCE_BASIS) )
                break;

            while ( fabs (U2[kk] - U2[kk+1]) < KNOT_TOLERANCE_BASIS )
                kk = kk+1;

            kk = kk+1;
            }
        mm = k2-1;    /* new high index for number of points and parameters */
        nn = km-p-1;  /* new high index of control points */

        R2.erase (R2.end() - (R2.size () - k2), R2.end());
        uq2.erase (uq2.end() - (uq2.size () - k2), uq2.end());

        computeParameterization (&uq2[0], mm + 1, &R2[0], par);
        if ( k3 >= 0 )
            { /* map old knot values to new knot values */
            pmk[++k3] = mm;
            k1 = p;    /* last knot of mult GE p */
            u = U2[p]; /* old knot value at U2[k1] */
            k2 = 0;    /* index in uq2 corresponding to k1 (new value) */
            k3 = 0;
            i = p+1;
            while ( i <= km )
                {
                j = 1;
                while ( fabs (U2[i]-U2[i+j]) < KNOT_TOLERANCE_BASIS && i+j < km )
                    j += 1;
                if ( j < p )
                    {
                    i = i+j;
                    continue;
                    }
                if ( i+j == km )
                    j += 1;   /* j is multiplicity */
                v = uq2[pmk[k3]];  /* new knot value at U2[i] */
                d1 = (uq2[k2]-v)/(U2[i]-u);
                u = U2[i];
                for ( k=k1+1; k<i; k++ )
                    U2[k] = d1*(u-U2[k]) + v;
                for ( k=0; k<j; k++ )
                    U2[i+k] = v;
                k2 = pmk[k3];
                k3 += 1;
                i = i+j;
                k1 = i-1;
                }
            }

        /* Ready now to least squares fit */
        knt3.assign (U2, U2+u2size);

        if ( closed || keepTangent )
            {
            wp .resize (mm+1);
            wp[0] = wp[mm] = -1.0;
            for ( i=1; i<mm; i++)
                wp[i] = 1.0;
            wd[0] = wd[1] = -1.0;
            I[0] = 0;
            I[1] = mm;

            status = weightedLeastSquaresApproximation (&cur2, R2, uq2, wp, D, wd, I, knt3, nn+1, p+1);
            }
        else
            {
            status = MSBsplineCurve::GeneralLeastSquaresApproximation (&cur2, R2, uq2, knt3, nn+1, p+1);
            }
        if ( MSB_SUCCESS == status )
            {
            cur3->ReleaseMem ();
            cur3->CopyFrom(cur2);  /* save this one */
            }
        BSIBaseGeom::Free (U2);
        }

    cur1.ReleaseMem ();
    cur2.ReleaseMem ();

    /* cur3 contains the current best approximation. uq2 and R2 */
    /* are the corresponding mm parameters and points. */

    err.resize (mm + 1);

    /* Compute errors */

    maxtol = std::max(Eg, Ep);
    err[0] = err[mm] = 0.0;

    for ( i=1; i<mm; i++ )
        {
        d1 = 0.0;
        cur3->FractionToPoint (Q, uq2[i]);

        d1 = Q.Distance (R2[i]);

        pointInversionOfCurve (Q, v, cur3, R2[i], uq2[i], top, toc);
        d2 = Q.Distance (R2[i]);

        d1 = maxtol - Ep + d1;
        d2 = maxtol - Eg + d2;

        err[i] = std::max(d1,d2);
        }

    if ( closed || keepTangent )
        {
        pp = cur3->params.order - 1;
        if ( pp == 2 )
            {
            err[1]    = 1.1*err[1];
            err[mm-1] = 1.1*err[mm-1];
            }
        }

    /* Now force C1 */

    ktol = ktol*(uq2[mm]-uq2[0]);
    nn = cur3->params.numPoles - 1;
    Pw = cur3->poles;
    pp = cur3->params.order - 1;
    km = nn + pp + 1;
    U2 = cur3->knots;

    U1 = (double*)BSIBaseGeom::Malloc ((nn+3)*sizeof (double));/* enough space for the refinement vector */
    bvector<double> temp (mm+1);  /* enough space for temporary errors */

    np = 0;    /* total number of pp-multiplicity knots found */
    km = km-pp;
    kk = -1;   /* high index of refinement knots */
    k = 0;     /* index into uq2 */
    i = pp+1;

    while ( i < km )   /* find pp-multiplicity knots */
        {
        j = 1;
        while ( fabs (U2[i]-U2[i+j]) < KNOT_TOLERANCE_BASIS )
            j += 1;
        if ( j < pp )
            {
            i = i+j;
            continue;
            }

        i = i+j-1;    /* pp-multiplicity */
        np += 1;
        while ( uq2[k] < U2[i] )
            k += 1;
        k1 = k2 = k;
        jj = i-pp;   /* index of basis function corres. to pp-mult knot */
        while ( uq2[k1-1] > U2[jj]  )  k1 -= 1;  /* get range of parameters */
        while ( uq2[k2+1] < U2[i+1] )  k2 += 1;  /* in neighbor knot spans  */

        d1 = (U2[i]-U2[jj])/(U2[i+1]-U2[jj]);  /* get knt removal err bnd */
        DPoint3d zeroP;
        zeroP.Zero ();
        Qw.SumOf (zeroP,Pw[jj+1], d1, Pw[jj-1], 1.0-d1);
        Ebnd = Pw[jj].Distance (Qw);
        if ( Ebnd < ptol )  /* can remove without insertion */
            {
            i += 1;
            continue;
            }

        for ( j=k1; j<=k2; j++ )  /* check if can remove without insertion */
            {
            status = oneBasisFuncs (&d2, jj, cur3->knots, cur3->params.numPoles+pp+1, pp + 1, uq2[j], 0);
            temp[j] = d2*Ebnd + err[j];
            if ( MSB_SUCCESS != status || temp[j] > maxtol )
                break;
            }
        if ( j > k2 )  /* can remove without insertion */
            {
            for ( j=k1; j<=k2; j++ )  err[j] = temp[j];
            i += 1;
            continue;
            }

        d1 = (maxtol-err[k])/Ebnd;    /* must insert a knot to each */
        if ( d1 < 0.0 )  d1 = 0.0;   /* side of U2[i]              */
        if ( d1 > 1.0 )  d1 = 1.0;
        v = d1*U2[jj] + (1.0-d1)*U2[i];
        if ( v < uq2[k-1] )
            v = uq2[k-1];
        else
            if ( v > U2[i]-ktol )  v = U2[i]-ktol;
        kk += 1;
        U1[kk] = v;
        v = d1*U2[i+1] + (1.0-d1)*U2[i];
        if ( v > uq2[k+1] )
            v = uq2[k+1];
        else
            if ( v < U2[i]+ktol )  v = U2[i]+ktol;
        kk += 1;
        U1[kk] = v;
        err[k] = maxtol;   /* remove only once */

        i += 1;
        }

    if ( kk >= 0 )   /* must refine knot vector */
        {
        bvector<double> kv (U1, U1+kk+1);
        status = MSBsplineCurve::KnotRefinement (kv, cur3);
        }

    if ( np > 0 )  /* now remove one occurrence of pp-mult knots */
        {
        int       kmSize;

        i = pp + 1;
        km = cur3->params.numPoles + cur3->params.order - 1;
        U2 = cur3->knots;
        kmSize = km;
        km = km - pp;

        while ( i < km )   /* find pp-multiplicity knots */
            {
            j = 1;
            while ( i+j <= kmSize && fabs(U2[i]-U2[i+j]) < KNOT_TOLERANCE_BASIS )
                j += 1;
            if ( j < pp )
                {
                i = i+j;
                continue;
                }

            k1 = removeMultiKnot (cur3, cur3, 1, i, j, 1e10);
            i = i+j-k1;
            km = cur3->params.numPoles + cur3->params.order - 1;
            U2 = cur3->knots;
            kmSize = km;
            km = km - pp;
            }
        }

    BSIBaseGeom::Free (U1);

    /* Now knot remove one final time over entire curve */

    status = knotRemoveWithParamsEnds (cur3, &uq2[0], &err[0], mm + 1, maxtol, 0, 0);

    if ( pp == 2 && (closed || keepTangent) )
        {
        nn = cur3->params.numPoles - 1;
        Pw = cur3->poles;

        DRay3d line;
        line.InitFromOriginAndVector (R2[0], Vs);
        Q = Pw[1];
        if (line.ProjectPointUnbounded (Q, d1, Q))
            Pw[1] = Q;

        line.InitFromOriginAndVector (R2[mm], Ve);
        Q = Pw[nn-1];
        if (line.ProjectPointUnbounded (Q, d1, Q))
            Pw[nn-1] = Q;
        }

    if ( pp < p )
        status = cur3->ElevateDegree (p);


    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus   MSBsplineCurve::ApproximateG1Curve
(
MSBsplineCurveP     pOut,          /* <= non-rational curve created (may be the same as pIn) */
MSBsplineCurveCP    pIn,           /* => input G1 curve */
int                 degree,         /* => desired degree of the pOut, 3 is recommended */
bool                keepTangent,    /* => true to maintain the end tangents */
int                 parameter,      /* => CHORDLENGTH = 2, CENTRIPETAL = 3 */
double              geomTol,        /* => geometric tolerance, this should be in general the chord height tol */
double              paramTol,       /* => parametric tolerance, recommand set to 10.0*geomTol */
double              pointTol        /* => point equal tolerance, recommand set to 0.01*geomTol */
)
    {
    int p = pIn->params.order - 1;
    if (p < 2)
        return MSB_ERROR;

    bvector<DPoint3d> P;
    bvector<double> up, uq;
    if (MSB_SUCCESS != MSBsplineCurve::SampleG1CurveByPoints (P, up, uq, pIn, parameter, geomTol, pointTol))
        return MSB_ERROR;

    /* Supply end tangents if required */

    DVec3d   Ts, Te;
    DPoint3d xyz;
    double   t0, t1;
    pIn->GetKnotRange (t0, t1);

    if ( keepTangent )
        {
        pIn->FractionToPoint (xyz, Ts, t0);
        pIn->FractionToPoint (xyz, Te, t1);
        }

    return approximateG1CurvePoints (pOut, pIn, P, up, uq, degree, keepTangent? &Ts : NULL, keepTangent? &Te : NULL,
                    parameter, geomTol, paramTol, pointTol);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool         isCurveG1ContinuousAtParameter
(
MSBsplineCurveCP pCurve,
double           parameter,
double           knotTolerance,
double           parallelTolerance,
bool             isClosed
)
    {
    double rightParam = 0.0, leftParam = 1.0, dot = 0.0;

    if (!pCurve || knotTolerance < 0.0)
        return false;

    if (parameter < knotTolerance)
        parameter = 0.0;
    else if (parameter > 1.0 - knotTolerance)
        parameter = 1.0;

    if (parameter > 0.0 && parameter < 1.0)
        rightParam = leftParam = parameter;
    else if (!isClosed)
        return false;   // not continuous at endpoint

    DPoint3d xyz;
    DVec3d tangent1, tangent2;

    if (isClosed)
        {
        pCurve->FractionToPoint (xyz, tangent1, 0.0);
        pCurve->FractionToPoint (xyz, tangent2, 1.0);
        dot = tangent1.DotProduct (tangent1);
        }
    else
        {
        MSBsplineCurve cur1, cur2;
        cur1.Zero ();
        cur2.Zero ();
        cur1.CopyFractionSegment (*pCurve, 0.0, leftParam);
        cur2.CopyFractionSegment (*pCurve, rightParam, 1.0);
        cur1.FractionToPoint (xyz, tangent1, 1.0);
        cur2.FractionToPoint (xyz, tangent2, 0.0);
        dot = tangent1.DotProduct (tangent1);
        cur1.ReleaseMem ();
        cur2.ReleaseMem ();
        }

    return dot > 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static MSBsplineStatus   curveSegmentParamsByMult
(
bvector<double> &params,
MSBsplineCurveCP    curveP,
int                 mult,
double              knotTolerance
)
    {
    int         i, status, nKnots, numDistinct, *knotMultiplicityP = NULL;
    double      *distinctKnotP = NULL;

    nKnots = curveP->NumberAllocatedKnots ();

    if (NULL == (distinctKnotP = (double*)BSIBaseGeom::Malloc (nKnots * sizeof(double))) ||
        NULL == (knotMultiplicityP = (int*)BSIBaseGeom::Malloc (nKnots * sizeof(int))))
        return MSB_ERROR;

    // get start/end knots too
    if (MSB_SUCCESS == (status = bsputil_getKnotMultiplicityExt (distinctKnotP, knotMultiplicityP, &numDistinct, curveP->knots,
                                                                curveP->params.numPoles, curveP->params.order, curveP->params.closed,
                                                                knotTolerance)))
        {
        for (i = 0; i < numDistinct; i++)
            if (knotMultiplicityP[i] >= mult)
                params.push_back (distinctKnotP[i]);
        }

    BSIBaseGeom::Free (distinctKnotP);
    BSIBaseGeom::Free (knotMultiplicityP);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     isCurveG1Continuous
(
MSBsplineCurveCP    pCurve,
double              parallelTolerance
)
    {
    double          knotTolerance;
    bool            bIsContinuous = false;
    bvector<double>params;
    if (!pCurve)
        return false;

    knotTolerance = bspknot_knotTolerance (pCurve);

    // get only those (unnormalized) knots whose multiplicity guarantees at least C-(degree-1) continuity
    if (MSB_SUCCESS == curveSegmentParamsByMult (params, pCurve, pCurve->params.order - 1, knotTolerance))
        {
        double      startKnot, endKnot, knotSpan;
        size_t      i, firstParam = 0;
        size_t numParams = params.size ();

        if (numParams == 0)
            return true;

        pCurve->GetKnotRange (startKnot, endKnot);
        knotSpan = endKnot - startKnot;
        if (knotSpan > 0.0 && knotSpan != 1.0)
            for (i = 0; i < numParams; i++)
                params[i] = (params[i] - startKnot) / knotSpan;

        startKnot = 0.0;
        endKnot   = 1.0;
        knotSpan  = 1.0;
        bIsContinuous = true;

        // special handling for startKnot: process only when closed bit is set
        if (params[0] - startKnot < knotTolerance)
            {
            if (pCurve->params.closed)
                bIsContinuous = isCurveG1ContinuousAtParameter (pCurve, startKnot, knotTolerance, parallelTolerance, true);

            // don't process startKnot again
            ++firstParam;
            }

        if (bIsContinuous)
            {
            // don't process endKnot
            if (endKnot - params[numParams - 1] < knotTolerance)
                --numParams;

            for (i = firstParam; i < numParams && bIsContinuous; i++)
                bIsContinuous = isCurveG1ContinuousAtParameter (pCurve, params[i], knotTolerance, parallelTolerance, false);
            }

        }

    return bIsContinuous;
    }

/*---------------------------------------------------------------------------------**//**
* EDL May 2018 recode for single bezier input.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    secondDerivativeBoundBezierCurve
(
double&          Muu,
MSBsplineCurveCR    curve
)
    {
    int p = curve.params.order - 1;
    int   r, a, al;
    double *U, ui, muu;
    DPoint3d xyz;
    DVec3d tangent;


    /* Compute maximum derivative */
    double u[MAX_BEZIER_ORDER];

    Muu = 0.0;
    al = 0;

    r = curve.params.numPoles + p;
    U = curve.knots;

    ui   = (U[r]-U[0])/p;
    u[0] = U[0];
    u[p] = U[r];

    for( a=1; a<=p-1; a++ )  u[a] = U[0] + a*ui;

    for( a=al; a<=p; a++ )
        {
        curve.FractionToPoint (xyz, tangent, u[a]);

        muu = tangent.Magnitude ();

        if( muu > Muu )
            Muu = muu;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void  computeKnotVectorEnds
(
bvector <double>&   knots,
bvector <double>    u,
int                     p
)
    {
    int   i, j, n, k = (int)u.size () - 1;
    double sum;

    /* Compute knot vector */

    n = k+2;
    knots.resize (n+p+2);
    for( i=0; i<=p; i++ )
        {
        knots[  i  ] = u[0];
        knots[n+i+1] = u[k];
        }

    if ( p == 1 )
        {
        if ( k > 1 )
            {
            knots[2] = 0.5 * (u[0] + u[1]);
            knots[n] = 0.5 * (u[k] + u[k-1]);
            }
        else
            {
            knots[2] = 0.75*u[0] + 0.25*u[1];
            knots[3] = 0.25*u[0] + 0.75*u[1];
            }

        for (i=1; i<k; i++)
            knots[i+2] = u[i];
        }
    else
        {
        for (i=0; i<=k-p+1; i++)
            {
            sum = 0.0;
            for (j=i; j<=i+p-1; j++)
                sum += u[j];
            knots[i+p+1] = sum/p;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void  computeKnotVector
(
bvector <double>&   knots,
bvector <double>    u,
int                     p
)
    {
    int   i, j, n = (int)u.size () - 1;
    double sum;

    knots.resize (n+p+2);

    /* Compute knot vector */

    for( i=0; i<=p; i++ )
        {
        knots[  i  ] = u[0];
        knots[n+i+1] = u[n];
        }

    for( i=1; i<=n-p; i++ )
        {
        sum = 0.0;
        for( j=i; j<=i+p-1; j++ )
            sum += u[j];
        knots[i+p] = sum/p;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus   MSBsplineCurve::ApproximateNurbsCurve
(
MSBsplineCurveP     pOut,          /* <= non-rational curve created (may be the same as pIn) */
MSBsplineCurveCP    pIn,           /* => input G1 curve */
int                 degree,         /* => desired degree of the pOut, 3 is recommended */
bool                keepTangent,    /* => true to maintain the end tangents */
int                 parametrization,/* => CHORDLENGTH = 2, CENTRIPETAL = 3 */
double              tol             /* => geometric tolerance, this should be in general the chord height tol */
)
    {
    MSBsplineStatus status = MSB_ERROR;
    int       i, j, k, l, mp, mu, der, p = pIn->params.order - 1;
    double    *UP, Muu, ul, ur, uinc, f1, f2, del, num, gro, exp;
    DPoint3d  xyz;
    DVec3d    Tangent, Ts, Te;
    bvector<MSBsplineCurvePtr> bez;
    bvector<double> knu, u;
    bvector<DPoint3d> P;

    // Bug #811970: NLib indexing stops at an array's highest index, not before its size (cf. N_appgca)
    mp = pIn->NumberAllocatedKnots() - 1;
    UP = pIn->knots;

    if (degree == 1)
        exp = 0.5;
    else
        exp = 0.34;

    if (tol < fc_tinyVal)
        return MSB_ERROR;

    f1 = 1.0/tol;
    f1 = pow(f1, exp);
    f2 = (double) degree;

    pIn->CopyAsBeziers (bez);
    k = (int)bez.size () - 1;

    /* Get number of sampling points */
    bvector<int> nu(k+1);
    mu = 0;
    for( i=0; i<=k; i++ )
        {
        bez[i]->MapKnots (0.0, 1.0);

        if (!secondDerivativeBoundBezierCurve (Muu, *bez[i]))
            goto wrapup;

        num   = 0.125*Muu;
        gro   = std::max (f2, sqrt(num));
        del   = f1*gro;
        del   = ceil (del);
        nu[i] = (int) del;
        mu   += nu[i];
        }

    /* Compute parameters */
    u.resize (mu+1);
    ul = UP[p];  i = p+1;  j = 0;  u[0] = ul;  l = 0;

    while( i < mp )
        {
        while( i < mp  &&  fabs (UP[i] - UP[i+1])< KNOT_TOLERANCE_BASIS )
            i++;
        ur = UP[i];

        uinc = (ur - ul)/nu[j];
        for( k=1; k<nu[j]; k++ )
            u[++l] = ul + k*uinc;
        u[++l] = ur;

        ul = ur;  i++;  j++;
        }

    /* Sample curve */
    P.resize (mu+1);

    for( i=0; i<=mu; i++ )
        pIn->FractionToPoint (P[i], u[i]);

    /* Get knot vector */
    if( keepTangent )
        j = mu + degree + 3;
    else
        j = mu + degree + 1;

    knu.resize (j+1);
    computeParameterization (&u[0], mu+1, &P[0], parametrization);

    if( keepTangent )
        computeKnotVectorEnds (knu, u, degree);
    else
        computeKnotVector (knu, u, degree);

    /* Interpolate points */

    if( keepTangent )
        {
        /* Get end tangents */
        pIn->FractionToPoint (xyz, Tangent, UP[0]);
        Ts = Tangent;

        pIn->FractionToPoint (xyz, Tangent, UP[mp]);
        Te = Tangent;

        /* Adjust magnitudes */
        del = pIn->Length ();

        Ts.Normalize ();
        Te.Normalize ();
        Ts.Scale (del);
        Te.Scale (del);

        status = InterpolationByPointsEndsParamsAndKnots (pOut, P, u, knu, degree + 1, Ts, Te);
        }
    else
        {
        status = InterpolationByPointsParamsAndKnots (pOut, P, u, knu, degree + 1);
        }

    if( MSB_SUCCESS != status )
        goto wrapup;

    /* Remove knots */
    if( keepTangent &&  degree == 2 )
        der = 1;
    else
        der = 0;

    status = pOut->RemoveKnotsBounded (tol, der == 1? 1 : 0, der == 1? 1 : 0);
wrapup:
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus   MSBsplineCurve::ApproximateAnyCurve
(
MSBsplineCurveCP        pIn,
double                  tolerance,
int                     order,
int                     parametrization,
bool                    bMaintainEndTangents
)
    {
    static double   s_coarseParallelTolerance = 1.0e-5;
    static int      s_poleThreshholdForG1 = 500;

    // approx degenerate rational curve explicitly to get around NLIB's N_gprcle hanging
    if (isRationalDegenerateCurve (this, pIn, tolerance))
        return MSB_SUCCESS;

    MSBsplineCurve  tmpCurve;
    tmpCurve.Zero ();
    double          t0, t1;
    MSBsplineStatus status;
    pIn->GetKnotRange (t0, t1);
    if (pIn->IsClosed ())
        {
        tmpCurve.CopyOpen (*pIn, t0);
        pIn = &tmpCurve;
        }

    // faster to use alternate function if G1, unless too many poles (cf. TR #193490): solving the large non-banded system is slow!
    if (pIn->params.numPoles < s_poleThreshholdForG1 && isCurveG1Continuous (pIn, s_coarseParallelTolerance))
        status = MSBsplineCurve::ApproximateG1Curve (this, pIn, order - 1, bMaintainEndTangents, parametrization, tolerance, 10.0 * tolerance, 0.01 * tolerance);
    else
        status = MSBsplineCurve::ApproximateNurbsCurve (this, pIn, order - 1, bMaintainEndTangents, parametrization, tolerance);

    tmpCurve.ReleaseMem ();
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::InitFromLeastSquaresFit
(
DPoint3dCP      points,     // => data to be interpolated
int             numPoints,  // => number of points
bool            endControl, // => true: pass both ends, false: do not require both end points
DVec3dCP        sTangent,   // => start tangent (optional)
DVec3dCP        eTangent,   // => end tangent (optional)
bool            keepTanMag, // => keep end tangent's magnitude
int             iterDegree, // => start iteration degree
int             reqDegree,  // => required degree of output curve
bool            singleKnot, // => use single knots
double          tolerance   // => fitting tolerance
)

    {
    const double    top = 1.0e-02, toc = 1.0e-03;
    MSBsplineStatus status = MSB_SUCCESS;
    bool            apr;
    int             k, l, nc, mc, nh, mh, mk, nsp, mlt, i, j, m = numPoints - 1, ps = iterDegree, pr = reqDegree;
    double          *U, d, ltop, ltoc;
    DPoint3d        R;
    DVec3d          As, Ae;
    As.Zero ();
    Ae.Zero ();
    bvector<double> wd(2);
    bvector<int>    I(2);
    bvector<DVec3d> D(2);

    if( pr > m || pr< ps)
        return MSB_ERROR;

    d = 0;
    for( i=1; i<=m; i++ )
        d += points[ i-1].Distance (points[ i]);

    ltop = d*top;
    ltoc = toc;

    /* Get tangents or derivatives */

    if( sTangent != NULL )
        {
        As = *sTangent;
        if( !keepTanMag )
            {
            As.Normalize ();
            As.Scale (d);
            }
        }

    if( eTangent != NULL )
        {
        Ae = *eTangent;
        if( !keepTanMag )
            {
            Ae.Normalize ();
            Ae.Scale (d);
            }
        }

    if (sTangent == NULL && eTangent == NULL)
        endControl = false;

    if( endControl )
        {
        D[0] = As;
        D[1] = Ae;

        wd[0] = -1.0;  wd[1] = -1.0;
        I[0]  = 0;     I[1]  = m;
        }

    bvector<double> u (m+1), us (m+1), er (m+1), wp (m+1), knt;
    bvector<DPoint3d> P;

    P.assign (points, points + m+1);

    /* Set up constraint array */

    for( i=0; i<=m; i++ )
        wp[i] = 1.0;

    if(endControl)
        wp[0] = wp[m] = -1.0;

    /* Get parameters */

    computeParameterization (&us[0], m+1, points, 2);

    /*********************************************************/
    /*  Fit curve as follows:                                */
    /*    for i=ps to pr do                                  */
    /*       interpolate with degree i                       */
    /*       for j=i to pr do                                */
    /*          remove knots                                 */
    /*          compute degree elevated curve's knot vector  */
    /*          least-squares approximate                    */
    /*          if approximation fails break                 */
    /*          adjust parameters and error vector           */
    /*********************************************************/

    for( i=ps; i<=pr; i++ )
        {
        /* Interpolate with degree i */

        for( k=0; k<=m; k++ )
            {
            u [k] = us[k];
            er[k] = 0.0;
            }

        if( !endControl )
            {
            computeKnotVector (knt, u, i);
            status = InterpolationByPointsParamsAndKnots (this, P, u, knt, i+1);
            }
        else
            {
            computeKnotVectorEnds (knt, u, i);
            status = InterpolationByPointsEndsParamsAndKnots (this, P, u, knt, i+1, As, Ae);
            }

        if( MSB_SUCCESS != status )
            return status;

        for( j=i; j<=pr; j++ )
            {
            /* Remove as many knots as possible */

            if ( pr <= 2 )
                status = knotRemoveWithParamsEnds (this, &u[0], &er[0], m+1, tolerance, endControl ? 3 : 0, 1);
            else
                status = knotRemoveWithParamsEnds (this, &u[0], &er[0], m+1, tolerance, 0, 0);

            if( MSB_SUCCESS != status )
                return status;

            if( j == pr )
                break;

            U = knots;

            nc = NumberAllocatedPoles () - 1;
            mc = NumberAllocatedKnots () - 1;

            /* Get new knot vector */

            if( singleKnot )
                {
                nh = nc + 1;
                mh = mc + 2;
                if( nh > m )
                    break;

                mk = mh + 1;
                for( k=0;  k<=j+1; k++ )  U[mh--] = U[mc];
                for( k=nc; k>=j+1; k-- )  U[mh--] = U[k];
                for( k=0;  k<=j+1; k++ )  U[mh--] = U[0];
                }
            else
                {
                nsp = 0;
                for( k=j; k<mc-j; j++ )
                    {
                    if( U[k] != U[k+1] )
                        nsp++;
                    }

                nh = nc + nsp;
                mh = mc + nsp+1;
                if( nh > m )
                    break;

                mk = mh + 1;;
                while( mc > 0 )
                    {
                    k = mc;
                    while( mc > 0  &&  fabs (U[mc] - U[mc-1]) < KNOT_TOLERANCE_BASIS )
                        mc--;
                    mlt = k-mc+1;

                    for( l=1; l<=mlt+1; l++ )
                        U[mh--] = U[mc];
                    mc--;
                    }
                }
            knt.assign (U, U + mk);

            /* Approximate by least-squares */
            ReleaseMem ();
            status = weightedLeastSquaresApproximation (this, P, u, wp, D, wd, I, knt, nh+1, j+2);
            if( MSB_SUCCESS != status )
                break;

            /* Adjust parameters and error vector */

            apr = true;
            for( k=1; k<m; k++ )
                {
                status = pointInversionOfCurve (R, u[k], this, P[k], u[k], ltop, ltoc);
                if( MSB_SUCCESS != status )
                    {
                    apr = false;
                    break;
                    }

                er[k] = P[k].Distance (R);

                if( er[k] > tolerance )
                    {
                    apr = false;
                    break;
                    }
                }
            if( !apr )
                break;
            }

        if( j == pr )
            break;

        if( MSB_SUCCESS == status )
            ReleaseMem ();
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::InitFromGeneralLeastSquares
(
double          *avgDistance,
double          *maxDistance,
BsplineParam    info,
bvector<double> *knts,
DPoint3d        *pnts,
double          *uValues,
int             numPnts
)
    {
    int         left, row, col, i, j, k, numPoles, order, numPoints,
                allocSize, status = MSB_SUCCESS;
    double      *u, *matrix, *distance, *rhs, *rhsP0, *rhsP1, *rhsP2, *dP, *endD,
                bfuncs[MAX_ORDER], totalDistance, divisor;
    DPoint3d    temp, *points, *pP, *endP;
    size_t      knotIndex;

    u = matrix = distance = rhs = NULL; points = NULL;
    params = info;
    numPoles = params.numPoles;
    order = params.order;

    /* Check ok params for number of data points */
    if (numPoles < order)
        return (MSB_ERROR);
    if (numPnts < numPoles)
        return (MSB_ERROR);
    if (params.numKnots && !knts)
        return (MSB_ERROR);

    /* Allocate memory */
    allocSize = (numPnts + (params.closed ? 1 : 0)) * sizeof(DPoint3d);
    if (NULL == (points = (DPoint3d*)BSIBaseGeom::Malloc (allocSize)))
        return MSB_ERROR;

    /* Get local copy of data points, removing repeated points to keep matrix good */
    points[0] = pnts[0];
    for (numPoints=1, pP=pnts+1; pP < pnts+numPnts; pP++)
        if (!bsputil_isSamePoint (points+numPoints-1, pP))
            points[numPoints++] = *pP;

    if (numPoints < numPoles)   /* not enough distinct data points */
        {
        BSIBaseGeom::Free (points);
        return (MSB_ERROR);
        }

    if (params.closed && !bsputil_isSamePoint (points, points+numPoints-1))
        points[numPoints++] = points[0];

    allocSize = numPoles * sizeof(DPoint3d);
    AllocatePoles (numPoles);
    memset (poles, 0, allocSize);

    if (NULL == (rhs = static_cast<double *>(BSIBaseGeom::Malloc (allocSize))))
        goto wrapup;

    allocSize = numPoles * numPoles * sizeof(double);
    if (NULL == (matrix = static_cast<double *>(BSIBaseGeom::Malloc (allocSize))))
        goto wrapup;
    memset (matrix, 0, allocSize);

    allocSize = numPoints * sizeof(double);
    if (NULL == (u = static_cast<double *>(BSIBaseGeom::Malloc (allocSize))))
        goto wrapup;

    /* Allocate memory and compute knot vector if it is not supplied */
    if (!knts)
        {
        allocSize= bspknot_numberKnots (numPoles, order, params.closed);
        AllocateKnots (allocSize);
        bspknot_computeKnotVector (knots, &params, NULL);
        }
    else
        AllocateKnots ((int)knts->size (), &knts->at (0));
    /* If uValues is passed use it, else calculate parameter values for
        approximation points; i.e. normalized cumulative length */
    if (uValues)
        {
        memcpy (u, uValues, numPnts * sizeof (double));
        if (params.closed)
            u[numPoints-1] = 1.0;
        }
    else
        {
        for (i=1, u[0] = 0.0; i < numPoints; i++)
            u[i] =  u[i-1] + points[i].Distance (points[i-1]);
        divisor = u[numPoints-1];
        for (dP=endD=u, endD += numPoints; dP < endD; dP++)
            *dP /= divisor;
        }

    /* Construct system of equations to be solved */
    /* If open curve, interpolate end points */
    if (!params.closed)
        {
        matrix[0] = matrix[numPoles*numPoles-1] = 1.0;
        poles[0] = points[0];
        poles[numPoles-1] = points[numPoints-1];
        }

    for (i=0; i < numPoints; i++)
        {
        /* Calculate blending functions at u[i] */
        KnotToBlendFunctions (bfuncs, NULL, knotIndex, u[i]);
        left = (int)knotIndex;

        /* Add to equations remembering that only order bfuncs are non-zero */
        for (j=0, row = (left - order + numPoles) % numPoles; j < order; j++, row++)
            {
            if (params.closed || (0 < row && row < numPoles-1))
                {
                temp.Scale (points[i], bfuncs[j]);
                poles[row%numPoles].Add (temp);
                for (k=0, col = (left - order + numPoles) % numPoles;
                     k < order; k++, col++)
                    matrix[row%numPoles*numPoles + col%numPoles] += bfuncs[j] * bfuncs[k];
                }
            }
        }

    /* Solve system of equations */
    for (rhsP0=rhs, rhsP1=rhs+numPoles, rhsP2=rhsP1+numPoles,
         pP=endP=poles, endP += numPoles;
         pP < endP; rhsP0++, rhsP1++, rhsP2++, pP++)
        {
        *rhsP0 = pP->x;    *rhsP1 = pP->y;    *rhsP2 = pP->z;
        }

    if (SUCCESS != (status = (bsiLinAlg_solveLinearGaussPartialPivot (matrix, numPoles, rhs, 3) ? SUCCESS : ERROR)))
        goto wrapup;

    for (rhsP0=rhs, rhsP1=rhs+numPoles, rhsP2=rhsP1+numPoles,
         pP=endP=poles, endP += numPoles;
         pP < endP; rhsP0++, rhsP1++, rhsP2++, pP++)
        {
        pP->x = *rhsP0;    pP->y = *rhsP1;    pP->z = *rhsP2;
        }

    /* Calculate error if requested */
    if (maxDistance || avgDistance)
        {
        allocSize = numPoints * sizeof(double);
        if (NULL == (distance = static_cast<double *>(BSIBaseGeom::Malloc (allocSize))))
            {
            status = MSB_ERROR;
            goto wrapup;
            }
        for (i=0, totalDistance = 0.0; i < numPoints; i++)
            {
            KnotToBlendFunctions (bfuncs, NULL, knotIndex, u[i]);
            left = (int)knotIndex;

            temp = DPoint3d::FromXYZ (0.0, 0.0, 0.0);
            for (j=0, k = (left - order + numPoles) % numPoles;
                 j < order; j++, k++)
                {
                temp.SumOf (temp, poles[k % numPoles], bfuncs[j]);
                }
            totalDistance += distance[i] = points[i].Distance (temp);
            }

        if (maxDistance)
            {
            for (dP=endD=distance, endD += numPoints, *maxDistance=0.0; dP < endD; dP++)
                {
                if (*dP > *maxDistance)
                    *maxDistance = *dP;
                }
            }

        *avgDistance = totalDistance / (numPoints-1);
        }

    /* Return B-Spline curve */
    type = BSCURVE_GENERAL;
    rational = FALSE;
    display.curveDisplay = TRUE;
    weights = NULL;

wrapup:
    if (points)         BSIBaseGeom::Free (points);
    if (u)              BSIBaseGeom::Free (u);
    if (matrix)         BSIBaseGeom::Free (matrix);
    if (distance)       BSIBaseGeom::Free (distance);
    if (rhs)            BSIBaseGeom::Free (rhs);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::WeightedLeastSquaresFit
(
MSBsplineCurveP         pOut,
bvector<DPoint3d>   Q,
bvector<double>     u,
bool                    endControl,
DVec3dCP                sTangent,
DVec3dCP                eTangent,
int                     numPoles,
int                     order
)
    {
    int i, m = (int)Q.size (), mm = m - 1;
    bvector<double> knt (numPoles+order);

    //compute knot vector.
    for (i=0; i<order; i++)
        knt[i] = 0.0;
    for (i=(int)knt.size () - 1; i >= numPoles; i--)
        knt[i] = 1.0;

    double d = double(m)/(numPoles - order + 1), alpha;
    for (int j = 1; j <= numPoles-order; j++)
        {
        i = int (j*d);
        alpha = j*d - i;
        knt[order+j-1] = (1.0-alpha)*u[i-1] +alpha*u[i];
        }

    if (endControl)
        {
        bvector<int> I(2);
        bvector<DVec3d> D(2);
        bvector<double> wd(2), wp (m);
        wp[0] = wp[mm] = -1.0;
        for ( i=1; i<mm; i++)
            wp[i] = 1.0;
        wd[0] = wd[1] = -1.0;
        I[0] = 0;
        I[1] = mm;
        D[0] = *sTangent;
        D[1] = *eTangent;

        double length = 0;
        for( i=1; i<m; i++ )
            length += Q[i-1].Distance(Q[i]);

        D[0].Normalize ();
        D[0].Scale (length);
        D[1].Normalize ();
        D[1].Scale (length);

        return weightedLeastSquaresApproximation (pOut, Q, u, wp, D, wd, I, knt, numPoles, order);
        }
    else
        return GeneralLeastSquaresApproximation (pOut, Q, u, knt, numPoles, order);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::InitFromInterpolatePoints
(
DPoint3dCP      points,
int             numPoints,
int             parametrization,
bool            endControl,
DVec3dCP        sTangent,
DVec3dCP        eTangent,
bool            keepTanMag,
int             order
)
    {
    if ((endControl && order > numPoints+2) || (!endControl && order > numPoints))
        return MSB_ERROR;

    int i;
    bvector<double> u (numPoints), myKnots;
    bvector<DPoint3d> Q (points, points + numPoints);

    computeParameterization (&u[0], numPoints, points, parametrization);

    if (endControl && sTangent != NULL && eTangent != NULL)
        {
        computeKnotVectorEnds (myKnots, u, order - 1);
        DVec3d Ts, Te;

        Ts = *sTangent;
        Te = *eTangent;

        if (!keepTanMag)
            {
            double length = 0;

            for( i=1; i<numPoints; i++ )
                length += Q[i-1].Distance(Q[i]);

            Ts.Normalize ();
            Te.Normalize ();
            Ts.Scale (length);
            Te.Scale (length);
            }

        return InterpolationByPointsEndsParamsAndKnots (this, Q, u, myKnots, order, Ts, Te);
        }
    else
        {
        computeKnotVector (myKnots, u, order - 1);
        return InterpolationByPointsParamsAndKnots (this, Q, u, myKnots, order);
        }
    }

MSBsplineCurvePtr MSBsplineCurve::CreateFromInterpolationAtGrevilleKnots
(
ICurvePrimitiveCR curve,
size_t numPoles,
int order,
bool normalizeKnots,
int knotSelector        // 0==> uniform, 1==>Chebyshev
)
    {
    bvector<DPoint3d> interpolationPoints;
    bvector<double> grevilleDistances;
    bvector<double> curveKnots;
    double totalLength;
    if (!curve.Length(totalLength))
        return nullptr;
    bool grevilleKnotsAreSimpleEvaluationFractions = false;
    if ((int)numPoles < order)
        numPoles = order;
    if (knotSelector == 1)
        {
        bvector<double>chebyshev;
        DoubleOps::ChebyshevPoints(chebyshev, numPoles + 2 - order, false, order, order);
        curveKnots = chebyshev;
        DoubleOps::LinearMapFrontBackToInterval (curveKnots, 0.0, totalLength);
        DoubleOps::MovingAverages (grevilleDistances, curveKnots, order - 1, 1, 1);
        }
    else if (knotSelector == 100)
        {
        DoubleOps::MovingAverages(grevilleDistances, 0.0, 1.0, numPoles, order - 1, order -1 , order - 1);
        DoubleOps::MovingAverages (curveKnots, 0.0, 1.0, numPoles + order, 1, order, order);
        grevilleKnotsAreSimpleEvaluationFractions = true;
        }
    else
        {
        DoubleOps::MovingAverages(grevilleDistances, 0.0, totalLength, numPoles, order - 1, order -1 , order - 1);
        DoubleOps::MovingAverages (curveKnots, 0.0, totalLength, numPoles + order, 1, order, order);
        }
    BeAssert (grevilleDistances.size () == numPoles);
    double startFraction = 0.0;
    DPoint3d xyz;
    curve.FractionToPoint (0.0, xyz);
    interpolationPoints.push_back (xyz);

    for (size_t i = 1; i + 1 < grevilleDistances.size (); i++)
        {
        if (grevilleKnotsAreSimpleEvaluationFractions)
            {
            curve.FractionToPoint (grevilleDistances[i], xyz);
            interpolationPoints.push_back (xyz);
            }
        else
            {
            double delta = grevilleDistances[i] - grevilleDistances[i-1];
            CurveLocationDetail detail;
            if (!curve.PointAtSignedDistanceFromFraction (startFraction, delta, false, detail))
                return nullptr;
            interpolationPoints.push_back (detail.point);
            startFraction = detail.fraction;
            }
        }
    curve.FractionToPoint(1.0, xyz);
    interpolationPoints.push_back (xyz);
    return MSBsplineCurve::CreateFromInterpolationPointsWithKnots (interpolationPoints,
                grevilleDistances, curveKnots, (size_t)order);
    }
