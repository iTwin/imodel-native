//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ext/jpeg/jconfig.h $
//:>
//:>  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

//This is a wrapper file that includes that define config for a visual studio compilation. We kept the original extension(.vc) to 
// ease future library update.
#if defined (ANDROID) || defined (__APPLE__)
#include "jconfig.vc"       // vc is suitable also for android and apple
#elif defined (_WIN32)
#include "jconfig.vc"
#endif
