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

END_BENTLEY_SQLITE_NAMESPACE
