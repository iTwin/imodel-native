//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCFileManager.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCMacros.h"
#include "HFCPtr.h"
#include "HFCURLFile.h"


class HFCFileManager
    {
public:
    _HDLLu bool Remove(const HFCPtr<HFCURLFile>& pi_rpURL);

private:
    HFC_DECLARE_SINGLETON_DLL(_HDLLu, HFCFileManager)

    HFCFileManager();
    };
