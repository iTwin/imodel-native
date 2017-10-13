/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlTypeInfo.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "../ECDbInternalTypes.h"

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
        ECN::PrimitiveType m_primitiveType;
        DateTime::Info m_dateTimeInfo;
        ECN::ECStructClassCP m_structType;
        uint32_t m_minOccurs;
        uint32_t m_maxOccurs;
        PropertyMap const* m_propertyMap;

        void DetermineTypeInfo(ECN::ECPropertyCR ecProperty);

        void Populate(bool isArray, ECN::PrimitiveType const* primitiveType, ECN::ECStructClassCP, int minOccurs, int maxOccurs, DateTime::Info const* dateTimeInfo);

    public:
        explicit ECSqlTypeInfo(Kind kind = Kind::Unset);
        explicit ECSqlTypeInfo(ECN::PrimitiveType primitiveType) : ECSqlTypeInfo(primitiveType, false, nullptr) {}
        ECSqlTypeInfo(ECN::PrimitiveType primitiveType, bool isArray, DateTime::Info const* dateTimeInfo);
        explicit ECSqlTypeInfo(ECN::ECStructClassCR structType) : ECSqlTypeInfo(structType, false) {}
        ECSqlTypeInfo(ECN::ECStructClassCR, bool isArray);
        explicit ECSqlTypeInfo(PropertyMap const& propertyMap);
        explicit ECSqlTypeInfo(ECN::ECPropertyCR ecProperty);

        ~ECSqlTypeInfo() {}

        ECSqlTypeInfo(ECSqlTypeInfo const& rhs);
        ECSqlTypeInfo& operator=(ECSqlTypeInfo const& rhs);
        ECSqlTypeInfo(ECSqlTypeInfo&& rhs);
        ECSqlTypeInfo& operator=(ECSqlTypeInfo&& rhs);

        //! Compares the two ECSqlTypeInfo for exact equality, i.e. GetKind, GetPrimitiveType and GetStructType must match.
        bool Equals(ECSqlTypeInfo const& rhs) const { return m_kind == rhs.m_kind && m_primitiveType == rhs.m_primitiveType && m_structType == rhs.m_structType; }
        //! Compares the two ECSqlTypeInfo for compatibility in ECSQL.
        bool CanCompare(ECSqlTypeInfo const& rhs, Utf8String* errorMessage = nullptr) const;

        Kind GetKind() const { return m_kind; }
        bool IsNull() const { return m_kind == Kind::Null; }
        bool IsPrimitive() const { return m_kind == Kind::Primitive; }
        bool IsNumeric() const { return IsExactNumeric() || IsApproximateNumeric(); }
        bool IsExactNumeric() const { return IsPrimitive() && (m_primitiveType == ECN::PRIMITIVETYPE_Integer || m_primitiveType == ECN::PRIMITIVETYPE_Long); }
        bool IsApproximateNumeric() const { return IsPrimitive() && (m_primitiveType == ECN::PRIMITIVETYPE_Double); }
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

        ECN::PrimitiveType GetPrimitiveType() const { return m_primitiveType; }
        DateTime::Info const& GetDateTimeInfo() const { return m_dateTimeInfo; }
        ECN::ECStructClassCR GetStructType() const { return *m_structType; }
        ECN::ArrayKind GetArrayKind() const { return (m_structType != nullptr) ? ECN::ARRAYKIND_Struct : ECN::ARRAYKIND_Primitive; }

        uint32_t GetArrayMinOccurs() const { return m_minOccurs; }
        uint32_t GetArrayMaxOccurs() const { return m_maxOccurs; }

        PropertyMap const* GetPropertyMap() const { return m_propertyMap; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE