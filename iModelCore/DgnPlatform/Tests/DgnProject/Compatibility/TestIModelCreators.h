/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/TestIModelCreators.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "CompatibilityTestFixture.h"

// List of Test Files against which tests are written.
// Note, they might also be from future versions of DgnPlatform in which case there is no TestImodelCreator, as only
// the newer versions of DgnPlatform can create them. But tests have to be written with the old code against those files
// to make sure that the old software can work with the newer files.

// *** Instructions for adding new test file creators ***
// 1) Define the test file name with a TESTIMODEL_ macro
// 2) Add a new subclass of TestIModelCreator
// 3) Add a unique_ptr to the TESTIMODELCREATOR_LIST macro
#define TESTIMODEL_EMPTY "empty.bim"
#define TESTIMODEL_EC31ENUMS "ec31enums.bim"
#define TESTIMODEL_EC32ENUMS_PROFILEUPGRADED "ec32enums_profileupgraded.bim"
#define TESTIMODEL_EC32ENUMS "ec32enums.bim"
#define TESTIMODEL_EC31KOQS "ec31koqs.bim"
#define TESTIMODEL_EC31THREADPITCHKOQS "ec31threadpitchkoqs.ecdb"
#define TESTIMODEL_EC32KOQS "ec32koqs.bim"
#define TESTIMODEL_EC32UNITS "ec32units.bim"
#define TESTIMODEL_EC31ENUMS_SCHEMAUPGRADE "ec31enums_schemaupgrade.bim"
#define TESTIMODEL_EC31KOQS_SCHEMAUPGRADE "ec31koqs_schemaupgrade.bim"
#define TESTIMODEL_EC32ENUMS_SCHEMAUPGRADE "ec32enums_schemaupgrade.bim"
#define TESTIMODEL_EC32KOQS_SCHEMAUPGRADE "ec32koqs_schemaupgrade.bim"
#define TESTIMODEL_TESTDOMAIN "testdomain.bim"

#define TESTIMODELCREATOR_LIST {std::make_shared<EmptyTestIModelCreator>(), \
                              std::make_shared<EC31EnumsTestIModelCreator>(), \
                              std::make_shared<EC32EnumsTestIModelCreator>(), \
                              std::make_shared<EC32EnumsProfileUpgradedTestIModelCreator>(), \
                              std::make_shared<EC31EnumsSchemaUpgradeTestIModelCreator>(), \
                              std::make_shared<EC32EnumsSchemaUpgradeTestIModelCreator>(), \
                              std::make_shared<EC31KoqsTestIModelCreator>(), \
                              std::make_shared<EC31ThreadPitchKoqsTestIModelCreator>(), \
                              std::make_shared<EC32KoqsTestIModelCreator>(), \
                              std::make_shared<EC31KoqsSchemaUpgradeTestIModelCreator>(), \
                              std::make_shared<EC32KoqsSchemaUpgradeTestIModelCreator>(), \
                              std::make_shared<EC32UnitsTestIModelCreator>(), \
                              std::make_shared<TestDomainTestIModelCreator>()}


//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct TestIModelCreation final
    {
private:
    static bool s_hasRun;

    TestIModelCreation() = delete;
    ~TestIModelCreation() = delete;

public:
    static BentleyStatus Run();
    };

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct TestIModelCreator : TestFileCreator
    {
private:
    BentleyStatus _UpgradeOldFiles() const override;

protected:
    explicit TestIModelCreator(Utf8CP fileName) : TestFileCreator(fileName) {}

    static DgnDbPtr CreateNewTestFile(Utf8StringCR fileName);
    static BentleyStatus ImportSchema(DgnDbR dgndb, SchemaItem const& schema) { return ImportSchemas(dgndb, {schema}); }
    static BentleyStatus ImportSchemas(DgnDbR dgndb, std::vector<SchemaItem> const& schemas);

public:
    virtual ~TestIModelCreator() {}
    };

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct EmptyTestIModelCreator final : TestIModelCreator
    {
    private:
        BentleyStatus _Create() override { return CreateNewTestFile(m_fileName) != nullptr ? SUCCESS : ERROR;  }

    public:
        EmptyTestIModelCreator() : TestIModelCreator(TESTIMODEL_EMPTY) {}
        ~EmptyTestIModelCreator() {}
    };


