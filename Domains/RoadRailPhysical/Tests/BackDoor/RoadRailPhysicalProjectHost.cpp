#include "../BackDoor/PublicApi/BackDoor/RoadRailPhysical/BackDoor.h"
#include <DgnPlatform/DgnPlatformLib.h>

#include <DgnPlatform\DgnGeoCoord.h>

BEGIN_BENTLEY_ROADRAILPHYSICAL_UNITTESTS_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct TestKnownLocationsAdmin : DgnPlatformLib::Host::IKnownLocationsAdmin
{
BeFileName m_tempDir;
BeFileName m_assetsDir;

virtual BeFileNameCR _GetLocalTempDirectoryBaseName() override { return m_tempDir; }
virtual BeFileNameCR _GetDgnPlatformAssetsDirectory() override { return m_assetsDir; }

TestKnownLocationsAdmin()
    {
    BeTest::GetHost().GetTempDir(m_tempDir);
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(m_assetsDir);
    }
};


//=======================================================================================
// @bsiclass
//! Refer to http://bsw-wiki.bentley.com/bin/view.pl/Main/DgnDbServerClientAPIExamples
//! This implementation bypasses all the checks for locks and codes
//=======================================================================================
struct RepositoryManager : IRepositoryManager
{
protected:
    virtual Response _ProcessRequest(Request const& req, DgnDbR db, bool queryOnly) override { return Response(queryOnly ? IBriefcaseManager::RequestPurpose::Query : IBriefcaseManager::RequestPurpose::Acquire, req.Options(), RepositoryStatus::Success); }
    virtual RepositoryStatus _Demote(DgnLockSet const&, DgnCodeSet const&, DgnDbR) override { return RepositoryStatus::Success; }
    virtual RepositoryStatus _Relinquish(Resources, DgnDbR) override { return RepositoryStatus::Success; }
    virtual RepositoryStatus _QueryHeldResources(DgnLockSet& locks, DgnCodeSet& codes, DgnLockSet& unavailableLocks, DgnCodeSet& unavailableCodes, DgnDbR db) override { return RepositoryStatus::Success; }
    virtual RepositoryStatus _QueryStates(DgnLockInfoSet&, DgnCodeInfoSet& codeStates, LockableIdSet const& locks, DgnCodeSet const& codes) override { return RepositoryStatus::Success; }
};

//=======================================================================================
// @bsiclass                            Alexandre.Gagnon                        04/2016
//=======================================================================================
struct TestRepositoryAdmin : DgnPlatformLib::Host::RepositoryAdmin
    {
    private:
        mutable RepositoryManager m_client;

    protected:
        virtual IRepositoryManagerP _GetRepositoryManager(DgnDbR db) const override { return &m_client; }
    };



//=======================================================================================
// @bsiclass
//=======================================================================================
struct RoadRailPhysicalProjectHostImpl : DgnPlatformLib::Host
    {
    bool m_isInitialized;

    RoadRailPhysicalProjectHostImpl();
    ~RoadRailPhysicalProjectHostImpl();

    virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new TestKnownLocationsAdmin(); }
    virtual RepositoryAdmin & _SupplyRepositoryAdmin() override { return *new TestRepositoryAdmin(); }

    //virtual HandlerAdmin& _SupplyHandlerAdmin() override { return *new TestHandlerAdmin; }
    virtual void _SupplyProductName(Utf8StringR name) override { name.assign("RoadRailPhysicalProjectHost"); }
    virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() { return BeSQLite::L10N::SqlangFiles(BeFileName()); } // no translatable strings
    };

