//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetIIPVersionHandler.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFInternetIIPVersionHandler
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetIIPVersionHandler.h>
#include <Imagepp/all/h/HRFInternetImagingFile.h>
#include <Imagepp/all/h/HRFInternetImagingException.h>
#include <Imagepp/all/h/HFCVersion.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

const static string s_Label("iip:");    // the colon helps to differentiate from iip-server


//-----------------------------------------------------------------------------
// Public
// Constructor - Do not use s_Label as the parameter of the parent class,
// because this object is contructed statically and the label may be empty
//-----------------------------------------------------------------------------
HRFInternetIIPVersionHandler::HRFInternetIIPVersionHandler()
    : HRFInternetASCIIHandler("iip:")   // the colon helps to differentiate from iip-server
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetIIPVersionHandler::~HRFInternetIIPVersionHandler()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFInternetIIPVersionHandler::Handle(HRFInternetImagingFile& pi_rFile,
                                          HFCBuffer&              pio_rBuffer,
                                          HFCInternetConnection&  pi_rConnection)
    {
    // Scan the connection untilt the end marker
    ReadUntilMarker(pi_rConnection, pio_rBuffer);

    //
    // The format of the IIP response is
    //
    // iip:major.minor
    //
    uint32_t Major, Minor;

    // Place the buffer in a string in order to force a '\0' at the end in order to avoid an access violation
    if (sscanf(string((const char*)pio_rBuffer.GetData(), pio_rBuffer.GetDataSize()).c_str() + s_Label.size(),
               "%lu.%lu",
               &Major,
               &Minor) != 2)
        throw HRFIIHandlerException(HRFII_INVALID_DATA_FORMAT_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::IIP_VERSION);

    // Set the protocol version
        {
        HFCMonitor Monitor(pi_rFile.m_DataKey);
        pi_rFile.m_Version = HFCVersion(WString(L""),
                                        WString(L""),
                                        3,
                                        Major,
                                        Minor,
                                        HRFInternetImagingFile::s_IIP_Protocol);
        }

    // Assume we are connected so that the main thread may continue
//    pi_rFile.SetState(HRFInternetImagingFile::CONNECTING_IIP);
    }
