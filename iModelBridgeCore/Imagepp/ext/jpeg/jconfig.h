//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
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
