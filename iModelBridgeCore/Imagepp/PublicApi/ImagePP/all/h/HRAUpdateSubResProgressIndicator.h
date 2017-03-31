//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAUpdateSubResProgressIndicator.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
