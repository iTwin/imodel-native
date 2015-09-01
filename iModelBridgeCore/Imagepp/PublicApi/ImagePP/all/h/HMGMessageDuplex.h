//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMGMessageDuplex.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HMGMessageDuplex
//-----------------------------------------------------------------------------
// This class describes message sender/receivers used in the HMG messaging mechanism
//-----------------------------------------------------------------------------
#pragma once

#include "HMGMessageSender.h"
#include "HMGMessageReceiver.h"

BEGIN_IMAGEPP_NAMESPACE

///////////////////////////////////////////////
// Declaration of class HMGMessageDuplex
///////////////////////////////////////////////

class HMGMessageDuplex : public HMGMessageSender, public HMGMessageReceiver
    {
public:

    ///////////////////
    // Primary methods
    ///////////////////

    IMAGEPP_EXPORT HMGMessageDuplex();
    HMGMessageDuplex(const HMGMessageDuplex& pi_rObj);


    HMGMessageDuplex& operator=(const HMGMessageDuplex& pi_rObj);

protected:

    ////////////////
    // Methods
    ////////////////

    // Make the object process a received message
    IMAGEPP_EXPORT virtual bool   ProcessMessage(const HMGMessage& pi_rMessage);
    };


END_IMAGEPP_NAMESPACE