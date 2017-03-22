/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECRelationshipClassInheritanceTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "SchemaImportTestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================    
// @bsiclass                                   Krischan.Eberle                  06/16
//=======================================================================================    
struct ECRelationshipInheritanceTestFixture : DbMappingTestFixture
    {};


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECRelationshipInheritanceTestFixture, BasicCRUD)
    {
    Utf8String ecdbFilePath;
    {
    SchemaItem testSchema("<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                          "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                          "  <ECEntityClass typeName='Model' >"
                          "    <ECProperty propertyName='Name' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='Element' modifier='Abstract' >"
                          "    <ECCustomAttributes>"
                          "         <ClassMap xmlns='ECDbMap.02.00'>"
                          "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                          "        </ClassMap>"
                          "    </ECCustomAttributes>"
                          "    <ECProperty propertyName='Code' typeName='string' />"
                          "    <ECNavigationProperty propertyName='Model' relationshipName='ModelHasElements' direction='Backward' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='InformationElement' modifier='Abstract'>"
                          "     <BaseClass>Element</BaseClass>"
                          "     <ECCustomAttributes>"
                          "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                          "     </ECCustomAttributes>"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='LinkElement' modifier='Abstract'>"
                          "     <BaseClass>InformationElement</BaseClass>"
                          "     <ECCustomAttributes>"
                          "         <ShareColumns xmlns='ECDbMap.02.00'>"
                          "              <SharedColumnCount>8</SharedColumnCount>"
                          "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                          "         </ShareColumns>"
                          "     </ECCustomAttributes>"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='UrlLink' modifier='Sealed'>"
                          "     <BaseClass>LinkElement</BaseClass>"
                          "    <ECProperty propertyName='Url' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='EmbeddedLink' modifier='Sealed'>"
                          "     <BaseClass>LinkElement</BaseClass>"
                          "    <ECProperty propertyName='Name' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='GeometricElement' modifier='Abstract'>"
                          "     <ECCustomAttributes>"
                          "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                          "     </ECCustomAttributes>"
                          "    <BaseClass>Element</BaseClass>"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='Geometric3dElement' modifier='Abstract'>"
                          "     <ECCustomAttributes>"
                          "         <ShareColumns xmlns='ECDbMap.02.00'>"
                          "              <SharedColumnCount>16</SharedColumnCount>"
                          "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                          "         </ShareColumns>"
                          "     </ECCustomAttributes>"
                          "    <BaseClass>GeometricElement</BaseClass>"
                          "    <ECProperty propertyName='Geometry' typeName='Bentley.Geometry.Common.IGeometry' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='VolumeElement'>"
                          "    <BaseClass>Geometric3dElement</BaseClass>"
                          "    <ECProperty propertyName='Name' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='Annotation3dElement'>"
                          "    <BaseClass>Geometric3dElement</BaseClass>"
                          "    <ECProperty propertyName='Font' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECRelationshipClass typeName='ModelHasElements' modifier='Abstract' strength='embedding'>"
                          "    <Source multiplicity='(1..1)' polymorphic='True' roleLabel='Model Has Elements'>"
                          "      <Class class='Model' />"
                          "    </Source>"
                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Elements (Reversed)'>"
                          "      <Class class='Element' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "  <ECRelationshipClass typeName='ModelHasGeometric3dElements' strength='embedding' modifier='Sealed'>"
                          "   <BaseClass>ModelHasElements</BaseClass>"
                          "    <Source multiplicity='(1..1)' polymorphic='True' roleLabel='Model Has Geometric 3d Elements'>"
                          "      <Class class='Model' />"
                          "    </Source>"
                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Geometric 3d Elements (Reversed)'>"
                          "      <Class class='Geometric3dElement' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "  <ECRelationshipClass typeName='ModelHasLinkElements' strength='embedding' modifier='Sealed'>"
                          "   <BaseClass>ModelHasElements</BaseClass>"
                          "    <Source multiplicity='(1..1)' polymorphic='True' roleLabel='Model Has Link Elements'>"
                          "      <Class class='Model' />"
                          "    </Source>"
                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Link Elements (Reversed)'>"
                          "      <Class class='LinkElement' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "  <ECRelationshipClass typeName='ElementDrivesElement' strength='referencing' modifier='Abstract'>"
                          "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='Element Drives Element'>"
                          "      <Class class='Element' />"
                          "    </Source>"
                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Element Drives Element (Reversed)'>"
                          "      <Class class='Element' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "  <ECRelationshipClass typeName='InformationElementDrivesInformationElement' strength='referencing' modifier='Sealed'>"
                          "   <BaseClass>ElementDrivesElement</BaseClass>"
                          "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='Information Element Drives Information Element'>"
                          "      <Class class='InformationElement' />"
                          "    </Source>"
                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Information Element Drives Information Element (Reversed)'>"
                          "      <Class class='InformationElement' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "  <ECRelationshipClass typeName='UrlLinkDrivesAnnotation3dElement' strength='referencing' modifier='Sealed'>"
                          "   <BaseClass>ElementDrivesElement</BaseClass>"
                          "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='Url Link Drives Annotation 3d Element'>"
                          "      <Class class='UrlLink' />"
                          "    </Source>"
                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Url Link Drives Annotation 3d Element (Reversed)'>"
                          "      <Class class='Annotation3dElement' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "</ECSchema>");
    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testSchema, "ecdbrelationshipinheritance.ecdb");
    ASSERT_FALSE(asserted);

    ecdbFilePath.assign(ecdb.GetDbFileName());
    }

    ECClassId modelHasGeom3dElementsRelClassId, modelHasLinkElementsRelClassId;

    std::vector<ECInstanceKey> modelKeys;
    std::vector<ECInstanceKey> urlLinkKeys;
    std::vector<ECInstanceKey> embeddedLinkKeys;
    std::vector<ECInstanceKey> volumeElKeys;
    std::vector<ECInstanceKey> annotationElKeys;

    {
    //insert test data
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbFilePath.c_str(), ECDb::OpenParams(Db::OpenMode::ReadWrite)));

    modelHasGeom3dElementsRelClassId = ecdb.Schemas().GetClassId("TestSchema", "ModelHasGeometric3dElements");
    ASSERT_TRUE(modelHasGeom3dElementsRelClassId.IsValid());
    modelHasLinkElementsRelClassId = ecdb.Schemas().GetClassId("TestSchema", "ModelHasLinkElements");
    ASSERT_TRUE(modelHasLinkElementsRelClassId.IsValid());

    ECSqlStatement stmt;

    //Model
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.Model(Name) VALUES(?)"));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Model1", IECSqlBinder::MakeCopy::No));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    modelKeys.push_back(key);
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Model2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    modelKeys.push_back(key);
    stmt.Finalize();

    //VolumeElement
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.VolumeElement(Code, Model.Id, Model.RelECClassId, Name) VALUES(?,?,?, 'Volume')"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "VolumeElement 1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[0].GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, modelHasGeom3dElementsRelClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    volumeElKeys.push_back(key);
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "VolumeElement 2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[1].GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, modelHasGeom3dElementsRelClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    volumeElKeys.push_back(key);
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "VolumeElement 3", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[1].GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, modelHasGeom3dElementsRelClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    volumeElKeys.push_back(key);
    stmt.Finalize();

    //AnnotationElement
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.Annotation3dElement(Code, Model.Id, Model.RelECClassId, Font) VALUES(?,?,?, 'Consolas Sans Serif')"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Annotation3dElement 1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[0].GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, modelHasGeom3dElementsRelClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    annotationElKeys.push_back(key);
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Annotation3dElement 2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[1].GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, modelHasGeom3dElementsRelClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    annotationElKeys.push_back(key);
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Annotation3dElement 3", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[1].GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, modelHasGeom3dElementsRelClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    annotationElKeys.push_back(key);
    stmt.Finalize();

    //UrlLink
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.UrlLink(Code, Model.Id,Model.RelECClassId, Url) VALUES(?,?,?, 'http://www.staufen.de')"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "UrlLinkElement 1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[0].GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, modelHasLinkElementsRelClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    urlLinkKeys.push_back(key);
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "UrlLinkElement 2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[0].GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, modelHasLinkElementsRelClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    urlLinkKeys.push_back(key);
    stmt.Finalize();

    //EmbeddedLink
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.EmbeddedLink(Code, Model.Id, Model.RelECClassId, Name) VALUES(?,?,?, 'bliblablub')"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "EmbeddedLinkElement 1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[1].GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, modelHasLinkElementsRelClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    embeddedLinkKeys.push_back(key);
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "EmbeddedLinkElement 2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[1].GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, modelHasLinkElementsRelClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    embeddedLinkKeys.push_back(key);
    stmt.Finalize();

    //InformationElementDrivesInformationElement
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.InformationElementDrivesInformationElement(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, urlLinkKeys[0].GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, urlLinkKeys[0].GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, embeddedLinkKeys[0].GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, embeddedLinkKeys[0].GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, urlLinkKeys[0].GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, urlLinkKeys[0].GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, embeddedLinkKeys[1].GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, embeddedLinkKeys[1].GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << ecdb.GetLastError().c_str();
    stmt.Finalize();

    //UrlLinkDrivesAnnotationElement
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.UrlLinkDrivesAnnotation3dElement(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, urlLinkKeys[1].GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, urlLinkKeys[1].GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, annotationElKeys[0].GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, annotationElKeys[0].GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, urlLinkKeys[1].GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, urlLinkKeys[1].GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, annotationElKeys[1].GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, annotationElKeys[1].GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, urlLinkKeys[1].GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, urlLinkKeys[1].GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, annotationElKeys[2].GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, annotationElKeys[2].GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << ecdb.GetLastError().c_str();
    stmt.Finalize();
    }

    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbFilePath.c_str(), ECDb::OpenParams(Db::OpenMode::Readonly)));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT COUNT(*) FROM ts.ModelHasElements"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(10, stmt.GetValueInt(0)) << stmt.GetECSql();
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId, ECClassId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ONLY ts.ModelHasElements"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "non-polymorphic SELECT against abstract rel class should return 0 rows";
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT COUNT(*) FROM ts.ModelHasGeometric3dElements"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(6, stmt.GetValueInt(0)) << stmt.GetECSql();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT COUNT(*) FROM ts.ModelHasLinkElements"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(4, stmt.GetValueInt(0)) << stmt.GetECSql();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT count(*) FROM ts.ElementDrivesElement"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(5, stmt.GetValueInt(0)) << stmt.GetECSql();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT count(*) FROM ONLY ts.ElementDrivesElement"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(0, stmt.GetValueInt(0)) << stmt.GetECSql();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT count(*) FROM ts.InformationElementDrivesInformationElement"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(2, stmt.GetValueInt(0)) << stmt.GetECSql();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT count(*) FROM ts.UrlLinkDrivesAnnotation3dElement"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(3, stmt.GetValueInt(0)) << stmt.GetECSql();
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECRelationshipInheritanceTestFixture, ValidCases)
    {
            {
            ECDb ecdb;
            bool asserted = false;
            AssertSchemaImport(ecdb, asserted,
                               SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                          "<ECSchema schemaName='Test' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                          "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                          "  <ECEntityClass typeName='Model' >"
                                          "    <ECProperty propertyName='Name' typeName='string' />"
                                          "  </ECEntityClass>"
                                          "  <ECEntityClass typeName='Element' modifier='Abstract' >"
                                          "    <ECCustomAttributes>"
                                          "         <ClassMap xmlns='ECDbMap.02.00'>"
                                          "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                          "        </ClassMap>"
                                          "    </ECCustomAttributes>"
                                          "    <ECProperty propertyName='Code' typeName='string' />"
                                          "    <ECNavigationProperty propertyName='ModelId' relationshipName='ModelHasElements' direction='Backward' />"
                                          "  </ECEntityClass>"
                                          "  <ECEntityClass typeName='PhysicalElement'>"
                                          "    <BaseClass>Element</BaseClass>"
                                          "    <ECProperty propertyName='Geometry' typeName='Bentley.Geometry.Common.IGeometry' />"
                                          "  </ECEntityClass>"
                                          "  <ECRelationshipClass typeName='ModelHasElements' modifier='Abstract' strength='embedding'>"
                                          "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Elements'>"
                                          "      <Class class='Model' />"
                                          "    </Source>"
                                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Elements (Reversed)'>"
                                          "      <Class class='Element' />"
                                          "    </Target>"
                                          "  </ECRelationshipClass>"
                                          "  <ECRelationshipClass typeName='ModelHasPhysicalElements' strength='embedding' modifier='Sealed'>"
                                          "    <ECCustomAttributes>"
                                          "        <ClassMap xmlns='ECDbMap.02.00'>"
                                          "                <MapStrategy>NotMapped</MapStrategy>"
                                          "        </ClassMap>"
                                          "    </ECCustomAttributes>"
                                          "   <BaseClass>ModelHasElements</BaseClass>"
                                          "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Physical Elements'>"
                                          "      <Class class='Model' />"
                                          "    </Source>"
                                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Physical Elements (Reversed)'>"
                                          "      <Class class='PhysicalElement' />"
                                          "    </Target>"
                                          "  </ECRelationshipClass>"
                                          "  <ECRelationshipClass typeName='ModelHasPhysicalElements2' strength='embedding' modifier='Sealed'>"
                                          "    <ECCustomAttributes>"
                                          "        <ClassMap xmlns='ECDbMap.02.00'>"
                                          "                <MapStrategy>NotMapped</MapStrategy>"
                                          "        </ClassMap>"
                                          "    </ECCustomAttributes>"
                                          "   <BaseClass>ModelHasElements</BaseClass>"
                                          "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Physical Elements'>"
                                          "      <Class class='Model' />"
                                          "    </Source>"
                                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Physical Elements (Reversed)'>"
                                          "      <Class class='PhysicalElement' />"
                                          "    </Target>"
                                          "  </ECRelationshipClass>"
                                          "</ECSchema>", true, "Subclass can have ClassMap CA for FK mapping if MapStrategy is set to NotMapped"),
                               "validrelinheritance.ecdb");

            MapStrategyInfo mapStrategy;
            ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetClass("Test", "ModelHasElements")->GetId()));
            ASSERT_EQ(MapStrategyInfo::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);

            ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetClass("Test", "ModelHasPhysicalElements")->GetId()));
            ASSERT_EQ(MapStrategyInfo::Strategy::NotMapped, mapStrategy.m_strategy);

            ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetClass("Test", "ModelHasPhysicalElements2")->GetId()));
            ASSERT_EQ(MapStrategyInfo::Strategy::NotMapped, mapStrategy.m_strategy);
            }

            {
            ECDb ecdb;
            bool asserted = false;
            AssertSchemaImport(ecdb, asserted,
                               SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                          "<ECSchema schemaName='Test' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                          "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                          "  <ECEntityClass typeName='Model' >"
                                          "    <ECProperty propertyName='Name' typeName='string' />"
                                          "  </ECEntityClass>"
                                          "  <ECEntityClass typeName='Element' >"
                                          "    <ECProperty propertyName='Code' typeName='string' />"
                                          "  </ECEntityClass>"
                                          "  <ECRelationshipClass typeName='ModelHasElements' modifier='Abstract' strength='embedding'>"
                                          "    <ECCustomAttributes>"
                                          "        <ClassMap xmlns='ECDbMap.02.00'>"
                                          "                <MapStrategy>NotMapped</MapStrategy>"
                                          "        </ClassMap>"
                                          "    </ECCustomAttributes>"
                                          "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Elements'>"
                                          "      <Class class='Model' />"
                                          "    </Source>"
                                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Elements (Reversed)'>"
                                          "      <Class class='Element' />"
                                          "    </Target>"
                                          "  </ECRelationshipClass>"
                                          "  <ECRelationshipClass typeName='ModelHasPhysicalElements' strength='embedding' modifier='Sealed'>"
                                          "   <BaseClass>ModelHasElements</BaseClass>"
                                          "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Physical Elements'>"
                                          "      <Class class='Model' />"
                                          "    </Source>"
                                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Physical Elements (Reversed)'>"
                                          "      <Class class='Element' />"
                                          "    </Target>"
                                          "  </ECRelationshipClass>"
                                          "  <ECRelationshipClass typeName='ModelHasPhysicalElements2' strength='embedding' modifier='Sealed'>"
                                          "   <BaseClass>ModelHasElements</BaseClass>"
                                          "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Physical Elements'>"
                                          "      <Class class='Model' />"
                                          "    </Source>"
                                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Physical Elements (Reversed)'>"
                                          "      <Class class='Element' />"
                                          "    </Target>"
                                          "  </ECRelationshipClass>"
                                          "</ECSchema>"),
                               "validrelinheritance.ecdb");

            MapStrategyInfo mapStrategy;
            ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetClass("Test", "ModelHasElements")->GetId()));
            ASSERT_EQ(MapStrategyInfo::Strategy::NotMapped, mapStrategy.m_strategy);

            ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetClass("Test", "ModelHasPhysicalElements")->GetId()));
            ASSERT_EQ(MapStrategyInfo::Strategy::NotMapped, mapStrategy.m_strategy);

            ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetClass("Test", "ModelHasPhysicalElements2")->GetId()));
            ASSERT_EQ(MapStrategyInfo::Strategy::NotMapped, mapStrategy.m_strategy);
            }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECRelationshipInheritanceTestFixture, InvalidCases)
    {
    std::vector<SchemaItem> testSchemas;
    testSchemas.push_back(
    SchemaItem("<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
    "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
    "  <ECEntityClass typeName='Model' >"
    "    <ECProperty propertyName='Name' typeName='string' />"
    "  </ECEntityClass>"
    "  <ECEntityClass typeName='Element' modifier='Abstract' >"
    "    <ECCustomAttributes>"
    "         <ClassMap xmlns='ECDbMap.02.00'>"
    "                <MapStrategy>TablePerHierarchy</MapStrategy>"
    "        </ClassMap>"
    "    </ECCustomAttributes>"
    "    <ECProperty propertyName='Code' typeName='string' />"
    "    <ECNavigationProperty propertyName='ModelId' relationshipName='ModelHasElements' direction='Backward' />"
    "  </ECEntityClass>"
    "  <ECEntityClass typeName='PhysicalElement'>"
    "    <BaseClass>Element</BaseClass>"
    "    <ECProperty propertyName='Geometry' typeName='Bentley.Geometry.Common.IGeometry' />"
    "  </ECEntityClass>"
    "  <ECRelationshipClass typeName='ModelHasElements' modifier='Abstract' strength='referencing'>"
    "    <Source multiplicity='(1..1)' polymorphic='True' roleLabel='Model Has Elements'>"
    "      <Class class='Model' />"
    "    </Source>"
    "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Elements (Reversed)'>"
    "      <Class class='Element' />"
    "    </Target>"
    "  </ECRelationshipClass>"
    "  <ECRelationshipClass typeName='ModelHasPhysicalElements' strength='referencing' modifier='Sealed'>"
    "   <BaseClass>ModelHasElements</BaseClass>"
    "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Physical Elements'>"
    "      <Class class='Model' />"
    "    </Source>"
    "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Physical Elements (Reversed)'>"
    "      <Class class='PhysicalElement' />"
    "    </Target>"
    "  </ECRelationshipClass>"
    "</ECSchema>", false, "Subclass must not imply link table if base class has FK mapping"));

    testSchemas.push_back(
    SchemaItem("<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
    "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
    "  <ECEntityClass typeName='Model' >"
    "    <ECProperty propertyName='Name' typeName='string' />"
    "  </ECEntityClass>"
    "  <ECEntityClass typeName='Element' modifier='Abstract' >"
    "    <ECCustomAttributes>"
    "         <ClassMap xmlns='ECDbMap.02.00'>"
    "                <MapStrategy>TablePerHierarchy</MapStrategy>"
    "        </ClassMap>"
    "    </ECCustomAttributes>"
    "    <ECProperty propertyName='Code' typeName='string' />"
    "    <ECNavigationProperty propertyName='ModelId' relationshipName='ModelHasElements' direction='Backward' />"
    "  </ECEntityClass>"
    "  <ECEntityClass typeName='PhysicalElement'>"
    "    <BaseClass>Element</BaseClass>"
    "    <ECProperty propertyName='Geometry' typeName='Bentley.Geometry.Common.IGeometry' />"
    "  </ECEntityClass>"
    "  <ECRelationshipClass typeName='ModelHasElements' modifier='Abstract' strength='embedding'>"
    "    <Source multiplicity='(1..1)' polymorphic='True' roleLabel='Model Has Elements'>"
    "      <Class class='Model' />"
    "    </Source>"
    "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Elements (Reversed)'>"
    "      <Class class='Element' />"
    "    </Target>"
    "  </ECRelationshipClass>"
    "  <ECRelationshipClass typeName='ModelHasPhysicalElements' strength='embedding' modifier='Sealed'>"
    "    <ECCustomAttributes>"
    "        <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
    "    </ECCustomAttributes>"
    "   <BaseClass>ModelHasElements</BaseClass>"
    "    <Source multiplicity='(1..1)' polymorphic='True' roleLabel='Model Has Physical Elements'>"
    "      <Class class='Model' />"
    "    </Source>"
    "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Physical Elements'>"
    "      <Class class='PhysicalElement' />"
    "    </Target>"
    "  </ECRelationshipClass>"
    "</ECSchema>", false, "Subclasses must not have ForeignKeyConstraint even if it doesn't violate the base class mapping"));

    testSchemas.push_back(
    SchemaItem("<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
    "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
    "  <ECEntityClass typeName='Model' >"
    "    <ECProperty propertyName='Name' typeName='string' />"
    "  </ECEntityClass>"
    "  <ECEntityClass typeName='Element' modifier='Abstract' >"
    "    <ECCustomAttributes>"
    "         <ClassMap xmlns='ECDbMap.02.00'>"
    "                <MapStrategy>TablePerHierarchy</MapStrategy>"
    "        </ClassMap>"
    "    </ECCustomAttributes>"
    "    <ECProperty propertyName='Code' typeName='string' />"
    "    <ECNavigationProperty propertyName='ModelId' relationshipName='ModelHasElements' direction='Backward' />"
    "  </ECEntityClass>"
    "  <ECEntityClass typeName='PhysicalElement'>"
    "    <BaseClass>Element</BaseClass>"
    "    <ECProperty propertyName='Geometry' typeName='Bentley.Geometry.Common.IGeometry' />"
    "  </ECEntityClass>"
    "  <ECRelationshipClass typeName='ModelHasElements' modifier='Abstract' strength='referencing'>"
    "    <Source multiplicity='(1..*)' polymorphic='True' roleLabel='Model Has Elements'>"
    "      <Class class='Model' />"
    "    </Source>"
    "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Elements (Reversed)'>"
    "      <Class class='Element' />"
    "    </Target>"
    "  </ECRelationshipClass>"
    "  <ECRelationshipClass typeName='ModelHasPhysicalElements' strength='referencing' modifier='Sealed'>"
    "    <ECCustomAttributes>"
    "        <LinkTableRelationshipMap xmlns='ECDbMap.02.00'/>"
    "    </ECCustomAttributes>"
    "   <BaseClass>ModelHasElements</BaseClass>"
    "    <Source multiplicity='(1..*)' polymorphic='True' roleLabel='Model Has Physical Elements'>"
    "      <Class class='Model' />"
    "    </Source>"
               "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Physical Elements (Reversed)'>"
    "      <Class class='PhysicalElement' />"
    "    </Target>"
    "  </ECRelationshipClass>"
    "</ECSchema>", false, "Subclasses must not have LinkTableRelationshipMap even if it doesn't violate the base class mapping"));

    testSchemas.push_back(
    SchemaItem("<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
    "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
    "  <ECEntityClass typeName='Model' >"
    "    <ECProperty propertyName='Name' typeName='string' />"
    "  </ECEntityClass>"
    "  <ECEntityClass typeName='Element' modifier='Abstract' >"
    "    <ECCustomAttributes>"
    "         <ClassMap xmlns='ECDbMap.02.00'>"
    "                <MapStrategy>TablePerHierarchy</MapStrategy>"
    "        </ClassMap>"
    "    </ECCustomAttributes>"
    "    <ECProperty propertyName='Code' typeName='string' />"
    "    <ECNavigationProperty propertyName='ModelId' relationshipName='ModelHasElements' direction='Backward' />"
    "  </ECEntityClass>"
    "  <ECEntityClass typeName='PhysicalElement'>"
    "    <BaseClass>Element</BaseClass>"
    "    <ECProperty propertyName='Geometry' typeName='Bentley.Geometry.Common.IGeometry' />"
    "  </ECEntityClass>"
    "  <ECRelationshipClass typeName='ModelHasElements' modifier='Abstract' strength='referencing'>"
    "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Elements'>"
    "      <Class class='Model' />"
    "    </Source>"
    "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Elements (Reversed)'>"
    "      <Class class='Element' />"
    "    </Target>"
    "  </ECRelationshipClass>"
    "  <ECRelationshipClass typeName='ModelHasPhysicalElements' strength='referencing' modifier='Sealed'>"
    "    <ECCustomAttributes>"
    "        <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
    "    </ECCustomAttributes>"
    "   <BaseClass>ModelHasElements</BaseClass>"
    "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Physical Elements'>"
    "      <Class class='Model' />"
    "    </Source>"
    "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Physical Elements (Reversed)'>"
    "      <Class class='PhysicalElement' />"
    "    </Target>"
    "  </ECRelationshipClass>"
    "</ECSchema>", false, "Subclass must not have ForeignKeyConstraint if base class maps to link table"));

    testSchemas.push_back(
        SchemaItem("<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                   "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                   "  <ECEntityClass typeName='Model' >"
                   "    <ECProperty propertyName='Name' typeName='string' />"
                   "  </ECEntityClass>"
                   "  <ECEntityClass typeName='Element' modifier='Abstract' >"
                   "    <ECCustomAttributes>"
                   "         <ClassMap xmlns='ECDbMap.02.00'>"
                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                   "        </ClassMap>"
                   "    </ECCustomAttributes>"
                   "    <ECProperty propertyName='Code' typeName='string' />"
                   "    <ECNavigationProperty propertyName='ModelId' relationshipName='ModelHasElements' direction='Backward' />"
                   "  </ECEntityClass>"
                   "  <ECEntityClass typeName='PhysicalElement'>"
                   "    <BaseClass>Element</BaseClass>"
                   "    <ECProperty propertyName='Geometry' typeName='Bentley.Geometry.Common.IGeometry' />"
                   "  </ECEntityClass>"
                   "  <ECRelationshipClass typeName='ModelHasElements' modifier='Abstract' strength='embedding'>"
                   "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Elements'>"
                   "      <Class class='Model' />"
                   "    </Source>"
                   "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Elements (Reversed)'>"
                   "      <Class class='Element' />"
                   "    </Target>"
                   "  </ECRelationshipClass>"
                   "  <ECRelationshipClass typeName='ModelHasPhysicalElements' strength='embedding' modifier='Sealed'>"
                   "    <ECCustomAttributes>"
                   "        <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                   "    </ECCustomAttributes>"
                   "   <BaseClass>ModelHasElements</BaseClass>"
                   "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Physical Elements'>"
                   "      <Class class='Model' />"
                   "    </Source>"
                   "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Physical Elements (Reversed)'>"
                   "      <Class class='PhysicalElement' />"
                   "    </Target>"
                   "  </ECRelationshipClass>"
                   "</ECSchema>", false, "Subclass must not have ForeignKeyConstraint even if it doesn't violate the mapping type"));

    testSchemas.push_back(
    SchemaItem("<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
    "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
    "  <ECEntityClass typeName='Model' >"
    "    <ECProperty propertyName='Name' typeName='string' />"
    "  </ECEntityClass>"
    "  <ECEntityClass typeName='Element' modifier='Abstract' >"
    "    <ECCustomAttributes>"
               "         <ClassMap xmlns='ECDbMap.02.00'>"
               "                <MapStrategy>TablePerHierarchy</MapStrategy>"
               "        </ClassMap>"
    "    </ECCustomAttributes>"
    "    <ECProperty propertyName='Code' typeName='string' />"
    "    <ECNavigationProperty propertyName='ModelId' relationshipName='ModelHasElements' direction='Backward' />"
    "  </ECEntityClass>"
    "  <ECEntityClass typeName='PhysicalElement'>"
    "    <BaseClass>Element</BaseClass>"
    "    <ECProperty propertyName='Geometry' typeName='Bentley.Geometry.Common.IGeometry' />"
    "  </ECEntityClass>"
    "  <ECRelationshipClass typeName='ModelHasElements' modifier='Abstract' strength='embedding'>"
    "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Elements'>"
    "      <Class class='Model' />"
    "    </Source>"
    "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Elements (Reversed)'>"
    "      <Class class='Element' />"
    "    </Target>"
    "  </ECRelationshipClass>"
    "  <ECRelationshipClass typeName='ModelHasPhysicalElements' strength='embedding' modifier='Sealed'>"
    "    <ECCustomAttributes>"
               "         <ClassMap xmlns='ECDbMap.02.00'>"
               "                <MapStrategy>TablePerHierarchy</MapStrategy>"
               "        </ClassMap>"
    "    </ECCustomAttributes>"
    "   <BaseClass>ModelHasElements</BaseClass>"
    "   <ECProperty propertyName='Prop1' typeName='string' />"
    "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Physical Elements'>"
    "      <Class class='Model' />"
    "    </Source>"
    "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Physical Elements (Reversed)'>"
    "      <Class class='PhysicalElement' />"
    "    </Target>"
    "  </ECRelationshipClass>"
    "</ECSchema>", false, "Subclass must not add ECProperties if base class has FK mapping"));
    
    testSchemas.push_back(
        SchemaItem("<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                   "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                   "  <ECEntityClass typeName='Model' >"
                   "    <ECProperty propertyName='Name' typeName='string' />"
                   "  </ECEntityClass>"
                   "  <ECEntityClass typeName='Element' modifier='Abstract' >"
                   "    <ECCustomAttributes>"
                   "         <ClassMap xmlns='ECDbMap.02.00'>"
                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                   "        </ClassMap>"
                   "    </ECCustomAttributes>"
                   "    <ECProperty propertyName='Code' typeName='string' />"
                   "    <ECNavigationProperty propertyName='ModelId' relationshipName='ModelHasElements' direction='Backward' />"
                   "  </ECEntityClass>"
                   "  <ECEntityClass typeName='PhysicalElement'>"
                   "    <BaseClass>Element</BaseClass>"
                   "    <ECProperty propertyName='Geometry' typeName='Bentley.Geometry.Common.IGeometry' />"
                   "  </ECEntityClass>"
                   "  <ECRelationshipClass typeName='ModelHasElements' modifier='Abstract' strength='embedding'>"
                   "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Elements'>"
                   "      <Class class='Model' />"
                   "    </Source>"
                   "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Elements (Reversed)'>"
                   "      <Class class='Element' />"
                   "    </Target>"
                   "  </ECRelationshipClass>"
                   "  <ECRelationshipClass typeName='ModelHasPhysicalElements' strength='embedding' modifier='Sealed'>"
                   "    <ECCustomAttributes>"
                   "        <ClassMap xmlns='ECDbMap.02.00'>"
                   "               <MapStrategy>OwnTable</MapStrategy>"
                   "        </ClassMap>"
                   "    </ECCustomAttributes>"
                   "   <BaseClass>ModelHasElements</BaseClass>"
                   "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Physical Elements'>"
                   "      <Class class='Model' />"
                   "    </Source>"
                   "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Physical Elements (Reversed)'>"
                   "      <Class class='PhysicalElement' />"
                   "    </Target>"
                   "  </ECRelationshipClass>"
                   "</ECSchema>", false, "Subclass must not have ClassMap CA if base class has FK mapping"));


    testSchemas.push_back(
        SchemaItem("<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                   "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                   "  <ECEntityClass typeName='Model' >"
                   "    <ECProperty propertyName='Name' typeName='string' />"
                   "  </ECEntityClass>"
                   "  <ECEntityClass typeName='Element' >"
                   "    <ECProperty propertyName='Code' typeName='string' />"
                   "  </ECEntityClass>"
                   "  <ECRelationshipClass typeName='ModelHasElements' modifier='Abstract' strength='embedding'>"
                   "    <ECCustomAttributes>"
                   "        <ClassMap xmlns='ECDbMap.02.00'>"
                   "            <MapStrategy>NotMapped</MapStrategy>"
                   "        </ClassMap>"
                   "    </ECCustomAttributes>"
                   "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Elements'>"
                   "      <Class class='Model' />"
                   "    </Source>"
                   "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Elements (Reversed)'>"
                   "      <Class class='Element' />"
                   "    </Target>"
                   "  </ECRelationshipClass>"
                   "  <ECRelationshipClass typeName='ModelHasPhysicalElements' strength='embedding' modifier='Sealed'>"
                   "    <ECCustomAttributes>"
                   "        <ClassMap xmlns='ECDbMap.02.00'>"
                   "            <MapStrategy>NotMapped</MapStrategy>"
                   "        </ClassMap>"
                   "    </ECCustomAttributes>"
                   "   <BaseClass>ModelHasElements</BaseClass>"
                   "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Physical Elements'>"
                   "      <Class class='Model' />"
                   "    </Source>"
                   "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Physical Elements (Reversed)'>"
                   "      <Class class='PhysicalElement' />"
                   "    </Target>"
                   "  </ECRelationshipClass>"
                   "  <ECRelationshipClass typeName='ModelHasPhysicalElements2' strength='embedding' modifier='Sealed'>"
                   "   <BaseClass>ModelHasElements</BaseClass>"
                   "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Physical Elements'>"
                   "      <Class class='Model' />"
                   "    </Source>"
                   "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Physical Elements (Reversed)'>"
                   "      <Class class='PhysicalElement' />"
                   "    </Target>"
                   "  </ECRelationshipClass>"
                   "</ECSchema>", false, "Subclass must not have define NotMapped if base class did already"));


/*    testSchemas.push_back(
        SchemaItem("<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                   "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                   "  <ECEntityClass typeName='Model' >"
                   "    <ECProperty propertyName='Name' typeName='string' />"
                   "  </ECEntityClass>"
                   "  <ECEntityClass typeName='Element' >"
                   "    <ECProperty propertyName='Code' typeName='string' />"
                   "    <ECNavigationProperty propertyName='ModelId' relationshipName='ModelHasElements' direction='Backward' />"
                   "  </ECEntityClass>"
                   "  <ECEntityClass typeName='PhysicalElement'>"
                   "    <BaseClass>Element</BaseClass>"
                   "    <ECProperty propertyName='Geometry' typeName='Bentley.Geometry.Common.IGeometry' />"
                   "  </ECEntityClass>"
                   "  <ECRelationshipClass typeName='ModelHasElements' modifier='Sealed' strength='embedding'>"
                   "    <Source multiplicity='(0..1)' polymorphic='True'>"
                   "      <Class class='Model' />"
                   "    </Source>"
                   "    <Target multiplicity='(0..*)' polymorphic='True'>"
                   "      <Class class='Element' />"
                   "    </Target>"
                   "  </ECRelationshipClass>"
                   "</ECSchema>", false, "FK end maps to multiple primary tables"));
                   */
    AssertSchemaImport(testSchemas, "invalidrelinheritance.ecdb");
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECRelationshipInheritanceTestFixture, RelECClassId)
    {
    enum class RelClassIdExistenceMode
        {
        Persisted,
        Virtual,
        DoesNotExist
        };

    auto assertRelECClassId = [this] (ECDbCR ecdb, Utf8CP tableName, Utf8CP relClassIdColumnName, RelClassIdExistenceMode expectedExistenceMode, bool expectedNotNull)
        {
        const int relClassIdColumnKind = 256;
        Utf8String ddl = RetrieveDdl(ecdb, tableName);
        ASSERT_FALSE(ddl.empty());

        CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT c.ColumnKind, c.IsVirtual, c.NotNullConstraint FROM ec_Column c, ec_Table t WHERE c.TableId=t.Id AND t.Name=? and c.Name=?");
        ASSERT_TRUE(stmt != nullptr);
        ASSERT_EQ(BE_SQLITE_OK, stmt->BindText(1, tableName, Statement::MakeCopy::No));
        ASSERT_EQ(BE_SQLITE_OK, stmt->BindText(2, relClassIdColumnName, Statement::MakeCopy::No));
        if (expectedExistenceMode == RelClassIdExistenceMode::DoesNotExist)
            {
            ASSERT_FALSE(ddl.ContainsI(relClassIdColumnName)) << "Table: " << tableName << " Column: " << relClassIdColumnName;
            ASSERT_EQ(BE_SQLITE_DONE, stmt->Step());
            return;
            }

        ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());

        const int actualKind = stmt->GetValueInt(0);
        const bool actualIsVirtual = stmt->GetValueInt(1) != 0;
        const bool actualIsNotNull = stmt->GetValueInt(2) != 0;
        stmt = nullptr;

        ASSERT_EQ(relClassIdColumnKind, actualKind) << "Table: " << tableName << " Column: " << relClassIdColumnName;
        ASSERT_EQ(actualIsVirtual, expectedExistenceMode == RelClassIdExistenceMode::Virtual) << "Table: " << tableName << " Column: " << relClassIdColumnName;
        ASSERT_EQ(actualIsNotNull, expectedNotNull) << "Table: " << tableName << " Column: " << relClassIdColumnName;

        if (expectedExistenceMode == RelClassIdExistenceMode::Virtual)
            ASSERT_FALSE(ddl.ContainsI(relClassIdColumnName)) << "Table: " << tableName << " Column: " << relClassIdColumnName;
        else
            {
            Utf8String columnDdl("[");
            columnDdl.append(relClassIdColumnName).append("] INTEGER");
            if (expectedNotNull)
                columnDdl.append(" NOT NULL");

            ASSERT_TRUE(ddl.ContainsI(columnDdl.c_str())) << "Table: " << tableName << " Column: " << relClassIdColumnName << " Expected DDL: " << columnDdl.c_str();
            }

        Utf8String indexName;
        indexName.Sprintf("ix_%s_%s", tableName, relClassIdColumnName);

        if (expectedExistenceMode == RelClassIdExistenceMode::Virtual)
            {
            AssertIndexExists(ecdb, indexName.c_str(), false);
            return;
            }

        if (expectedNotNull)
            AssertIndex(ecdb, indexName.c_str(), false, tableName, {relClassIdColumnName});
        else
            {
            Utf8String whereClause;
            whereClause.Sprintf("([%s] IS NOT NULL)", relClassIdColumnName);
            AssertIndex(ecdb, indexName.c_str(), false, tableName, {relClassIdColumnName}, whereClause.c_str());
            }
        };

    {
    #define SCHEMAALIAS "ts1"

    SchemaItem testSchema("<ECSchema schemaName='TestSchema' alias='" SCHEMAALIAS "' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                          "  <ECEntityClass typeName='A' >"
                          "    <ECProperty propertyName='Name' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='B' >"
                          "    <ECProperty propertyName='Code' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECRelationshipClass typeName='AHasB' modifier='Abstract' strength='embedding'>"
                          "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='A Has B'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='A Has B (Reversed)'>"
                          "      <Class class='B' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "  <ECEntityClass typeName='C' >"
                          "    <ECProperty propertyName='Name' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='D' >"
                          "    <ECProperty propertyName='Code' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECRelationshipClass typeName='CHasD' modifier='Sealed' strength='embedding'>"
                          "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='C Has D'>"
                          "      <Class class='C' />"
                          "    </Source>"
                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='C Has D (Reversed)'>"
                          "      <Class class='D' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "</ECSchema>");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testSchema, "relecclassid" SCHEMAALIAS ".ecdb");
    ASSERT_FALSE(asserted);

    assertRelECClassId(ecdb, SCHEMAALIAS "_B", "RelECClassId_" SCHEMAALIAS "_AHasB", RelClassIdExistenceMode::Persisted, false);
    assertRelECClassId(ecdb, SCHEMAALIAS "_D", "RelECClassId_" SCHEMAALIAS "_CHasD", RelClassIdExistenceMode::Virtual, false);
    }

    {
    #undef SCHEMAALIAS
    #define SCHEMAALIAS "ts2"

    SchemaItem testSchema("<ECSchema schemaName='TestSchema' alias='" SCHEMAALIAS "' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                          "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                          "  <ECEntityClass typeName='A' >"
                          "    <ECProperty propertyName='Name' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='B' modifier='Abstract' >"
                          "    <ECCustomAttributes>"
                          "         <ClassMap xmlns='ECDbMap.02.00'>"
                          "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                          "        </ClassMap>"
                          "    </ECCustomAttributes>"
                          "    <ECProperty propertyName='Code' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='BSub' modifier='Sealed' >"
                          "    <BaseClass>B</BaseClass>"
                          "    <ECProperty propertyName='Prop1' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECRelationshipClass typeName='AHasB' modifier='Abstract' strength='embedding'>"
                          "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='A Has B'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='A Has B (Reversed)'>"
                          "      <Class class='B' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "  <ECRelationshipClass typeName='AHasB1N' modifier='Abstract' strength='embedding'>"
                          "    <Source multiplicity='(1..1)' polymorphic='True' roleLabel='A Has B'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='A Has B (Reversed)'>"
                          "      <Class class='B' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "</ECSchema>");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testSchema, "relecclassid" SCHEMAALIAS ".ecdb");
    ASSERT_FALSE(asserted);

    assertRelECClassId(ecdb, SCHEMAALIAS "_B", "RelECClassId_" SCHEMAALIAS "_AHasB", RelClassIdExistenceMode::Persisted, false);
    assertRelECClassId(ecdb, SCHEMAALIAS "_B", "RelECClassId_" SCHEMAALIAS "_AHasB1N", RelClassIdExistenceMode::Persisted, true);
    }

    {
#undef SCHEMAALIAS
#define SCHEMAALIAS "ts3"

    SchemaItem testSchema("<ECSchema schemaName='TestSchema' alias='" SCHEMAALIAS "' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                          "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                          "  <ECEntityClass typeName='A' >"
                          "    <ECProperty propertyName='Name' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='B' modifier='Abstract' >"
                          "    <ECCustomAttributes>"
                          "         <ClassMap xmlns='ECDbMap.02.00'>"
                          "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                          "        </ClassMap>"
                          "    </ECCustomAttributes>"
                          "    <ECProperty propertyName='Code' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='BSub' modifier='Sealed' >"
                          "    <BaseClass>B</BaseClass>"
                          "    <ECProperty propertyName='Prop1' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECRelationshipClass typeName='AHasB' modifier='Abstract' strength='embedding'>"
                          "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='A Has B'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='A Has B (Reversed)'>"
                          "      <Class class='B' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "  <ECRelationshipClass typeName='AHasBSub' modifier='Sealed' strength='embedding'>"
                          "    <BaseClass>AHasB</BaseClass>"
                          "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='A Has B'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='A Has B (Reversed)'>"
                          "      <Class class='BSub' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "  <ECRelationshipClass typeName='AHasB1N' modifier='Abstract' strength='embedding'>"
                          "    <Source multiplicity='(1..1)' polymorphic='True' roleLabel='A Has B'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='A Has B (Reversed)'>"
                          "      <Class class='B' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "  <ECRelationshipClass typeName='AHasB1NSub' modifier='Sealed' strength='embedding'>"
                          "    <BaseClass>AHasB1N</BaseClass>"
                          "    <Source multiplicity='(1..1)' polymorphic='True' roleLabel='A Has B'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='A Has B (Reversed)'>"
                          "      <Class class='BSub' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "</ECSchema>");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testSchema, "relecclassid" SCHEMAALIAS ".ecdb");
    ASSERT_FALSE(asserted);

    Utf8CP tableName = SCHEMAALIAS "_B";
    assertRelECClassId(ecdb, tableName, "RelECClassId_" SCHEMAALIAS "_AHasB", RelClassIdExistenceMode::Persisted, false);
    assertRelECClassId(ecdb, tableName, "RelECClassId_" SCHEMAALIAS "_AHasBSub", RelClassIdExistenceMode::DoesNotExist, false);
    assertRelECClassId(ecdb, tableName, "RelECClassId_" SCHEMAALIAS "_AHasB1N", RelClassIdExistenceMode::Persisted, true);
    assertRelECClassId(ecdb, tableName, "RelECClassId_" SCHEMAALIAS "_AHasB1NSub", RelClassIdExistenceMode::DoesNotExist, false);
    }

    {
#undef SCHEMAALIAS
#define SCHEMAALIAS "ts4"

    SchemaItem testSchema("<ECSchema schemaName='TestSchema' alias='" SCHEMAALIAS "' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                          "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                          "  <ECEntityClass typeName='A' >"
                          "    <ECProperty propertyName='Name' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='B' modifier='Abstract' >"
                          "    <ECCustomAttributes>"
                          "         <ClassMap xmlns='ECDbMap.02.00'>"
                          "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                          "        </ClassMap>"
                          "    </ECCustomAttributes>"
                          "    <ECProperty propertyName='Code' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='BSub' modifier='Sealed' >"
                          "    <BaseClass>B</BaseClass>"
                          "    <ECProperty propertyName='Prop1' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECRelationshipClass typeName='AHasBSub' modifier='Abstract' strength='embedding'>"
                          "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='A Has B'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='A Has B (Reversed)'>"
                          "      <Class class='BSub' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "  <ECRelationshipClass typeName='AHasBSub1N' modifier='Abstract' strength='embedding'>"
                          "    <Source multiplicity='(1..1)' polymorphic='True' roleLabel='A Has B'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='A Has B (Reversed)'>"
                          "      <Class class='BSub' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "</ECSchema>");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testSchema, "relecclassid" SCHEMAALIAS ".ecdb");
    ASSERT_FALSE(asserted);

    assertRelECClassId(ecdb, SCHEMAALIAS "_B", "RelECClassId_" SCHEMAALIAS "_AHasBSub", RelClassIdExistenceMode::Persisted, false);
    //cardinality would imply NOT NULL on rel class id, but the column is shared by other base class rows, so no enforcement of NOT NULL.
    assertRelECClassId(ecdb, SCHEMAALIAS "_B", "RelECClassId_" SCHEMAALIAS "_AHasBSub1N", RelClassIdExistenceMode::Persisted, false);
    }

    {
#undef SCHEMAALIAS
#define SCHEMAALIAS "ts5"

    SchemaItem testSchema("<ECSchema schemaName='TestSchema' alias='" SCHEMAALIAS "' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                          "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                          "  <ECEntityClass typeName='A' >"
                          "    <ECProperty propertyName='Name' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='B' modifier='Abstract' >"
                          "    <ECCustomAttributes>"
                          "         <ClassMap xmlns='ECDbMap.02.00'>"
                          "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                          "        </ClassMap>"
                          "    </ECCustomAttributes>"
                          "    <ECProperty propertyName='Code' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='BSub' modifier='Sealed' >"
                          "    <BaseClass>B</BaseClass>"
                          "    <ECProperty propertyName='Prop1' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECRelationshipClass typeName='AHasBSub' modifier='Sealed' strength='embedding'>"
                          "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='A Has B'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='A Has B (Reversed)'>"
                          "      <Class class='BSub' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "  <ECRelationshipClass typeName='AHasB1N' modifier='Sealed' strength='embedding'>"
                          "    <Source multiplicity='(1..1)' polymorphic='True' roleLabel='A Has B'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='A Has B (Reversed)'>"
                          "      <Class class='B' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "  <ECRelationshipClass typeName='AHasBSub1N' modifier='Sealed' strength='embedding'>"
                          "    <Source multiplicity='(1..1)' polymorphic='True' roleLabel='A Has B'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='A Has B (Reversed)'>"
                          "      <Class class='BSub' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "</ECSchema>");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testSchema, "relecclassid" SCHEMAALIAS ".ecdb");
    ASSERT_FALSE(asserted);

    assertRelECClassId(ecdb, SCHEMAALIAS "_B", "RelECClassId_" SCHEMAALIAS "_AHasBSub", RelClassIdExistenceMode::Virtual, false);
    assertRelECClassId(ecdb, SCHEMAALIAS "_B", "RelECClassId_" SCHEMAALIAS "_AHasB1N", RelClassIdExistenceMode::Virtual, true);
    assertRelECClassId(ecdb, SCHEMAALIAS "_B", "RelECClassId_" SCHEMAALIAS "_AHasBSub1N", RelClassIdExistenceMode::Virtual, false);
    }

    {
    //LinkTable
#undef SCHEMAALIAS
#define SCHEMAALIAS "ts6"

    SchemaItem testSchema("<ECSchema schemaName='TestSchema' alias='" SCHEMAALIAS "' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                          "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                          "  <ECEntityClass typeName='A' >"
                          "    <ECProperty propertyName='Name' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='B' modifier='Abstract' >"
                          "    <ECProperty propertyName='Code' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='BSub' modifier='Sealed' >"
                          "    <BaseClass>B</BaseClass>"
                          "    <ECProperty propertyName='Prop1' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECRelationshipClass typeName='AHasB' modifier='Sealed' strength='referencing'>"
                          "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='A Has B'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='A Has B (Reversed)'>"
                          "      <Class class='B' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "</ECSchema>");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testSchema, "relecclassid" SCHEMAALIAS ".ecdb");
    ASSERT_FALSE(asserted);

    ASSERT_TRUE(ecdb.TableExists(SCHEMAALIAS "_AHasB"));
    ASSERT_FALSE(ecdb.ColumnExists(SCHEMAALIAS "_AHasB", "ECClassId"));
   }

    {
    //LinkTable
#undef SCHEMAALIAS
#define SCHEMAALIAS "ts7"

    SchemaItem testSchema("<ECSchema schemaName='TestSchema' alias='" SCHEMAALIAS "' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                          "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                          "  <ECEntityClass typeName='A' >"
                          "    <ECProperty propertyName='Name' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='B' modifier='Abstract' >"
                          "    <ECProperty propertyName='Code' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='BSub' modifier='Sealed' >"
                          "    <BaseClass>B</BaseClass>"
                          "    <ECProperty propertyName='Prop1' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECRelationshipClass typeName='AHasB' modifier='Abstract' strength='referencing'>"
                          "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='A Has B'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='A Has B (Reversed)'>"
                          "      <Class class='B' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "  <ECRelationshipClass typeName='AHasBSub' modifier='Sealed' strength='referencing'>"
                          "    <BaseClass>AHasB</BaseClass>"
                          "    <Source multiplicity='(0..*)' polymorphic='True'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target multiplicity='(0..*)' polymorphic='True'>"
                          "      <Class class='B' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "</ECSchema>");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testSchema, "relecclassid" SCHEMAALIAS ".ecdb");
    ASSERT_FALSE(asserted);

    ASSERT_TRUE(ecdb.TableExists(SCHEMAALIAS "_AHasB"));
    ASSERT_TRUE(ecdb.ColumnExists(SCHEMAALIAS "_AHasB", "ECClassId"));
    }

#undef SCHEMAALIAS
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  09/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECRelationshipInheritanceTestFixture, NarrowingSemanticsFKMapping)
    {
    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted,
                       SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                  "<ECSchema schemaName='Test' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                  "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                  "  <ECEntityClass typeName='Element' modifier='Abstract' >"
                                  "    <ECCustomAttributes>"
                                  "       <ClassMap xmlns='ECDbMap.02.00'>"
                                  "            <MapStrategy>TablePerHierarchy</MapStrategy>"
                                  "       </ClassMap>"
                                  "    </ECCustomAttributes>"
                                  "    <ECProperty propertyName='Code' typeName='string' />"
                                  "    <ECNavigationProperty propertyName='ParentId' relationshipName='ElementOwnsChildElements' direction='Backward' />"
                                  "  </ECEntityClass>"
                                  "  <ECEntityClass typeName='BoltElement'>"
                                  "    <BaseClass>Element</BaseClass>"
                                  "    <ECProperty propertyName='BoltType' typeName='string' />"
                                  "  </ECEntityClass>"
                                  "  <ECEntityClass typeName='ConnectionElement' modifier='Abstract' >"
                                  "    <BaseClass>Element</BaseClass>"
                                  "    <ECProperty propertyName='ConnectionType' typeName='string' />"
                                  "  </ECEntityClass>"
                                  "  <ECEntityClass typeName='SteelBeamConnectionElement'>"
                                  "    <BaseClass>ConnectionElement</BaseClass>"
                                  "  </ECEntityClass>"
                                  "  <ECEntityClass typeName='PipeFlangeConnectionElement'>"
                                  "    <BaseClass>ConnectionElement</BaseClass>"
                                  "  </ECEntityClass>"
                                  "  <ECRelationshipClass typeName='ElementOwnsChildElements' modifier='Abstract' strength='embedding'>"
                                  "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Element Owns Child Elements'>"
                                  "      <Class class='Element' />"
                                  "    </Source>"
                                  "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Element Owns Child Elements (Reversed)'>"
                                  "      <Class class='Element' />"
                                  "    </Target>"
                                  "  </ECRelationshipClass>"
                                  "  <ECRelationshipClass typeName='SteelBeamConnectionHasBolts' strength='embedding' modifier='Sealed'>"
                                  "   <BaseClass>ElementOwnsChildElements</BaseClass>"
                                  "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Steel Beam Connection Has Bolts'>"
                                  "      <Class class='SteelBeamConnectionElement' />"
                                  "    </Source>"
                                  "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Steel Beam Connection Has Bolts (Reversed)'>"
                                  "      <Class class='BoltElement' />"
                                  "    </Target>"
                                  "  </ECRelationshipClass>"
                                  "  <ECRelationshipClass typeName='PipeFlangeConnectionHasBolts' strength='embedding' modifier='Sealed'>"
                                  "   <BaseClass>ElementOwnsChildElements</BaseClass>"
                                  "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Pipe Flange Connection Has Bolts'>"
                                  "      <Class class='PipeFlangeConnectionElement' />"
                                  "    </Source>"
                                  "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Pipe Flange Connection Has Bolts (Reversed)'>"
                                  "      <Class class='BoltElement' />"
                                  "    </Target>"
                                  "  </ECRelationshipClass>"
                                  "</ECSchema>", true),
                       "narrowingsemanticsrelinheritance_fkmapping.ecdb");
    ASSERT_FALSE(asserted);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.SteelBeamConnectionElement(Code,ConnectionType) VALUES('SteelBeamConnection1','Steel')"));
    ECInstanceKey stealbeamConnKey;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(stealbeamConnKey));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.PipeFlangeConnectionElement(Code,ConnectionType) VALUES('PipeFlangeConnection1','Steel')"));
    ECInstanceKey pipeflangeConnKey;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(pipeflangeConnKey));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.BoltElement(Code,BoltType) VALUES('Bolt1','Dunno')"));
    ECInstanceKey bolt1Key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(bolt1Key));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.BoltElement(Code,BoltType) VALUES('Bolt2','Dunno')"));
    ECInstanceKey bolt2Key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(bolt2Key));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.SteelBeamConnectionHasBolts(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, stealbeamConnKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, stealbeamConnKey.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, bolt1Key.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, bolt1Key.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ecdb.SaveChanges();
    //verify equality between leaf and base rel class
    Utf8String ecsql;
    ecsql.Sprintf("SELECT ECInstanceId, ECClassId FROM ONLY ts.ElementOwnsChildElements WHERE SourceECInstanceId=%s AND TargetECInstanceId=%s",
                  stealbeamConnKey.GetInstanceId().ToString().c_str(), bolt1Key.GetInstanceId().ToString().c_str());

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.PipeFlangeConnectionHasBolts(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, pipeflangeConnKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, pipeflangeConnKey.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, bolt1Key.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, bolt1Key.GetClassId()));
    ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, stmt.Step()) << "The particular Bolt has already an ElementOwnsChildElement rel";
    }

