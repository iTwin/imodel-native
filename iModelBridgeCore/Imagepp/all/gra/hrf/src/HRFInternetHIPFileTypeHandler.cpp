//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetHIPFileTypeHandler.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetHIPFileTypeHandler
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetHIPFileTypeHandler.h>
#include <Imagepp/all/h/HRFInternetImagingFile.h>
#include <Imagepp/all/h/HFCEncodeDecodeASCII.h>


//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

const static string s_Label("hip-file-type");


//-----------------------------------------------------------------------------
// Public
// Constructor - Do not use s_Label as the parameter of the parent class,
// because this object is contructed statically and the label may be empty
//-----------------------------------------------------------------------------
HRFInternetHIPFileTypeHandler::HRFInternetHIPFileTypeHandler()
    : HRFInternetASCIIHandler("hip-file-type")
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetHIPFileTypeHandler::~HRFInternetHIPFileTypeHandler()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFInternetHIPFileTypeHandler::Handle(HRFInternetImagingFile& pi_rFile,
                                           HFCBuffer&              pio_rBuffer,
                                           HFCInternetConnection&  pi_rConnection)
    {
    // Scan the connection untilt the end marker
    ReadUntilMarker(pi_rConnection, pio_rBuffer);

    //
    // The format of the response is
    //
    // hip-file-type:file type string
    //

    // Find the first coma and the value is everything afterwards
    string Response((const char*)pio_rBuffer.GetData(), pio_rBuffer.GetDataSize());
    string::size_type Pos = Response.find(',');
    if (Pos == string::npos)
        throw HRFIIHandlerException(HRFII_INVALID_DATA_FORMAT_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::HIP_FILE_TYPE);

    // Copy the value and remove the ending marker if any
    string Value(Response.substr(Pos + 1, Response.size() - Pos - 1));
    while ((Pos = Value.find(s_Marker)) != string::npos)
        Value.erase(Pos, s_Marker.size());


    // Remove the escape sequence from the string
    HFCEncodeDecodeASCII::EscapeToASCII(Value);

    // The second token is now the file type from the server
    HFCMonitor Monitor(pi_rFile.m_DataKey);
    if (pi_rFile.GetProtocolVersion().GetNumber(2) == HRFInternetImagingFile::s_HIP_UTF8_Protocol)
        BeStringUtilities::Utf8ToWChar(pi_rFile.m_OriginalFileType,Value.c_str());
    else
        BeStringUtilities::CurrentLocaleCharToWChar( pi_rFile.m_OriginalFileType,Value.c_str());


    pi_rFile.m_OriginalFileTypeEvent.Signal();
    }

