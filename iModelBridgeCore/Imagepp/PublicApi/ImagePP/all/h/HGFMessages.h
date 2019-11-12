//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGF message classes
//-----------------------------------------------------------------------------
// Message classes for HMG mechanism used in HGF.
//-----------------------------------------------------------------------------

#pragma once



#include "HMGMessage.h"

BEGIN_IMAGEPP_NAMESPACE

// Forward declarations
class HMGMessageSender;


///////////////////////////
// HGGeometryChangedMsg
///////////////////////////

class HGFGeometryChangedMsg : public HMGAsynchronousMessage
    {
    HDECLARE_CLASS_ID(HGFMsgId_GeometryChanged, HMGAsynchronousMessage)

public:
    HGFGeometryChangedMsg();
    IMAGEPP_EXPORT virtual ~HGFGeometryChangedMsg();

    IMAGEPP_EXPORT virtual HMGMessage* Clone() const override;
protected:
    HGFGeometryChangedMsg(const HGFGeometryChangedMsg& pi_rObj);
    HGFGeometryChangedMsg& operator=(const HGFGeometryChangedMsg& pi_rObj);
    };
END_IMAGEPP_NAMESPACE

#include "HGFMessages.hpp"

