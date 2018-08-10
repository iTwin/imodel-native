/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/JsonLibraryTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <BeRapidJson/BeRapidJson.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// Test NaN handling 
// Expected behavior (after a Bentley change to JsonCpp) is conversion to a null value.
// @bsitest                                    Shaun.Sewall                     03/14
//---------------------------------------------------------------------------------------
TEST(JsonCpp, NaN)
    {
    Json::Value obj1(Json::objectValue);
    Json::Value obj2(Json::objectValue);

    {
    ScopedDisableFailOnAssertion disableFailOnAssertion;
    obj1["nan"] = std::numeric_limits<double>::quiet_NaN();
    }

    Utf8String str = Json::FastWriter().write(obj1);

    bool parseSuccessful = Json::Reader().parse(str.c_str(), obj2);

    ASSERT_TRUE(parseSuccessful);
    ASSERT_TRUE(obj2.isMember("nan"));
    ASSERT_TRUE(obj2["nan"].isNull());
    }

//---------------------------------------------------------------------------------------
// Make sure there is no loss of precision roundtripping double values.
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST(JsonCpp, RoundTripDoubles)
    {
    Json::Value obj1(Json::objectValue);
    Json::Value obj2(Json::objectValue);

    double d1 = PI;
    obj1["pi"] = d1;

    double n1 = -1.0 / 17.0;
    obj1["negative"] = n1;

    Utf8String str = Json::FastWriter().write(obj1);

    bool parseSuccessful = Json::Reader().parse(str.c_str(), obj2);
    ASSERT_TRUE(parseSuccessful);

    double d2 = obj2["pi"].asDouble();
    double n2 = obj2["negative"].asDouble();

#if 0
    printf("d1=%#.17g\n", d1);
    printf("n1=%#.17g\n", n1);
    printf("JsonCpp.RoundTripDoubles - %s\n", str.c_str());
    printf("d2=%#.17g\n", d2);
    printf("n2=%#.17g\n", n2);
#endif

    ASSERT_DOUBLE_EQ(d1, d2);
    ASSERT_DOUBLE_EQ(n1, n2);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST(JsonCpp, RoundTripInt64)
    {
    BeBriefcaseBasedId id(BeBriefcaseId(123), INT64_C(4129813293));

    const int64_t expectedPositiveInt64 = id.GetValue();
    const int64_t expectedNegativeInt64 = (-1) * id.GetValue();
    Json::Value obj1(Json::objectValue);
    obj1["positive"] = expectedPositiveInt64;
    obj1["negative"] = expectedNegativeInt64;

    Utf8String str = Json::FastWriter().write(obj1);

    Json::Value obj2(Json::objectValue);
    bool parseSuccessful = Json::Reader().parse(str.c_str(), obj2);
    ASSERT_TRUE(parseSuccessful);

    const int64_t actualPositiveInt64 = obj2["positive"].asInt64();
    const int64_t actualNegativeInt64 = obj2["negative"].asInt64();

    ASSERT_EQ(expectedPositiveInt64, actualPositiveInt64);
    ASSERT_EQ(expectedNegativeInt64, actualNegativeInt64);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST(JsonCpp, Int64Rendering)
    {
    BeBriefcaseBasedId id(BeBriefcaseId(123), INT64_C(4129813293));
    const int64_t expectedPositiveInt64 = id.GetValue();
    const int64_t expectedNegativeInt64 = (-1) * id.GetValue();
    Utf8String expectedPositiveInt64Str;
    expectedPositiveInt64Str.Sprintf("%" PRIi64, expectedPositiveInt64);

    Utf8String expectedNegativeInt64Str;
    expectedNegativeInt64Str.Sprintf("%" PRIi64, expectedNegativeInt64);

    Json::Value json1(expectedPositiveInt64);
    Utf8String jsonStr = Json::FastWriter().write(json1);
    jsonStr.TrimEnd();
    ASSERT_STREQ(expectedPositiveInt64Str.c_str(), jsonStr.c_str());

    Json::Value json2(expectedNegativeInt64);
    jsonStr = Json::FastWriter().write(json2);
    jsonStr.TrimEnd();
    ASSERT_STREQ(expectedNegativeInt64Str.c_str(), jsonStr.c_str());
    }

struct RapidJsonTests : public ECDbTestFixture
 {};

//---------------------------------------------------------------------------------------
// Test code grabbed (and refactored) from rapidjson/examples/tutorial/tutorial.cpp
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, FromRapidJsonTutorialCpp)
    {
    ////////////////////////////////////////////////////////////////////////////
    // 1. Parse a JSON text string to a document.

    const char json[] = " { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[0, 1, 2, 3, 4] } ";

    rapidjson::Document document;

    ASSERT_FALSE(document.Parse<0>(json).HasParseError());

    ////////////////////////////////////////////////////////////////////////////
    // 2. Access values in document. 

    ASSERT_TRUE(document.IsObject());
    ASSERT_TRUE(document.HasMember("hello"));
    ASSERT_TRUE(document["hello"].IsString());
    ASSERT_TRUE(0 == strcmp("world", document["hello"].GetString()));

    ASSERT_TRUE(document["t"].IsBool());
    ASSERT_TRUE(document["t"].IsTrue());

    ASSERT_TRUE(document["f"].IsBool());
    ASSERT_TRUE(document["f"].IsFalse());

    ASSERT_TRUE(document["n"].IsNull());

    ASSERT_TRUE(document["i"].IsNumber());
    ASSERT_TRUE(document["i"].IsInt());
    ASSERT_EQ(123, document["i"].GetInt());

    ASSERT_TRUE(document["pi"].IsNumber());
    ASSERT_TRUE(document["pi"].IsDouble());
    ASSERT_EQ(3.1416, document["pi"].GetDouble());

    
        {
        const rapidjson::Value& a = document["a"];
        ASSERT_TRUE(a.IsArray());

        for (rapidjson::SizeType i = 0; i < a.Size(); i++)
            ASSERT_EQ(i, a[i].GetInt());
        }

    ////////////////////////////////////////////////////////////////////////////
    // 3. Modify values in document.

    // Change i to a bigger number (factorial of 20)
    uint64_t f20 = 1;
    for (uint64_t j = 1; j <= 20; j++)
        f20 *= j;

    document["i"] = f20;
    ASSERT_FALSE(document["i"].IsInt());
    ASSERT_TRUE(document["i"].IsInt64());

    // Adding values to array.
    
        {
        rapidjson::Value& a = document["a"];
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

        for (int i = 5; i <= 10; i++)
            a.PushBack(i, allocator);    // May look a bit strange, allocator is needed for potentially realloc. We normally uses the document's.

        for (rapidjson::SizeType i = 0; i < a.Size(); i++)
            ASSERT_EQ(i, a[i].GetInt());

        a.PushBack("Lua", allocator).PushBack("Mio", allocator);   // Fluent API

        ASSERT_EQ(13, a.Size());
        }

    // Making string values.

    // This version of SetString() just stores the pointer to the string.
    // So it is for literal and strings that exists within value's life-cycle.
    document["hello"] = "rapidjson";

    ASSERT_TRUE(document.HasMember("hello"));
    ASSERT_TRUE(document["hello"].IsString());
    ASSERT_TRUE(0 == strcmp("rapidjson", document["hello"].GetString()));

    Utf8Char buffer[10];
    sprintf(buffer, "%s %s", "Milo", "Yip");    // synthetic example of dynamically created string.

    rapidjson::Value author(buffer, document.GetAllocator());

    memset(buffer, 0, sizeof(buffer));  // ensure copy was made

    document.AddMember("author", author, document.GetAllocator());

    ASSERT_TRUE(author.IsNull());  // Move semantic for assignment. After this variable is assigned as a member, the variable becomes null.
    ASSERT_TRUE(document.HasMember("author"));
    ASSERT_TRUE(0 == strcmp("Milo Yip", document["author"].GetString()));
    }

//---------------------------------------------------------------------------------------
// Demonstrate unfortunate add member behavior
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, AddMemberBehavior)
    {
    rapidjson::Document document;
    document.SetObject();

    ScopedDisableFailOnAssertion disableFailOnAssertion;

    // expected: nothing has been added yet
    ASSERT_FALSE(document.HasMember("x"));
    ASSERT_FALSE(document.HasMember("y"));
    ASSERT_TRUE(document["x"].IsNull());
    ASSERT_TRUE(document["y"].IsNull());

    // demonstrate unfortunate behavior
    document["x"] = 1;                          // this syntax works in JsonCpp
    document["y"] = 2;                          // this syntax works in JsonCpp

    ASSERT_FALSE(document.HasMember("x"));    // member was not really added
    ASSERT_FALSE(document.HasMember("y"));    // member was not really added
    ASSERT_TRUE(document["x"].IsNull());       // null value returned for nonexistent members
    ASSERT_TRUE(document["y"].IsNull());       // null value returned for nonexistent members

    // this is how members must be added
    rapidjson::Value x;
    x.SetInt(1);
    document.AddMember("x", x, document.GetAllocator());

    rapidjson::Value y;
    y.SetInt(2);
    document.AddMember("y", y, document.GetAllocator());

    ASSERT_TRUE(document.HasMember("x"));
    ASSERT_TRUE(document.HasMember("y"));
    ASSERT_FALSE(document["x"].IsNull());
    ASSERT_FALSE(document["y"].IsNull());
    ASSERT_EQ(1, document["x"].GetInt());
    ASSERT_EQ(2, document["y"].GetInt());

    //return value of AddMember is not the created member but the caller
    rapidjson::Value& returnValueOfAddMember = document.AddMember("testmember", rapidjson::Value(111).Move(), document.GetAllocator());
    ASSERT_TRUE(returnValueOfAddMember.IsObject());
    ASSERT_EQ(111, document["testmember"].GetInt());
    ASSERT_EQ(111, returnValueOfAddMember["testmember"].GetInt()) << "AddMember returns the calling object, not the inserted member";
    ASSERT_TRUE(&document == &returnValueOfAddMember) << "AddMember returns the calling object, not the inserted member";
    }

