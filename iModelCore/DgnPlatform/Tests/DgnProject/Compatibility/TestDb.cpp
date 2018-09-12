/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/TestDb.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "TestDb.h"

USING_NAMESPACE_BENTLEY_EC

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    07/18
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool TestDb::VersionSupportsFeature(ProfileVersion const& ecdbVersion, ECDbFeature feature)
    {
    switch (feature)
        {
            case ECDbFeature::PersistedECVersions:
            case ECDbFeature::NamedEnumerators:
            case ECDbFeature::UnitsAndFormats:
                return ecdbVersion >= ProfileVersion(4, 0, 0, 2);

            default:
                BeAssert(false && "Unhandled ECDbFeature enum value");
                return false;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    07/18
//+---------------+---------------+---------------+---------------+---------------+------
ProfileVersion TestDb::GetECDbInitialVersion() const
    {
    ProfileVersion version(0, 0, 0, 0);
    Utf8String versionStr;
    if (BE_SQLITE_ROW != GetDb().QueryProperty(versionStr, PropertySpec("InitialSchemaVersion", "ec_Db")))
        return version;

    version.FromJson(versionStr.c_str());
    return version;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
JsonValue TestDb::ExecuteECSqlSelect(Utf8CP ecsql) const
    {
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(GetDb(), ecsql))
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
SchemaVersion TestDb::GetSchemaVersion(Utf8CP schemaName) const
    {
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(GetDb(), "SELECT VersionMajor,VersionWrite,VersionMinor FROM meta.ECSchemaDef WHERE Name=?") ||
        ECSqlStatus::Success != stmt.BindText(1, schemaName, IECSqlBinder::MakeCopy::No) || stmt.Step() != BE_SQLITE_ROW)
        {
        return SchemaVersion();
        }

    SchemaVersion version((uint16_t) stmt.GetValueInt(0), (uint16_t) stmt.GetValueInt(1), (uint16_t) stmt.GetValueInt(2));

    //verify that version is the same if fetched via ECObjects
    ECSchemaCP schema = GetDb().Schemas().GetSchema(schemaName, false);
    EXPECT_TRUE(schema != nullptr) << schemaName << " | " << GetDescription();
    EXPECT_EQ(version, SchemaVersion(*schema)) << "Version of " << schemaName << " retrieved from ECObjects differs from when retrieved with ECSQL" << " | " << GetDescription();
    return version;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
BeVersion TestDb::GetOriginalECXmlVersion(Utf8CP schemaName) const
    {
    if (SupportsFeature(ECDbFeature::PersistedECVersions))
        {
        JsonValue rows = ExecuteECSqlSelect(Utf8PrintfString("SELECT OriginalECXmlVersionMajor major, OriginalECXmlVersionMinor minor FROM meta.ECSchemaDef WHERE Name='%s'", schemaName).c_str());
        if (!rows.m_value.isArray() || rows.m_value.size() != 1)
            return BeVersion();

        JsonValueCR versionJson = rows.m_value[0];
        if (!versionJson.isMember("major"))
            return BeVersion();

        const BeVersion originalXmlVersion = versionJson.isMember("minor") ? BeVersion(versionJson["major"].asInt(), versionJson["minor"].asInt()) : BeVersion(versionJson["major"].asInt(), 0);

        //verify that version is the same if fetched via ECObjects
        ECSchemaCP schema = GetDb().Schemas().GetSchema(schemaName, false);
        EXPECT_TRUE(schema != nullptr) << schemaName << " | " << GetDescription();
        EXPECT_EQ((int) originalXmlVersion.GetMajor(), (int) schema->GetOriginalECXmlVersionMajor()) << "Original ECXml Major Version of " << schemaName << " retrieved from ECObjects differs from when retrieved with ECSQL" << " | " << GetDescription();
        EXPECT_EQ((int) originalXmlVersion.GetMinor(), (int) schema->GetOriginalECXmlVersionMinor()) << "Original ECXml Minor Version of " << schemaName << " retrieved from ECObjects differs from when retrieved with ECSQL" << " | " << GetDescription();
        return originalXmlVersion;
        }

    //if not persisted, return an empty version
    return BeVersion();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
int TestDb::GetSchemaCount() const
    {
    JsonValue rows = ExecuteECSqlSelect("SELECT count(*) cnt FROM meta.ECSchemaDef");
    if (!rows.m_value.isArray() || rows.m_value.size() != 1 || !rows.m_value[0].isMember("cnt"))
        return -1;

    return rows.m_value[0]["cnt"].asInt();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
JsonValue TestDb::GetSchemaItemCounts(Utf8CP schemaName) const
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

    if (SupportsFeature(ECDbFeature::UnitsAndFormats))
        {
        JsonValue unitCount = ExecuteECSqlSelect(Utf8PrintfString("SELECT count(*) cnt FROM meta.UnitDef i JOIN meta.ECSchemaDef s ON i.Schema.Id=s.ECInstanceId WHERE s.Name='%s'", schemaName).c_str());
        if (unitCount.m_value.size() != 1)
            return JsonValue();

        count = unitCount.m_value[0]["cnt"].asInt();
        if (count != 0)
            counts.m_value["unitcount"] = count;

        JsonValue formatCount = ExecuteECSqlSelect(Utf8PrintfString("SELECT count(*) cnt FROM meta.FormatDef i JOIN meta.ECSchemaDef s ON i.Schema.Id=s.ECInstanceId WHERE s.Name='%s'", schemaName).c_str());
        if (formatCount.m_value.size() != 1)
            return JsonValue();

        count = formatCount.m_value[0]["cnt"].asInt();
        if (count != 0)
            counts.m_value["formatcount"] = count;

        JsonValue unitSystemCount = ExecuteECSqlSelect(Utf8PrintfString("SELECT count(*) cnt FROM meta.UnitSystemDef i JOIN meta.ECSchemaDef s ON i.Schema.Id=s.ECInstanceId WHERE s.Name='%s'", schemaName).c_str());
        if (unitSystemCount.m_value.size() != 1)
            return JsonValue();

        count = unitSystemCount.m_value[0]["cnt"].asInt();
        if (count != 0)
            counts.m_value["unitsystemcount"] = count;

        JsonValue phenCount = ExecuteECSqlSelect(Utf8PrintfString("SELECT count(*) cnt FROM meta.PhenomenonDef i JOIN meta.ECSchemaDef s ON i.Schema.Id=s.ECInstanceId WHERE s.Name='%s'", schemaName).c_str());
        if (phenCount.m_value.size() != 1)
            return JsonValue();

        count = phenCount.m_value[0]["cnt"].asInt();
        if (count != 0)
            counts.m_value["phenomenoncount"] = count;
        }

    return counts;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    08/18
//+---------------+---------------+---------------+---------------+---------------+------
void TestDb::AssertEnum(ECEnumerationCR ecEnum, Utf8CP schemaName, Utf8CP enumName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, ECN::PrimitiveType expectedType, bool expectedIsStrict, std::vector<std::tuple<Utf8CP, ECN::ECValue, Utf8CP>> const& expectedEnumerators) const
    {
    Utf8String assertMessage(ecEnum.GetFullName());
    assertMessage.append(" | ").append(GetDescription());

    EXPECT_EQ((int) expectedType, (int) ecEnum.GetType()) << assertMessage;
    EXPECT_EQ(expectedIsStrict, ecEnum.GetIsStrict()) << assertMessage;
    EXPECT_EQ(expectedDisplayLabel != nullptr, ecEnum.GetIsDisplayLabelDefined()) << assertMessage;
    if (expectedDisplayLabel != nullptr)
        EXPECT_STREQ(expectedDisplayLabel, ecEnum.GetDisplayLabel().c_str()) << assertMessage;

    if (expectedDescription != nullptr)
        EXPECT_STREQ(expectedDescription, ecEnum.GetDescription().c_str()) << assertMessage;
    else
        EXPECT_TRUE(ecEnum.GetDescription().empty()) << assertMessage;

    EXPECT_EQ(expectedEnumerators.size(), ecEnum.GetEnumeratorCount()) << assertMessage;

    size_t i = 0;
    for (ECEnumeratorCP enumerator : ecEnum.GetEnumerators())
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
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
void TestDb::AssertEnum(Utf8CP schemaName, Utf8CP enumName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, ECN::PrimitiveType expectedType, bool expectedIsStrict, std::vector<std::tuple<Utf8CP, ECN::ECValue, Utf8CP>> const& expectedEnumerators) const
    {
    Utf8String assertMessage(schemaName);
    assertMessage.append(".").append(enumName).append(" | ").append(GetDescription());

    // 1) Via schema manager
    ECEnumerationCP ecEnum = GetDb().Schemas().GetEnumeration(schemaName, enumName);
    ASSERT_TRUE(ecEnum != nullptr) << assertMessage;
    AssertEnum(*ecEnum, schemaName, enumName, expectedDisplayLabel, expectedDescription, expectedType, expectedIsStrict, expectedEnumerators);

    // if the file has EC3.2 enums, don't run the ECSQL verification as the persisted enumerator json differs from the expected
    if (SupportsFeature(ECDbFeature::NamedEnumerators))
        return;

    // 2) Via ECSQL
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetDb(), "SELECT e.ECInstanceId,e.DisplayLabel,e.Description,e.IsStrict,e.Type,e.EnumValues FROM meta.ECEnumerationDef e JOIN meta.ECSchemaDef s ON e.Schema.Id=s.ECInstanceId WHERE s.Name=? AND e.Name=?")) << assertMessage;
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
    int i = 0;
    for (IECSqlValue const& enumValue : enumValues.GetArrayIterable())
        {
        std::tuple<Utf8CP, ECValue, Utf8CP> const& expectedEnumValue = expectedEnumerators[i];
        Utf8CP expectedName = std::get<0>(expectedEnumValue);
        ECValueCR expectedValue = std::get<1>(expectedEnumValue);
        Utf8CP expectedDisplayLabel = std::get<2>(expectedEnumValue);

        if (GetDb().GetECDbProfileVersion() >= ProfileVersion(4, 0, 0, 2))
            {
            //enumerator names were added in 4.0.0.2
            EXPECT_STREQ(expectedName, enumValue["Name"].GetText()) << assertMessage << " Enumerator: " << i;
            }

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
// @bsimethod                                     Krischan.Eberle                    08/18
//+---------------+---------------+---------------+---------------+---------------+------
void TestDb::AssertKindOfQuantity(KindOfQuantityCR koq, Utf8CP expectedSchemaName, Utf8CP expectedKoqName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, Utf8CP expectedPersistenceUnit, JsonValue const& expectedPresentationFormats, double expectedRelError) const
    {
    Utf8String assertMessage(koq.GetFullName());
    assertMessage.append(" | ").append(GetDescription());

    EXPECT_STREQ(expectedSchemaName, koq.GetSchema().GetName().c_str()) << assertMessage;
    EXPECT_STREQ(expectedKoqName, koq.GetName().c_str()) << assertMessage;

    EXPECT_EQ(expectedDisplayLabel != nullptr, koq.GetIsDisplayLabelDefined()) << assertMessage;
    if (expectedDisplayLabel != nullptr)
        EXPECT_STREQ(expectedDisplayLabel, koq.GetInvariantDisplayLabel().c_str()) << assertMessage;

    if (expectedDescription != nullptr)
        EXPECT_STREQ(expectedDescription, koq.GetDescription().c_str()) << assertMessage;
    else
        EXPECT_TRUE(koq.GetDescription().empty()) << assertMessage;

    EXPECT_STREQ(expectedPersistenceUnit, koq.GetPersistenceUnit()->GetQualifiedName(koq.GetSchema()).c_str()) << assertMessage;
    if (expectedPresentationFormats.m_value.isNull() || expectedPresentationFormats.m_value.empty())
        EXPECT_TRUE(koq.GetPresentationFormats().empty()) << assertMessage;
    else
        {
        EXPECT_EQ((int) expectedPresentationFormats.m_value.size(), (int) koq.GetPresentationFormats().size()) << assertMessage;
        size_t i = 0;
        for (NamedFormat const& presFormat : koq.GetPresentationFormats())
            {
            EXPECT_STREQ(expectedPresentationFormats.m_value[(Json::ArrayIndex) i].asCString(), presFormat.GetQualifiedFormatString(koq.GetSchema()).c_str()) << "Presentation Format #" << i << " | " << assertMessage;
            i++;
            }
        }

    EXPECT_DOUBLE_EQ(expectedRelError, koq.GetRelativeError()) << assertMessage;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
void TestDb::AssertKindOfQuantity(Utf8CP schemaName, Utf8CP koqName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, Utf8CP expectedPersistenceUnit, JsonValue const& expectedPresentationFormats, double expectedRelError) const
    {
    Utf8String assertMessage(schemaName);
    assertMessage.append(".").append(koqName).append(" | ").append(GetDescription());

    // 1) Via schema manager
    KindOfQuantityCP koq = GetDb().Schemas().GetKindOfQuantity(schemaName, koqName);
    ASSERT_TRUE(koq != nullptr) << assertMessage;

    AssertKindOfQuantity(*koq, schemaName, koqName, expectedDisplayLabel, expectedDescription, expectedPersistenceUnit, expectedPresentationFormats, expectedRelError);

    //If the file wasn't upgraded yet, the persisted KOQs differ from the expected. They only get upgraded
    //in memory on the fly. So don't run the ECSQL based KOQ verification in that case
    if (!SupportsFeature(ECDbFeature::UnitsAndFormats))
        return;

    // 2) Via ECSQL
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetDb(), "SELECT koq.ECInstanceId,koq.DisplayLabel,koq.Description,koq.PersistenceUnit,koq.PresentationUnits,koq.RelativeError FROM meta.KindOfQuantityDef koq JOIN meta.ECSchemaDef s ON koq.Schema.Id=s.ECInstanceId WHERE s.Name=? AND koq.Name=?")) << assertMessage;
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, schemaName, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | " << assertMessage;
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, koqName, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | " << assertMessage;
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
void TestDb::AssertUnit(Utf8CP schemaName, Utf8CP unitName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, Utf8CP expectedDefinition, 
                            Nullable<double> expectedNumerator, Nullable<double> expectedDenominator, Nullable<double> expectedOffset, 
                            QualifiedName const& expectedUnitSystem, QualifiedName const& expectedPhenomenon, bool expectedIsConstant, QualifiedName const& expectedInvertingUnit) const
    {
    Utf8String assertMessage(schemaName);
    assertMessage.append(".").append(unitName).append(" | ").append(GetDescription());

    // 1) Via schema manager
    ECUnitCP unit = GetDb().Schemas().GetUnit(schemaName, unitName);
    ASSERT_TRUE(unit != nullptr) << assertMessage;

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
    if (!SupportsFeature(ECDbFeature::UnitsAndFormats))
        return;

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetDb(), "SELECT u.ECInstanceId,u.DisplayLabel,u.Description,u.Definition,u.Numerator,u.Denominator,u.[Offset],u.UnitSystem.Id,u.Phenomenon.Id,u.IsConstant,u.InvertingUnit.Id FROM meta.UnitDef u JOIN meta.ECSchemaDef s ON u.Schema.Id=s.ECInstanceId WHERE s.Name=? AND u.Name=?")) << assertMessage;
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, schemaName, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | " << assertMessage;
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, unitName, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | " << assertMessage;
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
void TestDb::AssertFormat(Utf8CP schemaName, Utf8CP formatName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, JsonValue const& expectedNumericSpec, JsonValue const& expectedCompSpec) const
    {
    Utf8String assertMessage(schemaName);
    assertMessage.append(".").append(formatName).append(" | ").append(GetDescription());

    // 1) Via schema manager
    ECFormatCP format = GetDb().Schemas().GetFormat(schemaName, formatName);
    ASSERT_TRUE(format != nullptr) << assertMessage;

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

    if (!SupportsFeature(ECDbFeature::UnitsAndFormats))
        return;

    // 2) Via ECSQL
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetDb(), "SELECT f.ECInstanceId,f.DisplayLabel,f.Description,f.NumericSpec,f.CompositeSpec FROM meta.FormatDef f JOIN meta.ECSchemaDef s ON f.Schema.Id=s.ECInstanceId WHERE s.Name=? AND f.Name=?")) << assertMessage;
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, schemaName, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | " << assertMessage;
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, formatName, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | " << assertMessage;
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | " << assertMessage;

    EXPECT_EQ(format->GetId(), stmt.GetValueId<ECN::FormatId>(0)) << stmt.GetECSql() << " | " << assertMessage;
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
        {
        EXPECT_TRUE(stmt.IsValueNull(4)) << stmt.GetECSql() << " | " << assertMessage;
        return;
        }

    ASSERT_TRUE(expectedCompSpec.m_value.isMember("units")) << stmt.GetECSql() << " | " << assertMessage;
    JsonValue expectedCompositeWithoutUnitsJson(expectedCompSpec.m_value);
    expectedCompositeWithoutUnitsJson.m_value.removeMember("units");
    EXPECT_EQ(expectedCompositeWithoutUnitsJson, JsonValue(stmt.GetValueText(4))) << assertMessage;
    stmt.Finalize();

    Json::Value const& expectedCompositeUnitsJson = expectedCompSpec.m_value["units"];
    ASSERT_TRUE(expectedCompositeUnitsJson.isArray()) << assertMessage;
    ECSqlStatement unitLookupStmt;
    ASSERT_EQ(ECSqlStatus::Success, unitLookupStmt.Prepare(GetDb(), "SELECT Name FROM meta.UnitDef WHERE ECInstanceId=?")) << assertMessage;

    ECSqlStatement compUnitStmt;
    ASSERT_EQ(ECSqlStatus::Success, compUnitStmt.Prepare(GetDb(), "SELECT Label,Unit.Id FROM meta.FormatCompositeUnitDef WHERE Format.Id=? ORDER BY Ordinal")) << assertMessage;
    ASSERT_EQ(ECSqlStatus::Success, compUnitStmt.BindId(1, format->GetId())) << compUnitStmt.GetECSql() << " | " << assertMessage;
    int ordinal = 0;
    while (BE_SQLITE_ROW == compUnitStmt.Step())
        {
        ASSERT_LT(ordinal, (int) expectedCompositeUnitsJson.size()) << assertMessage;
        Json::Value const& expectedCompositeUnitJson = expectedCompositeUnitsJson[(Json::ArrayIndex) ordinal];
        EXPECT_FALSE(compUnitStmt.IsValueNull(1)) << "Composite unit" << ordinal << " | " << assertMessage;
        BeInt64Id unitId = compUnitStmt.GetValueId<BeInt64Id>(1);
        ASSERT_EQ(ECSqlStatus::Success, unitLookupStmt.BindId(1, unitId)) << "Composite unit" << ordinal << " | " << assertMessage;
        ASSERT_EQ(BE_SQLITE_ROW, unitLookupStmt.Step()) << "Composite unit" << ordinal << " | " << assertMessage;
        EXPECT_STREQ(expectedCompositeUnitJson["name"].asCString(), unitLookupStmt.GetValueText(0)) << "Composite unit" << ordinal << " | " << assertMessage;
        unitLookupStmt.Reset();
        unitLookupStmt.ClearBindings();

        if (expectedCompositeUnitJson.isMember("label"))
            EXPECT_STREQ(expectedCompositeUnitJson["label"].asCString(), compUnitStmt.GetValueText(0)) << "Composite unit" << ordinal << " | " << assertMessage;
        else
            EXPECT_TRUE(compUnitStmt.IsValueNull(0)) << "Composite unit" << ordinal << " | " << assertMessage;

        ordinal++;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
void TestDb::AssertUnitSystem(Utf8CP schemaName, Utf8CP unitsystemName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription) const
    {
    Utf8String assertMessage(schemaName);
    assertMessage.append(".").append(unitsystemName).append(" | ").append(GetDescription());

    // 1) Via schema manager
    UnitSystemCP sys = GetDb().Schemas().GetUnitSystem(schemaName, unitsystemName);
    ASSERT_TRUE(sys != nullptr) << assertMessage;

    EXPECT_EQ(expectedDisplayLabel != nullptr, sys->GetIsDisplayLabelDefined()) << assertMessage;
    if (expectedDisplayLabel != nullptr)
        EXPECT_STREQ(expectedDisplayLabel, sys->GetInvariantDisplayLabel().c_str()) << assertMessage;

    if (expectedDescription != nullptr)
        EXPECT_STREQ(expectedDescription, sys->GetDescription().c_str()) << assertMessage;
    else
        EXPECT_TRUE(sys->GetDescription().empty()) << assertMessage;

    if (!SupportsFeature(ECDbFeature::UnitsAndFormats))
        return;

    // 2) Via ECSQL
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetDb(), "SELECT us.ECInstanceId,us.DisplayLabel,us.Description FROM meta.UnitSystemDef us JOIN meta.ECSchemaDef s ON us.Schema.Id=s.ECInstanceId WHERE s.Name=? AND us.Name=?")) << assertMessage;
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, schemaName, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | " << assertMessage;
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, unitsystemName, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | " << assertMessage;
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
void TestDb::AssertPhenomenon(Utf8CP schemaName, Utf8CP phenName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, Utf8CP expectedDefinition) const
    {
    Utf8String assertMessage(schemaName);
    assertMessage.append(".").append(phenName).append(" | ").append(GetDescription());

    // 1) Via schema manager
    PhenomenonCP phen = GetDb().Schemas().GetPhenomenon(schemaName, phenName);
    ASSERT_TRUE(phen != nullptr) << assertMessage;

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

    if (!SupportsFeature(ECDbFeature::UnitsAndFormats))
        return;

    // 2) Via ECSQL
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetDb(), "SELECT p.ECInstanceId,p.DisplayLabel,p.Description,p.Definition FROM meta.PhenomenonDef p JOIN meta.ECSchemaDef s ON p.Schema.Id=s.ECInstanceId WHERE s.Name=? AND p.Name=?")) << assertMessage;
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, schemaName, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | " << assertMessage;
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, phenName, IECSqlBinder::MakeCopy::No)) << stmt.GetECSql() << " | " << assertMessage;
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
void TestDb::AssertLoadSchemas() const
    {
    //1) via schema manager
    for (ECSchemaCP schema : GetDb().Schemas().GetSchemas(true))
        {
        const bool expectsHasId = SupportsFeature(ECDbFeature::UnitsAndFormats) || (!schema->GetName().EqualsIAscii("Units") && !schema->GetName().EqualsIAscii("Formats"));
        EXPECT_EQ(expectsHasId, schema->HasId()) << schema->GetFullSchemaName() << " | " << GetDescription();

        EXPECT_FALSE(schema->GetName().empty()) << schema->GetFullSchemaName() << " | " << GetDescription();
        for (ECClassCP ecClass : schema->GetClasses())
            {
            EXPECT_TRUE(ecClass->HasId()) << ecClass->GetFullName() << " | " << GetDescription();
            EXPECT_FALSE(ecClass->GetName().empty()) << ecClass->GetFullName() << " | " << GetDescription();
            for (ECPropertyCP prop : ecClass->GetProperties())
                {
                EXPECT_TRUE(prop->HasId()) << ecClass->GetFullName() << "." << prop->GetName() << " | " << GetDescription();
                EXPECT_FALSE(prop->GetName().empty()) << ecClass->GetFullName() << "." << prop->GetName() << " | " << GetDescription();
                }
            }

        for (ECEnumerationCP ecEnum : schema->GetEnumerations())
            {
            EXPECT_TRUE(ecEnum->HasId()) << ecEnum->GetFullName() << " | " << GetDescription();
            EXPECT_FALSE(ecEnum->GetName().empty()) << ecEnum->GetFullName() << " | " << GetDescription();
            }

        for (KindOfQuantityCP koq : schema->GetKindOfQuantities())
            {
            EXPECT_TRUE(koq->HasId()) << koq->GetFullName() << " | " << GetDescription();
            EXPECT_FALSE(koq->GetName().empty()) << koq->GetFullName() << " | " << GetDescription();
            }

        for (PropertyCategoryCP cat : schema->GetPropertyCategories())
            {
            EXPECT_TRUE(cat->HasId()) << cat->GetFullName() << " | " << GetDescription();
            EXPECT_FALSE(cat->GetName().empty()) << cat->GetFullName() << " | " << GetDescription();
            }


        for (ECUnitCP unit : schema->GetUnits())
            {
            EXPECT_EQ(expectsHasId, unit->HasId()) << unit->GetFullName() << " | " << GetDescription();
            EXPECT_FALSE(unit->GetName().empty()) << unit->GetFullName() << " | " << GetDescription();
            }

        for (ECFormatCP format : schema->GetFormats())
            {
            EXPECT_EQ(expectsHasId, format->HasId()) << format->GetFullName() << " | " << GetDescription();
            EXPECT_FALSE(format->GetName().empty()) << format->GetFullName() << " | " << GetDescription();
            }

        for (UnitSystemCP unitSystem : schema->GetUnitSystems())
            {
            EXPECT_EQ(expectsHasId, unitSystem->HasId()) << unitSystem->GetFullName() << " | " << GetDescription();
            EXPECT_FALSE(unitSystem->GetName().empty()) << unitSystem->GetFullName() << " | " << GetDescription();
            }

        for (PhenomenonCP phenomenon : schema->GetPhenomena())
            {
            EXPECT_EQ(expectsHasId, phenomenon->HasId()) << phenomenon->GetFullName() << " | " << GetDescription();
            EXPECT_FALSE(phenomenon->GetName().empty()) << phenomenon->GetFullName() << " | " << GetDescription();
            }
        }

    GetDb().ClearECDbCache();

    // 2) via ECSQL
    EXPECT_FALSE(ExecuteECSqlSelect("SELECT * FROM meta.ECSchemaDef").m_value.empty()) << GetDescription();
    EXPECT_FALSE(ExecuteECSqlSelect("SELECT * FROM meta.ECClassDef").m_value.empty()) << GetDescription();
    EXPECT_FALSE(ExecuteECSqlSelect("SELECT * FROM meta.ECEnumerationDef").m_value.empty()) << GetDescription();
    EXPECT_FALSE(ExecuteECSqlSelect("SELECT * FROM meta.ECPropertyDef").m_value.empty()) << GetDescription();
    // Not all test files contain KOQs or PropertyCategories, so just test that the result is a JSON array
    EXPECT_TRUE(ExecuteECSqlSelect("SELECT * FROM meta.KindOfQuantityDef").m_value.isArray()) << GetDescription();
    EXPECT_TRUE(ExecuteECSqlSelect("SELECT * FROM meta.PropertyCategoryDef").m_value.isArray()) << GetDescription();

    if (SupportsFeature(ECDbFeature::UnitsAndFormats))
        {
        EXPECT_TRUE(ExecuteECSqlSelect("SELECT * FROM meta.UnitDef").m_value.isArray()) << GetDescription();
        EXPECT_TRUE(ExecuteECSqlSelect("SELECT * FROM meta.FormatDef").m_value.isArray()) << GetDescription();
        EXPECT_TRUE(ExecuteECSqlSelect("SELECT * FROM meta.UnitSystemDef").m_value.isArray()) << GetDescription();
        EXPECT_TRUE(ExecuteECSqlSelect("SELECT * FROM meta.PhenomenonDef").m_value.isArray()) << GetDescription();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
DbResult TestDb::Open()
    {
    if (BeFileNameStatus::Success != m_testFile.CloneSeedToOutput())
        return BE_SQLITE_ERROR;

    return _Open();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String TestDb::GetDescription() const
    {
    Utf8String openModeStr = _OpenParamsToString();

    if (m_age == nullptr)
        return Utf8PrintfString("Open mode: %s | %s", openModeStr.c_str(), GetTestFile().ToString().c_str());

    Utf8CP ageStr = nullptr;
    switch (m_age.Value())
        {
            case ProfileState::Age::Older:
                ageStr = "older";
                break;
            case ProfileState::Age::UpToDate:
                ageStr = "up-to-date";
                break;
            case ProfileState::Age::Newer:
                ageStr = "newer";
                break;
            default:
                BeAssert(false && "Unhandled ProfileState::Age enum value");
                return Utf8String("Error: Unhandled ProfileState::Age enum value");
        }

    return Utf8PrintfString("Open mode: %s | Age: %s | %s", openModeStr.c_str(), ageStr, GetTestFile().ToString().c_str());
    }

//***************************** TestECDb ********************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    07/18
//+---------------+---------------+---------------+---------------+---------------+------
DbResult TestECDb::_Open()
    {
    DbResult stat = m_ecdb.OpenBeSQLiteDb(m_testFile.GetPath(), m_openParams);
    if (BE_SQLITE_OK != stat)
        return stat;

    const int compareVersions = m_ecdb.GetECDbProfileVersion().CompareTo(ECDb::CurrentECDbProfileVersion());
    if (compareVersions == 0)
        m_age = ProfileState::Age::UpToDate;
    else if (compareVersions < 0)
        m_age = ProfileState::Age::Older;
    else
        m_age = ProfileState::Age::Newer;

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
//static
TestECDb::Iterable TestECDb::GetPermutationsFor(TestFile const& testFile)
    {
    std::vector<ECDb::OpenParams> testParams {ECDb::OpenParams(ECDb::OpenMode::Readonly), ECDb::OpenParams(ECDb::OpenMode::ReadWrite)};
    if (testFile.GetAge() == ProfileState::Age::Older)
        testParams.push_back(ECDb::OpenParams(ECDb::OpenMode::ReadWrite, ECDb::ProfileUpgradeOptions::Upgrade));

    return Iterable(testFile, testParams);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
void TestECDb::AssertProfileVersion() const
    {
    ASSERT_TRUE(m_ecdb.IsDbOpen()) << "AssertProfileVersion must be called on open file";
    switch (GetAge())
        {
            case ProfileState::Age::UpToDate:
                EXPECT_TRUE(m_ecdb.CheckProfileVersion().IsUpToDate()) << GetDescription();
                EXPECT_EQ(ECDbProfile::Get().GetExpectedVersion(), m_ecdb.GetECDbProfileVersion()) << GetDescription();
                break;
            case ProfileState::Age::Newer:
                EXPECT_TRUE(m_ecdb.CheckProfileVersion().IsNewer()) << GetDescription();
                EXPECT_LT(ECDbProfile::Get().GetExpectedVersion(), m_ecdb.GetECDbProfileVersion()) << GetDescription();
                break;
            case ProfileState::Age::Older:
                EXPECT_TRUE(m_ecdb.CheckProfileVersion().IsOlder()) << GetDescription();
                EXPECT_GT(ECDbProfile::Get().GetExpectedVersion(), m_ecdb.GetECDbProfileVersion()) << GetDescription();
                break;
            default:
                FAIL() << "Unhandled ProfileState::Age enum value";
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    09/18
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String TestECDb::_OpenParamsToString() const
    {
    if (GetOpenParams().m_openMode == Db::OpenMode::Readonly)
        return "read-only";

    if (GetOpenParams().m_profileUpgradeOptions == Db::ProfileUpgradeOptions::Upgrade)
        return "read-write with profile upgrade";

    return "read-write w/o profile upgrade";
    }

//***************************** TestIModel ********************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    07/18
//+---------------+---------------+---------------+---------------+---------------+------
DbResult TestIModel::_Open()
    {
    DbResult stat = BE_SQLITE_OK;
    m_dgndb = DgnDb::OpenDgnDb(&stat, m_testFile.GetPath(), m_openParams);
    if (BE_SQLITE_OK != stat)
        return stat;

    const int compareVersions = m_dgndb->GetProfileVersion().CompareTo(ProfileVersion(DgnDbProfileValues::DGNDB_CURRENT_VERSION_Major, DgnDbProfileValues::DGNDB_CURRENT_VERSION_Minor, DgnDbProfileValues::DGNDB_CURRENT_VERSION_Sub1, DgnDbProfileValues::DGNDB_CURRENT_VERSION_Sub2));
    if (compareVersions == 0)
        m_age = ProfileState::Age::UpToDate;
    else if (compareVersions < 0)
        m_age = ProfileState::Age::Older;
    else
        m_age = ProfileState::Age::Newer;

    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
//static
TestIModel::Iterable TestIModel::GetPermutationsFor(TestFile const& testFile)
    {
    std::vector<DgnDb::OpenParams> testParams {DgnDb::OpenParams(DgnDb::OpenMode::Readonly), DgnDb::OpenParams(ECDb::OpenMode::ReadWrite), DgnDb::OpenParams(ECDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, Dgn::SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade)};
    if (testFile.GetAge() == ProfileState::Age::Older)
        {
        DgnDb::OpenParams params(DgnDb::OpenMode::ReadWrite);
        params.SetProfileUpgradeOptions(DgnDb::ProfileUpgradeOptions::Upgrade);
        testParams.push_back(params);

        params = DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade));
        params.SetProfileUpgradeOptions(DgnDb::ProfileUpgradeOptions::Upgrade);
        testParams.push_back(params);
        }

    return Iterable(testFile, testParams);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
void TestIModel::AssertProfileVersion() const
    {
    ASSERT_TRUE(m_dgndb != nullptr && m_dgndb->IsDbOpen()) << "AssertProfileVersion must be called on open file";
    switch (GetAge())
        {
            case ProfileState::Age::UpToDate:
                EXPECT_TRUE(m_dgndb->CheckProfileVersion().IsUpToDate()) << GetDescription();
                EXPECT_EQ(DgnDbProfile::Get().GetExpectedVersion(), m_dgndb->GetProfileVersion()) << GetDescription();
                break;
            case ProfileState::Age::Newer:
                EXPECT_TRUE(m_dgndb->CheckProfileVersion().IsNewer()) << GetDescription();
                EXPECT_LT(DgnDbProfile::Get().GetExpectedVersion(), m_dgndb->GetProfileVersion()) << GetDescription();
                break;
            case ProfileState::Age::Older:
                EXPECT_TRUE(m_dgndb->CheckProfileVersion().IsOlder()) << GetDescription();
                EXPECT_GT(DgnDbProfile::Get().GetExpectedVersion(), m_dgndb->GetProfileVersion()) << GetDescription();
                break;
            default:
                FAIL() << "Unhandled ProfileState::Age enum value";
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    09/18
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String TestIModel::_OpenParamsToString() const
    {
    if (m_openParams.m_openMode == Db::OpenMode::Readonly)
        return "read-only";

    Utf8String str("read-write");
    if (m_openParams.m_profileUpgradeOptions == Db::ProfileUpgradeOptions::Upgrade)
        str.append(", with profile upgrade");

    if (m_openParams.GetSchemaUpgradeOptions().GetDomainUpgradeOptions() == SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade)
        str.append(", with domain schema upgrade");

    return str;
    }