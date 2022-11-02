/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include <ECObjects/SchemaMerger.h>
#include <ECObjects/SchemaComparer.h> //for result comparisons
#include <Bentley/BeDirectoryIterator.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct SchemaMergerTests : ECTestFixture
    {};
struct MergerTestIssueListener : ECN::IIssueListener
    {
    mutable bvector<Utf8String> m_issues;

    void _OnIssueReported(ECN::IssueSeverity severity, ECN::IssueCategory category, ECN::IssueType type, Utf8CP message) const override
        {
        m_issues.push_back(message);
        }
    };

ECSchemaReadContextPtr InitializeReadContextWithAllSchemas(bvector<Utf8CP> const& schemasXml, bvector<ECSchemaCP>* loadedSchemas = nullptr)
    {
    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();

    for (auto schemaXml : schemasXml)
        {
        ECSchemaPtr schema;
        EXPECT_EQ (SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *readContext));
        if(loadedSchemas != nullptr)
            loadedSchemas->push_back(schema.get());
        }

    return readContext;
    }

void CompareResults(bvector<Utf8CP> const& expectedSchemasXml, SchemaMergeResult& actualResult, bool dumpFullSchemaOnError = false)
    {
    bvector<ECSchemaCP> expectedSchemas;
    ECSchemaReadContextPtr context = InitializeReadContextWithAllSchemas(expectedSchemasXml, &expectedSchemas);
    
    SchemaComparer comparer;
    SchemaComparer::Options options = SchemaComparer::Options(SchemaComparer::DetailLevel::NoSchemaElements, SchemaComparer::DetailLevel::NoSchemaElements);
    SchemaDiff diff;
    ASSERT_EQ(BentleyStatus::SUCCESS, comparer.Compare(diff, expectedSchemas, actualResult.GetResults(), options)) << "Failed to compare expected schemas to actual schemas";
    auto changes = diff.Changes();
    if(changes.IsChanged())
        {
        LOG.error("================================================================================");
        LOG.error("=Merged schema did not match expected result. Differences will be listed below.=");
        LOG.error("================================================================================");
        for(auto change : changes)
            {
            auto changeStr = change->ToString();
            LOG.error(changeStr.c_str());
            }

        if (dumpFullSchemaOnError)
            {
            LOG.error("================================================================================");
            LOG.error("=Actual Schemas as XML:                                                        =");
            LOG.error("================================================================================");
            for(auto result : actualResult.GetResults())
              {
              Utf8String schemaXml;
              result->WriteToXmlString(schemaXml);
              LOG.error(schemaXml.c_str());
              }
            }
        }
    


    ASSERT_EQ(false, changes.IsChanged()) << "Actual schemas did not match expected result";
    }

void CompareIssues(bvector<Utf8String> const& expectedIssues, bvector<Utf8String> const& loggedIssues)
    {
    bool issuesAreTheSame = (expectedIssues == loggedIssues);
    if(!issuesAreTheSame)
        {
        LOG.error("==================================================================================");
        LOG.error("=Reported issues did not match expected result. Differences will be listed below.=");
        LOG.error("==================================================================================");
        LOG.error("EXPECTED:");
        for(auto expected : expectedIssues)
            {
            LOG.errorv("    %s", expected.c_str());
            }
        LOG.error("ACTUAL:");
        for(auto actual : loggedIssues)
            {
            LOG.errorv("    %s", actual.c_str());
            }
        }

    ASSERT_TRUE(issuesAreTheSame) << "Logged issues did not match expected result";
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, Classes)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="SchemaMergeA" alias="sma" version="01.00.00" displayLabel="Schema Merge Test Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A" />
          <ECStructClass typeName="B" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="SchemaMergeA" alias="sma" version="01.00.00" displayLabel="Schema Merge Test Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="C" />
          <ECEntityClass typeName="D" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="SchemaMergeA" alias="sma" version="01.00.00" displayLabel="Schema Merge Test Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A" />
          <ECStructClass typeName="B" />
          <ECEntityClass typeName="C" />
          <ECEntityClass typeName="D" />
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, UpdateSchemaAndClassDescriptions)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity" />
          <ECStructClass typeName="MyStruct" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" description="Description" displayLabel="Schema Display Label" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity" description="Entity Description" displayLabel="Entity Label" modifier="Abstract" />
          <ECStructClass typeName="MyStruct" description="Struct Description" displayLabel="Struct Label" modifier="Abstract" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" description="Description" displayLabel="Schema Display Label" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity" description="Entity Description" displayLabel="Entity Label" modifier="Abstract" />
          <ECStructClass typeName="MyStruct" description="Struct Description" displayLabel="Struct Label" modifier="Abstract" />
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, Properties)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="string"/>
            <ECProperty propertyName="B" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="C" typeName="string"/>
            <ECProperty propertyName="D" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="string"/>
            <ECProperty propertyName="B" typeName="int"/>
            <ECProperty propertyName="C" typeName="string"/>
            <ECProperty propertyName="D" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, UpdatePropertyValues)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="int" description="Old Description" />
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="int" description="PropertyDescription." displayLabel="Property Display Label"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="int" description="Old Description" displayLabel="Property Display Label"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, PropertyWithDifferentCase)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="a" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, UpdatePropertyCategoryOnProperty)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <PropertyCategory typeName="Category1" priority="99"/>
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="int" category="Category1" />
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <PropertyCategory typeName="Category2" priority="98"/>
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="int"  category="Category2" />
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <PropertyCategory typeName="Category1" priority="99"/>
          <PropertyCategory typeName="Category2" priority="98"/>
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="int"  category="Category2" />
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }


/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, SetEnumAndCategoryWithDifferentCase)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <PropertyCategory typeName="Category1" priority="99"/>
          <ECEnumeration typeName="Enum1" backingTypeName="int" isStrict="true">
              <ECEnumerator name="a" value="0" displayLabel="Zero"/>
          </ECEnumeration>
          <ECEntityClass typeName="MyEntity">
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <PropertyCategory typeName="category1" priority="99"/>
          <ECEnumeration typeName="enum1" backingTypeName="int" isStrict="true">
              <ECEnumerator name="a" value="0" displayLabel="Zero"/>
              <ECEnumerator name="b" value="1" displayLabel="One"/>
          </ECEnumeration>
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="enum1"  category="category1" />
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <PropertyCategory typeName="Category1" priority="99"/>
          <ECEnumeration typeName="Enum1" backingTypeName="int" isStrict="true">
              <ECEnumerator name="a" value="0" displayLabel="Zero"/>
              <ECEnumerator name="b" value="1" displayLabel="One"/>
          </ECEnumeration>
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="Enum1"  category="Category1" />
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, UpdateKindOfQuantityOnProperty)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
          <UnitSystem typeName="SI" />
          <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" denominator="10.0" displayLabel="m" />
          <Format typeName="DefaultRealU" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
          <KindOfQuantity typeName="MYLENGTH" displayLabel="Area" persistenceUnit="M" presentationUnits="DefaultRealU[M]" relativeError="0.0001" />
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="int" kindOfQuantity="MYLENGTH" />
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
          <UnitSystem typeName="SI" />
          <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" denominator="10.0" displayLabel="m" />
          <Format typeName="DefaultRealU" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
          <KindOfQuantity typeName="MYLENGTH2" displayLabel="Area" persistenceUnit="M" presentationUnits="DefaultRealU[M]" relativeError="0.0001" />
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="int" kindOfQuantity="MYLENGTH2" />
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
          <UnitSystem typeName="SI" />
          <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" denominator="10.0" displayLabel="m" />
          <Format typeName="DefaultRealU" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
          <KindOfQuantity typeName="MYLENGTH" displayLabel="Area" persistenceUnit="M" presentationUnits="DefaultRealU[M]" relativeError="0.0001" />
          <KindOfQuantity typeName="MYLENGTH2" displayLabel="Area" persistenceUnit="M" presentationUnits="DefaultRealU[M]" relativeError="0.0001" />
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="int" kindOfQuantity="MYLENGTH2" />
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }


