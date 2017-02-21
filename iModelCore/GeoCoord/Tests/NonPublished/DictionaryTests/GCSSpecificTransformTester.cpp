//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/NonPublished/DictionaryTests/GCSSpecificTransformTester.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <Bentley/BeTest.h>

#include <GeoCoord/BaseGeoCoord.h>
#include <GeoCoord/GCSLibrary.h>
#include "GCSSpecificTransformTester.h"

using namespace ::testing;

GCSSpecificTransformTester::GCSSpecificTransformTester() 
    {
        BeTest::Host& host = BeTest::GetHost();

        BeFileName path;
        host.GetDgnPlatformAssetsDirectory(path);

        path.AppendToPath (L"DgnGeoCoord");

        GeoCoordinates::BaseGCS::Initialize(path.c_str());
    }

//==================================================================================
// Domain
//==================================================================================
TEST_F (GCSSpecificTransformTester, LatLongToFromXYZ)
    {
    GeoCoordinates::BaseGCSPtr currentGCSEllipsoid;
    GeoCoordinates::BaseGCSPtr currentGCSGeoid;

   
    // First test using WGS84 lat/long
    currentGCSEllipsoid = GeoCoordinates::BaseGCS::CreateGCS(L"LL84");

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
    currentGCSGeoid =  GeoCoordinates::BaseGCS::CreateGCS(L"LL84");
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

    // Equator on Greenwhich
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

//==================================================================================
// Domain
//==================================================================================
TEST_F (GCSSpecificTransformTester, KuwaitUtilityInstanciationFailureTest)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

   
    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"KuwaitUtility.KTM");

    EXPECT_TRUE(currentGCS.IsValid());
    }

    
//==================================================================================
// Domain
//==================================================================================
TEST_F (GCSSpecificTransformTester, SpecificWKT1)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

   
    currentGCS = GeoCoordinates::BaseGCS::CreateGCS();

    WString wellKnownText = L"COMPD_CS[\"LKS92 / Latvia TM + EGM96 geoid height\",PROJCS[\"LKS92 / Latvia TM\",GEOGCS[\"LKS92\",DATUM[\"Latvia_1992\",SPHEROID[\"GRS 1980\",6378137,298.257222101,AUTHORITY[\"EPSG\",\"7019\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6661\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4661\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",24],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",-6000000],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AUTHORITY[\"EPSG\",\"3059\"]],VERT_CS[\"EGM96 geoid height\",VERT_DATUM[\"EGM96 geoid\",2005,AUTHORITY[\"EPSG\",\"5171\"],EXTENSION[\"PROJ4_GRIDS\",\"egm96_15.gtx\"]],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Up\",UP],AUTHORITY[\"EPSG\",\"5773\"]]]";

    EXPECT_TRUE(SUCCESS == currentGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, wellKnownText.c_str()));

    EXPECT_TRUE(currentGCS->IsValid());

    EXPECT_TRUE(currentGCS->GetVerticalDatumCode() == GeoCoordinates::vdcGeoid);
    }

//==================================================================================
// Domain
//==================================================================================
TEST_F (GCSSpecificTransformTester, SpecificWKT2)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

   
    currentGCS = GeoCoordinates::BaseGCS::CreateGCS();

    WString wellKnownText = L"PROJCS[\"WGS 84 / UTM zone 13N\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",-105],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH],AUTHORITY[\"EPSG\",\"32613\"]]";

    EXPECT_TRUE(SUCCESS == currentGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, wellKnownText.c_str()));

    EXPECT_TRUE(currentGCS->IsValid());

    }

//==================================================================================
// Domain
//==================================================================================
TEST_F (GCSSpecificTransformTester, SpecificWKT3)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

   
    currentGCS = GeoCoordinates::BaseGCS::CreateGCS();

    WString wellKnownText = L"PROJCS[\"WGS 84 / UTM zone 13N\",GEOGCS[\"EPSG:6326\",DATUM[\"EPSG:6326\",SPHEROID[\"EPSG:7030\",6378137.000,298.25722360]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]],PROJECTION[\"Transverse Mercator\"],PARAMETER[\"False Easting\",500000.000],PARAMETER[\"False Northing\",0.000],PARAMETER[\"Scale Reduction\",0.999600000000],PARAMETER[\"Central Meridian\",-105.00000000000003],PARAMETER[\"Origin Latitude\",0.00000000000000],UNIT[\"Meter\",1.00000000000000]]";

    EXPECT_TRUE(SUCCESS == currentGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, wellKnownText.c_str()));

    EXPECT_TRUE(currentGCS->IsValid());

    }

