/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnCodesManager.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnChangeSummary.h>

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct UnrestrictedCodesManager : IDgnCodesManager
{
private:
    UnrestrictedCodesManager(DgnDbR db) : IDgnCodesManager(db) { }

    virtual Response _ReserveCodes(Request&) override { return Response(CodeStatus::Success); }
    virtual CodeStatus _ReleaseCodes(DgnCodeSet const&) override { return CodeStatus::Success; }
    virtual CodeStatus _RelinquishCodes() override { return CodeStatus::Success; }
    virtual CodeStatus _ReserveCode(DgnCodeCR) override { return CodeStatus::Success; }
    virtual CodeStatus _RefreshCodes() override { return CodeStatus::Success; }
    virtual CodeStatus _OnFinishRevision(DgnRevision const& rev) override { return CodeStatus::Success; }
    virtual CodeStatus _QueryCodeStates(DgnCodeInfoSet& states, DgnCodeSet const& codes) override
        {
        states.clear();
        auto bcId = GetDgnDb().GetBriefcaseId();
        for (auto const& code : codes)
            states.insert(DgnCodeInfo(code)).first->SetReserved(bcId);

        return CodeStatus::Success;
        }
public:
    static IDgnCodesManagerPtr Create(DgnDbR db) { return new UnrestrictedCodesManager(db); }
};

#define LOCAL_Table "Codes"
#define LOCAL_AuthorityId "AuthorityId"
#define LOCAL_NameSpace "NameSpace"
#define LOCAL_Value "Value"
#define LOCAL_Columns LOCAL_AuthorityId "," LOCAL_NameSpace "," LOCAL_Value
#define LOCAL_Values "(" LOCAL_Columns ")"
#define STMT_Insert "INSERT INTO " LOCAL_Table " " LOCAL_Values " Values (?,?,?)"
#define STMT_SelectInSet "SELECT " LOCAL_Columns " FROM " LOCAL_Table " WHERE InVirtualSet(@vset," LOCAL_Columns ")"
#define STMT_DeleteInSet "DELETE FROM " LOCAL_Table " WHERE InVirtualSet(@vset," LOCAL_Columns ")"
enum Column { AuthorityId=0, NameSpace, Value };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocalCodesManager : IDgnCodesManager
{
private:
    enum class DbState { New, Ready, Invalid };

    DbState m_dbState;

    DbR GetLocalDb() { return GetDgnDb().GetLocalStateDb().GetDb(); }

    LocalCodesManager(DgnDbR db) : IDgnCodesManager(db), m_dbState(DbState::New) { }

    virtual Response _ReserveCodes(Request&) override;
    virtual CodeStatus _ReleaseCodes(DgnCodeSet const&) override;
    virtual CodeStatus _RelinquishCodes() override;
    virtual CodeStatus _QueryCodeStates(DgnCodeInfoSet& states, DgnCodeSet const& codes) override;
    virtual CodeStatus _RefreshCodes() override;
    virtual CodeStatus _OnFinishRevision(DgnRevision const& rev) override;

    bool Validate(CodeStatus* status=nullptr);
    void Insert(DgnCodeSet const& codes);
    void Cull(DgnCodeSet& codes);

