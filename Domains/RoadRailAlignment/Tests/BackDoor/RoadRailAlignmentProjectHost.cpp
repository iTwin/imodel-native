#include "../BackDoor/PublicApi/BackDoor/RoadRailAlignment/BackDoor.h"
#include <DgnPlatform/DgnPlatformLib.h>

#include <DgnPlatform/DgnGeoCoord.h>

BEGIN_BENTLEY_ROADRAILALIGNMENT_UNITTESTS_NAMESPACE

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
struct RoadRailAlignmentProjectHostImpl : DgnPlatformLib::Host
    {
    bool m_isInitialized;

    RoadRailAlignmentProjectHostImpl();
    ~RoadRailAlignmentProjectHostImpl();

    virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new TestKnownLocationsAdmin(); }
    virtual RepositoryAdmin & _SupplyRepositoryAdmin() override { return *new TestRepositoryAdmin(); }

    //virtual HandlerAdmin& _SupplyHandlerAdmin() override { return *new TestHandlerAdmin; }
    virtual void _SupplyProductName(Utf8StringR name) override { name.assign("RoadRailAlignmentProjectHost"); }
    virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override { return BeSQLite::L10N::SqlangFiles(BeFileName()); } // no translatable strings
    };

END_BENTLEY_ROADRAILALIGNMENT_UNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
RoadRailAlignmentProjectHost::RoadRailAlignmentProjectHost()
    {
    m_pimpl = new RoadRailAlignmentProjectHostImpl;
    CleanOutputDirectory();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
RoadRailAlignmentProjectHost::~RoadRailAlignmentProjectHost()
    {
    delete m_pimpl;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
void RoadRailAlignmentProjectHost::CleanOutputDirectory()
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
BeFileName RoadRailAlignmentProjectHost::GetOutputDirectory()
    {
    BeFileName outputDir;
    BeTest::GetHost().GetOutputRoot(outputDir);
    outputDir.AppendToPath(L"RoadRailAlignment");
    return outputDir;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
BeFileName RoadRailAlignmentProjectHost::GetDgnPlatformAssetsDirectory()
    {
    BeFileName assetsRootDirectory;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsRootDirectory);
    return assetsRootDirectory;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
BeFileName RoadRailAlignmentProjectHost::BuildProjectFileName(WCharCP baseName)
    {
    BeFileName projectFileName = GetOutputDirectory();
    projectFileName.AppendToPath(baseName);
    return projectFileName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
DgnDbPtr RoadRailAlignmentProjectHost::CreateProject(WCharCP baseName)
    {
    CreateDgnDbParams createDgnDbParams;
    createDgnDbParams.SetOverwriteExisting(true);
    createDgnDbParams.SetRootSubjectName("RoadRailAlignmentProject");
    createDgnDbParams.SetRootSubjectDescription("Created by RoadRailAlignmentProjectHost");
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
DgnDbPtr RoadRailAlignmentProjectHost::OpenProject(WCharCP baseName)
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
RoadRailAlignmentProjectHostImpl::RoadRailAlignmentProjectHostImpl() : m_isInitialized(false)
    {
    BeAssert((DgnPlatformLib::QueryHost() == NULL) && L"This means an old host is still registered. You should have terminated it first before creating a new host.");

    DgnPlatformLib::Initialize(*this, false);
    DgnDomains::RegisterDomain(LinearReferencingDomain::GetDomain(), DgnDomain::Required::Yes);
    DgnDomains::RegisterDomain(RoadRailAlignmentDomain::GetDomain(), DgnDomain::Required::Yes);
    m_isInitialized = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
RoadRailAlignmentProjectHostImpl::~RoadRailAlignmentProjectHostImpl()
    {
    if (m_isInitialized)
        {
        Terminate(false);
        }
    }


RoadRailAlignmentProjectHost* RoadRailAlignmentTestsFixture::m_host = nullptr;
DgnDbPtr RoadRailAlignmentTestsFixture::s_currentProject = DgnDbPtr(nullptr);
//---------------------------------------------------------------------------------------
// Automatically called by gTest framework before running every test
//---------------------------------------------------------------------------------------
void RoadRailAlignmentTestsFixture::SetUpTestCase()
    {
    m_host = new RoadRailAlignmentProjectHost();
    }
//---------------------------------------------------------------------------------------
// Automatically called by gTest framework after running every test
//---------------------------------------------------------------------------------------
void RoadRailAlignmentTestsFixture::TearDownTestCase()
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
DgnModelId RoadRailAlignmentTestsFixture::QueryFirstModelIdOfType(DgnDbR db, DgnClassId classId)
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
DgnDbPtr RoadRailAlignmentTestsFixture::CreateProject(WCharCP baseName, bool needsSetBriefcase)
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

        if (DbResult::BE_SQLITE_OK != seedProject->SaveChanges())
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
Dgn::DgnDbPtr RoadRailAlignmentTestsFixture::OpenProject(WCharCP baseName, bool needsSetBriefcase)
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
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
CategorySelectorPtr RoadRailAlignmentTestsFixture::CreateSpatialCategorySelector(DefinitionModelR model)
    {
    // A CategorySelector is a definition element that is potentially shared by many ViewDefinitions.
    // To start off, we'll create a default selector that includes the one category that we use.
    // We have to give the selector a unique name of its own. Since we are settup up a new bim, we know that we can safely choose any name.
    auto categorySelector = new CategorySelector(model, "Default Spatial Categories");
    categorySelector->AddCategory(AlignmentCategory::Get(model.GetDgnDb()));
    return categorySelector;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
CategorySelectorPtr RoadRailAlignmentTestsFixture::CreateDrawingCategorySelector(DefinitionModelR model)
    {
    // A CategorySelector is a definition element that is potentially shared by many ViewDefinitions.
    // To start off, we'll create a default selector that includes the one category that we use.
    // We have to give the selector a unique name of its own. Since we are settup up a new bim, we know that we can safely choose any name.
    auto categorySelector = new CategorySelector(model, "Default Drawing Categories");
    categorySelector->AddCategory(AlignmentCategory::GetHorizontal(model.GetDgnDb()));
    categorySelector->AddCategory(AlignmentCategory::GetVertical(model.GetDgnDb()));
    return categorySelector;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
ModelSelectorPtr RoadRailAlignmentTestsFixture::CreateModelSelector(DefinitionModelR definitionModel, Utf8StringCR name)
    {
    // A ModelSelector is a definition element that is potentially shared by many ViewDefinitions.
    // To start off, we'll create a default selector that includes the one model that we use.
    // We have to give the selector a unique name of its own. Since we are settup up a new bim, we know that we can safely choose any name.
    return new ModelSelector(definitionModel, name);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
DisplayStyle2dPtr RoadRailAlignmentTestsFixture::CreateDisplayStyle2d(DefinitionModelR model)
    {
    auto displayStyle = new DisplayStyle2d(model, "Default-2d");
    displayStyle->SetBackgroundColor(ColorDef::White());
    return displayStyle;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
DisplayStyle3dPtr RoadRailAlignmentTestsFixture::CreateDisplayStyle3d(DefinitionModelR model)
    {
    // DisplayStyle is a definition element that is potentially shared by many ViewDefinitions.
    // To start off, we'll create a style that can be used as a good default for 3D views.
    // We have to give the style a unique name of its own. Since we are settup up a new bim, we know that we can safely choose any name.
    auto displayStyle = new DisplayStyle3d(model, "Default-3d");
    displayStyle->SetBackgroundColor(ColorDef::White());
    Render::ViewFlags viewFlags = displayStyle->GetViewFlags();
    viewFlags.SetRenderMode(Render::RenderMode::SmoothShade);
    displayStyle->SetViewFlags(viewFlags);
    return displayStyle;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus RoadRailAlignmentTestsFixture::Create3dView(DefinitionModelR model, Utf8CP viewName,
    CategorySelectorR categorySelector, ModelSelectorR modelSelector, DisplayStyle3dR displayStyle)
    {
    DgnDbR db = model.GetDgnDb();

    // CategorySelector, ModelSelector, and DisplayStyle are definition elements that are normally shared by many ViewDefinitions.
    // That is why they are inputs to this function. 
    OrthographicViewDefinition view(model, viewName, categorySelector, displayStyle, modelSelector);

    DgnViewId viewId;
    DgnViewId existingViewId = ViewDefinition::QueryViewId(db, view.GetCode());
    if (existingViewId.IsValid())
        viewId = existingViewId;
    else
        {
        // Define the view direction and volume.
        view.SetStandardViewRotation(Dgn::StandardView::Top); // Default to a top view
        view.LookAtVolume(db.GeoLocation().GetProjectExtents()); // A good default for a new view is to "fit" it to the contents of the bim.

                                                                 // Write the ViewDefinition to the bim
        if (!view.Insert().IsValid())
            return BentleyStatus::ERROR;

        viewId = view.GetViewId();
        }

    if (!viewId.IsValid())
        return BentleyStatus::ERROR;

    DgnViewId defaultViewId;
    if (db.QueryProperty(&defaultViewId, sizeof(defaultViewId), DgnViewProperty::DefaultView()) != BeSQLite::DbResult::BE_SQLITE_ROW)
        db.SaveProperty(DgnViewProperty::DefaultView(), &viewId, (uint32_t) sizeof(viewId));

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus RoadRailAlignmentTestsFixture::Create2dView(DefinitionModelR model, Utf8CP viewName, 
    CategorySelectorR categorySelector, DgnModelId modelToDisplay, DisplayStyle2dR displayStyle)
    {
    DgnDbR db = model.GetDgnDb();

    // CategorySelector and DisplayStyle are definition elements that are normally shared by many ViewDefinitions.
    // That is why they are inputs to this function. 
    DrawingViewDefinition view(model, viewName, modelToDisplay, categorySelector, displayStyle);

    DgnViewId viewId;
    DgnViewId existingViewId = ViewDefinition::QueryViewId(db, view.GetCode());
    if (existingViewId.IsValid())
        viewId = existingViewId;
    else
        {
        // Define the view direction and volume.
        view.SetStandardViewRotation(Dgn::StandardView::Top); // Default to a top view
        view.LookAtVolume(db.GeoLocation().GetProjectExtents()); // A good default for a new view is to "fit" it to the contents of the bim.

                                                                 // Write the ViewDefinition to the bim
        if (!view.Insert().IsValid())
            return BentleyStatus::ERROR;

        viewId = view.GetViewId();
        }

    if (!viewId.IsValid())
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }