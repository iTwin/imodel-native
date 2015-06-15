/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterFileSource.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RasterSchemaInternal.h"
#include "RasterFileSource.h"


//----------------------------------------------------------------------------------------
//-------------------------------   RasterFileSource   -----------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
RasterSourcePtr RasterFileSource::Create(RasterFileProperties const& properties)
    {
    return new RasterFileSource(properties);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
RasterFileSource::RasterFileSource(RasterFileProperties const& properties)
 :m_properties(properties) 
    {
//&&ep tmp - use from properties when available
m_properties.m_boundingBox.low.x = 0; 
m_properties.m_boundingBox.high.x = 10;  
m_properties.m_boundingBox.low.y = 0; 
m_properties.m_boundingBox.high.y = 20; 


    // WMS BBOX are in CRS units. i.e. cartesian.       // &&ep is this true ?
    DPoint3d corners[4];
    corners[0].x = corners[2].x = m_properties.m_boundingBox.low.x; 
    corners[1].x = corners[3].x = m_properties.m_boundingBox.high.x;  
    corners[0].y = corners[1].y = m_properties.m_boundingBox.low.y; 
    corners[2].y = corners[3].y = m_properties.m_boundingBox.high.y; 
    corners[0].z = corners[1].z = corners[2].z = corners[3].z = 0;

    //&&EP raster width and height must come from the raster file.
    //     and resolution definition should come from the raster resolution descriptor.
    bvector<Resolution> resolution;
    RasterSource::GenerateResolution(resolution, 1000, 2000, 256, 256);

/* &&ep - need this ? if no gcs ?
    GeoCoordinates::BaseGCSPtr pGcs = CreateBaseGcsFromWmsGcs(properties.m_csLabel);
    BeAssert(pGcs.IsValid()); //Is it an error if we do not have a GCS? We will assume coincident.

    Initialize(corners, resolution, pGcs.get());
*/
    Initialize(corners, resolution, nullptr);

/* &&ep d

    // WMS BBOX are in CRS units. i.e. cartesian.       //&&MM todo bbox reorder adjustment needed?
    DPoint3d corners[4];
    corners[0].x = corners[2].x = m_properties.m_boundingBox.low.x; 
    corners[1].x = corners[3].x = m_properties.m_boundingBox.high.x;  
    corners[0].y = corners[1].y = m_properties.m_boundingBox.low.y; 
    corners[2].y = corners[3].y = m_properties.m_boundingBox.high.y; 
    corners[0].z = corners[1].z = corners[2].z = corners[3].z = 0;

    // for WMS we define a 256x256 multi-resolution image.
    bvector<Resolution> resolution;
    RasterSource::GenerateResolution(resolution, m_properties.m_metaWidth, m_properties.m_metaHeight, 512, 512);

    GeoCoordinates::BaseGCSPtr pGcs = CreateBaseGcsFromWmsGcs(properties.m_csLabel);
    BeAssert(pGcs.IsValid()); //Is it an error if we do not have a GCS? We will assume coincident.

    Initialize(corners, resolution, pGcs.get());
*/
    }



//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
DisplayTilePtr RasterFileSource::_QueryTile(TileId const& id, bool request)
    {

//&&ep tmp
    DisplayTile::PixelType pixelType = DisplayTile::PixelType::Rgba;

    m_tileBuffer.resize(256*256*4);
    for (int i = 0; i < 256*256; i++)
        {
        int idx = i*4;
        m_tileBuffer[idx++] = 0;
        m_tileBuffer[idx++] = 255;
        m_tileBuffer[idx++] = 0;
        m_tileBuffer[idx] = 5;
        }

    DisplayTilePtr pDisplayTile = DisplayTile::Create(256, 256, pixelType, m_tileBuffer.data(), 0/*notPadded*/);
    return pDisplayTile;


/* &&ep - todo

    //&&MM source should return raster data so caller can do what they want with it.
    Utf8String tileUrl = BuildTileUrl(id);

    //&&MM I don't understand this concept of expected image info. Why we need it?
    ImageUtilities::RgbImageInfo expectedImageInfo;
    expectedImageInfo.width = GetTileSizeX(id);
    expectedImageInfo.height = GetTileSizeY(id);
    expectedImageInfo.hasAlpha = false;
    expectedImageInfo.isBGR = false;
    expectedImageInfo.isTopDown = true;

#if 0 //&&MM not now. need our own tiledraster
    //      We are using tiledRaster but it should be extended to support more pixeltype and compression or have a new type?
    //      Maybe we should create a better Image object than RgbImageInfo and use that.
    RefCountedPtr<TiledRaster::RequestOptions> pOptions;
    if(request)
        pOptions = TiledRaster::RequestOptions::Create(expectedImageInfo, true);
    else
        pOptions = TiledRaster::RequestOptions::Create(true);
        
    //&&MM for WMS it looks like I will need another kind of tiledRaster to handle exception response from the server.
    //for example, a badly formated request generate an XML response. This is badly interpreted as a valid response(HttpRealityDataSourceRequest::_Handle) and 
    // stored into the dataCache.  contentType equals "application/vnd.ogc.se_xml" (required by wms spec).
    // Poss sol:  reject in TiledRaster::_InitFrom
    //&&MM as the database get bigger(I guess 500MB) the first call is very very slow. sorting by string is probably not a good idea either
    //     Maybe one table per server?  and use TileId or hash the url ?
    RefCountedPtr<TiledRaster> pTiledRaster = T_HOST.GetRealityDataAdmin().GetCache().Get<TiledRaster>(tileUrl.c_str(), *pOptions);

    if(!pTiledRaster.IsValid())
        return NULL;

    auto const& data = pTiledRaster->GetData();
    ImageUtilities::RgbImageInfo actualImageInfo = pTiledRaster->GetImageInfo();
    Utf8String contentType = pTiledRaster->GetContentType();
    
    BentleyStatus status;

    m_decompressBuffer.clear(); // reuse the same buffer, in order to minimize mallocs

    if (contentType.Equals ("image/png"))
        {
        status = ImageUtilities::ReadImageFromPngBuffer (m_decompressBuffer, actualImageInfo, data.data(), data.size());
        }
    else if (contentType.Equals ("image/jpeg"))
        {
        status = ImageUtilities::ReadImageFromJpgBuffer (m_decompressBuffer, actualImageInfo, data.data(), data.size(), expectedImageInfo);
        }
    else
        {
        BeAssertOnce (false && "Unsupported image type");
        status = BSIERROR;
        }

    //&&MM we probably need a way to handle errors and avoid trying over and over again.
    if (SUCCESS != status)
        return NULL;
    
    BeAssert (!actualImageInfo.isBGR);    //&&MM todo 
    DisplayTile::PixelType pixelType = actualImageInfo.hasAlpha ? DisplayTile::PixelType::Rgba : DisplayTile::PixelType::Bgr;
    DisplayTilePtr pDisplayTile = DisplayTile::Create(actualImageInfo.width, actualImageInfo.height, pixelType, m_decompressBuffer.data(), 0/*notPadded*);

    return pDisplayTile;
#else 
    return NULL;
#endif

*/

    }

