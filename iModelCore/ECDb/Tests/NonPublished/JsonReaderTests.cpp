/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================    
// @bsiclass                                                 Krischan.Eberle     11/2018
//=======================================================================================    
struct IJson
    {
    private:
        virtual RapidJsonValueCR _RapidJson() const = 0;
        virtual JsonValueCR _JsonCpp() const = 0;

    protected:
        IJson() {}

    public:
        virtual ~IJson() {}

        Nullable<bool> Equals(IJson const& rhs) const
            {
            const bool rapidJsonEquals = _RapidJson() == rhs._RapidJson();
            const bool jsonCppEquals = _JsonCpp() == rhs._JsonCpp();

            if (rapidJsonEquals != jsonCppEquals)
                return nullptr;

            return rapidJsonEquals;
            }

        Nullable<bool> IsNull() const
            {
            const bool result = _RapidJson().IsNull();
            if (result != _JsonCpp().isNull())
                return nullptr;

            return result;
            }

        Nullable<bool> GetBool() const
            {
            const bool result = _RapidJson().GetBool();
            if (result != _JsonCpp().asBool())
                return nullptr;

            return result;
            }

        Nullable<int> GetInt() const
            {
            int result = 0;
            if (_RapidJson().IsInt())
                result = _RapidJson().GetInt();
            else if (_RapidJson().IsUint())
                result = _RapidJson().GetUint();
            else
                return nullptr;

            if (result != _JsonCpp().asInt())
                return nullptr;

            return result;
            }

        Nullable<int64_t> GetInt64() const
            {
            int64_t result = 0;
            if (_RapidJson().IsInt() || _RapidJson().IsInt64())
                result = _RapidJson().GetInt64();
            else if (_RapidJson().IsUint() || _RapidJson().IsUint64())
                result = _RapidJson().GetUint64();
            else
                return nullptr;

            if (result != _JsonCpp().asInt64())
                return nullptr;

            return result;
            }

        Nullable<double> GetDouble() const
            {
            if (!_RapidJson().IsNumber())
                return nullptr;

            const double result = _RapidJson().GetDouble();
            if (result != _JsonCpp().asDouble())
                return nullptr;

            return result;
            }

        Nullable<bool> IsString() const
            {
            const bool result = _RapidJson().IsString();
            if (result != _JsonCpp().isString())
                return nullptr;

            return result;
            }

        Utf8CP GetString() const
            {
            Utf8CP result = _RapidJson().GetString();
            if (strcmp(result, _JsonCpp().asCString()) != 0)
                return nullptr;

            return result;
            }

        Nullable<bool> IsArray() const
            {
            const bool result = _RapidJson().IsArray();
            if (result != _JsonCpp().isArray())
                return nullptr;

            return result;
            }

        Nullable<int> ArraySize() const
            {
            const int result = (int) _RapidJson().Size();
            if (result != (int) _JsonCpp().size())
                return nullptr;

            return result;
            }

        Nullable<bool> IsObject() const
            {
            const bool result = _RapidJson().IsObject();
            if (result != _JsonCpp().isObject())
                return nullptr;

            return result;
            }

        Nullable<int> MemberCount() const
            {
            const int result = (int) _RapidJson().MemberCount();
            if (result != (int) _JsonCpp().size())
                return nullptr;

            return result;
            }

        Nullable<bool> HasMember(Utf8StringCR memberName) const { return HasMember(memberName.c_str()); }
        Nullable<bool> HasMember(Utf8CP memberName) const
            {
            const bool result = _RapidJson().HasMember(memberName);
            if (result != _JsonCpp().isMember(memberName))
                return nullptr;

            return result;
            }
        

        Utf8String ToString() const
            {
            return Utf8PrintfString("[rapidjson: %s | jsoncpp: %s]", TestUtilities::ToString(_RapidJson()).c_str(), _JsonCpp().ToString().c_str());
            }

        RapidJsonValueCR RapidJson() const { return _RapidJson(); }
        JsonValueCR JsonCpp() const { return _JsonCpp(); }

    };

//=======================================================================================    
// @bsiclass                                                 Krischan.Eberle     11/2018
//=======================================================================================    
struct JsonRef : IJson
    {
    private:
        rapidjson::Value const& m_rapidjson;
        Json::Value const& m_jsoncpp;

        RapidJsonValueCR _RapidJson() const override { return m_rapidjson; }
        JsonValueCR _JsonCpp() const override { return m_jsoncpp; }

    public:
        JsonRef(rapidjson::Value const& rapidjson, Json::Value const& jsoncpp) : m_rapidjson(rapidjson), m_jsoncpp(jsoncpp) {}

        JsonRef operator[](Utf8StringCR memberName) const { return operator[](memberName.c_str()); }
        JsonRef operator[](Utf8CP memberName) const
            {
            BeAssert(HasMember(memberName) == true);
            return JsonRef(_RapidJson()[memberName], _JsonCpp()[memberName]);
            }

        JsonRef operator[](int arrayIndex) const
            {
            BeAssert(ArraySize() != nullptr && ArraySize().Value() > arrayIndex);
            return JsonRef(_RapidJson()[(rapidjson::SizeType) arrayIndex], _JsonCpp()[(Json::ArrayIndex) arrayIndex]);
            }

    };

//=======================================================================================    
// @bsiclass                                                 Krischan.Eberle     11/2018
//=======================================================================================    
struct JsonDoc final : IJson
    {
    private:
        rapidjson::Document m_rapidjson;
        Json::Value m_jsoncpp;

        RapidJsonValueCR _RapidJson() const override { return m_rapidjson;  }
        JsonValueCR _JsonCpp() const override { return m_jsoncpp; }

    public:
        JsonDoc() : IJson(){}
        ~JsonDoc() {}

        void Clear()
            {
            m_rapidjson.SetNull();
            m_jsoncpp = Json::Value(Json::nullValue);
            }

        Json::Value& JsonCpp() { return m_jsoncpp; }
        rapidjson::Document& RapidJson() { return m_rapidjson; }
        rapidjson::MemoryPoolAllocator<>& Allocator() { return m_rapidjson.GetAllocator(); }

        JsonRef operator[](Utf8StringCR memberName) const { return operator[](memberName.c_str()); }
        JsonRef operator[](Utf8CP memberName) const
            {
            BeAssert(HasMember(memberName) == true);
            return JsonRef(_RapidJson()[memberName], _JsonCpp()[memberName]);
            }

        JsonRef operator[](int arrayIndex) const
            {
            BeAssert(ArraySize() != nullptr && ArraySize().Value() > arrayIndex);
            return JsonRef(_RapidJson()[(rapidjson::SizeType) arrayIndex], _JsonCpp()[(Json::ArrayIndex) arrayIndex]);
            }

    };