/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, EnumerationValues)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEnumeration typeName="MyEnum" backingTypeName="int" isStrict="true">
              <ECEnumerator name="a" value="0" displayLabel="Zero"/>
              <ECEnumerator name="b" value="2" displayLabel="Two"/>
          </ECEnumeration>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEnumeration typeName="MyEnum" backingTypeName="int" isStrict="true">
              <ECEnumerator name="c" value="1" displayLabel="One"/>
              <ECEnumerator name="d" value="3" displayLabel="Three"/>
          </ECEnumeration>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEnumeration typeName="MyEnum" backingTypeName="int" isStrict="true">
              <ECEnumerator name="a" value="0" displayLabel="Zero"/>
              <ECEnumerator name="b" value="2" displayLabel="Two"/>
              <ECEnumerator name="c" value="1" displayLabel="One"/>
              <ECEnumerator name="d" value="3" displayLabel="Three"/>
          </ECEnumeration>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, EnumerationValuesSameSchemaVersion)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEnumeration typeName="MyEnum" backingTypeName="int" isStrict="true">
              <ECEnumerator name="a" value="0" displayLabel="Zero"/>
              <ECEnumerator name="b" value="2" displayLabel="Two"/>
          </ECEnumeration>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEnumeration typeName="MyEnum" backingTypeName="int" isStrict="true">
              <ECEnumerator name="c" value="1" displayLabel="One"/>
              <ECEnumerator name="d" value="3" displayLabel="Three"/>
          </ECEnumeration>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEnumeration typeName="MyEnum" backingTypeName="int" isStrict="true">
              <ECEnumerator name="a" value="0" displayLabel="Zero"/>
              <ECEnumerator name="b" value="2" displayLabel="Two"/>
              <ECEnumerator name="c" value="1" displayLabel="One"/>
              <ECEnumerator name="d" value="3" displayLabel="Three"/>
          </ECEnumeration>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, EnumerationsFromBothSides)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEnumeration typeName="MyEnum1" backingTypeName="int" isStrict="true">
              <ECEnumerator name="a" value="0" displayLabel="Zero"/>
              <ECEnumerator name="b" value="2" displayLabel="Two"/>
          </ECEnumeration>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEnumeration typeName="MyEnum2" backingTypeName="int" isStrict="true">
              <ECEnumerator name="c" value="1" displayLabel="One"/>
              <ECEnumerator name="d" value="3" displayLabel="Three"/>
          </ECEnumeration>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEnumeration typeName="MyEnum1" backingTypeName="int" isStrict="true">
              <ECEnumerator name="a" value="0" displayLabel="Zero"/>
              <ECEnumerator name="b" value="2" displayLabel="Two"/>
          </ECEnumeration>
          <ECEnumeration typeName="MyEnum2" backingTypeName="int" isStrict="true">
              <ECEnumerator name="c" value="1" displayLabel="One"/>
              <ECEnumerator name="d" value="3" displayLabel="Three"/>
          </ECEnumeration>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, EnumeratorValues)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEnumeration typeName="MyEnum" backingTypeName="string" isStrict="true">
              <ECEnumerator name="SameNameDifferentValue" value="Old" displayLabel="Old Label"/>
              <ECEnumerator name="SameNameSameValue" value="SameNameSameValue" displayLabel="SameNameSameValue Label"/>
          </ECEnumeration>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEnumeration typeName="MyEnum" backingTypeName="string" isStrict="true">
              <ECEnumerator name="SameNameDifferentValue" value="New" displayLabel="New Label"/>
              <ECEnumerator name="SameNameSameValue" value="SameNameSameValue" displayLabel="SameNameSameValue Label"/>
          </ECEnumeration>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEnumeration typeName="MyEnum" backingTypeName="string" isStrict="true">
              <ECEnumerator name="SameNameDifferentValue" value="New" displayLabel="Old Label"/>
              <ECEnumerator name="SameNameSameValue" value="SameNameSameValue" displayLabel="SameNameSameValue Label"/>
          </ECEnumeration>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, EnumeratorDuplicateValues)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEnumeration typeName="MyEnum" backingTypeName="string" isStrict="true">
              <ECEnumerator name="DifferentNameSameValue1" value="DifferentNameSameValue" displayLabel="DifferentNameSameValue"/>
          </ECEnumeration>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEnumeration typeName="MyEnum" backingTypeName="string" isStrict="true">
              <ECEnumerator name="DifferentNameSameValue2" value="DifferentNameSameValue" displayLabel="DifferentNameSameValue"/>
          </ECEnumeration>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    MergerTestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Enumeration 'MySchema:MyEnum' ends up having duplicate enumerator values after merge, which is not allowed. Name of new Enumerator: DifferentNameSameValue2" };
    CompareIssues(expectedIssues, issues.m_issues);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, ChangeEnumerationType_ShouldFail)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEnumeration typeName="MyEnum" backingTypeName="int" isStrict="true">
              <ECEnumerator name="a" value="0" displayLabel="Zero"/>
              <ECEnumerator name="b" value="2" displayLabel="Two"/>
          </ECEnumeration>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEnumeration typeName="MyEnum" backingTypeName="string" isStrict="true">
              <ECEnumerator name="c" value="Zero" displayLabel="Zero"/>
              <ECEnumerator name="d" value="Two" displayLabel="Two"/>
          </ECEnumeration>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, PropertyCategoryValues)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <PropertyCategory typeName="Category1" displayLabel="Category 1" description="Description OLD" priority="95"/>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <PropertyCategory typeName="Category1" displayLabel="Category 1 NEW" description="Description NEW" priority="99"/>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <PropertyCategory typeName="Category1" displayLabel="Category 1" description="Description OLD" priority="99"/>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, PropertyCategoryFromBothSides)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <PropertyCategory typeName="Category1" displayLabel="Category 1" description="Description" priority="99"/>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <PropertyCategory typeName="Category2" displayLabel="Category 2" description="Description" priority="17"/>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <PropertyCategory typeName="Category1" displayLabel="Category 1" description="Description" priority="99"/>
          <PropertyCategory typeName="Category2" displayLabel="Category 2" description="Description" priority="17"/>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, PhenomenonValues)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Phenomenon typeName="CURRENT" definition="CURRENT" displayLabel="Current" description="Current" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Phenomenon typeName="ANGLE" definition="ANGLE" displayLabel="Angle" description="Current" />
          <Phenomenon typeName="CURRENT" definition="CURRENT" displayLabel="Current NEW" description="Current NEW" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Phenomenon typeName="CURRENT" definition="CURRENT" displayLabel="Current" description="Current" />
          <Phenomenon typeName="ANGLE" definition="ANGLE" displayLabel="Angle" description="Current" />
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, PhenomenonFromBothSides)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Phenomenon typeName="CURRENT" definition="CURRENT" displayLabel="Current" description="Current" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Phenomenon typeName="ANGLE" definition="ANGLE" displayLabel="Angle" description="Current" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Phenomenon typeName="CURRENT" definition="CURRENT" displayLabel="Current" description="Current" />
          <Phenomenon typeName="ANGLE" definition="ANGLE" displayLabel="Angle" description="Current" />
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, UnitSystemValues)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <UnitSystem typeName="METRIC" displayLabel="A" description="dA" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
         <UnitSystem typeName="METRIC" displayLabel="B" description="dB" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <UnitSystem typeName="METRIC" displayLabel="A" description="dA" />
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, UnitSystemFromBothSides)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <UnitSystem typeName="METRIC" displayLabel="A" description="dA" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <UnitSystem typeName="IMPERIAL" displayLabel="B" description="dB" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <UnitSystem typeName="METRIC" displayLabel="A" description="dA" />
          <UnitSystem typeName="IMPERIAL" displayLabel="B" description="dB" />
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, UnitValues)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
            <Phenomenon typeName="NUMBER" definition="NUMBER" displayLabel="Number" />
            <UnitSystem typeName="SI" />
            <UnitSystem typeName="METRIC" />
            <UnitSystem typeName="INTERNATIONAL" />
            <Unit typeName="ONE" phenomenon="NUMBER" unitSystem="INTERNATIONAL" definition="ONE" displayLabel="one" />
            <Constant typeName="MILLI" phenomenon="NUMBER" definition="ONE" numerator="1.0e-3" displayLabel="milli"/>
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" displayLabel="m" />
            <Unit typeName="MM" phenomenon="LENGTH" unitSystem="METRIC" definition="[MILLI]*M" displayLabel="mm" description="millimeter" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
            <Phenomenon typeName="NUMBER" definition="NUMBER" displayLabel="Number" />
            <UnitSystem typeName="SI" />
            <UnitSystem typeName="METRIC" />
            <UnitSystem typeName="INTERNATIONAL" />
            <Unit typeName="ONE" phenomenon="NUMBER" unitSystem="INTERNATIONAL" definition="ONE" displayLabel="one" />
            <Constant typeName="MILLI" phenomenon="NUMBER" definition="ONE" numerator="1.0e-3" displayLabel="milli"/>
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" displayLabel="m" />
            <Unit typeName="MM" phenomenon="LENGTH" unitSystem="METRIC" definition="[MILLI]*M" displayLabel="mm NEW" description="millim NEW" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
            <Phenomenon typeName="NUMBER" definition="NUMBER" displayLabel="Number" />
            <UnitSystem typeName="SI" />
            <UnitSystem typeName="METRIC" />
            <UnitSystem typeName="INTERNATIONAL" />
            <Unit typeName="ONE" phenomenon="NUMBER" unitSystem="INTERNATIONAL" definition="ONE" displayLabel="one" />
            <Constant typeName="MILLI" phenomenon="NUMBER" definition="ONE" numerator="1.0e-3" displayLabel="milli"/>
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" displayLabel="m" />
            <Unit typeName="MM" phenomenon="LENGTH" unitSystem="METRIC" definition="[MILLI]*M" displayLabel="mm" description="millimeter" />
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, InvertingUnitValues)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
            <Phenomenon typeName="SLOPE" definition="LENGTH*LENGTH(-1)" displayLabel="Slope" />
            <UnitSystem typeName="SI" />
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" displayLabel="m" />
            <Unit typeName="M_PER_M" phenomenon="SLOPE" unitSystem="SI" definition="M*M(-1)" displayLabel="m/m" />
            <InvertedUnit typeName="M_HORIZONTAL_PER_M_VERTICAL" invertsUnit="M_PER_M" unitSystem="SI" displayLabel="dl1" description="de1" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
            <Phenomenon typeName="SLOPE" definition="LENGTH*LENGTH(-1)" displayLabel="Slope" />
            <UnitSystem typeName="SI" />
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" displayLabel="m" />
            <Unit typeName="M_PER_M" phenomenon="SLOPE" unitSystem="SI" definition="M*M(-1)" displayLabel="m/m" />
            <InvertedUnit typeName="M_HORIZONTAL_PER_M_VERTICAL" invertsUnit="M_PER_M" unitSystem="SI" displayLabel="dl2" description="de2" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
            <Phenomenon typeName="SLOPE" definition="LENGTH*LENGTH(-1)" displayLabel="Slope" />
            <UnitSystem typeName="SI" />
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" displayLabel="m" />
            <Unit typeName="M_PER_M" phenomenon="SLOPE" unitSystem="SI" definition="M*M(-1)" displayLabel="m/m" />
            <InvertedUnit typeName="M_HORIZONTAL_PER_M_VERTICAL" invertsUnit="M_PER_M" unitSystem="SI" displayLabel="dl1" description="de1" />
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, UnitsWithReversedDependencyOrder) //Checks that stuff works of dependency order is reversed
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
            <Phenomenon typeName="SLOPE" definition="LENGTH*LENGTH(-1)" displayLabel="Slope" />
            <UnitSystem typeName="SI" />
            <InvertedUnit typeName="M_HORIZONTAL_PER_M_VERTICAL" invertsUnit="M_PER_M" unitSystem="SI" />
            <Unit typeName="M_PER_M" phenomenon="SLOPE" unitSystem="SI" definition="M*M(-1)" displayLabel="m/m" />
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" displayLabel="m" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
            <Phenomenon typeName="SLOPE" definition="LENGTH*LENGTH(-1)" displayLabel="Slope" />
            <UnitSystem typeName="SI" />
            <InvertedUnit typeName="M2_HORIZONTAL_PER_M2_VERTICAL" invertsUnit="M2_PER_M2" unitSystem="SI" />
            <Unit typeName="M2_PER_M2" phenomenon="SLOPE" unitSystem="SI" definition="M2*M2(-1)" displayLabel="m/m" />
            <Unit typeName="M2" phenomenon="LENGTH" unitSystem="SI" definition="M2" displayLabel="m" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
            <Phenomenon typeName="SLOPE" definition="LENGTH*LENGTH(-1)" displayLabel="Slope" />
            <UnitSystem typeName="SI" />
            <InvertedUnit typeName="M_HORIZONTAL_PER_M_VERTICAL" invertsUnit="M_PER_M" unitSystem="SI" />
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" displayLabel="m" />
            <Unit typeName="M_PER_M" phenomenon="SLOPE" unitSystem="SI" definition="M*M(-1)" displayLabel="m/m" />
            <InvertedUnit typeName="M2_HORIZONTAL_PER_M2_VERTICAL" invertsUnit="M2_PER_M2" unitSystem="SI" />
            <Unit typeName="M2" phenomenon="LENGTH" unitSystem="SI" definition="M2" displayLabel="m" />
            <Unit typeName="M2_PER_M2" phenomenon="SLOPE" unitSystem="SI" definition="M2*M2(-1)" displayLabel="m/m" />
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, UpdateSystemOnUnit)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
            <UnitSystem typeName="SI" />
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" displayLabel="m" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
            <UnitSystem typeName="SI2" />
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI2" definition="M" displayLabel="m" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    MergerTestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Unit 'MySchema:M' has its UnitSystem changed. This is not supported." };
    CompareIssues(expectedIssues, issues.m_issues);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, UpdatePhenomenonOnUnit)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
            <UnitSystem typeName="SI" />
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" displayLabel="m" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="OTHERLENGTH" definition="OTHERLENGTH" displayLabel="Other Length" />
            <UnitSystem typeName="SI" />
            <Unit typeName="M" phenomenon="OTHERLENGTH" unitSystem="SI" definition="M" displayLabel="m" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    MergerTestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Unit 'MySchema:M' has its Phenomenon changed. This is not supported." };
    CompareIssues(expectedIssues, issues.m_issues);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, UpdateDefinitionOnUnit)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
            <UnitSystem typeName="SI" />
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" displayLabel="m" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
            <UnitSystem typeName="SI" />
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M(2)" displayLabel="m" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    MergerTestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Unit 'MySchema:M' has its Definition changed. This is not supported." };
    CompareIssues(expectedIssues, issues.m_issues);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, UpdateNumeratorOnUnit)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
            <UnitSystem typeName="SI" />
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" numerator="10.0" displayLabel="m" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
            <UnitSystem typeName="SI" />
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" numerator="5.0" displayLabel="m" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    MergerTestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Unit 'MySchema:M' has its Numerator changed. This is not supported." };
    CompareIssues(expectedIssues, issues.m_issues);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, UpdateDenominatorOnUnit)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
            <UnitSystem typeName="SI" />
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" denominator="10.0" displayLabel="m" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
            <UnitSystem typeName="SI" />
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" denominator="5.0" displayLabel="m" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    MergerTestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Unit 'MySchema:M' has its Denominator changed. This is not supported." };
    CompareIssues(expectedIssues, issues.m_issues);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, UpdateOffsetOnUnit)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
            <UnitSystem typeName="SI" />
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" offset="10.0" displayLabel="m" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
            <UnitSystem typeName="SI" />
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" offset="5.0" displayLabel="m" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    MergerTestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Unit 'MySchema:M' has its Offset changed. This is not supported." };
    CompareIssues(expectedIssues, issues.m_issues);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, UnitFromBothSides)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
            <UnitSystem typeName="SI" />
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" displayLabel="m" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="LENGTH2" definition="LENGTH2" displayLabel="Length2" />
            <UnitSystem typeName="SI2" />
            <Unit typeName="M2" phenomenon="LENGTH2" unitSystem="SI2" definition="M2" displayLabel="m2" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
            <UnitSystem typeName="SI" />
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" displayLabel="m" />
            <Phenomenon typeName="LENGTH2" definition="LENGTH2" displayLabel="Length2" />
            <UnitSystem typeName="SI2" />
            <Unit typeName="M2" phenomenon="LENGTH2" unitSystem="SI2" definition="M2" displayLabel="m2" />
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }


