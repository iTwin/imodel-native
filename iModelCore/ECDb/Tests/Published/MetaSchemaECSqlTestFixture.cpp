/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/MetaSchemaECSqlTestFixture.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "SchemaImportTestFixture.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// @bsistruct                                                   Mike.Embick     12/15
//=======================================================================================
struct MetaSchemaECSqlTestFixture : SchemaImportTestFixture
    {
private:
    void AssertSchemaDef(ECSchemaCR expectedSchema, ECSqlStatement const& actualSchemaDefRow);
    void AssertClassDefs(ECSchemaCR expectedSchema);
    void AssertClassDef(ECClassCR expectedClass, ECSqlStatement const& actualClassDefRow);
    void AssertEnumerationDefs(ECSchemaCR expectedSchema);
    void AssertEnumerationDef(ECEnumerationCR expectedEnum, ECSqlStatement const& actualEnumerationDefRow);
    void AssertEnumerationValue(ECEnumeratorCR expectedEnumValue, IECSqlStructValue const& actualEnumValue);
    void AssertPropertyDefs(ECClassCR expectedClass);
    void AssertPropertyDef(ECPropertyCR expectedProp, ECSqlStatement const& actualPropertyDefRow);

protected:
    void AssertSchemaDefs();
    };

//---------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle 04/2016
//+---------------+---------------+---------------+---------------+---------------+------
void MetaSchemaECSqlTestFixture::AssertSchemaDefs()
    {
    ECSqlStatement schemaStatement;
    ASSERT_EQ(ECSqlStatus::Success, schemaStatement.Prepare(GetECDb(), "SELECT Name,* FROM ec.ECSchemaDef"));
    
    int actualSchemaCount = 0;
    while (BE_SQLITE_ROW == schemaStatement.Step())
        {
        Utf8CP actualSchemaName = schemaStatement.GetValueText(0);
        ECSchemaCP expectedSchema = GetECDb().Schemas().GetECSchema(actualSchemaName);
        ASSERT_TRUE(expectedSchema != nullptr);

        AssertSchemaDef(*expectedSchema, schemaStatement);
        AssertClassDefs(*expectedSchema);
        actualSchemaCount++;
        }

    bvector<ECSchemaCP> expectedSchemas;
    ASSERT_EQ(SUCCESS, GetECDb().Schemas().GetECSchemas(expectedSchemas, false));
    ASSERT_EQ((int) expectedSchemas.size(), actualSchemaCount);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle 04/2016
//+---------------+---------------+---------------+---------------+---------------+------
void MetaSchemaECSqlTestFixture::AssertSchemaDef(ECSchemaCR expectedSchema, ECSqlStatement const& actualSchemaDefRow)
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
        else if (colName.EqualsI("Name"))
            ASSERT_STREQ(expectedSchema.GetName().c_str(), val.GetText()) << "ECSchemaDef.Name";
        else if (colName.EqualsI("DisplayLabel"))
            {
            if (expectedSchema.GetIsDisplayLabelDefined())
                ASSERT_STREQ(expectedSchema.GetDisplayLabel().c_str(), val.GetText()) << "ECSchemaDef.DisplayLabel";
            else
                ASSERT_TRUE(val.IsNull()) << "ECSchemaDef.DisplayLabel";
            }
        else if (colName.EqualsI("Description"))
            ASSERT_STREQ(expectedSchema.GetDescription().c_str(), val.GetText()) << "ECSchemaDef.Description";
        else if (colName.EqualsI("NamespacePrefix"))
            ASSERT_STREQ(expectedSchema.GetNamespacePrefix().c_str(), val.GetText()) << "ECSchemaDef.NamespacePrefix";
        else if (colName.EqualsI("VersionMajor"))
            ASSERT_EQ(expectedSchema.GetVersionMajor(), (uint32_t) val.GetInt()) << "ECSchemaDef.VersionMajor";
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
void MetaSchemaECSqlTestFixture::AssertClassDefs(ECSchemaCR expectedSchema)
    {
    ECSqlStatement classStatement;
    ASSERT_EQ(ECSqlStatus::Success, classStatement.Prepare(GetECDb(), "SELECT c.Name, c.* FROM ec.ECSchemaDef s "
                                                           "JOIN ec.ECClassDef c USING ec.SchemaOwnsClasses "
                                                           "WHERE s.Name=?"));

    ASSERT_EQ(ECSqlStatus::Success, classStatement.BindText(1, expectedSchema.GetName().c_str(), IECSqlBinder::MakeCopy::No));

    int actualClassCount = 0;
    while (BE_SQLITE_ROW == classStatement.Step())
        {
        Utf8CP actualClassName = classStatement.GetValueText(0);
        ECClassCP expectedClass = expectedSchema.GetClassCP(actualClassName);
        ASSERT_TRUE(expectedClass != nullptr);

        AssertClassDef(*expectedClass, classStatement);
        AssertPropertyDefs(*expectedClass);
        actualClassCount++;
        }

    ASSERT_EQ((int) expectedSchema.GetClassCount(), actualClassCount);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle 04/2016
//+---------------+---------------+---------------+---------------+---------------+------
void MetaSchemaECSqlTestFixture::AssertClassDef(ECClassCR expectedClass, ECSqlStatement const& actualClassDefRow)
    {
    const ECClassType classType = expectedClass.GetClassType();
    ECCustomAttributeClassCP expectedCAClass = expectedClass.GetCustomAttributeClassCP();
    ECRelationshipClassCP expectedRelClass = expectedClass.GetRelationshipClassCP();
    
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

        if (colName.EqualsI("SchemaId"))
            {
            ASSERT_EQ(expectedClass.GetSchema().GetId().GetValue(), val.GetId<ECSchemaId>().GetValue()) << "ECClassDef.SchemaId";
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
                ASSERT_STREQ(expectedClass.GetDisplayLabel().c_str(), val.GetText()) << "ECClassDef.DisplayLabel";
            else
                ASSERT_TRUE(val.IsNull()) << "ECClassDef.DisplayLabel";

            continue;
            }

        if (colName.EqualsI("Description"))
            {
            ASSERT_STREQ(expectedClass.GetDescription().c_str(), val.GetText()) << "ECClassDef.Description";
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
// @bsimethod                                                    Krischan.Eberle 04/2016
//+---------------+---------------+---------------+---------------+---------------+------
void MetaSchemaECSqlTestFixture::AssertEnumerationDefs(ECSchemaCR expectedSchema)
    {
    ECSqlStatement enumStatement;
    ASSERT_EQ(ECSqlStatus::Success, enumStatement.Prepare(GetECDb(), "SELECT e.Name, e.* FROM ec.ECSchemaDef s "
                                                           "JOIN ec.ECEnumerationDef e USING ec.SchemaOwnsEnumerations "
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
void MetaSchemaECSqlTestFixture::AssertEnumerationDef(ECEnumerationCR expectedEnum, ECSqlStatement const& actualEnumerationDefRow)
    {
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

        if (colName.EqualsI("SchemaId"))
            {
            ASSERT_EQ(expectedEnum.GetSchema().GetId().GetValue(), val.GetId<ECSchemaId>().GetValue()) << "ECEnumerationDef.SchemaId";
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
                ASSERT_STREQ(expectedEnum.GetDisplayLabel().c_str(), val.GetText()) << "ECEnumerationDef.DisplayLabel";
            else
                ASSERT_TRUE(val.IsNull()) << "ECEnumerationDef.DisplayLabel";

            continue;
            }

        if (colName.EqualsI("Description"))
            {
            ASSERT_STREQ(expectedEnum.GetDescription().c_str(), val.GetText()) << "ECEnumerationDef.Description";
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
            for (IECSqlValue const* arrayElem : val.GetArray())
                {
                ECEnumeratorCP expectedValue = expectedValues[(size_t) actualValueCount];
                ASSERT_TRUE(expectedValue != nullptr);
                IECSqlStructValue const& actualValue = arrayElem->GetStruct();
                AssertEnumerationValue(*expectedValue, actualValue);
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
void MetaSchemaECSqlTestFixture::AssertEnumerationValue(ECEnumeratorCR expectedEnumValue, IECSqlStructValue const& actualEnumValue)
    {
    const int actualStructMemberCount = actualEnumValue.GetMemberCount();
    for (int j = 0; j < actualStructMemberCount; j++)
        {
        IECSqlValue const& memberVal = actualEnumValue.GetValue(j);
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
                ASSERT_EQ(expectedEnumValue.GetString().c_str(), memberVal.GetText()) << "ECEnumerationDef.EnumValues[].StringValue";
            else
                ASSERT_TRUE(memberVal.IsNull()) << "ECEnumerationDef.EnumValues[].StringValue for non-string values";

            continue;
            }
        if (memberName.EqualsI("DisplayLabel"))
            {
            if (expectedEnumValue.GetIsDisplayLabelDefined())
                ASSERT_EQ(expectedEnumValue.GetDisplayLabel().c_str(), memberVal.GetText()) << "ECEnumerationDef.EnumValues[].DisplayLabel";
            else
                ASSERT_TRUE(memberVal.IsNull()) << "ECEnumerationDef.EnumValues[].DisplayLabel if not defined";

            continue;
            }

        FAIL() << "Untested Struct member: " << memberName.c_str() << " Please adjust the test";
        }
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle 04/2016
//+---------------+---------------+---------------+---------------+---------------+------
void MetaSchemaECSqlTestFixture::AssertPropertyDefs(ECClassCR expectedClass)
    {
    ECSqlStatement propStatement;
    ASSERT_EQ(ECSqlStatus::Success, propStatement.Prepare(GetECDb(), "SELECT p.Name, p.* FROM ec.ECPropertyDef p "
                                                           "JOIN ec.ECClassDef c USING ec.ClassOwnsLocalProperties "
                                                           "JOIN ec.ECSchemaDef s USING ec.SchemaOwnsClasses "
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

    //right now only local properties can be retrieved from the MetaSchema
    ASSERT_EQ((int) expectedClass.GetPropertyCount(false), actualPropCount) << expectedClass.GetFullName();
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle 04/2016
//+---------------+---------------+---------------+---------------+---------------+------
void MetaSchemaECSqlTestFixture::AssertPropertyDef(ECPropertyCR expectedProp, ECSqlStatement const& actualPropertyDefRow)
    {
    PrimitiveECPropertyCP primProp = expectedProp.GetAsPrimitiveProperty();
    StructECPropertyCP structProp = expectedProp.GetAsStructProperty();
    ArrayECPropertyCP primArrayProp = expectedProp.GetIsPrimitiveArray() ? expectedProp.GetAsArrayProperty() : nullptr;
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

        if (colName.EqualsI("ClassId"))
            {
            ASSERT_EQ(expectedProp.GetClass().GetId().GetValue(), val.GetId<ECClassId>().GetValue()) << "ECPropertyDef.ClassId for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
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
                ASSERT_STREQ(expectedProp.GetDisplayLabel().c_str(), val.GetText()) << "ECPropertyDef.DisplayLabel for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
            else
                ASSERT_TRUE(val.IsNull()) << "ECPropertyDef.DisplayLabel";

            continue;
            }

        if (colName.EqualsI("Description"))
            {
            ASSERT_STREQ(expectedProp.GetDescription().c_str(), val.GetText()) << "ECPropertyDef.Description for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
            continue;
            }

        if (colName.EqualsI("IsReadonly"))
            {
            ASSERT_EQ(expectedProp.GetIsReadOnly(), val.GetBoolean()) << "ECPropertyDef.IsReadonly for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
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

        if (colName.EqualsI("EnumerationId"))
            {
            if (primProp != nullptr && primProp->GetEnumeration() != nullptr)
                {
                //EnumerationId is not exposed in the API, so we cannot check more than that the actual id is a generally valid id
                ASSERT_TRUE(val.GetId<BeInt64Id>().IsValid()) << "ECPropertyDef.EnumerationId for prim prop with enumeration for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
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

        if (colName.EqualsI("StructClassId"))
            {
            if (structProp != nullptr)
               ASSERT_EQ(structProp->GetType().GetId().GetValue(), val.GetId<ECClassId>().GetValue()) << "ECPropertyDef.StructClassId for struct prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
            else if (structArrayProp != nullptr)
                ASSERT_EQ(structArrayProp->GetStructElementType()->GetId().GetValue(), val.GetId<ECClassId>().GetValue()) << "ECPropertyDef.StructClassId for struct array prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
            else
                ASSERT_TRUE(val.IsNull()) << "ECPropertyDef.StructClassId for neither struct nor struct array prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();

            continue;
            }

        if (colName.EqualsI("ArrayMinOccurs"))
            {
            ArrayECPropertyCP arrayProp = primArrayProp != nullptr ? primArrayProp : structArrayProp;
            if (arrayProp != nullptr)
                ASSERT_EQ((int) arrayProp->GetMinOccurs(), val.GetInt()) << "ECPropertyDef.ArrayMinOccurs for array prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
            else
                ASSERT_TRUE(val.IsNull()) << "ECPropertyDef.ArrayMinOccurs for non-array prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();

            continue;
            }

        if (colName.EqualsI("ArrayMaxOccurs"))
            {
            ArrayECPropertyCP arrayProp = primArrayProp != nullptr ? primArrayProp : structArrayProp;
            if (arrayProp != nullptr)
                ASSERT_EQ((int) arrayProp->GetStoredMaxOccurs(), val.GetInt()) << "ECPropertyDef.ArrayMaxOccurs for array prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
            else
                ASSERT_TRUE(val.IsNull()) << "ECPropertyDef.ArrayMaxOccurs for non-array prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();

            continue;
            }

        if (colName.EqualsI("NavigationRelationshipClassId"))
            {
            if (navProp != nullptr)
                ASSERT_EQ(navProp->GetRelationshipClass()->GetId().GetValue(), val.GetId<ECClassId>().GetValue()) << "ECPropertyDef.NavigationRelationshipClassId for nav prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();
            else
                ASSERT_TRUE(val.IsNull()) << "ECPropertyDef.NavigationRelationshipClassId for non-navigation prop for " << expectedProp.GetClass().GetFullName() << "." << expectedProp.GetName().c_str();

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
TEST_F(MetaSchemaECSqlTestFixture, VerifyQueries)
    {
    SetupECDb("metaschematests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));
    ASSERT_TRUE(GetECDb().IsDbOpen());
    AssertSchemaDefs();
    }

END_ECDBUNITTESTS_NAMESPACE