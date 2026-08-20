/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <ECDb/InstanceGraph.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
struct InstanceGraphTests : ECDbTestFixture
    {
    // Schema with link table rels, nav prop rels, inheritance, self-referential, and cycle-prone structures
    static constexpr Utf8CP s_testSchemaXml =
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="IGTest" alias="ig" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />

            <!-- ============ Entity Classes ============ -->
            <ECEntityClass typeName="Model">
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>

            <ECEntityClass typeName="Element" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="Code" typeName="string" />
                <ECNavigationProperty propertyName="Model" relationshipName="ModelHasElements" direction="Backward" />
            </ECEntityClass>

            <ECEntityClass typeName="PhysicalElement">
                <BaseClass>Element</BaseClass>
                <ECProperty propertyName="Geometry" typeName="string" />
            </ECEntityClass>

            <ECEntityClass typeName="FunctionalElement">
                <BaseClass>Element</BaseClass>
                <ECProperty propertyName="FuncData" typeName="string" />
            </ECEntityClass>

            <ECEntityClass typeName="Pipe">
                <BaseClass>PhysicalElement</BaseClass>
                <ECProperty propertyName="Diameter" typeName="double" />
            </ECEntityClass>

            <ECEntityClass typeName="Valve">
                <BaseClass>PhysicalElement</BaseClass>
                <ECProperty propertyName="MaxPressure" typeName="double" />
            </ECEntityClass>

            <ECEntityClass typeName="Category">
                <ECProperty propertyName="CatName" typeName="string" />
            </ECEntityClass>

            <!-- ============ End Table (Nav Prop) Relationships ============ -->
            <!-- Embedding: Model owns Elements via nav prop on Element -->
            <ECRelationshipClass typeName="ModelHasElements" strength="Embedding" modifier="Sealed">
                <Source multiplicity="(1..1)" polymorphic="False" roleLabel="Model">
                    <Class class="Model" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="Element">
                    <Class class="Element" />
                </Target>
            </ECRelationshipClass>

            <!-- ============ Link Table Relationships ============ -->
            <!-- Many-to-many: Element to Category -->
            <ECRelationshipClass typeName="ElementInCategory" strength="Referencing" modifier="Sealed">
                <Source multiplicity="(0..*)" polymorphic="True" roleLabel="has category">
                    <Class class="Element" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="categorizes">
                    <Class class="Category" />
                </Target>
            </ECRelationshipClass>

            <!-- Self-referential link table: Element connects to Element -->
            <ECRelationshipClass typeName="ElementConnectsToElement" strength="Referencing" modifier="None">
                <Source multiplicity="(0..*)" polymorphic="True" roleLabel="connects to">
                    <Class class="Element" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="connected from">
                    <Class class="Element" />
                </Target>
            </ECRelationshipClass>

            <!-- Derived relationship (TPH with base) -->
            <ECRelationshipClass typeName="PipeConnectsToPipe" modifier="Sealed">
                <BaseClass>ElementConnectsToElement</BaseClass>
                <Source multiplicity="(0..*)" polymorphic="True" roleLabel="pipe connects to">
                    <Class class="Pipe" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="pipe connected from">
                    <Class class="Pipe" />
                </Target>
            </ECRelationshipClass>

            <!-- Functional-Physical mapping (cross-hierarchy link table) -->
            <ECRelationshipClass typeName="FunctionalToPhysical" strength="Referencing" modifier="Sealed">
                <Source multiplicity="(0..*)" polymorphic="True" roleLabel="functional for">
                    <Class class="FunctionalElement" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="physical for">
                    <Class class="PhysicalElement" />
                </Target>
            </ECRelationshipClass>

        </ECSchema>)xml";

    ECInstanceKey InsertInstance(Utf8CP ecsql)
        {
        ECSqlStatement stmt;
        ECInstanceKey key;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        EXPECT_EQ(BE_SQLITE_DONE, stmt.Step(key));
        return key;
        }

    void InsertRelInstance(Utf8CP ecsql)
        {
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql));
        EXPECT_EQ(BE_SQLITE_DONE, stmt.Step());
        }

    ECClassId GetClassId(Utf8CP className) { return m_ecdb.Schemas().GetClassId("IGTest", className); }
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, LinkTable_ForwardTraversal)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_LinkTableFwd.ecdb", SchemaItem(s_testSchemaXml)));

    // Create: Model → Pipe1, Pipe1 in Category1
    auto modelKey = InsertInstance("INSERT INTO ig.Model(Name) VALUES('M1')");
    auto pipe1Key = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter, Model.Id) VALUES('P1', 100.0, 1)");
    auto cat1Key = InsertInstance("INSERT INTO ig.Category(CatName) VALUES('Cat1')");

    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementInCategory(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe1Key.GetInstanceId().ToString().c_str(), cat1Key.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    // Forward traversal from Pipe1: should find Category1 via ElementInCategory (Pipe1 is source)
    InstanceGraph graph(m_ecdb);
    graph.AddSeed(pipe1Key);
    ASSERT_EQ(SUCCESS, graph.ExpandNode(pipe1Key, TraversalDirection::Forward));

    auto const* related = graph.GetRelated(pipe1Key);
    ASSERT_NE(nullptr, related);

    // Should have at least the category (forward from source to target)
    bool foundCategory = false;
    for (auto const& rel : *related)
        {
        if (rel.GetKey().GetInstanceId() == cat1Key.GetInstanceId())
            {
            foundCategory = true;
            EXPECT_EQ(TraversalDirection::Forward, rel.GetDirection());
            EXPECT_EQ(GetClassId("ElementInCategory"), rel.GetRelClassId());
            }
        }
    EXPECT_TRUE(foundCategory) << "Should find Category1 via forward traversal";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, LinkTable_BackwardTraversal)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_LinkTableBwd.ecdb", SchemaItem(s_testSchemaXml)));

    auto modelKey = InsertInstance("INSERT INTO ig.Model(Name) VALUES('M1')");
    auto pipe1Key = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter, Model.Id) VALUES('P1', 100.0, 1)");
    auto cat1Key = InsertInstance("INSERT INTO ig.Category(CatName) VALUES('Cat1')");

    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementInCategory(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe1Key.GetInstanceId().ToString().c_str(), cat1Key.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    // Backward traversal from Category1: should find Pipe1 via ElementInCategory (Cat1 is target)
    InstanceGraph graph(m_ecdb);
    graph.AddSeed(cat1Key);
    ASSERT_EQ(SUCCESS, graph.ExpandNode(cat1Key, TraversalDirection::Backward));

    auto const* related = graph.GetRelated(cat1Key);
    ASSERT_NE(nullptr, related);

    bool foundPipe = false;
    for (auto const& rel : *related)
        {
        if (rel.GetKey().GetInstanceId() == pipe1Key.GetInstanceId())
            {
            foundPipe = true;
            EXPECT_EQ(TraversalDirection::Backward, rel.GetDirection());
            }
        }
    EXPECT_TRUE(foundPipe) << "Should find Pipe1 via backward traversal from Category";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, LinkTable_BothDirections)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_LinkTableBoth.ecdb", SchemaItem(s_testSchemaXml)));

    auto pipe1Key = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P1', 100.0)");
    auto pipe2Key = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P2', 200.0)");
    auto cat1Key = InsertInstance("INSERT INTO ig.Category(CatName) VALUES('Cat1')");

    // Pipe1 is source for ElementInCategory to Cat1
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementInCategory(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe1Key.GetInstanceId().ToString().c_str(), cat1Key.GetInstanceId().ToString().c_str()));
    // Pipe1 is also target of ElementConnectsToElement from Pipe2
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe2Key.GetInstanceId().ToString().c_str(), pipe1Key.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    // Both directions from Pipe1
    InstanceGraph graph(m_ecdb);
    graph.AddSeed(pipe1Key);
    ASSERT_EQ(SUCCESS, graph.ExpandNode(pipe1Key, TraversalDirection::Both));

    auto const* related = graph.GetRelated(pipe1Key);
    ASSERT_NE(nullptr, related);

    bool foundCat = false, foundPipe2 = false;
    for (auto const& rel : *related)
        {
        if (rel.GetKey().GetInstanceId() == cat1Key.GetInstanceId())
            foundCat = true;
        if (rel.GetKey().GetInstanceId() == pipe2Key.GetInstanceId())
            foundPipe2 = true;
        }
    EXPECT_TRUE(foundCat) << "Should find Cat1 forward via ElementInCategory";
    EXPECT_TRUE(foundPipe2) << "Should find Pipe2 backward via ElementConnectsToElement";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, NavProp_ForwardTraversal)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_NavPropFwd.ecdb", SchemaItem(s_testSchemaXml)));

    auto modelKey = InsertInstance("INSERT INTO ig.Model(Name) VALUES('M1')");
    auto pipe1Key = InsertInstance(SqlPrintfString("INSERT INTO ig.Pipe(Code, Diameter, Model.Id) VALUES('P1', 100.0, %s)",
        modelKey.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    // Forward from Model: Model is source of ModelHasElements → should find Pipe1
    InstanceGraph graph(m_ecdb);
    graph.AddSeed(modelKey);
    ASSERT_EQ(SUCCESS, graph.ExpandNode(modelKey, TraversalDirection::Forward));

    auto const* related = graph.GetRelated(modelKey);
    ASSERT_NE(nullptr, related);

    bool foundPipe = false;
    for (auto const& rel : *related)
        {
        if (rel.GetKey().GetInstanceId() == pipe1Key.GetInstanceId())
            {
            foundPipe = true;
            EXPECT_EQ(TraversalDirection::Forward, rel.GetDirection());
            EXPECT_EQ(GetClassId("ModelHasElements"), rel.GetRelClassId());
            }
        }
    EXPECT_TRUE(foundPipe) << "Should find Pipe1 forward from Model via NavProp";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, NavProp_BackwardTraversal)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_NavPropBwd.ecdb", SchemaItem(s_testSchemaXml)));

    auto modelKey = InsertInstance("INSERT INTO ig.Model(Name) VALUES('M1')");
    auto pipe1Key = InsertInstance(SqlPrintfString("INSERT INTO ig.Pipe(Code, Diameter, Model.Id) VALUES('P1', 100.0, %s)",
        modelKey.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    // Backward from Pipe1: Pipe1 is target of ModelHasElements → should find Model
    InstanceGraph graph(m_ecdb);
    graph.AddSeed(pipe1Key);
    ASSERT_EQ(SUCCESS, graph.ExpandNode(pipe1Key, TraversalDirection::Backward));

    auto const* related = graph.GetRelated(pipe1Key);
    ASSERT_NE(nullptr, related);

    bool foundModel = false;
    for (auto const& rel : *related)
        {
        if (rel.GetKey().GetInstanceId() == modelKey.GetInstanceId())
            {
            foundModel = true;
            EXPECT_EQ(TraversalDirection::Backward, rel.GetDirection());
            }
        }
    EXPECT_TRUE(foundModel) << "Should find Model backward from Pipe via NavProp";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, CycleAvoidance_DirectCycle)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_CycleDirect.ecdb", SchemaItem(s_testSchemaXml)));

    // A → B → A cycle via link table
    auto pipe1Key = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P1', 100.0)");
    auto pipe2Key = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P2', 200.0)");

    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe1Key.GetInstanceId().ToString().c_str(), pipe2Key.GetInstanceId().ToString().c_str()));
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe2Key.GetInstanceId().ToString().c_str(), pipe1Key.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    InstanceGraph graph(m_ecdb);
    graph.AddSeed(pipe1Key);
    ASSERT_EQ(SUCCESS, graph.ExpandAll(10));

    // Should visit exactly 2 nodes and terminate (no infinite loop)
    EXPECT_EQ(2u, graph.NodeCount());
    EXPECT_TRUE(graph.Contains(pipe1Key));
    EXPECT_TRUE(graph.Contains(pipe2Key));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, CycleAvoidance_TriangleCycle)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_CycleTriangle.ecdb", SchemaItem(s_testSchemaXml)));

    // A → B → C → A triangle cycle
    auto pipe1Key = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P1', 10.0)");
    auto pipe2Key = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P2', 20.0)");
    auto pipe3Key = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P3', 30.0)");

    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe1Key.GetInstanceId().ToString().c_str(), pipe2Key.GetInstanceId().ToString().c_str()));
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe2Key.GetInstanceId().ToString().c_str(), pipe3Key.GetInstanceId().ToString().c_str()));
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe3Key.GetInstanceId().ToString().c_str(), pipe1Key.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    InstanceGraph graph(m_ecdb);
    graph.AddSeed(pipe1Key);
    ASSERT_EQ(SUCCESS, graph.ExpandAll(10));

    EXPECT_EQ(3u, graph.NodeCount());
    EXPECT_TRUE(graph.Contains(pipe1Key));
    EXPECT_TRUE(graph.Contains(pipe2Key));
    EXPECT_TRUE(graph.Contains(pipe3Key));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, CycleAvoidance_SelfLoop)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_CycleSelf.ecdb", SchemaItem(s_testSchemaXml)));

    auto pipe1Key = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('SelfLoop', 50.0)");

    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe1Key.GetInstanceId().ToString().c_str(), pipe1Key.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    InstanceGraph graph(m_ecdb);
    graph.AddSeed(pipe1Key);
    ASSERT_EQ(SUCCESS, graph.ExpandAll(10));

    // Self-loop: only 1 node
    EXPECT_EQ(1u, graph.NodeCount());
    EXPECT_TRUE(graph.Contains(pipe1Key));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, Inheritance_DerivedRelationship)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_InheritRel.ecdb", SchemaItem(s_testSchemaXml)));

    // PipeConnectsToPipe is derived from ElementConnectsToElement
    auto pipe1Key = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P1', 100.0)");
    auto pipe2Key = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P2', 200.0)");

    // Insert as derived rel
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.PipeConnectsToPipe(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe1Key.GetInstanceId().ToString().c_str(), pipe2Key.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    // Forward from Pipe1
    InstanceGraph graph(m_ecdb);
    graph.AddSeed(pipe1Key);
    ASSERT_EQ(SUCCESS, graph.ExpandNode(pipe1Key, TraversalDirection::Forward));

    auto const* related = graph.GetRelated(pipe1Key);
    ASSERT_NE(nullptr, related);

    bool foundPipe2 = false;
    for (auto const& rel : *related)
        {
        if (rel.GetKey().GetInstanceId() == pipe2Key.GetInstanceId())
            {
            foundPipe2 = true;
            EXPECT_EQ(GetClassId("PipeConnectsToPipe"), rel.GetRelClassId());
            }
        }
    EXPECT_TRUE(foundPipe2) << "Should find Pipe2 via derived PipeConnectsToPipe relationship";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, DepthLimit_StopsAtMaxDepth)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_DepthLimit.ecdb", SchemaItem(s_testSchemaXml)));

    // Chain: P1 → P2 → P3 → P4 → P5
    auto p1 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P1', 10.0)");
    auto p2 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P2', 20.0)");
    auto p3 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P3', 30.0)");
    auto p4 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P4', 40.0)");
    auto p5 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P5', 50.0)");

    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        p1.GetInstanceId().ToString().c_str(), p2.GetInstanceId().ToString().c_str()));
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        p2.GetInstanceId().ToString().c_str(), p3.GetInstanceId().ToString().c_str()));
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        p3.GetInstanceId().ToString().c_str(), p4.GetInstanceId().ToString().c_str()));
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        p4.GetInstanceId().ToString().c_str(), p5.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    // Depth 2 from P1: should reach P1, P2, P3 but NOT P4, P5
    InstanceGraph graph(m_ecdb);
    graph.AddSeed(p1);
    ASSERT_EQ(SUCCESS, graph.ExpandAll(2));

    EXPECT_TRUE(graph.Contains(p1));
    EXPECT_TRUE(graph.Contains(p2));
    EXPECT_TRUE(graph.Contains(p3));
    EXPECT_FALSE(graph.Contains(p4)) << "Depth 2 should not reach P4";
    EXPECT_FALSE(graph.Contains(p5)) << "Depth 2 should not reach P5";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, MixedRelationships_LinkTableAndNavProp)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_Mixed.ecdb", SchemaItem(s_testSchemaXml)));

    // Model —(nav)→ Pipe1 —(link)→ Category1
    auto modelKey = InsertInstance("INSERT INTO ig.Model(Name) VALUES('M1')");
    auto pipe1Key = InsertInstance(SqlPrintfString("INSERT INTO ig.Pipe(Code, Diameter, Model.Id) VALUES('P1', 100.0, %s)",
        modelKey.GetInstanceId().ToString().c_str()));
    auto catKey = InsertInstance("INSERT INTO ig.Category(CatName) VALUES('Cat1')");

    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementInCategory(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe1Key.GetInstanceId().ToString().c_str(), catKey.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    // Full BFS from Model
    InstanceGraph graph(m_ecdb);
    graph.AddSeed(modelKey);
    ASSERT_EQ(SUCCESS, graph.ExpandAll(3));

    EXPECT_TRUE(graph.Contains(modelKey));
    EXPECT_TRUE(graph.Contains(pipe1Key)) << "Should find Pipe1 via nav prop";
    EXPECT_TRUE(graph.Contains(catKey)) << "Should find Category via link table through Pipe1";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, SetOps_Overlaps)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_Overlaps.ecdb", SchemaItem(s_testSchemaXml)));

    auto p1 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P1', 10.0)");
    auto p2 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P2', 20.0)");
    auto p3 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P3', 30.0)");

    // P1 → P2 and P3 → P2 (both reach P2)
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        p1.GetInstanceId().ToString().c_str(), p2.GetInstanceId().ToString().c_str()));
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        p3.GetInstanceId().ToString().c_str(), p2.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    InstanceGraph graphA(m_ecdb);
    graphA.AddSeed(p1);
    ASSERT_EQ(SUCCESS, graphA.ExpandAll(2));

    InstanceGraph graphB(m_ecdb);
    graphB.AddSeed(p3);
    ASSERT_EQ(SUCCESS, graphB.ExpandAll(2));

    EXPECT_TRUE(InstanceGraph::Overlaps(graphA, graphB)) << "Both graphs contain P2";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, SetOps_NoOverlap)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_NoOverlap.ecdb", SchemaItem(s_testSchemaXml)));

    auto p1 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P1', 10.0)");
    auto p2 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P2', 20.0)");
    auto p3 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P3', 30.0)");
    auto p4 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P4', 40.0)");

    // Disconnected: P1 → P2, P3 → P4
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        p1.GetInstanceId().ToString().c_str(), p2.GetInstanceId().ToString().c_str()));
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        p3.GetInstanceId().ToString().c_str(), p4.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    InstanceGraph graphA(m_ecdb);
    graphA.AddSeed(p1);
    ASSERT_EQ(SUCCESS, graphA.ExpandAll(2));

    InstanceGraph graphB(m_ecdb);
    graphB.AddSeed(p3);
    ASSERT_EQ(SUCCESS, graphB.ExpandAll(2));

    EXPECT_FALSE(InstanceGraph::Overlaps(graphA, graphB)) << "Disconnected graphs should not overlap";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, SetOps_Intersection)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_Intersection.ecdb", SchemaItem(s_testSchemaXml)));

    auto p1 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P1', 10.0)");
    auto p2 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P2', 20.0)");
    auto p3 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P3', 30.0)");

    // P1 → P2 and P3 → P2
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        p1.GetInstanceId().ToString().c_str(), p2.GetInstanceId().ToString().c_str()));
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        p3.GetInstanceId().ToString().c_str(), p2.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    InstanceGraph graphA(m_ecdb);
    graphA.AddSeed(p1);
    ASSERT_EQ(SUCCESS, graphA.ExpandAll(1));

    InstanceGraph graphB(m_ecdb);
    graphB.AddSeed(p3);
    ASSERT_EQ(SUCCESS, graphB.ExpandAll(1));

    auto intersection = InstanceGraph::Intersection(graphA, graphB);
    ASSERT_NE(nullptr, intersection);
    EXPECT_EQ(1u, intersection->NodeCount()) << "Only P2 is common";
    EXPECT_TRUE(intersection->Contains(p2));
    EXPECT_FALSE(intersection->Contains(p1));
    EXPECT_FALSE(intersection->Contains(p3));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, SetOps_Union)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_Union.ecdb", SchemaItem(s_testSchemaXml)));

    auto p1 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P1', 10.0)");
    auto p2 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P2', 20.0)");
    auto p3 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P3', 30.0)");
    auto p4 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P4', 40.0)");

    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        p1.GetInstanceId().ToString().c_str(), p2.GetInstanceId().ToString().c_str()));
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        p3.GetInstanceId().ToString().c_str(), p4.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    InstanceGraph graphA(m_ecdb);
    graphA.AddSeed(p1);
    ASSERT_EQ(SUCCESS, graphA.ExpandAll(2));

    InstanceGraph graphB(m_ecdb);
    graphB.AddSeed(p3);
    ASSERT_EQ(SUCCESS, graphB.ExpandAll(2));

    auto unionGraph = InstanceGraph::Union(graphA, graphB);
    ASSERT_NE(nullptr, unionGraph);
    EXPECT_EQ(4u, unionGraph->NodeCount());
    EXPECT_TRUE(unionGraph->Contains(p1));
    EXPECT_TRUE(unionGraph->Contains(p2));
    EXPECT_TRUE(unionGraph->Contains(p3));
    EXPECT_TRUE(unionGraph->Contains(p4));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, VTable_BasicQuery)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_VTable.ecdb", SchemaItem(s_testSchemaXml)));
    m_ecdb.GetECSqlConfig().SetExperimentalFeaturesEnabled(true);

    auto modelKey = InsertInstance("INSERT INTO ig.Model(Name) VALUES('M1')");
    auto pipe1Key = InsertInstance(SqlPrintfString("INSERT INTO ig.Pipe(Code, Diameter, Model.Id) VALUES('P1', 100.0, %s)",
        modelKey.GetInstanceId().ToString().c_str()));
    auto catKey = InsertInstance("INSERT INTO ig.Category(CatName) VALUES('Cat1')");

    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementInCategory(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe1Key.GetInstanceId().ToString().c_str(), catKey.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    // Query via virtual table (positional args map to hidden columns: ECInstanceId, ECClassId)
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        SqlPrintfString("SELECT RelatedECInstanceId, RelatedECClassId, Direction, RelationshipECClassId FROM ECVLib.Relations(%s, %s)",
            pipe1Key.GetInstanceId().ToString().c_str(),
            pipe1Key.GetClassId().ToString().c_str())));

    int rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        ++rowCount;

    EXPECT_GT(rowCount, 0) << "relations() vtable should return related instances for Pipe1";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, VTable_DirectionFilter)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_VTableDir.ecdb", SchemaItem(s_testSchemaXml)));
    m_ecdb.GetECSqlConfig().SetExperimentalFeaturesEnabled(true);

    auto modelKey = InsertInstance("INSERT INTO ig.Model(Name) VALUES('M1')");
    auto pipe1Key = InsertInstance(SqlPrintfString("INSERT INTO ig.Pipe(Code, Diameter, Model.Id) VALUES('P1', 100.0, %s)",
        modelKey.GetInstanceId().ToString().c_str()));
    auto catKey = InsertInstance("INSERT INTO ig.Category(CatName) VALUES('Cat1')");

    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementInCategory(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe1Key.GetInstanceId().ToString().c_str(), catKey.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    // Forward only (3rd positional arg = TraversalDirection)
    ECSqlStatement stmtFwd;
    ASSERT_EQ(ECSqlStatus::Success, stmtFwd.Prepare(m_ecdb,
        SqlPrintfString("SELECT RelatedECInstanceId FROM ECVLib.Relations(%s, %s, 'forward')",
            pipe1Key.GetInstanceId().ToString().c_str(),
            pipe1Key.GetClassId().ToString().c_str())));

    int fwdCount = 0;
    while (stmtFwd.Step() == BE_SQLITE_ROW)
        ++fwdCount;

    // Backward only
    ECSqlStatement stmtBwd;
    ASSERT_EQ(ECSqlStatus::Success, stmtBwd.Prepare(m_ecdb,
        SqlPrintfString("SELECT RelatedECInstanceId FROM ECVLib.Relations(%s, %s, 'backward')",
            pipe1Key.GetInstanceId().ToString().c_str(),
            pipe1Key.GetClassId().ToString().c_str())));

    int bwdCount = 0;
    while (stmtBwd.Step() == BE_SQLITE_ROW)
        ++bwdCount;

    // Forward should find Cat (ElementInCategory source), backward should find Model (ModelHasElements target)
    EXPECT_GT(fwdCount, 0) << "Forward should find related instances";
    EXPECT_GT(bwdCount, 0) << "Backward should find related instances";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, EmptySeed_ReturnsEmpty)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_Empty.ecdb", SchemaItem(s_testSchemaXml)));
    m_ecdb.SaveChanges();

    // Seed that doesn't exist — expand should succeed but return nothing
    ECInstanceKey fakeKey(GetClassId("Pipe"), ECInstanceId(UINT64_C(99999)));

    InstanceGraph graph(m_ecdb);
    graph.AddSeed(fakeKey);
    ASSERT_EQ(SUCCESS, graph.ExpandNode(fakeKey, TraversalDirection::Both));

    auto const* related = graph.GetRelated(fakeKey);
    ASSERT_NE(nullptr, related);
    EXPECT_EQ(0u, related->size()) << "No relationships should be found for nonexistent instance";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, MultipleSeeds)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_MultiSeed.ecdb", SchemaItem(s_testSchemaXml)));

    auto p1 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P1', 10.0)");
    auto p2 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P2', 20.0)");
    auto p3 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P3', 30.0)");
    auto cat = InsertInstance("INSERT INTO ig.Category(CatName) VALUES('Cat1')");

    // P1 → Cat, P2 → P3
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementInCategory(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        p1.GetInstanceId().ToString().c_str(), cat.GetInstanceId().ToString().c_str()));
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        p2.GetInstanceId().ToString().c_str(), p3.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    InstanceGraph graph(m_ecdb);
    graph.AddSeed(p1);
    graph.AddSeed(p2);
    ASSERT_EQ(SUCCESS, graph.ExpandAll(2));

    EXPECT_TRUE(graph.Contains(p1));
    EXPECT_TRUE(graph.Contains(p2));
    EXPECT_TRUE(graph.Contains(p3)) << "P3 reachable from seed P2";
    EXPECT_TRUE(graph.Contains(cat)) << "Cat reachable from seed P1";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, CrossHierarchyTraversal)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_CrossHierarchy.ecdb", SchemaItem(s_testSchemaXml)));

    auto funcElem = InsertInstance("INSERT INTO ig.FunctionalElement(Code, FuncData) VALUES('F1', 'data')");
    auto physElem = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P1', 100.0)");

    InsertRelInstance(SqlPrintfString("INSERT INTO ig.FunctionalToPhysical(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        funcElem.GetInstanceId().ToString().c_str(), physElem.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    // Forward from FunctionalElement
    InstanceGraph graph(m_ecdb);
    graph.AddSeed(funcElem);
    ASSERT_EQ(SUCCESS, graph.ExpandAll(1));

    EXPECT_TRUE(graph.Contains(physElem)) << "Should traverse cross-hierarchy FunctionalToPhysical";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, PolymorphicTraversal_MultipleSubclasses)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_Polymorphic.ecdb", SchemaItem(s_testSchemaXml)));

    // Model has 2 different subclasses of Element
    auto modelKey = InsertInstance("INSERT INTO ig.Model(Name) VALUES('M1')");
    auto pipeKey = InsertInstance(SqlPrintfString("INSERT INTO ig.Pipe(Code, Diameter, Model.Id) VALUES('P1', 100.0, %s)",
        modelKey.GetInstanceId().ToString().c_str()));
    auto valveKey = InsertInstance(SqlPrintfString("INSERT INTO ig.Valve(Code, MaxPressure, Model.Id) VALUES('V1', 500.0, %s)",
        modelKey.GetInstanceId().ToString().c_str()));
    auto funcKey = InsertInstance(SqlPrintfString("INSERT INTO ig.FunctionalElement(Code, FuncData, Model.Id) VALUES('F1', 'data', %s)",
        modelKey.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    // From Model: should find all element types
    InstanceGraph graph(m_ecdb);
    graph.AddSeed(modelKey);
    ASSERT_EQ(SUCCESS, graph.ExpandAll(1));

    EXPECT_TRUE(graph.Contains(pipeKey)) << "Should find Pipe (PhysicalElement subclass)";
    EXPECT_TRUE(graph.Contains(valveKey)) << "Should find Valve (PhysicalElement subclass)";
    EXPECT_TRUE(graph.Contains(funcKey)) << "Should find FunctionalElement";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, VTable_WithSchemaName)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_VTable_WithSchema.ecdb", SchemaItem(s_testSchemaXml)));
    m_ecdb.GetECSqlConfig().SetExperimentalFeaturesEnabled(true);

    auto modelKey = InsertInstance("INSERT INTO ig.Model(Name) VALUES('M1')");
    auto pipe1Key = InsertInstance(SqlPrintfString("INSERT INTO ig.Pipe(Code, Diameter, Model.Id) VALUES('P1', 100.0, %s)",
        modelKey.GetInstanceId().ToString().c_str()));
    auto catKey = InsertInstance("INSERT INTO ig.Category(CatName) VALUES('Cat1')");

    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementInCategory(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe1Key.GetInstanceId().ToString().c_str(), catKey.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    // Query via fully-qualified schema name: ECVLib.Relations(...)
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        SqlPrintfString("SELECT RelatedECInstanceId, RelatedECClassId, Direction, RelationshipECClassId FROM ECVLib.Relations(%s, %s)",
            pipe1Key.GetInstanceId().ToString().c_str(),
            pipe1Key.GetClassId().ToString().c_str())));

    int rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        ++rowCount;

    EXPECT_GT(rowCount, 0) << "Schema-qualified ECVLib.Relations() should return related instances";

    // Forward direction with schema name
    ECSqlStatement stmtFwd;
    ASSERT_EQ(ECSqlStatus::Success, stmtFwd.Prepare(m_ecdb,
        SqlPrintfString("SELECT RelatedECInstanceId FROM ECVLib.Relations(%s, %s, 'forward')",
            pipe1Key.GetInstanceId().ToString().c_str(),
            pipe1Key.GetClassId().ToString().c_str())));

    int fwdCount = 0;
    while (stmtFwd.Step() == BE_SQLITE_ROW)
        ++fwdCount;

    EXPECT_GT(fwdCount, 0) << "Schema-qualified forward query should return results";

    // Backward direction with schema name
    ECSqlStatement stmtBwd;
    ASSERT_EQ(ECSqlStatus::Success, stmtBwd.Prepare(m_ecdb,
        SqlPrintfString("SELECT RelatedECInstanceId FROM ECVLib.Relations(%s, %s, 'backward')",
            pipe1Key.GetInstanceId().ToString().c_str(),
            pipe1Key.GetClassId().ToString().c_str())));

    int bwdCount = 0;
    while (stmtBwd.Step() == BE_SQLITE_ROW)
        ++bwdCount;

    EXPECT_GT(bwdCount, 0) << "Schema-qualified backward query should return results";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, VTable_WithoutSchemaName)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_VTable_NoSchema.ecdb", SchemaItem(s_testSchemaXml)));
    m_ecdb.GetECSqlConfig().SetExperimentalFeaturesEnabled(true);

    auto modelKey = InsertInstance("INSERT INTO ig.Model(Name) VALUES('M1')");
    auto pipe1Key = InsertInstance(SqlPrintfString("INSERT INTO ig.Pipe(Code, Diameter, Model.Id) VALUES('P1', 100.0, %s)",
        modelKey.GetInstanceId().ToString().c_str()));
    auto catKey = InsertInstance("INSERT INTO ig.Category(CatName) VALUES('Cat1')");

    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementInCategory(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe1Key.GetInstanceId().ToString().c_str(), catKey.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    // Query via unqualified name: Relations(...) — no schema prefix
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        SqlPrintfString("SELECT RelatedECInstanceId, RelatedECClassId, Direction, RelationshipECClassId FROM Relations(%s, %s)",
            pipe1Key.GetInstanceId().ToString().c_str(),
            pipe1Key.GetClassId().ToString().c_str())));

    int rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        ++rowCount;

    EXPECT_GT(rowCount, 0) << "Unqualified Relations() should return related instances";

    // Forward direction without schema name
    ECSqlStatement stmtFwd;
    ASSERT_EQ(ECSqlStatus::Success, stmtFwd.Prepare(m_ecdb,
        SqlPrintfString("SELECT RelatedECInstanceId FROM Relations(%s, %s, 'forward')",
            pipe1Key.GetInstanceId().ToString().c_str(),
            pipe1Key.GetClassId().ToString().c_str())));

    int fwdCount = 0;
    while (stmtFwd.Step() == BE_SQLITE_ROW)
        ++fwdCount;

    EXPECT_GT(fwdCount, 0) << "Unqualified forward query should return results";

    // Backward direction without schema name
    ECSqlStatement stmtBwd;
    ASSERT_EQ(ECSqlStatus::Success, stmtBwd.Prepare(m_ecdb,
        SqlPrintfString("SELECT RelatedECInstanceId FROM Relations(%s, %s, 'backward')",
            pipe1Key.GetInstanceId().ToString().c_str(),
            pipe1Key.GetClassId().ToString().c_str())));

    int bwdCount = 0;
    while (stmtBwd.Step() == BE_SQLITE_ROW)
        ++bwdCount;

    EXPECT_GT(bwdCount, 0) << "Unqualified backward query should return results";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, VTable_WithAndWithoutSchemaName_SameResults)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_VTable_Consistency.ecdb", SchemaItem(s_testSchemaXml)));
    m_ecdb.GetECSqlConfig().SetExperimentalFeaturesEnabled(true);

    auto modelKey = InsertInstance("INSERT INTO ig.Model(Name) VALUES('M1')");
    auto pipe1Key = InsertInstance(SqlPrintfString("INSERT INTO ig.Pipe(Code, Diameter, Model.Id) VALUES('P1', 100.0, %s)",
        modelKey.GetInstanceId().ToString().c_str()));
    auto catKey = InsertInstance("INSERT INTO ig.Category(CatName) VALUES('Cat1')");

    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementInCategory(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe1Key.GetInstanceId().ToString().c_str(), catKey.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    // Collect results with schema-qualified name
    ECSqlStatement stmtQualified;
    ASSERT_EQ(ECSqlStatus::Success, stmtQualified.Prepare(m_ecdb,
        SqlPrintfString("SELECT RelatedECInstanceId, RelatedECClassId, Direction, RelationshipECClassId FROM ECVLib.Relations(%s, %s)",
            pipe1Key.GetInstanceId().ToString().c_str(),
            pipe1Key.GetClassId().ToString().c_str())));

    bvector<int64_t> qualifiedIds;
    while (stmtQualified.Step() == BE_SQLITE_ROW)
        qualifiedIds.push_back(stmtQualified.GetValueInt64(0));

    // Collect results with unqualified name
    ECSqlStatement stmtUnqualified;
    ASSERT_EQ(ECSqlStatus::Success, stmtUnqualified.Prepare(m_ecdb,
        SqlPrintfString("SELECT RelatedECInstanceId, RelatedECClassId, Direction, RelationshipECClassId FROM Relations(%s, %s)",
            pipe1Key.GetInstanceId().ToString().c_str(),
            pipe1Key.GetClassId().ToString().c_str())));

    bvector<int64_t> unqualifiedIds;
    while (stmtUnqualified.Step() == BE_SQLITE_ROW)
        unqualifiedIds.push_back(stmtUnqualified.GetValueInt64(0));

    // Both should produce the same results
    ASSERT_EQ(qualifiedIds.size(), unqualifiedIds.size()) << "Qualified and unqualified should return the same number of rows";
    EXPECT_GT(qualifiedIds.size(), 0u) << "Should have results to compare";
    for (size_t i = 0; i < qualifiedIds.size(); ++i)
        EXPECT_EQ(qualifiedIds[i], unqualifiedIds[i]) << "Row " << i << " should match between qualified and unqualified queries";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, VTable_ColumnRefWithAlias)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_VTable_ColRefAlias.ecdb", SchemaItem(s_testSchemaXml)));
    m_ecdb.GetECSqlConfig().SetExperimentalFeaturesEnabled(true);

    auto modelKey = InsertInstance("INSERT INTO ig.Model(Name) VALUES('M1')");
    auto pipe1Key = InsertInstance(SqlPrintfString("INSERT INTO ig.Pipe(Code, Diameter, Model.Id) VALUES('P1', 100.0, %s)",
        modelKey.GetInstanceId().ToString().c_str()));
    auto pipe2Key = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P2', 200.0)");
    auto catKey = InsertInstance("INSERT INTO ig.Category(CatName) VALUES('Cat1')");

    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementInCategory(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe1Key.GetInstanceId().ToString().c_str(), catKey.GetInstanceId().ToString().c_str()));
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe1Key.GetInstanceId().ToString().c_str(), pipe2Key.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    // Use alias 'a' for Pipe and pass a.ECInstanceId, a.ECClassId to Relations()
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "SELECT r.RelatedECInstanceId, r.Direction FROM ig.Pipe a, ECVLib.Relations(a.ECInstanceId, a.ECClassId) r WHERE a.Code = 'P1'"));

    bset<int64_t> foundIds;
    while (stmt.Step() == BE_SQLITE_ROW)
        foundIds.insert(stmt.GetValueInt64(0));

    EXPECT_TRUE(foundIds.find(catKey.GetInstanceId().GetValueUnchecked()) != foundIds.end()) << "Should find Category via a.ECInstanceId column ref";
    EXPECT_TRUE(foundIds.find(pipe2Key.GetInstanceId().GetValueUnchecked()) != foundIds.end()) << "Should find Pipe2 via a.ECInstanceId column ref";
    EXPECT_TRUE(foundIds.find(modelKey.GetInstanceId().GetValueUnchecked()) != foundIds.end()) << "Should find Model (backward via nav prop) with default both direction";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, VTable_ColumnRefWithoutAlias)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_VTable_ColRefNoAlias.ecdb", SchemaItem(s_testSchemaXml)));
    m_ecdb.GetECSqlConfig().SetExperimentalFeaturesEnabled(true);

    auto pipe1Key = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P1', 100.0)");
    auto pipe2Key = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P2', 200.0)");

    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe1Key.GetInstanceId().ToString().c_str(), pipe2Key.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    // Use bare ECInstanceId and ECClassId without table alias
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "SELECT r.RelatedECInstanceId FROM ig.Pipe, ECVLib.Relations(ECInstanceId, ECClassId) r WHERE Code = 'P1'"));

    bset<int64_t> foundIds;
    while (stmt.Step() == BE_SQLITE_ROW)
        foundIds.insert(stmt.GetValueInt64(0));

    EXPECT_TRUE(foundIds.find(pipe2Key.GetInstanceId().GetValueUnchecked()) != foundIds.end()) << "Should resolve bare ECInstanceId from ig.Pipe";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, VTable_DirectionOptionalWithColumnRef)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_VTable_DirOptColRef.ecdb", SchemaItem(s_testSchemaXml)));
    m_ecdb.GetECSqlConfig().SetExperimentalFeaturesEnabled(true);

    auto modelKey = InsertInstance("INSERT INTO ig.Model(Name) VALUES('M1')");
    auto pipe1Key = InsertInstance(SqlPrintfString("INSERT INTO ig.Pipe(Code, Diameter, Model.Id) VALUES('P1', 100.0, %s)",
        modelKey.GetInstanceId().ToString().c_str()));
    auto catKey = InsertInstance("INSERT INTO ig.Category(CatName) VALUES('Cat1')");

    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementInCategory(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe1Key.GetInstanceId().ToString().c_str(), catKey.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    // 2-arg form (no direction) — should default to both
    ECSqlStatement stmtBoth;
    ASSERT_EQ(ECSqlStatus::Success, stmtBoth.Prepare(m_ecdb,
        "SELECT r.RelatedECInstanceId FROM ig.Pipe a, ECVLib.Relations(a.ECInstanceId, a.ECClassId) r WHERE a.Code = 'P1'"));

    bset<int64_t> bothIds;
    while (stmtBoth.Step() == BE_SQLITE_ROW)
        bothIds.insert(stmtBoth.GetValueInt64(0));

    // 3-arg form with 'forward'
    ECSqlStatement stmtFwd;
    ASSERT_EQ(ECSqlStatus::Success, stmtFwd.Prepare(m_ecdb,
        "SELECT r.RelatedECInstanceId FROM ig.Pipe a, ECVLib.Relations(a.ECInstanceId, a.ECClassId, 'forward') r WHERE a.Code = 'P1'"));

    bset<int64_t> fwdIds;
    while (stmtFwd.Step() == BE_SQLITE_ROW)
        fwdIds.insert(stmtFwd.GetValueInt64(0));

    // 3-arg form with 'backward'
    ECSqlStatement stmtBwd;
    ASSERT_EQ(ECSqlStatus::Success, stmtBwd.Prepare(m_ecdb,
        "SELECT r.RelatedECInstanceId FROM ig.Pipe a, ECVLib.Relations(a.ECInstanceId, a.ECClassId, 'backward') r WHERE a.Code = 'P1'"));

    bset<int64_t> bwdIds;
    while (stmtBwd.Step() == BE_SQLITE_ROW)
        bwdIds.insert(stmtBwd.GetValueInt64(0));

    // Forward from Pipe1 should find Category1 (ElementInCategory, Pipe1 is source)
    EXPECT_TRUE(fwdIds.find(catKey.GetInstanceId().GetValueUnchecked()) != fwdIds.end()) << "Forward should find Category";
    // Backward from Pipe1 should find Model (ModelHasElements, Pipe1 is target)
    EXPECT_TRUE(bwdIds.find(modelKey.GetInstanceId().GetValueUnchecked()) != bwdIds.end()) << "Backward should find Model";

    // Both should be a superset of forward and backward
    EXPECT_GE(bothIds.size(), fwdIds.size()) << "Default (both) should have >= forward results";
    EXPECT_GE(bothIds.size(), bwdIds.size()) << "Default (both) should have >= backward results";
    for (auto id : fwdIds)
        EXPECT_TRUE(bothIds.find(id) != bothIds.end()) << "All forward results should be in default (both) results";
    for (auto id : bwdIds)
        EXPECT_TRUE(bothIds.find(id) != bothIds.end()) << "All backward results should be in default (both) results";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, VTable_TwoRelationsChained)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_VTable_TwoRelChain.ecdb", SchemaItem(s_testSchemaXml)));
    m_ecdb.GetECSqlConfig().SetExperimentalFeaturesEnabled(true);

    // Chain: P1 →(forward) P2 →(forward) Cat1
    auto pipe1Key = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P1', 100.0)");
    auto pipe2Key = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P2', 200.0)");
    auto catKey = InsertInstance("INSERT INTO ig.Category(CatName) VALUES('Cat1')");

    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe1Key.GetInstanceId().ToString().c_str(), pipe2Key.GetInstanceId().ToString().c_str()));
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementInCategory(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe2Key.GetInstanceId().ToString().c_str(), catKey.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    // Chain two Relations: first finds P2 from P1 (forward), second finds Cat1 from P2
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "SELECT r1.RelatedECInstanceId, r2.RelatedECInstanceId"
        " FROM ig.Pipe a,"
        "   ECVLib.Relations(a.ECInstanceId, a.ECClassId, 'forward') r1,"
        "   ECVLib.Relations(r1.RelatedECInstanceId, r1.RelatedECClassId, 'forward') r2"
        " WHERE a.Code = 'P1'"));

    bool foundChain = false;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        int64_t hop1Id = stmt.GetValueInt64(0);
        int64_t hop2Id = stmt.GetValueInt64(1);
        if (hop1Id == (int64_t) pipe2Key.GetInstanceId().GetValueUnchecked() &&
            hop2Id == (int64_t) catKey.GetInstanceId().GetValueUnchecked())
            foundChain = true;
        }
    EXPECT_TRUE(foundChain) << "Should find 2-hop chain: P1 → P2 → Cat1 via chained Relations";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, VTable_TwoRelationsIndependent)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_VTable_TwoRelIndep.ecdb", SchemaItem(s_testSchemaXml)));
    m_ecdb.GetECSqlConfig().SetExperimentalFeaturesEnabled(true);

    auto modelKey = InsertInstance("INSERT INTO ig.Model(Name) VALUES('M1')");
    auto pipe1Key = InsertInstance(SqlPrintfString("INSERT INTO ig.Pipe(Code, Diameter, Model.Id) VALUES('P1', 100.0, %s)",
        modelKey.GetInstanceId().ToString().c_str()));
    auto catKey = InsertInstance("INSERT INTO ig.Category(CatName) VALUES('Cat1')");

    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementInCategory(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe1Key.GetInstanceId().ToString().c_str(), catKey.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    // Two independent Relations with different direction filters on the same source
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "SELECT fwd.RelatedECInstanceId, bwd.RelatedECInstanceId"
        " FROM ig.Pipe a,"
        "   ECVLib.Relations(a.ECInstanceId, a.ECClassId, 'forward') fwd,"
        "   ECVLib.Relations(a.ECInstanceId, a.ECClassId, 'backward') bwd"
        " WHERE a.Code = 'P1'"));

    bset<int64_t> fwdIds, bwdIds;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        fwdIds.insert(stmt.GetValueInt64(0));
        bwdIds.insert(stmt.GetValueInt64(1));
        }

    // Forward from Pipe1: Cat1 (ElementInCategory, Pipe1 is source)
    EXPECT_TRUE(fwdIds.find(catKey.GetInstanceId().GetValueUnchecked()) != fwdIds.end()) << "Forward alias should find Category";
    // Backward from Pipe1: Model (ModelHasElements, Pipe1 is target)
    EXPECT_TRUE(bwdIds.find(modelKey.GetInstanceId().GetValueUnchecked()) != bwdIds.end()) << "Backward alias should find Model";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, VTable_ColumnRefWithoutSchemaPrefix)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_VTable_ColRefNoSchema.ecdb", SchemaItem(s_testSchemaXml)));
    m_ecdb.GetECSqlConfig().SetExperimentalFeaturesEnabled(true);

    auto pipe1Key = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P1', 100.0)");
    auto pipe2Key = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P2', 200.0)");

    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe1Key.GetInstanceId().ToString().c_str(), pipe2Key.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    // Unqualified Relations() with alias column refs
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "SELECT r.RelatedECInstanceId FROM ig.Pipe a, Relations(a.ECInstanceId, a.ECClassId, 'forward') r WHERE a.Code = 'P1'"));

    bset<int64_t> foundIds;
    while (stmt.Step() == BE_SQLITE_ROW)
        foundIds.insert(stmt.GetValueInt64(0));

    EXPECT_TRUE(foundIds.find(pipe2Key.GetInstanceId().GetValueUnchecked()) != foundIds.end()) << "Unqualified Relations with column refs should work";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, VTable_FailsWithoutExperimentalFeature)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_VTable_ExpGate.ecdb", SchemaItem(s_testSchemaXml)));

    auto pipe1Key = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P1', 100.0)");
    m_ecdb.SaveChanges();

    // Should fail when experimental features are disabled (default)
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb,
        SqlPrintfString("SELECT RelatedECInstanceId FROM ECVLib.Relations(%s, %s)",
            pipe1Key.GetInstanceId().ToString().c_str(),
            pipe1Key.GetClassId().ToString().c_str())));

    // Should also fail for unqualified name
    ECSqlStatement stmtUnqualified;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmtUnqualified.Prepare(m_ecdb,
        SqlPrintfString("SELECT RelatedECInstanceId FROM Relations(%s, %s)",
            pipe1Key.GetInstanceId().ToString().c_str(),
            pipe1Key.GetClassId().ToString().c_str())));

    // Should succeed with per-query ECSQLOPTIONS
    ECSqlStatement stmtOpt;
    ASSERT_EQ(ECSqlStatus::Success, stmtOpt.Prepare(m_ecdb,
        SqlPrintfString("SELECT RelatedECInstanceId FROM ECVLib.Relations(%s, %s) ECSQLOPTIONS ENABLE_EXPERIMENTAL_FEATURES",
            pipe1Key.GetInstanceId().ToString().c_str(),
            pipe1Key.GetClassId().ToString().c_str())));

    // Should succeed after enabling via pragma
    m_ecdb.GetECSqlConfig().SetExperimentalFeaturesEnabled(true);
    ECSqlStatement stmtEnabled;
    ASSERT_EQ(ECSqlStatus::Success, stmtEnabled.Prepare(m_ecdb,
        SqlPrintfString("SELECT RelatedECInstanceId FROM ECVLib.Relations(%s, %s)",
            pipe1Key.GetInstanceId().ToString().c_str(),
            pipe1Key.GetClassId().ToString().c_str())));
    }