/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, BaseClassAndPropertyWithDifferentCase)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="X" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="a">
          </ECEntityClass>
          <ECEntityClass typeName="b">
            <BaseClass>a</BaseClass>
            <ECProperty propertyName="x" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="X" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, DescriptionAndLabelDifferentCaseName)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
            <ECProperty propertyName="X" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="a" displayLabel="label" description="description">
            <ECProperty propertyName="x" typeName="int" displayLabel="label" description="description" />
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A" displayLabel="label" description="description">
            <ECProperty propertyName="X" typeName="int" displayLabel="label" description="description" />
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, AddBaseProperty)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyBase">
          </ECEntityClass>
          <ECEntityClass typeName="MyConflict">
            <BaseClass>MyBase</BaseClass>
            <ECProperty propertyName="A" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyBase">
            <ECProperty propertyName="A" typeName="int"/>
          </ECEntityClass>
          <ECEntityClass typeName="MyConflict">
            <BaseClass>MyBase</BaseClass>
            <ECProperty propertyName="A" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyBase">
            <ECProperty propertyName="A" typeName="int"/>
          </ECEntityClass>
          <ECEntityClass typeName="MyConflict">
            <BaseClass>MyBase</BaseClass>
            <ECProperty propertyName="A" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, AddBaseProperty2Levels)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyBase2">
          </ECEntityClass>
          <ECEntityClass typeName="MyBase">
            <BaseClass>MyBase2</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="MyConflict">
            <BaseClass>MyBase</BaseClass>
            <ECProperty propertyName="A" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyBase2">
            <ECProperty propertyName="A" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyBase2">
            <ECProperty propertyName="A" typeName="int"/>
          </ECEntityClass>
          <ECEntityClass typeName="MyBase">
            <BaseClass>MyBase2</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="MyConflict">
            <BaseClass>MyBase</BaseClass>
            <ECProperty propertyName="A" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, ReferencesInCorrectOrder)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MyBaseSchema" alias="mybs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="MyBaseSchema" version="01.00.00" alias="mybs"/>
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="string"/>
            <ECProperty propertyName="B" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MyBaseSchema" alias="mybs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="BaseEntity">
            <ECProperty propertyName="A" typeName="string"/>
            <ECProperty propertyName="B" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="MyBaseSchema" version="01.00.00" alias="mybs"/>
          <ECEntityClass typeName="DerivedEntity">
            <BaseClass>mybs:BaseEntity</BaseClass>
            <ECProperty propertyName="C" typeName="string"/>
            <ECProperty propertyName="D" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MyBaseSchema" alias="mybs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="BaseEntity">
            <ECProperty propertyName="A" typeName="string"/>
            <ECProperty propertyName="B" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="MyBaseSchema" version="01.00.00" alias="mybs"/>
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="string"/>
            <ECProperty propertyName="B" typeName="int"/>
          </ECEntityClass>
          <ECEntityClass typeName="DerivedEntity">
            <BaseClass>mybs:BaseEntity</BaseClass>
            <ECProperty propertyName="C" typeName="string"/>
            <ECProperty propertyName="D" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }


/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, ReferencesInWrongOrder) //we expect the merger to sort the input schemas
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MyBaseSchema" alias="mybs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="MyBaseSchema" version="01.00.00" alias="mybs"/>
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="string"/>
            <ECProperty propertyName="B" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemasRaw = leftContext->GetCache().GetSchemas();
    bvector<ECN::ECSchemaCP> leftSchemas;
    leftSchemas.push_back(leftSchemasRaw[1]);
    leftSchemas.push_back(leftSchemasRaw[0]);

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MyBaseSchema" alias="mybs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="BaseEntity">
            <ECProperty propertyName="A" typeName="string"/>
            <ECProperty propertyName="B" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="MyBaseSchema" version="01.00.00" alias="mybs"/>
          <ECEntityClass typeName="DerivedEntity">
            <BaseClass>mybs:BaseEntity</BaseClass>
            <ECProperty propertyName="C" typeName="string"/>
            <ECProperty propertyName="D" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemasRaw = rightContext->GetCache().GetSchemas();
    bvector<ECN::ECSchemaCP> rightSchemas;
    rightSchemas.push_back(rightSchemasRaw[1]);
    rightSchemas.push_back(rightSchemasRaw[0]);
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MyBaseSchema" alias="mybs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="BaseEntity">
            <ECProperty propertyName="A" typeName="string"/>
            <ECProperty propertyName="B" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="MyBaseSchema" version="01.00.00" alias="mybs"/>
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="string"/>
            <ECProperty propertyName="B" typeName="int"/>
          </ECEntityClass>
          <ECEntityClass typeName="DerivedEntity">
            <BaseClass>mybs:BaseEntity</BaseClass>
            <ECProperty propertyName="C" typeName="string"/>
            <ECProperty propertyName="D" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }


