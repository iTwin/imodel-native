//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFMessages.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