//=======================================================================================
// Regression tests for issues found during code review of the InstanceGraph / relations()
// feature.
//=======================================================================================

//! Schema exercising ShareColumns / overflow mappings, where a navigation property FK ends up
//! in a shared column.
static constexpr Utf8CP s_sharedColumnSchemaXml =
    R"xml(<?xml version="1.0" encoding="utf-8"?>
    <ECSchema schemaName="IGShared" alias="igs" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />

        <ECEntityClass typeName="Model">
            <ECProperty propertyName="Name" typeName="string" />
        </ECEntityClass>

        <ECEntityClass typeName="Element" modifier="Abstract">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
                <ShareColumns xmlns="ECDbMap.02.00.00">
                    <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                    <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="Code" typeName="string" />
        </ECEntityClass>

        <ECEntityClass typeName="Pipe">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="Diameter" typeName="double" />
            <ECProperty propertyName="Material" typeName="string" />
            <ECProperty propertyName="Length" typeName="double" />
            <ECNavigationProperty propertyName="Model" relationshipName="ModelHasElements" direction="Backward" />
        </ECEntityClass>

        <ECRelationshipClass typeName="ModelHasElements" strength="Embedding" modifier="Sealed">
            <Source multiplicity="(0..1)" polymorphic="False" roleLabel="Model">
                <Class class="Model" />
            </Source>
            <Target multiplicity="(0..*)" polymorphic="True" roleLabel="Element">
                <Class class="Pipe" />
            </Target>
        </ECRelationshipClass>
    </ECSchema>)xml";