/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, NotIncludedReferencedSchema) // We expect the merger to automatically include referenced schemas if they are not provided as input
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MyBaseSchema" alias="mybs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="MyBaseSchema" version="01.00.00" alias="mybs"/>
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="string"/>
            <ECProperty propertyName="B" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemasRaw = leftContext->GetCache().GetSchemas();
    bvector<ECN::ECSchemaCP> leftSchemas;
    leftSchemas.push_back(leftSchemasRaw[1]);

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MyBaseSchema" alias="mybs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="BaseEntity">
            <ECProperty propertyName="A" typeName="string"/>
            <ECProperty propertyName="B" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="MyBaseSchema" version="01.00.00" alias="mybs"/>
          <ECEntityClass typeName="DerivedEntity">
            <BaseClass>mybs:BaseEntity</BaseClass>
            <ECProperty propertyName="C" typeName="string"/>
            <ECProperty propertyName="D" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemasRaw = rightContext->GetCache().GetSchemas();
    bvector<ECN::ECSchemaCP> rightSchemas;
    rightSchemas.push_back(rightSchemasRaw[1]);
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MyBaseSchema" alias="mybs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="BaseEntity">
            <ECProperty propertyName="A" typeName="string"/>
            <ECProperty propertyName="B" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="MyBaseSchema" version="01.00.00" alias="mybs"/>
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="string"/>
            <ECProperty propertyName="B" typeName="int"/>
          </ECEntityClass>
          <ECEntityClass typeName="DerivedEntity">
            <BaseClass>mybs:BaseEntity</BaseClass>
            <ECProperty propertyName="C" typeName="string"/>
            <ECProperty propertyName="D" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, AddBaseClass)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="string"/>
            <ECProperty propertyName="B" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Base">
            <ECProperty propertyName="C" typeName="string"/>
            <ECProperty propertyName="D" typeName="int"/>
          </ECEntityClass>
          <ECEntityClass typeName="MyEntity">
            <BaseClass>Base</BaseClass>
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Base">
            <ECProperty propertyName="C" typeName="string"/>
            <ECProperty propertyName="D" typeName="int"/>
          </ECEntityClass>
          <ECEntityClass typeName="MyEntity">
            <BaseClass>Base</BaseClass>
            <ECProperty propertyName="A" typeName="string"/>
            <ECProperty propertyName="B" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, AddBaseClassBelow) //checks that the order in which the new base appears does not matter
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="string"/>
            <ECProperty propertyName="B" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <BaseClass>Base</BaseClass>
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="Base">
            <ECProperty propertyName="C" typeName="string"/>
            <ECProperty propertyName="D" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <BaseClass>Base</BaseClass>
            <ECProperty propertyName="A" typeName="string"/>
            <ECProperty propertyName="B" typeName="int"/>
          </ECEntityClass>
          <ECEntityClass typeName="Base">
            <ECProperty propertyName="C" typeName="string"/>
            <ECProperty propertyName="D" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, AddNewClassWithBaseClass)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Base">
            <ECProperty propertyName="C" typeName="string"/>
            <ECProperty propertyName="D" typeName="int"/>
          </ECEntityClass>
          <ECEntityClass typeName="MyEntity">
            <BaseClass>Base</BaseClass>
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <BaseClass>Base</BaseClass>
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="Base">
            <ECProperty propertyName="C" typeName="string"/>
            <ECProperty propertyName="D" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }


/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, AddNewClassWithBaseClassInReverseOrder)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <BaseClass>Base</BaseClass>
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="Base">
            <ECProperty propertyName="C" typeName="string"/>
            <ECProperty propertyName="D" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    

    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <BaseClass>Base</BaseClass>
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="Base">
            <ECProperty propertyName="C" typeName="string"/>
            <ECProperty propertyName="D" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, InjectBaseClassInHierarchy)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Base">
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="MyEntity">
            <BaseClass>Base</BaseClass>
            <ECProperty propertyName="X" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Base">
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="NewBase">
            <BaseClass>Base</BaseClass>
            <ECProperty propertyName="B" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="MyEntity">
            <BaseClass>NewBase</BaseClass>
            <ECProperty propertyName="X" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Base">
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="NewBase">
            <BaseClass>Base</BaseClass>
            <ECProperty propertyName="B" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="MyEntity">
            <BaseClass>NewBase</BaseClass>
            <ECProperty propertyName="X" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, KindOfQuantityFromBothSides)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
          <UnitSystem typeName="SI" />
          <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" denominator="10.0" displayLabel="m" />
          <Format typeName="DefaultRealU" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
          <KindOfQuantity typeName="MYLENGTH" persistenceUnit="M" presentationUnits="DefaultRealU(4)[M]" relativeError="0.0001"/>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
          <UnitSystem typeName="SI" />
          <UnitSystem typeName="USCUSTOM" />
          <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" denominator="10.0" displayLabel="m" />
          <Format typeName="DefaultRealU" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
          <KindOfQuantity typeName="MYLENGTH2" persistenceUnit="M" presentationUnits="DefaultRealU(4)[M]" relativeError="0.0001"/>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
          <UnitSystem typeName="SI" />
          <UnitSystem typeName="USCUSTOM" />
          <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" denominator="10.0" displayLabel="m" />
          <Format typeName="DefaultRealU" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
          <KindOfQuantity typeName="MYLENGTH" persistenceUnit="M" presentationUnits="DefaultRealU(4)[M]" relativeError="0.0001"/>
          <KindOfQuantity typeName="MYLENGTH2" persistenceUnit="M" presentationUnits="DefaultRealU(4)[M]" relativeError="0.0001"/>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, KindOfQuantityValues)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
          <UnitSystem typeName="SI" />
          <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" denominator="10.0" displayLabel="m" />
          <Format typeName="DefaultRealU" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
          <KindOfQuantity typeName="MYLENGTH" displayLabel="Area" description="Area Description" relativeError="2.0" persistenceUnit="M" presentationUnits="DefaultRealU[M]" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
          <UnitSystem typeName="SI" />
          <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" denominator="10.0" displayLabel="m" />
          <Format typeName="DefaultRealU" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
          <KindOfQuantity typeName="MYLENGTH" displayLabel="Area2" description="Area Description2" relativeError="1.0" persistenceUnit="M" presentationUnits="DefaultRealU[M]" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
          <UnitSystem typeName="SI" />
          <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" denominator="10.0" displayLabel="m" />
          <Format typeName="DefaultRealU" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
          <KindOfQuantity typeName="MYLENGTH" displayLabel="Area" description="Area Description" relativeError="1.0" persistenceUnit="M" presentationUnits="DefaultRealU[M]" />
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, UpdatePersistenceUnitOnKindOfQuantity)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
          <UnitSystem typeName="SI" />
          <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" denominator="10.0" displayLabel="m" />
          <Format typeName="DefaultRealU" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
          <KindOfQuantity typeName="MYLENGTH" persistenceUnit="M" presentationUnits="DefaultRealU[M]" relativeError="0.0001" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
          <UnitSystem typeName="SI" />
          <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" denominator="10.0" displayLabel="m" />
          <Unit typeName="KM" phenomenon="LENGTH" unitSystem="SI" definition="M" denominator="10.0" displayLabel="m" />
          <Format typeName="DefaultRealU" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
          <KindOfQuantity typeName="MYLENGTH" persistenceUnit="KM" presentationUnits="DefaultRealU[M]" relativeError="0.0001" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
          <UnitSystem typeName="SI" />
          <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" denominator="10.0" displayLabel="m" />
          <Unit typeName="KM" phenomenon="LENGTH" unitSystem="SI" definition="M" denominator="10.0" displayLabel="m" />
          <Format typeName="DefaultRealU" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
          <KindOfQuantity typeName="MYLENGTH" persistenceUnit="KM" presentationUnits="DefaultRealU[M]" relativeError="0.0001" />
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, MergePresentationFormatsOnKindOfQuantity)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
          <UnitSystem typeName="SI" />
          <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" denominator="10.0" displayLabel="m" />
          <Format typeName="Format1" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
          <Format typeName="Format2" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
          <Format typeName="Format3" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
          <KindOfQuantity typeName="MYLENGTH" persistenceUnit="M" presentationUnits="Format1[M];Format3[M]" relativeError="0.0001" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
          <UnitSystem typeName="SI" />
          <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" denominator="10.0" displayLabel="m" />
          <Format typeName="Format1" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
          <Format typeName="Format2" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
          <Format typeName="Format3" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
          <Format typeName="Format4" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
          <KindOfQuantity typeName="MYLENGTH" persistenceUnit="M" presentationUnits="Format1[M];Format2[M];Format4(2)[M]" relativeError="0.0001" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
          <UnitSystem typeName="SI" />
          <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" denominator="10.0" displayLabel="m" />
          <Format typeName="Format1" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
          <Format typeName="Format2" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
          <Format typeName="Format3" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
          <Format typeName="Format4" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
          <KindOfQuantity typeName="MYLENGTH" persistenceUnit="M" presentationUnits="Format1[M];Format3[M];Format2[M];Format4(2)[M]" relativeError="0.0001" />
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, FormatFromBothSides)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Format typeName="DefaultRealU" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Format typeName="DefaultRealU2" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Format typeName="DefaultRealU" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
          <Format typeName="DefaultRealU2" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, FormatValues)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Format typeName="DefaultRealU" displayLabel="Label1" description="Description1" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Format typeName="DefaultRealU"  displayLabel="Label2" description="Description2" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Format typeName="DefaultRealU"  displayLabel="Label1" description="Description1" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, FormatNumericSpecValues)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Format typeName="StationZ_100_2" type="station" precision="2" stationOffsetSize="2" minWidth="2" formatTraits="keepSingleZero|keepDecimalPoint|trailZeroes"/>
          <Format typeName="DefaultRealUNS" type="decimal" precision="6" uomSeparator="" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
          <Format typeName="Fractional" displayLabel="fract" type="fractional" precision="64" formatTraits="keepSingleZero|keepDecimalPoint"/>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Format typeName="StationZ_100_2" type="station" precision="3" stationOffsetSize="3" minWidth="3" formatTraits="keepSingleZero|keepDecimalPoint|trailZeroes"/>
          <Format typeName="DefaultRealUNS" type="decimal" precision="7" uomSeparator="," formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
          <Format typeName="Fractional" displayLabel="fract" type="fractional" precision="32" formatTraits="keepSingleZero"/>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <Format typeName="StationZ_100_2" type="station" precision="3" stationOffsetSize="3" minWidth="3" formatTraits="keepSingleZero|keepDecimalPoint|trailZeroes"/>
          <Format typeName="DefaultRealUNS" type="decimal" precision="7" uomSeparator="," formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
          <Format typeName="Fractional" displayLabel="fract" type="fractional" precision="32" formatTraits="keepSingleZero"/>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, AddReferenceToNewSchema)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MyBaseSchema" alias="mybs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="BaseEntity">
            <ECProperty propertyName="A" typeName="string"/>
            <ECProperty propertyName="B" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="MyBaseSchema" version="01.00.00" alias="mybs"/>
          <ECEntityClass typeName="DerivedEntity">
            <BaseClass>mybs:BaseEntity</BaseClass>
            <ECProperty propertyName="C" typeName="string"/>
            <ECProperty propertyName="D" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MyBaseSchema" alias="mybs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="BaseEntity">
            <ECProperty propertyName="A" typeName="string"/>
            <ECProperty propertyName="B" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="MyBaseSchema" version="01.00.00" alias="mybs"/>
          <ECEntityClass typeName="DerivedEntity">
            <BaseClass>mybs:BaseEntity</BaseClass>
            <ECProperty propertyName="C" typeName="string"/>
            <ECProperty propertyName="D" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, AddReferenceToExistingSchema)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MyBaseSchema" alias="mybs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MyBaseSchema" alias="mybs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="BaseEntity">
            <ECProperty propertyName="A" typeName="string"/>
            <ECProperty propertyName="B" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="MyBaseSchema" version="01.00.00" alias="mybs"/>
          <ECEntityClass typeName="DerivedEntity">
            <BaseClass>mybs:BaseEntity</BaseClass>
            <ECProperty propertyName="C" typeName="string"/>
            <ECProperty propertyName="D" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MyBaseSchema" alias="mybs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="BaseEntity">
            <ECProperty propertyName="A" typeName="string"/>
            <ECProperty propertyName="B" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="MyBaseSchema" version="01.00.00" alias="mybs"/>
          <ECEntityClass typeName="DerivedEntity">
            <BaseClass>mybs:BaseEntity</BaseClass>
            <ECProperty propertyName="C" typeName="string"/>
            <ECProperty propertyName="D" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, ClassesWithReversedDependencyOrder)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <BaseClass>ZBase</BaseClass>
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="ZBase">
            <ECProperty propertyName="C" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity2">
            <BaseClass>ZBase2</BaseClass>
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="ZBase2">
            <ECProperty propertyName="C" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <BaseClass>ZBase</BaseClass>
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="ZBase">
            <ECProperty propertyName="C" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="MyEntity2">
            <BaseClass>ZBase2</BaseClass>
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="ZBase2">
            <ECProperty propertyName="C" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, ModifiedSchemasOnResult)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="SchemaWithChanges" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Z">
            <ECProperty propertyName="C" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="SchemaWithoutChanges" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Z">
            <ECProperty propertyName="C" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="SchemaOnlyInLeft" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Z">
            <ECProperty propertyName="C" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="SchemaWithChanges" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Z">
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="SchemaWithoutChanges" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Z">
            <ECProperty propertyName="C" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="SchemaOnlyInRight" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Z">
            <ECProperty propertyName="C" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="SchemaWithChanges" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Z">
            <ECProperty propertyName="C" typeName="string"/>
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="SchemaWithoutChanges" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Z">
            <ECProperty propertyName="C" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="SchemaOnlyInLeft" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Z">
            <ECProperty propertyName="C" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="SchemaOnlyInRight" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Z">
            <ECProperty propertyName="C" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    auto modifiedSchemas = result.GetModifiedSchemas();
    ASSERT_EQ(1, modifiedSchemas.size());
    ASSERT_STREQ("SchemaWithChanges", modifiedSchemas[0]->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, MergeSchemaVersion)
    {
      const Utf8CP schemaXml(
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        </ECSchema>
        )schema"
      );
      auto MergeVersion = [&](Utf8CP versionLeft, Utf8CP versionRight)->Utf8String
          {
          Utf8PrintfString left(schemaXml, versionLeft);
          bvector<Utf8CP> leftSchemasXml{left.c_str()};
          ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
          bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

          Utf8PrintfString right(schemaXml, versionRight);
          bvector<Utf8CP> rightSchemasXml{right.c_str()};
          ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
          bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();

          SchemaMergeResult result;
          EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));
          auto merged = result.GetSchema("MySchema");
          return merged->GetSchemaKey().GetVersionString();
          };

    ASSERT_STREQ("02.00.00", MergeVersion("02.00.00", "01.01.01").c_str());
    ASSERT_STREQ("02.00.01", MergeVersion("02.00.01", "01.11.00").c_str());
    ASSERT_STREQ("02.00.00", MergeVersion("01.02.01", "02.00.00").c_str());
    ASSERT_STREQ("01.02.00", MergeVersion("01.01.01", "01.02.00").c_str());
    ASSERT_STREQ("01.02.00", MergeVersion("01.02.00", "01.01.01").c_str());
    ASSERT_STREQ("01.02.01", MergeVersion("01.02.01", "01.01.00").c_str());
    ASSERT_STREQ("01.02.01", MergeVersion("01.01.00", "01.02.01").c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, KeepSchemaVersion)
    {
      const Utf8CP schemaXml(
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        </ECSchema>
        )schema"
      );
      auto MergeVersion = [&](Utf8CP versionLeft, Utf8CP versionRight)->Utf8String
          {
          Utf8PrintfString left(schemaXml, versionLeft);
          bvector<Utf8CP> leftSchemasXml{left.c_str()};
          ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
          bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

          Utf8PrintfString right(schemaXml, versionRight);
          bvector<Utf8CP> rightSchemasXml{right.c_str()};
          ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
          bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();

          SchemaMergeResult result;
          SchemaMergeOptions options;
          options.SetKeepVersion(true);
          EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas, options));
          auto merged = result.GetSchema("MySchema");
          return merged->GetSchemaKey().GetVersionString();
          };

    ASSERT_STREQ("02.00.00", MergeVersion("02.00.00", "01.01.01").c_str());
    ASSERT_STREQ("02.00.01", MergeVersion("02.00.01", "01.11.00").c_str());
    ASSERT_STREQ("01.02.01", MergeVersion("01.02.01", "02.00.00").c_str());
    ASSERT_STREQ("01.01.01", MergeVersion("01.01.01", "01.02.00").c_str());
    ASSERT_STREQ("01.02.00", MergeVersion("01.02.00", "01.01.01").c_str());
    ASSERT_STREQ("01.02.01", MergeVersion("01.02.01", "01.01.00").c_str());
    ASSERT_STREQ("01.01.00", MergeVersion("01.01.00", "01.02.01").c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, Property_ChangeType)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    {
    SchemaMergeResult result;
    MergerTestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Property MySchema:MyEntity:A has its type changed."};
    CompareIssues(expectedIssues, issues.m_issues);
    }
    {
    SchemaMergeResult result;
    SchemaMergeOptions options;
    options.SetIgnoreIncompatiblePropertyTypeChanges(true);
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas, options));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, PropertyNameConflict_ChangePropertyToEnumeration)
    {
      //TODO: ECDb supports changing a property into a *non-strict* enum. The Merger should eventually permit this scenario.
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="MyEnum"/>
          </ECEntityClass>
          <ECEnumeration typeName="MyEnum" backingTypeName="int" isStrict="true">
              <ECEnumerator name="a" value="0" displayLabel="Zero"/>
              <ECEnumerator name="b" value="2" displayLabel="Two"/>
          </ECEnumeration>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    MergerTestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Property MySchema:MyEntity:A has its type changed." };
    CompareIssues(expectedIssues, issues.m_issues);
    }


