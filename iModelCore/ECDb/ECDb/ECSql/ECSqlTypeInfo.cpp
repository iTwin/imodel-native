/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlTypeInfo.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlTypeInfo.h"

using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlTypeInfo::ECSqlTypeInfo (ECSqlTypeInfo::Kind kind)
: m_kind (kind), m_structType (nullptr), m_minOccurs (0), m_maxOccurs (0), m_propertyMap (nullptr), m_primitiveType (static_cast<ECN::PrimitiveType>(0))
    {
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlTypeInfo::ECSqlTypeInfo (ECN::PrimitiveType primitiveType, DateTimeInfo const* dateTimeInfo /*= nullptr*/) 
    : m_structType (nullptr), m_propertyMap (nullptr)
    {
    Populate (false, &primitiveType, nullptr, 0, 0, dateTimeInfo);
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlTypeInfo::ECSqlTypeInfo (ECN::PrimitiveType primitiveType, uint32_t minOccurs, uint32_t maxOccurs, DateTimeInfo const* dateTimeInfo /*= nullptr*/)
: m_structType (nullptr), m_propertyMap (nullptr)
    {
    Populate (true, &primitiveType, nullptr, minOccurs, maxOccurs, dateTimeInfo);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlTypeInfo::ECSqlTypeInfo (ECN::ECClassCR structType) 
: m_structType (nullptr), m_propertyMap (nullptr), m_primitiveType (static_cast<ECN::PrimitiveType>(0))
    {
    Populate (false, nullptr, &structType, 0, 0, nullptr);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlTypeInfo::ECSqlTypeInfo (ECN::ECClassCR structType, uint32_t minOccurs, uint32_t maxOccurs)
: m_structType (nullptr), m_propertyMap (nullptr), m_primitiveType (static_cast<ECN::PrimitiveType>(0))
    {
    Populate (true, nullptr, &structType, minOccurs, maxOccurs, nullptr);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     04/2014
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlTypeInfo::ECSqlTypeInfo (ECN::ECPropertyCR ecProperty)
: m_structType (nullptr), m_primitiveType (static_cast<ECN::PrimitiveType>(0))
    {
    DetermineTypeInfo (ecProperty);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     09/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlTypeInfo::ECSqlTypeInfo (PropertyMapCR propertyMap)
: m_structType (nullptr), m_propertyMap (&propertyMap), m_primitiveType (static_cast<ECN::PrimitiveType>(0))
    {
    DetermineTypeInfo (propertyMap.GetProperty ());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlTypeInfo::ECSqlTypeInfo (ECSqlTypeInfo const& rhs) 
    : m_kind (rhs.m_kind), m_primitiveType (rhs.m_primitiveType), m_dateTimeInfo (rhs.m_dateTimeInfo), m_structType (rhs.m_structType),
     m_minOccurs (rhs.m_minOccurs), m_maxOccurs (rhs.m_maxOccurs), m_propertyMap (rhs.m_propertyMap)
    {
    }

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
ECSqlTypeInfo::ECSqlTypeInfo (ECSqlTypeInfo&& rhs) 
    : m_kind (std::move (rhs.m_kind)), m_primitiveType (std::move (rhs.m_primitiveType)), m_dateTimeInfo (std::move (rhs.m_dateTimeInfo)), m_structType (std::move (rhs.m_structType)), 
    m_minOccurs (std::move (rhs.m_minOccurs)), m_maxOccurs (std::move (rhs.m_maxOccurs)), m_propertyMap (std::move (rhs.m_propertyMap))
    {
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlTypeInfo& ECSqlTypeInfo::operator= (ECSqlTypeInfo&& rhs)
    {
    if (this != &rhs)
        {
        m_kind = std::move (rhs.m_kind);
        m_primitiveType = std::move (rhs.m_primitiveType);
        m_dateTimeInfo = std::move (rhs.m_dateTimeInfo);
        m_structType = std::move (rhs.m_structType);
        m_minOccurs = std::move (rhs.m_minOccurs);
        m_maxOccurs = std::move (rhs.m_maxOccurs);
        m_propertyMap = std::move (rhs.m_propertyMap);
        }

    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     09/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void ECSqlTypeInfo::Populate (bool isArray, ECN::PrimitiveType const* primitiveType, ECN::ECClassCP structType, uint32_t minOccurs, uint32_t maxOccurs, DateTimeInfo const* dateTimeInfo)
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

    m_minOccurs = minOccurs;
    m_maxOccurs = maxOccurs;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     09/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool ECSqlTypeInfo::Matches (ECSqlTypeInfo const& rhs, Utf8String* errorMessage) const
    {
    const auto lhsKind = GetKind ();
    const auto rhsKind = rhs.GetKind ();
    //if unset or varies, types are not comparable. If one is NULL it is always comparable.
    //In the remaining cases, the kind needs to be the same.
    if (lhsKind == Kind::Unset || rhsKind == Kind::Unset ||
        lhsKind == Kind::Varies || rhsKind == Kind::Varies ||
        (lhsKind != Kind::Null && rhsKind != Kind::Null && lhsKind != rhsKind))
        return false;

    if (lhsKind == Kind::Null || rhsKind == Kind::Null)
        return true;

    //now detail checks
    auto canCompare = false;
    //struct check
    if (lhsKind == Kind::Struct || lhsKind == Kind::StructArray)
        {
        canCompare = GetStructType () == rhs.GetStructType ();
        if (!canCompare && errorMessage != nullptr)
            *errorMessage = "Left and right side of expression must both be of same ECStruct type.";

        return canCompare;
        }

    //primitive or primitive array checks
    const auto lhsType = GetPrimitiveType ();
    const auto rhsType = rhs.GetPrimitiveType ();

    if (lhsType == PRIMITIVETYPE_DateTime || rhsType == PRIMITIVETYPE_DateTime)
        {
        if (lhsType == rhsType)
            {
            canCompare = DateTimeInfoMatches (rhs.GetDateTimeInfo ());
            if (!canCompare && errorMessage != nullptr)
                *errorMessage = "DateTime metadata of left and right side of expression do not match.";

            return canCompare;
            }
        else
            {
            //DateTimes must be on both sides. No implicit conversion
            if (errorMessage != nullptr)
                *errorMessage = "Left and right side of expression must both have DateTime type.";

            return false;
            }
        }

    //Points / IGeometry must be same on both sides, too. For all other cases, do what SQLite does with it
    canCompare = (lhsType != PRIMITIVETYPE_Point2D && rhsType != PRIMITIVETYPE_Point2D &&
            lhsType != PRIMITIVETYPE_Point3D && rhsType != PRIMITIVETYPE_Point3D &&
            lhsType != PRIMITIVETYPE_IGeometry && rhsType != PRIMITIVETYPE_IGeometry) || lhsType == rhsType;

    if (!canCompare && errorMessage != nullptr)
        *errorMessage = "Left and right side of expression must both have Point2D / Point3D / IGeometry type.";

    return canCompare;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     10/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool ECSqlTypeInfo::Equals (ECSqlTypeInfo const& rhs, bool ignoreDateTimeInfo) const
    {
    return m_kind == rhs.m_kind && 
        m_primitiveType == rhs.m_primitiveType && 
        (ignoreDateTimeInfo || m_dateTimeInfo == rhs.m_dateTimeInfo) &&
        m_structType == rhs.m_structType;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlTypeInfo::Kind ECSqlTypeInfo::GetKind () const
    {
    return m_kind;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool ECSqlTypeInfo::IsPrimitive() const
    {
    return m_kind == Kind::Primitive;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool ECSqlTypeInfo::IsNumeric() const
    {
    return IsExactNumeric () || IsApproximateNumeric ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool ECSqlTypeInfo::IsExactNumeric () const
    {
    return IsPrimitive () && (GetPrimitiveType () == ECN::PRIMITIVETYPE_Integer || GetPrimitiveType () == ECN::PRIMITIVETYPE_Long);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool ECSqlTypeInfo::IsApproximateNumeric() const
    {
    return IsPrimitive () && (GetPrimitiveType () == ECN::PRIMITIVETYPE_Double);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     04/2015
//+---------------+---------------+---------------+---------------+---------------+--------
bool ECSqlTypeInfo::IsBoolean() const
    {
    return IsPrimitive() && (GetPrimitiveType() == ECN::PRIMITIVETYPE_Boolean);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     06/2015
//+---------------+---------------+---------------+---------------+---------------+--------
bool ECSqlTypeInfo::IsString() const
    {
    return IsPrimitive() && (GetPrimitiveType() == ECN::PRIMITIVETYPE_String);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     09/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool ECSqlTypeInfo::IsDateTime () const
    {
    return IsPrimitive () && (GetPrimitiveType () == ECN::PRIMITIVETYPE_DateTime);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     06/2015
//+---------------+---------------+---------------+---------------+---------------+--------
bool ECSqlTypeInfo::IsBinary() const
    {
    return IsPrimitive() && (GetPrimitiveType() == ECN::PRIMITIVETYPE_Binary);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool ECSqlTypeInfo::IsPoint() const
    {
    return IsPrimitive () && (GetPrimitiveType () == ECN::PRIMITIVETYPE_Point2D || GetPrimitiveType () == ECN::PRIMITIVETYPE_Point3D);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     11/2014
//+---------------+---------------+---------------+---------------+---------------+--------
bool ECSqlTypeInfo::IsGeometry () const
    {
    return IsPrimitive () && (GetPrimitiveType () == ECN::PRIMITIVETYPE_IGeometry);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool ECSqlTypeInfo::IsStruct() const
    {
    return m_kind == Kind::Struct;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool ECSqlTypeInfo::IsArray() const
    {
    return m_kind == Kind::PrimitiveArray || m_kind == Kind::StructArray;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool ECSqlTypeInfo::DateTimeInfoMatches (DateTimeInfo const& rhs) const
    {
    DateTime::Info rhsMetadata = rhs.GetInfo (true);
    const auto rhsKind = rhsMetadata.GetKind ();
    const auto rhsComponent = rhsMetadata.GetComponent ();
    return DateTimeInfoMatches (rhs.IsKindNull () ? nullptr : &rhsKind,
        rhs.IsComponentNull () ? nullptr : &rhsComponent);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     07/2014
//+---------------+---------------+---------------+---------------+---------------+--------
bool ECSqlTypeInfo::DateTimeInfoMatches (DateTime::Info const* rhs) const
    {
    if (rhs == nullptr)
        return DateTimeInfoMatches (nullptr, nullptr);

    const auto rhsKind = rhs->GetKind ();
    const auto rhsComponent = rhs->GetComponent ();
    return DateTimeInfoMatches (&rhsKind, &rhsComponent);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     07/2014
//+---------------+---------------+---------------+---------------+---------------+--------
bool ECSqlTypeInfo::DateTimeInfoMatches (DateTime::Kind const* rhsKind, DateTime::Component const* rhsComponent) const
    {
    auto const& lhsDtInfo = GetDateTimeInfo ();
    auto lhsRawDtInfo = lhsDtInfo.GetInfo (true);

    //We allow date-only to interact with datetimes of any kind
    if ((!lhsDtInfo.IsComponentNull () && lhsRawDtInfo.GetComponent () == DateTime::Component::Date) ||
        (rhsComponent != nullptr && *rhsComponent == DateTime::Component::Date))
        return true;

    //if kind (or component) is null on one side, the kind (or component) on the other side is ignored
    return (lhsDtInfo.IsKindNull () || rhsKind == nullptr || lhsRawDtInfo.GetKind () == *rhsKind) &&
        (lhsDtInfo.IsComponentNull () || rhsComponent == nullptr || lhsRawDtInfo.GetComponent () == *rhsComponent);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECN::ArrayKind ECSqlTypeInfo::GetArrayKind () const
    {
    return (m_structType != nullptr) ? ECN::ARRAYKIND_Struct : ECN::ARRAYKIND_Primitive;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlTypeInfo::DetermineTypeInfo (ECPropertyCR ecProperty)  
    {
    auto primitiveType = ECN::PRIMITIVETYPE_Integer;
    ECClassCP structType = nullptr;
    const auto isArray = ecProperty.GetIsArray ();

    uint32_t minOccurs = 0;
    uint32_t maxOccurs = 0;
    if (isArray)
        {
        auto arrayProperty = ecProperty.GetAsArrayProperty();
        auto structArrayProperty = ecProperty.GetAsStructArrayProperty();
        if (nullptr != structArrayProperty)
            structType = structArrayProperty->GetStructElementType();
        else
            {
            BeAssert(nullptr != arrayProperty);
            primitiveType = arrayProperty->GetPrimitiveElementType ();
            }

        minOccurs = arrayProperty->GetMinOccurs ();
        maxOccurs = arrayProperty->GetMaxOccurs ();
        }
    //no array
    else if (ecProperty.GetIsStruct())
        structType = &ecProperty.GetAsStructProperty ()->GetType ();
    else
        {
        BeAssert (ecProperty.GetIsPrimitive ());
        primitiveType = ecProperty.GetAsPrimitiveProperty ()->GetType ();
        }

    if (structType != nullptr)
        {
        Populate (isArray, nullptr, structType, minOccurs, maxOccurs, nullptr);
        return;
        }

    if (primitiveType == ECN::PRIMITIVETYPE_DateTime)
        {
        DateTimeInfo dateTimeInfo;
        StandardCustomAttributeHelper::GetDateTimeInfo (dateTimeInfo, ecProperty);
        Populate (isArray, &primitiveType, nullptr, minOccurs, maxOccurs, &dateTimeInfo);
        }
    else
        Populate (isArray, &primitiveType, nullptr, minOccurs, maxOccurs, nullptr);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
