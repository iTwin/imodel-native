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
    mutable std::map<Stage, std::vector<std::unique_ptr<ECSqlField>>> m_fields;
    //Calls to OnAfterStep/Reset on ECSqlFields can be very many, so only call it on fields that require it.
    mutable std::vector<ECSqlField*> m_fieldsRequiringOnAfterStep;

    PreparedECChangesetReader(PreparedECChangesetReader const&) = delete;
    PreparedECChangesetReader& operator=(PreparedECChangesetReader const&) = delete;
    void clearFields();
    void ValidateAndUpdateField(std::unique_ptr<ECSqlField>, Stage);
    void ReFetchValues();
    DbResult OnAfterStep() const;
    Utf8StringCR GetTableName() const { return m_currentChange.GetTableName(); }
    DbOpcode GetOpcode() const { return m_currentChange.GetOpcode(); }
public:
    explicit PreparedECChangesetReader(ECDbCR ecdb);

    DbResult OpenFile(Utf8StringCR changesetFile, bool invert);
    DbResult Open(std::unique_ptr<ChangeStream> changeStream, bool invert);
    DbResult OpenGroup(T_Utf8StringVector const& files, Db const& db, bool invert);
    void Close();
    DbResult Step();

    bool IsOpen() const { return m_changeStream != nullptr; }

    IECSqlValue const& GetValue(Stage stage, int columnIndex) const;
};

END_BENTLEY_SQLITE_EC_NAMESPACE
