//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSGraphicToolAttributes.hpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HGSGraphicToolAttributes
//---------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// public
// Add
//-----------------------------------------------------------------------------
inline void HGSGraphicToolAttributes::Add(const HGSGraphicToolAttribute& pi_rAttribute)
    {
    // test if the attribute is already in the Attributes
    if(!Contains(pi_rAttribute))
        {
        // if not, add it in the Attributes
        m_Attributes.push_back(pi_rAttribute.Clone());
        }
    }

//-----------------------------------------------------------------------------
// public
// Contains
//-----------------------------------------------------------------------------
inline bool HGSGraphicToolAttributes::Contains(const HGSGraphicToolAttribute& pi_rAttribute) const
    {
    bool ContainsAttribute = false;

    // parse the list of attributes in the Attributes
    Attributes::const_iterator Itr;
    for (Itr = m_Attributes.begin(); Itr != m_Attributes.end() && !ContainsAttribute; Itr++)
        {
        ContainsAttribute = (*Itr)->IsSameAs(pi_rAttribute) && (*Itr)->Supports(pi_rAttribute);
        }

    return ContainsAttribute;
    }

//-----------------------------------------------------------------------------
// public
// Contains
//-----------------------------------------------------------------------------
inline bool HGSGraphicToolAttributes::Contains(const HGSGraphicToolAttributes& pi_rAttributes) const
    {
    bool ContainsAttributes = true;

    // parse each attribute of the Attributes parameter and test if they are
    // included in the current capabilties
    Attributes::const_iterator Itr;
    for (Itr = m_Attributes.begin();
         Itr != pi_rAttributes.m_Attributes.end() && ContainsAttributes;
         Itr++)
        {
        ContainsAttributes = Contains(**Itr);
        }

    return ContainsAttributes;
    }

//-----------------------------------------------------------------------------
// public
// FindOfType
//-----------------------------------------------------------------------------
inline const HGSGraphicToolAttribute* HGSGraphicToolAttributes::FindOfType(HCLASS_ID pi_AttributeID) const
    {
    const HGSGraphicToolAttribute* pAttribute = 0;

    // parse each attribute of the Attributes parameter and test if they have
    // the requested ID
    Attributes::const_iterator Itr;
    for (Itr = m_Attributes.begin(); pAttribute == 0 && Itr != m_Attributes.end(); Itr++)
        {
        // test if the capability is supported
        if ((*Itr)->GetClassID() == pi_AttributeID)
            pAttribute = (*Itr);
        }
    return pAttribute;
    }

//-----------------------------------------------------------------------------
// public
// GetCount
//-----------------------------------------------------------------------------
inline uint32_t HGSGraphicToolAttributes::GetCount() const
    {
    return (uint32_t)m_Attributes.size();
    }

//-----------------------------------------------------------------------------
// public
// GetAttribute
//-----------------------------------------------------------------------------
inline const HFCPtr<HGSGraphicToolAttribute>& HGSGraphicToolAttributes::GetAttribute(uint32_t pi_Index) const
    {
    return m_Attributes[pi_Index];
    }

//-----------------------------------------------------------------------------
// public
// GetRequiredCapabilities
//-----------------------------------------------------------------------------
inline HGSGraphicToolCapabilities* HGSGraphicToolAttributes::GetRequiredCapabilities() const
    {
    HGSGraphicToolCapabilities* pCapabilities = new HGSGraphicToolCapabilities();

    // parse each attribute of the Attributes parameter and test if they are
    // included in the current capabilties
    Attributes::const_iterator Itr;
    for (Itr = m_Attributes.begin(); Itr != m_Attributes.end(); Itr++)
        {
        pCapabilities->Add(new HGSGraphicToolCapability(**Itr));
        }

    return pCapabilities;
    }

