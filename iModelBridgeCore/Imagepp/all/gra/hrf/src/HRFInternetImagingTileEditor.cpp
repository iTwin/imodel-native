//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetImagingTileEditor.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetImagingTileEditor
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HRFInternetImagingTileEditor.h>
#include <Imagepp/all/h/HRFInternetImagingFile.h>
#include <Imagepp/all/h/HRFInternetImagingThread.h>
#include <Imagepp/all/h/HRFInternetImagingException.h>
#include <Imagepp/all/h/HRFInternetImaging.h>

// Codec Objects
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecZlib.h>
#include <Imagepp/all/h/HCDCodecHMRPackBits.h>
#include <Imagepp/all/h/HCDCodecFPXSingleColor.h>
#include <Imagepp/all/h/HCDCodecFlashpix.h>
#include <Imagepp/all/h/HCDCodecFlashpixOLDForMSI10.h>
#include <Imagepp/all/h/HCDCodecHMRCCITT.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>
#include <Imagepp/all/h/HCDCodecJpegAlpha.h>
#include <Imagepp/all/h/HCDPacket.h>
#ifdef __HMR_SUMMUS_SUPPORTED
//#include "HCDCodecSummus.h"
#endif


#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV16PRGray8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV24PhotoYCC.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRPPixelTypeV32PRPhotoYCCA8.h>
#include <Imagepp/all/h/HRPPixelTypeV32PR8PG8PB8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV8GrayWhite8.h>

#include <Imagepp/all/h/HFCThread.h>

#include <Imagepp/all/h/HFCResourceLoader.h>
#include <Imagepp/all/h/ImagePPMessages.xliff.h>


//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

static const HFCVersion s_HIP0100(WString(L""),
                                  WString(L""),
                                  3,
                                  1, 0, HRFInternetImagingFile::s_IIP_Protocol);

static const HFCVersion s_HIP0140(WString(L""),
                                  WString(L""),
                                  3,
                                  1, 4, HRFInternetImagingFile::s_HIP_Protocol);


//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
HRFInternetImagingTileEditor::HRFInternetImagingTileEditor(const HFCPtr<HRFRasterFile>& pi_rpRasterFile,
                                                           uint32_t                    pi_Page,
                                                           unsigned short               pi_ResIndex,
                                                           HFCAccessMode                pi_AccessMode,
                                                           bool                         pi_WaitTileOnRead)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_ResIndex,
                          pi_AccessMode),
    m_TileDescriptor(m_pResolutionDescriptor->GetWidth(),
                     m_pResolutionDescriptor->GetHeight(),
                     m_pResolutionDescriptor->GetBlockWidth(),
                     m_pResolutionDescriptor->GetBlockHeight()),
    m_WaitTileOnRead(pi_WaitTileOnRead)
    {
    HPRECONDITION(GetRasterFile()->GetClassID() == HRFInternetImagingFile::CLASS_ID);

    m_pPageFile = static_cast<HRFInternetImagingFile*>(pi_rpRasterFile->GetPageFile(pi_Page).GetPtr());

    // we need to compute a tile descriptor with the size of the resolution

    // Compute the size of a tile
    m_TileSize =  m_pResolutionDescriptor->GetPixelType()->CountPixelRawDataBits();
    m_TileSize *= m_pResolutionDescriptor->GetBlockWidth();
    m_TileSize =  m_TileSize/8 + (m_TileSize%8 ? 1 : 0);
    m_TileSize *= m_pResolutionDescriptor->GetBlockHeight();
    }

//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
HRFInternetImagingTileEditor::HRFInternetImagingTileEditor(const HFCPtr<HRFRasterFile>& pi_rpRasterFile,
                                                           uint32_t                    pi_Page,
                                                           double                       pi_Resolution,
                                                           HFCAccessMode                pi_AccessMode,
                                                           bool                         pi_WaitTileOnRead)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode),
    m_WaitTileOnRead(pi_WaitTileOnRead)
    {
    HPRECONDITION(GetRasterFile()->GetClassID() == HRFInternetImagingFile::CLASS_ID);
    HPRECONDITION(pi_rpRasterFile->GetPageDescriptor(pi_Page)->IsUnlimitedResolution());

    m_pPageFile = static_cast<HRFInternetImagingFile*>(pi_rpRasterFile->GetPageFile(pi_Page).GetPtr());
    
    // first, check if we already have this resolution

    char Res[64];
    sprintf(Res, "%.16lf", m_ResolutionFactor);
    m_StrResFactor = Res;

    // no entries found in the resolution map, create a new entries
    list<uint32_t>::iterator Itr(m_pPageFile->m_UsedResIndexList.begin());
    HPRECONDITION(m_pPageFile->m_UsedResIndexList.size() > 0);
    HPRECONDITION(m_pPageFile->m_UsedResIndexList.front() == 0);
    // Use HRFResolutionEditor.m_Resolution to set the resolution index used by the editor
    // (with unlimited resolution, HRFResolutionEditor.m_Resolution is not use)
    m_Resolution = 0;
    for (unsigned short i = 0; i < HRF_INTERNET_MAXIMUM_RESOLUTION && m_Resolution == 0; i++)
        {
        if (Itr == m_pPageFile->m_UsedResIndexList.end())
            {
            m_Resolution = i;
            m_pPageFile->m_UsedResIndexList.push_back(m_Resolution);
            }
        else if (*Itr != i)
            {
            m_Resolution = i;
            m_pPageFile->m_UsedResIndexList.insert(Itr, m_Resolution);
            }
        else
            Itr++;
        }

    // maximun resolution is reached
    if (m_Resolution == 0)
        throw HRFInternetImagingException(HRFII_NO_RESOLUTION_INDEX_LEFT_EXCEPTION,
                                          pi_rpRasterFile->GetURL()->GetURL());

    // insert a new entries into the unlimited resolution map
    m_pPageFile->m_UnlimitedResolutionMap.insert(HRFInternetImagingFile::UnlimitedResolutionMap::value_type(m_StrResFactor, m_Resolution));

    // we need the image size and tile size for this resolution
    char Request[255];
    uint32_t RequestSize;

    RequestSize = sprintf(Request, "obj=hip-tile-size,%s,%d&obj=hip-image-size,%s,%d", m_StrResFactor.c_str(), pi_Page, m_StrResFactor.c_str(), pi_Page);

    // Send the request
    try
        {
        m_pPageFile->m_TileSizeHandlerEvent.Reset();
        m_pPageFile->m_ImageSizeHandlerEvent.Reset();
        m_pPageFile->m_pConnection->Send((const Byte*)Request, RequestSize);

        m_pPageFile->m_TileSizeHandlerEvent.WaitUntilSignaled();
        m_pPageFile->m_ImageSizeHandlerEvent.WaitUntilSignaled();
        m_TileDescriptor = HGFTileIDDescriptor(m_pPageFile->m_ImageWidth[m_Resolution],
                                               m_pPageFile->m_ImageHeight[m_Resolution],
                                               m_pPageFile->m_TileWidth[m_Resolution],
                                               m_pPageFile->m_TileHeight[m_Resolution]);

        // build a new page descriptor
        HFCPtr<HRFResolutionDescriptor> pMainResolutionDescriptor(m_pRasterFile->GetPageDescriptor(m_Page)->GetResolutionDescriptor(0));

        // change de resolution descriptor of the HRFRasterFileEditor
        m_pResolutionDescriptor   = new HRFResolutionDescriptor(pMainResolutionDescriptor->GetAccessMode(),
                                                                m_pRasterFile->GetCapabilities(),
                                                                pi_Resolution,
                                                                pi_Resolution,
                                                                pMainResolutionDescriptor->GetPixelType(),
                                                                pMainResolutionDescriptor->GetCodec(),
                                                                pMainResolutionDescriptor->GetReaderBlockAccess(),
                                                                pMainResolutionDescriptor->GetWriterBlockAccess(),
                                                                pMainResolutionDescriptor->GetScanlineOrientation(),
                                                                pMainResolutionDescriptor->GetInterleaveType(),
                                                                pMainResolutionDescriptor->IsInterlace(),
                                                                m_pPageFile->m_ImageWidth[m_Resolution],
                                                                m_pPageFile->m_ImageHeight[m_Resolution],
                                                                m_pPageFile->m_TileWidth[m_Resolution],
                                                                m_pPageFile->m_TileHeight[m_Resolution],
                                                                (pMainResolutionDescriptor->HasBlocksDataFlag() ?
                                                                 pMainResolutionDescriptor->GetBlocksDataFlag() : 0),
                                                                pMainResolutionDescriptor->GetBlockType(),
                                                                pMainResolutionDescriptor->GetNumberOfPass(),
                                                                pMainResolutionDescriptor->GetPaddingBits(),
                                                                pMainResolutionDescriptor->GetDownSamplingMethod());

        }
    catch(...)
        {
        throw;
        }

    // Compute the size of a tile
    m_TileSize =  m_pResolutionDescriptor->GetPixelType()->CountPixelRawDataBits();
    m_TileSize *= m_pResolutionDescriptor->GetBlockWidth();
    m_TileSize =  m_TileSize/8 + (m_TileSize%8 ? 1 : 0);
    m_TileSize *= m_pResolutionDescriptor->GetBlockHeight();

    }