//---------------------------------------------------------------------------------------
// @bsitest                                    Krischan.Eberle                  01/17
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, PushBackBehavior)
    {
    rapidjson::Document json(rapidjson::kArrayType);

    const int arrayElementVal = 123;
    ScopedDisableFailOnAssertion disableFailOnAssertion;

    //return value is not the inserted array element, but the array itself!
    rapidjson::Value& returnValueOfPushBack = json.PushBack(rapidjson::Value(arrayElementVal).Move(), json.GetAllocator());

    ASSERT_TRUE(&returnValueOfPushBack == &json) << "PushBack returns the array, not the inserted array element";

    ASSERT_EQ(arrayElementVal, json[0].GetInt());
    ASSERT_EQ(arrayElementVal, returnValueOfPushBack[0].GetInt()) << "PushBack returns the array, not the inserted array element";
    }

//---------------------------------------------------------------------------------------
// Make sure there is no loss of precision roundtripping double values.
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, RoundTripDoubles)
    {
    rapidjson::Document document1;
    document1.SetObject();

    double d1 = PI;
    rapidjson::Value v;
    v.SetDouble(d1);
    document1.AddMember("pi", v, document1.GetAllocator());

    double n1 = -1.0 / 17.0;
    v.SetDouble(n1);
    document1.AddMember("negative", v, document1.GetAllocator());

    rapidjson::StringBuffer stringBuffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuffer);
    document1.Accept(writer);

    rapidjson::Document document2;
    bool parseSuccessful = !document2.Parse<0>(stringBuffer.GetString()).HasParseError();
    ASSERT_TRUE(parseSuccessful);

    double d2 = document2["pi"].GetDouble();
    double n2 = document2["negative"].GetDouble();

#if 0
    printf("d1=%#.17g\n", d1);
    printf("n1=%#.17g\n", n1);
    printf("RapidJson.RoundTripDoubles - %s\n", stringBuffer.GetString());
    printf("d2=%#.17g\n", d2);
    printf("n2=%#.17g\n", n2);
#endif

    ASSERT_DOUBLE_EQ(d1, d2);
    ASSERT_DOUBLE_EQ(n1, n2);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RapidJsonTests, Int64Rendering)
    {
    BeBriefcaseBasedId id(BeBriefcaseId(123), INT64_C(4129813293));
    const int64_t expectedPositiveInt64 = id.GetValue();
    const int64_t expectedNegativeInt64 = (-1) * id.GetValue();
    Utf8String expectedPositiveInt64Str;
    expectedPositiveInt64Str.Sprintf("%" PRIi64, expectedPositiveInt64);

    Utf8String expectedNegativeInt64Str;
    expectedNegativeInt64Str.Sprintf("%" PRIi64, expectedNegativeInt64);

    rapidjson::Value json;
    json.SetInt64(expectedPositiveInt64);

    rapidjson::StringBuffer stringBuffer1;
    rapidjson::Writer<rapidjson::StringBuffer> writer1(stringBuffer1);
    json.Accept(writer1);
    ASSERT_STREQ(expectedPositiveInt64Str.c_str(), stringBuffer1.GetString());

    json.SetInt64(expectedNegativeInt64);

    rapidjson::StringBuffer stringBuffer2;
    rapidjson::Writer<rapidjson::StringBuffer> writer2(stringBuffer2);
    json.Accept(writer2);
    ASSERT_STREQ(expectedNegativeInt64Str.c_str(), stringBuffer2.GetString());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RapidJsonTests, RoundTripInt64)
    {
    BeBriefcaseBasedId id(BeBriefcaseId(123), INT64_C(4129813293));

    rapidjson::Document document1;
    document1.SetObject();

    const int64_t expectedPositiveInt64 = id.GetValue();
    const int64_t expectedNegativeInt64 = (-1) * id.GetValue();

    rapidjson::Value v;
    v.SetInt64(expectedPositiveInt64);
    document1.AddMember("positive", v, document1.GetAllocator());

    v.SetInt64(expectedNegativeInt64);
    document1.AddMember("negative", v, document1.GetAllocator());

    rapidjson::StringBuffer stringBuffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuffer);
    document1.Accept(writer);

    rapidjson::Document document2;
    ASSERT_TRUE(!document2.Parse<0>(stringBuffer.GetString()).HasParseError());

    ASSERT_EQ(expectedPositiveInt64, document2["positive"].GetInt64());
    ASSERT_EQ(expectedNegativeInt64, document2["negative"].GetInt64());
    }

