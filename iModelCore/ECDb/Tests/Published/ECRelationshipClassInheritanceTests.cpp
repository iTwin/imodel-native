/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECRelationshipClassInheritanceTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "SchemaImportTestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================    
// @bsiclass                                   Krischan.Eberle                  06/16
//=======================================================================================    
struct ECRelationshipInheritanceTestFixture : ECDbMappingTestFixture
    {};


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECRelationshipInheritanceTestFixture, BasicCRUD)
    {
    Utf8String ecdbFilePath;
    {
    SchemaItem testSchema("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                          "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
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
                          "    <Source cardinality='(1,1)' polymorphic='True'>"
                          "      <Class class='Model' />"
                          "    </Source>"
                          "    <Target cardinality='(0,N)' polymorphic='True'>"
                          "      <Class class='Element' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "  <ECRelationshipClass typeName='ModelHasGeometric3dElements' strength='embedding' modifier='Sealed'>"
                          "   <BaseClass>ModelHasElements</BaseClass>"
                          "    <Source cardinality='(1,1)' polymorphic='True'>"
                          "      <Class class='Model' />"
                          "    </Source>"
                          "    <Target cardinality='(0,N)' polymorphic='True'>"
                          "      <Class class='Geometric3dElement' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "  <ECRelationshipClass typeName='ModelHasLinkElements' strength='embedding' modifier='Sealed'>"
                          "   <BaseClass>ModelHasElements</BaseClass>"
                          "    <Source cardinality='(1,1)' polymorphic='True'>"
                          "      <Class class='Model' />"
                          "    </Source>"
                          "    <Target cardinality='(0,N)' polymorphic='True'>"
                          "      <Class class='LinkElement' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "  <ECRelationshipClass typeName='ElementDrivesElement' strength='referencing' modifier='Abstract'>"
                          "    <Source cardinality='(0,N)' polymorphic='True'>"
                          "      <Class class='Element' />"
                          "    </Source>"
                          "    <Target cardinality='(0,N)' polymorphic='True'>"
                          "      <Class class='Element' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "  <ECRelationshipClass typeName='InformationElementDrivesInformationElement' strength='referencing' modifier='Sealed'>"
                          "   <BaseClass>ElementDrivesElement</BaseClass>"
                          "    <Source cardinality='(0,N)' polymorphic='True'>"
                          "      <Class class='InformationElement' />"
                          "    </Source>"
                          "    <Target cardinality='(0,N)' polymorphic='True'>"
                          "      <Class class='InformationElement' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "  <ECRelationshipClass typeName='UrlLinkDrivesAnnotation3dElement' strength='referencing' modifier='Sealed'>"
                          "   <BaseClass>ElementDrivesElement</BaseClass>"
                          "    <Source cardinality='(0,N)' polymorphic='True'>"
                          "      <Class class='UrlLink' />"
                          "    </Source>"
                          "    <Target cardinality='(0,N)' polymorphic='True'>"
                          "      <Class class='Annotation3dElement' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "</ECSchema>");
    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testSchema, "ecdbrelationshipinheritance.ecdb");
    ASSERT_FALSE(asserted);

    ASSERT_EQ(SUCCESS, ecdb.Schemas().CreateECClassViewsInDb());
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

    modelHasGeom3dElementsRelClassId = ecdb.Schemas().GetECClassId("TestSchema", "ModelHasGeometric3dElements");
    ASSERT_TRUE(modelHasGeom3dElementsRelClassId.IsValid());
    modelHasLinkElementsRelClassId = ecdb.Schemas().GetECClassId("TestSchema", "ModelHasLinkElements");
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[0].GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, modelHasGeom3dElementsRelClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    volumeElKeys.push_back(key);
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "VolumeElement 2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[1].GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, modelHasGeom3dElementsRelClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    volumeElKeys.push_back(key);
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "VolumeElement 3", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[1].GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, modelHasGeom3dElementsRelClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    volumeElKeys.push_back(key);
    stmt.Finalize();

    //AnnotationElement
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.Annotation3dElement(Code, Model.Id, Model.RelECClassId, Font) VALUES(?,?,?, 'Consolas Sans Serif')"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Annotation3dElement 1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[0].GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, modelHasGeom3dElementsRelClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    annotationElKeys.push_back(key);
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Annotation3dElement 2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[1].GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, modelHasGeom3dElementsRelClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    annotationElKeys.push_back(key);
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Annotation3dElement 3", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[1].GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, modelHasGeom3dElementsRelClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    annotationElKeys.push_back(key);
    stmt.Finalize();

    //UrlLink
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.UrlLink(Code, Model.Id,Model.RelECClassId, Url) VALUES(?,?,?, 'http://www.staufen.de')"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "UrlLinkElement 1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[0].GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, modelHasLinkElementsRelClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    urlLinkKeys.push_back(key);
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "UrlLinkElement 2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[0].GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, modelHasLinkElementsRelClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    urlLinkKeys.push_back(key);
    stmt.Finalize();

    //EmbeddedLink
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.EmbeddedLink(Code, Model.Id, Model.RelECClassId, Name) VALUES(?,?,?, 'bliblablub')"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "EmbeddedLinkElement 1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[1].GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, modelHasLinkElementsRelClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    embeddedLinkKeys.push_back(key);
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "EmbeddedLinkElement 2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[1].GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, modelHasLinkElementsRelClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    embeddedLinkKeys.push_back(key);
    stmt.Finalize();

    //InformationElementDrivesInformationElement
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.InformationElementDrivesInformationElement(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, urlLinkKeys[0].GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, urlLinkKeys[0].GetECClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, embeddedLinkKeys[0].GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, embeddedLinkKeys[0].GetECClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, urlLinkKeys[0].GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, urlLinkKeys[0].GetECClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, embeddedLinkKeys[1].GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, embeddedLinkKeys[1].GetECClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << ecdb.GetLastError().c_str();
    stmt.Finalize();

    //UrlLinkDrivesAnnotationElement
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.UrlLinkDrivesAnnotation3dElement(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, urlLinkKeys[1].GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, urlLinkKeys[1].GetECClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, annotationElKeys[0].GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, annotationElKeys[0].GetECClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, urlLinkKeys[1].GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, urlLinkKeys[1].GetECClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, annotationElKeys[1].GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, annotationElKeys[1].GetECClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, urlLinkKeys[1].GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, urlLinkKeys[1].GetECClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, annotationElKeys[2].GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, annotationElKeys[2].GetECClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << ecdb.GetLastError().c_str();
    stmt.Finalize();
    }

    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbFilePath.c_str(), ECDb::OpenParams(Db::OpenMode::Readonly)));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId, ECClassId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ts.ModelHasElements"));
    int rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ASSERT_EQ(modelKeys[0].GetECClassId().GetValue(), stmt.GetValueId<ECInstanceId>(2).GetValue()) << "SourceECInstanceId";
        rowCount++;
        }
    ASSERT_EQ(10, rowCount) << stmt.GetECSql();
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId, ECClassId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ONLY ts.ModelHasElements"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "non-polymorphic SELECT against abstract rel class should return 0 rows";
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId, ECClassId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ts.ModelHasGeometric3dElements"));
    rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ASSERT_EQ(modelKeys[0].GetECClassId().GetValue(), stmt.GetValueId<ECInstanceId>(2).GetValue()) << "SourceECInstanceId";
        rowCount++;
        }
    ASSERT_EQ(6, rowCount) << stmt.GetECSql();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId, ECClassId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ts.ModelHasLinkElements"));
    rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ASSERT_EQ(modelKeys[0].GetECClassId().GetValue(), stmt.GetValueId<ECInstanceId>(2).GetValue()) << "SourceECInstanceId";
        rowCount++;
        }
    ASSERT_EQ(4, rowCount) << stmt.GetECSql();
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
                                          "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                          "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
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
                                          "    <Source cardinality='(0,1)' polymorphic='True'>"
                                          "      <Class class='Model' />"
                                          "    </Source>"
                                          "    <Target cardinality='(0,N)' polymorphic='True'>"
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
                                          "    <Source cardinality='(0,1)' polymorphic='True'>"
                                          "      <Class class='Model' />"
                                          "    </Source>"
                                          "    <Target cardinality='(0,N)' polymorphic='True'>"
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
                                          "    <Source cardinality='(0,1)' polymorphic='True'>"
                                          "      <Class class='Model' />"
                                          "    </Source>"
                                          "    <Target cardinality='(0,N)' polymorphic='True'>"
                                          "      <Class class='PhysicalElement' />"
                                          "    </Target>"
                                          "  </ECRelationshipClass>"
                                          "</ECSchema>", true, "Subclass can have ClassMap CA for FK mapping if MapStrategy is set to NotMapped"),
                               "validrelinheritance.ecdb");

            MapStrategyInfo mapStrategy;
            ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetECClass("Test", "ModelHasElements")->GetId()));
            ASSERT_EQ(MapStrategyInfo::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);

            ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetECClass("Test", "ModelHasPhysicalElements")->GetId()));
            ASSERT_EQ(MapStrategyInfo::Strategy::NotMapped, mapStrategy.m_strategy);

            ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetECClass("Test", "ModelHasPhysicalElements2")->GetId()));
            ASSERT_EQ(MapStrategyInfo::Strategy::NotMapped, mapStrategy.m_strategy);
            }

            {
            ECDb ecdb;
            bool asserted = false;
            AssertSchemaImport(ecdb, asserted,
                               SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                          "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                          "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
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
                                          "    <Source cardinality='(0,1)' polymorphic='True'>"
                                          "      <Class class='Model' />"
                                          "    </Source>"
                                          "    <Target cardinality='(0,N)' polymorphic='True'>"
                                          "      <Class class='Element' />"
                                          "    </Target>"
                                          "  </ECRelationshipClass>"
                                          "  <ECRelationshipClass typeName='ModelHasPhysicalElements' strength='embedding' modifier='Sealed'>"
                                          "   <BaseClass>ModelHasElements</BaseClass>"
                                          "    <Source cardinality='(0,1)' polymorphic='True'>"
                                          "      <Class class='Model' />"
                                          "    </Source>"
                                          "    <Target cardinality='(0,N)' polymorphic='True'>"
                                          "      <Class class='Element' />"
                                          "    </Target>"
                                          "  </ECRelationshipClass>"
                                          "  <ECRelationshipClass typeName='ModelHasPhysicalElements2' strength='embedding' modifier='Sealed'>"
                                          "   <BaseClass>ModelHasElements</BaseClass>"
                                          "    <Source cardinality='(0,1)' polymorphic='True'>"
                                          "      <Class class='Model' />"
                                          "    </Source>"
                                          "    <Target cardinality='(0,N)' polymorphic='True'>"
                                          "      <Class class='Element' />"
                                          "    </Target>"
                                          "  </ECRelationshipClass>"
                                          "</ECSchema>"),
                               "validrelinheritance.ecdb");

            MapStrategyInfo mapStrategy;
            ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetECClass("Test", "ModelHasElements")->GetId()));
            ASSERT_EQ(MapStrategyInfo::Strategy::NotMapped, mapStrategy.m_strategy);

            ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetECClass("Test", "ModelHasPhysicalElements")->GetId()));
            ASSERT_EQ(MapStrategyInfo::Strategy::NotMapped, mapStrategy.m_strategy);

            ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetECClass("Test", "ModelHasPhysicalElements2")->GetId()));
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
    SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
    "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
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
    "    <Source cardinality='(1,1)' polymorphic='True'>"
    "      <Class class='Model' />"
    "    </Source>"
    "    <Target cardinality='(0,N)' polymorphic='True'>"
    "      <Class class='Element' />"
    "    </Target>"
    "  </ECRelationshipClass>"
    "  <ECRelationshipClass typeName='ModelHasPhysicalElements' strength='referencing' modifier='Sealed'>"
    "   <BaseClass>ModelHasElements</BaseClass>"
    "    <Source cardinality='(0,N)' polymorphic='True'>"
    "      <Class class='Model' />"
    "    </Source>"
    "    <Target cardinality='(0,N)' polymorphic='True'>"
    "      <Class class='PhysicalElement' />"
    "    </Target>"
    "  </ECRelationshipClass>"
    "</ECSchema>", false, "Subclass must not imply link table if base class has FK mapping"));

    testSchemas.push_back(
    SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
    "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
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
    "    <Source cardinality='(1,1)' polymorphic='True'>"
    "      <Class class='Model' />"
    "    </Source>"
    "    <Target cardinality='(0,N)' polymorphic='True'>"
    "      <Class class='Element' />"
    "    </Target>"
    "  </ECRelationshipClass>"
    "  <ECRelationshipClass typeName='ModelHasPhysicalElements' strength='embedding' modifier='Sealed'>"
    "    <ECCustomAttributes>"
    "        <ForeignKeyRelationshipMap xmlns='ECDbMap.02.00'/>"
    "    </ECCustomAttributes>"
    "   <BaseClass>ModelHasElements</BaseClass>"
    "    <Source cardinality='(1,1)' polymorphic='True'>"
    "      <Class class='Model' />"
    "    </Source>"
    "    <Target cardinality='(0,N)' polymorphic='True'>"
    "      <Class class='PhysicalElement' />"
    "    </Target>"
    "  </ECRelationshipClass>"
    "</ECSchema>", false, "Subclasses must not have ForeignKeyRelationshipMap even if it doesn't violate the base class mapping"));

    testSchemas.push_back(
    SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
    "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
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
    "    <Source cardinality='(1,N)' polymorphic='True'>"
    "      <Class class='Model' />"
    "    </Source>"
    "    <Target cardinality='(0,N)' polymorphic='True'>"
    "      <Class class='Element' />"
    "    </Target>"
    "  </ECRelationshipClass>"
    "  <ECRelationshipClass typeName='ModelHasPhysicalElements' strength='referencing' modifier='Sealed'>"
    "    <ECCustomAttributes>"
    "        <LinkTableRelationshipMap xmlns='ECDbMap.02.00'/>"
    "    </ECCustomAttributes>"
    "   <BaseClass>ModelHasElements</BaseClass>"
    "    <Source cardinality='(1,N)' polymorphic='True'>"
    "      <Class class='Model' />"
    "    </Source>"
    "    <Target cardinality='(0,N)' polymorphic='True'>"
    "      <Class class='PhysicalElement' />"
    "    </Target>"
    "  </ECRelationshipClass>"
    "</ECSchema>", false, "Subclasses must not have LinkTableRelationshipMap even if it doesn't violate the base class mapping"));

    testSchemas.push_back(
    SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
    "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
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
    "    <Source cardinality='(0,N)' polymorphic='True'>"
    "      <Class class='Model' />"
    "    </Source>"
    "    <Target cardinality='(0,N)' polymorphic='True'>"
    "      <Class class='Element' />"
    "    </Target>"
    "  </ECRelationshipClass>"
    "  <ECRelationshipClass typeName='ModelHasPhysicalElements' strength='referencing' modifier='Sealed'>"
    "    <ECCustomAttributes>"
    "        <ForeignKeyRelationshipMap xmlns='ECDbMap.02.00'/>"
    "    </ECCustomAttributes>"
    "   <BaseClass>ModelHasElements</BaseClass>"
    "    <Source cardinality='(0,1)' polymorphic='True'>"
    "      <Class class='Model' />"
    "    </Source>"
    "    <Target cardinality='(0,N)' polymorphic='True'>"
    "      <Class class='PhysicalElement' />"
    "    </Target>"
    "  </ECRelationshipClass>"
    "</ECSchema>", false, "Subclass must not have ForeignKeyRelationshipMap if base class maps to link table"));

    testSchemas.push_back(
        SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                   "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
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
                   "    <Source cardinality='(0,1)' polymorphic='True'>"
                   "      <Class class='Model' />"
                   "    </Source>"
                   "    <Target cardinality='(0,N)' polymorphic='True'>"
                   "      <Class class='Element' />"
                   "    </Target>"
                   "  </ECRelationshipClass>"
                   "  <ECRelationshipClass typeName='ModelHasPhysicalElements' strength='embedding' modifier='Sealed'>"
                   "    <ECCustomAttributes>"
                   "        <ForeignKeyRelationshipMap xmlns='ECDbMap.02.00'/>"
                   "    </ECCustomAttributes>"
                   "   <BaseClass>ModelHasElements</BaseClass>"
                   "    <Source cardinality='(0,1)' polymorphic='True'>"
                   "      <Class class='Model' />"
                   "    </Source>"
                   "    <Target cardinality='(0,N)' polymorphic='True'>"
                   "      <Class class='PhysicalElement' />"
                   "    </Target>"
                   "  </ECRelationshipClass>"
                   "</ECSchema>", false, "Subclass must not have ForeignKeyRelationshipMap even if it doesn't violate the mapping type"));

    testSchemas.push_back(
    SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
    "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
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
    "    <Source cardinality='(0,1)' polymorphic='True'>"
    "      <Class class='Model' />"
    "    </Source>"
    "    <Target cardinality='(0,N)' polymorphic='True'>"
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
    "    <Source cardinality='(0,1)' polymorphic='True'>"
    "      <Class class='Model' />"
    "    </Source>"
    "    <Target cardinality='(0,N)' polymorphic='True'>"
    "      <Class class='PhysicalElement' />"
    "    </Target>"
    "  </ECRelationshipClass>"
    "</ECSchema>", false, "Subclass must not add ECProperties if base class has FK mapping"));
    
    testSchemas.push_back(
        SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                   "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
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
                   "    <Source cardinality='(0,1)' polymorphic='True'>"
                   "      <Class class='Model' />"
                   "    </Source>"
                   "    <Target cardinality='(0,N)' polymorphic='True'>"
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
                   "    <Source cardinality='(0,1)' polymorphic='True'>"
                   "      <Class class='Model' />"
                   "    </Source>"
                   "    <Target cardinality='(0,N)' polymorphic='True'>"
                   "      <Class class='PhysicalElement' />"
                   "    </Target>"
                   "  </ECRelationshipClass>"
                   "</ECSchema>", false, "Subclass must not have ClassMap CA if base class has FK mapping"));


    testSchemas.push_back(
        SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                   "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
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
                   "    <Source cardinality='(0,1)' polymorphic='True'>"
                   "      <Class class='Model' />"
                   "    </Source>"
                   "    <Target cardinality='(0,N)' polymorphic='True'>"
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
                   "    <Source cardinality='(0,1)' polymorphic='True'>"
                   "      <Class class='Model' />"
                   "    </Source>"
                   "    <Target cardinality='(0,N)' polymorphic='True'>"
                   "      <Class class='PhysicalElement' />"
                   "    </Target>"
                   "  </ECRelationshipClass>"
                   "  <ECRelationshipClass typeName='ModelHasPhysicalElements2' strength='embedding' modifier='Sealed'>"
                   "   <BaseClass>ModelHasElements</BaseClass>"
                   "    <Source cardinality='(0,1)' polymorphic='True'>"
                   "      <Class class='Model' />"
                   "    </Source>"
                   "    <Target cardinality='(0,N)' polymorphic='True'>"
                   "      <Class class='PhysicalElement' />"
                   "    </Target>"
                   "  </ECRelationshipClass>"
                   "</ECSchema>", false, "Subclass must not have define NotMapped if base class did already"));


