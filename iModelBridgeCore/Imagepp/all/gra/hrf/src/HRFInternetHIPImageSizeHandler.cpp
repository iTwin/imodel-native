//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetHIPImageSizeHandler.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetHIPImageSizeHandler
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetHIPImageSizeHandler.h>
#include <Imagepp/all/h/HRFInternetImagingFile.h>


//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

const static string s_Label("hip-image-size");


//-----------------------------------------------------------------------------
// Public
// Constructor - Do not use s_Label as the parameter of the parent class,
// because this object is contructed statically and the label may be empty
//-----------------------------------------------------------------------------
HRFInternetHIPImageSizeHandler::HRFInternetHIPImageSizeHandler()
    : HRFInternetASCIIHandler("hip-image-size")
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetHIPImageSizeHandler::~HRFInternetHIPImageSizeHandler()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFInternetHIPImageSizeHandler::Handle(HRFInternetImagingFile& pi_rFile,
                                            HFCBuffer&              pio_rBuffer,
                                            HFCInternetConnection&  pi_rConnection)
    {
    // Scan the connection untilt the end marker
    ReadUntilMarker(pi_rConnection, pio_rBuffer);

    //
    // The format of the response is
    //
    // hip-image-size,Res,Sub:Width Height
    //

    // with protocol 1.4, the resolution can be -1 or the current resolution factor.
    // Place the buffer in a string in order to force a '\0' at the end in order to avoid an access violation
    string Response((const char*)pio_rBuffer.GetData(), pio_rBuffer.GetDataSize());

    if (pi_rFile.m_IsUnlimitedResolution)
        {
        string::size_type CommaPos = Response.find(",", s_Label.size() + 1);    // skip "hip-tile-size,"
        if (CommaPos == string::npos)
            throw HRFIIHandlerException(HRFII_INVALID_DATA_FORMAT_EXCEPTION,
                                        pi_rFile.GetURL()->GetURL(),
                                        HRFIIHandlerException::HIP_IMAGE_SIZE);

        string Res(Response.substr(s_Label.size() + 1, CommaPos - s_Label.size() - 1));

        uint32_t SubImage;
        uint32_t Width;
        uint32_t Height;
        if (sscanf(Response.c_str() + CommaPos + 1,  "%lu:%lu %lu\r\n", &SubImage, &Width, &Height) != 3)
            throw HRFIIHandlerException(HRFII_INVALID_DATA_FORMAT_EXCEPTION,
                                        pi_rFile.GetURL()->GetURL(),
                                        HRFIIHandlerException::HIP_IMAGE_SIZE);

        if (Res.compare("-1") == 0)
            {
            // Set the tile size
            HFCMonitor Monitor(pi_rFile.m_DataKey);
            HPRECONDITION(pi_rFile.m_UnlimitedResolutionMap.size() > 0);

            pi_rFile.m_ImageWidth[0] = Width;
            pi_rFile.m_ImageHeight[0] = Height;
            }
        else
            {
            HFCMonitor Monitor(pi_rFile.m_DataKey);
            HRFInternetImagingFile::UnlimitedResolutionMap::iterator Itr(pi_rFile.m_UnlimitedResolutionMap.find(Res));
            if (Itr == pi_rFile.m_UnlimitedResolutionMap.end())
                throw HRFIIHandlerException(HRFII_INVALID_RESOLUTION_VALUE_EXCEPTION,
                                            pi_rFile.GetURL()->GetURL(),
                                            HRFIIHandlerException::HIP_IMAGE_SIZE);



            pi_rFile.m_ImageWidth[Itr->second] = Width;
            pi_rFile.m_ImageHeight[Itr->second] = Height;

            pi_rFile.m_ImageSizeHandlerEvent.Signal();
            }
        }
    else
        {
        uint32_t ResIndex;
        uint32_t SubImage;
        uint32_t Width;
        uint32_t Height;

        if (sscanf(Response.c_str() + s_Label.size() + 1,  // skip "hip-tile-size,"
                   "%lu,%lu:%lu %lu\r\n",
                   &ResIndex,
                   &SubImage,
                   &Width,
                   &Height) != 4)
            throw HRFIIHandlerException(HRFII_INVALID_DATA_FORMAT_EXCEPTION,
                                        pi_rFile.GetURL()->GetURL(),
                                        HRFIIHandlerException::HIP_IMAGE_SIZE);

        if (ResIndex >= HRF_INTERNET_MAXIMUM_RESOLUTION)
            throw HRFIIHandlerException(HRFII_INVALID_RESOLUTION_VALUE_EXCEPTION,
                                        pi_rFile.GetURL()->GetURL(),
                                        HRFIIHandlerException::HIP_IMAGE_SIZE);

        HFCMonitor Monitor(pi_rFile.m_DataKey);
        pi_rFile.m_ImageWidth[ResIndex] = Width;
        pi_rFile.m_ImageHeight[ResIndex] = Height;

        pi_rFile.m_ImageSizeHandlerEvent.Signal();
        }
    }