/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ChangeUnifierSpill.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

namespace {

// SQLite orders integers as signed. Flipping the top bit makes that order match the unsigned order of UnifiedKey.
int64_t ToSqlOrder(uint64_t value) { return static_cast<int64_t>(value ^ (1ull << 63)); }
uint64_t FromSqlOrder(int64_t value) { return static_cast<uint64_t>(value) ^ (1ull << 63); }

//=======================================================================================
// Little-endian byte writer.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ByteWriter final {
    static void PutByte(std::vector<Byte>& out, Byte b) { out.push_back(b); }
    static void PutVarint(std::vector<Byte>& out, uint64_t v) {
        while (v >= 0x80) {
            out.push_back(static_cast<Byte>(v | 0x80));
            v >>= 7;
        }
        out.push_back(static_cast<Byte>(v));
    }
    static void PutFixed64(std::vector<Byte>& out, uint64_t v) {
        for (int i = 0; i < 8; ++i)
            out.push_back(static_cast<Byte>(v >> (8 * i)));
    }
    static void PutFixed32(std::vector<Byte>& out, uint32_t v) {
        for (int i = 0; i < 4; ++i)
            out.push_back(static_cast<Byte>(v >> (8 * i)));
    }
    static void PatchFixed32(std::vector<Byte>& out, size_t pos, uint32_t v) {
        for (int i = 0; i < 4; ++i)
            out[pos + i] = static_cast<Byte>(v >> (8 * i));
    }
    static void PutBytes(std::vector<Byte>& out, void const* data, size_t size) {
        PutVarint(out, size);
        if (size > 0) {
            Byte const* p = static_cast<Byte const*>(data);
            out.insert(out.end(), p, p + size);
        }
    }
};

//=======================================================================================
// Bounds-checked little-endian byte reader. Once a read runs past the end, IsOk() stays false.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ByteReader final {
private:
    Byte const* m_pos;
    Byte const* m_end;
    bool m_ok = true;

public:
    ByteReader(Byte const* data, size_t size) : m_pos(data), m_end(data + size) {}
    bool IsOk() const { return m_ok; }
    Byte GetByte() {
        if (m_pos >= m_end) { m_ok = false; return 0; }
        return *m_pos++;
    }
    uint64_t GetVarint() {
        uint64_t v = 0;
        for (int shift = 0; shift < 64; shift += 7) {
            if (m_pos >= m_end) { m_ok = false; return 0; }
            Byte b = *m_pos++;
            v |= static_cast<uint64_t>(b & 0x7F) << shift;
            if ((b & 0x80) == 0)
                return v;
        }
        m_ok = false;
        return 0;
    }
    uint64_t GetFixed64() {
        if (m_end - m_pos < 8) { m_ok = false; m_pos = m_end; return 0; }
        uint64_t v = 0;
        for (int i = 0; i < 8; ++i)
            v |= static_cast<uint64_t>(m_pos[i]) << (8 * i);
        m_pos += 8;
        return v;
    }
    uint32_t GetFixed32() {
        if (m_end - m_pos < 4) { m_ok = false; m_pos = m_end; return 0; }
        uint32_t v = 0;
        for (int i = 0; i < 4; ++i)
            v |= static_cast<uint32_t>(m_pos[i]) << (8 * i);
        m_pos += 4;
        return v;
    }
    //! Returns a pointer to @p size bytes and advances, or nullptr if not enough bytes are left.
    Byte const* GetRaw(size_t size) {
        if (static_cast<size_t>(m_end - m_pos) < size) { m_ok = false; m_pos = m_end; return nullptr; }
        Byte const* p = m_pos;
        m_pos += size;
        return p;
    }
};

