//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFCombinedRasterFileCapabilities.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFCombinedRasterFileCapabilities
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>
 // Must be the first include.
#include <Imagepp/all/h/HRFCombinedRasterFileCapabilities.h>
#include <Imagepp/all/h/HRFCapability.h>

//-----------------------------------------------------------------------------
// HRFCombinedRasterFileCapabilities
//-----------------------------------------------------------------------------
HRFCombinedRasterFileCapabilities::HRFCombinedRasterFileCapabilities()
    : HRFRasterFileCapabilities()
    {
    }

//-----------------------------------------------------------------------------
// HRFCombinedRasterFileCapabilities
//-----------------------------------------------------------------------------
HRFCombinedRasterFileCapabilities::HRFCombinedRasterFileCapabilities(const HFCPtr<HRFRasterFileCapabilities>& pi_rpCapabilitiesA,
                                                                     const HFCPtr<HRFRasterFileCapabilities>& pi_rpCapabilitiesB)
    : HRFRasterFileCapabilities()
    {
    m_pCapabilitiesA = pi_rpCapabilitiesA;
    m_pCapabilitiesB = pi_rpCapabilitiesB;
    }

//-----------------------------------------------------------------------------
// HRFCombinedRasterFileCapabilities
//-----------------------------------------------------------------------------
HRFCombinedRasterFileCapabilities::~HRFCombinedRasterFileCapabilities()
    {
    }


//-----------------------------------------------------------------------------
// test if the specific capability is supported
//-----------------------------------------------------------------------------
bool HRFCombinedRasterFileCapabilities::Supports(const HFCPtr<HRFCapability>& pi_rpCapability)
    {
    bool Found = false;

    Found = m_pCapabilitiesA->Supports(pi_rpCapability);
    if (!Found)
        Found = m_pCapabilitiesB->Supports(pi_rpCapability);

    return Found;
    }

//-----------------------------------------------------------------------------
// capabilities count "include all type of capabilities"
//-----------------------------------------------------------------------------
uint32_t HRFCombinedRasterFileCapabilities::CountCapabilities() const
    {
    return m_pCapabilitiesA->CountCapabilities() + m_pCapabilitiesB->CountCapabilities();
    }

//-----------------------------------------------------------------------------
// capabilities get "include all type of capabilities"
//-----------------------------------------------------------------------------
const HFCPtr<HRFCapability>& HRFCombinedRasterFileCapabilities::GetCapability(uint32_t pi_Index) const
    {
    if (pi_Index < m_pCapabilitiesA->CountCapabilities())
        return m_pCapabilitiesA->GetCapability(pi_Index);
    else
        return m_pCapabilitiesB->GetCapability(pi_Index - m_pCapabilitiesA->CountCapabilities());
    }

//-----------------------------------------------------------------------------
// Create a capabilities for a specific type
//-----------------------------------------------------------------------------
HRFRasterFileCapabilities* HRFCombinedRasterFileCapabilities::GetCapabilitiesOfType(
    HCLASS_ID   pi_CapabilityType,
    HFCAccessMode pi_AccessMode) const
    {
    HRFRasterFileCapabilities* pCapabilitiesOfType = 0;

    if (GetCapabilityOfType(pi_CapabilityType, pi_AccessMode) != 0)
        {
        // Initialize the capabilities
        pCapabilitiesOfType = new HRFRasterFileCapabilities();

        // Fill the capabilities with the specific type
        for (uint32_t Index = 0; (Index < CountCapabilities()); Index++)
            {
            HFCPtr<HRFCapability> pCapability = GetCapability(Index);

            if ((pCapability->GetClassID() == pi_CapabilityType) &&
                (pCapability->GetAccessMode().IsIncluded(pi_AccessMode)))
                pCapabilitiesOfType->Add(pCapability);
            }
        }

    // Give to the client the capabilities found
    // if the capabilities are not found return 0
    // The client is responsible for the destruction of this pointer
    return pCapabilitiesOfType;
    }

/** -----------------------------------------------------------------------------
    Return true if this HRFRasterFileCapabilities contain at least one capabitilities
    of type pi_CapabilityType that fit with the given access mode. <P>

    Note that the access mode is not exactly matched. If a capability of the appropriate type
    indicates read-write access mode and a read mode is desired then a match is
    concluded since read access is permitted.

    @param pi_CapabilityType  The class key of an HRFCapability derived class
                              to search match for in the set.
    @param pi_AccessMode      The required access mode will have to match (be included) with
                              access mode of capability found.

    @return true if this HRFRasterFileCapabilities contain at least one capabitilities
            of type pi_CapabilityType that fit with the given access mode.
    -----------------------------------------------------------------------------
 */

