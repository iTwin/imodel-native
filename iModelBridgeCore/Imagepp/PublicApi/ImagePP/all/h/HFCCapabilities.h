//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCCapabilities.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCCapabilities
//-----------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"
#include "HFCCapability.h"


BEGIN_IMAGEPP_NAMESPACE
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
    HDECLARE_BASECLASS_ID(HFCCapabilitiesId_Base)

public:
    // creation and destruction
    IMAGEPP_EXPORT                     HFCCapabilities();
    IMAGEPP_EXPORT                     HFCCapabilities(const HFCCapabilities& pi_rObj);
    IMAGEPP_EXPORT virtual             ~HFCCapabilities();

    // test if the specific capability is supported
    IMAGEPP_EXPORT virtual bool       Supports(const HFCCapability&   pi_rCapability) const;
    IMAGEPP_EXPORT virtual bool       Supports(const HFCCapabilities& pi_rpCapabilities) const;

    // capabilities information "include all type of capabilities"
    IMAGEPP_EXPORT virtual uint32_t    CountCapabilities() const;
    IMAGEPP_EXPORT virtual const HFCPtr<HFCCapability>&
    GetCapability(uint32_t pi_Index) const;

    // Test a capabilities is present for a specific type
    IMAGEPP_EXPORT virtual bool       HasCapabilityOfType(const HFCCapability& pi_rCapability) const;

    // obtains the first capability found for a specific type
    IMAGEPP_EXPORT virtual HFCPtr<HFCCapabilities>
    GetCapabilitiesOfType(const HFCCapability& pi_rCapability) const;

    IMAGEPP_EXPORT virtual void        Add(const HFCPtr<HFCCapability>& pi_rCapability);

protected:

    // List of capability
    typedef vector<HFCPtr<HFCCapability>, allocator<HFCPtr<HFCCapability> > >
    ListOfCapability;

    ListOfCapability    m_ListOfCapability;


private:

    };

END_IMAGEPP_NAMESPACE