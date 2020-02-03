//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRAUpdateSubResProgressIndicator
//----------------------------------------------------------------------------
#pragma once

//----------------------------------------------------------------------------

#include <ImagePP/all/h/HFCProgressIndicator.h>
#include <ImagePP/all/h/HFCMacros.h>

//----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
class HRAUpdateSubResProgressIndicator : public HFCProgressIndicator
    {
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRAUpdateSubResProgressIndicator)

private:

    // Disabled methodes
    HRAUpdateSubResProgressIndicator();
    };
END_IMAGEPP_NAMESPACE