/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, PropertyNameConflict_PrimitiveAndArrayProperty)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <ECArrayProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    MergerTestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Property MySchema:MyEntity:A has mismatching types between both sides." };
    CompareIssues(expectedIssues, issues.m_issues);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, SchemaItemNameConflict_Enumeration)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyConflict">
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEnumeration typeName="MyConflict" backingTypeName="int" isStrict="true">
              <ECEnumerator name="a" value="0" displayLabel="Zero"/>
              <ECEnumerator name="b" value="2" displayLabel="Two"/>
          </ECEnumeration>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();

    //merge the schemas
    SchemaMergeResult result;
    MergerTestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Another item with name MySchema:MyConflict already exists in the merged schema MySchema.01.00.01. RenameSchemaItemOnConflict is set to false." };
    CompareIssues(expectedIssues, issues.m_issues);

    //merge again with resolve conflict flag
    SchemaMergeResult result2;
    SchemaMergeOptions options;
    options.SetRenameSchemaItemOnConflict(true);
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result2, leftSchemas, rightSchemas, options));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyConflict">
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
          <ECEnumeration typeName="MyConflict_" backingTypeName="int" isStrict="true">
              <ECEnumerator name="a" value="0" displayLabel="Zero"/>
              <ECEnumerator name="b" value="2" displayLabel="Two"/>
          </ECEnumeration>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, SchemaItemNameConflict_PropertyCategory)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyConflict">
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <PropertyCategory typeName="MyConflict" priority="98"/>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();

    //merge the schemas
    SchemaMergeResult result;
    MergerTestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Another item with name MySchema:MyConflict already exists in the merged schema MySchema.01.00.01. RenameSchemaItemOnConflict is set to false." };
    CompareIssues(expectedIssues, issues.m_issues);

    //merge again with resolve conflict flag
    SchemaMergeResult result2;
    SchemaMergeOptions options;
    options.SetRenameSchemaItemOnConflict(true);
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result2, leftSchemas, rightSchemas, options));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyConflict">
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
          <PropertyCategory typeName="MyConflict_" priority="98"/>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, SchemaItemNameConflict_Class)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <PropertyCategory typeName="MyConflict" priority="98"/>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyConflict">
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();

    //merge the schemas
    SchemaMergeResult result;
    MergerTestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Another item with name MySchema:MyConflict already exists in the merged schema MySchema.01.00.01. RenameSchemaItemOnConflict is set to false." };
    CompareIssues(expectedIssues, issues.m_issues);

    //merge again with resolve conflict flag
    SchemaMergeResult result2;
    SchemaMergeOptions options;
    options.SetRenameSchemaItemOnConflict(true);
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result2, leftSchemas, rightSchemas, options));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <PropertyCategory typeName="MyConflict" priority="98"/>
          <ECEntityClass typeName="MyConflict_">
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, SchemaItemNameConflict_EntityAndStruct)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECStructClass typeName="MyConflict">
            <ECProperty propertyName="A" typeName="string"/>
          </ECStructClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyConflict">
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();

    //merge the schemas
    SchemaMergeResult result;
    MergerTestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Cannot merge class MySchema:MyConflict because the type of class is different." };
    CompareIssues(expectedIssues, issues.m_issues);


    //merge the schemas
    SchemaMergeResult result2;
    MergerTestIssueListener issues2;
    result2.AddIssueListener(issues2);
    SchemaMergeOptions options;
    options.SetRenameSchemaItemOnConflict(true);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result2, leftSchemas, rightSchemas, options));

    // Compare issues
    bvector<Utf8String> expectedIssues2 { "Cannot merge class MySchema:MyConflict because the type of class is different." };
    CompareIssues(expectedIssues2, issues2.m_issues);

    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, PropertyNameConflict_DisconnectedClasses)
    {
    //This one has a class deriving from another, both classes are not deriving from each other in the incoming schema, and the incoming schema produces a conflict
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyBase">
          </ECEntityClass>
          <ECEntityClass typeName="MyConflict">
            <BaseClass>MyBase</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyBase">
            <ECProperty propertyName="A" typeName="int"/>
          </ECEntityClass>
          <ECEntityClass typeName="MyConflict">
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();

    //merge the schemas
    SchemaMergeResult result;
    MergerTestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Failed to add property A to class MySchema:MyConflict because it conflicts with another property. RenamePropertyOnConflict flag is set to false." };
    CompareIssues(expectedIssues, issues.m_issues);

    //merge again, with resolve conflict flag set to true
    SchemaMergeResult result2;
    SchemaMergeOptions options;
    options.SetRenamePropertyOnConflict(true);
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result2, leftSchemas, rightSchemas, options));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECv3ConversionAttributes" version="01.00.01" alias="V2ToV3"/>
          <ECEntityClass typeName="MyBase">
            <ECProperty propertyName="A" typeName="int"/>
          </ECEntityClass>
          <ECEntityClass typeName="MyConflict">
            <BaseClass>MyBase</BaseClass>
            <ECCustomAttributes>
              <RenamedPropertiesMapping xmlns="ECv3ConversionAttributes.01.00.01">
                  <PropertyMapping>A|A_1</PropertyMapping>
              </RenamedPropertiesMapping>
            </ECCustomAttributes>
            <ECProperty propertyName="A_1" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result2);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, PropertyNameConflict_ConnectBaseClass)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyBase">
            <ECProperty propertyName="A" typeName="int"/>
          </ECEntityClass>
          <ECEntityClass typeName="MyConflict">
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyBase">
          </ECEntityClass>
          <ECEntityClass typeName="MyConflict">
            <BaseClass>MyBase</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    MergerTestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "New base class MySchema:MyBase is incompatible with properties on MySchema:MyConflict or its derived classes." };
    CompareIssues(expectedIssues, issues.m_issues);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, PropertyNameConflict_AddBaseClassWithIncomingProperty)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyConflict">
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyBase">
            <ECProperty propertyName="A" typeName="int"/>
          </ECEntityClass>
          <ECEntityClass typeName="MyConflict">
            <BaseClass>MyBase</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    MergerTestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "New base class MySchema:MyBase is incompatible with properties on MySchema:MyConflict or its derived classes." };
    CompareIssues(expectedIssues, issues.m_issues);

    //merge again, with resolve conflict flag set to true
    SchemaMergeResult result2;
    SchemaMergeOptions options;
    options.SetRenamePropertyOnConflict(true);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result2, leftSchemas, rightSchemas, options));
    //  We do not support this case for now, so there is no option to automatically rename/resolve this.
    }


