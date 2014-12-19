//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFInternetBinaryHandler.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFInternetBinaryHandler
//-----------------------------------------------------------------------------

#pragma once

#include "HRFInternetImagingHandler.h"

class HRFInternetBinaryHandler : public HRFInternetImagingHandler
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HRFInternetBinaryHandler(const string& pi_rLabel);
    virtual         ~HRFInternetBinaryHandler();


protected:
    //--------------------------------------
    // Methods
    //--------------------------------------

    // This method will finish reading the data up to the binary data
    void            ReadUntilData(HFCInternetConnection& pi_rConnection,
                                  HFCBuffer&             pio_rBuffer) const;

    // this method fills the buffer with the binary data
    void            ReadData     (HFCInternetConnection& pi_rConnection,
                                  HFCBuffer&             pio_rBuffer,
                                  int32_t                pi_DataSize,
                                  bool                  pi_Clear = false) const;


    // This methods skips a certain amount of data from the connection
    void            SkipData(HFCInternetConnection& pi_rConnection,
                             size_t                 pi_DataSize) const;

    // This method skips the ending marker of the current response
    void            SkipEndMarker(HFCInternetConnection& pi_rConnection) const;


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // The end marker of a response
    static const string s_Marker;

    // The label-data delimiter
    static const string s_Delimiter;
    };

