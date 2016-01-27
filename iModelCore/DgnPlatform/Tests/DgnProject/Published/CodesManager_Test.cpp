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

#define EXPECT_STATUS(STAT, EXPR) EXPECT_EQ(RepositoryStatus:: STAT, (EXPR))

//#define DUMP_SERVER 1

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct CodesServer : IDgnCodesServer
{
    enum class CodeState : uint8_t { Available, Reserved, Discarded, Used };
private:
    Db  m_db;

    virtual CodeResponse _ReserveCodes(CodeRequest const&, DgnDbR) override;
    virtual RepositoryStatus _ReleaseCodes(DgnCodeSet const&, DgnDbR) override;
    virtual RepositoryStatus _RelinquishCodes(DgnDbR) override;
    virtual RepositoryStatus _QueryCodeStates(DgnCodeInfoSet&, DgnCodeSet const&) override;
    virtual RepositoryStatus _QueryCodes(DgnCodeSet&, DgnDbR) override;

    RepositoryStatus ValidateRelease(DgnCodeInfoSet& toMarkDiscarded, Statement& stmt, BeBriefcaseId bcId);
    RepositoryStatus ValidateRelease(DgnCodeInfoSet&, DgnCodeSet const&, DgnDbR);
    RepositoryStatus ValidateRelinquish(DgnCodeInfoSet&, DgnDbR);
    void MarkDiscarded(DgnCodeInfoSet const& discarded);

    void MarkRevision(DgnCodeSet const& codes, bool discarded, Utf8StringCR revId);
public:
    CodesServer();

    void Dump(Utf8CP descr=nullptr);

    // Simulates what the real server does with codes when a revision is pushed.
    void OnFinishRevision(DgnRevision const& rev)
        {
        MarkRevision(rev.GetAssignedCodes(), false, rev.GetId());
        MarkRevision(rev.GetDiscardedCodes(), true, rev.GetId());
        }

    void MarkUsed(DgnCode const& code, Utf8StringCR revision)
        {
        DgnCodeSet codes;
        codes.insert(code);
        MarkRevision(codes, false, revision);
        }
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
RepositoryStatus CodesServer::ValidateRelease(DgnCodeInfoSet& discarded, Statement& stmt, BeBriefcaseId bcId)
    {
    while (BE_SQLITE_ROW == stmt.Step())
        {
        if (static_cast<int>(CodeState::Reserved) != stmt.GetValueInt(4) || bcId.GetValue() != stmt.GetValueInt(3))
            return RepositoryStatus::CodeNotReserved;

        if (!stmt.IsColumnNull(5))
            {
            DgnCode code(stmt.GetValueId<DgnAuthorityId>(0), stmt.GetValueText(2), stmt.GetValueText(1));
            DgnCodeInfo info(code);
            info.SetDiscarded(stmt.GetValueText(5));
            discarded.insert(info);
            }
        }

    return RepositoryStatus::Success;
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
CodeResponse CodesServer::_ReserveCodes(CodeRequest const& req, DgnDbR db)
    {
    Dump("ReserveCodes: before");
    VirtualCodeSet vset(req);
    Statement stmt;
    stmt.Prepare(m_db, "SELECT " SERVER_Authority "," SERVER_NameSpace "," SERVER_Value "," SERVER_State "," SERVER_Revision "," SERVER_Briefcase
                    "   FROM " SERVER_Table " WHERE InVirtualSet(@vset, " SERVER_Authority "," SERVER_NameSpace "," SERVER_Value ")");

    DgnCodeInfoSet discarded;

    RepositoryStatus status = RepositoryStatus::Success;
    CodeResponse response(status);
    stmt.BindVirtualSet(1, vset);
    bool wantInfos = CodeResponseOptions::IncludeState == (req.GetOptions() & CodeResponseOptions::IncludeState);
    while (BE_SQLITE_ROW == stmt.Step())
        {
        DgnCode code(stmt.GetValueId<DgnAuthorityId>(0), stmt.GetValueText(2), stmt.GetValueText(1));
        switch (static_cast<CodeState>(stmt.GetValueInt(3)))
            {
            case CodeState::Reserved:
                {
                BeBriefcaseId owner(static_cast<uint32_t>(stmt.GetValueInt(5)));
                if (owner.GetValue() != db.GetBriefcaseId().GetValue())
                    {
                    status = RepositoryStatus::CodeUnavailable;
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
                status = RepositoryStatus::CodeUnavailable;
                if (wantInfos)
                    {
                    DgnCodeInfo info(code);
                    info.SetUsed(stmt.GetValueText(4));
                    response.GetDetails().insert(info);
                    }
                break;
            case CodeState::Discarded:
                // ###TODO: Check if briefcase has pulled the required revision...
                {
                DgnCodeInfo info(code);
                info.SetDiscarded(stmt.GetValueText(4));
                discarded.insert(info);
                }
                break;
            default:
                BeAssert(false);
                break;
            }
        }

    response.SetResult(status);
    if (RepositoryStatus::Success != status)
        return response;

    auto bcId = static_cast<int>(db.GetBriefcaseId().GetValue());
    Statement insert;
    insert.Prepare(m_db, "INSERT OR REPLACE INTO " SERVER_Table "(" SERVER_Authority "," SERVER_NameSpace "," SERVER_Value "," SERVER_State "," SERVER_Briefcase "," SERVER_Revision
                        ") VALUES (?,?,?,1,?,?)");
    for (auto const& code : req)
        {
        insert.BindId(1, code.GetAuthority());
        insert.BindText(2, code.GetNamespace(), Statement::MakeCopy::No);
        insert.BindText(3, code.GetValue(), Statement::MakeCopy::No);
        insert.BindInt(4, bcId);
        auto revIter = discarded.find(DgnCodeInfo(code));
        if (discarded.end() != revIter)
            insert.BindText(5, revIter->GetRevisionId(), Statement::MakeCopy::No);

        insert.Step();
        insert.Reset();
        }

    Dump("ReserveCodes: after");
    return response;
    }

#define SELECT_ValidateRelease "SELECT " SERVER_Authority "," SERVER_NameSpace "," SERVER_Value "," SERVER_Briefcase "," SERVER_State "," SERVER_Revision " FROM " SERVER_Table

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus CodesServer::ValidateRelease(DgnCodeInfoSet& discarded, DgnCodeSet const& req, DgnDbR db)
    {
    Statement stmt;
    stmt.Prepare(m_db, SELECT_ValidateRelease " WHERE InVirtualSet(@vset, " SERVER_Authority "," SERVER_NameSpace "," SERVER_Value ")");
    VirtualCodeSet vset(req);
    return ValidateRelease(discarded, stmt, db.GetBriefcaseId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus CodesServer::_ReleaseCodes(DgnCodeSet const& req, DgnDbR db)
    {
    Dump("ReleaseCodes: before");
    DgnCodeInfoSet discarded;
    RepositoryStatus status = ValidateRelease(discarded, req, db);
    if (RepositoryStatus::Success != status)
        return status;

    VirtualCodeSet vset(req);
    Statement stmt;
    stmt.Prepare(m_db, "DELETE FROM " SERVER_Table " WHERE " SERVER_Briefcase "=? AND InVirtualSet(@vset, " SERVER_Authority "," SERVER_NameSpace "," SERVER_Value ")");
    stmt.BindInt(1, static_cast<int>(db.GetBriefcaseId().GetValue()));
    stmt.BindVirtualSet(2, vset);
    stmt.Step();

    MarkDiscarded(discarded);

    Dump("ReleaseCodes: after");
    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus CodesServer::ValidateRelinquish(DgnCodeInfoSet& discarded, DgnDbR db)
    {
    Statement stmt;
    stmt.Prepare(m_db, SELECT_ValidateRelease " WHERE " SERVER_Briefcase " =?");
    stmt.BindInt(1, static_cast<int>(db.GetBriefcaseId().GetValue()));
    return ValidateRelease(discarded, stmt, db.GetBriefcaseId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus CodesServer::_RelinquishCodes(DgnDbR db)
    {
    Dump("RelinquishCodes: before");
    DgnCodeInfoSet discarded;
    RepositoryStatus status = ValidateRelinquish(discarded, db);
    if (RepositoryStatus::Success != status)
        return status;

    Statement stmt;
    stmt.Prepare(m_db, "DELETE FROM " SERVER_Table " WHERE " SERVER_Briefcase " =?");
    stmt.BindInt(1, static_cast<int>(db.GetBriefcaseId().GetValue()));
    stmt.Step();

    MarkDiscarded(discarded);

    Dump("RelinquishCodes: after");
    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus CodesServer::_QueryCodeStates(DgnCodeInfoSet& infos, DgnCodeSet const& codes)
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
                return RepositoryStatus::SyncError;
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

    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus CodesServer::_QueryCodes(DgnCodeSet& codes, DgnDbR db)
    {
    Dump("QueryCodes");
    codes.clear();
    Statement stmt;
    stmt.Prepare(m_db, "SELECT " SERVER_Authority "," SERVER_NameSpace "," SERVER_Value "   FROM " SERVER_Table " WHERE " SERVER_Briefcase " =?");
    stmt.BindInt(1, static_cast<int>(db.GetBriefcaseId().GetValue()));
    
    while (BE_SQLITE_ROW == stmt.Step())
        codes.insert(DgnCode(stmt.GetValueId<DgnAuthorityId>(0), stmt.GetValueText(2), stmt.GetValueText(1)));

    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void CodesServer::MarkRevision(DgnCodeSet const& codes, bool discarded, Utf8StringCR revId)
    {
    if (codes.empty())
        return;

    Statement stmt;
    stmt.Prepare(m_db, "INSERT OR REPLACE INTO " SERVER_Table "(" SERVER_Authority "," SERVER_NameSpace "," SERVER_Value "," SERVER_State "," SERVER_Revision
            ") VALUES (?,?,?,?,?)");
    for (auto const& code : codes)
        {
        stmt.BindId(1, code.GetAuthority());
        stmt.BindText(2, code.GetNamespace(), Statement::MakeCopy::No);
        stmt.BindText(3, code.GetValue(), Statement::MakeCopy::No);
        stmt.BindInt(4, static_cast<int>(discarded ? CodeState::Discarded : CodeState::Used));
        stmt.BindText(5, revId, Statement::MakeCopy::No);

        stmt.Step();
        stmt.Reset();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct CodesManagerTest : public ::testing::Test, DgnPlatformLib::Host::RepositoryAdmin
{
    typedef CodeRequest Request;
    typedef CodeResponse Response;

    mutable CodesServer m_server;
    ScopedDgnHost m_host;
    
    CodesManagerTest()
        {
        m_host.SetRepositoryAdmin(this);
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

    Utf8String CommitRevision(DgnDbR db);

    static DgnDbStatus InsertStyle(Utf8CP name, DgnDbR db)
        {
        AnnotationTextStyle style(db);
        style.SetName(name);
        DgnDbStatus status;
        style.DgnElement::Insert(&status);
        return status;
        }
    static DgnCode MakeStyleCode(Utf8CP name, DgnDbR db)
        {
        // Because AnnotationTextStyle::CreateCodeFromName() is private for some reason...
        AnnotationTextStyle style(db);
        style.SetName(name);
        return style.GetCode();
        }

    DgnCodeSet GetReservedCodes(DgnDbR db)
        {
        DgnCodeSet codes;
        EXPECT_STATUS(Success, m_server.QueryCodes(codes, db));
        return codes;
        }
};

/*---------------------------------------------------------------------------------**//**
* Simulate pushing a revision to the server.
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CodesManagerTest::CommitRevision(DgnDbR db)
    {
    Utf8String revId;
    DgnRevisionPtr rev;
    if (BE_SQLITE_OK != db.SaveChanges() || (rev = db.Revisions().StartCreateRevision(nullptr, DgnRevision::Include::Codes)).IsNull())
        return revId;

    if (RevisionStatus::Success == db.Revisions().FinishCreateRevision())
        {
        m_server.OnFinishRevision(*rev);
        revId = rev->GetId();
        }

    return revId;
    }

/*---------------------------------------------------------------------------------**//**
* Exercise the basic functions: reserve + release codes, query code state
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CodesManagerTest, ReserveQueryRelinquish)
    {
    DgnDbPtr pDb = SetupDb(L"ReserveQueryRelinquish.dgndb", BeBriefcaseId(1));
    DgnDbR db = *pDb;
    IDgnCodesManagerR mgr = db.Codes();

    // Empty request
    CodeRequest req;
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

/*---------------------------------------------------------------------------------**//**
* Test that any INSERT or UPDATE will fail if the object's code has not been reserved.
* (If we don't explicitly reserve it prior to insert/update, an attempt will be made to
* reserve it for us).
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CodesManagerTest, AutoReserveCodes)
    {
    DgnDbPtr pDb = SetupDb(L"AutoReserveCodes.dgndb", BeBriefcaseId(1));
    DgnDbR db = *pDb;

    // Simulate a pre-existing style having been committed in a previous revision
    static const Utf8String s_initialRevisionId("InitialRevision");
    DgnCode existingStyleCode = MakeStyleCode("Preexisting", db);
    m_server.MarkUsed(existingStyleCode, s_initialRevisionId);

    ExpectState(MakeUsed(existingStyleCode, s_initialRevisionId), db);
    EXPECT_EQ(0, GetReservedCodes(db).size());

    // When we insert an element without having explicitly reserved its code, an attempt to reserve it will automatically occur
    EXPECT_EQ(DgnDbStatus::Success, InsertStyle("MyStyle", db));
    EXPECT_EQ(1, GetReservedCodes(db).size());
    ExpectState(MakeReserved(MakeStyleCode("MyStyle", db), db), db);

    // An attempt to insert an element with the same code as an already-used code will fail
    EXPECT_EQ(DgnDbStatus::CodeNotReserved, InsertStyle(existingStyleCode.GetValue().c_str(), db));

    // Updating an element and changing its code will reserve the new code if we haven't done so already
    auto pStyle = AnnotationTextStyle::Get(db, "MyStyle")->CreateCopy();
    pStyle->SetName("MyRenamedStyle");
    EXPECT_TRUE(pStyle->Update().IsValid());
    EXPECT_EQ(2, GetReservedCodes(db).size());
    ExpectState(MakeReserved(pStyle->GetCode(), db), db);
    pStyle = nullptr;

    // Attempting to change code to an already-used code will fail on update
    pStyle = AnnotationTextStyle::Get(db, "MyRenamedStyle")->CreateCopy();
    pStyle->SetName(existingStyleCode.GetValue().c_str());
    DgnDbStatus status;
    EXPECT_TRUE(pStyle->DgnElement::Update(&status).IsNull());
    EXPECT_EQ(DgnDbStatus::CodeNotReserved, status);

    pStyle = nullptr;

    db.SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* When we commit a revision, the changes made to the revision are processed such that:
*   - Any codes which were newly-assigned become marked as "used", and
*   - Any previously-used codes which became no-longer-used are marked as "discarded"
* In both cases the server records the revision ID.
* Codes which become "used" were necessarily previously "reserved" by the briefcase.
* After committing the revision, they are no longer "reserved".
* Codes which became "discarded" were necessarily previously "used".
* After committing the revision, they become available for reserving by any briefcase,
* provided the briefcase has pulled the revision in which they became discarded.
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CodesManagerTest, CodesInRevisions)
    {
    DgnDbPtr pDb = SetupDb(L"CodesInRevisions.dgndb", BeBriefcaseId(1));
    DgnDbR db = *pDb;
    IDgnCodesManagerR mgr = db.Codes();

    // Reserve some codes
    DgnCode unusedCode = MakeStyleCode("Unused", db);
    DgnCode usedCode = MakeStyleCode("Used", db);
    CodeRequest req;
    req.insert(unusedCode);
    req.insert(usedCode);
    EXPECT_STATUS(Success, mgr.ReserveCodes(req).GetResult());

    // Use one of the codes
    EXPECT_EQ(DgnDbStatus::Success, InsertStyle("Used", db));
    ExpectState(MakeReserved(unusedCode, db), db);
    ExpectState(MakeReserved(usedCode, db), db);

    // Commit the change as a revision
    Utf8String rev1 = CommitRevision(db);
    EXPECT_FALSE(rev1.empty());

    // The used code should not be marked as such
    ExpectState(MakeUsed(usedCode, rev1), db);
    ExpectState(MakeReserved(unusedCode, db), db);

    // We cannot reserve a used code
    EXPECT_STATUS(CodeUnavailable, mgr.ReserveCode(usedCode));

    // Swap the code so that "Used" becomes "Unused"
    auto pStyle = AnnotationTextStyle::Get(db, "Used")->CreateCopy();
    pStyle->SetName("Unused");
    EXPECT_TRUE(pStyle->Update().IsValid());
    pStyle = nullptr;

    // Commit the revision
    Utf8String rev2 = CommitRevision(db);
    EXPECT_FALSE(rev2.empty());

    // "Used" is now discarded; "Unused" is now used; both in the same revision
    ExpectState(MakeUsed(unusedCode, rev2), db);
    ExpectState(MakeDiscarded(usedCode, rev2), db);

    // Delete the style => its code becomes discarded
    // Ugh except you are not allowed to delete text styles...rename it again instead
    pStyle = AnnotationTextStyle::Get(db, "Unused")->CreateCopy();
    pStyle->SetName("Deleted");
    EXPECT_TRUE(pStyle->Update().IsValid());
    pStyle = nullptr;

    // Cannot release codes if transactions are pending
    EXPECT_EQ(1, GetReservedCodes(db).size());
    DgnCodeSet codes;
    codes.insert(MakeStyleCode("Deleted", db));
    EXPECT_STATUS(PendingTransactions, mgr.ReleaseCodes(codes));
    EXPECT_STATUS(PendingTransactions, mgr.RelinquishCodes());

    // Cannot release a code which is used locally
    db.SaveChanges();
    EXPECT_EQ(1, GetReservedCodes(db).size());
    EXPECT_STATUS(CodeUsed, mgr.ReleaseCodes(codes));
    EXPECT_STATUS(CodeUsed, mgr.RelinquishCodes());

    Utf8String rev3 = CommitRevision(db);
    EXPECT_FALSE(rev3.empty());
    ExpectState(MakeDiscarded(unusedCode, rev3), db);
    ExpectState(MakeDiscarded(usedCode, rev2), db);

    // We can reserve either code, since they are discarded and we have the latest revision
    EXPECT_STATUS(Success, mgr.ReserveCode(usedCode));
    EXPECT_STATUS(Success, mgr.ReserveCode(unusedCode));
    ExpectState(MakeReserved(usedCode, db), db);
    ExpectState(MakeReserved(unusedCode, db), db);

    // If we release these codes, they should return to "Discarded" and retain the most recent revision ID in which they were discarded.
    EXPECT_STATUS(Success, mgr.RelinquishCodes());
    ExpectState(MakeDiscarded(unusedCode, rev3), db);
    ExpectState(MakeDiscarded(usedCode, rev2), db);
    }

