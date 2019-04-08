//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMGMessageSender.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HMGMessageSender
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HMGNotifieeInfo::HMGNotifieeInfo(HMGThreadID pi_ThreadID)
    {
    m_ThreadID = pi_ThreadID;
    m_RefCount = 1;
    }


//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
inline HMGNotifieeInfo::HMGNotifieeInfo(const HMGNotifieeInfo& pi_rObj)
    {
    m_ThreadID = pi_rObj.m_ThreadID;
    m_RefCount = pi_rObj.m_RefCount;
    }


//-----------------------------------------------------------------------------
// Assignment
//-----------------------------------------------------------------------------
inline HMGNotifieeInfo& HMGNotifieeInfo::operator=(const HMGNotifieeInfo& pi_rObj)
    {
    m_ThreadID = pi_rObj.m_ThreadID;
    m_RefCount = pi_rObj.m_RefCount;

    return *this;
    }


//-----------------------------------------------------------------------------
// RefCount ++
//-----------------------------------------------------------------------------
inline uint32_t HMGNotifieeInfo::IncrementRef()
    {
    return ++m_RefCount;
    }


//-----------------------------------------------------------------------------
// RefCount --
//-----------------------------------------------------------------------------
inline uint32_t HMGNotifieeInfo::DecrementRef()
    {
    return --m_RefCount;
    }


//-----------------------------------------------------------------------------
// Retrieve the thread ID of the notifiee
//-----------------------------------------------------------------------------
inline HMGThreadID HMGNotifieeInfo::GetThreadID() const
    {
    return m_ThreadID;
    }
END_IMAGEPP_NAMESPACE