//---------------------------------------------------------------------------------------
//! A navigation property FK stored in a shared column must still be traversable from the
//! referenced end (Model -> its Elements). This used to silently return zero rows because the
//! generated SQL filtered the FK holder's own ECClassId against the *other* end's hierarchy.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, SharedColumn_NavPropTraversalFromReferencedEnd)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_SharedCol.ecdb", SchemaItem(s_sharedColumnSchemaXml)));

    auto modelKey = InsertInstance("INSERT INTO igs.Model(Name) VALUES('M1')");
    auto pipeKey = InsertInstance(SqlPrintfString("INSERT INTO igs.Pipe(Code, Diameter, Material, Length, Model.Id) VALUES('P1', 1.0, 'steel', 2.0, %s)",
        modelKey.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    // Backward: from Model (referenced end) to the Pipe holding the FK.
    InstanceGraph fromModel(m_ecdb);
    fromModel.AddSeed(modelKey);
    ASSERT_EQ(SUCCESS, fromModel.ExpandNode(modelKey, TraversalDirection::Both));

    auto const* fromModelRelated = fromModel.GetRelated(modelKey);
    ASSERT_NE(nullptr, fromModelRelated);
    ASSERT_EQ(1u, fromModelRelated->size()) << "Model must find its Pipe even though the FK is in a shared column";
    EXPECT_EQ(pipeKey.GetInstanceId(), (*fromModelRelated)[0].GetKey().GetInstanceId());
    EXPECT_EQ(pipeKey.GetClassId(), (*fromModelRelated)[0].GetKey().GetClassId());

    // Forward: from the FK holder back to the Model.
    InstanceGraph fromPipe(m_ecdb);
    fromPipe.AddSeed(pipeKey);
    ASSERT_EQ(SUCCESS, fromPipe.ExpandNode(pipeKey, TraversalDirection::Both));

    auto const* fromPipeRelated = fromPipe.GetRelated(pipeKey);
    ASSERT_NE(nullptr, fromPipeRelated);
    ASSERT_EQ(1u, fromPipeRelated->size());
    EXPECT_EQ(modelKey.GetInstanceId(), (*fromPipeRelated)[0].GetKey().GetInstanceId());
    EXPECT_EQ(modelKey.GetClassId(), (*fromPipeRelated)[0].GetKey().GetClassId());
    }

//---------------------------------------------------------------------------------------
//! A relationship class and its base classes are all applicable to the same seed and all match
//! the very same persisted row. That must produce exactly one edge, not one per class.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, Inheritance_DerivedRelationshipProducesNoDuplicateEdges)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_InheritRelNoDup.ecdb", SchemaItem(s_testSchemaXml)));

    auto pipe1Key = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P1', 100.0)");
    auto pipe2Key = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P2', 200.0)");

    // PipeConnectsToPipe derives from ElementConnectsToElement, both apply to a Pipe seed.
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.PipeConnectsToPipe(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe1Key.GetInstanceId().ToString().c_str(), pipe2Key.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    InstanceGraph graph(m_ecdb);
    graph.AddSeed(pipe1Key);
    ASSERT_EQ(SUCCESS, graph.ExpandNode(pipe1Key, TraversalDirection::Forward));

    auto const* related = graph.GetRelated(pipe1Key);
    ASSERT_NE(nullptr, related);
    ASSERT_EQ(1u, related->size()) << "The single PipeConnectsToPipe row must yield exactly one edge";
    EXPECT_EQ(pipe2Key.GetInstanceId(), (*related)[0].GetKey().GetInstanceId());
    EXPECT_EQ(GetClassId("PipeConnectsToPipe"), (*related)[0].GetRelClassId());
    }

//---------------------------------------------------------------------------------------
//! Expanding the same node twice must replace its edges, not append a second copy of each.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, ExpandNode_TwiceDoesNotDuplicateEdges)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_ExpandTwice.ecdb", SchemaItem(s_testSchemaXml)));

    auto pipe1Key = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P1', 100.0)");
    auto catKey = InsertInstance("INSERT INTO ig.Category(CatName) VALUES('Cat1')");
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementInCategory(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe1Key.GetInstanceId().ToString().c_str(), catKey.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    InstanceGraph graph(m_ecdb);
    graph.AddSeed(pipe1Key);
    ASSERT_EQ(SUCCESS, graph.ExpandNode(pipe1Key, TraversalDirection::Forward));
    size_t const firstCount = graph.GetRelated(pipe1Key)->size();
    ASSERT_EQ(1u, firstCount);

    ASSERT_EQ(SUCCESS, graph.ExpandNode(pipe1Key, TraversalDirection::Forward));
    EXPECT_EQ(firstCount, graph.GetRelated(pipe1Key)->size()) << "Re-expanding must not duplicate edges";
    }

//---------------------------------------------------------------------------------------
//! ExpandNode on a node that was never added as a seed must still make that node part of the
//! graph, otherwise Contains()/NodeCount() and all set operations disagree with GetRelated().
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, ExpandNode_WithoutAddSeedAddsNodeToGraph)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_ExpandNoSeed.ecdb", SchemaItem(s_testSchemaXml)));

    auto pipe1Key = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P1', 100.0)");
    auto catKey = InsertInstance("INSERT INTO ig.Category(CatName) VALUES('Cat1')");
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementInCategory(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe1Key.GetInstanceId().ToString().c_str(), catKey.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    InstanceGraph graph(m_ecdb);
    ASSERT_EQ(SUCCESS, graph.ExpandNode(pipe1Key, TraversalDirection::Forward));

    EXPECT_TRUE(graph.Contains(pipe1Key)) << "An expanded node must be part of the graph";
    EXPECT_TRUE(graph.Contains(catKey));
    EXPECT_EQ(2u, graph.NodeCount());
    EXPECT_NE(nullptr, graph.GetRelated(pipe1Key));
    }

