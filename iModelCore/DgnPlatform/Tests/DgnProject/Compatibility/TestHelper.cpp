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
JsonValue TestHelper::ExecuteECSqlSelect(Utf8CP ecsql) const
    {
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(m_db, ecsql))
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
SchemaVersion TestHelper::GetSchemaVersion(Utf8CP schemaName) const
    {
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(m_db, "SELECT VersionMajor,VersionWrite,VersionMinor FROM meta.ECSchemaDef WHERE Name=?") ||
        ECSqlStatus::Success != stmt.BindText(1, schemaName, IECSqlBinder::MakeCopy::No) || stmt.Step() != BE_SQLITE_ROW)
        {
        return SchemaVersion();
        }

    SchemaVersion version((uint16_t) stmt.GetValueInt(0), (uint16_t) stmt.GetValueInt(1), (uint16_t) stmt.GetValueInt(2));

    //verify that version is the same if fetched via ECObjects
    ECSchemaCP schema = m_db.Schemas().GetSchema(schemaName, false);
    EXPECT_TRUE(schema != nullptr) << schemaName;
    EXPECT_EQ(version, SchemaVersion(*schema)) << "Version of " << schemaName << " retrieved from ECObjects differs from when retrieved with ECSQL";
    return version;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
BeVersion TestHelper::GetOriginalECXmlVersion(Utf8CP schemaName) const
    {
    JsonValue rows = ExecuteECSqlSelect(Utf8PrintfString("SELECT OriginalECXmlVersionMajor major, OriginalECXmlVersionMinor minor FROM meta.ECSchemaDef WHERE Name='%s'", schemaName).c_str());
    if (!rows.m_value.isArray() || rows.m_value.size() != 1)
        return BeVersion();

    JsonValueCR versionJson = rows.m_value[0];
    if (!versionJson.isMember("major"))
        return BeVersion();

    if (versionJson.isMember("minor"))
        return BeVersion(versionJson["major"].asInt(), versionJson["minor"].asInt());

    return BeVersion(versionJson["major"].asInt(), 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
int TestHelper::GetSchemaCount() const
    {
    JsonValue rows = TestHelper::ExecuteECSqlSelect("SELECT count(*) cnt FROM meta.ECSchemaDef");
    if (!rows.m_value.isArray() || rows.m_value.size() != 1 || !rows.m_value[0].isMember("cnt"))
        return -1;

    return rows.m_value[0]["cnt"].asInt();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
JsonValue TestHelper::GetSchemaItemCounts(Utf8CP schemaName) const
    {
    JsonValue counts(Json::objectValue);
    JsonValue classCount = ExecuteECSqlSelect(Utf8PrintfString("SELECT count(*) cnt FROM meta.ECClassDef i JOIN meta.ECSchemaDef s ON i.Schema.Id=s.ECInstanceId WHERE s.Name='%s'", schemaName).c_str());
    if (classCount.m_value.size() != 1)
        return JsonValue();

    int count = classCount.m_value[0]["cnt"].asInt();
    if (count != 0)
        counts.m_value["classcount"] = count;

    JsonValue enumCount = ExecuteECSqlSelect(Utf8PrintfString("SELECT count(*) cnt FROM meta.ECEnumerationDef i JOIN meta.ECSchemaDef s ON i.Schema.Id=s.ECInstanceId WHERE s.Name='%s'", schemaName).c_str());
    if (enumCount.m_value.size() != 1)
        return JsonValue();

    count = enumCount.m_value[0]["cnt"].asInt();
    if (count != 0)
        counts.m_value["enumcount"] = count;

    JsonValue koqCount = ExecuteECSqlSelect(Utf8PrintfString("SELECT count(*) cnt FROM meta.KindOfQuantityDef i JOIN meta.ECSchemaDef s ON i.Schema.Id=s.ECInstanceId WHERE s.Name='%s'", schemaName).c_str());
    if (koqCount.m_value.size() != 1)
        return JsonValue();

    count = koqCount.m_value[0]["cnt"].asInt();
    if (count != 0)
        counts.m_value["koqcount"] = count;

    JsonValue catCount = ExecuteECSqlSelect(Utf8PrintfString("SELECT count(*) cnt FROM meta.PropertyCategoryDef i JOIN meta.ECSchemaDef s ON i.Schema.Id=s.ECInstanceId WHERE s.Name='%s'", schemaName).c_str());
    if (catCount.m_value.size() != 1)
        return JsonValue();

    count = catCount.m_value[0]["cnt"].asInt();
    if (count != 0)
        counts.m_value["propertycategorycount"] = count;

    return counts;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
void TestHelper::AssertEnum(Utf8CP schemaName, Utf8CP enumName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, ECN::PrimitiveType expectedType, bool expectedIsStrict, std::vector<std::pair<ECN::ECValue, Utf8CP>> const& expectedEnumerators) const
    {
    // 1) Via schema manager
    ECEnumerationCP ecEnum = m_db.Schemas().GetEnumeration(schemaName, enumName);
    ASSERT_TRUE(ecEnum != nullptr) << schemaName << "." << enumName;
    Utf8String assertMessage(schemaName);
    assertMessage.append(".").append(enumName).append(" | ").append(m_testFile.ToString());

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
            EXPECT_STREQ(expectedDisplayLabel, enumerator->GetInvariantDisplayLabel().c_str()) << assertMessage << " Enumerator: " << i;

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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_db, "SELECT e.ECInstanceId,e.DisplayLabel,e.Description,e.IsStrict,e.Type,e.EnumValues FROM meta.ECEnumerationDef e JOIN meta.ECSchemaDef s ON e.Schema.Id=s.ECInstanceId WHERE s.Name=? AND e.Name=?")) << assertMessage;
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
void TestHelper::AssertKindOfQuantity(Utf8CP schemaName, Utf8CP koqName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, Utf8CP expectedPersistenceUnit, JsonValue const& expectedPresentationUnits, double expectedRelError) const
    {
    // 1) Via schema manager
    KindOfQuantityCP koq = m_db.Schemas().GetKindOfQuantity(schemaName, koqName);
    ASSERT_TRUE(koq != nullptr) << schemaName << "." << koqName;
    Utf8String assertMessage(schemaName);
    assertMessage.append(".").append(koqName).append(" | ").append(m_testFile.ToString());

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
    ASSERT_EQ(BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(m_db, "SELECT koq.ECInstanceId,koq.DisplayLabel,koq.Description,koq.PersistenceUnit,koq.PresentationUnits,koq.RelativeError FROM meta.KindOfQuantityDef koq JOIN meta.ECSchemaDef s ON koq.Schema.Id=s.ECInstanceId WHERE s.Name=? AND koq.Name=?")) << assertMessage;
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

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
void TestHelper::AssertLoadSchemas() const
    {
    //1) via schema manager
    for (ECSchemaCP schema : m_db.Schemas().GetSchemas(true))
        {
        EXPECT_TRUE(schema->HasId()) << schema->GetFullSchemaName() << " | " << m_testFile.ToString();
        EXPECT_FALSE(schema->GetName().empty()) << schema->GetFullSchemaName() << " | " << m_testFile.ToString();
        for (ECClassCP ecClass : schema->GetClasses()) 
            {
            EXPECT_TRUE(ecClass->HasId()) << ecClass->GetFullName() << " | " << m_testFile.ToString();
            EXPECT_FALSE(ecClass->GetName().empty()) << ecClass->GetFullName() << " | " << m_testFile.ToString();
            for (ECPropertyCP prop : ecClass->GetProperties())
                { 
                EXPECT_TRUE(prop->HasId()) << ecClass->GetFullName() << "." << prop->GetName() << " | " << m_testFile.ToString();
                EXPECT_FALSE(prop->GetName().empty()) << ecClass->GetFullName() << "." << prop->GetName() << " | " << m_testFile.ToString();
                }
            }

        for (ECEnumerationCP ecEnum : schema->GetEnumerations()) 
            {
            EXPECT_TRUE(ecEnum->HasId()) << ecEnum->GetFullName() << " | " << m_testFile.ToString();
            EXPECT_FALSE(ecEnum->GetName().empty()) << ecEnum->GetFullName() << " | " << m_testFile.ToString();
            }

        for (KindOfQuantityCP koq : schema->GetKindOfQuantities()) 
            {
            EXPECT_TRUE(koq->HasId()) << koq->GetFullName() << " | " << m_testFile.ToString();
            EXPECT_FALSE(koq->GetName().empty()) << koq->GetFullName() << " | " << m_testFile.ToString();
            }

        for (PropertyCategoryCP cat : schema->GetPropertyCategories())
            {
            EXPECT_TRUE(cat->HasId()) << cat->GetFullName() << " | " << m_testFile.ToString();
            EXPECT_FALSE(cat->GetName().empty()) << cat->GetFullName() << " | " << m_testFile.ToString();
            }
        }

    m_db.ClearECDbCache();

    // 2) via ECSQL
    EXPECT_FALSE(ExecuteECSqlSelect("SELECT * FROM meta.ECSchemaDef").m_value.empty());
    EXPECT_FALSE(ExecuteECSqlSelect("SELECT * FROM meta.ECClassDef").m_value.empty());
    EXPECT_FALSE(ExecuteECSqlSelect("SELECT * FROM meta.ECEnumerationDef").m_value.empty());
    EXPECT_FALSE(ExecuteECSqlSelect("SELECT * FROM meta.ECPropertyDef").m_value.empty());
    // Not all test files contain KOQs or PropertyCategories, so just test that the result is a JSON array
    EXPECT_TRUE(ExecuteECSqlSelect("SELECT * FROM meta.KindOfQuantityDef").m_value.isArray());
    EXPECT_TRUE(ExecuteECSqlSelect("SELECT * FROM meta.PropertyCategoryDef").m_value.isArray());
    }