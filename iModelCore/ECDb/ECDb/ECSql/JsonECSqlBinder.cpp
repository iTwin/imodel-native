/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   09/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus JsonECSqlBinder::BindValue(IECSqlBinder& binder, JsonValueCR memberJson, ECN::ECProperty const& prop, IECClassLocater& classLocater)
    {
    if (memberJson.isNull())
        return ECSqlStatus::Success;

    if (!prop.GetIsNavigation())
        return BindValue(binder, memberJson, prop);

    return BindNavigationValue(binder, memberJson, classLocater);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   09/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus JsonECSqlBinder::BindValue(IECSqlBinder& binder, JsonValueCR memberJson, ECN::ECProperty const& prop)
    {
    BeAssert(!prop.GetIsNavigation() && "This overload must not be called for nav props");
    if (memberJson.isNull())
        return ECSqlStatus::Success;

    if (prop.GetIsPrimitive())
        return BindPrimitiveValue(binder, memberJson, prop.GetAsPrimitiveProperty()->GetType());

    if (prop.GetIsStruct())
        return BindStructValue(binder, memberJson, prop.GetAsStructProperty()->GetType());

    if (prop.GetIsPrimitiveArray())
        return BindPrimitiveArrayValue(binder, memberJson, prop.GetAsPrimitiveArrayProperty()->GetPrimitiveElementType());

    if (prop.GetIsStructArray())
        return BindStructArrayValue(binder, memberJson, prop.GetAsStructArrayProperty()->GetStructElementType());

    BeAssert(false);
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   09/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus JsonECSqlBinder::BindPrimitiveValue(IECSqlBinder& binder, JsonValueCR json, ECN::PrimitiveType primType)
    {
    if (json.isNull())
        return ECSqlStatus::Success;

    switch (primType)
        {
            case ECN::PRIMITIVETYPE_Binary:
            {
            ByteStream blob;
            if (SUCCESS != ECN::ECJsonUtilities::JsonToBinary(blob, json))
                return ECSqlStatus::Error;

            return binder.BindBlob(blob.data(), (int) blob.size(), IECSqlBinder::MakeCopy::Yes);
            }

            case ECN::PRIMITIVETYPE_Boolean:
                if (!json.isBool())
                    return ECSqlStatus::Error;

                return binder.BindBoolean(json.asBool());

            case ECN::PRIMITIVETYPE_DateTime:
            {
            DateTime dt;
            if (SUCCESS != ECN::ECJsonUtilities::JsonToDateTime(dt, json))
                return ECSqlStatus::Error;

            return binder.BindDateTime(dt);
            }

            case ECN::PRIMITIVETYPE_Double:
            {
            if (!json.isConvertibleTo(Json::ValueType::realValue))
                return ECSqlStatus::Error;

            return binder.BindDouble(json.asDouble());
            }

            case ECN::PRIMITIVETYPE_IGeometry:
            {
            IGeometryPtr geom = ECJsonUtilities::JsonToIGeometry(json);
            if (geom == nullptr)
                return ECSqlStatus::Error;

            return binder.BindGeometry(*geom);
            }

            case ECN::PRIMITIVETYPE_Integer:
            {
            if (!json.isConvertibleTo(Json::ValueType::intValue))
                return ECSqlStatus::Error;

            return binder.BindInt(json.asInt());
            }

            case ECN::PRIMITIVETYPE_Long:
            {
            int64_t val = 0;
            if (SUCCESS != ECJsonUtilities::JsonToInt64(val, json))
                return ECSqlStatus::Error;

            return binder.BindInt64(val);
            }

            case ECN::PRIMITIVETYPE_Point2d:
            {
            DPoint2d pt;
            if (SUCCESS != ECJsonUtilities::JsonToPoint2d(pt, json))
                return ECSqlStatus::Error;

            return binder.BindPoint2d(pt);
            }

            case ECN::PRIMITIVETYPE_Point3d:
            {
            DPoint3d pt;
            if (SUCCESS != ECJsonUtilities::JsonToPoint3d(pt, json))
                return ECSqlStatus::Error;

            return binder.BindPoint3d(pt);
            }

            case ECN::PRIMITIVETYPE_String:
            {
            if (!json.isString())
                return ECSqlStatus::Error;

            return binder.BindText(json.asCString(), IECSqlBinder::MakeCopy::No);
            }

            default:
                BeAssert(false);
                return ECSqlStatus::Error;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   09/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus JsonECSqlBinder::BindStructValue(IECSqlBinder& binder, JsonValueCR structJson, ECN::ECStructClassCR structClass)
    {
    if (structJson.isNull())
        return ECSqlStatus::Success;

    if (!structJson.isObject())
        return ECSqlStatus::Error;

    for (Json::Value::iterator it = structJson.begin(); it != structJson.end(); it++)
        {
        Utf8CP memberName = it.memberName();
        JsonValueCR memberJson = *it;

        ECN::ECPropertyCP memberProp = structClass.GetPropertyP(memberName);
        if (memberProp == nullptr)
            {
            LOG.errorv("Could not bind JSON struct member to ECSqlStatement. JSON struct member %s does not exist in struct ECClass %s.",
                       memberName, structClass.GetFullName());
            return ECSqlStatus::Error;
            }

        ECSqlStatus stat = BindValue(binder[memberProp->GetName().c_str()], memberJson, *memberProp);
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   09/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus JsonECSqlBinder::BindPrimitiveArrayValue(IECSqlBinder& binder, JsonValueCR arrayJson, PrimitiveType primType) { return BindArrayValue(binder, arrayJson, &primType, nullptr); }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   09/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus JsonECSqlBinder::BindStructArrayValue(IECSqlBinder& binder, JsonValueCR arrayJson, ECStructClassCR structType) { return BindArrayValue(binder, arrayJson, nullptr, &structType); }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   09/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus JsonECSqlBinder::BindArrayValue(IECSqlBinder& binder, JsonValueCR arrayJson, PrimitiveType const* primType, ECStructClassCP structType)
    {
    if (arrayJson.isNull())
        return ECSqlStatus::Success;

    if (!arrayJson.isArray() || (primType == nullptr && structType == nullptr))
        return ECSqlStatus::Error;

    BeAssert(primType == nullptr || structType == nullptr);

    const bool isPrimitive = primType != nullptr;
    for (JsonValueCR arrayElemJson : arrayJson)
        {
        ECSqlStatus stat = isPrimitive ?
            BindPrimitiveValue(binder.AddArrayElement(), arrayElemJson, *primType) :
            BindStructValue(binder.AddArrayElement(), arrayElemJson, *structType);

        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   09/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus JsonECSqlBinder::BindNavigationValue(IECSqlBinder& binder, JsonValueCR navJson, IECClassLocater& classLocater)
    {
    if (navJson.isNull())
        return ECSqlStatus::Success;

    if (!navJson.isObject())
        return ECSqlStatus::Error;

    if (navJson.isMember(ECJsonUtilities::json_navId()))
        {
        JsonValueCR navIdJson = navJson[ECJsonUtilities::json_navId()];
        if (navIdJson.isNull())
            {
            ECSqlStatus stat = binder[ECDBSYS_PROP_NavPropId].BindNull();
            if (!stat.IsSuccess())
                return stat;
            }
        else
            {
            ECInstanceId navId = ECJsonUtilities::JsonToId<ECInstanceId>(navIdJson);
            if (!navId.IsValid())
                return ECSqlStatus::Error; //wrong format

            ECSqlStatus stat = binder[ECDBSYS_PROP_NavPropId].BindId(navId);
            if (!stat.IsSuccess())
                return stat;
            }
        }

    if (navJson.isMember(ECJsonUtilities::json_navRelClassName()))
        {
        JsonValueCR relClassNameJson = navJson[ECJsonUtilities::json_navRelClassName()];
        if (relClassNameJson.isNull())
            return binder[ECDBSYS_PROP_NavPropRelECClassId].BindNull();

        ECClassId relClassId = ECJsonUtilities::GetClassIdFromClassNameJson(relClassNameJson, classLocater);
        if (!relClassId.IsValid())
            return ECSqlStatus::Error; //wrong format

        return binder[ECDBSYS_PROP_NavPropRelECClassId].BindId(relClassId);
        }

    return ECSqlStatus::Success;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   09/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus JsonECSqlBinder::BindValue(IECSqlBinder& binder, RapidJsonValueCR memberJson, ECN::ECProperty const& prop, IECClassLocater& classLocater)
    {
    if (memberJson.IsNull())
        return ECSqlStatus::Success;

    if (!prop.GetIsNavigation())
        return BindValue(binder, memberJson, prop);

    return BindNavigationValue(binder, memberJson, classLocater);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   09/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus JsonECSqlBinder::BindValue(IECSqlBinder& binder, RapidJsonValueCR memberJson, ECN::ECProperty const& prop)
    {
    BeAssert(!prop.GetIsNavigation() && "This overload must not be called for nav props");
    if (memberJson.IsNull())
        return ECSqlStatus::Success;

    if (prop.GetIsPrimitive())
        return BindPrimitiveValue(binder, memberJson, prop.GetAsPrimitiveProperty()->GetType());

    if (prop.GetIsStruct())
        return BindStructValue(binder, memberJson, prop.GetAsStructProperty()->GetType());

    if (prop.GetIsPrimitiveArray())
        return BindPrimitiveArrayValue(binder, memberJson, prop.GetAsPrimitiveArrayProperty()->GetPrimitiveElementType());

    if (prop.GetIsStructArray())
        return BindStructArrayValue(binder, memberJson, prop.GetAsStructArrayProperty()->GetStructElementType());

    BeAssert(false);
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   09/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus JsonECSqlBinder::BindPrimitiveValue(IECSqlBinder& binder, RapidJsonValueCR json, ECN::PrimitiveType primType)
    {
    if (json.IsNull())
        return ECSqlStatus::Success;

    switch (primType)
        {
            case ECN::PRIMITIVETYPE_Binary:
            {
            ByteStream blob;
            if (SUCCESS != ECN::ECJsonUtilities::JsonToBinary(blob, json))
                return ECSqlStatus::Error;

            return binder.BindBlob(blob.data(), (int) blob.size(), IECSqlBinder::MakeCopy::Yes);
            }

            case ECN::PRIMITIVETYPE_Boolean:
                if (!json.IsBool())
                    return ECSqlStatus::Error;

                return binder.BindBoolean(json.GetBool());

            case ECN::PRIMITIVETYPE_DateTime:
            {
            DateTime dt;
            if (SUCCESS != ECN::ECJsonUtilities::JsonToDateTime(dt, json))
                return ECSqlStatus::Error;

            return binder.BindDateTime(dt);
            }

            case ECN::PRIMITIVETYPE_Double:
            {
            if (!json.IsNumber())
                return ECSqlStatus::Error;

            return binder.BindDouble(json.GetDouble());
            }

            case ECN::PRIMITIVETYPE_IGeometry:
            {
            IGeometryPtr geom = ECJsonUtilities::JsonToIGeometry(json);
            if (geom == nullptr)
                return ECSqlStatus::Error;

            return binder.BindGeometry(*geom);
            }

            case ECN::PRIMITIVETYPE_Integer:
            {
            int val = 0;
            if (json.IsInt())
                val = json.GetInt();
            else if (json.IsUint())
                val = (int) json.GetUint();
            else
                return ECSqlStatus::Error;

            return binder.BindInt(val);
            }

            case ECN::PRIMITIVETYPE_Long:
            {
            int64_t val = 0;
            if (SUCCESS != ECJsonUtilities::JsonToInt64(val, json))
                return ECSqlStatus::Error;

            return binder.BindInt64(val);
            }

            case ECN::PRIMITIVETYPE_Point2d:
            {
            DPoint2d pt;
            if (SUCCESS != ECJsonUtilities::JsonToPoint2d(pt, json))
                return ECSqlStatus::Error;

            return binder.BindPoint2d(pt);
            }

            case ECN::PRIMITIVETYPE_Point3d:
            {
            DPoint3d pt;
            if (SUCCESS != ECJsonUtilities::JsonToPoint3d(pt, json))
                return ECSqlStatus::Error;

            return binder.BindPoint3d(pt);
            }

            case ECN::PRIMITIVETYPE_String:
            {
            if (!json.IsString())
                return ECSqlStatus::Error;

            return binder.BindText(json.GetString(), IECSqlBinder::MakeCopy::No);
            }

            default:
                BeAssert(false);
                return ECSqlStatus::Error;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   09/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus JsonECSqlBinder::BindStructValue(IECSqlBinder& binder, RapidJsonValueCR structJson, ECN::ECStructClassCR structClass)
    {
    if (structJson.IsNull())
        return ECSqlStatus::Success;

    if (!structJson.IsObject())
        return ECSqlStatus::Error;

    for (rapidjson::Value::ConstMemberIterator it = structJson.MemberBegin(); it != structJson.MemberEnd(); ++it)
        {
        Utf8CP memberName = it->name.GetString();
        RapidJsonValueCR memberJson = it->value;

        ECN::ECPropertyCP memberProp = structClass.GetPropertyP(memberName);
        if (memberProp == nullptr)
            {
            LOG.errorv("Could not bind JSON struct member to ECSqlStatement. JSON struct member %s does not exist in struct ECClass %s.",
                       memberName, structClass.GetFullName());
            return ECSqlStatus::Error;
            }

        ECSqlStatus stat = BindValue(binder[memberProp->GetName().c_str()], memberJson, *memberProp);
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   09/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus JsonECSqlBinder::BindPrimitiveArrayValue(IECSqlBinder& binder, RapidJsonValueCR arrayJson, PrimitiveType primType) { return BindArrayValue(binder, arrayJson, &primType, nullptr); }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   09/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus JsonECSqlBinder::BindStructArrayValue(IECSqlBinder& binder, RapidJsonValueCR arrayJson, ECStructClassCR structType) { return BindArrayValue(binder, arrayJson, nullptr, &structType); }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   09/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus JsonECSqlBinder::BindArrayValue(IECSqlBinder& binder, RapidJsonValueCR arrayJson, PrimitiveType const* primType, ECStructClassCP structType)
    {
    if (arrayJson.IsNull())
        return ECSqlStatus::Success;

    if (!arrayJson.IsArray() || (primType == nullptr && structType == nullptr))
        return ECSqlStatus::Error;

    BeAssert(primType == nullptr || structType == nullptr);

    const bool isPrimitive = primType != nullptr;
    for (RapidJsonValueCR arrayElemJson : arrayJson.GetArray())
        {
        ECSqlStatus stat = isPrimitive ?
            BindPrimitiveValue(binder.AddArrayElement(), arrayElemJson, *primType) :
            BindStructValue(binder.AddArrayElement(), arrayElemJson, *structType);

        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   09/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus JsonECSqlBinder::BindNavigationValue(IECSqlBinder& binder, RapidJsonValueCR navJson, IECClassLocater& classLocater)
    {
    if (navJson.IsNull())
        return ECSqlStatus::Success;

    if (!navJson.IsObject())
        return ECSqlStatus::Error;

    if (navJson.HasMember(ECJsonSystemNames::Navigation::Id()))
        {
        RapidJsonValueCR navIdJson = navJson[ECJsonSystemNames::Navigation::Id()];
        if (navIdJson.IsNull())
            {
            ECSqlStatus stat = binder[ECDBSYS_PROP_NavPropId].BindNull();
            if (!stat.IsSuccess())
                return stat;
            }
        else
            {
            ECInstanceId navId = ECJsonUtilities::JsonToId<ECInstanceId>(navIdJson);
            if (!navId.IsValid())
                return ECSqlStatus::Error; //wrong format

            ECSqlStatus stat = binder[ECDBSYS_PROP_NavPropId].BindId(navId);
            if (!stat.IsSuccess())
                return stat;
            }
        }

    if (navJson.HasMember(ECJsonSystemNames::Navigation::RelClassName()))
        {
        RapidJsonValueCR relClassNameJson = navJson[ECJsonSystemNames::Navigation::RelClassName()];
        if (relClassNameJson.IsNull())
            return binder[ECDBSYS_PROP_NavPropRelECClassId].BindNull();

        ECClassId relClassId = ECJsonUtilities::GetClassIdFromClassNameJson(relClassNameJson, classLocater);
        if (!relClassId.IsValid())
            return ECSqlStatus::Error; //wrong format

        return binder[ECDBSYS_PROP_NavPropRelECClassId].BindId(relClassId);
        }

    return ECSqlStatus::Success;
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
