//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFRasterFileCapabilities.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFRasterFileCapabilities
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>
 // Must be the first include.

#include <Imagepp/all/h/HFCAccessMode.h>
#include <Imagepp/all/h/HRFCapability.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>

/** -----------------------------------------------------------------------------
    This is the constructor. An empty capability set is created. Since capabilities
    cannot be added unless access is possible to the Add() protected method,
    this constructor should be used and considered Protected.
    -----------------------------------------------------------------------------
 */
HRFRasterFileCapabilities::HRFRasterFileCapabilities()
    {
    }

/** -----------------------------------------------------------------------------
    Destructor
    -----------------------------------------------------------------------------
 */
HRFRasterFileCapabilities::~HRFRasterFileCapabilities()
    {
    }

/** -----------------------------------------------------------------------------
    Indicates if the given capability exactly matches one in the capability set.

    <font color="#FF0000">
    Not only the type of capability is compared, but also individual parameters and
    configuration of the capability using the IsCompatibleWith() method of the HRFCapability
    class. Again access mode is not exactly matched but instead an included operation
    is performed. </font>

    @param pi_rpCapability

    @return true if a matching capability (see HRFCapability::IsCompatibleWith() for details
            concerning a match) is found and false otherwise.
    -----------------------------------------------------------------------------
 */
bool HRFRasterFileCapabilities::Supports(const HFCPtr<HRFCapability>& pi_rpCapability)
    {
    bool Found = false;

    for (size_t Index = 0; (Index < m_ListOfCapability.size()) && (!Found); Index++)
        Found = m_ListOfCapability[Index]->IsCompatibleWith(pi_rpCapability);

    return Found;
    }

/** -----------------------------------------------------------------------------
    This methods returns the number of capabilities of any type within the capability
    set. <font color="#FF0000"> Why is it virtual ???? </font>

    @return The number of all capabilities in the set
    -----------------------------------------------------------------------------
 */
uint32_t HRFRasterFileCapabilities::CountCapabilities() const
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
const HFCPtr<HRFCapability>& HRFRasterFileCapabilities::GetCapability(uint32_t pi_Index) const
    {
    return m_ListOfCapability[pi_Index];
    }

/** -----------------------------------------------------------------------------
    Returns a newly allocated HRFRasterFileCapabilities containing all references
    matching with desired capabilities. References only are duplicated. The copy of
    HRFRasterFileCapabilities must be deleted explicitely by caller when needed no more.
    All matching capabilities in the set are returned. The class key of an HRFCapability
    class can indicate the type of capability. The capabilities that at the same time
    match the capability type and has a fitting access mode are returned. Note that
    the access mode is not exactly matched. If a capability of the appropriate type
    indicates read-write access mode and a read mode is desired then a match is
    concluded since read access is permitted. If no match is found, then a NULL
    pointer is returned.<font color="#FF0000">Is the SameAs() method used ???? </font>

    @param pi_CapabilityType  The class key of an HRFCapability derived class (Is this validates ???)
                              to search match for in the set.
    @param pi_AccessMode      The required access mode will have to match (be included) with
                              access mode of capability found.

    @return A newly allocated HRFRasterFileCapabilities object containing references
            to all matching capabilities. If none is found, a NULL pointer is returned.
            <font color="#FF0000">
            The set must be explicitely deleted when by the client needed no more. </font>
    -----------------------------------------------------------------------------
 */

HRFRasterFileCapabilities* HRFRasterFileCapabilities::GetCapabilitiesOfType(
    HCLASS_ID   pi_CapabilityType,
    HFCAccessMode pi_AccessMode) const
    {
    HRFRasterFileCapabilities* pCapabilitiesOfType = 0;

    if (GetCapabilityOfType(pi_CapabilityType, pi_AccessMode) != 0)
        {
        // Initialize the capabilities
        pCapabilitiesOfType = new HRFRasterFileCapabilities();

        // Fill the capabilities with the specific type
        for (size_t Index = 0; (Index < m_ListOfCapability.size()); Index++)
            if ((m_ListOfCapability[Index]->GetClassID() == pi_CapabilityType) &&
                (m_ListOfCapability[Index]->GetAccessMode().IsIncluded(pi_AccessMode)))
                pCapabilitiesOfType->Add(m_ListOfCapability[Index]);
        }

    // Give to the client the capabilities found
    // if the capabilities are not found return 0
    // The client is responsible for the destruction of this pointer
    return pCapabilitiesOfType;
    }

/** -----------------------------------------------------------------------------
    Returns a newly allocated HRFRasterFileCapabilities containing all references
    matching with desired capabilities. References only are duplicated. The copy of
    HRFRasterFileCapabilities must be deleted explicitely by caller when needed no more.
    All matching capabilities in the set are returned. The class key of an HRFCapability
    class can indicate the type of capability. If no match is found, then a NULL
    pointer is returned.<font color="#FF0000">NO HRFCapability Methode are used !?!?!?!
    we only compare the class key !!!! </font>

    @param pi_CapabilityType  The class key of an HRFCapability derived class (Is this validates ???)
                              to search match for in the set.

    @return A newly allocated HRFRasterFileCapabilities object containing references
            to all matching capabilities. If none is found, a NULL pointer is returned.
            <font color="#FF0000">
            The set must be explicitely deleted when by the client needed no more. </font>
    -----------------------------------------------------------------------------
 */
