/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "SchemaUpdateTestFixture.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// ECDbMap custom attributes that decide how classes and properties are stored.
// ECDb rejects adding, changing or removing them on existing classes and properties with ECDb_0354.
// That rule does not depend on the schema version, so those updates come with a read version increment.
// Classes and properties added by an update bring their custom attributes without that rule, so mapping
// decides what happens to the existing data below them.
// Each update leaves a schema that ECDb could map if it were imported into an empty file.
//
// Three ways classes are mapped:
// - Element: TablePerHierarchy with a joined table and shared columns, like BisCore elements
// - Markup: TablePerHierarchy without joined tables and shared columns
// - Note: its own table, with a PropertyMap on NoteText
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct MappingCustomAttributeUpdateTests : SchemaUpdateTestFixture
    {
    static SchemaUpdateBaseline const& GetBaseline()
        {
        static SchemaUpdateBaseline s_baseline("MappingCustomAttributes", R"xml(<?xml version="1.0" encoding="utf-8"?>
<ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
    <ECEntityClass typeName="Element" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
            <ShareColumns xmlns="ECDbMap.02.00.00">
                <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
            </ShareColumns>
            <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00" />
        </ECCustomAttributes>
        <ECProperty propertyName="Code" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="PhysicalElement" modifier="Abstract">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="Category" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Pipe">
        <BaseClass>PhysicalElement</BaseClass>
        <ECProperty propertyName="Diameter" typeName="double" />
        <ECProperty propertyName="Material" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Valve">
        <BaseClass>PhysicalElement</BaseClass>
        <ECProperty propertyName="Pressure" typeName="double" />
    </ECEntityClass>
    <ECEntityClass typeName="Markup">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="Text" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Comment">
        <BaseClass>Markup</BaseClass>
        <ECProperty propertyName="Author" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Note">
        <ECProperty propertyName="NoteText" typeName="string">
            <ECCustomAttributes>
                <PropertyMap xmlns="ECDbMap.02.00.00">
                    <Collation>NoCase</Collation>
                </PropertyMap>
            </ECCustomAttributes>
        </ECProperty>
        <ECProperty propertyName="Priority" typeName="int" />
    </ECEntityClass>
</ECSchema>)xml");
        return s_baseline;
        }

    //! Expects the update to be accepted, then verifies all data.
    void ExpectAccepted(Utf8StringCR schemaXml)
        {
        EXPECT_EQ(SUCCESS, UpdateSchema(schemaXml));
        VerifyData();
        }

    //! Expects the update to be rejected with the given issue, then verifies that nothing changed.
    void ExpectRejected(Utf8StringCR schemaXml, Utf8CP issueId = "ECDb_0354", SchemaVersionChange versionChange = SchemaVersionChange::Major)
        {
        EXPECT_EQ(ERROR, UpdateSchema(schemaXml, versionChange));
        ExpectRejectionReason(issueId);
        VerifyData();
        }

    //! Inserts the abstract class Annotation with the given custom attributes between Markup and Comment.
    static Utf8String InsertAnnotation(Utf8StringCR schemaXml, Utf8CP customAttributesXml)
        {
        Utf8String updatedXml = schemaXml;
        AppendToSchema(updatedXml, Utf8PrintfString(R"xml(
    <ECEntityClass typeName="Annotation" modifier="Abstract">
        <ECCustomAttributes>
            %s
        </ECCustomAttributes>
        <BaseClass>Markup</BaseClass>
    </ECEntityClass>
)xml", customAttributesXml).c_str());
        ReplaceInClass(updatedXml, "Comment", "<BaseClass>Markup</BaseClass>", "<BaseClass>Annotation</BaseClass>");
        return updatedXml;
        }

    //! Adds a property to Comment and a new subclass of Annotation.
    static Utf8String ExtendAnnotationHierarchy(Utf8StringCR schemaXml)
        {
        Utf8String updatedXml = schemaXml;
        ReplaceInClass(updatedXml, "Comment", R"xml(<ECProperty propertyName="Author" typeName="string" />)xml", R"xml(<ECProperty propertyName="Author" typeName="string" />
        <ECProperty propertyName="Mood" typeName="string" />)xml");
        AppendToSchema(updatedXml, R"xml(
    <ECEntityClass typeName="Reply">
        <BaseClass>Annotation</BaseClass>
        <ECProperty propertyName="Topic" typeName="string" />
    </ECEntityClass>
)xml");
        return updatedXml;
        }
    };

//***************************************************************************************
// ClassMap
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, ChangingMapStrategyIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Markup", "<MapStrategy>TablePerHierarchy</MapStrategy>", "<MapStrategy>OwnTable</MapStrategy>");
    ExpectRejected(schemaXml);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, AddingClassMapIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Note", R"xml(typeName="Note">)xml", R"xml(typeName="Note">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>)xml");
    ExpectRejected(schemaXml);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, RemovingClassMapIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Markup", R"xml(<ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>)xml", "");
    ExpectRejected(schemaXml);
    }

