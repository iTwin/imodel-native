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
    
    // Allocate packet. We always use same pixel type.
    HFCPtr<HRPPixelType> pixelTypePtr = new HRPPixelTypeV32R8G8B8A8();
    size_t bufferSize;
    Byte* pBuffer = CreateWorkingBuffer(bufferSize, *pixelTypePtr, m_tileSize.x, m_tileSize.y);
    m_packetPtr = new HCDPacket(pBuffer, bufferSize, bufferSize);

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
    //&&ep need to take the 'request' param into account... When we will have a copyfrom thread.

    if (m_rasterFilePtr == nullptr)
        // RasterFile could not be initialized
        return nullptr;

    // Use integer type to avoid floating-points precision errors during origin multiplication.
    uint32_t scale = 1 << id.resolution;
    HGF2DStretch stretch(HGF2DDisplacement((id.x * m_tileSize.x) * scale, (id.y * m_tileSize.y) * scale), scale, scale);

    HFCPtr<HRABitmap> pDisplayBitmap;
    uint32_t effectiveTileSizeX = GetTileSizeX(id);
    uint32_t effectiveTileSizeY = GetTileSizeY(id);
    pDisplayBitmap = HRABitmap::Create(effectiveTileSizeX, effectiveTileSizeY, &stretch, m_rasterFilePtr->GetPhysicalCoordSys(), new HRPPixelTypeV32R8G8B8A8());

    HRACopyFromOptions opts;
    opts.SetResamplingMode(HGSResampling::BILINEAR);

    // Buffer size of m_packetPtr is supposed to be large enough 
    BeAssert((((m_tileSize.x * pDisplayBitmap->GetPixelType()->CountPixelRawDataBits()) + 7) / 8) * m_tileSize.y <= m_packetPtr->GetBufferSize());

    // The packet is kept as a member, which avoids to allocate a buffer each time we pass here.
    pDisplayBitmap->SetPacket(m_packetPtr);

	pDisplayBitmap->CopyFrom(*m_rasterFilePtr->GetStoredRasterP(), opts);
    Byte* pbSrcRow = pDisplayBitmap->GetPacket()->GetBufferAddress();

    bool alphaBlend = m_rasterFilePtr->GetStoredRasterP()->GetPixelType()->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE;

    DisplayTile::PixelType pixelType = DisplayTile::PixelType::Rgba;
    DisplayTilePtr pDisplayTile = DisplayTile::Create(effectiveTileSizeX, effectiveTileSizeY, pixelType, alphaBlend, pbSrcRow, 0/*notPadded*/);
    return pDisplayTile;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     7/2015
//----------------------------------------------------------------------------------------
Byte* RasterFileSource::CreateWorkingBuffer(size_t& bufferSize, const HRPPixelType& pi_rPixelType, uint32_t pi_Width, uint32_t pi_Height) const
{
	size_t BytesPerLine;

	// In 1 bit RLE, allocate worst case
	if (pi_rPixelType.IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID) ||
		pi_rPixelType.IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID))
		BytesPerLine = (pi_Width * 2 + 2) * sizeof(unsigned short);
	else
		BytesPerLine = ((pi_Width * pi_rPixelType.CountPixelRawDataBits()) + 7) / 8;

    bufferSize = BytesPerLine * pi_Height;
	return new Byte[bufferSize];
}
