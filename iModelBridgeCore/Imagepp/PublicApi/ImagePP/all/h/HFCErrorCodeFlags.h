//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCErrorCodeFlags.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

// HFCErrorCodeFlags class is in fact an 8-bit integer
class HFCErrorCodeFlags
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HFCErrorCodeFlags();
    HFCErrorCodeFlags(uint8_t pi_Value);
    HFCErrorCodeFlags(const WString& pi_rString);
    HFCErrorCodeFlags(const HFCErrorCodeFlags& pi_rObj);
    //virtual       ~HFCErrorCodeFlags();


    //--------------------------------------
    // Operators
    //--------------------------------------

    // Assignment operator
    HFCErrorCodeFlags&
    operator=(const HFCErrorCodeFlags& pi_rObj);

    // Cast operator
    operator uint8_t();
    operator uint8_t() const;

    // Return flag values
    bool           IsFatal() const;

    // Output the value to a stream
    void            GenerateString(wostringstream& po_rStream) const;

private:
    uint8_t        m_Flags;
    };

#include "HFCErrorCodeFlags.hpp"

