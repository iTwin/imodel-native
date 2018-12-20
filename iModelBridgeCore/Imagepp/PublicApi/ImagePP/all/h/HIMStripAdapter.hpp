//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIMStripAdapter.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    Copy the object's members
    -----------------------------------------------------------------------------
*/
inline void HIMStripAdapter::SetQualityFactor(double pi_Factor)
    {
    HASSERT(pi_Factor > 0.0 && pi_Factor <= 1.0);

    m_QualityFactor = pi_Factor;
    }


/** -----------------------------------------------------------------------------
    GetQualityFactor
    -----------------------------------------------------------------------------
*/
inline double HIMStripAdapter::GetQualityFactor() const
    {
    return m_QualityFactor;
    }


/** -----------------------------------------------------------------------------
    GetMaxSizeInBytes
    -----------------------------------------------------------------------------
*/
inline size_t HIMStripAdapter::GetMaxSizeInBytes() const
    {
    return m_MaxSizeInBytes;
    }


/** -----------------------------------------------------------------------------
    SetMaxSizeInBytes
    -----------------------------------------------------------------------------
*/
inline void HIMStripAdapter::SetMaxSizeInBytes(size_t pi_MaxSize)
    {
    m_MaxSizeInBytes = pi_MaxSize;
    }


/** -----------------------------------------------------------------------------
    StripsWillBeClipped
    -----------------------------------------------------------------------------
*/
inline bool HIMStripAdapter::StripsWillBeClipped() const
    {
    return m_ApplyClipping;
    }


/** -----------------------------------------------------------------------------
    ClipStripsBasedOnSource
    -----------------------------------------------------------------------------
*/
inline void HIMStripAdapter::ClipStripsBasedOnSource(bool pi_Clip)
    {
    m_ApplyClipping = pi_Clip;
    }
END_IMAGEPP_NAMESPACE