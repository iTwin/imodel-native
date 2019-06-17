/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/GeomPrivateApi.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*-----------------------------------------------------------------*//**
* @class SmallSetRange1d
* The SmallSetRange1d structure is used for small collections of
* angular ranges, such as occuring when an elliptical arc (originally one
* angular range) is intersected with a clip volume.  This separates the
* ellipse into several fragments.   Since clip volumes have limited number of
* planes, the clipped ellipse also has a limited number of fragments.
*
* Arc ranges are entered as a start and sweep angle. If the sweep
* is greater than 2pi the interval is entered as -pi to +pi.  Otherwise
* its endpoints are shifted to be in the interval -pi..+pi, and the
* interval may be split into two parts if needed.
* @author   EarlinLutz
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+===============+===============+===============+===============+======*/

/* MAP define bsiRange1d_clear(rangeSetP) rangeSetP->clear() ENDMAP */

/*-----------------------------------------------------------------*//**
*
* Clear the range set to an empty set. (No intervals)
*
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiRange1d_clear
(
SmallSetRange1dP rangeSetP
)

    {
    rangeSetP->n = 0;
    }


/* MAP define jmdlRange1d_addArcSweep(arcSetP,theta0,dtheta) arcSetP->addArcSweep(theta0,dtheta) ENDMAP */

/*-----------------------------------------------------------------*//**
*
* Adds an angular range to a SmallSetrange1d.  The angular range is
* normalized to the +=pi range, and is split into two parts if needed.
*
* @param theta0 => start of interval.
* @param dtheta => angular sweep.  may be negative.
* @see #setArcSweep
* @see #setUncheckedArcSweep
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRange1d_addArcSweep

(
SmallSetRange1dP arcSetP,
double          theta0,
double          dtheta
)

    {
    double theta1;
    int i = arcSetP->n;
    static double s_piTol = 1.0e-12;
    if ( i >= MSGEOM_SMALL_SET_SIZE )
        return false;

    if (dtheta < 0.0)
        {
        theta0 = theta0 + dtheta;
        dtheta = -dtheta;
        }

    if (dtheta >= msGeomConst_2pi)
        {
        arcSetP->interval[i].minValue = -msGeomConst_pi;
        arcSetP->interval[i].maxValue =  msGeomConst_pi;
        arcSetP->n += 1;
        return true;
        }

    while (theta0 < -msGeomConst_pi)
        theta0 += msGeomConst_2pi;
    while (theta0 > msGeomConst_pi)
        theta0 -= msGeomConst_2pi;

    if (fabs (theta0 - msGeomConst_pi) < s_piTol)
        {
        theta0 = -msGeomConst_pi;
        }

    theta1 = theta0 + dtheta;

    if (theta1 <= msGeomConst_pi)
        {
        arcSetP->interval[i].minValue = theta0;
        arcSetP->interval[i].maxValue = theta1;
        arcSetP->n += 1;
        }
    else
        {
        arcSetP->interval[i].minValue = theta0;
        arcSetP->interval[i].maxValue = msGeomConst_pi;
        i = (arcSetP->n += 1);
        if (i >= MSGEOM_SMALL_SET_SIZE)
            {
            return false;
            }
        else
            {
            arcSetP->interval[i].minValue = -msGeomConst_pi;
            arcSetP->interval[i].maxValue = theta1 - msGeomConst_2pi;
            arcSetP->n += 1;
            }
        }
    return true;
    }

/* MAP define bsiRange1d_isFullCircle(pRangeSet) pRangeSet->isFullCircle() ENDMAP */

/*-----------------------------------------------------------------*//**
* Test if a the range set is a full 360 range in angle space.

* @instance pRangeSet => range set to test
* @see
* @return true if the range set is a singel interval that is 2pi (within tolerance).
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRange1d_isFullCircle
(
SmallSetRange1dCP pRangeSet
)

    {
    static double piMultiplier = 1.99999;
    return pRangeSet->n == 1
        && (pRangeSet->interval[0].maxValue - pRangeSet->interval[0].minValue) > piMultiplier * msGeomConst_pi;
    }

/* MAP define bsiRange1d_setArcSweep(arcSetP,theta0,dtheta) arcSetP->setArcSweep(theta0,dtheta) ENDMAP */

