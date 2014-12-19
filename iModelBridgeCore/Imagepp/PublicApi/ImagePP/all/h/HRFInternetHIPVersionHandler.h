//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFInternetHIPVersionHandler.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetHIPVersionHandler
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

#pragma once

#include "HRFInternetASCIIHandler.h"

class HRFInternetHIPVersionHandler : public HRFInternetASCIIHandler
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HRFInternetHIPVersionHandler();
    virtual         ~HRFInternetHIPVersionHandler();


    //--------------------------------------
    // Handling
    //--------------------------------------

    // Handle the rest of the data
    virtual void    Handle(HRFInternetImagingFile& pi_rFile,
                           HFCBuffer&              pio_rBuffer,
                           HFCInternetConnection&  pi_rConnection);
    };

