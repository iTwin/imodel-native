//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRACachedHistogramRaster.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HRAImageView.h"

BEGIN_IMAGEPP_NAMESPACE

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
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRACachedHistogramRasterId)


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
    virtual HFCPtr<HRARaster> Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog=0) const override;

    bool           NotifySomethingChanged(const HMGMessage& pi_rMessage);

    virtual void    ComputeHistogram(HRAHistogramOptions* pio_pOptions,
                                     bool                pi_ForceRecompute = false);

protected:

    virtual void _Draw(HGFMappedSurface& pio_destSurface, HRADrawOptions const& pi_Options) const override;

private:

    void            ClearHistogramCache();

    // The list of computed histograms.
    typedef list< HRAHistogramOptions* > HistogramList;
    HistogramList   m_Histograms;

    HMG_DECLARE_MESSAGE_MAP_DLL(IMAGEPP_EXPORT)
    };

END_IMAGEPP_NAMESPACE