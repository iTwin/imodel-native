//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIMStripAdapter.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HIMStripAdapter
//-----------------------------------------------------------------------------
// This class describes the interface for any kind of image view.
//-----------------------------------------------------------------------------
#pragma once

#include "HRAImageView.h"
#include "HRABitmap.h"

class HIMStripAdapterIterator;


class HIMStripAdapter : public HRAImageView
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1760)

    friend class HIMStripAdapterIterator;

public:

    //:> Primary methods
    _HDLLg                 HIMStripAdapter();

    _HDLLg                 HIMStripAdapter(    const HFCPtr<HRARaster>&    pi_rpSource,
                                               const Byte*               pi_pRGBBackgroundColor,
                                               double                     pi_QualityFactor = 1.0,
                                               size_t                      pi_MaxSizeInBytes = (1024 * 1024L));

    _HDLLg                 HIMStripAdapter(const HFCPtr<HRARaster>&    pi_rpSource,
                                           const HFCPtr<HRPPixelType>& pi_rpStripPixelType,
                                           double                     pi_QualityFactor = 1.0,
                                           size_t                      pi_MaxSizeInBytes = (1024 * 1024L));

    _HDLLg                 HIMStripAdapter(    const HFCPtr<HRARaster>& pi_rpSource,
                                               const Byte*            pi_pRGBBackgroundColor,
                                               uint32_t                 pi_StripWidth,
                                               uint32_t                 pi_StripHeigth,
                                               HRABitmap::SLO           pi_SLO);

    _HDLLg                 HIMStripAdapter(const HIMStripAdapter& pi_rObj);

    _HDLLg virtual         ~HIMStripAdapter();


    //:> Overriden methods
    virtual HPMPersistentObject* Clone () const;
    virtual HRARaster* Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog=0) const;

    virtual HRARasterIterator*
    CreateIterator (const HRAIteratorOptions& pi_rOptions = HRAIteratorOptions()) const;

    virtual bool   IsStoredRaster  () const;

    virtual HFCPtr<HRPPixelType>
    GetPixelType() const;

    virtual void    PreDraw(HRADrawOptions* pio_pOptions);

    virtual void    Draw(const HFCPtr<HGFMappedSurface>& pio_pSurface, const HGFDrawOptions* pi_pOptions) const;

    //:> Added methods

    HFCPtr<HRABitmap>
    GetInputBitmapExample() const;

    size_t          GetMaxSizeInBytes() const;
    void            SetMaxSizeInBytes(size_t pi_MaxSize);

    double         GetQualityFactor() const;
    void            SetQualityFactor(double pi_Factor);

    bool           StripsWillBeClipped() const;
    void            ClipStripsBasedOnSource(bool pi_Clip);

protected:


private:
    size_t                m_MaxSizeInBytes;
    double               m_QualityFactor;
    HFCPtr<HRABitmap>     m_pInputBitmapExample;
    bool                 m_ApplyClipping;

    uint32_t              m_StripWidth;
    uint32_t              m_StripHeigth;

    uint32_t    SetBackgroundColor  (const HFCPtr<HRPPixelType>& pio_pPixelType,
                                     const HFCPtr<HRARaster>&    pi_pSource,
                                     const Byte*               pi_pRGBBackgroundColor,
                                     HRABitmap::SLO              pi_SLO);
    };

#include "HIMStripAdapter.hpp"
