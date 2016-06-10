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
                          "  <ECSchemaReference name='ECDbMap' version='01.01' prefix='ecdbmap' />"
                          "  <ECEntityClass typeName='Model' >"
                          "    <ECProperty propertyName='Name' typeName='string' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='Element' modifier='Abstract' >"
                          "    <ECCustomAttributes>"
                          "        <ClassMap xmlns='ECDbMap.01.01'>"
                          "                <MapStrategy>"
                          "                   <Strategy>SharedTable</Strategy>"
                          "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
                          "                </MapStrategy>"
                          "        </ClassMap>"
                          "    </ECCustomAttributes>"
                          "    <ECProperty propertyName='Code' typeName='string' />"
                          "    <ECNavigationProperty propertyName='ModelId' relationshipName='ModelHasElements' direction='Backward' />"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='InformationElement' modifier='Abstract'>"
                          "     <BaseClass>Element</BaseClass>"
                          "     <ECCustomAttributes>"
                          "      <ClassMap xmlns='ECDbMap.01.00'>"
                          "        <MapStrategy>"
                          "            <Options>JoinedTablePerDirectSubclass</Options>"
                          "        </MapStrategy>"
                          "       </ClassMap>"
                          "     </ECCustomAttributes>"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='LinkElement' modifier='Abstract'>"
                          "     <BaseClass>InformationElement</BaseClass>"
                          "     <ECCustomAttributes>"
                          "      <ClassMap xmlns='ECDbMap.01.00'>"
                          "        <MapStrategy>"
                          "            <Options>SharedColumnsForSubclasses</Options>"
                          "            <MinimumSharedColumnCount>8</MinimumSharedColumnCount>"
                          "        </MapStrategy>"
                          "       </ClassMap>"
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
                          "      <ClassMap xmlns='ECDbMap.01.00'>"
                          "        <MapStrategy>"
                          "            <Options>JoinedTablePerDirectSubclass</Options>"
                          "        </MapStrategy>"
                          "       </ClassMap>"
                          "     </ECCustomAttributes>"
                          "    <BaseClass>Element</BaseClass>"
                          "  </ECEntityClass>"
                          "  <ECEntityClass typeName='Geometric3dElement' modifier='Abstract'>"
                          "     <ECCustomAttributes>"
                          "      <ClassMap xmlns='ECDbMap.01.00'>"
                          "        <MapStrategy>"
                          "            <Options>SharedColumnsForSubclasses</Options>"
                          "            <MinimumSharedColumnCount>16</MinimumSharedColumnCount>"
                          "        </MapStrategy>"
                          "       </ClassMap>"
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


    std::vector<ECInstanceKey> modelKeys;
    std::vector<ECInstanceKey> urlLinkKeys;
    std::vector<ECInstanceKey> embeddedLinkKeys;
    std::vector<ECInstanceKey> volumeElKeys;
    std::vector<ECInstanceKey> annotationElKeys;

    {
    //insert test data
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbFilePath.c_str(), ECDb::OpenParams(Db::OpenMode::ReadWrite)));

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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.VolumeElement(Code, ModelId, Name) VALUES(?,?, 'Volume')"));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "VolumeElement 1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[0].GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    volumeElKeys.push_back(key);
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "VolumeElement 2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[1].GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    volumeElKeys.push_back(key);
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "VolumeElement 3", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[1].GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    volumeElKeys.push_back(key);
    stmt.Finalize();

    //AnnotationElement
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.Annotation3dElement(Code, ModelId, Font) VALUES(?,?, 'Consolas Sans Serif')"));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Annotation3dElement 1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[0].GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    annotationElKeys.push_back(key);
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Annotation3dElement 2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[1].GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    annotationElKeys.push_back(key);
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Annotation3dElement 3", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[1].GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    annotationElKeys.push_back(key);
    stmt.Finalize();

    //UrlLink
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.UrlLink(Code, ModelId, Url) VALUES(?,?, 'http://www.staufen.de')"));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "UrlLinkElement 1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[0].GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    urlLinkKeys.push_back(key);
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "UrlLinkElement 2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[0].GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    urlLinkKeys.push_back(key);
    stmt.Finalize();

    //EmbeddedLink
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.EmbeddedLink(Code, ModelId, Name) VALUES(?,?, 'bliblablub')"));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "EmbeddedLinkElement 1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[1].GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << ecdb.GetLastError().c_str();
    embeddedLinkKeys.push_back(key);
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "EmbeddedLinkElement 2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, modelKeys[1].GetECInstanceId()));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId, GetECClassId(), SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ts.ModelHasElements"));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId, GetECClassId(), SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ONLY ts.ModelHasElements"));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId, GetECClassId(), SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ts.ModelHasGeometric3dElements"));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId, GetECClassId(), SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ts.ModelHasLinkElements"));
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
// @bsimethod                                   Krischan.Eberle                  05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECRelationshipInheritanceTestFixture, InvalidCases)
    {
    std::vector<SchemaItem> testSchemas;
    testSchemas.push_back(
    SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
    "  <ECSchemaReference name='ECDbMap' version='01.01' prefix='ecdbmap' />"
    "  <ECEntityClass typeName='Model' >"
    "    <ECProperty propertyName='Name' typeName='string' />"
    "  </ECEntityClass>"
    "  <ECEntityClass typeName='Element' modifier='Abstract' >"
    "    <ECCustomAttributes>"
    "        <ClassMap xmlns='ECDbMap.01.01'>"
    "                <MapStrategy>"
    "                   <Strategy>SharedTable</Strategy>"
    "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
    "                </MapStrategy>"
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
    "  <ECSchemaReference name='ECDbMap' version='01.01' prefix='ecdbmap' />"
    "  <ECEntityClass typeName='Model' >"
    "    <ECProperty propertyName='Name' typeName='string' />"
    "  </ECEntityClass>"
    "  <ECEntityClass typeName='Element' modifier='Abstract' >"
    "    <ECCustomAttributes>"
    "        <ClassMap xmlns='ECDbMap.01.01'>"
    "                <MapStrategy>"
    "                   <Strategy>SharedTable</Strategy>"
    "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
    "                </MapStrategy>"
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
    "        <ForeignKeyRelationshipMap xmlns='ECDbMap.01.01'/>"
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
    "  <ECSchemaReference name='ECDbMap' version='01.01' prefix='ecdbmap' />"
    "  <ECEntityClass typeName='Model' >"
    "    <ECProperty propertyName='Name' typeName='string' />"
    "  </ECEntityClass>"
    "  <ECEntityClass typeName='Element' modifier='Abstract' >"
    "    <ECCustomAttributes>"
    "        <ClassMap xmlns='ECDbMap.01.01'>"
    "                <MapStrategy>"
    "                   <Strategy>SharedTable</Strategy>"
    "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
    "                </MapStrategy>"
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
    "        <LinkTableRelationshipMap xmlns='ECDbMap.01.01'/>"
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
    "  <ECSchemaReference name='ECDbMap' version='01.01' prefix='ecdbmap' />"
    "  <ECEntityClass typeName='Model' >"
    "    <ECProperty propertyName='Name' typeName='string' />"
    "  </ECEntityClass>"
    "  <ECEntityClass typeName='Element' modifier='Abstract' >"
    "    <ECCustomAttributes>"
    "        <ClassMap xmlns='ECDbMap.01.01'>"
    "                <MapStrategy>"
    "                   <Strategy>SharedTable</Strategy>"
    "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
    "                </MapStrategy>"
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
    "        <ForeignKeyRelationshipMap xmlns='ECDbMap.01.01'/>"
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
                   "  <ECSchemaReference name='ECDbMap' version='01.01' prefix='ecdbmap' />"
                   "  <ECEntityClass typeName='Model' >"
                   "    <ECProperty propertyName='Name' typeName='string' />"
                   "  </ECEntityClass>"
                   "  <ECEntityClass typeName='Element' modifier='Abstract' >"
                   "    <ECCustomAttributes>"
                   "        <ClassMap xmlns='ECDbMap.01.01'>"
                   "                <MapStrategy>"
                   "                   <Strategy>SharedTable</Strategy>"
                   "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
                   "                </MapStrategy>"
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
                   "        <ForeignKeyRelationshipMap xmlns='ECDbMap.01.01'/>"
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
    "  <ECSchemaReference name='ECDbMap' version='01.01' prefix='ecdbmap' />"
    "  <ECEntityClass typeName='Model' >"
    "    <ECProperty propertyName='Name' typeName='string' />"
    "  </ECEntityClass>"
    "  <ECEntityClass typeName='Element' modifier='Abstract' >"
    "    <ECCustomAttributes>"
    "        <ClassMap xmlns='ECDbMap.01.01'>"
    "                <MapStrategy>"
    "                   <Strategy>SharedTable</Strategy>"
    "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
    "                </MapStrategy>"
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
    "        <ClassMap xmlns='ECDbMap.01.01'>"
    "                <MapStrategy>"
    "                   <Strategy>SharedTable</Strategy>"
    "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
    "                </MapStrategy>"
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
    "  <ECSchemaReference name='ECDbMap' version='01.01' prefix='ecdbmap' />"
    "  <ECEntityClass typeName='Model' >"
    "    <ECProperty propertyName='Name' typeName='string' />"
    "  </ECEntityClass>"
    "  <ECEntityClass typeName='Element' modifier='Abstract' >"
    "    <ECCustomAttributes>"
    "        <ClassMap xmlns='ECDbMap.01.01'>"
    "            <MapStrategy>"
    "                <Strategy>SharedTable</Strategy>"
    "                <AppliesToSubclasses>True</AppliesToSubclasses>"
    "            </MapStrategy>"
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
    "        <ClassMap xmlns='ECDbMap.01.01'>"
    "            <MapStrategy>"
    "                <Strategy>SharedTable</Strategy>"
    "                <AppliesToSubclasses>True</AppliesToSubclasses>"
    "             </MapStrategy>"
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
    
/*    testSchemas.push_back(
        SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                   "  <ECSchemaReference name='ECDbMap' version='01.01' prefix='ecdbmap' />"
                   "  <ECEntityClass typeName='Model' >"
                   "    <ECProperty propertyName='Name' typeName='string' />"
                   "  </ECEntityClass>"
                   "  <ECEntityClass typeName='Element' modifier='Abstract' >"
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
                   "   <BaseClass>ModelHasElements</BaseClass>"
                   "    <Source cardinality='(0,1)' polymorphic='True'>"
                   "      <Class class='Model' />"
                   "    </Source>"
                   "    <Target cardinality='(0,N)' polymorphic='True'>"
                   "      <Class class='PhysicalElement' />"
                   "    </Target>"
                   "  </ECRelationshipClass>"
                   "</ECSchema>", false, "FK end maps to multiple primary tables"));*/

    AssertSchemaImport(testSchemas, "invalidrelinheritance.ecdb");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECRelationshipInheritanceTestFixture, InheritingAllowDuplicateRelationships)
    {
    ECDbCR ecdb = SetupECDb("ecrelinheritance_allowduplicaterelationships.ecdb", SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
               "  <ECSchemaReference name='ECDbMap' version='01.01' prefix='ecdbmap' />"
               "  <ECEntityClass typeName='A' >"
               "    <ECProperty propertyName='Name' typeName='string' />"
               "  </ECEntityClass>"
               "  <ECEntityClass typeName='B' >"
               "    <ECProperty propertyName='Code' typeName='string' />"
               "  </ECEntityClass>"
               "  <ECRelationshipClass typeName='BaseRel' modifier='Abstract' strength='referencing'>"
               "    <ECCustomAttributes>"
               "        <LinkTableRelationshipMap xmlns='ECDbMap.01.01'>"
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT GetECClassId(), ECInstanceId FROM ts.A LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    a = ECInstanceKey(stmt.GetValueId<ECClassId>(0), stmt.GetValueId<ECInstanceId>(1));
    ASSERT_TRUE(a.IsValid());
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT GetECClassId(), ECInstanceId FROM ts.B LIMIT 1"));
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