//---------------------------------------------------------------------------------------
//! ExpandAll must continue from nodes that a previous ExpandNode/ExpandAll call already
//! expanded, instead of stopping there.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, ExpandNode_ThenExpandAll)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_ExpandNodeThenAll.ecdb", SchemaItem(s_testSchemaXml)));

    // P1 -> P2 -> P3
    auto p1 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P1', 1.0)");
    auto p2 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P2', 2.0)");
    auto p3 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P3', 3.0)");
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        p1.GetInstanceId().ToString().c_str(), p2.GetInstanceId().ToString().c_str()));
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        p2.GetInstanceId().ToString().c_str(), p3.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    InstanceGraph graph(m_ecdb);
    graph.AddSeed(p1);
    ASSERT_EQ(SUCCESS, graph.ExpandNode(p1, TraversalDirection::Both));
    ASSERT_EQ(SUCCESS, graph.ExpandAll(5));

    EXPECT_TRUE(graph.Contains(p2));
    EXPECT_TRUE(graph.Contains(p3)) << "ExpandAll must continue past the already expanded seed";

    // Deepening an existing graph must work as well.
    InstanceGraph shallow(m_ecdb);
    shallow.AddSeed(p1);
    ASSERT_EQ(SUCCESS, shallow.ExpandAll(1));
    EXPECT_FALSE(shallow.Contains(p3));
    ASSERT_EQ(SUCCESS, shallow.ExpandAll(5));
    EXPECT_TRUE(shallow.Contains(p3)) << "A second ExpandAll must deepen the graph";
    }

