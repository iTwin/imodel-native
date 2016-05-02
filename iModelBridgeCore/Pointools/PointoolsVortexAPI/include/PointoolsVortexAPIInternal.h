/*--------------------------------------------------------------------------------------+
|
|     $Source: include/PointoolsVortexAPIInternal.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

// Windows Header Files:
#include <windows.h>

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

#include <ptapi/PointoolsVortexAPI.h>

