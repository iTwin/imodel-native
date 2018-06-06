/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/TestHelper.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "CompatibilityTestFixture.h"

//=======================================================================================
//! Provides helper methods for testing certain areas of a DgnDb or ECDb file in the compatibility tests
// @bsiclass                                                 Krischan.Eberle     06/2018
//=======================================================================================    
struct TestHelper final
    {
    static JsonValue ExecuteECSqlSelect(ECDb const&, Utf8CP ecsql);
    static SchemaVersion GetSchemaVersion(ECDb const&, Utf8CP schemaName);
    static BeVersion GetOriginalECXmlVersion(ECDb const&, Utf8CP schemaName);

    static void AssertEnum(ECDb const&, Utf8CP schemaName, Utf8CP enumName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, ECN::PrimitiveType expectedType, bool expectedIsStrict, std::vector<std::pair<ECN::ECValue, Utf8CP>> const& expectedEnumerators);
    static void AssertKindOfQuantity(ECDb const&, Utf8CP schemaName, Utf8CP koqName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, Utf8CP expectedPersistenceUnit, JsonValue const& expectedPresentationUnits, double expectedRelError);

    };