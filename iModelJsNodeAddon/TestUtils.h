/*--------------------------------------------------------------------------------------+
|
|     $Source: TestUtils.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDb.h>
#include <ECDb/ECDbApi.h>
#include <DgnPlatform/ECUtils.h>

#include <json/value.h>
#include <rapidjson/rapidjson.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_DGN_NAMESPACE


// =====================================================================================================================
// Test structure to contain all native test functions
//
// A test function should limit any returning values to a Utf8String in JSON string format, in order to wrap up
// all returning values as a JSON string to send back over to JS
//
// Note: Any new tests should be appended to the bottom and will "represent" the test for the next id value, although
//		 this value means nothing until actually used as a value for redirection in nodejs_addon.cpp
// =====================================================================================================================
struct TestUtils {																						// ID #
	static Json::Value ViewStateCreate(Utf8String);														// 1
	static Json::Value ViewStateVolumeAdjustments(Utf8String);											// 2
	static Json::Value ViewStateLookAt(Utf8String);														// 3
	static Json::Value DeserializeGeometryStream(Utf8String);											// 4
	static Json::Value BuildKnownGeometryStream(Utf8String);												// 5
};


END_BENTLEY_DGN_NAMESPACE