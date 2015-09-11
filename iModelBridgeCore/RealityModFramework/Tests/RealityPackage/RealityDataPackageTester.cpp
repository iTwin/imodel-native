//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/RealityPackage/RealityDataPackageTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    BeFileName GetXsd_1_0()
        {
        BeFileName out = GetXsdDirectory();
        out.AppendToPath(L"RealityPackage.1.0.xsd");
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

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
TEST_F (PackageTestFixture, InvalidDataSource)
    {
    Utf8CP package =
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<RealityDataPackage xmlns='http://www.bentley.com/RealityDataServer/v1' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' version='1.0'>"
            "<Name>ProjectName</Name>"
            "<ImageryGroup>"
                "<ImageryData>"
                    "<Source type='image/jpeg'/>"
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
        "<RealityDataPackage xmlns='http://www.bentley.com/RealityDataServer/v1' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' version='1.0'>"
            "<Name>ProjectName</Name>"
            "<ImageryGroup>"
                "<ImageryData>"
                    "<Source uri='./file.ext' type='image/jpeg'/>"
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
        "<RealityDataPackage xmlns='http://www.bentley.com/RealityDataServer/v1' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' version='1.0'>"
            "<Name>" RPACKAGE_NAME "</Name>"
            "<Description>" RPACKAGE_DESCRIPTION "</Description>"
            "<CreationDate>" RPACKAGE_DATE "</CreationDate>"
            "<Copyright>" RPACKAGE_COPYRIGHT "</Copyright>"
            UNKNOWN_PACKAGE_ELEMENT
            "<PackageId>" RPACKAGE_ID "</PackageId>"
            "<BoundingPolygon>" RPACKAGE_POLYGON "</BoundingPolygon>"
            "<ImageryGroup>"
                "<ImageryData>"
                    "<Source uri='" RPACKAGE_JPEG "' type='image/jpeg'/>"
                    "<Corners>"
                        "<LowerLeft>" STRINGIFY(RPACKAGE_JPEG_LL_x) " " STRINGIFY(RPACKAGE_JPEG_LL_y) "</LowerLeft>"
                        "<LowerRight>" STRINGIFY(RPACKAGE_JPEG_LR_x) " " STRINGIFY(RPACKAGE_JPEG_LR_y) "</LowerRight>"
                        "<UpperLeft>" STRINGIFY(RPACKAGE_JPEG_UL_x) " " STRINGIFY(RPACKAGE_JPEG_UL_y) "</UpperLeft>"
                        "<UpperRight>" STRINGIFY(RPACKAGE_JPEG_UR_x) " " STRINGIFY(RPACKAGE_JPEG_UR_y) "</UpperRight>"
                    "</Corners>"
                    UNKNOWN_IMAGERY_ELEMENT
                "</ImageryData>"
                "<ImageryData>"
                    "<WmsSource uri='" RPACKAGE_WMS_URI_AMP "' type='wms'/>"
                "</ImageryData>"
                UNKNOWN_IMAGERY_SOURCE_ELEMENT
            "</ImageryGroup>"
            "<ModelGroup>"
                "<ModelData>"
                    "<Source uri='" RPACKAGE_ROAD_URI "' type='shapefile'/>"
                    UNKNOWN_MODEL_ELEMENT
                "</ModelData>"
            "</ModelGroup>"
            UNKNOWN_GROUP_ELEMENT
            "<PinnedGroup>"
                "<PinnedData>"
                    "<Source uri='" RPACKAGE_HOUSE_URI "' type='image/jpeg'/>"
                    "<Position>" STRINGIFY(RPACKAGE_HOUSE_LONG) " " STRINGIFY(RPACKAGE_HOUSE_LAT) "</Position>"
                    UNKNOWN_PINNED_ELEMENT
                "</PinnedData>"
                "<PinnedData>"
                    "<Source uri='" RPACKAGE_TRAFFIC_URI "' type='video/avi'/>"
                    "<Position>" STRINGIFY(RPACKAGE_TRAFFIC_LONG) " " STRINGIFY(RPACKAGE_TRAFFIC_LAT) "</Position>"
                "</PinnedData>"
            "</PinnedGroup>"
            "<TerrainGroup>"
                "<TerrainData>"
                    "<Source uri='" RPACKAGE_CANADA_POD_URI "' type='pod'/>"
                "</TerrainData>"
                "<TerrainData>"
                    "<Source uri='" RPACKAGE_CANADA_DTM_URI "' type='dtm'/>"
                    UNKNOWN_TERRAIN_ELEMENT
                "</TerrainData>"
            "</TerrainGroup>"
            UNKNOWN_GROUP_ELEMENT
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

    ASSERT_STREQ(WIDEN(RPACKAGE_NAME), pPackage->GetName().c_str()); 
    ASSERT_STREQ(WIDEN(RPACKAGE_DESCRIPTION), pPackage->GetDescription().c_str()); 
    ASSERT_STREQ(WIDEN(RPACKAGE_DATE), pPackage->GetCreationDate().ToString().c_str()); 
    ASSERT_STREQ(WIDEN(RPACKAGE_COPYRIGHT), pPackage->GetCopyright().c_str());     
    ASSERT_STREQ(WIDEN(RPACKAGE_POLYGON), pPackage->GetBoundingPolygon().ToString().c_str());

    // Imagery
    ASSERT_STREQ(RPACKAGE_JPEG, pPackage->GetImageryGroup()[0]->GetSource().GetUri().c_str());
    ASSERT_STREQ(L"image/jpeg", pPackage->GetImageryGroup()[0]->GetSource().GetType().c_str());
    ASSERT_NEAR(pPackage->GetImageryGroup()[0]->GetCornersCP()[ImageryData::LowerLeft].x, RPACKAGE_JPEG_LL_x, LONGLAT_EPSILON);
    ASSERT_NEAR(pPackage->GetImageryGroup()[0]->GetCornersCP()[ImageryData::LowerLeft].y, RPACKAGE_JPEG_LL_y, LONGLAT_EPSILON);
    ASSERT_NEAR(pPackage->GetImageryGroup()[0]->GetCornersCP()[ImageryData::LowerRight].x, RPACKAGE_JPEG_LR_x, LONGLAT_EPSILON);
    ASSERT_NEAR(pPackage->GetImageryGroup()[0]->GetCornersCP()[ImageryData::LowerRight].y, RPACKAGE_JPEG_LR_y, LONGLAT_EPSILON);
    ASSERT_NEAR(pPackage->GetImageryGroup()[0]->GetCornersCP()[ImageryData::UpperLeft].x, RPACKAGE_JPEG_UL_x, LONGLAT_EPSILON);
    ASSERT_NEAR(pPackage->GetImageryGroup()[0]->GetCornersCP()[ImageryData::UpperLeft].y, RPACKAGE_JPEG_UL_y, LONGLAT_EPSILON);
    ASSERT_NEAR(pPackage->GetImageryGroup()[0]->GetCornersCP()[ImageryData::UpperRight].x, RPACKAGE_JPEG_UR_x, LONGLAT_EPSILON);
    ASSERT_NEAR(pPackage->GetImageryGroup()[0]->GetCornersCP()[ImageryData::UpperRight].y, RPACKAGE_JPEG_UR_y, LONGLAT_EPSILON);
    ASSERT_STREQ(RPACKAGE_WMS_URI, pPackage->GetImageryGroup()[1]->GetSource().GetUri().c_str());
    ASSERT_STREQ(L"wms", pPackage->GetImageryGroup()[1]->GetSource().GetType().c_str());

    // Model
    ASSERT_STREQ(RPACKAGE_ROAD_URI, pPackage->GetModelGroup()[0]->GetSource().GetUri().c_str());
    ASSERT_STREQ(L"shapefile", pPackage->GetModelGroup()[0]->GetSource().GetType().c_str());

    // Pinned
    ASSERT_STREQ(RPACKAGE_HOUSE_URI, pPackage->GetPinnedGroup()[0]->GetSource().GetUri().c_str());
    ASSERT_STREQ(L"image/jpeg", pPackage->GetPinnedGroup()[0]->GetSource().GetType().c_str());
    ASSERT_NEAR(pPackage->GetPinnedGroup()[0]->GetLocation().x, RPACKAGE_HOUSE_LONG, LONGLAT_EPSILON);
    ASSERT_NEAR(pPackage->GetPinnedGroup()[0]->GetLocation().y, RPACKAGE_HOUSE_LAT, LONGLAT_EPSILON);
    ASSERT_STREQ(RPACKAGE_TRAFFIC_URI, pPackage->GetPinnedGroup()[1]->GetSource().GetUri().c_str());
    ASSERT_STREQ(L"video/avi", pPackage->GetPinnedGroup()[1]->GetSource().GetType().c_str());
    ASSERT_NEAR(pPackage->GetPinnedGroup()[1]->GetLocation().x, RPACKAGE_TRAFFIC_LONG, LONGLAT_EPSILON);
    ASSERT_NEAR(pPackage->GetPinnedGroup()[1]->GetLocation().y, RPACKAGE_TRAFFIC_LAT, LONGLAT_EPSILON);

    // Terrain
    ASSERT_STREQ(RPACKAGE_CANADA_POD_URI, pPackage->GetTerrainGroup()[0]->GetSource().GetUri().c_str());
    ASSERT_STREQ(L"pod", pPackage->GetTerrainGroup()[0]->GetSource().GetType().c_str());
    ASSERT_STREQ(RPACKAGE_CANADA_DTM_URI, pPackage->GetTerrainGroup()[1]->GetSource().GetUri().c_str());
    ASSERT_STREQ(L"dtm", pPackage->GetTerrainGroup()[1]->GetSource().GetType().c_str());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
TEST_F (PackageTestFixture, CreateAndRead)
    {
    BeFileName outfilename  = BuildOutputFilename(L"CreateAndRead.xml");
    // Create a new package
    RealityDataPackagePtr pPackage = RealityDataPackage::Create(L"ProjectName");
    ASSERT_TRUE(pPackage.IsValid());
    ASSERT_STREQ(L"ProjectName", pPackage->GetName().c_str()); 

    pPackage->SetDescription(L"SomeDescription");
    ASSERT_STREQ(L"SomeDescription", pPackage->GetDescription().c_str()); 

    pPackage->SetCopyright(L"(c) 2015 Bentley Systems, Incorporated. All rights reserved.");
    ASSERT_STREQ(L"(c) 2015 Bentley Systems, Incorporated. All rights reserved.", pPackage->GetCopyright().c_str()); 

    pPackage->SetPackageId(L"123asd789avbdlk");
    ASSERT_STREQ(L"123asd789avbdlk", pPackage->GetPackageId().c_str()); 

    DPoint2d polygon[4] = {{1.0252, 53.04}, {41.024452, 53.0444}, {44, -55.54}, {-19.066252, -73.0666664}};
    BoundingPolygonPtr pBoundingPolygon = BoundingPolygon::Create(polygon, 4);
    pPackage->SetBoundingPolygon(*pBoundingPolygon);
    ASSERT_EQ(4+1/*closure point*/, pPackage->GetBoundingPolygon().GetPointCount()); 
    ASSERT_DOUBLE_EQ(polygon[2].x, pPackage->GetBoundingPolygon().GetPointCP()[2].x);
    ASSERT_DOUBLE_EQ(polygon[3].y, pPackage->GetBoundingPolygon().GetPointCP()[3].y);

    // **** Data sources
    RealityDataSourcePtr pJpegDataSource = RealityDataSource::Create("./imagery/map.jpeg", L"image/jpeg");
    ASSERT_TRUE(pJpegDataSource.IsValid());

    RealityDataSourcePtr pPodDataSource = RealityDataSource::Create("./terrain/canada.pod", L"pod");
    ASSERT_TRUE(pPodDataSource.IsValid());

    RealityDataSourcePtr pTifDataSource = RealityDataSource::Create("./terrain/canada.tif", L"image/tif");
    ASSERT_TRUE(pTifDataSource.IsValid());

    RealityDataSourcePtr pDtmDataSource = RealityDataSource::Create("./terrain/canada.dtm", L"dtm");
    ASSERT_TRUE(pDtmDataSource.IsValid());

    RealityDataSourcePtr pShapeDataSource = RealityDataSource::Create("./model/roads.shp", L"shapefile");
    ASSERT_TRUE(pShapeDataSource.IsValid());

    RealityDataSourcePtr pWmsDataDataSource = WmsDataSource::Create("http://sampleserver1.arcgisonline.com/ArcGIS/services/Specialty/ESRI_StatesCitiesRivers_USA/MapServer/WMSServer?service=WMS&request=GetCapabilities&version=1.3.0");
    ASSERT_TRUE(pWmsDataDataSource.IsValid());

    RealityDataSourcePtr pAviDataDataSource = RealityDataSource::Create("./pinned/roadTraffic.avi", L"video/avi");
    ASSERT_TRUE(pAviDataDataSource.IsValid());

    RealityDataSourcePtr pMyHouseDataDataSource = RealityDataSource::Create("./pinned/myHouse.jpeg", L"image/jpeg");
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

        status = pDom->SchemaValidate(GetXsd_1_0().c_str());
        if(BEXML_Success != status) // report error string first.
            {
            WString errorString;
            BeXmlDom::GetLastErrorString (errorString);
            ASSERT_STREQ(L"", errorString.c_str());
            }
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

        ASSERT_STREQ(pLeft->GetSource().GetUri().c_str(), pRight->GetSource().GetUri().c_str());
        ASSERT_STREQ(pLeft->GetSource().GetType().c_str(), pRight->GetSource().GetType().c_str());
        
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

        ASSERT_STREQ(pLeft->GetSource().GetUri().c_str(), pRight->GetSource().GetUri().c_str());
        ASSERT_STREQ(pLeft->GetSource().GetType().c_str(), pRight->GetSource().GetType().c_str());
        }

    // *** Pinned data
    ASSERT_EQ(pPackage->GetPinnedGroup().size(), pReadPackage->GetPinnedGroup().size());
    for(size_t index=0; index < pPackage->GetPinnedGroup().size(); ++index)
        {
        PinnedDataPtr pLeft = pPackage->GetPinnedGroup()[index];
        PinnedDataPtr pRight = pReadPackage->GetPinnedGroup()[index];

        ASSERT_STREQ(pLeft->GetSource().GetUri().c_str(), pRight->GetSource().GetUri().c_str());
        ASSERT_STREQ(pLeft->GetSource().GetType().c_str(), pRight->GetSource().GetType().c_str());
        ASSERT_NEAR(pLeft->GetLocation().x, pRight->GetLocation().x, LONGLAT_EPSILON);
        ASSERT_NEAR(pLeft->GetLocation().y, pRight->GetLocation().y, LONGLAT_EPSILON);
        }

    // *** Terrain data.
    ASSERT_EQ(pPackage->GetTerrainGroup().size(), pReadPackage->GetTerrainGroup().size());
    for(size_t index=0; index < pPackage->GetTerrainGroup().size(); ++index)
        {
        TerrainDataPtr pLeft = pPackage->GetTerrainGroup()[index];
        TerrainDataPtr pRight = pReadPackage->GetTerrainGroup()[index];

        ASSERT_STREQ(pLeft->GetSource().GetUri().c_str(), pRight->GetSource().GetUri().c_str());
        ASSERT_STREQ(pLeft->GetSource().GetType().c_str(), pRight->GetSource().GetType().c_str());
        }
    }

