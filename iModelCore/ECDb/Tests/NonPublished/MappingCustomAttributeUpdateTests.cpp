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

    //! XML of one DbIndex. Pass nullptr for isUnique or where to leave them out.
    static Utf8String DbIndexXml(Utf8CP name, Utf8CP isUnique, std::vector<Utf8CP> const& propertyNames, Utf8CP where = nullptr)
        {
        Utf8String xml;
        xml.append("<DbIndex><Name>").append(name).append("</Name>");
        if (isUnique != nullptr)
            xml.append("<IsUnique>").append(isUnique).append("</IsUnique>");

        xml.append("<Properties>");
        for (Utf8CP propertyName : propertyNames)
            xml.append("<string>").append(propertyName).append("</string>");

        xml.append("</Properties>");
        if (where != nullptr)
            xml.append("<Where>").append(where).append("</Where>");

        return xml.append("</DbIndex>");
        }

    //! Baseline schema with a DbIndexList on Markup or Comment.
    Utf8String WithIndexes(Utf8CP className, Utf8StringCR dbIndexesXml) const
        {
        Utf8String schemaXml = GetBaselineSchemaXml();
        Utf8PrintfString indexListXml(R"xml(<DbIndexList xmlns="ECDbMap.02.00.00"><Indexes>%s</Indexes></DbIndexList>)xml", dbIndexesXml.c_str());
        if (0 == strcmp(className, "Markup"))
            ReplaceInClass(schemaXml, "Markup", "</ClassMap>", Utf8PrintfString("</ClassMap>\n            %s", indexListXml.c_str()).c_str());
        else
            ReplaceInClass(schemaXml, className, "<BaseClass>", Utf8PrintfString("<ECCustomAttributes>%s</ECCustomAttributes>\n        <BaseClass>", indexListXml.c_str()).c_str());

        return schemaXml;
        }

    //! Inserts a comment with the Text and Author of an existing comment and returns the result of the insert.
    DbResult InsertCommentLikeExistingOne()
        {
        ECSqlStatement select;
        EXPECT_EQ(ECSqlStatus::Success, select.Prepare(m_ecdb, "SELECT Text, Author FROM ts.Comment ORDER BY ECInstanceId LIMIT 1"));
        EXPECT_EQ(BE_SQLITE_ROW, select.Step());
        ECSqlStatement insert;
        EXPECT_EQ(ECSqlStatus::Success, insert.Prepare(m_ecdb, "INSERT INTO ts.Comment (Text, Author) VALUES (?, ?)"));
        insert.BindText(1, select.GetValueText(0), IECSqlBinder::MakeCopy::Yes);
        insert.BindText(2, select.GetValueText(1), IECSqlBinder::MakeCopy::Yes);
        const DbResult result = insert.Step();
        insert.Finalize();
        select.Finalize();
        m_ecdb.AbandonChanges();
        return result;
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
// DbIndexList
// New indexes and changed index properties are allowed on existing classes. ECDb builds them on the
// existing rows, so a unique index fails when the data has duplicates.
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, AddIndex)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    ExpectAccepted(WithIndexes("Markup", DbIndexXml("ix_markup_text", "False", {"Text"})));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, AddUniqueIndex)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    EXPECT_EQ(BE_SQLITE_DONE, InsertCommentLikeExistingOne());

    ExpectAccepted(WithIndexes("Comment", DbIndexXml("uix_comment_text_author", "True", {"Text", "Author"})));
    EXPECT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, InsertCommentLikeExistingOne());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, AddingUniqueIndexOnDuplicateValuesIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    ChangeDataBeforeUpdate("UPDATE ts.Markup SET Text = 'Draft'");
    ExpectRejected(WithIndexes("Markup", DbIndexXml("uix_markup_text", "True", {"Text"})), "ECDb_0227", SchemaVersionChange::Minor);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, AddPropertyToIndex)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    ExpectAccepted(WithIndexes("Comment", DbIndexXml("ix_comment", "False", {"Text"})));
    ExpectAccepted(WithIndexes("Comment", DbIndexXml("ix_comment", "False", {"Text", "Author"})));
    }

//---------------------------------------------------------------------------------------
// Every comment gets the same Text, so the unique index holds only together with Author.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, RemovingPropertyFromUniqueIndexOnDuplicateValuesIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    ExpectAccepted(WithIndexes("Comment", DbIndexXml("uix_comment", "True", {"Text", "Author"})));

    ChangeDataBeforeUpdate("UPDATE ts.Comment SET Text = 'Draft'");
    ExpectRejected(WithIndexes("Comment", DbIndexXml("uix_comment", "True", {"Text"})), "ECDb_0227", SchemaVersionChange::Minor);
    }

