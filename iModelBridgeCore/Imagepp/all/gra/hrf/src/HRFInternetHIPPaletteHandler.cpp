//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetHIPPaletteHandler.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFInternetHIPPaletteHandler
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetHIPPaletteHandler.h>
#include <Imagepp/all/h/HRFInternetImagingFile.h>
#include <Imagepp/all/h/HRFInternetImagingException.h>
#include <Imagepp/all/h/HRPPixelPalette.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

const static string s_Label("hip-palette");


//-----------------------------------------------------------------------------
// Public
// Constructor - Do not use s_Label as the parameter of the parent class,
// because this object is contructed statically and the label may be empty
//-----------------------------------------------------------------------------
HRFInternetHIPPaletteHandler::HRFInternetHIPPaletteHandler()
    : HRFInternetBinaryHandler("hip-palette")
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetHIPPaletteHandler::~HRFInternetHIPPaletteHandler()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFInternetHIPPaletteHandler::Handle(HRFInternetImagingFile& pi_rFile,
                                          HFCBuffer&              pio_rBuffer,
                                          HFCInternetConnection&  pi_rConnection)
    {
    // Scan the connection until the binary data
    ReadUntilData(pi_rConnection, pio_rBuffer);

    //
    // Format:
    //
    //  hip-palette/size:<data>
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
                                    HRFIIHandlerException::HIP_PALETTE);

    // Set the palette in the internet image
    SetPixelPalette(pi_rFile,
                    pio_rBuffer.GetData(),
                    pio_rBuffer.GetDataSize());
    }


//-----------------------------------------------------------------------------
// Protected
//
//-----------------------------------------------------------------------------
void HRFInternetHIPPaletteHandler::SetPixelPalette(HRFInternetImagingFile& pi_rFile,
                                                   const Byte*            pi_pData,
                                                   size_t                  pi_DataSize)
    {
    HPRECONDITION(pi_pData != 0);
    HPRECONDITION(pi_DataSize > 0);

    // if the pixel types has already arrived, set the palette in it
    for (unsigned short Res = 0; Res < HRF_INTERNET_MAXIMUM_RESOLUTION; Res++)
        {
        // Pixel type must have arrived first
        HFCMonitor Monitor(pi_rFile.m_DataKey);
        HASSERT(pi_rFile.m_pPixelType[Res] != 0);

        // get the pixel type's palette
        HRPPixelPalette* pPalette = &pi_rFile.m_pPixelType[Res]->LockPalette();
        if (pPalette->GetMaxEntries() > 0)
            {
            // Get the number of entries
            uint32_t EntryCount = 0;
            memcpy(&EntryCount, pi_pData, 4);
            EntryCount = ntohl(EntryCount);

            // Get the entry size
            uint32_t EntrySize = 0;
            memcpy(&EntrySize, pi_pData + 4, 4);
            EntrySize = ntohl(EntrySize);

            // Fill the Palette
            const Byte* pEntry = pi_pData + 8;
            for (uint32_t Entry = 0; Entry < EntryCount; Entry++)
                {
                if (Entry < pPalette->CountUsedEntries())
                    pPalette->SetCompositeValue(Entry, pEntry);
                else
                    pPalette->AddEntry(pEntry);
                pEntry += EntrySize;
                }
            }

        // Release the pixel type palette
        pi_rFile.m_pPixelType[Res]->UnlockPalette();
        }
    }