//---------------------------------------------------------------------------------------
// Variation of PerformanceECJsonInserter.Insert from PerformanceECJsonInserterTests.cpp
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, InsertIntoECDb)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("RapidJsonInsertIntoECDb.ecdb", SchemaItem::CreateForFile("JsonTests.01.00.ecschema.xml")));

    // Read JSON input from file
    BeFileName jsonInputFile;
    BeTest::GetHost().GetDocumentsRoot(jsonInputFile);
    jsonInputFile.AppendToPath(L"ECDb");
    jsonInputFile.AppendToPath(L"JsonTestClass.json");

    // Parse JSON value using JsonCpp
    Json::Value jsonInput;
    ASSERT_EQ(SUCCESS, TestUtilities::ReadFile(jsonInput, jsonInputFile));

    // Parse JSON value using RapidJson
    rapidjson::Document rapidJsonInput;
    ASSERT_EQ(SUCCESS, TestUtilities::ParseJson(rapidJsonInput, Json::FastWriter().write(jsonInput)));

    ECClassCP documentClass = m_ecdb.Schemas().GetClass("JsonTests", "Document");
    ASSERT_TRUE(documentClass != nullptr);
    JsonInserter inserter(m_ecdb, *documentClass, nullptr);
    ASSERT_TRUE(inserter.IsValid());

    // insert 1 row using JsonCpp
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(jsonInput));
    m_ecdb.SaveChanges();

    // insert 1 row using RapidJson
    ECInstanceKey ecInstanceKey;
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(ecInstanceKey, rapidJsonInput));
    ASSERT_TRUE(ecInstanceKey.IsValid());
    m_ecdb.SaveChanges();

    // now update the row that was just inserted
    JsonUpdater updater(m_ecdb, *documentClass, nullptr);
    ASSERT_TRUE(updater.IsValid());
    rapidJsonInput.RemoveMember(ECJsonSystemNames::Id());
    rapidJsonInput.RemoveMember(ECJsonSystemNames::ClassName());
    rapidJsonInput.AddMember("urx", 3.3, rapidJsonInput.GetAllocator());
    rapidJsonInput.AddMember("ury", 4.4, rapidJsonInput.GetAllocator());

    ASSERT_EQ(BE_SQLITE_OK, updater.Update(ecInstanceKey.GetInstanceId(), rapidJsonInput));
    m_ecdb.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// From libsrc/rapidjson/test/unittest/valuetest.cpp
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, ValueDefaultConstructor)
    {
    rapidjson::Value x;
    ASSERT_EQ(rapidjson::kNullType, x.GetType());
    ASSERT_TRUE(x.IsNull());
    }

//---------------------------------------------------------------------------------------
// From libsrc/rapidjson/test/unittest/valuetest.cpp
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, ValueAssignmentOperator)
    {
    rapidjson::Value x(1234);
    rapidjson::Value y;
    y = x;
    ASSERT_TRUE(x.IsNull()); // move semantic
    ASSERT_EQ(1234, y.GetInt());
    }

//---------------------------------------------------------------------------------------
// From libsrc/rapidjson/test/unittest/valuetest.cpp
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, ValueNull)
    {
    // default constructor
    rapidjson::Value x;
    ASSERT_EQ(rapidjson::kNullType, x.GetType());
    ASSERT_TRUE(x.IsNull());

    ASSERT_FALSE(x.IsTrue());
    ASSERT_FALSE(x.IsFalse());
    ASSERT_FALSE(x.IsNumber());
    ASSERT_FALSE(x.IsString());
    ASSERT_FALSE(x.IsObject());
    ASSERT_FALSE(x.IsArray());

    // Constructor with type
    rapidjson::Value y(rapidjson::kNullType);
    ASSERT_TRUE(y.IsNull());

    // SetNull();
    rapidjson::Value z(true);
    z.SetNull();
    ASSERT_TRUE(z.IsNull());
    ASSERT_FALSE(z.IsBool());

    rapidjson::Document array(rapidjson::kArrayType);
    array.PushBack(rapidjson::Value(1).Move(), array.GetAllocator()).PushBack(rapidjson::Value(2).Move(), array.GetAllocator());
    ASSERT_EQ(2, (int) array.Size());
    array.SetNull();
    ASSERT_FALSE(array.IsArray());

    rapidjson::Document obj(rapidjson::kObjectType);
    obj.AddMember("m1", rapidjson::Value(1).Move(), array.GetAllocator()).AddMember("m2", rapidjson::Value(2).Move(), array.GetAllocator());
    ASSERT_EQ(2, (int) obj.MemberCount());
    obj.SetNull();
    ASSERT_FALSE(obj.IsObject());
    }

//---------------------------------------------------------------------------------------
// From libsrc/rapidjson/test/unittest/valuetest.cpp
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, ValueTrue)
    {
    // Constructor with bool
    rapidjson::Value x(true);
    ASSERT_EQ(rapidjson::kTrueType, x.GetType());
    ASSERT_TRUE(x.GetBool());
    ASSERT_TRUE(x.IsBool());
    ASSERT_TRUE(x.IsTrue());

    ASSERT_FALSE(x.IsNull());
    ASSERT_FALSE(x.IsFalse());
    ASSERT_FALSE(x.IsNumber());
    ASSERT_FALSE(x.IsString());
    ASSERT_FALSE(x.IsObject());
    ASSERT_FALSE(x.IsArray());

    // Constructor with type
    rapidjson::Value y(rapidjson::kTrueType);
    ASSERT_TRUE(y.IsTrue());

    // SetBool()
    rapidjson::Value z;
    z.SetBool(true);
    ASSERT_TRUE(z.IsTrue());
    }

//---------------------------------------------------------------------------------------
// From libsrc/rapidjson/test/unittest/valuetest.cpp
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, ValueFalse)
    {
    // Constructor with bool
    rapidjson::Value x(false);
    ASSERT_EQ(rapidjson::kFalseType, x.GetType());
    ASSERT_TRUE(x.IsBool());
    ASSERT_TRUE(x.IsFalse());

    ASSERT_FALSE(x.IsNull());
    ASSERT_FALSE(x.IsTrue());
    ASSERT_FALSE(x.GetBool());
    ASSERT_FALSE(x.IsNumber());
    ASSERT_FALSE(x.IsString());
    ASSERT_FALSE(x.IsObject());
    ASSERT_FALSE(x.IsArray());

    // Constructor with type
    rapidjson::Value y(rapidjson::kFalseType);
    ASSERT_TRUE(y.IsFalse());

    // SetBool()
    rapidjson::Value z;
    z.SetBool(false);
    ASSERT_TRUE(z.IsFalse());
    }

