/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/TestECDbCreators.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "CompatibilityTestFixture.h"

// List of Test Files against which tests are written.
// Note, they might also be from future versions of ECDb in which case there is no TestECDbCreator, as only
// the newer versions of ECDb can create them. But tests have to be written with the old code against those files
// to make sure that the old software can work with the newer files.
#define TESTECDB_EMPTY "empty.ecdb"
#define TESTECDB_PREEC32ENUMS "preec32enums.ecdb"
#define TESTECDB_EC32ENUMS "ec32enums.ecdb"
#define TESTECDB_PREEC32KOQS "preec32koqs.ecdb"
#define TESTECDB_EC32KOQS "ec32koqs.ecdb"


// *** Instructions for adding new test file creators ***
// 1) Define the test file name with a TESTECDB_ macro
// 2) Add a new subclass of TestECDbCreator
// 3) Add a unique_ptr to the TESTECDBCREATOR_LIST macro

#define TESTECDBCREATOR_LIST {std::make_shared<EmptyTestECDbCreator>(), \
                              std::make_shared<EC32EnumsTestECDbCreator>(), \
                              std::make_shared<PreEC32EnumsTestECDbCreator>(), \
                              std::make_shared<EC32KoqsTestECDbCreator>(), \
                               std::make_shared<PreEC32KoqsTestECDbCreator>()}

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct TestECDbCreation final
    {
private:
    static bool s_hasRun;

    TestECDbCreation() = delete;
    ~TestECDbCreation() = delete;

public:
    static BentleyStatus Run();
    };

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct TestECDbCreator : TestFileCreator
    {
    protected:
        TestECDbCreator() : TestFileCreator() {}

    public:
        virtual ~TestECDbCreator() {}

        static DbResult CreateNewTestFile(ECDbR, Utf8CP fileName);
        static BentleyStatus ImportSchema(ECDbCR ecdb, SchemaItem const& schema) { return ImportSchemas(ecdb, {schema}); }
        static BentleyStatus ImportSchemas(ECDbCR, std::vector<SchemaItem> const&);
    };

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct EmptyTestECDbCreator final : TestECDbCreator
    {
    private:
        BentleyStatus _Create() override
            {
            ECDb ecdb;
            return BE_SQLITE_OK == CreateNewTestFile(ecdb, TESTECDB_EMPTY) ? SUCCESS : ERROR;
            }
    public:
        ~EmptyTestECDbCreator() {}
    };

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct PreEC32EnumsTestECDbCreator final : TestECDbCreator
    {
    private:
        BentleyStatus _Create() override
            {
            ECDb ecdb;
            if (BE_SQLITE_OK != CreateNewTestFile(ecdb, TESTECDB_PREEC32ENUMS))
                return ERROR;

            // add types of enums which don't exist in the schemas already in the test file
            return ImportSchema(ecdb, SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
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
                                                     </ECSchema>)xml"));
            }
    public:
        ~PreEC32EnumsTestECDbCreator() {}
    };

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct EC32EnumsTestECDbCreator final : TestECDbCreator
    {
    private:
        BentleyStatus _Create() override
            {
            ECDb ecdb;
            if (BE_SQLITE_OK != CreateNewTestFile(ecdb, TESTECDB_EC32ENUMS))
                return ERROR;

            // add types of enums which don't exist in the schemas already in the test file
            return ImportSchema(ecdb, SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                                    <ECSchema schemaName="EC32Enums" alias="ec32enums" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
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
        ~EC32EnumsTestECDbCreator() {}
    };

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct PreEC32KoqsTestECDbCreator final : TestECDbCreator
    {
    private:
        BentleyStatus _Create() override
            {
            ECDb ecdb;
            if (BE_SQLITE_OK != CreateNewTestFile(ecdb, TESTECDB_PREEC32KOQS))
                return ERROR;

            //test schema for KOQs originates from AECUnits ECSchemas
            return ImportSchema(ecdb, SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
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

 <KindOfQuantity typeName="TestKoq_NoPresUnit"  persistenceUnit="W/(M*K)" relativeError="0.4"/>
 <KindOfQuantity typeName="TestKoq_PersUnitWithFormat_NoPresUnit"  persistenceUnit="W/(M*K)(DefaultReal)" relativeError="0.5"/>
 <KindOfQuantity typeName="TestKoq_PersUnitWithFormatWithUnit_NoPresUnit"  persistenceUnit="FT(AmerFI8)" relativeError="0.6"/>

</ECSchema>)xml"));
            }
    public:
        ~PreEC32KoqsTestECDbCreator() {}
    };

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct EC32KoqsTestECDbCreator final : TestECDbCreator
    {
    private:
        BentleyStatus _Create() override
            {
            ECDb ecdb;
            if (BE_SQLITE_OK != CreateNewTestFile(ecdb, TESTECDB_EC32KOQS))
                return ERROR;

            return ImportSchema(ecdb, SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                <ECSchema schemaName="EC32Koqs" alias="ec32koqs" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                    <ECSchemaReference name="Formats" version="01.00.00" alias="f" />
                    <KindOfQuantity typeName="TestKoq_PresFormatWithMandatoryComposite" displayLabel="My first test KOQ" persistenceUnit="u:CM" presentationUnits="f:DefaultRealU(4)[u:M]" relativeError="0.1"/>
                    <KindOfQuantity typeName="TestKoq_PresFormatWithOptionalComposite" description="My second test KOQ" persistenceUnit="u:CM" presentationUnits="f:AmerFI[u:FT|feet][u:IN|inches]" relativeError="0.2"/>
                    <KindOfQuantity typeName="TestKoq_PresFormatWithoutComposite" persistenceUnit="u:CM" presentationUnits="f:AmerFI" relativeError="0.3"/>
                    <KindOfQuantity typeName="TestKoq_NoPresFormat" persistenceUnit="u:KG" relativeError="0.4"/>
                </ECSchema>)xml"));
            }
    public:
        ~EC32KoqsTestECDbCreator() {}
    };