//---------------------------------------------------------------------------------------
//! ExpandAll(0) records the seeds without traversing anything.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, ExpandAll_ZeroDepthIsSeedOnly)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_ExpandZero.ecdb", SchemaItem(s_testSchemaXml)));

    auto p1 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P1', 1.0)");
    auto p2 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P2', 2.0)");
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        p1.GetInstanceId().ToString().c_str(), p2.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    InstanceGraph graph(m_ecdb);
    graph.AddSeed(p1);
    ASSERT_EQ(SUCCESS, graph.ExpandAll(0));

    EXPECT_EQ(1u, graph.NodeCount());
    auto const* related = graph.GetRelated(p1);
    ASSERT_NE(nullptr, related) << "The seed must have an (empty) adjacency entry";
    EXPECT_TRUE(related->empty());
    }

//---------------------------------------------------------------------------------------
//! Adding the same seed twice must not traverse it twice.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, AddSeed_TwiceIsIdempotent)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_SeedTwice.ecdb", SchemaItem(s_testSchemaXml)));

    auto p1 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P1', 1.0)");
    auto cat = InsertInstance("INSERT INTO ig.Category(CatName) VALUES('Cat1')");
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementInCategory(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        p1.GetInstanceId().ToString().c_str(), cat.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    InstanceGraph graph(m_ecdb);
    graph.AddSeed(p1);
    graph.AddSeed(p1);
    ASSERT_EQ(SUCCESS, graph.ExpandAll(2));

    ASSERT_NE(nullptr, graph.GetRelated(p1));
    EXPECT_EQ(1u, graph.GetRelated(p1)->size());
    }

