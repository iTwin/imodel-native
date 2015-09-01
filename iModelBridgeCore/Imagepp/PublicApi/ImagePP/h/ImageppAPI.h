//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/h/ImageppAPI.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <Bentley/Bentley.h>
#include <Bentley/BeAtomic.h>
#include <ImagePP/h/ExportMacros.h>
#include <ImagePP/h/HTypes.h>
#include <ImagePP/h/renew.h>
#include <ImagePP/h/HArrayAutoPtr.h>
#include <ImagePP/h/HAutoPtr.h>
#include <ImagePP/h/ImagePPClassId.h>

#include <fstream>
#include <ostream>
#include <iostream>
#include <memory>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <deque>
#include <set>
#include <string>
#include <sstream>
#include <iosfwd>
#include <locale>
#include <iterator>
#include <iosfwd>
#include <utility>
#include <stack>

using namespace std;

// General compiler Include files
#if defined (ANDROID) || defined (__APPLE__)

#elif defined (_WIN32)

#include <numeric>

#else
#   error Unknown compiler - No STL inclusion Standard defined
#endif

// Include after the using std command
#include "HNumeric.h"

// Include after the STL includes.
#include "HStlStuff.h"

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
#include <BeSQLite/L10N.h>

// I++ Include files
#include "../all/h/ImageppLib.h"
#include "../all/h/HFCExclusiveKey.h"
#include "../all/h/HFCPtr.h"
#include "../all/h/HFCBuffer.h"
#include "../all/h/HFCMatrix.h"

// ///////////////////////////////////////////////////////////////////////////////////////////////////
// ImagePP types
// ///////////////////////////////////////////////////////////////////////////////////////////////////
IMAGEPP_TYPEDEFS(HRAImageBuffer)
IMAGEPP_REF_COUNTED_PTR(HRAImageBuffer)

IMAGEPP_TYPEDEFS(HRACopyToOptions)

IMAGEPP_TYPEDEFS(HRAImageSurface)
IMAGEPP_REF_COUNTED_PTR(HRAImageSurface)

IMAGEPP_REF_COUNTED_PTR(HRASampleSurface)
IMAGEPP_TYPEDEFS(HRASampleN1Surface)
IMAGEPP_REF_COUNTED_PTR(HRASampleN1Surface)
IMAGEPP_TYPEDEFS(HRASampleN8Surface)
IMAGEPP_REF_COUNTED_PTR(HRASampleN8Surface)

IMAGEPP_TYPEDEFS(HRAPacketSurface)
IMAGEPP_TYPEDEFS(HRAPacketN1Surface)
IMAGEPP_TYPEDEFS(HRAPacketN8Surface)
IMAGEPP_TYPEDEFS(HRAPacketCodecRleSurface)
IMAGEPP_REF_COUNTED_PTR(HRAPacketCodecRleSurface)
IMAGEPP_TYPEDEFS(HRAPacketRleSurface)
IMAGEPP_REF_COUNTED_PTR(HRAPacketRleSurface)

IMAGEPP_TYPEDEFS(HRAImageBufferRle)
IMAGEPP_REF_COUNTED_PTR(HRAImageBufferRle)

IMAGEPP_TYPEDEFS(HRAImageSample)
IMAGEPP_REF_COUNTED_PTR(HRAImageSample)

IMAGEPP_TYPEDEFS(HRAImageOp)
IMAGEPP_REF_COUNTED_PTR(HRAImageOp)

IMAGEPP_TYPEDEFS(HRAImageOpShaper)
IMAGEPP_REF_COUNTED_PTR(HRAImageOpShaper)

IMAGEPP_TYPEDEFS(HRAImageOpPipeLine)

IMAGEPP_TYPEDEFS(ImageNode)
IMAGEPP_REF_COUNTED_PTR(ImageNode)

IMAGEPP_TYPEDEFS(ImageTransformNode)
IMAGEPP_REF_COUNTED_PTR(ImageTransformNode)

IMAGEPP_TYPEDEFS(ImageSourceNode)
IMAGEPP_REF_COUNTED_PTR(ImageSourceNode)

IMAGEPP_TYPEDEFS(ImageSinkNode)
IMAGEPP_REF_COUNTED_PTR(ImageSinkNode)

IMAGEPP_TYPEDEFS(IImageAllocator)
IMAGEPP_REF_COUNTED_PTR(ImageAllocatorRef)












