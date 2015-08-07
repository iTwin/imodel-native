//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetColorspaceHandler.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFInternetColorspaceHandler
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetColorspaceHandler.h>
#include <Imagepp/all/h/HRFInternetImagingFile.h>
#include <Imagepp/all/h/HRFInternetImaging.h>
#include <Imagepp/all/h/HRPPixelTypeGray.h>
#include <Imagepp/all/h/HRPPixelTypeRGB.h>
#include <Imagepp/all/h/HRPPixelTypeV24PhotoYCC.h>
#include <Imagepp/all/h/HRPPixelTypeV32PRPhotoYCCA8.h>
#include <Imagepp/all/h/HRPPixelTypeFactory.h>
#include <Imagepp/all/h/HRPChannelOrgGray.h>
#include <Imagepp/all/h/HRPChannelOrgRGB.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

const static string s_Label("colorspace");


//-----------------------------------------------------------------------------
// Public
// Constructor - Do not use s_Label as the parameter of the parent class,
// because this object is contructed statically and the label may be empty
//-----------------------------------------------------------------------------
HRFInternetColorspaceHandler::HRFInternetColorspaceHandler()
    : HRFInternetASCIIHandler("colorspace")
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetColorspaceHandler::~HRFInternetColorspaceHandler()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFInternetColorspaceHandler::Handle(HRFInternetImagingFile& pi_rFile,
                                          HFCBuffer&              pio_rBuffer,
                                          HFCInternetConnection&  pi_rConnection)
    {
    // Scan the connection untilt the end marker
    ReadUntilMarker(pi_rConnection, pio_rBuffer);

    //
    // The format of the resolution-number response is
    //
    // colorspace,res,subimage:calib pre colorspace numPla 1*(plane)
    //

    unsigned short Resolution;
    uint32_t SubImage, Calibrated, Opacity, Info, Plane1, Plane2, Plane3, Plane4;
    uint32_t NumPlanes=0;

    // Place the buffer in a string in order to force a '\0' at the end in order to avoid an access violation
    if (sscanf(string((const char*)pio_rBuffer.GetData(), pio_rBuffer.GetDataSize()).c_str() + s_Label.size() + 1,
               "%hu,%lu:%lu %lu %lu %lu %lu %lu %lu %lu",
               &Resolution,
               &SubImage,
               &Calibrated,
               &Opacity,
               &Info,
               &NumPlanes,
               &Plane1,
               &Plane2,
               &Plane3,
               &Plane4) < (int)(6 + NumPlanes))
        throw HRFIIHandlerException(HRFII_INVALID_DATA_FORMAT_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::COLOR_SPACE);

    if (Resolution >= HRF_INTERNET_MAXIMUM_RESOLUTION)
        throw HRFIIHandlerException(HRFII_INVALID_RESOLUTION_VALUE_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::COLOR_SPACE);

    // Build a pixel type
    HFCPtr<HRPPixelType> pPixelType;
    switch(Info)
        {
        case IIP_COLORSPACE_COLORLESS:
            HASSERT(NumPlanes == 1);
            HASSERT(Plane1 == IIP_COLORSPACE_FIELD_COLORLESS);
            pPixelType = HRPPixelTypeFactory::GetInstance()->Create(HRPChannelOrgGray(1,
                                                                                      HRPChannelType::UNUSED,
                                                                                      HRPChannelType::VOID_CH,
                                                                                      0),
                                                                    0);
            HASSERT(pPixelType != 0);
            break;

        case IIP_COLORSPACE_MONOCHROME:
            HASSERT(NumPlanes == 1);
            HASSERT(Plane1 == IIP_COLORSPACE_FIELD_MONOCHROME);
            pPixelType = HRPPixelTypeFactory::GetInstance()->Create(HRPChannelOrgGray(8,
                                                                                      HRPChannelType::UNUSED,
                                                                                      HRPChannelType::VOID_CH,
                                                                                      0),
                                                                    0);
            HASSERT(pPixelType != 0);
            break;

        case IIP_COLORSPACE_PHOTO_YCC:
            HASSERT((NumPlanes == 3) || (NumPlanes == 4));  // 4 includes opacity

            // NOTE: Analyze the planes to build the right pixel type
            // Just build YCC for now.
            if (NumPlanes == 3)
                pPixelType = new HRPPixelTypeV24PhotoYCC();
            else
                pPixelType = new HRPPixelTypeV32PRPhotoYCCA8();
            HASSERT(pPixelType != 0);
            break;

        case IIP_COLORSPACE_NIF_RGB:
            HASSERT((NumPlanes == 3) || (NumPlanes == 4));  // 4 includes opacity

            // NOTE: Analyze the planes to build the right pixel type
            // Just build RGB for now.
            if (NumPlanes == 3)
                pPixelType = HRPPixelTypeFactory::GetInstance()->Create(HRPChannelOrgRGB(8, 8, 8, 0, HRPChannelType::UNUSED,
                                                                                         HRPChannelType::VOID_CH,
                                                                                         0), 0);
            else
                pPixelType = HRPPixelTypeFactory::GetInstance()->Create(HRPChannelOrgRGB(8, 8, 8, 8, HRPChannelType::UNUSED,
                                                                                         HRPChannelType::VOID_CH,
                                                                                         0), 0);
            HASSERT(pPixelType != 0);
            break;
        }

    // Verify that there is a pixel type
    if (pPixelType == 0)
        throw HRFIIHandlerException(HRFII_INVALID_VALUE_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::COLOR_SPACE);

    // Set the pixel type in the internet image
    HFCMonitor Monitor(pi_rFile.m_DataKey);
    pi_rFile.m_pPixelType[Resolution] = pPixelType;
    }
