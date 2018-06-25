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
void TestHelper::AssertEnum(Utf8CP schemaName, Utf8CP enumName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, ECN::PrimitiveType expectedType, bool expectedIsStrict, std::vector<std::tuple<Utf8CP, ECValue, Utf8CP>> const& expectedEnumerators) const
    {
    // 1) Via schema manager
    ECEnumerationCP ecEnum = m_db.Schemas().GetEnumeration(schemaName, enumName);
    ASSERT_TRUE(ecEnum != nullptr) << schemaName << "." << enumName << " | " << m_testFile.ToString();
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
void TestHelper::AssertKindOfQuantity(Utf8CP schemaName, Utf8CP koqName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, Utf8CP expectedPersistenceUnit, JsonValue const& expectedPresentationFormats, double expectedRelError) const
    {
    // 1) Via schema manager
    KindOfQuantityCP koq = m_db.Schemas().GetKindOfQuantity(schemaName, koqName);
    ASSERT_TRUE(koq != nullptr) << schemaName << "." << koqName << " | " << m_testFile.ToString();
    Utf8String assertMessage(schemaName);
    assertMessage.append(".").append(koqName).append(" | ").append(m_testFile.ToString());

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
            EXPECT_STREQ(expectedPresentationFormats.m_value[(Json::ArrayIndex) i].asCString(), presFormat.GetQualifiedFormatString(koq->GetSchema()).c_str()) << "Presentation Format #" << i << " | " << assertMessage;
            i++;
            }
        }

    EXPECT_DOUBLE_EQ(expectedRelError, koq->GetRelativeError()) << assertMessage;

    // 2) Via ECSQL
    ECSqlStatement stmt;
    ASSERT_EQ(BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(m_db, "SELECT koq.ECInstanceId,koq.DisplayLabel,koq.Description,koq.PersistenceUnit,koq.PresentationUnits,koq.RelativeError FROM meta.KindOfQuantityDef koq JOIN meta.ECSchemaDef s ON koq.Schema.Id=s.ECInstanceId WHERE s.Name=? AND koq.Name=?")) << assertMessage;
    ASSERT_EQ(BeSQLite::EC::ECSqlStatus::Success, stmt.BindText(1, schemaName, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | " << assertMessage;
    ASSERT_EQ(BeSQLite::EC::ECSqlStatus::Success, stmt.BindText(2, koqName, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | " << assertMessage;
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | " << assertMessage;

    EXPECT_EQ(koq->GetId(), stmt.GetValueId<ECN::KindOfQuantityId>(0)) << stmt.GetECSql() << " | " << assertMessage;
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
void TestHelper::AssertUnit(Utf8CP schemaName, Utf8CP unitName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, Utf8CP expectedDefinition, 
                            Nullable<double> expectedNumerator, Nullable<double> expectedDenominator, Nullable<double> expectedOffset, 
                            QualifiedName const& expectedUnitSystem, QualifiedName const& expectedPhenomenon, bool expectedIsConstant, QualifiedName const& expectedInvertingUnit) const
    {
    // 1) Via schema manager
    ECUnitCP unit = m_db.Schemas().GetUnit(schemaName, unitName);
    ASSERT_TRUE(unit != nullptr) << schemaName << "." << unitName << " | " << m_testFile.ToString();
    Utf8String assertMessage(schemaName);
    assertMessage.append(".").append(unitName).append(" | ").append(m_testFile.ToString());

    EXPECT_EQ(expectedDisplayLabel != nullptr, unit->GetIsDisplayLabelDefined()) << assertMessage;
    if (expectedDisplayLabel != nullptr)
        EXPECT_STREQ(expectedDisplayLabel, unit->GetInvariantDisplayLabel().c_str()) << assertMessage;

    if (expectedDescription != nullptr)
        EXPECT_STREQ(expectedDescription, unit->GetDescription().c_str()) << assertMessage;
    else
        EXPECT_TRUE(unit->GetDescription().empty()) << assertMessage;

    if (Utf8String::IsNullOrEmpty(expectedDefinition))
        EXPECT_FALSE(unit->HasDefinition()) << assertMessage;
    else
        EXPECT_STREQ(expectedDefinition, unit->GetDefinition().c_str()) << assertMessage;

    if (expectedNumerator == nullptr)
        EXPECT_FALSE(unit->HasNumerator()) << assertMessage;
    else
        EXPECT_DOUBLE_EQ(expectedNumerator.Value(), unit->GetNumerator()) << assertMessage;

    if (expectedDenominator == nullptr)
        EXPECT_FALSE(unit->HasDenominator()) << assertMessage;
    else
        EXPECT_DOUBLE_EQ(expectedDenominator.Value(), unit->GetDenominator()) << assertMessage;

    if (expectedOffset == nullptr)
        EXPECT_FALSE(unit->HasOffset()) << assertMessage;
    else
        EXPECT_DOUBLE_EQ(expectedOffset.Value(), unit->GetOffset()) << assertMessage;

    if (!expectedUnitSystem.IsValid())
        EXPECT_FALSE(unit->HasUnitSystem()) << assertMessage;
    else
        {
        EXPECT_STREQ(expectedUnitSystem.GetName().c_str(), unit->GetUnitSystem()->GetName().c_str()) << assertMessage;
        EXPECT_STREQ(expectedUnitSystem.GetSchemaName().c_str(), unit->GetUnitSystem()->GetSchema().GetName().c_str()) << assertMessage;
        }

    EXPECT_STREQ(expectedPhenomenon.GetName().c_str(), unit->GetPhenomenon()->GetName().c_str()) << assertMessage;
    EXPECT_STREQ(expectedPhenomenon.GetSchemaName().c_str(), unit->GetPhenomenon()->GetSchema().GetName().c_str()) << assertMessage;

    EXPECT_EQ(expectedIsConstant, unit->IsConstant()) << assertMessage;

    if (!expectedInvertingUnit.IsValid())
        EXPECT_FALSE(unit->IsInvertedUnit()) << assertMessage;
    else
        {
        EXPECT_STREQ(expectedInvertingUnit.GetName().c_str(), unit->GetInvertingUnit()->GetName().c_str()) << assertMessage;
        EXPECT_STREQ(expectedInvertingUnit.GetSchemaName().c_str(), unit->GetInvertingUnit()->GetSchema().GetName().c_str()) << assertMessage;
        }

    // 2) Via ECSQL
    ECSqlStatement stmt;
    ASSERT_EQ(BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(m_db, "SELECT u.ECInstanceId,u.DisplayLabel,u.Description,u.Definition,u.Numerator,u.Denominator,u.[Offset],u.UnitSystem.Id,u.Phenomenon.Id,u.IsConstant,u.InvertingUnit.Id FROM meta.UnitDef u JOIN meta.ECSchemaDef s ON u.Schema.Id=s.ECInstanceId WHERE s.Name=? AND u.Name=?")) << assertMessage;
    ASSERT_EQ(BeSQLite::EC::ECSqlStatus::Success, stmt.BindText(1, schemaName, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | " << assertMessage;
    ASSERT_EQ(BeSQLite::EC::ECSqlStatus::Success, stmt.BindText(2, unitName, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | " << assertMessage;
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | " << assertMessage;

    EXPECT_EQ(unit->GetId(), stmt.GetValueId<ECN::UnitId>(0)) << stmt.GetECSql() << " | " << assertMessage;
    if (expectedDisplayLabel != nullptr)
        EXPECT_STREQ(expectedDisplayLabel, stmt.GetValueText(1)) << stmt.GetECSql() << " | " << assertMessage;
    else
        EXPECT_TRUE(stmt.IsValueNull(1)) << stmt.GetECSql() << " | " << assertMessage;

    if (expectedDescription != nullptr)
        EXPECT_STREQ(expectedDescription, stmt.GetValueText(2)) << stmt.GetECSql() << " | " << assertMessage;
    else
        EXPECT_TRUE(stmt.IsValueNull(2)) << stmt.GetECSql() << " | " << assertMessage;

    if (Utf8String::IsNullOrEmpty(expectedDefinition))
        EXPECT_TRUE(stmt.IsValueNull(3)) << stmt.GetECSql() << " | " << assertMessage;
    else
        EXPECT_STREQ(expectedDefinition, stmt.GetValueText(3)) << stmt.GetECSql() << " | " << assertMessage;

    if (expectedNumerator.IsNull())
        EXPECT_TRUE(stmt.IsValueNull(4)) << stmt.GetECSql() << " | " << assertMessage;
    else
        EXPECT_DOUBLE_EQ(expectedNumerator.Value(), stmt.GetValueDouble(4)) << stmt.GetECSql() << " | " << assertMessage;

    if (expectedDenominator.IsNull())
        EXPECT_TRUE(stmt.IsValueNull(5)) << stmt.GetECSql() << " | " << assertMessage;
    else
        EXPECT_DOUBLE_EQ(expectedDenominator.Value(), stmt.GetValueDouble(5)) << stmt.GetECSql() << " | " << assertMessage;

    if (expectedOffset.IsNull())
        EXPECT_TRUE(stmt.IsValueNull(6)) << stmt.GetECSql() << " | " << assertMessage;
    else
        EXPECT_DOUBLE_EQ(expectedOffset.Value(), stmt.GetValueDouble(6)) << stmt.GetECSql() << " | " << assertMessage;

    if (!expectedUnitSystem.IsValid())
        EXPECT_TRUE(stmt.IsValueNull(7)) << stmt.GetECSql() << " | " << assertMessage;
    else
        {
        UnitSystemId actualUnitSystemId = stmt.GetValueId<UnitSystemId>(7);
        EXPECT_EQ(JsonValue(Utf8PrintfString("[{\"schema\":\"%s\", \"unitsystem\":\"%s\"}]", expectedUnitSystem.GetSchemaName().c_str(), expectedUnitSystem.GetName().c_str())), ExecuteECSqlSelect(Utf8PrintfString("SELECT s.Name schema, us.Name unitsystem FROM meta.UnitSystemDef us JOIN meta.ECSchemaDef s ON s.ECInstanceId=us.Schema.Id WHERE us.ECInstanceId=%s", actualUnitSystemId.ToHexStr().c_str()).c_str())) << stmt.GetECSql() << " | " << assertMessage;
        }

    PhenomenonId actualPhenId = stmt.GetValueId<PhenomenonId>(8);
    EXPECT_EQ(JsonValue(Utf8PrintfString("[{\"schema\":\"%s\", \"phen\":\"%s\"}]", expectedPhenomenon.GetSchemaName().c_str(), expectedPhenomenon.GetName().c_str())),
                ExecuteECSqlSelect(Utf8PrintfString("SELECT s.Name schema, ph.Name phen FROM meta.PhenomenonDef ph JOIN meta.ECSchemaDef s ON s.ECInstanceId=ph.Schema.Id WHERE ph.ECInstanceId=%s", actualPhenId.ToHexStr().c_str()).c_str())) << stmt.GetECSql() << " | " << assertMessage;

    EXPECT_EQ(expectedIsConstant, stmt.GetValueBoolean(9)) << stmt.GetECSql() << " | " << assertMessage;

    if (!expectedInvertingUnit.IsValid())
        EXPECT_TRUE(stmt.IsValueNull(10)) << stmt.GetECSql() << " | " << assertMessage;
    else
        {
        UnitId actualInvertingUnitId = stmt.GetValueId<UnitId>(10);
        EXPECT_EQ(JsonValue(Utf8PrintfString("[{\"schema\":\"%s\", \"invertingunit\":\"%s\"}]", expectedInvertingUnit.GetSchemaName().c_str(), expectedInvertingUnit.GetName().c_str())),
                  ExecuteECSqlSelect(Utf8PrintfString("SELECT s.Name schema, u.Name invertingunit FROM meta.UnitDef u JOIN meta.ECSchemaDef s ON s.ECInstanceId=u.Schema.Id WHERE u.ECInstanceId=%s", actualInvertingUnitId.ToHexStr().c_str()).c_str())) << stmt.GetECSql() << " | " << assertMessage;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
void TestHelper::AssertFormat(Utf8CP schemaName, Utf8CP formatName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, JsonValue const& expectedNumericSpec, JsonValue const& expectedCompSpec) const
    {
    // 1) Via schema manager
    ECFormatCP format = m_db.Schemas().GetFormat(schemaName, formatName);
    ASSERT_TRUE(format != nullptr) << schemaName << "." << formatName << " | " << m_testFile.ToString();
    Utf8String assertMessage(schemaName);
    assertMessage.append(".").append(formatName).append(" | ").append(m_testFile.ToString());

    EXPECT_EQ(expectedDisplayLabel != nullptr, format->GetIsDisplayLabelDefined()) << assertMessage;
    if (expectedDisplayLabel != nullptr)
        EXPECT_STREQ(expectedDisplayLabel, format->GetInvariantDisplayLabel().c_str()) << assertMessage;

    if (expectedDescription != nullptr)
        EXPECT_STREQ(expectedDescription, format->GetDescription().c_str()) << assertMessage;
    else
        EXPECT_TRUE(format->GetDescription().empty()) << assertMessage;

    if (expectedNumericSpec.m_value.isNull())
        EXPECT_FALSE(format->HasNumeric()) << assertMessage;
    else
        {
        Json::Value actualNumericSpec;
        ASSERT_TRUE(format->GetNumericSpec()->ToJson(actualNumericSpec, false));
        EXPECT_EQ(expectedNumericSpec, JsonValue(actualNumericSpec)) << assertMessage;
        }

    if (expectedCompSpec.m_value.isNull())
        EXPECT_FALSE(format->HasComposite()) << assertMessage;
    else
        {
        Json::Value actualCompSpec;
        ASSERT_TRUE(format->GetCompositeSpec()->ToJson(actualCompSpec, true));
        EXPECT_EQ(expectedCompSpec, JsonValue(actualCompSpec)) << assertMessage;
        }

    // 2) Via ECSQL
    ECSqlStatement stmt;
    ASSERT_EQ(BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(m_db, "SELECT f.ECInstanceId,f.DisplayLabel,f.Description,f.NumericSpec,f.CompositeSpec FROM meta.FormatDef f JOIN meta.ECSchemaDef s ON f.Schema.Id=s.ECInstanceId WHERE s.Name=? AND f.Name=?")) << assertMessage;
    ASSERT_EQ(BeSQLite::EC::ECSqlStatus::Success, stmt.BindText(1, schemaName, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | " << assertMessage;
    ASSERT_EQ(BeSQLite::EC::ECSqlStatus::Success, stmt.BindText(2, formatName, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | " << assertMessage;
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | " << assertMessage;

    EXPECT_EQ(format->GetId(), stmt.GetValueId<ECN::UnitSystemId>(0)) << stmt.GetECSql() << " | " << assertMessage;
    if (expectedDisplayLabel != nullptr)
        EXPECT_STREQ(expectedDisplayLabel, stmt.GetValueText(1)) << stmt.GetECSql() << " | " << assertMessage;
    else
        EXPECT_TRUE(stmt.IsValueNull(1)) << stmt.GetECSql() << " | " << assertMessage;

    if (expectedDescription != nullptr)
        EXPECT_STREQ(expectedDescription, stmt.GetValueText(2)) << stmt.GetECSql() << " | " << assertMessage;
    else
        EXPECT_TRUE(stmt.IsValueNull(2)) << stmt.GetECSql() << " | " << assertMessage;

    if (expectedNumericSpec.m_value.isNull())
        EXPECT_TRUE(stmt.IsValueNull(3)) << stmt.GetECSql() << " | " << assertMessage;
    else
        EXPECT_EQ(expectedNumericSpec, JsonValue(stmt.GetValueText(3))) << assertMessage;

    if (expectedCompSpec.m_value.isNull())
        EXPECT_TRUE(stmt.IsValueNull(4)) << stmt.GetECSql() << " | " << assertMessage;
    else
        {
        Formatting::CompositeValueSpecCP expectedCompSpec = format->GetCompositeSpec();
        Json::Value expectedCompSpecWithoutUnitsJson;
        ASSERT_TRUE(expectedCompSpec->ToJson(expectedCompSpecWithoutUnitsJson, false, true)) << assertMessage;
        EXPECT_EQ(JsonValue(expectedCompSpecWithoutUnitsJson), JsonValue(stmt.GetValueText(4))) << assertMessage;

        ECSqlStatement compUnitStmt;
        ASSERT_EQ(BeSQLite::EC::ECSqlStatus::Success, compUnitStmt.Prepare(m_db, "SELECT Label,Unit.Id FROM meta.FormatCompositeUnitDef WHERE Format.Id=? ORDER BY Ordinal")) << assertMessage;
        ASSERT_EQ(BeSQLite::EC::ECSqlStatus::Success, compUnitStmt.BindId(1, format->GetId())) << stmt.GetECSql() << " | " << assertMessage;
        size_t ordinal = 0;
        while (BE_SQLITE_ROW == compUnitStmt.Step())
            {
            switch (ordinal)
                {
                    case 0:
                    {
                    if (expectedCompSpec->HasMajorLabel())
                        EXPECT_STREQ(expectedCompSpec->GetMajorLabel().c_str(), compUnitStmt.GetValueText(0)) << "Composite unit" << ordinal << " | " << assertMessage;
                    else
                        EXPECT_TRUE(compUnitStmt.IsValueNull(0)) << "Composite unit" << ordinal << " | " << assertMessage;

                    ASSERT_TRUE(expectedCompSpec->HasMajorUnit()) << "Composite unit" << ordinal << " | " << assertMessage;
                    EXPECT_EQ(((ECUnitCP) expectedCompSpec->GetMajorUnit())->GetId().GetValue(), compUnitStmt.GetValueId<UnitId>(1).GetValue()) << "Composite unit" << ordinal << " | " << assertMessage;
                    break;
                    }
                    case 1:
                    {
                    if (expectedCompSpec->HasMiddleLabel())
                        EXPECT_STREQ(expectedCompSpec->GetMiddleLabel().c_str(), compUnitStmt.GetValueText(0)) << "Composite unit" << ordinal << " | " << assertMessage;
                    else
                        EXPECT_TRUE(compUnitStmt.IsValueNull(0)) << "Composite unit" << ordinal << " | " << assertMessage;

                    ASSERT_TRUE(expectedCompSpec->HasMiddleUnit()) << "Composite unit" << ordinal << " | " << assertMessage;
                    EXPECT_EQ(((ECUnitCP) expectedCompSpec->GetMiddleUnit())->GetId().GetValue(), compUnitStmt.GetValueId<UnitId>(1).GetValue()) << "Composite unit" << ordinal << " | " << assertMessage;
                    break;
                    }
                    case 2:
                    {
                    if (expectedCompSpec->HasMinorLabel())
                        EXPECT_STREQ(expectedCompSpec->GetMinorLabel().c_str(), compUnitStmt.GetValueText(0)) << "Composite unit" << ordinal << " | " << assertMessage;
                    else
                        EXPECT_TRUE(compUnitStmt.IsValueNull(0)) << "Composite unit" << ordinal << " | " << assertMessage;

                    ASSERT_TRUE(expectedCompSpec->HasMinorUnit()) << "Composite unit" << ordinal << " | " << assertMessage;
                    EXPECT_EQ(((ECUnitCP) expectedCompSpec->GetMinorUnit())->GetId().GetValue(), compUnitStmt.GetValueId<UnitId>(1).GetValue()) << "Composite unit" << ordinal << " | " << assertMessage;
                    break;
                    }
                    case 3:
                    {
                    if (expectedCompSpec->HasSubLabel())
                        EXPECT_STREQ(expectedCompSpec->GetSubLabel().c_str(), compUnitStmt.GetValueText(0)) << "Composite unit" << ordinal << " | " << assertMessage;
                    else
                        EXPECT_TRUE(compUnitStmt.IsValueNull(0)) << "Composite unit" << ordinal << " | " << assertMessage;

                    ASSERT_TRUE(expectedCompSpec->HasSubUnit()) << "Composite unit" << ordinal << " | " << assertMessage;
                    EXPECT_EQ(((ECUnitCP) expectedCompSpec->GetSubUnit())->GetId().GetValue(), compUnitStmt.GetValueId<UnitId>(1).GetValue()) << "Composite unit" << ordinal << " | " << assertMessage;
                    break;
                    }
                    default:
                        FAIL() << "No more than 4 composite units expected. But was: " << ordinal << " | " << assertMessage;
                }

            ordinal++;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
void TestHelper::AssertUnitSystem(Utf8CP schemaName, Utf8CP unitsystemName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription) const
    {
    // 1) Via schema manager
    UnitSystemCP sys = m_db.Schemas().GetUnitSystem(schemaName, unitsystemName);
    ASSERT_TRUE(sys != nullptr) << schemaName << "." << unitsystemName << " | " << m_testFile.ToString();
    Utf8String assertMessage(schemaName);
    assertMessage.append(".").append(unitsystemName).append(" | ").append(m_testFile.ToString());

    EXPECT_EQ(expectedDisplayLabel != nullptr, sys->GetIsDisplayLabelDefined()) << assertMessage;
    if (expectedDisplayLabel != nullptr)
        EXPECT_STREQ(expectedDisplayLabel, sys->GetInvariantDisplayLabel().c_str()) << assertMessage;

    if (expectedDescription != nullptr)
        EXPECT_STREQ(expectedDescription, sys->GetDescription().c_str()) << assertMessage;
    else
        EXPECT_TRUE(sys->GetDescription().empty()) << assertMessage;

    // 2) Via ECSQL
    ECSqlStatement stmt;
    ASSERT_EQ(BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(m_db, "SELECT us.ECInstanceId,us.DisplayLabel,us.Description FROM meta.UnitSystemDef us JOIN meta.ECSchemaDef s ON us.Schema.Id=s.ECInstanceId WHERE s.Name=? AND us.Name=?")) << assertMessage;
    ASSERT_EQ(BeSQLite::EC::ECSqlStatus::Success, stmt.BindText(1, schemaName, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | " << assertMessage;
    ASSERT_EQ(BeSQLite::EC::ECSqlStatus::Success, stmt.BindText(2, unitsystemName, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | " << assertMessage;
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | " << assertMessage;

    EXPECT_EQ(sys->GetId(), stmt.GetValueId<ECN::UnitSystemId>(0)) << stmt.GetECSql() << " | " << assertMessage;
    if (expectedDisplayLabel != nullptr)
        EXPECT_STREQ(expectedDisplayLabel, stmt.GetValueText(1)) << stmt.GetECSql() << " | " << assertMessage;
    else
        EXPECT_TRUE(stmt.IsValueNull(1)) << stmt.GetECSql() << " | " << assertMessage;

    if (expectedDescription != nullptr)
        EXPECT_STREQ(expectedDescription, stmt.GetValueText(2)) << stmt.GetECSql() << " | " << assertMessage;
    else
        EXPECT_TRUE(stmt.IsValueNull(2)) << stmt.GetECSql() << " | " << assertMessage;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
void TestHelper::AssertPhenomenon(Utf8CP schemaName, Utf8CP phenName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, Utf8CP expectedDefinition) const
    {
    // 1) Via schema manager
    PhenomenonCP phen = m_db.Schemas().GetPhenomenon(schemaName, phenName);
    ASSERT_TRUE(phen != nullptr) << schemaName << "." << phenName << " | " << m_testFile.ToString();
    Utf8String assertMessage(schemaName);
    assertMessage.append(".").append(phenName).append(" | ").append(m_testFile.ToString());

    EXPECT_EQ(expectedDisplayLabel != nullptr, phen->GetIsDisplayLabelDefined()) << assertMessage;
    if (expectedDisplayLabel != nullptr)
        EXPECT_STREQ(expectedDisplayLabel, phen->GetInvariantDisplayLabel().c_str()) << assertMessage;

    if (expectedDescription != nullptr)
        EXPECT_STREQ(expectedDescription, phen->GetDescription().c_str()) << assertMessage;
    else
        EXPECT_TRUE(phen->GetDescription().empty()) << assertMessage;

    if (expectedDefinition != nullptr)
        EXPECT_STREQ(expectedDefinition, phen->GetDefinition().c_str()) << assertMessage;
    else
        EXPECT_TRUE(phen->GetDefinition().empty()) << assertMessage;

    // 2) Via ECSQL
    ECSqlStatement stmt;
    ASSERT_EQ(BeSQLite::EC::ECSqlStatus::Success, stmt.Prepare(m_db, "SELECT p.ECInstanceId,p.DisplayLabel,p.Description,p.Definition FROM meta.PhenomenonDef p JOIN meta.ECSchemaDef s ON p.Schema.Id=s.ECInstanceId WHERE s.Name=? AND p.Name=?")) << assertMessage;
    ASSERT_EQ(BeSQLite::EC::ECSqlStatus::Success, stmt.BindText(1, schemaName, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | " << assertMessage;
    ASSERT_EQ(BeSQLite::EC::ECSqlStatus::Success, stmt.BindText(2, phenName, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | " << assertMessage;
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | " << assertMessage;

    EXPECT_EQ(phen->GetId(), stmt.GetValueId<ECN::PhenomenonId>(0)) << stmt.GetECSql() << " | " << assertMessage;
    if (expectedDisplayLabel != nullptr)
        EXPECT_STREQ(expectedDisplayLabel, stmt.GetValueText(1)) << stmt.GetECSql() << " | " << assertMessage;
    else
        EXPECT_TRUE(stmt.IsValueNull(1)) << stmt.GetECSql() << " | " << assertMessage;

    if (expectedDescription != nullptr)
        EXPECT_STREQ(expectedDescription, stmt.GetValueText(2)) << stmt.GetECSql() << " | " << assertMessage;
    else
        EXPECT_TRUE(stmt.IsValueNull(2)) << stmt.GetECSql() << " | " << assertMessage;

    if (expectedDefinition != nullptr)
        EXPECT_STREQ(expectedDefinition, stmt.GetValueText(3)) << stmt.GetECSql() << " | " << assertMessage;
    else
        EXPECT_TRUE(stmt.IsValueNull(3)) << stmt.GetECSql() << " | " << assertMessage;
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