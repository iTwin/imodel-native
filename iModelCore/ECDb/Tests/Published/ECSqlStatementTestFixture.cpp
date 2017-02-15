/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlStatementTestFixture.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlStatementTestFixture.h"

using namespace std;
USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
void ECSqlStatementTestFixture::BindFromJson(BentleyStatus& succeeded, ECSqlStatement const& statement, JsonValueCR jsonValue, IECSqlBinder& binder)
    {
    succeeded = ERROR;
    BeTest::SetFailOnAssert(false);

    ECSqlStatus stat = ECSqlStatus::Success;
    switch (jsonValue.type())
        {
            case Json::nullValue:
                stat = binder.BindNull();
                break;
            case Json::booleanValue:
                stat = binder.BindBoolean(jsonValue.asBool());
                break;
            case Json::realValue:
                stat = binder.BindDouble(jsonValue.asDouble());
                break;
            case Json::intValue:
                stat = binder.BindInt64(jsonValue.asInt64());
                break;
            case Json::stringValue:
                stat = binder.BindText(jsonValue.asCString(), IECSqlBinder::MakeCopy::Yes);
                break;

            case Json::objectValue:
            {
            //special primitive types which don't match directly to JSON primitives
            if (jsonValue.isMember("type"))
                {
                Utf8String type = jsonValue["type"].asString();
                if (type.EqualsI("datetime"))
                    {
                    DateTime dt;
                    DateTime::FromString(dt, jsonValue["datetime"].asCString());
                    stat = binder.BindDateTime(dt);
                    }
                else if (type.EqualsI("point2d"))
                    stat = binder.BindPoint2d(DPoint2d::From(jsonValue["x"].asDouble(), jsonValue["y"].asDouble()));
                else if (type.EqualsI("point3d"))
                    stat = binder.BindPoint3d(DPoint3d::From(jsonValue["x"].asDouble(), jsonValue["y"].asDouble(), jsonValue["z"].asDouble()));
                }
            else //struct
                {
                for (Utf8StringCR memberName : jsonValue.getMemberNames())
                    {
                    Json::Value const& member = jsonValue[memberName.c_str()];
                    auto& memberBinder = binder[memberName.c_str()];

                    BindFromJson(succeeded, statement, member, memberBinder);
                    if (succeeded != SUCCESS)
                        return;
                    }
                stat = ECSqlStatus::Success;
                }
            break;
            }
            case Json::arrayValue:
            {
            for (JsonValueCR arrayElement : jsonValue)
                {
                IECSqlBinder& elementBinder = binder.AddArrayElement();
                BindFromJson(succeeded, statement, arrayElement, elementBinder);
                if (succeeded != SUCCESS)
                    return;
                }
            stat = ECSqlStatus::Success;
            }
        }

    succeeded = stat == ECSqlStatus::Success ? SUCCESS : ERROR;
    BeTest::SetFailOnAssert(true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
void ECSqlStatementTestFixture::VerifyECSqlValue(ECSqlStatement const& statement, JsonValueCR expectedValue, IECSqlValue const& actualValue)
    {
    ECTypeDescriptor const& typeInfo = actualValue.GetColumnInfo().GetDataType();
    Utf8StringCR name = actualValue.GetColumnInfo().GetProperty()->GetName();
    if (expectedValue.isNull())
        {
        ASSERT_TRUE(actualValue.IsNull()) << "Property: " << name.c_str();
        return;
        }

    const Json::ValueType expectedType = expectedValue.type();
    if (expectedType != Json::ValueType::objectValue && expectedType != Json::ValueType::arrayValue)
        {
        ASSERT_TRUE(typeInfo.IsPrimitive()) << "Property: " << name.c_str();;
        PrimitiveType actualPrimType = typeInfo.GetPrimitiveType();

        switch (expectedType)
            {
                case Json::ValueType::booleanValue:
                {
                ASSERT_EQ(PRIMITIVETYPE_Boolean, actualPrimType) << "Property: " << name.c_str();
                ASSERT_EQ(expectedValue.asBool(), actualValue.GetBoolean()) << "Property: " << name.c_str();
                break;
                }
                case Json::ValueType::realValue:
                {
                ASSERT_EQ(PRIMITIVETYPE_Double, actualPrimType) << "Property: " << name.c_str();;
                ASSERT_EQ(expectedValue.asDouble(), actualValue.GetDouble()) << "Property: " << name.c_str();;
                break;
                }
                case Json::ValueType::intValue:
                {
                ASSERT_TRUE(PRIMITIVETYPE_Integer == actualPrimType || PRIMITIVETYPE_Long == actualPrimType) << "Property: " << name.c_str();;
                ASSERT_EQ(expectedValue.asInt64(), actualValue.GetInt64()) << "Property: " << name.c_str();;
                break;
                }
                case Json::ValueType::stringValue:
                {
                ASSERT_EQ(PRIMITIVETYPE_String, actualPrimType) << "Property: " << name.c_str();;
                ASSERT_STREQ(expectedValue.asCString(), actualValue.GetText()) << "Property: " << name.c_str();;
                break;
                }
            }
        return;
        }

    if (expectedType == Json::ValueType::objectValue)
        {
        if (expectedValue.isMember("type"))
            {
            Utf8String typeStr = expectedValue["type"].asString();
            if (typeStr.EqualsI("datetime"))
                {
                ASSERT_TRUE(typeInfo.IsPrimitive() && typeInfo.GetPrimitiveType() == PRIMITIVETYPE_DateTime) << "Property: " << name.c_str();;
                ASSERT_STREQ(expectedValue["datetime"].asCString(), actualValue.GetDateTime().ToString().c_str()) << "Property: " << name.c_str();;
                }
            else if (typeStr.EqualsI("point2d"))
                {
                ASSERT_TRUE(typeInfo.IsPrimitive() && typeInfo.GetPrimitiveType() == PRIMITIVETYPE_Point2d) << "Property: " << name.c_str();;
                DPoint2d actualVal = actualValue.GetPoint2d();
                ASSERT_EQ(expectedValue["x"].asDouble(), actualVal.x) << "Property: " << name.c_str();;
                ASSERT_EQ(expectedValue["y"].asDouble(), actualVal.y) << "Property: " << name.c_str();;
                }
            else if (typeStr.EqualsI("point3d"))
                {
                ASSERT_TRUE(typeInfo.IsPrimitive() && typeInfo.GetPrimitiveType() == PRIMITIVETYPE_Point3d) << "Property: " << name.c_str();;
                DPoint3d actualVal = actualValue.GetPoint3d();
                ASSERT_EQ(expectedValue["x"].asDouble(), actualVal.x) << "Property: " << name.c_str();;
                ASSERT_EQ(expectedValue["y"].asDouble(), actualVal.y) << "Property: " << name.c_str();;
                ASSERT_EQ(expectedValue["z"].asDouble(), actualVal.z) << "Property: " << name.c_str();;
                }
            }
        else //structs
            {
            ASSERT_TRUE(typeInfo.IsStruct());
            for (IECSqlValue const& actualMemberValue : actualValue.GetStructIterable())
                {
                Utf8String memberName(actualMemberValue.GetColumnInfo().GetProperty()->GetName());
                Json::Value expectedMemberValue = expectedValue.get(memberName, Json::Value(Json::nullValue));
                VerifyECSqlValue(statement, expectedMemberValue, actualMemberValue);
                }
            }

        return;
        }

    ASSERT_EQ(Json::ValueType::arrayValue, expectedType);
    int actualArrayLength = 0;
    for (IECSqlValue const& actualArrayElement : actualValue.GetArrayIterable())
        {
        actualArrayLength++;
        VerifyECSqlValue(statement, expectedValue[actualArrayLength - 1], actualArrayElement);
        }

    ASSERT_EQ((int) expectedValue.size(), actualArrayLength);
    }

END_ECDBUNITTESTS_NAMESPACE