//***************************************************************************************
// ShareColumns and JoinedTablePerDirectSubclass
//***************************************************************************************

//---------------------------------------------------------------------------------------
// Rejected even though a higher limit could apply to columns allocated from now on, leaving existing columns alone.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, RaisingMaxSharedColumnsBeforeOverflowIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Element", "<MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>", "<MaxSharedColumnsBeforeOverflow>8</MaxSharedColumnsBeforeOverflow>");
    ExpectRejected(schemaXml);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, ChangingApplyToSubclassesOnlyIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Element", "<ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>", "<ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>");
    ExpectRejected(schemaXml);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, AddingShareColumnsIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Markup", "</ClassMap>", R"xml(</ClassMap>
            <ShareColumns xmlns="ECDbMap.02.00.00" />)xml");
    ExpectRejected(schemaXml);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, RemovingShareColumnsIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Element", R"xml(<ShareColumns xmlns="ECDbMap.02.00.00">
                <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
            </ShareColumns>)xml", "");
    ExpectRejected(schemaXml);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, AddingJoinedTablePerDirectSubclassIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Markup", "</ClassMap>", R"xml(</ClassMap>
            <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00" />)xml");
    ExpectRejected(schemaXml);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, RemovingJoinedTablePerDirectSubclassIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Element", R"xml(<JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00" />)xml", "");
    ExpectRejected(schemaXml);
    }

//***************************************************************************************
// PropertyMap
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, ChangingCollationIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Note", "<Collation>NoCase</Collation>", "<Collation>RTrim</Collation>");
    ExpectRejected(schemaXml);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, RemovingPropertyMapIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Note", R"xml(<ECCustomAttributes>
                <PropertyMap xmlns="ECDbMap.02.00.00">
                    <Collation>NoCase</Collation>
                </PropertyMap>
            </ECCustomAttributes>)xml", "");
    ExpectRejected(schemaXml);
    }

//---------------------------------------------------------------------------------------
// The baseline has a value for every Priority, so the data would fit.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, AddingNotNullPropertyMapIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Note", R"xml(<ECProperty propertyName="Priority" typeName="int" />)xml", R"xml(<ECProperty propertyName="Priority" typeName="int">
            <ECCustomAttributes>
                <PropertyMap xmlns="ECDbMap.02.00.00">
                    <IsNullable>False</IsNullable>
                </PropertyMap>
            </ECCustomAttributes>
        </ECProperty>)xml");
    ExpectRejected(schemaXml);
    }

//---------------------------------------------------------------------------------------
// The baseline values of Priority are unique, so the data would fit.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, AddingUniquePropertyMapIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Note", R"xml(<ECProperty propertyName="Priority" typeName="int" />)xml", R"xml(<ECProperty propertyName="Priority" typeName="int">
            <ECCustomAttributes>
                <PropertyMap xmlns="ECDbMap.02.00.00">
                    <IsUnique>True</IsUnique>
                </PropertyMap>
            </ECCustomAttributes>
        </ECProperty>)xml");
    ExpectRejected(schemaXml);
    }

//***************************************************************************************
// Custom attributes on new classes and properties
//***************************************************************************************

//---------------------------------------------------------------------------------------
// Comment keeps its columns, also for properties added later. Only classes mapped after the update
// share columns, so the file is laid out differently than after importing the same schema into an empty file.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, InsertBaseClassWithShareColumns)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = InsertAnnotation(GetBaselineSchemaXml(), R"xml(<ShareColumns xmlns="ECDbMap.02.00.00" />)xml");
    ExpectAccepted(schemaXml);
    ExpectAccepted(ExtendAnnotationHierarchy(schemaXml));
    }

