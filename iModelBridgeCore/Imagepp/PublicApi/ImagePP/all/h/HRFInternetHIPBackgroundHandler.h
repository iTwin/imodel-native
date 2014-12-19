//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFInternetHIPBackgroundHandler.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetHIPBackgroundHandler
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

#pragma once

#include "HRFInternetASCIIHandler.h"

class HRFInternetHIPBackgroundHandler : public HRFInternetASCIIHandler
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HRFInternetHIPBackgroundHandler();
    virtual         ~HRFInternetHIPBackgroundHandler();


    //--------------------------------------
    // Handling
    //--------------------------------------

    // Handle the rest of the data
    virtual void    Handle(HRFInternetImagingFile& pi_rFile,
                           HFCBuffer&              pio_rBuffer,
                           HFCInternetConnection&  pi_rConnection);
    };

