//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCResourceLoader.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HFCMacros.h>
#include <Imagepp/all/h/ImagePPMessages.xliff.h>
#include <Imagepp/h/ImagePPExceptionMessages.xliff.h>

class HFCResourceLoader
    {
    // Singleton
    HFC_DECLARE_SINGLETON_DLL(_HDLLu, HFCResourceLoader)

public:

#if defined (OLD_RSC_WAY)
    _HDLLu WString      GetString(unsigned int stringID) const;
    _HDLLu WString      GetString(unsigned int stringID, unsigned int tableID) const;
#else
    _HDLLu WString     GetString(ImagePPMessages stringID) const;
    _HDLLu WString     GetExceptionString(ExceptionID stringID) const;
#endif

private:

    HFCResourceLoader();
    ~HFCResourceLoader();
    };
