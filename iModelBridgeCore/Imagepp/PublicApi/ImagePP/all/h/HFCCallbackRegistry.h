//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCCallbackRegistry.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCMacros.h"

BEGIN_IMAGEPP_NAMESPACE

class HFCCallback;

class HFCCallbackRegistry
    {
public:
    HFCCallbackRegistry();
    virtual            ~HFCCallbackRegistry();

    IMAGEPP_EXPORT bool             AddCallback(const HFCCallback* pi_pCallback);

    IMAGEPP_EXPORT void             RemoveCallback(const HFCCallback* pi_pCallback);

    IMAGEPP_EXPORT HFCCallback*     GetCallback(HCLASS_ID pi_CallbackID, unsigned short pi_CallbackInd = 0) const;

    IMAGEPP_EXPORT unsigned short   GetNbCallbacks(HCLASS_ID pi_CallbackID) const;

private:

    typedef multimap<HCLASS_ID, HFCCallback*> CallbacksMultiMap;
    CallbacksMultiMap m_Callbacks;

    // Singleton
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HFCCallbackRegistry)
    };

END_IMAGEPP_NAMESPACE