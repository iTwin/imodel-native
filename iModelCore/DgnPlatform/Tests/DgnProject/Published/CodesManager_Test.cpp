/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/CodesManager_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <Bentley/BeTest.h>

USING_NAMESPACE_BENTLEY_SQLITE

#define SERVER_Table "Codes"
#define SERVER_Authority "Authority"
#define SERVER_NameSpace "NameSpace"
#define SERVER_Value "Value"
#define SERVER_State "State"
#define SERVER_Revision "Revision"
#define SERVER_Briefcase "Briefcase"


/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct CodesServer : IDgnCodesServer
{
    enum class CodeState : uint8_t { Available, Reserved, Discarded, Used };
private:
    Db  m_db;

    virtual Response _ReserveCodes(Request const&, DgnDbR) override;
    virtual CodeStatus _ReleaseCodes(DgnCodeSet const&, DgnDbR) override;
    virtual CodeStatus _RelinquishCodes(DgnDbR) override;
    virtual CodeStatus _QueryCodeStates(DgnCodeInfoSet&, DgnCodeSet const&) override;
    virtual CodeStatus _QueryCodes(DgnCodeSet&, DgnDbR) override;
public:
    CodesServer();
};

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
            return arg.GetAuthority().GetValueUnchecked() == vals[0].GetValueUInt64()
                && arg.GetNamespace().Equals(vals[1].GetValueText())
                && arg.GetValue().Equals(vals[2].GetValueText());
            });
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodesServer::CodesServer()
    {
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    BeSQLiteLib::Initialize(tempDir);

    DbResult result = m_db.CreateNewDb(BEDB_MemoryDb);
    if (BE_SQLITE_OK == result)
        {
        result = m_db.CreateTable(SERVER_Table,
                SERVER_Authority " INTEGER,"
                SERVER_NameSpace " TEXT,"
                SERVER_Value " TEXT,"
                SERVER_State " INTEGER,"
                SERVER_Revision " TEXT,"
                SERVER_Briefcase " INTEGER,"
                "PRIMARY KEY(" SERVER_Authority "," SERVER_NameSpace "," SERVER_Value ")");
        }

    BeAssert(BE_SQLITE_OK == result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodesServer::Response CodesServer::_ReserveCodes(Request const& req, DgnDbR db)
    {
    VirtualCodeSet vset(req);
    Statement stmt;
    stmt.Prepare(m_db, "SELECT " SERVER_Authority "," SERVER_NameSpace "," SERVER_Value "," SERVER_State "," SERVER_Revision "," SERVER_Briefcase
                    "   FROM " SERVER_Table " WHERE InVirtualSet(@vset, " SERVER_Authority "," SERVER_NameSpace "," SERVER_Value ")");

    CodeStatus status = CodeStatus::Success;
    Response response(status);
    bool wantInfos = ResponseOptions::IncludeState == (req.GetOptions() & ResponseOptions::IncludeState);
    while (BE_SQLITE_ROW == stmt.Step())
        {
        DgnCode code(stmt.GetValueId<DgnAuthorityId>(0), stmt.GetValueText(2), stmt.GetValueText(1));
        switch (static_cast<CodeState>(stmt.GetValueInt(4)))
            {
            case CodeState::Reserved:
                {
                BeBriefcaseId owner(static_cast<uint32_t>(stmt.GetValueInt(5)));
                if (owner.GetValue() != db.GetBriefcaseId().GetValue())
                    {
                    status = CodeStatus::CodeUnavailable;
                    if (wantInfos)
                        {
                        DgnCodeInfo info(code);
                        info.SetReserved(owner);
                        response.GetDetails().insert(info);
                        }
                    }
                break;
                }
            case CodeState::Used:
                status = CodeStatus::CodeUnavailable;
                if (wantInfos)
                    {
                    DgnCodeInfo info(code);
                    info.SetUsed(stmt.GetValueText(4));
                    response.GetDetails().insert(info);
                    }
                break;
            case CodeState::Discarded:
                // ###TODO: Check if briefcase has pulled the required revision...
                break;
            default:
                BeAssert(false);
                break;
            }
        }

    response.SetResult(status);
    if (CodeStatus::Success != status)
        return response;

    auto bcId = static_cast<int>(db.GetBriefcaseId().GetValue());
    for (auto const& code : req)
        {
        Statement insert;
        insert.Prepare(m_db, "INSERT OR REPLACE INTO " SERVER_Table "(" SERVER_Authority "," SERVER_NameSpace "," SERVER_Value "," SERVER_State "," SERVER_Briefcase
                            " VALUES (?,?,?,1,?)");
        insert.BindId(1, code.GetAuthority());
        insert.BindText(2, code.GetNamespace(), Statement::MakeCopy::No);
        insert.BindText(3, code.GetValue(), Statement::MakeCopy::No);
        insert.BindInt(4, bcId);

        insert.Step();
        }

    return response;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodeStatus CodesServer::_ReleaseCodes(DgnCodeSet const& req, DgnDbR db)
    {
    VirtualCodeSet vset(req);
    Statement stmt;
    stmt.Prepare(m_db, "DELETE FROM " SERVER_Table " WHERE " SERVER_Briefcase "=? AND InVirtualSet(@vset, " SERVER_Authority "," SERVER_NameSpace "," SERVER_Value ")");
    stmt.BindInt(1, static_cast<int>(db.GetBriefcaseId().GetValue()));
    stmt.BindVirtualSet(2, vset);
    stmt.Step();

    return CodeStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodeStatus CodesServer::_RelinquishCodes(DgnDbR db)
    {
    Statement stmt;
    stmt.Prepare(m_db, "DELETE FROM " SERVER_Table " WHERE " SERVER_Briefcase " =?");
    stmt.BindInt(1, static_cast<int>(db.GetBriefcaseId().GetValue()));
    stmt.Step();

    return CodeStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodeStatus CodesServer::_QueryCodeStates(DgnCodeInfoSet& infos, DgnCodeSet const& codes)
    {
    infos.clear();

    Statement stmt;
    stmt.Prepare(m_db, "SELECT " SERVER_Authority "," SERVER_NameSpace "," SERVER_Value "," SERVER_State "," SERVER_Revision "," SERVER_Briefcase
                    "   FROM " SERVER_Table " WHERE InVirtualSet(@vset, " SERVER_Authority "," SERVER_NameSpace "," SERVER_Value ")");
    while (BE_SQLITE_ROW == stmt.Step())
        {
        DgnCode code(stmt.GetValueId<DgnAuthorityId>(0), stmt.GetValueText(2), stmt.GetValueText(1));
        DgnCodeInfo info(code);
        switch (static_cast<CodeState>(stmt.GetValueInt(4)))
            {
            case CodeState::Reserved:
                info.SetReserved(BeBriefcaseId(static_cast<uint32_t>(stmt.GetValueInt(5))));
                break;
            case CodeState::Used:
                info.SetUsed(stmt.GetValueText(4));
                break;
            case CodeState::Discarded:
                info.SetDiscarded(stmt.GetValueText(4));
                break;
            case CodeState::Available:
            default:
                BeAssert(false && "This value should never be in the server db!");
                return CodeStatus::SyncError;
            }
        }

    return CodeStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodeStatus CodesServer::_QueryCodes(DgnCodeSet& codes, DgnDbR db)
    {
    codes.clear();
    Statement stmt;
    stmt.Prepare(m_db, "SELECT " SERVER_Authority "," SERVER_NameSpace "," SERVER_Value "   FROM " SERVER_Table " WHERE " SERVER_Briefcase " =?");
    stmt.BindInt(1, static_cast<int>(db.GetBriefcaseId().GetValue()));
    
    while (BE_SQLITE_ROW == stmt.Step())
        codes.insert(DgnCode(stmt.GetValueId<DgnAuthorityId>(0), stmt.GetValueText(2), stmt.GetValueText(1)));

    return CodeStatus::Success;
    }

