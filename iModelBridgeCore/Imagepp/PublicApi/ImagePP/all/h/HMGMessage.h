//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMGMessage.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HMGMessage
//-----------------------------------------------------------------------------
// This class describes message used in the HMG messaging mechanism
//-----------------------------------------------------------------------------

#pragma once

class HMGMessageSender;


///////////////////////////////////////////////
// Declaration of class HMGMessage
///////////////////////////////////////////////

class HMGMessage : public HFCShareableObject<HMGMessage>
    {
    HDECLARE_BASECLASS_ID(1091)

public:

    // Primary methods
    HMGMessage(bool pi_Synchronous = false);
    
    virtual ~HMGMessage();

    // Added
    bool           IsSynchronous() const;

    _HDLLu HMGMessageSender* GetSender() const;

    _HDLLu void            SetSender(HMGMessageSender* pi_pSender);

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
    HDECLARE_CLASS_ID(1093, HMGMessage)

public:

    // Primary methods
    _HDLLu HMGAsynchronousMessage();
    _HDLLu virtual ~HMGAsynchronousMessage();
protected:
    _HDLLu HMGAsynchronousMessage(const HMGAsynchronousMessage& pi_rObj);
    HMGAsynchronousMessage& operator=(const HMGAsynchronousMessage& pi_rObj);
    };


///////////////////////////////////////////////
// Declaration of class HMGSynchronousMessage
///////////////////////////////////////////////

class HMGSynchronousMessage : public HMGMessage
    {
    HDECLARE_CLASS_ID(1092, HMGMessage)

public:

    // Primary methods
    _HDLLu HMGSynchronousMessage();
    _HDLLu virtual ~HMGSynchronousMessage();
protected:
    _HDLLu HMGSynchronousMessage(const HMGSynchronousMessage& pi_rObj);
    HMGSynchronousMessage& operator=(const HMGSynchronousMessage& pi_rObj);
    };
