//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetHIPTileHandler.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetHIPTileHandler
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetHIPTileHandler.h>
#include <Imagepp/all/h/HRFInternetImagingFile.h>
#include <Imagepp/all/h/HRFInternetImagingException.h>
#include <Imagepp/all/h/HRFInternetImaging.h>
#include <Imagepp/all/h/HRFMessages.h>

#include <Imagepp/all/h/HGFTileIDDescriptor.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

// label for this response handler
const static string                 s_Label("hip-tile,");   // add ',' to the label to differenciate hip-tile to hip-tile-size


//-----------------------------------------------------------------------------
// Public
// Constructor - Do not use s_Label as the parameter of the parent class,
// because this object is contructed statically and the label may be empty
//-----------------------------------------------------------------------------
HRFInternetHIPTileHandler::HRFInternetHIPTileHandler()
    : HRFInternetBinaryHandler("hip-tile,")
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetHIPTileHandler::~HRFInternetHIPTileHandler()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFInternetHIPTileHandler::Handle(HRFInternetImagingFile& pi_rFile,
                                       HFCBuffer&              pio_rBuffer,
                                       HFCInternetConnection&  pi_rConnection)
    {
    // Scan the connection until the binary data
    ReadUntilData(pi_rConnection, pio_rBuffer);

    //
    // Format:
    //
    //  tile,res,index,sub/size:<data>
    //

    uint32_t ResIndex=0;
    uint64_t Index;
    uint32_t SubImage;
    uint32_t DataSize;
    bool   SkipTile = false;

    string Response((const char*)pio_rBuffer.GetData(), pio_rBuffer.GetDataSize());

    if (pi_rFile.m_IsUnlimitedResolution)
        {
        // find the resolution
        string::size_type Pos = Response.find(',', s_Label.size());
        string Resolution = Response.substr(s_Label.size(), Pos - s_Label.size());


        if (sscanf(Response.c_str() + Pos + 1,  "%I64u,%lu/%lu:", &Index, &SubImage, &DataSize) != 3)
            throw HRFIIHandlerException(HRFII_INVALID_DATA_FORMAT_EXCEPTION,
                                        pi_rFile.GetURL()->GetURL(),
                                        HRFIIHandlerException::HIP_TILE);

        // find in the map the corresponding resolution index
        HFCMonitor Monitor(pi_rFile.m_DataKey);
        HRFInternetImagingFile::UnlimitedResolutionMap::iterator Itr(pi_rFile.m_UnlimitedResolutionMap.find(Resolution));
        if (Itr == pi_rFile.m_UnlimitedResolutionMap.end())
            SkipTile = true; // we have no editor for this tile, read data an forget it
        else
            ResIndex = Itr->second;
        }
    else
        {
        // Place the buffer in a string in order to force a '\0' at the end in order to avoid an access violation
        if (sscanf(string((const char*)pio_rBuffer.GetData(), pio_rBuffer.GetDataSize()).c_str() + s_Label.size(),
                   "%lu,%I64u,%lu/%lu:",
                   &ResIndex,
                   &Index,
                   &SubImage,
                   &DataSize) != 4)
            throw HRFIIHandlerException(HRFII_INVALID_DATA_FORMAT_EXCEPTION,
                                        pi_rFile.GetURL()->GetURL(),
                                        HRFIIHandlerException::HIP_TILE);


        }

    // Create a byte array for the data
    // The pointer will be removed from the auto ptr if all goes well
    // if a exception occurs, it will be destroyed
    HArrayAutoPtr<Byte> pData(new Byte[DataSize]);

    // Read the data & skip end marker
    pi_rConnection.Receive(pData, DataSize);
    SkipEndMarker(pi_rConnection);

    if (!SkipTile)
        {
        // Convert IIP/HIP resolution to an Image++ resolution
        //UInt32 IPPResolution = (UShort)pi_rFile.m_ResolutionCount - ResIndex - 1;

        // Get the TileID
        uint64_t TileID = HRFInternetImagingFile::s_TileDescriptor.ComputeIDFromIndex(Index, ResIndex);

        // Get the tile from the pool
        HFCPtr<HRFTile> pTile(pi_rFile.m_TilePool.GetTile(TileID));

        // if the can be not in pool if a CancelLookAhead occurs

        if (pTile != 0)
            {
            // Set the data in the tile packet & signal it
            // The tile must be unlocked when TileHasArrived is called
            HFCMonitor TileMonitor(*pTile);
            pTile->SetData(pData, DataSize);

#ifdef TRACE_LOOKAHEAD
            WChar Msg[256];
            BeStringUtilities::Snwprintf(Msg, L"Tile %d has arrived\n", TileID);
            HDEBUGTEXT(Msg);
#endif

            pTile->Signal();
            TileMonitor.ReleaseKey();   // <== unlocking occurs here

            // Notify the Internet Imaging File that the tile is ready
            pi_rFile.TileHasArrived(pTile);
            }
        }
    }
