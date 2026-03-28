/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECChangesetReader.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! ECChangesetReader provides EC-typed value access for iterating over changesets.
//! It follows the same PIMPL pattern as ECSqlStatement.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECChangesetReader {
public:
    struct Impl;
    using Stage = Changes::Change::Stage;

private:
    Impl* m_pimpl = nullptr;

    ECChangesetReader(ECChangesetReader const&) = delete;
    ECChangesetReader& operator=(ECChangesetReader const&) = delete;
    ECChangesetReader(ECChangesetReader&& rhs) = delete;
    ECChangesetReader& operator=(ECChangesetReader&& rhs) = delete;

public:
    ECDB_EXPORT ECChangesetReader();
    ECDB_EXPORT ~ECChangesetReader();

    // Lifecycle
    ECDB_EXPORT DbResult OpenFile(ECDbCR ecdb, Utf8StringCR changesetFile, bool invert = false);
    ECDB_EXPORT DbResult OpenChangeStream(ECDbCR ecdb, std::unique_ptr<ChangeStream> changeStream, bool invert = false);
    ECDB_EXPORT DbResult OpenGroup(ECDbCR ecdb, T_Utf8StringVector const& changesetFiles, Db const& db, bool invert = false);
    ECDB_EXPORT void Close();
    ECDB_EXPORT DbResult Step();
    // Primary value accessor
    ECDB_EXPORT ECDb const* GetECDb() const;
    ECDB_EXPORT int GetColumnCount(Stage stage) const;
    ECDB_EXPORT DbResult GetTableName(Utf8StringR tableName) const;
    ECDB_EXPORT DbResult GetOpcode(DbOpcode& opcode) const;
    ECDB_EXPORT IECSqlValue const& GetValue(Stage stage, int columnIndex) const;
    ECDB_EXPORT DbResult GetInstanceKey(Stage stage, Utf8StringR key) const;

};


//=======================================================================================
//! @bsiclass
//=======================================================================================
struct ECChangesetRow : public IECSqlRow {
    private:
        ECChangesetReader const& m_reader;
        ECChangesetReader::Stage m_stage;       
    public:
        ECChangesetRow(ECChangesetReader const& reader, ECChangesetReader::Stage stage) : m_reader(reader), m_stage(stage) {}
        virtual int GetColumnCount() const override { return m_reader.GetColumnCount(m_stage); }
        virtual IECSqlValue const& GetValue(int columnIndex) const override { return m_reader.GetValue(m_stage, columnIndex); }
};

END_BENTLEY_SQLITE_EC_NAMESPACE