HRFRasterFileCapabilities* HRFRasterFileCapabilities::GetCapabilitiesOfType(
    HCLASS_ID   pi_CapabilityType) const
    {
    HRFRasterFileCapabilities* pCapabilitiesOfType = 0;

    if (GetCapabilityOfType(pi_CapabilityType) != 0)
        {
        // Initialize the capabilities
        pCapabilitiesOfType = new HRFRasterFileCapabilities();

        // Fill the capabilities with the specific type
        for (size_t Index = 0; (Index < m_ListOfCapability.size()); Index++)
            if (m_ListOfCapability[Index]->GetClassID() == pi_CapabilityType)
                pCapabilitiesOfType->Add(m_ListOfCapability[Index]);
        }

    // Give to the client the capabilities found
    // if the capabilities are not found return 0
    // The client is responsible for the destruction of this pointer
    return pCapabilitiesOfType;
    }

/** -----------------------------------------------------------------------------
    Return a reference to the internal smart pointer to the capability. This method
    should be used, as example, to get the first capability of type "pixel type capability"
    in the set. This method compare the given class key with the capability class key
    in the set of capabilities. <P>

    The first capability that at the same time matches the capability type and has a
    fitting access mode will be returned. Note that the access mode is not exactly matched.
    If a capability of the appropriate type indicates read-write access mode and a read mode
    is desired then a match is concluded since read access is permitted.<P>

    Note : No changes should be made to the capability since the pointer returned
           points directly to the copy shared with the set.<P>

    @param pi_CapabilityType  The class key of an HRFCapability derived (Is this
                              validated????) class to search match for in the set.
    @param pi_AccessMode      The required access mode. This access mode will have to match
                              (be included) with access mode of capability found.

    @return The first matching capabilities in the set. (as in the order added with
            the Add() method)

 *  -----------------------------------------------------------------------------
 */
HFCPtr<HRFCapability> HRFRasterFileCapabilities::GetCapabilityOfType(
    HCLASS_ID   pi_CapabilityType,
    HFCAccessMode pi_AccessMode) const
    {
    HFCPtr<HRFCapability> Capability;

    for (size_t Index = 0; (Index < m_ListOfCapability.size()) && (Capability == 0); Index++)
        if ((m_ListOfCapability[Index]->GetClassID() == pi_CapabilityType) &&
            (m_ListOfCapability[Index]->GetAccessMode().IsIncluded(pi_AccessMode)))
            Capability = m_ListOfCapability[Index];

    return Capability;
    }

/** -----------------------------------------------------------------------------
    Return a reference to the internal smart pointer to the capability. This method
    should be used, as example, to get the first capability of type "pixel type capability"
    in the set.This method compare the given class key with the capability class key
    in the set of capabilities.<P>

    Note : No changes should be made to the capability since the pointer returned
           points directly to the copy shared with the set. <P>

    @param pi_CapabilityType  The class key of an HRFCapability derived (Is this
                              validated????) class to search match for in the set.

    @return The first matching capabilities in the set. (as in the order added with
            the Add() method)

    -----------------------------------------------------------------------------
 */
HFCPtr<HRFCapability> HRFRasterFileCapabilities::GetCapabilityOfType(
    HCLASS_ID   pi_CapabilityType) const
    {
    HFCPtr<HRFCapability> Capability;

    for (size_t Index = 0; (Index < m_ListOfCapability.size()) && (Capability == 0); Index++)
        if (m_ListOfCapability[Index]->GetClassID() == pi_CapabilityType)
            Capability = m_ListOfCapability[Index];

    return Capability;
    }

/** -----------------------------------------------------------------------------
    Return a reference to the internal smart pointer to the capability. This method
    used the SameAs() of HRFCapability. <P>

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
HFCPtr<HRFCapability> HRFRasterFileCapabilities::GetCapabilityOfType(
    const HFCPtr<HRFCapability>& pi_rpCapability) const
    {
    HFCPtr<HRFCapability> Capability;

    for (size_t Index = 0; (Index < m_ListOfCapability.size()) && (Capability == 0); Index++)
        if (m_ListOfCapability[Index]->SameAs(pi_rpCapability))
            Capability = m_ListOfCapability[Index];

    return Capability;
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

bool HRFRasterFileCapabilities::HasCapabilityOfType(HCLASS_ID   pi_CapabilityType,
                                                     HFCAccessMode pi_AccessMode) const
    {
    bool Found = false;

    for (size_t Index = 0; (Index < m_ListOfCapability.size()) && !Found; Index++)
        if ((m_ListOfCapability[Index]->GetClassID() == pi_CapabilityType) &&
            (m_ListOfCapability[Index]->GetAccessMode().IsIncluded(pi_AccessMode)))
            Found = true;

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
bool HRFRasterFileCapabilities::HasCapabilityOfType(HCLASS_ID pi_CapabilityType) const
    {
    bool Found = false;

    for (size_t Index = 0; (Index < m_ListOfCapability.size()) && !Found; Index++)
        if (m_ListOfCapability[Index]->GetClassID() == pi_CapabilityType)
            Found = true;

    return Found;
    }

/** -----------------------------------------------------------------------------
    This protected method adds a capability to the set. No validation is performed;
    any capability will be accepted even if the exact capability is already in the set.
    It is up to the writer of the descendant class to make sure capabilities are not
    duplicated either by making note of previous additions or by querying the set for
    existing capabilities. <p>

    <B>THIS METHOD IS PUBLIC FOR IMPLANTATION REASON ONLY ... APPLICATIONS MUST NEVER
       CALL THIS METHOD</B>

    @param pi_rpCapability Reference to smart pointer to a capability of any type.
                           The capability is shared, not duplicated so any changes made
                           afterward to this capability will reflect on the contained capability.
    -----------------------------------------------------------------------------
 */
void HRFRasterFileCapabilities::Add(const HFCPtr<HRFCapability>& pi_rpCapability)
    {
    m_ListOfCapability.push_back(pi_rpCapability);
    }
