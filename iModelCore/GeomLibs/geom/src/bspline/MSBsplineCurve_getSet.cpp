/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bspline/MSBsplineCurve_getSet.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <memory.h>
#include <stdlib.h>
#include <math.h>
#if defined (INCLUDE_PPL)
    #include <Bentley/Iota.h>
    #include <ppl.h>
    //#define USE_PPL
    #if !defined (USE_PPL)
        #include <algorithm>
    #endif
#endif
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#include "msbsplinemaster.h"


// Index correctors
// 1) curve.params.numPoles is an int, but don't check it for negative.
// 2) the compiler will pick size_t or int flavor at the point of call
inline bool ValidateIndex (size_t i, MSBsplineCurveCP curve)
    {
    return i < (size_t)curve->params.numPoles;
    }

inline bool ValidateIndex (int i, MSBsplineCurveCP curve)
    {
    return i >= 0 &&  i < curve->params.numPoles;
    }

inline bool CorrectReverseIndex (int &i, MSBsplineCurveCP curve, bool reverse)
    {
    if (i >= 0 && i < curve->params.numPoles)
        {
        if (reverse)
            i = curve->params.numPoles - 1 - i;
        return true;
        }
    return false;
    }

inline bool CorrectReverseIndex (size_t &i, MSBsplineCurveCP curve, bool reverse)
    {
    size_t n = curve->params.numPoles;
    if (i < n)
        {
        if (reverse)
            i = n - 1 - i;
        return true;
        }
    return false;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             05/13
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d MSBsplineCurve::GetPole (size_t i) const
    {
    if (ValidateIndex (i, this))
        return poles[i];
    return DPoint3d::From (0,0,0);
    }

DPoint3d MSBsplineCurve::GetPole (int i) const
    {
    if (ValidateIndex (i, this))
        return poles[i];
    return DPoint3d::From (0,0,0);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             05/13
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d MSBsplineCurve::GetReversePole (size_t i) const
    {
    if (CorrectReverseIndex (i, this, true))
        return poles[i];
    return DPoint3d::From (0,0,0);
    }

DPoint3d MSBsplineCurve::GetReversePole (int i) const
    {
    if (CorrectReverseIndex (i, this, true))
        return poles[i];
    return DPoint3d::From (0,0,0);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             05/13
+---------------+---------------+---------------+---------------+---------------+------*/
double MSBsplineCurve::GetWeight (size_t i) const
    {
    if (weights != NULL && ValidateIndex (i, this))
        return weights[i];
    return 1.0;
    }


double MSBsplineCurve::GetWeight (int i) const
    {
    if (weights != NULL && ValidateIndex (i, this))
        return weights[i];
    return 1.0;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             05/13
+---------------+---------------+---------------+---------------+---------------+------*/
double MSBsplineCurve::GetReverseWeight (size_t i) const
    {
    if (weights != NULL && CorrectReverseIndex (i, this, true))
        return weights[i];
    return 1.0;
    }

double MSBsplineCurve::GetReverseWeight (int i) const
    {
    if (weights != NULL && CorrectReverseIndex (i, this, true))
        return weights[i];
    return 1.0;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             05/13
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d MSBsplineCurve::GetUnWeightedPole (size_t i, bool reverse) const
    {
    if (CorrectReverseIndex (i, this, reverse))
        {
        DPoint3d xyz = poles[i];
        if (weights != NULL && weights[i] != 0.0)
            xyz.Scale (1.0 / weights[i]);
        return xyz;
        }
    return DPoint3d::From (0,0,0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             05/13
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineCurve::TransformPoles (TransformCR transform, size_t i0, size_t n)
    {
    for (size_t k = 0; k < n; k++)
        {
        size_t i = i0 + k;
        if (ValidateIndex (i, this))
            {
            DPoint3d xyz = GetUnWeightedPole (i);
            transform.Multiply (xyz);
            SetReWeightedPole (i, xyz);
            }
        }
    }



DPoint3d MSBsplineCurve::GetUnWeightedPole (int i, bool reverse) const
    {
    if (CorrectReverseIndex (i, this, reverse))
        {
        DPoint3d xyz = poles[i];
        if (weights != NULL && weights[i] != 0.0)
            xyz.Scale (1.0 / weights[i]);
        return xyz;
        }
    return DPoint3d::From (0,0,0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineCurve::SetWeight (size_t i, double value)
    {
    if (weights != NULL && ValidateIndex (i, this))
        {
        weights[i] = value;
        return true;
        }
    return false;
    }

bool MSBsplineCurve::SetWeight (int i, double value)
    {
    if (weights != NULL && ValidateIndex (i, this))
        {
        weights[i] = value;
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineCurve::SetReWeightedPole (size_t i, DPoint3dCR value, bool reverse)
    {
    if (poles != NULL && CorrectReverseIndex (i, this, reverse))
        {
        DPoint3d xyz = value;
        if (NULL != weights)
            xyz.Scale (weights[i]);
        poles[i] = xyz;
        return true;
        }
    return false;
    }

bool MSBsplineCurve::SetReWeightedPole (int i, DPoint3dCR value, bool reverse)
    {
    if (poles != NULL && CorrectReverseIndex (i, this, reverse))
        {
        DPoint3d xyz = value;
        if (NULL != weights)
            xyz.Scale (weights[i]);
        poles[i] = xyz;
        return true;
        }
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineCurve::SetPole (size_t i, double x, double y, double z)
    {
    if (poles != NULL && ValidateIndex (i, this))
        {
        poles[i].x = x;
        poles[i].y = y;
        poles[i].z = z;
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             09/09
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint4d MSBsplineCurve::GetPoleDPoint4d (size_t i) const
    {
    DPoint4d xyzw;
    if (i < (size_t)this)
        {
        xyzw.x = poles[i].x;
        xyzw.y = poles[i].y;
        xyzw.z = poles[i].z;
        xyzw.w = weights  != NULL ? weights[i] : 1.0;
        }
    else
        xyzw.Zero ();
    return xyzw;
    }




/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineCurve::SetPole (size_t i, DPoint3dCR value)
    {
    if (poles != NULL && ValidateIndex (i, this))
        {
        poles[i] = value;
        return true;
        }
    return false;
    }
    
bool MSBsplineCurve::SetPole (int i, DPoint3dCR value)
    {
    if (poles != NULL && ValidateIndex (i, this))
        {
        poles[i] = value;
        return true;
        }
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void   MSBsplineCurve::GetKnotRange (double &knot0, double &knot1) const
    {
    knot0 = knots[params.order - 1];
    int numKnots = params.NumberAllocatedKnots ();
    knot1 = knots[numKnots - params.order];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
DSegment1d MSBsplineCurve::GetKnotRange () const
    {
    double knot0, knot1;
    GetKnotRange (knot0, knot1);
    return DSegment1d (knot0, knot1);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void   MSBsplineCurve::GetKnotRange
(
double &knotA,
double &knotB,
int &indexA,
int &indexB,
double &tolerance
) const
    {
    indexA = params.order - 1;
    indexB  = params.NumberAllocatedKnots () - params.order;
    knotA = knots[indexA];
    knotB = knots[indexB];
    tolerance = bspknot_knotTolerance ((MSBsplineCurveCP)this);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             05/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineCurve::GetCurveDisplay () const {return display.curveDisplay != 0;}
bool MSBsplineCurve::GetPolygonDisplay () const {return display.polygonDisplay != 0;}
void MSBsplineCurve::SetCurveDisplay (bool value) {display.curveDisplay = value ? 1 : 0;}
void MSBsplineCurve::SetPolygonDisplay (bool value) {display.polygonDisplay = value ? 1 : 0;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             09/09
+---------------+---------------+---------------+---------------+---------------+------*/
double MSBsplineCurve::GetKnot (size_t i) const 
    {
    if (i >= (size_t) params.NumberAllocatedKnots ())
        return 0.0;
    return knots[i];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineCurve::SetKnot (size_t i, double value)
    {
    if (i >= (size_t) params.NumberAllocatedKnots ())
        return false;
    knots[i] = value;
    return true;
    }




/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
int MSBsplineCurve::GetIntOrder () const
    {
    return params.order;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
size_t MSBsplineCurve::GetOrder () const
    {
    return (size_t)params.order;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineCurve::HasWeights() const
    {
    return rational && (weights != NULL);
    }


void MSBsplineCurve::GetPoles (bvector<DPoint3d> &data) const
    {
    data.clear ();
    data.reserve ((size_t)params.numPoles);
    for (size_t i = 0; i < (size_t)params.numPoles; i++)
        data.push_back(poles[i]);
    }

void MSBsplineCurve::GetWeights (bvector<double> &data) const
    {
    data.clear ();
    if (rational && NULL != weights)
        {
        data.reserve ((size_t)params.numPoles);
        for (size_t i = 0; i < (size_t)params.numPoles; i++)
            data.push_back(weights[i]);
        }
    }

void MSBsplineCurve::GetKnots (bvector<double> &data) const
    {
    data.clear ();
    size_t n = (size_t)params.NumberAllocatedKnots ();
    data.reserve (n);
    for (size_t i = 0; i < n; i++)
        data.push_back(knots[i]);
    }

void MSBsplineCurve::GetPoles4d (bvector<DPoint4d> &data) const
    {
    data.clear ();
    data.reserve((size_t)params.numPoles);
    DPoint4d xyzw;
    for (int i = 0; i < params.numPoles; i++)
        {
        xyzw.x = poles[i].x;
        xyzw.y = poles[i].y;
        xyzw.z = poles[i].z;
        xyzw.w = weights[i];
        data.push_back(xyzw);
        }
    }

void MSBsplineCurve::GetUnWeightedPoles (bvector<DPoint3d> &data) const
    {
    data.clear ();
    data.reserve((size_t)params.numPoles);
    if (NULL != weights)
        {
        for (int i = 0; i < params.numPoles; i++)
            {
            DPoint3d xyz = poles[i];
            xyz.Scale (1.0 / weights[i]);
            data.push_back (xyz);
            }
        }
    else
        {
        for (int i = 0; i < params.numPoles; i++)
            data.push_back(poles[i]);
        }
    }




/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             09/09
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d const *MSBsplineCurve::GetPoleCP () const
    {
    return poles;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             09/09
+---------------+---------------+---------------+---------------+---------------+------*/
double const *MSBsplineCurve::GetWeightCP () const
    {
    return weights;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             09/09
+---------------+---------------+---------------+---------------+---------------+------*/
double const *MSBsplineCurve::GetKnotCP () const
    {
    return knots;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             09/09
+---------------+---------------+---------------+---------------+---------------+------*/
DRange1d MSBsplineCurve::GetWeightRange () const
    {
    if (!weights)
        return DRange1d::From (1.0);
    DRange1d range = DRange1d::From (weights[0]);
    for (int i = 0; i < params.numPoles; i++)
        range.Extend (weights[i]);
    return range;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             09/09
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d *MSBsplineCurve::GetPoleP () const
    {
    return poles;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             09/09
+---------------+---------------+---------------+---------------+---------------+------*/
double *MSBsplineCurve::GetWeightP () const
    {
    return weights;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             09/09
+---------------+---------------+---------------+---------------+---------------+------*/
double *MSBsplineCurve::GetKnotP () const
    {
    return knots;
    }




/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineCurve::SetPoles (size_t index, DPoint3dCP value, size_t n)
    {
    if (poles != NULL && index + n <= (size_t)params.numPoles)
        {
        for (size_t i = 0; i < n; i++)
            poles[index + i] = value[i];
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineCurve::SetWeights (size_t index, double const * value, size_t n)
    {
    if (poles != NULL && index + n <= (size_t)params.numPoles)
        {
        for (size_t i = 0; i < n; i++)
            weights[index + i] = value[i];
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineCurve::SetKnots (size_t index, double const * value, size_t n)
    {
    size_t numKnots = NumberAllocatedKnots ();

    if (poles != NULL && index + n <= numKnots)
        {
        for (size_t i = 0; i < n; i++)
            knots[index + i] = value[i];
        return true;
        }
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             09/09
+---------------+---------------+---------------+---------------+---------------+------*/
int MSBsplineCurve::NumberAllocatedPoles () const {return params.numPoles;}
int MSBsplineCurve::NumberAllocatedKnots () const {return params.NumberAllocatedKnots ();}

size_t MSBsplineCurve::GetNumPoles () const {return (size_t)params.numPoles;}
size_t MSBsplineCurve::GetNumKnots () const {return (size_t)params.NumberAllocatedKnots ();}

int MSBsplineCurve::GetIntNumPoles () const {return params.numPoles;}
int MSBsplineCurve::GetIntNumKnots () const {return params.NumberAllocatedKnots ();}
END_BENTLEY_GEOMETRY_NAMESPACE