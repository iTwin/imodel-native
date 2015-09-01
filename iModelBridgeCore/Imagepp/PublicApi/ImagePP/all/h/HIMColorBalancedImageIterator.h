//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIMColorBalancedImageIterator.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HRARasterIterator.h"

BEGIN_IMAGEPP_NAMESPACE
class HRARaster;
class HRPPixelBuffer;
class HIMColorBalancedImage;

/** -----------------------------------------------------------------------------
    This is the iterator for the HIMColorBalancedImage class. The actual
    color balancing is applied here, one source raster part at a time.

    @see HIMColorBalancedImage
    -----------------------------------------------------------------------------
*/
class HIMColorBalancedImageIterator: public HRARasterIterator
    {
public:

    // Primary methods

    HIMColorBalancedImageIterator(const HFCPtr<HIMColorBalancedImage>& pi_pImage,
                                  const HRAIteratorOptions&       pi_rOptions);

    HIMColorBalancedImageIterator(const HIMColorBalancedImageIterator& pi_rObj);

    HIMColorBalancedImageIterator&
    operator=(const HIMColorBalancedImageIterator& pi_rObj);

    virtual         ~HIMColorBalancedImageIterator();

    // Iterator operation

    virtual const HFCPtr<HRARaster>&
    Next();

    virtual const HFCPtr<HRARaster>&
    operator()();

    virtual void    Reset();

protected:

private:

    // Internal iterator on source raster.
    HRARasterIterator*    m_pSrcIterator;

    // Current raster
    HFCPtr<HRARaster>   m_pCurRaster;

    // private methods

    void                InitObject();
    HFCPtr<HRARaster>   BalanceRaster(const HFCPtr<HRARaster> pi_pRaster);

    // General balance application (global + positional)
    void                ApplyColorBalanceRGB(HRPPixelBuffer* pi_pSourcePixels,
                                             HRPPixelBuffer* pi_pDestPixels,
                                             int32_t pi_OriginX,
                                             int32_t pi_OriginY,
                                             const HGF2DExtent& pi_rTotalExtent,
                                             int32_t pi_DisplacementX,
                                             int32_t pi_DisplacementY) ;

    // General balance application (positional)
    void                ApplyColorBalanceRGBPositionalOnly(HRPPixelBuffer* pi_pSourcePixels,
                                                           HRPPixelBuffer* pi_pDestPixels,
                                                           int32_t pi_OriginX,
                                                           int32_t pi_OriginY,
                                                           const HGF2DExtent& pi_rTotalExtent,
                                                           int32_t pi_DisplacementX,
                                                           int32_t pi_DisplacementY) ;

    // Balance application specific for 4 neighbors (global + positional)
    void                ApplyColorBalanceRGB4(HRPPixelBuffer* pi_pSourcePixels,
                                              HRPPixelBuffer* pi_pDestPixels,
                                              int32_t pi_OriginX,
                                              int32_t pi_OriginY,
                                              const HGF2DExtent& pi_rTotalExtent,
                                              int32_t pi_DisplacementX,
                                              int32_t pi_DisplacementY) ;

    // Balance application specific for 4 neighbors (positional)
    void                ApplyColorBalanceRGB4PositionalOnly(HRPPixelBuffer* pi_pSourcePixels,
                                                            HRPPixelBuffer* pi_pDestPixels,
                                                            int32_t pi_OriginX,
                                                            int32_t pi_OriginY,
                                                            const HGF2DExtent& pi_rTotalExtent,
                                                            int32_t pi_DisplacementX,
                                                            int32_t pi_DisplacementY) ;

    // Balance application (global)
    void                ApplyColorBalanceRGBGlobalOnly(HRPPixelBuffer* pi_pSourcePixels,
                                                       HRPPixelBuffer* pi_pDestPixels,
                                                       int32_t pi_DisplacementX,
                                                       int32_t pi_DisplacementY);

    // General balance application (global + positional)
    void                ApplyColorBalanceGray(HRPPixelBuffer* pi_pSourcePixels,
                                              HRPPixelBuffer* pi_pDestPixels,
                                              int32_t pi_OriginX,
                                              int32_t pi_OriginY,
                                              const HGF2DExtent& pi_rTotalExtent,
                                              int32_t pi_DisplacementX,
                                              int32_t pi_DisplacementY) ;

    // General balance application (positional)
    void                ApplyColorBalanceGrayPositionalOnly(HRPPixelBuffer* pi_pSourcePixels,
                                                            HRPPixelBuffer* pi_pDestPixels,
                                                            int32_t pi_OriginX,
                                                            int32_t pi_OriginY,
                                                            const HGF2DExtent& pi_rTotalExtent,
                                                            int32_t pi_DisplacementX,
                                                            int32_t pi_DisplacementY) ;

    // Balance application specific for 4 neighbors (global + positional)
    void                ApplyColorBalanceGray4(HRPPixelBuffer* pi_pSourcePixels,
                                               HRPPixelBuffer* pi_pDestPixels,
                                               int32_t pi_OriginX,
                                               int32_t pi_OriginY,
                                               const HGF2DExtent& pi_rTotalExtent,
                                               int32_t pi_DisplacementX,
                                               int32_t pi_DisplacementY) ;

    // Balance application specific for 4 neighbors (positional)
    void                ApplyColorBalanceGray4PositionalOnly(HRPPixelBuffer* pi_pSourcePixels,
                                                             HRPPixelBuffer* pi_pDestPixels,
                                                             int32_t pi_OriginX,
                                                             int32_t pi_OriginY,
                                                             const HGF2DExtent& pi_rTotalExtent,
                                                             int32_t pi_DisplacementX,
                                                             int32_t pi_DisplacementY) ;

    // Balance application (global)
    void                ApplyColorBalanceGrayGlobalOnly(HRPPixelBuffer* pi_pSourcePixels,
                                                        HRPPixelBuffer* pi_pDestPixels,
                                                        int32_t pi_DisplacementX,
                                                        int32_t pi_DisplacementY) ;

    };
END_IMAGEPP_NAMESPACE