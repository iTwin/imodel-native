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
    virtual Response _ReleaseCodes(Request const&) override { return Response(CodeStatus::Success); }
    virtual CodeStatus _RelinquishCodes() override { return CodeStatus::Success; }
    virtual CodeStatus _ReserveCode(DgnCodeCR) override { return CodeStatus::Success; }
    virtual CodeStatus _RefreshCodes() override { return CodeStatus::Success; }
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

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocalCodesManager : IDgnCodesManager
{
private:
    enum class DbState { New, Ready, Invalid };
    enum Column { AuthorityId=0, NameSpace, Value };

    DbState m_dbState;

    DbR GetLocalDb() { return GetDgnDb().GetLocalStateDb().GetDb(); }

    LocalCodesManager(DgnDbR db) : IDgnCodesManager(db), m_dbState(DbState::New) { }

    virtual Response _ReserveCodes(Request&) override;
    virtual Response _ReleaseCodes(Request const&) override;
    virtual CodeStatus _RelinquishCodes() override;
    virtual CodeStatus _QueryCodeStates(DgnCodeInfoSet& states, DgnCodeSet const& codes) override;
    virtual CodeStatus _RefreshCodes() override;

    bool Validate(CodeStatus* status=nullptr);
    void Insert(DgnCodeSet const& codes);
    void Cull(DgnCodeSet& codes);

    DbResult Save() { return GetLocalDb().SaveChanges(); }
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
        stmt->BindId(Column::AuthorityId+1, code.GetAuthority());
        stmt->BindText(Column::NameSpace+1, code.GetNamespace(), Statement::MakeCopy::No);
        stmt->BindText(Column::Value+1, code.GetValue(), Statement::MakeCopy::No);
        stmt->Step();
        }

    Save();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void LocalCodesManager::Cull(DgnCodeSet& codes)
    {
    struct VSet : VirtualSet
        {
        DgnCodeSet const& m_codes;
        VSet(DgnCodeSet const& codes) : m_codes(codes) { }
        virtual bool _IsInSet(int nVals, DbValue const* vals) const override
            {
            DgnCode code(DgnAuthorityId(vals[Column::AuthorityId].GetValueUInt64()), vals[Column::NameSpace].GetValueText(), vals[Column::Value].GetValueText());
            return m_codes.end() != m_codes.find(code);
            }
        };

    VSet vset(codes);
    CachedStatementPtr stmt = GetLocalDb().GetCachedStatement(STMT_SelectInSet);
    stmt->BindVirtualSet(1, vset);
    while (BE_SQLITE_ROW == stmt->Step())
        {
        DgnCode code(stmt->GetValueId<DgnAuthorityId>(Column::AuthorityId), stmt->GetValueText(Column::NameSpace), stmt->GetValueText(Column::Value));
        codes.erase(code);
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
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
IDgnCodesManager::Response LocalCodesManager::_ReleaseCodes(Request const& req)
    {
    return Response(CodeStatus::SyncError); // ###TODO: Verify none used, etc...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodeStatus LocalCodesManager::_RelinquishCodes()
    {
    return CodeStatus::SyncError; // ###TODO: Verify none used, etc...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodeStatus LocalCodesManager::_QueryCodeStates(DgnCodeInfoSet& states, DgnCodeSet const& codes)
    {
    return CodeStatus::SyncError;   // ###TODO
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
    Request req;
    req.insert(code);
    return _ReserveCodes(req).GetResult();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodeStatus IDgnCodesManager::QueryCodeState(DgnCodeStateR state, DgnCodeCR code)
    {
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