//---------------------------------------------------------------------------------------
// Rejected although the data has no duplicates.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, MakingIndexUniqueIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    ExpectAccepted(WithIndexes("Markup", DbIndexXml("ix_markup_text", "False", {"Text"})));
    ExpectRejected(WithIndexes("Markup", DbIndexXml("ix_markup_text", "True", {"Text"})), "ECDb_0354", SchemaVersionChange::Minor);
    }

//---------------------------------------------------------------------------------------
// Rejected although a non-unique index fits any data.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, MakingIndexNonUniqueIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    ExpectAccepted(WithIndexes("Markup", DbIndexXml("uix_markup_text", "True", {"Text"})));
    ExpectRejected(WithIndexes("Markup", DbIndexXml("uix_markup_text", "False", {"Text"})), "ECDb_0354", SchemaVersionChange::Minor);
    }

//---------------------------------------------------------------------------------------
// IsUnique defaults to false. Setting it where it was left out counts as a new value, which ECDb accepts,
// while changing it from False to True is rejected.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, SetIsUniqueOnIndexWithoutIt)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    ExpectAccepted(WithIndexes("Comment", DbIndexXml("ix_comment", nullptr, {"Text", "Author"})));
    EXPECT_EQ(BE_SQLITE_DONE, InsertCommentLikeExistingOne());

    ExpectAccepted(WithIndexes("Comment", DbIndexXml("ix_comment", "True", {"Text", "Author"})));
    EXPECT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, InsertCommentLikeExistingOne());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, SettingIsUniqueOnIndexWithoutItOnDuplicateValuesIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    ExpectAccepted(WithIndexes("Comment", DbIndexXml("ix_comment", nullptr, {"Text"})));

    ChangeDataBeforeUpdate("UPDATE ts.Comment SET Text = 'Draft'");
    ExpectRejected(WithIndexes("Comment", DbIndexXml("ix_comment", "True", {"Text"})), "ECDb_0227", SchemaVersionChange::Minor);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, SetWhereOnIndexWithoutIt)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    ExpectAccepted(WithIndexes("Markup", DbIndexXml("ix_markup_text", "False", {"Text"})));
    ExpectAccepted(WithIndexes("Markup", DbIndexXml("ix_markup_text", "False", {"Text"}, "IndexedColumnsAreNotNull")));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, RemovingIndexIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    ExpectAccepted(WithIndexes("Comment", DbIndexXml("ix_comment_text", "False", {"Text"}).append(DbIndexXml("ix_comment_author", "False", {"Author"}))));
    ExpectRejected(WithIndexes("Comment", DbIndexXml("ix_comment_text", "False", {"Text"})), "ECDb_0354", SchemaVersionChange::Minor);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, RenamingIndexIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    ExpectAccepted(WithIndexes("Comment", DbIndexXml("ix_comment_text", "False", {"Text"})));
    ExpectRejected(WithIndexes("Comment", DbIndexXml("ix_comment_body", "False", {"Text"})), "ECDb_0354", SchemaVersionChange::Minor);
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MappingCustomAttributeUpdateTests, InsertBaseClassWithNewProperty)
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

