/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECObjectsPch.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <Bentley/DateTime.h>
#include <Bentley/CatchNonPortable.h>
#include <ECObjects/ECObjectsAPI.h>
#include <assert.h>
#include <Bentley/BeStringUtilities.h>
#include "ecxml.h"
#include "DateTimeInfoAccessor.h"
#include "ECObjectsNativeLog.h"
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/BeFileName.h>
#include <Logging/bentleylogging.h>
#include <ECObjects/CalculatedProperty.h>
#include <ECObjects/ECInstanceIterable.h>
#include <ECObjects/SystemSymbolProvider.h>
#include <BeXml/BeXml.h>


BEGIN_BENTLEY_ECOBJECT_NAMESPACE
extern ECObjectsStatus GetMinorVersionFromSchemaFileName (uint32_t& versionMinor, WCharCP filePath);
END_BENTLEY_ECOBJECT_NAMESPACE


