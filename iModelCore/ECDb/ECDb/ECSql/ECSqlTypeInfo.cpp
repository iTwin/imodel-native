/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlTypeInfo& ECSqlTypeInfo::operator= (ECSqlTypeInfo const& rhs)
    {
    if (this != &rhs)
        {
        m_kind = rhs.m_kind;
        m_primitiveType = rhs.m_primitiveType;
        m_dateTimeInfo = rhs.m_dateTimeInfo;
        m_enumType = rhs.m_enumType;
        m_extendedTypeName = rhs.m_extendedTypeName;
        m_structType = rhs.m_structType;
        m_minOccurs = rhs.m_minOccurs;
        m_maxOccurs = rhs.m_maxOccurs;
        m_propertyMap = rhs.m_propertyMap;
        }

    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlTypeInfo& ECSqlTypeInfo::operator= (ECSqlTypeInfo&& rhs)
    {
    if (this != &rhs)
        {
        m_kind = std::move(rhs.m_kind);
        m_primitiveType = std::move(rhs.m_primitiveType);
        m_dateTimeInfo = std::move(rhs.m_dateTimeInfo);
        m_enumType = std::move(rhs.m_enumType);
        m_extendedTypeName = std::move(rhs.m_extendedTypeName);
        m_structType = std::move(rhs.m_structType);
        m_minOccurs = std::move(rhs.m_minOccurs);
        m_maxOccurs = std::move(rhs.m_maxOccurs);
        m_propertyMap = std::move(rhs.m_propertyMap);
        }

    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     09/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool ECSqlTypeInfo::CanCompare(ECSqlTypeInfo const& rhs, Utf8String* errorMessage) const
    {
    const Kind lhsKind = GetKind();
    const Kind rhsKind = rhs.GetKind();
    //if unset or varies, types are not comparable. If one is NULL it is always comparable.
    //In the remaining cases, the kind needs to be the same.
    if (lhsKind == Kind::Unset || rhsKind == Kind::Unset ||
        lhsKind == Kind::Varies || rhsKind == Kind::Varies ||
        (lhsKind != Kind::Null && rhsKind != Kind::Null && lhsKind != rhsKind))
        return false;

    if (lhsKind == Kind::Null || rhsKind == Kind::Null)
        return true;

    if ((lhsKind == Kind::Navigation && rhsKind != Kind::Navigation) ||
        (lhsKind != Kind::Navigation && rhsKind == Kind::Navigation))
        {
        if (errorMessage != nullptr)
            *errorMessage = "Left and right side of expression must both be Navigation properties.";

        return false;
        }

    //for navigation props, no further checks. Equality of kind is sufficient.
    if (lhsKind == Kind::Navigation)
        return true;

    //now detail checks (kinds are equal on both sides)

    bool canCompare = false;
    //struct check
    if (lhsKind == Kind::Struct || lhsKind == Kind::StructArray)
        {
        canCompare = GetStructType().GetId() == rhs.GetStructType().GetId();
        if (!canCompare && errorMessage != nullptr)
            *errorMessage = "Left and right side of expression must both be of same Struct or StructArray type.";

        return canCompare;
        }

    //primitive or primitive array checks
    const PrimitiveType lhsType = GetPrimitiveType();
    const PrimitiveType rhsType = rhs.GetPrimitiveType();

    //DateTime only matches if their DateTimeInfo matches too (for primitive and primitive array props)
    if (lhsType == PRIMITIVETYPE_DateTime && rhsType == PRIMITIVETYPE_DateTime)
        {
        canCompare = DateTimeInfoMatches(rhs.GetDateTimeInfo());
        if (!canCompare && errorMessage != nullptr)
            *errorMessage = "DateTime metadata of left and right side of expression do not match.";

        return canCompare;
        }

    //prim array check
    if (lhsKind == Kind::PrimitiveArray)
        {
        canCompare = lhsType == rhsType;
        if (!canCompare && errorMessage != nullptr)
            *errorMessage = "Left and right side of expression must both be of same primitive array type.";

        return canCompare;
        }

    //Points / IGeometry must be same on both sides, too. For all other cases, do what SQLite does with it
    canCompare = (lhsType != PRIMITIVETYPE_Point2d && rhsType != PRIMITIVETYPE_Point2d &&
                  lhsType != PRIMITIVETYPE_Point3d && rhsType != PRIMITIVETYPE_Point3d &&
                  lhsType != PRIMITIVETYPE_IGeometry && rhsType != PRIMITIVETYPE_IGeometry) || lhsType == rhsType;

    if (!canCompare && errorMessage != nullptr)
        *errorMessage = "Left and right side of expression must both have Point2d / Point3d / IGeometry type.";

    return canCompare;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     07/2014
//+---------------+---------------+---------------+---------------+---------------+--------
bool ECSqlTypeInfo::DateTimeInfoMatches(DateTime::Info const& rhs) const
    {
    BeAssert(m_dateTimeInfo != nullptr && "DateTime ECSqlTypeInfo always have m_dateTimeInfo set");
    if (!m_dateTimeInfo.Value().IsValid() || !rhs.IsValid() || m_dateTimeInfo.Value() == rhs)
        return true;

    //We allow date-only to interact with date and time (but not with time of day)
    if (m_dateTimeInfo.Value().GetComponent() == DateTime::Component::Date && rhs.GetComponent() == DateTime::Component::DateAndTime)
        return true;

    if (m_dateTimeInfo.Value().GetComponent() == DateTime::Component::DateAndTime && rhs.GetComponent() == DateTime::Component::Date)
        return true;

    return false;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlTypeInfo::DetermineTypeInfo(ECPropertyCR ecProperty)
    {
    if (ecProperty.GetIsNavigation())
        {
        BeAssert(!ecProperty.GetAsNavigationProperty()->IsMultiple() && "Should have been caught at schema import time");
        m_kind = Kind::Navigation;
        return;
        }

    const bool isArray = ecProperty.GetIsArray();
    if (isArray)
        {
        ArrayECPropertyCP arrayProperty = ecProperty.GetAsArrayProperty();
        BeAssert(nullptr != arrayProperty);
        if (arrayProperty->GetIsStructArray())
            {
            m_kind = Kind::StructArray;
            m_structType = &arrayProperty->GetAsStructArrayProperty()->GetStructElementType();
            }
        else if (arrayProperty->GetIsPrimitiveArray())
            {
            m_kind = Kind::PrimitiveArray;
            PrimitiveArrayECPropertyCP primArrayProp = arrayProperty->GetAsPrimitiveArrayProperty();
            m_enumType = primArrayProp->GetEnumeration();
            //we capture the primitive type of the enum as well, as we allow compare enum expressions with non-enum primitive expressions
            m_primitiveType = m_enumType != nullptr ? m_enumType->GetType(): primArrayProp->GetPrimitiveElementType();

            if (primArrayProp->HasExtendedType())
                m_extendedTypeName = primArrayProp->GetExtendedTypeName().c_str();
            }

        m_minOccurs = arrayProperty->GetMinOccurs();
        m_maxOccurs = arrayProperty->GetMaxOccurs();
        }
    else if (ecProperty.GetIsStruct())
        {
        m_kind = Kind::Struct;
        m_structType = &ecProperty.GetAsStructProperty()->GetType();
        }
    else if (ecProperty.GetIsPrimitive())
        {
        m_kind = Kind::Primitive;
        PrimitiveECPropertyCP primProp = ecProperty.GetAsPrimitiveProperty();
        m_enumType = primProp->GetEnumeration();
        //we capture the primitive type of the enum as well, as we allow compare enum expressions with non-enum primitive expressions
        m_primitiveType = m_enumType != nullptr ? m_enumType->GetType() : primProp->GetType();

        if (primProp->HasExtendedType())
            m_extendedTypeName = primProp->GetExtendedTypeName().c_str();
        }

    if (m_primitiveType == ECN::PRIMITIVETYPE_DateTime)
        {
        DateTime::Info dateTimeInfo;
        CoreCustomAttributeHelper::GetDateTimeInfo(dateTimeInfo, ecProperty);
        //always set datetimeinfo, even if DateTime::Info::IsValid is false. m_dateTimeInfo is only nullptr for non-DateTime types.
        m_dateTimeInfo = dateTimeInfo;
        }
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
