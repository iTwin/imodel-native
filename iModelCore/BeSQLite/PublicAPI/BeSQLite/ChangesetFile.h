/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "BeLzma.h"
#include "BeSQLite.h"
#include "ChangeSet.h"

BEGIN_BENTLEY_SQLITE_NAMESPACE

//=======================================================================================
//! Streams the contents of a file containing serialized change streams
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ChangesetFileReaderBase : ChangeStream {
private:
    Db const& m_db; // Used only for debugging
    bvector<BeFileName> m_files;

    struct Reader : Changes::Reader {
        ChangesetFileReaderBase const& m_base;
        Utf8String m_prefix = "";
        LzmaDecoder m_lzmaDecoder;
        BlockFilesLzmaInStream* m_inLzmaFileStream = nullptr;
        DbResult StartInput();
        DbResult ReadPrefix();
        BE_SQLITE_EXPORT void FinishInput();
        BE_SQLITE_EXPORT Utf8StringCR GetPrefix(DbResult& result);
        BE_SQLITE_EXPORT Reader(ChangesetFileReaderBase const& base) : m_base(base) {}
        ~Reader() { FinishInput(); }
        BE_SQLITE_EXPORT DbResult _Read(Byte* data, int* pSize) override;
        BE_SQLITE_EXPORT DbResult GetSchemaChanges(bool& containsSchemaChanges, DdlChangesR ddlChanges);
    };

    DbResult _Append(Byte const* pData, int nData) override { return BE_SQLITE_ERROR; }

    BE_SQLITE_EXPORT ChangeSet::ConflictResolution _OnConflict(ChangeSet::ConflictCause cause, Changes::Change iter) override;

public:
    virtual ~ChangesetFileReaderBase(){}
    bool _IsEmpty() const override { return m_files.size() == 0; }
    RefCountedPtr<Reader> MakeReader() const { return new Reader(*this); }
    RefCountedPtr<Changes::Reader> _GetReader() const override { return MakeReader(); }
    ChangesetFileReaderBase(bvector<BeFileName> const& files, Db const& db) : m_files(files), m_db(db) {}
    Db const& GetDb() const { return m_db; }
};

//=======================================================================================
//! Writes the contents of a change stream to a file
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ChangesetFileWriter : ChangeStream {
private:
    BeSQLite::LzmaEncoder m_lzmaEncoder;
    BeFileName m_pathname;
    BeFileLzmaOutStream* m_outLzmaFileStream;
    Utf8String m_prefix;
    Db const& m_db; // Only for debugging

    DbResult StartOutput();
    BE_SQLITE_EXPORT void FinishOutput();
    DbResult WritePrefix();

    RefCountedPtr<Changes::Reader> _GetReader() const override { return nullptr; }
    BE_SQLITE_EXPORT DbResult _Append(Byte const* pData, int nData) override;
    bool _IsEmpty() const override { return false; }
    BE_SQLITE_EXPORT ChangeSet::ConflictResolution _OnConflict(ChangeSet::ConflictCause cause, Changes::Change iter) override;

public:
    BE_SQLITE_EXPORT ChangesetFileWriter(BeFileNameCR pathname, bool containsEcSchemaChanges, DdlChangesCR ddlChanges, Db const&,
                                         BeSQLite::LzmaEncoder::LzmaParams const& lzmaParams = BeSQLite::LzmaEncoder::LzmaParams());
    BE_SQLITE_EXPORT DbResult Initialize();
    ~ChangesetFileWriter() { FinishOutput(); }
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct RevisionUtility final {
private:
    static Utf8String GetChangesetId(BeFileName changesetFile);
    static BentleyStatus ReadChangesetPrefix(BeSQLite::LzmaDecoder& lzmaDecoder, Utf8StringR prefix);
    static BentleyStatus OpenChangesetForReading(BeSQLite::LzmaDecoder& lzmaDecoder, BlockFilesLzmaInStream& inLzmaFileStream);
    static BentleyStatus ExportPrefixFile(BeFileName targetDir, Utf8StringCR changesetId, Utf8StringCR prefix);
    static BentleyStatus ExportChangesetFile(BeFileName targetDir, Utf8StringCR changesetId, BeSQLite::LzmaDecoder& lzmaDecoder);
    static BentleyStatus WritePrefix(BeSQLite::LzmaEncoder& lzmaEncoder, Utf8StringCR prefix);
    static BentleyStatus WriteChangeset(BeSQLite::LzmaEncoder& lzmaEncoder, BeFileName inChangesetFileName);
    static BentleyStatus GetUncompressSize(BeSQLite::LzmaDecoder& lzmaDecoder, uint32_t& uncompressSize);

public:
    RevisionUtility() = delete;
    BE_SQLITE_EXPORT static BentleyStatus RecompressRevision(Utf8CP sourceFile, Utf8CP targetFile, LzmaEncoder::LzmaParams param);
    BE_SQLITE_EXPORT static BentleyStatus DisassembleRevision(Utf8CP sourceFile, Utf8CP targetDir);
    BE_SQLITE_EXPORT static BentleyStatus AssembleRevision(Utf8CP inPrefixFile, Utf8CP inChangesetFile, Utf8CP outputFile, LzmaEncoder::LzmaParams params = LzmaEncoder::LzmaParams());
    BE_SQLITE_EXPORT static BentleyStatus ComputeStatistics(Utf8CP changesetFile, bool addPrefix, BeJsValue stats);
    BE_SQLITE_EXPORT static BentleyStatus GetUncompressSize(Utf8CP sourceFile, uint32_t& compressSize, uint32_t& changesetSize, uint32_t& prefixSize);
    BE_SQLITE_EXPORT static BentleyStatus DumpChangesetToDb(Utf8CP changesetFile, Utf8CP db, bool dumpColumns);
};
END_BENTLEY_SQLITE_NAMESPACE
