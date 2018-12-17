/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bspline/MSBsplineCurve_Modify.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <memory.h>
#include <stdlib.h>
#include <math.h>
#if defined (INCLUDE_PPL)
    #include <Bentley\Iota.h>
    #include <ppl.h>
    //#define USE_PPL
    #if !defined (USE_PPL)
        #include <algorithm>
    #endif
#endif
#include "msbsplinemaster.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::MakeOpen (double u)
    {
    return CopyOpen (*this, u);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    earlin.lutz                     04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineCurve::UnWeightPoles()
    {
    if (rational && weights != NULL)
        bsputil_unWeightPoles (poles, poles, weights, params.numPoles);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    earlin.lutz                     04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineCurve::WeightPoles()
    {
    if (rational && weights != NULL)
        bsputil_weightPoles (poles, poles, weights, params.numPoles);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::MakeClosed ()
    {
    return CopyClosed (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::MakeReversed ()
    {
    return CopyReversed (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::ElevateDegree (int newDegree)
    {
    return bspcurv_elevateDegree (this, this, newDegree);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineCurve::CopyAsBeziers (bvector<MSBsplineCurvePtr> &beziers) const
    {
    BCurveSegment segment;

    beziers.clear ();

    for (size_t spanIndex = 0; GetBezier (segment, spanIndex); spanIndex++)
        {
         if (!segment.IsNullU ())
            {
            MSBsplineCurve curve;
            curve.Zero ();
            curve.InitFromDPoint4dArray (segment.GetPoleP (), params.order, params.order);
            curve.display = display;
            beziers.push_back (curve.CreateCapture ());
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::MakeRational ()
    {
    return bspcurv_makeRational (this, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/08
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::AppendCurves (MSBsplineCurveCR inCurve1, MSBsplineCurveCR inCurve2, bool forceContinuity, bool reparam)
    {
    return bspcurv_appendCurves (this, (MSBsplineCurveP) &inCurve1, (MSBsplineCurveP) &inCurve2, forceContinuity, reparam);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static void weightSumDPoint4d
(

DPoint4d&       Cw, 
double          alpha, 
DPoint4d        Aw, 
double          beta, 
DPoint4d        Bw 
)
    {
    Cw.x = alpha * Aw.x + beta * Bw.x;
    Cw.y = alpha * Aw.y + beta * Bw.y;
    Cw.z = alpha * Aw.z + beta * Bw.z;
    Cw.w = alpha * Aw.w + beta * Bw.w;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static double distanceDPoint4d
(
DPoint4d    Pw, 
DPoint4d    Qw 
)
    {
    double  distance2; 

    distance2  = (Pw.x - Qw.x)*(Pw.x - Qw.x) + (Pw.y - Qw.y)*(Pw.y - Qw.y);
    distance2 += (Pw.z - Qw.z)*(Pw.z - Qw.z);
    distance2 += (Pw.w - Qw.w)*(Pw.w - Qw.w);

    return sqrt(distance2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
double MSBsplineCurve::GetRemovalKnotBound
(
MSBsplineCurveCP    pCurve, 
int                 index, 
int                 mult
)
    {
    int degree = pCurve->params.order - 1;
    int i  = index - degree;
    int j  = index - mult;
    int ii = 1;
    int jj = degree - mult + 1;
    int numPoles = pCurve->params.numPoles;
    double alf, oma, bet, omb, *knots = pCurve->knots;
    bvector<DPoint4d> tmpPoles (2*degree+1);
    bvector<DPoint4d> polesWeighted (numPoles);
    
    for (int k=0; k<numPoles; k++)
        {
        if (pCurve->rational)
            polesWeighted[k].InitFrom (*(&pCurve->poles[k]), pCurve->weights[k]);
        else
            polesWeighted[k].InitFrom (*(&pCurve->poles[k]), 1.0);
        }
    
    if (pCurve->rational)
        {
        tmpPoles[0].InitFrom (*(&pCurve->poles[index-degree-1]), pCurve->weights[index-degree-1]);
        tmpPoles[degree-mult+2].InitFrom (*(&pCurve->poles[index-mult+1]), pCurve->weights[index-mult+1]);
        }
    else
        {
        tmpPoles[0].InitFrom (*(&pCurve->poles[index-degree-1]), 1.0);
        tmpPoles[degree-mult+2].InitFrom (*(&pCurve->poles[index-mult+1]), 1.0);
        }
    
    while(j-i > 0)
        {
        alf = (knots[i+degree+1] - knots[i])/(knots[index] - knots[i]);
        oma = 1.0 - alf;
        bet = (knots[j+degree+1] - knots[j])/(knots[j+degree+1] - knots[index]);
        omb = 1.0 - bet;
        weightSumDPoint4d(tmpPoles[ii], alf, polesWeighted[i], oma, tmpPoles[ii-1]);
        weightSumDPoint4d(tmpPoles[jj], bet, polesWeighted[j], omb, tmpPoles[jj+1]);
        i ++;  j --;  
        ii++;  jj--;
        }
    
    if((j-i) < 0)
        {
        return distanceDPoint4d(tmpPoles[ii-1], tmpPoles[jj+1]);
        }
    else
        {
        DPoint4d A;
        alf = (knots[index]-knots[i])/(knots[i+degree+1]-knots[i]);
        oma = 1.0-alf;

        weightSumDPoint4d(A, alf, tmpPoles[ii+1], oma, tmpPoles[ii-1]);
        return distanceDPoint4d(polesWeighted[i], A);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static MSBsplineStatus removeKnotsWithConstraints 
(
MSBsplineCurveP     pCurve,
int                 num,  // Highest index in u
double              *u,   // Parameter values at which constraints are applied
int                 *cst, // cst[i] means: 1 : constrain C(u[i]), 2: constrain C'(u[i]), 3 : constrain both
double              tol   // Tolerance to check removability
)
    {
    const double WMIN = 1e-5;
    const double WMAX = 200.0;
    int i, j, k, ii, jj, first, last, off, n, m, r, s, fout, l, ns, lp, rp, lt, rt, left = 0, right = 0, p, nu, norem = 0;
    bool rmf, rem, rat = false;
    p = pCurve->params.order - 1;
    n = pCurve->params.numPoles - 1;
    m = n + p +1;
    ns = n;
    nu = num - 1;
    
    bvector<DPoint4d> tmpPoles (2*p+1);
    bvector<DPoint4d> polesWeighted (n+1);
    bvector<double> br (m+1), er (m+1), te (m+1); // br, er and te is the error bound, error and total error for one removal step respectively.
    bvector<int> sr (m+1), nr (m+1); // sr is the index of knot try to remove; nr[i] = 1 means i-th knot cannot be removed.
    double *UQ = pCurve->knots;
    double b, alf, oma, bet, omb, lam, oml = 0.0, wi, wj, wmin, wmax;
    
    if (pCurve->rational)
        {
        double wMin, pMax;
        curveMinWeightAndMaxProjectedMagnitude (wMin, pMax, pCurve);
        tol *= wMin/(1.0+pMax);
        rat = true;
        }
    
    for (k=0; k<n+1; k++)
        {
        if (pCurve->rational)
            polesWeighted[k].InitFrom (*(&pCurve->poles[k]), pCurve->weights[k]);
        else
            polesWeighted[k].InitFrom (*(&pCurve->poles[k]), 1.0);
        }
    
    for( i=0; i<=m; i++ )
        {
        br[i] = fc_hugeVal;
        sr[i] = 0;
        nr[i] = 0;
        er[i] = 0.0;
        }
    
    r = p + 1;
    while( r <= n )
        {
        i = r;
        while( r <= n && fabs (UQ[r]-UQ[r+1]) < RELATIVE_BSPLINE_KNOT_TOLERANCE)
            r++;
        sr[r] = r-i+1;
        br[r] = MSBsplineCurve::GetRemovalKnotBound (pCurve, r, sr[r] );
        r++;
        } 
    
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
                b = br[i];  s = sr[i];  r = i;  norem = nr[i];
                }
            }
        
        if( norem == 1 )  break;
        
        lp = std::max(p  ,r-p-1  );
        rp = std::min(n+1,r+p-s+2);
        lt = std::max(p  ,r-p    );
        rt = std::min(n+1,r+p-s+1);
        
        rem = true;
        for( i=0; i<=nu; i++ )
            {
            if(cst[i] == 1 || cst[i] == 3)  
                {   
                left = lp;
                right = rp;
                }
            if(cst[i] == 2) 
                {  
                left = lt;  
                right = rt;
                }
            if( u[i] > UQ[left] && u[i] < UQ[right] )  
                {  
                rem = false;  
                break;  
                }
            }
            
        if( !rem )  
            {
            nr[r] = 1;  
            continue;  
            }
            
            /* Check error of removal */
        lam = 0.0;
        if( (p+s)%2 )
            {
            k   = (p + s + 1)/2;
            l   = r - k + p + 1;
            alf = (UQ[r] - UQ[r-k  ])/(UQ[r-k+p+1] - UQ[r-k  ]);
            bet = (UQ[r] - UQ[r-k+1])/(UQ[r-k+p+2] - UQ[r-k+1]);
            lam = alf/(alf + bet);
            oml = 1.0 - lam; 
            }
        else
            {
            k = (p + s)/2;
            l = r - k + p;
            }
        
        rmf = true;
        for( i=r-k; i<=l; i++ )
            {
            if( fabs (UQ[i] - UQ[i+1]) > RELATIVE_BSPLINE_KNOT_TOLERANCE )  
                { 
                te[i] = er[i] + b; 
                if(te[i] > tol )  
                    {  
                    rmf = false;  
                    break;  
                    }
                }
            }
        
        if( rmf )
            {
            for( i=r-k; i<=l; i++ )  
                {
                if( fabs (UQ[i] - UQ[i+1]) > RELATIVE_BSPLINE_KNOT_TOLERANCE )  
                er[i] = te[i];
                }

            fout  = (2*r-s-p)/2;    
            first = r-p;    
            last  = r-s;
            off   = first-1;        
            i     = first;  
            j     = last;
            ii    = 1;
            jj    = last-off;
          
            tmpPoles[0] = polesWeighted[off];
            tmpPoles[last+1-off] = polesWeighted[last+1];
            
            /* Get new control points for one removal step */

            while( (j-i) > 0 )
                {
                alf = (UQ[i+p+1]-UQ[i])/(UQ[r]-UQ[i]);
                oma = 1.0-alf;
                bet = (UQ[j+p+1]-UQ[j])/(UQ[j+p+1]-UQ[r]);
                omb = 1.0-bet;
                weightSumDPoint4d(tmpPoles[ii], alf, polesWeighted[i], oma, tmpPoles[ii-1]);
                weightSumDPoint4d(tmpPoles[jj], bet, polesWeighted[j], omb, tmpPoles[jj+1]);
                i ++;  j --;  
                ii++;  jj--;
                }

            /* Check for disallowed weights */

            if( rat )
                {
                i    = first;
                j    = last;
                wmin = fc_hugeVal;
                wmax = fc_1em15;
                while( (j-i) > 0 )
                    {
                    wi = tmpPoles[i-off].w;
                    wj = tmpPoles[j-off].w;

                    if( wi < wmin )  wmin = wi;
                    if( wj < wmin )  wmin = wj;
                    if( wi > wmax )  wmax = wi;
                    if( wj > wmax )  wmax = wj;
                    i++;  j--;
                    }
                    
                if( wmin < WMIN || wmax > WMAX )  
                    {
                    nr[r] = 1;  
                    continue;  
                    }
                }
                
            /* Save new control points */

            if( (p+s)%2 )
                {
                weightSumDPoint4d(tmpPoles[jj+1], lam, tmpPoles[jj+1], oml, tmpPoles[ii-1]);
                }
            
            i = first;
            j = last;
            while( (j-i) > 0 )
                {
                polesWeighted[i] = tmpPoles[i-off];
                polesWeighted[j] = tmpPoles[j-off];
                i++;  j--;
                }
            
            /* Shift down some parameters */

            if( s == 1 )  er[r-1] = std::max(er[r-1], er[r]);
            if( s > 1 )  sr[r-1] = sr[r]-1;

            for( i=r+1; i<=m; i++ )
                {
                br[i-1] = br[i];
                sr[i-1] = sr[i];
                er[i-1] = er[i];
                }
            
            /* Shift down knots and control points */

            for( i=r+1;    i<=m; i++ )  UQ[i-1] = UQ[i];
            for( i=fout+1; i<=n; i++ )  polesWeighted[i-1] = polesWeighted[i];

            n--;  m--; 
            
            pCurve->params.numPoles = n+1;
            for (int cpi = 0; cpi < n+1; cpi++)
                {
                pCurve->poles[cpi].x = polesWeighted[cpi].x;
                pCurve->poles[cpi].y = polesWeighted[cpi].y;
                pCurve->poles[cpi].z = polesWeighted[cpi].z;
                if (rat)
                    pCurve->weights[cpi] = polesWeighted[cpi].w;
                }
            
            /* If no more internal knots -> finished */

            if( n == p )  break;
            
            k = std::max (r-p, p+1);
            l = std::min (n, r+p-s);
            for( i=k; i<=l; i++ )
                {
                if( fabs (UQ[i] - UQ[i+1]) > RELATIVE_BSPLINE_KNOT_TOLERANCE && nr[i] != 1 ) 
                    { 
                    br[i] = MSBsplineCurve::GetRemovalKnotBound (pCurve, i, sr[i]);
                    }
                }
                
            }
        else
            {
            /* Knot is not removable */
            nr[r] = 1;
            }
        }
        
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static MSBsplineStatus removeKnotsNoConstraints 
(
MSBsplineCurveP     pCurve,
double              tol
)
    {
    const double WMIN = 1e-5;
    const double WMAX = 200.0;
    int i, j, k, ii, jj, first, last, off, n, m, r, s, fout, l, ns, p, norem = 0;
    bool rmf, rat = false;
    p = pCurve->params.order - 1;
    n = pCurve->params.numPoles - 1;
    m = n + p +1;
    ns = n;
    
    bvector<DPoint4d> tmpPoles (2*p+1);
    bvector<DPoint4d> polesWeighted (n+1);
    bvector<double> br (m+1), er (m+1), te (m+1);// br, er and te is the error bound, error and total error for one removal step respectively.
    bvector<int> sr (m+1), nr (m+1); // sr is the index of knot try to remove; nr[i] = 1 means i-th knot cannot be removed.
    double *UQ = pCurve->knots;
    double b, alf, oma, bet, omb, lam = 0.0, oml = 0.0, wi, wj, wmin, wmax;
    
    if (pCurve->rational)
        {
        double wMin, pMax;
        curveMinWeightAndMaxProjectedMagnitude (wMin, pMax, pCurve);
        tol *= wMin/(1.0+pMax);
        rat = true;
        }
    
    for (k=0; k<n+1; k++)
        {
        if (pCurve->rational)
            polesWeighted[k].InitFrom (*(&pCurve->poles[k]), pCurve->weights[k]);
        else
            polesWeighted[k].InitFrom (*(&pCurve->poles[k]), 1.0);
        }
    
    for( i=0; i<=m; i++ )
        {
        br[i] = fc_hugeVal;
        sr[i] = 0;
        nr[i] = 0;
        er[i] = 0.0;
        }
    
    r = p + 1;
    while( r <= n )
        {
        i = r;
        while( r <= n && UQ[r] == UQ[r+1])
            r++;
        sr[r] = r-i+1;
        br[r] = MSBsplineCurve::GetRemovalKnotBound (pCurve, r, sr[r] );
        r++;
        } 
    
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
                b = br[i];  s = sr[i];  r = i; norem = nr[i];
                }
            }
        
        if( norem == 1 )  break;

        /* Check error of removal */

        if( (p+s)%2 )
            {
            k   = (p + s + 1)/2;
            l   = r - k + p + 1;
            alf = (UQ[r] - UQ[r-k  ])/(UQ[r-k+p+1] - UQ[r-k  ]);
            bet = (UQ[r] - UQ[r-k+1])/(UQ[r-k+p+2] - UQ[r-k+1]);
            lam = alf/(alf + bet);
            oml = 1.0 - lam; 
            }
        else
            {
            k = (p + s)/2;
            l = r - k + p;
            }
        
        rmf = true;
        for( i=r-k; i<=l; i++ )
            {
            if( UQ[i] != UQ[i+1] )  
                { 
                te[i] = er[i] + b; 
                if(te[i] > tol )  
                    {  
                    rmf = false;  
                    break;  
                    }
                }
            }
        
        if( rmf )
            {
            for( i=r-k; i<=l; i++ )  
                {
                if(UQ[i] != UQ[i+1] )  
                er[i] = te[i];
                }

            fout  = (2*r-s-p)/2;    
            first = r-p;    
            last  = r-s;
            off   = first-1;        
            i     = first;  
            j     = last;
            ii    = 1;
            jj    = last-off;
          
            tmpPoles[0] = polesWeighted[off];
            tmpPoles[last+1-off] = polesWeighted[last+1];
            
            /* Get new control points for one removal step */

            while( (j-i) > 0 )
                {
                alf = (UQ[i+p+1]-UQ[i])/(UQ[r]-UQ[i]);
                oma = 1.0-alf;
                bet = (UQ[j+p+1]-UQ[j])/(UQ[j+p+1]-UQ[r]);
                omb = 1.0-bet;
                weightSumDPoint4d(tmpPoles[ii], alf, polesWeighted[i], oma, tmpPoles[ii-1]);
                weightSumDPoint4d(tmpPoles[jj], bet, polesWeighted[j], omb, tmpPoles[jj+1]);
                i ++;  j --;  
                ii++;  jj--;
                }

            /* Check for disallowed weights */

            if( rat )
                {
                i    = first;
                j    = last;
                wmin = fc_hugeVal;
                wmax = fc_1em15;
                while( (j-i) > 0 )
                    {
                    wi = tmpPoles[i-off].w;
                    wj = tmpPoles[j-off].w;

                    if( wi < wmin )  wmin = wi;
                    if( wj < wmin )  wmin = wj;
                    if( wi > wmax )  wmax = wi;
                    if( wj > wmax )  wmax = wj;
                    i++;  j--;
                    }
                    
                if( wmin < WMIN || wmax > WMAX )  
                    {  
                    nr[r] = 1;
                    br[r] = fc_hugeVal;
                    continue;  
                    }
                }
                
            /* Save new control points */

            if( (p+s)%2 )
                {
                weightSumDPoint4d(tmpPoles[jj+1], lam, tmpPoles[jj+1], oml, tmpPoles[ii-1]);
                }
            
            i = first;
            j = last;
            while( (j-i) > 0 )
                {
                polesWeighted[i] = tmpPoles[i-off];
                polesWeighted[j] = tmpPoles[j-off];
                i++;  j--;
                }
            
            /* Shift down some parameters */

            if( s == 1 )  er[r-1] = std::max(er[r-1], er[r]);
            if( s > 1 )  sr[r-1] = sr[r]-1;

            for( i=r+1; i<=m; i++ )
                {
                br[i-1] = br[i];
                sr[i-1] = sr[i];
                er[i-1] = er[i];
                }
            
            /* Shift down knots and control points */

            for( i=r+1;    i<=m; i++ )  UQ[i-1] = UQ[i];
            for( i=fout+1; i<=n; i++ )  polesWeighted[i-1] = polesWeighted[i];

            n--;  m--; 
            
            pCurve->params.numPoles = n+1;
            for (int cpi = 0; cpi < n+1; cpi++)
                {
                pCurve->poles[cpi].x = polesWeighted[cpi].x;
                pCurve->poles[cpi].y = polesWeighted[cpi].y;
                pCurve->poles[cpi].z = polesWeighted[cpi].z;
                if (rat)
                    pCurve->weights[cpi] = polesWeighted[cpi].w;
                }
                
            /* If no more internal knots -> finished */

            if( n == p )  break;
            
            k = std::max (r-p, p+1);
            l = std::min (n, r+p-s);
            for( i=k; i<=l; i++ )
                {
                if( UQ[i] != UQ[i+1] && nr[i] != 1 ) 
                    { 
                    br[i] = MSBsplineCurve::GetRemovalKnotBound (pCurve, i, sr[i]);
                    }
                }
                
            }
        else
            {
            /* Knot is not removable */
            br[r] = fc_hugeVal;
            nr[r] = 1;
            }
        }
        
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::RemoveKnotsBounded (double tol, int startPreservation, int endPreservation)
    {
    double param[2], knot;
    int numParam = 0, status, paramType[2], iKnot, iMult, mult; 
    int degree = params.order - 1, m = params.numPoles + params.order - 1;
    bool    bOverSaturatedKnot = false;
    
    if (startPreservation)
        {
        param[numParam] = 0.0;
        paramType[numParam++] = startPreservation;
        }
    if (endPreservation)
        {
        param[numParam] = 1.0;
        paramType[numParam++] = endPreservation;
        }
    
        // flatten first knot if oversaturated
    if (knots[degree + 1] - knots[degree] < RELATIVE_BSPLINE_KNOT_TOLERANCE)
        {
        bOverSaturatedKnot = true;
        knot = knots[0];
        for (iKnot = 1; iKnot <= m && knots[iKnot] - knot < RELATIVE_BSPLINE_KNOT_TOLERANCE; iKnot++)
            knots[iKnot] = knot;
        }

    // flatten last knot if oversaturated
    if (knots[m - degree] - knots[m - degree - 1] < RELATIVE_BSPLINE_KNOT_TOLERANCE)
        {
        bOverSaturatedKnot = true;
        knot = knots[m];
        for (iKnot = m - 1; iKnot >= 0 && knot - knots[iKnot] < RELATIVE_BSPLINE_KNOT_TOLERANCE; iKnot--)
            knots[iKnot] = knot;
        }

    // flatten interior knot if oversaturated
    for (iKnot = degree + 1; iKnot < m - degree; iKnot += mult)
        {
        knot = knots[iKnot];
        for (mult = 1; iKnot + mult <= m && knots[iKnot + mult] - knot < RELATIVE_BSPLINE_KNOT_TOLERANCE; mult++);
        if (mult > degree)
            {
            for (iMult = 1; iMult < mult; iMult++)
                knots[iKnot + iMult] = knot;
            bOverSaturatedKnot = true;
            }
        }

    // remove oversaturation (knots are only removed if they are exact duplicates, requiring the flattening above)
    CleanKnots ();
    
    // remove knots
    if (startPreservation || endPreservation)
        status = removeKnotsWithConstraints (this, 2, param, paramType, tol);
    else
        status = removeKnotsNoConstraints (this, tol);
    
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::CleanKnots ()
    {
    return mdlBspline_removeRedundantKnotsFromCurve (this, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      7/92
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineCurve::ProjectToZFocalPlane (double focalLength)
    {
    int         numPoles = params.numPoles;
    double      *weightP;
    DPoint3d    *poleP, *endP;

    if (!rational)
        weights = (double*) BSIBaseGeom::Malloc (numPoles * sizeof(double));

    for (poleP = poles, weightP = weights, endP = poleP+numPoles;
            poleP<endP;
                poleP++, weightP++)
        {
        double originalWeight = rational ? *weightP : 1.0;

        *weightP = - poleP->z / focalLength;
        poleP->z *= (*weightP / originalWeight);
        }
    rational = true;
    }



END_BENTLEY_GEOMETRY_NAMESPACE