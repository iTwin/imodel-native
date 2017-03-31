/*--------------------------------------------------------------------------------------+
|
|     $Source: src/PointoolsVortexAPIInternal.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

// pragma once is giving 'error: #pragma once in main file' with GCC. Looks like a GCC bug :
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=47857
//#pragma once

#ifndef __VORTEXINTERNAL_H__
#define __VORTEXINTERNAL_H__

// C header files
#include <assert.h>#include <cassert>#include <cstdarg>#include <stdio.h>#include <math.h>#include <exception>#include <cstdio>#include <time.h>#include <iomanip>#include <memory.h>#include <stdarg.h>#include <wchar.h>#include <locale.h>#include <string.h>#include <stdexcept>#include <inttypes.h>
#include <map>#include <string>#include <vector>#include <list>#include <queue>#include <deque>#include <algorithm>#include <iomanip>#include <iostream>#include <set>#include <fstream>#include <stack>#include <bitset>#include <mutex>#include <random>#include <thread>
// Windows Header Files:
#if defined (BENTLEY_WIN32) 
    #include <windows.h>    #include <Shlwapi.h>    #include <Commdlg.h>
    // Conflict with DateTime::GetCurrentTime
    #ifdef GetCurrentTime
        #undef GetCurrentTime
    #endif
#else
    
#define LOBYTE(w)           ((uint8_t)(((uintptr_t)(w)) & 0xff))    
#define GetRValue(rgb)      (LOBYTE(rgb))
#define GetGValue(rgb)      (LOBYTE(((uint16_t)(rgb)) >> 8))
#define GetBValue(rgb)      (LOBYTE((rgb)>>16))
#define RGB(r,g,b)          ((uint32_t)(((uint8_t)(r)|((uint16_t)((uint8_t)(g))<<8))|(((uint16_t)(uint8_t)(b))<<16)))

#endif


#ifdef _DEBUG
#define FILE_TRACE 1
#endif

#include <ttl/var/variant.hpp>
#ifdef HAVE_WILDMAGIC
#include <WildMagic/math/Wm5Quaternion.h>#include <WildMagic/math/Wm5matrix3.h>#include <WildMagic/math/Wm5ApprPlaneFit3.h>#include <WildMagic/math/Wm5Plane3.h>#include <WildMagic/math/Wm5ApprGaussPointsFit3.h>#else
#include <Geom/GeomApi.h>#endif

#include <loki/Singleton.h>#include <loki/AssocVector.h>
#if defined(HAVE_OPENGL)
#include <GL/glew.h>#include <GL/gl.h>#include <GL/glu.h>#include <GL/glext.h>#include <ptengine/renderengine.h>#endif


#include <Bentley/Bentley.h>#include <Bentley/BeFile.h>#include <Bentley/BeFileName.h>#include <Bentley/BeStringUtilities.h>#include <Bentley/BeTimeUtilities.h>#include <Bentley/BeThread.h>    #include <Bentley/BeSharedMutex.h>#include <Bentley/DateTime.h>    #include <ptapi/PointoolsVortexAPI.h>#include <ptengine/pointsScene.h>#include <ptengine/renderContext.h>#include <ptfs/filepath.h>#include <pt/geomtypes.h>#include <pt/ptmath.h>#include <ptcloud2/bitvector.h>#include <math/matrix_math.h>
#if defined (NEEDS_WORK_VORTEX_DGNDB)
    #include <PTRMI/Manager.h>#endif
#include <ptengine/clipManager.h>
#endif //__VORTEXINTERNAL_H__

