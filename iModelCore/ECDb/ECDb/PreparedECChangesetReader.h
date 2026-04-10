/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ChangesetValueFactory.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PreparedECChangesetReader final {
private:
    using Stage = Changes::Change::Stage;
    using Mode  = ECChangesetReader::Mode;
    //! A map from SQLite column name to its DbValue for a single changeset row at one stage.
    //! Only columns that are actually present (non-absent) in the changeset are included.
    using ColumnValueMap = std::unordered_map<Utf8String, DbValue>;
    ECDbCR                         m_ecdb;

    // change mappings
    bool                           m_invert = false;
    Mode                           m_mode = Mode::All_Properties;
    std::unique_ptr<ChangeStream>  m_changeStream;
    std::unique_ptr<ChangeGroup>   m_changeGroup;
    std::unique_ptr<Changes>       m_changes;
    Changes::Change                m_currentChange;

    // data fields
    std::unordered_map<Stage, std::vector<std::unique_ptr<IECSqlValue>>> m_fields;
    std::vector<Utf8String> m_changedPropNames;

    //filters
    std::vector<Utf8String> m_tableFilters;
    std::vector<DbOpcode> m_opcodeFilters;
    std::vector<ECN::ECClassId> m_ecclassIdFilters;

    PreparedECChangesetReader(PreparedECChangesetReader const&) = delete;
    PreparedECChangesetReader& operator=(PreparedECChangesetReader const&) = delete;
    void ClearFields();
    DbResult ReFetchValues();
    // Utf8String GetTableName() const { return m_currentChange.GetTableName(); };
    // DbOpcode GetOpcode() const { return m_currentChange.GetOpcode(); };
    bool IsOpen() const { return m_changeStream != nullptr; }
    bool IsStepped() const { return m_changes != nullptr && m_currentChange.IsValid(); }
    //! Builds a map from SQLite column name to DbValue for the current change at @p stage.
    //! Only columns that are present (non-absent) in the changeset are included in the map.
    //! The map can be passed directly to ChangesetValueFactory::Create.
    DbResult GetColumnValues(Stage stage, ColumnValueMap& outMap) const;
    void DumpColumnValues(ColumnValueMap const& map) const;
    bool IsTableAllowedPostFilter(Utf8StringCR tableName) const;
    bool IsOpcodeAllowedPostFilter(DbOpcode const& opcode) const;
    bool IsECClassIdAllowedPostFilter(ECClassId const& classId) const;
    Utf8String DbOpcodeToString(DbOpcode const& opcode) const;
public:
    explicit PreparedECChangesetReader(ECDbCR ecdb);

    DbResult OpenFile(Utf8StringCR changesetFile, bool invert, Mode mode);
    DbResult Open(std::unique_ptr<ChangeStream> changeStream, bool invert, Mode mode);
    DbResult OpenGroup(T_Utf8StringVector const& files, bool invert, Mode mode);
    void Close();
    DbResult Step();
    ECDbCR GetECDb() const { return m_ecdb; }

    DbResult GetTableName(Utf8StringR tableName) const;
    DbResult GetOpcode(DbOpcode& opcode) const;
    int GetColumnCount(Stage stage) const;

    IECSqlValue const& GetValue(Stage stage, int columnIndex) const;
    DbResult GetInstanceKey(Stage stage, Utf8StringR key) const;
    DbResult IsECTable(bool& isECTable) const;
    DbResult GetChangeFetchedPropertyNames(std::vector<Utf8String>& out) const;
    DbResult IsIndirectChange(bool& isIndirect) const;
    //filtering apis
    void SetTableFilters(std::vector<Utf8String> const& tableFilters) { m_tableFilters.clear(); m_tableFilters = tableFilters; }
    void SetOpcodeFilters(std::vector<DbOpcode> const& opcodeFilters) { m_opcodeFilters.clear(); m_opcodeFilters = opcodeFilters; }
    void SetECClassIdFilters(std::vector<ECN::ECClassId> const& ecclassIdFilters) { m_ecclassIdFilters.clear(); m_ecclassIdFilters = ecclassIdFilters; }
    void ClearTableFilters() { m_tableFilters.clear(); }
    void ClearOpcodeFilters() { m_opcodeFilters.clear(); }
    void ClearECClassIdFilters() { m_ecclassIdFilters.clear(); }
};

END_BENTLEY_SQLITE_EC_NAMESPACE
