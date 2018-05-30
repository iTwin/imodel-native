/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bspline/bspcurvcurv.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "msbsplinemaster.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#ifdef compile_bspcurv_areCurvesCoincident
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
#endif


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             03/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_make2CurvesCompatible
(
MSBsplineCurve  *curve1,
MSBsplineCurve  *curve2
)
    {
    bvector<MSBsplineCurvePtr> in, out;
    in.push_back (curve1->CreateCapture ());
    in.push_back (curve2->CreateCapture());

    if (MSBsplineCurve::CloneCompatibleCurves (out, in, false, false))
        {
        curve1->SwapContents (*out[0]);
        curve2->SwapContents (*out[1]);
        return SUCCESS;
        }
    // restore originals
    curve1->SwapContents (*in[0]);
    curve2->SwapContents (*in[1]);
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             03/90
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineCurve::CloneCompatibleCurves
(
bvector<MSBsplineCurvePtr> &outputCurves,/* <= array of output curves */
bvector<MSBsplineCurvePtr> const &inputCurves,           /* => array of input curves */
bool             enableReverse,         /* => allows reversing output */
bool            openAll                /* => forces opening */
)
    {
    size_t numCurves = inputCurves.size ();
    outputCurves.clear ();
    int             done, status = ERROR,
                    maxNumKnots = 0;
    double          curveTolerance, knotTolerance, openAtParam,
                    dot, dist0, dist1;
    DPoint3d        minPt;
    DVec3d lastTan;
    bool sameNum = true;
    bool sameKnots = true;
    bool sameMult = true;

    if (numCurves < 2)
        {
        outputCurves.push_back (inputCurves[0]->CreateCopy ());
        return true;
        }
    if (numCurves > MAX_POLES)
        return false;
    
    bvector<int> numDistinct;
    bvector<KnotData> knotData;
    bvector<size_t> maxMult;
    knotTolerance = fc_hugeVal;

    /* We need to find the curve with the highest degree, because the others
        will have to be raised to that degree. Also check periodicity and
        rationality of all the curves. If all the curves are Bezier then we
        will not have to do the refining part, just up the degree */
    int highestOrder = inputCurves[0]->params.order;
    bool allClosed    = inputCurves[0]->params.closed != 0;
    bool rational     = inputCurves[0]->rational != 0;
    bool bezier       = inputCurves[0]->params.numPoles == inputCurves[0]->params.order;

    for (auto &curve : inputCurves)
        {
        auto cvP = curve.get ();
        highestOrder = cvP->params.order > highestOrder ? cvP->params.order : highestOrder;
        allClosed = allClosed && cvP->params.closed;
        rational  = rational || cvP->rational;
        bezier    = bezier && (cvP->params.numPoles == cvP->params.order);
        }
    int highestDegree = highestOrder - 1;

    for (size_t i=0; i < numCurves; i++)
        {
        /* allocate space for the output curves and copy the original to the
            output */
        outputCurves.push_back (inputCurves[i]->CreateCopy ());
        auto cvP = outputCurves[i].get ();
        if (SUCCESS != (status = bspknot_normalizeKnotVector (cvP->knots, cvP->params.numPoles, cvP->params.order, cvP->params.closed)))
            goto wrapup;

        /* Open all curves */
        if (cvP->params.closed)
            {
            if (i==0)
                {
                if (SUCCESS != (status = cvP->MakeOpen (0.0)))
                    goto wrapup;
                }
            else
                {
                DPoint3d startPointOnPreviousCurve;
                outputCurves[i-1]->FractionToPoint (startPointOnPreviousCurve, 0.0);
                cvP->ClosestPoint (minPt, openAtParam, startPointOnPreviousCurve);
                if (SUCCESS != (status = cvP->MakeOpen (openAtParam)))
                    goto wrapup;
                }
            }

        /* Reverse curve if necessary */
        if (enableReverse)
            {
            DVec3d tan = DVec3d::FromStartEndNormalize (cvP->poles[0], cvP->poles[1]);
            if (i)
                {
                auto cvP0 = outputCurves[i-1].get ();
                dot = tan.DotProduct (lastTan);
                dist0 = cvP0->poles[0].Distance (cvP->poles[0]);
                dist1 = cvP0->poles[0].Distance (cvP->poles[cvP->params.numPoles - 1]);
                if ((dot < 0.0) || (dist1 < dist0))
                    {
                    bspcurv_reverseCurve (cvP, cvP);
                    // and get the revised tangent
                    tan = DVec3d::FromStartEndNormalize (cvP->poles[0], cvP->poles[1]);
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
            int sameKnots = cvP->params.numPoles + cvP->params.order; /*OPEN*/

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
    maxMult.clear ();
    for (size_t i = 0; i < maxNumKnots; i++)
        maxMult.push_back (0);
    for (auto &curve : outputCurves)
        {
        auto cvP = curve.get ();
        knotData.push_back (KnotData ());
        knotData.back ().LoadCurveKnots (*cvP);
        for (size_t j=0; j < knotData.back ().GetNumActiveKnots (); j++)
            {
            auto m = knotData.back ().multiplicities[j];
            maxMult[j] = std::max (maxMult[j], m);
            }
        }

    /* find out if they all have the same number of distinct knots and
       if they are the same distinct knots */
    for (size_t i=1; i < numCurves; i++)
        {
        if (!sameNum || knotData[i-1].GetNumActiveKnots () != knotData[i].GetNumActiveKnots ())
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
                for (size_t j=1; j + 1 < knotData[i].GetNumActiveKnots (); j++)
                    {
                    sameKnots =
                        fabs (knotData[i-1].compressedKnots[j] - knotData[i].compressedKnots[j]) <= knotTolerance;
                    sameMult =
                        knotData[i-1].multiplicities[j] == knotData[i].multiplicities[j];
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
            for (size_t i=0; i < numCurves; i++)
                {
                auto cvP = outputCurves[i].get ();
                for (size_t j=1; j < knotData[i].GetNumActiveKnots() - 1; j++)
                    {
                    bspknot_addKnot (cvP, knotData[i].compressedKnots[j], knotTolerance,
                            (int)maxMult[j],
                            false);
                    }
                }
            }

        size_t index = (size_t)highestOrder; /* assumes all curves are open */
        for (size_t i=1; i < knotData[0].GetNumActiveKnots () - 1; i++)
            {
            /* find the average of each distinct knot between curves */
            double avgKnot = 0.0;
            for (size_t j=0; j < numCurves; j++)
                 avgKnot += knotData[j].compressedKnots[i];
            avgKnot /= numCurves;
            size_t newMult = 0;
            for (size_t j=0; j < numCurves; j++)
                {
                auto cvP = outputCurves[j].get ();
                newMult = (sameKnots && sameMult) ? knotData[j].multiplicities[i] : maxMult[i];
                for (size_t k=0; k < newMult; k++)
                    cvP->knots[index + k] = avgKnot;
                }
            index += (int)newMult;  // newMult was the same on all curves ??
            }
        }

    else /* general case */
        {
        done = false;
        size_t index = (size_t)highestOrder;
        bvector<double> knots;
        bvector<int> tags ((int)numCurves);
        do
            {
            size_t sum = 0;
            knots.clear ();
            for (size_t i=0; i < numCurves; i++)
                {
                auto cvP = outputCurves[i].get ();
                if (cvP->params.numPoles == index)
                    {
                    sum += 1;
                    knots.push_back (cvP->knots[cvP->params.numPoles]);
                    }
                else
                    knots.push_back (cvP->knots[index]);
                }
            if (sum == numCurves) /* been through all knots of all curves */
                done = true;
            else
                {
                /* sort the knots from smallest to largest */
                util_tagSort (tags.data (), knots.data (), (int)numCurves);

                /* find out how many knots have the smallest value,
                    then break and average this knot  */
                double avgKnot = knots[tags[0]];
                size_t i;   // at end, this is the index where knots were different.
                for(i=0; i < numCurves-1; i++)
                    {
                    if (fabs (knots[tags[i]]-knots[tags[i+1]]) < knotTolerance)
                        avgKnot += knots[tags[i+1]];
                    else
                        break;
                    }
                size_t j = i ;
                avgKnot /= (j+1);

                /* find largest multiplicity of this smallest averaged knot */
                size_t newMult = 1;
                for (ptrdiff_t i=j; i >= 0; i-- )
                    {
                    size_t curveMult = 1;
                    for (size_t k=0; k < highestDegree; k++)
                        {
                        if (fabs(outputCurves[tags[i]]->knots[index+k] -
                                 outputCurves[tags[i]]->knots[index+k+1]) < knotTolerance )
                            curveMult += 1;
                        else
                            break;
                        }
                    newMult = (curveMult > newMult) ? curveMult : newMult;

                    /* also set the knot in the knot vector to this avgKnot */
                    for (size_t k=0; k < curveMult; k++)
                        outputCurves[tags[i]]->knots[index+k] = avgKnot;
                    }

                /* add the knot to the curves to full multiplicity  */
                for (auto &curve : outputCurves)
                    bspknot_addKnot (curve.get (), avgKnot,knotTolerance, (int)newMult, false);
                index += newMult;
                }
            }
        while (!done);
        }

    /* close all curves if allClosed == true */
    if (allClosed && !openAll)
        for (size_t i=0; i< numCurves; i++)
            {
            auto cvP = outputCurves[i].get ();

            // generate a "fake closed" curve so that numPoles doesn't change
            // EDL May 6 2018: This called mdlBspline_closeCurve_V7.
            // Is there a difference?  Would any V7 specific curve materialize here?
            // per comments on mdlBspline_closeCurve, there is special behavior for 
            // linear curve knots
            if (SUCCESS != (status = bspcurv_closeCurve (cvP, cvP)))
                goto wrapup;
            }

wrapup:
    if (status != SUCCESS)
        outputCurves.clear ();
    return status == SUCCESS;
    }

END_BENTLEY_GEOMETRY_NAMESPACE