//---------------------------------------------------------------------------------------
// From libsrc/rapidjson/test/unittest/valuetest.cpp
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, ValueInt)
    {
    // Constructor with int
    rapidjson::Value x(1234);
    ASSERT_EQ(rapidjson::kNumberType, x.GetType());
    ASSERT_EQ(1234, x.GetInt());
    ASSERT_EQ(1234u, x.GetUint());
    ASSERT_EQ(1234, x.GetInt64());
    ASSERT_EQ(1234u, x.GetUint64());
    ASSERT_EQ(1234, x.GetDouble());
    ASSERT_TRUE(x.IsNumber());
    ASSERT_TRUE(x.IsInt());
    ASSERT_TRUE(x.IsUint());
    ASSERT_TRUE(x.IsInt64());
    ASSERT_TRUE(x.IsUint64());

    ASSERT_FALSE(x.IsDouble());
    ASSERT_FALSE(x.IsNull());
    ASSERT_FALSE(x.IsBool());
    ASSERT_FALSE(x.IsFalse());
    ASSERT_FALSE(x.IsTrue());
    ASSERT_FALSE(x.IsString());
    ASSERT_FALSE(x.IsObject());
    ASSERT_FALSE(x.IsArray());

    rapidjson::Value nx(-1234);
    ASSERT_EQ(-1234, nx.GetInt());
    ASSERT_EQ(-1234, nx.GetInt64());
    ASSERT_TRUE(nx.IsInt());
    ASSERT_TRUE(nx.IsInt64());
    ASSERT_FALSE(nx.IsUint());
    ASSERT_FALSE(nx.IsUint64());

    // Constructor with type
    rapidjson::Value y(rapidjson::kNumberType);
    ASSERT_TRUE(y.IsNumber());
    ASSERT_TRUE(y.IsInt());
    ASSERT_EQ(0, y.GetInt());

    // SetInt()
    rapidjson::Value z;
    z.SetInt(1234);
    ASSERT_EQ(1234, z.GetInt());

    // operator=(int)
    z = 5678;
    ASSERT_EQ(5678, z.GetInt());
    }

//---------------------------------------------------------------------------------------
// From libsrc/rapidjson/test/unittest/valuetest.cpp
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, ValueUint)
    {
    // Constructor with int
    rapidjson::Value x(1234u);
    ASSERT_EQ(rapidjson::kNumberType, x.GetType());
    ASSERT_EQ(1234, x.GetInt());
    ASSERT_EQ(1234u, x.GetUint());
    ASSERT_EQ(1234, x.GetInt64());
    ASSERT_EQ(1234u, x.GetUint64());
    ASSERT_TRUE(x.IsNumber());
    ASSERT_TRUE(x.IsInt());
    ASSERT_TRUE(x.IsUint());
    ASSERT_TRUE(x.IsInt64());
    ASSERT_TRUE(x.IsUint64());
    ASSERT_EQ(1234.0, x.GetDouble()); // Number can always be cast as double but !IsDouble().

    ASSERT_FALSE(x.IsDouble());
    ASSERT_FALSE(x.IsNull());
    ASSERT_FALSE(x.IsBool());
    ASSERT_FALSE(x.IsFalse());
    ASSERT_FALSE(x.IsTrue());
    ASSERT_FALSE(x.IsString());
    ASSERT_FALSE(x.IsObject());
    ASSERT_FALSE(x.IsArray());

    // SetUint()
    rapidjson::Value z;
    z.SetUint(1234);
    ASSERT_EQ(1234u, z.GetUint());

    // operator=(unsigned)
    z = 5678u;
    ASSERT_EQ(5678u, z.GetUint());

    z = 2147483648u; // 2^31, cannot cast as int
    ASSERT_EQ(2147483648u, z.GetUint());
    ASSERT_FALSE(z.IsInt());
    ASSERT_TRUE(z.IsInt64());
    }

//---------------------------------------------------------------------------------------
// From libsrc/rapidjson/test/unittest/valuetest.cpp
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, ValueInt64)
    {
    // Constructor with int
    rapidjson::Value x(INT64_C(1234));
    ASSERT_EQ(rapidjson::kNumberType, x.GetType());
    ASSERT_EQ(1234, x.GetInt());
    ASSERT_EQ(1234u, x.GetUint());
    ASSERT_EQ(1234, x.GetInt64());
    ASSERT_EQ(1234u, x.GetUint64());
    ASSERT_TRUE(x.IsNumber());
    ASSERT_TRUE(x.IsInt());
    ASSERT_TRUE(x.IsUint());
    ASSERT_TRUE(x.IsInt64());
    ASSERT_TRUE(x.IsUint64());

    ASSERT_FALSE(x.IsDouble());
    ASSERT_FALSE(x.IsNull());
    ASSERT_FALSE(x.IsBool());
    ASSERT_FALSE(x.IsFalse());
    ASSERT_FALSE(x.IsTrue());
    ASSERT_FALSE(x.IsString());
    ASSERT_FALSE(x.IsObject());
    ASSERT_FALSE(x.IsArray());

    rapidjson::Value nx(INT64_C(-1234));
    ASSERT_EQ(-1234, nx.GetInt());
    ASSERT_EQ(-1234, nx.GetInt64());
    ASSERT_TRUE(nx.IsInt());
    ASSERT_TRUE(nx.IsInt64());
    ASSERT_FALSE(nx.IsUint());
    ASSERT_FALSE(nx.IsUint64());

    // SetInt64()
    rapidjson::Value z;
    z.SetInt64(1234);
    ASSERT_EQ(1234, z.GetInt64());

    z.SetInt64(2147483648LL); // 2^31, cannot cast as int
    ASSERT_FALSE(z.IsInt());
    ASSERT_TRUE(z.IsUint());

    z.SetInt64(4294967296LL); // 2^32, cannot cast as uint
    ASSERT_FALSE(z.IsInt());
    ASSERT_FALSE(z.IsUint());
    }

