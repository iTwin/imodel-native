/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECSqlNavigationProps_Tests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "SchemaImportTestFixture.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 01/16
//+---------------+---------------+---------------+---------------+---------------+------
struct ECSqlNavigationPropertyTestFixture : SchemaImportTestFixture
    {
    protected:
        void AssertPrepare(Utf8CP ecsql, bool expectedToSucceed, Utf8CP assertMessage) const
            {
            ECSqlStatement stmt;
            ECSqlStatus stat = stmt.Prepare(GetECDb(), ecsql);
            if (expectedToSucceed)
                ASSERT_EQ(ECSqlStatus::Success, stat) << assertMessage << " - ECSQL: " << ecsql;
            else
                ASSERT_EQ(ECSqlStatus::InvalidECSql, stat) << assertMessage << " - ECSQL: " << ecsql;
            }
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, ECSqlSupport)
    {
    SetupECDb("ecsqlnavpropsupport.ecdb",
              SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                         "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                         "    <ECEntityClass typeName='A'>"
                         "        <ECProperty propertyName='PA' typeName='int' />"
                         "    </ECEntityClass>"
                         "    <ECEntityClass typeName='B'>"
                         "        <ECProperty propertyName='PB' typeName='int' />"
                         "        <ECNavigationProperty propertyName='AId' relationshipName='AHasB' direction='Backward' />"
                         "    </ECEntityClass>"
                         "    <ECEntityClass typeName='C'>"
                         "        <ECProperty propertyName='PC' typeName='int' />"
                         "    </ECEntityClass>"
                         "    <ECEntityClass typeName='D'>"
                         "        <ECProperty propertyName='PD' typeName='int' />"
                         "    </ECEntityClass>"
                         "   <ECRelationshipClass typeName='AHasB' strength='Embedding'  modifier='Sealed'>"
                         "      <Source cardinality='(0,1)' polymorphic='False'>"
                         "          <Class class ='A' />"
                         "      </Source>"
                         "      <Target cardinality='(0,N)' polymorphic='False'>"
                         "          <Class class ='B' />"
                         "      </Target>"
                         "   </ECRelationshipClass>"
                         "   <ECRelationshipClass typeName='CHasDLinkTable' strength='Referencing'  modifier='Sealed'>"
                         "      <Source cardinality='(0,1)' polymorphic='False'>"
                         "          <Class class ='C' />"
                         "      </Source>"
                         "      <Target cardinality='(0,N)' polymorphic='False'>"
                         "          <Class class ='D' />"
                         "      </Target>"
                         "      <ECProperty propertyName='P1' typeName='int' />"
                         "    </ECRelationshipClass>"
                         "</ECSchema>"));

    ASSERT_TRUE(GetECDb().IsDbOpen());

    AssertPrepare("SELECT PB FROM ts.B WHERE ECInstanceId=?", true, "");
    AssertPrepare("SELECT PB, AId FROM ts.B WHERE ECInstanceId=?", true, "");
    AssertPrepare("SELECT * FROM ts.B WHERE ECInstanceId=?", true, "");

    AssertPrepare("SELECT PD FROM ts.D WHERE ECInstanceId=?", true, "");

    AssertPrepare("INSERT INTO ts.B (PB,AId) VALUES(123,?)", true, "NavProp with single related instance is expected to be supported.");
    AssertPrepare("INSERT INTO ts.D (PD) VALUES(123)", true, "");

    AssertPrepare("UPDATE ONLY ts.B SET AId=?", true, "Updating NavProp with end table rel is supported.");
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, RelECClassId)
    {
    ECDbCR ecdb = SetupECDb("ecsqlnavpropsupport.ecdb",
              SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                         "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                         "<ECSchemaReference name='ECDbMap' version='01.00.01' prefix='ecdbmap' />"
                         "    <ECEntityClass typeName='Model'>"
                         "        <ECProperty propertyName='Name' typeName='string' />"
                         "    </ECEntityClass>"
                         "    <ECEntityClass typeName='Element' modifier='Abstract'>"
                         "        <ECCustomAttributes>"
                         "            <ClassMap xmlns='ECDbMap.01.00'>"
                         "                <MapStrategy>"
                         "                   <Strategy>SharedTable</Strategy>"
                         "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
                         "                </MapStrategy>"
                         "            </ClassMap>"
                         "        </ECCustomAttributes>"
                         "        <ECProperty propertyName='Code' typeName='string' />"
                         "        <ECNavigationProperty propertyName='Model' relationshipName='ModelHasElement' direction='Backward' />"
                         "        <ECNavigationProperty propertyName='Parent' relationshipName='ElementOwnsChildElement' direction='Backward' />"
                         "    </ECEntityClass>"
                         "    <ECEntityClass typeName='SubElement'>"
                         "        <BaseClass>Element</BaseClass>"
                         "        <ECProperty propertyName='SubProp1' typeName='int' />"
                         "    </ECEntityClass>"
                         "   <ECRelationshipClass typeName='ModelHasElement' strength='Embedding'  modifier='Sealed'>"
                         "      <Source cardinality='(1,1)' polymorphic='False'>"
                         "          <Class class ='Model' />"
                         "      </Source>"
                         "      <Target cardinality='(0,N)' polymorphic='True'>"
                         "          <Class class ='Element' />"
                         "      </Target>"
                         "   </ECRelationshipClass>"
                         "   <ECRelationshipClass typeName='ElementOwnsChildElement' strength='Embedding'  modifier='Abstract'>"
                         "      <Source cardinality='(0,1)' polymorphic='True'>"
                         "          <Class class ='Element' />"
                         "      </Source>"
                         "      <Target cardinality='(0,N)' polymorphic='True'>"
                         "          <Class class ='Element' />"
                         "      </Target>"
                         "   </ECRelationshipClass>"
                         "   <ECRelationshipClass typeName='ElementOwnsSubElement' strength='Embedding'  modifier='Sealed'>"
                         "        <BaseClass>ElementOwnsChildElement</BaseClass>"
                         "      <Source cardinality='(0,1)' polymorphic='True'>"
                         "          <Class class ='Element' />"
                         "      </Source>"
                         "      <Target cardinality='(0,N)' polymorphic='True'>"
                         "          <Class class ='SubElement' />"
                         "      </Target>"
                         "   </ECRelationshipClass>"
                         "</ECSchema>"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ASSERT_FALSE(ecdb.ColumnExists("ts_Element", "ModelRelECClassId"));
    ASSERT_TRUE(ecdb.ColumnExists("ts_Element", "ParentRelECClassId"));
    AssertIndexExists(ecdb, "ix_ts_Element_ModelRelECClassId", false);
    AssertIndex(ecdb, "ix_ts_Element_ParentRelECClassId", false, "ts_Element", {"ParentRelECClassId"});
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, SingleInstanceNavProp_ForeignKeyMapping)
    {
    const int rowCount = 3;
    ECDbR ecdb = SetupECDb("ecsqlnavpropsupport.ecdb",
              SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                         "<ECSchema schemaName='TestSchema' nameSpacePrefix='np' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                         "<ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                         "    <ECEntityClass typeName='DgnModel'>"
                         "        <ECProperty propertyName='Name' typeName='string' />"
                         "    </ECEntityClass>"
                         "    <ECEntityClass typeName='DgnElement'>"
                         "        <ECProperty propertyName='Code' typeName='string' />"
                         "        <ECNavigationProperty propertyName='ModelId' relationshipName='ParentHasChildren' direction='Backward' />"
                         "    </ECEntityClass>"
                         "   <ECRelationshipClass typeName='ParentHasChildren' strength='Referencing'  modifier='Sealed'>"
                         "      <Source cardinality='(0,1)' polymorphic='False'>"
                         "          <Class class ='DgnModel' />"
                         "      </Source>"
                         "      <Target cardinality='(0,N)' polymorphic='False'>"
                         "          <Class class ='DgnElement' />"
                         "      </Target>"
                         "   </ECRelationshipClass>"
                         "</ECSchema>"), rowCount);

    ASSERT_TRUE(ecdb.IsDbOpen());
    ASSERT_EQ(SUCCESS, ecdb.Schemas().CreateECClassViewsInDb());

    ECInstanceKey modelKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECClassId, ECInstanceId FROM np.DgnModel LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    modelKey = ECInstanceKey(stmt.GetValueId<ECClassId>(0), stmt.GetValueId<ECInstanceId>(1));
    ASSERT_TRUE(modelKey.IsValid());
    }

    ECInstanceKey elementKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO np.DgnElement(ModelId,Code) VALUES(?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, modelKey.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, "TestCode-1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(elementKey));

    ecdb.SaveChanges();
    }

    //verify relationship was inserted
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT SourceECInstanceId,SourceECClassId FROM np.ParentHasChildren WHERE TargetECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementKey.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(modelKey.GetECInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(modelKey.GetECClassId().GetValue(), stmt.GetValueId<ECClassId>(1).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT TargetECInstanceId,TargetECClassId FROM np.ParentHasChildren WHERE SourceECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, modelKey.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(elementKey.GetECInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(elementKey.GetECClassId().GetValue(), stmt.GetValueId<ECClassId>(1).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    //select from class with navprops
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT Code FROM np.DgnElement WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementKey.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT Code, ModelId FROM np.DgnElement WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementKey.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(modelKey.GetECInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(1).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT * FROM np.DgnElement WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementKey.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    int navPropIx = -1;
    for (int i = 0; i < stmt.GetColumnCount(); i++)
        {
        if (stmt.GetColumnInfo(i).GetProperty()->GetIsNavigation())
            {
            navPropIx = i;
            break;
            }
        }
    ASSERT_TRUE(navPropIx >= 0);
    ASSERT_EQ(modelKey.GetECInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(navPropIx).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //Nav prop in where clause
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId FROM np.DgnElement WHERE ModelId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, modelKey.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(elementKey.GetECInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //with literal values
    Utf8String ecsql;
    ecsql.Sprintf("SELECT ECInstanceId FROM np.DgnElement WHERE ModelId=%llu", modelKey.GetECInstanceId().GetValue());
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(elementKey.GetECInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //Nav prop in order by clause
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT * FROM np.DgnElement ORDER BY ModelId"));

    int actualRowCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        actualRowCount++;
        }
    //one element was inserted after the setup, therefore rowCount+1 is the expected value
    ASSERT_EQ(rowCount+1, actualRowCount) << stmt.GetECSql();
    stmt.Finalize();

    //Nav prop in order by clause
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT count(*), ModelId FROM np.DgnElement GROUP BY ModelId"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    stmt.Finalize();
    }
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, SingleInstanceNavProp_ForeignKeyMapping_ECInstanceAdapter)
    {
    const int rowCount = 3;
    ECDbR ecdb = SetupECDb("ecsqlnavpropsupport.ecdb",
                           SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                      "<ECSchema schemaName='np' nameSpacePrefix='np' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                      "<ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                                      "    <ECEntityClass typeName='DgnModel'>"
                                      "        <ECProperty propertyName='Name' typeName='string' />"
                                      "    </ECEntityClass>"
                                      "    <ECEntityClass typeName='DgnElement'>"
                                      "        <ECProperty propertyName='Code' typeName='string' />"
                                      "        <ECNavigationProperty propertyName='ModelId' relationshipName='ParentHasChildren' direction='Backward' />"
                                      "    </ECEntityClass>"
                                      "   <ECRelationshipClass typeName='ParentHasChildren' strength='Referencing'  modifier='Sealed'>"
                                      "      <Source cardinality='(0,1)' polymorphic='False'>"
                                      "          <Class class ='DgnModel' />"
                                      "      </Source>"
                                      "      <Target cardinality='(0,N)' polymorphic='False'>"
                                      "          <Class class ='DgnElement' />"
                                      "      </Target>"
                                      "   </ECRelationshipClass>"
                                      "</ECSchema>"), rowCount);

    ASSERT_TRUE(ecdb.IsDbOpen());

    ECInstanceKey modelKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECClassId, ECInstanceId FROM np.DgnModel LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    modelKey = ECInstanceKey(stmt.GetValueId<ECClassId>(0), stmt.GetValueId<ECInstanceId>(1));
    ASSERT_TRUE(modelKey.IsValid());
    }

    ECClassCP elementClass = ecdb.Schemas().GetECClass("np", "DgnElement");
    ASSERT_TRUE(elementClass != nullptr);

    ECInstanceKey elementKey;
    {
    ECInstanceInserter elementInserter(ecdb, *elementClass);
    ASSERT_TRUE(elementInserter.IsValid());

    IECInstancePtr elementInst = elementClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ECValue v;
    v.SetUtf8CP("TestCode-1", true);
    ASSERT_EQ(ECObjectsStatus::Success, elementInst->SetValue("Code", v));
    v.Clear();
    v.SetLong(modelKey.GetECInstanceId().GetValue());
    ASSERT_EQ(ECObjectsStatus::Success, elementInst->SetValue("ModelId", v));

    ASSERT_EQ(SUCCESS, elementInserter.Insert(elementKey, *elementInst));
    ecdb.SaveChanges();
    }

    //verify relationship was inserted
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT SourceECInstanceId,SourceECClassId FROM np.ParentHasChildren WHERE TargetECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementKey.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(modelKey.GetECInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(modelKey.GetECClassId().GetValue(), stmt.GetValueId<ECClassId>(1).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT TargetECInstanceId,TargetECClassId FROM np.ParentHasChildren WHERE SourceECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, modelKey.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(elementKey.GetECInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(elementKey.GetECClassId().GetValue(), stmt.GetValueId<ECClassId>(1).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    ECSqlStatement selStmt;
    ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(ecdb, "SELECT ECInstanceId, ECClassId, Code, ModelId FROM np.DgnElement"));

    ECInstanceECSqlSelectAdapter selAdapter(selStmt);
    bool verifiedElementWithSetNavProp = false;
    while (selStmt.Step() == BE_SQLITE_ROW)
        {
        IECInstancePtr inst = selAdapter.GetInstance();
        ECInstanceId id;
        ASSERT_EQ(SUCCESS, ECInstanceId::FromString(id, inst->GetInstanceId().c_str()));
        if (elementKey.GetECInstanceId() == id)
            {
            verifiedElementWithSetNavProp = true;

            ASSERT_EQ(modelKey.GetECInstanceId().GetValue(), selStmt.GetValueInt64(3)) << "ModelId via plain ECSQL";
            ECValue v;
            ASSERT_EQ(ECObjectsStatus::Success, inst->GetValue(v, "ModelId"));
            ASSERT_FALSE(v.IsNull()) << "ModelId is not expected to be null in the read ECInstance";
            ASSERT_EQ(modelKey.GetECInstanceId().GetValue(), v.GetLong());
            }
        else
            {
            ASSERT_TRUE(selStmt.IsValueNull(3)) << "ModelId via plain ECSQL";

            ECValue v;
            ASSERT_EQ(ECObjectsStatus::Success, inst->GetValue(v, "ModelId"));
            ASSERT_TRUE(v.IsNull());
            }
        }

    ASSERT_TRUE(verifiedElementWithSetNavProp);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, SingleInstanceNavProp_ForeignKeyMapping_JsonAdapter)
    {
    const int rowCount = 3;
    ECDbR ecdb = SetupECDb("ecsqlnavpropsupport.ecdb",
                            SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                        "<ECSchema schemaName='np' nameSpacePrefix='np' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                        "<ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                                        "    <ECEntityClass typeName='DgnModel'>"
                                        "        <ECProperty propertyName='Name' typeName='string' />"
                                        "    </ECEntityClass>"
                                        "    <ECEntityClass typeName='DgnElement'>"
                                        "        <ECProperty propertyName='Code' typeName='string' />"
                                        "        <ECNavigationProperty propertyName='ModelId' relationshipName='ParentHasChildren' direction='Backward' />"
                                        "    </ECEntityClass>"
                                        "   <ECRelationshipClass typeName='ParentHasChildren' strength='Referencing'  modifier='Sealed'>"
                                        "      <Source cardinality='(0,1)' polymorphic='False'>"
                                        "          <Class class ='DgnModel' />"
                                        "      </Source>"
                                        "      <Target cardinality='(0,N)' polymorphic='False'>"
                                        "          <Class class ='DgnElement' />"
                                        "      </Target>"
                                        "   </ECRelationshipClass>"
                                        "</ECSchema>"), rowCount);

    ASSERT_TRUE(ecdb.IsDbOpen());

    ECInstanceKey modelKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECClassId, ECInstanceId FROM np.DgnModel LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    modelKey = ECInstanceKey(stmt.GetValueId<ECClassId>(0), stmt.GetValueId<ECInstanceId>(1));
    ASSERT_TRUE(modelKey.IsValid());
    }

    ECClassCP elementClass = ecdb.Schemas().GetECClass("np", "DgnElement");
    ASSERT_TRUE(elementClass != nullptr);

    ECInstanceKey elementKey;
    {
    JsonInserter elementInserter(ecdb, *elementClass);
    ASSERT_TRUE(elementInserter.IsValid());

    Utf8String newElementJsonStr;
    newElementJsonStr.Sprintf("{\"Code\": \"TestCode-1\", \"ModelId\": \"%llu\"}", modelKey.GetECInstanceId().GetValue());

    rapidjson::Document newElementJson;
    ASSERT_FALSE(newElementJson.Parse<0>(newElementJsonStr.c_str()).HasParseError());

    ASSERT_EQ(SUCCESS, elementInserter.Insert(elementKey, newElementJson));
    ecdb.SaveChanges();
    }
    //verify relationship was inserted
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT SourceECInstanceId,SourceECClassId FROM np.ParentHasChildren WHERE TargetECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementKey.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(modelKey.GetECInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(modelKey.GetECClassId().GetValue(), stmt.GetValueId<ECClassId>(1).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT TargetECInstanceId,TargetECClassId FROM np.ParentHasChildren WHERE SourceECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, modelKey.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(elementKey.GetECInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(elementKey.GetECClassId().GetValue(), stmt.GetValueId<ECClassId>(1).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    ECSqlStatement selStmt;
    ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(ecdb, "SELECT ECInstanceId, Code, ModelId FROM np.DgnElement"));

    JsonECSqlSelectAdapter selAdapter(selStmt);
    bool verifiedElementWithSetNavProp = false;
    while (selStmt.Step() == BE_SQLITE_ROW)
        {
        Json::Value json;
        ASSERT_TRUE(selAdapter.GetRowInstance(json));

        Utf8CP idStr = json["$ECInstanceId"].asCString();
        ECInstanceId id;
        ASSERT_EQ(SUCCESS, ECInstanceId::FromString(id, idStr));
        if (elementKey.GetECInstanceId() == id)
            {
            verifiedElementWithSetNavProp = true;

            ASSERT_EQ(modelKey.GetECInstanceId().GetValue(), selStmt.GetValueInt64(2)) << "ModelId via plain ECSQL";

            Json::Value const& modelIdJson = json["ModelId"];
            ASSERT_FALSE(modelIdJson.isNull()) << "ModelId is not expected to be null in the read ECInstance";
            
            ASSERT_EQ(modelKey.GetECInstanceId().GetValue(), BeJsonUtilities::Int64FromValue(modelIdJson));
            }
        else
            {
            ASSERT_TRUE(selStmt.IsValueNull(2)) << "ModelId via plain ECSQL";
            ASSERT_TRUE(json["ModelId"].isNull()) << "ModelId is not expected to be null in the read ECInstance";
            }
        }

    ASSERT_TRUE(verifiedElementWithSetNavProp);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, SingleInstanceNavProp_ForeignKeyMappingWithJoinedTable)
    {
    ECDbR ecdb = SetupECDb("ecsqlnavpropsupport_joinedtable.ecdb",
                           SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                      "<ECSchema schemaName='TestSchema' nameSpacePrefix='np' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                      "<ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                                      "    <ECEntityClass typeName='DgnCategory'>"
                                      "        <ECProperty propertyName='Name' typeName='string' />"
                                      "    </ECEntityClass>"
                                      "    <ECEntityClass typeName='GeometrySource' modifier='Abstract'>"
                                      "        <ECProperty propertyName='Geometry' typeName='binary' />"
                                      "        <ECNavigationProperty propertyName='CategoryId' relationshipName='GeometryIsInsCategory' direction='Forward' />"
                                      "    </ECEntityClass>"
                                      "    <ECEntityClass typeName='GeometrySource3d' modifier='Abstract'>"
                                      "       <BaseClass>GeometrySource</BaseClass>"
                                      "    </ECEntityClass>"
                                      "    <ECEntityClass typeName='Element' modifier='Abstract'>"
                                      "     <ECCustomAttributes>"
                                      "         <ClassMap xmlns='ECDbMap.01.00'>"
                                      "             <MapStrategy>"
                                      "                 <Strategy>SharedTable</Strategy>"
                                      "                 <AppliesToSubclasses>True</AppliesToSubclasses>"
                                      "                 <Options>JoinedTablePerDirectSubclass</Options>"
                                      "             </MapStrategy>"
                                      "         </ClassMap>"
                                      "     </ECCustomAttributes>"
                                      "      <ECProperty propertyName='Code' typeName='string' />"
                                      "    </ECEntityClass>"
                                      "    <ECEntityClass typeName='SpatialElement' modifier='Abstract'>"
                                      "       <BaseClass>Element</BaseClass>"
                                      "       <BaseClass>GeometrySource3d</BaseClass>"
                                      "    </ECEntityClass>"
                                      "    <ECEntityClass typeName='AnnotationElement'>"
                                      "       <BaseClass>Element</BaseClass>"
                                      "       <BaseClass>GeometrySource3d</BaseClass>"
                                      "       <ECProperty propertyName='Text' typeName='string' />"
                                      "    </ECEntityClass>"
                                      "    <ECEntityClass typeName='PhysicalElement'>"
                                      "     <ECCustomAttributes>"
                                      "         <ClassMap xmlns='ECDbMap.01.00'>"
                                      "             <MapStrategy>"
                                      "                 <Options>SharedColumnsForSubclasses</Options>"
                                      "             </MapStrategy>"
                                      "         </ClassMap>"
                                      "     </ECCustomAttributes>"
                                      "       <BaseClass>SpatialElement</BaseClass>"
                                      "    </ECEntityClass>"
                                      "    <ECEntityClass typeName='FooElement'>"
                                      "       <BaseClass>PhysicalElement</BaseClass>"
                                      "        <ECProperty propertyName='Diameter' typeName='double' />"
                                      "    </ECEntityClass>"
                                      "    <ECEntityClass typeName='SystemElement' modifier='Abstract'>"
                                      "       <BaseClass>Element</BaseClass>"
                                      "    </ECEntityClass>"
                                      "    <ECEntityClass typeName='DictionaryElement'>"
                                      "       <BaseClass>Element</BaseClass>"
                                      "    </ECEntityClass>"
                                      "   <ECRelationshipClass typeName='GeometryIsInsCategory' strength='Referencing' modifier='Sealed'>"
                                      "      <Source cardinality='(0,N)' polymorphic='True'>"
                                      "          <Class class ='GeometrySource' />"
                                      "      </Source>"
                                      "      <Target cardinality='(1,1)' polymorphic='False'>"
                                      "          <Class class ='DgnCategory' />"
                                      "      </Target>"
                                      "   </ECRelationshipClass>"
                                      "</ECSchema>"), 0);

    ASSERT_TRUE(ecdb.IsDbOpen());

    ECInstanceKey catKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO np.DgnCategory(Name) VALUES('Main')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(catKey));
    ASSERT_TRUE(catKey.IsValid());
    }

    ECInstanceKey fooKey;
    double fooDiameter = 1.1;
    Utf8CP fooCode = "Foo-1";

    //INSERT
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO np.FooElement(Diameter, Code, CategoryId) VALUES(?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(1, fooDiameter));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, fooCode, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, catKey.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(fooKey));
    }

    //Verify via SELECT
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId, Code, CategoryId, Diameter FROM np.FooElement"));
    int rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        rowCount++;

        ECInstanceId currentId = stmt.GetValueId<ECInstanceId>(0);
        if (currentId == fooKey.GetECInstanceId())
            {
            ASSERT_STREQ(fooCode, stmt.GetValueText(1));
            ASSERT_EQ(catKey.GetECInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(2).GetValue());
            ASSERT_EQ(fooDiameter, stmt.GetValueDouble(3));
            }
        else
            ASSERT_TRUE(stmt.IsValueNull(2));
        }

    ASSERT_GT(rowCount, 0);
    stmt.Finalize();

    //select via base class
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId, ECClassId, CategoryId FROM np.GeometrySource"));
    rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        rowCount++;

        ECInstanceId currentId = stmt.GetValueId<ECInstanceId>(0);
        if (currentId == fooKey.GetECInstanceId())
            {
            ASSERT_EQ(fooKey.GetECClassId().GetValue(), stmt.GetValueId<ECClassId>(1).GetValue());
            ASSERT_EQ(catKey.GetECInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(2).GetValue());
            }
        else
            ASSERT_TRUE(stmt.IsValueNull(2));
        }

    ASSERT_GT(rowCount, 0);
    }
    ecdb.SaveChanges();
    
    //UPDATE CategoryId
    {
    //UPDATE via GeometrySource is not expected to work as it is mapped to multiple joined tables
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(ecdb, "UPDATE np.GeometrySource SET CategoryId=? WHERE CategoryId IS NULL")) << "ECSQL UPDATE on that kind of class not supported as it results in multiple UPDATES";
    stmt.Finalize();

    //UPDATE via classes that is mapped to a single joined table, is expected to work
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "UPDATE np.SpatialElement SET CategoryId=? WHERE CategoryId IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, catKey.GetECInstanceId())) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "UPDATE np.AnnotationElement SET CategoryId=? WHERE CategoryId IS NULL"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, catKey.GetECInstanceId())) << stmt.GetECSql();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    }

    //Verify via SELECT
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT CategoryId FROM np.FooElement"));
    int rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        rowCount++;
        ASSERT_FALSE(stmt.IsValueNull(0));
        ASSERT_EQ(catKey.GetECInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
        }

    ASSERT_GT(rowCount, 0);
    stmt.Finalize();

    //select via base class
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT CategoryId FROM np.GeometrySource"));
    rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        rowCount++;
        ASSERT_FALSE(stmt.IsValueNull(0));
        ASSERT_EQ(catKey.GetECInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
        }

    ASSERT_GT(rowCount, 0);
    stmt.Finalize();
    }
    }

END_ECDBUNITTESTS_NAMESPACE

