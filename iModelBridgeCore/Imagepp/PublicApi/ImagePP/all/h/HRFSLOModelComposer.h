//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFSLOModelComposer.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFSLOModelComposer
//-----------------------------------------------------------------------------

#pragma once

#include "HRFTypes.h"
#include "HGF2DPosition.h"

BEGIN_IMAGEPP_NAMESPACE
class HGF2DTransfoModel;
class HGF2DAffine;

class HRFSLOModelComposer
    {
public:

    // Rpimary methods
    HRFSLOModelComposer(HRFScanlineOrientation pi_ScanlineOrientation
                        = HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL);
    HRFSLOModelComposer(const HRFSLOModelComposer& pi_rObj);

    HRFSLOModelComposer&
    operator=(const HRFSLOModelComposer&);
    // Others
    HFCPtr<HGF2DTransfoModel>
    GetTransfoModelFrom(HRFScanlineOrientation pi_ScanlineOrientation,
                        uint32_t pi_Width,
                        uint32_t pi_Height,
                        HGF2DPosition& pi_Origin);

    HFCPtr<HGF2DAffine>
    GetIntergraphTransfoModelFrom(HRFScanlineOrientation  pi_ScanlineOrientation,
                                  uint32_t pi_Width,
                                  uint32_t pi_Height,
                                  HGF2DPosition& pi_Origin);

    HRFScanlineOrientation
    GetScanLineOrigin() const;

protected:

private:

    // Attributes
    HRFScanlineOrientation
    m_ScanLineOrigin;

    // Private methods
    HFCPtr<HGF2DAffine> GetTransfoModelToSLO4   (HRFScanlineOrientation   pi_ScanlineOrientation,
                                                 uint32_t                 pi_Width,
                                                 uint32_t                 pi_Height,
                                                 HGF2DPosition&           pi_Origin) const;

    HFCPtr<HGF2DAffine> GetTransfoModelFromSLO4 (HRFScanlineOrientation   pi_ScanlineOrientation,
                                                 uint32_t                 pi_Width,
                                                 uint32_t                 pi_Height,
                                                 HGF2DPosition&           pi_Origin) const;

    void            DeepCopy(const HRFSLOModelComposer& pi_rObj);
    void            DeepDelete();


    };
END_IMAGEPP_NAMESPACE
