/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECObjectsPch.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#if defined (_WIN32) // WIP_NONPORT
    #include <sstream>
    #include <atlbase.h>
    #include <windows.h>
#elif defined (__unix__)
    // *** NEEDS WORK: iostreams not supported on Android
#endif

#include <ECObjects/ECObjectsAPI.h>
#include <assert.h>
#include <boost/foreach.hpp>
#include <Bentley/BeStringUtilities.h>
#include "ecxml.h"
#include "Logger.h"
#include "FileUtilities.h"
#include "StopWatch.h"
#include <Bentley/BeFileName.h>
#include <ECObjects/BeXmlCommonGeometry.h>
#include <Logging/bentleylogging.h>
#include "LeakDetector.h"

BEGIN_BENTLEY_EC_NAMESPACE
extern ECObjectsStatus GetMinorVersionFromSchemaFileName (UInt32& versionMinor, WCharCP filePath);
END_BENTLEY_EC_NAMESPACE


