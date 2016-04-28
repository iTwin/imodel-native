/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlTypeInfo.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "../ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      08/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlTypeInfo
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
            StructArray //!< Struct array type
            };
    private:
        Kind m_kind;
        ECN::PrimitiveType m_primitiveType;
        ECN::DateTimeInfo m_dateTimeInfo;
        ECN::ECClassCP m_structType;
        uint32_t m_minOccurs;
        uint32_t m_maxOccurs;
        PropertyMapCP m_propertyMap;

        void DetermineTypeInfo(ECN::ECPropertyCR ecProperty);

        void Populate(bool isArray, ECN::PrimitiveType const* primitiveType, ECN::ECClassCP structType, uint32_t minOccurs, uint32_t maxOccurs, ECN::DateTimeInfo const* dateTimeInfo);

        bool DateTimeInfoMatches(DateTime::Kind const* rhsKind, DateTime::Component const* rhsComponent) const;

    public:
        explicit ECSqlTypeInfo(Kind kind = Kind::Unset);
        explicit ECSqlTypeInfo(ECN::PrimitiveType primitiveType, ECN::DateTimeInfo const* dateTimeInfo = nullptr);
        ECSqlTypeInfo(ECN::PrimitiveType primitiveType, uint32_t minOccurs, uint32_t maxOccurs, ECN::DateTimeInfo const* dateTimeInfo = nullptr);
        explicit ECSqlTypeInfo(ECN::ECClassCR structType);
        ECSqlTypeInfo(ECN::ECClassCR structType, uint32_t minOccurs, uint32_t maxOccurs);
        explicit ECSqlTypeInfo(PropertyMapCR propertyMap);
        explicit ECSqlTypeInfo(ECN::ECPropertyCR ecProperty);

        ~ECSqlTypeInfo() {}

        ECSqlTypeInfo(ECSqlTypeInfo const& rhs);
        ECSqlTypeInfo& operator=(ECSqlTypeInfo const& rhs);
        ECSqlTypeInfo(ECSqlTypeInfo&& rhs);
        ECSqlTypeInfo& operator=(ECSqlTypeInfo&& rhs);

        //! Compares the two ECSqlTypeInfo for exact equality, i.e. GetKind, GetPrimitiveType and GetStructType must match.
        bool Equals(ECSqlTypeInfo const& rhs) const;
        //! Compares the two ECSqlTypeInfo for comparibility in ECSQL.
        bool CanCompare(ECSqlTypeInfo const& rhs, Utf8String* errorMessage = nullptr) const;

        Kind GetKind() const { return m_kind; }
        bool IsPrimitive() const { return m_kind == Kind::Primitive; }
        bool IsNumeric() const { return IsExactNumeric() || IsApproximateNumeric(); }
        bool IsExactNumeric() const { return IsPrimitive() && (m_primitiveType == ECN::PRIMITIVETYPE_Integer || m_primitiveType == ECN::PRIMITIVETYPE_Long); }
        bool IsApproximateNumeric() const { return IsPrimitive() && (m_primitiveType == ECN::PRIMITIVETYPE_Double); }
        bool IsBoolean() const { return IsPrimitive() && (m_primitiveType == ECN::PRIMITIVETYPE_Boolean); }
        bool IsString() const { return IsPrimitive() && (m_primitiveType == ECN::PRIMITIVETYPE_String); }
        bool IsDateTime() const { return IsPrimitive() && (m_primitiveType == ECN::PRIMITIVETYPE_DateTime); }
        bool IsBinary() const { return IsPrimitive() && (m_primitiveType == ECN::PRIMITIVETYPE_Binary); }
        bool IsPoint() const { return IsPrimitive() && (m_primitiveType == ECN::PRIMITIVETYPE_Point2D || m_primitiveType == ECN::PRIMITIVETYPE_Point3D); }
        bool IsGeometry() const { return IsPrimitive() && (m_primitiveType == ECN::PRIMITIVETYPE_IGeometry); }
        bool IsStruct() const { return m_kind == Kind::Struct; }
        bool IsArray() const { return m_kind == Kind::PrimitiveArray || m_kind == Kind::StructArray; }


        //! Checks whether @p rhs matches the DateTimeInfo of this object.
        //! @remarks This is how matching is defined:
        //!         * if one of the sides is a date-only (DateTime::Component::Date). In this
        //!            case the time component of the date-only is interpreted as midnight and it uses the same date time kind.
        //!         * if kind / component of one side is null, the kind / component matches.
        //!         * if kind / component is not null on both sides, they match if they are equal
        //!         @e Examples:
        //!         * Dt(Kind::Utc) op Dt(KindIsNull) -> matches
        //!         * Dt(Kind::Utc) op Dt(Kind::Unspecified) -> no match
        //!         * Dt(Kind::Unspecified) op Dt(KindIsNull) -> matches
        //!         * Dt(Component::Date) op Dt(any kind) -> matches
        //!
        //! @param[in] rhs DateTimeInfo object to check against this object
        //! @return true if DateTimeInfo matches. false otherwise
        bool DateTimeInfoMatches(ECN::DateTimeInfo const& rhs) const;
        bool DateTimeInfoMatches(DateTime::Info const* rhs) const;

        ECN::PrimitiveType GetPrimitiveType() const { return m_primitiveType; }
        ECN::DateTimeInfo const& GetDateTimeInfo() const { return m_dateTimeInfo; }
        ECN::ECClassCR GetStructType() const { return *m_structType; }
        ECN::ArrayKind GetArrayKind() const { return (m_structType != nullptr) ? ECN::ARRAYKIND_Struct : ECN::ARRAYKIND_Primitive; }

        uint32_t GetArrayMinOccurs() const { return m_minOccurs; }
        uint32_t GetArrayMaxOccurs() const { return m_maxOccurs; }

        PropertyMapCP GetPropertyMap() const { return m_propertyMap; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE