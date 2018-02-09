/*--------------------------------------------------------------------------------------+
|
|     $Source: TestUtils.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/DgnPlatformApi.h>
#include <json/value.h>

// =====================================================================================================================
// Test structure to contain all native test functions
//
// A test function should limit any returning values to a Utf8String in JSON string format, in order to wrap up
// all returning values as a JSON string to send back over to JS
//
// Note: Any new tests should be appended to the bottom and will "represent" the test for the next id value, although
//		 this value means nothing until actually used as a value for redirection in nodejs_addon.cpp
// =====================================================================================================================
struct TestUtils
{																	   // ID #
	static Json::Value RotateCameraLocal(DgnDbR, Utf8String);		   // 1
	static Json::Value LookAtVolume(DgnDbR, Utf8String);			   // 2
	static Json::Value LookAtUsingLensAngle(DgnDbR, Utf8String);	   // 3
	static Json::Value DeserializeGeometryStream(DgnDbR, Utf8String);  // 4
	static Json::Value BuildKnownGeometryStream(DgnDbR, Utf8String);   // 5
};
