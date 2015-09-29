/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/BackDoor/DgnElementHelpers.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTest.h>
#include "PublicAPI/BackDoor/DgnProject/DgnElementHelpers.h"

USING_NAMESPACE_BENTLEY_DGN

BEGIN_DGNDB_UNIT_TESTS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      05/09
+---------------+---------------+---------------+---------------+---------------+------*/
void GenerateSpiralYAxis(DPoint3dP points, size_t sz)
    {
    double rads = 0.0;
    double incr = PI / 180.0;
    double scale = 1000.0;
    
    size_t i;
    for (i = 0; i < sz; i++)
        {
        rads += incr;
        points[i] = DPoint3d();
        points[i].x = scale * cos (rads);
        points[i].y = scale * rads;
        points[i].z = scale * sin (rads);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds      05/09
+---------------+---------------+---------------+---------------+---------------+------*/
void GeneratePoints(DPoint3dP points, size_t const sz)
    {
    double const SCALE = 1000.0;
    
    size_t i;
    for (i = 0; i < sz; i++)
        {
        points[i].x= SCALE * i; 
        points[i].y= SCALE * i; 
        points[i].z= SCALE * i; 
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AlexHowe        08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void GenerateSin3d (DPoint3dP points, size_t const numpoints, double const scale)
    {
    double x;
    for (size_t i = 0;i < numpoints;i++)
        {
        x = i * PI * 0.5;
        points[i].x = scale * x;
        points[i].y = scale * sin (x) * x;
        points[i].z = points[i].y;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AlexHowe        08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void GenerateCircle (DPoint3dP points, size_t const numpoints, double const radius)
    {
    double x;
    for (size_t i = 0;i < numpoints;i++)
        {
        x = ((double)i/(double)numpoints) * (2*PI);
        points[i].x = sin (x) * radius;
        points[i].y = cos (x) * radius;
        points[i].z = 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      05/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool isDPoint3dNear_NoAsserts (DPoint3dCR left, DPoint3dCR right, double const Epsilon)
    {
    return (fabs (left.x - right.x) < Epsilon &&
            fabs (left.y - right.y) < Epsilon &&
            fabs (left.z - right.z) < Epsilon);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      05/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool isDPoint3dNear (DPoint3dCR left, DPoint3dCR right, double const Epsilon)
    {
    EXPECT_NEAR (left.x, right.x, Epsilon);
    EXPECT_NEAR (left.y, right.y, Epsilon);
    EXPECT_NEAR (left.z, right.z, Epsilon);

    return (fabs (left.x - right.x) < Epsilon &&
            fabs (left.y - right.y) < Epsilon &&
            fabs (left.z - right.z) < Epsilon);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AlexHowe        07/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool isDVec3dNear (DVec3dCR left, DVec3dCR right, double const Epsilon)
    {
    EXPECT_NEAR (left.x, right.x, Epsilon);
    EXPECT_NEAR (left.y, right.y, Epsilon);
    EXPECT_NEAR (left.z, right.z, Epsilon);

    return (fabs (left.x - right.x) < Epsilon &&
            fabs (left.y - right.y) < Epsilon &&
            fabs (left.z - right.z) < Epsilon);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      06/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool isDPoint3dArrayNear (DPoint3dCP left, DPoint3dCP right, double const epsilon, size_t const numPoints)
    {
    bool result = true;
    for (size_t i = 0; i < numPoints; i++)
        {
        if (!isDPoint3dNear (left[i], right[i], epsilon))
            result = false;
        }
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      05/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool isQuaternionNear (double const* left, double const* right, double const Epsilon)
    {
    EXPECT_NEAR (left[0], right[0], Epsilon);
    EXPECT_NEAR (left[1], right[1], Epsilon);
    EXPECT_NEAR (left[2], right[2], Epsilon);
    EXPECT_NEAR (left[3], right[3], Epsilon);

    return (fabs (left[0] - right[0]) < Epsilon &&
            fabs (left[1] - right[1]) < Epsilon &&
            fabs (left[2] - right[2]) < Epsilon && 
            fabs (left[3] - right[3]) < Epsilon);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AlexHowe        08/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool isDoubleArrayNear (double const* left, double const* right, size_t arraySize, double const Epsilon)
    {
    bool keepTestingFabs = true;
    for (size_t i = 0;i < arraySize;i++)
        {
        EXPECT_NEAR (left[i], right[i], Epsilon);
        if (keepTestingFabs && fabs (left[i] - right[i]) < Epsilon)
            {
            keepTestingFabs = false;
            }
        }
    return keepTestingFabs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void CopyPoints (DPoint3dP out, DPoint3dCP in, size_t const numPoints)
    {
    memcpy (out, in, sizeof (DPoint3d) * numPoints);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void TranslatePointArray (DPoint3dP inout, DVec3d trans, size_t const numPoints)
    {
    for (size_t i = 0; i < numPoints; i++)
        {
        inout[i].x += trans.x;
        inout[i].y += trans.y;
        inout[i].z += trans.z;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds       05/09
+---------------+---------------+---------------+---------------+---------------+------*/
int CalculateElementSize(int size)
    {
    return size/2;
    }

END_DGNDB_UNIT_TESTS_NAMESPACE