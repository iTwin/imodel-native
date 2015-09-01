//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAGenericBicubicSampler.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/** ---------------------------------------------------------------------------
    Constructor
    ---------------------------------------------------------------------------
*/
template <class T>
inline HRAGenericBicubicSampler<T>::Sample::Sample(double pi_PositionX,
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
template <class T>
inline HRAGenericBicubicSampler<T>::Sample::Sample()
    {
    // Nothing. Members are invalid!
    }


/** ---------------------------------------------------------------------------
    Destructor
    ---------------------------------------------------------------------------
*/
template <class T>
inline HRAGenericBicubicSampler<T>::Sample::~Sample()
    {
    }


/** ---------------------------------------------------------------------------
    SetPosition
    ---------------------------------------------------------------------------
*/
template <class T>
inline void HRAGenericBicubicSampler<T>::Sample::SetPosition(double pi_PositionX,
                                                             double pi_PositionY)
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
template <class T>
inline void HRAGenericBicubicSampler<T>::Sample::TranslateX(double pi_DeltaX)
    {
    m_PositionX += pi_DeltaX;
    }


/** ---------------------------------------------------------------------------
    GetFirstLine
    ---------------------------------------------------------------------------
*/
template <class T>
inline uint32_t HRAGenericBicubicSampler<T>::Sample::GetLine0() const
    {
    return (uint32_t)MAX(m_PositionY - 2.0, 0.0);
    }


/** ---------------------------------------------------------------------------
    GetSecondLine
    ---------------------------------------------------------------------------
*/
template <class T>
inline uint32_t HRAGenericBicubicSampler<T>::Sample::GetLine1() const
    {
    return (uint32_t)MAX(m_PositionY - 1.0, 0.0);
    }

template <class T>
inline uint32_t HRAGenericBicubicSampler<T>::Sample::GetLine2() const
    {
    return (uint32_t)(m_PositionY);
    }

template <class T>
inline uint32_t HRAGenericBicubicSampler<T>::Sample::GetLine3() const
    {
    return (uint32_t)(m_PositionY + 1.0);
    }

/** ---------------------------------------------------------------------------
    GetFirstColumn
    ---------------------------------------------------------------------------
*/
template <class T>
inline uint32_t HRAGenericBicubicSampler<T>::Sample::GetColumn0() const
    {
    return (uint32_t)MAX(m_PositionX - 2.0, 0.0);
    }


/** ---------------------------------------------------------------------------
    GetSecondColumn
    ---------------------------------------------------------------------------
*/
template <class T>
inline uint32_t HRAGenericBicubicSampler<T>::Sample::GetColumn1() const
    {
    return (uint32_t)MAX(m_PositionX - 1.0, 0.0);
    }

template <class T>
inline uint32_t HRAGenericBicubicSampler<T>::Sample::GetColumn2() const
    {
    return (uint32_t)(m_PositionX);
    }

template <class T>
inline uint32_t HRAGenericBicubicSampler<T>::Sample::GetColumn3() const
    {
    return (uint32_t)(m_PositionX + 1.0);
    }

/** ---------------------------------------------------------------------------
    Gives X offset from pixel 1's center (0.0 <= xDelta < 1.0).
    ---------------------------------------------------------------------------
*/
template <class T>
inline double HRAGenericBicubicSampler<T>::Sample::GetXDelta() const
    {
    return m_PositionX - (double)((int32_t)m_PositionX);
    }


/** ---------------------------------------------------------------------------
    Gives Y offset from pixel 1's center (0.0 <= yDelta < 1.0).
    ---------------------------------------------------------------------------
*/
template <class T>
inline double HRAGenericBicubicSampler<T>::Sample::GetYDelta() const
    {
    return m_PositionY - (double)((int32_t)m_PositionY);
    }
END_IMAGEPP_NAMESPACE