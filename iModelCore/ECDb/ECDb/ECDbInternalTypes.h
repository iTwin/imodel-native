/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDbTypes.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/BeId.h>
#include <Bentley/BeAssert.h>
#include "ECDbLogger.h"
#include <type_traits>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define LOG (ECDbLogger::Get())


//=======================================================================================
//! ECSQL statement types
// @bsiclass
//+===============+===============+===============+===============+===============+======
enum class ECSqlType
    {
    Select,
    Insert,
    Update,
    Delete,
    Pragma,
    };

typedef BeInt64Id ECContainerId;
typedef BeInt64Id ECRelationshipConstraintId;

//=======================================================================================
// @bsiclass
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



///=======================================================================================
// FNVa1 hash
// @bsistruct
//+===============+===============+===============+===============+===============+======
struct FNV1HashBuilder final {
    private:
        uint64_t m_hashCode;
    public:
        FNV1HashBuilder(): m_hashCode(14695981039346656037ull){}
        void UpdateBytes(uint8_t const* bytes, size_t sz) {
            if (sz == 0 || bytes == nullptr) return;
            for (size_t i = 0; i< sz; ++i) {
                m_hashCode ^= (uint64_t)bytes[i];
                m_hashCode *= 1099511628211u;
            }
        }
        void UpdateString(Utf8StringCR str) {UpdateBytes((uint8_t const*)str.c_str(), str.length());}
        void UpdateCharCP(Utf8CP str) {UpdateBytes((uint8_t const*)str, strlen(str));}
        void UpdateUInt64(uint64_t v) {UpdateBytes((uint8_t const*)&v, sizeof(v));}
        void UpdateInt64(int64_t v) {UpdateBytes((uint8_t const*)&v, sizeof(v));}
        void UpdateUInt32(uint32_t v) {UpdateBytes((uint8_t const*)&v, sizeof(v));}
        void UpdateInt32(int32_t v) {UpdateBytes((uint8_t const*)&v, sizeof(v));}
        void UpdateUInt16(uint16_t v) {UpdateBytes((uint8_t const*)&v, sizeof(v));}
        void UpdateInt16(int16_t v) {UpdateBytes((uint8_t const*)&v, sizeof(v));}
        void UpdateUInt8(uint8_t v) {UpdateBytes((uint8_t const*)&v, sizeof(v));}
        void UpdateInt8(int8_t v) {UpdateBytes((uint8_t const*)&v, sizeof(v));}
        void UpdateDouble(double v) {UpdateBytes((uint8_t const*)&v, sizeof(v));}
        void UpdateInt(int v) {UpdateBytes((uint8_t const*)&v, sizeof(v));}
        void UpdateFloat(float v) {UpdateBytes((uint8_t const*)&v, sizeof(v));}
        void UpdateBool(bool v) {UpdateBytes((uint8_t const*)&v, sizeof(v));}
        void UpdateChar(char v) {UpdateBytes((uint8_t const*)&v, sizeof(v));}
        uint64_t GetHashCode() const { return m_hashCode;}
};
END_BENTLEY_SQLITE_EC_NAMESPACE
