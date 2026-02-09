/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeAssert.h>
#include <Bentley/BeId.h>
#include <Bentley/BeStringUtilities.h>
#include <ECDb/ECDbTypes.h>

#include <chrono>
#include <set>
#include <type_traits>
#include <unordered_map>

#include "ECDbLogger.h"
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define LOG (ECDbLogger::Get())
//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct GeomBlobHeader {
    enum { Signature = 0x0600,
    };                     // DgnDb06
    uint32_t m_signature;  // write this so we can detect errors on read
    uint32_t m_size;
    GeomBlobHeader(ByteStream const& geom) {
        m_signature = Signature;
        m_size = geom.GetSize();
    }
    GeomBlobHeader(SnappyReader& in) {
        uint32_t actuallyRead;
        in._Read((Byte*)this, sizeof(*this), actuallyRead);
    }
    static BentleyStatus Decompress(void const* pData, uint32_t nSize, SnappyFromMemory& reader, ByteStreamR out) {
        if (nSize == 0 || pData == nullptr)
            return SUCCESS;

        reader.Init(const_cast<void*>(pData), nSize);
        GeomBlobHeader header(reader);
        if ((GeomBlobHeader::Signature != header.m_signature) || 0 == header.m_size) {
            BeAssert(false);
            return ERROR;
        }

        out.Resize(header.m_size);
        uint32_t actuallyRead;
        auto readStatus = reader._Read(out.GetDataP(), out.GetSize(), actuallyRead);
        if (ZIP_SUCCESS != readStatus || actuallyRead != out.GetSize()) {
            BeAssert(false);
            return ERROR;
        }
        return SUCCESS;
    }
    static BentleyStatus Decompress(const IECSqlValue& val, SnappyFromMemory& reader, ByteStreamR out) {
        auto const& colInfo = val.GetColumnInfo().GetDataType();
        if (!colInfo.IsPrimitive() || colInfo.GetPrimitiveType() != ECN::PrimitiveType::PRIMITIVETYPE_Binary) {
            BeAssert(false);
            return ERROR;
        }

        if (val.IsNull() && colInfo.IsPrimitive()) {
            return SUCCESS;
        }

        int nSize = 0;
        const void* pData = val.GetBlob(&nSize);
        return Decompress(pData, nSize, reader, out);
    }
    static BentleyStatus Decompress(ByteStreamCR in, SnappyFromMemory& reader, ByteStreamR out) {
        return Decompress(in.GetDataP(), in.GetSize(), reader, out);
    }
    static BentleyStatus Compress(void const* pData, uint32_t nSize, SnappyToBlob& writer, ByteStreamR out) {
        if (nSize == 0 || pData == nullptr)
            return SUCCESS;

        writer.Init();
        GeomBlobHeader header(out);
        writer.Write((Byte const*)&header, sizeof(header));
        writer.Write((Byte const*)pData, nSize);
        writer.SaveTo(out);
        return SUCCESS;
    }
    static BentleyStatus Compress(void const* pData, uint32_t nSize, SnappyToBlob& writer, IECSqlBinder& binder) {
        ByteStream bs;
        auto status = Compress(pData, nSize, writer, bs);
        if (status != SUCCESS)
            return status;
        auto rc = binder.BindBlob((const void*)bs.GetData(), (int)bs.GetSize(), IECSqlBinder::MakeCopy::Yes);
        return rc == ECSqlStatus::Success ? SUCCESS : ERROR;
    }
    static BentleyStatus Compress(ByteStreamCR in, SnappyToBlob& writer, ByteStreamR out) {
        return Compress(in.GetDataP(), in.GetSize(), writer, out);
    }
};

//=======================================================================================
//! ECSQL statement types
// @bsiclass
//+===============+===============+===============+===============+===============+======
enum class ECSqlType {
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
struct Enum final {
   private:
    Enum();
    ~Enum();

    template <typename TEnum>
    static typename std::underlying_type<TEnum>::type ToUnderlyingType(TEnum val) { return Convert<TEnum, typename std::underlying_type<TEnum>::type>(val); }

