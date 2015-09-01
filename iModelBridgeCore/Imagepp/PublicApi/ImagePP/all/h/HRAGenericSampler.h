//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAGenericSampler.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>-----------------------------------------------------------------------------
//:> Class : HRAGenericSampler
//:>-----------------------------------------------------------------------------
//:> General class for HRASampler method.
//:>-----------------------------------------------------------------------------

#pragma once

#include "HGF2DRectangle.h"

BEGIN_IMAGEPP_NAMESPACE

class HRPPixelType;
class HGSMemoryBaseSurfaceDescriptor;

class HNOVTABLEINIT HRAGenericSampler
    {
    HDECLARE_BASECLASS_ID(HRAGenericId_Sampler)


public:

    // Primary methods
    HRAGenericSampler(HGSMemoryBaseSurfaceDescriptor const& pi_rMemorySurface,
                      const HGF2DRectangle&                 pi_rSampleDimension,
                      double                                pi_DeltaX,
                      double                                pi_DeltaY);
    virtual        ~HRAGenericSampler();

    virtual void const* GetPixel(double            pi_PosX,
                                 double            pi_PosY) const = 0;

    virtual void    GetPixels(const double*    pi_pPositionsX,
                              const double*    pi_pPositionsY,
                              size_t            pi_PixelCount,
                              void*             po_pBuffer) const = 0;

    virtual void    GetPixels(double           pi_PositionX,
                              double           pi_PositionY,
                              size_t            pi_PixelCount,
                              void*             po_pBuffer) const = 0;

    virtual HFCPtr<HRPPixelType>
    GetOutputPixelType() const;

    virtual bool   TryToUse(const HFCPtr<HRPPixelType>& pi_rpOutputPixelType);

    void            SetScale(double            pi_ScaleX,
                             double            pi_ScaleY);

protected:

    double         m_DeltaX;
    double         m_DeltaY;

    uint32_t        m_Width;
    uint32_t        m_Height;

    HFCPtr<HRPPixelType> m_PixelType;

    // optimization
    bool           m_StretchByLine;

    const HGF2DRectangle&    GetSampleDimension() const;

private:

    // Dimensions of a sample in source pixels.
    HGF2DRectangle           m_SampleDimension;

    //:> disabled methods
    HRAGenericSampler(const HRAGenericSampler& pi_rObj);
    HRAGenericSampler&
    operator=(const HRAGenericSampler& pi_rObj);
    };


END_IMAGEPP_NAMESPACE