//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMGMessageReceiver.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HMGMessageReceiver
//-----------------------------------------------------------------------------
// This class describes message receiver used in the HMG messaging mechanism
//-----------------------------------------------------------------------------
#pragma once


// Forward class declarations
class HMGMessageSender;
class HMGSink;
class HMGMessage;

///////////////////////////////////////////////
// Declaration of class HMGMessageReceiver
///////////////////////////////////////////////

class HMGMessageReceiver
    {
    friend class HMGSink;

public:

    ///////////////////
    // Primary methods
    ///////////////////

    _HDLLu HMGMessageReceiver();
    _HDLLu HMGMessageReceiver(const HMGMessageReceiver& pi_rObj);

    _HDLLu virtual ~HMGMessageReceiver();

    HMGMessageReceiver& operator=(const HMGMessageReceiver& pi_rObj);

    // Make the object process a received message
    _HDLLu virtual bool   ProcessMessage(const HMGMessage& pi_rMessage);

    // Call Link() on the specified sender.
    _HDLLu virtual void    LinkTo(HMGMessageSender* pi_pSender,
                                  bool pi_ReceiveMessagesInCurrentThread = false) const;

    // Call Unlink() on the specified sender.
    _HDLLu void            UnlinkFrom(HMGMessageSender* pi_pSender) const;

protected:

    ////////////////
    // Methods
    ////////////////

    // Tell if the receiver needs the coherence security mechanism
    _HDLLu virtual bool   NeedsCoherenceSecurity() const;

    // Constructs our sink object if necessary
    _HDLLu void            EnsureSinkIsConstructed() const;

    // Called automatically when one of our linked senders
    // we're still linked to gets deleted.
    _HDLLu virtual void    SenderDeleted(HMGMessageSender* pi_pSender);

    ////////////////
    // Attributes
    ////////////////

    // Our sink object.
    HFCPtr<HMGSink> m_pSink;

    };

