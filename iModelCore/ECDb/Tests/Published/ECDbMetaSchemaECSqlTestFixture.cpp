/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbMetaSchemaECSqlTestFixture.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// @bsistruct                                                   Mike.Embick     12/15
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

protected:
    void AssertSchemaDefs();
    };

//---------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle 04/2016
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
        actualSchemaCount++;
        }

    bvector<ECSchemaCP> expectedSchemas = m_ecdb.Schemas().GetSchemas(false);
    ASSERT_EQ((int) expectedSchemas.size(), actualSchemaCount);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle 04/2016
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
            ASSERT_EQ(expectedSchema.GetId().GetValue(), val.GetId<ECSchemaId>().GetValue()) << "ECSchemaDef.ECInstanceId";
        else if (colName.EqualsI("ECClassId"))
            ASSERT_EQ(m_ecdb.Schemas().GetClass("ECDbMeta", "ECSchemaDef")->GetId(), val.GetId<ECClassId>()) << "ECSchemaDef.ECClassId";
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
        else
            FAIL() << "ECProperty ECSchemaDef." << colName.c_str() << " not tested. Test needs to be adjusted";
        }
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle 04/2016
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
// @bsimethod                                                    Krischan.Eberle 04/2016
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
            continue;
            }

        if (colName.EqualsI("ECClassId"))
            {
            ASSERT_EQ(m_ecdb.Schemas().GetClass("ECDbMeta", "ECClassDef")->GetId(), val.GetId<ECClassId>()) << "ECClassDef.ECClassId";
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
            ASSERT_EQ((int) classType, val.GetInt()) << "ECClassDef.Type";
            continue;
            }

        if (colName.EqualsI("Modifier"))
            {
            ASSERT_EQ((int) expectedClass.GetClassModifier(), val.GetInt()) << "ECClassDef.Modifier";
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
                    ASSERT_EQ((int) expectedCAClass->GetContainerType(), val.GetInt()) << "ECClassDef.CustomAttributeContainerType for CA classes";
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
                }
            else
                ASSERT_TRUE(val.IsNull()) << "ECClassDef.RelationshipStrengthDirection for non-relationship class";

            continue;
            }

        FAIL() << "ECProperty ECClassDef." << colName.c_str() << " not tested. Test needs to be adjusted";
        }
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle 06/2016
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
// @bsimethod                                                    Krischan.Eberle 04/2016
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
// @bsimethod                                                    Krischan.Eberle 04/2016
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
            //EnumerationId is not exposed in API. So we can only test that the actual id is generally a valid one
            ASSERT_TRUE(val.GetId<BeInt64Id>().IsValid()) << "ECEnumerationDef.ECInstanceId";
            continue;
            }

        if (colName.EqualsI("ECClassId"))
            {
            ASSERT_EQ(m_ecdb.Schemas().GetClass("ECDbMeta", "ECEnumerationDef")->GetId(), val.GetId<ECClassId>()) << "ECEnumerationDef.ECClassId";
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
// @bsimethod                                                    Krischan.Eberle 04/2016
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbMetaSchemaECSqlTestFixture::AssertEnumerationValue(ECEnumeratorCR expectedEnumValue, IECSqlValue const& actualEnumValue)
    {
    for (IECSqlValue const& memberVal : actualEnumValue.GetStructIterable())
        {
        ASSERT_TRUE(memberVal.GetColumnInfo().GetProperty() != nullptr);
        Utf8StringCR memberName = memberVal.GetColumnInfo().GetProperty()->GetName();

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
            if (!expectedEnumValue.GetDisplayLabel().empty())
                ASSERT_STREQ(expectedEnumValue.GetDisplayLabel().c_str(), memberVal.GetText()) << "ECEnumerationDef.EnumValues[].DisplayLabel";
            else
                ASSERT_TRUE(memberVal.IsNull()) << "ECEnumerationDef.EnumValues[].DisplayLabel if not defined";

            continue;
            }

        FAIL() << "Untested Struct member: " << memberName.c_str() << " Please adjust the test";
        }
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle 06/2016
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
// @bsimethod                                                    Krischan.Eberle 06/2016
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
            //EnumerationId is not exposed in API. So we can only test that the actual id is generally a valid one
            ASSERT_TRUE(val.GetId<BeInt64Id>().IsValid()) << "KindOfQuantityDef.ECInstanceId";
            continue;
            }

        if (colName.EqualsI("ECClassId"))
            {
            ASSERT_EQ(m_ecdb.Schemas().GetClass("ECDbMeta", "KindOfQuantityDef")->GetId(), val.GetId<ECClassId>()) << "KindOfQuantityDef.ECClassId";
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

        if (colName.EqualsI("PersistenceUnit"))
            {
            ASSERT_STREQ(expectedKoq.GetPersistenceUnit().ToText(false).c_str(), val.GetText()) << "KindOfQuantityDef.PersistenceUnit";
            continue;
            }

        if (colName.EqualsI("RelativeError"))
            {
            ASSERT_DOUBLE_EQ(expectedKoq.GetRelativeError(), val.GetDouble()) << "KindOfQuantityDef.RelativeError";
            continue;
            }

        if (colName.EqualsI("PresentationUnits"))
            {
            if (expectedKoq.GetPresentationUnitList().empty())
                ASSERT_TRUE(val.IsNull()) << "KindOfQuantityDef.PresentationUnits";
            else
                {
                ASSERT_EQ((int) expectedKoq.GetPresentationUnitList().size(), val.GetArrayLength()) << "KindOfQuantityDef.PresentationUnits";

                size_t i = 0;
                for (IECSqlValue const& arrayElementVal : val.GetArrayIterable())
                    {
                    ASSERT_STREQ(expectedKoq.GetPresentationUnitList()[i].ToText(false).c_str(), arrayElementVal.GetText()) << "KindOfQuantityDef.PresentationUnits";
                    i++;
                    }
                }

            continue;
            }
        }
    }


