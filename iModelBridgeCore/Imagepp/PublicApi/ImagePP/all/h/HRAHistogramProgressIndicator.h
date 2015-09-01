//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAHistogramProgressIndicator.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRAUpdateSubResProgressIndicator
//----------------------------------------------------------------------------
#pragma once

//----------------------------------------------------------------------------

#include <Imagepp/all/h/HFCProgressIndicator.h>
#include <Imagepp/all/h/HFCMacros.h>

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