//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRADrawOptions.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// inline methods for class : HRADrawOptions
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// public
// GetShape
//-----------------------------------------------------------------------------
inline HFCPtr<HVEShape> HRADrawOptions::GetShape() const
    {
    return m_pShape;
    }

//-----------------------------------------------------------------------------
// public
// SetShape
//-----------------------------------------------------------------------------
inline void HRADrawOptions::SetShape(const HFCPtr<HVEShape>& pi_rpShape)
    {
    m_pShape = pi_rpShape;
    }

//-----------------------------------------------------------------------------
// public
// SetGridShape
//-----------------------------------------------------------------------------
inline void HRADrawOptions::SetGridShape(bool pi_GridShape)
    {
    m_ApplyGridShape = pi_GridShape;
    }

//-----------------------------------------------------------------------------
// public
// ApplyGridShape
//-----------------------------------------------------------------------------
inline bool HRADrawOptions::ApplyGridShape() const
    {
    return m_ApplyGridShape;
    }

//-----------------------------------------------------------------------------
// public
// GetReplacingCoordSys
//-----------------------------------------------------------------------------
inline HFCPtr<HGF2DCoordSys> HRADrawOptions::GetReplacingCoordSys() const
    {
    return m_pReplacingCoordSys;
    }

//-----------------------------------------------------------------------------
// public
// SetReplacingCoordSys
//-----------------------------------------------------------------------------
inline void HRADrawOptions::SetReplacingCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpReplacingCoordSys)
    {
    m_pReplacingCoordSys = pi_rpReplacingCoordSys;
    }

//-----------------------------------------------------------------------------
// public
// GetReplacingPixelType
//-----------------------------------------------------------------------------
inline HFCPtr<HRPPixelType> HRADrawOptions::GetReplacingPixelType() const
    {
    return m_pReplacingPixelType;
    }

//-----------------------------------------------------------------------------
// public
// SetReplacingPixelType
//-----------------------------------------------------------------------------
inline void HRADrawOptions::SetReplacingPixelType(const HFCPtr<HRPPixelType>& pi_rpReplacingPixelType)
    {
    m_pReplacingPixelType = pi_rpReplacingPixelType;
    }

//-----------------------------------------------------------------------------
// public
// GetAlphaBlend
//-----------------------------------------------------------------------------
inline bool HRADrawOptions::ApplyAlphaBlend() const
    {
    return m_ApplyAlphaBlend;
    }

//-----------------------------------------------------------------------------
// public
// SetAlphaBlend
//-----------------------------------------------------------------------------
inline void HRADrawOptions::SetAlphaBlend(bool pi_ApplyAlphaBlend)
    {
    m_ApplyAlphaBlend = pi_ApplyAlphaBlend;
    }


//-----------------------------------------------------------------------------
// public
// SetAveraging
//-----------------------------------------------------------------------------
inline void HRADrawOptions::SetResamplingMode(const HGSResampling& pi_rResampling)
    {
    m_Resampling = pi_rResampling;
    }

//-----------------------------------------------------------------------------
// public
// ApplyAveraging
//-----------------------------------------------------------------------------
inline const HGSResampling& HRADrawOptions::GetResamplingMode() const
    {
    return m_Resampling;
    }

//-----------------------------------------------------------------------------
// GetAttributes
//-----------------------------------------------------------------------------
inline const HPMAttributeSet& HRADrawOptions::GetAttributes() const
    {
    return m_Attributes;
    }

//-----------------------------------------------------------------------------
// SetAttributes
//-----------------------------------------------------------------------------
inline void HRADrawOptions::SetAttributes(const HPMAttributeSet& pi_rAttributes)
    {
    m_Attributes = pi_rAttributes;
    }

//-----------------------------------------------------------------------------
// public
// SetOverviewMode
//-----------------------------------------------------------------------------
inline void HRADrawOptions::SetOverviewMode(bool pi_mode)
    {
    m_OverviewMode = pi_mode;
    }

//-----------------------------------------------------------------------------
// public
// GetOverviewMode
//-----------------------------------------------------------------------------
inline bool HRADrawOptions::GetOverviewMode() const
    {
    return m_OverviewMode;
    }

END_IMAGEPP_NAMESPACE