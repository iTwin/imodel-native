/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <BeSQLite/BeSQLite.h>
#include <set>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <string>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Writes a binary blob containing all EC schema metadata from an ECDb's ec_ tables.
//! The output is consumed by TypeScript's RuntimeSchemaContext.fromBinary().
//!
//! The writer reads directly from ec_ SQLite tables, bypassing the C++ ECObjects object
//! graph entirely. This is significantly faster than JSON serialization via ECSchema.
//!
//! Binary format v1: tag-delimited records with a deduplicated string table and
//! property definition deduplication. Layout: header + PropertyDef table + schema
//! records (properties as PropRef) + string table (offset patched in header).
//! The TS reader's parseRuntimeSchemaBlob() understands this exact format.
//!
//! Format spec: docs/learning/metadata/RuntimeSchemaBinaryFormat.md in itwinjs-core.
//!
//! Thread safety: safe for concurrent reads when ECDb is in WAL mode.
//! Called from the PragmaRuntimeSchemas handler on the ConcurrentQuery thread pool.
// @bsistruct
//=======================================================================================
struct RuntimeSchemaWriter
    {
private:
    bvector<Byte> m_output;
    bvector<Utf8CP> m_stringTable; // pointers into m_stringIndex keys (stable across rehash)
    std::unordered_map<std::string, uint32_t> m_stringIndex;

    // Property definition dedup
    struct PropertyDefRecord
        {
        uint32_t nameSid;
        uint8_t kind;
        uint16_t primitiveType;
        uint32_t extTypeSid;
        uint32_t enumRowId;        // ec_Enumeration.Id (0 = none)
        uint32_t structClassRowId; // ec_Class.Id for struct type (0 = none)
        uint32_t koqRowId;         // ec_KindOfQuantity.Id (0 = none)
        uint32_t catRowId;         // ec_PropertyCategory.Id (0 = none)
        uint32_t arrayMinOccurs;
        uint32_t arrayMaxOccurs;
        uint32_t navRelClassRowId; // ec_Class.Id for nav relationship (0 = none)
        uint8_t navDirection;
        uint8_t isReadonly;
        uint8_t isHidden;
        uint32_t descriptionSid;

        bool operator==(PropertyDefRecord const& o) const
            {
            return nameSid == o.nameSid && kind == o.kind && primitiveType == o.primitiveType
                && extTypeSid == o.extTypeSid && enumRowId == o.enumRowId
                && structClassRowId == o.structClassRowId
                && koqRowId == o.koqRowId && catRowId == o.catRowId
                && arrayMinOccurs == o.arrayMinOccurs && arrayMaxOccurs == o.arrayMaxOccurs
                && navRelClassRowId == o.navRelClassRowId
                && navDirection == o.navDirection && isReadonly == o.isReadonly && isHidden == o.isHidden
                && descriptionSid == o.descriptionSid;
            }
        };

    struct PropertyDefRecordHash
        {
        size_t operator()(PropertyDefRecord const& d) const
            {
            // FNV-1a hash over all fields (all POD integers, safe to combine directly)
            size_t h = 14695981039346656037ULL;
            auto mix = [&h](uint64_t v) { h ^= v; h *= 1099511628211ULL; };
            mix(d.nameSid); mix(d.kind); mix(d.primitiveType); mix(d.extTypeSid);
            mix(d.enumRowId); mix(d.structClassRowId);
            mix(d.koqRowId); mix(d.catRowId);
            mix(d.arrayMinOccurs); mix(d.arrayMaxOccurs);
            mix(d.navRelClassRowId); mix(d.navDirection);
            mix(d.isReadonly); mix(d.isHidden); mix(d.descriptionSid);
            return h;
            }
        };

    struct PropertyRefRecord
        {
        uint32_t defIdx;
        uint32_t labelSid;
        int32_t priority;
        uint32_t ecInstanceId;  // ec_Property.Id - per-ref because defs are deduplicated
        };

    bvector<PropertyDefRecord> m_propDefs;
    std::unordered_map<PropertyDefRecord, uint32_t, PropertyDefRecordHash> m_propDefIndex;
    std::unordered_map<int64_t, bvector<PropertyRefRecord>> m_classPropRefs;
    std::optional<int64_t> m_hiddenPropertyCAClassId;
    std::optional<int64_t> m_queryViewCAClassId;
    std::unordered_set<int64_t> m_excludedSchemaIds;
    std::unordered_set<int64_t> m_mixinClassIds;
    std::unordered_set<int64_t> m_queryViewClassIds;

    // Pre-pass: collect metadata needed before writing
    DbResult CollectExcludedSchemaIds(DbCR db);
    DbResult ResolveHiddenPropertyCAClassId(DbCR db);
    DbResult ResolveQueryViewCAClassId(DbCR db);
    static bool IsHiddenFromInstanceXml(Utf8CP instanceXml);
    DbResult CollectPropertyDefsAndRefs(DbCR db);
    DbResult CollectMixinClassIds(DbCR db);
    DbResult CollectQueryViewClassIds(DbCR db);
    uint32_t InternPropertyDef(PropertyDefRecord const& def);

    // Per-table writers (v2 flat format)
    DbResult WritePropertyDefTable();
    DbResult WriteSchemaTable(DbCR db);
    DbResult WriteEnumTable(DbCR db);
    DbResult WriteKoqTable(DbCR db);
    DbResult WritePropCatTable(DbCR db);
    DbResult WriteClassTable(DbCR db);
    void WriteStringTable(size_t stOffsetPos);

    // Binary encoding helpers
    void PutU8(uint8_t v) { m_output.push_back(v); }
    void PutU16(uint16_t v) { m_output.push_back((Byte)(v & 0xFF)); m_output.push_back((Byte)(v >> 8)); }
    void PutU32(uint32_t v) { for (int i = 0; i < 4; i++) { m_output.push_back((Byte)(v & 0xFF)); v >>= 8; } }
    void PutI32(int32_t v) { PutU32((uint32_t)v); }
    void PutF64(double v) { auto p = reinterpret_cast<Byte const*>(&v); m_output.insert(m_output.end(), p, p + 8); }
    void PatchU32(size_t pos, uint32_t val) { m_output[pos]=(Byte)(val&0xFF); m_output[pos+1]=(Byte)((val>>8)&0xFF); m_output[pos+2]=(Byte)((val>>16)&0xFF); m_output[pos+3]=(Byte)((val>>24)&0xFF); }
    void PatchU16(size_t pos, uint16_t val) { m_output[pos]=(Byte)(val&0xFF); m_output[pos+1]=(Byte)(val>>8); }

    //! Intern a string and write its index as a U32 reference.
    void PutSRef(Utf8CP str) { PutU32(Intern(str)); }

    static Utf8CP Safe(Utf8CP s) { return s ? s : ""; }

public:
    //! Intern a string into the string table. Returns the SID (string index).
    //! Duplicate strings return the same SID. Empty/null strings map to SID 0.
    uint32_t Intern(Utf8CP str);

    //! Read all schemas from the given database and write the binary blob.
    //! @param db  The database to read from (ECDb or DgnDb).
    //! Schemas in the exclusion list (Formats, Units, ECDb internal, pure-CA system schemas)
    //! are silently skipped. Their classes, properties, enums, etc. are not emitted.
    //! KindOfQuantity persistence units and presentation formats reference Units/Formats
    //! only as strings, so consumers don't need those schemas in the context.
    //! Custom attributes are not included - they are loaded lazily via ECSQL in
    //! RuntimeSchemaContext when needed.
    //! @return BE_SQLITE_OK on success, or an error code if the ec_ tables are missing/corrupt.
    DbResult WriteAllSchemas(DbCR db);

    //! Get the output blob. Valid after WriteAllSchemas() completes.
    bvector<Byte> const& GetOutput() const { return m_output; }

    //! Move the output blob out. Invalidates internal state.
    bvector<Byte> TakeOutput() { return std::move(m_output); }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
