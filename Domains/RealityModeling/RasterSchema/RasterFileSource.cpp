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
    // Open raster file
    Utf8CP urlUtf8 = properties.m_url.c_str();
    WString fileName(urlUtf8, BentleyCharEncoding::Utf8);
    m_rasterFilePtr = RasterFile::Create(fileName.c_str());
    if (m_rasterFilePtr == nullptr)
        {
        // Can't create model; probably that file name is invalid. 
        return;
        }

    Point2d sizePixels;
    m_rasterFilePtr->GetSize(&sizePixels);

//&&ep tmp - use raster GCS to place it
    m_properties.m_boundingBox.low.x = 0; 
    m_properties.m_boundingBox.high.x = sizePixels.x / 10.0;  
    m_properties.m_boundingBox.low.y = 0; 
    m_properties.m_boundingBox.high.y = sizePixels.y / 10.0; 


//&&ep - add SLO transform to corners (don't apply it in QueryTile (in HRABitmap::Create))
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
    RasterSource::GenerateResolution(resolution, sizePixels.x, sizePixels.y, 256, 256);

/* &&ep - need this ? if no gcs ?
    GeoCoordinates::BaseGCSPtr pGcs = CreateBaseGcsFromWmsGcs(properties.m_csLabel);
    BeAssert(pGcs.IsValid()); //Is it an error if we do not have a GCS? We will assume coincident.

    Initialize(corners, resolution, pGcs.get());
*/
    Initialize(corners, resolution, nullptr);

    }



//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
DisplayTilePtr RasterFileSource::_QueryTile(TileId const& id, bool request)
    {
    if (m_rasterFilePtr == nullptr)
        // RasterFile could not be initialized
        return nullptr;

//&&ep - use my tile size definition ; remove tmp flip (investigate)   
    double scale = 1 << id.resolution;
    HGF2DStretch stretch(HGF2DDisplacement(id.tileX * 256.0 * scale, id.tileY * 256.0 * scale), scale, scale);

//&&ep    - review: use pixel type preferred by QV
    HFCPtr<HRABitmap> pDisplayBitmap;
    uint32_t effectiveTileSizeX = GetTileSizeX(id);
    uint32_t effectiveTileSizeY = GetTileSizeY(id);
    pDisplayBitmap = HRABitmap::Create(effectiveTileSizeX, effectiveTileSizeY, &stretch, m_rasterFilePtr->GetPhysicalCoordSys(), new HRPPixelTypeV32R8G8B8A8());

    HRACopyFromOptions opts;
    opts.SetResamplingMode(HGSResampling::BILINEAR);
    pDisplayBitmap->CopyFrom(*m_rasterFilePtr->GetStoredRaster(), opts);
    Byte* pbSrcRow = pDisplayBitmap->GetPacket()->GetBufferAddress();

//&&ep tmp
    DisplayTile::PixelType pixelType = DisplayTile::PixelType::Rgba;

//&&ep - see (CountPixelRawDataBit * nbPixels sur la largeur(width) +7)/8 (length of a line) * hauteur to set packet size; result of this
    // USE : ComputeBytesPerWidth - NO: but assert that buffer size is ok
//&&ep     - verify packet size is ok each time

//&&ep - keep packet as a member; do setPacket above
    m_tileBuffer.resize(256*256*4);

    DisplayTilePtr pDisplayTile = DisplayTile::Create(effectiveTileSizeX, effectiveTileSizeY, pixelType, pbSrcRow, 0/*notPadded*/);
    return pDisplayTile;
    }

