/*--------------------------------------------------------------------------------------+
|
|     $Source: src/PointoolsVortexAPIInternal.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

// Windows Header Files:
#if defined (BENTLEY_WIN32) 
    #include <windows.h>
    #include <Commdlg.h>
    #include <winsock.h>
    #include <Shlwapi.h>
    #include <io.h>
    #include <shlobj.h>
#else
    // NEEDS_WORK_VORTEX_DGNDB
    // use BeStringUtilities::Wcsncpy
    #define wcscpy_s
    #define wcsncpy_s
    #define _snwprintf_s
    #define wcsnlen_s

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



#endif


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

// Standard header files:
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

#include <omp.h>

#if defined(HAVE_OPENGL)
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <ptengine/renderengine.h>
#endif

#include <Bentley/Bentley.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/BeTimeUtilities.h>
#include <ptapi/PointoolsVortexAPI.h>
#include <ptengine/pointsScene.h>
#include <ptengine/renderContext.h>
#include <ptfs/filepath.h>
#include <pt/geomtypes.h>
#include <pt/ptmath.h>
#include <ptcloud2/bitvector.h>
#include <math/matrix_math.h>
#include <PTRMI/Manager.h>
#include <ptengine/ClipManager.h>



