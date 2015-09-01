//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIMStripProgressIndicator.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HIMStripProgressIndicator
//----------------------------------------------------------------------------
#pragma once

//----------------------------------------------------------------------------

#include <Imagepp/all/h/HFCProgressIndicator.h>
#include <Imagepp/all/h/HFCMacros.h>

BEGIN_IMAGEPP_NAMESPACE
//----------------------------------------------------------------------------

class HIMStripProgressIndicator : public HFCProgressIndicator
    {
private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HIMStripProgressIndicator)

    // Disabled methodes
    HIMStripProgressIndicator();
    };

END_IMAGEPP_NAMESPACE

