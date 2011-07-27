/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECObjectsPch.h $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ECObjects\ECObjectsAPI.h>
#include <sstream>
#include <assert.h>
#include <boost\foreach.hpp>
#include "ecxml.h"
#include "Logger.h"
#include "FileUtilities.h"
#include <Logging\bentleylogging.h>

#include "LeakDetector.h"

BEGIN_BENTLEY_EC_NAMESPACE
extern ECObjectsStatus GetMinorVersionFromSchemaFileName (UInt32& versionMinor, WCharCP filePath);
END_BENTLEY_EC_NAMESPACE




  