//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFDrawOptions.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// inline methods for class : HGFDrawOptions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// GetAlphaBlend
//-----------------------------------------------------------------------------
inline bool HGFDrawOptions::ApplyAlphaBlend() const
    {
    return m_ApplyAlphaBlend;
    }

//-----------------------------------------------------------------------------
// public
// SetAlphaBlend
//-----------------------------------------------------------------------------
inline void HGFDrawOptions::SetAlphaBlend(bool pi_ApplyAlphaBlend)
    {
    m_ApplyAlphaBlend = pi_ApplyAlphaBlend;
    }


//-----------------------------------------------------------------------------
// public
// SetAveraging
//-----------------------------------------------------------------------------
inline void HGFDrawOptions::SetResamplingMode(const HGSResampling& pi_rResampling)
    {
    m_Resampling = pi_rResampling;
    }

//-----------------------------------------------------------------------------
// public
// ApplyAveraging
//-----------------------------------------------------------------------------
inline const HGSResampling& HGFDrawOptions::GetResamplingMode() const
    {
    return m_Resampling;
    }

//-----------------------------------------------------------------------------
// AbortDraw
//-----------------------------------------------------------------------------
inline void HGFDrawOptions::AbortDraw()
    {
    m_AbortRequest = true;
    }

//-----------------------------------------------------------------------------
// ResetAbortRequest
//-----------------------------------------------------------------------------
inline void HGFDrawOptions::ResetAbortRequest()
    {
    m_AbortRequest = false;
    }

//-----------------------------------------------------------------------------
// ShouldAbort
//-----------------------------------------------------------------------------
inline bool HGFDrawOptions::ShouldAbort() const
    {
    return m_AbortRequest;
    }

//-----------------------------------------------------------------------------
// GetAttributes
//-----------------------------------------------------------------------------
inline const HPMAttributeSet& HGFDrawOptions::GetAttributes() const
    {
    return m_Attributes;
    }

//-----------------------------------------------------------------------------
// SetAttributes
//-----------------------------------------------------------------------------
inline void HGFDrawOptions::SetAttributes(const HPMAttributeSet& pi_rAttributes)
    {
    m_Attributes = pi_rAttributes;
    }


//-----------------------------------------------------------------------------
// TemporaryRenderMode
//-----------------------------------------------------------------------------
inline void HGFDrawOptions::SetTemporaryRenderMode(bool pi_TemporaryMode)
    {
    m_InTemporaryRenderMode = pi_TemporaryMode;
    }


//-----------------------------------------------------------------------------
// InTemporaryRenderMode
//-----------------------------------------------------------------------------
inline bool HGFDrawOptions::TemporaryRenderMode() const
    {
    return m_InTemporaryRenderMode;
    }

//-----------------------------------------------------------------------------
// public
// SetOverviewMode
//-----------------------------------------------------------------------------
inline void HGFDrawOptions::SetOverviewMode(bool pi_mode)
    {
    m_OverviewMode = pi_mode;
    }

//-----------------------------------------------------------------------------
// public
// GetOverviewMode
//-----------------------------------------------------------------------------
inline bool HGFDrawOptions::GetOverviewMode() const
    {
    return m_OverviewMode;
    }

//-----------------------------------------------------------------------------
// public
// SetPreDrawOptions
//-----------------------------------------------------------------------------
inline void HGFDrawOptions::SetPreDrawOptions(HGFPreDrawOptions* pi_prPreDrawOptions)
    {
    m_pPreDrawOptions = pi_prPreDrawOptions;
    }

//-----------------------------------------------------------------------------
// public
// GetPreDrawOptions
//-----------------------------------------------------------------------------
inline HGFPreDrawOptions* HGFDrawOptions::GetPreDrawOptions()
    {
    return m_pPreDrawOptions;
    }

//-----------------------------------------------------------------------------
// public
// GetPreDrawOptions
//-----------------------------------------------------------------------------
inline const HGFPreDrawOptions* HGFDrawOptions::GetPreDrawOptions() const
    {
    return m_pPreDrawOptions;
    }