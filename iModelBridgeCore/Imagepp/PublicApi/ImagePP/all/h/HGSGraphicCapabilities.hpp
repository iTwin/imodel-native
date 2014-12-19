//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSGraphicCapabilities.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
// Class HGSGraphicCapabilities
//---------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Add
//-----------------------------------------------------------------------------
inline void HGSGraphicCapabilities::Add(const HGSGraphicCapability* pi_pCapability)
    {
    // test if the Capability is already in the Capabilities
    if(!Supports(pi_pCapability))
        {
        // if not, add it in the Capabilities
        m_Capabilities.push_back(pi_pCapability->Clone());
        }
    }

//-----------------------------------------------------------------------------
// public
// Supports
//-----------------------------------------------------------------------------
inline bool HGSGraphicCapabilities::Supports(const HGSGraphicCapability* pi_pCapability) const
    {
    bool CapabilitySupported = false;

    // parse the list of Capabilities in the Capabilities
    Capabilities::const_iterator Itr;
    for (Itr = (const_cast<HGSGraphicCapabilities*> (this))->m_Capabilities.begin(); Itr != m_Capabilities.end() && !CapabilitySupported; Itr++)
        {
        // test if this is the same kind of Capability
        if((*Itr)->IsSameAs(pi_pCapability))
            CapabilitySupported = true;
        }

    return CapabilitySupported;
    }

//-----------------------------------------------------------------------------
// public
// Supports
//-----------------------------------------------------------------------------
inline bool HGSGraphicCapabilities::Supports(const HGSGraphicCapabilities& pi_rCapabilities) const
    {
    bool CapabilitiesSupported = true;

    // parse each Capability of the Capabilities parameter and test if they are
    // included in the current capabilties
    Capabilities::const_iterator Itr;
    for (Itr = (const_cast<HGSGraphicCapabilities&> (pi_rCapabilities)).m_Capabilities.begin();
         Itr != pi_rCapabilities.m_Capabilities.end() && CapabilitiesSupported;
         Itr++)
        {
        // test if the capability is supported
        if(!Supports(*Itr))
            CapabilitiesSupported = false;

        }

    return CapabilitiesSupported;
    }

