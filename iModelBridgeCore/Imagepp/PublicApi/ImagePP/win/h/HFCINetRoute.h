//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/win/h/HFCINetRoute.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HFCInetRoute
//-----------------------------------------------------------------------------

#pragma once



class HFCInetRoute
    {
public:

    // Constructor - Destructor
    HFCInetRoute(const WString& pi_NetworkAddress = L"",
                 const WString& pi_Netmask = L"",
                 const WString& pi_GatewayAddress = L"",
                 const WString& pi_Interface = L"",
                 uint32_t       pi_Metric = 1);
    HFCInetRoute(const HFCInetRoute& pi_rObj);
    virtual ~HFCInetRoute();

    // Assignment operator
    HFCInetRoute& operator=(const HFCInetRoute& pi_rObj);

    // Information
    const WString&  GetNetworkAddress() const;
    const WString&  GetNetmask() const;
    const WString&  GetGatewayAddress() const;
    const WString&  GetInterface() const;
    uint32_t        GetMetric() const;

protected:

private:

    // Columns of the routing table
    WString         m_NetworkAddress;
    WString         m_Netmask;
    WString         m_GatewayAddress;
    WString         m_Interface;
    uint32_t        m_Metric;
    };


#include "HFCInetRoute.hpp"