//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct EC31EnumsTestIModelCreator final : TestIModelCreator
    {
    private:
        BentleyStatus _Create() override
            {
            DgnDbPtr bim = CreateNewTestFile(m_fileName);
            if (bim == nullptr)
                return ERROR;

            // add types of enums which don't exist in the schemas already in the test file
            return ImportSchema(*bim, SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                                    <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                                        <ECEnumeration typeName="IntEnum_EnumeratorsWithoutDisplayLabel" displayLabel="Int Enumeration with enumerators without display label" description="Int Enumeration with enumerators without display label" backingTypeName="int" isStrict="true">
                                                            <ECEnumerator value="0"/>
                                                            <ECEnumerator value="1"/>
                                                            <ECEnumerator value="2"/>
                                                        </ECEnumeration>
                                                        <ECEnumeration typeName="StringEnum_EnumeratorsWithDisplayLabel" displayLabel="String Enumeration with enumerators with display label" backingTypeName="string" isStrict="false">
                                                            <ECEnumerator value="On" displayLabel="Turned On"/>
                                                            <ECEnumerator value="Off" displayLabel="Turned Off"/>
                                                        </ECEnumeration>
                                                     </ECSchema>)xml"));
            }
    public:
        explicit EC31EnumsTestIModelCreator() : TestIModelCreator(TESTIMODEL_EC31ENUMS) {}
        ~EC31EnumsTestIModelCreator() {}
    };

