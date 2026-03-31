/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ChangesetFieldFactory.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PreparedECChangesetReader final {
    using Stage = Changes::Change::Stage;
private:
    ECDbCR                         m_ecdb;
    bool                           m_invert = false;
    std::unique_ptr<ChangeStream>  m_changeStream;
    std::unique_ptr<ChangeGroup>   m_changeGroup;
    std::unique_ptr<Changes>       m_changes;
    Changes::Change                m_currentChange;
    Utf8String                     m_ddl;
    std::map<Stage, std::vector<std::unique_ptr<IECSqlValue>>> m_fields;

    PreparedECChangesetReader(PreparedECChangesetReader const&) = delete;
    PreparedECChangesetReader& operator=(PreparedECChangesetReader const&) = delete;
    void clearFields();
    void AddField(std::unique_ptr<IECSqlValue>, Stage);
    DbResult ReFetchValues();
    // Utf8String GetTableName() const { return m_currentChange.GetTableName(); };
    // DbOpcode GetOpcode() const { return m_currentChange.GetOpcode(); };
    bool IsOpen() const { return m_changeStream != nullptr; }
    bool IsStepped() const { return m_changes != nullptr && m_currentChange.IsValid(); }
    //! Builds a map from SQLite column name to DbValue for the current change at @p stage.
    //! Only columns that are present (non-absent) in the changeset are included in the map.
    //! The map can be passed directly to ChangesetFieldFactory::Create.
    DbResult GetColumnValues(Stage stage, ColumnValueMap& outMap) const;
    void FetchCurrentClassInfo();
    static void DumpColumnValues(ColumnValueMap const& map);
public:
    explicit PreparedECChangesetReader(ECDbCR ecdb);

    DbResult OpenFile(Utf8StringCR changesetFile, bool invert);
    DbResult Open(std::unique_ptr<ChangeStream> changeStream, bool invert);
    DbResult OpenGroup(T_Utf8StringVector const& files, Db const& db, bool invert);
    void Close();
    DbResult Step();
    ECDbCR GetECDb() const { return m_ecdb; }

    DbResult GetTableName(Utf8StringR tableName) const;
    DbResult GetOpcode(DbOpcode& opcode) const;
    int GetColumnCount(Stage stage) const;

    IECSqlValue const& GetValue(Stage stage, int columnIndex) const;
    DbResult GetInstanceKey(Stage stage, Utf8StringR key) const;
    DbResult IsECTable(bool& isECTable) const;
};

END_BENTLEY_SQLITE_EC_NAMESPACE
