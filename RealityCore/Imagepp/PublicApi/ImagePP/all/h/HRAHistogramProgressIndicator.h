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

BEGIN_IMAGEPP_NAMESPACE
//----------------------------------------------------------------------------

class HRAHistogramProgressIndicator : public HFCProgressIndicator
    {
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRAHistogramProgressIndicator)

private:

    // Disabled methods
    HRAHistogramProgressIndicator();
    };

END_IMAGEPP_NAMESPACE
