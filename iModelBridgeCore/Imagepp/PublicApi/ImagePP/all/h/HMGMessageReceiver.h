//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMGMessageReceiver.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HMGMessageReceiver
//-----------------------------------------------------------------------------
// This class describes message receiver used in the HMG messaging mechanism
//-----------------------------------------------------------------------------
#pragma once

BEGIN_IMAGEPP_NAMESPACE

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

    IMAGEPP_EXPORT HMGMessageReceiver();
    IMAGEPP_EXPORT HMGMessageReceiver(const HMGMessageReceiver& pi_rObj);

    IMAGEPP_EXPORT virtual ~HMGMessageReceiver();

    HMGMessageReceiver& operator=(const HMGMessageReceiver& pi_rObj);

    // Make the object process a received message
    IMAGEPP_EXPORT virtual bool   ProcessMessage(const HMGMessage& pi_rMessage);

    // Call Link() on the specified sender.
    IMAGEPP_EXPORT virtual void    LinkTo(HMGMessageSender* pi_pSender,
                                  bool pi_ReceiveMessagesInCurrentThread = false) const;

    // Call Unlink() on the specified sender.
    IMAGEPP_EXPORT void            UnlinkFrom(HMGMessageSender* pi_pSender) const;

protected:

    ////////////////
    // Methods
    ////////////////

    // Tell if the receiver needs the coherence security mechanism
    IMAGEPP_EXPORT virtual bool   NeedsCoherenceSecurity() const;

    // Constructs our sink object if necessary
    IMAGEPP_EXPORT void            EnsureSinkIsConstructed() const;

    // Called automatically when one of our linked senders
    // we're still linked to gets deleted.
    IMAGEPP_EXPORT virtual void    SenderDeleted(HMGMessageSender* pi_pSender);

    ////////////////
    // Attributes
    ////////////////

    // Our sink object.
    HFCPtr<HMGSink> m_pSink;

    };

END_IMAGEPP_NAMESPACE