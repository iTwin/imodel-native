/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CompatibilityTests/Parser/TestData.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "TestData.h"

void PrintTo(const TestRepository& value, ::std::ostream* os)
    {
    Utf8String prefix;
    if (!value.label.empty())
        prefix = value.label + ", ";

    if (value.schemasDir.empty())
        *os << Utf8PrintfString("%sServer: %s | %s", prefix.c_str(), value.serverUrl.c_str(), value.id.c_str());
    else
        *os << Utf8PrintfString("%sSchemas: %s", prefix.c_str(), Utf8String(value.schemasDir).c_str());
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