//=======================================================================================    
// @bsiclass                                                 Krischan.Eberle     11/2018
//=======================================================================================    
struct JsonECSqlSelectAdapterTests : public ECDbTestFixture {};

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                09/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonECSqlSelectAdapterTests, RepreparedStatements)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("RepreparedStatements.ecdb"));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM meta.ECSchemaDef LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    Json::Value json;
    JsonECSqlSelectAdapter adapter(stmt);
    ASSERT_EQ(SUCCESS, adapter.GetRow(json)) << stmt.GetECSql();
    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json({"id":"%s"})json", stmt.GetValueId<ECInstanceId>(0).ToHexStr().c_str())), JsonValue(json)) << stmt.GetECSql();
    stmt.Finalize();
    ASSERT_EQ(ERROR, adapter.GetRow(json)) << stmt.GetECSql();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM meta.ECSchemaDef LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(SUCCESS, adapter.GetRow(json)) << stmt.GetECSql();
    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json({"id":"%s"})json", stmt.GetValueId<ECInstanceId>(0).ToHexStr().c_str())), JsonValue(json)) << stmt.GetECSql();

    stmt.Finalize();
    ASSERT_EQ(ERROR, adapter.GetRow(json)) << stmt.GetECSql();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId MyId FROM meta.ECSchemaDef"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(SUCCESS, adapter.GetRow(json)) << stmt.GetECSql();
    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json({"MyId":"%s"})json", stmt.GetValueId<ECInstanceId>(0).ToString().c_str())), JsonValue(json)) << stmt.GetECSql();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                09/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonECSqlSelectAdapterTests, JsonMemberNames)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("JsonMemberNamesInJsonECSqlSelectAdapter.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    ECClassId relClassId = m_ecdb.Schemas().GetClassId("ECSqlTest", "PSAHasP_N1");
    ASSERT_TRUE(relClassId.IsValid());

    ECInstanceKey pKey, psaKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(pKey, "INSERT INTO ecsql.P(ECInstanceId) VALUES (NULL)"));
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(I,L,PStructProp.i,P) VALUES (10,10,10,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(1, pKey.GetInstanceId(), relClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(psaKey));
    }

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, ECInstanceId, ECClassId, ECClassId, I, I, P, P.Id, P.RelECClassId FROM ecsql.PSA LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    Utf8String expectedIdStr = psaKey.GetInstanceId().ToHexStr();
    Utf8String expectedNavIdHexStr = pKey.GetInstanceId().ToHexStr();
    Utf8String expectedNavIdStr = pKey.GetInstanceId().ToString();

    JsonECSqlSelectAdapter defaultAdapter(stmt);
    JsonDoc defaultJson;
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson.JsonCpp()));
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson.RapidJson(), defaultJson.Allocator()));

    JsonECSqlSelectAdapter javaScriptAdapter(stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsDecimalString));
    JsonDoc javaScriptJson;
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson.JsonCpp()));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson.RapidJson(), javaScriptJson.Allocator()));

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember(ECJsonSystemNames::Id())) << defaultJson.ToString();
    EXPECT_STRCASEEQ(expectedIdStr.c_str(), defaultJson[ECJsonSystemNames::Id()].GetString()) << defaultJson.ToString();


    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember(ECJsonSystemNames::Id())) << javaScriptJson.ToString();
    EXPECT_STRCASEEQ(expectedIdStr.c_str(), javaScriptJson[ECJsonSystemNames::Id()].GetString()) << javaScriptJson.ToString();
    }

    {
    Utf8String id1Member(ECJsonSystemNames::Id());
    id1Member.append("_1");
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember(id1Member)) << defaultJson.ToString();
    EXPECT_STRCASEEQ(expectedIdStr.c_str(), defaultJson[id1Member.c_str()].GetString()) << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember(id1Member)) << javaScriptJson.ToString();
    EXPECT_STRCASEEQ(expectedIdStr.c_str(), javaScriptJson[id1Member.c_str()].GetString()) << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember(ECJsonSystemNames::ClassName())) << defaultJson.ToString();
    EXPECT_STRCASEEQ("ECSqlTest.PSA", defaultJson[ECJsonSystemNames::ClassName()].GetString()) << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember(ECJsonSystemNames::ClassName())) << javaScriptJson.ToString();
    EXPECT_STRCASEEQ("ECSqlTest.PSA", javaScriptJson[ECJsonSystemNames::ClassName()].GetString()) << javaScriptJson.ToString();
    }

    {
    Utf8String className1Member(ECJsonSystemNames::ClassName());
    className1Member.append("_1");

    ASSERT_TRUE_NULLABLE(defaultJson.HasMember(className1Member.c_str())) << defaultJson.ToString();
    EXPECT_STRCASEEQ("ECSqlTest.PSA", defaultJson[className1Member.c_str()].GetString()) << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember(className1Member.c_str())) << javaScriptJson.ToString();
    EXPECT_STRCASEEQ("ECSqlTest.PSA", javaScriptJson[className1Member.c_str()].GetString()) << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("I")) << defaultJson.ToString();
    EXPECT_EQ(Nullable<int>(10), defaultJson["I"].GetInt()) << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("i")) << javaScriptJson.ToString();
    EXPECT_EQ(Nullable<int>(10), javaScriptJson["i"].GetInt()) << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("I_1")) << defaultJson.ToString();
    EXPECT_EQ(Nullable<int>(10), defaultJson["I_1"].GetInt()) << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("i_1")) << javaScriptJson.ToString();
    EXPECT_EQ(Nullable<int>(10), javaScriptJson["i_1"].GetInt()) << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("P")) << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(defaultJson["P"].IsObject()) << defaultJson.ToString();
    EXPECT_STRCASEEQ(expectedNavIdHexStr.c_str(), defaultJson["P"][ECJsonSystemNames::Navigation::Id()].GetString()) << defaultJson.ToString();
    EXPECT_STRCASEEQ("ECSqlTest.PSAHasP_N1", defaultJson["P"][ECJsonSystemNames::Navigation::RelClassName()].GetString()) << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("p")) << javaScriptJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("p")) << javaScriptJson.ToString();
    EXPECT_STRCASEEQ(expectedNavIdHexStr.c_str(), javaScriptJson["p"][ECJsonSystemNames::Navigation::Id()].GetString()) << javaScriptJson.ToString();
    EXPECT_STRCASEEQ("ECSqlTest.PSAHasP_N1", javaScriptJson["p"][ECJsonSystemNames::Navigation::RelClassName()].GetString()) << javaScriptJson.ToString();
    }
    /*
    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("P.id")) << defaultJson.ToString();
    EXPECT_STRCASEEQ(expectedNavIdStr.c_str(), defaultJson["P.id"].GetString()) << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("p.id")) << javaScriptJson.ToString();
    EXPECT_STRCASEEQ(expectedNavIdStr.c_str(), javaScriptJson["p.id"].GetString()) << javaScriptJson.ToString();
    }

    {
    //RelECClassId is not converted to relClassName if explicitly specified in select clause.
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("P.RelECClassId")) << defaultJson.ToString();
    EXPECT_STRCASEEQ(relClassId.ToString().c_str(), defaultJson["P.RelECClassId"].GetString()) << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("p.RelECClassId")) << javaScriptJson.ToString();
    EXPECT_STRCASEEQ(relClassId.ToString().c_str(), javaScriptJson["p.RelECClassId"].GetString()) << javaScriptJson.ToString();
    }*/
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                11/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonECSqlSelectAdapterTests, GetRowInstanceAndDuplicateMemberNames)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("JsonECSqlSelectAdapterTests_GetRowInstanceAndDuplicateMemberNames.ecdb"));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT s.ECInstanceId, c.ECInstanceId, c.Schema.Id id, c.Name, s.ECClassId, s.Name, s.ECClassId FROM meta.ECSchemaDef s JOIN meta.ECClassDef c ON c.Schema.Id=s.ECInstanceId"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    ECClassId schemaDefId = m_ecdb.Schemas().GetClassId("ECDbMeta", "ECSchemaDef");
    ASSERT_TRUE(schemaDefId.IsValid());
    ECClassId classDefId = m_ecdb.Schemas().GetClassId("ECDbMeta", "ECClassDef");
    ASSERT_TRUE(classDefId.IsValid());

    JsonECSqlSelectAdapter adapter(stmt);
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    JsonDoc rowJson, classJson, schemaJson;
    ASSERT_EQ(SUCCESS, adapter.GetRow(rowJson.RapidJson(), rowJson.Allocator()));
    ASSERT_EQ(SUCCESS, adapter.GetRow(rowJson.JsonCpp()));
    ASSERT_EQ(SUCCESS, adapter.GetRowInstance(classJson.RapidJson(), classDefId, classJson.Allocator()));
    ASSERT_EQ(SUCCESS, adapter.GetRowInstance(classJson.JsonCpp(), classDefId));
    ASSERT_EQ(SUCCESS, adapter.GetRowInstance(schemaJson.RapidJson(), schemaDefId, schemaJson.Allocator()));
    ASSERT_EQ(SUCCESS, adapter.GetRowInstance(schemaJson.JsonCpp(), schemaDefId));

    ASSERT_TRUE_NULLABLE(rowJson.HasMember("id")) << rowJson.ToString();
    EXPECT_STREQ(stmt.GetValueId<ECInstanceId>(0).ToHexStr().c_str(), rowJson["id"].GetString()) << rowJson.ToString();

    ASSERT_TRUE_NULLABLE(rowJson.HasMember("id_1")) << rowJson.ToString();
    EXPECT_STREQ(stmt.GetValueId<ECInstanceId>(1).ToHexStr().c_str(), rowJson["id_1"].GetString()) << rowJson.ToString();

    ASSERT_TRUE_NULLABLE(rowJson.HasMember("id_2")) << rowJson.ToString();
    EXPECT_STREQ(stmt.GetValueId<BeInt64Id>(2).ToString().c_str(), rowJson["id_2"].GetString()) << rowJson.ToString();

    ASSERT_TRUE_NULLABLE(rowJson.HasMember("Name")) << rowJson.ToString();
    EXPECT_STREQ(stmt.GetValueText(3), rowJson["Name"].GetString()) << rowJson.ToString();

    ASSERT_TRUE_NULLABLE(rowJson.HasMember("className")) << rowJson.ToString();
    EXPECT_STREQ("ECDbMeta.ECSchemaDef", rowJson["className"].GetString()) << rowJson.ToString();

    ASSERT_TRUE_NULLABLE(rowJson.HasMember("Name_1")) << rowJson.ToString();
    EXPECT_STREQ(stmt.GetValueText(5), rowJson["Name_1"].GetString()) << rowJson.ToString();

    ASSERT_TRUE_NULLABLE(rowJson.HasMember("className_1")) << rowJson.ToString();
    EXPECT_STREQ("ECDbMeta.ECSchemaDef", rowJson["className_1"].GetString()) << rowJson.ToString();

    // ECSchemaDef instance
    ASSERT_EQ_NULLABLE(3, schemaJson.MemberCount()) << schemaJson.ToString();

    ASSERT_TRUE_NULLABLE(schemaJson.HasMember("id")) << schemaJson.ToString();
    EXPECT_STREQ(stmt.GetValueId<ECInstanceId>(0).ToHexStr().c_str(), schemaJson["id"].GetString()) << schemaJson.ToString();

    ASSERT_TRUE_NULLABLE(schemaJson.HasMember("className")) << schemaJson.ToString();
    EXPECT_STREQ("ECDbMeta.ECSchemaDef", schemaJson["className"].GetString()) << schemaJson.ToString();

    ASSERT_TRUE_NULLABLE(schemaJson.HasMember("Name")) << schemaJson.ToString();
    EXPECT_STREQ(stmt.GetValueText(5), schemaJson["Name"].GetString()) << schemaJson.ToString();

    // ECClassDef instance
    ASSERT_EQ_NULLABLE(2, classJson.MemberCount()) << classJson.ToString();

    ASSERT_TRUE_NULLABLE(classJson.HasMember("id")) << classJson.ToString();
    EXPECT_STREQ(stmt.GetValueId<ECInstanceId>(1).ToHexStr().c_str(), classJson["id"].GetString()) << classJson.ToString();

    ASSERT_TRUE_NULLABLE(classJson.HasMember("Name")) << classJson.ToString();
    EXPECT_STREQ(stmt.GetValueText(3), classJson["Name"].GetString()) << classJson.ToString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                11/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonECSqlSelectAdapterTests, AppendToJson)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("JsonECSqlSelectAdapterTests_AppendToJson.ecdb"));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, Name, Type, Modifier FROM meta.ECClassDef LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    JsonECSqlSelectAdapter adapter(stmt);
    {
    Json::Value json;
    ASSERT_EQ(ERROR, adapter.GetRow(json, true)) << "Cannot append to JSON null";
    json = Json::Value(Json::arrayValue);
    ASSERT_EQ(ERROR, adapter.GetRow(json, true)) << "Cannot append to JSON array";
    json = Json::Value(10);
    ASSERT_EQ(ERROR, adapter.GetRow(json, true)) << "Cannot append to JSON number";
    json = Json::Value("12313");
    ASSERT_EQ(ERROR, adapter.GetRow(json, true)) << "Cannot append to JSON string";
    }
    {
    rapidjson::Document json;
    ASSERT_EQ(ERROR, adapter.GetRow(json, json.GetAllocator(), true)) << "Cannot append to JSON null";
    json.SetArray();
    ASSERT_EQ(ERROR, adapter.GetRow(json, json.GetAllocator(), true)) << "Cannot append to JSON array";
    json.SetInt(10);
    ASSERT_EQ(ERROR, adapter.GetRow(json, json.GetAllocator(), true)) << "Cannot append to JSON number";
    json.SetString("12313");
    ASSERT_EQ(ERROR, adapter.GetRow(json, json.GetAllocator(), true)) << "Cannot append to JSON string";
    }

    Utf8PrintfString expectedJsonCore(R"json("id":"%s", "Name": "%s", "Type": %d, "Modifier": %d)json", stmt.GetValueId<ECInstanceId>(0).ToHexStr().c_str(),
                                      stmt.GetValueText(1), stmt.GetValueInt(2), stmt.GetValueInt(3));
    {
    Json::Value json(Json::objectValue);
    ASSERT_EQ(SUCCESS, adapter.GetRow(json, true)) << "Append to empty JSON object";
    EXPECT_EQ(4, (int) json.size()) << json.ToString();
    EXPECT_EQ(JsonValue(Utf8PrintfString("{%s}", expectedJsonCore.c_str())), JsonValue(json)) << json.ToString();

    json = Json::Value(Json::objectValue);
    json["MyNumber"] = Json::Value(54321);
    json["MyArray"] = Json::Value(Json::arrayValue);
    json["MyArray"].append(Json::Value(5));
    json["MyArray"].append(Json::Value(4));
    json["MyObj"]["FirstName"] = Json::Value("Gustav");
    json["MyObj"]["LastName"] = Json::Value("Mueller");

    ASSERT_EQ(SUCCESS, adapter.GetRow(json, true)) << "Append to non-empty JSON object";
    EXPECT_EQ(7, (int) json.size()) << json.ToString();
    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json({"MyNumber":54321,"MyArray":[5,4],"MyObj":{"FirstName":"Gustav","LastName":"Mueller"},%s})json", expectedJsonCore.c_str())), JsonValue(json)) << json.ToString();
    }
    {
    rapidjson::Document json;
    json.SetObject();
    ASSERT_EQ(SUCCESS, adapter.GetRow(json, json.GetAllocator(), true)) << "Append to empty JSON object";
    EXPECT_EQ(4, (int) json.MemberCount()) << TestUtilities::ToString(json);
    EXPECT_EQ(JsonValue(Utf8PrintfString("{%s}", expectedJsonCore.c_str())), JsonValue(TestUtilities::ToString(json))) << TestUtilities::ToString(json);

    json.SetNull();
    json.SetObject();
    json.AddMember(rapidjson::StringRef("MyNumber"), rapidjson::Value(54321).Move(), json.GetAllocator());
    json.AddMember(rapidjson::StringRef("MyArray"), rapidjson::Value().SetArray().PushBack(5, json.GetAllocator()).PushBack(4, json.GetAllocator()).Move(), json.GetAllocator());
    json.AddMember(rapidjson::StringRef("MyObj"), rapidjson::Value().SetObject().AddMember(rapidjson::StringRef("FirstName"), rapidjson::Value("Gustav"), json.GetAllocator()).AddMember(rapidjson::StringRef("LastName"), rapidjson::Value("Mueller"), json.GetAllocator()), json.GetAllocator());
    ASSERT_EQ(SUCCESS, adapter.GetRow(json, json.GetAllocator(), true)) << "Append to non-empty JSON object";
    EXPECT_EQ(7, (int) json.MemberCount()) << TestUtilities::ToString(json);
    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json({"MyNumber":54321,"MyArray":[5,4],"MyObj":{"FirstName":"Gustav","LastName":"Mueller"},%s})json", expectedJsonCore.c_str())), JsonValue(TestUtilities::ToString(json))) << TestUtilities::ToString(json);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                09/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonECSqlSelectAdapterTests, SpecialSelectClauseItems)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("SpecialSelectClauseItemsInJsonECSqlSelectAdapter.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                                                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                                                            <ECStructClass typeName="MyStruct" >
                                                                                <ECProperty propertyName="SomeNumber" typeName="int" />
                                                                            </ECStructClass>
                                                                            <ECEntityClass typeName="Foo" >
                                                                                <ECProperty propertyName="IntProp" typeName="int" />
                                                                                <ECProperty propertyName="PtProp" typeName="Point3d" />
                                                                                <ECStructProperty propertyName="StructProp" typeName="MyStruct" />
                                                                            </ECEntityClass>
                                                                        </ECSchema>)xml")));

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key, "INSERT INTO ts.Foo(IntProp,PtProp.X,PtProp.Y,PtProp.Z,StructProp.SomeNumber) VALUES (1000,1000,1000,1000,1000)"));

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT NULL, NULL As MyNull FROM ts.Foo"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    JsonECSqlSelectAdapter defaultAdapter(stmt);
    JsonECSqlSelectAdapter javaScriptAdapter(stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsDecimalString));
    
    JsonDoc defaultJson, javaScriptJson;
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson.RapidJson(), defaultJson.Allocator()));
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson.JsonCpp()));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson.RapidJson(), javaScriptJson.Allocator()));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson.JsonCpp()));

    {
    ASSERT_TRUE_NULLABLE(defaultJson.IsObject()) << stmt.GetECSql() << " - " << defaultJson.ToString();
    ASSERT_EQ_NULLABLE(0, defaultJson.MemberCount()) << stmt.GetECSql() << " - " << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson.IsObject()) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    ASSERT_EQ_NULLABLE(0, javaScriptJson.MemberCount()) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    }

    {
    ASSERT_FALSE_NULLABLE(defaultJson.HasMember("NULL")) << "NULL literal results in null which is ignored by adapter. " << defaultJson.ToString();
    ASSERT_FALSE_NULLABLE(defaultJson.HasMember("nULL")) << "NULL literal results in null which is ignored by adapter. " << defaultJson.ToString();

    ASSERT_FALSE_NULLABLE(javaScriptJson.HasMember("nULL")) << "NULL literal results in null which is ignored by adapter. " << javaScriptJson.ToString();
    ASSERT_FALSE_NULLABLE(javaScriptJson.HasMember("NULL")) << "NULL literal results in null which is ignored by adapter. " << javaScriptJson.ToString();
    ASSERT_FALSE_NULLABLE(javaScriptJson.HasMember("myNull")) << "NULL literal results in null which is ignored by adapter. " << javaScriptJson.ToString();
    }
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT 123, 123, 123 AS MyNumber, 123 * 3, 123 * 3 AS MyProduct FROM ts.Foo"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    JsonECSqlSelectAdapter defaultAdapter(stmt);
    JsonECSqlSelectAdapter javaScriptAdapter(stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsDecimalString));

    JsonDoc defaultJson, javaScriptJson;
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson.RapidJson(), defaultJson.Allocator()));
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson.JsonCpp()));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson.RapidJson(), javaScriptJson.Allocator()));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson.JsonCpp()));

    {
    ASSERT_TRUE_NULLABLE(defaultJson.IsObject()) << defaultJson.ToString();
    ASSERT_EQ_NULLABLE(stmt.GetColumnCount(), defaultJson.MemberCount()) << stmt.GetECSql() << " - " << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.IsObject()) << javaScriptJson.ToString();
    ASSERT_EQ_NULLABLE(stmt.GetColumnCount(), javaScriptJson.MemberCount()) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("123")) << stmt.GetECSql() << " - " << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(defaultJson["123"].IsString()) << stmt.GetECSql() << " - " << defaultJson.ToString();
    ASSERT_STREQ("123", defaultJson["123"].GetString()) << stmt.GetECSql() << " - " << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("123")) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson["123"].IsString()) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    ASSERT_STREQ("123", javaScriptJson["123"].GetString()) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("123_1")) << stmt.GetECSql() << " - " << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(defaultJson["123_1"].IsString()) << stmt.GetECSql() << " - " << defaultJson.ToString();
    ASSERT_STREQ("123", defaultJson["123_1"].GetString()) << stmt.GetECSql() << " - " << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("123_1")) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson["123_1"].IsString()) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    ASSERT_STREQ("123", javaScriptJson["123_1"].GetString()) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("MyNumber")) << stmt.GetECSql() << " - " << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(defaultJson["MyNumber"].IsString()) << stmt.GetECSql() << " - " << defaultJson.ToString();
    ASSERT_STREQ("123", defaultJson["MyNumber"].GetString()) << stmt.GetECSql() << " - " << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("myNumber")) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson["myNumber"].IsString()) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    ASSERT_STREQ("123", javaScriptJson["myNumber"].GetString()) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("123 * 3")) << stmt.GetECSql() << " - " << defaultJson.ToString();
    ASSERT_DOUBLE_EQ_NULLABLE(369, defaultJson["123 * 3"].GetDouble()) << stmt.GetECSql() << " - " << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("123 * 3")) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    ASSERT_DOUBLE_EQ_NULLABLE(369, javaScriptJson["123 * 3"].GetDouble()) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("MyProduct")) << stmt.GetECSql() << " - " << defaultJson.ToString();
    ASSERT_DOUBLE_EQ_NULLABLE(369, defaultJson["MyProduct"].GetDouble()) << stmt.GetECSql() << " - " << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("myProduct")) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    ASSERT_DOUBLE_EQ_NULLABLE(369, javaScriptJson["myProduct"].GetDouble()) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    }
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT IntProp, IntProp As DupIntProp, IntProp, IntProp, IntProp + 10, IntProp + 10 IntPlus10, IntProp + IntProp, IntProp + IntProp [IntProp plus IntProp] FROM ts.Foo"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    JsonECSqlSelectAdapter defaultAdapter(stmt);
    JsonECSqlSelectAdapter javaScriptAdapter(stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsDecimalString));

    JsonDoc defaultJson, javaScriptJson;
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson.RapidJson(), defaultJson.Allocator()));
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson.JsonCpp()));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson.RapidJson(), javaScriptJson.Allocator()));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson.JsonCpp()));

    {
    ASSERT_TRUE_NULLABLE(defaultJson.IsObject()) << defaultJson.ToString();
    ASSERT_EQ_NULLABLE(stmt.GetColumnCount(), defaultJson.MemberCount()) << stmt.GetECSql() << " - " << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.IsObject()) << javaScriptJson.ToString();
    ASSERT_EQ_NULLABLE(stmt.GetColumnCount(), javaScriptJson.MemberCount()) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("IntProp")) << stmt.GetECSql() << " - " << defaultJson.ToString();
    ASSERT_EQ_NULLABLE(1000, defaultJson["IntProp"].GetInt()) << stmt.GetECSql() << " - " << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("intProp")) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    ASSERT_EQ_NULLABLE(1000, javaScriptJson["intProp"].GetInt()) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("DupIntProp")) << stmt.GetECSql() << " - " << defaultJson.ToString();
    ASSERT_EQ_NULLABLE(1000, defaultJson["DupIntProp"].GetInt()) << stmt.GetECSql() << " - " << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("dupIntProp")) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    ASSERT_EQ_NULLABLE(1000, javaScriptJson["dupIntProp"].GetInt()) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("IntProp_1")) << stmt.GetECSql() << " - " << defaultJson.ToString();
    ASSERT_EQ_NULLABLE(1000, defaultJson["IntProp_1"].GetInt()) << stmt.GetECSql() << " - " << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("intProp_1")) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    ASSERT_EQ_NULLABLE(1000, javaScriptJson["intProp_1"].GetInt()) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("IntProp_2")) << stmt.GetECSql() << " - " << defaultJson.ToString();
    ASSERT_EQ_NULLABLE(1000, defaultJson["IntProp_2"].GetInt()) << stmt.GetECSql() << " - " << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("intProp_2")) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    ASSERT_EQ_NULLABLE(1000, javaScriptJson["intProp_2"].GetInt()) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("[IntProp] + 10")) << stmt.GetECSql() << " - " << defaultJson.ToString();
    ASSERT_DOUBLE_EQ_NULLABLE(1010, defaultJson["[IntProp] + 10"].GetDouble()) << stmt.GetECSql() << " - " << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("[IntProp] + 10")) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    ASSERT_DOUBLE_EQ_NULLABLE(1010, javaScriptJson["[IntProp] + 10"].GetDouble()) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("IntPlus10")) << stmt.GetECSql() << " - " << defaultJson.ToString();
    ASSERT_DOUBLE_EQ_NULLABLE(1010, defaultJson["IntPlus10"].GetDouble()) << stmt.GetECSql() << " - " << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("intPlus10")) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    ASSERT_DOUBLE_EQ_NULLABLE(1010, javaScriptJson["intPlus10"].GetDouble()) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("[IntProp] + [IntProp]")) << stmt.GetECSql() << " - " << defaultJson.ToString();
    ASSERT_DOUBLE_EQ_NULLABLE(2000, defaultJson["[IntProp] + [IntProp]"].GetDouble()) << stmt.GetECSql() << " - " << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("[IntProp] + [IntProp]")) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    ASSERT_DOUBLE_EQ_NULLABLE(2000, javaScriptJson["[IntProp] + [IntProp]"].GetDouble()) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("IntProp plus IntProp")) << stmt.GetECSql() << " - " << defaultJson.ToString();
    ASSERT_DOUBLE_EQ_NULLABLE(2000, defaultJson["IntProp plus IntProp"].GetDouble()) << stmt.GetECSql() << " - " << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("intProp plus IntProp")) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    ASSERT_DOUBLE_EQ_NULLABLE(2000, javaScriptJson["intProp plus IntProp"].GetDouble()) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    }
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT PtProp.X, PtProp.Y, PtProp.Z FROM ts.Foo"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    JsonECSqlSelectAdapter defaultAdapter(stmt);
    JsonECSqlSelectAdapter javaScriptAdapter(stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsDecimalString));

    JsonDoc defaultJson, javaScriptJson;
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson.RapidJson(), defaultJson.Allocator()));
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson.JsonCpp()));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson.RapidJson(), javaScriptJson.Allocator()));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson.JsonCpp()));

    {
    ASSERT_TRUE_NULLABLE(defaultJson.IsObject()) << defaultJson.ToString();
    ASSERT_EQ_NULLABLE(stmt.GetColumnCount(), defaultJson.MemberCount()) << stmt.GetECSql() << " - " << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.IsObject()) << javaScriptJson.ToString();
    ASSERT_EQ_NULLABLE(stmt.GetColumnCount(), javaScriptJson.MemberCount()) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("PtProp.X")) << stmt.GetECSql() << " - " << defaultJson.ToString();
    ASSERT_DOUBLE_EQ_NULLABLE(1000.0, defaultJson["PtProp.X"].GetDouble()) << stmt.GetECSql() << " - " << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("PtProp.Y")) << stmt.GetECSql() << " - " << defaultJson.ToString();
    ASSERT_DOUBLE_EQ_NULLABLE(1000.0, defaultJson["PtProp.Y"].GetDouble()) << stmt.GetECSql() << " - " << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("PtProp.Z")) << stmt.GetECSql() << " - " << defaultJson.ToString();
    ASSERT_DOUBLE_EQ_NULLABLE(1000.0, defaultJson["PtProp.Z"].GetDouble()) << stmt.GetECSql() << " - " << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("ptProp.X")) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    ASSERT_DOUBLE_EQ_NULLABLE(1000.0, javaScriptJson["ptProp.X"].GetDouble()) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("ptProp.Y")) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    ASSERT_DOUBLE_EQ_NULLABLE(1000.0, javaScriptJson["ptProp.Y"].GetDouble()) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("ptProp.Z")) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    ASSERT_DOUBLE_EQ_NULLABLE(1000.0, javaScriptJson["ptProp.Z"].GetDouble()) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    }
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT StructProp.SomeNumber FROM ts.Foo"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    JsonECSqlSelectAdapter defaultAdapter(stmt);
    JsonECSqlSelectAdapter javaScriptAdapter(stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsDecimalString));

    JsonDoc defaultJson, javaScriptJson;
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson.RapidJson(), defaultJson.Allocator()));
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson.JsonCpp()));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson.RapidJson(), javaScriptJson.Allocator()));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson.JsonCpp()));

    {
    ASSERT_TRUE_NULLABLE(defaultJson.IsObject()) << defaultJson.ToString();
    ASSERT_DOUBLE_EQ_NULLABLE(stmt.GetColumnCount(), defaultJson.MemberCount()) << stmt.GetECSql() << " - " << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.IsObject()) << javaScriptJson.ToString();
    ASSERT_DOUBLE_EQ_NULLABLE(stmt.GetColumnCount(), javaScriptJson.MemberCount()) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("StructProp.SomeNumber")) << stmt.GetECSql() << " - " << defaultJson.ToString();
    ASSERT_DOUBLE_EQ_NULLABLE(1000.0, defaultJson["StructProp.SomeNumber"].GetDouble()) << stmt.GetECSql() << " - " << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("structProp.SomeNumber")) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    ASSERT_DOUBLE_EQ_NULLABLE(1000.0, javaScriptJson["structProp.SomeNumber"].GetDouble()) << stmt.GetECSql() << " - " << javaScriptJson.ToString();
    }
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                09/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonECSqlSelectAdapterTests, ReservedWordsCollisions)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("jsonecsqlselectadapter_ReservedWordsCollisions.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                                                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                                                            <ECEntityClass typeName="Foo" >
                                                                                <ECProperty propertyName="className" typeName="string" />
                                                                            </ECEntityClass>
                                                                            <ECRelationshipClass typeName="Rel" strength="referencing" modifier="None">
                                                                                <ECProperty propertyName="sourceClassName" typeName="string" />
                                                                                <ECProperty propertyName="targetClassName" typeName="string" />
                                                                                <Source multiplicity="(0..*)" roleLabel="has" polymorphic="False"><Class class="Foo"/></Source>
                                                                                <Target multiplicity="(0..*)" roleLabel="referenced by" polymorphic="False"><Class class="Foo"/></Target>
                                                                            </ECRelationshipClass>
                                                                        </ECSchema>)xml")));
    ECInstanceKey fooKey, relKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(fooKey, "INSERT INTO ts.Foo(className) VALUES('Foo class')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(relKey, Utf8PrintfString("INSERT INTO ts.Rel(SourceECInstanceId, TargetECInstanceId, sourceClassName, targetClassName) VALUES(%s,%s,'source','target')",
                                                                                      fooKey.GetInstanceId().ToString().c_str(), fooKey.GetInstanceId().ToString().c_str()).c_str()));

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECClassId, className FROM ts.Foo"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    JsonECSqlSelectAdapter defaultAdapter(stmt);
    JsonECSqlSelectAdapter javaScriptAdapter(stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsDecimalString));
    JsonDoc defaultJson, javaScriptJson;
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson.RapidJson(), defaultJson.Allocator()));
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson.JsonCpp()));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson.RapidJson(), javaScriptJson.Allocator()));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson.JsonCpp()));

    {
    ASSERT_TRUE_NULLABLE(defaultJson.IsObject()) << defaultJson.ToString();
    ASSERT_EQ_NULLABLE(2, defaultJson.MemberCount()) << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson.IsObject()) << javaScriptJson.ToString();
    ASSERT_EQ_NULLABLE(2, javaScriptJson.MemberCount()) << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember(ECJsonSystemNames::ClassName())) << defaultJson.ToString();
    ASSERT_STREQ("TestSchema.Foo", defaultJson[ECJsonSystemNames::ClassName()].GetString()) << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember(ECJsonSystemNames::ClassName())) << javaScriptJson.ToString();
    ASSERT_STREQ("TestSchema.Foo", javaScriptJson[ECJsonSystemNames::ClassName()].GetString()) << javaScriptJson.ToString();
    }

    {
    Utf8String className1(ECJsonSystemNames::ClassName());
    className1.append("_1");
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember(className1)) << defaultJson.ToString();
    ASSERT_STREQ("Foo class", defaultJson[className1].GetString()) << defaultJson.ToString();

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember(className1)) << javaScriptJson.ToString();
    ASSERT_STREQ("Foo class", javaScriptJson[className1].GetString()) << javaScriptJson.ToString();
    }
    }
    {
    //now using aliases to avoid collision
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECClassId AS SystemClassName, className FROM ts.Foo"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    JsonECSqlSelectAdapter defaultAdapter(stmt);
    JsonECSqlSelectAdapter javaScriptAdapter(stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsDecimalString));
    JsonDoc defaultJson, javaScriptJson;
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson.RapidJson(), defaultJson.Allocator()));
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson.JsonCpp()));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson.RapidJson(), javaScriptJson.Allocator()));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson.JsonCpp()));

    {
    ASSERT_TRUE_NULLABLE(defaultJson.IsObject()) << defaultJson.ToString();
    ASSERT_EQ_NULLABLE(2, defaultJson.MemberCount()) << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson.IsObject()) << javaScriptJson.ToString();
    ASSERT_EQ_NULLABLE(2, javaScriptJson.MemberCount()) << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("SystemClassName")) << defaultJson.ToString();
    ASSERT_STREQ(fooKey.GetClassId().ToString().c_str(), defaultJson["SystemClassName"].GetString()) << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("systemClassName")) << javaScriptJson.ToString();
    ASSERT_STREQ(fooKey.GetClassId().ToString().c_str(), javaScriptJson["systemClassName"].GetString()) << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("className")) << defaultJson.ToString();
    ASSERT_STREQ("Foo class", defaultJson["className"].GetString()) << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("className")) << javaScriptJson.ToString();
    ASSERT_STREQ("Foo class", javaScriptJson["className"].GetString()) << javaScriptJson.ToString();
    }
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECClassId, className AS UserClassName FROM ts.Foo"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    JsonECSqlSelectAdapter defaultAdapter(stmt);
    JsonECSqlSelectAdapter javaScriptAdapter(stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsDecimalString));
    JsonDoc defaultJson, javaScriptJson;
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson.RapidJson(), defaultJson.Allocator()));
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson.JsonCpp()));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson.RapidJson(), javaScriptJson.Allocator()));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson.JsonCpp()));

    {
    ASSERT_TRUE_NULLABLE(defaultJson.IsObject()) << defaultJson.ToString();
    ASSERT_EQ_NULLABLE(2, defaultJson.MemberCount()) << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson.IsObject()) << javaScriptJson.ToString();
    ASSERT_EQ_NULLABLE(2, javaScriptJson.MemberCount()) << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("className")) << defaultJson.ToString();
    ASSERT_STREQ("TestSchema.Foo", defaultJson["className"].GetString()) << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("className")) << javaScriptJson.ToString();
    ASSERT_STREQ("TestSchema.Foo", javaScriptJson["className"].GetString()) << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("UserClassName")) << defaultJson.ToString();
    ASSERT_STREQ("Foo class", defaultJson["UserClassName"].GetString()) << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("userClassName")) << javaScriptJson.ToString();
    ASSERT_STREQ("Foo class", javaScriptJson["userClassName"].GetString()) << javaScriptJson.ToString();
    }
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SourceECClassId, sourceClassName, TargetECClassId, targetClassName FROM ts.Rel"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    JsonECSqlSelectAdapter defaultAdapter(stmt);
    JsonECSqlSelectAdapter javaScriptAdapter(stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsDecimalString));
    JsonDoc defaultJson, javaScriptJson;
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson.RapidJson(), defaultJson.Allocator()));
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson.JsonCpp()));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson.RapidJson(), javaScriptJson.Allocator()));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson.JsonCpp()));

    {
    ASSERT_TRUE_NULLABLE(defaultJson.IsObject()) << defaultJson.ToString();
    ASSERT_EQ_NULLABLE(4, defaultJson.MemberCount()) << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson.IsObject()) << javaScriptJson.ToString();
    ASSERT_EQ_NULLABLE(4, javaScriptJson.MemberCount()) << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember(ECJsonUtilities::json_sourceClassName())) << defaultJson.ToString();
    ASSERT_STREQ("TestSchema.Foo", defaultJson[ECJsonUtilities::json_sourceClassName()].GetString()) << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember(ECJsonUtilities::json_sourceClassName())) << javaScriptJson.ToString();
    ASSERT_STREQ("TestSchema.Foo", javaScriptJson[ECJsonUtilities::json_sourceClassName()].GetString()) << javaScriptJson.ToString();
    }

    {
    Utf8String sourceClassName1(ECJsonUtilities::json_sourceClassName());
    sourceClassName1.append("_1");
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember(sourceClassName1)) << defaultJson.ToString();
    ASSERT_STREQ("source", defaultJson[sourceClassName1].GetString()) << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember(sourceClassName1)) << javaScriptJson.ToString();
    ASSERT_STREQ("source", javaScriptJson[sourceClassName1].GetString()) << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember(ECJsonUtilities::json_targetClassName())) << defaultJson.ToString();
    ASSERT_STREQ("TestSchema.Foo", defaultJson[ECJsonUtilities::json_targetClassName()].GetString()) << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember(ECJsonUtilities::json_targetClassName())) << javaScriptJson.ToString();
    ASSERT_STREQ("TestSchema.Foo", javaScriptJson[ECJsonUtilities::json_targetClassName()].GetString()) << javaScriptJson.ToString();
    }

    {
    Utf8String targetClassName1(ECJsonUtilities::json_targetClassName());
    targetClassName1.append("_1");
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember(targetClassName1)) << defaultJson.ToString();
    ASSERT_STREQ("target", defaultJson[targetClassName1].GetString()) << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember(targetClassName1)) << javaScriptJson.ToString();
    ASSERT_STREQ("target", javaScriptJson[targetClassName1].GetString()) << javaScriptJson.ToString();
    }
    }

    {
    //now using aliases to avoid collision
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SourceECClassId SystemSourceClassName, sourceClassName, TargetECClassId SystemTargetClassName, targetClassName FROM ts.Rel"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    JsonECSqlSelectAdapter defaultAdapter(stmt);
    JsonECSqlSelectAdapter javaScriptAdapter(stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsDecimalString));
    JsonDoc defaultJson, javaScriptJson;
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson.RapidJson(), defaultJson.Allocator()));
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson.JsonCpp()));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson.RapidJson(), javaScriptJson.Allocator()));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson.JsonCpp()));

    {
    ASSERT_TRUE_NULLABLE(defaultJson.IsObject()) << defaultJson.ToString();
    ASSERT_EQ_NULLABLE(4, defaultJson.MemberCount()) << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson.IsObject()) << javaScriptJson.ToString();
    ASSERT_EQ_NULLABLE(4, javaScriptJson.MemberCount()) << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("SystemSourceClassName")) << defaultJson.ToString();
    ASSERT_STREQ(fooKey.GetClassId().ToString().c_str(), defaultJson["SystemSourceClassName"].GetString()) << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("systemSourceClassName")) << javaScriptJson.ToString();
    ASSERT_STREQ(fooKey.GetClassId().ToString().c_str(), javaScriptJson["systemSourceClassName"].GetString()) << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("sourceClassName")) << defaultJson.ToString();
    ASSERT_STREQ("source", defaultJson["sourceClassName"].GetString()) << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("sourceClassName")) << javaScriptJson.ToString();
    ASSERT_STREQ("source", javaScriptJson["sourceClassName"].GetString()) << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("SystemTargetClassName")) << defaultJson.ToString();
    ASSERT_STREQ(fooKey.GetClassId().ToString().c_str(), defaultJson["SystemTargetClassName"].GetString()) << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("systemTargetClassName")) << javaScriptJson.ToString();
    ASSERT_STREQ(fooKey.GetClassId().ToString().c_str(), javaScriptJson["systemTargetClassName"].GetString()) << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("targetClassName")) << defaultJson.ToString();
    ASSERT_STREQ("target", defaultJson["targetClassName"].GetString()) << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("targetClassName")) << javaScriptJson.ToString();
    ASSERT_STREQ("target", javaScriptJson["targetClassName"].GetString()) << javaScriptJson.ToString();
    }
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SourceECClassId, sourceClassName UserSourceClass, TargetECClassId, targetClassName UserTargetClass FROM ts.Rel"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    JsonECSqlSelectAdapter defaultAdapter(stmt);
    JsonECSqlSelectAdapter javaScriptAdapter(stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsDecimalString));
    JsonDoc defaultJson, javaScriptJson;
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson.RapidJson(), defaultJson.Allocator()));
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson.JsonCpp()));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson.RapidJson(), javaScriptJson.Allocator()));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson.JsonCpp()));

    {
    ASSERT_TRUE_NULLABLE(defaultJson.IsObject()) << defaultJson.ToString();
    ASSERT_EQ_NULLABLE(4, defaultJson.MemberCount()) << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson.IsObject()) << javaScriptJson.ToString();
    ASSERT_EQ_NULLABLE(4, javaScriptJson.MemberCount()) << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember(ECJsonUtilities::json_sourceClassName())) << defaultJson.ToString();
    ASSERT_STREQ("TestSchema.Foo", defaultJson[ECJsonUtilities::json_sourceClassName()].GetString()) << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember(ECJsonUtilities::json_sourceClassName())) << javaScriptJson.ToString();
    ASSERT_STREQ("TestSchema.Foo", javaScriptJson[ECJsonUtilities::json_sourceClassName()].GetString()) << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("UserSourceClass")) << defaultJson.ToString();
    ASSERT_STREQ("source", defaultJson["UserSourceClass"].GetString()) << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("userSourceClass")) << javaScriptJson.ToString();
    ASSERT_STREQ("source", javaScriptJson["userSourceClass"].GetString()) << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember(ECJsonUtilities::json_targetClassName())) << defaultJson.ToString();
    ASSERT_STREQ("TestSchema.Foo", defaultJson[ECJsonUtilities::json_targetClassName()].GetString()) << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember(ECJsonUtilities::json_targetClassName())) << javaScriptJson.ToString();
    ASSERT_STREQ("TestSchema.Foo", javaScriptJson[ECJsonUtilities::json_targetClassName()].GetString()) << javaScriptJson.ToString();
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("UserTargetClass")) << defaultJson.ToString();
    ASSERT_STREQ("target", defaultJson["UserTargetClass"].GetString()) << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("userTargetClass")) << javaScriptJson.ToString();
    ASSERT_STREQ("target", javaScriptJson["userTargetClass"].GetString()) << javaScriptJson.ToString();
    }
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                09/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonECSqlSelectAdapterTests, DataTypes)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("jsonecsqlselectadapter_datatypes.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                                                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                                                            <ECStructClass typeName="Person" >
                                                                                <ECProperty propertyName="Name" typeName="string" />
                                                                                <ECProperty propertyName="Age" typeName="int" />
                                                                            </ECStructClass>
                                                                            <ECEntityClass typeName="Foo" >
                                                                                <ECProperty propertyName="B" typeName="boolean" />
                                                                                <ECProperty propertyName="Bi" typeName="binary" />
                                                                                <ECProperty propertyName="D" typeName="double" />
                                                                                <ECProperty propertyName="Dt" typeName="dateTime" />
                                                                                <ECProperty propertyName="G" typeName="Bentley.Geometry.Common.IGeometry" />
                                                                                <ECProperty propertyName="I" typeName="int" />
                                                                                <ECProperty propertyName="L" typeName="long" />
                                                                                <ECProperty propertyName="P2D" typeName="Point2d" />
                                                                                <ECProperty propertyName="P3D" typeName="Point3d" />
                                                                                <ECProperty propertyName="S" typeName="String" />
                                                                                <ECArrayProperty propertyName="D_Array" typeName="double" />
                                                                                <ECStructProperty propertyName="Struct" typeName="Person" />
                                                                                <ECStructArrayProperty propertyName="Struct_Array" typeName="Person" />
                                                                                <ECNavigationProperty propertyName="Parent" relationshipName="ParentHasFoo" direction="Backward" />
                                                                            </ECEntityClass>
                                                                            <ECEntityClass typeName="Parent" />
                                                                            <ECRelationshipClass typeName="ParentHasFoo" strength="referencing" modifier="None">
                                                                                <Source multiplicity="(0..1)" roleLabel="has" polymorphic="False"><Class class="Parent"/></Source>
                                                                                <Target multiplicity="(0..*)" roleLabel="referenced by" polymorphic="False"><Class class="Foo"/></Target>
                                                                            </ECRelationshipClass>
                                                                        </ECSchema>)xml")));

    ECInstanceKey parentKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(parentKey, "INSERT INTO ts.Parent(ECInstanceId) VALUES(NULL)"));

    const bool boolVal = true;
    const double doubleVal = 3.14;
    const DateTime dtVal = DateTime(DateTime::Kind::Unspecified, 2017,9,18,13,40);
    const int intVal = 123;
    const int64_t int64Val = INT64_C(123123123123);
    Utf8String int64Str;
    int64Str.Sprintf("%" PRIi64, int64Val);

    const DPoint2d p2dVal = DPoint2d::From(-1.1, 2.5);
    const DPoint3d p3dVal = DPoint3d::From(1.0, 2.0, -3.0);
    IGeometryPtr geomVal = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));

    void const* blobVal = &int64Val;
    const size_t blobSize = sizeof(int64Val);
    Utf8String blobValBase64Str;
    Base64Utilities::Encode(blobValBase64Str, (Byte const*) blobVal, blobSize);

    Utf8CP stringVal = "Hello, world";
    Utf8CP personName = "Johnny";
    const int personAge = 14;


    ECInstanceKey key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Foo(B,Bi,D,Dt,G,I,L,P2D,P3D,S,Struct,D_Array,Struct_Array,Parent) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBoolean(1, boolVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBlob(2, blobVal, blobSize, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(3, doubleVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(4, dtVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindGeometry(5, *geomVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(6, intVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(7, int64Val));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(8, p2dVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(9, p3dVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(10, stringVal, IECSqlBinder::MakeCopy::No));

    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(11)["Name"].BindText(personName, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(11)["Age"].BindInt(personAge));

    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(12).AddArrayElement().BindDouble(doubleVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(12).AddArrayElement().BindDouble(doubleVal));

    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(13).AddArrayElement()["Name"].BindText(personName, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(13).AddArrayElement()["Name"].BindText(personName, IECSqlBinder::MakeCopy::No));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(14, parentKey.GetInstanceId(), m_ecdb.Schemas().GetClassId("TestSchema", "ParentHasFoo")));

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId,ECClassId,B,Bi,D,Dt,G,I,L,P2D,P3D,S,Struct,D_Array,Struct_Array,Parent FROM ts.Foo WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    JsonECSqlSelectAdapter adapter(stmt);
    JsonDoc actualJson;
    ASSERT_EQ(SUCCESS, adapter.GetRow(actualJson.RapidJson(), actualJson.Allocator()));
    ASSERT_EQ(SUCCESS, adapter.GetRow(actualJson.JsonCpp()));

    ASSERT_TRUE_NULLABLE(actualJson.IsObject()) << actualJson.ToString();
    ASSERT_EQ_NULLABLE(16, actualJson.MemberCount()) << actualJson.ToString();

    ASSERT_TRUE_NULLABLE(actualJson.HasMember(ECJsonSystemNames::Id())) << actualJson.ToString();
    EXPECT_STREQ(key.GetInstanceId().ToHexStr().c_str(), actualJson[ECJsonSystemNames::Id()].GetString()) << actualJson.ToString();

    ASSERT_TRUE_NULLABLE(actualJson.HasMember(ECJsonSystemNames::ClassName())) << actualJson.ToString();
    EXPECT_STREQ("TestSchema.Foo", actualJson[ECJsonSystemNames::ClassName()].GetString()) << actualJson.ToString();

    ASSERT_TRUE_NULLABLE(actualJson.HasMember("B")) << actualJson.ToString();
    EXPECT_EQ_NULLABLE(boolVal, actualJson["B"].GetBool()) << actualJson.ToString();

    ASSERT_TRUE_NULLABLE(actualJson.HasMember("Bi")) << actualJson.ToString();
    EXPECT_STREQ(blobValBase64Str.c_str(), actualJson["Bi"].GetString()) << actualJson.ToString();

    ASSERT_TRUE_NULLABLE(actualJson.HasMember("D")) << actualJson.ToString();
    EXPECT_DOUBLE_EQ_NULLABLE(doubleVal, actualJson["D"].GetDouble()) << actualJson.ToString();

    ASSERT_TRUE_NULLABLE(actualJson.HasMember("Dt")) << actualJson.ToString();
    EXPECT_STREQ(dtVal.ToString().c_str(), actualJson["Dt"].GetString()) << actualJson.ToString();

    ASSERT_TRUE_NULLABLE(actualJson.HasMember("G")) << actualJson.ToString();
    ASSERT_TRUE_NULLABLE(actualJson["G"].IsObject()) << actualJson.ToString();
    EXPECT_EQ(JsonValue("{\"LineSegment\":{\"endPoint\":[1.0,1.0,1.0],\"startPoint\":[0.0,0.0,0.0]}}"), JsonValue(actualJson["G"].JsonCpp())) << actualJson.ToString();
    EXPECT_EQ(JsonValue("{\"LineSegment\":{\"endPoint\":[1.0,1.0,1.0],\"startPoint\":[0.0,0.0,0.0]}}"), JsonValue(TestUtilities::ToString(actualJson["G"].RapidJson()))) << actualJson.ToString();

    ASSERT_TRUE_NULLABLE(actualJson.HasMember("I")) << actualJson.ToString();
    EXPECT_EQ_NULLABLE(intVal, actualJson["I"].GetInt()) << actualJson.ToString();

    ASSERT_TRUE_NULLABLE(actualJson.HasMember("L")) << actualJson.ToString();
    EXPECT_STREQ(int64Str.c_str(), actualJson["L"].GetString()) << actualJson.ToString();

    ASSERT_TRUE_NULLABLE(actualJson.HasMember("P2D")) << actualJson.ToString();
    ASSERT_TRUE_NULLABLE(actualJson["P2D"].IsObject()) << actualJson.ToString();
    EXPECT_DOUBLE_EQ_NULLABLE(p2dVal.x, actualJson["P2D"]["x"].GetDouble()) << actualJson.ToString();
    EXPECT_DOUBLE_EQ_NULLABLE(p2dVal.y, actualJson["P2D"]["y"].GetDouble()) << actualJson.ToString();

    ASSERT_TRUE_NULLABLE(actualJson.HasMember("P3D")) << actualJson.ToString();
    ASSERT_TRUE_NULLABLE(actualJson["P3D"].IsObject()) << actualJson.ToString();
    EXPECT_DOUBLE_EQ_NULLABLE(p3dVal.x, actualJson["P3D"]["x"].GetDouble()) << actualJson.ToString();
    EXPECT_DOUBLE_EQ_NULLABLE(p3dVal.y, actualJson["P3D"]["y"].GetDouble()) << actualJson.ToString();
    EXPECT_DOUBLE_EQ_NULLABLE(p3dVal.z, actualJson["P3D"]["z"].GetDouble()) << actualJson.ToString();

    ASSERT_TRUE_NULLABLE(actualJson.HasMember("S")) << actualJson.ToString();
    EXPECT_STREQ(stringVal, actualJson["S"].GetString()) << actualJson.ToString();

    ASSERT_TRUE_NULLABLE(actualJson.HasMember("Struct")) << actualJson.ToString();
    ASSERT_TRUE_NULLABLE(actualJson["Struct"].IsObject()) << actualJson.ToString();
    EXPECT_STREQ(personName, actualJson["Struct"]["Name"].GetString()) << actualJson.ToString();
    EXPECT_EQ_NULLABLE(personAge, actualJson["Struct"]["Age"].GetInt()) << actualJson.ToString();

    ASSERT_TRUE_NULLABLE(actualJson.HasMember("D_Array")) << actualJson.ToString();
    ASSERT_TRUE_NULLABLE(actualJson["D_Array"].IsArray()) << actualJson.ToString();
    EXPECT_EQ_NULLABLE(2, actualJson["D_Array"].ArraySize()) << actualJson.ToString();
    EXPECT_DOUBLE_EQ_NULLABLE(doubleVal, actualJson["D_Array"][0].GetDouble()) << actualJson.ToString();
    EXPECT_DOUBLE_EQ_NULLABLE(doubleVal, actualJson["D_Array"][1].GetDouble()) << actualJson.ToString();

    ASSERT_TRUE_NULLABLE(actualJson.HasMember("Struct_Array")) << actualJson.ToString();
    ASSERT_TRUE_NULLABLE(actualJson["Struct_Array"].IsArray()) << actualJson.ToString();
    EXPECT_EQ_NULLABLE(2, actualJson["Struct_Array"].ArraySize()) << actualJson.ToString();
    EXPECT_STREQ(personName, actualJson["Struct_Array"][0]["Name"].GetString()) << actualJson.ToString();
    EXPECT_FALSE_NULLABLE(actualJson["Struct_Array"][0].HasMember("Age")) << actualJson.ToString();
    EXPECT_STREQ(personName, actualJson["Struct_Array"][1]["Name"].GetString()) << actualJson.ToString();
    EXPECT_FALSE_NULLABLE(actualJson["Struct_Array"][1].HasMember("Age")) << actualJson.ToString();

    ASSERT_TRUE_NULLABLE(actualJson.HasMember("Parent")) << actualJson.ToString();
    ASSERT_TRUE_NULLABLE(actualJson["Parent"].IsObject()) << actualJson.ToString();
    EXPECT_STREQ(parentKey.GetInstanceId().ToHexStr().c_str(), actualJson["Parent"][ECJsonSystemNames::Navigation::Id()].GetString()) << actualJson.ToString();
    ASSERT_TRUE_NULLABLE(actualJson["Parent"].HasMember(ECJsonSystemNames::Navigation::RelClassName())) << actualJson.ToString();
    EXPECT_STREQ("TestSchema.ParentHasFoo", actualJson["Parent"][ECJsonSystemNames::Navigation::RelClassName()].GetString()) << actualJson.ToString();
    }

    // Use IModelJs adapter
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT G FROM ts.Foo WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    JsonECSqlSelectAdapter::FormatOptions options(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsHexadecimalString);
    options.SetRowFormat(JsonECSqlSelectAdapter::RowFormat::IModelJs);
    JsonECSqlSelectAdapter adapter(stmt, options);
    JsonDoc actualJson;
    ASSERT_EQ(SUCCESS, adapter.GetRow(actualJson.RapidJson(), actualJson.Allocator()));
    ASSERT_EQ(SUCCESS, adapter.GetRow(actualJson.JsonCpp()));

    ASSERT_TRUE_NULLABLE(actualJson.HasMember("g")) << actualJson.ToString();
    ASSERT_TRUE_NULLABLE(actualJson["g"].IsObject()) << actualJson.ToString();
    EXPECT_EQ(JsonValue("{\"lineSegment\":[[0.0,0.0,0.0],[1.0,1.0,1.0]]}"), JsonValue(actualJson["g"].JsonCpp())) << actualJson.ToString();
    EXPECT_EQ(JsonValue("{\"lineSegment\":[[0.0,0.0,0.0],[1.0,1.0,1.0]]}"), JsonValue(TestUtilities::ToString(actualJson["g"].RapidJson()))) << actualJson.ToString();
    }

    //SELECT FROM relationship
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, ECClassId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ts.ParentHasFoo WHERE TargetECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    JsonECSqlSelectAdapter relAdapter(stmt);
    JsonDoc actualJson;
    ASSERT_EQ(SUCCESS, relAdapter.GetRow(actualJson.RapidJson(), actualJson.Allocator()));
    ASSERT_EQ(SUCCESS, relAdapter.GetRow(actualJson.JsonCpp()));

    ASSERT_TRUE_NULLABLE(actualJson.IsObject()) << actualJson.ToString();
    EXPECT_EQ_NULLABLE(6, actualJson.MemberCount()) << actualJson.ToString();

    ASSERT_TRUE_NULLABLE(actualJson.HasMember(ECJsonSystemNames::Id())) << actualJson.ToString();
    EXPECT_STREQ(key.GetInstanceId().ToHexStr().c_str(), actualJson[ECJsonSystemNames::Id()].GetString()) << actualJson.ToString();

    ASSERT_TRUE_NULLABLE(actualJson.HasMember(ECJsonSystemNames::ClassName())) << actualJson.ToString();
    EXPECT_STREQ("TestSchema.ParentHasFoo", actualJson[ECJsonSystemNames::ClassName()].GetString()) << actualJson.ToString();

    ASSERT_TRUE_NULLABLE(actualJson.HasMember(ECJsonSystemNames::SourceId())) << actualJson.ToString();
    EXPECT_STREQ(parentKey.GetInstanceId().ToHexStr().c_str(), actualJson[ECJsonSystemNames::SourceId()].GetString()) << actualJson.ToString();

    ASSERT_TRUE_NULLABLE(actualJson.HasMember(ECJsonSystemNames::SourceClassName())) << actualJson.ToString();
    EXPECT_STREQ("TestSchema.Parent", actualJson[ECJsonSystemNames::SourceClassName()].GetString()) << actualJson.ToString();

    ASSERT_TRUE_NULLABLE(actualJson.HasMember(ECJsonSystemNames::TargetId())) << actualJson.ToString();
    EXPECT_STREQ(key.GetInstanceId().ToHexStr().c_str(), actualJson[ECJsonSystemNames::TargetId()].GetString()) << actualJson.ToString();

    ASSERT_TRUE_NULLABLE(actualJson.HasMember(ECJsonSystemNames::TargetClassName())) << actualJson.ToString();
    EXPECT_STREQ("TestSchema.Foo", actualJson[ECJsonSystemNames::TargetClassName()].GetString()) << actualJson.ToString();
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                09/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonECSqlSelectAdapterTests, LongDataType)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("jsonecsqlselectadapter_longtype.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                                    <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                                        <ECEntityClass typeName="Foo" >
                                                            <ECProperty propertyName="L" typeName="long" />
                                                        </ECEntityClass>
                                                    </ECSchema>)xml")));

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key, "INSERT INTO ts.Foo(L) VALUES(1234567890)"));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT L FROM ts.Foo WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    JsonECSqlSelectAdapter adapter1(stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsNumber));
    JsonDoc actualJson;
    ASSERT_EQ(SUCCESS, adapter1.GetRow(actualJson.RapidJson(), actualJson.Allocator()));
    ASSERT_EQ(SUCCESS, adapter1.GetRow(actualJson.JsonCpp()));

    ASSERT_TRUE_NULLABLE(actualJson.IsObject()) << actualJson.ToString();
    EXPECT_EQ_NULLABLE(1, actualJson.MemberCount()) << actualJson.ToString();

    ASSERT_TRUE_NULLABLE(actualJson.HasMember("L")) << actualJson.ToString();
    EXPECT_EQ_NULLABLE(INT64_C(1234567890), actualJson["L"].GetInt64()) << "ECJsonInt64Format::AsNumber " << actualJson.ToString();

    JsonECSqlSelectAdapter adapter2(stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsDecimalString));
    actualJson.Clear();
    ASSERT_EQ(SUCCESS, adapter2.GetRow(actualJson.RapidJson(), actualJson.Allocator()));
    ASSERT_EQ(SUCCESS, adapter2.GetRow(actualJson.JsonCpp()));

    ASSERT_TRUE_NULLABLE(actualJson.IsObject()) << actualJson.ToString();
    EXPECT_EQ_NULLABLE(1, actualJson.MemberCount()) << actualJson.ToString();

    ASSERT_TRUE_NULLABLE(actualJson.HasMember("L")) << actualJson.ToString();
    ASSERT_TRUE_NULLABLE(actualJson["L"].IsString()) << actualJson.ToString();
    ASSERT_STREQ("1234567890", actualJson["L"].GetString()) << "ECJsonInt64Format::AsDecimalString " << actualJson.ToString();

    JsonECSqlSelectAdapter adapter3(stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsHexadecimalString));
    actualJson.Clear();
    ASSERT_EQ(SUCCESS, adapter3.GetRow(actualJson.RapidJson(), actualJson.Allocator()));
    ASSERT_EQ(SUCCESS, adapter3.GetRow(actualJson.JsonCpp()));
    ASSERT_TRUE_NULLABLE(actualJson.IsObject()) << actualJson.ToString();
    EXPECT_EQ_NULLABLE(1, actualJson.MemberCount()) << actualJson.ToString();

    ASSERT_TRUE_NULLABLE(actualJson.HasMember("L")) << actualJson.ToString();
    ASSERT_TRUE_NULLABLE(actualJson["L"].IsString()) << actualJson.ToString();
    ASSERT_STREQ(BeInt64Id(1234567890).ToHexStr().c_str(), actualJson["L"].GetString()) << "ECJsonInt64Format::AsHexadecimalString " << actualJson.ToString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonECSqlSelectAdapterTests, JsonStructAndArrays)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("jsonecsqlselectadapter_JsonStruct.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                                    <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                                        <ECStructClass typeName="MyStruct" >
                                                            <ECProperty propertyName="Alpha" typeName="int" />
                                                            <ECProperty propertyName="Beta" typeName="double" />
                                                        </ECStructClass>
                                                        <ECEntityClass typeName="Foo" >
                                                            <ECStructProperty propertyName="StructProp" typeName="MyStruct" />
                                                            <ECArrayProperty propertyName="DoubleArray" typeName="double" />
                                                            <ECStructArrayProperty propertyName="StructArray" typeName="MyStruct" />
                                                        </ECEntityClass>
                                                    </ECSchema>)xml")));

    ECInstanceKey key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Foo(StructProp,DoubleArray,StructArray) VALUES(?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1)["Alpha"].BindInt(100));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(1)["Beta"].BindDouble(1.5));

    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(2).AddArrayElement().BindDouble(1.5));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(2).AddArrayElement().BindDouble(2.5));

    IECSqlBinder& structElement1Binder = stmt.GetBinder(3).AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, structElement1Binder["Alpha"].BindInt(100));
    ASSERT_EQ(ECSqlStatus::Success, structElement1Binder["Beta"].BindDouble(1.5));

    IECSqlBinder& structElement2Binder = stmt.GetBinder(3).AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, structElement2Binder["Alpha"].BindInt(200));
    ASSERT_EQ(ECSqlStatus::Success, structElement2Binder["Beta"].BindDouble(2.5));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    m_ecdb.SaveChanges();
    }

    /* Retrieve the JSON for the inserted Instance */
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ECInstanceId,ECClassId,StructProp,DoubleArray,StructArray FROM ONLY ts.Foo LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());

    JsonDoc actualDefaultJson, actualJavaScriptJson;

    JsonECSqlSelectAdapter defaultAdapter(statement);
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(actualDefaultJson.RapidJson(), actualDefaultJson.Allocator()));
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(actualDefaultJson.JsonCpp()));

    JsonECSqlSelectAdapter jsAdapter(statement, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsDecimalString));
    ASSERT_EQ(SUCCESS, jsAdapter.GetRow(actualJavaScriptJson.RapidJson(), actualJavaScriptJson.Allocator()));
    ASSERT_EQ(SUCCESS, jsAdapter.GetRow(actualJavaScriptJson.JsonCpp()));

    statement.Finalize();

    Utf8CP expectedDefaultJsonStr = R"json({
      "id" : "0x1",
      "className" : "TestSchema.Foo",
      "StructProp" : { "Alpha" : 100, "Beta" : 1.5 },
      "DoubleArray" : [ 1.5, 2.5],
      "StructArray" : [ {"Alpha" : 100, "Beta" : 1.5}, {"Alpha" : 200, "Beta" : 2.5}]
      })json";

    Utf8CP expectedJavaScriptJsonStr = R"json({
      "id" : "0x1",
      "className" : "TestSchema.Foo",
      "structProp" : { "alpha" : 100, "beta" : 1.5 },
      "doubleArray" : [ 1.5, 2.5],
      "structArray" : [ {"alpha" : 100, "beta" : 1.5}, {"alpha" : 200, "beta" : 2.5}]
      })json";

    JsonDoc expectedDefaultJson;
    ASSERT_EQ(SUCCESS, TestUtilities::ParseJson(expectedDefaultJson.RapidJson(), expectedDefaultJsonStr));
    ASSERT_EQ(SUCCESS, TestUtilities::ParseJson(expectedDefaultJson.JsonCpp(), expectedDefaultJsonStr));

    JsonDoc expectedJavaScriptJson;
    ASSERT_EQ(SUCCESS, TestUtilities::ParseJson(expectedJavaScriptJson.RapidJson(), expectedJavaScriptJsonStr));
    ASSERT_EQ(SUCCESS, TestUtilities::ParseJson(expectedJavaScriptJson.JsonCpp(), expectedJavaScriptJsonStr));

    ASSERT_TRUE_NULLABLE(expectedDefaultJson.Equals(actualDefaultJson)) << "Expected: " << expectedDefaultJson.ToString() << " | Actual: " << actualDefaultJson.ToString();
    ASSERT_TRUE_NULLABLE(expectedJavaScriptJson.Equals(actualJavaScriptJson)) << "Expected: " << expectedDefaultJson.ToString() << " | Actual: " << actualDefaultJson.ToString();
    }

