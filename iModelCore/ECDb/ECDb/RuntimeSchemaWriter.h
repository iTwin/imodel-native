/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <BeSQLite/BeSQLite.h>
#include <vector>
#include <unordered_map>
#include <unordered_set>
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
//! Thread safety: safe for concurrent reads when ECDb is in WAL mode.
//! Called from the PragmaRuntimeSchemas handler on the ConcurrentQuery thread pool.
// @bsistruct
//=======================================================================================
struct RuntimeSchemaWriter
    {
private:
    bvector<Byte> m_output;
    bvector<Utf8String> m_stringTable;
    std::unordered_map<std::string, uint32_t> m_stringIndex;

    // Property definition dedup
    struct PropertyDefRecord
        {
        uint32_t nameSid;
        uint8_t kind;
        uint16_t primitiveType;
        uint32_t extTypeSid;
        uint32_t enumSchemaSid;
        uint32_t enumNameSid;
        uint32_t structSchemaSid;
        uint32_t structClassSid;
        uint32_t koqSchemaSid;
        uint32_t koqNameSid;
        uint32_t catSchemaSid;
        uint32_t catNameSid;
        uint32_t arrayMinOccurs;
        uint32_t arrayMaxOccurs;
        uint32_t navRelSchemaSid;
        uint32_t navRelClassSid;
        uint8_t navDirection;
        uint8_t isReadonly;
        uint8_t isHidden;
        uint32_t descriptionSid;
        };

    struct PropertyRefRecord
        {
        uint32_t defIdx;
        uint32_t labelSid;
        int32_t priority;
        };

    bvector<PropertyDefRecord> m_propDefs;
    std::unordered_map<std::string, uint32_t> m_propDefIndex;
    std::unordered_map<int64_t, bvector<PropertyRefRecord>> m_classPropRefs;
    std::unordered_set<int64_t> m_hiddenPropertyIds;
    std::unordered_set<int64_t> m_queryViewClassIds;

    void CollectHiddenPropertyIds(DbCR db);
    void CollectQueryViewClassIds(DbCR db);
    void CollectPropertyDedup(DbCR db);
    uint32_t AddPropertyDef(PropertyDefRecord const& def);
    std::string PropertyDefSignature(PropertyDefRecord const& def) const;

    // Binary encoding helpers
    void PutU8(uint8_t v) { m_output.push_back(v); }
    void PutU16(uint16_t v) { m_output.push_back((Byte)(v & 0xFF)); m_output.push_back((Byte)(v >> 8)); }
    void PutU32(uint32_t v) { for (int i = 0; i < 4; i++) { m_output.push_back((Byte)(v & 0xFF)); v >>= 8; } }
    void PutI32(int32_t v) { PutU32((uint32_t)v); }
    void PutF64(double v) { auto p = reinterpret_cast<Byte const*>(&v); m_output.insert(m_output.end(), p, p + 8); }

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
    void WriteAllSchemas(DbCR db);

    //! Get the output blob. Valid after WriteAllSchemas() completes.
    bvector<Byte> const& GetOutput() const { return m_output; }

    //! Move the output blob out. Invalidates internal state.
    bvector<Byte> TakeOutput() { return std::move(m_output); }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