//-----------------------------------------------------------------------------
// public
// Destructor
//
//-----------------------------------------------------------------------------
HRFInternetImagingTileEditor::~HRFInternetImagingTileEditor()
    {
    if (m_pPageFile->GetPageDescriptor(m_Page)->IsUnlimitedResolution())
        {
        // remove the entries into the unlimited resolution map
        HRFInternetImagingFile::UnlimitedResolutionMap::iterator Itr(m_pPageFile->m_UnlimitedResolutionMap.find(m_StrResFactor));
        HPOSTCONDITION(Itr != m_pPageFile->m_UnlimitedResolutionMap.end());

        m_pPageFile->m_UnlimitedResolutionMap.erase(Itr);

        HFCMonitor TilePoolMonitor(m_pPageFile->m_TilePool);

        const HRFTilePool::TileMap& rTilesMap(m_pPageFile->m_TilePool.GetTiles());
        HRFTilePool::TileMap::const_iterator TileMapItr(rTilesMap.begin());
        HGFTileIDList TileIDList;
        while (TileMapItr != rTilesMap.end())
            {
            if (m_TileDescriptor.GetLevel(TileMapItr->first) == m_Resolution)
                TileIDList.push_back(TileMapItr->first);
            TileMapItr++;
            }
        m_pPageFile->m_TilePool.InvalidateTiles(TileIDList, true);
        list<uint32_t>::iterator ResIndexItr(m_pPageFile->m_UsedResIndexList.begin());
        while (ResIndexItr != m_pPageFile->m_UsedResIndexList.end() && *ResIndexItr != m_Resolution)
            ResIndexItr++;

        HPOSTCONDITION(ResIndexItr != m_pPageFile->m_UsedResIndexList.end());
        m_pPageFile->m_UsedResIndexList.erase(ResIndexItr);
        }
    }


