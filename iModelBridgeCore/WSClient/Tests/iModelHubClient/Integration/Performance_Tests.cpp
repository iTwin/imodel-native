/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/Performance_Tests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "IntegrationTestsBase.h"
#include "IntegrationTestsHelper.h"
#include "PerformanceTestsHelper.h"
#include <DgnPlatform/DgnMaterial.h>
#include <DgnPlatform/GenericDomain.h>
#include <ECDb/ECInstanceId.h>
#include <ECDb/ECSqlStatement.h>
#include <ECDb/ECSqlStatus.h>
#include <ECObjects/ECSchema.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTimeUtilities.h>
#include <windows.h>

#define DPTEST_SCHEMA_NAME                               "DgnPlatformTest"
#define DPTEST_TEST_MULTI_ASPECT_CLASS_NAME              "TestMultiAspect"

USING_NAMESPACE_BENTLEY_DGN

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
USING_NAMESPACE_BENTLEY_SQLITE_EC

#define EXPECT_STATUS(STAT, EXPR) EXPECT_EQ(RepositoryStatus:: STAT, (EXPR))

//=======================================================================================
// @bsiclass                                           Algirdas.Mikoliunas       05/17
//=======================================================================================
struct TestMultiAspect : Dgn::DgnElement::MultiAspect
{
    DGNASPECT_DECLARE_MEMBERS(DPTEST_SCHEMA_NAME, DPTEST_TEST_MULTI_ASPECT_CLASS_NAME, Dgn::DgnElement::MultiAspect);
private:
    friend struct TestMultiAspectHandler;

    Utf8String m_testMultiAspectProperty;
    BeSQLite::EC::ECCrudWriteToken const* m_token;

    explicit TestMultiAspect(Utf8CP prop) : m_testMultiAspectProperty(prop) { ; }

    Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR el) override { return DgnDbStatus::Success; }
    Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* token) override {
        m_token = token;
        return DgnDbStatus::Success;
    }

protected:
    DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, const PropertyArrayIndex &) const { return DgnDbStatus::Success; }
    DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, const PropertyArrayIndex &) {return DgnDbStatus::Success;}

public:
    static RefCountedPtr<TestMultiAspect> Create(Utf8CP prop) { return new TestMultiAspect(prop); }

    static ECN::ECClassCP GetECClass(Dgn::DgnDbR db) { return db.Schemas().GetClass(DPTEST_SCHEMA_NAME, DPTEST_TEST_MULTI_ASPECT_CLASS_NAME); }

    Utf8StringCR GetTestMultiAspectProperty() const { return m_testMultiAspectProperty; }
    void SetTestMultiAspectProperty(Utf8CP s) { m_testMultiAspectProperty = s; }
    BeSQLite::EC::ECCrudWriteToken const* GetToken() { return m_token; }
};

//=======================================================================================
// @bsiclass                                           Algirdas.Mikoliunas       05/17
//=======================================================================================
struct TestMultiAspectHandler : Dgn::dgn_AspectHandler::Aspect
{
    DOMAINHANDLER_DECLARE_MEMBERS(DPTEST_TEST_MULTI_ASPECT_CLASS_NAME, TestMultiAspectHandler, Dgn::dgn_AspectHandler::Aspect, )
        RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override { return new TestMultiAspect(""); }
};

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                            Algirdas.Mikoliunas       05/17
+---------------+---------------+---------------+---------------+---------------+------*/
struct MultiAspectHandlerDomain : DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(MultiAspectHandlerDomain, );

private:
    WCharCP _GetSchemaRelativePath() const override { return L"ECSchemas/" DPTEST_SCHEMA_NAME L".ecschema.xml"; }

public:
    MultiAspectHandlerDomain() : DgnDomain(DPTEST_SCHEMA_NAME, "Missing Handlers domain", 1)
        {
        RegisterHandler(TestMultiAspectHandler::GetHandler());
        }

};

