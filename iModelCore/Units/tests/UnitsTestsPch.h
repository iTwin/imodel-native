/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/UnitsTestsPch.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>
#include <Bentley/bmap.h>
#include <Bentley/BeTest.h>
#include <Bentley/BeTimeUtilities.h>
#include <Logging/bentleylogging.h>
#include <Units/Units.h>

// For now this is in the PrivateAPI, it will be moved into the TestFixture soon
#include "../PrivateAPI/Units/UnitRegistry.h"

#define BEGIN_UNITS_UNITTESTS_NAMESPACE BEGIN_BENTLEY_UNITS_NAMESPACE namespace Tests {
#define END_UNITS_UNITTESTS_NAMESPACE } END_BENTLEY_UNITS_NAMESPACE

using namespace std;
