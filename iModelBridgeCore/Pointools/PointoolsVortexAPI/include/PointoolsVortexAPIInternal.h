/*--------------------------------------------------------------------------------------+
|
|     $Source: include/PointoolsVortexAPIInternal.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifndef __POINTOOLSVORTEXAPIINTERNAL_H__
#define __POINTOOLSVORTEXAPIINTERNAL_H__

// Windows Header Files:
#ifdef WIN32
#include <windows.h>
#else
#include <stdarg.h>
#endif

#include <shlwapi.h>
#include <shlobj.h>
#include <comutil.h>

// C header files
#include <assert.h>
#include <cassert>
#include <cstdarg>
#include <stdio.h>
#ifdef __INTEL_COMPILER
#include <mathimf.h> //not platform independent
#else
#include <math.h>
#endif
#include <exception>
#include <cstdio>
#include <time.h>
#include <iomanip>
#include <memory.h>
#include <stdarg.h>

#ifdef _DEBUG
#define FILE_TRACE 1
#endif


#pragma warning ( disable : 4100 ) //unreferenced formal parameter

#pragma warning(disable: 4512) // assignment operator could not be generated for pt::ValueToString
#include <ttl/var/variant.hpp>
#pragma warning(default:4512)

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
#include <tchar.h>
#include <strstream>
#include <bitset>
#include <mutex>
#include <random>

#include <omp.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include <ptengine/pointsScene.h>
#include <ptengine/renderContext.h>
#include <ptengine/renderengine.h>
#include <ptfs/filepath.h>
#include <pt/geomtypes.h>
#include <pt/ptmath.h>
#include <ptcloud2/bitvector.h>
#include <math/matrix_math.h>
#include <PTRMI/Manager.h>
#include <ptengine/ClipManager.h>


#endif