//---------------------------------------------------------------------------------------
// we actually want to disallow non-abstract base classes, but might need them for legacy schemas
// @bsimethod                                   Krischan.Eberle                  09/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECRelationshipInheritanceTestFixture, NarrowingSemanticsFKMapping_NonAbstractRelBaseClass)
    {
    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted,
                        SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                    "<ECSchema schemaName='Test' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                    "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                    "  <ECEntityClass typeName='Element' modifier='Abstract' >"
                                    "    <ECCustomAttributes>"
                                    "       <ClassMap xmlns='ECDbMap.02.00'>"
                                    "            <MapStrategy>TablePerHierarchy</MapStrategy>"
                                    "       </ClassMap>"
                                    "    </ECCustomAttributes>"
                                    "    <ECProperty propertyName='Code' typeName='string' />"
                                    "    <ECNavigationProperty propertyName='ParentId' relationshipName='ElementOwnsChildElements' direction='Backward' />"
                                    "  </ECEntityClass>"
                                    "  <ECEntityClass typeName='BoltElement'>"
                                    "    <BaseClass>Element</BaseClass>"
                                    "    <ECProperty propertyName='BoltType' typeName='string' />"
                                    "  </ECEntityClass>"
                                    "  <ECEntityClass typeName='ConnectionElement' modifier='Abstract' >"
                                    "    <BaseClass>Element</BaseClass>"
                                    "    <ECProperty propertyName='ConnectionType' typeName='string' />"
                                    "  </ECEntityClass>"
                                    "  <ECEntityClass typeName='SteelBeamConnectionElement'>"
                                    "    <BaseClass>ConnectionElement</BaseClass>"
                                    "  </ECEntityClass>"
                                    "  <ECEntityClass typeName='PipeFlangeConnectionElement'>"
                                    "    <BaseClass>ConnectionElement</BaseClass>"
                                    "  </ECEntityClass>"
                                    "  <ECRelationshipClass typeName='ElementOwnsChildElements' modifier='None' strength='embedding'>"
                                    "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Element Owns Child Elements'>"
                                    "      <Class class='Element' />"
                                    "    </Source>"
                                    "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Element Owns Child Elements (Reversed)'>"
                                    "      <Class class='Element' />"
                                    "    </Target>"
                                    "  </ECRelationshipClass>"
                                    "  <ECRelationshipClass typeName='SteelBeamConnectionHasBolts' strength='embedding' modifier='Sealed'>"
                                    "   <BaseClass>ElementOwnsChildElements</BaseClass>"
                                    "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Steel Beam Connection Has Bolts'>"
                                    "      <Class class='SteelBeamConnectionElement' />"
                                    "    </Source>"
                                    "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Steel Beam Connection Has Bolts (Reversed)'>"
                                    "      <Class class='BoltElement' />"
                                    "    </Target>"
                                    "  </ECRelationshipClass>"
                                    "  <ECRelationshipClass typeName='PipeFlangeConnectionHasBolts' strength='embedding' modifier='Sealed'>"
                                    "   <BaseClass>ElementOwnsChildElements</BaseClass>"
                                    "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Pipe Flange Connection Has Bolts'>"
                                    "      <Class class='PipeFlangeConnectionElement' />"
                                    "    </Source>"
                                    "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Pipe Flange Connection Has Bolts (Reversed)'>"
                                    "      <Class class='BoltElement' />"
                                    "    </Target>"
                                    "  </ECRelationshipClass>"
                                    "</ECSchema>", true),
                        "narrowingsemanticsrelinheritance_fkmapping_nonabstractrelbaseclass.ecdb");
    ASSERT_FALSE(asserted);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.SteelBeamConnectionElement(Code,ConnectionType) VALUES('SteelBeamConnection1','Steel')"));
    ECInstanceKey stealbeamConnKey;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(stealbeamConnKey));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.PipeFlangeConnectionElement(Code,ConnectionType) VALUES('PipeFlangeConnection1','Steel')"));
    ECInstanceKey pipeflangeConnKey;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(pipeflangeConnKey));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.BoltElement(Code,BoltType) VALUES('Bolt1','Dunno')"));
    ECInstanceKey bolt1Key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(bolt1Key));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.BoltElement(Code,BoltType) VALUES('Bolt2','Dunno')"));
    ECInstanceKey bolt2Key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(bolt2Key));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.ElementOwnsChildElements(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, stealbeamConnKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, stealbeamConnKey.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, bolt1Key.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, bolt1Key.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //verify equality between leaf and base rel class
    Utf8String ecsql;
    ecsql.Sprintf("SELECT ECInstanceId, ECClassId FROM ONLY ts.ElementOwnsChildElements WHERE SourceECInstanceId=%s AND TargetECInstanceId=%s",
                    stealbeamConnKey.GetInstanceId().ToString().c_str(), bolt1Key.GetInstanceId().ToString().c_str());

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(bolt1Key.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Only one row expected for ECSQL " << ecsql.c_str();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.PipeFlangeConnectionHasBolts(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, pipeflangeConnKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, pipeflangeConnKey.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, bolt1Key.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, bolt1Key.GetClassId()));
    ASSERT_EQ(BE_SQLITE_CONSTRAINT_UNIQUE, stmt.Step()) << "The particular Bolt has already an ElementOwnsChildElement rel";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  09/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECRelationshipInheritanceTestFixture, NarrowingSemanticsLinkTable)
    {
    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted,
                       SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                  "<ECSchema schemaName='Test' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                  "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                  "  <ECEntityClass typeName='Element' modifier='Abstract' >"
                                  "    <ECCustomAttributes>"
                                  "       <ClassMap xmlns='ECDbMap.02.00'>"
                                  "            <MapStrategy>TablePerHierarchy</MapStrategy>"
                                  "       </ClassMap>"
                                  "    </ECCustomAttributes>"
                                  "    <ECProperty propertyName='Code' typeName='string' />"
                                  "  </ECEntityClass>"
                                  "  <ECEntityClass typeName='BoltElement'>"
                                  "    <BaseClass>Element</BaseClass>"
                                  "    <ECProperty propertyName='BoltType' typeName='string' />"
                                  "  </ECEntityClass>"
                                  "  <ECEntityClass typeName='ConnectionElement' modifier='Abstract' >"
                                  "    <BaseClass>Element</BaseClass>"
                                  "    <ECProperty propertyName='ConnectionType' typeName='string' />"
                                  "  </ECEntityClass>"
                                  "  <ECEntityClass typeName='SteelBeamConnectionElement'>"
                                  "    <BaseClass>ConnectionElement</BaseClass>"
                                  "  </ECEntityClass>"
                                  "  <ECEntityClass typeName='PipeFlangeConnectionElement'>"
                                  "    <BaseClass>ConnectionElement</BaseClass>"
                                  "  </ECEntityClass>"
                                  "  <ECRelationshipClass typeName='ElementDrivesChildElements' modifier='Abstract' strength='referencing'>"
                                  "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='Element Drives Child Elements'>"
                                  "      <Class class='Element' />"
                                  "    </Source>"
                                  "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Element Drives Child Elements (Reversed)'>"
                                  "      <Class class='Element' />"
                                  "    </Target>"
                                  "  </ECRelationshipClass>"
                                  "  <ECRelationshipClass typeName='SteelBeamConnectionDrivesBolt' modifier='Sealed' strength='referencing'>"
                                  "   <BaseClass>ElementDrivesChildElements</BaseClass>"
                                  "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='Steel Beam Connection Drives Bolt'>"
                                  "      <Class class='SteelBeamConnectionElement' />"
                                  "    </Source>"
                                  "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Steel Beam Connection Drives Bolt (Reversed)'>"
                                  "      <Class class='BoltElement' />"
                                  "    </Target>"
                                  "  </ECRelationshipClass>"
                                  "  <ECRelationshipClass typeName='SteelBeamConnectionDrivesBolt2' modifier='Sealed' strength='referencing'>"
                                  "   <BaseClass>ElementDrivesChildElements</BaseClass>"
                                  "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='Steel Beam Connection Drives Bolt'>"
                                  "      <Class class='SteelBeamConnectionElement' />"
                                  "    </Source>"
                                  "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Steel Beam Connection Drives Bolt (Reversed)'>"
                                  "      <Class class='BoltElement' />"
                                  "    </Target>"
                                  "  </ECRelationshipClass>"
                                  "  <ECRelationshipClass typeName='PipeFlangeConnectionDrivesBolt' modifier='Sealed' strength='referencing'>"
                                  "   <BaseClass>ElementDrivesChildElements</BaseClass>"
                                  "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='Pipe Flange Connection Drives Bolt'>"
                                  "      <Class class='PipeFlangeConnectionElement' />"
                                  "    </Source>"
                                  "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Pipe Flange Connection Drives Bolt (Reversed)'>"
                                  "      <Class class='BoltElement' />"
                                  "    </Target>"
                                  "  </ECRelationshipClass>"
                                  "</ECSchema>", true),
                       "narrowingsemanticsrelinheritance_linktable.ecdb");
    ASSERT_FALSE(asserted);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.SteelBeamConnectionElement(Code,ConnectionType) VALUES('SteelBeamConnection1','Steel')"));
    ECInstanceKey stealbeamConnKey;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(stealbeamConnKey));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.PipeFlangeConnectionElement(Code,ConnectionType) VALUES('PipeFlangeConnection1','Steel')"));
    ECInstanceKey pipeflangeConnKey;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(pipeflangeConnKey));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.BoltElement(Code,BoltType) VALUES('Bolt1','Dunno')"));
    ECInstanceKey bolt1Key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(bolt1Key));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.BoltElement(Code,BoltType) VALUES('Bolt2','Dunno')"));
    ECInstanceKey bolt2Key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(bolt2Key));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.SteelBeamConnectionDrivesBolt(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, stealbeamConnKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, stealbeamConnKey.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, bolt1Key.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, bolt1Key.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.SteelBeamConnectionDrivesBolt2(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, stealbeamConnKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, stealbeamConnKey.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, bolt1Key.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, bolt1Key.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()); //this should actually fail, but narrowing is not enforced for link tables (yet)
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.PipeFlangeConnectionDrivesBolt(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, pipeflangeConnKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, pipeflangeConnKey.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, bolt1Key.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, bolt1Key.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //verify equality between leaf and base rel class
    Utf8String ecsql;
    ecsql.Sprintf("SELECT ECInstanceId, ECClassId FROM ONLY ts.ElementDrivesChildElements WHERE SourceECInstanceId=%s AND TargetECInstanceId=%s",
                  stealbeamConnKey.GetInstanceId().ToString().c_str(), bolt1Key.GetInstanceId().ToString().c_str());

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()); //For link tables ECDb doesn't enforce equality between base and leaf instances. They are standalone classes like regular classes
    stmt.Finalize();

    //now do polymorphic query to see whether we catch the subclass instances
    ecsql.Sprintf("SELECT count(*) FROM ts.ElementDrivesChildElements WHERE SourceECInstanceId=%s AND TargetECInstanceId=%s",
                  stealbeamConnKey.GetInstanceId().ToString().c_str(), bolt1Key.GetInstanceId().ToString().c_str());

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(2, stmt.GetValueInt(0)); //one for SteelBeamConnectionDrivesBolt and one for SteelBeamConnectionDrivesBolt2
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECRelationshipInheritanceTestFixture, InheritingAllowDuplicateRelationships)
    {
    ECDbCR ecdb = SetupECDb("ecrelinheritance_allowduplicaterelationships.ecdb", SchemaItem("<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
               "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
               "  <ECEntityClass typeName='A' >"
               "    <ECProperty propertyName='Name' typeName='string' />"
               "  </ECEntityClass>"
               "  <ECEntityClass typeName='B' >"
               "    <ECProperty propertyName='Code' typeName='string' />"
               "  </ECEntityClass>"
               "  <ECRelationshipClass typeName='BaseRel' modifier='Abstract' strength='referencing'>"
               "    <ECCustomAttributes>"
               "        <LinkTableRelationshipMap xmlns='ECDbMap.02.00'>"
               "             <AllowDuplicateRelationships>True</AllowDuplicateRelationships>"
               "        </LinkTableRelationshipMap>"
               "    </ECCustomAttributes>"
               "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='BaseRel'>"
               "      <Class class='A' />"
               "    </Source>"
               "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='BaseRel (Reversed)'>"
               "      <Class class='B' />"
               "    </Target>"
               "  </ECRelationshipClass>"
               "  <ECRelationshipClass typeName='SubRel' modifier='Sealed' strength='referencing'>"
               "   <BaseClass>BaseRel</BaseClass>"
               "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='SubRel'>"
               "      <Class class='A' />"
               "    </Source>"
               "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='SubRel (Reversed)'>"
               "      <Class class='B' />"
               "    </Target>"
               "  </ECRelationshipClass>"
               "</ECSchema>"), 3);

    ASSERT_TRUE(ecdb.IsDbOpen());

    ECInstanceKey a, b;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECClassId, ECInstanceId FROM ts.A LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    a = ECInstanceKey(stmt.GetValueId<ECClassId>(0), stmt.GetValueId<ECInstanceId>(1));
    ASSERT_TRUE(a.IsValid());
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECClassId, ECInstanceId FROM ts.B LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    b = ECInstanceKey(stmt.GetValueId<ECClassId>(0), stmt.GetValueId<ECInstanceId>(1));
    ASSERT_TRUE(b.IsValid());
    }

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.SubRel(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, a.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, a.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, b.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, b.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Insert of first relationship: " << ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();
    //now insert same rel again.
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, a.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, a.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, b.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, b.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Insert of duplicate relationship: " << ecdb.GetLastError().c_str();
    }

END_ECDBUNITTESTS_NAMESPACE
