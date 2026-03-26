/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
PreparedECChangesetReader::PreparedECChangesetReader(ECDbCR ecdb)
    : m_ecdb(ecdb), m_currentChange(nullptr, false)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PreparedECChangesetReader::OpenFile(Utf8StringCR changesetFile, bool invert) {
    BeFileName input;
    input.AppendUtf8(changesetFile.c_str());

    bvector<BeFileName> files{input};
    auto reader = std::make_unique<ChangesetFileReaderBase>(files);
    DdlChanges ddlChanges;
    bool hasSchemaChanges;
    reader->MakeReader()->GetSchemaChanges(hasSchemaChanges, ddlChanges);
    if (!ddlChanges._IsEmpty())
        m_ddl = ddlChanges.ToString();

    Open(std::move(reader), invert);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PreparedECChangesetReader::Open(std::unique_ptr<ChangeStream> changeStream, bool invert) {
    m_invert = invert;
    m_changeStream = std::move(changeStream);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PreparedECChangesetReader::OpenGroup(T_Utf8StringVector const& files, Db const& db, bool invert) {
    m_changeGroup = std::make_unique<ChangeGroup>(db);
    DdlChanges ddlGroup;
    for (auto& changesetFile : files) {
        BeFileName inputFile(changesetFile);
        bvector<BeFileName> fileVec{inputFile};
        ChangesetFileReaderBase reader(fileVec);
        bool containsSchemaChanges;
        DdlChanges ddlChanges;
        reader.MakeReader()->GetSchemaChanges(containsSchemaChanges, ddlChanges);
        for (auto& ddl : ddlChanges.GetDDLs()) {
            ddlGroup.AddDDL(ddl.c_str());
        }
        reader.AddToChangeGroup(*m_changeGroup);
    }

    m_changeStream = std::make_unique<ChangeSet>();
    m_changeStream->FromChangeGroup(*m_changeGroup);
    m_ddl = ddlGroup.ToString();
    m_invert = invert;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PreparedECChangesetReader::clearFields() {
    m_fields.clear();
    m_fieldsRequiringOnAfterStep.clear();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PreparedECChangesetReader::Close() {
    m_currentChange = Changes::Change(nullptr, false);
    m_changes = nullptr;
    m_changeStream = nullptr;
    m_changeGroup = nullptr;
    m_invert = false;
    m_ddl.clear();
    clearFields();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PreparedECChangesetReader::OnAfterStep() const {
    for (ECSqlField* field : m_fieldsRequiringOnAfterStep)
        {
        ECSqlStatus stat = field->OnAfterStep();
        if (!stat.IsSuccess())
            return BE_SQLITE_ERROR;
        }

    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PreparedECChangesetReader::Step() {
    if (m_changes == nullptr) {
        m_changes = std::make_unique<Changes>(*m_changeStream, m_invert);
        m_currentChange = m_changes->begin();
    } else {
        ++m_currentChange;
    }
    auto stat = m_currentChange.IsValid() ? BE_SQLITE_ROW : BE_SQLITE_DONE;
    ReFetchValues();

    if (BE_SQLITE_ROW == stat)
        {
        auto stat = OnAfterStep();
        if (stat != BE_SQLITE_OK)
            return stat;
        }

    return stat;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int PreparedECChangesetReader::GetColumnCount() const {
    if (!m_currentChange.IsValid())
        return 0;
    return (int)m_fields.size();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PreparedECChangesetReader::ReFetchValues() {
    clearFields();
    if (m_currentChange.IsValid()) {
        DbSchema const& dbSchema = m_ecdb.Schemas().Main().GetDbSchema();
        DbTable const* dbTable = dbSchema.FindTable(GetTableName());
        if (dbTable == nullptr) {
            LOG.errorv("Table '%s' not found in schema.", GetTableName().c_str());
            return;
        }
        m_fields[Stage::New] = {};
        m_fields[Stage::Old] = {};
        for (auto& field : ChangesetFieldFactory::Create(m_ecdb, *dbTable, m_currentChange, Stage::New)) {
            ValidateAndUpdateField(std::move(field), Stage::New);
        }
        for (auto& field : ChangesetFieldFactory::Create(m_ecdb, *dbTable, m_currentChange, Stage::Old)) {
            ValidateAndUpdateField(std::move(field), Stage::Old);
        }
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& PreparedECChangesetReader::GetValue(Stage stage, int columnIndex) const {
    if (columnIndex < 0 || columnIndex >= (int)m_fields[stage].size()) {
        LOG.errorv("Column index %d is out of range for table '%s'.", columnIndex, GetTableName().c_str());
        return NoopECSqlValue::GetSingleton();
    }
    return *(m_fields[stage][columnIndex]);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PreparedECChangesetReader::ValidateAndUpdateField(std::unique_ptr<ECSqlField> field, Stage stage) {
    BeAssert(field != nullptr);
    if (field != nullptr)
        {
        if (field->RequiresOnAfterStep())
            m_fieldsRequiringOnAfterStep.push_back(field.get());

        m_fields[stage].push_back(std::move(field));
        }
}

END_BENTLEY_SQLITE_EC_NAMESPACE