enum class ValueTag : Byte { Null = 0, False = 1, True = 2, Int64 = 3, UInt64 = 4, Double = 5, String = 6, Binary = 7, Array = 8, Object = 9 };

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus DecodeValue(ByteReader& reader, ChangeUnifier::IInstanceWriter& out) {
    ValueTag tag = (ValueTag) reader.GetByte();
    if (!reader.IsOk())
        return ERROR;
    switch (tag) {
        case ValueTag::Null:
            out.Null();
            return SUCCESS;
        case ValueTag::False:
            out.Bool(false);
            return SUCCESS;
        case ValueTag::True:
            out.Bool(true);
            return SUCCESS;
        case ValueTag::Int64: {
            const int64_t v = static_cast<int64_t>(reader.GetFixed64());
            if (!reader.IsOk())
                return ERROR;
            out.Int64(v);
            return SUCCESS;
        }
        case ValueTag::UInt64: {
            const uint64_t v = reader.GetFixed64();
            if (!reader.IsOk())
                return ERROR;
            out.UInt64(v);
            return SUCCESS;
        }
        case ValueTag::Double: {
            uint64_t bits = reader.GetFixed64();
            if (!reader.IsOk())
                return ERROR;
            double d;
            memcpy(&d, &bits, sizeof(d));
            out.Double(d);
            return SUCCESS;
        }
        case ValueTag::String: {
            size_t len = static_cast<size_t>(reader.GetVarint());
            Byte const* p = reader.GetRaw(len);
            if (!reader.IsOk())
                return ERROR;
            out.String(reinterpret_cast<Utf8CP>(p), len, 0);
            return SUCCESS;
        }
        case ValueTag::Binary: {
            size_t len = static_cast<size_t>(reader.GetVarint());
            Byte const* p = reader.GetRaw(len);
            if (!reader.IsOk())
                return ERROR;
            out.Binary(p, len);
            return SUCCESS;
        }
        case ValueTag::Array: {
            uint32_t count = reader.GetFixed32();
            if (!reader.IsOk())
                return ERROR;
            out.StartArray();
            for (uint32_t i = 0; i < count; ++i) {
                if (SUCCESS != DecodeValue(reader, out))
                    return ERROR;
            }
            out.EndArray();
            return SUCCESS;
        }
        case ValueTag::Object: {
            uint32_t count = reader.GetFixed32();
            if (!reader.IsOk())
                return ERROR;
            out.StartObject();
            for (uint32_t i = 0; i < count; ++i) {
                size_t len = static_cast<size_t>(reader.GetVarint());
                Byte const* p = reader.GetRaw(len);
                if (!reader.IsOk())
                    return ERROR;
                out.Key(reinterpret_cast<Utf8CP>(p), len, 0);
                if (SUCCESS != DecodeValue(reader, out))
                    return ERROR;
            }
            out.EndObject();
            return SUCCESS;
        }
    }
    return ERROR;
}

//---------------------------------------------------------------------------------------
// Appends one spill record: [uint32 length][payload].
// @bsimethod
//---------------------------------------------------------------------------------------
void WriteRecord(std::vector<Byte>& out, KeyedEntry const& record) {
    UnifiedKey const& key = record.first;
    UnifiedEntry const& entry = record.second;
    size_t lenPos = out.size();
    ByteWriter::PutFixed32(out, 0);
    size_t start = out.size();
    ByteWriter::PutFixed64(out, key.m_rootClassId);
    ByteWriter::PutFixed64(out, key.m_instanceId);
    ByteWriter::PutByte(out, key.m_stage);
    ByteWriter::PutFixed64(out, entry.m_classId);
    ByteWriter::PutByte(out, (Byte) entry.m_op);
    ByteWriter::PutByte(out, entry.m_isIndirect ? 1 : 0);
    auto putIds = [&](std::vector<uint32_t> const& ids) {
        ByteWriter::PutVarint(out, ids.size());
        for (uint32_t id : ids)
            ByteWriter::PutVarint(out, id);
    };
    putIds(entry.m_tables);
    putIds(entry.m_changeIndexes);
    putIds(entry.m_fetchedNames);
    ByteWriter::PutVarint(out, entry.m_props.size());
    for (auto const& prop : entry.m_props) {
        ByteWriter::PutVarint(out, prop.m_nameId);
        ByteWriter::PutByte(out, prop.m_isClassId ? 1 : 0);
        ByteWriter::PutBytes(out, prop.m_value.data(), prop.m_value.size());
    }
    ByteWriter::PatchFixed32(out, lenPos, static_cast<uint32_t>(out.size() - start));
}

