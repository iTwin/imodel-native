/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECObjectsExtensions.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     06/2015
//+---------------+---------------+---------------+---------------+---------------+--------
ECTypeInfo::ECTypeInfo(ECN::PrimitiveType primitiveType, ECN::DateTimeInfo const* dateTimeInfo /*= nullptr*/)
: m_structType(nullptr), m_minOccurs(0), m_maxOccurs(0)
    {
    if (dateTimeInfo != nullptr)
        m_dateTimeInfo = *dateTimeInfo;

    CreateTypeDescriptor(false, &primitiveType);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECTypeInfo::ECTypeInfo(ECN::PrimitiveType primitiveType, uint32_t minOccurs, uint32_t maxOccurs, DateTimeInfo const* dateTimeInfo /*= nullptr*/)
    : m_structType(nullptr), m_minOccurs(minOccurs), m_maxOccurs(maxOccurs)
    {
    if (dateTimeInfo != nullptr)
        m_dateTimeInfo = *dateTimeInfo;

    CreateTypeDescriptor(true, &primitiveType);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECTypeInfo::ECTypeInfo(ECN::ECClassCR structType)
    : m_structType(&structType), m_minOccurs(0), m_maxOccurs(0);
    {
    CreateTypeDescriptor(false, nullptr);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECTypeInfo::ECTypeInfo(ECN::ECClassCR structType, uint32_t minOccurs, uint32_t maxOccurs)
    : m_structType(&structType), m_minOccurs(minOccurs), m_maxOccurs(maxOccurs)
    {
    CreateTypeDescriptor(true, nullptr);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     04/2014
//+---------------+---------------+---------------+---------------+---------------+--------
ECTypeInfo::ECTypeInfo(ECN::ECPropertyCR ecProperty)
    : m_structType(nullptr), m_minOccurs(0), m_maxOccurs(0)
    {
    DetermineTypeInfo(ecProperty);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECTypeInfo::ECTypeInfo(ECTypeInfo const& rhs)
    : m_typeDescriptor(rhs.m_typeDescriptor), m_dateTimeInfo(rhs.m_dateTimeInfo), m_structType(rhs.m_structType),
    m_minOccurs(rhs.m_minOccurs), m_maxOccurs(rhs.m_maxOccurs)
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECTypeInfo& ECTypeInfo::operator= (ECTypeInfo const& rhs)
    {
    if (this != &rhs)
        {
        m_typeDescriptor = rhs.m_typeDescriptor;
        m_dateTimeInfo = rhs.m_dateTimeInfo;
        m_structType = rhs.m_structType;
        m_minOccurs = rhs.m_minOccurs;
        m_maxOccurs = rhs.m_maxOccurs;
        }

    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECTypeInfo::ECTypeInfo(ECTypeInfo&& rhs)
    : m_typeDescriptor(std::move(rhs.m_typeDescriptor)), m_dateTimeInfo(std::move(rhs.m_dateTimeInfo)), m_structType(std::move(rhs.m_structType)),
    m_minOccurs(std::move(rhs.m_minOccurs)), m_maxOccurs(std::move(rhs.m_maxOccurs))
    {}


//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECTypeInfo& ECTypeInfo::operator= (ECTypeInfo&& rhs)
    {
    if (this != &rhs)
        {
        m_typeDescriptor = std::move(rhs.m_typeDescriptor);
        m_dateTimeInfo = std::move(rhs.m_dateTimeInfo);
        m_structType = std::move(rhs.m_structType);
        m_minOccurs = std::move(rhs.m_minOccurs);
        m_maxOccurs = std::move(rhs.m_maxOccurs);
        }

    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     06/2015
//+---------------+---------------+---------------+---------------+---------------+--------
void ECTypeInfo::CreateTypeDescriptor(bool isArray, ECN::PrimitiveType const* primitiveType)
    {
    if (primitiveType != nullptr)
        {
        if (isArray)
           m_typeDescriptor = ECTypeDescriptor::CreatePrimitiveArrayTypeDescriptor(*primitiveType);
        else
           m_typeDescriptor = ECTypeDescriptor::CreatePrimitiveTypeDescriptor(*primitiveType);
        }
    else
        {
        if (isArray)
            m_typeDescriptor = ECTypeDescriptor::CreateStructArrayTypeDescriptor();
        else
            m_typeDescriptor = ECTypeDescriptor::CreateStructTypeDescriptor();
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ECTypeInfo::DetermineTypeInfo(ECPropertyCR ecProperty)
    {
    const bool isArray = ecProperty.GetIsArray();
    PrimitiveType primitiveType = ECN::PRIMITIVETYPE_Integer;
    bool isPrimitiveOrPrimitiveArray = false;
    if (isArray)
        {
        ArrayECPropertyCP arrayProperty = ecProperty.GetAsArrayProperty();
        if (arrayProperty->GetKind() == ARRAYKIND_Primitive)
            {
            primitiveType = arrayProperty->GetPrimitiveElementType();
            isPrimitiveOrPrimitiveArray = true;
            }
        else
            {
            BeAssert(arrayProperty->GetKind() == ARRAYKIND_Struct);
            m_structType = arrayProperty->GetStructElementType();
            }

        m_minOccurs = arrayProperty->GetMinOccurs();
        m_maxOccurs = arrayProperty->GetMaxOccurs();
        }
    //no array
    else if (ecProperty.GetIsStruct())
        m_structType = &ecProperty.GetAsStructProperty()->GetType();
    else
        {
        BeAssert(ecProperty.GetIsPrimitive());
        primitiveType = ecProperty.GetAsPrimitiveProperty()->GetType();
        isPrimitiveOrPrimitiveArray = true;
        }

    CreateTypeDescriptor(isArray, isPrimitiveOrPrimitiveArray ? &primitiveType : nullptr);

    if (isPrimitiveOrPrimitiveArray && primitiveType == ECN::PRIMITIVETYPE_DateTime)
        StandardCustomAttributeHelper::GetDateTimeInfo(m_dateTimeInfo, ecProperty);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     10/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool ECTypeInfo::Equals(ECTypeInfo const& rhs, bool ignoreDateTimeInfo) const
    {
    return m_typeDescriptor == rhs.m_typeDescriptor &&
        (ignoreDateTimeInfo || m_dateTimeInfo == rhs.m_dateTimeInfo) &&
        m_structType == rhs.m_structType &&
        m_minOccurs == rhs.m_minOccurs &&
        m_maxOccurs == rhs.m_maxOccurs;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
