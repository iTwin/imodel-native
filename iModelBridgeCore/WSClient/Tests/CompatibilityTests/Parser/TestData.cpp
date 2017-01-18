/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CompatibilityTests/Parser/TestData.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "TestData.h"

void PrintTo(const TestRepository& value, ::std::ostream* os)
    {
    if (value.schemasDir.empty())
        *os << Utf8PrintfString("Server: %s | %s", value.serverUrl.c_str(), value.id.c_str());
    else
        *os << Utf8PrintfString("Schemas: %s", Utf8String(value.schemasDir).c_str());
    }

void PrintTo(const TestRepositories& value, ::std::ostream* os)
    {
    *os << "Create: ";
    PrintTo(value.create, os);

    if (!value.upgrade.IsValid())
        return;

    *os << " Upgrade to: ";
    PrintTo(value.upgrade, os);
    }
