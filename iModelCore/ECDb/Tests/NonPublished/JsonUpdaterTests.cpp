/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

struct JsonUpdaterTests : ECDbTestFixture {};

//---------------------------------------------------------------------------------------
// @bsimethod
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
                                                            <ECProperty propertyName="LastMod" typeName="dateTime" readOnly="True" />
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
    EXPECT_FALSE(updater.IsValid()) << "Class with single property which is readonly. Default options";

    JsonUpdater updater2(m_ecdb, *lastModClass, nullptr, JsonUpdater::Options(JsonUpdater::ReadonlyPropertiesOption::Ignore));
    EXPECT_FALSE(updater2.IsValid()) << "Class with single property which is readonly. " << ENUM_TOSTRING(JsonUpdater::ReadonlyPropertiesOption::Ignore);

    JsonUpdater updater3(m_ecdb, *lastModClass, nullptr, JsonUpdater::Options(JsonUpdater::ReadonlyPropertiesOption::Update));
    EXPECT_TRUE(updater3.IsValid()) << "Class with single property which is readonly. "<< ENUM_TOSTRING(JsonUpdater::ReadonlyPropertiesOption::Update);
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

    JsonUpdater updater2(m_ecdb, *lastModClass, nullptr, JsonUpdater::Options(JsonUpdater::ReadonlyPropertiesOption::Ignore));
    EXPECT_FALSE(updater2.IsValid()) << "Property list with single property which is readonly. " << ENUM_TOSTRING(JsonUpdater::ReadonlyPropertiesOption::Ignore);

    JsonUpdater updater3(m_ecdb, *lastModClass, nullptr, JsonUpdater::Options(JsonUpdater::ReadonlyPropertiesOption::Update));
    EXPECT_TRUE(updater3.IsValid()) << "Property list with single property which is readonly. " << ENUM_TOSTRING(JsonUpdater::ReadonlyPropertiesOption::Update);
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonUpdaterTests, Options)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("jsonupdater_options.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                                    <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                                        <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA" />
                                                        <ECEntityClass typeName="NoReadonlyProps" >
                                                            <ECProperty propertyName="Code" typeName="int" />
                                                        </ECEntityClass>
                                                        <ECEntityClass typeName="ReadonlyProps" >
                                                            <ECCustomAttributes>
                                                                <ClassHasCurrentTimeStampProperty xmlns="CoreCustomAttributes.01.00">
                                                                    <PropertyName>LastMod</PropertyName>
                                                                </ClassHasCurrentTimeStampProperty>
                                                            </ECCustomAttributes>                                                            
                                                            <ECProperty propertyName="LastMod" typeName="dateTime" readOnly="True" />
                                                            <ECProperty propertyName="ReadonlyProp" typeName="int" readOnly="True" />
                                                            <ECProperty propertyName="WritableProp" typeName="int" />
                                                        </ECEntityClass>
                                                    </ECSchema>)xml")));

    ECClassCP noReadonlyPropsClass = m_ecdb.Schemas().GetClass("TestSchema", "NoReadonlyProps");
    ASSERT_TRUE(noReadonlyPropsClass != nullptr);
    ECClassCP readonlyPropsClass = m_ecdb.Schemas().GetClass("TestSchema", "ReadonlyProps");
    ASSERT_TRUE(readonlyPropsClass != nullptr);

    ECInstanceKey noReadonlyPropsKey, readonlyPropsKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(noReadonlyPropsKey, "INSERT INTO ts.NoReadonlyProps(Code) VALUES(1000)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(readonlyPropsKey, "INSERT INTO ts.ReadonlyProps(ReadonlyProp,WritableProp) VALUES(1000,1000)"));

    std::vector<JsonUpdater::Options> testOptions
        {
        JsonUpdater::Options(),
        JsonUpdater::Options(JsonUpdater::SystemPropertiesOption::Fail, JsonUpdater::ReadonlyPropertiesOption::Fail),
        JsonUpdater::Options(JsonUpdater::SystemPropertiesOption::Fail, JsonUpdater::ReadonlyPropertiesOption::Ignore),
        JsonUpdater::Options(JsonUpdater::SystemPropertiesOption::Fail, JsonUpdater::ReadonlyPropertiesOption::Update),
        JsonUpdater::Options(JsonUpdater::SystemPropertiesOption::Ignore, JsonUpdater::ReadonlyPropertiesOption::Fail),
        JsonUpdater::Options(JsonUpdater::SystemPropertiesOption::Ignore, JsonUpdater::ReadonlyPropertiesOption::Ignore),
        JsonUpdater::Options(JsonUpdater::SystemPropertiesOption::Ignore, JsonUpdater::ReadonlyPropertiesOption::Update),
        };

    for (JsonUpdater::Options const& options : testOptions)
        {
        ASSERT_TRUE(options.IsValid()) << ToString(options);
        }

    {
    ScopedDisableFailOnAssertion disableFailOnAssertion;
    JsonUpdater::Options invalidOptions = JsonUpdater::Options(JsonUpdater::ReadonlyPropertiesOption::Fail, "ReadonlyPropertiesAreUpdatable");
    ASSERT_FALSE(invalidOptions.IsValid()) << ToString(invalidOptions);
    testOptions.push_back(invalidOptions);

    invalidOptions = JsonUpdater::Options(JsonUpdater::ReadonlyPropertiesOption::Ignore, "ReadonlyPropertiesAreUpdatable");
    ASSERT_FALSE(invalidOptions.IsValid()) << ToString(invalidOptions);
    testOptions.push_back(invalidOptions);

    invalidOptions = JsonUpdater::Options(JsonUpdater::ReadonlyPropertiesOption::Update, "ReadonlyPropertiesAreUpdatable");
    ASSERT_FALSE(invalidOptions.IsValid()) << ToString(invalidOptions);
    testOptions.push_back(invalidOptions);
    }

    std::vector<Utf8String> testJsons {R"json({"Code" : 2000})json",
        R"json({"className" : "TestSchema.NoReadonlyProps", "Code" : 2000})json",
        Utf8PrintfString(R"json({"id" : "%s", "Code" : 2000})json", noReadonlyPropsKey.GetInstanceId().ToHexStr().c_str()),
        Utf8PrintfString(R"json({"id" : "%s", "className" : "TestSchema.NoReadonlyProps", "Code" : 2000})json", noReadonlyPropsKey.GetInstanceId().ToHexStr().c_str())};

    for (JsonUpdater::Options const& options : testOptions)
        {
        JsonUpdater noReadonlyPropsClassUpdater(m_ecdb, *noReadonlyPropsClass, nullptr, options);
        ASSERT_EQ(options.IsValid(), noReadonlyPropsClassUpdater.IsValid());
        if (!options.IsValid())
            continue;

        for (Utf8StringCR testJson : testJsons)
            {
            Json::Value json;
            ASSERT_TRUE(Json::Reader::Parse(testJson, json)) << testJson;

            DbResult expectedRes = BE_SQLITE_OK;
            if (options.GetSystemPropertiesOption() == JsonUpdater::SystemPropertiesOption::Fail && (json.isMember(ECJsonUtilities::json_className()) || json.isMember(ECJsonUtilities::json_id())))
                expectedRes = BE_SQLITE_ERROR;

            Savepoint sp(m_ecdb, "sp");
            ASSERT_EQ(expectedRes, noReadonlyPropsClassUpdater.Update(noReadonlyPropsKey.GetInstanceId(), json)) << ToString(options) << " JSON: " << json.ToString();

            if (expectedRes == BE_SQLITE_OK)
                {
                ECSqlStatement stmt;
                ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Code FROM ts.NoReadonlyProps WHERE ECInstanceId=?"));
                ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, noReadonlyPropsKey.GetInstanceId()));
                ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << ToString(options);
                ASSERT_EQ(2000, stmt.GetValueInt(0)) << ToString(options);
                }

            ASSERT_EQ(BE_SQLITE_OK, sp.Cancel());
            }
        }

    DateTime dtInJson(DateTime::Kind::Unspecified, 2000, 5, 5, 13, 55);
    uint64_t jdInJson;
    ASSERT_EQ(SUCCESS, dtInJson.ToJulianDay(jdInJson));

    Utf8String dtInJsonStr = dtInJson.ToString();

    testJsons = {Utf8PrintfString(R"json({"LastMod" : "%s", "ReadonlyProp" : 2000, "WritableProp" : 2000})json", dtInJsonStr.c_str()),
        Utf8PrintfString(R"json({"id" : "%s", "LastMod" : "%s", "ReadonlyProp" : 2000, "WritableProp" : 2000})json",readonlyPropsKey.GetInstanceId().ToHexStr().c_str(), dtInJsonStr.c_str()),
        Utf8PrintfString(R"json({"className" : "TestSchema.ReadonlyProps", "LastMod" : "%s", "ReadonlyProp" : 2000, "WritableProp" : 2000})json", dtInJsonStr.c_str()),
        Utf8PrintfString(R"json({"id" : "%s", "className" : "TestSchema.ReadonlyProps", "LastMod" : "%s", "ReadonlyProp" : 2000, "WritableProp" : 2000})json",readonlyPropsKey.GetInstanceId().ToHexStr().c_str(), dtInJsonStr.c_str())};

    for (JsonUpdater::Options const& options : testOptions)
        {
        JsonUpdater readonlyPropsClassUpdater(m_ecdb, *readonlyPropsClass, nullptr, options);

        if (!options.IsValid())
            {
            EXPECT_FALSE(readonlyPropsClassUpdater.IsValid()) << "Invalid options: " << ToString(options);
            continue;
            }

        if (options.GetReadonlyPropertiesOption() == JsonUpdater::ReadonlyPropertiesOption::Fail)
            {
            EXPECT_FALSE(readonlyPropsClassUpdater.IsValid()) << ToString(options);
            continue;
            }

        EXPECT_TRUE(readonlyPropsClassUpdater.IsValid()) << ToString(options);

        for (Utf8StringCR testJson : testJsons)
            {
            Json::Value json;
            ASSERT_TRUE(Json::Reader::Parse(testJson, json)) << testJson;

            DbResult expectedRes = BE_SQLITE_OK;
            if (options.GetSystemPropertiesOption() == JsonUpdater::SystemPropertiesOption::Fail && (json.isMember(ECJsonUtilities::json_className()) || json.isMember(ECJsonUtilities::json_id())))
                expectedRes = BE_SQLITE_ERROR;

            Savepoint sp(m_ecdb, "sp");
            ASSERT_EQ(expectedRes, readonlyPropsClassUpdater.Update(readonlyPropsKey.GetInstanceId(), json)) << ToString(options) << " JSON: " << json.ToString();

            if (expectedRes == BE_SQLITE_OK)
                {
                ECSqlStatement stmt;
                ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT LastMod,ReadonlyProp,WritableProp FROM ts.ReadonlyProps WHERE ECInstanceId=?"));
                ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, readonlyPropsKey.GetInstanceId()));
                ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << ToString(options);

                DateTime actualLastMod = stmt.GetValueDateTime(0);
                uint64_t actualLastModJd;
                ASSERT_EQ(SUCCESS, actualLastMod.ToJulianDay(actualLastModJd));

                if (options.GetReadonlyPropertiesOption() == JsonUpdater::ReadonlyPropertiesOption::Ignore)
                    {
                    ASSERT_LT(jdInJson, actualLastModJd) << "LastMod prop is not modified by the updater, so the last mod trigger does it." << ToString(options);
                    ASSERT_EQ(1000, stmt.GetValueInt(1)) << "Readonly prop is expected to be unmodified for " << ToString(options);
                    }
                else
                    {
                    ASSERT_EQ(dtInJson, actualLastMod) << "Readonly prop is expected to be modified for " << ToString(options);
                    ASSERT_EQ(2000, stmt.GetValueInt(1)) << "Readonly prop is expected to be modified for " << ToString(options);
                    }

                ASSERT_EQ(2000, stmt.GetValueInt(2)) << "Writable prop is expected to be modified for any option. Actual option: " << ToString(options);
                }

            ASSERT_EQ(BE_SQLITE_OK, sp.Cancel());
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
    ASSERT_STREQ("good morning", relationshipJson["Name"].asCString()) << relationshipJson.ToString().c_str();

    // Update relationship properties
    JsonUpdater updater(m_ecdb, *relClass, nullptr);
    ASSERT_TRUE(updater.IsValid());

    /*
    * Update relationship properties via Json::Value API
    */
    Utf8CP expectedVal = "good afternoon";
    relationshipJson.clear();
    relationshipJson["Name"] = expectedVal;
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
    relationshipRapidJson.AddMember("Name", rapidjson::StringRef(expectedVal), relationshipRapidJson.GetAllocator());

    ASSERT_EQ(BE_SQLITE_OK, updater.Update(relInstanceId, relationshipRapidJson));

    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindId(1, relInstanceId));
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindText(2, expectedVal, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ROW, checkStmt.Step());
    checkStmt.Reset();
    checkStmt.ClearBindings();
    }

