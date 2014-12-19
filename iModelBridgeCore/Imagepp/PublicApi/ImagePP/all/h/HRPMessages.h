//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPMessages.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRP message classes
//-----------------------------------------------------------------------------
// Message classes for HMG mechanism used in HRP.
//-----------------------------------------------------------------------------

#pragma once

#include "HMGMessage.h"

// Forward declarations
class HMGMessageSender;

///////////////////////////
// HRPPaletteChangedMsg
///////////////////////////

class HRPPaletteChangedMsg : public HMGAsynchronousMessage
    {
    HDECLARE_CLASS_ID(1097, HMGAsynchronousMessage)

public:
    HRPPaletteChangedMsg();
    _HDLLg virtual ~HRPPaletteChangedMsg();

    _HDLLg virtual HMGMessage* Clone() const override;
protected:
    HRPPaletteChangedMsg(const HRPPaletteChangedMsg& pi_rObj);
    HRPPaletteChangedMsg& operator=(const HRPPaletteChangedMsg& pi_rObj);
    };

