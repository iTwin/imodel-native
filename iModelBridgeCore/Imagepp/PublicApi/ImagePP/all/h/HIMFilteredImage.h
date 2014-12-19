//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIMFilteredImage.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HIMFilteredImage
//-----------------------------------------------------------------------------
// This class describes the interface for any kind of image view.
//-----------------------------------------------------------------------------
#pragma once

#include "HRAImageView.h"

class HGSSurface;
class HRPFilter;
class HRPPixelNeighbourhood;

class HIMFilteredImage : public HRAImageView
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1137)

public:

    // Primary methods
    _HDLLg                 HIMFilteredImage();

    _HDLLg                 HIMFilteredImage(const HFCPtr<HRARaster>& pi_pSource,
                                            const HFCPtr<HRPFilter>& pi_rpFilter,
                                            const HFCPtr<HRPPixelType> pi_pPixelType = 0);

    _HDLLg                 HIMFilteredImage(const HIMFilteredImage& pi_rFilteredImage);

    _HDLLg virtual         ~HIMFilteredImage();


    // Overriden methods
    virtual HPMPersistentObject* Clone () const;
    virtual HRARaster* Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog=0) const;

    virtual HFCPtr<HRPPixelType>
    GetPixelType() const;
    virtual void    SetPixelType(const HFCPtr<HRPPixelType> pi_pPixelType);

    bool           NotifyContentChanged(HMGMessage& pi_rMessage);

    virtual bool   ContainsPixelsWithChannel(
        HRPChannelType::ChannelRole pi_Role,
        Byte                      pi_Id) const;

    virtual HGF2DExtent
    GetAveragePixelSize () const;
    virtual void    GetPixelSizeRange(HGF2DExtent& po_rMinimum, HGF2DExtent& po_rMaximum) const;

    virtual void    PreDraw(HRADrawOptions* pio_pOptions);

    virtual void    Draw(const HFCPtr<HGFMappedSurface>& pio_rpSurface, const HGFDrawOptions* pi_pOptions) const;


    // Added methods

    _HDLLg HFCPtr<HRPFilter>
    GetFilter() const;

    void            InitObject();

    void            DeepDelete();

    _HDLLg void            SetFilter(const HFCPtr<HRPFilter>& pi_rpFilter);

protected:


private:

    void            HandleBorderCases(const HFCPtr<HGSSurface>&    pio_rpSurface,
                                      const HRPPixelNeighbourhood& pi_rNeighborhood) const;

    HFCPtr<HRPPixelType>  m_pPixelType;
    HFCPtr<HRPFilter>     m_pFilter;
    };