/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <Bentley/DateTime.h>
#include <Bentley/CatchNonPortable.h>
#include <ECObjects/ECObjectsAPI.h>
#include <assert.h>
#include <Bentley/BeStringUtilities.h>
#include "../PrivateApi/ECObjects/SchemaParseUtils.h"
#include "../PrivateApi/ECObjects/LegacyUnits.h"
#include "DateTimeInfoAccessor.h"
#include "ECObjectsNativeLog.h"
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/BeFileName.h>
#include <Bentley/Logging.h>
#include <ECObjects/CalculatedProperty.h>
#include <ECObjects/SystemSymbolProvider.h>
#include <BeXml/BeXml.h>
#include <pugixml/src/pugixml.hpp>
#include <pugixml/src/BePugiXmlHelper.h>
#include <Units/Units.h>
#include <Formatting/FormattingApi.h>