//==================================================================================
// Domain
//==================================================================================
TEST_F (GCSSpecificTransformTester, SpecificWKT4)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

   
    currentGCS = GeoCoordinates::BaseGCS::CreateGCS();

    WString wellKnownText = L"PROJCS[\"UTM84-13N\",GEOGCS[\"LL84\",DATUM[\"WGS84\",SPHEROID[\"WGS84\",6378137.000,298.25722293]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]],PROJECTION[\"Universal Transverse Mercator System\"],PARAMETER[\"UTM Zone Number (1 - 60)\",13.0],PARAMETER[\"Hemisphere, North or South\",1.0],UNIT[\"Meter\",1.00000000000000]]";

    EXPECT_TRUE(SUCCESS == currentGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, wellKnownText.c_str()));

    EXPECT_TRUE(currentGCS->IsValid());

    }

//==================================================================================
// Domain
//==================================================================================
TEST_F (GCSSpecificTransformTester, SpecificWKT5)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

   
    currentGCS = GeoCoordinates::BaseGCS::CreateGCS();

    WString wellKnownText = L"COMPD_CS[\"WGS84/UTM17N + EGM96 geoid height\",PROJCS[\"WGS 84 / UTM zone 17N\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",-81],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH],AUTHORITY[\"EPSG\",\"32617\"]],VERT_CS[\"EGM96 geoid height\",VERT_DATUM[\"EGM96 geoid\",2005]]]";

    EXPECT_TRUE(SUCCESS == currentGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, wellKnownText.c_str()));

    EXPECT_TRUE(currentGCS->IsValid());

    }


//==================================================================================
// Domain
//==================================================================================
TEST_F (GCSSpecificTransformTester, SpecificWKT6)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

   
    currentGCS = GeoCoordinates::BaseGCS::CreateGCS();

//    WString wellKnownText = L"PROJCS[\"NAD83(2011) / Florida East (ftUS)\",GEOGCS[\"NAD83(2011)\",DATUM[\"NAD83_National_Spatial_Reference_System_2011\",SPHEROID[\"GRS 1980\",6378137,298.257222101,AUTHORITY[\"EPSG\",\"7019\"]],AUTHORITY[\"EPSG\",\"1116\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"6318\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",24.33333333333333],PARAMETER[\"central_meridian\",-81],PARAMETER[\"scale_factor\",0.999941177],PARAMETER[\"false_easting\",656166.667],PARAMETER[\"false_northing\",0],UNIT[\"US survey foot\",0.3048006096012192,AUTHORITY[\"EPSG\",\"9003\"]],AXIS[\"X\",EAST],AXIS[\"Y\",NORTH],AUTHORITY[\"EPSG\",\"6438\"]]";

    WString wellKnownText = L"PROJCS[\"NAD83(2011) / Florida East (ftUS)\",GEOGCS[\"NAD83(2011)\",DATUM[\"NAD_1983_2011\",SPHEROID[\"GRS 1980\",6378137,298.257222101,AUTHORITY[\"EPSG\",\"7019\"]],AUTHORITY[\"EPSG\",\"1116\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"6318\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",24.33333333333333],PARAMETER[\"central_meridian\",-81],PARAMETER[\"scale_factor\",0.999941177],PARAMETER[\"false_easting\",656166.667],PARAMETER[\"false_northing\",0],UNIT[\"US survey foot\",0.3048006096012192,AUTHORITY[\"EPSG\",\"9003\"]],AXIS[\"X\",EAST],AXIS[\"Y\",NORTH],AUTHORITY[\"EPSG\",\"6438\"]]";

    EXPECT_TRUE(SUCCESS == currentGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, wellKnownText.c_str()));

    EXPECT_TRUE(currentGCS->IsValid());

    }


