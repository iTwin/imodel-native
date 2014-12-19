//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetHIPVersionHandler.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFInternetHIPVersionHandler
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetHIPVersionHandler.h>
#include <Imagepp/all/h/HRFInternetImagingFile.h>
#include <Imagepp/all/h/HRFInternetImagingException.h>
#include <Imagepp/all/h/HFCVersion.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

const static string s_Label("hip:");    // the colon helps to differentiate from hip-server


//-----------------------------------------------------------------------------
// Public
// Constructor - Do not use s_Label as the parameter of the parent class,
// because this object is contructed statically and the label may be empty
//-----------------------------------------------------------------------------
HRFInternetHIPVersionHandler::HRFInternetHIPVersionHandler()
    : HRFInternetASCIIHandler("hip:")   // the colon helps to differentiate from hip-server
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetHIPVersionHandler::~HRFInternetHIPVersionHandler()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFInternetHIPVersionHandler::Handle(HRFInternetImagingFile& pi_rFile,
                                          HFCBuffer&              pio_rBuffer,
                                          HFCInternetConnection&  pi_rConnection)
    {
    // Scan the connection until the end marker
    ReadUntilMarker(pi_rConnection, pio_rBuffer);

    HPOSTCONDITION(pio_rBuffer.GetDataSize() > s_Label.size() + s_Marker.size());

    //
    // The format of the IIP response is
    //
    // iip:major.minor[,utf8]
    //
    uint32_t Major, Minor;
    bool UTF8 = false;

    // Place the buffer in a string in order to force a '\0' at the end in order to avoid an access violation
    string Response((const char*)pio_rBuffer.GetData() + s_Label.size(), pio_rBuffer.GetDataSize() - s_Label.size() - s_Marker.size());
    if (sscanf(Response.c_str(), "%lu.%lu", &Major, &Minor) != 2)
        throw HRFIIHandlerException(HRFII_INVALID_DATA_FORMAT_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::HIP_VERSION);

    ostringstream Version;
    Version << Major << "." << Minor;
    if (Version.str() != Response)
        {
        // check if the protocol is utf8
        Version << ",utf8";

        if (BeStringUtilities::Stricmp(Response.c_str(), Version.str().c_str()) == 0)
            UTF8 = true;
        else
            throw HRFInternetImagingException(HRFII_INVALID_PROTOCOL_CHARSET_EXCEPTION, pi_rFile.GetURL()->GetURL());
        }

    // Set the protocol version
        {
        HFCMonitor Monitor(pi_rFile.m_DataKey);
        pi_rFile.m_Version = HFCVersion(WString(L""),
                                        WString(L""),
                                        3,
                                        Major,
                                        Minor,
                                        (UTF8 ? HRFInternetImagingFile::s_HIP_UTF8_Protocol : HRFInternetImagingFile::s_HIP_Protocol));
        }

    // Assume we are connected so that the main thread may continue
//    pi_rFile.SetState(HRFInternetImagingFile::CONNECTING_HIP);
    }
