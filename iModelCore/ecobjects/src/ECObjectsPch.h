/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECObjectsPch.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <Bentley/DateTime.h>
#include <Bentley/CatchNonPortable.h>
#include <ECObjects/ECObjectsAPI.h>
#include <assert.h>
#include <Bentley/BeStringUtilities.h>
#include "../PrivateApi/ECObjects/SchemaParseUtils.h"
#include "DateTimeInfoAccessor.h"
#include "ECObjectsNativeLog.h"
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/BeFileName.h>
#include <Logging/bentleylogging.h>
#include <ECObjects/CalculatedProperty.h>
#include <ECObjects/SystemSymbolProvider.h>
#include <BeXml/BeXml.h>
#include <Units/Units.h>
#include <Formatting/FormattingApi.h>
