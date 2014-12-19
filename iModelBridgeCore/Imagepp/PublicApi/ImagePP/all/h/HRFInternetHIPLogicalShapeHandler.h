//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFInternetHIPLogicalShapeHandler.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetHIPLogicalShapeHandler
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

#pragma once

#include "HRFInternetASCIIHandler.h"

class HRFInternetHIPLogicalShapeHandler : public HRFInternetASCIIHandler
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HRFInternetHIPLogicalShapeHandler();
    virtual         ~HRFInternetHIPLogicalShapeHandler();


    //--------------------------------------
    // Handling
    //--------------------------------------

    // Handle the rest of the data
    virtual void    Handle(HRFInternetImagingFile& pi_rFile,
                           HFCBuffer&              pio_rBuffer,
                           HFCInternetConnection&  pi_rConnection);
    };

