//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRP message classes
//-----------------------------------------------------------------------------
// Message classes for HMG mechanism used in HRP.
//-----------------------------------------------------------------------------

#pragma once

#include "HMGMessage.h"

BEGIN_IMAGEPP_NAMESPACE
// Forward declarations
class HMGMessageSender;

///////////////////////////
// HRPPaletteChangedMsg
///////////////////////////

class HRPPaletteChangedMsg : public HMGAsynchronousMessage
    {
    HDECLARE_CLASS_ID(HRPMsgId_PaletteChanged, HMGAsynchronousMessage)

public:
    HRPPaletteChangedMsg();
    IMAGEPP_EXPORT virtual ~HRPPaletteChangedMsg();

    IMAGEPP_EXPORT virtual HMGMessage* Clone() const override;
protected:
    HRPPaletteChangedMsg(const HRPPaletteChangedMsg& pi_rObj);
    HRPPaletteChangedMsg& operator=(const HRPPaletteChangedMsg& pi_rObj);
    };
END_IMAGEPP_NAMESPACE

