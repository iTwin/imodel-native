//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFInternetHIPPaletteHandler.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetHIPPaletteHandler
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

#pragma once

#include "HRFInternetBinaryHandler.h"

class HRFInternetHIPPaletteHandler : public HRFInternetBinaryHandler
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HRFInternetHIPPaletteHandler();
    virtual         ~HRFInternetHIPPaletteHandler();


    //--------------------------------------
    // Handling
    //--------------------------------------

    // Handle the rest of the data
    virtual void    Handle(HRFInternetImagingFile& pi_rFile,
                           HFCBuffer&              pio_rBuffer,
                           HFCInternetConnection&  pi_rConnection);


private:
    //--------------------------------------
    // methods
    //--------------------------------------

    void            SetPixelPalette(HRFInternetImagingFile& pi_rFile,
                                    const Byte*            pi_pData,
                                    size_t                  pi_DataSize);
    };