//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
HSTATUS HRFInternetImagingTileEditor::ReadBlock(uint32_t pi_PosTileX,
                                                uint32_t pi_PosTileY,
                                                Byte* po_pData,
                                                HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(po_pData != 0);
    HSTATUS Result = H_ERROR;
    uint64_t TileID = m_TileDescriptor.ComputeID(pi_PosTileX, pi_PosTileY, m_Resolution);

#ifdef TRACE_LOOKAHEAD
    WChar Msg[256];
    BeStringUtilities::Snwprintf(Msg, L"ReadBlock : %d\n", TileID);
    HDEBUGTEXT(Msg);
#endif

    // If disconnected, set error as IO_ERROR
    if (m_pPageFile->GetState() == HRFInternetImagingFile::DISCONNECTED)
        {
        Result = H_IOERROR;
        goto WRAPUP;
        }

    // Retry loop.  Retry until we successfully read the tile without exception or a
    // disconnection occurs
    size_t Retry = 0;
    bool  StopTrying = false;
    while (!StopTrying && (m_pPageFile->GetState() != HRFInternetImagingFile::DISCONNECTED))
        {
        try
            {
            // Read the tile
            Result = ReadTile(TileID, po_pData);

            // If we passed, it is OK
            StopTrying = true;
            }

        // Either a server exception or a connection exception occurred, so
        // we enter the retry loop.
        catch(HRFInternetImagingException& )
            {
            // Increment the retry count
            Retry++;

            // clear the last exception thread
            m_pPageFile->ClearThreadException();

            // Update Progress Information
                {
                HFCMonitor ProgressMonitor(m_pPageFile->s_ProgressKey);
                m_pPageFile->s_ProgressTotalTiles   -= m_pPageFile->m_ProgressCurrentTiles;
                m_pPageFile->m_ProgressCurrentTiles = 0;
                }

            // If the cancellation is needed, we have to get out of the loop
            // right now.  So place a ludicrous number in the retry so the
            // the condition just below fails and a throw can occur.
            if (m_pPageFile->m_RequestStopped)
                {
                Retry = INT_MAX;
                m_pPageFile->m_RequestStopped = false;
                }

            // Set the state to disconnected
            m_pPageFile->SetState(HRFInternetImagingFile::RECONNECTING);

            // if the retry count has not been reached, try to reconnect
            uint32_t reconnectionRetries(0);
            uint32_t reconnectionDelay(0);
            ImagePP::ImageppLib::GetHost().GetImageppLibAdmin()._GetInternetImagingReconnectionSettings(reconnectionRetries,reconnectionDelay);
            if (Retry < reconnectionRetries)
                {
                // try to reconnect
                if (!m_pPageFile->Reconnect())
                    HFCThread::Sleep(reconnectionDelay);

                // clear the last exception thread
                m_pPageFile->ClearThreadException();

                // Clear the thread buffer
                m_pPageFile->m_pThread->ClearBuffer();

                // Clear the current waiting tiles
                m_pPageFile->m_TilePool.Invalidate();
                m_pPageFile->m_TilePool.Clear();
                }

            // if the retry count has been reached, give the problem to somebody else
            else
                {
                // Clear the thread buffer
                m_pPageFile->m_pThread->ClearBuffer();

                // Clear the current waiting tiles
                m_pPageFile->m_TilePool.Invalidate();
                m_pPageFile->m_TilePool.Clear();

                // Set the state to disconnected
                m_pPageFile->SetState(HRFInternetImagingFile::DISCONNECTED);

                // set HSTATUS
                Result = H_ERROR;
                StopTrying = true;
                }
            }

        // Another type of exception occurred, that may have been transferred
        // by the HIP 1.2 Error Code, we can assume that the server could
        // not internally reopen the image, that a disk problem occurred,
        // anything, so we set the file to disconnect and throw the exception.
        catch(HFCException& rException)
            {
            // clear the last exception thread
            m_pPageFile->ClearThreadException();

            // Update Progress Information
                {
                HFCMonitor ProgressMonitor(m_pPageFile->s_ProgressKey);
                m_pPageFile->s_ProgressTotalTiles   -= m_pPageFile->m_ProgressCurrentTiles;
                m_pPageFile->m_ProgressCurrentTiles = 0;
                }

            // Clear the thread buffer
            m_pPageFile->m_pThread->ClearBuffer();

            // Clear the current waiting tiles
            m_pPageFile->m_TilePool.Invalidate();
            m_pPageFile->m_TilePool.Clear();

            // Set the state to disconnected
            m_pPageFile->SetState(HRFInternetImagingFile::DISCONNECTED);

            // Set HSTATUS
            if (rException.IsCompatibleWith(HFCInternetConnectionException::CLASS_ID))
                {
                switch(((const HFCInternetConnectionException&)rException).GetErrorType())
                    {
                    case HFCInternetConnectionException::CONNECTION_LOST:
                        Result = H_INTERNET_CONNECTION_LOST;
                        break;
                    case HFCInternetConnectionException::CANNOT_CONNECT:
                        Result = H_INTERNET_CANNOT_CONNECT;
                        break;
                    case HFCInternetConnectionException::UNKNOWN:
                        Result = H_INTERNET_UNKNOWN_ERROR;
                        break;
                    default:
                        Result = H_INTERNET_UNKNOWN_ERROR;
                        break;
                    }
                }
            else
                Result = H_ERROR;

            StopTrying = true;
            }
        }

WRAPUP:
    return (Result);
    }


//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
HSTATUS HRFInternetImagingTileEditor::ReadBlock(uint32_t           pi_PosTileX,
                                                uint32_t           pi_PosTileY,
                                                HFCPtr<HCDPacket>& po_rpPacket,
                                                HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(po_rpPacket != 0);
    HSTATUS Result = H_ERROR;
    uint64_t TileID = m_TileDescriptor.ComputeID(pi_PosTileX, pi_PosTileY, m_Resolution);

#ifdef TRACE_LOOKAHEAD
    WChar Msg[256];
    BeStringUtilities::Snwprintf(Msg, L"ReadBlock : %d\n", TileID);
    HDEBUGTEXT(Msg);
#endif
    // If disconnected, set error as IO_ERROR
    if (m_pPageFile->GetState() == HRFInternetImagingFile::DISCONNECTED)
        {
        Result = H_IOERROR;
        goto WRAPUP;
        }

    // Retry loop.  Retry until we successfully read the tile without exception or a
    // disconnection occurs
    size_t Retry = 0;
    bool  StopTrying = false;
    while (!StopTrying && (m_pPageFile->GetState() != HRFInternetImagingFile::DISCONNECTED))
        {
        try
            {
            // Read the tile
            Result = ReadTile(TileID, po_rpPacket);

            // If we passed, it is OK
            StopTrying = true;
            }

        // Either a server exception or a connection exception occurred, so
        // we enter the retry loop.
        catch(HRFInternetImagingException&)
            {
            // Increment the retry count
            Retry++;

            // clear the last exception thread
            m_pPageFile->ClearThreadException();

            // Update Progress Information
                {
                HFCMonitor ProgressMonitor(m_pPageFile->s_ProgressKey);
                m_pPageFile->s_ProgressTotalTiles   -= m_pPageFile->m_ProgressCurrentTiles;
                m_pPageFile->m_ProgressCurrentTiles = 0;
                }

            // If the cancellation is needed, we have to get out of the loop
            // right now.  So place a ludicrous number in the retry so the
            // the condition just below fails and a throw can occur.
            if (m_pPageFile->m_RequestStopped)
                {
                Retry = INT_MAX;
                m_pPageFile->m_RequestStopped = false;
                }

            // Set the state to disconnected
            m_pPageFile->SetState(HRFInternetImagingFile::RECONNECTING);

            // if the retry count has not been reached, try to reconnect
            uint32_t reconnectionRetries(0);
            uint32_t reconnectionDelay(0);
            ImagePP::ImageppLib::GetHost().GetImageppLibAdmin()._GetInternetImagingReconnectionSettings(reconnectionRetries,reconnectionDelay);

            if (Retry < reconnectionRetries)
                {
                // try to reconnect
                if (!m_pPageFile->Reconnect())
                    HFCThread::Sleep(reconnectionDelay);

                // clear the last exception thread
                m_pPageFile->ClearThreadException();

                // Clear the thread buffer
                m_pPageFile->m_pThread->ClearBuffer();

                // Clear the current waiting tiles
                m_pPageFile->m_TilePool.Invalidate();
                m_pPageFile->m_TilePool.Clear();
                }

            // if the retry count has been reached, give the problem to somebody else
            else
                {
                // Clear the thread buffer
                m_pPageFile->m_pThread->ClearBuffer();

                // Clear the current waiting tiles
                m_pPageFile->m_TilePool.Invalidate();
                m_pPageFile->m_TilePool.Clear();

                // Set the state to disconnected
                m_pPageFile->SetState(HRFInternetImagingFile::DISCONNECTED);

                // set HSTATUS
                Result = H_ERROR;
                StopTrying = true;
                }
            }

        // Another type of exception occurred, that may have been transferred
        // by the HIP 1.2 Error Code, we can assume that the server could
        // not internally reopen the image, that a disk problem occurred,
        // anything, so we set the file to disconnect and throw the exception.
        catch(HFCException&)
            {
            // clear the last exception thread
            m_pPageFile->ClearThreadException();

            // Update Progress Information
                {
                HFCMonitor ProgressMonitor(m_pPageFile->s_ProgressKey);
                m_pPageFile->s_ProgressTotalTiles   -= m_pPageFile->m_ProgressCurrentTiles;
                m_pPageFile->m_ProgressCurrentTiles = 0;
                }

            // Clear the thread buffer
            m_pPageFile->m_pThread->ClearBuffer();

            // Clear the current waiting tiles
            m_pPageFile->m_TilePool.Invalidate();
            m_pPageFile->m_TilePool.Clear();

            // Set the state to disconnected
            m_pPageFile->SetState(HRFInternetImagingFile::DISCONNECTED);

            // set HSTATUS
            Result = H_ERROR;
            StopTrying = true;
            }
        }

WRAPUP:
    return Result;
    }


