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

    //! A referenced schema with data and a schema deriving from it.
    static SchemaUpdateBaseline const& GetBaselineWithReferencedSchema()
        {
        static SchemaUpdateBaseline s_baseline("SchemaUpdateHarnessWithReference", bvector<Utf8String>{R"xml(<?xml version="1.0" encoding="utf-8"?>
<ECSchema schemaName="HarnessBase" alias="hb" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
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
    <ECEntityClass typeName="Marker">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="Label" typeName="string" />
    </ECEntityClass>
</ECSchema>)xml",
R"xml(<?xml version="1.0" encoding="utf-8"?>
<ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="HarnessBase" version="01.00.00" alias="hb" />
    <ECEntityClass typeName="Pipe">
        <BaseClass>hb:Element</BaseClass>
        <ECProperty propertyName="Material" typeName="string" />
    </ECEntityClass>
</ECSchema>)xml"});
        return s_baseline;
        }

    //! Relationship ends that list two classes under an abstract constraint, for a navigation property and a link table.
    static SchemaUpdateBaseline const& GetBaselineWithMultiClassConstraints()
        {
        static SchemaUpdateBaseline s_baseline("SchemaUpdateHarnessMultiClassConstraints", R"xml(<?xml version="1.0" encoding="utf-8"?>
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
    <ECEntityClass typeName="Pump">
        <BaseClass>Element</BaseClass>
        <ECProperty propertyName="Power" typeName="double" />
    </ECEntityClass>
    <ECEntityClass typeName="Valve">
        <BaseClass>Element</BaseClass>
        <ECNavigationProperty propertyName="Supplier" relationshipName="SupplierFeedsValves" direction="Backward" />
    </ECEntityClass>
    <ECEntityClass typeName="Note">
        <ECProperty propertyName="NoteText" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="SupplierFeedsValves" strength="referencing" modifier="Sealed">
        <Source multiplicity="(0..1)" roleLabel="feeds" polymorphic="false" abstractConstraint="Element">
            <Class class="Pipe" />
            <Class class="Pump" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is fed by" polymorphic="false">
            <Class class="Valve" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="SupplierRefersToNotes" strength="referencing" modifier="Sealed">
        <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="false" abstractConstraint="Element">
            <Class class="Pipe" />
            <Class class="Pump" />
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is referred to by" polymorphic="false">
            <Class class="Note" />
        </Target>
    </ECRelationshipClass>
</ECSchema>)xml");
        return s_baseline;
        }

    int CountRows(Utf8CP ecsql) const
        {
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql)) << ecsql;
        return BE_SQLITE_ROW == stmt.Step() ? stmt.GetValueInt(0) : -1;
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
TEST_F(SchemaUpdateHarnessTests, ReportsChangedValuesInReferencedSchema)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaselineWithReferencedSchema()));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("UPDATE hb.Marker SET Label='Changed'"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("UPDATE ts.Pipe SET Code='Changed'"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());

    Utf8String failures = CaptureVerifyDataFailures();
    EXPECT_TRUE(Contains(failures, "HarnessBase:Marker") && Contains(failures, "property Label")) << failures.c_str();
    EXPECT_TRUE(Contains(failures, "TestSchema:Pipe") && Contains(failures, "property Code")) << failures.c_str();
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

//---------------------------------------------------------------------------------------
// The integrity checks for navigation and link table ids have to look up the ids in every class of the
// referenced constraint, not only in the first one.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateHarnessTests, AcceptsIdsOfEveryConstraintClass)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaselineWithMultiClassConstraints()));
    ASSERT_LT(0, CountRows("SELECT COUNT(*) FROM ts.Valve v JOIN ts.Pump p ON v.Supplier.Id=p.ECInstanceId")) << "The baseline needs navigation values pointing to the second constraint class.";
    ASSERT_LT(0, CountRows("SELECT COUNT(*) FROM ts.SupplierRefersToNotes r JOIN ts.Pump p ON r.SourceECInstanceId=p.ECInstanceId")) << "The baseline needs relationships from the second constraint class.";

    Utf8String failures = CaptureVerifyDataFailures();
    EXPECT_TRUE(failures.empty()) << failures.c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaUpdateHarnessTests, ReportsNavigationIdsPointingToNothing)
    {
    ASSERT_EQ(SUCCESS, SetupFromBaseline(GetBaselineWithMultiClassConstraints()));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("UPDATE ts.Valve SET Supplier.Id=2147483647 WHERE Supplier.Id IS NOT NULL"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());

    Utf8String failures = CaptureVerifyDataFailures();
    EXPECT_TRUE(Contains(failures, "check_nav_ids")) << failures.c_str();
    }
#endif

END_ECDBUNITTESTS_NAMESPACE
