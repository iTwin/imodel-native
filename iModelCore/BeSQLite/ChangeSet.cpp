/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BeSQLite/ChangeSet.h>
#include <Bentley/Logging.h>

#define SQLITE_ENABLE_SESSION 1
#define SQLITE_ENABLE_PREUPDATE_HOOK 1

#include "SQLite/sqlite3.h"

#define STREAM_PAGE_BYTE_SIZE 64 * 1024
#define LOG (NativeLogging::CategoryLogger("BeSQLite"))
#define LOGCHANGESET (NativeLogging::CategoryLogger("Changeset"))

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeTracker::RecordDbSchemaChange(Utf8CP ddl)
    {
    if (!IsTracking())
        return BE_SQLITE_OK;

    m_ddlChanges.AddDDL(ddl);
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DdlChanges::AddDDL(Utf8CP ddl)
    {
    if (Utf8String::IsNullOrEmpty(ddl))
        {
        BeAssert(false);
        return;
        }

    if (!_IsEmpty())
        Append(";");

    Append(ddl);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DdlChanges::ToString() const {
    if (_IsEmpty())
        return "";

    Utf8String ddl;
    for (auto& chunk : m_data.m_chunks) {
        bvector<Byte> tmp = chunk;
        tmp.push_back(0); // make sure it's null terminated
        ddl.append((Utf8CP) tmp.data());
    }

    return ddl;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DdlChanges::Dump(Utf8CP label) const
    {
    if (label)
        LOGCHANGESET.infov("%s", label);

    if (_IsEmpty())
        {
        LOGCHANGESET.info("Empty");
        return;
        }

    auto ddl = ToString();
    bvector<Utf8String> tokens;
    BeStringUtilities::Split(ddl.c_str(), ";", tokens);
    for (Utf8StringCR str : tokens)
        {
        LOGCHANGESET.info(str.c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int filterCaller(void* tracker, Utf8CP tableName) {return (int) ((ChangeTracker*) tracker)->_FilterTable(tableName);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeTracker::CreateSession()
    {
    if (m_session)
        return  BE_SQLITE_OK;

    if (nullptr == m_db)
        {
        BeAssert(false);
        return  BE_SQLITE_ERROR;
        }

    DbResult result = (DbResult) sqlite3session_create(m_db->GetSqlDb(), "main", &m_session);
    BeAssert(BE_SQLITE_OK == result);

    if (BE_SQLITE_OK == result) {
        sqlite3session_table_filter(m_session, filterCaller, this); // set up auto-attach for all tables
        result = (DbResult)sqlite3session_object_config(m_session, SQLITE_SESSION_OBJCONFIG_SIZE, &m_enableChangesetSizeStats);
        if (result != BE_SQLITE_OK) {
            sqlite3session_delete(m_session);
            m_session = nullptr;
            return result;
        }
    }
    m_isTracking = true; // new sessions are on by default

    return  result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeTracker::EndTracking()
    {
    if (m_session)
        {
        sqlite3session_delete(m_session);
        m_session = nullptr;
        }
    m_ddlChanges.Clear();
    m_isTracking = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChangeTracker::EnableTracking(bool yesNo)
    {
    if (m_isTracking == yesNo)
        return m_isTracking;
    CreateSession();
    sqlite3session_enable(m_session, yesNo);
    m_isTracking = yesNo;
    return !yesNo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeTracker::Mode ChangeTracker::GetMode() const
    {
    return nullptr != m_session && 0 != sqlite3session_indirect(m_session, -1) ? Mode::Indirect : Mode::Direct;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeTracker::SetMode(Mode mode)
    {
    if (nullptr != m_session)
        sqlite3session_indirect(m_session, static_cast<int>(mode));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChangeTracker::HasDataChanges() const
    {
    return m_session && 0 == sqlite3session_isempty(m_session);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChangeTracker::HasChanges() const
    {
    return HasDataChanges() || HasDdlChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t ChangeTracker::GetMemoryUsed() const
    {
    return m_session ? sqlite3session_memory_used(m_session) : 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t ChangeTracker::GetChangesetSize() const
    {
    return m_session ? sqlite3session_changeset_size(m_session) : 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeTracker::EnableChangesetSizeStats(bool enabled) const
    {
    m_enableChangesetSizeStats =  enabled ? 1 : 0;
    if (m_session)
        return (DbResult)sqlite3session_object_config(m_session, SQLITE_SESSION_OBJCONFIG_SIZE, &m_enableChangesetSizeStats );

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeTracker::DifferenceToDb(Utf8StringP errMsgOut, BeFileNameCR baseFile)
    {
    // Check if Db GUIDs match
    if (true)
        {
        Db baseDb;
        DbResult result = baseDb.OpenBeSQLiteDb(baseFile, Db::OpenParams(Db::OpenMode::Readonly));
        if (BE_SQLITE_OK != result)
            {
            if (errMsgOut != nullptr)
                *errMsgOut = m_db->GetLastError();
            return result;
            }
        if (m_db->GetDbGuid() != baseDb.GetDbGuid())
            {
            if (errMsgOut != nullptr)
                *errMsgOut = "DbGuids differ";
            return BE_SQLITE_MISMATCH;
            }
        }

    DbResult result =  m_db->AttachDb(baseFile.GetNameUtf8().c_str(), "base");
    if (BE_SQLITE_OK != result)
        {
        if (errMsgOut != nullptr)
            *errMsgOut = m_db->GetLastError();
        return result;
        }

    Restart(); // make sure we don't currently have any changes

    Statement tablesStmt(*m_db, "SELECT name FROM main.sqlite_master WHERE type='table'");
    while (tablesStmt.Step() == BE_SQLITE_ROW)
        {
        Utf8CP tableName = tablesStmt.GetValueText(0);
        char* errMsg = nullptr;
        result = (DbResult) sqlite3session_diff(GetSqlSession(), "base", tableName, &errMsg);
        if (BE_SQLITE_OK != result)
            {
            if (errMsgOut != nullptr)
                {
                if (errMsg != nullptr)
                    *errMsgOut = errMsg;
                else
                    *errMsgOut = tableName;
                }
            break;
            }
        }

    m_db->DetachDb("base");
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeSet::Invert() {
    if (!IsValid()) {
        BeAssert(false);
        return BE_SQLITE_ERROR;
    }

    ChangeSet saved = std::move(*this);
    ChangeSet::Reader reader(saved);
    return (DbResult)sqlite3changeset_invert_strm(ChangeSet::Reader::ReadCallback, &reader, AppendCallback, this);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Changes::Finalize() const {
    m_reader = nullptr;

    if (nullptr != m_iter) {
        sqlite3changeset_finalize(m_iter);
        m_iter = nullptr;
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Changes::Change Changes::begin() const
    {
    Finalize();

    m_reader = m_changeStream._GetReader();
    Reader* reader = m_reader.get();
    if (nullptr != reader)
        sqlite3changeset_start_v2_strm(&m_iter, Changes::Reader::ReadCallback, (void*) reader, m_invert ? SQLITE_CHANGESETSTART_INVERT : 0);

    if (nullptr == m_iter)
        return Change(0, false);

    DbResult result = (DbResult) sqlite3changeset_next(m_iter);
    return Change(m_iter, result==BE_SQLITE_ROW);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Changes::~Changes() { Finalize(); }
DbResult Changes::Change::GetOperation(Utf8CP* tableName, int* nCols, DbOpcode* opcode, int* indirect) const { return (DbResult)sqlite3changeset_op(m_iter, tableName, nCols, (int*)opcode, indirect); }
DbResult Changes::Change::GetPrimaryKeyColumns(Byte** cols, int* nCols) const { return (DbResult)sqlite3changeset_pk(m_iter, cols, nCols); }
DbResult Changes::Change::GetFKeyConflicts(int* nConflicts) const { return (DbResult)sqlite3changeset_fk_conflicts(m_iter, nConflicts); }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbValue Changes::Change::GetOldValue(int colNum) const {
    SqlValueP val = nullptr;
    int rc = sqlite3changeset_old(m_iter, colNum, &val);
    BeAssert(rc == BE_SQLITE_OK);
    UNUSED_VARIABLE(rc);
    return DbValue(val);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbValue  Changes::Change::GetNewValue(int colNum) const {
    SqlValueP val = nullptr;
    int rc = sqlite3changeset_new(m_iter, colNum, &val);
    BeAssert(rc == BE_SQLITE_OK);
    UNUSED_VARIABLE(rc);
    return DbValue(val);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Changes::Change& Changes::Change::operator++()
    {
    m_isValid = (BE_SQLITE_ROW == (DbResult) sqlite3changeset_next(m_iter));
    return  *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DbValue::Format(int detailLevel) const
    {
    if (!IsValid())
        return "<<INVALID>>";

    switch (GetValueType())
        {
        case DbValueType::IntegerVal:
            return Utf8PrintfString("%" PRId64, GetValueInt64());

        case DbValueType::FloatVal:
            return Utf8PrintfString("%0.17lf", GetValueDouble());

        case DbValueType::TextVal:
            return Utf8PrintfString("\"%s\"", GetValueText());

        case DbValueType::BlobVal:
            return "...";

        case DbValueType::NullVal:
            return "NULL";
        }

    BeAssert(false);
    return "?";
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Changes::Change::FormatPrimarykeyColumns(bool isInsert, int detailLevel) const
    {
    Byte* pcols;
    int npcols;
    GetPrimaryKeyColumns(&pcols, &npcols);

    if (!pcols)
        return "";

    Utf8String pcolstr;
    for (int i=0; i<npcols; ++i)
        {
        if (pcols[i] == 0)
            continue;

        auto pkv = GetValue(i, isInsert ? Stage::New : Stage::Old);
        BeAssert(pkv.IsValid());

        if (pkv.IsNull())   // WIP_CHANGES -- why do we get this??
            continue;

        if (!pcolstr.empty())
            pcolstr.append(", ");

        pcolstr.append(pkv.Format(detailLevel));
        }
    return pcolstr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Changes::Change::FormatChange(Db const& db, Utf8CP tableName, DbOpcode opcode, int indirect, int detailLevel) const
    {
    bvector<Utf8String> columnNames;
    db.GetColumns(columnNames, tableName);

    Byte* pcols = nullptr;
    int npcols = 0;
    GetPrimaryKeyColumns(&pcols, &npcols);

    Utf8PrintfString line("key=%s", FormatPrimarykeyColumns((DbOpcode::Insert == opcode), detailLevel).c_str());

    switch (opcode)
        {
        case DbOpcode::Delete:
            line.append(",DELETE");
            break;
        case DbOpcode::Insert:
            line.append(",INSERT");
            break;
        case DbOpcode::Update:
            line.append(",UPDATE");
            break;
        default:
            BeAssert(false);
            line.append(",INVALID");
            return line;;
        }

    line.append(Utf8PrintfString("(%s)", indirect ? "indirect" : "direct"));

    for (int i = 0; i < npcols; ++i)
        {
        if (i >= columnNames.size())
            {
            line.append(Utf8PrintfString(" *** INVALID CHANGESET! MISSING SCHEMA CHANGESET! Table has only %d columns, but changeset thinks it has %d columns.", (int) columnNames.size(), npcols));
            break;
            }

        if (pcols[i] > 0)
            continue;

        Utf8String valStr;
        if (opcode == DbOpcode::Insert)
            {
            auto newVal = GetValue(i, Stage::New);
            if (newVal.IsNull())
                continue;
            valStr = newVal.Format(detailLevel);
            }
        else if (opcode == DbOpcode::Delete)
            {
            auto oldVal = GetValue(i, Stage::Old);
            if (oldVal.IsNull())
                continue;
            valStr = oldVal.Format(detailLevel);
            }
        else /* if (opcode == DbOpcode::Update) */
            {
            auto oldVal = GetValue(i, Stage::Old);
            auto newVal = GetValue(i, Stage::New);
            if (!oldVal.IsValid() && !newVal.IsValid())
                continue;
            valStr = Utf8PrintfString("%s->%s", oldVal.Format(detailLevel).c_str(), newVal.Format(detailLevel).c_str());
            }

        line.append(Utf8PrintfString(",%s=%s", columnNames[i].c_str(), valStr.c_str()));
        }

    if (npcols < columnNames.size())
        {
        line.append(Utf8PrintfString(" *** INVALID CHANGESET! MISSING SCHEMA CHANGESET! Table has %d columns, but changeset thinks it has only %d columns.", (int) columnNames.size(), npcols));
        }

    return line;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Changes::Change::Dump(Db const& db, bool isPatchSet, bset<Utf8String>& tablesSeen, int detailLevel) const
    {
    Utf8CP tableName;
    int nCols,indirect;
    DbOpcode opcode;
    DbResult rc = GetOperation(&tableName, &nCols, &opcode, &indirect);
    BeAssert(rc==BE_SQLITE_OK);
    UNUSED_VARIABLE(rc);

    if (tablesSeen.find(tableName) == tablesSeen.end())
        {
        LOGCHANGESET.infov("Table: %s", tableName);
        tablesSeen.insert(tableName);
        }

    LOGCHANGESET.info(FormatChange(db, tableName, opcode, indirect, detailLevel).c_str());

    if ((detailLevel > 0) && (DbOpcode::Insert != opcode))
        DumpCurrentValuesOfChangedColumns(db);
    }

    /*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeStream::Dump(Utf8CP label, Db const& db, bool isPatchSet, int detailLevel) const {
    if (label)
        LOGCHANGESET.infov("%s", label);

    bset<Utf8String> tablesSeen;

    Changes changes(*this, false);
    for (auto& change : changes)
        change.Dump(db, isPatchSet, tablesSeen, detailLevel);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ChangeStream::InterpretConflictCause(ChangeSet::ConflictCause cause, int detailLevel) {
    bool brief = (detailLevel == 0);
    switch (cause) {
    case ChangeSet::ConflictCause::Data:
        return brief ? "data" : "Data (PRIMARY KEY found, but data was changed)";
    case ChangeSet::ConflictCause::NotFound:
        return brief ? "not found" : "Not Found (PRIMARY KEY)";
    case ChangeSet::ConflictCause::Conflict:
        return brief ? "conflict" : "Conflict (Causes duplicate PRIMARY KEY)";
    case ChangeSet::ConflictCause::Constraint:
        return brief ? "constraint" : "Constraint (Causes UNIQUE, CHECK or NOT NULL constraint violation)";
    case ChangeSet::ConflictCause::ForeignKey:
        return brief ? "foreign key" : "ForeignKey (Causes FOREIGN KEY constraint violation)";
    }
    BeAssert(false);
    return "?";
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Changes::Change::DumpCurrentValuesOfChangedColumns(Db const& db) const
    {
    Utf8CP tableName = nullptr;
    int nCols, indirect;
    DbOpcode opcode;
    /* unused - DbResult result = */GetOperation(&tableName, &nCols, &opcode, &indirect);

    bvector<Utf8String> columnNames;
    db.GetColumns(columnNames, tableName);

    Byte* pcols = nullptr;
    int npcols = 0;
    GetPrimaryKeyColumns(&pcols, &npcols);

    if (!pcols)
        return;

    int64_t pk = 0;
    int pki = 0;
    for (int i = 0; i <= npcols; ++i)
        {
        if (pcols[i] == 0)
            continue;

        pk = GetValue(i, Changes::Change::Stage::Old).GetValueInt64();
        pki = i;
        break;
        }

    for (int i = 0; i <= npcols; ++i)
        {
        if (pcols[i] > 0)
            continue;

        if (!GetValue(i, Changes::Change::Stage::Old).IsValid()
         && !GetValue(i, Changes::Change::Stage::New).IsValid())
            continue;   // this col was not changed

        Statement stmt;
        stmt.Prepare(db, Utf8PrintfString("SELECT %s from %s WHERE %s=%lld",
            columnNames[i].c_str(),
            tableName,
            columnNames[pki].c_str(),
            pk).c_str());
        if (BE_SQLITE_ROW != stmt.Step())
            return; // The row is not found. This must be a NOT_FOUND conflict, i.e., there is no current row with this Id.

        Utf8String oldVal = stmt.GetDbValue(0).Format(1);

        LOG.infov(Utf8PrintfString("%s.%s was %s", tableName, columnNames[i].c_str(), oldVal.c_str()).c_str());
        }
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeGroup::ChangeGroup() { sqlite3changegroup_new((sqlite3_changegroup**)&m_changegroup); }
ChangeGroup::~ChangeGroup() { sqlite3changegroup_delete((sqlite3_changegroup*)m_changegroup); }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int ChangeStream::ConflictCallback(void* pCtx, int cause, SqlChangesetIterP iter) {
    return (int)((ChangeStream*)pCtx)->_OnConflict((ChangeSet::ConflictCause)cause, Changes::Change(iter, true));
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int ChangeStream::FilterTableCallback(void* pCtx, Utf8CP tableName) {
    return (int)((ChangeStream*)pCtx)->_FilterTable(tableName);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeStream::FromChangeTrack(ChangeTracker& session, ChangeSet::SetType setType) {
    if (ChangeSet::SetType::Full == setType)
        return (DbResult)sqlite3session_changeset_strm(session.GetSqlSession(), AppendCallback, this);

    return (DbResult)sqlite3session_patchset_strm(session.GetSqlSession(), AppendCallback, this);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeStream::FromChangeGroup(ChangeGroupCR changeGroup) {
    return (DbResult)sqlite3changegroup_output_strm((sqlite3_changegroup*)changeGroup.m_changegroup, AppendCallback, this);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeStream::AddToChangeGroup(ChangeGroup& changeGroup) {
    auto reader = _GetReader();
    return (DbResult)sqlite3changegroup_add_strm((sqlite3_changegroup*)changeGroup.m_changegroup, Changes::Reader::ReadCallback, (void*)reader.get());
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeStream::ToChangeSet(ChangeSet& changeSet, bool invert) {
    ChangeGroup changeGroup;
    DbResult result = AddToChangeGroup(changeGroup);
    if (result != BE_SQLITE_OK)
        return result;

    result = changeSet.FromChangeGroup(changeGroup);
    if (result != BE_SQLITE_OK)
        return result;

    if (!changeSet.IsValid())
        return BE_SQLITE_OK; // Empty change stream

    return invert ? changeSet.Invert() : BE_SQLITE_OK;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeStream::ApplyChanges(DbR db, Rebase* rebase, bool invert) const
    {
    int flags = SQLITE_CHANGESETAPPLY_NOSAVEPOINT;
    if (invert)
        flags |= SQLITE_CHANGESETAPPLY_INVERT;

    auto reader = _GetReader();
    DbResult result = (DbResult) sqlite3changeset_apply_v2_strm(db.GetSqlDb(), Changes::Reader::ReadCallback, (void*) reader.get(), FilterTableCallback, ConflictCallback, (void*) this,
        rebase ? &rebase->m_data : nullptr, rebase ? &rebase->m_size : nullptr, flags);
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeStream::ReadFrom(Changes::Reader& inStream)  {
    DbResult result = BE_SQLITE_OK;
    Byte buffer[STREAM_PAGE_BYTE_SIZE];
    do
        {
        int numBytes = STREAM_PAGE_BYTE_SIZE;
        result = (DbResult) Changes::Reader::ReadCallback(&inStream, buffer, &numBytes);
        if (result == BE_SQLITE_OK)
            {
            if (0 == numBytes)
                break; // Done!!

            result = (DbResult) _Append(buffer, numBytes);
            }
        } while (result == BE_SQLITE_OK);

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeStream::InvertFrom(Changes::Reader& reader) {
    return (DbResult)sqlite3changeset_invert_strm(Changes::Reader::ReadCallback, &reader, AppendCallback, this);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeStream::FromConcatenatedChangeStreams(ChangeStream const& inStream1, ChangeStream const& inStream2) {
    auto reader1 = inStream1._GetReader();
    auto reader2 = inStream2._GetReader();

    return (DbResult)sqlite3changeset_concat_strm(Changes::Reader::ReadCallback, reader1.get(), Changes::Reader::ReadCallback, reader2.get(), AppendCallback, this);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeSet::ConcatenateWith(ChangeSet const& second)
    {
    auto saved = std::move(*this);
    return FromConcatenatedChangeStreams(saved, second);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Rebaser::Rebaser() { sqlite3rebaser_create(&m_rebaser); }
Rebaser::~Rebaser() { sqlite3rebaser_delete(m_rebaser); }
DbResult Rebaser::AddRebase(Rebase const& rebase) { return (DbResult)sqlite3rebaser_configure(m_rebaser, rebase.GetSize(), rebase.GetData()); }
DbResult Rebaser::AddRebase(void const* data, int count) { return (DbResult)sqlite3rebaser_configure(m_rebaser, count, data); }
DbResult Rebaser::DoRebase(ChangeStream const& in, ChangeStream& out) {
    auto reader = in._GetReader();
    return (DbResult)sqlite3rebaser_rebase_strm(m_rebaser, Changes::Reader::ReadCallback, reader.get(), out.AppendCallback, &out);
}
Rebase::~Rebase() {
    if (m_data) BeSQLiteLib::FreeMem(m_data);
}