//-----------------------------------------------------------------------------
// Private
// Waits for a tile from the pool to be signaled.  Data is returned uncompressed.
//-----------------------------------------------------------------------------
HSTATUS HRFInternetImagingTileEditor::ReadTile(uint64_t pi_TileID,
                                               Byte*  po_pData)
    {
    HPRECONDITION(po_pData != 0);
    HSTATUS Result = H_ERROR;

    try
        {
        // verify thread exception to handle cancellation.
        m_pPageFile->HandleThreadException();

        // Obtain the tile from the tile pool
        HFCPtr<HRFTile> pTile(GetTile(pi_TileID));
        if (pTile != 0)
            {
            if (!m_WaitTileOnRead)
                {
                if (pTile->WaitUntilSignaled(0))
                    {
                    HFCMonitor TileMonitor(pTile);
                    if (pTile->IsValid())
                        {
                        HASSERT(pTile->GetDataSize() > 0);

                        // get the compression type in the data
                        uint32_t CompressionType = *((uint32_t*)pTile->GetData());

                        // if the tile is valid (compression other that IIP_COMPRESSION_INVALID, decode it
                        if (CompressionType != IIP_COMPRESSION_INVALID)
                            Result = DecodeTile(po_pData, pTile->GetData(), pTile->GetDataSize());
                        else
                            {
                            Result = H_INVALID;
                            }
                        }
                    else
                        {
                        Result = H_INVALID;
                        }

                    // remove the tile from the tile pool
                    m_pPageFile->m_TilePool.RemoveTile(pi_TileID);
                    }
                else
                    {
                    Result = H_NOT_FOUND;
                    }
                }
            else
                {
                if (pTile->WaitUntilSignaled(0) || m_pPageFile->WaitForEvent(*pTile, m_pPageFile->m_TileWaitTime))
                    {
                    // continue if the tile is valid
                    HFCMonitor TileMonitor(pTile);
                    if (pTile->IsValid())
                        {
                        HASSERT(pTile->GetDataSize() > 0);

                        // get the compression type in the data
                        uint32_t CompressionType = *((uint32_t*)pTile->GetData());

                        // if the tile is valid (compression other that IIP_COMPRESSION_INVALID, decode it
                        if (CompressionType != IIP_COMPRESSION_INVALID)
                            Result = DecodeTile(po_pData, pTile->GetData(), pTile->GetDataSize());
                        else
                            {
                            Result = H_INVALID;
                            }
                        }
                    else
                        {
                        Result = H_INVALID;
                        }
                    }
                else
                    {
                    Result = H_TIME_OUT;
                    }

                // remove the tile from the tile pool
                m_pPageFile->m_TilePool.RemoveTile(pi_TileID);
                }
            }
        }
    catch(HFCInternetConnectionException& ConnectionException)
        {
        // repackage as an Internet Imaging Connection Exception
        HRFInternetImagingConnectionException::ErrorType ErrorType;
        switch(ConnectionException.GetErrorType())
            {
            case HFCInternetConnectionException::CANNOT_CONNECT:
                ErrorType = HRFInternetImagingConnectionException::CANNOT_CONNECT;
                break;

            case HFCInternetConnectionException::CONNECTION_LOST:
                ErrorType = HRFInternetImagingConnectionException::CONNECTION_LOST;
                break;

            default:
            case HFCInternetConnectionException::UNKNOWN:
                ErrorType = HRFInternetImagingConnectionException::UNKNOWN;
                break;

            }

        if (m_WaitTileOnRead)
            {
            // set the exception
            throw HRFInternetImagingConnectionException(m_pRasterFile->GetURL()->GetURL(), ErrorType);
            }
        }

    return (Result);
    }


