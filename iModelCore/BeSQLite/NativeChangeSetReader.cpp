/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BeSQLite/NativeChangeSetReader.h>
#include <BeSQLite/ChangesetFile.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE

BEGIN_BENTLEY_SQLITE_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult NativeChangeSetReader::OpenFile(Utf8StringCR changesetFile, bool invert)
    {
    BeFileName input;
    input.AppendUtf8(changesetFile.c_str());

    if (!input.DoesPathExist())
        {
        m_lastError = "open(): changeset file specified does not exist";
        return BE_SQLITE_CANTOPEN;
        }

    auto reader = std::make_unique<ChangesetFileReaderBase>(bvector<BeFileName>{input});
    DdlChanges ddlChanges;
    bool hasSchemaChanges;
    reader->MakeReader()->GetSchemaChanges(hasSchemaChanges, ddlChanges);
    if (!ddlChanges._IsEmpty())
        m_ddl = ddlChanges.ToString();

    return OpenChangeStream(std::move(reader), invert);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult NativeChangeSetReader::OpenChangeStream(std::unique_ptr<ChangeStream> changeStream, bool invert)
    {
    if (m_changeStream != nullptr)
        {
        m_lastError = "openChangeStream(): reader is already in open state.";
        return BE_SQLITE_ERROR;
        }

    if (changeStream == nullptr)
        {
        m_lastError = "openChangeStream(): could not open an empty changeStream";
        return BE_SQLITE_ERROR;
        }

    m_invert = invert;
    m_changeStream = std::move(changeStream);
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult NativeChangeSetReader::OpenGroup(bvector<Utf8String> const& changesetFiles, Db const& db, bool invert)
    {
    m_changeGroup = std::make_unique<ChangeGroup>(db);
    DdlChanges ddlGroup;

    for (auto& changesetFile : changesetFiles)
        {
        BeFileName inputFile(changesetFile);
        if (!inputFile.DoesPathExist())
            {
            m_lastError = Utf8PrintfString("openGroup(): changeset file specified does not exist (%s)", inputFile.GetNameUtf8().c_str());
            return BE_SQLITE_CANTOPEN;
            }

        ChangesetFileReaderBase reader(bvector<BeFileName>{inputFile});
        bool containsSchemaChanges;
        DdlChanges ddlChanges;
        if (BE_SQLITE_OK != reader.MakeReader()->GetSchemaChanges(containsSchemaChanges, ddlChanges))
            {
            m_lastError = "openGroup(): unable to read schema changes";
            return BE_SQLITE_ERROR;
            }

        for (auto& ddl : ddlChanges.GetDDLs())
            ddlGroup.AddDDL(ddl.c_str());

        if (BE_SQLITE_OK != reader.AddToChangeGroup(*m_changeGroup))
            {
            m_lastError = "openGroup(): unable to add changeset to group";
            return BE_SQLITE_ERROR;
            }
        }

    m_changeStream = std::make_unique<ChangeSet>();
    if (BE_SQLITE_OK != m_changeStream->FromChangeGroup(*m_changeGroup))
        {
        m_lastError = "openGroup(): unable to create change stream";
        return BE_SQLITE_ERROR;
        }

    m_ddl = ddlGroup.ToString();
    m_invert = invert;
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NativeChangeSetReader::Close()
    {
    m_currentChange = Changes::Change(nullptr, false);
    m_changes = nullptr;
    m_changeStream = nullptr;
    m_changeGroup = nullptr;
    m_invert = false;
    m_ddl.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NativeChangeSetReader::Reset()
    {
    m_currentChange = Changes::Change(nullptr, false);
    m_changes = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult NativeChangeSetReader::WriteToFile(Utf8String const& fileName, bool containChanges, bool shouldOverride)
    {
    const auto kStmtDelimiter = ";";
    BeFileName outputFile(fileName);
    DdlChanges ddlChanges;
    bvector<Utf8String> individualDDLs;
    BeStringUtilities::Split(m_ddl.c_str(), kStmtDelimiter, individualDDLs);

    for (auto const& ddl : individualDDLs)
        ddlChanges.AddDDL(ddl.c_str());

    if (outputFile.DoesPathExist() && !shouldOverride)
        {
        m_lastError = "writeToFile(): changeset file already exists";
        return BE_SQLITE_ERROR;
        }

    if (outputFile.DoesPathExist() && shouldOverride)
        {
        if (outputFile.BeDeleteFile() != BeFileNameStatus::Success)
            {
            m_lastError = "writeToFile(): unable to delete existing changeset file";
            return BE_SQLITE_ERROR;
            }
        }

    ChangesetFileWriter writer(outputFile, containChanges, ddlChanges, nullptr);
    if (BE_SQLITE_OK != writer.Initialize())
        {
        m_lastError = "writeToFile(): unable to initialize changeset writer";
        return BE_SQLITE_ERROR;
        }

    if (m_changeGroup)
        {
        writer.FromChangeGroup(*m_changeGroup);
        }
    else if (m_changeStream)
        {
        ChangeGroup changeGroup;
        m_changeStream->AddToChangeGroup(changeGroup);
        writer.FromChangeGroup(changeGroup);
        }
    else
        {
        m_lastError = "writeToFile(): no changeset to write";
        return BE_SQLITE_ERROR;
        }

    if (!outputFile.DoesPathExist())
        {
        m_lastError = "writeToFile(): unable to write changeset file";
        return BE_SQLITE_ERROR;
        }

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult NativeChangeSetReader::Step()
    {
    if(!IsOpen())
        {
        m_lastError = "step(): no changeset opened.";
        return BE_SQLITE_ERROR;
        }
    if (m_changes == nullptr)
        {
        m_changes = std::make_unique<Changes>(*m_changeStream, m_invert);
        m_currentChange = m_changes->begin();
        }
    else
        {
        ++m_currentChange;
        }

    if (!m_currentChange.IsValid())
        return BE_SQLITE_DONE;

    auto rc = m_currentChange.GetOperation(&m_tableName, &m_columnCount, &m_opcode, &m_indirect);
    if (rc != BE_SQLITE_OK)
        {
        m_lastError = "step(): unable to read changeset";
        return rc;
        }

    rc = m_currentChange.GetPrimaryKeyColumns(&m_primaryKeyColumns, &m_primaryKeyColumnCount);
    if (rc != BE_SQLITE_OK)
        {
        m_lastError = "step(): unable to read changeset";
        return rc;
        }

    m_primaryKeyCount = 0;
    for (int i = 0; i < m_primaryKeyColumnCount; ++i)
        {
        if (m_primaryKeyColumns[i])
            ++m_primaryKeyCount;
        }

    return BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NativeChangeSetReader::IsOpCodeAndTargetMatch(DbOpcode opcode, int target) const
    {
    if (!HasRow())
        return false;
    if (target == 0 && opcode == DbOpcode::Insert)
        return false;
    if (target != 0 && opcode == DbOpcode::Delete)
        return false;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult NativeChangeSetReader::GetColumnValue(int col, int target, DbValue& out) const
    {
    if (!IsValidColumnIndex(col) || (target != 0 && target != 1))
        {
        m_lastError = "getColumnValue(): invalid column index or target.";
        out = DbValue(nullptr);
        return BE_SQLITE_ERROR;
        }
    if (!IsOpCodeAndTargetMatch(m_opcode, target))
        {
        m_lastError = "getColumnValue(): target does not match change opcode.";
        out = DbValue(nullptr);
        return BE_SQLITE_ERROR;
        }
    out = target == 0 ? m_currentChange.GetOldValue(col) : m_currentChange.GetNewValue(col);   
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult NativeChangeSetReader::GetRow(int target, std::vector<DbValue>& out) const
    {
    out.clear();
    int columnCount;
    auto rc = GetColumnCount(columnCount);
    if (rc != BE_SQLITE_OK)
        {
        m_lastError = "getRow(): unable to retrieve column count.";
        return rc;
        }

    for (int col = 0; col < columnCount; ++col)
        {
        DbValue val(nullptr);
        auto rc = GetColumnValue(col, target, val);
        if (rc != BE_SQLITE_OK)
            {
            m_lastError = "getRow(): unable to retrieve column value.";
            return rc;
            }
        out.push_back(val);
        }
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult NativeChangeSetReader::GetPrimaryKeys(std::vector<DbValue>& out) const
    {
    out.clear();
    int columnCount;
    auto rc = GetColumnCount(columnCount);
    if (rc != BE_SQLITE_OK)
        {
        m_lastError = "getPrimaryKeys(): unable to retrieve column count.";
        return rc;
        }

    DbOpcode opcode;
    rc = GetOpCode(opcode);
    if (rc != BE_SQLITE_OK)
        {
        m_lastError = "getPrimaryKeys(): unable to retrieve opcode.";
        return rc;
        }
    
    Byte* primaryKeyColumns = nullptr;
    rc = GetPrimaryKeyColumns(primaryKeyColumns);
    if (rc != BE_SQLITE_OK)        
        {
        m_lastError = "getPrimaryKeys(): unable to retrieve primary key column info.";
        return rc;
        }

    for (int col = 0; col < columnCount; ++col)
        {
        if(primaryKeyColumns[col])
            {
            DbValue val(nullptr);
            int target = (opcode == DbOpcode::Delete) ? 0 : 1;
            auto rc = GetColumnValue(col, target, val);
            if (rc != BE_SQLITE_OK)
                {
                m_lastError = "getPrimaryKeys(): unable to retrieve primary key column value.";
                return rc;
                }
            out.push_back(val);
            }
        }
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult NativeChangeSetReader::GetPrimaryKeyColumnIndexes(std::vector<uint64_t>& out) const
    {
    out.clear();
    int columnCount;
    auto rc = GetColumnCount(columnCount);
    if (rc != BE_SQLITE_OK)
        {
        m_lastError = "getPrimaryKeyColumnIndexes(): unable to retrieve column count.";
        return rc;
        }
    
    Byte* primaryKeyColumns = nullptr;
    rc = GetPrimaryKeyColumns(primaryKeyColumns);
    if (rc != BE_SQLITE_OK)        
        {
        m_lastError = "getPrimaryKeyColumnIndexes(): unable to retrieve primary key column info.";
        return rc;
        }

    for (int col = 0; col < columnCount; ++col)
        {
        if(primaryKeyColumns[col])
            {
            out.push_back(col);
            }
        }
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult NativeChangeSetReader::GetTableName(Utf8StringR out) const 
    { 
    if(!HasRow())
        {
        m_lastError = "getTableName(): there is no current row.";
        return BE_SQLITE_ERROR;
        }    
    out = m_tableName; 
    return BE_SQLITE_OK; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult NativeChangeSetReader::GetOpCode(DbOpcode& out) const 
    { 
    if(!HasRow())
        {
        m_lastError = "getOpCode(): there is no current row.";
        return BE_SQLITE_ERROR;
        }    
    out = m_opcode; 
    return BE_SQLITE_OK; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult NativeChangeSetReader::IsIndirectChange(bool& out) const 
    { 
    if(!HasRow())
        {
        m_lastError = "isIndirectChange(): there is no current row.";
        return BE_SQLITE_ERROR;
        }    
    out = m_indirect != 0; 
    return BE_SQLITE_OK; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult NativeChangeSetReader::GetColumnCount(int& out) const 
    { 
    if(!HasRow())
        {
        m_lastError = "getColumnCount(): there is no current row.";
        return BE_SQLITE_ERROR;
        }    
    out = m_columnCount; 
    return BE_SQLITE_OK; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult NativeChangeSetReader::GetDdlChanges(Utf8StringR out) const 
    { 
    if(!HasRow())
        {
        m_lastError = "getDdlChanges(): there is no current row.";
        return BE_SQLITE_ERROR;
        }    
    out = m_ddl; 
    return BE_SQLITE_OK; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult NativeChangeSetReader::GetPrimaryKeyColumns(Byte* out) const 
    { 
    if(!HasRow())
        {
        m_lastError = "getPrimaryKeyColumns(): there is no current row.";
        return BE_SQLITE_ERROR;
        }    
    out = m_primaryKeyColumns; 
    return BE_SQLITE_OK; 
    }

END_BENTLEY_SQLITE_NAMESPACE
