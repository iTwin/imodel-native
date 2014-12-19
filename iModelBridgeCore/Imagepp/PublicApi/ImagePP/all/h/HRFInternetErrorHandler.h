//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFInternetErrorHandler.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFInternetErrorHandler
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

#pragma once

#include "HRFInternetBinaryHandler.h"

class HRFInternetErrorHandler : public HRFInternetBinaryHandler
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HRFInternetErrorHandler();
    virtual         ~HRFInternetErrorHandler();


    //--------------------------------------
    // Handling
    //--------------------------------------

    // Handle the rest of the data
    virtual void    Handle(HRFInternetImagingFile& pi_rFile,
                           HFCBuffer&              pio_rBuffer,
                           HFCInternetConnection&  pi_rConnection);


private:
    //--------------------------------------
    // Methods
    //--------------------------------------
    void            SetError            (HRFInternetImagingFile& pi_rFile,
                                         uint32_t                pi_ErrorClass,
                                         uint32_t                pi_ErrorNumber,
                                         const WString&          pi_rErrorObject,
                                         const WString&          pi_rErrorMessage);

    };