/*    testSchemas.push_back(
        SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                   "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
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
                   "    <Source cardinality='(0,1)' polymorphic='True'>"
                   "      <Class class='Model' />"
                   "    </Source>"
                   "    <Target cardinality='(0,N)' polymorphic='True'>"
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
        const int relClassIdColumnKind = 2048;
        CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT sql FROM sqlite_master WHERE name=? AND type='table'");
        ASSERT_TRUE(stmt != nullptr);
        ASSERT_EQ(BE_SQLITE_OK, stmt->BindText(1, tableName, Statement::MakeCopy::No));
        ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
        Utf8String ddl(stmt->GetValueText(0));
        stmt = nullptr;

        stmt = ecdb.GetCachedStatement("SELECT c.ColumnKind, c.IsVirtual, c.NotNullConstraint FROM ec_Column c, ec_Table t WHERE c.TableId=t.Id AND t.Name=? and c.Name=?");
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
    #define NAMESPACEPREFIX "ts1"

    SchemaItem testSchema("<ECSchema schemaName='TestSchema' nameSpacePrefix='" NAMESPACEPREFIX "' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                          "  <ECEntityClass typeName='A' >"
                          "    <ECProperty propertyName='Name' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='B' >"
                          "    <ECProperty propertyName='Code' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECRelationshipClass typeName='AHasB' modifier='Abstract' strength='embedding'>"
                          "    <Source cardinality='(0,1)' polymorphic='True'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target cardinality='(0,N)' polymorphic='True'>"
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
                          "    <Source cardinality='(0,1)' polymorphic='True'>"
                          "      <Class class='C' />"
                          "    </Source>"
                          "    <Target cardinality='(0,N)' polymorphic='True'>"
                          "      <Class class='D' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "</ECSchema>");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testSchema, "relecclassid" NAMESPACEPREFIX ".ecdb");
    ASSERT_FALSE(asserted);

    assertRelECClassId(ecdb, NAMESPACEPREFIX "_B", "RelECClassId_" NAMESPACEPREFIX "_AHasB", RelClassIdExistenceMode::Persisted, false);
    assertRelECClassId(ecdb, NAMESPACEPREFIX "_D", "RelECClassId_" NAMESPACEPREFIX "_CHasD", RelClassIdExistenceMode::Virtual, false);
    }

    {
    #undef NAMESPACEPREFIX
    #define NAMESPACEPREFIX "ts2"

    SchemaItem testSchema("<ECSchema schemaName='TestSchema' nameSpacePrefix='" NAMESPACEPREFIX "' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                          "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
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
                          "    <Source cardinality='(0,1)' polymorphic='True'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target cardinality='(0,N)' polymorphic='True'>"
                          "      <Class class='B' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "  <ECRelationshipClass typeName='AHasB1N' modifier='Abstract' strength='embedding'>"
                          "    <Source cardinality='(1,1)' polymorphic='True'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target cardinality='(0,N)' polymorphic='True'>"
                          "      <Class class='B' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "</ECSchema>");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testSchema, "relecclassid" NAMESPACEPREFIX ".ecdb");
    ASSERT_FALSE(asserted);

    assertRelECClassId(ecdb, NAMESPACEPREFIX "_B", "RelECClassId_" NAMESPACEPREFIX "_AHasB", RelClassIdExistenceMode::Persisted, false);
    assertRelECClassId(ecdb, NAMESPACEPREFIX "_B", "RelECClassId_" NAMESPACEPREFIX "_AHasB1N", RelClassIdExistenceMode::Persisted, true);
    }

    {
#undef NAMESPACEPREFIX
#define NAMESPACEPREFIX "ts3"

    SchemaItem testSchema("<ECSchema schemaName='TestSchema' nameSpacePrefix='" NAMESPACEPREFIX "' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                          "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
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
                          "    <Source cardinality='(0,1)' polymorphic='True'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target cardinality='(0,N)' polymorphic='True'>"
                          "      <Class class='B' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "  <ECRelationshipClass typeName='AHasBSub' modifier='Sealed' strength='embedding'>"
                          "    <BaseClass>AHasB</BaseClass>"
                          "    <Source cardinality='(0,1)' polymorphic='True'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target cardinality='(0,N)' polymorphic='True'>"
                          "      <Class class='BSub' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "  <ECRelationshipClass typeName='AHasB1N' modifier='Abstract' strength='embedding'>"
                          "    <Source cardinality='(1,1)' polymorphic='True'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target cardinality='(0,N)' polymorphic='True'>"
                          "      <Class class='B' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "  <ECRelationshipClass typeName='AHasB1NSub' modifier='Sealed' strength='embedding'>"
                          "    <BaseClass>AHasB1N</BaseClass>"
                          "    <Source cardinality='(1,1)' polymorphic='True'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target cardinality='(0,N)' polymorphic='True'>"
                          "      <Class class='BSub' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "</ECSchema>");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testSchema, "relecclassid" NAMESPACEPREFIX ".ecdb");
    ASSERT_FALSE(asserted);

    Utf8CP tableName = NAMESPACEPREFIX "_B";
    assertRelECClassId(ecdb, tableName, "RelECClassId_" NAMESPACEPREFIX "_AHasB", RelClassIdExistenceMode::Persisted, false);
    assertRelECClassId(ecdb, tableName, "RelECClassId_" NAMESPACEPREFIX "_AHasBSub", RelClassIdExistenceMode::DoesNotExist, false);
    assertRelECClassId(ecdb, tableName, "RelECClassId_" NAMESPACEPREFIX "_AHasB1N", RelClassIdExistenceMode::Persisted, true);
    assertRelECClassId(ecdb, tableName, "RelECClassId_" NAMESPACEPREFIX "_AHasB1NSub", RelClassIdExistenceMode::DoesNotExist, false);
    }

    {
#undef NAMESPACEPREFIX
#define NAMESPACEPREFIX "ts4"

    SchemaItem testSchema("<ECSchema schemaName='TestSchema' nameSpacePrefix='" NAMESPACEPREFIX "' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                          "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
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
                          "    <Source cardinality='(0,1)' polymorphic='True'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target cardinality='(0,N)' polymorphic='True'>"
                          "      <Class class='BSub' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "  <ECRelationshipClass typeName='AHasBSub1N' modifier='Abstract' strength='embedding'>"
                          "    <Source cardinality='(1,1)' polymorphic='True'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target cardinality='(0,N)' polymorphic='True'>"
                          "      <Class class='BSub' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "</ECSchema>");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testSchema, "relecclassid" NAMESPACEPREFIX ".ecdb");
    ASSERT_FALSE(asserted);

    assertRelECClassId(ecdb, NAMESPACEPREFIX "_B", "RelECClassId_" NAMESPACEPREFIX "_AHasBSub", RelClassIdExistenceMode::Persisted, false);
    //cardinality would imply NOT NULL on rel class id, but the column is shared by other base class rows, so no enforcement of NOT NULL.
    assertRelECClassId(ecdb, NAMESPACEPREFIX "_B", "RelECClassId_" NAMESPACEPREFIX "_AHasBSub1N", RelClassIdExistenceMode::Persisted, false);
    }

    {
#undef NAMESPACEPREFIX
#define NAMESPACEPREFIX "ts5"

    SchemaItem testSchema("<ECSchema schemaName='TestSchema' nameSpacePrefix='" NAMESPACEPREFIX "' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                          "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
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
                          "    <Source cardinality='(0,1)' polymorphic='True'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target cardinality='(0,N)' polymorphic='True'>"
                          "      <Class class='BSub' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "  <ECRelationshipClass typeName='AHasB1N' modifier='Sealed' strength='embedding'>"
                          "    <Source cardinality='(1,1)' polymorphic='True'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target cardinality='(0,N)' polymorphic='True'>"
                          "      <Class class='B' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "  <ECRelationshipClass typeName='AHasBSub1N' modifier='Sealed' strength='embedding'>"
                          "    <Source cardinality='(1,1)' polymorphic='True'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target cardinality='(0,N)' polymorphic='True'>"
                          "      <Class class='BSub' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "</ECSchema>");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testSchema, "relecclassid" NAMESPACEPREFIX ".ecdb");
    ASSERT_FALSE(asserted);

    assertRelECClassId(ecdb, NAMESPACEPREFIX "_B", "RelECClassId_" NAMESPACEPREFIX "_AHasBSub", RelClassIdExistenceMode::Virtual, false);
    assertRelECClassId(ecdb, NAMESPACEPREFIX "_B", "RelECClassId_" NAMESPACEPREFIX "_AHasB1N", RelClassIdExistenceMode::Virtual, true);
    assertRelECClassId(ecdb, NAMESPACEPREFIX "_B", "RelECClassId_" NAMESPACEPREFIX "_AHasBSub1N", RelClassIdExistenceMode::Virtual, false);
    }

    {
    //LinkTable
#undef NAMESPACEPREFIX
#define NAMESPACEPREFIX "ts6"

    SchemaItem testSchema("<ECSchema schemaName='TestSchema' nameSpacePrefix='" NAMESPACEPREFIX "' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                          "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
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
                          "    <Source cardinality='(0,N)' polymorphic='True'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target cardinality='(0,N)' polymorphic='True'>"
                          "      <Class class='B' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "</ECSchema>");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testSchema, "relecclassid" NAMESPACEPREFIX ".ecdb");
    ASSERT_FALSE(asserted);

    ASSERT_TRUE(ecdb.TableExists(NAMESPACEPREFIX "_AHasB"));
    ASSERT_FALSE(ecdb.ColumnExists(NAMESPACEPREFIX "_AHasB", "ECClassId"));
   }

    {
    //LinkTable
#undef NAMESPACEPREFIX
#define NAMESPACEPREFIX "ts7"

    SchemaItem testSchema("<ECSchema schemaName='TestSchema' nameSpacePrefix='" NAMESPACEPREFIX "' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                          "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
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
                          "    <Source cardinality='(0,N)' polymorphic='True'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target cardinality='(0,N)' polymorphic='True'>"
                          "      <Class class='B' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "  <ECRelationshipClass typeName='AHasBSub' modifier='Sealed' strength='referencing'>"
                          "    <BaseClass>AHasB</BaseClass>"
                          "    <Source cardinality='(0,N)' polymorphic='True'>"
                          "      <Class class='A' />"
                          "    </Source>"
                          "    <Target cardinality='(0,N)' polymorphic='True'>"
                          "      <Class class='B' />"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "</ECSchema>");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testSchema, "relecclassid" NAMESPACEPREFIX ".ecdb");
    ASSERT_FALSE(asserted);

    ASSERT_TRUE(ecdb.TableExists(NAMESPACEPREFIX "_AHasB"));
    ASSERT_TRUE(ecdb.ColumnExists(NAMESPACEPREFIX "_AHasB", "ECClassId"));
    }

#undef NAMESPACEPREFIX
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECRelationshipInheritanceTestFixture, InheritingAllowDuplicateRelationships)
    {
    ECDbCR ecdb = SetupECDb("ecrelinheritance_allowduplicaterelationships.ecdb", SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
               "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
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
               "    <Source cardinality='(0,N)' polymorphic='True'>"
               "      <Class class='A' />"
               "    </Source>"
               "    <Target cardinality='(0,N)' polymorphic='True'>"
               "      <Class class='B' />"
               "    </Target>"
               "  </ECRelationshipClass>"
               "  <ECRelationshipClass typeName='SubRel' modifier='Sealed' strength='referencing'>"
               "   <BaseClass>BaseRel</BaseClass>"
               "    <Source cardinality='(0,N)' polymorphic='True'>"
               "      <Class class='A' />"
               "    </Source>"
               "    <Target cardinality='(0,N)' polymorphic='True'>"
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, a.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, a.GetECClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, b.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, b.GetECClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Insert of first relationship: " << ecdb.GetLastError().c_str();
    stmt.Reset();
    stmt.ClearBindings();
    //now insert same rel again.
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, a.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, a.GetECClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, b.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, b.GetECClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Insert of duplicate relationship: " << ecdb.GetLastError().c_str();
    }

END_ECDBUNITTESTS_NAMESPACE
