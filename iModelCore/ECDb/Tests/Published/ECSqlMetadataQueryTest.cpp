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
    bool operator () (IECInstancePtr const& actualElement)
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
    ECValue u, v;

    u.SetUtf8CP(expectedProperty.GetName().c_str());
    actualProperty.GetValue(v, "Name");
    EXPECT_TRUE(u.Equals(v));

    if (expectedProperty.GetIsDisplayLabelDefined())
        {
        u.SetUtf8CP(expectedProperty.GetDisplayLabel().c_str());
        actualProperty.GetValue(v, "DisplayLabel");
        EXPECT_TRUE(u.Equals(v));
        }
    else
        {
        actualProperty.GetValue(v, "DisplayLabel");
        EXPECT_TRUE(v.IsNull());
        }

    u.SetUtf8CP(expectedProperty.GetDescription().c_str());
    actualProperty.GetValue(v, "Description");
    EXPECT_TRUE(u.Equals(v));

    /* Not implemented in MetaSchema yet
    enum class ECPropertyKind
        {
        Primitive = 0,
        Struct = 1,
        PrimitiveArray = 2,
        StructArray = 3,
        Navigation = 4
        };

    actualProperty.GetValue(v, "Kind");
    ECPropertyKind propKind = (ECPropertyKind) v.GetInteger();

    if (propKind == ECPropertyKind::PrimitiveArray || propKind == ECPropertyKind::StructArray)
        {
        u.SetInteger(expectedProperty.GetAsArrayProperty()->GetMinOccurs());
        actualProperty.GetValue(v, "ArrayMinOccurs");
        EXPECT_TRUE(u.Equals(v));
        u.SetInteger(expectedProperty.GetAsArrayProperty()->GetStoredMaxOccurs());
        actualProperty.GetValue(v, "ArrayMaxOccurs");
        EXPECT_TRUE(u.Equals(v));

        if (ARRAYKIND_Primitive == expectedProperty.GetAsArrayProperty()->GetKind())
            {
            u.SetInteger(static_cast<int32_t>(expectedProperty.GetAsArrayProperty()->GetPrimitiveElementType()));
            actualProperty.GetValue(v, "PrimitiveType");
            EXPECT_TRUE(u.Equals(v));
            }
        else if (ARRAYKIND_Struct == expectedProperty.GetAsArrayProperty()->GetKind())
            {
            u.SetLong(expectedProperty.GetAsStructArrayProperty()->GetStructElementType()->GetId().GetValue());
            actualProperty.GetValue(v, "NonPrimitiveType");
            EXPECT_TRUE(u.Equals(v));
            }
        }
    else
        {
        actualProperty.GetValue(v, "ArrayMinOccurs");
        EXPECT_TRUE(v.IsNull());
        actualProperty.GetValue(v, "ArrayMaxOccurs");
        EXPECT_TRUE(v.IsNull());

        if (expectedProperty.GetIsPrimitive())
            {
            u.SetInteger(static_cast<int32_t>(expectedProperty.GetAsPrimitiveProperty()->GetType()));
            actualProperty.GetValue(v, "PrimitiveType");
            EXPECT_TRUE(u.Equals(v));
            }
        else
            {
            actualProperty.GetValue(v, "PrimitiveType");
            EXPECT_TRUE(v.IsNull());
            }

        if (expectedProperty.GetIsStruct())
            {
            u.SetLong(expectedProperty.GetAsStructProperty()->GetType().GetId().GetValue());
            actualProperty.GetValue(v, "NonPrimitiveType");
            EXPECT_TRUE(u.Equals(v));
            }
        else
            {
            actualProperty.GetValue(v, "NonPrimitiveType");
            EXPECT_TRUE(v.IsNull());
            }
        }

    u.SetBoolean(expectedProperty.GetIsReadOnly());
    actualProperty.GetValue(v, "IsReadOnly");
    EXPECT_TRUE(u.Equals(v));
    */
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSqlMetadataQueryTestFixture::ComparePropertyDefLists(bvector<ECPropertyCP> & expectedProperties, bvector<IECInstancePtr> & actualProperties)
    {
    for (ECPropertyCP const& expectedProperty : expectedProperties)
        {
#if defined (DEBUGGING)
        printf("   For expected property %s:\n", expectedProperty->GetName().c_str());
#endif
        FindMatchingNamePredicate predicate(expectedProperty->GetName().c_str());
        bvector<IECInstancePtr>::iterator matchedActualProperty = std::find_if(actualProperties.begin(), actualProperties.end(), predicate);

        if (actualProperties.end() != matchedActualProperty)
            {
            IECInstancePtr actualProperty = actualProperties[matchedActualProperty - actualProperties.begin()];
            ComparePropertyDefProperties(*expectedProperty, *actualProperty);

            actualProperties.erase(matchedActualProperty);
            }
        else
            {
            EXPECT_TRUE(actualProperties.end() != matchedActualProperty) <<
            "No match found for expected property " << expectedProperty->GetName().c_str() << "\n";
            }
        }
    ECValue v;
    for (IECInstancePtr const& actualProperty : actualProperties)
        {
        actualProperty->GetValue(v, "Name");
        EXPECT_TRUE(actualProperties.empty()) << "Unexpected actual property " << v.GetUtf8CP() << "\n";
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSqlMetadataQueryTestFixture::CompareClassDefProperties(ECClassCR expectedClass, IECInstanceCR actualClass)
    {
    ECValue u, v;

    u.SetUtf8CP(expectedClass.GetName().c_str());
    actualClass.GetValue(v, "Name");
    EXPECT_TRUE(u.Equals(v));

    if (expectedClass.GetIsDisplayLabelDefined())
        {
        u.SetUtf8CP(expectedClass.GetDisplayLabel().c_str());
        actualClass.GetValue(v, "DisplayLabel");
        EXPECT_TRUE(u.Equals(v));
        }
    else
        {
        actualClass.GetValue(v, "DisplayLabel");
        EXPECT_TRUE(v.IsNull());
        }

    u.SetUtf8CP(expectedClass.GetDescription().c_str());
    actualClass.GetValue(v, "Description");
    EXPECT_TRUE(u.Equals(v));

    u.SetInteger((int) expectedClass.GetClassType());
    actualClass.GetValue(v, "Type");
    EXPECT_TRUE(u.Equals(v));

    u.SetInteger((int) expectedClass.GetClassModifier());
    actualClass.GetValue(v, "Modifier");
    EXPECT_TRUE(u.Equals(v));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSqlMetadataQueryTestFixture::CompareClassDefLists(bvector<ECClassCP> & expectedClasses, bvector<IECInstancePtr> & actualClasses)
    {
    for (ECClassCP const& expectedClass : expectedClasses)
        {
#if defined (DEBUGGING)
        printf("  For expected class %s:\n", expectedClass->GetName().c_str());
#endif
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
#if defined (DEBUGGING)
                printf("   Expected property %s\n", expectedProperty->GetName().c_str());
#endif
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
#if defined (DEBUGGING)
                printf("   Actual property   %s\n", propStatement.GetValueText(1));
#endif
                }

#if defined (DEBUGGING)
            printf("    Expected property list size: %i\n    Actual property list size:   %i\n\n",
            (int)expectedProperties.size(), (int)actualProperties.size());
#endif
            ComparePropertyDefLists(expectedProperties, actualProperties);

            actualClasses.erase(matchedActualClass);
            }
        else
            {
            EXPECT_TRUE(actualClasses.end() != matchedActualClass) <<
            "No match found for expected class" << expectedClass->GetName().c_str() << "\n";
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
    ECValue u, v;

    u.SetUtf8CP(expectedSchema.GetName().c_str());
    actualSchema.GetValue(v, "Name");
    EXPECT_TRUE(u.Equals(v));

    if (expectedSchema.GetIsDisplayLabelDefined())
        {
        u.SetUtf8CP(expectedSchema.GetDisplayLabel().c_str());
        actualSchema.GetValue(v, "DisplayLabel");
        EXPECT_TRUE(u.Equals(v));
        }
    else
        {
        actualSchema.GetValue(v, "DisplayLabel");
        EXPECT_TRUE(v.IsNull());
        }

    u.SetUtf8CP(expectedSchema.GetDescription().c_str());
    actualSchema.GetValue(v, "Description");
    EXPECT_TRUE(u.Equals(v));

    u.SetUtf8CP(expectedSchema.GetNamespacePrefix().c_str());
    actualSchema.GetValue(v, "NameSpacePrefix");
    EXPECT_TRUE(u.Equals(v));

    u.SetInteger(expectedSchema.GetVersionMajor());
    actualSchema.GetValue(v, "VersionMajor");
    EXPECT_TRUE(u.Equals(v));

    u.SetInteger(expectedSchema.GetVersionMinor());
    actualSchema.GetValue(v, "VersionMinor");
    EXPECT_TRUE(u.Equals(v));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlMetadataQueryTestFixture, VerifyQueries)
    {
    SetupECDb("metaschematests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema"));
    ECDbSchemaManager const& manager = GetECDb().Schemas();
    ECSchemaCP expectedSchema = manager.GetECSchema("ECSqlTest");

    Utf8String schemaQuery =
    "SELECT ECSchemaDef.* FROM ec.ECSchemaDef \
    WHERE ECSchemaDef.Name='ECSqlTest'";
    ECSqlStatement schemaStatement;
    schemaStatement.Prepare(GetECDb(), schemaQuery.c_str());
    ASSERT_TRUE(BE_SQLITE_ROW == schemaStatement.Step());
    ECInstanceECSqlSelectAdapter schemaAdapter(schemaStatement);
    IECInstancePtr actualSchema = schemaAdapter.GetInstance();

    CompareSchemaDefProperties(*expectedSchema, *actualSchema);

    bvector<ECClassCP> expectedClasses;
    for (ECClassCP const& expectedClass : expectedSchema->GetClasses())
        {
        expectedClasses.push_back(expectedClass);
#if defined (DEBUGGING)
        printf("Expected class %s\n", expectedClass->GetName().c_str());
#endif
        }

    bvector<IECInstancePtr> actualClasses;
    Utf8String classQuery =
    "SELECT ECClassDef.* FROM ms.ECSchemaDef \
    JOIN ms.ECClassDef USING ms.SchemaHasClass \
    WHERE ECSchemaDef.Name='ECSqlTest'";
    ECSqlStatement classStatement;
    ASSERT_EQ (ECSqlStatus::Success, classStatement.Prepare(GetECDb(), classQuery.c_str()));
    while (DbResult::BE_SQLITE_ROW == classStatement.Step())
        {
        ECInstanceECSqlSelectAdapter classAdapter(classStatement);
        actualClasses.push_back(classAdapter.GetInstance());
#if defined (DEBUGGING)
        printf("Actual class   %s\n", classStatement.GetValueText(1));
#endif
        }

#if defined (DEBUGGING)
    printf(" Expected class list size: %i\n Actual class list size:   %i\n\n",
    (int)expectedClasses.size(), (int)actualClasses.size());
#endif
    CompareClassDefLists(expectedClasses, actualClasses);
    }

END_ECDBUNITTESTS_NAMESPACE