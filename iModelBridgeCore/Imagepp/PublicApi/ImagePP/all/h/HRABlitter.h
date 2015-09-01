//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRABlitter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HGSTypes.h>

BEGIN_IMAGEPP_NAMESPACE
class HRASurface;
class HRPPixelType;
class HGF2DStretch;
class HRATransaction;
class HRASurface;
class HRPFilter;
class HGF2DTransfoModel;

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
class HRABlitter
    {
public:

    // Primary methods
    HRABlitter(HRASurface& pi_destSurface);

    ~HRABlitter();

    void BlitFrom(const HRASurface&                  pi_srcSurface,
                  const HGF2DTransfoModel&           pi_rTransfoModel,
                  HRATransaction*                    pi_pTransaction = 0);

    void SetAlphaBlend(bool alphaBlend);
    void SetGridMode(bool gridMode);
    void SetSamplingMode(HGSResampling const& samplingMode);
    void SetOverviewsMode(bool overviews);
    void SetFilter(HFCPtr<HRPFilter> const& pFilter);

protected:

private:
    HRASurface& GetDestSurface() const {return m_destSurface;}

    void            Optimized8BitsBlit(const HRASurface&                pi_srcSurface,
                                       const HGF2DStretch&              pi_rTransfoModel,
                                       HRATransaction*                  pi_pTransaction);

    void            NormalBlit        (const HRASurface&                pi_srcSurface,
                                       const HGF2DStretch&              pi_rTransfoModel,
                                       HRATransaction*                  pi_pTransaction);

    HRASurface*     ApplyFilter(const HRASurface&                       pi_srcSurface,
                                const HFCPtr<HRPFilter>&                pi_rpFilter) const;

    Byte*          CreateWorkingBuffer(const HRPPixelType&             pi_rPixelType,
                                       uint32_t                        pi_Width,
                                       uint32_t                        pi_Height) const;
    
    HRASurface&    m_destSurface;

    bool           m_ComposeRequired;
    bool           m_ApplyGrid;
    HGSResampling  m_samplingMode;
    bool           m_overviewsMode;
    HFCPtr<HRPFilter>   m_pFilter;

    // disabled methods
    HRABlitter(const HRABlitter& pi_rObj);
    HRABlitter&  operator=(const HRABlitter& pi_rObj);
    };

END_IMAGEPP_NAMESPACE