/*-----------------------------------------------------------------*//**
*
* Set an arc sweep as the only entry in a small range set.
* All prior entries in the set are ignored.  The arc sweep is normalized
* and split as needed.
*
* @param theta0 => start of interval.
* @param dtheta => angular sweep.  May be negative.
* @see #addArcSweep
* @see #setUncheckedArcSweep
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiRange1d_setArcSweep

(
SmallSetRange1dP arcSetP,
double          theta0,
double          dtheta
)


    {
    double theta1;

    if (dtheta < 0.0)
        {
        theta0 = theta0 + dtheta;
        dtheta = -dtheta;
        }

    if (dtheta >= msGeomConst_2pi)
        {
        arcSetP->interval[0].minValue = -msGeomConst_pi;
        arcSetP->interval[0].maxValue =  msGeomConst_pi;
        arcSetP->n = 1;
        }
    else
        {
        theta1 = theta0 + dtheta;

        if (theta1 <= msGeomConst_pi)
            {
            arcSetP->interval[0].minValue = theta0;
            arcSetP->interval[0].maxValue = theta1;
            arcSetP->n = 1;
            }
        else
            {
            theta1 -= msGeomConst_2pi;
            arcSetP->interval[0].minValue = -msGeomConst_pi;
            arcSetP->interval[0].maxValue =  theta1;
            arcSetP->interval[1].minValue = theta0;
            arcSetP->interval[1].maxValue = msGeomConst_pi;
            arcSetP->n = 2;
            }
        }
    }

/* MAP define bsiRange1d_setUncheckedArcSweep(arcSetP,theta0,dtheta) arcSetP->setUncheckedArcSweep(theta0,dtheta) ENDMAP */

/*-----------------------------------------------------------------*//**
*
* Set an arc sweep as the only entry in a small range set.
* All prior entries in the set are ignored.  The sweep is inserted
* as is, with no attempt to normalize direction or range.
* Caveat emptor.
*
* @param theta0 => Start of interval.
* @param dtheta => Angular sweep.  May be negative.
* @see #addArcSweep
* @see #setArcSweep
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiRange1d_setUncheckedArcSweep

(
SmallSetRange1dP arcSetP,
double          theta0,
double          dtheta
)


    {
    double theta1 = theta0 + dtheta;

    arcSetP->interval[0].minValue = theta0;
    arcSetP->interval[0].maxValue = theta1;
    arcSetP->n = 1;
    }


/*-----------------------------------------------------------------*//**
*
* Add an interval with no test for min/max relationship
*
* @param minValue => new interval min.
* @param maxValue => new interval max
* @see #addArcSweep
* @see #setArcSweep
* @see #setUncheckedArcSweep
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void      bsiRange1d_addUnordered

(
SmallSetRange1dP setP,
double          minValue,
double          maxValue
)


    {
    int i = setP->n;
    if (i >= MSGEOM_SMALL_SET_SIZE)
        {
        /* ignore the overflow */
        }
    else
        {
        setP->interval[i].minValue = minValue;
        setP->interval[i].maxValue = maxValue;
        setP->n++;
        }
    }

/*-----------------------------------------------------------------*//**
*
* Add an interval with no test for min/max relationship
*
* @param minValue => new interval min.
* @param maxValue => new interval max
* @see #addArcSweep
* @see #setArcSweep
* @see #setUncheckedArcSweep
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void      bsiRange1d_addUnordered

(
SmallSetRange1dP setP,
double          minValue,
double          maxValue
)


    {
    int i = setP->n;
    if (i >= MSGEOM_SMALL_SET_SIZE)
        {
        /* ignore the overflow */
        }
    else
        {
        setP->interval[i].minValue = minValue;
        setP->interval[i].maxValue = maxValue;
        setP->n++;
        }
    }

/* MAP define jmdlRange1d_union(setCP,setAP,setBP) setCP->union(setAP,setBP) ENDMAP */

