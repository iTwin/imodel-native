/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/NotPublished/ConnectC/ConnectWebServicesClientCTests.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "../../Utils/WebServicesTestsHelper.h"
#include "../Connect/ConnectTestsHelper.h"

#include <WebServices/ConnectC/CWSCCPublic.h>
#include <WebServices/ConnectC/CWSCCBufferPublic.h>
#include <WebServices/ConnectC/ConnectWsgGlobal/GlobalSchemaBufferGenPublic.h>
#include <WebServices/ConnectC/ConnectWsgGlobal/GlobalSchemaGenPublic.h>

class ConnectWebServicesClientCTests : public BaseMockHttpHandlerTest
    {
    public:
        static void SetUpTestCase();
    };
