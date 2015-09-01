//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRACopyFromOptions.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// inline methods for class : HRACopyFromOptions
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// public
// GetAlphaBlend
//-----------------------------------------------------------------------------
inline bool HRACopyFromLegacyOptions::ApplyAlphaBlend() const
    {
    return m_AlphaBlend;
    }

//-----------------------------------------------------------------------------
// public
// SetAlphaBlend
//-----------------------------------------------------------------------------
inline void HRACopyFromLegacyOptions::SetAlphaBlend(bool pi_AlphaBlend)
    {
    m_AlphaBlend = pi_AlphaBlend;
    }

//-----------------------------------------------------------------------------
// public
// GetBorderGenerator
//-----------------------------------------------------------------------------
inline const HVEShape* HRACopyFromLegacyOptions::GetDestShape() const
    {
    return m_pDestShape;
    }


//-----------------------------------------------------------------------------
// public
// SetAveraging
//-----------------------------------------------------------------------------
inline void HRACopyFromLegacyOptions::SetResamplingMode(const HGSResampling& pi_rResampling)
    {
    m_Resampling = pi_rResampling;
    }

//-----------------------------------------------------------------------------
// public
// ApplyAveraging
//-----------------------------------------------------------------------------
inline const HGSResampling& HRACopyFromLegacyOptions::GetResamplingMode() const
    {
    return m_Resampling;
    }


//-----------------------------------------------------------------------------
// public
// MaxResolutionStretchingFactor
//-----------------------------------------------------------------------------
inline uint8_t HRACopyFromLegacyOptions::MaxResolutionStretchingFactor() const
    {
    return m_MaxResolutionStretchingFactor;
    }


//-----------------------------------------------------------------------------
// public
// SetMaxResolutionStretchingFactor
//-----------------------------------------------------------------------------
inline void HRACopyFromLegacyOptions::SetMaxResolutionStretchingFactor(uint8_t pi_Factor)
    {
    // Represents a percentage.
    HASSERT(pi_Factor <= 100);

    m_MaxResolutionStretchingFactor = pi_Factor;
    }


//-----------------------------------------------------------------------------
// public
// ApplySourceClipping
//-----------------------------------------------------------------------------
inline bool HRACopyFromLegacyOptions::ApplySourceClipping() const
    {
    return m_ApplySourceClipping;
    }


//-----------------------------------------------------------------------------
// public
// SetSourceClipping
//-----------------------------------------------------------------------------
inline void HRACopyFromLegacyOptions::SetSourceClipping(bool pi_Clip)
    {
    m_ApplySourceClipping = pi_Clip;
    }


//-----------------------------------------------------------------------------
// public
// GetGridShapeMode
//-----------------------------------------------------------------------------
inline bool HRACopyFromLegacyOptions::GetGridShapeMode() const
    {
    return m_GridShapeMode;
    }


//-----------------------------------------------------------------------------
// public
// SetGridShapeMode
//-----------------------------------------------------------------------------
inline void HRACopyFromLegacyOptions::SetGridShapeMode(bool pi_GridShapeMode)
    {
    m_GridShapeMode = pi_GridShapeMode;
    }

//-----------------------------------------------------------------------------
// public
// SetOverviewMode
//-----------------------------------------------------------------------------
inline void HRACopyFromLegacyOptions::SetOverviewMode(bool pi_mode)
    {
    m_OverviewMode = pi_mode;
    }

//-----------------------------------------------------------------------------
// public
// GetOverviewMode
//-----------------------------------------------------------------------------
inline bool HRACopyFromLegacyOptions::GetOverviewMode() const
    {
    return m_OverviewMode;
    }

//-----------------------------------------------------------------------------
// public
// GetReplacingPixelType
//-----------------------------------------------------------------------------
inline HFCPtr<HRPPixelType> HRACopyFromLegacyOptions::GetDestReplacingPixelType() const
    {
    return m_pDestReplacingPixelType;
    }

//-----------------------------------------------------------------------------
// public
// SetReplacingPixelType
//-----------------------------------------------------------------------------
inline void HRACopyFromLegacyOptions::SetDestReplacingPixelType(const HFCPtr<HRPPixelType>& pi_rpPixelType)
    {
    m_pDestReplacingPixelType = pi_rpPixelType;
    }
END_IMAGEPP_NAMESPACE