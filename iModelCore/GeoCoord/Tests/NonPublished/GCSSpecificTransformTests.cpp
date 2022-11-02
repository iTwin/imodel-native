//:>--------------------------------------------------------------------------------------+
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See LICENSE.md in the repository root for full copyright notice.
//:>+--------------------------------------------------------------------------------------

#include <Bentley/BeTest.h>

#include <GeoCoord/BaseGeoCoord.h>
#include "GeoCoordTestCommon.h"

using namespace ::testing;

 /*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
class GCSSpecificTransformTests : public ::testing::Test
    {   

    public:
        virtual void SetUp() { GeoCoordTestCommon::Initialize(); };
        virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

        GCSSpecificTransformTests() {};
        ~GCSSpecificTransformTests() {};
    };

/*---------------------------------------------------------------------------------**//**
* Domain tests
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (GCSSpecificTransformTests, SpecificElevationTestIsrael)
    {
    GeoCoordinates::BaseGCSPtr firstGCS;
    GeoCoordinates::BaseGCSPtr secondGCS;
   
    firstGCS = GeoCoordinates::BaseGCS::CreateGCS("UTM84-36N");
    secondGCS = GeoCoordinates::BaseGCS::CreateGCS();

    Utf8String wellKnownText = "PROJCS[\"Israel 1993 / Israeli T\",GEOGCS[\"EPSG:4141\",DATUM[\"EPSG:6141\",SPHEROID[\"EPSG:7019\",6378137.000,298.25722210],TOWGS84[-48.0000,55.0000,52.0000,0.000000,0.000000,0.000000,0.00000000]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"false_easting\",219529.584],PARAMETER[\"false_northing\",626907.390],PARAMETER[\"scale_factor\",1.000006700000],PARAMETER[\"central_meridian\",35.20451694444446],PARAMETER[\"latitude_of_origin\",31.73439361111112],UNIT[\"Meter\",1.00000000000000]]";
    secondGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, wellKnownText.c_str());
    secondGCS->SetVerticalDatumCode(GeoCoordinates::vdcLocalEllipsoid);

    ASSERT_TRUE(firstGCS.IsValid() && firstGCS->IsValid());
    ASSERT_TRUE(secondGCS.IsValid() && secondGCS->IsValid());

    ASSERT_TRUE(firstGCS->GetVerticalDatumCode() == GeoCoordinates::vdcFromDatum);
    ASSERT_TRUE(secondGCS->GetVerticalDatumCode() == GeoCoordinates::vdcLocalEllipsoid);

    DPoint3d resultPoint = { 0,0,0 };
    DPoint3d secondResultPoint = { 0,0,0 };
    DPoint3d inputPoint;

    inputPoint.x = 800000;
    inputPoint.y = 3000000;
    inputPoint.z = 10.24;

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
* Specific WKT test (to be removed)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (GCSSpecificTransformTests, ElevationTransfo1)
    {
    GeoCoordinates::BaseGCSPtr fromGCS;
    GeoCoordinates::BaseGCSPtr toGCS;

       fromGCS = GeoCoordinates::BaseGCS::CreateGCS();
    toGCS = GeoCoordinates::BaseGCS::CreateGCS();

    Utf8String wellKnownText1 = "PROJCS[\"WGS 84 / UTM zone 32N\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",9],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH],AUTHORITY[\"EPSG\",\"32632\"]]";

    Utf8String wellKnownText2 = "COMPD_CS[\"WGS 84 / UTM zone 32N + EGM96 geoid height\",PROJCS[\"WGS 84 / UTM zone32N\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",9],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH],AUTHORITY[\"EPSG\",\"32632\"]],VERT_CS[\"EGM96 geoid height\",VERT_DATUM[\"EGM96 geoid\",2005,EXTENSION[\"PROJ4_GRIDS\",\"egm96_15.gtx\"],AUTHORITY[\"EPSG\",\"5171\"]],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Up\",UP],AUTHORITY[\"EPSG\",\"5773\"]]]";

    EXPECT_TRUE(SUCCESS == fromGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, wellKnownText1.c_str()));

    EXPECT_TRUE(fromGCS->IsValid());

    EXPECT_TRUE(SUCCESS == toGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, wellKnownText2.c_str()));

    EXPECT_TRUE(toGCS->IsValid());

    fromGCS->SetReprojectElevation(true);
    toGCS->SetReprojectElevation(true);

    GeoPoint fromGeoPoint;
    GeoPoint toGeoPoint = { 0,0,0 };
   
    fromGeoPoint.longitude=6.6922634698675436;
    fromGeoPoint.latitude=43.955776401074210;
    fromGeoPoint.elevation=611.48800000000006;

    EXPECT_TRUE(REPROJECT_Success == fromGCS->LatLongFromLatLong(toGeoPoint, fromGeoPoint, *toGCS));

    EXPECT_TRUE(toGeoPoint.elevation != fromGeoPoint.elevation);
    }


   


