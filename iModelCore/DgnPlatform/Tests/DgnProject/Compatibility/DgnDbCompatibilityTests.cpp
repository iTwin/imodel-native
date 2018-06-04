/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/DgnDbCompatibilityTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "CompatibilityTestFixture.h"

USING_NAMESPACE_BENTLEY_EC

//---------------------------------------------------------------------------------------
// @bsiclass                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
struct DgnDbCompatibilityTestFixture : CompatibilityTestFixture
    {
protected:
    ScopedDgnHost m_host;

       Profile& Profile() const { return ProfileManager().GetProfile(ProfileType::DgnDb); }

       DgnDbPtr OpenTestFile(DbResult* stat, BeFileNameCR path) { return DgnDb::OpenDgnDb(stat, path, DgnDb::OpenParams(DgnDb::OpenMode::Readonly)); }
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
struct CreateTestDgnDbTestFixture : CompatibilityTestFixture
    {
protected:
    ScopedDgnHost m_host;

private:
    Profile& Profile() const { return ProfileManager().GetProfile(ProfileType::DgnDb); }

protected:
        DgnDbPtr CreateNewTestFile(Utf8CP fileName)
            {
            BeFileName filePath = Profile().GetPathForNewTestFile(fileName);
            BeFileName folder = filePath.GetDirectoryName();
            if (!folder.DoesPathExist())
                {
                if (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(folder))
                    return nullptr;
                }

            CreateDgnDbParams createParam(fileName);
            return DgnDb::CreateDgnDb(nullptr, filePath, createParam);
            }


        BentleyStatus ImportSchema(DgnDbR dgndb, SchemaItem const& schema) const { return ImportSchemas(dgndb, {schema}); }
        BentleyStatus ImportSchemas(DgnDbR dgndb, std::vector<SchemaItem> const& schemas) const
            {
            dgndb.SaveChanges();
            ECSchemaReadContextPtr ctx = DeserializeSchemas(dgndb, schemas);
            if (ctx == nullptr)
                return ERROR;

            if (SchemaStatus::Success == dgndb.ImportSchemas(ctx->GetCache().GetSchemas()))
                {
                dgndb.SaveChanges();
                return SUCCESS;
                }

            dgndb.AbandonChanges();
            return ERROR;
            }

    };

#define TESTFILE_PREEC32ENUMS "preec32enums.bim"
#define TESTFILE_PREEC32KOQS "preec32koqs.bim"

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      05/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CreateTestDgnDbTestFixture, PreEC32Enums)
    {
    DgnDbPtr dgndb = CreateNewTestFile(TESTFILE_PREEC32ENUMS);
    ASSERT_TRUE(dgndb != nullptr);

    // add types of enums which don't exist in the schemas already in the test file
    ASSERT_EQ(SUCCESS, ImportSchema(*dgndb, SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                                    <ECSchema schemaName="PreEC32Enums" alias="preec32" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                                        <ECEnumeration typeName="IntEnum_EnumeratorsWithoutDisplayLabel" displayLabel="Int Enumeration with enumerators without display label" description="Int Enumeration with enumerators without display label" backingTypeName="int" isStrict="true">
                                                            <ECEnumerator value="0"/>
                                                            <ECEnumerator value="1"/>
                                                            <ECEnumerator value="2"/>
                                                        </ECEnumeration>
                                                        <ECEnumeration typeName="StringEnum_EnumeratorsWithDisplayLabel" displayLabel="String Enumeration with enumerators with display label" backingTypeName="string" isStrict="false">
                                                            <ECEnumerator value="On" displayLabel="Turned On"/>
                                                            <ECEnumerator value="Off" displayLabel="Turned Off"/>
                                                        </ECEnumeration>
                                                     </ECSchema>)xml")));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CreateTestDgnDbTestFixture, PreEC32KindOfQuantities)
    {
    DgnDbPtr dgndb = CreateNewTestFile(TESTFILE_PREEC32KOQS);
    ASSERT_TRUE(dgndb != nullptr);

    //test schema for KOQs originates from AECUnits ECSchemas
    ASSERT_EQ(SUCCESS, ImportSchema(*dgndb, SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
<ECSchema schemaName="PreEC32Koqs" alias="preec32koqs" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
 <KindOfQuantity typeName="ANGLE" displayLabel="Angle" persistenceUnit="RAD(DefaultReal)" presentationUnits="ARC_DEG(real2u);ARC_DEG(dms)" relativeError="0.0001"/>
 <KindOfQuantity typeName="AREA" displayLabel="Area" persistenceUnit="SQ.M(DefaultReal)" presentationUnits="SQ.M(real4u);SQ.FT(real4u)" relativeError="0.0001"/>
 <KindOfQuantity typeName="AREA_SMALL" displayLabel="Small Area" persistenceUnit="SQ.M(DefaultReal)" presentationUnits="SQ.MM(real2u);SQ.IN(real4u)" relativeError="0.0001"/>
 <KindOfQuantity typeName="AREA_LARGE" displayLabel="Large Area" persistenceUnit="SQ.M(DefaultReal)" presentationUnits="SQ.KM(real4u);SQ.MILE(real4u)" relativeError="0.01"/>
 <KindOfQuantity typeName="LENGTH" displayLabel="Length" persistenceUnit="M(DefaultReal)" presentationUnits="M(real4u);MM(real4u);FT(fi8);FT(fi16);FT(real4u)" relativeError="0.00001"/>
 <KindOfQuantity typeName="LENGTH_LONG" displayLabel="Long Length" persistenceUnit="M(DefaultReal)" presentationUnits="KM(real4u);M(real4u);FT(fi8);FT(fi16);FT(real4u)" relativeError="0.0001"/>
 <KindOfQuantity typeName="LENGTH_SHORT" displayLabel="Short Length" persistenceUnit="M(DefaultReal)" presentationUnits="MM(real2u);CM(real4u);FT(fi8);IN(fi8);IN(fi16);IN(real4u)" relativeError="0.01"/>
 <KindOfQuantity typeName="VOLUME" displayLabel="Volume" persistenceUnit="CUB.M(DefaultReal)" presentationUnits="CUB.M(real4u);CUB.FT(real4u);CUB.YRD(real4u)" relativeError="0.0001"/>
 <KindOfQuantity typeName="VOLUME_SMALL" displayLabel="Small Volume" persistenceUnit="CUB.M(DefaultReal)" presentationUnits="CUB.MM(real4u);CUB.CM(real4u);CUB.IN(real4u);CUB.FT(real4u)" relativeError="0.0001"/>
 <KindOfQuantity typeName="VOLUME_LARGE" displayLabel="Large Volume" persistenceUnit="CUB.M(DefaultReal)" presentationUnits="CUB.KM(real4u);CUB.M(real4u);CUB.YRD(real4u);CUB.FT(real4u)" relativeError="0.01"/>
 <KindOfQuantity typeName="LIQUID_VOLUME" displayLabel="Liquid Volume" persistenceUnit="CUB.M(DefaultReal)" presentationUnits="LITRE(real4u);GALLON(real4u)" relativeError="0.0001"/>
 <KindOfQuantity typeName="LIQUID_VOLUME_SMALL" displayLabel="Liquid Small Volume" persistenceUnit="CUB.M(DefaultReal)" presentationUnits="LITRE(real4u);GALLON(real4u)" relativeError="0.0001"/>
 <KindOfQuantity typeName="LIQUID_VOLUME_LARGE" displayLabel="Liquid Large Volume" persistenceUnit="CUB.M(DefaultReal)" presentationUnits="THOUSAND_LITRE(real4u);THOUSAND_GALLON(real4u)" relativeError="0.01"/>
 <KindOfQuantity typeName="PROCESS_PIPING_PRESSURE"    displayLabel="Process Piping Pressure" persistenceUnit="PA(DefaultReal)" presentationUnits="PA(real4u);PSIG(real4u);PSI(real4u)" relativeError="0.0001"/>
 <KindOfQuantity typeName="PRESSURE"                   displayLabel="Pressure"                   persistenceUnit="PA(DefaultReal)"                presentationUnits="PA(real4u);PSIG(real4u);PSI(real4u)" relativeError="0.0001"/>
 <KindOfQuantity typeName="PROCESS_PIPING_TEMPERATURE" displayLabel="Process Piping Temperature" persistenceUnit="K(DefaultReal)"                 presentationUnits="CELSIUS(real2u);FAHRENHEIT(real2u);K(real4u)" relativeError="0.01"/>
 <KindOfQuantity typeName="TEMPERATURE"                displayLabel="Temperature"                persistenceUnit="K(DefaultReal)"                 presentationUnits="CELSIUS(real4u);FAHRENHEIT(real4u);K(real4u)" relativeError="0.01"/>
 <KindOfQuantity typeName="PROCESS_PIPING_FLOW"        displayLabel="Process Piping Flow Rate"   persistenceUnit="CUB.M/SEC(DefaultReal)"         presentationUnits="LITRE/MIN(real4u);CUB.FT/MIN(real4u);GALLON/MIN(real4u)" relativeError="0.00001"/>
 <KindOfQuantity typeName="FLOW"                       displayLabel="Flow Rate"                  persistenceUnit="CUB.M/SEC(DefaultReal)"         presentationUnits="LITRE/MIN(real4u);CUB.FT/MIN(real4u);GALLON/MIN(real4u)" relativeError="0.00001"/>
 <KindOfQuantity typeName="WEIGHT"                     displayLabel="Weight"                     persistenceUnit="KG(DefaultReal)"                presentationUnits="KG(real2u);LBM(real2u)" relativeError="0.001"/>
 <KindOfQuantity typeName="CURRENT"                    displayLabel="Current"                    persistenceUnit="A(DefaultReal)"                 presentationUnits="A(real4u);KILOAMPERE(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="FORCE"                      displayLabel="Force"                      persistenceUnit="N(DefaultReal)"                 presentationUnits="N(real4u);KN(real4u);LBF(real4u);KPF(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="POWER"                      displayLabel="Power"                      persistenceUnit="W(DefaultReal)"                 presentationUnits="W(real4u);KW(real4u);MEGAW(real4u);BTU/HR(real4u);KILOBTU/HR(real4u);HP(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="ELECTRIC_POTENTIAL"         displayLabel="Electric Potential"         persistenceUnit="VOLT(DefaultReal)"              presentationUnits="VOLT(real4u);KILOVOLT(real4u);MEGAVOLT(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="HEAT_TRANSFER"              displayLabel="Heat Transfer"              persistenceUnit="W/(SQ.M*K)(DefaultReal)"        presentationUnits="W/(SQ.M*K)(real4u);W/(SQ.M*CELSIUS)(real4u);BTU/(SQ.FT*HR*FAHRENHEIT)(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="LUMINOUS_FLUX"              displayLabel="Luminous Flux"              persistenceUnit="LUMEN(DefaultReal)"             presentationUnits="LUMEN(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="ILLUMINANCE"                displayLabel="Illuminance"                persistenceUnit="LUX(DefaultReal)"               presentationUnits="LUX(real4u);LUMEN/SQ.FT(real4u);" relativeError="0.001"/>
 <KindOfQuantity typeName="LUMINOUS_INTENSITY"         displayLabel="Luminous Intensity"         persistenceUnit="CD(DefaultReal)"                presentationUnits="CD(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="VELOCITY"                   displayLabel="Velocity"                   persistenceUnit="M/SEC(DefaultReal)"             presentationUnits="M/SEC(real4u);FT/SEC(real4u);MPH(real4u);KM/HR(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="FREQUENCY"                  displayLabel="Frequency"                  persistenceUnit="HZ(DefaultReal)"                presentationUnits="HZ(real4u);KHZ(real4u);MHZ(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="THERMAL_RESISTANCE"         displayLabel="Thermal Resistance"         persistenceUnit="(SQ.M*KELVIN)/WATT(DefaultReal)" presentationUnits="(SQ.M*KELVIN)/WATT(real4u);(SQ.M*CELSIUS)/WATT(real4u);(SQ.FT*HR*FAHRENHEIT)/BTU(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="PRESSURE_GRADIENT"          displayLabel="Pressure Gradient"          persistenceUnit="PA/M(DefaultReal)"              presentationUnits="PA/M(real4u);BAR/KM(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="ENERGY"                     displayLabel="Energy"                     persistenceUnit="J(DefaultReal)"                 presentationUnits="J(real4u);KJ(real4u);BTU(real4u);KWH(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="DYNAMIC_VISCOSITY"          displayLabel="Dynamic Viscosity"          persistenceUnit="PA-S(DefaultReal)"              presentationUnits="PA-S(real4u);POISE(real4u);CENTIPOISE(real4u);LBM/(FT*S)(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="TIME"                       displayLabel="Time"                       persistenceUnit="S(DefaultReal)"                 presentationUnits="S(real4u);MIN(real4u);HR(real4u);DAY(real4u);MS(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="ACCELERATION"               displayLabel="Acceleration"               persistenceUnit="M/SEC.SQ(DefaultReal)"          presentationUnits="M/SEC.SQ(real4u);CM/SEC.SQ(real4u);FT/SEC.SQ(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="LINEAR_FORCE"               displayLabel="Linear Force"               persistenceUnit="N/M(DefaultReal)"               presentationUnits="N/M(real4u);N/MM(real4u);LBF/IN(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="MOMENT_OF_INERTIA"          displayLabel="Moment Of Inertia"          persistenceUnit="M^4(DefaultReal)"               presentationUnits="M^4(real4u);MM^4(real4u);CM^4(real4u);IN^4(real4u);FT^4(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="DENSITY"                    displayLabel="Density"                    persistenceUnit="KG/CUB.M(DefaultReal)"          presentationUnits="KG/CUB.M(real4u);LBM/CUB.FT(real4u);G/CUB.CM(real4u);LBM/CUB.IN(real4u);KIP/CUB.FT(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="THERMAL_EXPANSION_COEFFICIENT"        displayLabel="Thermal Expansion Coefficient"        persistenceUnit="STRAIN/KELVIN(DefaultReal)"            presentationUnits="STRAIN/KELVIN(real4u);STRAIN/CELSIUS(real4u);STRAIN/FAHRENHEIT(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="SPECIFIC_HEAT_OF_VAPORIZATION"        displayLabel="Specific Heat Of Vaporization"        persistenceUnit="J/KG(DefaultReal)"                     presentationUnits="J/KG(real4u);KJ/KG(real4u);BTU/LBM(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="SPECIFIC_HEAT_CAPACITY"               displayLabel="Specific Heat Capacity"               persistenceUnit="J/(KG*K)(DefaultReal)"                 presentationUnits="J/(KG*K)(real4u);BTU/(LBM*RANKINE)(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="LINEAR_DENSITY"                       displayLabel="Linear Density"                       persistenceUnit="KG/M(DefaultReal)"                     presentationUnits="KG/M(real4u);KG/MM(real4u);LBM/FT(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="FORCE_DENSITY"                        displayLabel="Force Density"                        persistenceUnit="N/CUB.M(DefaultReal)"                  presentationUnits="N/CUB.M(real4u);KN/CUB.M(real4u);N/CUB.FT(real4u);KN/CUB.FT(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="LINEAR_ROTATIONAL_SPRING_CONSTANT"    displayLabel="Linear Rotational Spring Constant"    persistenceUnit="N/RAD(DefaultReal)"                    presentationUnits="N/RAD(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="WARPING_CONSTANT"                     displayLabel="Warping Constant"                     persistenceUnit="M^6(DefaultReal)"                      presentationUnits="M^6(real4u);FT^6(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="ANGULAR_VELOCITY"                     displayLabel="Angular Velocity"                     persistenceUnit="RAD/SEC(DefaultReal)"                  presentationUnits="RAD/SEC(real4u);DEG/SEC(real4u);RPM(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="THERMAL_CONDUCTIVITY"                 displayLabel="Thermal Conductivity"                 persistenceUnit="W/(M*K)(DefaultReal)"                  presentationUnits="W/(M*K)(real4u);W/(M*C)(real4u);(BTU*IN)/(SQ.FT*HR*FAHRENHEIT)(real4u)" relativeError="0.001"/>
</ECSchema>)xml")));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      05/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DgnDbCompatibilityTestFixture, PreEC32Enums)
    {
    auto assertEnum = [] (DgnDbCR dgndb, Utf8CP schemaName, Utf8CP enumName, Utf8CP expectedDisplayLabel, Utf8CP expectedDescription, PrimitiveType expectedType, bool expectedIsStrict, std::vector<std::pair<ECValue, Utf8CP>> const& expectedEnumerators)
        {
        // 1) Via schema manager
        ECEnumerationCP ecEnum = dgndb.Schemas().GetEnumeration(schemaName, enumName);
        ASSERT_TRUE(ecEnum != nullptr) << schemaName << "." << enumName;
        Utf8String assertMessage(schemaName);
        assertMessage.append(".").append(enumName);

        EXPECT_EQ((int) expectedType, (int) ecEnum->GetType()) << assertMessage;
        EXPECT_EQ(expectedIsStrict, ecEnum->GetIsStrict()) << assertMessage;
        EXPECT_EQ(expectedDisplayLabel != nullptr, ecEnum->GetIsDisplayLabelDefined()) << assertMessage;
        if (expectedDisplayLabel != nullptr)
            EXPECT_STREQ(expectedDisplayLabel, ecEnum->GetDisplayLabel().c_str()) << assertMessage;

        EXPECT_STREQ(expectedDescription, ecEnum->GetDescription().c_str()) << assertMessage;
        EXPECT_EQ(expectedEnumerators.size(), ecEnum->GetEnumeratorCount()) << assertMessage;

        size_t i = 0;
        for (ECEnumeratorCP enumerator : ecEnum->GetEnumerators())
            {
            std::pair<ECValue, Utf8CP> const& expectedEnumValue = expectedEnumerators[i];
            EXPECT_STRCASEEQ(expectedEnumValue.second, enumerator->GetDisplayLabel().c_str());
            ECValueCR expectedValue = expectedEnumValue.first;
            if (expectedType == PRIMITIVETYPE_Integer)
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
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(dgndb, "SELECT ECInstanceId,Name,DisplayLabel,Description,IsStrict,Type,EnumValues FROM meta.ECEnumerationDef WHERE Name='CustomHandledPropertyStatementType' AND Schema.Id=?")) << assertMessage;
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, ecEnum->GetSchema().GetId())) << stmt.GetECSql() << " | " << assertMessage;
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | " << assertMessage;
        EXPECT_EQ(ecEnum->GetId(), stmt.GetValueId<ECEnumerationId>(0)) << stmt.GetECSql() << " | " << assertMessage;
        EXPECT_STREQ(enumName, stmt.GetValueText(1)) << stmt.GetECSql() << " | " << assertMessage;
        EXPECT_EQ(expectedDisplayLabel == nullptr, stmt.IsValueNull(2)) << stmt.GetECSql() << " | " << assertMessage;
        if (expectedDisplayLabel != nullptr)
            EXPECT_STREQ(expectedDisplayLabel, stmt.GetValueText(2)) << stmt.GetECSql() << " | " << assertMessage;

        EXPECT_EQ(expectedDescription == nullptr, stmt.IsValueNull(3)) << stmt.GetECSql() << " | " << assertMessage;
        if (expectedDescription != nullptr)
            EXPECT_STREQ(expectedDescription, stmt.GetValueText(3)) << stmt.GetECSql() << " | " << assertMessage;

        EXPECT_EQ(expectedIsStrict, stmt.GetValueBoolean(4)) << stmt.GetECSql() << " | " << assertMessage;
        EXPECT_EQ((int) expectedType, stmt.GetValueInt(5)) << stmt.GetECSql() << " | " << assertMessage;

        IECSqlValue const& enumValues = stmt.GetValue(6);
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
                EXPECT_EQ(expectedValue.GetInteger(), enumValue["IntegerValue"].GetInt()) << assertMessage << " Enumerator: " << i;
                }
            else
                {
                EXPECT_TRUE(enumValue["IntegerValue"].IsNull()) << assertMessage << " Enumerator: " << i;
                ASSERT_TRUE(expectedValue.IsString()) << assertMessage << " Enumerator: " << i;
                EXPECT_STREQ(expectedValue.GetUtf8CP(), enumValue["StringValue"].GetText()) << assertMessage << " Enumerator: " << i;
                }
            i++;
            }

        };

    for (TestFile const& testFile : Profile().GetAllVersionsOfTestFile(TESTFILE_PREEC32ENUMS))
        {
        DbResult stat = BE_SQLITE_OK;
        DgnDbPtr dgndb = OpenTestFile(&stat, testFile.GetPath());
        ASSERT_EQ(BE_SQLITE_OK, stat) << testFile.GetPath().GetNameUtf8();
        ASSERT_TRUE(dgndb != nullptr) << testFile.GetPath().GetNameUtf8();

        assertEnum(*dgndb, "BisCore", "CustomHandledPropertyStatementType", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
                    {{ECValue(0), "None"},
                    {ECValue(1), "Select"},
                    {ECValue(2), "Insert"},
                    {ECValue(3), "ReadOnly = Select|Insert"},
                    {ECValue(4), "Update"},
                    {ECValue(6), "InsertUpdate = Insert | Update"},
                    {ECValue(7), "All = Select | Insert | Update"}});

        assertEnum(*dgndb, "CoreCustomAttributes", "DateTimeKind", nullptr, nullptr, PRIMITIVETYPE_String, true,
                    {{ECValue("Unspecified"), nullptr},
                    {ECValue("Utc"), nullptr},
                    {ECValue("Local"), nullptr}});

        assertEnum(*dgndb, "PreEC32Enums", "IntEnumWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                    {{ECValue(0), nullptr},
                    {ECValue(1), nullptr}});

        assertEnum(*dgndb, "PreEC32Enums", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
        {{ECValue("On"), "Turned On"},
        {ECValue("Off"), "Turned Off"}});

        }
    }


