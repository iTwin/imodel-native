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

struct CGSerializationTests : ::testing::Test
    {
    void RoundTripCurvePrimitiveArc (DEllipse3d ellipseData, bool isCircular)
        {
        ICurvePrimitivePtr originalArc = ICurvePrimitive::CreateArc (ellipseData);
        ASSERT_EQ(isCircular, originalArc->GetArcCP()->IsCircular());

        Utf8String cgBeXml;
        BeXmlCGWriter::Write(cgBeXml, *(originalArc.get()));
        //    wprintf(L"%ls", cgBeXml.c_str());

        ICurvePrimitivePtr result;
        ASSERT_TRUE(BeXmlCGParser::TryParse(cgBeXml.c_str(), result));
        ASSERT_EQ(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc, result->GetCurvePrimitiveType());
        DEllipse3dCP arc = result->GetArcCP();
        ASSERT_TRUE(NULL != arc);
        ASSERT_EQ(isCircular, arc->IsCircular());
        ASSERT_EQ(ellipseData.center.x, arc->center.x) << L"arc->center.x not as expected";
        ASSERT_EQ(ellipseData.center.y, arc->center.y) << L"arc->center.y not as expected";
        ASSERT_EQ(ellipseData.center.z, arc->center.z) << L"arc->center.z not as expected";

        ASSERT_EQ(ellipseData.vector0.x, arc->vector0.x) << L"arc->vector0.x not as expected";
        ASSERT_EQ(ellipseData.vector0.y, arc->vector0.y) << L"arc->vector0.y not as expected";
        ASSERT_EQ(ellipseData.vector0.z, arc->vector0.z) << L"arc->vector0.z not as expected";

        }

    void RoundTripDisk (DEllipse3d ellipseData, bool isCircular)
        {
        CurveVectorPtr originalDisk = CurveVector::CreateDisk (ellipseData);
        ASSERT_EQ(isCircular, ellipseData.IsCircular());

        Utf8String cgBeXml;
        BeXmlCGWriter::Write(cgBeXml, *(ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*originalDisk).get()));
        //    wprintf(L"%ls", cgBeXml.c_str());

        ICurvePrimitivePtr result;
        ASSERT_TRUE(BeXmlCGParser::TryParse(cgBeXml.c_str(), result));
        ASSERT_EQ(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector, result->GetCurvePrimitiveType());
        CurveVectorCR curveVector = *(result->GetChildCurveVectorCP());
        DEllipse3d arc;
        curveVector[0]->TryGetArc(arc);
        ASSERT_EQ(isCircular, arc.IsCircular());
        ASSERT_TRUE(arc.IsFullEllipse()) << L"disk should be a full ellipse";
        ASSERT_EQ(ellipseData.center.x, arc.center.x) << L"arc->center.x not as expected";
        ASSERT_EQ(ellipseData.center.y, arc.center.y) << L"arc->center.y not as expected";
        ASSERT_EQ(ellipseData.center.z, arc.center.z) << L"arc->center.z not as expected";

        ASSERT_EQ(ellipseData.vector0.x, arc.vector0.x) << L"arc->vector0.x not as expected";
        ASSERT_EQ(ellipseData.vector0.y, arc.vector0.y) << L"arc->vector0.y not as expected";
        ASSERT_EQ(ellipseData.vector0.z, arc.vector0.z) << L"arc->vector0.z not as expected";

        }
    };

TEST_F(CGSerializationTests, RoundTripEllipticArc)
    {
    DEllipse3d ellipseData = DEllipse3d::From (1,2,3,
        0,0,2,
        0,3,0,
        0.0, Angle::TwoPi ());
    RoundTripCurvePrimitiveArc(ellipseData, false);
    }

TEST_F(CGSerializationTests, RoundTripCircularArc)
    {
    DEllipse3d ellipseData = DEllipse3d::From (0.0, 0.0, 0.0, 
        1.0, 0.0, 0.0, 
        0.0, 1.0, 0.0, 
        0.0, Angle::TwoPi ());

    RoundTripCurvePrimitiveArc(ellipseData, true);
    }

TEST_F(CGSerializationTests, RoundTripCircularDisk)
    {
    DEllipse3d ellipseData = DEllipse3d::From (0.0, 0.0, 0.0, 
        1.0, 0.0, 0.0, 
        0.0, 1.0, 0.0, 
        0.0, Angle::TwoPi ());

    RoundTripDisk(ellipseData, true);

    }
END_BENTLEY_ECN_TEST_NAMESPACE
