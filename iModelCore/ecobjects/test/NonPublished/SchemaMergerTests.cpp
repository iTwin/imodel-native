/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include <ECObjects/SchemaMerger.h>
#include <ECObjects/SchemaComparer.h> //for result comparisons
#include <Bentley/BeDirectoryIterator.h>
#include "iostream"

USING_NAMESPACE_BENTLEY_EC
using namespace NativeLogging;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct SchemaMergerTests : ECTestFixture
    {};

ECSchemaReadContextPtr InitializeReadContextWithAllSchemas(bvector<Utf8CP> const& schemasXml, bvector<ECSchemaCP>* loadedSchemas = nullptr, bool skipValidation = false)
    {
    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
    if(skipValidation)
      readContext->SetSkipValidation(true);

    for (auto schemaXml : schemasXml)
        {
        ECSchemaPtr schema;
        EXPECT_EQ (SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *readContext));
        if(loadedSchemas != nullptr)
            loadedSchemas->push_back(schema.get());
        }

    return readContext;
    }

void CompareResults(bvector<Utf8CP> const& expectedSchemasXml, SchemaMergeResult& actualResult, bool dumpFullSchemaOnError = false, ECVersion ecXmlVersion = ECVersion::Latest, bool skipValidation = false)
    {
    bvector<ECSchemaCP> expectedSchemas;
    ECSchemaReadContextPtr context = InitializeReadContextWithAllSchemas(expectedSchemasXml, &expectedSchemas, skipValidation);
    
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
              result->WriteToXmlString(schemaXml, ecXmlVersion);
              LOG.error(schemaXml.c_str());
              }
            }
        }

    ASSERT_EQ(false, changes.IsChanged()) << "Actual schemas did not match expected result";
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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::NamedItemAlreadyExists, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Enumeration 'MySchema:MyEnum' ends up having duplicate enumerator values after merge, which is not allowed. Name of new Enumerator: DifferentNameSameValue2" };
    issues.CompareIssues(expectedIssues);
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
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));
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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Unit 'MySchema:M' has its UnitSystem changed. This is not supported." };
    issues.CompareIssues(expectedIssues);
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
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Unit 'MySchema:M' has its Phenomenon changed. This is not supported." };
    issues.CompareIssues(expectedIssues);
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
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Unit 'MySchema:M' has its Definition changed. This is not supported." };
    issues.CompareIssues(expectedIssues);
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
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Unit 'MySchema:M' has its Numerator changed. This is not supported." };
    issues.CompareIssues(expectedIssues);
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
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Unit 'MySchema:M' has its Denominator changed. This is not supported." };
    issues.CompareIssues(expectedIssues);
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
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Unit 'MySchema:M' has its Offset changed. This is not supported." };
    issues.CompareIssues(expectedIssues);
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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
TEST_F(SchemaMergerTests, EntityWithMultipleBaseClasses)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="C">
            <BaseClass>B</BaseClass>
            <BaseClass>A</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="C">
            <BaseClass>A</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();

    // Merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="C">
            <BaseClass>B</BaseClass>
            <BaseClass>A</BaseClass>
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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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

TEST_F(SchemaMergerTests, NullReferencedItemPropertyCategory)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="MyEntity">
                <ECProperty propertyName="A" typeName="int" category="Category">
                </ECProperty>
            </ECEntityClass>
            <PropertyCategory typeName="Category" priority="99"/>
        </ECSchema>)schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="MyEntity">
                <ECProperty propertyName="A" typeName="int"/>
            </ECEntityClass>
        </ECSchema>)schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();

    // Merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="MyEntity">
                <ECProperty propertyName="A" typeName="int" category="Category">
                </ECProperty>
            </ECEntityClass>
            <PropertyCategory typeName="Category" priority="99"/>
        </ECSchema>)schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

