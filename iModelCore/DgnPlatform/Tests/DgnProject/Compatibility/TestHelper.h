/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/TestHelper.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "CompatibilityTestFixture.h"
#include "Profiles.h"

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
    
    void AssertEnum(Utf8CP schemaName, Utf8CP enumName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, ECN::PrimitiveType expectedType, bool expectedIsStrict, std::vector<std::pair<ECN::ECValue, Utf8CP>> const& expectedEnumerators) const;
    void AssertKindOfQuantity(Utf8CP schemaName, Utf8CP koqName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, Utf8CP expectedPersistenceUnit, JsonValue const& expectedPresentationUnits, double expectedRelError) const;
    void AssertUnit(Utf8CP schemaName, Utf8CP unitName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, Utf8CP expectedDefinition,
                    Nullable<double> expectedNumerator, Nullable<double> expectedDenominator, Nullable<double> expectedOffset, QualifiedName const& expectedUnitSystem, QualifiedName const& expectedPhenomenon, bool expectedIsConstant, QualifiedName const& expectedInvertingUnit) const;
    void AssertFormat(Utf8CP schemaName, Utf8CP formatName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, JsonValue const& expectedNumericSpec, JsonValue const& expectedCompSpec) const;
    void AssertUnitSystem(Utf8CP schemaName, Utf8CP usName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription) const;
    void AssertPhenomenon(Utf8CP schemaName, Utf8CP phenName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, Utf8CP definition) const;

    void AssertLoadSchemas() const;
    };