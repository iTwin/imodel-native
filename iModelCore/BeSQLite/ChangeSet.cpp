/*--------------------------------------------------------------------------------------+
|
|     $Source: ChangeSet.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BeSQLite/ChangeSet.h>
#include <BeSQLite/SQLiteAPI.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE

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

void ChangeTracker::SetIndirectChanges(bool yesNo) {sqlite3session_indirect(m_session, yesNo);}
bool ChangeTracker::HasChanges() {return m_session && 0 == sqlite3session_isempty(m_session);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeSet::FromChangeTrack(ChangeTracker& session, SetType setType)
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
static int conflictCallback(void *pCtx, int cause, SqlChangesetIterP iter) {return (int) ((ChangeSet*) pCtx)->_OnConflict((ChangeSet::ConflictCause) cause, Changes::Change(iter, true));}
static int filterTableCallback(void *pCtx, Utf8CP tableName) {return (int) ((ChangeSet*) pCtx)->_FilterTable(tableName);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeSet::ApplyChanges(DbR db)
    {
    return (DbResult) sqlite3changeset_apply(db.GetSqlDb(), m_size, m_changeset, filterTableCallback, conflictCallback, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeSet::ConcatenateWith(ChangeSet const& second)
    {
    int outSize=0;
    void* outData=nullptr;

    DbResult rc = (DbResult) sqlite3changeset_concat(GetSize(), (void*) GetData(), second.GetSize(), (void*) second.GetData(), &outSize, &outData);
    if (BE_SQLITE_OK != rc)
        return rc;

    sqlite3_free(m_changeset);
    m_changeset = outData;
    m_size = outSize;
    return BE_SQLITE_OK;
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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
Changes::Change Changes::begin() const
    {
    Finalize();

    if (m_changeset.IsValid())
        sqlite3changeset_start(&m_iter, m_changeset.GetSize(),(void*) m_changeset.GetData());

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

BEGIN_UNNAMED_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String toHex(Byte* buf, size_t nb, size_t w)
    {
    Utf8String hd;
    size_t i, n = std::min(nb, w);
    unsigned char c;
    static Utf8Char hxdg[] = "0123456789abcdef";

    if (!buf)
        return "<null>";

    for (i=0; i<n; ++i)
        {
        if (i && i%4==0)
            hd.append(" ");

        c = buf[i];
        hd.append(1, hxdg[c>>4]);
        hd.append(1, hxdg[0xf&c]);
        }

    for (   ; i<w; ++i)
        {
        if (i && i%4==0)
            hd.append(" ");

        hd.append(" ");
        hd.append(" ");
        }

    return hd;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String toAsc(Byte* buf, size_t nb, size_t w)
    {
    Utf8String ad;
    size_t i, n = std::min(nb, w);
    for (i=0; i<n; ++i)
        if (isprint((int)buf[i]))
            ad.append(1, (Utf8Char)buf[i]);
        else
            ad.append(".");

    ad.append((w-i), ' ');
    return ad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2005
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String hexDump(Byte* bytes, size_t nbytes)
    {
    Utf8String output;
    size_t const nperline = 32;

    Byte* nextBytes = bytes;
    Byte* bytesX    = bytes + nbytes;

    size_t nlines = nbytes/nperline;
    for (size_t i=0; i<nlines; ++i)
        {
        output.append("\n");
        output.append(toHex(nextBytes, nperline, nperline));
        output.append(" |");
        output.append(toAsc(nextBytes, nperline, nperline));
        output.append(" |");
        nextBytes += nperline;
        }

    int nrem = static_cast<int>(bytesX - nextBytes);
    if (0 != nrem)
        {
        output.append("\n");
        output.append(toHex(nextBytes, nrem, nperline));
        output.append(" |");
        output.append(toAsc(nextBytes, nrem, nperline));
        output.append(" |");
        }

    output.append("\n");

    return output;
    }
END_UNNAMED_NAMESPACE

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
            return Utf8PrintfString("%lg", GetValueDouble());

        case DbValueType::TextVal:
            return Utf8PrintfString("\"%s\"", GetValueText());

        case DbValueType::BlobVal:
            if (detailLevel < 1)
                return "...";
            return Utf8PrintfString(hexDump((Byte*)GetValueBlob(), GetValueBytes()).c_str());

        case DbValueType::NullVal:
            return "NULL";
        }

    BeAssert(false);
    return "?";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void Changes::Change::DumpColumns(int startCol, int endCol, Stage stage, bvector<Utf8String> const& columns, int detailLevel) const
    {
    Byte* pcols;
    int npcols;
    GetPrimaryKeyColumns(&pcols, &npcols);

    int nprinted = 0;
    for (int i=startCol; i <= endCol; ++i)
        {
        if (std::find(pcols, pcols+npcols, (Byte)i) != pcols+npcols)    // we print the old value of the (unchanging) primary key columns separately
            continue;

        auto v = GetValue(i, stage);
        if (!v.IsValid() || v.IsNull())
            continue;

        if (nprinted != 0)
            printf(" ");

        printf("%s=", columns[i].c_str());

        if (v.IsValid())
            printf("%s", v.Format(detailLevel).c_str());

        ++nprinted;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Changes::Change::FormatPrimarykeyColumns(bool isInsert, int detailLevel) const
    {
    Byte* pcols;
    int npcols;
    GetPrimaryKeyColumns(&pcols, &npcols);

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
        printf("\nTable: %s", tableName);
        if (detailLevel > 0)
            printf("\n");
        tablesSeen.insert(tableName);
        }

    printf("\nkey=%s ", FormatPrimarykeyColumns((DbOpcode::Insert==opcode), detailLevel).c_str());

    bvector<Utf8String> columnNames;
    db.GetColumns(columnNames, tableName);

    switch (opcode)
        {
        case DbOpcode::Delete:
            printf("DELETE ");
            if (detailLevel > 0)
                printf("\n");
            DumpColumns(0, nCols-1, Stage::Old, columnNames, detailLevel);
            break;
        case DbOpcode::Insert:
            printf("INSERT ");
            if (detailLevel > 0)
                printf("\n");
            DumpColumns(0, nCols-1, Stage::New, columnNames, detailLevel);
            break;
        case DbOpcode::Update:
            printf("UPDATE ");
            if (detailLevel > 0)
                printf("\n");
            if (!isPatchSet)
                {
                printf("old: ");
                DumpColumns(0, nCols-1, Stage::Old, columnNames, detailLevel);
                printf("\nnew: ");
                }
            DumpColumns(0, nCols-1, Stage::New, columnNames, detailLevel);
            break;

        default:
            BeAssert(false);
        }
    if (detailLevel > 0)
        printf("\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeSet::Dump(Utf8CP label, Db const& db, bool isPatchSet, int detailLevel) const
    {
    if (label)
        printf("%s", label);

    bset<Utf8String> tablesSeen;

    Changes changes(*const_cast<ChangeSet*>(this));
    for (auto& change : changes)
        {
        change.Dump(db, isPatchSet, tablesSeen, detailLevel);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ChangeSet::InterpretConflictCause(ChangeSet::ConflictCause cause)
    {
    switch (cause)
        {
        case ChangeSet::ConflictCause::Data: return "data";
        case ChangeSet::ConflictCause::NotFound: return "not found";
        case ChangeSet::ConflictCause::Conflict: return "conflict";
        case ChangeSet::ConflictCause::Constraint: return "constraint";
        case ChangeSet::ConflictCause::ForeignKey: return "foreign key";
        }
    BeAssert(false);
    return "?";
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
    return (DbResult) sqlite3changegroup_add((sqlite3_changegroup*) m_changegroup, size, (void*) data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ChangeSet::FromChangeGroup(ChangeGroup& changegroup)
    {
    return (DbResult) sqlite3changegroup_output((sqlite3_changegroup*) changegroup.m_changegroup, &m_size, &m_changeset);
    }