TEST_F(SchemaMergerTests, NullReferencedItemKindOfQuantity)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
            <UnitSystem typeName="SI" />
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" denominator="10.0" displayLabel="m" />
            <Format typeName="DefaultRealU" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
            <KindOfQuantity typeName="DISTANCE" persistenceUnit="M" relativeError="0.0001" presentationUnits="DefaultRealU[M]"/>
            <ECEntityClass typeName="MyEntity">
                <ECProperty propertyName="A" typeName="int" kindOfQuantity="DISTANCE">
                </ECProperty>
            </ECEntityClass>
        </ECSchema>)schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="MyEntity">
                <ECProperty propertyName="A" typeName="int"/>
            </ECEntityClass>
        </ECSchema>)schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();

    // Merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="LENGTH" definition="LENGTH" displayLabel="Length" />
            <UnitSystem typeName="SI" />
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" denominator="10.0" displayLabel="m" />
            <Format typeName="DefaultRealU" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
            <KindOfQuantity typeName="DISTANCE" persistenceUnit="M" relativeError="0.0001" presentationUnits="DefaultRealU[M]"/>
            <ECEntityClass typeName="MyEntity">
                <ECProperty propertyName="A" typeName="int" kindOfQuantity="DISTANCE">
                </ECProperty>
            </ECEntityClass>
        </ECSchema>)schema"
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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
          EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));
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
          EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas, options));
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
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<ReportedIssue> expectedIssues { ReportedIssue(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSchema, ECIssueId::EC_0043, "Property MySchema:MyEntity:A has its type changed from string to int.")};
    issues.CompareIssues(expectedIssues);
    }
    {
    SchemaMergeResult result;
    TestIssueListener issues;
    result.AddIssueListener(issues);
    SchemaMergeOptions options;
    options.SetIgnoreIncompatiblePropertyTypeChanges(true);
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas, options));

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
    // Compare issues
    bvector<ReportedIssue> expectedIssues { ReportedIssue(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSchema, ECIssueId::EC_0058, "Ignoring invalid property type change on MySchema:MyEntity:A (from string to int) because IgnoreIncompatiblePropertyTypeChanges has been set.")};
    issues.CompareIssues(expectedIssues);
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
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Property MySchema:MyEntity:A has its type changed from int to MyEnum." };
    issues.CompareIssues(expectedIssues);
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
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Property MySchema:MyEntity:A is of a different kind between both sides. IsPrimitive changed from true to false IsPrimitiveArray changed from false to true " };
    issues.CompareIssues(expectedIssues);
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
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Another item with name MySchema:MyConflict already exists in the merged schema MySchema.01.00.01. RenameSchemaItemOnConflict is set to false." };
    issues.CompareIssues(expectedIssues);

    //merge again with resolve conflict flag
    SchemaMergeResult result2;
    SchemaMergeOptions options;
    options.SetRenameSchemaItemOnConflict(true);
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result2, leftSchemas, rightSchemas, options));

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
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Another item with name MySchema:MyConflict already exists in the merged schema MySchema.01.00.01. RenameSchemaItemOnConflict is set to false." };
    issues.CompareIssues(expectedIssues);

    //merge again with resolve conflict flag
    SchemaMergeResult result2;
    SchemaMergeOptions options;
    options.SetRenameSchemaItemOnConflict(true);
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result2, leftSchemas, rightSchemas, options));

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
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Another item with name MySchema:MyConflict already exists in the merged schema MySchema.01.00.01. RenameSchemaItemOnConflict is set to false." };
    issues.CompareIssues(expectedIssues);

    //merge again with resolve conflict flag
    SchemaMergeResult result2;
    SchemaMergeOptions options;
    options.SetRenameSchemaItemOnConflict(true);
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result2, leftSchemas, rightSchemas, options));

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
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Cannot merge class MySchema:MyConflict because the type of class is different." };
    issues.CompareIssues(expectedIssues);


    //merge the schemas
    SchemaMergeResult result2;
    TestIssueListener issues2;
    result2.AddIssueListener(issues2);
    SchemaMergeOptions options;
    options.SetRenameSchemaItemOnConflict(true);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result2, leftSchemas, rightSchemas, options));

    // Compare issues
    bvector<Utf8String> expectedIssues2 { "Cannot merge class MySchema:MyConflict because the type of class is different." };
    issues2.CompareIssues(expectedIssues2);
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
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Failed to add property A to class MySchema:MyConflict because it conflicts with another property. RenamePropertyOnConflict flag is set to false." };
    issues.CompareIssues(expectedIssues);

    //merge again, with resolve conflict flag set to true
    SchemaMergeResult result2;
    SchemaMergeOptions options;
    options.SetRenamePropertyOnConflict(true);
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result2, leftSchemas, rightSchemas, options));

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
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "New base class MySchema:MyBase is incompatible with properties on MySchema:MyConflict or its derived classes." };
    issues.CompareIssues(expectedIssues);
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
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "New base class MySchema:MyBase is incompatible with properties on MySchema:MyConflict or its derived classes." };
    issues.CompareIssues(expectedIssues);

    //merge again, with resolve conflict flag set to true
    SchemaMergeResult result2;
    SchemaMergeOptions options;
    options.SetRenamePropertyOnConflict(true);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result2, leftSchemas, rightSchemas, options));
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
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Failed to copy class MySchema:MyConflict into merged schema" }; //TODO: This needs a better check!
    issues.CompareIssues(expectedIssues);
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
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Enumeration 'MySchema:MyEnum' has its Type changed. This is not supported." };
    issues.CompareIssues(expectedIssues);
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
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Failed to add property A to class MySchema:MyBase because it conflicts with another property. RenamePropertyOnConflict flag is set to false." };
    issues.CompareIssues(expectedIssues);
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
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Failed to add property A to class MySchema:MyConflict because it conflicts with another property. RenamePropertyOnConflict flag is set to false." };
    issues.CompareIssues(expectedIssues);

    //merge again, with resolve conflict flag set to true
    SchemaMergeResult result2;
    SchemaMergeOptions options;
    options.SetRenamePropertyOnConflict(true);
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result2, leftSchemas, rightSchemas, options));

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
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Failed to add property A to class MySchema:MyBaseBase because it conflicts with another property. RenamePropertyOnConflict flag is set to false." };
    issues.CompareIssues(expectedIssues);

    //merge again, with resolve conflict flag set to true
    SchemaMergeResult result2;
    SchemaMergeOptions options;
    options.SetRenamePropertyOnConflict(true);
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result2, leftSchemas, rightSchemas, options));

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
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Failed to add property A to class MySchema:MyConflict because it conflicts with another property. RenamePropertyOnConflict flag is set to false." };
    issues.CompareIssues(expectedIssues);


    //merge again, with resolve conflict flag set to true
    SchemaMergeResult result2;
    SchemaMergeOptions options;
    options.SetRenamePropertyOnConflict(true);
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result2, leftSchemas, rightSchemas, options));

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
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Failed to add property A to class MyBaseSchema:MyBase because it conflicts with another property. RenamePropertyOnConflict flag is set to false." };
    issues.CompareIssues(expectedIssues);
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
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Failed to add property A to class MySchema:MyConflict because it conflicts with another property. RenamePropertyOnConflict flag is set to false." };
    issues.CompareIssues(expectedIssues);
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
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Property MySchema:MyClass:A has its type changed from binary to Bentley.Geometry.Common.IGeometry." };
    issues.CompareIssues(expectedIssues);

    //merge again, with keep left
    SchemaMergeResult result2;
    SchemaMergeOptions options;
    options.SetIgnoreIncompatiblePropertyTypeChanges(true);
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result2, leftSchemas, rightSchemas, options));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
TEST_F(SchemaMergerTests, MergeSchemasBothHavingIllegalRC)
    {
    //In the test case name RC is short for Relationship Class
    Utf8CP schemaXml1 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
      <ECSchema schemaName="Skimah" nameSpacePrefix="ski" version="01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="Joey" isDomainClass="True">
                <ECProperty propertyName="n" typeName="int"/>
            </ECClass>
            <ECClass typeName="Fastener" isDomainClass="True">
                <ECProperty propertyName="n" typeName="int"/>
            </ECClass>
            <ECClass typeName="Bolt" >
                <ECProperty propertyName="p" typeName="int" displayLabel="p"/>
            </ECClass>
            <ECClass typeName="Weld" >
                <ECProperty propertyName="p" typeName="int" displayLabel="p"/>
            </ECClass>
    
            <ECRelationshipClass typeName="JointHasRandomThings" isDomainClass="True" strength="referencing" strengthDirection="forward">
              <Source cardinality="(0,1)" polymorphic="true" roleLabel = "JointHasRandomThings">
                  <Class class="Joey"/>
              </Source>
              <Target cardinality="(0,N)" polymorphic="true" roleLabel = "JointHasRandomThings (Reversed)">
                  <Class class="Fastener"/>
                  <Class class="Bolt"/>
                  <Class class="Weld"/>
              </Target>
            </ECRelationshipClass>
      </ECSchema>)xml";
  
    Utf8CP schemaXml2 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
      <ECSchema schemaName="Skimah" nameSpacePrefix="ski" version="01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
          <ECClass typeName="Joey" isDomainClass="True">
              <ECProperty propertyName="n" typeName="int"/>
          </ECClass>
          <ECClass typeName="Chandler" isDomainClass="True">
              <ECProperty propertyName="n" typeName="int"/>
          </ECClass>
          <ECClass typeName="Nicks" >
              <ECProperty propertyName="p" typeName="int" displayLabel="p"/>
          </ECClass>
          <ECClass typeName="RuleAll" >
              <ECProperty propertyName="p" typeName="int" displayLabel="p"/>
          </ECClass>
    
          <ECRelationshipClass typeName="ILikeFriendsObviously" isDomainClass="True" strength="referencing" strengthDirection="forward">
            <Source cardinality="(0,1)" polymorphic="true" roleLabel = "ILikeFriendsObviously">
                <Class class="Joey"/>
            </Source>
            <Target cardinality="(0,N)" polymorphic="true" roleLabel = "ILikeFriendsObviously (Reversed)">
                <Class class="Chandler"/>
                <Class class="Nicks"/>
                <Class class="RuleAll"/>
            </Target>
          </ECRelationshipClass>
      </ECSchema>)xml";

    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas({schemaXml1}, nullptr, true);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas({schemaXml2}, nullptr, true);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();

    auto schemaMergeOptions = SchemaMergeOptions();
    SchemaMergeResult result1;
    EXPECT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, SchemaMerger::MergeSchemas(result1, leftSchemas, rightSchemas, schemaMergeOptions));
    schemaMergeOptions.SetSkipValidation(true);
    SchemaMergeResult result2;
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result2, leftSchemas, rightSchemas, schemaMergeOptions));

    bvector<Utf8CP> expectedSchemasXml = {
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Skimah" nameSpacePrefix="ski" version="01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="Weld" isStruct="false" isCustomAttributeClass="false" isDomainClass="true">
                <ECProperty propertyName="p" typeName="int" displayLabel="p"/>
            </ECClass>
            <ECClass typeName="Bolt" isStruct="false" isCustomAttributeClass="false" isDomainClass="true">
                <ECProperty propertyName="p" typeName="int" displayLabel="p"/>
            </ECClass>
            <ECClass typeName="Chandler" isStruct="false" isCustomAttributeClass="false" isDomainClass="true">
                <ECProperty propertyName="n" typeName="int"/>
            </ECClass>
            <ECClass typeName="Fastener" isStruct="false" isCustomAttributeClass="false" isDomainClass="true">
                <ECProperty propertyName="n" typeName="int"/>
            </ECClass>
            <ECClass typeName="Joey" isStruct="false" isCustomAttributeClass="false" isDomainClass="true">
                <ECProperty propertyName="n" typeName="int"/>
            </ECClass>
            <ECClass typeName="Nicks" isStruct="false" isCustomAttributeClass="false" isDomainClass="true">
                <ECProperty propertyName="p" typeName="int" displayLabel="p"/>
            </ECClass>
            <ECClass typeName="RuleAll" isStruct="false" isCustomAttributeClass="false" isDomainClass="true">
                <ECProperty propertyName="p" typeName="int" displayLabel="p"/>
            </ECClass>
            <ECRelationshipClass typeName="ILikeFriendsObviously" isStruct="false" isCustomAttributeClass="false" isDomainClass="true" strength="referencing">
                <Source cardinality="(0,1)" polymorphic="true" roleLabel = "ILikeFriendsObviously">
                    <Class class="Joey"/>
                </Source>
                <Target cardinality="(0,N)" polymorphic="true" roleLabel = "ILikeFriendsObviously (Reversed)">
                    <Class class="Chandler"/>
                    <Class class="Nicks"/>
                    <Class class="RuleAll"/>
                </Target>
            </ECRelationshipClass>
            
            <ECRelationshipClass typeName="JointHasRandomThings" isStruct="false" isCustomAttributeClass="false" isDomainClass="true" strength="referencing">
                <Source cardinality="(0,1)" polymorphic="true" roleLabel = "JointHasRandomThings">
                    <Class class="Joey"/>
                </Source>
                <Target cardinality="(0,N)" polymorphic="true" roleLabel = "JointHasRandomThings (Reversed)">
                    <Class class="Fastener"/>
                    <Class class="Bolt"/>
                    <Class class="Weld"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml"
    };
    
    //Compare actual and expected xml schemas
    CompareResults(expectedSchemasXml, result2, true, ECVersion::V2_0, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, MergeSchemasOneHavingIllegalRC)
    {
    //In the test case name RC is short for Relationship Class
    Utf8CP schemaXml1 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="Skimah" nameSpacePrefix="ski" version="01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
          <ECClass typeName="Joint" isDomainClass="True">
              <ECProperty propertyName="n" typeName="int"/>
          </ECClass>
          <ECClass typeName="Fastener" isDomainClass="True">
              <ECProperty propertyName="n" typeName="int"/>
          </ECClass>
          <ECClass typeName="Bolt" >
              <ECProperty propertyName="p" typeName="int" displayLabel="p"/>
          </ECClass>
          <ECClass typeName="Weld" >
              <ECProperty propertyName="p" typeName="int" displayLabel="p"/>
          </ECClass>
  
          <ECRelationshipClass typeName="JointHasRandomThings" isDomainClass="True" strength="referencing" strengthDirection="forward">
            <Source cardinality="(0,1)" polymorphic="True">
                <Class class="Joint" />
            </Source>
            <Target cardinality="(0,N)" polymorphic="True">
                <Class class="Fastener"/>
                <Class class="Bolt"/>
                <Class class="Weld"/>
            </Target>
        </ECRelationshipClass>
        <ECClass typeName="Zulu" isDomainClass="True">
            <BaseClass>Joint</BaseClass>
        </ECClass>
    </ECSchema>)xml";

    Utf8CP schemaXml2 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="Skimah" nameSpacePrefix="ski" version="01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
          <ECClass typeName="Voodoo" isDomainClass="True">
              <ECProperty propertyName="n" typeName="int"/>
          </ECClass>
    </ECSchema>)xml";

    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas({schemaXml1}, nullptr, true);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas({schemaXml2}, nullptr, true);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();

    auto schemaMergeOptions = SchemaMergeOptions();
    SchemaMergeResult result1;
    EXPECT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, SchemaMerger::MergeSchemas(result1, leftSchemas, rightSchemas, schemaMergeOptions));
    schemaMergeOptions.SetSkipValidation(true);
    SchemaMergeResult result2;
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result2, leftSchemas, rightSchemas, schemaMergeOptions));

    bvector<Utf8CP> expectedSchemasXml = {
      R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Skimah" nameSpacePrefix="ski" version="01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
        <ECClass typeName="Bolt" isStruct="false" isCustomAttributeClass="false" isDomainClass="true">
            <ECProperty propertyName="p" typeName="int" displayLabel="p"/>
        </ECClass>
        <ECClass typeName="Fastener" isStruct="false" isCustomAttributeClass="false" isDomainClass="true">
            <ECProperty propertyName="n" typeName="int"/>
        </ECClass>
        <ECClass typeName="Joint" isStruct="false" isCustomAttributeClass="false" isDomainClass="true">
            <ECProperty propertyName="n" typeName="int"/>
        </ECClass>
        <ECClass typeName="Weld" isStruct="false" isCustomAttributeClass="false" isDomainClass="true">
            <ECProperty propertyName="p" typeName="int" displayLabel="p"/>
        </ECClass>
        <ECRelationshipClass typeName="JointHasRandomThings" isStruct="false" isCustomAttributeClass="false" isDomainClass="true" strength="referencing">
            <Source cardinality="(0,1)" polymorphic="true">
                <Class class="Joint"/>
            </Source>
            <Target cardinality="(0,N)" polymorphic="true">
                <Class class="Fastener"/>
                <Class class="Bolt"/>
                <Class class="Weld"/>
            </Target>
        </ECRelationshipClass>
        <ECClass typeName="Voodoo" isStruct="false" isCustomAttributeClass="false" isDomainClass="true">
            <ECProperty propertyName="n" typeName="int"/>
        </ECClass>
        <ECClass typeName="Zulu" isStruct="false" isCustomAttributeClass="false" isDomainClass="true">
            <BaseClass>Joint</BaseClass>
        </ECClass>
    </ECSchema>)xml"
    };

    //Compare actual and expected xml schemas
    CompareResults(expectedSchemasXml, result2, true, ECVersion::V2_0, true);
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, MergeIdenticalSchemasHavingIllegalRC)
    {
      //In the test case name RC is short for Relationship Class
      Utf8CP schemaXml1 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
      <ECSchema schemaName="Skimah" nameSpacePrefix="ski" version="01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="Joint" isDomainClass="True">
                <ECProperty propertyName="n" typeName="int"/>
            </ECClass>
            <ECClass typeName="Fastener" isDomainClass="True">
                <ECProperty propertyName="n" typeName="int"/>
            </ECClass>
            <ECClass typeName="Bolt" >
                <ECProperty propertyName="p" typeName="int" displayLabel="p"/>
            </ECClass>
            <ECClass typeName="Weld" >
                <ECProperty propertyName="p" typeName="int" displayLabel="p"/>
            </ECClass>
    
            <ECRelationshipClass typeName="JointHasRandomThings" isDomainClass="True" strength="referencing" strengthDirection="forward">
              <Source cardinality="(0,1)" polymorphic="True">
                  <Class class="Joint" />
              </Source>
              <Target cardinality="(0,N)" polymorphic="True">
                  <Class class="Fastener"/>
                  <Class class="Bolt"/>
                  <Class class="Weld"/>
              </Target>
          </ECRelationshipClass>
          <ECClass typeName="Zulu" isDomainClass="True">
              <BaseClass>Joint</BaseClass>
          </ECClass>
      </ECSchema>)xml";
  
      Utf8CP schemaXml2 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
      <ECSchema schemaName="Skimah" nameSpacePrefix="ski" version="01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="Joint" isDomainClass="True">
                <ECProperty propertyName="n" typeName="int"/>
            </ECClass>
            <ECClass typeName="Fastener" isDomainClass="True">
                <ECProperty propertyName="n" typeName="int"/>
            </ECClass>
            <ECClass typeName="Bolt" >
                <ECProperty propertyName="p" typeName="int" displayLabel="p"/>
            </ECClass>
            <ECClass typeName="Weld" >
                <ECProperty propertyName="p" typeName="int" displayLabel="p"/>
            </ECClass>
    
            <ECRelationshipClass typeName="JointHasRandomThings" isDomainClass="True" strength="referencing" strengthDirection="forward">
              <Source cardinality="(0,1)" polymorphic="True">
                  <Class class="Joint" />
              </Source>
              <Target cardinality="(0,N)" polymorphic="True">
                  <Class class="Fastener"/>
                  <Class class="Bolt"/>
                  <Class class="Weld"/>
              </Target>
          </ECRelationshipClass>
          <ECClass typeName="Zulu" isDomainClass="True">
              <BaseClass>Joint</BaseClass>
          </ECClass>
      </ECSchema>)xml";

    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas({schemaXml1}, nullptr, true);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas({schemaXml2}, nullptr, true);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();

    auto schemaMergeOptions = SchemaMergeOptions();
    SchemaMergeResult result1;
    EXPECT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, SchemaMerger::MergeSchemas(result1, leftSchemas, rightSchemas, schemaMergeOptions));
    schemaMergeOptions.SetSkipValidation(true);
    SchemaMergeResult result2;
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result2, leftSchemas, rightSchemas, schemaMergeOptions));

    bvector<Utf8CP> expectedSchemasXml = {
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
          <ECSchema schemaName="Skimah" nameSpacePrefix="ski" version="01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
              <ECClass typeName="Joint" isDomainClass="True">
                  <ECProperty propertyName="n" typeName="int"/>
              </ECClass>
              <ECClass typeName="Fastener" isDomainClass="True">
                  <ECProperty propertyName="n" typeName="int"/>
              </ECClass>
              <ECClass typeName="Bolt" >
                  <ECProperty propertyName="p" typeName="int" displayLabel="p"/>
              </ECClass>
              <ECClass typeName="Weld" >
                  <ECProperty propertyName="p" typeName="int" displayLabel="p"/>
              </ECClass>
      
              <ECRelationshipClass typeName="JointHasRandomThings" isDomainClass="True" strength="referencing" strengthDirection="forward">
                <Source cardinality="(0,1)" polymorphic="True">
                    <Class class="Joint" />
                </Source>
                <Target cardinality="(0,N)" polymorphic="True">
                    <Class class="Fastener"/>
                    <Class class="Bolt"/>
                    <Class class="Weld"/>
                </Target>
              </ECRelationshipClass>
              <ECClass typeName="Zulu" isDomainClass="True">
                  <BaseClass>Joint</BaseClass>
              </ECClass>
          </ECSchema>)xml"
    };

    //Compare actual and expected xml schemas
    CompareResults(expectedSchemasXml, result2, true, ECVersion::V2_0, true);
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, MergeSchemasWhereResultWillHaveIllegalRC)
    {
      //In the test case name RC is short for Relationship Class
      Utf8CP schemaXml1 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
      <ECSchema schemaName="Skimah" nameSpacePrefix="ski" version="01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="Joint" isDomainClass="True">
                <ECProperty propertyName="n" typeName="int"/>
            </ECClass>
            <ECClass typeName="Weld" >
                <ECProperty propertyName="p" typeName="int"/>
            </ECClass>
    
            <ECRelationshipClass typeName="JointHasRandomThings" isDomainClass="True" strength="referencing" strengthDirection="forward">
              <Source cardinality="(0,1)" polymorphic="True">
                  <Class class="Joint" />
              </Source>
              <Target cardinality="(0,N)" polymorphic="True">
                  <Class class="Weld"/>
              </Target>
          </ECRelationshipClass>
      </ECSchema>)xml";
  
      Utf8CP schemaXml2 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
      <ECSchema schemaName="Skimah" nameSpacePrefix="ski" version="01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="Joint" isDomainClass="True">
                <ECProperty propertyName="n" typeName="int"/>
            </ECClass>
            <ECClass typeName="Fastener">
                <ECProperty propertyName="n" typeName="int"/>
            </ECClass>
    
            <ECRelationshipClass typeName="JointHasRandomThings" isDomainClass="True" strength="referencing" strengthDirection="forward">
              <Source cardinality="(0,1)" polymorphic="True">
                  <Class class="Joint" />
              </Source>
              <Target cardinality="(0,N)" polymorphic="True">
                  <Class class="Fastener"/>
              </Target>
          </ECRelationshipClass>
      </ECSchema>)xml";

    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas({schemaXml1}, nullptr, true);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas({schemaXml2}, nullptr, true);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();

    auto schemaMergeOptions = SchemaMergeOptions();
    SchemaMergeResult result1;
    EXPECT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, SchemaMerger::MergeSchemas(result1, leftSchemas, rightSchemas, schemaMergeOptions));
    schemaMergeOptions.SetSkipValidation(true);
    SchemaMergeResult result2;
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result2, leftSchemas, rightSchemas, schemaMergeOptions));

    bvector<Utf8CP> expectedSchemasXml = {
      R"xml(<?xml version="1.0" encoding="UTF-8"?>
      <ECSchema schemaName="Skimah" nameSpacePrefix="ski" version="01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="Joint" isDomainClass="True">
                <ECProperty propertyName="n" typeName="int"/>
            </ECClass>
            <ECClass typeName="Weld" >
                <ECProperty propertyName="p" typeName="int"/>
            </ECClass>
            <ECClass typeName="Fastener">
                <ECProperty propertyName="n" typeName="int"/>
            </ECClass>
    
            <ECRelationshipClass typeName="JointHasRandomThings" isDomainClass="True" strength="referencing" strengthDirection="forward">
              <Source cardinality="(0,1)" polymorphic="True">
                  <Class class="Joint" />
              </Source>
              <Target cardinality="(0,N)" polymorphic="True">
                  <Class class="Weld"/>
                  <Class class="Fastener"/>
              </Target>
          </ECRelationshipClass>
      </ECSchema>)xml"
    };

    //Compare actual and expected xml schemas
    CompareResults(expectedSchemasXml, result2, true, ECVersion::V2_0, true);
    
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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Setting AbstractConstraint on MySchema:MyRelationshipClass failed. Was trying to set to MySchema:ABase2." };
    issues.CompareIssues(expectedIssues);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, UnableToFindAbstractConstraint)
    {
    // This test tries to provoke EC_0025 where a schema cannot be copied. In this case it's an old 2.0 schema with no abstract constraint, and the setup
    // of constraint classes does not allow automatically setting the abstract constraint
    // The test uses 2.0 schemas on purpose as they allow this sort of thing to happen

    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Test1" nameSpacePrefix="ts" version="01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECClass typeName="Foo" isDomainClass="True">
                </ECClass>
                <ECClass typeName="Bar" isDomainClass="True">
                    <BaseClass>Foo</BaseClass>
                </ECClass>
            </ECSchema>)xml" };

    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Test1" nameSpacePrefix="ts" version="01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECClass typeName="Foo" isDomainClass="True">
                </ECClass>
                <ECClass typeName="Bar" isDomainClass="True">
                    <BaseClass>Foo</BaseClass>
                </ECClass>
            </ECSchema>)xml",
      R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Test2" nameSpacePrefix="ts" version="01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECClass typeName="Foo" isDomainClass="True">
                </ECClass>
                <ECClass typeName="Bar" isDomainClass="True">
                </ECClass>
                <ECRelationshipClass typeName="RelationshipWithNoAbstractConstraint" isDomainClass="True" strength="referencing" strengthDirection="forward">
                    <Source cardinality="(0,N)" polymorphic="false">
                        <Class class="Foo" />
                        <Class class="Bar" />
                    </Source>
                    <Target cardinality="(0,1)" polymorphic="false">
                        <Class class="Foo" />
                    </Target>
                </ECRelationshipClass>
            </ECSchema>)xml" };

    TestLogger logger;
    LogCatcher catcher(logger);
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();

    logger.ValidateMessageAtIndex(2, SEVERITY::LOG_INFO,
      "Abstract Constraint Violation (ResolveIssues: Yes): The Source-Constraint of 'Test2:RelationshipWithNoAbstractConstraint' does not contain or inherit an abstractConstraint attribute. It is a required attribute if there is more than one constraint class.");
    logger.ValidateMessageAtIndex(3, SEVERITY::LOG_ERROR,
      "Failed to find a common base class between the constraint classes of Source-Constraint on class 'Test2:RelationshipWithNoAbstractConstraint'");
    logger.ValidateMessageAtIndex(4, SEVERITY::LOG_ERROR,
      "Relationship Class Constraint Violation: Abstract Class Constraint validation failed for the 'Source' constraint of relationship 'Test2:RelationshipWithNoAbstractConstraint'");
    logger.ValidateMessageAtIndex(5, SEVERITY::LOG_WARNING,
      "ECSchemaXML for Test2 did not pass ECXml 3.1 validation, being downgraded to ECXml 3.0");
    
    SchemaMergeResult result;
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<ReportedIssue> expectedIssues { ReportedIssue(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSchema, ECIssueId::EC_0025, "Schema 'Test2.01.00.01' from right side failed to be copied.") };
    issues.CompareIssues(expectedIssues);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, RelationshipConstraintNotCompatibleProblem)
    {
    // This test tries to provoke EC_0024 when ECRelationshipConstraint::AddClass is called with no abstract constraint set
    // Apparently we fail to record a meaningful error in this case

    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Test2" nameSpacePrefix="ts" version="01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECClass typeName="Orange" isDomainClass="True" />
                <ECClass typeName="Apple" isDomainClass="True" />
                <ECClass typeName="Lemon" isDomainClass="True" />
                <ECRelationshipClass typeName="MyRelationshipClass" isDomainClass="True" strength="referencing" strengthDirection="forward">
                    <Source cardinality="(0,N)" polymorphic="false">
                        <Class class="Apple" />
                    </Source>
                    <Target cardinality="(0,1)" polymorphic="false">
                        <Class class="Apple" />
                        <ECClass typeName="Orange" isDomainClass="True" />
                    </Target>
                </ECRelationshipClass>
            </ECSchema>)xml"
    };

    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Test2" nameSpacePrefix="ts" version="01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECClass typeName="Orange" isDomainClass="True" />
                <ECClass typeName="Apple" isDomainClass="True" />
                <ECClass typeName="Lemon" isDomainClass="True" />
                <ECRelationshipClass typeName="MyRelationshipClass" isDomainClass="True" strength="referencing" strengthDirection="forward">
                    <Source cardinality="(0,N)" polymorphic="false">
                        <Class class="Apple" />
                    </Source>
                    <Target cardinality="(0,1)" polymorphic="false">
                        <Class class="Apple" />
                        <Class class="Orange" />
                        <Class class="Lemon" />
                    </Target>
                </ECRelationshipClass>
            </ECSchema>)xml" };

    TestLogger logger;
    LogCatcher catcher(logger);
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();

    logger.ValidateMessageAtIndex(2, SEVERITY::LOG_INFO,
      "Abstract Constraint Violation (ResolveIssues: Yes): The Target-Constraint of 'Test2:MyRelationshipClass' does not contain or inherit an abstractConstraint attribute. It is a required attribute if there is more than one constraint class.");
    logger.ValidateMessageAtIndex(3, SEVERITY::LOG_ERROR,
      "Failed to find a common base class between the constraint classes of Target-Constraint on class 'Test2:MyRelationshipClass'");
    logger.ValidateMessageAtIndex(4, SEVERITY::LOG_ERROR,
      "Relationship Class Constraint Violation: Abstract Class Constraint validation failed for the 'Source' constraint of relationship 'Test2:MyRelationshipClass'");
    logger.ValidateMessageAtIndex(5, SEVERITY::LOG_WARNING,
      "ECSchemaXML for Test2 did not pass ECXml 3.1 validation, being downgraded to ECXml 3.0");
    
    SchemaMergeResult result;
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<ReportedIssue> expectedIssues { ReportedIssue(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSchema, ECIssueId::EC_0024, "Setting ConstraintClass on Test2:MyRelationshipClass failed. Was trying to set to Test2:Lemon.") };
    issues.CompareIssues(expectedIssues);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, RelationshipConstraintOnLeftSchema)
    {
    // This test attempts to reproduce a case where left and right have an identical relationship but CopyClass failed with few details

    bvector<Utf8CP> leftSchemasXml {
      R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Test2" nameSpacePrefix="ts" version="01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECClass typeName="JOINT" isDomainClass="True" />
                <ECClass typeName="WELD" isDomainClass="True">
                    <BaseClass>FASTENER</BaseClass>
                </ECClass>
                <ECClass typeName="FASTENER" isDomainClass="True" />
                <ECClass typeName="BOLT" isDomainClass="True">
                    <BaseClass>FASTENER</BaseClass>
                </ECClass>
                <ECRelationshipClass typeName="JOINT_HAS_FASTENER" description="" displayLabel="Joint Has Fastener" isStruct="false" isDomainClass="true" isCustomAttributeClass="false" strength="referencing"
                  strengthDirection="forward">
                  <Source cardinality="(0,N)" roleLabel="Joint has Fastener" polymorphic="true">
                      <Class class="JOINT" />
                  </Source>
                  <Target cardinality="(0,N)" roleLabel="Joint has Fastener (reversed)" polymorphic="true">
                      <Class class="WELD" />
                      <Class class="FASTENER" />
                      <Class class="BOLT" />
                  </Target>
              </ECRelationshipClass>
            </ECSchema>)xml"
    };

    bvector<ECN::ECSchemaCP> leftSchemas;
    // We have to skip validation for this test, as validation would either fix the problem or log errors.
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml, &leftSchemas, true);

    bvector<Utf8CP> rightSchemasXml{
      R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Test2" nameSpacePrefix="ts" version="01.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECClass typeName="JOINT" isDomainClass="True" />
                <ECClass typeName="WELD" isDomainClass="True">
                    <BaseClass>FASTENER</BaseClass>
                </ECClass>
                <ECClass typeName="FASTENER" isDomainClass="True" />
                <ECClass typeName="BOLT" isDomainClass="True">
                    <BaseClass>FASTENER</BaseClass>
                </ECClass>
                <ECClass typeName="PIPE" isDomainClass="True" />
                <ECRelationshipClass typeName="JOINT_HAS_FASTENER" description="" displayLabel="Joint Has Fastener" isStruct="false" isDomainClass="true" isCustomAttributeClass="false" strength="referencing"
                  strengthDirection="forward">
                  <Source cardinality="(0,N)" roleLabel="Joint has Fastener" polymorphic="true">
                      <Class class="JOINT" />
                  </Source>
                  <Target cardinality="(0,N)" roleLabel="Joint has Fastener (reversed)" polymorphic="true">
                      <Class class="WELD" />
                      <Class class="FASTENER" />
                      <Class class="BOLT" />
                  </Target>
              </ECRelationshipClass>
            </ECSchema>)xml"
    };

    TestLogger logger;
    LogCatcher catcher(logger);
    bvector<ECN::ECSchemaCP> rightSchemas;
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml, &rightSchemas, true);

    logger.ValidateMessageAtIndex(1, SEVERITY::LOG_DEBUG,
      "Skipping validation for 'Test2.01.00.02' because the read context has skip validation property set to true");
    
    SchemaMergeResult result;
    TestIssueListener issues;
    result.AddIssueListener(issues);
    logger.Clear();
    EXPECT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    logger.ValidateMessageAtIndex(0, SEVERITY::LOG_ERROR,
      "Cannot add class Test2:FASTENER to target-constraint on Test2:JOINT_HAS_FASTENER. There is no abstract constraint defined, so adding this class would render the schema invalid.");
    logger.ValidateMessageAtIndex(1, SEVERITY::LOG_ERROR,
      "Failed to copy class Test2:JOINT_HAS_FASTENER to schema Test2.01.00.01");
    logger.ValidateMessageAtIndex(2, SEVERITY::LOG_ERROR,
      "Schema 'Test2.01.00.01' from left side failed to be copied.");

    // Compare issues
    bvector<ReportedIssue> expectedIssues { ReportedIssue(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSchema,
      ECIssueId::EC_0025, "Schema 'Test2.01.00.01' from left side failed to be copied.") };
    issues.CompareIssues(expectedIssues);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, ChangeStrength)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A1">
          </ECEntityClass>
          <ECEntityClass typeName="A2">
          </ECEntityClass>
          <ECRelationshipClass typeName="MyRelationshipClass" modifier="None" strength="referencing">
              <Source multiplicity="(0..*)" roleLabel="uses section" polymorphic="true">
                  <Class class="A1"/>
              </Source>
              <Target multiplicity="(0..1)" roleLabel="is part of built up section" polymorphic="true">
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
          <ECEntityClass typeName="A1">
          </ECEntityClass>
          <ECEntityClass typeName="A2">
          </ECEntityClass>
          <ECRelationshipClass typeName="MyRelationshipClass" modifier="None" strength="holding" >
              <Source multiplicity="(0..*)" roleLabel="uses section" polymorphic="true">
                  <Class class="A1"/>
              </Source>
              <Target multiplicity="(0..1)" roleLabel="is part of built up section" polymorphic="true">
                  <Class class="A2"/>
              </Target>
          </ECRelationshipClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    SchemaMergeResult result;
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A1">
          </ECEntityClass>
          <ECEntityClass typeName="A2">
          </ECEntityClass>
          <ECRelationshipClass typeName="MyRelationshipClass" modifier="None" strength="holding" >
              <Source multiplicity="(0..*)" roleLabel="uses section" polymorphic="true">
                  <Class class="A1"/>
              </Source>
              <Target multiplicity="(0..1)" roleLabel="is part of built up section" polymorphic="true">
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
TEST_F(SchemaMergerTests, AddBaseRelationship)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Element" />

          <ECRelationshipClass typeName="ElementGroupsMembers" strength="referencing" modifier="None">
              <Source multiplicity="(0..*)" roleLabel="groups" polymorphic="true">
                  <Class class="Element"/>
              </Source>
              <Target multiplicity="(0..*)" roleLabel="is grouped by" polymorphic="true">
                  <Class class="Element"/>
              </Target>
              <ECProperty propertyName="MemberPriority" typeName="int" displayLabel="Member Priority" />
          </ECRelationshipClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Element" />
          <ECRelationshipClass typeName="ElementRefersToElements" strength="referencing" modifier="Abstract">
              <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                  <Class class="Element"/>
              </Source>
              <Target multiplicity="(0..*)" roleLabel="is referenced by" polymorphic="true">
                  <Class class="Element"/>
              </Target>
          </ECRelationshipClass>

          <ECRelationshipClass typeName="ElementGroupsMembers" strength="referencing" modifier="None">
              <BaseClass>ElementRefersToElements</BaseClass>
              <Source multiplicity="(0..*)" roleLabel="groups" polymorphic="true">
                  <Class class="Element"/>
              </Source>
              <Target multiplicity="(0..*)" roleLabel="is grouped by" polymorphic="true">
                  <Class class="Element"/>
              </Target>
              <ECProperty propertyName="MemberPriority" typeName="int" displayLabel="Member Priority" />
          </ECRelationshipClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    SchemaMergeResult result;
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Element" />
          <ECRelationshipClass typeName="ElementRefersToElements" strength="referencing" modifier="Abstract">
              <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                  <Class class="Element"/>
              </Source>
              <Target multiplicity="(0..*)" roleLabel="is referenced by" polymorphic="true">
                  <Class class="Element"/>
              </Target>
          </ECRelationshipClass>

          <ECRelationshipClass typeName="ElementGroupsMembers" strength="referencing" modifier="None">
              <BaseClass>ElementRefersToElements</BaseClass>
              <Source multiplicity="(0..*)" roleLabel="groups" polymorphic="true">
                  <Class class="Element"/>
              </Source>
              <Target multiplicity="(0..*)" roleLabel="is grouped by" polymorphic="true">
                  <Class class="Element"/>
              </Target>
              <ECProperty propertyName="MemberPriority" typeName="int" displayLabel="Member Priority" />
          </ECRelationshipClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, AddBaseRelationshipAndChangeStrength)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Element" />

          <ECRelationshipClass typeName="ElementGroupsMembers" strength="holding" modifier="None">
              <Source multiplicity="(0..*)" roleLabel="groups" polymorphic="true">
                  <Class class="Element"/>
              </Source>
              <Target multiplicity="(0..*)" roleLabel="is grouped by" polymorphic="true">
                  <Class class="Element"/>
              </Target>
              <ECProperty propertyName="MemberPriority" typeName="int" displayLabel="Member Priority" />
          </ECRelationshipClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Element" />
          <ECRelationshipClass typeName="ElementRefersToElements" strength="referencing" modifier="Abstract">
              <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                  <Class class="Element"/>
              </Source>
              <Target multiplicity="(0..*)" roleLabel="is referenced by" polymorphic="true">
                  <Class class="Element"/>
              </Target>
          </ECRelationshipClass>

          <ECRelationshipClass typeName="ElementGroupsMembers" strength="referencing" modifier="None">
              <BaseClass>ElementRefersToElements</BaseClass>
              <Source multiplicity="(0..*)" roleLabel="groups" polymorphic="true">
                  <Class class="Element"/>
              </Source>
              <Target multiplicity="(0..*)" roleLabel="is grouped by" polymorphic="true">
                  <Class class="Element"/>
              </Target>
              <ECProperty propertyName="MemberPriority" typeName="int" displayLabel="Member Priority" />
          </ECRelationshipClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    SchemaMergeResult result;
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Element" />
          <ECRelationshipClass typeName="ElementRefersToElements" strength="referencing" modifier="Abstract">
              <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                  <Class class="Element"/>
              </Source>
              <Target multiplicity="(0..*)" roleLabel="is referenced by" polymorphic="true">
                  <Class class="Element"/>
              </Target>
          </ECRelationshipClass>

          <ECRelationshipClass typeName="ElementGroupsMembers" strength="referencing" modifier="None">
              <BaseClass>ElementRefersToElements</BaseClass>
              <Source multiplicity="(0..*)" roleLabel="groups" polymorphic="true">
                  <Class class="Element"/>
              </Source>
              <Target multiplicity="(0..*)" roleLabel="is grouped by" polymorphic="true">
                  <Class class="Element"/>
              </Target>
              <ECProperty propertyName="MemberPriority" typeName="int" displayLabel="Member Priority" />
          </ECRelationshipClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }


