//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFRasterFileCapabilities.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFRasterFileCapabilities
//-----------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"
#include "HRFCapability.h"

/** -----------------------------------------------------------------------------
    This class is a container for multiple file format capabilities. Capabilities derived
    from HRFCapability are added and can be queried or searched for within the raster file
    capabilities object. All raster file format implementations define a descendant from the
    present class applicable to the file format specifications to be supported. All applicable
    capabilities are added at construction. The raster file format capabilities with all
    capabilities added represent the essence of the file format.<P>

    Although most of the times a descendant of the present class will be used, this class is
    concrete and can be used in the present level. Descendants of the present class are required
    for the external addition of capabilities. Some methods however will create objects of the
    present class such as GetCapabilitiesOfType() methods that return a subset of the capability
    set.<P>

    For more details concerning raster file capabilities refere to the HRF User's Guide <P>


    <h3>see </h3>
    <LI><a href = "../../../doc/HRF.doc"> HRF user guide documentation </a></LI>
    <LI><a href = "../../../doc/HRFRasterFileCapabilities.doc"> HRFRasterFileCapabilities.doc </a></LI>
    -----------------------------------------------------------------------------
 */
class HRFRasterFileCapabilities : public HFCShareableObject<HRFRasterFileCapabilities>
    {
    HDECLARE_BASECLASS_ID(2270)

public:
    // creation and destruction
    _HDLLg HRFRasterFileCapabilities();
    _HDLLg virtual ~HRFRasterFileCapabilities();

    // test if the specific capability is supported
    _HDLLg virtual bool                           Supports(const HFCPtr<HRFCapability>& pi_rpCapability);

    // capabilities information "include all type of capabilities"
    _HDLLg virtual uint32_t                        CountCapabilities    () const;
    _HDLLg virtual const HFCPtr<HRFCapability>&    GetCapability        (uint32_t pi_Index) const;

    // obtains capabilities for a specific type
    _HDLLg virtual HRFRasterFileCapabilities*      GetCapabilitiesOfType(HCLASS_ID   pi_CapabilityType,
                                                                         HFCAccessMode pi_AccessMode) const;

    _HDLLg virtual HRFRasterFileCapabilities*      GetCapabilitiesOfType(HCLASS_ID   pi_CapabilityType) const;

    // Test a capabilities is present for a specific type
    _HDLLg virtual bool                           HasCapabilityOfType(HCLASS_ID   pi_CapabilityType,
                                                                       HFCAccessMode pi_AccessMode) const;

    _HDLLg virtual bool                           HasCapabilityOfType(HCLASS_ID   pi_CapabilityType) const;

    // obtains the first capability found for a specific type
    _HDLLg virtual HFCPtr<HRFCapability>           GetCapabilityOfType  (HCLASS_ID   pi_CapabilityType,
                                                                         HFCAccessMode pi_AccessMode) const;

    _HDLLg virtual HFCPtr<HRFCapability>           GetCapabilityOfType  (HCLASS_ID   pi_CapabilityType) const;

    _HDLLg virtual HFCPtr<HRFCapability>           GetCapabilityOfType  (const HFCPtr<HRFCapability>& pi_rpCapability) const;

    // Add a capability (this method is public for implemantation reason only)
    _HDLLg virtual void                            Add(const HFCPtr<HRFCapability>& pi_rpCapability);

protected:

    friend class HRFRasterFileBlockAdapterCapabilities;
    friend class HRFCombinedRasterFileCapabilities;

    // List of capability
    typedef vector<HFCPtr<HRFCapability>, allocator<HFCPtr<HRFCapability> > >
    ListOfCapability;

    ListOfCapability m_ListOfCapability;

private:

    // Methods Disabled
    HRFRasterFileCapabilities(const HRFRasterFileCapabilities& pi_rObj);
    HRFRasterFileCapabilities& operator=(const HRFRasterFileCapabilities& pi_rObj);
    };
