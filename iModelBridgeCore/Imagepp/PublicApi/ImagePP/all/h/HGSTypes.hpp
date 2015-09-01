//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSTypes.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// class HGSResampling
//-----------------------------------------------------------------------------
/**----------------------------------------------------------------------------
 Constructor that creates a new HGSResampling object.
-----------------------------------------------------------------------------*/
inline HGSResampling::HGSResampling(ResamplingMethod pi_ResamplingMethod)
    {
    m_ResamplingMethod = pi_ResamplingMethod;
    }

/**----------------------------------------------------------------------------
 Constructor that creates a new HGSResampling object.
-----------------------------------------------------------------------------*/
inline HGSResampling::HGSResampling(const HGSResampling& pi_rObj)
    {
    m_ResamplingMethod = pi_rObj.m_ResamplingMethod;
    }

/**----------------------------------------------------------------------------
 Destructor for the class HGSResampling.
-----------------------------------------------------------------------------*/
inline HGSResampling::~HGSResampling()
    {
    }

/**----------------------------------------------------------------------------
 Equality operator.
-----------------------------------------------------------------------------*/
inline HGSResampling& HGSResampling::operator=(const HGSResampling& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_ResamplingMethod = pi_rObj.m_ResamplingMethod;
        }
    return *this;
    }

/**----------------------------------------------------------------------------
 Comparaison operator.

 @see bool HGSResampling::operator!=(const HGSResampling&)
-----------------------------------------------------------------------------*/
inline bool HGSResampling::operator==(const HGSResampling& pi_rObj) const
    {
    return m_ResamplingMethod == pi_rObj.m_ResamplingMethod;
    }

/**----------------------------------------------------------------------------
 Comparaison operator.

 @see bool HGSResampling::operator==(const HGSResampling&)
-----------------------------------------------------------------------------*/
inline bool HGSResampling::operator!=(const HGSResampling& pi_rObj) const
    {
    return m_ResamplingMethod != pi_rObj.m_ResamplingMethod;
    }

/**----------------------------------------------------------------------------
 Supports.
-----------------------------------------------------------------------------*/
inline bool HGSResampling::Supports(const HGSResampling& pi_rObj) const
    {
    return m_ResamplingMethod == pi_rObj.m_ResamplingMethod;
    }

/**----------------------------------------------------------------------------
 GetResamplingMethod.
-----------------------------------------------------------------------------*/
inline HGSResampling::ResamplingMethod HGSResampling::GetResamplingMethod() const
    {
    return m_ResamplingMethod;
    }
END_IMAGEPP_NAMESPACE