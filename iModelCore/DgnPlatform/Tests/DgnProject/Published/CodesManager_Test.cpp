/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/CodesManager_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnMaterial.h>

USING_NAMESPACE_BENTLEY_SQLITE

#define SERVER_Table "Codes"
#define SERVER_Authority "Authority"
#define SERVER_NameSpace "NameSpace"
#define SERVER_Value "Value"
#define SERVER_State "State"
#define SERVER_Revision "Revision"
#define SERVER_Briefcase "Briefcase"

#define EXPECT_STATUS(STAT, EXPR) EXPECT_EQ(CodeStatus:: STAT, (EXPR))

//#define DUMP_SERVER 1

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

    CodeStatus ValidateRelease(DgnCodeInfoSet& toMarkDiscarded, Statement& stmt, BeBriefcaseId bcId);
    CodeStatus ValidateRelease(DgnCodeInfoSet&, DgnCodeSet const&, DgnDbR);
    CodeStatus ValidateRelinquish(DgnCodeInfoSet&, DgnDbR);
    void MarkDiscarded(DgnCodeInfoSet const& discarded);
public:
    CodesServer();

    void Dump(Utf8CP descr=nullptr);
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
CodeStatus CodesServer::ValidateRelease(DgnCodeInfoSet& discarded, Statement& stmt, BeBriefcaseId bcId)
    {
    while (BE_SQLITE_ROW == stmt.Step())
        {
        if (static_cast<int>(CodeState::Reserved) != stmt.GetValueInt(4) || bcId.GetValue() != stmt.GetValueInt(3))
            return CodeStatus::CodeNotReserved;

        if (!stmt.IsColumnNull(5))
            {
            DgnCode code(stmt.GetValueId<DgnAuthorityId>(0), stmt.GetValueText(2), stmt.GetValueText(1));
            DgnCodeInfo info(code);
            info.SetDiscarded(stmt.GetValueText(4));
            discarded.insert(info);
            }
        }

    return CodeStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void CodesServer::MarkDiscarded(DgnCodeInfoSet const& discarded)
    {
    Statement stmt;
    stmt.Prepare(m_db, "INSERT INTO " SERVER_Table "(" SERVER_Authority "," SERVER_NameSpace "," SERVER_Value "," SERVER_State "," SERVER_Revision
            ") VALUES (?,?,?,2,?)");

    for (auto const& info : discarded)
        {
        auto const& code = info.GetCode();
        stmt.BindId(1, code.GetAuthority());
        stmt.BindText(2, code.GetNamespace(), Statement::MakeCopy::No);
        stmt.BindText(3, code.GetValue(), Statement::MakeCopy::No);
        stmt.BindText(4, info.GetRevisionId(), Statement::MakeCopy::No);

        stmt.Step();
        stmt.Reset();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void CodesServer::Dump(Utf8CP descr)
    {
#ifdef DUMP_SERVER
    Statement stmt;
    stmt.Prepare(m_db, "SELECT * FROM " SERVER_Table);
    printf(">>>> %s >>>>\n", nullptr != descr ? descr : "Dumping Server");
    stmt.DumpResults();
    printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodesServer::Response CodesServer::_ReserveCodes(Request const& req, DgnDbR db)
    {
    Dump("ReserveCodes: before");
    VirtualCodeSet vset(req);
    Statement stmt;
    stmt.Prepare(m_db, "SELECT " SERVER_Authority "," SERVER_NameSpace "," SERVER_Value "," SERVER_State "," SERVER_Revision "," SERVER_Briefcase
                    "   FROM " SERVER_Table " WHERE InVirtualSet(@vset, " SERVER_Authority "," SERVER_NameSpace "," SERVER_Value ")");

    CodeStatus status = CodeStatus::Success;
    Response response(status);
    stmt.BindVirtualSet(1, vset);
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
                            ") VALUES (?,?,?,1,?)");
        insert.BindId(1, code.GetAuthority());
        insert.BindText(2, code.GetNamespace(), Statement::MakeCopy::No);
        insert.BindText(3, code.GetValue(), Statement::MakeCopy::No);
        insert.BindInt(4, bcId);

        insert.Step();
        }

    Dump("ReserveCodes: after");
    return response;
    }

#define SELECT_ValidateRelease "SELECT " SERVER_Authority "," SERVER_NameSpace "," SERVER_Value "," SERVER_Briefcase "," SERVER_State "," SERVER_Revision " FROM " SERVER_Table

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodeStatus CodesServer::ValidateRelease(DgnCodeInfoSet& discarded, DgnCodeSet const& req, DgnDbR db)
    {
    Statement stmt;
    stmt.Prepare(m_db, SELECT_ValidateRelease " WHERE InVirtualSet(@vset, " SERVER_Authority "," SERVER_NameSpace "," SERVER_Value ")");
    VirtualCodeSet vset(req);
    return ValidateRelease(discarded, stmt, db.GetBriefcaseId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodeStatus CodesServer::_ReleaseCodes(DgnCodeSet const& req, DgnDbR db)
    {
    Dump("ReleaseCodes: before");
    DgnCodeInfoSet discarded;
    CodeStatus status = ValidateRelease(discarded, req, db);
    if (CodeStatus::Success != status)
        return status;

    VirtualCodeSet vset(req);
    Statement stmt;
    stmt.Prepare(m_db, "DELETE FROM " SERVER_Table " WHERE " SERVER_Briefcase "=? AND InVirtualSet(@vset, " SERVER_Authority "," SERVER_NameSpace "," SERVER_Value ")");
    stmt.BindInt(1, static_cast<int>(db.GetBriefcaseId().GetValue()));
    stmt.BindVirtualSet(2, vset);
    stmt.Step();

    MarkDiscarded(discarded);

    Dump("ReleaseCodes: after");
    return CodeStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodeStatus CodesServer::ValidateRelinquish(DgnCodeInfoSet& discarded, DgnDbR db)
    {
    Statement stmt;
    stmt.Prepare(m_db, SELECT_ValidateRelease " WHERE " SERVER_Briefcase " =?");
    stmt.BindInt(1, static_cast<int>(db.GetBriefcaseId().GetValue()));
    return ValidateRelease(discarded, stmt, db.GetBriefcaseId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodeStatus CodesServer::_RelinquishCodes(DgnDbR db)
    {
    Dump("RelinquishCodes: before");
    DgnCodeInfoSet discarded;
    CodeStatus status = ValidateRelinquish(discarded, db);
    if (CodeStatus::Success != status)
        return status;

    Statement stmt;
    stmt.Prepare(m_db, "DELETE FROM " SERVER_Table " WHERE " SERVER_Briefcase " =?");
    stmt.BindInt(1, static_cast<int>(db.GetBriefcaseId().GetValue()));
    stmt.Step();

    MarkDiscarded(discarded);

    Dump("RelinquishCodes: after");
    return CodeStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodeStatus CodesServer::_QueryCodeStates(DgnCodeInfoSet& infos, DgnCodeSet const& codes)
    {
    Dump("QueryCodeStates");
    infos.clear();

    VirtualCodeSet vset(codes);
    Statement stmt;
    stmt.Prepare(m_db, "SELECT " SERVER_Authority "," SERVER_NameSpace "," SERVER_Value "," SERVER_State "," SERVER_Revision "," SERVER_Briefcase
                    "   FROM " SERVER_Table " WHERE InVirtualSet(@vset, " SERVER_Authority "," SERVER_NameSpace "," SERVER_Value ")");
    stmt.BindVirtualSet(1, vset);
    while (BE_SQLITE_ROW == stmt.Step())
        {
        DgnCode code(stmt.GetValueId<DgnAuthorityId>(0), stmt.GetValueText(2), stmt.GetValueText(1));
        DgnCodeInfo info(code);
        switch (static_cast<CodeState>(stmt.GetValueInt(3)))
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

        infos.insert(info);
        }

    for (auto const& code : codes)
        {
        // Server doesn't keep track of "available" codes...
        DgnCodeInfo info(code);
        if (infos.end() == infos.find(info))
            infos.insert(info);
        }

    return CodeStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodeStatus CodesServer::_QueryCodes(DgnCodeSet& codes, DgnDbR db)
    {
    Dump("QueryCodes");
    codes.clear();
    Statement stmt;
    stmt.Prepare(m_db, "SELECT " SERVER_Authority "," SERVER_NameSpace "," SERVER_Value "   FROM " SERVER_Table " WHERE " SERVER_Briefcase " =?");
    stmt.BindInt(1, static_cast<int>(db.GetBriefcaseId().GetValue()));
    
    while (BE_SQLITE_ROW == stmt.Step())
        codes.insert(DgnCode(stmt.GetValueId<DgnAuthorityId>(0), stmt.GetValueText(2), stmt.GetValueText(1)));

    return CodeStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct CodesManagerTest : public ::testing::Test, DgnPlatformLib::Host::ServerAdmin
{
    typedef IDgnCodesManager::Request Request;
    typedef IDgnCodesManager::Response Response;

    mutable CodesServer m_server;
    ScopedDgnHost m_host;
    
    CodesManagerTest()
        {
        m_host.SetServerAdmin(this);
        BackDoor::IDgnCodesManager::SetEnabled(true);
        }

    ~CodesManagerTest()
        {
        BackDoor::IDgnCodesManager::SetEnabled(false);
        }

    virtual IDgnCodesServerP _GetCodesServer(DgnDbR) const override { return &m_server; }

    DgnDbPtr SetupDb(WCharCP testFile, BeBriefcaseId bcId, WCharCP baseFile=L"3dMetricGeneral.idgndb")
        {
        BeFileName outFileName;
        EXPECT_EQ(SUCCESS, DgnDbTestDgnManager::GetTestDataOut(outFileName, baseFile, testFile, __FILE__));
        auto db = DgnDb::OpenDgnDb(nullptr, outFileName, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
        EXPECT_TRUE(db.IsValid());
        if (!db.IsValid())
            return nullptr;

        TestDataManager::MustBeBriefcase(db, Db::OpenMode::ReadWrite);

        db->ChangeBriefcaseId(bcId);
        return db;
        }

    static DgnCode MakeCode(Utf8StringCR name, Utf8CP nameSpace = nullptr)
        {
        return nullptr != nameSpace ? DgnMaterial::CreateMaterialCode(nameSpace, name) : DgnCategory::CreateCategoryCode(name);
        }

    static DgnCodeInfo MakeAvailable(DgnCodeCR code) { return DgnCodeInfo(code); }
    static DgnCodeInfo MakeUsed(DgnCodeCR code, Utf8StringCR revisionId) { return MakeUsedOrDiscarded(code, revisionId, false); }
    static DgnCodeInfo MakeDiscarded(DgnCodeCR code, Utf8StringCR revisionId) { return MakeUsedOrDiscarded(code, revisionId, true); }
    static DgnCodeInfo MakeReserved(DgnCodeCR code, DgnDbR db)
        {
        DgnCodeInfo info(code);
        info.SetReserved(db.GetBriefcaseId());
        return info;
        }
    static DgnCodeInfo MakeUsedOrDiscarded(DgnCodeCR code, Utf8StringCR rev, bool discarded)
        {
        DgnCodeInfo info(code);
        if (discarded)
            info.SetDiscarded(rev);
        else
            info.SetUsed(rev);
        return info;
        }

    void ExpectState(DgnCodeInfoCR expect, DgnDbR db)
        {
        DgnCodeInfoSet expectInfos;
        expectInfos.insert(expect);
        ExpectStates(expectInfos, db);
        }
    void ExpectStates(DgnCodeInfoSet const& expect, DgnDbR db)
        {
        DgnCodeInfoSet actual;
        DgnCodeSet codes;
        for (auto const& info : expect)
            codes.insert(info.GetCode());

        EXPECT_STATUS(Success, db.Codes().QueryCodeStates(actual, codes));
        ExpectEqual(expect, actual);
        }
    void ExpectEqual(DgnCodeInfoSet const& expected, DgnCodeInfoSet const& actual)
        {
        EXPECT_EQ(expected.size(), actual.size());
        for (auto expIter = expected.begin(), actIter = actual.begin(); expIter != expected.end() && actIter != actual.end(); ++expIter, ++actIter)
            ExpectEqual(*expIter, *actIter);
        }
    void ExpectEqual(DgnCodeInfoCR exp, DgnCodeInfoCR act)
        {
        EXPECT_EQ(exp.GetCode(), act.GetCode());
        ExpectEqual(static_cast<DgnCodeState>(exp), static_cast<DgnCodeState>(act));
        }
    void ExpectEqual(DgnCodeStateCR exp, DgnCodeStateCR act)
        {
        if (exp.IsAvailable())
            {
            EXPECT_TRUE(act.IsAvailable());
            }
        else if (exp.IsReserved())
            {
            EXPECT_TRUE(act.IsReserved());
            EXPECT_EQ(exp.GetReservedBy().GetValue(), act.GetReservedBy().GetValue());
            }
        else
            {
            EXPECT_EQ(exp.IsDiscarded(), act.IsDiscarded());
            EXPECT_EQ(exp.GetRevisionId(), act.GetRevisionId());
            EXPECT_FALSE(exp.GetRevisionId().empty());
            }
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CodesManagerTest, ReserveQueryRelinquish)
    {
    DgnDbPtr pDb = SetupDb(L"ReserveQueryRelinquish.dgndb", BeBriefcaseId(1));
    DgnDbR db = *pDb;
    IDgnCodesManagerR mgr = db.Codes();

    // Empty request
    Request req;
    EXPECT_STATUS(Success, mgr.ReserveCodes(req).GetResult());

    // Reserve single code
    DgnCode code = MakeCode("Palette", "Material");
    req.insert(code);
    EXPECT_STATUS(Success, mgr.ReserveCodes(req).GetResult());
    ExpectState(MakeReserved(code, db), db);

    // Relinquish all
    EXPECT_STATUS(Success, mgr.RelinquishCodes());
    ExpectState(MakeAvailable(code), db);

    // Reserve 2 codes
    DgnCode code2 = MakeCode("Category");
    req.insert(code2);
    EXPECT_STATUS(Success, mgr.ReserveCodes(req).GetResult());
    ExpectState(MakeReserved(code, db), db);
    ExpectState(MakeReserved(code2, db), db);

    // Release one code
    DgnCodeSet codes;
    codes.insert(code);
    EXPECT_STATUS(Success, mgr.ReleaseCodes(codes));
    ExpectState(MakeAvailable(code), db);
    ExpectState(MakeReserved(code2, db), db);

    // Relinquish all
    EXPECT_STATUS(Success, mgr.RelinquishCodes());
    ExpectState(MakeAvailable(code2), db);
    }