    template <typename TEnum>
    static TEnum FromUnderlyingType(typename std::underlying_type<TEnum>::type underlyingType) { return Convert<typename std::underlying_type<TEnum>::type, TEnum>(underlyingType); }

   public:
    template <typename TFromEnum, typename TToEnum>
    static TToEnum Convert(TFromEnum val) { return static_cast<TToEnum>(val); }

    template <typename TEnum>
    static int ToInt(TEnum val) { return Convert<TEnum, int>(val); }

    template <typename TEnum>
    static TEnum FromInt(int val) { return Convert<int, TEnum>(val); }

    template <typename TEnum>
    static TEnum Or(TEnum lhs, TEnum rhs) { return FromUnderlyingType<TEnum>(ToUnderlyingType<TEnum>(lhs) | ToUnderlyingType<TEnum>(rhs)); }

    template <typename TEnum>
    static TEnum And(TEnum lhs, TEnum rhs) { return FromUnderlyingType<TEnum>(ToUnderlyingType<TEnum>(lhs) & ToUnderlyingType<TEnum>(rhs)); }

    template <typename TEnum>
    static bool Contains(TEnum test, TEnum candidate) { return Enum::And(test, candidate) == candidate; }

    template <typename TEnum>
    static bool Intersects(TEnum lhs, TEnum rhs) { return (ToInt<TEnum>(lhs) & ToInt<TEnum>(rhs)) != 0; }
};

///=======================================================================================
// FNVa1 hash
// @bsistruct
//+===============+===============+===============+===============+===============+======
struct FNV1HashBuilder final {
   private:
    uint64_t m_hashCode;
    static char inline AsciiTolower(char c) { return ((c >= 'A') && (c <= 'Z')) ? (c - 'A' + 'a') : c; }

