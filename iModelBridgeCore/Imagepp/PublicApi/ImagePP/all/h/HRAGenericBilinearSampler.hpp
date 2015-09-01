//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAGenericBilinearSampler.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/** ---------------------------------------------------------------------------
    Constructor
    ---------------------------------------------------------------------------
*/
template<class T>
inline HRAGenericBilinearSampler<T>::Sample::Sample(double pi_PositionX,
                                                    double pi_PositionY)
    {
    m_PositionX = pi_PositionX;
    m_PositionY = pi_PositionY;

    m_XIsPastCenter = (m_PositionX - (double)((int32_t)m_PositionX) >= 0.5);
    m_YIsPastCenter = (m_PositionY - (double)((int32_t)m_PositionY) >= 0.5);
    }


/** ---------------------------------------------------------------------------
    Destructor
    ---------------------------------------------------------------------------
*/
template<class T>
inline HRAGenericBilinearSampler<T>::Sample::~Sample()
    {
    }


/** ---------------------------------------------------------------------------
    SetPosition
    ---------------------------------------------------------------------------
*/
template<class T>
inline void HRAGenericBilinearSampler<T>::Sample::SetPosition(double pi_PositionX,
                                                              double pi_PositionY)
    {
    m_PositionX = pi_PositionX;
    m_PositionY = pi_PositionY;

    m_XIsPastCenter = (m_PositionX - (double)((int32_t)m_PositionX) >= 0.5);
    m_YIsPastCenter = (m_PositionY - (double)((int32_t)m_PositionY) >= 0.5);
    }


/** ---------------------------------------------------------------------------
    TranslateX
    ---------------------------------------------------------------------------
*/
template<class T>
inline void HRAGenericBilinearSampler<T>::Sample::TranslateX(double pi_DeltaX)
    {
    m_PositionX += pi_DeltaX;

    m_XIsPastCenter = (m_PositionX - (double)((int32_t)m_PositionX) >= 0.5);
    }


/** ---------------------------------------------------------------------------
    GetFirstLine
    ---------------------------------------------------------------------------
*/
template<class T>
inline uint32_t HRAGenericBilinearSampler<T>::Sample::GetFirstLine() const
    {
    if (m_YIsPastCenter)
        return (uint32_t)m_PositionY;
    else
        return (uint32_t)MAX(m_PositionY - 1.0, 0.0);
    }


/** ---------------------------------------------------------------------------
    GetSecondLine
    ---------------------------------------------------------------------------
*/
template<class T>
inline uint32_t HRAGenericBilinearSampler<T>::Sample::GetSecondLine() const
    {
    if (m_YIsPastCenter)
        return (uint32_t)(m_PositionY + 1.0);
    else
        return (uint32_t)m_PositionY;
    }


/** ---------------------------------------------------------------------------
    GetFirstColumn
    ---------------------------------------------------------------------------
*/
template<class T>
inline uint32_t HRAGenericBilinearSampler<T>::Sample::GetFirstColumn() const
    {
    if (m_XIsPastCenter)
        return (uint32_t)m_PositionX;
    else
        return (uint32_t)MAX(m_PositionX - 1.0, 0.0);
    }


/** ---------------------------------------------------------------------------
    GetSecondColumn
    ---------------------------------------------------------------------------
*/
template<class T>
inline uint32_t HRAGenericBilinearSampler<T>::Sample::GetSecondColumn() const
    {
    if (m_XIsPastCenter)
        return (uint32_t)(m_PositionX + 1.0);
    else
        return (uint32_t)m_PositionX;
    }


/** ---------------------------------------------------------------------------
    Get the distance of the sampled coordinate from the center of the first
    pixel of the 2x2 sample on X.
    ---------------------------------------------------------------------------
*/
template<class T>
inline double HRAGenericBilinearSampler<T>::Sample::GetXDeltaOfFirstPixel() const
    {
    if (m_XIsPastCenter)
        {
        // Past center of current pixel. Compute distance
        // from center of current pixel (our distance).
        return m_PositionX - ((double)((int32_t)m_PositionX) + 0.5);
        }
    else
        {
        // Before center. Compute distance from center of previous pixel
        // (distance of X - 0.5 from start of pixel)
        return m_PositionX - ((double)((int32_t)m_PositionX) - 0.5);
        }
    }


/** ---------------------------------------------------------------------------
    Get the distance of the sampled coordinate from the center of the first
    pixel of the 2x2 sample on Y.
    ---------------------------------------------------------------------------
*/
template<class T>
inline double HRAGenericBilinearSampler<T>::Sample::GetYDeltaOfFirstPixel() const
    {
    if (m_YIsPastCenter)
        {
        // Past center of current pixel. Compute distance
        // from center of current pixel (our distance).
        return m_PositionY - ((double)((int32_t)m_PositionY) + 0.5);
        }
    else
        {
        // Before center. Compute distance from center of previous pixel
        // (distance of Y - 0.5 from start of pixel)
        return m_PositionY - ((double)((int32_t)m_PositionY) - 0.5);
        }
    }
END_IMAGEPP_NAMESPACE