/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, PropertyNameConflict_AddDerivedClassWithIncomingProperty)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyBase">
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyBase">
          </ECEntityClass>
          <ECEntityClass typeName="MyConflict">
            <BaseClass>MyBase</BaseClass>
            <ECProperty propertyName="A" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    MergerTestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Failed to copy class MySchema:MyConflict into merged schema" }; //TODO: This needs a better check!
    CompareIssues(expectedIssues, issues.m_issues);
    //TODO: This scenario implicitly happens inside CopyClass() so it does not handle conflicts
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, EnumerationTypeConflict)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEnumeration typeName="MyEnum" backingTypeName="int" isStrict="true">
              <ECEnumerator name="a" value="0" displayLabel="Zero"/>
              <ECEnumerator name="b" value="2" displayLabel="Two"/>
          </ECEnumeration>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEnumeration typeName="MyEnum" backingTypeName="string" isStrict="true">
              <ECEnumerator name="a" value="A" displayLabel="Zero"/>
              <ECEnumerator name="b" value="B" displayLabel="Two"/>
          </ECEnumeration>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    MergerTestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Enumeration 'MySchema:MyEnum' has its Type changed. This is not supported." };
    CompareIssues(expectedIssues, issues.m_issues);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, PropertyNameConflict_AddBaseProperty)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyBase">
          </ECEntityClass>
          <ECEntityClass typeName="MyConflict">
            <BaseClass>MyBase</BaseClass>
            <ECProperty propertyName="A" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyBase">
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    MergerTestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Failed to add property A to class MySchema:MyBase because it conflicts with another property. RenamePropertyOnConflict flag is set to false." };
    CompareIssues(expectedIssues, issues.m_issues);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, PropertyNameConflict_AddDerivedProperty)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyBase">
            <ECProperty propertyName="A" typeName="int"/>
          </ECEntityClass>
          <ECEntityClass typeName="MyConflict">
            <BaseClass>MyBase</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyConflict">
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    MergerTestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Failed to add property A to class MySchema:MyConflict because it conflicts with another property. RenamePropertyOnConflict flag is set to false." };
    CompareIssues(expectedIssues, issues.m_issues);

    //merge again, with resolve conflict flag set to true
    SchemaMergeResult result2;
    SchemaMergeOptions options;
    options.SetRenamePropertyOnConflict(true);
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result2, leftSchemas, rightSchemas, options));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECv3ConversionAttributes" version="01.00.01" alias="V2ToV3"/>
          <ECEntityClass typeName="MyBase">
            <ECProperty propertyName="A" typeName="int"/>
          </ECEntityClass>
          <ECEntityClass typeName="MyConflict">
            <BaseClass>MyBase</BaseClass>
            <ECCustomAttributes>
              <RenamedPropertiesMapping xmlns="ECv3ConversionAttributes.01.00.01">
                  <PropertyMapping>A|A_1</PropertyMapping>
              </RenamedPropertiesMapping>
            </ECCustomAttributes>
            <ECProperty propertyName="A_1" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, PropertyNameConflict_AddBaseProperty2Levels)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyBaseBase">
          </ECEntityClass>
          <ECEntityClass typeName="MyBase">
            <BaseClass>MyBaseBase</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="MyConflict">
            <BaseClass>MyBase</BaseClass>
            <ECProperty propertyName="A" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyBaseBase">
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    MergerTestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Failed to add property A to class MySchema:MyBaseBase because it conflicts with another property. RenamePropertyOnConflict flag is set to false." };
    CompareIssues(expectedIssues, issues.m_issues);

    //merge again, with resolve conflict flag set to true
    SchemaMergeResult result2;
    SchemaMergeOptions options;
    options.SetRenamePropertyOnConflict(true);
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result2, leftSchemas, rightSchemas, options));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECv3ConversionAttributes" version="01.00.01" alias="V2ToV3"/>
          <ECEntityClass typeName="MyBaseBase">
            <ECCustomAttributes>
              <RenamedPropertiesMapping xmlns="ECv3ConversionAttributes.01.00.01">
                  <PropertyMapping>A|A_1</PropertyMapping>
              </RenamedPropertiesMapping>
            </ECCustomAttributes>
            <ECProperty propertyName="A_1" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="MyBase">
            <BaseClass>MyBaseBase</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="MyConflict">
            <BaseClass>MyBase</BaseClass>
            <ECProperty propertyName="A" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, PropertyNameConflict_AddDerivedProperty2Levels)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyBaseBase">
            <ECProperty propertyName="A" typeName="int"/>
          </ECEntityClass>
          <ECEntityClass typeName="MyBase">
            <BaseClass>MyBaseBase</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="MyConflict">
            <BaseClass>MyBase</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyConflict">
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    MergerTestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Failed to add property A to class MySchema:MyConflict because it conflicts with another property. RenamePropertyOnConflict flag is set to false." };
    CompareIssues(expectedIssues, issues.m_issues);


    //merge again, with resolve conflict flag set to true
    SchemaMergeResult result2;
    SchemaMergeOptions options;
    options.SetRenamePropertyOnConflict(true);
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result2, leftSchemas, rightSchemas, options));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECv3ConversionAttributes" version="01.00.01" alias="V2ToV3"/>
          <ECEntityClass typeName="MyBaseBase">
            <ECProperty propertyName="A" typeName="int"/>
          </ECEntityClass>
          <ECEntityClass typeName="MyBase">
            <BaseClass>MyBaseBase</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="MyConflict">
            <BaseClass>MyBase</BaseClass>
            <ECCustomAttributes>
              <RenamedPropertiesMapping xmlns="ECv3ConversionAttributes.01.00.01">
                  <PropertyMapping>A|A_1</PropertyMapping>
              </RenamedPropertiesMapping>
            </ECCustomAttributes>
            <ECProperty propertyName="A_1" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result2);
    }


