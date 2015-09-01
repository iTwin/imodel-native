//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPMessages.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

