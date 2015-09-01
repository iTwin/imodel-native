//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMGSink.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HMGSink
//-----------------------------------------------------------------------------
// This class describes message sink used in the HMG messaging mechanism
//-----------------------------------------------------------------------------
#pragma once

BEGIN_IMAGEPP_NAMESPACE

// Forward class declarations. These are defined elsewhere.
class HMGMessageReceiver;
class HMGMessageSender;
class HMGMessage;


///////////////////////////////////////////////
// Declaration of class HMGMessageEnvelope
///////////////////////////////////////////////

class HMGMessageEnvelope : public HFCShareableObject<HMGMessageEnvelope>
    {
    HDECLARE_SEALEDCLASS_ID(HMGMessageId_Envelope)

public:

    // For persistence only!
    HMGMessageEnvelope();

    HMGMessageEnvelope(const HMGMessage& pi_rMessage);

    HMGMessageEnvelope(const HMGMessageEnvelope& pi_rObj);

    ~HMGMessageEnvelope();

    HMGMessageEnvelope& operator=(const HMGMessageEnvelope& pi_rObj);

    bool operator< (const HMGMessageEnvelope& pi_rObj) const {
        return false;
        }
    bool operator== (const HMGMessageEnvelope& pi_rObj) const {
        return false;
        }

    // Approving
    void            Approve();
    bool           IsApproved() const;

    // Verify message sender
    bool           ComesFrom(HMGMessageSender* pi_pSender) const;

    // Retrieve the real message
    HMGMessage&     GetMessageText() const;

private:

    // The message in the envelope...
    HMGMessage*     m_pMessage;

    // Is the message approved.
    bool           m_Approved;
    };


///////////////////////////////////////////////
// Declaration of class HMGSink
///////////////////////////////////////////////

class HMGSink : public HFCShareableObject<HMGSink>
    {
    HDECLARE_SEALEDCLASS_ID(HMGSinkId_Base)

public:

    ///////////////////
    // Primary methods
    ///////////////////

    IMAGEPP_EXPORT HMGSink(HMGMessageReceiver* pi_pReceiver = 0);

    IMAGEPP_EXPORT ~HMGSink();

    ////////////////
    // Added
    ////////////////

    // Receive a message
    void            Notify(const HMGMessage& pi_rMessage);

    // Container management
    void            ContainerLoaded(HMGMessageReceiver* pi_pReceiver);
    void            ContainerUnloaded();
    void            ContainerSaved();

    // Sender management
    void            SenderSaved(HMGMessageSender* pi_pSender);
    void            SenderDestroyed(HMGMessageSender* pi_pSender);
    IMAGEPP_EXPORT void     SenderUnlinked(HMGMessageSender* pi_pSender);

    // Validation
    bool           IsSinkValid() const;

private:

    //////////////////
    // Methods
    //////////////////

    // These are not useful
    HMGSink(const HMGSink& pi_rObj);
    HMGSink& operator=(const HMGSink& pi_rObj);

    //////////////////
    // Types
    //////////////////

    // Type used for the message queue
    typedef vector<HMGMessageEnvelope, allocator<HMGMessageEnvelope> > MessageQueue;

    //////////////////
    // Attributes
    //////////////////

    // The queue of unsent messages.
    MessageQueue    m_Messages;

    // Pointer to receiver object (our container). Null if
    // the container is not loaded. This attribute not to be saved.
    HMGMessageReceiver* m_pReceiver;

    // Our current position in the message queue
    MessageQueue::size_type m_CurrentQueuePosition;

    // Note if we keep messages that have been processed by the receiver.
    bool           m_KeepMessages;

    // Note if we initialized the coherence security flag
    bool           m_CoherenceSecurityInitialized;

    static HFCExclusiveKey* s_pInternalKey;
    };



END_IMAGEPP_NAMESPACE