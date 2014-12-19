//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetHIPFileHandler.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFInternetHIPFileHandler
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetHIPFileHandler.h>
#include <Imagepp/all/h/HRFInternetImagingFile.h>
#include <Imagepp/all/h/HRFInternetImaging.h>
#include <Imagepp/all/h/HRFException.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

const static string s_Label("hip-file/");   // to differentiate from hip-file-info
const static int32_t s_ReadSize = 4096;


//-----------------------------------------------------------------------------
// Public
// Constructor - Do not use s_Label as the parameter of the parent class,
// because this object is contructed statically and the label may be empty
//-----------------------------------------------------------------------------
HRFInternetHIPFileHandler::HRFInternetHIPFileHandler()
    : HRFInternetBinaryHandler("hip-file/") // to differentiate from hip-file-info
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetHIPFileHandler::~HRFInternetHIPFileHandler()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFInternetHIPFileHandler::Handle(HRFInternetImagingFile& pi_rFile,
                                       HFCBuffer&              pio_rBuffer,
                                       HFCInternetConnection&  pi_rConnection)
    {
    // Scan the connection until the binary data
    ReadUntilData(pi_rConnection, pio_rBuffer);

    //
    // Format:
    //
    //      hip-file/Length:<data>\r\n
    //

    // Extract the size of the file
    size_t DataSize = atol((const char*)pio_rBuffer.GetData() + s_Label.size());
    if (DataSize == 0)
        {
        SkipEndMarker(pi_rConnection);
        throw HRFIIHandlerException(HRFII_INVALID_VALUE_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::HIP_FILE);
        }

    // Change to read/write before writing
    _wchmod(pi_rFile.m_LocalCachedFileName.c_str(), _S_IREAD | _S_IWRITE);

    // Open a new file
    FILE* pFile = _wfopen(pi_rFile.m_LocalCachedFileName.c_str(), L"w+b");
    if (pFile == 0)
        {
        SkipData(pi_rConnection, DataSize);
        SkipEndMarker(pi_rConnection);
        throw HFCFileException(HFC_CANNOT_OPEN_FILE_EXCEPTION, pi_rFile.m_LocalCachedFileName);
        }

    // while there is data, read it and write to the file
    Byte Buffer[s_ReadSize];
    while (DataSize > 0)
        {
        // read a chunk of data
        size_t ReadSize = min(DataSize, s_ReadSize);
        pi_rConnection.Receive((Byte*)Buffer, &ReadSize);

        // write it to the file
        size_t WriteSize;
        if ((WriteSize = fwrite(Buffer, 1, ReadSize, pFile)) != ReadSize)
            {
            SkipData(pi_rConnection, DataSize - WriteSize);
            SkipEndMarker(pi_rConnection);
            throw HFCFileException(HFC_WRITE_FAULT_EXCEPTION, pi_rFile.m_LocalCachedFileName);
            }

        // Decrement the count
        DataSize -= ReadSize;
        }

    // read the end marker
    SkipEndMarker(pi_rConnection);

    // close the file
    if (fclose(pFile) != 0)
        throw HFCFileException(HFC_CANNOT_CLOSE_FILE_EXCEPTION,
                               pi_rFile.m_LocalCachedFileName);
    pFile = NULL;

    // set the image with the time stamp and Read-Only
    struct _utimbuf uTime;
    uTime.actime  = (time_t)pi_rFile.m_ModifyTime;
    uTime.modtime = (time_t)pi_rFile.m_ModifyTime;
    _wutime(pi_rFile.m_LocalCachedFileName.c_str(), &uTime);
    _wchmod(pi_rFile.m_LocalCachedFileName.c_str(), _S_IREAD);

    // signal the the file is ready
    pi_rFile.m_LocalCacheFileEvent.Signal();
    }
