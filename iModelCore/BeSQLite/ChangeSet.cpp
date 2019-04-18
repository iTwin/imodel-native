/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <BeSQLite/ChangeSet.h>
#include <Logging/bentleylogging.h>

#define SQLITE_ENABLE_SESSION 1
#define SQLITE_ENABLE_PREUPDATE_HOOK 1

#include "SQLite/sqlite3.h"

#define STREAM_PAGE_BYTE_SIZE 64 * 1024
#define LOG (*NativeLogging::LoggingManager::GetLogger(L"BeSQLite"))
#define LOGCHANGESET (*NativeLogging::LoggingManager::GetLogger(L"Changeset"))

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Ramanujam.Raman                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeTracker::RecordDbSchemaChange(Utf8CP ddl)
    {
    if (!IsTracking())
        return BE_SQLITE_OK;

    m_dbSchemaChanges.AddDDL(ddl);
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Ramanujam.Raman                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DbSchemaChangeSet::AddDDL(Utf8CP ddl)
    {
    if (Utf8String::IsNullOrEmpty(ddl))
        {
        BeAssert(false);
        return;
        }

    if (!m_ddl.empty())
        m_ddl.append(";");

    m_ddl.append(ddl);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Ramanujam.Raman                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DbSchemaChangeSet::Dump(Utf8CP label) const
    {
    if (label)
        LOGCHANGESET.infov("%s", label);

    if (IsEmpty())
        {
        LOGCHANGESET.info("Empty");
        return;
        }

    bvector<Utf8String> tokens;
    BeStringUtilities::Split(m_ddl.c_str(), ";", tokens);
    for (Utf8StringCR str : tokens)
        {
        LOGCHANGESET.info(str.c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
static int filterCaller(void* tracker, Utf8CP tableName) {return (int) ((ChangeTracker*) tracker)->_FilterTable(tableName);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/11
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

    if (BE_SQLITE_OK == result)
        sqlite3session_table_filter(m_session, filterCaller, this); // set up auto-attach for all tables

    m_isTracking = true; // new sessions are on by default

    return  result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeTracker::EndTracking()
    {
    if (m_session)
        {
        sqlite3session_delete(m_session);
        m_session = nullptr;
        }
    m_dbSchemaChanges.Clear();
    m_isTracking = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChangeTracker::EnableTracking(bool yesNo)
    {
    CreateSession();
    if (m_isTracking == yesNo)
        return m_isTracking;

    sqlite3session_enable(m_session, yesNo);
    m_isTracking = yesNo;
    return !yesNo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeTracker::Mode ChangeTracker::GetMode() const
    {
    return nullptr != m_session && 0 != sqlite3session_indirect(m_session, -1) ? Mode::Indirect : Mode::Direct;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeTracker::SetMode(Mode mode)
    {
    if (nullptr != m_session)
        sqlite3session_indirect(m_session, static_cast<int>(mode));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Ramanujam.Raman                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChangeTracker::HasDataChanges() const 
    { 
    return m_session && 0 == sqlite3session_isempty(m_session); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Ramanujam.Raman                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChangeTracker::HasChanges() const 
    { 
    return HasDataChanges() || HasDbSchemaChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeSet::_FromChangeTrack(ChangeTracker& session, SetType setType)
    {
    BeAssert(!IsValid());
    return (setType == SetType::Full) ? (DbResult) sqlite3session_changeset(session.GetSqlSession(), &m_size, &m_changeset) :
                                        (DbResult) sqlite3session_patchset(session.GetSqlSession(), &m_size, &m_changeset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
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
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeSet::FromData(int size, void const* data, bool invert)
    {
    if (IsValid())
        {
        BeAssert(false);
        return  BE_SQLITE_ERROR;
        }

    if (invert)
        return (DbResult) sqlite3changeset_invert(size, data, &m_size, &m_changeset);

    m_size = size;
    m_changeset = sqlite3_malloc(size);
    memcpy(m_changeset, data, size);

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeSet::Invert()
    {
    if (!IsValid())
        {
        BeAssert(false);
        return  BE_SQLITE_ERROR;
        }

    int size   = m_size;
    void* data = m_changeset;
    m_size = 0;
    m_changeset = nullptr;
    DbResult rc = (DbResult) sqlite3changeset_invert(size, data, &m_size, &m_changeset);
    sqlite3_free(data);
    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeSet::Free()
    {
    if (nullptr == m_changeset)
        return;

    sqlite3_free(m_changeset);
    m_changeset = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
static int conflictCallback(void *pCtx, int cause, SqlChangesetIterP iter) {return (int) ((ChangeSet*) pCtx)->OnConflict((ChangeSet::ConflictCause) cause, Changes::Change(iter, true));}
static int filterTableCallback(void *pCtx, Utf8CP tableName) {return (int) ((ChangeSet*) pCtx)->FilterTable(tableName);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeSet::_ApplyChanges(DbR db, Rebase* rebase, bool invert)
    {
    int flags = SQLITE_CHANGESETAPPLY_NOSAVEPOINT;
    if (invert) 
        flags |= SQLITE_CHANGESETAPPLY_INVERT;

    return (DbResult) sqlite3changeset_apply_v2(db.GetSqlDb(), m_size, m_changeset, filterTableCallback, conflictCallback, this, 
        rebase ? &rebase->m_data : nullptr, rebase ? &rebase->m_size : nullptr, flags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeSet::ConcatenateWith(ChangeSet const& second)
    {
    int outSize=0;
    void* outData=nullptr;

    DbResult rc = (DbResult) sqlite3changeset_concat(GetSize(), const_cast<void*>(GetData()), second.GetSize(), const_cast<void*>(second.GetData()), &outSize, &outData);
    if (BE_SQLITE_OK != rc)
        return rc;

    sqlite3_free(m_changeset);
    m_changeset = outData;
    m_size = outSize;
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Ramanujam.Raman                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
Changes::Changes(ChangeSet const& changeSet, bool invert)
    {
    m_data = const_cast<void*>(changeSet.GetData());
    m_size = changeSet.GetSize();
    m_invert = invert;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void Changes::Finalize() const
    {
    if (nullptr == m_iter)
        return;

    sqlite3changeset_finalize(m_iter);
    m_iter = nullptr;

    if (nullptr != m_changeStream)
        m_changeStream->Reset();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
Changes::Change Changes::begin() const
    {
    Finalize();

    if (nullptr != m_data)
        sqlite3changeset_start_v2(&m_iter, m_size, m_data, m_invert ? SQLITE_CHANGESETSTART_INVERT : 0);

    if (nullptr != m_changeStream)
        sqlite3changeset_start_v2_strm(&m_iter, ChangeStream::InputCallback, (void*) m_changeStream, m_invert ? SQLITE_CHANGESETSTART_INVERT : 0);

    if (nullptr == m_iter)
        return Change(0, false);

    DbResult result = (DbResult) sqlite3changeset_next(m_iter);
    return  Change(m_iter, result==BE_SQLITE_ROW);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
Changes::~Changes() {Finalize();}
DbResult Changes::Change::GetOperation(Utf8CP* tableName, int* nCols, DbOpcode* opcode, int* indirect)const {return (DbResult) sqlite3changeset_op(m_iter, tableName, nCols, (int*) opcode, indirect);}
DbResult Changes::Change::GetPrimaryKeyColumns(Byte** cols, int* nCols) const {return (DbResult) sqlite3changeset_pk(m_iter, cols, nCols);}
DbResult Changes::Change::GetFKeyConflicts(int *nConflicts) const { return (DbResult)sqlite3changeset_fk_conflicts(m_iter, nConflicts); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbValue  Changes::Change::GetOldValue(int colNum) const
    {
    SqlValueP val=nullptr;
    int rc=sqlite3changeset_old(m_iter, colNum, &val);
    BeAssert(rc==BE_SQLITE_OK);
    UNUSED_VARIABLE(rc);
    return DbValue(val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbValue  Changes::Change::GetNewValue(int colNum) const
    {
    SqlValueP val=nullptr;
    int rc=sqlite3changeset_new(m_iter, colNum, &val);
    BeAssert(rc==BE_SQLITE_OK);
    UNUSED_VARIABLE(rc);
    return DbValue(val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
Changes::Change& Changes::Change::operator++()
    {
    m_isValid = (BE_SQLITE_ROW == (DbResult) sqlite3changeset_next(m_iter));
    return  *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
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
* @bsimethod                                    Sam.Wilson                      07/14
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
* @bsimethod                                  Ramanujam.Raman                   10/15
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
* @bsimethod                                    Sam.Wilson                      07/14
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeSet::Dump(Utf8CP label, Db const& db, bool isPatchSet, int detailLevel) const
    {
    if (label)
        LOGCHANGESET.infov("%s", label);

    bset<Utf8String> tablesSeen;

    Changes changes(*const_cast<ChangeSet*>(this), false);
    for (auto& change : changes)
        {
        change.Dump(db, isPatchSet, tablesSeen, detailLevel);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ChangeSet::InterpretConflictCause(ChangeSet::ConflictCause cause, int detailLevel)
    {
    bool brief = (detailLevel == 0);
    switch (cause)
        {
        case ChangeSet::ConflictCause::Data:
            return brief? "data": "Data (PRIMARY KEY found, but data was changed)";
        case ChangeSet::ConflictCause::NotFound:
            return brief? "not found": "Not Found (PRIMARY KEY)";
        case ChangeSet::ConflictCause::Conflict:
            return brief? "conflict": "Conflict (Causes duplicate PRIMARY KEY)";
        case ChangeSet::ConflictCause::Constraint:
            return brief? "constraint": "Constraint (Causes UNIQUE, CHECK or NOT NULL constraint violation)";
        case ChangeSet::ConflictCause::ForeignKey:
            return brief? "foreign key": "ForeignKey (Causes FOREIGN KEY constraint violation)";
        }
    BeAssert(false);
    return "?";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/18
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeGroup::ChangeGroup()  {sqlite3changegroup_new((sqlite3_changegroup**) &m_changegroup); }
ChangeGroup::~ChangeGroup() {sqlite3changegroup_delete((sqlite3_changegroup*) m_changegroup);}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeGroup::AddChanges(int size, void const* data)
    {
    return (DbResult) sqlite3changegroup_add((sqlite3_changegroup*) m_changegroup, size, const_cast<void*>(data));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeSet::_FromChangeGroup(ChangeGroupCR changegroup)
    {
    return (DbResult) sqlite3changegroup_output((sqlite3_changegroup*) changegroup.m_changegroup, &m_size, &m_changeset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Ramanujam.Raman                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
int ChangeStream::OutputCallback(void *pOut, const void *pData, int nData)
    {
    return (int) ((ChangeStream*) pOut)->_OutputPage(pData, nData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Ramanujam.Raman                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
int ChangeStream::InputCallback(void *pIn, void *pData, int *pnData)
    {
    return (int) ((ChangeStream*) pIn)->_InputPage(pData, pnData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Ramanujam.Raman                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
int ChangeStream::ConflictCallback(void *pCtx, int cause, SqlChangesetIterP iter)
    {
    return (int) ((ChangeStream*) pCtx)->_OnConflict((ChangeSet::ConflictCause) cause, Changes::Change(iter, true));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Ramanujam.Raman                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
int ChangeStream::FilterTableCallback(void *pCtx, Utf8CP tableName)
    {
    return (int) ((ChangeStream*) pCtx)->_FilterTable(tableName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Ramanujam.Raman                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeStream::_FromChangeTrack(ChangeTracker& session, ChangeSet::SetType setType)
    {
    DbResult result;
    if (ChangeSet::SetType::Full == setType)
        result = (DbResult) sqlite3session_changeset_strm(session.GetSqlSession(), OutputCallback, this);
    else
        result = (DbResult) sqlite3session_patchset_strm(session.GetSqlSession(), OutputCallback, this);

    _Reset();
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Ramanujam.Raman                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeStream::_FromChangeGroup(ChangeGroupCR changeGroup)
    {
    DbResult result = (DbResult) sqlite3changegroup_output_strm((sqlite3_changegroup*) changeGroup.m_changegroup, OutputCallback, this);
    _Reset();
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Ramanujam.Raman                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeStream::ToChangeGroup(ChangeGroup& changeGroup)
    {
    DbResult result = (DbResult) sqlite3changegroup_add_strm((sqlite3_changegroup*) changeGroup.m_changegroup, InputCallback, (void*) this);
    _Reset();
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Affan.Khan                        10/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChangeStream::_IsEmpty() const
    {
    Changes changes = const_cast<ChangeStream*>(this)->GetChanges(false); 
    return changes.begin() == changes.end(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Ramanujam.Raman                   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeStream::ToChangeSet(ChangeSet& changeSet, bool invert /*=false*/)
    {
    ChangeGroup changeGroup;
    DbResult result = ToChangeGroup(changeGroup);
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
* @bsimethod                                  Ramanujam.Raman                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeStream::_ApplyChanges(DbR db, Rebase* rebase, bool invert)
    {
    int flags = SQLITE_CHANGESETAPPLY_NOSAVEPOINT;
    if (invert) 
        flags |= SQLITE_CHANGESETAPPLY_INVERT;

    DbResult result = (DbResult) sqlite3changeset_apply_v2_strm(db.GetSqlDb(), InputCallback, (void*) this, FilterTableCallback, ConflictCallback, (void*) this,
        rebase ? &rebase->m_data : nullptr, rebase ? &rebase->m_size : nullptr, flags);
    _Reset();
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Ramanujam.Raman                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeStream::Dump(Utf8CP label, DbCR db, bool isPatchSet /*=false*/, int detailLevel/*=0*/)
    {
    ChangeGroup changeGroup;
    if (BE_SQLITE_OK == ToChangeGroup(changeGroup))
        {
        AbortOnConflictChangeSet changeSet;
        if (BE_SQLITE_OK == changeSet.FromChangeGroup(changeGroup))
            changeSet.Dump(label, db, isPatchSet, detailLevel);
        }

    _Reset();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Ramanujam.Raman                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeStream::TransferBytesBetweenStreams(ChangeStream& inStream, ChangeStream& outStream)
    {
    DbResult result = BE_SQLITE_OK;
    Byte buffer[STREAM_PAGE_BYTE_SIZE];
    do
        {
        int numBytes = STREAM_PAGE_BYTE_SIZE;
        result = (DbResult) InputCallback(&inStream, &buffer, &numBytes);
        if (result == BE_SQLITE_OK)
            {
            if (0 == numBytes)
                break; // Done!!

            result = (DbResult) OutputCallback(&outStream, &buffer, numBytes);
            }
        } while (result == BE_SQLITE_OK);

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Ramanujam.Raman                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeStream::FromChangeStream(ChangeStream& inStream, bool invert /* = false */)
    {
    DbResult result = BE_SQLITE_OK;
    if (invert)
        result = (DbResult) sqlite3changeset_invert_strm(InputCallback, &inStream, OutputCallback, this);
    else
        result = TransferBytesBetweenStreams(inStream, *this);

    inStream._Reset();
    _Reset();
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Ramanujam.Raman                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeStream::ToChangeStream(ChangeStream& outStream, bool invert /* = false */)
    {
    DbResult result = BE_SQLITE_OK;
    if (invert)
        result = (DbResult) sqlite3changeset_invert_strm(InputCallback, this, OutputCallback, &outStream);
    else
        result = TransferBytesBetweenStreams(*this, outStream);

    outStream._Reset();
    _Reset();
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Ramanujam.Raman                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeStream::FromConcatenatedChangeStreams(ChangeStream& inStream1, ChangeStream& inStream2)
    {
    DbResult result = (DbResult) sqlite3changeset_concat_strm(InputCallback, &inStream1, InputCallback, &inStream2, OutputCallback, this);

    inStream1._Reset();
    inStream2._Reset();
    _Reset();

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
Rebaser::Rebaser() {sqlite3rebaser_create(&m_rebaser);}
Rebaser::~Rebaser() {sqlite3rebaser_delete(m_rebaser);}
void Rebaser::AddRebase(Rebase const& rebase) {sqlite3rebaser_configure(m_rebaser, rebase.GetSize(), rebase.GetData());}
void Rebaser::AddRebase(void const* data, int count) {sqlite3rebaser_configure(m_rebaser, count, data);}
DbResult Rebaser::DoRebase(ChangeSet const&in, ChangeSet& out) {return (DbResult) sqlite3rebaser_rebase(m_rebaser, in.m_size, in.m_changeset, &out.m_size, &out.m_changeset);}
DbResult Rebaser::DoRebase(ChangeStream const& in, ChangeStream& out) {return (DbResult) sqlite3rebaser_rebase_strm(m_rebaser, in.InputCallback, (void*) &in, out.OutputCallback, &out);}
Rebase::~Rebase() {if (m_data) BeSQLiteLib::FreeMem(m_data);}