/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, ChangeStrengthIllegal)
    {
    // Setting the strength of a derived relationship to something other than its base class is illegal. This test attempts to do just that.
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Element" />

          <ECRelationshipClass typeName="ElementRefersToElements" strength="referencing" modifier="Abstract">
              <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                  <Class class="Element"/>
              </Source>
              <Target multiplicity="(0..*)" roleLabel="is referenced by" polymorphic="true">
                  <Class class="Element"/>
              </Target>
          </ECRelationshipClass>

          <ECRelationshipClass typeName="ElementGroupsMembers" strength="referencing" modifier="None">
              <BaseClass>ElementRefersToElements</BaseClass>
              <Source multiplicity="(0..*)" roleLabel="groups" polymorphic="true">
                  <Class class="Element"/>
              </Source>
              <Target multiplicity="(0..*)" roleLabel="is grouped by" polymorphic="true">
                  <Class class="Element"/>
              </Target>
              <ECProperty propertyName="MemberPriority" typeName="int" displayLabel="Member Priority" />
          </ECRelationshipClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Element" />

          <ECRelationshipClass typeName="ElementGroupsMembers" strength="holding" modifier="None">
              <Source multiplicity="(0..*)" roleLabel="groups" polymorphic="true">
                  <Class class="Element"/>
              </Source>
              <Target multiplicity="(0..*)" roleLabel="is grouped by" polymorphic="true">
                  <Class class="Element"/>
              </Target>
              <ECProperty propertyName="MemberPriority" typeName="int" displayLabel="Member Priority" />
          </ECRelationshipClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();

    {
    SchemaMergeResult result;
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "The setter for StrengthType on item MySchema:ElementGroupsMembers returned an error." };
    issues.CompareIssues(expectedIssues);
    }
    
    {
    SchemaMergeResult result;
    SchemaMergeOptions options;
    options.SetIgnoreStrengthChangeProblems(true);
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas, options));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Element" />
          <ECRelationshipClass typeName="ElementRefersToElements" strength="referencing" modifier="Abstract">
              <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                  <Class class="Element"/>
              </Source>
              <Target multiplicity="(0..*)" roleLabel="is referenced by" polymorphic="true">
                  <Class class="Element"/>
              </Target>
          </ECRelationshipClass>

          <ECRelationshipClass typeName="ElementGroupsMembers" strength="referencing" modifier="None">
              <BaseClass>ElementRefersToElements</BaseClass>
              <Source multiplicity="(0..*)" roleLabel="groups" polymorphic="true">
                  <Class class="Element"/>
              </Source>
              <Target multiplicity="(0..*)" roleLabel="is grouped by" polymorphic="true">
                  <Class class="Element"/>
              </Target>
              <ECProperty propertyName="MemberPriority" typeName="int" displayLabel="Member Priority" />
          </ECRelationshipClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }
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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, schemas32, schemas31));

    auto mergedSchema = result.GetSchema("SchemaMergeA");
    EXPECT_EQ(3, mergedSchema->GetOriginalECXmlVersionMajor());
    EXPECT_EQ(2, mergedSchema->GetOriginalECXmlVersionMinor());
    }

    {
    SchemaMergeResult result;
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, schemas31, schemas32));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, schemas31, schemas31_));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

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

