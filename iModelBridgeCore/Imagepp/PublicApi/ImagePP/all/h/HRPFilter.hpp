//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPFilter.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPFilter
//-----------------------------------------------------------------------------
// Inline methods for HRPFilter class.
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// public
// GetInputPixelType
//-----------------------------------------------------------------------------
inline const HFCPtr<HRPPixelType>& HRPFilter::GetInputPixelType() const
    {
    return(m_pInputPixelType);
    }

//-----------------------------------------------------------------------------
// public
// GetNeighbourhood
//-----------------------------------------------------------------------------
inline const HRPPixelNeighbourhood& HRPFilter::GetNeighbourhood() const
    {
    return(m_Neighbourhood);
    }

//-----------------------------------------------------------------------------
// public
// GetOutputPixelType
//-----------------------------------------------------------------------------
inline const HFCPtr<HRPPixelType>& HRPFilter::GetOutputPixelType() const
    {
    return(m_pOutputPixelType);
    }

//-----------------------------------------------------------------------------
// public
// SetInputPixelType.
//-----------------------------------------------------------------------------
inline void HRPFilter::SetInputPixelType(const HFCPtr<HRPPixelType>& pi_pInputPixelType)
    {
    m_pInputPixelType = pi_pInputPixelType;
    }

//-----------------------------------------------------------------------------
// protected
// SetNeighbourhood
//-----------------------------------------------------------------------------
inline void HRPFilter::SetNeighbourhood(const HRPPixelNeighbourhood& pi_rNeighbourhood)
    {
    m_Neighbourhood = pi_rNeighbourhood;
    }

//-----------------------------------------------------------------------------
// public
// SetOutputPixelType.
//-----------------------------------------------------------------------------
inline void HRPFilter::SetOutputPixelType(const HFCPtr<HRPPixelType>& pi_pOutputPixelType)
    {
    m_pOutputPixelType = pi_pOutputPixelType;
    }

END_IMAGEPP_NAMESPACE
