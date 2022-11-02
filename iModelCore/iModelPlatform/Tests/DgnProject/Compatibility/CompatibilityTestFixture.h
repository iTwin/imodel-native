/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "CompatibilityTests.h"
#include <ECObjects/ECObjectsAPI.h>
#include <Bentley/BeVersion.h>
#include <json/json.h>
#include <ostream>

//=======================================================================================
// @bsiclass
//======================================================================================
struct CompatibilityTestFixture : ::testing::Test
    {
private:
    static bool s_isInitialized;

protected:
    //! Initializes the test environment by setting up the schema read context and search dirs etc.
    //! Gets implicitly called in the Setup of the subclassing test fixture
    static void Initialize();

    void SetUp() override { Initialize(); }
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct SchemaItem final
    {
    private:
        Utf8String m_xmlString;

    public:
        explicit SchemaItem(Utf8StringCR xmlString) : m_xmlString(xmlString) {}
        Utf8StringCR GetXml() const { return m_xmlString; }
    };

//=======================================================================================
//! Base class for implementing the creation of new test files
// @bsiclass
//=======================================================================================
struct TestFileCreator
    {
    protected:
        Utf8String m_fileName;

    private:
        virtual BentleyStatus _Create() = 0;
        virtual BentleyStatus _UpgradeOldFiles() const = 0;
        virtual BentleyStatus _UpgradeSchemas() const { return SUCCESS; }

    protected:
        explicit TestFileCreator(Utf8CP fileName) : m_fileName(fileName) {}

    public:
        virtual ~TestFileCreator() {}
        BentleyStatus Run();

        static ECN::ECSchemaReadContextPtr DeserializeSchemas(ECDbCR, std::vector<SchemaItem> const&, std::vector<BeFileName> const& additionalSearchPaths = {});
        static ECN::ECSchemaReadContextPtr DeserializeSchema(ECDbCR ecdb, SchemaItem const& schema, std::vector<BeFileName> const& additionalSearchPaths = {}) { return DeserializeSchemas(ecdb, {schema}, additionalSearchPaths); }
    };

//=======================================================================================
//! Runs a list of TestFileCreators
// @bsiclass
//=======================================================================================
struct TestFileCreation final
    {
private:
    TestFileCreation() = delete;
    ~TestFileCreation() = delete;

public:
    static BentleyStatus Run(std::vector<std::shared_ptr<TestFileCreator>> const& creators)
        {
        for (std::shared_ptr<TestFileCreator> const& creator : creators)
            {
            if (SUCCESS != creator->Run())
                return ERROR;
            }

        return SUCCESS;
        }
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct QualifiedName final
    {
private:
    Utf8String m_schemaName;
    Utf8String m_name;

public:
    QualifiedName() {}
    QualifiedName(Utf8CP schemaName, Utf8CP name) : m_schemaName(schemaName), m_name(name) {}

    bool operator==(QualifiedName const& rhs) const { return m_schemaName == rhs.m_schemaName && m_name == rhs.m_name; }
    bool operator!=(QualifiedName const& rhs) const { return !(*this == rhs); }

    bool IsValid() const { return !m_schemaName.empty() && !m_name.empty(); }
    Utf8StringCR GetSchemaName() const { return m_schemaName; }
    Utf8StringCR GetName() const { return m_name; }
    };

void PrintTo(QualifiedName const&, std::ostream*);

//=======================================================================================
// Wrapper for JsonCpp that allows for easy use in GTest macros
// @bsiclass
//=======================================================================================
struct JsonValue final
    {
    public:
        Json::Value m_value = Json::Value(Json::nullValue);

        JsonValue() {}
        explicit JsonValue(Json::ValueType type) : m_value(type) {}
        explicit JsonValue(JsonValueCR json) : m_value(json) {}
        explicit JsonValue(Utf8CP json);
        explicit JsonValue(Utf8StringCR json) : JsonValue(json.c_str()) {}

        bool operator==(JsonValue const& rhs) const;
        bool operator!=(JsonValue const& rhs) const { return !(*this == rhs); }
        Json::Value const &Value() const { return m_value; }
        Utf8String ToString() const { return m_value.ToString(); }
    };

void PrintTo(JsonValue const&, std::ostream*);

//=======================================================================================
// Represents an ECSchema version
// @bsiclass
//=======================================================================================
struct SchemaVersion final : BeVersion
    {
public:
    SchemaVersion() : BeVersion() {}
    SchemaVersion(uint16_t versionRead, uint16_t versionWrite, uint16_t versionMinor) : BeVersion(versionRead, versionWrite, versionMinor, 0) {}
    explicit SchemaVersion(ECN::ECSchemaCR schema) : SchemaVersion((uint16_t) schema.GetVersionRead(), (uint16_t) schema.GetVersionWrite(), (uint16_t) schema.GetVersionMinor()) {}
    Utf8String ToString() const { return Utf8PrintfString("%" PRIu16 ".%" PRIu16 ".%" PRIu16, GetMajor(), GetMinor(), GetSub1()); }
    };

void PrintTo(SchemaVersion const&, std::ostream*);

BEGIN_BENTLEY_NAMESPACE

void PrintTo(BeVersion const&, std::ostream*);

END_BENTLEY_NAMESPACE

BEGIN_BENTLEY_SQLITE_NAMESPACE

void PrintTo(ProfileState const&, std::ostream*);

END_BENTLEY_SQLITE_NAMESPACE

//=======================================================================================
//! Disables "Fail On Assertion" for the lifetime of this object.
//! Use this helper classes instead of BeTest::SetFailOnAssert as it automatically resets
//! the FailOnAssert state when the ScopedDisableFailOnAssertion object goes out of scope
// @bsiclass
//=======================================================================================
struct ScopedDisableFailOnAssertion final
    {
    private:
        bool m_isNoop = false;

    public:
        explicit ScopedDisableFailOnAssertion(bool disable = true) : m_isNoop(!BeTest::GetFailOnAssert() || !disable)
            {
            if (!m_isNoop)
                BeTest::SetFailOnAssert(false);
            }

        ~ScopedDisableFailOnAssertion()
            {
            if (!m_isNoop)
                BeTest::SetFailOnAssert(true);
            }
    };

