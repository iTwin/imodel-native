/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "CompatibilityTestFixture.h"
#include <Bentley/BeNumerical.h>

USING_NAMESPACE_BENTLEY_EC

//**************************************************************************************
// CompatibilityTestFixture
//**************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle     07/2018
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool CompatibilityTestFixture::s_isInitialized = false;

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle     07/2018
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
// @bsimethod                                     Krischan.Eberle                    06/18
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
// @bsimethod                                     Krischan.Eberle                    07/18
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
// @bsimethod                                                   Krischan.Eberle     12/17
//---------------------------------------------------------------------------------------
JsonValue::JsonValue(Utf8CP json) { EXPECT_TRUE(Json::Reader::Parse(json, m_value)) << json; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle     10/17
//---------------------------------------------------------------------------------------
bool JsonValue::operator==(JsonValue const& rhs) const
    {
    if (m_value.isNull())
        return rhs.m_value.isNull();

    if (m_value.isArray())
        {
        if (!rhs.m_value.isArray() || m_value.size() != rhs.m_value.size())
            return false;

        for (Json::ArrayIndex i = 0; i < m_value.size(); i++)
            {
            if (JsonValue(m_value[i]) != JsonValue(rhs.m_value[i]))
                return false;
            }

        return true;
        }

    if (m_value.isObject())
        {
        if (!rhs.m_value.isObject())
            return false;

        bvector<Utf8String> lhsMemberNames = m_value.getMemberNames();
        if (lhsMemberNames.size() != rhs.m_value.size())
            return false;

        for (Utf8StringCR memberName : lhsMemberNames)
            {
            if (!rhs.m_value.isMember(memberName))
                return false;

            if (JsonValue(m_value[memberName]) != JsonValue(rhs.m_value[memberName]))
                return false;
            }

        return true;
        }

    if (m_value.isIntegral())
        {
        if (!rhs.m_value.isIntegral())
            return false;

        if (m_value.isBool())
            return rhs.m_value.isBool() && m_value.asBool() == rhs.m_value.asBool();

        if (m_value.isInt())
            return rhs.m_value.isConvertibleTo(Json::intValue) && m_value.asInt64() == rhs.m_value.asInt64();

        if (m_value.isUInt())
            return rhs.m_value.isConvertibleTo(Json::uintValue) && m_value.asUInt64() == rhs.m_value.asUInt64();

        BeAssert(false && "Should not end up here");
        return false;
        }

    if (m_value.isDouble())
        return rhs.m_value.isDouble() && fabs(m_value.asDouble() - rhs.m_value.asDouble()) <= BeNumerical::ComputeComparisonTolerance(m_value.asDouble(), rhs.m_value.asDouble());

    if (m_value.isString())
        return rhs.m_value.isString() && strcmp(m_value.asCString(), rhs.m_value.asCString()) == 0;

    BeAssert(false && "Unhandled JsonCPP value type. This method needs to be adjusted");
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  06/18
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(QualifiedName const& name, std::ostream* os) 
    { 
    if (!name.IsValid())
        *os << "<invalid QualifiedName>";
    else
        *os << name.GetSchemaName() << "." << name.GetName(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(JsonValue const& json, std::ostream* os) { *os << json.ToString(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  06/18
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(SchemaVersion const& ver, std::ostream* os) { *os << ver.ToString(); }

BEGIN_BENTLEY_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  06/18
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(BeVersion const& ver, std::ostream* os) { *os << ver.ToString(); }

END_BENTLEY_NAMESPACE

BEGIN_BENTLEY_SQLITE_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  06/18
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