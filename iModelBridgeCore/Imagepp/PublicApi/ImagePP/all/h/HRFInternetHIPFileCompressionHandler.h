//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFInternetHIPFileCompressionHandler.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetHIPFileCompressionHandler
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

#pragma once

#include "HRFInternetBinaryHandler.h"

class HRFInternetHIPFileCompressionHandler : public HRFInternetBinaryHandler
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HRFInternetHIPFileCompressionHandler();
    virtual         ~HRFInternetHIPFileCompressionHandler();


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

    void            AddCompression(HRFInternetImagingFile& pi_rFile,
                                   const HFCBuffer&        pio_rBuffer);
    };

