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
// @bsimethod                                                       Eric.Paquet     6/2015
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

    DPoint3d corners[4];
    m_rasterFilePtr->GetCorners(corners);

    Point2d sizePixels;
    m_rasterFilePtr->GetSize(&sizePixels);

    // Tile size. We always use 256.
    m_tileSize.x = 256;
    m_tileSize.y = 256;

    // Raster width and height come from the raster file.
    // And resolution definition should come from the raster resolution descriptor.
    bvector<Resolution> resolution;
    RasterSource::GenerateResolution(resolution, sizePixels.x, sizePixels.y, m_tileSize.x, m_tileSize.y);

    GeoCoordinates::BaseGCSPtr baseGcsPtr = m_rasterFilePtr->GetBaseGcs();
//&&ep    - validate baseGcsPtr

/* &&ep - need this ? if no gcs ?
    GeoCoordinates::BaseGCSPtr pGcs = CreateBaseGcsFromWmsGcs(properties.m_csLabel);
    BeAssert(pGcs.IsValid()); //Is it an error if we do not have a GCS? We will assume coincident.

    Initialize(corners, resolution, pGcs.get());
    Initialize(corners, resolution, nullptr);
*/
    DMatrix4d physicalToLowerLeft = m_rasterFilePtr->GetPhysicalToLowerLeft();

    //&&ep fill the transform from raster file. See RasterSource::GetGeoreference(library/RasterCore/RasterSource.cpp)
    DMatrix4d rasterTransform;
    rasterTransform.InitIdentity(); 
    
    DMatrix4d physicalToCartesian;
    physicalToCartesian.InitProduct(rasterTransform, physicalToLowerLeft);
 
    Initialize(resolution, physicalToCartesian, baseGcsPtr.get());
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

    DisplayTile::PixelType pixelType = DisplayTile::PixelType::Rgba;
    DisplayTilePtr pDisplayTile = DisplayTile::Create(effectiveTileSizeX, effectiveTileSizeY, pixelType, pbSrcRow, 0/*notPadded*/);
    return pDisplayTile;
    }

