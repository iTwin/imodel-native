//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRANearestSamplerN8.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>-----------------------------------------------------------------------------
//:> Class : HRANearestSamplerN8
//:>-----------------------------------------------------------------------------

#pragma once

#include "HRANearestSampler.h"

class HCDPacket;
class HGSMemorySurfaceDescriptor;


class HRANearestSamplerN8 : public HRANearestSampler
    {
    HDECLARE_CLASS_ID(1753, HRANearestSampler)

public:

    // Primary methods
    HRANearestSamplerN8(HGSMemorySurfaceDescriptor const&  pi_rMemorySurface,
                        const HGF2DRectangle&              pi_rSampleDimension,
                        double                             pi_DeltaX,
                        double                             pi_DeltaY);
    virtual         ~HRANearestSamplerN8();

    virtual void*   GetPixel(double             pi_PosX,
                             double             pi_PosY) const;

    virtual void    GetPixels(const double*     pi_pPositionsX,
                              const double*     pi_pPositionsY,
                              size_t            pi_PixelCount,
                              void*             po_pBuffer) const;

    virtual void    GetPixels(double            pi_PositionX,
                              double            pi_PositionY,
                              size_t            pi_PixelCount,
                              void*             po_pBuffer) const;


protected:


private:

    uint32_t            m_DataWidth;
    uint32_t            m_DataHeight;
    uint32_t            m_BytesPerPixel;
    size_t              m_BytesPerLine;
    bool               m_SLO4;
    HFCPtr<HCDPacket>   m_pPacket;

    bool               m_StretchByLine;
    bool               m_StretchByLineWithNoScale;
    bool               m_StretchLineByTwo;

    Byte* ComputeAddress(HUINTX  pi_PosX,
                           HUINTX  pi_PosY) const;


    // disabled methods
    HRANearestSamplerN8();
    HRANearestSamplerN8(const HRANearestSamplerN8& pi_rObj);
    HRANearestSamplerN8&      operator=(const HRANearestSamplerN8& pi_rObj);
    };