bool HRFCombinedRasterFileCapabilities::HasCapabilityOfType(HCLASS_ID   pi_CapabilityType,
                                                             HFCAccessMode pi_AccessMode) const
    {
    bool Found = false;

    for (uint32_t Index = 0; (Index < m_pCapabilitiesA->CountCapabilities()) && !Found; Index++)
        if ((m_pCapabilitiesA->GetCapability(Index)->GetClassID() == pi_CapabilityType) &&
            (m_pCapabilitiesA->GetCapability(Index)->GetAccessMode().IsIncluded(pi_AccessMode)))
            Found = true;

    if (!Found)
        {
        for (uint32_t Index = 0; (Index < m_pCapabilitiesB->CountCapabilities()) && !Found; Index++)
            if ((m_pCapabilitiesB->GetCapability(Index)->GetClassID() == pi_CapabilityType) &&
                (m_pCapabilitiesB->GetCapability(Index)->GetAccessMode().IsIncluded(pi_AccessMode)))
                Found = true;
        }

    return Found;
    }

/** -----------------------------------------------------------------------------
    Return true if this HRFRasterFileCapabilities contain at least one capabitilities
    of type pi_CapabilityType. <P>

    @param pi_CapabilityType  The class key of an HRFCapability derived class
                              to search match for in the set.

    @return true if this HRFRasterFileCapabilities contain at least one capabitilities
            of type pi_CapabilityType.
    -----------------------------------------------------------------------------
 */
bool HRFCombinedRasterFileCapabilities::HasCapabilityOfType(HCLASS_ID pi_CapabilityType) const
    {
    bool Found = false;

    for (uint32_t Index = 0; (Index < m_pCapabilitiesA->CountCapabilities()) && !Found; Index++)
        if (m_pCapabilitiesA->GetCapability(Index)->GetClassID() == pi_CapabilityType)
            Found = true;

    if (!Found)
        {
        for (uint32_t Index = 0; (Index < m_pCapabilitiesB->CountCapabilities()) && !Found; Index++)
            if (m_pCapabilitiesB->GetCapability(Index)->GetClassID() == pi_CapabilityType)
                Found = true;
        }

    return Found;
    }



//-----------------------------------------------------------------------------
// Create a capabilities for a specific type
//-----------------------------------------------------------------------------
HRFRasterFileCapabilities*  HRFCombinedRasterFileCapabilities::GetCapabilitiesOfType(HCLASS_ID pi_CapabilityType) const
    {
    HRFRasterFileCapabilities* pCapabilitiesOfType = 0;

    if (GetCapabilityOfType(pi_CapabilityType) != 0)
        {
        // Initialize the capabilities
        pCapabilitiesOfType = new HRFRasterFileCapabilities();

        // Fill the capabilities with the specific type
        for (uint32_t Index = 0; (Index < CountCapabilities()); Index++)
            {
            HFCPtr<HRFCapability> pCapability = GetCapability(Index);

            if (pCapability->GetClassID() == pi_CapabilityType)
                pCapabilitiesOfType->Add(pCapability);
            }
        }

    // Give to the client the capabilities found
    // if the capabilities are not found return 0
    // The client is responsible for the destruction of this pointer
    return pCapabilitiesOfType;
    }

//-----------------------------------------------------------------------------
// Create a capability for a specific type
//-----------------------------------------------------------------------------
HFCPtr<HRFCapability> HRFCombinedRasterFileCapabilities::GetCapabilityOfType(
    HCLASS_ID   pi_CapabilityType,
    HFCAccessMode pi_AccessMode) const
    {
    HFCPtr<HRFCapability> Capability;

    Capability = m_pCapabilitiesA->GetCapabilityOfType(pi_CapabilityType, pi_AccessMode);
    if (Capability == 0)
        Capability = m_pCapabilitiesB->GetCapabilityOfType(pi_CapabilityType, pi_AccessMode);

    return Capability;
    }

//-----------------------------------------------------------------------------
// Create a capability for a specific type
//-----------------------------------------------------------------------------
HFCPtr<HRFCapability> HRFCombinedRasterFileCapabilities::GetCapabilityOfType(HCLASS_ID   pi_CapabilityType) const
    {
    HFCPtr<HRFCapability> Capability;

    Capability = m_pCapabilitiesA->GetCapabilityOfType(pi_CapabilityType);
    if (Capability == 0)
        Capability = m_pCapabilitiesB->GetCapabilityOfType(pi_CapabilityType);

    return Capability;
    }

//-----------------------------------------------------------------------------
// Create a capability for a specific type
//-----------------------------------------------------------------------------
HFCPtr<HRFCapability> HRFCombinedRasterFileCapabilities::GetCapabilityOfType(
    const HFCPtr<HRFCapability>& pi_rpCapability) const
    {
    HFCPtr<HRFCapability> Capability;

    Capability = m_pCapabilitiesA->GetCapabilityOfType(pi_rpCapability);
    if (Capability == 0)
        Capability = m_pCapabilitiesB->GetCapabilityOfType(pi_rpCapability);

    return Capability;
    }
