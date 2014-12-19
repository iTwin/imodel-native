//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/win/h/HFCInetAddr.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCInetAddr
//-----------------------------------------------------------------------------
#pragma once


//####################################################
// INCLUDE FILES
//####################################################



class HFCInetAddr
    {
public:

    // Constructor - Destructor
    _HDLLw HFCInetAddr();
    _HDLLw HFCInetAddr(const HFCInetAddr&);
    _HDLLw HFCInetAddr(const WString& pi_Address, unsigned short pi_Port);
    _HDLLw virtual ~HFCInetAddr();

    // Assignment operator
    _HDLLw HFCInetAddr& operator=(const HFCInetAddr&);

    // Conversions
    WString AsChar();

    // Get - Set methods
    const WString& GetAddress() const;
    void SetAddress(const WString& pi_Address);

    unsigned short GetPort() const;
    void SetPort(unsigned short pi_Port);

    // Proximity information
    bool IsOnLocalSubnet() const;

protected:

private:

    static WString  GetSubnetMaskForInterface(const WString& pi_rInterface);

    void           CommonCopy(const HFCInetAddr&);
    uint32_t         GetLength();
    void            EnsureAddressIsNumeric();

    // Attribute
    WString m_Address;
    unsigned short m_Port;
    };

#include "HFCInetAddr.hpp"

