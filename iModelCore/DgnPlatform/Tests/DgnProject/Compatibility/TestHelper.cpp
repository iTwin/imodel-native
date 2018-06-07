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
BeVersion TestHelper::GetOriginalECXmlVersion(ECDbCR ecdb, Utf8CP schemaName)
    {
    JsonValue rows = ExecuteECSqlSelect(ecdb, Utf8PrintfString("SELECT OriginalECXmlVersionMajor major, OriginalECXmlVersionMinor minor FROM meta.ECSchemaDef WHERE Name='%s'", schemaName).c_str());
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
//static
int TestHelper::GetSchemaCount(ECDbCR ecdb)
    {
    JsonValue rows = TestHelper::ExecuteECSqlSelect(ecdb, "SELECT count(*) cnt FROM meta.ECSchemaDef");
    if (!rows.m_value.isArray() || rows.m_value.size() != 1 || !rows.m_value[0].isMember("cnt"))
        return -1;

    return rows.m_value[0]["cnt"].asInt();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
//static
JsonValue TestHelper::GetSchemaItemCounts(ECDbCR ecdb, Utf8CP schemaName)
    {
    JsonValue counts(Json::objectValue);
    JsonValue classCount = ExecuteECSqlSelect(ecdb, Utf8PrintfString("SELECT count(*) cnt FROM meta.ECClassDef i JOIN meta.ECSchemaDef s ON i.Schema.Id=s.ECInstanceId WHERE s.Name='%s'", schemaName).c_str());
    if (classCount.m_value.size() != 1)
        return JsonValue();

    int count = classCount.m_value[0]["cnt"].asInt();
    if (count != 0)
        counts.m_value["classcount"] = count;

    JsonValue enumCount = ExecuteECSqlSelect(ecdb, Utf8PrintfString("SELECT count(*) cnt FROM meta.ECEnumerationDef i JOIN meta.ECSchemaDef s ON i.Schema.Id=s.ECInstanceId WHERE s.Name='%s'", schemaName).c_str());
    if (enumCount.m_value.size() != 1)
        return JsonValue();

    count = enumCount.m_value[0]["cnt"].asInt();
    if (count != 0)
        counts.m_value["enumcount"] = count;

    JsonValue koqCount = ExecuteECSqlSelect(ecdb, Utf8PrintfString("SELECT count(*) cnt FROM meta.KindOfQuantityDef i JOIN meta.ECSchemaDef s ON i.Schema.Id=s.ECInstanceId WHERE s.Name='%s'", schemaName).c_str());
    if (koqCount.m_value.size() != 1)
        return JsonValue();

    count = koqCount.m_value[0]["cnt"].asInt();
    if (count != 0)
        counts.m_value["koqcount"] = count;

    JsonValue catCount = ExecuteECSqlSelect(ecdb, Utf8PrintfString("SELECT count(*) cnt FROM meta.PropertyCategoryDef i JOIN meta.ECSchemaDef s ON i.Schema.Id=s.ECInstanceId WHERE s.Name='%s'", schemaName).c_str());
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
//static
void TestHelper::AssertEnum(ECDb const& db, Utf8CP schemaName, Utf8CP enumName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, ECN::PrimitiveType expectedType, bool expectedIsStrict, std::vector<std::tuple<Utf8CP, ECValue, Utf8CP>> const& expectedEnumerators)
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
        std::tuple<Utf8CP, ECValue, Utf8CP> const& expectedEnumValue = expectedEnumerators[i];
        Utf8CP expectedName = std::get<0>(expectedEnumValue);
        ECValueCR expectedValue = std::get<1>(expectedEnumValue);
        Utf8CP expectedDisplayLabel = std::get<2>(expectedEnumValue);

        EXPECT_STREQ(expectedName, enumerator->GetName().c_str()) << assertMessage << " Enumerator: " << i;

        if (expectedDisplayLabel == nullptr)
            EXPECT_FALSE(enumerator->GetIsDisplayLabelDefined()) << assertMessage << " Enumerator: " << i;
        else
            EXPECT_STREQ(expectedDisplayLabel, enumerator->GetInvariantDisplayLabel().c_str()) << assertMessage << " Enumerator: " << i;

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
        std::tuple<Utf8CP, ECValue, Utf8CP> const& expectedEnumValue = expectedEnumerators[i];
        Utf8CP expectedName = std::get<0>(expectedEnumValue);
        ECValueCR expectedValue = std::get<1>(expectedEnumValue);
        Utf8CP expectedDisplayLabel = std::get<2>(expectedEnumValue);

        EXPECT_STREQ(expectedName, enumValue["Name"].GetText()) << assertMessage << " Enumerator: " << i;

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
void TestHelper::AssertKindOfQuantity(ECDb const& db, Utf8CP schemaName, Utf8CP koqName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, Utf8CP expectedPersistenceUnit, JsonValue const& expectedPresentationFormats, double expectedRelError)
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

    EXPECT_STREQ(expectedPersistenceUnit, koq->GetPersistenceUnit()->GetQualifiedName(koq->GetSchema()).c_str()) << assertMessage;
    if (expectedPresentationFormats.m_value.isNull() || expectedPresentationFormats.m_value.empty())
        EXPECT_TRUE(koq->GetPresentationFormats().empty()) << assertMessage;
    else
        {
        ASSERT_EQ((int) koq->GetPresentationFormats().size(), (int) expectedPresentationFormats.m_value.size()) << assertMessage;
        size_t i = 0;
        for (NamedFormat const& presFormat : koq->GetPresentationFormats())
            {
            EXPECT_STREQ(expectedPresentationFormats.m_value[(Json::ArrayIndex) i].asCString(), presFormat.GetQualifiedName(koq->GetSchema()).c_str()) << "Presentation Format #" << i << " | " << assertMessage;
            i++;
            }
        }

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

    // persisted persistence unit may differ from expected if it was persisted pre EC3.2, so don't verify that via plain ECSQL

    if (expectedPresentationFormats.m_value.isNull() || expectedPresentationFormats.m_value.empty())
        EXPECT_TRUE(stmt.IsValueNull(4)) << stmt.GetECSql() << " | " << assertMessage;
    else
        {
        IECSqlValue const& presUnitsVal = stmt.GetValue(4);
        EXPECT_EQ((int) expectedPresentationFormats.m_value.size(), presUnitsVal.GetArrayLength()) << stmt.GetECSql() << " | " << assertMessage;
        // persisted presentation format may differ from expected if it was persisted pre EC3.2, so don't verify that via plain ECSQL
        }

    EXPECT_DOUBLE_EQ(expectedRelError, stmt.GetValueDouble(5)) << stmt.GetECSql() << " | " << assertMessage;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
//static
void TestHelper::AssertLoadSchemas(ECDb const& db)
    {
    //1) via schema manager
    for (ECSchemaCP schema : db.Schemas().GetSchemas(true))
        {
        EXPECT_TRUE(schema->HasId()) << schema->GetFullSchemaName() << " | " << db.GetDbFileName();
        EXPECT_FALSE(schema->GetName().empty()) << schema->GetFullSchemaName() << " | " << db.GetDbFileName();
        for (ECClassCP ecClass : schema->GetClasses()) 
            {
            EXPECT_TRUE(ecClass->HasId()) << ecClass->GetFullName() << " | " << db.GetDbFileName();
            EXPECT_FALSE(ecClass->GetName().empty()) << ecClass->GetFullName() << " | " << db.GetDbFileName();
            for (ECPropertyCP prop : ecClass->GetProperties())
                { 
                EXPECT_TRUE(prop->HasId()) << ecClass->GetFullName() << "." << prop->GetName() << " | " << db.GetDbFileName();
                EXPECT_FALSE(prop->GetName().empty()) << ecClass->GetFullName() << "." << prop->GetName() << " | " << db.GetDbFileName();
                }
            }

        for (ECEnumerationCP ecEnum : schema->GetEnumerations()) 
            {
            EXPECT_TRUE(ecEnum->HasId()) << ecEnum->GetFullName() << " | " << db.GetDbFileName();
            EXPECT_FALSE(ecEnum->GetName().empty()) << ecEnum->GetFullName() << " | " << db.GetDbFileName();
            }

        for (KindOfQuantityCP koq : schema->GetKindOfQuantities()) 
            {
            EXPECT_TRUE(koq->HasId()) << koq->GetFullName() << " | " << db.GetDbFileName();
            EXPECT_FALSE(koq->GetName().empty()) << koq->GetFullName() << " | " << db.GetDbFileName();
            }

        for (PropertyCategoryCP cat : schema->GetPropertyCategories())
            {
            EXPECT_TRUE(cat->HasId()) << cat->GetFullName() << " | " << db.GetDbFileName();
            EXPECT_FALSE(cat->GetName().empty()) << cat->GetFullName() << " | " << db.GetDbFileName();
            }
        }

    db.ClearECDbCache();

    // 2) via ECSQL
    EXPECT_FALSE(ExecuteECSqlSelect(db, "SELECT * FROM meta.ECSchemaDef").m_value.empty());
    EXPECT_FALSE(ExecuteECSqlSelect(db, "SELECT * FROM meta.ECClassDef").m_value.empty());
    EXPECT_FALSE(ExecuteECSqlSelect(db, "SELECT * FROM meta.ECEnumerationDef").m_value.empty());
    EXPECT_FALSE(ExecuteECSqlSelect(db, "SELECT * FROM meta.ECPropertyDef").m_value.empty());
    // Not all test files contain KOQs or PropertyCategories, so just test that the result is a JSON array
    EXPECT_TRUE(ExecuteECSqlSelect(db, "SELECT * FROM meta.KindOfQuantityDef").m_value.isArray());
    EXPECT_TRUE(ExecuteECSqlSelect(db, "SELECT * FROM meta.PropertyCategoryDef").m_value.isArray());
    }