//======================================================================================
// @bsiclass                                               Krischan.Eberle      07/2018
//======================================================================================
struct EC32EnumsProfileUpgradedTestIModelCreator final : TestIModelCreator
    {
    private:
        BentleyStatus _Create() override
            {
            DgnDbPtr bim = CreateNewTestFile(m_fileName);
            if (bim == nullptr)
                return ERROR;

            //Initial import of EC3.1 enums. _UpgradeSchemas will then upgrade to EC32 enums
            return ImportSchema(*bim, SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                                    <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                                        <ECEnumeration typeName="IntEnum_EnumeratorsWithoutDisplayLabel" displayLabel="Int Enumeration with enumerators without display label" description="Int Enumeration with enumerators without display label" backingTypeName="int" isStrict="true">
                                                            <ECEnumerator value="0"/>
                                                            <ECEnumerator value="1"/>
                                                            <ECEnumerator value="2"/>
                                                        </ECEnumeration>
                                                        <ECEnumeration typeName="StringEnum_EnumeratorsWithDisplayLabel" displayLabel="String Enumeration with enumerators with display label" backingTypeName="string" isStrict="false">
                                                            <ECEnumerator value="On" displayLabel="Turned On"/>
                                                            <ECEnumerator value="Off" displayLabel="Turned Off"/>
                                                        </ECEnumeration>
                                                     </ECSchema>)xml"));
            }

        BentleyStatus _UpgradeSchemas() const override;

    public:
        explicit EC32EnumsProfileUpgradedTestIModelCreator() : TestIModelCreator(TESTIMODEL_EC32ENUMS_PROFILEUPGRADED) {}
        ~EC32EnumsProfileUpgradedTestIModelCreator() {}
    };

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct EC31KoqsTestIModelCreator final : TestIModelCreator
    {
    private:
        BentleyStatus _Create() override
            {
            DgnDbPtr bim = CreateNewTestFile(m_fileName);
            if (bim == nullptr)
                return ERROR;

            //test schema for KOQs originates from AECUnits ECSchemas
            return ImportSchema(*bim, SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
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
 <KindOfQuantity typeName="PROCESS_PIPING_PRESSURE" displayLabel="Process Piping Pressure" persistenceUnit="PA(DefaultReal)" presentationUnits="PA(real4u);PSIG(real4u);PSI(real4u)" relativeError="0.0001"/>
 <KindOfQuantity typeName="PRESSURE" displayLabel="Pressure" persistenceUnit="PA(DefaultReal)" presentationUnits="PA(real4u);PSIG(real4u);PSI(real4u)" relativeError="0.0001"/>
 <KindOfQuantity typeName="PROCESS_PIPING_TEMPERATURE" displayLabel="Process Piping Temperature" persistenceUnit="K(DefaultReal)" presentationUnits="CELSIUS(real2u);FAHRENHEIT(real2u);K(real4u)" relativeError="0.01"/>
 <KindOfQuantity typeName="TEMPERATURE"                displayLabel="Temperature"                persistenceUnit="K(DefaultReal)" presentationUnits="CELSIUS(real4u);FAHRENHEIT(real4u);K(real4u)" relativeError="0.01"/>
 <KindOfQuantity typeName="PROCESS_PIPING_FLOW"        displayLabel="Process Piping Flow Rate"   persistenceUnit="CUB.M/SEC(DefaultReal)" presentationUnits="LITRE/MIN(real4u);CUB.FT/MIN(real4u);GALLON/MIN(real4u)" relativeError="0.00001"/>
 <KindOfQuantity typeName="FLOW"                       displayLabel="Flow Rate"                  persistenceUnit="CUB.M/SEC(DefaultReal)" presentationUnits="LITRE/MIN(real4u);CUB.FT/MIN(real4u);GALLON/MIN(real4u)" relativeError="0.00001"/>
 <KindOfQuantity typeName="WEIGHT"                     displayLabel="Weight"                     persistenceUnit="KG(DefaultReal)" presentationUnits="KG(real2u);LBM(real2u)" relativeError="0.001"/>
 <KindOfQuantity typeName="CURRENT"                    displayLabel="Current"                    persistenceUnit="A(DefaultReal)" presentationUnits="A(real4u);KILOAMPERE(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="FORCE"                      displayLabel="Force"                      persistenceUnit="N(DefaultReal)" presentationUnits="N(real4u);KN(real4u);LBF(real4u);KPF(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="POWER"                      displayLabel="Power"                      persistenceUnit="W(DefaultReal)" presentationUnits="W(real4u);KW(real4u);MEGAW(real4u);BTU/HR(real4u);KILOBTU/HR(real4u);HP(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="ELECTRIC_POTENTIAL"         displayLabel="Electric Potential"         persistenceUnit="VOLT(DefaultReal)" presentationUnits="VOLT(real4u);KILOVOLT(real4u);MEGAVOLT(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="HEAT_TRANSFER"              displayLabel="Heat Transfer"              persistenceUnit="W/(SQ.M*K)(DefaultReal)" presentationUnits="W/(SQ.M*K)(real4u);W/(SQ.M*CELSIUS)(real4u);BTU/(SQ.FT*HR*FAHRENHEIT)(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="LUMINOUS_FLUX"              displayLabel="Luminous Flux"              persistenceUnit="LUMEN(DefaultReal)" presentationUnits="LUMEN(real4u)" relativeError="0.001"/>
 <KindOfQuantity typeName="ILLUMINANCE"                displayLabel="Illuminance"                persistenceUnit="LUX(DefaultReal)" presentationUnits="LUX(real4u);LUMEN/SQ.FT(real4u);" relativeError="0.001"/>
 <KindOfQuantity typeName="LUMINOUS_INTENSITY"         displayLabel="Luminous Intensity"         persistenceUnit="CD(DefaultReal)" presentationUnits="CD(real4u)" relativeError="0.001"/>
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

 <KindOfQuantity typeName="TestKoq_NoPresUnit" persistenceUnit="W/(M*K)" relativeError="0.4"/>
 <KindOfQuantity typeName="TestKoq_PersUnitWithFormat_NoPresUnit" persistenceUnit="W/(M*K)(DefaultReal)" relativeError="0.5"/>
 <KindOfQuantity typeName="TestKoq_PersUnitWithFormatWithUnit_NoPresUnit" persistenceUnit="FT(AmerFI8)" relativeError="0.6"/>
 <KindOfQuantity typeName="TestKoq_M_Mfi8" persistenceUnit="M" presentationUnits="M(fi8)" relativeError="0.7"/>
 <KindOfQuantity typeName="TestKoq_Mfi8" persistenceUnit="M(fi8)" relativeError="0.8"/>
 <KindOfQuantity typeName="TestKoq_SQFTfi8" persistenceUnit="SQ.FT(fi8)" relativeError="0.9"/>
 <KindOfQuantity typeName="TestKoq_SQFTfi8_SQFTreal4u" persistenceUnit="SQ.FT(fi8)" presentationUnits="SQ.FT(real4u)" relativeError="1.0"/>

 <!-- KOQs which fail to deserialize with EC3.1 and are just dropped in EC3.2 -->
 <KindOfQuantity typeName="TestKoq_LUX_M" persistenceUnit="LUX" presentationUnits="M" relativeError="1.1"/>
 <KindOfQuantity typeName="TestKoq_M_LUX" persistenceUnit="M" presentationUnits="LUX" relativeError="1.2"/>
 <KindOfQuantity typeName="TestKoq_M_SQFTreal4u" persistenceUnit="M" presentationUnits="SQ.FT(real4u)" relativeError="1.3"/>
 <KindOfQuantity typeName="TestKoq_M_CM_LUX" persistenceUnit="M" presentationUnits="CM;LUX" relativeError="1.4"/>
 <KindOfQuantity typeName="TestKoq_LUX_CM_MM" persistenceUnit="LUX" presentationUnits="CM;MM" relativeError="1.5"/>
 <KindOfQuantity typeName="TestKoq_LUXreal4u_CM_MM" persistenceUnit="LUX(real4u)" presentationUnits="CM;MM" relativeError="1.6"/>

</ECSchema>)xml"));
            }
    public:
        explicit EC31KoqsTestIModelCreator() : TestIModelCreator(TESTIMODEL_EC31KOQS) {}
        ~EC31KoqsTestIModelCreator() {}
    };

//======================================================================================
// Thread pitch units were only added later to the legacy units handling. So we 
// dedicate a separate test for them
// @bsiclass                                               Krischan.Eberle      09/2018
//======================================================================================
struct EC31ThreadPitchKoqsTestIModelCreator final : TestIModelCreator
    {
    private:
        BentleyStatus _Create() override
            {
            DgnDbPtr bim = CreateNewTestFile(m_fileName);
            if (bim == nullptr)
                return ERROR;

            return ImportSchema(*bim, SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                            <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                <KindOfQuantity typeName="TestKoq_IN_DEGREE" persistenceUnit="IN/DEGREE" presentationUnits="" relativeError="1.0"/>
                                <KindOfQuantity typeName="TestKoq_IN_DEGREE_DEFAULTREALU" persistenceUnit="IN/DEGREE(DefaultRealU)" presentationUnits="" relativeError="1.1"/>
                                <KindOfQuantity typeName="TestKoq_M_REVOLUTION" persistenceUnit="M/REVOLUTION" presentationUnits="" relativeError="1.2"/>
                                <KindOfQuantity typeName="TestKoq_M_REVOLUTION_DEFAULTREALU" persistenceUnit="M/REVOLUTION(DefaultRealU)" presentationUnits="" relativeError="1.3"/>
                            </ECSchema>)xml"));
            }

    public:
        explicit EC31ThreadPitchKoqsTestIModelCreator() : TestIModelCreator(TESTIMODEL_EC31THREADPITCHKOQS) {}
        ~EC31ThreadPitchKoqsTestIModelCreator() {}
    };

//======================================================================================
// @bsiclass                                               Krischan.Eberle      08/2018
//======================================================================================
struct EC31EnumsSchemaUpgradeTestIModelCreator final : TestIModelCreator
    {
    private:
        BentleyStatus _Create() override
            {
            DgnDbPtr bim = CreateNewTestFile(m_fileName);
            if (bim == nullptr)
                return ERROR;

            return ImportSchema(*bim, SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                 <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
                 <ECEnumeration typeName="StatusEnum" displayLabel="Int Enumeration with enumerators without display label" backingTypeName="int" isStrict="true">
                    <ECEnumerator value="0"/>
                    <ECEnumerator value="1"/>
                    <ECEnumerator value="2"/>
                 </ECEnumeration>
                 <ECEntityClass typeName="MyDomainClass">
                    <BaseClass>bis:PhysicalElement</BaseClass>
                    <ECProperty propertyName="Status" typeName="StatusEnum" />
                 </ECEntityClass>
            </ECSchema>)xml"));
            }
    public:
        explicit EC31EnumsSchemaUpgradeTestIModelCreator() : TestIModelCreator(TESTIMODEL_EC31ENUMS_SCHEMAUPGRADE) {}
        ~EC31EnumsSchemaUpgradeTestIModelCreator() {}
    };

//======================================================================================
// @bsiclass                                               Krischan.Eberle      08/2018
//======================================================================================
struct EC31KoqsSchemaUpgradeTestIModelCreator final : TestIModelCreator
    {
    private:
        BentleyStatus _Create() override
            {
            DgnDbPtr bim = CreateNewTestFile(m_fileName);
            if (bim == nullptr)
                return ERROR;

            return ImportSchema(*bim, SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
                <KindOfQuantity typeName="AREA" displayLabel="Area" persistenceUnit="SQ.M(DefaultReal)" presentationUnits="SQ.M(real4u);SQ.FT(real4u)" relativeError="0.0001"/>
                <ECEntityClass typeName="MyDomainClass">
                   <BaseClass>bis:PhysicalElement</BaseClass>
                   <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
                </ECEntityClass>
            </ECSchema>)xml"));
            }
    public:
        explicit EC31KoqsSchemaUpgradeTestIModelCreator() : TestIModelCreator(TESTIMODEL_EC31KOQS_SCHEMAUPGRADE) {}
        ~EC31KoqsSchemaUpgradeTestIModelCreator() {}
    };

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct EC32KoqsTestIModelCreator final : TestIModelCreator
    {
    private:
        BentleyStatus _Create() override
            {
            DgnDbPtr bim = CreateNewTestFile(m_fileName);
            if (bim == nullptr)
                return ERROR;

            return ImportSchema(*bim, SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                    <ECSchemaReference name="Formats" version="01.00.00" alias="f" />
                    <KindOfQuantity typeName="TestKoq_PresFormatWithMandatoryComposite" displayLabel="My first test KOQ" persistenceUnit="u:CM" presentationUnits="f:DefaultRealU(4)[u:M]" relativeError="0.1"/>
                    <KindOfQuantity typeName="TestKoq_PresFormatWithOptionalComposite" description="My second test KOQ" persistenceUnit="u:CM" presentationUnits="f:AmerFI[u:FT|feet][u:IN|inches]" relativeError="0.2"/>
                    <KindOfQuantity typeName="TestKoq_PresFormatWithoutComposite" persistenceUnit="u:CM" presentationUnits="f:AmerFI" relativeError="0.3"/>
                    <KindOfQuantity typeName="TestKoq_NoPresFormat" persistenceUnit="u:KG" relativeError="0.4"/>
                </ECSchema>)xml"));
            }
    public:
        EC32KoqsTestIModelCreator() : TestIModelCreator(TESTIMODEL_EC32KOQS) {}
        ~EC32KoqsTestIModelCreator() {}
    };

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct EC32EnumsTestIModelCreator final : TestIModelCreator
    {
    private:
        BentleyStatus _Create() override
            {
            DgnDbPtr bim = CreateNewTestFile(m_fileName);
            if (bim == nullptr)
                return ERROR;

            // ECObjects downgrades an EC3.2 file to EC3.1 during deserialization
            return ImportSchema(*bim, SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                                    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                                        <ECEnumeration typeName="IntEnum_EnumeratorsWithoutDisplayLabel" displayLabel="Int Enumeration with enumerators without display label" description="Int Enumeration with enumerators without display label" backingTypeName="int" isStrict="true">
                                                            <ECEnumerator name="Unknown" value="0"/>
                                                            <ECEnumerator name="On" value="1"/>
                                                            <ECEnumerator name="Off" value="2"/>
                                                        </ECEnumeration>
                                                        <ECEnumeration typeName="StringEnum_EnumeratorsWithDisplayLabel" displayLabel="String Enumeration with enumerators with display label" backingTypeName="string" isStrict="false">
                                                            <ECEnumerator name="On" value="On" displayLabel="Turned On"/>
                                                            <ECEnumerator name="Off" value="Off" displayLabel="Turned Off"/>
                                                        </ECEnumeration>
                                                     </ECSchema>)xml"));
            }
    public:
        EC32EnumsTestIModelCreator() : TestIModelCreator(TESTIMODEL_EC32ENUMS) {}
        ~EC32EnumsTestIModelCreator() {}
    };

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct EC32EnumsSchemaUpgradeTestIModelCreator final : TestIModelCreator
    {
    private:
        BentleyStatus _Create() override
            {
            DgnDbPtr bim = CreateNewTestFile(m_fileName);
            if (bim == nullptr)
                return ERROR;

            return ImportSchema(*bim, SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                                    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                                         <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
                                                         <ECEnumeration typeName="StatusEnum" displayLabel="Int Enumeration with enumerators without display label" backingTypeName="int" isStrict="true">
                                                            <ECEnumerator name="On" value="0"/>
                                                            <ECEnumerator name="Off" value="1"/>
                                                            <ECEnumerator name="Unknown" value="2"/>
                                                         </ECEnumeration>
                                                        <ECEntityClass typeName="MyDomainClass">
                                                            <BaseClass>bis:PhysicalElement</BaseClass>
                                                            <ECProperty propertyName="Status" typeName="StatusEnum" />
                                                        </ECEntityClass>
                                                   </ECSchema>)xml"));
            }
    public:
        EC32EnumsSchemaUpgradeTestIModelCreator() : TestIModelCreator(TESTIMODEL_EC32ENUMS_SCHEMAUPGRADE) {}
        ~EC32EnumsSchemaUpgradeTestIModelCreator() {}
    };

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct EC32KoqsSchemaUpgradeTestIModelCreator final : TestIModelCreator
    {
    private:
        BentleyStatus _Create() override
            {
            DgnDbPtr bim = CreateNewTestFile(m_fileName);
            if (bim == nullptr)
                return ERROR;

            return ImportSchema(*bim, SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                                    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                                         <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
                                                         <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                                                         <ECSchemaReference name="Formats" version="01.00.00" alias="f" />
                                                         <KindOfQuantity typeName="AREA" displayLabel="Area" persistenceUnit="u:SQ_M" presentationUnits="f:DefaultRealU(4)[u:SQ_M];f:DefaultRealU(4)[u:SQ_FT]" relativeError="0.0001"/>
                                                         <ECEntityClass typeName="MyDomainClass">
                                                            <BaseClass>bis:PhysicalElement</BaseClass>
                                                            <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
                                                        </ECEntityClass>
                                                   </ECSchema>)xml"));
            }
    public:
        EC32KoqsSchemaUpgradeTestIModelCreator() : TestIModelCreator(TESTIMODEL_EC32KOQS_SCHEMAUPGRADE) {}
        ~EC32KoqsSchemaUpgradeTestIModelCreator() {}
    };

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct EC32UnitsTestIModelCreator final : TestIModelCreator
    {
    private:
        BentleyStatus _Create() override
            {
            DgnDbPtr bim = CreateNewTestFile(m_fileName);
            if (bim == nullptr)
                return ERROR;

            return ImportSchema(*bim, SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                    <ECSchemaReference name="Formats" version="01.00.00" alias="f" />
                    <UnitSystem typeName="MyMetric" displayLabel="Metric" description="Metric Units of measure" />
                    <UnitSystem typeName="MyImperial" displayLabel="Imperial" description="Units of measure from the British Empire" />
                    <Phenomenon typeName="MyArea" displayLabel="Area" definition="LENGTH*LENGTH" />
                    <Unit typeName="MySquareM" displayLabel="Square Meter" definition="M*M" numerator="1.0" phenomenon="MyArea" unitSystem="MyMetric" />
                    <Unit typeName="MySquareFt" displayLabel="Square Feet" definition="Ft*Ft" numerator="10.0" offset="0.4" phenomenon="MyArea" unitSystem="MyImperial" />
                    <Format typeName="MyFormat" displayLabel="My Format" roundFactor="0.3" type="Fractional" showSignOption="OnlyNegative" formatTraits="TrailZeroes|KeepSingleZero"
                            precision="4" decimalSeparator="." thousandSeparator="," uomSeparator=" ">
                    </Format>
                    <Format typeName="MyFormatWithComposite" displayLabel="My Format with composite" type="Decimal" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel" precision="2" >
                        <Composite spacer="-">
                            <Unit label="hour">u:HR</Unit>
                            <Unit label="min">u:MIN</Unit>
                        </Composite>
                    </Format>
                    <KindOfQuantity typeName="KoqWithCustomFormat" persistenceUnit="u:M" presentationUnits="MyFormat[u:M]" relativeError="0.1"/>
                    <KindOfQuantity typeName="KoqWithCustomUnit" persistenceUnit="MySquareM" presentationUnits="f:DefaultRealU(4)[MySquareM]" relativeError="0.2"/>
                    <KindOfQuantity typeName="KoqWithCustomUnitAndFormat" persistenceUnit="MySquareFt" presentationUnits="MyFormat[MySquareFt]" relativeError="0.3"/>
                </ECSchema>)xml"));
            }
    public:
        EC32UnitsTestIModelCreator() : TestIModelCreator(TESTIMODEL_EC32UNITS) {}
        ~EC32UnitsTestIModelCreator() {}
    };

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct TestDomainTestIModelCreator final : TestIModelCreator
    {
    private:
        BentleyStatus _Create() override;
    public:
        TestDomainTestIModelCreator() : TestIModelCreator(TESTIMODEL_TESTDOMAIN) {}
        ~TestDomainTestIModelCreator() {}
    };