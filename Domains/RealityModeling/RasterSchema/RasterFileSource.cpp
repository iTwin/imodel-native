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
RasterSourcePtr RasterFileSource::Create(Utf8StringCR resolvedName)
    {
    return new RasterFileSource(resolvedName);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
RasterFileSource::RasterFileSource(Utf8StringCR resolvedName)
    {
    // Open raster file
    m_rasterFilePtr = RasterFile::Create(resolvedName);
    if (m_rasterFilePtr == nullptr)
        {
        // Can't create model; probably that file name is invalid. 
        return;
        }

    Point2d sizePixels;
    m_rasterFilePtr->GetSize(&sizePixels);

    // Tile size. We always use 256.
    m_tileSize.x = 256;
    m_tileSize.y = 256;

    // Raster width and height come from the raster file.
    // And resolution definition should come from the raster resolution descriptor.
    bvector<Resolution> resolution;
    RasterSource::GenerateResolution(resolution, sizePixels.x, sizePixels.y, m_tileSize.x, m_tileSize.y);

    // Get the raster transform.
    DMatrix4d geoTransform;
    geoTransform = m_rasterFilePtr->GetGeoTransform();
    GeoCoordinates::BaseGCSPtr baseGcsPtr = m_rasterFilePtr->GetBaseGcs();
    Initialize(resolution, geoTransform, baseGcsPtr.get());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
DisplayTilePtr RasterFileSource::_QueryTile(TileId const& id, bool request)
    {
    if (m_rasterFilePtr == nullptr)
        // RasterFile could not be initialized
        return nullptr;

    // Use integer type to avoid floating-points precision errors durint origin multiplication.
    uint32_t scale = 1 << id.resolution;
    HGF2DStretch stretch(HGF2DDisplacement((id.x * m_tileSize.x) * scale, (id.y * m_tileSize.y) * scale), scale, scale);

//&&ep    - review: use pixel type preferred by QV
    HFCPtr<HRABitmap> pDisplayBitmap;
    uint32_t effectiveTileSizeX = GetTileSizeX(id);
    uint32_t effectiveTileSizeY = GetTileSizeY(id);
    pDisplayBitmap = HRABitmap::Create(effectiveTileSizeX, effectiveTileSizeY, &stretch, m_rasterFilePtr->GetPhysicalCoordSys(), new HRPPixelTypeV32R8G8B8A8());

    HRACopyFromOptions opts;
    opts.SetResamplingMode(HGSResampling::BILINEAR);
    pDisplayBitmap->CopyFrom(*m_rasterFilePtr->GetStoredRasterP(), opts);
    Byte* pbSrcRow = pDisplayBitmap->GetPacket()->GetBufferAddress();

//&&ep - see (CountPixelRawDataBit * nbPixels sur la largeur(width) +7)/8 (length of a line) * hauteur to set packet size; result of this
    // USE : ComputeBytesPerWidth - NO: but assert that buffer size is ok
//&&ep     - verify packet size is ok each time

//&&ep - keep packet as a member; do setPacket above
    //m_tileBuffer.resize(m_tileSize.x * m_tileSize.y *4);

    bool alphaBlend = m_rasterFilePtr->GetStoredRasterP()->GetPixelType()->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE;

    DisplayTile::PixelType pixelType = DisplayTile::PixelType::Rgba;
    DisplayTilePtr pDisplayTile = DisplayTile::Create(effectiveTileSizeX, effectiveTileSizeY, pixelType, alphaBlend, pbSrcRow, 0/*notPadded*/);
    return pDisplayTile;
    }