//---------------------------------------------------------------------------------------
// Comment stays in the table of Markup, also for properties added later. Only classes mapped after the update
// get a joined table, so the file is laid out differently than after importing the same schema into an empty file.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, InsertBaseClassWithJoinedTablePerDirectSubclass)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = InsertAnnotation(GetBaselineSchemaXml(), R"xml(<JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00" />)xml");
    ExpectAccepted(schemaXml);
    ExpectAccepted(ExtendAnnotationHierarchy(schemaXml));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, InsertingBaseClassWithOwnTableIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = InsertAnnotation(GetBaselineSchemaXml(), R"xml(<ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>OwnTable</MapStrategy>
            </ClassMap>)xml");
    ExpectRejected(schemaXml, "ECDb_0061", SchemaVersionChange::Minor);
    }

//---------------------------------------------------------------------------------------
// Element already shares columns for its subclasses.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, InsertingBaseClassWithShareColumnsIntoSharedHierarchyIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    AppendToSchema(schemaXml, R"xml(
    <ECEntityClass typeName="LinearElement" modifier="Abstract">
        <ECCustomAttributes>
            <ShareColumns xmlns="ECDbMap.02.00.00">
                <MaxSharedColumnsBeforeOverflow>1</MaxSharedColumnsBeforeOverflow>
            </ShareColumns>
        </ECCustomAttributes>
        <BaseClass>PhysicalElement</BaseClass>
    </ECEntityClass>
)xml");
    ReplaceInClass(schemaXml, "Pipe", "<BaseClass>PhysicalElement</BaseClass>", "<BaseClass>LinearElement</BaseClass>");
    ExpectRejected(schemaXml, "ECDb_0239", SchemaVersionChange::Minor);
    }

//---------------------------------------------------------------------------------------
// SQLite cannot add a NOT NULL column without a default value to a table.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, AddingNotNullPropertyIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Note", R"xml(<ECProperty propertyName="Priority" typeName="int" />)xml", R"xml(<ECProperty propertyName="Priority" typeName="int" />
        <ECProperty propertyName="Owner" typeName="string">
            <ECCustomAttributes>
                <PropertyMap xmlns="ECDbMap.02.00.00">
                    <IsNullable>False</IsNullable>
                </PropertyMap>
            </ECCustomAttributes>
        </ECProperty>)xml");
    ExpectRejected(schemaXml, "ECDb_0226", SchemaVersionChange::Minor);
    }

//---------------------------------------------------------------------------------------
// SQLite cannot add a UNIQUE column to a table. Existing notes would read NULL, which does not violate it.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, AddingUniquePropertyIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Note", R"xml(<ECProperty propertyName="Priority" typeName="int" />)xml", R"xml(<ECProperty propertyName="Priority" typeName="int" />
        <ECProperty propertyName="Owner" typeName="string">
            <ECCustomAttributes>
                <PropertyMap xmlns="ECDbMap.02.00.00">
                    <IsUnique>True</IsUnique>
                </PropertyMap>
            </ECCustomAttributes>
        </ECProperty>)xml");
    ExpectRejected(schemaXml, "ECDb_0226", SchemaVersionChange::Minor);
    }

//***************************************************************************************
// Hierarchy without shared columns
//***************************************************************************************

//---------------------------------------------------------------------------------------
// Comment inherits Subject from the inserted base class, so ECSQL with $-> has to find it on existing comments.
// The harness reads data without $->, so this test checks it separately.
// Disabled: the update stores no property map for Subject on Comment. $-> looks properties up in the stored
// property maps, so it skips all comments, while plain ECSQL and $ return Subject.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, DISABLED_InsertBaseClassWithNewProperty)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    AppendToSchema(schemaXml, R"xml(
    <ECEntityClass typeName="Annotation" modifier="Abstract">
        <BaseClass>Markup</BaseClass>
        <ECProperty propertyName="Subject" typeName="string" />
    </ECEntityClass>
)xml");
    ReplaceInClass(schemaXml, "Comment", "<BaseClass>Markup</BaseClass>", "<BaseClass>Annotation</BaseClass>");
    ExpectAccepted(schemaXml);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Comment SET Subject = 'Draft'"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM ts.Comment"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    const int commentCount = stmt.GetValueInt(0);
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT $->Subject FROM ts.Comment"));
    int rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ++rowCount;
        EXPECT_STREQ("Draft", stmt.GetValueText(0));
        }

    EXPECT_LT(0, commentCount);
    EXPECT_EQ(commentCount, rowCount);
    m_ecdb.AbandonChanges();
    }

END_ECDBUNITTESTS_NAMESPACE
