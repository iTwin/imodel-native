//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetHIPFileInfoHandler.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetHIPFileInfoHandler
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetHIPFileInfoHandler.h>
#include <Imagepp/all/h/HRFInternetImagingFile.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

const static string s_Label("hip-file-info");


//-----------------------------------------------------------------------------
// Public
// Constructor - Do not use s_Label as the parameter of the parent class,
// because this object is contructed statically and the label may be empty
//-----------------------------------------------------------------------------
HRFInternetHIPFileInfoHandler::HRFInternetHIPFileInfoHandler()
    : HRFInternetASCIIHandler("hip-file-info")
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetHIPFileInfoHandler::~HRFInternetHIPFileInfoHandler()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFInternetHIPFileInfoHandler::Handle(HRFInternetImagingFile& pi_rFile,
                                           HFCBuffer&              pio_rBuffer,
                                           HFCInternetConnection&  pi_rConnection)
    {
    // Scan the connection untilt the end marker
    ReadUntilMarker(pi_rConnection, pio_rBuffer);

    //
    // The format of the response is
    //
    // hip-file-info:creation modify access size
    //

    time_t Creation, Modify, Access;
    uint64_t Size;

    // Place the buffer in a string in order to force a '\0' at the end in order to avoid an access violation
    if (sscanf(string((const char*)pio_rBuffer.GetData(), pio_rBuffer.GetDataSize()).c_str() + s_Label.size() + 1,
               "%I64u %I64u %I64u %I64u\r\n",
               &Creation,
               &Modify,
               &Access,
               &Size) != 4)
        throw HRFIIHandlerException(HRFII_INVALID_DATA_FORMAT_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::HIP_FILE_INFO);

    HFCMonitor Monitor(pi_rFile.m_DataKey);

    // Set the creation time
    pi_rFile.m_CreationTime = Creation;

    // Set the modify time
    if (Modify < 0)
        {
        pi_rFile.m_ModifyTime = 0;
        pi_rFile.m_TimeStamp = string("");  // this field is not used
        }
    else
        {
        pi_rFile.m_ModifyTime = Modify;

        // Set the time stamp from the modify time
        pi_rFile.m_TimeStamp = string(ctime(&Modify));  
        }

    // Set the last access time
    if (Access < 0)
        pi_rFile.m_AccessTime = 0;
    else
        {
        pi_rFile.m_AccessTime = Access;
        }

    // Set the file size
    pi_rFile.m_FileSize = Size;
    }
