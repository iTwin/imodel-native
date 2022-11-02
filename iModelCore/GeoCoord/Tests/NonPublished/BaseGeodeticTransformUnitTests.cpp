//:>--------------------------------------------------------------------------------------+
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See LICENSE.md in the repository root for full copyright notice.
//:>+--------------------------------------------------------------------------------------
#include <Bentley/BeTest.h>
#include <Bentley/BeFileName.h>

#include <GeoCoord/BaseGeoCoord.h>
#include <GeoCoord/BaseGeoTiffKeysList.h>
#include <Bentley/Desktop/FileSystem.h>

#include "GeoCoordTestCommon.h"

using namespace ::testing;

using namespace GeoCoordinates;
//--------------------------------------------------------------------------------------
// @bsiclass
//--------------------------------------------------------------------------------------
class BaseGeodeticTransformUnitTests : public ::testing::Test
    {   
    public:
        virtual void SetUp() { GeoCoordTestCommon::Initialize(); };
        virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

        BaseGeodeticTransformUnitTests() {};
        ~BaseGeodeticTransformUnitTests() {};
    };

/*---------------------------------------------------------------------------------**//**
* Test for DatumConverter and GeodeticTransform 1
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGeodeticTransformUnitTests, GeodeticTransformTest1)
{
    GeoCoordinates::DatumCP wgs84 = GeoCoordinates::Datum::CreateDatum("WGS84");
    GeoCoordinates::DatumCP otherDatum = GeoCoordinates::Datum::CreateDatum("ETRF89");

    ASSERT_TRUE(wgs84 != NULL && wgs84->IsValid());
    ASSERT_TRUE(otherDatum != NULL && otherDatum->IsValid());

    GeoCoordinates::DatumConverterP theConverter = GeoCoordinates::DatumConverter::Create(*otherDatum, *wgs84, GeoCoordinates::vdcGeoid, GeoCoordinates::vdcGeoid);

    ASSERT_TRUE(theConverter != NULL);

    // Only one transform for this pair
    ASSERT_TRUE(theConverter->GetGeodeticTransformCount() == 1);

    GeoCoordinates::GeodeticTransformP theTransform = theConverter->GetGeodeticTransform(0);

    // This particular transformation should be a null transform
    ASSERT_TRUE(theTransform->IsNullTransform());

    // Should have been NON but we defined for historical reason as a null GeoCentric
    ASSERT_TRUE(theTransform->GetConvertMethodCode() == GeoCoordinates::GenConvertCode::GenConvertType_GEOCTR);

    // There must not be other transform
    ASSERT_TRUE(NULL == theConverter->GetGeodeticTransform(1));

    bool             deltaValid, rotationValid, scaleValid, translationValid, gridValid;
    theTransform->ParametersValid (deltaValid, rotationValid, scaleValid, translationValid, gridValid);

    ASSERT_TRUE(deltaValid);
    ASSERT_FALSE(rotationValid);
    ASSERT_FALSE(scaleValid);
    ASSERT_FALSE(translationValid);
    ASSERT_FALSE(gridValid);

    DPoint3d delta = {0,0,0};
    DPoint3d rotation;
    DPoint3d translation;

    ASSERT_TRUE(SUCCESS == theTransform->GetDelta(delta));

    ASSERT_NEAR(delta.x, 0.0, 0.0000001);
    ASSERT_NEAR(delta.y, 0.0, 0.0000001);
    ASSERT_NEAR(delta.z, 0.0, 0.0000001);

    ASSERT_TRUE(GEOCOORDERR_ParameterNotUsed == theTransform->GetRotation(rotation));
    ASSERT_TRUE(GEOCOORDERR_ParameterNotUsed == theTransform->GetTranslation(translation));
    ASSERT_TRUE(0.0 == theTransform->GetScale());

    theTransform->Destroy();
    theConverter->Destroy();

    wgs84->Destroy();
    otherDatum->Destroy();
}

/*---------------------------------------------------------------------------------**//**
* Test for DatumConverter and GeodeticTransform 2
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGeodeticTransformUnitTests, GeodeticTransformTest2)
{
    GeoCoordinates::DatumCP wgs84 = GeoCoordinates::Datum::CreateDatum("WGS84");
    GeoCoordinates::DatumCP otherDatum = GeoCoordinates::Datum::CreateDatum("ADINDAN");

    ASSERT_TRUE(wgs84 != NULL && wgs84->IsValid());
    ASSERT_TRUE(otherDatum != NULL && otherDatum->IsValid());

    GeoCoordinates::DatumConverterP theConverter = GeoCoordinates::DatumConverter::Create(*otherDatum, *wgs84, GeoCoordinates::vdcGeoid, GeoCoordinates::vdcGeoid);

    wgs84->Destroy();
    otherDatum->Destroy();

    ASSERT_TRUE(theConverter != NULL);

    // Only one transform for this pair
    ASSERT_TRUE(theConverter->GetGeodeticTransformCount() == 1);

    GeoCoordinates::GeodeticTransformP theTransform = theConverter->GetGeodeticTransform(0);

    // This particular transformation should NOT be a null transform
    ASSERT_TRUE(!theTransform->IsNullTransform());

    ASSERT_TRUE(theTransform->GetConvertMethodCode() == GeoCoordinates::GenConvertCode::GenConvertType_MREG);

    // Multiple regression etails cannot be extracted.

    // There must not be other transform
    ASSERT_TRUE(NULL == theConverter->GetGeodeticTransform(1));

    bool             deltaValid, rotationValid, scaleValid, translationValid, gridValid;
    theTransform->ParametersValid (deltaValid, rotationValid, scaleValid, translationValid, gridValid);

    ASSERT_FALSE(deltaValid);
    ASSERT_FALSE(rotationValid);
    ASSERT_FALSE(scaleValid);
    ASSERT_FALSE(translationValid);
    ASSERT_FALSE(gridValid);

    DPoint3d delta;
    DPoint3d rotation;
    DPoint3d translation;

    ASSERT_TRUE(GEOCOORDERR_ParameterNotUsed == theTransform->GetDelta(delta));
    ASSERT_TRUE(GEOCOORDERR_ParameterNotUsed == theTransform->GetRotation(rotation));
    ASSERT_TRUE(GEOCOORDERR_ParameterNotUsed == theTransform->GetTranslation(translation));
    ASSERT_TRUE(0.0 == theTransform->GetScale());

    theTransform->Destroy();
    theConverter->Destroy();
}

/*---------------------------------------------------------------------------------**//**
* Test for DatumConverter and GeodeticTransform 3
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGeodeticTransformUnitTests, GeodeticTransformTest3)
{
    GeoCoordinates::DatumCP wgs84 = GeoCoordinates::Datum::CreateDatum("WGS84");
    GeoCoordinates::DatumCP otherDatum = GeoCoordinates::Datum::CreateDatum("AFGOOYE");

    ASSERT_TRUE(wgs84 != NULL && wgs84->IsValid());
    ASSERT_TRUE(otherDatum != NULL && otherDatum->IsValid());

    GeoCoordinates::DatumConverterP theConverter = GeoCoordinates::DatumConverter::Create(*otherDatum, *wgs84, GeoCoordinates::vdcGeoid, GeoCoordinates::vdcGeoid);

    ASSERT_TRUE(theConverter != NULL);

    // Only one transform for this pair
    ASSERT_TRUE(theConverter->GetGeodeticTransformCount() == 1);

    GeoCoordinates::GeodeticTransformP theTransform = theConverter->GetGeodeticTransform(0);

    // This particular transformation should NOT be a null transform
    ASSERT_TRUE(!theTransform->IsNullTransform());

    ASSERT_TRUE(theTransform->GetConvertMethodCode() == GeoCoordinates::GenConvertCode::GenConvertType_MOLO);

    // There must not be other transform
    ASSERT_TRUE(NULL == theConverter->GetGeodeticTransform(1));

    bool             deltaValid, rotationValid, scaleValid, translationValid, gridValid;
    theTransform->ParametersValid (deltaValid, rotationValid, scaleValid, translationValid, gridValid);

    ASSERT_TRUE(deltaValid);
    ASSERT_FALSE(rotationValid);
    ASSERT_FALSE(scaleValid);
    ASSERT_FALSE(translationValid);
    ASSERT_FALSE(gridValid);

    DPoint3d delta = {0,0,0};
    DPoint3d rotation;
    DPoint3d translation;

    ASSERT_TRUE(SUCCESS == theTransform->GetDelta(delta));

    ASSERT_NEAR(delta.x, -43.0, 0.0000001);
    ASSERT_NEAR(delta.y, -163.0, 0.0000001);
    ASSERT_NEAR(delta.z, 45.0, 0.0000001);

    ASSERT_TRUE(GEOCOORDERR_ParameterNotUsed == theTransform->GetRotation(rotation));
    ASSERT_TRUE(GEOCOORDERR_ParameterNotUsed == theTransform->GetTranslation(translation));
    ASSERT_TRUE(0.0 == theTransform->GetScale());

    theTransform->Destroy();
    theConverter->Destroy();
    
    wgs84->Destroy();
    otherDatum->Destroy();
}

/*---------------------------------------------------------------------------------**//**
* Test for DatumConverter and GeodeticTransform 4
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGeodeticTransformUnitTests, GeodeticTransformTest4)
{
    GeoCoordinates::DatumCP wgs84 = GeoCoordinates::Datum::CreateDatum("WGS84");
    GeoCoordinates::DatumCP otherDatum = GeoCoordinates::Datum::CreateDatum("Tokyo-Grid");

    ASSERT_TRUE(wgs84 != NULL && wgs84->IsValid());
    ASSERT_TRUE(otherDatum != NULL && otherDatum->IsValid());

    GeoCoordinates::DatumConverterP theConverter = GeoCoordinates::DatumConverter::Create(*otherDatum, *wgs84, GeoCoordinates::vdcGeoid, GeoCoordinates::vdcGeoid);

    ASSERT_TRUE(theConverter != NULL);

    // TODO In theory Tokyo should convert to JGD2000 then JGD2011 both through grid files but the
    // implementation will require some tweaking so some tests are remaining deactivated...
    ASSERT_TRUE(theConverter->GetGeodeticTransformCount() == 2);

    GeoCoordinates::GeodeticTransformP theTransform = theConverter->GetGeodeticTransform(0);

    // This particular transformation should NOT be a null transform
    ASSERT_TRUE(!theTransform->IsNullTransform());

    ASSERT_TRUE(theTransform->GetConvertMethodCode() == GeoCoordinates::GenConvertCode::GenConvertType_GFILE);

    bool             deltaValid, rotationValid, scaleValid, translationValid, gridValid;
    theTransform->ParametersValid (deltaValid, rotationValid, scaleValid, translationValid, gridValid);

    ASSERT_FALSE(deltaValid);
    ASSERT_FALSE(rotationValid);
    ASSERT_FALSE(scaleValid);
    ASSERT_FALSE(translationValid);
    ASSERT_TRUE(gridValid);

    DPoint3d delta;
    DPoint3d rotation;
    DPoint3d translation;

    ASSERT_TRUE(GEOCOORDERR_ParameterNotUsed == theTransform->GetDelta(delta));
    ASSERT_TRUE(GEOCOORDERR_ParameterNotUsed == theTransform->GetRotation(rotation));
    ASSERT_TRUE(GEOCOORDERR_ParameterNotUsed == theTransform->GetTranslation(translation));
    ASSERT_TRUE(0.0 == theTransform->GetScale());

    ASSERT_TRUE(theTransform->GetGridFileDefinitionCount() == 1);

    GridFileDefinition firstGridFile = theTransform->GetGridFileDefinition(0);

    ASSERT_STREQ(firstGridFile.GetFileName().c_str(), "./Japan/TKY2JGD.par");
    ASSERT_TRUE(firstGridFile.GetFormat() == GeoCoordinates::GridFileFormat::FORMAT_JAPAN);
    ASSERT_TRUE(firstGridFile.GetDirection() == GeoCoordinates::GridFileDirection::DIRECTION_DIRECT);

    // The other transform must NOT be null
    auto theTempTransform = theConverter->GetGeodeticTransform(1);
    
    ASSERT_TRUE(NULL != theTempTransform);
    theTempTransform->Destroy();

    theTransform->Destroy();

    theTransform = theConverter->GetGeodeticTransform(1);

    // This particular transformation should be a null transform (JGD2000 to WGS84)
    // TODO ... should be another grid to JGD2011
    ASSERT_TRUE(theTransform->IsNullTransform());

    theTransform->Destroy();
    theConverter->Destroy();

    wgs84->Destroy();
    otherDatum->Destroy();
}

/*---------------------------------------------------------------------------------**//**
* Test for DatumConverter and GeodeticTransform 5
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGeodeticTransformUnitTests, GeodeticTransformTest5)
{
    GeoCoordinates::DatumCP wgs84 = GeoCoordinates::Datum::CreateDatum("WGS84");
    GeoCoordinates::DatumCP otherDatum = GeoCoordinates::Datum::CreateDatum("NAD27");

    ASSERT_TRUE(wgs84 != NULL && wgs84->IsValid());
    ASSERT_TRUE(otherDatum != NULL && otherDatum->IsValid());

    GeoCoordinates::DatumConverterP theConverter = GeoCoordinates::DatumConverter::Create(*otherDatum, *wgs84, GeoCoordinates::vdcGeoid, GeoCoordinates::vdcGeoid);

    ASSERT_TRUE(theConverter != NULL);

    ASSERT_TRUE(theConverter->GetGeodeticTransformCount() == 2);

    GeoCoordinates::GeodeticTransformP theTransform = theConverter->GetGeodeticTransform(0);

    // This particular transformation should NOT be a null transform
    ASSERT_TRUE(!theTransform->IsNullTransform());

    ASSERT_TRUE(theTransform->GetConvertMethodCode() == GeoCoordinates::GenConvertCode::GenConvertType_GFILE);

    bool             deltaValid, rotationValid, scaleValid, translationValid, gridValid;
    theTransform->ParametersValid(deltaValid, rotationValid, scaleValid, translationValid, gridValid);

    ASSERT_FALSE(deltaValid);
    ASSERT_FALSE(rotationValid);
    ASSERT_FALSE(scaleValid);
    ASSERT_FALSE(translationValid);
    ASSERT_TRUE(gridValid);

    DPoint3d delta;
    DPoint3d rotation;
    DPoint3d translation;

    ASSERT_TRUE(GEOCOORDERR_ParameterNotUsed == theTransform->GetDelta(delta));
    ASSERT_TRUE(GEOCOORDERR_ParameterNotUsed == theTransform->GetRotation(rotation));
    ASSERT_TRUE(GEOCOORDERR_ParameterNotUsed == theTransform->GetTranslation(translation));
    ASSERT_TRUE(0.0 == theTransform->GetScale());

    ASSERT_TRUE(theTransform->GetGridFileDefinitionCount() == 7);

    GridFileDefinition firstGridFile = theTransform->GetGridFileDefinition(0);

    ASSERT_STREQ(firstGridFile.GetFileName().c_str(), "./Usa/Nadcon/conus.l?s");
    ASSERT_TRUE(firstGridFile.GetFormat() == GeoCoordinates::GridFileFormat::FORMAT_NADCON);
    ASSERT_TRUE(firstGridFile.GetDirection() == GeoCoordinates::GridFileDirection::DIRECTION_DIRECT);

    GridFileDefinition otherGridFile = theTransform->GetGridFileDefinition(1);

    ASSERT_STREQ(otherGridFile.GetFileName().c_str(), "./Usa/Nadcon/alaska.l?s");
    ASSERT_TRUE(otherGridFile.GetFormat() == GeoCoordinates::GridFileFormat::FORMAT_NADCON);
    ASSERT_TRUE(otherGridFile.GetDirection() == GeoCoordinates::GridFileDirection::DIRECTION_DIRECT);

    otherGridFile = theTransform->GetGridFileDefinition(2);

    ASSERT_STREQ(otherGridFile.GetFileName().c_str(), "./Usa/Nadcon/prvi.l?s");
    ASSERT_TRUE(otherGridFile.GetFormat() == GeoCoordinates::GridFileFormat::FORMAT_NADCON);
    ASSERT_TRUE(otherGridFile.GetDirection() == GeoCoordinates::GridFileDirection::DIRECTION_DIRECT);

    otherGridFile = theTransform->GetGridFileDefinition(3);

    ASSERT_STREQ(otherGridFile.GetFileName().c_str(), "./Usa/Nadcon/hawaii.l?s");
    ASSERT_TRUE(otherGridFile.GetFormat() == GeoCoordinates::GridFileFormat::FORMAT_NADCON);
    ASSERT_TRUE(otherGridFile.GetDirection() == GeoCoordinates::GridFileDirection::DIRECTION_DIRECT);

    otherGridFile = theTransform->GetGridFileDefinition(4);

    ASSERT_STREQ(otherGridFile.GetFileName().c_str(), "./Usa/Nadcon/stgeorge.l?s");
    ASSERT_TRUE(otherGridFile.GetFormat() == GeoCoordinates::GridFileFormat::FORMAT_NADCON);
    ASSERT_TRUE(otherGridFile.GetDirection() == GeoCoordinates::GridFileDirection::DIRECTION_DIRECT);

    otherGridFile = theTransform->GetGridFileDefinition(5);

    ASSERT_STREQ(otherGridFile.GetFileName().c_str(), "./Usa/Nadcon/stlrnc.l?s");
    ASSERT_TRUE(otherGridFile.GetFormat() == GeoCoordinates::GridFileFormat::FORMAT_NADCON);
    ASSERT_TRUE(otherGridFile.GetDirection() == GeoCoordinates::GridFileDirection::DIRECTION_DIRECT);

    otherGridFile = theTransform->GetGridFileDefinition(6);

    ASSERT_STREQ(otherGridFile.GetFileName().c_str(), "./Usa/Nadcon/stpaul.l?s");
    ASSERT_TRUE(otherGridFile.GetFormat() == GeoCoordinates::GridFileFormat::FORMAT_NADCON);
    ASSERT_TRUE(otherGridFile.GetDirection() == GeoCoordinates::GridFileDirection::DIRECTION_DIRECT);

    theTransform->Destroy();

    theConverter->Destroy();
    
    wgs84->Destroy();
    otherDatum->Destroy();
}

/*---------------------------------------------------------------------------------**//**
* Test for GeodeticTransform Reversal
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGeodeticTransformUnitTests, GeodeticTransformReversalTest)
{
    Utf8String geodeticTransform1 = 
"          {"
"            \"method\": \"GridFiles\","
"            \"gridFile\": {"
"              \"files\": ["
"                {"
"                  \"fileName\": \"./Australia/GDA94_GDA2020_conformal_and_distortion.gsb\","
"                  \"format\": \"NTv2\","
"                  \"direction\": \"Direct\""
"                }"
"              ]"
"            }"
"          }";

    GeoCoordinates::GeodeticTransformP transform1 = GeoCoordinates::GeodeticTransform::CreateGeodeticTransform();
    
    Json::Value transformJson1;
    EXPECT_TRUE(Json::Reader::Parse(geodeticTransform1, transformJson1));

    Utf8String errorMessage;
    EXPECT_EQ(SUCCESS, transform1->FromJson(transformJson1, errorMessage)) << errorMessage.c_str();

    EXPECT_TRUE(SUCCESS == transform1->Reverse());

    EXPECT_TRUE(transform1->GetGridFileDefinitionCount() == 1);
    GeoCoordinates::GridFileDefinition gridFile1 = transform1->GetGridFileDefinition(0);

    EXPECT_TRUE(gridFile1.GetDirection() == GeoCoordinates::GridFileDirection::DIRECTION_INVERSE);

    transform1->Destroy();

    Utf8String geodeticTransform2 = 
"        {"
"          \"method\": \"PositionalVector\","
"          \"positionalVector\": {"
"            \"scalePPM\": 2.4985,"
"            \"delta\": {"
"              \"x\": -120.271,"
"              \"y\": -64.543,"
"              \"z\": 161.632"
"            },"
"            \"rotation\": {"
"              \"x\": 0.2175,"
"              \"y\": -0.0672,"
"              \"z\": -0.1291"
"            }"
"          }"
"        }";

    GeoCoordinates::GeodeticTransformP transform2 = GeoCoordinates::GeodeticTransform::CreateGeodeticTransform();
    
    Json::Value transformJson2;
    EXPECT_TRUE(Json::Reader::Parse(geodeticTransform2, transformJson2));

    EXPECT_EQ(SUCCESS, transform2->FromJson(transformJson2, errorMessage)) << errorMessage.c_str();

    DPoint3d delta = {0,0,0};
    DPoint3d rotation = {0,0,0};
    double scalePPM;

    // Extract values
    EXPECT_TRUE(transform2->GetDelta(delta) == SUCCESS);
    EXPECT_TRUE(transform2->GetRotation(rotation) == SUCCESS);
    scalePPM = transform2->GetScale();

    EXPECT_TRUE(SUCCESS == transform2->Reverse());

    DPoint3d deltaRev = {0,0,0};
    DPoint3d rotationRev = {0,0,0};
    double scalePPMRev;

    EXPECT_TRUE(transform2->GetDelta(deltaRev) == SUCCESS);
    EXPECT_TRUE(transform2->GetRotation(rotationRev) == SUCCESS);
    scalePPMRev = transform2->GetScale();

    // These values taken from the Excel sheet to compute reversal
    EXPECT_NEAR(deltaRev.x, 120.27060644788900, 0.0000001);
    EXPECT_NEAR(deltaRev.y, 64.54274358076250, 0.0000001);
    EXPECT_NEAR(deltaRev.z, -161.63170340546400, 0.0000001);
    
    // Note that JSON convention is PositionalVector while GeoCoord is Coordinate Frame
    EXPECT_NEAR(rotationRev.x, 0.21749995793986, 0.0000001);
    EXPECT_NEAR(rotationRev.y, -0.06720013613200, 0.0000001);
    EXPECT_NEAR(rotationRev.z, -0.12909992913957, 0.0000001);

    EXPECT_NEAR(scalePPMRev, -2.49849375755229, 0.0000001);

    // Reverse back and check values are same as original
    EXPECT_TRUE(SUCCESS == transform2->Reverse());

    DPoint3d deltaBack = {0,0,0};
    DPoint3d rotationBack = {0,0,0};
    double scalePPMBack;

    EXPECT_TRUE(transform2->GetDelta(deltaBack) == SUCCESS);
    EXPECT_TRUE(transform2->GetRotation(rotationBack) == SUCCESS);
    scalePPMBack = transform2->GetScale();

    EXPECT_NEAR(delta.x,    deltaBack.x, 0.0000001);
    EXPECT_NEAR(delta.y,    deltaBack.y, 0.0000001);
    EXPECT_NEAR(delta.z,    deltaBack.z, 0.0000001);
    
    EXPECT_NEAR(rotation.x, rotationBack.x, 0.0000001);
    EXPECT_NEAR(rotation.y, rotationBack.y, 0.0000001);
    EXPECT_NEAR(rotation.z, rotationBack.z, 0.0000001);

    EXPECT_NEAR(scalePPM,   scalePPMBack, 0.0000001);

    transform2->Destroy();

    // Another test with 7 parameters but with more chunky values. When rotation is small the reversal is almost negating signs only
    // This is the Heathrow T5 datum so a real case
    Utf8String geodeticTransform3 = 
"        {"
"          \"method\": \"PositionalVector\","
"          \"positionalVector\": {"
"            \"scalePPM\": -6.26386076385543,"
"            \"delta\": {"
"              \"x\": 358.39821218189800,"
"              \"y\": -213.70284487073100,"
"              \"z\": 495.31831976971600"
"            },"
"            \"rotation\": {"
"              \"x\": 668.80613932004700,"
"              \"y\": -4.72664217602752,"
"              \"z\": 719.67109718139600"
"            }"
"          }"
"        }";

    GeoCoordinates::GeodeticTransformP transform3 = GeoCoordinates::GeodeticTransform::CreateGeodeticTransform();
    
    Json::Value transformJson3;
    EXPECT_TRUE(Json::Reader::Parse(geodeticTransform3, transformJson3));

    EXPECT_EQ(SUCCESS, transform3->FromJson(transformJson3, errorMessage)) << errorMessage.c_str();

    // Extract values
    EXPECT_TRUE(transform3->GetDelta(delta) == SUCCESS);
    EXPECT_TRUE(transform3->GetRotation(rotation) == SUCCESS);
    scalePPM = transform3->GetScale();

    EXPECT_TRUE(SUCCESS == transform3->Reverse());

    EXPECT_TRUE(transform3->GetDelta(deltaRev) == SUCCESS);
    EXPECT_TRUE(transform3->GetRotation(rotationRev) == SUCCESS);
    scalePPMRev = transform3->GetScale();

    // These values taken from the Excel sheet to compute reversal
    EXPECT_NEAR(deltaRev.x, -357.664, 0.01);
    EXPECT_NEAR(deltaRev.y, 213.346200000000, 0.0000001);
    EXPECT_NEAR(deltaRev.z, -496.007600000000, 0.0000001);
    
    EXPECT_NEAR(rotationRev.x, 668.818560000000, 0.0000001);
    EXPECT_NEAR(rotationRev.y, -2.393090000000, 0.0000001);
    EXPECT_NEAR(rotationRev.z, 719.682640000000, 0.0000001);

    EXPECT_NEAR(scalePPMRev, 6.263900000000, 0.0000001);

    EXPECT_TRUE(SUCCESS == transform3->Reverse());

    EXPECT_TRUE(transform3->GetDelta(deltaBack) == SUCCESS);
    EXPECT_TRUE(transform3->GetRotation(rotationBack) == SUCCESS);
    scalePPMBack = transform3->GetScale();

    EXPECT_NEAR(delta.x,    deltaBack.x, 0.0000001);
    EXPECT_NEAR(delta.y,    deltaBack.y, 0.0000001);
    EXPECT_NEAR(delta.z,    deltaBack.z, 0.0000001);
    
    EXPECT_NEAR(rotation.x, rotationBack.x, 0.0000001);
    EXPECT_NEAR(rotation.y, rotationBack.y, 0.0000001);
    EXPECT_NEAR(rotation.z, rotationBack.z, 0.0000001);

    EXPECT_NEAR(scalePPM,   scalePPMBack, 0.0000001);

    transform3->Destroy();

    Utf8String geodeticTransform4 = 
"        {"
"          \"method\": \"Geocentric\","
"          \"geocentric\": {"
"            \"delta\": {"
"              \"x\": -115,"
"              \"y\": 118,"
"              \"z\": 426"
"            }"
"          }"
"        }";

    GeoCoordinates::GeodeticTransformP transform4 = GeoCoordinates::GeodeticTransform::CreateGeodeticTransform();
    
    Json::Value transformJson4;
    EXPECT_TRUE(Json::Reader::Parse(geodeticTransform4, transformJson4));

    EXPECT_EQ(SUCCESS, transform4->FromJson(transformJson4, errorMessage)) << errorMessage.c_str();

    // Extract values
    EXPECT_TRUE(transform4->GetDelta(delta) == SUCCESS);

    EXPECT_TRUE(SUCCESS == transform4->Reverse());

    EXPECT_TRUE(transform4->GetDelta(deltaRev) == SUCCESS);

    // These values taken from the Excel sheet to compute reversal
    EXPECT_NEAR(deltaRev.x, 115.0, 0.00000001);
    EXPECT_NEAR(deltaRev.y, -118.0, 0.00000001);
    EXPECT_NEAR(deltaRev.z, -426.0, 0.00000001);
    
    EXPECT_TRUE(SUCCESS == transform4->Reverse());

    EXPECT_TRUE(transform4->GetDelta(deltaBack) == SUCCESS);

    EXPECT_NEAR(delta.x,    deltaBack.x, 0.001);
    EXPECT_NEAR(delta.y,    deltaBack.y, 0.001);
    EXPECT_NEAR(delta.z,    deltaBack.z, 0.001);
    
    transform4->Destroy();
}




    