/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, PropertyNameConflict_AddBasePropertyInReferencedSchema)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MyBaseSchema" alias="mybs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyBase">
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="MyBaseSchema" version="01.00.00" alias="mybs"/>
          <ECEntityClass typeName="MyConflict">
            <BaseClass>mybs:MyBase</BaseClass>
            <ECProperty propertyName="A" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MyBaseSchema" alias="mybs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyBase">
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="MyBaseSchema" version="01.00.00" alias="mybs"/>
          <ECEntityClass typeName="MyConflict">
            <BaseClass>mybs:MyBase</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    MergerTestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Failed to add property A to class MyBaseSchema:MyBase because it conflicts with another property. RenamePropertyOnConflict flag is set to false." };
    CompareIssues(expectedIssues, issues.m_issues);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, PropertyNameConflict_AddDerivedPropertyForReferencedSchema)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MyBaseSchema" alias="mybs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyBase">
            <ECProperty propertyName="A" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="MyBaseSchema" version="01.00.00" alias="mybs"/>
          <ECEntityClass typeName="MyConflict">
            <BaseClass>mybs:MyBase</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MyBaseSchema" alias="mybs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyBase">
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="MyBaseSchema" version="01.00.00" alias="mybs"/>
          <ECEntityClass typeName="MyConflict">
            <BaseClass>mybs:MyBase</BaseClass>
            <ECProperty propertyName="A" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    MergerTestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Failed to add property A to class MySchema:MyConflict because it conflicts with another property. RenamePropertyOnConflict flag is set to false." };
    CompareIssues(expectedIssues, issues.m_issues);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, ChangePropertyFromBinaryToIGeometry)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyClass">
            <ECProperty propertyName="A" typeName="binary"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyClass">
            <ECProperty propertyName="A" typeName="Bentley.Geometry.Common.IGeometry"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    MergerTestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Property MySchema:MyClass:A has its type changed." };
    CompareIssues(expectedIssues, issues.m_issues);

    //merge again, with keep left
    SchemaMergeResult result2;
    SchemaMergeOptions options;
    options.SetIgnoreIncompatiblePropertyTypeChanges(true);
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result2, leftSchemas, rightSchemas, options));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyClass">
            <ECProperty propertyName="A" typeName="binary"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, ChangeRoleLabel_KeepLeft)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyClass">
            <ECProperty propertyName="A" typeName="binary"/>
          </ECEntityClass>
          <ECRelationshipClass typeName="MyRelationshipClass" modifier="None" strength="referencing">
              <Source multiplicity="(0..*)" roleLabel="uses section" polymorphic="false">
                  <Class class="MyClass"/>
              </Source>
              <Target multiplicity="(0..1)" roleLabel="is part of built up section" polymorphic="true">
                  <Class class="MyClass"/>
              </Target>
          </ECRelationshipClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyClass">
            <ECProperty propertyName="A" typeName="binary"/>
          </ECEntityClass>
          <ECRelationshipClass typeName="MyRelationshipClass" modifier="None" strength="referencing">
              <Source multiplicity="(0..*)" roleLabel="uses section *NEW*" polymorphic="false">
                  <Class class="MyClass"/>
              </Source>
              <Target multiplicity="(0..1)" roleLabel="is part of built up section" polymorphic="true">
                  <Class class="MyClass"/>
              </Target>
          </ECRelationshipClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyClass">
            <ECProperty propertyName="A" typeName="binary"/>
          </ECEntityClass>
          <ECRelationshipClass typeName="MyRelationshipClass" modifier="None" strength="referencing">
              <Source multiplicity="(0..*)" roleLabel="uses section" polymorphic="false">
                  <Class class="MyClass"/>
              </Source>
              <Target multiplicity="(0..1)" roleLabel="is part of built up section" polymorphic="true">
                  <Class class="MyClass"/>
              </Target>
          </ECRelationshipClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, MergeRelationshipConstraints)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="ABase">
          </ECEntityClass>
          <ECEntityClass typeName="A1">
            <BaseClass>ABase</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="A2">
            <BaseClass>ABase</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="A3">
            <BaseClass>ABase</BaseClass>
          </ECEntityClass>
          <ECRelationshipClass typeName="MyRelationshipClass" modifier="None" strength="referencing">
              <Source multiplicity="(0..*)" roleLabel="uses section" polymorphic="true" abstractConstraint="ABase">
                  <Class class="A1"/>
                  <Class class="A2"/>
              </Source>
              <Target multiplicity="(0..1)" roleLabel="is part of built up section" polymorphic="true" abstractConstraint="ABase">
                  <Class class="A2"/>
                  <Class class="A3"/>
              </Target>
          </ECRelationshipClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="ABase">
          </ECEntityClass>
          <ECEntityClass typeName="A1">
            <BaseClass>ABase</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="A2">
            <BaseClass>ABase</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="A4">
            <BaseClass>ABase</BaseClass>
          </ECEntityClass>
          <ECRelationshipClass typeName="MyRelationshipClass" modifier="None" strength="referencing" >
              <Source multiplicity="(0..*)" roleLabel="uses section" polymorphic="true" abstractConstraint="ABase">
                  <Class class="A1"/>
                  <Class class="A4"/>
              </Source>
              <Target multiplicity="(0..1)" roleLabel="is part of built up section" polymorphic="true" abstractConstraint="ABase">
                  <Class class="A2"/>
                  <Class class="A4"/>
              </Target>
          </ECRelationshipClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="ABase">
          </ECEntityClass>
          <ECEntityClass typeName="A1">
            <BaseClass>ABase</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="A2">
            <BaseClass>ABase</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="A3">
            <BaseClass>ABase</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="A4">
            <BaseClass>ABase</BaseClass>
          </ECEntityClass>
          <ECRelationshipClass typeName="MyRelationshipClass" modifier="None" strength="referencing">
              <Source multiplicity="(0..*)" roleLabel="uses section" polymorphic="true" abstractConstraint="ABase">
                  <Class class="A1"/>
                  <Class class="A2"/>
                  <Class class="A4"/>
              </Source>
              <Target multiplicity="(0..1)" roleLabel="is part of built up section" polymorphic="true" abstractConstraint="ABase">
                  <Class class="A2"/>
                  <Class class="A3"/>
                  <Class class="A4"/>
              </Target>
          </ECRelationshipClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, ChangeAbstractConstraint)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="ABase">
          </ECEntityClass>
          <ECEntityClass typeName="A1">
            <BaseClass>ABase</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="A2">
            <BaseClass>ABase</BaseClass>
          </ECEntityClass>
          <ECRelationshipClass typeName="MyRelationshipClass" modifier="None" strength="referencing">
              <Source multiplicity="(0..*)" roleLabel="uses section" polymorphic="true" abstractConstraint="ABase">
                  <Class class="A1"/>
              </Source>
              <Target multiplicity="(0..1)" roleLabel="is part of built up section" polymorphic="true" abstractConstraint="ABase">
                  <Class class="A2"/>
              </Target>
          </ECRelationshipClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="ABaseBase">
          </ECEntityClass>
          <ECEntityClass typeName="ABase">
            <BaseClass>ABaseBase</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="A1">
            <BaseClass>ABase</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="A2">
            <BaseClass>ABase</BaseClass>
          </ECEntityClass>
          <ECRelationshipClass typeName="MyRelationshipClass" modifier="None" strength="referencing" >
              <Source multiplicity="(0..*)" roleLabel="uses section" polymorphic="true" abstractConstraint="ABaseBase">
                  <Class class="A1"/>
              </Source>
              <Target multiplicity="(0..1)" roleLabel="is part of built up section" polymorphic="true" abstractConstraint="ABaseBase">
                  <Class class="A2"/>
              </Target>
          </ECRelationshipClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="ABaseBase">
          </ECEntityClass>
          <ECEntityClass typeName="ABase">
            <BaseClass>ABaseBase</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="A1">
            <BaseClass>ABase</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="A2">
            <BaseClass>ABase</BaseClass>
          </ECEntityClass>
          <ECRelationshipClass typeName="MyRelationshipClass" modifier="None" strength="referencing" >
              <Source multiplicity="(0..*)" roleLabel="uses section" polymorphic="true" abstractConstraint="ABaseBase">
                  <Class class="A1"/>
              </Source>
              <Target multiplicity="(0..1)" roleLabel="is part of built up section" polymorphic="true" abstractConstraint="ABaseBase">
                  <Class class="A2"/>
              </Target>
          </ECRelationshipClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, ChangeAbstractConstraint_InvalidCase)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="ABase" />
          <ECEntityClass typeName="ABase2" />
          <ECEntityClass typeName="A1">
            <BaseClass>ABase</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="A2">
            <BaseClass>ABase2</BaseClass>
          </ECEntityClass>
          <ECRelationshipClass typeName="MyRelationshipClass" modifier="None" strength="referencing">
              <Source multiplicity="(0..*)" roleLabel="uses section" polymorphic="true" abstractConstraint="ABase">
                  <Class class="A1"/>
              </Source>
              <Target multiplicity="(0..1)" roleLabel="is part of built up section" polymorphic="true" abstractConstraint="ABase">
                  <Class class="A1"/>
              </Target>
          </ECRelationshipClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="ABase" />
          <ECEntityClass typeName="ABase2" />
          <ECEntityClass typeName="A1">
            <BaseClass>ABase</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="A2">
            <BaseClass>ABase2</BaseClass>
          </ECEntityClass>
          <ECRelationshipClass typeName="MyRelationshipClass" modifier="None" strength="referencing">
              <Source multiplicity="(0..*)" roleLabel="uses section" polymorphic="true" abstractConstraint="ABase">
                  <Class class="A1"/>
              </Source>
              <Target multiplicity="(0..1)" roleLabel="is part of built up section" polymorphic="true" abstractConstraint="ABase2">
                  <Class class="A2"/>
              </Target>
          </ECRelationshipClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    SchemaMergeResult result;
    MergerTestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Setting AbstractConstraint on MySchema:MyRelationshipClass failed. Was trying to set to MySchema:ABase2." };
    CompareIssues(expectedIssues, issues.m_issues);

    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, OriginalXmlVersion)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> schemas32Xml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="SchemaMergeA" alias="sma" version="01.00.00" displayLabel="Schema Merge Test Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="C" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr context32 = InitializeReadContextWithAllSchemas(schemas32Xml);
    bvector<ECN::ECSchemaCP> schemas32 = context32->GetCache().GetSchemas();
    ECSchemaCP schema32 = schemas32[0];

    bvector<Utf8CP> schemas31Xml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="SchemaMergeA" alias="sma" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
          <ECEntityClass typeName="C" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr context31 = InitializeReadContextWithAllSchemas(schemas31Xml);
    bvector<ECN::ECSchemaCP> schemas31 = context31->GetCache().GetSchemas();
    ECSchemaCP schema31 = schemas31[0];

    EXPECT_EQ(3, schema32->GetOriginalECXmlVersionMajor());
    EXPECT_EQ(2, schema32->GetOriginalECXmlVersionMinor());
    EXPECT_EQ(3, schema31->GetOriginalECXmlVersionMajor());
    EXPECT_EQ(1, schema31->GetOriginalECXmlVersionMinor());
    
    //merge the schemas
    {
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, schemas32, schemas31));

    auto mergedSchema = result.GetSchema("SchemaMergeA");
    EXPECT_EQ(3, mergedSchema->GetOriginalECXmlVersionMajor());
    EXPECT_EQ(2, mergedSchema->GetOriginalECXmlVersionMinor());
    }

    {
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, schemas31, schemas32));

    auto mergedSchema = result.GetSchema("SchemaMergeA");
    EXPECT_EQ(3, mergedSchema->GetOriginalECXmlVersionMajor());
    EXPECT_EQ(2, mergedSchema->GetOriginalECXmlVersionMinor());
    }

    //do another run with two 3.1 schemas, containing a different class so we actually merge.
    bvector<Utf8CP> schemas31Xml_ {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="SchemaMergeA" alias="sma" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
          <ECEntityClass typeName="A" />
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr context31_ = InitializeReadContextWithAllSchemas(schemas31Xml_);
    bvector<ECN::ECSchemaCP> schemas31_ = context31_->GetCache().GetSchemas();

    {
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, schemas31, schemas31_));

    auto mergedSchema = result.GetSchema("SchemaMergeA");
    EXPECT_EQ(3, mergedSchema->GetOriginalECXmlVersionMajor());
    EXPECT_EQ(1, mergedSchema->GetOriginalECXmlVersionMinor());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaMergerTests, CustomAttributesAddedFromReferencedSchema)
    {
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="RefMergeA" alias="refa" version="01.00.00" displayLabel="Schema Merge Ref Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECCustomAttributeClass typeName="isPhysical" />
        </ECSchema>
       )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="SchemaMergeA" alias="sma" version="01.00.00" displayLabel="Schema Merge Test Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="RefMergeA" version="01.00.00" alias="refa" />
          <ECEntityClass typeName="A" />
          <ECEntityClass typeName="B">
            <ECProperty propertyName="foo" typeName="string"/>
          </ECEntityClass> 
          <ECEntityClass typeName="C">
            <ECCustomAttributes>
              <isPhysical xmlns="RefMergeA.01.00.00" />
            </ECCustomAttributes>
          </ECEntityClass>
          <ECStructClass typeName="D" />
        </ECSchema>
        )schema"
        };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="RefMergeA" alias="refa" version="01.00.00" displayLabel="Schema Merge Ref Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECCustomAttributeClass typeName="isPhysical" />
          <ECCustomAttributeClass typeName="isVisible" />
        </ECSchema>
       )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="SchemaMergeA" alias="sma" version="01.00.00" displayLabel="Schema Merge Test Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
          <ECSchemaReference name="RefMergeA" version="01.00.00" alias="refa" />
          <ECCustomAttributes>
            <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
          </ECCustomAttributes>
          <ECEntityClass typeName="A">
            <ECCustomAttributes>
              <isVisible xmlns="RefMergeA.01.00.00" />
              <HiddenClass xmlns="CoreCustomAttributes.01.00.03"/>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <ECProperty propertyName="foo" typeName="string">
              <ECCustomAttributes>
                <HiddenProperty xmlns="CoreCustomAttributes.01.00.03"/>
              </ECCustomAttributes>
            </ECProperty>
          </ECEntityClass> 
          <ECEntityClass typeName="E">
            <ECCustomAttributes>
              <isVisible xmlns="RefMergeA.01.00.00" />
            </ECCustomAttributes>
          </ECEntityClass>
        </ECSchema>
        )schema"
        };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();

    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    result.GetSchemaCache().DropSchema(result.GetSchema("CoreCustomAttributes")->GetSchemaKey());
    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="RefMergeA" alias="refa" version="01.00.00" displayLabel="Schema Merge Ref Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECCustomAttributeClass typeName="isPhysical" />
          <ECCustomAttributeClass typeName="isVisible" />
        </ECSchema>
       )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="SchemaMergeA" alias="sma" version="01.00.00" displayLabel="Schema Merge Test Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
          <ECSchemaReference name="RefMergeA" version="01.00.00" alias="refa" />
          <ECCustomAttributes>
            <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
          </ECCustomAttributes>
          <ECEntityClass typeName="A">
            <ECCustomAttributes>
              <isVisible xmlns="RefMergeA.01.00.00" />
              <HiddenClass xmlns="CoreCustomAttributes.01.00.03"/>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <ECProperty propertyName="foo" typeName="string">
              <ECCustomAttributes>
                <HiddenProperty xmlns="CoreCustomAttributes.01.00.03"/>
              </ECCustomAttributes>
            </ECProperty>
          </ECEntityClass> 
          <ECEntityClass typeName="C">
            <ECCustomAttributes>
              <isPhysical xmlns="RefMergeA.01.00.00" />
            </ECCustomAttributes>
          </ECEntityClass>
          <ECStructClass typeName="D" />
          <ECEntityClass typeName="E">
            <ECCustomAttributes>
              <isVisible xmlns="RefMergeA.01.00.00" />
            </ECCustomAttributes>
          </ECEntityClass>
        </ECSchema>
        )schema"
        };

    CompareResults(expectedSchemasXml, result);
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaMergerTests, CustomAttributesAddedLocalSchema)
    {
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="SchemaMergeA" alias="sma" version="01.00.00" displayLabel="Schema Merge Test Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECCustomAttributeClass typeName="isPhysical" />
          <ECEntityClass typeName="A" />
          <ECEntityClass typeName="B">
            <ECProperty propertyName="foo" typeName="string"/>
          </ECEntityClass> 
          <ECEntityClass typeName="C">
            <ECCustomAttributes>
              <isPhysical xmlns="SchemaMergeA.01.00.00" />
            </ECCustomAttributes>
          </ECEntityClass>
          <ECStructClass typeName="D" />
        </ECSchema>
        )schema"
        };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="SchemaMergeA" alias="sma" version="01.00.00" displayLabel="Schema Merge Test Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
          <ECCustomAttributes>
            <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
          </ECCustomAttributes>
          <ECCustomAttributeClass typeName="isPhysical" />
          <ECCustomAttributeClass typeName="isVisible" />
          <ECEntityClass typeName="A">
            <ECCustomAttributes>
              <isVisible xmlns="SchemaMergeA.01.00.00" />
              <HiddenClass xmlns="CoreCustomAttributes.01.00.03"/>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <ECProperty propertyName="foo" typeName="string">
              <ECCustomAttributes>
                <HiddenProperty xmlns="CoreCustomAttributes.01.00.03"/>
              </ECCustomAttributes>
            </ECProperty>
          </ECEntityClass> 
          <ECEntityClass typeName="E">
            <ECCustomAttributes>
              <isVisible xmlns="SchemaMergeA.01.00.00" />
            </ECCustomAttributes>
          </ECEntityClass>
        </ECSchema>
        )schema"
        };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();

    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(BentleyStatus::SUCCESS, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    result.GetSchemaCache().DropSchema(result.GetSchema("CoreCustomAttributes")->GetSchemaKey());
    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="SchemaMergeA" alias="sma" version="01.00.00" displayLabel="Schema Merge Test Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
          <ECCustomAttributes>
            <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
          </ECCustomAttributes>
          <ECCustomAttributeClass typeName="isPhysical" />
          <ECCustomAttributeClass typeName="isVisible" />
          <ECEntityClass typeName="A">
            <ECCustomAttributes>
              <isVisible xmlns="SchemaMergeA.01.00.00" />
              <HiddenClass xmlns="CoreCustomAttributes.01.00.03"/>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <ECProperty propertyName="foo" typeName="string">
              <ECCustomAttributes>
                <HiddenProperty xmlns="CoreCustomAttributes.01.00.03"/>
              </ECCustomAttributes>
            </ECProperty>
          </ECEntityClass> 
          <ECEntityClass typeName="C">
            <ECCustomAttributes>
              <isPhysical xmlns="SchemaMergeA.01.00.00" />
            </ECCustomAttributes>
          </ECEntityClass>
          <ECStructClass typeName="D" />
          <ECEntityClass typeName="E">
            <ECCustomAttributes>
              <isVisible xmlns="SchemaMergeA.01.00.00" />
            </ECCustomAttributes>
          </ECEntityClass>
        </ECSchema>
        )schema"
        };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
