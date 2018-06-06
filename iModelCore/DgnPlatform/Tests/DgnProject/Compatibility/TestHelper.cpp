/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/TestHelper.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "TestHelper.h"

USING_NAMESPACE_BENTLEY_EC


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
//static
JsonValue TestHelper::ExecuteECSqlSelect(ECDb const& db, Utf8CP ecsql)
    {
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(db, ecsql))
        return JsonValue();

    JsonValue val(Json::arrayValue);
    JsonECSqlSelectAdapter adapter(stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsNumber));
    while (BE_SQLITE_ROW == stmt.Step())
        {
        Json::Value row;
        if (SUCCESS != adapter.GetRow(row))
            return JsonValue();

        val.m_value.append(row);
        }

    return val;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
//static
SchemaVersion TestHelper::GetSchemaVersion(ECDb const& db, Utf8CP schemaName)
    {
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(db, "SELECT VersionMajor,VersionWrite,VersionMinor FROM meta.ECSchemaDef WHERE Name=?") ||
        ECSqlStatus::Success != stmt.BindText(1, schemaName, IECSqlBinder::MakeCopy::No) || stmt.Step() != BE_SQLITE_ROW)
        {
        return SchemaVersion();
        }

    SchemaVersion version((uint16_t) stmt.GetValueInt(0), (uint16_t) stmt.GetValueInt(1), (uint16_t) stmt.GetValueInt(2));

    //verify that version is the same if fetched via ECObjects
    ECSchemaCP schema = db.Schemas().GetSchema(schemaName, false);
    EXPECT_TRUE(schema != nullptr) << schemaName;
    EXPECT_EQ(version, SchemaVersion(*schema)) << "Version of " << schemaName << " retrieved from ECObjects differs from when retrieved with ECSQL";
    return version;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