//---------------------------------------------------------------------------------------
// @bsiMethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonUpdaterTests, UpdateReadonlyProperties)
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
    ASSERT_FALSE(updater.IsValid()) << "JsonUpdater with default options | expected to fail because of readonly prop";
    }

    {
    JsonUpdater updater(m_ecdb, *ecClass, nullptr, JsonUpdater::Options(JsonUpdater::ReadonlyPropertiesOption::Fail));
    ASSERT_FALSE(updater.IsValid()) << "JsonUpdater with JsonUpdater::ReadonlyPropertiesOption::Fail | expected to fail because of readonly prop";
    }

    JsonUpdater ignoreReadonlyPropsUpdater(m_ecdb, *ecClass, nullptr, JsonUpdater::Options(JsonUpdater::ReadonlyPropertiesOption::Ignore));
    ASSERT_TRUE(ignoreReadonlyPropsUpdater.IsValid()) << "JsonUpdater with JsonUpdater::ReadonlyPropertiesOption::Ignore";

    JsonUpdater readonlyPropsAreUpdatableUpdater(m_ecdb, *ecClass, nullptr, JsonUpdater::Options(JsonUpdater::ReadonlyPropertiesOption::Update));
    ASSERT_TRUE(readonlyPropsAreUpdatableUpdater.IsValid()) << "JsonUpdater with JsonUpdater::ReadonlyPropertiesOption::Update";

    JsonReader reader(m_ecdb,*ecClass);
    Json::Value json;
    ASSERT_EQ(SUCCESS, reader.Read(json, key.GetInstanceId()));
    ASSERT_EQ(100, json["P1"].asInt());
    ASSERT_STREQ("JsonTest", json["P2"].asCString());
    ASSERT_DOUBLE_EQ(1000.10, json["P3"].asDouble());

    /*
    * Update class properties via Json::Value API
    */
    Utf8CP expectedVal = "Json API";
    json["P1"] = 200;
    json["P2"] = expectedVal;
    json["P3"] = 2000.20;
    ASSERT_TRUE(json.isMember(ECJsonUtilities::json_id()) && json.isMember(ECJsonUtilities::json_className())) << json.ToString().c_str();

    ASSERT_EQ(BE_SQLITE_ERROR, ignoreReadonlyPropsUpdater.Update(key.GetInstanceId(), json)) << "because of system props";
    ASSERT_EQ(BE_SQLITE_ERROR, readonlyPropsAreUpdatableUpdater.Update(key.GetInstanceId(), json)) << "Contains system props: " << json.ToString();


    json.removeMember(ECJsonUtilities::json_id());
    json.removeMember(ECJsonUtilities::json_className());
    
    Savepoint sp(m_ecdb, "sp");
    ASSERT_EQ(BE_SQLITE_OK, ignoreReadonlyPropsUpdater.Update(key.GetInstanceId(), json)) << "system props have been removed.";

    ECSqlStatement checkStmt;
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.Prepare(m_ecdb, "SELECT NULL FROM ts.A WHERE ECInstanceId=? AND P1=? AND P2=? AND P3=?"));

    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindInt(2, 200));
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindText(3, expectedVal, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindDouble(4, 1000.10));
    ASSERT_EQ(BE_SQLITE_ROW, checkStmt.Step());
    checkStmt.Reset();
    checkStmt.ClearBindings();
    ASSERT_EQ(BE_SQLITE_OK, sp.Cancel());

    ASSERT_EQ(BE_SQLITE_OK, sp.Begin());
    ASSERT_EQ(BE_SQLITE_OK, readonlyPropsAreUpdatableUpdater.Update(key.GetInstanceId(), json));

    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindInt(2, 200));
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindText(3, expectedVal, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindDouble(4, 2000.20));

    ASSERT_EQ(BE_SQLITE_ROW, checkStmt.Step());
    checkStmt.Reset();
    checkStmt.ClearBindings();
    ASSERT_EQ(BE_SQLITE_OK, sp.Cancel());

    /*
    * Update class properties via rapidjson
    */
    ASSERT_EQ(BE_SQLITE_OK, sp.Begin());
    expectedVal = "RapidJson";
    rapidjson::Document ecClassRapidJson;
    ecClassRapidJson.SetObject();
    ecClassRapidJson.AddMember("P1", 300, ecClassRapidJson.GetAllocator());
    ecClassRapidJson.AddMember("P2", rapidjson::StringRef(expectedVal), ecClassRapidJson.GetAllocator());
    ecClassRapidJson.AddMember("P3", 3000.30, ecClassRapidJson.GetAllocator());

    ASSERT_EQ(BE_SQLITE_OK, readonlyPropsAreUpdatableUpdater.Update(key.GetInstanceId(), ecClassRapidJson));

    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindInt(2, 300));
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindText(3, expectedVal, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, checkStmt.BindDouble(4, 3000.30));
    ASSERT_EQ(BE_SQLITE_ROW, checkStmt.Step());
    checkStmt.Reset();
    checkStmt.ClearBindings();
    ASSERT_EQ(BE_SQLITE_OK, sp.Cancel());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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

    Utf8String expectedJson(R"json({ "Location" : { "Polygon" : { "Point" : [[17.672993680591887, 48.591463341759322, 0.0], 
                                                                    [26.072599028627593, 48.591463341759322, 0.0],
                                                                    [26.072599028627593, 54.891247623320339, 0.0],
                                                                    [17.672993680591887, 54.891247623320339, 0.0],
                                                                    [17.672993680591887, 48.591463341759322, 0.0]] }},
                    "LLP" : {
                        "Coordinate" : {
                            "xyz" : [17.672993680591887, 48.591463341759322, 0.0]
                        }
                    },
                    "URP" : {
                        "Coordinate" : {
                            "xyz" : [26.072599028627593, 54.891247623320339, 0.0]
                        }
                    },
                    "Center" : {
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonUpdaterTests, ReadonlyProperties)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("updateClassProperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='testSchema' alias='ts' version='01.00.00' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
        "     <ECEntityClass typeName='Foo' >"
        "       <ECProperty propertyName='Num' typeName='int' readOnly ='True' />"
        "     </ECEntityClass>"
        "</ECSchema>")));

    ECClassCP ecClass = m_ecdb.Schemas().GetClass("testSchema", "Foo");

    const int oldNum = 2;
    const int newNum = 3;

    //Insert test instance
    ECInstanceKey key;
    Json::Value properties;
    properties["Num"] = oldNum;
    JsonInserter inserter(m_ecdb, *ecClass, nullptr);
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(key, properties));

    //Update test instance
    properties["Num"] = newNum;

    ECSqlStatement validateStmt;
    ASSERT_EQ(ECSqlStatus::Success, validateStmt.Prepare(m_ecdb, "SELECT Num FROM ts.Foo WHERE ECInstanceId=?"));

    //default updater
    {
    JsonUpdater updater(m_ecdb, *ecClass, nullptr);
    ASSERT_FALSE(updater.IsValid()) << "Updater invalid because of readonly props";
    }

    //updater with ReadonlyPropertiesOption::Update
    {
    JsonUpdater updater(m_ecdb, *ecClass, nullptr, JsonUpdater::Options(JsonUpdater::ReadonlyPropertiesOption::Update));
    ASSERT_TRUE(updater.IsValid());
    ASSERT_EQ(BE_SQLITE_OK, updater.Update(key.GetInstanceId(), properties));

    ASSERT_EQ(ECSqlStatus::Success, validateStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, validateStmt.Step());

    ASSERT_EQ(newNum, validateStmt.GetValueInt(0)) << "Readonly property Num is expected to be modified with JsonUpdater::ReadonlyPropertiesOption::Update";
    validateStmt.Reset();
    validateStmt.ClearBindings();
    }

    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonUpdaterTests, UpdateTimeOfDayValues)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("UpdateTimeOfDayValues.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
            <ECEntityClass typeName="CalendarEntry" modifier="None">
                <ECProperty propertyName="StartTime" typeName="dateTime">
                    <ECCustomAttributes>
                        <DateTimeInfo xmlns="CoreCustomAttributes.01.00.01">
                            <DateTimeComponent>TimeOfDay</DateTimeComponent>
                        </DateTimeInfo>
                    </ECCustomAttributes>
                </ECProperty>
                <ECProperty propertyName="EndTime" typeName="dateTime">
                    <ECCustomAttributes>
                        <DateTimeInfo xmlns="CoreCustomAttributes.01.00.01">
                            <DateTimeComponent>TimeOfDay</DateTimeComponent>
                        </DateTimeInfo>
                    </ECCustomAttributes>
                </ECProperty>
            </ECEntityClass>
        </ECSchema>)xml")));

    ECInstanceKey key1, key2;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key1, "INSERT INTO ts.CalendarEntry(StartTime,EndTime) VALUES(TIME '08:00', TIME '17:30:45.500')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key2, "INSERT INTO ts.CalendarEntry(StartTime,EndTime) VALUES(TIME '00:00', TIME '24:00')"));


    ECClassCP calendarEntryClass = m_ecdb.Schemas().GetClass("TestSchema", "CalendarEntry");
    ASSERT_TRUE(calendarEntryClass != nullptr);

    {
    JsonUpdater updater(m_ecdb, *calendarEntryClass, nullptr);
    ASSERT_TRUE(updater.IsValid());
    ASSERT_EQ(BE_SQLITE_OK, updater.Update(key1.GetInstanceId(), JsonValue("{\"StartTime\":\"08:30\", \"EndTime\":\"20:00\"}").m_value));
    }

    {
    bvector<Utf8CP> propsToUpdate;
    propsToUpdate.push_back("EndTime");
    JsonUpdater updater(m_ecdb, *calendarEntryClass, propsToUpdate, nullptr);
    ASSERT_TRUE(updater.IsValid());
    ASSERT_EQ(BE_SQLITE_OK, updater.Update(key2.GetInstanceId(), JsonValue("{\"EndTime\":\"23:59:59.999\"}").m_value));
    }

    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

    EXPECT_EQ(JsonValue("[{\"StartTime\": \"08:30:00.000\", \"EndTime\":\"20:00:00.000\"}]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT StartTime,EndTime FROM ts.CalendarEntry WHERE ECInstanceId=%s", key1.GetInstanceId().ToString().c_str()).c_str()));
    EXPECT_EQ(JsonValue("[{\"StartTime\": \"00:00:00.000\", \"EndTime\":\"23:59:59.999\"}]"), GetHelper().ExecuteSelectECSql(Utf8PrintfString("SELECT StartTime,EndTime FROM ts.CalendarEntry WHERE ECInstanceId=%s", key2.GetInstanceId().ToString().c_str()).c_str()));
    }

END_ECDBUNITTESTS_NAMESPACE
