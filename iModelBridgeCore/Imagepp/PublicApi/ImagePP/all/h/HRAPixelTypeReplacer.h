//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRAPixelTypeReplacer
//-----------------------------------------------------------------------------
// This class describes the interface for any kind of image view.
//-----------------------------------------------------------------------------
#pragma once

#include "HRAImageView.h"

#include "HRAStoredRaster.h"
#include "HRPAlphaRange.h"

BEGIN_IMAGEPP_NAMESPACE
class HRAPixelTypeReplacer : public HRAImageView
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRAPixelTypeReplacerId)

public:
    DEFINE_T_SUPER(HRAImageView)


    // Primary methods
    HRAPixelTypeReplacer();

    IMAGEPP_EXPORT                 HRAPixelTypeReplacer(const HFCPtr<HRARaster>& pi_pSource,
                                                const HFCPtr<HRPPixelType>& pi_pPixelType);

    HRAPixelTypeReplacer(const HRAPixelTypeReplacer& pi_rObject);

    virtual         ~HRAPixelTypeReplacer();


    // Overriden methods

    HPMPersistentObject* Clone () const override;
    virtual HFCPtr<HRARaster> Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog=0) const override;

    uint16_t GetRepresentativePalette(
        HRARepPalParms* pio_pRepPalParms) override;

    void    ComputeHistogram(HRAHistogramOptions* pio_pOptions,
                                     bool                pi_ForceRecompute = false) override;

    // messaging

    bool           NotifyContentChanged(const HMGMessage& pi_rMessage);
    bool            NotifyPaletteChanged (const HMGMessage& pi_rMessage);

    // Added methods

    HFCPtr<HRPPixelType>
    GetPixelType() const override;

    bool   ContainsPixelsWithChannel(
        HRPChannelType::ChannelRole pi_Role,
        Byte                      pi_Id) const override;

protected:
 
    virtual ImagePPStatus _CopyFrom(HRARaster& srcRaster, HRACopyFromOptions const& pi_rOptions) override;
    virtual ImagePPStatus _BuildCopyToContext(ImageTransformNodeR imageNode, HRACopyToOptionsCR options) override;


private:

    HFCPtr<HRPPixelType>    m_pPixelType;

    HMG_DECLARE_MESSAGE_MAP_DLL(IMAGEPP_EXPORT)
    };
END_IMAGEPP_NAMESPACE

