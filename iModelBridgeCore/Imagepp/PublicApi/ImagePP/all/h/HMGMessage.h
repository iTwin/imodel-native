//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMGMessage.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HMGMessage
//-----------------------------------------------------------------------------
// This class describes message used in the HMG messaging mechanism
//-----------------------------------------------------------------------------

#pragma once

BEGIN_IMAGEPP_NAMESPACE

class HMGMessageSender;


///////////////////////////////////////////////
// Declaration of class HMGMessage
///////////////////////////////////////////////

class HMGMessage : public HFCShareableObject<HMGMessage>
    {
    HDECLARE_BASECLASS_ID(HMGMessageId_Base)

public:

    // Primary methods
    HMGMessage(bool pi_Synchronous = false);
    
    virtual ~HMGMessage();

    // Added
    bool           IsSynchronous() const;

    IMAGEPP_EXPORT HMGMessageSender* GetSender() const;

    IMAGEPP_EXPORT void            SetSender(HMGMessageSender* pi_pSender);

    virtual HMGMessage* Clone() const = 0;

protected:
    HMGMessage& operator=(const HMGMessage& pi_rObj);
    HMGMessage(const HMGMessage& pi_rObj);
private:
    // Keep note of synchronization state.
    bool               m_IsSynchronous;

    HMGMessageSender*   m_pSender;
    };


///////////////////////////////////////////////
// Declaration of class HMGAsynchronousMessage
///////////////////////////////////////////////

class HMGAsynchronousMessage : public HMGMessage
    {
    HDECLARE_CLASS_ID(HMGAsynchronousMessageId, HMGMessage)

public:

    // Primary methods
    IMAGEPP_EXPORT HMGAsynchronousMessage();
    IMAGEPP_EXPORT virtual ~HMGAsynchronousMessage();
protected:
    IMAGEPP_EXPORT HMGAsynchronousMessage(const HMGAsynchronousMessage& pi_rObj);
    HMGAsynchronousMessage& operator=(const HMGAsynchronousMessage& pi_rObj);
    };


///////////////////////////////////////////////
// Declaration of class HMGSynchronousMessage
///////////////////////////////////////////////

class HMGSynchronousMessage : public HMGMessage
    {
    HDECLARE_CLASS_ID(HMGSynchronousMessageId, HMGMessage)

public:

    // Primary methods
    IMAGEPP_EXPORT HMGSynchronousMessage();
    IMAGEPP_EXPORT virtual ~HMGSynchronousMessage();
protected:
    IMAGEPP_EXPORT HMGSynchronousMessage(const HMGSynchronousMessage& pi_rObj);
    HMGSynchronousMessage& operator=(const HMGSynchronousMessage& pi_rObj);
    };

END_IMAGEPP_NAMESPACE