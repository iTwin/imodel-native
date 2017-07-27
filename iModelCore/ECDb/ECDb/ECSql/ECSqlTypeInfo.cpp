/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlTypeInfo.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlTypeInfo::ECSqlTypeInfo(ECSqlTypeInfo::Kind kind)
    : m_kind(kind), m_structType(nullptr), m_minOccurs(0), m_maxOccurs(std::numeric_limits<uint32_t>::max()), m_propertyMap(nullptr), m_primitiveType(static_cast<ECN::PrimitiveType>(0))
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlTypeInfo::ECSqlTypeInfo(ECN::PrimitiveType primitiveType, bool isArray, DateTime::Info const* dateTimeInfo /*= nullptr*/)
    : m_structType(nullptr), m_minOccurs(0), m_maxOccurs(std::numeric_limits<uint32_t>::max()), m_propertyMap(nullptr)
    {
    Populate(isArray, &primitiveType, nullptr, -1, -1, dateTimeInfo);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlTypeInfo::ECSqlTypeInfo(ECN::ECStructClassCR structType, bool isArray)
    : m_primitiveType(static_cast<ECN::PrimitiveType>(0)), m_structType(nullptr), m_minOccurs(0), m_maxOccurs(std::numeric_limits<uint32_t>::max()), m_propertyMap(nullptr)
    {
    Populate(isArray, nullptr, &structType, -1, -1, nullptr);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     04/2014
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlTypeInfo::ECSqlTypeInfo(ECN::ECPropertyCR ecProperty)
    : m_structType(nullptr), m_primitiveType(static_cast<ECN::PrimitiveType>(0)), m_minOccurs(0), m_maxOccurs(std::numeric_limits<uint32_t>::max()), m_propertyMap(nullptr)
    {
    DetermineTypeInfo(ecProperty);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     09/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlTypeInfo::ECSqlTypeInfo(PropertyMap const& propertyMap)
    : m_structType(nullptr), m_propertyMap(&propertyMap), m_primitiveType(static_cast<ECN::PrimitiveType>(0)), m_minOccurs(0), m_maxOccurs(std::numeric_limits<uint32_t>::max())
    {
    DetermineTypeInfo(propertyMap.GetProperty());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlTypeInfo::ECSqlTypeInfo(ECSqlTypeInfo const& rhs)
    : m_kind(rhs.m_kind), m_primitiveType(rhs.m_primitiveType), m_dateTimeInfo(rhs.m_dateTimeInfo), m_structType(rhs.m_structType),
    m_minOccurs(rhs.m_minOccurs), m_maxOccurs(rhs.m_maxOccurs), m_propertyMap(rhs.m_propertyMap)
    {}

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
ECSqlTypeInfo::ECSqlTypeInfo(ECSqlTypeInfo&& rhs)
    : m_kind(std::move(rhs.m_kind)), m_primitiveType(std::move(rhs.m_primitiveType)), m_dateTimeInfo(std::move(rhs.m_dateTimeInfo)), m_structType(std::move(rhs.m_structType)),
    m_minOccurs(std::move(rhs.m_minOccurs)), m_maxOccurs(std::move(rhs.m_maxOccurs)), m_propertyMap(std::move(rhs.m_propertyMap))
    {}


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
void ECSqlTypeInfo::Populate(bool isArray, ECN::PrimitiveType const* primitiveType, ECN::ECStructClassCP structType, int minOccurs, int maxOccurs, DateTime::Info const* dateTimeInfo)
    {
    if (primitiveType != nullptr)
        {
        m_primitiveType = *primitiveType;
        m_kind = isArray ? Kind::PrimitiveArray : Kind::Primitive;

        if (dateTimeInfo != nullptr)
            m_dateTimeInfo = *dateTimeInfo;
        }
    else
        m_primitiveType = static_cast<ECN::PrimitiveType>(0);

    if (structType != nullptr)
        {
        m_structType = structType;
        m_kind = isArray ? Kind::StructArray : Kind::Struct;
        }
    else
        m_structType = nullptr;

    m_minOccurs = minOccurs >= 0 ? (uint32_t) minOccurs : 0;
    m_maxOccurs = maxOccurs >= 0 ? (uint32_t) maxOccurs : std::numeric_limits<uint32_t>::max();
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
    if (!m_dateTimeInfo.IsValid() || !rhs.IsValid())
        return true;

    //We allow date-only to interact with datetimes of any kind
    if (m_dateTimeInfo.GetComponent() == DateTime::Component::Date ||
        rhs.GetComponent() == DateTime::Component::Date)
        return true;

    return m_dateTimeInfo.GetKind() == rhs.GetKind();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlTypeInfo::DetermineTypeInfo(ECPropertyCR ecProperty)
    {
    if (ecProperty.GetIsNavigation())
        {
        NavigationECPropertyCP navProp = ecProperty.GetAsNavigationProperty();
        BeAssert(!navProp->IsMultiple() && "Should have been caught at schema import time");

        m_kind = Kind::Navigation;
        return;
        }

    PrimitiveType primitiveType = ECN::PRIMITIVETYPE_Integer;
    ECStructClassCP structType = nullptr;
    bool isArray = ecProperty.GetIsArray();

    uint32_t minOccurs = 0;
    uint32_t maxOccurs = 0;

    if (isArray)
        {
        ArrayECPropertyCP arrayProperty = ecProperty.GetAsArrayProperty();
        if (arrayProperty->GetIsStructArray())
            structType = &arrayProperty->GetAsStructArrayProperty()->GetStructElementType();
        else if (arrayProperty->GetIsPrimitiveArray())
            primitiveType = arrayProperty->GetAsPrimitiveArrayProperty()->GetPrimitiveElementType();

        BeAssert(nullptr != arrayProperty);

        minOccurs = arrayProperty->GetMinOccurs();
        maxOccurs = arrayProperty->GetMaxOccurs();
        }
    //no array
    else if (ecProperty.GetIsStruct())
        structType = &ecProperty.GetAsStructProperty()->GetType();
    else if (ecProperty.GetIsPrimitive())
        primitiveType = ecProperty.GetAsPrimitiveProperty()->GetType();

    if (structType != nullptr)
        {
        Populate(isArray, nullptr, structType, minOccurs, maxOccurs, nullptr);
        return;
        }

    if (primitiveType == ECN::PRIMITIVETYPE_DateTime)
        {
        DateTime::Info dateTimeInfo;
        CoreCustomAttributeHelper::GetDateTimeInfo(dateTimeInfo, ecProperty);
        Populate(isArray, &primitiveType, nullptr, minOccurs, maxOccurs, &dateTimeInfo);
        }
    else
        Populate(isArray, &primitiveType, nullptr, minOccurs, maxOccurs, nullptr);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
