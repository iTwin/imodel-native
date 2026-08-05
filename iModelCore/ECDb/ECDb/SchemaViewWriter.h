/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <BeSQLite/BeSQLite.h>
#include <set>
#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <string>
#include <string_view>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Writes a binary blob containing all EC schema metadata from an ECDb's ec_ tables.
//! The output is consumed by TypeScript's SchemaViewContext.fromBinary().
//!
//! The writer reads directly from ec_ SQLite tables, bypassing the C++ ECObjects object
//! graph entirely. This is significantly faster than JSON serialization via ECSchema.
//!
//! Binary format v1: tag-delimited records with a deduplicated string table and
//! property definition deduplication. Layout: header + PropertyDef table + schema
//! records (properties as PropRef) + string table (offset patched in header).
//! The TS reader's parseSchemaViewBlob() understands this exact format.
//!
//! Format spec: docs/learning/metadata/SchemaViewBinaryFormat.md in itwinjs-core.
//!
//! Thread safety: safe for concurrent reads when ECDb is in WAL mode.
//! HOWEVER, the writer itself is **not designed for concurrent use.** Each thread must create its own SchemaViewWriter instance.
//! Called from the PragmaSchemaView handler on the ConcurrentQuery thread pool.
// @bsistruct
//=======================================================================================
struct SchemaViewWriter
    {
private:
    bvector<Byte> m_output;
    // deque (not vector): push_back must not invalidate the string_views held by m_stringIndex.
    std::deque<std::string> m_stringTable;
    std::unordered_map<std::string_view, uint32_t> m_stringIndex;

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
    std::optional<int64_t> m_hiddenSchemaCAClassId;
    std::optional<int64_t> m_hiddenClassCAClassId;
    std::optional<int64_t> m_queryViewCAClassId;
    std::unordered_set<int64_t> m_excludedSchemaIds;
    std::unordered_set<int64_t> m_mixinClassIds;
    std::unordered_set<int64_t> m_queryViewClassIds;
    std::unordered_set<int64_t> m_hiddenSchemaIds;
    std::unordered_set<int64_t> m_schemasWithHiddenClasses; // HiddenSchema with ShowClasses != true
    std::unordered_set<int64_t> m_hiddenClassIds;
    std::unordered_set<int64_t> m_explicitlyShownClassIds; // HiddenClass with Show = true

    // Fragment support. When m_isFragment is true, only rows owned by schemas in
    // m_requestedSchemaIds are emitted (still intersected with the standard-schema exclusion).
    // When false, every non-excluded schema is emitted - the whole-schema blob.
    std::unordered_set<int64_t> m_requestedSchemaIds;
    bool m_isFragment = false;

    // Pre-pass: collect metadata needed before writing
    DbResult CollectExcludedSchemaIds(DbCR db);
    DbResult ResolveHiddenPropertyCAClassId(DbCR db);
    DbResult ResolveHiddenSchemaCAClassId(DbCR db);
    DbResult ResolveHiddenClassCAClassId(DbCR db);
    DbResult ResolveQueryViewCAClassId(DbCR db);
    static bool IsHiddenFromInstanceXml(Utf8CP instanceXml, Utf8CP showPropName = "Show");
    DbResult CollectPropertyDefsAndRefs(DbCR db);
    DbResult CollectMixinClassIds(DbCR db);
    DbResult CollectQueryViewClassIds(DbCR db);
    DbResult CollectHiddenSchemaIds(DbCR db);
    DbResult CollectHiddenClassIds(DbCR db);
    uint32_t InternPropertyDef(PropertyDefRecord const& def);

    // Append a "WHERE ..." filter (leading space, no trailing) restricting `column` - an
    // ec_Schema.Id or ec_*.SchemaId column - to the schemas this writer emits: always excludes
    // standard schemas, and for a fragment additionally restricts to m_requestedSchemaIds.
    void AppendSchemaFilter(Utf8StringR sql, Utf8CP column) const;

    // Verify every id in m_requestedSchemaIds is a real ec_Schema row. BE_SQLITE_NOTFOUND if not.
    DbResult ValidateRequestedSchemaIds(DbCR db);

    // Reset all per-run accumulation state so a single instance can be reused for multiple
    // WriteAllSchemas / WriteSchemas calls.
    void ResetState();

    // Shared body of WriteAllSchemas / WriteSchemas - resets state, runs the pre-passes and writes the tables.
    DbResult WriteBlob(DbCR db);

    // Per-table writers
    DbResult WritePropertyDefTable();
    DbResult WriteSchemaTable(DbCR db);
    DbResult WriteEnumTable(DbCR db);
    DbResult WriteKoqTable(DbCR db);
    DbResult WritePropCatTable(DbCR db);
    DbResult WriteClassTable(DbCR db);
    void WriteStringTable(size_t stOffsetPos);

    // Binary encoding helpers (all little-endian, matching the TS DataView reader)
    void PutU8(uint8_t v) { m_output.push_back(v); }
    void PutU16(uint16_t v) { m_output.push_back((Byte)(v & 0xFF)); m_output.push_back((Byte)(v >> 8)); }
    void PutU32(uint32_t v) { for (int i = 0; i < 4; i++) { m_output.push_back((Byte)(v & 0xFF)); v >>= 8; } }
    void PutI32(int32_t v) { PutU32((uint32_t)v); }
    void PutF64(double v) { uint64_t bits; memcpy(&bits, &v, 8); for (int i = 0; i < 8; i++) { m_output.push_back((Byte)(bits & 0xFF)); bits >>= 8; } }
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
    //! SchemaView when needed.
    //! @return BE_SQLITE_OK on success, or an error code if the ec_ tables are missing/corrupt.
    DbResult WriteAllSchemas(DbCR db);

    //! Read a chosen subset of schemas and write the binary blob - a "fragment".
    //! Only rows owned by schemas in requestedSchemaIds are emitted; standard/excluded schemas
    //! are skipped even when requested. Cross-schema references to schemas outside the fragment
    //! are written with the same global encoding as the whole-schema blob (row ids, name pairs);
    //! the consumer resolves them against its accumulated view when merging fragments.
    //! @param requestedSchemaIds ec_Schema.Id values to include. The caller is responsible for
    //!        passing a dependency-closed set (the reference closure); the writer does not expand
    //!        references. Must be non-empty.
    //! @return BE_SQLITE_OK on success; BE_SQLITE_ERROR on an empty set; BE_SQLITE_NOTFOUND if any
    //!         id is not an ec_Schema row.
    DbResult WriteSchemas(DbCR db, std::unordered_set<int64_t> const& requestedSchemaIds);

    //! Get the output blob. Only valid after WriteAllSchemas() / WriteSchemas() returns BE_SQLITE_OK;
    //! on an error return the blob is partial and must not be used.
    bvector<Byte> const& GetOutput() const { return m_output; }

    //! Move the output blob out. Invalidates internal state.
    bvector<Byte> TakeOutput() { return std::move(m_output); }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
