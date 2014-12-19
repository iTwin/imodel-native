//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFInternetIIPVersionHandler.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetIIPVersionHandler
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

#pragma once

#include "HRFInternetASCIIHandler.h"

class HRFInternetIIPVersionHandler : public HRFInternetASCIIHandler
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HRFInternetIIPVersionHandler();
    virtual         ~HRFInternetIIPVersionHandler();


    //--------------------------------------
    // Handling
    //--------------------------------------

    // Handle the rest of the data
    virtual void    Handle(HRFInternetImagingFile& pi_rFile,
                           HFCBuffer&              pio_rBuffer,
                           HFCInternetConnection&  pi_rConnection);
    };

