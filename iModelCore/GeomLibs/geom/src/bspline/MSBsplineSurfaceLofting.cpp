/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bspline/MSBsplineSurfaceLofting.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <memory.h>
#include <stdlib.h>
#include <math.h>
#define MDLERR_NOPOLES ERROR
#define MDLERR_INSFMEMORY ERROR

#include "msbsplinemaster.h"

#if !defined BIGD
#define      BIGD         1.7976931348623158e+308
#endif

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static void curmwp 
(
double              &wMin,
double              &pMax,
MSBsplineCurveCP    pCurve
)
    {
    int i, num = pCurve->params.numPoles;
    double length;
    wMin = pCurve->weights[0];
    DVec3d vec;
    DPoint3d pt;
    bsiDPoint3d_scale (&pt, &pCurve->poles[0], 1.0 / pCurve->weights[0]);
    bsiDVec3d_fromDPoint3d (&vec, &pt);
    pMax = bsiDVec3d_magnitude (&vec);
    
    for (i=1; i<num; i++)
        {
        if (wMin > pCurve->weights[i])
            wMin = pCurve->weights[i];

        bsiDPoint3d_scale (&pt, &pCurve->poles[i], 1.0 / pCurve->weights[i]);
        bsiDVec3d_fromDPoint3d (&vec, &pt);
        length = bsiDVec3d_magnitude (&vec);
        if (pMax < length)
            pMax = length;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Peter.Yu                        11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static int makeCurvesCompatibleFast
(
MSBsplineCurve  *pOut,          /* <= array of input curves, shuld be allocated before calling this */
MSBsplineCurve  *pIn,           /* => array of input curves */
int             numCurves,      /* => number of curves */
double          tol,            /* => approximation tolerance, set to zero for precise compatibility */
int             tangentControl, /* => 0: no tangent control, 1: start point, 2: end point, 3: Both ends */
bool            keepMagnitude,  /* => true: derivatives maintained at specified ends, false: only directions */
int             derivative      /* => Highest derivatives maintained. Ignored when keepMagnitude is false */
)
    {
    MSBsplineStatus status;

    bool rat = false, rmf, wfl;
    const double NOREM = 1.0e+25;
    const double WMIN = 1e-5;
    const double WMAX = 200.0;

    int   i, j, k, l, ll, ii, jj, first, last, off, n, ns, m, r, s = 0, fout, p, kk = numCurves - 1;

    double    *U, *a, wmin, wmax, pmax, alf, bet, lam = 0.0, oml = 0.0, bsum, wi, wj;

    bvector<DPoint4d> Pw;

    if (tangentControl > 3)
        return MSB_ERROR;

    MSBsplineCurve **inC = static_cast<MSBsplineCurve**>(BSIBaseGeom::Malloc (numCurves * sizeof(MSBsplineCurve*)));
    MSBsplineCurve **outC = static_cast<MSBsplineCurve**>(BSIBaseGeom::Malloc (numCurves * sizeof(MSBsplineCurve*)));

    for (i = 0; i < numCurves; i++)
        {
        inC[i] = &pIn[i];
        outC[i] = &pOut[i];
        }
    if (MSB_SUCCESS != (status = bspcurv_makeCurvesCompatible (outC, inC, numCurves, 1, 1)))
        {
        BSIBaseGeom::Free (outC);
        BSIBaseGeom::Free (inC);
        return status;
        }

    /************************************/
    /* Remove as many knots as possible */
    /************************************/

    /* Adjust knot removal tolerance for rational curves */

    n = pOut[kk].NumberAllocatedPoles () - 1;
    for (i=0; i<=n; i++)
        Pw.push_back (pOut[kk].GetPoleDPoint4d (i));
    p = pOut[kk].GetIntOrder () - 1;
    m = pOut[kk].NumberAllocatedKnots () - 1;
    U = pOut[kk].knots;
    ns = n;

    if( pOut[0].rational )
        {
        curmwp (wmin, pmax, &pOut[0]);
        alf = (tol*wmin)/(1.0+pmax);

        for( ll=1; ll<=kk; ll++ )
            {
            curmwp(wmin, pmax, &pOut[ll]);
            bet = (tol*wmin)/(1.0 + pmax);
            if( bet < alf )  
                alf = bet;
            }
        tol = alf;
        rat = true;
        }

    /* Get local memory */

    bvector<bvector<DPoint4d> > Rw(numCurves);
    bvector<bvector<double> > br(numCurves), er(numCurves), te(numCurves);
    for (i=0; i<numCurves; i++)
        {
        Rw[i].resize (2*p+1);
        br[i].resize (m+1);
        er[i].resize (m+1);
        te[i].resize (m+1);
        }
    bvector<double> b(kk+1), alfs(2*p+1), omas(2*p+1), bets(2*p+1), ombs(2*p+1);
    bvector<int> sr(m+1);

    /* Initialize */

    for( j=0; j<=m; j++ )
        {
        sr[j] = 0;
        for( ll=0; ll<=kk; ll++ )
            {
            br[ll][j] = BIGD;
            er[ll][j] = 0.0;
            }
        }

    /* Compute knot removal errors for each distinct knot for each curve */

    r = p+1;
    while( r <= n )
        {
        i = r;
        while( r <= n  &&  U[r] == U[r+1] )  r++;
        sr[r] = r-i+1;

        for( ll=0; ll<=kk; ll++ )
            {
            br[ll][r] = MSBsplineCurve::GetRemovalKnotBound (&pOut[ll], r, sr[r]);
            }

        r++;
        } 

    /* Try to remove each knot from each curve */

    while( 1 )
        {
        /* Find knot with smallest error sum */

        bsum = BIGD;
        for( j=p+1; j<=m-p-1; j++ )
            {
            if( br[0][j] != BIGD  &&  br[0][j] != NOREM )
                {
                alf = 0.0;
                for( ll=0; ll<=kk; ll++ )  alf += br[ll][j];
        
                if( alf < bsum )
                    {
                    bsum = alf;  s = sr[j];  r = j;

                    for( ll=0; ll<=kk; ll++ )  b[ll] = br[ll][j];
                    }
                }
            }

        /* If no more removable knot -> finished */

        if( bsum == BIGD )  break;

        /* Check constraints */

        if( !keepMagnitude )
            {
            if( p <= 2 )  derivative = 1;  else  derivative = 0;
            }

        if( derivative >= 1 )
            {
            rmf = true;
            if( tangentControl == 1  ||  tangentControl == 3 )
                { 
                if( r <= (p+derivative) )  rmf = false;
                }

            if( tangentControl == 2  ||  tangentControl == 3 )
                { 
                if( r >= (n-derivative+1) )  rmf = false;
                }

            if( !rmf )  
                {  
                for( ll=0; ll<=kk; ll++ )  br[ll][r] = NOREM;  
                continue;  
                }
            }

        /* Check error of removal */

        if( (p+s)%2 )
            {
            k   = (p+s+1)/2;
            l   = r-k+p+1;
            alf = (U[r]-U[r-k  ])/(U[r-k+p+1]-U[r-k  ]);
            bet = (U[r]-U[r-k+1])/(U[r-k+p+2]-U[r-k+1]);
            lam = alf/(alf+bet);
            oml = 1.0-lam; 
            }
        else
            {
            k = (p+s)/2;
            l = r-k+p;
            }

        rmf = true;
        for( j=r-k; j<=l; j++ )
            {
            if( U[j] != U[j+1] )  
                { 
                for( ll=0; ll<=kk; ll++ )
                    {
                    te[ll][j] = er[ll][j]+b[ll]; 
                    if( te[ll][j] > tol )  {  rmf = false;  break;  }
                    }
                }

            if( !rmf )  break;
            }

        /* If error test passed -> update error vector and remove knot */

        if( rmf )
            {
            for( j=r-k; j<=l; j++ )  
                {
                if( U[j] != U[j+1] )
                    {
                    for( ll=0; ll<=kk; ll++ )  er[ll][j] = te[ll][j];
                    }
                }

            fout  = (2*r-s-p)/2;    
            first = r-p;    
            last  = r-s;
            off   = first-1;        

            /* Save alphas and betas */

            i = first;  
            j = last;
            while( (j-i) > 0 )
                {
                alfs[i-first] = (U[i+p+1]-U[i])/(U[r]-U[i]);
                omas[i-first] = 1.0-alfs[i-first];
                bets[j-first] = (U[j+p+1]-U[j])/(U[j+p+1]-U[r]);
                ombs[j-first] = 1.0-bets[j-first];
                i ++;  j --;  
                }
            
            /* Remove the knot from all curves */

            wfl = true;
            for( ll=0; ll<=kk; ll++ )
                {
                for (int iTemp=0; iTemp<pOut[ll].NumberAllocatedPoles (); iTemp++)
                    Pw[iTemp] = pOut[ll].GetPoleDPoint4d (iTemp);
                a = pOut[ll].knots;

                i  = first;  
                j  = last;
                ii = 1;
                jj = last-off;

                Rw[ll][0] = Pw[off];
                Rw[ll][last+1-off] = Pw[last+1];

                /* Get new control points for one removal step */

                while( (j-i) > 0 )
                    {
                    Rw[ll][ii].SumOf (Pw[i], alfs[i-first], Rw[ll][ii-1], omas[i-first]);
                    Rw[ll][jj].SumOf (Pw[j], bets[j-first], Rw[ll][jj+1], ombs[j-first]);
                    i ++;  j --;  
                    ii++;  jj--;
                    }
            
                /* Check for disallowed weights */

                if( rat )
                    {
                    i    = first;
                    j    = last;
                    wmin = BIGD;
                    wmax = fc_1em15;
                    while( (j-i) > 0 )
                        {
                        wi = Rw[ll][i-off].w;
                        wj = Rw[ll][j-off].w;

                        if( wi < wmin )  wmin = wi;
                        if( wj < wmin )  wmin = wj;
                        if( wi > wmax )  wmax = wi;
                        if( wj > wmax )  wmax = wj;
                        i++;  j--;
                        }
                    if( wmin < WMIN  ||  wmax > WMAX )  {  wfl = false;  break;  }
                    }

                if( (p+s)%2 )
                    {
                    Rw[ll][jj+1].SumOf (Rw[ll][jj+1],  lam, Rw[ll][ii-1], oml);
                    }
                } /* End for each curve */
        
            /* Check weights */

            if( !wfl )
                {
                for( ll=0; ll<=kk; ll++ )  br[ll][r] = NOREM;
                continue;
                }
            else
                {
                /* Save control points */
  
                for( ll=0; ll<=kk; ll++ )
                    {
                    for (int iTemp=0; iTemp<pOut[ll].NumberAllocatedPoles (); iTemp++)
                        Pw[iTemp] = pOut[ll].GetPoleDPoint4d (iTemp);
                    a = pOut[ll].knots;

                    i = first;
                    j = last;
                    while( (j-i) > 0 )
                        {
                        Pw[i] = Rw[ll][i-off];
                        Pw[j] = Rw[ll][j-off];
                        i++;  j--;
                        }
                    }
                }
        
            /* Shift down some parameters */

            if( s == 1 )  
                {
                for( ll=0; ll<=kk; ll++ )  
                    {
                    er[ll][r-1] = std::max(er[ll][r-1], er[ll][r]);
                    }
                }

            if( s > 1 )  sr[r-1] = sr[r]-1;

            for( j=r+1; j<=m; j++ )
                {
                sr[j-1] = sr[j];
                for( ll=0; ll<=kk; ll++ )
                    {
                    br[ll][j-1] = br[ll][j];
                    er[ll][j-1] = er[ll][j];
                    }
                }
        
            /* Shift down knots and control points */

            for( ll=0; ll<=kk; ll++ )
                {
                for (int iTemp=0; iTemp<pOut[ll].NumberAllocatedPoles (); iTemp++)
                        Pw[iTemp] = pOut[ll].GetPoleDPoint4d (iTemp);
                U = pOut[ll].knots;

                for( j=r+1; j<=m; j++ )  U[j-1] = U[j];

                for( j=fout+1; j<=n; j++ )
                    {
                    Pw[j-1] = Pw[j];
                    }

                pOut[ll].params.numPoles = n;
                for (int cpi = 0; cpi < n; cpi++)
                    {
                    pOut[ll].poles[cpi].x = Pw[cpi].x;
                    pOut[ll].poles[cpi].y = Pw[cpi].y;
                    pOut[ll].poles[cpi].z = Pw[cpi].z;
                    if (rat)
                        pOut[ll].weights[cpi] = Pw[cpi].w;
                    }
                }
            n--;   m--;  

            /* If no more internal knots -> finished */

            if( n == p )  break;

            /* Update error bounds */

            k = std::max(r-p,p+1);
            l = std::max(n,r+p-s);
            for( j=k; j<=l; j++ )
                {
                if( U[j] != U[j+1] ) 
                    { 
                    for( ll=0; ll<=kk; ll++ )
                        {
                        if( br[ll][j] != NOREM )
                            {
                            br[ll][j] = MSBsplineCurve::GetRemovalKnotBound(&pOut[ll], j, sr[j]);
                            }
                        }
                    }
                }
            }
        else
            {
            /* Knot is not removable */

            for( ll=0; ll<=kk; ll++ )  br[ll][r] = NOREM;
            }

        } /* End of while loop */
    
    BSIBaseGeom::Free (outC);
    BSIBaseGeom::Free (inC);
    return MSB_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Peter.Yu                        11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bspcurv_makeCurvesCompatibleWithConsByApprox
(
MSBsplineCurve  *pOut,          /* <= array of input curves, shuld be allocated before calling this */
MSBsplineCurve  *pIn,           /* => array of input curves */
int             numCurves,      /* => number of curves */
double          tolerance,      /* => approximation tolerance, set to zero for precise compatibility */
int             tangentControl, /* => 0: no tangent control, 1: start point, 2: end point, 3: Both ends */
bool            keepMagnitude,  /* => true: derivatives maintained at specified ends, false: only directions */
int             derivative,     /* => Highest derivatives maintained. Ignored when keepMagnitude is false */
bool            fastMode        /* => true: to remove less data but faster */
)
    {
    return makeCurvesCompatibleFast (pOut, pIn, numCurves, tolerance, tangentControl, keepMagnitude, derivative);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          allPointsCoincide                                       |
|                                                                       |
| author        LuHan                                   9/93            |
|                                                                       |
+----------------------------------------------------------------------*/
static bool    allPointsCoincide
(
DPoint3d        *points,        /* weighted points if weights != NULL */
double          *weights,       /* weights or NULL */
int             numPts,         /* number of points */
bool            *shift          /* if true, the coincident points are shifted */
)
    {
    double      shiftDist;
    DPoint3d    *pP, *endP;

    if (weights)
        bsputil_unWeightPoles (points, points, weights, numPts);

    shiftDist = 0.0001;
    for (pP = endP = points, endP += numPts-1; pP < endP; pP++)
        if (bsiDPoint3d_distance (pP, pP+1) > 0.00001)
            {
            if (*shift)
                for (pP = endP = points, endP += numPts-1; pP < endP; pP++)
                    if (bsiDPoint3d_distance (pP, pP+1) < 0.00001)
                        {
                        pP->x -= shiftDist;
                        shiftDist /= 2.0;
                        *shift = false;
                        }

            if (weights)
                bsputil_weightPoles (points, points, weights, numPts);
            return (false);
            }

    if (weights)
        bsputil_weightPoles (points, points, weights, numPts);

    return (true);
    }

END_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Peter.Yu                        11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineSurface::LoftingSurface 
(
MSBsplineCurveP     pCurves,     
DVec3dP             pStartNormal,
DVec3dP             pEndNormal,  
int                 numCurves,    
bool                approxComp,   
bool                closed,       
bool                smoothStart,  
bool                smoothEnd,    
bool                chordLength,  
bool                applyComp,    
double              tolerance     
)
    {
    int             status = SUCCESS;

    if (numCurves == 2)
        {
        MSBsplineCurve  ruleCurves[2];

        if (SUCCESS == (status = bspcurv_makeCurvesCompatibleWithConsByApprox (
            ruleCurves, pCurves, 2,
            (approxComp == true) ? tolerance : 0.0, 3, true, 1, false)))
            status = bspsurf_ruledSurfaceFromCompatibleCopiesOfCurves (this, &ruleCurves[0], &ruleCurves[1]);

        bspcurv_freeCurve (&ruleCurves[0]);
        bspcurv_freeCurve (&ruleCurves[1]);
        }
    else
        {
        bool            shift;
        int             i, j, numU, numV;
        double          *wPts = NULL, *wPoles = NULL;
        DPoint3d        *pPts = NULL, *pPoles = NULL;
        MSBsplineCurve  *cvArray = NULL;

        if (NULL == (cvArray = static_cast<MSBsplineCurve*>(BSIBaseGeom::Malloc (numCurves * sizeof(MSBsplineCurve)))))
            return MDLERR_INSFMEMORY;

        if (applyComp == true)
            {
            if (SUCCESS != (status = bspcurv_makeCurvesCompatibleWithConsByApprox (
                cvArray, pCurves, numCurves,
                (approxComp == true) ? tolerance : 0.0, 3, true, 1, true)))
                {
                BSIBaseGeom::Free (cvArray);
                return  status;
                }
            }
        else
            {
            for (i = 0; i < numCurves; i++)
                {
                bspcurv_copyCurve (&cvArray[i], &pCurves[i]);
                }
            }

        /* Create the surface from the cross sections */
        Zero ();
        rational = cvArray[0].rational;
        numU = cvArray[0].params.numPoles;
        uParams = cvArray[0].params;
        vParams.closed = false;
        vParams.order = 4;
        numV = vParams.numPoles = numCurves + 2;
        vParams.numKnots = numCurves - 2;

        if (SUCCESS != (status = Allocate ()))
            goto wrapup;

        /* Malloc some tmp variables */
        pPts = pPoles = NULL;
        wPts = wPoles = NULL;
        if (NULL == (pPts = static_cast<DPoint3d*>(BSIBaseGeom::Malloc (numCurves * sizeof(DPoint3d)))) ||
            NULL == (pPoles = static_cast<DPoint3d*>(BSIBaseGeom::Malloc (numV * sizeof(DPoint3d)))))
            {
            status = MDLERR_INSFMEMORY;
            goto wrapup;
            }

        if (rational)
            if (NULL == (wPts = static_cast<double*>(BSIBaseGeom::Malloc (numCurves*sizeof(double)))) ||
                NULL == (wPoles = static_cast<double*>(BSIBaseGeom::Malloc (numV*sizeof(double)))))
                {
                status = MDLERR_INSFMEMORY;
                goto wrapup;
                }

        /* Compute the poles and weights if rational */
        for (i = 0; i < numU; i++)
            {
            for (j=0; j<numCurves; j++)
                {
                pPts[j] = cvArray[j].poles[i];
                if (rational) wPts[j] = cvArray[j].weights[i];
                }

            /* Check if all input data points conincide to one single point */
            shift = true;
            if (allPointsCoincide (pPts, rational ? wPts : NULL, numCurves, &shift))
                {
                for (j=0; j<numV; j++)
                    {
                    poles[j*numU+i] = pPts[0];
                    if (rational)
                        weights[j*numU+i] = wPts[0];
                    }
                }
            else
                {
                if (SUCCESS != (status = bspcurv_c2CubicInterpolatePoles (
                                            pPoles,
                                            rational ?  wPoles : NULL, NULL, /* Knot */
                                            NULL, /* in params */
                                            pPts, NULL, rational ?  wPts : NULL,
                                            &vParams,
                                            numCurves)))
                    goto wrapup;

                for (j=0; j<numV; j++)
                    {
                    poles[j*numU+i] = pPoles[j];
                    if (rational)
                        weights[j*numU+i] = wPoles[j];
                    }
                }
            }

        /* Compute the u knot vector */
        memcpy (uKnots, cvArray[0].knots,
                bspknot_numberKnots (numU, cvArray[0].params.order,
                cvArray[0].params.closed) * sizeof(double));

        /* Compute the v knot vector uniformly */
        vParams.numKnots = 0;
        bspknot_computeKnotVector (vKnots, &vParams, NULL);

wrapup:
        for (i = 0; i < numCurves; i++)
            bspcurv_freeCurve (&cvArray[i]);
        if (pPts)
            BSIBaseGeom::Free (pPts);
        if (wPts)
            BSIBaseGeom::Free (wPts);
        if (pPoles)
            BSIBaseGeom::Free (pPoles);
        if (wPoles)
            BSIBaseGeom::Free (wPoles);
        if (cvArray)
            BSIBaseGeom::Free (cvArray);
        }

    /* Swap surface U and V direction so that V direction has more poles */
    if (SUCCESS == status)
        {
        if (uParams.numPoles > MAX_POLES &&
            vParams.numPoles <= MAX_POLES)
            SwapUV ();

        display.curveDisplay = true;
        display.polygonDisplay = false;
        uParams.numRules = vParams.numPoles;
        vParams.numRules = uParams.numPoles;
        }

    return status;
    }
