//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/h/hstdcpp.h $
//:>
//:>  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//*****************************************************************************
// hstdcpp.h
//
//      Header for HMR C++ standards
//
//*****************************************************************************
#pragma once

// Some color space conversion need a Gamma value. Provide the standard default
// gamma factor for the PC world. (MAC is 1.8)
#define  DEFAULT_GAMMA_FACTOR   2.2

// htypes.h is the header for the HMR C standards
// Including it here allows to call HMR C libraries from HMR C++ code
#include "HTypes.h"
#include "renew.h"
#include "HArrayAutoPtr.h"
#include "HAutoPtr.h"

// Precompiled Header files list
#if defined (ANDROID) || defined (__APPLE__)

#   include <fstream>
#   include <ostream>
#   include <iostream>
#   include <memory>
#   include <vector>
#   include <list>
#   include <map>
#   include <algorithm>
#   include <deque>
#   include <set>
#   include <string>
#   include <sstream>
#   include <iosfwd>
#   include <locale>
#   include <iterator>
#   include <iosfwd>
#   include <utility>
#   include <stack>
//#   include <cpml.h>

using namespace std;

#elif defined (_WIN32)
#   include <fstream>
#   include <ostream>
#   include <iostream>
#   include <memory>
#   include <vector>
#   include <list>
#   include <map>
#   include <algorithm>
#   include <deque>
#   include <set>
#   include <numeric>

#   include <string>
#   include <sstream>
#   include <iosfwd>
#   include <locale>
#   include <iterator>
#   include <iosfwd>
#   include <utility>
#   include <stack>

#   include <Winsock2.h>
#   include <Winerror.h>
#   include <wininet.h>

using namespace std;     // Used by STL with VisualC++ 5.00
#else
#   error Unknown compiler - No STL inclusion Standard defined
#endif

// Include after the using std command
#include "HNumeric.h"

// Include after the STL includes.
#include "HStlStuff.h"


// For windows dependant stuff ?????
#if defined(_WIN32) || defined(WIN32)

#   ifndef NO_STRICT        // Compile in STRICT mode with VC5, it is the default with VC6
#       ifndef STRICT
#           define STRICT 1
#       endif
#   endif

#  if !defined(WIN32_LEAN_AND_MEAN)
#    define WIN32_LEAN_AND_MEAN
#    include "windows.h"
#    undef WIN32_LEAN_AND_MEAN
#  else
#    include "windows.h"
#  endif

// We will need _WIN32_WINNT to activate extended COM features... See HRFMacros.h
// (WINVER is defined by windows.h)
#  if !defined(_WIN32_WINNT)
#    if defined(_OBJBASE_H_)
#      error objbase.h must be included after _WIN32_WINNT is defined
#    endif
#    define _WIN32_WINNT WINVER
#  endif

#endif


// DLL support (default values)
//
// See in HDllSupport.h for more information
//
#define _HDLLNone
#if !defined (_HDLL_SUPPORT)
#   define _HDLLu                                  // UtlLibs
#   define _HDLLw                                  // WinLibs
#   define _HDLLg                                  // GraLibs
#   define _HDLLn                                  // NetLibs
#   define IPPIMAGING_EXPORT
#endif

// General compiler Include files
#if defined (ANDROID) || defined (__APPLE__)

// Minimum and maximum macros - These macros are defined in stdlib.h for Windows.
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#define min(a,b)  (((a) < (b)) ? (a) : (b))

#elif defined (_WIN32)
#   include <conio.h>
#   include <direct.h>
#   include <io.h>
#   include <wtypes.h>
#   include <urlmon.h>
#   include <initguid.h>

#else
#   error Unknown compiler - No STL inclusion Standard defined
#endif


#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utime.h>
#include <time.h>
#include <math.h>
#include <fcntl.h>


// Bentley include files
#include <Bentley/BeThread.h>
#include <Bentley/BeFile.h>
#include <Bentley/BeFileName.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/BeNumerical.h>
#include <Bentley/BeFileListIterator.h>

// I++ Include files
#include "../all/h/ImageppLib.h"
#include "../all/h/HFCExclusiveKey.h"
#include "../all/h/HFCPtr.h"
#include "../all/h/HFCBuffer.h"
#include "../all/h/HFCMatrix.h"