END_BENTLEY_ROADRAILPHYSICAL_UNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
RoadRailPhysicalProjectHost::RoadRailPhysicalProjectHost()
    {
    m_pimpl = new RoadRailPhysicalProjectHostImpl;
    CleanOutputDirectory();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
RoadRailPhysicalProjectHost::~RoadRailPhysicalProjectHost()
    {
    delete m_pimpl;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
void RoadRailPhysicalProjectHost::CleanOutputDirectory()
    {
    static bool wasOutputDirectoryCleaned = false;
    if (!wasOutputDirectoryCleaned)
        {
        // clean up files from last run
        BeFileName outputDir = GetOutputDirectory();
        BeFileName::EmptyAndRemoveDirectory(outputDir.GetName());
        BeFileName::CreateNewDirectory(outputDir.GetName());
        wasOutputDirectoryCleaned = true;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
BeFileName RoadRailPhysicalProjectHost::GetOutputDirectory()
    {
    BeFileName outputDir;
    BeTest::GetHost().GetOutputRoot(outputDir);
    outputDir.AppendToPath(L"RoadRailPhysical");
    return outputDir;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
BeFileName RoadRailPhysicalProjectHost::GetDgnPlatformAssetsDirectory()
    {
    BeFileName assetsRootDirectory;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsRootDirectory);
    return assetsRootDirectory;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
BeFileName RoadRailPhysicalProjectHost::BuildProjectFileName(WCharCP baseName)
    {
    BeFileName projectFileName = GetOutputDirectory();
    projectFileName.AppendToPath(baseName);
    return projectFileName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
DgnDbPtr RoadRailPhysicalProjectHost::CreateProject(WCharCP baseName)
    {
    CreateDgnDbParams createDgnDbParams;
    createDgnDbParams.SetOverwriteExisting(true);
    createDgnDbParams.SetRootSubjectName("RoadRailPhysicalProject");
    createDgnDbParams.SetRootSubjectDescription("Created by RoadRailPhysicalProjectHost");
    createDgnDbParams.SetStartDefaultTxn(DefaultTxn::Exclusive);

    DbResult createStatus;
    BeFileName fileName = BuildProjectFileName(baseName);
    DgnDbPtr projectPtr = DgnDb::CreateDgnDb(&createStatus, fileName, createDgnDbParams);
    if (!projectPtr.IsValid() || DbResult::BE_SQLITE_OK != createStatus)
        return nullptr;

    BeAssert(BentleyStatus::SUCCESS == projectPtr->Schemas().CreateClassViewsInDb());

    return projectPtr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        04/2015
//---------------------------------------------------------------------------------------
DgnDbPtr RoadRailPhysicalProjectHost::OpenProject(WCharCP baseName)
    {
    DbResult openStatus;
    BeFileName fileName = BuildProjectFileName(baseName);

    DgnDbPtr projectPtr = DgnDb::OpenDgnDb(&openStatus, fileName, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    if (!projectPtr.IsValid() || (DbResult::BE_SQLITE_OK != openStatus))
        return nullptr;

    return projectPtr;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
RoadRailPhysicalProjectHostImpl::RoadRailPhysicalProjectHostImpl() : m_isInitialized(false)
    {
    BeAssert((DgnPlatformLib::QueryHost() == NULL) && L"This means an old host is still registered. You should have terminated it first before creating a new host.");

    DgnPlatformLib::Initialize(*this, false);
    DgnDomains::RegisterDomain(LinearReferencingDomain::GetDomain(), DgnDomain::Required::Yes);
    DgnDomains::RegisterDomain(RoadRailAlignmentDomain::GetDomain(), DgnDomain::Required::Yes);
    DgnDomains::RegisterDomain(RoadRailPhysicalDomain::GetDomain(), DgnDomain::Required::Yes);
    m_isInitialized = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
RoadRailPhysicalProjectHostImpl::~RoadRailPhysicalProjectHostImpl()
    {
    if (m_isInitialized)
        {
        Terminate(false);
        }
    }


RoadRailPhysicalProjectHost* RoadRailPhysicalTestsFixture::m_host = nullptr;
DgnDbPtr RoadRailPhysicalTestsFixture::s_currentProject = DgnDbPtr(nullptr);
//---------------------------------------------------------------------------------------
// Automatically called by gTest framework before running every test
//---------------------------------------------------------------------------------------
void RoadRailPhysicalTestsFixture::SetUpTestCase()
    {
    m_host = new RoadRailPhysicalProjectHost();
    }
//---------------------------------------------------------------------------------------
// Automatically called by gTest framework after running every test
//---------------------------------------------------------------------------------------
void RoadRailPhysicalTestsFixture::TearDownTestCase()
    {
    if (s_currentProject.IsValid())
        s_currentProject->SaveChanges();

    s_currentProject = nullptr;

    delete m_host;
    m_host = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId RoadRailPhysicalTestsFixture::QueryFirstModelIdOfType(DgnDbR db, DgnClassId classId)
    {
    ECSqlStatement stmt;
    stmt.Prepare(db, "SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_Model) " WHERE ECClassId = ? LIMIT 1;");
    BeAssert(stmt.IsPrepared());

    stmt.BindId(1, classId);

    if (DbResult::BE_SQLITE_ROW != stmt.Step())
        return DgnModelId();

    return stmt.GetValueId<DgnModelId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnDbPtr RoadRailPhysicalTestsFixture::CreateProject(WCharCP baseName, bool needsSetBriefcase)
    {
    BeAssert(nullptr != m_host);

    const WCharCP testSeedName = L"TestSeed.bim";
    const BeFileName testSeedPath = m_host->BuildProjectFileName(testSeedName);
    const BeFileName projectName = m_host->BuildProjectFileName(baseName);

    //! Create seed
    if (!testSeedPath.DoesPathExist())
        {
        DgnDbPtr seedProject = m_host->CreateProject(testSeedName);

        //! Error
        if (seedProject.IsNull())
            return nullptr;

        seedProject->CloseDb();
        }

    if (s_currentProject.IsValid())
        {
        s_currentProject->SaveChanges();
        s_currentProject->CloseDb();
        s_currentProject = nullptr;
        }

    if (BeFileNameStatus::Success != BeFileName::BeCopyFile(testSeedPath.c_str(), projectName.c_str(), false))
        return nullptr;

    s_currentProject = m_host->OpenProject(baseName);
    if (s_currentProject.IsNull())
        return nullptr;

    if (needsSetBriefcase)
        {
        s_currentProject->SetAsBriefcase(BeBriefcaseId(1));
        s_currentProject->CloseDb();
        s_currentProject = m_host->OpenProject(baseName);
        }

    return s_currentProject;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Dgn::DgnDbPtr RoadRailPhysicalTestsFixture::OpenProject(WCharCP baseName, bool needsSetBriefcase)
    {
    BeAssert(nullptr != m_host);

    const BeFileName projectName = m_host->BuildProjectFileName(baseName);

    bool wantsCurrentProject = false;

    if (s_currentProject.IsValid())
        {
        Utf8String projectNameUtf(projectName.c_str());
        if (0 == projectNameUtf.CompareTo(s_currentProject->GetDbFileName()))
            wantsCurrentProject = true;
        }

    if (!wantsCurrentProject)
        s_currentProject = m_host->OpenProject(baseName);

    if (needsSetBriefcase && s_currentProject.IsValid())
        {
        s_currentProject->SetAsBriefcase(BeBriefcaseId(1));
        s_currentProject->SaveChanges();
        s_currentProject->CloseDb();
        s_currentProject = m_host->OpenProject(baseName);
        }

    return s_currentProject;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
RoadDesignSpeedDefinitionPtr RoadRailPhysicalTestsFixture::InsertRoadDesignSpeedDefinition(Dgn::DgnDbR db)
    {
    RoadDesignSpeedDefinitionPtr designSpeedDefPtr;

    // Insert dummy design-speed definition
    auto roadwayStandardsModelId = QueryFirstModelIdOfType(db, RoadwayStandardsModel::QueryClassId(db));
    if (!roadwayStandardsModelId.IsValid())
        return designSpeedDefPtr;

    auto designSpeedTablePtr = DesignSpeedDefinitionTable::Create(*RoadwayStandardsModel::Get(db, roadwayStandardsModelId), "Dummy", UnitSystem::Metric);

    DgnDbStatus status;
    DesignSpeedDefinitionModelPtr designSpeedDefModelPtr;
    if (designSpeedTablePtr->Insert(designSpeedDefModelPtr, &status).IsNull())
        return designSpeedDefPtr;

    designSpeedDefPtr = RoadDesignSpeedDefinition::Create(*designSpeedDefModelPtr, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    if (designSpeedDefPtr->Insert().IsNull())
        return designSpeedDefPtr;

    return designSpeedDefPtr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus RoadRailPhysicalTestsFixture::InsertTestPointNames(RoadwayStandardsModelCR model)
    {
    TravelwaySignificantPointDef::CreateAndInsert(model, "CL", "Center-line");
    TravelwaySignificantPointDef::CreateAndInsert(model, "EOTW_L", "Edge-of-Travelway left-side");
    TravelwaySignificantPointDef::CreateAndInsert(model, "EOTW_R", "Edge-of-Travelway right-side");

    TravelwaySideSignificantPointDef::CreateAndInsert(model, "EOS_L", "Edge-of-Shoulder left-side");
    TravelwaySideSignificantPointDef::CreateAndInsert(model, "EOS_R", "Edge-of-Shoulder right-side");
    
    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus RoadRailPhysicalTestsFixture::InsertFourLanes(TypicalSectionPortionBreakDownModelCR model)
    {
    auto definitionCPtr = TravelwayDefinitionElement::Get(model.GetDgnDb(), model.GetModeledElement()->GetElementId());
    DefinitionModelCP standardsModelCP = model.GetModeledElement()->GetModel()->ToDefinitionModel();

    auto clPointCPtr = TypicalSectionPoint::CreateAndInsert(model, *SignificantPointDefinition::QueryByCode(*standardsModelCP, "CL"));

    TravelwayDefinitionElement::SetOriginPoint(*definitionCPtr, *clPointCPtr);

    auto leftPointCPtr = TypicalSectionPoint::CreateAndInsert(model);

    bvector<ITypicalSectionConstraintPointCP> points;
    points.push_back(clPointCPtr.get());
    points.push_back(leftPointCPtr.get());

    TypicalSectionHorizontalConstraint::CreateAndInsert(*leftPointCPtr, *clPointCPtr, 
        *TypicalSectionConstraintConstantOffset::Create(model, -3.0), 1);
    TypicalSectionVerticalConstraint::CreateAndInsert(*leftPointCPtr, *clPointCPtr,
        *TypicalSectionConstraintConstantOffset::Create(model, -0.05), 2);

    RoadLaneComponent::CreateAndInsert(model, points);

    auto eotwLPointCPtr = TypicalSectionPoint::CreateAndInsert(model, *SignificantPointDefinition::QueryByCode(*standardsModelCP, "EOTW_L"));

    TypicalSectionSlopeConstraint::CreateAndInsert(*eotwLPointCPtr, *leftPointCPtr,
        *TypicalSectionConstraintConstantSlope::Create(model, -0.05), 1);
    TypicalSectionHorizontalConstraint::CreateAndInsert(*eotwLPointCPtr, *leftPointCPtr,
        *TypicalSectionConstraintConstantOffset::Create(model, 3.0), 2);    

    points.clear();
    points.push_back(leftPointCPtr.get());
    points.push_back(eotwLPointCPtr.get());

    RoadLaneComponent::CreateAndInsert(model, points);

    auto rightPointCPtr = TypicalSectionPoint::CreateAndInsert(model);
    TypicalSectionHorizontalConstraint::CreateAndInsert(*rightPointCPtr, *clPointCPtr,
        *TypicalSectionConstraintConstantOffset::Create(model, 3.0), 1);
    TypicalSectionVerticalConstraint::CreateAndInsert(*rightPointCPtr, *clPointCPtr,
        *TypicalSectionConstraintConstantOffset::Create(model, -0.05), 2);

    points.clear();
    points.push_back(clPointCPtr.get());
    points.push_back(rightPointCPtr.get());

    RoadLaneComponent::CreateAndInsert(model, points);

    auto eotwRPointCPtr = TypicalSectionPoint::CreateAndInsert(model, *SignificantPointDefinition::QueryByCode(*standardsModelCP, "EOTW_R"));
    TypicalSectionHorizontalConstraint::CreateAndInsert(*eotwRPointCPtr, *rightPointCPtr,
        *TypicalSectionConstraintConstantOffset::Create(model, 3.0), 1);
    TypicalSectionVerticalConstraint::CreateAndInsert(*eotwRPointCPtr, *rightPointCPtr,
        *TypicalSectionConstraintConstantOffset::Create(model, -0.05), 2);

    points.clear();
    points.push_back(rightPointCPtr.get());
    points.push_back(eotwRPointCPtr.get());

    RoadLaneComponent::CreateAndInsert(model, points);

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus RoadRailPhysicalTestsFixture::InsertShouldersRightSide(TypicalSectionPortionBreakDownModelCR model)
    {
    DefinitionModelCP standardsModelCP = model.GetModeledElement()->GetModel()->ToDefinitionModel();

    auto eotwRProxyCPtr = TypicalSectionProxyPoint::CreateAndInsert(model, *SignificantPointDefinition::QueryByCode(*standardsModelCP, "EOTW_R"));

    auto rightPointCPtr = TypicalSectionPoint::CreateAndInsert(model);

    TypicalSectionHorizontalConstraint::CreateAndInsert(*rightPointCPtr, *eotwRProxyCPtr,
        *TypicalSectionConstraintConstantOffset::Create(model, 1.0), 1);
    TypicalSectionVerticalConstraint::CreateAndInsert(*rightPointCPtr, *eotwRProxyCPtr,
        *TypicalSectionConstraintConstantOffset::Create(model, -0.01), 2);

    auto bottomRightPointCPtr = TypicalSectionPoint::CreateAndInsert(model);

    TypicalSectionHorizontalConstraint::CreateAndInsert(*bottomRightPointCPtr, *rightPointCPtr,
        *TypicalSectionConstraintConstantOffset::Create(model, 0.0), 1);
    TypicalSectionVerticalConstraint::CreateAndInsert(*bottomRightPointCPtr, *rightPointCPtr,
        *TypicalSectionConstraintConstantOffset::Create(model, -0.03), 2);

    auto bottomLeftPointCPtr = TypicalSectionPoint::CreateAndInsert(model);

    TypicalSectionHorizontalConstraint::CreateAndInsert(*bottomLeftPointCPtr, *eotwRProxyCPtr,
        *TypicalSectionConstraintConstantOffset::Create(model, 0.0), 1);
    TypicalSectionVerticalConstraint::CreateAndInsert(*bottomLeftPointCPtr, *eotwRProxyCPtr,
        *TypicalSectionConstraintConstantOffset::Create(model, -0.03), 2);

    bvector<ITypicalSectionConstraintPointCP> points;
    points.push_back(eotwRProxyCPtr.get());
    points.push_back(rightPointCPtr.get());
    points.push_back(bottomRightPointCPtr.get());
    points.push_back(bottomLeftPointCPtr.get());

    RoadShoulderComponent::CreateAndInsert(model, points); // Paved shoulder

    auto eosRPointCPtr = TypicalSectionPoint::CreateAndInsert(model, *SignificantPointDefinition::QueryByCode(*standardsModelCP, "EOS_R"));

    TypicalSectionHorizontalConstraint::CreateAndInsert(*eosRPointCPtr, *rightPointCPtr,
        *TypicalSectionConstraintConstantOffset::Create(model, 1.0), 1);
    TypicalSectionVerticalConstraint::CreateAndInsert(*eosRPointCPtr, *rightPointCPtr,
        *TypicalSectionConstraintConstantOffset::Create(model, -0.01), 2);

    auto bottomEOSRPointCPtr = TypicalSectionPoint::CreateAndInsert(model);

    TypicalSectionHorizontalConstraint::CreateAndInsert(*bottomEOSRPointCPtr, *eosRPointCPtr,
        *TypicalSectionConstraintConstantOffset::Create(model, 0.0), 1);
    TypicalSectionVerticalConstraint::CreateAndInsert(*bottomEOSRPointCPtr, *eosRPointCPtr,
        *TypicalSectionConstraintConstantOffset::Create(model, -0.03), 2);

    points.clear();
    points.push_back(rightPointCPtr.get());
    points.push_back(eosRPointCPtr.get());
    points.push_back(bottomEOSRPointCPtr.get());
    points.push_back(bottomRightPointCPtr.get());

    RoadShoulderComponent::CreateAndInsert(model, points); // Unpaved shoulder

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus RoadRailPhysicalTestsFixture::InsertSideSlopeRightSide(TypicalSectionPortionBreakDownModelCR model)
    {
    DefinitionModelCP standardsModelCP = model.GetModeledElement()->GetModel()->ToDefinitionModel();

    auto eosRProxyCPtr = TypicalSectionProxyPoint::CreateAndInsert(model, *SignificantPointDefinition::QueryByCode(*standardsModelCP, "EOS_R"));

    auto ecHingePointCPtr = TypicalSectionPoint::CreateAndInsert(model);

    TypicalSectionHorizontalConstraint::CreateAndInsert(*ecHingePointCPtr, *eosRProxyCPtr,
        *TypicalSectionConstraintConstantOffset::Create(model, 2.0), 1);
    TypicalSectionVerticalConstraint::CreateAndInsert(*ecHingePointCPtr, *eosRProxyCPtr,
        *TypicalSectionConstraintConstantOffset::Create(model, 0.0), 2);

    bvector<ITypicalSectionConstraintPointCP> points;
    points.push_back(eosRProxyCPtr.get());
    points.push_back(ecHingePointCPtr.get());

    BufferComponent::CreateAndInsert(model, points);

    auto ecFillPointCPtr = TypicalSectionPoint::CreateAndInsert(model);

    TypicalSectionSlopeConstraint::CreateAndInsert(*ecFillPointCPtr, *ecHingePointCPtr,
        *TypicalSectionConstraintConstantSlope::Create(model, 1.0), 1);
    TypicalSectionHorizontalConstraint::CreateAndInsert(*ecFillPointCPtr, *ecHingePointCPtr,
        *TypicalSectionConstraintConstantOffset::Create(model, 4.0), 2);    

    points.clear();
    points.push_back(ecHingePointCPtr.get());
    points.push_back(ecFillPointCPtr.get());

    SideSlopeConditionComponent::CreateAndInsert(model, 10, points); // Fill condition

    auto ecCutPointCPtr = TypicalSectionPoint::CreateAndInsert(model);

    TypicalSectionSlopeConstraint::CreateAndInsert(*ecCutPointCPtr, *ecHingePointCPtr,
        *TypicalSectionConstraintConstantSlope::Create(model, -1.0), 1);
    TypicalSectionHorizontalConstraint::CreateAndInsert(*ecCutPointCPtr, *ecHingePointCPtr,
        *TypicalSectionConstraintConstantOffset::Create(model, 4.0), 2);

    points.clear();
    points.push_back(ecHingePointCPtr.get());
    points.push_back(ecCutPointCPtr.get());

    SideSlopeConditionComponent::CreateAndInsert(model, 20, points); // Cut condition

    return BentleyStatus::SUCCESS;
    }