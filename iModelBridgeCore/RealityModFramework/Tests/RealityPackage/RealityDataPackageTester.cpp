//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/RealityPackage/RealityDataPackageTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <Bentley/BeTest.h>
#include <RealityPackage/RealityDataPackage.h>

USING_BENTLEY_NAMESPACE_REALITYPACKAGE

#define LATLONG_EPSILON 0.000000001 // precision of 0.1 millimeter

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
class PackageTestFixture : public testing::Test 
{
public:
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
    RealityDataSourcePtr pJpegDataSource = RealityDataSource::Create(L"./imagery/map.jpeg", L"image/jpeg");
    ASSERT_TRUE(pJpegDataSource.IsValid());

    RealityDataSourcePtr pPodDataSource = RealityDataSource::Create(L"./terrain/canada.pod", L"pod");
    ASSERT_TRUE(pPodDataSource.IsValid());

    RealityDataSourcePtr pTifDataSource = RealityDataSource::Create(L"./terrain/canada.tif", L"image/tif");
    ASSERT_TRUE(pTifDataSource.IsValid());

    RealityDataSourcePtr pDtmDataSource = RealityDataSource::Create(L"./terrain/canada.dtm", L"dtm");
    ASSERT_TRUE(pDtmDataSource.IsValid());

    RealityDataSourcePtr pShapeDataSource = RealityDataSource::Create(L"./model/roads.shp", L"shapefile");
    ASSERT_TRUE(pShapeDataSource.IsValid());

    RealityDataSourcePtr pWmsDataDataSource = WmsDataSource::Create(L"http://sampleserver1.arcgisonline.com/ArcGIS/services/Specialty/ESRI_StatesCitiesRivers_USA/MapServer/WMSServer?service=WMS&request=GetCapabilities&version=1.3.0");
    ASSERT_TRUE(pWmsDataDataSource.IsValid());

    RealityDataSourcePtr pAviDataDataSource = RealityDataSource::Create(L"./pinned/roadTraffic.avi", L"video/avi");
    ASSERT_TRUE(pAviDataDataSource.IsValid());

    RealityDataSourcePtr pMyHouseDataDataSource = RealityDataSource::Create(L"./pinned/myHouse.jpeg", L"image/jpeg");
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

    PinnedDataPtr pPinnedJpeg = PinnedData::Create(*pMyHouseDataDataSource, 89.123456789, -180);
    ASSERT_TRUE(pPinnedJpeg.IsValid());
    pPackage->GetPinnedGroupR().push_back(pPinnedJpeg);
    pPackage->GetPinnedGroupR().push_back(PinnedData::Create(*pAviDataDataSource, -90, 166.987654321));

    TerrainDataPtr pPodTerrain = TerrainData::Create(*pPodDataSource);
    ASSERT_TRUE(pPodTerrain.IsValid());
    pPackage->GetTerrainGroupR().push_back(pPodTerrain);
    pPackage->GetTerrainGroupR().push_back(TerrainData::Create(*pDtmDataSource));
    pPackage->GetTerrainGroupR().push_back(TerrainData::Create(*pTifDataSource));
       
    
    ASSERT_EQ(RealityPackageStatus::Success, pPackage->Write(outfilename));
    
    //&&MM needwork.
    // Schema validation
//         {
//         BeXmlStatus status = BEXML_Success;
//         BeXmlDomPtr pDom= BeXmlDom::CreateAndReadFromFile (status, outfilename.c_str(), NULL);
// 
//         status = pDom->SchemaValidate(L"C:/Users/Mathieu.Marchand/Desktop/PackageFile/package.xsd");
// 
//         WString errorString;
//         BeXmlDom::GetLastErrorString (errorString);
// 
//         ASSERT_STREQ(L"", errorString.c_str());
//         }
    

    // Validate what we have created.
    WString parseError;
    RealityPackageStatus readStatus = RealityPackageStatus::UnknownError;
    RealityDataPackagePtr pReadPackage = RealityDataPackage::CreateFromFile(readStatus, outfilename, &parseError);
    ASSERT_EQ(RealityPackageStatus::Success, readStatus);
    ASSERT_TRUE(pReadPackage.IsValid());
    ASSERT_STREQ(L"", parseError.c_str()); // we use _STREQ to display the parse error if we have one.


    ASSERT_STREQ(pPackage->GetName().c_str(), pReadPackage->GetName().c_str());
    ASSERT_STREQ(pPackage->GetDescription().c_str(), pReadPackage->GetDescription().c_str());
    ASSERT_STREQ(pPackage->GetCopyright().c_str(), pReadPackage->GetCopyright().c_str());
    ASSERT_STREQ(pPackage->GetPackageId().c_str(), pReadPackage->GetPackageId().c_str());
    ASSERT_TRUE(pPackage->GetCreationDate().Equals(pReadPackage->GetCreationDate()));

    ASSERT_EQ(pPackage->GetBoundingPolygon().GetPointCount(), pReadPackage->GetBoundingPolygon().GetPointCount());
    for(size_t index=0; index < pPackage->GetBoundingPolygon().GetPointCount(); ++index)
        {
        ASSERT_NEAR(pPackage->GetBoundingPolygon().GetPointCP()[index].x, pReadPackage->GetBoundingPolygon().GetPointCP()[index].x, LATLONG_EPSILON);
        ASSERT_NEAR(pPackage->GetBoundingPolygon().GetPointCP()[index].y, pReadPackage->GetBoundingPolygon().GetPointCP()[index].y, LATLONG_EPSILON);
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
            ASSERT_NEAR(pLeft->GetCornersCP()[ImageryData::LowerLeft].x, pRight->GetCornersCP()[ImageryData::LowerLeft].x, LATLONG_EPSILON);
            ASSERT_NEAR(pLeft->GetCornersCP()[ImageryData::LowerLeft].y, pRight->GetCornersCP()[ImageryData::LowerLeft].y, LATLONG_EPSILON);
            ASSERT_NEAR(pLeft->GetCornersCP()[ImageryData::LowerRight].x, pRight->GetCornersCP()[ImageryData::LowerRight].x, LATLONG_EPSILON);
            ASSERT_NEAR(pLeft->GetCornersCP()[ImageryData::LowerRight].y, pRight->GetCornersCP()[ImageryData::LowerRight].y, LATLONG_EPSILON);
            ASSERT_NEAR(pLeft->GetCornersCP()[ImageryData::UpperLeft].x, pRight->GetCornersCP()[ImageryData::UpperLeft].x, LATLONG_EPSILON);
            ASSERT_NEAR(pLeft->GetCornersCP()[ImageryData::UpperLeft].y, pRight->GetCornersCP()[ImageryData::UpperLeft].y, LATLONG_EPSILON);
            ASSERT_NEAR(pLeft->GetCornersCP()[ImageryData::UpperRight].x, pRight->GetCornersCP()[ImageryData::UpperRight].x, LATLONG_EPSILON);
            ASSERT_NEAR(pLeft->GetCornersCP()[ImageryData::UpperRight].y, pRight->GetCornersCP()[ImageryData::UpperRight].y, LATLONG_EPSILON);
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
        ASSERT_NEAR(pLeft->GetLocation().x, pRight->GetLocation().x, LATLONG_EPSILON);
        ASSERT_NEAR(pLeft->GetLocation().y, pRight->GetLocation().y, LATLONG_EPSILON);
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

