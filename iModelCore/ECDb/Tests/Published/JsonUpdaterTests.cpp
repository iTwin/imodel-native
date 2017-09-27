/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/JsonUpdaterTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

struct JsonUpdaterTests : ECDbTestFixture {};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle     09/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonUpdaterTests, ValidInput)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("jsonupdater_validinput.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                                    <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                                        <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA" />
                                                        <ECEntityClass typeName="Parent" >
                                                            <ECProperty propertyName="Code" typeName="int" />
                                                        </ECEntityClass>
                                                        <ECEntityClass typeName="Child" >
                                                            <ECProperty propertyName="Name" typeName="string" />
                                                            <ECNavigationProperty propertyName="Parent" relationshipName="FkRel" direction="Backward" />
                                                        </ECEntityClass>
                                                        <ECRelationshipClass typeName="FkRel" strength="referencing" modifier="None">
                                                            <Source multiplicity="(0..1)" roleLabel="has" polymorphic="False"><Class class="Parent"/></Source>
                                                            <Target multiplicity="(0..*)" roleLabel="referenced by" polymorphic="False"><Class class="Child"/></Target>
                                                        </ECRelationshipClass>
                                                        <ECRelationshipClass typeName="LinkTableRel" strength="referencing" modifier="None">
                                                            <Source multiplicity="(0..*)" roleLabel="has" polymorphic="False"><Class class="Parent"/></Source>
                                                            <Target multiplicity="(0..*)" roleLabel="referenced by" polymorphic="False"><Class class="Child"/></Target>
                                                            <ECProperty propertyName="Order" typeName="int" />
                                                            <ECProperty propertyName="Something" typeName="int" />
                                                        </ECRelationshipClass>
                                                        <ECEntityClass typeName="Empty" >
                                                        </ECEntityClass>
                                                        <ECEntityClass typeName="ClassWithOnlyLastModProp" >
                                                            <ECCustomAttributes>
                                                                <ClassHasCurrentTimeStampProperty xmlns="CoreCustomAttributes.01.00">
                                                                    <PropertyName>LastMod</PropertyName>
                                                                </ClassHasCurrentTimeStampProperty>
                                                            </ECCustomAttributes>                                                            
                                                            <ECProperty propertyName="LastMod" typeName="dateTime" />
                                                        </ECEntityClass>
                                                    </ECSchema>)xml")));

    ECClassCP testClass = m_ecdb.Schemas().GetClass("TestSchema", "Parent");
    ASSERT_TRUE(testClass != nullptr);

    {
    JsonUpdater updater(m_ecdb, *testClass, nullptr);
    EXPECT_TRUE(updater.IsValid()) << testClass->GetFullName();
    }

    {
    bvector<Utf8CP> propNames;
    JsonUpdater updater(m_ecdb, *testClass, propNames, nullptr);
    EXPECT_FALSE(updater.IsValid()) << "Empty prop names arg";
    }

    {
    bvector<Utf8CP> propNames;
    propNames.push_back("Code");
    JsonUpdater updater(m_ecdb, *testClass, propNames, nullptr);
    EXPECT_TRUE(updater.IsValid()) << testClass->GetFullName() << " with prop name list";
    }

    {
    bvector<Utf8CP> propNames;
    propNames.push_back("code");
    JsonUpdater updater(m_ecdb, *testClass, propNames, nullptr);
    EXPECT_TRUE(updater.IsValid()) << testClass->GetFullName() << " with camel-cased prop name list";
    }

    ECClassCP emptyClass = m_ecdb.Schemas().GetClass("TestSchema", "Empty");
    EXPECT_TRUE(emptyClass != nullptr);

    {
    JsonUpdater updater(m_ecdb, *emptyClass, nullptr);
    EXPECT_FALSE(updater.IsValid()) << emptyClass->GetFullName();
    }

    ECClassCP lastModClass = m_ecdb.Schemas().GetClass("TestSchema", "ClassWithOnlyLastModProp");
    ASSERT_TRUE(lastModClass != nullptr);

    {
    JsonUpdater updater(m_ecdb, *lastModClass, nullptr);
    EXPECT_FALSE(updater.IsValid()) << lastModClass->GetFullName() << " Last mod prop is not updatable";
    }

    {
    bvector<Utf8CP> propNames;
    propNames.push_back("LastMod");
    JsonUpdater updater(m_ecdb, *lastModClass, propNames, nullptr);
    EXPECT_FALSE(updater.IsValid()) << lastModClass->GetFullName() << " with {'LastMod'}";
    }

    ECClassCP fkRelClass = m_ecdb.Schemas().GetClass("TestSchema", "FkRel");
    ASSERT_TRUE(fkRelClass != nullptr);

    {
    JsonUpdater updater(m_ecdb, *fkRelClass, nullptr);
    EXPECT_FALSE(updater.IsValid()) << fkRelClass->GetFullName();
    }

    {
    bvector<Utf8CP> propNames;
    propNames.push_back("SourceECInstanceId");
    JsonUpdater updater(m_ecdb, *fkRelClass, propNames, nullptr);
    EXPECT_FALSE(updater.IsValid()) << fkRelClass->GetFullName() << " with {'SourceECInstanceId'}";
    }

    ECClassCP linkTableRelClass = m_ecdb.Schemas().GetClass("TestSchema", "LinkTableRel");
    ASSERT_TRUE(linkTableRelClass != nullptr);

    {
    JsonUpdater updater(m_ecdb, *linkTableRelClass, nullptr);
    EXPECT_TRUE(updater.IsValid()) << linkTableRelClass->GetFullName();
    }

    {
    bvector<Utf8CP> propNames;
    propNames.push_back("Order");
    JsonUpdater updater(m_ecdb, *linkTableRelClass, propNames, nullptr);
    EXPECT_TRUE(updater.IsValid()) << linkTableRelClass->GetFullName() << " with {'Order'}";
    }

    {
    bvector<Utf8CP> propNames;
    propNames.push_back("Something");
    JsonUpdater updater(m_ecdb, *linkTableRelClass, propNames, nullptr);
    EXPECT_TRUE(updater.IsValid()) << linkTableRelClass->GetFullName() << " with {'Something'}";
    }

    {
    bvector<Utf8CP> propNames;
    propNames.push_back("Something");
    propNames.push_back("Order");
    JsonUpdater updater(m_ecdb, *linkTableRelClass, propNames, nullptr);
    EXPECT_TRUE(updater.IsValid()) << linkTableRelClass->GetFullName() << " with {'Something', 'Order'}";
    }

    {
    bvector<Utf8CP> propNames;
    propNames.push_back("Order");
    propNames.push_back("SourceECInstanceId");
    JsonUpdater updater(m_ecdb, *linkTableRelClass, propNames, nullptr);
    EXPECT_FALSE(updater.IsValid()) << linkTableRelClass->GetFullName() << " with {'Order','SourceECInstanceId'}";
    }

    {
    bvector<Utf8CP> propNames;
    propNames.push_back("SourceECInstanceId");
    JsonUpdater updater(m_ecdb, *linkTableRelClass, propNames, nullptr);
    EXPECT_FALSE(updater.IsValid()) << linkTableRelClass->GetFullName() << " with {'SourceECInstanceId'}";
    }

    {
    bvector<Utf8CP> propNames;
    propNames.push_back("SourceECClassId");
    JsonUpdater updater(m_ecdb, *linkTableRelClass, propNames, nullptr);
    EXPECT_FALSE(updater.IsValid()) << linkTableRelClass->GetFullName() << " with {'SourceECClassId'}";
    }

    {
    bvector<Utf8CP> propNames;
    propNames.push_back("TargetECInstanceId");
    JsonUpdater updater(m_ecdb, *linkTableRelClass, propNames, nullptr);
    EXPECT_FALSE(updater.IsValid()) << linkTableRelClass->GetFullName() << " with {'TargetECInstanceId'}";
    }

    {
    bvector<Utf8CP> propNames;
    propNames.push_back("TargetECClassId");
    JsonUpdater updater(m_ecdb, *linkTableRelClass, propNames, nullptr);
    EXPECT_FALSE(updater.IsValid()) << linkTableRelClass->GetFullName() << " with {'TargetECClassId'}";
    }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle     09/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonUpdaterTests, UpdateNavPropAndFkRelationship)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("jsonupdater_rels.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                                    <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                                        <ECEntityClass typeName="Parent" >
                                                            <ECProperty propertyName="Code" typeName="int" />
                                                        </ECEntityClass>
                                                        <ECEntityClass typeName="Child" >
                                                            <ECProperty propertyName="Name" typeName="string" />
                                                            <ECNavigationProperty propertyName="Parent" relationshipName="FkRel" direction="Backward" />
                                                        </ECEntityClass>
                                                        <ECRelationshipClass typeName="FkRel" strength="referencing" modifier="None">
                                                            <Source multiplicity="(0..1)" roleLabel="has" polymorphic="False"><Class class="Parent"/></Source>
                                                            <Target multiplicity="(0..*)" roleLabel="referenced by" polymorphic="False"><Class class="Child"/></Target>
                                                        </ECRelationshipClass>
                                                    </ECSchema>)xml")));

    ECClassCP childClass = m_ecdb.Schemas().GetClass("TestSchema", "Child");
    ASSERT_TRUE(childClass != nullptr);
    ECClassCP fkRelClass = m_ecdb.Schemas().GetClass("TestSchema", "FkRel");
    ASSERT_TRUE(fkRelClass != nullptr);


    ECInstanceKey parent1Key, parent2Key, child1Key, child2Key;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(parent1Key, "INSERT INTO ts.Parent(Code) VALUES('Parent-1')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(parent2Key, "INSERT INTO ts.Parent(Code) VALUES('Parent-2')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(child1Key, Utf8PrintfString("INSERT INTO ts.Child(Name,Parent.Id,Parent.RelECClassId) VALUES('Child-1',%s,%s)", parent1Key.GetInstanceId().ToHexStr().c_str(), fkRelClass->GetId().ToHexStr().c_str()).c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(child2Key, Utf8PrintfString("INSERT INTO ts.Child(Name,Parent.Id,Parent.RelECClassId) VALUES('Child-2',%s,%s)", parent1Key.GetInstanceId().ToHexStr().c_str(), fkRelClass->GetId().ToHexStr().c_str()).c_str()));

    JsonUpdater fkRelUpdater(m_ecdb, *fkRelClass, nullptr);
    ASSERT_FALSE(fkRelUpdater.IsValid()) << "FK relationships are never updatable - already on ECSQL level";

    //Update nav prop
    auto validateNavProp = [] (ECDbCR ecdb, ECInstanceId id, JsonValueCR newJson)
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT Parent.Id, Parent.RelECClassId FROM ts.Child WHERE ECInstanceId=?"));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, id));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Id: " << id.ToString().c_str();

        if (newJson.size() == 0 || !newJson.isMember("parent"))
            {
            ASSERT_TRUE(stmt.IsValueNull(0)) << "JsonUpdater initialized with class -> all missing members are nulled out | " << newJson.ToString();
            ASSERT_TRUE(stmt.IsValueNull(1)) << "JsonUpdater initialized with class -> all missing members are nulled out | " << newJson.ToString();
            return;
            }

        JsonValueCR navPropJson = newJson["parent"];
        if (navPropJson.isNull())
            {
            ASSERT_TRUE(stmt.IsValueNull(0)) << "Parent set to null in JSON -> UPDATE must null it out | " << newJson.ToString();
            ASSERT_TRUE(stmt.IsValueNull(1)) << "Parent set to null in JSON -> UPDATE must null it out | " << newJson.ToString();
            return;
            }

        if (navPropJson.isMember(ECJsonUtilities::json_navId()))
            {
            JsonValueCR navIdJson = navPropJson[ECJsonUtilities::json_navId()];
            if (navIdJson.isNull())
                ASSERT_TRUE(stmt.IsValueNull(0)) << "Parent.Id set to null in JSON -> UPDATE must null it out | " << newJson.ToString();
            else
                ASSERT_STREQ(navIdJson.asCString(), stmt.GetValueId<ECInstanceId>(0).ToHexStr().c_str()) << "Parent.Id | " << newJson.ToString();

            if (navPropJson.isMember(ECJsonUtilities::json_navRelClassName()))
                {
                JsonValueCR navRelClassNameJson = navPropJson[ECJsonUtilities::json_navRelClassName()];
                if (navRelClassNameJson.isNull())
                    ASSERT_TRUE(stmt.IsValueNull(1)) << "Parent.RelECClassId set to null in JSON -> UPDATE nulls out RelClassId it out | " << newJson.ToString();
                else
                    {
                    ECClassId expectedRelClassId = ECJsonUtilities::GetClassIdFromClassNameJson(navRelClassNameJson, ecdb.GetClassLocater());
                    ASSERT_EQ(expectedRelClassId, stmt.GetValueId<ECClassId>(1)) << "Parent.RelECClassId | " << newJson.ToString();
                    }
                }
            else
                ASSERT_TRUE(stmt.IsValueNull(1)) << "Parent.RelECClassId missing JSON -> UPDATE nulls it out | " << newJson.ToString();
            }
        else
            {
            ASSERT_TRUE(stmt.IsValueNull(0)) << "Parent.Id missing JSON -> UPDATE nulls it out | " << newJson.ToString();
            ASSERT_TRUE(stmt.IsValueNull(1)) << "Parent.Id is null -> RelClassId returned is null too | " << newJson.ToString();
            }

        };

    bvector<Utf8CP> propsToUpdate;
    propsToUpdate.push_back("Parent");
    JsonUpdater navPropUpdater(m_ecdb, *childClass, propsToUpdate, nullptr);
    ASSERT_TRUE(navPropUpdater.IsValid());

    Savepoint sp(m_ecdb, "sp", false);

    Utf8String jsonStr;
    Json::Value json;
    rapidjson::Document rapidJson;

    std::map<Utf8String, DbResult> testJsons {
    /*        {Utf8PrintfString(R"json({ "parent" : { "id" : "%s", "relClassName" : "TestSchema.FkRel" } })json", parent2Key.GetInstanceId().ToHexStr().c_str()), BE_SQLITE_OK},
            {Utf8PrintfString(R"json({ "parent" : { "id" : "%s", "relClassName" : null} })json", parent2Key.GetInstanceId().ToHexStr().c_str()), BE_SQLITE_OK},
            {Utf8PrintfString(R"json({ "parent" : { "id" : "%s" } })json", parent2Key.GetInstanceId().ToHexStr().c_str()), BE_SQLITE_OK},
            {R"json({ "parent" : { "id" : null } })json", BE_SQLITE_OK},
            {R"json({ "parent" : null })json", BE_SQLITE_OK},
            {R"json({ "name" : "Child-1", "parent" : { "id" : null } })json", BE_SQLITE_ERROR},
            {R"json({ "name" : "Child-1" })json", BE_SQLITE_ERROR},
            {"{}", BE_SQLITE_ERROR},
            {"null", BE_SQLITE_ERROR},
            {R"json({ "parent" : {} })json", BE_SQLITE_OK},*/
            {R"json({ "parent" : { "relClassName" : "TestSchema.FkRel"} })json", BE_SQLITE_OK},
        };

    for (std::pair<Utf8String, DbResult> testItem : testJsons)
        {
        Utf8StringCR jsonStr = testItem.first;
        DbResult expectedResult = testItem.second;

        Json::Value json;
        ASSERT_TRUE(Json::Reader::Parse(jsonStr, json)) << jsonStr.c_str();

        Savepoint sp(m_ecdb, "sp");
        ASSERT_EQ(expectedResult, navPropUpdater.Update(child1Key.GetInstanceId(), json)) << jsonStr.c_str();
        if (expectedResult != BE_SQLITE_OK)
            {
            sp.Cancel();
            continue;
            }

        validateNavProp(m_ecdb, child1Key.GetInstanceId(), json);
        /*ASSERT_EQ(BE_SQLITE_OK, sp.Cancel());

        rapidjson::Document rapidJson;
        ASSERT_FALSE(rapidJson.Parse<0>(jsonStr.c_str()).HasParseError()) << jsonStr.c_str();

        ASSERT_EQ(BE_SQLITE_OK, sp.Begin());
        ASSERT_EQ(BE_SQLITE_OK, navPropUpdater.Update(child1Key.GetInstanceId(), rapidJson)) << jsonStr.c_str();
        validateNavProp(m_ecdb, child1Key.GetInstanceId(), json);
        ASSERT_EQ(BE_SQLITE_OK, sp.Cancel());*/
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle     09/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonUpdaterTests, UpdateLinkTableRelationship)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("jsonupdater_rels.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                                    <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                                        <ECEntityClass typeName="Parent" >
                                                            <ECProperty propertyName="Code" typeName="int" />
                                                        </ECEntityClass>
                                                        <ECEntityClass typeName="Child" >
                                                            <ECProperty propertyName="Name" typeName="string" />
                                                        </ECEntityClass>
                                                        <ECRelationshipClass typeName="LinkTableRel" strength="referencing" modifier="None">
                                                            <ECProperty propertyName="Order" typeName="int" />
                                                            <ECProperty propertyName="Something" typeName="string" />
                                                            <Source multiplicity="(0..*)" roleLabel="has" polymorphic="False"><Class class="Parent"/></Source>
                                                            <Target multiplicity="(0..*)" roleLabel="referenced by" polymorphic="False"><Class class="Child"/></Target>
                                                        </ECRelationshipClass>
                                                    </ECSchema>)xml")));

    ECClassCP relClass = m_ecdb.Schemas().GetClass("TestSchema", "LinkTableRel");
    ASSERT_TRUE(relClass != nullptr);

    ECClassId childClassId = m_ecdb.Schemas().GetClassId("TestSchema", "Child");
    ASSERT_TRUE(childClassId.IsValid());

    ECInstanceKey parent1Key, parent2Key, child1Key, child2Key, linkTableRelKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(parent1Key, "INSERT INTO ts.Parent(Code) VALUES('Parent-1')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(parent2Key, "INSERT INTO ts.Parent(Code) VALUES('Parent-2')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(child1Key, "INSERT INTO ts.Child(Name) VALUES('Child-1')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(child2Key, "INSERT INTO ts.Child(Name) VALUES('Child-2')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(linkTableRelKey, Utf8PrintfString("INSERT INTO ts.LinkTableRel(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId,[Order],Something) VALUES(%s,%s,%s,%s,10,'what?')",
                                                                                               parent1Key.GetInstanceId().ToHexStr().c_str(), parent1Key.GetClassId().ToHexStr().c_str(),
                                                                                               child1Key.GetInstanceId().ToHexStr().c_str(), child1Key.GetClassId().ToHexStr().c_str()).c_str()));



    auto validate = [] (ECDbCR ecdb, ECInstanceId relId, JsonValueCR expectedPropJson)
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT [Order], Something FROM ts.LinkTableRel WHERE ECInstanceId=?"));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, relId));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Id: " << relId.ToString().c_str();

        if (expectedPropJson.isNull() || expectedPropJson.size() == 0)
            {
            ASSERT_TRUE(stmt.IsValueNull(0)) << expectedPropJson.ToString().c_str();
            ASSERT_TRUE(stmt.IsValueNull(1)) << expectedPropJson.ToString().c_str();
            return;
            }

        if (expectedPropJson.isMember("order") && !expectedPropJson["order"].isNull())
            ASSERT_EQ(expectedPropJson["order"].asInt(), stmt.GetValueInt(0)) << expectedPropJson.ToString().c_str();
        else
            ASSERT_TRUE(stmt.IsValueNull(0)) << "Order | " << expectedPropJson.ToString().c_str();

        if (expectedPropJson.isMember("something") && !expectedPropJson["something"].isNull())
            ASSERT_STREQ(expectedPropJson["something"].asCString(), stmt.GetValueText(1)) << expectedPropJson.ToString().c_str();
        else
            ASSERT_TRUE(stmt.IsValueNull(1)) << "Something | " << expectedPropJson.ToString().c_str();
        };

    JsonUpdater updater(m_ecdb, *relClass, nullptr);
    ASSERT_TRUE(updater.IsValid());

    Savepoint sp(m_ecdb, "sp", false);

    Utf8String jsonStr;
    Json::Value json;
    rapidjson::Document rapidJson;
    std::map<Utf8CP, DbResult> testData {
            {R"json({ "order" : 11 })json", BE_SQLITE_OK},
            {R"json({ "order" : null })json", BE_SQLITE_OK},
            {R"json({ "something" : "that" })json", BE_SQLITE_OK},
            {R"json({ "order" : 100, "something" : "that" })json", BE_SQLITE_OK},
            {R"json({ "order" : 100, "something" : null })json", BE_SQLITE_OK},
            {R"json({ "order" : null, "something" : "this" })json", BE_SQLITE_OK},
            {"{}", BE_SQLITE_ERROR},
            {"null", BE_SQLITE_ERROR},
        };
    for (std::pair<Utf8CP, DbResult> const& kvPair : testData)
        {
        Utf8CP jsonStr = kvPair.first;
        Json::Value json;
        ASSERT_TRUE(Json::Reader::Parse(jsonStr, json)) << jsonStr;

        Savepoint sp(m_ecdb, "sp");

        const DbResult expectedRes = kvPair.second;
        ASSERT_EQ(expectedRes, updater.Update(linkTableRelKey.GetInstanceId(), json)) << "JSON: " << jsonStr;
        if (expectedRes != BE_SQLITE_OK)
            {
            ASSERT_EQ(BE_SQLITE_OK, sp.Cancel());
            continue;
            }

        validate(m_ecdb, linkTableRelKey.GetInstanceId(), json);
        ASSERT_EQ(BE_SQLITE_OK, sp.Cancel());

        rapidjson::Document rapidJson;
        ASSERT_FALSE(rapidJson.Parse<0>(jsonStr).HasParseError()) << jsonStr;

        ASSERT_EQ(BE_SQLITE_OK, sp.Begin());
        ASSERT_EQ(expectedRes, updater.Update(linkTableRelKey.GetInstanceId(), rapidJson)) << "JSON: " << jsonStr;
        if (expectedRes != BE_SQLITE_OK)
            {
            ASSERT_EQ(BE_SQLITE_OK, sp.Cancel());
            continue;
            }

        validate(m_ecdb, linkTableRelKey.GetInstanceId(), json);
        ASSERT_EQ(BE_SQLITE_OK, sp.Cancel());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                  10/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonUpdaterTests, UpdateRelationshipProperty)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("updaterelationshipprop.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                            "<ECSchema schemaName='test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                                            "    <ECEntityClass typeName='A' >"
                                                            "        <ECProperty propertyName='P1' typeName='int' />"
                                                            "    </ECEntityClass>"
                                                            "    <ECRelationshipClass typeName='AHasA' strength='referencing' modifier='Sealed'>"
                                                            "        <ECProperty propertyName='Name' typeName='string' />"
                                                            "        <Source cardinality='(0,N)' polymorphic='False'><Class class='A'/></Source>"
                                                            "        <Target cardinality='(0,N)' polymorphic='False'><Class class='A'/></Target>"
                                                            "    </ECRelationshipClass>"
                                                            "</ECSchema>")));


    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.A (P1) VALUES(?)"));


    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, 111));
    ECInstanceKey sourceKey;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(sourceKey));
    ECInstanceId sourceInstanceId = sourceKey.GetInstanceId();

    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, 222));
    ECInstanceKey targetKey;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(targetKey));
    ECInstanceId targetInstanceId = targetKey.GetInstanceId();

    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.AHasA (SourceECInstanceId, TargetECInstanceId, Name) VALUES(?,?,'good morning')"));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, sourceInstanceId));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, targetInstanceId));

    ECInstanceKey relKey;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(relKey));
    ECInstanceId relInstanceId = relKey.GetInstanceId();

    ECClassCP relClass = m_ecdb.Schemas().GetClass("test", "AHasA");
    ASSERT_TRUE(relClass != nullptr);
    JsonReader reader(m_ecdb, *relClass);
    Json::Value relationshipJson;
    ASSERT_EQ(SUCCESS, reader.Read(relationshipJson, relInstanceId));
    ASSERT_STREQ("good morning", relationshipJson["name"].asCString()) << relationshipJson.ToString().c_str();

    // Update relationship properties
    JsonUpdater updater(m_ecdb, *relClass, nullptr);
    ASSERT_TRUE(updater.IsValid());

    /*
    * Update relationship properties via Json::Value API
    */
    Utf8CP expectedVal = "good afternoon";
    relationshipJson.clear();
    relationshipJson["name"] = expectedVal;
    ASSERT_EQ(BE_SQLITE_OK, updater.Update(relInstanceId, relationshipJson));

    ECSqlStatement checkStmt;
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.Prepare(m_ecdb, "SELECT NULL FROM ts.AHasA WHERE ECInstanceId=? AND Name=?"));

    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindId(1, relInstanceId));
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindText(2, expectedVal, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ROW, checkStmt.Step());
    checkStmt.Reset();
    checkStmt.ClearBindings();

    /*
    * Update relationship properties via rapidjson
    */
    expectedVal = "good evening";
    rapidjson::Document relationshipRapidJson;
    relationshipRapidJson.SetObject();
    relationshipRapidJson.AddMember("name", rapidjson::StringRef(expectedVal), relationshipRapidJson.GetAllocator());

    ASSERT_EQ(BE_SQLITE_OK, updater.Update(relInstanceId, relationshipRapidJson));

    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindId(1, relInstanceId));
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindText(2, expectedVal, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ROW, checkStmt.Step());
    checkStmt.Reset();
    checkStmt.ClearBindings();
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonUpdaterTests, UpdateProperties)
    {
    ECInstanceKey key;
    {
    ASSERT_EQ(SUCCESS, SetupECDb("updateClassProperties.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                        "<ECSchema schemaName='testSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                        "    <ECEntityClass typeName='A' >"
                        "        <ECProperty propertyName='P1' typeName='int' />"
                        "        <ECProperty propertyName='P2' typeName='string' />"
                        "        <ECProperty propertyName='P3' typeName='double' readOnly='True'/>"
                        "    </ECEntityClass>"
                        "</ECSchema>")));


    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.A (P1, P2, P3) VALUES(100, 'JsonTest', 1000.10)"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step(key));
    }

    ECClassCP ecClass = m_ecdb.Schemas().GetClass("testSchema", "A");
    ASSERT_TRUE(ecClass != nullptr);

    {
    JsonUpdater updater(m_ecdb, *ecClass, nullptr);
    ASSERT_FALSE(updater.IsValid()) << "because of readonly prop";
    }

    JsonUpdater updater(m_ecdb, *ecClass, nullptr, "ReadonlyPropertiesAreUpdatable");
    ASSERT_TRUE(updater.IsValid());

    JsonReader reader(m_ecdb,*ecClass);
    Json::Value ecClassJson;
    ASSERT_EQ(SUCCESS, reader.Read(ecClassJson, key.GetInstanceId()));
    ASSERT_EQ(100, ecClassJson["p1"].asInt());
    ASSERT_STREQ("JsonTest", ecClassJson["p2"].asCString());
    ASSERT_DOUBLE_EQ(1000.10, ecClassJson["p3"].asDouble());

    /*
    * Update class properties via Json::Value API
    */
    Utf8CP expectedVal = "Json API";
    ecClassJson.clear();
    ecClassJson["p1"] = 200;
    ecClassJson["p2"] = expectedVal;
    ecClassJson["p3"] = 2000.20;
    ASSERT_EQ(BE_SQLITE_OK, updater.Update(key.GetInstanceId(), ecClassJson));

    ECSqlStatement checkStmt;
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.Prepare(m_ecdb, "SELECT NULL FROM ts.A WHERE ECInstanceId=? AND P1=? AND P2=? AND P3=?"));

    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindInt(2, 200));
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindText(3, expectedVal, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindDouble(4, 2000.20));

    ASSERT_EQ(BE_SQLITE_ROW, checkStmt.Step());
    checkStmt.Reset();
    checkStmt.ClearBindings();

    /*
    * Update class properties via rapidjson
    */
    expectedVal = "RapidJson";
    rapidjson::Document ecClassRapidJson;
    ecClassRapidJson.SetObject();
    ecClassRapidJson.AddMember("p1", 300, ecClassRapidJson.GetAllocator());
    ecClassRapidJson.AddMember("p2", rapidjson::StringRef(expectedVal), ecClassRapidJson.GetAllocator());
    ecClassRapidJson.AddMember("p3", 3000.30, ecClassRapidJson.GetAllocator());

    ASSERT_EQ(BE_SQLITE_OK, updater.Update(key.GetInstanceId(), ecClassRapidJson));

    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindInt(2, 300));
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindText(3, expectedVal, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindDouble(4, 3000.30));
    ASSERT_EQ(BE_SQLITE_ROW, checkStmt.Step());
    checkStmt.Reset();
    checkStmt.ClearBindings();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                              Ramanujam.Raman                   10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonUpdaterTests, CommonGeometryJsonSerialization)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("cgjsonserialization.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8' ?>"
                                                                  "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                                                  "   <ECEntityClass typeName='SpatialLocation' >"
                                                                  "       <ECProperty propertyName='Center' typeName='Bentley.Geometry.Common.IGeometry' />"
                                                                  "       <ECProperty propertyName='URP' typeName='Bentley.Geometry.Common.IGeometry' />"
                                                                  "       <ECProperty propertyName='LLP' typeName='Bentley.Geometry.Common.IGeometry' />"
                                                                  "   <ECProperty propertyName='Location' typeName='Bentley.Geometry.Common.IGeometry'/>"
                                                                  "   </ECEntityClass>"
                                                                  "</ECSchema>")));
    ASSERT_EQ(SUCCESS, PopulateECDb(3));

    ECClassCP spatialClass = m_ecdb.Schemas().GetClass("Test", "SpatialLocation");
    ASSERT_TRUE(nullptr != spatialClass);

    Utf8String expectedJson(R"json({ "location" : { "Polygon" : { "Point" : [[17.672993680591887, 48.591463341759322, 0.0], 
                                                                    [26.072599028627593, 48.591463341759322, 0.0],
                                                                    [26.072599028627593, 54.891247623320339, 0.0],
                                                                    [17.672993680591887, 54.891247623320339, 0.0],
                                                                    [17.672993680591887, 48.591463341759322, 0.0]] }},
                    "lLP" : {
                        "Coordinate" : {
                            "xyz" : [17.672993680591887, 48.591463341759322, 0.0]
                        }
                    },
                    "uRP" : {
                        "Coordinate" : {
                            "xyz" : [26.072599028627593, 54.891247623320339, 0.0]
                        }
                    },
                    "center" : {
                        "Coordinate" : {
                            "xyz" : [21.87279635460974, 51.741355482539831, 0.0]
                        }
                    }
                        }
                       )json");

    Json::Value expectedJsonCppValue;
    ASSERT_TRUE(Json::Reader::Parse(expectedJson, expectedJsonCppValue));

    rapidjson::Document expectedRapidJsonValue;
    ASSERT_FALSE(expectedRapidJsonValue.Parse<0>(expectedJson.c_str()).HasParseError());

    // Insert using RapidJson API
    JsonInserter inserter(m_ecdb, *spatialClass, nullptr);
    ASSERT_TRUE(inserter.IsValid());
    ECInstanceKey rapidJsonInstanceKey;
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(rapidJsonInstanceKey, expectedRapidJsonValue));

    // Insert using JsonCpp API
    ECInstanceKey jsonCppInstanceKey;
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(jsonCppInstanceKey, expectedJsonCppValue));

    m_ecdb.SaveChanges();

    // Validate
    JsonReader reader(m_ecdb, *spatialClass);
    ASSERT_TRUE(reader.IsValid());

    for (ECInstanceId id : {rapidJsonInstanceKey.GetInstanceId(), jsonCppInstanceKey.GetInstanceId()})
        {
        Json::Value actualJson;
        ASSERT_EQ(SUCCESS, reader.Read(actualJson, id));
        ASSERT_TRUE(actualJson.isObject()) << id.ToString().c_str();
        for (int coordIx = 0; coordIx < 3; coordIx++)
            {
            ASSERT_DOUBLE_EQ(expectedJsonCppValue["Center"]["Coordinate"]["xyz"][coordIx].asDouble(), actualJson["Center"]["Coordinate"]["xyz"][coordIx].asDouble()) << id.ToString().c_str() << " Actual Json: " << actualJson.ToString().c_str();
            ASSERT_DOUBLE_EQ(expectedJsonCppValue["LLP"]["Coordinate"]["xyz"][coordIx].asDouble(), actualJson["LLP"]["Coordinate"]["xyz"][coordIx].asDouble()) << id.ToString().c_str() << " Actual Json: " << actualJson["Center"]["Coordinate"]["xyz"].ToString().c_str();
            ASSERT_DOUBLE_EQ(expectedJsonCppValue["URP"]["Coordinate"]["xyz"][coordIx].asDouble(), actualJson["URP"]["Coordinate"]["xyz"][coordIx].asDouble()) << id.ToString().c_str() << " Actual Json: " << actualJson["Center"]["Coordinate"]["xyz"].ToString().c_str();
            }

        for (int i = 0; i < (int) expectedJsonCppValue["Location"]["Polygon"]["Point"].size(); i++)
            {
            for (int coordIx = 0; coordIx < 3; coordIx++)
                {
                ASSERT_DOUBLE_EQ(expectedJsonCppValue["Location"]["Polygon"]["Point"][i][coordIx].asDouble(), actualJson["Location"]["Polygon"]["Point"][i][coordIx].asDouble()) << id.ToString().c_str();
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonUpdaterTests, ReadonlyAndCalculatedProperties)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("updateClassProperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='testSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.00' prefix='bsca' />"
        "     <ECEntityClass typeName='Foo' >"
        "       <ECProperty propertyName='Num' typeName='int' readOnly ='True' />"
        "       <ECProperty propertyName='Square' typeName='string' >"
        "           <ECCustomAttributes>"
        "               <CalculatedECPropertySpecification xmlns = 'Bentley_Standard_CustomAttributes.01.00'>"
        "                   <ECExpression>this.Num * this.Num</ECExpression>"
        "               </CalculatedECPropertySpecification>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "     </ECEntityClass>"
        "</ECSchema>")));

    ECClassCP ecClass = m_ecdb.Schemas().GetClass("testSchema", "Foo");

    const int oldNum = 2;
    Utf8CP oldSquare = "4";
    const int newNum = 3;

    //Insert test instance
    ECInstanceKey key;
    Json::Value properties;
    properties["num"] = oldNum;
    properties["square"] = oldSquare;
    JsonInserter inserter(m_ecdb, *ecClass, nullptr);
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(key, properties));

    //Update test instance
    properties["num"] = newNum;

    ECSqlStatement validateStmt;
    ASSERT_EQ(ECSqlStatus::Success, validateStmt.Prepare(m_ecdb, "SELECT Num, Square FROM ts.Foo WHERE ECInstanceId=?"));

    //default updater
    {
    JsonUpdater updater(m_ecdb, *ecClass, nullptr);
    ASSERT_FALSE(updater.IsValid()) << "Updater invalid because of readonly props";
    }

    //updater with readonly prop options
    {
    JsonUpdater updater(m_ecdb, *ecClass, nullptr, "ReadonlyPropertiesAreUpdatable");
    ASSERT_TRUE(updater.IsValid());
    ASSERT_EQ(BE_SQLITE_OK, updater.Update(key.GetInstanceId(), properties));

    ASSERT_EQ(ECSqlStatus::Success, validateStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, validateStmt.Step());

    ASSERT_EQ(newNum, validateStmt.GetValueInt(0)) << "Readonly property Num is expected to be modified with ECSQLOPTION ReadonlyPropertiesAreUpdatable";
    ASSERT_STREQ(oldSquare, validateStmt.GetValueText(1)) << "Calculated property Square is not expected to be recalculated. ECDb does never evaluate calculated props";
    validateStmt.Reset();
    validateStmt.ClearBindings();
    }

    }

END_ECDBUNITTESTS_NAMESPACE
