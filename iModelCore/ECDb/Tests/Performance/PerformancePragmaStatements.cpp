/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

class PeformancePragmaStatements : public ECDbTestFixture
    {};

TEST_F(PeformancePragmaStatements, PurgeOrphanRelationships)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDbForCurrentTest());

    const auto schemaXml = R"xml(
        <?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="rst" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
	        <ECSchemaReference name="ECDbMap" version="2.0.0" alias="ecdbmap" />
            <ECEntityClass typeName="ClassA" modifier="None">
                <ECProperty propertyName="ClassAProp" typeName="string" displayLabel="ClassAProp"/>
            </ECEntityClass>
            <ECEntityClass typeName="ClassB" modifier="None">
                <ECProperty propertyName="ClassBProp" typeName="string" displayLabel="ClassBProp"/>
            </ECEntityClass>
            <ECRelationshipClass typeName="RelA_A" modifier="Sealed" strength="referencing" strengthDirection="backward">
                <ECCustomAttributes>
                    <LinkTableRelationshipMap xmlns="ECDbMap.2.0.0">
                        <CreateForeignKeyConstraints>False</CreateForeignKeyConstraints>
                    </LinkTableRelationshipMap>
                </ECCustomAttributes>
                <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                    <Class class="ClassA"/>
                </Source>
                <Target multiplicity="(1..*)" roleLabel="is referenced by" polymorphic="true">
                    <Class class="ClassA"/>
                </Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="RelB_B" modifier="Sealed" strength="referencing" strengthDirection="backward">
                <ECCustomAttributes>
                    <LinkTableRelationshipMap xmlns="ECDbMap.2.0.0">
                        <CreateForeignKeyConstraints>False</CreateForeignKeyConstraints>
                    </LinkTableRelationshipMap>
                </ECCustomAttributes>
                <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                    <Class class="ClassB"/>
                </Source>
                <Target multiplicity="(1..*)" roleLabel="is referenced by" polymorphic="true">
                    <Class class="ClassB"/>
                </Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="RelA_B" modifier="Sealed" strength="referencing" strengthDirection="backward">
                <ECCustomAttributes>
                    <LinkTableRelationshipMap xmlns="ECDbMap.2.0.0">
                        <CreateForeignKeyConstraints>False</CreateForeignKeyConstraints>
                    </LinkTableRelationshipMap>
                </ECCustomAttributes>
                <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                    <Class class="ClassA"/>
                </Source>
                <Target multiplicity="(1..*)" roleLabel="is referenced by" polymorphic="true">
                    <Class class="ClassB"/>
                </Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="RelB_A" modifier="Sealed" strength="referencing" strengthDirection="backward">
                <ECCustomAttributes>
                    <LinkTableRelationshipMap xmlns="ECDbMap.2.0.0">
                        <CreateForeignKeyConstraints>False</CreateForeignKeyConstraints>
                    </LinkTableRelationshipMap>
                </ECCustomAttributes>
                <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                    <Class class="ClassB"/>
                </Source>
                <Target multiplicity="(1..*)" roleLabel="is referenced by" polymorphic="true">
                    <Class class="ClassA"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    const auto context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(m_ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas()).IsOk());

    auto insertEntry = [&](Utf8CP className, Utf8CP propName)
        {
        ECSqlStatement stmt;
        ECInstanceKey outKey;
        if (ECSqlStatus::Success == stmt.Prepare(m_ecdb, Utf8PrintfString("INSERT INTO TestSchema.%s(%sProp) VALUES('%s')", className, className, propName).c_str()))
            stmt.Step(outKey);
        return outKey;
        };

    auto insertRelationship = [&](Utf8CP className, const ECInstanceKey& sourceKey, const ECInstanceKey& targetKey)
        {
        ECSqlStatement stmt;
        ECInstanceKey outKey;
        if (ECSqlStatus::Success == stmt.Prepare(m_ecdb, Utf8PrintfString("INSERT INTO TestSchema.%s(SourceECInstanceId, TargetECInstanceId) VALUES(%s,%s)", className, sourceKey.GetInstanceId().ToString().c_str(), targetKey.GetInstanceId().ToString().c_str()).c_str()))
            stmt.Step(outKey);
        return outKey;
        };

    bvector<Utf8String> classAInstancesToDelete;
    bvector<Utf8String> classBInstancesToDelete;

    auto propertyIndex = 1U;
    for (auto index = 1U; index <= 10000U; ++index)
        {
        const auto commonClassA = insertEntry("ClassA", Utf8String("A" + std::to_string(++propertyIndex)).c_str());
        ASSERT_TRUE(commonClassA.IsValid());
        const auto commonClassB = insertEntry("ClassB", Utf8String("B" + std::to_string(++propertyIndex)).c_str());
        ASSERT_TRUE(commonClassB.IsValid());

        insertRelationship("RelA_B", commonClassA, commonClassB);
        insertRelationship("RelB_A", commonClassB, commonClassA);

        classAInstancesToDelete.push_back(commonClassA.GetInstanceId().ToString());
        classBInstancesToDelete.push_back(commonClassB.GetInstanceId().ToString());

        // Insert a relationship A(deleted) -> A
        insertRelationship("RelA_A", commonClassA, insertEntry("ClassA", Utf8String("A" + std::to_string(++propertyIndex)).c_str()));

        // Insert two relationships B -> B(deleted) -> B
        insertRelationship("RelB_B", insertEntry("ClassB", Utf8String("B" + std::to_string(++propertyIndex)).c_str()), commonClassB);
        insertRelationship("RelB_B", commonClassB, insertEntry("ClassB", Utf8String("B" + std::to_string(++propertyIndex)).c_str()));

        /* Insert relationships:
                    A       A(deleted)
                   / \     / \
                  B   \   /   B
                        B(deleted)
        */
        const auto classAInstance = insertEntry("ClassA", Utf8String("A" + std::to_string(++propertyIndex)).c_str());
        const auto classBInstance = insertEntry("ClassB", Utf8String("B" + std::to_string(++propertyIndex)).c_str());
        insertRelationship("RelA_B", commonClassA, insertEntry("ClassB", Utf8String("B" + std::to_string(++propertyIndex)).c_str()));
        insertRelationship("RelA_B", classAInstance, commonClassB);
        insertRelationship("RelA_B", classAInstance, classBInstance);

        /* Insert relationships:
                B(deleted)  B        B
                   / \     /         |
                  /   \   /          |
                 /     \ /           |
                A       A(deleted)   A
        */
        insertRelationship("RelB_A", commonClassB, insertEntry("ClassA", Utf8String("A" + std::to_string(++propertyIndex)).c_str()));
        insertRelationship("RelB_A", insertEntry("ClassB", Utf8String("B" + std::to_string(++propertyIndex)).c_str()), commonClassA);
        insertRelationship("RelB_A", insertEntry("ClassB", Utf8String("B" + std::to_string(++propertyIndex)).c_str()), insertEntry("ClassA", Utf8String("A" + std::to_string(++propertyIndex)).c_str()));
        }

    // Check that no orphan relationships exist
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "pragma integrity_check(check_linktable_fk_ids) options enable_experimental_features"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    // Delete the common classes to create orphan relationships
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, Utf8PrintfString("DELETE FROM TestSchema.ClassA WHERE ECInstanceId in (%s)", BeStringUtilities::Join(classAInstancesToDelete, ",").c_str()).c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, Utf8PrintfString("DELETE FROM TestSchema.ClassB WHERE ECInstanceId in (%s)", BeStringUtilities::Join(classBInstancesToDelete, ",").c_str()).c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    // Run the integrity check command to verify that there are orphan relationship rows
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "pragma integrity_check(check_linktable_fk_ids) options enable_experimental_features"));
    auto rowCount = 0U;
    while (stmt.Step() == BE_SQLITE_ROW)
        rowCount++;
    stmt.Finalize();
    ASSERT_GT(rowCount, 0U);

    // Run the purge_orphan_relationships to remove the orphan relationship rows
    StopWatch timer(true);
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "pragma purge_orphan_relationships options enable_experimental_features"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    timer.Stop();
    stmt.Finalize();

    // Run the integrity check command to verify that all orphan relationship rows have been deleted
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "pragma integrity_check(check_linktable_fk_ids) options enable_experimental_features"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), rowCount, Utf8String("Purging " + std::to_string(rowCount) + " orphan relationships.").c_str());
    }

END_ECDBUNITTESTS_NAMESPACE