TEST_F(SchemaMergerTests, TestPreferRightSideDisplayLabelSchemaMergerFlag)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemaXml {
      R"schema(<?xml version='1.0' encoding='utf-8'?>
      <ECSchema schemaName='MySchema' alias='mys' version='01.00.00' displayLabel='Initial Schema Display Label' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
        <ECEntityClass typeName='MyEntity' displayLabel='Initial MyEntity Label'>
           <ECProperty propertyName='A' typeName='string' displayLabel='Initial Property 1' />
        </ECEntityClass>
        <ECStructClass typeName='MyStruct' displayLabel='Initial MyStruct Label'/>
        <ECCustomAttributeClass typeName='CustomAttrib' description='CustomAttribute' displayLabel='Initial CA' />
        <ECEnumeration typeName='MyEnum1' backingTypeName='int' isStrict='true'>
          <ECEnumerator name='a' value='0' displayLabel='Initial Enum'/>
        </ECEnumeration>
        <Phenomenon typeName='LENGTH' definition='LENGTH' displayLabel='Initial Phenomenon' />
        <UnitSystem typeName='SI' displayLabel='Initial UnitSystem' />
        <Unit typeName='M' phenomenon='LENGTH' unitSystem='SI' definition='M' denominator='10.0' displayLabel='Old M' />
        <Format typeName='Format1' type='decimal' precision='6' formatTraits='keepSingleZero|keepDecimalPoint|showUnitLabel' displayLabel='Initial Format'/>
        <KindOfQuantity typeName='MYLENGTH' displayLabel='Initial KOQ' persistenceUnit='M' presentationUnits='Format1[M]' relativeError='0.0001' />
      </ECSchema>)schema"};

    bvector<Utf8CP> rightSchemaXml {
      R"schema(<?xml version='1.0' encoding='utf-8'?>
      <ECSchema schemaName='MySchema' alias='mys' version='01.00.01' description='Description' displayLabel='New Display Label' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
        <ECEntityClass typeName='MyEntity' description='Entity Description' displayLabel='New MyEntity Label' modifier='Abstract'>
          <ECProperty propertyName='A' typeName='string' displayLabel='New Property 1' />
        </ECEntityClass>
        <ECStructClass typeName='MyStruct' description='Struct Description' displayLabel='New MyStruct Label' modifier='Abstract'>
          <ECProperty propertyName='B' typeName='string' displayLabel='New Property 2' />
        </ECStructClass>
        <ECCustomAttributeClass typeName='CustomAttrib' description='CustomAttribute' displayLabel='New CA' />
        <ECEnumeration typeName='MyEnum1' backingTypeName='int' isStrict='true'>
          <ECEnumerator name='a' value='0' displayLabel='New Enum'/>
        </ECEnumeration>
        <Phenomenon typeName='LENGTH' definition='LENGTH' displayLabel='New Phenomenon' />
        <UnitSystem typeName='SI' displayLabel='New UnitSystem' />
        <Unit typeName='M' phenomenon='LENGTH' unitSystem='SI' definition='M' denominator='10.0' displayLabel='New M' />
        <Format typeName='Format1' type='decimal' precision='6' formatTraits='keepSingleZero|keepDecimalPoint|showUnitLabel' displayLabel='New Format'/>
        <KindOfQuantity typeName='MYLENGTH' displayLabel='New KOQ' persistenceUnit='M' presentationUnits='Format1[M]' relativeError='0.0001' />
      </ECSchema>)schema"};

    // Schemas to compare against
    bvector<Utf8CP> expectedSchemaXmlLabelNotUpdated {
      R"schema(<?xml version='1.0' encoding='utf-8'?>'
      <ECSchema schemaName='MySchema' alias='mys' version='01.00.01' description='Description' displayLabel='Initial Schema Display Label' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
        <ECEntityClass typeName='MyEntity' description='Entity Description' displayLabel='Initial MyEntity Label' modifier='Abstract'>
          <ECProperty propertyName='A' typeName='string' displayLabel='Initial Property 1' />
        </ECEntityClass>
        <ECStructClass typeName='MyStruct' description='Struct Description' displayLabel='Initial MyStruct Label' modifier='Abstract'>
          <ECProperty propertyName='B' typeName='string' displayLabel='New Property 2' />
        </ECStructClass>
        <ECCustomAttributeClass typeName='CustomAttrib' description='CustomAttribute' displayLabel='Initial CA' />
        <ECEnumeration typeName='MyEnum1' backingTypeName='int' isStrict='true'>
          <ECEnumerator name='a' value='0' displayLabel='Initial Enum'/>
        </ECEnumeration>
        <Phenomenon typeName='LENGTH' definition='LENGTH' displayLabel='Initial Phenomenon' />
        <UnitSystem typeName='SI' displayLabel='Initial UnitSystem' />
        <Unit typeName='M' phenomenon='LENGTH' unitSystem='SI' definition='M' denominator='10.0' displayLabel='Old M' />
        <Format typeName='Format1' type='decimal' precision='6' formatTraits='keepSingleZero|keepDecimalPoint|showUnitLabel' displayLabel='Initial Format'/>
        <KindOfQuantity typeName='MYLENGTH' displayLabel='Initial KOQ' persistenceUnit='M' presentationUnits='Format1[M]' relativeError='0.0001' />
      </ECSchema>)schema"};

    bvector<Utf8CP> expectedSchemaXmlLabelUpdated {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>'
      <ECSchema schemaName='MySchema' alias='mys' version='01.00.01' description='Description' displayLabel='New Display Label' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>
        <ECEntityClass typeName='MyEntity' description='Entity Description' displayLabel='New MyEntity Label' modifier='Abstract'>
          <ECProperty propertyName='A' typeName='string' displayLabel='New Property 1' />
        </ECEntityClass>
        <ECStructClass typeName='MyStruct' description='Struct Description' displayLabel='New MyStruct Label' modifier='Abstract'>
          <ECProperty propertyName='B' typeName='string' displayLabel='New Property 2' />
        </ECStructClass>
        <ECCustomAttributeClass typeName='CustomAttrib' description='CustomAttribute' displayLabel='New CA' />
        <ECEnumeration typeName='MyEnum1' backingTypeName='int' isStrict='true'>
          <ECEnumerator name='a' value='0' displayLabel='New Enum'/>
        </ECEnumeration>
        <Phenomenon typeName='LENGTH' definition='LENGTH' displayLabel='New Phenomenon' />
        <UnitSystem typeName='SI' displayLabel='New UnitSystem' />
        <Unit typeName='M' phenomenon='LENGTH' unitSystem='SI' definition='M' denominator='10.0' displayLabel='New M' />
        <Format typeName='Format1' type='decimal' precision='6' formatTraits='keepSingleZero|keepDecimalPoint|showUnitLabel' displayLabel='New Format'/>
        <KindOfQuantity typeName='MYLENGTH' displayLabel='New KOQ' persistenceUnit='M' presentationUnits='Format1[M]' relativeError='0.0001' />
      </ECSchema>)schema"};

    auto leftContext = InitializeReadContextWithAllSchemas(leftSchemaXml);
    auto leftSchema = leftContext->GetCache().GetSchemas();
    auto rightContext = InitializeReadContextWithAllSchemas(rightSchemaXml);
    auto rightSchema = rightContext->GetCache().GetSchemas();

    // Test Case 1 : OverwriteDisplayLabel not specified (defaults to false)
    // Result : Display label should NOT be overwritten.
    SchemaMergeResult testCase1Result;
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(testCase1Result, leftSchema, rightSchema));
    CompareResults({expectedSchemaXmlLabelNotUpdated}, testCase1Result);

    // Test Case 2 : OverwriteDisplayLabel set to true
    // Result : Display label should be updated.
    SchemaMergeResult testCase2Result;
    SchemaMergeOptions options;
    options.SetPreferRightSideDisplayLabel(true);
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(testCase2Result, leftSchema, rightSchema, options));
    CompareResults({expectedSchemaXmlLabelUpdated}, testCase2Result);

    // Test Case 3 : OverwriteDisplayLabel set to false
    // Result : Display label should NOT be overwritten.
    SchemaMergeResult testCase3Result;
    options.SetPreferRightSideDisplayLabel(false);
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(testCase3Result, leftSchema, rightSchema, options));
    CompareResults({expectedSchemaXmlLabelNotUpdated}, testCase3Result);
    }