   public:
    FNV1HashBuilder() : m_hashCode(14695981039346656037ull) {}
    void UpdateBytes(uint8_t const* bytes, size_t sz) {
        if (sz == 0 || bytes == nullptr)
            return;
        for (size_t i = 0; i < sz; ++i) {
            m_hashCode ^= (uint64_t)bytes[i];
            m_hashCode *= 1099511628211u;
        }
    }
    void UpdateNoCaseAsciiChars(char const* bytes, size_t sz) {
        for (size_t i = 0; i < sz; ++i) {
            UpdateChar(AsciiTolower(bytes[i]));
        }
    }
    void UpdateNoCaseAsciiCharCP(char const* bytes) {
        size_t i = 0;
        while (bytes[i] != 0) {
            UpdateChar(AsciiTolower(bytes[i]));
            ++i;
        }
    }
    void Reset() { m_hashCode = 14695981039346656037ull; }
    void UpdateString(Utf8StringCR str) { UpdateBytes((uint8_t const*)str.c_str(), str.length()); }
    void UpdateCharCP(Utf8CP str) { UpdateBytes((uint8_t const*)str, strlen(str)); }
    void UpdateCharCP(Utf8CP str, size_t sz) { UpdateBytes((uint8_t const*)str, sz); }
    void UpdateNoCaseAsciiString(Utf8StringCR str) { UpdateNoCaseAsciiCharCP(str.c_str(), str.length()); }
    void UpdateNoCaseAsciiCharCP(Utf8CP str, size_t sz) { UpdateNoCaseAsciiChars(str, sz); }
    void UpdateUInt64(uint64_t v) { UpdateBytes((uint8_t const*)&v, sizeof(v)); }
    void UpdateInt64(int64_t v) { UpdateBytes((uint8_t const*)&v, sizeof(v)); }
    void UpdateUInt32(uint32_t v) { UpdateBytes((uint8_t const*)&v, sizeof(v)); }
    void UpdateInt32(int32_t v) { UpdateBytes((uint8_t const*)&v, sizeof(v)); }
    void UpdateUInt16(uint16_t v) { UpdateBytes((uint8_t const*)&v, sizeof(v)); }
    void UpdateInt16(int16_t v) { UpdateBytes((uint8_t const*)&v, sizeof(v)); }
    void UpdateUInt8(uint8_t v) { UpdateBytes((uint8_t const*)&v, sizeof(v)); }
    void UpdateInt8(int8_t v) { UpdateBytes((uint8_t const*)&v, sizeof(v)); }
    void UpdateDouble(double v) { UpdateBytes((uint8_t const*)&v, sizeof(v)); }
    void UpdateInt(int v) { UpdateBytes((uint8_t const*)&v, sizeof(v)); }
    void UpdateFloat(float v) { UpdateBytes((uint8_t const*)&v, sizeof(v)); }
    void UpdateBool(bool v) { UpdateBytes((uint8_t const*)&v, sizeof(v)); }
    void UpdateChar(char v) { UpdateBytes((uint8_t const*)&v, sizeof(v)); }
    uint64_t GetHashCode() const { return m_hashCode; }
};

///=======================================================================================
// Use with std::unordered_map/unordered_set
// @bsistruct
//+===============+===============+===============+===============+===============+======
struct NoCaseAsciiStrHash final {
    size_t operator()(const Utf8String& val) const {
        FNV1HashBuilder builder;
        builder.UpdateNoCaseAsciiString(val);
        return static_cast<size_t>(builder.GetHashCode());
    }
    size_t operator()(Utf8CP val) const {
        FNV1HashBuilder builder;
        builder.UpdateNoCaseAsciiCharCP(val);
        return static_cast<size_t>(builder.GetHashCode());
    }
};
///=======================================================================================
// Use with std::unordered_map/unordered_set
// @bsistruct
//+===============+===============+===============+===============+===============+======
struct NoCaseAsciiStrEqual final {
    bool operator()(const Utf8String& lhs, const Utf8String& rhs) const {
        return lhs.size() == rhs.size() && lhs.EqualsIAscii(rhs);
    }
    bool operator()(Utf8CP lhs, Utf8CP rhs) const {
        return BeStringUtilities::StricmpAscii(lhs, rhs) == 0;
    }
};

//---------------------------------------------------------------------------------------
// @bsistruct
//---------------------------------------------------------------------------------------
struct InMemoryPropertyExistMap final {
   private:
    std::vector<std::unique_ptr<std::string>> m_cachedPropNames;
    std::unordered_map<const char*, std::set<uint64_t>, NoCaseAsciiStrHash, NoCaseAsciiStrEqual> m_propMap;

   private:
    void Insert(ECN::ECClassId classId, Utf8CP propertyName);

   public:
    InMemoryPropertyExistMap() {}
    ~InMemoryPropertyExistMap() {}
    DbResult Build(ECDbCR ecdb, bool onlyTopLevelProperty);
    bool Empty() const { return m_cachedPropNames.empty(); }
    bool Exist(ECN::ECClassId classId, Utf8CP propertyName) const;
};

///=======================================================================================
// @bsistruct
//+===============+===============+===============+===============+===============+======
struct PerfLogScope final {
   private:
    Utf8CP m_module;
    Utf8CP m_method;
    bool m_disposed;

   public:
    PerfLogScope(Utf8CP module, Utf8CP method) : m_module(module), m_method(method), m_disposed(false) { PERFLOG_START(m_module, m_method); }
    void Dispose() {
        m_disposed = true;
        PERFLOG_FINISH(m_module, m_method);
    }
    ~PerfLogScope() {
        if (!m_disposed)
            Dispose();
    }
};
#define ECDB_PERF_LOG_SCOPE_BEGIN(NAME, DESC) PerfLogScope __perfScope_##NAME("ECDb", DESC)
#define ECDB_PERF_LOG_SCOPE_END(NAME) __perfScope_##NAME.Dispose()
#define ECDB_PERF_LOG_SCOPE(DESC) ECDB_PERF_LOG_SCOPE_BEGIN(main, DESC)

END_BENTLEY_SQLITE_EC_NAMESPACE
