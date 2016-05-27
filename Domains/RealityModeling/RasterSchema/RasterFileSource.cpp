/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterFileSource.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

    // Raster width and height come from the raster file.
    // And resolution definition should come from the raster resolution descriptor.
    bvector<Resolution> resolution;
    RasterSource::GenerateResolution(resolution, sizePixels.x, sizePixels.y, 256, 256);     // Tile size. We always use 256.

    // Get the raster transform.
    DMatrix4d geoTransform;
    geoTransform = m_rasterFilePtr->GetGeoTransform();
    GeoCoordinates::BaseGCSPtr baseGcsPtr = m_rasterFilePtr->GetBaseGcs();
    Initialize(resolution, geoTransform, baseGcsPtr.get());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  2/2016
//----------------------------------------------------------------------------------------
static HFCPtr<HRPPixelType> s_GetTileQueryPixelType(HRARaster const& raster, Render::Image::Format& format)
    {
    BeAssertOnce(raster.GetPixelType()->CountIndexBits() != 1);       //TODO binary support.

    // if source holds alpha use Rgba
    if (raster.GetPixelType()->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE)
        {
        format = Render::Image::Format::Rgba;
        return new HRPPixelTypeV32R8G8B8A8();
        }

#if defined (NEEDS_WORK_READ_IMAGE)
    if (raster.GetPixelType()->IsCompatibleWith(HRPPixelTypeV8Gray8::CLASS_ID))
        {
        format = Render::Image::Format::Gray;
        return new HRPPixelTypeV8Gray8();
        }
#endif

    format = Render::Image::Format::Rgb;
    return new HRPPixelTypeV24R8G8B8();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
Render::Image RasterFileSource::_QueryTile(TileId const& id, bool& alphaBlend)
    {
    // Imagepp CopyFrom is not thread safe. sequentialize the queries.
    static std::mutex s_ippMutex;
    std::unique_lock<std::mutex> __ippLock(s_ippMutex);

    // Must be done within lock because GetRasterP might load the raster.
    if (m_rasterFilePtr == nullptr || m_rasterFilePtr->GetRasterP() == nullptr)
        return Render::Image();

    Render::Image::Format imageFormat = Render::Image::Format::Rgb;
    HFCPtr<HRPPixelType> pPixelType = s_GetTileQueryPixelType(*m_rasterFilePtr->GetRasterP(), imageFormat);

    uint32_t effectiveTileSizeX = GetTileSizeX(id);
    uint32_t effectiveTileSizeY = GetTileSizeY(id);

    Resolution const& resInfo = GetResolution(0);

    // Use integer type to avoid floating-points precision errors during origin multiplication.
    uint32_t scale = 1 << id.resolution;
    HGF2DStretch stretch(HGF2DDisplacement((id.x * resInfo.GetTileSizeX()) * scale, (id.y * resInfo.GetTileSizeY()) * scale), scale, scale);

    HFCPtr<HRABitmap> pTileBitmap;
    pTileBitmap = HRABitmap::Create(effectiveTileSizeX, effectiveTileSizeY, &stretch, m_rasterFilePtr->GetPhysicalCoordSys(), pPixelType);

    ByteStream dataStream((uint32_t)(effectiveTileSizeY*pTileBitmap->ComputeBytesPerWidth()));

    HFCPtr<HCDPacket> pPacket(new HCDPacket(dataStream.GetDataP(), dataStream.GetSize(), dataStream.GetSize()));
    pTileBitmap->SetPacket(pPacket);

    HRACopyFromOptions opts;
    opts.SetResamplingMode(HGSResampling::BILINEAR);
    if (ImagePPStatus::IMAGEPP_STATUS_Success != pTileBitmap->CopyFrom(*m_rasterFilePtr->GetRasterP(), opts))
        return Render::Image();

    __ippLock.unlock(); // done with imagepp...

#if defined (NEEDS_WORK_READ_IMAGE)
    alphaBlend = (Render::Image::Format::Rgba == imageFormat || Render::Image::Format::Bgra == imageFormat);
#endif
    alphaBlend = (Render::Image::Format::Rgba == imageFormat);
    return Render::Image(effectiveTileSizeX, effectiveTileSizeY, std::move(dataStream), imageFormat);
    }