//-----------------------------------------------------------------------------
// Private
// Waits for a tile from the pool to be signaled. Data is returned compressed.
//-----------------------------------------------------------------------------
HSTATUS HRFInternetImagingTileEditor::ReadTile(uint64_t          pi_TileID,
                                               HFCPtr<HCDPacket>& po_rpPacket)
    {
    HPRECONDITION(po_rpPacket != 0);
    HSTATUS Result = H_ERROR;

    try
        {
        // verify thread exception to handle cancellation.
        m_pPageFile->HandleThreadException();

        // Obtain the tile from the tile pool
        HFCPtr<HRFTile> pTile(GetTile(pi_TileID));
        if (pTile != 0)
            {
            if (!m_WaitTileOnRead)
                {
                if (pTile->WaitUntilSignaled(0))
                    {
                    HFCMonitor TileMonitor(pTile);
                    if (pTile->IsValid())
                        {
                        HASSERT(pTile->GetDataSize() > 0);

                        // get the compression type in the data
                        uint32_t CompressionType = *((uint32_t*)pTile->GetData());

                        // if the tile is valid (compression other that IIP_COMPRESSION_INVALID, decode it
                        if (CompressionType != IIP_COMPRESSION_INVALID)
                            {
                            // verify if the packet has a buffer.  If not, create one for it
                            if (po_rpPacket->GetBufferSize() == 0)
                                {
                                po_rpPacket->SetBuffer(new Byte[pTile->GetDataSize() - 8],
                                                       pTile->GetDataSize() - 8);
                                po_rpPacket->SetBufferOwnership(true);
                                }
                            HASSERT(po_rpPacket->GetBufferSize() >= pTile->GetDataSize() - 8);

                            // Setup the packet with the tile data
                            memcpy(po_rpPacket->GetBufferAddress(), pTile->GetData() + 8, pTile->GetDataSize() - 8);
                            po_rpPacket->SetDataSize(pTile->GetDataSize() - 8);
                            po_rpPacket->SetCodec(GetCodec(pTile->GetData()));
                            }
                        else
                            {
                            Result = H_INVALID;
                            }
                        }
                    else
                        {
                        Result = H_INVALID;
                        }

                    // remove the tile from the tile pool
                    m_pPageFile->m_TilePool.RemoveTile(pi_TileID);
                    }
                else
                    {
                    Result = H_NOT_FOUND;
                    }
                }
            else
                {
                if (pTile->WaitUntilSignaled(0) || m_pPageFile->WaitForEvent(*pTile, m_pPageFile->m_TileWaitTime))
                    {
                    // continue if the tile is valid
                    HFCMonitor TileMonitor(pTile);
                    if (pTile->IsValid())
                        {
                        HASSERT(pTile->GetDataSize() >= 8);

                        // get the compression type in the data (first 4 bytes of the tile data)
                        uint32_t CompressionType = *((uint32_t*)pTile->GetData());

                        // if the tile is valid (compression other that IIP_COMPRESSION_INVALID, decode it
                        if (CompressionType != IIP_COMPRESSION_INVALID)
                            {
                            // verify if the packet has a buffer.  If not, create one for it
                            if (po_rpPacket->GetBufferSize() == 0)
                                {
                                po_rpPacket->SetBuffer(new Byte[pTile->GetDataSize() - 8],
                                                       pTile->GetDataSize() - 8);
                                po_rpPacket->SetBufferOwnership(true);
                                }
                            HASSERT(po_rpPacket->GetBufferSize() >= pTile->GetDataSize() - 8);

                            // Setup the packet with the tile data
                            memcpy(po_rpPacket->GetBufferAddress(), pTile->GetData() + 8, pTile->GetDataSize() - 8);
                            po_rpPacket->SetDataSize(pTile->GetDataSize() - 8);
                            po_rpPacket->SetCodec(GetCodec(pTile->GetData()));
                            Result = H_SUCCESS;
                            }
                        else
                            {
                            Result = H_INVALID;
                            }
                        }
                    else
                        {
                        Result = H_INVALID;
                        }
                    }
                else
                    {
                    Result = H_TIME_OUT;
                    }

                // remove the tile from the tile pool
                m_pPageFile->m_TilePool.RemoveTile(pi_TileID);
                }
            }
        }
    catch(HFCInternetConnectionException& ConnectionException)
        {
        // repackage as an Internet Imaging Connection Exception
        HRFInternetImagingConnectionException::ErrorType ErrorType;
        switch(ConnectionException.GetErrorType())
            {
            case HFCInternetConnectionException::CANNOT_CONNECT:
                ErrorType = HRFInternetImagingConnectionException::CANNOT_CONNECT;
                break;

            case HFCInternetConnectionException::CONNECTION_LOST:
                ErrorType = HRFInternetImagingConnectionException::CONNECTION_LOST;
                break;

            default:
            case HFCInternetConnectionException::UNKNOWN:
                ErrorType = HRFInternetImagingConnectionException::UNKNOWN;
                break;

            }

        if (m_WaitTileOnRead)
            {
            // set the exception
            throw HRFInternetImagingConnectionException(m_pRasterFile->GetURL()->GetURL(), ErrorType);
            }
        }

    return (Result);
    }


