/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"
#include <ECPresentation/Rules/PresentationRules.h>
#include "../../../../Source/Rules/CommonToolsInternal.h"

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct ClassNamesParserTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassNamesParserTests, ReturnsEmptyListWhenParsingEmptyString)
    {
    ClassNamesParser p("", true);
    EXPECT_TRUE(p.begin() == p.end());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassNamesParserTests, ReturnsEmptyListWhenOnlyClassSelectionFlagsAreSpecified)
    {
    ClassNamesParser p1("PE:", true);
    EXPECT_TRUE(p1.begin() == p1.end());

    ClassNamesParser p2("E:", true);
    EXPECT_TRUE(p2.begin() == p2.end());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassNamesParserTests, ReturnsEmptyListWhenOnlySchemaIsSpecified)
    {
    ClassNamesParser p("Schema:", false);
    EXPECT_TRUE(p.begin() == p.end());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassNamesParserTests, ReturnsEmptyListWhenOnlyClassSelectionFlagsAndSchemaAreSpecified)
    {
    ClassNamesParser p1("PE:Schema", true);
    EXPECT_TRUE(p1.begin() == p1.end());

    ClassNamesParser p2("PE:Schema:", true);
    EXPECT_TRUE(p2.begin() == p2.end());

    ClassNamesParser p3("E:Schema", true);
    EXPECT_TRUE(p3.begin() == p3.end());

    ClassNamesParser p4("E:Schema:", true);
    EXPECT_TRUE(p4.begin() == p4.end());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassNamesParserTests, ReturnsOneClassWithExclusion)
    {
    ClassNamesParser p1("Schema:Class", true);
    auto iter = p1.begin();
    EXPECT_TRUE(iter != p1.end());
    EXPECT_STREQ("Schema", (*iter).GetSchemaName());
    EXPECT_STREQ("Class", (*iter).GetClassName());
    EXPECT_TRUE((*iter).IsPolymorphic());
    EXPECT_FALSE((*iter).IsExclude());
    EXPECT_TRUE(++iter == p1.end());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassNamesParserTests, ReturnsOneClassWithoutExclusion)
    {
    ClassNamesParser p1("Schema:Class", false);
    auto iter = p1.begin();
    EXPECT_TRUE(iter != p1.end());
    EXPECT_STREQ("Schema", (*iter).GetSchemaName());
    EXPECT_STREQ("Class", (*iter).GetClassName());
    EXPECT_TRUE((*iter).IsPolymorphic());
    EXPECT_FALSE((*iter).IsExclude());
    EXPECT_TRUE(++iter == p1.end());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassNamesParserTests, ReturnsExcludedClass)
    {
    ClassNamesParser p1("E:Schema:Class", true);
    auto iter = p1.begin();
    EXPECT_TRUE(iter != p1.end());
    EXPECT_STREQ("Schema", (*iter).GetSchemaName());
    EXPECT_STREQ("Class", (*iter).GetClassName());
    EXPECT_FALSE((*iter).IsPolymorphic());
    EXPECT_TRUE((*iter).IsExclude());
    EXPECT_TRUE(++iter == p1.end());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassNamesParserTests, ReturnsPolymorphicallyExcludedClass)
    {
    ClassNamesParser p1("PE:Schema:Class", true);
    auto iter = p1.begin();
    EXPECT_TRUE(iter != p1.end());
    EXPECT_STREQ("Schema", (*iter).GetSchemaName());
    EXPECT_STREQ("Class", (*iter).GetClassName());
    EXPECT_TRUE((*iter).IsPolymorphic());
    EXPECT_TRUE((*iter).IsExclude());
    EXPECT_TRUE(++iter == p1.end());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassNamesParserTests, ReturnsMultipleClasses)
    {
    ClassNamesParser p1("Schema:Class1,Class2", true);
    auto iter = p1.begin();

    EXPECT_TRUE(iter != p1.end());
    EXPECT_STREQ("Schema", (*iter).GetSchemaName());
    EXPECT_STREQ("Class1", (*iter).GetClassName());
    EXPECT_TRUE((*iter).IsPolymorphic());
    EXPECT_FALSE((*iter).IsExclude());
    EXPECT_TRUE(++iter != p1.end());

    EXPECT_STREQ("Schema", (*iter).GetSchemaName());
    EXPECT_STREQ("Class2", (*iter).GetClassName());
    EXPECT_TRUE((*iter).IsPolymorphic());
    EXPECT_FALSE((*iter).IsExclude());
    EXPECT_TRUE(++iter == p1.end());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassNamesParserTests, ReturnsMultipleExcludedClasses)
    {
    ClassNamesParser p1("E:Schema1:Class1;Schema2:Class2", true);
    auto iter = p1.begin();
    EXPECT_TRUE(iter != p1.end());

    EXPECT_STREQ("Schema1", (*iter).GetSchemaName());
    EXPECT_STREQ("Class1", (*iter).GetClassName());
    EXPECT_FALSE((*iter).IsPolymorphic());
    EXPECT_TRUE((*iter).IsExclude());
    EXPECT_TRUE(++iter != p1.end());

    EXPECT_STREQ("Schema2", (*iter).GetSchemaName());
    EXPECT_STREQ("Class2", (*iter).GetClassName());
    EXPECT_FALSE((*iter).IsPolymorphic());
    EXPECT_TRUE((*iter).IsExclude());
    EXPECT_TRUE(++iter == p1.end());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassNamesParserTests, ReturnsMultiplePolymorphicallyExcludedClasses)
    {
    ClassNamesParser p1("PE:Schema1:Class1;Schema2:Class2", true);
    auto iter = p1.begin();
    EXPECT_TRUE(iter != p1.end());

    EXPECT_STREQ("Schema1", (*iter).GetSchemaName());
    EXPECT_STREQ("Class1", (*iter).GetClassName());
    EXPECT_TRUE((*iter).IsPolymorphic());
    EXPECT_TRUE((*iter).IsExclude());
    EXPECT_TRUE(++iter != p1.end());

    EXPECT_STREQ("Schema2", (*iter).GetSchemaName());
    EXPECT_STREQ("Class2", (*iter).GetClassName());
    EXPECT_TRUE((*iter).IsPolymorphic());
    EXPECT_TRUE((*iter).IsExclude());
    EXPECT_TRUE(++iter == p1.end());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassNamesParserTests, ReturnsMultipleSchemasAndClasses)
    {
    ClassNamesParser p1("SchemaA:Class1;SchemaB:Class1,Class2", true);
    auto iter = p1.begin();

    EXPECT_TRUE(iter != p1.end());
    EXPECT_STREQ("SchemaA", (*iter).GetSchemaName());
    EXPECT_STREQ("Class1", (*iter).GetClassName());
    EXPECT_TRUE((*iter).IsPolymorphic());
    EXPECT_FALSE((*iter).IsExclude());
    EXPECT_TRUE(++iter != p1.end());

    EXPECT_STREQ("SchemaB", (*iter).GetSchemaName());
    EXPECT_STREQ("Class1", (*iter).GetClassName());
    EXPECT_TRUE((*iter).IsPolymorphic());
    EXPECT_FALSE((*iter).IsExclude());
    EXPECT_TRUE(++iter != p1.end());

    EXPECT_STREQ("SchemaB", (*iter).GetSchemaName());
    EXPECT_STREQ("Class2", (*iter).GetClassName());
    EXPECT_TRUE((*iter).IsPolymorphic());
    EXPECT_FALSE((*iter).IsExclude());
    EXPECT_TRUE(++iter == p1.end());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassNamesParserTests, ReturnsMultipleSchemasAndClassesIgnoringSpaces)
    {
    ClassNamesParser p1("   SchemaA  :  Class  ;  SchemaB  :  Class  ", true);
    auto iter = p1.begin();

    EXPECT_TRUE(iter != p1.end());
    EXPECT_STREQ("SchemaA", (*iter).GetSchemaName());
    EXPECT_STREQ("Class", (*iter).GetClassName());
    EXPECT_TRUE((*iter).IsPolymorphic());
    EXPECT_FALSE((*iter).IsExclude());
    EXPECT_TRUE(++iter != p1.end());

    EXPECT_STREQ("SchemaB", (*iter).GetSchemaName());
    EXPECT_STREQ("Class", (*iter).GetClassName());
    EXPECT_TRUE((*iter).IsPolymorphic());
    EXPECT_FALSE((*iter).IsExclude());
    EXPECT_TRUE(++iter == p1.end());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassNamesParserTests, ReturnsIncludedAndExcludedClasses)
    {
    ClassNamesParser p1("SchemaA:Class; E:SchemaB:Class; PE:SchemaC:Class1,Class2; E:SchemaD:Class", true);
    auto iter = p1.begin();

    EXPECT_TRUE(iter != p1.end());
    EXPECT_STREQ("SchemaA", (*iter).GetSchemaName());
    EXPECT_STREQ("Class", (*iter).GetClassName());
    EXPECT_TRUE((*iter).IsPolymorphic());
    EXPECT_FALSE((*iter).IsExclude());
    EXPECT_TRUE(++iter != p1.end());

    EXPECT_STREQ("SchemaB", (*iter).GetSchemaName());
    EXPECT_STREQ("Class", (*iter).GetClassName());
    EXPECT_FALSE((*iter).IsPolymorphic());
    EXPECT_TRUE((*iter).IsExclude());
    EXPECT_TRUE(++iter != p1.end());

    EXPECT_STREQ("SchemaC", (*iter).GetSchemaName());
    EXPECT_STREQ("Class1", (*iter).GetClassName());
    EXPECT_TRUE((*iter).IsPolymorphic());
    EXPECT_TRUE((*iter).IsExclude());
    EXPECT_TRUE(++iter != p1.end());

    EXPECT_STREQ("SchemaC", (*iter).GetSchemaName());
    EXPECT_STREQ("Class2", (*iter).GetClassName());
    EXPECT_TRUE((*iter).IsPolymorphic());
    EXPECT_TRUE((*iter).IsExclude());
    EXPECT_TRUE(++iter != p1.end());

    EXPECT_STREQ("SchemaD", (*iter).GetSchemaName());
    EXPECT_STREQ("Class", (*iter).GetClassName());
    EXPECT_FALSE((*iter).IsPolymorphic());
    EXPECT_TRUE((*iter).IsExclude());
    EXPECT_TRUE(++iter == p1.end());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassNamesParserTests, IgnoresExclusionFlagsIfNotSupported)
    {
    ClassNamesParser p1("E:SchemaA:Class; PE:SchemaB:Class", false);
    auto iter = p1.begin();

    EXPECT_TRUE(iter != p1.end());
    EXPECT_STREQ("SchemaA", (*iter).GetSchemaName());
    EXPECT_STREQ("Class", (*iter).GetClassName());
    EXPECT_TRUE((*iter).IsPolymorphic());
    EXPECT_FALSE((*iter).IsExclude());
    EXPECT_TRUE(++iter != p1.end());

    EXPECT_STREQ("SchemaB", (*iter).GetSchemaName());
    EXPECT_STREQ("Class", (*iter).GetClassName());
    EXPECT_TRUE((*iter).IsPolymorphic());
    EXPECT_FALSE((*iter).IsExclude());
    EXPECT_TRUE(++iter == p1.end());
    }
