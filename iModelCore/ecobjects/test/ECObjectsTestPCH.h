/*--------------------------------------------------------------------------------------+
|
|     $Source: test/ECObjectsTestPCH.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <boost/foreach.hpp>

#if defined (COMPILING_PUBLISHED_TESTS)
   // Need to reach in and grab this header since it won't be part of the published API yet we still
   // need to utilize it in the published API tests
   #include "..\..\PublicApi\ECObjects\DesignByContract.h"
#else
   #include <ECObjects\DesignByContract.h>
#endif

#if defined (USE_PUBLISHED_HEADERS)
   #include "BackDoor\BackDoor.h"
#endif

#include <ECObjects\ECObjectsAPI.h>
#include <gtest\gtest.h>

#undef FOR_EACH
#define FOR_EACH(V,C) for each (V in C)
