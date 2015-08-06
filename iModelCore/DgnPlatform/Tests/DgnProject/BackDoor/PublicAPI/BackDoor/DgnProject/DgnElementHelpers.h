/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/BackDoor/PublicAPI/BackDoor/DgnProject/DgnElementHelpers.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <UnitTests/BackDoor/DgnProject/BackDoor.h>

USING_NAMESPACE_BENTLEY_DGN

BEGIN_DGNDB_UNIT_TESTS_NAMESPACE

double const EPSILON = 0.000000001;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      05/09
* Adds an element to a model and verifies that it was added.
*
* @return SUCCESS if it worked, BSIERROR otherwise.
+---------------+---------------+---------------+---------------+---------------+------*/
#if defined (NEEDS_WORK_ELEMDSCR_REWORK)
StatusInt AddElementToModel (EditElementHandleR eeh);
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      05/09
+---------------+---------------+---------------+---------------+---------------+------*/
void GenerateSpiralYAxis(DPoint3dP points, size_t sz);
void GeneratePoints(DPoint3dP points, size_t const sz);

/*---------------------------------------------------------------------------------**//**
* Arranges an array of points so that they represent an increasing 3D sinusoidal line
* for bests results numpoints should be >= 5
* @bsimethod                                                     AlexHowe       08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void GenerateSin3d (DPoint3dP points, size_t const numpoints, double const scale = 1000.0);

/*---------------------------------------------------------------------------------**//**
* Arranges an array of points so that they represent a circle on the XY plane
* for best results numpoints should be >= 4
* @bsimethod                                                     AlexHowe       08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void GenerateCircle (DPoint3dP points, size_t const numpoints, double const radius = 100.0);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      05/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool isDPoint3dNear_NoAsserts (DPoint3dCR left, DPoint3dCR right, double const Epsilon);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      05/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool isDPoint3dNear (DPoint3dCR left, DPoint3dCR right, double const Epsilon);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AlexHowe        07/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool isDVec3dNear (DVec3dCR left, DVec3dCR right, double const Epsilon);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      06/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool isDPoint3dArrayNear (DPoint3dCP left, DPoint3dCP right, double const epsilon, size_t const numPoints);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      05/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool isQuaternionNear (double const* left, double const* right, double const Epsilon);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AlexHowe        08/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool isDoubleArrayNear (double const* left, double const* right, size_t arraySize, double const Epsilon);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void CopyPoints (DPoint3dP out, DPoint3dCP in, size_t const numPoints);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void TranslatePointArray (DPoint3dP inout, DVec3d trans, size_t const numPoints);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds       05/09
+---------------+---------------+---------------+---------------+---------------+------*/
int CalculateElementSize(int size);

struct GeomTests
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static double ComputeDistancePrecision (DPoint3dCR p1, DPoint3dCR p2)
    {
    DRange3d r = DRange3d::From (p1, p2);
    return r.LargestCoordinate() * 1.0e-14;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsPointOnPlane (DPoint3dCR pt, DPlane3dCR dPlane3d)
    {
    return dPlane3d.Evaluate (pt) <= ComputeDistancePrecision (pt, dPlane3d.origin);
    }
};

END_DGNDB_UNIT_TESTS_NAMESPACE