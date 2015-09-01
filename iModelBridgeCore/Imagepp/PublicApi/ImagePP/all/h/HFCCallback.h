//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCCallback.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

BEGIN_IMAGEPP_NAMESPACE
class HFCCallback
    {
    HDECLARE_BASECLASS_ID(HFCCallbackId_Base)

public:
    virtual ~HFCCallback() {};

protected:
    HFCCallback() {};
    };

END_IMAGEPP_NAMESPACE
