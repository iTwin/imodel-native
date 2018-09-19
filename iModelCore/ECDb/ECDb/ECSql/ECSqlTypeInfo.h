/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlTypeInfo.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "../ECDbInternalTypes.h"
#include <Bentley/Nullable.h>

#define EXTENDEDTYPENAME_Id "Id"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      08/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlTypeInfo final
    {
    public:
        enum class Kind
            {
            Unset, //!< type info not set yet
            Varies, //!< Type info not available as it may vary for each child expression. Therefore look at type infos of each child.
            Null, //!< Expression is the NULL token
            Primitive, //!< Primitive type
            Struct, //!< Struct type
            PrimitiveArray, //!< Primitive array type
            StructArray, //!< Struct array type
            Navigation //!< Navigation property
            };
    private:
        Kind m_kind;
        Nullable<ECN::PrimitiveType> m_primitiveType;
        Nullable<DateTime::Info> m_dateTimeInfo;
        ECN::ECEnumerationCP m_enumType = nullptr;
        Utf8String m_extendedTypeName;
        ECN::ECStructClassCP m_structType = nullptr;
        Nullable<uint32_t> m_minOccurs;
        Nullable<uint32_t> m_maxOccurs;
        PropertyMap const* m_propertyMap = nullptr;

        ECSqlTypeInfo(ECN::PrimitiveType primitiveType, bool isArray, Nullable<DateTime::Info> dateTimeInfo, ECN::ECEnumerationCP enumType, Utf8CP extendedTypeName)
            : m_kind(isArray ? Kind::PrimitiveArray : Kind::Primitive), m_primitiveType(primitiveType), m_dateTimeInfo(dateTimeInfo), m_enumType(enumType), m_extendedTypeName(extendedTypeName)
            {
            //for date time always assign a date time info
            if (m_primitiveType != nullptr && m_primitiveType.Value() == ECN::PRIMITIVETYPE_DateTime && m_dateTimeInfo == nullptr)
                m_dateTimeInfo = DateTime::Info();
            }

        ECSqlTypeInfo(ECN::ECStructClassCR structType, bool isArray) : m_kind(isArray ? Kind::StructArray : Kind::Struct), m_structType(&structType) {}

        void DetermineTypeInfo(ECN::ECPropertyCR ecProperty);

    public:
        explicit ECSqlTypeInfo(Kind kind = Kind::Unset) : m_kind(kind) {}
        
        static ECSqlTypeInfo CreatePrimitive(ECN::PrimitiveType primitiveType, bool isArray = false, Utf8CP extendedTypeName = nullptr) { return ECSqlTypeInfo(primitiveType, isArray, nullptr, nullptr, extendedTypeName); }
        static ECSqlTypeInfo CreateEnum(ECN::ECEnumerationCR ecEnum, bool isArray = false, Utf8CP extendedTypeName = nullptr) { return ECSqlTypeInfo(ecEnum.GetType(), isArray, nullptr, &ecEnum, extendedTypeName); }
        static ECSqlTypeInfo CreateDateTime(DateTime::Info const& info, bool isArray = false, Utf8CP extendedTypeName = nullptr) { return ECSqlTypeInfo(ECN::PrimitiveType::PRIMITIVETYPE_DateTime, isArray, info, nullptr, extendedTypeName); }
        static ECSqlTypeInfo CreateStruct(ECN::ECStructClassCR structType, bool isArray = false) { return ECSqlTypeInfo(structType, isArray); }

        static ECSqlTypeInfo CreateArrayElement(ECSqlTypeInfo const& arrayInfo) 
            {
            BeAssert(arrayInfo.IsArray());
            if (arrayInfo.GetKind() == Kind::PrimitiveArray)
                return ECSqlTypeInfo(arrayInfo.GetPrimitiveType(), false, arrayInfo.m_dateTimeInfo, arrayInfo.m_enumType, arrayInfo.m_extendedTypeName.c_str());

            return CreateStruct(arrayInfo.GetStructType());
            }

        explicit ECSqlTypeInfo(PropertyMap const& propertyMap) : m_propertyMap(&propertyMap) { DetermineTypeInfo(propertyMap.GetProperty()); }
        explicit ECSqlTypeInfo(ECN::ECPropertyCR ecProperty) { DetermineTypeInfo(ecProperty); }

        ~ECSqlTypeInfo() {}

        ECSqlTypeInfo(ECSqlTypeInfo const& rhs) 
            : m_kind(rhs.m_kind), m_primitiveType(rhs.m_primitiveType), m_dateTimeInfo(rhs.m_dateTimeInfo), m_enumType(rhs.m_enumType), m_extendedTypeName(rhs.m_extendedTypeName), m_structType(rhs.m_structType),
            m_minOccurs(rhs.m_minOccurs), m_maxOccurs(rhs.m_maxOccurs), m_propertyMap(rhs.m_propertyMap) {}
        ECSqlTypeInfo& operator=(ECSqlTypeInfo const&);

        ECSqlTypeInfo(ECSqlTypeInfo&& rhs)
            : m_kind(std::move(rhs.m_kind)), m_primitiveType(std::move(rhs.m_primitiveType)), m_dateTimeInfo(std::move(rhs.m_dateTimeInfo)), m_enumType(std::move(rhs.m_enumType)), m_extendedTypeName(std::move(rhs.m_extendedTypeName)), m_structType(std::move(rhs.m_structType)),
            m_minOccurs(std::move(rhs.m_minOccurs)), m_maxOccurs(std::move(rhs.m_maxOccurs)), m_propertyMap(std::move(rhs.m_propertyMap)) {}

        ECSqlTypeInfo& operator=(ECSqlTypeInfo&&);

        bool operator==(ECSqlTypeInfo const& rhs) const { return m_kind == rhs.m_kind && m_primitiveType == rhs.m_primitiveType && m_dateTimeInfo == rhs.m_dateTimeInfo && m_enumType == rhs.m_enumType && m_extendedTypeName == rhs.m_extendedTypeName && m_structType == rhs.m_structType && m_minOccurs == rhs.m_minOccurs && m_maxOccurs == rhs.m_maxOccurs; }
        bool operator!=(ECSqlTypeInfo const& rhs) const { return !(*this == rhs); }

        //! Compares the two ECSqlTypeInfo for compatibility in ECSQL.
        bool CanCompare(ECSqlTypeInfo const& rhs, Utf8String* errorMessage = nullptr) const;

        Kind GetKind() const { return m_kind; }
        bool IsNull() const { return m_kind == Kind::Null; }
        bool IsPrimitive() const { return m_kind == Kind::Primitive; }
        bool IsNumeric() const { return IsExactNumeric() || IsApproximateNumeric(); }
        bool IsExactNumeric() const { return IsPrimitive() && (m_primitiveType == ECN::PRIMITIVETYPE_Integer || m_primitiveType == ECN::PRIMITIVETYPE_Long); }
        bool IsApproximateNumeric() const { return IsPrimitive() && (m_primitiveType == ECN::PRIMITIVETYPE_Double); }
        bool IsEnum() const { return IsPrimitive() && m_enumType != nullptr; }
        bool IsBoolean() const { return IsPrimitive() && (m_primitiveType == ECN::PRIMITIVETYPE_Boolean); }
        bool IsString() const { return IsPrimitive() && (m_primitiveType == ECN::PRIMITIVETYPE_String); }
        bool IsDateTime() const { return IsPrimitive() && (m_primitiveType == ECN::PRIMITIVETYPE_DateTime); }
        bool IsBinary() const { return IsPrimitive() && (m_primitiveType == ECN::PRIMITIVETYPE_Binary); }
        bool IsPoint() const { return IsPrimitive() && (m_primitiveType == ECN::PRIMITIVETYPE_Point2d || m_primitiveType == ECN::PRIMITIVETYPE_Point3d); }
        bool IsGeometry() const { return IsPrimitive() && (m_primitiveType == ECN::PRIMITIVETYPE_IGeometry); }
        bool IsStruct() const { return m_kind == Kind::Struct; }
        bool IsArray() const { return m_kind == Kind::PrimitiveArray || m_kind == Kind::StructArray; }
        bool IsNavigation() const { return m_kind == Kind::Navigation; }

        bool DateTimeInfoMatches(DateTime::Info const& rhs) const;

        ECN::PrimitiveType GetPrimitiveType() const 
            {
            if (m_primitiveType == nullptr)
                BeAssert(IsPrimitive() || m_kind == Kind::PrimitiveArray);
                
            return m_primitiveType.Value(); 
            }
        DateTime::Info const& GetDateTimeInfo() const { BeAssert(GetPrimitiveType() == ECN::PRIMITIVETYPE_DateTime); return m_dateTimeInfo.Value(); }
        ECN::ECEnumerationCP GetEnumerationType() const { BeAssert(IsPrimitive() || m_kind == Kind::PrimitiveArray); return m_enumType; }
        bool HasExtendedType() const { return !m_extendedTypeName.empty(); }
        Utf8StringCR GetExtendedTypeName() const { BeAssert(IsPrimitive() || m_kind == Kind::PrimitiveArray); return m_extendedTypeName; }
        ECN::ECStructClassCR GetStructType() const { BeAssert(IsStruct() || m_kind == Kind::StructArray); return *m_structType; }

        Nullable<uint32_t> GetArrayMinOccurs() const { BeAssert(IsArray()); return m_minOccurs; }
        Nullable<uint32_t> GetArrayMaxOccurs() const { BeAssert(IsArray()); return m_maxOccurs; }

        PropertyMap const* GetPropertyMap() const { return m_propertyMap; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE