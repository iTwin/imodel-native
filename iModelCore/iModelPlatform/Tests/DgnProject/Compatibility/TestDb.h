/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <cstddef>
#include "CompatibilityTestFixture.h"
#include "Profiles.h"

//=======================================================================================
// Features that were added later. Use TestDb::SupportsFeature to check whether a certain file supports a 
// feature or not.
// @bsiclass
//=======================================================================================    
enum class ECDbFeature
    {
    EC32,
    PersistedECVersions,
    NamedEnumerators,
    UnitsAndFormats,
    SystemPropertiesHaveIdExtendedType
    };

//=======================================================================================
//! Provides helper methods for testing certain areas of a DgnDb or ECDb file in the compatibility tests
// @bsiclass
//=======================================================================================    
struct TestDb
    {
protected:
    TestFile const& m_testFile;
    Nullable<BeSQLite::ProfileState::Age> m_age;

private:
    virtual ECDbR _GetDb() const = 0;
    virtual DbResult _Open() = 0;
    virtual void _Close() = 0;
    virtual ECDb::OpenParams const& _GetOpenParams() const = 0;
    virtual Utf8String _OpenParamsToString() const = 0;

protected:
    explicit TestDb(TestFile const& testFile) : m_testFile(testFile) {}

 public:
    virtual ~TestDb() {}
    TestDb(TestDb const&) = delete;
    TestDb& operator=(TestDb const&) = delete;
    TestDb(TestDb&&) = default;
    TestDb& operator=(TestDb&&) = default;

    DbResult Open();
    void Close() { _Close(); }

    BeSQLite::ProfileState::Age GetAge() const { BeAssert(m_age != nullptr); return m_age.Value(); }
    BeSQLite::ProfileState::Age GetECDbAge() const;
    bool IsUpgraded() const { return GetOpenParams().GetProfileUpgradeOptions() == ECDb::ProfileUpgradeOptions::Upgrade; }
    TestFile const& GetTestFile() const { return m_testFile; }
    ECDbR GetDb() const { return _GetDb(); }
    ECDb::OpenParams const& GetOpenParams() const { return _GetOpenParams(); }
    Utf8String GetDescription() const;
    bool DoesECDbVersionAllowSchemaImport() const;

    bool SupportsFeature(ECDbFeature feature) const { return VersionSupportsFeature(GetDb().GetECDbProfileVersion(), feature); }
    static bool VersionSupportsFeature(ProfileVersion const&, ECDbFeature);
    BeSQLite::ProfileVersion GetECDbInitialVersion() const;
    JsonValue ExecuteECSqlSelect(Utf8CP ecsql) const;
    SchemaVersion GetSchemaVersion(Utf8CP schemaName) const;
    //! Returns the Original ECXML version as persisted in the file.
    //! If the file hasn't persisted the version yet, an empty version is returned
    BeVersion GetOriginalECXmlVersion(Utf8CP schemaName) const;
    int GetSchemaCount() const;
    JsonValue GetSchemaItemCounts(Utf8CP schemaName) const;
    
    void AssertEnum(Utf8CP schemaName, Utf8CP enumName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, ECN::PrimitiveType expectedType, bool expectedIsStrict, std::vector<std::tuple<Utf8CP, ECN::ECValue, Utf8CP>> const& expectedEnumerators) const;
    void AssertEnum(ECN::ECEnumerationCR, Utf8CP schemaName, Utf8CP enumName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, ECN::PrimitiveType expectedType, bool expectedIsStrict, std::vector<std::tuple<Utf8CP, ECN::ECValue, Utf8CP>> const& expectedEnumerators) const;
    void AssertKindOfQuantity(Utf8CP schemaName, Utf8CP koqName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, Utf8CP expectedPersistenceUnit, JsonValue const& expectedPresentationFormats, double expectedRelError) const;
    void AssertKindOfQuantity(ECN::KindOfQuantityCR, Utf8CP expectedSchemaName, Utf8CP expectedKoqName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, Utf8CP expectedPersistenceUnit, JsonValue const& expectedPresentationFormats, double expectedRelError) const;
    void AssertUnit(Utf8CP schemaName, Utf8CP unitName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, Utf8CP expectedDefinition,
                    Nullable<double> expectedNumerator, Nullable<double> expectedDenominator, Nullable<double> expectedOffset, QualifiedName const& expectedUnitSystem, QualifiedName const& expectedPhenomenon, bool expectedIsConstant, QualifiedName const& expectedInvertingUnit) const;
    void AssertFormat(Utf8CP schemaName, Utf8CP formatName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, JsonValue const& expectedNumericSpec, JsonValue const& expectedCompSpec) const;
    void AssertUnitSystem(Utf8CP schemaName, Utf8CP usName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription) const;
    void AssertPhenomenon(Utf8CP schemaName, Utf8CP phenName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, Utf8CP definition) const;

    void AssertLoadSchemas() const;
    };

//=======================================================================================
//! Provides helper methods for testing certain areas of an ECDb file in the compatibility tests
// @bsiclass
//=======================================================================================    
struct TestECDb final : TestDb
    {
public:
    struct Iterable final
        {
        public:
            struct const_iterator final
                {
                using iterator_category=std::forward_iterator_tag;
                using value_type=std::unique_ptr<TestECDb>;
                using difference_type=std::ptrdiff_t;
                using pointer=std::unique_ptr<TestECDb>*;
                using reference=std::unique_ptr<TestECDb>&;

                private:
                    TestFile const& m_testFile;
                    std::vector<ECDb::OpenParams>::const_iterator m_paramsIt;
                public:
                    const_iterator(TestFile const& testFile, std::vector<ECDb::OpenParams>::const_iterator paramsIt) : m_testFile(testFile), m_paramsIt(paramsIt) {}
                    std::unique_ptr<TestECDb> operator* () const { return std::make_unique<TestECDb>(m_testFile, *m_paramsIt); }

                    const_iterator& operator++ () { m_paramsIt++; return *this; }
                    bool operator== (const_iterator const& rhs) const { return m_paramsIt == rhs.m_paramsIt; }
                    bool operator!= (const_iterator const& rhs) const { return !(*this == rhs); }
                };

        private:
            TestFile const& m_testFile;
            std::vector<ECDb::OpenParams> m_params;

        public:
            Iterable(TestFile const& testFile, std::vector<ECDb::OpenParams> const& params) : m_testFile(testFile), m_params(params) {}
            const_iterator begin() const { return const_iterator(m_testFile, m_params.begin()); }
            const_iterator end() const { return const_iterator(m_testFile, m_params.end()); }
        };
private:
    ECDb m_ecdb;
    ECDb::OpenParams m_openParams;

    ECDbR _GetDb() const override { return const_cast<ECDbR> (m_ecdb); }
    DbResult _Open() override;
    void _Close() override
        {
        if (m_ecdb.IsDbOpen())
            {
            m_ecdb.SaveChanges();
            m_ecdb.CloseDb();
            }
        }

    ECDb::OpenParams const& _GetOpenParams() const override { return m_openParams; }
    Utf8String _OpenParamsToString() const override;

public:
    TestECDb(TestFile const& testFile, ECDb::OpenParams const& openParams = ECDb::OpenParams(ECDb::OpenMode::Readonly)): TestDb(testFile), m_openParams(openParams) {}
    ~TestECDb() { _Close(); }

    static Iterable GetPermutationsFor(TestFile const&);
    ProfileVersion const& GetECDbProfileVersion() const { return m_ecdb.GetECDbProfileVersion(); }
    void AssertProfileVersion() const;
    };

//=======================================================================================
//! Provides helper methods for testing certain areas of an iModel file in the compatibility tests
// @bsiclass
//=======================================================================================    
struct TestIModel final : TestDb
    {
    struct Iterable final
        {
        public:
            struct const_iterator final
                {
                using iterator_category=std::forward_iterator_tag;
                using value_type=std::unique_ptr<TestIModel>;
                using difference_type=std::ptrdiff_t;
                using pointer=std::unique_ptr<TestIModel>*;
                using reference=std::unique_ptr<TestIModel>&;

                private:
                    TestFile const& m_testFile;
                    std::vector<DgnDb::OpenParams>::const_iterator m_paramsIt;
                public:
                    const_iterator(TestFile const& testFile, std::vector<DgnDb::OpenParams>::const_iterator paramsIt) : m_testFile(testFile), m_paramsIt(paramsIt) {}
                    std::unique_ptr<TestIModel> operator* () const { return std::make_unique<TestIModel>(m_testFile, *m_paramsIt); }

                    const_iterator& operator++ () { m_paramsIt++; return *this; }
                    bool operator== (const_iterator const& rhs) const { return m_paramsIt == rhs.m_paramsIt; }
                    bool operator!= (const_iterator const& rhs) const { return !(*this == rhs); }
                };

        private:
            TestFile const& m_testFile;
            std::vector<DgnDb::OpenParams> m_params;

        public:
            Iterable(TestFile const& testFile, std::vector<DgnDb::OpenParams> const& params) : m_testFile(testFile), m_params(params) {}
            const_iterator begin() const { return const_iterator(m_testFile, m_params.begin()); }
            const_iterator end() const { return const_iterator(m_testFile, m_params.end()); }
        };

    private:
        DgnDbPtr m_dgndb = nullptr;
        DgnDb::OpenParams m_openParams;

        ECDbR _GetDb() const override { return GetDgnDb(); }
        DbResult _Open() override;
        void _Close() override
            {
            if (m_dgndb != nullptr && m_dgndb->IsDbOpen())
                m_dgndb->CloseDb();

            m_dgndb = nullptr;
            }

        ECDb::OpenParams const& _GetOpenParams() const override { return m_openParams; }
        Utf8String _OpenParamsToString() const override;

    public:
        TestIModel(TestFile const& testFile, DgnDb::OpenParams const& openParams = DgnDb::OpenParams(DgnDb::OpenMode::Readonly)) : TestDb(testFile), m_openParams(openParams) {}
        ~TestIModel() { _Close(); }

        SchemaUpgradeOptions const& GetSchemaUpgradeOptions() { return m_openParams.GetSchemaUpgradeOptions(); }

        DgnDbR GetDgnDb() const { BeAssert(m_dgndb != nullptr); return *m_dgndb; }
        static Iterable GetPermutationsFor(TestFile const&);
        DgnDbProfileVersion  GetDgnDbProfileVersion() const { return m_dgndb->GetProfileVersion(); }
        void AssertProfileVersion() const;
    };
