/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECObjectsPch.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>
#include <Bentley/CatchNonPortable.h>
#include <ECObjects/ECObjectsAPI.h>
#include <assert.h>
#include <boost/foreach.hpp>
#include <Bentley/BeStringUtilities.h>
#include "ecxml.h"
#include "Logger.h"
#include "StopWatch.h"
#include <Bentley/BeFileName.h>
#include <ECObjects/BeXmlCommonGeometry.h>
#include <Logging/bentleylogging.h>
#include "LeakDetector.h"

BEGIN_BENTLEY_EC_NAMESPACE
extern ECObjectsStatus GetMinorVersionFromSchemaFileName (UInt32& versionMinor, WCharCP filePath);
END_BENTLEY_EC_NAMESPACE


