/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "../../Utils/WebServicesTestsHelper.h"

class ServerQueryHelperTests : public WSClientBaseTest
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