//=======================================================================================
// ECDbMap:QueryView and ECDbMap:ForeignKeyView. View classes take their rows from an ECSQL query
// over other classes and store no data, so the harness leaves them out and the tests query them.
// ECDb accepts changes to the query of an existing view and rejects adding or removing QueryView
// and every ForeignKeyView change on existing classes with ECDb_0354.
// ECDb checks all view classes after every import, so an update that breaks a view query fails.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ViewUpdateTests : SchemaUpdateTestFixture
    {
    //! PumpView is a view over Pump. Pump keeps its properties in shared columns, so they can be deleted.
    //! PumpSummary is a plain abstract class that could become a view.
    static SchemaUpdateBaseline const& GetBaseline()
        {
        static SchemaUpdateBaseline s_baseline("QueryViews", R"xml(<?xml version="1.0" encoding="utf-8"?>
<ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="ECDbMap" version="02.00.04" alias="ecdbmap" />
    <ECEntityClass typeName="Equipment" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00.04">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
            <ShareColumns xmlns="ECDbMap.02.00.04">
                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
            </ShareColumns>
        </ECCustomAttributes>
        <ECProperty propertyName="Code" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Pump">
        <BaseClass>Equipment</BaseClass>
        <ECProperty propertyName="Tag" typeName="string" />
        <ECProperty propertyName="Power" typeName="double" />
    </ECEntityClass>
    <ECEntityClass typeName="PumpView" modifier="Abstract">
        <ECCustomAttributes>
            <QueryView xmlns="ECDbMap.02.00.04">
                <Query>SELECT ECInstanceId, ECClassId, Tag FROM ts.Pump</Query>
            </QueryView>
        </ECCustomAttributes>
        <ECProperty propertyName="Tag" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="PumpSummary" modifier="Abstract">
        <ECProperty propertyName="Tag" typeName="string" />
    </ECEntityClass>
</ECSchema>)xml");
        return s_baseline;
        }

    //! OwnedPump and ListedPump are views over Pump with a navigation property to Person.
    //! Only the relationship of OwnedPump has ForeignKeyView.
    static SchemaUpdateBaseline const& GetForeignKeyViewBaseline()
        {
        static SchemaUpdateBaseline s_baseline("ForeignKeyViews", R"xml(<?xml version="1.0" encoding="utf-8"?>
<ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="ECDbMap" version="02.00.04" alias="ecdbmap" />
    <ECEntityClass typeName="Person">
        <ECProperty propertyName="Name" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Pump">
        <ECProperty propertyName="Tag" typeName="string" />
        <ECNavigationProperty propertyName="Owner" relationshipName="PersonOwnsPump" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="PersonOwnsPump" strength="referencing" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="false">
            <Class class="Person" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="false">
            <Class class="Pump" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="OwnedPump" modifier="Abstract">
        <ECCustomAttributes>
            <QueryView xmlns="ECDbMap.02.00.04">
                <Query>SELECT ECInstanceId, ECClassId, Tag, Owner FROM ts.Pump</Query>
            </QueryView>
        </ECCustomAttributes>
        <ECProperty propertyName="Tag" typeName="string" />
        <ECNavigationProperty propertyName="Owner" relationshipName="PersonOwnsOwnedPump" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="PersonOwnsOwnedPump" strength="referencing" modifier="Abstract">
        <ECCustomAttributes>
            <ForeignKeyView xmlns="ECDbMap.02.00.04" />
        </ECCustomAttributes>
        <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="false">
            <Class class="Person" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="false">
            <Class class="OwnedPump" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="ListedPump" modifier="Abstract">
        <ECCustomAttributes>
            <QueryView xmlns="ECDbMap.02.00.04">
                <Query>SELECT ECInstanceId, ECClassId, Tag, Owner FROM ts.Pump</Query>
            </QueryView>
        </ECCustomAttributes>
        <ECProperty propertyName="Tag" typeName="string" />
        <ECNavigationProperty propertyName="Owner" relationshipName="PersonListsPump" direction="Backward" />
    </ECEntityClass>
    <ECRelationshipClass typeName="PersonListsPump" strength="referencing" modifier="Abstract">
        <Source multiplicity="(0..1)" roleLabel="lists" polymorphic="false">
            <Class class="Person" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is listed by" polymorphic="false">
            <Class class="ListedPump" />
        </Target>
    </ECRelationshipClass>
</ECSchema>)xml");
        return s_baseline;
        }

    //! Expects the update to be accepted, then verifies all data.
    void ExpectAccepted(Utf8StringCR schemaXml, SchemaVersionChange versionChange = SchemaVersionChange::Minor)
        {
        EXPECT_EQ(SUCCESS, UpdateSchema(schemaXml, versionChange));
        VerifyData();
        }

    //! Expects the update to be rejected with the given issue, then verifies that nothing changed.
    void ExpectRejected(Utf8StringCR schemaXml, Utf8CP issueId = "ECDb_0354", SchemaVersionChange versionChange = SchemaVersionChange::Minor)
        {
        EXPECT_EQ(ERROR, UpdateSchema(schemaXml, versionChange));
        ExpectRejectionReason(issueId);
        VerifyData();
        }

    //! Expects every row of the view to be the pump with the same ECInstanceId and Tag, and returns the row count.
    int ReadPumpView(Utf8CP viewClassName)
        {
        ECSqlStatement stmt;
        Utf8PrintfString ecsql("SELECT v.ECInstanceId, v.Tag, p.Tag FROM ts.%s v LEFT JOIN ts.Pump p ON p.ECInstanceId = v.ECInstanceId", viewClassName);
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str() << " " << m_ecdb.GetLastError().c_str();
        int rowCount = 0;
        while (BE_SQLITE_ROW == stmt.Step())
            {
            ++rowCount;
            EXPECT_FALSE(stmt.IsValueNull(2)) << viewClassName << " returned a row that is no pump: " << stmt.GetValueId<ECInstanceId>(0).ToHexStr().c_str();
            EXPECT_STREQ(stmt.GetValueText(2), stmt.GetValueText(1)) << viewClassName;
            }

        return rowCount;
        }

    int CountRows(Utf8CP ecsql)
        {
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql)) << ecsql;
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << ecsql;
        return stmt.GetValueInt(0);
        }
    };

