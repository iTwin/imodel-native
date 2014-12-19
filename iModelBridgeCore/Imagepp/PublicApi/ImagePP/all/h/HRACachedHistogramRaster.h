//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRACachedHistogramRaster.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HRAImageView.h"


/** -----------------------------------------------------------------------------
    This is a wrapper class whose sole purpose is to cache a list of
    already computed histograms. It does nothing else.

    We will only cache histograms that have a specified region to scan. Those
    that don't have a region will be dealt with normally. That is, they will
    be stored and reused as defined in the HRARaster class.

    The histograms are merely cached in memory, never on a permanent storage.
    All the histograms are kept, there is no memory management mechanism.
    -----------------------------------------------------------------------------
*/
class HRACachedHistogramRaster : public HRAImageView
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1276)


public:

    //:> Primary methods
    HRACachedHistogramRaster();

    HRACachedHistogramRaster(const HFCPtr<HRARaster>& pi_pSource);

    HRACachedHistogramRaster(const HRACachedHistogramRaster& pi_rObj);
    HRACachedHistogramRaster&
    operator=(const HRACachedHistogramRaster& pi_rObj);

    virtual         ~HRACachedHistogramRaster();


    //:> Overriden methods
    virtual HPMPersistentObject* Clone () const;
    virtual HRARaster* Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog=0) const;

    bool           NotifySomethingChanged(const HMGMessage& pi_rMessage);

    virtual void    ComputeHistogram(HRAHistogramOptions* pio_pOptions,
                                     bool                pi_ForceRecompute = false);

    virtual void    PreDraw(HRADrawOptions* pio_pOptions);

    virtual void    Draw(const HFCPtr<HGFMappedSurface>& pio_pSurface, const HGFDrawOptions* pi_pOptions) const;

private:

    void            ClearHistogramCache();

    // The list of computed histograms.
    typedef list< HRAHistogramOptions* > HistogramList;
    HistogramList   m_Histograms;

    HMG_DECLARE_MESSAGE_MAP_DLL(_HDLLNone)
    };