//==================================================================================
// Domain
//==================================================================================
TEST_F (GCSSpecificTransformTester, ElevationTransfo1)
    {
    GeoCoordinates::BaseGCSPtr fromGCS;
    GeoCoordinates::BaseGCSPtr toGCS;

   
    fromGCS = GeoCoordinates::BaseGCS::CreateGCS();
    toGCS = GeoCoordinates::BaseGCS::CreateGCS();

    WString wellKnownText1 = L"PROJCS[\"WGS 84 / UTM zone 32N\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",9],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH],AUTHORITY[\"EPSG\",\"32632\"]]";

    WString wellKnownText2 = L"COMPD_CS[\"WGS 84 / UTM zone 32N + EGM96 geoid height\",PROJCS[\"WGS 84 / UTM zone32N\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",9],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH],AUTHORITY[\"EPSG\",\"32632\"]],VERT_CS[\"EGM96 geoid height\",VERT_DATUM[\"EGM96 geoid\",2005,EXTENSION[\"PROJ4_GRIDS\",\"egm96_15.gtx\"],AUTHORITY[\"EPSG\",\"5171\"]],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Up\",UP],AUTHORITY[\"EPSG\",\"5773\"]]]";

    EXPECT_TRUE(SUCCESS == fromGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, wellKnownText1.c_str()));


    EXPECT_TRUE(fromGCS->IsValid());

    EXPECT_TRUE(SUCCESS == toGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, wellKnownText2.c_str()));

    EXPECT_TRUE(toGCS->IsValid());

    fromGCS->SetReprojectElevation(true);
    toGCS->SetReprojectElevation(true);

    GeoPoint fromGeoPoint;
    GeoPoint toGeoPoint;
   

    fromGeoPoint.longitude=6.6922634698675436;
    fromGeoPoint.latitude=43.955776401074210;
    fromGeoPoint.elevation=611.48800000000006;


    EXPECT_TRUE(REPROJECT_Success == fromGCS->LatLongFromLatLong(toGeoPoint, fromGeoPoint, *toGCS));

    EXPECT_TRUE(toGeoPoint.elevation != fromGeoPoint.elevation);


    }

//==================================================================================
// Test generation WKT for london grid
//==================================================================================
TEST_F (GCSSpecificTransformTester, SpecificWKTGenerationLondonGrid)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

   
    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"LondonGrid");

    WString wellKnownText = L"";

    EXPECT_TRUE(SUCCESS == currentGCS->GetWellKnownText(wellKnownText, GeoCoordinates::BaseGCS::wktFlavorOGC, false));

    EXPECT_TRUE(currentGCS->IsValid());

    }


