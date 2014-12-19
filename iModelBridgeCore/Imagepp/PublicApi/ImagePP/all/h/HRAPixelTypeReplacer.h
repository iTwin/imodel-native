//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAPixelTypeReplacer.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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

class HRAPixelTypeReplacer : public HRAImageView
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1210)

public:
    DEFINE_T_SUPER(HRAImageView)


    // Primary methods
    HRAPixelTypeReplacer();

    _HDLLg                 HRAPixelTypeReplacer(const HFCPtr<HRARaster>& pi_pSource,
                                                const HFCPtr<HRPPixelType>& pi_pPixelType);

    HRAPixelTypeReplacer(const HRAPixelTypeReplacer& pi_rObject);

    virtual         ~HRAPixelTypeReplacer();


    // Overriden methods

    virtual HPMPersistentObject* Clone () const;
    virtual HRARaster* Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog=0) const;

    virtual unsigned short GetRepresentativePalette(
        HRARepPalParms* pio_pRepPalParms);

    virtual void    ComputeHistogram(HRAHistogramOptions* pio_pOptions,
                                     bool                pi_ForceRecompute = false);

    // messaging

    bool           NotifyContentChanged(const HMGMessage& pi_rMessage);
    bool            NotifyPaletteChanged (const HMGMessage& pi_rMessage);

    // Added methods

    virtual HFCPtr<HRPPixelType>
    GetPixelType() const;

    virtual bool   ContainsPixelsWithChannel(
        HRPChannelType::ChannelRole pi_Role,
        Byte                      pi_Id) const;

    virtual void    PreDraw(HRADrawOptions* pio_pOptions);

    virtual void    Draw(const HFCPtr<HGFMappedSurface>& pio_pSurface, const HGFDrawOptions* pi_pOptions) const;

    virtual void    CopyFrom   (const HFCPtr<HRARaster>& pi_pSrcRaster, const HRACopyFromOptions& pi_rOptions) override;
    virtual void    CopyFrom   (const HFCPtr<HRARaster>& pi_pSrcRaster) override;

protected:


private:

    HFCPtr<HRPPixelType>    m_pPixelType;

    HMG_DECLARE_MESSAGE_MAP_DLL(_HDLLg)
    };

