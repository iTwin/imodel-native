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
#include <assert.h>
#include <cassert>
#include <cstdarg>
#include <stdio.h>
#include <math.h>
#include <exception>
#include <cstdio>
#include <time.h>
#include <iomanip>
#include <memory.h>
#include <stdarg.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>
#include <stdexcept>
#include <inttypes.h>

#include <map>
#include <string>
#include <vector>
#include <list>
#include <queue>
#include <deque>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <set>
#include <fstream>
#include <stack>
#include <bitset>
#include <mutex>
#include <random>
#include <thread>
#include <omp.h>

// Windows Header Files:
#if defined (BENTLEY_WIN32) 
    #include <windows.h>
    #include <Commdlg.h>
    #include <winsock.h>
    #include <Shlwapi.h>
    #include <io.h>
    #include <shlobj.h>
    #include <strstream>
#else
    // NEEDS_WORK_VORTEX_DGNDB
    // use BeStringUtilities::Wcsncpy
    #define wcscpy_s
    #define wcsncpy_s
    #define _snwprintf_s
    #define wcsnlen_s

    #define lstrcmpW wcscmp
    #define wcsnlen(str,size) wcslen(str)

    #define strcpy_s(dest, destSize, src) BeStringUtilities::Strncpy(dest, destSize, src, BeStringUtilities::AsManyAsPossible);

    #define FILE_ATTRIBUTE_NORMAL 0x00000080  
    #define PathStripPathW
    #define PathRemoveExtensionW
    #define PathRemoveFileSpecW
    #define PathFindExtensionW
    #define PathRemoveFileSpecW
    #define PathAppendW
    #define PathRenameExtensionW
    #define PathRelativePathToW
    #define SetCurrentDirectoryW
    #define _waccess(a,b) 1
    #define DeleteFileW(a)

    #define LOBYTE(w)           ((uint8_t)(((uintptr_t)(w)) & 0xff))    
    #define GetRValue(rgb)      (LOBYTE(rgb))
    #define GetGValue(rgb)      (LOBYTE(((uint16_t)(rgb)) >> 8))
    #define GetBValue(rgb)      (LOBYTE((rgb)>>16))
    #define RGB(r,g,b)          ((uint32_t)(((uint8_t)(r)|((uint16_t)((uint8_t)(g))<<8))|(((uint16_t)(uint8_t)(b))<<16)))

    #define Sleep(t_ms)   BeThreadUtilities::BeSleep(t_ms)

    typedef void* HANDLE;
   

    #define swscanf_s swscanf
    #define sprintf_s BeStringUtilities::Snprintf

    char* itoa(int value, char * str, int base)
        {
        sprintf(str, "%d", value);
        return str;
        }


#endif




#ifdef _DEBUG
#define FILE_TRACE 1
#endif

#include <ttl/var/variant.hpp>

#include <WildMagic/math/Wm5Quaternion.h>
#include <WildMagic/math/Wm5matrix3.h>
#include <WildMagic/math/Wm5ApprPlaneFit3.h>
#include <WildMagic/math/Wm5Plane3.h>
#include <WildMagic/math/Wm5ApprGaussPointsFit3.h>

#include <Loki/Singleton.h>
#include <Loki/AssocVector.h>

#if defined(HAVE_OPENGL)
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <ptengine/renderengine.h>
#endif

#include <Bentley/Bentley.h>
#include <Bentley/BeFileName.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/BeThread.h>    
#include <ptapi/PointoolsVortexAPI.h>
#include <ptengine/pointsScene.h>
#include <ptengine/renderContext.h>
#include <ptfs/filepath.h>
#include <pt/geomtypes.h>
#include <pt/ptmath.h>
#include <ptcloud2/bitvector.h>
#include <math/matrix_math.h>

#if defined (BENTLEY_WIN32)     //NEEDS_WORK_VORTEX_DGNDB 
    #include <PTRMI/Manager.h>
#endif
#include <ptengine/ClipManager.h>

#endif //__VORTEXINTERNAL_H__

