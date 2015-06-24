/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECObjectsExtensions.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      06/2015
//+===============+===============+===============+===============+===============+======
struct ECTypeInfo
    {
private:
    ECN::ECTypeDescriptor m_typeDescriptor;
    ECN::DateTimeInfo m_dateTimeInfo;
    ECN::ECClassCP m_structType;
    uint32_t m_minOccurs;
    uint32_t m_maxOccurs;

    void CreateTypeDescriptor(bool isArray, ECN::PrimitiveType const* primitiveType);
    void DetermineTypeInfo(ECN::ECPropertyCR ecProperty);

public:
    explicit ECTypeInfo(ECN::PrimitiveType primitiveType, ECN::DateTimeInfo const* dateTimeInfo = nullptr);
    ECTypeInfo(ECN::PrimitiveType primitiveType, uint32_t minOccurs, uint32_t maxOccurs, ECN::DateTimeInfo const* dateTimeInfo = nullptr);
    explicit ECTypeInfo(ECN::ECClassCR structType);
    ECTypeInfo(ECN::ECClassCR structType, uint32_t minOccurs, uint32_t maxOccurs);
    explicit ECTypeInfo(ECN::ECPropertyCR ecProperty);
    virtual ~ECTypeInfo() {}

    ECTypeInfo(ECTypeInfo const& rhs);
    ECTypeInfo& operator= (ECTypeInfo const& rhs);
    ECTypeInfo(ECTypeInfo&& rhs);
    ECTypeInfo& operator= (ECTypeInfo&& rhs);

    bool Equals(ECTypeInfo const& rhs, bool ignoreDateTimeInfo = false) const;

    ECN::ECTypeDescriptor const& GetDescriptor() const { return m_typeDescriptor; }
    bool IsPrimitive() const { return m_typeDescriptor.IsPrimitive(); }
    bool IsStruct() const { return m_typeDescriptor.IsStruct (); }
    bool IsArray() const { return m_typeDescriptor.IsArray (); }

    ECN::PrimitiveType GetPrimitiveType() const { return m_typeDescriptor.GetPrimitiveType (); }
    bool IsExactNumeric() const { return IsPrimitive() && (GetPrimitiveType() == ECN::PRIMITIVETYPE_Integer || GetPrimitiveType() == ECN::PRIMITIVETYPE_Long); }
    bool IsApproximateNumeric() const { return IsPrimitive() && GetPrimitiveType() == ECN::PRIMITIVETYPE_Double; }
    bool IsNumeric() const { return IsExactNumeric() || IsApproximateNumeric(); }
    bool IsBoolean() const { return IsPrimitive() && GetPrimitiveType() == ECN::PRIMITIVETYPE_Boolean; }
    bool IsString() const { return IsPrimitive() && GetPrimitiveType() == ECN::PRIMITIVETYPE_String; }
    bool IsDateTime() const { return IsPrimitive() && GetPrimitiveType() == ECN::PRIMITIVETYPE_DateTime; }
    bool IsBinary() const { return IsPrimitive() && GetPrimitiveType() == ECN::PRIMITIVETYPE_Binary; }
    bool IsPoint() const { return IsPrimitive() && (GetPrimitiveType() == ECN::PRIMITIVETYPE_Point2D || GetPrimitiveType() == ECN::PRIMITIVETYPE_Point3D); }
    bool IsGeometry() const { return IsPrimitive() && GetPrimitiveType() == ECN::PRIMITIVETYPE_IGeometry; }
    ECN::DateTimeInfo const& GetDateTimeInfo() const { return m_dateTimeInfo; }

    ECN::ECClassCR GetStructType() const { BeAssert(IsStruct()); return *m_structType; }

    ECN::ArrayKind GetArrayKind() const { BeAssert(IsArray()); return m_typeDescriptor.GetArrayKind(); }
    uint32_t GetArrayMinOccurs() const { BeAssert(IsArray()); return m_minOccurs; }
    uint32_t GetArrayMaxOccurs() const { BeAssert(IsArray()); return m_maxOccurs; }
    };
END_BENTLEY_SQLITE_EC_NAMESPACE