//***************************************************************************************
// QueryView
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ViewUpdateTests, ChangeViewQuery)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    ASSERT_EQ(CountRows("SELECT COUNT(*) FROM ts.Pump"), ReadPumpView("PumpView"));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "PumpView", "FROM ts.Pump</Query>", "FROM ts.Pump ORDER BY ECInstanceId LIMIT 1</Query>");
    ExpectAccepted(schemaXml);

    EXPECT_EQ(1, ReadPumpView("PumpView"));
    }

//---------------------------------------------------------------------------------------
// The view class and its query get the new property in the same update.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ViewUpdateTests, AddPropertyToView)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "PumpView", "ECClassId, Tag FROM", "ECClassId, Tag, Power FROM");
    ReplaceInClass(schemaXml, "PumpView", R"xml(<ECProperty propertyName="Tag" typeName="string" />)xml", R"xml(<ECProperty propertyName="Tag" typeName="string" />
        <ECProperty propertyName="Power" typeName="double" />)xml");
    ExpectAccepted(schemaXml);

    EXPECT_EQ(CountRows("SELECT COUNT(*) FROM ts.Pump"), CountRows("SELECT COUNT(*) FROM ts.PumpView v JOIN ts.Pump p ON p.ECInstanceId = v.ECInstanceId AND p.Power = v.Power"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ViewUpdateTests, AddViewClass)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    AppendToSchema(schemaXml, R"xml(
    <ECEntityClass typeName="FirstPump" modifier="Abstract">
        <ECCustomAttributes>
            <QueryView xmlns="ECDbMap.02.00.04">
                <Query>SELECT ECInstanceId, ECClassId, Tag FROM ts.Pump ORDER BY ECInstanceId LIMIT 1</Query>
            </QueryView>
        </ECCustomAttributes>
        <ECProperty propertyName="Tag" typeName="string" />
    </ECEntityClass>
)xml");
    ExpectAccepted(schemaXml);

    EXPECT_EQ(1, ReadPumpView("FirstPump"));
    }

//---------------------------------------------------------------------------------------
// A view stores no data, so deleting it only needs the rules of class deletion.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ViewUpdateTests, DeleteViewClass)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    RemoveClass(schemaXml, "PumpView");
    ExpectAccepted(schemaXml, SchemaVersionChange::Major);

    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT Tag FROM ts.PumpView"));
    }

//---------------------------------------------------------------------------------------
// Rejected although PumpSummary stores no data and its properties match the query.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ViewUpdateTests, MakingClassViewIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "PumpSummary", R"xml(<ECProperty propertyName="Tag" typeName="string" />)xml", R"xml(<ECCustomAttributes>
            <QueryView xmlns="ECDbMap.02.00.04">
                <Query>SELECT ECInstanceId, ECClassId, Tag FROM ts.Pump</Query>
            </QueryView>
        </ECCustomAttributes>
        <ECProperty propertyName="Tag" typeName="string" />)xml");
    ExpectRejected(schemaXml);
    }

//---------------------------------------------------------------------------------------
// Rejected although the view stores no data.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ViewUpdateTests, RemovingQueryViewIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "PumpView", R"xml(<QueryView xmlns="ECDbMap.02.00.04">
                <Query>SELECT ECInstanceId, ECClassId, Tag FROM ts.Pump</Query>
            </QueryView>)xml", "");
    ExpectRejected(schemaXml);
    }

//---------------------------------------------------------------------------------------
// Deleting Tag is a valid major change for Pump, but the query of PumpView uses it.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ViewUpdateTests, DeletingPropertyUsedByViewIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Pump", R"xml(<ECProperty propertyName="Tag" typeName="string" />)xml", "");
    ExpectRejected(schemaXml, "ECDb_0706", SchemaVersionChange::Major);
    }

//***************************************************************************************
// ForeignKeyView
//***************************************************************************************

//---------------------------------------------------------------------------------------
// Rejected although the relationship stores no data.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ViewUpdateTests, AddingForeignKeyViewIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetForeignKeyViewBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "PersonListsPump", R"xml(<Source multiplicity="(0..1)" roleLabel="lists" polymorphic="false">)xml", R"xml(<ECCustomAttributes>
            <ForeignKeyView xmlns="ECDbMap.02.00.04" />
        </ECCustomAttributes>
        <Source multiplicity="(0..1)" roleLabel="lists" polymorphic="false">)xml");
    ExpectRejected(schemaXml);
    }

//---------------------------------------------------------------------------------------
// Rejected although the relationship stores no data.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ViewUpdateTests, RemovingForeignKeyViewIsRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetForeignKeyViewBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "PersonOwnsOwnedPump", R"xml(<ECCustomAttributes>
            <ForeignKeyView xmlns="ECDbMap.02.00.04" />
        </ECCustomAttributes>)xml", "");
    ExpectRejected(schemaXml);
    }

END_ECDBUNITTESTS_NAMESPACE
