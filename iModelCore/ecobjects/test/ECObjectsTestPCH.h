/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/nativeatp/ECObjectsTestPCH.h $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#if defined (COMPILING_PUBLISHED_TESTS)
   // Need to reach in and grab this header since it won't be part of the published API yet we still
   // need to utilize it in the published API tests
   #include "..\..\PublicAPI\ECObjects\DesignByContract.h"
#endif

#include <ECObjects\ECObjectsAPI.h>
#include <gtest\gtest.h>

#include "ScenarioTests\TestClasses\VerifierClassesPCH.h"
