//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCCallbackRegistry.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCMacros.h"

class HFCCallback;

class HFCCallbackRegistry
    {
public:
    HFCCallbackRegistry();
    virtual            ~HFCCallbackRegistry();

    _HDLLu bool             AddCallback(const HFCCallback* pi_pCallback);

    _HDLLu void             RemoveCallback(const HFCCallback* pi_pCallback);

    _HDLLu HFCCallback*     GetCallback(HCLASS_ID pi_CallbackID, unsigned short pi_CallbackInd = 0) const;

    _HDLLu unsigned short   GetNbCallbacks(HCLASS_ID pi_CallbackID) const;

private:

    typedef multimap<HCLASS_ID, HFCCallback*> CallbacksMultiMap;
    CallbacksMultiMap m_Callbacks;

    // Singleton
    HFC_DECLARE_SINGLETON_DLL(_HDLLu, HFCCallbackRegistry)
    };
