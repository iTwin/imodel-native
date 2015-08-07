//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetHIPBackgroundHandler.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetHIPBackgroundHandler
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetHIPBackgroundHandler.h>
#include <Imagepp/all/h/HRFInternetImagingFile.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

const static string s_Label("hip-background");


//-----------------------------------------------------------------------------
// Public
// Constructor - Do not use s_Label as the parameter of the parent class,
// because this object is contructed statically and the label may be empty
//-----------------------------------------------------------------------------
HRFInternetHIPBackgroundHandler::HRFInternetHIPBackgroundHandler()
    : HRFInternetASCIIHandler("hip-background")
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetHIPBackgroundHandler::~HRFInternetHIPBackgroundHandler()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFInternetHIPBackgroundHandler::Handle(HRFInternetImagingFile& pi_rFile,
                                             HFCBuffer&              pio_rBuffer,
                                             HFCInternetConnection&  pi_rConnection)
    {
    // Scan the connection untilt the end marker
    ReadUntilMarker(pi_rConnection, pio_rBuffer);

    //
    // The format of the response is
    //
    // hip-background:background
    //
    uint32_t Background;
    uint32_t unused;

    // Place the buffer in a string in order to force a '\0' at the end in order to avoid an access violation
    if (sscanf(string((const char*)pio_rBuffer.GetData(), pio_rBuffer.GetDataSize()).c_str() + s_Label.size() + 1,
               "%lu:%lu\r\n",
               &Background, &unused) != 1)
        throw HRFIIHandlerException(HRFII_INVALID_DATA_FORMAT_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::HIP_BACKGROUND);

    // Set the image size for this resolution
    HFCMonitor Monitor(pi_rFile.m_DataKey);
    pi_rFile.m_HasBackground = true;
    pi_rFile.m_Background    = Background;
    }
