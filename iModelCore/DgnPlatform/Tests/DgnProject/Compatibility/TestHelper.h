/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/TestHelper.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "CompatibilityTestFixture.h"
#include "ProfileManager.h"

//=======================================================================================
//! Provides helper methods for testing certain areas of a DgnDb or ECDb file in the compatibility tests
// @bsiclass                                                 Krischan.Eberle     06/2018
//=======================================================================================    
struct TestHelper final
    {
private:
    TestFile const& m_testFile;
    ECDbCR m_db;

public:
    TestHelper(TestFile const& testFile, ECDbCR bimOrECDb) : m_testFile(testFile), m_db(bimOrECDb) {}

    JsonValue ExecuteECSqlSelect(Utf8CP ecsql) const;
    SchemaVersion GetSchemaVersion(Utf8CP schemaName) const;
    BeVersion GetOriginalECXmlVersion(Utf8CP schemaName) const;
    int GetSchemaCount() const;
    JsonValue GetSchemaItemCounts(Utf8CP schemaName) const;
    
    void AssertEnum(Utf8CP schemaName, Utf8CP enumName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, ECN::PrimitiveType expectedType, bool expectedIsStrict, std::vector<std::tuple<Utf8CP, ECN::ECValue, Utf8CP>> const& expectedEnumerators) const;
    void AssertKindOfQuantity(Utf8CP schemaName, Utf8CP koqName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, Utf8CP expectedPersistenceUnit, JsonValue const& expectedPresentationFormats, double expectedRelError) const;
    void AssertLoadSchemas() const;
    };