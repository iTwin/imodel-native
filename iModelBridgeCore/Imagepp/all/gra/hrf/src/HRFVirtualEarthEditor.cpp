//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFVirtualEarthEditor.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFVirtualEarthEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include "HRFVirtualEarthEditor.h"
#include <Imagepp/all/h/HRFVirtualEarthFile.h>
#include <Imagepp/all/h/HRPPixelConverter.h>
#include <Imagepp/all/h/HFCExceptionHandler.h>
#include <Imagepp/all/h/HFCURLMemFile.h>
#include <Imagepp/all/h/HRFJpegFile.h>
#include <Imagepp/all/h/HRFPngFile.h>

#include <ImagePPInternal/gra/Task.h>

#define RASTER_FILE     static_cast<HRFVirtualEarthFile*>(GetRasterFile().GetPtr())

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  1/2016
//----------------------------------------------------------------------------------------
void VirtualEarthTileQuery::_Run()
    {
    HSTATUS Status = H_SUCCESS;
    
    WString ServerURL = m_rasterFile.GetTileURI((int)m_tileRequest.m_PosX, (int)m_tileRequest.m_PosY, m_tileRequest.m_LevelOfDetail);

    WString::size_type RequestPos = ServerURL.find('?');
    WString            Request    = ServerURL.substr(RequestPos + 1);

    ServerURL.erase(RequestPos);

    HFCPtr<HRFVirtualEarthConnection> pConnection = new HRFVirtualEarthConnection(ServerURL);

    if (!pConnection->Connect(pConnection->GetUserName(), pConnection->GetPassword()))
        {
        Status = H_ERROR;
        }
    else
        {
        pConnection->NewRequest();

        try
            {
            LangCodePage codePage;
            BeStringUtilities::GetCurrentCodePage (codePage);

            AString requestA;
            BeStringUtilities::WCharToLocaleChar (requestA, codePage, Request.c_str());

            //&&Backlog should have a unicode version of pConnection->Send

            //Send the command
            pConnection->Send((Byte const*)requestA.c_str(), requestA.size());

            size_t            DataAvailable;
            HFCPtr<HFCBuffer> pBuffer(new HFCBuffer(1, 1));

            //Read the response
            while (!pConnection->RequestEnded())
                {
                //Wait for data to arrive
                if ((DataAvailable = pConnection->WaitDataAvailable(pConnection->GetTimeOut())) > 0)
                    {
                    //Read it
                    pConnection->Receive(pBuffer->PrepareForNewData(DataAvailable), (int32_t)DataAvailable);
                    pBuffer->SetNewDataSize(DataAvailable);
                    }
                }

            pConnection->Disconnect(); //Release the connection

            //Create a memory JPEG file with the server response
            HFCPtr<HFCURL>        pURL(new HFCURLMemFile(L"memory://mem.file", pBuffer));
            HFCPtr<HRFRasterFile> pFile;

            //Creators are singletons and are not thread safe
            HFCMonitor HRFMonitor(m_rasterFile.m_HRFKey);       //&&MM this is wrong! the key is on virtual earth file instance and
                                                                // will not prevent other instances to access the singletons.
                                                                // Moreover jpeg and png lib are thread safe so we should find
                                                                // a way to take advantage of that instead of locking.

            //&&MM we should read the type of file from the http header.
            if (HRFJpegCreator::GetInstance()->IsKindOfFile(pURL))
                {
                pFile = new HRFJpegFile(pURL, HFC_READ_ONLY);
                }
            else if (HRFPngCreator::GetInstance()->IsKindOfFile(pURL) == true)
                {
                pFile = new HRFPngFile(pURL, HFC_READ_ONLY);
                }
            else
                {
                HASSERT(0);
                }
            HRFMonitor.ReleaseKey();

            HAutoPtr<HRFResolutionEditor> pEditor(pFile->CreateResolutionEditor(0, (unsigned short)0, HFC_READ_ONLY));

            m_dataSize = m_tileRequest.m_BlockSizeInBytes;
            m_data.reset(new Byte[m_dataSize]);
            
            if (!pEditor->GetResolutionDescriptor()->GetPixelType()->HasSamePixelInterpretation(*m_tileRequest.m_PixelType))
                {
                //Need to convert the data
                HFCPtr<HRPPixelConverter> pConverter;
                HAutoPtr<Byte>           pReadBuffer;
                pConverter = pEditor->GetResolutionDescriptor()->GetPixelType()->GetConverterTo(m_tileRequest.m_PixelType);

                pReadBuffer = new Byte[pEditor->GetResolutionDescriptor()->GetBlockSizeInBytes()];

                if (pEditor->GetResolutionDescriptor()->GetBlockHeight() == 1)
                    {
                    Byte* pOutput = m_data.get();
                    size_t LineSizeInBytes = m_tileRequest.m_BytesPerBlockWidth;

                    HFCMonitor HRFMonitor(m_rasterFile.m_HRFKey);
                    for (uint32_t i = 0; (i < m_tileRequest.m_BlockHeight) && (Status == H_SUCCESS); i++)
                        {
                        Status = pEditor->ReadBlock(0, i, pReadBuffer);
                        pConverter->Convert(pReadBuffer, pOutput, m_tileRequest.m_BlockWidth);
                        pOutput += LineSizeInBytes;
                        }
                    }
                else if (pEditor->GetResolutionDescriptor()->GetBlockHeight() == m_tileRequest.m_BlockWidth)
                    {
                    HFCMonitor HRFMonitor(m_rasterFile.m_HRFKey);
                    Status = pEditor->ReadBlock((uint64_t)0, (uint64_t)0, pReadBuffer);
                    pConverter->Convert(pReadBuffer, m_data.get(), m_tileRequest.m_BlockWidth * m_tileRequest.m_BlockHeight);
                    }
                else
                    {
                    //Not supported
                    HASSERT(0);
                    Status = H_ERROR;
                    }
                }
            else
                {
                //Can read directly into the output buffer
                if (pEditor->GetResolutionDescriptor()->GetBlockHeight() == 1)
                    {
                    Byte* pOutput = m_data.get();
                    size_t LineSizeInBytes = m_tileRequest.m_BytesPerBlockWidth;

                    for (uint32_t i = 0; i < m_tileRequest.m_BlockHeight && Status == H_SUCCESS; i++)
                        {
                        Status = pEditor->ReadBlock(0, i, pOutput);
                        pOutput += LineSizeInBytes;
                        }
                    }
                else if (pEditor->GetResolutionDescriptor()->GetBlockHeight() == m_tileRequest.m_BlockHeight)
                    {
                    Status = pEditor->ReadBlock((uint64_t)0, (uint64_t)0, m_data.get());
                    }
                else
                    {
                    //Not supported
                    HASSERT(0);
                    Status = H_ERROR;
                    }
                }

            m_rasterFile.NotifyBlockReady(m_tileRequest.m_Page, m_tileRequest.m_TileID);            
            }
        catch(...)
            {
            m_rasterFile.NotifyBlockReady(m_tileRequest.m_Page, m_tileRequest.m_TileID);

            Status = H_ERROR;
            }
        }
    }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HRFVirtualEarthEditor::HRFVirtualEarthEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                             uint32_t              pi_Page,
                                             unsigned short       pi_Resolution,
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
HSTATUS HRFVirtualEarthEditor::ReadBlock(uint64_t             pi_PosBlockX,
                                         uint64_t             pi_PosBlockY,
                                         Byte*                po_pData)
    {
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(m_pResolutionDescriptor->GetBlockType() == HRFBlockType::TILE);
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION(po_pData != 0);

    HSTATUS Status = H_ERROR;

    // check if the tile is already in the pool
    uint64_t TileID = m_TileIDDescriptor.ComputeID(pi_PosBlockX, pi_PosBlockY, m_Resolution);

    RefCountedPtr<VirtualEarthTileQuery> pTileQuery;

    auto tileQueryItr = RASTER_FILE->m_tileQueryMap.find(TileID);
    if(tileQueryItr == RASTER_FILE->m_tileQueryMap.end())
        {
#ifdef _RETURN_RED_TILES_IF_NOT_IN_LOOKAHEAD    // Debug purpose only
        memset(po_pData, 0, GetResolutionDescriptor()->GetBlockSizeInBytes()); 

        // Mark all non lookahead tiles in red.
        for(uint64_t i=0; i < GetResolutionDescriptor()->GetBlockWidth()*GetResolutionDescriptor()->GetBlockHeight(); ++i)
            po_pData[i*3] = 255;
        
        return H_SUCCESS;
#else
        HRFVirtualEarthTileRequest Request;
        InitTileRequest(TileID, Request);

        // Tile was not in lookAHead request.
        pTileQuery = new VirtualEarthTileQuery(Request, *RASTER_FILE);

        WorkerPool& pool = RASTER_FILE->GetWorkerPool();
        pool.Enqueue(*pTileQuery, true/*atFront*/); 
#endif    
        }
    else
        {
        pTileQuery = tileQueryItr->second;
        }

    pTileQuery->Wait();

    if (pTileQuery->m_dataSize > 0)
        {
        memcpy(po_pData, pTileQuery->m_data.get(), pTileQuery->m_dataSize);
        Status = H_SUCCESS;
        }
    else
        {
        // If the connection failed or whatever...
        // Clear output and return SUCCESS anyway because HRFObjectStore will add an empty tile to the pool and 
        // every CopyFrom will failed to copy pixels which will leave the destination with uninitialized pixels.
        // Lets hope we can improve this behavior in the future. e.g. do not add it to the pool and try again next time.
        memset(po_pData, 0, GetResolutionDescriptor()->GetBlockSizeInBytes()); 

        Status = H_SUCCESS;
        }

    return Status;
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
void HRFVirtualEarthEditor::RequestLookAhead(const HGFTileIDList& pi_rTileIDList)
    {
    HPRECONDITION(!pi_rTileIDList.empty());

    std::map<uint64_t, RefCountedPtr<VirtualEarthTileQuery>> newTileQuery;

    WorkerPool& pool = RASTER_FILE->GetWorkerPool();

    for(auto tileId : pi_rTileIDList)
        {
        auto tileQueryItr = RASTER_FILE->m_tileQueryMap.find(tileId);
        if(tileQueryItr != RASTER_FILE->m_tileQueryMap.end())
            {
            // Reuse existing query.
            newTileQuery.insert(*tileQueryItr);
            
            //&&MM Not sure about that?? tile may or may not be ready
            //GetRasterFile()->NotifyBlockReady(GetPage(), TileItr->first);
            }
        else
            {
            // Tile was not in lookAHead, create a new request

            HRFVirtualEarthTileRequest Request;
            InitTileRequest(tileId, Request);

            RefCountedPtr<VirtualEarthTileQuery> pTileQuery = new VirtualEarthTileQuery(Request, *RASTER_FILE);
            newTileQuery.insert({tileId, pTileQuery});
            pool.Enqueue(*pTileQuery);
            }        
        }

    RASTER_FILE->m_tileQueryMap = std::move(newTileQuery);       // Replace with the new queries, old ones will be canceled.
    }


//-----------------------------------------------------------------------------
// Translate resolution index to Virtual earth level of detail.
//-----------------------------------------------------------------------------
int HRFVirtualEarthEditor::GetLevelOfDetail() const
    {
    return m_pRasterFile->GetPageDescriptor(m_Page)->CountResolutions() - GetResolutionIndex();
    }

//-----------------------------------------------------------------------------
// Initialize a tile request struct
//-----------------------------------------------------------------------------
void HRFVirtualEarthEditor::InitTileRequest(uint64_t TileId, HRFVirtualEarthTileRequest& TileRequest)
{
    TileRequest.m_TileID = TileId;
    m_TileIDDescriptor.GetPositionFromID(TileId, &TileRequest.m_PosX, &TileRequest.m_PosY);
    TileRequest.m_LevelOfDetail = GetLevelOfDetail();
    TileRequest.m_BlockHeight = GetResolutionDescriptor()->GetBlockHeight();
    TileRequest.m_BlockWidth = GetResolutionDescriptor()->GetBlockWidth();
    TileRequest.m_BlockSizeInBytes = GetResolutionDescriptor()->GetBlockSizeInBytes();
    TileRequest.m_BytesPerBlockWidth = GetResolutionDescriptor()->GetBytesPerBlockWidth();
    TileRequest.m_Page = GetPage();
    TileRequest.m_PixelType = GetResolutionDescriptor()->GetPixelType();
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
unsigned int HRFVirtualEarthEditor::VirtualEarthTileSystem::MapSize(int levelOfDetail)
    {
    return (unsigned int) 256 << levelOfDetail;
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
double HRFVirtualEarthEditor::VirtualEarthTileSystem::GroundResolution(double latitude, int levelOfDetail)
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
double HRFVirtualEarthEditor::VirtualEarthTileSystem::MapScale(double latitude, int levelOfDetail, int screenDpi)
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
void HRFVirtualEarthEditor::VirtualEarthTileSystem::LatLongToPixelXY(double latitude, double longitude, int levelOfDetail,
                                                                     int* pixelX, int* pixelY)
    {
    latitude = Clip(latitude, MinLatitude, MaxLatitude);
    longitude = Clip(longitude, MinLongitude, MaxLongitude);

    double x = (longitude + 180) / 360;
    double sinLatitude = sin(latitude * PI / 180);
    double y = 0.5 - log((1 + sinLatitude) / (1 - sinLatitude)) / (4 * PI);

    unsigned int mapSize = MapSize(levelOfDetail);
    *pixelX = (int) Clip(x * mapSize + 0.5, 0, mapSize - 1);
    *pixelY = (int) Clip(y * mapSize + 0.5, 0, mapSize - 1);
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
void HRFVirtualEarthEditor::VirtualEarthTileSystem::PixelXYToLatLong(int pixelX, int pixelY, int levelOfDetail,
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
void HRFVirtualEarthEditor::VirtualEarthTileSystem::PixelXYToTileXY(int pixelX, int pixelY,
                                                                    int* tileX, int* tileY)
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
void HRFVirtualEarthEditor::VirtualEarthTileSystem::TileXYToPixelXY(int tileX, int tileY,
                                                                    int* pixelX, int* pixelY)
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
string HRFVirtualEarthEditor::VirtualEarthTileSystem::TileXYToQuadKey(int tileX, int tileY, int levelOfDetail)
    {
    string quadKey;
    for (int i = levelOfDetail; i > 0; i--)
        {
        char digit = '0';
        int mask = 1 << (i - 1);
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
string HRFVirtualEarthEditor::VirtualEarthTileSystem::PixelXYToQuadKey(int pi_PixelX, int pi_PixelY, int pi_LevelOfDetail)
    {
    string quadKey;

    pi_PixelX /= 256;
    pi_PixelY /= 256;

    for (int i = pi_LevelOfDetail; i > 0; i--)
        {
        char digit = '0';
        int mask = 1 << (i - 1);
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
                                                                    int* tileX, int* tileY, int* levelOfDetail)
    {
    *tileX = *tileY = 0;
    *levelOfDetail = (int)quadKey.size();
    for (int i = *levelOfDetail; i > 0; i--)
        {
        int mask = 1 << (i - 1);
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

