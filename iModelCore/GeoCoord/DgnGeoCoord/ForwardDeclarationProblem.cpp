/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnGeoCoord/ForwardDeclarationProblem.cpp $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma component(browser, off)
#pragma component( minrebuild, off )
#pragma warning(disable:4189) // Initialized but not used

#using <mscorlib.dll>
using namespace System;

#pragma unmanaged
#include <stddef.h>     // For #define of NULL
#pragma managed

namespace Bentley {
    namespace DgnPlatform {

    struct Run {};
    void bentleyUstnForwardStructDefintions ()
        {
        struct Run *            runP    = NULL;
        }
}};


