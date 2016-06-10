/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbInternalTypes.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <ECDb/ECDbTypes.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/BeId.h>
#include <Bentley/BeAssert.h>
#include "ECDbLogger.h"
#include <type_traits>
ECDB_TYPEDEFS_PTR(ClassMap);
ECDB_TYPEDEFS_PTR(PropertyMap);
ECDB_TYPEDEFS(RelationshipClassMap);

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define LOG (ECDbLogger::Get())


//=======================================================================================
//! ECSQL statement types
// @bsienum                                                Krischan.Eberle      12/2013
//+===============+===============+===============+===============+===============+======
enum class ECSqlType
    {
    Select, //!< ECSQL SELECT
    Insert, //!< ECSQL INSERT
    Update, //!< ECSQL UPDATE
    Delete //!< ECSQL DELETE
    };

//*** ECSQL / SQL Operators ***
//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
enum class BinarySqlOperator
    {
    Plus,
    Minus,
    Divide,
    Multiply,
    Modulo,
    ShiftLeft,
    ShiftRight,
    BitwiseOr,
    BitwiseAnd,
    BitwiseXOr,
    Concat
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
enum class BooleanSqlOperator
    {
    EqualTo,
    NotEqualTo,
    LessThan,
    LessThanOrEqualTo,
    GreaterThan,
    GreaterThanOrEqualTo,
    Is,
    IsNot,
    In,
    NotIn,
    Between,
    NotBetween,
    Like,
    NotLike,
    Or,
    And,
    Match,
    NotMatch
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
enum class UnarySqlOperator
    {
    Minus,
    Plus,
    BitwiseNot
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
enum class SqlCompareListType
    {
    All,
    Any,
    Some
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
enum class SqlSetQuantifier
    {
    NotSpecified,
    Distinct,
    All,
    };

typedef BeInt64Id ECContainerId;
typedef BeInt64Id ECRelationshipConstraintId;

#define ECDB_COL_ECInstanceId "ECInstanceId"
#define ECDB_COL_ECClassId "ECClassId"

//=======================================================================================
// @bsiclass                                 Affan.Khan                10/2015
//+===============+===============+===============+===============+===============+======
struct Enum
    {
private:
    Enum();
    ~Enum();

    template<typename TEnum>
    static typename std::underlying_type<TEnum>::type ToUnderlyingType(TEnum val) { return Convert<TEnum, typename std::underlying_type<TEnum>::type>(val); }

    template<typename TEnum>
    static TEnum FromUnderlyingType(typename std::underlying_type<TEnum>::type underlyingType) { return Convert<typename std::underlying_type<TEnum>::type, TEnum>(underlyingType); }

public:
    template<typename TFromEnum, typename TToEnum>
    static TToEnum Convert(TFromEnum val) { return static_cast<TToEnum>(val); }

    template<typename TEnum>
    static int ToInt(TEnum val) { return Convert<TEnum,int>(val); }

    template<typename TEnum>
    static TEnum FromInt(int val) { return Convert<int,TEnum>(val); }
    
    template<typename TEnum>
    static TEnum Or(TEnum lhs, TEnum rhs) { return FromUnderlyingType<TEnum>(ToUnderlyingType<TEnum>(lhs) | ToUnderlyingType<TEnum>(rhs)); }
    
    template<typename TEnum>
    static TEnum And(TEnum lhs, TEnum rhs) { return FromUnderlyingType<TEnum>(ToUnderlyingType<TEnum>(lhs) & ToUnderlyingType<TEnum>(rhs)); }
        
    template<typename TEnum>
    static bool Contains(TEnum test, TEnum candidate) { return Enum::And(test, candidate) == candidate; }

    template<typename TEnum>
    static bool Intersects(TEnum lhs, TEnum rhs) { return (ToInt<TEnum>(lhs) & ToInt<TEnum>(rhs)) != 0; }
    };

//=======================================================================================
// For case-sensitive UTF-8 string comparisons in STL collections.
// @bsistruct
//+===============+===============+===============+===============+===============+======
struct CompareUtf8
    {
    bool operator()(Utf8CP s1, Utf8CP s2) const { return strcmp(s1, s2) < 0;}
    };


//=======================================================================================
// For case-insensitive UTF-8 string comparisons in STL collections that only use ASCII
// strings
// @bsistruct
//+===============+===============+===============+===============+===============+======
struct CompareIUtf8Ascii
    {
    bool operator()(Utf8CP s1, Utf8CP s2) const { return BeStringUtilities::StricmpAscii(s1, s2) < 0; }
    bool operator()(Utf8StringCR s1, Utf8StringCR s2) const {  return BeStringUtilities::StricmpAscii(s1.c_str(), s2.c_str()) < 0;  }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
