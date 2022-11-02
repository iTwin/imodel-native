/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*-----------------------------------------------------------------*//**
*
* Clear the range set to an empty set. (No intervals)
*
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiRange1d_clear
(
SmallSetRange1dP rangeSetP
);

/*-----------------------------------------------------------------*//**
*
* Adds an angular range to a SmallSetrange1d.  The angular range is
* normalized to the +=pi range, and is split into two parts if needed.
*
* @param theta0 IN      start of interval.
* @param dtheta IN      angular sweep.  may be negative.
* @see #setArcSweep
* @see #setUncheckedArcSweep
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRange1d_addArcSweep

(
SmallSetRange1dP arcSetP,
double          theta0,
double          dtheta
);

/*-----------------------------------------------------------------*//**
* Test if a the range set is a full 360 range in angle space.

* @param pRangeSet IN      range set to test
* @see
* @return true if the range set is a singel interval that is 2pi (within tolerance).
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRange1d_isFullCircle
(
SmallSetRange1dCP pRangeSet
);

/*-----------------------------------------------------------------*//**
*
* Set an arc sweep as the only entry in a small range set.
* All prior entries in the set are ignored.  The arc sweep is normalized
* and split as needed.
*
* @param theta0 IN      start of interval.
* @param dtheta IN      angular sweep.  May be negative.
* @see #addArcSweep
* @see #setUncheckedArcSweep
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiRange1d_setArcSweep

(
SmallSetRange1dP arcSetP,
double          theta0,
double          dtheta
);

/*-----------------------------------------------------------------*//**
*
* Set an arc sweep as the only entry in a small range set.
* All prior entries in the set are ignored.  The sweep is inserted
* as is, with no attempt to normalize direction or range.
* Caveat emptor.
*
* @param theta0 IN      Start of interval.
* @param dtheta IN      Angular sweep.  May be negative.
* @see #addArcSweep
* @see #setArcSweep
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiRange1d_setUncheckedArcSweep

(
SmallSetRange1dP arcSetP,
double          theta0,
double          dtheta
);

/*-----------------------------------------------------------------*//**
*
* Add an interval with no test for min/max relationship
*
* @param minValue IN      new interval min.
* @param maxValue IN      new interval max
* @see #addArcSweep
* @see #setArcSweep
* @see #setUncheckedArcSweep
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void      bsiRange1d_addUnordered

(
SmallSetRange1dP setP,
double          minValue,
double          maxValue
);

/*-----------------------------------------------------------------*//**
*
* Unite two range sets A and B, returning sets of aggregated
* portions in C.
* All set pointers are ASSUMED to be distinct.
*
* @param SmallSetRange1d *setAP IN      First input intervals
* @param SmallSetRange1d *setBP           IN      Second input intervals
* @return false if too many intervals in set.
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRange1d_union

(
SmallSetRange1dP setCP,
SmallSetRange1dP setAP,
SmallSetRange1dP setBP
);

/*-----------------------------------------------------------------*//**
* Sort a range set in place.
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void      bsiRange1d_sort

(
SmallSetRange1dP setP
);

/*-----------------------------------------------------------------*//**
*
* Test if a given value is contained in the range set.
*
* @param value IN      value to test against set
* @return true if point is contained in one of the intervals of the range set.
* @see
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool         bsiRange1d_pointIsIn

(
SmallSetRange1dCP setP,
double          value
);

/*-----------------------------------------------------------------*//**
*
* Subtract two range sets A and B, returning sets of remaining
* portions in C.
*
* @param setAP IN      First input intervals
* @param setBP IN      Second input intervals
* @return true if operation completed. false if too many intervals in result.
* @see #union
* @see #intersect
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRange1d_subtract


(
SmallSetRange1dP setCP,
SmallSetRange1dP setAP,
SmallSetRange1dP setBP
);

/*-----------------------------------------------------------------*//**
*
* Intersect two range sets A and B, returning sets of overlapping
* portions in C.
*
* @param setAP IN      First input intervals
* @param setBP IN      Second input intervals
* @return true if operation completed. false if too many intervals in result.
* @see #union
* @see #subtract
* @indexVerb
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRange1d_intersect

(
SmallSetRange1dP setCP,
SmallSetRange1dP setAP,
SmallSetRange1dP setBP
);

/*-----------------------------------------------------------------*//**
Drop a specified sector. Sectors AFTER the dropped spot are shifted down.
@param setP IN OUT interval set
@param sector IN index of sector to drop.
@bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRange1d_dropSector
(
SmallSetRange1dP setP,
int sector
);

/*-----------------------------------------------------------------*//**
Join intervals end-to-end, optionally allowing periodic step.
@param setP IN OUT intervals to consolidate
@param period IN period.  0 means non-periodic.
@bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRange1d_consolidatePeriodic
(
SmallSetRange1dP setP,
double period
);

