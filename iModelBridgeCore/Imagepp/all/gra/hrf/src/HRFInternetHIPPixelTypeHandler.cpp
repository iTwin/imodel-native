//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetHIPPixelTypeHandler.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFInternetHIPPixelTypeHandler
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetHIPPixelTypeHandler.h>
#include <Imagepp/all/h/HRFInternetImagingFile.h>
#include <Imagepp/all/h/HRFInternetImagingException.h>
#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HRPPixelTypeFactory.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

const static string s_Label("hip-pixel-type");


//-----------------------------------------------------------------------------
// Public
// Constructor - Do not use s_Label as the parameter of the parent class,
// because this object is contructed statically and the label may be empty
//-----------------------------------------------------------------------------
HRFInternetHIPPixelTypeHandler::HRFInternetHIPPixelTypeHandler()
    : HRFInternetBinaryHandler("hip-pixel-type")
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetHIPPixelTypeHandler::~HRFInternetHIPPixelTypeHandler()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFInternetHIPPixelTypeHandler::Handle(HRFInternetImagingFile& pi_rFile,
                                            HFCBuffer&              pio_rBuffer,
                                            HFCInternetConnection&  pi_rConnection)
    {
    // Scan the connection untilt the binary data
    ReadUntilData(pi_rConnection, pio_rBuffer);

    //
    // Format:
    //
    //  hip-pixel-type/size:data
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
                                    HRFIIHandlerException::HIP_PIXEL_TYPE);

    // Create a channel org from the fingerprint data

    // Calculate size of buffer to allocate for this fingerprint:
    //     1 byte                               for hash value
    //     sizeof(UShort)                      for index bits
    //     nb channels * channel size           for channels
    size_t ChannelSize = sizeof(HRPChannelType::ChannelRole) +
                         sizeof(HRPChannelType::DataType) +
                         sizeof(unsigned short) +
                         sizeof(unsigned short);

    unsigned short NumberOfChannels = (unsigned short)((pio_rBuffer.GetDataSize() - 1 - sizeof(unsigned short)) / ChannelSize);


    // position the data at the beginning of the channel org structure
    const Byte* pData = pio_rBuffer.GetData();
    pData += (1 + sizeof(unsigned short));

    HRPChannelOrg ChannelOrg;

    // extract channel org info and store it in the channel org object
    for(unsigned short ChannelIndex=0; ChannelIndex < NumberOfChannels; ChannelIndex++)
        {
        HRPChannelType::ChannelRole Role = *((HRPChannelType::ChannelRole*)pData);
        pData += sizeof(HRPChannelType::ChannelRole);

        HRPChannelType::DataType DataType = *((HRPChannelType::DataType*)pData);
        pData +=  sizeof(HRPChannelType::DataType);

        unsigned short Size = *((unsigned short*)pData);
        pData += sizeof(unsigned short);

        unsigned short Id = *((unsigned short*)pData);
        pData += sizeof(unsigned short);

        ChannelOrg.AddChannel(HRPChannelType(Role, DataType, Size, Id));
        }

    uint32_t IndexBits = *((unsigned short*)(pio_rBuffer.GetData() + 1));

    // Create the Pixel TYpe
    HFCPtr<HRPPixelType> pPixelType = HRPPixelTypeFactory::GetInstance()->Create(ChannelOrg, (unsigned short)IndexBits);
    if (pPixelType == 0)
        throw HRFException(HRF_PIXEL_TYPE_NOT_SUPPORTED_EXCEPTION, pi_rFile.GetURL()->GetURL());

    // Set the pixel type in the internet image
    // NOTE!!!: Since the pixel type is for all resolution, set the pixel type for all
    for (unsigned short Res = 0; Res < HRF_INTERNET_MAXIMUM_RESOLUTION; Res++)
        {
        HFCMonitor Monitor(pi_rFile.m_DataKey);

        // set the pixel type
        pi_rFile.m_pPixelType[Res] = pPixelType;
        }
    }
