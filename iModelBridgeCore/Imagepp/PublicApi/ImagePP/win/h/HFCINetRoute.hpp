//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/win/h/HFCINetRoute.hpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HFCInetRoute
//-----------------------------------------------------------------------------




//-----------------------------------------------------------------------------
// public
// GetNetworkAddress
//-----------------------------------------------------------------------------
inline const WString& HFCInetRoute::GetNetworkAddress() const
    {
    return m_NetworkAddress;
    }


//-----------------------------------------------------------------------------
// public
// GetNetmask
//-----------------------------------------------------------------------------
inline const WString& HFCInetRoute::GetNetmask() const
    {
    return m_Netmask;
    }


//-----------------------------------------------------------------------------
// public
// GetGatewayAddress
//-----------------------------------------------------------------------------
inline const WString& HFCInetRoute::GetGatewayAddress() const
    {
    return m_GatewayAddress;
    }


//-----------------------------------------------------------------------------
// public
// GetInterface
//-----------------------------------------------------------------------------
inline const WString& HFCInetRoute::GetInterface() const
    {
    return m_Interface;
    }


//-----------------------------------------------------------------------------
// public
// GetMetric
//-----------------------------------------------------------------------------
inline uint32_t HFCInetRoute::GetMetric() const
    {
    return m_Metric;
    }


