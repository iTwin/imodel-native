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

END_ECDBUNITTESTS_NAMESPACE
