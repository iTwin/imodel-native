//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetCompGroupHandler.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFInternetCompGroupHandler
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetCompGroupHandler.h>
#include <Imagepp/all/h/HRFInternetImagingFile.h>
#include <Imagepp/all/h/HRFInternetImaging.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

const static string s_Label("comp-group");


//-----------------------------------------------------------------------------
// Public
// Constructor - Do not use s_Label as the parameter of the parent class,
// because this object is contructed statically and the label may be empty
//-----------------------------------------------------------------------------
HRFInternetCompGroupHandler::HRFInternetCompGroupHandler()
    : HRFInternetBinaryHandler("comp-group")
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetCompGroupHandler::~HRFInternetCompGroupHandler()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFInternetCompGroupHandler::Handle(HRFInternetImagingFile& pi_rFile,
                                         HFCBuffer&              pio_rBuffer,
                                         HFCInternetConnection&  pi_rConnection)
    {
    // Scan the connection until the binary data
    ReadUntilData(pi_rConnection, pio_rBuffer);

    //
    // Format:
    //
    //  comp-group,type,index/size:<data>
    //

    uint32_t Type, Index, DataSize;

    // Place the buffer in a string in order to force a '\0' at the end in order to avoid an access violation
    if (sscanf(string((const char*)pio_rBuffer.GetData(), pio_rBuffer.GetDataSize()).c_str() + s_Label.size() + 1,
               "%lu,%lu/%lu:",
               &Type,
               &Index,
               &DataSize) != 3)
        throw HRFIIHandlerException(HRFII_INVALID_DATA_FORMAT_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::COMP_GROUP);

    // Read the data in the buffer & skip end marker
    ReadData(pi_rConnection, pio_rBuffer, DataSize, true);
    SkipEndMarker(pi_rConnection);

    // verify validity
    if (DataSize == 0)
        throw HRFIIHandlerException(HRFII_INVALID_VALUE_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::COMP_GROUP);

    // if the compression group is JPEG, set the table
    if (Type == IIP_COMPRESSION_JPEG)
        {
        HFCMonitor Monitor(pi_rFile.m_DataKey);

        // verify that there is place in the jpoeg table vector
        if (Index + 1 > pi_rFile.m_JpegTables.size())
            pi_rFile.m_JpegTables.resize(Index + 1, 0);

        pi_rFile.m_JpegTables[Index] = new HRFInternetJpegTable();
        pi_rFile.m_JpegTables[Index]->SetData(pio_rBuffer.GetData(), (uint32_t)pio_rBuffer.GetDataSize());
        }
    }
