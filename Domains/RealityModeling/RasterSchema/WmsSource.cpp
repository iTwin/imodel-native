/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/WmsSource.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RasterSchemaInternal.h"
#include <DgnPlatform/DgnCore/ImageUtilities.h>
#include "WmsSource.h"
#include <DgnPlatform/DgnCore/WebMercator.h>        //&&MM temporary for TiledRaster.


//----------------------------------------------------------------------------------------
//-------------------------------  WmsSource      ----------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
RasterSourcePtr WmsSource::Create(Utf8CP url, Utf8CP version, Utf8CP layers, Utf8CP styles, Utf8CP csType, Utf8CP cs, Utf8CP format, 
                                 DRange2dCR bbox, uint32_t metaWidth, uint32_t metaHeight)
    {
    return new WmsSource(url, version, layers, styles, csType, cs, format, bbox, metaWidth, metaHeight);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
WmsSource::WmsSource(Utf8CP url, Utf8CP version, Utf8CP layers, Utf8CP styles, Utf8CP csType, Utf8CP cs, Utf8CP format, 
                     DRange2dCR bbox, uint32_t metaWidth, uint32_t metaHeight)
 :m_serverUrl(url),
  m_version(version),
  m_layers(layers),
  m_styles(styles),
  m_gcsType(csType),
  m_gcs(cs),
  m_format(format),
  m_boundingBox(bbox),
  m_transparent("TRUE"),
  m_backgroundColor("0xFFFFFF"),
  m_metaWidth(metaWidth),
  m_metaHeight(metaHeight)
    {
    // WMS BBOX are in CRS units. i.e. cartesian.       //&&MM todo bbox reorder adjustment needed?
    DPoint3d corners[4];
    corners[0].x = corners[2].x = m_boundingBox.low.x; 
    corners[1].x = corners[3].x = m_boundingBox.high.x;  
    corners[0].y = corners[1].y = m_boundingBox.low.y; 
    corners[2].y = corners[3].y = m_boundingBox.high.y; 
    corners[0].z = corners[1].z = corners[2].z = corners[3].z = 0;

    // for WMS we user a generic 256x256 multi-resolution image.
    bvector<Resolution> resolution;
    RasterSource::GenerateResolution(resolution, metaWidth, metaHeight, 256, 256);

    Initialize(WString(url, true).c_str(), corners, resolution);
            
    GeoCoordinates::BaseGCSPtr pGcs = CreateBaseGcsFromWmsGcs(m_gcs);
    BeAssert(pGcs.IsValid()); //&&MM for WMS it must be an error if we do not have a GCS.
    SetGcsP(pGcs.get());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
GeoCoordinates::BaseGCSPtr WmsSource::CreateBaseGcsFromWmsGcs(Utf8StringCR gcsStr)
    {
    // WMS GCS ex:
    //  - EPSG:26986
    //  - CRS:83, CRS:27, CRS:84, or some Bentley WMS server geocoord keyname.
   
    //1) Attempt to build from EPSG code.   
    if(0 == BeStringUtilities::Strnicmp(gcsStr.c_str(), "EPSG:", 5))
        {
        Utf8String epsgCodeStr = gcsStr.substr(5);
        int epsgCode = atoi(epsgCodeStr.c_str());

        GeoCoordinates::BaseGCSPtr pGcs = GeoCoordinates::BaseGCS::CreateGCS();
        if(SUCCESS == pGcs->InitFromEPSGCode(NULL, NULL, epsgCode))
            return pGcs;       
        }

    //2) Attempt from keyname
    WString keyName(gcsStr.c_str(), true/*utf8*/);
    GeoCoordinates::BaseGCSPtr pGcs = GeoCoordinates::BaseGCS::CreateGCS(keyName.c_str());

    return pGcs->IsValid() ? pGcs : NULL;    
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
DisplayTilePtr WmsSource::_QueryTile(TileId const& id, bool request)
    {
    //&&MM source should return raster data so caller can do what they want with it.
    Utf8String tileUrl = BuildTileUrl(id);

    //&&MM I don't understand this concept of expected image info. Why we need it?
    ImageUtilities::RgbImageInfo expectedImageInfo;
    expectedImageInfo.width = GetEffectiveTileSizeX(id);
    expectedImageInfo.height = GetEffectiveTileSizeY(id);
    expectedImageInfo.hasAlpha = false;
    expectedImageInfo.isBGR = false;
    expectedImageInfo.isTopDown = true;

#if 0 //&&MM not now. need our own tiledraster
    //&&MM We are using tiledRaster but it should be extended to support more pixeltype and compression or have a new type?
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
    DisplayTilePtr pDisplayTile = DisplayTile::Create(actualImageInfo.width, actualImageInfo.height, pixelType, m_decompressBuffer.data(), 0/*notPadded*/);

    return pDisplayTile;
#else 
    return NULL;
#endif
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
Utf8String WmsSource::BuildTileUrl(TileId const& tileId)
    {
    DPoint3d tileCorners[4];
    ComputeTileCorners(tileCorners, tileId);

    // &&MM need to limit our request for border tiles to the data window range?  
    // &&MM Snap border tiles to requested data window?
    GeoPoint2d tileOrigin;
    tileOrigin.latitude = tileCorners[0].y;
    tileOrigin.longitude = tileCorners[0].x;

    GeoPoint2d tileCorner;
    tileCorner.latitude = tileCorners[3].y;
    tileCorner.longitude = tileCorners[3].x;
    
    //&&MM order of lat/long is a mess. review for all version and I guess define a way to be user defined. ex lat_long_LAT_LONG or long_lat_LONG_LAT
    // spec 1.1.1 >>> minimum longitude, minimum latitude, maximum longitude, maximum latitude 
    // also not sure if latitude I think it's cartesian
    double minX = tileOrigin.longitude;
    double minY = tileOrigin.latitude;
    double maxX = tileCorner.longitude;
    double maxY = tileCorner.latitude;

    // Mandatory parameters
    Utf8String tileUrl;
    tileUrl.Sprintf("%s?VERSION=%s&REQUEST=GetMap&LAYERS=%s&STYLES=%s&%s=%s&BBOX=%f,%f,%f,%f&WIDTH=%d&HEIGHT=%d&FORMAT=%s", 
        m_serverUrl.c_str(), m_version.c_str(), m_layers.c_str(), m_styles.c_str(), m_gcsType.c_str(), m_gcs.c_str(), 
        minX, minY, maxX, maxY,
        GetEffectiveTileSizeX(tileId), GetEffectiveTileSizeY(tileId), m_format.c_str());
       
    // Optional parameters
    if(!m_transparent.empty())
        {
        tileUrl.append("&TRANSPARENT=");
        tileUrl.append(m_transparent);
        }

    if(!m_backgroundColor.empty())
        {
        tileUrl.append("&BGCOLOR=");
        tileUrl.append(m_backgroundColor);
        }

    if(!m_vendorSpecific.empty())
        {
        tileUrl.append("&");
        tileUrl.append(m_vendorSpecific);
        }
    
    return tileUrl;   
    }
