/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/ServerQueryHelperTests.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "../WebServicesTestsHelper.h"

class ServerQueryHelperTests : public ::testing::Test
    {
    public:
        static ECSchemaPtr GetTestSchema();
        static void AddCalculatedECPropertySpecification
            (
            ECSchemaPtr schema,
            Utf8StringCR className,
            Utf8StringCR propertyName,
            Utf8StringCR ecExpression
            );
    };