//-----------------------------------------------------------------------------
// Private
//
//-----------------------------------------------------------------------------
HFCPtr<HRFTile> HRFInternetImagingTileEditor::GetTile(uint64_t pi_TileID)
    {
    // Try to obtain the tile from the raster file tile pool
    HFCPtr<HRFTile> pResult(m_pPageFile->m_TilePool.GetTile(pi_TileID));

    // If the tile is not present, generate a request for it
    if (pResult == 0)
        {
        // Secure the connection
        HFCMonitor Monitor(m_pPageFile->m_ConnectionKey);

        // if the connection does not exists, throw an exception
        if (m_pPageFile->m_pConnection.get() == 0)
            throw HRFInternetImagingConnectionException(m_pRasterFile->GetURL()->GetURL(),
                                                        HRFInternetImagingConnectionException::CONNECTION_LOST);

        // Get the Index & Resolution from the ID
        unsigned short Resolution = (unsigned short)m_TileDescriptor.GetLevel(pi_TileID); // the maximum level is 255
        uint64_t TileIndex  = m_TileDescriptor.GetIndex(pi_TileID);

#ifdef __HMR_DEBUG
        WChar Temp[33]; // see ltoa doc
        WString Message(L"Maybe you should think about using the LookAhead: Tile #");
        Message += _i64tow(TileIndex, Temp, 10);
        Message += L", Resolution #";
        Message += _ltow(Resolution, Temp, 10);
        Message += L".\n";
        HDEBUGTEXT(Message.c_str());
#endif

        // Compute the position of the tile
        uint64_t PosX, PosY;
        m_TileDescriptor.GetPositionFromIndex(TileIndex, &PosX, &PosY);

        // Compute the IIP resolution from the sub-image
        unsigned short IIPResolution;
        if (m_pPageFile->m_IsUnlimitedResolution)   // for the unlimited resolution, use the resolution set in m_pPageFile->m_UnlimitedResolutionMap
            IIPResolution = m_Resolution;
        else
            IIPResolution = m_pPageFile->GetPageDescriptor(m_Page)->CountResolutions() - Resolution - 1;

        // Add the tile to the waiting list
        pResult = new HRFTile(pi_TileID, TileIndex, PosX, PosY, Resolution);
        HASSERT(pResult != 0);

        // append the current tile to the request
        ostringstream Request;

        if (m_pPageFile->m_IsUnlimitedResolution)
            {
            Request << "HIP-TIL=" << m_StrResFactor << "," << TileIndex << "," << m_Page;
            }
        else
            Request << "TIL=" << IIPResolution << "," << TileIndex << "," << m_Page;

        // Send the request
        try
            {
            m_pPageFile->m_pConnection->Send((const Byte*)Request.str().c_str(), Request.str().size());
            m_pPageFile->m_TilePool.AddTile(pi_TileID, pResult);

            // Update Progress info
            HFCMonitor ProgressMonitor(m_pPageFile->s_ProgressKey);
            m_pPageFile->s_ProgressTotalTiles++;
            m_pPageFile->m_ProgressCurrentTiles++;
            }
        catch(HFCInternetConnectionException& ConnectionException)
            {
            // repackage as an Internet Imaging Connection Exception
            HRFInternetImagingConnectionException::ErrorType ErrorType;
            switch(ConnectionException.GetErrorType())
                {
                case HFCInternetConnectionException::CANNOT_CONNECT:
                    ErrorType = HRFInternetImagingConnectionException::CANNOT_CONNECT;
                    break;

                case HFCInternetConnectionException::CONNECTION_LOST:
                    ErrorType = HRFInternetImagingConnectionException::CONNECTION_LOST;
                    break;

                default:
                case HFCInternetConnectionException::UNKNOWN:
                    ErrorType = HRFInternetImagingConnectionException::UNKNOWN;
                    break;

                }

            // set the exception
            throw HRFInternetImagingConnectionException(m_pRasterFile->GetURL()->GetURL(),
                                                        ErrorType);
            }
        }

    return (pResult);
    }


//-----------------------------------------------------------------------------
// Private
//
//-----------------------------------------------------------------------------
HSTATUS HRFInternetImagingTileEditor::DecodeTile(Byte*       po_pDecoded,
                                                 const Byte* pi_pEncoded,
                                                 size_t       pi_EncodedSize)
    {
    HPRECONDITION(po_pDecoded != 0);
    HPRECONDITION(pi_pEncoded != 0);
    HPRECONDITION(pi_EncodedSize >= 8);
    HSTATUS Result = H_ERROR;

    // Get the data info (after the 8 compression info bytes);
    const Byte* pEncoded = pi_pEncoded    + 8;
    size_t EncodedSize    = pi_EncodedSize - 8;

    // Find codec to use & Decompress data
    HFCPtr<HCDCodec> pCodec = GetCodec(pi_pEncoded);
    if (pCodec != 0)
        {
        // Create source and destination packets
        HCDPacket CompressedImage(pCodec, (Byte*)pEncoded, EncodedSize, EncodedSize);
        HCDPacket UncompressedImage(po_pDecoded, m_TileSize);

        // Decompress
        if (CompressedImage.Decompress(&UncompressedImage))
            Result = H_SUCCESS;
        }

    return (Result);
    }


