//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRACopyFromOptions.hpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// inline methods for class : HRACopyFromOptions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// GetAlphaBlend
//-----------------------------------------------------------------------------
inline bool HRACopyFromOptions::ApplyAlphaBlend() const
    {
    return m_AlphaBlend;
    }

//-----------------------------------------------------------------------------
// public
// SetAlphaBlend
//-----------------------------------------------------------------------------
inline void HRACopyFromOptions::SetAlphaBlend(bool pi_AlphaBlend)
    {
    m_AlphaBlend = pi_AlphaBlend;
    }

//-----------------------------------------------------------------------------
// public
// GetBorderGenerator
//-----------------------------------------------------------------------------
inline const HVEShape* HRACopyFromOptions::GetDestShape() const
    {
    return m_pDestShape;
    }


//-----------------------------------------------------------------------------
// public
// SetAveraging
//-----------------------------------------------------------------------------
inline void HRACopyFromOptions::SetResamplingMode(const HGSResampling& pi_rResampling)
    {
    m_Resampling = pi_rResampling;
    }

//-----------------------------------------------------------------------------
// public
// ApplyAveraging
//-----------------------------------------------------------------------------
inline const HGSResampling& HRACopyFromOptions::GetResamplingMode() const
    {
    return m_Resampling;
    }


//-----------------------------------------------------------------------------
// public
// MaxResolutionStretchingFactor
//-----------------------------------------------------------------------------
inline uint8_t HRACopyFromOptions::MaxResolutionStretchingFactor() const
    {
    return m_MaxResolutionStretchingFactor;
    }


//-----------------------------------------------------------------------------
// public
// SetMaxResolutionStretchingFactor
//-----------------------------------------------------------------------------
inline void HRACopyFromOptions::SetMaxResolutionStretchingFactor(uint8_t pi_Factor)
    {
    // Represents a percentage.
    HASSERT(pi_Factor <= 100);

    m_MaxResolutionStretchingFactor = pi_Factor;
    }


//-----------------------------------------------------------------------------
// public
// ApplyMosaicSupersampling
//-----------------------------------------------------------------------------
inline bool HRACopyFromOptions::ApplyMosaicSupersampling() const
    {
    return m_ApplyMosaicSupersampling;
    }


//-----------------------------------------------------------------------------
// public
// SetMosaicSupersampling
//-----------------------------------------------------------------------------
inline void HRACopyFromOptions::SetMosaicSupersampling(bool pi_Quality)
    {
    m_ApplyMosaicSupersampling = pi_Quality;
    }


//-----------------------------------------------------------------------------
// public
// ApplySourceClipping
//-----------------------------------------------------------------------------
inline bool HRACopyFromOptions::ApplySourceClipping() const
    {
    return m_ApplySourceClipping;
    }


//-----------------------------------------------------------------------------
// public
// SetSourceClipping
//-----------------------------------------------------------------------------
inline void HRACopyFromOptions::SetSourceClipping(bool pi_Clip)
    {
    m_ApplySourceClipping = pi_Clip;
    }


//-----------------------------------------------------------------------------
// public
// GetGridShapeMode
//-----------------------------------------------------------------------------
inline bool HRACopyFromOptions::GetGridShapeMode() const
    {
    return m_GridShapeMode;
    }


//-----------------------------------------------------------------------------
// public
// SetGridShapeMode
//-----------------------------------------------------------------------------
inline void HRACopyFromOptions::SetGridShapeMode(bool pi_GridShapeMode)
    {
    m_GridShapeMode = pi_GridShapeMode;
    }

//-----------------------------------------------------------------------------
// public
// SetOverviewMode
//-----------------------------------------------------------------------------
inline void HRACopyFromOptions::SetOverviewMode(bool pi_mode)
    {
    m_OverviewMode = pi_mode;
    }

//-----------------------------------------------------------------------------
// public
// GetOverviewMode
//-----------------------------------------------------------------------------
inline bool HRACopyFromOptions::GetOverviewMode() const
    {
    return m_OverviewMode;
    }

//-----------------------------------------------------------------------------
// public
// SetPreDrawOptions
//-----------------------------------------------------------------------------
inline void    HRACopyFromOptions::SetPreDrawOptions(HGFPreDrawOptions* pi_prPreDrawOptions)
    {
    m_pPreDrawOptions = pi_prPreDrawOptions;
    }

//-----------------------------------------------------------------------------
// public
// GetPreDrawOptions
//-----------------------------------------------------------------------------
inline HGFPreDrawOptions* HRACopyFromOptions::GetPreDrawOptions() const
    {
    return m_pPreDrawOptions;
    }

//-----------------------------------------------------------------------------
// public
// GetReplacingPixelType
//-----------------------------------------------------------------------------
inline HFCPtr<HRPPixelType> HRACopyFromOptions::GetDestReplacingPixelType() const
    {
    return m_pDestReplacingPixelType;
    }

//-----------------------------------------------------------------------------
// public
// SetReplacingPixelType
//-----------------------------------------------------------------------------
inline void HRACopyFromOptions::SetDestReplacingPixelType(const HFCPtr<HRPPixelType>& pi_rpPixelType)
    {
    m_pDestReplacingPixelType = pi_rpPixelType;
    }