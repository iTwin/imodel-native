//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMGMessageDuplex.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HMGMessageDuplex
//-----------------------------------------------------------------------------
// This class describes message sender/receivers used in the HMG messaging mechanism
//-----------------------------------------------------------------------------
#pragma once

#include "HMGMessageSender.h"
#include "HMGMessageReceiver.h"



///////////////////////////////////////////////
// Declaration of class HMGMessageDuplex
///////////////////////////////////////////////

class HMGMessageDuplex : public HMGMessageSender, public HMGMessageReceiver
    {
public:

    ///////////////////
    // Primary methods
    ///////////////////

    _HDLLu HMGMessageDuplex();
    HMGMessageDuplex(const HMGMessageDuplex& pi_rObj);


    HMGMessageDuplex& operator=(const HMGMessageDuplex& pi_rObj);

protected:

    ////////////////
    // Methods
    ////////////////

    // Make the object process a received message
    _HDLLu virtual bool   ProcessMessage(const HMGMessage& pi_rMessage);
    };