struct JsonReaderTests : public ECDbTestFixture {};


//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonReaderTests, PartialPoints)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("jsonreaderpartialpoints.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(P2D.X,P3D.Y,PStructProp.p2d.y,PStructProp.p3d.z) VALUES(1.0, 2.0, 3.0, 4.0)"));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
    stmt.Finalize();

    ECSqlStatement selStmt;
    ASSERT_EQ(ECSqlStatus::Success, selStmt.Prepare(m_ecdb, "SELECT P2D,P3D,PStructProp.p2d,PStructProp.p3d FROM ecsql.PSA WHERE ECInstanceId=?"));

    ASSERT_EQ(ECSqlStatus::Success, selStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, selStmt.Step());

    JsonDoc defaultJson, javaScriptJson;
    JsonECSqlSelectAdapter defaultAdapter(selStmt);
    JsonECSqlSelectAdapter javaScriptAdapter(selStmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsDecimalString));

    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson.RapidJson(), defaultJson.Allocator()));
    ASSERT_EQ(SUCCESS, defaultAdapter.GetRow(defaultJson.JsonCpp()));

    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson.RapidJson(), javaScriptJson.Allocator()));
    ASSERT_EQ(SUCCESS, javaScriptAdapter.GetRow(javaScriptJson.JsonCpp()));
    selStmt.Finalize();

    ASSERT_TRUE_NULLABLE(defaultJson.IsObject()) << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson.IsObject()) << javaScriptJson.ToString();

    //ECSqlStatement fills the NULL coordinates with the SQLite defaults for NULL which is 0
    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("P2D"));
    ASSERT_TRUE_NULLABLE(defaultJson["P2D"].IsObject());
    EXPECT_DOUBLE_EQ_NULLABLE(1.0, defaultJson["P2D"][ECJsonSystemNames::Point::X()].GetDouble());
    EXPECT_DOUBLE_EQ_NULLABLE(0.0, defaultJson["P2D"][ECJsonSystemNames::Point::Y()].GetDouble());

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("p2D"));
    ASSERT_TRUE_NULLABLE(javaScriptJson["p2D"].IsObject());
    EXPECT_DOUBLE_EQ_NULLABLE(1.0, javaScriptJson["p2D"][ECJsonSystemNames::Point::X()].GetDouble());
    EXPECT_DOUBLE_EQ_NULLABLE(0.0, javaScriptJson["p2D"][ECJsonSystemNames::Point::Y()].GetDouble());
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("P3D"));
    ASSERT_TRUE_NULLABLE(defaultJson["P3D"].IsObject());
    EXPECT_DOUBLE_EQ_NULLABLE(0, defaultJson["P3D"][ECJsonSystemNames::Point::X()].GetDouble());
    EXPECT_DOUBLE_EQ_NULLABLE(2, defaultJson["P3D"][ECJsonSystemNames::Point::Y()].GetDouble());
    EXPECT_DOUBLE_EQ_NULLABLE(0, defaultJson["P3D"][ECJsonSystemNames::Point::Z()].GetDouble());

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("p3D"));
    ASSERT_TRUE_NULLABLE(javaScriptJson["p3D"].IsObject());
    EXPECT_DOUBLE_EQ_NULLABLE(0, javaScriptJson["p3D"][ECJsonSystemNames::Point::X()].GetDouble());
    EXPECT_DOUBLE_EQ_NULLABLE(2, javaScriptJson["p3D"][ECJsonSystemNames::Point::Y()].GetDouble());
    EXPECT_DOUBLE_EQ_NULLABLE(0, javaScriptJson["p3D"][ECJsonSystemNames::Point::Z()].GetDouble());
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("PStructProp.p2d"));
    ASSERT_TRUE_NULLABLE(defaultJson["PStructProp.p2d"].IsObject());
    EXPECT_DOUBLE_EQ_NULLABLE(0.0, defaultJson["PStructProp.p2d"][ECJsonSystemNames::Point::X()].GetDouble());
    EXPECT_DOUBLE_EQ_NULLABLE(3.0, defaultJson["PStructProp.p2d"][ECJsonSystemNames::Point::Y()].GetDouble());

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("pStructProp.p2d"));
    ASSERT_TRUE_NULLABLE(javaScriptJson["pStructProp.p2d"].IsObject());
    EXPECT_DOUBLE_EQ_NULLABLE(0.0, javaScriptJson["pStructProp.p2d"][ECJsonSystemNames::Point::X()].GetDouble());
    EXPECT_DOUBLE_EQ_NULLABLE(3.0, javaScriptJson["pStructProp.p2d"][ECJsonSystemNames::Point::Y()].GetDouble());
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson["PStructProp.p3d"].IsObject());
    EXPECT_DOUBLE_EQ_NULLABLE(0.0, defaultJson["PStructProp.p3d"][ECJsonSystemNames::Point::X()].GetDouble());
    EXPECT_DOUBLE_EQ_NULLABLE(0.0, defaultJson["PStructProp.p3d"][ECJsonSystemNames::Point::Y()].GetDouble());
    EXPECT_DOUBLE_EQ_NULLABLE(4.0, defaultJson["PStructProp.p3d"][ECJsonSystemNames::Point::Z()].GetDouble());

    ASSERT_TRUE_NULLABLE(javaScriptJson["pStructProp.p3d"].IsObject());
    EXPECT_DOUBLE_EQ_NULLABLE(0.0, javaScriptJson["pStructProp.p3d"][ECJsonSystemNames::Point::X()].GetDouble());
    EXPECT_DOUBLE_EQ_NULLABLE(0.0, javaScriptJson["pStructProp.p3d"][ECJsonSystemNames::Point::Y()].GetDouble());
    EXPECT_DOUBLE_EQ_NULLABLE(4.0, javaScriptJson["pStructProp.p3d"][ECJsonSystemNames::Point::Z()].GetDouble());
    }

    ECClassCP testClass = m_ecdb.Schemas().GetClass("ECSqlTest", "PSA");
    ASSERT_TRUE(testClass != nullptr);
    JsonReader defaultReader(m_ecdb, *testClass);
    ASSERT_TRUE(defaultReader.IsValid());
    ASSERT_EQ(SUCCESS, defaultReader.Read(defaultJson.JsonCpp(), key.GetInstanceId()));
    ASSERT_EQ(SUCCESS, defaultReader.Read(defaultJson.RapidJson(), key.GetInstanceId(), defaultJson.Allocator()));

    JsonReader javaScriptReader(m_ecdb, *testClass, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsDecimalString));
    ASSERT_TRUE(javaScriptReader.IsValid());
    ASSERT_EQ(SUCCESS, javaScriptReader.Read(javaScriptJson.JsonCpp(), key.GetInstanceId()));
    ASSERT_EQ(SUCCESS, javaScriptReader.Read(javaScriptJson.RapidJson(), key.GetInstanceId(), javaScriptJson.Allocator()));

    ASSERT_TRUE_NULLABLE(defaultJson.IsObject()) << defaultJson.ToString();
    ASSERT_TRUE_NULLABLE(javaScriptJson.IsObject()) << javaScriptJson.ToString();

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("P2D"));
    ASSERT_TRUE_NULLABLE(defaultJson["P2D"].IsObject());
    EXPECT_DOUBLE_EQ_NULLABLE(1.0, defaultJson["P2D"][ECJsonSystemNames::Point::X()].GetDouble());
    EXPECT_DOUBLE_EQ_NULLABLE(0.0, defaultJson["P2D"][ECJsonSystemNames::Point::Y()].GetDouble());

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("p2D"));
    ASSERT_TRUE_NULLABLE(javaScriptJson["p2D"].IsObject());
    EXPECT_DOUBLE_EQ_NULLABLE(1.0, javaScriptJson["p2D"][ECJsonSystemNames::Point::X()].GetDouble());
    EXPECT_DOUBLE_EQ_NULLABLE(0.0, javaScriptJson["p2D"][ECJsonSystemNames::Point::Y()].GetDouble());
    }

    {
    ASSERT_TRUE_NULLABLE(defaultJson.HasMember("P3D"));
    ASSERT_TRUE_NULLABLE(defaultJson["P3D"].IsObject());
    EXPECT_DOUBLE_EQ_NULLABLE(0.0, defaultJson["P3D"][ECJsonSystemNames::Point::X()].GetDouble());
    EXPECT_DOUBLE_EQ_NULLABLE(2.0, defaultJson["P3D"][ECJsonSystemNames::Point::Y()].GetDouble());
    EXPECT_DOUBLE_EQ_NULLABLE(0.0, defaultJson["P3D"][ECJsonSystemNames::Point::Z()].GetDouble());

    ASSERT_TRUE_NULLABLE(javaScriptJson.HasMember("p3D"));
    ASSERT_TRUE_NULLABLE(javaScriptJson["p3D"].IsObject());
    EXPECT_DOUBLE_EQ_NULLABLE(0.0, javaScriptJson["p3D"][ECJsonSystemNames::Point::X()].GetDouble());
    EXPECT_DOUBLE_EQ_NULLABLE(2.0, javaScriptJson["p3D"][ECJsonSystemNames::Point::Y()].GetDouble());
    EXPECT_DOUBLE_EQ_NULLABLE(0.0, javaScriptJson["p3D"][ECJsonSystemNames::Point::Z()].GetDouble());
    }

    {
    EXPECT_DOUBLE_EQ_NULLABLE(0.0, defaultJson["PStructProp"]["p2d"][ECJsonSystemNames::Point::X()].GetDouble());
    EXPECT_DOUBLE_EQ_NULLABLE(3.0, defaultJson["PStructProp"]["p2d"][ECJsonSystemNames::Point::Y()].GetDouble());

    EXPECT_DOUBLE_EQ_NULLABLE(0.0, javaScriptJson["pStructProp"]["p2d"][ECJsonSystemNames::Point::X()].GetDouble());
    EXPECT_DOUBLE_EQ_NULLABLE(3.0, javaScriptJson["pStructProp"]["p2d"][ECJsonSystemNames::Point::Y()].GetDouble());
    }

    {
    EXPECT_DOUBLE_EQ_NULLABLE(0.0, defaultJson["PStructProp"]["p3d"][ECJsonSystemNames::Point::X()].GetDouble());
    EXPECT_DOUBLE_EQ_NULLABLE(0.0, defaultJson["PStructProp"]["p3d"][ECJsonSystemNames::Point::Y()].GetDouble());
    EXPECT_DOUBLE_EQ_NULLABLE(4.0, defaultJson["PStructProp"]["p3d"][ECJsonSystemNames::Point::Z()].GetDouble());

    EXPECT_DOUBLE_EQ_NULLABLE(0.0, javaScriptJson["pStructProp"]["p3d"][ECJsonSystemNames::Point::X()].GetDouble());
    EXPECT_DOUBLE_EQ_NULLABLE(0.0, javaScriptJson["pStructProp"]["p3d"][ECJsonSystemNames::Point::Y()].GetDouble());
    EXPECT_DOUBLE_EQ_NULLABLE(4.0, javaScriptJson["pStructProp"]["p3d"][ECJsonSystemNames::Point::Z()].GetDouble());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                09/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JsonReaderTests, RoundTrip_ReadThenInsert)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("jsonroundtrip_readtheninsert.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    ECInstanceKey pKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(pKey, "INSERT INTO ecsql.P(ECInstanceId) VALUES(NULL)"));

    ECClassCP psaClass = m_ecdb.Schemas().GetClass("ECSqlTest", "PSA");
    ASSERT_TRUE(psaClass != nullptr);

    const ECClassId navRelClassId = m_ecdb.Schemas().GetClassId("ECSqlTest", "PSAHasP_N1");
    ASSERT_TRUE(navRelClassId.IsValid());

    const int numericVal = 123;
    const DPoint2d pt2dVal = DPoint2d::From(1.0, 2.0);
    const DPoint3d pt3dVal = DPoint3d::From(1.0, 2.0, 3.0);
    const DateTime dtVal(DateTime::Kind::Utc, 2017, 9, 21, 10, 0);
    std::vector<Utf8CP> stringArrayVal {"Hello", "world"};

    ECInstanceKey psaKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,"INSERT INTO ecsql.PSA(I,L,P2D,P3D,DtUtc,P,S_Array,PStructProp.i) VALUES(?,?,?,?,?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, numericVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(2, (int64_t) numericVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(3, pt2dVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(4, pt3dVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(5, dtVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(6, pKey.GetInstanceId(), navRelClassId));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(7).AddArrayElement().BindText(stringArrayVal[0], IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.GetBinder(7).AddArrayElement().BindText(stringArrayVal[1], IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(8, numericVal));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(psaKey));
    }

    auto validate = [&] (ECInstanceId newId)
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ecsql.PSA WHERE ECInstanceId=?"));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, newId));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        const int colCount = stmt.GetColumnCount();
        for (int i = 0; i < colCount; i++)
            {
            Utf8StringCR propName = stmt.GetColumnInfo(i).GetProperty()->GetName();
            if (propName.EqualsIAscii("I"))
                ASSERT_EQ(numericVal, stmt.GetValueInt(i)) << propName.c_str();
            else if (propName.EqualsIAscii("L"))
                ASSERT_EQ((int64_t) numericVal, stmt.GetValueInt64(i)) << propName.c_str();
            else if (propName.EqualsIAscii("P2D"))
                ASSERT_TRUE(pt2dVal.AlmostEqual(stmt.GetValuePoint2d(i))) << propName.c_str();
            else if (propName.EqualsIAscii("P3D"))
                ASSERT_TRUE(pt3dVal.AlmostEqual(stmt.GetValuePoint3d(i))) << propName.c_str();
            else if (propName.EqualsIAscii("DtUtc"))
                ASSERT_EQ(dtVal, stmt.GetValueDateTime(i)) << propName.c_str();
            else if (propName.EqualsIAscii("P"))
                {
                ECClassId actualRelClassId;
                ASSERT_EQ(pKey.GetInstanceId(), stmt.GetValueNavigation<ECInstanceId>(i, &actualRelClassId)) << propName.c_str();
                ASSERT_EQ(navRelClassId, actualRelClassId) << propName.c_str();
                }
            else if (propName.EqualsIAscii("S_Array"))
                {
                IECSqlValue const& val = stmt.GetValue(i);
                ASSERT_EQ(2, val.GetArrayLength());
                int arrayIx = 0;
                for (IECSqlValue const& arrayEl : stmt.GetValue(i).GetArrayIterable())
                    {
                    ASSERT_STREQ(stringArrayVal[arrayIx], arrayEl.GetText()) << propName.c_str() << "[" << arrayIx << "]";
                    arrayIx++;
                    }
                }
            else if (propName.EqualsIAscii("PStructProp"))
                {
                IECSqlValue const& val = stmt.GetValue(i);
                for (IECSqlValue const& memberVal : val.GetStructIterable())
                    {
                    if (memberVal.GetColumnInfo().GetProperty()->GetName().Equals("i"))
                        ASSERT_EQ(numericVal, memberVal.GetInt()) << propName.c_str() << ".i";
                    else
                        ASSERT_TRUE(memberVal.IsNull()) << propName.c_str() << "." << memberVal.GetColumnInfo().GetProperty()->GetName().c_str();
                    }
                }
            else if (propName.EqualsIAscii("ECInstanceId"))
                ASSERT_EQ(newId, stmt.GetValueId<ECInstanceId>(i)) << "ECInstanceId";
            else if (propName.EqualsIAscii("ECClassId"))
                ASSERT_EQ(psaClass->GetId(), stmt.GetValueId<ECClassId>(i)) << "ECClassId";
            else
                ASSERT_TRUE(stmt.IsValueNull(i)) << propName;
            }
        };

    JsonInserter inserter(m_ecdb, *psaClass, nullptr);
    ECInstanceKey newKey;
    for (JsonECSqlSelectAdapter::FormatOptions const& formatOption : {JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsNumber),
                                                                      JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsNumber),
                                                                     JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsDecimalString),
                                                                     JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsDecimalString),
                                                                     JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsHexadecimalString),
                                                                     JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsHexadecimalString)})
        {
        JsonReader reader(m_ecdb, psaKey.GetClassId(), formatOption);
        ASSERT_TRUE(reader.IsValid());

        JsonDoc actualJson;
        ASSERT_EQ(SUCCESS, reader.Read(actualJson.RapidJson(), psaKey.GetInstanceId(), actualJson.Allocator()));
        ASSERT_EQ(SUCCESS, reader.Read(actualJson.JsonCpp(), psaKey.GetInstanceId()));

        actualJson.JsonCpp().removeMember(ECJsonUtilities::json_id());
        ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(newKey, actualJson.JsonCpp())) << "Insert after removing id member from read JSON ";
        validate(newKey.GetInstanceId());

        actualJson.JsonCpp().removeMember(ECJsonUtilities::json_className());
        ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(newKey, actualJson.JsonCpp())) << "Insert after removing id and className member from read JSON";
        validate(newKey.GetInstanceId());

        actualJson.RapidJson().RemoveMember(ECJsonSystemNames::Id());
        ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(newKey, actualJson.RapidJson())) << "Insert after removing id member from read JSON ";
        validate(newKey.GetInstanceId());

        actualJson.RapidJson().RemoveMember(ECJsonSystemNames::ClassName());
        ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(newKey, actualJson.RapidJson())) << "Insert after removing id and className member from read JSON";
        validate(newKey.GetInstanceId());
        }
    }
END_ECDBUNITTESTS_NAMESPACE