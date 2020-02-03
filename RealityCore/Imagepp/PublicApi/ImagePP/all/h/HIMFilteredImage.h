//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HIMFilteredImage
//-----------------------------------------------------------------------------
// This class describes the interface for any kind of image view.
//-----------------------------------------------------------------------------
#pragma once

#include "HRAImageView.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPFilter;
class HRPPixelNeighbourhood;

class HIMFilteredImage : public HRAImageView
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HIMFilteredImageId)

public:

    // Primary methods
    IMAGEPP_EXPORT                 HIMFilteredImage();

    IMAGEPP_EXPORT                 HIMFilteredImage(const HFCPtr<HRARaster>& pi_pSource,
                                                   const HFCPtr<HRPFilter>& pi_rpFilter,
                                                   const HFCPtr<HRPPixelType> pi_pPixelType = 0);

    IMAGEPP_EXPORT                 HIMFilteredImage(const HIMFilteredImage& pi_rFilteredImage);

    IMAGEPP_EXPORT virtual         ~HIMFilteredImage();


    // Overriden methods
    HPMPersistentObject* Clone () const override;
    virtual HFCPtr<HRARaster> Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog=0) const override;

    HFCPtr<HRPPixelType>
    GetPixelType() const override;
    virtual void    SetPixelType(const HFCPtr<HRPPixelType> pi_pPixelType);

    bool           NotifyContentChanged(HMGMessage& pi_rMessage);

    bool   ContainsPixelsWithChannel(
        HRPChannelType::ChannelRole pi_Role,
        Byte                      pi_Id) const override;

    HGF2DExtent
    GetAveragePixelSize () const override;
    void    GetPixelSizeRange(HGF2DExtent& po_rMinimum, HGF2DExtent& po_rMaximum) const override;

    // Added methods

    IMAGEPP_EXPORT HFCPtr<HRPFilter>
    GetFilter() const;

    void            InitObject();

    void            DeepDelete();

    IMAGEPP_EXPORT void            SetFilter(const HFCPtr<HRPFilter>& pi_rpFilter);

protected:
    virtual ImagePPStatus _BuildCopyToContext(ImageTransformNodeR imageNode, HRACopyToOptionsCR options) override;

private:

    HFCPtr<HRPPixelType>  m_pPixelType;
    HFCPtr<HRPFilter>     m_pFilter;

    std::list<HRAImageOpPtr> m_imageOps;
    };
END_IMAGEPP_NAMESPACE
