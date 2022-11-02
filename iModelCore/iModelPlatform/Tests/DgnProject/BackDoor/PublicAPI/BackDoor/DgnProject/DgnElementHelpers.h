/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "BackDoor.h"

USING_NAMESPACE_BENTLEY_DGN

BEGIN_DGNDB_UNIT_TESTS_NAMESPACE

double const EPSILON = 0.000000001;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
* Adds an element to a model and verifies that it was added.
*
* @return SUCCESS if it worked, BSIERROR otherwise.
+---------------+---------------+---------------+---------------+---------------+------*/
#if defined (NEEDS_WORK_ELEMDSCR_REWORK)
StatusInt AddElementToModel (EditElementHandleR eeh);
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GenerateSpiralYAxis(DPoint3dP points, size_t sz);
void GeneratePoints(DPoint3dP points, size_t const sz);

/*---------------------------------------------------------------------------------**//**
* Arranges an array of points so that they represent an increasing 3D sinusoidal line
* for bests results numpoints should be >= 5
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GenerateSin3d (DPoint3dP points, size_t const numpoints, double const scale = 1000.0);

/*---------------------------------------------------------------------------------**//**
* Arranges an array of points so that they represent a circle on the XY plane
* for best results numpoints should be >= 4
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GenerateCircle (DPoint3dP points, size_t const numpoints, double const radius = 100.0);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool isDPoint3dNear_NoAsserts (DPoint3dCR left, DPoint3dCR right, double const Epsilon);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool isDPoint3dNear (DPoint3dCR left, DPoint3dCR right, double const Epsilon);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool isDVec3dNear (DVec3dCR left, DVec3dCR right, double const Epsilon);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool isDPoint3dArrayNear (DPoint3dCP left, DPoint3dCP right, double const epsilon, size_t const numPoints);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool isQuaternionNear (double const* left, double const* right, double const Epsilon);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool isDoubleArrayNear (double const* left, double const* right, size_t arraySize, double const Epsilon);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CopyPoints (DPoint3dP out, DPoint3dCP in, size_t const numPoints);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TranslatePointArray (DPoint3dP inout, DVec3d trans, size_t const numPoints);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int CalculateElementSize(int size);

struct GeomTests
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static double ComputeDistancePrecision (DPoint3dCR p1, DPoint3dCR p2)
    {
    DRange3d r = DRange3d::From (p1, p2);
    return r.LargestCoordinate() * 1.0e-14;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsPointOnPlane (DPoint3dCR pt, DPlane3dCR dPlane3d)
    {
    return dPlane3d.Evaluate (pt) <= ComputeDistancePrecision (pt, dPlane3d.origin);
    }
};

END_DGNDB_UNIT_TESTS_NAMESPACE