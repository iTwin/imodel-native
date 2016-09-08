//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/RealityPackage/RealityDataPackageTester.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <Bentley/BeTest.h>
#include <RealityPackage/RealityDataPackage.h>

USING_NAMESPACE_BENTLEY_REALITYPACKAGE

#define _STRINGIFY(s) #s
#define STRINGIFY(s) _STRINGIFY(s)
#define WIDEN(quote) _WIDEN(quote)
#define _WIDEN(quote) L##quote

#define LONGLAT_EPSILON 0.000000001 // precision of 0.1 millimeter

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
class PackageTestFixture : public testing::Test 
{
public:

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  4/2015
    //----------------------------------------------------------------------------------------
    BeFileName GetXsdDirectory()
        {
        BeFileName outDir;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory (outDir);
        outDir.AppendToPath(L"xsd");
        return outDir;
        }  

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  4/2015
    //----------------------------------------------------------------------------------------
    BeFileName GetXsd_2_0()
        {
        BeFileName out = GetXsdDirectory();
        out.AppendToPath(L"RealityPackage.2.0.xsd");
        return out;
        }  

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  3/2015
    //----------------------------------------------------------------------------------------
    BeFileName BuildOutputFilename(WCharCP filename)
        {
        BeFileName outputFilePath;
        BeTest::GetHost().GetOutputRoot (outputFilePath);
        outputFilePath.AppendToPath(filename);

        return outputFilePath;
        }
};

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
TEST_F (PackageTestFixture, OptionalCreationDataAndBoundingPolygon)
    {
    Utf8CP package =
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<RealityDataPackage xmlns='http://www.bentley.com/RealityDataServer/v1' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' version='1.0'>"
            "<Name>ProjectName</Name>"
        "</RealityDataPackage>";
    
    WString parseError;
    RealityPackageStatus status = RealityPackageStatus::UnknownError;
    RealityDataPackagePtr pPackage = RealityDataPackage::CreateFromString(status, package, &parseError);

    ASSERT_STREQ(L"", parseError.c_str()); // we use _STREQ to display the parse error if we have one.
    ASSERT_EQ(RealityPackageStatus::Success, status);
    ASSERT_TRUE(pPackage.IsValid());

    ASSERT_TRUE(!pPackage->GetCreationDate().IsValid());
    ASSERT_TRUE(!pPackage->GetBoundingPolygon().IsValid());
    ASSERT_EQ(0, pPackage->GetBoundingPolygon().GetPointCount());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
TEST_F (PackageTestFixture, InvalidVersion)
    {
    Utf8CP package =
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<RealityDataPackage xmlns='http://www.bentley.com/RealityDataServer/v1' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' version='999999.0'>"
            "<Name>ProjectName</Name>"
        "</RealityDataPackage>";
    
    WString parseError;
    RealityPackageStatus status = RealityPackageStatus::UnknownError;
    RealityDataPackagePtr pPackage = RealityDataPackage::CreateFromString(status, package, &parseError);

    ASSERT_STREQ(L"", parseError.c_str()); // we use _STREQ to display the parse error if we have one.
    ASSERT_EQ(RealityPackageStatus::UnsupportedVersion, status);
    ASSERT_TRUE(!pPackage.IsValid());
    }

#if (0)
//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
TEST_F (PackageTestFixture, InvalidDataSource)
    {
    Utf8CP package =
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<RealityDataPackage xmlns='http://www.bentley.com/RealityDataServer/v1' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' version='2.0'>"
            "<Name>ProjectName</Name>"
            "<ImageryGroup>"
                "<ImageryData>"
                    "<Sources>"
                        "<Source type='image/jpeg'/>"
                    "</Sources>"
                "</ImageryData>"
            "</ImageryGroup>"
        "</RealityDataPackage>";
    
    WString parseError;
    RealityPackageStatus status = RealityPackageStatus::UnknownError;
    RealityDataPackagePtr pPackage = RealityDataPackage::CreateFromString(status, package, &parseError);

    ASSERT_STREQ(L"", parseError.c_str()); // we use _STREQ to display the parse error if we have one.
    ASSERT_EQ(RealityPackageStatus::MissingSourceAttribute, status);
    ASSERT_TRUE(!pPackage.IsValid());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
TEST_F(PackageTestFixture, InvalidLongLat)
    {
    Utf8CP package =
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<RealityDataPackage xmlns='http://www.bentley.com/RealityDataServer/v1' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' version='2.0'>"
            "<Name>ProjectName</Name>"
            "<ImageryGroup>"
                "<ImageryData>"
                    "<Sources>"
                        "<Source uri='./file.ext' type='image/jpeg'/>"
                    "</Sources>"
                    "<Corners>"
                        "<LowerLeft>999.9999 55.55</LowerLeft>"
                    "</Corners>"
                "</ImageryData>"
            "</ImageryGroup>"
        "</RealityDataPackage>";
    
    WString parseError;
    RealityPackageStatus status = RealityPackageStatus::UnknownError;
    RealityDataPackagePtr pPackage = RealityDataPackage::CreateFromString(status, package, &parseError);

    ASSERT_STREQ(L"", parseError.c_str()); // we use _STREQ to display the parse error if we have one.
    ASSERT_EQ(RealityPackageStatus::InvalidLongitudeLatitude, status);
    ASSERT_TRUE(!pPackage.IsValid());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
TEST_F (PackageTestFixture, BoundingPolygonTest)
    {
    DPoint2d polygon[4] = {{1.0252, 53.04}, {41.024452, 53.0444}, {44, -55.54}, {-19.066252, -73.0666664}};
    BoundingPolygonPtr pPoly = BoundingPolygon::Create(polygon, 4);
    ASSERT_TRUE(pPoly.IsValid() && pPoly->IsValid());

    // invalid Lat.
    polygon[3].x = -10009.066252;
    //ASSERT_DEATH(BoundingPolygon::Create(polygon, 4), "");
    pPoly = BoundingPolygon::Create(polygon, 4);
    ASSERT_TRUE(!pPoly.IsValid()); 

    // valid 3 pts
    pPoly = BoundingPolygon::Create(polygon, 3);
    ASSERT_TRUE(pPoly.IsValid() && pPoly->IsValid());

    // invalid Long.
    polygon[2].y = 1111.066252;
    pPoly = BoundingPolygon::Create(polygon, 3);
    ASSERT_TRUE(!pPoly.IsValid());

    // missing points.
    pPoly = BoundingPolygon::Create(polygon, 2);
    ASSERT_TRUE(!pPoly.IsValid());
    pPoly = BoundingPolygon::Create(polygon, 1);
    ASSERT_TRUE(!pPoly.IsValid());
    pPoly = BoundingPolygon::Create(polygon, 0);
    ASSERT_TRUE(!pPoly.IsValid());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
TEST_F (PackageTestFixture, ReadVersion_1_0)
    {
    #define RPACKAGE_NAME           "name"
    #define RPACKAGE_DESCRIPTION    "description"
    #define RPACKAGE_DATE           "2015-03-25T15:17:11.588Z"
    #define RPACKAGE_COPYRIGHT      "(c) 2015 Bentley Systems"
    #define RPACKAGE_ID             "123asd789avbdlk"
    #define RPACKAGE_POLYGON        "1.0252 53.04 41.024452 53.0444 44 -55.54 -19.066252 -73.0666664 1.0252 53.04"
    #define RPACKAGE_JPEG           "./imagery/map.jpeg"
    #define RPACKAGE_JPEG_LL_x      4.987654321
    #define RPACKAGE_JPEG_LL_y      5.123456789
    #define RPACKAGE_JPEG_LR_x      22.44
    #define RPACKAGE_JPEG_LR_y      55.4554
    #define RPACKAGE_JPEG_UL_x      111.22
    #define RPACKAGE_JPEG_UL_y      15.4551234
    #define RPACKAGE_JPEG_UR_x      89.999
    #define RPACKAGE_JPEG_UR_y      5.14
    #define RPACKAGE_WMS_URI        "http://sampleserver1.arcgisonline.com/ArcGIS/services/Specialty/ESRI_StatesCitiesRivers_USA/MapServer/WMSServer?service=WMS&request=GetCapabilities&version=1.3.0"
    #define RPACKAGE_WMS_URI_AMP    "http://sampleserver1.arcgisonline.com/ArcGIS/services/Specialty/ESRI_StatesCitiesRivers_USA/MapServer/WMSServer?service=WMS&amp;request=GetCapabilities&amp;version=1.3.0"
    #define RPACKAGE_ROAD_URI       "./model/roads.shp"
    #define RPACKAGE_HOUSE_URI      "./pinned/myHouse.jpeg"
    #define RPACKAGE_HOUSE_LONG     -180
    #define RPACKAGE_HOUSE_LAT      89.123456789
    #define RPACKAGE_TRAFFIC_URI    "./pinned/roadTraffic.avi"
    #define RPACKAGE_TRAFFIC_LONG   166.987654321
    #define RPACKAGE_TRAFFIC_LAT    -90
    #define RPACKAGE_CANADA_POD_URI "./terrain/canada.pod"
    #define RPACKAGE_CANADA_DTM_URI "./terrain/canada.dtm"

    // Add unknown elements to validate our ability to support future extension of the package format.
    #define UNKNOWN_PACKAGE_ELEMENT "<NewPackageElement>data</NewPackageElement>"
    #define UNKNOWN_IMAGERY_ELEMENT "<NewImageryElement>data</NewImageryElement>"
    #define UNKNOWN_IMAGERY_SOURCE_ELEMENT "<ImageryData><NewSourceElement uri='./newSource/file.ext' type='newThing'/></ImageryData>"
    #define UNKNOWN_MODEL_ELEMENT "<NewModelElement>data</NewModelElement>"
    #define UNKNOWN_PINNED_ELEMENT "<NewPinnedElement>data</NewPinnedElement>"
    #define UNKNOWN_TERRAIN_ELEMENT "<NewTerrainElement>data</NewTerrainElement>"
    #define UNKNOWN_GROUP_ELEMENT "<NewGroupElement><NewGroupData>data</NewGroupData></NewGroupElement>"

    Utf8CP packageString =
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<RealityDataPackage xmlns='http://www.bentley.com/RealityDataServer/v1' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' version='2.0'>"
            "<Name>" RPACKAGE_NAME "</Name>"
            "<Description>" RPACKAGE_DESCRIPTION "</Description>"
            "<CreationDate>" RPACKAGE_DATE "</CreationDate>"
            "<Copyright>" RPACKAGE_COPYRIGHT "</Copyright>"
            UNKNOWN_PACKAGE_ELEMENT
            "<PackageId>" RPACKAGE_ID "</PackageId>"
            "<BoundingPolygon>" RPACKAGE_POLYGON "</BoundingPolygon>"
            "<ImageryGroup>"
                "<ImageryData>"
                    "<Sources>"
                        "<Source uri='" RPACKAGE_JPEG "' type='image/jpeg'/>"
                    "</Sources>"
                    "<Corners>"
                        "<LowerLeft>" STRINGIFY(RPACKAGE_JPEG_LL_x) " " STRINGIFY(RPACKAGE_JPEG_LL_y) "</LowerLeft>"
                        "<LowerRight>" STRINGIFY(RPACKAGE_JPEG_LR_x) " " STRINGIFY(RPACKAGE_JPEG_LR_y) "</LowerRight>"
                        "<UpperLeft>" STRINGIFY(RPACKAGE_JPEG_UL_x) " " STRINGIFY(RPACKAGE_JPEG_UL_y) "</UpperLeft>"
                        "<UpperRight>" STRINGIFY(RPACKAGE_JPEG_UR_x) " " STRINGIFY(RPACKAGE_JPEG_UR_y) "</UpperRight>"
                    "</Corners>"
                    UNKNOWN_IMAGERY_ELEMENT
                "</ImageryData>"
                "<ImageryData>"
                    "<Sources>"
                        "<WmsSource uri='" RPACKAGE_WMS_URI_AMP "' type='wms'/>"
                    "</Sources>"
                "</ImageryData>"
                UNKNOWN_IMAGERY_SOURCE_ELEMENT
            "</ImageryGroup>"
            "<ModelGroup>"
                "<ModelData>"
                    "<Sources>"
                        "<Source uri='" RPACKAGE_ROAD_URI "' type='shapefile'/>"
                    "</Sources>"
                    UNKNOWN_MODEL_ELEMENT
                "</ModelData>"
            "</ModelGroup>"
            UNKNOWN_GROUP_ELEMENT
            "<PinnedGroup>"
                "<PinnedData>"
                    "<Sources>"
                        "<Source uri='" RPACKAGE_HOUSE_URI "' type='image/jpeg'/>"
                    "</Sources>"
                    "<Position>" STRINGIFY(RPACKAGE_HOUSE_LONG) " " STRINGIFY(RPACKAGE_HOUSE_LAT) "</Position>"
                    UNKNOWN_PINNED_ELEMENT
                "</PinnedData>"
                "<PinnedData>"
                    "<Sources>"
                        "<Source uri='" RPACKAGE_TRAFFIC_URI "' type='video/avi'/>"
                    "</Sources>"
                    "<Position>" STRINGIFY(RPACKAGE_TRAFFIC_LONG) " " STRINGIFY(RPACKAGE_TRAFFIC_LAT) "</Position>"
                "</PinnedData>"
            "</PinnedGroup>"
            "<TerrainGroup>"
                "<TerrainData>"
                    "<Sources>"
                        "<Source uri='" RPACKAGE_CANADA_POD_URI "' type='pod'/>"
                    "</Sources>"
                "</TerrainData>"
                "<TerrainData>"
                    "<Sources>"
                        "<Source uri='" RPACKAGE_CANADA_DTM_URI "' type='dtm'/>"
                    "</Sources>"
                    UNKNOWN_TERRAIN_ELEMENT
                "</TerrainData>"
            "</TerrainGroup>"
            UNKNOWN_GROUP_ELEMENT
        "</RealityDataPackage>";

    // Important note ... a version 1.0 package can only have one single source per data except for Osm which has alternate encoded elsewhere.

    WString parseError;
    RealityPackageStatus status = RealityPackageStatus::UnknownError;
    RealityDataPackagePtr pPackage = RealityDataPackage::CreateFromString(status, packageString, &parseError);

    ASSERT_STREQ(L"", parseError.c_str()); // we use _STREQ to display the parse error if we have one.
    ASSERT_EQ(RealityPackageStatus::Success, status);
    ASSERT_TRUE(pPackage.IsValid());
    ASSERT_TRUE(pPackage->HasUnknownElements());

// Failed because we added unknown elements.
//     // Validate against schema
//         {
//         BeXmlStatus status = BEXML_Success;
//         BeXmlDomPtr pDom = BeXmlDom::CreateAndReadFromString (status, packageString);
// 
//         status = pDom->SchemaValidate(GetXsd_1_0().c_str());
//         if(BEXML_Success != status) // report error string first.
//             {
//             WString errorString;
//             BeXmlDom::GetLastErrorString (errorString);
//             ASSERT_STREQ(L"", errorString.c_str());
//             }
//         ASSERT_EQ(BEXML_Success, status);
//         }    

    ASSERT_EQ(1, pPackage->GetMajorVersion()); 
    ASSERT_EQ(0, pPackage->GetMinorVersion()); 

    ASSERT_STREQ(RPACKAGE_NAME, pPackage->GetName().c_str()); 
    ASSERT_STREQ(RPACKAGE_DESCRIPTION, pPackage->GetDescription().c_str()); 
    ASSERT_STREQ(WIDEN(RPACKAGE_DATE), pPackage->GetCreationDate().ToString().c_str()); 
    ASSERT_STREQ(RPACKAGE_COPYRIGHT, pPackage->GetCopyright().c_str());     
    ASSERT_STREQ(WIDEN(RPACKAGE_POLYGON), pPackage->GetBoundingPolygon().ToString().c_str());

    // Imagery
    ASSERT_TRUE(pPackage->GetImageryGroup()[0]->GetNumSources() == 1);
    ASSERT_STREQ(RPACKAGE_JPEG, pPackage->GetImageryGroup()[0]->GetSource(0).GetUri().c_str());
    ASSERT_STREQ("image/jpeg", pPackage->GetImageryGroup()[0]->GetSource(0).GetType().c_str());
    ASSERT_NEAR(pPackage->GetImageryGroup()[0]->GetCornersCP()[ImageryData::LowerLeft].x, RPACKAGE_JPEG_LL_x, LONGLAT_EPSILON);
    ASSERT_NEAR(pPackage->GetImageryGroup()[0]->GetCornersCP()[ImageryData::LowerLeft].y, RPACKAGE_JPEG_LL_y, LONGLAT_EPSILON);
    ASSERT_NEAR(pPackage->GetImageryGroup()[0]->GetCornersCP()[ImageryData::LowerRight].x, RPACKAGE_JPEG_LR_x, LONGLAT_EPSILON);
    ASSERT_NEAR(pPackage->GetImageryGroup()[0]->GetCornersCP()[ImageryData::LowerRight].y, RPACKAGE_JPEG_LR_y, LONGLAT_EPSILON);
    ASSERT_NEAR(pPackage->GetImageryGroup()[0]->GetCornersCP()[ImageryData::UpperLeft].x, RPACKAGE_JPEG_UL_x, LONGLAT_EPSILON);
    ASSERT_NEAR(pPackage->GetImageryGroup()[0]->GetCornersCP()[ImageryData::UpperLeft].y, RPACKAGE_JPEG_UL_y, LONGLAT_EPSILON);
    ASSERT_NEAR(pPackage->GetImageryGroup()[0]->GetCornersCP()[ImageryData::UpperRight].x, RPACKAGE_JPEG_UR_x, LONGLAT_EPSILON);
    ASSERT_NEAR(pPackage->GetImageryGroup()[0]->GetCornersCP()[ImageryData::UpperRight].y, RPACKAGE_JPEG_UR_y, LONGLAT_EPSILON);

    ASSERT_TRUE(pPackage->GetImageryGroup()[1]->GetNumSources() == 1);
    ASSERT_STREQ(RPACKAGE_WMS_URI, pPackage->GetImageryGroup()[1]->GetSource(0).GetUri().c_str());
    ASSERT_STREQ("wms", pPackage->GetImageryGroup()[1]->GetSource(0).GetType().c_str());

    // Model
    ASSERT_TRUE(pPackage->GetModelGroup()[0]->GetNumSources() == 1);
    ASSERT_STREQ(RPACKAGE_ROAD_URI, pPackage->GetModelGroup()[0]->GetSource(0).GetUri().c_str());
    ASSERT_STREQ("shapefile", pPackage->GetModelGroup()[0]->GetSource(0).GetType().c_str());

    // Pinned
    ASSERT_TRUE(pPackage->GetPinnedGroup()[0]->GetNumSources() == 1);
    ASSERT_STREQ(RPACKAGE_HOUSE_URI, pPackage->GetPinnedGroup()[0]->GetSource(0).GetUri().c_str());
    ASSERT_STREQ("image/jpeg", pPackage->GetPinnedGroup()[0]->GetSource(0).GetType().c_str());
    ASSERT_NEAR(pPackage->GetPinnedGroup()[0]->GetLocation().x, RPACKAGE_HOUSE_LONG, LONGLAT_EPSILON);
    ASSERT_NEAR(pPackage->GetPinnedGroup()[0]->GetLocation().y, RPACKAGE_HOUSE_LAT, LONGLAT_EPSILON);

    ASSERT_TRUE(pPackage->GetPinnedGroup()[1]->GetNumSources() == 1);
    ASSERT_STREQ(RPACKAGE_TRAFFIC_URI, pPackage->GetPinnedGroup()[1]->GetSource(0).GetUri().c_str());
    ASSERT_STREQ("video/avi", pPackage->GetPinnedGroup()[1]->GetSource(0).GetType().c_str());
    ASSERT_NEAR(pPackage->GetPinnedGroup()[1]->GetLocation().x, RPACKAGE_TRAFFIC_LONG, LONGLAT_EPSILON);
    ASSERT_NEAR(pPackage->GetPinnedGroup()[1]->GetLocation().y, RPACKAGE_TRAFFIC_LAT, LONGLAT_EPSILON);

    // Terrain
    ASSERT_TRUE(pPackage->GetTerrainGroup()[0]->GetNumSources() == 1);
    ASSERT_STREQ(RPACKAGE_CANADA_POD_URI, pPackage->GetTerrainGroup()[0]->GetSource(0).GetUri().c_str());
    ASSERT_STREQ("pod", pPackage->GetTerrainGroup()[0]->GetSource(0).GetType().c_str());

    ASSERT_TRUE(pPackage->GetTerrainGroup()[1]->GetNumSources() == 1);
    ASSERT_STREQ(RPACKAGE_CANADA_DTM_URI, pPackage->GetTerrainGroup()[1]->GetSource(0).GetUri().c_str());
    ASSERT_STREQ("dtm", pPackage->GetTerrainGroup()[1]->GetSource(0).GetType().c_str());
    }



//----------------------------------------------------------------------------------------
// @bsimethod                                                   Alain.Robert  3/2015
//----------------------------------------------------------------------------------------
TEST_F (PackageTestFixture, ReadVersion_2_0A)
    {
    #define RPACKAGE_ORIGIN2                        "dev-ual.contextservices.bentley.com"
    #define RPACKAGE_NAME2                          "7c71c780-8b52-47ba-adac-a29b407c5ac6"
    #define RPACKAGE_DESCRIPTION2                   "This is simple sample package listing typical two groups for terrain and imagery."
    #define RPACKAGE_DATE2                          "2016-02-02T16:28:01.727Z"
    #define RPACKAGE_COPYRIGHT2                     "(c) 2015 Bentley Systems"
    #define RPACKAGE_ID2                            "7c71c780-8b52-47ba-adac-a29b407c5ac6"
    #define RPACKAGE_POLYGON2                       "-71.072445 42.339894 -71.056824 42.339894 -71.056824 42.332027 -71.072445 42.332027 -71.072445 42.339894"

    #define RPACKAGE_IMDATA_A_ID                    "412"
    #define RPACKAGE_IMDATA_A_NAME                  "Canadian Digital Elevation Model Mosaic (CDEM) 014E"
    #define RPACKAGE_IMDATA_A_SRC_1_URI_PART1       "http://ftp2.cits.rncan.gc.ca/pub/cdem/014/cdem_dem_014E_tif.zip"
    #define RPACKAGE_IMDATA_A_SRC_1_URI_PART2       "cdem_dem_014.tif"
    #define RPACKAGE_IMDATA_A_SRC_1_URI_TYPE        "tif"
    #define RPACKAGE_IMDATA_A_SRC_1_ID              "6830609"
    #define RPACKAGE_IMDATA_A_SRC_1_COPYRIGHT       "http://open.canada.ca/en/open-government-licence-canada"
    #define RPACKAGE_IMDATA_A_SRC_1_PROVIDER        "GeoGratis"
    #define RPACKAGE_IMDATA_A_SRC_1_SIZE            "43827"
    #define RPACKAGE_IMDATA_A_SRC_1_METTYPE         "ISO-19115"
    #define RPACKAGE_IMDATA_A_SRC_1_METADATA        "cdem_014E_pna.xml"
    #define RPACKAGE_IMDATA_A_SRC_1_NUMSIS          0
    #define RPACKAGE_IMDATA_A_SRC_1_GEOCS           ""


    #define RPACKAGE_IMDATA_B_ID                    "46341"
    #define RPACKAGE_IMDATA_B_NAME                  "GeoBase Landsat 7 Orthoimagery S4_05230_4828_20060814"
    #define RPACKAGE_IMDATA_B_SRC_1_URI_PART1       "http://ftp2.cits.rncan.gc.ca/pub/cdem/014/cdem_dem_014E_tif.zip"
    #define RPACKAGE_IMDATA_B_SRC_1_URI_PART2       "S4_05230_4828_20060814_m20_1_utm22.tif"
    #define RPACKAGE_IMDATA_B_SRC_1_URI_TYPE        "tif"
    #define RPACKAGE_IMDATA_B_SRC_1_ID              "545"
    #define RPACKAGE_IMDATA_B_SRC_1_COPYRIGHT       "http://open.canada.ca/en/open-government-licence-canada"
    #define RPACKAGE_IMDATA_B_SRC_1_PROVIDER        "GeoGratis"
    #define RPACKAGE_IMDATA_B_SRC_1_SIZE            "437627"
    #define RPACKAGE_IMDATA_B_SRC_1_METTYPE         "ISO-19115"
    #define RPACKAGE_IMDATA_B_SRC_1_METADATA        "IMR_S4_05230_4828_20060814_1.0_pna.xml"
    #define RPACKAGE_IMDATA_B_SRC_1_NUMSIS          0
    #define RPACKAGE_IMDATA_B_SRC_1_GEOCS           ""
    #define RPACKAGE_IMDATA_B_CORNER_LL_LONG        -71.24
    #define RPACKAGE_IMDATA_B_CORNER_LL_LAT         41.28
    #define RPACKAGE_IMDATA_B_CORNER_LR_LONG        -71.239
    #define RPACKAGE_IMDATA_B_CORNER_LR_LAT         41.2801
    #define RPACKAGE_IMDATA_B_CORNER_UL_LONG        -71.24009
    #define RPACKAGE_IMDATA_B_CORNER_UL_LAT         41.281
    #define RPACKAGE_IMDATA_B_CORNER_UR_LONG        -71.23901
    #define RPACKAGE_IMDATA_B_CORNER_UR_LAT         41.28101



    #define RPACKAGE_IMDATA_B_SRC_2_URI_PART1       "http://ftp2.cits.rncan.gc.ca/pub/cdem/014/cdem_dem_014E_tif.zip"
    #define RPACKAGE_IMDATA_B_SRC_2_URI_PART2       "S4_05230_4828_20060814_m20_1_lcc00.tif"
    #define RPACKAGE_IMDATA_B_SRC_2_URI_TYPE        "tif"
    #define RPACKAGE_IMDATA_B_SRC_2_ID              "546"
    #define RPACKAGE_IMDATA_B_SRC_2_COPYRIGHT       "You are free to: Copy, modify, publish, translate, adapt, distribute or otherwise use the Information in any medium, mode or format for any lawful purpose."
    #define RPACKAGE_IMDATA_B_SRC_2_PROVIDER        "GeoGratis"
    #define RPACKAGE_IMDATA_B_SRC_2_SIZE            "437627"
    #define RPACKAGE_IMDATA_B_SRC_2_METTYPE         "ISO-19115"
    #define RPACKAGE_IMDATA_B_SRC_2_METADATA        "IMR_S4_05230_4828_20060814_1.0_pna.xml"
    #define RPACKAGE_IMDATA_B_SRC_2_NUMSIS          0
    #define RPACKAGE_IMDATA_B_SRC_2_GEOCS           "CANLCC"

    #define RPACKAGE_IMDATA_C_ID                    "41342"
    #define RPACKAGE_IMDATA_C_NAME                  "Canadian Digital Elevation Data (CDED) 001K11"
    #define RPACKAGE_IMDATA_C_SRC_1_URI_PART1       "http://ftp2.cits.rncan.gc.ca/pub/cdem/014/cdem_dem_014E_tif.zip"
    #define RPACKAGE_IMDATA_C_SRC_1_URI_PART2       "*.dem"
    #define RPACKAGE_IMDATA_C_SRC_1_URI_TYPE        "dem"
    #define RPACKAGE_IMDATA_C_SRC_1_ID              "75643"
    #define RPACKAGE_IMDATA_C_SRC_1_COPYRIGHT       "http://ouvert.canada.ca/fr/licence-du-gouvernement-ouvert-canada"
    #define RPACKAGE_IMDATA_C_SRC_1_PROVIDER        "GeoGratis"
    #define RPACKAGE_IMDATA_C_SRC_1_SIZE            "43827"
    #define RPACKAGE_IMDATA_C_SRC_1_METTYPE         "FGDC"
    #define RPACKAGE_IMDATA_C_SRC_1_METADATA        "cded_001K11_1_0_fgdc_en.xml"
    #define RPACKAGE_IMDATA_C_SRC_1_NUMSIS          0
    #define RPACKAGE_IMDATA_C_SRC_1_GEOCS           ""


    #define RPACKAGE_IMDATA_D_ID                    "341342"
    #define RPACKAGE_IMDATA_D_NAME                  "UAO2003"
    #define RPACKAGE_IMDATA_D_SRC_1_URI_PART1       "http://ftp.utah.gouv.com/imagery/ua02003/12tvk035685.tif"
    #define RPACKAGE_IMDATA_D_SRC_1_URI_PART2       ""
    #define RPACKAGE_IMDATA_D_SRC_1_URI_TYPE        "tif"
    #define RPACKAGE_IMDATA_D_SRC_1_ID              "gds:945m393090"
    #define RPACKAGE_IMDATA_D_SRC_1_COPYRIGHT       ""
    #define RPACKAGE_IMDATA_D_SRC_1_PROVIDER        ""
    #define RPACKAGE_IMDATA_D_SRC_1_SIZE            "4927"
    #define RPACKAGE_IMDATA_D_SRC_1_METTYPE         ""
    #define RPACKAGE_IMDATA_D_SRC_1_METADATA        "http://ftp.utah.gouv.com/imagery/ua02003/12tvk035685.xml"
    #define RPACKAGE_IMDATA_D_SRC_1_NUMSIS          1
    #define RPACKAGE_IMDATA_D_SRC_1_SIS1            "http://ftp.utah.gouv.com/imagery/ua02003/12tvk035685.tfw"
    #define RPACKAGE_IMDATA_D_SRC_1_GEOCS           ""
    #define RPACKAGE_IMDATA_D_CORNER_LL_LONG        -95.24
    #define RPACKAGE_IMDATA_D_CORNER_LL_LAT         30.28
    #define RPACKAGE_IMDATA_D_CORNER_LR_LONG        -95.239
    #define RPACKAGE_IMDATA_D_CORNER_LR_LAT         30.2801
    #define RPACKAGE_IMDATA_D_CORNER_UL_LONG        -95.24009
    #define RPACKAGE_IMDATA_D_CORNER_UL_LAT         30.281
    #define RPACKAGE_IMDATA_D_CORNER_UR_LONG        -95.23901
    #define RPACKAGE_IMDATA_D_CORNER_UR_LAT         30.28101

    #define RPACKAGE_IMDATA_E_ID                    "341343"
    #define RPACKAGE_IMDATA_E_NAME                  "Utah DOQ San Juan County"
    #define RPACKAGE_IMDATA_E_SRC_1_URI_PART1       "http://ftp.utah.gouv.com/imagery/UDOQ/SanJuanDOQ.zip"
    #define RPACKAGE_IMDATA_E_SRC_1_URI_PART2       ""
    #define RPACKAGE_IMDATA_E_SRC_1_URI_TYPE        ""
    #define RPACKAGE_IMDATA_E_SRC_1_ID              "gds:945m393089"
    #define RPACKAGE_IMDATA_E_SRC_1_COPYRIGHT       ""
    #define RPACKAGE_IMDATA_E_SRC_1_PROVIDER        ""
    #define RPACKAGE_IMDATA_E_SRC_1_SIZE            "681927"
    #define RPACKAGE_IMDATA_E_SRC_1_METTYPE         "text"
    #define RPACKAGE_IMDATA_E_SRC_1_METADATA        "SanJuan_DOQ.fgdc"
    #define RPACKAGE_IMDATA_E_SRC_1_NUMSIS          1
    #define RPACKAGE_IMDATA_E_SRC_1_SIS1            "http://ftp.utah.gouv.com/imagery/ua02003/12tvk035685.tfw"
    #define RPACKAGE_IMDATA_E_SRC_1_GEOCS           ""


    #define RPACKAGE_IMDATA_F_ID                    "USGS:55098640e4b02e76d757f634"
    #define RPACKAGE_IMDATA_F_NAME                  "USGS: FSA 10:1 NAIP Imagery m_4207148_nw_19_1_20140712_20140923"
    #define RPACKAGE_IMDATA_F_SRC_1_URI_PART1       "ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NAIP/ma_2014/42071/m_4207148_nw_19_1_20140712_20140923.jp2"
    #define RPACKAGE_IMDATA_F_SRC_1_URI_PART2       ""
    #define RPACKAGE_IMDATA_F_SRC_1_URI_TYPE        "jp2"
    #define RPACKAGE_IMDATA_F_SRC_1_ID              "USGS:55098640e4b02e76d757f634"
    #define RPACKAGE_IMDATA_F_SRC_1_COPYRIGHT       "FSA 10:1 NAIP Imagery m_4207148_nw_19_1_20140712_20140923 3.75 x 3.75 minute JPEG2000 from The National Map courtesy of the U.S. Geological Survey"
    #define RPACKAGE_IMDATA_F_SRC_1_PROVIDER        "usgs"
    #define RPACKAGE_IMDATA_F_SRC_1_SIZE            "25827"
    #define RPACKAGE_IMDATA_F_SRC_1_METTYPE         ""
    #define RPACKAGE_IMDATA_F_SRC_1_METADATA        "https://www.sciencebase.gov/catalog/file/get/55098640e4b02e76d757f634?f=__disk__44%2F29%2F0f%2F44290fb16573423f0c5e24542ac772670cae87f7"
    #define RPACKAGE_IMDATA_F_SRC_1_NUMSIS          1
    #define RPACKAGE_IMDATA_F_SRC_1_SIS1            "http://ftp.utah.gouv.com/imagery/ua02003/12tvk035685.tfw"
    #define RPACKAGE_IMDATA_F_SRC_1_GEOCS           "EPSG:3857"

    #define RPACKAGE_IMDATA_F_SRC_2_URI_PART1       "ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NAIP/ma_2014/42071/m_4207148_nw_19_1_20140712_20140923.cit"
    #define RPACKAGE_IMDATA_F_SRC_2_URI_PART2       ""
    #define RPACKAGE_IMDATA_F_SRC_2_URI_TYPE        "cit"
    #define RPACKAGE_IMDATA_F_SRC_2_ID              "USGS:44098640e4b02e76d747f323"
    #define RPACKAGE_IMDATA_F_SRC_2_COPYRIGHT       "FSA 10:1 NAIP Imagery m_4207148_nw_19_1_20140712_20140923 3.75 x 3.75 minute JPEG2000 from The National Map courtesy of the U.S. Geological Survey"
    #define RPACKAGE_IMDATA_F_SRC_2_PROVIDER        "usgs"
    #define RPACKAGE_IMDATA_F_SRC_2_SIZE            "34825"
    #define RPACKAGE_IMDATA_F_SRC_2_METTYPE         ""
    #define RPACKAGE_IMDATA_F_SRC_2_METADATA        "https://www.sciencebase.gov/catalog/file/get/44098640e4b02e76d747f323?f=__disk__44%2F29%2F0f%2F44290fb16573423f0c5e24542ac772670cae87f7"
    #define RPACKAGE_IMDATA_F_SRC_2_NUMSIS          0
    #define RPACKAGE_IMDATA_F_SRC_2_GEOCS           "EPSG:3857"

    #define RPACKAGE_IMDATA_F_SRC_3_URI_PART1       "ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NAIP/ma_2014/42071/m_4207148_nw_19_1_20140712_20140923.png"
    #define RPACKAGE_IMDATA_F_SRC_3_URI_PART2       ""
    #define RPACKAGE_IMDATA_F_SRC_3_URI_TYPE        "png"
    #define RPACKAGE_IMDATA_F_SRC_3_ID              "24098640e4b02e76d7475666"
    #define RPACKAGE_IMDATA_F_SRC_3_COPYRIGHT       "FSA 10:1 NAIP Imagery m_4207148_nw_19_1_20140712_20140923 3.75 x 3.75 minute JPEG2000 from The National Map courtesy of the U.S. Geological Survey"
    #define RPACKAGE_IMDATA_F_SRC_3_PROVIDER        "usgs"
    #define RPACKAGE_IMDATA_F_SRC_3_SIZE            "54721"
    #define RPACKAGE_IMDATA_F_SRC_3_METTYPE         ""
    #define RPACKAGE_IMDATA_F_SRC_3_METADATA        "https://www.sciencebase.gov/catalog/file/get/24098640e4b02e76d7475666?f=__disk__44%2F29%2F0f%2F44290fb16573423f0c5e24542ac772670cae87f7"
    #define RPACKAGE_IMDATA_F_SRC_3_GEOCS           "EPSG:3857"
    #define RPACKAGE_IMDATA_F_SRC_3_NUMSIS          1
    #define RPACKAGE_IMDATA_F_SRC_3_SIS1            "ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NAIP/ma_2014/42071/m_4207148_nw_19_1_20140712_20140923.pgw"

    #define RPACKAGE_IMDATA_G_ID                    "USGS:55098634e4b02e76d757f5c6"
    #define RPACKAGE_IMDATA_G_NAME                  "USGS: FSA 10:1 NAIP Imagery m_4207148_ne_19_1_20140712_20140923"
    #define RPACKAGE_IMDATA_G_SRC_1_URI_PART1       "ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NAIP/ma_2014/42071/m_4207148_ne_19_1_20140712_20140923.zip"
    #define RPACKAGE_IMDATA_G_SRC_1_URI_PART2       "18TVK470310_201004_0x3000m_CL_1.jp2"
    #define RPACKAGE_IMDATA_G_SRC_1_URI_TYPE        "jp2"
    #define RPACKAGE_IMDATA_G_SRC_1_ID              "USGS:55098634e4b02e76d757f5c6"
    #define RPACKAGE_IMDATA_G_SRC_1_COPYRIGHT       "FSA 10:1 NAIP Imagery m_4207148_ne_19_1_20140712_20140923 3.75 x 3.75 minute JPEG2000 from The National Map courtesy of the U.S. Geological Survey"
    #define RPACKAGE_IMDATA_G_SRC_1_PROVIDER        "usgs"
    #define RPACKAGE_IMDATA_G_SRC_1_SIZE            "25831"
    #define RPACKAGE_IMDATA_G_SRC_1_METTYPE         ""
    #define RPACKAGE_IMDATA_G_SRC_1_METADATA        "https://www.sciencebase.gov/catalog/file/get/55098634e4b02e76d757f5c6?f=__disk__94%2F3c%2F45%2F943c454b8481aa188053612d4bc4541b9f37abf0"
    #define RPACKAGE_IMDATA_G_SRC_1_GEOCS           "EPSG:3857"
    #define RPACKAGE_IMDATA_G_SRC_1_NUMSIS          2
    #define RPACKAGE_IMDATA_G_SRC_1_SIS1            "18TVK470310_201004_0x3000m_CL_1.j2w"
    #define RPACKAGE_IMDATA_G_SRC_1_SIS2            "18TVK470310_201004_0x3000m_CL_1.prj"

    #define RPACKAGE_IMDATA_MULTI_A_ID              "55098634e4b02e76d757f5c6"
    #define RPACKAGE_IMDATA_MULTI_A_NAME            "Landsat 8 multiband raw imagery LC81390452014295LGN00"
    #define RPACKAGE_IMDATA_MULTI_A_SRC             "http://landsat-pds.s3.amazonaws.com/L8/139/045/LC81390452014295LGN00/LC81390452014295LGN00_B8.TIF"
    #define RPACKAGE_IMDATA_MULTI_A_SRC_URI_TYPE    "tif"
    #define RPACKAGE_IMDATA_MULTI_A_SRC_ID          "55098634e4b02e76d757f5c6"
    #define RPACKAGE_IMDATA_MULTI_A_SRC_COPYRIGHT   "There are no restrictions on the use of data received from the U.S. Geological Survey's Earth Resources Observation and Science (EROS) Center or NASA's Land Processes Distributed Active Archive Center (LP DAAC), unless expressly identified prior to or at the time of receipt. More information on licensing and Landsat data citation is available from USGS"
    #define RPACKAGE_IMDATA_MULTI_A_SRC_PROVIDER    "Landsat8"
    #define RPACKAGE_IMDATA_MULTI_A_SRC_SIZE        "253831"
    #define RPACKAGE_IMDATA_MULTI_A_SRC_METTYPE     "text"
    #define RPACKAGE_IMDATA_MULTI_A_SRC_METADATA    "http://landsat-pds.s3.amazonaws.com/L8/139/045/LC81390452014295LGN00/LC81390452014295LGN00_MTL.txt"
    #define RPACKAGE_IMDATA_MULTI_A_SRC_GEOCS       "EPSG:4326"
    #define RPACKAGE_IMDATA_MULTI_A_SRC_NUMSIS      0
    #define RPACKAGE_IMDATA_MULTI_A_SRC_NODATA      "0"

    #define RPACKAGE_IMDATA_MULTI_A_RED_SRC             "http://landsat-pds.s3.amazonaws.com/L8/139/045/LC81390452014295LGN00/LC81390452014295LGN00_B4.TIF"
    #define RPACKAGE_IMDATA_MULTI_A_RED_SRC_URI_TYPE    "tif"
    #define RPACKAGE_IMDATA_MULTI_A_RED_SRC_ID          "55098634e4b02e76d757f5c6"
    #define RPACKAGE_IMDATA_MULTI_A_RED_SRC_COPYRIGHT   "There are no restrictions on the use of data received from the U.S. Geological Survey's Earth Resources Observation and Science (EROS) Center or NASA's Land Processes Distributed Active Archive Center (LP DAAC), unless expressly identified prior to or at the time of receipt. More information on licensing and Landsat data citation is available from USGS"
    #define RPACKAGE_IMDATA_MULTI_A_RED_SRC_PROVIDER    "Landsat8"
    #define RPACKAGE_IMDATA_MULTI_A_RED_SRC_SIZE        "93831"
    #define RPACKAGE_IMDATA_MULTI_A_RED_SRC_GEOCS       "EPSG:4326"
    #define RPACKAGE_IMDATA_MULTI_A_RED_SRC_NUMSIS      1
    #define RPACKAGE_IMDATA_MULTI_A_RED_SRC_SIS1        "http://landsat-pds.s3.amazonaws.com/L8/139/045/LC81390452014295LGN00/LC81390452014295LGN00_B4.TFW"
    #define RPACKAGE_IMDATA_MULTI_A_RED_SRC_NODATA      "0"



    #define RPACKAGE_IMDATA_MULTI_A_GREEN_SRC             "http://landsat-pds.s3.amazonaws.com/L8/139/045/LC81390452014295LGN00/LC81390452014295LGN00_B3.TIF"
    #define RPACKAGE_IMDATA_MULTI_A_GREEN_SRC_URI_TYPE    "tif"
    #define RPACKAGE_IMDATA_MULTI_A_GREEN_SRC_ID          "55098634e4b02e76d757f5c6"
    #define RPACKAGE_IMDATA_MULTI_A_GREEN_SRC_COPYRIGHT   "There are no restrictions on the use of data received from the U.S. Geological Survey's Earth Resources Observation and Science (EROS) Center or NASA's Land Processes Distributed Active Archive Center (LP DAAC), unless expressly identified prior to or at the time of receipt. More information on licensing and Landsat data citation is available from USGS"
    #define RPACKAGE_IMDATA_MULTI_A_GREEN_SRC_PROVIDER    "Landsat8"
    #define RPACKAGE_IMDATA_MULTI_A_GREEN_SRC_GEOCS       "EPSG:4326"
    #define RPACKAGE_IMDATA_MULTI_A_GREEN_SRC_NUMSIS      1
    #define RPACKAGE_IMDATA_MULTI_A_GREEN_SRC_SIS1        "http://landsat-pds.s3.amazonaws.com/L8/139/045/LC81390452014295LGN00/LC81390452014295LGN00_B3.TFW"
    #define RPACKAGE_IMDATA_MULTI_A_GREEN_SRC_NODATA      "0"


    #define RPACKAGE_IMDATA_MULTI_A_BLUE_SRC             "http://landsat-pds.s3.amazonaws.com/L8/139/045/LC81390452014295LGN00/LC81390452014295LGN00_B2.TIF"
    #define RPACKAGE_IMDATA_MULTI_A_BLUE_SRC_URI_TYPE    "tif"
    #define RPACKAGE_IMDATA_MULTI_A_BLUE_SRC_ID          "55098634e4b02e76d757f5c6"
    #define RPACKAGE_IMDATA_MULTI_A_BLUE_SRC_COPYRIGHT   "There are no restrictions on the use of data received from the U.S. Geological Survey's Earth Resources Observation and Science (EROS) Center or NASA's Land Processes Distributed Active Archive Center (LP DAAC), unless expressly identified prior to or at the time of receipt. More information on licensing and Landsat data citation is available from USGS"
    #define RPACKAGE_IMDATA_MULTI_A_BLUE_SRC_PROVIDER    "Landsat8"
    #define RPACKAGE_IMDATA_MULTI_A_BLUE_SRC_GEOCS       "EPSG:4326"
    #define RPACKAGE_IMDATA_MULTI_A_BLUE_SRC_NUMSIS      1
    #define RPACKAGE_IMDATA_MULTI_A_BLUE_SRC_SIS1        "http://landsat-pds.s3.amazonaws.com/L8/139/045/LC81390452014295LGN00/LC81390452014295LGN00_B2.TFW"
    #define RPACKAGE_IMDATA_MULTI_A_BLUE_SRC_NODATA      "0"


    #define RPACKAGE_IMDATA_MULTI_A_PANCHROMATIC_SRC             "http://landsat-pds.s3.amazonaws.com/L8/139/045/LC81390452014295LGN00/LC81390452014295LGN00_B8.TIF"
    #define RPACKAGE_IMDATA_MULTI_A_PANCHROMATIC_SRC_URI_TYPE    "tif"
    #define RPACKAGE_IMDATA_MULTI_A_PANCHROMATIC_SRC_ID          "55098634e4b02e76d757f5c6"
    #define RPACKAGE_IMDATA_MULTI_A_PANCHROMATIC_SRC_COPYRIGHT   "There are no restrictions on the use of data received from the U.S. Geological Survey's Earth Resources Observation and Science (EROS) Center or NASA's Land Processes Distributed Active Archive Center (LP DAAC), unless expressly identified prior to or at the time of receipt. More information on licensing and Landsat data citation is available from USGS"
    #define RPACKAGE_IMDATA_MULTI_A_PANCHROMATIC_SRC_PROVIDER    "Landsat8"
    #define RPACKAGE_IMDATA_MULTI_A_PANCHROMATIC_SRC_GEOCS       "EPSG:4326"
    #define RPACKAGE_IMDATA_MULTI_A_PANCHROMATIC_SRC_NUMSIS      1
    #define RPACKAGE_IMDATA_MULTI_A_PANCHROMATIC_SRC_SIS1        "http://landsat-pds.s3.amazonaws.com/L8/139/045/LC81390452014295LGN00/LC81390452014295LGN00_B8.TFW"
    #define RPACKAGE_IMDATA_MULTI_A_PANCHROMATIC_SRC_NODATA      "0"


    #define RPACKAGE_MODDATA_A_ID                   ""
    #define RPACKAGE_MODDATA_A_NAME                 ""
    #define RPACKAGE_MODDATA_A_SRC_1_URI_PART1      "./ModelData/ModelFile1.dgndb"
    #define RPACKAGE_MODDATA_A_SRC_1_URI_PART2      ""
    #define RPACKAGE_MODDATA_A_SRC_1_URI_TYPE       "dgndb"
    #define RPACKAGE_MODDATA_A_SRC_1_ID             ""
    #define RPACKAGE_MODDATA_A_SRC_1_COPYRIGHT      "CartoLand Survey inc."
    #define RPACKAGE_MODDATA_A_SRC_1_PROVIDER       ""
    #define RPACKAGE_MODDATA_A_SRC_1_SIZE           "23456"
    #define RPACKAGE_MODDATA_A_SRC_1_METTYPE        ""
    #define RPACKAGE_MODDATA_A_SRC_1_METADATA       ""
    #define RPACKAGE_MODDATA_A_SRC_1_NUMSIS         0
    #define RPACKAGE_MODDATA_A_SRC_1_GEOCS          ""



    #define RPACKAGE_MODDATA_A_SRC_2_URI_PART1      "pw://pw-utr5.bentley.com/geospatial/projectA/ModelFile1.dgndb"
    #define RPACKAGE_MODDATA_A_SRC_2_URI_PART2      ""
    #define RPACKAGE_MODDATA_A_SRC_2_URI_TYPE       "dgndb"
    #define RPACKAGE_MODDATA_A_SRC_2_ID             ""
    #define RPACKAGE_MODDATA_A_SRC_2_COPYRIGHT      "Copyright CartoLand Survey inc. All rights reserved"
    #define RPACKAGE_MODDATA_A_SRC_2_PROVIDER       ""
    #define RPACKAGE_MODDATA_A_SRC_2_SIZE           ""
    #define RPACKAGE_MODDATA_A_SRC_2_METTYPE        ""
    #define RPACKAGE_MODDATA_A_SRC_2_METADATA       ""
    #define RPACKAGE_MODDATA_A_SRC_2_NUMSIS         0
    #define RPACKAGE_MODDATA_A_SRC_2_GEOCS          ""

    #define RPACKAGE_PINDATA_A_ID                   "6455288846-3552"
    #define RPACKAGE_PINDATA_A_NAME                 "Picture of my house in 2016 during winter"
    #define RPACKAGE_PINDATA_A_LONG                 -71.2457
    #define RPACKAGE_PINDATA_A_LAT                  47.4876
    #define RPACKAGE_PINDATA_A_SRC_1_URI_PART1      "./pinned/myHouse.jpeg"
    #define RPACKAGE_PINDATA_A_SRC_1_URI_PART2      ""
    #define RPACKAGE_PINDATA_A_SRC_1_URI_TYPE       "jpeg"
    #define RPACKAGE_PINDATA_A_SRC_1_ID             ""
    #define RPACKAGE_PINDATA_A_SRC_1_COPYRIGHT      ""
    #define RPACKAGE_PINDATA_A_SRC_1_PROVIDER       ""
    #define RPACKAGE_PINDATA_A_SRC_1_SIZE           ""
    #define RPACKAGE_PINDATA_A_SRC_1_METTYPE        ""
    #define RPACKAGE_PINDATA_A_SRC_1_METADATA       ""
    #define RPACKAGE_PINDATA_A_SRC_1_NUMSIS         0
    #define RPACKAGE_PINDATA_A_SRC_1_GEOCS          ""

    #define RPACKAGE_PINDATA_A_SRC_2_URI_PART1      "pw://alpo.bentley.com/Geospatial/Project/Images/myHouse.jpg"
    #define RPACKAGE_PINDATA_A_SRC_2_URI_PART2      ""
    #define RPACKAGE_PINDATA_A_SRC_2_URI_TYPE       "jpg"
    #define RPACKAGE_PINDATA_A_SRC_2_ID             ""
    #define RPACKAGE_PINDATA_A_SRC_2_COPYRIGHT      ""
    #define RPACKAGE_PINDATA_A_SRC_2_PROVIDER       ""
    #define RPACKAGE_PINDATA_A_SRC_2_SIZE           ""
    #define RPACKAGE_PINDATA_A_SRC_2_METTYPE        ""
    #define RPACKAGE_PINDATA_A_SRC_2_METADATA       ""
    #define RPACKAGE_PINDATA_A_SRC_2_NUMSIS         0
    #define RPACKAGE_PINDATA_A_SRC_2_GEOCS          ""

    #define RPACKAGE_PINDATA_B_ID                   ""
    #define RPACKAGE_PINDATA_B_NAME                 ""
    #define RPACKAGE_PINDATA_B_LONG                 -71.23876 
    #define RPACKAGE_PINDATA_B_LAT                  47.4754
    #define RPACKAGE_PINDATA_B_SRC_1_URI_PART1      "http://www.bentley.com/userdata/pinned/roadTraffic.zip"
    #define RPACKAGE_PINDATA_B_SRC_1_URI_PART2      "roadTraffic.avi"
    #define RPACKAGE_PINDATA_B_SRC_1_URI_TYPE       "avi"
    #define RPACKAGE_PINDATA_B_SRC_1_ID             "457076095-66632-ROLE"
    #define RPACKAGE_PINDATA_B_SRC_1_COPYRIGHT      "Copyright Ville de Quebec 2014; all rights reserved."
    #define RPACKAGE_PINDATA_B_SRC_1_PROVIDER       "CanadaGouv"
    #define RPACKAGE_PINDATA_B_SRC_1_SIZE           "155977"
    #define RPACKAGE_PINDATA_B_SRC_1_METTYPE        "text"
    #define RPACKAGE_PINDATA_B_SRC_1_METADATA       "FilmingConditions.pdf"
    #define RPACKAGE_PINDATA_B_SRC_1_NUMSIS         1
    #define RPACKAGE_PINDATA_B_SRC_1_SIS1           "SpecialCodecToReadTheAvi.cod"
    #define RPACKAGE_PINDATA_B_SRC_1_GEOCS          "EPSG:4326"

    #define RPACKAGE_PINDATA_C_ID                   ""
    #define RPACKAGE_PINDATA_C_NAME                 ""
    #define RPACKAGE_PINDATA_C_LONG                 -71.23876 
    #define RPACKAGE_PINDATA_C_LAT                  47.4754
    #define RPACKAGE_PINDATA_C_SRC_1_URI_PART1      "http://www.bentley.com/evaluation_role/quebec_city/lot_746253_eval.pdf"
    #define RPACKAGE_PINDATA_C_SRC_1_URI_PART2      ""
    #define RPACKAGE_PINDATA_C_SRC_1_URI_TYPE       "pdf"
    #define RPACKAGE_PINDATA_C_SRC_1_ID             ""
    #define RPACKAGE_PINDATA_C_SRC_1_COPYRIGHT      ""
    #define RPACKAGE_PINDATA_C_SRC_1_PROVIDER       ""
    #define RPACKAGE_PINDATA_C_SRC_1_SIZE           ""
    #define RPACKAGE_PINDATA_C_SRC_1_METTYPE        ""
    #define RPACKAGE_PINDATA_C_SRC_1_METADATA       ""
    #define RPACKAGE_PINDATA_C_SRC_1_NUMSIS         0
    #define RPACKAGE_PINDATA_C_SRC_1_GEOCS          ""

    #define RPACKAGE_PINDATA_D_ID                   ""
    #define RPACKAGE_PINDATA_D_NAME                 ""

    #define RPACKAGE_PINDATA_D_LONG                 -71.2209  
    #define RPACKAGE_PINDATA_D_LAT                  46.8273
    #define RPACKAGE_PINDATA_D_LONG1                -71.3972 
    #define RPACKAGE_PINDATA_D_LAT1                 46.7362
    #define RPACKAGE_PINDATA_D_LONG2                -71.2075
    #define RPACKAGE_PINDATA_D_LAT2                 46.8126
    #define RPACKAGE_PINDATA_D_LONG3                -71.3844  
    #define RPACKAGE_PINDATA_D_LAT3                 46.9196 
    #define RPACKAGE_PINDATA_D_LONG4                -71.2833 
    #define RPACKAGE_PINDATA_D_LAT4                 46.9557 
    #define RPACKAGE_PINDATA_D_LONG5                -71.1209 
    #define RPACKAGE_PINDATA_D_LAT5                 46.8973 
    #define RPACKAGE_PINDATA_D_LONG6                -71.2011 
    #define RPACKAGE_PINDATA_D_LAT6                 46.8067
    #define RPACKAGE_PINDATA_D_LONG7                -71.2863 
    #define RPACKAGE_PINDATA_D_LAT7                 46.7452
    #define RPACKAGE_PINDATA_D_SRC_1_URI_PART1      "https://www.ville.quebec.qc.ca/"
    #define RPACKAGE_PINDATA_D_SRC_1_URI_PART2      ""
    #define RPACKAGE_PINDATA_D_SRC_1_URI_TYPE       "html"
    #define RPACKAGE_PINDATA_D_SRC_1_ID             ""
    #define RPACKAGE_PINDATA_D_SRC_1_COPYRIGHT      ""
    #define RPACKAGE_PINDATA_D_SRC_1_PROVIDER       ""
    #define RPACKAGE_PINDATA_D_SRC_1_SIZE           ""
    #define RPACKAGE_PINDATA_D_SRC_1_METTYPE        ""
    #define RPACKAGE_PINDATA_D_SRC_1_METADATA       ""
    #define RPACKAGE_PINDATA_D_SRC_1_NUMSIS         0
    #define RPACKAGE_PINDATA_D_SRC_1_GEOCS          ""

    #define RPACKAGE_TERDATA_A_ID                   "USGS:53174905e4b0cd4cd83c06a6"
    #define RPACKAGE_TERDATA_A_NAME                 "USGS NED ned19_n42x50_w071x25_ma_lftne_lot7_2011"
    #define RPACKAGE_TERDATA_A_SRC_1_URI_PART1      "ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/19/IMG/ned19_n42x50_w071x25_ma_lftne_lot7_2011.zip"
    #define RPACKAGE_TERDATA_A_SRC_1_URI_PART2      ""
    #define RPACKAGE_TERDATA_A_SRC_1_URI_TYPE       "img"
    #define RPACKAGE_TERDATA_A_SRC_1_ID             "USGS:53174905e4b0cd4cd83c06a6"
    #define RPACKAGE_TERDATA_A_SRC_1_COPYRIGHT      "USGS NED ned19_n42x50_w071x25_ma_lftne_lot7_2011 1/9 arc-second 2012 15 x 15 minute IMG courtesy of the U.S. Geological Survey"
    #define RPACKAGE_TERDATA_A_SRC_1_PROVIDER       "usgs"
    #define RPACKAGE_TERDATA_A_SRC_1_SIZE           "9930"
    #define RPACKAGE_TERDATA_A_SRC_1_METTYPE        ""
    #define RPACKAGE_TERDATA_A_SRC_1_METADATA       "https://www.sciencebase.gov/catalog/file/get/53174905e4b0cd4cd83c06a6?f=__disk__f0%2F61%2F87%2Ff06187cf3376a8bc80334aab019b14d74a4cda9f"
    #define RPACKAGE_TERDATA_A_SRC_1_NUMSIS         0
    #define RPACKAGE_TERDATA_A_SRC_1_GEOCS          ""

    #define RPACKAGE_TERDATA_B_ID                   "USGS:53174905e4b0cd4cd83c06a6"
    #define RPACKAGE_TERDATA_B_NAME                 "USGS NED n40w086 1/3 arc-second 2013 1 x 1 degree"
    #define RPACKAGE_TERDATA_B_SRC_1_URI_PART1      "ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/Elevation/13/IMG/n46w092.zip"
    #define RPACKAGE_TERDATA_B_SRC_1_URI_PART2      "*.img"
    #define RPACKAGE_TERDATA_B_SRC_1_URI_TYPE       "img"
    #define RPACKAGE_TERDATA_B_SRC_1_ID             "USGS:53174905e4b0cd4cd83cf45d"
    #define RPACKAGE_TERDATA_B_SRC_1_COPYRIGHT      "USGS NED n40w086 1/3 arc-second 2013 1 x 1 degree IMG courtesy of the U.S. Geological Survey"
    #define RPACKAGE_TERDATA_B_SRC_1_PROVIDER       "usgs"
    #define RPACKAGE_TERDATA_B_SRC_1_SIZE           "319061"
    #define RPACKAGE_TERDATA_B_SRC_1_METTYPE        ""
    #define RPACKAGE_TERDATA_B_SRC_1_METADATA       "https://www.sciencebase.gov/catalog/file/get/53174905e4b0cd4cd83cf45d?f=__disk__f0%2F61%2F87%2Ff06187cf3376a8bc80334aab019b14d74a4cda9f"
    #define RPACKAGE_TERDATA_B_SRC_1_NUMSIS         0
    #define RPACKAGE_TERDATA_B_SRC_1_GEOCS          ""

    #define RPACKAGE_TERDATA_B_SRC_2_URI_PART1      "ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/Elevation/13/ArcGrid/n46w092.zip"
    #define RPACKAGE_TERDATA_B_SRC_2_URI_PART2      ""
    #define RPACKAGE_TERDATA_B_SRC_2_URI_TYPE       "grd"
    #define RPACKAGE_TERDATA_B_SRC_2_ID             "USGS:53174905e4b0cd4cd83cf45d"
    #define RPACKAGE_TERDATA_B_SRC_2_COPYRIGHT      "USGS NED n40w086 1/3 arc-second 2013 1 x 1 degree IMG courtesy of the U.S. Geological Survey"
    #define RPACKAGE_TERDATA_B_SRC_2_PROVIDER       "usgs"
    #define RPACKAGE_TERDATA_B_SRC_2_SIZE           "338121"
    #define RPACKAGE_TERDATA_B_SRC_2_METTYPE        ""
    #define RPACKAGE_TERDATA_B_SRC_2_METADATA       "https://www.sciencebase.gov/catalog/file/get/53174905e4b0cd4cd83cf45d?f=__disk__f0%2F61%2F87%2Ff06187cf3376a8bc80334aab019b14d74a4cda9f"
    #define RPACKAGE_TERDATA_B_SRC_2_NUMSIS         0
    #define RPACKAGE_TERDATA_B_SRC_2_GEOCS          ""

    #define RPACKAGE_TERDATA_B_SRC_3_URI_PART1      "ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/Elevation/13/GridFloat/n46w092.zip"
    #define RPACKAGE_TERDATA_B_SRC_3_URI_PART2      ""
    #define RPACKAGE_TERDATA_B_SRC_3_URI_TYPE       "flt"
    #define RPACKAGE_TERDATA_B_SRC_3_ID             "USGS:53174905e4b0cd4cd83cf45d"
    #define RPACKAGE_TERDATA_B_SRC_3_COPYRIGHT      "USGS NED n40w086 1/3 arc-second 2013 1 x 1 degree IMG courtesy of the U.S. Geological Survey"
    #define RPACKAGE_TERDATA_B_SRC_3_PROVIDER       "usgs"
    #define RPACKAGE_TERDATA_B_SRC_3_SIZE           "346171"
    #define RPACKAGE_TERDATA_B_SRC_3_METTYPE        ""
    #define RPACKAGE_TERDATA_B_SRC_3_METADATA       "https://www.sciencebase.gov/catalog/file/get/53174905e4b0cd4cd83cf45d?f=__disk__f0%2F61%2F87%2Ff06187cf3376a8bc80334aab019b14d74a4cda9f"
    #define RPACKAGE_TERDATA_B_SRC_3_NUMSIS         0
    #define RPACKAGE_TERDATA_B_SRC_3_GEOCS          ""

    #define RPACKAGE_TERDATA_B_SRC_4_URI_PART1      "ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/Elevation/13/XYZ/n46w092.zip"
    #define RPACKAGE_TERDATA_B_SRC_4_URI_PART2      ""
    #define RPACKAGE_TERDATA_B_SRC_4_URI_TYPE       ""
    #define RPACKAGE_TERDATA_B_SRC_4_ID             "USGS:53174905e4b0cd4cd83cf45d"
    #define RPACKAGE_TERDATA_B_SRC_4_COPYRIGHT      "USGS NED n40w086 1/3 arc-second 2013 1 x 1 degree IMG courtesy of the U.S. Geological Survey"
    #define RPACKAGE_TERDATA_B_SRC_4_PROVIDER       "usgs"
    #define RPACKAGE_TERDATA_B_SRC_4_SIZE           "346171"
    #define RPACKAGE_TERDATA_B_SRC_4_METTYPE        ""
    #define RPACKAGE_TERDATA_B_SRC_4_METADATA       "https://www.sciencebase.gov/catalog/file/get/53174905e4b0cd4cd83cf45d?f=__disk__f0%2F61%2F87%2Ff06187cf3376a8bc80334aab019b14d74a4cda9f"
    #define RPACKAGE_TERDATA_B_SRC_4_NUMSIS         0
    #define RPACKAGE_TERDATA_B_SRC_4_GEOCS          ""

    #define RPACKAGE_JPEG2                  "./imagery/map.jpeg"
    #define RPACKAGE_JPEG_LL_x2             4.987654321
    #define RPACKAGE_JPEG_LL_y2      5.123456789
    #define RPACKAGE_JPEG_LR_x2      22.44
    #define RPACKAGE_JPEG_LR_y2      55.4554
    #define RPACKAGE_JPEG_UL_x2      111.22
    #define RPACKAGE_JPEG_UL_y2      15.4551234
    #define RPACKAGE_JPEG_UR_x2      89.999
    #define RPACKAGE_JPEG_UR_y2      5.14
    #define RPACKAGE_WMS_URI2        "http://sampleserver1.arcgisonline.com/ArcGIS/services/Specialty/ESRI_StatesCitiesRivers_USA/MapServer/WMSServer?service=WMS&request=GetCapabilities&version=1.3.0"
    #define RPACKAGE_WMS_URI_AMP2    "http://sampleserver1.arcgisonline.com/ArcGIS/services/Specialty/ESRI_StatesCitiesRivers_USA/MapServer/WMSServer?service=WMS&amp;request=GetCapabilities&amp;version=1.3.0"
    #define RPACKAGE_ROAD_URI2       "./model/roads.shp"
    #define RPACKAGE_HOUSE_URI2      "./pinned/myHouse.jpeg"
    #define RPACKAGE_HOUSE_LONG2     -180
    #define RPACKAGE_HOUSE_LAT2      89.123456789
    #define RPACKAGE_TRAFFIC_URI2    "./pinned/roadTraffic.avi"
    #define RPACKAGE_TRAFFIC_LONG2   166.987654321
    #define RPACKAGE_TRAFFIC_LAT2    -90
    #define RPACKAGE_CANADA_POD_URI2 "./terrain/canada.pod"
    #define RPACKAGE_CANADA_DTM_URI2 "./terrain/canada.dtm"

    // Add unknown elements to validate our ability to support future extension of the package format.
    #define UNKNOWN_PACKAGE_ELEMENT2 "<NewPackageElement>data</NewPackageElement>"
    #define UNKNOWN_IMAGERY_ELEMENT2 "<NewImageryElement>data</NewImageryElement>"
    #define UNKNOWN_IMAGERY_SOURCE_ELEMENT2 "<ImageryData><NewSourceElement uri='./newSource/file.ext' type='newThing'/></ImageryData>"
    #define UNKNOWN_MODEL_ELEMENT2 "<NewModelElement>data</NewModelElement>"
    #define UNKNOWN_PINNED_ELEMENT2 "<NewPinnedElement>data</NewPinnedElement>"
    #define UNKNOWN_TERRAIN_ELEMENT2 "<NewTerrainElement>data</NewTerrainElement>"
    #define UNKNOWN_GROUP_ELEMENT2 "<NewGroupElement><NewGroupData>data</NewGroupData></NewGroupElement>"


    Utf8CP packageString = "<?xml version='1.0' encoding='UTF-8'?>"
  "<RealityDataPackage xmlns='http://www.bentley.com/RealityDataServer/v2' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xsi:noNamespaceSchemaLocation='../../RealityPackage/RealityPackage.2.0.xsd' version='2.0'>"
  "<PackageOrigin>" RPACKAGE_ORIGIN2 "</PackageOrigin>"
  "<Name>" RPACKAGE_NAME2 "</Name>"
  "<Description>" RPACKAGE_DESCRIPTION2 "</Description>"
  "<CreationDate>" RPACKAGE_DATE2 "</CreationDate>"
  "<PackageId>" RPACKAGE_ID2 "</PackageId>"
  "<BoundingPolygon>" RPACKAGE_POLYGON2 "</BoundingPolygon>"
  "<ImageryGroup>"
    "<ImageryData>"
        "<!-- This ID enables to request additional information to the origin (ContextServerOrigin) about the data. The service does not garantee that "
            "the id will remain stable over a long period but immediately after reception of the package and for a few hours it will be accessible and consistent with current information. -->"
        "<Id>" RPACKAGE_IMDATA_A_ID "</Id>"
        "<Name>" RPACKAGE_IMDATA_A_NAME "</Name>"
        "<Sources>"
            "<!-- The URI below indicates the name and location of the compound document to be downloaded as well as the name of the file containing the data separated by a '#' character."
                 "Since this portion is a REGEX all '.' characters must be enclosed in brackets! -->"
            "<Source uri='" RPACKAGE_IMDATA_A_SRC_1_URI_PART1 "#" RPACKAGE_IMDATA_A_SRC_1_URI_PART2 "' type='" RPACKAGE_IMDATA_A_SRC_1_URI_TYPE "'>"
                "<!-- This ID enables to request additional information to the origin (ContextServerOrigin) about the source. The service does not garantee that "
                "the id will remain stable over a long period but immediately after reception of the package and for a few hours it will be accessible and consistent with current information. -->"
                "<Id>" RPACKAGE_IMDATA_A_SRC_1_ID "</Id>"
                "<Copyright>" RPACKAGE_IMDATA_A_SRC_1_COPYRIGHT "</Copyright>"
                "<!-- The provider provides a hint to the application for the interpretation of some data such as the internal structure of a compound document"
                    "It is optional and provides little information --> "
                "<Provider>" RPACKAGE_IMDATA_A_SRC_1_PROVIDER "</Provider>"
                "<!-- Size is always in kilo bytes. If element is absent or value is zero it can be assumed the "
                     "size of the source can only be known at dowload time or is a stream format -->"
                "<Size>" RPACKAGE_IMDATA_A_SRC_1_SIZE "</Size>"
                "<!-- The following indicates a file located within the compound. If the string had been a fully formed URI then it would have indicated"
                    "an external resource. Note the presence of a type attribute. This type can be specified if known otherwise the "
                    "application will be in charge of trying to determine the type from the file content. -->"
                "<Metadata type='" RPACKAGE_IMDATA_A_SRC_1_METTYPE "'>" RPACKAGE_IMDATA_A_SRC_1_METADATA "</Metadata>"
            "</Source>"
        "</Sources>"
    "</ImageryData>"
    "<ImageryData>"
        "<Id>" RPACKAGE_IMDATA_B_ID "</Id>"
        "<Name>" RPACKAGE_IMDATA_B_NAME "</Name>"
        "<Sources>"
            "<!-- The present of a hash sign # separator inside the uri indicates that the left part is the name of a compound document and the right part"
                 "(if present) indicates the file within compound of the source of data. The later part can designate more than one file in compound by use of wildcards (* or ?)."
                 "Note that the first part indicating the name of the file to be downloaded cannot make use of wildcards wether it be compound or not -->"
            "<Source uri='" RPACKAGE_IMDATA_B_SRC_1_URI_PART1 "#" RPACKAGE_IMDATA_B_SRC_1_URI_PART2 "' type='" RPACKAGE_IMDATA_B_SRC_1_URI_TYPE "'>"
                "<Id>" RPACKAGE_IMDATA_B_SRC_1_ID "</Id>"
                "<Copyright>" RPACKAGE_IMDATA_B_SRC_1_COPYRIGHT "</Copyright>"
                "<Provider>" RPACKAGE_IMDATA_B_SRC_1_PROVIDER "</Provider>"
                "<Size>" RPACKAGE_IMDATA_B_SRC_1_SIZE "</Size>"
                "<Metadata type='" RPACKAGE_IMDATA_B_SRC_1_METTYPE "'>" RPACKAGE_IMDATA_B_SRC_1_METADATA "</Metadata>"
            "</Source>"
            "<!-- The mirror source is identical in format and structure but makes use of a different Geographic Coordinate System (LCC instead of UTM22)-->"
            "<Source uri='" RPACKAGE_IMDATA_B_SRC_2_URI_PART1 "#" RPACKAGE_IMDATA_B_SRC_2_URI_PART2 "' type='" RPACKAGE_IMDATA_B_SRC_2_URI_TYPE "'>"
                "<Id>" RPACKAGE_IMDATA_B_SRC_2_ID "</Id>"
                 "<!-- Copyright notice can or cannot contain links. -->"
                "<Copyright>" RPACKAGE_IMDATA_B_SRC_2_COPYRIGHT "</Copyright>"
                "<Provider>" RPACKAGE_IMDATA_B_SRC_2_PROVIDER "</Provider>"
               " <Size>" RPACKAGE_IMDATA_B_SRC_2_SIZE "</Size>"
                "<Metadata>" RPACKAGE_IMDATA_B_SRC_2_METADATA "</Metadata>"
                "<!-- Notice the presence of spatial reference system (or geographic coordinate system). This value can be used as a default if the file format does not support it or if it is not set."
                "     Any code within the Bentley Geographic Coordinate System library can be used here. -->"
                "<GeoCS>" RPACKAGE_IMDATA_B_SRC_2_GEOCS "</GeoCS>"
            "</Source>"
        "</Sources>"
        "<Corners>"
            "<LowerLeft>" STRINGIFY(RPACKAGE_IMDATA_B_CORNER_LL_LONG) " " STRINGIFY(RPACKAGE_IMDATA_B_CORNER_LL_LAT) "</LowerLeft>"
            "<LowerRight>" STRINGIFY(RPACKAGE_IMDATA_B_CORNER_LR_LONG) " " STRINGIFY(RPACKAGE_IMDATA_B_CORNER_LR_LAT) "</LowerRight>"
            "<UpperLeft>" STRINGIFY(RPACKAGE_IMDATA_B_CORNER_UL_LONG) " " STRINGIFY(RPACKAGE_IMDATA_B_CORNER_UL_LAT) "</UpperLeft>"
            "<UpperRight>" STRINGIFY(RPACKAGE_IMDATA_B_CORNER_UR_LONG) " " STRINGIFY(RPACKAGE_IMDATA_B_CORNER_UR_LAT) "</UpperRight>"
        "</Corners>"
    "</ImageryData>"
      "<ImageryData>"
          "<Id>" RPACKAGE_IMDATA_C_ID "</Id>"
          "<Name>" RPACKAGE_IMDATA_C_NAME "</Name>"
          "<Sources>"
              "<!-- Compound file contains two dem files representing eastern and western portions of the source. Both files form the source. This is expressed using the regexp located after the # -->"
              "<Source uri='" RPACKAGE_IMDATA_C_SRC_1_URI_PART1 "#" RPACKAGE_IMDATA_C_SRC_1_URI_PART2 "' type='" RPACKAGE_IMDATA_C_SRC_1_URI_TYPE "'>"
                  "<Id>" RPACKAGE_IMDATA_C_SRC_1_ID "</Id>"
                  "<Copyright>" RPACKAGE_IMDATA_C_SRC_1_COPYRIGHT "</Copyright>"
                  "<Provider>" RPACKAGE_IMDATA_C_SRC_1_PROVIDER "</Provider>"
                  "<Size>" RPACKAGE_IMDATA_C_SRC_1_SIZE "</Size>"
                  "<Metadata type='" RPACKAGE_IMDATA_C_SRC_1_METTYPE "'>" RPACKAGE_IMDATA_C_SRC_1_METADATA "</Metadata>"
              "</Source>"
          "</Sources>"
      "</ImageryData>"
    "<ImageryData>"
        "<Id>" RPACKAGE_IMDATA_D_ID "</Id>"
        "<Name>" RPACKAGE_IMDATA_D_NAME "</Name>"
        "<Sources>"
            "<Source uri='" RPACKAGE_IMDATA_D_SRC_1_URI_PART1 "' type='" RPACKAGE_IMDATA_D_SRC_1_URI_TYPE "'>"
                "<Id>" RPACKAGE_IMDATA_D_SRC_1_ID "</Id>"
                "<Size>" RPACKAGE_IMDATA_D_SRC_1_SIZE "</Size>"
                "<Metadata>" RPACKAGE_IMDATA_D_SRC_1_METADATA "</Metadata>"
                "<!-- The following sister file is essential to the interpretation of the TIFF file that does not contain the georeference -->"
                "<SisterFiles>"
                    "<File>" RPACKAGE_IMDATA_D_SRC_1_SIS1 "</File>"
                "</SisterFiles>"
                "<!-- Notice that this entry does not contain the Copyright. This copyright notice is most probably included in the metadata and it is the "
                " responsibility of the application to extract and display this information to the end-user -->"
            "</Source>"
        "</Sources>"
        "<Corners>"
            "<LowerLeft>" STRINGIFY(RPACKAGE_IMDATA_D_CORNER_LL_LONG) " " STRINGIFY(RPACKAGE_IMDATA_D_CORNER_LL_LAT) "</LowerLeft>"
            "<LowerRight>" STRINGIFY(RPACKAGE_IMDATA_D_CORNER_LR_LONG) " " STRINGIFY(RPACKAGE_IMDATA_D_CORNER_LR_LAT) "</LowerRight>"
            "<UpperLeft>" STRINGIFY(RPACKAGE_IMDATA_D_CORNER_UL_LONG) " " STRINGIFY(RPACKAGE_IMDATA_D_CORNER_UL_LAT) "</UpperLeft>"
            "<UpperRight>" STRINGIFY(RPACKAGE_IMDATA_D_CORNER_UR_LONG) " " STRINGIFY(RPACKAGE_IMDATA_D_CORNER_UR_LAT) "</UpperRight>"
        "</Corners>"
    "</ImageryData>"
    "<ImageryData>"
        "<Id>" RPACKAGE_IMDATA_E_ID "</Id>"
        "<Name>" RPACKAGE_IMDATA_E_NAME "</Name>"
        "<!-- Here is an example of a source that we know to be imagery but not the format. In this case it is a SID but the packager provider could"
             "not determine for sure prior to creation. It is then up to the application to select and interpret the data regardless no type is specified. -->"
        "<Source uri='" RPACKAGE_IMDATA_E_SRC_1_URI_PART1 "'>"
            "<Id>" RPACKAGE_IMDATA_E_SRC_1_ID "</Id>"
            "<Size>" RPACKAGE_IMDATA_E_SRC_1_SIZE "</Size>"
            "<!-- Notice the type of the metadata. This TEXT indicates that the metadata is not in either FGDC or ISO-19115 format the only two other formats"
            "     supported. All other formats will be indicated as TEXT to be interpreted by a human. -->"
            "<Metadata type='" RPACKAGE_IMDATA_E_SRC_1_METTYPE "'>" RPACKAGE_IMDATA_E_SRC_1_METADATA "</Metadata>"
        "</Source>"
    "</ImageryData>"
    "<ImageryData>"
        "<Id>" RPACKAGE_IMDATA_F_ID "</Id>"
        "<!-- USGS Imagery data do not contain specification of the Geographic Coordinate System even though some format support it."
             "For this reason it is imperative that a GCS default be provided in this specific case. As an alternative it is technically"
             "possible for the application to extract the content of the ISO-19115 metadata tag  <gmd:referenceSystemInfo> if available (which is not the case here) -->"
        "<Name>" RPACKAGE_IMDATA_F_NAME "</Name>"
        "<Sources>"
            "<Source uri='" RPACKAGE_IMDATA_F_SRC_1_URI_PART1 "' type='" RPACKAGE_IMDATA_F_SRC_1_URI_TYPE "'>"
                "<Id>" RPACKAGE_IMDATA_F_SRC_1_ID "</Id>"
                "<Copyright>" RPACKAGE_IMDATA_F_SRC_1_COPYRIGHT "</Copyright>"
                "<Provider>" RPACKAGE_IMDATA_F_SRC_1_PROVIDER "</Provider>"
                "<Size>" RPACKAGE_IMDATA_F_SRC_1_SIZE "</Size>"
                "<Metadata>" RPACKAGE_IMDATA_F_SRC_1_METADATA "</Metadata>"
                "<!-- Notice the presence of spatial reference system (or geographic coordinate system). This value can be used as a default if the file format does not support it or if it is not set -->"
                "<GeoCS>" RPACKAGE_IMDATA_F_SRC_1_GEOCS "</GeoCS>"
            "</Source>"
            "<Source uri='" RPACKAGE_IMDATA_F_SRC_2_URI_PART1 "' type='" RPACKAGE_IMDATA_F_SRC_2_URI_TYPE "'>"
                "<Id>" RPACKAGE_IMDATA_F_SRC_2_ID "</Id>"
                "<Copyright>" RPACKAGE_IMDATA_F_SRC_2_COPYRIGHT "</Copyright>"
                "<Provider>" RPACKAGE_IMDATA_F_SRC_2_PROVIDER "</Provider>"
                "<Size>" RPACKAGE_IMDATA_F_SRC_2_SIZE "</Size>"
                "<Metadata>" RPACKAGE_IMDATA_F_SRC_2_METADATA "</Metadata>"
                "<GeoCS>" RPACKAGE_IMDATA_F_SRC_2_GEOCS "</GeoCS>"
            "</Source>"
            "<Source uri='" RPACKAGE_IMDATA_F_SRC_3_URI_PART1 "' type='" RPACKAGE_IMDATA_F_SRC_3_URI_TYPE "'>"
                "<Id>" RPACKAGE_IMDATA_F_SRC_3_ID "</Id>"
                "<Copyright>" RPACKAGE_IMDATA_F_SRC_3_COPYRIGHT "</Copyright>"
                "<Provider>" RPACKAGE_IMDATA_F_SRC_3_PROVIDER "</Provider>"
                "<Size>" RPACKAGE_IMDATA_F_SRC_3_SIZE "</Size>"
                "<Metadata>" RPACKAGE_IMDATA_F_SRC_3_METADATA "</Metadata>"
                "<GeoCS>" RPACKAGE_IMDATA_F_SRC_3_GEOCS "</GeoCS>"
                "<SisterFiles>"
                    "<!-- Georeference in this case is in the world file associated -->"
                    "<File>" RPACKAGE_IMDATA_F_SRC_3_SIS1 "</File>"
                "</SisterFiles>"
            "</Source>"
        "</Sources>"
    "</ImageryData>"
    "<ImageryData>"
        "<Id>" RPACKAGE_IMDATA_G_ID "</Id>"
        "<Name>" RPACKAGE_IMDATA_G_NAME "</Name>"
        "<!-- JPEG 2000 file located in a compound. The full name of the raster is provided. -->"
        "<Sources>"
            "<Source uri='" RPACKAGE_IMDATA_G_SRC_1_URI_PART1 "#" RPACKAGE_IMDATA_G_SRC_1_URI_PART2 "' type='" RPACKAGE_IMDATA_G_SRC_1_URI_TYPE "'>"
                "<Id>" RPACKAGE_IMDATA_G_SRC_1_ID "</Id>"
                "<Copyright>" RPACKAGE_IMDATA_G_SRC_1_COPYRIGHT "</Copyright>"
                "<Provider>" RPACKAGE_IMDATA_G_SRC_1_PROVIDER "</Provider>"
                "<Size>" RPACKAGE_IMDATA_G_SRC_1_SIZE "</Size>"
                "<Metadata>" RPACKAGE_IMDATA_G_SRC_1_METADATA "</Metadata>"
                "<!-- Even though the PRJ sister file specifies the georeference of the JPEG 2000 raster, the default SRS can be specified but should be ignored in this case.-->"
                "<GeoCS>" RPACKAGE_IMDATA_G_SRC_1_GEOCS "</GeoCS>"
                "<!-- Specification of the following sister files is not technically necessary. Both files are included in the compound document and need not be downloaded."
                " The presence of the entry is only useful in hinting to the fact these files 'should' be present and interpreted. -->"
                "<SisterFiles>"
                    "<File>" RPACKAGE_IMDATA_G_SRC_1_SIS1 "</File>"
                    "<File>" RPACKAGE_IMDATA_G_SRC_1_SIS2 "</File>"
                "</SisterFiles>"
            "</Source>"
        "</Sources>"
    "</ImageryData>"
    "<ImageryData>"
        "<Sources>"
            "<MultiBandSource uri='" RPACKAGE_IMDATA_MULTI_A_SRC_URI "' type='" RPACKAGE_IMDATA_MULTI_A_SRC_URI_TYPE "'>"
                "<Id>" RPACKAGE_IMDATA_MULTI_A_SRC_ID "</Id>"
                "<Copyright>" RPACKAGE_IMDATA_MULTI_A_SRC_COPYRIGHT "</Copyright>"
                "<Provider>" RPACKAGE_IMDATA_MULTI_A_SRC_PROVIDER "</Provider>"
                "<!-- Here the size will be the one of the band only but this info will not have much use -->"
                "<Size>" RPACKAGE_IMDATA_MULTI_A_SRC_SIZE "</Size>"
                "<Metadata type='" RPACKAGE_IMDATA_MULTI_A_SRC_METATYPE ">" RPACKAGE_IMDATA_MULTI_A_SRC_METADATA "</Metadata>"
                "<!-- The coordinate system is fixed -->"
                "<GeoCS>" RPACKAGE_IMDATA_MULTI_A_SRC_GEOCS "</GeoCS>"
                "<!-- The no data value is the padding color of raster file -->"
                "<NoDataValue>" RPACKAGE_IMDATA_MULTI_A_SRC_NODATA "</NoDataValue>"
                "<Red>"
                    "<Source uri='" RPACKAGE_IMDATA_MULTI_A_RED_SRC_URI "' type='" RPACKAGE_IMDATA_MULTI_A_RED_SRC_URI_TYPE "'>"
                        "<Id>" RPACKAGE_IMDATA_MULTI_A_RED_SRC_ID "</Id>"
                        "<Copyright>" RPACKAGE_IMDATA_MULTI_A_RED_SRC_COPYRIGHT "</Copyright>"
                        "<Provider>" RPACKAGE_IMDATA_MULTI_A_RED_SRC_PROVIDER "</Provider>"
                        "<!-- Here the size will be the one of the band only but this info will not have much use -->"
                        "<Size>" RPACKAGE_IMDATA_MULTI_A_RED_SRC_SIZE "</Size>"
                        "<GeoCS>" RPACKAGE_IMDATA_MULTI_A_RED_SRC_GEOCS "</GeoCS>"
                        "<!-- The no data value is the padding color of raster file -->"
                        "<NoDataValue>" RPACKAGE_IMDATA_MULTI_A_RED_SRC_NODATA "</NoDataValue>"
                        "<!-- A band may have sister files -->"
                        "<SisterFiles>"
                            "<File>" RPACKAGE_IMDATA_MULTI_A_RED_SRC_SIS1 "</File>"
                        "</SisterFiles>"
                    "</Source>"

                "</Red>"
                "<Green>"
                    "<Source uri='" RPACKAGE_IMDATA_MULTI_A_GREEN_SRC_URI "' type='" RPACKAGE_IMDATA_MULTI_A_GREEN_SRC_URI_TYPE "'>"
                        "<Id>" RPACKAGE_IMDATA_MULTI_A_GREEN_SRC_ID "</Id>"
                        "<Copyright>" RPACKAGE_IMDATA_MULTI_A_GREEN_SRC_COPYRIGHT "</Copyright>"
                        "<Provider>" RPACKAGE_IMDATA_MULTI_A_GREEN_SRC_PROVIDER "</Provider>"
                        "<GeoCS>" RPACKAGE_IMDATA_MULTI_A_GREEN_SRC_GEOCS "</GeoCS>"
                        "<!-- The no data value is the padding color of raster file -->"
                        "<NoDataValue>" RPACKAGE_IMDATA_MULTI_A_GREEN_SRC_NODATA "</NoDataValue>"
                        "<!-- A band may have sister files -->"
                        "<SisterFiles>"
                            "<File>" RPACKAGE_IMDATA_MULTI_A_GREEN_SRC_SIS1 "</File>"
                        "</SisterFiles>"
                    "</Source>"
                "</Green>"
                "<Blue>"
                    "<Source uri='" RPACKAGE_IMDATA_MULTI_A_BLUE_SRC_URI "' type='" RPACKAGE_IMDATA_MULTI_A_BLUE_SRC_URI_TYPE "'>"
                        "<Id>" RPACKAGE_IMDATA_MULTI_A_BLUE_SRC_ID "</Id>"
                        "<Copyright>" RPACKAGE_IMDATA_MULTI_A_BLUE_SRC_COPYRIGHT "</Copyright>"
                        "<Provider>" RPACKAGE_IMDATA_MULTI_A_BLUE_SRC_PROVIDER "</Provider>"
                        "<GeoCS>" RPACKAGE_IMDATA_MULTI_A_BLUE_SRC_GEOCS "</GeoCS>"
                        "<!-- The no data value is the padding color of raster file -->"
                        "<NoDataValue>" RPACKAGE_IMDATA_MULTI_A_BLUE_SRC_NODATA "</NoDataValue>"
                        "<!-- A band may have sister files -->"
                        "<SisterFiles>"
                            "<File>" RPACKAGE_IMDATA_MULTI_A_BLUE_SRC_SIS1 "</File>"
                        "</SisterFiles>"
                    "</Source>"
                "</Blue>"
                "<PanChromatic>"
                    "<Source uri='" RPACKAGE_IMDATA_MULTI_A_PANCHROMATIC_SRC_URI "' type='" RPACKAGE_IMDATA_MULTI_A_PANCHROMATIC_SRC_URI_TYPE "'>"
                        "<Id>" RPACKAGE_IMDATA_MULTI_A_PANCHROMATIC_SRC_ID "</Id>"
                        "<Copyright>" RPACKAGE_IMDATA_MULTI_A_PANCHROMATIC_SRC_COPYRIGHT "</Copyright>"
                        "<Provider>" RPACKAGE_IMDATA_MULTI_A_PANCHROMATIC_SRC_PROVIDER "</Provider>"
                        "<GeoCS>" RPACKAGE_IMDATA_MULTI_A_PANCHROMATIC_SRC_GEOCS "</GeoCS>"
                        "<!-- The no data value is the padding color of raster file -->"
                        "<NoDataValue>" RPACKAGE_IMDATA_MULTI_A_PANCHROMATIC_SRC_NODATA "</NoDataValue>"
                        "<!-- A band may have sister files -->"
                        "<SisterFiles>"
                            "<File>" RPACKAGE_IMDATA_MULTI_A_PANCHROMATIC_SRC_SIS1 "</File>"
                        "</SisterFiles>"
                    "</Source>"
                "</PanChromatic>"
            "</MultiBandSource>"
        "</Sources>"
    "</ImageryData>"
  "</ImageryGroup>"
  "<ModelGroup>"
      "<ModelData>"
            "<!-- Here is another example of multiple sources for the same data. The package can make reference to local files using either "
                 "absolute path or relative path (absolute path should not be used if possible). In the event the local file cannot be located the second source"
                 "provides the location of a copy located on a ProjectWise server. The application can use the alternate source to regenerate the "
                 "missing local file or use it directly. -->"
            "<Sources>"
                "<Source uri='" RPACKAGE_MODDATA_A_SRC_1_URI_PART1 "' type='" RPACKAGE_MODDATA_A_SRC_1_URI_TYPE "'> "
                    "<Copyright>" RPACKAGE_MODDATA_A_SRC_1_COPYRIGHT "</Copyright>"
                    "<Size>" RPACKAGE_MODDATA_A_SRC_1_SIZE "</Size>"
                "</Source>"
                "<Source uri='" RPACKAGE_MODDATA_A_SRC_2_URI_PART1 "' type='" RPACKAGE_MODDATA_A_SRC_2_URI_TYPE "'>"
                   "<Copyright>" RPACKAGE_MODDATA_A_SRC_2_COPYRIGHT "</Copyright>"
                    "<!-- Notice the absence of a Size element. If the source is of unknown size then the element is omitted. --> "
                "</Source>"
            "</Sources>"
        "</ModelData>"
    "</ModelGroup>"
    "<!-- Pinned group contain pinned data. -->"
    "<PinnedGroup>"
        "<!-- A pinned data is any spatially located data that is not spatial in nature"
             "For example the property evaluation role can be stored in a PDF document but is associated "
             "to a specific property that can be represented here by a single point part of the property. Likewise a"
             "project site can contain multiple plain picture that illustrate some infrastructure possibly taken by a phone on site."
             "The pin location is associated to the infrastructure and the document is a jpeg file. -->"
        "<PinnedData>"
            "<!-- Pin data can have id and name like any other data but is not always specified -->"
            "<Id>" RPACKAGE_PINDATA_A_ID "</Id>"
            "<Name>" RPACKAGE_PINDATA_A_NAME "</Name>"
            "<Sources>"
                "<!-- Notice here that no xml elements are associated with the image. It may be possible that the image contains"
                     "all necessary information as part of its EXIF fields -->"
                "<Source uri='" RPACKAGE_PINDATA_A_SRC_1_URI_PART1 "' type='" RPACKAGE_PINDATA_A_SRC_1_URI_TYPE "'/>"
                "<!-- Multiple sources are possible like any other type of data. All sources associated to data"
                     "should refer to the exact same content and a single source part of listed sources will be accessed normally"
                      "except if one is lost, corrupt or unavailable. If multiple images are associated to the same location then many"
                      "PinnedData entries must be specified. -->"
                "<Source uri='" RPACKAGE_PINDATA_A_SRC_2_URI_PART1 "' type='" RPACKAGE_PINDATA_A_SRC_2_URI_TYPE "'>"
           "</Sources>"
           "<!-- Exactly like the corners of image data the location is specified by space separated longitude latitde in decimal degrees."
                "These coordinate are to be interpreted as WGS84 or the equivalent in the project context. -->"
           "<Position>" STRINGIFY(RPACKAGE_PINDATA_A_LONG) " " STRINGIFY(RPACKAGE_PINDATA_A_LAT) "</Position>"
       "</PinnedData>"
       "<PinnedData>"
            "<Sources>"
                "<!-- Pinned data can be of various formats even a video clip and this clip can even be inbedded in a compound document -->"
                "<Source uri='" RPACKAGE_PINDATA_B_SRC_1_URI_PART1 "#" RPACKAGE_PINDATA_B_SRC_1_URI_PART2 "' type='" RPACKAGE_PINDATA_B_SRC_1_URI_TYPE "'>"
                    "<!-- Like any source entry additional fields can be specified -->"
                    "<Id>" RPACKAGE_PINDATA_B_SRC_1_ID "</Id>"
                    "<Copyright>" RPACKAGE_PINDATA_B_SRC_1_COPYRIGHT "</Copyright>"
                    "<Provider>" RPACKAGE_PINDATA_B_SRC_1_PROVIDER "</Provider>"
                    "<Size>" RPACKAGE_PINDATA_B_SRC_1_SIZE "</Size>"
                    "<!-- Metadata refers to a PDF file part of the compound -->"
                    "<Metadata type='" RPACKAGE_PINDATA_B_SRC_1_METTYPE "'>" RPACKAGE_PINDATA_B_SRC_1_METADATA "</Metadata>"
                    "<!-- The use of GeoCS element is meaningless in the context of pinned data since the data has no spatial nature"
                         "Note that the use of GeoCS element is allowed but will be disregarded; the field will not be used"
                         "for the interpretation of coordinates of the Position element of the data entry -->"
                    "<GeoCS>" RPACKAGE_PINDATA_B_SRC_1_GEOCS "</GeoCS>"
                    "<!-- Rarely will pinned data have sister file but it is supported -->"
                    "<SisterFiles>"
                        "<!-- Here the sister file example is a codec file containing the decompression algorithm plug-in."
                        "     Such use is far fetched as this requires complex behavior from the application. -->"
                        "<File>" RPACKAGE_PINDATA_B_SRC_1_SIS1 "</File>"
                    "</SisterFiles>"
                "</Source>"
            "</Sources>"
            "<Position>" STRINGIFY(RPACKAGE_PINDATA_B_LONG) " " STRINGIFY(RPACKAGE_PINDATA_B_LAT) "</Position>"
       "</PinnedData>"
       "<PinnedData>"
            "<Sources>"
                "<!-- Here is a text document in PDF format -->"
                "<Source uri='" RPACKAGE_PINDATA_C_SRC_1_URI_PART1 "' type='" RPACKAGE_PINDATA_C_SRC_1_URI_TYPE "'/>"
            "</Sources>"
            "<Position>" STRINGIFY(RPACKAGE_PINDATA_C_LONG) " " STRINGIFY(RPACKAGE_PINDATA_C_LAT) "</Position>"
       "</PinnedData>"
       "<PinnedData>"
            "<Sources>"
                "<!-- Pinned data can refer to a web site or be pinned to a large area by specification of a polygon instead"
                     "of a single coordinate. Note that a single polygon may be specified without holes. If a document applies to multiple disjoint"
                     "areas then multiple PinnedData entries must be present for each area. -->"
                "<Source uri='" RPACKAGE_PINDATA_D_SRC_1_URI_PART1 "' type='" RPACKAGE_PINDATA_D_SRC_1_URI_TYPE "'/>"
            "</Sources>"
            "<Position>" STRINGIFY(RPACKAGE_PINDATA_D_LONG) " " STRINGIFY(RPACKAGE_PINDATA_D_LAT) "</Position>"
            "<Area>" STRINGIFY(RPACKAGE_PINDATA_D_LONG1) " " STRINGIFY(RPACKAGE_PINDATA_D_LAT1) " "
                     STRINGIFY(RPACKAGE_PINDATA_D_LONG2) " " STRINGIFY(RPACKAGE_PINDATA_D_LAT2) " "
                     STRINGIFY(RPACKAGE_PINDATA_D_LONG3) " " STRINGIFY(RPACKAGE_PINDATA_D_LAT3) " "
                     STRINGIFY(RPACKAGE_PINDATA_D_LONG4) " " STRINGIFY(RPACKAGE_PINDATA_D_LAT4) " "
                     STRINGIFY(RPACKAGE_PINDATA_D_LONG5) " " STRINGIFY(RPACKAGE_PINDATA_D_LAT5) " "
                     STRINGIFY(RPACKAGE_PINDATA_D_LONG6) " " STRINGIFY(RPACKAGE_PINDATA_D_LAT6) " "
                     STRINGIFY(RPACKAGE_PINDATA_D_LONG7) " " STRINGIFY(RPACKAGE_PINDATA_D_LAT7) " "
                     STRINGIFY(RPACKAGE_PINDATA_D_LONG1) " " STRINGIFY(RPACKAGE_PINDATA_D_LAT1) "</Area>"
       "</PinnedData>"
    "</PinnedGroup>"
    "<TerrainGroup>"
        "<TerrainData>"
            "<Id>" RPACKAGE_TERDATA_A_ID "</Id>"
            "<Name>" RPACKAGE_TERDATA_A_NAME "</Name>"
            "<Sources>"
                "<!-- Notice in the URI the absence of indication of the file within compound containing the data. Sometimes"
                 "this information cannot be known and it is then up to the application to attempt determination of the "
                 "file or files that contain the data. In this case the source type (img) and the provider (usgs) should be sufficient"
                 "to try lodcating any *.img file contained in the compound. -->"
                "<Source uri='" RPACKAGE_TERDATA_A_SRC_1_URI_PART1 "' type='" RPACKAGE_TERDATA_A_SRC_1_URI_TYPE "'>"
                    "<Id>" RPACKAGE_TERDATA_A_SRC_1_ID "</Id>"
                    "<Copyright>" RPACKAGE_TERDATA_A_SRC_1_COPYRIGHT "</Copyright>"
                    "<Provider>" RPACKAGE_TERDATA_A_SRC_1_PROVIDER "</Provider>"
                    "<Size>" RPACKAGE_TERDATA_A_SRC_1_SIZE "</Size>"
                    "<Metadata>" RPACKAGE_TERDATA_A_SRC_1_METADATA "</Metadata>"
                "</Source>"
            "</Sources>"
        "</TerrainData>"
        "<TerrainData>"
            "<Id>" RPACKAGE_TERDATA_B_ID "</Id>"
            "<Name>" RPACKAGE_TERDATA_B_NAME "</Name>"
            "<!--  Notice here that there are many sources for the same data. Each source points to a different file stored using different format."
               "Some providers will provide many distribution format to accomodate client. Although technically the package should be fully qualified"
              " source of data including the selected format, the additional sources are provided in case the originally selected source of data"
              " is unavailable, corrupt, lost or simply the original selected format is not comprehensible by application."
              " Only one of these sources should be accessed since they sould contain equivalent data. Note that the data may not be identical between sources but should be"
              " equivalent, containing at least the same information as originally requested for the creation of the package. -->"
            "<Sources>"
                "<Source uri='" RPACKAGE_TERDATA_B_SRC_1_URI_PART1 "#" RPACKAGE_TERDATA_B_SRC_1_URI_PART2 "' type='" RPACKAGE_TERDATA_B_SRC_1_URI_TYPE "'>"
                    "<Id>" RPACKAGE_TERDATA_B_SRC_1_ID "</Id>"
                    "<Copyright>" RPACKAGE_TERDATA_B_SRC_1_COPYRIGHT "</Copyright>"
                    "<Provider>" RPACKAGE_TERDATA_B_SRC_1_PROVIDER "</Provider>"
                    "<Size>" RPACKAGE_TERDATA_B_SRC_1_SIZE "</Size>"
                    "<Metadata>" RPACKAGE_TERDATA_B_SRC_1_METADATA "</Metadata>"
                "</Source>"
                "<!-- Here the compound format should contain a grd ESRI format yet we could not determine the name of the file or the extension because"
                     "the pattern is not predictible from this provider -->"
                "<Source uri='" RPACKAGE_TERDATA_B_SRC_2_URI_PART1 "#' type='" RPACKAGE_TERDATA_B_SRC_2_URI_TYPE "'>"
                    "<Id>" RPACKAGE_TERDATA_B_SRC_2_ID "</Id>"
                    "<Copyright>" RPACKAGE_TERDATA_B_SRC_2_COPYRIGHT "</Copyright>"
                    "<Provider>" RPACKAGE_TERDATA_B_SRC_2_PROVIDER "</Provider>"
                    "<Size>" RPACKAGE_TERDATA_B_SRC_2_SIZE "</Size>"
                    "<Metadata>" RPACKAGE_TERDATA_B_SRC_2_METADATA "</Metadata>"
                "</Source>"
                "<Source uri='" RPACKAGE_TERDATA_B_SRC_3_URI_PART1"#' type='" RPACKAGE_TERDATA_B_SRC_3_URI_TYPE "'>"
                    "<Id>" RPACKAGE_TERDATA_B_SRC_3_ID "</Id>"
                    "<Copyright>" RPACKAGE_TERDATA_B_SRC_3_COPYRIGHT "</Copyright>"
                    "<Provider>" RPACKAGE_TERDATA_B_SRC_3_PROVIDER "</Provider>"
                    "<Size>" RPACKAGE_TERDATA_B_SRC_3_SIZE "</Size>"
                    "<Metadata>" RPACKAGE_TERDATA_B_SRC_3_METADATA "</Metadata>"
                "</Source>"
                "<!-- Notice here the empty type field. We know that the type is one form of XYZ but could not know if it is binary or ascii. "
                     "The empty type field gives a hint to the client application that some human intervention should be used or that"
                     "attempts at reading coherent terrain data must be performed in all possible formats. -->"
                "<Source uri='" RPACKAGE_TERDATA_B_SRC_4_URI_PART1 "#' type='" RPACKAGE_TERDATA_B_SRC_4_URI_TYPE "'>"
                    "<Id>" RPACKAGE_TERDATA_B_SRC_4_ID "</Id>"
                    "<Copyright>" RPACKAGE_TERDATA_B_SRC_4_COPYRIGHT "</Copyright>"
                    "<Provider>" RPACKAGE_TERDATA_B_SRC_4_PROVIDER "</Provider>"
                    "<Size>" RPACKAGE_TERDATA_B_SRC_4_SIZE "</Size>"
                    "<Metadata>" RPACKAGE_TERDATA_B_SRC_4_METADATA "</Metadata>"
                "</Source>"
            "</Sources>"
        "</TerrainData>"
    "</TerrainGroup>"
"</RealityDataPackage>";

    WString parseError;
    RealityPackageStatus status = RealityPackageStatus::UnknownError;
    RealityDataPackagePtr pPackage = RealityDataPackage::CreateFromString(status, packageString, &parseError);

    ASSERT_STREQ(L"", parseError.c_str()); // we use _STREQ to display the parse error if we have one.
    ASSERT_EQ(RealityPackageStatus::Success, status);
    ASSERT_TRUE(pPackage.IsValid());
    ASSERT_TRUE(pPackage->HasUnknownElements());

// Failed because we added unknown elements.
//     // Validate against schema
//         {
//         BeXmlStatus status = BEXML_Success;
//         BeXmlDomPtr pDom = BeXmlDom::CreateAndReadFromString (status, packageString);
// 
//         status = pDom->SchemaValidate(GetXsd_2_0().c_str());
//         if(BEXML_Success != status) // report error string first.
//             {
//             WString errorString;
//             BeXmlDom::GetLastErrorString (errorString);
//             ASSERT_STREQ(L"", errorString.c_str());
//             }
//         ASSERT_EQ(BEXML_Success, status);
//         }    

    ASSERT_EQ(2, pPackage->GetMajorVersion()); 
    ASSERT_EQ(0, pPackage->GetMinorVersion()); 

    ASSERT_STREQ(RPACKAGE_NAME, pPackage->GetName().c_str()); 
    ASSERT_STREQ(RPACKAGE_DESCRIPTION, pPackage->GetDescription().c_str()); 
    ASSERT_STREQ(WIDEN(RPACKAGE_DATE), pPackage->GetCreationDate().ToString().c_str()); 
    ASSERT_STREQ(RPACKAGE_COPYRIGHT, pPackage->GetCopyright().c_str());     
    ASSERT_STREQ(WIDEN(RPACKAGE_POLYGON), pPackage->GetBoundingPolygon().ToString().c_str());

    // Imagery
    ASSERT_STREQ(RPACKAGE_JPEG, pPackage->GetImageryGroup()[0]->GetSource(0).GetUri().c_str());
    ASSERT_STREQ("image/jpeg", pPackage->GetImageryGroup()[0]->GetSource(0).GetType().c_str());
    ASSERT_NEAR(pPackage->GetImageryGroup()[0]->GetCornersCP()[ImageryData::LowerLeft].x, RPACKAGE_JPEG_LL_x, LONGLAT_EPSILON);
    ASSERT_NEAR(pPackage->GetImageryGroup()[0]->GetCornersCP()[ImageryData::LowerLeft].y, RPACKAGE_JPEG_LL_y, LONGLAT_EPSILON);
    ASSERT_NEAR(pPackage->GetImageryGroup()[0]->GetCornersCP()[ImageryData::LowerRight].x, RPACKAGE_JPEG_LR_x, LONGLAT_EPSILON);
    ASSERT_NEAR(pPackage->GetImageryGroup()[0]->GetCornersCP()[ImageryData::LowerRight].y, RPACKAGE_JPEG_LR_y, LONGLAT_EPSILON);
    ASSERT_NEAR(pPackage->GetImageryGroup()[0]->GetCornersCP()[ImageryData::UpperLeft].x, RPACKAGE_JPEG_UL_x, LONGLAT_EPSILON);
    ASSERT_NEAR(pPackage->GetImageryGroup()[0]->GetCornersCP()[ImageryData::UpperLeft].y, RPACKAGE_JPEG_UL_y, LONGLAT_EPSILON);
    ASSERT_NEAR(pPackage->GetImageryGroup()[0]->GetCornersCP()[ImageryData::UpperRight].x, RPACKAGE_JPEG_UR_x, LONGLAT_EPSILON);
    ASSERT_NEAR(pPackage->GetImageryGroup()[0]->GetCornersCP()[ImageryData::UpperRight].y, RPACKAGE_JPEG_UR_y, LONGLAT_EPSILON);
    ASSERT_STREQ(RPACKAGE_WMS_URI, pPackage->GetImageryGroup()[1]->GetSource(0).GetUri().c_str());
    ASSERT_STREQ("wms", pPackage->GetImageryGroup()[1]->GetSource(0).GetType().c_str());

    // Model
    ASSERT_STREQ(RPACKAGE_ROAD_URI, pPackage->GetModelGroup()[0]->GetSource(0).GetUri().c_str());
    ASSERT_STREQ("shapefile", pPackage->GetModelGroup()[0]->GetSource(0).GetType().c_str());

    // Pinned
    ASSERT_STREQ(RPACKAGE_HOUSE_URI, pPackage->GetPinnedGroup()[0]->GetSource(0).GetUri().c_str());
    ASSERT_STREQ("image/jpeg", pPackage->GetPinnedGroup()[0]->GetSource(0).GetType().c_str());
    ASSERT_NEAR(pPackage->GetPinnedGroup()[0]->GetLocation().x, RPACKAGE_HOUSE_LONG, LONGLAT_EPSILON);
    ASSERT_NEAR(pPackage->GetPinnedGroup()[0]->GetLocation().y, RPACKAGE_HOUSE_LAT, LONGLAT_EPSILON);
    ASSERT_STREQ(RPACKAGE_TRAFFIC_URI, pPackage->GetPinnedGroup()[1]->GetSource(0).GetUri().c_str());
    ASSERT_STREQ("video/avi", pPackage->GetPinnedGroup()[1]->GetSource(0).GetType().c_str());
    ASSERT_NEAR(pPackage->GetPinnedGroup()[1]->GetLocation().x, RPACKAGE_TRAFFIC_LONG, LONGLAT_EPSILON);
    ASSERT_NEAR(pPackage->GetPinnedGroup()[1]->GetLocation().y, RPACKAGE_TRAFFIC_LAT, LONGLAT_EPSILON);

    // Terrain
    ASSERT_STREQ(RPACKAGE_CANADA_POD_URI, pPackage->GetTerrainGroup()[0]->GetSource(0).GetUri().c_str());
    ASSERT_STREQ("pod", pPackage->GetTerrainGroup()[0]->GetSource(0).GetType().c_str());
    ASSERT_STREQ(RPACKAGE_CANADA_DTM_URI, pPackage->GetTerrainGroup()[1]->GetSource(0).GetUri().c_str());
    ASSERT_STREQ("dtm", pPackage->GetTerrainGroup()[1]->GetSource(0).GetType().c_str());
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
TEST_F (PackageTestFixture, CreateAndRead)
    {
    BeFileName outfilename  = BuildOutputFilename(L"CreateAndRead.xml");
    // Create a new package
    RealityDataPackagePtr pPackage = RealityDataPackage::Create("ProjectName");
    ASSERT_TRUE(pPackage.IsValid());
    ASSERT_STREQ("ProjectName", pPackage->GetName().c_str()); 

    pPackage->SetDescription("SomeDescription");
    ASSERT_STREQ("SomeDescription", pPackage->GetDescription().c_str()); 

    pPackage->SetCopyright("(c) 2015 Bentley Systems, Incorporated. All rights reserved.");
    ASSERT_STREQ("(c) 2015 Bentley Systems, Incorporated. All rights reserved.", pPackage->GetCopyright().c_str()); 

    pPackage->SetPackageId("123asd789avbdlk");
    ASSERT_STREQ("123asd789avbdlk", pPackage->GetPackageId().c_str()); 

    DPoint2d polygon[4] = {{1.0252, 53.04}, {41.024452, 53.0444}, {44, -55.54}, {-19.066252, -73.0666664}};
    BoundingPolygonPtr pBoundingPolygon = BoundingPolygon::Create(polygon, 4);
    pPackage->SetBoundingPolygon(*pBoundingPolygon);
    ASSERT_EQ(4+1/*closure point*/, pPackage->GetBoundingPolygon().GetPointCount()); 
    ASSERT_DOUBLE_EQ(polygon[2].x, pPackage->GetBoundingPolygon().GetPointCP()[2].x);
    ASSERT_DOUBLE_EQ(polygon[3].y, pPackage->GetBoundingPolygon().GetPointCP()[3].y);

    // **** Data sources
    RealityDataSourcePtr pJpegDataSource = RealityDataSource::Create("./imagery/map.jpeg", "image/jpeg");
    ASSERT_TRUE(pJpegDataSource.IsValid());

    RealityDataSourcePtr pPodDataSource = RealityDataSource::Create("./terrain/canada.pod", "pod");
    ASSERT_TRUE(pPodDataSource.IsValid());

    RealityDataSourcePtr pTifDataSource = RealityDataSource::Create("./terrain/canada.tif", "image/tif");
    ASSERT_TRUE(pTifDataSource.IsValid());

    RealityDataSourcePtr pDtmDataSource = RealityDataSource::Create("./terrain/canada.dtm", "dtm");
    ASSERT_TRUE(pDtmDataSource.IsValid());

    RealityDataSourcePtr pShapeDataSource = RealityDataSource::Create("./model/roads.shp", "shapefile");
    ASSERT_TRUE(pShapeDataSource.IsValid());

    RealityDataSourcePtr pWmsDataDataSource = WmsDataSource::Create("http://sampleserver1.arcgisonline.com/ArcGIS/services/Specialty/ESRI_StatesCitiesRivers_USA/MapServer/WMSServer?service=WMS&request=GetCapabilities&version=1.3.0");
    ASSERT_TRUE(pWmsDataDataSource.IsValid());

    RealityDataSourcePtr pAviDataDataSource = RealityDataSource::Create("./pinned/roadTraffic.avi", "video/avi");
    ASSERT_TRUE(pAviDataDataSource.IsValid());

    RealityDataSourcePtr pMyHouseDataDataSource = RealityDataSource::Create("./pinned/myHouse.jpeg", "image/jpeg");
    ASSERT_TRUE(pMyHouseDataDataSource.IsValid());

    // *** Reality entries
    DPoint2d jpegCorners[4] = {{5.123456789, 4.987654321}, {55.4554, 22.44}, {15.4551234, 111.22}, {5.14, 89.999}};
    ImageryDataPtr pJpegImagery = ImageryData::Create(*pJpegDataSource, jpegCorners);
    ASSERT_TRUE(pJpegImagery.IsValid());
    pPackage->GetImageryGroupR().push_back(pJpegImagery);
    ImageryDataPtr pWmsImagery = ImageryData::Create(*pWmsDataDataSource, NULL);
    ASSERT_TRUE(pWmsImagery.IsValid());
    pPackage->GetImageryGroupR().push_back(pWmsImagery);
    
    ModelDataPtr pShapeModel = ModelData::Create(*pShapeDataSource);
    ASSERT_TRUE(pShapeModel.IsValid());
    pPackage->GetModelGroupR().push_back(pShapeModel);

    PinnedDataPtr pPinnedJpeg = PinnedData::Create(*pMyHouseDataDataSource, -180, 89.123456789);
    ASSERT_TRUE(pPinnedJpeg.IsValid());
    pPackage->GetPinnedGroupR().push_back(pPinnedJpeg);
    pPackage->GetPinnedGroupR().push_back(PinnedData::Create(*pAviDataDataSource, 166.987654321, -90));

    TerrainDataPtr pPodTerrain = TerrainData::Create(*pPodDataSource);
    ASSERT_TRUE(pPodTerrain.IsValid());
    pPackage->GetTerrainGroupR().push_back(pPodTerrain);
    pPackage->GetTerrainGroupR().push_back(TerrainData::Create(*pDtmDataSource));
    pPackage->GetTerrainGroupR().push_back(TerrainData::Create(*pTifDataSource));
       
    
    ASSERT_EQ(RealityPackageStatus::Success, pPackage->Write(outfilename));
    ASSERT_TRUE(!pPackage->HasUnknownElements());
    
    // Validate against schema
        {
        BeXmlStatus status = BEXML_Success;
        BeXmlDomPtr pDom = BeXmlDom::CreateAndReadFromFile (status, outfilename.c_str(), NULL);
        /*
        status = pDom->SchemaValidate(GetXsd_1_0().c_str());
        if(BEXML_Success != status) // report error string first.
            {
            WString errorString;
            BeXmlDom::GetLastErrorString (errorString);
            ASSERT_STREQ(L"", errorString.c_str());
            }
        */
        ASSERT_EQ(BEXML_Success, status);
        } 

    // Validate what we have created.
    WString parseError;
    RealityPackageStatus readStatus = RealityPackageStatus::UnknownError;
    RealityDataPackagePtr pReadPackage = RealityDataPackage::CreateFromFile(readStatus, outfilename, &parseError);
    ASSERT_EQ(RealityPackageStatus::Success, readStatus);
    ASSERT_TRUE(pReadPackage.IsValid());
    ASSERT_STREQ(L"", parseError.c_str()); // we use _STREQ to display the parse error if we have one.
    ASSERT_TRUE(!pReadPackage->HasUnknownElements());
    
    ASSERT_STREQ(pPackage->GetName().c_str(), pReadPackage->GetName().c_str());
    ASSERT_STREQ(pPackage->GetDescription().c_str(), pReadPackage->GetDescription().c_str());
    ASSERT_STREQ(pPackage->GetCopyright().c_str(), pReadPackage->GetCopyright().c_str());
    ASSERT_STREQ(pPackage->GetPackageId().c_str(), pReadPackage->GetPackageId().c_str());
    ASSERT_TRUE(pPackage->GetCreationDate().Equals(pReadPackage->GetCreationDate()));

    ASSERT_EQ(pPackage->GetBoundingPolygon().GetPointCount(), pReadPackage->GetBoundingPolygon().GetPointCount());
    for(size_t index=0; index < pPackage->GetBoundingPolygon().GetPointCount(); ++index)
        {
        ASSERT_NEAR(pPackage->GetBoundingPolygon().GetPointCP()[index].x, pReadPackage->GetBoundingPolygon().GetPointCP()[index].x, LONGLAT_EPSILON);
        ASSERT_NEAR(pPackage->GetBoundingPolygon().GetPointCP()[index].y, pReadPackage->GetBoundingPolygon().GetPointCP()[index].y, LONGLAT_EPSILON);
        }

    // *** Imagery Data
    ASSERT_EQ(pPackage->GetImageryGroup().size(), pReadPackage->GetImageryGroupR().size());
    for(size_t index=0; index < pPackage->GetImageryGroup().size(); ++index)
        {
        ImageryDataPtr pLeft = pPackage->GetImageryGroup()[index];
        ImageryDataPtr pRight = pReadPackage->GetImageryGroup()[index];

        ASSERT_STREQ(pLeft->GetSource(0).GetUri().c_str(), pRight->GetSource(0).GetUri().c_str());
        ASSERT_STREQ(pLeft->GetSource(0).GetType().c_str(), pRight->GetSource(0).GetType().c_str());
        
        ASSERT_EQ(0 != (pLeft->GetCornersCP()), 0 != (pRight->GetCornersCP()));
        if(pLeft->GetCornersCP())
            {
            ASSERT_NEAR(pLeft->GetCornersCP()[ImageryData::LowerLeft].x, pRight->GetCornersCP()[ImageryData::LowerLeft].x, LONGLAT_EPSILON);
            ASSERT_NEAR(pLeft->GetCornersCP()[ImageryData::LowerLeft].y, pRight->GetCornersCP()[ImageryData::LowerLeft].y, LONGLAT_EPSILON);
            ASSERT_NEAR(pLeft->GetCornersCP()[ImageryData::LowerRight].x, pRight->GetCornersCP()[ImageryData::LowerRight].x, LONGLAT_EPSILON);
            ASSERT_NEAR(pLeft->GetCornersCP()[ImageryData::LowerRight].y, pRight->GetCornersCP()[ImageryData::LowerRight].y, LONGLAT_EPSILON);
            ASSERT_NEAR(pLeft->GetCornersCP()[ImageryData::UpperLeft].x, pRight->GetCornersCP()[ImageryData::UpperLeft].x, LONGLAT_EPSILON);
            ASSERT_NEAR(pLeft->GetCornersCP()[ImageryData::UpperLeft].y, pRight->GetCornersCP()[ImageryData::UpperLeft].y, LONGLAT_EPSILON);
            ASSERT_NEAR(pLeft->GetCornersCP()[ImageryData::UpperRight].x, pRight->GetCornersCP()[ImageryData::UpperRight].x, LONGLAT_EPSILON);
            ASSERT_NEAR(pLeft->GetCornersCP()[ImageryData::UpperRight].y, pRight->GetCornersCP()[ImageryData::UpperRight].y, LONGLAT_EPSILON);
            }
        }
    
    // *** Model data
    ASSERT_EQ(pPackage->GetModelGroup().size(), pReadPackage->GetModelGroup().size());
    for(size_t index=0; index < pPackage->GetModelGroup().size(); ++index)
        {
        ModelDataPtr pLeft = pPackage->GetModelGroup()[index];
        ModelDataPtr pRight = pReadPackage->GetModelGroup()[index];

        ASSERT_STREQ(pLeft->GetSource(0).GetUri().c_str(), pRight->GetSource(0).GetUri().c_str());
        ASSERT_STREQ(pLeft->GetSource(0).GetType().c_str(), pRight->GetSource(0).GetType().c_str());
        }

    // *** Pinned data
    ASSERT_EQ(pPackage->GetPinnedGroup().size(), pReadPackage->GetPinnedGroup().size());
    for(size_t index=0; index < pPackage->GetPinnedGroup().size(); ++index)
        {
        PinnedDataPtr pLeft = pPackage->GetPinnedGroup()[index];
        PinnedDataPtr pRight = pReadPackage->GetPinnedGroup()[index];

        ASSERT_STREQ(pLeft->GetSource(0).GetUri().c_str(), pRight->GetSource(0).GetUri().c_str());
        ASSERT_STREQ(pLeft->GetSource(0).GetType().c_str(), pRight->GetSource(0).GetType().c_str());
        ASSERT_NEAR(pLeft->GetLocation().x, pRight->GetLocation().x, LONGLAT_EPSILON);
        ASSERT_NEAR(pLeft->GetLocation().y, pRight->GetLocation().y, LONGLAT_EPSILON);
        }

    // *** Terrain data.
    ASSERT_EQ(pPackage->GetTerrainGroup().size(), pReadPackage->GetTerrainGroup().size());
    for(size_t index=0; index < pPackage->GetTerrainGroup().size(); ++index)
        {
        TerrainDataPtr pLeft = pPackage->GetTerrainGroup()[index];
        TerrainDataPtr pRight = pReadPackage->GetTerrainGroup()[index];

        ASSERT_STREQ(pLeft->GetSource(0).GetUri().c_str(), pRight->GetSource(0).GetUri().c_str());
        ASSERT_STREQ(pLeft->GetSource(0).GetType().c_str(), pRight->GetSource(0).GetType().c_str());
        }
    }
#endif