HANDLER_DEFINE_MEMBERS(TestMultiAspectHandler);
DOMAIN_DEFINE_MEMBERS(MultiAspectHandlerDomain);

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             10/2016
//---------------------------------------------------------------------------------------
struct PerformanceTests : public IntegrationTestsBase
    {
    ClientPtr    m_client;
    iModelInfoPtr m_imodel;
    iModelConnectionPtr m_imodelConnection;

    std::shared_ptr<MultiAspectHandlerDomain> m_domain;

    BriefcasePtr m_briefcase;
    DgnModelPtr m_model;

    int m_codeValueIndex = 0;
    int m_codeValueBatch = 0;

    uint64_t s_elementId = UINT64_C(2000000);

    virtual void SetUp() override
        {
        IntegrationTestsBase::SetUp();

        m_client = SetUpClient(IntegrationTestSettings::Instance().GetValidHost(), IntegrationTestSettings::Instance().GetValidAdminCredentials()); 
        auto db = CreateTestDb("PerformanceTests");
        MultiAspectHandlerDomain::GetDomain().ImportSchema(*db);
        EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db->SaveChanges());
        m_imodel = CreateNewiModelFromDb(*m_client, *db);
        m_imodelConnection = ConnectToiModel(*m_client, m_imodel);

        m_pHost->SetRepositoryAdmin(m_client->GetiModelAdmin());
        m_briefcase = IntegrationTestsBase::AcquireBriefcase(*m_client, *m_imodel);
        m_model = CreateModel("Model_Test", m_briefcase->GetDgnDb());
        }

    virtual void TearDown() override
        {
        DeleteiModel(*m_client, *m_imodel);
        m_client = nullptr;
        m_briefcase->GetDgnDb().CloseDb();
        IntegrationTestsBase::TearDown();
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     julius.cepukenas             10/2016
    //---------------------------------------------------------------------------------------
    void LogTiming(StopWatch& timer, int numberOfInstances, int total, bool isPassed)
        {
        Utf8String description;
        description.Sprintf("Test searched for [%d] of [%d] instances", numberOfInstances, total);
        LogTiming(timer, description, isPassed);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     julius.cepukenas             10/2016
    //---------------------------------------------------------------------------------------
    void LogTiming(StopWatch& timer, int numberOfInstances, bool isPassed)
        {
        Utf8String description;
        description.Sprintf("Test used[%d] number of instances", numberOfInstances);
        LogTiming(timer, description, isPassed);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     julius.cepukenas             10/2016
    //---------------------------------------------------------------------------------------
    void LogTiming(StopWatch& timer, Utf8String description, bool isPassed)
        {
        Utf8String totalDescription;
        Utf8String status = "failed";
        if (isPassed)
            status = "passed";

        totalDescription.Sprintf("Test %s. %s", status, description);
        LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), -1, totalDescription.c_str());
        printf("%.8f %s\n", timer.GetElapsedSeconds(), totalDescription.c_str());
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     julius.cepukenas             10/2016
    //---------------------------------------------------------------------------------------
    DgnCode GetNextCode(DgnDbR db)
        {
        Utf8String name;
        name.Sprintf("%i-%i", m_codeValueBatch, m_codeValueIndex);
        DgnCode code = PhysicalMaterial::CreateCode(db.GetDictionaryModel(), name);
        m_codeValueIndex++;
        auto postRequestSize = PerformanceTestSettings::Instance().GetCodePostRequestSize();
        if (postRequestSize != 0 && m_codeValueIndex >= postRequestSize)
            {
            m_codeValueIndex = 0;
            m_codeValueBatch++;
            }
        return code;
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     julius.cepukenas             10/2016
    //---------------------------------------------------------------------------------------
    GenericPhysicalObjectPtr CreateElement(DgnModelPtr model, ECSqlStatement& stmt)
        {
        const ECInstanceId id(s_elementId++);
        DgnCategoryId catId = (*SpatialCategory::MakeIterator(model->GetDgnDb()).begin()).GetId<DgnCategoryId>();
        auto element = GenericPhysicalObject::Create(*model->ToPhysicalModelP(), catId);

        EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(stmt.GetParameterIndex("ECInstanceId"), id));
        EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(stmt.GetParameterIndex("Modelid"), element->GetModelId()));

        if (!element->GetFederationGuid().IsValid())
            EXPECT_EQ(ECSqlStatus::Success, stmt.BindNull(stmt.GetParameterIndex("FederationGuid")));

        // Bind Code
        DgnCode elementCode = DgnCode::CreateEmpty();

        EXPECT_EQ(ECSqlStatus::Success, stmt.BindNull(stmt.GetParameterIndex("CodeValue")));

        EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(stmt.GetParameterIndex("CodeSpecId"), elementCode.GetCodeSpecId()));

        EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(stmt.GetParameterIndex("CodeScopeId"), elementCode.GetScopeElementId(model->GetDgnDb())));

        EXPECT_EQ(ECSqlStatus::Success, stmt.BindNull(stmt.GetParameterIndex("UserLabel")));

        EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(stmt.GetParameterIndex("ParentId"), element->GetParentId()));
        EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(stmt.GetParameterIndex("CategoryId"), element->ToGeometrySource()->GetCategoryId()));

        EXPECT_EQ(ECSqlStatus::Success, stmt.BindInt(stmt.GetParameterIndex("InSpatialIndex"), element->GetModel()->IsTemplate() ? 0 : 1));

        auto isStep = stmt.Step();
        if (BeSQLite::DbResult::BE_SQLITE_DONE != isStep || model->GetDgnDb().GetModifiedRowCount() == 0)
            EXPECT_TRUE(false);
        stmt.Reset();
        stmt.ClearBindings();

        return element;
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     julius.cepukenas             10/2016
    //---------------------------------------------------------------------------------------
    ECSqlStatement GetStatement(DgnModelPtr model, BeSQLite::EC::ECCrudWriteToken const* token)
        {
        ECSqlStatement stmt;
        Utf8String insertECSql;
        DgnCategoryId catId = (*SpatialCategory::MakeIterator(model->GetDgnDb()).begin()).GetId<DgnCategoryId>();
        auto elmnt = GenericPhysicalObject::Create(*model->ToPhysicalModelP(), catId);
        auto ecClass = elmnt->GetElementClass();
        insertECSql = Utf8String("INSERT INTO ");
        insertECSql.append(ecClass->GetECSqlName()).append(" ([ECInstanceId], Model.Id, CodeSpec.Id, CodeScope.Id, Parent.Id, Category.Id,");
        Utf8String insertValuesSql(") VALUES (:[ECInstanceId], :Modelid, :CodeSpecId, :CodeScopeId, :ParentId, :CategoryId,");
        bool isFirstItem = true;
        for (auto prop : ecClass->GetProperties(true))
            {
            if (0 == strcmp("LastMod", prop->GetName().c_str()) || 0 == strcmp("UserProperties", prop->GetName().c_str()) || 0 == strcmp("PhysicalType", prop->GetName().c_str()))
                continue;
            if (!isFirstItem)
                {
                insertECSql.append(", ");
                insertValuesSql.append(", ");
                }

            insertECSql.append("[").append(prop->GetName()).append("]");
            insertValuesSql.append(":[").append(prop->GetName()).append("]");

            isFirstItem = false;
            }

        insertECSql.append(insertValuesSql).append(")");

        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(model->GetDgnDb(), insertECSql.c_str(), token));
        return stmt;
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                Algirdas.Mikoliunas             05/2017
    //---------------------------------------------------------------------------------------
    RefCountedPtr<TestMultiAspect> CreateAspect(DgnModelPtr model, Utf8CP aspectName="Test", bool registerDomain=true)
        {
        DgnCategoryId catId = (*SpatialCategory::MakeIterator(model->GetDgnDb()).begin()).GetId<DgnCategoryId>();
        RefCountedPtr<TestMultiAspect> aspect = TestMultiAspect::Create(aspectName);
        auto element = GenericPhysicalObject::Create(*model->ToPhysicalModelP(), catId);

        if (registerDomain)
            {
            m_domain = std::make_shared<MultiAspectHandlerDomain>();
            DgnDomains::RegisterDomain(*m_domain, DgnDomain::Required::No, DgnDomain::Readonly::No);
            }

        DgnElement::MultiAspect::AddAspect(*element, *aspect);
        auto newElement = model->GetDgnDb().Elements().Insert(*element);
        return aspect;
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     julius.cepukenas             10/2016
    //---------------------------------------------------------------------------------------
    bool ExpectCodesCount(Briefcase& briefcase, int expectedCount)
        {
        auto result = briefcase.GetiModelConnection().QueryCodesLocks(briefcase.GetBriefcaseId())->GetResult();
        EXPECT_SUCCESS(result);
        auto actualCount = result.GetValue().GetCodes().size();
        EXPECT_EQ(expectedCount, actualCount);
        return (result.IsSuccess() && expectedCount == actualCount);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     julius.cepukenas             10/2016
    //---------------------------------------------------------------------------------------
    bool ExpectCodesCount(Briefcase& briefcase, int expectedCount, DgnCodeSet& codes)
        {
        LockableIdSet ids;
        auto result = briefcase.GetiModelConnection().QueryCodesLocksById(codes, ids)->GetResult();
        EXPECT_SUCCESS(result);
        auto actualCount = result.GetValue().GetCodes().size();
        EXPECT_EQ(expectedCount, actualCount);
        return (result.IsSuccess() && expectedCount == actualCount);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     julius.cepukenas             10/2016
    //---------------------------------------------------------------------------------------
    Utf8String PushPendingChanges(Briefcase& briefcase, bool relinquishLocksCodes = false)
        {
        auto pushResult = briefcase.PullMergeAndPush(nullptr, relinquishLocksCodes)->GetResult();
        EXPECT_SUCCESS(pushResult);
        return briefcase.GetLastChangeSetPulled();
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     julius.cepukenas             10/2016
    //---------------------------------------------------------------------------------------
    bool ExpectLocksCount(Briefcase& briefcase, int expectedCount)
        {
        auto result = briefcase.GetiModelConnection().QueryCodesLocks(briefcase.GetBriefcaseId())->GetResult();
        EXPECT_SUCCESS(result);
        auto actualCount = result.GetValue().GetLocks().size();
        EXPECT_EQ(expectedCount, actualCount);
        return (result.IsSuccess() && expectedCount == actualCount);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     julius.cepukenas             10/2016
    //---------------------------------------------------------------------------------------
    bool ExpectLocksCount(Briefcase& briefcase, int expectedCount, LockableIdSet& locks)
        {
        DgnCodeSet codes;
        auto result = briefcase.GetiModelConnection().QueryCodesLocksById(codes, locks)->GetResult();
        EXPECT_SUCCESS(result);
        auto actualCount = result.GetValue().GetLocks().size();
        EXPECT_EQ(expectedCount, actualCount);
        return (result.IsSuccess() && expectedCount == actualCount);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     julius.cepukenas             02/2017
    //---------------------------------------------------------------------------------------
    DgnCodeSet RetrieveCodesHelper(int postRequestSize, int getRequestSize, BriefcasePtr briefcase)
        {
        //Prapare imodel
        DgnDbR db = briefcase->GetDgnDb();

        DgnCodeSet requestCodesSet;
        DgnCodeSet fullCodeSet;

        int index = 0;
        //Create and then acquire many codes
        for (int i = 0; i < getRequestSize; i++)
            {
            auto code = GetNextCode(db);
            requestCodesSet.insert(code);
            fullCodeSet.insert(code);

            index++;
            if (index == postRequestSize)
                {
                EXPECT_STATUS(Success, db.BriefcaseManager().ReserveCodes(requestCodesSet).Result());
                requestCodesSet.clear();
                index = 0;
                }
            }

        EXPECT_STATUS(Success, db.BriefcaseManager().ReserveCodes(requestCodesSet).Result());

        return fullCodeSet;
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     julius.cepukenas             02/2017
    //---------------------------------------------------------------------------------------
    LockableIdSet RetrieveLocksHelper(int postRequestSize, int getRequestSize, BriefcasePtr briefcase, DgnModelPtr model, BeSQLite::EC::ECCrudWriteToken const* token)
        {
        //Prapare imodel
        DgnDbR db = briefcase->GetDgnDb();
        auto stmt = GetStatement(model, token);

        int index = 0;
        //Create and then acquire many locks
        for (int i = 0; i < getRequestSize; i++)
            {
            CreateElement(model, stmt);
            }

        db.SaveChanges();

        LockRequest req;
        LockableIdSet queryLockSet;
        for (auto const& elem : model->MakeIterator())
            {
            DgnLock lock(elem.GetElementId(), LockLevel::Exclusive);
            req.GetLockSet().insert(lock);
            queryLockSet.insert(LockableId(elem.GetElementId()));

            index++;
            if (index == postRequestSize)
                {
                briefcase->PullAndMerge(nullptr, false)->GetResult().IsSuccess();
                EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().AcquireLocks(req, IBriefcaseManager::ResponseOptions::None).Result());
                req.Clear();
                index = 0;
                }
            }

        EXPECT_SUCCESS(briefcase->PullAndMerge(nullptr, false)->GetResult());
        EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().AcquireLocks(req, IBriefcaseManager::ResponseOptions::None).Result());

        return queryLockSet;
        }
    };

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(PerformanceTests, ReserveCodes)
    {
    //Prapare imodel and acquire briefcases
    DgnDbR db = m_briefcase->GetDgnDb();
    auto postRequestSize = PerformanceTestSettings::Instance().GetCodePostRequestSize();

    DgnCodeSet codesSet;
    //Create and then acquire many codes
    for (int i = 0; i < postRequestSize; i++)
        {
        codesSet.insert(GetNextCode(db));
        }

    StopWatch timer(true);
    auto result = db.BriefcaseManager().ReserveCodes(codesSet).Result();
    EXPECT_STATUS(Success, result);
    timer.Stop();
    LogTiming(timer, postRequestSize, result == RepositoryStatus::Success);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(PerformanceTests, RetrieveCodes)
    {
    auto postRequestSize = PerformanceTestSettings::Instance().GetCodePostRequestSize();
    auto getRequestSize = PerformanceTestSettings::Instance().GetCodeGetRequestSize();

    RetrieveCodesHelper(postRequestSize, getRequestSize, m_briefcase);

    StopWatch timer(true);
    auto isPassed = ExpectCodesCount(*m_briefcase, getRequestSize + 1); //One model code
    timer.Stop();
    LogTiming(timer, getRequestSize, isPassed);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(PerformanceTests, RetrieveFractionOfAllAvailableCodes)
    {
    auto postRequestSize = PerformanceTestSettings::Instance().GetCodePostRequestSize();
    auto getRequestSize = PerformanceTestSettings::Instance().GetCodeGetRequestSize();
    auto totalGetSize = getRequestSize;

    RetrieveCodesHelper(postRequestSize, getRequestSize, m_briefcase);

    auto secondBriefcase = IntegrationTestsBase::AcquireBriefcase(*m_client, *m_imodel, true);

    getRequestSize = PerformanceTestSettings::Instance().GetCodeGetRequestSizeSecondCall();
    totalGetSize += getRequestSize;

    RetrieveCodesHelper(postRequestSize, getRequestSize, secondBriefcase);

    StopWatch timer(true);
    auto isPassed = ExpectCodesCount(*secondBriefcase, getRequestSize);
    timer.Stop();
    LogTiming(timer, getRequestSize, totalGetSize, isPassed);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(PerformanceTests, RetrieveCodesById)
    {
    auto postRequestSize = PerformanceTestSettings::Instance().GetCodePostRequestSize();
    auto getRequestSize = PerformanceTestSettings::Instance().GetCodeGetByIdRequestSize();

    auto fullCodeSet = RetrieveCodesHelper(postRequestSize, getRequestSize, m_briefcase);

    StopWatch timer(true);
    auto isPassed = ExpectCodesCount(*m_briefcase, getRequestSize, fullCodeSet);
    timer.Stop();
    LogTiming(timer, getRequestSize, isPassed);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             10/2016
//---------------------------------------------------------------------------------------
TEST_F(PerformanceTests, RetrieveFractionOfAllAvailableCodesById)
    {
    auto postRequestSize = PerformanceTestSettings::Instance().GetCodePostRequestSize();
    auto getRequestSize = PerformanceTestSettings::Instance().GetCodeGetByIdRequestSize();
    auto totalGetSize = getRequestSize;

    RetrieveCodesHelper(postRequestSize, getRequestSize, m_briefcase);

    getRequestSize = PerformanceTestSettings::Instance().GetCodeGetByIdRequestSizeSecondCall();
    totalGetSize += getRequestSize;

    auto fullCodeSet = RetrieveCodesHelper(postRequestSize, getRequestSize, m_briefcase);

    StopWatch timer(true);
    auto isPassed = ExpectCodesCount(*m_briefcase, getRequestSize, fullCodeSet);
    timer.Stop();
    LogTiming(timer, getRequestSize, totalGetSize, isPassed);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
TEST_F(PerformanceTests, PushLocks)
    {
    //Prapare imodel and acquire briefcases
    auto postRequestSize = PerformanceTestSettings::Instance().GetLockPostRequestSize();

    DgnDbR db = m_briefcase->GetDgnDb();
    auto aspect = CreateAspect(m_model);
    auto sqlStatement = GetStatement(m_model, aspect->GetToken());

    //Create and then acquire
    for (int i = 0; i < postRequestSize; i++)
        {
        CreateElement(m_model, sqlStatement);
        }

    StopWatch timer(true);
    db.SaveChanges();
    auto result = m_briefcase->PullMergeAndPush(nullptr, false)->GetResult();
    ExpectLocksCount(*m_briefcase, postRequestSize + 5); // Was +6 before, changed to +5
    EXPECT_SUCCESS(result);
    timer.Stop();
    LogTiming(timer, postRequestSize, result.IsSuccess());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
TEST_F(PerformanceTests, AcquireLocks)
    {
    //Prapare imodel and acquire briefcases
    auto postRequestSize = PerformanceTestSettings::Instance().GetLockPostRequestSize();

    DgnDbR db = m_briefcase->GetDgnDb();
    auto aspect = CreateAspect(m_model);
    auto sqlStatement = GetStatement(m_model, aspect->GetToken());

    for (int i = 0; i < postRequestSize; i++)
        {
        CreateElement(m_model, sqlStatement);
        }

    db.SaveChanges();
    LockRequest req;

    for (auto const& elem : m_model->MakeIterator())
        {
        DgnLock lock(elem.GetElementId(), LockLevel::Exclusive);
        req.GetLockSet().insert(lock);
        }

    StopWatch timer(true);

    EXPECT_SUCCESS(m_briefcase->PullAndMerge(nullptr, false)->GetResult());
    auto result = db.BriefcaseManager().AcquireLocks(req, IBriefcaseManager::ResponseOptions::None).Result();
    EXPECT_EQ(RepositoryStatus::Success, result);
    timer.Stop();
    LogTiming(timer, postRequestSize, RepositoryStatus::Success == result);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
TEST_F(PerformanceTests, RetrieveLocks)
    {
    auto postRequestSize = PerformanceTestSettings::Instance().GetLockPostRequestSize();
    auto getRequestSize = PerformanceTestSettings::Instance().GetLockGetRequestSize();
    auto aspect = CreateAspect(m_model);

    RetrieveLocksHelper(postRequestSize, getRequestSize, m_briefcase, m_model, aspect->GetToken());

    StopWatch timer(true);
    auto isPassed = ExpectLocksCount(*m_briefcase, getRequestSize + 2); //One model lock
    timer.Stop();
    LogTiming(timer, getRequestSize, isPassed);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
TEST_F(PerformanceTests, RetrieveFractionOfAllAvailableLocks)
    {
    auto postRequestSize = PerformanceTestSettings::Instance().GetLockPostRequestSize();
    auto getRequestSize = PerformanceTestSettings::Instance().GetLockGetRequestSize();
    auto totalGetSize = getRequestSize;
    auto aspect = CreateAspect(m_model);
    m_briefcase->GetiModelConnection().RelinquishCodesLocks(m_briefcase->GetBriefcaseId());

    RetrieveLocksHelper(postRequestSize, getRequestSize, m_briefcase, m_model, aspect->GetToken());

    auto secondBriefcase = IntegrationTestsBase::AcquireBriefcase(*m_client, *m_imodel, true);
    auto secondModel = CreateModel("Second-Model", secondBriefcase->GetDgnDb());
    aspect = CreateAspect(secondModel, "Second", false);

    getRequestSize = PerformanceTestSettings::Instance().GetLockGetRequestSizeSecondCall();
    totalGetSize += getRequestSize;

    RetrieveLocksHelper(postRequestSize, getRequestSize, secondBriefcase, secondModel, aspect->GetToken());

    StopWatch timer(true);
    auto isPassed = ExpectLocksCount(*secondBriefcase, getRequestSize + 2); //One model lock
    timer.Stop();
    LogTiming(timer, getRequestSize, totalGetSize, isPassed);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
TEST_F(PerformanceTests, RetrieveLocksById)
    {
    auto postRequestSize = PerformanceTestSettings::Instance().GetLockPostRequestSize();
    auto getRequestSize = PerformanceTestSettings::Instance().GetLockGetByIdRequestSize();
    auto aspect = CreateAspect(m_model);

    auto queryLockSet = RetrieveLocksHelper(postRequestSize, getRequestSize, m_briefcase, m_model, aspect->GetToken());

    StopWatch timer(true);
    auto isPassed = ExpectLocksCount(*m_briefcase, getRequestSize, queryLockSet);
    timer.Stop();
    LogTiming(timer, getRequestSize, isPassed);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
TEST_F(PerformanceTests, RetrieveFractionOfAllAvailableLocksById)
    {
    auto postRequestSize = PerformanceTestSettings::Instance().GetLockPostRequestSize();
    auto getRequestSize = PerformanceTestSettings::Instance().GetLockGetByIdRequestSize();
    auto totalGetSize = getRequestSize;
    auto aspect = CreateAspect(m_model);

    RetrieveLocksHelper(postRequestSize, getRequestSize, m_briefcase, m_model, aspect->GetToken());

    getRequestSize = PerformanceTestSettings::Instance().GetLockGetByIdRequestSizeSecondCall();
    totalGetSize += getRequestSize;

    auto secondModel = CreateModel("Second-Model", m_briefcase->GetDgnDb());
    auto queryLockSet = RetrieveLocksHelper(postRequestSize, getRequestSize, m_briefcase, secondModel, aspect->GetToken());
    EXPECT_EQ(getRequestSize, queryLockSet.size());
    StopWatch timer(true);
    auto isPassed = ExpectLocksCount(*m_briefcase, getRequestSize, queryLockSet); //Two model locks
    timer.Stop();
    LogTiming(timer, getRequestSize, totalGetSize, isPassed);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             11/2016
//---------------------------------------------------------------------------------------
TEST_F(PerformanceTests, PullMergeAndPush)
    {
    bset<BriefcasePtr> briefcases;
    const int briefcaseCount = 20;

    WString imodelIdWide;
    BeStringUtilities::Utf8ToWChar(imodelIdWide, m_imodel->GetId().c_str());

    PROCESS_INFORMATION pifs[briefcaseCount];
    BeFileName imhsClientPath;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(imhsClientPath);
    imhsClientPath.AppendToPath(L"..\\IMHSClientExe.exe");

    imodelIdWide = imhsClientPath + L" " + imodelIdWide;
    LPWSTR imodelId = const_cast<wchar_t *>(imodelIdWide.c_str());
    LPCWSTR imhsExePath = imhsClientPath.c_str();

    StopWatch timer(true);
    // Start all processes
    for (int i = 0; i < briefcaseCount; i++)
        {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        if (!CreateProcess(imhsExePath, imodelId, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
            {
            std::cout << "CreateProcess failed (%d).\n", GetLastError();
            return;
            }

        pifs[i] = pi;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

    // Wait for processes to finish
    for (int i = 0; i < briefcaseCount; i++)
        {
        PROCESS_INFORMATION pi = pifs[i];
        WaitForSingleObject(pi.hProcess, INFINITE); 
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        }

    timer.Stop();
    double testTime = timer.GetElapsedSeconds();
    std::cout << "Test finished in " << testTime << ".\n";
    EXPECT_LT(testTime, 50);
    }
