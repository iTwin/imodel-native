/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "SchemaUpdateTestFixture.h"
#if defined(USE_GTEST)
#include <gtest/gtest-spi.h>
#endif

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

#if defined(USE_GTEST)
//=======================================================================================
// Checks that the harness reports broken data. Each test breaks something on purpose and
// expects VerifyData to report it.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SchemaUpdateHarnessTests : SchemaUpdateTestFixture
    {
    static SchemaUpdateBaseline const& GetBaseline()
        {
        static SchemaUpdateBaseline s_baseline("SchemaUpdateHarness", R"xml(<?xml version="1.0" encoding="utf-8"?>
<ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
    <ECEntityClass typeName="Element" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
            <ShareColumns xmlns="ECDbMap.02.00.00">
                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
            </ShareColumns>
        </ECCustomAttributes>
        <ECProperty propertyName="Code" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Pipe">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="Material" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Valve">
        <BaseClass>Element</BaseClass>
        <ECNavigationProperty propertyName="AttachedPipe" relationshipName="PipeHasValves" direction="Backward" />
    </ECEntityClass>
    <ECEntityClass typeName="Sensor">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="Reading" typeName="double" />
    </ECEntityClass>
    <ECEntityClass typeName="Note">
        <ECProperty propertyName="NoteText" typeName="string" />
        <ECProperty propertyName="Priority" typeName="int" />
    </ECEntityClass>
    <ECRelationshipClass typeName="PipeHasValves" strength="referencing" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="has" polymorphic="false">
            <Class class="Pipe" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is attached to" polymorphic="false">
            <Class class="Valve" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="PipeRefersToNotes" strength="referencing" modifier="None">
        <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
            <Class class="Pipe" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is referred to by" polymorphic="true">
            <Class class="Note" />
        </Target>
        <ECProperty propertyName="Reason" typeName="string" />
    </ECRelationshipClass>
</ECSchema>)xml");
        return s_baseline;
        }

    //! Runs the check and returns the failures it reported, without failing the test.
    static Utf8String CaptureFailures(std::function<void()> const& check)
        {
        ::testing::TestPartResultArray failures;
            {
            ::testing::ScopedFakeTestPartResultReporter reporter(::testing::ScopedFakeTestPartResultReporter::INTERCEPT_ONLY_CURRENT_THREAD, &failures);
            check();
            }

        Utf8String messages;
        for (int i = 0; i < failures.size(); i++)
            messages.append(failures.GetTestPartResult(i).message()).append("\n");

        return messages;
        }

    Utf8String CaptureVerifyDataFailures(ExpectedDataChanges const& expectedChanges = ExpectedDataChanges())
        {
        return CaptureFailures([&] () { VerifyData(expectedChanges); });
        }

    //! Deleting a property of a class with its own table is rejected with ECDb_0386.
    void RejectUpdate()
        {
        Utf8String schemaXml = GetBaselineSchemaXml();
        ReplaceInClass(schemaXml, "Note", R"xml(<ECProperty propertyName="Priority" typeName="int" />)xml", "");
        ASSERT_EQ(ERROR, UpdateSchema(schemaXml, SchemaVersionChange::Major));
        }

    static bool Contains(Utf8StringCR messages, Utf8CP text) { return messages.find(text) != Utf8String::npos; }
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateHarnessTests, ReportsUndeclaredDeletedInstances)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String schemaXml = GetBaselineSchemaXml();
    RemoveClass(schemaXml, "Sensor");
    ASSERT_EQ(SUCCESS, UpdateSchema(schemaXml, SchemaVersionChange::Major));

    Utf8String failures = CaptureVerifyDataFailures();
    EXPECT_TRUE(Contains(failures, "TestSchema:Sensor") && Contains(failures, "is missing after the update")) << failures.c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateHarnessTests, ReportsInstancesThatWereExpectedToBeDeleted)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String failures = CaptureVerifyDataFailures(ExpectedDataChanges().DeleteInstancesOf("TestSchema:Note"));
    EXPECT_TRUE(Contains(failures, "was expected to be deleted")) << failures.c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateHarnessTests, ReportsChangedValues)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("UPDATE ts.Pipe SET Material='Changed'"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("UPDATE ts.PipeRefersToNotes SET Reason='Changed'"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());

    Utf8String failures = CaptureVerifyDataFailures();
    EXPECT_TRUE(Contains(failures, "property Material")) << failures.c_str();
    EXPECT_TRUE(Contains(failures, "property Reason")) << failures.c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateHarnessTests, ReportsReferencingInstancesThatReferenceNothingDeleted)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));

    Utf8String failures = CaptureVerifyDataFailures(ExpectedDataChanges().DeleteReferencingInstancesOf("TestSchema:Valve"));
    EXPECT_TRUE(Contains(failures, "DeleteReferencingInstancesOf declares nothing")) << failures.c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateHarnessTests, ReportsAddedInstances)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.Note(NoteText, Priority) VALUES('Added', 1)"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());

    Utf8String failures = CaptureVerifyDataFailures();
    EXPECT_TRUE(Contains(failures, "appeared after the update")) << failures.c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateHarnessTests, ReportsRejectionWithoutReason)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    RejectUpdate();

    Utf8String failures = CaptureVerifyDataFailures();
    EXPECT_TRUE(Contains(failures, "Call ExpectRejectionReason")) << failures.c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateHarnessTests, ReportsWrongRejectionReason)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    RejectUpdate();

    Utf8String failures = CaptureFailures([&] () { ExpectRejectionReason("ECDb_0001"); });
    EXPECT_TRUE(Contains(failures, "was not rejected with ECDb_0001") && Contains(failures, "ECDb_0386")) << failures.c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateHarnessTests, ReportsAcceptedUpdateExpectedToBeRejected)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaseline()));
    Utf8String schemaXml = GetBaselineSchemaXml();
    ReplaceInClass(schemaXml, "Note", R"xml(<ECProperty propertyName="Priority" typeName="int" />)xml", R"xml(<ECProperty propertyName="Priority" typeName="int" displayLabel="Note priority" />)xml");
    ASSERT_EQ(SUCCESS, UpdateSchema(schemaXml));

    Utf8String failures = CaptureFailures([&] () { ExpectRejectionReason("ECDb_0386"); });
    EXPECT_TRUE(Contains(failures, "was accepted")) << failures.c_str();
    }
#endif

END_ECDBUNITTESTS_NAMESPACE
