/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECSqlMetadataQueryTest.cpp $
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
struct ECSqlMetadataQueryTestFixture : SchemaImportTestFixture
    {
    void ComparePropertyDefProperties(ECPropertyCR expectedProperty, IECInstanceCR actualProperty);
    void ComparePropertyDefLists(bvector<ECPropertyCP> & expectedProperties, bvector<IECInstancePtr> & actualProperties);
    void CompareClassDefProperties(ECClassCR expectedClass, IECInstanceCR actualClass);
    void CompareClassDefLists(bvector<ECClassCP> & expectedClasses, bvector<IECInstancePtr> & actualClasses);
    void CompareSchemaDefProperties(ECSchemaCR expectedSchema, IECInstanceCR actualSchema);
    };

//=======================================================================================
// @bsistruct                                                   Mike.Embick     12/15
//=======================================================================================
struct FindMatchingNamePredicate
    {
    ECValue m_expectedValue;
    ECValue m_actualValue;
    FindMatchingNamePredicate(Utf8CP expectedName)
        {
        m_expectedValue.SetUtf8CP(expectedName);
        }
    bool operator()(IECInstancePtr const& actualElement)
        {
        actualElement->GetValue(m_actualValue, "Name");
        return m_expectedValue.Equals(m_actualValue);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSqlMetadataQueryTestFixture::ComparePropertyDefProperties(ECPropertyCR expectedProperty, IECInstanceCR actualProperty)
    {
    ECValue v;
    ASSERT_EQ(ECObjectsStatus::Success, actualProperty.GetValue(v, "Name"));
    ASSERT_STREQ(expectedProperty.GetName().c_str(), v.GetUtf8CP()) << "ECPropertyDef.Name";

    ASSERT_EQ(ECObjectsStatus::Success, actualProperty.GetValue(v, "DisplayLabel"));
    if (expectedProperty.GetIsDisplayLabelDefined())
        ASSERT_STREQ(expectedProperty.GetDisplayLabel().c_str(), v.GetUtf8CP()) << "ECPropertyDef.DisplayLabel";
    else
        ASSERT_TRUE(v.IsNull()) << "ECPropertyDef.DisplayLabel";

    ASSERT_EQ(ECObjectsStatus::Success, actualProperty.GetValue(v, "Description"));
    ASSERT_STREQ(expectedProperty.GetDescription().c_str(), v.GetUtf8CP()) << "ECPropertyDef.Description";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSqlMetadataQueryTestFixture::ComparePropertyDefLists(bvector<ECPropertyCP> & expectedProperties, bvector<IECInstancePtr> & actualProperties)
    {
    for (ECPropertyCP const& expectedProperty : expectedProperties)
        {
        FindMatchingNamePredicate predicate(expectedProperty->GetName().c_str());
        bvector<IECInstancePtr>::iterator matchedActualProperty = std::find_if(actualProperties.begin(), actualProperties.end(), predicate);

        if (actualProperties.end() != matchedActualProperty)
            {
            IECInstancePtr actualProperty = actualProperties[matchedActualProperty - actualProperties.begin()];
            ComparePropertyDefProperties(*expectedProperty, *actualProperty);

            actualProperties.erase(matchedActualProperty);
            }
        else
            ASSERT_TRUE(actualProperties.end() != matchedActualProperty) << "No match found for expected property " << expectedProperty->GetName().c_str();
        }

    for (IECInstancePtr const& actualProperty : actualProperties)
        {
        ECValue v;
        actualProperty->GetValue(v, "Name");
        ASSERT_TRUE(actualProperties.empty()) << "Unexpected actual property " << v.GetUtf8CP();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSqlMetadataQueryTestFixture::CompareClassDefProperties(ECClassCR expectedClass, IECInstanceCR actualClass)
    {
    ECValue v;
    ASSERT_EQ(ECObjectsStatus::Success, actualClass.GetValue(v, "Name"));

    ASSERT_STREQ(expectedClass.GetName().c_str(), v.GetUtf8CP()) << "ECClassDef.Name";

    ASSERT_EQ(ECObjectsStatus::Success, actualClass.GetValue(v, "DisplayLabel"));
    if (expectedClass.GetIsDisplayLabelDefined())
        ASSERT_STREQ(expectedClass.GetDisplayLabel().c_str(), v.GetUtf8CP()) << "ECClassDef.DisplayLabel";
    else
        ASSERT_TRUE(v.IsNull());

    ASSERT_EQ(ECObjectsStatus::Success, actualClass.GetValue(v, "Description"));
    ASSERT_STREQ(expectedClass.GetDescription().c_str(), v.GetUtf8CP()) << "ECClassDef.Description";

    ASSERT_EQ(ECObjectsStatus::Success, actualClass.GetValue(v, "Type"));
    ASSERT_EQ((int) expectedClass.GetClassType(), v.GetInteger()) << "ECClassDef.Type";

    ASSERT_EQ(ECObjectsStatus::Success, actualClass.GetValue(v, "Modifier"));
    ASSERT_EQ((int) expectedClass.GetClassModifier(), v.GetInteger()) << "ECClassDef.Modifier";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSqlMetadataQueryTestFixture::CompareClassDefLists(bvector<ECClassCP> & expectedClasses, bvector<IECInstancePtr> & actualClasses)
    {
    for (ECClassCP const& expectedClass : expectedClasses)
        {
        FindMatchingNamePredicate predicate(expectedClass->GetName().c_str());
        bvector<IECInstancePtr>::iterator matchedActualClass = std::find_if(actualClasses.begin(), actualClasses.end(), predicate);

        if (actualClasses.end() != matchedActualClass)
            {
            IECInstancePtr actualClass = actualClasses[matchedActualClass - actualClasses.begin()];
            CompareClassDefProperties(*expectedClass, *actualClass);

            bvector<ECPropertyCP> expectedProperties;
            for (ECPropertyCP const& expectedProperty : expectedClass->GetProperties(false))
                {
                expectedProperties.push_back(expectedProperty);
                }

            bvector<IECInstancePtr> actualProperties;
            Utf8String propQuery =
                "SELECT prop.* FROM ec.ECPropertyDef prop \
            JOIN ec.ECClassDef cl USING ec.ClassOwnsProperties \
            JOIN ec.ECSchemaDef sch USING ec.SchemaOwnsClasses \
            WHERE sch.Name='ECSqlTest' AND cl.Name='";
            propQuery += expectedClass->GetName().c_str();
            propQuery += "'";
            ECSqlStatement propStatement;
            ASSERT_EQ(ECSqlStatus::Success, propStatement.Prepare(GetECDb(), propQuery.c_str()));
            while (DbResult::BE_SQLITE_ROW == propStatement.Step())
                {
                ECInstanceECSqlSelectAdapter propAdapter(propStatement);
                actualProperties.push_back(propAdapter.GetInstance());
                }

            ComparePropertyDefLists(expectedProperties, actualProperties);
            actualClasses.erase(matchedActualClass);
            }
        else
            {
            EXPECT_TRUE(actualClasses.end() != matchedActualClass) << "No match found for expected class" << expectedClass->GetName().c_str() << "\n";
            }
        }
    ECValue v;
    for (IECInstancePtr const& actualClass : actualClasses)
        {
        actualClass->GetValue(v, "Name");
        EXPECT_TRUE(actualClasses.empty()) << "Unexpected actual class" << v.GetUtf8CP() << "\n";
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSqlMetadataQueryTestFixture::CompareSchemaDefProperties(ECSchemaCR expectedSchema, IECInstanceCR actualSchema)
    {
    ECValue v;
    ASSERT_EQ(ECObjectsStatus::Success, actualSchema.GetValue(v, "Name"));
    ASSERT_STREQ(expectedSchema.GetName().c_str(), v.GetUtf8CP()) << "ECSchema.Name";

    ASSERT_EQ(ECObjectsStatus::Success, actualSchema.GetValue(v, "DisplayLabel"));
    if (expectedSchema.GetIsDisplayLabelDefined())
        ASSERT_STREQ(expectedSchema.GetDisplayLabel().c_str(), v.GetUtf8CP()) << "ECSchemaDef.DisplayLabel";
    else
        ASSERT_TRUE(v.IsNull());

    ASSERT_EQ(ECObjectsStatus::Success, actualSchema.GetValue(v, "Description"));
    ASSERT_STREQ(expectedSchema.GetDescription().c_str(), v.GetUtf8CP()) << "ECSchemaDef.Description";

    ASSERT_EQ(ECObjectsStatus::Success, actualSchema.GetValue(v, "NameSpacePrefix"));
    ASSERT_STREQ(expectedSchema.GetNamespacePrefix().c_str(), v.GetUtf8CP()) << "ECSchemaDef.NamespacePrefix";

    ASSERT_EQ(ECObjectsStatus::Success, actualSchema.GetValue(v, "VersionMajor"));
    ASSERT_EQ(expectedSchema.GetVersionMajor(), v.GetInteger()) << "ECSchemaDef.VersionMajor";

    ASSERT_EQ(ECObjectsStatus::Success, actualSchema.GetValue(v, "VersionWrite"));
    ASSERT_EQ(expectedSchema.GetVersionWrite(), v.GetInteger()) << "ECSchemaDef.VersionWrite";

    ASSERT_EQ(ECObjectsStatus::Success, actualSchema.GetValue(v, "VersionMinor"));
    ASSERT_EQ(expectedSchema.GetVersionMinor(), v.GetInteger()) << "ECSchemaDef.VersionMinor";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlMetadataQueryTestFixture, VerifyQueries)
    {
    SetupECDb("metaschematests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    ECSchemaCP expectedSchema = GetECDb().Schemas().GetECSchema("ECSqlTest");

    ECSqlStatement schemaStatement;
    ASSERT_EQ(ECSqlStatus::Success, schemaStatement.Prepare(GetECDb(), "SELECT * FROM ec.ECSchemaDef WHERE Name='ECSqlTest'"));
    ASSERT_EQ(BE_SQLITE_ROW, schemaStatement.Step());
    ECInstanceECSqlSelectAdapter schemaAdapter(schemaStatement);
    IECInstancePtr actualSchema = schemaAdapter.GetInstance();

    CompareSchemaDefProperties(*expectedSchema, *actualSchema);

    bvector<ECClassCP> expectedClasses;
    for (ECClassCP const& expectedClass : expectedSchema->GetClasses())
        {
        expectedClasses.push_back(expectedClass);
        }

    bvector<IECInstancePtr> actualClasses;
    ECSqlStatement classStatement;
    ASSERT_EQ(ECSqlStatus::Success, classStatement.Prepare(GetECDb(), "SELECT c.* FROM ec.ECSchemaDef s "
                                                           "JOIN ec.ECClassDef c USING ec.SchemaOwnsClasses "
                                                           "WHERE s.Name='ECSqlTest'"));
    while (BE_SQLITE_ROW == classStatement.Step())
        {
        ECInstanceECSqlSelectAdapter classAdapter(classStatement);
        actualClasses.push_back(classAdapter.GetInstance());
        }

    CompareClassDefLists(expectedClasses, actualClasses);
    }

END_ECDBUNITTESTS_NAMESPACE