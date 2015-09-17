/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlTestFixture.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlTestFixture.h"
#include "ECSqlStatementCrudAsserter.h"

using namespace std;
USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//********************* ECSqlTestFixture ********************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestFixture::ECSqlTestFixture ()
    : m_testProject (nullptr)
    {
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestFixture::~ECSqlTestFixture ()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
ECDbR ECSqlTestFixture::SetUp (Utf8CP ecdbFileName, WCharCP schemaECXmlFileName, ECDb::OpenParams openParams, int perClassRowCount)
    {
    return _SetUp (ecdbFileName, schemaECXmlFileName, openParams, perClassRowCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
//virtual
ECDbR ECSqlTestFixture::_SetUp (Utf8CP ecdbFileName, WCharCP schemaECXmlFileName, ECDb::OpenParams openParams, int perClassRowCount)
    {
    // Create and populate a sample project
    m_testProject = CreateTestProject (ecdbFileName, schemaECXmlFileName, openParams, perClassRowCount);
    return GetTestProject ().GetECDb ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
unique_ptr<ECDbTestProject> ECSqlTestFixture::CreateTestProject (Utf8CP ecdbFileName, WCharCP schemaECXmlFileName, ECDb::OpenParams openParams, int perClassRowCount)
    {
    Utf8String filePath;
    // Create and populate a sample project
        {
        ECDbTestProject testProject;
        auto& ecdb = testProject.Create (ecdbFileName, schemaECXmlFileName, perClassRowCount);
        filePath = ecdb.GetDbFileName();
        }

    //re-open the file so that we can determine the open mode

    auto testProject = unique_ptr<ECDbTestProject> (new ECDbTestProject ());
    testProject->Open (filePath.c_str (), openParams);

    return move (testProject);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
void ECSqlTestFixture::BindFromJson (BentleyStatus& succeeded, ECSqlStatement const& statement, JsonValueCR jsonValue, IECSqlBinder& binder)
    {
    succeeded = ERROR;
    BeTest::SetFailOnAssert (false);

    ECSqlStatus stat = ECSqlStatus::Success;
    switch (jsonValue.type ())
        {
            case Json::nullValue:
                stat = binder.BindNull ();
                break;
            case Json::booleanValue:
                stat = binder.BindBoolean (jsonValue.asBool ());
                break;
            case Json::realValue:
                stat = binder.BindDouble (jsonValue.asDouble ());
                break;
            case Json::intValue:
                stat = binder.BindInt64 (jsonValue.asInt64 ());
                break;
            case Json::stringValue:
                stat = binder.BindText (jsonValue.asCString (), IECSqlBinder::MakeCopy::Yes);
                break;

            case Json::objectValue:
                {
                //special primitive types which don't match directly to JSON primitives
                if (jsonValue.isMember ("type"))
                    {
                    Utf8String type = jsonValue["type"].asString ();
                    if (type.EqualsI ("datetime"))
                        {
                        DateTime dt;
                        DateTime::FromString (dt, jsonValue["datetime"].asCString ());
                        stat = binder.BindDateTime (dt);
                        }
                    else if (type.EqualsI ("point2d"))
                        stat = binder.BindPoint2D (DPoint2d::From (jsonValue["x"].asDouble (), jsonValue["y"].asDouble ()));
                    else if (type.EqualsI ("point3d"))
                        stat = binder.BindPoint3D (DPoint3d::From (jsonValue["x"].asDouble (), jsonValue["y"].asDouble (), jsonValue["z"].asDouble ()));
                    }
                else //struct
                    {
                    IECSqlStructBinder& structBinder = binder.BindStruct ();
                    for (Utf8StringCR memberName : jsonValue.getMemberNames ())
                        {
                        Json::Value const& member = jsonValue[memberName.c_str ()];
                        auto& memberBinder = structBinder.GetMember (memberName.c_str ());

                        BindFromJson (succeeded, statement, member, memberBinder);
                        if (succeeded != SUCCESS)
                            return;
                        }
                    stat = ECSqlStatus::Success;
                    }
                break;
                }
            case Json::arrayValue:
                {
                IECSqlArrayBinder& arrayBinder = binder.BindArray ((uint32_t) jsonValue.size ());
                for (JsonValueCR arrayElement : jsonValue)
                    {
                    IECSqlBinder& arrayElementBinder = arrayBinder.AddArrayElement ();
                    BindFromJson (succeeded, statement, arrayElement, arrayElementBinder);
                    if (succeeded != SUCCESS)
                        return;
                    }
                stat = ECSqlStatus::Success;
                }
        }

    succeeded = stat == ECSqlStatus::Success ? SUCCESS : ERROR;
    BeTest::SetFailOnAssert (true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
void ECSqlTestFixture::VerifyECSqlValue (ECSqlStatement const& statement, JsonValueCR expectedValue, IECSqlValue const& actualValue)
    {
    auto const& typeInfo = actualValue.GetColumnInfo ().GetDataType ();
    Utf8String name (actualValue.GetColumnInfo ().GetProperty ()->GetName ());
    if (expectedValue.isNull ())
        {
        if (typeInfo.IsArray ())
            ASSERT_FALSE (actualValue.IsNull ()) << "Property: " << name.c_str () << "> For arrays IECSqlValue::IsNull is always expected to return false";
        else
            ASSERT_TRUE (actualValue.IsNull ()) << "Property: " << name.c_str ();

        return;
        }

    const auto expectedType = expectedValue.type ();
    if (expectedType != Json::ValueType::objectValue && expectedType != Json::ValueType::arrayValue)
        {
        ASSERT_TRUE (typeInfo.IsPrimitive ()) << "Property: " << name.c_str ();;
        auto actualPrimType = typeInfo.GetPrimitiveType ();

        switch (expectedType)
            {
                case Json::ValueType::booleanValue:
                    {
                    ASSERT_EQ (PRIMITIVETYPE_Boolean, actualPrimType) << "Property: " << name.c_str ();;
                    ASSERT_EQ (expectedValue.asBool (), actualValue.GetBoolean ()) << "Property: " << name.c_str ();;
                    break;
                    }
                case Json::ValueType::realValue:
                    {
                    ASSERT_EQ (PRIMITIVETYPE_Double, actualPrimType) << "Property: " << name.c_str ();;
                    ASSERT_EQ (expectedValue.asDouble (), actualValue.GetDouble ()) << "Property: " << name.c_str ();;
                    break;
                    }
                case Json::ValueType::intValue:
                    {
                    ASSERT_TRUE (PRIMITIVETYPE_Integer == actualPrimType || PRIMITIVETYPE_Long == actualPrimType) << "Property: " << name.c_str ();;
                    ASSERT_EQ (expectedValue.asInt64 (), actualValue.GetInt64 ()) << "Property: " << name.c_str ();;
                    break;
                    }
                case Json::ValueType::stringValue:
                    {
                    ASSERT_EQ (PRIMITIVETYPE_String, actualPrimType) << "Property: " << name.c_str ();;
                    ASSERT_STREQ (expectedValue.asCString (), actualValue.GetText ()) << "Property: " << name.c_str ();;
                    break;
                    }
            }
        return;
        }

    if (expectedType == Json::ValueType::objectValue)
        {
        if (expectedValue.isMember ("type"))
            {
            Utf8String typeStr = expectedValue["type"].asString ();
            if (typeStr.EqualsI ("datetime"))
                {
                ASSERT_TRUE (typeInfo.IsPrimitive () && typeInfo.GetPrimitiveType () == PRIMITIVETYPE_DateTime) << "Property: " << name.c_str ();;
                ASSERT_STREQ (expectedValue["datetime"].asCString (), actualValue.GetDateTime ().ToUtf8String ().c_str ()) << "Property: " << name.c_str ();;
                }
            else if (typeStr.EqualsI ("point2d"))
                {
                ASSERT_TRUE (typeInfo.IsPrimitive () && typeInfo.GetPrimitiveType () == PRIMITIVETYPE_Point2D) << "Property: " << name.c_str ();;
                DPoint2d actualVal = actualValue.GetPoint2D ();
                ASSERT_EQ (expectedValue["x"].asDouble (), actualVal.x) << "Property: " << name.c_str ();;
                ASSERT_EQ (expectedValue["y"].asDouble (), actualVal.y) << "Property: " << name.c_str ();;
                }
            else if (typeStr.EqualsI ("point3d"))
                {
                ASSERT_TRUE (typeInfo.IsPrimitive () && typeInfo.GetPrimitiveType () == PRIMITIVETYPE_Point3D) << "Property: " << name.c_str ();;
                DPoint3d actualVal = actualValue.GetPoint3D ();
                ASSERT_EQ (expectedValue["x"].asDouble (), actualVal.x) << "Property: " << name.c_str ();;
                ASSERT_EQ (expectedValue["y"].asDouble (), actualVal.y) << "Property: " << name.c_str ();;
                ASSERT_EQ (expectedValue["z"].asDouble (), actualVal.z) << "Property: " << name.c_str ();;
                }
            }
        else //structs
            {
            ASSERT_TRUE (typeInfo.IsStruct ());
            IECSqlStructValue const& actualStructValue = actualValue.GetStruct ();
            const int structMemberCount = actualStructValue.GetMemberCount ();
            for (int i = 0; i < structMemberCount; i++)
                {
                auto const& actualMemberValue = actualStructValue.GetValue (i);
                Utf8String memberName (actualMemberValue.GetColumnInfo ().GetProperty ()->GetName ());
                Json::Value expectedMemberValue = expectedValue.get (memberName, Json::Value (Json::nullValue));
                VerifyECSqlValue (statement, expectedMemberValue, actualMemberValue);
                }
            }

        return;
        }

    ASSERT_EQ (Json::ValueType::arrayValue, expectedType);
    IECSqlArrayValue const& actualArrayValue = actualValue.GetArray ();
    int actualArrayLength = 0;
    for (IECSqlValue const* actualArrayElement : actualArrayValue)
        {
        ASSERT_TRUE (actualArrayElement != nullptr);
        actualArrayLength++;
        VerifyECSqlValue (statement, expectedValue[actualArrayLength - 1], *actualArrayElement);
        }

    ASSERT_EQ ((int) expectedValue.size (), actualArrayLength);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlTestFixture::SetTestProject (unique_ptr<ECDbTestProject> testProject)
    {
    m_testProject = move (testProject);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ECDbTestProject& ECSqlTestFixture::GetTestProject () const
    {
    return _GetTestProject ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
//virtual
ECDbTestProject& ECSqlTestFixture::_GetTestProject () const
    {
    return *m_testProject;
    }




END_ECDBUNITTESTS_NAMESPACE