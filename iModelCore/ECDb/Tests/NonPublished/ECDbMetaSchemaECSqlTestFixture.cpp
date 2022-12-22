/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// @bsistruct
//=======================================================================================
struct ECDbMetaSchemaECSqlTestFixture : ECDbTestFixture
    {
private:
    void AssertSchemaDef(ECSchemaCR expectedSchema, ECSqlStatement const& actualSchemaDefRow);
    void AssertClassDefs(ECSchemaCR expectedSchema);
    void AssertClassDef(ECClassCR expectedClass, ECSqlStatement const& actualClassDefRow);
    void AssertBaseClasses(ECClassCR expectedClass);
    void AssertEnumerationDefs(ECSchemaCR expectedSchema);
    void AssertEnumerationDef(ECEnumerationCR expectedEnum, ECSqlStatement const& actualEnumerationDefRow);
    void AssertEnumerationValue(ECEnumeratorCR expectedEnumValue, IECSqlValue const& actualEnumValue);
    void AssertKindOfQuantityDefs(ECSchemaCR expectedSchema);
    void AssertKindOfQuantityDef(KindOfQuantityCR expectedKoq, ECSqlStatement const& actualKoqDefRow);
    void AssertPropertyCategoryDefs(ECSchemaCR expectedSchema);
    void AssertPropertyCategoryDef(PropertyCategoryCR expectedCat, ECSqlStatement const& actualCatDefRow);
    void AssertPropertyDefs(ECClassCR expectedClass);
    void AssertPropertyDef(ECPropertyCR expectedProp, ECSqlStatement const& actualPropertyDefRow);
    void AssertUnitSystemDefs(ECSchemaCR expectedSchema);
    void AssertUnitSystemDef(UnitSystemCR expected, ECSqlStatement const& actualRow);
    void AssertPhenomenonDefs(ECSchemaCR expectedSchema);
    void AssertPhenomenonDef(PhenomenonCR expected, ECSqlStatement const& actualRow);
    void AssertUnitDefs(ECSchemaCR expectedSchema);
    void AssertUnitDef(ECUnitCR expected, ECSqlStatement const& actualRow);
    void AssertFormatDefs(ECSchemaCR expectedSchema);
    void AssertFormatDef(ECFormatCR expected, ECSqlStatement const& actualRow);

protected:
    void AssertSchemaDefs();
    };

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbMetaSchemaECSqlTestFixture::AssertSchemaDefs()
    {
    ECSqlStatement schemaStatement;
    ASSERT_EQ(ECSqlStatus::Success, schemaStatement.Prepare(m_ecdb, "SELECT Name,* FROM meta.ECSchemaDef"));
    
    int actualSchemaCount = 0;
    while (BE_SQLITE_ROW == schemaStatement.Step())
        {
        Utf8CP actualSchemaName = schemaStatement.GetValueText(0);
        ECSchemaCP expectedSchema = m_ecdb.Schemas().GetSchema(actualSchemaName);
        ASSERT_TRUE(expectedSchema != nullptr);

        AssertSchemaDef(*expectedSchema, schemaStatement);
        AssertClassDefs(*expectedSchema);
        AssertEnumerationDefs(*expectedSchema);
        AssertKindOfQuantityDefs(*expectedSchema);
        AssertPropertyCategoryDefs(*expectedSchema);
        AssertUnitSystemDefs(*expectedSchema);
        AssertPhenomenonDefs(*expectedSchema);
        AssertUnitDefs(*expectedSchema);
        AssertFormatDefs(*expectedSchema);
        actualSchemaCount++;
        }

    bvector<ECSchemaCP> expectedSchemas = m_ecdb.Schemas().GetSchemas(false);
    ASSERT_EQ((int) expectedSchemas.size(), actualSchemaCount);
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbMetaSchemaECSqlTestFixture::AssertSchemaDef(ECSchemaCR expectedSchema, ECSqlStatement const& actualSchemaDefRow)
    {
    const int colCount = actualSchemaDefRow.GetColumnCount();
    for (int i = 0; i < colCount; i++)
        {
        IECSqlValue const& val = actualSchemaDefRow.GetValue(i);
        ECSqlColumnInfoCR colInfo = val.GetColumnInfo();

        ECPropertyCP colInfoProp = colInfo.GetProperty();
        ASSERT_TRUE(colInfoProp != nullptr) << "ECSchemaDef is expected to not contain array props";

        Utf8StringCR colName = colInfoProp->GetName();

        if (colName.EqualsI("ECInstanceId"))
            {
            ASSERT_EQ(expectedSchema.GetId().GetValue(), val.GetId<ECSchemaId>().GetValue()) << "ECSchemaDef.ECInstanceId";
            ASSERT_STREQ("Id", colInfo.GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str()) << "ECSchemaDef.ECInstanceId";
            }
        else if (colName.EqualsI("ECClassId"))
            {
            ASSERT_EQ(m_ecdb.Schemas().GetClass("ECDbMeta", "ECSchemaDef")->GetId(), val.GetId<ECClassId>()) << "ECSchemaDef.ECClassId";
            ASSERT_STREQ("ClassId", colInfo.GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str()) << "ECSchemaDef.ECClassId";
            }
        else if (colName.EqualsI("Name"))
            ASSERT_STREQ(expectedSchema.GetName().c_str(), val.GetText()) << "ECSchemaDef.Name";
        else if (colName.EqualsI("DisplayLabel"))
            {
            if (expectedSchema.GetIsDisplayLabelDefined())
                ASSERT_STREQ(expectedSchema.GetInvariantDisplayLabel().c_str(), val.GetText()) << "ECSchemaDef.DisplayLabel";
            else
                ASSERT_TRUE(val.IsNull()) << "ECSchemaDef.DisplayLabel";
            }
        else if (colName.EqualsI("Description"))
            {
            if (!expectedSchema.GetInvariantDescription().empty())
                ASSERT_STREQ(expectedSchema.GetInvariantDescription().c_str(), val.GetText()) << "ECSchemaDef.Description";
            else
                ASSERT_TRUE(val.IsNull()) << "ECSchemaDef.Description";

            continue;
            }
        else if (colName.EqualsI("Alias"))
            ASSERT_STREQ(expectedSchema.GetAlias().c_str(), val.GetText()) << "ECSchemaDef.Alias";
        else if (colName.EqualsI("VersionMajor"))
            ASSERT_EQ(expectedSchema.GetVersionRead(), (uint32_t) val.GetInt()) << "ECSchemaDef.VersionRead";
        else if (colName.EqualsI("VersionMinor"))
            ASSERT_EQ(expectedSchema.GetVersionMinor(), (uint32_t) val.GetInt()) << "ECSchemaDef.VersionMinor";
        else if (colName.EqualsI("VersionWrite"))
            ASSERT_EQ(expectedSchema.GetVersionWrite(), (uint32_t) val.GetInt()) << "ECSchemaDef.VersionWrite";
        else if (colName.EqualsI("OriginalECXmlVersionMajor"))
            ASSERT_EQ(expectedSchema.GetOriginalECXmlVersionMajor(), (uint32_t) val.GetInt()) << "ECSchemaDef.OriginalECXmlVersionMajor";
        else if (colName.EqualsI("OriginalECXmlVersionMinor"))
            ASSERT_EQ(expectedSchema.GetOriginalECXmlVersionMinor(), (uint32_t) val.GetInt()) << "ECSchemaDef.OriginalECXmlVersionMinor";
        else
            FAIL() << "ECProperty ECSchemaDef." << colName.c_str() << " not tested. Test needs to be adjusted";
        }
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbMetaSchemaECSqlTestFixture::AssertClassDefs(ECSchemaCR expectedSchema)
    {
    ECSqlStatement classStatement;
    ASSERT_EQ(ECSqlStatus::Success, classStatement.Prepare(m_ecdb, "SELECT c.Name, c.* FROM meta.ECSchemaDef s "
                                                           "JOIN meta.ECClassDef c USING meta.SchemaOwnsClasses "
                                                           "WHERE s.Name=?"));

    ASSERT_EQ(ECSqlStatus::Success, classStatement.BindText(1, expectedSchema.GetName().c_str(), IECSqlBinder::MakeCopy::No));

    int actualClassCount = 0;
    while (BE_SQLITE_ROW == classStatement.Step())
        {
        Utf8CP actualClassName = classStatement.GetValueText(0);
        ECClassCP expectedClass = expectedSchema.GetClassCP(actualClassName);
        ASSERT_TRUE(expectedClass != nullptr);

        AssertClassDef(*expectedClass, classStatement);
        AssertBaseClasses(*expectedClass);
        AssertPropertyDefs(*expectedClass);
        actualClassCount++;
        }

    ASSERT_EQ((int) expectedSchema.GetClassCount(), actualClassCount);
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbMetaSchemaECSqlTestFixture::AssertClassDef(ECClassCR expectedClass, ECSqlStatement const& actualClassDefRow)
    {
    const ECClassType classType = expectedClass.GetClassType();
    ECCustomAttributeClassCP expectedCAClass = expectedClass.GetCustomAttributeClassCP();
    ECRelationshipClassCP expectedRelClass = expectedClass.GetRelationshipClassCP();
    
    ECClassId schemaOwnsClassesRelClassId = m_ecdb.Schemas().GetClassId("ECDbMeta", "SchemaOwnsClasses");
    ASSERT_TRUE(schemaOwnsClassesRelClassId.IsValid());

    const int colCount = actualClassDefRow.GetColumnCount();
    for (int i = 0; i < colCount; i++)
        {
        IECSqlValue const& val = actualClassDefRow.GetValue(i);
        ECSqlColumnInfoCR colInfo = val.GetColumnInfo();

        ECPropertyCP colInfoProp = colInfo.GetProperty();
        ASSERT_TRUE(colInfoProp != nullptr) << "ECClassDef is expected to not contain array props";

        Utf8StringCR colName = colInfoProp->GetName();

        if (colName.EqualsI("ECInstanceId"))
            {
            ASSERT_EQ(expectedClass.GetId().GetValue(), val.GetId<ECClassId>().GetValue()) << "ECClassDef.ECInstanceId";
            ASSERT_STREQ("Id", colInfo.GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str()) << "ECClassDef.ECInstanceId";
            continue;
            }

        if (colName.EqualsI("ECClassId"))
            {
            ASSERT_EQ(m_ecdb.Schemas().GetClass("ECDbMeta", "ECClassDef")->GetId(), val.GetId<ECClassId>()) << "ECClassDef.ECClassId";
            ASSERT_STREQ("ClassId", colInfo.GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str()) << "ECClassDef.ECClassId";
            continue;
            }

        if (colName.EqualsI("Schema"))
            {
            ECClassId actualRelClassId;
            ASSERT_EQ(expectedClass.GetSchema().GetId().GetValue(), val.GetNavigation(&actualRelClassId).GetValueUnchecked()) << "ECClassDef.Schema";
            ASSERT_EQ(schemaOwnsClassesRelClassId.GetValue(), actualRelClassId.GetValueUnchecked());
            continue;
            }

        if (colName.EqualsI("Name"))
            {
            ASSERT_STREQ(expectedClass.GetName().c_str(), val.GetText()) << "ECClassDef.Name";
            continue;
            }

        if (colName.EqualsI("DisplayLabel"))
            {
            if (expectedClass.GetIsDisplayLabelDefined())
                ASSERT_STREQ(expectedClass.GetInvariantDisplayLabel().c_str(), val.GetText()) << "ECClassDef.DisplayLabel";
            else
                ASSERT_TRUE(val.IsNull()) << "ECClassDef.DisplayLabel";

            continue;
            }

        if (colName.EqualsI("Description"))
            {
            if (!expectedClass.GetInvariantDescription().empty())
                ASSERT_STREQ(expectedClass.GetInvariantDescription().c_str(), val.GetText()) << "ECClassDef.Description";
            else
                ASSERT_TRUE(val.IsNull()) << "ECClassDef.Description";

            continue;
            }

        if (colName.EqualsI("Type"))
            {
            ASSERT_EQ((int) expectedClass.GetClassType(), val.GetInt()) << "ECClassDef.Type";
            ASSERT_EQ((int) expectedClass.GetClassType(), val.GetEnum()->GetInteger()) << "ECClassDef.Type";
            continue;
            }

        if (colName.EqualsI("Modifier"))
            {
            ASSERT_EQ((int) expectedClass.GetClassModifier(), val.GetInt()) << "ECClassDef.Modifier";
            ASSERT_EQ((int) expectedClass.GetClassModifier(), val.GetEnum()->GetInteger()) << "ECClassDef.Modifier";
            continue;
            }

        if (colName.EqualsI("CustomAttributeContainerType"))
            {
            if (classType == ECClassType::CustomAttribute)
                {
                ASSERT_TRUE(expectedCAClass != nullptr);
                if (expectedCAClass->GetContainerType() == CustomAttributeContainerType::Any)
                    ASSERT_TRUE(val.IsNull()) << "ECClassDef.CustomAttributeContainerType == Any";
                else
                    {
                    ASSERT_EQ((int) expectedCAClass->GetContainerType(), val.GetInt()) << "ECClassDef.CustomAttributeContainerType for CA classes";
                    //IECSqlValue::GetEnum cannot be used here as the values can be OR'ed together
                    }
                }
            else
                ASSERT_TRUE(val.IsNull()) << "ECClassDef.CustomAttributeContainerType for non-CA classes";

            continue;
            }

        if (colName.EqualsI("RelationshipStrength"))
            {
            if (classType == ECClassType::Relationship)
                {
                ASSERT_TRUE(expectedRelClass != nullptr);
                ASSERT_EQ((int) expectedRelClass->GetStrength(), val.GetInt()) << "ECClassDef.RelationshipStrength for relationship class";
                ASSERT_EQ((int) expectedRelClass->GetStrength(), val.GetEnum()->GetInteger()) << "ECClassDef.RelationshipStrength for relationship class";

                }
            else
                ASSERT_TRUE(val.IsNull()) << "ECClassDef.RelationshipStrength for non-relationship class";

            continue;
            }

        if (colName.EqualsI("RelationshipStrengthDirection"))
            {
            if (classType == ECClassType::Relationship)
                {
                ASSERT_TRUE(expectedRelClass != nullptr);
                ASSERT_EQ((int) expectedRelClass->GetStrengthDirection(), val.GetInt()) << "ECClassDef.RelationshipStrengthDirection for relationship class";
                ASSERT_EQ((int) expectedRelClass->GetStrengthDirection(), val.GetEnum()->GetInteger()) << "ECClassDef.RelationshipStrengthDirection for relationship class";

                }
            else
                ASSERT_TRUE(val.IsNull()) << "ECClassDef.RelationshipStrengthDirection for non-relationship class";

            continue;
            }

        FAIL() << "ECProperty ECClassDef." << colName.c_str() << " not tested. Test needs to be adjusted";
        }
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbMetaSchemaECSqlTestFixture::AssertBaseClasses(ECClassCR expectedClass)
    {
    ECSqlStatement directBaseClassStatement;
    ASSERT_EQ(ECSqlStatus::Success, directBaseClassStatement.Prepare(m_ecdb, "SELECT TargetECInstanceId FROM meta.ClassHasBaseClasses WHERE SourceECInstanceId=? ORDER BY Ordinal"));
    ASSERT_EQ(ECSqlStatus::Success, directBaseClassStatement.BindId(1, expectedClass.GetId()));

    int ordinal = 0;
    const int expectedBaseClassCount = (int) expectedClass.GetBaseClasses().size();
    while (BE_SQLITE_ROW == directBaseClassStatement.Step())
        {
        ASSERT_LT(ordinal, expectedBaseClassCount);
        ASSERT_EQ(expectedClass.GetBaseClasses()[ordinal]->GetId().GetValue(), directBaseClassStatement.GetValueId<ECInstanceId>(0).GetValue());
        ordinal++;
        }

    ASSERT_EQ(ordinal, expectedBaseClassCount);
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbMetaSchemaECSqlTestFixture::AssertEnumerationDefs(ECSchemaCR expectedSchema)
    {
    ECSqlStatement enumStatement;
    ASSERT_EQ(ECSqlStatus::Success, enumStatement.Prepare(m_ecdb, "SELECT e.Name, e.* FROM meta.ECSchemaDef s "
                                                           "JOIN meta.ECEnumerationDef e USING meta.SchemaOwnsEnumerations "
                                                           "WHERE s.Name=?"));

    ASSERT_EQ(ECSqlStatus::Success, enumStatement.BindText(1, expectedSchema.GetName().c_str(), IECSqlBinder::MakeCopy::No));

    int actualEnumCount = 0;
    while (BE_SQLITE_ROW == enumStatement.Step())
        {
        Utf8CP actualEnumName = enumStatement.GetValueText(0);
        ECEnumerationCP expectedEnum = expectedSchema.GetEnumerationCP(actualEnumName);
        ASSERT_TRUE(expectedEnum != nullptr);

        AssertEnumerationDef(*expectedEnum, enumStatement);
        actualEnumCount++;
        }

    ASSERT_EQ((int) expectedSchema.GetEnumerationCount(), actualEnumCount);
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbMetaSchemaECSqlTestFixture::AssertEnumerationDef(ECEnumerationCR expectedEnum, ECSqlStatement const& actualEnumerationDefRow)
    {
    ECClassId schemaOwnsEnumsRelClassId = m_ecdb.Schemas().GetClassId("ECDbMeta", "SchemaOwnsEnumerations");
    ASSERT_TRUE(schemaOwnsEnumsRelClassId.IsValid());

    const int colCount = actualEnumerationDefRow.GetColumnCount();
    for (int i = 0; i < colCount; i++)
        {
        IECSqlValue const& val = actualEnumerationDefRow.GetValue(i);
        ECSqlColumnInfoCR colInfo = val.GetColumnInfo();

        ECPropertyCP colInfoProp = colInfo.GetProperty();
        ASSERT_TRUE(colInfoProp != nullptr);

        Utf8StringCR colName = colInfoProp->GetName();

        if (colName.EqualsI("ECInstanceId"))
            {
            ASSERT_EQ(expectedEnum.GetId().GetValue(), val.GetId<ECEnumerationId>().GetValue()) << "ECEnumerationDef.ECInstanceId";
            ASSERT_STREQ("Id", colInfo.GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str()) << "ECEnumerationDef.ECInstanceId";
            continue;
            }

        if (colName.EqualsI("ECClassId"))
            {
            ASSERT_EQ(m_ecdb.Schemas().GetClass("ECDbMeta", "ECEnumerationDef")->GetId(), val.GetId<ECClassId>()) << "ECEnumerationDef.ECClassId";
            ASSERT_STREQ("ClassId", colInfo.GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str()) << "ECEnumerationDef.ECClassId";
            continue;
            }


        if (colName.EqualsI("Schema"))
            {
            ECClassId actualRelClassId;
            ASSERT_EQ(expectedEnum.GetSchema().GetId().GetValue(), val.GetNavigation(&actualRelClassId).GetValueUnchecked()) << "ECEnumerationDef.Schema";
            ASSERT_EQ(schemaOwnsEnumsRelClassId.GetValue(), actualRelClassId.GetValueUnchecked()) << "ECEnumerationDef.Schema";
            continue;
            }

        if (colName.EqualsI("Name"))
            {
            ASSERT_STREQ(expectedEnum.GetName().c_str(), val.GetText()) << "ECEnumerationDef.Name";
            continue;
            }

        if (colName.EqualsI("DisplayLabel"))
            {
            if (expectedEnum.GetIsDisplayLabelDefined())
                ASSERT_STREQ(expectedEnum.GetInvariantDisplayLabel().c_str(), val.GetText()) << "ECEnumerationDef.DisplayLabel";
            else
                ASSERT_TRUE(val.IsNull()) << "ECEnumerationDef.DisplayLabel";

            continue;
            }

        if (colName.EqualsI("Description"))
            {
            if (!expectedEnum.GetInvariantDescription().empty())
                ASSERT_STREQ(expectedEnum.GetInvariantDescription().c_str(), val.GetText()) << "ECEnumerationDef.Description";
            else
                ASSERT_TRUE(val.IsNull()) << "ECEnumerationDef.Description";

            continue;
            }

        if (colName.EqualsI("Type"))
            {
            ASSERT_EQ((int) expectedEnum.GetType(), val.GetInt()) << "ECEnumerationDef.Type";
            ASSERT_EQ((int) expectedEnum.GetType(), (int) val.GetEnum()->GetInteger()) << "ECEnumerationDef.Type";
            continue;
            }

        if (colName.EqualsI("IsStrict"))
            {
            ASSERT_EQ(expectedEnum.GetIsStrict(), val.GetBoolean()) << "ECEnumerationDef.IsStrict";
            continue;
            }

        if (colName.EqualsI("EnumValues"))
            {
            bvector<ECEnumeratorCP> expectedValues;
            for (ECEnumeratorCP expectedVal : expectedEnum.GetEnumerators())
                {
                expectedValues.push_back(expectedVal);
                }

            int actualValueCount = 0;
            for (IECSqlValue const& arrayElem : val.GetArrayIterable())
                {
                ECEnumeratorCP expectedValue = expectedValues[(size_t) actualValueCount];
                ASSERT_TRUE(expectedValue != nullptr);
                AssertEnumerationValue(*expectedValue, arrayElem);
                actualValueCount++;
                }

            ASSERT_EQ((int) expectedEnum.GetEnumeratorCount(), actualValueCount);
            continue;
            }
        }
    }
    
//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbMetaSchemaECSqlTestFixture::AssertEnumerationValue(ECEnumeratorCR expectedEnumValue, IECSqlValue const& actualEnumValue)
    {
    for (IECSqlValue const& memberVal : actualEnumValue.GetStructIterable())
        {
        ASSERT_TRUE(memberVal.GetColumnInfo().GetProperty() != nullptr);
        Utf8StringCR memberName = memberVal.GetColumnInfo().GetProperty()->GetName();

        if (memberName.EqualsI("Name"))
            {
            ASSERT_STREQ(expectedEnumValue.GetName().c_str(), memberVal.GetText()) << "ECEnumerationDef.EnumValues[].Name";
            continue;
            }

        if (memberName.EqualsI("IntValue"))
            {
            if (expectedEnumValue.IsInteger())
                ASSERT_EQ(expectedEnumValue.GetInteger(), memberVal.GetInt()) << "ECEnumerationDef.EnumValues[].IntValue";
            else
                ASSERT_TRUE(memberVal.IsNull()) << "ECEnumerationDef.EnumValues[].IntValue for non-int values";

            continue;
            }

        if (memberName.EqualsI("StringValue"))
            {
            if (expectedEnumValue.IsString())
                ASSERT_STREQ(expectedEnumValue.GetString().c_str(), memberVal.GetText()) << "ECEnumerationDef.EnumValues[].StringValue";
            else
                ASSERT_TRUE(memberVal.IsNull()) << "ECEnumerationDef.EnumValues[].StringValue for non-string values";

            continue;
            }
        if (memberName.EqualsI("DisplayLabel"))
            {
            if (expectedEnumValue.GetIsDisplayLabelDefined())
                ASSERT_STREQ(expectedEnumValue.GetDisplayLabel().c_str(), memberVal.GetText()) << "ECEnumerationDef.EnumValues[].DisplayLabel";
            else
                ASSERT_TRUE(memberVal.IsNull()) << "ECEnumerationDef.EnumValues[].DisplayLabel if not defined";

            continue;
            }

        if (memberName.EqualsI("Description"))
            {
            if (!expectedEnumValue.GetDescription().empty())
                ASSERT_STREQ(expectedEnumValue.GetDescription().c_str(), memberVal.GetText()) << "ECEnumerationDef.EnumValues[].Description";
            else
                ASSERT_TRUE(memberVal.IsNull()) << "ECEnumerationDef.EnumValues[].Description if not defined";

            continue;
            }

        FAIL() << "Untested Struct member: " << memberName.c_str() << " Please adjust the test";
        }
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbMetaSchemaECSqlTestFixture::AssertKindOfQuantityDefs(ECSchemaCR expectedSchema)
    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT koq.Name, koq.* FROM meta.ECSchemaDef s "
                                                          "JOIN meta.KindOfQuantityDef koq USING meta.SchemaOwnsKindOfQuantities "
                                                          "WHERE s.Name=?"));

    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(1, expectedSchema.GetName().c_str(), IECSqlBinder::MakeCopy::No));

    int actualKoqCount = 0;
    while (BE_SQLITE_ROW == statement.Step())
        {
        Utf8CP actualKoqName = statement.GetValueText(0);
        KindOfQuantityCP expectedKoq = expectedSchema.GetKindOfQuantityCP(actualKoqName);
        ASSERT_TRUE(expectedKoq != nullptr);

        AssertKindOfQuantityDef(*expectedKoq, statement);
        actualKoqCount++;
        }

    ASSERT_EQ((int) expectedSchema.GetKindOfQuantityCount(), actualKoqCount);
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbMetaSchemaECSqlTestFixture::AssertKindOfQuantityDef(KindOfQuantityCR expectedKoq, ECSqlStatement const& actualKoqDefRow)
    {
    const int colCount = actualKoqDefRow.GetColumnCount();
    for (int i = 0; i < colCount; i++)
        {
        IECSqlValue const& val = actualKoqDefRow.GetValue(i);
        ECSqlColumnInfoCR colInfo = val.GetColumnInfo();

        ECPropertyCP colInfoProp = colInfo.GetProperty();
        ASSERT_TRUE(colInfoProp != nullptr);

        Utf8StringCR colName = colInfoProp->GetName();

        if (colName.EqualsI("ECInstanceId"))
            {
            ASSERT_EQ(expectedKoq.GetId().GetValue(), val.GetId<KindOfQuantityId>().GetValue()) << "KindOfQuantityDef.ECInstanceId";
            ASSERT_STREQ("Id", colInfo.GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str()) << "KindOfQuantityDef.ECInstanceId";
            continue;
            }

        if (colName.EqualsI("ECClassId"))
            {
            ASSERT_EQ(m_ecdb.Schemas().GetClass("ECDbMeta", "KindOfQuantityDef")->GetId(), val.GetId<ECClassId>()) << "KindOfQuantityDef.ECClassId";
            ASSERT_STREQ("ClassId", colInfo.GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str()) << "KindOfQuantityDef.ECClassId";
            continue;
            }

        if (colName.EqualsI("Schema"))
            {
            ECClassId actualRelClassId;
            ASSERT_EQ(expectedKoq.GetSchema().GetId().GetValue(), val.GetNavigation(&actualRelClassId).GetValueUnchecked()) << "KindOfQuantityDef.Schema";
            ASSERT_EQ(colInfoProp->GetAsNavigationProperty()->GetRelationshipClass()->GetId().GetValue(), actualRelClassId.GetValue()) << "KindOfQuantityDef.Schema";
            continue;
            }

        if (colName.EqualsI("Name"))
            {
            ASSERT_STREQ(expectedKoq.GetName().c_str(), val.GetText()) << "KindOfQuantityDef.Name";
            continue;
            }

        if (colName.EqualsI("DisplayLabel"))
            {
            if (expectedKoq.GetIsDisplayLabelDefined())
                ASSERT_STREQ(expectedKoq.GetInvariantDisplayLabel().c_str(), val.GetText()) << "KindOfQuantityDef.DisplayLabel";
            else
                ASSERT_TRUE(val.IsNull()) << "KindOfQuantityDef.DisplayLabel";

            continue;
            }

        if (colName.EqualsI("Description"))
            {
            if (!expectedKoq.GetInvariantDescription().empty())
                ASSERT_STREQ(expectedKoq.GetInvariantDescription().c_str(), val.GetText()) << "KindOfQuantityDef.Description";
            else
                ASSERT_TRUE(val.IsNull()) << "KindOfQuantityDef.Description";

            continue;
            }

        if (colName.EqualsI("RelativeError"))
            {
            ASSERT_DOUBLE_EQ(expectedKoq.GetRelativeError(), val.GetDouble()) << "KindOfQuantityDef.RelativeError";
            continue;
            }


        if (colName.EqualsI("PersistenceUnit"))
            {
            ASSERT_STREQ(expectedKoq.GetPersistenceUnit()->GetQualifiedName(expectedKoq.GetSchema()).c_str(), val.GetText()) << "KindOfQuantityDef.PersistenceUnit";
            continue;
            }

        if (colName.EqualsI("PresentationUnits"))
            {
            if (expectedKoq.GetPresentationFormats().empty())
                ASSERT_TRUE(val.IsNull()) << "KindOfQuantityDef.PresentationFormatList";
            else
                {
                ASSERT_EQ((int) expectedKoq.GetPresentationFormats().size(), val.GetArrayLength()) << "KindOfQuantityDef.PresentationFormatList";

                size_t i = 0;
                for (IECSqlValue const& arrayElementVal : val.GetArrayIterable())
                    {
                    ASSERT_STREQ(expectedKoq.GetPresentationFormats()[i].GetQualifiedFormatString(expectedKoq.GetSchema()).c_str(), arrayElementVal.GetText()) << "KindOfQuantityDef.PresentationFormatList";
                    i++;
                    }
                }

            continue;
            }

        FAIL() << "Untested KindOfQuantityDef property: " << colName.c_str() << " Please adjust the test";
        }
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbMetaSchemaECSqlTestFixture::AssertUnitSystemDefs(ECSchemaCR expectedSchema)
    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT us.Name, us.* FROM meta.ECSchemaDef s "
                                                      "JOIN meta.UnitSystemDef us USING meta.SchemaOwnsUnitSystems "
                                                      "WHERE s.Name=?"));

    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(1, expectedSchema.GetName().c_str(), IECSqlBinder::MakeCopy::No));

    int actualUsCount = 0;
    while (BE_SQLITE_ROW == statement.Step())
        {
        Utf8CP actualName = statement.GetValueText(0);
        UnitSystemCP expected = expectedSchema.GetUnitSystemCP(actualName);
        ASSERT_TRUE(expected != nullptr);

        AssertUnitSystemDef(*expected, statement);
        actualUsCount++;
        }

    ASSERT_EQ((int) expectedSchema.GetUnitSystemCount(), actualUsCount);
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbMetaSchemaECSqlTestFixture::AssertUnitSystemDef(UnitSystemCR expected, ECSqlStatement const& actualRow)
    {
    const int colCount = actualRow.GetColumnCount();
    for (int i = 0; i < colCount; i++)
        {
        IECSqlValue const& val = actualRow.GetValue(i);
        ECSqlColumnInfoCR colInfo = val.GetColumnInfo();

        ECPropertyCP colInfoProp = colInfo.GetProperty();
        ASSERT_TRUE(colInfoProp != nullptr);

        Utf8StringCR colName = colInfoProp->GetName();

        if (colName.EqualsI("ECInstanceId"))
            {
            ASSERT_EQ(expected.GetId().GetValue(), val.GetId<UnitSystemId>().GetValue()) << "UnitSystemDef.ECInstanceId";
            ASSERT_STREQ("Id", colInfo.GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str()) << "UnitSystemDef.ECInstanceId";
            continue;
            }

        if (colName.EqualsI("ECClassId"))
            {
            ASSERT_EQ(m_ecdb.Schemas().GetClass("ECDbMeta", "UnitSystemDef")->GetId(), val.GetId<ECClassId>()) << "UnitSystemDef.ECClassId";
            ASSERT_STREQ("ClassId", colInfo.GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str()) << "UnitSystemDef.ECClassId";
            continue;
            }

        if (colName.EqualsI("Schema"))
            {
            ECClassId actualRelClassId;
            ASSERT_EQ(expected.GetSchema().GetId().GetValue(), val.GetNavigation(&actualRelClassId).GetValueUnchecked()) << "UnitSystemDef.Schema";
            ASSERT_EQ(colInfoProp->GetAsNavigationProperty()->GetRelationshipClass()->GetId().GetValue(), actualRelClassId.GetValue()) << "UnitSystemDef.Schema";
            continue;
            }

        if (colName.EqualsI("Name"))
            {
            ASSERT_STREQ(expected.GetName().c_str(), val.GetText()) << "UnitSystemDef.Name";
            continue;
            }

        if (colName.EqualsI("DisplayLabel"))
            {
            if (expected.GetIsDisplayLabelDefined())
                ASSERT_STREQ(expected.GetInvariantDisplayLabel().c_str(), val.GetText()) << "UnitSystemDef.DisplayLabel";
            else
                ASSERT_TRUE(val.IsNull()) << "UnitSystemDef.DisplayLabel";

            continue;
            }

        if (colName.EqualsI("Description"))
            {
            if (!expected.GetInvariantDescription().empty())
                ASSERT_STREQ(expected.GetInvariantDescription().c_str(), val.GetText()) << "UnitSystemDef.Description";
            else
                ASSERT_TRUE(val.IsNull()) << "UnitSystemDef.Description";

            continue;
            }

        FAIL() << "Untested UnitSystemDef property: " << colName.c_str() << " Please adjust the test";
        }
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbMetaSchemaECSqlTestFixture::AssertPhenomenonDefs(ECSchemaCR expectedSchema)
    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ph.Name, ph.* FROM meta.ECSchemaDef s "
                                                      "JOIN meta.PhenomenonDef ph USING meta.SchemaOwnsPhenomena "
                                                      "WHERE s.Name=?"));

    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(1, expectedSchema.GetName().c_str(), IECSqlBinder::MakeCopy::No));

    int actualCount = 0;
    while (BE_SQLITE_ROW == statement.Step())
        {
        Utf8CP actualName = statement.GetValueText(0);
        PhenomenonCP expected = expectedSchema.GetPhenomenonCP(actualName);
        ASSERT_TRUE(expected != nullptr);

        AssertPhenomenonDef(*expected, statement);
        actualCount++;
        }

    ASSERT_EQ((int) expectedSchema.GetPhenomenonCount(), actualCount);
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbMetaSchemaECSqlTestFixture::AssertPhenomenonDef(PhenomenonCR expected, ECSqlStatement const& actualRow)
    {
    const int colCount = actualRow.GetColumnCount();
    for (int i = 0; i < colCount; i++)
        {
        IECSqlValue const& val = actualRow.GetValue(i);
        ECSqlColumnInfoCR colInfo = val.GetColumnInfo();

        ECPropertyCP colInfoProp = colInfo.GetProperty();
        ASSERT_TRUE(colInfoProp != nullptr);

        Utf8StringCR colName = colInfoProp->GetName();

        if (colName.EqualsI("ECInstanceId"))
            {
            ASSERT_EQ(expected.GetId().GetValue(), val.GetId<PhenomenonId>().GetValue()) << "PhenomenonDef.ECInstanceId";
            ASSERT_STREQ("Id", colInfo.GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str()) << "PhenomenonDef.ECInstanceId";
            continue;
            }

        if (colName.EqualsI("ECClassId"))
            {
            ASSERT_EQ(m_ecdb.Schemas().GetClass("ECDbMeta", "PhenomenonDef")->GetId(), val.GetId<ECClassId>()) << "PhenomenonDef.ECClassId";
            ASSERT_STREQ("ClassId", colInfo.GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str()) << "PhenomenonDef.ECClassId";
            continue;
            }

        if (colName.EqualsI("Schema"))
            {
            ECClassId actualRelClassId;
            ASSERT_EQ(expected.GetSchema().GetId().GetValue(), val.GetNavigation(&actualRelClassId).GetValueUnchecked()) << "PhenomenonDef.Schema";
            ASSERT_EQ(colInfoProp->GetAsNavigationProperty()->GetRelationshipClass()->GetId().GetValue(), actualRelClassId.GetValue()) << "PhenomenonDef.Schema";
            continue;
            }

        if (colName.EqualsI("Name"))
            {
            ASSERT_STREQ(expected.GetName().c_str(), val.GetText()) << "PhenomenonDef.Name";
            continue;
            }

        if (colName.EqualsI("DisplayLabel"))
            {
            if (expected.GetIsDisplayLabelDefined())
                ASSERT_STREQ(expected.GetInvariantDisplayLabel().c_str(), val.GetText()) << "PhenomenonDef.DisplayLabel";
            else
                ASSERT_TRUE(val.IsNull()) << "PhenomenonDef.DisplayLabel";

            continue;
            }

        if (colName.EqualsI("Description"))
            {
            if (!expected.GetInvariantDescription().empty())
                ASSERT_STREQ(expected.GetInvariantDescription().c_str(), val.GetText()) << "PhenomenonDef.Description";
            else
                ASSERT_TRUE(val.IsNull()) << "PhenomenonDef.Description";

            continue;
            }

        if (colName.EqualsI("Definition"))
            {
            ASSERT_STREQ(expected.GetDefinition().c_str(), val.GetText()) << "PhenomenonDef.Definition";
            continue;
            }

        FAIL() << "Untested PhenomenonDef property: " << colName.c_str() << " Please adjust the test";
        }
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbMetaSchemaECSqlTestFixture::AssertUnitDefs(ECSchemaCR expectedSchema)
    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT u.Name, u.* FROM meta.ECSchemaDef s "
                                                      "JOIN meta.UnitDef u USING meta.SchemaOwnsUnits "
                                                      "WHERE s.Name=?"));

    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(1, expectedSchema.GetName().c_str(), IECSqlBinder::MakeCopy::No));

    int actualCount = 0;
    while (BE_SQLITE_ROW == statement.Step())
        {
        Utf8CP actualName = statement.GetValueText(0);
        ECUnitCP expected = expectedSchema.GetUnitCP(actualName);
        ASSERT_TRUE(expected != nullptr);

        AssertUnitDef(*expected, statement);
        actualCount++;
        }

    ASSERT_EQ((int) expectedSchema.GetUnitCount(), actualCount);
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbMetaSchemaECSqlTestFixture::AssertUnitDef(ECUnitCR expected, ECSqlStatement const& actualRow)
    {
    const int colCount = actualRow.GetColumnCount();
    for (int i = 0; i < colCount; i++)
        {
        IECSqlValue const& val = actualRow.GetValue(i);
        ECSqlColumnInfoCR colInfo = val.GetColumnInfo();

        ECPropertyCP colInfoProp = colInfo.GetProperty();
        ASSERT_TRUE(colInfoProp != nullptr);

        Utf8StringCR colName = colInfoProp->GetName();

        if (colName.EqualsI("ECInstanceId"))
            {
            ASSERT_EQ(expected.GetId().GetValue(), val.GetId<UnitId>().GetValue()) << "UnitDef.ECInstanceId of " << expected.GetFullName();
            ASSERT_STREQ("Id", colInfo.GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str()) << "UnitDef.ECInstanceId of " << expected.GetFullName();
            continue;
            }

        if (colName.EqualsI("ECClassId"))
            {
            ASSERT_EQ(m_ecdb.Schemas().GetClass("ECDbMeta", "UnitDef")->GetId(), val.GetId<ECClassId>()) << "UnitDef.ECClassId of " << expected.GetFullName();
            ASSERT_STREQ("ClassId", colInfo.GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str()) << "UnitDef.ECClassId of " << expected.GetFullName();
            continue;
            }

        if (colName.EqualsI("Schema"))
            {
            ECClassId actualRelClassId;
            ASSERT_EQ(expected.GetSchema().GetId().GetValue(), val.GetNavigation(&actualRelClassId).GetValueUnchecked()) << "UnitDef.Schema of " << expected.GetFullName();
            ASSERT_EQ(colInfoProp->GetAsNavigationProperty()->GetRelationshipClass()->GetId().GetValue(), actualRelClassId.GetValue()) << "UnitDef.Schema of " << expected.GetFullName();
            continue;
            }

        if (colName.EqualsI("Name"))
            {
            ASSERT_STREQ(expected.GetName().c_str(), val.GetText()) << "UnitDef.Name of " << expected.GetFullName();
            continue;
            }

        if (colName.EqualsI("DisplayLabel"))
            {
            if (expected.GetIsDisplayLabelDefined())
                ASSERT_STREQ(expected.GetInvariantDisplayLabel().c_str(), val.GetText()) << "UnitDef.DisplayLabel of " << expected.GetFullName();
            else
                ASSERT_TRUE(val.IsNull()) << "UnitDef.DisplayLabel of " << expected.GetFullName();

            continue;
            }

        if (colName.EqualsI("Description"))
            {
            if (!expected.GetInvariantDescription().empty())
                ASSERT_STREQ(expected.GetInvariantDescription().c_str(), val.GetText()) << "UnitDef.Description of " << expected.GetFullName();
            else
                ASSERT_TRUE(val.IsNull()) << "UnitDef.Description of " << expected.GetFullName();

            continue;
            }

        if (colName.EqualsI("UnitSystem"))
            {
            if (expected.HasUnitSystem())
                {
                ECClassId actualRelClassId;
                ASSERT_EQ(static_cast<ECN::UnitSystemCP>(expected.GetUnitSystem())->GetId().GetValue(), val.GetNavigation(&actualRelClassId).GetValueUnchecked()) << "UnitDef.UnitSystem of " << expected.GetFullName();
                ASSERT_EQ(colInfoProp->GetAsNavigationProperty()->GetRelationshipClass()->GetId().GetValue(), actualRelClassId.GetValue()) << "UnitDef.UnitSystem of " << expected.GetFullName();
                }
            else
                ASSERT_TRUE(val.IsNull()) << "UnitDef.UnitSystem of " << expected.GetFullName();

            continue;
            }

        if (colName.EqualsI("Phenomenon"))
            {
            ECClassId actualRelClassId;
            ASSERT_EQ(static_cast<ECN::PhenomenonCP>(expected.GetPhenomenon())->GetId().GetValue(), val.GetNavigation(&actualRelClassId).GetValueUnchecked()) << "UnitDef.Phenomenon of " << expected.GetFullName();
            ASSERT_EQ(colInfoProp->GetAsNavigationProperty()->GetRelationshipClass()->GetId().GetValue(), actualRelClassId.GetValue()) << "UnitDef.Phenomenon of " << expected.GetFullName();
            continue;
            }

        if (colName.EqualsIAscii("IsConstant"))
            {
            ASSERT_EQ(expected.IsConstant(), val.GetBoolean()) << "UnitDef.IsConstant of " << expected.GetFullName();
            continue;
            }

        if (colName.EqualsI("Definition"))
            {
            if (!expected.HasDefinition())
                ASSERT_TRUE(val.IsNull()) << "UnitDef.Definition of " << expected.GetFullName();
            else
                ASSERT_STREQ(expected.GetDefinition().c_str(), val.GetText()) << "UnitDef.Definition of " << expected.GetFullName();
            
            continue;
            }

        if (colName.EqualsI("Numerator"))
            {
            if (expected.HasNumerator())
                ASSERT_DOUBLE_EQ(expected.GetNumerator(), val.GetDouble()) << "UnitDef.Numerator of " << expected.GetFullName();
            else
                ASSERT_TRUE(val.IsNull()) << "UnitDef.Numerator of " << expected.GetFullName();

            continue;
            }

        if (colName.EqualsI("Denominator"))
            {
            if (expected.HasDenominator())
                ASSERT_DOUBLE_EQ(expected.GetDenominator(), val.GetDouble()) << "UnitDef.Denominator of " << expected.GetFullName();
            else
                ASSERT_TRUE(val.IsNull()) << "UnitDef.Denominator of " << expected.GetFullName();

            continue;
            }

        if (colName.EqualsI("Offset"))
            {
            if (expected.HasOffset())
                ASSERT_DOUBLE_EQ(expected.GetOffset(), val.GetDouble()) << "UnitDef.Offset of " << expected.GetFullName();
            else
                ASSERT_TRUE(val.IsNull()) << "UnitDef.Offset of " << expected.GetFullName();

            continue;
            }

        if (colName.EqualsI("InvertingUnit"))
            {
            if (expected.IsInvertedUnit())
                {
                ECClassId actualRelClassId;
                ASSERT_EQ(expected.GetInvertingUnit()->GetId().GetValue(), val.GetNavigation(&actualRelClassId).GetValueUnchecked()) << "UnitDef.InvertingUnit of " << expected.GetFullName();
                ASSERT_EQ(colInfoProp->GetAsNavigationProperty()->GetRelationshipClass()->GetId().GetValue(), actualRelClassId.GetValue()) << "UnitDef.InvertingUnit of " << expected.GetFullName();
                }
            else
                ASSERT_TRUE(val.IsNull());

            continue;
            }

        FAIL() << "Untested UnitDef property: " << colName.c_str() << " Please adjust the test";
        }
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbMetaSchemaECSqlTestFixture::AssertFormatDefs(ECSchemaCR expectedSchema)
    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT f.Name, f.* FROM meta.ECSchemaDef s "
                                                        "JOIN meta.FormatDef f USING meta.SchemaOwnsFormats "
                                                        "WHERE s.Name=?"));

    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(1, expectedSchema.GetName().c_str(), IECSqlBinder::MakeCopy::No));

    int actualCount = 0;
    while (BE_SQLITE_ROW == statement.Step())
        {
        Utf8CP actualName = statement.GetValueText(0);
        ECFormatCP expected = expectedSchema.GetFormatCP(actualName);
        ASSERT_TRUE(expected != nullptr);

        AssertFormatDef(*expected, statement);
        actualCount++;
        }

    ASSERT_EQ((int) expectedSchema.GetFormatCount(), actualCount);
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbMetaSchemaECSqlTestFixture::AssertFormatDef(ECFormatCR expected, ECSqlStatement const& actualRow)
    {
    const int colCount = actualRow.GetColumnCount();
    for (int i = 0; i < colCount; i++)
        {
        IECSqlValue const& val = actualRow.GetValue(i);
        ECSqlColumnInfoCR colInfo = val.GetColumnInfo();

        ECPropertyCP colInfoProp = colInfo.GetProperty();
        ASSERT_TRUE(colInfoProp != nullptr);

        Utf8StringCR colName = colInfoProp->GetName();

        if (colName.EqualsI("ECInstanceId"))
            {
            ASSERT_EQ(expected.GetId().GetValue(), val.GetId<UnitId>().GetValue()) << "FormatDef.ECInstanceId of " << expected.GetFullName();
            ASSERT_STREQ("Id", colInfo.GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str()) << "FormatDef.ECInstanceId of " << expected.GetFullName();
            continue;
            }

        if (colName.EqualsI("ECClassId"))
            {
            ASSERT_EQ(m_ecdb.Schemas().GetClass("ECDbMeta", "FormatDef")->GetId(), val.GetId<ECClassId>()) << "FormatDef.ECClassId of " << expected.GetFullName();
            ASSERT_STREQ("ClassId", colInfo.GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str()) << "FormatDef.ECClassId of " << expected.GetFullName();
            continue;
            }

        if (colName.EqualsI("Schema"))
            {
            ECClassId actualRelClassId;
            ASSERT_EQ(expected.GetSchema().GetId().GetValue(), val.GetNavigation(&actualRelClassId).GetValueUnchecked()) << "FormatDef.Schema of " << expected.GetFullName();
            ASSERT_EQ(colInfoProp->GetAsNavigationProperty()->GetRelationshipClass()->GetId().GetValue(), actualRelClassId.GetValue()) << "FormatDef.Schema of " << expected.GetFullName();
            continue;
            }

        if (colName.EqualsI("Name"))
            {
            ASSERT_STREQ(expected.GetName().c_str(), val.GetText()) << "FormatDef.Name of " << expected.GetFullName();
            continue;
            }

        if (colName.EqualsI("DisplayLabel"))
            {
            if (expected.GetIsDisplayLabelDefined())
                ASSERT_STREQ(expected.GetInvariantDisplayLabel().c_str(), val.GetText()) << "FormatDef.DisplayLabel of " << expected.GetFullName();
            else
                ASSERT_TRUE(val.IsNull()) << "FormatDef.DisplayLabel of " << expected.GetFullName();

            continue;
            }

        if (colName.EqualsI("Description"))
            {
            if (!expected.GetInvariantDescription().empty())
                ASSERT_STREQ(expected.GetInvariantDescription().c_str(), val.GetText()) << "FormatDef.Description of " << expected.GetFullName();
            else
                ASSERT_TRUE(val.IsNull()) << "FormatDef.Description of " << expected.GetFullName();

            continue;
            }

        if (colName.EqualsI("NumericSpec"))
            {
            if (!expected.HasNumeric())
                ASSERT_TRUE(val.IsNull()) << "FormatDef.NumericSpec of " << expected.GetFullName();
            else
                {
                Json::Value jval;
                ASSERT_TRUE(expected.GetNumericSpec()->ToJson(jval, false));
                ASSERT_STREQ(jval.ToString().c_str(), val.GetText()) << "FormatDef.NumericSpec of " << expected.GetFullName();
                }

            continue;
            }

        if (colName.EqualsI("CompositeSpec"))
            {
            if (!expected.HasComposite())
                ASSERT_TRUE(val.IsNull()) << "FormatDef.CompositeSpec of " << expected.GetFullName();
            else
                {
                Json::Value jval;
                ASSERT_TRUE(expected.GetCompositeSpec()->ToJson(jval, false, true));
                ASSERT_STREQ(jval.ToString().c_str(), val.GetText()) << "FormatDef.CompositeSpec of " << expected.GetFullName();
                }

            continue;
            }

        FAIL() << "Untested FormatDef property: " << colName.c_str() << " Please adjust the test";
        }

    //now check the composite units
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT fcu.Label, fcu.Unit.Id FROM meta.FormatCompositeUnitDef fcu WHERE fcu.Format.Id=? ORDER BY fcu.Ordinal"));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindId(1, expected.GetId()));

    const bool expectedHasComposite = expected.HasComposite();
    Formatting::CompositeValueSpecCP expectedCompSpec = expected.GetCompositeSpec();
    const int expectedCompositeUnitCount = expectedHasComposite ? (int) expectedCompSpec->GetUnitCount() : 0;
    int actualCompositeUnitCount = 0;
    while (BE_SQLITE_ROW == statement.Step())
        {
        Utf8CP actualLabel = statement.IsValueNull(0) ? nullptr : statement.GetValueText(0);
        UnitId actualUnitId;
        if (!statement.IsValueNull(1))
            actualUnitId = statement.GetValueId<UnitId>(1);

        ASSERT_TRUE(expectedHasComposite) << "Actual Format has at least one composite unit";
        ASSERT_LT(actualCompositeUnitCount, expectedCompositeUnitCount);

        switch (actualCompositeUnitCount)
            {
                case 0:
                {
                if (expectedCompSpec->HasMajorLabel())
                    ASSERT_STREQ(expectedCompSpec->GetMajorLabel().c_str(), actualLabel) << "FormatCompositeUnitDef.MajorLabel of " << expected.GetFullName();
                else
                    ASSERT_TRUE(Utf8String::IsNullOrEmpty(actualLabel)) << "FormatCompositeUnitDef.MajorLabel of " << expected.GetFullName();

                ASSERT_EQ(((ECUnitCP) expectedCompSpec->GetMajorUnit())->GetId(), actualUnitId) << "FormatCompositeUnitDef.MajorUnit of " << expected.GetFullName();
                break;
                }
                case 1:
                {
                if (expectedCompSpec->HasMiddleLabel())
                    ASSERT_STREQ(expectedCompSpec->GetMiddleLabel().c_str(), actualLabel) << "FormatCompositeUnitDef.MiddleLabel of " << expected.GetFullName();
                else
                    ASSERT_TRUE(Utf8String::IsNullOrEmpty(actualLabel)) << "FormatCompositeUnitDef.MiddleLabel of " << expected.GetFullName();

                ASSERT_EQ(((ECUnitCP) expectedCompSpec->GetMiddleUnit())->GetId(), actualUnitId) << "FormatCompositeUnitDef.MiddleUnit of " << expected.GetFullName();
                break;
                }
                case 2:
                {
                if (expectedCompSpec->HasMinorLabel())
                    ASSERT_STREQ(expectedCompSpec->GetMinorLabel().c_str(), actualLabel) << "FormatCompositeUnitDef.MinorLabel of " << expected.GetFullName();
                else
                    ASSERT_TRUE(Utf8String::IsNullOrEmpty(actualLabel)) << "FormatCompositeUnitDef.MinorLabel of " << expected.GetFullName();

                ASSERT_EQ(((ECUnitCP) expectedCompSpec->GetMinorUnit())->GetId(), actualUnitId) << "FormatCompositeUnitDef.MinorUnit of " << expected.GetFullName();
                break;
                }
                case 3:
                {
                if (expectedCompSpec->HasSubLabel())
                    ASSERT_STREQ(expectedCompSpec->GetSubLabel().c_str(), actualLabel) << "FormatCompositeUnitDef.SubLabel of " << expected.GetFullName();
                else
                    ASSERT_TRUE(Utf8String::IsNullOrEmpty(actualLabel)) << "FormatCompositeUnitDef.SubLabel of " << expected.GetFullName();

                ASSERT_EQ(((ECUnitCP) expectedCompSpec->GetSubUnit())->GetId(), actualUnitId) << "FormatCompositeUnitDef.SubUnit of " << expected.GetFullName();
                break;
                }
                default:
                    FAIL() << "FormatDef has more than 4 composite units";
            }

        actualCompositeUnitCount++;
        }

    ASSERT_EQ(expectedCompositeUnitCount, actualCompositeUnitCount);
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbMetaSchemaECSqlTestFixture::AssertPropertyCategoryDefs(ECSchemaCR expectedSchema)
    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT cat.Name, cat.* FROM meta.ECSchemaDef s "
                                                      "JOIN meta.PropertyCategoryDef cat USING meta.SchemaOwnsPropertyCategories "
                                                      "WHERE s.Name=?"));

    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(1, expectedSchema.GetName().c_str(), IECSqlBinder::MakeCopy::No));

    int actualCatCount = 0;
    while (BE_SQLITE_ROW == statement.Step())
        {
        Utf8CP actualCatName = statement.GetValueText(0);
        PropertyCategoryCP expectedCat = expectedSchema.GetPropertyCategoryCP(actualCatName);
        ASSERT_TRUE(expectedCat != nullptr);

        AssertPropertyCategoryDef(*expectedCat, statement);
        actualCatCount++;
        }

    ASSERT_EQ((int) expectedSchema.GetPropertyCategoryCount(), actualCatCount);
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbMetaSchemaECSqlTestFixture::AssertPropertyCategoryDef(PropertyCategoryCR expectedCat, ECSqlStatement const& actualCatDefRow)
    {
    const int colCount = actualCatDefRow.GetColumnCount();
    for (int i = 0; i < colCount; i++)
        {
        IECSqlValue const& val = actualCatDefRow.GetValue(i);
        ECSqlColumnInfoCR colInfo = val.GetColumnInfo();

        ECPropertyCP colInfoProp = colInfo.GetProperty();
        ASSERT_TRUE(colInfoProp != nullptr);

        Utf8StringCR colName = colInfoProp->GetName();

        if (colName.EqualsI("ECInstanceId"))
            {
            ASSERT_EQ(expectedCat.GetId().GetValue(), val.GetId<PropertyCategoryId>().GetValue()) << "PropertyCategoryDef.ECInstanceId";
            ASSERT_STREQ("Id", colInfo.GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str()) << "PropertyCategoryDef.ECInstanceId";
            continue;
            }

        if (colName.EqualsI("ECClassId"))
            {
            ASSERT_EQ(m_ecdb.Schemas().GetClass("ECDbMeta", "PropertyCategoryDef")->GetId(), val.GetId<ECClassId>()) << "PropertyCategoryDef.ECClassId";
            ASSERT_STREQ("ClassId", colInfo.GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str()) << "PropertyCategoryDef.ECClassId";
            continue;
            }

        if (colName.EqualsI("Schema"))
            {
            ECClassId actualRelClassId;
            ASSERT_EQ(expectedCat.GetSchema().GetId().GetValue(), val.GetNavigation(&actualRelClassId).GetValueUnchecked()) << "PropertyCategoryDef.Schema";
            ASSERT_EQ(colInfoProp->GetAsNavigationProperty()->GetRelationshipClass()->GetId().GetValue(), actualRelClassId.GetValue()) << "PropertyCategoryDef.Schema";
            continue;
            }

        if (colName.EqualsI("Name"))
            {
            ASSERT_STREQ(expectedCat.GetName().c_str(), val.GetText()) << "PropertyCategoryDef.Name";
            continue;
            }

        if (colName.EqualsI("DisplayLabel"))
            {
            if (expectedCat.GetIsDisplayLabelDefined())
                ASSERT_STREQ(expectedCat.GetInvariantDisplayLabel().c_str(), val.GetText()) << "PropertyCategoryDef.DisplayLabel";
            else
                ASSERT_TRUE(val.IsNull()) << "PropertyCategoryDef.DisplayLabel";

            continue;
            }

        if (colName.EqualsI("Description"))
            {
            if (!expectedCat.GetInvariantDescription().empty())
                ASSERT_STREQ(expectedCat.GetInvariantDescription().c_str(), val.GetText()) << "PropertyCategoryDef.Description";
            else
                ASSERT_TRUE(val.IsNull()) << "PropertyCategoryDef.Description";

            continue;
            }

        if (colName.EqualsI("Priority"))
            {
            ASSERT_EQ((int) expectedCat.GetPriority(), val.GetInt()) << "PropertyCategoryDef.Priority";
            continue;
            }
        }
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbMetaSchemaECSqlTestFixture::AssertPropertyDefs(ECClassCR expectedClass)
    {
    ECSqlStatement propStatement;
    ASSERT_EQ(ECSqlStatus::Success, propStatement.Prepare(m_ecdb, "SELECT p.Name, p.* FROM meta.ECPropertyDef p "
                                                           "JOIN meta.ECClassDef c USING meta.ClassOwnsLocalProperties "
                                                           "JOIN meta.ECSchemaDef s USING meta.SchemaOwnsClasses "
                                                           "WHERE s.Name=? AND c.Name=? ORDER BY p.Ordinal"));

    ASSERT_EQ(ECSqlStatus::Success, propStatement.BindText(1, expectedClass.GetSchema().GetName().c_str(), IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, propStatement.BindText(2, expectedClass.GetName().c_str(), IECSqlBinder::MakeCopy::No));

    int actualPropCount = 0;
    while (BE_SQLITE_ROW == propStatement.Step())
        {
        Utf8CP actualPropName = propStatement.GetValueText(0);
        ECPropertyCP expectedProp = expectedClass.GetPropertyP(actualPropName);
        ASSERT_TRUE(expectedProp != nullptr);
        AssertPropertyDef(*expectedProp, propStatement);
        actualPropCount++;
        }

    //right now only local properties can be retrieved from the ECDbMeta schema
    ASSERT_EQ((int) expectedClass.GetPropertyCount(false), actualPropCount) << expectedClass.GetFullName();
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbMetaSchemaECSqlTestFixture::AssertPropertyDef(ECPropertyCR expectedProp, ECSqlStatement const& actualPropertyDefRow)
    {
    PrimitiveECPropertyCP primProp = expectedProp.GetAsPrimitiveProperty();
    StructECPropertyCP structProp = expectedProp.GetAsStructProperty();
    PrimitiveArrayECPropertyCP primArrayProp = expectedProp.GetAsPrimitiveArrayProperty();
    StructArrayECPropertyCP structArrayProp = expectedProp.GetAsStructArrayProperty();
    NavigationECPropertyCP navProp = expectedProp.GetAsNavigationProperty();

    enum class ECPropertyKind
        {
        Primitive = 0,
        Struct = 1,
        PrimitiveArray = 2,
        StructArray = 3,
        Navigation = 4
        };

    const int colCount = actualPropertyDefRow.GetColumnCount();
    for (int i = 0; i < colCount; i++)
        {
        IECSqlValue const& val = actualPropertyDefRow.GetValue(i);
        ECSqlColumnInfoCR colInfo = val.GetColumnInfo();

        ECPropertyCP colInfoProp = colInfo.GetProperty();
        ASSERT_TRUE(colInfoProp != nullptr);

        Utf8StringCR colName = colInfoProp->GetName();

        if (colName.EqualsI("ECInstanceId"))
            {
            ASSERT_EQ(expectedProp.GetId().GetValue(), val.GetId<ECPropertyId>().GetValue()) << "ECPropertyDef.ECInstanceId for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
            ASSERT_STREQ("Id", colInfo.GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str()) << "ECPropertyDef.ECInstanceId for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
            continue;
            }
        
        if (colName.EqualsI("ECClassId"))
            {
            ASSERT_EQ(m_ecdb.Schemas().GetClass("ECDbMeta", "ECPropertyDef")->GetId(), val.GetId<ECClassId>()) << "ECPropertyDef.ECClassId";
            ASSERT_STREQ("ClassId", colInfo.GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str()) << "ECPropertyDef.ECClassId for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
            continue;
            }

        if (colName.EqualsI("Class"))
            {
            ECClassId relClassId;
            ASSERT_EQ(expectedProp.GetClass().GetId().GetValue(), val.GetNavigation(&relClassId).GetValueUnchecked()) << "ECPropertyDef.Class for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
            ASSERT_EQ(colInfoProp->GetAsNavigationProperty()->GetRelationshipClass()->GetId().GetValue(), relClassId.GetValueUnchecked()) << "ECPropertyDef.Class for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
            continue;
            }

        if (colName.EqualsI("Name"))
            {
            ASSERT_STREQ(expectedProp.GetName().c_str(), val.GetText()) << "ECPropertyDef.Name for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
            continue;
            }

        if (colName.EqualsI("DisplayLabel"))
            {
            if (expectedProp.GetIsDisplayLabelDefined())
                ASSERT_STREQ(expectedProp.GetInvariantDisplayLabel().c_str(), val.GetText()) << "ECPropertyDef.DisplayLabel for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
            else
                ASSERT_TRUE(val.IsNull()) << "ECPropertyDef.DisplayLabel";

            continue;
            }

        if (colName.EqualsI("Description"))
            {
            if (!expectedProp.GetInvariantDescription().empty())
                ASSERT_STREQ(expectedProp.GetInvariantDescription().c_str(), val.GetText()) << "ECPropertyDef.Description for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
            else
                ASSERT_TRUE(val.IsNull()) << "ECPropertyDef.Description";
            continue;
            }

        if (colName.EqualsI("IsReadonly"))
            {
            ASSERT_EQ(expectedProp.GetIsReadOnly(), val.GetBoolean()) << "ECPropertyDef.IsReadonly for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
            continue;
            }

        if (colName.EqualsI("Priority"))
            {
            ASSERT_EQ(expectedProp.GetPriority(), val.GetInt()) << "ECPropertyDef.Priority for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
            continue;
            }

        if (colName.EqualsI("Ordinal"))
            {
            int actualOrdinal = val.GetInt();
            ECPropertyCP actualPropByOrdinal = expectedProp.GetClass().GetPropertyByIndex(actualOrdinal);
            ASSERT_EQ(&expectedProp, actualPropByOrdinal) << "ECPropertyDef.Ordinal. Actual value: " << actualOrdinal << " for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
            continue;
            }

        if (colName.EqualsI("Kind"))
            {
            ECPropertyKind actualKind = (ECPropertyKind) val.GetInt();
            ECEnumeratorCP actualEnumerator = val.GetEnum();
            ASSERT_EQ((int) actualKind, actualEnumerator->GetInteger()) << "ECPropertyDef.Kind";
            switch (actualKind)
                {
                    case ECPropertyKind::Navigation:
                        ASSERT_TRUE(navProp != nullptr) << "ECPropertyDef.Kind. Actual value: Navigation for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                        ASSERT_STREQ("Navigation", actualEnumerator->GetName().c_str()) << "ECPropertyDef.Kind for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                        break;
                    case ECPropertyKind::Primitive:
                        ASSERT_TRUE(primProp != nullptr) << "ECPropertyDef.Kind. Actual value: Primitive for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                        ASSERT_STREQ("Primitive", actualEnumerator->GetName().c_str()) << "ECPropertyDef.Kind for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                        break;
                    case ECPropertyKind::PrimitiveArray:
                        ASSERT_TRUE(primArrayProp != nullptr) << "ECPropertyDef.Kind. Actual value: PrimitiveArray for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                        ASSERT_STREQ("PrimitiveArray", actualEnumerator->GetName().c_str()) << "ECPropertyDef.Kind for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                        break;
                    case ECPropertyKind::Struct:
                        ASSERT_TRUE(structProp != nullptr) << "ECPropertyDef.Kind. Actual value: Struct for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                        ASSERT_STREQ("Struct", actualEnumerator->GetName().c_str()) << "ECPropertyDef.Kind for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                        break;
                    case ECPropertyKind::StructArray:
                        ASSERT_TRUE(structArrayProp != nullptr) << "ECPropertyDef.Kind. Actual value: StructArray for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                        ASSERT_STREQ("StructArray", actualEnumerator->GetName().c_str()) << "ECPropertyDef.Kind for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                        break;
                    default:
                        FAIL() << "Untested ECPropertyDef.Kind: " << (int) actualKind << "Please adjust the test. for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                        break;
                }

            continue;
            }

        if (colName.EqualsI("KindOfQuantity"))
            {
            KindOfQuantityCP expectedKoq = expectedProp.GetKindOfQuantity();

            if (expectedKoq != nullptr)
                {
                ECClassId actualRelClassId;
                ASSERT_EQ(expectedKoq->GetId().GetValue(), val.GetNavigation(&actualRelClassId).GetValueUnchecked()) << "ECPropertyDef.KindOfQuantity for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                ASSERT_EQ(colInfoProp->GetAsNavigationProperty()->GetRelationshipClass()->GetId().GetValue(), actualRelClassId.GetValue()) << "ECPropertyDef.KindOfQuantity for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                }
            else
                ASSERT_TRUE(val.IsNull()) << "ECPropertyDef.KindOfQuantity for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();

            continue;
            }

        if (colName.EqualsI("Category"))
            {
            if (expectedProp.GetCategory() != nullptr)
                {
                ECClassId actualRelClassId;
                ASSERT_EQ(expectedProp.GetCategory()->GetId().GetValue(), val.GetNavigation(&actualRelClassId).GetValueUnchecked()) << "ECPropertyDef.Category for prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                ASSERT_EQ(colInfoProp->GetAsNavigationProperty()->GetRelationshipClass()->GetId().GetValue(), actualRelClassId.GetValue()) << "ECPropertyDef.Category for prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                }
            else
                ASSERT_TRUE(val.IsNull()) << "ECPropertyDef.Category for prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();

            continue;
            }

        if (colName.EqualsI("PrimitiveType"))
            {
            if (primProp != nullptr && primProp->GetEnumeration() == nullptr)
                {
                ASSERT_EQ((int) primProp->GetType(), val.GetInt()) << "ECPropertyDef.PrimitiveType for prim prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                ASSERT_EQ((int) primProp->GetType(), val.GetEnum()->GetInteger()) << "ECPropertyDef.PrimitiveType for prim prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                }
            else if (primArrayProp != nullptr)
                {
                ASSERT_EQ((int) primArrayProp->GetPrimitiveElementType(), val.GetInt()) << "ECPropertyDef.PrimitiveType for prim array prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                ASSERT_EQ((int) primArrayProp->GetPrimitiveElementType(), val.GetEnum()->GetInteger()) << "ECPropertyDef.PrimitiveType for prim array prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                }
            else
                ASSERT_TRUE(val.IsNull()) << "ECPropertyDef.PrimitiveType for neither prim nor prim array prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();

            continue;
            }

        if (colName.EqualsI("PrimitiveTypeMinLength"))
            {
            if (expectedProp.IsMinimumLengthDefined())
                ASSERT_EQ((int) expectedProp.GetMinimumLength(), val.GetInt()) << "ECPropertyDef.PrimitiveTypeMinLength for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
            else
                ASSERT_TRUE(val.IsNull()) << "ECPropertyDef.PrimitiveTypeMinLength for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();

            continue;
            }

        if (colName.EqualsI("PrimitiveTypeMaxLength"))
            {
            if (expectedProp.IsMaximumLengthDefined())
                ASSERT_EQ((int) expectedProp.GetMaximumLength(), val.GetInt()) << "ECPropertyDef.PrimitiveTypeMaxLength for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
            else
                ASSERT_TRUE(val.IsNull()) << "ECPropertyDef.PrimitiveTypeMaxLength for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();

            continue;
            }

        if (colName.EqualsI("PrimitiveTypeMinValue"))
            {
            if (expectedProp.IsMinimumValueDefined())
                {
                ECValue expectedMinValue;
                ASSERT_EQ(ECObjectsStatus::Success, expectedProp.GetMinimumValue(expectedMinValue)) << "ECPropertyDef.PrimitiveTypeMinValue for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                ASSERT_TRUE(expectedMinValue.IsPrimitive());
                double expectedMin = 0.0;
                switch (expectedMinValue.GetPrimitiveType())
                    {
                        case PRIMITIVETYPE_Double:
                            expectedMin = expectedMinValue.GetDouble();
                            break;

                        case PRIMITIVETYPE_Integer:
                            expectedMin = (double) expectedMinValue.GetInteger();
                            break;

                        case PRIMITIVETYPE_Long:
                            expectedMin = (double) expectedMinValue.GetLong();
                            break;

                        default:
                            FAIL() << "Unexpected min value for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                            return;
                    }

                ASSERT_DOUBLE_EQ(expectedMin, val.GetDouble()) << "ECPropertyDef.PrimitiveTypeMinValue for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                }
            else
                ASSERT_TRUE(val.IsNull()) << "ECPropertyDef.PrimitiveTypeMinValue for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();

            continue;
            }

        if (colName.EqualsI("PrimitiveTypeMaxValue"))
            {
            if (expectedProp.IsMaximumValueDefined())
                {
                ECValue expectedMaxValue;
                ASSERT_EQ(ECObjectsStatus::Success, expectedProp.GetMaximumValue(expectedMaxValue)) << "ECPropertyDef.PrimitiveTypeMaxValue for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                ASSERT_TRUE(expectedMaxValue.IsPrimitive());
                double expectedMax = 0.0;
                switch (expectedMaxValue.GetPrimitiveType())
                    {
                        case PRIMITIVETYPE_Double:
                            expectedMax = expectedMaxValue.GetDouble();
                            break;

                        case PRIMITIVETYPE_Integer:
                            expectedMax = (double) expectedMaxValue.GetInteger();
                            break;

                        case PRIMITIVETYPE_Long:
                            expectedMax = (double) expectedMaxValue.GetLong();
                            break;

                        default:
                            FAIL() << "Unexpected max value for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                            return;
                    }

                ASSERT_DOUBLE_EQ(expectedMax, val.GetDouble()) << "ECPropertyDef.PrimitiveTypeMaxValue for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                }
            else
                ASSERT_TRUE(val.IsNull()) << "ECPropertyDef.PrimitiveTypeMaxValue for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();

            continue;
            }


        if (colName.EqualsI("Enumeration"))
            {
            ECEnumerationCP expectedEnum = nullptr;
            if (primProp != nullptr)
                expectedEnum = primProp->GetEnumeration();

            if (expectedEnum != nullptr)
                {
                ECClassId actualRelClassId;
                ASSERT_EQ(expectedEnum->GetId().GetValue(), val.GetNavigation(&actualRelClassId).GetValueUnchecked()) << "ECPropertyDef.Enumeration for prim prop with enumeration for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                ASSERT_EQ(colInfoProp->GetAsNavigationProperty()->GetRelationshipClass()->GetId(), actualRelClassId) << "ECPropertyDef.Enumeration for prim prop with enumeration for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                }
            else
                ASSERT_TRUE(val.IsNull()) << "ECPropertyDef.EnumerationId for prim prop without enumeration or non-prim prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();

            continue;
            }

        if (colName.EqualsI("ExtendedTypeName"))
            {
            if (primProp != nullptr)
                {
                if (primProp->HasExtendedType())
                    ASSERT_STREQ(primProp->GetExtendedTypeName().c_str(), val.GetText()) << "ECPropertyDef.ExtendedTypeName for prim prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                else
                    ASSERT_TRUE(val.IsNull()) << "ECPropertyDef.ExtendedTypeName for prim prop with unset extended type for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                }
            else if (primArrayProp != nullptr)
                {
                if (primArrayProp->HasExtendedType())
                    ASSERT_STREQ(primArrayProp->GetExtendedTypeName().c_str(), val.GetText()) << "ECPropertyDef.ExtendedTypeName for prim array prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                else
                    ASSERT_TRUE(val.IsNull()) << "ECPropertyDef.ExtendedTypeName for prim array prop with unset extended type for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                }
            else
                ASSERT_TRUE(val.IsNull()) << "ECPropertyDef.ExtendedTypeName for neither prim nor prim array prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();

            continue;
            }

        if (colName.EqualsI("StructClass"))
            {
            if (structProp != nullptr)
                {
                ECClassId actualRelClassId;
                ASSERT_EQ(structProp->GetType().GetId().GetValue(), val.GetNavigation(&actualRelClassId).GetValue()) << "ECPropertyDef.StructClass for struct prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                ASSERT_EQ(colInfoProp->GetAsNavigationProperty()->GetRelationshipClass()->GetId().GetValue(), actualRelClassId.GetValue()) << "ECPropertyDef.StructClass for struct prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                }
            else if (structArrayProp != nullptr)
                {
                ECClassId actualRelClassId;
                ASSERT_EQ(structArrayProp->GetStructElementType().GetId().GetValue(), val.GetNavigation(&actualRelClassId).GetValueUnchecked()) << "ECPropertyDef.StructClass for struct array prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                ASSERT_EQ(colInfoProp->GetAsNavigationProperty()->GetRelationshipClass()->GetId().GetValue(), actualRelClassId.GetValue()) << "ECPropertyDef.StructClass for struct prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                }
            else
                ASSERT_TRUE(val.IsNull()) << "ECPropertyDef.StructClass for neither struct nor struct array prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();

            continue;
            }

        
        if (colName.EqualsI("ArrayMinOccurs"))
            {
            ArrayECPropertyCP arrayProp = nullptr;
            if (nullptr != primArrayProp)
                arrayProp = primArrayProp;
            else if (nullptr != structArrayProp)
                arrayProp = structArrayProp;

            if (arrayProp != nullptr)
                ASSERT_EQ((int64_t) arrayProp->GetMinOccurs(), val.GetInt64()) << "ECPropertyDef.ArrayMinOccurs for array prop " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
            else
                ASSERT_TRUE(val.IsNull()) << "ECPropertyDef.ArrayMinOccurs for non-array prop " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();

            continue;
            }

        if (colName.EqualsI("ArrayMaxOccurs"))
            {
            ArrayECPropertyCP arrayProp = nullptr;
            if (nullptr != primArrayProp)
                arrayProp = primArrayProp;
            else if (nullptr != structArrayProp)
                arrayProp = structArrayProp;

            if (arrayProp != nullptr)
                {
                if (arrayProp->IsStoredMaxOccursUnbounded())
                    ASSERT_TRUE(val.IsNull()) << "Unbounded ECPropertyDef.ArrayMaxOccurs for array prop " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                else
                    ASSERT_EQ((int64_t) arrayProp->GetStoredMaxOccurs(), val.GetInt64()) << "ECPropertyDef.ArrayMaxOccurs for array prop " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                }
            else
                ASSERT_TRUE(val.IsNull()) << "ECPropertyDef.ArrayMaxOccurs for non-array prop " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();

            continue;
            }

        if (colName.EqualsI("NavigationRelationshipClass"))
            {
            if (navProp != nullptr)
                {
                ECClassId actualRelClassId;
                ASSERT_EQ(navProp->GetRelationshipClass()->GetId().GetValue(), val.GetNavigation(&actualRelClassId).GetValueUnchecked()) << "ECPropertyDef.NavigationRelationshipClass for nav prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                ASSERT_EQ(colInfoProp->GetAsNavigationProperty()->GetRelationshipClass()->GetId().GetValue(), actualRelClassId.GetValue()) << "ECPropertyDef.NavigationRelationshipClass for nav prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                }
            else
                ASSERT_TRUE(val.IsNull()) << "ECPropertyDef.NavigationRelationshipClass for non-navigation prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();

            continue;
            }

        if (colName.EqualsI("NavigationDirection"))
            {
            if (navProp != nullptr)
                {
                ASSERT_EQ((int) navProp->GetDirection(), val.GetInt()) << "ECPropertyDef.NavigationDirection for nav prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                ASSERT_EQ((int) navProp->GetDirection(), val.GetEnum()->GetInteger()) << "ECPropertyDef.NavigationDirection for nav prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                }
            else
                ASSERT_TRUE(val.IsNull()) << "ECPropertyDef.NavigationDirection for non-nav prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();

            continue;
            }

        FAIL() << "ECProperty ECPropertyDef." << colName.c_str() << " not tested. Test needs to be adjusted for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbMetaSchemaECSqlTestFixture, VerifyQueries)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbmetaschematests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
    AssertSchemaDefs();
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMetaSchemaECSqlTestFixture, ECClassId)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("metaschematests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * from meta.ECSchemaDef WHERE ECClassId IS NOT NULL"));
    }
 
/*---------------------------------------------------------------------------------------
<summary>Creates a class chain and add properties and then verifies if they
come in the expected sequence.</summary>
<Scenario>
This is the class hierarchy used in this test. The numbers indicate override priority,
and the letters indicate ECClass name and their inital properties, e.g.
"4cd" represents the ECClass named "cd" which has ECProperties named "c" and "d"
and which is 4th overall in override priority... it can override properties from ECClass
"kl", but not properties from "ab".

//     3ab 4cd 6gh 7ij
//       \/      \/
//       2ef    5kl
//          \  /
//          1mn
</scenario>
@bsimethod
-------------+---------------+---------------+---------------+---------------+---------*/
TEST_F(ECDbMetaSchemaECSqlTestFixture, PropertyOverrides)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("metaschema_propertyoverrides.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
                    <ECEntityClass typeName="AB">
                        <ECProperty propertyName="a" typeName="double" />
                        <ECProperty propertyName="b" typeName="double" />
                    </ECEntityClass>
                    <ECEntityClass typeName="ICD" modifier="Abstract">
                        <ECCustomAttributes>
                            <IsMixin xmlns="CoreCustomAttributes.01.00">
                                <AppliesToEntityClass>AB</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                        <ECProperty propertyName="c" typeName="double" />
                        <ECProperty propertyName="d" typeName="double" />
                    </ECEntityClass>
                    <ECEntityClass typeName="EF">
                        <BaseClass>AB</BaseClass>
                        <BaseClass>ICD</BaseClass>
                        <ECProperty propertyName="e" typeName="double" />
                        <ECProperty propertyName="f" typeName="double" />
                    </ECEntityClass>
                    <ECEntityClass typeName="IGH" modifier="Abstract">
                        <ECCustomAttributes>
                            <IsMixin xmlns="CoreCustomAttributes.01.00">
                                <AppliesToEntityClass>AB</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                        <ECProperty propertyName="g" typeName="double" />
                        <ECProperty propertyName="h" typeName="double" />
                    </ECEntityClass>
                    <ECEntityClass typeName="IIJ" modifier="Abstract">
                        <ECCustomAttributes>
                            <IsMixin xmlns="CoreCustomAttributes.01.00">
                                <AppliesToEntityClass>AB</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                        <ECProperty propertyName="i" typeName="double" />
                        <ECProperty propertyName="j" typeName="double" />
                    </ECEntityClass>
                    <ECEntityClass typeName="IKL" modifier="Abstract">
                        <ECCustomAttributes>
                            <IsMixin xmlns="CoreCustomAttributes.01.00">
                                <AppliesToEntityClass>AB</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                        <BaseClass>IGH</BaseClass>
                        <ECProperty propertyName="k" typeName="double" />
                        <ECProperty propertyName="l" typeName="double" />
                    </ECEntityClass>
                    <ECEntityClass typeName="MN">
                        <BaseClass>EF</BaseClass>
                        <BaseClass>IIJ</BaseClass>
                        <BaseClass>IKL</BaseClass>
                        <ECProperty propertyName="m" typeName="double" />
                        <ECProperty propertyName="n" typeName="double" />
                    </ECEntityClass>
                </ECSchema>)xml")));

    auto verifyPropertyOverride = [] (ECDbCR ecdb, ECClassCP ecClass)
        {
        ASSERT_TRUE(ecClass != nullptr);
        // retrieve all local, inherited, and overridden properties from a given class
        ECSqlStatement propertyStatement;
        ASSERT_EQ(ECSqlStatus::Success, propertyStatement.Prepare(ecdb, "SELECT ECInstanceId, PrimitiveType FROM meta.ECPropertyDef WHERE Class.Id=? AND Name=?"));

        // iterate through properties using ECObjects' GetProperties(true) and compare to ECSql-retrieved properties
        for (ECPropertyCP prop : ecClass->GetProperties(true))
            {
            ASSERT_EQ(ECSqlStatus::Success, propertyStatement.BindId(1, prop->GetClass().GetId())) << ecClass->GetFullName() << "." << prop->GetName().c_str();
            ASSERT_EQ(ECSqlStatus::Success, propertyStatement.BindText(2, prop->GetName().c_str(), IECSqlBinder::MakeCopy::No)) << ecClass->GetFullName() << "." << prop->GetName().c_str();
            ASSERT_EQ(BE_SQLITE_ROW, propertyStatement.Step()) << ecClass->GetFullName() << "." << prop->GetName().c_str();
            
            ASSERT_EQ(prop->GetId().GetValue(), propertyStatement.GetValueId<ECPropertyId>(0).GetValue()) << ecClass->GetFullName() << "." << prop->GetName().c_str();
            ASSERT_TRUE(prop->GetIsPrimitive()) << ecClass->GetFullName() << "." << prop->GetName().c_str();
            ASSERT_EQ(PRIMITIVETYPE_Double, prop->GetAsPrimitiveProperty()->GetType()) << ecClass->GetFullName() << "." << prop->GetName().c_str();
            ASSERT_EQ(prop->GetId().GetValue(), propertyStatement.GetValueId<ECPropertyId>(0).GetValue()) << ecClass->GetFullName() << "." << prop->GetName().c_str();
            
            ASSERT_EQ(BE_SQLITE_DONE, propertyStatement.Step()) << ecClass->GetFullName() << "." << prop->GetName().c_str();
            propertyStatement.Reset();
            propertyStatement.ClearBindings();
            }
        };

    // compare local copy properties with ECSql-retrieved properties
    verifyPropertyOverride(m_ecdb, m_ecdb.Schemas().GetClass("TestSchema", "MN"));

    // add some duplicate properties to MN, overriding those from the base classes 
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                  <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
                  <ECEntityClass typeName="AB">
                        <ECProperty propertyName="a" typeName="double" />
                        <ECProperty propertyName="b" typeName="double" />
                    </ECEntityClass>
                    <ECEntityClass typeName="ICD" modifier="Abstract">
                        <ECCustomAttributes>
                            <IsMixin xmlns="CoreCustomAttributes.01.00">
                                <AppliesToEntityClass>AB</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                        <ECProperty propertyName="c" typeName="double" />
                        <ECProperty propertyName="d" typeName="double" />
                    </ECEntityClass>
                    <ECEntityClass typeName="EF">
                        <BaseClass>AB</BaseClass>
                        <BaseClass>ICD</BaseClass>
                        <ECProperty propertyName="e" typeName="double" />
                        <ECProperty propertyName="f" typeName="double" />
                    </ECEntityClass>
                    <ECEntityClass typeName="IGH" modifier="Abstract">
                        <ECCustomAttributes>
                            <IsMixin xmlns="CoreCustomAttributes.01.00">
                                <AppliesToEntityClass>AB</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                        <ECProperty propertyName="g" typeName="double" />
                        <ECProperty propertyName="h" typeName="double" />
                    </ECEntityClass>
                    <ECEntityClass typeName="IIJ" modifier="Abstract">
                        <ECCustomAttributes>
                            <IsMixin xmlns="CoreCustomAttributes.01.00">
                                <AppliesToEntityClass>AB</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                        <ECProperty propertyName="i" typeName="double" />
                        <ECProperty propertyName="j" typeName="double" />
                    </ECEntityClass>
                    <ECEntityClass typeName="IKL" modifier="Abstract">
                        <ECCustomAttributes>
                            <IsMixin xmlns="CoreCustomAttributes.01.00">
                                <AppliesToEntityClass>AB</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                        <BaseClass>IGH</BaseClass>
                        <ECProperty propertyName="k" typeName="double" />
                        <ECProperty propertyName="l" typeName="double" />
                    </ECEntityClass>
                    <ECEntityClass typeName="MN">
                        <BaseClass>EF</BaseClass>
                        <BaseClass>IIJ</BaseClass>
                        <BaseClass>IKL</BaseClass>
                        <ECProperty propertyName="m" typeName="double" />
                        <ECProperty propertyName="n" typeName="double" />
                        <ECProperty propertyName="b" typeName="double" />
                        <ECProperty propertyName="d" typeName="double" />
                        <ECProperty propertyName="f" typeName="double" />
                        <ECProperty propertyName="h" typeName="double" />
                        <ECProperty propertyName="j" typeName="double" />
                        <ECProperty propertyName="k" typeName="double" />
                    </ECEntityClass>
                </ECSchema>)xml")));

    verifyPropertyOverride(m_ecdb, m_ecdb.Schemas().GetClass("TestSchema", "MN"));

    // override more properties of base classes (add eacg to kl, iab to gh, l to ef, g to ij and gh to ab)
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                      <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
                 <ECEntityClass typeName="AB">
                        <ECProperty propertyName="a" typeName="double" />
                        <ECProperty propertyName="b" typeName="double" />
                        <ECProperty propertyName="g" typeName="double" />
                        <ECProperty propertyName="h" typeName="double" />
                    </ECEntityClass>
                    <ECEntityClass typeName="ICD" modifier="Abstract">
                         <ECCustomAttributes>
                            <IsMixin xmlns="CoreCustomAttributes.01.00">
                                <AppliesToEntityClass>AB</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                       <ECProperty propertyName="c" typeName="double" />
                        <ECProperty propertyName="d" typeName="double" />
                    </ECEntityClass>
                    <ECEntityClass typeName="EF">
                        <BaseClass>AB</BaseClass>
                        <BaseClass>ICD</BaseClass>
                        <ECProperty propertyName="e" typeName="double" />
                        <ECProperty propertyName="f" typeName="double" />
                        <ECProperty propertyName="l" typeName="double" />
                    </ECEntityClass>
                    <ECEntityClass typeName="IGH" modifier="Abstract">
                          <ECCustomAttributes>
                            <IsMixin xmlns="CoreCustomAttributes.01.00">
                                <AppliesToEntityClass>AB</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                        <ECProperty propertyName="g" typeName="double" />
                        <ECProperty propertyName="h" typeName="double" />
                        <ECProperty propertyName="a" typeName="double" />
                        <ECProperty propertyName="b" typeName="double" />
                        <ECProperty propertyName="i" typeName="double" />
                    </ECEntityClass>
                    <ECEntityClass typeName="IIJ" modifier="Abstract">
                        <ECCustomAttributes>
                            <IsMixin xmlns="CoreCustomAttributes.01.00">
                                <AppliesToEntityClass>AB</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                        <ECProperty propertyName="i" typeName="double" />
                        <ECProperty propertyName="j" typeName="double" />
                        <ECProperty propertyName="g" typeName="double" />
                    </ECEntityClass>
                    <ECEntityClass typeName="IKL" modifier="Abstract">
                        <ECCustomAttributes>
                            <IsMixin xmlns="CoreCustomAttributes.01.00">
                                <AppliesToEntityClass>AB</AppliesToEntityClass>
                            </IsMixin>
                        </ECCustomAttributes>
                        <BaseClass>IGH</BaseClass>
                        <ECProperty propertyName="k" typeName="double" />
                        <ECProperty propertyName="l" typeName="double" />
                        <ECProperty propertyName="e" typeName="double" />
                        <ECProperty propertyName="a" typeName="double" />
                        <ECProperty propertyName="c" typeName="double" />
                        <ECProperty propertyName="g" typeName="double" />
                    </ECEntityClass>
                    <ECEntityClass typeName="MN">
                        <BaseClass>EF</BaseClass>
                        <BaseClass>IIJ</BaseClass>
                        <BaseClass>IKL</BaseClass>
                        <ECProperty propertyName="m" typeName="double" />
                        <ECProperty propertyName="n" typeName="double" />
                        <ECProperty propertyName="b" typeName="double" />
                        <ECProperty propertyName="d" typeName="double" />
                        <ECProperty propertyName="f" typeName="double" />
                        <ECProperty propertyName="h" typeName="double" />
                        <ECProperty propertyName="j" typeName="double" />
                        <ECProperty propertyName="k" typeName="double" />
                    </ECEntityClass>
                </ECSchema>)xml")));
    verifyPropertyOverride(m_ecdb, m_ecdb.Schemas().GetClass("TestSchema", "MN"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbMetaSchemaECSqlTestFixture, CustomAttributes) {
    NativeLogging::Logging::SetLogger(&NativeLogging::ConsoleLogger::GetLogger());
    NativeLogging::ConsoleLogger::GetLogger().SetSeverity("ECDb", BentleyApi::NativeLogging::LOG_TRACE);
    NativeLogging::ConsoleLogger::GetLogger().SetSeverity("ECObjectsNative", BentleyApi::NativeLogging::LOG_TRACE);
    ASSERT_EQ(SUCCESS, SetupECDb("metaschema_customattributes.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECCustomAttributeClass typeName="CAClass" modifier="Sealed" appliesTo="EntityClass">
            <ECProperty propertyName="CAProp" typeName="string" />
        </ECCustomAttributeClass>
        <ECEntityClass typeName="Foo">
            <ECCustomAttributes>
                <CAClass>
                    <CAProp>Test</CAProp>
                </CAClass>
           </ECCustomAttributes>
        </ECEntityClass>
    </ECSchema>)xml")));

    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM meta.CustomAttribute"));
        //printf("%s\n", stmt.GetNativeSql());
        ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
    }

    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT c.Name FROM meta.CustomAttribute ca JOIN meta.ECClassDef c USING meta.CustomAttributeClassHasInstance"));
        while (BE_SQLITE_ROW == stmt.Step())
            {
            printf("%s\n", stmt.GetValueText(0));
            }
        //ASSERT_EQ(stmt.GetValueInt(0), 10);
    }

    {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT XmlCAToJson(ca.Class.Id, ca.Instance) FROM meta.ECClassDef c JOIN meta.CustomAttribute ca ON ca.Class.Id=c.ECInstanceId"));
        while (BE_SQLITE_ROW == stmt.Step())
            {
            printf("%s\n", stmt.GetValueText(0));
            }
        //ASSERT_EQ(stmt.GetValueInt(0), 10);
    }
}

END_ECDBUNITTESTS_NAMESPACE