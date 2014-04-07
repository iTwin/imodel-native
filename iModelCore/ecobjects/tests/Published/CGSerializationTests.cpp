/*--------------------------------------------------------------------------------------+
|
|     $Source: tests/Published/CGSerializationTests.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include <ECObjects/BeXmlCommonGeometry.h>

using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

TEST(CGSerializationTests, RoundTripCurvePrimitiveArc)
    {
    DEllipse3d ellipseData = DEllipse3d::From (1,2,3,
        0,0,2,
        0,3,0,
        0.0, Angle::TwoPi ());
    ICurvePrimitivePtr ellipse = ICurvePrimitive::CreateArc (ellipseData);
    CurveVectorPtr pathA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    pathA->push_back (ellipse);

    Utf8String cgBeXml;
    BeXmlCGWriter::Write(cgBeXml, *(ellipse.get()));
//    wprintf(L"%ls", cgBeXml.c_str());

    ICurvePrimitivePtr result;
    ASSERT_TRUE(BeXmlCGParser::TryParse(cgBeXml.c_str(), result));
    ASSERT_EQ(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc, result->GetCurvePrimitiveType());
    DEllipse3dCP arc = result->GetArcCP();
    ASSERT_TRUE(NULL != arc);
    ASSERT_EQ(1, arc->center.x) << L"arc->center.x not as expected";
    ASSERT_EQ(2, arc->center.y) << L"arc->center.y not as expected";
    ASSERT_EQ(3, arc->center.z) << L"arc->center.z not as expected";

    ASSERT_EQ(0, arc->vector0.x) << L"arc->vector0.x not as expected";
    ASSERT_EQ(0, arc->vector0.y) << L"arc->vector0.y not as expected";
    ASSERT_EQ(2, arc->vector0.z) << L"arc->vector0.z not as expected";

    }
END_BENTLEY_ECN_TEST_NAMESPACE
