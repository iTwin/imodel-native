//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRASampler.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>-----------------------------------------------------------------------------
//:> Class : HRASampler
//:>-----------------------------------------------------------------------------
//:> General class for sampler.
//:>-----------------------------------------------------------------------------

#pragma once

#include "HRAGenericSampler.h"

BEGIN_IMAGEPP_NAMESPACE
class HGSMemoryBaseSurfaceDescriptor;
class HGSResampling;

class HRASampler
    {
    HDECLARE_BASECLASS_ID(HRASamplerId_Base)

public:

    // Primary methods
    HRASampler(HGSResampling const&                  pi_SamplingMode,
               HGSMemoryBaseSurfaceDescriptor const& pi_pSurfaceImplementation,
               const HGF2DRectangle&                 pi_rSampleDimension,
               double                                pi_DeltaX,
               double                                pi_DeltaY);
    virtual        ~HRASampler();

    void const* GetPixel(double pi_PosX, double pi_PosY) const;

    void            GetPixels(const double*    pi_pPositionsX,
                              const double*    pi_pPositionsY,
                              size_t            pi_PixelCount,
                              void*             po_pBuffer) const;

    void            GetPixels(double           pi_PositionX,
                              double           pi_PositionY,
                              size_t            pi_PixelCount,
                              void*             po_pBuffer) const;

    HFCPtr<HRPPixelType> GetOutputPixelType() const;

    bool           TryToUse(const HFCPtr<HRPPixelType>& pi_rpOutputPixelType);

    void            SetScale(double            pi_ScaleX,
                             double            pi_ScaleY);

private:

    HAutoPtr<HRAGenericSampler> m_pSampler;

    mutable double         m_PosY;
    mutable double         m_PosX;

    // disabled methods
    HRASampler();
    HRASampler(const HRASampler& pi_rObj);
    HRASampler&     operator=(const HRASampler& pi_rObj);
    };
END_IMAGEPP_NAMESPACE

#include "HRASampler.hpp"








