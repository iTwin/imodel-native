/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <set>
#include <ECObjects/SchemaComparer.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE
struct ReportedIssuesTestFixture : public ECDbTestFixture {};


/*
    Reported by: ECPresentation
    Query add UNION for OVERFLOW table as it see it as Vertical partition.
*/
TEST_F(ReportedIssuesTestFixture, VerticalPartitionShouldNeverIncludeOverflowTable) {
    auto bisCore = SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="BisCore" alias="bis" version="01.00.14" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
        <ECEntityClass typeName="Element" modifier="Abstract">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
            </ECCustomAttributes>    
            <ECProperty propertyName="LastMod" typeName="dateTime" readOnly="true"/>
            <ECProperty propertyName="CodeValue" typeName="string"/>
            <ECProperty propertyName="UserLabel" typeName="string"/>
            <ECProperty propertyName="FederationGuid" typeName="binary" extendedTypeName="BeGuid"/>
            <ECProperty propertyName="JsonProperties" typeName="string" extendedTypeName="Json"/>
        </ECEntityClass>
        <ECEntityClass typeName="RoleElement" modifier="Abstract">
            <BaseClass>Element</BaseClass>
            <ECCustomAttributes>
                <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
            </ECCustomAttributes>            
        </ECEntityClass>
    </ECSchema>)xml");
    auto functional = SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="Functional" alias="func" version="01.00.03" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name="BisCore" version="01.00.14" alias="bis"/>
        <ECEntityClass typeName="FunctionalElement" modifier="Abstract">
            <BaseClass>bis:RoleElement</BaseClass>
            <ECCustomAttributes>
                <ShareColumns xmlns="ECDbMap.02.00.00">
                    <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                </ShareColumns>
            </ECCustomAttributes>            
        </ECEntityClass>
        <ECEntityClass typeName="FunctionalComponentElement" modifier="Abstract">
            <BaseClass>FunctionalElement</BaseClass>
        </ECEntityClass>
    </ECSchema>)xml");
    auto processFunctional = SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="ProcessFunctional" alias="pfunc" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name="BisCore" version="01.00.14" alias="bis"/>
        <ECSchemaReference name="Functional" version="01.00.03" alias="func"/>
        <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
        <ECEntityClass typeName="PLANT_BASE_OBJECT" modifier="Abstract">
            <BaseClass>func:FunctionalComponentElement</BaseClass>
            <ECProperty propertyName="OpenPlantTypeName" typeName="string"/>
            <ECProperty propertyName="DesignState" typeName="string"/>
        </ECEntityClass>
        <ECEntityClass typeName="NAMED_ITEM" modifier="Abstract">
            <BaseClass>PLANT_BASE_OBJECT</BaseClass>
            <ECProperty propertyName="DESCRIPTION" typeName="string" priority="99"/>
            <ECProperty propertyName="ALIAS" typeName="string" priority="98"/>
            <ECProperty propertyName="STATUS" typeName="string"/>
            <ECProperty propertyName="INSULATION_FUNCTION" typeName="string"/>
            <ECProperty propertyName="SECONDARY_INSULATION_FUNCTION" typeName="string"/>
            <ECProperty propertyName="FLUSHING_SYSTEM" typeName="string"/>
            <ECProperty propertyName="FLUSHING_TYPE" typeName="string"/>
            <ECProperty propertyName="SPEC_LOCATION_SECONDARY" typeName="string"/>
            <ECProperty propertyName="SPEC_LOCATION" typeName="string"/>
            <ECProperty propertyName="MODULE" typeName="string"/>
            <ECProperty propertyName="COMMISSION" typeName="string"/>
            <ECProperty propertyName="COMMISSION_SUB_SYSTEM" typeName="string"/>
            <ECProperty propertyName="COMMISSIONING_SCP" typeName="string"/>
            <ECProperty propertyName="LEAK_TEST" typeName="string"/>
            <ECProperty propertyName="INSULATION_TOW" typeName="string"/>
            <ECProperty propertyName="MATERIAL_MARK" typeName="string"/>
        </ECEntityClass>
        <ECEntityClass typeName="DEVICE" modifier="Abstract">
            <BaseClass>NAMED_ITEM</BaseClass>
            <ECProperty propertyName="DESIGNER" typeName="string" priority="88"/>
            <ECProperty propertyName="SIZE" typeName="string"/>
            <ECProperty propertyName="MATERIAL" typeName="string" priority="89"/>
            <ECProperty propertyName="ELEVATION" typeName="double" priority="90"/>
            <ECProperty propertyName="PAINT_CODE" typeName="string" priority="91"/>
            <ECProperty propertyName="CREATE_TIMESTAMP" typeName="dateTime" priority="87"/>
            <ECProperty propertyName="TOTAL_WEIGHT" typeName="double" priority="92"/>
            <ECProperty propertyName="DRY_WEIGHT" typeName="double" priority="93"/>
            <ECProperty propertyName="MANUFACTURER" typeName="string" priority="97"/>
            <ECProperty propertyName="DEVICE_TYPE_CODE" typeName="string" priority="95"/>
            <ECProperty propertyName="NUMBER" typeName="string" priority="94"/>
            <ECProperty propertyName="NOMINAL_SIZE" typeName="string"/>
            <ECProperty propertyName="PLANT_AREA" typeName="string"/>
            <ECProperty propertyName="PLANT" typeName="string"/>
            <ECProperty propertyName="SERVICE" typeName="string"/>
            <ECProperty propertyName="SYSTEM" typeName="string"/>
            <ECProperty propertyName="UNIT" typeName="string"/>
            <ECProperty propertyName="ADDITIONAL_CODE" typeName="string"/>
            <ECProperty propertyName="EQUIPMENT_UNIT_CLASSIFICATION" typeName="string"/>
            <ECProperty propertyName="COMPONENT_CLASSIFICATION" typeName="string"/>
            <ECProperty propertyName="COMPONENT_NUMBER" typeName="string"/>
            <ECProperty propertyName="PIPE_TAG_NUMBER" typeName="string"/>
            <ECProperty propertyName="EQUIPMENT_PLANT" typeName="string"/>
            <ECProperty propertyName="PIPING_PLANT" typeName="string"/>
            <ECProperty propertyName="EQUIPMENT_UNIT_CODE" typeName="string"/>
            <ECProperty propertyName="EQUIPMENT_SYSTEM" typeName="string"/>
            <ECProperty propertyName="PIPING_SYSTEM" typeName="string"/>
            <ECProperty propertyName="TRIM_TYPE" typeName="string"/>
            <ECProperty propertyName="CLASS_DESCRIPTION" typeName="string"/>
            <ECProperty propertyName="AABBCC_CODE" typeName="string"/>
            <ECProperty propertyName="MODEL_NUMBER" typeName="string" priority="96"/>
        </ECEntityClass>
        <ECEntityClass typeName="PIPING_AND_INSTRUMENT_COMPONENT" modifier="Abstract">
            <BaseClass>DEVICE</BaseClass>
            <ECProperty propertyName="STOCK_NUMBER" typeName="string"/>
            <ECProperty propertyName="SUFFIX" typeName="string"/>
            <ECProperty propertyName="ORDER_NUMBER" typeName="string"/>
            <ECProperty propertyName="TYPE" typeName="string"/>
            <ECProperty propertyName="AREAID" typeName="string"/>
            <ECProperty propertyName="NOMINAL_DIAMETER" typeName="double"/>
            <ECProperty propertyName="RATING" typeName="string"/>
            <ECProperty propertyName="SCHEDULE" typeName="string"/>
            <ECProperty propertyName="SHORT_DESCRIPTION" typeName="string"/>
            <ECProperty propertyName="SPECIFICATION" typeName="string"/>
            <ECProperty propertyName="SPECID" typeName="string"/>
            <ECProperty propertyName="STANDARD" typeName="string"/>
            <ECProperty propertyName="WEIGHT" typeName="double"/>
            <ECProperty propertyName="UNIT_NUMBER" typeName="string"/>
            <ECProperty propertyName="TRIM_TYPE_RELATED_EQUIPMENT" typeName="string"/>
            <ECProperty propertyName="OPENING_ACTION" typeName="string"/>
            <ECProperty propertyName="PRESSURE_CLASS" typeName="string"/>
            <ECProperty propertyName="IS_INLINE" typeName="boolean"/>
            <ECProperty propertyName="IS_REDUCING" typeName="boolean"/>
            <ECProperty propertyName="IS_SPECIALITY_ITEM" typeName="boolean"/>
            <ECProperty propertyName="CONTROL_SYSTEM" typeName="string"/>
            <ECProperty propertyName="FURNISHED_BY" typeName="string"/>
            <ECProperty propertyName="GROUNDING_DETAIL" typeName="string"/>
            <ECProperty propertyName="INSTALLATION_DETAIL" typeName="string"/>
            <ECProperty propertyName="INSTALLED_BY" typeName="string"/>
            <ECProperty propertyName="PARENT_INSTRUMENT" typeName="string"/>
            <ECProperty propertyName="PLAN_DRAWINGS" typeName="string"/>
            <ECProperty propertyName="TYPICAL_COMPONENTS" typeName="string"/>
            <ECProperty propertyName="LOCATION" typeName="string"/>
            <ECProperty propertyName="PID_DRAWINGS" typeName="string"/>
            <ECProperty propertyName="TAG_CLASS" typeName="string"/>
            <ECProperty propertyName="LOOP_NAME" typeName="string"/>
            <ECProperty propertyName="LOOP_SERVICE" typeName="string"/>
            <ECProperty propertyName="IO_TYPE" typeName="string"/>
            <ECProperty propertyName="TYPE_DESCRIPTION" typeName="string"/>
            <ECProperty propertyName="LOOP_TYPICAL_NUMBER" typeName="string"/>
            <ECProperty propertyName="SKID" typeName="string"/>
            <ECProperty propertyName="DOCUMENT" typeName="string"/>
            <ECProperty propertyName="LLE_PACKAGE" typeName="string"/>
            <ECProperty propertyName="DOCUMENT_NUMBER" typeName="string"/>
            <ECProperty propertyName="WORK_BREAKDOWN_STRUCTURE" typeName="string"/>
            <ECProperty propertyName="LOCATION_PLAN" typeName="string"/>
            <ECProperty propertyName="REVISION" typeName="string"/>
            <ECProperty propertyName="SOI" typeName="string"/>
            <ECProperty propertyName="ISONOTE" typeName="string"/>
            <ECProperty propertyName="ISONAME" typeName="string"/>
            <ECProperty propertyName="COMMISSIONGROUPNO" typeName="string"/>
            <ECProperty propertyName="COMMISSIONGROUPCOLOR" typeName="string"/>
            <ECProperty propertyName="DETAILBUB" typeName="string"/>
            <ECProperty propertyName="TP" typeName="string"/>
            <ECProperty propertyName="CODE_NAME" typeName="string"/>
        </ECEntityClass>
        <ECEntityClass typeName="PIPING_COMPONENT" modifier="Abstract">
            <BaseClass>PIPING_AND_INSTRUMENT_COMPONENT</BaseClass>
            <ECProperty propertyName="PIECE_MARK" typeName="string"/>
            <ECProperty propertyName="FABRICATION_CATEGORY" typeName="string"/>
            <ECProperty propertyName="LINENUMBER" typeName="string"/>
            <ECProperty propertyName="INSULATION_THICKNESS" typeName="double"/>
            <ECProperty propertyName="INSULATION" typeName="string"/>
            <ECProperty propertyName="LENGTH" typeName="double"/>
            <ECProperty propertyName="INSIDE_DIAMETER" typeName="double"/>
            <ECProperty propertyName="NORMAL_OPERATING_PRESSURE" typeName="double" priority="90"/>
            <ECProperty propertyName="OUTSIDE_DIAMETER" typeName="double"/>
            <ECProperty propertyName="PIPE_FLANGE_TYPE" typeName="string"/>
            <ECProperty propertyName="GRADE" typeName="string"/>
            <ECProperty propertyName="SHOP_FIELD" typeName="string"/>
            <ECProperty propertyName="HUB_DEPTH" typeName="double"/>
            <ECProperty propertyName="HUB_WIDTH" typeName="double"/>
            <ECProperty propertyName="TRACING" typeName="string"/>
            <ECProperty propertyName="COMPONENT_CLASSIFICATION" typeName="string"/>
            <ECProperty propertyName="COMPONENT_SUB_CLASSIFICATION" typeName="string"/>
        </ECEntityClass>
        <ECEntityClass typeName="FLUID_REGULATOR" modifier="Abstract">
            <BaseClass>PIPING_COMPONENT</BaseClass>
            <ECProperty propertyName="DESIGN_PRESSURE" typeName="double"/>
            <ECProperty propertyName="DESIGN_TEMPERATURE" typeName="double"/>
            <ECProperty propertyName="OPERATING_TEMPERATURE" typeName="double"/>
            <ECProperty propertyName="OPERATOR" typeName="string"/>
            <ECProperty propertyName="SUB_TYPE" typeName="string"/>
        </ECEntityClass>
        <ECEntityClass typeName="VALVE" modifier="Abstract">
            <BaseClass>FLUID_REGULATOR</BaseClass>
            <ECProperty propertyName="FLOW_RATE" typeName="double"/>
            <ECProperty propertyName="FUNCTION" typeName="string"/>
            <ECProperty propertyName="END_TO_END_LENGTH" typeName="double"/>
            <ECProperty propertyName="DIRECTION" typeName="string"/>
            <ECProperty propertyName="WALL_THICKNESS" typeName="double"/>
            <ECProperty propertyName="PATTERN" typeName="string"/>
            <ECProperty propertyName="UPPER_LIMIT_OPERATING_PRESSURE" typeName="double"/>
            <ECProperty propertyName="DESIGN_LENGTH_CENTER_TO_OUTLET_END" typeName="double"/>
            <ECProperty propertyName="DESIGN_LENGTH_CENTER_TO_RUN_END" typeName="double"/>
            <ECProperty propertyName="OPERATING_TEMPERATURE_RANGE" typeName="double"/>
        </ECEntityClass>
        <ECEntityClass typeName="INSTRUMENT" modifier="Abstract">
            <ECCustomAttributes>
                <IsMixin xmlns="CoreCustomAttributes.01.00.03">
                    <AppliesToEntityClass>DEVICE</AppliesToEntityClass>
                </IsMixin>
            </ECCustomAttributes>        
            <ECProperty propertyName="ACTUATOR_TYPE" typeName="string"/>
            <ECProperty propertyName="ALARM_LIMIT_HH" typeName="string"/>
            <ECProperty propertyName="ALARM_LIMIT_LL" typeName="string"/>
            <ECProperty propertyName="CALIBRATED_RANGE" typeName="string"/>
            <ECProperty propertyName="CONTACT_GAP" typeName="string"/>
            <ECProperty propertyName="ELECTRICAL_PROTECTION" typeName="string"/>
            <ECProperty propertyName="FAIL_MODE" typeName="string"/>
            <ECProperty propertyName="INPUT_SIGNAL" typeName="string"/>
            <ECProperty propertyName="INTERLOCK" typeName="string"/>
            <ECProperty propertyName="OUTPUT_SIGNAL" typeName="string"/>
            <ECProperty propertyName="OUTPUT_TYPE" typeName="string"/>
            <ECProperty propertyName="SET_POINT" typeName="string"/>
            <ECProperty propertyName="SUB_TYPE" typeName="string"/>
            <ECProperty propertyName="UPPER_LIMIT_OVERPRESSURE_PROTECTION" typeName="double"/>
            <ECProperty propertyName="SUPPLIER" typeName="string"/>
            <ECProperty propertyName="TAG_CODE" typeName="string"/>
            <ECProperty propertyName="PIPE_LINE_NUMBER" typeName="string"/>
            <ECProperty propertyName="PID_NUMBER" typeName="string"/>
            <ECProperty propertyName="HOUSING_MATERIAL" typeName="string"/>
            <ECProperty propertyName="CALIBRATION_DATA_REQUIREMENTS" typeName="string"/>
            <ECProperty propertyName="INTRINSICALLY_SAFE_INSTALLATION" typeName="string"/>
            <ECProperty propertyName="INTERFACE_TYPE" typeName="string"/>
            <ECProperty propertyName="ALARM_RANGE" typeName="string"/>
            <ECProperty propertyName="ALARM_SET_POINT" typeName="string"/>
            <ECProperty propertyName="ALARM_TYPE" typeName="string"/>
            <ECProperty propertyName="ALTERNATIVE_NAME" typeName="string"/>
            <ECProperty propertyName="FIELD_CONTACT" typeName="string"/>
            <ECProperty propertyName="INPUT_DESCRIPTION_CLOSED" typeName="string"/>
            <ECProperty propertyName="INPUT_DESCRIPTION_OPEN" typeName="string"/>
            <ECProperty propertyName="INPUT_NORMAL_STATE" typeName="string"/>
            <ECProperty propertyName="LOWER_LIMIT_OUTPUT_PULSE_DURATION" typeName="string"/>
            <ECProperty propertyName="LOWER_LIMIT_OUTPUT_SIGNAL" typeName="string"/>
            <ECProperty propertyName="MEMORY_ADDRESS" typeName="string"/>
            <ECProperty propertyName="PROJECT_STATUS" typeName="string"/>
            <ECProperty propertyName="REQUISITION_NUMBER" typeName="string"/>
            <ECProperty propertyName="REVISION_NUMBER" typeName="string"/>
            <ECProperty propertyName="SEGMENT_ADDRESS" typeName="string"/>
            <ECProperty propertyName="COMMENT" typeName="string"/>
            <ECProperty propertyName="OUTPUT_DIRECTION" typeName="string"/>
            <ECProperty propertyName="UPPER_LIMIT_OUTPUT_SIGNAL" typeName="string"/>
            <ECProperty propertyName="INPUT_LIMIT_H" typeName="string"/>
            <ECProperty propertyName="DATA_TYPE" typeName="string"/>
            <ECProperty propertyName="DCS_KEYWORD" typeName="string"/>
            <ECProperty propertyName="DCS_TAG_IDENTIFICATION_CODE" typeName="string"/>
            <ECProperty propertyName="DCS_TAG_DESCRIPTION" typeName="string"/>
            <ECProperty propertyName="COST" typeName="string"/>
            <ECProperty propertyName="OPERATING_VOLTAGE" typeName="double"/>
            <ECProperty propertyName="PROCESS_FUNCTION" typeName="string"/>
            <ECProperty propertyName="FAILURE_ACTION" typeName="string"/>
            <ECProperty propertyName="INSTRUMENT_CLASS" typeName="string"/>
            <ECProperty propertyName="SEIXGRDET" typeName="string"/>
        </ECEntityClass>
        <ECEntityClass typeName="CONTROL_VALVE">
            <BaseClass>VALVE</BaseClass>
            <BaseClass>INSTRUMENT</BaseClass>
        </ECEntityClass>
    </ECSchema>)xml");

    BeFileName filePath(R"(D:\temp\final2.ecdb)");
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("union_err.ecdb"));
    ASSERT_EQ(SUCCESS, ImportSchemas({bisCore,functional,processFunctional}));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT PAINT_CODE from pfunc.CONTROL_VALVE"));
    Utf8String nativeSql = stmt.GetNativeSql();
    ASSERT_EQ(Utf8String::npos, nativeSql.find("UNION"));
    ASSERT_STREQ("SELECT [CONTROL_VALVE].[js23] FROM (SELECT [ElementId] ECInstanceId,[ECClassId],[js23] FROM [main].[func_FunctionalElement] WHERE [func_FunctionalElement].ECClassId=77) [CONTROL_VALVE]", stmt.GetNativeSql());
}

END_ECDBUNITTESTS_NAMESPACE