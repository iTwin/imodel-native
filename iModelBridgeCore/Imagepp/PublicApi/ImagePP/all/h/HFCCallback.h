//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
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
