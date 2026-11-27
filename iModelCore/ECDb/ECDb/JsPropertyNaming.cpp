/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/

#include "ECDbPch.h"
#include "JsPropertyNaming.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

Utf8String GetJsMemberName(ECN::ECPropertyCR ecProperty, bool useClassFullName, bool useNavigationNames) {
    Utf8String memberName = ecProperty.GetName();
    const auto prim = ecProperty.GetAsPrimitiveProperty();
    if (prim && !prim->GetExtendedTypeName().empty()) {
        const auto extendTypeId = ExtendedTypeHelper::GetExtendedType(prim->GetExtendedTypeName());
        if (extendTypeId == ExtendedTypeHelper::ExtendedType::Id && memberName.EqualsIAscii(ECDBSYS_PROP_ECInstanceId))
            memberName = ECN::ECJsonSystemNames::Id();
        else if (extendTypeId == ExtendedTypeHelper::ExtendedType::ClassId && memberName.EqualsIAscii(ECDBSYS_PROP_ECClassId))
            memberName = useClassFullName ? ECN::ECJsonSystemNames::ClassFullName() : ECN::ECJsonSystemNames::ClassName();
        else if (extendTypeId == ExtendedTypeHelper::ExtendedType::SourceId && memberName.EqualsIAscii(ECDBSYS_PROP_SourceECInstanceId))
            memberName = ECN::ECJsonSystemNames::SourceId();
        else if (extendTypeId == ExtendedTypeHelper::ExtendedType::SourceClassId && memberName.EqualsIAscii(ECDBSYS_PROP_SourceECClassId))
            memberName = ECN::ECJsonSystemNames::SourceClassName();
        else if (extendTypeId == ExtendedTypeHelper::ExtendedType::TargetId && memberName.EqualsIAscii(ECDBSYS_PROP_TargetECInstanceId))
            memberName = ECN::ECJsonSystemNames::TargetId();
        else if (extendTypeId == ExtendedTypeHelper::ExtendedType::TargetClassId && memberName.EqualsIAscii(ECDBSYS_PROP_TargetECClassId))
            memberName = ECN::ECJsonSystemNames::TargetClassName();
        else if (useNavigationNames && extendTypeId == ExtendedTypeHelper::ExtendedType::NavId && memberName.EqualsIAscii(ECDBSYS_PROP_NavPropId))
            memberName = ECN::ECJsonSystemNames::Navigation::Id();
        else if (useNavigationNames && extendTypeId == ExtendedTypeHelper::ExtendedType::NavRelClassId && memberName.EqualsIAscii(ECDBSYS_PROP_NavPropRelECClassId))
            memberName = ECN::ECJsonSystemNames::Navigation::RelClassName();
        else
            ECN::ECJsonUtilities::LowerFirstChar(memberName);
    } else {
        ECN::ECJsonUtilities::LowerFirstChar(memberName);
    }
    return memberName;
}

END_BENTLEY_SQLITE_EC_NAMESPACE
