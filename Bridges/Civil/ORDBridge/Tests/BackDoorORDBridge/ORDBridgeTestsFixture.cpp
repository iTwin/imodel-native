#include "../BackDoorORDBridge/PublicApi/BackDoor/ORDBridge/BackDoor.h"

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
//! @bsiclass
//=======================================================================================
struct TestViewManager : ViewManager
    {
    protected:
        virtual Display::SystemContext* _GetSystemContext() override { return nullptr; }
        virtual bool _DoesHostHaveFocus() override { return true; }
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ORDBridgeTestsHostImpl : DgnViewLib::Host
    {
    bool m_isInitialized;

    ORDBridgeTestsHostImpl();
    ~ORDBridgeTestsHostImpl();

    virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new TestKnownLocationsAdmin(); }
    //virtual RepositoryAdmin & _SupplyRepositoryAdmin() override { return *new TestRepositoryAdmin(); }

    virtual void _SupplyProductName(Utf8StringR name) override { name.assign("ORDBridgeTestsHostImpl"); }
    virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() { return BeSQLite::L10N::SqlangFiles(BeFileName()); } // no translatable strings
    virtual ViewManager& _SupplyViewManager() override { return *new TestViewManager(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName ORDBridgeTestsHost::GetTestAppProductDirectory()
    {
    char* outPath = getenv("OutRoot");

    BeFileName testAppPath(outPath);
    testAppPath.AppendA("Winx64\\Product\\ORDBridge\\");

    return testAppPath;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
BeFileName ORDBridgeTestsHost::GetOutputDirectory()
    {
    BeFileName outputDir;
    BeTest::GetHost().GetOutputRoot(outputDir);
    outputDir.AppendToPath(L"CiviliModelBridges\\ORDBridge\\");
    return outputDir;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
BeFileName ORDBridgeTestsHost::GetDgnPlatformAssetsDirectory()
    {
    BeFileName assetsRootDirectory;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsRootDirectory);
    return assetsRootDirectory;
    }

WString* ORDBridgeTestsHost::GetInputFileArgument(BeFileName inputPath, WCharCP input)
{
    char* outPath = getenv("OutRoot");

    BeFileName inputPath1(outPath);
    inputPath1.AppendString(WCharCP(L"Winx64\\Product\\CiviliModelBridges-Tests\\Assets\\TestFiles\\ORD\\"));
    inputPath1.AppendString(WCharCP(input));

    WCharCP inputArgument(WString(WCharCP(L"--input=\"")).append(inputPath1.c_str()).append(WCharCP(L"\"")).c_str());
    auto inputArgumentAllocated = new WString(inputArgument);
    return inputArgumentAllocated;
}

WString* ORDBridgeTestsHost::GetOutputFileArgument(BeFileName outputPath1, WCharCP bimFileName)
{
    char* outPath = getenv("OutRoot");

    BeFileName outputPath = GetOutputDirectory();
    outputPath.AppendString(WCharCP(bimFileName));
    WCharCP outputArgument(WString(WCharCP(L"--output=\"")).append(outputPath.c_str()).append(WCharCP(L"\"")).c_str());
    auto outputArgumentAllocated = new WString(outputArgument);
    return outputArgumentAllocated;
}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
BeFileName ORDBridgeTestsHost::BuildProjectFileName(WCharCP baseName)
    {
    BeFileName projectFileName = GetOutputDirectory();
    projectFileName.AppendToPath(baseName);
    return projectFileName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
ORDBridgeTestsHost::ORDBridgeTestsHost()
    {
    m_pimpl = new ORDBridgeTestsHostImpl;
    CleanOutputDirectory();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
ORDBridgeTestsHost::~ORDBridgeTestsHost()
    {
    delete m_pimpl;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
void ORDBridgeTestsHost::CleanOutputDirectory()
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
// @bsimethod                                   Jonathan.DeCarlo                07/2019
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::IKnownLocationsAdmin& ORDBridgeTestsHost::_SupplyIKnownLocationsAdmin()
    {
    return m_pimpl->_SupplyIKnownLocationsAdmin();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Jonathan.DeCarlo                07/2019
//---------------------------------------------------------------------------------------
void ORDBridgeTestsHost::_SupplyProductName(Utf8StringR name)
    {
    m_pimpl->_SupplyProductName(name);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Jonathan.DeCarlo                07/2019
//---------------------------------------------------------------------------------------
BeSQLite::L10N::SqlangFiles ORDBridgeTestsHost::_SupplySqlangFiles()
    {
    return m_pimpl->_SupplySqlangFiles();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Jonathan.DeCarlo                07/2019
//---------------------------------------------------------------------------------------
Dgn::ViewManager& ORDBridgeTestsHost::_SupplyViewManager()
    {
    return m_pimpl->_SupplyViewManager();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
ORDBridgeTestsHostImpl::ORDBridgeTestsHostImpl() : m_isInitialized(false)
    {
    m_isInitialized = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
ORDBridgeTestsHostImpl::~ORDBridgeTestsHostImpl()
    {
    }

ORDBridgeTestsHost* CiviliModelBridgesORDBridgeTestsFixture::m_host = nullptr;

//---------------------------------------------------------------------------------------
// Automatically called by gTest framework before running every test
//---------------------------------------------------------------------------------------
void CiviliModelBridgesORDBridgeTestsFixture::SetUpTestCase()
    {
    m_host = new ORDBridgeTestsHost();
    }
//---------------------------------------------------------------------------------------
// Automatically called by gTest framework after running every test
//---------------------------------------------------------------------------------------
void CiviliModelBridgesORDBridgeTestsFixture::TearDownTestCase()
    {
    delete m_host;
    m_host = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool CiviliModelBridgesORDBridgeTestsFixture::CopyTestFile(Utf8CP source, Utf8CP target)
    {
    char* outPath = getenv("OutRoot");

    BeFileName sourcePath(outPath);
    sourcePath.AppendA("Winx64\\Product\\CiviliModelBridges-Tests\\Assets\\TestFiles\\ORD\\");
    sourcePath.AppendA(source);

    BeFileName targetPath(outPath);
    targetPath.AppendA("Winx64\\Product\\CiviliModelBridges-Tests\\Assets\\TestFiles\\ORD\\");
    targetPath.AppendA(target);

    return (BeFileNameStatus::Success == BeFileName::BeCopyFile(sourcePath, targetPath, false));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool CiviliModelBridgesORDBridgeTestsFixture::RunTestApp(WCharCP input, WCharCP bimFileName, bool updateMode)
    {
    char* outPath = getenv("OutRoot");
    BeFileName testAppPath = m_host->GetTestAppProductDirectory();
    testAppPath.AppendA("PublishORDToBim.exe");

    BeFileName inputPath(outPath);
    inputPath.AppendString(WCharCP(L"Winx64\\Product\\CiviliModelBridges-Tests\\Assets\\TestFiles\\ORD\\"));
    inputPath.AppendString(WCharCP(input));

    BeFileName outputPath = m_host->GetOutputDirectory();
    outputPath.AppendString(WCharCP(bimFileName));

    WCharCP testAppPathArgument = testAppPath;
    auto inputArgument = m_host->GetInputFileArgument(inputPath, input);
    auto outputArgument = m_host->GetOutputFileArgument(outputPath, bimFileName);
    WCharCP noAssertDialoguesArgument = WCharCP(L"--no-assert-dialogs");
    WCharCP command[4] = { testAppPathArgument, WCharCP((*inputArgument).c_str()), WCharCP((*outputArgument).c_str()), noAssertDialoguesArgument };

    int errcode = PublishORDToBimDLL::RunBridge(4, command);

    //had to store the arguments on the heap to prevent the strings from colliding
    delete inputArgument;
    delete outputArgument;

    bool retVal = (0 == errcode);
    
    if (retVal)
        {
        if (!outputPath.DoesPathExist())
            BeAssert(false);
        }

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr CiviliModelBridgesORDBridgeTestsFixture::VerifyConvertedElements(Utf8CP bimFileName, size_t alignmentCount, size_t corridorCount)
    {
    BeFileName outputPath = m_host->GetOutputDirectory();
    outputPath.AppendA(bimFileName);

    DbResult result;
    DgnDbPtr dgnDbPtr = DgnDb::OpenDgnDb(&result, outputPath, DgnDb::OpenParams(Db::OpenMode::Readonly));

    ECSqlStatement stmt;
    stmt.Prepare(*dgnDbPtr, "SELECT COUNT(*) FROM "
        BRRA_SCHEMA(BRRA_CLASS_Alignment) " a," BIS_SCHEMA(BIS_CLASS_Model) " m, " BRRA_SCHEMA(BRRA_CLASS_DesignAlignments) " d "
        "WHERE m.ModeledElement.Id = d.ECInstanceId AND a.Model.Id = m.ECInstanceId ");
    BeAssert(stmt.IsPrepared());
    BeAssert(DbResult::BE_SQLITE_ROW == stmt.Step());
    BeAssert(alignmentCount == stmt.GetValueInt(0));
    
    stmt.Finalize();

    stmt.Prepare(*dgnDbPtr, "SELECT COUNT(*) FROM "
        BRRP_SCHEMA(BRRP_CLASS_Corridor) " c ");
    BeAssert(stmt.IsPrepared());

    DbResult stepResult = stmt.Step();
    if (stepResult == DbResult::BE_SQLITE_DONE)
        BeAssert(corridorCount == 0);
    else
        {
		BeAssert(stepResult == DbResult::BE_SQLITE_ROW);
        BeAssert(corridorCount == stmt.GetValueInt(0));
		}

    return dgnDbPtr;
    }