//---------------------------------------------------------------------------------------
//! A self loop of a relationship whose source and target constraints overlap is traversable in
//! both directions and is therefore reported once per direction - never more.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, CycleAvoidance_SelfLoopEdgeContents)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_SelfLoopEdges.ecdb", SchemaItem(s_testSchemaXml)));

    auto pipe1Key = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('SelfLoop', 50.0)");
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe1Key.GetInstanceId().ToString().c_str(), pipe1Key.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    InstanceGraph graph(m_ecdb);
    graph.AddSeed(pipe1Key);
    ASSERT_EQ(SUCCESS, graph.ExpandNode(pipe1Key, TraversalDirection::Both));

    auto const* related = graph.GetRelated(pipe1Key);
    ASSERT_NE(nullptr, related);
    ASSERT_EQ(2u, related->size()) << "A self loop is reported once as Forward and once as Backward";

    bool forward = false, backward = false;
    for (auto const& rel : *related)
        {
        EXPECT_EQ(pipe1Key, rel.GetKey());
        EXPECT_EQ(GetClassId("ElementConnectsToElement"), rel.GetRelClassId());
        forward |= (rel.GetDirection() == TraversalDirection::Forward);
        backward |= (rel.GetDirection() == TraversalDirection::Backward);
        }
    EXPECT_TRUE(forward);
    EXPECT_TRUE(backward);

    // Traversing only one direction reports it exactly once.
    InstanceGraph fwdOnly(m_ecdb);
    fwdOnly.AddSeed(pipe1Key);
    ASSERT_EQ(SUCCESS, fwdOnly.ExpandNode(pipe1Key, TraversalDirection::Forward));
    ASSERT_NE(nullptr, fwdOnly.GetRelated(pipe1Key));
    EXPECT_EQ(1u, fwdOnly.GetRelated(pipe1Key)->size());
    }

