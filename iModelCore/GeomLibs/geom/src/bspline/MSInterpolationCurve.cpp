/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define MDLERR_NOPOLES ERROR
#define MDLERR_INSFMEMORY ERROR

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MSInterpolationCurve::ReleaseMem ()
    {
    bspcurv_freeInterpolationCurve (this);
    }

int MSInterpolationCurve::GetOrder () const
    {
    return params.order;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/09
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSInterpolationCurve::AllocateFitPoints (int count, DPoint3dCP data)
    {
    int allocSize = count * sizeof (DPoint3d);
    if (NULL == (fitPoints = static_cast<DPoint3d *>(BSIBaseGeom::Malloc (allocSize))))
        return MDLERR_INSFMEMORY;
    if (NULL != data)
        memcpy (fitPoints, data, allocSize);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/09
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSInterpolationCurve::AllocateKnots (int count, double const * data)
    {
    int allocSize = count * sizeof (double);
    if (NULL == (knots = static_cast<double *>(BSIBaseGeom::Malloc (allocSize))))
        return MDLERR_INSFMEMORY;
    if (NULL != data)
        memcpy (knots, data, allocSize);
    return SUCCESS;
    }

//! Allocate memory for the B-spline curve and copies all data from the input B-spline curve.
MSBsplineStatus MSInterpolationCurve::CopyFrom (MSInterpolationCurveCR source)
    {
    *this = source;
    this->fitPoints = NULL;
    this->knots     = NULL;
    if (SUCCESS == AllocateKnots (source.params.numKnots, source.knots)
        && SUCCESS == AllocateFitPoints (source.params.numPoints, source.fitPoints))
        return SUCCESS;
    return ERROR;
    }

void MSInterpolationCurve::Zero ()
    {
    memset (this, 0, sizeof (MSInterpolationCurve));
    }

MSBsplineStatus MSInterpolationCurve::InitFromPointsAndEndTangents (DPoint3d *inPts, int numPts, bool remvData, double remvTol, DPoint3d *endTangents, 
                                        bool closedCurve, bool colinearTangents, bool chordLenTangents, bool naturalTangents)
    {
    return bspcurv_constructInterpolationCurve (this, inPts, numPts, remvData, remvTol, endTangents, closedCurve, closedCurve ? false : true,
                            colinearTangents, chordLenTangents, naturalTangents);
    }

MSBsplineStatus MSInterpolationCurve::InitFromPointsAndEndTangents (bvector<DPoint3d> &points, bool remvData, double remvTol, DPoint3d *endTangents, 
                                        bool closedCurve, bool colinearTangents, bool chordLenTangents, bool naturalTangents)
    {
    if (points.size () < 2)
        return ERROR;
    return bspcurv_constructInterpolationCurve (this, &points[0], (int)points.size (), remvData, remvTol, endTangents, closedCurve, closedCurve ? false : true,
                            colinearTangents, chordLenTangents, naturalTangents);
    }

MSBsplineStatus MSInterpolationCurve::Populate
(
int order,
bool isPeriodic,
int isChordLenKnots,
int isColinearTangents,
int isChordLenTangents,
int isNaturalTangents,
DPoint3dCP pFitPoints,
int numFitPoints,
double const *pKnots,
int numKnots,
DVec3dCP tangent0,
DVec3dCP tangent1
)
    {
    Zero ();
    if (numFitPoints < 0)
        numFitPoints = 0;
    if (numKnots < 0)
        numKnots = 0;
    display.polygonDisplay = 1;
    display.curveDisplay = 1;
    display.rulesByLength = 0;

    params.order = order;
    params.isPeriodic = (int)isPeriodic;
    params.numPoints = numFitPoints;
    params.numKnots = numKnots;
    
    params.isChordLenKnots = isChordLenKnots;
    params.isColinearTangents = isColinearTangents;
    params.isChordLenTangents = isChordLenTangents;
    params.isNaturalTangents = isNaturalTangents;

    if (tangent0 != nullptr)
        startTangent = *tangent0;
    if (tangent1 != nullptr)
        endTangent = *tangent1;


    if (numFitPoints > 0)
        {
        size_t numBytes = (size_t)(numFitPoints * sizeof(DPoint3d));
        fitPoints = (DPoint3d*)BSIBaseGeom::Malloc (numBytes);
        memcpy (fitPoints, pFitPoints, numBytes);
        }
    if (numKnots > 0)
        {
        size_t numBytes = (size_t)(numKnots * sizeof(double));
        knots = (double*)BSIBaseGeom::Malloc (numBytes);
        memcpy (knots, pKnots, numBytes);
        }
    return SUCCESS;        
    }


bool MSInterpolationCurve::AlmostEqual (MSInterpolationCurveCR other, double tolerance) const
    {
    if (params.order != other.params.order)
        return false;
    if (params.isPeriodic != other.params.isPeriodic)
        return false;
    if (params.numPoints != other.params.numPoints)
        return false;
    if (params.numKnots != other.params.numKnots)
        return false;

    if (params.isChordLenKnots != other.params.isChordLenKnots)
        return false;
    if (params.isColinearTangents != other.params.isColinearTangents)
        return false;
    if (params.isChordLenTangents != other.params.isChordLenTangents)
        return false;
    if (params.isNaturalTangents != other.params.isNaturalTangents)
        return false;
    if (!startTangent.AlmostEqual (other.startTangent))
        return false;
    if (!endTangent.AlmostEqual (other.endTangent))
        return false;

    for (int i = 0; i < params.numPoints; i++)
        if (!fitPoints[i].AlmostEqual (other.fitPoints[i]))
            return false;
    double tol = DoubleOps::SmallCoordinateRelTol () * (1.0 + fabs (knots[params.numKnots - 1]) + fabs (knots[0]));
    for (int i = 0; i < params.numKnots; i++)
        if (fabs (knots[i] - other.knots[i]) > tol)
            return false;

    return true;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