//---------------------------------------------------------------------------------------
// From libsrc/rapidjson/test/unittest/valuetest.cpp
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, ValueUint64)
    {
    // Constructor with int
    rapidjson::Value x(INT64_C(1234));
    ASSERT_EQ(rapidjson::kNumberType, x.GetType());
    ASSERT_EQ(1234, x.GetInt());
    ASSERT_EQ(1234u, x.GetUint());
    ASSERT_EQ(1234, x.GetInt64());
    ASSERT_EQ(1234u, x.GetUint64());
    ASSERT_TRUE(x.IsNumber());
    ASSERT_TRUE(x.IsInt());
    ASSERT_TRUE(x.IsUint());
    ASSERT_TRUE(x.IsInt64());
    ASSERT_TRUE(x.IsUint64());

    ASSERT_FALSE(x.IsDouble());
    ASSERT_FALSE(x.IsNull());
    ASSERT_FALSE(x.IsBool());
    ASSERT_FALSE(x.IsFalse());
    ASSERT_FALSE(x.IsTrue());
    ASSERT_FALSE(x.IsString());
    ASSERT_FALSE(x.IsObject());
    ASSERT_FALSE(x.IsArray());

    // SetUint64()
    rapidjson::Value z;
    z.SetUint64(1234);
    ASSERT_EQ(1234u, z.GetUint64());

    z.SetUint64(2147483648LL); // 2^31, cannot cast as int
    ASSERT_FALSE(z.IsInt());
    ASSERT_TRUE(z.IsUint());
    ASSERT_TRUE(z.IsInt64());

    z.SetUint64(4294967296LL); // 2^32, cannot cast as uint
    ASSERT_FALSE(z.IsInt());
    ASSERT_FALSE(z.IsUint());
    ASSERT_TRUE(z.IsInt64());

    z.SetUint64(9223372036854775808uLL); // 2^63 cannot cast as int64
    ASSERT_FALSE(z.IsInt64());
    ASSERT_EQ(9223372036854775808uLL, z.GetUint64());
    }

//---------------------------------------------------------------------------------------
// From libsrc/rapidjson/test/unittest/valuetest.cpp
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, ValueDouble)
    {
    // Constructor with double
    rapidjson::Value x(12.34);
    ASSERT_EQ(rapidjson::kNumberType, x.GetType());
    ASSERT_EQ(12.34, x.GetDouble());
    ASSERT_TRUE(x.IsNumber());
    ASSERT_TRUE(x.IsDouble());

    ASSERT_FALSE(x.IsInt());
    ASSERT_FALSE(x.IsNull());
    ASSERT_FALSE(x.IsBool());
    ASSERT_FALSE(x.IsFalse());
    ASSERT_FALSE(x.IsTrue());
    ASSERT_FALSE(x.IsString());
    ASSERT_FALSE(x.IsObject());
    ASSERT_FALSE(x.IsArray());

    // SetDouble()
    rapidjson::Value z;
    z.SetDouble(12.34);
    ASSERT_EQ(12.34, z.GetDouble());

    z = 56.78;
    ASSERT_EQ(56.78, z.GetDouble());
    }

//---------------------------------------------------------------------------------------
// From libsrc/rapidjson/test/unittest/valuetest.cpp
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, ValueString)
    {
    // Constructor with const string
    rapidjson::Value x("Hello", 5);
    ASSERT_EQ(rapidjson::kStringType, x.GetType());
    ASSERT_TRUE(x.IsString());
    ASSERT_STREQ("Hello", x.GetString());
    ASSERT_EQ(5u, x.GetStringLength());

    ASSERT_FALSE(x.IsNumber());
    ASSERT_FALSE(x.IsNull());
    ASSERT_FALSE(x.IsBool());
    ASSERT_FALSE(x.IsFalse());
    ASSERT_FALSE(x.IsTrue());
    ASSERT_FALSE(x.IsObject());
    ASSERT_FALSE(x.IsArray());

    // Constructor with copy string
    rapidjson::MemoryPoolAllocator<> allocator;
    rapidjson::Value c(x.GetString(), x.GetStringLength(), allocator);
    x.SetString("World", 5);
    ASSERT_STREQ("Hello", c.GetString());
    ASSERT_EQ(5u, c.GetStringLength());

    // Constructor with type
    rapidjson::Value y(rapidjson::kStringType);
    ASSERT_TRUE(y.IsString());
    ASSERT_STREQ("", y.GetString());
    ASSERT_EQ(0u, y.GetStringLength());

    // SetConsttring()
    rapidjson::Value z;
    z.SetString("Hello", 5);
    ASSERT_STREQ("Hello", z.GetString());
    ASSERT_EQ(5u, z.GetStringLength());

    // SetString()
    char s[] = "World";
    rapidjson::Value w;
    w.SetString(s, (rapidjson::SizeType) strlen(s), allocator);
    s[0] = '\0';
    ASSERT_STREQ("World", w.GetString());
    ASSERT_EQ(5u, w.GetStringLength());
    }

//---------------------------------------------------------------------------------------
// From libsrc/rapidjson/test/unittest/valuetest.cpp
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, ValueArray)
    {
    rapidjson::Value x(rapidjson::kArrayType);
    const rapidjson::Value& y = x;
    rapidjson::Value::AllocatorType allocator;

    ASSERT_EQ(rapidjson::kArrayType, x.GetType());
    ASSERT_TRUE(x.IsArray());
    ASSERT_TRUE(x.Empty());
    ASSERT_EQ(0u, x.Size());
    ASSERT_TRUE(y.IsArray());
    ASSERT_TRUE(y.Empty());
    ASSERT_EQ(0u, y.Size());

    ASSERT_FALSE(x.IsNull());
    ASSERT_FALSE(x.IsBool());
    ASSERT_FALSE(x.IsFalse());
    ASSERT_FALSE(x.IsTrue());
    ASSERT_FALSE(x.IsString());
    ASSERT_FALSE(x.IsObject());

    // PushBack()
    rapidjson::Value v;
    x.PushBack(v, allocator);
    v.SetBool(true);
    x.PushBack(v, allocator);
    v.SetBool(false);
    x.PushBack(v, allocator);
    v.SetInt(123);
    x.PushBack(v, allocator);

    ASSERT_FALSE(x.Empty());
    ASSERT_EQ(4u, x.Size());
    ASSERT_FALSE(y.Empty());
    ASSERT_EQ(4u, y.Size());
    ASSERT_TRUE(x[rapidjson::SizeType(0)].IsNull());
    ASSERT_TRUE(x[1u].IsTrue());
    ASSERT_TRUE(x[2u].IsFalse());
    ASSERT_TRUE(x[3u].IsInt());
    ASSERT_EQ(123, x[3u].GetInt());
    ASSERT_TRUE(y[rapidjson::SizeType(0)].IsNull());
    ASSERT_TRUE(y[1u].IsTrue());
    ASSERT_TRUE(y[2u].IsFalse());
    ASSERT_TRUE(y[3u].IsInt());
    ASSERT_EQ(123, y[3u].GetInt());

    // iterator
    rapidjson::Value::ValueIterator itr = x.Begin();
    ASSERT_TRUE(itr != x.End());
    ASSERT_TRUE(itr->IsNull());
    ++itr;
    ASSERT_TRUE(itr != x.End());
    ASSERT_TRUE(itr->IsTrue());
    ++itr;
    ASSERT_TRUE(itr != x.End());
    ASSERT_TRUE(itr->IsFalse());
    ++itr;
    ASSERT_TRUE(itr != x.End());
    ASSERT_TRUE(itr->IsInt());
    ASSERT_EQ(123, itr->GetInt());

    // const iterator
    rapidjson::Value::ConstValueIterator citr = y.Begin();
    ASSERT_TRUE(citr != y.End());
    ASSERT_TRUE(citr->IsNull());
    ++citr;
    ASSERT_TRUE(citr != y.End());
    ASSERT_TRUE(citr->IsTrue());
    ++citr;
    ASSERT_TRUE(citr != y.End());
    ASSERT_TRUE(citr->IsFalse());
    ++citr;
    ASSERT_TRUE(citr != y.End());
    ASSERT_TRUE(citr->IsInt());
    ASSERT_EQ(123, citr->GetInt());

    // PopBack()
    x.PopBack();
    ASSERT_EQ(3u, x.Size());
    ASSERT_TRUE(y[rapidjson::SizeType(0)].IsNull());
    ASSERT_TRUE(y[1].IsTrue());
    ASSERT_TRUE(y[2].IsFalse());

    // Clear()
    x.Clear();
    ASSERT_TRUE(x.Empty());
    ASSERT_EQ(0u, x.Size());
    ASSERT_TRUE(y.Empty());
    ASSERT_EQ(0u, y.Size());

    // SetArray()
    rapidjson::Value z;
    z.SetArray();
    ASSERT_TRUE(z.IsArray());
    ASSERT_TRUE(z.Empty());
    }

