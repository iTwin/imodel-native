//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRADrawProgressIndicator.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRADrawProgressIndicator
//----------------------------------------------------------------------------
#pragma once

//----------------------------------------------------------------------------

#include <Imagepp/all/h/HFCProgressIndicator.h>
#include <Imagepp/all/h/HFCMacros.h>

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