/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlPropertyNamer.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ECSqlPropertyNamer::GetJsNameForProp(
    ExtendedTypeHelper::ExtendedType extType, Utf8StringCR ecName, bool useClassFullName) {

    if (extType == ExtendedTypeHelper::ExtendedType::Id && ecName.EqualsIAscii(ECDBSYS_PROP_ECInstanceId))
        return ECN::ECJsonSystemNames::Id();

    if (extType == ExtendedTypeHelper::ExtendedType::ClassId && ecName.EqualsIAscii(ECDBSYS_PROP_ECClassId))
        return useClassFullName ? ECN::ECJsonSystemNames::ClassFullName() : ECN::ECJsonSystemNames::ClassName();

    if (extType == ExtendedTypeHelper::ExtendedType::SourceId && ecName.EqualsIAscii(ECDBSYS_PROP_SourceECInstanceId))
        return ECN::ECJsonSystemNames::SourceId();

    if (extType == ExtendedTypeHelper::ExtendedType::SourceClassId && ecName.EqualsIAscii(ECDBSYS_PROP_SourceECClassId))
        return ECN::ECJsonSystemNames::SourceClassName();

    if (extType == ExtendedTypeHelper::ExtendedType::TargetId && ecName.EqualsIAscii(ECDBSYS_PROP_TargetECInstanceId))
        return ECN::ECJsonSystemNames::TargetId();

    if (extType == ExtendedTypeHelper::ExtendedType::TargetClassId && ecName.EqualsIAscii(ECDBSYS_PROP_TargetECClassId))
        return ECN::ECJsonSystemNames::TargetClassName();

    if (extType == ExtendedTypeHelper::ExtendedType::NavId && ecName.EqualsIAscii(ECDBSYS_PROP_NavPropId))
        return ECN::ECJsonSystemNames::Navigation::Id();

    if (extType == ExtendedTypeHelper::ExtendedType::NavRelClassId && ecName.EqualsIAscii(ECDBSYS_PROP_NavPropRelECClassId))
        return ECN::ECJsonSystemNames::Navigation::RelClassName();

    Utf8String name = ecName;
    ECN::ECJsonUtilities::LowerFirstChar(name);
    return name;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP ECSqlPropertyNamer::GetSubPropertyLeafJsName(Utf8StringCR leafName) {
    if (leafName == ECDBSYS_PROP_NavPropId)
        return ECN::ECJsonSystemNames::Navigation::Id();
    if (leafName == ECDBSYS_PROP_NavPropRelECClassId)
        return ECN::ECJsonSystemNames::Navigation::RelClassName();
    if (leafName == ECDBSYS_PROP_PointX)
        return ECN::ECJsonSystemNames::Point::X();
    if (leafName == ECDBSYS_PROP_PointY)
        return ECN::ECJsonSystemNames::Point::Y();
    if (leafName == ECDBSYS_PROP_PointZ)
        return ECN::ECJsonSystemNames::Point::Z();
    return leafName.c_str();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ECSqlPropertyNamer::GetJsNameForProp(ECN::ECPropertyCP prop, bool useClassFullName) {
    BeAssert(prop != nullptr);
    if(prop->GetClass().IsStructClass()) {
        // For struct properties we want to return the name with the first char lower-cased
        auto name = prop->GetName();
        ECN::ECJsonUtilities::LowerFirstChar(name);
        return name;
    }

    const auto prim = prop->GetAsPrimitiveProperty();
    const auto extendTypeId = (prim && !prim->GetExtendedTypeName().empty())
        ? ExtendedTypeHelper::GetExtendedType(prim->GetExtendedTypeName())
        : ExtendedTypeHelper::ExtendedType::Unknown;
    return GetJsNameForProp(extendTypeId, prop->GetName(), useClassFullName);
}

END_BENTLEY_SQLITE_EC_NAMESPACE
