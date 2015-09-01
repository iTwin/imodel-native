//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRANearestSamplerRLE1.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HRANearestSampler.h"

BEGIN_IMAGEPP_NAMESPACE
class HGSMemorySurfaceDescriptor;
class HGSMemoryRLESurfaceDescriptor;
class HCDPacket;
class HCDPacketRLE;

//:>-----------------------------------------------------------------------------
//:> Class : HRANearestSamplerRLE1Base
//:>-----------------------------------------------------------------------------
class HRANearestSamplerRLE1Base : public HRANearestSampler
    {
    HDECLARE_CLASS_ID(HRANearestSamplerId_RLE1Base, HRANearestSampler)

public:

    // Primary methods
    HRANearestSamplerRLE1Base(HGSMemoryBaseSurfaceDescriptor const& pi_rMemorySurface,
                              const HGF2DRectangle&                 pi_rSampleDimension,
                              double                                pi_DeltaX,
                              double                                pi_DeltaY);
    virtual         ~HRANearestSamplerRLE1Base();

    virtual void const* GetPixel(double pi_PosX, double pi_PosY) const override;

    virtual void    GetPixels(const double*    pi_pPositionsX,
                              const double*    pi_pPositionsY,
                              size_t            pi_PixelCount,
                              void*             po_pBuffer) const override;

    virtual void    GetPixels(double           pi_PositionX,
                              double           pi_PositionY,
                              size_t            pi_PixelCount,
                              void*             po_pBuffer) const override;

protected:

    virtual Byte*     GetLineBufferAddress(HUINTX pi_PosY) const = 0;
    virtual size_t      GetLineDataSize(HUINTX pi_PosY) const = 0;

private:

    struct RLEBufferPosition
        {
        size_t m_IndexInBuffer;     // Index in RLE buffer
        size_t m_StartPosition;     // Pixel start position for indexInBuffer.
        };

    bool               m_ForegroundMode;
    bool               m_ForegroundState;

    // Temporary data used in GetPixel()
    mutable unsigned short m_aData[2];

    // optimization
    bool               m_StretchByLine;
    bool               m_StretchByLineWithNoScale;
    bool               m_StretchLineByTwo;
    bool               m_ReverseLine;

    mutable HArrayAutoPtr<RLEBufferPosition>
    m_pLastRLEBufferPosition;

    unsigned short* ComputeAddress(HUINTX   pi_PosX,
                            HUINTX   pi_PosY,
                            uint32_t*  po_pPixelsToSkipInFirstLen,
                            uint32_t*  po_pPixelsToSkipInSecondLen,
                            uint32_t*  po_pRunPos) const;

    bool    IsPixelOn(uint32_t pi_PosX, uint32_t pi_PosY) const;

    // disabled methods
    HRANearestSamplerRLE1Base();
    HRANearestSamplerRLE1Base(const HRANearestSamplerRLE1Base& pi_rObj);
    HRANearestSamplerRLE1Base&      operator=(const HRANearestSamplerRLE1Base& pi_rObj);
    };


//:>-----------------------------------------------------------------------------
//:> Class : HRANearestSamplerRLE1
//:>-----------------------------------------------------------------------------
class HRANearestSamplerRLE1 : public HRANearestSamplerRLE1Base
    {
    HDECLARE_CLASS_ID(HRANearestSamplerId_RLE1, HRANearestSamplerRLE1Base)

public:
    HRANearestSamplerRLE1(HGSMemorySurfaceDescriptor const&    pi_rMemorySurface,
                          const HGF2DRectangle&                pi_rSampleDimension,
                          double                               pi_DeltaX,
                          double                               pi_DeltaY);

protected:

    virtual Byte* GetLineBufferAddress(HUINTX pi_PosY) const override;
    virtual size_t  GetLineDataSize(HUINTX pi_PosY) const override;

private:
    HFCPtr<HCDPacket>   m_pPacket;
    uint32_t*             m_pLineIndexes;
    uint32_t            m_LineIndexesCount;

    // disabled methods
    HRANearestSamplerRLE1();
    HRANearestSamplerRLE1(const HRANearestSamplerRLE1& pi_rObj);
    HRANearestSamplerRLE1&      operator=(const HRANearestSamplerRLE1& pi_rObj);
    };

//:>-----------------------------------------------------------------------------
//:> Class : HRANearestSamplerRLE1Line
//:>-----------------------------------------------------------------------------
class HRANearestSamplerRLE1Line : public HRANearestSamplerRLE1Base
    {
    HDECLARE_CLASS_ID(HRANearestSamplerId_RLE1Line, HRANearestSamplerRLE1Base)

public:
    HRANearestSamplerRLE1Line(HGSMemoryRLESurfaceDescriptor const& pi_rMemorySurface,
                              const HGF2DRectangle&                pi_rSampleDimension,
                              double                               pi_DeltaX,
                              double                               pi_DeltaY);

protected:

    virtual Byte* GetLineBufferAddress(HUINTX pi_PosY) const override;
    virtual size_t  GetLineDataSize(HUINTX pi_PosY) const override;

private:
    HFCPtr<HCDPacketRLE> m_pPacketRLE;

    // disabled methods
    HRANearestSamplerRLE1Line();
    HRANearestSamplerRLE1Line(const HRANearestSamplerRLE1Line& pi_rObj);
    HRANearestSamplerRLE1Line&      operator=(const HRANearestSamplerRLE1Line& pi_rObj);
    };
END_IMAGEPP_NAMESPACE
