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
struct TestDb
    {
protected:
    TestFile const& m_testFile;

private:
    virtual ECDbR _GetDb() const = 0;
    virtual ECDb::OpenParams const& _GetOpenParams() const = 0;

protected:
    explicit TestDb(TestFile const& testFile) : m_testFile(testFile) {}

public:
    virtual ~TestDb() {}
    TestFile const& GetTestFile() const { return m_testFile; }
    ECDbR GetDb() const { return _GetDb(); }
    ECDb::OpenParams const& GetOpenParams() const { return _GetOpenParams(); }
    Utf8String GetDescription() const;

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

//=======================================================================================
//! Provides helper methods for testing certain areas of an ECDb file in the compatibility tests
// @bsiclass                                                 Krischan.Eberle     06/2018
//=======================================================================================    
struct TestECDb final : TestDb
    {
private:
    ECDb m_ecdb;
    ECDb::OpenParams m_openParams;

    ECDbR _GetDb() const override { return const_cast<ECDbR> (m_ecdb); }
    ECDb::OpenParams const& _GetOpenParams() const override { return m_openParams; }
public:
    TestECDb(TestFile const& testFile, ECDb::OpenParams const& openParams = ECDb::OpenParams(ECDb::OpenMode::Readonly)): TestDb(testFile), m_openParams(openParams) {}
    ~TestECDb() { Close(); }
    DbResult Open() { return m_ecdb.OpenBeSQLiteDb(m_testFile.GetPath(), m_openParams); }
    void Close()
        {
        if (m_ecdb.IsDbOpen())
            m_ecdb.CloseDb();
        }

    void AssertProfileVersion() const;

    static std::vector<std::unique_ptr<TestECDb>> CreateFor(TestFile const& testFile)
        {
        std::vector<std::unique_ptr<TestECDb>> testECDbs {std::make_unique<TestECDb>(testFile, ECDb::OpenParams(ECDb::OpenMode::Readonly)), std::make_unique<TestECDb>(testFile, ECDb::OpenParams(ECDb::OpenMode::ReadWrite))};
        if (testFile.GetAge() == ProfileState::Age::Older)
            testECDbs.pusb_back(std::make_unique<TestECDb>(testFile, ECDb::OpenParams(ECDb::OpenMode::ReadWrite, ECDb::ProfileUpgradeOptions::Upgrade)));

        return testECDbs;
        }
    };

//=======================================================================================
//! Provides helper methods for testing certain areas of an iModel file in the compatibility tests
// @bsiclass                                                 Krischan.Eberle     06/2018
//=======================================================================================    
struct TestIModel final : TestDb
    {
    private:
        DgnDbPtr m_dgndb = nullptr;
        DgnDb::OpenParams m_openParams;

        ECDbR _GetDb() const override { return *m_dgndb; }
        ECDb::OpenParams const& _GetOpenParams() const override { return m_openParams; }

    public:
        TestIModel(TestFile const& testFile, DgnDb::OpenParams const& openParams = DgnDb::OpenParams(DgnDb::OpenMode::Readonly)) : TestDb(testFile), m_openParams(openParams) {}
        ~TestIModel() { Close(); }
        DbResult Open()
            { 
            DbResult stat = BeSQLite::BE_SQLITE_OK;
            m_dgndb = DgnDb::OpenDgnDb(&stat, m_testFile.GetPath(), m_openParams);
            return stat;
            }

        void Close()
            {
            if (m_dgndb != nullptr && m_dgndb->IsDbOpen())
                m_dgndb->CloseDb();
            }

        void AssertProfileVersion() const;
    };
