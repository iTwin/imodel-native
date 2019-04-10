/*--------------------------------------------------------------------------------------+
|
|  $Source: CivilBaseGeometry/Tests/TestHeader.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <Geom/GeomApi.h>
#include <GeomSerialization/GeomSerializationApi.h>
#include <CivilBaseGeometry/CivilBaseGeometryApi.h>

USING_NAMESPACE_BENTLEY_CIVILGEOMETRY

//=======================================================================================
// Fixture class
//=======================================================================================
struct CivilBaseGeometryTestsFixture : ::testing::Test
{
protected:
    //! Called before running all tests
    static void SetUpTestCase() {}
    //! Called after running all tests
    static void TearDownTestCase() {}

    //! Called before each test
    void SetUp() {}
    //! Called after each test
    void TearDown() {}
};
typedef CivilBaseGeometryTestsFixture CivilBaseGeometryTests;

//---------------------------------------------------------------------------------------
//! Toleranced asserts
//---------------------------------------------------------------------------------------
#define EXPECT_EQ_DPOINT3D(expected, actual) EXPECT_TRUE(expected.AlmostEqual(actual));
#define EXPECT_EQ_DOUBLE(expected, actual) EXPECT_TRUE(DoubleOps::AlmostEqual(expected, actual));
