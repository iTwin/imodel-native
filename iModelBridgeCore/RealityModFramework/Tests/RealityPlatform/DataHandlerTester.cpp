//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/RealityPlatform/DataHandlerTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#ifdef REALITYMODFRAMEWORK_LOCALTEST


#include <Bentley/BeTest.h>
#include <RealityPlatform/RealityDataHandler.h>


#define THUMBNAIL_WIDTH     512
#define THUMBNAIL_HEIGHT    512

#define RASTER_FILENAME             "D:\\Test\\In\\2180.tif"
#define RASTER_THUMBNAIL_FILENAME   "RasterThumbnail.png"
#define RASTER_FOOTPRINT_FILENAME   "RasterFootprint.txt"

#define POINTCLOUD_FILENAME             "D:\\Test\\In\\VancouverLibraryGeoRef.pod"
#define POINTCLOUD_THUMBNAIL_FILENAME   "PointCloudThumbnail.jpg"
#define POINTCLOUD_FOOTPRINT_FILENAME   "PointCloudFootprint.txt"
#define POINTCLOUD_VIEW                 RealityPlatform::PointCloudView::Top

#define WMS_GETMAP_URL          "http://giswebservices.massgis.state.ma.us/geoserver/wms?VERSION=1.1.1&REQUEST=GetMap&SERVICE=WMS&LAYERS=massgis:GISDATA.TOWNS_POLYM,massgis:GISDATA.NAVTEQRDS_ARC,massgis:GISDATA.NAVTEQRDS_ARC_INT&SRS=EPSG:26986&BBOX=232325.38526025353,898705.3447384972,238934.49648710093,903749.1401484597&WIDTH=570&HEIGHT=435&FORMAT=image/png&STYLES=Black_Lines,GISDATA.NAVTEQRDS_ARC::ForOrthos,GISDATA.NAVTEQRDS_ARC_INT::Default&TRANSPARENT=TRUE"
#define WMS_THUMBNAIL_FILENAME  "WmsThumbnail.jpg"  
#define WMS_FOOTPRINT_FILENAME  "WmsFootprint.txt"


USING_NAMESPACE_BENTLEY_REALITYPLATFORM


//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote               9/2015
//=====================================================================================
class RealityDataTestFixture : public testing::Test
    {
    public:
        //-------------------------------------------------------------------------------------
        // @bsimethod                                   Jean-Francois.Cote         		 9/2015
        //-------------------------------------------------------------------------------------
        BeFileName BuildOutputFilename(Utf8CP filename)
            {
            WString filenameW;
            BeStringUtilities::Utf8ToWChar(filenameW, filename);

            BeFileName outputFilePath;
            BeTest::GetHost().GetOutputRoot(outputFilePath);
            outputFilePath.AppendToPath(filenameW.c_str());

            return outputFilePath;
            }
    };

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
TEST_F(RealityDataTestFixture, GetRasterThumbnail)
    {
    bvector<Byte> data;
    StatusInt status = ERROR;


    RealityDataPtr pRasterData = RasterData::Create(RASTER_FILENAME);
    ASSERT_TRUE(pRasterData.IsValid());

    status = pRasterData->GetThumbnail(data, THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT);
    ASSERT_EQ(SUCCESS, status);

    status = pRasterData->SaveThumbnail(data, BuildOutputFilename(RASTER_THUMBNAIL_FILENAME));
    ASSERT_EQ(SUCCESS, status);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
//TEST_F(RealityDataTestFixture, GetRasterFootprint)
//    {
//    DRange2d shape;
//    StatusInt status = ERROR;
//
//
//    RealityDataPtr pRasterData = RealityPlatform::RasterData::Create(RASTER_FILENAME);
//    ASSERT_TRUE(pRasterData.IsValid());
//
//    status = pRasterData->GetFootprint(&shape);
//    ASSERT_EQ(SUCCESS, status);
//
//    status = Save(shape, RASTER_FOOTPRINT_FILENAME);
//    ASSERT_EQ(SUCCESS, status);
//    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
TEST_F(RealityDataTestFixture, GetPointCloudThumbnail)
    {
    bvector<Byte> data;
    StatusInt status = ERROR;


    RealityDataPtr pPointCloudData = PointCloudData::Create(POINTCLOUD_FILENAME, POINTCLOUD_VIEW);
    ASSERT_TRUE(pPointCloudData.IsValid());

    status = pPointCloudData->GetThumbnail(data, THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT);
    ASSERT_EQ(SUCCESS, status);

    status = pPointCloudData->SaveThumbnail(data, BuildOutputFilename(POINTCLOUD_THUMBNAIL_FILENAME));
    ASSERT_EQ(SUCCESS, status);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
//TEST_F(RealityDataTestFixture, GetPointCloudFootprint)
//    {
//    DRange2d shape;
//    StatusInt status = ERROR;
//
//
//    RealityDataPtr pPointCloudData = RealityPlatform::PointCloudData::Create(POINTCLOUD_FILENAME, POINTCLOUD_VIEW);
//    ASSERT_TRUE(pPointCloudData.IsValid());
//
//    status = pPointCloudData->GetFootprint(&shape);
//    ASSERT_EQ(SUCCESS, status);
//
//    status = Save(shape, POINTCLOUD_FOOTPRINT_FILENAME);
//    ASSERT_EQ(SUCCESS, status);
//    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
TEST_F(RealityDataTestFixture, GetWmsThumbnail)
    {
    bvector<Byte> data;
    StatusInt status = ERROR;


    RealityDataPtr pWmsData = WmsData::Create(WMS_GETMAP_URL);
    ASSERT_TRUE(pWmsData.IsValid());

    status = pWmsData->GetThumbnail(data, THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT);
    ASSERT_EQ(SUCCESS, status);

    status = pWmsData->SaveThumbnail(data, BuildOutputFilename(WMS_THUMBNAIL_FILENAME));
    ASSERT_EQ(SUCCESS, status);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
//TEST_F(RealityDataTestFixture, GetWmsFootprint)
//    {
//    DRange2d shape;
//    StatusInt status = ERROR;
//
//
//    RealityDataPtr pWmsData = RealityPlatform::WmsData::Create(WMS_GETMAP_URL);
//    ASSERT_TRUE(pWmsData.IsValid());
//
//    status = pWmsData->GetFootprint(&shape);
//    ASSERT_EQ(SUCCESS, status);
//
//    status = Save(shape, WMS_FOOTPRINT_FILENAME);
//    ASSERT_EQ(SUCCESS, status);
//    }

#endif