//---------------------------------------------------------------------------------------
// From libsrc/rapidjson/test/unittest/valuetest.cpp
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, ValueObject)
    {
    rapidjson::Value x(rapidjson::kObjectType);
    RapidJsonValueCR y = x; // const version
    rapidjson::Value::AllocatorType allocator;

    ASSERT_EQ(rapidjson::kObjectType, x.GetType());
    ASSERT_TRUE(x.IsObject());
    ASSERT_EQ(rapidjson::kObjectType, y.GetType());
    ASSERT_TRUE(y.IsObject());

    // AddMember()
    rapidjson::Value name("A", 1);
    rapidjson::Value value("Apple", 5);
    x.AddMember(name, value, allocator);
    name.SetString("B", 1);
    value.SetString("Banana", 6);
    x.AddMember(name, value, allocator);

    // HasMember()
    ASSERT_TRUE(x.HasMember("A"));
    ASSERT_TRUE(x.HasMember("B"));
    ASSERT_TRUE(y.HasMember("A"));
    ASSERT_TRUE(y.HasMember("B"));

    // operator[]
    ASSERT_STREQ("Apple", x["A"].GetString());
    ASSERT_STREQ("Banana", x["B"].GetString());

    // const operator[]
    ASSERT_STREQ("Apple", y["A"].GetString());
    ASSERT_STREQ("Banana", y["B"].GetString());

    // member iterator
    rapidjson::Value::MemberIterator itr = x.MemberBegin();
    ASSERT_TRUE(itr != x.MemberEnd());
    ASSERT_STREQ("A", itr->name.GetString());
    ASSERT_STREQ("Apple", itr->value.GetString());
    ++itr;
    ASSERT_TRUE(itr != x.MemberEnd());
    ASSERT_STREQ("B", itr->name.GetString());
    ASSERT_STREQ("Banana", itr->value.GetString());
    ++itr;
    ASSERT_FALSE(itr != x.MemberEnd());

    // const member iterator
    rapidjson::Value::ConstMemberIterator citr = y.MemberBegin();
    ASSERT_TRUE(citr != y.MemberEnd());
    ASSERT_STREQ("A", citr->name.GetString());
    ASSERT_STREQ("Apple", citr->value.GetString());
    ++citr;
    ASSERT_TRUE(citr != y.MemberEnd());
    ASSERT_STREQ("B", citr->name.GetString());
    ASSERT_STREQ("Banana", citr->value.GetString());
    ++citr;
    ASSERT_FALSE(citr != y.MemberEnd());

    // RemoveMember()
    x.RemoveMember("A");
    ASSERT_FALSE(x.HasMember("A"));

    x.RemoveMember("B");
    ASSERT_FALSE(x.HasMember("B"));

    ASSERT_TRUE(x.MemberBegin() == x.MemberEnd());

    // SetObject()
    rapidjson::Value z;
    z.SetObject();
    ASSERT_TRUE(z.IsObject());
    }

//---------------------------------------------------------------------------------------
// From libsrc/rapidjson/test/unittest/valuetest.cpp
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, ValueBigNestedArray)
    {
    rapidjson::MemoryPoolAllocator<> allocator;
    rapidjson::Value x(rapidjson::kArrayType);
    static const rapidjson::SizeType n = 200;

    for (rapidjson::SizeType i = 0; i < n; i++)
        {
        rapidjson::Value y(rapidjson::kArrayType);
        for (rapidjson::SizeType j = 0; j < n; j++)
            {
            rapidjson::Value number((int) (i * n + j));
            y.PushBack(number, allocator);
            }

        x.PushBack(y, allocator);
        }

    for (rapidjson::SizeType i = 0; i < n; i++)
        {
        for (rapidjson::SizeType j = 0; j < n; j++)
            {
            ASSERT_TRUE(x[i][j].IsInt());
            ASSERT_EQ((int) (i * n + j), x[i][j].GetInt());
            }
        }
    }

//---------------------------------------------------------------------------------------
// From libsrc/rapidjson/test/unittest/valuetest.cpp
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, ValueBigNestedObject)
    {
    rapidjson::MemoryPoolAllocator<> allocator;
    rapidjson::Value x(rapidjson::kObjectType);
    static const rapidjson::SizeType n = 200;

    for (rapidjson::SizeType i = 0; i < n; i++)
        {
        char name1[10];
        sprintf(name1, "%d", i);

        rapidjson::Value name(name1, (rapidjson::SizeType) strlen(name1), allocator);
        rapidjson::Value object(rapidjson::kObjectType);

        for (rapidjson::SizeType j = 0; j < n; j++)
            {
            char name2[10];
            sprintf(name2, "%d", j);

            rapidjson::Value name(name2, (rapidjson::SizeType) strlen(name2), allocator);
            rapidjson::Value number((int) (i * n + j));
            object.AddMember(name, number, allocator);
            }

        x.AddMember(name, object, allocator);
        }

    for (rapidjson::SizeType i = 0; i < n; i++)
        {
        char name1[10];
        sprintf(name1, "%d", i);

        for (rapidjson::SizeType j = 0; j < n; j++)
            {
            char name2[10];
            sprintf(name2, "%d", j);
            x[name1];
            ASSERT_EQ((int) (i * n + j), x[name1][name2].GetInt());
            }
        }
    }

