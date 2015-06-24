/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlTypeInfo.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

    void DetermineTypeInfo (ECN::ECPropertyCR ecProperty);

    void Populate (bool isArray, ECN::PrimitiveType const* primitiveType, ECN::ECClassCP structType, uint32_t minOccurs, uint32_t maxOccurs, ECN::DateTimeInfo const* dateTimeInfo);

    bool DateTimeInfoMatches (DateTime::Kind const* rhsKind, DateTime::Component const* rhsComponent) const;

public:
    explicit ECSqlTypeInfo (Kind kind = Kind::Unset);
    explicit ECSqlTypeInfo (ECN::PrimitiveType primitiveType, ECN::DateTimeInfo const* dateTimeInfo = nullptr);
    ECSqlTypeInfo (ECN::PrimitiveType primitiveType, uint32_t minOccurs, uint32_t maxOccurs, ECN::DateTimeInfo const* dateTimeInfo = nullptr);
    explicit ECSqlTypeInfo (ECN::ECClassCR structType);
    ECSqlTypeInfo (ECN::ECClassCR structType, uint32_t minOccurs, uint32_t maxOccurs);
    explicit ECSqlTypeInfo (PropertyMapCR propertyMap);
    explicit ECSqlTypeInfo (ECN::ECPropertyCR ecProperty);

    ~ECSqlTypeInfo () {}

    ECSqlTypeInfo (ECSqlTypeInfo const& rhs);
    ECSqlTypeInfo& operator= (ECSqlTypeInfo const& rhs);
    ECSqlTypeInfo (ECSqlTypeInfo&& rhs);
    ECSqlTypeInfo& operator= (ECSqlTypeInfo&& rhs);

    bool Equals (ECSqlTypeInfo const& rhs, bool ignoreDateTimeInfo = false) const;
    bool Matches (ECSqlTypeInfo const& rhs, Utf8String* errorMessage = nullptr) const;

    Kind GetKind () const;
    bool IsPrimitive () const;
    bool IsNumeric () const;
    bool IsExactNumeric () const;
    bool IsApproximateNumeric () const;
    bool IsBoolean() const;
    bool IsString() const;
    bool IsDateTime () const;
    bool IsBinary() const;
    bool IsPoint() const;
    bool IsGeometry () const;
    bool IsStruct () const;
    bool IsArray () const;

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
    bool DateTimeInfoMatches (ECN::DateTimeInfo const& rhs) const;
    bool DateTimeInfoMatches (DateTime::Info const* rhs) const;

    ECN::PrimitiveType GetPrimitiveType () const {return m_primitiveType;}
    ECN::DateTimeInfo const& GetDateTimeInfo () const { return m_dateTimeInfo; }
    ECN::ECClassCR GetStructType () const {return *m_structType;}
    ECN::ArrayKind GetArrayKind () const;
    uint32_t GetArrayMinOccurs () const { return m_minOccurs; }
    uint32_t GetArrayMaxOccurs () const { return m_maxOccurs; }

    PropertyMapCP GetPropertyMap () const { return m_propertyMap; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE