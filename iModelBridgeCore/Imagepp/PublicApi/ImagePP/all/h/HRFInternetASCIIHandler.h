//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFInternetASCIIHandler.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetASCIIHandler
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

#pragma once

#include "HRFInternetImagingHandler.h"

class HRFInternetASCIIHandler : public HRFInternetImagingHandler
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HRFInternetASCIIHandler(const string& pi_rLabel);
    virtual         ~HRFInternetASCIIHandler();


protected:
    //--------------------------------------
    // Methods
    //--------------------------------------

    // This method will finish reading the data up to the
    // end marker
    void            ReadUntilMarker(HFCInternetConnection& pi_rConnection,
                                    HFCBuffer&             pio_rBuffer);


    //--------------------------------------
    // Attributes
    //--------------------------------------

    // The end marker of a response
    static const string s_Marker;
    };