//---------------------------------------------------------------------------------------
// From libsrc/rapidjson/test/unittest/valuetest.cpp
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, ValueRemoveLastElement)
    {
    rapidjson::Document doc;
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
    rapidjson::Value objVal(rapidjson::kObjectType);

    objVal.AddMember("var1", 123, allocator);
    objVal.AddMember("var2", "444", allocator);
    objVal.AddMember("var3", 555, allocator);
    ASSERT_TRUE(objVal.HasMember("var3"));
    objVal.RemoveMember("var3");
    ASSERT_FALSE(objVal.HasMember("var3"));
    }

//---------------------------------------------------------------------------------------
// From libsrc/rapidjson/test/unittest/valuetest.cpp
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, DocumentCrtAllocator)
    {
    typedef rapidjson::GenericValue<rapidjson::UTF8<>, rapidjson::CrtAllocator> V;

    V::AllocatorType allocator;
    V o(rapidjson::kObjectType);
    o.AddMember("x", 1, allocator); // Should not call destructor on uninitialized name/value of newly allocated members.

    V a(rapidjson::kArrayType);
    a.PushBack(1, allocator); // Should not call destructor on uninitialized Value of newly allocated elements.
    }

//---------------------------------------------------------------------------------------
// From libsrc/rapidjson/test/unittest/documenttest.cpp
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, DocumentParse)
    {
    rapidjson::Document doc;

    doc.Parse<0>(" { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4] } ");

    ASSERT_TRUE(doc.IsObject());

    ASSERT_TRUE(doc.HasMember("hello"));
    RapidJsonValueCR hello = doc["hello"];
    ASSERT_TRUE(hello.IsString());
    ASSERT_STREQ("world", hello.GetString());

    ASSERT_TRUE(doc.HasMember("t"));
    RapidJsonValueCR t = doc["t"];
    ASSERT_TRUE(t.IsTrue());

    ASSERT_TRUE(doc.HasMember("f"));
    RapidJsonValueCR f = doc["f"];
    ASSERT_TRUE(f.IsFalse());

    ASSERT_TRUE(doc.HasMember("n"));
    RapidJsonValueCR n = doc["n"];
    ASSERT_TRUE(n.IsNull());

    ASSERT_TRUE(doc.HasMember("i"));
    RapidJsonValueCR i = doc["i"];
    ASSERT_TRUE(i.IsNumber());
    ASSERT_EQ(123, i.GetInt());

    ASSERT_TRUE(doc.HasMember("pi"));
    RapidJsonValueCR pi = doc["pi"];
    ASSERT_TRUE(pi.IsNumber());
    ASSERT_EQ(3.1416, pi.GetDouble());

    ASSERT_TRUE(doc.HasMember("a"));
    RapidJsonValueCR a = doc["a"];
    ASSERT_TRUE(a.IsArray());
    ASSERT_EQ(4u, a.Size());

    for (rapidjson::SizeType i = 0; i < 4; i++)
        ASSERT_EQ(i + 1, a[i].GetUint());
    }

//---------------------------------------------------------------------------------------
// From libsrc/rapidjson/test/unittest/documenttest.cpp
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, DocumentAcceptWriter)
    {
    rapidjson::Document doc;
    doc.Parse<0>(" { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"a\":[1, 2, 3, 4] } ");

    rapidjson::StringBuffer stringBuffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuffer);

    doc.Accept(writer);
    ASSERT_STREQ("{\"hello\":\"world\",\"t\":true,\"f\":false,\"n\":null,\"i\":123,\"a\":[1,2,3,4]}", stringBuffer.GetString());
    }

//---------------------------------------------------------------------------------------
// From libsrc/rapidjson/test/unittest/writertest.cpp
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, WriterCompact)
    {
    rapidjson::StringStream s("{ \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"a\":[1, 2, 3] } ");
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    rapidjson::Reader reader;

    reader.Parse<0>(s, writer);
    ASSERT_STREQ("{\"hello\":\"world\",\"t\":true,\"f\":false,\"n\":null,\"i\":123,\"a\":[1,2,3]}", buffer.GetString());
    ASSERT_EQ(65u, buffer.GetSize());
    }

//---------------------------------------------------------------------------------------
// From libsrc/rapidjson/test/unittest/writertest.cpp
// json -> parse -> writer -> json
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
#define RAPIDJSON_TEST_ROUNDTRIP(json) \
    { \
    rapidjson::StringStream s (json); \
    rapidjson::StringBuffer buffer; \
    rapidjson::Writer<rapidjson::StringBuffer> writer (buffer); \
    rapidjson::Reader reader; \
    reader.Parse<0>(s, writer); \
    ASSERT_STREQ (json, buffer.GetString()); \
    }

//---------------------------------------------------------------------------------------
// From libsrc/rapidjson/test/unittest/writertest.cpp
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, WriterInt)
    {
    RAPIDJSON_TEST_ROUNDTRIP("[-1]");
    RAPIDJSON_TEST_ROUNDTRIP("[-123]");
    RAPIDJSON_TEST_ROUNDTRIP("[-2147483648]");
    }

//---------------------------------------------------------------------------------------
// From libsrc/rapidjson/test/unittest/writertest.cpp
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, WriterUInt)
    {
    RAPIDJSON_TEST_ROUNDTRIP("[0]");
    RAPIDJSON_TEST_ROUNDTRIP("[1]");
    RAPIDJSON_TEST_ROUNDTRIP("[123]");
    RAPIDJSON_TEST_ROUNDTRIP("[2147483647]");
    RAPIDJSON_TEST_ROUNDTRIP("[4294967295]");
    }

//---------------------------------------------------------------------------------------
// From libsrc/rapidjson/test/unittest/writertest.cpp
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, WriterInt64)
    {
    RAPIDJSON_TEST_ROUNDTRIP("[-1234567890123456789]");
    RAPIDJSON_TEST_ROUNDTRIP("[-9223372036854775808]");
    }

//---------------------------------------------------------------------------------------
// From libsrc/rapidjson/test/unittest/writertest.cpp
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, WriterUint64)
    {
    RAPIDJSON_TEST_ROUNDTRIP("[1234567890123456789]");
    RAPIDJSON_TEST_ROUNDTRIP("[9223372036854775807]");
    }

