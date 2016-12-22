/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CompatibilityTests/Cache/TestData.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "TestData.h"

void PrintTo(const TestRepository& value, ::std::ostream* os)
    {
    *os << Utf8PrintfString("%s (%s)", value.serverUrl.c_str(), value.id.c_str());
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
