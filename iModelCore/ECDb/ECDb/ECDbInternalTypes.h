/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbInternalTypes.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define LOG (ECDbLogger::Get())

#define CHANGESUMMARY_CLASS_Instance "Instance"
#define CHANGESUMMARY_CLASS_PropertyValue "PropertyValue"
#define CHANGESUMMARY_CLASS_Summary "Summary"
#define CHANGESUMMARY_SCHEMA_Name "ECDbChangeSummaries"

#define CHANGESUMMARY_PROP_Operation "Operation"
#define CHANGESUMMARY_PROP_IdOfChangedInstance "IdOfChangedInstance"
#define CHANGESUMMARY_PROP_ClassIdOfChangedInstance "ClassIdOfChangedInstance"
#define CHANGESUMMARY_PROP_SummaryId "Summary.Id"

#define CHANGESUMMARY_FUNC_Name "changesummary"
#define CHANGESUMMARY_FUNC_ARG_Operation "operation"
#define CHANGESUMMARY_FUNC_ARG_SummaryId "summary_id"
#define CHANGESUMMARY_FUNC_ARG_Stage "stage"
#define CHANGESUMMARY_ChangedValue "ChangedValue"
//=======================================================================================
//! ECSQL statement types
// @bsienum                                                Krischan.Eberle      12/2013
//+===============+===============+===============+===============+===============+======
enum class ECSqlType
    {
    Select,
    Insert,
    Update,
    Delete
    };


typedef BeInt64Id ECContainerId;
typedef BeInt64Id ECRelationshipConstraintId;

//=======================================================================================
// @bsiclass                                 Affan.Khan                10/2015
//+===============+===============+===============+===============+===============+======
struct Enum final
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
// For case-insensitive UTF-8 string comparisons in STL collections that only use ASCII
// strings
// @bsistruct
//+===============+===============+===============+===============+===============+======
struct CompareIUtf8Ascii
    {
    bool operator()(Utf8CP s1, Utf8CP s2) const { return BeStringUtilities::StricmpAscii(s1, s2) < 0; }
    bool operator()(Utf8StringCR s1, Utf8StringCR s2) const { return BeStringUtilities::StricmpAscii(s1.c_str(), s2.c_str()) < 0;  }
    bool operator()(Utf8StringCP s1, Utf8StringCP s2) const { BeAssert(s1 != nullptr && s2 != nullptr); return BeStringUtilities::StricmpAscii(s1->c_str(), s2->c_str()) < 0; }
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
