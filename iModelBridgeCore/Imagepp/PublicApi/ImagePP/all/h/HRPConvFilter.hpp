//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPConvFilter.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPConvFilter
//-----------------------------------------------------------------------------
// Inline methods
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// protected
// GetDivisionFactor
//-----------------------------------------------------------------------------
inline int32_t HRPConvFilter::GetDivisionFactor() const
    {
    return(m_DivisionFactor);
    }

//-----------------------------------------------------------------------------
// protected
// GetWeightMatrix
//-----------------------------------------------------------------------------
inline const int32_t* HRPConvFilter::GetWeightMatrix() const
    {
    return(m_pWeightMatrix);
    }

//-----------------------------------------------------------------------------
// public
// IsAConvolutionFilter
//-----------------------------------------------------------------------------
inline bool HRPConvFilter::IsAConvolutionFilter() const
    {
    return true;
    }

//-----------------------------------------------------------------------------
// public
// IsAConvolutionFilter
//-----------------------------------------------------------------------------
inline void HRPConvFilter::SetNeighbourhood(const HRPPixelNeighbourhood& pi_rNeighbourhood)
    {
    HPRECONDITION(!IsAConvolutionFilter());

    HRPTypedFilter::SetNeighbourhood(pi_rNeighbourhood);
    }

//-----------------------------------------------------------------------------
// NOT DOCUMENTED, NOT SUPPORTED YET
// protected
// IsHorizontallySymetric
//-----------------------------------------------------------------------------
inline bool HRPConvFilter::IsHorizontallySymetric() const
    {
    const HRPPixelNeighbourhood& rNeighbourhood = GetNeighbourhood();

    int32_t XOrigin = rNeighbourhood.GetXOrigin();
    int32_t Width = rNeighbourhood.GetWidth();

    if(XOrigin == (Width - 1 - XOrigin))
        {
        int32_t YOrigin = rNeighbourhood.GetYOrigin();

        for(uint32_t X = -1 * XOrigin; X < 0; X++)
            for(uint32_t Y = -1 * YOrigin; Y < 0; Y++)
                if(m_pWeightMatrix[(Y + YOrigin) * Width + (X + XOrigin)] !=
                   m_pWeightMatrix[(-1 * Y + YOrigin) * Width + (X + XOrigin)])
                    return false;


        return true;
        }
    else
        return false;
    }

//-----------------------------------------------------------------------------
// NOT DOCUMENTED, NOT SUPPORTED YET
// protected
// IsPerfectlySymetric
//-----------------------------------------------------------------------------
inline bool HRPConvFilter::IsPerfectlySymetric() const
    {
    return((IsHorizontallySymetric() && IsVerticallySymetric()));
    }

//-----------------------------------------------------------------------------
// NOT DOCUMENTED, NOT SUPPORTED YET
// protected
// IsVerticallySymetric
//-----------------------------------------------------------------------------
inline bool HRPConvFilter::IsVerticallySymetric() const
    {
    const HRPPixelNeighbourhood& rNeighbourhood = GetNeighbourhood();

    int32_t YOrigin = rNeighbourhood.GetYOrigin();

    if(YOrigin == (rNeighbourhood.GetHeight() - 1 - YOrigin))
        {
        int32_t XOrigin = rNeighbourhood.GetXOrigin();
        int32_t Width = rNeighbourhood.GetWidth();

        for(uint32_t Y = -1 * YOrigin; Y < 0; Y++)
            for(uint32_t X = -1 * XOrigin; X < 0; X++)
                if(m_pWeightMatrix[(Y + YOrigin) * Width + (X + XOrigin)] !=
                   m_pWeightMatrix[(Y + YOrigin) * Width + (-1 * X + XOrigin)])
                    return false;


        return true;
        }
    else
        return false;
    }
END_IMAGEPP_NAMESPACE
