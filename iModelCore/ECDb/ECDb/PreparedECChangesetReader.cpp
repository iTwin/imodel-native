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
DbResult PreparedECChangesetReader::OpenFile(Utf8StringCR changesetFile, bool invert) {
    BeFileName input;
    input.AppendUtf8(changesetFile.c_str());

    if (!input.DoesPathExist())
        return BE_SQLITE_CANTOPEN;

    bvector<BeFileName> files{input};
    auto reader = std::make_unique<ChangesetFileReaderBase>(files);
    DdlChanges ddlChanges;
    bool hasSchemaChanges;
    reader->MakeReader()->GetSchemaChanges(hasSchemaChanges, ddlChanges);
    if (!ddlChanges._IsEmpty())
        m_ddl = ddlChanges.ToString();

    return Open(std::move(reader), invert);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PreparedECChangesetReader::Open(std::unique_ptr<ChangeStream> changeStream, bool invert) {
    if (m_changeStream != nullptr)
        return BE_SQLITE_ERROR;

    if (changeStream == nullptr)
        return BE_SQLITE_ERROR;

    m_invert = invert;
    m_changeStream = std::move(changeStream);
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PreparedECChangesetReader::OpenGroup(T_Utf8StringVector const& files, Db const& db, bool invert) {
    m_changeGroup = std::make_unique<ChangeGroup>(db);
    DdlChanges ddlGroup;
    for (auto& changesetFile : files) {
        BeFileName inputFile(changesetFile);
        if (!inputFile.DoesPathExist())
            return BE_SQLITE_CANTOPEN;

        bvector<BeFileName> fileVec{inputFile};
        ChangesetFileReaderBase reader(fileVec);
        bool containsSchemaChanges;
        DdlChanges ddlChanges;
        if (BE_SQLITE_OK != reader.MakeReader()->GetSchemaChanges(containsSchemaChanges, ddlChanges))
            return BE_SQLITE_ERROR;

        for (auto& ddl : ddlChanges.GetDDLs()) {
            ddlGroup.AddDDL(ddl.c_str());
        }
        if (BE_SQLITE_OK != reader.AddToChangeGroup(*m_changeGroup))
            return BE_SQLITE_ERROR;
    }

    m_changeStream = std::make_unique<ChangeSet>();
    if (BE_SQLITE_OK != m_changeStream->FromChangeGroup(*m_changeGroup))
        return BE_SQLITE_ERROR;

    m_ddl = ddlGroup.ToString();
    m_invert = invert;
    return BE_SQLITE_OK;
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
        if(GetOpcode() != DbOpcode::Delete) {
            for (auto& field : ChangesetFieldFactory::Create(m_ecdb, *dbTable, m_currentChange, Stage::New)) {
                ValidateAndUpdateField(std::move(field), Stage::New);
            }
        }
        if(GetOpcode() != DbOpcode::Insert) {
            for (auto& field : ChangesetFieldFactory::Create(m_ecdb, *dbTable, m_currentChange, Stage::Old)) {
                ValidateAndUpdateField(std::move(field), Stage::Old);
            }
        }
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& PreparedECChangesetReader::GetValue(Stage stage, int columnIndex) const {
    if (columnIndex < 0 || columnIndex >= (int)m_fields[stage].size()) {
        LOG.warningv("Column index %d is out of range for table '%s'.", columnIndex, GetTableName().c_str());
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
