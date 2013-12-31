/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECObjectsPch.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>
#include <Bentley/DateTime.h>
#include <Bentley/CatchNonPortable.h>
#include <ECObjects/ECObjectsAPI.h>
#include <assert.h>
#include <Bentley/BeStringUtilities.h>
#include "ecxml.h"
#include "DateTimeInfoAccessor.h"
#include "ECObjectsNativeLog.h"
#include "FileUtilities.h"
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/BeFileName.h>
#include <ECObjects/BeXmlCommonGeometry.h>
#include <Logging/bentleylogging.h>
#include <ECObjects/CalculatedProperty.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
extern ECObjectsStatus GetMinorVersionFromSchemaFileName (UInt32& versionMinor, WCharCP filePath);
END_BENTLEY_ECOBJECT_NAMESPACE