//==================================================================================
// Domain
//==================================================================================
TEST_F (GCSSpecificTransformTester, IndianaDOT_InGCSTests)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

  
    GeoPoint2d testGeoPoint;
    testGeoPoint.latitude = 42.0;
    testGeoPoint.longitude = -85.0;
    
    DPoint2d resultPoint;
    
    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Adams");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 235857.321, 0.001);
    EXPECT_NEAR(resultPoint.y, 197042.576, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Allen");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 244142.667, 0.001);
    EXPECT_NEAR(resultPoint.y, 158173.879, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Bartholomew");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 310425.254, 0.001);
    EXPECT_NEAR(resultPoint.y, 369491.117, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Benton");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 430567.721, 0.001);
    EXPECT_NEAR(resultPoint.y, 210705.421, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Blackford");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 273141.593, 0.001);
    EXPECT_NEAR(resultPoint.y, 252641.732, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Boone");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 364282.128, 0.001);
    EXPECT_NEAR(resultPoint.y, 303618.467, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Brown");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 347710.206, 0.001);
    EXPECT_NEAR(resultPoint.y, 369960.596, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Carroll");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 376709.323, 0.001);
    EXPECT_NEAR(resultPoint.y, 215014.436, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Cass");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 355995.543, 0.001);
    EXPECT_NEAR(resultPoint.y, 197988.761, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Clark");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 289711.598, 0.001);
    EXPECT_NEAR(resultPoint.y, 463672.324, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Clay");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 418137.860, 0.001);
    EXPECT_NEAR(resultPoint.y, 354724.927, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Clinton");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 372567.295, 0.001);
    EXPECT_NEAR(resultPoint.y, 242697.736, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Crawford");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 364280.761 , 0.001);
    EXPECT_NEAR(resultPoint.y, 470138.638, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Daviess");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 413993.876, 0.001);
    EXPECT_NEAR(resultPoint.y, 432329.722, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Dearborn");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 231714.683, 0.001);
    EXPECT_NEAR(resultPoint.y, 408002.771, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Decatur");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 293855.057, 0.001);
    EXPECT_NEAR(resultPoint.y, 358247.257, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-DeKalb");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 235857.313, 0.001);
    EXPECT_NEAR(resultPoint.y, 119303.715, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Delaware");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 273141.593, 0.001);
    EXPECT_NEAR(resultPoint.y, 252641.732, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Dubois");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 401565.536, 0.001);
    EXPECT_NEAR(resultPoint.y, 459787.679, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Elkhart");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 310425.747, 0.001);
    EXPECT_NEAR(resultPoint.y, 186285.784, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Fayette");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 244142.696 , 0.001);
    EXPECT_NEAR(resultPoint.y, 341391.242, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Floyd");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 289711.598, 0.001);
    EXPECT_NEAR(resultPoint.y, 463672.324, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Fountain");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 430566.959, 0.001);
    EXPECT_NEAR(resultPoint.y, 266225.351, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Franklin");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 244142.696, 0.001);
    EXPECT_NEAR(resultPoint.y, 341391.242, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Fulton");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 347710.313, 0.001);
    EXPECT_NEAR(resultPoint.y, 158990.378, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Gibson");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 459565.693, 0.001);
    EXPECT_NEAR(resultPoint.y, 466893.589, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Grant");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 297997.659, 0.001);
    EXPECT_NEAR(resultPoint.y, 219487.851, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Greene");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 413993.876, 0.001);
    EXPECT_NEAR(resultPoint.y, 432329.722, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Hamilton");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 322854.027, 0.001);
    EXPECT_NEAR(resultPoint.y, 269702.978, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Hancock");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 306283.225, 0.001);
    EXPECT_NEAR(resultPoint.y, 297287.836, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Harrison");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 335281.630, 0.001);
    EXPECT_NEAR(resultPoint.y, 486340.679, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Hendricks");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 364282.128 , 0.001);
    EXPECT_NEAR(resultPoint.y, 303618.467, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Henry");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 277284.487, 0.001);
    EXPECT_NEAR(resultPoint.y, 285974.624, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Howard");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 335282.011, 0.001);
    EXPECT_NEAR(resultPoint.y, 219890.110, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Huntington");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 281426.845, 0.001);
    EXPECT_NEAR(resultPoint.y, 186057.323, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Jackson");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 318710.339 , 0.001);
    EXPECT_NEAR(resultPoint.y, 402881.394, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Jasper");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 413995.442, 0.001);
    EXPECT_NEAR(resultPoint.y, 182516.908, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Jay");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 240000.000, 0.001);
    EXPECT_NEAR(resultPoint.y, 224803.768, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Jefferson");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 268998.598, 0.001);
    EXPECT_NEAR(resultPoint.y, 419157.924, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Jennings");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 306282.496, 0.001);
    EXPECT_NEAR(resultPoint.y, 391654.136, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Johnson");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 335282.011, 0.001);
    EXPECT_NEAR(resultPoint.y, 336476.565, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Knox");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 442993.988 , 0.001);
    EXPECT_NEAR(resultPoint.y, 438649.761, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Kosciusko");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 310425.747, 0.001);
    EXPECT_NEAR(resultPoint.y, 186285.784, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-LaGrange");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 277284.263 , 0.001);
    EXPECT_NEAR(resultPoint.y, 119400.561, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Lake");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 438853.181, 0.001);
    EXPECT_NEAR(resultPoint.y, 183170.285, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-LaPorte");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 384995.147, 0.001);
    EXPECT_NEAR(resultPoint.y, 159654.089, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Lawrence");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 364280.761 , 0.001);
    EXPECT_NEAR(resultPoint.y, 470138.638, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Madison");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 306283.225 , 0.001);
    EXPECT_NEAR(resultPoint.y, 297287.836, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Marion");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 335282.011, 0.001);
    EXPECT_NEAR(resultPoint.y, 336476.565, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Marshall");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 347710.313, 0.001);
    EXPECT_NEAR(resultPoint.y, 158990.378, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Martin");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 401565.536 , 0.001);
    EXPECT_NEAR(resultPoint.y, 459787.679, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Miami");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 335282.011, 0.001);
    EXPECT_NEAR(resultPoint.y, 219890.110, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Monroe");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 364281.134, 0.001);
    EXPECT_NEAR(resultPoint.y, 375781.826, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Montgomery");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 401567.313, 0.001);
    EXPECT_NEAR(resultPoint.y, 321022.846, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Morgan");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 364281.134, 0.001);
    EXPECT_NEAR(resultPoint.y, 375781.826, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Newton");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 438853.181, 0.001);
    EXPECT_NEAR(resultPoint.y, 183170.285, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Noble");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 277284.263, 0.001);
    EXPECT_NEAR(resultPoint.y, 119400.561, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Ohio");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 231714.683, 0.001);
    EXPECT_NEAR(resultPoint.y, 408002.771, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Orange");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 364280.761, 0.001);
    EXPECT_NEAR(resultPoint.y, 470138.638, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Owen");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 397423.611, 0.001);
    EXPECT_NEAR(resultPoint.y, 354235.478, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Parke");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 434709.379, 0.001);
    EXPECT_NEAR(resultPoint.y, 305198.679, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Perry");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 380851.312, 0.001);
    EXPECT_NEAR(resultPoint.y, 503745.518, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Pike");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 430565.053, 0.001);
    EXPECT_NEAR(resultPoint.y, 499355.108, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Porter");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 413995.442, 0.001);
    EXPECT_NEAR(resultPoint.y, 182516.908, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Posey");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 484424.394, 0.001);
    EXPECT_NEAR(resultPoint.y, 512105.909, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Pulaski");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 384995.147, 0.001);
    EXPECT_NEAR(resultPoint.y, 159654.089, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Putnam");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 401567.313, 0.001);
    EXPECT_NEAR(resultPoint.y, 321022.846, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Randolph");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 244142.720, 0.001);
    EXPECT_NEAR(resultPoint.y, 291429.823, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Ripley");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 264856.185, 0.001);
    EXPECT_NEAR(resultPoint.y, 380290.971, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Rush");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 293855.057, 0.001);
    EXPECT_NEAR(resultPoint.y, 358247.257, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-St-Joseph");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 347710.313 , 0.001);
    EXPECT_NEAR(resultPoint.y, 158990.378, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Scott");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 289711.598, 0.001);
    EXPECT_NEAR(resultPoint.y, 463672.324, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Shelby");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 314568.249, 0.001);
    EXPECT_NEAR(resultPoint.y, 336228.284, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Spencer");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 409850.295, 0.001);
    EXPECT_NEAR(resultPoint.y, 509927.647, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Starke");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 384995.147, 0.001);
    EXPECT_NEAR(resultPoint.y, 159654.089, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Steuben");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 240000.000, 0.001);
    EXPECT_NEAR(resultPoint.y, 91536.493, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Sullivan");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 447137.413, 0.001);
    EXPECT_NEAR(resultPoint.y, 383265.043, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Switzerland");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 231714.683, 0.001);
    EXPECT_NEAR(resultPoint.y, 408002.771, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Tippecanoe");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 397423.611, 0.001);
    EXPECT_NEAR(resultPoint.y, 237652.628, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Tipton");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 322854.027 , 0.001);
    EXPECT_NEAR(resultPoint.y, 269702.978, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Union");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 244142.696, 0.001);
    EXPECT_NEAR(resultPoint.y, 341391.242, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Vanderburgh");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 451280.026, 0.001);
    EXPECT_NEAR(resultPoint.y, 505491.860, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Vermillion");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 434709.379, 0.001);
    EXPECT_NEAR(resultPoint.y, 305198.679, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Vigo");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 442995.003, 0.001);
    EXPECT_NEAR(resultPoint.y, 344289.561, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Wabash");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 310425.747, 0.001);
    EXPECT_NEAR(resultPoint.y, 186285.784, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Warren");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 430566.959, 0.001);
    EXPECT_NEAR(resultPoint.y, 266225.351, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Warrick");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 430565.053, 0.001);
    EXPECT_NEAR(resultPoint.y, 499355.108, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Washington");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 335281.630, 0.001);
    EXPECT_NEAR(resultPoint.y, 486340.679, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Wayne");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 244142.720, 0.001);
    EXPECT_NEAR(resultPoint.y, 291429.823, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Wells");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 260713.402, 0.001);
    EXPECT_NEAR(resultPoint.y, 197071.604, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-White");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 397423.611, 0.001);
    EXPECT_NEAR(resultPoint.y, 237652.628, 0.001);

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS(L"InGCS-Whitley");
    EXPECT_TRUE(currentGCS->IsValid());
    EXPECT_TRUE(REPROJECT_Success == currentGCS->CartesianFromLatLong2D(resultPoint, testGeoPoint));
    EXPECT_NEAR(resultPoint.x, 281426.845, 0.001);
    EXPECT_NEAR(resultPoint.y, 186057.323, 0.001);

    }
   


