/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <BeSQLite/BeSQLite.h>
#include <vector>
#include <unordered_map>
#include <string>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Writes a binary blob containing all EC schema metadata from an ECDb's ec_ tables.
//! The output is consumed by TypeScript's RuntimeSchemaContext.fromBinary().
//!
//! The writer reads directly from ec_ SQLite tables, bypassing the C++ ECObjects object
//! graph entirely. This is significantly faster than JSON serialization via ECSchema.
//!
//! Binary format v2: tag-delimited records with a deduplicated string table.
//! Layout: header + records + string table (offset patched in header).
//! The TS reader's parseRuntimeSchemaBlob() understands this exact format.
//!
//! Thread safety: safe for concurrent reads when ECDb is in WAL mode.
//! Intended to be called from a DgnDbWorker::Execute() on a background thread.
// @bsistruct
//=======================================================================================
struct RuntimeSchemaWriter
    {
private:
    bvector<Byte> m_output;
    bvector<Utf8String> m_stringTable;
    std::unordered_map<std::string, uint32_t> m_stringIndex;

    // Binary encoding helpers
    void PutU8(uint8_t v) { m_output.push_back(v); }
    void PutU16(uint16_t v) { m_output.push_back((Byte)(v & 0xFF)); m_output.push_back((Byte)(v >> 8)); }
    void PutU32(uint32_t v) { for (int i = 0; i < 4; i++) { m_output.push_back((Byte)(v & 0xFF)); v >>= 8; } }
    void PutI32(int32_t v) { PutU32((uint32_t)v); }
    void PutF64(double v) { auto p = reinterpret_cast<Byte const*>(&v); m_output.insert(m_output.end(), p, p + 8); }

    //! Intern a string and write its index as a U32 reference.
    void PutSRef(Utf8CP str) { PutU32(Intern(str)); }

    //! Write raw bytes with a U32 length prefix.
    void PutRaw(Utf8CP data, uint32_t len) { PutU32(len); if (len) m_output.insert(m_output.end(), (Byte const*)data, (Byte const*)data + len); }

    static Utf8CP Safe(Utf8CP s) { return s ? s : ""; }

public:
    //! Intern a string into the string table. Returns the SID (string index).
    //! Duplicate strings return the same SID. Empty/null strings map to SID 0.
    uint32_t Intern(Utf8CP str);

    //! Read all schemas from the given database and write the binary blob.
    //! @param db  The database to read from (ECDb or DgnDb).
    //! @param includeCustomAttributes  Whether to include custom attribute data.
    //!        Should be false for RuntimeSchemaContext (CAs are loaded lazily via ECSQL).
    void WriteAllSchemas(DbCR db, bool includeCustomAttributes);

    //! Get the output blob. Valid after WriteAllSchemas() completes.
    bvector<Byte> const& GetOutput() const { return m_output; }

    //! Move the output blob out. Invalidates internal state.
    bvector<Byte> TakeOutput() { return std::move(m_output); }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
