/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
static inline bool ValidateIndex (size_t i, MSBsplineSurfaceCP surf)
    {
    return i < (size_t)(surf->uParams.numPoles * surf->vParams.numPoles);
    }

static inline bool ValidateIndex (size_t i, size_t j, MSBsplineSurfaceCP surf, size_t &index)
    {
    if (surf->poles != NULL 
        && i < (size_t)surf->uParams.numPoles
        && j < (size_t)surf->vParams.numPoles)
        {
        index = i + j * (size_t)surf->uParams.numPoles;
        return true;
        }
    index = 0;
    return false;
    }


static inline bool ValidateIndex (int i, MSBsplineSurfaceCP surf)
    {
    return i >= 0 &&  i < (surf->uParams.numPoles * surf->vParams.numPoles);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             05/13
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d MSBsplineSurface::GetPole (size_t i) const
    {
    if (ValidateIndex (i, this))
        return poles[i];
    return DPoint3d::From (0,0,0);
    }

DPoint3d MSBsplineSurface::GetPole (int i) const
    {
    if (ValidateIndex (i, this))
        return poles[i];
    return DPoint3d::From (0,0,0);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             05/13
+---------------+---------------+---------------+---------------+---------------+------*/
double MSBsplineSurface::GetWeight (size_t i) const
    {
    if (weights != NULL && ValidateIndex (i, this))
        return weights[i];
    return 1.0;
    }


double MSBsplineSurface::GetWeight (int i) const
    {
    if (weights != NULL && ValidateIndex (i, this))
        return weights[i];
    return 1.0;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             05/13
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d MSBsplineSurface::GetUnWeightedPole (size_t i) const
    {
    if (ValidateIndex (i, this))
        {
        DPoint3d xyz = poles[i];
        if (weights != NULL && weights[i] != 0.0)
            xyz.Scale (1.0 / weights[i]);
        return xyz;
        }
    return DPoint3d::From (0,0,0);
    }

DPoint3d MSBsplineSurface::GetUnWeightedPole (int i) const
    {
    if (ValidateIndex (i, this))
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
void MSBsplineSurface::TransformPoles (TransformCR transform, size_t i0, size_t j0, size_t numI, size_t numJ)
    {
    DPoint3d xyz;
    for (size_t jj = 0; jj < numJ; jj++)
        {
        for (size_t ii = 0; ii < numI; ii++)
            {
            size_t i = i0 + ii;
            size_t j = j0 + jj;
            if (TryGetUnWeightedPole (i, j, xyz))
                {
                transform.Multiply (xyz);
                SetReWeightedPole (i, j, xyz);
                }
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineSurface::SetWeight (size_t i, double value)
    {
    if (weights != NULL && ValidateIndex (i, this))
        {
        weights[i] = value;
        return true;
        }
    return false;
    }

bool MSBsplineSurface::SetWeight (int i, double value)
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
bool MSBsplineSurface::SetReWeightedPole (size_t i, DPoint3dCR value)
    {
    if (poles != NULL && ValidateIndex (i, this))
        {
        DPoint3d xyz = value;
        if (NULL != weights)
            xyz.Scale (weights[i]);
        poles[i] = xyz;
        return true;
        }
    return false;
    }

bool MSBsplineSurface::SetReWeightedPole (size_t i, size_t j, DPoint3dCR value)
    {
    size_t index;
    if (ValidateIndex (i, j, this, index))
        {
        DPoint3d xyz = value;
        if (NULL != weights)
            xyz.Scale (weights[index]);
        poles[index] = xyz;
        return true;
        }
    return false;
    }

bool MSBsplineSurface::SetPole (size_t i, size_t j, DPoint3dCR value)
    {
    size_t index;
    if (ValidateIndex (i, j, this, index))
        {
        poles[index] = value;
        return true;
        }
    return false;
    }

bool MSBsplineSurface::SetReWeightedPole (int i, DPoint3dCR value)
    {
    if (poles != NULL && ValidateIndex (i, this))
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
bool MSBsplineSurface::SetPole (size_t i, double x, double y, double z)
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
DPoint4d MSBsplineSurface::GetPoleDPoint4d (size_t i) const
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
bool MSBsplineSurface::SetPole (size_t i, DPoint3dCR value)
    {
    if (poles != NULL && ValidateIndex (i, this))
        {
        poles[i] = value;
        return true;
        }
    return false;
    }
    
bool MSBsplineSurface::SetPole (int i, DPoint3dCR value)
    {
    if (poles != NULL && ValidateIndex (i, this))
        {
        poles[i] = value;
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
  7 count queries size_t GetXXX with partner int GetIntXXX
* @bsimethod                                    Earlin.Lutz             09/09
+---------------+---------------+---------------+---------------+---------------+------*/
size_t MSBsplineSurface::GetNumPoles () const {return (size_t)uParams.numPoles * (size_t)vParams.numPoles;}
size_t MSBsplineSurface::GetNumUPoles () const {return (size_t)uParams.numPoles;}
size_t MSBsplineSurface::GetNumVPoles () const {return (size_t)vParams.numPoles;}
size_t MSBsplineSurface::GetUOrder () const {return (size_t)uParams.order;}
size_t MSBsplineSurface::GetVOrder () const {return (size_t)vParams.order;}
size_t MSBsplineSurface::GetNumUKnots () const {return (size_t)uParams.NumberAllocatedKnots ();}
size_t MSBsplineSurface::GetNumVKnots () const {return (size_t)vParams.NumberAllocatedKnots ();}

int MSBsplineSurface::GetIntNumPoles () const {return uParams.numPoles * vParams.numPoles;}
int MSBsplineSurface::GetIntNumUPoles () const {return uParams.numPoles;}
int MSBsplineSurface::GetIntNumVPoles () const {return vParams.numPoles;}
int MSBsplineSurface::GetIntUOrder () const {return uParams.order;}
int MSBsplineSurface::GetIntVOrder () const {return vParams.order;}
int MSBsplineSurface::GetIntNumUKnots () const {return uParams.NumberAllocatedKnots ();}
int MSBsplineSurface::GetIntNumVKnots () const {return vParams.NumberAllocatedKnots ();}

bool MSBsplineSurface::GetIsUClosed () const {return uParams.closed != 0;}
bool MSBsplineSurface::GetIsVClosed () const {return vParams.closed != 0;}


bool MSBsplineSurface::HasWeights () const {return rational && NULL != weights;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineSurface::SetNumRules (int numU, int numV)
    {
    uParams.numRules = numU;
    vParams.numRules = numV;
    }

void MSBsplineSurface::SetSurfaceDisplay (bool data)    {display.curveDisplay = data ? 1 : 0;}
void MSBsplineSurface::SetPolygonDisplay (bool data)    {display.polygonDisplay = data ? 1 : 0;}

bool MSBsplineSurface::GetSurfaceDisplay () const    {return display.curveDisplay   != 0;}
bool MSBsplineSurface::GetPolygonDisplay () const   {return display.polygonDisplay != 0;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineSurface::SetPoles (size_t index, DPoint3dCP value, size_t n)
    {
    size_t numPoles = (size_t)GetNumPoles ();
    if (poles != NULL && index + n <= numPoles)
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
bool MSBsplineSurface::SetWeights (size_t index, double const * value, size_t n)
    {
    size_t numPoles = (size_t)GetNumPoles ();
    if (poles != NULL && index + n <= numPoles)
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
bool MSBsplineSurface::SetUKnots (size_t index, double const * value, size_t n)
    {
    size_t numKnots = uParams.NumberAllocatedKnots ();

    if (poles != NULL && index + n <= numKnots)
        {
        for (size_t i = 0; i < n; i++)
            uKnots[index + i] = value[i];
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineSurface::SetVKnots (size_t index, double const * value, size_t n)
    {
    size_t numKnots = vParams.NumberAllocatedKnots ();

    if (poles != NULL && index + n <= numKnots)
        {
        for (size_t i = 0; i < n; i++)
            vKnots[index + i] = value[i];
        return true;
        }
    return false;
    }


void MSBsplineSurface::GetPoles (bvector<DPoint3d> &data) const
    {
    data.clear ();
    size_t n = (size_t)GetNumPoles ();
    data.reserve (n);
    for (size_t i = 0; i < n; i++)
        data.push_back(poles[i]);
    }

void MSBsplineSurface::GetWeights (bvector<double> &data) const
    {
    data.clear ();
    if (rational && NULL != weights)
        {
        size_t n = (size_t)GetNumPoles ();
        data.reserve (n);
        for (size_t i = 0; i < n; i++)
            data.push_back(weights[i]);
        }
    }

void MSBsplineSurface::GetUnWeightedPoles (bvector<DPoint3d> &data) const
    {
    data.clear ();
    size_t n = (size_t)GetNumPoles ();
    data.reserve(n);
    if (NULL != weights)
        {
        for (size_t i = 0; i < n; i++)
            {
            DPoint3d xyz = poles[i];
            xyz.Scale (1.0 / weights[i]);
            data.push_back (xyz);            
            }
        }
    else
        {
        for (size_t i = 0; i < n; i++)
            data.push_back(poles[i]);
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             05/13
+---------------+---------------+---------------+---------------+---------------+------*/
double MSBsplineSurface::GetWeight (size_t i, size_t j) const
    {
    size_t index;
    if (ValidateIndex (i, j, this, index))
        return weights[index];

    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             05/13
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d MSBsplineSurface::GetPole (size_t i, size_t j) const
    {
    size_t index;
    if (ValidateIndex (i, j, this, index))
        return poles[index];
    return DPoint3d::From (0,0,0);
    }

DPoint3d MSBsplineSurface::GetUnWeightedPole (size_t i, size_t j) const
  {
  DPoint3d xyz;
  TryGetUnWeightedPole (i, j, xyz);
  return xyz;
  }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             05/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineSurface::TryGetUnWeightedPole (size_t i, size_t j, DPoint3dR xyz) const
    {
    size_t index;
    if (ValidateIndex (i, j, this, index))
        {
        xyz = poles[index];
        if (NULL == weights)
            return true;
        double w = weights[index];
        double a;
        if (DoubleOps::SafeDivide (a, 1.0, w, 0.0))
            xyz.Scale (a);
        return true;        
        }
    xyz.Zero ();
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             05/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineSurface::TryGetUnWeightedPole (size_t i, DPoint3dR xyz) const
    {
    if (i < GetNumUPoles () * GetNumVPoles ())
        {
        xyz = poles[i];
        if (NULL == weights)
            return true;
        double w = weights[i];
        double a;
        if (DoubleOps::SafeDivide (a, 1.0, w, 0.0))
            xyz.Scale (a);
        return true;        
        }
    xyz.Zero ();
    return false;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             09/09
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint4d MSBsplineSurface::GetPoleDPoint4d (size_t i, size_t j) const
    {
    size_t index;
    if (ValidateIndex (i, j, this, index))
        {
        return DPoint4d::From (poles[index], weights[index]);
        }
    return DPoint4d::From (0,0,0,1);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             09/09
+---------------+---------------+---------------+---------------+---------------+------*/
double MSBsplineSurface::GetUKnot (size_t i) const
    {
    if (i < GetNumUKnots ())
        return uKnots[i];
    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             09/09
+---------------+---------------+---------------+---------------+---------------+------*/
double MSBsplineSurface::GetVKnot (size_t i) const
    {
    if (i < GetNumVKnots ())
        return vKnots[i];
    return 0.0;
    }


END_BENTLEY_GEOMETRY_NAMESPACE