//---------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle 06/2017
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
// @bsimethod                                                    Krischan.Eberle 06/2016
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
            //EnumerationId is not exposed in API. So we can only test that the actual id is generally a valid one
            ASSERT_TRUE(val.GetId<BeInt64Id>().IsValid()) << "PropertyCategoryDef.ECInstanceId";
            continue;
            }

        if (colName.EqualsI("ECClassId"))
            {
            ASSERT_EQ(m_ecdb.Schemas().GetClass("ECDbMeta", "PropertyCategoryDef")->GetId(), val.GetId<ECClassId>()) << "PropertyCategoryDef.ECClassId";
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
// @bsimethod                                                    Krischan.Eberle 04/2016
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
// @bsimethod                                                    Krischan.Eberle 04/2016
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
            continue;
            }
        
        if (colName.EqualsI("ECClassId"))
            {
            ASSERT_EQ(m_ecdb.Schemas().GetClass("ECDbMeta", "ECPropertyDef")->GetId(), val.GetId<ECClassId>()) << "ECPropertyDef.ECClassId";
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
            switch (actualKind)
                {
                    case ECPropertyKind::Navigation:
                        ASSERT_TRUE(navProp != nullptr) << "ECPropertyDef.Kind. Actual value: Navigation for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                        break;
                    case ECPropertyKind::Primitive:
                        ASSERT_TRUE(primProp != nullptr) << "ECPropertyDef.Kind. Actual value: Primitive for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                        break;
                    case ECPropertyKind::PrimitiveArray:
                        ASSERT_TRUE(primArrayProp != nullptr) << "ECPropertyDef.Kind. Actual value: PrimitiveArray for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                        break;
                    case ECPropertyKind::Struct:
                        ASSERT_TRUE(structProp != nullptr) << "ECPropertyDef.Kind. Actual value: Struct for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
                        break;
                    case ECPropertyKind::StructArray:
                        ASSERT_TRUE(structArrayProp != nullptr) << "ECPropertyDef.Kind. Actual value: StructArray for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
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
                ASSERT_EQ((int) primProp->GetType(), val.GetInt()) << "ECPropertyDef.PrimitiveType for prim prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
            else if (primArrayProp != nullptr)
                ASSERT_EQ((int) primArrayProp->GetPrimitiveElementType(), val.GetInt()) << "ECPropertyDef.PrimitiveType for prim array prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
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
                ASSERT_EQ((int) navProp->GetDirection(), val.GetInt()) << "ECPropertyDef.NavigationDirection for nav prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
            else
                ASSERT_TRUE(val.IsNull()) << "ECPropertyDef.NavigationDirection for non-nav prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();

            continue;
            }

        FAIL() << "ECProperty ECPropertyDef." << colName.c_str() << " not tested. Test needs to be adjusted for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbMetaSchemaECSqlTestFixture, VerifyQueries)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbmetaschematests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.ecschema.xml")));
    AssertSchemaDefs();
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMetaSchemaECSqlTestFixture, ECClassId)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("metaschematests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.ecschema.xml")));

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
@bsimethod                              Zakary.Olyarnik                         08/16
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

        // iterate through properties using ECObjects' GetProperties() and compare to ECSql-retrieved properties
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

END_ECDBUNITTESTS_NAMESPACE