//---------------------------------------------------------------------------------------
// Reads the payload of one spill record. Reuses the capacity of the vectors in @p record.
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ReadRecord(Byte const* data, size_t size, KeyedEntry& record) {
    ByteReader reader(data, size);
    UnifiedKey& key = record.first;
    UnifiedEntry& entry = record.second;
    key.m_rootClassId = reader.GetFixed64();
    key.m_instanceId = reader.GetFixed64();
    key.m_stage = reader.GetByte();
    entry.m_classId = reader.GetFixed64();
    entry.m_op = (UnifiedEntry::Op) reader.GetByte();
    entry.m_isIndirect = reader.GetByte() != 0;
    auto getIds = [&](std::vector<uint32_t>& ids) {
        uint64_t count = reader.GetVarint();
        if (!reader.IsOk() || count > size)
            return false;
        ids.clear();
        ids.reserve(static_cast<size_t>(count));
        for (uint64_t i = 0; i < count && reader.IsOk(); ++i)
            ids.push_back(static_cast<uint32_t>(reader.GetVarint()));
        return reader.IsOk();
    };
    if (!getIds(entry.m_tables) || !getIds(entry.m_changeIndexes) || !getIds(entry.m_fetchedNames))
        return ERROR;
    uint64_t propCount = reader.GetVarint();
    if (!reader.IsOk() || propCount > size)
        return ERROR;
    entry.m_props.resize(static_cast<size_t>(propCount));
    for (auto& prop : entry.m_props) {
        prop.m_nameId = static_cast<uint32_t>(reader.GetVarint());
        prop.m_isClassId = reader.GetByte() != 0;
        size_t len = static_cast<size_t>(reader.GetVarint());
        Byte const* p = reader.GetRaw(len);
        if (!reader.IsOk())
            return ERROR;
        prop.m_value.assign(p, p + len);
    }
    return reader.IsOk() ? SUCCESS : ERROR;
}

} // namespace

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct RenderedValue::Node final {
    enum class Kind : uint8_t { Null, Bool, Int64, UInt64, Double, String, Binary, Object, Array };

    Kind m_kind = Kind::Null;
    bool m_bool = false;
    int64_t m_int64 = 0;
    uint64_t m_uint64 = 0;
    double m_double = 0;
    Utf8String m_string;
    std::vector<Byte> m_binary;
    std::vector<std::pair<Utf8String, std::unique_ptr<Node>>> m_members;
    std::vector<std::unique_ptr<Node>> m_items;

    void Reset() {
        m_kind = Kind::Null;
        m_string.clear();
        m_binary.clear();
        m_members.clear();
        m_items.clear();
    }
    bool IsNumeric() const { return m_kind == Kind::Int64 || m_kind == Kind::UInt64 || m_kind == Kind::Double; }
    double AsDouble() const {
        switch (m_kind) {
            case Kind::Int64: return static_cast<double>(m_int64);
            case Kind::UInt64: return static_cast<double>(m_uint64);
            default: return m_double;
        }
    }
    Node const* FindMember(Utf8CP name) const {
        if (m_kind == Kind::Object) {
            for (auto const& member : m_members)
                if (member.first.Equals(name))
                    return member.second.get();
        }
        return nullptr;
    }
};

namespace {

//=======================================================================================
// JsValueRef over a RenderedValue::Node. Numbers are classified like BeRapidJsonValue does, so that
// UnifiedValueCodec stores the same kinds as it did for a BeJsDocument.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct RenderedValueRef final : JsValueRef {
private:
    using Node = RenderedValue::Node;
    using Kind = Node::Kind;
    Node* m_node; //!< nullptr for a missing member

    template <typename T> static T IntegralFromDouble(double d) {
        if (std::isnan(d))
            return 0;
        if (d <= static_cast<double>(std::numeric_limits<T>::lowest()))
            return std::numeric_limits<T>::lowest();
        if (d >= static_cast<double>(std::numeric_limits<T>::max()))
            return std::numeric_limits<T>::max();
        return static_cast<T>(d);
    }
    Kind GetKind() const { return m_node == nullptr ? Kind::Null : m_node->m_kind; }
    Node& Set(Kind kind) {
        m_node->Reset();
        m_node->m_kind = kind;
        return *m_node;
    }

