/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/PerformanceTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "Helpers.h"
#include "iModelTestsBase.h"
#include <DgnPlatform/DgnMaterial.h>
#include <DgnPlatform/GenericDomain.h>
#include <ECDb/ECInstanceId.h>
#include <ECDb/ECSqlStatement.h>
#include <ECDb/ECSqlStatus.h>
#include <ECObjects/ECSchema.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTimeUtilities.h>
#include <windows.h>
#include "../Helpers/Domains/DgnPlatformTestDomain.h"

USING_NAMESPACE_BENTLEY_DGN

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST

#define EXPECT_STATUS(STAT, EXPR) EXPECT_EQ(RepositoryStatus:: STAT, (EXPR))
/*--------------------------------------------------------------------------------------+
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

DOMAIN_DEFINE_MEMBERS(MultiAspectHandlerDomain);

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             10/2016
//---------------------------------------------------------------------------------------
struct PerformanceTests : public IntegrationTestsBase
    {
    iModelInfoPtr m_info;
    static std::shared_ptr<MultiAspectHandlerDomain> s_domain;
    BriefcasePtr m_briefcase;
    DgnModelPtr m_model;

    int m_codeValueIndex = 0;
    int m_codeValueBatch = 0;

    uint64_t s_elementId = UINT64_C(2000000);
    
    static void SetUpTestCase()
        {
        IntegrationTestsBase::SetUpTestCase();
        s_domain = std::make_shared<MultiAspectHandlerDomain>();
        DgnDomains::RegisterDomain(*s_domain, DgnDomain::Required::No, DgnDomain::Readonly::No);
        }

    static void TearDownTestCase()
        {
        IntegrationTestsBase::TearDownTestCase();
        }

    virtual void SetUp() override
        {
        IntegrationTestsBase::SetUp();
        ASSERT_SUCCESS(iModelHubHelpers::DeleteiModelByName(s_client, GetTestiModelName()));

        DgnDbPtr db = CreateTestDb();
        ASSERT_TRUE(db.IsValid());
        MultiAspectHandlerDomain::GetDomain().ImportSchema(*db);
        ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db->SaveChanges());

        iModelResult imodelResult = CreateiModel(db);
        ASSERT_SUCCESS(imodelResult);
        m_info = imodelResult.GetValue();

        BriefcaseResult briefcaseResult = iModelHubHelpers::AcquireAndOpenBriefcase(s_client, m_info);
        ASSERT_SUCCESS(briefcaseResult);
        m_briefcase = briefcaseResult.GetValue();
        m_model = CreateModel(TestCodeName().c_str(), m_briefcase->GetDgnDb());

        auto pushResult = m_briefcase->PullMergeAndPush(nullptr, true)->GetResult();
        ASSERT_SUCCESS(pushResult);
        }

    virtual void TearDown() override
        {
        if (m_info.IsValid())
            {
            m_info = nullptr;
            }
        if (m_briefcase.IsValid())
            {
            m_briefcase = nullptr;
            }
        if (m_model.IsValid())
            {
            m_model = nullptr;
            }
        iModelHubHelpers::DeleteiModelByName(s_client, GetTestiModelName());
        IntegrationTestsBase::TearDown();
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     julius.cepukenas             10/2016
    //---------------------------------------------------------------------------------------
    void LogTiming(StopWatch& timer, int numberOfInstances, int total, bool isPassed, int counter)
        {
        Utf8String description;
        description.Sprintf("Test searched for [%d] of [%d] instances for the time no. %d", numberOfInstances, total, counter);
        LogTiming(timer, description, isPassed);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     julius.cepukenas             10/2016
    //---------------------------------------------------------------------------------------
    void LogTiming(StopWatch& timer, int numberOfInstances, bool isPassed)
        {
        Utf8String description;
        description.Sprintf("Test used [%d] number of instances", numberOfInstances);
        LogTiming(timer, description, isPassed);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     Benas.Kikutis             01/2018
    //---------------------------------------------------------------------------------------
    void LogTiming(StopWatch& timer, int numberOfInstances, bool isPassed, int counter, Utf8String method)
        {
        Utf8String description;
        description.Sprintf("Test part used [%d] number of instances for the time no. %d with %s method", numberOfInstances, counter, method.c_str());
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

        totalDescription.Sprintf("Test %s. %s", status.c_str(), description.c_str());
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
    void CreateElement(GenericPhysicalObjectPtr& element, DgnModelPtr model, ECSqlStatement& stmt)
        {
        const ECInstanceId id(s_elementId++);
        DgnCategoryId catId = (*SpatialCategory::MakeIterator(model->GetDgnDb()).begin()).GetId<DgnCategoryId>();
        element = GenericPhysicalObject::Create(*model->ToPhysicalModelP(), catId);

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(stmt.GetParameterIndex("ECInstanceId"), id));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(stmt.GetParameterIndex("Modelid"), element->GetModelId()));

        if (!element->GetFederationGuid().IsValid())
            ASSERT_EQ(ECSqlStatus::Success, stmt.BindNull(stmt.GetParameterIndex("FederationGuid")));

        // Bind Code
        DgnCode elementCode = DgnCode::CreateEmpty();

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindNull(stmt.GetParameterIndex("CodeValue")));

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(stmt.GetParameterIndex("CodeSpecId"), elementCode.GetCodeSpecId()));

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(stmt.GetParameterIndex("CodeScopeId"), elementCode.GetScopeElementId(model->GetDgnDb())));

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindNull(stmt.GetParameterIndex("UserLabel")));

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(stmt.GetParameterIndex("ParentId"), element->GetParentId()));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(stmt.GetParameterIndex("CategoryId"), element->ToGeometrySource()->GetCategoryId()));

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(stmt.GetParameterIndex("InSpatialIndex"), element->GetModel()->IsTemplate() ? 0 : 1));

        auto isStep = stmt.Step();
        if (BeSQLite::DbResult::BE_SQLITE_DONE != isStep || model->GetDgnDb().GetModifiedRowCount() == 0)
            ASSERT_TRUE(false);
        stmt.Reset();
        stmt.ClearBindings();
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
    RefCountedPtr<TestMultiAspect> CreateAspect(DgnModelPtr model, Utf8CP aspectName="Test")
        {
        DgnCategoryId catId = (*SpatialCategory::MakeIterator(model->GetDgnDb()).begin()).GetId<DgnCategoryId>();
        RefCountedPtr<TestMultiAspect> aspect = TestMultiAspect::Create(aspectName);
        auto element = GenericPhysicalObject::Create(*model->ToPhysicalModelP(), catId);

        DgnElement::MultiAspect::AddAspect(*element, *aspect);

        IBriefcaseManager::Request req;
        RepositoryStatus statusR = model->GetDgnDb().BriefcaseManager().PrepareForElementInsert(req, *element, IBriefcaseManager::PrepareAction::Acquire);
        auto newElement = model->GetDgnDb().Elements().Insert(*element);
        return aspect;
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     Benas.Kikutis             01/2018
    //---------------------------------------------------------------------------------------
    bool ExpectAllCodesCount(Briefcase& briefcase, int expectedCount)
        {
        auto result = briefcase.GetiModelConnection().QueryAllCodes()->GetResult();
        EXPECT_SUCCESS(result);
        auto actualCount = result.GetValue().size();
        EXPECT_EQ(expectedCount, actualCount);
        return (result.IsSuccess() && expectedCount == actualCount);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     julius.cepukenas             10/2016
    //---------------------------------------------------------------------------------------
    bool ExpectCodesCountByBriefcase(Briefcase& briefcase, int expectedCount)
        {
        auto result = briefcase.GetiModelConnection().QueryCodesByBriefcaseId(briefcase.GetBriefcaseId())->GetResult();
        EXPECT_SUCCESS(result);
        auto actualCount = result.GetValue().size();
        EXPECT_EQ(expectedCount, actualCount);
        return (result.IsSuccess() && expectedCount == actualCount);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     julius.cepukenas             10/2016
    //---------------------------------------------------------------------------------------
    bool ExpectCodesCountByIdsAndBriefcase(Briefcase& briefcase, int expectedCount, DgnCodeSet& codes)
        {
        auto result = briefcase.GetiModelConnection().QueryCodesByIds(codes)->GetResult();
        EXPECT_SUCCESS(result);
        auto actualCount = result.GetValue().size();
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
    //@bsimethod                                     Benas.Kikutis             01/2018
    //---------------------------------------------------------------------------------------
    bool ExpectAllLocksCount(Briefcase& briefcase, int expectedCount)
        {
        auto result = briefcase.GetiModelConnection().QueryAllLocks()->GetResult();
        EXPECT_SUCCESS(result);
        auto actualCount = result.GetValue().size();
        EXPECT_EQ(expectedCount, actualCount);
        return (result.IsSuccess() && expectedCount == actualCount);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     julius.cepukenas             10/2016
    //---------------------------------------------------------------------------------------
    bool ExpectLocksCountByBriefcase(Briefcase& briefcase, int expectedCount)
        {
        auto result = briefcase.GetiModelConnection().QueryLocksByBriefcaseId(briefcase.GetBriefcaseId())->GetResult();
        EXPECT_SUCCESS(result);
        auto actualCount = result.GetValue().size();
        EXPECT_EQ(expectedCount, actualCount);
        return (result.IsSuccess() && expectedCount == actualCount);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                     julius.cepukenas             10/2016
    //---------------------------------------------------------------------------------------
    bool ExpectLocksCountByIdsAndBriefcase(Briefcase& briefcase, int expectedCount, LockableIdSet& locks)
        {
        auto result = briefcase.GetiModelConnection().QueryLocksByIds(locks)->GetResult();
        EXPECT_SUCCESS(result);
        auto actualCount = result.GetValue().size();
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

        int counter = 0;
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
                auto result = db.BriefcaseManager().ReserveCodes(requestCodesSet).Result();
                EXPECT_STATUS(Success, result);
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
            GenericPhysicalObjectPtr element;
            CreateElement(element, model, stmt);
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
std::shared_ptr<MultiAspectHandlerDomain> PerformanceTests::s_domain = nullptr;

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
//@bsimethod                                     Benas.Kikutis             01/2018
//---------------------------------------------------------------------------------------
TEST_F(PerformanceTests, RetrieveCodes)
    {
    auto postRequestSize = PerformanceTestSettings::Instance().GetCodePostRequestSize();
    auto finalGetRequestSize = PerformanceTestSettings::Instance().GetCodeGetRequestSize();
    auto getRequestAttempts = PerformanceTestSettings::Instance().GetCodeGetAttemptsCount();
    auto splitMaxGetRequestSize = PerformanceTestSettings::Instance().GetCodeGetSplitCount();
    auto oneGetRequestSize = finalGetRequestSize / splitMaxGetRequestSize;

    auto codesCountBeforePush = m_briefcase->GetiModelConnection().QueryAllCodes()->GetResult().GetValue().size();

    auto currentGetRequestSize = 0;
    while (currentGetRequestSize < finalGetRequestSize)
        {
        currentGetRequestSize += oneGetRequestSize;
        RetrieveCodesHelper(postRequestSize, oneGetRequestSize, m_briefcase);

        auto i = 0;
        while (i < getRequestAttempts)
            {
            StopWatch timer(true);
            auto isPassed = ExpectAllCodesCount(*m_briefcase, currentGetRequestSize + codesCountBeforePush);
            timer.Stop();
            LogTiming(timer, currentGetRequestSize, isPassed, ++i, "GET");
            }
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis             01/2018
//---------------------------------------------------------------------------------------
TEST_F(PerformanceTests, RetrieveFractionOfAllAvailableCodes)
    {
    auto postRequestSize = PerformanceTestSettings::Instance().GetCodePostRequestSize();
    auto finalGetRequestSize = PerformanceTestSettings::Instance().GetCodeGetRequestSize();
    auto getRequestAttempts = PerformanceTestSettings::Instance().GetCodeGetAttemptsCount();
    auto splitMaxGetRequestSize = PerformanceTestSettings::Instance().GetCodeGetSplitCount();
    auto oneGetRequestSize = finalGetRequestSize / splitMaxGetRequestSize;
    auto totalGetSize = finalGetRequestSize;

    BriefcaseResult briefcaseResult = iModelHubHelpers::AcquireAndOpenBriefcase(s_client, m_info);
    ASSERT_SUCCESS(briefcaseResult);
    auto secondBriefcase = briefcaseResult.GetValue();

    auto currentGetRequestSize = 0;
    while (currentGetRequestSize < finalGetRequestSize)
        {
        currentGetRequestSize += oneGetRequestSize;
        
        RetrieveCodesHelper(postRequestSize, oneGetRequestSize, m_briefcase);
        RetrieveCodesHelper(postRequestSize, oneGetRequestSize, secondBriefcase);

        auto i = 0;
        while (i < getRequestAttempts)
            {
            StopWatch timer(true);
            auto isPassed = ExpectCodesCountByBriefcase(*secondBriefcase, currentGetRequestSize);
            timer.Stop();
            LogTiming(timer, currentGetRequestSize, currentGetRequestSize * 2, isPassed, ++i);
            }
        }
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
    auto isPassed = ExpectCodesCountByIdsAndBriefcase(*m_briefcase, getRequestSize, fullCodeSet);
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
    auto isPassed = ExpectCodesCountByIdsAndBriefcase(*m_briefcase, getRequestSize, fullCodeSet);
    timer.Stop();
    LogTiming(timer, getRequestSize, totalGetSize, isPassed, 0);
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

    db.SaveChanges();
    auto result = m_briefcase->PullMergeAndPush(nullptr, true)->GetResult();
    ASSERT_SUCCESS(result);

    //Create and then acquire
    for (int i = 0; i < postRequestSize; i++)
        {
        GenericPhysicalObjectPtr element;
        CreateElement(element, m_model, sqlStatement);
        }

    StopWatch timer(true);
    db.SaveChanges();
    result = m_briefcase->PullMergeAndPush(nullptr, false)->GetResult();
    ExpectAllLocksCount(*m_briefcase, postRequestSize + 2);
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
        GenericPhysicalObjectPtr element;
        CreateElement(element, m_model, sqlStatement);
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
//@bsimethod                                     Benas.Kikutis             01/2018
//---------------------------------------------------------------------------------------
TEST_F(PerformanceTests, RetrieveLocks)
    {
    auto postRequestSize = PerformanceTestSettings::Instance().GetLockPostRequestSize();
    auto finalGetRequestSize = PerformanceTestSettings::Instance().GetLockGetRequestSize();
    auto getRequestAttempts = PerformanceTestSettings::Instance().GetLockGetAttemptsCount();
    auto splitMaxGetRequestSize = PerformanceTestSettings::Instance().GetLockGetSplitCount();
    auto oneGetRequestSize = finalGetRequestSize / splitMaxGetRequestSize;
    auto aspect = CreateAspect(m_model);

    auto locksCountBeforePush = m_briefcase->GetiModelConnection().QueryAllLocks()->GetResult().GetValue().size();

    auto currentGetRequestSize = 0;
    while (currentGetRequestSize < finalGetRequestSize)
        {
        currentGetRequestSize += oneGetRequestSize;
        RetrieveLocksHelper(postRequestSize, oneGetRequestSize, m_briefcase, m_model, aspect->GetToken());

        auto i = 0;
        while (i < getRequestAttempts)
            {
            StopWatch timer(true);
            auto isPassed = ExpectAllLocksCount(*m_briefcase, currentGetRequestSize + locksCountBeforePush);
            timer.Stop();
            LogTiming(timer, currentGetRequestSize, isPassed, ++i, "GET");
            }
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Benas.Kikutis             01/2018
//---------------------------------------------------------------------------------------
TEST_F(PerformanceTests, RetrieveFractionOfAllAvailableLocks)
    {
    auto postRequestSize = PerformanceTestSettings::Instance().GetLockPostRequestSize();
    auto finalGetRequestSize = PerformanceTestSettings::Instance().GetLockGetRequestSize();
    auto getRequestAttempts = PerformanceTestSettings::Instance().GetLockGetAttemptsCount();
    auto splitMaxGetRequestSize = PerformanceTestSettings::Instance().GetLockGetSplitCount();
    auto oneGetRequestSize = finalGetRequestSize / splitMaxGetRequestSize;
    auto totalGetSize = finalGetRequestSize;
    auto aspectModel1 = CreateAspect(m_model);

    auto locksCountBeforePush = m_briefcase->GetiModelConnection().QueryAllLocks()->GetResult().GetValue().size();

    m_briefcase->GetiModelConnection().RelinquishCodesLocks(m_briefcase->GetBriefcaseId());
    BriefcaseResult briefcaseResult = iModelHubHelpers::AcquireAndOpenBriefcase(s_client, m_info);
    ASSERT_SUCCESS(briefcaseResult);
    auto secondBriefcase = briefcaseResult.GetValue();
    auto secondModel = CreateModel(TestCodeName(1).c_str(), secondBriefcase->GetDgnDb());

    auto pushResult = secondBriefcase->PullMergeAndPush(nullptr, true)->GetResult();
    ASSERT_SUCCESS(pushResult);

    auto aspectModel2 = CreateAspect(secondModel, TestCodeName(2).c_str());

    auto currentGetRequestSize = 0;
    while (currentGetRequestSize < finalGetRequestSize)
        {
        currentGetRequestSize += oneGetRequestSize;

        RetrieveLocksHelper(postRequestSize, oneGetRequestSize, m_briefcase, m_model, aspectModel1->GetToken());
        RetrieveLocksHelper(postRequestSize, oneGetRequestSize, secondBriefcase, secondModel, aspectModel2->GetToken());

        auto i = 0;
        while (i < getRequestAttempts)
            {
            StopWatch timer(true);
            auto isPassed = ExpectLocksCountByBriefcase(*secondBriefcase, currentGetRequestSize + locksCountBeforePush);
            timer.Stop();
            LogTiming(timer, currentGetRequestSize, currentGetRequestSize * 2, isPassed, ++i);
            }
        }
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
    auto isPassed = ExpectLocksCountByIdsAndBriefcase(*m_briefcase, getRequestSize, queryLockSet);
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

    auto secondModel = CreateModel(TestCodeName(1).c_str(), m_briefcase->GetDgnDb());
    auto pushResult = m_briefcase->PullMergeAndPush(nullptr, true)->GetResult();
    ASSERT_SUCCESS(pushResult);

    auto queryLockSet = RetrieveLocksHelper(postRequestSize, getRequestSize, m_briefcase, secondModel, aspect->GetToken());
    EXPECT_EQ(getRequestSize, queryLockSet.size());
    StopWatch timer(true);
    auto isPassed = ExpectLocksCountByIdsAndBriefcase(*m_briefcase, getRequestSize, queryLockSet); //Two model locks
    timer.Stop();
    LogTiming(timer, getRequestSize, totalGetSize, isPassed, 0);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             11/2016
//---------------------------------------------------------------------------------------
TEST_F(PerformanceTests, PullMergeAndPush)
    {
    bset<BriefcasePtr> briefcases;
    const int briefcaseCount = 20;

    WString imodelIdWide;
    BeStringUtilities::Utf8ToWChar(imodelIdWide, m_info->GetId().c_str());

    WString projectIdWide;
    BeStringUtilities::Utf8ToWChar(projectIdWide, IntegrationTestsSettings::Instance().GetProjectId().c_str());

    PROCESS_INFORMATION pifs[briefcaseCount];
    BeFileName imhsClientPath;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(imhsClientPath);
    imhsClientPath.AppendToPath(L"..\\IMHSClientExe.exe");

    imodelIdWide = imhsClientPath + L" " + projectIdWide + L" " + imodelIdWide;
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
