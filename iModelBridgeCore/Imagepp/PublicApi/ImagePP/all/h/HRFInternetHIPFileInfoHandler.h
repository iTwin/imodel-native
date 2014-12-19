//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFInternetHIPFileInfoHandler.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetHIPFileInfoHandler
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

#pragma once

#include "HRFInternetASCIIHandler.h"

class HRFInternetHIPFileInfoHandler : public HRFInternetASCIIHandler
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HRFInternetHIPFileInfoHandler();
    virtual         ~HRFInternetHIPFileInfoHandler();


    //--------------------------------------
    // Handling
    //--------------------------------------

    // Handle the rest of the data
    virtual void    Handle(HRFInternetImagingFile& pi_rFile,
                           HFCBuffer&              pio_rBuffer,
                           HFCInternetConnection&  pi_rConnection);
    };