//---------------------------------------------------------------------------------------
//! Union and Intersection must not duplicate the edges the two graphs have in common.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, SetOps_EdgesAreDeduplicated)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_SetOpsDedup.ecdb", SchemaItem(s_testSchemaXml)));

    auto p1 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P1', 1.0)");
    auto cat = InsertInstance("INSERT INTO ig.Category(CatName) VALUES('Cat1')");
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementInCategory(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        p1.GetInstanceId().ToString().c_str(), cat.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    InstanceGraph a(m_ecdb);
    a.AddSeed(p1);
    ASSERT_EQ(SUCCESS, a.ExpandAll(2));

    InstanceGraph b(m_ecdb);
    b.AddSeed(p1);
    ASSERT_EQ(SUCCESS, b.ExpandAll(2));

    auto unionGraph = InstanceGraph::Union(a, b);
    ASSERT_NE(nullptr, unionGraph);
    ASSERT_NE(nullptr, unionGraph->GetRelated(p1));
    EXPECT_EQ(a.GetRelated(p1)->size(), unionGraph->GetRelated(p1)->size()) << "Union must not duplicate common edges";

    auto intersectionGraph = InstanceGraph::Intersection(a, b);
    ASSERT_NE(nullptr, intersectionGraph);
    ASSERT_NE(nullptr, intersectionGraph->GetRelated(p1));
    EXPECT_EQ(a.GetRelated(p1)->size(), intersectionGraph->GetRelated(p1)->size()) << "Intersection must not duplicate common edges";
    }

//---------------------------------------------------------------------------------------
//! Instance keys of different files are not comparable, so set operations must refuse graphs
//! that belong to different ECDbs.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, SetOps_DifferentECDbsAreRejected)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_SetOpsDbA.ecdb", SchemaItem(s_testSchemaXml)));
    auto p1 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P1', 1.0)");
    m_ecdb.SaveChanges();

    ECDb other;
    ASSERT_EQ(BE_SQLITE_OK, other.OpenBeSQLiteDb(m_ecdb.GetDbFileName(), ECDb::OpenParams(ECDb::OpenMode::Readonly)));

    InstanceGraph a(m_ecdb);
    a.AddSeed(p1);
    ASSERT_EQ(SUCCESS, a.ExpandAll(1));

    InstanceGraph b(other);
    b.AddSeed(p1);
    ASSERT_EQ(SUCCESS, b.ExpandAll(1));

    ScopedDisableFailOnAssertion disableFailOnAssertion;
    EXPECT_FALSE(InstanceGraph::Overlaps(a, b));
    EXPECT_EQ(nullptr, InstanceGraph::Intersection(a, b));
    EXPECT_EQ(nullptr, InstanceGraph::Union(a, b));
    }

//---------------------------------------------------------------------------------------
//! The virtual table arguments must be decoded correctly no matter in which order SQLite hands
//! the constraints over. SQLite lists explicit WHERE terms before table-valued-function
//! arguments, so a WHERE clause on a hidden column used to shift every argument.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, VTable_WhereClauseConstraintOrdering)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_VTableWhereOrder.ecdb", SchemaItem(s_testSchemaXml)));

    auto modelKey = InsertInstance("INSERT INTO ig.Model(Name) VALUES('M1')");
    auto pipe1Key = InsertInstance(SqlPrintfString("INSERT INTO ig.Pipe(Code, Diameter, Model.Id) VALUES('P1', 100.0, %s)",
        modelKey.GetInstanceId().ToString().c_str()));
    auto catKey = InsertInstance("INSERT INTO ig.Category(CatName) VALUES('Cat1')");
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementInCategory(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe1Key.GetInstanceId().ToString().c_str(), catKey.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    // The hidden argument columns are only addressable through raw SQLite. This is exactly the
    // path where SQLite reports the WHERE constraints before the table valued function
    // arguments, so it is the one that exposes an argv/column order mismatch.
    uint64_t const instanceId = pipe1Key.GetInstanceId().GetValueUnchecked();
    uint64_t const classId = pipe1Key.GetClassId().GetValueUnchecked();

    auto collect = [&] (Utf8CP sql, bvector<uint64_t>& relatedIds)
        {
        relatedIds.clear();
        Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, sql)) << sql;
        DbResult stepStatus;
        while ((stepStatus = stmt.Step()) == BE_SQLITE_ROW)
            relatedIds.push_back((uint64_t) stmt.GetValueInt64(0));
        ASSERT_EQ(BE_SQLITE_DONE, stepStatus) << sql;
        std::sort(relatedIds.begin(), relatedIds.end());
        };

    // Reference: arguments supplied positionally.
    bvector<uint64_t> expectedForward;
    collect(SqlPrintfString("SELECT RelatedECInstanceId FROM relations(%" PRIu64 ",%" PRIu64 ",'forward')", instanceId, classId), expectedForward);
    ASSERT_EQ(1u, expectedForward.size()) << "Forward from P1 must find exactly Cat1";
    EXPECT_EQ(catKey.GetInstanceId().GetValueUnchecked(), expectedForward[0]);

    // The very same query, but the direction arrives through an explicit WHERE clause, which
    // SQLite lists *before* the two table valued function arguments.
    bvector<uint64_t> actual;
    collect(SqlPrintfString("SELECT RelatedECInstanceId FROM relations(%" PRIu64 ",%" PRIu64 ") WHERE TraversalDirection='forward'", instanceId, classId), actual);
    EXPECT_EQ(expectedForward, actual);

    // All three arguments through the WHERE clause, in an order different from the column order.
    collect(SqlPrintfString("SELECT RelatedECInstanceId FROM relations WHERE TraversalDirection='forward' AND ECInstanceId=%" PRIu64 " AND ECClassId=%" PRIu64,
        instanceId, classId), actual);
    EXPECT_EQ(expectedForward, actual);

    collect(SqlPrintfString("SELECT RelatedECInstanceId FROM relations WHERE ECClassId=%" PRIu64 " AND TraversalDirection='forward' AND ECInstanceId=%" PRIu64,
        classId, instanceId), actual);
    EXPECT_EQ(expectedForward, actual);

    // Backward must still be backward when supplied through the WHERE clause.
    bvector<uint64_t> backward;
    collect(SqlPrintfString("SELECT RelatedECInstanceId FROM relations(%" PRIu64 ",%" PRIu64 ") WHERE TraversalDirection='backward'", instanceId, classId), backward);
    ASSERT_EQ(1u, backward.size()) << "Backward from P1 must find exactly its Model";
    EXPECT_EQ(modelKey.GetInstanceId().GetValueUnchecked(), backward[0]);
    }

//---------------------------------------------------------------------------------------
//! An unrecognised TraversalDirection must be reported instead of silently degrading to 'both'.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, VTable_InvalidDirectionIsRejected)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_VTableBadDir.ecdb", SchemaItem(s_testSchemaXml)));
    m_ecdb.GetECSqlConfig().SetExperimentalFeaturesEnabled(true);

    auto modelKey = InsertInstance("INSERT INTO ig.Model(Name) VALUES('M1')");
    auto pipe1Key = InsertInstance(SqlPrintfString("INSERT INTO ig.Pipe(Code, Diameter, Model.Id) VALUES('P1', 100.0, %s)",
        modelKey.GetInstanceId().ToString().c_str()));
    auto catKey = InsertInstance("INSERT INTO ig.Category(CatName) VALUES('Cat1')");
    // P1 has a forward edge (to Cat1) and a backward edge (to M1), so every valid direction
    // produces at least one row.
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementInCategory(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipe1Key.GetInstanceId().ToString().c_str(), catKey.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    Utf8String instanceId = pipe1Key.GetInstanceId().ToString();
    Utf8String classId = pipe1Key.GetClassId().ToString();

    for (Utf8CP badDirection : {"sideways", "fwd", "", "both "})
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
            SqlPrintfString("SELECT RelatedECInstanceId FROM ECVLib.Relations(%s, %s, '%s')",
                instanceId.c_str(), classId.c_str(), badDirection)));
        EXPECT_NE(BE_SQLITE_ROW, stmt.Step()) << "Direction '" << badDirection << "' must not be accepted";
        }

    // 'both' and the case-insensitive spellings are valid.
    for (Utf8CP goodDirection : {"both", "BOTH", "Forward", "BACKWARD"})
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
            SqlPrintfString("SELECT RelatedECInstanceId FROM ECVLib.Relations(%s, %s, '%s')",
                instanceId.c_str(), classId.c_str(), goodDirection)));
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Direction '" << goodDirection << "' must be accepted";
        }

    // A NULL direction means 'both'.
    ECSqlStatement nullDirStmt;
    ASSERT_EQ(ECSqlStatus::Success, nullDirStmt.Prepare(m_ecdb,
        SqlPrintfString("SELECT RelatedECInstanceId FROM ECVLib.Relations(%s, %s, NULL)",
            instanceId.c_str(), classId.c_str())));
    EXPECT_EQ(BE_SQLITE_ROW, nullDirStmt.Step());
    }

//---------------------------------------------------------------------------------------
//! Relations() without its mandatory arguments must be diagnosed rather than silently
//! returning an empty result, which is indistinguishable from "no relationships".
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, VTable_MissingRequiredArgumentsAreRejected)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_VTableMissingArgs.ecdb", SchemaItem(s_testSchemaXml)));

    auto pipe1Key = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P1', 100.0)");
    m_ecdb.SaveChanges();

    // BestIndex rejects any plan that does not provide both required arguments, so SQLite
    // reports an error instead of the vtable silently returning an empty result (which is
    // indistinguishable from "this instance has no relationships").
    Statement noArgs;
    EXPECT_NE(BE_SQLITE_OK, noArgs.TryPrepare(m_ecdb, "SELECT RelatedECInstanceId FROM relations"))
        << "relations without arguments must be rejected";

    Statement oneArg;
    EXPECT_NE(BE_SQLITE_OK, oneArg.TryPrepare(m_ecdb,
        SqlPrintfString("SELECT RelatedECInstanceId FROM relations(%" PRIu64 ")", pipe1Key.GetInstanceId().GetValueUnchecked())))
        << "relations with a single argument must be rejected";

    Statement instanceIdOnly;
    EXPECT_NE(BE_SQLITE_OK, instanceIdOnly.TryPrepare(m_ecdb,
        SqlPrintfString("SELECT RelatedECInstanceId FROM relations WHERE ECInstanceId=%" PRIu64, pipe1Key.GetInstanceId().GetValueUnchecked())))
        << "relations without an ECClassId must be rejected";

    // Too many arguments is diagnosed by SQLite itself.
    Statement tooManyArgs;
    EXPECT_NE(BE_SQLITE_OK, tooManyArgs.TryPrepare(m_ecdb, "SELECT RelatedECInstanceId FROM relations(1,2,'forward',4)"))
        << "relations with too many arguments must be rejected";
    }

