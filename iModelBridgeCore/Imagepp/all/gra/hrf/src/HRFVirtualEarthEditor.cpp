//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFVirtualEarthEditor
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include "HRFVirtualEarthEditor.h"
#include <ImagePP/all/h/HFCCallbacks.h>
#include <ImagePP/all/h/HRFVirtualEarthFile.h>
#include <ImagePP/all/h/HRPPixelConverter.h>
#include <ImagePP/all/h/HRFPngFile.h>
#include <ImagePPInternal/gra/Task.h>
#include <ImagePPInternal/HttpConnection.h>
#include <BeJpeg/BeJpeg.h>

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  1/2016
//----------------------------------------------------------------------------------------
void VirtualEarthTileQuery::_Run()
    {
    HttpSession& session = m_rasterFile.GetThreadLocalHttpSession();
    HttpRequest request(m_tileUri.c_str());
    
    SetProxyInfo(request);

    HttpResponsePtr response;
    if(HttpRequestStatus::Success != session.Request(response, request) || response.IsNull() || response->GetBody().empty())
        return;

    auto contentTypeItr = response->GetHeader().find("Content-Type"); // case insensitive find.
    if(contentTypeItr == response->GetHeader().end())
        return;

    auto tileInfoItr = response->GetHeader().find("X-VE-Tile-Info");
    if (tileInfoItr != response->GetHeader().end())
        {
        // We can tell from the header response if the tile is a missing tile(Kodak) or the real thing.
        m_isNoTile = tileInfoItr->second.EqualsI("no-tile");
        } 
       
    if(contentTypeItr->second.EqualsI("image/png"))
        {
        bool isRGBA = false;
        uint32_t width, height;       
        if(SUCCESS == HRFPngFile::ReadToBuffer(m_tileData, width, height, isRGBA, response->GetBody().data(), response->GetBody().size()))
            {
            BeAssert(256 == width && 256 == height);

            if(isRGBA)  // we always output rgb, convert now if not.
                {
                bvector<Byte> rgbData(256*256*3);

                HRPPixelTypeV24R8G8B8 v24;
                HFCPtr<HRPPixelConverter> pRgbaToRgb = HRPPixelTypeV32R8G8B8A8().GetConverterTo(&v24);

                for(uint32_t line=0; line < height; ++line)
                    pRgbaToRgb->Convert(m_tileData.data() + line*width*4, rgbData.data() + line*256*3, width);

                m_tileData = std::move(rgbData);
                }
            }
        else
            {
            m_tileData.clear(); // ERROR
            }
        }
    else if(contentTypeItr->second.EqualsI("image/jpeg"))
        {
        m_tileData.resize(256*256*3); // assumed we have rgb data.
        
        BeJpegDecompressor decomp;                             
        if(SUCCESS != decomp.Decompress(m_tileData.data(), m_tileData.size(), response->GetBody().data(), response->GetBody().size(), BeJpegPixelType::BE_JPEG_PIXELTYPE_Rgb))
            m_tileData.clear(); // ERROR
        }
    else
        {
        BeAssertOnce(!"unsupported Content-Type");
        }

    m_rasterFile.NotifyBlockReady(0/*page*/, m_tileId);
    }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HRFVirtualEarthEditor::HRFVirtualEarthEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                             uint32_t              pi_Page,
                                             uint16_t       pi_Resolution,
                                             HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    m_TileIDDescriptor = HGFTileIDDescriptor(m_pResolutionDescriptor->GetWidth(),
                                             m_pResolutionDescriptor->GetHeight(),
                                             m_pResolutionDescriptor->GetBlockWidth(),
                                             m_pResolutionDescriptor->GetBlockHeight());

    // Our tile query assumed that we received 256x256 tiles and that we want RGB output.
    BeAssertOnce(m_pResolutionDescriptor->GetBlockWidth() == 256 && m_pResolutionDescriptor->GetBlockHeight() == 256 && 
                 m_pResolutionDescriptor->GetPixelType()->IsCompatibleWith(HRPPixelTypeId_V24R8G8B8));
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRFVirtualEarthEditor::~HRFVirtualEarthEditor()
    {
    }

