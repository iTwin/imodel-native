//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetHIPFileCompressionHandler.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetHIPFileCompressionHandler
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetHIPFileCompressionHandler.h>
#include <Imagepp/all/h/HRFInternetImagingFile.h>
#include <ImagePP/all/h/HRFInternetImaging.h>

// Codec Objects
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecZlib.h>
#include <Imagepp/all/h/HCDCodecHMRPackBits.h>
#include <Imagepp/all/h/HCDCodecFPXSingleColor.h>
#include <Imagepp/all/h/HCDCodecFlashpix.h>
#include <Imagepp/all/h/HCDCodecHMRCCITT.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>
#include <Imagepp/all/h/HCDCodecJPEGAlpha.h>
#ifdef __HMR_SUMMUS_SUPPORTED
//#include "HCDCodecSummus.h"
#endif

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

const static string s_Label("hip-file-compression");


//-----------------------------------------------------------------------------
// Public
// Constructor - Do not use s_Label as the parameter of the parent class,
// because this object is contructed statically and the label may be empty
//-----------------------------------------------------------------------------
HRFInternetHIPFileCompressionHandler::HRFInternetHIPFileCompressionHandler()
    : HRFInternetBinaryHandler("hip-file-compression")
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetHIPFileCompressionHandler::~HRFInternetHIPFileCompressionHandler()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFInternetHIPFileCompressionHandler::Handle(HRFInternetImagingFile& pi_rFile,
                                                  HFCBuffer&              pio_rBuffer,
                                                  HFCInternetConnection&  pi_rConnection)
    {
    // Scan the connection untilt the binary data
    ReadUntilData(pi_rConnection, pio_rBuffer);

    //
    // Format:
    //
    //  hip-file-compression/size:data
    //

    // Read the data size
    uint32_t DataSize = atol((const char*)pio_rBuffer.GetData() + s_Label.size() + 1);

    // Read the data in the buffer & skip end marker
    ReadData(pi_rConnection, pio_rBuffer, DataSize, true);
    SkipEndMarker(pi_rConnection);

    // Verify validity
    if (DataSize == 0)
        throw HRFIIHandlerException(HRFII_INVALID_VALUE_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::HIP_FILE_COMPRESSION);

    // Add the compression
    AddCompression(pi_rFile, pio_rBuffer);
    }


//-----------------------------------------------------------------------------
// Private
//
//-----------------------------------------------------------------------------
void HRFInternetHIPFileCompressionHandler::AddCompression(HRFInternetImagingFile& pi_rFile,
                                                          const HFCBuffer&        pio_rBuffer)
    {
    HPRECONDITION(pio_rBuffer.GetDataSize() == 8);

    // Type used for analyzing the compression sub-type.  For certain compressions,
    // the type is an int and for others it is 4 bytes.
    union
        {
        uint32_t m_IntValue;
        Byte m_ByteValue[4];
        }                SubType;
    uint32_t          Type;

    // Get the compression info
    // The tile's compression data is the first 8 bytes of the data stream
    // after which it is the actual tile image.
    //
    // The compression data contains 2 unsigned 32-bit integers:
    //
    //  Compression Type    :   Indicates which compression is in use
    //  Compression SubType :   Information for the compression in use
    Type               = ((uint32_t*) pio_rBuffer.GetData())[0];
    SubType.m_IntValue = ((uint32_t*) pio_rBuffer.GetData())[1];


    // Find codec
    HFCPtr<HCDCodec> pCodec;
    switch (Type)
        {
            // No Compression
        case IIP_COMPRESSION_NONE:
            pCodec = new HCDCodecIdentity();
            break;

            // Single Color
        case IIP_COMPRESSION_SINGLE_COLOR:
            pCodec = new HCDCodecFPXSingleColor();
            break;

            // Jpeg
        case IIP_COMPRESSION_JPEG:
            pCodec = new HCDCodecFlashpix();
            break;

        case HIP_COMPRESSION_JPEG_RLE8:
            pCodec = new HCDCodecJPEGAlpha();
            break;

#ifdef __HMR_SUMMUS_SUPPORTED
            // Wavelet
        case HIP_COMPRESSION_WAVELET:
            pCodec = new HCDCodecSummus();
            break;
#endif

            // Deflate
        case HIP_COMPRESSION_DEFLATE:
            pCodec = new HCDCodecZlib();
            break;

            // Packbits
        case HIP_COMPRESSION_PACKBITS:
            pCodec = new HCDCodecHMRPackBits();
            break;

            // CCITT3, CCITT4
        case HIP_COMPRESSION_CCITT3:
        case HIP_COMPRESSION_CCITT4:
            pCodec = new HCDCodecHMRCCITT();
            break;

            // RLE
        case HIP_COMPRESSION_RLE:
            pCodec = new HCDCodecHMRRLE1();
            break;
        }

    // Must be a valid codec
    if (pCodec == 0)
        throw HRFIIHandlerException(HRFII_INVALID_VALUE_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::HIP_FILE_COMPRESSION);

    // add the codec to the codec list
    HFCMonitor Monitor(pi_rFile.m_DataKey);
    pi_rFile.m_pCodec = pCodec;
    }

