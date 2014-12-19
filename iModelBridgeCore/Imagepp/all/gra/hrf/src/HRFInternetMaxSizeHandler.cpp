//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetMaxSizeHandler.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetMaxSizeHandler
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetMaxSizeHandler.h>
#include <Imagepp/all/h/HRFInternetImagingFile.h>
#include <Imagepp/all/h/HRFInternetImagingException.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

const static string s_Label("max-size");


//-----------------------------------------------------------------------------
// Public
// Constructor - Do not use s_Label as the parameter of the parent class,
// because this object is contructed statically and the label may be empty
//-----------------------------------------------------------------------------
HRFInternetMaxSizeHandler::HRFInternetMaxSizeHandler()
    : HRFInternetASCIIHandler("max-size")
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetMaxSizeHandler::~HRFInternetMaxSizeHandler()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFInternetMaxSizeHandler::Handle(HRFInternetImagingFile& pi_rFile,
                                       HFCBuffer&              pio_rBuffer,
                                       HFCInternetConnection&  pi_rConnection)
    {
    // Scan the connection untilt the end marker
    ReadUntilMarker(pi_rConnection, pio_rBuffer);

    // Scan the string for the values
    uint32_t Width, Height;

    // Place the buffer in a string in order to force a '\0' at the end in order to avoid an access violation
    if (sscanf(string((const char*)pio_rBuffer.GetData(), pio_rBuffer.GetDataSize()).c_str() + s_Label.size() + 1,
               "%lu %lu",
               &Width,
               &Height) != 2)
        throw HRFIIHandlerException(HRFII_INVALID_VALUE_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::MAX_SIZE);

    // Set the image sizes
    ComputeResolutionSize(pi_rFile, Width, Height);
    }


//-----------------------------------------------------------------------------
// Public
// Handles a resolution number response
//-----------------------------------------------------------------------------
uint32_t HRFInternetMaxSizeHandler::ComputeResolutionSize(HRFInternetImagingFile& pi_rFile,
                                                        uint32_t                pi_Width,
                                                        uint32_t                pi_Height)
    {
    uint32_t Result = 0;

    // if the current size is greater than 64 divide by 2 and compute the
    // resolution below
    if ((pi_Width/64  > 0) || (pi_Height/64 > 0))
        Result = 1 + ComputeResolutionSize(pi_rFile, (pi_Width+1)/2, (pi_Height+1)/2);

    // Set the image size for this resolution
    HFCMonitor Monitor(pi_rFile.m_DataKey);
    pi_rFile.m_ImageWidth[Result]  = pi_Width;
    pi_rFile.m_ImageHeight[Result] = pi_Height;
    pi_rFile.m_TileWidth[Result]   = 64;
    pi_rFile.m_TileHeight[Result]  = 64;

    return (Result);
    }
