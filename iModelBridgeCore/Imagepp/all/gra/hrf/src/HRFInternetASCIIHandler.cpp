//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetASCIIHandler.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetASCIIHandler
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetASCIIHandler.h>
#include <Imagepp/all/h/HFCInternetConnection.h>


//-----------------------------------------------------------------------------
// Constant
//-----------------------------------------------------------------------------

const string HRFInternetASCIIHandler::s_Marker("\r\n");


//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HRFInternetASCIIHandler::HRFInternetASCIIHandler(const string& pi_rLabel)
    : HRFInternetImagingHandler(pi_rLabel)
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetASCIIHandler::~HRFInternetASCIIHandler()
    {
    }

//-----------------------------------------------------------------------------
// Protected
// Finishes reading the current ASCII response
//-----------------------------------------------------------------------------
void HRFInternetASCIIHandler::ReadUntilMarker(HFCInternetConnection& pi_rConnection,
                                              HFCBuffer&             pio_rBuffer)
    {
    // read byte by byte until "\r\n"
    bool MarkerFound = false;
    Byte NextByte;
    while (!MarkerFound)
        {
        // Read the next byte
        pi_rConnection.Receive(&NextByte, 1);

        // add the current byte to the string
        pio_rBuffer.AddData(&NextByte, 1);

        // verify if the last characters of the string are "\r\n"
        if (pio_rBuffer.GetDataSize() > s_Marker.size())
            {
            size_t StartPos = pio_rBuffer.GetDataSize() - s_Marker.size();
            MarkerFound = (memcmp(pio_rBuffer.GetData() + StartPos,
                                  s_Marker.data(),
                                  s_Marker.size()) == 0);
            }
        }
    }
