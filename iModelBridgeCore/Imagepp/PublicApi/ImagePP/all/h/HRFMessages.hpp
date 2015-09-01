//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFMessages.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRF message classes
//-----------------------------------------------------------------------------
// Inline methods for Message classes used in HGF.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HRFProgressImageChangedMsg
//-----------------------------------------------------------------------------
inline HRFProgressImageChangedMsg::HRFProgressImageChangedMsg()
    : HMGAsynchronousMessage()
    {
    m_Page          = 0;
    m_SubResolution = 0;
    m_XPos          = 0;
    m_YPos          = 0;
    m_Ended          = false;
    }


inline HRFProgressImageChangedMsg::HRFProgressImageChangedMsg(const HRFProgressImageChangedMsg& pi_rObj)
    : HMGAsynchronousMessage(pi_rObj)
    {
    m_Page          = pi_rObj.m_Page;
    m_SubResolution = pi_rObj.m_SubResolution;
    m_XPos          = pi_rObj.m_XPos;
    m_YPos          = pi_rObj.m_YPos;
    m_Ended         = pi_rObj.m_Ended;
    }


inline HRFProgressImageChangedMsg::HRFProgressImageChangedMsg(uint32_t  pi_Page,
                                                              unsigned short pi_SubResolution,
                                                              uint64_t pi_XPos,
                                                              uint64_t pi_YPos,
                                                              bool     pi_Ended)
    : HMGAsynchronousMessage()
    {
    m_Page          = pi_Page;
    m_SubResolution = pi_SubResolution;
    m_XPos          = pi_XPos;
    m_YPos          = pi_YPos;
    m_Ended         = pi_Ended;
    }

inline uint32_t HRFProgressImageChangedMsg::GetPage() const
    {
    return m_Page;
    }


inline unsigned short HRFProgressImageChangedMsg::GetSubResolution () const
    {
    return m_SubResolution;
    }

inline uint64_t HRFProgressImageChangedMsg::GetPosX () const
    {
    return m_XPos;
    }


inline uint64_t HRFProgressImageChangedMsg::GetPosY () const
    {
    return m_YPos;
    }


inline bool HRFProgressImageChangedMsg::IsEnded() const
    {
    return m_Ended;
    }



//-----------------------------------------------------------------------------
// HRFBlockNotificationMsg
//-----------------------------------------------------------------------------

inline HRFBlockNotificationMsg::HRFBlockNotificationMsg()
    : HMGAsynchronousMessage()
    {
    m_Page    = 0;
    m_BlockID = 0;
    }


inline HRFBlockNotificationMsg::HRFBlockNotificationMsg(const HRFBlockNotificationMsg& pi_rObj)
    : HMGAsynchronousMessage(pi_rObj)
    {
    m_Page = pi_rObj.m_Page;
    m_BlockID = pi_rObj.m_BlockID;
    }


inline HRFBlockNotificationMsg::HRFBlockNotificationMsg(uint32_t pi_Page,
                                                        uint64_t pi_BlockID)
    {
    m_Page = pi_Page;
    m_BlockID = pi_BlockID;
    }


inline uint32_t HRFBlockNotificationMsg::GetPage() const
    {
    return m_Page;
    }


inline uint64_t HRFBlockNotificationMsg::GetBlockID () const
    {
    return m_BlockID;
    }