//-----------------------------------------------------------------------------
// Read a block of data
//-----------------------------------------------------------------------------
HSTATUS HRFVirtualEarthEditor::ReadBlock(uint64_t pi_PosBlockX, uint64_t pi_PosBlockY, Byte* po_pData)
    {
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(m_pResolutionDescriptor->GetBlockType() == HRFBlockType::TILE);

    ReadTileStatus readStatus = QueryTile(po_pData, pi_PosBlockX, pi_PosBlockY);
    if (ReadTileStatus::Success == readStatus)
        return H_SUCCESS;

    if (ReadTileStatus::NoTile == readStatus)
        {
        // No tile data(aka Kodak/cameras), attempt to generate one using sub-resolution otherwise we return the original data.
        QueryTileSubstitute(po_pData, pi_PosBlockX, pi_PosBlockY);
        return H_SUCCESS;
        }

    // If the connection failed or whatever...
    // Clear output and return SUCCESS anyway because HRFObjectStore will add an empty tile to the pool and 
    // every CopyFrom will failed to copy pixels which will leave the destination with uninitialized pixels.
    // Lets hope we can improve this behavior in the future. e.g. do not add it to the pool and try again next time.
    memset(po_pData, 0, GetResolutionDescriptor()->GetBlockSizeInBytes());

    return H_SUCCESS;
    }

