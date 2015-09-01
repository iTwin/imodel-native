//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFCombinedRasterFileCapabilities.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFCombinedRasterFileCapabilities
//-----------------------------------------------------------------------------
// This is the class used to describe the capabilities of a raster file format.
//-----------------------------------------------------------------------------

#pragma once

#include "HRFRasterFileCapabilities.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFCombinedRasterFileCapabilities : public HRFRasterFileCapabilities
    {
public:
    // creation and destruction
    HRFCombinedRasterFileCapabilities();
    HRFCombinedRasterFileCapabilities(const HFCPtr<HRFRasterFileCapabilities>& pi_rpCapabilitiesA,
                                      const HFCPtr<HRFRasterFileCapabilities>& pi_rpCapabilitiesB);

    virtual ~HRFCombinedRasterFileCapabilities();

    // test if the specific capability is supported
    virtual bool                        Supports(const HFCPtr<HRFCapability>& pi_rpCapability);

    // capabilities information "include all type of capabilities"
    virtual uint32_t                     CountCapabilities    () const;
    virtual const HFCPtr<HRFCapability>& GetCapability        (uint32_t pi_Index) const;

    // obtains capabilities for a specific type
    virtual HRFRasterFileCapabilities*   GetCapabilitiesOfType(HCLASS_ID   pi_CapabilityType,
                                                               HFCAccessMode pi_AccessMode) const;

    virtual HRFRasterFileCapabilities*   GetCapabilitiesOfType(HCLASS_ID   pi_CapabilityType) const;

    // Test a capabilities is present for a specific type
    virtual bool                        HasCapabilityOfType(HCLASS_ID   pi_CapabilityType,
                                                             HFCAccessMode pi_AccessMode) const;

    virtual bool                        HasCapabilityOfType(HCLASS_ID   pi_CapabilityType) const;

    // obtains the first capability found for a specific type
    virtual HFCPtr<HRFCapability>        GetCapabilityOfType  (HCLASS_ID   pi_CapabilityType,
                                                               HFCAccessMode pi_AccessMode) const;

    virtual HFCPtr<HRFCapability>        GetCapabilityOfType  (HCLASS_ID   pi_CapabilityType) const;

    virtual HFCPtr<HRFCapability>        GetCapabilityOfType  (const HFCPtr<HRFCapability>& pi_rpCapability) const;

protected:
    HFCPtr<HRFRasterFileCapabilities> m_pCapabilitiesA;
    HFCPtr<HRFRasterFileCapabilities> m_pCapabilitiesB;

private:
    // Methods Disabled
    HRFCombinedRasterFileCapabilities(const HRFCombinedRasterFileCapabilities& pi_rObj);
    HRFCombinedRasterFileCapabilities& operator=(const HRFCombinedRasterFileCapabilities& pi_rObj);
    };
END_IMAGEPP_NAMESPACE