    ValueRefType GetType() const override { return ValueRefType::RapidJson; } // GetType only serves debugging
    JsValueRef& GetMember(Utf8CP name, bool) override {
        if (GetKind() != Kind::Object)
            Set(Kind::Object);
        for (auto& member : m_node->m_members) {
            if (member.first.Equals(name))
                return *new RenderedValueRef(member.second.get());
        }
        m_node->m_members.emplace_back(name, std::make_unique<Node>());
        return *new RenderedValueRef(m_node->m_members.back().second.get());
    }
    JsValueRef const& GetConstMember(Utf8CP name) const override {
        return *new RenderedValueRef(m_node == nullptr ? nullptr : const_cast<Node*>(m_node->FindMember(name)));
    }
    JsValueRef& GetArrayMember(ArrayIndex index) override {
        if (GetKind() != Kind::Array)
            Set(Kind::Array);
        if (index >= m_node->m_items.size()) {
            m_node->m_items.push_back(std::make_unique<Node>());
            index = static_cast<ArrayIndex>(m_node->m_items.size() - 1);
        }
        return *new RenderedValueRef(m_node->m_items[index].get());
    }
    JsValueRef const& GetConstArrayMember(ArrayIndex index) const override {
        Node* item = (GetKind() == Kind::Array && index < m_node->m_items.size()) ? m_node->m_items[index].get() : nullptr;
        return *new RenderedValueRef(item);
    }
    void SetNull() override { Set(Kind::Null); }
    void removeIndex(ArrayIndex index) override {
        if (GetKind() == Kind::Array && index < m_node->m_items.size())
            m_node->m_items.erase(m_node->m_items.begin() + index);
    }
    void removeMember(Utf8CP name) override {
        if (GetKind() != Kind::Object)
            return;
        auto& members = m_node->m_members;
        members.erase(std::remove_if(members.begin(), members.end(), [&](auto const& member) { return member.first.Equals(name); }), members.end());
    }
    bool isMember(Utf8CP name) const override { return m_node != nullptr && m_node->FindMember(name) != nullptr; }
    bool hasMember(Utf8CP name) const override { return isMember(name); }
    bool isNull() const override { return GetKind() == Kind::Null; }
    bool isBool() const override { return GetKind() == Kind::Bool; }
    bool isNumeric() const override { return m_node != nullptr && m_node->IsNumeric(); }
    bool isString() const override { return GetKind() == Kind::String; }
    bool isArray() const override { return GetKind() == Kind::Array; }
    bool isObject() const override { return GetKind() == Kind::Object; }
    bool isDate() const override { return false; }
    bool isArrayBuffer() const override { return false; }
    bool isTypedArray() const override { return false; }
    bool isFunction() const override { return false; }
    bool isPromise() const override { return false; }
    bool isDataView() const override { return false; }
    bool isBuffer() const override { return false; }
    bool isExternal() const override { return false; }
    void toArray() override {
        if (isNull())
            Set(Kind::Array);
    }
    void toObject() override {
        if (isNull())
            Set(Kind::Object);
    }
    uint32_t size() const override {
        switch (GetKind()) {
            case Kind::Array: return static_cast<uint32_t>(m_node->m_items.size());
            case Kind::Object: return static_cast<uint32_t>(m_node->m_members.size());
            default: return 0;
        }
    }
    Utf8CP ToUtf8CP() const override { return isString() ? m_node->m_string.c_str() : ""; }
    Utf8String ToJsonString() const override { return ToUtf8CP(); }
    void operator=(double value) override { Set(Kind::Double).m_double = value; }
    void operator=(bool value) override { Set(Kind::Bool).m_bool = value; }
    void operator=(int32_t value) override { Set(Kind::Int64).m_int64 = value; }
    void operator=(uint32_t value) override { Set(Kind::Int64).m_int64 = value; }
    void operator=(int64_t value) override { Set(Kind::Int64).m_int64 = value; }
    void operator=(uint64_t value) override {
        if (value <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max()))
            Set(Kind::Int64).m_int64 = static_cast<int64_t>(value);
        else
            Set(Kind::UInt64).m_uint64 = value;
    }
    void operator=(Utf8CP value) override { Set(Kind::String).m_string = value; }
    double GetDouble(double defVal) const override { return isNumeric() ? m_node->AsDouble() : defVal; }
    bool GetBoolean(bool defVal) const override {
        switch (GetKind()) {
            case Kind::Null: return defVal;
            case Kind::Bool: return m_node->m_bool;
            case Kind::String: return !m_node->m_string.empty();
            default: return isNumeric() ? m_node->AsDouble() != 0.0 : true;
        }
    }
    int32_t GetInt(int32_t defVal) const override {
        if (!isNumeric())
            return defVal;
        if (GetKind() == Kind::Int64 && m_node->m_int64 >= std::numeric_limits<int32_t>::min() && m_node->m_int64 <= std::numeric_limits<int32_t>::max())
            return static_cast<int32_t>(m_node->m_int64);
        return IntegralFromDouble<int32_t>(m_node->AsDouble());
    }
    uint32_t GetUInt(uint32_t defVal) const override {
        if (!isNumeric())
            return defVal;
        if (GetKind() == Kind::Int64 && m_node->m_int64 >= 0 && m_node->m_int64 <= std::numeric_limits<uint32_t>::max())
            return static_cast<uint32_t>(m_node->m_int64);
        return IntegralFromDouble<uint32_t>(m_node->AsDouble());
    }
    int64_t GetInt64(int64_t defVal) const override {
        if (!isNumeric())
            return defVal;
        return GetKind() == Kind::Int64 ? m_node->m_int64 : IntegralFromDouble<int64_t>(m_node->AsDouble());
    }
    uint64_t GetUInt64(uint64_t defVal) const override {
        switch (GetKind()) {
            case Kind::UInt64: return m_node->m_uint64;
            case Kind::Int64: return m_node->m_int64 >= 0 ? static_cast<uint64_t>(m_node->m_int64) : defVal;
            case Kind::Double: {
                const double d = m_node->m_double;
                return (d >= 0.0 && d < 0x1p64 && d == std::floor(d)) ? static_cast<uint64_t>(d) : defVal;
            }
            default: return defVal;
        }
    }
    NumericKind GetNumericKind() const override {
        switch (GetKind()) {
            case Kind::Int64: return NumericKind::Int64;
            case Kind::UInt64: return NumericKind::UInt64;
            case Kind::Double: return NumericKind::Double;
            default: return NumericKind::NotNumeric;
        }
    }
    bool isBinary() const override { return GetKind() == Kind::Binary; }
    BentleyStatus GetBinary(std::vector<Byte>& dest) const override {
        if (!isBinary()) {
            dest.clear();
            return ERROR;
        }
        dest = m_node->m_binary;
        return SUCCESS;
    }
    BentleyStatus GetBinary(ByteStream& dest) const override {
        dest.clear();
        if (!isBinary())
            return ERROR;
        dest.Append(m_node->m_binary.data(), static_cast<uint32_t>(m_node->m_binary.size()));
        return SUCCESS;
    }
    void SetBinary(Byte const* data, size_t size) override { Set(Kind::Binary).m_binary.assign(data, data + size); }
    bool ForEachProperty(std::function<bool(Utf8CP name, BeJsConst)> fn) const override {
        if (isObject()) {
            for (auto const& member : m_node->m_members) {
                if (fn(member.first.c_str(), BeJsConst(*new RenderedValueRef(member.second.get()))))
                    return true;
            }
        }
        return false;
    }
    bool ForEachArrayMember(std::function<bool(ArrayIndex, BeJsConst)> fn) const override {
        if (isArray()) {
            for (size_t i = 0; i < m_node->m_items.size(); ++i) {
                if (fn(static_cast<ArrayIndex>(i), BeJsConst(*new RenderedValueRef(m_node->m_items[i].get()))))
                    return true;
            }
        }
        return false;
    }
    bool ForEachArrayMemberValue(std::function<bool(ArrayIndex, BeJsValue)> fn) override {
        if (isArray()) {
            for (size_t i = 0; i < m_node->m_items.size(); ++i) {
                if (fn(static_cast<ArrayIndex>(i), BeJsValue(*new RenderedValueRef(m_node->m_items[i].get()))))
                    return true;
            }
        }
        return false;
    }
    Utf8String Stringify(StringifyFormat format) const override {
        BeJsDocument doc;
        doc.From(BeJsConst(*new RenderedValueRef(m_node)));
        return doc.Stringify(format);
    }

