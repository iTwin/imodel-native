//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFMessages.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGF message classes
//-----------------------------------------------------------------------------
// Message classes for HMG mechanism used in HGF.
//-----------------------------------------------------------------------------

#pragma once

#include "HMGMessage.h"

BEGIN_IMAGEPP_NAMESPACE
// Forward declarations
class HMGMessageSender;

///////////////////////////
// HRFProgressImageChangedMsg
///////////////////////////

class HRFProgressImageChangedMsg : public HMGAsynchronousMessage
    {
    HDECLARE_CLASS_ID(HRFProgressImageChangedMsgId, HMGAsynchronousMessage)

public:
    HRFProgressImageChangedMsg();
    HRFProgressImageChangedMsg(const HRFProgressImageChangedMsg& pi_rObj);
    IMAGEPP_EXPORT HRFProgressImageChangedMsg(uint32_t pi_Page,
                               unsigned short pi_SubResolution,
                               uint64_t pi_XPos,
                               uint64_t pi_YPos,
                               bool    pi_Ended = false);
    IMAGEPP_EXPORT virtual ~HRFProgressImageChangedMsg();

    uint32_t GetPage() const;
    unsigned short GetSubResolution () const;
    uint64_t GetPosX () const;
    uint64_t GetPosY () const;
    bool   IsEnded() const;

    virtual HMGMessage* Clone() const override;

private:

    // Message Data
    uint32_t    m_Page;
    unsigned short m_SubResolution;
    uint64_t   m_XPos;
    uint64_t   m_YPos;
    bool       m_Ended;
    };


///////////////////////////
// HRFBlockNotificationMsg
///////////////////////////

class HRFBlockNotificationMsg : public HMGAsynchronousMessage
    {
    HDECLARE_CLASS_ID(HRFBlockNotificationMsgId, HMGAsynchronousMessage)

public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HRFBlockNotificationMsg();
    HRFBlockNotificationMsg(const HRFBlockNotificationMsg& pi_rObj);
    HRFBlockNotificationMsg(uint32_t pi_Page,
                            uint64_t pi_BlockID);
    virtual ~HRFBlockNotificationMsg();


    //--------------------------------------
    // methods
    //--------------------------------------

    // return the page id
    uint32_t        GetPage() const;

    // Returns the ID of the block
    uint64_t       GetBlockID () const;


    //--------------------------------------
    // Overriden from HPMPersistentObject
    //--------------------------------------

    virtual HMGMessage* Clone() const override;


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    uint32_t        m_Page;
    uint64_t       m_BlockID;
    };
END_IMAGEPP_NAMESPACE


#include "HRFMessages.hpp"

