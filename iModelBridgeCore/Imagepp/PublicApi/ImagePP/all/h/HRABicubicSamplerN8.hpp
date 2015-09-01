//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRABicubicSamplerN8.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/** ---------------------------------------------------------------------------
    Constructor
    ---------------------------------------------------------------------------
*/
inline HRABicubicSamplerN8::Sample::Sample(double pi_PositionX,
                                           double pi_PositionY)
    {
    // Offset positions by 0.5 so that we're positioned relatively to
    // the middle of the pixel 1 (line 1 and column 1) instead of the
    // beginning boundary of the image.
    m_PositionX = pi_PositionX + 0.5;
    m_PositionY = pi_PositionY + 0.5;
    }


/** ---------------------------------------------------------------------------
    Default constructor.
    SetPosition must be called before any other method.
    ---------------------------------------------------------------------------
*/
inline HRABicubicSamplerN8::Sample::Sample()
    {
    // Nothing. Members are invalid!
    }


/** ---------------------------------------------------------------------------
    Destructor
    ---------------------------------------------------------------------------
*/
inline HRABicubicSamplerN8::Sample::~Sample()
    {
    }


/** ---------------------------------------------------------------------------
    SetPosition
    ---------------------------------------------------------------------------
*/
inline void HRABicubicSamplerN8::Sample::SetPosition(double pi_PositionX, double pi_PositionY)
    {
    // Offset positions by 0.5 so that we're positioned relatively to
    // the middle of the pixel 1 (line 1 and column 1) instead of the
    // beginning boundary of the image.
    m_PositionX = pi_PositionX + 0.5;
    m_PositionY = pi_PositionY + 0.5;
    }


/** ---------------------------------------------------------------------------
    TranslateX
    ---------------------------------------------------------------------------
*/
inline void HRABicubicSamplerN8::Sample::TranslateX(double pi_DeltaX)
    {
    m_PositionX += pi_DeltaX;
    }


/** ---------------------------------------------------------------------------
    GetFirstLine
    ---------------------------------------------------------------------------
*/
inline uint32_t HRABicubicSamplerN8::Sample::GetLine0() const
    {
    return (uint32_t)MAX(m_PositionY - 2.0, 0.0);
    }


/** ---------------------------------------------------------------------------
    GetSecondLine
    ---------------------------------------------------------------------------
*/
inline uint32_t HRABicubicSamplerN8::Sample::GetLine1() const
    {
    return (uint32_t)MAX(m_PositionY - 1.0, 0.0);
    }

inline uint32_t HRABicubicSamplerN8::Sample::GetLine2() const
    {
    return (uint32_t)(m_PositionY);
    }

inline uint32_t HRABicubicSamplerN8::Sample::GetLine3() const
    {
    return (uint32_t)(m_PositionY + 1.0);
    }

/** ---------------------------------------------------------------------------
    GetFirstColumn
    ---------------------------------------------------------------------------
*/
inline uint32_t HRABicubicSamplerN8::Sample::GetColumn0() const
    {
    return (uint32_t)MAX(m_PositionX - 2.0, 0.0);
    }


/** ---------------------------------------------------------------------------
    GetSecondColumn
    ---------------------------------------------------------------------------
*/
inline uint32_t HRABicubicSamplerN8::Sample::GetColumn1() const
    {
    return (uint32_t)MAX(m_PositionX - 1.0, 0.0);
    }

inline uint32_t HRABicubicSamplerN8::Sample::GetColumn2() const
    {
    return (uint32_t)(m_PositionX);
    }

inline uint32_t HRABicubicSamplerN8::Sample::GetColumn3() const
    {
    return (uint32_t)(m_PositionX + 1.0);
    }

/** ---------------------------------------------------------------------------
    Gives X offset from pixel 1's center (0.0 <= xDelta < 1.0).
    ---------------------------------------------------------------------------
*/
inline double HRABicubicSamplerN8::Sample::GetXDelta() const
    {
    return m_PositionX - (double)((int32_t)m_PositionX);
    }


/** ---------------------------------------------------------------------------
    Gives Y offset from pixel 1's center (0.0 <= yDelta < 1.0).
    ---------------------------------------------------------------------------
*/
inline double HRABicubicSamplerN8::Sample::GetYDelta() const
    {
    return m_PositionY - (double)((int32_t)m_PositionY);
    }


END_IMAGEPP_NAMESPACE