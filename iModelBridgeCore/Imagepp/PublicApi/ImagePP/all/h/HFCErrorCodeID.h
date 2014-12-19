//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCErrorCodeID.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

// HFCErrorCodeID class is in fact a 32-bit integer
class HFCErrorCodeID
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HFCErrorCodeID();
    HFCErrorCodeID(uint32_t pi_Value);
    HFCErrorCodeID(const WString& pi_rString);
    HFCErrorCodeID(const HFCErrorCodeID& pi_rObj);
    //virtual       ~HFCErrorCodeID();


    //--------------------------------------
    // Operators
    //--------------------------------------

    // Assignment operator
    HFCErrorCodeID& operator=(const HFCErrorCodeID& pi_rObj);

    // Cast operator
    operator uint32_t();
    operator uint32_t() const;

    // Output the value to a stream
//chck UNICODE
    void            GenerateString(wostringstream&  po_rStream,
                                   uint32_t         pi_DigitCount) const;

private:
    uint32_t       m_Value;
    };

#include "HFCErrorCodeID.hpp"