//---------------------------------------------------------------------------------------
//! A syntactically valid but non existing seed yields an empty result, not an error.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, VTable_UnknownSeedReturnsEmpty)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_VTableUnknownSeed.ecdb", SchemaItem(s_testSchemaXml)));
    m_ecdb.GetECSqlConfig().SetExperimentalFeaturesEnabled(true);

    auto pipe1Key = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P1', 100.0)");
    m_ecdb.SaveChanges();

    // Unknown instance id, valid class id.
    ECSqlStatement unknownInstance;
    ASSERT_EQ(ECSqlStatus::Success, unknownInstance.Prepare(m_ecdb,
        SqlPrintfString("SELECT RelatedECInstanceId FROM ECVLib.Relations(999999, %s)", pipe1Key.GetClassId().ToString().c_str())));
    EXPECT_EQ(BE_SQLITE_DONE, unknownInstance.Step());

    // Unknown class id, valid instance id.
    ECSqlStatement unknownClass;
    ASSERT_EQ(ECSqlStatus::Success, unknownClass.Prepare(m_ecdb,
        SqlPrintfString("SELECT RelatedECInstanceId FROM ECVLib.Relations(%s, 999999)", pipe1Key.GetInstanceId().ToString().c_str())));
    EXPECT_EQ(BE_SQLITE_DONE, unknownClass.Step());

    // NULL arguments (only expressible through raw SQLite).
    Statement nullArgs;
    ASSERT_EQ(BE_SQLITE_OK, nullArgs.Prepare(m_ecdb, "SELECT RelatedECInstanceId FROM relations(NULL,NULL)"));
    EXPECT_EQ(BE_SQLITE_DONE, nullArgs.Step());
    }

//---------------------------------------------------------------------------------------
//! The graph statement cache is keyed on schema derived data and must be dropped when the
//! schemas change.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, CacheIsInvalidatedOnSchemaImport)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_CacheInvalidation.ecdb", SchemaItem(s_testSchemaXml)));

    auto p1 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P1', 1.0)");
    auto p2 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P2', 2.0)");
    m_ecdb.SaveChanges();

    // Populate the cache while there is no relationship between P1 and P2.
    InstanceGraph before(m_ecdb);
    before.AddSeed(p1);
    ASSERT_EQ(SUCCESS, before.ExpandAll(2));
    EXPECT_FALSE(before.Contains(p2));

    // Import an additional schema introducing a new relationship, then use it.
    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="IGTest2" alias="ig2" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="IGTest" version="01.00.00" alias="ig" />
            <ECRelationshipClass typeName="ElementFeedsElement" strength="Referencing" modifier="Sealed">
                <Source multiplicity="(0..*)" polymorphic="True" roleLabel="feeds">
                    <Class class="ig:Element" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="fed by">
                    <Class class="ig:Element" />
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml")));

    InsertRelInstance(SqlPrintfString("INSERT INTO ig2.ElementFeedsElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        p1.GetInstanceId().ToString().c_str(), p2.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    InstanceGraph after(m_ecdb);
    after.AddSeed(p1);
    ASSERT_EQ(SUCCESS, after.ExpandAll(2));
    EXPECT_TRUE(after.Contains(p2)) << "The relationship added by the schema import must be discovered";
    }

//---------------------------------------------------------------------------------------
//! Two distinct relationship rows between the same pair of instances are two distinct edges.
//! Without the relationship ECInstanceId in the edge identity they collapsed into one and the
//! second row was silently lost.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, RelationshipInstanceId_DistinguishesDuplicateLinkTableRows)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_DupRelRows.ecdb", SchemaItem(s_testSchemaXml)));
    m_ecdb.GetECSqlConfig().SetExperimentalFeaturesEnabled(true);

    auto p1 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P1', 1.0)");
    auto p2 = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P2', 2.0)");

    // ElementConnectsToElement is a TablePerHierarchy link table, so duplicate rows between the
    // same pair of instances are allowed.
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        p1.GetInstanceId().ToString().c_str(), p2.GetInstanceId().ToString().c_str()));
    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        p1.GetInstanceId().ToString().c_str(), p2.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    InstanceGraph graph(m_ecdb);
    graph.AddSeed(p1);
    ASSERT_EQ(SUCCESS, graph.ExpandNode(p1, TraversalDirection::Forward));

    auto const* related = graph.GetRelated(p1);
    ASSERT_NE(nullptr, related);
    ASSERT_EQ(2u, related->size()) << "Both persisted relationship rows must be reported";

    bset<uint64_t> relInstanceIds;
    for (auto const& rel : *related)
        {
        EXPECT_EQ(p2.GetInstanceId(), rel.GetKey().GetInstanceId());
        EXPECT_EQ(GetClassId("ElementConnectsToElement"), rel.GetRelClassId());
        ASSERT_TRUE(rel.GetRelInstanceId().IsValid()) << "The relationship ECInstanceId must be resolved for a link table";
        relInstanceIds.insert(rel.GetRelInstanceId().GetValueUnchecked());
        }

    EXPECT_EQ(2u, relInstanceIds.size()) << "The two edges must carry different relationship ECInstanceIds";

    // The same must be observable through the virtual table.
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        SqlPrintfString("SELECT RelationshipECInstanceId FROM ECVLib.Relations(%s, %s, 'forward') ORDER BY RelationshipECInstanceId",
            p1.GetInstanceId().ToString().c_str(), p1.GetClassId().ToString().c_str())));

    bset<uint64_t> queriedIds;
    while (stmt.Step() == BE_SQLITE_ROW)
        queriedIds.insert(stmt.GetValueId<ECInstanceId>(0).GetValueUnchecked());

    EXPECT_EQ(relInstanceIds, queriedIds);
    }

//---------------------------------------------------------------------------------------
//! For a foreign key relationship the relationship instance is identified by the ECInstanceId of
//! the row holding the navigation property, in both traversal directions.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, RelationshipInstanceId_NavPropIsForeignKeyHolderId)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_NavPropRelId.ecdb", SchemaItem(s_testSchemaXml)));

    auto modelKey = InsertInstance("INSERT INTO ig.Model(Name) VALUES('M1')");
    auto pipeKey = InsertInstance(SqlPrintfString("INSERT INTO ig.Pipe(Code, Diameter, Model.Id) VALUES('P1', 1.0, %s)",
        modelKey.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    InstanceGraph fromPipe(m_ecdb);
    fromPipe.AddSeed(pipeKey);
    ASSERT_EQ(SUCCESS, fromPipe.ExpandNode(pipeKey, TraversalDirection::Both));
    auto const* fromPipeRelated = fromPipe.GetRelated(pipeKey);
    ASSERT_NE(nullptr, fromPipeRelated);
    ASSERT_EQ(1u, fromPipeRelated->size());
    EXPECT_EQ(pipeKey.GetInstanceId(), (*fromPipeRelated)[0].GetRelInstanceId());

    InstanceGraph fromModel(m_ecdb);
    fromModel.AddSeed(modelKey);
    ASSERT_EQ(SUCCESS, fromModel.ExpandNode(modelKey, TraversalDirection::Both));
    auto const* fromModelRelated = fromModel.GetRelated(modelKey);
    ASSERT_NE(nullptr, fromModelRelated);
    ASSERT_EQ(1u, fromModelRelated->size());
    EXPECT_EQ(pipeKey.GetInstanceId(), (*fromModelRelated)[0].GetRelInstanceId());
    }

//---------------------------------------------------------------------------------------
//! An instance whose id collides with the seed's id but which lives in a different table must
//! not be picked up. ECInstanceIds are only unique per table, so the seed's own ECClassId has to
//! take part in the lookup.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, CollidingInstanceIdsInDifferentTablesAreNotConfused)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_IdCollision.ecdb", SchemaItem(s_testSchemaXml)));

    // Model and Element map to different tables, so their ECInstanceIds are independent
    // sequences and are very likely to collide.
    auto modelKey = InsertInstance("INSERT INTO ig.Model(Name) VALUES('M1')");
    auto pipeKey = InsertInstance("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P1', 1.0)");
    auto catKey = InsertInstance("INSERT INTO ig.Category(CatName) VALUES('Cat1')");

    InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementInCategory(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
        pipeKey.GetInstanceId().ToString().c_str(), catKey.GetInstanceId().ToString().c_str()));
    m_ecdb.SaveChanges();

    // Seeding with the Model - which is not a valid source of ElementInCategory at all - must
    // not return the Category even when the Model's id happens to equal the Pipe's id.
    InstanceGraph fromModel(m_ecdb);
    fromModel.AddSeed(modelKey);
    ASSERT_EQ(SUCCESS, fromModel.ExpandNode(modelKey, TraversalDirection::Both));
    auto const* related = fromModel.GetRelated(modelKey);
    ASSERT_NE(nullptr, related);
    for (auto const& rel : *related)
        EXPECT_NE(catKey, rel.GetKey()) << "The Category must only be reachable from the Pipe";
    }

//---------------------------------------------------------------------------------------
//! ExpandAll must terminate at the maximum depth even when the graph is fully cyclic.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceGraphTests, ExpandAll_MaxDepthTerminatesOnCyclicGraph)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("IG_MaxDepthCycle.ecdb", SchemaItem(s_testSchemaXml)));

    bvector<ECInstanceKey> pipes;
    for (int i = 0; i < 12; ++i)
        pipes.push_back(InsertInstance(SqlPrintfString("INSERT INTO ig.Pipe(Code, Diameter) VALUES('P%d', 1.0)", i)));

    // Fully connected ring, traversable in both directions.
    for (size_t i = 0; i < pipes.size(); ++i)
        {
        ECInstanceKeyCR from = pipes[i];
        ECInstanceKeyCR to = pipes[(i + 1) % pipes.size()];
        InsertRelInstance(SqlPrintfString("INSERT INTO ig.ElementConnectsToElement(SourceECInstanceId, TargetECInstanceId) VALUES(%s, %s)",
            from.GetInstanceId().ToString().c_str(), to.GetInstanceId().ToString().c_str()));
        }
    m_ecdb.SaveChanges();

    InstanceGraph graph(m_ecdb);
    graph.AddSeed(pipes[0]);
    ASSERT_EQ(SUCCESS, graph.ExpandAll(UINT8_MAX)) << "A cyclic graph must not make ExpandAll run forever";
    EXPECT_EQ(pipes.size(), graph.NodeCount());

    for (auto const& pipe : pipes)
        EXPECT_TRUE(graph.Contains(pipe));
    }

END_ECDBUNITTESTS_NAMESPACE
