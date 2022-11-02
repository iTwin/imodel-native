//:>--------------------------------------------------------------------------------------+
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See LICENSE.md in the repository root for full copyright notice.
//:>+--------------------------------------------------------------------------------------


#include <Bentley/BeTest.h>
#include <Bentley/BeFileName.h>
#include <Bentley/Desktop/FileSystem.h>
#include <GeoCoord/BaseGeoCoord.h>
#include <GeoCoord/BaseGeoTiffKeysList.h>

#include "GeoCoordTestCommon.h"

using namespace ::testing;

using namespace GeoCoordinates;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
class BaseGCSUnitTests : public ::testing::Test
    {   
    public:
        virtual void SetUp() { GeoCoordTestCommon::Initialize(); };
        virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

        BaseGCSUnitTests() {};
        ~BaseGCSUnitTests() {};
    };

/*---------------------------------------------------------------------------------**//**
* Compare Web Mercator to plain Mercator (must be different by polar radius)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGCSUnitTests, WebMercatorDiffToMercator)
{
    GeoCoordinates::BaseGCSPtr webMercator = GeoCoordinates::BaseGCS::CreateGCS("EPSG:3857");

    ASSERT_TRUE(webMercator.IsValid() && webMercator->IsValid());

    GeoCoordinates::BaseGCSPtr plainMercator = GeoCoordinates::BaseGCS::CreateGCS("Sabotaged3857");

    ASSERT_TRUE(plainMercator.IsValid() && plainMercator->IsValid());

    ASSERT_FALSE(webMercator->IsEquivalent(*plainMercator));
    ASSERT_FALSE(plainMercator->IsEquivalent(*webMercator));
}

/*---------------------------------------------------------------------------------**//**
* Initial test for Json support
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGCSUnitTests, ToJsonFromJson1)
{
    GeoCoordinates::BaseGCSPtr ll84 = GeoCoordinates::BaseGCS::CreateGCS("LL84");

    ASSERT_TRUE(ll84.IsValid() && ll84->IsValid());

    Json::Value result ;

    ASSERT_TRUE(SUCCESS == ll84->ToHorizontalJson(result));

    GeoCoordinates::BaseGCSPtr resultGCS = GeoCoordinates::BaseGCS::CreateGCS();

    Utf8String errMessage;
    ASSERT_EQ(SUCCESS, resultGCS->FromHorizontalJson(result, errMessage)) << errMessage.c_str();

    ASSERT_TRUE(ll84->IsEquivalent(*resultGCS));
    
    EXPECT_TRUE(GeoCoordTestCommon::doubleSame(ll84->GetMinimumLatitude(), resultGCS->GetMinimumLatitude()));
    EXPECT_TRUE(GeoCoordTestCommon::doubleSame(ll84->GetMinimumLongitude(), resultGCS->GetMinimumLongitude()));
    EXPECT_TRUE(GeoCoordTestCommon::doubleSame(ll84->GetMaximumLatitude(), resultGCS->GetMaximumLatitude()));
    EXPECT_TRUE(GeoCoordTestCommon::doubleSame(ll84->GetMaximumLongitude(), resultGCS->GetMaximumLongitude()));

    EXPECT_TRUE(GeoCoordTestCommon::doubleSame(ll84->GetMinimumUsefulLatitude(), resultGCS->GetMinimumUsefulLatitude()));
    EXPECT_TRUE(GeoCoordTestCommon::doubleSame(ll84->GetMinimumUsefulLongitude(), resultGCS->GetMinimumUsefulLongitude()));
    EXPECT_TRUE(GeoCoordTestCommon::doubleSame(ll84->GetMaximumUsefulLatitude(), resultGCS->GetMaximumUsefulLatitude()));
    EXPECT_TRUE(GeoCoordTestCommon::doubleSame(ll84->GetMaximumUsefulLongitude(), resultGCS->GetMaximumUsefulLongitude()));

    EXPECT_STREQ(ll84->GetDescription(), resultGCS->GetDescription());
    Utf8String source1, source2;
    EXPECT_STREQ(ll84->GetSource(source1), resultGCS->GetSource(source2));
}

/*---------------------------------------------------------------------------------**//**
* Test for IsDeprecated
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGCSUnitTests, IsDeprecatedTest)
{
    // Datum deprecation test
    GeoCoordinates::DatumCP deprecatedDatum = GeoCoordinates::Datum::CreateDatum("CAMP_ASTR");

    ASSERT_TRUE(deprecatedDatum != nullptr && deprecatedDatum->IsValid());

    ASSERT_TRUE(deprecatedDatum->IsDeprecated());

    GeoCoordinates::DatumCP nonDeprecatedDatum = GeoCoordinates::Datum::CreateDatum("ETRF89");

    ASSERT_TRUE(nonDeprecatedDatum != nullptr && nonDeprecatedDatum->IsValid());

    ASSERT_TRUE(!nonDeprecatedDatum->IsDeprecated());

    deprecatedDatum->Destroy();
    nonDeprecatedDatum->Destroy();

    // BaseGCS deprecation test
    GeoCoordinates::BaseGCSPtr deprecatedGCS = GeoCoordinates::BaseGCS::CreateGCS("IGN-I");
    ASSERT_TRUE(deprecatedGCS.IsValid() && deprecatedGCS->IsValid());

    ASSERT_TRUE(deprecatedGCS->IsDeprecated());

    GeoCoordinates::BaseGCSPtr nonDeprecatedGCS = GeoCoordinates::BaseGCS::CreateGCS("LL84");

    ASSERT_TRUE(nonDeprecatedGCS.IsValid() && nonDeprecatedGCS->IsValid());

    ASSERT_TRUE(!nonDeprecatedGCS->IsDeprecated());
}

/*---------------------------------------------------------------------------------**//**
* Test for IsProjected
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGCSUnitTests, IsProjectedTest)
{
    // non-projected BaseGCS
    GeoCoordinates::BaseGCSPtr nonProjectedGCS = GeoCoordinates::BaseGCS::CreateGCS("LL84");
    ASSERT_TRUE(nonProjectedGCS.IsValid() && nonProjectedGCS->IsValid());

    ASSERT_TRUE(!nonProjectedGCS->IsProjected());

    // projected BaseGCS
    GeoCoordinates::BaseGCSPtr projectedGCS = GeoCoordinates::BaseGCS::CreateGCS("UTM84-1N");

    ASSERT_TRUE(projectedGCS.IsValid() && projectedGCS->IsValid());

    ASSERT_TRUE(projectedGCS->IsProjected());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGCSUnitTests, SpecificGetLinearTransformLinear_Test)
{
    GeoCoordinates::BaseGCSPtr firstGCS = GeoCoordinates::BaseGCS::CreateGCS("UTM84-13N");
    GeoCoordinates::BaseGCSPtr secondGCS = GeoCoordinates::BaseGCS::CreateGCS("UTM84-13N");

    ASSERT_TRUE(firstGCS.IsValid() && firstGCS->IsValid());
    ASSERT_TRUE(secondGCS.IsValid() && secondGCS->IsValid());

    Transform tfReproject;
    DRange3d extent;
    extent.low.x = 0.0;
    extent.low.y = 0.0;
    extent.low.z = 0.0;

    extent.high.x = 0.01;
    extent.high.y = 0.01;
    extent.high.z = 0.01;

    double maxError = 0.0;
    double meanError = 0.0;
    ASSERT_TRUE(REPROJECT_Success == secondGCS->GetLinearTransform(&tfReproject, extent, *firstGCS, &maxError, &meanError));

    ASSERT_TRUE(tfReproject.IsIdentity());
    ASSERT_TRUE(maxError == 0.0);  // Exact floating point compare is intentional
    ASSERT_TRUE(meanError == 0.0); // Exact floating point compare is intentional
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGCSUnitTests, SpecificGetLinearTransformInvalidExtent_Test1)
{
    GeoCoordinates::BaseGCSPtr firstGCS = GeoCoordinates::BaseGCS::CreateGCS("UTM84-13N");
    GeoCoordinates::BaseGCSPtr secondGCS = GeoCoordinates::BaseGCS::CreateGCS("UTM84-14N");

    ASSERT_TRUE(firstGCS.IsValid() && firstGCS->IsValid());
    ASSERT_TRUE(secondGCS.IsValid() && secondGCS->IsValid());

    Transform tfReproject;

    // Define NULL extent
    DRange3d extent;
    extent.low.x = 0.0;
    extent.low.y = 0.0;
    extent.low.z = 0.0;

    extent.high.x = 0.0;
    extent.high.y = 0.0;
    extent.high.z = 0.0;

    double maxError;
    double meanError;

    // Null extent will result in an error
    ASSERT_FALSE(REPROJECT_Success == secondGCS->GetLinearTransform(&tfReproject, extent, *firstGCS, &maxError, &meanError));
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGCSUnitTests, SpecificGetLinearTransformInvalidExtent_Test2)
{
    GeoCoordinates::BaseGCSPtr firstGCS = GeoCoordinates::BaseGCS::CreateGCS("UTM84-13N");
    GeoCoordinates::BaseGCSPtr secondGCS = GeoCoordinates::BaseGCS::CreateGCS("UTM84-14N");

    ASSERT_TRUE(firstGCS.IsValid() && firstGCS->IsValid());
    ASSERT_TRUE(secondGCS.IsValid() && secondGCS->IsValid());

    Transform tfReproject;

    // Define NULL extent
    DRange3d extent;
    extent.low.x = 0.0;
    extent.low.y = 0.0;
    extent.low.z = 0.0;

    extent.high.x = 0.001; // x dim extent too small
    extent.high.y = 0.01;
    extent.high.z = 0.01;

    double maxError;
    double meanError;

    // too small extent will result in an error
    ASSERT_FALSE(REPROJECT_Success == secondGCS->GetLinearTransform(&tfReproject, extent, *firstGCS, &maxError, &meanError));

    extent.high.x = 0.01;
    extent.high.y = 0.001; // y extent too small

    // too small extent will result in an error
    ASSERT_FALSE(REPROJECT_Success == secondGCS->GetLinearTransform(&tfReproject, extent, *firstGCS, &maxError, &meanError));

    extent.high.y = 0.01;
    extent.high.z = 0.001; // z extent too small

    // too small extent will result in an error
    ASSERT_FALSE(REPROJECT_Success == secondGCS->GetLinearTransform(&tfReproject, extent, *firstGCS, &maxError, &meanError));
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGCSUnitTests, SpecificGetLinearTransformECEFInvalidExtent_Test1)
{
    GeoCoordinates::BaseGCSPtr firstGCS = GeoCoordinates::BaseGCS::CreateGCS("UTM84-13N");

    ASSERT_TRUE(firstGCS.IsValid() && firstGCS->IsValid());

    Transform tfReproject;

    // Define too small extent
    DRange3d extent;
    extent.low.x = 6378137.000;
    extent.low.y = 0.0;
    extent.low.z = 0.0;

    extent.high.x = 6378137.001; // x dim extent too small
    extent.high.y = 0.01;
    extent.high.z = 0.01;

    double maxError;
    double meanError;

    // too small extent will result in an error
    ASSERT_FALSE(REPROJECT_Success == GeoCoordinates::BaseGCS::GetLinearTransformECEF(&tfReproject, extent, *firstGCS, &maxError, &meanError));

    extent.high.x = 6378137.01;
    extent.high.y = 0.001; // y extent too small

    // too small extent will result in an error
    ASSERT_FALSE(REPROJECT_Success == GeoCoordinates::BaseGCS::GetLinearTransformECEF(&tfReproject, extent, *firstGCS, &maxError, &meanError));

    extent.high.y = 0.01;
    extent.high.z = 0.001; // z extent too small

    // too small extent will result in an error
    ASSERT_FALSE(REPROJECT_Success == GeoCoordinates::BaseGCS::GetLinearTransformECEF(&tfReproject, extent, *firstGCS, &maxError, &meanError));
}

/*---------------------------------------------------------------------------------**//**
* Specific test (reported by Chris Wu)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGCSUnitTests, SpecificGetLinearTransformECEF_Test)
{
    GeoCoordinates::BaseGCSPtr firstGCS = GeoCoordinates::BaseGCS::CreateGCS("UTM84-30N");

    ASSERT_TRUE(firstGCS.IsValid() && firstGCS->IsValid());

    Transform tfReproject;
    DRange3d extent;
    extent.low.x = 6378137.000;
    extent.low.y = 0.0;
    extent.low.z = 0.0;

    extent.high.x = 6378139.000;
    extent.high.y = 2.01;
    extent.high.z = 2.01;

    double maxError = 0.0;
    double meanError = 0.0;
    ASSERT_TRUE(REPROJECT_Success == GeoCoordinates::BaseGCS::GetLinearTransformECEF(&tfReproject, extent, *firstGCS, &maxError, &meanError));

    ASSERT_TRUE(maxError < 0.01);
    ASSERT_TRUE(meanError < 0.01);

    DPoint3d ecefPtOrig, cartPtTarget1, cartPtTarget2;

    ecefPtOrig.x = 6378139.000;
    ecefPtOrig.y = 2.01;
    ecefPtOrig.z = 2.01;

    GeoCoordinates::BaseGCS::CartesianFromECEF(cartPtTarget1, ecefPtOrig, *firstGCS);
    tfReproject.Multiply(cartPtTarget2, ecefPtOrig);

    // TODO AR ... Have a valid tolerance.
    EXPECT_NEAR(cartPtTarget1.x, cartPtTarget2.x, 0.01);
    EXPECT_NEAR(cartPtTarget1.y, cartPtTarget2.y, 0.01);
    EXPECT_NEAR(cartPtTarget1.z, cartPtTarget2.z, 0.01);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGCSUnitTests, SpecificGetLinearTransformToECEF_Test)
{
    GeoCoordinates::BaseGCSPtr firstGCS = GeoCoordinates::BaseGCS::CreateGCS("EPSG:102738");

    ASSERT_TRUE(firstGCS.IsValid() && firstGCS->IsValid());

    Transform tfReproject;
    DRange3d extent;
    extent.low.x = 2431534.8286385699 ;
    extent.low.y = 7031137.3455734001;
    extent.low.z = 503.69678316163589;

    extent.high.x = 2431766.4053030075;
    extent.high.y = 7032461.7876020726;
    extent.high.z = 546.26824951171875 ;

    double maxError = 0.0;
    double meanError = 0.0;
    ASSERT_TRUE(REPROJECT_Success == firstGCS->GetLinearTransformToECEF(&tfReproject, extent, &maxError, &meanError));

    ASSERT_TRUE(maxError < 0.01);
    ASSERT_TRUE(meanError < 0.01);

    DPoint3d ecefPtTarget1 = {0,0,0}, ecefPtTarget2, cartPtSource;

    cartPtSource.x = 2431700.8286385699;
    cartPtSource.y = 7031267.3455734001;
    cartPtSource.z = 503.69678316163589;

    ASSERT_TRUE(REPROJECT_Success == firstGCS->ECEFFromCartesian(ecefPtTarget1, cartPtSource));
    tfReproject.Multiply(ecefPtTarget2, cartPtSource);

    // TODO AR ... Have a valid tolerance.
    EXPECT_NEAR(ecefPtTarget1.x, ecefPtTarget2.x, 0.01);
    EXPECT_NEAR(ecefPtTarget1.y, ecefPtTarget2.y, 0.01);
    EXPECT_NEAR(ecefPtTarget1.z, ecefPtTarget2.z, 0.01);

    // Now we test that the ToECEF and FromECEF are complementary

    DPoint3d ecefExtentLow = { 0,0,0 }, ecefExtentHigh = {0,0,0};
    DPoint3d cartExtentLow, cartExtentHigh;

    cartExtentLow.Init(extent.low.x, extent.low.y, extent.low.z);
    cartExtentHigh.Init(extent.high.x, extent.high.y, extent.high.z);
    ASSERT_TRUE(REPROJECT_Success == firstGCS->ECEFFromCartesian(ecefExtentLow, cartExtentLow));
    ASSERT_TRUE(REPROJECT_Success == firstGCS->ECEFFromCartesian(ecefExtentHigh, cartExtentHigh));

    Transform tfReprojectReverse;
    DRange3d extentReverse;
    extentReverse.low.x = ecefExtentLow.x;
    extentReverse.low.y = ecefExtentLow.y;
    extentReverse.low.z = ecefExtentLow.z;

    extentReverse.high.x = ecefExtentHigh.x;
    extentReverse.high.y = ecefExtentHigh.y;
    extentReverse.high.z = ecefExtentHigh.z;

    ASSERT_TRUE(REPROJECT_Success == GeoCoordinates::BaseGCS::GetLinearTransformECEF(&tfReprojectReverse, extentReverse, *firstGCS, &maxError, &meanError));

    ASSERT_TRUE(maxError < 0.01);
    ASSERT_TRUE(meanError < 0.01);

    // Convert through and back using transformations
    DPoint3d ecefIntermediary;
    DPoint3d returnedCartesian;

    tfReproject.Multiply(ecefIntermediary, cartPtSource);
    tfReprojectReverse.Multiply(returnedCartesian, ecefIntermediary);

    EXPECT_NEAR(cartPtSource.x, returnedCartesian.x, 0.01);
    EXPECT_NEAR(cartPtSource.y, returnedCartesian.y, 0.01);
    EXPECT_NEAR(cartPtSource.z, returnedCartesian.z, 0.01);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGCSUnitTests, SpecificGetLinearTransformExtentForLatLong_Test)
{
    GeoCoordinates::BaseGCSPtr firstGCS = GeoCoordinates::BaseGCS::CreateGCS("UTM84-30N");
    GeoCoordinates::BaseGCSPtr secondGCS = GeoCoordinates::BaseGCS::CreateGCS("EPSG:4326");

    ASSERT_TRUE(firstGCS.IsValid() && firstGCS->IsValid());
    ASSERT_TRUE(secondGCS.IsValid() && secondGCS->IsValid());

    Transform tfReproject;

    // Define Correct extent
    DRange3d extent;
    extent.low.x = 0.0;
    extent.low.y = 0.0;
    extent.low.z = 0.0;

    extent.high.x = 0.000001; // Invalid in linear units ... valid in degrees
    extent.high.y = 0.000001;
    extent.high.z = 0.01;     // z is always linear units

    double maxError;
    double meanError;

    // Will succeed because input is lat/long gcs.
    ASSERT_TRUE(REPROJECT_Success == secondGCS->GetLinearTransform(&tfReproject, extent, *firstGCS, &maxError, &meanError));

    extent.high.x = 0.00000001; // x extent too small

    // too small extent will result in an error
    ASSERT_FALSE(REPROJECT_Success == secondGCS->GetLinearTransform(&tfReproject, extent, *firstGCS, &maxError, &meanError));

    extent.high.x = 0.000001;
    extent.high.y = 0.00000001; // y extent too small

    // too small extent will result in an error
    ASSERT_FALSE(REPROJECT_Success == secondGCS->GetLinearTransform(&tfReproject, extent, *firstGCS, &maxError, &meanError));

    extent.high.y = 0.000001;
    extent.high.z = 0.001; // z extent too small

    // too small extent will result in an error
    ASSERT_FALSE(REPROJECT_Success == secondGCS->GetLinearTransform(&tfReproject, extent, *firstGCS, &maxError, &meanError));
}

/*---------------------------------------------------------------------------------**//**
* Specific test (reported by Chris Wu)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGCSUnitTests, SpecificCoordConversionEPSG_102012_Test)
{
    GeoCoordinates::BaseGCSPtr firstGCS = GeoCoordinates::BaseGCS::CreateGCS("EPSG:102012");
    GeoCoordinates::BaseGCSPtr secondGCS = GeoCoordinates::BaseGCS::CreateGCS("EPSG:4326");

    ASSERT_TRUE(firstGCS.IsValid() && firstGCS->IsValid());
    ASSERT_TRUE(secondGCS.IsValid() && secondGCS->IsValid());

    Transform tfReproject;
    DRange3d extent;
    extent.low.x = 0.0;
    extent.low.y = 0.0;
    extent.low.z = 0.0;

    extent.high.x = 0.01;
    extent.high.y = 0.01;
    extent.high.z = 0.01;

    double maxError;
    double meanError;
    ASSERT_TRUE(REPROJECT_Success == secondGCS->GetLinearTransform(&tfReproject, extent, *firstGCS, &maxError, &meanError));

    GeoPoint geoPtOrig, geoPtTarget;
    DPoint3d cartPtOrig, cartPtTarget;

    geoPtOrig.longitude = 0.0;
    geoPtOrig.latitude = 0.0;
    geoPtOrig.elevation = 0.0;

    secondGCS->CartesianFromLatLong(cartPtOrig, geoPtOrig);
    tfReproject.Multiply(cartPtTarget, cartPtOrig);
    firstGCS->LatLongFromCartesian(geoPtTarget, cartPtTarget);

    GeoPoint test_geoPtTarget = {0,0,0};
    ASSERT_TRUE(REPROJECT_Success == secondGCS->LatLongFromLatLong(test_geoPtTarget, geoPtOrig, *firstGCS));

    // TODO AR ... Have a valid tolerance.
    EXPECT_NEAR(test_geoPtTarget.longitude, geoPtTarget.longitude, 20.0);
    EXPECT_NEAR(test_geoPtTarget.latitude, geoPtTarget.latitude, 20.0);
    EXPECT_NEAR(test_geoPtTarget.elevation, geoPtTarget.elevation, 20.0);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGCSUnitTests, SpecificCoordConversionShellAppomattox)
{
    GeoCoordinates::BaseGCSPtr firstGCS = GeoCoordinates::BaseGCS::CreateGCS("Shell-Appomattox");
    firstGCS->SetVerticalDatumCode(vdcGeoid);
    firstGCS->SetReprojectElevation(true);

    double localParams[12];
    localParams[0] = 1.0;
    localParams[1] = 0.0;
    localParams[2] = 0.0; // Additional offset X
    localParams[3] = 0.0; // Additional offset y
    localParams[4] = 89.167; // Additional offset z
    localParams[5] = 0.0;
    localParams[6] = 0.0;
    localParams[7] = 0.0;
    localParams[8] = 0.0;
    localParams[9] = 0.0;
    localParams[10] = 0.0;
    localParams[11] = 0.0;

    LocalTransformerP localTransfo = LocalTransformer::CreateLocalTransformer(TRANSFORM_Helmert, localParams);

    if (localTransfo != nullptr)
        firstGCS->SetLocalTransformer(localTransfo);

    GeoCoordinates::BaseGCSPtr secondGCS = GeoCoordinates::BaseGCS::CreateGCS("NAD27.BLM-16N.ft");

    secondGCS->SetVerticalDatumCode(vdcGeoid);

    secondGCS->SetReprojectElevation(true);

    ASSERT_TRUE(firstGCS.IsValid() && firstGCS->IsValid());
    ASSERT_TRUE(secondGCS.IsValid() && secondGCS->IsValid());

    DPoint3d ptTarget = {0,0,0};
    DPoint3d pointInSecondGCS;

    pointInSecondGCS.x = 1340656.0;
    pointInSecondGCS.y = 10370441.0;
    pointInSecondGCS.z = 0.0;
    ASSERT_TRUE(REPROJECT_Success == secondGCS->CartesianFromCartesian(ptTarget, pointInSecondGCS, *firstGCS));

    EXPECT_NEAR(ptTarget.x, 0.0, 0.002);
    EXPECT_NEAR(ptTarget.y, 0.0, 0.002);
    EXPECT_NEAR(ptTarget.z, -89.167, 0.002);

    DPoint3d pointInFirstGCS;

    pointInFirstGCS.x = 0.0;
    pointInFirstGCS.y = 0.0;
    pointInFirstGCS.z = 0.0;

    ASSERT_TRUE(REPROJECT_Success == firstGCS->CartesianFromCartesian(ptTarget, pointInFirstGCS, *secondGCS));

    EXPECT_NEAR(ptTarget.x, 1340656.0, 0.002);
    EXPECT_NEAR(ptTarget.y, 10370441.0, 0.002);
    EXPECT_NEAR(ptTarget.z, 89.167, 0.002);

    pointInFirstGCS.x = 140.0;
    pointInFirstGCS.y = -140.0;
    pointInFirstGCS.z = 50.0;

    ASSERT_TRUE(REPROJECT_Success == firstGCS->CartesianFromCartesian(ptTarget, pointInFirstGCS, *secondGCS));

    EXPECT_NEAR(ptTarget.x, 1340793.494, 0.002);
    EXPECT_NEAR(ptTarget.y, 10370298.620, 0.002);
    EXPECT_NEAR(ptTarget.z, 139.167, 0.002);

    pointInFirstGCS.x = -140.0;
    pointInFirstGCS.y = 140.0;
    pointInFirstGCS.z = 150.0;

    ASSERT_TRUE(REPROJECT_Success == firstGCS->CartesianFromCartesian(ptTarget, pointInFirstGCS, *secondGCS));

    EXPECT_NEAR(ptTarget.x, 1340518.506, 0.002);
    EXPECT_NEAR(ptTarget.y, 10370583.380, 0.002);
    EXPECT_NEAR(ptTarget.z, 239.167, 0.002);
}

/*---------------------------------------------------------------------------------**//**
* Domain tests
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, SpecificElevationTesgtIsrael)
    {
    GeoCoordinates::BaseGCSPtr firstGCS;
    GeoCoordinates::BaseGCSPtr secondGCS;

    firstGCS = GeoCoordinates::BaseGCS::CreateGCS("UTM84-36N");
    secondGCS = GeoCoordinates::BaseGCS::CreateGCS();

    Utf8String wellKnownText = "PROJCS[\"Israel 1993 / Israeli T\",GEOGCS[\"EPSG:4141\",DATUM[\"EPSG:6141\",SPHEROID[\"EPSG:7019\",6378137.000,298.25722210],TOWGS84[-48.0000,55.0000,52.0000,0.000000,0.000000,0.000000,0.00000000]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"false_easting\",219529.584],PARAMETER[\"false_northing\",626907.390],PARAMETER[\"scale_factor\",1.000006700000],PARAMETER[\"central_meridian\",35.20451694444446],PARAMETER[\"latitude_of_origin\",31.73439361111112],UNIT[\"Meter\",1.00000000000000]]";
    secondGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, wellKnownText.c_str());

    ASSERT_TRUE(firstGCS.IsValid() && firstGCS->IsValid());
    ASSERT_TRUE(secondGCS.IsValid() && secondGCS->IsValid());

    ASSERT_TRUE(firstGCS->GetVerticalDatumCode() == GeoCoordinates::vdcFromDatum);
    ASSERT_TRUE(secondGCS->GetVerticalDatumCode() == GeoCoordinates::vdcFromDatum);

    DPoint3d resultPoint = {0,0,0};
    DPoint3d secondResultPoint = {0,0,0};
    DPoint3d inputPoint;

    inputPoint.x = 800000;
    inputPoint.y = 3000000;
    inputPoint.z = 10.24;

    firstGCS->SetReprojectElevation(true);
    secondGCS->SetReprojectElevation(true);

    ASSERT_TRUE(REPROJECT_Success == firstGCS->CartesianFromCartesian(resultPoint, inputPoint, *secondGCS));

    EXPECT_NEAR(resultPoint.x, 300862.38969781203, 0.001);
    EXPECT_NEAR(resultPoint.y, 112309.01716421195, 0.001);
    EXPECT_NEAR(resultPoint.z, 10.24, 0.001);

    ASSERT_TRUE(REPROJECT_Success == secondGCS->CartesianFromCartesian(secondResultPoint, resultPoint, *firstGCS));

    EXPECT_NEAR(secondResultPoint.x, 800000, 0.001);
    EXPECT_NEAR(secondResultPoint.y, 3000000, 0.001);
    EXPECT_NEAR(secondResultPoint.z, 10.24, 0.001);

    secondGCS->SetVerticalDatumCode(GeoCoordinates::vdcLocalEllipsoid);

    ASSERT_TRUE(REPROJECT_Success == firstGCS->CartesianFromCartesian(resultPoint, inputPoint, *secondGCS));

    EXPECT_NEAR(resultPoint.x, 300862.38969781203, 0.001);
    EXPECT_NEAR(resultPoint.y, 112309.01716421195, 0.001);
    EXPECT_NEAR(resultPoint.z, -7.6771248336881399, 0.001);

    ASSERT_TRUE(REPROJECT_Success == secondGCS->CartesianFromCartesian(secondResultPoint, resultPoint, *firstGCS));

    EXPECT_NEAR(secondResultPoint.x, 800000, 0.001);
    EXPECT_NEAR(secondResultPoint.y, 3000000, 0.001);
    EXPECT_NEAR(secondResultPoint.z, 10.24, 0.001);
    }

/*---------------------------------------------------------------------------------**//**
* Vertical Datum Name
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, VariousVerticalTest1)
    {
    GeoCoordinates::BaseGCSPtr theGCS;

    theGCS = GeoCoordinates::BaseGCS::CreateGCS("UTM84-36N");

    ASSERT_TRUE(theGCS.IsValid() && theGCS->IsValid());

    // Default vertical datum should be vdcFromDatum
    ASSERT_TRUE(vdcFromDatum == theGCS->GetVerticalDatumCode());
    ASSERT_TRUE(!theGCS->IsNAD27());
    ASSERT_TRUE(!theGCS->IsNAD83());
    ASSERT_STREQ("Ellipsoid", theGCS->GetVerticalDatumName());

    ASSERT_TRUE(SUCCESS == theGCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(vdcGeoid == theGCS->GetVerticalDatumCode());
    ASSERT_TRUE(!theGCS->IsNAD27());
    ASSERT_TRUE(!theGCS->IsNAD83());
    ASSERT_STREQ("Geoid", theGCS->GetVerticalDatumName());

    // This call should fail since GCS is not NAD27/83 based ... value must remain unchanged.
    ASSERT_TRUE(SUCCESS != theGCS->SetVerticalDatumCode(vdcNGVD29));
    ASSERT_TRUE(vdcGeoid == theGCS->GetVerticalDatumCode());
    ASSERT_TRUE(!theGCS->IsNAD27());
    ASSERT_TRUE(!theGCS->IsNAD83());
    ASSERT_STREQ("Geoid", theGCS->GetVerticalDatumName());

    // This call should fail since GCS is not NAD27/83 based ... value must remain unchanged.
    ASSERT_TRUE(SUCCESS != theGCS->SetVerticalDatumCode(vdcNAVD88));
    ASSERT_TRUE(vdcGeoid == theGCS->GetVerticalDatumCode());
    ASSERT_TRUE(!theGCS->IsNAD27());
    ASSERT_TRUE(!theGCS->IsNAD83());
    ASSERT_STREQ("Geoid", theGCS->GetVerticalDatumName());

    ASSERT_TRUE(SUCCESS == theGCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(vdcEllipsoid == theGCS->GetVerticalDatumCode());
    ASSERT_TRUE(!theGCS->IsNAD27());
    ASSERT_TRUE(!theGCS->IsNAD83());
    ASSERT_STREQ("Ellipsoid", theGCS->GetVerticalDatumName());

    GeoCoordinates::BaseGCSPtr theNAD83GCS;

    theNAD83GCS = GeoCoordinates::BaseGCS::CreateGCS("UTM83-15");

    ASSERT_TRUE(theNAD83GCS.IsValid() && theNAD83GCS->IsValid());

    // Default vertical datum should be vdcFromDatum
    ASSERT_TRUE(vdcFromDatum == theNAD83GCS->GetVerticalDatumCode());
    ASSERT_TRUE(!theNAD83GCS->IsNAD27());
    ASSERT_TRUE(theNAD83GCS->IsNAD83());
    ASSERT_STREQ("NAVD88", theNAD83GCS->GetVerticalDatumName()); // From datum is reinterpreted as NAVD88

    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(vdcGeoid == theNAD83GCS->GetVerticalDatumCode());
    ASSERT_STREQ("Geoid", theNAD83GCS->GetVerticalDatumName());

    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcNGVD29));
    ASSERT_TRUE(vdcNGVD29 == theNAD83GCS->GetVerticalDatumCode());
    ASSERT_STREQ("NGVD29", theNAD83GCS->GetVerticalDatumName());

    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcNAVD88));
    ASSERT_TRUE(vdcNAVD88 == theNAD83GCS->GetVerticalDatumCode());
    ASSERT_STREQ("NAVD88", theNAD83GCS->GetVerticalDatumName());

    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(vdcEllipsoid == theNAD83GCS->GetVerticalDatumCode());
    ASSERT_STREQ("Ellipsoid", theNAD83GCS->GetVerticalDatumName());

    GeoCoordinates::BaseGCSPtr theNAD27GCS;

    theNAD27GCS = GeoCoordinates::BaseGCS::CreateGCS("UTM27-15");

    ASSERT_TRUE(theNAD27GCS.IsValid() && theNAD27GCS->IsValid());

    // Default vertical datum should be vdcFromDatum
    ASSERT_TRUE(vdcFromDatum == theNAD27GCS->GetVerticalDatumCode());
    ASSERT_TRUE(theNAD27GCS->IsNAD27());
    ASSERT_TRUE(!theNAD27GCS->IsNAD83());
    ASSERT_STREQ("NGVD29", theNAD27GCS->GetVerticalDatumName()); // From datum is reinterpreted as NAVD88

    ASSERT_TRUE(SUCCESS == theNAD27GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(vdcGeoid == theNAD27GCS->GetVerticalDatumCode());
    ASSERT_STREQ("Geoid", theNAD27GCS->GetVerticalDatumName());

    ASSERT_TRUE(SUCCESS == theNAD27GCS->SetVerticalDatumCode(vdcNGVD29));
    ASSERT_TRUE(vdcNGVD29 == theNAD27GCS->GetVerticalDatumCode());
    ASSERT_STREQ("NGVD29", theNAD27GCS->GetVerticalDatumName());

    ASSERT_TRUE(SUCCESS == theNAD27GCS->SetVerticalDatumCode(vdcNAVD88));
    ASSERT_TRUE(vdcNAVD88 == theNAD27GCS->GetVerticalDatumCode());
    ASSERT_STREQ("NAVD88", theNAD27GCS->GetVerticalDatumName());

    ASSERT_TRUE(SUCCESS == theNAD27GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(vdcEllipsoid == theNAD27GCS->GetVerticalDatumCode());
    ASSERT_STREQ("Ellipsoid", theNAD27GCS->GetVerticalDatumName());
    }

/*---------------------------------------------------------------------------------**//**
* Vertical Datum Name
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, VariousVerticalTestWGS84_to_WGS84)
    {
    GeoCoordinates::BaseGCSPtr theWGS84GCS;
    GeoCoordinates::BaseGCSPtr theOtherWGS84GCS;

    theWGS84GCS = GeoCoordinates::BaseGCS::CreateGCS("UTM84-16N");
    ASSERT_TRUE(theWGS84GCS.IsValid() && theWGS84GCS->IsValid());

    // Default vertical datum should be vdcFromDatum and it should mean ellipsoid
    ASSERT_TRUE(vdcFromDatum == theWGS84GCS->GetVerticalDatumCode());
    ASSERT_STREQ("Ellipsoid", theWGS84GCS->GetVerticalDatumName());

    // Default is to reproject elevation
    ASSERT_TRUE(theWGS84GCS->GetReprojectElevation());

    theOtherWGS84GCS = GeoCoordinates::BaseGCS::CreateGCS("UTM84-15N");
    ASSERT_TRUE(theOtherWGS84GCS.IsValid() && theOtherWGS84GCS->IsValid());

    // Default vertical datum should be vdcFromDatum and it should mean ellipsoid
    ASSERT_TRUE(vdcFromDatum == theOtherWGS84GCS->GetVerticalDatumCode());
    ASSERT_STREQ("Ellipsoid", theOtherWGS84GCS->GetVerticalDatumName());

    // Default is to reproject elevation
    ASSERT_TRUE(theOtherWGS84GCS->GetReprojectElevation());

    // Start with a coordinate in the USA
    DPoint3d myInPoint;
    myInPoint.x = 600000;
    myInPoint.y = 3600000;
    myInPoint.z = 1.1;

    double GeoidSeparation = -29.519501732186900;

    DPoint3d myOutPoint = {0,0,0};

    // Convert Ellipsoid to Ellipsoid (using vdcFromDatum)
    ASSERT_TRUE(REPROJECT_Success == theWGS84GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherWGS84GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Change destination to vdcEllipsoid
    ASSERT_TRUE(SUCCESS == theOtherWGS84GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(REPROJECT_Success == theWGS84GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherWGS84GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Change destination to vdcGeoid
    ASSERT_TRUE(SUCCESS == theOtherWGS84GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(REPROJECT_Success == theWGS84GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherWGS84GCS));
    ASSERT_NEAR(GeoidSeparation, myInPoint.z - myOutPoint.z, 0.00001); // Elevation different by Geoid separation.

    // Change source to vdcEllipsoid and change back dest to vdcEllipsoid
    ASSERT_TRUE(SUCCESS == theOtherWGS84GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(SUCCESS == theWGS84GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(REPROJECT_Success == theWGS84GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherWGS84GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Change destination to vdcGeoid
    ASSERT_TRUE(SUCCESS == theOtherWGS84GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(REPROJECT_Success == theWGS84GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherWGS84GCS));
    ASSERT_NEAR(GeoidSeparation, myInPoint.z - myOutPoint.z, 0.00001); // Elevation different by Geoid separation.

    // Both are geoid.
    ASSERT_TRUE(SUCCESS == theWGS84GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(SUCCESS == theOtherWGS84GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(REPROJECT_Success == theWGS84GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherWGS84GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001); // Can there be a slight difference?
    }

/*---------------------------------------------------------------------------------**//**
* Vertical Datum Name
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, VariousVerticalTestNAD27_to_NAD27)
    {
    GeoCoordinates::BaseGCSPtr theNAD27GCS;
    GeoCoordinates::BaseGCSPtr theOtherNAD27GCS;

    theNAD27GCS = GeoCoordinates::BaseGCS::CreateGCS("UTM27-16");
    ASSERT_TRUE(theNAD27GCS.IsValid() && theNAD27GCS->IsValid());

    // Default vertical datum should be vdcFromDatum and it should mean NGVD29
    ASSERT_TRUE(vdcFromDatum == theNAD27GCS->GetVerticalDatumCode());
    ASSERT_STREQ("NGVD29", theNAD27GCS->GetVerticalDatumName());

    // Default is to reproject elevation
    ASSERT_TRUE(theNAD27GCS->GetReprojectElevation());

    theOtherNAD27GCS = GeoCoordinates::BaseGCS::CreateGCS("UTM27-15");
    ASSERT_TRUE(theOtherNAD27GCS.IsValid() && theOtherNAD27GCS->IsValid());

    // Default vertical datum should be vdcFromDatum and it should mean NGVD29
    ASSERT_TRUE(vdcFromDatum == theOtherNAD27GCS->GetVerticalDatumCode());
    ASSERT_STREQ("NGVD29", theOtherNAD27GCS->GetVerticalDatumName());

    // Default is to reproject elevation
    ASSERT_TRUE(theOtherNAD27GCS->GetReprojectElevation());

    // Start with a coordinate in the USA
    DPoint3d myInPoint;
    myInPoint.x = 600000;
    myInPoint.y = 3600000;
    myInPoint.z = 1.1;

    double NAVD88Offset = 0.024167327540054240;
    // double GeoidSeparation = -29.519501732186900;
    double GeoidSeparation = -29.523165519559370;
    double NAVD88AndGeoid = GeoidSeparation + NAVD88Offset;

    DPoint3d myOutPoint = {0,0,0};

    // Convert NGVD29 to NGVD29 (using vdcFromDatum)
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD27GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Change destination to vdcNGVD29 (same)
    ASSERT_TRUE(SUCCESS == theOtherNAD27GCS->SetVerticalDatumCode(vdcNGVD29));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD27GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Change destination to vdcEllipsoid
    ASSERT_TRUE(SUCCESS == theOtherNAD27GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD27GCS));
    ASSERT_NEAR(-NAVD88AndGeoid, myInPoint.z - myOutPoint.z, 0.001); // Elevation different by Geoid separation + NAVD88 offset.

    // Change destination to vdcGeoid
    ASSERT_TRUE(SUCCESS == theOtherNAD27GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD27GCS));
    ASSERT_NEAR(-NAVD88Offset, myInPoint.z - myOutPoint.z, 0.00001); // Elevation different by NAVD88 offset.

    // Change destination to vdcNAVD88
    ASSERT_TRUE(SUCCESS == theOtherNAD27GCS->SetVerticalDatumCode(vdcNAVD88));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD27GCS));
    ASSERT_NEAR(-NAVD88Offset, myInPoint.z - myOutPoint.z, 0.00001); // Elevation different by NAVD88 offset.

    // Set 2
    // Change source from vdcFromDatum to vdcNGVD29 (no change) and start same tests
    ASSERT_TRUE(SUCCESS == theNAD27GCS->SetVerticalDatumCode(vdcNGVD29));
    ASSERT_TRUE(SUCCESS == theOtherNAD27GCS->SetVerticalDatumCode(vdcFromDatum));

    // Convert NGVD29 to NGVD29 (using vdcFromDatum for dest)
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD27GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Change destination to vdcNGVD29 (same)
    ASSERT_TRUE(SUCCESS == theOtherNAD27GCS->SetVerticalDatumCode(vdcNGVD29));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD27GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Change destination to vdcEllipsoid
    ASSERT_TRUE(SUCCESS == theOtherNAD27GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD27GCS));
    ASSERT_NEAR(-NAVD88AndGeoid, myInPoint.z - myOutPoint.z, 0.001); // Elevation different by Geoid separation + NAVD88 offset.

    // Change destination to vdcGeoid
    ASSERT_TRUE(SUCCESS == theOtherNAD27GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD27GCS));
    ASSERT_NEAR(-NAVD88Offset, myInPoint.z - myOutPoint.z, 0.001); // Elevation different by NAVD88 offset.

    // Change destination to vdcNAVD88
    ASSERT_TRUE(SUCCESS == theOtherNAD27GCS->SetVerticalDatumCode(vdcNAVD88));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD27GCS));
    ASSERT_NEAR(-NAVD88Offset, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by NAVD88 offset.

    // Set 3
    // Change source to vdcNAVD88 and start same tests
    ASSERT_TRUE(SUCCESS == theNAD27GCS->SetVerticalDatumCode(vdcNAVD88));
    ASSERT_TRUE(SUCCESS == theOtherNAD27GCS->SetVerticalDatumCode(vdcFromDatum));

    // Convert NAVD88 to NGVD29 (using vdcFromDatum for dest)
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD27GCS));
    ASSERT_NEAR(NAVD88Offset, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by NAVD88 offset.

    // Change destination to vdcNGVD29
    ASSERT_TRUE(SUCCESS == theOtherNAD27GCS->SetVerticalDatumCode(vdcNGVD29));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD27GCS));
    ASSERT_NEAR(NAVD88Offset, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by NAVD88 offset.

    // Change destination to vdcEllipsoid
    ASSERT_TRUE(SUCCESS == theOtherNAD27GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD27GCS));
    ASSERT_NEAR(-GeoidSeparation, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by Geoid separation + NAVD88 offset.

    // Change destination to vdcGeoid
    ASSERT_TRUE(SUCCESS == theOtherNAD27GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD27GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Change destination to vdcNAVD88
    ASSERT_TRUE(SUCCESS == theOtherNAD27GCS->SetVerticalDatumCode(vdcNAVD88));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD27GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);


    // Set 4
    // Change source to vdcGeoid and start same tests
    ASSERT_TRUE(SUCCESS == theNAD27GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(SUCCESS == theOtherNAD27GCS->SetVerticalDatumCode(vdcFromDatum));

    // Convert Geoid to NGVD29 (using vdcFromDatum for dest)
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD27GCS));
    ASSERT_NEAR(NAVD88Offset, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by NAVD88 offset.

    // Change destination to vdcNGVD29
    ASSERT_TRUE(SUCCESS == theOtherNAD27GCS->SetVerticalDatumCode(vdcNGVD29));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD27GCS));
    ASSERT_NEAR(NAVD88Offset, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by NAVD88 offset.

    // Change destination to vdcEllipsoid
    ASSERT_TRUE(SUCCESS == theOtherNAD27GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD27GCS));
    ASSERT_NEAR(-GeoidSeparation, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by Geoid separation.

    // Change destination to vdcGeoid
    ASSERT_TRUE(SUCCESS == theOtherNAD27GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD27GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Change destination to vdcNAVD88
    ASSERT_TRUE(SUCCESS == theOtherNAD27GCS->SetVerticalDatumCode(vdcNAVD88));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD27GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Set 5
    // Change source to vdcEllipsoid and start same tests
    ASSERT_TRUE(SUCCESS == theNAD27GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(SUCCESS == theOtherNAD27GCS->SetVerticalDatumCode(vdcFromDatum));

    // Convert Geoid to NGVD29 (using vdcFromDatum for dest)
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD27GCS));
    ASSERT_NEAR(NAVD88AndGeoid, myInPoint.z - myOutPoint.z, 0.1); // Elevation different by NAVD88 offset + Geoid separation.

    // Change destination to vdcNGVD29
    ASSERT_TRUE(SUCCESS == theOtherNAD27GCS->SetVerticalDatumCode(vdcNGVD29));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD27GCS));
    ASSERT_NEAR(NAVD88AndGeoid, myInPoint.z - myOutPoint.z, 0.1); // Elevation different by NAVD88 offset + Geoid separation.

    // Change destination to vdcEllipsoid
    ASSERT_TRUE(SUCCESS == theOtherNAD27GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD27GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Change destination to vdcGeoid
    ASSERT_TRUE(SUCCESS == theOtherNAD27GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD27GCS));
    ASSERT_NEAR(GeoidSeparation, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by Geoid separation.

    // Change destination to vdcNAVD88
    ASSERT_TRUE(SUCCESS == theOtherNAD27GCS->SetVerticalDatumCode(vdcNAVD88));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD27GCS));
    ASSERT_NEAR(GeoidSeparation, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by Geoid separation.
    }

/*---------------------------------------------------------------------------------**//**
* Vertical Datum Name
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, VariousVerticalTestNAD83_to_NAD83)
    {
    GeoCoordinates::BaseGCSPtr theNAD83GCS;
    GeoCoordinates::BaseGCSPtr theOtherNAD83GCS;

    theNAD83GCS = GeoCoordinates::BaseGCS::CreateGCS("UTM83-16");
    ASSERT_TRUE(theNAD83GCS.IsValid() && theNAD83GCS->IsValid());

    // Default vertical datum should be vdcFromDatum and it should mean NAVD88
    ASSERT_TRUE(vdcFromDatum == theNAD83GCS->GetVerticalDatumCode());
    ASSERT_STREQ("NAVD88", theNAD83GCS->GetVerticalDatumName());

    // Default is to reproject elevation
    ASSERT_TRUE(theNAD83GCS->GetReprojectElevation());

    theOtherNAD83GCS = GeoCoordinates::BaseGCS::CreateGCS("UTM83-15");
    ASSERT_TRUE(theOtherNAD83GCS.IsValid() && theOtherNAD83GCS->IsValid());

    // Default vertical datum should be vdcFromDatum and it should mean NAVD88
    ASSERT_TRUE(vdcFromDatum == theOtherNAD83GCS->GetVerticalDatumCode());
    ASSERT_STREQ("NAVD88", theOtherNAD83GCS->GetVerticalDatumName());

    // Default is to reproject elevation
    ASSERT_TRUE(theOtherNAD83GCS->GetReprojectElevation());


    // Start with a coordinate in the USA
    DPoint3d myInPoint;
    myInPoint.x = 600000;
    myInPoint.y = 3600000;
    myInPoint.z = 1.1;
    double NAVD88Offset = 0.024167327540054240;
    double GeoidSeparation = -29.519501732186900;
    double NAVD88AndGeoid = NAVD88Offset + GeoidSeparation;

    DPoint3d myOutPoint = {0,0,0};

    // Convert NAVD88 to NAVD88 (using vdcFromDatum)
    ASSERT_TRUE(REPROJECT_Success == theNAD83GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD83GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Change destination to NAVD88 (same)
    ASSERT_TRUE(SUCCESS == theOtherNAD83GCS->SetVerticalDatumCode(vdcNAVD88));
    ASSERT_TRUE(REPROJECT_Success == theNAD83GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD83GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Change destination to vdcEllipsoid
    ASSERT_TRUE(SUCCESS == theOtherNAD83GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD83GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD83GCS));
    ASSERT_NEAR(-GeoidSeparation, myInPoint.z - myOutPoint.z, 0.00001); // Elevation different by Geoid separation.

    // Change destination to vdcGeoid
    ASSERT_TRUE(SUCCESS == theOtherNAD83GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD83GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD83GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Change destination to vdcNGVD29
    ASSERT_TRUE(SUCCESS == theOtherNAD83GCS->SetVerticalDatumCode(vdcNGVD29));
    ASSERT_TRUE(REPROJECT_Success == theNAD83GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD83GCS));
    ASSERT_NEAR(NAVD88Offset, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by NAVD88 offset.

    // Set 2
    // Change source from vdcFromDatum to vdcNAVD88 (no change) and start same tests
    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcNAVD88));
    ASSERT_TRUE(SUCCESS == theOtherNAD83GCS->SetVerticalDatumCode(vdcFromDatum));

    // Convert vdcNAVD88 to vdcNAVD88 (using vdcFromDatum for dest)
    ASSERT_TRUE(REPROJECT_Success == theNAD83GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD83GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Change destination to vdcNAVD88 (same)
    ASSERT_TRUE(SUCCESS == theOtherNAD83GCS->SetVerticalDatumCode(vdcNAVD88));
    ASSERT_TRUE(REPROJECT_Success == theNAD83GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD83GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Change destination to vdcEllipsoid
    ASSERT_TRUE(SUCCESS == theOtherNAD83GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD83GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD83GCS));
    ASSERT_NEAR(-GeoidSeparation, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by Geoid separation.

    // Change destination to vdcGeoid
    ASSERT_TRUE(SUCCESS == theOtherNAD83GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD83GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD83GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Change destination to vdcNGVD29
    ASSERT_TRUE(SUCCESS == theOtherNAD83GCS->SetVerticalDatumCode(vdcNGVD29));
    ASSERT_TRUE(REPROJECT_Success == theNAD83GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD83GCS));
    ASSERT_NEAR(NAVD88Offset, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by NAVD88 offset.

    // Set 3
    // Change source to vdcNGVD29 and start same tests
    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcNGVD29));
    ASSERT_TRUE(SUCCESS == theOtherNAD83GCS->SetVerticalDatumCode(vdcFromDatum));

    // Convert vdcNGVD29 to NAVD88 (using vdcFromDatum for dest)
    ASSERT_TRUE(REPROJECT_Success == theNAD83GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD83GCS));
    ASSERT_NEAR(-NAVD88Offset, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by NAVD88 offset.

    // Change destination to NAVD88
    ASSERT_TRUE(SUCCESS == theOtherNAD83GCS->SetVerticalDatumCode(vdcNAVD88));
    ASSERT_TRUE(REPROJECT_Success == theNAD83GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD83GCS));
    ASSERT_NEAR(-NAVD88Offset, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by NAVD88 offset.

    // Change destination to vdcEllipsoid
    ASSERT_TRUE(SUCCESS == theOtherNAD83GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD83GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD83GCS));
    ASSERT_NEAR(-NAVD88AndGeoid, myInPoint.z - myOutPoint.z, 0.1); // Elevation different by Geoid separation + NAVD88 offset.

    // Change destination to vdcGeoid
    ASSERT_TRUE(SUCCESS == theOtherNAD83GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD83GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD83GCS));
    ASSERT_NEAR(-NAVD88Offset, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by NAVD88 offset.

    // Change destination to vdcNGVD29
    ASSERT_TRUE(SUCCESS == theOtherNAD83GCS->SetVerticalDatumCode(vdcNGVD29));
    ASSERT_TRUE(REPROJECT_Success == theNAD83GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD83GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Set 4
    // Change source to vdcGeoid and start same tests
    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(SUCCESS == theOtherNAD83GCS->SetVerticalDatumCode(vdcFromDatum));

    // Convert Geoid to NAVD88 (using vdcFromDatum for dest)
    ASSERT_TRUE(REPROJECT_Success == theNAD83GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD83GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Change destination to vdcNAVD88
    ASSERT_TRUE(SUCCESS == theOtherNAD83GCS->SetVerticalDatumCode(vdcNAVD88));
    ASSERT_TRUE(REPROJECT_Success == theNAD83GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD83GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Change destination to vdcEllipsoid
    ASSERT_TRUE(SUCCESS == theOtherNAD83GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD83GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD83GCS));
    ASSERT_NEAR(-GeoidSeparation, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by Geoid separation.

    // Change destination to vdcGeoid
    ASSERT_TRUE(SUCCESS == theOtherNAD83GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD83GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD83GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Change destination to vdcNGVD29
    ASSERT_TRUE(SUCCESS == theOtherNAD83GCS->SetVerticalDatumCode(vdcNGVD29));
    ASSERT_TRUE(REPROJECT_Success == theNAD83GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD83GCS));
    ASSERT_NEAR(NAVD88Offset, myInPoint.z - myOutPoint.z, 0.01); // Elevation different byNAVD88 offset.

    // Set 5
    // Change source to vdcEllipsoid and start same tests
    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(SUCCESS == theOtherNAD83GCS->SetVerticalDatumCode(vdcFromDatum));

    // Convert Geoid to NAVD88 (using vdcFromDatum for dest)
    ASSERT_TRUE(REPROJECT_Success == theNAD83GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD83GCS));
    ASSERT_NEAR(GeoidSeparation, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by Geoid separation.

    // Change destination to vdcNAVD88
    ASSERT_TRUE(SUCCESS == theOtherNAD83GCS->SetVerticalDatumCode(vdcNAVD88));
    ASSERT_TRUE(REPROJECT_Success == theNAD83GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD83GCS));
    ASSERT_NEAR(GeoidSeparation, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by Geoid separation.

    // Change destination to vdcEllipsoid
    ASSERT_TRUE(SUCCESS == theOtherNAD83GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD83GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD83GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Change destination to vdcGeoid
    ASSERT_TRUE(SUCCESS == theOtherNAD83GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD83GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD83GCS));
    ASSERT_NEAR(GeoidSeparation, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by Geoid separation.

    // Change destination to vdcNGVD29
    ASSERT_TRUE(SUCCESS == theOtherNAD83GCS->SetVerticalDatumCode(vdcNGVD29));
    ASSERT_TRUE(REPROJECT_Success == theNAD83GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOtherNAD83GCS));
    ASSERT_NEAR(NAVD88AndGeoid, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by Geoid separation + NAVD88 offset.
    }

/*---------------------------------------------------------------------------------**//**
* Vertical Datum Name
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, VariousVerticalTestNAD27_to_NAD83)
    {
    GeoCoordinates::BaseGCSPtr theNAD27GCS;
    GeoCoordinates::BaseGCSPtr theNAD83GCS;

    theNAD27GCS = GeoCoordinates::BaseGCS::CreateGCS("UTM27-16");
    ASSERT_TRUE(theNAD27GCS.IsValid() && theNAD27GCS->IsValid());

    // Default vertical datum should be vdcFromDatum and it should mean NAVD88
    ASSERT_TRUE(vdcFromDatum == theNAD27GCS->GetVerticalDatumCode());
    ASSERT_STREQ("NGVD29", theNAD27GCS->GetVerticalDatumName());

    // Default is to reproject elevation
    ASSERT_TRUE(theNAD27GCS->GetReprojectElevation());

    theNAD83GCS = GeoCoordinates::BaseGCS::CreateGCS("UTM83-15");
    ASSERT_TRUE(theNAD83GCS.IsValid() && theNAD83GCS->IsValid());

    // Default vertical datum should be vdcFromDatum and it should mean NAVD88
    ASSERT_TRUE(vdcFromDatum == theNAD83GCS->GetVerticalDatumCode());
    ASSERT_STREQ("NAVD88", theNAD83GCS->GetVerticalDatumName());

    // Default is to reproject elevation
    ASSERT_TRUE(theNAD83GCS->GetReprojectElevation());

    // Start with a coordinate in the USA
    DPoint3d myInPoint;
    myInPoint.x = 600000;
    myInPoint.y = 3600000;
    myInPoint.z = 1.1;

    double NAVD88Offset = 0.024167327540054240;
    double GeoidSeparation = -29.519501732186900;
    double NAVD88AndGeoid = NAVD88Offset + GeoidSeparation;

    DPoint3d myOutPoint = {0,0,0};

    // Convert NGVD29 to NAVD88 (using vdcFromDatum)
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theNAD83GCS));
    ASSERT_NEAR(-NAVD88Offset, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by NAVD88 offset.

    // Change destination to NAVD88 (same)
    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcNAVD88));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theNAD83GCS));
    ASSERT_NEAR(-NAVD88Offset, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by NAVD88 offset.

    // Change destination to vdcEllipsoid
    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theNAD83GCS));
    ASSERT_NEAR(-NAVD88AndGeoid, myInPoint.z - myOutPoint.z, 0.1); // Elevation different by Geoid separation and NAVD88 offset.

    // Change destination to vdcGeoid
    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theNAD83GCS));
    ASSERT_NEAR(-NAVD88Offset, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by NAVD88 offset.

    // Change destination to vdcNGVD29
    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcNGVD29));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theNAD83GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Set 2
    // Change source from vdcFromDatum to vdcNGVD29 (no change) and start same tests
    ASSERT_TRUE(SUCCESS == theNAD27GCS->SetVerticalDatumCode(vdcNGVD29));
    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcFromDatum));

    // Convert vdcNGVD29 to vdcNAVD88 (using vdcFromDatum for dest)
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theNAD83GCS));
    ASSERT_NEAR(-NAVD88Offset, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by NAVD88 offset.

    // Change destination to vdcNAVD88 (same)
    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcNAVD88));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theNAD83GCS));
    ASSERT_NEAR(-NAVD88Offset, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by NAVD88 offset.

    // Change destination to vdcEllipsoid
    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theNAD83GCS));
    ASSERT_NEAR(-NAVD88AndGeoid, myInPoint.z - myOutPoint.z, 0.1); // Elevation different by Geoid separation and NAVD88 separation.

    // Change destination to vdcGeoid
    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theNAD83GCS));
    ASSERT_NEAR(-NAVD88Offset, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by NAVD88 offset.

    // Change destination to vdcNGVD29
    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcNGVD29));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theNAD83GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Set 3
    // Change source to vdcNGVD29 and start same tests
    ASSERT_TRUE(SUCCESS == theNAD27GCS->SetVerticalDatumCode(vdcNGVD29));
    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcFromDatum));

    // Convert vdcNGVD29 to NAVD88 (using vdcFromDatum for dest)
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theNAD83GCS));
    ASSERT_NEAR(-NAVD88Offset, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by NAVD88 offset.

    // Change destination to NAVD88
    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcNAVD88));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theNAD83GCS));
    ASSERT_NEAR(-NAVD88Offset, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by NAVD88 offset.

    // Change destination to vdcEllipsoid
    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theNAD83GCS));
    ASSERT_NEAR(-NAVD88AndGeoid, myInPoint.z - myOutPoint.z, 0.1); // Elevation different by Geoid separation + NAVD88 offset.

    // Change destination to vdcGeoid
    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theNAD83GCS));
    ASSERT_NEAR(-NAVD88Offset, myInPoint.z - myOutPoint.z, 0.00001); // Elevation different byNAVD88 offset.

    // Change destination to vdcNGVD29
    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcNGVD29));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theNAD83GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Set 4
    // Change source to vdcGeoid and start same tests
    ASSERT_TRUE(SUCCESS == theNAD27GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcFromDatum));

    // Convert Geoid to NAVD88 (using vdcFromDatum for dest)
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theNAD83GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Change destination to vdcNAVD88
    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcNAVD88));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theNAD83GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Change destination to vdcEllipsoid
    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theNAD83GCS));
    ASSERT_NEAR(-GeoidSeparation, myInPoint.z - myOutPoint.z, 0.1); // Elevation different by Geoid separation.

    // Change destination to vdcGeoid
    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theNAD83GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Change destination to vdcNGVD29
    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcNGVD29));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theNAD83GCS));
    ASSERT_NEAR(NAVD88Offset, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by NAVD88 offset.

    // Set 5
    // Change source to vdcEllipsoid and start same tests
    ASSERT_TRUE(SUCCESS == theNAD27GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcFromDatum));

    // Convert Geoid to NAVD88 (using vdcFromDatum for dest)
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theNAD83GCS));
    ASSERT_NEAR(GeoidSeparation, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by Geoid separation.

    // Change destination to vdcNAVD88
    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcNAVD88));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theNAD83GCS));
    ASSERT_NEAR(GeoidSeparation, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by Geoid separation.

    // Change destination to vdcEllipsoid
    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theNAD83GCS));
    ASSERT_NEAR(0.0, myInPoint.z - myOutPoint.z, 0.01); // Elevation different is ellipsoid difference

    // Change destination to vdcGeoid
    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theNAD83GCS));
    ASSERT_NEAR(GeoidSeparation, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by Geoid separation.

    // Change destination to vdcNGVD29
    ASSERT_TRUE(SUCCESS == theNAD83GCS->SetVerticalDatumCode(vdcNGVD29));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theNAD83GCS));
    ASSERT_NEAR(NAVD88AndGeoid, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by Geoid separation + NAVD88 offset.
    }

/*---------------------------------------------------------------------------------**//**
* Vertical Datum Name
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, VariousVerticalTestNAD27_to_WGS84)
    {
    GeoCoordinates::BaseGCSPtr theNAD27GCS;
    GeoCoordinates::BaseGCSPtr theWGS84GCS;

    theNAD27GCS = GeoCoordinates::BaseGCS::CreateGCS("UTM27-16");
    ASSERT_TRUE(theNAD27GCS.IsValid() && theNAD27GCS->IsValid());

    // Default vertical datum should be vdcFromDatum and it should mean NAVD88
    ASSERT_TRUE(vdcFromDatum == theNAD27GCS->GetVerticalDatumCode());
    ASSERT_STREQ("NGVD29", theNAD27GCS->GetVerticalDatumName());

    // Default is to reproject elevation
    ASSERT_TRUE(theNAD27GCS->GetReprojectElevation());

    theWGS84GCS = GeoCoordinates::BaseGCS::CreateGCS("UTM84-15N");
    ASSERT_TRUE(theWGS84GCS.IsValid() && theWGS84GCS->IsValid());

    // Default vertical datum should be vdcFromDatum and it should mean NAVD88
    ASSERT_TRUE(vdcFromDatum == theWGS84GCS->GetVerticalDatumCode());
    ASSERT_STREQ("Ellipsoid", theWGS84GCS->GetVerticalDatumName());

    // Default is to reproject elevation
    ASSERT_TRUE(theWGS84GCS->GetReprojectElevation());

    // Start with a coordinate in the USA
    DPoint3d myInPoint;
    myInPoint.x = 600000;
    myInPoint.y = 3600000;
    myInPoint.z = 1.1;

    double NAVD88Offset = 0.024167327540054240;
    double GeoidSeparation = -29.523165519559370;
    double NAVD88AndGeoid = NAVD88Offset + GeoidSeparation;

    DPoint3d myOutPoint = {0,0,0};

    // Convert NGVD29 to NAVD88 (using vdcFromDatum)
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theWGS84GCS));
    ASSERT_NEAR(-NAVD88AndGeoid, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by NAVD88 offset.

    // Change destination to vdcEllipsoid (same)
    ASSERT_TRUE(SUCCESS == theWGS84GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theWGS84GCS));
    ASSERT_NEAR(-NAVD88AndGeoid, myInPoint.z - myOutPoint.z, 0.1); // Elevation different by Geoid separation and NAVD88 offset.

    // Change destination to vdcGeoid
    ASSERT_TRUE(SUCCESS == theWGS84GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theWGS84GCS));
    ASSERT_NEAR(-NAVD88Offset, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by NAVD88 offset.

    // Set 2
    // Change source from vdcFromDatum to vdcNGVD29 (no change) and start same tests
    ASSERT_TRUE(SUCCESS == theNAD27GCS->SetVerticalDatumCode(vdcNGVD29));
    ASSERT_TRUE(SUCCESS == theWGS84GCS->SetVerticalDatumCode(vdcFromDatum));

    // Convert vdcNGVD29 to vdcGeoid (using vdcFromDatum for dest)
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theWGS84GCS));
    ASSERT_NEAR(-NAVD88AndGeoid, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by NAVD88 offset.

    // Change destination to vdcEllipsoid
    ASSERT_TRUE(SUCCESS == theWGS84GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theWGS84GCS));
    ASSERT_NEAR(-NAVD88AndGeoid, myInPoint.z - myOutPoint.z, 0.1); // Elevation different by Geoid separation and NAVD88 separation.

    // Change destination to vdcGeoid
    ASSERT_TRUE(SUCCESS == theWGS84GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theWGS84GCS));
    ASSERT_NEAR(-NAVD88Offset, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by NAVD88 offset.

    // Set 3
    // Change source to vdcNGVD29 and start same tests
    ASSERT_TRUE(SUCCESS == theNAD27GCS->SetVerticalDatumCode(vdcNGVD29));
    ASSERT_TRUE(SUCCESS == theWGS84GCS->SetVerticalDatumCode(vdcFromDatum));

    // Convert vdcNGVD29 to NAVD88 (using vdcFromDatum for dest)
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theWGS84GCS));
    ASSERT_NEAR(-NAVD88AndGeoid, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by NAVD88 offset.

    // Change destination to vdcEllipsoid
    ASSERT_TRUE(SUCCESS == theWGS84GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theWGS84GCS));
    ASSERT_NEAR(-NAVD88AndGeoid, myInPoint.z - myOutPoint.z, 0.1); // Elevation different by Geoid separation + NAVD88 offset.

    // Change destination to vdcGeoid
    ASSERT_TRUE(SUCCESS == theWGS84GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theWGS84GCS));
    ASSERT_NEAR(-NAVD88Offset, myInPoint.z - myOutPoint.z, 0.00001); // Elevation different byNAVD88 offset.

    // Set 4
    // Change source to vdcGeoid and start same tests
    ASSERT_TRUE(SUCCESS == theNAD27GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(SUCCESS == theWGS84GCS->SetVerticalDatumCode(vdcFromDatum));

    // Convert Geoid to NAVD88 (using vdcFromDatum for dest)
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theWGS84GCS));
    ASSERT_NEAR(-GeoidSeparation, myInPoint.z - myOutPoint.z, 0.1); // Elevation different by Geoid separation.

    // Change destination to vdcEllipsoid
    ASSERT_TRUE(SUCCESS == theWGS84GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theWGS84GCS));
    ASSERT_NEAR(-GeoidSeparation, myInPoint.z - myOutPoint.z, 0.1); // Elevation different by Geoid separation.

    // Change destination to vdcGeoid
    ASSERT_TRUE(SUCCESS == theWGS84GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theWGS84GCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Set 5
    // Change source to vdcEllipsoid and start same tests
    ASSERT_TRUE(SUCCESS == theNAD27GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(SUCCESS == theWGS84GCS->SetVerticalDatumCode(vdcFromDatum));

    // Convert Geoid to NAVD88 (using vdcFromDatum for dest)
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theWGS84GCS));
    ASSERT_NEAR(0.0, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by Geoid separation.

    // Change destination to vdcEllipsoid
    ASSERT_TRUE(SUCCESS == theWGS84GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theWGS84GCS));
    ASSERT_NEAR(0.0, myInPoint.z - myOutPoint.z, 0.01); // Elevation different is ellipsoid difference

    // Change destination to vdcGeoid
    ASSERT_TRUE(SUCCESS == theWGS84GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(REPROJECT_Success == theNAD27GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theWGS84GCS));
    ASSERT_NEAR(GeoidSeparation, myInPoint.z - myOutPoint.z, 0.01); // Elevation different by Geoid separation.

    // Set 6
    // Change source to vdcLocalEllipsoid is not possible
    ASSERT_TRUE(SUCCESS != theNAD27GCS->SetVerticalDatumCode(vdcLocalEllipsoid));
    }

#if (0)
/*---------------------------------------------------------------------------------**//**
* Vertical Datum test with two ellipsoids very different
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, VariousVerticalTestWGS84_to_OSGB)
    {
    GeoCoordinates::BaseGCSPtr theWGS84GCS;
    GeoCoordinates::BaseGCSPtr theOSGBGCS;

    theWGS84GCS = GeoCoordinates::BaseGCS::CreateGCS("UTM84-16N");
    ASSERT_TRUE(theWGS84GCS.IsValid() && theWGS84GCS->IsValid());

    // Default vertical datum should be vdcFromDatum and it should mean ellipsoid
    ASSERT_TRUE(vdcFromDatum == theWGS84GCS->GetVerticalDatumCode());
    ASSERT_STREQ("Ellipsoid", theWGS84GCS->GetVerticalDatumName());

    // Default is to reproject elevation
    ASSERT_TRUE(theWGS84GCS->GetReprojectElevation());

    theOSGBGCS = GeoCoordinates::BaseGCS::CreateGCS("EPSG:27700");
    ASSERT_TRUE(theOSGBGCS.IsValid() && theOSGBGCS->IsValid());

    // Default vertical datum should be vdcFromDatum and it should mean ellipsoid
    ASSERT_TRUE(vdcFromDatum == theOSGBGCS->GetVerticalDatumCode());
    ASSERT_STREQ("Ellipsoid", theOSGBGCS->GetVerticalDatumName());

    // Default is to reproject elevation
    ASSERT_TRUE(theOSGBGCS->GetReprojectElevation());

    // Start with a coordinate in the USA
    DPoint3d myInPoint;
    myInPoint.x = 600000;
    myInPoint.y = 3600000;
    myInPoint.z = 1.1;

    double GeoidSeparation = -29.519501732186900;

    DPoint3d myOutPoint;

    // Convert Ellipsoid to Ellipsoid (using vdcFromDatum)
    ASSERT_TRUE(REPROJECT_Success == theWGS84GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOSGBGCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Change destination to vdcEllipsoid
    ASSERT_TRUE(SUCCESS == theOSGBGCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(REPROJECT_Success == theWGS84GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOSGBGCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Change destination to vdcGeoid
    ASSERT_TRUE(SUCCESS == theOSGBGCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(REPROJECT_Success == theWGS84GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOSGBGCS));
    ASSERT_NEAR(GeoidSeparation, myInPoint.z - myOutPoint.z, 0.00001); // Elevation different by Geoid separation.

    // Change source to vdcEllipsoid and change back dest to vdcEllipsoid
    ASSERT_TRUE(SUCCESS == theOSGBGCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(SUCCESS == theWGS84GCS->SetVerticalDatumCode(vdcEllipsoid));
    ASSERT_TRUE(REPROJECT_Success == theWGS84GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOSGBGCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001);

    // Change destination to vdcGeoid
    ASSERT_TRUE(SUCCESS == theOSGBGCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(REPROJECT_Success == theWGS84GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOSGBGCS));
    ASSERT_NEAR(GeoidSeparation, myInPoint.z - myOutPoint.z, 0.00001); // Elevation different by Geoid separation.

    // Both are geoid.
    ASSERT_TRUE(SUCCESS == theWGS84GCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(SUCCESS == theOSGBGCS->SetVerticalDatumCode(vdcGeoid));
    ASSERT_TRUE(REPROJECT_Success == theWGS84GCS->CartesianFromCartesian(myOutPoint, myInPoint, *theOSGBGCS));
    ASSERT_NEAR(myInPoint.z, myOutPoint.z, 0.00001); // Can there be a slight difference?
    }
#endif

/*---------------------------------------------------------------------------------**//**
* Vertical Datum test with two ellipsoids very different using direct DATUM API
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, VariousVerticalThrougDatumTestWGS84_to_OSGB)
    {
    GeoCoordinates::DatumCP theWGS84Datum;
    GeoCoordinates::DatumCP theOSGBDatum;

    theWGS84Datum = GeoCoordinates::Datum::CreateDatum("WGS84");
    ASSERT_TRUE(nullptr != theWGS84Datum && theWGS84Datum->IsValid());

    theOSGBDatum = GeoCoordinates::Datum::CreateDatum("OSGB-7P-2");
    ASSERT_TRUE(nullptr != theOSGBDatum && theOSGBDatum->IsValid());

    GeoCoordinates::DatumConverter* datumConverter = GeoCoordinates::DatumConverter::Create(*theWGS84Datum, *theOSGBDatum);

    ASSERT_TRUE(nullptr != datumConverter);

    datumConverter->SetReprojectElevation(false);

    GeoPoint thePoint;
    thePoint.longitude = -0.7;
    thePoint.latitude = 51.2;
    thePoint.elevation = 1.0;

    GeoPoint theOutPoint;

    datumConverter->ConvertLatLong3D(theOutPoint, thePoint);

    EXPECT_NEAR(theOutPoint.elevation, 1.0, 0.00001);

    datumConverter->SetReprojectElevation(true);

    datumConverter->ConvertLatLong3D(theOutPoint, thePoint);

    EXPECT_NEAR(theOutPoint.elevation, 1.0, 0.00001);

    datumConverter->Destroy();

    // -----------------------------------------
    datumConverter = GeoCoordinates::DatumConverter::Create(*theWGS84Datum, *theOSGBDatum, vdcEllipsoid, vdcEllipsoid);
    ASSERT_TRUE(nullptr != datumConverter);
    datumConverter->SetReprojectElevation(false);

    datumConverter->ConvertLatLong3D(theOutPoint, thePoint);

    EXPECT_NEAR(theOutPoint.elevation, 1.0, 0.00001);

    datumConverter->SetReprojectElevation(true);

    datumConverter->ConvertLatLong3D(theOutPoint, thePoint);

    EXPECT_NEAR(theOutPoint.elevation, 1.0, 0.00001);

    datumConverter->Destroy();

    // -----------------------------------------
    datumConverter = GeoCoordinates::DatumConverter::Create(*theWGS84Datum, *theOSGBDatum, vdcEllipsoid, vdcGeoid);

    ASSERT_TRUE(nullptr != datumConverter);
    datumConverter->SetReprojectElevation(false);

    datumConverter->ConvertLatLong3D(theOutPoint, thePoint);

    EXPECT_NEAR(theOutPoint.elevation, 1.0, 0.00001);

    datumConverter->SetReprojectElevation(true);

    datumConverter->ConvertLatLong3D(theOutPoint, thePoint);

    EXPECT_NEAR(theOutPoint.elevation, -45.116043018510410, 0.00001);

    datumConverter->Destroy();

    // -----------------------------------------
    datumConverter = GeoCoordinates::DatumConverter::Create(*theWGS84Datum, *theOSGBDatum, vdcGeoid, vdcGeoid);

    ASSERT_TRUE(nullptr != datumConverter);
    datumConverter->SetReprojectElevation(false);

    datumConverter->ConvertLatLong3D(theOutPoint, thePoint);

    EXPECT_NEAR(theOutPoint.elevation, 1.0, 0.00001);

    datumConverter->SetReprojectElevation(true);

    datumConverter->ConvertLatLong3D(theOutPoint, thePoint);

    EXPECT_NEAR(theOutPoint.elevation, 1.0, 0.00001);

    datumConverter->Destroy();

    // -----------------------------------------
    datumConverter = GeoCoordinates::DatumConverter::Create(*theWGS84Datum, *theOSGBDatum, vdcGeoid, vdcEllipsoid);

    ASSERT_TRUE(nullptr != datumConverter);
    datumConverter->SetReprojectElevation(false);


    datumConverter->ConvertLatLong3D(theOutPoint, thePoint);

    EXPECT_NEAR(theOutPoint.elevation, 1.0, 0.00001);

    datumConverter->SetReprojectElevation(true);

    datumConverter->ConvertLatLong3D(theOutPoint, thePoint);

    EXPECT_NEAR(theOutPoint.elevation, 47.118440551757807, 0.00001);

    datumConverter->Destroy();

    theWGS84Datum->Destroy();
    theOSGBDatum->Destroy();
    }

/*---------------------------------------------------------------------------------**//**
* IsNADXX for datum - Tests the Datum::IsNAD27/IsNAD83 methods.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, InternalIsNADForDatums)
    {
    GeoCoordinates::DatumCP theDatum1;

    // NAD83
    theDatum1 = GeoCoordinates::Datum::CreateDatum("NAD83");
    ASSERT_TRUE(theDatum1 != nullptr && theDatum1->IsValid());
    ASSERT_TRUE(theDatum1->IsNAD83());
    ASSERT_TRUE(!theDatum1->IsNAD27());
    theDatum1->Destroy();

    // EPSG:6269
    GeoCoordinates::DatumCP theDatum2;
    theDatum2 = GeoCoordinates::Datum::CreateDatum("EPSG:6269");
    ASSERT_TRUE(theDatum2 != nullptr && theDatum2->IsValid());
    ASSERT_TRUE(theDatum2->IsNAD83());
    ASSERT_TRUE(!theDatum2->IsNAD27());
    theDatum2->Destroy();

    // HPGN
    GeoCoordinates::DatumCP theDatum3;
    theDatum3 = GeoCoordinates::Datum::CreateDatum("HPGN");
    ASSERT_TRUE(theDatum3 != nullptr && theDatum3->IsValid());
    ASSERT_TRUE(theDatum3->IsNAD83());
    ASSERT_TRUE(!theDatum3->IsNAD27());
    theDatum3->Destroy();

    // EPSG:6152
    GeoCoordinates::DatumCP theDatum4;
    theDatum4 = GeoCoordinates::Datum::CreateDatum("EPSG:6152");
    ASSERT_TRUE(theDatum4 != nullptr && theDatum4->IsValid());
    ASSERT_TRUE(theDatum4->IsNAD83());
    ASSERT_TRUE(!theDatum4->IsNAD27());
    theDatum4->Destroy();

    // NAD83/2011
    GeoCoordinates::DatumCP theDatum5;
    theDatum5 = GeoCoordinates::Datum::CreateDatum("NAD83/2011");
    ASSERT_TRUE(theDatum5 != nullptr && theDatum5->IsValid());
    ASSERT_TRUE(theDatum5->IsNAD83());
    ASSERT_TRUE(!theDatum5->IsNAD27());
    theDatum5->Destroy();

    // NAD83/HARN
    GeoCoordinates::DatumCP theDatum6;
    theDatum6 = GeoCoordinates::Datum::CreateDatum("NAD83/HARN");
    ASSERT_TRUE(theDatum6 != nullptr && theDatum6->IsValid());
    ASSERT_TRUE(theDatum6->IsNAD83());
    ASSERT_TRUE(!theDatum6->IsNAD27());
    theDatum6->Destroy();

    // NSRS07
    GeoCoordinates::DatumCP theDatum7;
    theDatum7 = GeoCoordinates::Datum::CreateDatum("NSRS07");
    ASSERT_TRUE(theDatum7 != nullptr && theDatum7->IsValid());
    ASSERT_TRUE(theDatum7->IsNAD83());
    ASSERT_TRUE(!theDatum7->IsNAD27());
    theDatum7->Destroy();

    // NSRS11
    GeoCoordinates::DatumCP theDatum8;
    theDatum8 = GeoCoordinates::Datum::CreateDatum("NSRS11");
    ASSERT_TRUE(theDatum8 != nullptr && theDatum8->IsValid());
    ASSERT_TRUE(theDatum8->IsNAD83());
    ASSERT_TRUE(!theDatum8->IsNAD27());
    theDatum8->Destroy();

    // NAD27
    GeoCoordinates::DatumCP theDatum9;
    theDatum9 = GeoCoordinates::Datum::CreateDatum("NAD27");
    ASSERT_TRUE(theDatum9 != nullptr && theDatum9->IsValid());
    ASSERT_TRUE(!theDatum9->IsNAD83());
    ASSERT_TRUE(theDatum9->IsNAD27());
    theDatum9->Destroy();

    // EPSG:6267
    GeoCoordinates::DatumCP theDatum10;
    theDatum10 = GeoCoordinates::Datum::CreateDatum("EPSG:6267");
    ASSERT_TRUE(theDatum10 != nullptr && theDatum10->IsValid());
    ASSERT_TRUE(!theDatum10->IsNAD83());
    ASSERT_TRUE(theDatum10->IsNAD27());
    theDatum10->Destroy();

    // NAD27-48
    GeoCoordinates::DatumCP theDatum11;
    theDatum11 = GeoCoordinates::Datum::CreateDatum("NAD27-48");
    ASSERT_TRUE(theDatum11 != nullptr && theDatum11->IsValid());
    ASSERT_TRUE(!theDatum11->IsNAD83());
    ASSERT_TRUE(theDatum11->IsNAD27());
    theDatum11->Destroy();

    // Canadian variations should indicate not NAD27 and not NAD83
    // CSRS
    GeoCoordinates::DatumCP theDatum12;
    theDatum12 = GeoCoordinates::Datum::CreateDatum("CSRS");
    ASSERT_TRUE(theDatum12 != nullptr && theDatum12->IsValid());
    ASSERT_TRUE(!theDatum12->IsNAD83());
    ASSERT_TRUE(!theDatum12->IsNAD27());
    theDatum12->Destroy();

    // EPSG:6140
    GeoCoordinates::DatumCP theDatum13;
    theDatum13 = GeoCoordinates::Datum::CreateDatum("EPSG:6140");
    ASSERT_TRUE(theDatum13 != nullptr && theDatum13->IsValid());
    ASSERT_TRUE(!theDatum13->IsNAD83());
    ASSERT_TRUE(!theDatum13->IsNAD27());
    theDatum13->Destroy();

    // EPSG:6609 (NAD27 for Quebec)
    GeoCoordinates::DatumCP theDatum14;
    theDatum14 = GeoCoordinates::Datum::CreateDatum("EPSG:6609");
    ASSERT_TRUE(theDatum14 != nullptr && theDatum14->IsValid());
    ASSERT_TRUE(!theDatum14->IsNAD83());
    ASSERT_TRUE(!theDatum14->IsNAD27());
    theDatum14->Destroy();

    // NAD27/CGQ77-98 (NAD27 for Quebec)
    GeoCoordinates::DatumCP theDatum15;
    theDatum15 = GeoCoordinates::Datum::CreateDatum("NAD27/CGQ77-98");
    ASSERT_TRUE(theDatum15 != nullptr && theDatum15->IsValid());
    ASSERT_TRUE(!theDatum15->IsNAD83());
    ASSERT_TRUE(!theDatum15->IsNAD27());
    theDatum15->Destroy();

    // NAD27/CGQ77-83 (NAD27 for Quebec)
    GeoCoordinates::DatumCP theDatum16;
    theDatum16 = GeoCoordinates::Datum::CreateDatum("NAD27/CGQ77-83");
    ASSERT_TRUE(theDatum16 != nullptr && theDatum16->IsValid());
    ASSERT_TRUE(!theDatum16->IsNAD83());
    ASSERT_TRUE(!theDatum16->IsNAD27());
    theDatum16->Destroy();

    // NAD27-CN
    GeoCoordinates::DatumCP theDatum17;
    theDatum17 = GeoCoordinates::Datum::CreateDatum("NAD27-CN");
    ASSERT_TRUE(theDatum17 != nullptr && theDatum17->IsValid());
    ASSERT_TRUE(!theDatum17->IsNAD83());
    ASSERT_TRUE(!theDatum17->IsNAD27());
    theDatum17->Destroy();

    // NAD27-CZ (NAD27 for Panama)
    GeoCoordinates::DatumCP theDatum18;
    theDatum18 = GeoCoordinates::Datum::CreateDatum("NAD27-CZ");
    ASSERT_TRUE(theDatum18 != nullptr && theDatum18->IsValid());
    ASSERT_TRUE(!theDatum18->IsNAD83());
    ASSERT_TRUE(!theDatum18->IsNAD27());
    theDatum18->Destroy();

    // NAD27-CB (NAD27 for carribean)
    GeoCoordinates::DatumCP theDatum19;
    theDatum19 = GeoCoordinates::Datum::CreateDatum("NAD27-CB");
    ASSERT_TRUE(theDatum19 != nullptr && theDatum19->IsValid());
    ASSERT_TRUE(!theDatum19->IsNAD83());
    ASSERT_TRUE(!theDatum19->IsNAD27());
    theDatum19->Destroy();

    // NAD27-CA (NAD27 for Central America)
    GeoCoordinates::DatumCP theDatum20;
    theDatum20 = GeoCoordinates::Datum::CreateDatum("NAD27-CA");
    ASSERT_TRUE(theDatum20 != nullptr && theDatum20->IsValid());
    ASSERT_TRUE(!theDatum20->IsNAD83());
    ASSERT_TRUE(!theDatum20->IsNAD27());
    theDatum20->Destroy();

    // CUBA (NAD27 for CUBA)
    GeoCoordinates::DatumCP theDatum21;
    theDatum21 = GeoCoordinates::Datum::CreateDatum("CUBA");
    ASSERT_TRUE(theDatum21 != nullptr && theDatum21->IsValid());
    ASSERT_TRUE(!theDatum21->IsNAD83());
    ASSERT_TRUE(!theDatum21->IsNAD27());
    theDatum21->Destroy();

    // NAD27-GR (Greenland)
    GeoCoordinates::DatumCP theDatum22;
    theDatum22 = GeoCoordinates::Datum::CreateDatum("NAD27-GR");
    ASSERT_TRUE(theDatum22 != nullptr && theDatum22->IsValid());
    ASSERT_TRUE(!theDatum22->IsNAD83());
    ASSERT_TRUE(!theDatum22->IsNAD27());
    theDatum22->Destroy();

    // NAD27-MX (Mexico)
    GeoCoordinates::DatumCP theDatum23;
    theDatum23 = GeoCoordinates::Datum::CreateDatum("NAD27-GR");
    ASSERT_TRUE(theDatum23 != nullptr && theDatum23->IsValid());
    ASSERT_TRUE(!theDatum23->IsNAD83());
    ASSERT_TRUE(!theDatum23->IsNAD27());
    theDatum23->Destroy();

    // NAD27HII (Hawai on INTL Ellipsoid) ... even if USA we do not consider it NAD27
    GeoCoordinates::DatumCP theDatum24;
    theDatum24 = GeoCoordinates::Datum::CreateDatum("NAD27HII");
    ASSERT_TRUE(theDatum24 != nullptr && theDatum24->IsValid());
    ASSERT_TRUE(!theDatum24->IsNAD83());
    ASSERT_TRUE(!theDatum24->IsNAD27());
    theDatum24->Destroy();

    // EPSG:6608 (NAD27 Ontario)
    GeoCoordinates::DatumCP theDatum25;
    theDatum25 = GeoCoordinates::Datum::CreateDatum("EPSG:6608");
    ASSERT_TRUE(theDatum25 != nullptr && theDatum25->IsValid());
    ASSERT_TRUE(!theDatum25->IsNAD83());
    ASSERT_TRUE(!theDatum25->IsNAD27());
    theDatum25->Destroy();

    // NAD27/1976 (NAD27 Ontario)
    GeoCoordinates::DatumCP theDatum26;
    theDatum26 = GeoCoordinates::Datum::CreateDatum("NAD27/1976");
    ASSERT_TRUE(theDatum26 != nullptr && theDatum26->IsValid());
    ASSERT_TRUE(!theDatum26->IsNAD83());
    ASSERT_TRUE(!theDatum26->IsNAD27());
    theDatum26->Destroy();
    }

/*---------------------------------------------------------------------------------**//**
* IsNADXX for GCS - Tests the BaseGCS::IsNAD27/IsNAD83 methods.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, InternalIsNADForGCS)
    {
    GeoCoordinates::BaseGCSPtr theGCS;

    // NAD83
    theGCS = GeoCoordinates::BaseGCS::CreateGCS("UTM83-15");
    ASSERT_TRUE(theGCS.IsValid() && theGCS->IsValid());
    ASSERT_TRUE(theGCS->IsNAD83());
    ASSERT_TRUE(!theGCS->IsNAD27());
    theGCS = nullptr;

    // HPGN
    theGCS = GeoCoordinates::BaseGCS::CreateGCS("HARN.Texas/Albers");
    ASSERT_TRUE(theGCS.IsValid() && theGCS->IsValid());
    ASSERT_TRUE(theGCS->IsNAD83());
    ASSERT_TRUE(!theGCS->IsNAD27());
    theGCS = nullptr;

    // NAD83/2011
    theGCS = GeoCoordinates::BaseGCS::CreateGCS("AK83/2011-1");
    ASSERT_TRUE(theGCS.IsValid() && theGCS->IsValid());
    ASSERT_TRUE(theGCS->IsNAD83());
    ASSERT_TRUE(!theGCS->IsNAD27());
    theGCS = nullptr;

    // NSRS07
    theGCS = GeoCoordinates::BaseGCS::CreateGCS("NSRS07.AL-E");
    ASSERT_TRUE(theGCS.IsValid() && theGCS->IsValid());
    ASSERT_TRUE(theGCS->IsNAD83());
    ASSERT_TRUE(!theGCS->IsNAD27());
    theGCS = nullptr;

    // NSRS11
    theGCS = GeoCoordinates::BaseGCS::CreateGCS("NSRS11.SFO-CS13");
    ASSERT_TRUE(theGCS.IsValid() && theGCS->IsValid());
    ASSERT_TRUE(theGCS->IsNAD83());
    ASSERT_TRUE(!theGCS->IsNAD27());
    theGCS = nullptr;

    // NAD27
    theGCS = GeoCoordinates::BaseGCS::CreateGCS("AL-E");
    ASSERT_TRUE(theGCS.IsValid() && theGCS->IsValid());
    ASSERT_TRUE(!theGCS->IsNAD83());
    ASSERT_TRUE(theGCS->IsNAD27());
    theGCS = nullptr;

    // Canadian variations should indicate not NAD27 and not NAD83
    // CSRS
    theGCS = GeoCoordinates::BaseGCS::CreateGCS("NewBrunswickCSRS");
    ASSERT_TRUE(theGCS.IsValid() && theGCS->IsValid());
    ASSERT_TRUE(!theGCS->IsNAD83());
    ASSERT_TRUE(!theGCS->IsNAD27());
    theGCS = nullptr;

    // NAD27/CGQ77-98 (NAD27 for Quebec)
    theGCS = GeoCoordinates::BaseGCS::CreateGCS("NAD27/CGQ77-98.SCoPQ2");
    ASSERT_TRUE(theGCS.IsValid() && theGCS->IsValid());
    ASSERT_TRUE(!theGCS->IsNAD83());
    ASSERT_TRUE(!theGCS->IsNAD27());
    theGCS = nullptr;

    // NAD27/1976 (NAD27 Ontario)
    theGCS = GeoCoordinates::BaseGCS::CreateGCS("NAD27/1976.MTM-8");
    ASSERT_TRUE(theGCS.IsValid() && theGCS->IsValid());
    ASSERT_TRUE(!theGCS->IsNAD83());
    ASSERT_TRUE(!theGCS->IsNAD27());
    theGCS = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGCSUnitTests, LinearTransform1)
{
    GeoCoordinates::BaseGCSPtr theSourceGCS;
    GeoCoordinates::BaseGCSPtr theDestGCS;

    theSourceGCS = GeoCoordinates::BaseGCS::CreateGCS("EPSG:102012");
    ASSERT_TRUE(theSourceGCS.IsValid() && theSourceGCS->IsValid());

    theDestGCS = GeoCoordinates::BaseGCS::CreateGCS("EPSG:4326");
    ASSERT_TRUE(theDestGCS.IsValid() && theDestGCS->IsValid());

    DRange3d              extent;
    extent.low.x = 0.0;
    extent.low.y = 0.0;
    extent.low.z = 0.0;

    extent.high.x = 0.01;
    extent.high.y = 0.01;
    extent.high.z = 0.01;

    Transform              outTransform;
    double maxError = 0.0;
    double meanError = 0.0;
    ASSERT_TRUE(REPROJECT_Success == theSourceGCS->GetLinearTransform(&outTransform, extent, *theDestGCS, &maxError, &meanError));

    ASSERT_TRUE(maxError < 0.01);
    ASSERT_TRUE(meanError < 0.01);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, LinearTransform2)
    {
    GeoCoordinates::BaseGCSPtr theSourceGCS;
    GeoCoordinates::BaseGCSPtr theDestGCS;

    theSourceGCS = GeoCoordinates::BaseGCS::CreateGCS("TX83-NCF");
    ASSERT_TRUE(theSourceGCS.IsValid() && theSourceGCS->IsValid());

    theDestGCS = GeoCoordinates::BaseGCS::CreateGCS("UTM84-14N");
    ASSERT_TRUE(theDestGCS.IsValid() && theDestGCS->IsValid());

    DRange3d              extent;
    extent.low.x = 2411760.9129999126;
    extent.low.y = 7016097.8990061926;
    extent.low.z = 350.0;
    extent.high.x = 2532534.5129998885;
    extent.high.y = 7057793.8990063900;
    extent.high.z = 990.0;

    Transform              outTransform;
    double maxError = 0.0;
    double meanError = 0.0;
    ASSERT_TRUE(REPROJECT_Success == theSourceGCS->GetLinearTransform(&outTransform, extent, *theDestGCS, &maxError, &meanError));

    ASSERT_TRUE(maxError < 2.0);
    ASSERT_TRUE(meanError < 0.5);
    }

/*---------------------------------------------------------------------------------**//**
* Domain tests
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, LatLongToFromXYZ)
    {
    GeoCoordinates::BaseGCSPtr currentGCSEllipsoid;
    GeoCoordinates::BaseGCSPtr currentGCSGeoid;

    // First test using WGS84 lat/long
    currentGCSEllipsoid = GeoCoordinates::BaseGCS::CreateGCS("LL84");
    currentGCSEllipsoid->SetReprojectElevation(true);

    GeoPoint point1;
    point1.longitude = -71;
    point1.latitude = 48;
    point1.elevation = 0.0;

    DPoint3d xyz = {0.0, 0.0, 0.0};

    EXPECT_TRUE(REPROJECT_Success == currentGCSEllipsoid->XYZFromLatLong(xyz, point1));

    EXPECT_TRUE(REPROJECT_Success == currentGCSEllipsoid->LatLongFromXYZ(point1, xyz));

    EXPECT_NEAR(point1.longitude, -71, 0.00001);
    EXPECT_NEAR(point1.latitude, 48, 0.00001);
    EXPECT_NEAR(point1.elevation, 0.0, 0.001);

    point1.longitude = -71;
    point1.latitude = 48;
    point1.elevation = 0.0;

    // Same test but this time we use a geoid based GCS
    currentGCSGeoid =  GeoCoordinates::BaseGCS::CreateGCS("LL84");
    currentGCSEllipsoid->SetReprojectElevation(true);
    currentGCSGeoid->SetVerticalDatumCode(GeoCoordinates::vdcGeoid);

    DPoint3d xyz2 = {0.0, 0.0, 0.0};
    EXPECT_TRUE(REPROJECT_Success == currentGCSGeoid->XYZFromLatLong(xyz2, point1));

    EXPECT_TRUE(REPROJECT_Success == currentGCSGeoid->LatLongFromXYZ(point1, xyz2));

    // Round trip will yield the same result.
    EXPECT_NEAR(point1.longitude, -71, 0.00001);
    EXPECT_NEAR(point1.latitude, 48, 0.00001);
    EXPECT_NEAR(point1.elevation, 0.0, 0.001);

    // All we know is that XYZ values are different
    EXPECT_TRUE(xyz2.x != xyz.x);
    EXPECT_TRUE(xyz2.y != xyz.y);
    EXPECT_TRUE(xyz2.z != xyz.z);

    // Now we will make computation at precise locations on Earth

    // Equator on Greenwich
    GeoPoint pointEquatorGreenwich;
    pointEquatorGreenwich.longitude = 0;
    pointEquatorGreenwich.latitude = 0;
    pointEquatorGreenwich.elevation = 0.0;

    EXPECT_TRUE(REPROJECT_Success == currentGCSEllipsoid->XYZFromLatLong(xyz, pointEquatorGreenwich));

    EXPECT_TRUE(REPROJECT_Success == currentGCSGeoid->XYZFromLatLong(xyz2, pointEquatorGreenwich));

    EXPECT_NEAR(xyz.x - xyz2.x, -17.1630, 0.01); // The X value should represent the geoid separation
    EXPECT_NEAR(xyz.y, xyz2.y, 0.001);
    EXPECT_NEAR(xyz.z, xyz2.z, 0.001);

    GeoPoint pointEquatorMinus90;
    pointEquatorMinus90.longitude = -90;
    pointEquatorMinus90.latitude = 0;
    pointEquatorMinus90.elevation = 0.0;

    EXPECT_TRUE(REPROJECT_Success == currentGCSEllipsoid->XYZFromLatLong(xyz, pointEquatorMinus90));

    EXPECT_TRUE(REPROJECT_Success == currentGCSGeoid->XYZFromLatLong(xyz2, pointEquatorMinus90));

    EXPECT_NEAR(xyz.x, xyz2.x, 0.001);
    EXPECT_NEAR(xyz.y - xyz2.y, -4.2873, 0.01); // The Y value should represent the geoid separation
    EXPECT_NEAR(xyz.z, xyz2.z, 0.001);

    GeoPoint pointEquatorPlus90;
    pointEquatorPlus90.longitude = 90;
    pointEquatorPlus90.latitude = 0;
    pointEquatorPlus90.elevation = 0.0;

    EXPECT_TRUE(REPROJECT_Success == currentGCSEllipsoid->XYZFromLatLong(xyz, pointEquatorPlus90));

    EXPECT_TRUE(REPROJECT_Success == currentGCSGeoid->XYZFromLatLong(xyz2, pointEquatorPlus90));

    EXPECT_NEAR(xyz.x, xyz2.x, 0.001);
    EXPECT_NEAR(xyz.y - xyz2.y, 63.2371, 0.01); // The Y value should represent the geoid separation
    EXPECT_NEAR(xyz.z, xyz2.z, 0.001);

    GeoPoint pointNorthPole;
    pointNorthPole.longitude = 0.0;
    pointNorthPole.latitude = 90;
    pointNorthPole.elevation = 0.0;

    EXPECT_TRUE(REPROJECT_Success == currentGCSEllipsoid->XYZFromLatLong(xyz, pointNorthPole));

    EXPECT_TRUE(REPROJECT_Success == currentGCSGeoid->XYZFromLatLong(xyz2, pointNorthPole));

    EXPECT_NEAR(xyz.x, xyz2.x, 0.001);
    EXPECT_NEAR(xyz.y, xyz2.y, 0.001);
    EXPECT_NEAR(xyz.z - xyz2.z, -13.605, 0.001); // The Z value should represent the geoid separation

    GeoPoint pointSouthPole;
    pointSouthPole.longitude = 0.0;
    pointSouthPole.latitude = -90;
    pointSouthPole.elevation = 0.0;

    EXPECT_TRUE(REPROJECT_Success == currentGCSEllipsoid->XYZFromLatLong(xyz, pointSouthPole));

    EXPECT_TRUE(REPROJECT_Success == currentGCSGeoid->XYZFromLatLong(xyz2, pointSouthPole));

    EXPECT_NEAR(xyz.x, xyz2.x, 0.001);
    EXPECT_NEAR(xyz.y, xyz2.y, 0.001);
    EXPECT_NEAR(xyz.z - xyz2.z, -29.5350, 0.001); // The Z value should represent the geoid separation
    }

/*---------------------------------------------------------------------------------**//**
* Domain tests
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, LatLongToFromXYZForOSGB15)
    {
    GeoCoordinates::BaseGCSPtr ostn15GCS;
    GeoCoordinates::BaseGCSPtr wgs84GCS;

    // First test using WGS84 lat/long
    ostn15GCS = GeoCoordinates::BaseGCS::CreateGCS("OSTN15.BNG-Approx");
    ostn15GCS->SetReprojectElevation(true);
    ostn15GCS->SetVerticalDatumCode(GeoCoordinates::vdcEllipsoid);

    // Verify that the round trip in OSGB works
    GeoPoint latLongOSGB;
    latLongOSGB.longitude = -0.01;
    latLongOSGB.latitude = 51.5;
    latLongOSGB.elevation = 0.0;

    DPoint3d xyzOSGB1 = {0.0, 0.0, 0.0};

    // Since conversion to WGS is based on NTv2 files but ellipsoid is of different shape this call will fail.
    EXPECT_TRUE(REPROJECT_Success != ostn15GCS->XYZFromLatLong(xyzOSGB1, latLongOSGB));
    }

/*---------------------------------------------------------------------------------**//**
* Domain tests
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGCSUnitTests, LatLongToFromXYZForBritishNatGrid)
{
    GeoCoordinates::BaseGCSPtr BNGGCS;
    GeoCoordinates::BaseGCSPtr wgs84GCS;

    // First test using WGS84 lat/long
    BNGGCS = GeoCoordinates::BaseGCS::CreateGCS("BritishNatGrid");
    BNGGCS->SetReprojectElevation(true);
    BNGGCS->SetVerticalDatumCode(GeoCoordinates::vdcLocalEllipsoid);

    wgs84GCS = GeoCoordinates::BaseGCS::CreateGCS("LL84");
    wgs84GCS->SetReprojectElevation(true);

    // Verify that the round trip in OSGB works
    GeoPoint latLongOSGB;
    latLongOSGB.longitude = -0.01;
    latLongOSGB.latitude = 51.5;
    latLongOSGB.elevation = 0.0;

    DPoint3d xyzOSGB1 = { 0.0, 0.0, 0.0 };

    // Since conversion to WGS is already local ellipsoid no vertical shift needed and this call will succeed.
    EXPECT_TRUE(REPROJECT_Success == BNGGCS->XYZFromLatLong(xyzOSGB1, latLongOSGB));

    EXPECT_TRUE(REPROJECT_Success == BNGGCS->LatLongFromXYZ(latLongOSGB, xyzOSGB1));

    EXPECT_NEAR(latLongOSGB.longitude, -0.01, 0.00001);
    EXPECT_NEAR(latLongOSGB.latitude, 51.5, 0.00001);
    EXPECT_NEAR(latLongOSGB.elevation, 0.0, 0.001);

    // Verify that the XYZ differ between WGS84 and BritishNatGrid
    GeoPoint latLongWGS84;

    latLongWGS84.longitude = -0.01;
    latLongWGS84.latitude = 51.5;
    latLongWGS84.elevation = 0.0;

    DPoint3d xyzWGS84_1 = { 0.0, 0.0, 0.0 };
    EXPECT_TRUE(REPROJECT_Success == wgs84GCS->XYZFromLatLong(xyzWGS84_1, latLongWGS84));

    EXPECT_TRUE(REPROJECT_Success == wgs84GCS->LatLongFromXYZ(latLongWGS84, xyzWGS84_1));

    // Round trip will yield the same result.
    EXPECT_NEAR(latLongWGS84.longitude, -0.01, 0.00001);
    EXPECT_NEAR(latLongWGS84.latitude, 51.5, 0.00001);
    EXPECT_NEAR(latLongWGS84.elevation, 0.0, 0.001);

    // OSGB and WGS will have different values since they use different ellipsoids and not same location on Earth
    EXPECT_NE(xyzWGS84_1.x, xyzOSGB1.x);
    EXPECT_NE(xyzWGS84_1.y, xyzOSGB1.y);
    EXPECT_NE(xyzWGS84_1.z, xyzOSGB1.z);

    double distance = sqrt((xyzOSGB1.x - xyzWGS84_1.x) * (xyzOSGB1.x - xyzWGS84_1.x) + (xyzOSGB1.y - xyzWGS84_1.y) * (xyzOSGB1.y - xyzWGS84_1.y) + (xyzOSGB1.z - xyzWGS84_1.z) * (xyzOSGB1.z - xyzWGS84_1.z));

    EXPECT_NE(distance, 0.0);

    // Starting with a lat/long coordinate in WGS84, compute XYZ in both BritishNatGrid and WGS84 and the values should differ.

    latLongWGS84.longitude = -0.01;
    latLongWGS84.latitude = 51.5;
    latLongWGS84.elevation = 0.0;
    GeoPoint latLongOSGB_2 = {0, 0, 0};

    EXPECT_TRUE(REPROJECT_Success == wgs84GCS->LatLongFromLatLong(latLongOSGB_2, latLongWGS84, *BNGGCS));

    // Latitude/longitude should be different
    EXPECT_NE(latLongWGS84.longitude, latLongOSGB_2.longitude);
    EXPECT_NE(latLongWGS84.latitude, latLongOSGB_2.latitude);
    EXPECT_TRUE(latLongWGS84.elevation == latLongOSGB_2.elevation);

    // Compute XYZ for both ...
    DPoint3d xyzOSGB_X = { 0.0, 0.0, 0.0 };
    DPoint3d xyzWGS84_X = { 0.0, 0.0, 0.0 };

    EXPECT_TRUE(REPROJECT_Success == BNGGCS->XYZFromLatLong(xyzOSGB_X, latLongOSGB_2));
    EXPECT_TRUE(REPROJECT_Success == wgs84GCS->XYZFromLatLong(xyzWGS84_X, latLongWGS84));

    // OSGB and WGS must have different XYZ values (but same location on Earth)
    EXPECT_NE(xyzWGS84_X.x, xyzOSGB_X.x);
    EXPECT_NE(xyzWGS84_X.y, xyzOSGB_X.y);
    EXPECT_NE(xyzWGS84_X.z, xyzOSGB_X.z);

    // Change vertical datum to WGS84 Ellipsoid
    BNGGCS->SetVerticalDatumCode(GeoCoordinates::vdcEllipsoid);

    // Thi call will cail because it would require conversion from WGSEllipsoid to local Ellipsoid and the method does not do that.
    EXPECT_TRUE(REPROJECT_Success != BNGGCS->XYZFromLatLong(xyzOSGB1, latLongOSGB));

    // Change vertical datum to Geoid
    BNGGCS->SetVerticalDatumCode(GeoCoordinates::vdcGeoid);

    // Thi call will cail because it would require conversion from WGSEllipsoid to local Ellipsoid prior to geoid and the method does not do that.
    EXPECT_TRUE(REPROJECT_Success != BNGGCS->XYZFromLatLong(xyzOSGB1, latLongOSGB));
}

/*---------------------------------------------------------------------------------**//**
* Domain tests
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGCSUnitTests, LatLongToFromXYZForUTM83)
{
    GeoCoordinates::BaseGCSPtr utm83GCS;
    GeoCoordinates::BaseGCSPtr wgs84GCS;

    utm83GCS = GeoCoordinates::BaseGCS::CreateGCS("UTM83-10");
    utm83GCS->SetReprojectElevation(true);
    utm83GCS->SetVerticalDatumCode(GeoCoordinates::vdcEllipsoid);

    wgs84GCS = GeoCoordinates::BaseGCS::CreateGCS("LL84");
    wgs84GCS->SetReprojectElevation(true);

    // Verify that the round trip in OSGB works
    GeoPoint latLongutm83;
    latLongutm83.longitude = -100.01;
    latLongutm83.latitude = 51.5;
    latLongutm83.elevation = 0.0;

    DPoint3d xyzUTM83 = { 0.0, 0.0, 0.0 };

    EXPECT_TRUE(REPROJECT_Success == utm83GCS->XYZFromLatLong(xyzUTM83, latLongutm83));

    EXPECT_TRUE(REPROJECT_Success == utm83GCS->LatLongFromXYZ(latLongutm83, xyzUTM83));

    EXPECT_NEAR(latLongutm83.longitude, -100.01, 0.00001);
    EXPECT_NEAR(latLongutm83.latitude, 51.5, 0.00001);
    EXPECT_NEAR(latLongutm83.elevation, 0.0, 0.001);

    // Verify that the XYZ differ between WGS84 and OSGB15
    GeoPoint latLongWGS84;

    latLongWGS84.longitude = -100.01;
    latLongWGS84.latitude = 51.5;
    latLongWGS84.elevation = 0.0;

    DPoint3d xyzWGS84_1 = { 0.0, 0.0, 0.0 };
    EXPECT_TRUE(REPROJECT_Success == wgs84GCS->XYZFromLatLong(xyzWGS84_1, latLongWGS84));

    EXPECT_TRUE(REPROJECT_Success == wgs84GCS->LatLongFromXYZ(latLongWGS84, xyzWGS84_1));

    // Round trip will yield the same result.
    EXPECT_NEAR(latLongWGS84.longitude, -100.01, 0.00001);
    EXPECT_NEAR(latLongWGS84.latitude, 51.5, 0.00001);
    EXPECT_NEAR(latLongWGS84.elevation, 0.0, 0.001);

    // OSGB and WGS will have different values since they use different ellipsoids and not same location on Earth
    EXPECT_NEAR(xyzWGS84_1.x, xyzUTM83.x, 0.001);
    EXPECT_NEAR(xyzWGS84_1.y, xyzUTM83.y, 0.001);
    EXPECT_NEAR(xyzWGS84_1.z, xyzUTM83.z, 0.001);

    double distance = sqrt((xyzUTM83.x - xyzWGS84_1.x) * (xyzUTM83.x - xyzWGS84_1.x) + (xyzUTM83.y - xyzWGS84_1.y) * (xyzUTM83.y - xyzWGS84_1.y) + (xyzUTM83.z - xyzWGS84_1.z) * (xyzUTM83.z - xyzWGS84_1.z));

    EXPECT_NEAR(distance, 0.0, 0.001);

    // Starting with a lat/long coordinate in WGS84, compute XYZ in both OSGB and WGS84 and the values should differ.

    latLongWGS84.longitude = -100.01;
    latLongWGS84.latitude = 51.5;
    latLongWGS84.elevation = 0.0;
    GeoPoint latLongUTM83_2 = {0, 0, 0};

    EXPECT_TRUE(REPROJECT_Success == wgs84GCS->LatLongFromLatLong(latLongUTM83_2, latLongWGS84, *utm83GCS));

    // Latitude/longitude should be different
    EXPECT_NEAR(latLongWGS84.longitude, latLongUTM83_2.longitude, 0.000001);
    EXPECT_NEAR(latLongWGS84.latitude, latLongUTM83_2.latitude, 0.000001);
    EXPECT_NEAR(latLongWGS84.elevation, latLongUTM83_2.elevation, 0.001);

    // Compute XYZ for both ...
    DPoint3d xyzUTM83_X = { 0.0, 0.0, 0.0 };
    DPoint3d xyzWGS84_X = { 0.0, 0.0, 0.0 };

    EXPECT_TRUE(REPROJECT_Success == utm83GCS->XYZFromLatLong(xyzUTM83_X, latLongUTM83_2));
    EXPECT_TRUE(REPROJECT_Success == wgs84GCS->XYZFromLatLong(xyzWGS84_X, latLongWGS84));

    // NAD83 and WGS must have smae XYZ values
    EXPECT_NEAR(xyzWGS84_X.x, xyzUTM83_X.x, 0.001);
    EXPECT_NEAR(xyzWGS84_X.y, xyzUTM83_X.y, 0.001);
    EXPECT_NEAR(xyzWGS84_X.z, xyzUTM83_X.z, 0.001);
}

/*---------------------------------------------------------------------------------**//**
* Domain tests
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, XYZFromLatLong1)
    {
    GeoCoordinates::BaseGCSPtr wgs84GCS;

    // First test using WGS84 lat/long
    wgs84GCS = GeoCoordinates::BaseGCS::CreateGCS("LL84");
    wgs84GCS->SetReprojectElevation(true);

    GeoPoint latLongWGS84;

    latLongWGS84.longitude = -0.79301653077468948;
    latLongWGS84.latitude = 51.983781739290649;
    latLongWGS84.elevation = 0.0;

    DPoint3d xyzWGS84_1 = {0.0, 0.0, 0.0};
    EXPECT_TRUE(REPROJECT_Success == wgs84GCS->XYZFromLatLong(xyzWGS84_1, latLongWGS84));

    EXPECT_TRUE(REPROJECT_Success == wgs84GCS->LatLongFromXYZ(latLongWGS84, xyzWGS84_1));

    // Round trip will yield the same result.
    EXPECT_NEAR(latLongWGS84.longitude, -0.79301653077468948, 0.00001);
    EXPECT_NEAR(latLongWGS84.latitude, 51.983781739290649, 0.00001);
    EXPECT_NEAR(latLongWGS84.elevation, 0.0, 0.001);

    // Test exact value as extracted from http://www.sysense.com/products/ecef_lla_converter/index.html
    // and https://www.oc.nps.edu/oc2902w/coord/llhxyz.htm
    EXPECT_NEAR(xyzWGS84_1.x, 3936005.29, 0.01);
    EXPECT_NEAR(xyzWGS84_1.y, -54480.74, 0.01);
    EXPECT_NEAR(xyzWGS84_1.z, 5001692.15, 0.01);
    }

/*---------------------------------------------------------------------------------**//**
* Domain tests
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, XRail09Test)
    {
    GeoCoordinates::BaseGCSPtr XRail09_2015GCS;
    GeoCoordinates::BaseGCSPtr XRail09_2002GCS;
    GeoCoordinates::BaseGCSPtr wgs84GCS;

    // First test using WGS84 lat/long
    XRail09_2015GCS = GeoCoordinates::BaseGCS::CreateGCS("XRail09_Snake_2015");
    XRail09_2015GCS->SetReprojectElevation(true);

    XRail09_2002GCS = GeoCoordinates::BaseGCS::CreateGCS("XRail09_Snake_2002");
    XRail09_2002GCS->SetReprojectElevation(true);

    wgs84GCS = GeoCoordinates::BaseGCS::CreateGCS("LL84");
    wgs84GCS->SetReprojectElevation(true);

    DPoint3d XRail09Coordinates = {0,0,0};
    DPoint3d WGS84Coordinates;
    WGS84Coordinates.Init(-0.0000001701, 51.4999999820, 0.0);

    ASSERT_TRUE(REPROJECT_Success == wgs84GCS->CartesianFromCartesian(XRail09Coordinates, WGS84Coordinates, *XRail09_2015GCS));

    EXPECT_NEAR(XRail09Coordinates.x, 101200.000, 0.001);
    EXPECT_NEAR(XRail09Coordinates.y, 85000.000, 0.001);
    EXPECT_NEAR(XRail09Coordinates.z, 0.0, 0.001);

    WGS84Coordinates.Init(0.0, 51.50, 0.0);

    ASSERT_TRUE(REPROJECT_Success == wgs84GCS->CartesianFromCartesian(XRail09Coordinates, WGS84Coordinates, *XRail09_2002GCS));

    EXPECT_NEAR(XRail09Coordinates.x, 101200.000, 0.001);
    EXPECT_NEAR(XRail09Coordinates.y, 85000.000, 0.001);
    EXPECT_NEAR(XRail09Coordinates.z, 0.0, 0.001);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, KuwaitUtilityInstanciationFailureTest)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS("KuwaitUtility.KTM");

    EXPECT_TRUE(currentGCS.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, InitFromEPSGFailureTest)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS();

    EXPECT_TRUE(SUCCESS == currentGCS->InitFromEPSGCode(NULL, NULL, 4326));

    EXPECT_TRUE(currentGCS.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, InitFromEPSG31370FailureTest)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS();

    EXPECT_TRUE(SUCCESS == currentGCS->InitFromEPSGCode(NULL, NULL, 31370));
    EXPECT_TRUE(currentGCS.IsValid());

    EXPECT_TRUE(!currentGCS->IsDeprecated());
    }

/*---------------------------------------------------------------------------------**//**
* Specific LondonGrid WKT generation
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, SpecificWKTGenerationLondonGrid)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS("LondonGrid");

    Utf8String wellKnownText = "";

    EXPECT_TRUE(SUCCESS == currentGCS->GetWellKnownText(wellKnownText, GeoCoordinates::BaseGCS::wktFlavorOGC, false));

    EXPECT_TRUE(currentGCS->IsValid());
    }

   /*---------------------------------------------------------------------------------**//**
* WKT Generation for Lambert AF and TRMER AF GCS
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, WKTGenerationForWithAffineProjectionMethods)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;
    GeoCoordinates::BaseGCSPtr resultGCS;

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS("Shell-Appomattox");

    Utf8String wellKnownText = "";

    EXPECT_TRUE(SUCCESS == currentGCS->GetWellKnownText(wellKnownText, GeoCoordinates::BaseGCS::wktFlavorOGC, false));

    resultGCS = GeoCoordinates::BaseGCS::CreateGCS();

    EXPECT_TRUE(SUCCESS == resultGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, wellKnownText.c_str()));
    EXPECT_TRUE(resultGCS->IsEquivalent(*currentGCS));
    EXPECT_TRUE(SUCCESS == resultGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorAutodesk, wellKnownText.c_str()));
    EXPECT_TRUE(resultGCS->IsEquivalent(*currentGCS));

    // Change to Lambert AF
    currentGCS->SetProjectionCode(GeoCoordinates::BaseGCS::pcvLambertConformalConicAffinePostProcess);
    currentGCS->SetStandardParallel1(20.0);
    currentGCS->SetStandardParallel2(30.0);
    currentGCS->SetCentralMeridian(25.0);
    currentGCS->SetOriginLatitude(23.0);

    currentGCS->DefinitionComplete();

    EXPECT_TRUE(SUCCESS == currentGCS->GetWellKnownText(wellKnownText, GeoCoordinates::BaseGCS::wktFlavorOGC, false));

    EXPECT_TRUE(SUCCESS == resultGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, wellKnownText.c_str()));
    EXPECT_TRUE(resultGCS->IsEquivalent(*currentGCS));
    EXPECT_TRUE(SUCCESS == resultGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorAutodesk, wellKnownText.c_str()));
    EXPECT_TRUE(resultGCS->IsEquivalent(*currentGCS));
    }

/*---------------------------------------------------------------------------------**//**
* Creation of a GCS from a WKT then creation of geotiff keys
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, WKTToGeoKeys)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS();

    EXPECT_TRUE(currentGCS.IsValid());

    EXPECT_EQ(SUCCESS, currentGCS->InitFromWellKnownText(
        nullptr, nullptr, GeoCoordinates::BaseGCS::WktFlavor::wktFlavorOGC,
        "PROJCS[\"WGS 84 / UTM zone 10N\",GEOGCS[\"LL84\",DATUM[\"WGS84\",SPHEROID[\"WGS84\",6378137.000,298.25722293]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]],PROJECTION[\"Transverse Mercator\"],PARAMETER[\"False Easting\",500000.000],PARAMETER[\"False Northing\",0.000],PARAMETER[\"Scale Reduction\",0.999600000000],PARAMETER[\"Central Meridian\",-123.00000000000003],PARAMETER[\"Origin Latitude\",0.00000000000000],UNIT[\"Meter\",1.00000000000000]]"
    ));
    ASSERT_TRUE(currentGCS->IsValid());

    GeoCoordinates::BaseGeoTiffKeysList keyList;

    EXPECT_EQ(SUCCESS, currentGCS->GetGeoTiffKeys(&keyList, false));

    // Add manually those we need but are not added by the GetGeotiffKeys
    keyList.AddKey(1025, (uint32_t)1);
    keyList.AddKey(1026, currentGCS->GetDescription());
    keyList.AddKey(2049, currentGCS->GetDatumName());
    int unitEPSG = currentGCS->GetEPSGUnitCode();
    if (unitEPSG > 0)
        keyList.AddKey(3076, (uint32_t)unitEPSG);

    bool GTRasterTypeGeoKeyPresent = false;
    bool GTModelTypeGeoKeyPresent = false;
    bool ProjectedCSTypeGeoKeyPresent = false;
    bool GTCitationGeoKeyPresent = false;
    bool GeogCitationGeoKeyPresent = false;
    bool ProjLinearUnitsGeoKey = (unitEPSG == 0); // If epsg for unit is zero then it will not be present

    IGeoTiffKeysList::GeoKeyItem theCurrentKey;
    if (keyList.GetFirstKey(&theCurrentKey))
        {
        do
            {
            if ((theCurrentKey.KeyID == 1024) && (theCurrentKey.KeyDataType == IGeoTiffKeysList::LONG) && (theCurrentKey.KeyValue.LongVal == 1))
                GTModelTypeGeoKeyPresent = true;
            if ((theCurrentKey.KeyID == 1025) && (theCurrentKey.KeyDataType == IGeoTiffKeysList::LONG) && (theCurrentKey.KeyValue.LongVal == 1))
                GTRasterTypeGeoKeyPresent = true;
            if ((theCurrentKey.KeyID == 3072) && (theCurrentKey.KeyDataType == IGeoTiffKeysList::LONG) && (theCurrentKey.KeyValue.LongVal == 32610))
                ProjectedCSTypeGeoKeyPresent = true;
            if ((theCurrentKey.KeyID == 1026) && (theCurrentKey.KeyDataType == IGeoTiffKeysList::ASCII) && (Utf8String(theCurrentKey.KeyValue.StringVal) == Utf8String(currentGCS->GetDescription())))
                GTCitationGeoKeyPresent = true;
            if ((theCurrentKey.KeyID == 2049) && (theCurrentKey.KeyDataType == IGeoTiffKeysList::ASCII) && (Utf8String(theCurrentKey.KeyValue.StringVal) == Utf8String(currentGCS->GetDatumName())))
                GeogCitationGeoKeyPresent = true;
            if ((theCurrentKey.KeyID == 3076) && (theCurrentKey.KeyDataType == IGeoTiffKeysList::LONG) && (theCurrentKey.KeyValue.LongVal == unitEPSG))
                ProjLinearUnitsGeoKey = true;
            } while(keyList.GetNextKey(&theCurrentKey));
        }
    EXPECT_TRUE(GTModelTypeGeoKeyPresent);
    EXPECT_TRUE(GTRasterTypeGeoKeyPresent);
    EXPECT_TRUE(ProjectedCSTypeGeoKeyPresent);
    EXPECT_TRUE(GTCitationGeoKeyPresent);
    EXPECT_TRUE(GeogCitationGeoKeyPresent);
    EXPECT_TRUE(ProjLinearUnitsGeoKey);
    }

/*---------------------------------------------------------------------------------**//**
* Creation of a GCS from a WKT then creation of geotiff keys
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, WKTToGeoKeys2)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS();

    EXPECT_TRUE(currentGCS.IsValid());

    EXPECT_EQ(SUCCESS, currentGCS->InitFromWellKnownText(
        nullptr, nullptr, GeoCoordinates::BaseGCS::WktFlavor::wktFlavorUnknown,
        R"(PROJCS["UTM Zone 18, Northern Hemisphere",GEOGCS["NAD27",DATUM["North_American_Datum_1927",SPHEROID["Clarke 1866",6378206.4,294.978698213898,AUTHORITY["EPSG","7008"]],AUTHORITY["EPSG","6267"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9108"]],AUTHORITY["EPSG","4267"]],PROJECTION["Transverse_Mercator"],PARAMETER["latitude_of_origin",0],PARAMETER["central_meridian",-75],PARAMETER["scale_factor",0.9996],PARAMETER["false_easting",500000],PARAMETER["false_northing",0],UNIT["METERS",1]])"
      ));

    EXPECT_TRUE(currentGCS->IsValid());

    GeoCoordinates::BaseGeoTiffKeysList keyList;

    EXPECT_EQ(SUCCESS, currentGCS->GetGeoTiffKeys(&keyList, false));

    // Add manually those we need but are not added by the GetGeotiffKeys
    keyList.AddKey(1025, (uint32_t)1);
    keyList.AddKey(1026, Utf8String(currentGCS->GetDescription()).c_str());
    keyList.AddKey(2049, Utf8String(currentGCS->GetDatumName()).c_str());
    int unitEPSG = currentGCS->GetEPSGUnitCode();
    if (unitEPSG > 0)
        keyList.AddKey(3076, (uint32_t)unitEPSG);

    bool GTRasterTypeGeoKeyPresent = false;
    bool GTModelTypeGeoKeyPresent = false;
    bool ProjectedCSTypeGeoKeyPresent = false;
    bool GTCitationGeoKeyPresent = false;
    bool GeogCitationGeoKeyPresent = false;
    bool ProjLinearUnitsGeoKey = (unitEPSG == 0); // If epsg for unit is zero then it will not be present

    IGeoTiffKeysList::GeoKeyItem theCurrentKey;
    if (keyList.GetFirstKey(&theCurrentKey))
        {
        do
            {
            if ((theCurrentKey.KeyID == 1024) && (theCurrentKey.KeyDataType == IGeoTiffKeysList::LONG) && (theCurrentKey.KeyValue.LongVal == 1))
                GTModelTypeGeoKeyPresent = true;
            if ((theCurrentKey.KeyID == 1025) && (theCurrentKey.KeyDataType == IGeoTiffKeysList::LONG) && (theCurrentKey.KeyValue.LongVal == 1))
                GTRasterTypeGeoKeyPresent = true;
            if ((theCurrentKey.KeyID == 3072) && (theCurrentKey.KeyDataType == IGeoTiffKeysList::LONG) && (theCurrentKey.KeyValue.LongVal == 26718))
                ProjectedCSTypeGeoKeyPresent = true;
            if ((theCurrentKey.KeyID == 1026) && (theCurrentKey.KeyDataType == IGeoTiffKeysList::ASCII) && (Utf8String(theCurrentKey.KeyValue.StringVal) == Utf8String(currentGCS->GetDescription())))
                GTCitationGeoKeyPresent = true;
            if ((theCurrentKey.KeyID == 2049) && (theCurrentKey.KeyDataType == IGeoTiffKeysList::ASCII) && (Utf8String(theCurrentKey.KeyValue.StringVal) == Utf8String(currentGCS->GetDatumName())))
                GeogCitationGeoKeyPresent = true;
            if ((theCurrentKey.KeyID == 3076) && (theCurrentKey.KeyDataType == IGeoTiffKeysList::LONG) && (theCurrentKey.KeyValue.LongVal == unitEPSG))
                ProjLinearUnitsGeoKey = true;
            } while(keyList.GetNextKey(&theCurrentKey));
        }
    EXPECT_TRUE(GTModelTypeGeoKeyPresent);
    EXPECT_TRUE(GTRasterTypeGeoKeyPresent);
    EXPECT_TRUE(ProjectedCSTypeGeoKeyPresent);
    EXPECT_TRUE(GTCitationGeoKeyPresent);
    EXPECT_TRUE(GeogCitationGeoKeyPresent);
    EXPECT_TRUE(ProjLinearUnitsGeoKey);
    }

/*---------------------------------------------------------------------------------**//**
* Test correction of vertical datum to preserve legacy elevation for a GCS created
* from a WKT that does not contain a vertical datum
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGCSUnitTests, ElevationCorrectionForNonCompoundCS_Test)
{
    Utf8String srcWKT = "PROJCS[\"NAD83 / Texas North Cen\",GEOGCS[\"NAD83\",DATUM[\"NAD83\",SPHEROID[\"GRS1980\",6378137.000,298.25722210]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]],PROJECTION[\"Lambert Conformal Conic, Two Standard Parallels\"],PARAMETER[\"False Easting\",1968500.000],PARAMETER[\"False Northing\",6561666.667],PARAMETER[\"Central Meridian\",-98.50000000000003],PARAMETER[\"Origin Latitude\",31.66666666666668],PARAMETER[\"Northern Standard Parallel\",33.96666666666668],PARAMETER[\"Southern Standard Parallel\",32.13333333333334],UNIT[\"Foot\",0.30480060960122]]";

    GeoCoordinates::BaseGCSPtr srcGCS = GeoCoordinates::BaseGCS::CreateGCS();
    srcGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, srcWKT.c_str());

    GeoCoordinates::BaseGCSPtr targetGCS = GeoCoordinates::BaseGCS::CreateGCS("LL84");

    Transform transformNoElevation;
    Transform transformWithElevation;

    DRange3d extent = DRange3d::From(2411760.9129999126, 7016097.8990061926, 350.00000000000000, 2532534.5129998885, 7057793.8990063900, 990.00000000000000);
    DPoint3d midPoint = DPoint3d::From((extent.high.x + extent.low.x) / 2, (extent.high.y + extent.low.y) / 2, (extent.high.z + extent.low.z) / 2);

    srcGCS->SetReprojectElevation(false);
    EXPECT_TRUE(REPROJECT_Success == srcGCS->GetLinearTransform(&transformNoElevation, extent, *targetGCS, nullptr, nullptr));

    srcGCS->SetReprojectElevation(true);
    EXPECT_TRUE(SUCCESS == srcGCS->CorrectVerticalDatumToPreserveLegacyElevation(extent, *targetGCS));

    // make sure the compound WKT has a vertical datum set to "Ellipsoid"
    Utf8String correctedWKT = "";
    srcGCS->GetCompoundCSWellKnownText(correctedWKT, GeoCoordinates::BaseGCS::wktFlavorOGC, false);
    ASSERT_TRUE(correctedWKT.ContainsI("VERT_CS[\"Ellipsoid Height\",VERT_DATUM[\"Ellipsoid\",2002]")) << correctedWKT.c_str();

    EXPECT_TRUE(REPROJECT_Success == srcGCS->GetLinearTransform(&transformWithElevation, extent, *targetGCS, nullptr, nullptr));

    DPoint3d transformedPtNoElevation, transformedPtWithElevation;
    transformNoElevation.Multiply(transformedPtNoElevation, midPoint);
    transformWithElevation.Multiply(transformedPtWithElevation, midPoint);

    // make sure the elevation is the same as before when no elevation was used in the reprojection
    EXPECT_NEAR(transformedPtNoElevation.x, transformedPtWithElevation.x, 1e-6);
    EXPECT_NEAR(transformedPtNoElevation.y, transformedPtWithElevation.y, 1e-6);
    EXPECT_NEAR(transformedPtNoElevation.z, transformedPtWithElevation.z, 1e-6);
}


/*---------------------------------------------------------------------------------**//**
* Test using a API defined self contained GCS
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGCSUnitTests, CreateAFullySelfContainedGCS_Test1)
{
    GeoCoordinates::BaseGCSPtr theGCS = GeoCoordinates::BaseGCS::CreateGCS("LL84");

    ASSERT_TRUE(theGCS.IsValid() && theGCS->IsValid());

    GeoCoordinates::DatumP theDatum = const_cast<GeoCoordinates::DatumP>(GeoCoordinates::Datum::CreateDatum("WGS84"));

    GeoCoordinates::EllipsoidP theEllipsoid = const_cast<GeoCoordinates::EllipsoidP>(GeoCoordinates::Ellipsoid::CreateEllipsoid("WGS84"));

    theEllipsoid->SetName("CustomEllipsoid1");
    theEllipsoid->SetDescription("Custom Ellipsoid Description");
    theEllipsoid->SetSource("CustomEllipsoid Source");
    theEllipsoid->SetEPSGCode(0);
    theEllipsoid->SetGroup("");
    theEllipsoid->SetPolarRadius(6356752.31418888); // WGS84 is 6356752.3142 so 12 microns off
    theEllipsoid->SetEquatorialRadius(6378137.000012); // WGS84 is 6378137.0 so 12 microns off

    theDatum->SetEllipsoid(theEllipsoid); // This copies the given ellipsoid so it must still be destroyed

    // Now we modify the datum
    theDatum->SetName("CustomDatum1");
    theDatum->SetDescription("Custom Datum Description");
    theDatum->SetSource("Custom Datum Source");
    theDatum->SetConvertToWGS84MethodCode(ConvertType_7PARM);
    DPoint3d delta;
    delta.x = 23.1;
    delta.y = 22.4;
    delta.z = -56.1;
    DPoint3d rotation;
    rotation.x = -2.34;
    rotation.y = 0.001;
    rotation.z = 3.14;
    theDatum->SetDelta(delta);
    theDatum->SetRotation(rotation);
    theDatum->SetScale(0.001);
    theDatum->SetEPSGCode(0);
    theDatum->SetGroup("");

    EXPECT_TRUE(theDatum->GetEllipsoidCode() == -2);

    EXPECT_TRUE(SUCCESS == theGCS->SetName("CustomGCS1"));
    EXPECT_TRUE(SUCCESS == theGCS->SetDescription("Custom GCS Description"));
    EXPECT_TRUE(SUCCESS == theGCS->SetSource("Custom CRS Source"));
    EXPECT_TRUE(SUCCESS == theGCS->SetStoredEPSGCode(0));

    EXPECT_TRUE(SUCCESS == theGCS->SetProjectionCode(GeoCoordinates::BaseGCS::pcvTransverseMercator));

// TODO if units are degree shift them to meters
    EXPECT_TRUE(SUCCESS == theGCS->SetFalseEasting(1000.1));
    EXPECT_TRUE(SUCCESS == theGCS->SetFalseNorthing(2000.1));
    EXPECT_TRUE(SUCCESS == theGCS->SetCentralMeridian(-71.24));
    EXPECT_TRUE(SUCCESS == theGCS->SetOriginLatitude(46.48));
    EXPECT_TRUE(SUCCESS == theGCS->SetScaleReduction(0.9995));

    theGCS->SetDatum(theDatum);
    theGCS->DefinitionComplete();

    EXPECT_TRUE(theGCS->GetDatumCode() == GeoCoordinates::Datum::CUSTOM_DATUM_CODE);

    GeoCoordinates::BaseGCSPtr otherGCS = GeoCoordinates::BaseGCS::CreateGCS("LL84");

    DPoint3d inPoint, outPoint;
    inPoint.x = -71.0;
    inPoint.y = 46.0;
    inPoint.z = 0.0;

    EXPECT_TRUE(otherGCS->CartesianFromCartesian(outPoint, inPoint, *theGCS) == REPROJECT_Success);
}

/*---------------------------------------------------------------------------------**//**
* Test using a JSON defined self contained GCS
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGCSUnitTests, CreateAFullySelfContainedGCS_Test2)
{
Utf8String customDatum1A =    "    {"
        "      \"id\": \"CustomDatum1A\","
        "      \"description\": \"Custom Datum 1A Description\","
        "      \"deprecated\": false,"
        "      \"source\": \"Custom Datum 1A Source\","
        "      \"epsg\": 0,"
        "      \"ellipsoidId\": \"ANS66\","
        "      \"ellipsoid\" : {"
        "        \"id\": \"ANS66\","
        "        \"description\": \"Australian National Spheroid of 1966\","
        "        \"source\": \"US Defense Mapping Agency, TR-8350.2-B, December 1987\","
        "        \"equatorialRadius\": 6378160.0,"
        "        \"polarRadius\": 6356774.719195305951"
        "      },"
        "      \"transforms\": ["
        "        {"
        "          \"method\": \"PositionalVector\","
        "          \"geocentric\": null,"
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
        "          },"
        "          \"gridFile\": null"
        "        }"
        "      ]"
        "    }";


    GeoCoordinates::BaseGCSPtr theGCS = GeoCoordinates::BaseGCS::CreateGCS("LL84");

    ASSERT_TRUE(theGCS.IsValid() && theGCS->IsValid());

    GeoCoordinates::DatumP theDatum = const_cast<GeoCoordinates::DatumP>(GeoCoordinates::Datum::CreateDatum());

    Utf8String errorMessage;
    ASSERT_TRUE(SUCCESS == theDatum->FromJson(BeJsDocument(customDatum1A), errorMessage));

    EXPECT_TRUE(SUCCESS == theGCS->SetName("CustomGCS1"));
    EXPECT_TRUE(SUCCESS == theGCS->SetDescription("Custom GCS Description"));
    EXPECT_TRUE(SUCCESS == theGCS->SetSource("Custom CRS Source"));
    EXPECT_TRUE(SUCCESS == theGCS->SetStoredEPSGCode(0));

    EXPECT_TRUE(SUCCESS == theGCS->SetProjectionCode(GeoCoordinates::BaseGCS::pcvTransverseMercator));

    // TODO if units are degree shift them to meters
    EXPECT_TRUE(SUCCESS == theGCS->SetFalseEasting(1000.1));
    EXPECT_TRUE(SUCCESS == theGCS->SetFalseNorthing(2000.1));
    EXPECT_TRUE(SUCCESS == theGCS->SetCentralMeridian(142.0));
    EXPECT_TRUE(SUCCESS == theGCS->SetOriginLatitude(-36.1441)); // Jeparit Australia
    EXPECT_TRUE(SUCCESS == theGCS->SetScaleReduction(0.9995));

    theGCS->SetDatum(theDatum);
    theGCS->DefinitionComplete();

    EXPECT_TRUE(theGCS->GetDatumCode() == GeoCoordinates::Datum::CUSTOM_DATUM_CODE);

    GeoCoordinates::BaseGCSPtr otherGCS = GeoCoordinates::BaseGCS::CreateGCS("LL84");

    DPoint3d inPoint, outPoint;
    inPoint.x = 142.0;
    inPoint.y = -36.1441;
    inPoint.z = 0.0;

    EXPECT_TRUE(otherGCS->CartesianFromCartesian(outPoint, inPoint, *theGCS) == REPROJECT_Success);
}

/*---------------------------------------------------------------------------------**//**
* Test using a JSON defined self contained GCS that uses grid shift files
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGCSUnitTests, CreateAFullySelfContainedGCS_Test3)
{
Utf8String customDatum2  =  "    {"
        "      \"id\": \"CustomDatum2\","
        "      \"description\": \"Custom Datum 2 Description\","
        "      \"deprecated\": false,"
        "      \"source\": \"Custom Datum 2 Source\","
        "      \"epsg\": 0,"
        "      \"ellipsoidId\": \"GRS1980\","
        "      \"ellipsoid\" : {"
        "        \"id\": \"GRS1980\","
        "        \"description\": \"Australian National Spheroid of 1966\","
        "        \"source\": \"US Defense Mapping Agency, TR-8350.2-B, December 1987\","
        "        \"equatorialRadius\": 6378160.0,"
        "        \"polarRadius\": 6356774.719195305951"
        "      },"
        "      \"transforms\": ["
        "        {"
        "          \"method\": \"GridFiles\","
        "          \"geocentric\": null,"
        "          \"positionalVector\": null,"
        "          \"gridFile\": {"
        "            \"files\": ["
        "              {"
        "                \"fileName\": \"./Australia/GDA94_GDA2020_conformal_and_distortion.gsb\","
        "                \"format\": \"NTv2\","
        "                \"direction\": \"Direct\""
        "              }"
        "            ]"
        "          }"
        "        }"
        "      ]"
        "    }";

    GeoCoordinates::BaseGCSPtr theGCS = GeoCoordinates::BaseGCS::CreateGCS("LL84");

    ASSERT_TRUE(theGCS.IsValid() && theGCS->IsValid());

    GeoCoordinates::DatumP theDatum = const_cast<GeoCoordinates::DatumP>(GeoCoordinates::Datum::CreateDatum());

    Utf8String errorMessage;
    ASSERT_TRUE(SUCCESS == theDatum->FromJson(BeJsDocument(customDatum2), errorMessage));

    EXPECT_TRUE(SUCCESS == theGCS->SetName("CustomGCS1"));
    EXPECT_TRUE(SUCCESS == theGCS->SetDescription("Custom GCS Description"));
    EXPECT_TRUE(SUCCESS == theGCS->SetSource("Custom CRS Source"));
    EXPECT_TRUE(SUCCESS == theGCS->SetStoredEPSGCode(0));

    EXPECT_TRUE(SUCCESS == theGCS->SetProjectionCode(GeoCoordinates::BaseGCS::pcvTransverseMercator));

    // TODO if units are degree shift them to meters
    EXPECT_TRUE(SUCCESS == theGCS->SetFalseEasting(1000.1));
    EXPECT_TRUE(SUCCESS == theGCS->SetFalseNorthing(2000.1));
    EXPECT_TRUE(SUCCESS == theGCS->SetCentralMeridian(142.0));
    EXPECT_TRUE(SUCCESS == theGCS->SetOriginLatitude(-36.1441)); // Jeparit Australia
    EXPECT_TRUE(SUCCESS == theGCS->SetScaleReduction(0.9995));

    theGCS->SetDatum(theDatum);
    theGCS->SetVerticalDatumCode(GeoCoordinates::vdcGeoid);
    theGCS->DefinitionComplete();

    EXPECT_TRUE(theGCS->GetDatumCode() == GeoCoordinates::Datum::CUSTOM_DATUM_CODE);

    GeoCoordinates::BaseGCSPtr otherGCS = GeoCoordinates::BaseGCS::CreateGCS("LL84");

    DPoint3d inPoint, outPoint = {0,0,0};
    inPoint.x = 142.0;
    inPoint.y = -36.1441;
    inPoint.z = 0.0;

    EXPECT_TRUE(otherGCS->CartesianFromCartesian(outPoint, inPoint, *theGCS) == REPROJECT_Success);

    EXPECT_NEAR(outPoint.z, -2.8586293668882208, 0.001);
}

/*---------------------------------------------------------------------------------**//**
* Test using a JSON defined self contained Datum that uses grid shift files and custom ellipsoid
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGCSUnitTests, CreateAFullySelfContainedDatumWithCustomEllipsoid_Test)
{
    Utf8String customDatum3 = "    {"
        "      \"id\": \"CustomDatum3\","
        "      \"description\": \"Custom Datum 3 Description\","
        "      \"deprecated\": false,"
        "      \"source\": \"Custom Datum 3 Source\","
        "      \"epsg\": 0,"
        "      \"ellipsoidId\": \"CustomEllipsoid\","
        "      \"ellipsoid\" : {"
        "        \"id\": \"CustomEllipsoid\","
        "        \"description\": \"Custom ellipsoid description\","
        "        \"source\": \"Custom Ellipsoid source\","
        "        \"equatorialRadius\": 6378120.0,"
        "        \"polarRadius\": 6356794.719195305951"
        "      },"
        "      \"transforms\": ["
        "        {"
        "          \"method\": \"GridFiles\","
        "          \"geocentric\": null,"
        "          \"positionalVector\": null,"
        "          \"gridFile\": {"
        "            \"files\": ["
        "              {"
        "                \"fileName\": \"./Australia/GDA94_GDA2020_conformal_and_distortion.gsb\","
        "                \"format\": \"NTv2\","
        "                \"direction\": \"Direct\""
        "              }"
        "            ]"
        "          }"
        "        }"
        "      ]"
        "    }";

    GeoCoordinates::DatumP theDatum = const_cast<GeoCoordinates::DatumP>(GeoCoordinates::Datum::CreateDatum());

    Utf8String errorMessage;
    ASSERT_TRUE(SUCCESS == theDatum->FromJson(Json::Value::From(customDatum3), errorMessage));
    Utf8String source;

    EXPECT_TRUE(Utf8String(theDatum->GetName()) == "CustomDatum3");
    EXPECT_TRUE(Utf8String(theDatum->GetDescription()) == "Custom Datum 3 Description");
    EXPECT_TRUE(Utf8String(theDatum->GetSource(source)) == "Custom Datum 3 Source");

    GeoCoordinates::GeodeticTransformPathCP theStoredPath = theDatum->GetStoredGeodeticTransformPath();

    ASSERT_TRUE(theStoredPath != nullptr);
    EXPECT_TRUE(theStoredPath->GetGeodeticTransformCount() == 1);

    GeoCoordinates::GeodeticTransformCP theTransform = theStoredPath->GetGeodeticTransform(0);
    ASSERT_TRUE(theTransform != nullptr);

    EXPECT_TRUE(theTransform->IsValid());
    EXPECT_TRUE(!theTransform->IsDeprecated());
    EXPECT_TRUE(!theTransform->IsNullTransform());
    EXPECT_TRUE(Utf8String(theTransform->GetSourceDatumName()) == "CustomDatum3");
    EXPECT_TRUE(Utf8String(theTransform->GetTargetDatumName()) == "WGS84");
    EXPECT_TRUE(Utf8String(theTransform->GetName()) == "CustomDatum3_to_WGS84");
    EXPECT_TRUE(theTransform->GetAccuracy() == 1.0); // Default value
    EXPECT_TRUE(theTransform->GetEPSGCode() == 0); // Default value
    EXPECT_TRUE(theTransform->GetDataAvailability() == GeoCoordinates::GeodeticTransformDataAvailability::DataAvailable); // Availability resolved here
    EXPECT_TRUE(theTransform->GetConvertMethodCode() == GeoCoordinates::GenConvertCode::GenConvertType_GFILE);
    EXPECT_TRUE(theTransform->GetGridFileDefinitionCount() == 1);
    GeoCoordinates::GridFileDefinition theFileDef = theTransform->GetGridFileDefinition(0);
    // This also tests the forward/backward slashes conversion. We use UNIX style in API but allow Windows on input.
    EXPECT_TRUE(Utf8String(theFileDef.GetFileName()) == "./Australia/GDA94_GDA2020_conformal_and_distortion.gsb");
    EXPECT_TRUE(theFileDef.GetFormat() == GeoCoordinates::GridFileFormat::FORMAT_NTv2);
    EXPECT_TRUE(theFileDef.GetDirection() == GeoCoordinates::GridFileDirection::DIRECTION_DIRECT);

    GeoCoordinates::EllipsoidCP theEllipsoid = theDatum->GetEllipsoid();
    ASSERT_TRUE(theEllipsoid != nullptr);

    EXPECT_TRUE(Utf8String(theEllipsoid->GetName()) == "CustomEllipsoid");
    EXPECT_TRUE(Utf8String(theEllipsoid->GetDescription()) == "Custom ellipsoid description");
    EXPECT_TRUE(Utf8String(theEllipsoid->GetSource(source)) == "Custom Ellipsoid source");
    EXPECT_TRUE(theEllipsoid->GetEquatorialRadius() == 6378120.0);
    EXPECT_TRUE(theEllipsoid->GetPolarRadius() == 6356794.719195305951);

    bvector<Utf8String> listOfFiles;
    EXPECT_FALSE(theDatum->HasMissingGridFiles(listOfFiles));
    EXPECT_TRUE(listOfFiles.size() == 0);
    
    theDatum->Destroy();
}

/*---------------------------------------------------------------------------------**//**
* Test using a JSON defined self contained Datum that uses grid shift files and custom ellipsoid
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGCSUnitTests, CreateAFullySelfContainedDatumWithCustomEllipsoidButFileInexistant_Test1)
{
    Utf8String customDatum4 = "    {"
        "      \"id\": \"CustomDatum4\","
        "      \"description\": \"Custom Datum 4 Description\","
        "      \"deprecated\": false,"
        "      \"source\": \"Custom Datum 4 Source\","
        "      \"epsg\": 0,"
        "      \"ellipsoidId\": \"CustomEllipsoid\","
        "      \"ellipsoid\" : {"
        "        \"id\": \"CustomEllipsoid\","
        "        \"description\": \"Custom ellipsoid description\","
        "        \"source\": \"Custom Ellipsoid source\","
        "        \"equatorialRadius\": 6378120.0,"
        "        \"polarRadius\": 6356794.719195305951"
        "      },"
        "      \"transforms\": ["
        "        {"
        "          \"method\": \"GridFiles\","
        "          \"geocentric\": null,"
        "          \"positionalVector\": null,"
        "          \"gridFile\": {"
        "            \"files\": ["
        "              {"
        "                \"fileName\": \"./NoPath/InexistantFile.gsb\","
        "                \"format\": \"NTv2\","
        "                \"direction\": \"Direct\""
        "              }"
        "            ]"
        "          }"
        "        }"
        "      ]"
        "    }";

    GeoCoordinates::DatumP theDatum = const_cast<GeoCoordinates::DatumP>(GeoCoordinates::Datum::CreateDatum());

    Utf8String errorMessage;
    ASSERT_TRUE(SUCCESS == theDatum->FromJson(Json::Value::From(customDatum4), errorMessage));
    Utf8String source;


    EXPECT_TRUE(Utf8String(theDatum->GetName()) == "CustomDatum4");
    EXPECT_TRUE(Utf8String(theDatum->GetDescription()) == "Custom Datum 4 Description");
    EXPECT_TRUE(Utf8String(theDatum->GetSource(source)) == "Custom Datum 4 Source");

    GeoCoordinates::GeodeticTransformPathCP theStoredPath = theDatum->GetStoredGeodeticTransformPath();

    ASSERT_TRUE(theStoredPath != nullptr);
    EXPECT_TRUE(theStoredPath->GetGeodeticTransformCount() == 1);

    GeoCoordinates::GeodeticTransformCP theTransform = theStoredPath->GetGeodeticTransform(0);
    ASSERT_TRUE(theTransform != nullptr);

    EXPECT_TRUE(theTransform->IsValid());
    EXPECT_TRUE(!theTransform->IsDeprecated());
    EXPECT_TRUE(!theTransform->IsNullTransform());
    EXPECT_TRUE(Utf8String(theTransform->GetSourceDatumName()) == "CustomDatum4");
    EXPECT_TRUE(Utf8String(theTransform->GetTargetDatumName()) == "WGS84");
    EXPECT_TRUE(Utf8String(theTransform->GetName()) == "CustomDatum4_to_WGS84");
    EXPECT_TRUE(theTransform->GetAccuracy() == 1.0); // Default value
    EXPECT_TRUE(theTransform->GetEPSGCode() == 0); // Default value
    EXPECT_TRUE(theTransform->GetDataAvailability() == GeoCoordinates::GeodeticTransformDataAvailability::DataUnavailable); // Availability resolved here
    EXPECT_TRUE(theTransform->GetConvertMethodCode() == GeoCoordinates::GenConvertCode::GenConvertType_GFILE);
    EXPECT_TRUE(theTransform->GetGridFileDefinitionCount() == 1);
    GeoCoordinates::GridFileDefinition theFileDef = theTransform->GetGridFileDefinition(0);
    // This also tests the forward/backward slashes conversion. We use UNIX style in API but allow Windows on input.
    EXPECT_TRUE(Utf8String(theFileDef.GetFileName()) == "./NoPath/InexistantFile.gsb");
    EXPECT_TRUE(theFileDef.GetFormat() == GeoCoordinates::GridFileFormat::FORMAT_NTv2);
    EXPECT_TRUE(theFileDef.GetDirection() == GeoCoordinates::GridFileDirection::DIRECTION_DIRECT);

    GeoCoordinates::EllipsoidCP theEllipsoid = theDatum->GetEllipsoid();
    ASSERT_TRUE(theEllipsoid != nullptr);

    EXPECT_TRUE(Utf8String(theEllipsoid->GetName()) == "CustomEllipsoid");
    EXPECT_TRUE(Utf8String(theEllipsoid->GetDescription()) == "Custom ellipsoid description");
    EXPECT_TRUE(Utf8String(theEllipsoid->GetSource(source)) == "Custom Ellipsoid source");
    EXPECT_TRUE(theEllipsoid->GetEquatorialRadius() == 6378120.0);
    EXPECT_TRUE(theEllipsoid->GetPolarRadius() == 6356794.719195305951);

    bvector<Utf8String> listOfFiles;
    EXPECT_TRUE(theDatum->HasMissingGridFiles(listOfFiles));
    EXPECT_TRUE(listOfFiles.size() == 1);
    EXPECT_TRUE(listOfFiles[0] == "./NoPath/InexistantFile.gsb");
    
    theDatum->Destroy();
}

/*---------------------------------------------------------------------------------**//**
* Test using a JSON defined self contained Datum that uses grid shift files and custom ellipsoid
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/

TEST_F(BaseGCSUnitTests, CreateAFullySelfContainedDatumWithCustomEllipsoidButFileInexistant_Test2)
{
    Utf8String customDatum4 = "    {"
        "      \"id\": \"CustomDatum4\","
        "      \"description\": \"Custom Datum 4 Description\","
        "      \"deprecated\": false,"
        "      \"source\": \"Custom Datum 4 Source\","
        "      \"epsg\": 0,"
        "      \"ellipsoidId\": \"CustomEllipsoid\","
        "      \"ellipsoid\" : {"
        "        \"id\": \"CustomEllipsoid\","
        "        \"description\": \"Custom ellipsoid description\","
        "        \"source\": \"Custom Ellipsoid source\","
        "        \"equatorialRadius\": 6378120.0,"
        "        \"polarRadius\": 6356794.719195305951"
        "      },"
        "      \"transforms\": ["
        "        {"
        "          \"method\": \"GridFiles\","
        "          \"geocentric\": null,"
        "          \"positionalVector\": null,"
        "          \"gridFile\": {"
        "            \"files\": ["
        "              {"
        "                \"fileName\": \"./NoPath/InexistantFile.gsb\","
        "                \"format\": \"NTv2\","
        "                \"direction\": \"Direct\""
        "              }"
        "            ]"
        "          }"
        "        }"
        "      ]"
        "    }";

    GeoCoordinates::DatumP theDatum = const_cast<GeoCoordinates::DatumP>(GeoCoordinates::Datum::CreateDatum());

    Utf8String errorMessage;
    ASSERT_TRUE(SUCCESS == theDatum->FromJson(Json::Value::From(customDatum4), errorMessage));
    Utf8String source;


    EXPECT_TRUE(Utf8String(theDatum->GetName()) == "CustomDatum4");
    EXPECT_TRUE(Utf8String(theDatum->GetDescription()) == "Custom Datum 4 Description");
    EXPECT_TRUE(Utf8String(theDatum->GetSource(source)) == "Custom Datum 4 Source");

    GeoCoordinates::GeodeticTransformPathCP theStoredPath = theDatum->GetStoredGeodeticTransformPath();

    ASSERT_TRUE(theStoredPath != nullptr);
    EXPECT_TRUE(theStoredPath->GetGeodeticTransformCount() == 1);

    GeoCoordinates::GeodeticTransformCP theTransform = theStoredPath->GetGeodeticTransform(0);
    ASSERT_TRUE(theTransform != nullptr);

    EXPECT_TRUE(theTransform->IsValid());
    EXPECT_TRUE(!theTransform->IsDeprecated());
    EXPECT_TRUE(!theTransform->IsNullTransform());
    EXPECT_TRUE(Utf8String(theTransform->GetSourceDatumName()) == "CustomDatum4");
    EXPECT_TRUE(Utf8String(theTransform->GetTargetDatumName()) == "WGS84");
    EXPECT_TRUE(Utf8String(theTransform->GetName()) == "CustomDatum4_to_WGS84");
    EXPECT_TRUE(theTransform->GetAccuracy() == 1.0); // Default value
    EXPECT_TRUE(theTransform->GetEPSGCode() == 0); // Default value
    EXPECT_TRUE(theTransform->GetDataAvailability() == GeoCoordinates::GeodeticTransformDataAvailability::DataUnavailable); // Availability resolved here
    EXPECT_TRUE(theTransform->GetConvertMethodCode() == GeoCoordinates::GenConvertCode::GenConvertType_GFILE);
    EXPECT_TRUE(theTransform->GetGridFileDefinitionCount() == 1);
    GeoCoordinates::GridFileDefinition theFileDef = theTransform->GetGridFileDefinition(0);
    // This also tests the forward/backward slashes conversion. We use UNIX style in API but allow Windows on input.
    EXPECT_TRUE(Utf8String(theFileDef.GetFileName()) == "./NoPath/InexistantFile.gsb");
    EXPECT_TRUE(theFileDef.GetFormat() == GeoCoordinates::GridFileFormat::FORMAT_NTv2);
    EXPECT_TRUE(theFileDef.GetDirection() == GeoCoordinates::GridFileDirection::DIRECTION_DIRECT);

    GeoCoordinates::EllipsoidCP theEllipsoid = theDatum->GetEllipsoid();
    ASSERT_TRUE(theEllipsoid != nullptr);


    EXPECT_TRUE(Utf8String(theEllipsoid->GetName()) == "CustomEllipsoid");
    EXPECT_TRUE(Utf8String(theEllipsoid->GetDescription()) == "Custom ellipsoid description");
    EXPECT_TRUE(Utf8String(theEllipsoid->GetSource(source)) == "Custom Ellipsoid source");
    EXPECT_TRUE(theEllipsoid->GetEquatorialRadius() == 6378120.0);
    EXPECT_TRUE(theEllipsoid->GetPolarRadius() == 6356794.719195305951);

    bvector<Utf8String> listOfFiles;
    EXPECT_TRUE(theDatum->HasMissingGridFiles(listOfFiles));
    EXPECT_TRUE(listOfFiles.size() == 1);
    EXPECT_TRUE(listOfFiles[0] == "./NoPath/InexistantFile.gsb");
    
    theDatum->Destroy();
}

/*---------------------------------------------------------------------------------**//**
* Test using a JSON defined self contained Datum that uses grid shift files and custom ellipsoid
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGCSUnitTests, CreateAFullySelfContainedDatumWithCustomEllipsoidButFileIncorrectFormat_Test)
{
    Utf8String customDatum5 = "    {"
        "      \"id\": \"CustomDatum5\","
        "      \"description\": \"Custom Datum 5 Description\","
        "      \"deprecated\": false,"
        "      \"source\": \"Custom Datum 5 Source\","
        "      \"epsg\": 0,"
        "      \"ellipsoidId\": \"CustomEllipsoid\","
        "      \"ellipsoid\" : {"
        "        \"id\": \"CustomEllipsoid\","
        "        \"description\": \"Custom ellipsoid description\","
        "        \"source\": \"Custom Ellipsoid source\","
        "        \"equatorialRadius\": 6378120.0,"
        "        \"polarRadius\": 6356794.719195305951"
        "      },"
        "      \"transforms\": ["
        "        {"
        "          \"method\": \"GridFiles\","
        "          \"geocentric\": null,"
        "          \"positionalVector\": null,"
        "          \"gridFile\": {"
        "            \"files\": ["
        "              {"
        "                \"fileName\": \"./Germany/BETA2007.gsb\","
        "                \"format\": \"NADCON\","
        "                \"direction\": \"Direct\""
        "              }"
        "            ]"
        "          }"
        "        }"
        "      ]"
        "    }";

    GeoCoordinates::DatumP theDatum = const_cast<GeoCoordinates::DatumP>(GeoCoordinates::Datum::CreateDatum());

    Utf8String errorMessage;
    ASSERT_TRUE(SUCCESS == theDatum->FromJson(Json::Value::From(customDatum5), errorMessage));
    Utf8String source;

    EXPECT_TRUE(Utf8String(theDatum->GetName()) == "CustomDatum5");
    EXPECT_TRUE(Utf8String(theDatum->GetDescription()) == "Custom Datum 5 Description");
    EXPECT_TRUE(Utf8String(theDatum->GetSource(source)) == "Custom Datum 5 Source");

    GeoCoordinates::GeodeticTransformPathCP theStoredPath = theDatum->GetStoredGeodeticTransformPath();

    ASSERT_TRUE(theStoredPath != nullptr);
    EXPECT_TRUE(theStoredPath->GetGeodeticTransformCount() == 1);

    GeoCoordinates::GeodeticTransformCP theTransform = theStoredPath->GetGeodeticTransform(0);
    ASSERT_TRUE(theTransform != nullptr);

    EXPECT_TRUE(theTransform->IsValid());
    EXPECT_TRUE(!theTransform->IsDeprecated());
    EXPECT_TRUE(!theTransform->IsNullTransform());
    EXPECT_TRUE(Utf8String(theTransform->GetSourceDatumName()) == "CustomDatum5");
    EXPECT_TRUE(Utf8String(theTransform->GetTargetDatumName()) == "WGS84");
    EXPECT_TRUE(Utf8String(theTransform->GetName()) == "CustomDatum5_to_WGS84");
    EXPECT_TRUE(theTransform->GetAccuracy() == 1.0); // Default value
    EXPECT_TRUE(theTransform->GetEPSGCode() == 0); // Default value
    EXPECT_TRUE(theTransform->GetDataAvailability() == GeoCoordinates::GeodeticTransformDataAvailability::DataUnavailable); // Availability resolved here
    EXPECT_TRUE(theTransform->GetConvertMethodCode() == GeoCoordinates::GenConvertCode::GenConvertType_GFILE);
    EXPECT_TRUE(theTransform->GetGridFileDefinitionCount() == 1);
    GeoCoordinates::GridFileDefinition theFileDef = theTransform->GetGridFileDefinition(0);
    // This also tests the forward/backward slashes conversion. We use UNIX style in API but allow Windows on input.
    EXPECT_TRUE(Utf8String(theFileDef.GetFileName()) == "./Germany/BETA2007.gsb");
    EXPECT_TRUE(theFileDef.GetFormat() == GeoCoordinates::GridFileFormat::FORMAT_NADCON);
    EXPECT_TRUE(theFileDef.GetDirection() == GeoCoordinates::GridFileDirection::DIRECTION_DIRECT);

    GeoCoordinates::EllipsoidCP theEllipsoid = theDatum->GetEllipsoid();
    ASSERT_TRUE(theEllipsoid != nullptr);


    EXPECT_TRUE(Utf8String(theEllipsoid->GetName()) == "CustomEllipsoid");
    EXPECT_TRUE(Utf8String(theEllipsoid->GetDescription()) == "Custom ellipsoid description");
    EXPECT_TRUE(Utf8String(theEllipsoid->GetSource(source)) == "Custom Ellipsoid source");
    EXPECT_TRUE(theEllipsoid->GetEquatorialRadius() == 6378120.0);
    EXPECT_TRUE(theEllipsoid->GetPolarRadius() == 6356794.719195305951);

    bvector<Utf8String> listOfFiles;
    EXPECT_TRUE(theDatum->HasMissingGridFiles(listOfFiles));
    EXPECT_TRUE(listOfFiles.size() == 1);
    EXPECT_TRUE(listOfFiles[0] == "./Germany/BETA2007.gsb");
    
    theDatum->Destroy();
}

/*---------------------------------------------------------------------------------**//**
* Test using a JSON defined self contained Datum that uses grid shift files and custom ellipsoid
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGCSUnitTests, FullCustomReprojectionTest_Test)
{
    Utf8String gcs1 = "    {"
        "      \"id\": \"TESTGCS6\","
        "      \"description\": \"USES CUSTOM DATUM (TESTDATUM3)\","
        "      \"source\": \"EPSG:2165\","
        "      \"deprecated\": false,"
        "      \"epsg\": 0,"
        "      \"datumId\": \"TEST-GRID\","
        "      \"datum\" : {"
        "        \"id\": \"TEST-GRID\","
        "        \"description\": \"TEST DATUM 3 - Uses custom ell but custom transfo\","
        "        \"deprecated\": false,"
        "        \"source\": \"Emmo\","
        "        \"epsg\": 0,"
        "        \"ellipsoidId\": \"CustomEllipsoid1\","
        "        \"ellipsoid\" : {"
        "          \"id\": \"CustomEllipsoid1\","
        "          \"description\": \"Custom Ellipsoid1 Description\","
        "          \"source\": \"Custom Ellipsoid1 Source\","
        "          \"equatorialRadius\": 6378171.0,"
        "          \"polarRadius\": 6356795.719195305951"
        "        },"
        "        \"transforms\": ["
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
        "          }"
        "        ]"
        "      },"
        "      \"unit\": \"Meter\","
        "      \"projection\": {"
        "        \"method\": \"TransverseMercator\","
        "        \"centralMeridian\": 143,"
        "        \"latitudeOfOrigin\": 0,"
        "        \"scaleFactor\": 0.9996,"
        "        \"falseEasting\": 500000.000,"
        "        \"falseNorthing\": 10000000.000"
        "      },"
        "      \"extent\": {"
        "        \"southWest\": {"
        "          \"latitude\": 3.9,"
        "          \"longitude\": -7.55"
        "        },"
        "        \"northEast\": {"
        "          \"latitude\": 5.13,"
        "          \"longitude\": -2.75"
        "        }"
        "      }"
        "    }";

        Utf8String gcs2 = "    {"
        "      \"id\": \"TESTGCS7\","
        "      \"description\": \"USES CUSTOM DATUM\","
        "      \"source\": \"EPSG:2165\","
        "      \"deprecated\": false,"
        "      \"epsg\": 0,"
        "      \"datumId\": \"TEST-GRID2\","
        "      \"datum\" : {"
        "        \"id\": \"TEST-GRID2\","
        "        \"description\": \"TEST DATUM - Uses custom ell and custom transfo\","
        "        \"deprecated\": false,"
        "        \"source\": \"Emmo\","
        "        \"epsg\": 0,"
        "        \"ellipsoidId\": \"CustomEllipsoid2\","
        "        \"ellipsoid\" : {"
        "          \"id\": \"CustomEllipsoid2\","
        "          \"description\": \"Custom Ellipsoid2 Description\","
        "          \"source\": \"Custom Ellipsoid2 Source\","
        "          \"equatorialRadius\": 6378174.1,"
        "          \"polarRadius\": 6356794.51"
        "        },"
        "        \"transforms\": ["
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
        "          }"
        "        ]"
        "      },"
        "      \"unit\": \"Meter\","
        "      \"projection\": {"
        "        \"method\": \"TransverseMercator\","
        "        \"centralMeridian\": 143,"
        "        \"latitudeOfOrigin\": 0,"
        "        \"scaleFactor\": 0.9996,"
        "        \"falseEasting\": 500000.000,"
        "        \"falseNorthing\": 10000000.000"
        "      },"
        "      \"extent\": {"
        "        \"southWest\": {"
        "          \"latitude\": 3.9,"
        "          \"longitude\": -7.55"
        "        },"
        "        \"northEast\": {"
        "          \"latitude\": 5.13,"
        "          \"longitude\": -2.75"
        "        }"
        "      }"
        "    }";

    GeoCoordinates::BaseGCSPtr firstGCS = GeoCoordinates::BaseGCS::CreateGCS();

    Utf8String errMessage;
    ASSERT_EQ(SUCCESS, firstGCS->FromHorizontalJson(BeJsDocument(gcs1), errMessage)) << errMessage.c_str();

    GeoCoordinates::BaseGCSPtr secondGCS = GeoCoordinates::BaseGCS::CreateGCS();

    ASSERT_EQ(SUCCESS, secondGCS->FromHorizontalJson(BeJsDocument(gcs2), errMessage)) << errMessage.c_str();

  //  DPoint3d inPoint, outPoint;
  //  inPoint.x = 500000.000;
  //  inPoint.y = 6000000.000;
  //  inPoint.z = 0.0;

   // EXPECT_TRUE(firstGCS->CartesianFromCartesian(outPoint, inPoint, *secondGCS) == REPROJECT_Success);
}

/*---------------------------------------------------------------------------------**//**
* Test using a JSON defined self contained Datum that uses grid shift files and custom ellipsoid
* and some additional transforms
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGCSUnitTests, FullCustomReprojectionTestManyTransformsCrash_Test)
{
    Utf8String gcs1 =
        R"X({
            "id": "TESTGCS6",
                "description" : "USES CUSTOM DATUM (TESTDATUM3)",
                "source" : "EPSG:2165",
                "deprecated" : false,
                "epsg" : 0,
                "datumId" : "TEST-GRID",
                "datum" : {
                "id": "TEST-GRID",
                    "description" : "TEST DATUM 3 - Uses custom ell but custom transfo",
                    "deprecated" : false,
                    "source" : "Emmo",
                    "epsg" : 0,
                    "ellipsoidId" : "CustomEllipsoid1",
                    "ellipsoid" : {
                    "id": "CustomEllipsoid1",
                        "description" : "Custom Ellipsoid1 Description",
                        "source" : "Custom Ellipsoid1 Source",
                        "equatorialRadius" : 6378171.0,
                        "polarRadius" : 6356795.719195306
                },
                    "transforms": [
                    {
                        "method": "GridFiles",
                            "sourceEllipsoidId" : "CustomEllipsoid1",
                            "sourceEllipsoid" : {
                            "id": "CustomEllipsoid1",
                                "equatorialRadius" : 6378171.0,
                                "polarRadius" : 6356795.719195306
                        },
                            "targetEllipsoidId" : "CustomEllipsoid2",
                                "targetEllipsoid" : {
                                "id": "CustomEllipsoid2",
                                    "equatorialRadius" : 6378171.1,
                                    "polarRadius" : 6356795.8
                            },
                                "gridFile" : {
                                    "files": [
                                    {
                                        "fileName": "./Australia/AGD66/A66National-13.09.01.gsb",
                                            "format" : "NTv2",
                                            "direction" : "Direct"
                                    }
                                    ]
                                }
                    },
            {
                "method": "Geocentric",
                "sourceEllipsoidId" : "CustomEllipsoid2",
                "sourceEllipsoid" : {
                    "id": "CustomEllipsoid2",
                    "equatorialRadius" : 6378171.1,
                    "polarRadius" : 6356795.8
                },
                "targetEllipsoidId" : "CustomEllipsoid3",
                "targetEllipsoid" : {
                    "id": "CustomEllipsoid3",
                    "equatorialRadius" : 6378174.1,
                    "polarRadius" : 6356796.1
                },
                "geocentric" : {
                    "delta": {
                        "x": -115,
                        "y" : 118,
                        "z" : 426
                    }
                }
            },
            {
                "method": "PositionalVector",
                "geocentric" : null,
                "positionalVector" : {
                    "scalePPM": 2.4985,
                    "delta" : {
                        "x": -120.271,
                        "y" : -64.543,
                        "z" : 161.632
                    },
                    "rotation" : {
                        "x": 0.2175,
                        "y" : -0.0672,
                        "z" : -0.1291
                    }
                },
                "sourceEllipsoidId": "CustomEllipsoid3",
                "sourceEllipsoid" : {
                    "id": "CustomEllipsoid3",
                    "equatorialRadius" : 6378174.1,
                    "polarRadius" : 6356796.1
                },
                "targetEllipsoidId" : "WGS84",
                "targetEllipsoid" : {
                    "id": "WGS84",
                    "equatorialRadius" : 6378137.0,
                    "polarRadius" : 6356752.3142
                }
            }
                    ]
            },
                "unit": "Meter",
                "projection" : {
                "method": "TransverseMercator",
                    "centralMeridian" : 143,
                    "latitudeOfOrigin" : 0,
                    "scaleFactor" : 0.9996,
                    "falseEasting" : 500000.0,
                    "falseNorthing" : 10000000.0
            },
            "extent": {
              "southWest": {
                "latitude": 3.9,
                "longitude": -7.55
              },
              "northEast": {
                "latitude": 5.13,
                "longitude": -2.75
              }
            }
        })X";

    Utf8String gcs2 = "    {"
        "      \"id\": \"TESTGCS7\","
        "      \"description\": \"USES CUSTOM DATUM\","
        "      \"source\": \"EPSG:2165\","
        "      \"deprecated\": false,"
        "      \"epsg\": 0,"
        "      \"datumId\": \"TEST-GRID2\","
        "      \"datum\" : {"
        "        \"id\": \"TEST-GRID2\","
        "        \"description\": \"TEST DATUM - Uses custom ell and custom transfo\","
        "        \"deprecated\": false,"
        "        \"source\": \"Emmo\","
        "        \"epsg\": 0,"
        "        \"ellipsoidId\": \"CustomEllipsoid2\","
        "        \"ellipsoid\" : {"
        "          \"id\": \"CustomEllipsoid2\","
        "          \"description\": \"Custom Ellipsoid2 Description\","
        "          \"source\": \"Custom Ellipsoid2 Source\","
        "          \"equatorialRadius\": 6378174.1,"
        "          \"polarRadius\": 6356794.51"
        "        },"
        "        \"transforms\": ["
        "          {"
        "            \"method\": \"GridFiles\","
        "            \"gridFile\": {"
        "              \"files\": ["
        "                {"
        "                  \"fileName\": \"./Australia/AGD84/National84-02.07.01.gsb\","
        "                  \"format\": \"NTv2\","
        "                  \"direction\": \"Direct\""
        "                }"
        "              ]"
        "            }"
        "          }"
        "        ]"
        "      },"
        "      \"unit\": \"Meter\","
        "      \"projection\": {"
        "        \"method\": \"TransverseMercator\","
        "        \"centralMeridian\": 143,"
        "        \"latitudeOfOrigin\": 0,"
        "        \"scaleFactor\": 0.9996,"
        "        \"falseEasting\": 500000.000,"
        "        \"falseNorthing\": 10000000.000"
        "      },"
        "      \"extent\": {"
        "        \"southWest\": {"
        "          \"latitude\": 3.9,"
        "          \"longitude\": -7.55"
        "        },"
        "        \"northEast\": {"
        "          \"latitude\": 5.13,"
        "          \"longitude\": -2.75"
        "        }"
        "      }"
        "    }";

    GeoCoordinates::BaseGCSPtr firstGCS = GeoCoordinates::BaseGCS::CreateGCS();

    Utf8String errMessage;
    ASSERT_EQ(SUCCESS, firstGCS->FromHorizontalJson(BeJsDocument(gcs1), errMessage)) << errMessage.c_str();
    firstGCS->SetVerticalDatumCode(GeoCoordinates::vdcGeoid);

    GeoCoordinates::BaseGCSPtr secondGCS = GeoCoordinates::BaseGCS::CreateGCS();

    ASSERT_EQ(SUCCESS, secondGCS->FromHorizontalJson(BeJsDocument(gcs2), errMessage)) << errMessage.c_str();

    DPoint3d inPoint, outPoint;
    inPoint.x = 500000.000;
    inPoint.y = 6000000.000;
    inPoint.z = 0.0;
    // Since fallback is used a soft error is returned
    ReprojectStatus status = firstGCS->CartesianFromCartesian(outPoint, inPoint, *secondGCS);
    if (status != REPROJECT_CSMAPERR_OutOfUsefulRange)
      {
        printf("Status is not out of useful range. It is value %ld \n", (long) status);
        bvector<Utf8String> listOfFiles;
        if (firstGCS->GetDatum()->HasMissingGridFiles(listOfFiles))
        {
          for (auto file : listOfFiles)
              printf("File missing: %s \n", file.c_str());
        }
      }
          
    EXPECT_TRUE(firstGCS->CartesianFromCartesian(outPoint, inPoint, *secondGCS) == REPROJECT_CSMAPERR_OutOfUsefulRange);
}

/*---------------------------------------------------------------------------------**//**
* Test using a JSON defined self contained Datum that uses grid shift files and custom ellipsoid
* and some additional transforms
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGCSUnitTests, FullCustomReprojectionTestManyTransforms_Test)
{
    Utf8String gcs1 =
        R"X({
            "id": "TESTGCS6",
                "description" : "USES CUSTOM DATUM (TESTDATUM3)",
                "source" : "EPSG:2165",
                "deprecated" : false,
                "epsg" : 0,
                "datumId" : "TEST-GRID",
                "datum" : {
                "id": "TEST-GRID",
                    "description" : "TEST DATUM 3 - Uses custom ell but custom transfo",
                    "deprecated" : false,
                    "source" : "Emmo",
                    "epsg" : 0,
                    "ellipsoidId" : "CustomEllipsoid1",
                    "ellipsoid" : {
                    "id": "CustomEllipsoid1",
                        "description" : "Custom Ellipsoid1 Description",
                        "source" : "Custom Ellipsoid1 Source",
                        "equatorialRadius" : 6378171.0,
                        "polarRadius" : 6356795.719195306
                },
                    "transforms": [
                    {
                        "method": "GridFiles",
                            "sourceEllipsoidId" : "CustomEllipsoid1",
                            "sourceEllipsoid" : {
                            "id": "CustomEllipsoid1",
                                "equatorialRadius" : 6378171.0,
                                "polarRadius" : 6356795.719195306
                        },
                            "targetEllipsoidId" : "CustomEllipsoid2",
                                "targetEllipsoid" : {
                                "id": "CustomEllipsoid2",
                                    "equatorialRadius" : 6378171.1,
                                    "polarRadius" : 6356795.8
                            },
                                "gridFile" : {
                                    "fallback":
                                    {
                                        "scalePPM": 2.4985,
                                        "delta" : {
                                            "x": -120.271,
                                            "y" : -64.543,
                                            "z" : 161.632
                                        },
                                        "rotation" : {
                                            "x": 0.2175,
                                            "y" : -0.0672,
                                            "z" : -0.1291
                                        }
                                    },
                                    "files": [
                                    {
                                        "fileName": "./Australia/AGD66/A66National-13.09.01.gsb",
                                            "format" : "NTv2",
                                            "direction" : "Direct"
                                    }
                                    ]
                                }
                    },
            {
                "method": "Geocentric",
                "sourceEllipsoidId" : "CustomEllipsoid2",
                "sourceEllipsoid" : {
                    "id": "CustomEllipsoid2",
                    "equatorialRadius" : 6378171.1,
                    "polarRadius" : 6356795.8
                },
                "targetEllipsoidId" : "CustomEllipsoid3",
                "targetEllipsoid" : {
                    "id": "CustomEllipsoid3",
                    "equatorialRadius" : 6378174.1,
                    "polarRadius" : 6356796.1
                },
                "geocentric" : {
                    "delta": {
                        "x": -115,
                        "y" : 118,
                        "z" : 426
                    }
                }
            },
            {
                "method": "PositionalVector",
                "geocentric" : null,
                "positionalVector" : {
                    "scalePPM": 2.4985,
                    "delta" : {
                        "x": -120.271,
                        "y" : -64.543,
                        "z" : 161.632
                    },
                    "rotation" : {
                        "x": 0.2175,
                        "y" : -0.0672,
                        "z" : -0.1291
                    }
                },
                "sourceEllipsoidId": "CustomEllipsoid3",
                "sourceEllipsoid" : {
                    "id": "CustomEllipsoid3",
                    "equatorialRadius" : 6378174.1,
                    "polarRadius" : 6356796.1
                },
                "targetEllipsoidId" : "WGS84",
                "targetEllipsoid" : {
                    "id": "WGS84",
                    "equatorialRadius" : 6378137.0,
                    "polarRadius" : 6356752.3142
                }
            }
                    ]
            },
                "unit": "Meter",
                "projection" : {
                "method": "TransverseMercator",
                    "centralMeridian" : 143,
                    "latitudeOfOrigin" : 0,
                    "scaleFactor" : 0.9996,
                    "falseEasting" : 500000.0,
                    "falseNorthing" : 10000000.0
            },
            "extent": {
              "southWest": {
                "latitude": 3.9,
                "longitude": -7.55
              },
              "northEast": {
                "latitude": 5.13,
                "longitude": -2.75
              }
            }
        })X";

        Utf8String gcs2 = "    {"
        "      \"id\": \"TESTGCS7\","
        "      \"description\": \"USES CUSTOM DATUM\","
        "      \"source\": \"EPSG:2165\","
        "      \"deprecated\": false,"
        "      \"epsg\": 0,"
        "      \"datumId\": \"TEST-GRID2\","
        "      \"datum\" : {"
        "        \"id\": \"TEST-GRID2\","
        "        \"description\": \"TEST DATUM - Uses custom ell and custom transfo\","
        "        \"deprecated\": false,"
        "        \"source\": \"Emmo\","
        "        \"epsg\": 0,"
        "        \"ellipsoidId\": \"CustomEllipsoid2\","
        "        \"ellipsoid\" : {"
        "          \"id\": \"CustomEllipsoid2\","
        "          \"description\": \"Custom Ellipsoid2 Description\","
        "          \"source\": \"Custom Ellipsoid2 Source\","
        "          \"equatorialRadius\": 6378174.1,"
        "          \"polarRadius\": 6356794.51"
        "        },"
        "        \"transforms\": ["
        "          {"
        "            \"method\": \"GridFiles\","
        "            \"gridFile\": {"
        "             \"fallback\":"
        "             {"
        "                \"scalePPM\": 2.4985,"
        "                \"delta\" : {"
        "                  \"x\": -120.271,"
        "                  \"y\" : -64.543,"
        "                  \"z\" : 161.632"
        "                },"
        "                \"rotation\" : {"
        "                  \"x\": 0.2175,"
        "                  \"y\" : -0.0672,"
        "                  \"z\" : -0.1291"
        "                }"
        "              },"
        "              \"files\": ["
        "                {"
        "                  \"fileName\": \"./Australia/AGD84/National84-02.07.01.gsb\","
        "                  \"format\": \"NTv2\","
        "                  \"direction\": \"Direct\""
        "                }"
        "              ]"
        "            }"
        "          }"
        "        ]"
        "      },"
        "      \"unit\": \"Meter\","
        "      \"projection\": {"
        "        \"method\": \"TransverseMercator\","
        "        \"centralMeridian\": 143,"
        "        \"latitudeOfOrigin\": 0,"
        "        \"scaleFactor\": 0.9996,"
        "        \"falseEasting\": 500000.000,"
        "        \"falseNorthing\": 10000000.000"
        "      },"
        "      \"extent\": {"
        "        \"southWest\": {"
        "          \"latitude\": 3.9,"
        "          \"longitude\": -7.55"
        "        },"
        "        \"northEast\": {"
        "          \"latitude\": 5.13,"
        "          \"longitude\": -2.75"
        "        }"
        "      }"
        "    }";

    GeoCoordinates::BaseGCSPtr firstGCS = GeoCoordinates::BaseGCS::CreateGCS();

    Utf8String errMessage;
    ASSERT_EQ(SUCCESS, firstGCS->FromHorizontalJson(BeJsDocument(gcs1), errMessage)) << errMessage.c_str();

    GeoCoordinates::BaseGCSPtr secondGCS = GeoCoordinates::BaseGCS::CreateGCS();

    ASSERT_EQ(SUCCESS, secondGCS->FromHorizontalJson(BeJsDocument(gcs2), errMessage)) << errMessage.c_str();

    DPoint3d inPoint, outPoint;
    inPoint.x = 500000.000;
    inPoint.y = 6000000.000;
    inPoint.z = 0.0;

    // Since fallback is used a soft error is returned
    ReprojectStatus status = firstGCS->CartesianFromCartesian(outPoint, inPoint, *secondGCS);
    if (status != REPROJECT_CSMAPERR_OutOfUsefulRange)
      {
        printf("Status is not out of useful range. It is value %ld \n", (long) status);
        bvector<Utf8String> listOfFiles;
        if (firstGCS->GetDatum()->HasMissingGridFiles(listOfFiles))
        {
          for (auto file : listOfFiles)
              printf("File missing: %s \n", file.c_str());
        }
      }
    EXPECT_TRUE(firstGCS->CartesianFromCartesian(outPoint, inPoint, *secondGCS) == REPROJECT_CSMAPERR_OutOfUsefulRange);
}

/*---------------------------------------------------------------------------------**//**
* InitTransverseMercator test
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, IntTransverseMercatorTest)
    {
    GeoCoordinates::BaseGCSPtr theGCS;

    theGCS = GeoCoordinates::BaseGCS::CreateGCS();

    Utf8String errorMessage;

    theGCS->InitTransverseMercator(&errorMessage, "ETRF89", "METER", -3.0, 0.0, 0.9996, 500000.0, 0.0, 1);

    GeoCoordinates::BaseGCSPtr compareGCS = GeoCoordinates::BaseGCS::CreateGCS("ETRS89.UTM-30N");

    EXPECT_TRUE(compareGCS->IsEquivalent(*theGCS));

    DPoint3d inPoint, outPoint = {0,0,0};
    inPoint.x = 500000.000;
    inPoint.y = 6000000.000;
    inPoint.z = 0.0;

    EXPECT_TRUE(compareGCS->CartesianFromCartesian(outPoint, inPoint, *theGCS) == REPROJECT_Success);

    EXPECT_NEAR(outPoint.x, inPoint.x, 0.0001);
    EXPECT_NEAR(outPoint.y, inPoint.y, 0.0001);
    EXPECT_NEAR(outPoint.z, inPoint.z, 0.0001);
    }

/*---------------------------------------------------------------------------------**//**
* InitTransverseMercator test
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, BasicSetVerticalDatumFromJson)
    {
    GeoCoordinates::BaseGCSPtr theGCS;

    theGCS = GeoCoordinates::BaseGCS::CreateGCS();

    Utf8String errorMessage;

    GeoCoordinates::BaseGCSPtr utmGCS = GeoCoordinates::BaseGCS::CreateGCS("ETRS89.UTM-30N");

    EXPECT_TRUE(utmGCS.IsValid() && utmGCS->IsValid());

    Utf8String vertJsonGeoid            = "{\"id\": \"GEOID\"}";
    Utf8String vertJsonEllipsoid        = "{\"id\": \"ELLIPSOID\"}";
    Utf8String vertJsonNGVD29           = "{\"id\": \"NGVD29\"}";
    Utf8String vertJsonNAVD88           = "{\"id\": \"NAVD88\"}";
    Utf8String vertJsonLocalEllipsoid   = "{\"id\": \"LOCAL_ELLIPSOID\"}";

    EXPECT_TRUE(utmGCS->GetVerticalDatumCode() == vdcFromDatum);

    EXPECT_TRUE(SUCCESS == utmGCS->FromVerticalJson(BeJsDocument(vertJsonGeoid), errorMessage));

    EXPECT_TRUE(utmGCS->GetVerticalDatumCode() == vdcGeoid);

    EXPECT_TRUE(SUCCESS == utmGCS->FromVerticalJson(BeJsDocument(vertJsonEllipsoid), errorMessage));

    EXPECT_TRUE(utmGCS->GetVerticalDatumCode() == vdcEllipsoid);

    // Make sure we cannot set forbidden values for datum
    EXPECT_TRUE(GEOCOORDERR_BadArg == utmGCS->FromVerticalJson(BeJsDocument(vertJsonNGVD29), errorMessage));
    EXPECT_TRUE(utmGCS->GetVerticalDatumCode() == vdcEllipsoid); // Unchanged

    EXPECT_TRUE(GEOCOORDERR_BadArg == utmGCS->FromVerticalJson(BeJsDocument(vertJsonNAVD88), errorMessage));
    EXPECT_TRUE(utmGCS->GetVerticalDatumCode() == vdcEllipsoid); // Unchanged

    EXPECT_TRUE(GEOCOORDERR_BadArg == utmGCS->FromVerticalJson(BeJsDocument(vertJsonLocalEllipsoid), errorMessage));
    EXPECT_TRUE(utmGCS->GetVerticalDatumCode() == vdcEllipsoid); // Unchanged

    GeoCoordinates::BaseGCSPtr nad27GCS = GeoCoordinates::BaseGCS::CreateGCS("AK-1");

    EXPECT_TRUE(nad27GCS.IsValid() && nad27GCS->IsValid());

    EXPECT_TRUE(nad27GCS->GetVerticalDatumCode() == vdcFromDatum);

    EXPECT_TRUE(SUCCESS == nad27GCS->FromVerticalJson(BeJsDocument(vertJsonGeoid), errorMessage));
    EXPECT_TRUE(nad27GCS->GetVerticalDatumCode() == vdcGeoid);

    EXPECT_TRUE(SUCCESS == nad27GCS->FromVerticalJson(BeJsDocument(vertJsonEllipsoid), errorMessage));
    EXPECT_TRUE(nad27GCS->GetVerticalDatumCode() == vdcEllipsoid);

    // Make sure we cannot set forbiden values for datum
    EXPECT_TRUE(SUCCESS == nad27GCS->FromVerticalJson(BeJsDocument(vertJsonNGVD29), errorMessage));
    EXPECT_TRUE(nad27GCS->GetVerticalDatumCode() == vdcNGVD29);

    EXPECT_TRUE(SUCCESS == nad27GCS->FromVerticalJson(BeJsDocument(vertJsonNAVD88), errorMessage));
    EXPECT_TRUE(nad27GCS->GetVerticalDatumCode() == vdcNAVD88);

    EXPECT_TRUE(GEOCOORDERR_BadArg == nad27GCS->FromVerticalJson(BeJsDocument(vertJsonLocalEllipsoid), errorMessage));
    EXPECT_TRUE(nad27GCS->GetVerticalDatumCode() == vdcNAVD88); // Unchanged

    // NAD83 datum
    GeoCoordinates::BaseGCSPtr nad83GCS = GeoCoordinates::BaseGCS::CreateGCS("MAHP-IS");

    EXPECT_TRUE(nad83GCS.IsValid() && nad83GCS->IsValid());

    EXPECT_TRUE(nad83GCS->GetVerticalDatumCode() == vdcFromDatum);

    EXPECT_TRUE(SUCCESS == nad83GCS->FromVerticalJson(BeJsDocument(vertJsonGeoid), errorMessage));
    EXPECT_TRUE(nad83GCS->GetVerticalDatumCode() == vdcGeoid);

    EXPECT_TRUE(SUCCESS == nad83GCS->FromVerticalJson(BeJsDocument(vertJsonEllipsoid), errorMessage));
    EXPECT_TRUE(nad83GCS->GetVerticalDatumCode() == vdcEllipsoid);

    // Make sure we cannot set forbiden values for datum
    EXPECT_TRUE(SUCCESS == nad83GCS->FromVerticalJson(BeJsDocument(vertJsonNGVD29), errorMessage));
    EXPECT_TRUE(nad83GCS->GetVerticalDatumCode() == vdcNGVD29);

    EXPECT_TRUE(SUCCESS == nad83GCS->FromVerticalJson(BeJsDocument(vertJsonNAVD88), errorMessage));
    EXPECT_TRUE(nad83GCS->GetVerticalDatumCode() == vdcNAVD88);

    EXPECT_TRUE(GEOCOORDERR_BadArg == nad83GCS->FromVerticalJson(BeJsDocument(vertJsonLocalEllipsoid), errorMessage));
    EXPECT_TRUE(nad83GCS->GetVerticalDatumCode() == vdcNAVD88); // Unchanged

    // OSGB36 datum
    GeoCoordinates::BaseGCSPtr britGCS = GeoCoordinates::BaseGCS::CreateGCS("BritishNatGrid");

    EXPECT_TRUE(britGCS.IsValid() && britGCS->IsValid());

    EXPECT_TRUE(britGCS->GetVerticalDatumCode() == vdcFromDatum);

    EXPECT_TRUE(SUCCESS == britGCS->FromVerticalJson(BeJsDocument(vertJsonGeoid), errorMessage));
    EXPECT_TRUE(britGCS->GetVerticalDatumCode() == vdcGeoid);

    EXPECT_TRUE(SUCCESS == britGCS->FromVerticalJson(BeJsDocument(vertJsonEllipsoid), errorMessage));
    EXPECT_TRUE(britGCS->GetVerticalDatumCode() == vdcEllipsoid);

    // Make sure we cannot set forbiden values for datum
    EXPECT_TRUE(GEOCOORDERR_BadArg == britGCS->FromVerticalJson(BeJsDocument(vertJsonNGVD29), errorMessage));
    EXPECT_TRUE(britGCS->GetVerticalDatumCode() == vdcEllipsoid); // Unchanged

    EXPECT_TRUE(GEOCOORDERR_BadArg == britGCS->FromVerticalJson(BeJsDocument(vertJsonNAVD88), errorMessage));
    EXPECT_TRUE(britGCS->GetVerticalDatumCode() == vdcEllipsoid); // Unchanged

    // This one exceptionally allowed
    EXPECT_TRUE(SUCCESS == britGCS->FromVerticalJson(BeJsDocument(vertJsonLocalEllipsoid), errorMessage));
    EXPECT_TRUE(britGCS->GetVerticalDatumCode() == vdcLocalEllipsoid);
    }

/*---------------------------------------------------------------------------------**//**
* BasicSetFullDatumFromJson test
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, BasicSetFullDatumFromJson)
    {
    Utf8String testJson = R"X({
   "additionalTransform" : {
      "helmert2DWithZOffset" : {
         "rotDeg" : 90.0,
         "scale" : 1.0,
         "translationX" : 0.0,
         "translationY" : -20.0,
         "translationZ" : 300.0
      }
   },
   "horizontalCRS" : {
      "area" : {
         "latitude" : {
            "max" : 84.0,
            "min" : 48.0
         },
         "longitude" : {
            "max" : -109.50,
            "min" : -120.50
         }
      },
      "datum" : {
         "description" : "North American Datum of 1927 (US48, AK, HI, and Canada)",
         "ellipsoid" : {
            "description" : "Clarke 1866, Benoit Ratio",
            "epsg" : 7008,
            "equatorialRadius" : 6378206.4000000004,
            "id" : "CLRK66",
            "polarRadius" : 6356583.7999999998,
            "source" : "US Defense Mapping Agency, TR-8350.2-B, December 1987"
         },
         "ellipsoidId" : "CLRK66",
         "epsg" : 6267,
         "id" : "NAD27",
         "source" : "US Defense Mapping Agency, TR-8350.2-B, December 1987",
         "transforms" : [
            {
               "gridFile" : {
                  "files" : [
                     {
                        "direction" : "Direct",
                        "fileName" : "./Usa/Nadcon/conus.l?s",
                        "format" : "NADCON"
                     },
                     {
                        "direction" : "Direct",
                        "fileName" : "./Usa/Nadcon/alaska.l?s",
                        "format" : "NADCON"
                     },
                     {
                        "direction" : "Direct",
                        "fileName" : "./Usa/Nadcon/prvi.l?s",
                        "format" : "NADCON"
                     },
                     {
                        "direction" : "Direct",
                        "fileName" : "./Usa/Nadcon/hawaii.l?s",
                        "format" : "NADCON"
                     },
                     {
                        "direction" : "Direct",
                        "fileName" : "./Usa/Nadcon/stgeorge.l?s",
                        "format" : "NADCON"
                     },
                     {
                        "direction" : "Direct",
                        "fileName" : "./Usa/Nadcon/stlrnc.l?s",
                        "format" : "NADCON"
                     },
                     {
                        "direction" : "Direct",
                        "fileName" : "./Usa/Nadcon/stpaul.l?s",
                        "format" : "NADCON"
                     }
                  ]
               },
               "method" : "GridFiles",
               "sourceDatumId" : "NAD27",
               "targetDatumId" : "NAD83"
            }
         ]
      },
      "datumId" : "NAD27",
      "description" : "",
      "id" : "XYZ",
      "projection" : {
         "centralMeridian" : -115.0,
         "falseEasting" : 0.0,
         "falseNorthing" : 0.0,
         "latitudeOfOrigin" : 0.0,
         "method" : "TransverseMercator",
         "scaleFactor" : 0.99919999999999998
      },
      "source" : "Mentor Software Client",
      "unit" : "Meter"
   },
   "verticalCRS" : {
      "id" : "GEOID"
   }
})X";


    GeoCoordinates::BaseGCSPtr theGCS;

    theGCS = GeoCoordinates::BaseGCS::CreateGCS();

    Utf8String errorMessage;
    EXPECT_TRUE(SUCCESS == theGCS->FromJson(BeJsDocument(testJson), errorMessage));
    }

/*---------------------------------------------------------------------------------**//**
* JSon init simply specifying an EPSG code
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, BasicSetFromJsonWithEPSGCode)
    {
    Utf8String testJson = R"X({
   "horizontalCRS" : {
      "epsg" : 27700,
      "id" : "XYZ"
   },
   "verticalCRS" : {
      "id" : "GEOID"
   }
   })X";

    GeoCoordinates::BaseGCSPtr theGCS;

    theGCS = GeoCoordinates::BaseGCS::CreateGCS();

    Utf8String errorMessage;
    EXPECT_TRUE(SUCCESS == theGCS->FromJson(BeJsDocument(testJson), errorMessage));

    EXPECT_TRUE(theGCS->GetStoredEPSGCode() == 27700);
    }

/*---------------------------------------------------------------------------------**//**
* Unit tests
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, UnitRatioToMeterTest)
    {
    GeoCoordinates::BaseGCSPtr theGCS;

    theGCS = GeoCoordinates::BaseGCS::CreateGCS("UTM84-10N");

    EXPECT_NEAR(theGCS->UnitsFromMeters(), 1.0, 0.000001);

    theGCS = NULL;

    theGCS = GeoCoordinates::BaseGCS::CreateGCS("AK-10");

    EXPECT_NEAR(theGCS->UnitsFromMeters(), 1/ 0.3048006096, 0.000001);

    theGCS = NULL;

    theGCS = GeoCoordinates::BaseGCS::CreateGCS("CookNorthShoreMN-IF");

    EXPECT_NEAR(theGCS->UnitsFromMeters(), 1 / 0.3048, 0.000001);

    theGCS = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* Various Vertical Datum in JSON test
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, BasicSetVariousVerticalDatumNAD27FromJson)
    {
    Utf8String testJsonGeoid = R"X({
   "horizontalCRS" : {
      "area" : {
         "latitude" : {
            "max" : 84.0,
            "min" : 48.0
         },
         "longitude" : {
            "max" : -109.50,
            "min" : -120.50
         }
      },
      "datumId" : "NAD27",
      "description" : "",
      "id" : "XYZ",
      "projection" : {
         "centralMeridian" : -115.0,
         "falseEasting" : 0.0,
         "falseNorthing" : 0.0,
         "latitudeOfOrigin" : 0.0,
         "method" : "TransverseMercator",
         "scaleFactor" : 0.99919999999999998
      },
      "source" : "Mentor Software Client",
      "unit" : "Meter"
   },
   "verticalCRS" : {
      "id" : "GEOID"
   }
   })X";

    Utf8String testJsonEllipsoid = R"X({
   "horizontalCRS" : {
      "area" : {
         "latitude" : {
            "max" : 84.0,
            "min" : 48.0
         },
         "longitude" : {
            "max" : -109.50,
            "min" : -120.50
         }
      },
      "datumId" : "NAD27",
      "description" : "",
      "id" : "XYZ",
      "projection" : {
         "centralMeridian" : -115.0,
         "falseEasting" : 0.0,
         "falseNorthing" : 0.0,
         "latitudeOfOrigin" : 0.0,
         "method" : "TransverseMercator",
         "scaleFactor" : 0.99919999999999998
      },
      "source" : "Mentor Software Client",
      "unit" : "Meter"
   },
   "verticalCRS" : {
      "id" : "ELLIPSOID"
   }
   })X";

    Utf8String testJsonNGVD29 = R"X({
   "horizontalCRS" : {
      "area" : {
         "latitude" : {
            "max" : 84.0,
            "min" : 48.0
         },
         "longitude" : {
            "max" : -109.50,
            "min" : -120.50
         }
      },
      "datumId" : "NAD27",
      "description" : "",
      "id" : "XYZ",
      "projection" : {
         "centralMeridian" : -115.0,
         "falseEasting" : 0.0,
         "falseNorthing" : 0.0,
         "latitudeOfOrigin" : 0.0,
         "method" : "TransverseMercator",
         "scaleFactor" : 0.99919999999999998
      },
      "source" : "Mentor Software Client",
      "unit" : "Meter"
   },
   "verticalCRS" : {
      "id" : "NGVD29"
   }
   })X";

    Utf8String testJsonNAVD88 = R"X({
   "horizontalCRS" : {
      "area" : {
         "latitude" : {
            "max" : 84.0,
            "min" : 48.0
         },
         "longitude" : {
            "max" : -109.50,
            "min" : -120.50
         }
      },
      "datumId" : "NAD27",
      "description" : "",
      "id" : "XYZ",
      "projection" : {
         "centralMeridian" : -115.0,
         "falseEasting" : 0.0,
         "falseNorthing" : 0.0,
         "latitudeOfOrigin" : 0.0,
         "method" : "TransverseMercator",
         "scaleFactor" : 0.99919999999999998
      },
      "source" : "Mentor Software Client",
      "unit" : "Meter"
   },
   "verticalCRS" : {
      "id" : "NAVD88"
   }
})X";

    Utf8String testJsonLocalEllipsoid = R"X({
   "horizontalCRS" : {
      "area" : {
         "latitude" : {
            "max" : 84.0,
            "min" : 48.0
         },
         "longitude" : {
            "max" : -109.50,
            "min" : -120.50
         }
      },
      "datumId" : "NAD27",
      "description" : "",
      "id" : "XYZ",
      "projection" : {
         "centralMeridian" : -115.0,
         "falseEasting" : 0.0,
         "falseNorthing" : 0.0,
         "latitudeOfOrigin" : 0.0,
         "method" : "TransverseMercator",
         "scaleFactor" : 0.99919999999999998
      },
      "source" : "Mentor Software Client",
      "unit" : "Meter"
   },
   "verticalCRS" : {
      "id" : "LOCAL_ELLIPSOID"
   }
})X";

    GeoCoordinates::BaseGCSPtr theGCS;

    theGCS = GeoCoordinates::BaseGCS::CreateGCS();

    Utf8String errorMessage;
    EXPECT_TRUE(SUCCESS == theGCS->FromJson(BeJsDocument(testJsonGeoid), errorMessage));
    EXPECT_TRUE(theGCS->IsValid());
    EXPECT_TRUE(theGCS->GetVerticalDatumCode() == GeoCoordinates::vdcGeoid);

    EXPECT_TRUE(SUCCESS == theGCS->FromJson(BeJsDocument(testJsonEllipsoid), errorMessage));
    EXPECT_TRUE(theGCS->IsValid());
    EXPECT_TRUE(theGCS->GetVerticalDatumCode() == GeoCoordinates::vdcEllipsoid);

    EXPECT_TRUE(SUCCESS == theGCS->FromJson(BeJsDocument(testJsonNGVD29), errorMessage));
    EXPECT_TRUE(theGCS->IsValid());
    EXPECT_TRUE(theGCS->GetVerticalDatumCode() == GeoCoordinates::vdcNGVD29);

    EXPECT_TRUE(SUCCESS == theGCS->FromJson(BeJsDocument(testJsonNAVD88), errorMessage));
    EXPECT_TRUE(theGCS->IsValid());
    EXPECT_TRUE(theGCS->GetVerticalDatumCode() == GeoCoordinates::vdcNAVD88);

    EXPECT_TRUE(SUCCESS != theGCS->FromJson(BeJsDocument(testJsonLocalEllipsoid), errorMessage)); // Invalid
    EXPECT_TRUE(theGCS->IsValid());
    EXPECT_TRUE(theGCS->GetVerticalDatumCode() == GeoCoordinates::vdcNAVD88); // Unchanged
    }

/*---------------------------------------------------------------------------------**//**
* Various Vertical Datum in JSON test
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGCSUnitTests, BasicSetVariousVerticalDatumOSGBFromJson)
{
    Utf8String testJsonGeoid = R"X({
   "horizontalCRS" : {
      "area" : {
         "latitude" : {
            "max" : 84.0,
            "min" : 48.0
         },
         "longitude" : {
            "max" : 1.50,
            "min" : -2.50
         }
      },
      "datumId" : "OSGB",
      "description" : "",
      "id" : "XYZ",
      "projection" : {
         "centralMeridian" : -1.0,
         "falseEasting" : 0.0,
         "falseNorthing" : 0.0,
         "latitudeOfOrigin" : 0.0,
         "method" : "TransverseMercator",
         "scaleFactor" : 0.99919999999999998
      },
      "source" : "Mentor Software Client",
      "unit" : "Meter"
   },
   "verticalCRS" : {
      "id" : "GEOID"
   }
   })X";

    Utf8String testJsonEllipsoid = R"X({
    "horizontalCRS" : {
      "area" : {
         "latitude" : {
            "max" : 84.0,
            "min" : 48.0
         },
         "longitude" : {
            "max" : 1.50,
            "min" : -2.50
         }
      },
      "datumId" : "OSGB",
      "description" : "",
      "id" : "XYZ",
      "projection" : {
         "centralMeridian" : -1.0,
         "falseEasting" : 0.0,
         "falseNorthing" : 0.0,
         "latitudeOfOrigin" : 0.0,
         "method" : "TransverseMercator",
         "scaleFactor" : 0.99919999999999998
      },
      "source" : "Mentor Software Client",
      "unit" : "Meter"
   },
   "verticalCRS" : {
      "id" : "ELLIPSOID"
   }
   })X";

    Utf8String testJsonNGVD29 = R"X({
   "horizontalCRS" : {
      "area" : {
         "latitude" : {
            "max" : 84.0,
            "min" : 48.0
         },
         "longitude" : {
            "max" : 1.50,
            "min" : -2.50
         }
      },
      "datumId" : "OSGB",
      "description" : "",
      "id" : "XYZ",
      "projection" : {
         "centralMeridian" : -1.0,
         "falseEasting" : 0.0,
         "falseNorthing" : 0.0,
         "latitudeOfOrigin" : 0.0,
         "method" : "TransverseMercator",
         "scaleFactor" : 0.99919999999999998
      },
      "source" : "Mentor Software Client",
      "unit" : "Meter"
   },
   "verticalCRS" : {
      "id" : "NGVD29"
   }
   })X";

    Utf8String testJsonNAVD88 = R"X({
   "horizontalCRS" : {
      "area" : {
         "latitude" : {
            "max" : 84.0,
            "min" : 48.0
         },
         "longitude" : {
            "max" : 1.50,
            "min" : -2.50
         }
      },
      "datumId" : "OSGB",
      "description" : "",
      "id" : "XYZ",
      "projection" : {
         "centralMeridian" : -1.0,
         "falseEasting" : 0.0,
         "falseNorthing" : 0.0,
         "latitudeOfOrigin" : 0.0,
         "method" : "TransverseMercator",
         "scaleFactor" : 0.99919999999999998
      },
      "source" : "Mentor Software Client",
      "unit" : "Meter"
   },
   "verticalCRS" : {
      "id" : "NAVD88"
   }
   })X";

    Utf8String testJsonLocalEllipsoid = R"X({
   "horizontalCRS" : {
      "area" : {
         "latitude" : {
            "max" : 84.0,
            "min" : 48.0
         },
         "longitude" : {
            "max" : 1.50,
            "min" : -2.50
         }
      },
      "datumId" : "OSGB",
      "description" : "",
      "id" : "XYZ",
      "projection" : {
         "centralMeridian" : -1.0,
         "falseEasting" : 0.0,
         "falseNorthing" : 0.0,
         "latitudeOfOrigin" : 0.0,
         "method" : "TransverseMercator",
         "scaleFactor" : 0.99919999999999998
      },
      "source" : "Mentor Software Client",
      "unit" : "Meter"
   },
   "verticalCRS" : {
      "id" : "LOCAL_ELLIPSOID"
   }
   })X";

        GeoCoordinates::BaseGCSPtr theGCS;

        theGCS = GeoCoordinates::BaseGCS::CreateGCS();

        Utf8String errorMessage;
        EXPECT_TRUE(SUCCESS == theGCS->FromJson(BeJsDocument(testJsonGeoid), errorMessage));
        EXPECT_TRUE(theGCS->IsValid());
        EXPECT_TRUE(theGCS->GetVerticalDatumCode() == GeoCoordinates::vdcGeoid);

        EXPECT_TRUE(SUCCESS == theGCS->FromJson(BeJsDocument(testJsonEllipsoid), errorMessage));
        EXPECT_TRUE(theGCS->IsValid());
        EXPECT_TRUE(theGCS->GetVerticalDatumCode() == GeoCoordinates::vdcEllipsoid);

        EXPECT_TRUE(SUCCESS != theGCS->FromJson(BeJsDocument(testJsonNGVD29), errorMessage));

        EXPECT_TRUE(SUCCESS != theGCS->FromJson(BeJsDocument(testJsonNAVD88), errorMessage));

        EXPECT_TRUE(SUCCESS == theGCS->FromJson(BeJsDocument(testJsonLocalEllipsoid), errorMessage));
        EXPECT_TRUE(theGCS->IsValid());
        EXPECT_TRUE(theGCS->GetVerticalDatumCode() == GeoCoordinates::vdcLocalEllipsoid);
    }

    /*---------------------------------------------------------------------------------**//**
    * Test description is retained if different from dictionary
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    TEST_F(BaseGCSUnitTests, OnJSONParseDescriptionIsRetained)
    {
    Utf8String BristishNationalGridWithDescr = R"X(
 {
      "horizontalCRS": {
        "id": "EPSG:27700",
        "description": "A DIFFERENT DESCRITION",
        "datumId": "EPSG:6277",
        "unit": "Meter",
        "projection": {
          "method": "TransverseMercator",
          "falseEasting": 400000,
          "falseNorthing": -100000,
          "centralMeridian": -2,
          "latitudeOfOrigin": 49,
          "scaleFactor": 0.999601272737422},
      "verticalCRS": {
        "id": "ELLIPSOID"},
      "additionalTransform": {
        "helmert2DWithZOffset": {
          "translationX": 284597.3343,
          "translationY": 79859.4651,
          "translationZ": 0,
          "rotDeg": 0.5263624458992088,
          "scale": 0.9996703340508721}}}})X";
    
        GeoCoordinates::BaseGCSPtr theGCS;

        theGCS = GeoCoordinates::BaseGCS::CreateGCS();

        Utf8String errorMessage;
        EXPECT_TRUE(SUCCESS == theGCS->FromJson(BeJsDocument(BristishNationalGridWithDescr), errorMessage));
        EXPECT_TRUE(theGCS->IsValid());
        EXPECT_TRUE(Utf8String(theGCS->GetDescription()) == "A DIFFERENT DESCRITION");
    }

    /*---------------------------------------------------------------------------------**//**
    * Test geodetic distance
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    TEST_F(BaseGCSUnitTests, GeodeticDistance)
    {
        GeoCoordinates::BaseGCSPtr theGCS;
        theGCS = GeoCoordinates::BaseGCS::CreateGCS("TN83F");
        ASSERT_TRUE(theGCS.IsValid() && theGCS->IsValid());

        GeoPoint start;
        start.latitude = 45.48;
        start.longitude = 71.24;
        start.elevation = 0.0;
        GeoPoint end;
        end.latitude = 45.40;
        end.longitude = 71.20;
        end.elevation = 0.0;

        double distance1 = 0.0;
        double azimuth1 = 0.0;
        double distance2 = 0.0;
        double azimuth2 = 0.0;
        EXPECT_TRUE(SUCCESS == theGCS->GetDistanceInMeters(&distance1, &azimuth1, start, end));

        EXPECT_TRUE(SUCCESS == theGCS->GetDistance(&distance2, &azimuth2, start, end));

        EXPECT_NEAR(distance1 * theGCS->UnitsFromMeters(), distance2, 0.00001);
        EXPECT_NEAR(azimuth1, azimuth2, 0.00000001);

        start.latitude = 45.48;
        start.longitude = 71.24;

        end.latitude = 45.40;
        end.longitude = 71.24;

        EXPECT_TRUE(SUCCESS == theGCS->GetDistanceInMeters(&distance1, &azimuth1, start, end));

        EXPECT_TRUE(SUCCESS == theGCS->GetDistance(&distance2, &azimuth2, start, end));

        EXPECT_NEAR(distance1 * theGCS->UnitsFromMeters(), distance2, 0.00001);
        EXPECT_NEAR(azimuth1, azimuth2, 0.00000001);
        EXPECT_NEAR(azimuth1, 180.0, 0.00000001);
 
    }

    /*---------------------------------------------------------------------------------**//**
    * Test geodetic distance
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    TEST_F(BaseGCSUnitTests, AzimuthEqualAreaReverse)
    {
        GeoCoordinates::BaseGCSPtr azimuthGCS;
        azimuthGCS = GeoCoordinates::BaseGCS::CreateGCS();

        Utf8String errorMsg;

        azimuthGCS->InitAzimuthalEqualArea(&errorMsg, "WGS84", "Meter", -122.69595344800067, 41.999606314287703, 0.0, 1.0, 0.0, 0.0, 1);

        ASSERT_TRUE(azimuthGCS.IsValid() && azimuthGCS->IsValid());

        GeoPoint xLatLong, yLatLong;
        azimuthGCS->LatLongFromCartesian(xLatLong, DPoint3d::From(1.0, 0.0, 0.0));
        azimuthGCS->LatLongFromCartesian(yLatLong, DPoint3d::From(0.0, 1.0, 0.0));

        DPoint3d xCartesian, yCartesian;
        azimuthGCS->CartesianFromLatLong(xCartesian, xLatLong);
        azimuthGCS->CartesianFromLatLong(yCartesian, yLatLong);
        double distanceX, distanceY;
        distanceX = xCartesian.Distance(DPoint3d::From(1.0, 0.0, 0.0));
        distanceY = yCartesian.Distance(DPoint3d::From(0.0, 1.0, 0.0));

        EXPECT_NEAR(distanceX, 0.0, 0.0001);
        EXPECT_NEAR(distanceY, 0.0, 0.0001);
    }

/*---------------------------------------------------------------------------------**//**
* Test alternate transform path
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseGCSUnitTests, AdditionalTransformPath)
{
    GeoCoordinates::DatumCP theDatum = GeoCoordinates::Datum::CreateDatum("NAD83/2011");

    ASSERT_TRUE(nullptr != theDatum);

    bvector<GeodeticTransformPathCP> const & toto = theDatum->GetAdditionalGeodeticTransformPaths();

    EXPECT_TRUE(toto.size() > 0);
    
    theDatum->Destroy();
}

/*---------------------------------------------------------------------------------**//**
* test all transformation to full json then back
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BaseGCSUnitTests, GCSTransformToFullJsonThenBack)
    {
    const bvector<Utf8String>& listOfGCS = GeoCoordTestCommon::GetSmallListOfGCS();

    double parameters[12] = {1.0, 0.0, 10.0, 20.0, 30.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

    GeoCoordinates::LocalTransformerPtr localTransform1 = GeoCoordinates::LocalTransformer::CreateLocalTransformer(GeoCoordinates::TRANSFORM_Helmert, parameters);

    double parameters2[12] = {0.0, 1.0, 0.0, -20.0, 300.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

    GeoCoordinates::LocalTransformerPtr localTransform2 = GeoCoordinates::LocalTransformer::CreateLocalTransformer(GeoCoordinates::TRANSFORM_Helmert, parameters2);

    double parameters3[12] = {2, 2, 0.0, -200.0, 450.0, 34.1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

    GeoCoordinates::LocalTransformerPtr localTransform3 = GeoCoordinates::LocalTransformer::CreateLocalTransformer(GeoCoordinates::TRANSFORM_Helmert, parameters3);

    for (int index = 0 ; index < listOfGCS.size() ; index++)
        {
        std::cerr << "Testing " << index << " of " << listOfGCS.size() << std::endl;

        Utf8String theKeyname(listOfGCS[index]);
        GeoCoordinates::BaseGCSPtr currentGCS = GeoCoordinates::BaseGCS::CreateGCS(theKeyname.c_str());

        if (!currentGCS.IsValid() || ! currentGCS->IsValid())
            continue;  // This occurs for weird GCS like OSGB-GPS-1997

        // vertical datum set
        int remainder;
        GeoCoordinates::DatumCP theDatum = currentGCS->GetDatum(); // May be null for ellipsoid based GCS
        
        if (nullptr != theDatum && (theDatum->IsNAD27() || theDatum->IsNAD83()))
            {
            remainder = index % 5;
            if (remainder == 0)
                currentGCS->SetVerticalDatumCode(GeoCoordinates::vdcGeoid);
            else if (remainder == 1)
                currentGCS->SetVerticalDatumCode(GeoCoordinates::vdcEllipsoid);
            else if (remainder == 2)
                currentGCS->SetVerticalDatumCode(GeoCoordinates::vdcNGVD29);
            else if (remainder == 3)
                currentGCS->SetVerticalDatumCode(GeoCoordinates::vdcNAVD88);
            else if (remainder == 4)
                currentGCS->SetVerticalDatumCode(GeoCoordinates::vdcLocalEllipsoid);
            }
        else
            {
            remainder = index % 3;

            if (remainder == 0)
                currentGCS->SetVerticalDatumCode(GeoCoordinates::vdcGeoid);
            else if (remainder == 1)
                currentGCS->SetVerticalDatumCode(GeoCoordinates::vdcEllipsoid);
            else if (remainder == 2)
                currentGCS->SetVerticalDatumCode(GeoCoordinates::vdcLocalEllipsoid);
            }

        remainder = (index + 1) % 5;

        if (remainder == 0)
            currentGCS->SetLocalTransformer(localTransform1.get());
        else if (remainder == 1)
            currentGCS->SetLocalTransformer(localTransform2.get());
        else if (remainder == 2)
            currentGCS->SetLocalTransformer(localTransform3.get());

        if (currentGCS.IsValid() && currentGCS->IsValid())
            {
            Json::Value result;

            if (SUCCESS == currentGCS->ToJson(result, true))
                {
                // Transform to string (for debug purposes)
                Utf8String resultString = result.toStyledString();

                // Make sure that domain is specified
                EXPECT_TRUE(!result["horizontalCRS"].isNull());
                EXPECT_TRUE(!result["verticalCRS"].isNull());

                Utf8String resultString2 = result.toStyledString();

                // Sabotage GCS name to make sure everything is parsed
                result["horizontalCRS"]["id"] = "XYZ";
            
                GeoCoordinates::BaseGCSPtr resultGCS = GeoCoordinates::BaseGCS::CreateGCS();

                Utf8String errMessage;
                EXPECT_EQ(SUCCESS, resultGCS->FromJson(result, errMessage)) << errMessage.c_str();

                EXPECT_TRUE(currentGCS->IsEquivalent(*resultGCS));
                
                EXPECT_TRUE(GeoCoordTestCommon::doubleSame(currentGCS->GetMinimumLatitude(), resultGCS->GetMinimumLatitude()));
                EXPECT_TRUE(GeoCoordTestCommon::doubleSame(currentGCS->GetMinimumLongitude(), resultGCS->GetMinimumLongitude()));
                EXPECT_TRUE(GeoCoordTestCommon::doubleSame(currentGCS->GetMaximumLatitude(), resultGCS->GetMaximumLatitude()));
                EXPECT_TRUE(GeoCoordTestCommon::doubleSame(currentGCS->GetMaximumLongitude(), resultGCS->GetMaximumLongitude()));

                EXPECT_TRUE(GeoCoordTestCommon::doubleSame(currentGCS->GetMinimumUsefulLatitude(), resultGCS->GetMinimumUsefulLatitude()));
                EXPECT_TRUE(GeoCoordTestCommon::doubleSame(currentGCS->GetMinimumUsefulLongitude(), resultGCS->GetMinimumUsefulLongitude()));
                EXPECT_TRUE(GeoCoordTestCommon::doubleSame(currentGCS->GetMaximumUsefulLatitude(), resultGCS->GetMaximumUsefulLatitude()));
                EXPECT_TRUE(GeoCoordTestCommon::doubleSame(currentGCS->GetMaximumUsefulLongitude(), resultGCS->GetMaximumUsefulLongitude()));
                EXPECT_STREQ(currentGCS->GetDescription(), resultGCS->GetDescription());
                Utf8String source1, source2;
                EXPECT_STREQ(currentGCS->GetSource(source1), resultGCS->GetSource(source2));
                }
            }
        }
    }