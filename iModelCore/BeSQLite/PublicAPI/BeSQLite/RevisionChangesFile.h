/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "BeSQLite.h"
#include "ChangeSet.h"
#include "BeLzma.h"

BEGIN_BENTLEY_SQLITE_NAMESPACE

//=======================================================================================
//! Streams the contents of a file containing serialized change streams
// @bsiclass                                                 Ramanujam.Raman   10/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RevisionChangesFileReaderBase : ChangeStream
{
private:
    Db const& m_db; // Used only for debugging
    bvector<BeFileName> m_files;
    Utf8String m_prefix;

    LzmaDecoder m_lzmaDecoder;
    BlockFilesLzmaInStream* m_inLzmaFileStream;
    
    DbResult StartInput();
    DbResult ReadPrefix();
    void FinishInput();
    
    BE_SQLITE_EXPORT DbResult _InputPage(void *pData, int *pnData) override;
    BE_SQLITE_EXPORT void _Reset() override;
    BE_SQLITE_EXPORT ChangeSet::ConflictResolution _OnConflict(ChangeSet::ConflictCause cause, Changes::Change iter) override;

public:
    RevisionChangesFileReaderBase(bvector<BeFileName> const& files, Db const& db) : m_files(files), m_db(db), m_inLzmaFileStream(nullptr), m_prefix("") {}

    Db const& GetDb() const {return m_db;}

    BE_SQLITE_EXPORT Utf8StringCR GetPrefix(DbResult& result);

    BE_SQLITE_EXPORT DbResult GetSchemaChanges(bool& containsSchemaChanges, DbSchemaChangeSetR dbSchemaChanges);

    ~RevisionChangesFileReaderBase() { _Reset(); }
};

//=======================================================================================
//! Writes the contents of a change stream to a file
// @bsiclass                                                 Ramanujam.Raman   10/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RevisionChangesFileWriter : ChangeStream
{
private:
    BeSQLite::LzmaEncoder m_lzmaEncoder;
    BeFileName m_pathname;
    BeFileLzmaOutStream* m_outLzmaFileStream;
    Utf8String m_prefix;
    Db const& m_db; // Only for debugging

    DbResult StartOutput();
    void FinishOutput();
    DbResult WritePrefix();
    void InitPrefix(bool containsSchemaChanges, DbSchemaChangeSetCR dbSchemaChanges);

    DbResult _OutputPage(const void *pData, int nData) override;
    void _Reset() override;
    ChangeSet::ConflictResolution _OnConflict(ChangeSet::ConflictCause cause, Changes::Change iter) override;
public:
    BE_SQLITE_EXPORT RevisionChangesFileWriter(BeFileNameCR pathname, bool containsSchemaChanges, DbSchemaChangeSetCR dbSchemaChanges, Db const&);
    BE_SQLITE_EXPORT ~RevisionChangesFileWriter();
    BE_SQLITE_EXPORT DbResult Initialize();
    BE_SQLITE_EXPORT DbResult CallOutputPage(const void* pData, int nData);
    };

END_BENTLEY_SQLITE_NAMESPACE
