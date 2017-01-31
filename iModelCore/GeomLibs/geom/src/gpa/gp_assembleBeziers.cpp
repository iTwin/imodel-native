/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/gpa/gp_assembleBeziers.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/GeomPrivateApi.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Peter.Yu                        12/2007
+---------------+---------------+---------------+---------------+---------------+------*/
static int     removeMultiKnot
(
MSBsplineCurve* pOut,
MSBsplineCurve* pIn,
int             num,
int             r,
int             s,
double          tol
)
    {
    int         n, p;
    double      u, alfi, alfj;
    int         m, ord, fout, last, first, t, off, i, j, ii, jj, k;
    //bool        remflag;

    n = pIn->params.numPoles - 1;
    p = pIn->params.order - 1;
    DPoint4d pt;

    DPoint4d* temp = (DPoint4d*)malloc ((2*p+1) * sizeof(DPoint4d));
    DPoint4d* ctrlpt = (DPoint4d*)malloc ((n+1) * sizeof(DPoint4d));

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
        //remflag = false;

        while( j-i > t )
            {
            alfi = (u-pIn->knots[i])/(pIn->knots[i+ord+t]-pIn->knots[i]);
            alfj = (u-pIn->knots[j-t])/(pIn->knots[j+ord]-pIn->knots[j-t]);
            bsiDPoint4d_addScaledDPoint4d (&pt, ctrlpt+i, temp+ii-1, -(1.0-alfi));
            bsiDPoint4d_scale (temp+ii, &pt, 1.0/alfi);
            bsiDPoint4d_addScaledDPoint4d (&pt, ctrlpt+j, temp+jj+1, -alfj);
            bsiDPoint4d_scale (temp+jj, &pt, 1.0/(1.0-alfj));

            i = i+1;
            j = j-1;
            ii = ii+1;
            jj = jj-1;
            }
        /* We can ommit these judgements since the knots can always be removed in this case */
        /*
        if(j-i < t)
            {
            if( bsiDPoint4d_realDistance(temp+ii-1, temp+jj+1) < tol )
                    remflag = true;
            }
        else
            {
            alfi = (u-pIn->knots[i])/(pIn->knots[i+ord+t]-pIn->knots[i]);
            bsiDPoint4d_interpolate (&pt, temp+ii+t+1, 1.0-alfi, temp+ii-1);
            if( bsiDPoint4d_realDistance( ctrlpt+i, &pt ) < tol )
                    remflag = true;
            }

        if(!remflag)
            break;
        */
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
        free (ctrlpt);
        free (temp);
        return 0;
        }

    int numk = bspknot_numberKnots (pOut->params.numPoles, pOut->params.order, pOut->params.closed) - t;

    double* knots = (double*)malloc (numk * sizeof(double));
    DPoint3d* poles = (DPoint3d*)malloc ((n+1-t) * sizeof(DPoint3d));
    double *weights = (double*)malloc ((n+1-t) * sizeof(double));

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
        free (pOut->weights);
    free (pOut->knots);
    free (pOut->poles);

    pOut->display.curveDisplay = true;
    pOut->display.polygonDisplay = false;
    pOut->poles = poles;
    pOut->knots = knots;

    if (pOut->rational)
        pOut->weights = weights;
    else
        free (weights);

    free (ctrlpt);
    free (temp);
    return t;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2007
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus   bCurveFromDPoint4d (MSBsplineCurveP curve, DPoint4dCP pPoleArray, int numPole)
    {
    memset (curve, 0, sizeof (MSBsplineCurve));

    curve->rational = !bsiBezierDPoint4d_isUnitWeight (pPoleArray, numPole, -1.0);
    curve->display.curveDisplay = true;
    curve->params.order = numPole;
    curve->params.numPoles = numPole;
    curve->params.numKnots = 0;

    if (SUCCESS != bspcurv_allocateCurve (curve))
        return ERROR;

    bspknot_computeKnotVector (curve->knots, &curve->params, NULL);

    for (int i = 0; i < numPole; i++)
        {
        curve->poles[i].x = pPoleArray[i].x;
        curve->poles[i].y = pPoleArray[i].y;
        curve->poles[i].z = pPoleArray[i].z;

        if (curve->rational)
            curve->weights[i] = pPoleArray[i].w;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Peter.Yu                        01/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus   bCurveFromDPoint4dArray (MSBsplineCurveP curve, DPoint4dCP pPoleArray, int numPole, int order)
    {
    memset (curve, 0, sizeof (MSBsplineCurve));

    curve->rational = !bsiBezierDPoint4d_isUnitWeight (pPoleArray, numPole, -1.0);
    curve->display.curveDisplay = true;
    curve->params.order = order;
    curve->params.numPoles = numPole;
    curve->params.numKnots = 0;

    if (SUCCESS != bspcurv_allocateCurve (curve))
        return ERROR;

    bspknot_computeKnotVector (curve->knots, &curve->params, NULL);

    for (int i = 0; i < numPole; i++)
        {
        curve->poles[i].x = pPoleArray[i].x;
        curve->poles[i].y = pPoleArray[i].y;
        curve->poles[i].z = pPoleArray[i].z;

        if (curve->rational)
            curve->weights[i] = pPoleArray[i].w;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Peter.Yu                        01/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus   getBCurveWithoutUserData (MSBsplineCurveP curve, int *index, GraphicsPointArrayCP pInstance)
    {
    DPoint4d        poleArray[MAX_BEZIER_CURVE_ORDER];
    int             numPole, k = 1, count = jmdlGraphicsPointArray_getCount (pInstance);
    BentleyStatus   status = SUCCESS;

    bvector<MSBsplineCurve> beziers;
    MSBsplineCurve tmpCurve;
    tmpCurve.Zero ();

    tmpCurve.CopyFrom (*curve);
    beziers.push_back (tmpCurve);

    double          paramLength = 0.0;

    while (0 == jmdlGraphicsPointArray_getPointMask (pInstance, *index - 1, HPOINT_MASK_MAJOR_BREAK | HPOINT_MASK_BREAK) &&
           jmdlGraphicsPointArray_getBezier (pInstance, index, poleArray, &numPole, MAX_BEZIER_CURVE_ORDER) &&
           SUCCESS == bCurveFromDPoint4d (&tmpCurve, poleArray, numPole))
        {
        beziers.push_back (tmpCurve);
        }

    if (k > 1)
        status = (BentleyStatus) MSBspline::InitFromBeziers (*curve, beziers);

    for (int i=0; i<k; i++)
        beziers[i].ReleaseMem ();
    beziers.clear ();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Peter.Yu                        10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus jmdlGraphicsPointArray_assembleBeziersIntoBspline
(
GraphicsPointArrayCP    pGPA,
int                     inputReadIndex,
MSBsplineCurveR         curve,
int                     &nextReadIndex
)
    {
    DPoint4d        poleArray[MAX_BEZIER_CURVE_ORDER];
    int             i, j, k = 1, size = 0, numPole, old = inputReadIndex, mark;
    BentleyStatus   status = ERROR;
    bool            isNormal = true;

    if (!jmdlGraphicsPointArray_getBezier (pGPA, &inputReadIndex, poleArray, &numPole, MAX_BEZIER_CURVE_ORDER))
        return status;

    nextReadIndex = inputReadIndex;
    int numBezier = 1;

    GraphicsPoint const*    pGP = jmdlGraphicsPointArray_getConstPtr (pGPA, nextReadIndex);

    status = bCurveFromDPoint4d (&curve, poleArray, numPole);

    if (!pGP || SUCCESS != status)
        return status;

    mark = nextReadIndex;
    pGP = jmdlGraphicsPointArray_getConstPtr (pGPA, mark - 1);

    if (pGP->userData < 1 || pGP->a < 1.0E-8)
        {

        status = getBCurveWithoutUserData (&curve, &nextReadIndex, pGPA);
        return status;
        }

    int         order = numPole, count = jmdlGraphicsPointArray_getCount (pGPA);
    int         maxBezier = count/numPole;
    int         *remMultP = (int*)malloc (maxBezier * sizeof(int));
    double      *remKnotP = (double*)malloc (maxBezier * sizeof(double));
    DPoint4d    *polesP = (DPoint4d*)malloc (count * sizeof(DPoint4d));
    remKnotP[0] = pGP->a;
    remMultP[0] = numPole - 1 - pGP->userData;
    for (i=0; i<numPole; i++)
        polesP[size++] = poleArray[i];

    // Concatenate any adjacent bezier segments until next break.
    // Maybe we need not use function bspcurv_appendCurves if the contained Beziers have the same order.
    double paramLength = 0.0;
    while (0 == jmdlGraphicsPointArray_getPointMask (pGPA, nextReadIndex - 1, HPOINT_MASK_MAJOR_BREAK | HPOINT_MASK_BREAK) &&
           jmdlGraphicsPointArray_getBezier (pGPA, &nextReadIndex, poleArray, &numPole, MAX_BEZIER_CURVE_ORDER))
        {
        numBezier++;
        paramLength += 1.0;
        if (nextReadIndex <= count)
            {
            pGP = jmdlGraphicsPointArray_getConstPtr (pGPA, nextReadIndex - 1);
            remKnotP[k] = pGP->a;
            if (!(remKnotP[k] > remKnotP[k-1]))
                {
                isNormal = false;
                break;
                }
            remMultP[k] = numPole - 1 - pGP->userData;
            k++;
            }
        for (i=1; i<numPole; i++)
            polesP[size++] = poleArray[i];
        }

    if (!isNormal)
        {
        free (polesP);
        free (remMultP);
        free (remKnotP);
        status = getBCurveWithoutUserData (&curve, &mark, pGPA);
        nextReadIndex = mark;
        return status;
        }

    curve.ReleaseMem();                          // At this point the curve contains the first segment. Free it to avoid a memory leak.
    bCurveFromDPoint4dArray (&curve, polesP, size, order);

    int p = curve.params.order - 1;
    int numKnots = bspknot_numberKnots (curve.params.numPoles, p + 1, curve.params.closed);
    curve.params.numKnots = numKnots - 2*curve.params.order;

    pGP = jmdlGraphicsPointArray_getConstPtr (pGPA, old);
    for (i=0; i<=p; i++)
        curve.knots[i] = pGP->a;
    for (i=p+1; i<numKnots-p-1; i+=p)
        {
        for (j=0; j<p; j++)
            curve.knots[i+j] = remKnotP[(i-1)/p-1];
        }
    for (i=numKnots-1; i>=numKnots-p-1; i--)
        curve.knots[i] = pGP[numBezier * order - 1].a;

    // We can use two methods to remove redundant knots
    // 1. mdlBspline_removeSpecificKnotsFromCurve, which wrapped a Nlib function N_toocrm
    // 2. Rewrite N_toocrm which omitted computing tolerance because we know the knots can be removed safely.
    //First method:
    //if (paramLength > 0.0)
    //    mdlBspline_removeSpecificKnotsFromCurve (curve, curve, remKnotP, remMultP, k, 1.0e+10);
    //Second method:
    int r = 2*p, t;
    if (paramLength > 0.0)
        {
        for (i=0; i<k-1; i++)
            {
            t = removeMultiKnot (&curve, &curve, remMultP[i], r, p, 1.0e+10);
            r += p - t;
            }
        }

    free (polesP);
    free (remMultP);
    free (remKnotP);

    return SUCCESS;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
