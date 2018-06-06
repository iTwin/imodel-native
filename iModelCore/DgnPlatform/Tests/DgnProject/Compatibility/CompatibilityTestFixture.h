/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/CompatibilityTestFixture.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "../TestFixture/DgnDbTestFixtures.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include <ECObjects/ECObjectsAPI.h>
#include <Bentley/BeVersion.h>
#include <Logging/bentleylogging.h>
#include <json/json.h>
#include <ostream>

#define LOG (*NativeLogging::LoggingManager::GetLogger (L"Compatibility"))

//=======================================================================================
// @bsiclass                                                 Affan.Khan          03/2018
//======================================================================================
struct CompatibilityTestFixture : ::testing::Test {};

//=======================================================================================    
// @bsiclass                                   Krischan.Eberle                  06/18
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
// @bsiclass                                   Krischan.Eberle                  06/18
//=======================================================================================    
struct CompatibilityTestHelper final
    {
    private:
        CompatibilityTestHelper() = delete;
        ~CompatibilityTestHelper() = delete;
    public:
        static ECN::ECSchemaReadContextPtr DeserializeSchemas(ECDbCR, std::vector<SchemaItem> const&);
        static ECN::ECSchemaReadContextPtr DeserializeSchema(ECDbCR ecdb, SchemaItem const& schema) { return DeserializeSchemas(ecdb, {schema}); }
    };

//=======================================================================================
//! Base class for implementing the creation of new test files
// @bsiclass                                   Krischan.Eberle                  06/18
//=======================================================================================    
struct TestFileCreator
    {
    private:
        virtual BentleyStatus _Create() = 0;
    protected:
        TestFileCreator() {}
    public:
        virtual ~TestFileCreator() {}
        BentleyStatus Create() { return _Create(); }
    };

//=======================================================================================
//! Runs a list of TestFileCreators
// @bsiclass                                   Krischan.Eberle                  06/18
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
            if (SUCCESS != creator->Create())
                return ERROR;
            }

        return SUCCESS;
        }
    };


//=======================================================================================
// Wrapper for JsonCpp that allows for easy use in GTest macros
// @bsiclass                                                 Krischan.Eberle     06/18
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

        Utf8String ToString() const { return m_value.ToString(); }
    };

void PrintTo(JsonValue const&, std::ostream*);

//=======================================================================================
// Represents an ECSchema version
// @bsiclass                                                 Krischan.Eberle     06/18
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

//=======================================================================================
//! Disables "Fail On Assertion" for the lifetime of this object.
//! Use this helper classes instead of BeTest::SetFailOnAssert as it automatically resets
//! the FailOnAssert state when the ScopedDisableFailOnAssertion object goes out of scope
// @bsiclass                                               Krischan.Eberle     06/2018
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

