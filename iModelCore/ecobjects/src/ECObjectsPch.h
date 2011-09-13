/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECObjectsPch.h $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ECObjects/ECObjectsAPI.h>
#if defined (_WIN32) // WIP_NONPORT
    #include <sstream>
#elif defined (__unix__)
    // *** NEEDS WORK: iostreams not supported on Android
#endif
#include <assert.h>
#include <boost/foreach.hpp>
#include <Bentley/BeStringUtilities.h>
#include "ecxml.h"
#include "Logger.h"
#include "FileUtilities.h"
#include <Logging/bentleylogging.h>
#include <Bentley/BeFileName.h>
#include <ECObjects/BeXmlCommonGeometry.h>
#include "LeakDetector.h"

BEGIN_BENTLEY_EC_NAMESPACE
extern ECObjectsStatus GetMinorVersionFromSchemaFileName (UInt32& versionMinor, WCharCP filePath);
END_BENTLEY_EC_NAMESPACE




  