//static
void TestHelper::AssertEnum(ECDb const& db, Utf8CP schemaName, Utf8CP enumName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, ECN::PrimitiveType expectedType, bool expectedIsStrict, std::vector<std::pair<ECN::ECValue, Utf8CP>> const& expectedEnumerators)
    {
    // 1) Via schema manager
    ECEnumerationCP ecEnum = db.Schemas().GetEnumeration(schemaName, enumName);
    ASSERT_TRUE(ecEnum != nullptr) << schemaName << "." << enumName;
    Utf8String assertMessage(schemaName);
    assertMessage.append(".").append(enumName);

    EXPECT_EQ((int) expectedType, (int) ecEnum->GetType()) << assertMessage;
    EXPECT_EQ(expectedIsStrict, ecEnum->GetIsStrict()) << assertMessage;
    EXPECT_EQ(expectedDisplayLabel != nullptr, ecEnum->GetIsDisplayLabelDefined()) << assertMessage;
    if (expectedDisplayLabel != nullptr)
        EXPECT_STREQ(expectedDisplayLabel, ecEnum->GetDisplayLabel().c_str()) << assertMessage;

    if (expectedDescription != nullptr)
        EXPECT_STREQ(expectedDescription, ecEnum->GetDescription().c_str()) << assertMessage;
    else
        EXPECT_TRUE(ecEnum->GetDescription().empty()) << assertMessage;

    EXPECT_EQ(expectedEnumerators.size(), ecEnum->GetEnumeratorCount()) << assertMessage;

    size_t i = 0;
    for (ECEnumeratorCP enumerator : ecEnum->GetEnumerators())
        {
        std::pair<ECValue, Utf8CP> const& expectedEnumValue = expectedEnumerators[i];
        Utf8CP expectedDisplayLabel = expectedEnumValue.second;
        if (expectedDisplayLabel == nullptr)
            EXPECT_FALSE(enumerator->GetIsDisplayLabelDefined()) << assertMessage << " Enumerator: " << i;
        else
            EXPECT_STRCASEEQ(expectedDisplayLabel, enumerator->GetInvariantDisplayLabel().c_str());

        ECValueCR expectedValue = expectedEnumValue.first;
        if (expectedType == ECN::PRIMITIVETYPE_Integer)
            {
            ASSERT_TRUE(expectedValue.IsInteger()) << assertMessage << " Enumerator: " << i;
            EXPECT_EQ(expectedValue.GetInteger(), enumerator->GetInteger()) << assertMessage << " Enumerator: " << i;
            }
        else
            {
            ASSERT_TRUE(expectedValue.IsString()) << assertMessage << " Enumerator: " << i;
            EXPECT_STREQ(expectedValue.GetUtf8CP(), enumerator->GetString().c_str()) << assertMessage << " Enumerator: " << i;
            }

        i++;
        }

    // 2) Via ECSQL
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "SELECT e.ECInstanceId,e.DisplayLabel,e.Description,e.IsStrict,e.Type,e.EnumValues FROM meta.ECEnumerationDef e JOIN meta.ECSchemaDef s ON e.Schema.Id=s.ECInstanceId WHERE s.Name=? AND e.Name=?")) << assertMessage;
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, schemaName, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | " << assertMessage;
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, enumName, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | " << assertMessage;
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | " << assertMessage;

    EXPECT_EQ(ecEnum->GetId(), stmt.GetValueId<ECEnumerationId>(0)) << stmt.GetECSql() << " | " << assertMessage;
    if (expectedDisplayLabel != nullptr)
        EXPECT_STREQ(expectedDisplayLabel, stmt.GetValueText(1)) << stmt.GetECSql() << " | " << assertMessage;
    else
        EXPECT_TRUE(stmt.IsValueNull(1)) << stmt.GetECSql() << " | " << assertMessage;

    if (expectedDescription != nullptr)
        EXPECT_STREQ(expectedDescription, stmt.GetValueText(2)) << stmt.GetECSql() << " | " << assertMessage;
    else
        EXPECT_TRUE(stmt.IsValueNull(2)) << stmt.GetECSql() << " | " << assertMessage;

    EXPECT_EQ(expectedIsStrict, stmt.GetValueBoolean(3)) << stmt.GetECSql() << " | " << assertMessage;
    EXPECT_EQ((int) expectedType, stmt.GetValueInt(4)) << stmt.GetECSql() << " | " << assertMessage;

    IECSqlValue const& enumValues = stmt.GetValue(5);
    ASSERT_EQ((int) expectedEnumerators.size(), enumValues.GetArrayLength()) << stmt.GetECSql() << " | " << assertMessage;
    i = 0;
    for (IECSqlValue const& enumValue : enumValues.GetArrayIterable())
        {
        std::pair<ECValue, Utf8CP> const& expectedEnumValue = expectedEnumerators[i];
        ECValueCR expectedValue = expectedEnumValue.first;
        Utf8CP expectedDisplayLabel = expectedEnumValue.second;
        if (expectedDisplayLabel == nullptr)
            EXPECT_TRUE(enumValue["DisplayLabel"].IsNull()) << assertMessage << " Enumerator: " << i;
        else
            EXPECT_STREQ(expectedDisplayLabel, enumValue["DisplayLabel"].GetText()) << assertMessage << " Enumerator: " << i;

        if (expectedType == PRIMITIVETYPE_Integer)
            {
            EXPECT_TRUE(enumValue["StringValue"].IsNull()) << assertMessage << " Enumerator: " << i;
            ASSERT_TRUE(expectedValue.IsInteger()) << assertMessage << " Enumerator: " << i;
            EXPECT_EQ(expectedValue.GetInteger(), enumValue["IntValue"].GetInt()) << assertMessage << " Enumerator: " << i;
            }
        else
            {
            EXPECT_TRUE(enumValue["IntValue"].IsNull()) << assertMessage << " Enumerator: " << i;
            ASSERT_TRUE(expectedValue.IsString()) << assertMessage << " Enumerator: " << i;
            EXPECT_STREQ(expectedValue.GetUtf8CP(), enumValue["StringValue"].GetText()) << assertMessage << " Enumerator: " << i;
            }
        i++;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
//static
void TestHelper::AssertKindOfQuantity(ECDb const& db, Utf8CP schemaName, Utf8CP koqName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, Utf8CP expectedPersistenceUnit, JsonValue const& expectedPresentationUnits, double expectedRelError)
    {
    // 1) Via schema manager
    KindOfQuantityCP koq = db.Schemas().GetKindOfQuantity(schemaName, koqName);
    ASSERT_TRUE(koq != nullptr) << schemaName << "." << koqName;
    Utf8String assertMessage(schemaName);
    assertMessage.append(".").append(koqName);

    EXPECT_EQ(expectedDisplayLabel != nullptr, koq->GetIsDisplayLabelDefined()) << assertMessage;
    if (expectedDisplayLabel != nullptr)
        EXPECT_STREQ(expectedDisplayLabel, koq->GetInvariantDisplayLabel().c_str()) << assertMessage;

    if (expectedDescription != nullptr)
        EXPECT_STREQ(expectedDescription, koq->GetDescription().c_str()) << assertMessage;
    else
        EXPECT_TRUE(koq->GetDescription().empty()) << assertMessage;

    EXPECT_STREQ(expectedPersistenceUnit, koq->GetPersistenceUnit().ToText(false).c_str()) << assertMessage;
    if (expectedPresentationUnits.m_value.isNull() || expectedPresentationUnits.m_value.empty())
        EXPECT_TRUE(koq->GetPresentationUnitList().empty()) << assertMessage;
    else
        EXPECT_EQ(expectedPresentationUnits, JsonValue(koq->GetPresentationsJson(false))) << assertMessage;

    EXPECT_DOUBLE_EQ(expectedRelError, koq->GetRelativeError()) << assertMessage;

    // 2) Via ECSQL
    ECSqlStatement stmt;
    ASSERT_EQ(BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(db, "SELECT koq.ECInstanceId,koq.DisplayLabel,koq.Description,koq.PersistenceUnit,koq.PresentationUnits,koq.RelativeError FROM meta.KindOfQuantityDef koq JOIN meta.ECSchemaDef s ON koq.Schema.Id=s.ECInstanceId WHERE s.Name=? AND koq.Name=?")) << assertMessage;
    ASSERT_EQ(BeSQLite::EC::ECSqlStatus::Success, stmt.BindText(1, schemaName, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | " << assertMessage;
    ASSERT_EQ(BeSQLite::EC::ECSqlStatus::Success, stmt.BindText(2, koqName, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | " << assertMessage;
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | " << assertMessage;

    EXPECT_EQ(koq->GetId(), stmt.GetValueId<ECN::ECEnumerationId>(0)) << stmt.GetECSql() << " | " << assertMessage;
    if (expectedDisplayLabel != nullptr)
        EXPECT_STREQ(expectedDisplayLabel, stmt.GetValueText(1)) << stmt.GetECSql() << " | " << assertMessage;
    else
        EXPECT_TRUE(stmt.IsValueNull(1)) << stmt.GetECSql() << " | " << assertMessage;

    if (expectedDescription != nullptr)
        EXPECT_STREQ(expectedDescription, stmt.GetValueText(2)) << stmt.GetECSql() << " | " << assertMessage;
    else
        EXPECT_TRUE(stmt.IsValueNull(2)) << stmt.GetECSql() << " | " << assertMessage;

    EXPECT_STREQ(expectedPersistenceUnit, stmt.GetValueText(3)) << stmt.GetECSql() << " | " << assertMessage;

    if (expectedPresentationUnits.m_value.isNull() || expectedPresentationUnits.m_value.empty())
        EXPECT_TRUE(stmt.IsValueNull(4)) << stmt.GetECSql() << " | " << assertMessage;
    else
        {
        IECSqlValue const& presUnitsVal = stmt.GetValue(4);
        EXPECT_EQ((int) expectedPresentationUnits.m_value.size(), presUnitsVal.GetArrayLength()) << stmt.GetECSql() << " | " << assertMessage;
        size_t i = 0;
        for (IECSqlValue const& presUnitVal : presUnitsVal.GetArrayIterable())
            {
            EXPECT_STREQ(expectedPresentationUnits.m_value[(Json::ArrayIndex) i].asCString(), presUnitVal.GetText()) << "Array index: " << i << " | " << stmt.GetECSql() << " | " << assertMessage;
            i++;
            }
        }

    EXPECT_DOUBLE_EQ(expectedRelError, stmt.GetValueDouble(5)) << stmt.GetECSql() << " | " << assertMessage;
    }