    DbResult Save() { return GetLocalDb().SaveChanges(); }
    CodeStatus Remove(DgnCodeSet const& codes);
public:
    static IDgnCodesManagerPtr Create(DgnDbR db) { return new LocalCodesManager(db); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool LocalCodesManager::Validate(CodeStatus* pStatus)
    {
    CodeStatus localStatus = CodeStatus::Success;
    CodeStatus& status = nullptr != pStatus ? *pStatus : localStatus;

    switch (m_dbState)
        {
        case DbState::Ready:    return true;
        case DbState::Invalid:  return false;
        }

    m_dbState = DbState::Invalid;
    status = CodeStatus::SyncError;

    DgnDb::LocalStateDb& localState = GetDgnDb().GetLocalStateDb();
    if (!localState.IsValid())
        return false;

    DbResult result = localState.GetDb().CreateTable(LOCAL_Table,
                                                     LOCAL_AuthorityId " INTEGER,"
                                                     LOCAL_NameSpace " TEXT,"
                                                     LOCAL_Value " TEXT,"
                                                     "PRIMARY KEY(" LOCAL_AuthorityId "," LOCAL_NameSpace "," LOCAL_Value ")");
    if (BE_SQLITE_OK != result)
        return false;

    DgnCodeSet codes;
    auto server = GetCodesServer();
    status = nullptr != server ? server->QueryCodes(codes, GetDgnDb()) : CodeStatus::ServerUnavailable;
    if (CodeStatus::Success != status)
        return false;

    if (!codes.empty())
        Insert(codes);

    m_dbState = DbState::Ready;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodeStatus LocalCodesManager::_RefreshCodes()
    {
    CodeStatus status;
    if (DbState::Ready != m_dbState)
        {
        Validate(&status);
        return status;
        }

    auto server = GetCodesServer();
    if (nullptr == server)
        return CodeStatus::ServerUnavailable;

    DgnCodeSet codes;
    status = server->QueryCodes(codes, GetDgnDb());
    if (CodeStatus::Success != status)
        return status;

    if (BE_SQLITE_OK != GetLocalDb().ExecuteSql("DELETE FROM " LOCAL_Table))
        return CodeStatus::SyncError;

    if (!codes.empty())
        Insert(codes);

    Save();
    return CodeStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void LocalCodesManager::Insert(DgnCodeSet const& codes)
    {
    CachedStatementPtr stmt = GetLocalDb().GetCachedStatement(STMT_Insert);
    for (auto const& code : codes)
        {
        if (code.IsEmpty())
            {
            BeAssert(false);
            continue;
            }

        stmt->BindId(Column::AuthorityId+1, code.GetAuthority());
        stmt->BindText(Column::NameSpace+1, code.GetNamespace(), Statement::MakeCopy::No);
        stmt->BindText(Column::Value+1, code.GetValue(), Statement::MakeCopy::No);
        stmt->Step();
        }

    Save();
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct VirtualCodeSet : VirtualSet
{
    DgnCodeSet const& m_codes;

    VirtualCodeSet(DgnCodeSet const& codes) : m_codes(codes) { }

    virtual bool _IsInSet(int nVals, DbValue const* vals) const override
        {
        return m_codes.end() != std::find_if(m_codes.begin(), m_codes.end(), [&](DgnCode const& arg)
            {
            return arg.GetAuthority().GetValueUnchecked() == vals[Column::AuthorityId].GetValueUInt64()
                && arg.GetNamespace().Equals(vals[Column::NameSpace].GetValueText())
                && arg.GetValue().Equals(vals[Column::Value].GetValueText());
            });
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void LocalCodesManager::Cull(DgnCodeSet& codes)
    {
    // Don't bother asking server to reserve codes which we've already reserved...
    VirtualCodeSet vset(codes);
    CachedStatementPtr stmt = GetLocalDb().GetCachedStatement(STMT_SelectInSet);
    stmt->BindVirtualSet(1, vset);
    while (BE_SQLITE_ROW == stmt->Step())
        {
        DgnCode code(stmt->GetValueId<DgnAuthorityId>(Column::AuthorityId), stmt->GetValueText(Column::NameSpace), stmt->GetValueText(Column::Value));
        codes.erase(code);
        }

    // Don't bother asking server to reserve empty codes...
    for (auto iter = codes.begin(); iter != codes.end(); /* */)
        {
        BeAssert(!iter->IsEmpty());
        if (iter->IsEmpty())
            iter = codes.erase(iter);
        else
            ++iter;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
IDgnCodesManager::Response LocalCodesManager::_ReserveCodes(Request& req)
    {
    if (!Validate())
        return Response(CodeStatus::SyncError);

    Cull(req);
    if (req.empty())
        return Response(CodeStatus::Success);

    auto server = GetCodesServer();
    if (nullptr == server)
        return Response(CodeStatus::ServerUnavailable);

    auto response = server->ReserveCodes(req, GetDgnDb());
    if (CodeStatus::Success == response.GetResult())
        Insert(req);
        
    return response;
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct CodeReleaseContext
{
private:
    DgnDbR              m_db;
    CodeStatus          m_status;
    DgnCodeSet          m_used;
    TxnManager::TxnId   m_endTxnId;
public:
    explicit CodeReleaseContext(DgnDbR db);

    CodeStatus GetStatus() const { return m_status; }
    DgnCodeSet const& GetUsedCodes() const { return m_used; }

    CodeStatus ClearTxns();
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodeReleaseContext::CodeReleaseContext(DgnDbR db) : m_db(db), m_status(CodeStatus::CannotCreateRevision)
    {
    TxnManager& txns = db.Txns();
    if (txns.HasChanges() || txns.IsInDynamics())
        {
        m_status = CodeStatus::PendingTransactions;
        return;
        }

    RevisionStatus revStatus;
    DgnRevisionPtr rev = db.Revisions().StartCreateRevision(&revStatus, DgnRevision::Include::Codes);
    if (rev.IsValid())
        {
        rev->ExtractAssignedCodes(m_used);
        m_status = CodeStatus::Success;
        m_endTxnId = db.Revisions().GetCurrentRevisionEndTxnId();

        db.Revisions().AbandonCreateRevision();
        }
    else if (RevisionStatus::NoTransactions == revStatus)
        {
        m_status = CodeStatus::Success;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodeStatus CodeReleaseContext::ClearTxns()
    {
    if (CodeStatus::Success == m_status)
        m_db.Txns().DeleteReversedTxns();

    return m_status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodeStatus LocalCodesManager::_RelinquishCodes()
    {
    IDgnCodesServerP server;
    if (!Validate())
        return CodeStatus::SyncError;
    else if (nullptr == (server = GetCodesServer()))
        return CodeStatus::ServerUnavailable;

    CodeReleaseContext context(GetDgnDb());
    if (CodeStatus::Success != context.GetStatus())
        return context.GetStatus();
    else if (!context.GetUsedCodes().empty())
        return CodeStatus::CodeUsed;

    if (BE_SQLITE_OK != GetLocalDb().ExecuteSql("DELETE FROM " LOCAL_Table) || BE_SQLITE_OK != Save())
        return CodeStatus::SyncError;

    auto status = server->RelinquishCodes(GetDgnDb());
    if (CodeStatus::Success == status)
        status = context.ClearTxns();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodeStatus LocalCodesManager::_ReleaseCodes(DgnCodeSet const& req)
    {
    if (!Validate())
        return CodeStatus::SyncError;

    IDgnCodesServerP server;
    if (!Validate())
        return CodeStatus::SyncError;
    else if (nullptr == (server = GetCodesServer()))
        return CodeStatus::ServerUnavailable;

    CodeReleaseContext context(GetDgnDb());
    if (CodeStatus::Success != context.GetStatus())
        return context.GetStatus();

    for (auto const& usedCode : context.GetUsedCodes())
        {
        auto iter = req.find(usedCode);
        if (iter != req.end())
            return CodeStatus::CodeUsed;
        }

    auto status = server->ReleaseCodes(req, GetDgnDb());
    if (CodeStatus::Success == status)
        {
        Remove(req);
        context.ClearTxns();
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodeStatus LocalCodesManager::_QueryCodeStates(DgnCodeInfoSet& states, DgnCodeSet const& codes)
    {
    auto server = GetCodesServer();
    return nullptr != server ? server->QueryCodeStates(states, codes) : CodeStatus::ServerUnavailable;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodeStatus LocalCodesManager::_OnFinishRevision(DgnRevision const& rev)
    {
    // Any codes which became Used as a result of these changes must necessarily have been Reserved by this briefcase,
    // and are now no longer Reserved by any briefcase
    // (Any codes which became Discarded were necessarily previously Used, therefore no local state needs to be updated for them).
    if (rev.GetAssignedCodes().empty())
        return CodeStatus::Success;
    else if (!Validate())
        return CodeStatus::SyncError;
    else
        return Remove(rev.GetAssignedCodes());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodeStatus LocalCodesManager::Remove(DgnCodeSet const& codes)
    {
    VirtualCodeSet vset(codes);
    CachedStatementPtr stmt = GetLocalDb().GetCachedStatement(STMT_DeleteInSet);
    stmt->BindVirtualSet(1, vset);
    if (BE_SQLITE_OK != stmt->Step())
        return CodeStatus::SyncError;

    Save();
    return CodeStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* We still don't have a server. Some apps apparently use briefcase IDs other than zero
* (ConceptStation). Therefore, always use the unrestricted codes manager until we have
* an actual server implementation; or while explicitly enabled for tests.
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool s_enableCodeManager = false;
void IDgnCodesManager::BackDoor_SetEnabled(bool enable) { s_enableCodeManager = enable; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
IDgnCodesManagerPtr DgnPlatformLib::Host::ServerAdmin::_CreateCodesManager(DgnDbR db) const
    {
    return (db.IsMasterCopy() || !s_enableCodeManager) ? UnrestrictedCodesManager::Create(db) : LocalCodesManager::Create(db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
IDgnCodesServerP IDgnCodesManager::GetCodesServer() const
    {
    return T_HOST.GetServerAdmin()._GetCodesServer(GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodeStatus IDgnCodesManager::_ReserveCode(DgnCodeCR code)
    {
    if (code.IsEmpty())
        return CodeStatus::Success;

    Request req;
    req.insert(code);
    return _ReserveCodes(req).GetResult();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodeStatus IDgnCodesManager::QueryCodeState(DgnCodeStateR state, DgnCodeCR code)
    {
    if (code.IsEmpty())
        {
        state.SetAvailable();
        return CodeStatus::Success;
        }

    DgnCodeSet codes;
    codes.insert(code);
    DgnCodeInfoSet states;

    auto status = _QueryCodeStates(states, codes);
    if (CodeStatus::Success == status)
        {
        BeAssert(1 == states.size());
        state = *states.begin();
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodeStatus IDgnCodesServer::QueryCodeState(DgnCodeStateR state, DgnCodeCR code)
    {
    if (code.IsEmpty())
        {
        state.SetAvailable();
        return CodeStatus::Success;
        }

    DgnCodeSet codes;
    codes.insert(code);
    DgnCodeInfoSet states;

    auto status = QueryCodeStates(states, codes);
    if (CodeStatus::Success == status)
        {
        BeAssert(1 == states.size());
        state = *states.begin();
        }

    return status;
    }

