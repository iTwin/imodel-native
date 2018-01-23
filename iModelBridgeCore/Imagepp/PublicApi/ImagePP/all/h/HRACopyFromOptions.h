//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRACopyFromOptions.h $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRACopyFromOptions
//-----------------------------------------------------------------------------
#pragma once

#include "HGF2DTransfoModel.h"
#include "HGF2DExtent.h"
#include "HVEShape.h"
#include "HRPFilter.h"
#include "HGSTypes.h"

BEGIN_IMAGEPP_NAMESPACE

struct HRACopyFromOptionsImpl;

/*---------------------------------------------------------------------------------**//**
* To ignore source clip shape, set the effective copy region to the destination's effective shape.
*   copyFromOptions.SetEffectiveCopyRegion(dst->GetEffectiveShape());
*   dst->CopyFrom(src, copyFromOptions);
* To ignore src clip and provide an additional dst clip.
*   dstShape->Intersect(dst->GetEffectiveShape());
*   copyFromOptions.SetEffectiveCopyRegion(dstShape.GetPtr());
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct HRACopyFromOptions
{
    IMAGEPP_EXPORT HRACopyFromOptions(bool alphaBlend = false);
    IMAGEPP_EXPORT ~HRACopyFromOptions();

    //! Query if alpha blend is enabled
    IMAGEPP_EXPORT bool ApplyAlphaBlend() const;

    //! Enable or disabled alpha blending.
    IMAGEPP_EXPORT void SetAlphaBlend(bool alphaBlend);

    //! Get/Set a replacing pixeltype. Usually for palette rasters. Pixels are not physically changed only their interpretation is.
    IMAGEPP_EXPORT HFCPtr<HRPPixelType> GetDestReplacingPixelType() const;
    IMAGEPP_EXPORT void SetDestReplacingPixelType(const HFCPtr<HRPPixelType>& pPixelType);

    //! Get/Set an extra destination shape. If an effectiveCopyRegion is set this parameter is ignored.
    IMAGEPP_EXPORT HVEShape const* GetDestShape() const;
    IMAGEPP_EXPORT void SetDestShape(HVEShape const* pShape);

    //! Get/Set the effective copy region. Is it assumed that the effective copy region is the result of the source and destination intersection
    //! and every shaping operation is included.
    //! If effectiveCopyRegion is non NULL DestShape is ignored.
    //! 
    //! If NULL, the effective copy region is calculated by the Non-virtual ::CopyFrom, it then call the virtual ::_CopyFrom where the effective copy 
    //! region is assumed to be non-NULL. In the virtual ::_CopyFrom the 'DestShape' is ignored since it should have been included when computing the 
    //! effective copy region.
    //! This could be used to replace HRACopyFromLegacyOptions::ApplySourceClipping(), by providing immediately the effective region.
    IMAGEPP_EXPORT HVEShape const* GetEffectiveCopyRegion() const;
    IMAGEPP_EXPORT void SetEffectiveCopyRegion(HVEShape const* pShape);

    //! Sampling method to use when pixel transformation is involved. If unsupported a lower quality method is automatically selected.
    IMAGEPP_EXPORT HGSResampling const& GetResamplingMode() const;
    IMAGEPP_EXPORT void                 SetResamplingMode(HGSResampling const& resampling);
    
    HRACopyFromOptions(HRACopyFromOptions const& opts);
    HRACopyFromOptions& operator=(const HRACopyFromOptions& pi_rObj);
private:   

    std::unique_ptr<HRACopyFromOptionsImpl> m_pImpl; // Hide implementation to allow extension.
};

END_IMAGEPP_NAMESPACE

