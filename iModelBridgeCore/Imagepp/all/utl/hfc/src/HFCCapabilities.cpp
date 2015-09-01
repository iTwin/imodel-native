//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCCapabilities.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCCapabilities
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>
 // Must be the first include.

#include <Imagepp/all/h/HFCCapabilities.h>
#include <Imagepp/all/h/HFCCapability.h>


/** -----------------------------------------------------------------------------
    This is the constructor. An empty capability set is created. Since capabilities
    cannot be added unless access is possible to the Add() protected method,
    this constructor should be used and considered Protected.
    -----------------------------------------------------------------------------
 */
HFCCapabilities::HFCCapabilities()
    {
    }

/** -----------------------------------------------------------------------------
    This is the copy constructor.

    @param pi_rObj
    -----------------------------------------------------------------------------
 */
HFCCapabilities::HFCCapabilities(const HFCCapabilities& pi_rObj)
    {
    ListOfCapability::const_iterator Itr(m_ListOfCapability.begin());
    while (Itr != m_ListOfCapability.end())
        {
        m_ListOfCapability.push_back((*Itr)->Clone());
        Itr++;
        }
    }

/** -----------------------------------------------------------------------------
    Destructor
    -----------------------------------------------------------------------------
 */
HFCCapabilities::~HFCCapabilities()
    {
    }

/** -----------------------------------------------------------------------------
    Indicates if the given capability exactly matches one in the capability set.

    <font color="#FF0000">
    Not only the type of capability is compared, but also individual parameters and
    configuration of the capability using the IsCompatibleWith() method of the HFCCapability
    class. Again access mode is not exactly matched but instead an included operation
    is performed. </font>

    @param pi_rpCapability

    @return true if a matching capability (see HRFCapability::IsCompatibleWith() for details
            concerning a match) is found and false otherwise.
    -----------------------------------------------------------------------------
 */
bool HFCCapabilities::Supports(const HFCCapability& pi_rCapability) const
    {
    bool CapabilitySupported = false;

    ListOfCapability::const_iterator Itr(m_ListOfCapability.begin());
    while (!CapabilitySupported && Itr != m_ListOfCapability.end())
        {
        CapabilitySupported = (*Itr)->Supports(pi_rCapability);
        Itr++;
        }

    return CapabilitySupported;
    }


/** -----------------------------------------------------------------------------
    Indicates if the given capability exactly matches one in the capability set.

    <font color="#FF0000">
    Not only the type of capability is compared, but also individual parameters and
    configuration of the capability using the IsCompatibleWith() method of the HFCCapability
    class. Again access mode is not exactly matched but instead an included operation
    is performed. </font>

    @param pi_rpCapability

    @return true if a matching capability (see HRFCapability::IsCompatibleWith() for details
            concerning a match) is found and false otherwise.
    -----------------------------------------------------------------------------
 */
bool HFCCapabilities::Supports(const HFCCapabilities& pi_rCapabilities) const
    {
    bool CapabilitiesSupported = true;

    // parse the list of attributes in the capabilities
    ListOfCapability::const_iterator Itr(pi_rCapabilities.m_ListOfCapability.begin());
    while (CapabilitiesSupported && Itr != pi_rCapabilities.m_ListOfCapability.end())
        {
        CapabilitiesSupported = Supports(**Itr);
        Itr++;
        }

    return CapabilitiesSupported;
    }


/** -----------------------------------------------------------------------------
    This methods returns the number of capabilities of any type within the capability
    set. <font color="#FF0000"> Why is it virtual ???? </font>

    @return The number of all capabilities in the set
    -----------------------------------------------------------------------------
 */
uint32_t HFCCapabilities::CountCapabilities() const
    {
    return (uint32_t)m_ListOfCapability.size();
    }

/** -----------------------------------------------------------------------------
    Returns a reference to the internal smart pointer to the capability. No changes
    should be made to the capability since the pointer returned points directly to the
    copy shared with the set.

    @param pi_Index    The index of the capability to gain access to. This number must
                       be between 0 and CountCapabilities() - 1.

    @return Reference to smart pointer to capability indicated.
    -----------------------------------------------------------------------------
 */
const HFCPtr<HFCCapability>& HFCCapabilities::GetCapability(uint32_t pi_Index) const
    {
    return m_ListOfCapability[pi_Index];
    }


/** -----------------------------------------------------------------------------
    Return a reference to the internal smart pointer to the capability. This method
    used the SameAs() of HFCCapability. <P>

    This method allows to select a capability based on an example capability provided.
    The example is compared relative to the class type, not to the content of this capability,
    which can be empty. If no match is found, a null pointer is returned.

    Note : No changes should be made to the capability since the pointer returned
           points directly to the copy shared with the set. <P>

    @param  pi_rpCapability  Constant reference to smart pointer to example capability
                             to find match for.

    @return The first matching capabilities in the set. (as in the order added with
            the Add() method)

    -----------------------------------------------------------------------------
 */
HFCPtr<HFCCapabilities> HFCCapabilities::GetCapabilitiesOfType(const HFCCapability& pi_rCapability) const
    {
    HFCPtr<HFCCapabilities> pCapabilities;

    ListOfCapability::const_iterator Itr(m_ListOfCapability.begin());
    while (Itr != m_ListOfCapability.end())
        {
        if ((*Itr)->IsSameAs(pi_rCapability))
            pCapabilities->Add(*Itr);

        Itr++;
        }

    return pCapabilities;
    }

/** -----------------------------------------------------------------------------
    Return true if this HFCCapabilities contain at least one capabitilities
    of type pi_CapabilityType. <P>


    @param pi_CapabilityType  The class key of an HFCCapability derived class
                              to search match for in the set.

    @return true if this HFCCapabilities contain at least one capabitilities
            of type pi_CapabilityType.
    -----------------------------------------------------------------------------
 */
bool HFCCapabilities::HasCapabilityOfType(const HFCCapability& pi_rCapability) const
    {
    bool Found = false;

    ListOfCapability::const_iterator Itr(m_ListOfCapability.begin());
    while (!Found && Itr != m_ListOfCapability.end())
        {
        Found = (*Itr)->IsSameAs(pi_rCapability);
        Itr++;
        }

    return Found;
    }

/** -----------------------------------------------------------------------------
    Return true if this HFCCapabilities contain at least one capabitilities
    of type pi_CapabilityType. <P>


    @param pi_CapabilityType  The class key of an HFCCapability derived class
                              to search match for in the set.

    @return true if this HFCCapabilities contain at least one capabitilities
            of type pi_CapabilityType.
    -----------------------------------------------------------------------------
 */
void HFCCapabilities::Add(const HFCPtr<HFCCapability>& pi_rCapability)
    {
    HPRECONDITION(pi_rCapability != 0);

    if (!Supports(*pi_rCapability))
        m_ListOfCapability.push_back(pi_rCapability);
    }