//-----------------------------------------------------------------------------
// Private
//
//-----------------------------------------------------------------------------
HFCPtr<HCDCodec> HRFInternetImagingTileEditor::GetCodec(const Byte* pi_pData)
    {
    HPRECONDITION(pi_pData != 0);

    // To simplify my life
    uint32_t BlockWidth   = m_pResolutionDescriptor->GetBlockWidth();
    uint32_t BlockHeight  = m_pResolutionDescriptor->GetBlockHeight();
    uint32_t BitsPerPixel = m_pResolutionDescriptor->GetPixelType()->CountPixelRawDataBits();

    // Type used for analyzing the compression sub-type.  For certain compressions,
    // the type is an int and for others it is 4 bytes.
    union
        {
        uint32_t m_IntValue;
        Byte m_ByteValue[4];
        }                SubType;
    uint32_t          Type;
    HFCPtr<HCDCodec> pResult;


    // Get the compression info
    // The tile's compression data is the first 8 bytes of the data stream
    // after which it is the actual tile image.
    //
    // The compression data contains 2 unsigned 32-bit integers:
    //
    //  Compression Type    :   Indicates which compression is in use
    //  Compression SubType :   Information for the compression in use
    Type               = ((uint32_t*) pi_pData)[0];
    SubType.m_IntValue = ((uint32_t*) pi_pData)[1];

    // Find codec
    switch (Type)
        {
            //
            // No Compression
            //
        case IIP_COMPRESSION_NONE:
            if (m_pCodecIdentity == 0)
                {
                m_pCodecIdentity = new HCDCodecIdentity(m_TileSize);
                HASSERT(m_pCodecIdentity != 0);
                }
            pResult = m_pCodecIdentity;
            break;

            //
            // Single Color
            //
        case IIP_COMPRESSION_SINGLE_COLOR:
            if (m_pCodecSingleColor == 0)
                {
                m_pCodecSingleColor = new HCDCodecFPXSingleColor(BlockWidth,
                                                                 BlockHeight,
                                                                 BitsPerPixel);
                HASSERT(m_pCodecSingleColor != 0);
                }
            pResult = m_pCodecSingleColor;
            break;

            //
            // Jpeg-Alpha
            //
        case HIP_COMPRESSION_JPEG_RLE8:
            if (m_pCodecJpeg == 0)
                {
                uint32_t BitsperPixel;
                if(m_pResolutionDescriptor->GetPixelType()->GetClassID() == HRPPixelTypeV32PR8PG8PB8A8::CLASS_ID)
                    BitsperPixel = 32;
                else if(m_pResolutionDescriptor->GetPixelType()->GetClassID() == HRPPixelTypeV16PRGray8A8::CLASS_ID)
                    BitsperPixel = 16;
                else
                    {
                    BitsperPixel=0;
                    HASSERT(0);
                    }

                // create the codec
                m_pCodecJpeg = new HCDCodecJPEGAlpha(BlockWidth,
                                                     BlockHeight,
                                                     BitsperPixel);
                HASSERT(m_pCodecJpeg != 0);
                HFCMonitor Monitor(m_pPageFile->m_DataKey);

                // if the table exists and is not empty, add it to the codec
                if ((m_pPageFile->m_JpegTables[1] != 0) &&
                    (m_pPageFile->m_JpegTables[1]->GetDataSize() > 0))
                    {
                    ((HFCPtr<HCDCodecJPEGAlpha>&)m_pCodecJpeg)->SetAbbreviateMode(true);
                    ((HFCPtr<HCDCodecJPEGAlpha>&)m_pCodecJpeg)->ReadHeader(m_pPageFile->m_JpegTables[1]->GetData(),
                                                                           m_pPageFile->m_JpegTables[1]->GetDataSize());
                    ((HFCPtr<HCDCodecJPEGAlpha>&)m_pCodecJpeg)->CopyTablesFromDecoderToEncoder();
                    }
                }
            pResult = m_pCodecJpeg;
            break;

            //
            // Jpeg
            //
        case IIP_COMPRESSION_JPEG:
            // if the version of the protocol is HIP 1.0, use the old flashpix codec,
            // otherwise use the new one
            if (m_pPageFile->GetProtocolVersion() == s_HIP0100)
                {
                if (m_pCodecJpeg == 0)
                    {
                    HCDCodecFlashpixOLDForMSI10::ColorModes ColorMode;
                    if(m_pResolutionDescriptor->GetPixelType()->GetClassID() == HRPPixelTypeV24R8G8B8::CLASS_ID)
                        ColorMode = HCDCodecFlashpixOLDForMSI10::RGB;
                    else if(m_pResolutionDescriptor->GetPixelType()->GetClassID() == HRPPixelTypeV24PhotoYCC::CLASS_ID)
                        ColorMode = HCDCodecFlashpixOLDForMSI10::PHOTOYCC;
                    else if(m_pResolutionDescriptor->GetPixelType()->GetClassID() == HRPPixelTypeV32PR8PG8PB8A8::CLASS_ID)
                        ColorMode = HCDCodecFlashpixOLDForMSI10::RGB_OPACITY;
                    else if(m_pResolutionDescriptor->GetPixelType()->GetClassID() == HRPPixelTypeV32PRPhotoYCCA8::CLASS_ID)
                        ColorMode = HCDCodecFlashpixOLDForMSI10::PHOTOYCC_OPACITY;
                    else if(m_pResolutionDescriptor->GetPixelType()->GetClassID() == HRPPixelTypeV8Gray8::CLASS_ID)
                        ColorMode = HCDCodecFlashpixOLDForMSI10::MONOCHROME;
                    else if(m_pResolutionDescriptor->GetPixelType()->GetClassID() == HRPPixelTypeV8GrayWhite8::CLASS_ID)
                        ColorMode = HCDCodecFlashpixOLDForMSI10::MONOCHROME;
                    else if(m_pResolutionDescriptor->GetPixelType()->GetClassID() == HRPPixelTypeV16PRGray8A8::CLASS_ID)
                        ColorMode = HCDCodecFlashpixOLDForMSI10::MONOCHROME_OPACITY;
                    else
                        {
                        ColorMode = HCDCodecFlashpixOLDForMSI10::RGB;
                        HASSERT(0);
                        }

                    m_pCodecJpeg = new HCDCodecFlashpixOLDForMSI10(BlockWidth,
                                                                   BlockHeight,
                                                                   ColorMode);
                    HASSERT(m_pCodecJpeg != 0);
                    HFCMonitor Monitor(m_pPageFile->m_DataKey);
                    for (uint32_t Table = 1;
                         Table < m_pPageFile->m_JpegTables.size();
                         Table++)
                        {
                        // if the table exists and is not empty, add it to the codec
                        if ((m_pPageFile->m_JpegTables[Table] != 0) &&
                            (m_pPageFile->m_JpegTables[Table]->GetDataSize() > 0))
                            {
                            // extract data for programmer convenience (AKA laziness)
                            uint32_t     TableSize = m_pPageFile->m_JpegTables[Table]->GetDataSize();

                            // if the table is not empty, set it in the codec
                            if (TableSize > 0)
                                {
                                const Byte* pTable = m_pPageFile->m_JpegTables[Table]->GetData();
                                ((HFCPtr<HCDCodecFlashpixOLDForMSI10>&)m_pCodecJpeg)->SetTable(Table, (Byte*)pTable, TableSize);
                                }
                            }
                        }
                    }

                ((HFCPtr<HCDCodecFlashpixOLDForMSI10>&)m_pCodecJpeg)->EnableInterleave      (SubType.m_ByteValue[IIP_COMPRESSION_JPEG_INTERLEAVE_INDEX] != 0);
                ((HFCPtr<HCDCodecFlashpixOLDForMSI10>&)m_pCodecJpeg)->SetSubSampling        (SubType.m_ByteValue[IIP_COMPRESSION_JPEG_SUBSAMPLING_INDEX]);
                ((HFCPtr<HCDCodecFlashpixOLDForMSI10>&)m_pCodecJpeg)->EnableColorConversion (SubType.m_ByteValue[IIP_COMPRESSION_JPEG_COLOR_INDEX] != 0);
                ((HFCPtr<HCDCodecFlashpixOLDForMSI10>&)m_pCodecJpeg)->SetCurrentTable       (SubType.m_ByteValue[IIP_COMPRESSION_JPEG_TABLE_INDEX]);
                pResult = m_pCodecJpeg;
                }
            else
                {
                if (m_pCodecJpeg == 0)
                    {
                    HCDCodecFlashpix::ColorModes ColorMode;
                    if(m_pResolutionDescriptor->GetPixelType()->GetClassID() == HRPPixelTypeV24R8G8B8::CLASS_ID ||
                       m_pResolutionDescriptor->GetPixelType()->GetClassID() == HRPPixelTypeV24B8G8R8::CLASS_ID)
                        ColorMode = HCDCodecFlashpix::RGB;
                    else if(m_pResolutionDescriptor->GetPixelType()->GetClassID() == HRPPixelTypeV24PhotoYCC::CLASS_ID)
                        ColorMode = HCDCodecFlashpix::PHOTOYCC;
                    else if(m_pResolutionDescriptor->GetPixelType()->GetClassID() == HRPPixelTypeV32PR8PG8PB8A8::CLASS_ID)
                        ColorMode = HCDCodecFlashpix::RGB_OPACITY;
                    else if(m_pResolutionDescriptor->GetPixelType()->GetClassID() == HRPPixelTypeV32PRPhotoYCCA8::CLASS_ID)
                        ColorMode = HCDCodecFlashpix::PHOTOYCC_OPACITY;
                    else if(m_pResolutionDescriptor->GetPixelType()->GetClassID() == HRPPixelTypeV8Gray8::CLASS_ID)
                        ColorMode = HCDCodecFlashpix::MONOCHROME;
                    else if(m_pResolutionDescriptor->GetPixelType()->GetClassID() == HRPPixelTypeV8GrayWhite8::CLASS_ID)
                        ColorMode = HCDCodecFlashpix::MONOCHROME;
                    else if(m_pResolutionDescriptor->GetPixelType()->GetClassID() == HRPPixelTypeV16PRGray8A8::CLASS_ID)
                        ColorMode = HCDCodecFlashpix::MONOCHROME_OPACITY;
                    else
                        {
                        ColorMode = HCDCodecFlashpix::RGB;
                        HASSERT(0);
                        }

                    m_pCodecJpeg = new HCDCodecFlashpix(BlockWidth,
                                                        BlockHeight,
                                                        ColorMode);
                    HASSERT(m_pCodecJpeg != 0);
                    HFCMonitor Monitor(m_pPageFile->m_DataKey);
                    for (uint32_t Table = 1;
                         Table < m_pPageFile->m_JpegTables.size();
                         Table++)
                        {
                        // if the table exists and is not empty, add it to the codec
                        if ((m_pPageFile->m_JpegTables[Table] != 0) &&
                            (m_pPageFile->m_JpegTables[Table]->GetDataSize() > 0))
                            {
                            // extract data for programmer convenience (AKA laziness)
                            size_t       TableSize = m_pPageFile->m_JpegTables[Table]->GetDataSize();

                            // if the table is not empty, set it in the codec
                            if (TableSize > 0)
                                {
                                const Byte* pTable = m_pPageFile->m_JpegTables[Table]->GetData();
                                ((HFCPtr<HCDCodecFlashpix>&)m_pCodecJpeg)->SetTable(Table, (Byte*)pTable, TableSize);
                                }
                            }
                        }
                    }

                ((HFCPtr<HCDCodecFlashpix>&)m_pCodecJpeg)->EnableInterleave      (SubType.m_ByteValue[IIP_COMPRESSION_JPEG_INTERLEAVE_INDEX] != 0);
                ((HFCPtr<HCDCodecFlashpix>&)m_pCodecJpeg)->SetSubSampling        (SubType.m_ByteValue[IIP_COMPRESSION_JPEG_SUBSAMPLING_INDEX]);
                ((HFCPtr<HCDCodecFlashpix>&)m_pCodecJpeg)->EnableColorConversion (SubType.m_ByteValue[IIP_COMPRESSION_JPEG_COLOR_INDEX] != 0);
                ((HFCPtr<HCDCodecFlashpix>&)m_pCodecJpeg)->SetCurrentTable       (SubType.m_ByteValue[IIP_COMPRESSION_JPEG_TABLE_INDEX]);
                pResult = m_pCodecJpeg;
                }
            break;

            //
            // Wavelet
            //
        case HIP_COMPRESSION_WAVELET:
#ifdef __HMR_SUMMUS_SUPPORTED
            if ((m_pCodecWavelet == 0) &&
                (HCDCodecSummus::IsAvailable()) )
                {
                m_pCodecWavelet = new HCDCodecSummus(BlockWidth, BlockHeight, BitsPerPixel);
                HASSERT(m_pCodecWavelet != 0);
                }
#endif
            pResult = m_pCodecWavelet;
            break;

            //
            // Deflate
            //
        case HIP_COMPRESSION_DEFLATE:
            if (m_pCodecDeflate == 0)
                {
                m_pCodecDeflate = new HCDCodecZlib(m_TileSize);
                HASSERT(m_pCodecDeflate != 0);
                }
            pResult = m_pCodecDeflate;
            break;

            //
            // Packbits
            //
        case HIP_COMPRESSION_PACKBITS:
            if (m_pCodecPackBits == 0)
                {
                m_pCodecPackBits = new HCDCodecHMRPackBits(BlockWidth, BlockHeight, BitsPerPixel);
                HASSERT(m_pCodecPackBits != 0);
                }
            pResult = m_pCodecPackBits;
            break;

            //
            // CCITT3
            //
        case HIP_COMPRESSION_CCITT3:
            if (m_pCodecCCITT3 == 0)
                {
                m_pCodecCCITT3 = new HCDCodecHMRCCITT(BlockWidth, BlockHeight);
                HASSERT(m_pCodecCCITT3 != 0);
                ((HFCPtr<HCDCodecHMRCCITT>&)m_pCodecCCITT3)->SetCCITT3(true);
                }
            ((HFCPtr<HCDCodecHMRCCITT>&)m_pCodecCCITT3)->SetPhotometric(SubType.m_ByteValue[IIP_COMPRESSION_CCITT_PHOTOMETRIC]);
            pResult = m_pCodecCCITT3;
            break;

            //
            // CCITT4
            //
        case HIP_COMPRESSION_CCITT4:
            if (m_pCodecCCITT4 == 0)
                {
                m_pCodecCCITT4 = new HCDCodecHMRCCITT(BlockWidth, BlockHeight);
                HASSERT(m_pCodecCCITT4 != 0);
                ((HFCPtr<HCDCodecHMRCCITT>&)m_pCodecCCITT4)->SetCCITT3(false);
                }
            ((HFCPtr<HCDCodecHMRCCITT>&)m_pCodecCCITT4)->SetPhotometric(SubType.m_ByteValue[IIP_COMPRESSION_CCITT_PHOTOMETRIC]);
            pResult = m_pCodecCCITT4;
            break;

            //
            // RLE
            //
        case HIP_COMPRESSION_RLE:
            if (m_pCodecRLE == 0)
                {
                m_pCodecRLE = new HCDCodecHMRRLE1(BlockWidth, BlockHeight);
                HASSERT(m_pCodecRLE != 0);
                }
            ((HFCPtr<HCDCodecHMRRLE1>&)m_pCodecRLE)->SetOneLineMode(SubType.m_ByteValue[IIP_COMPRESSION_RLE_ONE_LINE] != 0);
            pResult = m_pCodecRLE;
            break;
        }
    return (pResult);
    }