/*TEST_F(SchemaMergerTests, MergeSchemasFromLocalDirectories)
    {
    BeFileName leftFolder(L"f:\\defects\\schemaDump_constraint\\Left\\");
    BeFileName rightFolder(L"f:\\defects\\schemaDump_constraint\\Right\\");

    NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::CONSOLE_LOGGING_PROVIDER);
    NativeLogging::LoggingConfig::SetSeverity("ECDb", BentleyApi::NativeLogging::LOG_TRACE);
    NativeLogging::LoggingConfig::SetSeverity("ECObjectsNative", BentleyApi::NativeLogging::LOG_TRACE);

    ECSchemaReadContextPtr leftContext = ECSchemaReadContext::CreateContext(false, true);
    leftContext->AddSchemaPath(leftFolder);
    {
    bvector<BeFileName> schemaPaths;
    BeDirectoryIterator::WalkDirsAndMatch(schemaPaths, leftFolder, L"*.ecschema.xml", false);
    for (BeFileName const& schemaXmlFile : schemaPaths)
        {
        ECN::ECSchemaPtr ecSchema = nullptr;
        ECN::ECSchema::ReadFromXmlFile(ecSchema, schemaXmlFile.GetName(), *leftContext);
        }
    }

    ECSchemaReadContextPtr rightContext = ECSchemaReadContext::CreateContext(false, true);
    rightContext->AddSchemaPath(rightFolder);
    {
    bvector<BeFileName> schemaPaths;
    BeDirectoryIterator::WalkDirsAndMatch(schemaPaths, rightFolder, L"*.ecschema.xml", false);
    for (BeFileName const& schemaXmlFile : schemaPaths)
        {
        ECN::ECSchemaPtr ecSchema = nullptr;
        ECN::ECSchema::ReadFromXmlFile(ecSchema, schemaXmlFile.GetName(), *rightContext);
        }
    }

    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    MergerTestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(BentleyStatus::ERROR, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Failed to add property A to class MySchema:MyConflict because it conflicts with another property. RenamePropertyOnConflict flag is set to false." };
    CompareIssues(expectedIssues, issues.m_issues);
    }*/

END_BENTLEY_ECN_TEST_NAMESPACE

