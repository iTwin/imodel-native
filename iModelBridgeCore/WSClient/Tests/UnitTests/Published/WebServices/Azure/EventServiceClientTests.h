/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Azure/EventServiceClientTests.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "../../Utils/WebServicesTestsHelper.h"

class EventServiceClientTests : public BaseMockHttpHandlerTest
    {
	public:
		bvector<Credentials> credentials;
		bvector<Utf8String> servers;
		void SetUp();
	};

