//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetTileHandler.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetTileHandler
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetTileHandler.h>
#include <Imagepp/all/h/HRFInternetImagingFile.h>
#include <Imagepp/all/h/HRFInternetImaging.h>
#include <Imagepp/all/h/HRFMessages.h>
#include <Imagepp/all/h/HRFInternetImagingException.h>


#include <Imagepp/all/h/HGFTileIDDescriptor.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

// label for this response handler
const static string                 s_Label("tile");


//-----------------------------------------------------------------------------
// Public
// Constructor - Do not use s_Label as the parameter of the parent class,
// because this object is contructed statically and the label may be empty
//-----------------------------------------------------------------------------
HRFInternetTileHandler::HRFInternetTileHandler()
    : HRFInternetBinaryHandler("tile")
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetTileHandler::~HRFInternetTileHandler()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFInternetTileHandler::Handle(HRFInternetImagingFile& pi_rFile,
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

    uint32_t Resolution, SubImage, DataSize;
    uint64_t Index;

    // Place the buffer in a string in order to force a '\0' at the end in order to avoid an access violation
    if (sscanf(string((const char*)pio_rBuffer.GetData(), pio_rBuffer.GetDataSize()).c_str() + s_Label.size() + 1,
               "%lu,%I64u,%lu/%lu:",
               &Resolution,
               &Index,
               &SubImage,
               &DataSize) != 4)
        throw HRFIIHandlerException(HRFII_INVALID_DATA_FORMAT_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::TILE);

    // Create a byte array for the data
    // The pointer will be removed from the auto ptr if all goes well
    // if a exception occurs, it will be destroyed
    HArrayAutoPtr<Byte> pData(new Byte[DataSize]);

    // Read the data & skip end marker
    pi_rConnection.Receive(pData, DataSize);
    SkipEndMarker(pi_rConnection);

    // Convert IIP/HIP resolution to an Image++ resolution
    uint32_t IPPResolution = (unsigned short)pi_rFile.m_ResolutionCount - Resolution - 1;

    // Get the TileID
    uint64_t TileID = HRFInternetImagingFile::s_TileDescriptor.ComputeIDFromIndex(Index, IPPResolution);

    // Get the tile from the pool
    HFCPtr<HRFTile> pTile(pi_rFile.m_TilePool.GetTile(TileID));
    if (pTile == 0)
        throw HRFInternetImagingException(HRFII_TILE_NOT_FOUND_EXCEPTION, pi_rFile.GetURL()->GetURL());

    // Set the data in the tile packet & signal it
    // The tile must be unlocked when TileHasArrived is called
        {
        HFCMonitor TileMonitor(*pTile);
        pTile->SetData(pData, DataSize);
        pTile->Signal();
        } // <== unlocking occurs here

    // Notify the Internet Imaging File that the tile is ready
    pi_rFile.TileHasArrived(pTile);
    }
