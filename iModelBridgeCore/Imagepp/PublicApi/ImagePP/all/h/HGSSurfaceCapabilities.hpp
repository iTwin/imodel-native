//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSSurfaceCapabilities.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGSSurfaceCapabilities
//-----------------------------------------------------------------------------
// General class for surfaces.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// public
// Add
//-----------------------------------------------------------------------------
inline void HGSSurfaceCapabilities::Add(const HGSSurfaceAttribute* pi_pAttribute)
    {
    // test if the attribute is already in the capabilities
    if(!Supports(pi_pAttribute))
        {
        // if not, add it in the capabilities
        m_Attributes.push_back(pi_pAttribute->Clone());
        }
    }

//-----------------------------------------------------------------------------
// public
// Supports
//-----------------------------------------------------------------------------
inline bool HGSSurfaceCapabilities::Supports(const HGSSurfaceAttribute* pi_pAttribute) const
    {
    bool CapabilitySupported = false;

    // parse the list of attributes in the capabilities
    Attributes::const_iterator Itr;
    for (Itr = m_Attributes.begin(); Itr != m_Attributes.end() && !CapabilitySupported; Itr++)
        {
        // test if this is the same kind of attribute
        if((*Itr)->IsSameAs(pi_pAttribute))
            CapabilitySupported = true;
        }

    return CapabilitySupported;
    }

//-----------------------------------------------------------------------------
// public
// Supports
//-----------------------------------------------------------------------------
inline bool HGSSurfaceCapabilities::Supports(const HGSSurfaceCapabilities& pi_rCapabilities) const
    {
    bool CapabilitiesSupported = true;

    // parse each attribute of the capabilities parameter and test if they are
    // included in the current capabilties
    Attributes::const_iterator Itr;
    for (Itr = pi_rCapabilities.m_Attributes.begin();
         Itr != pi_rCapabilities.m_Attributes.end() && CapabilitiesSupported;
         Itr++)
        {
        // test if the capability is supported
        if(!Supports(*Itr))
            CapabilitiesSupported = false;

        }

    return CapabilitiesSupported;
    }
