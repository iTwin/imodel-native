//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCCapabilities.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCCapabilities
//-----------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"
#include "HFCCapability.h"



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
class HFCCapabilities : public HFCShareableObject<HFCCapabilities>
    {
    HDECLARE_BASECLASS_ID(1383)

public:
    // creation and destruction
    _HDLLu                     HFCCapabilities();
    _HDLLu                     HFCCapabilities(const HFCCapabilities& pi_rObj);
    _HDLLu virtual             ~HFCCapabilities();

    // test if the specific capability is supported
    _HDLLu virtual bool       Supports(const HFCCapability&   pi_rCapability) const;
    _HDLLu virtual bool       Supports(const HFCCapabilities& pi_rpCapabilities) const;

    // capabilities information "include all type of capabilities"
    _HDLLu virtual uint32_t    CountCapabilities() const;
    _HDLLu virtual const HFCPtr<HFCCapability>&
    GetCapability(uint32_t pi_Index) const;

    // Test a capabilities is present for a specific type
    _HDLLu virtual bool       HasCapabilityOfType(const HFCCapability& pi_rCapability) const;

    // obtains the first capability found for a specific type
    _HDLLu virtual HFCPtr<HFCCapabilities>
    GetCapabilitiesOfType(const HFCCapability& pi_rCapability) const;

    _HDLLu virtual void        Add(const HFCPtr<HFCCapability>& pi_rCapability);

protected:

    // List of capability
    typedef vector<HFCPtr<HFCCapability>, allocator<HFCPtr<HFCCapability> > >
    ListOfCapability;

    ListOfCapability    m_ListOfCapability;


private:

    };