TEST_F(SchemaMergerTests, TestBaseClassAdditionAndRemoval)
  {
  bvector<Utf8CP> leftSchemaXml {
    R"schema(<?xml version='1.0' encoding='utf-8' ?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" displayLabel="Schema Merge Test Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
      <ECEntityClass typeName="BaseEntity" />
      <ECEntityClass typeName="EntityCommonToBoth1" />
      <ECEntityClass typeName="EntitycommonToBoth2" />
      <ECEntityClass typeName="Entity_OnlyInLeft" />
      <ECEntityClass typeName="TestClass">
        <BaseClass>BaseEntity</BaseClass>
        <BaseClass>EntityCommonToBoth1</BaseClass>
        <BaseClass>EntitycommonToBoth2</BaseClass>
        <BaseClass>Entity_OnlyInLeft</BaseClass>
      </ECEntityClass>
    </ECSchema>)schema"};

  bvector<Utf8CP> rightSchemaXml {
    R"schema(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" displayLabel="Schema Merge Test Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
      <ECEntityClass typeName="BaseEntity" />
      <ECEntityClass typeName="EntityCommonToBoth1" />
      <ECEntityClass typeName="EntitycommonToBoth2" />
      <ECEntityClass typeName="Entity_OnlyInRight" />
      <ECEntityClass typeName="TestClass">
        <BaseClass>BaseEntity</BaseClass>
        <BaseClass>EntityCommonToBoth1</BaseClass>
        <BaseClass>EntitycommonToBoth2</BaseClass>
        <BaseClass>Entity_OnlyInRight</BaseClass>
      </ECEntityClass>
    </ECSchema>)schema"};

  auto leftContext = InitializeReadContextWithAllSchemas(leftSchemaXml);
  auto leftSchema = leftContext->GetCache().GetSchemas();
  auto rightContext = InitializeReadContextWithAllSchemas(rightSchemaXml);
  auto rightSchema = rightContext->GetCache().GetSchemas();

  //merge the schemas
  SchemaMergeResult result;
  TestIssueListener issues;
  result.AddIssueListener(issues);
  EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchema, rightSchema));

  // Compare result
  bvector<Utf8CP> expectedSchemasXml {
    R"schema(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" displayLabel="Schema Merge Test Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
      <ECEntityClass typeName="BaseEntity" />
      <ECEntityClass typeName="EntityCommonToBoth1" />
      <ECEntityClass typeName="EntitycommonToBoth2" />
      <ECEntityClass typeName="Entity_OnlyInLeft" />
      <ECEntityClass typeName="Entity_OnlyInRight" />
      <ECEntityClass typeName="TestClass">
        <BaseClass>BaseEntity</BaseClass>
        <BaseClass>EntityCommonToBoth1</BaseClass>
        <BaseClass>EntitycommonToBoth2</BaseClass>
        <BaseClass>Entity_OnlyInLeft</BaseClass>
        <BaseClass>Entity_OnlyInRight</BaseClass>
      </ECEntityClass>
    </ECSchema>)schema"};

  CompareResults(expectedSchemasXml, result);
  EXPECT_TRUE(issues.m_issues.empty());
  }

