//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMGMacros.h $
//:>
//:>  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Macros for the HMG messaging mechanism
//-----------------------------------------------------------------------------
#pragma once

//////////////////////////////////////////////////
// For HMGMessageDuplex and HMGMessageReceiver
//////////////////////////////////////////////////


// Last parameter to the message map beginning macros.
// Tell if user wishes to use coherence security.

//#define HMG_NEEDS_COHERENCE_SECURITY    true              // Must be used now, Persistence is removed.
#define HMG_NO_NEED_COHERENCE_SECURITY  false

#define HMG_NO_ARG

// Macro to call inside declaration of receiver class.

#define HMG_DECLARE_MESSAGE_MAP_DLL(HDLL) \
    public: \
        HDLL virtual bool ProcessMessage(const HMGMessage& pi_rMessage); \
    protected: \
        HDLL virtual bool NeedsCoherenceSecurity() const;

#define HMG_DECLARE_MESSAGE_MAP()   HMG_DECLARE_MESSAGE_MAP_DLL(HMG_NO_ARG)


// Macro to call in .cpp file of a receiver or duplex class, to
// start defining the base message map. Used if the ancestor is
// not a receiver.

#define HMG_BEGIN_RECEIVER_MESSAGE_MAP(pi_ClassName, pi_Ancestor, pi_UseCoherence) \
    bool pi_ClassName::NeedsCoherenceSecurity() const\
    { \
        return ((pi_UseCoherence) || pi_Ancestor::NeedsCoherenceSecurity()); \
    } \
    typedef bool (pi_ClassName::*pi_ClassName##HMGMessageHandler)(const HMGMessage&); \
    struct pi_ClassName##HMGMessageMapEntry { \
        HCLASS_ID                       MessageID; \
        pi_ClassName##HMGMessageHandler pHandler; \
    }; \
    extern struct pi_ClassName##HMGMessageMapEntry pi_ClassName##s_MessageMap[]; \
    bool pi_ClassName::ProcessMessage(const HMGMessage& pi_rMessage) \
    { \
        bool Processed = false; \
        bool NeedToPropagate = true; \
        int i = 0; \
        while (!Processed && pi_ClassName##s_MessageMap[i].MessageID != 0) \
        { \
            if (pi_rMessage.IsCompatibleWith(pi_ClassName##s_MessageMap[i].MessageID)) \
            { \
                NeedToPropagate = (this->*(pi_ClassName##s_MessageMap[i].pHandler))(pi_rMessage); \
                Processed = true; \
            } \
            i++; \
        } \
        if (!Processed) NeedToPropagate = pi_Ancestor::ProcessMessage(pi_rMessage); \
        return NeedToPropagate; \
    } \
    struct pi_ClassName##HMGMessageMapEntry pi_ClassName##s_MessageMap[] = {


#define HMG_BEGIN_DUPLEX_MESSAGE_MAP(pi_ClassName, pi_Ancestor, pi_UseCoherence) \
    bool pi_ClassName::NeedsCoherenceSecurity() const \
    { \
        return ((pi_UseCoherence) || pi_Ancestor::NeedsCoherenceSecurity()); \
    } \
    typedef bool (pi_ClassName::*pi_ClassName##HMGMessageHandler)(const HMGMessage&); \
    struct pi_ClassName##HMGMessageMapEntry { \
        HCLASS_ID                       MessageID; \
        pi_ClassName##HMGMessageHandler pHandler; \
    }; \
    extern struct pi_ClassName##HMGMessageMapEntry pi_ClassName##s_MessageMap[]; \
    bool pi_ClassName::ProcessMessage(const HMGMessage& pi_rMessage) { \
        bool Processed = false; \
        bool NeedToPropagate = true; \
        int i = 0; \
        while (!Processed && pi_ClassName##s_MessageMap[i].MessageID != 0) \
            { \
            if (pi_rMessage.IsCompatibleWith(pi_ClassName##s_MessageMap[i].MessageID)) { \
                NeedToPropagate = (this->*(pi_ClassName##s_MessageMap[i].pHandler))(pi_rMessage); \
                Processed = true; \
                } \
            i++; \
            } \
        if (Processed == false) NeedToPropagate = pi_Ancestor::ProcessMessage(pi_rMessage); \
        if (NeedToPropagate == true) { \
            const_cast<HMGMessage&>(pi_rMessage).SetSender(this); \
            Propagate(pi_rMessage); \
        } \
        return false; \
    } \
    pi_ClassName##HMGMessageMapEntry pi_ClassName##s_MessageMap[] = {


// Macro to call in .cpp file of a receiver or duplex class, to
// end the message map definition.

#define HMG_END_MESSAGE_MAP() \
    { 0, 0 } };


// Macro to call in .cpp file of a receiver or duplex class, to
// map messages to member functions. Must be between a HMG_BEGIN_...
// and a HMG_END_MESSAGE_MAP().

#define HMG_REGISTER_MESSAGE(pi_ClassName, pi_MessageClass, pi_pHandler) \
{ pi_MessageClass::CLASS_ID, &pi_ClassName::pi_pHandler },

