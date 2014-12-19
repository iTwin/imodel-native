//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetBinaryHandler.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetBinaryHandler
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetBinaryHandler.h>
#include <Imagepp/all/h/HFCInternetConnection.h>


//-----------------------------------------------------------------------------
// Constant
//-----------------------------------------------------------------------------

const string HRFInternetBinaryHandler::s_Marker   ("\r\n");
const string HRFInternetBinaryHandler::s_Delimiter(":");


//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HRFInternetBinaryHandler::HRFInternetBinaryHandler(const string& pi_rLabel)
    : HRFInternetImagingHandler(pi_rLabel)
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetBinaryHandler::~HRFInternetBinaryHandler()
    {
    }

//-----------------------------------------------------------------------------
// Protected
// Reads until the label-data delimiter has been reached
//-----------------------------------------------------------------------------
void HRFInternetBinaryHandler::ReadUntilData(HFCInternetConnection& pi_rConnection,
                                             HFCBuffer&             pio_rBuffer) const
    {
    // read byte by byte until ":"
    bool MarkerFound = false;
    Byte NextByte;
    while (!MarkerFound)
        {
        // Read the next byte
        pi_rConnection.Receive(&NextByte, 1);

        // add the current byte to the string
        pio_rBuffer.AddData(&NextByte, 1);

        // verify if the last characters of the string are ":"
        if (pio_rBuffer.GetDataSize() > s_Delimiter.size())
            {
            size_t StartPos = pio_rBuffer.GetDataSize() - s_Delimiter.size();
            MarkerFound = (memcmp(pio_rBuffer.GetData() + StartPos,
                                  s_Delimiter.data(),
                                  s_Delimiter.size()) == 0);
            }
        }
    }

//-----------------------------------------------------------------------------
// Protected
// Skips the ending marker of the current response
//-----------------------------------------------------------------------------
void HRFInternetBinaryHandler::ReadData(HFCInternetConnection& pi_rConnection,
                                        HFCBuffer&             pio_rBuffer,
                                        int32_t                pi_DataSize,
                                        bool                  pi_Clear) const
    {
    HPRECONDITION(pi_DataSize > 0);

    // Clear the current data in the buffer
    if (pi_Clear)
        pio_rBuffer.Clear();

    // Read the specified amount of data
    pi_rConnection.Receive(pio_rBuffer.PrepareForNewData(pi_DataSize), pi_DataSize);

    // Adjust the buffer size
    pio_rBuffer.SetNewDataSize(pi_DataSize);
    }


//-----------------------------------------------------------------------------
// Protected
// Skips a certain amount of data
//-----------------------------------------------------------------------------
void HRFInternetBinaryHandler::SkipData(HFCInternetConnection& pi_rConnection,
                                        size_t                 pi_DataSize) const
    {
    Byte NextByte;
    for (size_t i = 0; i < pi_DataSize; i++)
        pi_rConnection.Receive(&NextByte, 1);
    }

//-----------------------------------------------------------------------------
// Protected
// Skips the ending marker of the current response
//-----------------------------------------------------------------------------
void HRFInternetBinaryHandler::SkipEndMarker(HFCInternetConnection& pi_rConnection) const
    {
    SkipData(pi_rConnection, s_Marker.size());
    }