/*---------------------------------------------------------------------------------------
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, DuplicateSchemaNamesMergeResultLeftNameCaps)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MYSCHEMA" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();

    // Merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MYSCHEMA" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A"/>
          <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------------
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, DuplicateSchemaNamesMergeRightNameCaps)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MYSCHEMA" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();

    // Merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A"/>
          <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------------
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, DuplicateSchemaNamesLeftMergeNoReferences)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MyOtherSchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MYOTHERSCHEMA" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
          </ECEntityClass>
        </ECSchema>
        )schema"

    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();

    // Merge the schemas
    SchemaMergeOptions options;
    options.SetDoNotMergeReferences(true);
    SchemaMergeResult result;
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas, options));

    // Compare issues
    bvector<Utf8String> expectedIssues { "The schema name entry MyOtherSchema is non-unique in the left schema list. The schemas names are case-insensitive.",
                                         "The schema name entry MySchema is non-unique in the left schema list. The schemas names are case-insensitive." };
    issues.CompareIssues(expectedIssues);
    }

/*---------------------------------------------------------------------------------------
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, DuplicateSchemaNamesRightMergeNoReferences)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MyOtherSchema" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MYOTHERSCHEMA" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();

    // Merge the schemas
    SchemaMergeOptions options;
    options.SetDoNotMergeReferences(true);
    SchemaMergeResult result;
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas, options));

    // Compare issues
    bvector<Utf8String> expectedIssues { "The schema name entry MyOtherSchema is non-unique in the right schema list. The schemas names are case-insensitive.",
                                         "The schema name entry MySchema is non-unique in the right schema list. The schemas names are case-insensitive." };
    issues.CompareIssues(expectedIssues);
    }
  
/*---------------------------------------------------------------------------------------
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, DuplicateSchemaNamesLeftAndRightMergeNoReferences)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="MySchema1" alias="mys" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="myschema1" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
          </ECEntityClass>
        </ECSchema>
        )schema"

    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="myschema2" alias="mys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="myschema2" alias="mys" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="C">
            <BaseClass>B</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();

    // Merge the schemas
    SchemaMergeOptions options;
    options.SetDoNotMergeReferences(true);
    SchemaMergeResult result;
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas, options));

    // Compare issues
    bvector<Utf8String> expectedIssues { "The schema name entry myschema1 is non-unique in the left schema list. The schemas names are case-insensitive.", 
                                         "The schema name entry myschema2 is non-unique in the right schema list. The schemas names are case-insensitive." };
    issues.CompareIssues(expectedIssues);
    }

/*---------------------------------------------------------------------------------------
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, UncleanSchemaGraphNoReferences)
    {
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestReference" alias="tr" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="MyProperty" typeName="int" />
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="TestReference" version="01.00.00" alias="tr"/>
        </ECSchema>
        )schema"
    };

    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    // Simulate an unclean schema graph with two different in-memory references of the same schema on one side
    bvector<Utf8CP> rightSchemas1Xml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestReference" alias="tr" version="01.00.07" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="MyProperty" typeName="int" />
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="TestReference" version="01.00.07" alias="tr"/>
        </ECSchema>
        )schema"
    };

    bvector<Utf8CP> rightSchemas2Xml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestReference" alias="tr" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <PropertyCategory typeName="MyCategory" priority="98"/>
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="MyProperty" typeName="int" category="MyCategory" />
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema2" alias="ts2" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="TestReference" version="01.00.00" alias="tr"/>
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="MyProperty" typeName="int" />
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemas1Xml);
    ECSchemaReadContextPtr right2Context = InitializeReadContextWithAllSchemas(rightSchemas2Xml);

    SchemaKey key1("TestSchema", 1, 0, 1);
    SchemaKey key2("TestSchema2", 1, 0, 0);
    ECSchemaPtr testSchema = rightContext->LocateSchema(key1, SchemaMatchType::Latest);
    ECSchemaPtr testSchema2 = right2Context->LocateSchema(key2, SchemaMatchType::Latest);
    EXPECT_TRUE(testSchema.IsValid());
    EXPECT_TRUE(testSchema2.IsValid());
    bvector<ECN::ECSchemaCP> rightSchemas;
    rightSchemas.push_back(testSchema.get());
    rightSchemas.push_back(testSchema2.get());

    // Merge the schemas
    SchemaMergeOptions options;
    options.SetDoNotMergeReferences(true);
    SchemaMergeResult result;
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas, options));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestReference" alias="tr" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="MyProperty" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="TestReference" version="01.00.00" alias="tr"/>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema2" alias="ts2" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="TestReference" version="01.00.00" alias="tr"/>
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="MyProperty" typeName="int"/>
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------------
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, UncleanSchemaGraphMergedWithReferences)
    {
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestReference" alias="tr" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="MyProperty" typeName="int" />
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="TestReference" version="01.00.00" alias="tr"/>
        </ECSchema>
        )schema"
    };

    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    // Simulate an unclean schema graph with two different in-memory references of the same schema on one side
    bvector<Utf8CP> rightSchemas1Xml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestReference" alias="tr" version="01.00.07" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="MyProperty" typeName="int" />
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="TestReference" version="01.00.07" alias="tr"/>
        </ECSchema>
        )schema"
    };

    bvector<Utf8CP> rightSchemas2Xml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestReference" alias="tr" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <PropertyCategory typeName="MyCategory" priority="98"/>
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="MyProperty" typeName="int" category="MyCategory" />
          </ECEntityClass>
        </ECSchema>
        )schema",
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema2" alias="ts2" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="TestReference" version="01.00.00" alias="tr"/>
          <ECEntityClass typeName="MyEntity">
            <ECProperty propertyName="MyProperty" typeName="int" />
          </ECEntityClass>
        </ECSchema>
        )schema"
    };

    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemas1Xml);
    ECSchemaReadContextPtr right2Context = InitializeReadContextWithAllSchemas(rightSchemas2Xml);

    SchemaKey key1("TestSchema", 1, 0, 1);
    SchemaKey key2("TestSchema2", 1, 0, 0);
    ECSchemaPtr testSchema = rightContext->LocateSchema(key1, SchemaMatchType::Latest);
    ECSchemaPtr testSchema2 = right2Context->LocateSchema(key2, SchemaMatchType::Latest);
    EXPECT_TRUE(testSchema.IsValid());
    EXPECT_TRUE(testSchema2.IsValid());
    bvector<ECN::ECSchemaCP> rightSchemas;
    rightSchemas.push_back(testSchema.get());
    rightSchemas.push_back(testSchema2.get());

    // Merge the schemas
    SchemaMergeResult result;
    TestIssueListener issues;
    result.AddIssueListener(issues);
    EXPECT_EQ(ECObjectsStatus::Error, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare issues
    bvector<Utf8String> expectedIssues { "Failed to find item with name MyCategory in right schema TestReference.01.00.07. This usually indicates a dirty schema graph where multiple memory references of the same schema with different contents are provided." };
    issues.CompareIssues(expectedIssues);
    }

/*---------------------------------------------------------------------------------------
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, SchemaAlphabeticalSortIssue)
    {
    // There was a bug where we internally stored differencing schemas in a bset with ascii comparator which put them into alphabetical order instead of dependency order.
    // This test reproduces the issue and ensures it no longer happens

    Utf8CP leftRefXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="ZRefSchema" alias="refSch" version="01.00.00" displayLabel="Raised Floor" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="CoreCustomAttributes" version="01.00.04" alias="CoreCA"/>
        <ECCustomAttributes>
            <DynamicSchema xmlns="CoreCustomAttributes.01.00.04"/>
        </ECCustomAttributes>
        <ECEntityClass typeName="PlateElementAspect" displayLabel="Base Plate">
        </ECEntityClass>
    </ECSchema>)xml";

    Utf8CP leftXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="testSchema" version="01.00.00" displayLabel="Raised Floor (Parametric Modeling)" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="CoreCustomAttributes" version="01.00.04" alias="CoreCA"/>
        <ECSchemaReference name="ZRefSchema" version="01.00.00" alias="refSch"/>
        <ECCustomAttributes>
            <DynamicSchema xmlns="CoreCustomAttributes.01.00.04"/>
        </ECCustomAttributes>
        <ECEntityClass typeName="Plate_DgnActiveParametersElementAspect" displayLabel="Base Plate">
            <BaseClass>refSch:PlateElementAspect</BaseClass>
        </ECEntityClass>
    </ECSchema>)xml";

    BentleyApi::bvector<BentleyApi::WString> schemaDirs;
    BentleyApi::BeFileName assetsDir;
    BentleyApi::BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsDir);
    assetsDir = assetsDir.AppendToPath(L"ECSchemas");
    schemaDirs.push_back(assetsDir.AppendToPath(L"Standard"));
    SearchPathSchemaFileLocaterPtr locater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(schemaDirs, true);

    ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->AddSchemaLocater(*locater);
    schemaContext->SetSkipValidation(true);

    BentleyApi::bvector<ECSchemaCP> existingSchemas, incomingSchemas;
    ECSchemaPtr refSchema;
    ECSchema::ReadFromXmlString(refSchema, leftRefXml, *schemaContext);
    existingSchemas.push_back(refSchema.get());
    ECSchemaPtr ecSchema;
    ECSchema::ReadFromXmlString(ecSchema, leftXml, *schemaContext);
    existingSchemas.push_back(ecSchema.get());

    Utf8CP rightRefXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="ZRefSchema" alias="refSch" version="01.00.00" displayLabel="Raised Floor" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.04" alias="CoreCA"/>
            <ECCustomAttributes>
                <DynamicSchema xmlns="CoreCustomAttributes.01.00.04"/>
            </ECCustomAttributes>
            <ECEntityClass typeName="Plate" displayLabel="Base Plate">
            </ECEntityClass>
            <ECEntityClass typeName="PlateElementAspect" displayLabel="Base Plate">
            </ECEntityClass>
        </ECSchema>)xml";

    Utf8CP rightXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="testSchema" version="01.00.00" displayLabel="Raised Floor (Parametric Modeling)" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.04" alias="CoreCA"/>
            <ECSchemaReference name="ZRefSchema" version="01.00.00" alias="refSch"/>
            <ECCustomAttributes>
                <DynamicSchema xmlns="CoreCustomAttributes.01.00.04"/>
            </ECCustomAttributes>
            <ECEntityClass typeName="Plate_DgnActiveParameters" displayLabel="Base Plate">
                <BaseClass>refSch:Plate</BaseClass>
            </ECEntityClass>
            <ECEntityClass typeName="Plate_DgnActiveParametersElementAspect" displayLabel="Base Plate">
                <BaseClass>refSch:PlateElementAspect</BaseClass>
            </ECEntityClass>
        </ECSchema>)xml";

    ECSchemaReadContextPtr  schemaContext2 = ECSchemaReadContext::CreateContext();
    SearchPathSchemaFileLocaterPtr locater2 = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(schemaDirs, true);

    schemaContext2->AddSchemaLocater(*locater2);
    schemaContext2->SetSkipValidation(true);
    ECSchemaPtr refSchema2;
    ECSchema::ReadFromXmlString(refSchema2, rightRefXml, *schemaContext2);
    incomingSchemas.push_back(refSchema2.get());
    ECSchemaPtr ecSchema2;
    ECSchema::ReadFromXmlString(ecSchema2, rightXml, *schemaContext2);
    incomingSchemas.push_back(ecSchema2.get());

    SchemaMergeResult result;
    SchemaMergeOptions options;
    options.SetKeepVersion(true);
    options.SetRenamePropertyOnConflict(true);
    options.SetRenameSchemaItemOnConflict(true);
    options.SetMergeOnlyDynamicSchemas(true);
    options.SetIgnoreIncompatiblePropertyTypeChanges(true);

    ECSchema::SortSchemasInDependencyOrder(existingSchemas);
    ECSchema::SortSchemasInDependencyOrder(incomingSchemas);

    auto mergeStatus = SchemaMerger::MergeSchemas(result, existingSchemas, incomingSchemas, options);
    EXPECT_EQ(ECObjectsStatus::Success, mergeStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, BasePropAndLocalPropSameNameWithDiffCase)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" nameSpacePrefix="test" version="01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" nameSpacePrefix="test" version="01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="Alpha">
                <ECProperty propertyName="PIPE_DIA" typeName="double" displayLabel="Pipe Diameter" />
            </ECClass>
            <ECClass typeName = "Bravo">
                <BaseClass>Alpha</BaseClass>
                <ECProperty propertyName="Pipe_Dia" typeName="double" displayLabel="Pipe Diameter" />
            </ECClass>
        </ECSchema>)xml"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" nameSpacePrefix="test" version="01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
          <ECClass typeName="Alpha">
                <ECProperty propertyName="PIPE_DIA" typeName="double" displayLabel="Pipe Diameter" />
            </ECClass>
            <ECClass typeName = "Bravo">
                <BaseClass>Alpha</BaseClass>
                <ECProperty propertyName="PIPE_DIA" typeName="double" displayLabel="Pipe_Dia" />
            </ECClass>
        </ECSchema>
        )schema"
    }; // Preserving old behavior, note here display label also changed, this was the old behavior, just preserving it

    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, BasePropAndLocalPropSameExactName)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="test" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="test" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Alpha">
                <ECProperty propertyName="PIPE_DIA" typeName="double" displayLabel="Pipe Diameter" />
            </ECEntityClass>
            <ECEntityClass typeName = "Bravo">
                <BaseClass>Alpha</BaseClass>
                <ECProperty propertyName="PIPE_DIA" typeName="double" displayLabel="Pipe Diameter" />
            </ECEntityClass>
        </ECSchema>)xml"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="test" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Alpha">
                <ECProperty propertyName="PIPE_DIA" typeName="double" displayLabel="Pipe Diameter"/>
            </ECEntityClass>
            <ECEntityClass typeName="Bravo">
                <BaseClass>Alpha</BaseClass>
                <ECProperty propertyName="PIPE_DIA" typeName="double" displayLabel="Pipe Diameter"/>
            </ECEntityClass>
    </ECSchema>
        )schema"
    }; 
    
    CompareResults(expectedSchemasXml, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaMergerTests, TwoLocalPropSameNameWithDiffCase)
    {
    // Initialize two sets of schemas
    bvector<Utf8CP> leftSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" nameSpacePrefix="test" version="01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="foo" isDomainClass="True">
                <ECArrayProperty propertyName="StringArray" typeName="string"  minOccurs="0" maxOccurs="10" />
                <ECProperty propertyName="String" typeName="string" />
            </ECClass>
            <ECClass typeName="Foo" >
                <ECProperty propertyName="String2" typeName="string" />
            </ECClass>
            <ECClass typeName="Goo" >
                <ECProperty propertyName="GooProp" typeName="string" />
                <ECProperty propertyName="gooprop" typeName="string" />
            </ECClass>
        </ECSchema>
        )schema"
    };
    ECSchemaReadContextPtr leftContext = InitializeReadContextWithAllSchemas(leftSchemasXml);
    bvector<ECN::ECSchemaCP> leftSchemas = leftContext->GetCache().GetSchemas();

    bvector<Utf8CP> rightSchemasXml {
      R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" nameSpacePrefix="test" version="01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="Alpha">
                <ECProperty propertyName="PIPE_DIA" typeName="double" displayLabel="Pipe Diameter" />
            </ECClass>
            <ECClass typeName = "Bravo">
                <BaseClass>Alpha</BaseClass>
                <ECProperty propertyName="Pipe_Dia" typeName="double" displayLabel="Pipe Diameter" />
            </ECClass>
        </ECSchema>)xml"
    };
    ECSchemaReadContextPtr rightContext = InitializeReadContextWithAllSchemas(rightSchemasXml);
    bvector<ECN::ECSchemaCP> rightSchemas = rightContext->GetCache().GetSchemas();
    
    //merge the schemas
    SchemaMergeResult result;
    EXPECT_EQ(ECObjectsStatus::Success, SchemaMerger::MergeSchemas(result, leftSchemas, rightSchemas));

    // Compare result
    bvector<Utf8CP> expectedSchemasXml {
      R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" nameSpacePrefix="test" version="01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="foo" isDomainClass="True">
                <ECArrayProperty propertyName="StringArray" typeName="string"  minOccurs="0" maxOccurs="10" />
                <ECProperty propertyName="String" typeName="string" />
            </ECClass>
            <ECClass typeName="Foo" >
                <ECProperty propertyName="String2" typeName="string" />
            </ECClass>
            <ECClass typeName="Goo" >
                <ECProperty propertyName="GooProp" typeName="string" />
            </ECClass>
            <ECClass typeName="Alpha">
                <ECProperty propertyName="PIPE_DIA" typeName="double" displayLabel="Pipe Diameter" />
            </ECClass>
            <ECClass typeName = "Bravo">
                <BaseClass>Alpha</BaseClass>
                <ECProperty propertyName="PIPE_DIA" typeName="double" displayLabel="Pipe_Dia" />
            </ECClass>
        </ECSchema>
        )schema"
    }; // Preserving old behavior, note here display label also changed, this was the old behavior, just preserving it

    CompareResults(expectedSchemasXml, result);
    }

END_BENTLEY_ECN_TEST_NAMESPACE