//---------------------------------------------------------------------------------------
// From libsrc/rapidjson/test/unittest/writertest.cpp
// @bsitest                                    Shaun.Sewall                     01/14
//---------------------------------------------------------------------------------------
TEST_F(RapidJsonTests, WriterString)
    {
    RAPIDJSON_TEST_ROUNDTRIP("[\"Hello\"]");
    RAPIDJSON_TEST_ROUNDTRIP("[\"Hello\\u0000World\"]");
    RAPIDJSON_TEST_ROUNDTRIP("[\"\\\"\\\\/\\b\\f\\n\\r\\t\"]");
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/17
//+---------------+---------------+---------------+---------------+---------------+------
struct SqliteJsonTests : public ECDbTestFixture
    {};


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SqliteJsonTests, Int64Rendering)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("sqlitejsontests.ecdb"));

    BeBriefcaseBasedId id(BeBriefcaseId(123), INT64_C(4129813293));
    const int64_t expectedPositiveInt64 = id.GetValue();
    const int64_t expectedNegativeInt64 = (-1) * id.GetValue();
    Utf8String expectedPositiveInt64Str;
    expectedPositiveInt64Str.Sprintf("%" PRIi64, expectedPositiveInt64);

    Utf8String expectedNegativeInt64Str;
    expectedNegativeInt64Str.Sprintf("%" PRIi64, expectedNegativeInt64);

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT json_object('positive',?), json_object('negative',?)"));
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindInt64(1, expectedPositiveInt64));
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindInt64(2, expectedNegativeInt64));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    Utf8String positiveInt64JsonStr(stmt.GetValueText(0));
    Utf8String negativeInt64JsonStr(stmt.GetValueText(1));
    stmt.Finalize();

    size_t colon = positiveInt64JsonStr.find_first_of(':');
    ASSERT_STREQ(expectedPositiveInt64Str.c_str(), Utf8String(positiveInt64JsonStr.substr(colon + 1, expectedPositiveInt64Str.size())).Trim().c_str());

    colon = negativeInt64JsonStr.find_first_of(':');
    ASSERT_STREQ(expectedNegativeInt64Str.c_str(), Utf8String(negativeInt64JsonStr.substr(colon + 1, expectedNegativeInt64Str.size())).Trim().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SqliteJsonTests, RoundTripDoublesAndInt64)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("sqlitejsontests.ecdb"));

    const double expectedPositiveDouble = PI;
    const double expectedNegativeDouble = -1.0 / 17.0;
    BeBriefcaseBasedId id(BeBriefcaseId(123), INT64_C(4129813293));
    const int64_t expectedPositiveInt64 = id.GetValue();
    const int64_t expectedNegativeInt64 = (-1) * id.GetValue();


    //double round-trip
    Statement toStmt;
    ASSERT_EQ(BE_SQLITE_OK, toStmt.Prepare(m_ecdb, "SELECT json_object('positive',?,'negative',?)"));
    ASSERT_EQ(BE_SQLITE_OK, toStmt.BindDouble(1, expectedPositiveDouble));
    ASSERT_EQ(BE_SQLITE_OK, toStmt.BindDouble(2, expectedNegativeDouble));
    ASSERT_EQ(BE_SQLITE_ROW, toStmt.Step());
    Utf8String jsonStr(toStmt.GetValueText(0));
    toStmt.Reset();
    toStmt.ClearBindings();

    Statement fromStmt;
    ASSERT_EQ(BE_SQLITE_OK, fromStmt.Prepare(m_ecdb, "SELECT json_extract(?1,'$.positive'), json_extract(?1,'$.negative')"));
    ASSERT_EQ(BE_SQLITE_OK, fromStmt.BindText(1, jsonStr.c_str(), Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ROW, fromStmt.Step());
    ASSERT_NEAR(expectedPositiveDouble, fromStmt.GetValueDouble(0), 0.00000000000001);
    ASSERT_NEAR(expectedNegativeDouble, fromStmt.GetValueDouble(1), 0.00000000000001);
    fromStmt.Reset();
    fromStmt.ClearBindings();

    //int64 round-trip
    ASSERT_EQ(BE_SQLITE_OK, toStmt.BindInt64(1, expectedPositiveInt64));
    ASSERT_EQ(BE_SQLITE_OK, toStmt.BindInt64(2, expectedNegativeInt64));
    ASSERT_EQ(BE_SQLITE_ROW, toStmt.Step());
    jsonStr.assign(toStmt.GetValueText(0));
    toStmt.Reset();
    toStmt.ClearBindings();

    ASSERT_EQ(BE_SQLITE_OK, fromStmt.BindText(1, jsonStr.c_str(), Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ROW, fromStmt.Step());
    ASSERT_EQ(expectedPositiveInt64, fromStmt.GetValueInt64(0));
    ASSERT_EQ(expectedNegativeInt64, fromStmt.GetValueInt64(1));
    fromStmt.Reset();
    fromStmt.ClearBindings();
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  04/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST(JsonValueHelper, Comparisons)
    {
    JsonValue uintVal(Json::Value(UINT32_C(1)));
    EXPECT_TRUE(uintVal.m_value.isUInt());
    EXPECT_FALSE(uintVal.m_value.isInt());
    EXPECT_TRUE(uintVal.m_value.isIntegral());

    JsonValue intVal(Json::Value((int) 1));
    EXPECT_TRUE(intVal.m_value.isInt());
    EXPECT_FALSE(intVal.m_value.isUInt());
    EXPECT_TRUE(intVal.m_value.isIntegral());

    EXPECT_FALSE(uintVal.m_value == intVal.m_value) << "JsonCpp comparison is strict on exact integral type";
    EXPECT_TRUE(uintVal == intVal) << "JsonValue helper API does integral type coercion";

    JsonValue int64Val(Json::Value((int64_t) 1));
    EXPECT_TRUE(int64Val.m_value.isInt());
    EXPECT_TRUE(int64Val.m_value.isIntegral());

    EXPECT_TRUE(int64Val.m_value == intVal.m_value) << "JsonCpp comparison allows comparing int32 with int64";
    EXPECT_TRUE(int64Val == intVal) << "JsonValue helper API does integral type coercion";
    EXPECT_FALSE(int64Val.m_value == uintVal.m_value) << "JsonCpp comparison is strict on exact integral type";
    EXPECT_TRUE(int64Val == uintVal) << "JsonValue helper API does integral type coercion";

    JsonValue uint64Val(Json::Value((uint64_t) 1));
    EXPECT_TRUE(uint64Val.m_value.isUInt());
    EXPECT_TRUE(uint64Val.m_value.isIntegral());

    EXPECT_FALSE(int64Val.m_value == uint64Val.m_value) << "JsonCpp comparison is strict on exact integral type";
    EXPECT_TRUE(int64Val == uint64Val) << "JsonValue helper API does integral type coercion";

    EXPECT_TRUE(uintVal.m_value == uint64Val.m_value) << "JsonCpp comparison allows comparing uint32 with uint64";
    EXPECT_TRUE(uintVal == uint64Val) << "JsonValue helper API does integral type coercion";

    JsonValue boolVal(Json::Value(true));
    EXPECT_TRUE(boolVal.m_value.isBool());
    EXPECT_FALSE(boolVal.m_value.isUInt());
    EXPECT_FALSE(boolVal.m_value.isInt());
    EXPECT_TRUE(boolVal.m_value.isIntegral());
    EXPECT_FALSE(boolVal.m_value == int64Val.m_value) << "JsonCpp comparison is strict on exact integral type";
    EXPECT_EQ(boolVal.m_value.asUInt(), int64Val.m_value.asUInt()) << "JsonCpp asUInt is supported on bools";
    EXPECT_FALSE(boolVal == int64Val) << "JsonValueHelper does not treat bools equal with other numeric values";
    }

END_ECDBUNITTESTS_NAMESPACE
