//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRACopyFromOptions.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRACopyFromOptions
//-----------------------------------------------------------------------------
#pragma once

#include "HGFPreDrawOptions.h"
#include "HGF2DTransfoModel.h"
#include "HGF2DExtent.h"
#include "HVEShape.h"
#include "HRPFilter.h"
#include "HGSTypes.h"

class HNOVTABLEINIT HRACopyFromOptions
    {
public:

    // Primary methods

    _HDLLg                             HRACopyFromOptions();

    _HDLLg                             HRACopyFromOptions(
        const HRACopyFromOptions& pi_rOptions);

    _HDLLg                         explicit HRACopyFromOptions(bool pi_AlphaBlend);

    _HDLLg                         explicit HRACopyFromOptions(const HVEShape* pi_pDestShape, bool pi_AlphaBlend = false);

    _HDLLg                             ~HRACopyFromOptions();


    // Operators

    _HDLLg HRACopyFromOptions&  operator=(const HRACopyFromOptions& pi_rObj);


    // Filters

    const HVEShape*             GetDestShape() const;
    _HDLLg  void                        SetDestShape(const HVEShape* pi_pShape);

    bool                       GetGridShapeMode() const;
    void                        SetGridShapeMode(bool pi_GridShapeMode);

    bool                       ApplySourceClipping() const;
    void                        SetSourceClipping(bool pi_Clip);

    bool                       ApplyAlphaBlend() const;
    void                        SetAlphaBlend(bool pi_AlphaBlend);

    const HGSResampling&        GetResamplingMode() const;
    void                        SetResamplingMode(const HGSResampling& pi_rResampling);

    void                        SetMosaicSupersampling(bool pi_Quality);
    bool                       ApplyMosaicSupersampling() const;

    uint8_t                    MaxResolutionStretchingFactor() const;
    void                        SetMaxResolutionStretchingFactor(uint8_t pi_Factor);

    void                        SetOverviewMode(bool pi_mode);
    bool                       GetOverviewMode()const;

    void                        SetPreDrawOptions(HGFPreDrawOptions* pi_prPreDrawOptions);
    HGFPreDrawOptions*            GetPreDrawOptions() const;

    HFCPtr<HRPPixelType>        GetDestReplacingPixelType() const;
    void                        SetDestReplacingPixelType(const HFCPtr<HRPPixelType>& pi_rpPixelType);

protected:

private:

    const HVEShape*             m_pDestShape;
    bool                       m_GridShapeMode;
    bool                       m_ApplySourceClipping;
    bool                       m_AlphaBlend;
    HGSResampling               m_Resampling;
    uint8_t                    m_MaxResolutionStretchingFactor;
    bool                       m_ApplyMosaicSupersampling;
    bool                       m_OverviewMode;
    HFCPtr<HGFPreDrawOptions>   m_pPreDrawOptions;
    HFCPtr<HRPPixelType>        m_pDestReplacingPixelType;
    };

#include "HRACopyFromOptions.hpp"