/*-----------------------------------------------------------------*//**
*
* Unite two range sets A and B, returning sets of aggregated
* portions in C.
* All set pointers are ASSUMED to be distinct.
*
* @param SmallSetRange1d *setAP => First input intervals
* @param SmallSetRange1d *setBP           => Second input intervals
* @return false if too many intervals in set.
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRange1d_union

(
SmallSetRange1dP setCP,
SmallSetRange1dP setAP,
SmallSetRange1dP setBP
)

    {
    int     i,j,k;
    double  min1,max1,min2,max2;

    setCP->n = 0;
    for ( i = 0 ; i < setAP->n ; i++ )
        {
         min1 = setAP->interval[i].minValue;
         max1 = setAP->interval[i].maxValue;

        for ( j = 0 ; j < setBP->n ; j++ )
            {
            min2 = setBP->interval[j].minValue;
            max2 = setBP->interval[j].maxValue;

            if ((min2>max1) || (min1>max2))
                continue;
            else
                {
                if (setCP->n >= MSGEOM_SMALL_SET_SIZE)
                    {
                     return false;
                    }

                else
                    {
                    k = setCP->n++;
                    if (min1<min2)
                        setCP->interval[k].minValue = min1;
                    else
                        setCP->interval[k].minValue = min2;
                    if (max1>max2)
                        setCP->interval[k].maxValue = max1;
                    else
                        setCP->interval[k].maxValue = max2;
                    }

                }

            }
        }
    return true;
    }

/* MAP define bsiRange1d_sort(setP) setP->sort() ENDMAP */

/*-----------------------------------------------------------------*//**
* Sort a range set in place.
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void      bsiRange1d_sort

(
SmallSetRange1dP setP
)

    {
    int unsorted, location, done, totNumber;
    Range1d nextItem;

    totNumber = setP->n;

    for (unsorted = 1;unsorted < totNumber ;unsorted++)
        {
         nextItem = setP->interval[unsorted];
         location = unsorted;
         done = false;
         while ((location > 0) && !done)
            {
             if (setP->interval[location -1].minValue > nextItem.minValue)
                {
                setP->interval[location] = setP->interval[location-1];
                location--;
                }
             else
                done = true;
             setP->interval[location] = nextItem;

            }
        }

    }

/* MAP define bsiRange1d_pointIsIn(setP,value) setP->pointIsIn(value) ENDMAP */

/*-----------------------------------------------------------------*//**
*
* Test if a given value is contained in the range set.
*
* @param value => value to test against set
* @return true if point is contained in one of the intervals of the range set.
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool         bsiRange1d_pointIsIn

(
SmallSetRange1dCP setP,
double          value
)



    {
    int i;
    for (i = 0; i < setP->n; i++)
        {
        if (bsiTrig_angleInSweep (value,
                    setP->interval[i].minValue,
                    setP->interval[i].maxValue
                        - setP->interval[i].minValue))
            {
            return true;
            }
        }
    return false;
    }


/* MAP define jmdlRange1d_intersect(setCP,setAP,setBP) setCP->subtract(setAP,setBP) ENDMAP */

