/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/CGSerializationTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include <GeomSerialization/GeomSerializationApi.h>

using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct CGSerializationTests : ::testing::Test
    {
    void RoundTripCurvePrimitiveArc (DEllipse3d ellipseData, bool isCircular)
        {
        ICurvePrimitivePtr originalArc = ICurvePrimitive::CreateArc (ellipseData);
        ASSERT_EQ(isCircular, originalArc->GetArcCP()->IsCircular());

        IGeometryPtr g1 = IGeometry::Create (originalArc);

        Utf8String cgBeXml;
        BeXmlCGWriter::Write(cgBeXml, *g1);
        //    wprintf(L"%ls", cgBeXml.c_str());

        bvector<IGeometryPtr> geoms;
        bmap<OrderedIGeometryPtr, BeExtendedData> extendedData;
        ASSERT_TRUE(BeXmlCGStreamReader::TryParse(cgBeXml.c_str(), geoms, extendedData, 0)) << "Failed to deserialize string: " << cgBeXml.c_str();

        //ICurvePrimitivePtr result;
        //ASSERT_TRUE(BeXmlCGParser::TryParse(cgBeXml.c_str(), result));
        //ASSERT_EQ(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc, result->GetCurvePrimitiveType());
        //DEllipse3dCP arc = result->GetArcCP();
        //ASSERT_TRUE(NULL != arc);
        //ASSERT_EQ(isCircular, arc->IsCircular());
        //ASSERT_EQ(ellipseData.center.x, arc->center.x) << L"arc->center.x not as expected";
        //ASSERT_EQ(ellipseData.center.y, arc->center.y) << L"arc->center.y not as expected";
        //ASSERT_EQ(ellipseData.center.z, arc->center.z) << L"arc->center.z not as expected";

        //ASSERT_EQ(ellipseData.vector0.x, arc->vector0.x) << L"arc->vector0.x not as expected";
        //ASSERT_EQ(ellipseData.vector0.y, arc->vector0.y) << L"arc->vector0.y not as expected";
        //ASSERT_EQ(ellipseData.vector0.z, arc->vector0.z) << L"arc->vector0.z not as expected";

        }

    void RoundTripDisk (DEllipse3d ellipseData, bool isCircular)
        {
        CurveVectorPtr originalDisk = CurveVector::CreateDisk (ellipseData);
        ASSERT_EQ(isCircular, ellipseData.IsCircular());
        IGeometryPtr g1 = IGeometry::Create (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*originalDisk).get());

        Utf8String cgBeXml;
        BeXmlCGWriter::Write(cgBeXml, *g1);
        //    wprintf(L"%ls", cgBeXml.c_str());

        bvector<IGeometryPtr> geoms;
        bmap<OrderedIGeometryPtr, BeExtendedData> extendedData;
        ASSERT_TRUE(BeXmlCGStreamReader::TryParse(cgBeXml.c_str(), geoms, extendedData, 0)) << "Failed to deserialize string: " << cgBeXml.c_str();

        //ICurvePrimitivePtr result;
        //ASSERT_TRUE(BeXmlCGParser::TryParse(cgBeXml.c_str(), result));
        //ASSERT_EQ(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector, result->GetCurvePrimitiveType());
        //CurveVectorCR curveVector = *(result->GetChildCurveVectorCP());
        //DEllipse3d arc;
        //curveVector[0]->TryGetArc(arc);
        //ASSERT_EQ(isCircular, arc.IsCircular());
        //ASSERT_TRUE(arc.IsFullEllipse()) << L"disk should be a full ellipse";
        //ASSERT_EQ(ellipseData.center.x, arc.center.x) << L"arc->center.x not as expected";
        //ASSERT_EQ(ellipseData.center.y, arc.center.y) << L"arc->center.y not as expected";
        //ASSERT_EQ(ellipseData.center.z, arc.center.z) << L"arc->center.z not as expected";

        //ASSERT_EQ(ellipseData.vector0.x, arc.vector0.x) << L"arc->vector0.x not as expected";
        //ASSERT_EQ(ellipseData.vector0.y, arc.vector0.y) << L"arc->vector0.y not as expected";
        //ASSERT_EQ(ellipseData.vector0.z, arc.vector0.z) << L"arc->vector0.z not as expected";

        }
    //---------------------------------------------------------------------------------------
    // @bsimethod                                        Ahmed.Rizwan               09/14
    //---------------------------------------------------------------------------------------
    void CurveVectorRoundTripDisk (DEllipse3d ellipseData, bool isCircular)
        {
        CurveVectorPtr originalDisk = CurveVector::CreateDisk (ellipseData);
        ASSERT_EQ (isCircular, ellipseData.IsCircular ());
        IGeometryPtr g1 = IGeometry::Create (originalDisk);
        Utf8String cgBeXml;
        //Serialize the ellipseData
        BeXmlCGWriter::Write (cgBeXml, *g1);

        bvector<IGeometryPtr> geoms;
        bmap<OrderedIGeometryPtr, BeExtendedData> extendedData;
        ASSERT_TRUE(BeXmlCGStreamReader::TryParse(cgBeXml.c_str(), geoms, extendedData, 0)) << "Failed to deserialize string: " << cgBeXml.c_str();

        //ICurvePrimitivePtr result;
        ////Parse the data written
        //ASSERT_TRUE (BeXmlCGParser::TryParse (cgBeXml.c_str (), result));
        //ASSERT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector, result->GetCurvePrimitiveType ());
        //CurveVectorCR curveVector = *(result->GetChildCurveVectorCP ());
        //ASSERT_TRUE (originalDisk->IsSameStructureAndGeometry (curveVector, 0.0)) << L"DEllipse3d Structure and Geometry not as expected";
        }
    //---------------------------------------------------------------------------------------
    // @bsimethod                                        Ahmed.Rizwan               09/14
    //---------------------------------------------------------------------------------------
    void CurveVectorCompactRoundTripDisk (DEllipse3d ellipseData, bool isCircular, bool isCompact)
        {
        CurveVectorPtr originalDisk = CurveVector::CreateDisk (ellipseData);
        ASSERT_EQ (isCircular, ellipseData.IsCircular ());
        IGeometryPtr g1 = IGeometry::Create (originalDisk);
        Utf8String cgBeXml;
        //Serialize the ellipseData
        BeXmlCGWriter::Write (cgBeXml, *g1/*, isCompact*/);

        bvector<IGeometryPtr> geoms;
        bmap<OrderedIGeometryPtr, BeExtendedData> extendedData;
        ASSERT_TRUE(BeXmlCGStreamReader::TryParse(cgBeXml.c_str(), geoms, extendedData, 0)) << "Failed to deserialize string: " << cgBeXml.c_str();

        //ICurvePrimitivePtr result;
        ////Parse the data written
        //ASSERT_TRUE (BeXmlCGParser::TryParse (cgBeXml.c_str (), result));
        //ASSERT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector, result->GetCurvePrimitiveType ());
        //CurveVectorCR curveVector = *(result->GetChildCurveVectorCP ());
        //ASSERT_TRUE (originalDisk->IsSameStructureAndGeometry (curveVector, 0.0)) << L"DEllipse3d Structure and Geometry not as expected";
        }
    //---------------------------------------------------------------------------------------
    // @bsimethod                                        Ahmed.Rizwan               09/14
    //---------------------------------------------------------------------------------------
    void ISolidPrimitiveDgnBoxDetail (DgnBoxDetail dgnBoxDetailData)
        {
        ISolidPrimitivePtr originalDgnBoxPtr = ISolidPrimitive::CreateDgnBox (dgnBoxDetailData);
        IGeometryPtr g1 = IGeometry::Create (originalDgnBoxPtr);
        Utf8String cgBeXml;
        //Serialize
        BeXmlCGWriter::Write (cgBeXml, *g1);
        ISolidPrimitivePtr result;
        ASSERT_TRUE (BeXmlCGParser::TryParse (cgBeXml.c_str (), result));
        //Verify Type, Structure and Geometry of the DgnBox
        ASSERT_EQ (SolidPrimitiveType::SolidPrimitiveType_DgnBox, (result->GetSolidPrimitiveType ())) << L"Type mismatch";

        DgnBoxDetail dgnResult;
        result->TryGetDgnBoxDetail (dgnResult);

        ASSERT_TRUE (originalDgnBoxPtr->IsSameStructureAndGeometry (*ISolidPrimitive::CreateDgnBox (dgnResult).get (), 0.0)) << L"DgnBox Structure and Geometry not as expected";
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                        Ahmed.Rizwan               09/14
    //---------------------------------------------------------------------------------------
    void RoundTripDgnCone (ISolidPrimitivePtr cone)
    {
        Utf8String cgBeXml;
        IGeometryPtr g1 = IGeometry::Create (cone);
        BeXmlCGWriter::Write (cgBeXml, *g1);
        bvector<IGeometryPtr> geoms;
        bmap<OrderedIGeometryPtr, BeExtendedData> extendedData;
        ASSERT_TRUE(BeXmlCGStreamReader::TryParse(cgBeXml.c_str(), geoms, extendedData, 0)) << "Failed to deserialize string: " << cgBeXml.c_str();

        //ISolidPrimitivePtr result;
        //ASSERT_TRUE (BeXmlCGParser::TryParse (cgBeXml.c_str (), result));
        ////Print out the contents of the xml
        ////EXPECT_TRUE (false) << cgBeXml.c_str ();
        //ASSERT_EQ (SolidPrimitiveType::SolidPrimitiveType_DgnCone, (result->GetSolidPrimitiveType ())) << L"Type mismatch";
    }
    //---------------------------------------------------------------------------------------
    // @bsimethod                                        Ahmed.Rizwan               09/14
    //---------------------------------------------------------------------------------------
    void IGeometryRoundTripArc (DEllipse3d ellipseData, bool isCircular)
        {
        ICurvePrimitivePtr originalArc = ICurvePrimitive::CreateArc (ellipseData);
        ASSERT_EQ (isCircular, originalArc->GetArcCP ()->IsCircular ());
        IGeometryPtr iGeometryPtr = IGeometry::Create (originalArc);
        Utf8String cgBeXml;
        BeXmlCGWriter::Write (cgBeXml, *iGeometryPtr);
        bvector<IGeometryPtr> geoms;
        bmap<OrderedIGeometryPtr, BeExtendedData> extendedData;
        ASSERT_TRUE(BeXmlCGStreamReader::TryParse(cgBeXml.c_str(), geoms, extendedData, 0));

        //ASSERT_EQ (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc, geoms[0]->GetCurvePrimitiveType ());
        //ASSERT_TRUE (originalArc->IsSameStructureAndGeometry (*geoms[0].get (), 0.0)) << L"DEllipse3d Structure and Geometry not as expected";
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
//---------------------------------------------------------------------------------------
// Tests the IGeometryPtr Xml Writer method with Circular Ellipse
//
// @bsimethod                                        Ahmed.Rizwan               09/14
//---------------------------------------------------------------------------------------
TEST_F (CGSerializationTests, RoundTripIGeometryCircularArc)
    {
    DEllipse3d ellipseData = DEllipse3d::From (0.0, 0.0, 0.0,
                                               1.0, 0.0, 0.0,
                                               0.0, 1.0, 0.0,
                                               0.0, Angle::TwoPi ());
    IGeometryRoundTripArc (ellipseData, true);
    }

//---------------------------------------------------------------------------------------
// Tests the IGeometryPtr Xml Writer method with Non-Circular Ellipse
//
// @bsimethod                                        Ahmed.Rizwan               09/14
//---------------------------------------------------------------------------------------
TEST_F (CGSerializationTests, RoundTripIGeometryEllipticArc)
    {
    DEllipse3d ellipseData = DEllipse3d::From (1, 2, 3,
                                               0, 0, 2,
                                               0, 3, 0,
                                               0.0, Angle::TwoPi ());
    IGeometryRoundTripArc (ellipseData, false);
    }

//---------------------------------------------------------------------------------------
// Tests the CurveVector Xml Writer method with Circular Ellipse
//
// @bsimethod                                        Ahmed.Rizwan               09/14
//---------------------------------------------------------------------------------------
TEST_F (CGSerializationTests, RoundTripCurveVectorCompactCircularDisk)
    {
    DEllipse3d ellipseData = DEllipse3d::From (0.0, 0.0, 0.0,
                                               1.0, 0.0, 0.0,
                                               0.0, 1.0, 0.0,
                                               0.0, Angle::TwoPi ());
    CurveVectorRoundTripDisk (ellipseData, true);
    CurveVectorCompactRoundTripDisk (ellipseData, true, true);
    CurveVectorCompactRoundTripDisk (ellipseData, true, false);
    }

//---------------------------------------------------------------------------------------
// Tests the CurveVector Xml Writer method with Non Circular Ellipse
//
// @bsimethod                                        Ahmed.Rizwan               09/14
//---------------------------------------------------------------------------------------
TEST_F (CGSerializationTests, RoundTripCurveVectorCompactEllipticDisk)
    {
    DEllipse3d ellipseData = DEllipse3d::From (1, 2, 3,
                                               0, 0, 2,
                                               0, 3, 0,
                                               0.0, Angle::TwoPi ());
    CurveVectorRoundTripDisk (ellipseData, false);
    CurveVectorCompactRoundTripDisk (ellipseData, false, true);
    CurveVectorCompactRoundTripDisk (ellipseData, false, false);
    }
//---------------------------------------------------------------------------------------
// Tests the ISolidPrimitive Xml Writer method with DgnCone
//
// @bsimethod                                        Ahmed.Rizwan               09/14
//---------------------------------------------------------------------------------------
TEST_F (CGSerializationTests, RoundTripDgnCone)
    {
    DgnConeDetail coneDetail (DPoint3d::From (1, 2, 2), DPoint3d::From (3, 2, 1), 1, 2, true);
    coneDetail.m_vector0 = DVec3d::From (1, 0, 0);
    coneDetail.m_vector90 = DVec3d::From (0, 1, 1);
    ISolidPrimitivePtr cone = ISolidPrimitive::CreateDgnCone (coneDetail);
    RoundTripDgnCone (cone);
    }

END_BENTLEY_ECN_TEST_NAMESPACE
