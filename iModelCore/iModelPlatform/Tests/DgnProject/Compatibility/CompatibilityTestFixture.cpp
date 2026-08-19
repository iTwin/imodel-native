/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "CompatibilityTestFixture.h"
#include <cmath>
#include <Bentley/BeNumerical.h>

USING_NAMESPACE_BENTLEY_EC

//**************************************************************************************
// CompatibilityTestFixture
//**************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool CompatibilityTestFixture::s_isInitialized = false;

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
void CompatibilityTestFixture::Initialize()
    {
    if (!s_isInitialized)
        {
        //establish standard schema search paths (they are in the application dir)
        BeFileName applicationSchemaDir;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(applicationSchemaDir);

        BeFileName temporaryDir;
        BeTest::GetHost().GetOutputRoot(temporaryDir);

        DgnDb::Initialize(temporaryDir, &applicationSchemaDir);
        srand((uint32_t) (BeTimeUtilities::QueryMillisecondsCounter() & 0xFFFFFFFF));

        s_isInitialized = true;
        }
    }

//**************************************************************************************
// TestFileCreator
//**************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECN::ECSchemaReadContextPtr TestFileCreator::DeserializeSchemas(ECDbCR ecdb, std::vector<SchemaItem> const& schemas, std::vector<BeFileName> const& additionalSearchPaths)
    {
    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(ecdb.GetSchemaLocater());
    for (BeFileNameCR searchPath : additionalSearchPaths)
        {
        context->AddSchemaPath(searchPath.c_str());
        }

    for (SchemaItem const& schemaItem : schemas)
        {
        ScopedDisableFailOnAssertion disableFailOnAssert;
        ECSchemaPtr schema = nullptr;
        if (SchemaReadStatus::Success != ECSchema::ReadFromXmlString(schema, schemaItem.GetXml().c_str(), *context))
            {
            LOG.errorv("Failed to deserialize schema '%s'", schemaItem.GetXml().c_str());
            return nullptr;
            }
        }

    return context;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TestFileCreator::Run()
    {
    if (SUCCESS != _Create())
        return ERROR;

    LOG.infov("Created new test file '%s'.", m_fileName.c_str());

    if (SUCCESS != _UpgradeOldFiles())
        return ERROR;

    return _UpgradeSchemas();
    }

//**************************************************************************************
// JsonValue
//**************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
JsonValue::JsonValue(Utf8CP json)
    {
    // BeJsDocument::Parse asserts on nullptr, whereas Json::Reader::Parse silently absorbed it
    // through its Utf8String parameter.
    if (Utf8String::IsNullOrEmpty(json))
        {
        ADD_FAILURE() << "Empty JSON string";
        return;
        }

    m_value.Parse(json);
    EXPECT_FALSE(m_value.hasParseError()) << json;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
// NOTE: this mirrors the original JsonCpp implementation as closely as the BeJs API allows.
// BeJs has no isInt/isUInt/isIntegral/isConvertibleTo - only isNumeric() - because RapidJson does
// not preserve JsonCpp's signed/unsigned integral subtype distinction. So whole-number values are
// compared exactly as int64 and only genuinely fractional values use the numeric tolerance. Note
// also that bools are NOT numeric in BeJs (JsonCpp counted them as integral), hence the separate
// isBool() branch first. Object comparison is now key-order-independent, which is required because
// RapidJson preserves insertion order where JsonCpp sorted keys alphabetically.
static bool jsonValuesEqual(BeJsConst lhs, BeJsConst rhs)
    {
    if (lhs.isNull())
        return rhs.isNull();

    if (lhs.isArray())
        {
        if (!rhs.isArray() || lhs.size() != rhs.size())
            return false;

        return false == lhs.ForEachArrayMember([&](BeJsConst::ArrayIndex i, BeJsConst member)
            {
            return !jsonValuesEqual(member, rhs[i]);
            });
        }

    if (lhs.isObject())
        {
        if (!rhs.isObject() || lhs.size() != rhs.size())
            return false;

        return false == lhs.ForEachProperty([&](Utf8CP memberName, BeJsConst member)
            {
            return !rhs.hasMember(memberName) || !jsonValuesEqual(member, rhs[memberName]);
            });
        }

    if (lhs.isBool())
        return rhs.isBool() && lhs.asBool() == rhs.asBool();

    if (lhs.isNumeric())
        {
        if (!rhs.isNumeric())
            return false;

        double l = lhs.asDouble();
        double r = rhs.asDouble();
        if (l == std::trunc(l) && r == std::trunc(r))
            return lhs.asInt64() == rhs.asInt64();

        return fabs(l - r) <= BeNumerical::ComputeComparisonTolerance(l, r);
        }

    if (lhs.isString())
        return rhs.isString() && strcmp(lhs.asCString(), rhs.asCString()) == 0;

    BeAssert(false && "Unhandled JSON value type. This method needs to be adjusted");
    return false;
    }

bool JsonValue::operator==(JsonValue const& rhs) const { return jsonValuesEqual(m_value, rhs.m_value); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(QualifiedName const& name, std::ostream* os) 
    { 
    if (!name.IsValid())
        *os << "<invalid QualifiedName>";
    else
        *os << name.GetSchemaName() << "." << name.GetName(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(JsonValue const& json, std::ostream* os) { *os << json.ToString(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(SchemaVersion const& ver, std::ostream* os) { *os << ver.ToString(); }

BEGIN_BENTLEY_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(BeVersion const& ver, std::ostream* os) { *os << ver.ToString(); }

END_BENTLEY_NAMESPACE

BEGIN_BENTLEY_SQLITE_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(ProfileState const& state, std::ostream* os) 
    { 
    if (state.IsError())
        {
        *os << "Error";
        return;
        }

    switch (state.GetAge())
        {
            case ProfileState::Age::Newer:
                *os << "Newer";
                break;
            case ProfileState::Age::Older:
                *os << "Older";
                break;
            case ProfileState::Age::UpToDate:
                *os << "Up-to-date";
                break;
            default:
                BeAssert(false);
                *os << "<programmer error>";
                return;
        }

    *os << " | ";

    switch (state.GetCanOpen())
        {
            case ProfileState::CanOpen::No:
                *os << "CanOpen::No";
                break;
            case ProfileState::CanOpen::Readonly:
                *os << "CanOpen::Readonly";
                break;
            case ProfileState::CanOpen::Readwrite:
                *os << "CanOpen::Readwrite";
                break;
            default:
                BeAssert(false);
                *os << "<programmer error>";
                return;
        }

    *os << " | Upgradable: " << state.IsUpgradable();
    }

END_BENTLEY_SQLITE_NAMESPACE