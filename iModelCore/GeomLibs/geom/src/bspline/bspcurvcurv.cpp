/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bspline/bspcurvcurv.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "msbsplinemaster.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/93
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bspcurv_areCurvesCoincident
(
MSBsplineCurve      *segment,
MSBsplineCurve      *testCurve,
double              tolerance,
RotMatrix           *rotMatrixP
)
    {
    int             i;
    double          testParam, minDist;
    DPoint3d        point;

    /* For now, check if four points on segment are "on" testCurve
       this could certainly use some improvement.  */
    for (i=0, testParam=0.0; i<4; i++, testParam+=.25)
        {
        bspcurv_evaluateCurvePoint (&point, NULL, segment, testParam);
        bsprcurv_minDistToCurve (&minDist, NULL, NULL, &point, testCurve,
                                 &tolerance, rotMatrixP);

        if (minDist > tolerance)        break;
        }
    return i==4;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             03/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_makeCurvesCompatible
(
MSBsplineCurve  **outputCurves,
MSBsplineCurve  **inputCurves,
int             numCurves,
int             enableReverse,         /* => allows reversing output */
int             openAll                /* => forces opening */
)
    {
    int             i, j, k, highestOrder, highestDegree, bezier,
                    rational, sameNum, sameKnots, sameMult, newMult = 0,
                    curveMult, index, done, sum, status = ERROR, allocSize,
                    *numDistinct = NULL, **knotMultiplicity = NULL,
                    *tags = NULL, allClosed, *maxMult, maxNumKnots = 0;
    double          curveTolerance, knotTolerance, avgKnot, openAtParam, minDist,
                    **distinctKnots = NULL, *knots = NULL, dot, dist0, dist1;
    DPoint3d        minPt, tan, lastTan;
    MSBsplineCurve  *cvP;

    if (numCurves < 2)
        return SUCCESS;
    if (numCurves > MAX_POLES)
        return (ERROR);
    
    numDistinct      = (int*)BSIBaseGeom::Malloc (numCurves * sizeof(int));
    knotMultiplicity = (int**)BSIBaseGeom::Malloc (numCurves * sizeof(int*));
    tags             = (int*)BSIBaseGeom::Malloc (numCurves * sizeof(int));
    distinctKnots    = (double**)BSIBaseGeom::Malloc (numCurves * sizeof(double*));
    knots            = (double*)BSIBaseGeom::Malloc (numCurves * sizeof(double));

    memset (distinctKnots, 0, numCurves *sizeof(distinctKnots) );
    memset (knotMultiplicity, 0, numCurves *sizeof(knotMultiplicity) );
    maxMult = NULL;
    knotTolerance = fc_hugeVal;

    /* We need to find the curve with the highest degree, because the others
        will have to be raised to that degree. Also check periodicity and
        rationality of all the curves. If all the curves are Bezier then we
        will not have to do the refining part, just up the degree */
    highestOrder = inputCurves[0]->params.order;
    allClosed    = inputCurves[0]->params.closed;
    rational     = inputCurves[0]->rational;
    bezier       = inputCurves[0]->params.numPoles == inputCurves[0]->params.order;

    for (i=1; i < numCurves; i++)
        {
        cvP = inputCurves[i];
        highestOrder = cvP->params.order > highestOrder ? cvP->params.order : highestOrder;
        allClosed = allClosed && cvP->params.closed;
        rational  = rational || cvP->rational;
        bezier    = bezier && (cvP->params.numPoles == cvP->params.order);
        }
    highestDegree = highestOrder - 1;

    for (i=0; i < numCurves; i++)
        {
        cvP = outputCurves[i];

        /* allocate space for the output curves and copy the original to the
            output */
        if (SUCCESS != (status = bspcurv_copyCurve (cvP, inputCurves[i])) ||
            SUCCESS != (status = bspknot_normalizeKnotVector (cvP->knots, cvP->params.numPoles, cvP->params.order, cvP->params.closed)))
            goto wrapup;

        /* Open all curves */
        if (cvP->params.closed)
            {
            if (i==0)
                {
                if (SUCCESS != (status = bspcurv_openCurve (cvP, cvP, 0.0)))
                    goto wrapup;
                }
            else
                {
                bsprcurv_minDistToCurve (&minDist, &minPt, &openAtParam, outputCurves[i-1]->poles, cvP, NULL, NULL);

                if (SUCCESS != (status = bspcurv_openCurve (cvP, cvP, openAtParam)))
                    goto wrapup;
                }
            }

        /* Reverse curve if necessary */
        if (enableReverse)
            {
            bsiDPoint3d_computeNormal (&tan, cvP->poles, cvP->poles+1);
            if (i)
                {
                bsiDPoint3d_computeNormal (&tan, cvP->poles, cvP->poles+1);
                dot = bsiDPoint3d_dotProduct (&tan, &lastTan);
                dist0 = bsiDPoint3d_distance ((cvP-1)->poles, cvP->poles);
                dist1 = bsiDPoint3d_distance ((cvP-1)->poles, cvP->poles +
                                        cvP->params.numPoles - 1);
                if ((dot < 0.0) || (dist1 < dist0))
                    {
                    bspcurv_reverseCurve (cvP, cvP);
                    bsiDPoint3d_computeNormal (&tan, cvP->poles, cvP->poles+1);
                    }
                }
            lastTan = tan;
            }

        /* Raise degree if necessary */
        if (cvP->params.order < highestOrder)
            {
            if (SUCCESS != (status = bspcurv_elevateDegree (cvP, cvP, highestDegree)))
                goto wrapup;
            }

        /* Make rational if necessary */
        if (rational && ! cvP->rational)
            {
            if (SUCCESS != (status = bspcurv_makeRational (cvP, cvP)))
                goto wrapup;
            }

        if (!bezier)
            {
            curveTolerance = bspknot_knotTolerance (cvP);
            sameKnots = cvP->params.numPoles + cvP->params.order; /*OPEN*/

            /* we want to find the smallest knot tolerance of all curves */
            if ( i == 0  ||  curveTolerance < knotTolerance)
                knotTolerance = curveTolerance;

            /* and the maximum number of knots in any curve */
            if ( i == 0  ||  maxNumKnots < sameKnots)
                maxNumKnots = sameKnots;
            }
        }

    if (bezier)
        {
        status = SUCCESS;
        goto wrapup;
        }

    /* refine all the knot vectors, so curve have the same number of knots */

    /* find the distinct knots of all the OPEN(!!) curves */

    allocSize = maxNumKnots * sizeof(int);
    if (NULL == (maxMult = static_cast<int *>(BSIBaseGeom::Malloc (allocSize))))
        {
        status = ERROR;
        goto wrapup;
        }
    memset (maxMult, 0, allocSize);
    for (i=0; i < numCurves; i++)
        {
        cvP = outputCurves[i];
        allocSize = cvP->params.numPoles + cvP->params.order;
        if (NULL ==
            (distinctKnots[i]    = static_cast<double *>(BSIBaseGeom::Malloc (allocSize * sizeof(double)))) ||
            NULL ==
            (knotMultiplicity[i] = static_cast<int *>(BSIBaseGeom::Malloc (allocSize * sizeof(int)))))
            {
            status = ERROR;
            goto wrapup;
            }
        bspknot_getKnotMultiplicity (distinctKnots[i], knotMultiplicity[i], numDistinct + i,
                                     cvP->knots, cvP->params.numPoles, cvP->params.order,
                                     cvP->params.closed, knotTolerance);
        for (j=0; j < numDistinct[i]; j++)
            maxMult[j] = maxMult[j] > knotMultiplicity[i][j] ?
                         maxMult[j] : knotMultiplicity[i][j];
        }

    /* find out if they all have the same number of distinct knots and
       if they are the same distinct knots */
    sameNum = true;
    sameKnots = true;
    sameMult = true;
    for (i=1; i < numCurves; i++)
        {
        if (!sameNum || numDistinct[i-1] != numDistinct[i])
            {
            sameNum = false;
            break;
            }
        else
            {
            if (sameKnots && sameMult)
                {
                /* only check if all previous have same distinct knots
                   NOTE: this loop assumes all curves are open !!! */
                for (j=1; j < numDistinct[0]-1; j++)
                    {
                    sameKnots =
                        fabs (distinctKnots[i-1][j] - distinctKnots[i][j]) <= knotTolerance;
                    sameMult =
                        knotMultiplicity[i-1][j] == knotMultiplicity[i][j];
                    if (!sameKnots || !sameMult)
                        break;
                    }
                }
            }
        }

    if (sameNum && sameKnots && sameMult)
        {
        if (!sameKnots || !sameMult)
            {
            /* make all interior knots of maximum multiplicity */
            for (i=0; i < numCurves; i++)
                {
                cvP = outputCurves[i];
                for (j=1; j < numDistinct[i]-1; j++)
                    {
                    bspknot_addKnot (cvP, distinctKnots[i][j], knotTolerance, maxMult[j],
                                     false);
                    }
                }
            }

        index = highestOrder; /* assumes all curves are open */
        for (i=1; i < numDistinct[0] - 1; i++)
            {
            /* find the average of each distinct knot between curves */
            for (j=0, avgKnot=0.0; j < numCurves; j++)
                 avgKnot += distinctKnots[j][i];
            avgKnot /= numCurves;

            for (j=0; j < numCurves; j++)
                {
                cvP = outputCurves[j];
                newMult = (sameKnots && sameMult) ? knotMultiplicity[j][i] : maxMult[i];
                for (k=0; k < newMult; k++)
                    cvP->knots[index + k] = avgKnot;
                }
            index += newMult;
            }
        }

    else /* general case */
        {
        done = false;
        index = highestOrder;
        do
            {
            sum = 0;
            for (i=0; i < numCurves; i++)
                {
                cvP = outputCurves[i];
                if (cvP->params.numPoles == index)
                    {
                    sum += 1;
                    knots[i] = cvP->knots[cvP->params.numPoles];
                    }
                else
                    knots[i] = cvP->knots[index];
                }
            if (sum == numCurves) /* been through all knots of all curves */
                done = true;
            else
                {
                /* sort the knots from smallest to largest */
                util_tagSort (tags, knots, numCurves);

                /* find out how many knots have the smallest value,
                    then break and average this knot  */
                avgKnot = knots[tags[0]];
                for(i=0; i < numCurves-1; i++)
                    {
                    if (fabs (knots[tags[i]]-knots[tags[i+1]]) < knotTolerance)
                        avgKnot += knots[tags[i+1]];
                    else
                        break;
                    }
                j = i ;
                avgKnot /= (j+1);

                /* find largest multiplicity of this smallest averaged knot */
                newMult = 1;
                for (i=j; i >= 0; i-- )
                    {
                    curveMult = 1;
                    for (k=0; k < highestDegree; k++)
                        {
                        if (fabs(outputCurves[tags[i]]->knots[index+k] -
                                 outputCurves[tags[i]]->knots[index+k+1]) < knotTolerance )
                            curveMult += 1;
                        else
                            break;
                        }
                    newMult = (curveMult > newMult) ? curveMult : newMult;

                    /* also set the knot in the knot vector to this avgKnot */
                    for (k=0; k < curveMult; k++)
                        outputCurves[tags[i]]->knots[index+k] = avgKnot;
                    }

                /* add the knot to the curves to full multiplicity  */
                for (i=0; i < numCurves; i++)
                    {
                    cvP = outputCurves[i];
                    bspknot_addKnot (cvP, avgKnot,knotTolerance, newMult, false);
                    }
                index += newMult;
                }
            }
        while (!done);
        }

    /* close all curves if allClosed == true */
    if (allClosed && !openAll)
        for (i=0; i< numCurves; i++)
            {
            cvP = outputCurves[i];

            // generate a "fake closed" curve so that numPoles doesn't change
            if (SUCCESS != (status = mdlBspline_closeCurve_V7 (cvP, cvP, false)))
                goto wrapup;
            }

wrapup:
    if (maxMult)        BSIBaseGeom::Free  (maxMult);
    for (i=0; i < numCurves; i++)
        {
        if (distinctKnots[i])       BSIBaseGeom::Free (distinctKnots[i]);
        if (knotMultiplicity[i])    BSIBaseGeom::Free (knotMultiplicity[i]);
        }
    
    BSIBaseGeom::Free (knotMultiplicity);
    BSIBaseGeom::Free (distinctKnots);
    BSIBaseGeom::Free (numDistinct);
    BSIBaseGeom::Free (tags);
    BSIBaseGeom::Free (knots);

    if (status != SUCCESS)
        {
        for (i=0; i < numCurves; i++)
            {
            cvP = outputCurves[i];
            if (cvP)    bspcurv_freeCurve (cvP);
            }
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             03/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_make2CurvesCompatible
(
MSBsplineCurve  *curve1,
MSBsplineCurve  *curve2
)
    {
    int             status;
    MSBsplineCurve  compat[2], *compatPtr[2], *curvePtr[2];

    memset (compat, 0, sizeof(compat));

    curvePtr[0] = curve1;
    curvePtr[1] = curve2;
    compatPtr[0] = &compat[0];
    compatPtr[1] = &compat[1];

    if (SUCCESS !=
        (status = bspcurv_makeCurvesCompatible (compatPtr, curvePtr, 2, false, false)))
        return status;

    bspcurv_freeCurve (curve1);
    bspcurv_freeCurve (curve2);

    *curve1 = compat[0];
    *curve2 = compat[1];

    return SUCCESS;
    }


END_BENTLEY_GEOMETRY_NAMESPACE