/*-----------------------------------------------------------------*//**
*
* Subtract two range sets A and B, returning sets of remaining
* portions in C.
*
* @param setAP => First input intervals
* @param setBP => Second input intervals
* @return true if operation completed. false if too many intervals in result.
* @see #union
* @see #intersect
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRange1d_subtract


(
SmallSetRange1dP setCP,
SmallSetRange1dP setAP,
SmallSetRange1dP setBP
)

    {
    int     currB, currA, oldA, oldB;
    double  minA = DBL_MAX, maxA = -DBL_MAX, minB = DBL_MAX, maxB = -DBL_MAX;

    bool    boolStat = true;


    bsiRange1d_clear (setCP);

    if (setAP->n <= 0)
        {
        /* nothing in A */
        }
    if (setBP->n <= 0)
        {
        *setCP = *setAP;
        }
    else
        {
        bsiRange1d_sort (setAP);
        bsiRange1d_sort (setBP);

        /* Loop conditions:
            currA is the interval most recently extracted
                    from A.  currB similar.
            Whenever currA or currB advances, minA..maxA or minB..maxB
                    are set to the new interval limits.
            minA may move forward within interval currA if a B interval
                    is subtracted.  That is, minA is the LARGER of currA's
                    min and currB's max.
            Each pass through the loop advances EXACTLY one of currA and
            currB.  It may also advance minA so that the right end of
            the A interval is reconsidered on the next pass with a later B.
        */
        currA = currB = 0;
        oldA = oldB = -1;

        while (currA < setAP->n)
            {
            if (currA != oldA)
                {
                minA  = setAP->interval[currA].minValue;
                maxA  = setAP->interval[currA].maxValue;
                oldA = currA;
                }

            if (currB >= setBP->n)
                {
                /* No more B to subtract.  Force this A interval to the output
                    and increment for next pass */
                bsiRange1d_addUnordered(setCP, minA, maxA);
                currA++;
                }
            else
                {

                if (currB != oldB)
                    {
                    minB  = setBP->interval[currB].minValue;
                    maxB  = setBP->interval[currB].maxValue;
                    oldB = currB;
                    }

                if (maxB <= minA)
                    {
                    /* This B interval is off to the left skip to next */
                    currB++;
                    }
                else if (minB >= maxA)
                    {
                    /* This B interval is fully to the right.  Move on from A */
                    bsiRange1d_addUnordered (setCP, minA, maxA);
                    currA++;
                    }
                else
                    {
                    /* There is at least some overlap. */
                    if (minB > minA)
                        bsiRange1d_addUnordered (setCP, minA, minB);
                    if (maxB < maxA)
                        {
                        /* move minA ahead within the A interval, let next pass
                                see what things to the right need to be put in C */
                        minA = maxB;
                        currB++;
                        }
                    else
                        {
                        /* This B completely covers this A.  Move on. */
                        currA++;
                        }
                    }
                }
             }
         }
    return boolStat;
    }


/*-----------------------------------------------------------------*//**
*
* Intersect two range sets A and B, returning sets of overlapping
* portions in C.
*
* @param setAP => First input intervals
* @param setBP => Second input intervals
* @return true if operation completed. false if too many intervals in result.
* @see #union
* @see #subtract
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRange1d_intersect

(
SmallSetRange1dP setCP,
SmallSetRange1dP setAP,
SmallSetRange1dP setBP
)


    {
    int     currB, currA, oldA, oldB;
    double  minA = DBL_MAX, maxA = -DBL_MAX, minB = DBL_MAX, maxB = -DBL_MAX;
    bool    boolStat = true;


    bsiRange1d_clear (setCP);

    if (setAP->n <= 0)
        {
        /* nothing in A */
        }
    if (setBP->n <= 0)
        {
        /* nothing to intersect with */
        }
    else
        {
        bsiRange1d_sort (setAP);
        bsiRange1d_sort (setBP);

        /* Loop conditions:
            currA is the interval most recently extracted
                    from A.  currB similar.
            Whenever currA or currB advances, minA..maxA or minB..maxB
                    are set to the new interval limits.
            minA may move forward within interval currA if a B interval
                    is subtracted.  That is, minA is the LARGER of currA's
                    min and currB's max.
            Each pass through the loop advances EXACTLY one of currA and
            currB.  It may also advance minA so that the right end of
            the A interval is reconsidered on the next pass with a later B.
        */
        currA = currB = 0;
        oldA = oldB = -1;

        while (currA < setAP->n)
            {
            if (currA != oldA)
                {
                minA  = setAP->interval[currA].minValue;
                maxA  = setAP->interval[currA].maxValue;
                oldA = currA;
                }

            if (currB >= setBP->n)
                {
                /* No more B to intersect.  Force this A interval to the output
                    and increment for next pass */
                currA++;
                }
            else
                {

                if (currB != oldB)
                    {
                    minB  = setBP->interval[currB].minValue;
                    maxB  = setBP->interval[currB].maxValue;
                    oldB = currB;
                    }

                if (maxB <= minA)
                    {
                    /* This B interval is off to the left skip to next */
                    currB++;
                    }
                else if (minB >= maxA)
                    {
                    /* This B interval is fully to the right.  Move on from A */
                    currA++;
                    }
                else
                    {
                    /* There is at least some overlap. */
                    if (minB > minA)
                        minA = minB;
                    if (maxB < maxA)
                        {
                        /* move minA ahead within the A interval, let next pass
                                see what things to the right need to be put in C */
                        bsiRange1d_addUnordered (setCP, minA, maxB);
                        minA = maxB;
                        currB++;
                        }
                    else
                        {
                        /* This B completely covers this A.  Move on. */
                        bsiRange1d_addUnordered (setCP, minA, maxA);
                        currA++;
                        }
                    }
                }
             }
         }
    return boolStat;
    }


