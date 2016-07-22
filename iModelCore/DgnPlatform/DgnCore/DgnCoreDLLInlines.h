/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnCoreDLLInlines.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeThreadLocalStorage.h>

#if defined (_MANAGED)
    #error
#endif

/*--------------------------------------------------------------------------------------+
/* This file exists to allow non-virtual-interface methods to be inlined in
/* the production builds of dgncore.dll, but non-inlined for debug builds (otherwise you can't step into them).
+--------------------------------------------------------------------------------------*/

#if !defined (__DGNPLATFORM_BUILD__)
    #error This file is only valid inside DgnPlatform.dll
#endif

#if defined (NDEBUG) && !defined (__DGNCORE_DONT_INLINE__)
    #define DG_INLINE inline
#else
    #define DG_INLINE
#endif

extern BeThreadLocalStorage g_hostForThread;

DG_INLINE IACSManagerR IACSManager::GetManager() {return T_HOST.GetAcsManager();}
DG_INLINE DgnPlatformLib::Host& DgnPlatformLib::GetHost() {return *static_cast<DgnPlatformLib::Host*>(g_hostForThread.GetValueAsPointer());}

