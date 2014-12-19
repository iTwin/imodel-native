//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetHIPPageNumberHandler.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetHIPPageNumberHandler
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetHIPPageNumberHandler.h>
#include <Imagepp/all/h/HRFInternetImagingFile.h>
#include <Imagepp/all/h/HRFInternetImagingException.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

const static string s_Label("hip-page-number");


//-----------------------------------------------------------------------------
// Public
// Constructor - Do not use s_Label as the parameter of the parent class,
// because this object is contructed statically and the label may be empty
//-----------------------------------------------------------------------------
HRFInternetHIPPageNumberHandler::HRFInternetHIPPageNumberHandler()
    : HRFInternetASCIIHandler("hip-page-number")
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetHIPPageNumberHandler::~HRFInternetHIPPageNumberHandler()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
// The format of the resolution-number response is
//
// page-number:number
//
//-----------------------------------------------------------------------------
void HRFInternetHIPPageNumberHandler::Handle(HRFInternetImagingFile& pi_rFile,
                                             HFCBuffer&              pio_rBuffer,
                                             HFCInternetConnection&  pi_rConnection)
    {
    // Scan the connection until the end marker
    ReadUntilMarker(pi_rConnection, pio_rBuffer);

    uint32_t PageCount = atol((const char*)pio_rBuffer.GetData() + s_Label.size() + 1);

    if (PageCount == 0)
        throw HRFIIHandlerException(HRFII_INVALID_VALUE_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::HIP_PAGE_NUMBER);

    // Set the resolution number
    HFCMonitor Monitor(pi_rFile.m_DataKey);

    HPRECONDITION(pi_rFile.m_PageCount == 0 || pi_rFile.m_PageCount == PageCount);
    pi_rFile.m_PageCount = PageCount;
    pi_rFile.m_MultiPageSupported = true;
    }