/*-----------------------------------------------------------------*//**
Drop a specified sector. Sectors AFTER the dropped spot are shifted down.
@param setP IN OUT interval set
@param sector IN index of sector to drop.
@bsihdr                                                 EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRange1d_dropSector
(
SmallSetRange1dP setP,
int sector
)
    {
    if (sector >= 0 && sector < setP->n)
        {
        for (int i = sector + 1; i < setP->n; i++)
            setP->interval[i-1] = setP->interval[i];
        setP->n -= 1;
        }
    }

/*-----------------------------------------------------------------*//**
Try to join two sectors.
@param pShift OUT shift (add to a or subtract from b) to make a, b match.
@param a IN first value
@param b IN second value
@param period IN optional period (ignored if not positive)
@param tolerance IN tolerance
@bsihdr                                                 EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static bool    valuesMatchWithPeriodShift
(
double *pShift,
double a,
double b,
double period,
double tolerance
)
    {
    double delta = b - a;
    double absdelta = fabs (delta);
    if (absdelta < tolerance)
        {
        *pShift = 0.0;
        return true;
        }

    if (period > 0.0)
        {
        int numPeriod = (int) floor (0.5 + absdelta / period);
        double del = fabs (absdelta - numPeriod * period);
        if (del < tolerance)
            {
            if (delta < 0.0)
                numPeriod = -numPeriod;
            *pShift = numPeriod * period;
            return true;
            }
        }
    return false;
    }

/*-----------------------------------------------------------------*//**
Try to join two sectors.
@param setP IN OUT interval set
@param sector IN index of sector to drop.
@bsihdr                                                 EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static bool    joinSectors
(
Range1d *pIntervalA,
Range1d const *pIntervalB,
double period,
double tolerance
)
    {
    double a0 = pIntervalA->minValue;
    double a1 = pIntervalA->maxValue;
    double da = a1 - a0;

    double b0 = pIntervalB->minValue;
    double b1 = pIntervalB->maxValue;
    double db = b1 - b0;

    if (da * db <= 0.0)
        return false;

    double shift;
    if (valuesMatchWithPeriodShift (&shift, a1, b0, period, tolerance))
        {
        pIntervalA->maxValue = b1 - shift;
        return true;
        }

    if (valuesMatchWithPeriodShift (&shift, b1, a0, period, tolerance))
        {
        pIntervalA->minValue = b0 + shift;
        return true;
        }
    return false;
    }


/*-----------------------------------------------------------------*//**
Join intervals end-to-end, optionally allowing periodic step.
@param setP IN OUT intervals to consolidate
@param period IN period.  0 means non-periodic.
@bsihdr                                                 EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRange1d_consolidatePeriodic
(
SmallSetRange1dP setP,
double period
)
    {
    double tolerance = bsiTrig_smallAngle ();
    // Sweep forward from each sector.
    // Note that setP->n can change ....
    int nextBaseSector;
    for (int baseSector = 0; baseSector < setP->n; baseSector = nextBaseSector)
        {
        nextBaseSector = baseSector + 1;
        for (int childSector = baseSector + 1; childSector < setP->n; childSector++)
            {
            if (joinSectors (&setP->interval[baseSector], &setP->interval[childSector], period, tolerance))
                {
                nextBaseSector = baseSector;    // force rescan of the (smaller) tail
                bsiRange1d_dropSector (setP, childSector);
                }
            }
        }
    }END_BENTLEY_GEOMETRY_NAMESPACE
