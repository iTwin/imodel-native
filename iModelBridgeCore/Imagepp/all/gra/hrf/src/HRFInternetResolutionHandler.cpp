//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetResolutionHandler.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetResolutionHandler
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetResolutionHandler.h>
#include <Imagepp/all/h/HRFInternetImagingFile.h>
#include <Imagepp/all/h/HRFInternetImagingException.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

const static string s_Label("resolution-number");


//-----------------------------------------------------------------------------
// Public
// Constructor - Do not use s_Label as the parameter of the parent class,
// because this object is contructed statically and the label may be empty
//-----------------------------------------------------------------------------
HRFInternetResolutionHandler::HRFInternetResolutionHandler()
    : HRFInternetASCIIHandler("resolution-number")
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetResolutionHandler::~HRFInternetResolutionHandler()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFInternetResolutionHandler::Handle(HRFInternetImagingFile& pi_rFile,
                                          HFCBuffer&              pio_rBuffer,
                                          HFCInternetConnection&  pi_rConnection)
    {
    // Scan the connection untilt the end marker
    ReadUntilMarker(pi_rConnection, pio_rBuffer);

    //
    // The format of the resolution-number response is
    //
    // resolution-number:number
    //
    // with HIP 1.4, resolution number can be -1, this means that the
    // raster file is an unlimited resolution
    int32_t ResolutionCount = atol((const char*)pio_rBuffer.GetData() + s_Label.size() + 1);

    if ((ResolutionCount == 0) || (ResolutionCount >= HRF_INTERNET_MAXIMUM_RESOLUTION) || ResolutionCount < -1)
        throw HRFIIHandlerException(HRFII_INVALID_VALUE_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::RESOLUTION);

    // Set the resolution number
    HFCMonitor Monitor(pi_rFile.m_DataKey);
    if (ResolutionCount == -1)
        {
        pi_rFile.m_IsUnlimitedResolution = true;
        pi_rFile.m_ResolutionCount = 1;
        // insert the main resolution in the map
        HPRECONDITION(pi_rFile.m_UnlimitedResolutionMap.size() == 0);
        pi_rFile.m_UnlimitedResolutionMap.insert(HRFInternetImagingFile::UnlimitedResolutionMap::value_type("1.0", 0));
        pi_rFile.m_UsedResIndexList.push_front(0);
        }
    else
        {
        pi_rFile.m_IsUnlimitedResolution = false;
        pi_rFile.m_ResolutionCount = (unsigned short)ResolutionCount;
        }
    }
