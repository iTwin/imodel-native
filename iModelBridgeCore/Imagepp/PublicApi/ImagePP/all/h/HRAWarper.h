//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAWarper.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HGSTypes.h>

BEGIN_IMAGEPP_NAMESPACE
class HRATransaction;
class HGSResampling;
class HRASurface;
class HRPFilter;
class HRPPixelType;
class HGF2DTransfoModel;

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
class HRAWarper
    {
public:

    // Primary methods
    HRAWarper(HRASurface& pi_destSurface);

    ~HRAWarper();


    void    WarpFrom(const HRASurface&                  pi_srcSurface,
                     const HGF2DTransfoModel&           pi_rTransfoModel,
                     HRATransaction*                    pi_pTransaction = 0);

    void SetAlphaBlend(bool alphaBlend);
    void SetGridMode(bool gridMode);
    void SetSamplingMode(HGSResampling const& samplingMode);
    void SetFilter(HFCPtr<HRPFilter> const& pFilter);

protected:

private:
    HRASurface& GetDestSurface() const {return m_destSurface;}

    void                Optimized8BitsWarp(const HRASurface&                pi_srcSurface,
                                           const HGF2DTransfoModel&         pi_rTransfoModel,
                                           HRATransaction*                  pi_pTransaction);

    void                NormalWarp(const HRASurface&                        pi_srcSurface,
                                   const HGF2DTransfoModel&                 pi_rTransfoModel,
                                   HRATransaction*                          pi_pTransaction);

    HRASurface*         ApplyFilter(const HRASurface&                       pi_srcSurface,
                                    const HFCPtr<HRPFilter>&                pi_rpFilter) const;

    Byte*              CreateWorkingBuffer(const HRPPixelType&             pi_rPixelType,
                                            uint32_t                        pi_Width,
                                            uint32_t                        pi_Height) const;

    // Members
    HRASurface&    m_destSurface;

    bool           m_ComposeRequired;
    bool           m_ApplyGrid;
    HGSResampling  m_samplingMode;
    HFCPtr<HRPFilter>   m_pFilter;

    // disabled methods
    HRAWarper();
    HRAWarper(const HRAWarper& pi_rObj);
    HRAWarper&      operator=(const HRAWarper& pi_rObj);
    };
END_IMAGEPP_NAMESPACE