public:
    explicit RenderedValueRef(Node* node) : m_node(node) {}
};

} // namespace

RenderedValue::RenderedValue() : m_root(std::make_unique<Node>()) {}
RenderedValue::~RenderedValue() {}
BeJsValue RenderedValue::Root() { return BeJsValue(*new RenderedValueRef(m_root.get())); }
void RenderedValue::Clear() { m_root->Reset(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void UnifiedValueCodec::Encode(std::vector<Byte>& out, BeJsConst val) {
    if (val.isNull()) {
        ByteWriter::PutByte(out, (Byte) ValueTag::Null);
        return;
    }
    if (val.isBool()) {
        ByteWriter::PutByte(out, (Byte) (val.asBool() ? ValueTag::True : ValueTag::False));
        return;
    }
    if (val.isNumeric()) {
        switch (val.GetNumericKind()) {
            case JsValueRef::NumericKind::Int64:
                ByteWriter::PutByte(out, (Byte) ValueTag::Int64);
                ByteWriter::PutFixed64(out, static_cast<uint64_t>(val.asInt64()));
                return;
            case JsValueRef::NumericKind::UInt64:
                ByteWriter::PutByte(out, (Byte) ValueTag::UInt64);
                ByteWriter::PutFixed64(out, val.asUInt64());
                return;
            default: {
                double d = val.asDouble();
                uint64_t bits;
                static_assert(sizeof(bits) == sizeof(d), "unexpected double size");
                memcpy(&bits, &d, sizeof(bits));
                ByteWriter::PutByte(out, (Byte) ValueTag::Double);
                ByteWriter::PutFixed64(out, bits);
                return;
            }
        }
    }
    // RenderedValue keeps binary distinct from strings; with a BeJsValue that stores binary as a base64 string with a
    // header (rapidjson, JsonCpp), checking isBinary first decodes that string as binary.
    if (val.isBinary()) {
        std::vector<Byte> data;
        val.GetBinary(data);
        ByteWriter::PutByte(out, (Byte) ValueTag::Binary);
        ByteWriter::PutBytes(out, data.data(), data.size());
        return;
    }
    if (val.isString()) {
        Utf8CP str = val.asCString();
        ByteWriter::PutByte(out, (Byte) ValueTag::String);
        ByteWriter::PutBytes(out, str, strlen(str));
        return;
    }
    if (val.isArray()) {
        ByteWriter::PutByte(out, (Byte) ValueTag::Array);
        size_t countPos = out.size();
        ByteWriter::PutFixed32(out, 0);
        uint32_t count = 0;
        val.ForEachArrayMember([&](BeJsConst::ArrayIndex, BeJsConst member) {
            Encode(out, member);
            ++count;
            return false;
        });
        ByteWriter::PatchFixed32(out, countPos, count);
        return;
    }
    if (val.isObject()) {
        ByteWriter::PutByte(out, (Byte) ValueTag::Object);
        size_t countPos = out.size();
        ByteWriter::PutFixed32(out, 0);
        uint32_t count = 0;
        val.ForEachProperty([&](Utf8CP name, BeJsConst member) {
            ByteWriter::PutBytes(out, name, strlen(name));
            Encode(out, member);
            ++count;
            return false;
        });
        ByteWriter::PatchFixed32(out, countPos, count);
        return;
    }
    ByteWriter::PutByte(out, (Byte) ValueTag::Null);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus UnifiedValueCodec::Decode(std::vector<Byte> const& in, ChangeUnifier::IInstanceWriter& out) {
    ByteReader reader(in.data(), in.size());
    return DecodeValue(reader, out);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult UnifierSpillFile::Open(Utf8StringR err) {
    // nullptr: SQLite creates the database in its temp directory and deletes it on close or when the process ends.
    Db::CreateParams params(Db::PageSize::PAGESIZE_4K, Db::Encoding::Utf8, true, DefaultTxn::No);
    params.SetRawSQLite();
    DbResult rc = m_db.CreateNewDb(nullptr, params);
    if (BE_SQLITE_OK == rc) {
        // journal_mode can only change outside a transaction, and the final SELECT is one long read; each run's inserts get a Savepoint.
        m_db.SetAllowImplicitTransactions(true);
        rc = m_db.ExecuteSql("PRAGMA journal_mode=OFF;PRAGMA synchronous=OFF;PRAGMA locking_mode=EXCLUSIVE");
    }
    if (BE_SQLITE_OK == rc)
        rc = m_db.ExecuteDdl("CREATE TABLE run(root INTEGER, id INTEGER, stage INTEGER, seq INTEGER, payload BLOB, PRIMARY KEY(root, id, stage, seq)) WITHOUT ROWID");
    if (BE_SQLITE_OK == rc)
        rc = m_insert.Prepare(m_db, "INSERT INTO run(root, id, stage, seq, payload) VALUES(?, ?, ?, ?, ?)");
    if (BE_SQLITE_OK != rc) {
        err.Sprintf("ChangeUnifier: failed to create spill database (error %d).", (int) rc);
        Delete();
    }
    return rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult UnifierSpillFile::WriteRun(std::vector<KeyedEntry> const& sorted, Utf8StringR err) {
    if (sorted.empty())
        return BE_SQLITE_OK;
    if (!m_db.IsDbOpen()) {
        const DbResult rc = Open(err);
        if (BE_SQLITE_OK != rc)
            return rc;
    }
    Savepoint run(m_db, "unifierSpillRun");
    if (!run.IsActive()) {
        err = "ChangeUnifier: failed to start a transaction in the spill database.";
        return BE_SQLITE_ERROR;
    }
    for (KeyedEntry const& entry : sorted) {
        m_writeBuffer.clear();
        WriteRecord(m_writeBuffer, entry);
        static constexpr size_t lengthPrefix = 4; // WriteRecord's [uint32 length], not needed in a BLOB column
        m_insert.BindInt64(1, ToSqlOrder(entry.first.m_rootClassId));
        m_insert.BindInt64(2, ToSqlOrder(entry.first.m_instanceId));
        m_insert.BindInt(3, entry.first.m_stage);
        m_insert.BindInt(4, static_cast<int>(m_runCount));
        m_insert.BindBlob(5, m_writeBuffer.data() + lengthPrefix, static_cast<int>(m_writeBuffer.size() - lengthPrefix), Statement::MakeCopy::No);
        const DbResult rc = m_insert.Step();
        m_insert.Reset();
        if (BE_SQLITE_DONE != rc) {
            err.Sprintf("ChangeUnifier: failed to write to the spill database (error %d).", (int) rc);
            run.Cancel();
            return rc;
        }
    }
    const DbResult rc = run.Commit();
    if (BE_SQLITE_OK != rc) {
        err.Sprintf("ChangeUnifier: failed to commit to the spill database (error %d).", (int) rc);
        return rc;
    }
    ++m_runCount;
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// Moves m_select to its next row and reads its key.
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus UnifierSpillFile::StepSelect(Utf8StringR err) {
    const DbResult rc = m_select.Step();
    m_hasNext = BE_SQLITE_ROW == rc;
    if (!m_hasNext) {
        if (BE_SQLITE_DONE == rc)
            return SUCCESS;
        err.Sprintf("ChangeUnifier: failed to read the spill database (error %d).", (int) rc);
        return ERROR;
    }
    m_nextKey.m_rootClassId = FromSqlOrder(m_select.GetValueInt64(0));
    m_nextKey.m_instanceId = FromSqlOrder(m_select.GetValueInt64(1));
    m_nextKey.m_stage = static_cast<uint8_t>(m_select.GetValueInt(2));
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus UnifierSpillFile::StartMerge(uint64_t, Utf8StringR err) {
    m_insert.Finalize();
    // The primary key stores the rows in this order, so SQLite reads them without a sort (plan: SCAN run).
    if (BE_SQLITE_OK != m_select.Prepare(m_db, "SELECT root, id, stage, payload FROM run ORDER BY root, id, stage, seq")) {
        err = "ChangeUnifier: failed to read the spill database.";
        return ERROR;
    }
    return StepSelect(err);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus UnifierSpillFile::Pop(KeyedEntry& out, Utf8StringR err) {
    Byte const* payload = static_cast<Byte const*>(m_select.GetValueBlob(3));
    const int size = m_select.GetColumnBytes(3);
    if (payload == nullptr || SUCCESS != ReadRecord(payload, static_cast<size_t>(size), out)) {
        err = "ChangeUnifier: failed to read the spill database.";
        return ERROR;
    }
    return StepSelect(err);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void UnifierSpillFile::Delete() {
    m_select.Finalize();
    m_insert.Finalize();
    if (m_db.IsDbOpen())
        m_db.CloseDb();
    m_runCount = 0;
    m_hasNext = false;
    m_writeBuffer.clear();
}

END_BENTLEY_SQLITE_EC_NAMESPACE
