/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/CompatibilityTestFixture.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "CompatibilityTestFixture.h"
#include <Bentley/BeNumerical.h>

USING_NAMESPACE_BENTLEY_EC

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECN::ECSchemaReadContextPtr TestFileCreator::DeserializeSchemas(ECDbCR ecdb, std::vector<SchemaItem> const& schemas)
    {
    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(ecdb.GetSchemaLocater());
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

//**************************************************************************************
// JsonValue
//**************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle     12/17
//---------------------------------------------------------------------------------------
JsonValue::JsonValue(Utf8CP json)
    {
    if (!Json::Reader::Parse(json, m_value))
        m_value = Json::Value(Json::nullValue);
    }

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