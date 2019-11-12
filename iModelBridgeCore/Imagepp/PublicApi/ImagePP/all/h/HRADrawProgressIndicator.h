//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRADrawProgressIndicator
//----------------------------------------------------------------------------
#pragma once

//----------------------------------------------------------------------------

#include <ImagePP/all/h/HFCProgressIndicator.h>
#include <ImagePP/all/h/HFCMacros.h>

BEGIN_IMAGEPP_NAMESPACE
//----------------------------------------------------------------------------

class HRADrawProgressIndicator : public HFCProgressIndicator
    {
private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRADrawProgressIndicator)

    // Disabled methodes
    HRADrawProgressIndicator();
    };


END_IMAGEPP_NAMESPACE