//-----------------------------------------------------------------------------
// Read a block of data
//-----------------------------------------------------------------------------
HRFVirtualEarthEditor::ReadTileStatus HRFVirtualEarthEditor::QueryTile(Byte* po_pData, uint64_t pi_PosBlockX, uint64_t pi_PosBlockY)
    {
    // check if the tile is already in the pool
    uint64_t TileID = m_TileIDDescriptor.ComputeID(pi_PosBlockX, pi_PosBlockY, m_Resolution);

    RefCountedPtr<VirtualEarthTileQuery> pTileQuery;

    HRFVirtualEarthFile& rasterFile = static_cast<HRFVirtualEarthFile&>(*GetRasterFile());

    rasterFile.m_tileQueryMapMutex.lock();
    auto tileQueryItr = rasterFile.m_tileQueryMap.find(TileID);
    bool notFoundNode = tileQueryItr == rasterFile.m_tileQueryMap.end();    

    if(notFoundNode)
        {
        rasterFile.m_tileQueryMapMutex.unlock();

#ifdef _RETURN_RED_TILES_IF_NOT_IN_LOOKAHEAD    // Debug purpose only
        memset(po_pData, 0, GetResolutionDescriptor()->GetBlockSizeInBytes()); 

        // Mark all non lookahead tiles in red.
        for(uint64_t i=0; i < GetResolutionDescriptor()->GetBlockWidth()*GetResolutionDescriptor()->GetBlockHeight(); ++i)
            po_pData[i*3] = 255;
        
        return ReadTileStatus::Success;
#else
        // Tile was not in lookAHead create a request.
        pTileQuery = new VirtualEarthTileQuery(TileID, BuildTileUri(TileID), rasterFile);

        // Add to the query map so QueryTileSubstitute can reuse them when possible.
        rasterFile.m_tileQueryMap.insert({TileID, pTileQuery});

        WorkerPool& pool = rasterFile.GetWorkerPool();
        pool.Enqueue(*pTileQuery, true/*atFront*/); 
#endif    
        }
    else
        {
        pTileQuery = tileQueryItr->second;
        rasterFile.m_tileQueryMapMutex.unlock();
        }

    pTileQuery->Wait();

    if (pTileQuery->m_tileData.empty())
        return ReadTileStatus::Error;

    BeAssert(pTileQuery->m_tileData.size() == GetResolutionDescriptor()->GetBlockSizeInBytes());
    memcpy(po_pData, pTileQuery->m_tileData.data(), GetResolutionDescriptor()->GetBlockSizeInBytes());

    return pTileQuery->m_isNoTile ? ReadTileStatus::NoTile : ReadTileStatus::Success;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2016
//----------------------------------------------------------------------------------------
void HRFVirtualEarthEditor::MagnifyTile(Byte* pOutput, Byte const* pSource, size_t srcOffsetX, size_t srcOffsetY, uint32_t srcResolution)
    {
    BeAssert(srcResolution > GetResolutionIndex());

    const size_t tilePitch = 256 * 3;
    Byte const* pSrc = pSource + (srcOffsetY*tilePitch) + (srcOffsetX * 3);

    uint32_t resolutionDelta = srcResolution - GetResolutionIndex();

    static bool s_nearest = false;
    if (s_nearest)
        {
        for (size_t line = 0; line < 256; ++line)
            {
            Byte* pOutLine = pOutput + line * tilePitch;
            Byte const* pSrcLine = pSrc + ((line >> resolutionDelta) * tilePitch);
            for (uint32_t pixel = 0; pixel < 256; ++pixel)
                {
                memcpy(pOutLine + pixel * 3, pSrcLine + (pixel >> resolutionDelta) * 3, 3);
                }
            }
        }
    else // bilinear
        {
        BeAssert(srcOffsetX <= 256 && srcOffsetY <= 256);

        size_t xMax = (256 - srcOffsetX) - 1;
        size_t yMax = (256 - srcOffsetY) - 1;

        for (size_t line = 0; line < 256; ++line)
            {
            Byte* pOutLine = pOutput + line * tilePitch;

            float srcY = (line + 0.5f) / (1 << resolutionDelta);
            uint32_t   srcPixelY = (uint32_t) srcY;

            Byte const* pSrcLine0 = pSrc + (srcPixelY * tilePitch);
            Byte const* pSrcLine1 = pSrc + (MIN(srcPixelY + 1, yMax) * tilePitch);

            float fy = srcY - srcPixelY;
            float fy1 = 1.0f - fy;

            for (size_t pixel = 0; pixel < 256; ++pixel)
                {
                float srcX = (pixel + 0.5f) / (1 << resolutionDelta);
                uint32_t   srcPixelX = (uint32_t) srcX;

                // Calculate the weights for each pixel  
                float fx = srcX - srcPixelX;
                float fx1 = 1.0f - fx;
                float w1 = fx1 * fy1;
                float w2 = fx  * fy1;
                float w3 = fx1 * fy;
                float w4 = fx  * fy;

                for (size_t channel = 0; channel < 3; ++channel)
                    {
                    pOutLine[pixel * 3 + channel] = (Byte) (pSrcLine0[srcPixelX * 3 + channel] * w1 +
                                                            pSrcLine0[MIN(srcPixelX + 1, xMax) * 3 + channel] * w2 +
                                                            pSrcLine1[srcPixelX * 3 + channel] * w3 +
                                                            pSrcLine1[MIN(srcPixelX + 1, xMax) * 3 + channel] * w4);
                    }
                }
            }
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2016
//----------------------------------------------------------------------------------------
bool HRFVirtualEarthEditor::QueryTileSubstitute(Byte* pOutput, uint64_t blockPosX, uint64_t blockPosY)
    {
    uint64_t subPixelPosX = blockPosX;
    uint64_t subPixelPosY = blockPosY;

    if (m_pSubTileData.get() == nullptr)
        m_pSubTileData.reset(new Byte[GetResolutionDescriptor()->GetBlockSizeInBytes()]);

    HRFVirtualEarthFile& rasterFile = static_cast<HRFVirtualEarthFile&>(*GetRasterFile());

    ReadTileStatus rasterStatus = ReadTileStatus::Error;

    for (uint16_t subResolution = GetResolutionIndex() + 1; subResolution < rasterFile.GetPageDescriptor(m_Page)->CountResolutions(); ++subResolution)
        {
        HRFVirtualEarthEditor* pSubResolutionEditor = (HRFVirtualEarthEditor*) rasterFile.GetResolutionEditor(subResolution);
        if (nullptr == pSubResolutionEditor)
            break;

        // Compute position in next sub-res.
        subPixelPosX = subPixelPosX >> 1;
        subPixelPosY = subPixelPosY >> 1;

        // snap to sub-res tile boundary
        uint64_t subTilePosX = subPixelPosX / 256;
        uint64_t subTilePosY = subPixelPosY / 256;

        uint64_t subTile_PixelPosX = subTilePosX * 256;
        uint64_t subTile_PixelPosY = subTilePosY * 256;

        rasterStatus = pSubResolutionEditor->QueryTile(m_pSubTileData.get(), subTile_PixelPosX, subTile_PixelPosY);
        if (ReadTileStatus::Success == rasterStatus)
            {
            size_t srcOffsetX = (size_t)(subPixelPosX - subTile_PixelPosX);
            size_t srcOffsetY = (size_t)(subPixelPosY - subTile_PixelPosY);
            MagnifyTile(pOutput, m_pSubTileData.get(), srcOffsetX, srcOffsetY, pSubResolutionEditor->GetResolutionIndex());
            break;  // we found a valid tile.
            }
        }

    return ReadTileStatus::Success == rasterStatus ? true : false;
    }

//-----------------------------------------------------------------------------
// Write a block of data
//-----------------------------------------------------------------------------
HSTATUS HRFVirtualEarthEditor::WriteBlock(uint64_t pi_PosBlockX,
                                          uint64_t pi_PosBlockY,
                                          const Byte* pi_pData)
    {
    //Virtual Earth is supported only in read only mode.
    HASSERT(false);
    return H_ERROR;
    }

//-----------------------------------------------------------------------------
// Write a block of data
//-----------------------------------------------------------------------------
HSTATUS HRFVirtualEarthEditor::WriteBlock(uint64_t pi_PosBlockX,
                                          uint64_t pi_PosBlockY,
                                          const HFCPtr<HCDPacket>&    pi_rpPacket)
    {
    //Virtual Earth is supported only in read only mode.
    HASSERT(false);
    return H_ERROR;
    }

//-----------------------------------------------------------------------------
// Protected
// Request a look ahead for this resolution editor
//-----------------------------------------------------------------------------
void HRFVirtualEarthEditor::RequestLookAhead(const HGFTileIDList& pi_rTileIDList, uint32_t pi_ConsumerID)
    {
    HPRECONDITION(!pi_rTileIDList.empty());

    std::map<uint64_t, RefCountedPtr<VirtualEarthTileQuery>> newTileQuery;

    HRFVirtualEarthFile& rasterFile = static_cast<HRFVirtualEarthFile&>(*GetRasterFile());

    WorkerPool& pool = rasterFile.GetWorkerPool();

    for(auto tileId : pi_rTileIDList)
        {
        rasterFile.m_tileQueryMapMutex.lock();
        auto tileQueryItr = rasterFile.m_tileQueryMap.find(tileId);
        bool found = (tileQueryItr != rasterFile.m_tileQueryMap.end());
        rasterFile.m_tileQueryMapMutex.unlock();

        if(found)
            {
            // Reuse existing query.
            
            if (pi_ConsumerID < BINGMAPS_MULTIPLE_SETLOOKAHEAD_MIN_CONSUMER_ID)
                newTileQuery.insert(*tileQueryItr);
            
            // Not sure about that?? should we notify again? tile may or may not be ready at this point.
            //GetRasterFile()->NotifyBlockReady(GetPage(), TileItr->first);
            }
        else
            {
            // Tile was not in lookAHead, create a new request
            RefCountedPtr<VirtualEarthTileQuery> pTileQuery = new VirtualEarthTileQuery(tileId, BuildTileUri(tileId), rasterFile);

            if (pi_ConsumerID < BINGMAPS_MULTIPLE_SETLOOKAHEAD_MIN_CONSUMER_ID)
                { 
                newTileQuery.insert({ tileId, pTileQuery });
                }
            else
                {
                rasterFile.m_tileQueryMapMutex.lock();
                rasterFile.m_tileQueryMap.insert({ tileId, pTileQuery });
                rasterFile.m_tileQueryMapMutex.unlock();
                }
            
            pool.Enqueue(*pTileQuery);
            }        
        }

    if (pi_ConsumerID < BINGMAPS_MULTIPLE_SETLOOKAHEAD_MIN_CONSUMER_ID)
        {
        rasterFile.m_tileQueryMap = std::move(newTileQuery);       // Replace with the new queries, old ones will be canceled.
        }
    }


//-----------------------------------------------------------------------------
// Translate resolution index to Virtual earth level of detail.
//-----------------------------------------------------------------------------
int32_t HRFVirtualEarthEditor::GetLevelOfDetail() const
    {
    return m_pRasterFile->GetPageDescriptor(m_Page)->CountResolutions() - GetResolutionIndex();
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  1/2016
//----------------------------------------------------------------------------------------
Utf8String HRFVirtualEarthEditor::BuildTileUri(uint64_t tileId)
    {
    HRFVirtualEarthFile& rasterFile = static_cast<HRFVirtualEarthFile&>(*GetRasterFile());

    uint64_t tilePosX, tilePosY;
    m_TileIDDescriptor.GetPositionFromID(tileId, &tilePosX, &tilePosY);
    Utf8String tileUri(rasterFile.GetTileURI((int32_t)tilePosX, (int32_t)tilePosY, GetLevelOfDetail()));
    return tileUri;
    }


//-----------------------------------------------------------------------------//
//                         Extern - VirtualEarthTileSystem API                 //
// This is an extern API that has been put here instead of in the extern       //
// library because it was more practical.                                      //
//-----------------------------------------------------------------------------//
const double HRFVirtualEarthEditor::VirtualEarthTileSystem::EarthRadius = 6378137;
const double HRFVirtualEarthEditor::VirtualEarthTileSystem::MinLatitude = -85.05112878;
const double HRFVirtualEarthEditor::VirtualEarthTileSystem::MaxLatitude = 85.05112878;
const double HRFVirtualEarthEditor::VirtualEarthTileSystem::MinLongitude = -180;
const double HRFVirtualEarthEditor::VirtualEarthTileSystem::MaxLongitude = 180;

/// <summary>
/// Clips a number to the specified minimum and maximum values.
/// </summary>
/// <param name="n">The number to clip.</param>
/// <param name="minValue">Minimum allowable value.</param>
/// <param name="maxValue">Maximum allowable value.</param>
/// <returns>The clipped value.</returns>
double HRFVirtualEarthEditor::VirtualEarthTileSystem::Clip(double n, double minValue, double maxValue)
    {
    return MIN(MAX(n, minValue), maxValue);
    }

/// <summary>
/// Determines the map width and height (in pixels) at a specified level
/// of detail.
/// </summary>
/// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)
/// to 23 (highest detail).</param>
/// <returns>The map width and height in pixels.</returns>
uint32_t HRFVirtualEarthEditor::VirtualEarthTileSystem::MapSize(int32_t levelOfDetail)
    {
    return (uint32_t) 256 << levelOfDetail;
    }

/// <summary>
/// Determines the ground resolution (in meters per pixel) at a specified
/// latitude and level of detail.
/// </summary>
/// <param name="latitude">Latitude (in degrees) at which to measure the
/// ground resolution.</param>
/// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)
/// to 23 (highest detail).</param>
/// <returns>The ground resolution, in meters per pixel.</returns>
double HRFVirtualEarthEditor::VirtualEarthTileSystem::GroundResolution(double latitude, int32_t levelOfDetail)
    {
    latitude = Clip(latitude, MinLatitude, MaxLatitude);
    return cos(latitude * PI / 180) * 2 * PI * EarthRadius /
           MapSize(levelOfDetail);
    }

/// <summary>
/// Determines the map scale at a specified latitude, level of detail,
/// and screen resolution.
/// </summary>
/// <param name="latitude">Latitude (in degrees) at which to measure the
/// map scale.</param>
/// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)
/// to 23 (highest detail).</param>
/// <param name="screenDpi">Resolution of the screen, in dots per inch.</param>
/// <returns>The map scale, expressed as the denominator N of the ratio 1 : N.</returns>
double HRFVirtualEarthEditor::VirtualEarthTileSystem::MapScale(double latitude, int32_t levelOfDetail, int32_t screenDpi)
    {
    return GroundResolution(latitude, levelOfDetail) * screenDpi / 0.0254;
    }

/// <summary>
/// Converts a point from latitude/longitude WGS-84 coordinates (in degrees)
/// into pixel XY coordinates at a specified level of detail.
/// </summary>
/// <param name="latitude">Latitude of the point, in degrees.</param>
/// <param name="longitude">Longitude of the point, in degrees.</param>
/// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)
/// to 23 (highest detail).</param>
/// <param name="pixelX">Output parameter receiving the X coordinate in pixels.</param>
/// <param name="pixelY">Output parameter receiving the Y coordinate in pixels.</param>
void HRFVirtualEarthEditor::VirtualEarthTileSystem::LatLongToPixelXY(double latitude, double longitude, int32_t levelOfDetail,
                                                                     int32_t* pixelX, int32_t* pixelY)
    {
    latitude = Clip(latitude, MinLatitude, MaxLatitude);
    longitude = Clip(longitude, MinLongitude, MaxLongitude);

    double x = (longitude + 180) / 360;
    double sinLatitude = sin(latitude * PI / 180);
    double y = 0.5 - log((1 + sinLatitude) / (1 - sinLatitude)) / (4 * PI);

    uint32_t mapSize = MapSize(levelOfDetail);
    *pixelX = (int32_t) Clip(x * mapSize + 0.5, 0, mapSize - 1);
    *pixelY = (int32_t) Clip(y * mapSize + 0.5, 0, mapSize - 1);
    }

/// <summary>
/// Converts a pixel from pixel XY coordinates at a specified level of detail
/// into latitude/longitude WGS-84 coordinates (in degrees).
/// </summary>
/// <param name="pixelX">X coordinate of the point, in pixels.</param>
/// <param name="pixelY">Y coordinates of the point, in pixels.</param>
/// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)
/// to 23 (highest detail).</param>
/// <param name="latitude">Output parameter receiving the latitude in degrees.</param>
/// <param name="longitude">Output parameter receiving the longitude in degrees.</param>
void HRFVirtualEarthEditor::VirtualEarthTileSystem::PixelXYToLatLong(int32_t pixelX, int32_t pixelY, int32_t levelOfDetail,
                                                                     double* latitude, double* longitude)
    {
    double mapSize = MapSize(levelOfDetail);
    double x = (Clip(pixelX, 0, mapSize - 1) / mapSize) - 0.5;
    double y = 0.5 - (Clip(pixelY, 0, mapSize - 1) / mapSize);

    *latitude = 90 - 360 * atan(exp(-y * 2 * PI)) / PI;
    *longitude = 360 * x;
    }

/// <summary>
/// Converts pixel XY coordinates into tile XY coordinates of the tile containing
/// the specified pixel.
/// </summary>
/// <param name="pixelX">Pixel X coordinate.</param>
/// <param name="pixelY">Pixel Y coordinate.</param>
/// <param name="tileX">Output parameter receiving the tile X coordinate.</param>
/// <param name="tileY">Output parameter receiving the tile Y coordinate.</param>
void HRFVirtualEarthEditor::VirtualEarthTileSystem::PixelXYToTileXY(int32_t pixelX, int32_t pixelY,
                                                                    int32_t* tileX, int32_t* tileY)
    {
    *tileX = pixelX / 256;
    *tileY = pixelY / 256;
    }

/// <summary>
/// Converts tile XY coordinates into pixel XY coordinates of the upper-left pixel
/// of the specified tile.
/// </summary>
/// <param name="tileX">Tile X coordinate.</param>
/// <param name="tileY">Tile Y coordinate.</param>
/// <param name="pixelX">Output parameter receiving the pixel X coordinate.</param>
/// <param name="pixelY">Output parameter receiving the pixel Y coordinate.</param>
void HRFVirtualEarthEditor::VirtualEarthTileSystem::TileXYToPixelXY(int32_t tileX, int32_t tileY,
                                                                    int32_t* pixelX, int32_t* pixelY)
    {
    *pixelX = tileX * 256;
    *pixelY = tileY * 256;
    }

/// <summary>
/// Converts tile XY coordinates into a QuadKey at a specified level of detail.
/// </summary>
/// <param name="tileX">Tile X coordinate.</param>
/// <param name="tileY">Tile Y coordinate.</param>
/// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)
/// to 23 (highest detail).</param>
/// <returns>A string containing the QuadKey.</returns>
string HRFVirtualEarthEditor::VirtualEarthTileSystem::TileXYToQuadKey(int32_t tileX, int32_t tileY, int32_t levelOfDetail)
    {
    string quadKey;
    for (int32_t i = levelOfDetail; i > 0; i--)
        {
        char digit = '0';
        int32_t mask = 1 << (i - 1);
        if ((tileX & mask) != 0)
            {
            digit++;
            }
        if ((tileY & mask) != 0)
            {
            digit++;
            digit++;
            }
        quadKey += digit;
        }
    return quadKey;
    }

/// <summary>
/// Converts pixel XY coordinates into a QuadKey at a specified level of detail.
/// </summary>
/// <param name="pixelX">Pixel X coordinate.</param>
/// <param name="pixelY">Pixel Y coordinate.</param>
/// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)
/// to 23 (highest detail).</param>
/// <returns>A string containing the QuadKey.</returns>
string HRFVirtualEarthEditor::VirtualEarthTileSystem::PixelXYToQuadKey(int32_t pi_PixelX, int32_t pi_PixelY, int32_t pi_LevelOfDetail)
    {
    string quadKey;

    pi_PixelX /= 256;
    pi_PixelY /= 256;

    for (int32_t i = pi_LevelOfDetail; i > 0; i--)
        {
        char digit = '0';
        int32_t mask = 1 << (i - 1);
        if ((pi_PixelX & mask) != 0)
            {
            digit++;
            }
        if ((pi_PixelY & mask) != 0)
            {
            digit++;
            digit++;
            }
        quadKey += digit;
        }
    return quadKey;
    }

/// <summary>
/// Converts a QuadKey into tile XY coordinates.
/// </summary>
/// <param name="quadKey">QuadKey of the tile.</param>
/// <param name="tileX">Output parameter receiving the tile X coordinate.</param>
/// <param name="tileY">Output parameter receiving the tile Y coordinate.</param>
/// <param name="levelOfDetail">Output parameter receiving the level of detail.</param>
void HRFVirtualEarthEditor::VirtualEarthTileSystem::QuadKeyToTileXY(string quadKey,
                                                                    int32_t* tileX, int32_t* tileY, int32_t* levelOfDetail)
    {
    *tileX = *tileY = 0;
    *levelOfDetail = (int32_t)quadKey.size();
    for (int32_t i = *levelOfDetail; i > 0; i--)
        {
        int32_t mask = 1 << (i - 1);
        switch (quadKey[*levelOfDetail - i])
            {
            case '0':
                break;

            case '1':
                *tileX |= mask;
                break;

            case '2':
                *tileY |= mask;
                break;

            case '3':
                *tileX |= mask;
                *tileY |= mask;
                break;

            default:
                break;
            }
        }
    }

